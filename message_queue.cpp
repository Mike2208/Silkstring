#include "message_queue.h"
#include "error_exception.h"

namespace message_queue
{
	using namespace error_exception;

	class TestMessageQueue
	{
		public:
			static bool Testing();
	};

	bool TestMessageQueue::Testing()
	{
		try
		{

			MessageQueue<int, float> testQueue;
			auto testInterface = testQueue.GetInterface();

			if(testInterface.GetQueueSize() != 0)
				return 0;

			const float testFloat = 1;
			testInterface.Push(4, testFloat);
			testInterface.Push(5, 1);
			testInterface.PushFront(3, 1);

			if(testInterface.GetQueueSize() != 3)
				return 0;

			const auto testPop = testQueue.Pop();
			if(testPop.Get<0>() != 3 || testPop.Get<1>() != testFloat)
				return 0;

			if(testQueue.Pop().Get<0>() != 4)
				return 0;

			if(testQueue.Pop().Get<0>() != 5)
				return 0;

			return 1;
		}
		catch(Exception&)
		{
			return 0;
		}
	}
}
