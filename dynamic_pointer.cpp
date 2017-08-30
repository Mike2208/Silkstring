#include "dynamic_pointer.h"
#include <vector>

namespace dynamic_pointer
{
	void test()
	{
		SharedDynamicPointer<void> test(new std::vector<int>(0));

		auto *pTest = test.Get<std::vector<int>>();
		(void)pTest;
	}
}

namespace dynamic_pointer
{
	class TestDynamicPointer
	{
		public:
			static bool Testing();
	};

	bool TestDynamicPointer::Testing()
	{
		try
		{
			auto *const pTestData = new int(5);

			DynamicPointer<int> testInt(pTestData);

			DynamicPointer<void> testVoid = testInt.ReleasePtr();

			DynamicPointer<int> testInt2 = testVoid.ReturnPtr<int>();

			if(testInt2.Get() != pTestData)
				return 0;

			return 1;
		}
		catch(Exception &)
		{
			return 0;
		}
	}

	class TestSharedDynamicPointer
	{
		public:
			static bool Testing();
	};

	bool TestSharedDynamicPointer::Testing()
	{
		try
		{
			auto *const pTestData = new int(5);

			SharedDynamicPointer<void> testVoid(pTestData);
			SharedDynamicPointer<void> testShared = testVoid;

			if(testVoid.Get<int>() != pTestData)
				return 0;

			if(testShared.Get<int>() != pTestData)
				return 0;

			return 1;
		}
		catch(Exception &)
		{
			return 0;
		}
	}

	enum TestSharedTyped
	{
		INT,
		FLOAT,
		CHAR
	};

	class TestSharedDynamicPointerTyped
	{
		public:
			static bool Testing();
	};

	bool TestSharedDynamicPointerTyped::Testing()
	{
		using test_t = SharedDynamicPointerTyped<int, float, char>;

		try
		{
			auto *const pTestInt = new int(5);
			auto *const pTestFloat = new float(1.04f);
			auto *const pTestChar = new char(1);

			test_t testInt(pTestInt);
			test_t testFloat(pTestFloat);
			test_t testChar(pTestChar);

			auto testShare = testChar;
			(void)testShare;

			if(testInt.GetTypeNumber() != 0)
				return 0;

			if(testFloat.GetTypeNumber() != 1)
				return 0;

			if(testChar.GetTypeNumber() != 2)
				return 0;

			if(testInt.Get<0>() != pTestInt)
				return 0;

			if(testFloat.Get<1>() != pTestFloat)
				return 0;

			if(testChar.Get<2>() != pTestChar)
				return 0;

			return 1;
		}
		catch(Exception &)
		{
			return 0;
		}
	}
}
