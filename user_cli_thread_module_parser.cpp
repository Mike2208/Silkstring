#include "user_cli_thread_module_parser.h"

namespace user_cli_thread_module_parser
{
	using silkstring_message::MessageDataNum;

	UserCLIThreadModuleParser::UserCLIThreadModuleParser(UserIOThread &ThreadManager)
		: thread_multi_module_t(identifier_t(UserIOQueueID, UserIOCLIReadModuleID, UserCLIThreadID)),
		  _IOThread(ThreadManager),
		  _RegistrationModule(new UserCLIThreadModuleRegister())
	{
		this->_InputString.resize(100);
		ThreadManager.Register(this->_RegistrationModule);
	}

	void UserCLIThreadModuleParser::ParseInput(const string &Input)
	{
		iterator_t curPos = Input.begin();

		// Skip trailing white space
		curPos = UserCLIThreadModuleParser::SkipWhiteSpace(Input, curPos);

		// Read command
		string command = UserCLIThreadModuleParser::ReadWord(Input, curPos);

		// Check that a command was read
		if(command.size() > 0)
		{
			if(command.compare(CMDListRegistrationRequests) == 0)
			{
				this->OutputRegistrationRequestList();
			}
			else if(command.compare(CMDGrantRegistrationRequest) == 0)
			{
				// Skip trailing white space
				curPos = UserCLIThreadModuleParser::SkipWhiteSpace(Input, curPos);

				string number = UserCLIThreadModuleParser::ReadWord(Input, curPos);

				StringUserID userID;

				// Check whether this is a valid string
				if(StringUserID::IsValidString(number))
				{
					userID.ImportFromHexString(number);
				}
				else if(UserCLIThreadModuleParser::IsNumber(number))
				{
					// If not a StringUserID but a number, check whether it is a vector position
					UserCLIThreadModuleRegister::registration_request_vector_t::size_type vectorPos = std::stoi(number, nullptr, 10);

					const auto &requestVector = this->_RegistrationModule->GetRegistrationRequests();
					if(vectorPos < requestVector.size())
					{
						userID = requestVector.at(vectorPos).NewUserID;
					}
				}
				else
					this->PrintError("Invalid argument\n");
			}
			else if(command.compare(CMDEchoTest) == 0)
			{
				curPos = UserCLIThreadModuleParser::SkipWhiteSpace(Input, curPos);

				this->EchoString(Input.substr(curPos-Input.begin(), Input.end()-curPos));
			}
		}
	}

	UserCLIThreadModuleParser::iterator_t UserCLIThreadModuleParser::SkipWhiteSpace(const string &Input, iterator_t CurPos)
	{
		auto newPos = CurPos;

		while(newPos != Input.end() && UserCLIThreadModuleParser::IsWhiteSpace(*newPos))
		{
			newPos++;
		}

		return newPos;
	}

	bool UserCLIThreadModuleParser::IsNumber(const string &Input)
	{
		for(const auto curChar : Input)
		{
			if(!UserCLIThreadModuleParser::IsNumeric(curChar))
				return false;
		}

		return true;
	}

	string UserCLIThreadModuleParser::ReadWord(const string &Input, iterator_t &CurPos)
	{
		string word;

		// Continue reading string until word end is found
		while(CurPos != Input.end() && UserCLIThreadModuleParser::IsWord(*CurPos))
		{
			word.push_back(*CurPos);

			CurPos++;
		}

		return word;
	}

	bool UserCLIThreadModuleParser::IsWhiteSpace(string::value_type Character)
	{
		if(std::isspace(static_cast<unsigned char>(Character)))
			return true;

		return false;
	}

	bool UserCLIThreadModuleParser::IsWord(character_t Character)
	{
		if(Character != ' ' &&
				std::isprint(static_cast<unsigned char>(Character)))
			return true;

		return false;
	}

	bool UserCLIThreadModuleParser::IsHexValue(character_t Character)
	{
		if(std::isxdigit(static_cast<unsigned char>(Character)))
			return true;

		return false;
	}

	bool UserCLIThreadModuleParser::IsNumeric(character_t Character)
	{
		if(std::isdigit(static_cast<unsigned char>(Character)))
			return true;

		return false;
	}

	void UserCLIThreadModuleParser::HandleRead(UserCLIThreadModuleParser *Class, msg_struct_t &Message)
	{
		const auto &inputData = *(Message.Get<MessageDataNum>().Get<cli_read_message_t>());

		Class->ParseInput(inputData.InputString);
	}

	void UserCLIThreadModuleParser::HandleMessage(msg_struct_t &Message)
	{
		thread_multi_module_fcn_t<UserCLIThreadModuleParser, HandleRead>::HandleMessages(this, Message);
	}

	void UserCLIThreadModuleParser::OutputRegistrationRequestList()
	{
		const auto &requestVector = this->_RegistrationModule->GetRegistrationRequests();
		for(auto curRequestIterator = requestVector.begin(); curRequestIterator != requestVector.end(); ++curRequestIterator)
		{
			// Output request number
			cout << curRequestIterator-requestVector.begin() << ": ";

			// Output User ID of request
			cout << curRequestIterator->NewUserID.ConvertToString() << "\n";

			// Output extra info
			cout << curRequestIterator->ExtraInfo << "\n" << "\n";

			cout.flush();
		}
	}

	void UserCLIThreadModuleParser::GrantRegistrationRequest(const StringUserID &UserID)
	{
		this->_IOThread.PushMessage(UserIORegistrationID, this->GetID(), message_t(UserIORegistrationAnswerMessageType, 0), message_ptr(new registration_answer_message_t(UserID, true)));
		// Push request grant to UserIO

		// Also check if any module requested an answer
		for(const auto &curRequest : this->_RegistrationModule->GetRegistrationRequests())
		{
			if(curRequest.NewUserID == UserID)
			{
				for(const auto &curModuleID : curRequest.ReturnModules)
					this->_IOThread.PushMessage(curModuleID, this->GetID(), message_t(UserIORegistrationAnswerMessageType, 1), message_ptr(new registration_answer_message_t{UserID, true}));
			}
		}
	}

	void UserCLIThreadModuleParser::EchoString(const string &EchoString)
	{
		cout << EchoString;
		cout.flush();
	}

	void UserCLIThreadModuleParser::PrintError(string ErrorString)
	{
		cerr << ErrorString << std::endl;
	}
}
