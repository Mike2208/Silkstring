#ifndef THREAD_QUEUED_H
#define THREAD_QUEUED_H

/*! \file thread_queued.h
 *  \brief Header for ThreadQueued class
 */


#include "thread_message_queue.h"
#include "thread_function.h"
#include "debug_flag.h"
#include "testing_class_declaration.h"

#include <atomic>
#include <thread>
#include <chrono>
#include <assert.h>

#ifdef DEBUG
#include <iostream>
#endif

/*!
 *  \brief Namespace for ThreadQueued class
 */
namespace thread_queued
{
	using thread_message_queue::ThreadMessageQueue;
	using message_queue::message_struct_t;

	using thread_function::ThreadFunction;

	using std::atomic;

	using std::this_thread::sleep_for;
	using std::chrono::duration;
	using std::ratio;

	extern void SleepForMs(unsigned int MicroSeconds);

	enum thread_state_t
	{
		THREAD_RUNNING,
		THREAD_PAUSED,
		THREAD_STOPPED
	};

	/*!
	 * \brief The ThreadQueued class
	 * Warning: Can't use references as MessageParameters. Use pointers instead (See ThreadFunction for explanation)
	 */
	template<class... MessageParameters>
	class ThreadQueued : protected ThreadMessageQueue<MessageParameters...>
	{
		public:
			using msg_struct_t = message_struct_t<MessageParameters...>;

		protected:
			using message_fcn_t = void(msg_struct_t &, void *);

		private:
			using message_queue_t = ThreadMessageQueue<MessageParameters...>;

			using sleep_type = unsigned int;
			using sleep_t = atomic<sleep_type>;

			using state_t = atomic<thread_state_t>;
			using thread_fcn_t = void(ThreadQueued<MessageParameters...> *const);
			using thread_t = ThreadFunction<thread_fcn_t, void, ThreadQueued<MessageParameters...> *const>;

		public:

			/*!
			 *	\brief Constructor
			 */
			ThreadQueued(message_fcn_t *const MessageCallback, void *ExtraData = nullptr, const thread_state_t ThreadState = THREAD_RUNNING, const sleep_type SleepMicroS = 1)
				: message_queue_t(),
				  _MessageFcn(MessageCallback),
				  _ExtraData(ExtraData),
				  _AcceptMessages(true),
				  _State(ThreadState),
				  _SleepMicroS(SleepMicroS),
				  _Thread(ThreadMessageFunction, this)
			{}

			ThreadQueued(const ThreadQueued &S) = delete;

			ThreadQueued(ThreadQueued &&S)
				: message_queue_t(),
				  _MessageFcn(nullptr),
				  _ExtraData(std::move(S._ExtraData)),
				  _AcceptMessages(static_cast<bool>(S._AcceptMessages)),
				  _State(THREAD_PAUSED),
				  _SleepMicroS(static_cast<sleep_type>(S._SleepMicroS)),
				  _Thread(ThreadMessageFunction, this)
			{
				auto tmpState = S.GetThreadState();

				// Pause this thread
				S.SetThreadState(THREAD_PAUSED);

				static_cast<message_queue_t&>(*this) = std::move(static_cast<message_queue_t&>(S));
				this->_MessageFcn = std::move(S._MessageFcn);

				// Set same state as S
				this->SetThreadState(tmpState);

				// Stop thread S (done in destructor)
				//S.SetThreadState(THREAD_STOPPED);
			}

			ThreadQueued &operator=(const ThreadQueued &S) = delete;

			~ThreadQueued()
			{
				// Empty queue if thread still running
				if(this->_State == THREAD_RUNNING)
				{
					// Accept no more messages
					this->SetMessageAcceptance(false);

					// Wait for all messages to be processes
					while(!this->IsQueueEmpty())
					{}
				}

				// Stop thread
				this->StopThread();
			}

			template<class ...FcnArgs>
			void PushMessage(FcnArgs &&...MessageData)
			{
				static_assert(sizeof...(FcnArgs) == sizeof...(MessageParameters),
							  "ERROR ThreadedQueue::PushMessage(): Number of arguments must match number of parameters");

				static_assert(message_queue::template_convertible<typename ThreadQueued<FcnArgs...>::msg_struct_t, msg_struct_t>::value,
							  "ERROR ThreadedQueue::PushMessage(): Function Arguments must match template parameters");

				assert(this->_AcceptMessages == true);

				if(this->_AcceptMessages == true)
					static_cast<message_queue_t&>(*this).Push(std::forward<FcnArgs>(MessageData)...);
			}

			template<class FcnArg>
			void PushMessage(FcnArg &&Message)
			{
				static_assert(message_queue::template_convertible<FcnArg, msg_struct_t>::value,
							  "ERROR ThreadedQueue::PushMessage(): Function Argument must match template parameter");

				assert(this->_AcceptMessages == true);

				if(this->_AcceptMessages == true)
					static_cast<message_queue_t&>(*this).Push(std::forward<FcnArg>(Message));
			}

			template<class ...FcnArgs>
			void PushMessageFront(FcnArgs &&...MessageData)
			{
				static_assert(sizeof...(FcnArgs) == sizeof...(MessageParameters),
							  "ERROR ThreadedQueue::PushMessage(): Number of arguments must match number of parameters");

				static_assert(message_queue::template_convertible<typename ThreadQueued<FcnArgs...>::msg_struct_t, msg_struct_t>::value,
							  "ERROR ThreadedQueue::PushMessage(): Function Arguments must match template parameters");

				assert(this->_AcceptMessages == true);

				if(this->_AcceptMessages == true)
					static_cast<message_queue_t&>(*this).PushFront(std::forward<FcnArgs>(MessageData)...);
			}

			template<class FcnArg>
			void PushMessageFront(FcnArg &&Message)
			{
				static_assert(message_queue::template_convertible<FcnArg, msg_struct_t>::value,
							  "ERROR ThreadedQueue::PushMessage(): Function Argument must match template parameter");

				assert(this->_AcceptMessages == true);

				if(this->_AcceptMessages == true)
					static_cast<message_queue_t&>(*this).PushFront(std::forward<FcnArg>(Message));
			}

			void SetSleepTime(const sleep_type MicroS)
			{
				this->_SleepMicroS = MicroS;
			}

			void SetThreadState(thread_state_t NewState)
			{
				this->_State = NewState;
			}

			thread_state_t GetThreadState() const
			{
				return this->_State;
			}

			void RestartThread()
			{
				// Stop if still running
				this->_State = THREAD_STOPPED;

				// Wait for it to complete
				this->Wait();

				// Set running state for next start
				this->_State = THREAD_RUNNING;

				// Start a new thread
				this->_Thread = thread_t(ThreadMessageFunction, this);
			}

			void StopThread()
			{
				this->SetThreadState(THREAD_STOPPED);
			}

			void Wait()
			{
				this->_Thread.Wait();
			}

			void SetMessageFunction(message_fcn_t *NewFunction)
			{
				this->_MessageFcn = NewFunction;
			}

			void SetExtraData(void *ExtraData)
			{
				this->_ExtraData = ExtraData;
			}

			void SetMessageAcceptance(bool AllowNewMessages)
			{
				this->_AcceptMessages = AllowNewMessages;
			}

			bool GetMessageAcceptance() const
			{
				return this->_AcceptMessages;
			}

			bool IsQueueEmpty() const
			{
				if(this->GetQueueSize() > 0)
					return false;

				return true;
			}

		public:

			ThreadQueued &operator=(ThreadQueued &&S)
			{
				auto tmpState = S.GetThreadState();

				// Pause this thread
				S.SetThreadState(THREAD_PAUSED);
				this->SetThreadState(THREAD_PAUSED);

				static_cast<message_queue_t&>(*this) = std::move(static_cast<message_queue_t&>(S));
				this->_MessageFcn = std::move(S._MessageFcn);

				this->_ExtraData = std::move(S._ExtraData);

				this->_AcceptMessages = static_cast<bool>(S._AcceptMessages);

				// Set same state as S
				this->SetThreadState(tmpState);

				// Stop thread S (done in destructor)
				//S.SetThreadState(THREAD_STOPPED);

				return *this;
			}

		private:

			message_fcn_t		*_MessageFcn;

			void					*_ExtraData = nullptr;

			/*!
			 * \brief Allow new messages to be accepted into queue
			 */
			atomic<bool>			_AcceptMessages = true;

			state_t					_State = THREAD_RUNNING;

			sleep_t					_SleepMicroS = 1;

			thread_t				_Thread;

			static void ThreadMessageFunction(ThreadQueued *const ThreadData)
			{
#ifdef DEBUG
				std::cout << "Starting queue thread function\n";
#endif

				// Check if thread should be stopped
				while(ThreadData->_State != THREAD_STOPPED)
				{
					// Check if a message is in queue and thread is not paused
					if(ThreadData->_State != THREAD_PAUSED && static_cast<message_queue_t&>(*ThreadData).GetQueueSize() > 0)
					{
						auto tmpMessage = static_cast<message_queue_t&>(*ThreadData).Pop();
						ThreadData->_MessageFcn(tmpMessage, ThreadData->_ExtraData);
					}
					else
					{
						// If thread is paused or no message is in queue, sleep for the specified time
						SleepForMs(ThreadData->_SleepMicroS);
					}
				}

#ifdef DEBUG
				std::cout << "Ending queue thread function\n";
#endif
			}

			template<class U>
			friend class ::TestingClass;
	};
} // namespace thread_queued


#endif // THREAD_QUEUED_H
