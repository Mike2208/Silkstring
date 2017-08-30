#include "thread_message_queue.h"

namespace thread_message_queue
{
	int Mtest()
	{
		ThreadMessageQueue<int> test;
		ThreadMessageQueue<int> test2;

		int testi = 5;
		int &rtesti = testi;
		test.Push(rtesti);

		//test2 = test;

		return 1;
	}
}
