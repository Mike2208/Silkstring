#ifndef THREAD_MODULE_MANAGER_MULTI_MESSAGE_H
#define THREAD_MODULE_MANAGER_MULTI_MESSAGE_H

/*! \file thread_module_manager_multi_message.h
 *  \brief Header for ThreadModuleManagerMultiMessage class
 */


#include "testing_class_declaration.h"
#include "thread_module_manager.h"
#include "vector_t.h"

/*!
 *  \brief Namespace for ThreadModuleManagerMultiMessage class
 */
namespace thread_module_manager_multi_message
{
	using std::mutex;
	using std::shared_ptr;

	using vector_t::vector_type;

	using message_queue::message_struct_t;

	using thread_module_manager::ThreadModule;
	using thread_module_manager::ThreadModuleManager;

	using thread_module_manager::thread_state_t;
	using thread_module_manager::THREAD_PAUSED;
	using thread_module_manager::THREAD_RUNNING;
	using thread_module_manager::THREAD_STOPPED;

	template<class ...MessageParameters>
	using message_shared_ptr_t = shared_ptr<message_struct_t<MessageParameters...>>;

	template<class Identifier>
	using module_id_vector_t = vector_type<Identifier>;

	/*!
	 * \brief Struct linking Sending ID with the IDs of the modules a message from this sender should reach
	 */
	template<class Identifier>
	struct module_id_link_t : public module_id_vector_t<Identifier>
	{
		using id_vector_t = module_id_vector_t<Identifier>;

		Identifier SendID;

		typename id_vector_t::iterator Find(const Identifier &ModuleID)
		{
			for(auto curIterator = id_vector_t::begin(); curIterator != id_vector_t::end(); ++curIterator)
			{
				if(*curIterator == ModuleID)
					return curIterator;
			}

			return id_vector_t::end();
		}

		module_id_link_t(const Identifier &_SendID, id_vector_t &&_ModuleIDs)
			: id_vector_t(std::forward<id_vector_t>(_ModuleIDs)),
			  SendID(_SendID)
		{}
	};

	template<class Identifier>
	using module_id_link_vector_type = vector_type<module_id_link_t<Identifier>>;

	template<class Identifier>
	struct module_id_link_vector_t : public module_id_link_vector_type<Identifier>
	{
		typename module_id_link_vector_t::iterator Find(const Identifier &SendID)
		{
			for(auto curIterator = module_id_link_vector_t::begin(); curIterator != module_id_link_vector_t::end(); ++curIterator)
			{
				if(curIterator->SendID == SendID)
					return curIterator;
			}

			return module_id_link_vector_t::end();
		}
	};

	/*!
	 * \brief Same as the ThreadModuleManager, only with the added option of sending messages to multiple messages at once
	 */
	template<class Identifier, class ...MessageParameters>
	class ThreadModuleManagerMultiMessage : public ThreadModuleManager<Identifier, Identifier, MessageParameters...>
	{
		protected:
			using module_manager_t = ThreadModuleManager<Identifier, Identifier, MessageParameters...>;

		private:
			using module_extra_message_t = message_struct_t<MessageParameters...>;

			using id_link_t = module_id_link_t<Identifier>;

			static constexpr auto _ParamReceiveIDNumber = module_manager_t::_ParamIDNumber;
			static constexpr auto _ParamSendIDNumber = 1;

		public:
			using msg_struct_t = typename module_manager_t::msg_struct_t;

			using module_t = typename module_manager_t::module_t;
			using module_shared_ptr_t = typename module_manager_t::module_shared_ptr_t;

			using id_vector_t = typename id_link_t::id_vector_t;

			using id_link_vector_t = module_id_link_vector_t<Identifier>;

			/*!
			 * 	\brief Constructor
			 */
			ThreadModuleManagerMultiMessage()
				: module_manager_t(THREAD_PAUSED)
			{
				//this->_MessageCallbackFcn = bind(&ThreadModuleManagerMultiMessage::MessageCallback, this, std::placeholders::_1);
				this->SetMessageFunction(&ThreadModuleManagerMultiMessage::MessageCallback);
				this->SetExtraData(this);

				module_manager_t::SetThreadState(THREAD_RUNNING);
			}

			ThreadModuleManagerMultiMessage(thread_state_t ThreadState)
				: module_manager_t(thread_queued::THREAD_PAUSED)
			{
				//this->_MessageCallbackFcn = bind(&ThreadModuleManagerMultiMessage::MessageCallback, this, std::placeholders::_1);
				this->SetMessageFunction(&ThreadModuleManagerMultiMessage::MessageCallback);
				this->SetExtraData(this);

				module_manager_t::SetThreadState(ThreadState);
			}

			ThreadModuleManagerMultiMessage(ThreadModuleManagerMultiMessage &&S)
				: module_manager_t(std::move(static_cast<module_manager_t&>(S)))
			{
				const auto lastState = this->GetThreadState();

				this->SetThreadState(THREAD_PAUSED);
				S.SetThreadState(THREAD_PAUSED);
				//this->_MessageCallbackFcn = bind(&ThreadModuleManagerMultiMessage::MessageCallback, this, std::placeholders::_1);
				this->SetMessageFunction(&ThreadModuleManagerMultiMessage::MessageCallback);
				this->SetExtraData(this);

				this->_LockLinks.lock();
				S._LockLinks.lock();

				// Check if links were added before lock
				MoveLinksNoLock(this->_SenderIDLinks, S._SenderIDLinks);
				MoveLinksNoLock(this->_ReceiverIDLinks, S._ReceiverIDLinks);

				this->_LockLinks.unlock();
				S._LockLinks.unlock();

				this->SetThreadState(lastState);
			}

			ThreadModuleManagerMultiMessage(const ThreadModuleManagerMultiMessage &S) = delete;
			ThreadModuleManagerMultiMessage &operator=(ThreadModuleManagerMultiMessage &&S) = delete;
			ThreadModuleManagerMultiMessage &operator=(const ThreadModuleManagerMultiMessage &S) = delete;

			~ThreadModuleManagerMultiMessage()
			{
				// Stop thread before deleting Link list
				this->~module_manager_t();
			}

			void AddSendLink(const Identifier &SendID, const Identifier &ModuleID)
			{
				// Lock vector
				this->_LockLinks.lock();

				// Link this ID
				ThreadModuleManagerMultiMessage::LinkIDsNoLock(this->_SenderIDLinks, SendID, ModuleID);

				// Unlock vector
				this->_LockLinks.unlock();
			}

			void AddReceiverLink(const Identifier &ReceiverID, const Identifier &ModuleID)
			{
				// Lock vector
				this->_LockLinks.lock();

				// Link this ID
				ThreadModuleManagerMultiMessage::LinkIDsNoLock(this->_ReceiverIDLinks, ReceiverID, ModuleID);

				// Unlock vector
				this->_LockLinks.unlock();
			}

			void Register(const module_shared_ptr_t &NewModule, const id_vector_t SendIDs, const id_vector_t ReceiverIDs)
			{
				// Lock vector
				this->_LockLinks.lock();

				this->Register(NewModule);

				// Link module ID to all SendIDs that are requested
				for(const auto &curID : SendIDs)
				{
					ThreadModuleManagerMultiMessage::LinkIDsNoLock(this->_SenderIDLinks, curID, NewModule->GetID());
				}

				// Link module ID to all ReceiverIDs that are requested
				for(const auto &curID : ReceiverIDs)
				{
					ThreadModuleManagerMultiMessage::LinkIDsNoLock(this->_ReceiverIDLinks, curID, NewModule->GetID());
				}

				// Unlock vector
				this->_LockLinks.unlock();
			}

			void Register(const module_shared_ptr_t &NewModule)
			{
				this->module_manager_t::Register(NewModule);
			}

			module_shared_ptr_t Unregister(const Identifier &ModuleID)
			{
				// Lock vector
				this->_LockLinks.lock();

				// Store return value
				const auto retValue = this->module_manager_t::Unregister(ModuleID);

				// Unlink module
				UnlinkModuleNoLock(ModuleID);

				// Unlock vector
				this->_LockLinks.unlock();

				return retValue;
			}

			template<class FcnMessageStruct>
			void PushMessage(FcnMessageStruct &&MessageStruct)
			{
				this->module_manager_t::PushMessage(std::forward<FcnMessageStruct>(MessageStruct));
			}

			template<class FcnIdentifier1, class FcnIdentifier2, class ...FcnMessageParameters>
			void PushMessage(FcnIdentifier1 &&ReceiveID, FcnIdentifier2 &&SendID, FcnMessageParameters &&...Data)
			{
				// Move message parameters into shared pointer
				//auto tmpPtr = module_extra_message_shared_ptr_t(new module_extra_message_t(std::forward<FcnMessageParameters>(Data)...));

				// Push message
				this->module_manager_t::PushMessage(std::forward<FcnIdentifier1>(ReceiveID), std::forward<FcnIdentifier2>(SendID), std::forward<FcnMessageParameters>(Data)...);
			}

			template<class FcnMessageStruct>
			void PushMessageFront(FcnMessageStruct &&MessageStruct)
			{
				this->module_manager_t::PushMessageFront(std::forward<FcnMessageStruct>(MessageStruct));
			}

			template<class FcnIdentifier1, class FcnIdentifier2, class ...FcnMessageParameters>
			void PushMessageFront(FcnIdentifier1 &&ReceiveID, FcnIdentifier2 &&SendID, FcnMessageParameters &&...Data)
			{
				// Move message parameters into shared pointer
				//auto tmpPtr = module_extra_message_shared_ptr_t(new module_extra_message_t(std::forward<FcnMessageParameters>(Data)...));

				// Push message
				this->module_manager_t::PushMessageFront(std::forward<FcnIdentifier1>(ReceiveID), std::forward<FcnIdentifier2>(SendID), std::forward<FcnMessageParameters>(Data)...);
			}

			void UnlinkModuleNoLock(const Identifier &ModuleIDToUnlink)
			{
				ThreadModuleManagerMultiMessage::UnlinkModuleNoLock(this->_SenderIDLinks, ModuleIDToUnlink);
				ThreadModuleManagerMultiMessage::UnlinkModuleNoLock(this->_ReceiverIDLinks, ModuleIDToUnlink);
			}

		protected:

			/*!
			 * \brief Links between Sended IDs and the receiving module's IDs
			 */
			id_link_vector_t	_SenderIDLinks;

			/*!
			 * \brief Links between Receiver ID and other receiving module's IDs
			 */
			id_link_vector_t	_ReceiverIDLinks;

			/*!
			 * \brief Lock the links vector
			 */
			mutex				_LockLinks;

			static void LinkIDsNoLock(id_link_vector_t &IDLinks, const Identifier &SendID, const Identifier &ModuleID)
			{
				// Link module ID to all SendIDs that are requested
				auto curSendIterator = IDLinks.Find(SendID);

				// Add to vector if not yet present
				if(curSendIterator == IDLinks.end())
				{
					IDLinks.push_back(id_link_t(SendID, id_vector_t{ModuleID}));
				}
				else
				{
					auto curModuleIterator = curSendIterator->Find(ModuleID);
					if(curModuleIterator == curSendIterator->end())
					{
						// Add module ID if not yet present
						curSendIterator->push_back(ModuleID);
					}
				}
			}

		private:

			/*!
			 * \brief Function that handles a message
			 * \param MessageData Data of message
			 */
			static void MessageCallback(msg_struct_t &MessageData, void *ExtraData)
			{
				auto *const pClass = reinterpret_cast<ThreadModuleManagerMultiMessage*>(ExtraData);

				// Lock Links
				pClass->_LockLinks.lock();

				// Send message data to Receiver module
				pClass->module_manager_t::PropagateMessageToModule(MessageData, MessageData.template Get<pClass->_ParamReceiveIDNumber>());

				// Get all ModuleIDs linked to this send ID
				auto receiverIDIterator = pClass->_ReceiverIDLinks.Find(MessageData.template Get<pClass->_ParamReceiveIDNumber>());
				if(receiverIDIterator != pClass->_ReceiverIDLinks.end())
				{
					for(auto &curID : *receiverIDIterator)
					{
						// Send message data to module
						pClass->module_manager_t::PropagateMessageToModule(MessageData, curID);
					}
				}

				// Get all ModuleIDs linked to this send ID
				auto sendIDIterator = pClass->_SenderIDLinks.Find(MessageData.template Get<pClass->_ParamSendIDNumber>());
				if(sendIDIterator != pClass->_SenderIDLinks.end())
				{
					for(auto &curID : *sendIDIterator)
					{
						// Send message data to module
						pClass->module_manager_t::PropagateMessageToModule(MessageData, curID);
					}
				}

				// Unlock list
				pClass->_LockLinks.unlock();
			}

			static void MoveLinksNoLock(id_link_vector_t &CurLinks, id_link_vector_t &OtherLinks)
			{
				// Check if links were added before lock
				if(CurLinks.empty())
				{
					CurLinks = std::move(OtherLinks);
				}
				else
				{
					auto tmpIDLinks = std::move(CurLinks);

					CurLinks = std::move(OtherLinks);

					for(const auto &curIDLink : tmpIDLinks)
					{
						// Check if it exists already
						auto curLinkIterator = tmpIDLinks.Find(curIDLink.SendID);

						if(curLinkIterator != tmpIDLinks.end())
							CurLinks.push_back(curIDLink);
						else
						{
							// If they exist, check if modules already registered
							for(const auto &curModuleID : curIDLink)
							{
								auto curModuleIterator = curLinkIterator->Find(curModuleID);

								// If not, register it
								if(curModuleIterator == curLinkIterator->end())
									curLinkIterator->push_back(curModuleID);
							}
						}
					}
				}
			}

			static void UnlinkModuleNoLock(id_link_vector_t &IDLinks, const Identifier &ModuleIDToUnlink)
			{
				for(auto idIterator = IDLinks.begin(); idIterator != IDLinks.end(); ++idIterator)
				{
					// Continue Finding the moduleID until all elements are erased
					// NOTE: This is inefficient, but easily readable. Maybe change it later
					do
					{
						auto moduleIDIterator = idIterator->Find(ModuleIDToUnlink);

						if(moduleIDIterator != idIterator->end())
						{
							 idIterator->erase(moduleIDIterator);
						}
						else
						{
							break;
						}
					}
					while(1);

					// If empty, erase ID
					if(idIterator->empty())
						idIterator = IDLinks.erase(idIterator) - 1;
				}
			}

			template<class U>
			friend class ::TestingClass;
	};
} // namespace thread_module_manager_multi_message


#endif // THREAD_MODULE_MANAGER_MULTI_MESSAGE_H
