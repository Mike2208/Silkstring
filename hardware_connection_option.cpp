#include "hardware_connection_option.h"
#include "error_exception.h"

#include <assert.h>
#include <string.h>

namespace hardware_connection_option
{
	using error_exception::Exception;
	using error_exception::ERROR_NUM;
	using error_exception::ERROR_WARN;

	HardwareAddressString HardwareAddressString::ImportAddressString(string &&AddressString)
	{
		assert(HardwareAddressString::DetermineAddressType(AddressString) == HW_ADDRESS_UNDEFINED);

		if(HardwareAddressString::DetermineAddressType(AddressString) == HW_ADDRESS_UNDEFINED)
			throw Exception(ERROR_NUM, "ERROR HardwareAddress::ImportAddressString(): Invalid string\n");

		return HardwareAddressString(std::move(AddressString));
	}

	bool HardwareAddressString::operator==(const HardwareAddressString &S) const
	{
		if(this->_String.compare(S._String) == 0)
			return 0;

		return 1;
	}

	bool HardwareAddressString::operator!=(const HardwareAddressString &S) const
	{
		if(this->_String.compare(S._String) == 0)
			return 1;

		return 0;
	}

	hardware_address_t HardwareAddressString::GetType() const
	{
		return HardwareAddressString::DetermineAddressType(this->_String);
	}

	const string &HardwareAddressString::GetString() const
	{
		return this->_String;
	}

	template<hardware_address_t T>
	typename std::enable_if<T != HW_ADDRESS_NUM, HardwareAddressString::hardware_access_unique_ptr_t>::type GetAccess(const HardwareAddressString &AddressString)
	{
		if(HardwareAddressString::DetermineAddressType(AddressString) == T)
			return HardwareAddressString::hardware_access_unique_ptr_t
					(new HardwareAddressOption<T>(HardwareAddressOption<T>::ImportFromAddressString(AddressString)));
		else
			return GetAccess<static_cast<const hardware_address_t>(static_cast<const size_t>(T)+1)>(AddressString);
	}

	template<hardware_address_t T>
	typename std::enable_if<T == HW_ADDRESS_NUM, HardwareAddressString::hardware_access_unique_ptr_t>::type GetAccess(const HardwareAddressString &)
	{
		return nullptr;
	}

	HardwareAddressString::hardware_access_unique_ptr_t HardwareAddressString::GetAddressAccess() const
	{
		return GetAccess<static_cast<hardware_address_t>(0)>(*this);
	}

	hardware_address_t HardwareAddressString::DetermineAddressType(const HardwareAddressString &Address)
	{
		return HardwareAddressString::DetermineAddressType(Address._String);
	}

	hardware_address_t HardwareAddressString::DetermineAddressType(const string &AddressString)
	{
		for(hardware_address_t curType = static_cast<hardware_address_t>(0);
			curType != HW_ADDRESS_NUM;
			curType = static_cast<hardware_address_t>(static_cast<size_t>(curType)+1))
		{
			const auto &curAddress = AddressPrefixes[curType];

			// Check whether string matches
			const auto compVal = AddressString.compare(curAddress);
			if(compVal == -static_cast<decltype(compVal)>(strlen(curAddress)) || compVal == 0)
				return curType;
		}

		return HW_ADDRESS_UNDEFINED;
	}

	HardwareAddressString::HardwareAddressString(string &&String)
		: _String(std::move(String))
	{}

	HardwareAddressOption<HW_ADDRESS_IP> HardwareAddressOption<HW_ADDRESS_IP>::ImportFromAddressString(const HardwareAddressString &AddressString)
	{
		return HardwareAddressOption<HW_ADDRESS_IP>(AddressString);
	}

	HardwareAddressString HardwareAddressOption<HW_ADDRESS_IP>::ToHardwareAddressString() const
	{
		string hwString = string(AddressPrefixes[HW_ADDRESS_IP]) + this->_IPAddress.AddrToString() + ":" + this->_IPAddress.PortToString();
		return HardwareAddressString::ImportAddressString(std::move(hwString));
	}

	HardwareAddressOption<HW_ADDRESS_IP>::connection_t HardwareAddressOption<HW_ADDRESS_IP>::OpenConnection(hardware_protocols_t Protocol) const
	{
		try
		{
			if(Protocol == HW_CONNECTION_TCP)
				return unique_ptr<NetworkConnection>(new NetworkSocketTCPClient(NetworkSocketTCPClient::StartConnection(this->_IPAddress)));
		}
		catch(Exception &)
		{
			return nullptr;
		}

		return nullptr;
	}

	HardwareAddressOption<HW_ADDRESS_IP>::HardwareAddressOption(const HardwareAddressString &AddressString)
		: _IPAddress(ip_addr_t::ImportFromString(HardwareAddressOption<HW_ADDRESS_IP>::GetAddressString(AddressString.GetString())))
	{}

	string HardwareAddressOption<HW_ADDRESS_IP>::GetAddressString(const string &StringAddress)
	{
		auto addressPos = StringAddress.find(AddressPrefixes[HW_ADDRESS_IP]);
		if(addressPos == StringAddress.npos)
			throw Exception(ERROR_NUM, "ERROR HWNetworkConnection::GetAddressStart(): Couldn't find prefix\n");
		addressPos += strlen(AddressPrefixes[HW_ADDRESS_IP]);

		auto addressLength = StringAddress.find(HWNetworkConnectionEnd, addressPos);
		if(addressLength == StringAddress.npos)
			addressLength = StringAddress.length() - addressPos;
		else
			addressLength = addressLength - addressPos;

		return StringAddress.substr(addressPos, addressLength);
	}
}

namespace hardware_connection_option
{
	class TestHardwareAddress
	{
		public:
			static bool Testing();
	};

	bool TestHardwareAddress::Testing()
	{
		try
		{
			const string testString("<ip>://127.0.0.1");
			HardwareAddressString testIP = HardwareAddressString::ImportAddressString(string(testString));

			if(testIP != HardwareAddressString::ImportAddressString(string(testString)) ||
					!(testIP == HardwareAddressString::ImportAddressString(string(testString))))
				return 0;

			if(testIP.GetType() != HW_ADDRESS_IP)
				return 0;

			auto testAccess = testIP.GetAddressAccess();
			if(testAccess == nullptr)
				return 0;

			auto testConnection = testAccess->OpenConnection(HW_CONNECTION_TCP);
			if(testConnection == nullptr)
				return 0;

			auto testConversion = testAccess->ToHardwareAddressString();
			if(testConversion != testIP)
				return 0;


			return 1;
		}
		catch(Exception &)
		{
			return 0;
		}
	}
}
