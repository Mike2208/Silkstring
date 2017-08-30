#include "user_cert_id.h"

namespace user_cert_id
{
	UserCertID UserCertID::ImportFromCertificate(const X509Certificate &Cert)
	{
		return UserCertID(X509CertificateID::ImportFromCertificate(Cert));
	}

	UserCertID UserCertID::ImportFromIDArray(id_array_t &&ID) noexcept
	{
		return UserCertID(X509CertificateID::ImportFromIDArray(std::move(ID)));
	}

	UserCertID::UserCertID(X509CertificateID &&ID) noexcept
		: X509CertificateID(std::move(ID))
	{}
}
