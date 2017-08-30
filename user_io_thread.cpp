#include "user_io_thread.h"

namespace user_io_thread
{
	UserIOThread::UserIOThread()
		: user_thread_t(UserIOQueueID)
	{}

//	void UserIOThread::Register(const module_shared_ptr_t &ModulePtr, const id_vector_t *SendIDs)
//	{
//		// Get module ID
//		auto userIOID = ModulePtr->GetID();

//		// Add general ID to linked vector
//		id_vector_t tmpIDs;
//		if(SendIDs != nullptr)
//			tmpIDs = *SendIDs;
//		tmpIDs.push_back(identifier_t(UserIOQueueID, userIOID.ModuleID, UserIOThreadID));

//		// Register module and links
//		this->user_thread_t::Register(ModulePtr, tmpIDs);
//	}

//	void UserIOThread::RequestRegistration(StringUserID NewUserID, string ExtraInfo, identifier_t SendModuleID)
//	{
//		// Create message
//		auto newMsg = registration_request_message_t::CreateMessageFromSender(SendModuleID, registration_request_message_t(NewUserID, ExtraInfo));

//		// Push message to the corresponding modules
//		this->PushMessage(std::move(newMsg));
//	}

//	void UserIOThread::PushMessage(identifier_t ReceiverID, identifier_t SenderID, user_io_messages::message_t MessageType, message_ptr &&Message)
//	{
//		this->user_thread_t::PushMessage(ReceiverID, SenderID, MessageType, std::move(Message));
//	}
}
