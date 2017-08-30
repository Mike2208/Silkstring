#ifndef PROTOCOL_MAP_H
#define PROTOCOL_MAP_H

/*! \file protocol_map.h
 *  \brief Header for ProtocolMap class
 */


#include "map_type.h"

/*!
 *  \brief Namespace for ProtocolMap class
 */
namespace protocol_map
{
	using map_type::MapType;

	/*!
	 * \brief Map for elements in the protocol
	 */
	template<class KeyType, class ValueType>
	class ProtocolMap : public MapType<KeyType, ValueType>
	{
		public:
			/*!
			 * \brief Constructor
			 */
			ProtocolMap() = default;
	};
} // namespace protocol_map


#endif // PROTOCOL_MAP_H
