#ifndef TEST_VARIADIC_H
#define TEST_VARIADIC_H

/*! \file testvariadic.h
 *  \brief Header for test_variadic class
 */


#include <memory>

/*!
 *  \brief Namespace for test_variadic class
 */
namespace test_variadic
{
	using variadic_counter_t = unsigned int;

	template<class Class1, class Class2>
	using template_compare = std::is_same<typename std::remove_reference<Class1>::type, typename std::remove_reference<Class2>::type>;

	template<class Class1, class Class2>
	using template_convertible = std::is_convertible<Class1, Class2>;

	template<class ...Args>
	class TestVariadic;

	template<variadic_counter_t N, class ...Args>
	struct type_struct
	{
		using type = void;
		using rest_type = void;
	};

	template<variadic_counter_t N, class T, class ...Args>
	struct type_struct<N, T, Args...>
	{
		using type = typename type_struct<N-1, Args...>::type;
		using rest_type = typename type_struct<N-1, Args...>::rest_type;
	};

	template<class T, class ...Args>
	struct type_struct<0, T, Args...>
	{
		using type = T;
		using rest_type = TestVariadic<T, Args...>;
	};

	template<class ...Args>
	class TestVariadic
	{};

	/*!
	 * \brief The TestVariadic class
	 */
	template<class T, class ...Args>
	class TestVariadic<T, Args...> : public TestVariadic<Args...>
	{
		public:
			/*!
			 * 	\brief Constructor
			 */
			template<class FcnT, class ...FcnArgs>
			TestVariadic(FcnT &&Data, FcnArgs &&...ExtraData)
				: TestVariadic<Args...>(std::forward<FcnArgs>(ExtraData)...),
				  _Data(std::forward<FcnT>(Data))
			{
				static_assert(template_convertible<FcnT, T>::value, "ERROR TestVariadic(): Template Parameters of constructor and class should be the same");
			}

			TestVariadic(const TestVariadic &S) = default;
			TestVariadic(TestVariadic &S) = default;
			TestVariadic(TestVariadic &&S) = default;

			TestVariadic &operator=(const TestVariadic &S) = default;
			TestVariadic &operator=(TestVariadic &&S) = default;

			template<variadic_counter_t N>
			typename std::enable_if<N==0, T>::type &Get()
			{
				return this->_Data;
			}

			template<variadic_counter_t N>
			typename std::enable_if<N!=0, typename type_struct<N-1, Args...>::type>::type &Get()
			{
				return TestVariadic<Args...>::template Get<N-1>();
			}

			template<variadic_counter_t N>
			typename std::enable_if<N==0, typename type_struct<0, T, Args...>::rest_type>::type &GetRest()
			{
				return *this;
			}

			template<variadic_counter_t N>
			typename std::enable_if<N!=0, typename type_struct<N-1, Args...>::rest_type>::type &GetRest()
			{
				return static_cast<TestVariadic<Args...>&>(*this).template GetRest<N-1>();
			}

		private:
			T _Data;
	};
} // namespace test_variadic


#endif // TEST_VARIADIC_H
