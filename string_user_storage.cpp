#include "string_user_storage.h"

namespace string_user_storage
{
	using namespace error_exception;

	StringUser *StringUserStorage::FindUserInUserStorage(const StringUserID &UserID)
	{
		// Try to find user
		auto foundUserIterator = this->FindUserIteratorInUserStorage(UserID);

		// Check whether ID was found
		if(foundUserIterator == this->_StringUsers.end())
			return nullptr;
		else
			return &(*foundUserIterator);
	}

	const StringUser *StringUserStorage::FindUserInUserStorage(const StringUserID &UserID) const
	{
		// Try to find user
		auto foundUserIterator = this->FindUserIteratorInUserStorage(UserID);

		// Check whether ID was found
		if(foundUserIterator == this->_StringUsers.end())
			return nullptr;
		else
			return &(*foundUserIterator);
	}

	StringUser *StringUserStorage::FindUserEverywhere(const StringUserID &UserID)
	{
		auto *const pFindInUserStorage = this->FindUserInUserStorage(UserID);
		if(pFindInUserStorage == nullptr)
			return this->FindUserAdmin(UserID);
		else
			return pFindInUserStorage;
	}

	const StringUser *StringUserStorage::FindUserEverywhere(const StringUserID &UserID) const
	{
		const auto *const pFindInUserStorage = this->FindUserInUserStorage(UserID);
		if(pFindInUserStorage == nullptr)
			return this->FindUserAdmin(UserID);
		else
			return pFindInUserStorage;
	}

	const StringUser *StringUserStorage::FindUserCertOwner(const UserCertID &UserID) const
	{
		for(const auto &curStringUser : this->_StringUsers)
		{
			auto *const pFoundOwner = curStringUser.FindUserCert(UserID);
			if(pFoundOwner != nullptr)
				return &curStringUser;
		}

		for(const auto &curStringUser : this->_StringUserAdmins)
		{
			auto *const pFoundOwner = curStringUser.FindUserCert(UserID);
			if(pFoundOwner != nullptr)
				return &curStringUser;
		}

		return nullptr;
	}

	const StringUserAdmin *StringUserStorage::FindAdminUserCertOwner(const UserCertID &UserID) const
	{
		for(const auto &curStringUser : this->_StringUserAdmins)
		{
			auto *const pFoundOwner = curStringUser.FindUserCertAdmin(UserID);
			if(pFoundOwner != nullptr)
				return &curStringUser;
		}

		return nullptr;
	}

	const UserCert *StringUserStorage::FindUserCert(const UserCertID &UserID) const
	{
		for(const auto &curStringUser : this->_StringUsers)
		{
			auto *const pUser = curStringUser.FindUserCert(UserID);
			if(pUser != nullptr)
				return pUser;
		}

		for(const auto &curStringUserAdmin : this->_StringUserAdmins)
		{
			auto *const pUser = curStringUserAdmin.FindUserCert(UserID);
			if(pUser != nullptr)
				return pUser;
		}

		return this->FindUserCertAdmin(UserID);
	}

	const UserCert *StringUserStorage::FindUserCert(const UserCertID &UserID, const StringUserID &StrUserID) const
	{
		const StringUser *const pCurStringUser = this->FindUserInUserStorage(StrUserID);
		if(pCurStringUser != nullptr)
			return pCurStringUser->FindUserCert(UserID);

		const StringUserAdmin *const pCurStringUserAdmin = this->FindUserAdmin(StrUserID);
		if(pCurStringUserAdmin != nullptr)
		{
			const auto *const pUserCert = pCurStringUserAdmin->FindUserCert(UserID);
			if(pUserCert != nullptr)
				return pUserCert;

			return this->FindUserCertAdmin(UserID, StrUserID);
		}

		return nullptr;
	}

	const UserCertAdmin *StringUserStorage::FindUserCertAdmin(const UserCertID &UserID) const
	{
		for(const auto &curStringUserAdmin : this->_StringUserAdmins)
		{
			auto *const pUser = curStringUserAdmin.FindUserCertAdmin(UserID);
			if(pUser != nullptr)
				return pUser;
		}

		return nullptr;
	}

	const UserCertAdmin *StringUserStorage::FindUserCertAdmin(const UserCertID &UserID, const StringUserID &StrUserID) const
	{
		const StringUserAdmin * const pCurStringUser = this->FindUserAdmin(StrUserID);
		if(pCurStringUser != nullptr)
			return pCurStringUser->FindUserCertAdmin(UserID);

		return nullptr;
	}

	StringUserAdmin *StringUserStorage::FindUserAdmin(const StringUserID &UserID)
	{
		// Try to find user
		auto foundUserIterator = this->FindUserAdminIterator(UserID);

		// Check whether ID was found
		if(foundUserIterator == this->_StringUserAdmins.end())
			return nullptr;
		else
			return &(*foundUserIterator);
	}

	const StringUserAdmin *StringUserStorage::FindUserAdmin(const StringUserID &UserID) const
	{
		// Try to find user
		auto foundUserIterator = this->FindUserAdminIterator(UserID);

		// Check whether ID was found
		if(foundUserIterator == this->_StringUserAdmins.end())
			return nullptr;
		else
			return &(*foundUserIterator);
	}

	StringUser *StringUserStorage::RegisterNewUser(const StringUserID &NewUserID)
	{
		return this->AddNewUser(StringUser(NewUserID));
	}

	StringUserAdmin *StringUserStorage::RegisterNewUserAdmin(UserCertAdmin &&AdminCertificate)
	{
		return this->AddNewUserAdmin(StringUserAdmin::CreateNewAdmin(std::move(AdminCertificate)));
	}

	StringUserAdmin *StringUserStorage::AddNewUserAdmin(StringUserAdmin &&NewUserAdmin)
	{
		// Make sure this ID isn't in use yet
		assert(this->FindUserAdminIterator(NewUserAdmin.GetMainID()) == this->_StringUserAdmins.end());

		// Search for user in StringUser vector
		auto foundUserIterator = this->FindUserIteratorInUserStorage(NewUserAdmin.GetMainID());

		if(foundUserIterator != this->_StringUsers.end())
		{
			// If it does exist, move it to admin vector
			this->_StringUserAdmins.push_back(StringUserAdmin::ImportUser(std::move(*foundUserIterator)));
			this->_StringUserAdmins.back().CombineWithStringUserAdmin(std::move(NewUserAdmin));

			this->_StringUsers.erase(foundUserIterator);
		}
		else
		{
			// Else simply add it to storage
			this->_StringUserAdmins.push_back(std::move(NewUserAdmin));
		}

		// Get new admin certificate and add new certificate to it
		return &(this->_StringUserAdmins.back());
	}

	StringUser *StringUserStorage::AddNewUser(StringUser &&NewUser)
	{
		// Search for user
		auto *pFoundUser = this->FindUserInUserStorage(NewUser.GetMainID());

		if(pFoundUser == nullptr)
		{
			// If it doesn't exist yet, check admin category
			auto *pFoundAdmin = this->FindUserAdmin(NewUser.GetMainID());
			if(pFoundAdmin != nullptr)
			{
				// Add new certificate
				pFoundAdmin->CombineWithStringUser(std::move(NewUser));

				// Return if found
				return static_cast<StringUser*>(pFoundAdmin);
			}
			else
			{
				// If not found in admin storage either, create and add it to standard _StringUsers
				this->_StringUsers.push_back(std::move(NewUser));

				return &(this->_StringUsers.back());
			}
		}
		else
		{
			// TODO: Combine if it already exists
			assert(0);

			return pFoundUser;
		}
	}

	X509CertificateChain StringUserStorage::GetValidationChain(const UserCertID &UserID, const StringUserID *StrID) const
	{
		// Find certificate
		const auto *const pCert = (StrID == nullptr ? this->FindUserCertAdmin(UserID) : this->FindUserCertAdmin(UserID, *StrID));
		if(pCert == nullptr)
			throw Exception(ERROR_NUM, "ERROR StringUserStorage::GetAdminCertAndValidationChain(): Couldn't find requested UserCertAdmin\n");

		X509CertificateChain retChain;
		//retChain.AppendElement(UserCert(*pCert));

		// Find issuer chain
		auto issuerID = pCert->GetIssuer();
		while(issuerID != UserCertID())
		{
			const auto *const pIssuerCert = this->FindUserCert(issuerID);
			if(pIssuerCert == nullptr)
				throw Exception(ERROR_NUM, "ERROR StringUserStorage::GetAdminCertAndValidationChain(): Couldn't find requested issuer\n");

			retChain.AppendElement(static_cast<X509Certificate&&>(UserCert(*pIssuerCert)));

			issuerID = pIssuerCert->GetIssuer();
		}

		// Return chain
		return retChain;
	}

	user_admin_cert_chain_t StringUserStorage::GetAdminCertAndValidationChain(const UserCertID &AdminUserID, const StringUserID *StrAdminID) const
	{
		// Find certificate
		const auto *const pAdminCert = (StrAdminID == nullptr ? this->FindUserCertAdmin(AdminUserID) : this->FindUserCertAdmin(AdminUserID, *StrAdminID));
		if(pAdminCert == nullptr)
			throw Exception(ERROR_NUM, "ERROR StringUserStorage::GetAdminCertAndValidationChain(): Couldn't find requested UserCertAdmin\n");

		X509CertificateChain retChain;
		retChain.AppendElement(static_cast<X509Certificate&&>(UserCert(*pAdminCert)));

		// Find issuer chain
		auto issuerID = pAdminCert->GetIssuer();
		while(issuerID != UserCertID())
		{
			const auto *const pIssuerCert = this->FindUserCert(issuerID);
			if(pIssuerCert == nullptr)
				throw Exception(ERROR_NUM, "ERROR StringUserStorage::GetAdminCertAndValidationChain(): Couldn't find requested issuer\n");

			retChain.AppendElement(static_cast<X509Certificate&&>(UserCert(*pIssuerCert)));

			issuerID = pIssuerCert->GetIssuer();
		}

		// Return chain
		return user_admin_cert_chain_t{std::move(retChain), pAdminCert->GetKey()};
	}

	TLSCertificateCredentials StringUserStorage::GenerateCompleteTLSCredentials() const
	{
		TLSCertificateCredentials retVal;

		// Go through all StringUserAdmins
		for(const auto &curAdmin : this->_StringUserAdmins)
		{
			// Get all admins
			for(const auto &curAdminCert : curAdmin.GetAdminCertificates())
			{
				// If end cert, add chain
				if(curAdminCert.GetIssuer() == UserCertID())
				{
					auto curChain = this->GetAdminCertAndValidationChain(curAdminCert, &(curAdminCert.GetMainStringUserID()));

					retVal.AddKeyCredentials(std::move(curChain.CertChain), std::move(curChain.Key));
				}
			}

			// Get all users
			for(const auto &curCert : curAdmin.GetAdminCertificates())
			{
				// If end cert, add chain
				if(curCert.GetIssuer() == UserCertID())
				{
					auto curChain = this->GetValidationChain(curCert, &(curCert.GetMainStringUserID()));

					retVal.AddTrustCredentials(std::move(curChain));
				}
			}
		}

		// Go through all StringUsers
		for(const auto &curUser : this->_StringUsers)
		{
			// Get all users
			for(auto curCertIterator = curUser.GetUserCertBegin(); curCertIterator != curUser.GetUserCertEnd(); ++curCertIterator)
			{
				// If end cert, add chain
				if(curCertIterator->GetIssuer() == UserCertID())
				{
					auto curChain = this->GetValidationChain(curCertIterator->GetCertificateID(), &(curCertIterator->GetMainStringUserID()));

					retVal.AddTrustCredentials(std::move(curChain));
				}
			}
		}

		return retVal;
	}

	void StringUserStorage::CombineWithStorage(StringUserStorage &&ExtraStorage)
	{
		// Combine StringUsers
		for(auto &otherStringUserCert : ExtraStorage._StringUsers)
		{
			auto *const pInternalStringUser = this->FindUserInUserStorage(otherStringUserCert.GetMainID());
			if(pInternalStringUser == nullptr)
				pInternalStringUser->CombineWithStringUser(std::move(otherStringUserCert));
			else
			{
				// If not in _StringUsers check if it exists in _StringUserAdmins
				auto *const pInternalStringUserAdmin = this->FindUserAdmin(otherStringUserCert.GetMainID());
				if(pInternalStringUserAdmin == nullptr)
				{
					// If in neither category, just store it
					this->_StringUsers.push_back(std::move(otherStringUserCert));
				}
				else
				{
					// If in _StringUserAdmins, combine it
					pInternalStringUserAdmin->CombineWithStringUser(std::move(otherStringUserCert));
				}
			}
		}

		// Combine StringUserAdmins
		for(auto &otherStringUserAdmin : ExtraStorage._StringUserAdmins)
		{
			// Check if this is an ordinary user
			auto internalStringUserIterator = this->FindUserIteratorInUserStorage(otherStringUserAdmin.GetMainID());

			if(internalStringUserIterator != this->_StringUsers.end())
			{
				// Upgrade to admin if only StringUser
				this->UpgradeToStringUserAdmin(internalStringUserIterator, std::move(otherStringUserAdmin));
			}
			else
			{
				// Combine with existing StringUserAdmin
				auto internalStringUserAdminIterator = this->FindUserAdminIterator(otherStringUserAdmin.GetMainID());
				if(internalStringUserAdminIterator != this->_StringUserAdmins.end())
					internalStringUserAdminIterator->CombineWithStringUserAdmin(std::move(otherStringUserAdmin));
				else
					this->_StringUserAdmins.push_back(std::move(otherStringUserAdmin));
			}
		}
	}

	StringUserStorage::string_user_admin_vector_t &StringUserStorage::GetUserAdmins()
	{
		return this->_StringUserAdmins;
	}

	const StringUserStorage::string_user_admin_vector_t &StringUserStorage::GetUserAdmins() const
	{
		return this->_StringUserAdmins;
	}

	void StringUserStorage::Reset()
	{
		this->_StringUserAdmins.clear();
		this->_StringUsers.clear();
	}

	StringUserStorage::string_user_vector_t::iterator StringUserStorage::FindUserIteratorInUserStorage(const StringUserID &UserID)
	{
		return this->_StringUsers.Find<const StringUserID &>(UserID,
									[] (const string_user_vector_t::value_type &Element, const StringUserID &ID)
									{ return Element == ID; });
	}

	StringUserStorage::string_user_vector_t::const_iterator StringUserStorage::FindUserIteratorInUserStorage(const StringUserID &UserID) const
	{
		return this->_StringUsers.Find<const StringUserID &>(UserID,
									[] (const string_user_vector_t::value_type &Element, const StringUserID &ID)
									{ return Element == ID; });
	}

	StringUserStorage::string_user_admin_vector_t::iterator StringUserStorage::FindUserAdminIterator(const StringUserID &UserID)
	{
		return this->_StringUserAdmins.Find<const StringUserID &>(UserID,
											[] (const string_user_admin_vector_t::value_type &Element, const StringUserID &ID)
											{ return Element == ID; });
	}

	StringUserStorage::string_user_admin_vector_t::const_iterator StringUserStorage::FindUserAdminIterator(const StringUserID &UserID) const
	{
		return this->_StringUserAdmins.Find<const StringUserID &>(UserID,
											[] (const string_user_admin_vector_t::value_type &Element, const StringUserID &ID)
											{ return Element == ID; });
	}

	StringUserStorage::string_user_admin_vector_t::iterator StringUserStorage::UpgradeToStringUserAdmin(string_user_vector_t::iterator StringUserIterator, StringUserAdmin &&NewAdminData)
	{
		auto existingIterator = this->FindUserAdminIterator(StringUserIterator->GetMainID());

		// Combine with existing Admin if it exists
		if(existingIterator != this->_StringUserAdmins.end())
		{
			existingIterator->CombineWithStringUser(std::move(*StringUserIterator));
			existingIterator->CombineWithStringUserAdmin(std::move(NewAdminData));
		}
		else
		{
			this->_StringUserAdmins.push_back(std::move(NewAdminData));

			existingIterator = this->_StringUserAdmins.end()-1;

			existingIterator->CombineWithStringUser(std::move(*StringUserIterator));
		}

		// Erase old data
		this->_StringUsers.erase(StringUserIterator);

		return existingIterator;
	}
}
