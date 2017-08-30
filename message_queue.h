#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

/*! \file message_queue.h
 *  \brief Header for MessageQueue class
 */


#include <memory>
#include <list>
#include "testing_class_declaration.h"

/*!
 *  \brief Namespace for MessageQueue class
 */
namespace message_queue
{
	using std::list;

	using variadic_counter_t = unsigned int;

	template<class Class1, class Class2>
	using template_compare = std::is_same<typename std::remove_reference<Class1>::type, typename std::remove_reference<Class2>::type>;

	template<class Class1, class Class2>
	using template_convertible = std::is_convertible<Class1, Class2>;

	template<unsigned int N, class... Args>
	struct type_struct
	{
		using type = void;
		using rest_type = void;
	};

	/*!
	 *	\brief variadic template struct
	 */
	template<class...Args>
	struct message_struct_t
	{
	};

	template<class T, class... Args>
	struct message_struct_t<T, Args...> : public message_struct_t<Args...>
	{
		template<class FcnT, class ...FcnArgs>
		message_struct_t(FcnT &&Data, FcnArgs &&...ExtraData)
			: message_struct_t<Args...>(std::forward<FcnArgs>(ExtraData)...), _Data(std::forward<FcnT>(Data))
		{
			static_assert(template_convertible<FcnT, T>::value, "Parameter type must match class type");
		}

		message_struct_t(const message_struct_t &S) = default;
		message_struct_t(message_struct_t &S) = default;
		message_struct_t(message_struct_t &&S) = default;

		message_struct_t &operator=(message_struct_t &&S) = default;
		message_struct_t &operator=(const message_struct_t &S) = default;
		message_struct_t &operator=(message_struct_t &S) = default;

		constexpr decltype(sizeof...(Args)) Size() const
		{
			return sizeof...(Args)+1;
		}

		template<unsigned int N>
		typename std::enable_if<N == 0, T>::type &Get()
		{
			return this->_Data;
		}

		template<unsigned int N>
		typename std::enable_if<(N > 0 && N <= sizeof...(Args)), typename type_struct<N-1, Args...>::type>::type &Get()
		{
			return static_cast<message_struct_t<Args...>&>(*this).template Get<N-1>();
			//return this->_ExtraData.template Get<N-1>();
		}

		template<unsigned int N>
		const typename std::enable_if<N == 0, T>::type &Get() const
		{
			return this->_Data;
		}

		template<unsigned int N>
		const typename std::enable_if<(N > 0 && N <= sizeof...(Args)), typename type_struct<N-1, Args...>::type>::type &Get() const
		{
			return static_cast<const message_struct_t<Args...>&>(*this).template Get<N-1>();
			//return this->_ExtraData.template Get<N-1>();
		}

		template<unsigned int N>
		typename std::enable_if<N == 0, message_struct_t<T, Args...>>::type &GetRestStruct()
		{
			return *this;
		}

		template<unsigned int N>
		typename std::enable_if<(N > 0 && N <= sizeof...(Args)), typename type_struct<N-1, Args...>::rest_type>::type &GetRestStruct()
		{
			return static_cast<message_struct_t<Args...>&>(*this).template GetRestStruct<N-1>();
		}

		template<unsigned int N>
		const typename std::enable_if<N == 0, message_struct_t<T, Args...>>::type &GetRestStruct() const
		{
			return *this;
		}

		template<unsigned int N>
		const typename std::enable_if<(N > 0 && N <= sizeof...(Args)), typename type_struct<N-1, Args...>::rest_type>::type &GetRestStruct() const
		{
			return static_cast<message_struct_t<Args...>&>(*this).template GetRestStruct<N-1>();
		}


		private:

			T _Data;

			//message_struct_t<Args...> _ExtraData;
			//typename std::enable_if<(sizeof...(Args)>0), message_struct_t<Args...>>::type _ExtraData;
	};

	template<class T, class... Args>
	struct type_struct<0, T, Args...>
	{
		using type = T;
		using rest_type = message_struct_t<T, Args...>;
	};

	/*!
	 *	\brief struct for accessing types in a variadic template
	 */
	template<unsigned int N, class T, class... Args>
	struct type_struct<N, T, Args...>
	{
		using type = typename type_struct<N-1, Args...>::type;
		using rest_type = typename type_struct<N-1, Args...>::rest_type;
	};

	class TestMessageQueue;

	/*!
	 * \brief Only provides access to the Push function to prevent accidental popping
	 */
	template<class... Args>
	class MessageQueueInterface
	{
		public:
			/*!
			 *	\brief Structure of message
			 */
			using msg_struct_t = message_struct_t<Args...>;

			MessageQueueInterface() = default;
			~MessageQueueInterface() = default;

			/*!
			 * \brief Push a new message onto list
			 */
			template<template<class ...VarArgs> class FcnVariadic, class ...FcnArgs>
			void Push(FcnVariadic<FcnArgs...> &&Message)
			{
				static_assert(template_convertible<FcnVariadic<FcnArgs...>, msg_struct_t>::value,"Can't call push without the same parameter as template parameter's class");

				this->_Messages.push_back(std::forward<FcnVariadic<FcnArgs...>>(Message));
			}

			/*!
			 * \brief Push a new message onto list
			 */
			template<class ...FcnArgs>
			void Push(FcnArgs &&...MessageData)
			{
				static_assert(template_convertible<message_struct_t<FcnArgs...>, msg_struct_t>::value,"Can't call push without the same parameter as template parameter's class");

				this->_Messages.push_back(msg_struct_t(std::forward<FcnArgs>(MessageData)...));
			}

			/*!
			 * \brief Push a new message onto the front of the list
			 */
			template<template<class ...VarArgs> class FcnVariadic, class ...FcnArgs>
			void PushFront(FcnVariadic<FcnArgs...> &&Message)
			{
				static_assert(template_convertible<FcnVariadic<FcnArgs...>, msg_struct_t>::value,"Can't call push without the same parameter as template parameter's class");

				this->_Messages.push_front(std::forward<FcnVariadic<FcnArgs...>>(Message));
			}

			/*!
			 * \brief Push a new message onto the front of the list
			 */
			template<class ...FcnArgs>
			void PushFront(FcnArgs &&...MessageData)
			{
				static_assert(template_convertible<message_struct_t<FcnArgs...>, msg_struct_t>::value,"Can't call push without the same parameter as template parameter's class");

				this->_Messages.push_front(msg_struct_t(std::forward<FcnArgs>(MessageData)...));
			}

			size_t GetQueueSize() const
			{
				return this->_Messages.size();
			}

		protected:

			/*!
			 * \brief List of all messages
			 */
			list<msg_struct_t> _Messages;

			// Prevent interfaces from copying or moving data, only MessageQueue can do that
			MessageQueueInterface(const MessageQueueInterface &S) = default;
			MessageQueueInterface(MessageQueueInterface &&S) = default;

			MessageQueueInterface &operator=(const MessageQueueInterface &S) = default;
			MessageQueueInterface &operator=(MessageQueueInterface &&S) = default;

		private:
			friend class TestMessageQueue;

			template<class U>
			friend class ::TestingClass;
	};

	/*!
	 * \brief The MessageQueue class
	 */
	template<class... Args>
	class MessageQueue : public MessageQueueInterface<Args...>
	{
		public:
			/*!
			 *	\brief Structure of message
			 */
			using msg_struct_t = message_struct_t<Args...>;

			MessageQueue() = default;
			~MessageQueue() = default;

			MessageQueue(const MessageQueue &S) = default;
			MessageQueue(MessageQueue &&S) = default;

			MessageQueue &operator=(const MessageQueue &S) = default;
			MessageQueue &operator=(MessageQueue &&S) = default;

			/*!
			 * \brief Get an interface that prevents popping messages
			 */
			MessageQueueInterface<Args...> &GetInterface()
			{
				return *this;
			}

			/*!
			 * \brief Pop first element from list
			 * \return Returns structure with all elements
			 */
			msg_struct_t Pop()
			{
				auto tmp = std::move(this->_Messages.front());

				this->_Messages.pop_front();

				return tmp;
			}

		private:
			friend class TestMessageQueue;

			template<class U>
			friend class ::TestingClass;
	};
} // namespace message_queue


#endif // MESSAGE_QUEUE_H
