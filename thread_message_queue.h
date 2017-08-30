#ifndef THREAD_MESSAGE_QUEUE_H
#define THREAD_MESSAGE_QUEUE_H

/*! \file thread_message_queue.h
 *  \brief Header for ThreadMessageQueue class
 */


#include "message_queue.h"
#include "testing_class_declaration.h"
#include <mutex>

/*!
 *  \brief Namespace for ThreadMessageQueue class
 */
namespace thread_message_queue
{
	using std::mutex;

	using message_queue::MessageQueue;
	/*!
	 * \brief Message Queue with mutex to ensure only one thread can access it
	 */
	template<class ...Args>
	class ThreadMessageQueue : public MessageQueue<Args...>
	{
			using message_queue_t = MessageQueue<Args...>;

		public:
			using msg_struct_t = typename MessageQueue<Args...>::msg_struct_t;

			ThreadMessageQueue() = default;
			~ThreadMessageQueue() = default;

			ThreadMessageQueue(const ThreadMessageQueue &S)
			{
				const_cast<mutex &>(S._QueueLock).lock();
				this->_QueueLock.lock();

				// Check if any new elements were added before lock
				if(this->GetQueueSize() == 0)
				{
					static_cast<message_queue_t&>(*this) = static_cast<message_queue_t&>(S);
				}
				else
				{
					// If yes, prepend existing queue
					auto tmpQueue = std::move(static_cast<message_queue_t&>(*this));

					static_cast<message_queue_t&>(*this) = static_cast<message_queue_t&>(S);

					static_cast<message_queue_t&>(*this)->_Messages.insert(
								static_cast<message_queue_t&>(*this)->_Messages.end(),
								tmpQueue._Messages.begin(),
								tmpQueue._Messages.end());
				}

				this->_QueueLock.unlock();
				const_cast<mutex &>(S._QueueLock).unlock();
			}

			ThreadMessageQueue(ThreadMessageQueue &&S)
			{
				S._QueueLock.lock();
				this->_QueueLock.lock();

				// Check if any new elements were added before lock
				if(this->GetQueueSize() == 0)
				{
					static_cast<message_queue_t&>(*this) = std::move(static_cast<message_queue_t&>(S));
				}
				else
				{
					// If yes, prepend existing queue
					auto tmpQueue = std::move(static_cast<message_queue_t&>(*this));

					static_cast<message_queue_t&>(*this) = std::move(static_cast<message_queue_t&>(S));

					static_cast<message_queue_t&>(*this)->_Messages.insert(
								static_cast<message_queue_t&>(*this)->_Messages.end(),
								tmpQueue._Messages.begin(),
								tmpQueue._Messages.end());
				}

				this->_QueueLock.unlock();
			}

			template<template<class ...VarArgs> class FcnType, class ...FcnArgs>
			void Push(FcnType<FcnArgs...> &&Message)
			{
				static_assert(message_queue::template_convertible<FcnType<FcnArgs...>, msg_struct_t>::value, "ERROR thread_message_queue(): Function Arguments must match Parameter types");

				this->_QueueLock.lock();

				MessageQueue<Args...>::Push(std::forward<FcnType<FcnArgs...>>(Message));

				this->_QueueLock.unlock();
			}

			template<class ...FcnArgs>
			void Push(FcnArgs &&...MessageData)
			{
				this->_QueueLock.lock();

				MessageQueue<Args...>::Push(std::forward<FcnArgs>(MessageData)...);

				this->_QueueLock.unlock();
			}

			template<template<class ...VarArgs> class FcnType, class ...FcnArgs>
			void PushFront(FcnType<FcnArgs...> &&Message)
			{
				static_assert(message_queue::template_convertible<FcnType<FcnArgs...>, msg_struct_t>::value, "ERROR thread_message_queue(): Function Arguments must match Parameter types");

				this->_QueueLock.lock();

				MessageQueue<Args...>::PushFront(std::forward<FcnType<FcnArgs...>>(Message));

				this->_QueueLock.unlock();
			}

			template<class ...FcnArgs>
			void PushFront(FcnArgs &&...MessageData)
			{
				this->_QueueLock.lock();

				MessageQueue<Args...>::PushFront(std::forward<FcnArgs>(MessageData)...);

				this->_QueueLock.unlock();
			}

			msg_struct_t Pop()
			{
				this->_QueueLock.lock();

				auto tmp = MessageQueue<Args...>::Pop();

				this->_QueueLock.unlock();

				return tmp;
			}

		public:

			// Should not be used lightly because some messages may get lost
			ThreadMessageQueue &operator=(const ThreadMessageQueue &S)
			{
				const_cast<mutex &>(S._QueueLock).lock();
				this->_QueueLock.lock();

				// Copy elements
				static_cast<message_queue_t&>(*this) = static_cast<message_queue_t&>(S);

				this->_QueueLock.unlock();
				const_cast<mutex &>(S._QueueLock).unlock();

				return *this;
			}

			// Should not be used lightly because some messages may get lost
			ThreadMessageQueue &operator=(ThreadMessageQueue &&S)
			{
				this->_QueueLock.lock();
				S._QueueLock.lock();

				// Move elements
				static_cast<message_queue_t&>(*this) = std::move(static_cast<message_queue_t&>(S));

				this->_QueueLock.unlock();
				S._QueueLock.unlock();

				return *this;
			}

		private:
			/*!
			 * \brief Access queue
			 */
			mutex _QueueLock;

			template<class U>
			friend class ::TestingClass;
	};
} // namespace thread_message_queue


#endif // THREAD_MESSAGE_QUEUE_H
