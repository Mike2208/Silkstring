#include "error_exception.h"
#include <iostream>

namespace error_exception
{
	Exception::Exception() noexcept
	{}

	Exception::Exception(exception e) noexcept : exception(e), _ErrNumber(ERROR_NUM), _ErrMsg("\0")
	{}

	Exception::~Exception() noexcept
	{
		if(this->_PrintError)
			this->PrintError();
	}

	Exception::Exception(error_t ErrorNumber, const string &ErrorMessage, bool PrintError) noexcept : _PrintError(PrintError), _ErrNumber(ErrorNumber), _ErrMsg(ErrorMessage)
	{
		if(ErrorMessage.empty())
			this->_ErrMsg = '\0';

		if(this->_PrintError)
			this->PrintError();
	}

	Exception::Exception(error_t ErrNumber) noexcept : _PrintError(false), _ErrNumber(ErrNumber), _ErrMsg("\0")
	{}

	void Exception::SetPrintError(bool PrintError) noexcept
	{
		this->_PrintError = PrintError;
	}

	void Exception::PrintError() noexcept
	{
		if(!this->_ErrorPrinted)
		{
			std::cerr << this->what();
			this->_ErrorPrinted = true;
		}
	}

	const char *Exception::what() const noexcept
	{
		return this->_ErrMsg.c_str();
	}

	error_t Exception::GetErrorNumber() const noexcept
	{
		return this->_ErrNumber;
	}
}
