#pragma once

#include "JsonDatabase_base.h"
#include "JsonDatabase_Declaration.h"

#include "async/JDManagerAsyncWorker.h"
#include "async/work/JDManagerWorkLoadSingleObject.h"
#include "async/WorkProgress.h"
#include "JDManagerSignals.h"
#include "JDManagerFileSystem.h"
#include "JDManagerObjectManager.h"


#include "object/JDObjectInterface.h"
#include "object/JDObjectRegistry.h"

#include "utilities/Signal.h"
#include "utilities/JDUser.h"



#include <string>
#include <mutex>


namespace JsonDatabase
{

class JSON_DATABASE_EXPORT JDManager: 
    public Internal::JDManagerObjectManager, 
    public Internal::JDManagerFileSystem,
    //public Internal::JDObjectLocker,
    public Internal::JDManagerAsyncWorker
{
    friend class Internal::JDManagerFileSystem;
    friend class Internal::JDManagerObjectManager;
    friend class Internal::JDManagerSignals;
    friend class Internal::JDObjectLocker;

    friend class Internal::JDManagerAsyncWorker;

    friend class Internal::JDManagerAysncWorkLoadAllObjects;
    friend class Internal::JDManagerAysncWorkLoadSingleObject;
    friend class Internal::JDManagerAysncWorkSaveSingle;
    friend class Internal::JDManagerAysncWorkSaveList;

    public:
        JDManager(const std::string& databasePath,
            const std::string& databaseName);
        JDManager(const std::string &databasePath,
                  const std::string &databaseName,
                  const std::string &user);
        JDManager(const JDManager &other);
        virtual ~JDManager();

        bool setup();

        /*
            Returns the signals handler of this manager.
            Connect callbacks to the available signals.
        */
        Internal::JDManagerSignals& getSignals();

        

        void enableZipFormat(bool enable);
        bool isZipFormatEnabled() const;

        /*
            Overrides the data of the object with the data from the database file.
        */
        bool loadObject(const JDObject &obj);
        void loadObjectAsync(const JDObject &obj);


        
        /*
            Loads the objects from the database file.
            New objects are created and added to the database. See signal objectAddedToDatabase.
            Objects that have loaded different data are replaced with the new instance. See signal objectChangedFromDatabase.
            Objects that are not in the database file anymore are removed from the database. See signal objectRemovedFromDatabase.
        
            Emits signals:
				objectAddedToDatabase
				objectChangedFromDatabase
				objectRemovedFromDatabase
                objectOverrideChangeFromDatabase
        */
        bool loadObjects(int mode = LoadMode::allObjects);

        /*
            Loads the objects from the database file asynchronously.

            Emits signals:
                objectAddedToDatabase
                objectChangedFromDatabase
                objectRemovedFromDatabase
                objectOverrideChangeFromDatabase
                onLoadObjectsDone after the async work is done
        */
        void loadObjectsAsync(int mode = LoadMode::allObjects);

        /*
            Saves the object to the database file.
            If the object is not in the database file, it will be added.
        */
        bool saveObject(const JDObject &obj);

        /*
            Saves the object to the database file asynchronously.
            If the object is not in the database file, it will be added.
            A copy of the obj is made while calling this function.
            only the copy gets saved.

            Emits signals:
                onSaveObjectDone after the async work is done
                databaseOutdated if the database has changed before the save could be completed
                in this case the object is not saved. Try load the database first.
        */
        void saveObjectAsync(const JDObject &obj);

        /*
            Saves all objects which are in this database instance to the database file.
            
            Emits signals:
                databaseOutdated if the database has changed before the save could be completed
                in this case the object is not saved. Try load the database first.
        */
        bool saveObjects();

        /*
            Saves all objects which are in this database instance to the database file asynchronously.

            Emits signals:
				onSaveObjectsDone after the async work is done
				databaseOutdated if the database has changed before the save could be completed
				in this case the object is not saved. Try load the database first.
        */
        void saveObjectsAsync();
        
        



        const Utilities::JDUser& getUser() const; // Owner of this database instance
        const std::string& getSessionID() const; // Session ID of this database instance
        const std::string& getLoadModeStr(int mode) const; // Returns a string representation of the load mode


        // Checks for changes in the database file
        void update();
        

        
    protected:


    private:
        
        bool loadObject_internal(const JDObject &obj, Internal::WorkProgress* progress);
        bool loadObjects_internal(int mode, Internal::WorkProgress* progress);
        bool saveObject_internal(const JDObject &obj, unsigned int timeoutMillis, Internal::WorkProgress* progress);
        bool saveObjects_internal(unsigned int timeoutMillis, Internal::WorkProgress* progress);
        bool saveObjects_internal(const std::vector<JDObject> & objList, unsigned int timeoutMillis, Internal::WorkProgress* progress);


        void onAsyncWorkDone(std::shared_ptr<Internal::JDManagerAysncWork> work);
        void onAsyncWorkError(std::shared_ptr<Internal::JDManagerAysncWork> work);

        void onObjectLockerFileChanged();

        

        
        Utilities::JDUser m_user;

        mutable std::mutex m_mutex;
        mutable std::mutex m_updateMutex;
        bool m_useZipFormat;

        // Prevent multiple updates at the same time
        bool m_signleEntryUpdateLock;

        
        
        Internal::JDManagerSignals m_signals;
        //Internal::JDManagerAsyncWorker m_asyncWorker;
        

        //static const QString s_timeFormat;
        //static const QString s_dateFormat;
        //
        //static const QString s_tag_sessionID;
        //static const QString s_tag_user;
        //static const QString s_tag_date;
        //static const QString s_tag_time;

        static const unsigned int s_fileLockTimeoutMs;


};
}
