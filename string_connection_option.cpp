#include "string_connection_option.h"
#include "error_exception.h"
#include <arpa/inet.h>
#include <netdb.h>

#ifdef __linux__
#include "linux_network_socket.h"
#endif

namespace string_connection_option
{
#ifdef __linux__
	using linux_network_socket::ip_addr_t;
	using linux_network_socket::ip_version;
	using linux_network_socket::IPV4;
	using linux_network_socket::IPV6;
#endif

	using error_exception::Exception;
	using error_exception::ERROR_NUM;


//	StringConnectionCertificateID::StringConnectionCertificateID(const byte_vector_t &CertData) : _UserID(this->GetUserString(CertData)), _GroupID(this->GetGroupString(CertData))
//	{}

//	StringConnectionCertificateID::StringConnectionCertificateID(const UserCertID &Group, const UserCertID &User) noexcept : _UserID(User), _GroupID(Group)
//	{}

//	string StringConnectionCertificateID::ToString() const noexcept
//	{
//		return this->_UserID.GetID() + "." + this->_GroupID.GetID();
//	}

//	string StringConnectionCertificateID::GetUserString(const string &CertData) const
//	{
//		auto userStartPos = CertData.find_last_of(HWNetworkConnectionEnd);
//		if(userStartPos == CertData.npos)
//			userStartPos = 0;
//		else
//			userStartPos++;

//		auto userCertLength = CertData.find(UserCertSeparator, userStartPos);
//		if(userCertLength == CertData.npos)
//			userCertLength = CertData.length() - userStartPos;

//		return CertData.substr(userStartPos, userCertLength);
//	}

//	string StringConnectionCertificateID::GetGroupString(const string &CertData) const
//	{
//		auto groupStartPos = CertData.find_last_of(UserCertSeparator);
//		if(groupStartPos == CertData.npos)
//			throw Exception(ERROR_NUM, "ERROR StringConnectionCertificateID::GetGroupString(): Couldn't find separator\n");

//		groupStartPos++;

//		auto groupCertLength = CertData.find(UserCertSeparator, groupStartPos);
//		if(groupCertLength == CertData.npos)
//			groupCertLength = CertData.length() - groupStartPos;

//		return CertData.substr(groupStartPos, groupCertLength);
//	}

	connection_option_vector_t::iterator StringConnectionOptionVector::Find(const string &Option)
	{
		return this->connection_option_vector_t::Find<const string &>(Option, [](const connection_option_vector_t::value_type &Element, const string &CompVal) { return (Element.compare(CompVal) == 0); });
	}

	void StringConnectionOptionVector::CombineWithConnectionOptions(StringConnectionOptionVector &&ExtraStringOptions)
	{
		for(auto &curOptions : ExtraStringOptions)
		{
			// Add if not yet included
			if(this->Find(curOptions) == this->end())
			{
				this->push_back(std::move(curOptions));
			}
		}
	}
}
