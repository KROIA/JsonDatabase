#include "manager/JDManager.h"
#include "object/JDObjectInterface.h"
#include "object/JDObjectRegistry.h"
#include "utilities/JDUniqueMutexLock.h"
#include "utilities/SystemCommand.h"
#include "utilities/JsonUtilities.h"
#include "utilities/AsyncContextDrivenDeleter.h"


#include "manager/async/work/JDManagerWorkLoadAllObjects.h"
#include "manager/async/work/JDManagerWorkLoadSingleObject.h"
#include "manager/async/work/JDManagerWorkSaveList.h"
#include "manager/async/work/JDManagerWorkSaveSingle.h"





namespace JsonDatabase
{
    using namespace Internal;
   
    const unsigned int JDManager::s_fileLockTimeoutMs = 1000;


    JDManager::JDManager(const std::string& databasePath,
                         const std::string& databaseName)
        : JDManagerObjectManager(*this, m_mutex)
        , JDManagerFileSystem(databasePath, databaseName, *this, m_mutex)
        //, JDObjectLocker(*this, m_mutex)
        , JDManagerAsyncWorker(*this, m_mutex)
        , m_useZipFormat(false)
        , m_signleEntryUpdateLock(false)
        //, m_signals(*this, m_mutex)
    {
        m_logger  = new Log::LogObject("JDManager");
        JDManagerObjectManager::setParentLogger(m_logger);
        JDManagerFileSystem::setParentLogger(m_logger);
        JDManagerAsyncWorker::setParentLogger(m_logger);
        

        m_user = Utilities::JDUser::generateUser();
        JDManagerObjectManager::setDomainName(m_user.getSessionID());
    }
    JDManager::JDManager(const std::string& databasePath,
        const std::string& databaseName,
        const std::string& user)
        : JDManagerObjectManager(*this, m_mutex)
        , JDManagerFileSystem(databasePath, databaseName, *this, m_mutex)
        //, JDObjectLocker(*this, m_mutex)
        , JDManagerAsyncWorker(*this, m_mutex)
        , m_useZipFormat(false)
        , m_signleEntryUpdateLock(false)
        //, m_signals(*this, m_mutex)
    {
        m_logger = new Log::LogObject("JDManager");
        JDManagerObjectManager::setParentLogger(m_logger);
        JDManagerFileSystem::setParentLogger(m_logger);
        JDManagerAsyncWorker::setParentLogger(m_logger);
        
        m_user = Utilities::JDUser::generateUser(user);
        JDManagerObjectManager::setDomainName(m_user.getSessionID());
    }
    JDManager::JDManager(const JDManager &other)
        : JDManagerObjectManager(*this, m_mutex)
        , JDManagerFileSystem(other.getDatabaseFilePath(), other.getDatabaseName(), *this, m_mutex)
        //, JDObjectLocker(*this, m_mutex)
        , JDManagerAsyncWorker(*this, m_mutex)
        , m_user(other.m_user)
        , m_useZipFormat(other.m_useZipFormat)
        , m_signleEntryUpdateLock(false)
        //, m_signals(*this, m_mutex)
    {
        if (other.m_logger)
        {

            m_logger = new Log::LogObject(other.m_logger->getParentID(),"JDManager");
            JDManagerObjectManager::setParentLogger(m_logger);
            JDManagerFileSystem::setParentLogger(m_logger);
            JDManagerAsyncWorker::setParentLogger(m_logger);
            
        }
        m_user = Utilities::JDUser::generateUser(m_user.getName());
        JDManagerObjectManager::setDomainName(m_user.getSessionID());
    }
JDManager::~JDManager()
{
    stop();
    delete m_logger;
}

bool JDManager::setup()
{
    bool success = true;
    if(m_logger)
        m_logger->logInfo("JDManager::setup() : Setting up JDManager");
    success &= JDManagerFileSystem::setup();
    success &= JDManagerObjectManager::setup();
    JDObjectLocker::Error lockerError;
    JDManagerAsyncWorker::setup();
    if (m_logger)
    {
        if (success)
            m_logger->log("JDManager::setup() : JDManager setup successful", Log::Level::info, Log::Colors::green);
        else
            m_logger->logError("JDManager::setup() : JDManager setup failed");
    }
    return success;
}
bool JDManager::stop()
{
    bool success = true;
    if (m_logger)
		m_logger->logInfo("JDManager::stop() : Stopping JDManager");
	success &= JDManagerFileSystem::stop();
	success &= JDManagerObjectManager::stop();
    JDManagerAsyncWorker::stop();
    if (m_logger)
    {
		if (success)
			m_logger->log("JDManager::stop() : JDManager stopped successfully", Log::Level::info, Log::Colors::green);
		else
			m_logger->logError("JDManager::stop() : JDManager stop failed");
	}
	return success;
}



void JDManager::enableZipFormat(bool enable)
{
    m_useZipFormat = enable;
}
bool JDManager::isZipFormatEnabled() const
{
    return m_useZipFormat;
}

bool JDManager::loadObject(const JDObject &obj)
{
    JDM_UNIQUE_LOCK_P;
    return loadObject_internal(obj, nullptr);
}
void JDManager::loadObjectAsync(const JDObject &obj)
{
    JDManagerAsyncWorker::addWork(std::make_shared<Internal::JDManagerAysncWorkLoadSingleObject>(*this, m_mutex, obj));
}
bool JDManager::loadObjects(int mode)
{
    JDM_UNIQUE_LOCK_P;
	return loadObjects_internal(mode, nullptr);
}
void JDManager::loadObjectsAsync(int mode)
{
    JDManagerAsyncWorker::addWork(std::make_shared < Internal::JDManagerAysncWorkLoadAllObjects>(*this, m_mutex, mode));
}
bool JDManager::saveObject(const JDObject &obj)
{
    JDM_UNIQUE_LOCK_P;
    return saveObject_internal(obj, s_fileLockTimeoutMs, nullptr);
}
void JDManager::saveObjectAsync(const JDObject &obj)
{
    JDManagerAsyncWorker::addWork(std::make_shared < Internal::JDManagerAysncWorkSaveSingle>(*this, m_mutex, obj));
}
bool JDManager::saveObjects()
{
    JDM_UNIQUE_LOCK_P;
    std::vector<JDObject> objs = JDManagerObjectManager::getObjects();
    return saveObjects_internal(objs, s_fileLockTimeoutMs, nullptr);
}
void JDManager::saveObjectsAsync()
{
    std::vector<JDObject> objs = JDManagerObjectManager::getObjects();
    JDManagerAsyncWorker::addWork(std::make_shared < Internal::JDManagerAysncWorkSaveList>(*this, m_mutex, objs, m_logger));
}

bool JDManager::loadObject_internal(const JDObject& obj, Internal::WorkProgress* progress)
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
    if (!JDManagerObjectManager::exists_internal(obj))
        return false;
    if (m_logger)
        m_logger->log("Loading object with ID: " + obj->getObjectID()->toString(), Log::Level::info);

    LockedFileAccessor fileAccessor(getDatabasePath(), getDatabaseFileName(), getJsonFileEnding(), m_logger);
    fileAccessor.setProgress(progress);
    LockedFileAccessor::Error fileError = fileAccessor.lock(LockedFileAccessor::AccessMode::read, s_fileLockTimeoutMs);

    if (fileError != LockedFileAccessor::Error::none)
    {
        if (m_logger)m_logger->logError("bool JDManager::loadObject_internal(JDObject) : Error: " + LockedFileAccessor::getErrorStr(fileError));
		return false;
    }
    double progressScalar = 0;
    if (progress)
    {
        progressScalar = progress->getScalar();
    }
    bool success = true;
    const JDObjectIDptr &id = obj->getObjectID();

    JsonArray jsons;

    if (progress)
    {
        progress->setComment("Reading database file");
        progress->startNewSubProgress(progressScalar * 0.5);
    }
    fileError = fileAccessor.readJsonFile(jsons);
    fileAccessor.unlock();
    if (fileError != LockedFileAccessor::Error::none)
    {
        if (m_logger)m_logger->logError("bool JDManager::loadObject_internal(JDObject): Error: " + LockedFileAccessor::getErrorStr(fileError));
		return false;
	}


    size_t index = JDObjectInterface::getJsonIndexByID(jsons, id->get());
    if (index == std::string::npos)
    {
        if (m_logger)m_logger->logError("bool JDManager::loadObject_internal(JDObject) Object with ID: \"" + id->toString() + "\" not found");
        return false;
    }

    const JsonObject &objData = jsons[index].get<JsonObject>();
    if (progress) progress->setComment("Deserializing object");
    success &= JDManagerObjectManager::loadObjectFromJson_internal(objData, obj);
    if (progress)
    {
        progress->startNewSubProgress(progressScalar * 0.5);
        progress->addProgress(1);
    }
    if (m_logger)
        if (success)
            m_logger->log("Object (id=" + id.get()->toString() + ") loaded successfully", Log::Level::info, Log::Colors::green);
        else
            m_logger->logError("Object (id=" + id.get()->toString() + ") can't be loaded");
    return success;
}




bool JDManager::loadObjects_internal(int mode, Internal::WorkProgress* progress)
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
    if(m_logger)
        m_logger->log("Loading objects with mode: " + getLoadModeStr(mode), Log::Level::info);
    double progressScalar = 0;
    if (progress)
    {
        progressScalar = progress->getScalar();
    }
    bool success = true;
    LockedFileAccessor fileAccessor(getDatabasePath(), getDatabaseFileName(), getJsonFileEnding(), m_logger);
    fileAccessor.setProgress(progress);
    LockedFileAccessor::Error fileError = fileAccessor.lock(LockedFileAccessor::AccessMode::read, s_fileLockTimeoutMs);

    if (fileError != LockedFileAccessor::Error::none)
    {
        if (m_logger)m_logger->logError("bool JDManager::loadObjects_internal(mode): Error: " + LockedFileAccessor::getErrorStr(fileError) + "\n");
        return false;
    }

    JsonArray *jsons = new JsonArray();
    AsyncContextDrivenDeleter asyncDeleter(jsons);

    const double loadingBarRatio = 0.5;
    if (progress)
    {
        progress->setComment("Reading database file");
        progress->startNewSubProgress(progressScalar * loadingBarRatio);
    }
    fileError = fileAccessor.readJsonFile(*jsons);
    
    if (fileError != LockedFileAccessor::Error::none)
    {
        if (m_logger)m_logger->logError("bool JDManager::loadObject_internal(JDObject): Error: " + LockedFileAccessor::getErrorStr(fileError) + "\n");
        return false;
    }


    bool modeNewObjects = (mode & (int)LoadMode::newObjects);
    bool modeChangedObjects = (mode & (int)LoadMode::changedObjects);
    bool modeRemovedObjects = (mode & (int)LoadMode::removedObjects);

    bool overrideChanges = (mode & (int)LoadMode::overrideChanges);

    //const bool hasOverrideChangeFromDatabaseSlots = m_signals.objectOverrideChangeFromDatabase.signal.getSlotCount();
    //const bool hasChangeFromDatabaseSlots = m_signals.objectChangedFromDatabase.signal.getSlotCount();
    //const bool hasObjectAddedToDatabaseSlots = m_signals.objectAddedToDatabase.signal.getSlotCount();
    //const bool hasObjectRemovedFromDatabaseSlots = m_signals.objectRemovedFromDatabase.signal.getSlotCount();



    std::vector<JDObject> overridingObjs;
    std::vector<JDObjectID::IDType> newObjIDs;
    std::vector<JDObject> newObjInstances;
    std::vector<JDObject> removedObjs;
    std::vector<JDObjectPair> pairsForSignal;

    if (progress)
    {
        progress->startNewSubProgress(progressScalar * (1- loadingBarRatio));
    }
    success &= JDManagerObjectManager::loadObjectsFromJson_internal(*jsons, mode, progress, 
        overridingObjs,
        newObjIDs,
        newObjInstances, 
        removedObjs,
        pairsForSignal);


    fileAccessor.unlock();

    // Copy the data to the signals
    if (modeChangedObjects)
    {
        if (overrideChanges)
        {
            //if (hasOverrideChangeFromDatabaseSlots && overridingObjs.size())
            //    m_signals.objectOverrideChangeFromDatabase.addObjs(overridingObjs);
            if (overridingObjs.size())
            {
                //std::lock_guard lck(m_signalDataMutex);
                //m_signalData.onObjectOverrideChangeFromDatabase = overridingObjs;
                emit onObjectOverrideChangeFromDatabase(overridingObjs);
            }
        }
		else
		{
            if (pairsForSignal.size())
            {
                //std::lock_guard lck(m_signalDataMutex);
                //m_signalData.onObjectChangedFromDatabase = pairsForSignal;
                //if (hasChangeFromDatabaseSlots)
                //    m_signals.objectChangedFromDatabase.addPairs(pairsForSignal);
                emit onObjectChangedFromDatabase(pairsForSignal);
            }
		}
    }
    if (modeRemovedObjects)
    {
        if (removedObjs.size())
        {
            //std::lock_guard lck(m_signalDataMutex);
            //m_signalData.onObjectRemovedFromDatabase = removedObjs;
            //if (hasObjectRemovedFromDatabaseSlots)
            //    m_signals.objectRemovedFromDatabase.addObjs(removedObjs);
            emit onObjectRemovedFromDatabase(removedObjs);
        }
    }
    if (modeNewObjects)
    {
        if (newObjInstances.size())
        {
            //std::lock_guard lck(m_signalDataMutex);
            //m_signalData.onObjectAddedToDatabase = newObjInstances;
            /*if (hasObjectAddedToDatabaseSlots)
            {
                m_signals.objectAddedToDatabase.reserve(m_signals.objectAddedToDatabase.size() + newObjInstances.size());
                m_signals.objectAddedToDatabase.addObjs(newObjInstances);
            }*/
            emit onObjectAddedToDatabase(newObjInstances);
        }
    }
    if (m_logger)
        if (success)
        {
            //m_logger->log("Loaded "+std::to_string(newObjInstances.size()) + " new objects successfully", Log::Level::info, Log::Color::green);
            //m_logger->log("Loaded "+std::to_string(overridingObjs.size()) + " changed objects successfully", Log::Level::info, Log::Color::green);
            //m_logger->log("Removed "+std::to_string(removedObjs.size()) + " deleted objects successfully", Log::Level::info, Log::Color::green);
            m_logger->log("Loading done", Log::Level::info, Log::Colors::green);
        }
        else
        {
            m_logger->logError("Objects can't be loaded");
        }
    return success;
}



bool JDManager::saveObject_internal(const JDObject &obj, unsigned int timeoutMillis, Internal::WorkProgress* progress)
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
    if (!obj)
        return false;
    JDObjectLocker::Error lockerError;
    if (!JDManagerObjectManager::isObjectLockedByMe(obj, lockerError))
    {
        if (m_logger)
            m_logger->logWarning("Can't save object with ID: " + obj->getObjectID()->toString() + 
                " because it's not locked by this manager instance. Lock Error: "+ JDObjectLocker::getErrorStr(lockerError));
        return false;
    }
    if(m_logger)
        m_logger->log("Saving object with ID: " + obj->getObjectID()->toString(), Log::Level::info);
    double progressScalar = 0;
    if (progress)
    {
        progressScalar = progress->getScalar();
        progress->startNewSubProgress(progressScalar * 0.33);
    }
    bool success = true;

    LockedFileAccessor fileAccessor(getDatabasePath(), getDatabaseFileName(), getJsonFileEnding(), m_logger);
    fileAccessor.setProgress(progress);
    LockedFileAccessor::Error fileError = fileAccessor.lock(LockedFileAccessor::AccessMode::readWrite, timeoutMillis);

    if (fileError != LockedFileAccessor::Error::none)
    {
        if(fileError == LockedFileAccessor::Error::fileLock_alreadyLockedForWritingByOther)
            emit onDatabaseOutdated();
            //m_signals.addToQueue(Internal::JDManagerSignals::Signals::signal_databaseOutdated, true);

        if (m_logger)m_logger->logError("bool JDManager::saveObject_internal(const JDObject &obj, unsigned int timeoutMillis): Error: " + LockedFileAccessor::getErrorStr(fileError));
        return false;
    }

    FileWatcherAutoPause paused(JDManagerFileSystem::getDatabaseFileWatcher());

    if (progress) progress->setComment("Serializing object");
    JDObjectIDptr ID = obj->getObjectID();
    
    JsonArray jsons;
    std::shared_ptr<JsonObject> data = std::make_shared<JsonObject>();
    success &= obj->saveInternal(*data);


    if (progress)
    {
        progress->setProgress(1);
        progress->setComment("Reading database file");
        progress->startNewSubProgress(progressScalar * 0.33);
    }

    fileError = fileAccessor.readJsonFile(jsons);

    if (fileError != LockedFileAccessor::Error::none)
    {
        if (m_logger)m_logger->logError("bool JDManager::saveObject_internal(const JDObject &obj, unsigned int timeoutMillis,): Error: " + LockedFileAccessor::getErrorStr(fileError));
        return false;
    }
    size_t index = JDObjectInterface::getJsonIndexByID(jsons, ID->get());

    if (index == std::string::npos)
    {
        jsons.push_back(std::move(data));
    }
    else
    {
        jsons[index] = std::move(data);
    }

    if (progress)
    { 
        progress->startNewSubProgress(progressScalar * 0.33);
    }

    fileError = fileAccessor.writeJsonFile(jsons);

    if (fileError != LockedFileAccessor::Error::none)
    {
        if (m_logger)m_logger->logError("bool JDManager::saveObject_internal(JDObject, unsigned int timeoutMs): Error: " + LockedFileAccessor::getErrorStr(fileError));
        return false;
    }
    if(m_logger)
        if(success)
            m_logger->log("Object (id="+ ID.get()->toString() + ") saved successfully", Log::Level::info, Log::Colors::green);
		else
            m_logger->logError("Object (id=" + ID.get()->toString() + ") can't be saved");
    return success;
}
bool JDManager::saveObjects_internal(unsigned int timeoutMillis, Internal::WorkProgress* progress)
{
    std::vector<JDObject> objs = JDManagerObjectManager::getObjects();
    return saveObjects_internal(objs, timeoutMillis, progress);
}
bool JDManager::saveObjects_internal(std::vector<JDObject> objList, unsigned int timeoutMillis, Internal::WorkProgress* progress)
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
    if(objList.size() == 0)
		return true;
    if(m_logger)
		m_logger->log("Saving " + std::to_string(objList.size()) + " objects", Log::Level::info);
    double progressScalar = 0;
    if(progress)
        progressScalar = progress->getScalar();

    std::vector<JDObjectLocker::LockData> lockedObjects;
    JDObjectLocker::Error lockerError;
    if (!JDManagerObjectManager::getLockedObjects(lockedObjects, lockerError))
    {
        if (m_logger)
			m_logger->logError("bool JDManager::saveObjects_internal(const std::vector<JDObject>& objList, unsigned int timeoutMillis): Error: " + JDObjectLocker::getErrorStr(lockerError));
		return false;
    }

    bool notAllSaved = false;
    int removedFromListCount = 0;
    for (size_t i = 0; i < objList.size(); ++i)
    {
		bool found = false;
        for (size_t j = 0; j < lockedObjects.size(); ++j)
        {
            if (objList[i]->getShallowObjectID() == lockedObjects[j].objectID)
            {
				found = true;
                if (lockedObjects[j].user.getSessionID() != m_user.getSessionID())
                {
                    if(m_logger)
						m_logger->logWarning("Object with ID: " + std::to_string(objList[i]->getShallowObjectID()) + " is locked by another user. It will not be saved");
                    objList.erase(objList.begin() + i);
                    notAllSaved = true;
                    ++removedFromListCount;
                    --i;
                }
				break;
			}
		}
        if (!found)
        {
			if (m_logger)
				m_logger->logWarning("Object with ID: " + std::to_string(objList[i]->getShallowObjectID()) + " is not locked by this user. It will not be saved");
			objList.erase(objList.begin() + i);
            ++removedFromListCount;
            notAllSaved = true;
			--i;
		}
	}
   


    LockedFileAccessor fileAccessor(getDatabasePath(), getDatabaseFileName(), getJsonFileEnding(), m_logger);
    fileAccessor.setProgress(progress);
    LockedFileAccessor::Error fileError = fileAccessor.lock(LockedFileAccessor::AccessMode::readWrite, timeoutMillis);

    if (fileError != LockedFileAccessor::Error::none)
    {
        if (fileError == LockedFileAccessor::Error::fileLock_alreadyLockedForWritingByOther)
            emit onDatabaseOutdated();
            //m_signals.addToQueue(Internal::JDManagerSignals::Signals::signal_databaseOutdated, true);

        if (m_logger)m_logger->logError("bool JDManager::saveObjects_internal(const std::vector<JDObject>& objList, unsigned int timeoutMillis): Error: " + LockedFileAccessor::getErrorStr(fileError));
        return false;
    }
    FileWatcherAutoPause paused(JDManagerFileSystem::getDatabaseFileWatcher());

    if (progress) progress->setComment("Serializing objects");
    JsonArray *jsonData = new JsonArray;
    JsonArray origJsonData;
    AsyncContextDrivenDeleter asyncDeleter(jsonData);
   
    fileError = fileAccessor.readJsonFile(origJsonData);

    if (fileError != LockedFileAccessor::Error::none)
    {
        if (m_logger)m_logger->logError("bool JDManager::saveObjects_internal(const JDObject &obj, unsigned int timeoutMillis,): Error: " + LockedFileAccessor::getErrorStr(fileError));
        return false;
    }
    
    bool success = true;
    if (progress)
    {
        progress->startNewSubProgress(progressScalar * 0.6);
        success &= Internal::JDObjectManager::getJsonArray(objList, *jsonData, progress);
    }
	else
	{
		success &= Internal::JDObjectManager::getJsonArray(objList, *jsonData);
	}

    // Replace the objects in the original json data
    int replacedCount = 0;
    if (m_removedObjectIDs.size() > 0)
    {
        for (size_t i = 0; i < origJsonData.size(); ++i)
        {
            JDObjectID::IDType id = JDObjectInterface::getIDFromJson(origJsonData[i].get<JsonObject>());
            if (std::find(m_removedObjectIDs.begin(), m_removedObjectIDs.end(), id) != m_removedObjectIDs.end())
            {
				origJsonData.erase(origJsonData.begin() + i);
				--i;
			}
		}
    }
    m_removedObjectIDs.clear();

    for (size_t i = 0; i < origJsonData.size(); ++i)
    {
        const JsonObject &objData = origJsonData[i].get<JsonObject>();
        size_t index = JDObjectInterface::getJsonIndexByID(*jsonData, JDObjectInterface::getIDFromJson(objData));
        if (index == std::string::npos)
			continue;
        origJsonData[i] = (*jsonData)[index];
        jsonData->erase(jsonData->begin() + index);
        ++replacedCount;
    }
    if (jsonData->size())
    {
        for (size_t i = 0; i < jsonData->size(); ++i)
        {
			origJsonData.push_back((*jsonData)[i]);
		}
	}
    
    // Save the serialized objects
    if(progress) progress->startNewSubProgress(progressScalar * 0.4);
    fileError = fileAccessor.writeJsonFile(origJsonData);
    if (fileError != LockedFileAccessor::Error::none)
    {
        if (m_logger)m_logger->logError("bool JDManager::saveObject_internal(const std::vector<JDObject>& objList, unsigned int timeoutMillis): Error: " + LockedFileAccessor::getErrorStr(fileError));
        return false;
    }
    if (m_logger)
        if (success)
        {
            if(objList.size() > 0)
                m_logger->log(std::to_string(objList.size()) + " objects saved successfully", Log::Level::info, Log::Colors::green);
            if(removedFromListCount > 0)
                m_logger->logWarning(std::to_string(removedFromListCount) + " objects can't be saved because they are not locked by this user.");
        }
        else
            m_logger->logError(std::to_string(objList.size() + removedFromListCount) + " objects can't be saved");
    return success && !notAllSaved;
}

void JDManager::onAsyncWorkDone(std::shared_ptr<Internal::JDManagerAysncWork> work)
{
    if(!work)
        return;
    JDManagerAsyncWorker::removeDoneWork(work);

    std::shared_ptr<Internal::JDManagerAysncWorkLoadAllObjects> loadAllWork = std::dynamic_pointer_cast<Internal::JDManagerAysncWorkLoadAllObjects>(work);
    std::shared_ptr<Internal::JDManagerAysncWorkLoadSingleObject> loadSingle = std::dynamic_pointer_cast<Internal::JDManagerAysncWorkLoadSingleObject>(work);
    std::shared_ptr<Internal::JDManagerAysncWorkSaveSingle> saveSingle = std::dynamic_pointer_cast<Internal::JDManagerAysncWorkSaveSingle>(work);
    std::shared_ptr<Internal::JDManagerAysncWorkSaveList> saveList = std::dynamic_pointer_cast<Internal::JDManagerAysncWorkSaveList>(work);

    if (loadAllWork)
    {
        emit onLoadObjectsDone(loadAllWork->hasSucceeded());
        //m_signalData.onLoadObjectsDone.success = loadAllWork->hasSucceeded();
        //m_signalData.onLoadObjectsDone.signalActive = true;
        //m_signals.addToQueue(Internal::JDManagerSignals::Signals::signal_onLoadObjectsDone, loadAllWork->hasSucceeded(), true);
    }
    else if (loadSingle)
    {
        emit onLoadObjectDone(loadSingle->hasSucceeded(), loadSingle->getObject());
		//m_signals.addToQueue(Internal::JDManagerSignals::Signals::signal_onLoadObjectDone, loadSingle->hasSucceeded(), loadSingle->getObject(), true);
	}
    else if (saveSingle)
    {
        emit onSaveObjectDone(saveSingle->hasSucceeded(), saveSingle->getObject());
		//m_signals.addToQueue(Internal::JDManagerSignals::Signals::signal_onSaveObjectDone, saveSingle->hasSucceeded(), loadSingle->getObject(), true);
	}
    else if (saveList)
    {
        emit onSaveObjectsDone(saveList->hasSucceeded());
		//m_signals.addToQueue(Internal::JDManagerSignals::Signals::signal_onSaveObjectsDone, saveList->hasSucceeded(), true);
	}
    emit onLockedObjectsChanged();
    //m_signals.lockedObjectsChanged.emitSignal();
    if (!work->hasSucceeded())
        onAsyncWorkError(work);
}
void JDManager::onAsyncWorkError(std::shared_ptr<Internal::JDManagerAysncWork> work)
{
    if (m_logger)m_logger->logError("Async work failed: " + work->getErrorMessage());
}

void JDManager::onObjectLockerFileChanged()
{

}



const Utilities::JDUser& JDManager::getUser() const
{
    return m_user;
}
const std::string& JDManager::getSessionID() const
{
    return m_user.getSessionID();
}


const std::string& JDManager::getLoadModeStr(int mode) const
{
    static std::string str;
    str.clear();
    str.reserve(100);
    if (mode == 0)
    {
        str = "none";
        return str;
    }

    if(mode & (int)LoadMode::allObjects)
		str += "allObjects";
    else
    {
        if (mode & (int)LoadMode::newObjects)
        {
            str += "newObjects";
        }
        if (mode & (int)LoadMode::changedObjects)
        {
            if (str.size())
                str += " + ";
            str += "changedObjects";
        }
        if (mode & (int)LoadMode::removedObjects)
        {
            if (str.size())
                str += " + ";
            str += "removedObjects";
        }
    }
    if (mode & (int)LoadMode::overrideChanges)
    {
        if (str.size())
            str += " + ";
        str += "overrideChanges";
    }
	return str;

}


void JDManager::update()
{
    if(m_signleEntryUpdateLock)
        return; // Update is already running
    if (!m_updateMutex.try_lock())
        return;
    m_signleEntryUpdateLock = true;

    JDManagerAsyncWorker::process();
    
    JDManagerFileSystem::update();
    JDManagerObjectManager::update();
    
    //m_signals.emitIfNotEmpty();
    //m_signals.emitQueue();
    m_signleEntryUpdateLock = false;
    m_updateMutex.unlock();

    /*SignalData data;
    {
        std::lock_guard lck(m_signalDataMutex);
        data = m_signalData;
        m_signalData = SignalData();
    }
    if(data.onObjectRemovedFromDatabase.size() > 0)
        emit onObjectRemovedFromDatabase(data.onObjectRemovedFromDatabase);
    if(data.onObjectAddedToDatabase.size() > 0)
        emit onObjectAddedToDatabase(data.onObjectAddedToDatabase);
    if(data.onObjectChangedFromDatabase.size() > 0)
        emit onObjectChangedFromDatabase(data.onObjectChangedFromDatabase);
    if(data.onObjectOverrideChangeFromDatabase.size() > 0)
        emit onObjectOverrideChangeFromDatabase(data.onObjectOverrideChangeFromDatabase);
    if(data.onLoadObjectDone.obj)
        emit onLoadObjectDone(data.onLoadObjectDone.success, data.onLoadObjectDone.obj);
   // if(data.onLoadObjectsDone)
	//	emit onLoadObjectsDone(data.onLoadObjectsDone);
    if(data.onSaveObjectDone.obj)
		emit onSaveObjectDone(data.onSaveObjectDone.success, data.onSaveObjectDone.obj);*/
    
}
/*Internal::JDManagerSignals& JDManager::getSignals()
{
    return m_signals;
}*/

}
