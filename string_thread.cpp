#include "string_thread.h"

namespace string_thread
{
	StringThreadModuleRequests::StringThreadModuleRequests(StringThreadMemory &Memory)
		: thread_multi_module_t(identifier_t(StringQueueID, GeneralModuleID, StringThreadID)),
		  _Memory(Memory)
	{}

	void StringThreadModuleRequests::HandleMessage(msg_struct_t &Message)
	{
		if(user_storage_request_t::CheckMessageDataType(Message))
		{
			// Handle request for an ID
			this->HandleUserStorageRequest(Message);
		}
		else if(registration_message_t::CheckMessageDataType(Message))
		{
			// Handle registration request
			const auto *const pData = registration_message_t::GetMessageData(Message);

			this->_Memory.UserStorage.RegisterNewUser(pData->NewID);
		}
		else if(tls_credentials_request_t::CheckMessageDataType(Message))
		{
			this->HandleTLSCredentialsRequest(Message);
		}
		else if(string_user_modification_t::CheckMessageDataType(Message))
		{
			this->HandleModificationRequest(*(string_user_modification_t::GetMessageData(Message)));
		}
	}

	void StringThreadModuleRequests::RegisterModule(GlobalMessageQueueThread &GlobalQueue, const StringThreadModuleRequestsSharedPtr &Module)
	{
		// Link to RegistrationModule module and UserRequestModule to handle their messages
		id_vector_t linkedReceiverIDs{	identifier_t(StringQueueID, ModificationModuleID, StringThreadID),
										identifier_t(StringQueueID, RegistrationModuleID, StringThreadID),
										identifier_t(StringQueueID, UserRequestModuleID, StringThreadID)};

		GlobalQueue.RegisterModule(Module, id_vector_t(), std::move(linkedReceiverIDs));
	}

	void StringThreadModuleRequests::HandleModificationRequest(string_user_modification_t &ModificationRequest)
	{
		assert(ModificationRequest.ModificationData.Get<0>() != nullptr);

		const auto &modType = ModificationRequest.ModificationData.GetTypeNumber();

		if(modType == STRING_USER_MODIFICATION_ADD_USER_ADMIN_CERT)
		{
			// Add new UserAdmin certificate to storage
			auto &modData = *(ModificationRequest.ModificationData.Get<STRING_USER_MODIFICATION_ADD_USER_ADMIN_CERT>());

			// Find user admin
			auto *pStringAdmin = this->_Memory.UserStorage.FindUserAdmin(ModificationRequest.StringID);
			if(pStringAdmin == nullptr)
			{
				// If it doesn't exist yet, create it and add certificate (this will also upgrade any existing StringUser with StringID to StringUserAdmin state)
				this->_Memory.UserStorage.RegisterNewUserAdmin(std::move(modData.NewAdminCert));
			}
			else
			{
				// Add certificate to StringUserAdmin
				pStringAdmin->AddUserCertAdmin(std::move(modData.NewAdminCert));
			}
		}
		else if(modType == STRING_USER_MODIFICATION_ADD_USER_CERT)
		{
			// Add new User certificate to storage
			auto &modData = *(ModificationRequest.ModificationData.Get<STRING_USER_MODIFICATION_ADD_USER_CERT>());

			// Find existing StringUser either in StringUsers or StringUsersAdmin storage
			auto *pStringUser = this->_Memory.UserStorage.FindUserEverywhere(ModificationRequest.StringID);
			if(pStringUser == nullptr)
			{
				// Register new StringUser if it doesn't exist yet
				pStringUser = this->_Memory.UserStorage.RegisterNewUser(ModificationRequest.StringID);
			}

			assert(pStringUser != nullptr);

			pStringUser->AddUserCert(std::move(modData.NewCert));
		}
	}

	void StringThreadModuleRequests::HandleUserStorageRequest(msg_struct_t &Message)
	{
		auto &request = (*user_storage_request_t::GetMessageData(Message));
		switch(request.RequestData.GetTypeNumber())
		{
			case PEER_AUTHENTICATION:
			{
				auto *const pData = request.RequestData.Get<PEER_AUTHENTICATION>();

				// Find ID for authentication
				bool requestGranted = false;

				// TODO: Find best certificate for peer authentication, possibly one where both admin and peer share an issuer
				//			For now, just select first certificate
				assert(!this->_Memory.UserStorage.GetUserAdmins().empty());
				const StringUserAdmin *const pUserAdmin = &(this->_Memory.UserStorage.GetUserAdmins().front());
				if(pUserAdmin != nullptr)
				{
					request.UserStorage->AddNewUserAdmin(StringUserAdmin(*pUserAdmin));
					StringThreadModuleRequests::AddIssuerCertificateChain(*(request.UserStorage), this->_Memory.UserStorage, pUserAdmin);

					requestGranted = true;
				}
				else
					requestGranted = false;

				// Send answer
				this->_Memory.GlobalQueue->PushMessage(user_storage_answer_t::CreateMessageToReceiver(Message.Get<MessageSenderIDNum>(), user_storage_answer_t(std::move(request.UserStorage), std::move(request), requestGranted)));

				break;
			}

			case PEER_VERIFICATION:
			{
				auto *const pData = request.RequestData.Get<PEER_VERIFICATION>();

				// Find ID that should be verified
				bool requestGranted = false;
				const auto *const pUser = this->_Memory.UserStorage.FindUserEverywhere(pData->PeerID);
				if(pUser != nullptr)
				{
					request.UserStorage->AddNewUser(StringUser(*pUser));
					StringThreadModuleRequests::AddIssuerCertificateChain(*(request.UserStorage), this->_Memory.UserStorage, pUser);

					requestGranted = true;
				}
				else
					requestGranted = false;

				// Send answer
				this->_Memory.GlobalQueue->PushMessage(user_storage_answer_t::CreateMessageToReceiver(Message.Get<MessageSenderIDNum>(), user_storage_answer_t(std::move(request.UserStorage), std::move(request), requestGranted)));

				break;
			}

			default:
				assert(0);
		}
	}

	void StringThreadModuleRequests::HandleTLSCredentialsRequest(msg_struct_t &RequestMessage)
	{
		auto *const pData = tls_credentials_request_t::GetMessageData(RequestMessage);

		// Get old credentials if supplied, or create a new one
		TLSCertificateCredentials credentials = (pData == nullptr ? TLSCertificateCredentials() : std::move(*(pData->Credentials.release())));

		if(pData->RequestReason == PEER_AUTHENTICATION)
		{
			// Find suitable certificate to authenticate with peer
			// TODO: More than test
			assert(!this->_Memory.UserStorage.GetUserAdmins().empty());

			const auto &stringAdmin = this->_Memory.UserStorage.GetUserAdmins().front();

			assert(!stringAdmin.GetAdminCertificates().empty());

			// Add all possible chains to credentials
			for(const auto &curAdminCert : stringAdmin.GetAdminCertificates())
			{
				auto curAdminChain = this->_Memory.UserStorage.GetAdminCertAndValidationChain(curAdminCert, &stringAdmin.GetMainID());

				credentials.AddKeyCredentials(std::move(curAdminChain.CertChain), std::move(curAdminChain.Key));
			}

			// Send answer back to sender
			auto tmpMessage = tls_credentials_answer_t::CreateMessageToReceiver(RequestMessage.Get<MessageSenderIDNum>(), tls_credentials_answer_t(std::move(credentials), extra_data_t((int*)nullptr), true));
			this->_Memory.GlobalQueue->PushMessage(std::move(tmpMessage));
		}
		else if(pData->RequestReason == PEER_VERIFICATION)
		{
			assert(pData->RequestData.Get<void>() != nullptr);

			const auto *const pCerts = pData->RequestData.Get<string_user_id_and_certs>();

			user_cert_id_vector_t validatableCerts;
			for(const auto &curUserCertID : pCerts->UserCertIDs)
			{
				auto verificationChain = this->_Memory.UserStorage.GetValidationChain(curUserCertID, &pCerts->StringID);
				if(verificationChain.GetNumCerts() > 0)
				{
					credentials.AddTrustCredentials(std::move(verificationChain));
					validatableCerts.push_back(curUserCertID);
				}
			}

			// Send answer back to sender
			const bool requestGranted = !validatableCerts.empty();
			auto tmpMessage = tls_credentials_answer_t::CreateMessageToReceiver(RequestMessage.Get<MessageSenderIDNum>(), tls_credentials_answer_t(std::move(credentials), extra_data_t(new user_cert_id_vector_t(std::move(validatableCerts))), requestGranted));
			this->_Memory.GlobalQueue->PushMessage(std::move(tmpMessage));
		}
	}

	void StringThreadModuleRequests::AddIssuerCertificateChain(StringUserStorage &NewStorage, const StringUserStorage &OldStorage, const StringUser *User)
	{
		if(User == nullptr)
			return;

		for(auto curUserCertIterator = User->GetUserCertBegin(); curUserCertIterator != User->GetUserCertEnd(); ++curUserCertIterator)
		{
			if(curUserCertIterator->HasIssuer())
			{
				const auto issuerID = curUserCertIterator->GetIssuer();

				// Skip this issuer if it is already in the NewStorage
				if(NewStorage.FindUserCertOwner(issuerID) == nullptr)
					continue;

				const auto *const pStringAdminIssuer = OldStorage.FindAdminUserCertOwner(issuerID);
				// Ceck if admin or user
				if(pStringAdminIssuer == nullptr)
				{
					const auto *const pStringUserIssuer = OldStorage.FindUserCertOwner(issuerID);

					assert(pStringUserIssuer != nullptr);

					NewStorage.AddNewUser(StringUser(*pStringUserIssuer));

					// Add all issuers recursively
					StringThreadModuleRequests::AddIssuerCertificateChain(NewStorage, OldStorage, pStringUserIssuer);
				}
				else
				{
					NewStorage.AddNewUserAdmin(StringUserAdmin(*pStringAdminIssuer));

					// Add all issuers recursively
					StringThreadModuleRequests::AddIssuerCertificateChain(NewStorage, OldStorage, pStringAdminIssuer);
				}
			}
		}
	}

	void StringThreadModuleRequests::AddIssuerCertificateChain(StringUserStorage &NewStorage, const StringUserStorage &OldStorage, const StringUserAdmin *StringAdmin)
	{
		if(StringAdmin == nullptr)
			return;

		for(const auto &curAdminCert : StringAdmin->GetAdminCertificates())
		{
			if(curAdminCert.HasIssuer())
			{
				const auto issuerCertID = curAdminCert.GetIssuer();

				// Skip this issuer if it is already in the NewStorage
				if(NewStorage.FindUserCertOwner(issuerCertID) == nullptr)
					continue;

				const auto *const pStringAdminIssuer = OldStorage.FindAdminUserCertOwner(issuerCertID);
				// Ceck if admin or user
				if(pStringAdminIssuer == nullptr)
				{
					const auto *const pStringUserIssuer = OldStorage.FindUserCertOwner(issuerCertID);

					assert(pStringUserIssuer != nullptr);

					NewStorage.AddNewUser(StringUser(*pStringUserIssuer));

					// Add all issuers recursively
					AddIssuerCertificateChain(NewStorage, OldStorage, pStringUserIssuer);
				}
				else
				{
					NewStorage.AddNewUserAdmin(StringUserAdmin(*pStringAdminIssuer));

					// Add all issuers recursively
					AddIssuerCertificateChain(NewStorage, OldStorage, pStringAdminIssuer);
				}
			}
		}

		// Do the same for all
		StringThreadModuleRequests::AddIssuerCertificateChain(NewStorage, OldStorage, static_cast<const StringUser *>(StringAdmin));
	}

	StringThread::StringThread(GlobalMessageQueueThread &_GlobalQueue)
		: thread_multi_module_manager_t(StringQueueID),
		  _Memory{&_GlobalQueue},
		  _GeneralModule(this->_Memory),
		  _GeneralModulePtr(&this->_GeneralModule, [](const void*){})
	{}

	void StringThread::RegisterThread(GlobalMessageQueueThread &GlobalQueue, const StringThreadSharedPtr &Queue)
	{
		GlobalQueue.RegisterQueue(Queue);

		StringThreadModuleRequests::RegisterModule(GlobalQueue, Queue->_GeneralModulePtr);
	}

	StringThreadMemory &StringThread::GetMemory()
	{
		return this->_Memory;
	}

	const StringThreadMemory &StringThread::GetMemory() const
	{
		return this->_Memory;
	}
}
