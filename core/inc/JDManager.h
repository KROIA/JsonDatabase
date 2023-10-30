#pragma once

#include "JDObjectInterface.h"
#include "ThreadWorker.h"
#include <string>
#include <map>
#include <QJsonObject>
#include <QDir>
#include <filesystem>


#define JSON_DATABSE_USE_THREADS
#define JSON_DATABSE_MAX_THREAD_COUNT 5

#define JSON_DATABASE_USE_CMD_FILE_SEARCH

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
        bool fileWriteSuccess = false;
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
        JDManager(const std::string &databasePath,
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
        int addObjectDefinition();
        template<typename T>
        bool isInObjectDefinition();
        bool isInObjectDefinition(const std::string &className);

        void setDatabasePath(const std::string &path);
        const std::string &getDatabasePath() const;

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
    protected:

        virtual void onNewObjectsInstantiated(const std::vector<JDObjectInterface*>& newObjects);

    private:
        

        // relativePath without fileEnding
        bool writeJsonFile(const QJsonObject &obj, const std::string &relativePath) const;
        bool writeJsonFile(const QJsonObject &obj, const std::string &relativePath, const std::string &fileEnding) const;
        bool readJsonFile(QJsonObject &obj, const std::string &relativePath) const;
        bool readJsonFile(QJsonObject &obj, const std::string &relativePath, const std::string &fileEnding) const;

        bool lockFile(const std::string &relativePath) const;
        bool unlockFile(const std::string &relativePath) const;

      

        void loadObjects_chunked(Thread_loadChunkData &data);
        void loadObjects_threaded(const std::vector<ObjectLoaderData*> & loaderData, bool &success);

        
        void saveObjects_chunked(Thread_saveChunkData& data) const;
        void saveObjects_threaded(const std::vector<ObjectSaverData*>& saverData, bool& success) const;
        static void QTUpdateEvents();

        std::string getRelativeFilePath(const std::string &objID) const;
        void getObjectFileList(QDir dir, const std::string &relativePath, std::vector<std::string> &list) const;
        void getObjectFileList_internal(const QDir &dirFilter,const QDir &fileFilter, const std::string &relativePath, std::vector<std::string> &list) const;
        void findJsonFilesRecursive(const std::filesystem::path& start_dir, std::vector<std::string>& results) const;
        void findJsonFilesRecursive(const std::string& start_dir, std::vector<std::string>& results) const;
        static void getDirsRecursive_internal(const std::string& dir, const std::string& relativeRoot, std::vector<std::string>& subDirs);
        static void getDirectories(const std::string& path, std::vector<std::string>& dirNames);
        static void getFileList(const std::string& path, const std::string& relativeRoot, const std::string& fileEndingFilter, std::vector<std::string>& fullFilePathList); // fileEndingFilter: ".json" not "json"
        //static std::string systemExec(const std::string& command);
        static void splitString(const std::string &str, std::vector<std::string> &container, const std::string& delimiter = "\n");

        bool saveObject_internal(JDObjectInterface *obj) const;
        bool saveObject_internal_noLock(JDObjectInterface *obj, const std::string *relativeFilePath = nullptr) const;

        // if objOut != nullptr, the loader try's to override the existing values with the loaded ones
        bool loadObject_internal(JDObjectInterface *&objOut, std::string relativePath);
        bool loadObject_internal_noLock(JDObjectInterface *&objOut, std::string relativePath, const std::string &uniqueID);

        bool removeObject_internal(JDObjectInterface* obj);

        // Filesystem
        bool makeDatabaseDirs() const;
        bool deleteJsonFile(JDObjectInterface *obj) const;
        bool deleteDir(const std::string &dir) const;

        std::string getFolderName(JDObjectInterface *obj) const;
        std::string getFileName(JDObjectInterface *obj) const;
        std::string getRelativeFilePath(JDObjectInterface *obj) const;

        bool getJsonValue(const QJsonObject &obj, QVariant &value, const QString &key) const;
        bool getJsonValue(const QJsonObject &obj, QTime &value, const QString &key) const;
        bool getJsonValue(const QJsonObject &obj, QDate &value, const QString &key) const;
        bool getJsonValue(const QJsonObject &obj, QString &value, const QString &key) const;
        bool getJsonValue(const QJsonObject &obj, std::string &value, const QString &key) const;
        bool getJsonValue(const QJsonObject &obj, int &value, const QString &key) const;

        static int executeCommand(const std::string& command);
        static std::string executeCommandPiped(const std::string& command);

        std::string m_databasePath;
        std::string m_sessionID;
        std::string m_user;

        // All objects contained in the database
        std::map<std::string, JDObjectInterface*> m_objs;

        // Instances to clone from
        std::map<std::string, JDObjectInterface*> m_objDefinitions;

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
        if(m_objDefinitions.find(className) != m_objDefinitions.end())
            error = 2;
    }
    else
        error = 3;
    if(error)
    {
        delete obj;
        return error;
    }
    m_objDefinitions.insert(std::pair<std::string, JDObjectInterface*>(className, obj));
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
        if(m_objDefinitions.find(className) != m_objDefinitions.end())
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
    for (auto def : m_objDefinitions)
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
    for (auto def : m_objDefinitions)
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
