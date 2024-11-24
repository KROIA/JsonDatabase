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
#include <QTimer>


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
    friend class Internal::JDObjectManager;
    friend class Internal::JDManagerSignals;
    friend class Internal::JDObjectLocker;

    friend class Internal::JDManagerAsyncWorker;

    friend class Internal::JDManagerAysncWorkLoadAllObjects;
    friend class Internal::JDManagerAysncWorkLoadSingleObject;
    friend class Internal::JDManagerAysncWorkSaveSingle;
    friend class Internal::JDManagerAysncWorkSaveList;
    Q_OBJECT
    public:
        JDManager();
        JDManager(const JDManager &other);
        virtual ~JDManager();

        void setUpdateInterval(int ms);
		int getUpdateInterval() const { return m_updateTimer.interval(); }
        bool setup(const std::string& databasePath,
                   const std::string& databaseName);
        bool setup(const std::string& databasePath,
                   const std::string& databaseName,
                   const std::string& user);
        bool stop();

        /**
         * @brief 
		 * Specifies if the database file should be in zip format.
         * @param enable 
         */
        void enableZipFormat(bool enable);

        /**
         * @brief 
		 * Returns if the database file is in zip format.
		 * @return true if the database file is in zip format, otherwise false
         */
        bool isZipFormatEnabled() const;


        /**
         * @brief 
         * Overrides the data of the object with the data from the database file.
		 * @param obj which should be loaded from the database
		 * @return true if the object was loaded successfully, otherwise false
         */
        bool loadObject(const JDObject &obj);

        /**
         * @brief 
		 * Overrides the data of the object with the data from the database file asynchronously.
		 * @param obj which should be loaded from the database
         */
        void loadObjectAsync(const JDObject &obj);

        /**
         * @brief 
		 * Loads the objects from the database file.
		 * @param mode to define which objects should be loaded
		 * @return true if the loading was successful, otherwise false
         */
        bool loadObjects(int mode = LoadMode::allObjects);

        /**
         * @brief 
		 * Loads the objects from the database file.
		 * @param mode to define which objects should be loaded
		 * @param progress which is used to report the progress of the loading. Used for loading bar etc.
		 * @return true if the loading was successful, otherwise false
         */
        bool loadObjects(int mode, Internal::WorkProgress* progress = nullptr);

        /**
         * @brief 
		 * Loads the objects from the database file asynchronously.
		 * @param mode to define which objects should be loaded
         */
        void loadObjectsAsync(int mode = LoadMode::allObjects);

        /**
         * @brief 
		 * Tries to save the object to the database file.
		 * The object must be locked by this session.
		 * @param obj to save to the database
		 * @return true if the object was saved successfully, otherwise false
         */
        bool saveObject(const JDObject &obj);

        /**
         * @brief 
		 * Tries to save the object to the database file asynchronously.
		 * The object must be locked by this session.
		 * @param obj to save to the database
         */
        void saveObjectAsync(const JDObject &obj);

        /**
         * @brief 
		 * Saves all objects which are in this database instance to the database file.
		 * All objects must be locked by this session.
		 * @return true if all objects were saved successfully, otherwise false
         */
        bool saveObjects();

        /**
         * @brief 
		 * Saves all objects from the list to the database file.
		 * All objects must be locked by this session.
		 * @param objs to save to the database
		 * @return true if all objects were saved successfully, otherwise false
         */
        bool saveObjects(const std::vector<JDObject> &objs);

        /**
         * @brief 
		 * Saves all objects which are locked by this user to the database file.
		 * All objects must be locked by this session.
         */
        void saveObjectsAsync();

        /**
         * @brief 
         * Saves all objects from the list to the database file asynchronously
         * All objects must be locked by this session.
         * @param objs 
         */
        void saveObjectsAsync(const std::vector<JDObject>& objs);

        /**
         * @brief 
         * Saves all objects that are locked by this session
         * @return true if all locked objects are saved, otherwise false
         */
        bool saveLockedObjects();

        /**
         * @brief 
         * Saves all objects that are locked by this session asynchronously
         */
        void saveLockedObjectsAsync();



        static void setDefaultFileWatchMode(Internal::FileChangeWatcher::Mode mode);
        static Internal::FileChangeWatcher::Mode getDefaultFileWatchMode();
        
        



        const Utilities::JDUser& getUser() const; // Owner of this database instance
        const std::string& getSessionID() const; // Session ID of this database instance
        const std::string& getLoadModeStr(int mode) const; // Returns a string representation of the load mode


       

        signals:
            void databaseFileChanged(); // Emitted when the database file has changed
            void lockedObjectsChanged(); // Emitted when the locked objects have changed
            //void onObjectRemovedFromDatabase(std::vector<JDObject> objs); // Emitted when an object is removed from the database
           // void onObjectAddedToDatabase(std::vector<JDObject> objs); // Emitted when an object is added to the database
            //void objectChangedFromDatabase(std::vector<JDObjectPair> objs); // Emitted when an object is changed in the database
            //void objectOverrideChangeFromDatabase(std::vector<JDObject> objs); // Emitted when an object is changed in the database
            void databaseOutdated(); // Emitted when the database is outdated

            void startAsyncWork(); // Emitted when an async work is started
            void endAsyncWork(); // Emitted when an async work is done
            void loadObjectDone(bool success, JDObject obj); 
            void loadObjectsDone(bool success); 
            void saveObjectDone(bool success, JDObject obj);
            void saveObjectsDone(bool success);

			void objectLocked(JDObject obj);
			void objectUnlocked(JDObject obj);
            void objectAdded(JDObject obj);
			void objectRemoved(JDObject obj);
            void objectChanged(JDObject obj);

    public slots:
                // Checks for changes in the database file
            void update();
    protected:

    



    private:
        
        bool loadObject_internal(const JDObject &obj, Internal::WorkProgress* progress);
        bool loadObjects_internal(int mode, Internal::WorkProgress* progress);
        bool saveObject_internal(const JDObject &obj, unsigned int timeoutMillis, Internal::WorkProgress* progress);
        bool saveObjects_internal(unsigned int timeoutMillis, Internal::WorkProgress* progress);
        bool saveObjects_internal(std::vector<JDObject> objList, unsigned int timeoutMillis, Internal::WorkProgress* progress, bool objsRemoved = false);


        void onAsyncWorkDone(std::shared_ptr<Internal::JDManagerAysncWork> work);
        void onAsyncWorkError(std::shared_ptr<Internal::JDManagerAysncWork> work);

        void onObjectLockerFileChanged();

		void emitSignals();
        

        Log::LogObject* m_logger = nullptr;
        Utilities::JDUser m_user;

        mutable std::mutex m_mutex;
        mutable std::mutex m_updateMutex;
        bool m_useZipFormat;

        // Prevent multiple updates at the same time
        bool m_signleEntryUpdateLock;
        bool m_setUp = false;
        QTimer m_updateTimer;

        struct SignalData
        {
            SignalData()
            {

            }
			SignalData(const SignalData& other)
			{
				m_databaseFileChanged = other.m_databaseFileChanged;
				m_lockedObjectsChanged = other.m_lockedObjectsChanged;
				m_databaseOutdated = other.m_databaseOutdated;

				objectLocked = other.objectLocked;
				objectUnlocked = other.objectUnlocked;
				objectAdded = other.objectAdded;
				objectRemoved = other.objectRemoved;
				objectChanged = other.objectChanged;
			}
			SignalData copyAndClear()
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				SignalData copy = *this;
                m_databaseFileChanged = false;
                m_lockedObjectsChanged = false;
                m_databaseOutdated = false;

                objectLocked.clear();
                objectUnlocked.clear();
                objectAdded.clear();
                objectRemoved.clear();
                objectChanged.clear();
				return copy;
			}

			void setDatabaseFileChanged() { std::lock_guard<std::mutex> lock(m_mutex); m_databaseFileChanged = true; }
			void setLockedObjectsChanged() { std::lock_guard<std::mutex> lock(m_mutex); m_lockedObjectsChanged = true; }
			void setDatabaseOutdated() { std::lock_guard<std::mutex> lock(m_mutex); m_databaseOutdated = true; }

			void addObjectLocked(JDObject obj) {
                std::lock_guard<std::mutex> lock(m_mutex);
				for (size_t i = 0; i < objectLocked.size(); ++i)
				{
					if (objectLocked[i] == obj)
						return;
				}
				objectLocked.push_back(obj);
            }
			void addObjectLocked(const std::vector<JDObject>& objs) {
                std::lock_guard<std::mutex> lock(m_mutex);
				for (const JDObject& obj : objs)
				{
                    for (size_t i = 0; i < objectLocked.size(); ++i)
                    {
                        if (objectLocked[i] == obj)
                            goto next;
                    }
                    objectLocked.push_back(obj);
                    next:;
				}
			}

			void addObjectUnlocked(JDObject obj) {
                std::lock_guard<std::mutex> lock(m_mutex);
				for (size_t i = 0; i < objectUnlocked.size(); ++i)
				{
					if (objectUnlocked[i] == obj)
						return;
				}
				objectUnlocked.push_back(obj);
			}
			void addObjectUnlocked(const std::vector<JDObject>& objs) {
                std::lock_guard<std::mutex> lock(m_mutex);
				for (const JDObject& obj : objs)
				{
                    for (size_t i = 0; i < objectUnlocked.size(); ++i)
                    {
                        if (objectUnlocked[i] == obj)
                            goto next;
                    }
                    objectUnlocked.push_back(obj);
                    next:;
				}
			}

			void addObjectAdded(JDObject obj) {
                std::lock_guard<std::mutex> lock(m_mutex);
                for (size_t i = 0; i < objectAdded.size(); ++i)
                {
                    if (objectAdded[i] == obj)
                        return;
                }
                objectAdded.push_back(obj);
				
			}
			void addObjectAdded(const std::vector<JDObject>& objs) {
                std::lock_guard<std::mutex> lock(m_mutex);
				for (const JDObject& obj : objs)
				{
                    for (size_t i = 0; i < objectAdded.size(); ++i)
                    {
                        if (objectAdded[i] == obj)
                            goto next;
                    }
                    objectAdded.push_back(obj);
                    next:;      
				}
			}

			void addObjectRemoved(JDObject obj) {
                std::lock_guard<std::mutex> lock(m_mutex);
				for (size_t i = 0; i < objectRemoved.size(); ++i)
				{
					if (objectRemoved[i] == obj)
						return;
				}
				objectRemoved.push_back(obj);
			}
			void addObjectRemoved(const std::vector<JDObject>& objs) {
                std::lock_guard<std::mutex> lock(m_mutex);
				for (const JDObject& obj : objs)
				{
                    for (size_t i = 0; i < objectRemoved.size(); ++i)
                    {
                        if (objectRemoved[i] == obj)
                            goto next;
                    }
                    objectRemoved.push_back(obj);
                    next:;
				}
			}

			void addObjectChanged(JDObject obj) {
                std::lock_guard<std::mutex> lock(m_mutex);
				for (size_t i = 0; i < objectChanged.size(); ++i)
				{
					if (objectChanged[i] == obj)
						return;
				}
				objectChanged.push_back(obj);
			}
			void addObjectChanged(const std::vector<JDObject>& objs) {
                std::lock_guard<std::mutex> lock(m_mutex);
				for (const JDObject& obj : objs)
				{
                    for (size_t i = 0; i < objectChanged.size(); ++i)
                    {
                        if (objectChanged[i] == obj)
                            goto next;
                    }
                    objectChanged.push_back(obj);
                    next:;
				}
			}

			bool databaseFileChanged() const { return m_databaseFileChanged; }
			bool lockedObjectsChanged() const { return m_lockedObjectsChanged; }
			bool databaseOutdated() const { return m_databaseOutdated; }

			const std::vector<JDObject>& getObjectLocked() const { return objectLocked; }
			const std::vector<JDObject>& getObjectUnlocked() const { return objectUnlocked; }
			const std::vector<JDObject>& getObjectAdded() const { return objectAdded; }
			const std::vector<JDObject>& getObjectRemoved() const { return objectRemoved; }
			const std::vector<JDObject>& getObjectChanged() const { return objectChanged; }



            void clear()
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_databaseFileChanged = false;
                m_lockedObjectsChanged = false;
                m_databaseOutdated = false;

                objectLocked.clear();
                objectUnlocked.clear();
                objectAdded.clear();
                objectRemoved.clear();
                objectChanged.clear();
            }
            private:
            bool m_databaseFileChanged = false;
            bool m_lockedObjectsChanged = false;
            bool m_databaseOutdated = false;

            std::vector<JDObject> objectLocked;
            std::vector<JDObject> objectUnlocked;
            std::vector<JDObject> objectAdded;
            std::vector<JDObject> objectRemoved;
            std::vector<JDObject> objectChanged;

			std::mutex m_mutex;
        };
        SignalData m_signalsToEmit;
        static const unsigned int s_fileLockTimeoutMs;
    };
}
