#include "string_connection.h"
#include "error_exception.h"

#include "hardware_protocols.h"
#include "hardware_connection_option.h"

#include "crypto_tls_session.h"
#include "string_connection_functions.h"

#include <memory>
#include <functional>

namespace string_connection
{	
	using error_exception::Exception;
	using error_exception::ERROR_NUM;
	using error_exception::ERROR_CON_ABORT;
	using error_exception::ERROR_CON_DENY;

	using network_connection::SERVER_SIDE;

	using string_connection_functions::SendAcceptOrDecline;
	using string_connection_functions::ReceiveAcceptOrDecline;
	using string_connection_functions::SendStringWithReturn;
	using string_connection_functions::ReceiveStringWithReturn;
	using string_connection_functions::SendCertID;
	using string_connection_functions::ReceiveCertID;

	const ConnectionFlagOptions StringConnection<STRING_TCP_CONNECTION>::_ConnectionFlags = DefConnectionFlags;

	StringConnection<STRING_TCP_CONNECTION>::StringConnection(NetworkTCPSocket &&Connection) noexcept : NetworkTCPSocket(std::move(Connection))
	{}

	bool StringConnection<STRING_TCP_CONNECTION>::ExchangeIDValidity(bool IDAccepted)
	{
		if(this->_State != TCP_EXCHANGED_ID)
			throw Exception(ERROR_NUM, "ERROR StringConnection<STRING_TCP_CONNECTION>::ExchangeIDValidity(): Wrong state for exchanging validity\n");

		bool tmpValidity;
		try
		{
			// Determine whether to send or receive first
			if(this->GetConnectionSide() == SERVER_SIDE)
			{
				SendAcceptOrDecline(*this, IDAccepted, this->_ConnectionFlags);
				tmpValidity = ReceiveAcceptOrDecline(*this, this->_ConnectionFlags);

			}
			else
			{
				tmpValidity = ReceiveAcceptOrDecline(*this, this->_ConnectionFlags);
				SendAcceptOrDecline(*this, IDAccepted, this->_ConnectionFlags);
			}
		}
		catch(Exception e)
		{
			this->_State = TCP_ERROR_STATE;
			throw Exception(e.GetErrorNumber(), "ERROR StringConnection<STRING_TCP_CONNECTION>::ExchangeIDValidity(): Failed to exchange IDs\n");
		}

		if(tmpValidity && IDAccepted)
			this->_State = TCP_ID_ACCEPTED;
		else if(tmpValidity && !IDAccepted)
			this->_State = TCP_ID_DECLINED_LOCAL;
		else if(!tmpValidity && IDAccepted)
			this->_State = TCP_ID_DECLINED_REMOTE;
		else
			this->_State = TCP_ID_DECLINED_BOTH;

		return tmpValidity;
	}

	void StringConnection<STRING_TCP_CONNECTION>::SetToRawTCPConnection()
	{
		if(this->_State != TCP_ID_ACCEPTED)
			throw Exception(ERROR_NUM, "ERROR StringConnection<STRING_TCP_CONNECTION>::SetToRawTCPConnection(): IDs not yet accpeted\n");

		try
		{
			bool acceptState;

			// Determine connection side
			if(this->GetConnectionSide() == SERVER_SIDE)
			{
				acceptState = SendStringWithReturn(*this, RawTCPState, this->_ConnectionFlags);
			}
			else
			{
				acceptState = ReceiveStringWithReturn(*this, RawTCPState, this->_ConnectionFlags);
			}

			if(!acceptState)
				throw Exception(ERROR_CON_DENY, "ERROR StringConnection<STRING_TCP_CONNECTION>::SetToRawTCPConnection(): Other side rejected state change\n");
		}
		catch(Exception e)
		{
			this->_State = TCP_ERROR_STATE;
			throw Exception(e.GetErrorNumber(), "ERROR StringConnection<STRING_TCP_CONNECTION>::SetToRawTCPConnection(): Failed to set state\n");
		}
	}
}
