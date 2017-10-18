#ifndef PROTOCOL_SEND_HANDLE_H
#define PROTOCOL_SEND_HANDLE_H

/*! \file protocol_send_handle.h
 *  \brief Header for ProtocolSendHandle class
 */


#include "protocol_vector.h"
#include "protocol_data.h"
#include "protocol_messages.h"

/*!
 *  \brief Namespace for ProtocolSendHandle class
 */
namespace protocol_send_handle
{
	using protocol_vector::protocol_vector_t;
	using protocol_data::protocol_header_t;
	using protocol_data::protocol_header_name_t;
	using protocol_data::protocol_header_size_t;

	using protocol_messages::module_message_connection_t;

	/*!
	 * \brief The ProtocolSendHandle class
	 */
	class ProtocolSendHandle
	{
		public:
			/*!
			 * 	brief Constructor
			 */
			ProtocolSendHandle() = default;

			virtual ~ProtocolSendHandle() = default;

			void SendData(protocol_vector_t &Data);

		private:

			virtual void SendDataHandle(protocol_vector_t &Data) = 0;
	};

	/*!
	* \brief Appends the proper header before a module message
	 */
	class ProtocolModuleSendHandle
	{
		public:
			explicit ProtocolModuleSendHandle(ProtocolSendHandle *Sender);

			template<class T>
			void SendData(module_message_connection_t DataHeaderName, T &&Data)
			{
				assert(this->_Sender != nullptr);

				if(this->_Sender != nullptr)
				{
					protocol_vector_t tmpVector(sizeof(protocol_header_t)+sizeof(typename std::remove_reference<T>::type));

					// Add data
					reinterpret_cast<typename std::remove_reference<T>::type &>(*(tmpVector.begin()+sizeof(protocol_header_t))) = std::forward<T>(Data);

					// Prepend header
					ProtocolModuleSendHandle::PrependHeaderNoResize(DataHeaderName, tmpVector);

					this->_Sender->SendData(tmpVector);
				}
			}

			void SendData(module_message_connection_t DataHeaderName, protocol_vector_t &Data);

			void SendData(module_message_connection_t DataHeaderName, protocol_vector_t &&Data);

			ProtocolSendHandle *GetSender();
			const ProtocolSendHandle *GetSender() const;

		private:

			ProtocolSendHandle *_Sender = nullptr;

			static void PrependHeaderNoResize(module_message_connection_t HeaderName, protocol_vector_t &Data);
	};
} // namespace protocol_send_handle


#endif // PROTOCOL_SEND_HANDLE_H
