#include "string_tls_connection.h"
#include "hardware_connection_option.h"
#include "error_exception.h"

#include <time.h>

namespace string_tls_connection
{
//	using std::unique_ptr;

//	using network_connection::GetCurrentClockMS;

//	using error_exception::Exception;
//	using error_exception::error_t;

//	using tls_connection::byte_vector_t;

//	using crypto_x509_certificate_chain::X509CertificateChain;

//	using crypto_x509_certificate::X509Certificate;
//	using crypto_x509_certificate::X509CertificateID;

//	StringTLSBaseConnection::StringTLSBaseConnection(TLSConnection &&Connection, const shared_ptr<const StringUserAdmin> &OwnUser, const shared_ptr<const StringUser> &ServerUser, const shared_ptr<const StringUserManager> &Groups, hook_fcn_t HookFunction)
//		: TLSConnection(std::move(Connection)), _HookFunction(HookFunction), _OwnData(OwnUser), _ConnectionCerts(ServerUser), _Groups(Groups)
//	{
//		// Attempt to open a TLS connection
//		try
//		{
//			// Add the hook function to the TLS session
//			this->SetHandshakeFunction(this->_HookPos, this->_HookPre, this->_HookFunction);

//			// Try to perform handshake
//			this->Handshake();
//		}
//		catch(Exception e)
//		{
//			throw Exception(e.GetErrorNumber(), "ERROR StringTLSBaseConnection::StringTLSBaseConnection(): Failed to connect\n");
//		}
//	}

//	int StringTLSBaseConnection::SendCertificateInfoHookFcn(gnutls_session_t, unsigned int, unsigned, unsigned int, const gnutls_datum_t *)
//	{
//		try
//		{
//			//this->SendCertificateInfo();
//		}
//		catch(Exception e)
//		{
//			Exception(e.GetErrorNumber(), "ERROR StringTLSBaseConnection::SendCertificateInfoHookFcn(): Failed to establish common server certificate\n");

//			// Return error number
//			return e.GetErrorNumber().get();
//		}

//		return 0;
//	}

//	int StringTLSBaseConnection::ReceiveCertificateInfoHookFcn(gnutls_session_t, unsigned int, unsigned, unsigned int, const gnutls_datum_t *)
//	{
//		try
//		{
//			//this->ReceiveCertificateInfo();
//		}
//		catch(Exception e)
//		{
//			Exception(e.GetErrorNumber(), "ERROR StringTLSBaseConnection::SendCertificateInfoHookFcn(): Failed to establish common server certificate\n");

//			// Return error number
//			return e.GetErrorNumber().get();
//		}

//		return 0;
//	}

//	StringTLSClientConnection::StringTLSClientConnection(NetworkSocketTCPClient &&Connection, const shared_ptr<const StringUserAdmin> &OwnUser, const shared_ptr<const StringUser> &ServerUser, const shared_ptr<const StringUserManager> &Groups)
//		: StringTLSBaseConnection(std::move(Connection), OwnUser, ServerUser, Groups, bind(&StringTLSClientConnection::SendCertificateInfoHookFcn, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5))
//	{}

//	StringTLSClientConnection::StringTLSClientConnection(StringTLSClientConnection &&S) noexcept
//		: StringTLSBaseConnection(std::move(S))
//	{
//		this->_HookFunction = bind(&StringTLSClientConnection::SendCertificateInfoHookFcn, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
//	}

//	StringTLSClientConnection &StringTLSClientConnection::operator=(StringTLSClientConnection &&S) noexcept
//	{
//		static_cast<StringTLSBaseConnection&>(*this) = std::move(static_cast<StringTLSBaseConnection&>(S));
//		this->_HookFunction = bind(&StringTLSClientConnection::SendCertificateInfoHookFcn, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);

//		return *this;
//	}


//	StringTLSServerConnection::StringTLSServerConnection(NetworkSocketTCPClient &&Connection, const shared_ptr<const StringUserAdmin> &OwnUser, const shared_ptr<const StringUser> &ServerUser, const shared_ptr<const StringUserManager> &Groups)
//		: StringTLSBaseConnection(std::move(Connection), OwnUser, ServerUser, Groups, bind(&StringTLSServerConnection::ReceiveCertificateInfoHookFcn, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5))
//	{}

//	StringTLSServerConnection::StringTLSServerConnection(StringTLSServerConnection &&S) noexcept
//		: StringTLSBaseConnection(std::move(S))
//	{
//		this->_HookFunction = bind(&StringTLSServerConnection::ReceiveCertificateInfoHookFcn, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
//	}

//	StringTLSServerConnection &StringTLSServerConnection::operator=(StringTLSServerConnection &&S) noexcept
//	{
//		static_cast<StringTLSBaseConnection&>(*this) = std::move(static_cast<StringTLSBaseConnection&>(S));
//		this->_HookFunction = bind(&StringTLSServerConnection::ReceiveCertificateInfoHookFcn, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);

//		return *this;
//	}
}
