#include "string_user_admin.h"
#include "tls_connection.h"

namespace string_user_admin
{
	using namespace error_exception;

	using tls_connection::TLSConnection;

	using string_connection_option::StringConnectionOptionVector;

	using linux_network_socket::NetworkSocketTCPClient;

	using crypto_x509_certificate::serial_t;
	using crypto_x509_certificate::DN_COMMON_NAME;
	using crypto_x509_certificate::X509PublicKeyID;

	UserCertAdmin::UserCertAdmin(UserCert &&_CertificateData, X509CertificateRequest &&Request, X509PrivateKey &&_PrivateKey) : UserCert(std::move(_CertificateData)), _Request(std::move(Request)), _CertificateKey(_PrivateKey)
	{}

	UserCertAdmin UserCertAdmin::GenerateNewUserCertAdmin(const string &SubjectField, const time_t ExpirationTime)
	{
		// Create new private key
		X509PrivateKey stringUserAdminKey = X509PrivateKey::GenerateKey();

		// Create new request
		X509CertificateRequest stringUserAdminRequest;
		stringUserAdminRequest.SetDNField(DN_COMMON_NAME, SubjectField);
		stringUserAdminRequest.SetAndSignWithPrivateKey(stringUserAdminKey);

		// Create self-signed certificate
		X509Certificate stringUserAdminCert = stringUserAdminRequest.GenerateSelfSigned(stringUserAdminKey, serial_t(1000), 0, ExpirationTime);

		// Get Main ID
		StringUserID stringUserAdminMainID(X509PublicKeyID::ImportFromCertificate(stringUserAdminCert));

		// Return UserCertAdmin struct
		return UserCertAdmin(UserCert(stringUserAdminCert, stringUserAdminMainID, UserCertID()), std::move(stringUserAdminRequest), std::move(stringUserAdminKey));
	}

	const X509CertificateRequest &UserCertAdmin::GetRequest() const
	{
		return this->_Request;
	}

	const X509PrivateKey &UserCertAdmin::GetKey() const
	{
		return this->_CertificateKey;
	}

	void UserCertAdmin::CombineWithUserCertAdmin(UserCertAdmin &&ExtraUserCertAdmin)
	{
		if(this->_CertificateKey != ExtraUserCertAdmin._CertificateKey ||
				this->_Request.ExportToPEMString().compare(ExtraUserCertAdmin.ExportToPEMString()) == 0)
			throw Exception(ERROR_NUM, "ERROR UserCertAdmin::CombineWithUserCertAdmin(): Admin certs not equal\n");

		this->CombineWithCert(std::move(static_cast<UserCert&>(ExtraUserCertAdmin)));
	}

	//StringUserAdmin::StringUserAdmin(const StringUserID &MainID)
	//	: StringUser(MainID)
	//{}

	//StringUserAdmin::StringUserAdmin(StringUser &&UserData)
	//	: StringUser(std::move(UserData))
	//{}

	StringUserAdmin StringUserAdmin::GenerateNewAdmin(const string &SubjectField, const time_t ExpirationTime)
	{
		return StringUserAdmin(UserCertAdmin::GenerateNewUserCertAdmin(SubjectField, ExpirationTime));
	}

	StringUserAdmin StringUserAdmin::CreateNewAdmin(UserCertAdmin &&NewAdmin)
	{
		return StringUserAdmin(std::move(NewAdmin));
	}

	StringUserAdmin StringUserAdmin::ImportUser(StringUser &&User)
	{
		return StringUserAdmin(std::move(User));
	}

	UserCert *StringUserAdmin::FindUserCert(const UserCertID &CertID)
	{
		// Check both _UserCerts and _UserCertAdmins
		auto *const pFoundUserCert = this->StringUser::FindUserCert(CertID);
		if(pFoundUserCert == nullptr)
			return this->FindUserCertAdmin(CertID);
		else
			return pFoundUserCert;
	}

	const UserCert *StringUserAdmin::FindUserCert(const UserCertID &CertID) const
	{
		// Check both _UserCerts and _UserCertAdmins
		auto *const pFoundUserCert = this->StringUser::FindUserCert(CertID);
		if(pFoundUserCert == nullptr)
			return this->FindUserCertAdmin(CertID);
		else
			return pFoundUserCert;
	}

	UserCertAdmin *StringUserAdmin::FindUserCertAdmin(const UserCertID &CertID)
	{
		const auto foundIterator = this->FindUserCertAdminIterator(CertID);

		if(foundIterator == this->_UserCertAdmins.end())
			return nullptr;

		return foundIterator.base();
	}

	const UserCertAdmin *StringUserAdmin::FindUserCertAdmin(const UserCertID &CertID) const
	{
		const auto foundIterator = this->FindUserCertAdminIterator(CertID);

		if(foundIterator == this->_UserCertAdmins.end())
			return nullptr;

		return foundIterator.base();
	}

	void StringUserAdmin::AddUserCert(UserCert &&NewCert) noexcept
	{
		// Make sure NewCert is neither in _Usercerts nor in _UserCertAdmins
		assert(this->FindUserCert(NewCert.GetCertificateID()) == nullptr);

		if(this->FindUserCert(NewCert.GetCertificateID()) == nullptr)
			this->_UserCerts.push_back(std::move(NewCert));
	}

	void StringUserAdmin::AddUserCertAdmin(UserCertAdmin &&NewAdmin)
	{
		assert(StringUserAdmin::FindUserCertAdmin(NewAdmin.GetCertificateID()) == nullptr);

		// TODO: Modify old UserCert if one exists with same ID
		this->_UserCertAdmins.push_back(std::move(NewAdmin));
	}

	void StringUserAdmin::CombineWithStringUser(StringUser &&ExtraStringUser)
	{
		// Make sure both have same ID
		assert(this->GetMainID() == ExtraStringUser.GetMainID());

		// Combine with both _UserCerts and _UserCertAdmins
		this->CombineUserCerts(std::move(ExtraStringUser._UserCerts));

		// Use standard combination for rest
		this->StringUser::CombineWithStringUser(std::move(ExtraStringUser));
	}

	void StringUserAdmin::CombineWithStringUserAdmin(StringUserAdmin &&ExtraStringUserAdmin)
	{
		// Combine string user
		this->CombineWithStringUser(std::move(static_cast<StringUser&>(ExtraStringUserAdmin)));

		// Combine UserCertAdmin
		this->CombineUserCertAdmins(std::move(ExtraStringUserAdmin._UserCertAdmins));
	}

	user_cert_admin_vector_t &StringUserAdmin::GetAdminCertificates()
	{
		return this->_UserCertAdmins;
	}

	const user_cert_admin_vector_t &StringUserAdmin::GetAdminCertificates() const
	{
		return this->_UserCertAdmins;
	}

	UserCertIterator StringUserAdmin::GetUserCertBegin() noexcept
	{
		return UserCertIterator(&(this->_UserCerts), &(this->_UserCertAdmins), 0);
	}

	UserCertIterator StringUserAdmin::GetUserCertEnd() noexcept
	{
		return UserCertIterator(&(this->_UserCerts), &(this->_UserCertAdmins), this->_UserCertAdmins.size() + this->_UserCerts.size());
	}

	ConstUserCertIterator StringUserAdmin::GetUserCertBegin() const noexcept
	{
		return ConstUserCertIterator(&(this->_UserCerts), &(this->_UserCertAdmins), 0);
	}

	ConstUserCertIterator StringUserAdmin::GetUserCertEnd() const noexcept
	{
		return ConstUserCertIterator(&(this->_UserCerts), &(this->_UserCertAdmins), this->_UserCertAdmins.size() + this->_UserCerts.size());
	}

	user_cert_admin_vector_t::iterator StringUserAdmin::FindUserCertAdminIterator(const UserCertID &ID)
	{
		return this->_UserCertAdmins.Find<const UserCertID &>(ID,
						[] (const user_cert_admin_vector_t::value_type &Element, const UserCertID &CompareID)
						{ return Element.GetCertificateID() == CompareID; });
	}

	user_cert_admin_vector_t::const_iterator StringUserAdmin::FindUserCertAdminIterator(const UserCertID &ID) const
	{
		return this->_UserCertAdmins.Find<const UserCertID &>(ID,
						[] (const user_cert_admin_vector_t::value_type &Element, const UserCertID &CompareID)
						{ return Element.GetCertificateID() == CompareID; });
	}

	void StringUserAdmin::CombineUserCertAndUserCertAdmin(UserCertAdmin &AdminCert, user_cert_vector_t::iterator UserCertIterator)
	{
		// Combine certificates
		AdminCert.CombineWithCert(std::move(*UserCertIterator));

		// Erase empty place from _UserCerts storage
		this->_UserCerts.erase(UserCertIterator);
	}

	void StringUserAdmin::CombineUserCerts(user_cert_vector_t &&ExtraUserCerts)
	{
		for(auto &curExtraCert : ExtraUserCerts)
		{
			// Check if in _UserCerts or in _UserCertAdmins
			auto *const pCurInternalUserCert = this->FindUserCert(curExtraCert.GetCertificateID());
			if(pCurInternalUserCert != nullptr)
			{
				// Combine if it already exists
				pCurInternalUserCert->CombineWithCert(std::move(curExtraCert));
			}
			else
				//Add otherwise
				this->AddUserCert(std::move(curExtraCert));
		}
	}

	void StringUserAdmin::CombineUserCertAdmins(user_cert_admin_vector_t &&ExtraUserCertAdmins)
	{
		for(auto &curExtraCert : ExtraUserCertAdmins)
		{
			// Check if in _UserCertAdmins
			auto curInternalIterator = this->FindUserCertAdminIterator(curExtraCert.GetCertificateID());
			if(curInternalIterator != this->_UserCertAdmins.end())
			{
				curInternalIterator->CombineWithUserCertAdmin(std::move(curExtraCert));
			}
			else
			{
				// If not in _UserCertAdmins, add it and then check for same certificate in _UserCerts
				this->_UserCertAdmins.push_back(std::move(curExtraCert));

				auto foundUserCertIterator = this->StringUser::FindUserCertIterator(curExtraCert.GetCertificateID());
				if(foundUserCertIterator != this->_UserCerts.end())
				{
					// Upgrade existing certificate with same ID
					this->CombineUserCertAndUserCertAdmin(this->_UserCertAdmins.back(), foundUserCertIterator);
				}
			}
		}
	}

	StringUserAdmin::StringUserAdmin(UserCertAdmin &&AdminCert)
		: StringUser(AdminCert.GetMainStringUserID()),
		  _UserCertAdmins{std::move(AdminCert)}
	{}

	StringUserAdmin::StringUserAdmin(StringUser &&User)
		: StringUser(std::move(User))
	{}
}
