#ifndef STRING_SERVER_H
#define STRING_SERVER_H

/*! \file string_server.h
 *  \brief Header for StringServer class
 */


#ifdef __linux__
#include "linux_network_socket.h"
#endif

#include "hardware_protocols.h"

#include "string_connection.h"
#include "string_thread.h"
#include "string_user_admin.h"

#include <memory>

/*!
 *  \brief Namespace for StringServer class
 */
namespace string_server
{
	using network_socket::NetworkSocketTCPServer;
	using network_socket::ip_addr_t;
	using network_socket::server_connection_queue_t;

	using hardware_protocols::hardware_protocols_t;
	using hardware_protocols::HW_CONNECTION_TCP;

	using string_thread::StringThread;
	using string_thread::StringThreadSharedPtr;

	using string_user_admin::StringUserAdmin;

	using string_connection::string_protocols_t;
	using string_connection::STRING_TCP_CONNECTION;
	using string_connection::StringConnection;

	using std::shared_ptr;

	/*!
	 * \brief The StringServer class
	 */
	template<hardware_protocols_t>
	class StringServer;

	template<>
	class StringServer<HW_CONNECTION_TCP>
	{
		public:
			/*!
			 * 	\brief Constructor
			 */
			StringServer(ip_addr_t BindAddress, const StringThreadSharedPtr &StringUsers, server_connection_queue_t QueueSize);

			/*!
			 * \brief Accept one TCP Connection
			 * \return
			 */
			StringConnection<STRING_TCP_CONNECTION> AcceptConnection();

		private:
			/*!
			 * \brief TCP Server
			 */
			NetworkSocketTCPServer _TCPServer;

			StringThreadSharedPtr _StringUsers;
	};
} // namespace string_server


#endif // STRING_SERVER_H
