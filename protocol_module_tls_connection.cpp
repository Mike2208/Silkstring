#include "protocol_module_tls_connection.h"
#include "crypto_x509_certificate_chain.h"

namespace protocol_module_tls_connection
{
	using namespace error_exception;

	using crypto_x509_certificate_chain::X509CertificateChain;
	using crypto_x509_private_key::X509PrivateKey;

	using thread_queued::SleepForMs;

	ProtocolModuleTLSConnection::ProtocolModuleTLSConnection(ProtocolThreadMemory &Memory)
		: thread_multi_module_t(identifier_t(ProtocolQueueID, ProtocolTLSConnectionModuleID, Memory.ProtocolThreadID)),
		  ProtocolModuleSendHandle(Memory.SendHandle.GetSender()),
		  _Memory(Memory),
		  _StopThread(true),
		  _HandshakeState(TLS_HANDSHAKE_UNDEFINED),
		  _TLSThread(&ProtocolModuleTLSNetworkConnection::ThreadFcn, this, CLIENT_SIDE, std::move(this->_Credentials))
	{
		// Assign data from peer with ProtocolTLSConnectionReadDataUpdateHeaderName to this module
		const auto element_pair = decltype(this->_Memory.HeaderModuleMap)::value_type(ProtocolTLSConnectionReadDataUpdateHeaderName, identifier_t(ProtocolQueueID, ProtocolTLSConnectionModuleID, this->_Memory.ProtocolThreadID));
		if(!this->_Memory.HeaderModuleMap.AlreadyRegistered(element_pair))
			this->_Memory.HeaderModuleMap.insert(element_pair);
	}

	void ProtocolModuleTLSConnection::HandleMessage(msg_struct_t &Message)
	{
		if(received_data_t::CheckMessageDataType(Message, this->_Memory.ProtocolThreadID))
		{
			// Handle data received from peer
			assert(received_data_t::GetMessageData(Message)->DataType == ProtocolTLSConnectionReadDataUpdateHeaderName);

			if(received_data_t::GetMessageData(Message)->DataType == ProtocolTLSConnectionReadDataUpdateHeaderName)
			{
				// Copy data sent by peer to read buffer
				ProtocolModuleTLSConnection::CopyDataToBufferEnd(received_data_t::GetMessageData(Message)->ReceivedData, this->_ReadBuffer, this->_BuffersLock);
			}
		}
		else if(tls_write_data_updated_t::CheckMessageDataType(Message, this->_Memory.ProtocolThreadID))
		{
			// Send encrypted data to peer
			this->_Memory.SendHandle.SendData(ProtocolTLSConnectionReadDataUpdateHeaderName, ProtocolModuleTLSConnection::MoveDataFromBuffer(this->_WriteBuffer, this->_BuffersLock));
		}
		else if(tls_input_data_update_t::CheckMessageDataType(Message, this->_Memory.ProtocolThreadID))
		{
			// Handle encryption requests

			// Copy data to input buffer
			const auto *const pData = tls_input_data_update_t::GetMessageData(Message);
			ProtocolModuleTLSConnection::CopyDataToBufferEnd(*pData, this->_InputBuffer, this->_BuffersLock);
		}
		else if(tls_output_data_updated_t::CheckMessageDataType(Message, this->_Memory.ProtocolThreadID))
		{			
			// Send decrypted data to appropriate module for processing
			this->_Memory.GlobalQueue->PushMessage(peer_data_t::CreateMessageToReceiver(this->GetID(), this->_Memory.ProtocolThreadID, peer_data_t(ProtocolModuleTLSConnection::MoveDataFromBuffer(this->_InputBuffer, this->_BuffersLock))));
		}
		else if(connection_state_change_distribution_t::CheckMessageDataType(Message, this->_Memory.ProtocolThreadID))
		{
			this->HandleStateUpdate(connection_state_change_distribution_t::GetMessageData(Message)->UpdatedState);
		}
		else if(certificate_tls_credentials_answer_t::CheckMessageDataType(Message, this->_Memory.ProtocolThreadID))
		{
			// TODO: Make sure both trust and key chains are in credentials
			this->_KeyChainReceived = true;
			this->_TrustChainReceived = true;

			this->_Credentials = std::move(certificate_tls_credentials_answer_t::GetMessageData(Message)->Credentials);
		}
		else if(tls_handshake_completed_t::CheckMessageDataType(Message, this->_Memory.ProtocolThreadID))
		{
			// Handle handshake success/fail
			const auto *const pData = tls_handshake_completed_t::GetMessageDataConst(Message);

			protocol_state_t nextState;
			if(!pData->Success)
			{
				this->_HandshakeAttempts++;
				if(this->_HandshakeAttempts < this->_Memory.MaximumTLSHandshakeAttempts)
				{
					// Retry handshake
					this->_HandshakeState = TLS_HANDSHAKE_PERFORM;

					return;
				}
				else
				{
					// Stop TLS thread and send credentials back to credentials manager if handshake failed
					this->_Credentials = std::move(this->_TLSThread.GetResult());
					this->_Memory.ProtocolThreadQueue->PushMessage(certificate_tls_credentials_answer_t::CreateMessageFromSender(this->_Memory.ProtocolThreadID, this->GetID(), certificate_tls_credentials_answer_t{std::move(this->_Credentials)}));
				}
			}

			// Determine next state
			if(this->_Memory.State == PROTOCOL_CLIENT_TLS_VERIFICATION_STATE)
			{
				if(pData->Success)
					nextState = PROTOCOL_SERVER_CREDENTIALS_TRANSFER_STATE;
				else
					nextState = PROTOCOL_STARTED;
			}
			else if(this->_Memory.State == PROTOCOL_SERVER_TLS_VERIFICATION_STATE)
			{
				if(pData->Success)
					nextState = PROTOCOL_SECURE_CONNECTION_STATE;
				else
					nextState = PROTOCOL_STARTED;
			}

			// Request state change
			this->_Memory.GlobalQueue->PushMessage(connection_state_change_request_t::CreateMessageFromSender(this->_Memory.ProtocolThreadID, this->GetID(), {nextState}));
		}
		else if(certificate_tls_credentials_answer_t::CheckMessageDataType(Message, this->_Memory.ProtocolThreadID))
		{
			// Store new credentials
			this->_Credentials = std::move(certificate_tls_credentials_answer_t::GetMessageData(Message)->Credentials);
		}
	}

	void ProtocolModuleTLSConnection::HandleStateUpdate(protocol_state_t UpdatedState)
	{
		switch(UpdatedState)
		{
			case PROTOCOL_STARTED:
			{
				// Reset to original state
				this->_Credentials.RemoveAllCredentials();

				this->_KeyChainReceived = false;
				this->_TrustChainReceived = false;

				this->_HandshakeAttempts = 0;

				break;
			}

			case PROTOCOL_CLIENT_TLS_VERIFICATION_STATE:
			{
				// Set correct connection side (client acts as TLS server, server as TLS client)
				connection_side_t connectionSide;
				if(this->_Memory.ConnectionSide == CLIENT_SIDE)
				{
					// Start thread, don't request peer identity
					connectionSide = SERVER_SIDE;
				}
				else
				{
					// Start thread, don't reveal own identity yet
					connectionSide = CLIENT_SIDE;
				}

				this->_HandshakeAttempts = 0;
				this->_TLSThread(this, connectionSide, std::move(this->_Credentials));

				break;
			}

			case PROTOCOL_SERVER_TLS_VERIFICATION_STATE:
			{
				// Set correct connection side (client acts as TLS client, server as TLS server)
				const auto connectionSide = this->_Memory.ConnectionSide;

				this->_HandshakeAttempts = 0;

				// Start thread
				this->_TLSThread(this, connectionSide, std::move(this->_Credentials));

				break;
			}

			default:
				break;
		}
	}

	void ProtocolModuleTLSConnection::CopyDataToBufferEnd(const protocol_vector_t &Data, protocol_vector_t &Buffer, mutex &BufferLock)
	{
		BufferLock.lock();

		// Copy data to buffer
		Buffer.reserve(Buffer.size()+Data.size());
		Buffer.insert(Buffer.end(), Data.begin(), Data.end());

		BufferLock.unlock();
	}

	protocol_vector_t ProtocolModuleTLSConnection::MoveDataFromBuffer(protocol_vector_t &Buffer, mutex &BufferLock)
	{
		BufferLock.lock();

		auto retVal = std::move(Buffer);

		BufferLock.unlock();

		return retVal;
	}

	ProtocolModuleTLSConnectionInstantiator::ProtocolModuleTLSConnectionInstantiator()
		: ProtocolModuleInstantiator(ProtocolTLSConnectionModuleID,
									 id_vector_t(), module_id_vector_t{ProtocolConnectionStateModuleID},
									 id_vector_t(), module_id_vector_t{})
	{}

	thread_multi_module_shared_ptr_t ProtocolModuleTLSConnectionInstantiator::CreateNewInstanceHandle(instantiation_data_t &InstanceData) const
	{
		return thread_multi_module_shared_ptr_t(new ProtocolModuleTLSConnection(InstanceData.Memory));
	}

	ProtocolModuleTLSNetworkConnectionBase::ProtocolModuleTLSNetworkConnectionBase(ProtocolModuleTLSConnection &Module, GlobalMessageQueueThread &GlobalQueue)
		: _Module(&Module),
		  _GlobalQueue(GlobalQueue)
	{}

	void ProtocolModuleTLSNetworkConnectionBase::Close()
	{}

	bool ProtocolModuleTLSNetworkConnectionBase::IsReadDataAvailable()
	{
		bool retVal;

		this->_Module->_BuffersLock.lock();

		retVal = !this->_Module->_ReadBuffer.empty();

		this->_Module->_BuffersLock.unlock();

		return retVal;
	}

	size_t ProtocolModuleTLSNetworkConnectionBase::Read(byte_t *NetOutput, size_t OutputSize)
	{
		this->_Module->_BuffersLock.lock();

		auto &readBuffer = this->_Module->_ReadBuffer;

		// Get smaller size
		auto readSize = readBuffer.size();
		readSize = readSize < OutputSize ? readSize : OutputSize;

		// Copy data
		memcpy(NetOutput, readBuffer.data(), readSize);

		// Erase data from buffer
		readBuffer.erase(readBuffer.begin(), readBuffer.begin()+readSize);

		this->_Module->_BuffersLock.unlock();

		return readSize;
	}

	size_t ProtocolModuleTLSNetworkConnectionBase::Write(const byte_t *NetInput, size_t InputSize)
	{
		this->_Module->_BuffersLock.lock();

		auto &writeBuffer = this->_Module->_WriteBuffer;

		// Resize write buffer
		const auto writeSize = InputSize;
		writeBuffer.resize(writeBuffer.size() + writeSize);

		// Copy data
		memcpy(&(writeBuffer.back())-writeSize+1, NetInput, writeSize);

		this->_Module->_BuffersLock.unlock();

		// Push message that new data is in write buffer
		if(writeSize > 0)
			this->_GlobalQueue.PushMessage(tls_write_data_updated_t::CreateMessageToReceiver(this->_Module->GetID(), this->_Module->GetID().ThreadID, tls_write_data_updated_t()));

		return writeSize;
	}

	ProtocolModuleTLSNetworkConnectionBase::NetworkConnectionUniquePtr ProtocolModuleTLSNetworkConnectionBase::Move()
	{
		return NetworkConnectionUniquePtr(new ProtocolModuleTLSNetworkConnectionBase(std::move(*this)));
	}

	ProtocolModuleTLSNetworkConnection::ProtocolModuleTLSNetworkConnection(NetworkConnectionUniquePtr &&BaseConnection, connection_side_t ConnectionSide, TLSCertificateCredentials &&Credentials, transferred_shared_data_t &&TransferredData)
		: TLSConnection(TLSConnection::SetupTLSConnection(std::move(BaseConnection),
												ConnectionSide,
												std::move(Credentials))),
		  _ModuleData{std::move(TransferredData), static_cast<NetworkConnection&>(*this)},
		  _HookFunction(this->BindHook())
	{}

	ProtocolModuleTLSNetworkConnection::ProtocolModuleTLSNetworkConnection(ProtocolModuleTLSNetworkConnection &&S)
		: TLSConnection(std::move(S)),
		  _ModuleData(std::move(S._ModuleData)),
		  _HookFunction(this->BindHook())
	{}

	//ProtocolModuleTLSNetworkConnection &operator=(ProtocolModuleTLSNetworkConnection &&S);

	TLSCertificateCredentials ProtocolModuleTLSNetworkConnection::ThreadFcn(ProtocolModuleTLSConnection *const Module, connection_side_t ConnectionSide, TLSCertificateCredentials &&Credentials)
	{
		// Stop if requested, before creating TLS instance
		if(Module->_StopThread)
		{
			return std::move(Credentials);
		}

		transferred_shared_data_t extraData{Module->_OwnID, *(Module->_Memory.GlobalQueue), Module->_Memory.ProtocolThreadID, Module->_Memory.ConnectionSide};
		NetworkConnectionUniquePtr connection(new ProtocolModuleTLSNetworkConnectionBase(*Module, *(Module->_Memory.GlobalQueue)));
		ProtocolModuleTLSNetworkConnection tlsConnection(std::move(connection), ConnectionSide, std::move(Credentials), std::move(extraData));

		while(!Module->_StopThread)
		{
			// Check if handshake should be performed
			if(Module->_HandshakeState == TLS_HANDSHAKE_PERFORM)
			{
				Module->_HandshakeState = TLS_HANDSHAKE_RUNNING;

				const auto success = tlsConnection.Handshake();
				Module->_Memory.GlobalQueue->PushMessage(tls_handshake_completed_t::CreateMessageToReceiver(Module->GetID(), Module->GetID().ThreadID, tls_handshake_completed_t(success)));
				if(success)
					Module->_HandshakeState = TLS_HANDSHAKE_SUCCEEDED;
				else
					Module->_HandshakeState = TLS_HANDSHAKE_FAILED;
			}
			else if(Module->_HandshakeState == TLS_HANDSHAKE_SUCCEEDED)
			{
				// Write and read only data after handshake
				Module->_BuffersLock.lock();

				// Write data
				auto &inputBuffer = Module->_InputBuffer;
				const auto writeSize = tlsConnection.Write(inputBuffer.data(), inputBuffer.size());
				if(writeSize > 0)
					inputBuffer.erase(inputBuffer.begin(), inputBuffer.begin() + writeSize);

				// Make any output available
				if(tlsConnection.IsReadDataAvailable())
				{
					auto &outputBuffer = Module->_OutputBuffer;
					const auto oldSize = outputBuffer.size();
					outputBuffer.resize(oldSize + TLSReadBufferSize);

					// Read and resize buffer
					const auto readSize = tlsConnection.ReadTimeout(&(outputBuffer.at(oldSize)), TLSReadBufferSize, TLSReadWriteTransferTime);
					outputBuffer.resize(oldSize + readSize);

					// Inform that new data is available
					if(readSize > 0)
						Module->_Memory.GlobalQueue->PushMessage(tls_output_data_updated_t::CreateMessageToReceiver(Module->GetID(), Module->GetID().ThreadID, tls_output_data_updated_t()));
				}

				Module->_BuffersLock.unlock();
			}
		}

		return tlsConnection.ReleaseCredentials();
	}

	bool ProtocolModuleTLSNetworkConnection::SendStringUserID(const StringUserID &OwnID, shared_data_t &SharedData)
	{
		const auto sendData = OwnID.ConvertToString();
		try
		{
			SharedData.Connection.WriteString(sendData, TLSWriteTransferTime);
		}
		catch(Exception&)
		{
			return 0;
		}

		return 1;
	}

	StringUserID ProtocolModuleTLSNetworkConnection::ReceiveStringUserID(shared_data_t &SharedData)
	{
		try
		{
			const auto readData = SharedData.Connection.ReadString(TLSReadTransferTime);
			return StringUserID::ImportFromHexString(readData);
		}
		catch(Exception&)
		{
			return StringUserID();
		}
	}

	int ProtocolModuleTLSNetworkConnection::SendCertificateInfo(shared_data_t &SharedData)
	{
		// Send server certificate
		if(!static_cast<const StringUserID&>(SharedData.OwnID).IsValid())
		{
			// Request server certificate if not yet available
			SharedData.MessageQueue.PushMessage(tls_request_authentication_id_t::CreateMessageFromSender(SharedData.ThreadID, UnusedID, tls_request_authentication_id_t()));

			SleepForMs(TLSRequestCertTime.get());

			if(!static_cast<const StringUserID&>(SharedData.OwnID).IsValid())
				return -1;
		}

		// Try to send string
		if(ProtocolModuleTLSNetworkConnection::SendStringUserID(SharedData.OwnID, SharedData))
			return 0;
		else
			return -2;
	}

	int ProtocolModuleTLSNetworkConnection::ReceiveCertificateInfo(shared_data_t &SharedData)
	{
		auto peerID = ProtocolModuleTLSNetworkConnection::ReceiveStringUserID(SharedData);
		const auto validPeer = peerID.IsValid();

		SharedData.MessageQueue.PushMessage(tls_received_authentication_id_t::CreateMessageFromSender(SharedData.ThreadID, UnusedID, tls_received_authentication_id_t(std::move(peerID))));

		if(validPeer)
			return 0;
		else
			return -1;
	}

	int ProtocolModuleTLSNetworkConnection::ExchangeCertificateInfo(shared_data_t SharedData, gnutls_session_t Session, unsigned int htype, unsigned post, unsigned int incoming, const gnutls_datum_t *msg)
	{
		if(htype == GNUTLS_HANDSHAKE_SERVER_HELLO &&
				post == GNUTLS_HOOK_PRE)
		{
			if(SharedData.ConnectionSide == SERVER_SIDE)
			{
				return ProtocolModuleTLSNetworkConnection::SendCertificateInfo(SharedData);
			}
			else if(SharedData.ConnectionSide == CLIENT_SIDE)
			{
				return ProtocolModuleTLSNetworkConnection::ReceiveCertificateInfo(SharedData);
			}
		}
		else if(htype == GNUTLS_HANDSHAKE_CERTIFICATE_VERIFY &&
				post == GNUTLS_HOOK_PRE)
		{
			if(SharedData.ConnectionSide == SERVER_SIDE)
			{
				return ProtocolModuleTLSNetworkConnection::ReceiveCertificateInfo(SharedData);
			}
			else if(SharedData.ConnectionSide == CLIENT_SIDE)
			{
				return ProtocolModuleTLSNetworkConnection::SendCertificateInfo(SharedData);
			}
		}
		else
		{
			return 0;
		}

		return -1;
	}

	ProtocolModuleTLSNetworkConnection::hook_fcn_t ProtocolModuleTLSNetworkConnection::BindHook()
	{
		return std::bind(&ProtocolModuleTLSNetworkConnection::ExchangeCertificateInfo, this->_ModuleData, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
	}
}

#include "error_exception.h"
#include "network_dummy_connection.h"
#include "string_user_admin.h"

using namespace network_dummy_connection;
using namespace error_exception;
using namespace protocol_module_tls_connection;

using string_user_admin::StringUserAdmin;
using crypto_x509_certificate::X509Certificate;
using crypto_x509_private_key::X509PrivateKey;

using std::unique_ptr;

using thread_function::ThreadFunction;

using test_tls_thread_t = ThreadFunction<bool(StringUserAdmin*, StringUserAdmin*, unique_ptr<NetworkDummyConnection>*), bool, StringUserAdmin*, StringUserAdmin*, unique_ptr<NetworkDummyConnection>*>;

bool TLSTestFunction(StringUserAdmin *TrustAdmin, StringUserAdmin *KeyAdmin, unique_ptr<NetworkDummyConnection> *Connection)
{
	try
	{
		TLSCertificateCredentials clientCredentials;

		clientCredentials.AddTrustCredentials(X509CertificateChain::ImportCerts(X509Certificate(TrustAdmin->GetAdminCertificates().front())));
		clientCredentials.AddKeyCredentials(X509CertificateChain::ImportCerts(X509Certificate(KeyAdmin->GetAdminCertificates().front())), X509PrivateKey(KeyAdmin->GetAdminCertificates().front().GetKey()));

		protocol_network_connection::ProtocolNetworkConnection testClient(std::move(*Connection), network_connection::CLIENT_SIDE);
		ProtocolModuleSendHandle testHandle(&testClient);

		auto testMemory = ProtocolThreadMemory{0, nullptr, nullptr, testHandle};
		ProtocolModuleTLSConnection testClientModule(testMemory);

		ProtocolModuleTLSNetworkConnection testTLSClient(std::move(*Connection), CLIENT_SIDE, std::move(clientCredentials));

		if(testTLSClient.Handshake() == 0)
			return 0;

		return 1;
	}
	catch(Exception&)
	{
		return 0;
	}
}

template<>
class TestingClass<protocol_module_tls_connection::ProtocolModuleTLSNetworkConnection>
{
	public:
		static bool Testing();
};

bool TestingClass<protocol_module_tls_connection::ProtocolModuleTLSNetworkConnection>::Testing()
{
	try
	{
		TLSCertificateCredentials serverCredentials;

		StringUserAdmin serverCerts = StringUserAdmin::GenerateNewAdmin("test", time(nullptr)+100000);
		StringUserAdmin clientCerts = StringUserAdmin::GenerateNewAdmin("test", time(nullptr)+100000);

		serverCredentials.AddKeyCredentials(X509CertificateChain::ImportCerts(X509Certificate(serverCerts.GetAdminCertificates().front())), X509PrivateKey(serverCerts.GetAdminCertificates().front().GetKey()));
		serverCredentials.AddTrustCredentials(X509CertificateChain::ImportCerts(X509Certificate(clientCerts.GetAdminCertificates().front())));

		unique_ptr<NetworkDummyConnection> testServerDummy(new NetworkDummyConnection);
		unique_ptr<NetworkDummyConnection> testClientDummy(new NetworkDummyConnection);

		ProtocolModuleTLSNetworkConnection testTLSServer(std::move(testServerDummy), SERVER_SIDE, std::move(serverCredentials));

		test_tls_thread_t testThread(TLSTestFunction, &serverCerts, &clientCerts, &testClientDummy);//, &serverCerts, &clientCerts, &testClientDummy);

		if(testTLSServer.Handshake() == false)
			return 0;

		return testThread.GetResult();
	}
	catch(Exception&)
	{
		return 0;
	}
}
