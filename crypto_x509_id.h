#ifndef CRYPTO_X509_ID_H
#define CRYPTO_X509_ID_H

/*! \file crypto_x509_id.h
 *  \brief Header for X509ID class
 */


#include <string>
#include <array>

#include "crypto_header.h"
//#include "vector_t.h"

/*!
 *  \brief Namespace for X509ID class
 */
namespace crypto_x509_id
{
	//using vector_t::byte_vector_t;
	using std::array;
	using byte_t = uint8_t;

	using std::string;

	using crypto_header::DefIDSize;

	static constexpr size_t IDLength = DefIDSize;
	using id_array_t = array<byte_t, IDLength>;

	/*!
	 *	\brief Error ID. All 0
	 */
	static constexpr id_array_t ErrorIDBytes{};

	class TestX509ID;

	/*!
	 * \brief The X509ID class
	 */
	class X509ID
	{
		public:
			using id_array_t = crypto_x509_id::id_array_t;

			static constexpr id_array_t::size_type IDLength = crypto_x509_id::IDLength;
			static constexpr string::size_type StringIDLength = IDLength*2;

			/*!
			 * \brief Constructor
			 */
			X509ID() = default;

			static X509ID ImportFromIDArray(id_array_t &&IDArray) noexcept;

			/*!
			 * \brief Imports ID from hex string
			 */
			static X509ID ImportFromHexString(const string &HexString);

			/*!
			 * \brief GetID
			 * \return Returns ID
			 */
			const id_array_t &GetID() const noexcept;

			/*!
			 * \brief Converts ID from bytes to string (all byte values are converted to two hexadecimal chars)
			 * \return Returns a string
			 */
			string ConvertToString() const;

			/*!
			 * \brief Checks whether a string is valid (Has length StringIDLength and all characters are alphanumeric)
			 */
			static bool IsValidString(const string &StringToCheck);

			bool operator==(const X509ID &Other) const noexcept;
			bool operator!=(const X509ID &Other) const noexcept;

			/*!
			 * \brief Checks whether the ID is ErrorID or not
			 * \return
			 */
			bool IsValid() const;

		protected:
			/*!
			 * \brief ID
			 */
			id_array_t _UniqueID = ErrorIDBytes;

		private:

			static id_array_t ConvertStringToByteVector(const string &HexStringID);

			/*!
			 * \brief Constructor
			 * \param StringID ID of certificate as string
			 */
			explicit X509ID(id_array_t &&ID) noexcept;

			/*!
			 * \brief Constructor
			 * \param HexStringID ID as string of hexadecimal values
			 */
			explicit X509ID(const string &HexStringID);

			friend class TestX509ID;
	};

	const auto ErrorID = X509ID::ImportFromIDArray(id_array_t(ErrorIDBytes));
} // namespace crypto_x509_id


#endif // CRYPTO_X509_ID_H
