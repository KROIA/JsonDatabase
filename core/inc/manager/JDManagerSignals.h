#pragma once

#include "JD_base.h"
#include "JDDeclaration.h"
#include "utilities/signal.h"
#include "object/JDObjectContainer.h"

#include <vector>


#define DECLARE_SIGNAL_CONNECT_DISCONNECT(signalName, ...) \
    void connect_##signalName##_slot(const Signal<__VA_ARGS__>::SlotFunction& slotFunction); \
    void disconnect_##signalName##_slot(const Signal<__VA_ARGS__>::SlotFunction& slotFunction);


namespace JsonDatabase
{
    namespace Internal
    {
        //class JDManager;
        class JSONDATABASE_EXPORT JDManagerSignals
        {
            friend class JDManager;
            JDManagerSignals();
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

        protected:
            
            struct ContainerSignal
            {
                Signal<const JDObjectContainer&> signal;
                JDObjectContainer container;

                ContainerSignal(const std::string& name);
                void emitSignalIfNotEmpty();

                void connectSlot(const Signal<const JDObjectContainer&>::SlotFunction& slot);
                void disconnectSlot(const Signal<const JDObjectContainer&>::SlotFunction& slot);
            };
            struct ObjectChangeSignal
            {
                Signal<const std::vector<JDObjectPair>&> signal;
                std::vector<JDObjectPair> container;

                ObjectChangeSignal(const std::string& name);
                void emitSignalIfNotEmpty();

                void connectSlot(const Signal<const std::vector<JDObjectPair>&>::SlotFunction& slot);
                void disconnectSlot(const Signal<const std::vector<JDObjectPair>&>::SlotFunction& slot);
            };

            Signal<> databaseFileChanged;
            ContainerSignal objectRemovedFromDatabase;
            ContainerSignal objectAddedToDatabase;
            ObjectChangeSignal objectChangedFromDatabase;
            ContainerSignal objectOverrideChangeFromDatabase;
            Signal<> databaseOutdated;

            enum Signals
            {
                signal_databaseFileChanged,
                signal_objectRemovedFromDatabase,
                signal_objectAddedToDatabase,
                signal_objectChangedFromDatabase,
                signal_objectOverrideChangeFromDatabase,
                signal_databaseOutdated,
            };

            
            void clearContainer();
            void emitIfNotEmpty();
            

            void addToQueue(Signals signal, bool onlyOnce = true);
            void emitQueue();
            std::vector< Signals> m_signalQueue;
        };
    }
}