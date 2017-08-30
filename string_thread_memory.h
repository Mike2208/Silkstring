#ifndef STRING_THREAD_MEMORY_H
#define STRING_THREAD_MEMORY_H

/*! \file string_memory.h
 *  \brief Header for StringMemory class
 */


#include "string_user_storage.h"

#include "global_message_queue_thread.h"

/*!
 *  \brief Namespace for StringThreadMemory class
 */
namespace string_thread_memory
{
	using string_user_storage::StringUserStorage;

	using global_message_queue_thread::GlobalMessageQueueThread;

	/*!
	 * \brief Memory used in the StringManager thread
	 */
	class StringThreadMemory
	{
		public:

			/*!
			 * \brief Pointer to global message queue
			 */
			GlobalMessageQueueThread *GlobalQueue;

			/*!
			 * \brief Contains all StringUser and StringUserAdmin structs
			 */
			StringUserStorage UserStorage = StringUserStorage();

		private:
	};
} // namespace string_thread_memory


#endif // STRING_THREAD_MEMORY_H
