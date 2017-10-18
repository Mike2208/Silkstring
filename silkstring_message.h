#ifndef SILKSTRING_MESSAGE_H
#define SILKSTRING_MESSAGE_H

/*! \file silkstring_message.h
 *  \brief Header for SilkstringMessage class
 */


#include "thread_module_manager_multi_message.h"
#include "dynamic_pointer.h"

/*!
 *  \brief Namespace for SilkstringMessage class
 */
namespace silkstring_message
{
	using std::array;
	using std::shared_ptr;

	using dynamic_pointer::SharedDynamicPointer;

	using thread_module_manager_multi_message::ThreadModuleManagerMultiMessage;

	using message_list_size_t = size_t;

	/*!
	 *	\brief Identifier Type used by all modules
	 */
	struct identifier_t
	{
		using type = uint32_t;
		using queue_id_t = uint8_t;
		using thread_id_t = uint8_t;
		using module_id_t = uint16_t;

		type MessageQueueID	: 8;
		type ModuleID		: 16;
		type ThreadID		: 8;

		constexpr identifier_t()
			: MessageQueueID(0), ModuleID(0), ThreadID(0)
		{}

		constexpr identifier_t(queue_id_t _QueueID, module_id_t _ModuleID, thread_id_t _ThreadID)
			: MessageQueueID(_QueueID), ModuleID(_ModuleID), ThreadID(_ThreadID)
		{}

		constexpr bool operator==(const identifier_t &S) const
		{
			//if(*(reinterpret_cast<const type *>(this)) == *(reinterpret_cast<const type*>(&S)))
			return *((const type*)this) == *((const type*)&S);
		}
	};

	/*!
	 *	\brief Identify message type
	 */
	//using message_t = uint32_t;
	struct message_t
	{
		using owner_t = bool;
		using message_type_t = uint16_t;

		static constexpr owner_t ReceiverOwner	= 0;
		static constexpr owner_t SenderOwner	= 1;

		/*!
		 * \brief Does the received message type refer to a message type of the receiver (0) or sender (1)
		 */
		owner_t MessageTypeOwner;

		/*!
		 * \brief Message Type
		 */
		message_type_t MessageType;

		constexpr bool IsSenderMessageType() const
		{
			return MessageTypeOwner == SenderOwner;
		}

		explicit constexpr message_t(message_type_t _MessageType, bool _SenderOwner = ReceiverOwner)
			: MessageTypeOwner(_SenderOwner), MessageType(_MessageType)
		{}

		constexpr bool operator==(const message_t &S) const
		{
			return (this->MessageTypeOwner == S.MessageTypeOwner && this->MessageType == S.MessageType);
		}
	};

	/*!
	 *	\brief Generic message pointer
	 */
	using message_ptr = SharedDynamicPointer<void>;

	/*!
	 *	\brief Module manager
	 */
	using thread_multi_module_manager_type = ThreadModuleManagerMultiMessage<identifier_t, message_t, message_ptr>;

	static constexpr message_queue::variadic_counter_t MessageReceiverIDNum = 0;
	static constexpr message_queue::variadic_counter_t MessageSenderIDNum = 1;
	static constexpr message_queue::variadic_counter_t MessageTypeNum = 2;
	static constexpr message_queue::variadic_counter_t MessageDataNum = 3;

	static constexpr message_t::message_type_t DefaultMessageType = 0;
	static constexpr message_t::message_type_t UnusedMessageType = -1;
	static constexpr message_t UnusedMessage(UnusedMessageType, 1);

	static constexpr identifier_t::queue_id_t StartQueueID = 1;

	static constexpr identifier_t::module_id_t UnusedModuleID = 0;
	static constexpr identifier_t::module_id_t DefaultQueueModuleID = 1;
	static constexpr identifier_t::module_id_t StartModuleID = 2;

	static constexpr identifier_t::thread_id_t DefaultThreadID = 0;

	static constexpr identifier_t UnusedID = identifier_t();

	/*!
	 *	\brief Module that can be registered to run in an instance of thread_multi_module_manager_t
	 */
	using thread_multi_module_t = thread_multi_module_manager_type::module_t;
	using thread_multi_module_shared_ptr_t = thread_multi_module_manager_type::module_shared_ptr_t;

	/*!
	 *	\brief Module manager that runs a separate thread to handle messages. Can be included into GlobalMessageQueueThread as a module
	 */
	class thread_multi_module_manager_t
		:	public thread_multi_module_manager_type,
			public thread_multi_module_t
	{
		public:
			using msg_struct_t = thread_multi_module_manager_type::msg_struct_t;

			thread_multi_module_manager_t(identifier_t::queue_id_t QueueID, identifier_t::thread_id_t ThreadID = DefaultThreadID, thread_queued::thread_state_t ThreadState = thread_queued::THREAD_RUNNING);

		private:

			void HandleMessage(msg_struct_t &Message);

			template<class U>
			friend class ::TestingClass;
	};

	/*!
	 *	\brief Pointer to thread
	 */
	using thread_multi_module_manager_shared_ptr_t = shared_ptr<thread_multi_module_manager_t>;

	/*!
	 *	\brief Vector for IDs
	 */
	using id_vector_t = thread_multi_module_manager_t::id_vector_t;

	/*!
	 *	\brief Message that can be transmitted to any thread_multi_module_manager_t
	 */
	using thread_multi_module_message_t = thread_multi_module_t::msg_struct_t;

	/*!
	 *	\brief Module Function
	 */
	template<class BaseClass>
	struct module_fcn_t
	{
		using type = void(BaseClass *Class, thread_multi_module_t::msg_struct_t&);
	};

	template<typename BaseClass, typename module_fcn_t<BaseClass>::type* ...Functions>
	class thread_multi_module_fcn_t
	{
			using fcn_t = typename module_fcn_t<BaseClass>::type;
			using fcn_array_t = array<fcn_t*, sizeof...(Functions)>;

			static constexpr fcn_array_t _FcnArray = fcn_array_t{Functions...};

		public:
			static void HandleMessages(BaseClass *Class, thread_multi_module_t::msg_struct_t &Message)
			{
				const auto messageType = Message.Get<MessageTypeNum>();
				const auto messagePos = messageType.MessageType - DefaultMessageType;
				if(!messageType.IsSenderMessageType() &&
						static_cast<decltype(thread_multi_module_fcn_t::_FcnArray.size())>(messagePos) < thread_multi_module_fcn_t::_FcnArray.size() && messagePos >= 0)
				{
					(*(thread_multi_module_fcn_t::_FcnArray[messageType.MessageType - DefaultMessageType]))(Class, Message);
				}
			}
	};

	template<typename BaseClass, typename module_fcn_t<BaseClass>::type* ...Functions>
	constexpr typename thread_multi_module_fcn_t<BaseClass, Functions...>::fcn_array_t thread_multi_module_fcn_t<BaseClass, Functions...>::_FcnArray;

	template<identifier_t::queue_id_t _QueueID, identifier_t::module_id_t _ModuleID, message_t::message_type_t _MessageType, class T>
	struct message_id_struct_t
	{
		using data_t = T;

		static constexpr decltype(_QueueID) QueueID = _QueueID;
		static constexpr decltype(_ModuleID) ModuleID = _ModuleID;
		static constexpr decltype(_MessageType) MessageType = _MessageType;

		static constexpr identifier_t CreateID(identifier_t::thread_id_t ThreadID)
		{
			return identifier_t(QueueID, ModuleID, ThreadID);
		}

		static inline thread_multi_module_message_t CreateMessageFromSender(identifier_t::thread_id_t ReceiverThreadID, identifier_t SenderID, T *MemData)
		{
			return thread_multi_module_message_t{CreateID(ReceiverThreadID), SenderID, message_t(_MessageType, message_t::ReceiverOwner), message_ptr(MemData)};
		}

		static inline thread_multi_module_message_t CreateMessageFromSender(identifier_t::thread_id_t ReceiverThreadID, identifier_t SenderID, T &&Data)
		{
			return CreateMessageFromSender(ReceiverThreadID, SenderID, new T(std::move(Data)));
		}

		static inline thread_multi_module_message_t CreateMessageToReceiver(identifier_t ReceiverID, identifier_t::thread_id_t SenderThreadID, T *MemData)
		{
			return thread_multi_module_message_t{ReceiverID, CreateID(SenderThreadID), message_t(_MessageType, message_t::SenderOwner), message_ptr(MemData)};
		}

		static inline thread_multi_module_message_t CreateMessageToReceiver(identifier_t ReceiverID, identifier_t::thread_id_t SenderThreadID, T &&Data)
		{
			return CreateMessageToReceiver(ReceiverID, SenderThreadID, new T(std::move(Data)));
		}

		static inline T *GetMessageData(thread_multi_module_message_t &Data)
		{
			return Data.Get<MessageDataNum>().Get<T>();
		}

		static inline const T *GetMessageDataConst(const thread_multi_module_message_t &Data)
		{
			return Data.Get<MessageDataNum>().Get<T>();
		}

		static inline bool CheckMessageDataType(thread_multi_module_message_t &Data, const identifier_t::thread_id_t ThreadID)
		{
			const auto &messageType = Data.Get<MessageTypeNum>();
			if(messageType.MessageType == MessageType)
			{
				const identifier_t *pOwnerID;
				if(messageType.IsSenderMessageType())
					pOwnerID = &(Data.Get<MessageSenderIDNum>());
				else
					pOwnerID = &(Data.Get<MessageReceiverIDNum>());

				if(*pOwnerID == identifier_t(QueueID, ModuleID, ThreadID))
					return 1;
			}

			return 0;
		}
	};

	template<identifier_t::queue_id_t _QueueID, identifier_t::module_id_t _ModuleID, message_t::message_type_t _MessageType, class T>
	constexpr decltype(_QueueID) message_id_struct_t<_QueueID, _ModuleID, _MessageType, T>::QueueID;

	template<identifier_t::queue_id_t _QueueID, identifier_t::module_id_t _ModuleID, message_t::message_type_t _MessageType, class T>
	constexpr decltype(_ModuleID) message_id_struct_t<_QueueID, _ModuleID, _MessageType, T>::ModuleID;

	template<identifier_t::queue_id_t _QueueID, identifier_t::module_id_t _ModuleID, message_t::message_type_t _MessageType, class T>
	constexpr decltype(_MessageType) message_id_struct_t<_QueueID, _ModuleID, _MessageType, T>::MessageType;

	template<identifier_t::queue_id_t _QueueID, identifier_t::module_id_t _ModuleID, identifier_t::thread_id_t _ThreadID, message_t::message_type_t _MessageType, class T>
	struct message_id_thread_struct_t : public message_id_struct_t<_QueueID, _ModuleID, _MessageType, T>
	{
		static constexpr decltype(_ThreadID) ThreadID = _ThreadID;
		static constexpr identifier_t ID = identifier_t(_QueueID, _ModuleID, ThreadID);

		static constexpr identifier_t CreateID()
		{
			return ID;
		}


		static inline thread_multi_module_message_t CreateMessageFromSender(identifier_t SenderID, T *MemData)
		{
			return message_id_struct_t<_QueueID, _ModuleID, _MessageType, T>::CreateMessageFromSender(ThreadID, SenderID, MemData);
		}

		static inline thread_multi_module_message_t CreateMessageFromSender(identifier_t SenderID, T &&Data)
		{
			return CreateMessageFromSender(SenderID, new T(std::move(Data)));
		}

		static inline thread_multi_module_message_t CreateMessageToReceiver(identifier_t ReceiverID, T *MemData)
		{
			return message_id_struct_t<_QueueID, _ModuleID, _MessageType, T>::CreateMessageToReceiver(ReceiverID, ThreadID, MemData);
		}

		static inline thread_multi_module_message_t CreateMessageToReceiver(identifier_t ReceiverID, T &&Data)
		{
			return CreateMessageToReceiver(ReceiverID, new T(std::move(Data)));
		}

		static inline bool CheckMessageDataType(thread_multi_module_message_t &Data)
		{
			return message_id_struct_t<_QueueID, _ModuleID, _MessageType, T>::CheckMessageDataType(Data, ThreadID);
		}
	};

	template<identifier_t::queue_id_t _QueueID, identifier_t::module_id_t _ModuleID, identifier_t::thread_id_t _ThreadID, message_t::message_type_t _MessageType, class T>
	constexpr decltype(_ThreadID) message_id_thread_struct_t<_QueueID, _ModuleID, _ThreadID, _MessageType, T>::ThreadID;

	template<identifier_t::queue_id_t _QueueID, identifier_t::module_id_t _ModuleID, identifier_t::thread_id_t _ThreadID, message_t::message_type_t _MessageType, class T>
	constexpr identifier_t message_id_thread_struct_t<_QueueID, _ModuleID, _ThreadID, _MessageType, T>::ID;

	/*!
	 * \brief ID for sending a module registration message
	 */
	static constexpr identifier_t ModuleRegistrationID = identifier_t(0,0,1);
	static constexpr message_t::message_type_t ModuleRegistrationMessageType = DefaultMessageType;
	struct module_registration_message_t : public message_id_thread_struct_t<ModuleRegistrationID.MessageQueueID, ModuleRegistrationID.ModuleID, ModuleRegistrationID.ThreadID, ModuleRegistrationMessageType, module_registration_message_t>
	{
		thread_multi_module_shared_ptr_t ModuleToRegister;
		id_vector_t SenderLinkIDs;
		id_vector_t ReceiverLinkIDs;

		module_registration_message_t(thread_multi_module_shared_ptr_t _ModuleToRegister, id_vector_t &&_SenderIDLinks, id_vector_t &&_RecevierIDLinks);
	};

	static constexpr message_t::message_type_t ModuleUnregistrationMessageType = DefaultMessageType + 1;
	struct module_unregistration_message_t : public message_id_thread_struct_t<ModuleRegistrationID.MessageQueueID, ModuleRegistrationID.ModuleID, ModuleRegistrationID.ThreadID, ModuleUnregistrationMessageType, module_unregistration_message_t>
	{
		identifier_t ModuleID;

		/*!
		 * \brief Once the module is unregistered, it is stored here so that it can be retrieved.
		 *
		 * You should know what you're doing if you're using this (aka, keep the pointer valid)
		 */
		thread_multi_module_shared_ptr_t *ppReturnValue = nullptr;

		explicit module_unregistration_message_t(identifier_t _ModuleID, thread_multi_module_shared_ptr_t *_ppReturnValue = nullptr);
	};

	//  -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	template<identifier_t::thread_id_t ThreadID, class ...ModuleStructs>
	struct message_id_thread_struct_functions_t
	{
		static bool HandleMessage(thread_multi_module_message_t &Message);

		struct message_thread_id_t
		{
			static constexpr identifier_t::thread_id_t SubThreadID = ThreadID;
		};
	};

	template<class ...ModuleStructsAndFunctions>
	struct message_id_thread_struct_functions_sub_t
	{
		static bool HandleMessage(thread_multi_module_message_t &)
		{
			return 0;
		}
	};

	template<class CurThreadID, class CurModuleStruct, class CurModuleFunction, class ...ModuleStructsAndFunctions>
	struct message_id_thread_struct_functions_sub_t<CurThreadID, CurModuleStruct, CurModuleFunction, ModuleStructsAndFunctions...>
	{
		static bool HandleMessage(thread_multi_module_message_t &Message)
		{
			if(CurModuleStruct::CheckMessageDataType(Message, CurThreadID::SubThreadID))
			{
				CurModuleFunction(Message);

				return 1;
			}
			else
				return message_id_thread_struct_functions_sub_t<CurThreadID, ModuleStructsAndFunctions...>::HandleMessage(Message);
		}
	};

	template<identifier_t::thread_id_t ThreadID, class ...ModuleStructs>
	bool message_id_thread_struct_functions_t<ThreadID, ModuleStructs...>::HandleMessage(thread_multi_module_message_t &Message)
	{
		return message_id_thread_struct_functions_sub_t<message_id_thread_struct_functions_t<ThreadID, ModuleStructs...>::message_thread_id_t, ModuleStructs...>::HandleMessage(Message);
	}

	template<class ...ModuleStructs>
	struct message_struct_functions_t
	{
		static bool HandleMessage(thread_multi_module_message_t&)
		{
			return 0;
		}
	};

	template<class CurModuleStruct, class CurModuleFunction, class ...ModuleStructsAndFunctions>
	struct message_struct_functions_t<CurModuleStruct, CurModuleFunction, ModuleStructsAndFunctions...>
	{
		static bool HandleMessage(thread_multi_module_message_t &Message)
		{
			if(CurModuleStruct::CheckMessageDataType(Message))
			{
				CurModuleFunction(Message);

				return 1;
			}
			else
				return message_struct_functions_t<ModuleStructsAndFunctions...>::HandleMessage(Message);
		}
	};
} // namespace silkstring_message

#endif // SILKSTRING_MESSAGE_H
