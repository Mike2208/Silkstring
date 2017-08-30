#include "thread_module_manager.h"

namespace thread_module_manager
{
	class TestModule : public ThreadModule<int, char>
	{
		public:
			TestModule()
				: ThreadModule<int, char>(0)
			{}

			void HandleMessage(msg_struct_t &NewData)
			{
				NewData.Get<0>();
			}

		private:
	};

	using TestModuleSharedPtr = shared_ptr<TestModule>;

	int Mtest()
	{
		ThreadModuleManager<int, char> test;

		TestModuleSharedPtr testPtr(new TestModule());

		auto tmpMessage = TestModule::msg_struct_t(5,2);

		testPtr->HandleMessage(tmpMessage);

		test.Register(testPtr);
		test.PushMessage(0,0);

		auto test2(std::move(test));

		return 1;
	}
}
