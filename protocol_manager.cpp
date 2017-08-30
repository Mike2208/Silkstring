#include "protocol_manager.h"
#include "protocol_messages.h"

namespace protocol_manager
{
	using protocol_messages::connection_state_change_request_t;
	using protocol_messages::request_receive_t;
	using protocol_messages::ProtocolQueueID;
	using protocol_messages::UnusedID;

	using error_exception::Exception;
	using error_exception::ERROR_NUM;

	ProtocolManager::ProtocolManager(GlobalMessageQueueThread &GlobalMessageQueue)
		: _GlobalMessageQueue(GlobalMessageQueue)
	{}

	protocol_thread_id_t ProtocolManager::CreateNewInstance(NetworkConnectionUniquePtr &&Connection, connection_side_t ConnectionSide)
	{
		const auto newThreadID = this->_NextFreeID++;

		// Take ownership of connection
		ProtocolNetworkConnection newConnection(std::move(Connection), newThreadID);
		this->_Connections.push_front(std::move(newConnection));

		// Create new thread
		auto pNewThread  = thread_instance_shared_ptr_t(new thread_instance_t(newThreadID, this->_Connections.front(), this->_GlobalMessageQueue, ConnectionSide));
		this->_Threads.emplace(newThreadID, pNewThread);

		// Register new thread with global queue
		this->_GlobalMessageQueue.RegisterQueue(pNewThread);

		// Create and register all modules
		pNewThread->ExecuteModuleInstantiators(this->GetAllInstantionModules(), this->_Connections.front());

		// Send out start signal
		auto tmpMessage = connection_state_change_request_t::CreateMessageFromSender(newThreadID, UnusedID, connection_state_change_request_t(protocol_messages::PROTOCOL_STARTED));
		this->_GlobalMessageQueue.PushMessage(std::move(tmpMessage));

		return newThreadID;
	}

	NetworkConnectionUniquePtr ProtocolManager::StopInstance(protocol_thread_id_t ConnectionID)
	{
		// Find connection
		auto connectionInstance = this->_Connections.begin();
		for(; connectionInstance != this->_Connections.end(); ++connectionInstance)
		{
			if(connectionInstance->GetID() == ConnectionID)
				break;
		}

		// Find thread
		auto threadInstance = this->_Threads.Find(ConnectionID);
		if(threadInstance != this->_Threads.end())
		{
			// If thread was found, unregister from global queue and empty the thread queue
			//const auto curID = threadInstance->second->GetID();
			//this->_GlobalMessageQueue.UnregisterQueue(curID.MessageQueueID, curID.ThreadID);
			this->_GlobalMessageQueue.UnregisterQueue(threadInstance->second);

			threadInstance->second->SetMessageAcceptance(0);

			while(!threadInstance->second->IsQueueEmpty())
			{}

			threadInstance->second->SetThreadState(thread_module_manager_multi_message::THREAD_PAUSED);

			// Erase thread
			this->_Threads.Unregister(threadInstance);
		}

		// Erase connection instance and return Network Connection
		if(connectionInstance != this->_Connections.end())
		{
			NetworkConnectionUniquePtr tmpPtr = connectionInstance->ReleaseOwnership();

			this->_Connections.erase(connectionInstance);

			return tmpPtr;
		}

		// If no connection was found, return nullptr
		return nullptr;
	}

	void ProtocolManager::RegisterModuleInstantiator(ProtocolModuleInstantiatorUniquePtr &&ModuleInstantiator)
	{
		for(const auto &curModule : this->_ModuleInstantiators)
		{
			if(curModule->GetModuleID() == ModuleInstantiator->GetModuleID())
				throw Exception(ERROR_NUM, "ERROR ProtocolModuleManager::RegisterModule(): ModuleID already registered");
		}

		this->_ModuleInstantiators.push_back(std::move(ModuleInstantiator));
	}

	ProtocolModuleInstantiatorUniquePtr ProtocolManager::UnregisterModule(identifier_t::module_id_t ModuleID)
	{
		for(auto curIterator = this->_ModuleInstantiators.begin(); curIterator != this->_ModuleInstantiators.end(); ++curIterator)
		{
			if(curIterator->get()->GetModuleID() == ModuleID)
			{
				// Save Pointer
				auto tmpPtr = std::move(*curIterator);

				// Erase out of vector
				this->_ModuleInstantiators.erase(curIterator);

				// Return pointer
				return tmpPtr;
			}
		}

		throw Exception(ERROR_NUM, "ERROR ProtocolModuleManager::UnregisterModule(): ModuleID not registered");
	}

	void ProtocolManager::RequestRead(protocol_thread_id_t ProtocolThreadID)
	{
		// Push message onto queue
		this->_GlobalMessageQueue.PushMessage(request_receive_t::CreateMessageFromSender(ProtocolThreadID, UnusedID, request_receive_t()));
	}

	const ProtocolManager::instantiator_vector_t &ProtocolManager::GetAllInstantionModules() const noexcept
	{
		return this->_ModuleInstantiators;
	}
}

#include "network_dummy_connection.h"
#include "protocol_connection_module.h"

namespace protocol_manager
{
	using network_dummy_connection::NetworkDummyConnection;
	using std::unique_ptr;

	using protocol_connection_module::ProtocolConnectionModuleInstantiator;

	class TestProtocolManager
	{
		public:
			static bool Testing();
	};

	bool TestProtocolManager::Testing()
	{
		try
		{
			GlobalMessageQueueThread testQueue;

			ProtocolManager testManager(testQueue);

			unique_ptr<NetworkDummyConnection> testServerDummy(new NetworkDummyConnection());
			const NetworkDummyConnection *const pTestPtr = testServerDummy.get();
			NetworkDummyConnection testClientDummy;
			NetworkDummyConnection::ConnectTwoDummies(*testServerDummy, testClientDummy);

			// void RegisterModuleInstantiator(ProtocolModuleInstantiatorUniquePtr &&ModuleInstantiator)
			unique_ptr<ProtocolConnectionModuleInstantiator> testInstantiator(new ProtocolConnectionModuleInstantiator());
			const auto moduleID = testInstantiator->GetModuleID();
			const auto *const pTestInstanciatorPtr = testInstantiator.get();
			testManager.RegisterModuleInstantiator(std::move(testInstantiator));

			// Test instance creation
			const auto testID = testManager.CreateNewInstance(std::move(testServerDummy), network_connection::SERVER_SIDE);

			// void RequestRead(protocol_thread_id_t ProtocolThreadID)
			testManager.RequestRead(testID);

			// Test instance stop
			auto pRetVal = testManager.StopInstance(testID);
			if(pRetVal.get() != pTestPtr)
				return 0;

			// ProtocolModuleInstantiatorUniquePtr UnregisterModule(identifier_t::module_id_t ModuleID)
			auto testUnregister = testManager.UnregisterModule(moduleID);
			if(testUnregister.get() != pTestInstanciatorPtr)
				return 0;

			return 1;
		}
		catch(Exception&)
		{
			return 0;
		}
	}
}
