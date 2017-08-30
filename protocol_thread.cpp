#include "protocol_thread.h"
#include "protocol_connection_module.h"
#include "error_exception.h"
#include "protocol_connection_module.h"

namespace protocol_thread
{
	using error_exception::Exception;
	using error_exception::ERROR_NUM;

	using protocol_connection_module::ProtocolConnectionModule;

	using namespace protocol_messages;

	ProtocolThread::ProtocolThread(protocol_thread_id_t ThreadID, ProtocolNetworkConnection &Connection, GlobalMessageQueueThread &GlobalQueue, connection_side_t ConnectionSide)
		: thread_multi_module_manager_t(ProtocolQueueID, ThreadID, thread_module_manager::THREAD_PAUSED),
		  _ThreadMemory{ThreadID, &GlobalQueue, this, ProtocolModuleSendHandle(nullptr), ConnectionSide}
	{
		// Register the connection module that receives data
		auto connectionModulePtr = std::shared_ptr<ProtocolConnectionModule>(new ProtocolConnectionModule(Connection, this->_ThreadMemory));
		this->Register(connectionModulePtr);

		// Adjust send module
		this->_ThreadMemory.SendHandle = ProtocolModuleSendHandle(connectionModulePtr.get());

		// Register header handle to parse received data
		//this->Register(std::shared_ptr<ProtocolHeaderHandle>(new ProtocolHeaderHandle(this->_ThreadMemory.ProtocolThreadID, *this, *connectionModulePtr)));

		this->SetThreadState(thread_module_manager_multi_message::THREAD_RUNNING);
	}

	void ProtocolThread::ExecuteModuleInstantiators(const instantiator_vector_t &ModuleInstantiators, ProtocolNetworkConnection &Connection)
	{
		// Register all other modules
		for(const auto &curInstantiator : ModuleInstantiators)
		{
			// Identify all links
			id_vector_t senderLinks = curInstantiator->CreateSenderLinkVector(ProtocolQueueID, this->_ThreadMemory.ProtocolThreadID);
			id_vector_t receiverLinks = curInstantiator->CreateReceiverLinkVector(ProtocolQueueID, this->_ThreadMemory.ProtocolThreadID);

			// Register new module
			protocol_module_instantiator::instantiation_data_t instantiationData{this->GetMemory(), Connection};
			this->_ThreadMemory.GlobalQueue->RegisterModule(curInstantiator->CreateNewInstance(instantiationData), std::move(senderLinks), std::move(receiverLinks));
		}
	}

	ProtocolThreadMemory &ProtocolThread::GetMemory()
	{
		return this->_ThreadMemory;
	}

	const ProtocolThreadMemory &ProtocolThread::GetMemory() const
	{
		return this->_ThreadMemory;
	}
}


#include "network_dummy_connection.h"
#include "protocol_module_certificate_manager.h"

using network_dummy_connection::NetworkDummyConnection;
using network_connection::CLIENT_SIDE;
using std::unique_ptr;
using protocol_module_certificate_manager::ProtocolModuleCertificateManagerInstantiator;

using namespace protocol_thread;
using namespace error_exception;

template<>
class TestingClass<ProtocolThread>
{
	public:
		static bool Testing();
};

bool TestingClass<ProtocolThread>::Testing()
{
	try
	{
		GlobalMessageQueueThread testQueue;
		ProtocolNetworkConnection testConnection(unique_ptr<NetworkDummyConnection>(new NetworkDummyConnection), CLIENT_SIDE);

		ProtocolThread testThread(0, testConnection, testQueue, CLIENT_SIDE);


		// Test Module registration
		instantiator_vector_t testVector;
		testVector.push_back(unique_ptr<ProtocolModuleInstantiator>(new ProtocolModuleCertificateManagerInstantiator()));
		testThread.ExecuteModuleInstantiators(testVector, testConnection);

		thread_queued::SleepForMs(1000000);

		if(testThread._Modules.empty())
			return 0;

		if(testThread._Modules.back()->GetID().ModuleID != testVector.back()->GetModuleID())
			return 0;

		return 1;
	}
	catch(Exception&)
	{
		return 0;
	}
}
