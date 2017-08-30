#include "string_connection_functions.h"
#include "error_exception.h"

namespace string_connection_functions
{
	using error_exception::Exception;
	using error_exception::ERROR_CON_ABORT;
	using error_exception::ERROR_CON_DENY;

	void HandleAbort(NetworkConnection &Connection, const ConnectionFlagOptions &ConnectionFlags) noexcept
	{
		// At error, try to send abort
		try{Connection.WriteString(ConnectionFlags.Abort, ConnectionFlags.Timeout);}
		catch(Exception) {}
	}

	void SendAcceptOrDecline(NetworkConnection &Connection, bool Accept, const ConnectionFlagOptions &ConnectionFlags)
	{
		try
		{
			if(Accept)
				Connection.WriteString(ConnectionFlags.Accept, ConnectionFlags.Timeout);
			else
				Connection.WriteString(ConnectionFlags.Decline, ConnectionFlags.Timeout);
		}
		catch(Exception e)
		{
			// At error, try to send abort
			HandleAbort(Connection, ConnectionFlags);

			throw Exception(ERROR_CON_ABORT, "ERROR SendAcceptOrDecline(): Failed to send data\n");
		}
	}

	bool ReceiveAcceptOrDecline(NetworkConnection &Connection, const ConnectionFlagOptions &ConnectionFlags)
	{
		try
		{
			auto tmpString = Connection.ReadString(ConnectionFlags.Timeout);

			if(tmpString.compare(ConnectionFlags.Accept) != 0)
				return true;
			else if(tmpString.compare(ConnectionFlags.Decline) != 0)
				return false;
			else
				throw Exception(ERROR_CON_ABORT, "ERROR ReceiveAcceptOrDecline(): Received unknown string\n");
		}
		catch(Exception e)
		{
			// At error, try to send abort
			HandleAbort(Connection, ConnectionFlags);

			throw Exception(ERROR_CON_ABORT, "ERROR ReceiveAcceptOrDecline(): Failed to receive data\n");
		}
	}

	bool SendStringWithReturn(NetworkConnection &Connection, const string &String, const ConnectionFlagOptions &ConnectionFlags)
	{
		try
		{
			Connection.WriteString(String, ConnectionFlags.Timeout);
		}
		catch(Exception e)
		{
			// At error, try to send abort
			HandleAbort(Connection, ConnectionFlags);

			throw Exception(ERROR_CON_ABORT, "ERROR SendStringWithReturn(): Failed to send data\n");
		}

		if(ReceiveAcceptOrDecline(Connection, ConnectionFlags))
			return true;

		return false;
	}

	bool ReceiveStringWithReturn(NetworkConnection &Connection, const std::__cxx11::string &CompareString, const ConnectionFlagOptions &ConnectionFlags)
	{
		string tmpString;
		try
		{
			tmpString = Connection.ReadString(ConnectionFlags.Timeout);
		}
		catch(Exception e)
		{
			// At error, try to send abort
			HandleAbort(Connection, ConnectionFlags);

			throw Exception(ERROR_CON_ABORT, "ERROR ReceiveStringWithReturn(): Failed to receive data\n");
		}

		if(tmpString.compare(CompareString) == 0)
		{
			SendAcceptOrDecline(Connection, 1, ConnectionFlags);

			return true;
		}
		else
		{
			SendAcceptOrDecline(Connection, 0, ConnectionFlags);

			return false;
		}
	}
}
