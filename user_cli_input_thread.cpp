#include "user_cli_input_thread.h"

namespace user_cli_input_thread
{
	UserCLIInputThread::UserCLIInputThread(GlobalMessageQueueThread *GlobalMessageThread)
		: thread_fcn_t(&UserCLIInputThread::ReadFcn, GlobalMessageThread, this),
		  _GlobalMessageThread(GlobalMessageThread),
		  _StopThread(false)
	{}

	UserCLIInputThread::~UserCLIInputThread()
	{
		this->StopThread();
	}

	void UserCLIInputThread::StopThread()
	{
		this->_StopThread = true;
	}

	void UserCLIInputThread::ReadAndProcessInput(GlobalMessageQueueThread *Queue)
	{
		string input;

		// Read line
		//cin >> input;
		getline(cin, input);

		// Create message to send
		thread_multi_module_message_t tmpMessage{identifier_t(UserIOQueueID, UserIOCLIReadModuleID, UserCLIThreadID),
												identifier_t(UserCLIInputQueueID, UserCLIInputModuleID, UserCLIInputThreadID),
												message_t(UserIOCLIReadMessageType, 0),
												message_ptr(new cli_read_message_t{string(std::move(input))})};

		// Send out message
		Queue->PushMessage(tmpMessage);

		// Reset error state
		cin.clear();
	}

	void UserCLIInputThread::ReadFcn(GlobalMessageQueueThread *Queue, UserCLIInputThread *Class)
	{
		// Set function as constructor takes too long
		Class->_StopThread = false;

		do
		{
			// Read input and send it as message to Queue
			ReadAndProcessInput(Queue);

		} while(!Class->_StopThread);
	}
}
