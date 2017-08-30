#include "protocol_module_certificate_manager.h"
#include "error_exception.h"

namespace protocol_module_certificate_manager
{
	using namespace error_exception;

	using namespace string_thread_messages;

	using namespace user_io_messages;

	constexpr decltype(ProtocolModuleCertificateManager::ModuleID) ProtocolModuleCertificateManager::ModuleID;

	ProtocolModuleCertificateManager::ProtocolModuleCertificateManager(ProtocolThreadMemory &ThreadMemory)
		: thread_multi_module_t(identifier_t(ProtocolQueueID, ProtocolModuleCertificateManager::ModuleID, ThreadMemory.ProtocolThreadID)),
		  _Memory(ThreadMemory)
	{
		if(!this->_Memory.HeaderModuleMap.AlreadyRegistered(ProtocolCertificateManagerIDVerificationRequestHeaderName, this->GetID()))
			this->_Memory.HeaderModuleMap.emplace(ProtocolCertificateManagerIDVerificationRequestHeaderName, this->GetID());
	}

	void ProtocolModuleCertificateManager::HandleMessage(msg_struct_t &Message)
	{
		// Check message type
		if(connection_state_change_distribution_t::CheckMessageDataType(Message, this->_Memory.ProtocolThreadID))
		{
			this->HandleInternalStateChangeMessage(connection_state_change_distribution_t::GetMessageData(Message)->UpdatedState);
		}
		else if(received_data_t::CheckMessageDataType(Message, this->_Memory.ProtocolThreadID))
		{
			this->HandlePeerMessage(*received_data_t::GetMessageData(Message));
		}
		else if(user_storage_answer_t::CheckMessageDataType(Message))
		{
			this->HandleStorageAnswer(*user_storage_answer_t::GetMessageData(Message));
		}
		else if(registration_answer_message_t::CheckMessageDataType(Message))
		{
			this->HandleUserRegistrationAnswer(*registration_answer_message_t::GetMessageData(Message));
		}
	}

	void ProtocolModuleCertificateManager::HandleInternalStateChangeMessage(protocol_state_t UpdatedState)
	{
		switch(UpdatedState)
		{
			case PROTOCOL_STARTED:
			{
				// Reset certificate data to starting state
				this->_Memory.PeerConnectionCertificateID = StringUserID();
				this->_Memory.OwnConnectionCertificateID = StringUserID();

				this->_PendingRegistrationID = StringUserID();

				break;
			}

			case PROTOCOL_CLIENT_CREDENTIAL_TRANSFER_STATE:
			{
				// During the transfer state, request an ID to initiate the connection with
				if(this->_Memory.ConnectionSide == CLIENT_SIDE)
				{
					this->RequestID(StringUserID());

					// Continue the credential transfer once an ID was received from the string thread (see HandleAdminCertificateReceive() method)
				}

				break;
			}

			case PROTOCOL_SERVER_CREDENTIALS_TRANSFER_STATE:
			{
				// During the transfer state, request an ID to initiate the connection with
				if(this->_Memory.ConnectionSide == SERVER_SIDE)
				{
					this->RequestID(this->_Memory.PeerConnectionCertificateID);

					// Continue the credential transfer once an ID was received from the string thread (see HandleAdminCertificateReceive() method)
				}

				break;
			}

			// Ignore other state changes
			default:
			{
				break;
			}
		}
	}

	void ProtocolModuleCertificateManager::HandlePeerMessage(received_data_t &PeerData)
	{
		// Check data type and parse data
		const auto *const pVerificationData = PeerData.CheckAndParseData<id_verification_request_t>(ProtocolCertificateManagerIDVerificationRequestHeaderName);
		if(pVerificationData == nullptr)
			throw Exception(ERROR_NUM, "ERROR ProtocolModuleCertificateManager::HandlePeerMessage(): Received id_verification_request_t is ill-formed\n");

		if(this->_Memory.State == PROTOCOL_CLIENT_CREDENTIAL_TRANSFER_STATE)
		{
			if(this->_Memory.ConnectionSide == SERVER_SIDE)
			{
				// If the server received a verification request during PROTOCOL_CLIENT_CREDENTIAL_TRANSFER_STATE state, verify the received ID
				this->RequestIDVerification(pVerificationData->IDToVerify);
			}
		}
		else if(this->_Memory.State == PROTOCOL_SERVER_CREDENTIALS_TRANSFER_STATE)
		{
			if(this->_Memory.ConnectionSide == CLIENT_SIDE)
			{
				// If the client received a verification request during PROTOCOL_SERVER_CREDENTIALS_TRANSFER_STATE state, verify the received ID
				this->RequestIDVerification(pVerificationData->IDToVerify);
			}
		}
	}

	void ProtocolModuleCertificateManager::HandleStorageAnswer(user_storage_answer_t &StorageAnswer)
	{
		this->_CertStorage.CombineWithStorage(std::move(*StorageAnswer.UserStorage.release()));

		const auto requestType = StorageAnswer.Request.RequestData.GetTypeNumber();
		if(requestType == PEER_VERIFICATION)
		{
			const auto &peerID = StorageAnswer.Request.RequestData.Get<PEER_VERIFICATION>()->PeerID;

			// Handle request for authentication certificates
			if(this->_Memory.State == PROTOCOL_CLIENT_CREDENTIAL_TRANSFER_STATE)
			{
				if(this->_Memory.ConnectionSide == SERVER_SIDE)
				{
					// At client verification state, the server should determine whether the received ID is already registered
					if(StorageAnswer.RequestGranted)
					{
						this->_Memory.PeerConnectionCertificateID = peerID;

						// Send certificate to TLS module
						this->SendTLSCredentialsToModule(this->_CertStorage);

						this->RequestStateChange(PROTOCOL_CLIENT_TLS_VERIFICATION_STATE);
					}
					else
					{
						// Request Registration of unknown ID by user
						this->RequestIDRegistration(peerID);
						this->RequestStateChange(PROTOCOL_CLIENT_REGISTRATION_STATE);
					}
				}
			}
			else if(this->_Memory.State == PROTOCOL_CLIENT_REGISTRATION_STATE)
			{
				if(this->_Memory.ConnectionSide == SERVER_SIDE)
				{
					// At client registration state, the server should determine whether the received ID has been registered registered
					if(StorageAnswer.RequestGranted)
					{
						this->_Memory.PeerConnectionCertificateID = peerID;

						// Send certificate to TLS module
						this->SendTLSCredentialsToModule(this->_CertStorage);

						this->RequestStateChange(PROTOCOL_CLIENT_TLS_VERIFICATION_STATE);
					}
					else
					{
						// Registration failed, retry
						this->AbortSequence();
					}
				}
			}
			else if(this->_Memory.State == PROTOCOL_SERVER_CREDENTIALS_TRANSFER_STATE)
			{
				if(this->_Memory.ConnectionSide == CLIENT_SIDE)
				{
					// At server verification state, the client should determine whether the received ID is already registered
					if(StorageAnswer.RequestGranted)
					{
						this->_Memory.PeerConnectionCertificateID = peerID;

						// Send certificate to TLS module
						this->SendTLSCredentialsToModule(this->_CertStorage);

						this->RequestStateChange(PROTOCOL_SERVER_TLS_VERIFICATION_STATE);
					}
					else
					{
						// Abort if server has invalid certificate
						this->AbortSequence();
					}
				}
			}
		}
		else if(requestType == PEER_AUTHENTICATION)
		{
			// Handle answer for admin certificates for authentication to peer
			assert(!this->_CertStorage.GetUserAdmins().empty());

			// TODO: Select best certificate by finding one that has same signer as peer if available
			//			For now, just use first available cert
			this->_Memory.OwnConnectionCertificateID = this->_CertStorage.GetUserAdmins().front().GetMainID();
			this->HandleAdminCertificateSend(this->_Memory.OwnConnectionCertificateID);

			// Send certificate to TLS module
			this->SendTLSCredentialsToModule(this->_CertStorage);
		}
	}

	void ProtocolModuleCertificateManager::HandleUserRegistrationAnswer(registration_answer_message_t &RegistrationAnswer)
	{
		// Check that message content refers to this registration request
		if(RegistrationAnswer.NewUserID != this->_PendingRegistrationID)
			return;

		if(this->_Memory.State == PROTOCOL_CLIENT_REGISTRATION_STATE)
		{
			if(this->_Memory.ConnectionSide == SERVER_SIDE)
			{
				// If the server is at the client registration state, determine whether client was registered
				if(RegistrationAnswer.RequestGranted)
				{
					// Request certificate again
					this->RequestIDVerification(RegistrationAnswer.NewUserID);
				}
				else
					this->AbortSequence();
			}
		}
	}

	void ProtocolModuleCertificateManager::HandleAdminCertificateSend(const StringUserID &OwnCertificateID)
	{
		// Transfer ID of certificate to other side
		if(this->_Memory.State == PROTOCOL_CLIENT_CREDENTIAL_TRANSFER_STATE)
		{
			if(this->_Memory.ConnectionSide == CLIENT_SIDE)
			{
				assert(OwnCertificateID != StringUserID());

				if(OwnCertificateID != StringUserID())
					this->_Memory.SendHandle.SendData<id_verification_request_t>(ProtocolCertificateManagerIDVerificationRequestHeaderName, id_verification_request_t(OwnCertificateID));
				else
				{
					// Abort if no certificate available
					this->AbortSequence();
				}
			}
		}
		else if(this->_Memory.State == PROTOCOL_SERVER_CREDENTIALS_TRANSFER_STATE)
		{
			if(this->_Memory.ConnectionSide == SERVER_SIDE)
			{
				assert(OwnCertificateID != StringUserID());

				if(OwnCertificateID != StringUserID())
					this->_Memory.SendHandle.SendData<id_verification_request_t>(ProtocolCertificateManagerIDVerificationRequestHeaderName, id_verification_request_t(OwnCertificateID));
				else
				{
					// Abort if no certificate available
					this->AbortSequence();
				}
			}
		}
		else
		{
			assert(0/* Send Request issued at invalid time */);
		}
	}

	void ProtocolModuleCertificateManager::RequestStateChange(protocol_state_t NewState) const
	{
		this->_Memory.GlobalQueue->PushMessage(connection_state_change_request_t::CreateMessageFromSender(this->_Memory.ProtocolThreadID, this->GetID(), connection_state_change_request_t{NewState}));
	}

	void ProtocolModuleCertificateManager::RequestID(const StringUserID &PeerID)
	{
		// Request data for authentication to peer. This also checks if peer is registered
		auto tmpMessage = user_storage_request_t::CreateMessageFromSender(this->GetID(), user_storage_request_t(user_storage_ptr_t(new StringUserStorage(std::move(this->_CertStorage))), user_storage_request_t::request_data_t(new peer_authentication_data_t(StringUserID(PeerID)))));
		this->_Memory.GlobalQueue->PushMessage(std::move(tmpMessage));
	}

	void ProtocolModuleCertificateManager::RequestIDVerification(const StringUserID &ReceivedID)
	{
		// Request data for peer verification. This also checks if peer is registered
		auto tmpMessage = user_storage_request_t::CreateMessageFromSender(this->GetID(), user_storage_request_t(user_storage_ptr_t(new StringUserStorage(std::move(this->_CertStorage))), user_storage_request_t::request_data_t(new peer_verification_data_t(StringUserID(ReceivedID)))));
		this->_Memory.GlobalQueue->PushMessage(std::move(tmpMessage));
	}

	void ProtocolModuleCertificateManager::RequestIDRegistration(const StringUserID &IDToRegister)
	{
		auto tmpMessage = registration_request_message_t::CreateMessageFromSender(this->GetID(), registration_request_message_t(IDToRegister));
		this->_Memory.GlobalQueue->PushMessage(std::move(tmpMessage));
	}

	void ProtocolModuleCertificateManager::SendTLSCredentialsToModule(const StringUserStorage &Storage)
	{
		auto tmpMessage = certificate_tls_credentials_answer_t::CreateMessageToReceiver(identifier_t(ProtocolQueueID, ProtocolTLSConnectionModuleID, this->_Memory.ProtocolThreadID), this->_Memory.ProtocolThreadID, certificate_tls_credentials_answer_t(Storage.GenerateCompleteTLSCredentials()));
		this->_Memory.GlobalQueue->PushMessage(std::move(tmpMessage));
	}

	void ProtocolModuleCertificateManager::AbortSequence()
	{
		// Reset ID
		this->_PendingRegistrationID = StringUserID();

		// Increment error counter
		this->_Memory.ErrorCounter++;

		// Reset certificate storage
		this->_CertStorage.Reset();

		// Restart connection after failure
		this->RequestStateChange(PROTOCOL_STARTED);
	}

	ProtocolModuleCertificateManagerInstantiator::ProtocolModuleCertificateManagerInstantiator()
		: ProtocolModuleInstantiator(ProtocolModuleCertificateManager::ModuleID,
									 id_vector_t{}, module_id_vector_t{ProtocolConnectionStateChangeInformerID},
									 id_vector_t{}, module_id_vector_t{})
	{}

	thread_multi_module_shared_ptr_t ProtocolModuleCertificateManagerInstantiator::CreateNewInstanceHandle(instantiation_data_t &Instance) const
	{
		return thread_multi_module_shared_ptr_t(new ProtocolModuleCertificateManager(Instance.Memory));
	}
}

#include "string_user_admin.h"
#include "testing_class_declaration.h"

using namespace error_exception;
using namespace protocol_module_certificate_manager;

using string_user_admin::StringUserAdmin;
using protocol_send_handle::ProtocolModuleSendHandle;

template<>
class TestingClass<protocol_module_certificate_manager::TestProtocolModuleCertificateManager>
{
	public:
		static bool Testing();
};

bool TestingClass<protocol_module_certificate_manager::TestProtocolModuleCertificateManager>::Testing()
{
	try
	{
		StringUserAdmin testAdmin = StringUserAdmin::GenerateNewAdmin("test", time(nullptr)+100000);

		GlobalMessageQueueThread testQueue;
		testQueue.SetThreadState(thread_queued::THREAD_PAUSED);

		ProtocolThreadMemory testMemory{0, nullptr, nullptr, ProtocolModuleSendHandle(nullptr)};
		testMemory.GlobalQueue = &testQueue;

		ProtocolModuleCertificateManager testCertManager(testMemory);

		// void RequestID(const StringUserID &PeerID);
		testCertManager.RequestID(testAdmin.GetMainID());

		auto testMessage = testQueue.Pop();
		if(testMessage.Get<MessageDataNum>().Get<user_storage_request_t>()->RequestData.Get<0>()->PeerID != testAdmin.GetMainID())
			return 0;

		// void RequestStateChange(protocol_state_t NewState) const;
		testCertManager.RequestStateChange(PROTOCOL_STARTED);

		testMessage = testQueue.Pop();
		if(testMessage.Get<MessageDataNum>().Get<user_storage_request_t>()->RequestData.Get<0>()->PeerID != testAdmin.GetMainID())
			return 0;

		// void RequestIDVerification(const StringUserID &ReceivedID);

		// void RequestIDRegistration(const StringUserID &IDToRegister);

		// void HandleMessage(msg_struct_t &Message);

		// void HandleInternalStateChangeMessage(protocol_state_t UpdatedState);

		// void HandlePeerMessage(received_data_t &PeerData);

		// void HandleStorageAnswer(user_storage_answer_t &StorageAnswer);

		// void HandleUserRegistrationAnswer(registration_answer_message_t &RegistrationAnswer);

		// void HandleAdminCertificateSend(const StringUserID &OwnCertificateID);

		// void SendTLSCredentialsToModule(const StringUserStorage &Storage);

		// void AbortSequence();

		return 1;
	}
	catch(Exception &)
	{
		return 0;
	}
}

namespace protocol_module_certificate_manager
{
	class TestProtocolModuleCertificateManager : public TestingClass<protocol_module_certificate_manager::TestProtocolModuleCertificateManager>
	{
		public:
			static bool Testing();
	};

	bool TestProtocolModuleCertificateManager::Testing()
	{
		return TestingClass<protocol_module_certificate_manager::TestProtocolModuleCertificateManager>::Testing();
	}
}
