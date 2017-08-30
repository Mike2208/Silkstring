#include "string_user.h"
#include "string_user_admin.h"
#include "error_exception.h"

namespace string_user
{
	using error_exception::Exception;
	using error_exception::ERROR_WARN;
	using error_exception::ERROR_NUM;

	UserCert::UserCert(X509Certificate Cert, StringUserID MainStringUserID, const UserCertID Issuer) noexcept
		: X509Certificate(Cert),
		  _IssuerID(Issuer),
		  _MainStringUserID(MainStringUserID)
	{}

	UserCert::operator UserCertID() const
	{
		return UserCertID::ImportFromCertificate(static_cast<const X509Certificate &>(*this));
	}

	UserCertID UserCert::GetCertificateID() const
	{
		return *this;
	}

	const UserCertID &UserCert::GetIssuer() const
	{
		return this->_IssuerID;
	}

	bool UserCert::HasIssuer() const
	{
		if(this->_IssuerID == UserCertID())
			return true;

		return false;
	}

	void UserCert::CombineWithCert(UserCert &&ExtraCert)
	{
		if(this->_IssuerID != ExtraCert._IssuerID ||
				this->_MainStringUserID != ExtraCert._MainStringUserID ||
				static_cast<X509Certificate&>(*this) != static_cast<X509Certificate&>(ExtraCert))
			throw Exception(ERROR_NUM, "UserCert::CombineWithCert(): Certs not same type\n");

		// Combine signed cert ID vectors
		for(auto &curExtraSignedID : ExtraCert._SignedCertificateIDs)
		{
			// Insert all new signed certificates
			if(this->_SignedCertificateIDs.Find(curExtraSignedID) == this->_SignedCertificateIDs.end())
				this->_SignedCertificateIDs.push_back(std::move(curExtraSignedID));
		}
	}

	const StringUserID &UserCert::GetMainStringUserID() const noexcept
	{
		return this->_MainStringUserID;
	}

	const UserCertIDVector &UserCert::GetSignedCertificateIDs() const noexcept
	{
		return this->_SignedCertificateIDs;
	}

	ConstUserCertIterator::ConstUserCertIterator(const user_cert_vector_t *UserCerts, const user_cert_admin_vector_t *UserCertAdmins, size_t CurPosInVectors)
		:	_CurPosInVectors(CurPosInVectors),
			_UserCerts(UserCerts),
			_UserCertAdmins(UserCertAdmins)
	{}

	ConstUserCertIterator &ConstUserCertIterator::operator++()
	{
		this->_CurPosInVectors++;

		return *this;
	}

	ConstUserCertIterator ConstUserCertIterator::operator++(int)
	{
		this->_CurPosInVectors++;

		return *this;
	}

	ConstUserCertIterator &ConstUserCertIterator::operator--()
	{
		this->_CurPosInVectors--;

		return *this;
	}

	ConstUserCertIterator ConstUserCertIterator::operator--(int)
	{
		this->_CurPosInVectors--;

		return *this;
	}

	bool ConstUserCertIterator::operator==(const ConstUserCertIterator &S) const
	{
		return (this->_CurPosInVectors	== S._CurPosInVectors &&
				this->_UserCertAdmins	== S._UserCertAdmins &&
				this->_UserCerts		== S._UserCerts);
	}

	bool ConstUserCertIterator::operator!=(const ConstUserCertIterator &S) const
	{
		return !(this->_CurPosInVectors	== S._CurPosInVectors &&
				this->_UserCertAdmins	== S._UserCertAdmins &&
				this->_UserCerts		== S._UserCerts);
	}

	const UserCert *ConstUserCertIterator::base() const
	{
		const auto userCertVectorSize = this->_UserCerts->size();
		if(this->_CurPosInVectors >= userCertVectorSize)
			return &(this->_UserCertAdmins->at(this->_CurPosInVectors-userCertVectorSize));

		return &(this->_UserCerts->at(this->_CurPosInVectors));
	}

	const UserCert *ConstUserCertIterator::operator*(int) const
	{
		return this->base();
	}

	const UserCert *ConstUserCertIterator::operator->() const
	{
		return this->base();
	}

	UserCertIterator::UserCertIterator(user_cert_vector_t *UserCerts, user_cert_admin_vector_t *UserCertAdmins, size_t CurPosInVectors)
		:	_CurPosInVectors(CurPosInVectors),
			_UserCerts(UserCerts),
			_UserCertAdmins(UserCertAdmins)
	{}

	UserCertIterator &UserCertIterator::operator++()
	{
		this->_CurPosInVectors++;

		return *this;
	}

	UserCertIterator UserCertIterator::operator++(int)
	{
		this->_CurPosInVectors++;

		return *this;
	}

	UserCertIterator &UserCertIterator::operator--()
	{
		this->_CurPosInVectors--;

		return *this;
	}

	UserCertIterator UserCertIterator::operator--(int)
	{
		this->_CurPosInVectors--;

		return *this;
	}

	bool UserCertIterator::operator==(const UserCertIterator &S) const
	{
		return (this->_CurPosInVectors	== S._CurPosInVectors &&
				this->_UserCertAdmins	== S._UserCertAdmins &&
				this->_UserCerts		== S._UserCerts);
	}

	bool UserCertIterator::operator!=(const UserCertIterator &S) const
	{
		return !(this->_CurPosInVectors	== S._CurPosInVectors &&
				this->_UserCertAdmins	== S._UserCertAdmins &&
				this->_UserCerts		== S._UserCerts);
	}

	UserCert *UserCertIterator::base()
	{
		const auto userCertVectorSize = this->_UserCerts->size();
		if(this->_CurPosInVectors >= userCertVectorSize)
			return &(this->_UserCertAdmins->at(this->_CurPosInVectors-userCertVectorSize));

		return &(this->_UserCerts->at(this->_CurPosInVectors));
	}

	const UserCert *UserCertIterator::base() const
	{
		const auto userCertVectorSize = this->_UserCerts->size();
		if(this->_CurPosInVectors >= userCertVectorSize)
			return &(this->_UserCertAdmins->at(this->_CurPosInVectors-userCertVectorSize));

		return &(this->_UserCerts->at(this->_CurPosInVectors));
	}

	UserCert *UserCertIterator::operator*(int)
	{
		return this->base();
	}

	const UserCert *UserCertIterator::operator*(int) const
	{
		return this->base();
	}

	const UserCert *UserCertIterator::operator->() const
	{
		return this->base();
	}

	UserCert *UserCertIterator::operator->()
	{
		return this->base();
	}

	UserCertIterator::operator ConstUserCertIterator() const noexcept
	{
		return ConstUserCertIterator(this->_UserCerts, this->_UserCertAdmins, this->_CurPosInVectors);
	}

	UserCert *StringUser::FindUserCert(const UserCertID &ID)
	{
		auto foundCert = this->FindUserCertIterator(ID);

		if(foundCert != this->_UserCerts.end())
			return foundCert.base();

		return nullptr;
	}

	const UserCert *StringUser::FindUserCert(const UserCertID &ID) const
	{
		const auto foundCert = this->FindUserCertIterator(ID);

		if(foundCert != this->_UserCerts.end())
			return foundCert.base();

		return nullptr;
	}

//	const user_cert_vector_t &StringUser::GetCertificates() const noexcept
//	{
//		return this->_UserCerts;
//	}

//	user_cert_vector_t &StringUser::GetCertificates() noexcept
//	{
//		return this->_UserCerts;
//	}

	const StringUserID &StringUser::GetMainID() const noexcept
	{
		return *this;
	}

	void StringUser::AddUserCert(UserCert &&NewCert) noexcept
	{
		assert(this->FindUserCert(NewCert.GetCertificateID()) == nullptr);
		if(this->FindUserCert(NewCert.GetCertificateID()) == nullptr)
			this->_UserCerts.push_back(std::move(NewCert));
	}

	void StringUser::RemoveUserCert(const UserCertID &IDToRemove) noexcept
	{
		auto foundCert = this->FindUserCertIterator(IDToRemove);
		if(foundCert != this->_UserCerts.end())
			this->_UserCerts.erase(foundCert);
	}

	void StringUser::CombineWithStringUser(StringUser &&ExtraStringUser)
	{
		// Assert both have same unique ID
		assert(this->GetMainID() == ExtraStringUser.GetMainID());
		if(this->GetMainID() != ExtraStringUser.GetMainID())
			throw Exception(ERROR_NUM, "ERROR StringUser::CombineWithStringUser(): MainIDs don't match \n");

		// Combine connection options
		this->_ConnectionOptions.CombineWithConnectionOptions(std::move(ExtraStringUser._ConnectionOptions));

		// Combine _UserCerts
		this->CombineUserCerts(std::move(ExtraStringUser._UserCerts));
	}

	StringUser::StringUser(StringUserID &&MainID) : StringUserID(std::move(MainID))
	{}

	StringUser::StringUser(const StringUserID &MainID) : StringUserID(MainID)
	{}

	const StringConnectionOptionVector &StringUser::GetConnections() const noexcept
	{
		return this->_ConnectionOptions;
	}

	StringConnectionOptionVector &StringUser::GetConnections() noexcept
	{
		return this->_ConnectionOptions;
	}

	ConstUserCertIterator StringUser::GetUserCertBegin() const noexcept
	{
		return ConstUserCertIterator(&(this->_UserCerts), nullptr, 0);
	}

	ConstUserCertIterator StringUser::GetUserCertEnd() const noexcept
	{
		return ConstUserCertIterator(&(this->_UserCerts), nullptr, this->_UserCerts.size());
	}

	UserCertIterator StringUser::GetUserCertBegin() noexcept
	{
		return UserCertIterator(&(this->_UserCerts), nullptr, 0);
	}

	UserCertIterator StringUser::GetUserCertEnd() noexcept
	{
		return UserCertIterator(&(this->_UserCerts), nullptr, this->_UserCerts.size());
	}

	user_cert_vector_t::iterator StringUser::FindUserCertIterator(const UserCertID &ID)
	{
		return this->_UserCerts.Find<const UserCertID &>(ID,
						[] (const user_cert_vector_t::value_type &Element, const UserCertID &CompareID)
						{ return Element.GetCertificateID() == CompareID; });
	}

	user_cert_vector_t::const_iterator StringUser::FindUserCertIterator(const UserCertID &ID) const
	{
		return this->_UserCerts.Find<const UserCertID &>(ID,
						[] (const user_cert_vector_t::value_type &Element, const UserCertID &CompareID)
						{ return Element.GetCertificateID() == CompareID; });
	}

	void StringUser::CombineUserCerts(user_cert_vector_t &&ExtraCerts)
	{
		for(auto &curExtraCert : ExtraCerts)
		{
			auto curInternalIterator = this->FindUserCertIterator(curExtraCert.GetCertificateID());
			if(curInternalIterator != this->_UserCerts.end())
			{
				curInternalIterator->CombineWithCert(std::move(curExtraCert));
			}
			else
				this->_UserCerts.push_back(std::move(curExtraCert));
		}
	}
}
