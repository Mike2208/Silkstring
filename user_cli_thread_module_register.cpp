#include "user_cli_thread_module_register.h"

namespace user_cli_thread_module_register
{
	using silkstring_message::MessageSenderIDNum;
	using silkstring_message::MessageTypeNum;
	using silkstring_message::MessageDataNum;
	using silkstring_message::message_t;

	registration_request_data_t::registration_request_data_t(const registration_request_message_t &Message, identifier_t ReturnModule)
		: registration_request_message_t(Message), ReturnModules{ReturnModule}
	{}

	UserCLIThreadModuleRegister::UserCLIThreadModuleRegister()
		: thread_multi_module_t(identifier_t(UserIOQueueID, UserIORegistrationModuleID, UserCLIThreadID))
	{}

	const UserCLIThreadModuleRegister::registration_request_vector_t &UserCLIThreadModuleRegister::GetRegistrationRequests() const
	{
		return this->_RegistrationRequests;
	}

	void UserCLIThreadModuleRegister::HandleRegistrationRequest(UserCLIThreadModuleRegister *Module, msg_struct_t &Message)
	{
		// Get message data and sender ID
		const auto &messageData = *(Message.Get<MessageDataNum>().Get<registration_request_message_t>());
		const auto &messageModuleID = Message.Get<MessageSenderIDNum>();

		// Check if request exists
		for(auto &curRequestData : Module->_RegistrationRequests)
		{
			if(curRequestData.NewUserID == messageData.NewUserID)
			{
				bool alreadyRecorded = false;
				for(const auto &curModuleID : curRequestData.ReturnModules)
				{
					if(curModuleID == messageModuleID)
					{
						alreadyRecorded = true;
						break;
					}
				}

				if(!alreadyRecorded)
					curRequestData.ReturnModules.push_back(messageModuleID);

				// Request handled, stop
				return;
			}
		}

		// If it doesn't exist yet, add it
		Module->_RegistrationRequests.push_back(registration_request_data_t(messageData, messageModuleID));
	}

	void UserCLIThreadModuleRegister::HandleRegistrationAnswer(UserCLIThreadModuleRegister *, msg_struct_t &Message)
	{
		// Get message data
		const auto &messageData = *(Message.Get<MessageDataNum>().Get<registration_answer_message_t>());

		// If the request was granted, add this ID to the list
		if(messageData.RequestGranted)
		{

		}
	}

	void UserCLIThreadModuleRegister::HandleMessage(msg_struct_t &Parameters)
	{
		if(registration_answer_message_t::CheckMessageDataType(Parameters))
			this->HandleRegistrationAnswer(this, Parameters);
		else if(registration_request_message_t::CheckMessageDataType(Parameters))
			this->HandleRegistrationRequest(this, Parameters);
		//thread_multi_module_fcn_t<UserCLIThreadModuleRegister, HandleRegistrationRequest, HandleRegistrationAnswer>::HandleMessages(this, Parameters);
	}

	void test()
	{
		UserCLIThreadModuleRegister testRegister;

		UserCLIThreadModuleRegister::msg_struct_t testParam(identifier_t(0,0,0), identifier_t(0,0,0), message_t(0, 0), message_ptr(new int(0)));
	}
}
