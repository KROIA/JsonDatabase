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
#endif

			{ 
				addObjectsToDelete(objectToDelete);
			}

			AsyncContextDrivenDeleter(const std::shared_ptr<T>& objectToDelete)
#ifdef JD_ENABLE_MULTITHREADING
				: m_deleterThread(nullptr)
#endif

			{
				addObjectsToDelete(objectToDelete);
			}
			AsyncContextDrivenDeleter(const std::vector<T*> &objectsToDelete)
#ifdef JD_ENABLE_MULTITHREADING
				: m_deleterThread(nullptr)
#endif
			{
				addObjectsToDelete(objectsToDelete);
				//objectsToDelete.clear();
			}

			AsyncContextDrivenDeleter(const std::vector<std::shared_ptr<T>> &objectsToDelete)
#ifdef JD_ENABLE_MULTITHREADING
				: m_deleterThread(nullptr)
#endif
			{ 
				addObjectsToDelete(objectsToDelete);
			}

			~AsyncContextDrivenDeleter()
			{

#ifdef JD_ENABLE_MULTITHREADING
				m_deleterThread = new std::thread(&AsyncContextDrivenDeleter::deleteObjects, m_objectsToDelete, m_sharedPtrObjectsToDelete);
				m_objectsToDelete.clear();
				m_deleterThread->detach();
				// thread memory is not freed until the program ends
#elif JD_ACTIVE_JSON == JD_JSON_GLAZE || JD_ACTIVE_JSON == JD_JSON_INTERNAL
				deleteObjects_internal(m_objectsToDelete);
#endif
			}

			void addObjectsToDelete(T* objectToDelete)
			{
				m_objectsToDelete.push_back(objectToDelete);
				//objectToDelete = nullptr;
			}
			void addObjectsToDelete(const std::vector<T*>& objectsToDelete)
			{
				m_objectsToDelete.insert(m_objectsToDelete.end(), objectsToDelete.begin(), objectsToDelete.end());
				//objectsToDelete.clear();
			}

			void addObjectsToDelete(const std::shared_ptr<T> &objectToDelete)
			{
				//m_objectsToDelete.push_back(objectToDelete);
				m_sharedPtrObjectsToDelete.push_back(objectToDelete);
				//objectToDelete.reset();
			}
			void addObjectsToDelete(const std::vector<std::shared_ptr<T>>& objectsToDelete)
			{
				m_sharedPtrObjectsToDelete.insert(m_sharedPtrObjectsToDelete.end(), objectsToDelete.begin(), objectsToDelete.end());
			}



		private:


			static void deleteObjects(std::vector<T*> ptrList,
				std::vector<std::shared_ptr<T>> sharedList)
			{
				JD_ASYNC_DELETER_PROFILING_THREAD("Async deleter");
				JD_ASYNC_DELETER_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
				deleteObjects_internal(ptrList);
				for(auto &obj : sharedList)
				{
					if (obj.use_count() == 1)
					{
						obj.reset();
						//T* objPtr = obj.get();
					}
					else
					{
						//JD_LOG_WARNING("Object is still in use. Object will not be deleted.");
						//int a = 0;
					}
				}
			}

			static void deleteObjects_internal(std::vector<T*> objectsToDelete)
			{
				JD_ASYNC_DELETER_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
				for (size_t i = 0; i < objectsToDelete.size(); ++i)
					delete objectsToDelete[i];
			}

			std::vector<T*> m_objectsToDelete;
			std::vector<std::shared_ptr<T>> m_sharedPtrObjectsToDelete;
#ifdef JD_ENABLE_MULTITHREADING
			std::thread* m_deleterThread;
#endif
		};
	}
}