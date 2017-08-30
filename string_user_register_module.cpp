#include "string_thread_module_user_register.h"

namespace string_thread_module_user_register
{
	StringThreadModuleUserRegister::StringThreadModuleUserRegister(const StringThreadSharedPtr &Manager)
		: thread_multi_module_t(registration_message_t::ID),
		  _Memory(&(Manager->GetMemory()))
	{}

	void StringThreadModuleUserRegister::RegisterWithStringUserManager(const shared_ptr_t &RegisterModule,  const StringThreadSharedPtr &Manager)
	{
		RegisterModule->_Memory = &(Manager->GetMemory());

		Manager->Register(RegisterModule);
	}

	void StringThreadModuleUserRegister::HandleMessage(thread_multi_module_message_t &Message)
	{
		// Check message type
		if(registration_message_t::CheckMessageDataType(Message))
		{
			const auto &messageData = *registration_message_t::GetMessageData(Message);

			// Register new id
			this->_Memory->UserStorage.RegisterNewUser(messageData.NewID);
		}
	}

	void test()
	{
		StringThreadSharedPtr *pTest = nullptr;
		StringThreadModuleUserRegister testModule(*pTest);

		StringThreadModuleUserRegister::RegisterWithStringUserManager(shared_ptr<StringThreadModuleUserRegister>(&testModule), *pTest);
	}
}
