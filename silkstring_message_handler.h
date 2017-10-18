#ifndef SILKSTRING_MESSAGE_HANDLER_H
#define SILKSTRING_MESSAGE_HANDLER_H

/*! \file silkstring_message_handler.h
 *  \brief Header for class class
 */


#include "silkstring_message.h"

/*!
 *  \brief Namespace for class class
 */
namespace silkstring_message_handler
{
	using namespace silkstring_message;

	using msg_struct_t = thread_multi_module_message_t;

	class MessageHandler
	{
		public:
	};
}


#include "protocol_messages.h"

using namespace protocol_messages;
using namespace silkstring_message_handler;

int test()
{

	return 0;
}

///*!
// *  \brief Namespace for class class
// */
//namespace silkstring_message_handler
//{
//	using namespace silkstring_message;

//	using msg_struct_t = thread_multi_module_message_t;

//	template<class T, int N>
//	struct flag
//	{
//		friend constexpr int adl_flag (flag<T, N>);
//	};

//	template<class T, int N>
//	struct writer
//	{
//		friend constexpr int adl_flag(flag<T, N>)
//		{
//			return N;
//		}

//		static constexpr int value = N;
//	};

//	template<class T, int N, int = adl_flag (flag<T, N> {})>
//	int constexpr reader(int, flag<T, N>)
//	{
//		return N;
//	}

//	template<class T, int N>
//	int constexpr reader(float, flag<T, N>, int R = reader (0, flag<T, N-1> {}))
//	{
//		return R;
//	}

//	template<class T>
//	int constexpr reader (float, flag<T, 0>)
//	{
//		return 0;
//	}

//	template<class T, int N = 1>
//	int constexpr next(int R = writer<T, reader<T>(0, flag<T, 32> {}) + N>::value)
//	{
//		return R;
//	}

//	template<class T>
//	using handle_fcn_t = message_t(T *const, msg_struct_t&);

//	template<class T, size_t N>
//	struct tmp_flag_num {};

//	template<class T, size_t N>
//	constexpr size_t flagNumFcn(int, tmp_flag_num<T,N>)
//	{
//		return N;
//	}

//	template<class T, size_t N>
//	constexpr handle_fcn_t<T> *flagHandleFcn(tmp_flag_num<T,N>, handle_fcn_t<T> *const Function = nullptr);

//	template<class T, class MessageStruct>
//	class MessageFunction
//	{
//			static constexpr auto _Number = next<T>();

//			template<class U, size_t UN>
//			friend constexpr handle_fcn_t<T> *flagHandleFcn(tmp_flag_num<T, _Number>, handle_fcn_t<T> *const Function = MessageFunction::Handle)
//			{
//				return Function;
//			}

//		public:
//			static message_t Handle(T *const Class, msg_struct_t &Message)
//			{
//				if(MessageStruct::CheckMessageDataType(Message))
//				{
//					MessageFunction::HandleFcn(Class, Message);

//					return Message.Get<MessageTypeNum>();
//				}

//				return UnusedMessage;
//			}

//			static void HandleFcn(T *const Class, msg_struct_t &Message);
//	};

//	template<class T>
//	class MessageHandler
//	{
//		public:
//			using msg_struct_t = silkstring_message_handler::msg_struct_t;

//			static message_t HandleMessages(T *const Class, msg_struct_t &Message)
//			{
//				if constexpr(MessageHandler::HandleMessageN<0> != nullptr)
//				{
//					return MessageHandler::HandleMessageN<0>(Class, Message);
//				}
//				else
//					return UnusedMessage;
//			}

//		private:
//			static constexpr size_t _NumFunctions = next<T>();

//			template<size_t N>
//			static message_t HandleMessageN(T *const Class, msg_struct_t &Message)
//			{
//				const auto retVal = (flagHandleFcn<T,N>(tmp_flag_num<T,N>{}))(Class, Message);

//				if(retVal == UnusedMessage)
//				{
//					if constexpr(MessageHandler::HandleMessageN<N+1> != UnusedMessage)
//					{
//						return MessageHandler<T>::HandleMessageN<N+1>(Class, Message);
//					}
//					else
//						return UnusedMessage;
//				}
//				else
//					return retVal;
//			}

//			template<class U, class UMessageStruct>
//			friend class MessageFunction;
//	};
//} // namespace silkstring_message_handler

//#include "protocol_messages.h"

//using namespace protocol_messages;
//using namespace silkstring_message_handler;

////int test()
////{
////	constexpr auto test1 = flagNumFcn(static_cast<float>(0.0f), tmp_flag_num<int,0>{});
////	constexpr auto test2 = flagNumFcn(static_cast<float>(0.0f), tmp_flag_num<int,0>{});

////	constexpr flag<int, 0> testFlag1;
////	constexpr flag<int, 1> testFlag2;

////	constexpr auto test3 = flagNumFcn(static_cast<float>(0.0f), tmp_flag_num<int,0>{});

////	static_assert(test3 == 0, "");
////}

//template<>
//void MessageFunction<int, protocol_messages::tls_handshake_completed_t>::HandleFcn(int *const Class, msg_struct_t &Message)
//{
//	*Class = 0;
//}

//template<>
//void MessageFunction<int, protocol_messages::tls_input_data_update_t>::HandleFcn(int *const Class, msg_struct_t &Message)
//{
//	*Class = 1;
//}


//int test ()
//{
//	constexpr MessageHandler<int> test;

//	auto testMsg = tls_handshake_completed_t::CreateMessageFromSender(0, UnusedID, tls_handshake_completed_t(true));
//	test.HandleMessages(new int(0), testMsg);
//}

#endif // SILKSTRING_MESSAGE_HANDLER_H
