#include "tls_connection.h"
#include <gnutls/dtls.h>
#include <chrono>
#include <thread>
#include <assert.h>

namespace tls_connection
{
	using std::this_thread::sleep_for;
	using std::chrono::duration;
	using std::ratio;

	using error_exception::Exception;
	using error_exception::error_t;
	using error_exception::ERROR_NUM;

	using network_connection::SERVER_SIDE;
	using network_connection::CLIENT_SIDE;

	TLSConnection TLSConnection::SetupNetworkConnection(NetworkConnectionUniquePtr &&Connection, connection_side_t ConnectionSide)
	{
		return TLSConnection(std::move(Connection), ConnectionSide);
	}

	TLSConnection TLSConnection::SetupTLSConnection(NetworkConnectionUniquePtr &&Connection, connection_side_t ConnectionSide, TLSCertificateCredentials &&CertificateCredentials)
	{
		return TLSConnection(std::move(Connection), ConnectionSide, std::move(CertificateCredentials));
	}

	TLSConnection::TLSConnection(TLSConnection &&S) noexcept
		: _ConnectionPtr(std::move(S._ConnectionPtr)),
		  _Session(std::move(S._Session)),
		  _Credentials(std::move(S._Credentials))
	{}

	TLSConnection &TLSConnection::operator=(TLSConnection &&S) noexcept
	{
		try
		{
			this->Close();
		}
		catch(Exception e)
		{
			Exception(e.GetErrorNumber(), "ERROR: TLSConnection::operator=(TLSConnection &&): Couldn't close socket\n");
		}

		this->_ConnectionPtr = std::move(S._ConnectionPtr);
		this->_Session = std::move(S._Session);
		this->_Credentials = std::move(S._Credentials);

		return *this;
	}

	TLSConnection::~TLSConnection() noexcept
	{
		try
		{
			this->Close();
		}
		catch(Exception e)
		{
			Exception(e.GetErrorNumber(), "ERROR: TLSConnection::~TLSConnection() couldn't close socket\n");
		}
	}

	bool TLSConnection::Handshake()
	{
		// Perform handshake
		auto err = gnutls_handshake(this->_Session);
		if(err < 0)
			return 0;
			//throw Exception(error_t(err), "ERROR TLSConnection::Handshake(): Handshake failed\n");

		return 1;
	}

	vector_size_t TLSConnection::Read(byte_t *NetOutput, size_t OutputSize)
	{
		auto err = gnutls_record_recv(this->_Session, NetOutput, OutputSize);
		if(err < 0)
			throw Exception(error_t(err), "ERROR TLSConnection::Read(): Failed to receive data\n");

		return vector_size_t(err);
	}

	vector_size_t TLSConnection::Write(const byte_t *NetInput, size_t InputSize)
	{
		auto err = gnutls_record_send(this->_Session, NetInput, InputSize);
		if(err < 0)
			throw Exception(error_t(err), "ERROR TLSConnection::Write(): Failed to send data\n");

		return vector_size_t(err);
	}

	void TLSConnection::Close()
	{
		// Shutdown connection
		auto err = gnutls_bye(this->_Session, GNUTLS_SHUT_WR);
		if(err < 0)
			throw Exception(error_t(err), "ERROR TLSConnection::Close(): Couldn't close TLS connection properly\n");

		// Free credentials TODO: Don't call destructors?
		//this->_Session.~TLSSession();
		//this->_Credentials.~TLSCertificateCredentials();

		// Close connection
		try
		{
			this->_ConnectionPtr->Close();
		}
		catch(Exception e)
		{
			throw Exception(e.GetErrorNumber(), "ERROR: TLSConnection::Close() couldn't close socket\n");
		}
	}

	bool TLSConnection::IsReadDataAvailable()
	{
		return gnutls_record_check_pending(this->_Session);
	}

	TLSCertificateCredentials &&TLSConnection::ReleaseCredentials()
	{
		auto tmp = std::move(this->_Credentials);
		this->_Credentials = TLSCertificateCredentials();

		this->AssociateSessionAndCredentials();

		return std::move(tmp);
	}

	void TLSConnection::AddTrustChain(X509CertificateChain &&TrustChain)
	{
		this->_Credentials.AddTrustCredentials(std::move(TrustChain));
	}

	void TLSConnection::SetConnection(NetworkConnectionUniquePtr &&Connection)
	{
		this->_ConnectionPtr = std::move(Connection);

		this->SetConnection();
	}

	void TLSConnection::AddCertChainAndKey(X509CertificateChain &&CertChain, X509PrivateKey &&PrivateKey)
	{
		this->_Credentials.AddKeyCredentials(std::move(CertChain), std::move(PrivateKey));
	}

	void TLSConnection::SetHandshakeFunction(gnutls_handshake_description_t HandshakeStep, tls_handshake_post_t Post, const hook_fcn_t HookFcn) noexcept
	{
		gnutls_handshake_set_hook_function(this->_Session, HandshakeStep, Post, HookFcn.target<hook_fcn_type>());
	}

	TLSConnection::NetworkConnectionUniquePtr TLSConnection::Move()
	{
		return NetworkConnectionUniquePtr(new TLSConnection(std::move(*this)));
	}

	void TLSConnection::SessionSetup()
	{
		// Set priority
		auto err = gnutls_set_default_priority(this->_Session);
		if(err < 0)
			throw Exception(error_t(err), "ERROR TLSConnection::TLSConnection(): Failed to set priority\n");

		this->AssociateSessionAndCredentials();
	}

	void TLSConnection::AssociateSessionAndCredentials()
	{
		// Associate credentials with session
		const auto err = gnutls_credentials_set(this->_Session, GNUTLS_CRD_CERTIFICATE, this->_Credentials);
		if(err < 0)
			throw Exception(error_t(err), "ERROR TLSConnection::TLSConnection(): Failed to associate credentials with TLS session\n");
	}

	void TLSConnection::SetConnection() noexcept
	{
		// Set socket ID
		//gnutls_transport_set_int(this->_Session, this->_ConnectionPtr->GetSocketID().get());

		// Set Extra data of transport layer to this class
		gnutls_transport_set_ptr(this->_Session, this);

		// Set write functions
		gnutls_transport_set_push_function(this->_Session,
				[] (gnutls_transport_ptr_t pTransport, const void *WriteData, size_t WriteSize)
				{ return reinterpret_cast<TLSConnection*>(pTransport)->PushFcn(reinterpret_cast<const byte_t*>(WriteData), WriteSize); });
		//gnutls_transport_set_vec_push_function();

		// Set read functions
		gnutls_transport_set_pull_function(this->_Session,
										   [] (gnutls_transport_ptr_t pTransport, void *ReadData, size_t ReadSize)
										   { return reinterpret_cast<TLSConnection*>(pTransport)->PullFcn(reinterpret_cast<byte_t*>(ReadData), ReadSize); });

		// Set timeout function
		gnutls_transport_set_pull_timeout_function(this->_Session,
											[] (gnutls_transport_ptr_t pTransport, unsigned int ms)
											{ return reinterpret_cast<TLSConnection*>(pTransport)->PullWaitTimeFcn(ms); });
	}

	ssize_t TLSConnection::PushFcn(const byte_t *WriteData, size_t WriteSize)
	{
		try
		{
			return this->_ConnectionPtr->Write(WriteData, WriteSize);
		}
		catch(Exception e)
		{
			return -1;
		}
	}

	ssize_t TLSConnection::PullFcn(byte_t *ReadData, size_t ReadSize)
	{
		try
		{
			return this->_ConnectionPtr->Read(ReadData, ReadSize);
		}
		catch(Exception &e)
		{
			return -1;
		}
	}

	int TLSConnection::PullWaitTimeFcn(unsigned int MilliSeconds)
	{
		try
		{
			// Wait for the requested time
			sleep_for(std::chrono::duration<unsigned int, ratio<1, 1000>>{MilliSeconds});

			// Check whether data is available
			if(this->_ConnectionPtr->IsReadDataAvailable())
				return 1;

			// If not, return 0
			return 0;
		}
		catch(Exception &e)
		{
			return -1;
		}
	}

	TLSConnection::TLSConnection(NetworkConnectionUniquePtr &&Connection, connection_side_t ConnectionSide)
		: _ConnectionPtr(std::move(Connection)),
		  _Session(ConnectionSide == CLIENT_SIDE ? static_cast<gnutls_init_flags_t>(GNUTLS_CLIENT) : static_cast<gnutls_init_flags_t>(GNUTLS_SERVER))
	{
		// Basic setup
		this->SessionSetup();

		// Set connection
		this->SetConnection();
	}

	TLSConnection::TLSConnection(NetworkConnectionUniquePtr &&Connection, connection_side_t ConnectionSide, TLSCertificateCredentials &&CertificateCredentials)
		: _ConnectionPtr(std::move(Connection)),
		  _Session(ConnectionSide == CLIENT_SIDE ? static_cast<gnutls_init_flags_t>(GNUTLS_CLIENT) : static_cast<gnutls_init_flags_t>(GNUTLS_SERVER)),
		  _Credentials(std::move(CertificateCredentials))
	{
		// Basic setup
		this->SessionSetup();

		// Set connection
		this->SetConnection();
	}

	TLSConnection::TLSConnection(NetworkConnectionUniquePtr &&Connection, X509CertificateChain &&TrustChain, X509CertificateChain &&CertChain, X509PrivateKey &&PrivateKey, connection_side_t ConnectionSide, bool PerformHandshake)
		: TLSConnection(std::move(Connection), ConnectionSide)
	{
		// Set trust CA
		this->_Credentials.AddTrustCredentials(std::move(TrustChain));

		// Set certificate connection
		this->_Credentials.AddKeyCredentials(std::move(CertChain), std::move(PrivateKey));

		// Perform handshake
		if(PerformHandshake)
		{
			try{ this->Handshake(); }
			catch(Exception e) { throw Exception(e.GetErrorNumber(), "ERROR TLSConnection::TLSConnection(): Handshake failed\n"); }
		}
	}
}
