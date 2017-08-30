#ifndef STRING_CONNECTION_FUNCTIONS_H
#define STRING_CONNECTION_FUNCTIONS_H

/*! \file string_connection_functions.h
 *  \brief Header for Fu class
 */


#include "network_connection.h"
#include "crypto_x509_certificate.h"

#include <string>

/*!
 *  \brief Namespace for connection functions
 */
namespace string_connection_functions
{
	using network_connection::NetworkConnection;
	using std::string;
	using crypto_x509_certificate::X509CertificateID;

	using network_connection::timeout_ms_t;

	struct ConnectionFlagOptions
	{
		timeout_ms_t Timeout = network_connection::STRING_TIMEOUT_MS;

		string Accept = "OK";
		string Decline = "DENY";
		string Abort  = "ABORT";
	};

	const ConnectionFlagOptions DefConnectionFlags;

	void SendAcceptOrDecline(NetworkConnection &Connection, bool Accept, const ConnectionFlagOptions &ConnectionFlags);
	bool ReceiveAcceptOrDecline(NetworkConnection &Connection, const ConnectionFlagOptions &ConnectionFlags);

	bool SendStringWithReturn(NetworkConnection &Connection, const string &String, const ConnectionFlagOptions &ConnectionFlags);
	bool ReceiveStringWithReturn(NetworkConnection &Connection, const string &CompareString, const ConnectionFlagOptions &ConnectionFlags);

	// Send and receive certificate
	void SendCertID(NetworkConnection &Connection, const X509CertificateID &CertToSend, const ConnectionFlagOptions &SendFlags);
	X509CertificateID ReceiveCertID(NetworkConnection &Connection, const ConnectionFlagOptions &SendFlags);
} // namespace string_connection_functions


#endif // STRING_CONNECTION_FUNCTIONS_H
