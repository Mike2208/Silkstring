#ifndef DYNAMIC_POINTER_H
#define DYNAMIC_POINTER_H

/*! \file dynamic_pointer.h
 *  \brief Header for DynamicPointer class
 */


#include <memory>
#include "error_exception.h"

#include "debug_flag.h"

#ifdef DEBUG
#include <typeinfo>
#endif

/*!
 *  \brief Namespace for DynamicPointer class
 */
namespace dynamic_pointer
{
	using error_exception::Exception;
	using error_exception::ERROR_NUM;

	using std::shared_ptr;

	using type_number_t = unsigned int;

	class TestDynamicPointer;
	class TestSharedDynamicPointer;
	class TestSharedDynamicPointerTyped;

	template<class T>
	class DynamicPointer;

	template<>
	class DynamicPointer<void>
	{
		protected:
			using destructor_fcn_t = void(const void*) noexcept;

		public:
			template<class T>
			DynamicPointer(T *MemData)
				: _MemData(MemData),
				  _Destructor( std::is_destructible<T>::value ? [](const void* ClassPtr) { static_cast<const T*>(ClassPtr)->~T(); } : [](const void*){})
			{}

			DynamicPointer(const DynamicPointer &S) = delete;
			DynamicPointer(DynamicPointer &&S)
				: _MemData(std::move(S._MemData)),
				  _Destructor(S._Destructor)
			{
				S._MemData = nullptr;
			}

			DynamicPointer &operator=(const DynamicPointer &S) = delete;
			DynamicPointer &operator=(DynamicPointer &&S) = delete;

			template<class T>
			DynamicPointer<T> ReturnPtr()
			{
				DynamicPointer<T> tmpPtr(reinterpret_cast<T*>(this->_MemData));
				this->_MemData = nullptr;

				return tmpPtr;
			}

			template<class T>
			T *Get()
			{
				return reinterpret_cast<T*>(this->_MemData);
			}

			template<class T>
			const T *Get() const
			{
				return reinterpret_cast<const T*>(this->_MemData);
			}

			~DynamicPointer()
			{
				// Free data if necessary
				if(_MemData != nullptr)
				{
					if(this->_Destructor != nullptr)
						this->_Destructor(this->_MemData);

					free(this->_MemData);
				}
			}

		private:
			/*!
			 * \brief Pointer to data
			 */
			void *_MemData;

			/*!
			 * \brief Destructor to call before erasing memory
			 */
			 destructor_fcn_t *const _Destructor;
	};

	/*!
	 * \brief The DynamicPointer class
	 */
	template<class T>
	class DynamicPointer
	{
		public:			
			/*!
			 * 	\brief Constructor
			 */
			template<class FcnT>
			DynamicPointer(FcnT &&Data)
				: _MemData(malloc(sizeof(FcnT)))
			{
				static_assert(std::is_convertible<FcnT, T>::value, "ERROR DynamicPointer::DynamicPointer(): Function Argument and Parameter Type must be the same");
				*(this->_MemData) = std::forward<T>(Data);
			}

			template<class FcnT>
			DynamicPointer(FcnT *MemPtr)
				: _MemData(MemPtr)
			{
				static_assert(std::is_convertible<FcnT, T>::value, "ERROR DynamicPointer::DynamicPointer(): Function Argument and Parameter Type must be the same");
			}

			DynamicPointer(const DynamicPointer &S) = delete;
			DynamicPointer(DynamicPointer &&S)
				: _MemData(std::move(S._MemData))
			{
				S._MemData = nullptr;
			}

			DynamicPointer &operator=(const DynamicPointer &S) = delete;
			DynamicPointer &operator=(DynamicPointer &&S) = delete;

			~DynamicPointer()
			{
				if(this->_MemData != nullptr)
				{
					free(this->_MemData);
				}
			}

			/*!
			 * \brief ReleasePtr Releases Ownership of this data
			 * \return Returns an undefined pointer that still has the necessary destructor saved
			 */
			DynamicPointer<void> ReleasePtr()
			{
				DynamicPointer<void> tmpPtr(this->_MemData);
				this->_MemData = nullptr;

				return tmpPtr;
			}

			T *Get()
			{
				return this->_MemData;
			}

			const T *Get() const
			{
				return this->_MemData;
			}

		private:
			/*!
			 * \brief Pointer to data
			 */
			T *_MemData;
	};

	template<class T>
	class SharedDynamicPointer;

	template<>
	class SharedDynamicPointer<void>
	{
		public:

			template<class T>
			SharedDynamicPointer(T *MemPtr)
				: _MemData(MemPtr,
						   //&SharedDynamicPointer<void>::Destructor<T>)
						   std::is_destructible<T>::value
								? [](const void *ClassPtr){ if(!ClassPtr) {static_cast<const T*>(ClassPtr)->~T(); delete static_cast<const T*>(ClassPtr); }}
								: [](const void *ClassPtr){ if(!ClassPtr) {delete static_cast<const T*>(ClassPtr); }})
#ifdef DEBUG
				, _DataType(&(typeid(T)))
#endif
			{}

			SharedDynamicPointer()
				: _MemData(),
#ifdef DEBUG
				  _DataType(&(typeid(void)))
#endif
			{}

			~SharedDynamicPointer() = default;

			SharedDynamicPointer(const SharedDynamicPointer &S) = default;
			SharedDynamicPointer &operator=(const SharedDynamicPointer &S) = default;
			SharedDynamicPointer(SharedDynamicPointer &&S) = default;
			SharedDynamicPointer &operator=(SharedDynamicPointer &&S) = default;

			template<class T>
			T *Get()
			{
				return reinterpret_cast<T*>(this->_MemData.get());
			}

			template<class T>
			const T *Get() const
			{
				return reinterpret_cast<const T*>(this->_MemData.get());
			}

#ifdef DEBUG
			std::string PrintType() const
			{
				return _DataType->name();
			}
#endif

		private:

			shared_ptr<void> _MemData;

#ifdef DEBUG
			const std::type_info *_DataType;
#endif

			template<class T>
			static void Destructor(const void *MemData)
			{
				if(MemData != nullptr)
				{
					if(std::is_destructible<T>::value)
					{
						reinterpret_cast<const T*>(MemData)->~T();
					}

					delete reinterpret_cast<const T*>(MemData);
				}
			}
	};

	template<type_number_t N, class ...DataTypes>
	struct type_struct_t
	{
		//using type = void;
	};

	template<type_number_t N, class CurType, class ...DataTypes>
	struct type_struct_t<N, CurType, DataTypes...>
	{
		using type = typename type_struct_t<N-1, DataTypes...>::type;
	};

	template<class CurType, class ...DataTypes>
	struct type_struct_t<0, CurType, DataTypes...>
	{
		using type = CurType;
	};

	struct find_type_number_t
	{
		template<class TypeToFind, type_number_t CurNum, class CurType, class ...DataTypes>
		static constexpr typename std::enable_if<std::is_same<CurType, TypeToFind>::value, type_number_t>::type GetTypeNumber()
		{
			return CurNum;
		}

		template<class TypeToFind, type_number_t CurNum, class CurType, class ...DataTypes>
		static constexpr typename std::enable_if<(!(std::is_same<CurType, TypeToFind>::value) && sizeof...(DataTypes) > 0), type_number_t>::type GetTypeNumber()
		{
			static_assert(sizeof...(DataTypes) > 0, "ERROR FindTypeNumber(): Type not found\n");

			return find_type_number_t::GetTypeNumber<TypeToFind, CurNum+1, DataTypes...>();
		}
	};

	template<class ...DataTypes>
	class SharedDynamicPointerTyped : protected SharedDynamicPointer<void>
	{
			using shared_dynamic_ptr_t = SharedDynamicPointer<void>;
		public:

			template<class T>
			SharedDynamicPointerTyped(T *MemData)
				: shared_dynamic_ptr_t(MemData),
				  _TypeNumber(find_type_number_t::GetTypeNumber<T, 0, DataTypes...>())
			{}

			type_number_t GetTypeNumber() const
			{
				return this->_TypeNumber;
			}

			template<type_number_t ClassNumber>
			typename type_struct_t<ClassNumber, DataTypes...>::type *Get()
			{
				return shared_dynamic_ptr_t::Get<typename type_struct_t<ClassNumber, DataTypes...>::type>();
			}

			template<type_number_t ClassNumber>
			const typename type_struct_t<ClassNumber, DataTypes...>::type *Get() const
			{
				return shared_dynamic_ptr_t::Get<typename type_struct_t<ClassNumber, DataTypes...>::type>();
			}

		private:

			const type_number_t _TypeNumber;
	};
} // namespace dynamic_pointer


#endif // DYNAMIC_POINTER_H
