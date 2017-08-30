#ifndef ERROR_EXCEPTION_H
#define ERROR_EXCEPTION_H

/*! \file error_exception.h
 *  \brief Header for Exception class
 */


#include "typesafe_class.h"
#include <exception>
#include <string>

/*!
 *  \brief Namespace for Exception class
 */
namespace error_exception
{
	using std::exception;
	using std::string;

	/*!	\class error_t
	 *	\brief Stores error value
	 */
	using error_type = int;
	class error_t
	{
		public:
			constexpr explicit error_t(error_type Value) : _Value(Value) {}

			EQUALITY_OPERATORS(error_t, _Value)
			COMPARISON_OPERATORS(error_t, _Value)

			GET_OPERATOR(error_type, _Value)

		private:
			error_type _Value;
	};

	const error_t ERROR_NONE(0);
	const error_t ERROR_NUM(-1);
	const error_t ERROR_WARN(-2);

	const error_t ERROR_CON_ABORT(-3);
	const error_t ERROR_CON_DENY(-4);

	/*!
	 * \brief The Exception class
	 */
	class Exception : public exception
	{
		public:
			/*!
			 * 	brief Constructor
			 */
			Exception() noexcept;

			Exception(exception e) noexcept;

			~Exception() noexcept;

			/*!
			 * \brief Constructor
			 * \param ErrNumber Corresponding error number
			 * \param ErrorMessage Error message to print
			 */
			Exception(error_t ErrNumber, const string &ErrorMessage, bool PrintError = true) noexcept;

			Exception(error_t ErrNumber) noexcept;

			/*!
			 * \brief Should the error be printed to stderr
			 * \param PrintError
			 */
			void SetPrintError(bool PrintError) noexcept;

			/*!
			 * \brief Prints the error to stderr
			 */
			void PrintError() noexcept;

			const char *what() const noexcept;

			error_t GetErrorNumber() const noexcept;

			void SetErrorNumber(error_t ErrNum) noexcept;

		private:
			bool _ErrorPrinted = false;
			bool _PrintError = true;

			error_t _ErrNumber = ERROR_NONE;
			string _ErrMsg = "\0";
	};
} // namespace error_exception


#endif // ERROR_EXCEPTION_H
