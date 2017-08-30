#include "crypto_x509_id.h"
#include "error_exception.h"
#include <sstream>

namespace crypto_x509_id
{
	using error_exception::Exception;
	using error_exception::ERROR_NUM;

	constexpr decltype(X509ID::IDLength) X509ID::IDLength;
	constexpr decltype(X509ID::StringIDLength) X509ID::StringIDLength;

	X509ID X509ID::ImportFromIDArray(id_array_t &&IDArray) noexcept
	{
		return X509ID(std::move(IDArray));
	}

	X509ID X509ID::ImportFromHexString(const string &HexStringID)
	{
		return X509ID(X509ID::ConvertStringToByteVector(HexStringID));
	}

	const id_array_t &X509ID::GetID() const noexcept
	{
		return this->_UniqueID;
	}

	string X509ID::ConvertToString() const
	{
		string retString;
		retString.reserve(this->_UniqueID.size()*2);

		// Convert each element to hex
		char buffer[3];
		for(auto curChar : this->_UniqueID)
		{
			sprintf(buffer, "%02x", curChar);
			retString.append(buffer);
		}

		return retString;
	}

	bool X509ID::IsValidString(const string &StringToCheck)
	{
		// Check length
		if(StringToCheck.size() != X509ID::StringIDLength)
			return false;

		// Check that all characters are hex characters
		for(const unsigned char curChar : StringToCheck)
		{
			if(!std::isxdigit(curChar))
				return false;
		}

		return true;
	}

	bool X509ID::operator==(const X509ID &Other) const noexcept
	{
		auto iterator1 = this->_UniqueID.begin();
		auto iterator2 = Other._UniqueID.begin();

		while(1)
		{
			if(iterator1 == this->_UniqueID.end())
			{
				if(iterator2 == this->_UniqueID.end())
				{
					// All elements are equal, return true
					return true;
				}

				// Iterator2 continues but Iterator1 has stopped, so they are not equal
				return false;
			}

			if(iterator2 == this->_UniqueID.end())
			{
				// Iterator1 continues but Iterator2 has stopped, so they are not equal
				return false;
			}

			// Check whether characters are equal
			if( *iterator1 != *iterator2)
				return false;
		}
	}

	bool X509ID::operator!=(const X509ID &Other) const noexcept
	{
		return !(*this == Other);
	}

	bool X509ID::IsValid() const
	{
		if(*this == ErrorID)
			return 1;

		return 0;
	}

	id_array_t X509ID::ConvertStringToByteVector(const string &HexStringID)
	{
		if(HexStringID.length() == 0)
		{
			return ErrorIDBytes;
		}

		id_array_t retVal;

		// Check validity
		if(!X509ID::IsValidString(HexStringID))
			throw Exception(ERROR_NUM, "ERROR X509ID::X509ID(): Invalid ID length\n");

		// Size byte vector to proper length
		//retVal.resize(HexStringID.length()/2);

		// Go through string and convert to unsigned characters
		size_t curPos = 0;
		while(curPos < retVal.size())
		{
			retVal.at(curPos) = std::stoi(HexStringID.substr(curPos*2, 2), nullptr, 16);

			curPos++;
		}

		return retVal;
	}

	X509ID::X509ID(id_array_t &&ID) noexcept
		: _UniqueID(std::move(ID))
	{}

	X509ID::X509ID(const string &HexStringID)
		: _UniqueID(X509ID::ConvertStringToByteVector(HexStringID))
	{}
}

#include "crypto_x509_certificate.h"

namespace crypto_x509_id
{
	using namespace crypto_x509_certificate;

	class TestX509ID
	{
		public:
			static bool Testing();
	};

	bool TestX509ID::Testing()
	{
		try
		{
			const string mikeCertString	= "-----BEGIN CERTIFICATE-----MIIEtDCCA5ygAwIBAgICEAAwDQYJKoZIhvcNAQELBQAwdjELMAkGA1UEBhMCREUxEDAOBgNVBAgMB0JhdmFyaWExDzANBgNVBAcMBk11bmljaDERMA8GA1UECgwIWmVjaCBJbmMxDjAMBgNVBAMMBUludENBMSEwHwYJKoZIhvcNAQkBFhJtaWtlQG16ZWNobWFpci5jb20wIBcNMTYwNjI1MjEwMDQ0WhgPMjI5MDA0MDkyMTAwNDRaMH4xCzAJBgNVBAYTAkRFMRAwDgYDVQQIDAdCYXZhcmlhMQ8wDQYDVQQHDAZNdW5pY2gxETAPBgNVBAoMCFplY2ggSW5jMRYwFAYDVQQDDA1temVjaG1haXIuY29tMSEwHwYJKoZIhvcNAQkBFhJtaWtlQG16ZWNobWFpci5jb20wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC6Rhz0y9+98WZMzFk2NlQuI1henTFfbA7fAketHA8Mba3B4oZJkJmjiphSpJytfyO6P1HXcjSfZjv1VRRNiYjW/f0xXAmkCPGYx/rIgVLYAN24l/RT6mSwDxJ47fQvM7jxDH4Assv2IcxyxFHB+XFtXbeYZuvGY7NIYTpwSMPJ4MPz6eWYsUps5yUHS7szIgqRYEXjgH2iWOIlx5i0cWnHNQhe9Mk5KPzZs++WpiIsGp8fS32WxN8Jz1n2FWzkMCmJ1EfJSnLtR6my3KiliS2eS439KoPcByw7pdMKXuf7E3NxTIbz5od9ojy6a6Y4FnxBUCrOuvXW8DgP1qXdhesBAgMBAAGjggFAMIIBPDAJBgNVHRMEAjAAMBEGCWCGSAGG+EIBAQQEAwIGQDAzBglghkgBhvhCAQ0EJhYkT3BlblNTTCBHZW5lcmF0ZWQgU2VydmVyIENlcnRpZmljYXRlMB0GA1UdDgQWBBS8EthCOU6c5MthI3W7HC5c9n7TtTCBogYDVR0jBIGaMIGXgBR8QIFc4DetH6v1oZqV8dUED/ZDjKF7pHkwdzELMAkGA1UEBhMCREUxEDAOBgNVBAgMB0JhdmFyaWExDzANBgNVBAcMBk11bmljaDERMA8GA1UECgwIWmVjaCBJbmMxDzANBgNVBAMMBlJvb3RDQTEhMB8GCSqGSIb3DQEJARYSbWlrZUBtemVjaG1haXIuY29tggIQADAOBgNVHQ8BAf8EBAMCBaAwEwYDVR0lBAwwCgYIKwYBBQUHAwEwDQYJKoZIhvcNAQELBQADggEBALHVY3Zkmjs/7PBIH2Z4/nJLJVzqHBsjyH/eQI4Cp8O2FpaJd3CblY7Uo/yV0VeODV1L2i+S81ePS5Nvc8KSaSWhrNikSKFogB675WwC6H1ipjUWiH4g8hLH/pY50yWbjXHN3UTWgwAEUzfRX9APVhU84hZXHURPekUkdfO00LgoHKhSiW5RbIKYh0hiiGw87Q/bR1v/HqgJrM8M+YkAbf/odKV6VZpjKZIcMG9YFya1oFrggW0YlFPY3a/n+WWq2KPO6LTUA9k91zaYp/6Hu7XQgI0PobsokgtJF8DQkN22DQkfOmYJyN4WEsusEzmLiwA2geRMfbGOV0i24HZRiHo=-----END CERTIFICATE-----";
			const string intCertString	= "-----BEGIN CERTIFICATE-----MIID0TCCArmgAwIBAgICEAAwDQYJKoZIhvcNAQELBQAwdzELMAkGA1UEBhMCREUxEDAOBgNVBAgMB0JhdmFyaWExDzANBgNVBAcMBk11bmljaDERMA8GA1UECgwIWmVjaCBJbmMxDzANBgNVBAMMBlJvb3RDQTEhMB8GCSqGSIb3DQEJARYSbWlrZUBtemVjaG1haXIuY29tMCAXDTE2MDYyNTIxMDAzMFoYDzIyOTAwNDA5MjEwMDMwWjB2MQswCQYDVQQGEwJERTEQMA4GA1UECAwHQmF2YXJpYTEPMA0GA1UEBwwGTXVuaWNoMREwDwYDVQQKDAhaZWNoIEluYzEOMAwGA1UEAwwFSW50Q0ExITAfBgkqhkiG9w0BCQEWEm1pa2VAbXplY2htYWlyLmNvbTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALjzHSROGS3p+P8DQvPdAdI2nHRxD3bqwbw8YcMzVIMa9V6OhJAe2PMZPNpFoWIKn6UQLtO/Gssez+EWdjk7aoviSCjjidulOQK4TCZvCffX47aFLlWjO2+oZ4axxqqUudkTMByiPcmnFKXWszYS7Q6iEkGgh58IyISSDlgopLs4RxShRgS/cq8BLpz43y9M4w04J7rU/ZqKa9664UQP/QH2eP0jmPmqo4dFTcqO1Dp6dlNuOirvtKEgkG3HGdVcXGZBRdfBh7Jsm+HNDpxj867jMdlde/ioNgpKwgsEvYVaSqUdCOqxh6lPSKpjgJTAAxQrlan7Z7c92WeYPSKCpi8CAwEAAaNmMGQwHQYDVR0OBBYEFHxAgVzgN60fq/WhmpXx1QQP9kOMMB8GA1UdIwQYMBaAFCthtopuESjiCfBX4Q8smpaAzsXqMBIGA1UdEwEB/wQIMAYBAf8CAQAwDgYDVR0PAQH/BAQDAgGGMA0GCSqGSIb3DQEBCwUAA4IBAQDFNwSRUMlvVTd4nsAqtaDs4XzUKf0wP0yi5eOzOuLAwSf0oCs+VfGeJIGPDCAkNpwt2sM3tNjxKQFzI2e25RInJQ5e2kgQFQrN57rVxgbINYD7qpbcd0nv+UJptuigeTJ/sd7cvH3KfbQeZFi/cEIJ+MeKWK6rLrtXPxLjEd7rRanetAukxxwi+7aWoV33FFC/Gf0r6apbOpDX3beE8KRO9lydT4kA+jGn7lz7fkb7s+DFObwv7P8b8QVz3e9e6Soh4NBdocwOJqyF9+SgGfJZLPauW+ent92Nh56qOXgr30mS4s6sT9Rd5RzROdErNCuCRU/zRzt0bo24WTXYGBLp-----END CERTIFICATE-----";

			X509ID testIDMike = X509CertificateID::ImportFromCertificate(X509Certificate::ImportFromPEMString(mikeCertString));
			X509ID testIDInt = X509CertificateID::ImportFromCertificate(X509Certificate::ImportFromPEMString(intCertString));

			if(!testIDInt.IsValid())
				return 0;

			// Test Comparison
			if(!(testIDMike != testIDInt) ||
					testIDInt == testIDMike)
				return 0;

			const auto testString = testIDInt.ConvertToString();
			if(!X509ID::IsValidString(testString))
				return 0;

			auto testStringConverted = X509ID::ImportFromHexString(testString);
			if(testStringConverted != testIDInt)
				return 0;

			auto testCopy = testIDMike;
			auto testMove = std::move(testIDMike);

			testCopy = testMove;

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
