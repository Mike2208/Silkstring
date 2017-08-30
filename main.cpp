#define DEBUG

#include "global_message_queue_thread.h"

#include "user_io_thread.h"
#include "user_cli_thread.h"
#include "user_cli_thread_module_parser.h"
#include "user_cli_thread_module_register.h"

#include "user_cli_input_thread.h"

#include "protocol_manager.h"

#include "string_thread.h"

#include "network_dummy_connection.h"

#include "protocol_module_tls_connection.h"
#include "protocol_module_certificate_manager.h"
#include "protocol_module_connection_state.h"

#include <iostream>

using std::unique_ptr;
using std::shared_ptr;

using std::this_thread::sleep_for;
using std::chrono::duration;
using std::ratio;


using global_message_queue_thread::GlobalMessageQueueThread;

using user_cli_input_thread::UserCLIInputThread;

using user_io_thread::UserIOThread;
using user_io_thread::UserIOThreadSharedPtr;

using user_cli_thread_module_parser::UserCLIThreadModuleParser;
using user_cli_thread_module_parser::UserCLIThreadModuleParserSharedPtr;

using user_io_messages::UserIOQueueID;
//using user_cli_interface::UserCLIThreadID;
using user_io_messages::UserIOCLIReadModuleID;

using protocol_manager::ProtocolManager;
using protocol_module_instantiator::ProtocolModuleInstantiatorUniquePtr;

using network_dummy_connection::NetworkDummyConnection;
using NetworkConnectionUniquePtr = NetworkDummyConnection::NetworkConnectionUniquePtr;

using user_cli_thread::UserCLIThread;

using string_thread::StringThread;

using namespace silkstring_message;
using namespace string_thread_messages;
using namespace user_io_messages;

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	auto newCert = string_user_admin::UserCertAdmin::GenerateNewUserCertAdmin("HELLO", time(nullptr)+10000000);
	newCert.GetMainStringUserID();

	GlobalMessageQueueThread globalQueue;

	shared_ptr<StringThread> pStringThread(new StringThread(globalQueue));
	StringThread::RegisterThread(globalQueue, pStringThread);

	//auto subMessage = string_user_modification_t::mod_data_t(new modification_add_user_admin_cert_t{std::move(newCert)});
	auto tmpMessage = string_user_modification_t::CreateMessageFromSender(UnusedID, string_user_modification_t(StringUserID(newCert.GetMainStringUserID()), string_user_modification_t::mod_data_t(new modification_add_user_admin_cert_t{std::move(newCert)})));
	globalQueue.PushMessage(std::move(tmpMessage));

	//while(1){}

	UserIOThreadSharedPtr pUserIoThread(new UserIOThread());

	shared_ptr<UserCLIThread> pCLIThreadQueue(new UserCLIThread());
	UserCLIThreadModuleParserSharedPtr pUserInterface(new UserCLIThreadModuleParser(*pUserIoThread));

	globalQueue.RegisterQueue(pUserIoThread);
	globalQueue.RegisterQueue(pCLIThreadQueue);
	globalQueue.RegisterModule(pUserInterface, id_vector_t(), id_vector_t{UserIORegistrationID});

	// Start reading user input
	UserCLIInputThread inputReaderThread(&globalQueue);

	// Manage connections
	ProtocolManager protManager(globalQueue);

	// Add modules to protocol
	unique_ptr<protocol_module_certificate_manager::ProtocolModuleCertificateManagerInstantiator>	pCertificateModule(new protocol_module_certificate_manager::ProtocolModuleCertificateManagerInstantiator());
	unique_ptr<protocol_module_tls_connection::ProtocolModuleTLSConnectionInstantiator>				pTLSModule(new protocol_module_tls_connection::ProtocolModuleTLSConnectionInstantiator());
	unique_ptr<protocol_module_connection_state::ProtocolModuleConnectionStateInstantiator>			pStateModule(new protocol_module_connection_state::ProtocolModuleConnectionStateInstantiator());

	protManager.RegisterModuleInstantiator(std::move(pCertificateModule));
	protManager.RegisterModuleInstantiator(std::move(pTLSModule));
	protManager.RegisterModuleInstantiator(std::move(pStateModule));

	// Create dummy connection
	unique_ptr<NetworkDummyConnection> serverCon(new NetworkDummyConnection());
	unique_ptr<NetworkDummyConnection> clientCon(new NetworkDummyConnection());

	NetworkDummyConnection::ConnectTwoDummies(*serverCon, *clientCon);

	// Add dummies to protocol handler
	const auto serverThreadID = protManager.CreateNewInstance(std::move(serverCon), network_connection::SERVER_SIDE);
	const auto clientThreadID = protManager.CreateNewInstance(std::move(clientCon), network_connection::CLIENT_SIDE);

	protManager.RequestRead(clientThreadID);

	// Request read every second
	while(1)
	{
		protManager.RequestRead(serverThreadID);
		protManager.RequestRead(clientThreadID);

		thread_queued::SleepForMs(1000*1000*5);
	}
}
