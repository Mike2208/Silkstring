#include "thread_function.h"

namespace thread_function
{
	int fcn(int *)
	{
		return 5;
	}

	int test()
	{
		int val = 5;
		ThreadFunction<decltype(fcn), int, int*> test(fcn, &val);

		test.GetResult();

		return 1;
	}
}
