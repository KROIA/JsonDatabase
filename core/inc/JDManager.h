#pragma once

#include "JD_base.h"
#include "JDObjectInterface.h"
#include "JDObjectRegistry.h"
#include "FileReadWriteLock.h"
#include "ThreadWorker.h"
#include "JDObjectLocker.h"
#include "FileChangeWatcher.h"
#include "Signal.h"
#include "JDObjectContainer.h"

#include <string>
#include <map>
#include <QJsonObject>
#include <QDir>
#include <filesystem>
#include <mutex>


//#define JSON_DATABSE_USE_THREADS
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

        /*
            Overrides the data of the object with the data from the database file.
        */
        bool loadObject(JDObjectInterface *obj);


        enum LoadMode
        {
            newObjects = 1,
            changedObjects = 2,
            removedObjects = 4,
            allObjects = 7,

            overrideChanges = 8,
        };
        /*
            Loads the objects from the database file.
            New objects are created and added to the database. See signal objectAddedToDatabase.
            Objects that have loaded different data are replaced with the new instance. See signal objectChangedFromDatabase.
            Objects that are not in the database file anymore are removed from the database. See signal objectRemovedFromDatabase.
        */
        bool loadObjects(int mode = LoadMode::allObjects);


        void clearObjects();
        bool addObject(JDObjectInterface* obj);
        bool addObject(const std::vector<JDObjectInterface*> &objList);
        JDObjectInterface* replaceObject(JDObjectInterface* obj);
        std::vector<JDObjectInterface*> replaceObjects(const std::vector<JDObjectInterface*>& objList);
        bool removeObject(JDObjectInterface* obj);
        bool removeObjects(const std::vector<JDObjectInterface*> &objList);
        template<typename T>
        bool removeObjects();
        template<typename T>
        bool deleteObjects();
        template<typename T>
        size_t getObjectCount() const;
        size_t getObjectCount() const;
        bool exists(JDObjectInterface* obj) const;
        bool exists(const std::string &id) const;

        template<typename T>
        T* getObject(const std::string &objID);
        JDObjectInterface* getObject(const std::string &objID);
        template<typename T>
        std::vector<T*> getObjects() const;
        const std::vector<JDObjectInterface*> &getObjects() const;

        const std::string& getUser() const; // Owner of this database instance
        const std::string& getSessionID() const;

        bool lockObj(JDObjectInterface* obj) const;
        bool unlockObj(JDObjectInterface* obj) const;
        bool isObjLocked(JDObjectInterface* obj) const;


        // Checks for changes in the database file
        void update();
        // Signals 
        /*
            The fileChanged signal gets emited if the database json file has changed.
            Can be used to reload the database.
        */
        void connectDatabaseFileChangedSlot(const Signal<>::SlotFunction& slotFunction);
        void disconnectDatabaseFileChangedSlot(const Signal<>::SlotFunction& slotFunction);
        
        /*
            The removedFromDatabase signal gets emited if the database has loaded less objects as
            currently in this instance contained.
            The removed objects are removed from this database but not deleted.
        */
        void connectObjectRemovedFromDatabaseSlot(const Signal<const JDObjectContainer&>::SlotFunction& slotFunction);
        void disconnectObjectRemovedFromDatabaseSlot(const Signal<const JDObjectContainer&>::SlotFunction& slotFunction);
	    
        /*
            The objectAddedToDatabase signal gets emited if the database has loaded more objects as
            currently in this instance contained.
            The added objects are added to this database.
        */
        void connectObjectAddedToDatabaseSlot(const Signal<const JDObjectContainer&>::SlotFunction& slotFunction);
        void disconnectObjectAddedToDatabaseSlot(const Signal<const JDObjectContainer&>::SlotFunction& slotFunction);
        
        /*
            The objectChangedFromDatabase signal gets emited if the database has loaded an object with the same id as
            an object in this instance but with different data.
            The changed objects are new instances.
            The old object gets replaced with the new one.
            the old object will not be deleted.
        */
        void connectObjectChangedFromDatabaseSlot(const Signal<const std::vector<JDObjectPair>&>::SlotFunction& slotFunction);
        void disconnectObjectChangedFromDatabaseSlot(const Signal<const std::vector<JDObjectPair>&>::SlotFunction& slotFunction);


        void connectObjectOverrideChangeFromDatabaseSlot(const Signal<const JDObjectContainer&>::SlotFunction& slotFunction);
        void disconnectObjectOverrideChangeFromDatabaseSlot(const Signal<const JDObjectContainer&>::SlotFunction& slotFunction);

    protected:

        virtual void onNewObjectsInstantiated(const std::vector<JDObjectInterface*>& newObjects);

    private:
        

        bool saveObjects_internal(const std::vector<JDObjectInterface*>& objList) const;
        bool addObject_internal(JDObjectInterface* obj);
        JDObjectInterface* replaceObject_internal(JDObjectInterface* obj);
        bool removeObject_internal(JDObjectInterface* obj);
        bool exists_internal(JDObjectInterface* obj) const;
        bool exists_internal(const std::string& id) const;
        JDObjectInterface* getObject_internal(const std::string& objID);
        const std::vector<JDObjectInterface*> &getObjects_internal() const;

        std::string getDatabaseFilePath() const;
        
        bool getJsonArray(const std::vector<JDObjectInterface*>& objs, std::vector<QJsonObject>& jsonOut) const;
        bool serializeObject(JDObjectInterface* obj, std::string& serializedOut) const;
        bool serializeJson(const QJsonObject& obj, std::string& serializedOut) const;

        bool deserializeJson(const QJsonObject& json, JDObjectInterface* objOriginal, JDObjectInterface*& objOut) const;
        bool deserializeOverrideFromJson(const QJsonObject& json, JDObjectInterface* obj, bool &hasChangedOut) const;
        bool deserializeOverrideFromJson(const QJsonObject& json, JDObjectInterface* obj) const;


        bool lockFile(
            const std::string& directory,
            const std::string& fileName,
            FileReadWriteLock::Access direction) const;
        bool lockFile(
            const std::string& directory,
            const std::string& fileName,
            FileReadWriteLock::Access direction,
            unsigned int timeoutMillis) const;
        bool unlockFile() const;
        bool isFileLockedByOther(const 
            std::string& directory,
            const std::string& fileName, 
            FileReadWriteLock::Access accessType) const;
        FileLock::Error getLastLockError() const;
        const std::string &getLastLockErrorStr() const;

        bool writeJsonFile(
            const std::vector<QJsonObject>& jsons,
            const std::string& directory,
            const std::string& fileName,
            const std::string& fileEnding,
            bool zipFormat,
            bool lockedRead) const;
        bool writeJsonFile(
            const QJsonObject& json,
            const std::string& directory,
            const std::string& fileName,
            const std::string& fileEnding,
            bool zipFormat,
            bool lockedRead) const;

        bool readJsonFile(
            std::vector<QJsonObject>& jsonsOut,
            const std::string& directory,
            const std::string& fileName,
            const std::string& fileEnding,
            bool zipFormat,
            bool lockedRead) const;
        bool readJsonFile(
            QJsonObject &objOut, 
            const std::string& directory,
            const std::string& fileName,
            const std::string& fileEnding,
            bool zipFormat,
            bool lockedRead) const;

        bool readFile(
            QByteArray& fileDataOut,
            const std::string& directory,
            const std::string& fileName,
            const std::string& fileEnding,
            bool lockedRead) const;
        bool writeFile(
            const QByteArray& fileData, 
            const std::string& directory, 
            const std::string& fileName,
            const std::string& fileEnding,
            bool lockedRead) const;


        static void QTUpdateEvents();

        void restartFileWatcher();
        


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
        bool decompressString(const QByteArray& compressedData, QString& outputString) const;

        std::string m_databasePath;
        std::string m_databaseName;
        std::string m_sessionID;
        std::string m_user;

        JDObjectLocker m_lockTable;
        mutable FileReadWriteLock* m_fileLock;
        mutable std::mutex m_mutex;
        mutable std::mutex m_updateMutex;
        bool m_useZipFormat;

        FileChangeWatcher *m_databaseFileWatcher;

        struct Signals
        {
            struct ContainerSignal
            {
                Signal<const JDObjectContainer&> signal;
                JDObjectContainer container;

                ContainerSignal(const std::string& name)
                    : signal(name) {}

                void emitSignalIfNotEmpty()
				{
                    if (container.size() == 0)
                        return;
					signal.emitSignal(container);
                    container.clear();
				}
            };
            struct ObjectChangeSignal
            {
                Signal<const std::vector<JDObjectPair>&> signal;
                std::vector<JDObjectPair> container;

                ObjectChangeSignal(const std::string& name)
                    : signal(name) {}
                void emitSignalIfNotEmpty()
                {
                    if (container.size() == 0)
                        return;
                    signal.emitSignal(container);
                    container.clear();
                }
            };
            Signal<> databaseFileChanged;
            ContainerSignal objectRemovedFromDatabase;
            ContainerSignal objectAddedToDatabase;
            ContainerSignal objectOverrideChangeFromDatabase;
            ObjectChangeSignal objectChangedFromDatabase;

            Signals()
                : databaseFileChanged("DatabaseFileChanged")
                , objectRemovedFromDatabase("RemovedFromDatabase")
                , objectAddedToDatabase("AddedToDatabase")
                , objectOverrideChangeFromDatabase("OverrideChangeFromDatabase")
                , objectChangedFromDatabase("ChangedFromDatabase")
            { }
            void clearContainer()
            {
				objectRemovedFromDatabase.container.clear();
				objectAddedToDatabase.container.clear();
                objectOverrideChangeFromDatabase.container.clear();
                objectChangedFromDatabase.container.clear();
            }
        };
        Signals m_signals;
        // All objects contained in the database
        //std::map<std::string, JDObjectInterface*> m_objs;
        JDObjectContainer m_objs;

        // Instances to clone from
        //static std::map<std::string, JDObjectInterface*> s_objDefinitions;
        static std::mutex s_mutex;

        static const std::string s_jsonFileEnding;

        static const QString s_timeFormat;
        static const QString s_dateFormat;

        static const QString s_tag_sessionID;
        static const QString s_tag_user;
        static const QString s_tag_date;
        static const QString s_tag_time;

        static const unsigned int s_fileLockTimeoutMs;


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
/*template<typename T>
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
}*/
template<typename T>
bool JDManager::removeObjects()
{
    JD_GENERAL_PROFILING_FUNCTION(COLOR_STAGE_1)
    /*std::string jsonPath;
    bool folderDeleted = false;
    for (auto def : JDObjectRegistry::getRegisteredTypes())
    {
        T* el = dynamic_cast<T*> (def.second);
        if (el)
        {
            jsonPath = m_databasePath + "\\" + getFolderName(el);
            folderDeleted = deleteDir(jsonPath);
        }
    }
    */

    std::vector<T*> toRemove = getObjects<T>();
    //bool ret = folderDeleted;
    for(auto obj : toRemove)
    {
        m_objs.removeObject(obj);
        //if(!folderDeleted)
        //    ret &= removeObject_internal(obj);
    }

    return true;
}


template<typename T>
bool JDManager::deleteObjects()
{
    JD_GENERAL_PROFILING_FUNCTION(COLOR_STAGE_1)
    /*std::string jsonPath;
    bool folderDeleted = false;
    for (auto def : JDObjectRegistry::getRegisteredTypes())
    {
        T* el = dynamic_cast<T*> (def.second);
        if (el)
        {
            jsonPath = m_databasePath + "\\" + getFolderName(el);
            folderDeleted = deleteDir(jsonPath);
        }
    }*/


    std::vector<T*> toDelete = getObjects<T>();
    //bool ret = folderDeleted;
    for (auto obj : toDelete)
    {
        m_objs.removeObject(obj);
        //if (!folderDeleted)
        //    ret &= removeObject_internal(obj);
        delete obj;
    }
    return true;
}

template<typename T>
std::size_t JDManager::getObjectCount() const
{
    size_t c=0;
    for(auto &o : m_objs)
    {
        if(dynamic_cast<T*>(o))
            ++c;
    }
    return c;
}

template<typename T>
T* JDManager::getObject(const std::string &objID)
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
        T* obj = dynamic_cast<T*>(p);
        if(obj)
            list.push_back(obj);
    }
    return list;
}

}
