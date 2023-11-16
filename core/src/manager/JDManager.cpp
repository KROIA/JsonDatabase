#include "manager/JDManager.h"
#include "manager/JDManagerSignals.h"
#include "object/JDObjectInterface.h"
#include "object/JDObjectRegistry.h"
#include "utilities/JDUniqueMutexLock.h"
#include "utilities/SystemCommand.h"
#include "utilities/JsonUtilities.h"

#include "manager/async/work/JDManagerWorkLoadAllObjects.h"
#include "manager/async/work/JDManagerWorkLoadSingleObject.h"
#include "manager/async/work/JDManagerWorkSaveList.h"
#include "manager/async/work/JDManagerWorkSaveSingle.h"



namespace JsonDatabase
{

    
    const QString JDManager::s_timeFormat = "hh:mm:ss.zzz";
    const QString JDManager::s_dateFormat = "dd.MM.yyyy";

    const QString JDManager::s_tag_sessionID = "sessionID";
    const QString JDManager::s_tag_user = "user";
    const QString JDManager::s_tag_date = "date";
    const QString JDManager::s_tag_time = "time";

    const unsigned int JDManager::s_fileLockTimeoutMs = 1000;


    void JDManager::startProfiler()
    {
#ifdef JD_PROFILING
        EASY_PROFILER_ENABLE;
#endif
    }
    void JDManager::stopProfiler(const std::string profileFilePath)
    {
#ifdef JD_PROFILING
        profiler::dumpBlocksToFile(profileFilePath.c_str());
#endif
    }

    JDManager::JDManager(const std::string& databasePath,
        const std::string& databaseName,
        const std::string& sessionID,
        const std::string& user)
        : JDManagerObjectManager(m_mutex)
        , JDManagerFileSystem(*this, m_mutex)
        , JDObjectLocker(*this, m_mutex)
        , JDManagerAsyncWorker(*this, m_mutex)
        , m_databasePath(databasePath)
        , m_databaseName(databaseName)
        , m_sessionID(sessionID)
        , m_user(user)
        , m_useZipFormat(false)
        , m_signleEntryUpdateLock(false)
        , m_signals(*this, m_mutex)
        //, m_asyncWorker(*this, m_mutex)
    {
        
    }
    JDManager::JDManager(const JDManager &other)
        : JDManagerObjectManager(m_mutex)
        , JDManagerFileSystem(*this, m_mutex)
        , JDObjectLocker(*this, m_mutex)
        , JDManagerAsyncWorker(*this, m_mutex)
        , m_sessionID(other.m_sessionID)
        , m_user(other.m_user)
        , m_useZipFormat(other.m_useZipFormat)
        , m_signleEntryUpdateLock(false)
        , m_signals(*this, m_mutex)
        //, m_asyncWorker(*this, m_mutex)
    {

    }
JDManager::~JDManager()
{
    JDObjectLocker::Error lockerError;
    //JDObjectLocker::stop
    //JDObjectLocker::unlockAllObjs(lockerError);
    //m_asyncWorker.stop();
    JDManagerAsyncWorker::stop();
    JDManagerFileSystem::getDatabaseFileWatcher().stop();
}

bool JDManager::setup()
{
    bool success = true;
    success &= JDManagerObjectManager::setup();
    success &= JDManagerFileSystem::setup();
    JDObjectLocker::Error lockerError;
    success &= JDObjectLocker::setup(lockerError);
    if (lockerError != JDObjectLocker::Error::none)
    {
        // Unhandled error
    }
    //m_asyncWorker.setup();
    JDManagerAsyncWorker::setup();
    return success;
}

void JDManager::setDatabaseName(const std::string& name)
{
    JDM_UNIQUE_LOCK_P;
    m_databaseName = name;
    JDManagerFileSystem::restartDatabaseFileWatcher();
}

void JDManager::setDatabasePath(const std::string &path)
{
    JDM_UNIQUE_LOCK_P;
    if (path == m_databasePath)
        return;
    JDObjectLocker::Error lockerError;
    JDObjectLocker::onDatabasePathChange(m_databasePath, path, lockerError);
    m_databasePath = path;
    JDManagerFileSystem::makeDatabaseDirs();
    JDManagerFileSystem::makeDatabaseFiles();
    JDManagerFileSystem::restartDatabaseFileWatcher();
}
const std::string& JDManager::getDatabaseName() const
{
    return m_databaseName;
}
const std::string &JDManager::getDatabasePath() const
{
    return m_databasePath;
}
std::string JDManager::getDatabaseFilePath() const
{
    return  m_databasePath + "//" + m_databaseName + Internal::JDManagerFileSystem::getJsonFileEnding();
}


void JDManager::enableZipFormat(bool enable)
{
    m_useZipFormat = enable;
}
bool JDManager::isZipFormatEnabled() const
{
    return m_useZipFormat;
}

bool JDManager::loadObject(JDObjectInterface* obj)
{
    JDM_UNIQUE_LOCK_P;
    return loadObject_internal(obj, nullptr);
}
void JDManager::loadObjectAsync(JDObjectInterface* obj)
{
    JDManagerAsyncWorker::addWork(std::make_shared<Internal::JDManagerAysncWorkLoadSingleObject>(*this, m_mutex, obj));
    //m_asyncWorker.addWork(std::make_shared<Internal::JDManagerAysncWorkLoadSingleObject>(*this, m_mutex, obj));
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
bool JDManager::saveObject(JDObjectInterface* obj)
{
    JDM_UNIQUE_LOCK_P;
    return saveObject_internal(obj, s_fileLockTimeoutMs, nullptr);
}
void JDManager::saveObjectAsync(JDObjectInterface* obj)
{
    JDManagerAsyncWorker::addWork(std::make_shared < Internal::JDManagerAysncWorkSaveSingle>(*this, m_mutex, obj));
}
bool JDManager::saveObjects()
{
    //JDM_UNIQUE_LOCK_P;
    std::vector<JDObjectInterface*> objs = JDManagerObjectManager::getObjects();
    return saveObjects_internal(objs, s_fileLockTimeoutMs, nullptr);
}
void JDManager::saveObjectsAsync()
{
    std::vector<JDObjectInterface*> objs = JDManagerObjectManager::getObjects();
    JDManagerAsyncWorker::addWork(std::make_shared < Internal::JDManagerAysncWorkSaveList>(*this, m_mutex, objs));
}
/*bool JDManager::saveObjects(const std::vector<JDObjectInterface*>& objList)
{
    JDM_UNIQUE_LOCK_P;
    return saveObjects_internal(objList);
}*/





bool JDManager::loadObject_internal(JDObjectInterface* obj, Internal::WorkProgress* progress)
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
    if (!JDManagerObjectManager::exists_internal(obj))
        return false;
    bool wasLockedForWritingByOther = false;
    if (!JDManagerFileSystem::lockFile(getDatabasePath(), getDatabaseName(), FileReadWriteLock::Access::read, wasLockedForWritingByOther, s_fileLockTimeoutMs))
    {
        return false;
    }

    bool success = true;
    const std::string& ID = obj->getObjectID();

    if (progress) progress->setComment("Reading database file");
    std::vector<QJsonObject> jsons; 

    success &= JDManagerFileSystem::readJsonFile(jsons, 
        getDatabasePath(), 
        getDatabaseName(), 
        Internal::JDManagerFileSystem::getJsonFileEnding(), 
        m_useZipFormat, 
        false);

    if (progress) progress->setProgress(0.5);
    size_t index = JDObjectInterface::getJsonIndexByID(jsons, ID);
    if (index == std::string::npos)
    {
        JD_CONSOLE("bool JDManager::loadObject_internal(JDObjectInterface*) Object with ID: " << ID << " not found");
        return false;
    }
    const QJsonObject& objData = jsons[index];
    bool hasChanged = false;
    if (progress) progress->setComment("Deserializing object");
    success &= Internal::JsonUtilities::deserializeOverrideFromJson(objData, obj, hasChanged);
    JDManagerFileSystem::unlockFile();
    if (progress) progress->addProgress(0.5);

    return success;
}
bool JDManager::loadObjects_internal(int mode, Internal::WorkProgress* progress)
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
    bool success = true;
    bool wasLockedForWritingByOther = false;
    if (!JDManagerFileSystem::lockFile(getDatabasePath(), getDatabaseName(), FileReadWriteLock::Access::read, wasLockedForWritingByOther, s_fileLockTimeoutMs))
    {
        JD_CONSOLE("bool JDManager::loadObjects_internal(mode=\""<< getLoadModeStr(mode) <<"\") Can't lock database\n");
        return false;
    }

    bool modeNewObjects = (mode & (int)LoadMode::newObjects);
    bool modeChangedObjects = (mode & (int)LoadMode::changedObjects);
    bool modeRemovedObjects = (mode & (int)LoadMode::removedObjects);

    bool overrideChanges = (mode & (int)LoadMode::overrideChanges);


    if (progress) progress->setComment("Reading database file");
    std::vector<QJsonObject> jsons;

    success &= JDManagerFileSystem::readJsonFile(jsons, 
        getDatabasePath(), 
        getDatabaseName(), 
        Internal::JDManagerFileSystem::getJsonFileEnding(), 
        m_useZipFormat, 
        false);

    if(progress) progress->setProgress(0.2);

    struct Pair
    {
        JDObjectInterface* objOriginal;
        JDObjectInterface* obj;
        QJsonObject json;
    };

    std::vector< Pair> pairs;
    std::vector< Pair> newObjectPairs;
    {
        JD_GENERAL_PROFILING_BLOCK("Match objects with json data", JD_COLOR_STAGE_2);
        if (progress) progress->setComment("Matching objects with json data");
        for (size_t i = 0; i < jsons.size(); ++i)
        {
            std::string ID;
            if (!JDSerializable::getJsonValue(jsons[i], ID, JDObjectInterface::s_tag_objID))
            {
                JD_CONSOLE("bool JDManager::loadObjects_internal(mode=\"" << getLoadModeStr(mode) 
                    << "\") Object with no ID found: " << QJsonValue(jsons[i]).toString().toStdString() + "\n");
                success = false;
            }
            Pair p;
            p.objOriginal = getObject_internal(ID);
            p.obj = nullptr;
            p.json = jsons[i];
            if (p.objOriginal)
                pairs.push_back(p);
            else
                newObjectPairs.push_back(p);
        }
        if (progress) progress->addProgress(0.03);
    }
    if (!success)
    {
        JDManagerFileSystem::unlockFile();
        return false;
    }

    double subProgress = 1;
    if(progress)
        subProgress = (1 - progress->getProgress()) / 3.0;

    
    const bool hasOverrideChangeFromDatabaseSlots = m_signals.objectOverrideChangeFromDatabase.signal.getSlotCount();
    const bool hasChangeFromDatabaseSlots = m_signals.objectChangedFromDatabase.signal.getSlotCount();
    const bool hasObjectAddedToDatabase = m_signals.objectAddedToDatabase.signal.getSlotCount();
    const bool hasObjectRemovedFromDatabase = m_signals.objectRemovedFromDatabase.signal.getSlotCount();

    std::vector<JDObjectInterface*> overridingObjs;
    std::vector<JDObjectInterface*> newObjs;
    std::vector<JDObjectInterface*> removedObjs;
    std::vector<JDObjectInterface*> replaceObjs;
    std::vector<JDObjectPair> pairsForSignal;

    std::unordered_map<JDObjectInterface*, JDObjectInterface*> loadedObjects;
    loadedObjects.reserve(jsons.size());

    if (modeChangedObjects && pairs.size() > 0)
    {
        double dProgress = subProgress / pairs.size();
        if (overrideChanges)
        {
            JD_GENERAL_PROFILING_BLOCK("Deserialize objects override mode", JD_COLOR_STAGE_2);
            if (progress) progress->setComment("Deserializing objects override mode");
            
            overridingObjs.reserve(pairs.size());
            // Loads the existing objects and overrides the data in the current object instance
            for (Pair& pair : pairs)
            {

                if (hasOverrideChangeFromDatabaseSlots)
                {
                    bool hasChanged = false;
                    success &= Internal::JsonUtilities::deserializeOverrideFromJson(pair.json, pair.objOriginal, hasChanged);
                    if (modeRemovedObjects) loadedObjects[pair.objOriginal] = pair.objOriginal;
                    if (hasChanged)
                    {
                        // The loaded object is not equal to the original object
                        overridingObjs.push_back(pair.objOriginal);
                    }
                }
                else
                    success &= Internal::JsonUtilities::deserializeOverrideFromJson(pair.json, pair.objOriginal);
                pair.obj = pair.objOriginal;
                if(progress) progress->addProgress(dProgress);
            }
            
        }
        else
        {
            JD_GENERAL_PROFILING_BLOCK("Deserialize objects reinstatiation mode", JD_COLOR_STAGE_2);
            if (progress) progress->setComment("Deserializing objects reinstatiation mode");
            
            pairsForSignal.reserve(pairs.size());

            // Loads the existing objects and creates a new object instance if the data has changed
            for (Pair& pair : pairs)
            {
                success &= Internal::JsonUtilities::deserializeJson(pair.json, pair.objOriginal, pair.obj);
                if (modeRemovedObjects) loadedObjects[pair.objOriginal] = pair.objOriginal;
                if (pair.objOriginal != pair.obj)
                {
                    // The loaded object is not equal to the original object
                    if (hasChangeFromDatabaseSlots)
                        pairsForSignal.push_back(JDObjectPair(pair.objOriginal, pair.obj));
                    replaceObjs.push_back(pair.obj);
                    //replaceObject_internal(pair.obj);
                }
                if (progress) progress->addProgress(dProgress);
            }
            
            
        }
        
    }
    else if (progress) progress->addProgress(subProgress);

    if (modeNewObjects && newObjectPairs.size() > 0)
    {
        JD_GENERAL_PROFILING_BLOCK("Deserialize and create new objects", JD_COLOR_STAGE_2);
        if (progress) progress->setComment("Deserializing and creating new objects");
        double dProgress = subProgress / newObjectPairs.size();
        
        
        newObjs.reserve(newObjectPairs.size());
        // Loads the new objects and creates a new object instance
        for (Pair& pair : newObjectPairs)
        {
            success &= Internal::JsonUtilities::deserializeJson(pair.json, pair.objOriginal, pair.obj);
            if (modeRemovedObjects) loadedObjects[pair.obj] = pair.obj;
            // Add the new generated object to the database
            //addObject_internal(pair.obj);
            newObjs.emplace_back(pair.obj);
            if (progress) progress->addProgress(dProgress);
        }
        
        
    }
    else if (progress) progress->addProgress(subProgress);

    if (modeRemovedObjects)
    {
        JD_GENERAL_PROFILING_BLOCK("Search for erased objects", JD_COLOR_STAGE_2);
        if (progress) progress->setComment("Searching for erased objects");
        std::vector<JDObjectInterface*> allObjs = getObjects_internal();
        double dProgress = subProgress / (allObjs.size()+1);
        
        removedObjs.reserve(allObjs.size());
        for (auto obj : allObjs)
        {
            if (loadedObjects.find(obj) != loadedObjects.end())
				continue;
            /*for (const Pair& pair : pairs)
            {
                if (pair.obj == allObjs[i])
                    goto nextObj;
            }
            for (const Pair& pair : newObjectPairs)
            {
                if (pair.obj == allObjs[i])
                    goto nextObj;
            }
            */
            if (hasObjectRemovedFromDatabase)
                removedObjs.emplace_back(obj);
                
            
        nextObj:;
            if (progress) progress->addProgress(dProgress);
        }
        
    }
    else if (progress) progress->addProgress(subProgress);
    JDManagerFileSystem::unlockFile();


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
            replaceObject_internal(replaceObjs);
            if (hasChangeFromDatabaseSlots && pairsForSignal.size())
                m_signals.objectChangedFromDatabase.addPairs(pairsForSignal);
		}
    }
    if (modeRemovedObjects)
    {
        removeObject_internal(removedObjs);
        if (hasObjectRemovedFromDatabase && removedObjs.size())
            m_signals.objectRemovedFromDatabase.addObjs(removedObjs);
    }
    if (modeNewObjects)
    {
        addObject_internal(newObjs);
        if (hasObjectAddedToDatabase && newObjs.size())
            m_signals.objectAddedToDatabase.addObjs(newObjs);
    }
    

    return success;
}
bool JDManager::saveObject_internal(JDObjectInterface* obj, unsigned int timeoutMillis, Internal::WorkProgress* progress)
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
    if (!obj)
        return false;
    bool success = true;
    bool wasLockedForWritingByOther = false;
    bool hasLock = JDManagerFileSystem::lockFile(getDatabasePath(), getDatabaseName(), FileReadWriteLock::Access::readWrite, wasLockedForWritingByOther, timeoutMillis);
    if (wasLockedForWritingByOther)
    {
        m_signals.addToQueue(Internal::JDManagerSignals::Signals::signal_databaseOutdated, true);
    	return false;
    }
    if (!hasLock)
    {
        return false;
    }

    if (progress) progress->setComment("Serializing object");
    std::string ID = obj->getObjectID();
    std::vector<QJsonObject> jsons;

    QJsonObject data;
    success &= obj->saveInternal(data);

    if (progress)
    {
        progress->setProgress(0.33);
        progress->setComment("Reading database file");
    }

    success &= JDManagerFileSystem::readJsonFile(jsons, 
        getDatabasePath(),
        getDatabaseName(), 
        Internal::JDManagerFileSystem::getJsonFileEnding(), 
        m_useZipFormat, 
        false);
    size_t index = JDObjectInterface::getJsonIndexByID(jsons, ID);

    if (index == std::string::npos)
    {
        jsons.push_back(data);
    }
    else
    {
        jsons[index] = data;
    }

    if (progress)
    { 
        progress->addProgress(0.33);
        progress->setComment("Writing database file"); 
    }

    // Save the serialized objects
    success &= JDManagerFileSystem::writeJsonFile(jsons, 
        getDatabasePath(), 
        getDatabaseName(), 
        Internal::JDManagerFileSystem::getJsonFileEnding(), 
        m_useZipFormat, 
        false);
    if (progress) progress->addProgress(0.33);
    JDManagerFileSystem::unlockFile();
    return success;
}
bool JDManager::saveObjects_internal(unsigned int timeoutMillis, Internal::WorkProgress* progress)
{
    std::vector<JDObjectInterface*> objs = JDManagerObjectManager::getObjects();
    return saveObjects_internal(objs, timeoutMillis, progress);
}
bool JDManager::saveObjects_internal(const std::vector<JDObjectInterface*>& objList, unsigned int timeoutMillis, Internal::WorkProgress* progress)
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
    if(objList.size() == 0)
		return true;
    bool wasLockedForWritingByOther = false;
    
    bool hasLock = JDManagerFileSystem::lockFile(getDatabasePath(), getDatabaseName(), FileReadWriteLock::Access::write, wasLockedForWritingByOther, timeoutMillis);
    if (wasLockedForWritingByOther)
    {
        m_signals.addToQueue(Internal::JDManagerSignals::Signals::signal_databaseOutdated, true);
        return false;
    }
    if (!hasLock)
    {
        JD_CONSOLE("bool JDManager::saveObjects_internal(vector<JDObjectInterface*>&, timeout=" << timeoutMillis << "ms) Can't lock database\n");
        return false;
    }
    bool success = true;

    if (progress) progress->setComment("Serializing objects");
    std::vector<QJsonObject> jsonData;
   
    
    if (progress)
    {
        double dProgress = 0.6 / objList.size();
        success &= Internal::JsonUtilities::getJsonArray(objList, jsonData, progress, dProgress);
        progress->setComment("Writing database file");
    }
	else
	{
		success &= Internal::JsonUtilities::getJsonArray(objList, jsonData);
        progress->addProgress(0.6);
	}
    

    // Save the serialized objects
    success &= JDManagerFileSystem::writeJsonFile(jsonData, 
        getDatabasePath(), 
        getDatabaseName(), Internal::JDManagerFileSystem::getJsonFileEnding(), 
        m_useZipFormat, 
        false);
    if (progress) progress->addProgress(0.4);
    JDManagerFileSystem::unlockFile();
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



const std::string& JDManager::getUser() const
{
    return m_user;
}
const std::string& JDManager::getSessionID() const
{
    return m_sessionID;
}


const std::string& JDManager::getLoadModeStr(int mode) const
{
    static std::string str;
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
    JDObjectLocker::update();

    
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
