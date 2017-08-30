#include "crypto_x509_certificate_chain.h"
#include "error_exception.h"

namespace crypto_x509_certificate_chain
{
	using error_exception::Exception;
	using error_exception::error_t;
	using error_exception::ERROR_NUM;

	using vector_t::vector_type;

	X509CertificateChain X509CertificateChain::ImportFromPEMStringChain(const string &ImportPEMChain)
	{
		X509CertificateChain retVal;

		unsigned int bufferSize = 10;
		vector_type<gnutls_x509_crt_t> tmpBuffer(bufferSize, nullptr);

		const gnutls_datum_t stringPointer{ const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(&(ImportPEMChain.front()))), static_cast<unsigned int>(ImportPEMChain.size()) };

		auto err = gnutls_x509_crt_list_import(&(tmpBuffer.front()), &bufferSize, &stringPointer, GNUTLS_X509_FMT_PEM, 0);
		if(err == GNUTLS_E_SHORT_MEMORY_BUFFER)
		{
			tmpBuffer.resize(bufferSize, nullptr);
			err = gnutls_x509_crt_list_import(&(tmpBuffer.front()), &bufferSize, &stringPointer, GNUTLS_X509_FMT_PEM, 0);
		}

		if(err < 0)
			throw Exception(error_t(err), "ERROR X509CertificateChain::X509CertificateChain(string&): Couldn't import certificates\n");

		tmpBuffer.resize(bufferSize);

		retVal._CertificateChain.reserve(tmpBuffer.size());
		for(auto &curCert : tmpBuffer)
		{
			retVal._CertificateChain.push_back(X509Certificate::ImportGnuTLSCertificate(std::move(curCert)));
		}

		return retVal;
	}

	gnutls_cert_vector_t X509CertificateChain::GetCertVector(bool MoveOwnership)
	{
		gnutls_cert_vector_t retVector;
		retVector.reserve(this->_CertificateChain.size());

		for(auto &curCert : this->_CertificateChain)
		{
			if(MoveOwnership)
				retVector.push_back(curCert.ReleaseOwnership());
			else
				retVector.push_back(curCert);
		}

		if(MoveOwnership)
			this->_CertificateChain.clear();

		return retVector;
	}

	size_t X509CertificateChain::GetNumCerts() const
	{
		return this->_CertificateChain.size();
	}

	X509CertificateID X509CertificateChain::GetLowestCertID() const
	{
		if(this->_CertificateChain.empty())
			throw Exception(ERROR_NUM, "ERROR X509CertificateChain::GetLowestCertID(): Chain is empty\n");

		return X509CertificateID::ImportFromCertificate(this->_CertificateChain.front());
	}

	string X509CertificateChain::ExportToPEMString() const
	{
		string buffer;
		for(auto &curCert : this->_CertificateChain)
		{
			try
			{
				buffer += curCert.ExportToPEMString();
			}
			catch(Exception e)
			{
				throw Exception(e.GetErrorNumber(), "ERROR X509CertificateChain::ExportToPEMString(): Failed to generate string\n");
			}
		}

		return buffer;
	}

	gnutls_cert_vector_t X509CertificateChain::ReleaseOwnership()
	{
		return this->GetCertVector(true);
	}

	class TestX509CertificateChain
	{
		public:
			static bool Testing();
	};

	bool TestX509CertificateChain::Testing()
	{
		const string rootCertString	= "-----BEGIN CERTIFICATE-----MIID1jCCAr6gAwIBAgIJAJe+Leg8rKr3MA0GCSqGSIb3DQEBCwUAMHcxCzAJBgNVBAYTAkRFMRAwDgYDVQQIDAdCYXZhcmlhMQ8wDQYDVQQHDAZNdW5pY2gxETAPBgNVBAoMCFplY2ggSW5jMQ8wDQYDVQQDDAZSb290Q0ExITAfBgkqhkiG9w0BCQEWEm1pa2VAbXplY2htYWlyLmNvbTAgFw0xNjA2MjUyMTAwMTVaGA8yMjkwMDQwOTIxMDAxNVowdzELMAkGA1UEBhMCREUxEDAOBgNVBAgMB0JhdmFyaWExDzANBgNVBAcMBk11bmljaDERMA8GA1UECgwIWmVjaCBJbmMxDzANBgNVBAMMBlJvb3RDQTEhMB8GCSqGSIb3DQEJARYSbWlrZUBtemVjaG1haXIuY29tMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAx0mXgf/uIh0+oAdpKKDOFzcQyvTawsRu/ptEY+YBYTWLh6aoRzFckH+Dt+/WX/Zm4+KrRK7oKWcB14oNRmDoK4OfSTen2pI5LZRaDYtLbEyAJfpHBIy1muU7mW68ssSryaPXwi7EKobP0Y9re5PwyKfdnoB9dgUNf6UTpjwejD6BUjkLS+8j+P63T8J7TqX9D5wAArHKBiJVkAC4ANLleHlPf7XwBF27JVEysxgSviISJ36eeSTGob9ntpSA4LoZxks2E3C1KA62+UfEz+4CO0lV5j1ECUyrc3B6qER3EJ1qa8GO/sKPH9obl65vaFU//69AjiBznQ9LBMttYJt8dwIDAQABo2MwYTAdBgNVHQ4EFgQUK2G2im4RKOIJ8FfhDyyaloDOxeowHwYDVR0jBBgwFoAUK2G2im4RKOIJ8FfhDyyaloDOxeowDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMCAYYwDQYJKoZIhvcNAQELBQADggEBAD1F3UlS20Z3/CKWpWbTsE+Yt3DlE3emM+E0kfaJ6XeF26X/c+nhno/jG1TTC/BCm53BFJassJZpJdmVaxWfiJpesIjqSnj3VMD+Y1DmvokP3blcMReSot2MSJbVP77IfEvaTAu3CoEyVP8T6kqta8fU816YFYkUqQtmigc1Gt0IzZjr+AgVlNhWoysVB8BrM4FMLfY2OKkb97dP2UEyfONc2K6nntFXvIdm/1tcfutfuMbqVdOK42Oo4VIY+IBObMBVlH3xV4vN+wYcLlNwAyYmjG1ZSJ3dupa4dlpaLKVKXDVtsp4DqSj7A7L0lHIK2uxwaDiUuTmo+6LfnwVC/98=-----END CERTIFICATE-----";
		const string intCertString	= "-----BEGIN CERTIFICATE-----MIID0TCCArmgAwIBAgICEAAwDQYJKoZIhvcNAQELBQAwdzELMAkGA1UEBhMCREUxEDAOBgNVBAgMB0JhdmFyaWExDzANBgNVBAcMBk11bmljaDERMA8GA1UECgwIWmVjaCBJbmMxDzANBgNVBAMMBlJvb3RDQTEhMB8GCSqGSIb3DQEJARYSbWlrZUBtemVjaG1haXIuY29tMCAXDTE2MDYyNTIxMDAzMFoYDzIyOTAwNDA5MjEwMDMwWjB2MQswCQYDVQQGEwJERTEQMA4GA1UECAwHQmF2YXJpYTEPMA0GA1UEBwwGTXVuaWNoMREwDwYDVQQKDAhaZWNoIEluYzEOMAwGA1UEAwwFSW50Q0ExITAfBgkqhkiG9w0BCQEWEm1pa2VAbXplY2htYWlyLmNvbTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALjzHSROGS3p+P8DQvPdAdI2nHRxD3bqwbw8YcMzVIMa9V6OhJAe2PMZPNpFoWIKn6UQLtO/Gssez+EWdjk7aoviSCjjidulOQK4TCZvCffX47aFLlWjO2+oZ4axxqqUudkTMByiPcmnFKXWszYS7Q6iEkGgh58IyISSDlgopLs4RxShRgS/cq8BLpz43y9M4w04J7rU/ZqKa9664UQP/QH2eP0jmPmqo4dFTcqO1Dp6dlNuOirvtKEgkG3HGdVcXGZBRdfBh7Jsm+HNDpxj867jMdlde/ioNgpKwgsEvYVaSqUdCOqxh6lPSKpjgJTAAxQrlan7Z7c92WeYPSKCpi8CAwEAAaNmMGQwHQYDVR0OBBYEFHxAgVzgN60fq/WhmpXx1QQP9kOMMB8GA1UdIwQYMBaAFCthtopuESjiCfBX4Q8smpaAzsXqMBIGA1UdEwEB/wQIMAYBAf8CAQAwDgYDVR0PAQH/BAQDAgGGMA0GCSqGSIb3DQEBCwUAA4IBAQDFNwSRUMlvVTd4nsAqtaDs4XzUKf0wP0yi5eOzOuLAwSf0oCs+VfGeJIGPDCAkNpwt2sM3tNjxKQFzI2e25RInJQ5e2kgQFQrN57rVxgbINYD7qpbcd0nv+UJptuigeTJ/sd7cvH3KfbQeZFi/cEIJ+MeKWK6rLrtXPxLjEd7rRanetAukxxwi+7aWoV33FFC/Gf0r6apbOpDX3beE8KRO9lydT4kA+jGn7lz7fkb7s+DFObwv7P8b8QVz3e9e6Soh4NBdocwOJqyF9+SgGfJZLPauW+ent92Nh56qOXgr30mS4s6sT9Rd5RzROdErNCuCRU/zRzt0bo24WTXYGBLp-----END CERTIFICATE-----";
		const string mikeCertString	= "-----BEGIN CERTIFICATE-----MIIEtDCCA5ygAwIBAgICEAAwDQYJKoZIhvcNAQELBQAwdjELMAkGA1UEBhMCREUxEDAOBgNVBAgMB0JhdmFyaWExDzANBgNVBAcMBk11bmljaDERMA8GA1UECgwIWmVjaCBJbmMxDjAMBgNVBAMMBUludENBMSEwHwYJKoZIhvcNAQkBFhJtaWtlQG16ZWNobWFpci5jb20wIBcNMTYwNjI1MjEwMDQ0WhgPMjI5MDA0MDkyMTAwNDRaMH4xCzAJBgNVBAYTAkRFMRAwDgYDVQQIDAdCYXZhcmlhMQ8wDQYDVQQHDAZNdW5pY2gxETAPBgNVBAoMCFplY2ggSW5jMRYwFAYDVQQDDA1temVjaG1haXIuY29tMSEwHwYJKoZIhvcNAQkBFhJtaWtlQG16ZWNobWFpci5jb20wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC6Rhz0y9+98WZMzFk2NlQuI1henTFfbA7fAketHA8Mba3B4oZJkJmjiphSpJytfyO6P1HXcjSfZjv1VRRNiYjW/f0xXAmkCPGYx/rIgVLYAN24l/RT6mSwDxJ47fQvM7jxDH4Assv2IcxyxFHB+XFtXbeYZuvGY7NIYTpwSMPJ4MPz6eWYsUps5yUHS7szIgqRYEXjgH2iWOIlx5i0cWnHNQhe9Mk5KPzZs++WpiIsGp8fS32WxN8Jz1n2FWzkMCmJ1EfJSnLtR6my3KiliS2eS439KoPcByw7pdMKXuf7E3NxTIbz5od9ojy6a6Y4FnxBUCrOuvXW8DgP1qXdhesBAgMBAAGjggFAMIIBPDAJBgNVHRMEAjAAMBEGCWCGSAGG+EIBAQQEAwIGQDAzBglghkgBhvhCAQ0EJhYkT3BlblNTTCBHZW5lcmF0ZWQgU2VydmVyIENlcnRpZmljYXRlMB0GA1UdDgQWBBS8EthCOU6c5MthI3W7HC5c9n7TtTCBogYDVR0jBIGaMIGXgBR8QIFc4DetH6v1oZqV8dUED/ZDjKF7pHkwdzELMAkGA1UEBhMCREUxEDAOBgNVBAgMB0JhdmFyaWExDzANBgNVBAcMBk11bmljaDERMA8GA1UECgwIWmVjaCBJbmMxDzANBgNVBAMMBlJvb3RDQTEhMB8GCSqGSIb3DQEJARYSbWlrZUBtemVjaG1haXIuY29tggIQADAOBgNVHQ8BAf8EBAMCBaAwEwYDVR0lBAwwCgYIKwYBBQUHAwEwDQYJKoZIhvcNAQELBQADggEBALHVY3Zkmjs/7PBIH2Z4/nJLJVzqHBsjyH/eQI4Cp8O2FpaJd3CblY7Uo/yV0VeODV1L2i+S81ePS5Nvc8KSaSWhrNikSKFogB675WwC6H1ipjUWiH4g8hLH/pY50yWbjXHN3UTWgwAEUzfRX9APVhU84hZXHURPekUkdfO00LgoHKhSiW5RbIKYh0hiiGw87Q/bR1v/HqgJrM8M+YkAbf/odKV6VZpjKZIcMG9YFya1oFrggW0YlFPY3a/n+WWq2KPO6LTUA9k91zaYp/6Hu7XQgI0PobsokgtJF8DQkN22DQkfOmYJyN4WEsusEzmLiwA2geRMfbGOV0i24HZRiHo=-----END CERTIFICATE-----";

		const auto rootCert = X509Certificate::ImportFromPEMString(rootCertString);
		const auto intCert = X509Certificate::ImportFromPEMString(intCertString);
		const auto mikeCert = X509Certificate::ImportFromPEMString(mikeCertString);

		try
		{
			X509CertificateChain testChain = X509CertificateChain::ImportCerts(mikeCert, intCert, rootCert);

			auto testVector = testChain.GetCertVector(0);
			(void)testVector;

			const auto testStringChain = testChain.ExportToPEMString();
			if(testStringChain.compare(mikeCertString) != 0 ||
					testStringChain.substr(mikeCertString.size(), intCertString.size()).compare(intCertString) != 0 ||
					testStringChain.substr(mikeCertString.size()+intCertString.size(), rootCertString.size()).compare(rootCertString) != 0)
				return 0;

			testChain.AppendElement(rootCert);

			if(testChain.GetLowestCertID() != X509CertificateID::ImportFromCertificate(mikeCert))
				return 0;

			auto testCopy = testChain;
			auto testMove = std::move(testChain);

			testCopy = testMove;
		}
		catch(Exception &)
		{
			return 0;
		}

		return 1;
	}
}
