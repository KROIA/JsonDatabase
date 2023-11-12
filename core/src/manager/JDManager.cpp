#include "manager/JDManager.h"
#include "manager/JDManagerSignals.h"
#include "object/JDObjectInterface.h"
#include "object/JDObjectRegistry.h"
#include "utilities/JDUniqueMutexLock.h"
#include "utilities/SystemCommand.h"
#include "utilities/JsonUtilities.h"




namespace JsonDatabase
{

    
    const QString JDManager::s_timeFormat = "hh:mm:ss.zzz";
    const QString JDManager::s_dateFormat = "dd.MM.yyyy";

    const QString JDManager::s_tag_sessionID = "sessionID";
    const QString JDManager::s_tag_user = "user";
    const QString JDManager::s_tag_date = "date";
    const QString JDManager::s_tag_time = "time";

    const unsigned int JDManager::s_fileLockTimeoutMs = 100;


    //std::map<std::string, JDObjectInterface*> JDManager::s_objDefinitions;
    std::mutex JDManager::s_mutex;

    void JDManager::startProfiler()
    {
#ifdef JD_PROFILING
        EASY_PROFILER_ENABLE;
#endif
    }
    void JDManager::stopProfiler(const std::string profileFilePath)
    {
#ifdef JD_PROFILING
        profiler::dumpBlocksToFile("JDProfile.prof");
#endif
    }

    JDManager::JDManager(const std::string& databasePath,
        const std::string& databaseName,
        const std::string& sessionID,
        const std::string& user)
        : JDManagerObjectManager(m_mutex)
        , JDManagerFileSystem(*this, m_mutex)
        , JDObjectLocker(*this, m_mutex)
        , m_databasePath(databasePath)
        , m_databaseName(databaseName)
        , m_sessionID(sessionID)
        , m_user(user)
        , m_useZipFormat(false)
        , m_signleEntryUpdateLock(false)
    {
        JDManagerObjectManager::setup();
        JDManagerFileSystem::setup();
        JDObjectLocker::setup();
    }
JDManager::JDManager(const JDManager &other)
    : JDManagerObjectManager(m_mutex)
    , JDManagerFileSystem(*this, m_mutex)
    , JDObjectLocker(*this, m_mutex)
    , m_sessionID(other.m_sessionID)
    , m_user(other.m_user)
    , m_useZipFormat(other.m_useZipFormat)
    , m_signleEntryUpdateLock(false)
    {
        JDManagerObjectManager::setup();
        JDManagerFileSystem::setup();
        JDObjectLocker::setup();
    }
JDManager::~JDManager()
{
    JDObjectLocker::unlockAllObjs();
}
void JDManager::setDatabaseName(const std::string& name)
{
    JDM_UNIQUE_LOCK_P;
    m_databaseName = name;
    JDManagerFileSystem::restartFileWatcher();
}

void JDManager::setDatabasePath(const std::string &path)
{
    JDM_UNIQUE_LOCK_P;
    if (path == m_databasePath)
        return;
    JDObjectLocker::onDatabasePathChange(m_databasePath, path);
    m_databasePath = path;
    JDManagerFileSystem::makeDatabaseDirs();
    JDManagerFileSystem::makeDatabaseFiles();
    JDManagerFileSystem::restartFileWatcher();
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
    return  m_databasePath + "//" + m_databaseName + Internal::JDManagerFileSystem::s_jsonFileEnding;
}


void JDManager::enableZipFormat(bool enable)
{
    m_useZipFormat = enable;
}
bool JDManager::isZipFormatEnabled() const
{
    return m_useZipFormat;
}

bool JDManager::saveObject(JDObjectInterface* obj)
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
    JDM_UNIQUE_LOCK_P;
    if (!obj)
        return false;
    bool success = true;
    if (!JDManagerFileSystem::lockFile(getDatabasePath(), getDatabaseName(), FileReadWriteLock::Access::readWrite, s_fileLockTimeoutMs))
    {
        JD_CONSOLE_FUNCTION(" Can't lock database\n");
        return false;
    }

    std::string ID = obj->getObjectID();
    std::vector<QJsonObject> jsons;

    QJsonObject data;
    success &= obj->saveInternal(data);

    
    success &= JDManagerFileSystem::readJsonFile(jsons, getDatabasePath(), getDatabaseName(), s_jsonFileEnding, m_useZipFormat, false);
    size_t index = JDObjectInterface::getJsonIndexByID(jsons, ID);
    
    if (index == std::string::npos)
    {
        jsons.push_back(data);
    }
    else
    {
        jsons[index] = data;
    }

    JDManagerFileSystem::pauseFileWatcher();
    // Save the serialized objects
    success &= JDManagerFileSystem::writeJsonFile(jsons, getDatabasePath(), getDatabaseName(), s_jsonFileEnding, m_useZipFormat, false);
    
    JDManagerFileSystem::unpauseFileWatcher();
    JDManagerFileSystem::unlockFile();
    return true;
}
bool JDManager::saveObjects()
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
    JDM_UNIQUE_LOCK_P;
    return saveObjects_internal(m_objs.getAllObjects());
}
bool JDManager::saveObjects(const std::vector<JDObjectInterface*> &objList)
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
    JDM_UNIQUE_LOCK_P;
    return saveObjects_internal(objList);
}
bool JDManager::saveObjects_internal(const std::vector<JDObjectInterface*>& objList)
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
    if (!JDManagerFileSystem::lockFile(getDatabasePath(), getDatabaseName(), FileReadWriteLock::Access::write))
    {
        JD_CONSOLE_FUNCTION(" Can't lock database\n");
        if (getLastLockError() == FileLock::Error::alreadyLockedForWriting)
        {
            m_signals.addToQueue(Internal::JDManagerSignals::Signals::signal_databaseOutdated);
        }
        return false;
    }
    bool success = true;

    std::vector<QJsonObject> jsonData;
    success &= Internal::JsonUtilities::getJsonArray(objList, jsonData);

    JDManagerFileSystem::pauseFileWatcher();

    // Save the serialized objects
    success &= JDManagerFileSystem::writeJsonFile(jsonData, getDatabasePath(), getDatabaseName(), s_jsonFileEnding, m_useZipFormat, false);

    JDManagerFileSystem::unpauseFileWatcher();
    JDManagerFileSystem::unlockFile();
    return success;
}

bool JDManager::loadObject(JDObjectInterface* obj)
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
    JDM_UNIQUE_LOCK_P;
    if (!JDManagerObjectManager::exists_internal(obj))
        return false;

    if (!JDManagerFileSystem::lockFile(getDatabasePath(), getDatabaseName(), FileReadWriteLock::Access::read, s_fileLockTimeoutMs))
    {
        JD_CONSOLE_FUNCTION(" Can't lock database\n");
        return false;
    }

    bool success = true;
    std::string ID = obj->getObjectID();

    std::vector<QJsonObject> jsons;
    success &= JDManagerFileSystem::readJsonFile(jsons, getDatabasePath(), getDatabaseName(), s_jsonFileEnding, m_useZipFormat, false);

    size_t index = JDObjectInterface::getJsonIndexByID(jsons, ID);
    if (index == std::string::npos)
    {
        JD_CONSOLE_FUNCTION("Object with ID: " << ID << " not found");
        return false;
    }
    const QJsonObject &objData = jsons[index];
    bool hasChanged = false;
    success &= Internal::JsonUtilities::deserializeOverrideFromJson(objData, obj, hasChanged);
    JDManagerFileSystem::unlockFile();

    return success;
}
bool JDManager::loadObjects(int mode)
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
    JDM_UNIQUE_LOCK_P;
    bool success = true;

    if (!JDManagerFileSystem::lockFile(getDatabasePath(), getDatabaseName(), FileReadWriteLock::Access::read, s_fileLockTimeoutMs))
    {
        JD_CONSOLE_FUNCTION(" Can't lock database\n");
        return false;
    }

    bool modeNewObjects = (mode & (int)LoadMode::newObjects);
    bool modeChangedObjects = (mode & (int)LoadMode::changedObjects);
    bool modeRemovedObjects = (mode & (int)LoadMode::removedObjects);

    bool overrideChanges = (mode & (int)LoadMode::overrideChanges);

    m_signals.clearContainer();
    
    
    
    std::vector<QJsonObject> jsons;
    success &= JDManagerFileSystem::readJsonFile(jsons, getDatabasePath(), getDatabaseName(), s_jsonFileEnding, m_useZipFormat, false);

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
        for (size_t i = 0; i < jsons.size(); ++i)
        {
            std::string ID;
            if (!JDSerializable::getJsonValue(jsons[i], ID, JDObjectInterface::s_tag_objID))
            {
                JD_CONSOLE_FUNCTION("Object with no ID found: " << QJsonValue(jsons[i]).toString().toStdString() + "\n");
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
    }
    if (!success)
        return false;            
    
    if (modeChangedObjects)
    {
        if (overrideChanges)
        {
            JD_GENERAL_PROFILING_BLOCK("Deserialize objects override mode", JD_COLOR_STAGE_2);
            bool hasOverrideChangeFromDatabaseSlots = m_signals.objectOverrideChangeFromDatabase.signal.getSlotCount();
            // Loads the existing objects and overrides the data in the current object instance
            for (Pair& pair : pairs)
            {
                
                if (hasOverrideChangeFromDatabaseSlots)
                {
                    bool hasChanged = false;
                    success &= Internal::JsonUtilities::deserializeOverrideFromJson(pair.json, pair.objOriginal, hasChanged);
                    if (hasChanged)
                    {
                        // The loaded object is not equal to the original object
                        m_signals.objectOverrideChangeFromDatabase.container.addObject(pair.objOriginal);
                    }
                }
                else
                    success &= Internal::JsonUtilities::deserializeOverrideFromJson(pair.json, pair.objOriginal);
                pair.obj = pair.objOriginal;
            }
        }
        else
        {
            JD_GENERAL_PROFILING_BLOCK("Deserialize objects reinstatiation mode", JD_COLOR_STAGE_2);
            bool hasChangeFromDatabaseSlots = m_signals.objectChangedFromDatabase.signal.getSlotCount();
            // Loads the existing objects and creates a new object instance if the data has changed
            for (Pair& pair : pairs)
            {
                success &= Internal::JsonUtilities::deserializeJson(pair.json, pair.objOriginal, pair.obj);

                if (pair.objOriginal != pair.obj)
                {
                    // The loaded object is not equal to the original object
                    if(hasChangeFromDatabaseSlots)
                        m_signals.objectChangedFromDatabase.container.push_back(JDObjectPair(pair.objOriginal, pair.obj));
                    replaceObject_internal(pair.obj);
                }
            }
        }
    }

    if(modeNewObjects)
    {
        JD_GENERAL_PROFILING_BLOCK("Deserialize and create new objects", JD_COLOR_STAGE_2);
        bool hasObjectAddedToDatabase = m_signals.objectAddedToDatabase.signal.getSlotCount();
        // Loads the new objects and creates a new object instance
        for (Pair& pair : newObjectPairs)
        {
            success &= Internal::JsonUtilities::deserializeJson(pair.json, pair.objOriginal, pair.obj);
            // Add the new generated object to the database
            addObject_internal(pair.obj);
            if(hasObjectAddedToDatabase)
                m_signals.objectAddedToDatabase.container.addObject(pair.obj);
        }
    }
    
    if(modeRemovedObjects)
    {
        JD_GENERAL_PROFILING_BLOCK("Search for erased objects", JD_COLOR_STAGE_2);
        std::vector<JDObjectInterface*> allObjs = getObjects_internal();
        m_signals.objectRemovedFromDatabase.container.reserve(allObjs.size());
        bool hasObjectRemovedFromDatabase = m_signals.objectRemovedFromDatabase.signal.getSlotCount();
        for (size_t i = 0; i < allObjs.size(); ++i)
        {
            for (const Pair& pair : pairs)
            {
                if (pair.obj == allObjs[i])
                    goto nextObj;
            }
            for (const Pair& pair : newObjectPairs)
            {
                if (pair.obj == allObjs[i])
                    goto nextObj;
            }

            if(hasObjectRemovedFromDatabase)
                m_signals.objectRemovedFromDatabase.container.addObject(allObjs[i]);
            removeObject_internal(allObjs[i]);
            nextObj:;
        }
    }

    JDManagerFileSystem::unlockFile();
    return success;
}


const std::string& JDManager::getUser() const
{
    return m_user;
}
const std::string& JDManager::getSessionID() const
{
    return m_sessionID;
}


void JDManager::update()
{
    if(m_signleEntryUpdateLock)
        return; // Update is already running
    m_signleEntryUpdateLock = true;
    JDM_UNIQUE_LOCK_M(m_updateMutex);
    
    //JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
    if (JDManagerFileSystem::fileWatcherHasFileChanged())
    {
        m_signals.databaseFileChanged.emitSignal();
        JDManagerFileSystem::clearFileWatcherHasFileChanged();
    }

    m_signals.objectAddedToDatabase.emitSignalIfNotEmpty();
    m_signals.objectChangedFromDatabase.emitSignalIfNotEmpty();
    m_signals.objectOverrideChangeFromDatabase.emitSignalIfNotEmpty();
    m_signals.objectRemovedFromDatabase.emitSignalIfNotEmpty();
    m_signals.emitQueue();
    m_signleEntryUpdateLock = false;
}
Internal::JDManagerSignals& JDManager::getSignals()
{
    return m_signals;
}

}
