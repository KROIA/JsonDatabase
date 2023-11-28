#pragma once

#include "JD_base.h"
#include <vector>
#include <memory>
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

			AsyncContextDrivenDeleter(const std::vector<std::shared_ptr<T>>& objectsToDelete)
#ifdef JD_ENABLE_MULTITHREADING
				: m_deleterThread(nullptr)
				, m_sharedObjectsToDelete(objectsToDelete)
#else
				: m_sharedObjectsToDelete(objectsToDelete)
#endif
			{ }

			~AsyncContextDrivenDeleter()
			{

#ifdef JD_ENABLE_MULTITHREADING
				m_deleterThread = new std::thread(&AsyncContextDrivenDeleter::deleteObjects, m_objectsToDelete, m_sharedObjectsToDelete);
				m_deleterThread->detach();
				// thread memory is not freed until the program ends
#else
				deleteObjects_internal(m_objectsToDelete);
				deleteObjects_internal(m_sharedObjectsToDelete);
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

			void addObjectsToDelete(std::shared_ptr<T> objectToDelete)
			{
				m_sharedObjectsToDelete.push_back(objectToDelete);
			}
			void addObjectsToDelete(const std::vector<std::shared_ptr<T>>& objectsToDelete)
			{
				m_sharedObjectsToDelete.insert(m_objectsToDelete.end(), objectsToDelete.begin(), objectsToDelete.end());
			}



		private:


			static void deleteObjects(std::vector<T*> ptrList, std::vector<std::shared_ptr<T>> sharedList)
			{
				JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
				deleteObjects_internal(ptrList);
				deleteObjects_internal(sharedList);
			}

			static void deleteObjects_internal(std::vector<T*> objectsToDelete)
			{
				JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
				for (size_t i = 0; i < objectsToDelete.size(); ++i)
					delete objectsToDelete[i];
			}

			static void deleteObjects_internal(std::vector<std::shared_ptr<T>> objectsToDelete)
			{
				JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
				for (size_t i = 0; i < objectsToDelete.size(); ++i)
				{
					T* obj = objectsToDelete[i].get();
					objectsToDelete[i].reset();
					delete obj;
				}
			}

			std::vector<T*> m_objectsToDelete;
			std::vector<std::shared_ptr<T>> m_sharedObjectsToDelete;
#ifdef JD_ENABLE_MULTITHREADING
			std::thread* m_deleterThread;
#endif
		};
	}
}