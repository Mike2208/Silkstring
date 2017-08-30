#include "testvariadic.h"

namespace test_variadic
{
	void testI()
	{
		int testi = 5;
		int &rtesti = testi;

		TestVariadic<int&, int, int> test(rtesti, 7.0, 6);

		test.Get<0>();

		auto testot = test.GetRest<2>();
		//testot.Get<1>();

		TestVariadic<int, int, int> tmp(rtesti, 5, testot);

		testot = tmp.GetRest<0>();
		testot = TestVariadic<int, int>(5, 5);
	}
}
