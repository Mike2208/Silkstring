#include "thread_queued.h"

namespace thread_queued
{	
	void SleepForMs(unsigned int MicroSeconds)
	{
		// If thread is paused or no message is in queue, sleep for the specified time
		sleep_for(duration<unsigned int, ratio<1, 1000000>>{MicroSeconds});
	}

	void ThreadFcn(message_struct_t<int> &Test, void *)
	{
		const int &element = Test.Get<0>();

		(void)element;
	}

	int tester()
	{
		ThreadQueued<int> teste(ThreadFcn);

		const int test = 5;

		teste.PushMessage(std::ref(test));

		ThreadQueued<int> teste2 = std::move(teste);

		return  1;
	}
}
