#include "protocol_module_connection_state.h"
#include "error_exception.h"

namespace protocol_module_connection_state
{
	using namespace error_exception;

	constexpr decltype(ProtocolModuleConnectionState::HeaderName) ProtocolModuleConnectionState::HeaderName;

	ProtocolModuleConnectionState::ProtocolModuleConnectionState(ProtocolThreadMemory &Memory)
		: thread_multi_module_t(identifier_t(ProtocolQueueID, ProtocolConnectionStateModuleID, Memory.ProtocolThreadID)), _Memory(Memory)
	{
		if(!this->_Memory.HeaderModuleMap.AlreadyRegistered(this->HeaderName, this->GetID()))
			this->_Memory.HeaderModuleMap.emplace(this->HeaderName, this->GetID());
	}

	void ProtocolModuleConnectionState::HandleMessage(msg_struct_t &Message)
	{
		const auto &senderID = Message.Get<MessageSenderIDNum>();
		const auto &messageType = Message.Get<MessageTypeNum>();

		if(senderID.ModuleID == ProtocolConnectionModuleID && messageType == message_t(received_data_t::MessageType, 1))
		{
			const auto &dataHeader = received_data_t::GetMessageData(Message)->DataType;
			auto &pDataVector = received_data_t::GetMessageData(Message)->ReceivedData;

			if(dataHeader != ProtocolConnectionStateChangeRequestHeader)
				throw Exception(ERROR_NUM, "ERROR ProtocolModuleConnectionState::HandleMessage(): Message received by peer has incorrect header\n");

			pDataVector.CurPos = pDataVector.begin();
			const auto *const pReceivedState = pDataVector.ParseVector<connection_state_change_request_t>();
			if(pReceivedState == nullptr)
				throw Exception(ERROR_NUM, "ERROR ProtocolModuleConnectionState::HandleMessage(): Message received by peer is ill-formed\n");

#ifdef DEBUG
			std::cout << "Received state change from peer to: " << pReceivedState->NewState << "\n";
#endif

			this->OnPeerStateChange(pReceivedState->NewState);
		}
		else if(connection_state_change_request_t::CheckMessageDataType(Message, this->_Memory.ProtocolThreadID))
		{
#ifdef DEBUG
			std::cout << "Received internal state change request to: " << connection_state_change_request_t::GetMessageData(Message)->NewState << "\n";
#endif
			this->OnInternalStateChangeRequest(senderID, connection_state_change_request_t::GetMessageData(Message)->NewState);
		}
	}

	void ProtocolModuleConnectionState::OnPeerStateChange(protocol_state_t NewRequestedState)
	{
		// Store new peer state
		this->_Memory.PeerState = NewRequestedState;

		// Handle state change request
		switch(NewRequestedState)
		{
			case PROTOCOL_STARTED:
			{
				this->OnRestartRequest();

				break;
			}

			case PROTOCOL_CLIENT_CREDENTIAL_TRANSFER_STATE:
			{
				// Change state
				if(this->_Memory.ConnectionSide == CLIENT_SIDE)
				{
					if(this->_Memory.State == PROTOCOL_STARTED)
					{
						this->_Memory.State = PROTOCOL_CLIENT_CREDENTIAL_TRANSFER_STATE;
						this->SendInternalStateChangeMessage();
					}
					else if(this->_Memory.State != PROTOCOL_CLIENT_CREDENTIAL_TRANSFER_STATE)
						throw Exception(ERROR_NUM, "ERROR ProtocolModuleConnectionState::OnPeerStateChangeRequest(): Illegal state change request to PROTOCOL_CLIENT_CREDENTIAL_TRANSFER_STATE received from server\n");
				}
				else if(this->_Memory.ConnectionSide == SERVER_SIDE)
				{
					if(this->_Memory.State == PROTOCOL_STARTED)
					{
						this->_Memory.State = PROTOCOL_CLIENT_CREDENTIAL_TRANSFER_STATE;
						this->SendInternalStateChangeMessage();
					}
					else if(this->_Memory.State != PROTOCOL_CLIENT_CREDENTIAL_TRANSFER_STATE)
						throw Exception(ERROR_NUM, "ERROR ProtocolModuleConnectionState::OnPeerStateChangeRequest(): Illegal state change request to PROTOCOL_CLIENT_CREDENTIAL_TRANSFER_STATE received from client\n");
				}

				break;
			}

			case PROTOCOL_CLIENT_REGISTRATION_STATE:
			{
				// Check whether peer can change state
				if(this->_Memory.ConnectionSide == CLIENT_SIDE)
				{
					if(this->_Memory.State != PROTOCOL_CLIENT_CREDENTIAL_TRANSFER_STATE)
						throw Exception(ERROR_NUM, "ERROR ProtocolModuleConnectionState::OnPeerStateChangeRequest(): Illegal state change request to PROTOCOL_CLIENT_CREDENTIAL_TRANSFER_STATE received from server\n");

					// If this is client, then this state change means the server is currently requesting the user to register this ID
					// Just wait until server registers this ID
					this->_Memory.State = PROTOCOL_CLIENT_REGISTRATION_STATE;
					this->SendInternalStateChangeMessage();
				}
				else if(this->_Memory.ConnectionSide == SERVER_SIDE)
				{
					// Check whether this was confirmation
					if(this->_Memory.State != PROTOCOL_CLIENT_REGISTRATION_STATE)
						throw Exception(ERROR_NUM, "ERROR ProtocolModuleConnectionState::OnPeerStateChangeRequest(): Illegal state change request to PROTOCOL_CLIENT_CREDENTIAL_TRANSFER_STATE received from client\n");
				}

				break;
			}

			case PROTOCOL_CLIENT_TLS_VERIFICATION_STATE:
			{
				// Check whether peer can change state
				if(this->_Memory.ConnectionSide == CLIENT_SIDE)
				{
					if(this->_Memory.State != PROTOCOL_CLIENT_CREDENTIAL_TRANSFER_STATE &&
							this->_Memory.State != PROTOCOL_CLIENT_REGISTRATION_STATE)
						throw Exception(ERROR_NUM, "ERROR ProtocolModuleConnectionState::OnPeerStateChangeRequest(): Illegal state change request to PROTOCOL_CLIENT_TLS_VERIFICATION_STATE received from server\n");

					// If this is client, then this state change means the server is currently requesting the user to register this ID
					// Just wait until server registers this ID
					this->_Memory.State = PROTOCOL_CLIENT_TLS_VERIFICATION_STATE;
					this->SendInternalStateChangeMessage();
				}
				else if(this->_Memory.ConnectionSide == SERVER_SIDE)
				{
					// Check whether this was confirmation
					if(this->_Memory.State != PROTOCOL_CLIENT_TLS_VERIFICATION_STATE)
						throw Exception(ERROR_NUM, "ERROR ProtocolModuleConnectionState::OnPeerStateChangeRequest(): Illegal state change request to PROTOCOL_CLIENT_TLS_VERIFICATION_STATE received from client\n");
				}

				break;
			}

			case PROTOCOL_SERVER_CREDENTIALS_TRANSFER_STATE:
			{
				// Check whether peer can change state
				if(this->_Memory.ConnectionSide == CLIENT_SIDE)
				{
					if(this->_Memory.State != PROTOCOL_CLIENT_TLS_VERIFICATION_STATE)
						throw Exception(ERROR_NUM, "ERROR ProtocolModuleConnectionState::OnPeerStateChangeRequest(): Illegal state change request to PROTOCOL_SERVER_CREDENTIALS_TRANSFER_STATE received from server\n");

					this->_Memory.State = PROTOCOL_SERVER_CREDENTIALS_TRANSFER_STATE;
					this->SendInternalStateChangeMessage();
				}
				else if(this->_Memory.ConnectionSide == SERVER_SIDE)
				{
					// Check whether this was confirmation
					if(this->_Memory.State != PROTOCOL_SERVER_CREDENTIALS_TRANSFER_STATE)
						throw Exception(ERROR_NUM, "ERROR ProtocolModuleConnectionState::OnPeerStateChangeRequest(): Illegal state change request to PROTOCOL_SERVER_CREDENTIALS_TRANSFER_STATE received from client\n");
				}

				break;
			}

			case PROTOCOL_SERVER_TLS_VERIFICATION_STATE:
			{
				// Check whether peer can change state
				if(this->_Memory.ConnectionSide == CLIENT_SIDE)
				{
					// Check whether this was confirmation
					if(this->_Memory.State != PROTOCOL_SERVER_TLS_VERIFICATION_STATE)
						throw Exception(ERROR_NUM, "ERROR ProtocolModuleConnectionState::OnPeerStateChangeRequest(): Illegal state change request to PROTOCOL_SERVER_TLS_VERIFICATION_STATE received from server\n");
				}
				else if(this->_Memory.ConnectionSide == SERVER_SIDE)
				{
					if(this->_Memory.State != PROTOCOL_SERVER_CREDENTIALS_TRANSFER_STATE)
						throw Exception(ERROR_NUM, "ERROR ProtocolModuleConnectionState::OnPeerStateChangeRequest(): Illegal state change request to PROTOCOL_SERVER_TLS_VERIFICATION_STATE received from client\n");

					// If this is server, then this state change means the client has accepted the ID
					this->_Memory.State = PROTOCOL_SERVER_TLS_VERIFICATION_STATE;
					this->SendInternalStateChangeMessage();
				}

				break;
			}

			case PROTOCOL_SECURE_CONNECTION_STATE:
			{
				// Check whether peer can change state (this state can only be a confirmation of the state. Whether the connection is secure must be established internally)
				if(this->_Memory.ConnectionSide == CLIENT_SIDE)
				{
					// Check whether this was confirmation
					if(this->_Memory.State != PROTOCOL_SECURE_CONNECTION_STATE)
						throw Exception(ERROR_NUM, "ERROR ProtocolModuleConnectionState::OnPeerStateChangeRequest(): Illegal state change request to PROTOCOL_SECURE_CONNECTION_STATE received from server\n");
				}
				else if(this->_Memory.ConnectionSide == SERVER_SIDE)
				{
					// Check whether this was confirmation
					if(this->_Memory.State != PROTOCOL_SECURE_CONNECTION_STATE)
						throw Exception(ERROR_NUM, "ERROR ProtocolModuleConnectionState::OnPeerStateChangeRequest(): Illegal state change request to PROTOCOL_SECURE_CONNECTION_STATE received from client\n");
				}

				break;
			}

			case PROTOCOL_END_STATE:
			{
				// Receiving this means the peer will end the connection. Just accept and handle it
				this->_Memory.State = PROTOCOL_END_STATE;
				this->SendInternalStateChangeMessage();

				break;
			}

			default:
			{
				throw Exception(ERROR_NUM, "ERROR ProtocolModuleConnectionState::OnPeerStateChangeRequest(): Invalid peer state received\n");
				break;
			}
		}
	}

	void ProtocolModuleConnectionState::OnInternalStateChangeRequest(identifier_t SenderID, protocol_state_t NewRequestedState)
	{
		// Handle state change request
		switch(NewRequestedState)
		{
			case PROTOCOL_STARTED:
			{
				this->OnRestartRequest();

				break;
			}

			case PROTOCOL_CLIENT_CREDENTIAL_TRANSFER_STATE:
			{
				assert(this->_Memory.State == PROTOCOL_STARTED && SenderID == this->GetID());

				if(this->_Memory.State == PROTOCOL_STARTED && SenderID == this->GetID())
				{
					this->ChangeInternalState(PROTOCOL_CLIENT_CREDENTIAL_TRANSFER_STATE);
					this->SendExternalStateChangeMessage();
				}

				break;
			}

			case PROTOCOL_CLIENT_REGISTRATION_STATE:
			{
				// Check whether module can change state
				assert(this->_Memory.State == PROTOCOL_CLIENT_CREDENTIAL_TRANSFER_STATE && SenderID == identifier_t(ProtocolQueueID, ProtocolCertificateManagerModuleID, this->_Memory.ProtocolThreadID));

				if(this->_Memory.State == PROTOCOL_CLIENT_CREDENTIAL_TRANSFER_STATE && SenderID == identifier_t(ProtocolQueueID, ProtocolCertificateManagerModuleID, this->_Memory.ProtocolThreadID))
				{
					this->ChangeInternalState(PROTOCOL_CLIENT_REGISTRATION_STATE);
					this->SendExternalStateChangeMessage();
				}

				break;
			}

			case PROTOCOL_CLIENT_TLS_VERIFICATION_STATE:
			{
				// Check whether module can change state
				assert((this->_Memory.State == PROTOCOL_CLIENT_CREDENTIAL_TRANSFER_STATE || this->_Memory.State == PROTOCOL_CLIENT_REGISTRATION_STATE) && SenderID == identifier_t(ProtocolQueueID, ProtocolCertificateManagerModuleID, this->_Memory.ProtocolThreadID));

				if((this->_Memory.State == PROTOCOL_CLIENT_CREDENTIAL_TRANSFER_STATE || this->_Memory.State == PROTOCOL_CLIENT_REGISTRATION_STATE) && SenderID == identifier_t(ProtocolQueueID, ProtocolCertificateManagerModuleID, this->_Memory.ProtocolThreadID))
				{
					this->ChangeInternalState(PROTOCOL_CLIENT_TLS_VERIFICATION_STATE);
					this->SendExternalStateChangeMessage();
				}

				break;
			}

			case PROTOCOL_SERVER_CREDENTIALS_TRANSFER_STATE:
			{
				// Check whether module can change state
				assert(this->_Memory.State == PROTOCOL_CLIENT_TLS_VERIFICATION_STATE && SenderID == identifier_t(ProtocolQueueID, ProtocolCertificateManagerModuleID, this->_Memory.ProtocolThreadID));

				if(this->_Memory.State == PROTOCOL_CLIENT_TLS_VERIFICATION_STATE && SenderID == identifier_t(ProtocolQueueID, ProtocolCertificateManagerModuleID, this->_Memory.ProtocolThreadID))
				{
					this->ChangeInternalState(PROTOCOL_SERVER_CREDENTIALS_TRANSFER_STATE);
					this->SendExternalStateChangeMessage();
				}

				break;
			}

			case PROTOCOL_SERVER_TLS_VERIFICATION_STATE:
			{
				// Check whether module can change state
				assert(this->_Memory.State == PROTOCOL_SERVER_CREDENTIALS_TRANSFER_STATE && SenderID == identifier_t(ProtocolQueueID, ProtocolCertificateManagerModuleID, this->_Memory.ProtocolThreadID));

				if(this->_Memory.State == PROTOCOL_SERVER_CREDENTIALS_TRANSFER_STATE && SenderID == identifier_t(ProtocolQueueID, ProtocolCertificateManagerModuleID, this->_Memory.ProtocolThreadID))
				{
					this->ChangeInternalState(PROTOCOL_SERVER_TLS_VERIFICATION_STATE);
					this->SendExternalStateChangeMessage();
				}

				break;
			}

			case PROTOCOL_SECURE_CONNECTION_STATE:
			{
				// ERROR: Not yet implemented
				assert(0);

				break;
			}

			case PROTOCOL_END_STATE:
			{
				// Receiving this means the peer will end the connection. Just accept and handle it
				this->_Memory.State = PROTOCOL_END_STATE;
				this->SendInternalStateChangeMessage();
				this->SendExternalStateChangeMessage();

				break;
			}

			default:
			{
				assert(0);
				// "ERROR ProtocolModuleConnectionState::OnInternalStateChangeRequest(): Invalid state received\n;
				break;
			}
		}
	}

	void ProtocolModuleConnectionState::OnRestartRequest()
	{
		if(this->_Memory.State != PROTOCOL_STARTED)
		{
			// Reset state
			this->_Memory.State = PROTOCOL_STARTED;

			// Send state change message to peer
			if(this->_Memory.PeerState != PROTOCOL_STARTED)
				this->SendExternalStateChangeMessage();

			this->SendInternalStateChangeMessage();

			// Later switch to next state
			connection_state_change_request_t::CreateMessageFromSender(this->_Memory.ProtocolThreadID, this->GetID(), connection_state_change_request_t{PROTOCOL_CLIENT_CREDENTIAL_TRANSFER_STATE});
		}
		else if(this->_Memory.PeerState == PROTOCOL_STARTED)
		{
			// If both this side and peer are ready to start connection, switch to next state
			this->_Memory.State = PROTOCOL_CLIENT_CREDENTIAL_TRANSFER_STATE;
			this->SendInternalStateChangeMessage();

			if(this->_Memory.PeerState != PROTOCOL_CLIENT_CREDENTIAL_TRANSFER_STATE)
				this->SendExternalStateChangeMessage();
		}
	}

	void ProtocolModuleConnectionState::SendInternalStateChangeMessage()
	{
		auto tmpMessage = connection_state_change_distribution_t::CreateMessageToReceiver(identifier_t(ProtocolQueueID, ProtocolConnectionStateChangeInformerID, this->_Memory.ProtocolThreadID), this->_Memory.ProtocolThreadID, {this->_Memory.State});
		this->_Memory.GlobalQueue->PushMessage(std::move(tmpMessage));
	}

	void ProtocolModuleConnectionState::SendExternalStateChangeMessage()
	{
		// Send message to peer
		this->_Memory.SendHandle.SendData(ProtocolConnectionStateChangeRequestHeader, connection_state_change_request_t(this->_Memory.State));
	}

	void ProtocolModuleConnectionState::ChangeInternalState(protocol_state_t NewState)
	{
		this->_Memory.State = NewState;
		this->SendInternalStateChangeMessage();
	}

	ProtocolModuleConnectionStateInstantiator::ProtocolModuleConnectionStateInstantiator()
		: ProtocolModuleInstantiator(ProtocolConnectionStateModuleID,
										id_vector_t(),
										module_id_vector_t(),
										id_vector_t(),
										module_id_vector_t())
	{}

	thread_multi_module_shared_ptr_t ProtocolModuleConnectionStateInstantiator::CreateNewInstanceHandle(instantiation_data_t &Instance) const
	{
		return thread_multi_module_shared_ptr_t(new ProtocolModuleConnectionState(Instance.Memory));
	}
}
