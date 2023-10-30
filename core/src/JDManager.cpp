#include "JDManager.h"
#include "JDObjectInterface.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QDateTime>
#include <QDebug>
#include <thread>
#include <qcoreapplication.h>
#include <QDirIterator>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif


#include <filesystem>
#include <algorithm>
#include <execution>



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

size_t JDManager::s_instanceCounter = 0;

JDManager::JDManager(const std::string &databasePath,
                     const std::string& databaseName,
                     const std::string &sessionID,
                     const std::string &user)
    :   m_databasePath(databasePath)
    ,   m_databaseName(databaseName)
    ,   m_sessionID(sessionID)
    ,   m_user(user)
    ,   m_fileLock(nullptr)
#ifdef JSON_DATABSE_USE_THREADS
    ,   m_threadWorker("JDManager File IO")
    ,   m_threadWorker_fileFinder("JDManager File finder")
#endif
{
    ++s_instanceCounter;
#ifdef JD_PROFILING
    if (s_instanceCounter == 1)
    {
        EASY_PROFILER_ENABLE;
    }
#endif
#ifdef JSON_DATABSE_USE_THREADS
    setupThreadWorker();
#endif


}
JDManager::JDManager(const JDManager &other)
    :   m_databasePath(other.m_databasePath)
    ,   m_databaseName(other.m_databaseName)
    ,   m_sessionID(other.m_sessionID)
    ,   m_user(other.m_user)
    ,   m_fileLock(nullptr)
    ,   m_objDefinitions(other.m_objDefinitions)
#ifdef JSON_DATABSE_USE_THREADS
    ,  m_threadWorker("JDManager File IO")
    ,  m_threadWorker_fileFinder("JDManager File finder")
#endif
{
    ++s_instanceCounter;
#ifdef JD_PROFILING
    if (s_instanceCounter == 1)
    {
        EASY_PROFILER_ENABLE;
    }
#endif
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
bool JDManager::isInObjectDefinition(const std::string &className) const
{
    if(m_objDefinitions.find(className) != m_objDefinitions.end())
        return true;
    return false;
}
void JDManager::setDatabaseName(const std::string& name)
{
    m_databaseName = name;
}
const std::string& JDManager::getDatabaseName() const
{
    return m_databaseName;
}
void JDManager::setDatabasePath(const std::string &path)
{
    m_databasePath = path;
}
const std::string &JDManager::getDatabasePath() const
{
    return m_databasePath;
}

bool JDManager::saveObject(JDObjectInterface* obj) const
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_1);

    if (!obj)
        return false;
    bool success = true;
    std::string ID = obj->getObjectID();
    std::vector<QJsonObject> jsons;

    QJsonObject data;
    success &= obj->saveInternal(data);


    if (!lockFile(m_databaseName))
    {
        JD_CONSOLE_FUNCTION(" Can't lock database");
        return false;
    }

    success &= readJsonFile(getDatabaseFilePath(), jsons);
    size_t index = getJsonIndexByID(jsons, ID);
    
    if (index == std::string::npos)
    {
        jsons.push_back(data);
    }
    else
    {
        jsons[index] = data;
    }

    // Save the serialized objects
    success &= writeJsonFile(jsons, getDatabaseFilePath());
    unlockFile(m_databaseName);
    return true;
}
bool JDManager::saveObjects() const
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_1);
    bool success = true;
    std::vector<JDObjectInterface*> objs(m_objs.size(), nullptr);
    size_t i = 0;
    for (auto& p : m_objs)
    {
        objs[i++] = p.second;
    }

    success &= saveObjects(objs);   
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

    std::vector<QJsonObject> jsonData;
    success &= getJsonArray(objList, jsonData);

    if (!lockFile(m_databaseName))
    {
        JD_CONSOLE_FUNCTION(" Can't lock database");
        return false;
    }

    // Save the serialized objects
    success &= writeJsonFile(jsonData, getDatabaseFilePath());

    unlockFile(m_databaseName);
    return success;
}

bool JDManager::loadObject(JDObjectInterface* obj)
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_1)
        if (!exists(obj))
            return false;

    bool success = true;
    std::string ID = obj->getObjectID();

    if (!lockFile(m_databaseName))
    {
        JD_CONSOLE_FUNCTION(" Can't lock database");
        return false;
    }

    std::vector<QJsonObject> jsons;
    success &= readJsonFile(getDatabaseFilePath(), jsons);

    unlockFile(m_databaseName);


    size_t index = getJsonIndexByID(jsons, ID);
    if (index == std::string::npos)
    {
        JD_CONSOLE_FUNCTION("Object with ID: " << ID << " not found");
        return false;
    }

    const QJsonObject &objData = jsons[index];
    success &= deserializeJson(objData, obj);

    return success;
}
bool JDManager::loadObjects()
{
    bool success = true;
    JD_PROFILING_FUNCTION(COLOR_STAGE_1);

    if (!lockFile(m_databaseName))
    {
        JD_CONSOLE_FUNCTION(" Can't lock database");
        return false;
    }

    std::vector<QJsonObject> jsons;
    success &= readJsonFile(getDatabaseFilePath(), jsons);

    unlockFile(m_databaseName);

    struct Pair
    {
        JDObjectInterface* obj;
        QJsonObject json;
    };

    std::vector< Pair> pairs;
    for(size_t i=0; i<jsons.size(); ++i)
    {
        std::string ID;
        if (!getJsonValue(jsons[i], ID, JDObjectInterface::m_tag_objID))
        {
            JD_CONSOLE_FUNCTION("Object with no ID found: "<<QJsonValue(jsons[i]).toString().toStdString());
            success = false;
        }
        Pair p;
        p.obj = getObject(ID);
        p.json = jsons[i];
        pairs.push_back(p);
    }
    if (!success)
        return false;

    for (auto &pair : pairs)
    {
        bool newObj = !pair.obj;
        success &= deserializeJson(pair.json, pair.obj);

        // Add the new generated object to the database
        if (newObj)
        {
            addObject(pair.obj);
        }
    }

    return success;
}

void JDManager::QTUpdateEvents()
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_1);
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
    m_objs.erase(obj->getObjectID());
    return true;
}
/*bool JDManager::removeObject_internal(JDObjectInterface* obj)
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_2);
    m_objs.erase(obj->getObjectID());
    return deleteJsonFile(obj);
}*/
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

void JDManager::saveProfilerFile()
{
#ifdef JD_PROFILING
    profiler::dumpBlocksToFile("JDProfile.prof");
#endif
}
/*void JDManager::onNewObjectsInstantiated(const std::vector<JDObjectInterface*>& newObjects)
{

}*/

std::string JDManager::getDatabaseFilePath() const
{
    return  m_databasePath + "//" + m_databaseName + ".json";
}

bool JDManager::getJsonArray(const std::vector<JDObjectInterface*>& objs, std::vector<QJsonObject>& jsonOut) const
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_2);
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
    JD_PROFILING_FUNCTION(COLOR_STAGE_4);
    if (!obj) return false;

    QJsonObject data;
    if (!obj->saveInternal(data))
        return false;

    return serializeJson(data, serializedOut);
}
bool JDManager::serializeJson(const QJsonObject& obj, std::string& serializedOut) const
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_2);
    QJsonDocument document;
    document.setObject(obj);
    QByteArray bytes = document.toJson(QJsonDocument::Indented);

    serializedOut = bytes.constData();
    return true;
}

bool JDManager::deserializeJson(const QJsonObject& json, JDObjectInterface*& objOut) const
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_2);
    if (!objOut)
    {
        JDObjectInterface* clone = getObjectDefinition(json);
        if (!clone)
        {
            std::string className;
            getJsonValue(json, className, JDObjectInterface::m_tag_className);

            JD_CONSOLE_FUNCTION("Objecttype: " << className.c_str() << " is not known by this database. "
                "Call: JDManager::addObjectDefinition<" << className.c_str() << ">(); first");
            return false;
        }

        std::string ID;
        getJsonValue(json, ID, JDObjectInterface::m_tag_objID);

        objOut = clone->clone(json, ID);
    }
    else if (!objOut->loadInternal(json))
    {
        JD_CONSOLE_FUNCTION("Can't load data in object: " << objOut->getObjectID() << " classType: " << objOut->className());
        return false;
    }
    return true;
}

void JDManager::getJsonFileContent(const std::vector<QJsonObject>& jsons, std::string& fileContentOut) const
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_2);
    QJsonArray jsonArray;

    // Convert QJsonObject instances to QJsonValue and add them to QJsonArray
    for (const auto& jsonObject : jsons) {
        jsonArray.append(QJsonValue(jsonObject));
    }

    QJsonDocument jsonDocument(jsonArray);

    fileContentOut = jsonDocument.toJson().toStdString();
}
bool JDManager::writeJsonFile(const std::vector<QJsonObject>& jsons, const std::string& outputFile) const 
{
    JDFILE_IO_PROFILING_FUNCTION(COLOR_STAGE_5);
    QJsonArray jsonArray;

    // Convert QJsonObject instances to QJsonValue and add them to QJsonArray
    for (const auto& jsonObject : jsons) {
        jsonArray.append(QJsonValue(jsonObject));
    }

    QJsonDocument jsonDocument(jsonArray);

    // Convert QJsonDocument to a QByteArray for writing to a file
    QByteArray data = jsonDocument.toJson();

    // Open the file for writing
    QFile file(outputFile.c_str());
    if (file.open(QIODevice::WriteOnly)) {
        // Write the JSON data to the file
        file.write(data);
        file.close();
        //JD_CONSOLE_FUNCTION("JSON objects saved to " << outputFile);
        return true;
    }
    else {
        JD_CONSOLE_FUNCTION("Error: Could not open file " << outputFile << " for writing.");
    }
    return false;
}
bool JDManager::readJsonFile(const std::string& inputFile, std::vector<QJsonObject>& jsonsOut) const
{
    JDFILE_IO_PROFILING_FUNCTION(COLOR_STAGE_5);
    // Open the file for reading
    QFile file(inputFile.c_str());

    if (file.open(QIODevice::ReadOnly)) {
        // Read all data from the file
        QByteArray data = file.readAll();

        // Parse the JSON data
        QJsonDocument jsonDocument = QJsonDocument::fromJson(data);

        // Check if the JSON document is an array
        if (jsonDocument.isArray()) {
            QJsonArray jsonArray = jsonDocument.array();

            // Iterate through the array and add QJsonObjects to the vector
            for (const auto& jsonValue : jsonArray) {
                if (jsonValue.isObject()) {
                    jsonsOut.push_back(jsonValue.toObject());
                }
            }
        }
        else {
            JD_CONSOLE_FUNCTION("Error: JSON document from file: " << inputFile << " is not an array.");
            file.close();
            return false;
        }

        file.close();
    }
    else {
        JD_CONSOLE_FUNCTION("Error: Could not open file" << inputFile << "for reading.");
        return false;
    }
    return true;
}

bool JDManager::writeJsonFile(const QJsonObject& obj, const std::string& relativePath) const
{
    return writeJsonFile(obj, relativePath, m_jsonFileEnding);
}
bool JDManager::writeJsonFile(const QJsonObject &obj, const std::string &relativePath, const std::string &fileEnding) const
{
    JDFILE_IO_PROFILING_FUNCTION(COLOR_STAGE_5)
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
    JDFILE_IO_PROFILING_FUNCTION(COLOR_STAGE_5)
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
bool JDManager::lockFile(const std::string &relativePath, unsigned int timeoutMillis) const
{
    JD_PROFILING_FUNCTION_C(profiler::colors::Red);

    if (m_fileLock)
    {
        JD_CONSOLE_FUNCTION("Lock already aquired");
        return false;
    }
    

    std::string abspath = m_databasePath+"\\"+ relativePath +m_lockFileEnding;

    m_fileLock = new FileLock(abspath);


    QDateTime currentDateTime = QDateTime::currentDateTime();
    size_t tryCount = 0;
    while(!m_fileLock->lock())
    {
        JD_PROFILING_BLOCK_C("WaitForFreeLock", profiler::colors::Red200);
        QTUpdateEvents();

        QDateTime waitingTime = QDateTime::currentDateTime();
        if (currentDateTime.msecsTo(waitingTime) > timeoutMillis)
        {
            JD_CONSOLE_FUNCTION("Timeout while trying to aquire file lock for: " << abspath);
            return false;
        }
        //using namespace std::literals;
        //std::this_thread::sleep_for(1ms);
#ifdef _WIN32
        Sleep(1);
#else
        usleep(1000);
#endif

        /*QJsonObject data;
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
        if(dateTime.msecsTo(currentDateTime) > timeoutMillis)
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
        }*/
    }

    return true;
}
bool JDManager::unlockFile(const std::string &relativePath) const
{
    JD_PROFILING_FUNCTION_C(profiler::colors::Red);

    if (!m_fileLock)
        return true;


    std::string path = relativePath;
    std::string abspath = m_databasePath+"\\"+path+m_lockFileEnding;

    m_fileLock->unlock();
    delete m_fileLock;
    m_fileLock = nullptr;
    if (QFile::exists(abspath.c_str()))
    {
        QFile f(abspath.c_str());
        if (!f.remove())
        {
            JD_CONSOLE_FUNCTION("Can't remove filelock for: " << abspath);
            return false;
        }
    }

    return true;
}

bool JDManager::makeDatabaseDirs() const
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_2)
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
        JD_CONSOLE_FUNCTION("Can't create database folder: " << path.c_str());
    }
    success &= exists;
    
    return success;
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

size_t JDManager::getJsonIndexByID(const std::vector<QJsonObject>& jsons, const std::string objID) const
{
    for (size_t i = 0; i < jsons.size(); ++i)
    {
        std::string id;
        if (getJsonValue(jsons[i], id, JDObjectInterface::m_tag_objID))
        {
            if (id == objID)
                return i;
        }
    }
    return std::string::npos;
}

JDObjectInterface* JDManager::getObjectDefinition(const QJsonObject& json) const
{
    std::string className;
    if (getJsonValue(json, className, JDObjectInterface::m_tag_className))
    {
        return getObjectDefinition(className);
    }
    return nullptr;
}
JDObjectInterface* JDManager::getObjectDefinition(const std::string& className) const
{
    auto it = m_objDefinitions.find(className);
    if (it == m_objDefinitions.end())
        return nullptr;
    return it->second;
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
