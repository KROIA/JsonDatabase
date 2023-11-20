#pragma once

#include "JD_base.h"
#include <vector>
#ifdef JD_ENABLE_MULTITHREADING
#include <thread>
#endif

/*
	This Object is used to delete objects in a different thread.
	This can be helpfull when deleting a lot of objects at once or the deletion of an object takes a long time.
	
	The object deletes the objects automaticly if its instances goes out of scope.
*/

namespace JsonDatabase
{
	namespace Internal
	{
		template<class T>
		class AsyncContextDrivenDeleter
		{
		public:
			AsyncContextDrivenDeleter(T* objectToDelete)
#ifdef JD_ENABLE_MULTITHREADING
				: m_deleterThread(nullptr)
				, m_objectsToDelete({ objectToDelete })
#else
				: m_objectsToDelete({ objectToDelete })
#endif

			{ }
			AsyncContextDrivenDeleter(const std::vector<T*>& objectsToDelete)
#ifdef JD_ENABLE_MULTITHREADING
				: m_deleterThread(nullptr)
				, m_objectsToDelete(objectsToDelete)
#else
				: m_objectsToDelete(objectsToDelete)
#endif
			{ }

			~AsyncContextDrivenDeleter()
			{

#ifdef JD_ENABLE_MULTITHREADING
				m_deleterThread = new std::thread(&AsyncContextDrivenDeleter::deleteObjects, m_objectsToDelete);
				m_deleterThread->detach();
				// thread memory is not freed until the program ends
#else
				deleteObjects(m_objectsToDelete);
#endif
			}

			void addObjectsToDelete(T* objectToDelete)
			{
				m_objectsToDelete.push_back(objectToDelete);
			}
			void addObjectsToDelete(const std::vector<T*>& objectsToDelete)
			{
				m_objectsToDelete.insert(m_objectsToDelete.end(), objectsToDelete.begin(), objectsToDelete.end());
			}



		private:

			static void deleteObjects(std::vector<T*> objectsToDelete)
			{
				JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
				for (size_t i = 0; i < objectsToDelete.size(); ++i)
					delete objectsToDelete[i];
			}
			std::vector<T*> m_objectsToDelete;
#ifdef JD_ENABLE_MULTITHREADING
			std::thread* m_deleterThread;
#endif
		};
	}
}