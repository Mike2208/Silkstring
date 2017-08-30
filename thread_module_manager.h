#ifndef THREAD_MODULE_MANAGER_H
#define THREAD_MODULE_MANAGER_H

/*! \file thread_module_manager.h
 *  \brief Header for ThreadModuleManager class
 */


#include "thread_queued.h"
#include "error_exception.h"

#include <list>
#include <memory>
#include <functional>
#include <assert.h>

/*!
 *  \brief Namespace for ThreadModuleManager class
 */
namespace thread_module_manager
{
	using std::shared_ptr;

	using std::list;
	using std::mutex;

	using std::atomic;

	using std::function;
	using std::bind;

	using error_exception::Exception;
	using error_exception::ERROR_NUM;

	using thread_queued::ThreadQueued;
	using thread_queued::thread_state_t;
	using thread_queued::THREAD_PAUSED;
	using thread_queued::THREAD_RUNNING;
	using thread_queued::THREAD_STOPPED;

	/*!
	 * \brief Module that can be registered with the manager
	 */
	template<class Identifier, class ...ModuleParameters>
	class ThreadModule
	{
			using thread_t = ThreadQueued<Identifier, ModuleParameters...>;

		public:

			using msg_struct_t = typename thread_t::msg_struct_t;

			ThreadModule(const Identifier ID)
				: _ID(ID)
			{}

			ThreadModule(const ThreadModule &S) = delete;
			ThreadModule &operator=(const ThreadModule &S) = delete;

			ThreadModule(ThreadModule &&S) = default;
			ThreadModule &operator=(ThreadModule &&S) = default;

			virtual ~ThreadModule() = default;

			virtual void HandleMessage(msg_struct_t &Parameters) = 0;

			const Identifier &GetID() const
			{
				return this->_ID;
			}

		protected:

			/*!
			 * \brief ID of this module
			 */
			Identifier _ID;

			template<class U>
			friend class ::TestingClass;
	};

	template<class Identifier, class ...ModuleParameters>
	using ThreadModuleSharedPtr = shared_ptr<ThreadModule<Identifier, ModuleParameters...>>;

	/*!
	 * \brief The ThreadModuleManager class
	 */
	template<class Identifier, class ...ModuleParameters>
	class ThreadModuleManager : protected ThreadQueued<Identifier, ModuleParameters...>
	{
			/*!
			 *	\brief Thread
			 */
			using thread_t = ThreadQueued<Identifier, ModuleParameters...>;

		protected:
			using msg_struct_t = typename thread_t::msg_struct_t;

			using callback_fcn_t = typename thread_t::message_fcn_t;

			/*!
			 * \brief Number of ID in param_t template
			 */
			static constexpr auto _ParamIDNumber = 0;

		public:

			using module_t = ThreadModule<Identifier, ModuleParameters...>;
			using module_shared_ptr_t = ThreadModuleSharedPtr<Identifier, ModuleParameters...>;

			using module_list_t = list<module_shared_ptr_t>;

			/*!
			 * 	\brief Constructor
			 */
			ThreadModuleManager(thread_state_t ThreadState = THREAD_RUNNING)
				: thread_t(&ThreadModuleManager::MessageCallback, this, THREAD_PAUSED),
				  _ModuleListLock(),
				  _Modules()
				  //_MessageCallbackFcn(bind(&ThreadModuleManager::MessageCallback, std::placeholders::_1, std::placeholders::_2))
			{
				//this->SetMessageFunction(const_cast<const function<callback_fcn_t>&>(_MessageCallbackFcn).template target<callback_fcn_t>());
				this->SetThreadState(ThreadState);
			}

			/*!
			 * \brief Copy Constructor. Only one instance allowed
			 */
			ThreadModuleManager(const ThreadModuleManager &S) = delete;

			/*!
			 * \brief Move Constructor. Changes the _MessageCallbackFcn to point to the new class
			 */
			ThreadModuleManager(ThreadModuleManager &&S)
				: thread_t(&ThreadModuleManager::MessageCallback, this, THREAD_PAUSED)
				  //_MessageCallbackFcn(bind(&ThreadModuleManager::MessageCallback, std::placeholders::_1, this))
			{
				auto tmpState = static_cast<thread_t &>(S).GetThreadState();

				// Pause thread
				static_cast<thread_t &>(S).SetThreadState(THREAD_PAUSED);

				// Move module list
				this->_ModuleListLock.lock();
				S._ModuleListLock.lock();

				// Check if any modules were added before lock
				if(this->_Modules.empty())
					this->_Modules = std::move(S._Modules);
				else
				{
					// If yes, save the new mocules
					auto tmpList = std::move(this->_Modules);

					this->_Modules = std::move(S._Modules);

					// Reregister these elements
					for(const auto &curModule : tmpList)
					{
						this->RegisterNoLock(curModule);
					}
				}

				this->_ModuleListLock.unlock();

				// Move thread
				static_cast<thread_t &>(*this) = std::move(static_cast<thread_t &>(S));

				// Set callback function
				//static_cast<thread_t &>(*this).SetMessageFunction(const_cast<const function<callback_fcn_t>&>(_MessageCallbackFcn).template target<callback_fcn_t>());

				// Set proper state again
				static_cast<thread_t &>(*this).SetThreadState(tmpState);
			}

			/*!
			 * \brief Copy Operator. Only one instance allowed
			 */
			ThreadModuleManager &operator=(const ThreadModuleManager &S) = delete;

			/*!
			 * \brief Move Operator.
			 */
			ThreadModuleManager &operator=(ThreadModuleManager &&S) = delete;

			~ThreadModuleManager()
			{
				// Stop thread before deleting module vector
				this->~thread_t();

				// Wait for thread to complete
				this->Wait();

				// Erase modules
				this->_ModuleListLock.lock();

				auto curModuleIterator = this->_Modules.begin();
				while(curModuleIterator != this->_Modules.end())
				{
					curModuleIterator = this->_Modules.erase(curModuleIterator);
				}

				this->_ModuleListLock.unlock();
			}

			void Register(const module_shared_ptr_t &NewModule)
			{
				// Lock list
				this->_ModuleListLock.lock();

				this->RegisterNoLock(NewModule);

				// Unlock list
				this->_ModuleListLock.unlock();
			}

			module_shared_ptr_t Unregister(const Identifier IDToUnregister)
			{
				module_shared_ptr_t retVal;

				// Lock list
				this->_ModuleListLock.lock();

				// Check whether module with this ID already exists
				for(auto curIterator = this->_Modules.begin(); curIterator != this->_Modules.end(); ++curIterator)
				{
					if((*curIterator)->GetID() == IDToUnregister)
					{
						// Store shared ptr
						retVal = *curIterator;

						this->_Modules.erase(curIterator);

						break;
					}
				}

				// Unlock list
				this->_ModuleListLock.unlock();

				return retVal;
			}

			template<class FcnIdentifier, class ...FcnModuleParameters>
			void PushMessage(FcnIdentifier &&ID, FcnModuleParameters &&...Data)
			{
				static_cast<thread_t &>(*this).PushMessage(std::forward<FcnIdentifier>(ID), std::forward<FcnModuleParameters>(Data)...);
			}

			template<class FcnMessageStruct>
			void PushMessage(FcnMessageStruct &&Message)
			{
				static_assert(message_queue::template_convertible<FcnMessageStruct, msg_struct_t>::value,
							  "ERROR ThreadModuleManager::PushMessage(): Function Argument must match template parameter");

				static_cast<thread_t &>(*this).PushMessage(std::forward<FcnMessageStruct>(Message));
			}

			template<class FcnIdentifier, class ...FcnModuleParameters>
			void PushMessageFront(FcnIdentifier &&ID, FcnModuleParameters &&...Data)
			{
				static_cast<thread_t &>(*this).PushMessageFront(std::forward<FcnIdentifier>(ID), std::forward<FcnModuleParameters>(Data)...);
			}

			template<class FcnMessageStruct>
			void PushMessageFront(FcnMessageStruct &&Message)
			{
				static_assert(message_queue::template_convertible<FcnMessageStruct, msg_struct_t>::value,
							  "ERROR ThreadModuleManager::PushMessage(): Function Argument must match template parameter");

				static_cast<thread_t &>(*this).PushMessageFront(std::forward<FcnMessageStruct>(Message));
			}


			void SetThreadState(thread_state_t ThreadState)
			{
				static_cast<thread_t &>(*this).SetThreadState(ThreadState);
			}

			bool IsQueueEmpty() const
			{
				return thread_t::IsQueueEmpty();
			}

			void SetMessageAcceptance(bool AllowNewMessages)
			{
				this->thread_t::SetMessageAcceptance(AllowNewMessages);
			}

			bool GetMessageAcceptance() const
			{
				return this->thread_t::GetMessageAcceptance();
			}

			const module_list_t &GetModulesNoLock() const
			{
				return this->_Modules;
			}

		protected:

			/*!
			 * \brief Make sure list is only accessed by one thread at a time
			 */
			mutex						_ModuleListLock;

			/*!
			 * \brief All registered modules
			 */
			module_list_t				_Modules;

			/*!
			 * \brief Pointer to function that handles message callbacks
			 */
			//function<callback_fcn_t> _MessageCallbackFcn = bind(&ThreadModuleManager::MessageCallback, std::placeholders::_1, this);

			/*!
			 * \brief Function that handles a message
			 * \param MessageData Data of message
			 */
			static void MessageCallback(msg_struct_t &MessageData, void *Class)
			{
				auto *const pClass = reinterpret_cast<ThreadModuleManager*>(Class);
				pClass->PropagateMessageToModule(MessageData, MessageData.template Get<pClass->_ParamIDNumber>());
				//this->PropagateMessageToModule(MessageData, MessageData.template Get<this->_ParamIDNumber>());
			}

			void PropagateMessageToModule(msg_struct_t &MessageData, const Identifier &ModuleID)
			{
				// Lock list
				this->_ModuleListLock.lock();

				PropagateMessageToModuleNoLock(MessageData, ModuleID);

				// Unlock list
				this->_ModuleListLock.unlock();
			}

			void PropagateMessageToModuleNoLock(msg_struct_t &MessageData, const Identifier &ModuleID)
			{
				// Find module
				auto tmpPtr = this->GetModuleNoLock(ModuleID);

				// Handle message
				if(tmpPtr != nullptr)
					HandleModuleData(tmpPtr, MessageData);
			}

			typename module_list_t::value_type GetModule(const Identifier &ModuleID)
			{
				// Lock list
				this->_ModuleListLock.lock();

				// Find correct element
				auto tmpPtr = this->GetModuleNoLock(ModuleID);

				// Unlock list
				this->_ModuleListLock.unlock();

				return tmpPtr;
			}

			typename module_list_t::value_type GetModuleNoLock(const Identifier &ModuleID)
			{
				// Find correct element
				for(const auto &curModule : this->_Modules)
				{
					if(curModule->GetID() == ModuleID)
					{
						// Store module pointer in temporary variable
						return curModule;

						break;
					}
				}

				return nullptr;
			}

			void HandleModuleData(typename module_list_t::value_type &Module, msg_struct_t &MessageData)
			{
				// Handle message
				Module->HandleMessage(MessageData);
			}

		private:

			void RegisterNoLock(const module_shared_ptr_t &NewModule)
			{
				// Check whether module with this ID already exists
				for(const auto &curModule : this->_Modules)
				{
					if(curModule->GetID() == NewModule->GetID())
					{
						// Unlock list
						this->_ModuleListLock.unlock();

						throw Exception(ERROR_NUM, "ERROR ThreadModuleManager::Register(): Element with this ID already exists\n");
					}
				}

				// Add Module if not yet registered
				this->_Modules.push_back(NewModule);
			}

			template<class U>
			friend class ::TestingClass;
	};
} // namespace thread_module_manager


#endif // THREAD_MODULE_MANAGER_H
