#pragma once

#include "JD_base.h"
#include "JDObjectInterface.h"
#include "FileLock.h"
#include "ThreadWorker.h"
#include "JDObjectLocker.h"

#include <string>
#include <map>
#include <QJsonObject>
#include <QDir>
#include <filesystem>
#include <mutex>


//#define JSON_DATABSE_USE_THREADS
#define JSON_DATABSE_MAX_THREAD_COUNT 5

#define JSON_DATABASE_USE_CMD_FILE_SEARCH

#define USE_MUTEX_LOCK



namespace JsonDatabase
{

class JSONDATABASE_EXPORT JDManager
{
    struct ObjectLoaderData
    {
        JDObjectInterface* obj = nullptr;
        std::string id;
        std::string filePath; // relative file path without fileending
        bool fileReadSuccess = false;
        bool newInstanceLoadedSuccess = false;
    };
    struct ObjectSaverData
    {
        JDObjectInterface* obj = nullptr;
        std::string serializedData;
    };
    struct Thread_loadChunkData
    {
        const std::vector<ObjectLoaderData*>* loaderData;
        size_t start=0, end=0, threadIndex=0, threadCount=0;
        bool success=false, finished=false;
    };
    struct Thread_saveChunkData
    {
        const std::vector<ObjectSaverData*>* saverData;
        size_t start, end, threadIndex, threadCount;
        bool success, finished;
    };
    public:
        static void startProfiler();
        static void stopProfiler(const std::string profileFilePath);


        JDManager(const std::string &databasePath,
                  const std::string &databaseName,
                  const std::string &sessionID,
                  const std::string &user);
        JDManager(const JDManager &other);
        virtual ~JDManager();

        /**
         * \return 0 no error
         *         1 class name is empty
         *           -> implement getClassName() of the DatabaseObject T
         *         2 object type was already added
         *         3 object type does not have the base JDObjectInterface
         */
        template<typename T>
        static int addObjectDefinition();
        template<typename T>
        static bool isInObjectDefinition();
        static bool isInObjectDefinition(const std::string &className);


        void setDatabaseName(const std::string& name);
        const std::string& getDatabaseName() const;

        void setDatabasePath(const std::string &path);
        const std::string &getDatabasePath() const;

        void enableZipFormat(bool enable);
        bool isZipFormatEnabled() const;

        bool saveObject(JDObjectInterface *obj) const;
        bool saveObjects() const;
        bool saveObjects(const std::vector<JDObjectInterface*> &objList) const;

        bool loadObject(JDObjectInterface *obj);
        bool loadObjects();

        void clearObjects();
        bool addObject(JDObjectInterface* obj);
        bool addObject(const std::vector<JDObjectInterface*> &objList);
        bool removeObject(JDObjectInterface* obj);
        bool removeObjects(const std::vector<JDObjectInterface*> &objList);
        template<typename T>
        bool removeObjects();
        template<typename T>
        bool deleteObjects();
        template<typename T>
        std::size_t getObjectCount() const;
        std::size_t getObjectCount() const;
        bool exists(JDObjectInterface* obj) const;
        bool exists(const std::string &id) const;

        template<typename T>
        T* getObject(const std::string &objID) const;
        JDObjectInterface* getObject(const std::string &objID) const;
        template<typename T>
        std::vector<T*> getObjects() const;
        std::vector<JDObjectInterface*> getObjects() const;

        const std::string& getUser() const; // Owner of this database instance
        const std::string& getSessionID() const;

        bool lockObj(JDObjectInterface* obj) const;
        bool unlockObj(JDObjectInterface* obj) const;
        bool isObjLocked(JDObjectInterface* obj) const;
        
    protected:

        //virtual void onNewObjectsInstantiated(const std::vector<JDObjectInterface*>& newObjects);

    private:
        bool saveObjects_internal(const std::vector<JDObjectInterface*>& objList) const;
        bool addObject_internal(JDObjectInterface* obj);
        bool removeObject_internal(JDObjectInterface* obj);
        bool exists_internal(JDObjectInterface* obj) const;
        bool exists_internal(const std::string& id) const;
        JDObjectInterface* getObject_internal(const std::string& objID) const;
        std::vector<JDObjectInterface*> getObjects_internal() const;

        std::string getDatabaseFilePath() const;
        
        bool getJsonArray(const std::vector<JDObjectInterface*>& objs, std::vector<QJsonObject>& jsonOut) const;
        bool serializeObject(JDObjectInterface* obj, std::string& serializedOut) const;
        bool serializeJson(const QJsonObject& obj, std::string& serializedOut) const;

        bool deserializeJson(const QJsonObject& json, JDObjectInterface*& objOut) const;

        void getJsonFileContent(const std::vector<QJsonObject>& jsons, std::string& fileContentOut) const;
        bool writeJsonFile(const std::vector<QJsonObject>& jsons, const std::string &outputFile) const;
        bool readJsonFile(const std::string& inputFile, std::vector<QJsonObject>& jsonsOut) const;
        // bool deserializeJson(std::string)

        // relativePath without fileEnding
        //bool writeJsonFile(const QJsonObject &obj, const std::string &relativePath) const;
        //bool writeJsonFile(const QJsonObject &obj, const std::string &relativePath, const std::string &fileEnding) const;
        
        bool readJsonFile(QJsonObject &obj, const std::string &relativePath) const;
        bool readJsonFile(QJsonObject &obj, const std::string &relativePath, const std::string &fileEnding) const;

        bool lockFile(const std::string &relativePath, unsigned int timeoutMillis = 1000) const;
        bool unlockFile(const std::string &relativePath) const;


        static void QTUpdateEvents();


        // Filesystem
        bool makeDatabaseDirs() const;
        bool deleteDir(const std::string &dir) const;
        bool deleteFile(const std::string& file) const;

        static bool getJsonValue(const QJsonObject &obj, QVariant &value, const QString &key);
        static bool getJsonValue(const QJsonObject &obj, QTime &value, const QString &key);
        static bool getJsonValue(const QJsonObject &obj, QDate &value, const QString &key);
        static bool getJsonValue(const QJsonObject &obj, QString &value, const QString &key);
        static bool getJsonValue(const QJsonObject &obj, std::string &value, const QString &key);
        static bool getJsonValue(const QJsonObject &obj, int &value, const QString &key);

        static size_t getJsonIndexByID(const std::vector<QJsonObject>& jsons, const std::string objID);

        static JDObjectInterface* getObjectDefinition(const QJsonObject& json);
        static JDObjectInterface* getObjectDefinition(const std::string &className);

        static int executeCommand(const std::string& command);
        static std::string executeCommandPiped(const std::string& command);

        void compressString(const QString& inputString, QByteArray& compressedData) const;
        void decompressString(const QByteArray& compressedData, QString& outputString) const;

        std::string m_databasePath;
        std::string m_databaseName;
        std::string m_sessionID;
        std::string m_user;

        JDObjectLocker m_lockTable;
        mutable FileLock* m_fileLock;
        mutable std::mutex m_mutex;
        bool m_useZipFormat;

        // All objects contained in the database
        std::map<std::string, JDObjectInterface*> m_objs;

        // Instances to clone from
        static std::map<std::string, JDObjectInterface*> s_objDefinitions;
        static std::mutex s_mutex;

        static const std::string m_jsonFileEnding;
        static const std::string m_lockFileEnding;

        static const QString m_timeFormat;
        static const QString m_dateFormat;

        static const QString m_tag_sessionID;
        static const QString m_tag_user;
        static const QString m_tag_date;
        static const QString m_tag_time;


#ifdef JSON_DATABSE_USE_THREADS
        enum ThreadWorkType
        {
            load,
            save
        };
        class ThreadWorkObj : public ThreadWork
        {
            public:
                ThreadWorkObj(JDManager* manager);
                ~ThreadWorkObj();
                void setLoadData(const Thread_loadChunkData& data);
                void setSaveData(const Thread_saveChunkData& data);

                const Thread_loadChunkData& getLoadData() const;
                const Thread_saveChunkData& getSaveData() const;
            private:
                void process(int threadIndex) override;

                Thread_loadChunkData m_loadData;
                Thread_saveChunkData m_saveData;
                JDManager* m_manager;
                ThreadWorkType m_workType;
        };
        friend ThreadWorkObj;
        std::vector< ThreadWorkObj*> m_threadJobs;
        mutable ThreadWorker m_threadWorker;
        
        void setupThreadWorker();

        class ThreadWorkFindJsonFiles : public ThreadWork
        {
        public:
            ThreadWorkFindJsonFiles(JDManager* manager);
        private:
            void process(int threadIndex) override;
            JDManager* m_manager;

        public:
            std::string m_start_dir;
            std::vector<std::string> m_results;
            bool m_finished;
            bool m_crashed;
        };
        void findJsonFilesRecustive_threaded(const std::string& start_dir,
            std::vector<std::string>& results,
            bool& finished,
            bool& crashed);

        ThreadWorkFindJsonFiles* m_threadJsonFinderJob = nullptr;
        mutable ThreadWorker m_threadWorker_fileFinder;
#endif

};
template<typename T>
int JDManager::addObjectDefinition()
{
    int error = 0;
    T* obj = new T();
    JDObjectInterface *i = dynamic_cast<JDObjectInterface*>(obj);
    std::string className;
    if(i)
    {
        className = obj->className();
        if(className.size() == 0)
            error = 1;
        if(s_objDefinitions.find(className) != s_objDefinitions.end())
            error = 2;
    }
    else
        error = 3;
    if(error)
    {
        delete obj;
        return error;
    }
    s_objDefinitions.insert(std::pair<std::string, JDObjectInterface*>(className, obj));
    return error;
}
template<typename T>
bool JDManager::isInObjectDefinition()
{
    T* obj = new T();
    JDObjectInterface *i = dynamic_cast<JDObjectInterface*>(obj);
    std::string className;
    if(i)
    {
        className = obj->className();
        if(className.size() == 0)
            return false;
        if(s_objDefinitions.find(className) != s_objDefinitions.end())
            return true;
    }
    return false;
}
template<typename T>
bool JDManager::removeObjects()
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_1)
    std::string jsonPath;
    bool folderDeleted = false;
    for (auto def : s_objDefinitions)
    {
        T* el = dynamic_cast<T*> (def.second);
        if (el)
        {
            jsonPath = m_databasePath + "\\" + getFolderName(el);
            folderDeleted = deleteDir(jsonPath);
        }
    }


    std::vector<T*> toRemove = getObjects<T>();
    bool ret = folderDeleted;
    for(auto obj : toRemove)
    {
        m_objs.erase(obj->getObjectID());
        if(!folderDeleted)
            ret &= removeObject_internal(obj);
    }

    return ret;
}


template<typename T>
bool JDManager::deleteObjects()
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_1)
    std::string jsonPath;
    bool folderDeleted = false;
    for (auto def : s_objDefinitions)
    {
        T* el = dynamic_cast<T*> (def.second);
        if (el)
        {
            jsonPath = m_databasePath + "\\" + getFolderName(el);
            folderDeleted = deleteDir(jsonPath);
        }
    }


    std::vector<T*> toDelete = getObjects<T>();
    bool ret = folderDeleted;
    for (auto obj : toDelete)
    {
        m_objs.erase(obj->getObjectID());
        if (!folderDeleted)
            ret &= removeObject_internal(obj);
        delete obj;
    }
    return ret;
}

template<typename T>
std::size_t JDManager::getObjectCount() const
{
    size_t c=0;
    for(auto &o : m_objs)
    {
        if(dynamic_cast<T*>(o.second))
            ++c;
    }
    return c;
}

template<typename T>
T* JDManager::getObject(const std::string &objID) const
{
    JDObjectInterface *obj = getObject(objID);
    T *casted = dynamic_cast<T*>(obj);
    return casted;
}

template<typename T>
std::vector<T*> JDManager::getObjects() const
{
    std::vector<T*> list;
    list.reserve(m_objs.size());
    for(auto &p : m_objs)
    {
        T* obj = dynamic_cast<T*>(p.second);
        if(obj)
            list.push_back(obj);
    }
    return list;
}

}
