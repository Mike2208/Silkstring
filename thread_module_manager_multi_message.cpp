#include "thread_module_manager_multi_message.h"

namespace thread_module_manager_multi_message
{
	class TestModule : public ThreadModuleManagerMultiMessage<int, float>::module_t
	{
		public:
			TestModule()
				: ThreadModuleManagerMultiMessage<int, float>::module_t(2)
			{}

			void HandleMessage(msg_struct_t &NewData)
			{
				NewData.Get<0>();
			}
	};

	using TestModuleSharedPtr = shared_ptr<TestModule>;


	int Mtest()
	{
		ThreadModuleManagerMultiMessage<int, float> teste;

		TestModuleSharedPtr testM(new TestModule());

		ThreadModuleManagerMultiMessage<int, float>::id_vector_t sendTest;

		teste.Register(testM, sendTest, ThreadModuleManagerMultiMessage<int, float>::id_vector_t());

		teste.PushMessage(0, 2, 5);

		return 1;
	}
}
