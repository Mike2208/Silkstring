#include "crypto_x509_certificate.h"
#include "error_exception.h"
#include "vector_t.h"
#include <gnutls/abstract.h>
#include <fstream>
#include <gnutls/abstract.h>

namespace crypto_x509_certificate
{
	using std::fstream;
	using vector_t::byte_vector_t;

	using error_exception::Exception;
	using error_exception::error_t;
	using error_exception::ERROR_NUM;

	using vector_t::vector_type;

	const size_t DefaultFileWriteBufferSize = 10*1024;

	X509Certificate::X509Certificate()
	{
		auto err = gnutls_x509_crt_init(&this->_CertificateData);
		if(err < 0)
			throw Exception(error_t(err), "ERROR X509Certificate::X509Certificate(): Failed to initialize certificate data\n");
	}

	X509Certificate X509Certificate::ImportFromPEMString(const string &ImportPEMString)
	{
		X509Certificate retCert;

		const gnutls_datum_t tmpBuffer{const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(&ImportPEMString.front())), static_cast<unsigned int>(ImportPEMString.size())};
		auto err = gnutls_x509_crt_import(retCert._CertificateData, &tmpBuffer, GNUTLS_X509_FMT_PEM);
		if(err < 0)
			throw Exception(error_t(err), "ERROR X509Certificate::X509Certificate(string&): Couldn't import certificate\n");

		return retCert;
	}

	X509Certificate X509Certificate::ImportGnuTLSCertificate(gnutls_x509_crt_t &&GnuTLSCertificate)
	{
		return X509Certificate(std::move(GnuTLSCertificate));
	}

	X509Certificate::X509Certificate(const X509Certificate &S) : X509Certificate()
	{
		this->Copy(S);
	}

	X509Certificate &X509Certificate::operator=(const X509Certificate &S)
	{
		this->Copy(S);

		return *this;
	}

	X509Certificate::X509Certificate(X509Certificate &&S) noexcept : _CertificateData(std::move(S._CertificateData))
	{
		S._CertificateData = nullptr;
	}

	X509Certificate &X509Certificate::operator=(X509Certificate &&S) noexcept
	{
		// Delete old data
		this->Deinitialize();

		// Move data here
		this->_CertificateData = S._CertificateData;
		S._CertificateData = nullptr;

		return *this;
	}

	X509Certificate::~X509Certificate() noexcept
	{
		this->Deinitialize();
	}

	bool X509Certificate::operator==(X509Certificate &S)
	{
		return X509CertificateID::ImportFromCertificate(*this) == X509CertificateID::ImportFromCertificate(S);
	}

	bool X509Certificate::operator!=(X509Certificate &S)
	{
		return X509CertificateID::ImportFromCertificate(*this) != X509CertificateID::ImportFromCertificate(S);
	}

	string X509Certificate::ExportToPEMString() const
	{
		// Buffer to write PEM data to
		string buffer;
		buffer.resize(DefaultFileWriteBufferSize);
		auto bufferSize = buffer.size();

		// Export certificate
		auto err = gnutls_x509_crt_export(this->_CertificateData, GNUTLS_X509_FMT_PEM, &(buffer.front()), &bufferSize);
		if(err == GNUTLS_E_SHORT_MEMORY_BUFFER)
		{
			buffer.resize(bufferSize);
			err = gnutls_x509_crt_export(this->_CertificateData, GNUTLS_X509_FMT_PEM, &(buffer.front()), &bufferSize);
		}

		if(err < 0)
			throw Exception(ERROR_NUM, "ERROR X509Certificate::ExportToPEMString(): Failed to convert to PEM string with error:"+ string(gnutls_strerror(err))+"\n");

		buffer.resize(bufferSize);

		return buffer;
	}

	void X509Certificate::ExportToPEMFile(const string &FileName)
	{
		// Export to string
		auto buffer = this->ExportToPEMString();

		// Open file
		fstream fileStream(FileName, std::ios_base::out);
		if(!fileStream.is_open())
			throw Exception(ERROR_NUM, "ERROR X509Certificate::ExportToPEMFile(): Failed to open file\n");

		// Write to file
		fileStream.write(&(buffer.at(0)), buffer.size());
		if(fileStream.fail())
			throw Exception(ERROR_NUM, "ERROR X509Certificate::ExportToPEMFile(): Failed to write PEM data to file\n");
	}

	X509Certificate::operator gnutls_x509_crt_t()
	{
		return this->_CertificateData;
	}

	gnutls_x509_crt_t X509Certificate::ReleaseOwnership()
	{
		auto tmp = this->_CertificateData;
		this->_CertificateData = nullptr;

		return tmp;
	}

	void X509Certificate::Deinitialize() noexcept
	{
		gnutls_x509_crt_deinit(this->_CertificateData);
		this->_CertificateData = nullptr;
	}


	X509Certificate::X509Certificate(gnutls_x509_crt_t &&Certificate)
		: _CertificateData(std::move(Certificate))
	{
		Certificate = nullptr;
	}

	void X509Certificate::Copy(const X509Certificate &S)
	{
		if(S._CertificateData != nullptr)
		{
			// Initialize memory if required
			if(this->_CertificateData == nullptr)
			{
				auto err = gnutls_x509_crt_init(&this->_CertificateData);
				if(err < 0)
					throw Exception(error_t(err), "ERROR X509Certificate::Copy(): Failed to initialize certificate data\n");
			}

			gnutls_datum_t tmpBuffer{nullptr, 0};
			auto err = gnutls_x509_crt_export2(S._CertificateData, GNUTLS_X509_FMT_DER, &tmpBuffer);
			if(err < 0)
				throw Exception(error_t(err), "ERROR X509Certificate::Copy(): Failed to move to temporary buffer\n");

			err = gnutls_x509_crt_import(this->_CertificateData, &tmpBuffer, GNUTLS_X509_FMT_DER);
			if(err < 0)
				throw Exception(error_t(err), "ERROR X509Certificate::Copy(): Failed to move from temporary buffer\n");

			gnutls_free(tmpBuffer.data);
		}
		else
		{
			this->Deinitialize();
		}
	}

	X509CertificateRequest::X509CertificateRequest()
	{
		this->Initialize();

		auto error = gnutls_x509_crq_set_version(this->_CertRequest, 3);
		if(error < 0)
			throw Exception(error_t(error), "ERROR X509CertificateRequest::X509CertificateRequest(): Failed to set version\n");
	}


	X509CertificateRequest X509CertificateRequest::ImportFromPEMString(const string &PEMString)
	{
		X509CertificateRequest retVal;

		gnutls_datum_t data;
		data.data = const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(PEMString.data()));
		data.size = PEMString.size();

		const auto err = gnutls_x509_crq_import(retVal._CertRequest, &data, GNUTLS_X509_FMT_PEM);
		if(err < 0)
			throw Exception(ERROR_NUM, "ERROR X509CertificateRequest::ImportFromPEMString(): Couldn't import certificate request\n");

		return retVal;
	}

	X509CertificateRequest::X509CertificateRequest(const X509CertificateRequest &S)
	{
		this->Copy(S);
	}

	X509CertificateRequest &X509CertificateRequest::operator=(const X509CertificateRequest &S)
	{
		this->Copy(S);

		return *this;
	}

	X509CertificateRequest::X509CertificateRequest(X509CertificateRequest &&S) noexcept : _CertRequest(std::move(S._CertRequest))
	{
		S._CertRequest = nullptr;
	}

	X509CertificateRequest &X509CertificateRequest::operator=(X509CertificateRequest &&S) noexcept
	{
		// Free any previous data
		this->Deinitialize();

		this->_CertRequest = S._CertRequest;
		S._CertRequest = nullptr;

		return *this;
	}

	X509CertificateRequest::~X509CertificateRequest() noexcept
	{
		this->Deinitialize();
	}

	void X509CertificateRequest::SetDNField(dn_field_element_t DNField, string Entry)
	{
		const char *pEntry;
		if(Entry.empty())
			pEntry = nullptr;
		else
			pEntry = &(Entry.at(0));

		const unsigned int entrySize = static_cast<unsigned int>(Entry.size());
		int err;
		switch(DNField)
		{
			case DN_COMMON_NAME:
				err = gnutls_x509_crq_set_dn_by_oid(this->_CertRequest, GNUTLS_OID_X520_COMMON_NAME, 0, pEntry, entrySize);
				break;

			case DN_COUNTRY:
				err = gnutls_x509_crq_set_dn_by_oid(this->_CertRequest, GNUTLS_OID_X520_COUNTRY_NAME, 0, pEntry, entrySize);
				break;

			case DN_LOCALITY:
				err = gnutls_x509_crq_set_dn_by_oid(this->_CertRequest, GNUTLS_OID_X520_LOCALITY_NAME, 0, pEntry, entrySize);
				break;

			case DN_ORGANIZATION:
				err = gnutls_x509_crq_set_dn_by_oid(this->_CertRequest, GNUTLS_OID_X520_ORGANIZATION_NAME, 0, pEntry, entrySize);
				break;

			case DN_ORGANIZATIONAL_UNIT:
				err = gnutls_x509_crq_set_dn_by_oid(this->_CertRequest, GNUTLS_OID_X520_ORGANIZATIONAL_UNIT_NAME, 0, pEntry, entrySize);
				break;

			case DN_PROVINCE:
				err = gnutls_x509_crq_set_dn_by_oid(this->_CertRequest, GNUTLS_OID_X520_STATE_OR_PROVINCE_NAME, 0, pEntry, entrySize);
				break;

			default:
				throw Exception(ERROR_NUM, "ERROR X509CertificateRequest::SetDNField(): Unknown DN type\n");
		}

		if(err < 0)
			throw Exception(error_t(err), "ERROR X509CertificateRequest::SetDNField(): Failed to set DN field\n");
	}

	void X509CertificateRequest::SetCertVersion(x509_cert_version_t Version)
	{
		error_t err(gnutls_x509_crq_set_version(this->_CertRequest, Version));
		if(err < ERROR_NUM)
			throw Exception(err, "ERROR X509CertificateRequest::SetCertVersion(): Failed to set version\n");
	}

	void X509CertificateRequest::SetAndSignWithPrivateKey(X509PrivateKey &PrivateKey, gnutls_digest_algorithm_t DigestAlgorithm)
	{
		// Set private key
		auto err = gnutls_x509_crq_set_key(this->_CertRequest, PrivateKey);
		if(err < 0)
			throw Exception(error_t(err), "ERROR X509CertificateRequest::SetPrivateKey(): Failed to set private key\n");

		// Self sign request
		err = gnutls_x509_crq_sign2(this->_CertRequest, PrivateKey, DigestAlgorithm, 0);
		if(err < 0)
			throw Exception(error_t(err), "ERROR X509CertificateRequest::SetPrivateKey(): Failed to self-sign request\n");
	}

	X509Certificate X509CertificateRequest::SignRequest(X509Certificate &IssuerCertificate, X509PrivateKey &IssuerPrivateKey, serial_t Serial, time_t ActivationTime, time_t ExpiryTime, gnutls_digest_algorithm_t SignAlgorithm)
	{
		auto newCert = this->CreateCertificate(Serial, ActivationTime, ExpiryTime);

		auto err = gnutls_x509_crt_sign2(newCert._CertificateData, IssuerCertificate, IssuerPrivateKey, SignAlgorithm, 0);
		if(err < 0)
			throw Exception(error_t(err), "ERROR X509CertificateRequest::SignRequest(): Failed to sign cert with new issuer\n");

		return newCert;
	}

	X509Certificate X509CertificateRequest::GenerateSelfSigned(X509PrivateKey &PrivateKey, serial_t Serial, time_t ActivationTime, time_t ExpiryTime, gnutls_digest_algorithm_t SignAlgorithm)
	{
		auto newCert = this->CreateCertificate(Serial, ActivationTime, ExpiryTime);

		auto err = gnutls_x509_crt_sign2(newCert._CertificateData, newCert._CertificateData, PrivateKey, SignAlgorithm, 0);
		if(err < 0)
			throw Exception(error_t(err), "ERROR X509CertificateRequest::SignRequest(): Failed to sign cert with new issuer\n");

		return newCert;
	}

	string X509CertificateRequest::ExportToPEMString()
	{
		size_t stringLength = 10*1024;
		string pemString;
		pemString.resize(stringLength);

		auto err = gnutls_x509_crq_export(this->_CertRequest, GNUTLS_X509_FMT_PEM, &(pemString.front()), &stringLength);
		if(err == GNUTLS_E_SHORT_MEMORY_BUFFER)
		{
			pemString.resize(stringLength);
			err = gnutls_x509_crq_export(this->_CertRequest, GNUTLS_X509_FMT_PEM, &(pemString.front()), &stringLength);
		}

		if(err < 0)
			throw Exception(error_t(err), "ERROR X509CertificateRequest::ExportToPEMString(): Failed to export request");

		pemString.resize(stringLength);

		return pemString;
	}

	void X509CertificateRequest::ExportToPEMFile(const string &FileName)
	{
		gnutls_datum_t buffer {nullptr, 0};
		int err = gnutls_x509_crq_export2(this->_CertRequest, GNUTLS_X509_FMT_PEM, &buffer);
		if(err < 0)
			throw Exception(error_t(err), "ERROR X509CertificateRequest::ExportToPEMFile(): Failed to write to buffer\n");

		fstream outFile(FileName, std::ios_base::out);
		if(!outFile.is_open())
			throw Exception(error_t(err), "ERROR X509CertificateRequest::ExportToPEMFile(): Failed to open file\n");

		outFile.write(reinterpret_cast<const char*>(buffer.data), buffer.size);
		if(outFile.fail())
			throw Exception(error_t(err), "ERROR X509CertificateRequest::ExportToPEMFile(): Failed to write to file\n");

		gnutls_free(buffer.data);
	}

	void X509CertificateRequest::Deinitialize() noexcept
	{
		if(this->_CertRequest != nullptr)
		{
			gnutls_x509_crq_deinit(this->_CertRequest);
			this->_CertRequest = nullptr;
		}
	}

	void X509CertificateRequest::Initialize()
	{
		if(this->_CertRequest == nullptr)
		{
			auto error = gnutls_x509_crq_init(&this->_CertRequest);
			if(error < 0)
				throw Exception(error_t(error), "ERROR X509CertificateRequest::Initialize(): Failed to init request\n");
		}
	}

	void X509CertificateRequest::Copy(const X509CertificateRequest &S)
	{
		if(S._CertRequest != nullptr)
		{
			this->Initialize();

			gnutls_datum_t tmpBuffer{nullptr, 0};
			auto err = gnutls_x509_crq_export2(S._CertRequest, GNUTLS_X509_FMT_DER, &tmpBuffer);
			if(err < 0)
				throw Exception(error_t(err), "ERROR X509CertificateRequest::Copy(): Failed to move to buffer\n");

			err = gnutls_x509_crq_import(this->_CertRequest, &tmpBuffer, GNUTLS_X509_FMT_DER);
			if(err < 0)
				throw Exception(error_t(err), "ERROR X509CertificateRequest::Copy(): Failed to move from buffer\n");

			gnutls_free(tmpBuffer.data);
		}
		else
			this->Deinitialize();
	}

	X509Certificate X509CertificateRequest::CreateCertificate(serial_t Serial, time_t ActivationTime, time_t ExpiryTime)
	{
		X509Certificate newCert;

		auto err = gnutls_x509_crt_set_crq(newCert._CertificateData, this->_CertRequest);
		if(err < 0)
			throw Exception(error_t(err), "ERROR X509CertificateRequest::CreateCertificate(): Failed to create cert from request: "+ string(gnutls_strerror(err)) +"\n");

		auto tmpSerial = Serial.get();
		err = gnutls_x509_crt_set_serial(newCert._CertificateData, &tmpSerial, sizeof(tmpSerial));
		if(err < 0)
			throw Exception(error_t(err), "ERROR X509CertificateRequest::CreateCertificate(): Failed to set serial\n");

		err = gnutls_x509_crt_set_activation_time(newCert._CertificateData, ActivationTime);
		if(err < 0)
			throw Exception(error_t(err), "ERROR X509CertificateRequest::CreateCertificate(): Failed to set activation time\n");

		err = gnutls_x509_crt_set_expiration_time(newCert._CertificateData, ExpiryTime);
		if(err < 0)
			throw Exception(error_t(err), "ERROR X509CertificateRequest::CreateCertificate(): Failed to set expiration time\n");

		return newCert;
	}

	X509CertificateID X509CertificateID::ImportFromCertificate(const X509Certificate &Certificate, gnutls_digest_algorithm_t Algorithm)
	{
		X509CertificateID retID;

		size_t bufferSize = retID.IDLength;
		//this->_UniqueID.resize(bufferSize);

		auto err = gnutls_x509_crt_get_fingerprint(const_cast<X509Certificate &>(Certificate), Algorithm, reinterpret_cast<unsigned char*>(&(retID._UniqueID.front())), &bufferSize);
		if (err == GNUTLS_E_SHORT_MEMORY_BUFFER)
		{
			// If buffer was too small, throw exception
			throw Exception(error_t(err), "ERROR X509CertificateID::X509CertificateID(): Buffer too small for getting unique ID\n");

			// If buffer was too small, resize it and try again (gnutls_x509_crt_get_key_id set bufferSize to the size of the ID)
			//this->_UniqueID.resize(bufferSize);

			//err = gnutls_x509_crt_get_key_id(const_cast<X509Certificate &>(Certificate), Algorithm, reinterpret_cast<unsigned char*>(&(this->_UniqueID.front())), &bufferSize);
		}

		if(err < 0)
			throw Exception(error_t(err), "ERROR X509CertificateID::X509CertificateID(): Failed to get unique ID\n");

		// Resize to id size (gnutls_x509_crt_get_key_id set bufferSize to the size of the ID)
		//this->_UniqueID.resize(bufferSize);

		return retID;
	}

	X509CertificateID X509CertificateID::ImportFromHexString(const string &HexString)
	{
		return X509CertificateID(X509ID::ImportFromHexString(HexString));
	}

	X509CertificateID X509CertificateID::ImportFromIDArray(id_array_t &&IDArray)
	{
		return X509CertificateID(X509ID::ImportFromIDArray(std::move(IDArray)));
	}

	bool X509CertificateID::operator==(const X509CertificateID &Other) const noexcept
	{
		return (static_cast<const X509ID&>(*this) == static_cast<const X509ID&>(Other));
	}

	bool X509CertificateID::operator!=(const X509CertificateID &Other) const noexcept
	{
		return (static_cast<const X509ID&>(*this) != static_cast<const X509ID&>(Other));
	}

	X509CertificateID::X509CertificateID(X509ID &&ID)
		: X509ID(std::move(ID))
	{}

	X509PublicKeyID X509PublicKeyID::ImportFromCertificate(const X509Certificate &Certificate, gnutls_keyid_flags_t Algorithm)
	{
		X509PublicKeyID retVal;

		size_t bufferSize = retVal.IDLength;
		//this->_UniqueID.resize(bufferSize);

		auto err = gnutls_x509_crt_get_key_id(const_cast<X509Certificate &>(Certificate), Algorithm, reinterpret_cast<unsigned char*>(&(retVal._UniqueID.front())), &bufferSize);
		if (err == GNUTLS_E_SHORT_MEMORY_BUFFER)
		{
			throw Exception(error_t(err), "ERROR X509CertificateID::X509CertificateID(): Buffer too small\n");

			// If buffer was too small, resize it and try again (gnutls_x509_crt_get_key_id set bufferSize to the size of the ID)
			//this->_UniqueID.resize(bufferSize);

			//err = gnutls_x509_crt_get_key_id(const_cast<X509Certificate &>(Certificate), Algorithm, reinterpret_cast<unsigned char*>(&(this->_UniqueID.front())), &bufferSize);
		}

		if(err < 0)
			throw Exception(error_t(err), "ERROR X509CertificateID::X509CertificateID(): Failed to get unique ID\n");

		// Resize to id size (gnutls_x509_crt_get_key_id set bufferSize to the size of the ID)
		//this->_UniqueID.resize(bufferSize);

		return retVal;
	}

	X509PublicKeyID X509PublicKeyID::ImportFromHexString(const string &HexString)
	{
		return X509PublicKeyID(X509ID::ImportFromHexString(HexString));
	}

	X509PublicKeyID X509PublicKeyID::ImportFromIDArray(id_array_t &&StringID) noexcept
	{
		return X509PublicKeyID(X509ID::ImportFromIDArray(std::move(StringID)));
	}

	bool X509PublicKeyID::operator==(const X509PublicKeyID &Other) const noexcept
	{
		return (static_cast<const X509ID&>(*this) == static_cast<const X509ID&>(Other));
	}

	bool X509PublicKeyID::operator!=(const X509PublicKeyID &Other) const noexcept
	{
		return (static_cast<const X509ID&>(*this) != static_cast<const X509ID&>(Other));
	}

	X509PublicKeyID::X509PublicKeyID(X509ID &&ID)
		: X509ID(std::move(ID))
	{}

//	void to_json(json &J, const X509CertificateID &CertID)
//	{
//		J = json{ JSONCertificateID, CertID.GetID() };
//	}

//	void from_json(const json &J, X509CertificateID &CertID)
//	{
//		CertID = X509CertificateID(J.at(JSONCertificateID).get<std::string>());
//	}
}


namespace crypto_x509_certificate
{
	class TestX509Certificate
	{
		public:

			static bool Testing();
	};

	bool TestX509Certificate::Testing()
	{
		try
		{
			const string testInputString("-----BEGIN CERTIFICATE-----MIIEtDCCA5ygAwIBAgICEAAwDQYJKoZIhvcNAQELBQAwdjELMAkGA1UEBhMCREUxEDAOBgNVBAgMB0JhdmFyaWExDzANBgNVBAcMBk11bmljaDERMA8GA1UECgwIWmVjaCBJbmMxDjAMBgNVBAMMBUludENBMSEwHwYJKoZIhvcNAQkBFhJtaWtlQG16ZWNobWFpci5jb20wIBcNMTYwNjI1MjEwMDQ0WhgPMjI5MDA0MDkyMTAwNDRaMH4xCzAJBgNVBAYTAkRFMRAwDgYDVQQIDAdCYXZhcmlhMQ8wDQYDVQQHDAZNdW5pY2gxETAPBgNVBAoMCFplY2ggSW5jMRYwFAYDVQQDDA1temVjaG1haXIuY29tMSEwHwYJKoZIhvcNAQkBFhJtaWtlQG16ZWNobWFpci5jb20wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC6Rhz0y9+98WZMzFk2NlQuI1henTFfbA7fAketHA8Mba3B4oZJkJmjiphSpJytfyO6P1HXcjSfZjv1VRRNiYjW/f0xXAmkCPGYx/rIgVLYAN24l/RT6mSwDxJ47fQvM7jxDH4Assv2IcxyxFHB+XFtXbeYZuvGY7NIYTpwSMPJ4MPz6eWYsUps5yUHS7szIgqRYEXjgH2iWOIlx5i0cWnHNQhe9Mk5KPzZs++WpiIsGp8fS32WxN8Jz1n2FWzkMCmJ1EfJSnLtR6my3KiliS2eS439KoPcByw7pdMKXuf7E3NxTIbz5od9ojy6a6Y4FnxBUCrOuvXW8DgP1qXdhesBAgMBAAGjggFAMIIBPDAJBgNVHRMEAjAAMBEGCWCGSAGG+EIBAQQEAwIGQDAzBglghkgBhvhCAQ0EJhYkT3BlblNTTCBHZW5lcmF0ZWQgU2VydmVyIENlcnRpZmljYXRlMB0GA1UdDgQWBBS8EthCOU6c5MthI3W7HC5c9n7TtTCBogYDVR0jBIGaMIGXgBR8QIFc4DetH6v1oZqV8dUED/ZDjKF7pHkwdzELMAkGA1UEBhMCREUxEDAOBgNVBAgMB0JhdmFyaWExDzANBgNVBAcMBk11bmljaDERMA8GA1UECgwIWmVjaCBJbmMxDzANBgNVBAMMBlJvb3RDQTEhMB8GCSqGSIb3DQEJARYSbWlrZUBtemVjaG1haXIuY29tggIQADAOBgNVHQ8BAf8EBAMCBaAwEwYDVR0lBAwwCgYIKwYBBQUHAwEwDQYJKoZIhvcNAQELBQADggEBALHVY3Zkmjs/7PBIH2Z4/nJLJVzqHBsjyH/eQI4Cp8O2FpaJd3CblY7Uo/yV0VeODV1L2i+S81ePS5Nvc8KSaSWhrNikSKFogB675WwC6H1ipjUWiH4g8hLH/pY50yWbjXHN3UTWgwAEUzfRX9APVhU84hZXHURPekUkdfO00LgoHKhSiW5RbIKYh0hiiGw87Q/bR1v/HqgJrM8M+YkAbf/odKV6VZpjKZIcMG9YFya1oFrggW0YlFPY3a/n+WWq2KPO6LTUA9k91zaYp/6Hu7XQgI0PobsokgtJF8DQkN22DQkfOmYJyN4WEsusEzmLiwA2geRMfbGOV0i24HZRiHo=-----END CERTIFICATE-----");

			// Test String Input
			X509Certificate testStringCert = X509Certificate::ImportFromPEMString(testInputString);
			if(X509CertificateID::ImportFromCertificate(testStringCert) == X509CertificateID())
				return 0;

			// Test String export
			auto testExportString = testStringCert.ExportToPEMString();
			if(testExportString.compare(testInputString) != 0)
				return 0;

			// Test comparison
			if(testStringCert != X509Certificate::ImportFromPEMString(testInputString) ||
					!(testStringCert == X509Certificate::ImportFromPEMString(testInputString)))
				return 0;

			auto testCopy = testStringCert;
			auto testMove = std::move(testStringCert);

			testCopy = testMove;

			if(testCopy != testMove)
				return 0;

			return 1;
		}
		catch(Exception e)
		{
			return 0;
		}
	}

	class TestX509CertificateID
	{
		public:

			static bool Testing();
	};

	bool TestX509CertificateID::Testing()
	{
		try
		{
			const string testInputString("-----BEGIN CERTIFICATE-----MIIEtDCCA5ygAwIBAgICEAAwDQYJKoZIhvcNAQELBQAwdjELMAkGA1UEBhMCREUxEDAOBgNVBAgMB0JhdmFyaWExDzANBgNVBAcMBk11bmljaDERMA8GA1UECgwIWmVjaCBJbmMxDjAMBgNVBAMMBUludENBMSEwHwYJKoZIhvcNAQkBFhJtaWtlQG16ZWNobWFpci5jb20wIBcNMTYwNjI1MjEwMDQ0WhgPMjI5MDA0MDkyMTAwNDRaMH4xCzAJBgNVBAYTAkRFMRAwDgYDVQQIDAdCYXZhcmlhMQ8wDQYDVQQHDAZNdW5pY2gxETAPBgNVBAoMCFplY2ggSW5jMRYwFAYDVQQDDA1temVjaG1haXIuY29tMSEwHwYJKoZIhvcNAQkBFhJtaWtlQG16ZWNobWFpci5jb20wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC6Rhz0y9+98WZMzFk2NlQuI1henTFfbA7fAketHA8Mba3B4oZJkJmjiphSpJytfyO6P1HXcjSfZjv1VRRNiYjW/f0xXAmkCPGYx/rIgVLYAN24l/RT6mSwDxJ47fQvM7jxDH4Assv2IcxyxFHB+XFtXbeYZuvGY7NIYTpwSMPJ4MPz6eWYsUps5yUHS7szIgqRYEXjgH2iWOIlx5i0cWnHNQhe9Mk5KPzZs++WpiIsGp8fS32WxN8Jz1n2FWzkMCmJ1EfJSnLtR6my3KiliS2eS439KoPcByw7pdMKXuf7E3NxTIbz5od9ojy6a6Y4FnxBUCrOuvXW8DgP1qXdhesBAgMBAAGjggFAMIIBPDAJBgNVHRMEAjAAMBEGCWCGSAGG+EIBAQQEAwIGQDAzBglghkgBhvhCAQ0EJhYkT3BlblNTTCBHZW5lcmF0ZWQgU2VydmVyIENlcnRpZmljYXRlMB0GA1UdDgQWBBS8EthCOU6c5MthI3W7HC5c9n7TtTCBogYDVR0jBIGaMIGXgBR8QIFc4DetH6v1oZqV8dUED/ZDjKF7pHkwdzELMAkGA1UEBhMCREUxEDAOBgNVBAgMB0JhdmFyaWExDzANBgNVBAcMBk11bmljaDERMA8GA1UECgwIWmVjaCBJbmMxDzANBgNVBAMMBlJvb3RDQTEhMB8GCSqGSIb3DQEJARYSbWlrZUBtemVjaG1haXIuY29tggIQADAOBgNVHQ8BAf8EBAMCBaAwEwYDVR0lBAwwCgYIKwYBBQUHAwEwDQYJKoZIhvcNAQELBQADggEBALHVY3Zkmjs/7PBIH2Z4/nJLJVzqHBsjyH/eQI4Cp8O2FpaJd3CblY7Uo/yV0VeODV1L2i+S81ePS5Nvc8KSaSWhrNikSKFogB675WwC6H1ipjUWiH4g8hLH/pY50yWbjXHN3UTWgwAEUzfRX9APVhU84hZXHURPekUkdfO00LgoHKhSiW5RbIKYh0hiiGw87Q/bR1v/HqgJrM8M+YkAbf/odKV6VZpjKZIcMG9YFya1oFrggW0YlFPY3a/n+WWq2KPO6LTUA9k91zaYp/6Hu7XQgI0PobsokgtJF8DQkN22DQkfOmYJyN4WEsusEzmLiwA2geRMfbGOV0i24HZRiHo=-----END CERTIFICATE-----");

			X509Certificate testCert = X509Certificate::ImportFromPEMString(testInputString);

			X509CertificateID testID = X509CertificateID::ImportFromCertificate(testCert);
			if(testID == X509CertificateID())
				return 0;

			// Test copy and move
			auto testCopy = testID;
			auto testMove = std::move(testID);

			// Test comparison
			if(!(testCopy == testMove) ||
					testCopy != testMove)
				return 0;

			testCopy = std::move(testMove);

			if(testCopy != testMove)
				return 0;

			return 1;
		}
		catch(Exception &e)
		{
			return 0;
		}
	}

	class TestX509CertificateRequest
	{
		public:
			static bool Testing();
	};

	bool TestX509CertificateRequest::Testing()
	{
		const auto testTime = 1000000000;

		try
		{
			X509PrivateKey testIssuerPrivKey	= X509PrivateKey::GenerateKey();
			X509PrivateKey testCertPrivKey		= X509PrivateKey::GenerateKey();

			X509CertificateRequest testRequest;

			testRequest.SetDNField(DN_COMMON_NAME, "TEST");
			testRequest.SetDNField(DN_COUNTRY, "");

			testRequest.SetCertVersion(X509_VERSION_1);
			testRequest.SetCertVersion(X509_VERSION_1);
			testRequest.SetCertVersion(X509_VERSION_3);

			auto testIssuer = testRequest.GenerateSelfSigned(testIssuerPrivKey, serial_t(1000), 0, testTime);

			testRequest.SetAndSignWithPrivateKey(testCertPrivKey);
			auto testCert = testRequest.SignRequest(testIssuer, testIssuerPrivKey, serial_t(1001), 0, testTime);

			auto testString = testRequest.ExportToPEMString();

			auto testCopy = testRequest;
			auto testMove = std::move(testRequest);

			testCopy = testMove;
			testCopy = std::move(testMove);

			return 1;
		}
		catch(Exception e)
		{
			return 0;
		}
	}

	class TestX509PublicKeyID
	{
		public:

			static bool Testing();
	};

	bool TestX509PublicKeyID::Testing()
	{
		try
		{
			const string testInputString("-----BEGIN CERTIFICATE-----MIIEtDCCA5ygAwIBAgICEAAwDQYJKoZIhvcNAQELBQAwdjELMAkGA1UEBhMCREUxEDAOBgNVBAgMB0JhdmFyaWExDzANBgNVBAcMBk11bmljaDERMA8GA1UECgwIWmVjaCBJbmMxDjAMBgNVBAMMBUludENBMSEwHwYJKoZIhvcNAQkBFhJtaWtlQG16ZWNobWFpci5jb20wIBcNMTYwNjI1MjEwMDQ0WhgPMjI5MDA0MDkyMTAwNDRaMH4xCzAJBgNVBAYTAkRFMRAwDgYDVQQIDAdCYXZhcmlhMQ8wDQYDVQQHDAZNdW5pY2gxETAPBgNVBAoMCFplY2ggSW5jMRYwFAYDVQQDDA1temVjaG1haXIuY29tMSEwHwYJKoZIhvcNAQkBFhJtaWtlQG16ZWNobWFpci5jb20wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC6Rhz0y9+98WZMzFk2NlQuI1henTFfbA7fAketHA8Mba3B4oZJkJmjiphSpJytfyO6P1HXcjSfZjv1VRRNiYjW/f0xXAmkCPGYx/rIgVLYAN24l/RT6mSwDxJ47fQvM7jxDH4Assv2IcxyxFHB+XFtXbeYZuvGY7NIYTpwSMPJ4MPz6eWYsUps5yUHS7szIgqRYEXjgH2iWOIlx5i0cWnHNQhe9Mk5KPzZs++WpiIsGp8fS32WxN8Jz1n2FWzkMCmJ1EfJSnLtR6my3KiliS2eS439KoPcByw7pdMKXuf7E3NxTIbz5od9ojy6a6Y4FnxBUCrOuvXW8DgP1qXdhesBAgMBAAGjggFAMIIBPDAJBgNVHRMEAjAAMBEGCWCGSAGG+EIBAQQEAwIGQDAzBglghkgBhvhCAQ0EJhYkT3BlblNTTCBHZW5lcmF0ZWQgU2VydmVyIENlcnRpZmljYXRlMB0GA1UdDgQWBBS8EthCOU6c5MthI3W7HC5c9n7TtTCBogYDVR0jBIGaMIGXgBR8QIFc4DetH6v1oZqV8dUED/ZDjKF7pHkwdzELMAkGA1UEBhMCREUxEDAOBgNVBAgMB0JhdmFyaWExDzANBgNVBAcMBk11bmljaDERMA8GA1UECgwIWmVjaCBJbmMxDzANBgNVBAMMBlJvb3RDQTEhMB8GCSqGSIb3DQEJARYSbWlrZUBtemVjaG1haXIuY29tggIQADAOBgNVHQ8BAf8EBAMCBaAwEwYDVR0lBAwwCgYIKwYBBQUHAwEwDQYJKoZIhvcNAQELBQADggEBALHVY3Zkmjs/7PBIH2Z4/nJLJVzqHBsjyH/eQI4Cp8O2FpaJd3CblY7Uo/yV0VeODV1L2i+S81ePS5Nvc8KSaSWhrNikSKFogB675WwC6H1ipjUWiH4g8hLH/pY50yWbjXHN3UTWgwAEUzfRX9APVhU84hZXHURPekUkdfO00LgoHKhSiW5RbIKYh0hiiGw87Q/bR1v/HqgJrM8M+YkAbf/odKV6VZpjKZIcMG9YFya1oFrggW0YlFPY3a/n+WWq2KPO6LTUA9k91zaYp/6Hu7XQgI0PobsokgtJF8DQkN22DQkfOmYJyN4WEsusEzmLiwA2geRMfbGOV0i24HZRiHo=-----END CERTIFICATE-----");

			X509Certificate testCert = X509Certificate::ImportFromPEMString(testInputString);

			X509PublicKeyID testID = X509PublicKeyID::ImportFromCertificate(testCert);
			if(testID == X509PublicKeyID())
				return 0;

			// Test copy and move
			auto testCopy = testID;
			auto testMove = std::move(testID);

			// Test comparison
			if(!(testCopy == testMove) ||
					testCopy != testMove)
				return 0;

			testCopy = std::move(testMove);

			if(testCopy != testMove)
				return 0;

			return 1;
		}
		catch(Exception &)
		{
			return 0;
		}
	}
}
