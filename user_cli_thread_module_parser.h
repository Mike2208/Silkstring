#ifndef USER_CLI_THREAD_MODULE_PARSER_H
#define USER_CLI_THREAD_MODULE_PARSER_H

/*! \file user_cli_interface.h
 *  \brief Header for UserCLIInterface class
 */


#include "user_cli_thread_data.h"
#include "user_cli_thread_module_register.h"
#include "user_cli_thread_messages.h"
#include "user_io_thread.h"

#include <getopt.h>

/*!
 *  \brief Namespace for UserCLIInterface class
 */
namespace user_cli_thread_module_parser
{
	using std::array;
	using std::shared_ptr;

	using string_user_id::StringUserID;

	using user_cli_thread_data::cin;
	using user_cli_thread_data::cout;
	using user_cli_thread_data::cerr;
	using user_cli_thread_data::string;

	using namespace user_cli_thread_messages;

	using user_io_thread::UserIOThread;

	using user_cli_thread_module_register::UserCLIThreadModuleRegister;
	using user_cli_thread_module_register::UserCLIRegisterSharedPtr;

	static constexpr char CMDListRegistrationRequests[] = "listrequests";
	static constexpr char CMDGrantRegistrationRequest[] = "grantrequests";
	static constexpr char CMDEchoTest[] = "echo";

	class UserCLIThreadModuleParser : public thread_multi_module_t
	{
			using iterator_t = string::const_iterator;
			using character_t = string::value_type;

		public:
			UserCLIThreadModuleParser(UserIOThread &ThreadManager);

		private:

			/*!
			 * \brief Parses the given string
			 * \param Input User Input as string
			 */
			void ParseInput(const string &Input);

			/*!
			 * \brief Reads the current word and continues
			 * \param Input String Input
			 * \param CurPos Current position, will be incremented until ' ' or '/n' is found
			 * \return Returns word
			 */
			static string ReadWord(const string &Input, iterator_t &CurPos);

			/*!
			 * \brief Skips white space
			 * \param Input input string
			 * \param CurPos Current position in string
			 * \return Returns position after white space
			 */
			static iterator_t SkipWhiteSpace(const string &Input, iterator_t CurPos);

			/*!
			 * \brief Checks whether all characters in Input are numbers
			 */
			static bool IsNumber(const string &Input);

			/*!
			 * \brief Checks whether the given Character is a whitespace
			 * \return  True if whitespace
			 */
			static bool IsWhiteSpace(character_t Character);

			/*!
			 * \brief Checks whether the given Character is part of a word (not whitespace and not line end)
			 */
			static bool IsWord(character_t Character);

			/*!
			 * \brief Checks whether a Character is a hexadecimal value (0-9, a-f, or A-F)
			 */
			static bool IsHexValue(character_t Character);

			/*!
			 * \brief Checks whether a character is numeric
			 */
			static bool IsNumeric(character_t Character);

			static void HandleRead(UserCLIThreadModuleParser *Class, msg_struct_t &Message);

			void HandleMessage(msg_struct_t &Message);

			/*!
			 * \brief Store previous read if it was not yet completed
			 */
			string _InputString;

			UserIOThread &_IOThread;

			UserCLIRegisterSharedPtr _RegistrationModule;

			/*!
			 * \brief Displays a list of requests
			 */
			void OutputRegistrationRequestList();

			/*!
			 * \brief Grants the Registration Request of the given UserID
			 * \param UserID that should be registered
			 */
			void GrantRegistrationRequest(const StringUserID &UserID);

			/*!
			 * \brief Output the received string
			 */
			void EchoString(const string &EchoString);

			/*!
			 * \brief Output an Error to cerr
			 * \param ErrorString
			 */
			void PrintError(string ErrorString);

//			thread_multi_module_fcn_t<2, UserCLIInterface, HandleReadRequest, HandleRead> _Functions;
	};

	using UserCLIThreadModuleParserSharedPtr = shared_ptr<UserCLIThreadModuleParser>;
} // namespace user_cli_thread_module_parser


#endif // USER_CLI_THREAD_MODULE_PARSER_H
