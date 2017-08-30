#ifndef GLOBAL_MESSAGE_QUEUE_THREAD_H
#define GLOBAL_MESSAGE_QUEUE_THREAD_H

/*! \file global_message_queue_thread.h
 *  \brief Header for GlobalMessageQueueThread class
 */


#include "silkstring_message.h"
#include "thread_queued.h"

#include "testing_class_declaration.h"
#include "debug_flag.h"

#ifdef DEBUG
#include <typeinfo>
#include <iostream>
#endif

/*!
 *  \brief Namespace for GlobalMessageQueueThread class
 */
namespace global_message_queue_thread
{
	using std::function;
	using std::bind;
	using std::mutex;

	using thread_queued::ThreadQueued;
	using thread_queued::thread_state_t;
	using thread_queued::THREAD_PAUSED;
	using thread_queued::THREAD_RUNNING;
	using thread_queued::THREAD_STOPPED;

	using silkstring_message::message_ptr;
	using silkstring_message::message_t;
	using silkstring_message::identifier_t;
	using silkstring_message::thread_multi_module_manager_t;
	using silkstring_message::thread_multi_module_manager_shared_ptr_t;
	using silkstring_message::thread_multi_module_message_t;

	using silkstring_message::thread_multi_module_t;
	using silkstring_message::thread_multi_module_shared_ptr_t;

	using silkstring_message::id_vector_t;

	using silkstring_message::MessageReceiverIDNum;
	using silkstring_message::MessageSenderIDNum;
	using silkstring_message::MessageTypeNum;
	using silkstring_message::MessageDataNum;
	using silkstring_message::StartQueueID;
	using silkstring_message::DefaultThreadID;

	using silkstring_message::ModuleRegistrationID;
	using silkstring_message::ModuleRegistrationMessageType;
	using silkstring_message::module_registration_message_t;
	using silkstring_message::ModuleUnregistrationMessageType;
	using silkstring_message::module_unregistration_message_t;

	using vector_t::vector_t;

	class TestGlobalMessageQueueThread;

	/*!
	 * \brief The GlobalMessageQueueThread class
	 */
	class GlobalMessageQueueThread : protected thread_multi_module_manager_t
	{
			template<class U>
			friend class ::TestingClass;

		public:
			using thread_multi_module_manager_t = silkstring_message::thread_multi_module_manager_t;

			static constexpr identifier_t::queue_id_t QueueID = StartQueueID + 0;

			/*!
			 * 	\brief Constructor
			 */
			GlobalMessageQueueThread();

			GlobalMessageQueueThread(const GlobalMessageQueueThread &S) = delete;
			GlobalMessageQueueThread &operator=(const GlobalMessageQueueThread &S) = delete;
			GlobalMessageQueueThread (GlobalMessageQueueThread &&S) = delete;
			GlobalMessageQueueThread &operator=(GlobalMessageQueueThread &&S) = delete;

			~GlobalMessageQueueThread();

			/*!
			 * \brief Register a new message queue
			 */
			void RegisterQueue(const thread_multi_module_manager_shared_ptr_t &NewMessageQueue);

			/*!
			 * \brief Unregister an existing message queue
			 * \param MessageQueueID ID of queue to unregister
			 * \return Returns pointer to unregisterd message queue
			 */
			//thread_multi_module_shared_ptr_t UnregisterQueue(identifier_t::queue_id_t MessageQueueID, identifier_t::thread_id_t MessageQueueThreadID = DefaultThreadID);
			thread_multi_module_shared_ptr_t UnregisterQueue(const thread_multi_module_manager_shared_ptr_t &MessageQueueToUnregister);

			/*!
			 * \brief Register a new Module. The ModuleID is checked to see which queue it belongs to
			 * \param NewModule Module to register
			 * \param Links Messages to other modules that should be sent to this module as well
			 */
			void RegisterModule(const thread_multi_module_shared_ptr_t &NewModule, const id_vector_t &SenderLinks, const id_vector_t &ReceiverLinks);

			/*!
			 * \brief Register a new Module. The ModuleID is checked to see which queue it belongs to
			 * \param NewModule Module to register
			 * \param Links Messages to other modules that should be sent to this module as well
			 */
			void RegisterModule(const thread_multi_module_shared_ptr_t &NewModule, id_vector_t &&SendLinks, id_vector_t &&ReceiverLinks);

			/*!
			 * \brief Unregister a Module
			 * \param ModuleID ID of module to unregister
			 * \return Returns pointer to unregistered module
			 */
			thread_multi_module_shared_ptr_t UnregisterModule(identifier_t ModuleID);

			/*!
			 * \brief Push Message to queues
			 * \param Message Message to push
			 */
			void PushMessage(thread_multi_module_message_t Message);

			/*!
			 * \brief Push Message to front of queues
			 * \param Message Message to push
			 */
			void PushMessageFront(thread_multi_module_message_t Message);

			/*!
			 * \brief Change thread state
			 */
			void SetThreadState(thread_state_t ThreadState);

		protected:

			msg_struct_t Pop();

		private:

			/*!
			 * \brief Propagates the Message to the corresponding threads
			 * \param Message Message to propagate
			 */
			static void PropagateMessageToQueues(msg_struct_t &Message, void *ExtraData);

			/*!
			 * \brief Finds the Queue with QueueID. Doesn't lock _ModuleListLock
			 * \param QueueID ID of queue to find
			 * \return Returns iterator to queue. End() if not found
			 */
			module_list_t::iterator FindQueueNoLock(identifier_t::queue_id_t QueueID, identifier_t::thread_id_t MessageQueueThreadID);

			void AddLinkedQueues(vector_t<module_shared_ptr_t> &Queues, identifier_t ID, id_link_vector_t &LinkedIDs);

			/*!
			 * \brief Finds the link associated with a moduleQueueLinkNoLock
			 * \param QueueID
			 * \return
			 */
			//id_link_vector_t::iterator FindQueueLinkNoLock(identifier_t::queue_id_t QueueID);

			friend class TestGlobalMessageQueueThread;
	};
} // namespace global_message_queue_thread


#endif // GLOBAL_MESSAGE_QUEUE_THREAD_H
