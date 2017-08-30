#include "crypto_tls_certificate_credentials.h"
#include "error_exception.h"

namespace crypto_tls_certificate_credentials
{
	using namespace error_exception;

	TLSCertificateCredentials::TLSCertificateCredentials()
	{
		if(gnutls_certificate_allocate_credentials(&this->_Credentials) != GNUTLS_E_SUCCESS)
			throw Exception(ERROR_NUM, "ERROR TLSCertificateCredentials::TLSCertificateCredentials(): Failed to allocate memory for certificate credentials\n");
	}

	TLSCertificateCredentials::TLSCertificateCredentials(TLSCertificateCredentials &&S)
		: _Credentials(std::move(S._Credentials))
	{
		S._Credentials = nullptr;
	}

	TLSCertificateCredentials &TLSCertificateCredentials::operator=(TLSCertificateCredentials &&S)
	{
		// Deallocate any previous data
		this->~TLSCertificateCredentials();

		this->_Credentials = std::move(nullptr);
		S._Credentials = nullptr;

		return *this;
	}

	TLSCertificateCredentials::~TLSCertificateCredentials()
	{
		if(this->_Credentials != nullptr)
			gnutls_certificate_free_credentials(this->_Credentials);
	}

	TLSCertificateCredentials::operator gnutls_certificate_credentials_t()
	{
		return this->_Credentials;
	}

	TLSCertificateCredentials::operator const gnutls_certificate_credentials_t() const
	{
		return this->_Credentials;
	}

	void TLSCertificateCredentials::AddTrustCredentials(X509CertificateChain &&CertificateChain)
	{
		auto certVector = CertificateChain.GetCertVector(true);
		const auto res = gnutls_certificate_set_x509_trust(this->_Credentials, certVector.data(), certVector.size());
		if(res != GNUTLS_E_SUCCESS)
			throw Exception(ERROR_NUM, "ERROR TLSCertificateCredentials::AddTrustCredentials(): Failed to add credentials\n");
	}

	void TLSCertificateCredentials::AddKeyCredentials(X509CertificateChain &&CertificateChain, X509PrivateKey &&PrivateKey)
	{
		auto certVector = CertificateChain.GetCertVector(true);
		const auto res = gnutls_certificate_set_x509_key(this->_Credentials, certVector.data(), certVector.size(), PrivateKey.ReleaseOwnership());
		if(res != GNUTLS_E_SUCCESS)
			throw Exception(ERROR_NUM, "ERROR TLSCertificateCredentials::AddKeyCredentials(): Failed to add key credentials\n");
	}

	void TLSCertificateCredentials::RemoveAllCredentials()
	{
		// Reset credentials
		*this = TLSCertificateCredentials();
	}

	using namespace crypto_tls_certificate_credentials;

	using crypto_x509_certificate::X509Certificate;
	using crypto_x509_certificate::X509CertificateRequest;
	using crypto_x509_certificate::serial_t;
	using crypto_x509_private_key::X509PrivateKey;

	class TestTLSCertificateCredentials
	{
		public:

			static bool Testing();
	};

	bool TestTLSCertificateCredentials::Testing()
	{
		X509CertificateRequest newCertRequest;

		X509PrivateKey newIssuerPrivKey = X509PrivateKey::GenerateKey();
		X509PrivateKey newPrivKey = X509PrivateKey::GenerateKey();
		X509Certificate newIssuerCert = newCertRequest.GenerateSelfSigned(newIssuerPrivKey, serial_t(1000), 0, time(nullptr)+100000);
		X509Certificate newCert = newCertRequest.SignRequest(newIssuerCert, newIssuerPrivKey, serial_t(1001), 0, time_t(nullptr)+100000);

		try
		{
			// Test Operator
			TLSCertificateCredentials testCred;

			// Test Adding trust credentials
			testCred.AddTrustCredentials(X509CertificateChain::ImportCerts(newIssuerCert));

			testCred.RemoveAllCredentials();

			// Test Adding Key Credentials and Trust Credentials
			testCred.AddKeyCredentials(X509CertificateChain::ImportCerts(newCert, newIssuerCert), X509PrivateKey(newPrivKey));
			testCred.AddTrustCredentials(X509CertificateChain::ImportCerts(newIssuerCert));

			testCred.RemoveAllCredentials();

			// Test double adding
			testCred.AddTrustCredentials(X509CertificateChain::ImportCerts(newIssuerCert));
			testCred.AddTrustCredentials(X509CertificateChain::ImportCerts(newIssuerCert));

			testCred.RemoveAllCredentials();

			// Test single adding
			testCred.AddKeyCredentials(X509CertificateChain::ImportCerts(newCert, newIssuerCert), X509PrivateKey(newPrivKey));
			testCred.AddKeyCredentials(X509CertificateChain::ImportCerts(newCert, newIssuerCert), X509PrivateKey(newPrivKey));

			// Test Move
			auto testMove = std::move(testCred);

			// Test Destructor
			testMove.~TLSCertificateCredentials();

			return true;
		}
		catch(Exception)
		{
			return false;
		}
	}
}
