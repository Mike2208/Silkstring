#ifndef STRING_PROTOCOLS_H
#define STRING_PROTOCOLS_H

/*!	\file string_protocols.h
 *	\brief Contains supported protocols
 */

namespace string_protocols
{
	/*!
	 * \brief Supported protocols
	 */
	enum string_protocols_t
	{
		STRING_TLS_CONNECTION = 1,
		STRING_DTLS_CONNECTION,
		STRING_TCP_CONNECTION,
		STRING_UDP_CONNECTION
	};
}

#endif // STRING_PROTOCOLS_H
