#include "global_message_queue_thread.h"

namespace global_message_queue_thread
{
	constexpr decltype(GlobalMessageQueueThread::QueueID) GlobalMessageQueueThread::QueueID;

	GlobalMessageQueueThread::GlobalMessageQueueThread()
		: thread_multi_module_manager_t(GlobalMessageQueueThread::QueueID, thread_queued::THREAD_PAUSED)
	{
		// Set correct callback function
		//this->_MessageCallbackFcn = bind(&GlobalMessageQueueThread::PropagateMessageToQueues, this, std::placeholders::_1);
		this->SetMessageFunction(&GlobalMessageQueueThread::PropagateMessageToQueues);;
		this->SetExtraData(this);

		this->SetThreadState(thread_queued::THREAD_RUNNING);
	}

	GlobalMessageQueueThread::~GlobalMessageQueueThread()
	{
		// Stop thread before deleting queues
		this->thread_multi_module_manager_t::~thread_multi_module_manager_t();
	}

	void GlobalMessageQueueThread::RegisterQueue(const thread_multi_module_manager_shared_ptr_t &NewMessageQueue)
	{
#ifdef DEBUG
		std::cout << "Registering queue with ID " << NewMessageQueue->GetID().MessageQueueID << ":" << NewMessageQueue->GetID().ModuleID << ":" << NewMessageQueue->GetID().ThreadID << "\n";
#endif

		this->Register(NewMessageQueue);
	}

	thread_multi_module_shared_ptr_t GlobalMessageQueueThread::UnregisterQueue(const thread_multi_module_manager_shared_ptr_t &MessageQueueToUnregister)
	{
		bool queueFound = false;
		identifier_t deleteQueueID(0,0,0);

		this->_LockLinks.lock();

		this->_ModuleListLock.lock();

		// Look for queue and store ID
		auto deleteQueue = this->FindQueueNoLock(MessageQueueToUnregister->GetID().MessageQueueID, MessageQueueToUnregister->GetID().ThreadID);
		if(deleteQueue != this->_Modules.end())
		{
			// Store queue ID to delete later. This must be done separately (see below) to be able to unlock the mutex again
			queueFound = true;
			deleteQueueID = (*deleteQueue)->GetID();
		}

		// Go through all queue modules and unlink them
		for(const auto &curModule : MessageQueueToUnregister->GetModulesNoLock())
		{
			MessageQueueToUnregister->UnlinkModuleNoLock(curModule->GetID());
		}

		this->_LockLinks.lock();

		this->_ModuleListLock.unlock();

		// Delete queue if it was found
		if(queueFound)
		{
#ifdef DEBUG
		std::cout << "Unregistering queue with ID " << deleteQueueID.MessageQueueID << ":" << deleteQueueID.ModuleID << ":" << deleteQueueID.ThreadID << "\n";
#endif
			return this->Unregister(deleteQueueID);
		}
		else
			return nullptr;
	}

	void GlobalMessageQueueThread::RegisterModule(const thread_multi_module_shared_ptr_t &NewModule, const id_vector_t &SenderLinks, const id_vector_t &ReceiverLinks)
	{
		this->_LockLinks.lock();

		this->_ModuleListLock.lock();

		// Save links
		for(const auto &curID : SenderLinks)
			this->LinkIDsNoLock(this->_SenderIDLinks, curID, NewModule->GetID());

		for(const auto &curID : ReceiverLinks)
			this->LinkIDsNoLock(this->_ReceiverIDLinks, curID, NewModule->GetID());


		// Find correct queue
		auto moduleQueue = this->FindQueueNoLock(NewModule->GetID().MessageQueueID, NewModule->GetID().ThreadID);
		if(moduleQueue != this->_Modules.end())
		{
			// Register to manager

			// Create registration message
			auto registrationMessage = module_registration_message_t::CreateMessageToReceiver(ModuleRegistrationID, module_registration_message_t(NewModule, id_vector_t(SenderLinks), id_vector_t(ReceiverLinks)));

			// Register module
#ifdef DEBUG
			std::cout << "Registering module with ID " << NewModule->GetID().MessageQueueID << ":" << NewModule->GetID().ModuleID << ":" << NewModule->GetID().ThreadID << "\n";
#endif
			(*moduleQueue)->HandleMessage(registrationMessage);
		}
#ifdef DEBUG
		else
			std::cout << "Couldn't find queue for module ID" << NewModule->GetID().MessageQueueID << ":" << NewModule->GetID().ModuleID << ":" << NewModule->GetID().ThreadID << "\n";
#endif

		this->_ModuleListLock.unlock();

		this->_LockLinks.unlock();
	}

	void GlobalMessageQueueThread::RegisterModule(const thread_multi_module_shared_ptr_t &NewModule, id_vector_t &&SendLinks, id_vector_t &&ReceiverLinks)
	{
		this->RegisterModule(NewModule, static_cast<const id_vector_t &>(SendLinks), static_cast<const id_vector_t &>(ReceiverLinks));
	}

	thread_multi_module_shared_ptr_t GlobalMessageQueueThread::UnregisterModule(identifier_t ModuleID)
	{
		thread_multi_module_shared_ptr_t retVal;

		this->_LockLinks.lock();

		this->_ModuleListLock.lock();

		// Find correct queue that has registered module
		auto moduleQueue = this->FindQueueNoLock(ModuleID.MessageQueueID, ModuleID.ThreadID);
		if(moduleQueue != this->_Modules.end())
		{
			// Unregister from manager

			// Create registration message
			module_unregistration_message_t unregistrationMessageData{ModuleID, &retVal};

			thread_multi_module_message_t unregistrationMessage(ModuleRegistrationID, ModuleRegistrationID, message_t(ModuleUnregistrationMessageType, 0), message_ptr(new module_unregistration_message_t(std::move(unregistrationMessageData))));

			// Unregister module (this function also stores the unregistered Module address in retVal)
#ifdef DEBUG
			std::cout << "Unregistering module with ID " << ModuleID.MessageQueueID << ":" << ModuleID.ModuleID << ":" << ModuleID.ThreadID << "\n";
#endif
			(*moduleQueue)->HandleMessage(unregistrationMessage);
		}

		// Unlink from this module
		this->UnlinkModuleNoLock(ModuleID);

		this->_LockLinks.unlock();

		this->_ModuleListLock.unlock();

		return retVal;
	}

	void GlobalMessageQueueThread::PushMessage(thread_multi_module_message_t Message)
	{
#ifdef DEBUG
		std::cout << "Pushing message type onto queue: " << Message.Get<MessageDataNum>().PrintType() << "\n";
#endif
		this->thread_multi_module_manager_t::PushMessage(Message);
	}

	void GlobalMessageQueueThread::PushMessageFront(thread_multi_module_message_t Message)
	{
#ifdef DEBUG
		std::cout << "Pushing message type onto queue front: " << Message.Get<MessageDataNum>().PrintType() << "\n";
#endif
		this->thread_multi_module_manager_t::PushMessageFront(Message);
	}

	void GlobalMessageQueueThread::SetThreadState(thread_state_t ThreadState)
	{
		this->thread_multi_module_manager_t::SetThreadState(ThreadState);
	}

	GlobalMessageQueueThread::msg_struct_t GlobalMessageQueueThread::Pop()
	{
		return thread_multi_module_manager_t::Pop();
	}

	void GlobalMessageQueueThread::PropagateMessageToQueues(msg_struct_t &Message, void *ExtraData)
	{
		auto *const pClass = reinterpret_cast<GlobalMessageQueueThread*>(ExtraData);

		const auto &senderID = Message.Get<MessageSenderIDNum>();
		const auto &receiverID = Message.Get<MessageReceiverIDNum>();

		// Store all queues that should receive the message
		vector_t<module_shared_ptr_t> queueReceivers;

		pClass->_LockLinks.lock();

		pClass->_ModuleListLock.lock();

#ifdef DEBUG
		std::cout << "Propagating message type: " << Message.Get<MessageDataNum>().PrintType() << "\n";
#endif

		// Send to receiver queue
		auto receiverQueue = pClass->FindQueueNoLock(receiverID.MessageQueueID, receiverID.ThreadID);
		if(receiverQueue != pClass->_Modules.end())
		{
			queueReceivers.push_back(*receiverQueue);
			//(*receiverQueue)->HandleMessage(Message);

#ifdef DEBUG
			std::cout << "\tTo Receiver queue with QueueID " << receiverID.MessageQueueID << " and ThreadID " << receiverID.ThreadID << "\n";
#endif
		}

		// Send to receiver linked queues
		pClass->AddLinkedQueues(queueReceivers, receiverID, pClass->_ReceiverIDLinks);

		// Send to sender linked queues
		pClass->AddLinkedQueues(queueReceivers, senderID, pClass->_SenderIDLinks);

		// Send to all receivers that require message
		for(auto &curReceiverQueue : queueReceivers)
		{
			curReceiverQueue->HandleMessage(Message);
		}

		pClass->_LockLinks.unlock();

		pClass->_ModuleListLock.unlock();
	}

	GlobalMessageQueueThread::module_list_t::iterator GlobalMessageQueueThread::FindQueueNoLock(identifier_t::queue_id_t QueueID, identifier_t::thread_id_t MessageQueueThreadID)
	{
		// Find queue with ID
		for(auto curIterator = this->_Modules.begin(); curIterator != this->_Modules.end(); ++curIterator)
		{
			const auto curID = (*curIterator)->GetID();
			if(curID.MessageQueueID == QueueID && curID.ThreadID == MessageQueueThreadID)
				return curIterator;
		}

		return this->_Modules.end();
	}

	void GlobalMessageQueueThread::AddLinkedQueues(vector_t<module_shared_ptr_t> &Queues, identifier_t ID, id_link_vector_t &LinkedIDs)
	{
		// Send to receiver linked queues
		auto curLink = LinkedIDs.Find(ID);
		if(curLink != LinkedIDs.end())
		{
			for(const auto &linkID : *curLink)
			{
				// Send to linked queue
				auto linkedQueue = this->FindQueueNoLock(linkID.MessageQueueID, linkID.ThreadID);
				if(linkedQueue != this->_Modules.end())
				{
					// Check that message isn't already in vector, then add it
					if(Queues.end() == Queues.Find<identifier_t>(linkID, [] (const module_shared_ptr_t &ModulePtr, identifier_t ID) { return ModulePtr->GetID().MessageQueueID == ID.MessageQueueID && ModulePtr->GetID().ThreadID == ID.ThreadID; } ))
					{
						Queues.push_back(*linkedQueue);

#ifdef DEBUG
						std::cout << "\tTo linked queue with QueueID " << linkID.MessageQueueID << " and ThreadID " << linkID.ThreadID << "\n";
#endif
					}
				}
			}
		}
	}

	void test()
	{
		GlobalMessageQueueThread testQueue;

		thread_multi_module_message_t *pMsg = nullptr;

		testQueue.PushMessage(*pMsg);
	}
}

#include "error_exception.h"
#include "dynamic_pointer.h"
#include "string_thread_message.h"

namespace global_message_queue_thread
{
	using namespace error_exception;
	using namespace dynamic_pointer;
	using namespace silkstring_message;

	using thread_queued::THREAD_PAUSED;
	using thread_queued::THREAD_RUNNING;

	static constexpr message_t::message_type_t MessageIntType = 0;

	struct message_data_int_t : public message_id_thread_struct_t<1,1,0, MessageIntType, message_data_int_t>
	{
		int Data;
		SharedDynamicPointer<void> Target;
	};

	static constexpr message_t::message_type_t MessageCharType = 1;

	struct message_data_char_t : public message_id_thread_struct_t<1,1,0, MessageCharType, message_data_char_t>
	{
		char Data;
		SharedDynamicPointer<void> Target;
	};

	template<identifier_t::queue_id_t QueueID, identifier_t::thread_id_t ThreadID>
	struct TestQueueClasses
	{
		class TestQueue : public thread_multi_module_manager_t
		{
			public:
				TestQueue()
					: thread_multi_module_manager_t(QueueID, ThreadID, THREAD_PAUSED)
				{}
		};

		template<identifier_t::module_id_t ModuleID>
		class TestModuleClass : public thread_multi_module_t
		{
			public:
				using msg_struct_t = thread_multi_module_t::msg_struct_t;

				TestModuleClass()
					: thread_multi_module_t(identifier_t(QueueID, ModuleID, ThreadID))
				{}

				void HandleMessage(msg_struct_t &Message) = 0;
		};

		//using msg_struct_t = thread_multi_module_t::msg_struct_t;

		class TestModule1 : public TestModuleClass<1>
		{
			public:
				//using msg_struct_t = typename TestModuleClass<1>::msg_struct_t;

				TestModule1() = default;

				void HandleMessage(thread_multi_module_t::msg_struct_t &Message);
		};

		class TestModule2 : public TestModuleClass<2>
		{
			public:
				TestModule2() = default;

				void HandleMessage(thread_multi_module_t::msg_struct_t &Message);
		};
	};

	using TestQueueClasses10 = TestQueueClasses<1,0>;

	template<>
	void TestQueueClasses10::TestModule1::HandleMessage(msg_struct_t &Message)
	{
		if(message_data_int_t::CheckMessageDataType(Message))
		{
			auto *const pMessageData = message_data_int_t::GetMessageData(Message);

			*(pMessageData->Target.Get<int>()) = pMessageData->Data;
		}
		else if(message_data_char_t::CheckMessageDataType(Message))
		{
			auto *const pMessageData = message_data_char_t::GetMessageData(Message);

			*(pMessageData->Target.Get<char>()) = pMessageData->Data;
		}
	}

	template<>
	void TestQueueClasses10::TestModule2::HandleMessage(msg_struct_t &Message)
	{
		if(message_data_int_t::CheckMessageDataType(Message))
		{
			auto *const pMessageData = message_data_int_t::GetMessageData(Message);

			*(pMessageData->Target.Get<int>()) = 0;
		}
		else if(message_data_char_t::CheckMessageDataType(Message))
		{
			auto *const pMessageData = message_data_char_t::GetMessageData(Message);

			*(pMessageData->Target.Get<char>()) = 0;
		}
	}

	class TestGlobalMessageQueueThread
	{
		public:
			static bool Testing();
	};

	bool TestGlobalMessageQueueThread::Testing()
	{
		try
		{
			GlobalMessageQueueThread testQueueHandle;

			testQueueHandle.SetThreadState(THREAD_PAUSED);

			auto testQueue = shared_ptr<TestQueueClasses10::TestQueue>(new TestQueueClasses10::TestQueue());
			auto testQueueModule1 = shared_ptr<TestQueueClasses10::TestModule1>(new TestQueueClasses10::TestModule1());
			auto testQueueModule2 = shared_ptr<TestQueueClasses10::TestModule2>(new TestQueueClasses10::TestModule2());

			testQueueHandle.RegisterQueue(testQueue);
			testQueueHandle.RegisterModule(testQueueModule1, id_vector_t(), id_vector_t());
			testQueueHandle.RegisterModule(testQueueModule2, id_vector_t(), id_vector_t());

			auto testNumber1 = 2;
			SharedDynamicPointer<void> pTestInt(new int(0));
			message_data_int_t testIntData;
			testIntData.Data = testNumber1;
			testIntData.Target = pTestInt;

			// Test first module
			testQueueHandle.PushMessage(message_data_int_t::CreateMessageFromSender(identifier_t(0,0,0), message_data_int_t(testIntData)));

			testQueueHandle.SetThreadState(THREAD_RUNNING);
			testQueue->SetThreadState(THREAD_RUNNING);

			// Wait for message to be sent to module
			thread_queued::SleepForMs(1000000);

			if(*(pTestInt.Get<int>()) != testNumber1)
				return 0;

			// Test 2nd module
			testIntData.Data = 0;
			testIntData.Target = pTestInt;
			testQueueHandle.PushMessage(message_data_int_t::CreateMessageToReceiver(testQueueModule2->GetID(), message_data_int_t(testIntData)));

			// Wait for message to be sent to module
			thread_queued::SleepForMs(1000000);

			if(*(pTestInt.Get<int>()) != 0)
				return 0;

			// Test Receiver Linking
			const auto testReceiverID = identifier_t(testQueue->GetID().MessageQueueID, 5, testQueue->GetID().ThreadID);
			testQueueHandle.AddReceiverLink(testReceiverID, testQueueModule1->GetID());

			testIntData.Data = testNumber1;
			testIntData.Target = pTestInt;
			testQueueHandle.PushMessage(message_data_int_t::CreateMessageToReceiver(testReceiverID, message_data_int_t(testIntData)));

			// Wait for message to be sent to module
			thread_queued::SleepForMs(1000000);

			if(*(pTestInt.Get<int>()) != testNumber1)
				return 0;

			// Test Sender Linking
			const auto testSenderID = identifier_t(testQueue->GetID().MessageQueueID, 6, testQueue->GetID().ThreadID);
			testQueueHandle.AddSendLink(testSenderID, testQueueModule2->GetID());

			testIntData.Data = testNumber1;
			testIntData.Target = pTestInt;
			testQueueHandle.PushMessage(message_data_int_t::CreateMessageFromSender(testSenderID, message_data_int_t(testIntData)));

			// Wait for message to be sent to module
			thread_queued::SleepForMs(1000000);

			if(*(pTestInt.Get<int>()) != 0)
				return 0;

			// Test unregistering
			auto testUnregisterModule = testQueueHandle.UnregisterModule(testQueueModule1->GetID());
			if(testUnregisterModule != testQueueModule1)
				return 0;

			*(pTestInt.Get<int>()) = 0;
			testIntData.Data = testNumber1;
			testIntData.Target = pTestInt;
			testQueueHandle.PushMessage(message_data_int_t::CreateMessageFromSender(identifier_t(0,0,0), message_data_int_t(testIntData)));

			// Wait for message to be sent to module
			thread_queued::SleepForMs(1000000);

			// Check that module really was unregistered
			if(*(pTestInt.Get<int>()) == testNumber1)
				return 0;

			// Test queue Unregistering
			auto testUnregisterQueue = testQueueHandle.UnregisterQueue(testQueue);
			if(testUnregisterQueue != testQueue)
				return 0;

			// Check that queue really was unregistered
			*(pTestInt.Get<int>()) = testNumber1;
			testIntData.Data = 0;
			testIntData.Target = pTestInt;
			testQueueHandle.PushMessage(message_data_int_t::CreateMessageToReceiver(testQueueModule2->GetID(), message_data_int_t(testIntData)));

			// Wait for message to be sent to module
			thread_queued::SleepForMs(1000000);

			// Check that queue really was unregistered
			if(*(pTestInt.Get<int>()) == 0)
				return 0;

			return 1;
		}
		catch(Exception &)
		{
			return 0;
		}
	}
}
