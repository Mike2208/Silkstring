#include "silkstring_message.h"
#include "user_io_messages.h"

namespace silkstring_message
{
//	template<identifier_t::queue_id_t _QueueID, identifier_t::module_id_t _ModuleID, const message_t &_MessageType, class T>
//	constexpr decltype(_QueueID) message_id_struct_t<_QueueID, _ModuleID, _MessageType, T>::QueueID;

//	template<identifier_t::queue_id_t _QueueID, identifier_t::module_id_t _ModuleID, const message_t &_MessageType, class T>
//	constexpr decltype(_ModuleID) message_id_struct_t<_QueueID, _ModuleID, _MessageType, T>::ModuleID;

//	template<identifier_t::queue_id_t _QueueID, identifier_t::module_id_t _ModuleID, const message_t &_MessageType, class T>
//	constexpr message_t message_id_struct_t<_QueueID, _ModuleID, _MessageType, T>::MessageType;

//	template<identifier_t::queue_id_t _QueueID, identifier_t::module_id_t _ModuleID, identifier_t::thread_id_t _ThreadID, const message_t &_MessageType, class T>
//	constexpr decltype(_ThreadID) message_id_thread_struct_t<_QueueID, _ModuleID, _ThreadID, _MessageType, T>::ThreadID;

//	template<identifier_t::queue_id_t _QueueID, identifier_t::module_id_t _ModuleID, identifier_t::thread_id_t _ThreadID, const message_t &_MessageType, class T>
//	constexpr identifier_t message_id_thread_struct_t<_QueueID, _ModuleID, _ThreadID, _MessageType, T>::ID;

	thread_multi_module_manager_t::thread_multi_module_manager_t(identifier_t::queue_id_t QueueID, identifier_t::thread_id_t ThreadID, thread_queued::thread_state_t ThreadState)
		: thread_multi_module_manager_type(ThreadState), thread_multi_module_t(identifier_t(QueueID, DefaultQueueModuleID, ThreadID))
	{}

	void thread_multi_module_manager_t::HandleMessage(msg_struct_t &Message)
	{
		// If it was sent as a registration regquest, perform the registration
		if(Message.Get<MessageSenderIDNum>() == ModuleRegistrationID)
		{
			if(module_registration_message_t::CheckMessageDataType(Message))
			{
				// Register module
				const auto *const pMessage = module_registration_message_t::GetMessageData(Message);
				this->Register(pMessage->ModuleToRegister, pMessage->SenderLinkIDs, pMessage->ReceiverLinkIDs);
			}
			else if(Message.Get<MessageTypeNum>() == message_t(ModuleUnregistrationMessageType, 0))
			{
				// Unregister module
				const auto *const pMessage = module_unregistration_message_t::GetMessageData(Message);
				auto retVal = this->Unregister(pMessage->ModuleID);
				if(pMessage->ppReturnValue != nullptr)
					*pMessage->ppReturnValue = retVal;
			}
		}
		else	// Else just push message to modules
			this->module_manager_t::PushMessage(Message);
	}

	module_registration_message_t::module_registration_message_t(thread_multi_module_shared_ptr_t _ModuleToRegister, id_vector_t &&_SenderIDLinks, id_vector_t &&_RecevierIDLinks)
		: ModuleToRegister(_ModuleToRegister),
		  SenderLinkIDs(std::move(_SenderIDLinks)),
		  ReceiverLinkIDs(std::move(_RecevierIDLinks))
	{}

	module_unregistration_message_t::module_unregistration_message_t(identifier_t _ModuleID, thread_multi_module_shared_ptr_t *_ppReturnValue)
		: ModuleID(_ModuleID), ppReturnValue(_ppReturnValue)
	{}

	void test()
	{
		thread_multi_module_manager_t testQueue(0);

		auto testMsg = thread_multi_module_message_t(identifier_t(0,0,0), identifier_t(0,0,0), message_t(5), message_ptr((int*)nullptr));

		//testQueue.HandleMessage(testMsg);

	}
}
