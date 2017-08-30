#ifndef HARDWARE_CONNECTION_OPTION_H
#define HARDWARE_CONNECTION_OPTION_H

/*! \file hardware_connection_option.h
 *  \brief Header for HWConn class
 */


#ifdef __linux__
#include "linux_network_socket.h"
#endif
#include "network_connection.h"
#include "hardware_protocols.h"
#include <string>
#include <array>
#include <memory>

/*!
 *  \brief Namespace for HWConn class
 */
namespace hardware_connection_option
{
#ifdef __linux__
	using linux_network_socket::ip_addr_t;
	using linux_network_socket::NetworkSocketTCPClient;
#endif

	using network_connection::NetworkConnection;

	using hardware_protocols::hardware_protocols_t;
	using hardware_protocols::HW_CONNECTION_TCP;
	using hardware_protocols::HW_CONNECTION_NUM;
	using hardware_protocols::HW_CONNECTION_UNDEFINED;

	using hardware_protocols::hardware_address_t;
	using hardware_protocols::HW_ADDRESS_IP;
	using hardware_address_t::HW_ADDRESS_NUM;
	using hardware_address_t::HW_ADDRESS_UNDEFINED;

	using std::string;
	using std::unique_ptr;
	using std::array;

	const string HWNetworkConnectionEnd = "/";

	static constexpr array<const char *const, HW_ADDRESS_NUM> AddressPrefixes =
	{
		{"<ip>://"}
	};

	class TestHardwareAddress;
	class TestHardwareAddress;

	class HardwareAddressAccess;

	class HardwareAddressString
	{
		public:
			using hardware_access_unique_ptr_t = unique_ptr<HardwareAddressAccess>;

			/*!
			 * \brief ImportString
			 */
			static HardwareAddressString ImportAddressString(string &&AddressString);

			bool operator==(const HardwareAddressString &S) const;
			bool operator!=(const HardwareAddressString &S) const;

			/*!
			 * \brief Get Type of stored string
			 */
			hardware_address_t GetType() const;

			/*!
			 * \brief Returns address string
			 */
			const string &GetString() const;

			/*!
			 *	\brief Returns access to the address
			 */
			hardware_access_unique_ptr_t GetAddressAccess() const;

			/*!
			 * \brief Determine Type that of address stored in this Address
			 */
			static hardware_address_t DetermineAddressType(const HardwareAddressString &Address);

		private:
			/*!
			 * \brief String containing this address data
			 */
			string _String;

			/*!
			 * \brief Determine Type that of address stored in this string
			 */
			static hardware_address_t DetermineAddressType(const string &AddressString);

			/*!
			 * \brief Constructor
			 */
			HardwareAddressString(string &&String);
	};

	/*!
	 *	\brief Basic class for any address
	 */
	class HardwareAddressAccess
	{
		public:
			using connection_t = unique_ptr<NetworkConnection>;

			virtual ~HardwareAddressAccess() = default;

			/*!
			 * \brief Converts option to HardwareAddress
			 */
			virtual HardwareAddressString ToHardwareAddressString() const = 0;

			/*!
			 *	\brief Opens a connection using this address and the given protocol
			 */
			virtual connection_t OpenConnection(hardware_protocols_t Protocol) const = 0;

			/*!
			 * \brief List of supported protocols
			 */
			//constexpr static const std::array<hardware_protocols_t, 1> SupportedProtocols{{HW_CONNECTION_TCP}};
		private:

			friend class TestHardwareAddress;
	};

	template<hardware_address_t>
	class HardwareAddressOption : public HardwareAddressAccess
	{
		public:
			static HardwareAddressOption ImportFromAddressString(const HardwareAddressString &AddressString)
			{ return HardwareAddressOption(AddressString); }

			virtual ~HardwareAddressOption() = default;

			HardwareAddressString ToHardwareAddressString() const;

			connection_t OpenConnection(hardware_protocols_t Protocol) const;

		private:
			/*!
			 * \brief Constructor
			 */
			HardwareAddressOption(const HardwareAddressString &Address);

			friend class TestHardwareAddress;
	};

	/*!	\class HardwareAddress<HW_ADDRESS_IP>
	 *	\brief Stores IP addresses
	 */
	template<>
	class HardwareAddressOption<HW_ADDRESS_IP> : public HardwareAddressAccess
	{
		public:
			static HardwareAddressOption ImportFromAddressString(const HardwareAddressString &AddressString);

			virtual ~HardwareAddressOption() = default;

			HardwareAddressString ToHardwareAddressString() const;

			connection_t OpenConnection(hardware_protocols_t Protocol) const;

		private:

			/*!
			 * \brief IP Address of this address
			 */
			ip_addr_t _IPAddress;

			static string GetAddressString(const string &StringAddress);

			HardwareAddressOption(const HardwareAddressString &Address);

			friend class TestHardwareAddress;
	};
} // namespace hardware_connection_option


#endif // HARDWARE_CONNECTION_OPTION_H
