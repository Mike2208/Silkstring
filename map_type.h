#ifndef MAP_TYPE_H
#define MAP_TYPE_H

/*! \file map_type.h
 *  \brief Header for MapType class
 */


#include <map>

/*!
 *  \brief Namespace for MapType class
 */
namespace map_type
{
	using std::map;

	class TestMapType;

	/*!
	 * \brief The MapType class
	 */
	template<class KeyType, class ValueType>
	class MapType : public map<KeyType, ValueType>
	{
			using map_t = map<KeyType, ValueType>;
		public:
			/*!
			 * \brief Constructor
			 */
			MapType() = default;

			typename map_t::iterator Find(const KeyType &Key)
			{
				return this->find(Key);
			}

			template<class FcnValueType>
			void Register(typename map_t::iterator &Position, FcnValueType &&Value)
			{
				static_assert(std::is_convertible<typename std::remove_reference<FcnValueType>::type, ValueType>::value, "ERROR MapType::Register(): Parameter and template parameters must match");
				Position->second = std::forward<FcnValueType>(Value);
			}

			void Unregister(const KeyType &Position)
			{
				auto dataIterator = this->Find(Position);
				if(dataIterator != this->end())
					return this->Unregister(dataIterator);
			}

			void Unregister(typename map_t::iterator &Position)
			{
				this->erase(Position);
			}

			template<class FcnValueType>
			void Register(const KeyType &Key, FcnValueType &&Value)
			{
				static_assert(std::is_convertible<typename std::remove_reference<FcnValueType>::type, ValueType>::value, "ERROR MapType::Register(): Parameter and template parameters must match");

				// Find key
				auto curElementIterator = this->Find(Key);

				// Create new element or change existing one
				if(curElementIterator == this->end())
					this->emplace(Key, std::forward<FcnValueType>(Value));
				else
					this->Register(curElementIterator, std::forward<FcnValueType>(Value));
			}

			template<class T>
			void Unregister(typename T::key_type Key)
			{
				// Find key
				auto curElementIterator = this->Find(Key);

				// Erase iterator if it exists
				if(curElementIterator != this->End())
					this->Unregister(curElementIterator);
			}

			using iterator = typename map_t::iterator;

		private:
			friend class TestMapType;
	};
} // namespace map_type


#endif // MAP_TYPE_H
