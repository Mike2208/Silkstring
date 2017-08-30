#include "map_type.h"
#include "error_exception.h"

namespace map_type
{
	using namespace error_exception;

	class TestMapType
	{
		public:
			static bool Testing();
	};

	bool TestMapType::Testing()
	{
		try
		{
			MapType<int, char> testMap;

			testMap.Register(5, 6);

			auto testIterator = testMap.Find(5);
			if(testIterator == testMap.end())
				return 0;

			if(testIterator->second != 6)
				return 0;

			testMap.Register(2,9);

			testMap.Register(5,4);
			if(testIterator->second != 4)
				return 0;

			testMap.Unregister(5);
			if(testMap.Find(5) != testMap.end())
				return 0;

			return 1;
		}
		catch(Exception &)
		{
			return 0;
		}
	}
}
