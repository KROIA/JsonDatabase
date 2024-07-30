#pragma once

#include "JsonDatabase_base.h"
#include "JsonDatabase_Declaration.h"

#include "async/JDManagerAsyncWorker.h"
#include "async/work/JDManagerWorkLoadSingleObject.h"
#include "async/WorkProgress.h"
#include "JDManagerFileSystem.h"
#include "JDManagerObjectManager.h"


#include "object/JDObjectInterface.h"
#include "object/JDObjectRegistry.h"

#include "utilities/JDUser.h"

#include "Logger.h"

#include <string>
#include <mutex>

#include <QObject>


namespace JsonDatabase
{

class JSON_DATABASE_EXPORT JDManager: 
    public QObject,
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

    Q_OBJECT
    public:
        JDManager(
            const std::string& databasePath,
            const std::string& databaseName);
        JDManager(
            const std::string &databasePath,
            const std::string &databaseName,
            const std::string &user);
        JDManager(const JDManager &other);
        virtual ~JDManager();

        bool setup();
        bool stop();

        /*
            Returns the signals handler of this manager.
            Connect callbacks to the available signals.
        */
        //Internal::JDManagerSignals& getSignals();

        

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
        

        signals:
            void onDatabaseFileChanged(); // Emitted when the database file has changed
            void onLockedObjectsChanged(); // Emitted when the locked objects have changed
            void onObjectRemovedFromDatabase(std::vector<JDObject> objs); // Emitted when an object is removed from the database
            void onObjectAddedToDatabase(std::vector<JDObject> objs); // Emitted when an object is added to the database
            void onObjectChangedFromDatabase(std::vector<JDObjectPair> objs); // Emitted when an object is changed in the database
            void onObjectOverrideChangeFromDatabase(std::vector<JDObject> objs); // Emitted when an object is changed in the database
            void onDatabaseOutdated(); // Emitted when the database is outdated

            void onStartAsyncWork(); // Emitted when an async work is started
            void onEndAsyncWork(); // Emitted when an async work is done
            void onLoadObjectDone(bool success, JDObject obj); 
            void onLoadObjectsDone(bool success); 
            void onSaveObjectDone(bool success, JDObject obj);
            void onSaveObjectsDone(bool success);


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

        

        Log::LogObject* m_logger = nullptr;
        Utilities::JDUser m_user;

        mutable std::mutex m_mutex;
        mutable std::mutex m_updateMutex;
        bool m_useZipFormat;

        // Prevent multiple updates at the same time
        bool m_signleEntryUpdateLock;

        
        
        /*struct SignalData
        {
            std::vector<JDObject> onObjectRemovedFromDatabase;
            std::vector<JDObject> onObjectAddedToDatabase;
            std::vector<JDObjectPair> onObjectChangedFromDatabase;
            std::vector<JDObject> onObjectOverrideChangeFromDatabase;

            struct OnLoadObjectDone
            {
				bool success;
				JDObject obj;
			} onLoadObjectDone;
            struct OnLoadObjectsDone
            {
                bool signalActive = false;
				bool success;
			} onLoadObjectsDone;

            struct OnSaveObjectDone
            {
                bool success;
                JDObject obj;
            } onSaveObjectDone;
            struct OnSaveObjectsDone
            {
				bool signalActive = false;
				bool success;
			} onSaveObjectsDone;
        } m_signalData;
        std::mutex m_signalDataMutex;*/

        //Internal::JDManagerSignals m_signals;


        static const unsigned int s_fileLockTimeoutMs;


};
}
