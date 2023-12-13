#include "object/JDObjectManager.h"
#include "object/JDObjectInterface.h"

#include "object/JDObjectRegistry.h"
#include "manager/async/WorkProgress.h"

#ifdef JD_ENABLE_MULTITHREADING
#include <thread>
#endif

namespace JsonDatabase
{
	namespace Internal
	{
		JDObjectManager::JDObjectManager(const JDObject& obj, const JDObjectIDptr& id)
			: m_obj(obj)
			, m_id(id)
			, m_lockstate(Lockstate::unlocked)
			, m_changestate(ChangeState::unchanged)
		{
			m_obj->setManager(this);
		}
		JDObjectManager::~JDObjectManager()
		{
			m_obj->setManager(nullptr);
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

#if JD_ACTIVE_JSON == JD_JSON_QT
		bool JDObjectManager::getJsonArray(const std::vector<JDObject>& objs, std::vector<QJsonObject>& jsonOut)
#elif JD_ACTIVE_JSON == JD_JSON_GLAZE || JD_ACTIVE_JSON == JD_JSON_INTERNAL
		bool JDObjectManager::getJsonArray(const std::vector<JDObject>& objs, JsonArray& jsonOut)
#endif
		{
			return getJsonArray(objs, jsonOut, nullptr);
		}
#if JD_ACTIVE_JSON == JD_JSON_QT
		bool JDObjectManager::getJsonArray(const std::vector<JDObject>& objs,
			std::vector<QJsonObject>& jsonOut,
			WorkProgress* progress)
#elif JD_ACTIVE_JSON == JD_JSON_GLAZE || JD_ACTIVE_JSON == JD_JSON_INTERNAL
		bool JDObjectManager::getJsonArray(const std::vector<JDObject>& objs,
			JsonArray& jsonOut,
			WorkProgress* progress)
#endif
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
#if JD_ACTIVE_JSON == JD_JSON_QT
								QJsonObject jsonData;
								objs[j]->saveInternal(jsonData);
								jsonOut[j] = std::move(jsonData);

#elif JD_ACTIVE_JSON == JD_JSON_GLAZE || JD_ACTIVE_JSON == JD_JSON_INTERNAL
								std::shared_ptr<JsonObject> jsonData = std::make_shared<JsonObject>();
								objs[j]->saveInternal(*jsonData);
								*jsonOut[j] = std::move(jsonData);
#endif
								
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
#if JD_ACTIVE_JSON == JD_JSON_QT
					QJsonObject data;
#elif JD_ACTIVE_JSON == JD_JSON_GLAZE || JD_ACTIVE_JSON == JD_JSON_INTERNAL
					JsonObject data;
#endif
					success &= o->saveInternal(data);
					jsonOut.emplace_back(std::move(data));
					if (progress)
						progress->addProgress(deltaProgress);
				}
			}
			return success;
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

#if JD_ACTIVE_JSON == JD_JSON_QT
		JDObjectManager::ManagedLoadStatus JDObjectManager::managedLoad(
			const QJsonObject& json, 
			JDObjectManager* manager,
			ManagedLoadContainers& containers, 
			const ManagedLoadMode& loadMode,
			const ManagedLoadMisc& misc)
#elif JD_ACTIVE_JSON == JD_JSON_GLAZE || JD_ACTIVE_JSON == JD_JSON_INTERNAL
		JDObjectManager::ManagedLoadStatus JDObjectManager::managedLoad(
			const JsonObject& json,
			JDObjectManager* manager,
			ManagedLoadContainers& containers, 
			const ManagedLoadMode& loadMode,
			const ManagedLoadMisc& misc)
#endif
		{
			if (manager)
				return managedLoadExisting_internal(json, manager, containers, loadMode);
			return managedLoadNew_internal(json, containers, misc);
		}

#if JD_ACTIVE_JSON == JD_JSON_QT
		JDObjectManager::ManagedLoadStatus JDObjectManager::managedLoadExisting_internal(
			const QJsonObject& json,
			JDObjectManager* manager,
			ManagedLoadContainers& containers,
			const ManagedLoadMode& loadMode/*,
			const ManagedLoadMisc& misc*/)
#elif JD_ACTIVE_JSON == JD_JSON_GLAZE || JD_ACTIVE_JSON == JD_JSON_INTERNAL
		JDObjectManager::ManagedLoadStatus JDObjectManager::managedLoadExisting_internal(
			const JsonObject& json,
			JDObjectManager* manager,
			ManagedLoadContainers& containers,
			const ManagedLoadMode& loadMode/*,
			const ManagedLoadMisc& misc*/)
#endif
		{
			JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
			JDObject obj = manager->getObject();
			bool hasChanged = !obj->equalData(json);
			if (!loadMode.changedObjects)
				return ManagedLoadStatus::noLoadNeeded;
			if (hasChanged)
			{
				if (loadMode.overridingObjects)
				{
					//if (misc.hasOverrideChangeFromDatabaseSlots)
					//{
					if (!manager->loadAndOverrideData(json))
						return ManagedLoadStatus::loadFailed;

					containers.overridingObjs.push_back(obj);



					// To do: Simplify
					//containers.loadedObjects[obj] = obj;

					/* }
					else
					{
						if (!manager->loadAndOverrideData(json))
							return ManagedLoadStatus::loadFailed;

						JDObject obj = manager->getObject();
						containers.overridingObjs.push_back(obj);

						// To do: Simplify
						containers.loadedObjects[obj] = obj;
					}*/
				}
				else
				{
					JDObject instance = obj->shallowClone();
					if (!instance.get())
						return ManagedLoadStatus::loadFailed;
					if (!deserializeOverrideFromJson_internal(json, instance))
						return ManagedLoadStatus::loadFailed;

					containers.replaceObjs.push_back(instance);

				}
			}
			containers.loadedObjects[obj] = obj;
			return ManagedLoadStatus::success;
		}

#if JD_ACTIVE_JSON == JD_JSON_QT
		JDObjectManager::ManagedLoadStatus JDObjectManager::managedLoadNew_internal(
			const QJsonObject& json,
			ManagedLoadContainers& containers/*,
			const ManagedLoadMode& loadMode*/,
			const ManagedLoadMisc& misc)
#elif JD_ACTIVE_JSON == JD_JSON_GLAZE || JD_ACTIVE_JSON == JD_JSON_INTERNAL
		JDObjectManager::ManagedLoadStatus JDObjectManager::managedLoadNew_internal(
			const JsonObject& json,
			ManagedLoadContainers& containers/*,
			const ManagedLoadMode& loadMode*/,
			const ManagedLoadMisc& misc)
#endif
		{
			JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
			JDObject templateObj = JDObjectRegistry::getObjectDefinition(json);
			if (!templateObj.get())
			{
#ifdef JD_DEBUG
				std::string objType = JDObjectRegistry::getObjectTypeString(json);
				JD_CONSOLE_FUNCTION("Can't find object definition for object type: \"" << objType << "\"\n");
#endif
				return ManagedLoadStatus::loadFailed_UnknownObjectType;
			}
			/*JDObjectID::IDType id;
#ifndef JD_USE_QJSON
			if (!json.getInt(id, JDObjectInterface::s_tag_objID))
#elif JD_ACTIVE_JSON == JD_JSON_GLAZE || JD_ACTIVE_JSON == JD_JSON_INTERNAL
			if (!JDSerializable::getJsonValue(json, id, JDObjectInterface::s_tag_objID))
#endif
			{
#ifndef JD_USE_QJSON
				JD_CONSOLE_FUNCTION("Objet has incomplete data. Type: \""<< templateObj->className()<<"\" Key: \""
					<< JDObjectInterface::s_tag_objID << "\" is missed\n"
					<< "Object: \"" << json<< "\"\n");
#elif JD_ACTIVE_JSON == JD_JSON_GLAZE || JD_ACTIVE_JSON == JD_JSON_INTERNAL
				JD_CONSOLE_FUNCTION("Objet has incomplete data. Type: \"" << templateObj->className() << "\" Key: \""
					<< JDObjectInterface::s_tag_objID.toStdString() << "\" is missed\n");
#endif
				return ManagedLoadStatus::loadFailed_IncompleteData;
			}
			*/
			JDObject instance = templateObj->shallowClone();
			instance->loadInternal(json);
			containers.newObjIDs.push_back(misc.id);
			containers.newObjInstances.push_back(instance);

			return ManagedLoadStatus::success;
		}


#if JD_ACTIVE_JSON == JD_JSON_QT
		bool JDObjectManager::loadAndOverrideData(const QJsonObject& json)
#elif JD_ACTIVE_JSON == JD_JSON_GLAZE || JD_ACTIVE_JSON == JD_JSON_INTERNAL
		bool JDObjectManager::loadAndOverrideData(const JsonObject& json)
#endif
		{
			JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
			return deserializeOverrideFromJson_internal(json, m_obj);
		}

#if JD_ACTIVE_JSON == JD_JSON_QT
		bool JDObjectManager::loadAndOverrideDataIfChanged(const QJsonObject& json, bool& hasChangesOut)
#elif JD_ACTIVE_JSON == JD_JSON_GLAZE || JD_ACTIVE_JSON == JD_JSON_INTERNAL
		bool JDObjectManager::loadAndOverrideDataIfChanged(const JsonObject& json, bool& hasChangesOut)
#endif
		{
			JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
			return deserializeOverrideFromJsonIfChanged_internal(json, m_obj, hasChangesOut);
		}

#if JD_ACTIVE_JSON == JD_JSON_QT
		JDObjectManager* JDObjectManager::instantiateAndLoadObject(const QJsonObject& json, const JDObjectIDptr& id)
#elif JD_ACTIVE_JSON == JD_JSON_GLAZE || JD_ACTIVE_JSON == JD_JSON_INTERNAL
		JDObjectManager* JDObjectManager::instantiateAndLoadObject(const JsonObject& json, const JDObjectIDptr& id)
#endif
		{
			JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
			JDObject templateObj = JDObjectRegistry::getObjectDefinition(json);
			if (!templateObj.get())
			{
#ifdef JD_DEBUG
				std::string objType = JDObjectRegistry::getObjectTypeString(json);

				JD_CONSOLE_FUNCTION("Can't find object definition for object: \"" 
					<< id->toString() << "\" type: \"" << objType << "\"\n");

#endif
				return nullptr; // Object type not defined
			}

			return cloneAndLoadObject(templateObj, json, id);
		}
#if JD_ACTIVE_JSON == JD_JSON_QT
		JDObjectManager* JDObjectManager::cloneAndLoadObject(const JDObject& original, const QJsonObject& json, const JDObjectIDptr& id)
#elif JD_ACTIVE_JSON == JD_JSON_GLAZE || JD_ACTIVE_JSON == JD_JSON_INTERNAL
		JDObjectManager* JDObjectManager::cloneAndLoadObject(const JDObject& original, const JsonObject& json, const JDObjectIDptr& id)
#endif
		{
			JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
			JDObject instance = original->shallowClone();
			JDObjectManager* manager = new JDObjectManager(original, id);
			if (!manager->loadAndOverrideData(json))
			{
				delete manager;
				manager = nullptr;
			}
			return manager;
		}


#if JD_ACTIVE_JSON == JD_JSON_QT
		bool JDObjectManager::deserializeOverrideFromJsonIfChanged_internal(const QJsonObject& json, JDObject obj, bool& hasChangedOut)
#elif JD_ACTIVE_JSON == JD_JSON_GLAZE || JD_ACTIVE_JSON == JD_JSON_INTERNAL
		bool JDObjectManager::deserializeOverrideFromJsonIfChanged_internal(const JsonObject& json, JDObject obj, bool& hasChangedOut)
#endif
		{
			JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
			if (!obj->equalData(json))
			{
				hasChangedOut = true;
				return true;
			}
			if (!obj->loadInternal(json))
			{
				JD_CONSOLE_FUNCTION("Can't load data in object: " << obj->getObjectID() << " classType: " << obj->className() + "\n");
				return false;
			}
			return true;
		}
#if JD_ACTIVE_JSON == JD_JSON_QT
		bool JDObjectManager::deserializeOverrideFromJson_internal(const QJsonObject& json, JDObject obj)
#elif JD_ACTIVE_JSON == JD_JSON_GLAZE || JD_ACTIVE_JSON == JD_JSON_INTERNAL
		bool JDObjectManager::deserializeOverrideFromJson_internal(const JsonObject& json, JDObject obj)
#endif
		{
			JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
			if (!obj->loadInternal(json))
			{
				JD_CONSOLE_FUNCTION("Can't load data in object: " << obj->getObjectID() << " classType: " << obj->className() + "\n");
				return false;
			}
			return true;
		}
	}
}