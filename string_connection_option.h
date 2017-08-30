#ifndef STRING_CONNECTION_OPTION_H
#define STRING_CONNECTION_OPTION_H

/*! \file string_connection_option.h
 *  \brief Header for StringConnectionOption class
 */


#include <string>
#include "vector_t.h"
#include "hardware_connection_option.h"
#include "user_cert_id.h"
#include "error_exception.h"

#ifdef __linux__
#include "linux_network_socket.h"
#endif

/*!
 *  \brief Namespace for StringConnectionOption class
 */
namespace string_connection_option
{
#ifdef __linux__
	using linux_network_socket::ip_addr_t;
#endif

	using error_exception::Exception;

	using hardware_protocols::hardware_address_t;
	using hardware_protocols::hardware_protocols_t;

	using hardware_connection_option::HardwareAddressOption;
	using hardware_connection_option::HWNetworkConnectionEnd;

	using user_cert_id::UserCertID;

	using vector_t::byte_vector_t;
	using vector_t::vector_type;

	using std::string;

	const string JSONStringConnectionCertificate = "StringConnectionCertificate";
	const string JSONStringConnectionVector = "StringConnectionVector";
	const string UserCertSeparator = ".";

	/*!
	 * \brief ID that should be used for hardware connection
	 */
	class StringConnectionCertificateID
	{
		public:
			/*!
			 * \brief Constructor
			 * \param CertData Data as certificate
			 */
			StringConnectionCertificateID(const string &CertData);

			/*!
			 * \brief Constructor
			 * \param Group Group Certificate ID
			 * \param User User Certificate ID
			 */
			StringConnectionCertificateID(const UserCertID &Group, const UserCertID &User) noexcept;

			string ToString() const noexcept;

		private:
			UserCertID _UserID;
			UserCertID _GroupID;

			string GetUserString(const string &CertData) const;
			string GetGroupString(const string &CertData) const;
	};

	/*!	\class StringConnectionOption
	 *	\brief Stores connection option
	 */
	template<class T>
	class StringConnectionOption
	{
		public:
			/*!
			 * \brief Constructor
			 * \param ConnectionData Data in string format
			 */
			StringConnectionOption(const string &ConnectionData);

			/*!
			 * \brief Constructor
			 * \param HardwareConnection HW Connection data
			 * \param CertGroup Certificate group for HW connection
			 * \param CertUser Certificate user for HW connection
			 */
			StringConnectionOption(const T &HardwareConnection, const UserCertID &CertGroup, const UserCertID &CertUser) noexcept;

			/*!
			 * \brief GetHardwareConnection
			 * \return
			 */
			const T &GetHardwareConnection() const noexcept;

			/*!
			 * \brief GetCertificateData
			 * \return
			 */
			const StringConnectionCertificateID &GetCertificateData() const noexcept;

			/*!
			 * \brief ToString
			 * \return
			 */
			string ToString() const;

		private:
			/*!
			 * \brief Connection data
			 */
			T _HardwareConnection;

			/*!
			 * \brief Certificate Data
			 */
			StringConnectionCertificateID _CertificateData;
	};

	using connection_option_vector_t = vector_t::vector_t<string>;
	class StringConnectionOptionVector : public connection_option_vector_t
	{
		public:
			/*!
			 * \brief Constructor
			 */
			StringConnectionOptionVector() = default;

			/*!
			 * \brief Constructor
			 */
			StringConnectionOptionVector(const connection_option_vector_t &Options);

			/*!
			 * \brief Find
			 * \return
			 */
			connection_option_vector_t::iterator Find(const string &Option);

			void CombineWithConnectionOptions(StringConnectionOptionVector &&ExtraStringOptions);

			/*!	\class AddOption
			 *	\brief Adds the given Option as string to vector
			 */
			template<class T>
			void AddOption(const T &Option);

			/*!	\class GetOptionType
			 *	\brief Retrieves all connection options of a certain type
			 */
			template<hardware_address_t T>
			vector_type<HardwareAddressOption<T>> GetOptionType() const noexcept;

		private:
	};

	template<class T>
	StringConnectionOption<T>::StringConnectionOption(const string &ConnectionData) : _HardwareConnection(ConnectionData), _CertificateData(ConnectionData)
	{}

	template<class T>
	StringConnectionOption<T>::StringConnectionOption(const T &HardwareConnection, const UserCertID &CertGroup, const UserCertID &CertUser) noexcept : _HardwareConnection(HardwareConnection), _CertificateData(CertGroup, CertUser)
	{}

	template<class T>
	const T &StringConnectionOption<T>::GetHardwareConnection() const noexcept
	{
		return this->_HardwareConnection;
	}

	template<class T>
	const StringConnectionCertificateID &StringConnectionOption<T>::GetCertificateData() const noexcept
	{
		return this->_CertificateData;
	}

	template<class T>
	string StringConnectionOption<T>::ToString() const
	{
		return this->_HardwareConnection.ToString() + HWNetworkConnectionEnd + this->_CertificateData.ToString();
	}

	template<class T>
	void StringConnectionOptionVector::AddOption(const T &Option)
	{
		this->push_back(Option.ToString());
	}

	template<hardware_address_t T>
	vector_type<HardwareAddressOption<T>> StringConnectionOptionVector::GetOptionType() const noexcept
	{
		vector_type<HardwareAddressOption<T>> retVector;

		for(const auto &curOption : static_cast<const connection_option_vector_t &>(*this))
		{
			try
			{
				// Attempt to construct with correct type
				retVector.push_back(HardwareAddressOption<T>(curOption));
			}
			catch(Exception)
			{}
		}

		return retVector;
	}

} // namespace string_connection_option



// JSON Serialization
//namespace nlohmann
//{
//	template<class T>
//	struct adl_serializer<string_connection_option::StringConnectionOption<T>>
//	{
//		// note: the return type is no longer 'void', and the method only takes
//		// one argument
//		static string_connection_option::StringConnectionOption<T> from_json(const json &J)
//		{
//			return string_connection_option::StringConnectionOption<T>(J.at(string_connection_option::JSONStringConnectionCertificate));
//		}

//		// Here's the catch! You must provide a to_json method! Otherwise you
//		// will not be able to convert move_only_type to json, since you fully
//		// specialized adl_serializer on that type
//		static void to_json(json &J, const string_connection_option::StringConnectionOption<T> &O)
//		{
//			J = json{ string_connection_option::JSONStringConnectionCertificate, O.ToString() };
//		}
//	};

//	template<>
//	struct adl_serializer<string_connection_option::StringConnectionOptionVector>
//	{
//		// note: the return type is no longer 'void', and the method only takes
//		// one argument
//		static string_connection_option::StringConnectionOptionVector from_json(const json &J)
//		{
//			return string_connection_option::StringConnectionOptionVector(J.at(string_connection_option::JSONStringConnectionVector).get<string_connection_option::connection_option_vector_t>());
//		}

//		// Here's the catch! You must provide a to_json method! Otherwise you
//		// will not be able to convert move_only_type to json, since you fully
//		// specialized adl_serializer on that type
//		static void to_json(json &J, const string_connection_option::StringConnectionOptionVector &V)
//		{
//			J = json{ string_connection_option::JSONStringConnectionVector, json(static_cast<const string_connection_option::connection_option_vector_t&>(V)) };
//		}
//	};
//}

#endif // STRING_CONNECTION_OPTION_H
