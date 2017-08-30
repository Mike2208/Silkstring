#include "global_message_queue_thread.h"
//#include "string_managers.h"
//#include "user_io_thread.h"
//#include "protocol_manager.h"

#include <iostream>
//#include <pthread.h>
//#include "crypto_x509_certificate.h"
//#include "error_exception.h"
//#include "tls_connection.h"
//#include "linux_network_socket.h"

//#include "string_connection.h"
//#include "string_user_admin.h"

//#include "global_message_queue_thread.h"

//#include <arpa/inet.h>

//using namespace std;
//using error_exception::Exception;
//using namespace crypto_x509_certificate;
//using namespace crypto_x509_private_key;
//using namespace network_socket;
//using namespace string_user_admin;
//using namespace tls_connection;
//using namespace string_connection;
//using namespace string_connection_option;

using thread_message_queue::ThreadMessageQueue;
using thread_function::ThreadFunction;
using thread_queued::ThreadQueued;
using thread_module_manager::ThreadModuleManager;
using thread_module_manager_multi_message::ThreadModuleManagerMultiMessage;
using global_message_queue_thread::GlobalMessageQueueThread;
using silkstring_message::message_t;
using silkstring_message::id_vector_t;

namespace test_functions
{
	int testFcn(int a, int b, void *)
	{
		return a+b;
	}

	using threaded_message_queue_t = ThreadQueued<int, int, void*>;

	void testMessageHandler(threaded_message_queue_t::msg_struct_t &Message, void*)
	{
		std::cout << Message.Get<0>() + Message.Get<1>();
	}

	using threaded_module_manager_t = ThreadModuleManager<int,int>;
	using threaded_module_t = threaded_module_manager_t::module_t;

	class TestModule : public threaded_module_t
	{
		public:
			TestModule()
				: threaded_module_t(5)
			{}

		private:

			void HandleMessage(msg_struct_t &Message)
			{
				std::cout << Message.Get<1>();
				std::cout.flush();
			}
	};

	using threaded_multi_module_manager_t = ThreadModuleManagerMultiMessage<int,int>;
	using threaded_multi_module_t = threaded_multi_module_manager_t::module_t;

	class TestMultiModule : public threaded_multi_module_t
	{
		public:
			TestMultiModule()
				: threaded_multi_module_t(5)
			{}

		private:

			void HandleMessage(msg_struct_t &Message)
			{
				std::cout << Message.Get<2>();
				std::cout.flush();
			}
	};

	using threaded_queue_thread_t = GlobalMessageQueueThread::thread_multi_module_manager_t;
	using threaded_queue_module_thread_t = silkstring_message::thread_multi_module_t;
	class TestQueueModule : public threaded_queue_thread_t
	{
		public:
			TestQueueModule()
				: threaded_queue_thread_t(1)
			{}
	};

	class TestQueueModulesModule : public  threaded_queue_module_thread_t
	{
		public:
			TestQueueModulesModule()
				: threaded_queue_module_thread_t(silkstring_message::identifier_t(1,0,0))
			{}

		private:
			void HandleMessage(msg_struct_t &)
			{
				std::cout << "MessageReceived\n";
				std::cout.flush();
			}
	};


	int TestThreads(int argc, char *argv[])
	{
		(void)argc;
		(void)argv;

		ThreadMessageQueue<int, int, void*> testQueue;

		testQueue.Push(5,5, nullptr);
		auto testMsg = testQueue.Pop();
		(void)testMsg;

		ThreadFunction<int(int,int,void*), int, int, int, void*> testThreadFcn(&testFcn, 4,6,nullptr);
		auto testFcnResult = testThreadFcn.GetResult();
		(void)testFcnResult;

		threaded_message_queue_t testMsgThread(&testMessageHandler);
		testMsgThread.PushMessage(5,6, nullptr);
		testMsgThread.PushMessage(5,7, nullptr);

		threaded_module_manager_t testModuleManager;
		std::shared_ptr<TestModule> pTestModule(new TestModule());

		testModuleManager.Register(pTestModule);

		testModuleManager.PushMessage(5,6);

		threaded_multi_module_manager_t testMultiModuleManager;
		std::shared_ptr<TestMultiModule> pTestModule2(new TestMultiModule());

		testMultiModuleManager.Register(pTestModule2, threaded_multi_module_manager_t::id_vector_t(), threaded_multi_module_manager_t::id_vector_t());

		testMultiModuleManager.PushMessage(5,5,8);

		GlobalMessageQueueThread testGlobalQueue;
		std::shared_ptr<TestQueueModule> testGlobalQueueModule(new TestQueueModule());

		std::shared_ptr<TestQueueModulesModule> testGlobalQueueModulesModule(new TestQueueModulesModule());

		testGlobalQueue.RegisterQueue(testGlobalQueueModule);

		testGlobalQueue.PushMessage(silkstring_message::thread_multi_module_message_t(silkstring_message::identifier_t(1,0,0), silkstring_message::identifier_t(0,0,0), message_t(0,0), silkstring_message::message_ptr((int*)nullptr)));

		testGlobalQueue.RegisterModule(testGlobalQueueModulesModule, id_vector_t(), id_vector_t());


		return 0;
	}

	//int TestCertificateCreation()
	//{
	//	try
	//	{
	//		X509ID tmpID("a5a5");
	//		auto test = tmpID.ConvertToString();

	//		UserCertAdmin clientCertAdmin = UserCertAdmin::GenerateNewUserCertAdmin("testcert", time(nullptr)+10000);
	//		UserCertAdmin serverCertAdmin = UserCertAdmin::GenerateNewUserCertAdmin("testcert", time(nullptr)+10000);

	//		StringUserAdmin clientString(clientCertAdmin.MainStringUserID);
	//		clientString.GetAdminCertificates().push_back(clientCertAdmin);

	//		StringUserAdmin serverUserAdmin(serverCertAdmin.MainStringUserID);
	//		serverUserAdmin.GetAdminCertificates().push_back(serverCertAdmin);

	//		ip_addr_t ipServerAddr("0.0.0.0", IPV4, PortNum);
	//		ip_addr_t ipClientAddr("127.0.0.1", IPV4, PortNum);

	//		//StringConnectionOption<ip_addr_t> serverOption(ipServerAddr, UserCertID(), UserCertID(serverUserAdmin.GetAdminCertificates().at(0)));

	//		//serverUserAdmin.GetConnections().push_back(serverOption.ToString());

	//		//NetworkSocketTCPServer serverSocket(ipServerAddr, server_connection_queue_t(5));
	//		//NetworkSocketTCPClient clientSocket(ipClientAddr);
	//	}
	//	catch(Exception e)
	//	{}

	//	global_message_queue_thread::thread_multi_module_message_t *pMsg = nullptr;

	//	global_message_queue_thread::GlobalMessageQueueThread test;
	//	test.PushMessage(*pMsg);

	//	return 0;
	//}

}
