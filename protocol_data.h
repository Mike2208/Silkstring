#ifndef PROTOCOL_DATA_H
#define PROTOCOL_DATA_H

/*! \file protocol_data.h
 *  \brief Header for Pro class
 */

#include <array>
#include "silkstring_message.h"
#include "protocol_vector.h"
#include "typesafe_class.h"

/*!
 *  \brief Namespace for Pro class
 */
namespace protocol_data
{
	using std::array;

	using silkstring_message::identifier_t;

	using protocol_vector::byte_t;

	using protocol_thread_id_type = identifier_t::thread_id_t;
	using protocol_thread_id_t = protocol_thread_id_type;
//	class protocol_thread_id_t
//	{
//		public:
//			constexpr protocol_thread_id_t() = default;
//			constexpr protocol_thread_id_t(protocol_thread_id_type Value) : _Value(Value) {}

//			COMPARISON_OPERATORS(protocol_thread_id_t, _Value)
//			EQUALITY_OPERATORS(protocol_thread_id_t, _Value)

//			INCREMENT_OPERATORS(protocol_thread_id_t, _Value)
//			DECREMENT_OPERATORS(protocol_thread_id_t, _Value)

//		private:
//			protocol_thread_id_type _Value = 0;
//	};

	const protocol_thread_id_t ConnectionIDInvalid = protocol_thread_id_t();
	const protocol_thread_id_t ConnectionIDAll = 1;
	const protocol_thread_id_t ConnectionIDStart = 2;

	using protocol_header_size_t = u_int32_t;

	/*!
	 * \brief Size of Module Name
	 */
	static constexpr size_t ProtocolNameSize = 3;

	/*!
	 * \brief Name of Module
	 */
	struct protocol_header_name_t : public array<byte_t, ProtocolNameSize>
	{
		using header_array_t = array<byte_t, ProtocolNameSize>;

		protocol_header_name_t() : header_array_t{{'\0', '\0', '\0'}} {}

		constexpr protocol_header_name_t(const array<byte_t, ProtocolNameSize> &S) : header_array_t(S) {}

		bool operator==(const protocol_header_name_t &Other) const;
		bool operator!=(const protocol_header_name_t &Other) const;

		bool operator<(const protocol_header_name_t &Other) const;
	};

	/*!
	 *	\brief Name of module without header
	 */
	static const protocol_header_name_t EmptyModuleName{{{'0', '0', '0'}}};

	/*!
	 * \brief Struct of headers
	 */
	struct protocol_header_t
	{
		/*!
		 * \brief Name of module to send information to
		 */
		protocol_header_name_t Name;

		/*!
		 * \brief Size of header + data
		 */
		protocol_header_size_t Size;

		constexpr protocol_header_t(const protocol_header_name_t _Name, const protocol_header_size_t _Size)
			: Name(_Name), Size(_Size)
		{}
	};
} // namespace protocol_data


#endif // PROTOCOL_DATA_H
