#ifndef TESTING_CLASS_DECLARATION_H
#define TESTING_CLASS_DECLARATION_H

#include <memory>

template<class SubClass>
class TestingClass
{
	public:
		template<class Fcn, class RetVal, class ...Args>
		static RetVal PerformFunction(Fcn *const Function, Args ...Arguments)
		{
			return Function(Arguments...);
		}

	private:
		friend SubClass;
};

#endif // TESTING_CLASS_DECLARATION_H
