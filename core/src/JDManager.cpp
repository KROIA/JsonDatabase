#include "JDManager.h"
#include "JDObjectInterface.h"
#include <QJsonDocument>
#include <QFile>
#include <QDateTime>
#include <QDebug>
#include <thread>
#include <qcoreapplication.h>
#include <QDirIterator>

#include <filesystem>
#include <algorithm>
#include <execution>
//#include "qwindowdefs_win.h"



#include <windows.h>
#include <time.h>
#include <winsock.h>
#include <Wincon.h>
#include <winuser.h>

namespace JsonDatabase
{

const std::string JDManager::m_jsonFileEnding = ".json";
const std::string JDManager::m_lockFileEnding = ".lck";

const QString JDManager::m_timeFormat    = "hh:mm:ss.zzz";
const QString JDManager::m_dateFormat    = "dd.MM.yyyy";

const QString JDManager::m_tag_sessionID = "sessionID";
const QString JDManager::m_tag_user      = "user";
const QString JDManager::m_tag_date      = "date";
const QString JDManager::m_tag_time      = "time";

JDManager::JDManager(const std::string &databasePath,
                     const std::string &sessionID,
                     const std::string &user)
    :   m_databasePath(databasePath)
    ,   m_sessionID(sessionID)
    ,   m_user(user)
#ifdef JSON_DATABSE_USE_THREADS
    ,   m_threadWorker("JDManager File IO")
    ,   m_threadWorker_fileFinder("JDManager File finder")
#endif
{
#ifdef JSON_DATABSE_USE_THREADS
    setupThreadWorker();
#endif


}
JDManager::JDManager(const JDManager &other)
    :   m_databasePath(other.m_databasePath)
    ,   m_sessionID(other.m_sessionID)
    ,   m_user(other.m_user)
    ,   m_objDefinitions(other.m_objDefinitions)
#ifdef JSON_DATABSE_USE_THREADS
    ,  m_threadWorker("JDManager File IO")
    ,  m_threadWorker_fileFinder("JDManager File finder")
#endif
{
#ifdef JSON_DATABSE_USE_THREADS
    setupThreadWorker();
#endif
}
JDManager::~JDManager()
{
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
    if(m_objDefinitions.find(className) != m_objDefinitions.end())
        return true;
    return false;
}
void JDManager::setDatabasePath(const std::string &path)
{
    m_databasePath = path;
}
const std::string &JDManager::getDatabasePath() const
{
    return m_databasePath;
}

bool JDManager::saveObject(JDObjectInterface *obj) const
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_1)
    if(!makeDatabaseDirs())
    {
        return false;
    }
    return saveObject_internal(obj);
}
bool JDManager::saveObjects() const
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_1)
    if(!makeDatabaseDirs())
    {
        return false;
    }
    bool success = true;
    if (!lockFile("all"))
    {
        JD_CONSOLE_FUNCTION(" Can't lock all objects to save all");
        return false;
    }


    std::vector<ObjectSaverData*> objectsToSave(m_objs.size(), nullptr);
    size_t i = 0;
    for (auto& p : m_objs)
    {
        ObjectSaverData* data = new ObjectSaverData();
        data->obj = p.second;
        data->fileWriteSuccess = false;
        objectsToSave[i++] = data;
    }

#ifdef JSON_DATABSE_USE_THREADS
    saveObjects_threaded(objectsToSave, success);
#else
    Thread_saveChunkData data;
    data.saverData = &objectsToSave;
    data.start = 0;
    data.end = objectsToSave.size();
    data.threadIndex = 0;
    data.threadCount = 1;
    data.success = false;
    data.finished = false;
    saveObjects_chunked(data);
    success = data.success;
#endif
    unlockFile("all");
    
    return success;
}
bool JDManager::saveObjects(const std::vector<JDObjectInterface*> &objList) const
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_1)

    if(!makeDatabaseDirs())
    {
        return false;
    }
    bool success = true;
    if (!lockFile("all"))
    {
        JD_CONSOLE_FUNCTION(" Can't lock all objects to save all");
        return false;
    }

    std::vector<ObjectSaverData*> objectsToSave(objList.size(), nullptr);
    size_t i = 0;
    for (auto& p : objList)
    {
        ObjectSaverData* data = new ObjectSaverData();
        data->obj = p;
        data->fileWriteSuccess = false;
        objectsToSave[i++] = data;
    }

#ifdef JSON_DATABSE_USE_THREADS
    saveObjects_threaded(objectsToSave, success);
#else
    Thread_saveChunkData data;
    data.saverData = &objectsToSave;
    data.start = 0;
    data.end = objectsToSave.size();
    data.threadIndex = 0;
    data.threadCount = 1;
    data.success = false;
    data.finished = false;
    saveObjects_chunked(data);
    success = data.success;
#endif
    unlockFile("all");
    return success;
}

bool JDManager::loadObject(JDObjectInterface *obj)
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_1)
    if(!exists(obj))
        return false;

    std::string filePath = getRelativeFilePath(obj->getObjectID());
    return loadObject_internal(obj, filePath);
}
bool JDManager::loadObjects()
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_1);
    std::vector<ObjectLoaderData*> objectsToLoad;

    std::vector<JDObjectInterface*> noFileFound;
    noFileFound.reserve(m_objs.size());

    std::map<std::string, JDObjectInterface*> cpyTable = m_objs;

    if (!lockFile("all"))
    {
        JD_CONSOLE_FUNCTION(" Can't lock all objects to save all");
        return false;
    }

    QDir dir(m_databasePath.c_str());
    std::vector<std::string> files;
    // Search all files
    {
        JD_PROFILING_BLOCK("Recusrive file search for *.json", COLOR_STAGE_2);
        findJsonFilesRecursive(m_databasePath, files);
        //findJsonFilesRecursive(std::filesystem::path(m_databasePath) , files); // Very slow
    }

    // Prepare data to load
    objectsToLoad.reserve(files.size() + m_objs.size());
    for (size_t i = 0; i < files.size(); ++i)
    {
        std::string id = files[i].substr(files[i].rfind("\\") + 1);
        if (cpyTable.find(id) == cpyTable.end())
        {
            ObjectLoaderData* toLoad = new ObjectLoaderData();
            toLoad->filePath = files[i];
            toLoad->id = id;
            toLoad->obj = nullptr;
            objectsToLoad.push_back(toLoad);
        }
        else
        {
            JDObjectInterface* obj = cpyTable[id.c_str()];
            cpyTable[id.c_str()] = nullptr;
            ObjectLoaderData* toLoad = new ObjectLoaderData();
            toLoad->filePath = files[i];
            toLoad->id = id;
            toLoad->obj = obj;
            objectsToLoad.push_back(toLoad);
        }
    }

    // Check for missing files
    for (auto& p : cpyTable)
    {
        if (p.second)
        {
            noFileFound.push_back(p.second);
        }
    }


    bool success = false;
    
#ifdef JSON_DATABSE_USE_THREADS
    loadObjects_threaded(objectsToLoad, success);
#else
    Thread_loadChunkData data;
    data.loaderData = &objectsToLoad;
    data.start = 0;
    data.end = objectsToLoad.size();
    data.threadIndex = 0;
    data.threadCount = 1;
    data.success = false;
    data.finished = false;
    loadObjects_chunked(data);
    success = data.success;
#endif

    unlockFile("all");
    success &= noFileFound.size() == 0;


    std::vector<JDObjectInterface*> newInstantiated;
    newInstantiated.reserve(objectsToLoad.size());
    for (size_t i = 0; i < objectsToLoad.size(); ++i)
    {
        if (objectsToLoad[i]->newInstanceLoadedSuccess)
        {
            m_objs.insert(std::pair<std::string, JDObjectInterface*>(objectsToLoad[i]->obj->getObjectID(), objectsToLoad[i]->obj));
            newInstantiated.push_back(objectsToLoad[i]->obj);
        }
    }
    onNewObjectsInstantiated(newInstantiated);

    for (size_t i = 0; i < objectsToLoad.size(); ++i)
        delete objectsToLoad[i];
    return success;
}

void JDManager::loadObjects_chunked(Thread_loadChunkData &data) 
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_2)

    data.success = false;
    bool _success = true;
    for (size_t i = data.start; i < data.end; ++i)
    {
        ObjectLoaderData* loading = (*data.loaderData)[i];
        loading->fileReadSuccess = false;
        loading->newInstanceLoadedSuccess = false;
        
        if (loading->obj)
        {
            // Load existing instance
            if (loadObject_internal_noLock(loading->obj, loading->filePath, loading->id))
            {
                loading->fileReadSuccess = true;
            }
            else
                _success = false;
        }
        else
        {
            if (loadObject_internal_noLock(loading->obj, loading->filePath, loading->id))
            {
                loading->fileReadSuccess = true;
                loading->newInstanceLoadedSuccess = true;
            }
            else
                _success = false;
        }
    }
    data.success = _success;
    data.finished = true;
}
void JDManager::loadObjects_threaded(const std::vector<ObjectLoaderData*>& loaderData, bool& success) 
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_2);
#ifdef JSON_DATABSE_USE_THREADS
    if (loaderData.size() == 0)
        return;
    //size_t numThreads = std::thread::hardware_concurrency();
    //if (loaderData.size() < numThreads)
    //    numThreads = loaderData.size();
    //if (numThreads > JSON_DATABSE_MAX_THREAD_COUNT)
    //    numThreads = JSON_DATABSE_MAX_THREAD_COUNT;
    Thread_loadChunkData data;
    data.loaderData = &loaderData;
    data.start = 0;
    data.end = loaderData.size();
    data.threadIndex = 0;
    data.threadCount = 1;
    data.success = false;
    data.finished = false;
    size_t numThreads = m_threadWorker.getThreadCount();
    if (numThreads <= 1)
    {
        loadObjects_chunked(data);
        return;
    }
    int chunk_size = loaderData.size() / numThreads;
    //std::vector<std::thread*> threads(numThreads, nullptr);

    data.threadCount = numThreads;
    //std::vector<Thread_loadChunkData> threadData(numThreads, data);
    size_t start = 0;
    for (size_t i = 0; i < numThreads; ++i) {
        size_t end = (i == numThreads - 1) ? loaderData.size() : start + chunk_size;
        Thread_loadChunkData threadData(data);
        threadData.start = start;
        threadData.end = end;
        threadData.threadIndex = i;

        m_threadJobs[i]->setLoadData(threadData);
       // threads[i] = new std::thread(&JDManager::loadObjects_chunked,this, std::ref(threadData[i]));
        start = end;
    }
    m_threadWorker.signalExecution();
    m_threadWorker.waitForThreads();
    
    /*bool all_threads_joined = false;
    while (!all_threads_joined) {
        // Do some other work here

        
       // QEventLoop::
       // AllEvents = 0x00,
       // ExcludeUserInputEvents = 0x01,
       // ExcludeSocketNotifiers = 0x02,
       // WaitForMoreEvents = 0x04,
       // X11ExcludeTimers = 0x08,
       // EventLoopExec = 0x20,
       // DialogExec = 0x40,
       // ApplicationExec = 0x80,
        QTUpdateEvents();
        std::this_thread::sleep_for(std::chrono::microseconds(1));

        all_threads_joined = true;
        for (int i = 0; i < threads.size(); ++i) {
            if (!threadData[i].finished) {
                all_threads_joined = false;
                break;
            }
        }
    }
    bool allSuccess = true;
    for (int i = 0; i < numThreads; ++i) {
        threads[i]->join();
        allSuccess &= threadData[i].success;
        delete threads[i];
        threads[i] = nullptr;
    }*/
    bool allSuccess = true;
    for (int i = 0; i < numThreads; ++i) {
        allSuccess &= m_threadJobs[i]->getLoadData().success;
    }
    success = allSuccess;
#endif
}

void JDManager::saveObjects_chunked(Thread_saveChunkData& data) const
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_2)

    data.success = false;
    bool _success = true;
    for (size_t i = data.start; i < data.end; ++i)
    {
        ObjectSaverData* saving = (*data.saverData)[i];
        if (!saveObject_internal_noLock(saving->obj))
        {
            saving->fileWriteSuccess = false;
            _success = false;
            JD_CONSOLE_FUNCTION("Can't save object: " << saving->obj->className().c_str() << ":" << saving->obj->getObjectID().c_str());
        }
        else
            saving->fileWriteSuccess = true;
    }
    data.success = _success;
    data.finished = true;
}

void JDManager::saveObjects_threaded(const std::vector<ObjectSaverData*>& saverData, bool& success) const
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_2)

#ifdef JSON_DATABSE_USE_THREADS
    if (saverData.size() == 0)
        return;
    //size_t numThreads = std::thread::hardware_concurrency();
    //if (saverData.size() < numThreads)
    //    numThreads = saverData.size();
    //if (numThreads > JSON_DATABSE_MAX_THREAD_COUNT)
    //    numThreads = JSON_DATABSE_MAX_THREAD_COUNT;
    Thread_saveChunkData data;
    data.saverData = &saverData;
    data.start = 0;
    data.end = saverData.size();
    data.threadIndex = 0;
    data.threadCount = 1;
    data.success = false;
    data.finished = false;
    //if (numThreads <= 1)
    //{
    //
    //    saveObjects_chunked(data);
    //    return;
    //}
    size_t numThreads = m_threadWorker.getThreadCount();
    int chunk_size = saverData.size() / numThreads;
    //std::vector<std::thread*> threads(numThreads, nullptr);

    data.threadCount = numThreads;
    //std::vector<Thread_saveChunkData> threadData(numThreads, data);
    size_t start = 0;
    for (size_t i = 0; i < numThreads; ++i) {
        size_t end = (i == numThreads - 1) ? saverData.size() : start + chunk_size;
        Thread_saveChunkData threadData(data);
        threadData.start = start;
        threadData.end = end;
        threadData.threadIndex = i;

        m_threadJobs[i]->setSaveData(threadData);

        //threads[i] = new std::thread(&JDManager::saveObjects_chunked, this, std::ref(threadData[i]));
        start = end;
    }
    m_threadWorker.signalExecution();
    m_threadWorker.waitForThreads();
   /* bool all_threads_joined = false;
    while (!all_threads_joined) {
        // Do some other work here


        // QEventLoop::
        // AllEvents = 0x00,
        // ExcludeUserInputEvents = 0x01,
        // ExcludeSocketNotifiers = 0x02,
        // WaitForMoreEvents = 0x04,
        // X11ExcludeTimers = 0x08,
        // EventLoopExec = 0x20,
        // DialogExec = 0x40,
        // ApplicationExec = 0x80,
        QTUpdateEvents();
        std::this_thread::sleep_for(std::chrono::microseconds(1));

        all_threads_joined = true;
        for (int i = 0; i < threads.size(); ++i) {
            if (!threadData[i].finished) {
                all_threads_joined = false;
                break;
            }
        }
    }
    bool allSuccess = true;
    for (int i = 0; i < numThreads; ++i) {
        threads[i]->join();
        allSuccess &= threadData[i].success;
        delete threads[i];
        threads[i] = nullptr;
    }*/
    bool allSuccess = true;
    for (int i = 0; i < numThreads; ++i)
    {
        allSuccess &= m_threadJobs[i]->getSaveData().success;
    }
    success = allSuccess;
#endif
}
void JDManager::QTUpdateEvents()
{
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents | 
                                    QEventLoop::ExcludeSocketNotifiers |
                                    QEventLoop::X11ExcludeTimers);
}
void JDManager::clearObjects()
{
    m_objs.clear();
}
bool JDManager::addObject(JDObjectInterface* obj)
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_1)
    if(!obj) return false;
    if(exists(obj)) return false;
    m_objs.insert(std::pair<std::string, JDObjectInterface*>(obj->getObjectID(), obj));
    return true;
}
bool JDManager::addObject(const std::vector<JDObjectInterface*> &objList)
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_1)
    bool success = true;
    for(size_t i=0; i<objList.size(); ++i)
        success &= addObject(objList[i]);
    return success;
}
bool JDManager::removeObject(JDObjectInterface* obj)
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_1)
    if(!obj) return false;
    if(!exists(obj)) return false;
    return removeObject_internal(obj);
}
bool JDManager::removeObject_internal(JDObjectInterface* obj)
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_2);
    m_objs.erase(obj->getObjectID());
    return deleteJsonFile(obj);
}
bool JDManager::removeObjects(const std::vector<JDObjectInterface*> &objList)
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_1)
    bool success = true;
    for(size_t i=0; i<objList.size(); ++i)
        success &= removeObject(objList[i]);
    return success;
}
std::size_t JDManager::getObjectCount() const
{
    return m_objs.size();
}
bool JDManager::exists(JDObjectInterface* obj) const
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_1)
    if(!obj) return false;
    if(m_objs.find(obj->getObjectID()) == m_objs.end())
        return false;
    return true;
}
bool JDManager::exists(const std::string &id) const
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_1)
    if(m_objs.find(id) == m_objs.end())
        return false;
    return true;
}

JDObjectInterface* JDManager::getObject(const std::string &objID) const
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_1)
        if (objID.size() == 0)
            return nullptr;
    auto it = m_objs.find(objID);
    if(it != m_objs.end())
        return m_objs.at(objID.c_str());
    return nullptr;
}
std::vector<JDObjectInterface*> JDManager::getObjects() const
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_1)
    std::vector<JDObjectInterface*> list;
    list.reserve(m_objs.size());
    for(auto &p : m_objs)
        list.push_back(p.second);
    return list;
}
void JDManager::onNewObjectsInstantiated(const std::vector<JDObjectInterface*>& newObjects)
{

}

bool JDManager::writeJsonFile(const QJsonObject &obj, const std::string &relativePath) const
{
    return writeJsonFile(obj, relativePath, m_jsonFileEnding);
}
bool JDManager::writeJsonFile(const QJsonObject &obj, const std::string &relativePath, const std::string &fileEnding) const
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_2)
    QJsonDocument document;
    document.setObject(obj);
    QByteArray bytes = document.toJson( QJsonDocument::Indented );
    QFile file((m_databasePath+"\\"+relativePath+fileEnding).c_str());
    if( file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ))
    {
        QTextStream iStream( &file );
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        iStream.setCodec( "utf-8" );
#else
        iStream.setEncoding(QStringConverter::Encoding::Utf8);
#endif
        iStream << bytes;
        file.close();
        return true;
    }
    return false;
}
bool JDManager::readJsonFile(QJsonObject &obj, const std::string &relativePath) const
{
    return readJsonFile(obj, relativePath, m_jsonFileEnding);
}
bool JDManager::readJsonFile(QJsonObject &obj, const std::string &relativePath, const std::string &fileEnding) const
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_2)
    QFile file((m_databasePath+"\\"+relativePath+fileEnding).c_str());
    if( file.open( QIODevice::ReadOnly ) )
    {
        QByteArray bytes = file.readAll();
        file.close();

        QJsonParseError jsonError;
        QJsonDocument document = QJsonDocument::fromJson( bytes, &jsonError );
        if( jsonError.error != QJsonParseError::NoError )
        {
            JD_CONSOLE_FUNCTION("Can't read JsonFile: " << jsonError.errorString().toStdString().c_str());
            return false;
        }
        if( document.isObject() )
        {
            obj = document.object();
            return true;
        }
    }
    JD_CONSOLE_FUNCTION("Can't open File: "<<(m_databasePath+"\\"+relativePath+fileEnding).c_str());
    return false;
}
bool JDManager::lockFile(const std::string &relativePath) const
{
    JD_PROFILING_FUNCTION_C(profiler::colors::Red)
    std::string abspath = m_databasePath+"\\"+ relativePath +m_lockFileEnding;
    QDateTime currentDateTime = QDateTime::currentDateTime();
    size_t tryCount = 0;
    while(QFile::exists(abspath.c_str()))
    {
        QTUpdateEvents();
        QJsonObject data;
        if(!readJsonFile(data, relativePath, m_lockFileEnding))
            return false;
        std::string sessionID;
        std::string user;
        QDate date;
        QTime time;
        bool success = getJsonValue(data, sessionID, m_tag_sessionID);
        success &= getJsonValue(data, user, m_tag_user);
        success &= getJsonValue(data, date, m_tag_date);
        success &= getJsonValue(data, time, m_tag_time);

        if(!success)
        {
            JD_CONSOLE_FUNCTION("Corrupted lock file: "<<abspath.c_str());
            return false; // can't read Lock file
        }

        QDateTime dateTime(date, time);
        if(dateTime.secsTo(currentDateTime) > 5)
        {
            // LockTimeOut
            if(QFile::remove(abspath.c_str()))
            {
                JD_CONSOLE_FUNCTION("Lockfile: "<< abspath.c_str()
                                 << " timed out and gets removed."
                                 << " Was locked by user: "<<user.c_str()
                                 << " Session ID: "<<sessionID.c_str());
            }
            if(QFile::exists(abspath.c_str()))
            {
                JD_CONSOLE_FUNCTION("Can't remove lock file: "<<abspath.c_str());
                return false;
            }
        }
        else
        {
            using namespace std::literals;
            ++tryCount;
            if(tryCount > 100)
            {
                JD_CONSOLE_FUNCTION("Timeout for waiting until lock gets released: "<<abspath.c_str());
                return false;
            }
            JD_CONSOLE_FUNCTION("["<<tryCount<<"] Waiting for lock release: "<<abspath.c_str());
            std::this_thread::sleep_for(100ms);
        }
    }

    QJsonObject data;

    data[m_tag_sessionID] = m_sessionID.c_str();
    data[m_tag_user]      = m_user.c_str();
    data[m_tag_date]      = currentDateTime.toString(m_dateFormat);
    data[m_tag_time]      = currentDateTime.toString(m_timeFormat);

    return writeJsonFile(data, relativePath.c_str(), m_lockFileEnding);
}
bool JDManager::unlockFile(const std::string &relativePath) const
{
    JD_PROFILING_FUNCTION_C(profiler::colors::Red)
    std::string path = relativePath;
    std::string abspath = m_databasePath+"\\"+path+m_lockFileEnding;
    if(!QFile::exists(abspath.c_str()))
        return true;


    QJsonObject data;
    if(!readJsonFile(data, path, m_lockFileEnding))
        return false;
    std::string sessionID;
    std::string user;
    QDate date;
    QTime time;
    bool success = getJsonValue(data, sessionID, m_tag_sessionID);
    success &= getJsonValue(data, user, m_tag_user);
    success &= getJsonValue(data, date, m_tag_date);
    success &= getJsonValue(data, time, m_tag_time);

    if(!success)
    {
        JD_CONSOLE_FUNCTION("Corrupted lock file: "<<abspath.c_str());
        return false; // can't read Lock file
    }
    if(sessionID != m_sessionID)
    {
        JD_CONSOLE_FUNCTION("Lock belongs not to this session: "<<m_sessionID.c_str()
                        << " Lock session: "<<sessionID.c_str());
        return false;
    }
    if(user != m_user)
    {
        JD_CONSOLE_FUNCTION("Lock belongs not to this user: "<<m_user.c_str()
                        << " Lock user: "<<user.c_str());
        return false;
    }

    QFile::remove(abspath.c_str());

    if(QFile::exists(abspath.c_str()))
    {
        JD_CONSOLE_FUNCTION("Can't remove lock file: "<<abspath.c_str());
        return false;
    }
    return true;
}

std::string JDManager::getRelativeFilePath(const std::string &objID) const
{
    QDir dir(m_databasePath.c_str());
    std::vector<std::string> files;
    findJsonFilesRecursive(m_databasePath, files);
    std::string match;
    for(size_t i=0; i<files.size(); ++i)
    {
        if(files[i].find(objID) != std::string::npos)
            match = files[i];
    }
    return match;
}
void JDManager::getObjectFileList(QDir dir, const std::string &relativePath,
                                  std::vector<std::string> &list) const
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_2);
    QDir fileFilter(dir);
    QDir dirFilter(dir);
    fileFilter.setNameFilters(QStringList(("*"+m_jsonFileEnding).c_str()));
    fileFilter.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    dirFilter.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    getObjectFileList_internal(dirFilter, fileFilter, relativePath, list);
}
void JDManager::getObjectFileList_internal(const QDir& dirFilter, const QDir& fileFilter, const std::string& relativePath, std::vector<std::string>& list) const
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_2)

    QStringList fileList;
    {
        JD_PROFILING_BLOCK("fileFilter.entryList()", COLOR_STAGE_3)
        fileList = fileFilter.entryList();
    }
    list.reserve(list.size() + fileList.count());
    for (int i = 0; i < fileList.count(); i++)
    {
        QString fileName = fileList[i];
        if (fileName.contains(m_jsonFileEnding.c_str()))
            fileName = fileName.mid(0, fileName.indexOf(m_jsonFileEnding.c_str()));
        list.push_back(relativePath + "\\" + fileName.toStdString());
    }

    QStringList dirList;
    {
        JD_PROFILING_BLOCK("dirFilter.entryList()", COLOR_STAGE_3)
        dirList = dirFilter.entryList();
    }
    for (int i = 0; i < dirList.size(); ++i)
    {
        QString newPath = QString("%1/%2").arg(dirFilter.absolutePath()).arg(dirList.at(i));
        std::string relPath = relativePath;
        if (relativePath.size() > 0)
            relPath += "/";
        relPath += newPath.mid(newPath.lastIndexOf("/") + 1).toStdString();

        QDir fileFilter1(fileFilter);
        QDir dirFilter1(dirFilter);
        fileFilter1.setPath(newPath);
        dirFilter1.setPath(newPath);
        
        getObjectFileList_internal(dirFilter1, fileFilter1, relPath, list);
    }
}
void JDManager::findJsonFilesRecursive(const std::filesystem::path& start_dir, std::vector<std::string>& results) const
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_3)
//#define OPTIMIZED_SEARCH

#ifdef OPTIMIZED_SEARCH
    // Check if the starting directory exists and is a directory
    if (!std::filesystem::exists(start_dir) || !std::filesystem::is_directory(start_dir))
        return;

    // Count the number of files with the ".json" extension
    size_t count = 0;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(start_dir))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".json")
            ++count;
    }

    // Preallocate the results vector with the expected size
    results.resize(count);

    // Use a parallel for loop to search for files
    std::for_each(std::execution::par, std::filesystem::recursive_directory_iterator(start_dir),
        [&](const std::filesystem::directory_entry& entry) {
            if (entry.is_regular_file() && entry.path().extension() == ".json")
            {
                // Calculate the index of the path in the results vector
                size_t index = std::distance(std::filesystem::recursive_directory_iterator(start_dir), std::filesystem::directory_iterator(entry.path())) - 1;

                // Set the path in the results vector
                results[index] = std::filesystem::relative(entry.path(), start_dir).string();
            }
        }
    );
#else
    // CMD: dir /b /aa | findstr .json

    // Check if the starting directory exists and is a directory
    if (!std::filesystem::exists(start_dir) || !std::filesystem::is_directory(start_dir))
        return;

    // Iterate over all files and directories in the starting directory
    for (const auto& entry : std::filesystem::recursive_directory_iterator(start_dir))
    {
        JD_PROFILING_BLOCK("dir entry iteration", COLOR_STAGE_3)
        // Check if the current entry is a regular file and has the ".json" file extension
        if (entry.is_regular_file() && entry.path().extension() == ".json")
        {
            // Add the relative path of the file to the results vector
            results.push_back(std::filesystem::relative(entry.path(), start_dir).string());
        }
    }
#endif
}
void JDManager::findJsonFilesRecursive(const std::string& start_dir, std::vector<std::string>& results) const
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_3);
#ifdef JSON_DATABSE_USE_THREADS

    m_threadJsonFinderJob->m_start_dir = start_dir;
    m_threadJsonFinderJob->m_results.clear();
    m_threadJsonFinderJob->m_finished = false;
    m_threadJsonFinderJob->m_crashed = false;

    m_threadWorker_fileFinder.signalExecution();
    m_threadWorker_fileFinder.waitForThreads();
    results = m_threadJsonFinderJob->m_results;
    /*
    while (!threadFinished && !threadCrashed) {
        QTUpdateEvents();
        std::this_thread::sleep_for(std::chrono::microseconds(1));        
    }
    if (threadCrashed)
    {
        int error = GetLastError();
        error = error;
    }
    bool allSuccess = true;
    thread.join();*/
#else
    results.reserve(1000);
    std::vector<std::string> subDirs;
    getDirsRecursive_internal(start_dir, "", subDirs);

    for (size_t i = 0; i < subDirs.size(); ++i)
    {
        getFileList(start_dir+"\\"+subDirs[i], subDirs[i], m_jsonFileEnding, results);
    }
#endif
}
#ifdef JSON_DATABSE_USE_THREADS
void JDManager::findJsonFilesRecustive_threaded(const std::string& start_dir,
    std::vector<std::string>& results,
    bool& finished,
    bool& crashed)
{
    class CrashDetector
    {
    public:
        CrashDetector(bool& result) {
            m_signal = &result;
        }
        void normalExit()
        {
            m_signal = nullptr;
        }
        ~CrashDetector()
        {
            if (m_signal)
                *m_signal = true;
        }
    private:
        bool* m_signal = nullptr;
    };
    CrashDetector crashDetector(crashed);
    results.reserve(1000);
    std::vector<std::string> subDirs;
    getDirsRecursive_internal(start_dir, "", subDirs);

    for (size_t i = 0; i < subDirs.size(); ++i)
    {
        getFileList(start_dir + "\\" + subDirs[i], subDirs[i], m_jsonFileEnding, results);
    }
    finished = true;
    crashDetector.normalExit();
}
#endif
void JDManager::getDirsRecursive_internal(const std::string& dir, const std::string& relativeRoot, std::vector<std::string>& subDirs)
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_4);
    std::vector<std::string> dirNames; 

    getDirectories(dir, dirNames);
    for (size_t i = 0; i < dirNames.size(); ++i)
    {
        std::string subRoot = relativeRoot;
        if (subRoot.size() > 0)
            subRoot += "\\";
        subRoot += dirNames[i];

        subDirs.push_back(subRoot);
    }
    for (size_t i = 0; i < dirNames.size(); ++i)
    {
        std::string subRoot = relativeRoot;
        if (subRoot.size() > 0)
            subRoot += "\\";
        subRoot += dirNames[i];
        getDirsRecursive_internal(dir+"\\"+dirNames[i], subRoot, subDirs);
    }


    
}
void JDManager::getDirectories(const std::string& path, std::vector<std::string>& dirNames)
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_5);
#ifdef JSON_DATABASE_USE_CMD_FILE_SEARCH
    std::string command = "cd /D \"" + path + "\" & dir /b /ad";
    //executeCommand("echo Hello");
    std::string result = executeCommandPiped(command);
    //std::string result = systemExec(command);

    splitString(result, dirNames);
#else
    QDirIterator directories(path.c_str(), QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);

    while (directories.hasNext()) {
        directories.next();
        dirNames.push_back(directories.fileName().toStdString());
    }
#endif
}
void JDManager::getFileList(const std::string& path, const std::string& relativeRoot, const std::string& fileEndingFilter, std::vector<std::string>& fullFilePathList)
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_5);
#ifdef JSON_DATABASE_USE_CMD_FILE_SEARCH
    std::string command = "cd /D \"" + path + "\" & dir /b /aa";// | findstr "+fileEndingFilter;
    std::string result = executeCommandPiped(command);
    //std::string result = systemExec(command);

    std::vector<std::string> files;
    files.reserve(1000);

    splitString(result, files);
    fullFilePathList.reserve(fullFilePathList.size() + files.size());
    for (size_t i = 0; i < files.size(); ++i)
    {
        if (files[i].find(fileEndingFilter) == std::string::npos)
            continue;
        std::string relativeFilePath = relativeRoot;
        if (relativeFilePath.size() > 0)
            relativeFilePath += "\\";
        std::string fileName = files[i];
        if (fileName.find(".") != std::string::npos)
            fileName = fileName.substr(0, fileName.rfind("."));
        relativeFilePath += fileName;
        fullFilePathList.push_back(relativeFilePath);
    }
#else
    QDirIterator directories(path.c_str(), QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    
    while (directories.hasNext()) {
        directories.next();

        std::string relativeFilePath = relativeRoot;
        if (relativeFilePath.size() > 0)
            relativeFilePath += "\\";
        std::string fileName = directories.fileName().toStdString();
        if (fileName.find(m_jsonFileEnding) != std::string::npos)
        {
            fileName = fileName.substr(0, fileName.rfind(m_jsonFileEnding));
            relativeFilePath += fileName;
            fullFilePathList.push_back(relativeFilePath);
        }
    }
#endif
}

std::wstring convertToWideString(const std::string& str) {
    int num_chars = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.size(), NULL, 0);
    std::vector<wchar_t> wide_str(num_chars);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.size(), &wide_str[0], num_chars);
    return std::wstring(wide_str.begin(), wide_str.end());
}
/*std::string JDManager::systemExec(const std::string& command)
{
    char buffer[1024];

    std::string result = "";
    FILE* pipe = _popen(command.c_str(), "r");
    ShowWindow(GetConsoleWindow(), SW_HIDE);
    if (!pipe) throw std::runtime_error("popen() failed!");
    try {
        while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
            result += buffer;
        }
    }
    catch (...) {
        _pclose(pipe);
        throw;
    }
    _pclose(pipe);
    return result;
}*/
void JDManager::splitString(const std::string& str, std::vector<std::string>& container, const std::string& delimiter)
{
    size_t start = 0;
    size_t end = str.find(delimiter);
    while (end != std::string::npos) {
        std::string substr = str.substr(start, end - start);
        for (size_t i = 0; i < substr.size(); ++i)
        {
            if (substr[i] == '\r')
            {
                substr = substr.substr(0, i);
                break;
            }
        }
        container.push_back(substr);
        start = end + delimiter.length();
        end = str.find(delimiter, start);
    }
    std::string substr = str.substr(start, end);
    for (size_t i = 0; i < substr.size(); ++i)
    {
        if (substr[i] == '\r')
        {
            substr[i] = 0;
            break;
        }
    }
    if (substr.empty())
        return;
    container.push_back(substr);
}

bool JDManager::saveObject_internal(JDObjectInterface *obj) const
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_4)
    if(!obj) return false;

    std::string relativePath = getRelativeFilePath(obj);
    if(lockFile(relativePath))
    {
        bool res = saveObject_internal_noLock(obj);
        unlockFile(relativePath);
        return res;
    }
    return false;
}
bool JDManager::saveObject_internal_noLock(JDObjectInterface* obj, const std::string* relativeFilePath) const
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_4)
        if (!obj) return false;

    QJsonObject data;
    if (!obj->saveInternal(data))
        return false;

    std::string relativePath;
    if (!relativeFilePath)
    {
        relativePath = getRelativeFilePath(obj);
        relativeFilePath = &relativePath;
    }
    

    return writeJsonFile(data, *relativeFilePath);
}
bool JDManager::loadObject_internal(JDObjectInterface *&objOut, std::string relativePath)
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_4)
    bool overrideMode = objOut;
    if(relativePath.find(m_jsonFileEnding) != std::string::npos)
        relativePath = relativePath.substr(0, relativePath.rfind(m_jsonFileEnding));
    std::string pathNoFileEnding = m_databasePath+"\\"+relativePath;
    bool res = false; 
    if (lockFile(relativePath))
    {
        std::string id = relativePath;
        size_t index = relativePath.rfind("\\");
        if (index != std::string::npos)
            id = relativePath.substr(index, relativePath.size());
        res = loadObject_internal_noLock(objOut, relativePath, id);
        unlockFile(relativePath);
    }
    return res;
}
bool JDManager::loadObject_internal_noLock(JDObjectInterface*& objOut, std::string relativePath, const std::string& uniqueID)
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_3)
    bool overrideMode = objOut;
    if (relativePath.find(m_jsonFileEnding) != std::string::npos)
        relativePath = relativePath.substr(0, relativePath.rfind(m_jsonFileEnding));
    std::string pathNoFileEnding = m_databasePath + "\\" + relativePath;
    std::string abspath = m_databasePath + "\\" + relativePath + m_jsonFileEnding;

    QJsonObject obj;
    bool res = readJsonFile(obj, relativePath);

    if (!res)
        return false;

    std::string className;
    if (getJsonValue(obj, className, JDObjectInterface::m_tag_className))
    {
        if (overrideMode)
        {
            std::string name = objOut->className();
            if (className != name)
            {
                JD_CONSOLE_FUNCTION("Loaded object of type: " << className.c_str() << " can't be "
                                    "converted to type: " << objOut->className().c_str());
                return false;
            }
            return objOut->loadInternal(obj);
        }
        else if (isInObjectDefinition(className))
        {

            JDObjectInterface* templateObj = m_objDefinitions[className];
            objOut = templateObj->clone(obj, uniqueID);

            return true;
        }
        else
        {
            JD_CONSOLE_FUNCTION("Objecttype: " << className.c_str() << " is not known by this database. "
                                "Call: JDManager::addObjectDefinition<" << className.c_str() << ">(); first");
        }
    }
    else
    {
        JD_CONSOLE_FUNCTION("Can't read class name of the loaded object file: " << abspath.c_str());
    }
    return false;
}
bool JDManager::makeDatabaseDirs() const
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_2)
    bool success = true;
    for(auto &p : m_objDefinitions)
    {
        std::string className = p.second->className();
        std::string path = m_databasePath+"\\"+className;
        QDir dir(path.c_str());
        QTUpdateEvents();
        if (!dir.exists())
        {
            QDir d;
            d.mkpath(path.c_str());
        }
            

        bool exists = dir.exists();
        if(!exists)
        {
            JD_CONSOLE_FUNCTION("Can't create database folder: "<<path.c_str());
        }
        success &= exists;
    }
    return success;
}
bool JDManager::deleteJsonFile(JDObjectInterface *obj) const
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_2)
    std::string relativePath = getRelativeFilePath(obj);
    if(relativePath.size() == 0)
        return false;
    std::string absPath = m_databasePath+"\\"+relativePath+m_jsonFileEnding;
    if(QFile::exists(absPath.c_str()))
    {
        QFile::remove(absPath.c_str());
        if(!QFile::exists(absPath.c_str()))
            return true;
    }
    else
        return true;
    return false;
}
bool JDManager::deleteDir(const std::string& dir) const
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_2)
    int ret = executeCommand("rd /s /q \"" + dir+"\"");
    QDir folder(dir.c_str());
    if (folder.exists())
    {
        JD_CONSOLE_FUNCTION(" Folder could not be deleted: "+dir);
        return false;
    }
    return true;
}
std::string JDManager::getFolderName(JDObjectInterface *obj) const
{
    if(obj)
        return obj->className();
    JD_CONSOLE_FUNCTION(" obj is nullptr");
    return "";
}
std::string JDManager::getFileName(JDObjectInterface *obj) const
{
    if(obj)
        return obj->getObjectID();
    JD_CONSOLE_FUNCTION(" obj is nullptr");
    return "";
}
std::string JDManager::getRelativeFilePath(JDObjectInterface *obj) const
{
    std::string folder = getFolderName(obj);
    std::string name = getFileName(obj);
    if(folder.size() == 0 || name.size() == 0)
    {
        JD_CONSOLE_FUNCTION(" path is wrong");
        return "";
    }
    return folder + "\\" + name;
}

bool JDManager::getJsonValue(const QJsonObject &obj, QVariant &value, const QString &key) const
{
    if(obj.contains(key))
    {
        value = obj[key];
        return true;
    }
    return false;
}
bool JDManager::getJsonValue(const QJsonObject &obj, QTime &value, const QString &key) const
{
    if(obj.contains(key))
    {
        value = QTime::fromString(obj[key].toString(), m_timeFormat);
        return true;
    }
    return false;
}
bool JDManager::getJsonValue(const QJsonObject &obj, QDate &value, const QString &key) const
{
    if(obj.contains(key))
    {
        value = QDate::fromString(obj[key].toString(), m_dateFormat);
        return true;
    }
    return false;
}
bool JDManager::getJsonValue(const QJsonObject &obj, QString &value, const QString &key) const
{
    if(obj.contains(key))
    {
        value = obj[key].toString();
        return true;
    }
    return false;
}
bool JDManager::getJsonValue(const QJsonObject &obj, std::string &value, const QString &key) const
{
    if(obj.contains(key))
    {
        value = obj[key].toString().toStdString();
        return true;
    }
    return false;
}
bool JDManager::getJsonValue(const QJsonObject &obj, int &value, const QString &key) const
{
    if(obj.contains(key))
    {
        value = obj[key].toInt(value);
        return true;
    }
    return false;
}
int JDManager::executeCommand(const std::string& command)
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_1)
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
    JD_PROFILING_FUNCTION(COLOR_STAGE_1);
    
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
        JD_CONSOLE_FUNCTION("Failed to create pipe");
        return "";
    }

    // Set the read end of the pipe as the standard output and error
    if (!SetHandleInformation(pipeRead, HANDLE_FLAG_INHERIT, 0))
    {
        //std::cerr << "Failed to set pipe handle information." << std::endl;
        JD_CONSOLE_FUNCTION("Failed to set pipe handle information");
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
                    JD_CONSOLE_FUNCTION("Failed to read from the pipe");
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
        JD_CONSOLE_FUNCTION("Failed to execute command");
        CloseHandle(pipeRead);
        CloseHandle(pipeWrite);
        return "";
    }
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
