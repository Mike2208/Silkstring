#include "user_cli_thread.h"

namespace user_cli_thread
{
	UserCLIThread::UserCLIThread()
		: thread_multi_module_manager_t(UserIOQueueID, UserCLIThreadID)
	{}
}
