#include "protocol_connection_module.h"
#include "error_exception.h"

namespace protocol_connection_module
{
	using namespace error_exception;

	constexpr decltype(ProtocolConnectionModule::ModuleID) ProtocolConnectionModule::ModuleID;

	ProtocolConnectionModule::ProtocolConnectionModule(ProtocolNetworkConnection &Connection, ProtocolThreadMemory &ThreadMemory)
		: thread_multi_module_t(identifier_t(ProtocolQueueID, ModuleID, ThreadMemory.ProtocolThreadID)), _Connection(Connection), _ThreadMemory(ThreadMemory)
	{}

	void ProtocolConnectionModule::HandleMessage(msg_struct_t &Message)
	{
		// If messageType is ReceiveMessage, get data from connection
		if(request_receive_t::CheckMessageDataType(Message, this->_ThreadMemory.ProtocolThreadID))//!messageType.IsSenderMessageType() && messageType.MessageType == request_receive_t::MessageType)
		{
			// Read data
			this->HandleReceiveRequest();
		}
		else if(peer_data_t::CheckMessageDataType(Message, this->_ThreadMemory.ProtocolThreadID))
		{
			// Get peer data from message
			auto &peerData = *(peer_data_t::GetMessageData(Message));
			this->HandlePeerData(peerData);
		}
	}

	bool ProtocolConnectionModule::HandleReceiveRequest()
	{
		// Read data
		auto tmpData = this->_Connection.ReceiveData();
		return this->ParseReceivedData(tmpData);
	}

	bool ProtocolConnectionModule::HandlePeerData(peer_data_t &PeerData)
	{
		// Get peer data from message
		return this->ParseReceivedData(PeerData.PeerData);
	}

	bool ProtocolConnectionModule::ParseReceivedData(protocol_vector_t &ReceivedData)
	{
		// Parse data from start
		ReceivedData.CurPos = ReceivedData.begin();

		// Give data to modules
		bool parseComplete = false;
		if(!ReceivedData.empty())
		{
			// Parse data and send individual headers to the requested modules
			do
			{
				const auto *const curHeader = ReceivedData.ParseVector<protocol_header_t>();
				if(curHeader != nullptr &&
					curHeader->Size >= sizeof(protocol_header_t))
				{
					const auto dataSize = curHeader->Size - sizeof(protocol_header_t);

					if(dataSize > ReceivedData.GetRestSize())
						throw Exception(ERROR_NUM, "ERROR ProtocolConnectionModule::HandleMessage(): Ill-formed header\n");

					// Save data to message_ptr
					auto tmpMessage = received_data_t::CreateMessageToReceiver(this->_ID, this->_ID.ThreadID, received_data_t(curHeader->Name, protocol_vector_t(ReceivedData.CurPos, ReceivedData.CurPos + dataSize)));
					//message_ptr pData(new received_data_t{{}, protocol_vector_t(tmpData.CurPos, tmpData.CurPos + dataSize)});
					//memcpy(pData.Get<protocol_vector_t::value_type>(), &(*tmpData.CurPos), dataSize);
					ReceivedData.CurPos += dataSize;

					// Send message all modules that requested this header
					auto curModuleIterator = this->_ThreadMemory.HeaderModuleMap.find(curHeader->Name);
					while(curModuleIterator != this->_ThreadMemory.HeaderModuleMap.end() &&
						  curModuleIterator->first == curHeader->Name)
					{
						// Set correct receiver
						tmpMessage.Get<MessageReceiverIDNum>() = curModuleIterator->second;
						this->_ThreadMemory.GlobalQueue->PushMessage(tmpMessage);
						//this->_ThreadMemory.ProtocolThreadQueue->PushMessage(curModuleIterator->second, this->_ID, ProtocolConnectionModuleSendingMessageType, pData);

						parseComplete = true;

						curModuleIterator++;
					}
				}
				else
					break;
			}while(1);
		}

		return parseComplete;
	}

	void ProtocolConnectionModule::SendDataHandle(protocol_vector_t &Data)
	{
		this->_Connection.SendData(Data);
	}

	ProtocolConnectionModuleInstantiator::ProtocolConnectionModuleInstantiator()
		: ProtocolModuleInstantiator(ProtocolConnectionModule::ModuleID,
									 id_vector_t(), module_id_vector_t(),
									 id_vector_t(), module_id_vector_t())
	{}

	thread_multi_module_shared_ptr_t ProtocolConnectionModuleInstantiator::CreateNewInstanceHandle(instantiation_data_t &Instance) const
	{
		return thread_multi_module_shared_ptr_t(new ProtocolConnectionModule(Instance.Connection, Instance.Memory));
	}
}

#include "network_dummy_connection.h"

namespace protocol_connection_module
{
	using network_dummy_connection::NetworkDummyConnection;
	using protocol_send_handle::ProtocolModuleSendHandle;
	using std::unique_ptr;

	class TestProtocolConnectionModule
	{
		public:
			static bool Testing();
	};

	bool TestProtocolConnectionModule::Testing()
	{
		try
		{
			unique_ptr<NetworkDummyConnection> testServerDummy(new NetworkDummyConnection());
			NetworkDummyConnection testClientDummy;
			NetworkDummyConnection::ConnectTwoDummies(*testServerDummy, testClientDummy);

			ProtocolNetworkConnection testServer(std::move(testServerDummy), SERVER_SIDE);
			ProtocolModuleSendHandle testSendHandle(&testServer);

			ProtocolThreadMemory testMemory{0, nullptr, nullptr, testSendHandle};

			ProtocolConnectionModule testModule(testServer, testMemory);

			// Test Write
			protocol_vector_t testWrite{'5'};
			testModule.SendData(testWrite);

			const auto testWriteString = testClientDummy.ReadString();
			if(testWriteString.compare("DATA1:<5>") != 0)
				return 0;

			// Test partial read
			const string testString = "DATA1:<5>";

			testClientDummy.WriteString(testString.substr(0, 3));
			if(testModule.HandleReceiveRequest())
				return 0;

			testClientDummy.WriteString(testString.substr(3,5));
			if(testModule.HandleReceiveRequest())
				return 0;

			testClientDummy.WriteString(testString.substr(5));
			if(!testModule.HandleReceiveRequest())
				return 0;

			return 1;
		}
		catch(Exception &)
		{
			return 0;
		}
	}
}
