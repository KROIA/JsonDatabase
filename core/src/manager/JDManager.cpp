#include "manager/JDManager.h"
#include "manager/JDManagerSignals.h"
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
    
    //const QString JDManager::s_timeFormat = "hh:mm:ss.zzz";
    //const QString JDManager::s_dateFormat = "dd.MM.yyyy";
    //
    //const QString JDManager::s_tag_sessionID = "sessionID";
    //const QString JDManager::s_tag_user = "user";
    //const QString JDManager::s_tag_date = "date";
    //const QString JDManager::s_tag_time = "time";

    const unsigned int JDManager::s_fileLockTimeoutMs = 1000;


    JDManager::JDManager(const std::string& databasePath,
                         const std::string& databaseName)
        : JDManagerObjectManager(*this, m_mutex)
        , JDManagerFileSystem(databasePath, databaseName, *this, m_mutex)
        //, JDObjectLocker(*this, m_mutex)
        , JDManagerAsyncWorker(*this, m_mutex)
        , m_useZipFormat(false)
        , m_signleEntryUpdateLock(false)
        , m_signals(*this, m_mutex)
    {
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
        , m_signals(*this, m_mutex)
    {
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
        , m_signals(*this, m_mutex)
    {
        m_user = Utilities::JDUser::generateUser(m_user.getName());
        JDManagerObjectManager::setDomainName(m_user.getSessionID());
    }
JDManager::~JDManager()
{
    JDObjectLocker::Error lockerError;
    JDManagerAsyncWorker::stop();
    JDManagerFileSystem::getDatabaseFileWatcher().stop();
}

bool JDManager::setup()
{
    bool success = true;
    success &= JDManagerFileSystem::setup();
    success &= JDManagerObjectManager::setup();
    JDObjectLocker::Error lockerError;
    JDManagerAsyncWorker::setup();
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
    JDManagerAsyncWorker::addWork(std::make_shared < Internal::JDManagerAysncWorkSaveList>(*this, m_mutex, objs));
}

bool JDManager::loadObject_internal(const JDObject& obj, Internal::WorkProgress* progress)
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
    if (!JDManagerObjectManager::exists_internal(obj))
        return false;

    LockedFileAccessor fileAccessor(getDatabasePath(), getDatabaseFileName(), getJsonFileEnding());
    fileAccessor.setProgress(progress);
    LockedFileAccessor::Error fileError = fileAccessor.lock(LockedFileAccessor::AccessMode::read, s_fileLockTimeoutMs);

    if (fileError != LockedFileAccessor::Error::none)
    {
		JD_CONSOLE("bool JDManager::loadObject_internal(JDObject): Error: " + LockedFileAccessor::getErrorStr(fileError) + "\n");
		return false;
    }

   /* bool wasLockedForWritingByOther = false;
    JDManagerFileSystem::Error error;
    if (!JDManagerFileSystem::lockFile(getDatabasePath(), getDatabaseName(), FileReadWriteLock::Access::read, 
        wasLockedForWritingByOther, s_fileLockTimeoutMs, error))
    {
        return false;
    }*/
    double progressScalar = 0;
    if (progress)
    {
        progressScalar = progress->getScalar();
    }
    bool success = true;
    const JDObjectIDptr &id = obj->getObjectID();

    
#if JD_ACTIVE_JSON == JD_JSON_QT
    std::vector<QJsonObject> jsons; 
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
    JsonArray jsons;
#endif

    if (progress)
    {
        progress->setComment("Reading database file");
        progress->startNewSubProgress(progressScalar * 0.5);
    }
    fileError = fileAccessor.readJsonFile(jsons);
    fileAccessor.unlock();
    if (fileError != LockedFileAccessor::Error::none)
    {
		JD_CONSOLE("bool JDManager::loadObject_internal(JDObject): Error: " + LockedFileAccessor::getErrorStr(fileError) + "\n");
		return false;
	}
    
    /*success &= JDManagerFileSystem::readJsonFile(jsons,
        getDatabasePath(), 
        getDatabaseName(), 
        Internal::JDManagerFileSystem::getJsonFileEnding(), 
        m_useZipFormat, 
        false,
        error,
        progress);*/

    size_t index = JDObjectInterface::getJsonIndexByID(jsons, id);
    if (index == std::string::npos)
    {
        JD_CONSOLE("bool JDManager::loadObject_internal(JDObject) Object with ID: \"" << id << "\" not found");
        return false;
    }
#if JD_ACTIVE_JSON == JD_JSON_QT
    const QJsonObject& objData = jsons[index];
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
    const JsonObject &objData = jsons[index].get<JsonObject>();
#endif
    if (progress) progress->setComment("Deserializing object");
    success &= JDManagerObjectManager::loadObjectFromJson_internal(objData, obj);
    //success &= Internal::JsonUtilities::deserializeOverrideFromJson(objData, obj, hasChanged);
    //JDManagerFileSystem::unlockFile(error);
    if (progress)
    {
        progress->startNewSubProgress(progressScalar * 0.5);
        progress->addProgress(1);
    }

    return success;
}




bool JDManager::loadObjects_internal(int mode, Internal::WorkProgress* progress)
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
    double progressScalar = 0;
    if (progress)
    {
        progressScalar = progress->getScalar();
        
    }
    bool success = true;
    /*bool wasLockedForWritingByOther = false;
    JDManagerFileSystem::Error error;
    if (!JDManagerFileSystem::lockFile(getDatabasePath(), getDatabaseName(), FileReadWriteLock::Access::read, 
        wasLockedForWritingByOther, s_fileLockTimeoutMs, error))
    {
        JD_CONSOLE("bool JDManager::loadObjects_internal(mode=\""<< getLoadModeStr(mode) <<"\") Can't lock database\n");
        return false;
    }*/

    LockedFileAccessor fileAccessor(getDatabasePath(), getDatabaseFileName(), getJsonFileEnding());
    fileAccessor.setProgress(progress);
    LockedFileAccessor::Error fileError = fileAccessor.lock(LockedFileAccessor::AccessMode::read, s_fileLockTimeoutMs);

    if (fileError != LockedFileAccessor::Error::none)
    {
        JD_CONSOLE("bool JDManager::loadObjects_internal(mode): Error: " + LockedFileAccessor::getErrorStr(fileError) + "\n");
        return false;
    }
    


    
#if JD_ACTIVE_JSON == JD_JSON_QT
    std::vector<QJsonObject> *jsons = new std::vector<QJsonObject>();
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
    JsonArray *jsons = new JsonArray();
#endif
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
        JD_CONSOLE("bool JDManager::loadObject_internal(JDObject): Error: " + LockedFileAccessor::getErrorStr(fileError) + "\n");
        return false;
    }
    /*success &= JDManagerFileSystem::readJsonFile(*jsons,
        getDatabasePath(), 
        getDatabaseName(), 
        Internal::JDManagerFileSystem::getJsonFileEnding(), 
        m_useZipFormat, 
        false,
        error,
        progress);*/

    bool modeNewObjects = (mode & (int)LoadMode::newObjects);
    bool modeChangedObjects = (mode & (int)LoadMode::changedObjects);
    bool modeRemovedObjects = (mode & (int)LoadMode::removedObjects);

    bool overrideChanges = (mode & (int)LoadMode::overrideChanges);

    const bool hasOverrideChangeFromDatabaseSlots = m_signals.objectOverrideChangeFromDatabase.signal.getSlotCount();
    const bool hasChangeFromDatabaseSlots = m_signals.objectChangedFromDatabase.signal.getSlotCount();
    const bool hasObjectAddedToDatabaseSlots = m_signals.objectAddedToDatabase.signal.getSlotCount();
    const bool hasObjectRemovedFromDatabaseSlots = m_signals.objectRemovedFromDatabase.signal.getSlotCount();



    std::vector<JDObject> overridingObjs;
    std::vector<JDObjectID::IDType> newObjIDs;
    std::vector<JDObject> newObjInstances;
    std::vector<JDObject> removedObjs;
    std::vector<JDObjectPair> pairsForSignal;

    if (progress)
    {
        //progress->setComment("Deserialize objects");
        progress->startNewSubProgress(progressScalar * (1- loadingBarRatio));
    }
    success &= JDManagerObjectManager::loadObjectsFromJson_internal(*jsons, mode, progress, 
       /* hasOverrideChangeFromDatabaseSlots,
        hasChangeFromDatabaseSlots,
        hasObjectAddedToDatabaseSlots,
        hasObjectRemovedFromDatabaseSlots,*/
        overridingObjs,
        newObjIDs,
        newObjInstances, 
        removedObjs,
        pairsForSignal);


    //JDManagerFileSystem::unlockFile(error);
    fileAccessor.unlock();

    // Copy the data to the signals
    if (modeChangedObjects)
    {
        if (overrideChanges)
        {
            if (hasOverrideChangeFromDatabaseSlots && overridingObjs.size())
                m_signals.objectOverrideChangeFromDatabase.addObjs(overridingObjs);
        }
		else
		{
            if (pairsForSignal.size())
            {
                if (hasChangeFromDatabaseSlots)
                    m_signals.objectChangedFromDatabase.addPairs(pairsForSignal);
            }
		}
    }
    if (modeRemovedObjects)
    {
        if (removedObjs.size())
        {
            if (hasObjectRemovedFromDatabaseSlots)
                m_signals.objectRemovedFromDatabase.addObjs(removedObjs);
        }
    }
    if (modeNewObjects)
    {
        if (newObjInstances.size())
        {
            if (hasObjectAddedToDatabaseSlots)
            {
                m_signals.objectAddedToDatabase.reserve(m_signals.objectAddedToDatabase.size() + newObjInstances.size());
                m_signals.objectAddedToDatabase.addObjs(newObjInstances);
            }
        }
    }
    return success;
}



bool JDManager::saveObject_internal(const JDObject &obj, unsigned int timeoutMillis, Internal::WorkProgress* progress)
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
    if (!obj)
        return false;
    double progressScalar = 0;
    if (progress)
    {
        progressScalar = progress->getScalar();
        progress->startNewSubProgress(progressScalar * 0.33);
    }
    bool success = true;

    LockedFileAccessor fileAccessor(getDatabasePath(), getDatabaseFileName(), getJsonFileEnding());
    fileAccessor.setProgress(progress);
    LockedFileAccessor::Error fileError = fileAccessor.lock(LockedFileAccessor::AccessMode::readWrite, timeoutMillis);

    if (fileError != LockedFileAccessor::Error::none)
    {
        if(fileError == LockedFileAccessor::Error::fileLock_alreadyLockedForWritingByOther)
            m_signals.addToQueue(Internal::JDManagerSignals::Signals::signal_databaseOutdated, true);

        JD_CONSOLE("bool JDManager::saveObject_internal(const JDObject &obj, unsigned int timeoutMillis): Error: " + LockedFileAccessor::getErrorStr(fileError) + "\n");
        return false;
    }

    FileWatcherAutoPause paused(JDManagerFileSystem::getDatabaseFileWatcher());
    /*bool wasLockedForWritingByOther = false;
    JDManagerFileSystem::Error error;
    bool hasLock = JDManagerFileSystem::lockFile(getDatabasePath(), getDatabaseName(), 
        FileReadWriteLock::Access::readWrite, wasLockedForWritingByOther, 
        timeoutMillis, error);
    if (wasLockedForWritingByOther)
    {
        m_signals.addToQueue(Internal::JDManagerSignals::Signals::signal_databaseOutdated, true);
    	return false;
    }
    if (!hasLock)
    {
        return false;
    }*/

    if (progress) progress->setComment("Serializing object");
    JDObjectIDptr ID = obj->getObjectID();
    
#if JD_ACTIVE_JSON == JD_JSON_QT
    std::vector<QJsonObject> jsons;
    QJsonObject data;
    success &= obj->saveInternal(data);
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
    JsonArray jsons;
    std::shared_ptr<JsonObject> data = std::make_shared<JsonObject>();
    success &= obj->saveInternal(*data);
#endif
    

    if (progress)
    {
        progress->setProgress(1);
        progress->setComment("Reading database file");
        progress->startNewSubProgress(progressScalar * 0.33);
    }

    fileError = fileAccessor.readJsonFile(jsons);

    if (fileError != LockedFileAccessor::Error::none)
    {
        JD_CONSOLE("bool JDManager::saveObject_internal(const JDObject &obj, unsigned int timeoutMillis,): Error: " + LockedFileAccessor::getErrorStr(fileError) + "\n");
        return false;
    }

    /*
    success &= JDManagerFileSystem::readJsonFile(jsons,
        getDatabasePath(),
        getDatabaseName(), 
        Internal::JDManagerFileSystem::getJsonFileEnding(), 
        m_useZipFormat, 
        false,
        error,
        progress);
        */
    size_t index = JDObjectInterface::getJsonIndexByID(jsons, ID);

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
        JD_CONSOLE("bool JDManager::saveObject_internal(JDObject, unsigned int timeoutMs): Error: " + LockedFileAccessor::getErrorStr(fileError) + "\n");
        return false;
    }
    /*
    // Save the serialized objects
    success &= JDManagerFileSystem::writeJsonFile(jsons, 
        getDatabasePath(), 
        getDatabaseName(), 
        Internal::JDManagerFileSystem::getJsonFileEnding(), 
        m_useZipFormat, 
        false,
        error,
        progress);
        */
    
    //JDManagerFileSystem::unlockFile(error);

    return success;
}
bool JDManager::saveObjects_internal(unsigned int timeoutMillis, Internal::WorkProgress* progress)
{
    std::vector<JDObject> objs = JDManagerObjectManager::getObjects();
    return saveObjects_internal(objs, timeoutMillis, progress);
}
bool JDManager::saveObjects_internal(const std::vector<JDObject>& objList, unsigned int timeoutMillis, Internal::WorkProgress* progress)
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
    if(objList.size() == 0)
		return true;
    double progressScalar = 0;
    if(progress)
        progressScalar = progress->getScalar();

    LockedFileAccessor fileAccessor(getDatabasePath(), getDatabaseFileName(), getJsonFileEnding());
    fileAccessor.setProgress(progress);
    LockedFileAccessor::Error fileError = fileAccessor.lock(LockedFileAccessor::AccessMode::readWrite, timeoutMillis);

    if (fileError != LockedFileAccessor::Error::none)
    {
        if (fileError == LockedFileAccessor::Error::fileLock_alreadyLockedForWritingByOther)
            m_signals.addToQueue(Internal::JDManagerSignals::Signals::signal_databaseOutdated, true);

        JD_CONSOLE("bool JDManager::saveObjects_internal(const std::vector<JDObject>& objList, unsigned int timeoutMillis): Error: " + LockedFileAccessor::getErrorStr(fileError) + "\n");
        return false;
    }
    FileWatcherAutoPause paused(JDManagerFileSystem::getDatabaseFileWatcher());

   /* bool wasLockedForWritingByOther = false;
    JDManagerFileSystem::Error error;
    bool hasLock = JDManagerFileSystem::lockFile(getDatabasePath(), getDatabaseName(), 
        FileReadWriteLock::Access::write, wasLockedForWritingByOther, 
        timeoutMillis, error);
    if (wasLockedForWritingByOther)
    {
        m_signals.addToQueue(Internal::JDManagerSignals::Signals::signal_databaseOutdated, true);
        JDManagerFileSystem::unlockFile(error);
        return false;
    }
    if (!hasLock)
    {
        JD_CONSOLE("bool JDManager::saveObjects_internal(vector<JDObject>&, timeout=" << timeoutMillis << "ms) Can't lock database\n");
        return false;
    }*/
    bool success = true;

    if (progress) progress->setComment("Serializing objects");
#if JD_ACTIVE_JSON == JD_JSON_QT
    std::vector<QJsonObject>* jsonData = new std::vector<QJsonObject>();
#elif JD_ACTIVE_JSON == JD_JSON_GLAZE || JD_ACTIVE_JSON == JD_JSON_INTERNAL
    JsonArray *jsonData = new JsonArray;
#endif
    AsyncContextDrivenDeleter asyncDeleter(jsonData);
   
    
    if (progress)
    {
        progress->startNewSubProgress(progressScalar * 0.6);
        success &= Internal::JDObjectManager::getJsonArray(objList, *jsonData, progress);
        //progress->setComment("Writing database file");
    }
	else
	{
		success &= Internal::JDObjectManager::getJsonArray(objList, *jsonData);
	}
    

    // Save the serialized objects
    if(progress) progress->startNewSubProgress(progressScalar * 0.4);
    fileError = fileAccessor.writeJsonFile(*jsonData);
    if (fileError != LockedFileAccessor::Error::none)
    {
        JD_CONSOLE("bool JDManager::saveObject_internal(const std::vector<JDObject>& objList, unsigned int timeoutMillis): Error: " + LockedFileAccessor::getErrorStr(fileError) + "\n");
        return false;
    }
    /*
    success &= JDManagerFileSystem::writeJsonFile(*jsonData, 
        getDatabasePath(), 
        getDatabaseName(), Internal::JDManagerFileSystem::getJsonFileEnding(), 
        m_useZipFormat, 
        false,
        error,
        progress);
    success &= JDManagerFileSystem::unlockFile(error);*/

    return success;
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
        m_signals.addToQueue(Internal::JDManagerSignals::Signals::signal_onLoadObjectsDone, loadAllWork->hasSucceeded(), true);
    }
    else if (loadSingle)
    {
		m_signals.addToQueue(Internal::JDManagerSignals::Signals::signal_onLoadObjectDone, loadSingle->hasSucceeded(), loadSingle->getObject(), true);
	}
    else if (saveSingle)
    {
		m_signals.addToQueue(Internal::JDManagerSignals::Signals::signal_onSaveObjectDone, saveSingle->hasSucceeded(), loadSingle->getObject(), true);
	}
    else if (saveList)
    {
		m_signals.addToQueue(Internal::JDManagerSignals::Signals::signal_onSaveObjectsDone, saveList->hasSucceeded(), true);
	}
    m_signals.lockedObjectsChanged.emitSignal();
    if (!work->hasSucceeded())
        onAsyncWorkError(work);
}
void JDManager::onAsyncWorkError(std::shared_ptr<Internal::JDManagerAysncWork> work)
{
    JD_CONSOLE("Async work failed: " + work->getErrorMessage() + "\n");
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
    //JDObjectLocker::update();

    
    m_signals.emitIfNotEmpty();
    m_signals.emitQueue();
    m_signleEntryUpdateLock = false;
    m_updateMutex.unlock();
}
Internal::JDManagerSignals& JDManager::getSignals()
{
    return m_signals;
}

}
