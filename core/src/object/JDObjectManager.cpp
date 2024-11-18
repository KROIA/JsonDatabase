#include "object/JDObjectManager.h"
#include "object/JDObjectInterface.h"

#include "object/JDObjectRegistry.h"
#include "manager/async/WorkProgress.h"

#include "manager/JDManager.h"

#ifdef JD_ENABLE_MULTITHREADING
#include <thread>
#endif

namespace JsonDatabase
{
	namespace Internal
	{
		JDObjectManager::JDObjectManager(JDManager* manager, const JDObject& obj, const JDObjectIDptr& id, Log::LogObject* parentLogger)
			: m_databaseManager(manager)
			, m_obj(obj)
			, m_id(id)
			, m_lockstate(Lockstate::unlocked)
			, m_changestate(ChangeState::unchanged)
		{
			JD_UNUSED(parentLogger);
			//if(parentLogger)
			//	m_logger = parentLogger->createContext("OBJ:"+id.get()->toString());
			m_obj->setManager(this);
		}
		JDObjectManager::~JDObjectManager()
		{
			m_obj->setManager(nullptr);
			delete m_logger;
		}


		const JDObjectIDptr& JDObjectManager::getID() const
		{
			return m_id;
		}
		const JDObject& JDObjectManager::getObject() const
		{
			return m_obj;
		}
		Lockstate JDObjectManager::getLockstate() const
		{
			return m_lockstate;
		}
		ChangeState JDObjectManager::getChangeState() const
		{
			return m_changestate;
		}


		bool JDObjectManager::getJsonArray(const std::vector<JDObject>& objs, JsonArray& jsonOut)
		{
			return getJsonArray(objs, jsonOut, nullptr);
		}
		bool JDObjectManager::getJsonArray(const std::vector<JDObject>& objs,
			JsonArray& jsonOut,
			WorkProgress* progress)
		{
			JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
			if (objs.size() == 0)
			{
				if(progress)
					progress->setProgress(1.0);
				return true;
			}

			bool success = true;
			double deltaProgress = 1.0 / (double)objs.size();
#ifdef JD_ENABLE_MULTITHREADING
			unsigned int threadCount = std::thread::hardware_concurrency();
			if (threadCount > 100)
				threadCount = 100;
			size_t objCount = objs.size();
			if (threadCount > objCount)
				threadCount = objCount;
			if (objCount > 100 && threadCount)
			{
				struct ThreadData
				{
					size_t start;
					size_t end;
					std::atomic<int> finishCount;
				};
				std::vector<std::thread*> threads(threadCount, nullptr);
				std::vector<ThreadData> threadData(threadCount);
				size_t chunkSize = objCount / threadCount;
				size_t remainder = objCount % threadCount;
				size_t start = 0;
				jsonOut.resize(objCount);


				for (size_t i = 0; i < threadCount; ++i)
				{
					threadData[i].start = start;
					threadData[i].end = start + chunkSize;
					threadData[i].finishCount = 0;
					start += chunkSize;
					if (i == threadCount - 1)
						threadData[i].end += remainder;
					threads[i] = new std::thread([&threadData, &objs, i, &jsonOut]()
						{
							ThreadData& data = threadData[i];
							std::atomic<int> & finishCount = threadData[i].finishCount;
							for (size_t j = data.start; j < data.end; ++j)
							{

								std::shared_ptr<JsonObject> jsonData = std::make_shared<JsonObject>();
								objs[j]->saveInternal(*jsonData);
								*jsonOut[j] = std::move(jsonData);
								
								finishCount++;
							}
						});
				}

				// Create a progress updater Thread
				std::thread* progressUpdater = nullptr;
				std::atomic<bool> progressUpdaterRunning = false;
				if (progress)
				{
					progressUpdaterRunning = true;
					size_t objectCount = objs.size();
					progressUpdater = new std::thread([&threadData, progress, deltaProgress, &progressUpdaterRunning, objectCount]()
						{
							while (progressUpdaterRunning.load())
							{
								int finishCount = 0;
								for (size_t i = 0; i < threadData.size(); ++i)
								{
									finishCount += threadData[i].finishCount;
								}

								progress->setProgress((double)finishCount * deltaProgress);
								if(finishCount < objectCount/2) // Only sleep if it is not almost finished
									std::this_thread::sleep_for(std::chrono::milliseconds(1));
							}
						});
				}

				// Wait for all threads to finish
				for (size_t i = 0; i < threadCount; ++i)
				{
					threads[i]->join();
					delete threads[i];
				}
				if(progressUpdater)
				{
					progressUpdaterRunning = false;
					progressUpdater->join();
					delete progressUpdater;
					progressUpdater = nullptr;
				}
			}
			else
#endif
			{
				jsonOut.reserve(objs.size());
				for (auto o : objs)
				{
					JsonObject data;
					success &= o->saveInternal(data);
					jsonOut.emplace_back(std::move(data));
					if (progress)
						progress->addProgress(deltaProgress);
				}
			}
			return success;
		}

		bool JDObjectManager::isLocked() const
		{
			if (m_databaseManager)
			{
				Error err;
				return m_databaseManager->isObjectLocked(getObject(), err);
			}
			return false;
		}
		bool JDObjectManager::lock()
		{
			if (m_databaseManager)
			{
				Error err;
				return m_databaseManager->lockObject(getObject(), err);
			}
			return false;
		}
		bool JDObjectManager::unlock()
		{
			if (m_databaseManager)
			{
				Error err;
				return m_databaseManager->unlockObject(getObject(), err);
			}
			return false;
		}
		Utilities::JDUser JDObjectManager::getLockOwner(bool& isLocked) const
		{
			if (m_databaseManager)
			{
				Utilities::JDUser userOut; 
				Error err;
				isLocked = m_databaseManager->getLockOwner(getObject(), userOut, err);
				return userOut;
			}
			isLocked = false;
			return Utilities::JDUser();
		}
		bool JDObjectManager::saveToDatabase()
		{
			if (m_databaseManager)
			{
				return m_databaseManager->saveObject(getObject());
			}
			return false;
		}
		void JDObjectManager::saveToDatabaseAsync()
		{
			if (m_databaseManager)
			{
				m_databaseManager->saveObjectAsync(getObject());
			}
		}
		bool JDObjectManager::loadFromDatabase()
		{
			if(m_databaseManager)
			{
				return m_databaseManager->loadObject(getObject());
			}
			return false;
		}
		void JDObjectManager::loadFromDatabaseAsync()
		{
			if (m_databaseManager)
			{
				m_databaseManager->loadObjectAsync(getObject());
			}
		}

		const std::string& JDObjectManager::managedLoadStatusToString(ManagedLoadStatus status)
		{
			
			const static std::string s_success = "success";
			const static std::string s_noLoadNeeded = "noLoadNeeded";
			const static std::string s_loadFailed = "loadFailed";
			const static std::string s_loadFailed_UnknownObjectType = "loadFailed_UnknownObjectType";
			const static std::string s_loadFailed_IncompleteData = "loadFailed_IncompleteData";
			

			switch (status)
			{
			case ManagedLoadStatus::success:
				return s_success;
			case ManagedLoadStatus::noLoadNeeded:
				return s_noLoadNeeded;
			case ManagedLoadStatus::loadFailed:
				return s_loadFailed;
			case ManagedLoadStatus::loadFailed_UnknownObjectType:
				return s_loadFailed_UnknownObjectType;
			case ManagedLoadStatus::loadFailed_IncompleteData:
				return s_loadFailed_IncompleteData;
			}

			static std::string s_undef = "";
			s_undef = "unknown JDObjectManager::ManagedLoadStatus("+std::to_string((int)status)+")";
			return s_undef;
		}
		JDObjectManager::ManagedLoadStatus JDObjectManager::managedLoad(
			const JsonObject& json,
			JDObjectManager* manager,
			ManagedLoadContainers& containers, 
			const ManagedLoadMode& loadMode,
			const ManagedLoadMisc& misc,
			Log::LogObject* logger)
		{
			if (manager)
				return managedLoadExisting_internal(json, manager, containers, loadMode, logger);
			return managedLoadNew_internal(json, containers, misc, logger);
		}
		JDObjectManager::ManagedLoadStatus JDObjectManager::managedLoadExisting_internal(
			const JsonObject& json,
			JDObjectManager* manager,
			ManagedLoadContainers& containers,
			const ManagedLoadMode& loadMode,
			Log::LogObject *logger)
		{
			JD_UNUSED(logger);
			JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
			JDObject obj = manager->getObject();
			bool hasChanged = !obj->equalData(json);
			if (!loadMode.changedObjects)
				return ManagedLoadStatus::noLoadNeeded;
			if (hasChanged)
			{
				//if (loadMode.overridingObjects)
				//{
					
					if (!manager->loadAndOverrideData(json))
						return ManagedLoadStatus::loadFailed;

					containers.overridingObjs.push_back(obj);
				//}
				/*else
				{
					JDObject instance = obj->deepClone();
					if (!instance.get())
						return ManagedLoadStatus::loadFailed;
					if (!manager->deserializeOverrideFromJson_internal(json, instance))
						return ManagedLoadStatus::loadFailed;

					containers.changedPairs.push_back(std::make_pair(obj, instance));
					containers.replaceObjs.push_back(instance);

				}*/
			}
			containers.loadedObjects[obj] = obj;
			return ManagedLoadStatus::success;
		}
		JDObjectManager::ManagedLoadStatus JDObjectManager::managedLoadNew_internal(
			const JsonObject& json,
			ManagedLoadContainers& containers/*,
			const ManagedLoadMode& loadMode*/,
			const ManagedLoadMisc& misc,
			Log::LogObject* logger)
		{
			JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
			JDObject templateObj = JDObjectRegistry::getObjectDefinition(json);
			if (!templateObj.get())
			{
#ifdef JD_DEBUG
				std::string objType = JDObjectRegistry::getObjectTypeString(json);
				if(logger)logger->logError("Can't find object definition for object type: \"" + objType + "\"\n");
#else
				JD_UNUSED(logger);
#endif
				return ManagedLoadStatus::loadFailed_UnknownObjectType;
			}
			JDObject instance = templateObj->shallowClone();
			instance->loadInternal(json);
			containers.newObjIDs.push_back(misc.id);
			containers.newObjInstances.push_back(instance);

			return ManagedLoadStatus::success;
		}



		bool JDObjectManager::loadAndOverrideData(const JsonObject& json)
		{
			JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
			return deserializeOverrideFromJson_internal(json, m_obj);
		}

		bool JDObjectManager::loadAndOverrideDataIfChanged(const JsonObject& json, bool& hasChangesOut)
		{
			JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
			return deserializeOverrideFromJsonIfChanged_internal(json, m_obj, hasChangesOut);
		}

		/*JDObjectManager* JDObjectManager::instantiateAndLoadObject(const JsonObject& json, const JDObjectIDptr& id, Log::LogObject* parentLogger)
		{
			JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
			JDObject templateObj = JDObjectRegistry::getObjectDefinition(json);
			if (!templateObj.get())
			{
#ifdef JD_DEBUG
				std::string objType = JDObjectRegistry::getObjectTypeString(json);
				if(parentLogger)parentLogger->logError("Can't find object definition for object: \"" + id->toString() + "\" type: \"" + objType + "\"\n");
#endif
				return nullptr; // Object type not defined
			}

			return cloneAndLoadObject(templateObj, json, id, parentLogger);
		}
		JDObjectManager* JDObjectManager::cloneAndLoadObject(const JDObject& original, const JsonObject& json, const JDObjectIDptr& id, Log::LogObject* parentLogger)
		{
			JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
			JDObject instance = original->shallowClone();
			JDObjectManager* manager = new JDObjectManager(original, id, parentLogger);
			if (!manager->loadAndOverrideData(json))
			{
				delete manager;
				manager = nullptr;
			}
			return manager;
		}
		*/
		bool JDObjectManager::deserializeOverrideFromJsonIfChanged_internal(const JsonObject& json, JDObject obj, bool& hasChangedOut)
		{
			JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
			if (!obj->equalData(json))
			{
				hasChangedOut = true;
				return true;
			}
			if (!obj->loadInternal(json))
			{
				if (m_logger)m_logger->logError("Can't load data in object: " + obj->getObjectID().get()->toString() + " classType: " + obj->className());
				return false;
			}
			return true;
		}
		bool JDObjectManager::deserializeOverrideFromJson_internal(const JsonObject& json, JDObject obj)
		{
			JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
			if (!obj->loadInternal(json))
			{
				if (m_logger)m_logger->logError("Can't load data in object: " + obj->getObjectID().get()->toString() + " classType: " + obj->className());
				return false;
			}
			return true;
		}
	}
}