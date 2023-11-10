#include "manager/JDManager.h"
#include "object/JDObjectInterface.h"
#include "object/JDObjectRegistry.h"
#include "utilities/JDUniqueMutexLock.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QDateTime>
#include <QDebug>
#include <thread>
#include <qcoreapplication.h>
#include <QDirIterator>


#include <windows.h>
#include <filesystem>
#include <algorithm>
#include <execution>


#include <QtZlib/zlib.h>

#include <time.h>
#include <winsock.h>
#include <Wincon.h>
#include <winuser.h>



namespace JsonDatabase
{

    const std::string JDManager::s_jsonFileEnding = ".json";

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
        : m_databasePath(databasePath)
        , m_databaseName(databaseName)
        , m_sessionID(sessionID)
        , m_user(user)
        , m_lockTable(this)
        , m_fileLock(nullptr)
        , m_useZipFormat(false)
        , m_databaseFileWatcher(nullptr)
#ifdef JSON_DATABSE_USE_THREADS
        , m_threadWorker("JDManager File IO")
        , m_threadWorker_fileFinder("JDManager File finder")
#endif
{
        makeDatabaseDirs();
        restartFileWatcher();

#ifdef JSON_DATABSE_USE_THREADS
    setupThreadWorker();
#endif


}
JDManager::JDManager(const JDManager &other)
    :   m_databasePath(other.m_databasePath)
    ,   m_databaseName(other.m_databaseName)
    ,   m_sessionID(other.m_sessionID)
    ,   m_user(other.m_user)
    ,   m_lockTable(this)
    ,   m_fileLock(nullptr)
    ,   m_useZipFormat(other.m_useZipFormat)
    ,   m_databaseFileWatcher(nullptr)
    //,   m_objDefinitions(other.m_objDefinitions)
#ifdef JSON_DATABSE_USE_THREADS
    ,  m_threadWorker("JDManager File IO")
    ,  m_threadWorker_fileFinder("JDManager File finder")
#endif
{
    makeDatabaseDirs();
    restartFileWatcher();
#ifdef JSON_DATABSE_USE_THREADS
    setupThreadWorker();
#endif
}
JDManager::~JDManager()
{
    if (m_databaseFileWatcher)
    {
        m_databaseFileWatcher->stopWatching();
        delete m_databaseFileWatcher;
    }
    m_lockTable.unlockAll();
#ifdef JSON_DATABSE_USE_THREADS
    m_threadWorker.stop();
    for (size_t i = 0; i < m_threadJobs.size(); ++i)
    {
        delete m_threadJobs[i];
    }
    m_threadWorker_fileFinder.stop();
    delete m_threadJsonFinderJob;
#endif
}
#ifdef JSON_DATABSE_USE_THREADS
void JDManager::setupThreadWorker()
{
    size_t numThreads = std::thread::hardware_concurrency();
    numThreads = min(numThreads, JSON_DATABSE_MAX_THREAD_COUNT);
    m_threadJobs.reserve(numThreads);
    for (size_t i = 0; i < numThreads; ++i)
    {
        ThreadWorkObj* job = new ThreadWorkObj(this);
        m_threadJobs.push_back(job);
        m_threadWorker.addWork(job);
    }
    m_threadWorker.setWaitingFunc([] {
        QTUpdateEvents();
        std::this_thread::sleep_for(std::chrono::microseconds(1));
        });

    m_threadJsonFinderJob = new ThreadWorkFindJsonFiles(this);
    m_threadWorker_fileFinder.addWork(m_threadJsonFinderJob);
    m_threadWorker_fileFinder.setWaitingFunc([] {
        QTUpdateEvents();
        std::this_thread::sleep_for(std::chrono::microseconds(1));
        });
}
#endif
bool JDManager::isInObjectDefinition(const std::string &className)
{
    JDM_UNIQUE_LOCK_P_M(s_mutex);
    if(JDObjectRegistry::getRegisteredTypes().find(className) != JDObjectRegistry::getRegisteredTypes().end())
        return true;
    return false;
}
void JDManager::setDatabaseName(const std::string& name)
{
    JDM_UNIQUE_LOCK_P;
    m_databaseName = name;
    restartFileWatcher();
}
const std::string& JDManager::getDatabaseName() const
{
    return m_databaseName;
}
void JDManager::setDatabasePath(const std::string &path)
{
    JDM_UNIQUE_LOCK_P;
    if (path == m_databasePath)
        return;
    m_lockTable.onDatabasePathChange(m_databasePath, path);
    m_databasePath = path;
    makeDatabaseDirs();
    restartFileWatcher();
}
const std::string &JDManager::getDatabasePath() const
{
    return m_databasePath;
}
void JDManager::enableZipFormat(bool enable)
{
    m_useZipFormat = enable;
}
bool JDManager::isZipFormatEnabled() const
{
    return m_useZipFormat;
}

bool JDManager::saveObject(JDObjectInterface* obj) const
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
    JDM_UNIQUE_LOCK_P;
    if (!obj)
        return false;
    bool success = true;
    if (!lockFile(getDatabasePath(), getDatabaseName(), FileReadWriteLock::Access::readWrite, s_fileLockTimeoutMs))
    {
        JD_CONSOLE_FUNCTION(" Can't lock database\n");
        return false;
    }

    std::string ID = obj->getObjectID();
    std::vector<QJsonObject> jsons;

    QJsonObject data;
    success &= obj->saveInternal(data);

    
    success &= readJsonFile(jsons, getDatabasePath(), getDatabaseName(), s_jsonFileEnding, m_useZipFormat, false);
    size_t index = JDObjectInterface::getJsonIndexByID(jsons, ID);
    
    if (index == std::string::npos)
    {
        jsons.push_back(data);
    }
    else
    {
        jsons[index] = data;
    }

    if (m_databaseFileWatcher)
        m_databaseFileWatcher->pause();
    // Save the serialized objects
    success &= writeJsonFile(jsons, getDatabasePath(), getDatabaseName(), s_jsonFileEnding, m_useZipFormat, false);
    
    if (m_databaseFileWatcher)
        m_databaseFileWatcher->unpause();
    unlockFile();
    return true;
}
bool JDManager::saveObjects() const
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
    JDM_UNIQUE_LOCK_P;
    return saveObjects_internal(m_objs.getAllObjects());
}
bool JDManager::saveObjects(const std::vector<JDObjectInterface*> &objList) const
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
    JDM_UNIQUE_LOCK_P;
    return saveObjects_internal(objList);
}
bool JDManager::saveObjects_internal(const std::vector<JDObjectInterface*>& objList) const
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
    if (!lockFile(getDatabasePath(), getDatabaseName(), FileReadWriteLock::Access::write))
    {
        JD_CONSOLE_FUNCTION(" Can't lock database\n");
        if (getLastLockError() == FileLock::Error::alreadyLockedForWriting)
        {

        }
        return false;
    }
    bool success = true;

    std::vector<QJsonObject> jsonData;
    success &= getJsonArray(objList, jsonData);

    if (m_databaseFileWatcher)
        m_databaseFileWatcher->pause();

    // Save the serialized objects
    success &= writeJsonFile(jsonData, getDatabasePath(), getDatabaseName(), s_jsonFileEnding, m_useZipFormat, false);

    if (m_databaseFileWatcher)
        m_databaseFileWatcher->unpause();
    unlockFile();
    return success;
}

bool JDManager::loadObject(JDObjectInterface* obj)
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
    JDM_UNIQUE_LOCK_P;
    if (!exists_internal(obj))
        return false;

    if (!lockFile(getDatabasePath(), getDatabaseName(), FileReadWriteLock::Access::read, s_fileLockTimeoutMs))
    {
        JD_CONSOLE_FUNCTION(" Can't lock database\n");
        return false;
    }

    bool success = true;
    std::string ID = obj->getObjectID();

    std::vector<QJsonObject> jsons;
    success &= readJsonFile(jsons, getDatabasePath(), getDatabaseName(), s_jsonFileEnding, m_useZipFormat, false);

    size_t index = JDObjectInterface::getJsonIndexByID(jsons, ID);
    if (index == std::string::npos)
    {
        JD_CONSOLE_FUNCTION("Object with ID: " << ID << " not found");
        return false;
    }
    const QJsonObject &objData = jsons[index];
    bool hasChanged = false;
    success &= deserializeOverrideFromJson(objData, obj, hasChanged);
    unlockFile();

    return success;
}
bool JDManager::loadObjects(int mode)
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
    JDM_UNIQUE_LOCK_P;
    bool success = true;

    if (!lockFile(getDatabasePath(), getDatabaseName(), FileReadWriteLock::Access::read, s_fileLockTimeoutMs))
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
    success &= readJsonFile(jsons, getDatabasePath(), getDatabaseName(), s_jsonFileEnding, m_useZipFormat, false);

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
                    success &= deserializeOverrideFromJson(pair.json, pair.objOriginal, hasChanged);
                    if (hasChanged)
                    {
                        // The loaded object is not equal to the original object
                        m_signals.objectOverrideChangeFromDatabase.container.addObject(pair.objOriginal);
                    }
                }
                else
                    success &= deserializeOverrideFromJson(pair.json, pair.objOriginal);
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
                success &= deserializeJson(pair.json, pair.objOriginal, pair.obj);

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
            success &= deserializeJson(pair.json, pair.objOriginal, pair.obj);
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

    unlockFile();
    return success;
}

void JDManager::QTUpdateEvents()
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents | 
                                    QEventLoop::ExcludeSocketNotifiers |
                                    QEventLoop::X11ExcludeTimers);
}
void JDManager::restartFileWatcher()
{
    if (m_databaseFileWatcher)
    {
        m_databaseFileWatcher->stopWatching();
        delete m_databaseFileWatcher;
        m_databaseFileWatcher = nullptr;
    }
    m_databaseFileWatcher = new FileChangeWatcher(getDatabaseFilePath());
    m_databaseFileWatcher->startWatching();
}

void JDManager::clearObjects()
{
    JDM_UNIQUE_LOCK_P;
    m_objs.clear();
}
bool JDManager::addObject(JDObjectInterface* obj)
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
    JDM_UNIQUE_LOCK_P;
    return addObject_internal(obj);
}
bool JDManager::addObject(const std::vector<JDObjectInterface*> &objList)
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
    JDM_UNIQUE_LOCK_P;
    bool success = true;
    for(size_t i=0; i<objList.size(); ++i)
        success &= addObject_internal(objList[i]);
    return success;
}
bool JDManager::addObject_internal(JDObjectInterface* obj)
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
    if (!obj) return false;
    if (exists_internal(obj)) return false;
    m_objs.addObject(obj);
    return true;

}
JDObjectInterface* JDManager::replaceObject(JDObjectInterface* obj)
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
    JDM_UNIQUE_LOCK_P;
    return replaceObject_internal(obj);
}
std::vector<JDObjectInterface*> JDManager::replaceObjects(const std::vector<JDObjectInterface*>& objList)
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
    JDM_UNIQUE_LOCK_P;
    std::vector<JDObjectInterface*> replacedObjs;
	replacedObjs.reserve(objList.size());
	for (size_t i = 0; i < objList.size(); ++i)
	{
		replacedObjs.push_back(replaceObject_internal(objList[i]));
	}
	return replacedObjs;
}
JDObjectInterface* JDManager::replaceObject_internal(JDObjectInterface* obj)
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
    return m_objs.replaceObject(obj);
}
bool JDManager::removeObject(JDObjectInterface* obj)
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
    JDM_UNIQUE_LOCK_P;
    return removeObject_internal(obj);
}
bool JDManager::removeObject_internal(JDObjectInterface* obj)
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
    if (!obj) return false;
    if (!exists_internal(obj)) return false;
    m_objs.removeObject(obj);
    return true;
}
bool JDManager::removeObjects(const std::vector<JDObjectInterface*> &objList)
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
    JDM_UNIQUE_LOCK_P;
    bool success = true;
    for(size_t i=0; i<objList.size(); ++i)
        success &= removeObject_internal(objList[i]);
    return success;
}
std::size_t JDManager::getObjectCount() const
{
    JDM_UNIQUE_LOCK_P;
    return m_objs.size();
}
bool JDManager::exists(JDObjectInterface* obj) const
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
    JDM_UNIQUE_LOCK_P;
    return exists_internal(obj);
}
bool JDManager::exists(const std::string &id) const
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
    JDM_UNIQUE_LOCK_P;
    return exists_internal(id);
}
bool JDManager::exists_internal(JDObjectInterface* obj) const
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
    if (!obj) return false;
    return m_objs.exists(obj);
}
bool JDManager::exists_internal(const std::string& id) const
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
    return m_objs.exists(id);
}

JDObjectInterface* JDManager::getObject(const std::string &objID)
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
    JDM_UNIQUE_LOCK_P;

    return getObject_internal(objID);
}
const std::vector<JDObjectInterface*>& JDManager::getObjects() const
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
    JDM_UNIQUE_LOCK_P;

    return getObjects_internal();
}

JDObjectInterface* JDManager::getObject_internal(const std::string& objID) 
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
    if (objID.size() == 0)
        return nullptr;
    return m_objs[objID];
}
const std::vector<JDObjectInterface*> &JDManager::getObjects_internal() const
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
    return m_objs.getAllObjects();
}

const std::string& JDManager::getUser() const
{
    return m_user;
}
const std::string& JDManager::getSessionID() const
{
    return m_sessionID;
}

bool JDManager::lockObj(JDObjectInterface* obj) const
{
    JDM_UNIQUE_LOCK_P;
    return m_lockTable.lock(obj);
}
bool JDManager::unlockObj(JDObjectInterface* obj) const
{
    JDM_UNIQUE_LOCK_P;
    return m_lockTable.unlock(obj);
}
bool JDManager::isObjLocked(JDObjectInterface* obj) const
{
    JDM_UNIQUE_LOCK_P;
    return m_lockTable.isLocked(obj);
}


void JDManager::update()
{
    JDM_UNIQUE_LOCK_M(m_updateMutex);
    
    //JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
    if (m_databaseFileWatcher)
    {
        if (m_databaseFileWatcher->hasFileChanged())
        {
           // JD_GENERAL_PROFILING_BLOCK("Database file changed", JD_COLOR_STAGE_2);
            m_signals.databaseFileChanged.emitSignal();
            m_databaseFileWatcher->clearFileChangedFlag();
        }
    }
    m_signals.objectAddedToDatabase.emitSignalIfNotEmpty();
    m_signals.objectChangedFromDatabase.emitSignalIfNotEmpty();
    m_signals.objectOverrideChangeFromDatabase.emitSignalIfNotEmpty();
    m_signals.objectRemovedFromDatabase.emitSignalIfNotEmpty();
}

void JDManager::connectDatabaseFileChangedSlot(const Signal<>::SlotFunction& slotFunction)
{
    m_signals.databaseFileChanged.connectSlot(slotFunction);
}
void JDManager::disconnectDatabaseFileChangedSlot(const Signal<>::SlotFunction& slotFunction)
{
    m_signals.databaseFileChanged.disconnectSlot(slotFunction);
}
void JDManager::connectObjectRemovedFromDatabaseSlot(const Signal<const JDObjectContainer&>::SlotFunction& slotFunction)
{
    m_signals.objectRemovedFromDatabase.signal.connectSlot(slotFunction);
}
void JDManager::disconnectObjectRemovedFromDatabaseSlot(const Signal<const JDObjectContainer&>::SlotFunction& slotFunction)
{
    m_signals.objectRemovedFromDatabase.signal.disconnectSlot(slotFunction);
}
void JDManager::connectObjectAddedToDatabaseSlot(const Signal<const JDObjectContainer&>::SlotFunction& slotFunction)
{
    m_signals.objectAddedToDatabase.signal.connectSlot(slotFunction);
}
void JDManager::disconnectObjectAddedToDatabaseSlot(const Signal<const JDObjectContainer&>::SlotFunction& slotFunction)
{
    m_signals.objectAddedToDatabase.signal.disconnectSlot(slotFunction);
}
void JDManager::connectObjectChangedFromDatabaseSlot(const Signal<const std::vector<JDObjectPair>&>::SlotFunction& slotFunction)
{
    m_signals.objectChangedFromDatabase.signal.connectSlot(slotFunction);
}
void JDManager::disconnectObjectChangedFromDatabaseSlot(const Signal<const std::vector<JDObjectPair>&>::SlotFunction& slotFunction)
{
    m_signals.objectChangedFromDatabase.signal.disconnectSlot(slotFunction);
}
void JDManager::connectObjectOverrideChangeFromDatabaseSlot(const Signal<const JDObjectContainer&>::SlotFunction& slotFunction)
{
    m_signals.objectOverrideChangeFromDatabase.signal.connectSlot(slotFunction);
}
void JDManager::disconnectObjectOverrideChangeFromDatabaseSlot(const Signal<const JDObjectContainer&>::SlotFunction& slotFunction)
{
    m_signals.objectOverrideChangeFromDatabase.signal.disconnectSlot(slotFunction);
}


void JDManager::onNewObjectsInstantiated(const std::vector<JDObjectInterface*>& newObjects)
{

}
std::string JDManager::getDatabaseFilePath() const
{
    return  m_databasePath + "//" + m_databaseName + s_jsonFileEnding;
}


bool JDManager::getJsonArray(const std::vector<JDObjectInterface*>& objs, std::vector<QJsonObject>& jsonOut) const
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
    jsonOut.reserve(objs.size());

    bool success = true;

    for (auto o : objs)
    {
        QJsonObject data;
        success &= o->saveInternal(data);
        jsonOut.push_back(data);
    }
    return success;
}
bool JDManager::serializeObject(JDObjectInterface* obj, std::string& serializedOut) const
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_4);
    if (!obj) return false;

    QJsonObject data;
    if (!obj->saveInternal(data))
        return false;

    return serializeJson(data, serializedOut);
}
bool JDManager::serializeJson(const QJsonObject& obj, std::string& serializedOut) const
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
    QJsonDocument document;
    document.setObject(obj);
    QByteArray bytes = document.toJson(QJsonDocument::Indented);

    serializedOut = bytes.constData();
    return true;
}
bool JDManager::deserializeJson(const QJsonObject& json, JDObjectInterface* objOriginal, JDObjectInterface*& objOut) const
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
    if(objOriginal)
    {
        std::string ID;
        JDSerializable::getJsonValue(json, ID, JDObjectInterface::s_tag_objID);
        if (objOriginal->equalData(json))
        {
            objOut = objOriginal;
            objOriginal->setVersion(json); // Update version value from loaded object
        }
        else
        {
            objOut = objOriginal->clone(json, ID);
        }
    }
    else
    {
        JDObjectInterface* clone = JDObjectRegistry::getObjectDefinition(json);
        if (!clone)
        {
            std::string className;
            JDSerializable::getJsonValue(json, className, JDObjectInterface::s_tag_className);

            JD_CONSOLE_FUNCTION("Objecttype: " << className.c_str() << " is not known by this database. "
                "Call: JDManager::addObjectDefinition<" << className.c_str() << ">(); first\n");
            return false;
        }

        std::string ID;
        JDSerializable::getJsonValue(json, ID, JDObjectInterface::s_tag_objID);

        objOut = clone->clone(json, ID);
    }
    return true;
}
bool JDManager::deserializeOverrideFromJson(const QJsonObject& json, JDObjectInterface* obj, bool& hasChangedOut) const
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
    if(!obj->equalData(json))
        hasChangedOut = true;
    if (!obj->loadInternal(json))
    {
        JD_CONSOLE_FUNCTION("Can't load data in object: " << obj->getObjectID() << " classType: " << obj->className() + "\n");
        return false;
    }
    return true;
}
bool JDManager::deserializeOverrideFromJson(const QJsonObject& json, JDObjectInterface* obj) const
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
    if (!obj->loadInternal(json))
    {
        JD_CONSOLE_FUNCTION("Can't load data in object: " << obj->getObjectID() << " classType: " << obj->className() + "\n");
        return false;
    }
    return true;
}
bool JDManager::lockFile(
    const std::string& directory,
    const std::string& fileName,
    FileReadWriteLock::Access direction) const
{
    if (m_fileLock)
    {
        JD_CONSOLE_FUNCTION("Lock already aquired\n");
        return false;
    }

    m_fileLock = new FileReadWriteLock(directory, fileName);

    if (!m_fileLock->lock(direction))
    {
        JD_CONSOLE_FUNCTION("Can't aquire lock for: " << directory << "\\" << fileName << "\n");
        delete m_fileLock;
        m_fileLock = nullptr;
        return false;
    }
    if (m_fileLock->getLastError() != FileLock::Error::none)
    {
        JD_CONSOLE_FUNCTION("Lock error: " << m_fileLock->getLastErrorStr() + "\n");
    }
    return true;
}
bool JDManager::lockFile(
    const std::string& directory,
    const std::string& fileName,
    FileReadWriteLock::Access direction,
    unsigned int timeoutMillis) const
{
    if (m_fileLock)
    {
        JD_CONSOLE_FUNCTION("Lock already aquired\n");
        return false;
    }

    m_fileLock = new FileReadWriteLock(directory, fileName);

    if (!m_fileLock->lock(direction, timeoutMillis))
    {
        JD_CONSOLE_FUNCTION("Timeout while trying to aquire file lock for: " << directory << "\\" << fileName << "\n");
        delete m_fileLock;
        m_fileLock = nullptr;
        return false;
    }
    if (m_fileLock->getLastError() != FileLock::Error::none)
    {
        JD_CONSOLE_FUNCTION("Lock error: " << m_fileLock->getLastErrorStr() + "\n");
    }
    return true;
}
bool JDManager::unlockFile() const
{
    //JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2); 
    //JD_GENERAL_PROFILING_FUNCTION_C(profiler::colors::Red);

    if (!m_fileLock)
        return true;

    m_fileLock->unlock();
    delete m_fileLock;
    m_fileLock = nullptr;

    return true;
}
bool JDManager::isFileLockedByOther(
    const std::string& directory,
    const std::string& fileName, 
    FileReadWriteLock::Access accessType) const
{
    FileReadWriteLock::Access a = FileReadWriteLock::Access::unknown;
    if (!m_fileLock)
    {
        FileReadWriteLock lock(directory, fileName);
        a = lock.getAccessStatus();
    }
    else
    {
        m_fileLock->getAccessStatus();
    }
    return a == accessType;
}
FileLock::Error JDManager::getLastLockError() const
{
    if (!m_fileLock)
        return FileLock::Error::none;
    return m_fileLock->getLastError();
}
const std::string& JDManager::getLastLockErrorStr() const
{
    if (!m_fileLock)
    {
        static const std::string dummy;
        return dummy;
    }
    return m_fileLock->getLastErrorStr();
}

bool JDManager::writeJsonFile(
    const std::vector<QJsonObject>& jsons, 
    const std::string& directory,
    const std::string& fileName,
    const std::string& fileEnding,
    bool zipFormat,
    bool lockedRead) const
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_5);
    JD_GENERAL_PROFILING_NONSCOPED_BLOCK("std::vector to QJsonArray", JD_COLOR_STAGE_6);
    QJsonArray jsonArray;

    // Convert QJsonObject instances to QJsonValue and add them to QJsonArray
    for (const auto& jsonObject : jsons) {
        jsonArray.append(QJsonValue(jsonObject));
    }
    JD_GENERAL_PROFILING_END_BLOCK;

    
    JD_GENERAL_PROFILING_NONSCOPED_BLOCK("toJson", JD_COLOR_STAGE_6);
    QJsonDocument jsonDocument(jsonArray);
    // Convert QJsonDocument to a QByteArray for writing to a file
    QByteArray data = jsonDocument.toJson(QJsonDocument::JsonFormat::Indented);
    JD_GENERAL_PROFILING_END_BLOCK;


    // Write the JSON data to the file
    if (zipFormat)
    {
        JD_GENERAL_PROFILING_NONSCOPED_BLOCK("compressing data", JD_COLOR_STAGE_6);
        QByteArray fileData;
        compressString(data, fileData);
        JD_GENERAL_PROFILING_END_BLOCK;
        
        return writeFile(fileData, directory, fileName, s_jsonFileEnding, lockedRead);
    }
    else
    {
        return writeFile(data, directory, fileName, s_jsonFileEnding, lockedRead);
    }
    return false;
}

bool JDManager::writeJsonFile(
    const QJsonObject& json,
    const std::string& directory,
    const std::string& fileName,
    const std::string& fileEnding,
    bool zipFormat,
    bool lockedRead) const
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_5);

    JD_GENERAL_PROFILING_NONSCOPED_BLOCK("toJson", JD_COLOR_STAGE_6);
    QJsonDocument jsonDocument(json);
    // Convert QJsonDocument to a QByteArray for writing to a file
    QByteArray data = jsonDocument.toJson(QJsonDocument::JsonFormat::Indented);
    JD_GENERAL_PROFILING_END_BLOCK;


    // Write the JSON data to the file
    if (zipFormat)
    {
        JD_GENERAL_PROFILING_NONSCOPED_BLOCK("compressing data", JD_COLOR_STAGE_6);
        QByteArray fileData;
        compressString(data, fileData);
        JD_GENERAL_PROFILING_END_BLOCK;

        return writeFile(fileData, directory, fileName, s_jsonFileEnding, lockedRead);
    }
    else
    {
        return writeFile(data, directory, fileName, s_jsonFileEnding, lockedRead);
    }
    return false;
}

bool JDManager::readJsonFile(
    std::vector<QJsonObject>& jsonsOut,
    const std::string& directory,
    const std::string& fileName,
    const std::string& fileEnding,
    bool zipFormat,
    bool lockedRead) const
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_5);
    QByteArray fileData;
    if (!readFile(fileData, directory, fileName, fileEnding, lockedRead))
    {
        return false;
    }

    // Parse the JSON data
    QJsonDocument jsonDocument;
    if (zipFormat)
    {
        JD_GENERAL_PROFILING_NONSCOPED_BLOCK("uncompressing data", JD_COLOR_STAGE_6);
        QString uncompressed;
        if (decompressString(fileData, uncompressed))
        {
            JD_GENERAL_PROFILING_END_BLOCK;
            JD_GENERAL_PROFILING_NONSCOPED_BLOCK("import json", JD_COLOR_STAGE_6);
            jsonDocument = QJsonDocument::fromJson(uncompressed.toUtf8());
            JD_GENERAL_PROFILING_END_BLOCK;
        }
        else
        {
            JD_GENERAL_PROFILING_END_BLOCK;
            JD_GENERAL_PROFILING_NONSCOPED_BLOCK("import json", JD_COLOR_STAGE_6);
            jsonDocument = QJsonDocument::fromJson(fileData);
            JD_GENERAL_PROFILING_END_BLOCK;
        }
    }
    else
    {
        JD_GENERAL_PROFILING_NONSCOPED_BLOCK("import json", JD_COLOR_STAGE_6);
        jsonDocument = QJsonDocument::fromJson(fileData);
        JD_GENERAL_PROFILING_END_BLOCK;
    }



    // Check if the JSON document is an array
    if (jsonDocument.isArray()) {
        JD_GENERAL_PROFILING_NONSCOPED_BLOCK("QJsonArray to std::vector", JD_COLOR_STAGE_6);
        QJsonArray jsonArray = jsonDocument.array();

        // Iterate through the array and add QJsonObjects to the vector
        jsonsOut.reserve(jsonArray.size());
        for (const auto& jsonValue : jsonArray) {
            if (jsonValue.isObject()) {
                jsonsOut.push_back(jsonValue.toObject());
            }
        }
        JD_GENERAL_PROFILING_END_BLOCK;
        return true;
    }
    else {
        JD_CONSOLE_FUNCTION("Error: JSON document from file: " << inputFile << " is not an array\n");
        return false;
    }
    
    return false;
}
bool JDManager::readJsonFile(
    QJsonObject& objOut,
    const std::string& directory,
    const std::string& fileName,
    const std::string& fileEnding,
    bool zipFormat,
    bool lockedRead) const
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_5);
    QByteArray fileData;
    if (!readFile(fileData, directory, fileName, fileEnding, lockedRead))
    {
        return false;
    }
    QJsonParseError jsonError;

    QJsonDocument document;
    if (zipFormat)
    {
        JD_GENERAL_PROFILING_NONSCOPED_BLOCK("uncompressing data", JD_COLOR_STAGE_6);
        QString uncompressed;
        if (decompressString(fileData, uncompressed))
        {
            JD_GENERAL_PROFILING_END_BLOCK;
            JD_GENERAL_PROFILING_NONSCOPED_BLOCK("import json", JD_COLOR_STAGE_6);
            document = QJsonDocument::fromJson(uncompressed.toUtf8());
            JD_GENERAL_PROFILING_END_BLOCK;
        }
        else
        {
            JD_GENERAL_PROFILING_END_BLOCK;
            JD_GENERAL_PROFILING_NONSCOPED_BLOCK("import json", JD_COLOR_STAGE_6);
            document = QJsonDocument::fromJson(fileData);
            JD_GENERAL_PROFILING_END_BLOCK;
        }
    }
    else
    {
        JD_GENERAL_PROFILING_NONSCOPED_BLOCK("import json", JD_COLOR_STAGE_6);
        document = QJsonDocument::fromJson(fileData);
        JD_GENERAL_PROFILING_END_BLOCK;
    }
    if (jsonError.error != QJsonParseError::NoError)
    {
        JD_CONSOLE_FUNCTION("Can't read Jsonfile: " << jsonError.errorString().toStdString().c_str() << "\n");
        return false;
    }
    if (document.isObject())
    {
        objOut = document.object();
        return true;
    }
    
    return false;

    /*QFile file((m_databasePath + "\\" + relativePath + fileEnding).c_str());
    if( file.open( QIODevice::ReadOnly ) )
    {
        QByteArray bytes = file.readAll();
        file.close();

        QJsonParseError jsonError;
        QJsonDocument document = QJsonDocument::fromJson( bytes, &jsonError );
        if( jsonError.error != QJsonParseError::NoError )
        {
            JD_CONSOLE_FUNCTION("Can't read Jsonfile: " << jsonError.errorString().toStdString().c_str() << "\n");
            return false;
        }
        if( document.isObject() )
        {
            obj = document.object();
            return true;
        }
    }
    JD_CONSOLE_FUNCTION("Can't open file: "<<(m_databasePath+"\\"+relativePath+fileEnding).c_str()<<"\n");
    return false;*/
}

bool JDManager::readFile(
    QByteArray& fileDataOut,
    const std::string& directory,
    const std::string& fileName,
    const std::string& fileEnding,
    bool lockedRead) const
{
    JDFILE_IO_PROFILING_FUNCTION(JD_COLOR_STAGE_5);
    if (lockedRead)
    {
        if (!lockFile(directory, fileName, FileReadWriteLock::Access::read))
        {
            JD_CONSOLE_FUNCTION(" Can't lock file\n");
            return false;
        }
    }
    /*
    JDFILE_IO_PROFILING_NONSCOPED_BLOCK("open file", JD_COLOR_STAGE_6);
    std::string filePath = directory + "\\" + fileName + fileEnding;
    QFile file(filePath.c_str());
    if (file.open(QIODevice::ReadOnly))
    {
        JDFILE_IO_PROFILING_END_BLOCK;
        JDFILE_IO_PROFILING_NONSCOPED_BLOCK("read from file", JD_COLOR_STAGE_6);
        fileDataOut = file.readAll();
        file.close();
        JDFILE_IO_PROFILING_END_BLOCK;
        if (lockedRead)
            unlockFile();
        return true;
    }
    else
    {
        JDFILE_IO_PROFILING_END_BLOCK;
    }
    JD_CONSOLE_FUNCTION("Can't open file: " << filePath.c_str() << "\n");
    if (lockedRead)
        unlockFile();
    return false;*/


    JDFILE_IO_PROFILING_NONSCOPED_BLOCK("open file", JD_COLOR_STAGE_6);
    std::string filePath = directory + "\\" + fileName + fileEnding;
    HANDLE fileHandle = CreateFile(
        filePath.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (fileHandle == INVALID_HANDLE_VALUE) {
        JDFILE_IO_PROFILING_END_BLOCK;
        JD_CONSOLE_FUNCTION("Can't open file: " << filePath.c_str() << "\n");
        if (lockedRead)
            unlockFile();
        return false;
    }
    JDFILE_IO_PROFILING_END_BLOCK;
    JDFILE_IO_PROFILING_NONSCOPED_BLOCK("read from file", JD_COLOR_STAGE_6);
    DWORD fileSize = GetFileSize(fileHandle, nullptr);
    if (fileSize == INVALID_FILE_SIZE) {
        CloseHandle(fileHandle);
        JD_CONSOLE_FUNCTION("Can't get filesize of: " << filePath.c_str() << "\n");
        JDFILE_IO_PROFILING_END_BLOCK;
        if (lockedRead)
            unlockFile();
        return false;
    }

    std::string content(fileSize, '\0');
    DWORD bytesRead;
    BOOL readResult = ReadFile(
        fileHandle,
        &content[0],
        fileSize,
        &bytesRead,
        nullptr
    );

    CloseHandle(fileHandle);
    JDFILE_IO_PROFILING_END_BLOCK;
    if (lockedRead)
        unlockFile();

    if (!readResult) {
        JD_CONSOLE_FUNCTION("Can't read file: " << filePath.c_str() << "\n");
    }

    fileDataOut = content.c_str();
    return true;
}
bool JDManager::writeFile(
    const QByteArray& fileData,
    const std::string& directory,
    const std::string& fileName,
    const std::string& fileEnding,
    bool lockedRead) const
{
    JDFILE_IO_PROFILING_FUNCTION(JD_COLOR_STAGE_5);
    if (lockedRead)
    {
        if (!lockFile(directory, fileName, FileReadWriteLock::Access::write))
        {
            JD_CONSOLE_FUNCTION(" Can't lock file\n");
            return false;
        }
    }
    
    /*
    // Open the file for writing
    JDFILE_IO_PROFILING_NONSCOPED_BLOCK("open file", JD_COLOR_STAGE_6);
    std::string filePath = directory + "\\" + fileName + fileEnding;
    QFile file(filePath.c_str());
    if (file.open(QIODevice::WriteOnly)) {
        // Write the JSON data to the file
        JDFILE_IO_PROFILING_END_BLOCK;
        JDFILE_IO_PROFILING_NONSCOPED_BLOCK("write to file", JD_COLOR_STAGE_6);
        file.write(fileData);
        file.close();
        JDFILE_IO_PROFILING_END_BLOCK;
        if (lockedRead)
            unlockFile();
        return true;
    }
    else {
        JDFILE_IO_PROFILING_END_BLOCK;
        JD_CONSOLE_FUNCTION("Error: Could not open file " << filePath << " for writing\n");
    }
    if (lockedRead)
        unlockFile();
    return false;*/

    // Open the file for writing
    JDFILE_IO_PROFILING_NONSCOPED_BLOCK("open file", JD_COLOR_STAGE_6);
    std::string filePath = directory + "\\" + fileName + fileEnding;
    HANDLE fileHandle = CreateFile(
        filePath.c_str(),
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (fileHandle == INVALID_HANDLE_VALUE) {
        JDFILE_IO_PROFILING_END_BLOCK;
        JD_CONSOLE_FUNCTION("Error: Could not open file " << filePath << " for writing\n");
        // Error opening file
        if (lockedRead)
            unlockFile();
        return false;
    }
    JDFILE_IO_PROFILING_END_BLOCK;

    JDFILE_IO_PROFILING_NONSCOPED_BLOCK("write to file", JD_COLOR_STAGE_6);
    // Write the content to the file
    DWORD bytesWritten;
    BOOL writeResult = WriteFile(
        fileHandle,
        fileData.constData(),
        fileData.size(),
        &bytesWritten,
        nullptr
    );
    

    // Close the file handle
    CloseHandle(fileHandle);
    JDFILE_IO_PROFILING_END_BLOCK;
    if (lockedRead)
        unlockFile();

    if (!writeResult) {
        // Error writing to file
        JD_CONSOLE_FUNCTION("Error: Could not write to file " << filePath << "\n");
        return false;
    }

    return true;
}


bool JDManager::makeDatabaseDirs() const
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2)
    bool success = true;

    std::string path = m_databasePath;
    QDir dir(path.c_str());
    if (!dir.exists())
    {
        QDir d;
        d.mkpath(path.c_str());
    }


    bool exists = dir.exists();
    if (!exists)
    {
        JD_CONSOLE_FUNCTION("Can't create database folder: " << path.c_str()<<"\n");
    }
    success &= exists;
    
    return success;
}

bool JDManager::deleteDir(const std::string& dir) const
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
    int ret = executeCommand("rd /s /q \"" + dir+"\"");
    QDir folder(dir.c_str());
    if (folder.exists())
    {
        JD_CONSOLE_FUNCTION(" Folder could not be deleted: "+dir+ "\n");
        return false;
    }
    return true;
}
bool JDManager::deleteFile(const std::string& file) const
{
    QFile f(file.c_str());
    if (f.exists())
        return f.remove();
    return true;
}

/*
bool JDManager::getJsonValue(const QJsonObject &obj, QVariant &value, const QString &key)
{
    if(obj.contains(key))
    {
        value = obj[key];
        return true;
    }
    return false;
}
bool JDManager::getJsonValue(const QJsonObject &obj, QTime &value, const QString &key)
{
    if(obj.contains(key))
    {
        value = QTime::fromString(obj[key].toString(), s_timeFormat);
        return true;
    }
    return false;
}
bool JDManager::getJsonValue(const QJsonObject &obj, QDate &value, const QString &key)
{
    if(obj.contains(key))
    {
        value = QDate::fromString(obj[key].toString(), s_dateFormat);
        return true;
    }
    return false;
}
bool JDManager::getJsonValue(const QJsonObject &obj, QString &value, const QString &key)
{
    if(obj.contains(key))
    {
        value = obj[key].toString();
        return true;
    }
    return false;
}
bool JDManager::getJsonValue(const QJsonObject &obj, std::string &value, const QString &key)
{
    if(obj.contains(key))
    {
        value = obj[key].toString().toStdString();
        return true;
    }
    return false;
}
bool JDManager::getJsonValue(const QJsonObject &obj, int &value, const QString &key)
{
    if(obj.contains(key))
    {
        value = obj[key].toInt(value);
        return true;
    }
    return false;
}
*/




int JDManager::executeCommand(const std::string& command)
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1)
    // Convert the command to wide string
    std::wstring wideCommand(command.begin(), command.end());
   // int execResult = WinExec(("C:\\Windows\\System32\\cmd.exe /c "+command).c_str(), SW_HIDE);
   // return execResult;
   // wideCommand += L" exit";

    // Create a STARTUPINFOW structure and set its properties
    STARTUPINFOW startupInfo;
    ZeroMemory(&startupInfo, sizeof(STARTUPINFOW));
    startupInfo.cb = sizeof(STARTUPINFOW);
    startupInfo.dwFlags = STARTF_USESHOWWINDOW;
    startupInfo.wShowWindow = SW_HIDE;  // Hide the console window

    // Create a PROCESS_INFORMATION structure
    PROCESS_INFORMATION processInfo;
    ZeroMemory(&processInfo, sizeof(PROCESS_INFORMATION));

    // Create the child process
    if (CreateProcessW(NULL, const_cast<LPWSTR>((L"C:\\Windows\\System32\\cmd.exe /c "+wideCommand).c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo))
    {
        // Wait for the child process to finish
        WaitForSingleObject(processInfo.hProcess, INFINITE);

        // Get the exit code of the child process
        DWORD exitCode = 0;
        GetExitCodeProcess(processInfo.hProcess, &exitCode);

        // Close process and thread handles
        CloseHandle(processInfo.hProcess);
        CloseHandle(processInfo.hThread);

        return static_cast<int>(exitCode);
    }

    return -1; // Return -1 if the command execution fails
}
std::string JDManager::executeCommandPiped(const std::string& command)
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
    
    //return "";
    //C:\\Windows\\System32\\cmd.exe
    // Create pipe for capturing the command output
    HANDLE pipeRead, pipeWrite;
    SECURITY_ATTRIBUTES pipeAttributes;
    ZeroMemory(&pipeAttributes, sizeof(SECURITY_ATTRIBUTES));
    pipeAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
    pipeAttributes.bInheritHandle = TRUE;
    pipeAttributes.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&pipeRead, &pipeWrite, &pipeAttributes, 0))
    {
        //std::cerr << "Failed to create pipe." << std::endl;
        JD_CONSOLE_FUNCTION("Failed to create pipe\n");
        return "";
    }

    // Set the read end of the pipe as the standard output and error
    if (!SetHandleInformation(pipeRead, HANDLE_FLAG_INHERIT, 0))
    {
        //std::cerr << "Failed to set pipe handle information." << std::endl;
        JD_CONSOLE_FUNCTION("Failed to set pipe handle information\n");
        CloseHandle(pipeRead);
        CloseHandle(pipeWrite);
        return "";
    }

    // Create a STARTUPINFO structure and set its properties
    STARTUPINFOW startupInfo;
    ZeroMemory(&startupInfo, sizeof(STARTUPINFOW));
    startupInfo.cb = sizeof(STARTUPINFOW);
    startupInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    startupInfo.wShowWindow = SW_HIDE;  // Hide the console window
    startupInfo.hStdOutput = pipeWrite;
    startupInfo.hStdError = pipeWrite;

    // Create a PROCESS_INFORMATION structure
    PROCESS_INFORMATION processInfo;
    ZeroMemory(&processInfo, sizeof(PROCESS_INFORMATION));

    // Convert the command to wide string
    std::wstring wideCommand(command.begin(), command.end());

    // Create the child process
    if (CreateProcessW(NULL, const_cast<LPWSTR>((L"C:\\Windows\\System32\\cmd.exe /c " + wideCommand).c_str()), NULL, NULL, TRUE, 0, NULL, NULL, &startupInfo, &processInfo))
    {
        // Close the write end of the pipe since it's not needed in this process
        CloseHandle(pipeWrite);

        // Read the command output from the pipe
        const int bufferSize = 4096;
        std::vector<char> buffer(bufferSize);

        std::string output;
        DWORD bytesRead;

        while (true)
        {
            if (!ReadFile(pipeRead, buffer.data(), bufferSize - 1, &bytesRead, NULL))
            {
                if (GetLastError() == ERROR_BROKEN_PIPE) // Pipe has been closed
                    break;
                else
                {
                    JD_CONSOLE_FUNCTION("Failed to read from the pipe\n");
                    //std::cerr << "Failed to read from the pipe." << std::endl;
                    CloseHandle(pipeRead);
                    CloseHandle(processInfo.hProcess);
                    CloseHandle(processInfo.hThread);
                    return "";
                }
            }

            if (bytesRead == 0)
                continue;

            buffer[bytesRead] = '\0';
            output += buffer.data();
        }

        // Wait for the child process to finish
        WaitForSingleObject(processInfo.hProcess, INFINITE);

        // Get the exit code of the child process
        DWORD exitCode = 0;
        GetExitCodeProcess(processInfo.hProcess, &exitCode);

        // Close process and thread handles
        CloseHandle(processInfo.hProcess);
        CloseHandle(processInfo.hThread);

        CloseHandle(pipeRead);

        return output;
    }
    else
    {
        //std::cerr << "Failed to execute command." << std::endl;
        JD_CONSOLE_FUNCTION("Failed to execute command\n");
        CloseHandle(pipeRead);
        CloseHandle(pipeWrite);
        return "";
    }
}

void JDManager::compressString(const QString& inputString, QByteArray& compressedData) const
{
    compressedData = qCompress(inputString.toUtf8(), -1);
}
bool JDManager::decompressString(const QByteArray& compressedData, QString& outputString) const
{
    QByteArray decompressedData = qUncompress(compressedData);
    if (decompressedData.size() == 0)
    {
        return false; // Corrupt data 
    }
    outputString = QString::fromUtf8(decompressedData);
    return true;
}

#ifdef JSON_DATABSE_USE_THREADS
JDManager::ThreadWorkObj::ThreadWorkObj(JDManager* manager)
    : m_manager(manager)
{

}
JDManager::ThreadWorkObj::~ThreadWorkObj()
{

}
void JDManager::ThreadWorkObj::setLoadData(const Thread_loadChunkData& data)
{
    m_loadData = data;
    m_workType = ThreadWorkType::load;
}
void JDManager::ThreadWorkObj::setSaveData(const Thread_saveChunkData& data)
{
    m_saveData = data;
    m_workType = ThreadWorkType::save;
}
const JDManager::Thread_loadChunkData& JDManager::ThreadWorkObj::getLoadData() const
{
    return m_loadData;
}
const JDManager::Thread_saveChunkData& JDManager::ThreadWorkObj::getSaveData() const
{
    return m_saveData;
}
void JDManager::ThreadWorkObj::process(int threadIndex)
{
    switch (m_workType)
    {
    case ThreadWorkType::load:
        m_manager->loadObjects_chunked(m_loadData);
        break;
    case ThreadWorkType::save:
        m_manager->saveObjects_chunked(m_saveData);
        break;
    }
}



JDManager::ThreadWorkFindJsonFiles::ThreadWorkFindJsonFiles(JDManager* manager)
    : m_manager(manager)
{

}
void JDManager::ThreadWorkFindJsonFiles::process(int threadIndex)
{
    m_manager->findJsonFilesRecustive_threaded(m_start_dir, m_results, m_finished, m_crashed);
}
#endif

}
