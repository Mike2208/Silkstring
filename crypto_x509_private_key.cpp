#include "crypto_x509_private_key.h"
#include "error_exception.h"
#include <fstream>

namespace crypto_x509_private_key
{
	using error_exception::Exception;
	using error_exception::error_t;
	using error_exception::ERROR_NUM;

	using std::fstream;

	X509PrivateKey::X509PrivateKey()
	{
		this->Initialize();
	}

	X509PrivateKey X509PrivateKey::ImportFromPEMString(const string &ImportPEMKey)
	{
		X509PrivateKey retVal;

		const gnutls_datum_t tmpString{ const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(&(ImportPEMKey.front()))), static_cast<unsigned int>(ImportPEMKey.size()) };

		auto err = gnutls_x509_privkey_import(retVal._KeyData, &tmpString, GNUTLS_X509_FMT_PEM);
		if(err < 0)
			throw Exception(error_t(err), "ERROR X509PrivateKey::X509PrivateKey(string&): Failed to import PEM string\n");

		return retVal;
	}

	X509PrivateKey X509PrivateKey::GenerateKey(gnutls_pk_algorithm_t AlgorithmType, key_bits_t NumBits)
	{
		X509PrivateKey newKey;

		const auto err = gnutls_x509_privkey_generate(newKey._KeyData, AlgorithmType, NumBits.get(), 0);
		if(err < 0)
			throw Exception(error_t(err), "ERROR X509PrivateKey::GenerateKey(): Couldn't generate key\n");

		return newKey;
	}

	X509PrivateKey::X509PrivateKey(const X509PrivateKey &S)
	{
		this->Copy(S);
	}

	X509PrivateKey &X509PrivateKey::operator=(const X509PrivateKey &S)
	{
		this->Copy(S);

		return *this;
	}

	X509PrivateKey::X509PrivateKey(X509PrivateKey &&S) noexcept : _KeyData(std::move(S._KeyData))
	{
		S._KeyData = nullptr;
	}

	X509PrivateKey &X509PrivateKey::operator=(X509PrivateKey && S) noexcept
	{
		this->Deinitialize();

		this->_KeyData = std::move(S._KeyData);
		S._KeyData = nullptr;

		return *this;
	}

	X509PrivateKey::~X509PrivateKey() noexcept
	{
		this->Deinitialize();
	}

	string X509PrivateKey::ExportToPEMString() const
	{
		string buffer;
		size_t bufferSize = 10*1024;
		buffer.resize(bufferSize);

		auto err = gnutls_x509_privkey_export(this->_KeyData, GNUTLS_X509_FMT_PEM, &(buffer.front()), &bufferSize);
		if(err == GNUTLS_E_SHORT_MEMORY_BUFFER)
		{
			buffer.resize(bufferSize);
			err = gnutls_x509_privkey_export(this->_KeyData, GNUTLS_X509_FMT_PEM, &(buffer.front()), &bufferSize);
		}

		if(err < 0)
			throw Exception(error_t(err), "ERROR X509PrivateKey::ExportToPEMString(): Couldn't generate string\n");

		buffer.resize(bufferSize);

		return buffer;
	}

	void X509PrivateKey::ExportToPEMFile(const string &FileName)
	{
		// Save data to buffer
		gnutls_datum_t passwordBuffer {nullptr, 0};
		auto err = gnutls_x509_privkey_export2(this->_KeyData, GNUTLS_X509_FMT_PEM, &passwordBuffer);
		if(err < 0)
			throw Exception(error_t(err), "ERROR X509PrivateKey::ExportToPEMFile(): Failed to convert to PEM format\n");

		// Open file
		fstream keyFile(FileName, std::ios_base::out);
		if(!keyFile.is_open())
			throw Exception(error_t(err), "ERROR X509PrivateKey::ExportToPEMFile(): Failed to open file\n");

		// Write data to file
		keyFile.write(reinterpret_cast<const char*>(passwordBuffer.data), passwordBuffer.size);
		if(keyFile.fail())
			throw Exception(error_t(err), "ERROR X509PrivateKey::ExportToPEMFile(): Failed to write key to file\n");

		// Free buffer again
		gnutls_free(passwordBuffer.data);
	}

	X509PrivateKey::operator gnutls_x509_privkey_t() noexcept
	{
		return this->_KeyData;
	}

	X509PrivateKey::operator const gnutls_x509_privkey_t&() const noexcept
	{
		return this->_KeyData;
	}

	gnutls_x509_privkey_t X509PrivateKey::ReleaseOwnership() noexcept
	{
		auto tmp = this->_KeyData;
		this->_KeyData = nullptr;

		return tmp;
	}

	void X509PrivateKey::Deinitialize() noexcept
	{
		if(this->_KeyData != nullptr)
		{
			gnutls_x509_privkey_deinit(this->_KeyData);
			this->_KeyData = nullptr;
		}
	}

	void X509PrivateKey::Initialize()
	{
		if(this->_KeyData == nullptr)
		{
			auto err = gnutls_x509_privkey_init(&this->_KeyData);
			if(err < 0)
				throw Exception(error_t(err), "ERROR X509PrivateKey::Initialize(): Failed to allocate memory\n");
		}
	}

	void X509PrivateKey::Copy(const X509PrivateKey &S)
	{
		if(S._KeyData != nullptr)
		{
			this->Initialize();

			auto err = gnutls_x509_privkey_cpy(this->_KeyData, S._KeyData);
			if(err < 0)
				throw Exception(error_t(err), "ERROR X509PrivateKey::Copy(): Failed to copy key\n");
		}
		else
			this->Deinitialize();
	}

	X509PrivateKeyID X509PrivateKeyID::ImportFromPrivateKey(X509PrivateKey &PrivateKey, gnutls_keyid_flags_t Algorithm)
	{
		X509PrivateKeyID retVal;

		size_t bufferSize = retVal.IDLength;
		//this->_UniqueID.resize(bufferSize);

		auto err = gnutls_x509_privkey_get_key_id(PrivateKey, Algorithm, reinterpret_cast<unsigned char*>(&(retVal._UniqueID.front())), &bufferSize);
		if(err == GNUTLS_E_SHORT_MEMORY_BUFFER)
		{
			throw Exception(error_t(err), "ERROR X509PrivateKeyID::X509PrivateKeyID(): Buffer too small\n");

			// Retry with more space
			//this->_UniqueID.resize(bufferSize);

			//err = gnutls_x509_privkey_get_key_id(PrivateKey, Algorithm, reinterpret_cast<unsigned char*>(&(this->_UniqueID.front())), &bufferSize);
		}

		if(err < 0)
			throw Exception(error_t(err), "ERROR X509PrivateKeyID::X509PrivateKeyID(): Couldn't generate ID\n");

		//this->_UniqueID.resize(bufferSize);

		return retVal;
	}

	X509PrivateKeyID X509PrivateKeyID::ImportFromHexString(const string &HexString)
	{
		return X509PrivateKeyID(X509ID::ImportFromHexString(HexString));
	}

	X509PrivateKeyID::X509PrivateKeyID(X509ID &&ID)
		: X509ID(std::move(ID))
	{}
}

namespace crypto_x509_private_key
{
	class TestX509PrivateKey
	{
		public:
			static bool Testing();
	};

	bool TestX509PrivateKey::Testing()
	{
		try
		{
			X509PrivateKey testKey = X509PrivateKey::GenerateKey();

			auto testKeyString = testKey.ExportToPEMString();
			X509PrivateKey testStringInput = X509PrivateKey::ImportFromPEMString(testKeyString);

			// Test operators
			if(testKey != testStringInput ||
					!(testKey == testStringInput))
				return 0;

			// Test move and copy
			auto testCopy = testKey;
			auto testMove = std::move(testKey);

			if(testCopy != testMove)
				return 0;

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
