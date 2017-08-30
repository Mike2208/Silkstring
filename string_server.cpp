#include "string_server.h"

namespace string_server
{
	StringServer<HW_CONNECTION_TCP>::StringServer(ip_addr_t BindAddress, const StringThreadSharedPtr &StringUsers, server_connection_queue_t QueueSize)
		: _TCPServer(NetworkSocketTCPServer::StartServer(BindAddress, QueueSize)),
		  _StringUsers(StringUsers)
	{}

	StringConnection<STRING_TCP_CONNECTION> StringServer<HW_CONNECTION_TCP>::AcceptConnection()
	{
		return this->_TCPServer.AcceptConnection();
	}
}
