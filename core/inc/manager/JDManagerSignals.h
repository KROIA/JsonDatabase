#pragma once

#include "JD_base.h"
#include "JDDeclaration.h"
#include "utilities/signal.h"
#include "object/JDObjectContainer.h"

#include <vector>
#include <mutex>

#define DECLARE_SIGNAL_CONNECT_DISCONNECT(signalName, ...) \
    void connect_##signalName##_slot(const Signal<__VA_ARGS__>::SlotFunction& slotFunction); \
    template<typename ObjectType> \
    void connect_##signalName##_slot(ObjectType* obj, void(ObjectType::* memberFunc)(__VA_ARGS__)) \
    { \
        ##signalName##.connectSlot(obj, memberFunc); \
    } \
    void disconnect_##signalName##_slot(const Signal<__VA_ARGS__>::SlotFunction& slotFunction); \
    template<typename ObjectType> \
    void disconnect_##signalName##_slot(ObjectType* obj, void(ObjectType::* memberFunc)(__VA_ARGS__)) \
    { \
        ##signalName##.disconnectSlot(obj, memberFunc); \
    } 


namespace JsonDatabase
{
    namespace Internal
    {
        //class JDManager;
        class JSONDATABASE_EXPORT JDManagerSignals
        {
            friend JDManager;
            friend JDManagerObjectManager;
            friend JDManagerFileSystem;
            friend JDObjectLocker;
        
            JDManagerSignals(JDManager& manager, std::mutex& mtx);
        public:
            /*
                Signals have all the syntax like the following example:

                DECLARE_SIGNAL_CONNECT_DISCONNECT(objectRemovedFromDatabase, const JDObjectContainer&)
                Will be expanded to:
                    void connect_objectRemovedFromDatabase_slot(
                        const Signal<const JDObjectContainer&>::SlotFunction& slotFunction);

            */
            // Signals 
            /*
                The databaseFileChanged signal gets emited if the database json file has changed.
                Can be used to reload the database.
            */
            DECLARE_SIGNAL_CONNECT_DISCONNECT(databaseFileChanged, )


            /*
                The lockedObjectsChanged signal gets emited if new objects are locked or unlocked.
            	Can be used to update the UI.
            */
            DECLARE_SIGNAL_CONNECT_DISCONNECT(lockedObjectsChanged, )

            /*
                The objectRemovedFromDatabase signal gets emited if the database has loaded less objects as
                currently in this instance contained.
                The removed objects are removed from this database but not deleted.
            */
            DECLARE_SIGNAL_CONNECT_DISCONNECT(objectRemovedFromDatabase, const JDObjectContainer&)

            /*
                The objectAddedToDatabase signal gets emited if the database has loaded more objects as
                currently in this instance contained.
                The added objects are added to this database.
            */
            DECLARE_SIGNAL_CONNECT_DISCONNECT(objectAddedToDatabase, const JDObjectContainer&)

            /*
                The objectChangedFromDatabase signal gets emited if the database has loaded an object with the same id as
                an object in this instance but with different data.
                The changed objects are new instances.
                The old object gets replaced with the new one.
                the old object will not be deleted.
            */
            DECLARE_SIGNAL_CONNECT_DISCONNECT(objectChangedFromDatabase, const std::vector<JDObjectPair>&)

            /*
                The objectOverrideChangeFromDatabase signal gets emited if the database has loaded
                data into an object which was alreay instantiated in the manager.
                The signal gets only emited if the object data was different from the data in the database file.
            */
            DECLARE_SIGNAL_CONNECT_DISCONNECT(objectOverrideChangeFromDatabase, const JDObjectContainer&)

            /*
                The databaseOutdated signal gets emited if the user tries to save the database but the database file
                has changed since the last load.
                The user should reload the database before saving.
            */
            DECLARE_SIGNAL_CONNECT_DISCONNECT(databaseOutdated, )


            // -----------------------------------------------------------------------------------------------
            //                              JDManager async callbacks
            // -----------------------------------------------------------------------------------------------

            /*
                The loadObject signal gets emited if the manager has loaded the given object in async mode.
                The first parameter is the success state of the load operation.
                The second parameter is the object that was loaded.
            */
            DECLARE_SIGNAL_CONNECT_DISCONNECT(onLoadObjectDone, bool, JDObjectInterface*)
            //void connect_onLoadObjectDone_slot(const Signal<bool, JDObjectInterface*>::SlotFunction& slotFunction, bool onlyOnce);

            /*
                The loadObjects gets emited if the manager has loaded all objects in async mode.
                The first parameter is the success state of the load operation.
            */
            DECLARE_SIGNAL_CONNECT_DISCONNECT(onLoadObjectsDone, bool)

            /*
                The saveObject signal gets emited if the manager has saved the given object in async mode.
                The first parameter is the success state of the save operation.
                The second parameter is the object that was saved.
            */
            DECLARE_SIGNAL_CONNECT_DISCONNECT(onSaveObjectDone, bool, JDObjectInterface*)

            /*
                The saveObjects signal gets emited if the manager has saved all objects in async mode.
                The first parameter is the success state of the save operation.
            */
            DECLARE_SIGNAL_CONNECT_DISCONNECT(onSaveObjectsDone, bool)

        protected:
            
            struct ContainerSignal
            {
                Signal<const JDObjectContainer&> signal;
                

                ContainerSignal(const std::string& name);
                void reserve(size_t size);
                void addObjs(const std::vector<JDObjectInterface*>& objs);
                void addObj(JDObjectInterface* obj);
                void clear();
                void emitSignalIfNotEmpty();

                void connectSlot(const Signal<const JDObjectContainer&>::SlotFunction& slot);
                template<typename ObjectType>
                void connectSlot(ObjectType* obj, void(ObjectType::* memberFunc)(const JDObjectContainer&))
                {
                    signal.connectSlot(obj, memberFunc);
                }
                void disconnectSlot(const Signal<const JDObjectContainer&>::SlotFunction& slot);
                template<typename ObjectType>
                void disconnectSlot(ObjectType* obj, void(ObjectType::* memberFunc)(const JDObjectContainer&))
                {
                    signal.disconnectSlot(obj, memberFunc);
                }
            private:
                JDObjectContainer container;
                std::mutex m_mutex;
            };
            struct ObjectChangeSignal
            {
                Signal<const std::vector<JDObjectPair>&> signal;
                

                ObjectChangeSignal(const std::string& name);
                void reserve(size_t size);
                void addPairs(const std::vector<JDObjectPair>& pairs);
                void addPair(const JDObjectPair& pair);
                void clear();
                void emitSignalIfNotEmpty();

                void connectSlot(const Signal<const std::vector<JDObjectPair>&>::SlotFunction& slot);
                template<typename ObjectType>
                void connectSlot(ObjectType* obj, void(ObjectType::* memberFunc)(const std::vector<JDObjectPair>&))
                {
                    signal.connectSlot(obj, memberFunc);
                }
                void disconnectSlot(const Signal<const std::vector<JDObjectPair>&>::SlotFunction& slot);
                template<typename ObjectType>
                void disconnectSlot(ObjectType* obj, void(ObjectType::* memberFunc)(const std::vector<JDObjectPair>&))
                {
                    signal.disconnectSlot(obj, memberFunc);
                }
            private:
                std::vector<JDObjectPair> container;
                std::mutex m_mutex;
            };

            Signal<> databaseFileChanged;
            Signal<> lockedObjectsChanged;
            ContainerSignal objectRemovedFromDatabase;
            ContainerSignal objectAddedToDatabase;
            ObjectChangeSignal objectChangedFromDatabase;
            ContainerSignal objectOverrideChangeFromDatabase;
            Signal<> databaseOutdated;

            // JDManager async callbacks
            struct AsyncLoadObjectData
            {
                bool success;
                JDObjectInterface* object;
            };
            struct AsyncLoadObjectsData
            {
				bool success;
			};
            struct AsyncSaveObjectData
            {
				bool success;
                JDObjectInterface* object;
			};
            struct AsyncSaveObjectsData
			{
                bool success;
            };
            Signal<bool, JDObjectInterface*> onLoadObjectDone;
            Signal<bool> onLoadObjectsDone;
            Signal<bool, JDObjectInterface*> onSaveObjectDone;
            Signal<bool> onSaveObjectsDone;

            enum Signals
            {
                signal_databaseFileChanged,
                signal_lockedObjectsChanged,
                signal_objectRemovedFromDatabase,
                signal_objectAddedToDatabase,
                signal_objectChangedFromDatabase,
                signal_objectOverrideChangeFromDatabase,
                signal_databaseOutdated,

                signal_onLoadObjectDone,
                signal_onLoadObjectsDone,
                signal_onSaveObjectDone,
                signal_onSaveObjectsDone,

                signal_count
            };

            
            void clearContainer();
            void emitIfNotEmpty();
            

            void addToQueue(Signals signal, bool onlyOnce);
            void addToQueue(Signals signal, bool success, JDObjectInterface*obj, bool onlyOnce);
            void addToQueue(Signals signal, bool success, bool onlyOnce);
            void emitQueue();

            struct QueueSignal
            {
                void* data;
                Signals signal;
            };
            std::vector< QueueSignal> m_signalQueue;

            JDManager& m_manager;
            std::mutex& m_mutex;
            std::mutex m_signalsMutex;
        };
    }
}