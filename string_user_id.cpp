#include "string_user_id.h"

namespace string_user_id
{
	StringUserID::StringUserID(const X509PublicKeyID &_StringUserID) : X509PublicKeyID(_StringUserID)
	{}

	bool StringUserID::operator==(const StringUserID &S) const noexcept
	{
		return static_cast<const X509PublicKeyID&>(*this) == static_cast<const X509PublicKeyID&>(S);
	}

	bool StringUserID::operator!=(const StringUserID &S) const noexcept
	{
		return static_cast<const X509PublicKeyID&>(*this) != static_cast<const X509PublicKeyID&>(S);
	}
}
