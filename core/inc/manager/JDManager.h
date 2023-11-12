#pragma once

#include "JD_base.h"

#include "JDManagerSignals.h"
#include "JDManagerFileSystem.h"
#include "JDManagerObjectManager.h"
#include "JDObjectLocker.h"

#include "object/JDObjectInterface.h"
#include "object/JDObjectRegistry.h"

#include "utilities/ThreadWorker.h"
#include "utilities/Signal.h"


#include <string>
//#include <map>
//#include <QJsonObject>
//#include <QDir>
//#include <filesystem>
#include <mutex>


namespace JsonDatabase
{

class JSONDATABASE_EXPORT JDManager: 
    public Internal::JDManagerObjectManager, 
    public Internal::JDManagerFileSystem,
    public Internal::JDObjectLocker
{
    public:
        static void startProfiler();
        static void stopProfiler(const std::string profileFilePath);


        JDManager(const std::string &databasePath,
                  const std::string &databaseName,
                  const std::string &sessionID,
                  const std::string &user);
        JDManager(const JDManager &other);
        virtual ~JDManager();

        /*
            Returns the signals handler of this manager.
            Connect callbacks to the available signals.
        */
        Internal::JDManagerSignals& getSignals();

        void setDatabasePath(const std::string& path);
        void setDatabaseName(const std::string& name);

        const std::string& getDatabaseName() const;
        const std::string& getDatabasePath() const;

        std::string getDatabaseFilePath() const;

        void enableZipFormat(bool enable);
        bool isZipFormatEnabled() const;

        bool saveObject(JDObjectInterface *obj);
        bool saveObjects();
        bool saveObjects(const std::vector<JDObjectInterface*> &objList);

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



        const std::string& getUser() const; // Owner of this database instance
        const std::string& getSessionID() const;


        // Checks for changes in the database file
        void update();
        

        
    protected:


    private:
        

        bool saveObjects_internal(const std::vector<JDObjectInterface*>& objList);

        std::string m_databasePath;
        std::string m_databaseName;
        std::string m_sessionID;
        std::string m_user;

        mutable std::mutex m_mutex;
        mutable std::mutex m_updateMutex;
        bool m_useZipFormat;

        // Prevent multiple updates at the same time
        bool m_signleEntryUpdateLock;

        
        Internal::JDManagerSignals m_signals;

        static std::mutex s_mutex;

        

        static const QString s_timeFormat;
        static const QString s_dateFormat;

        static const QString s_tag_sessionID;
        static const QString s_tag_user;
        static const QString s_tag_date;
        static const QString s_tag_time;

        static const unsigned int s_fileLockTimeoutMs;



};
}
