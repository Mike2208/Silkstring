#ifndef HARDWARE_PROTOCOLS_H
#define HARDWARE_PROTOCOLS_H

/*!	\file hardware_protocols.h
 *	\brief Contains supported hardware protocols
 */

namespace hardware_protocols
{
	/*!
	 * \brief Supported hardware protocols
	 */
	enum hardware_protocols_t
	{
		HW_CONNECTION_UNDEFINED = -1,
		HW_CONNECTION_TCP = 0,
		HW_CONNECTION_NUM
	};

	/*!
	 * \brief
	 */
	enum hardware_address_t
	{
		HW_ADDRESS_UNDEFINED = -1,
		HW_ADDRESS_IP = 0,
		HW_ADDRESS_NUM
	};
}

#endif // HARDWARE_PROTOCOLS_H
