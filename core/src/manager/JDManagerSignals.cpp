#include "manager/JDManager.h"

namespace JsonDatabase
{
    namespace Internal
    {
#define DEFINE_SIGNAL_CONNECT_DISCONNECT(signalName, ...) \
        void JDManagerSignals::connect_##signalName##_slot(const Signal<__VA_ARGS__>::SlotFunction& slotFunction) \
        { \
            ##signalName##.connectSlot(slotFunction); \
        } \
        void JDManagerSignals::disconnect_##signalName##_slot(const Signal<__VA_ARGS__>::SlotFunction& slotFunction) \
        { \
            ##signalName##.disconnectSlot(slotFunction); \
        }

        JDManagerSignals::JDManagerSignals()
            : databaseFileChanged("DatabaseFileChanged")
            , objectRemovedFromDatabase("RemovedFromDatabase")
            , objectAddedToDatabase("AddedToDatabase")
            , objectOverrideChangeFromDatabase("OverrideChangeFromDatabase")
            , objectChangedFromDatabase("ChangedFromDatabase")
            , databaseOutdated("DatabaseOutdated")
        { }

        /*
          -----------------------------------------------------------------------------------------------
          ------------------ S I G N A L   D E F I N I T I O N S ----------------------------------------
          -----------------------------------------------------------------------------------------------
        */

        DEFINE_SIGNAL_CONNECT_DISCONNECT(databaseFileChanged, )
        DEFINE_SIGNAL_CONNECT_DISCONNECT(objectRemovedFromDatabase, const JDObjectContainer&)
        DEFINE_SIGNAL_CONNECT_DISCONNECT(objectAddedToDatabase, const JDObjectContainer&)
        DEFINE_SIGNAL_CONNECT_DISCONNECT(objectChangedFromDatabase, const std::vector<JDObjectPair>&)
        DEFINE_SIGNAL_CONNECT_DISCONNECT(objectOverrideChangeFromDatabase, const JDObjectContainer&)


        /*
          -----------------------------------------------------------------------------------------------
          ------------------ S I G N A L   D E F I N I T I O N S ----------------------------------------
          -----------------------------------------------------------------------------------------------
        */

        JDManagerSignals::ContainerSignal::ContainerSignal(const std::string& name)
            : signal(name) {}
        void JDManagerSignals::ContainerSignal::emitSignalIfNotEmpty()
        {
            if (container.size() == 0)
                return;
            signal.emitSignal(container);
            container.clear();
        }
        void JDManagerSignals::ContainerSignal::connectSlot(const Signal<const JDObjectContainer&>::SlotFunction& slot)
        {
            signal.connectSlot(slot);
        }
        void JDManagerSignals::ContainerSignal::disconnectSlot(const Signal<const JDObjectContainer&>::SlotFunction& slot)
        {
            signal.disconnectSlot(slot);
        }



        JDManagerSignals::ObjectChangeSignal::ObjectChangeSignal(const std::string& name)
            : signal(name) {}
        void JDManagerSignals::ObjectChangeSignal::emitSignalIfNotEmpty()
        {
            if (container.size() == 0)
                return;
            signal.emitSignal(container);
            container.clear();
        }
        void JDManagerSignals::ObjectChangeSignal::connectSlot(const Signal<const std::vector<JDObjectPair>&>::SlotFunction& slot)
        {
            signal.connectSlot(slot);
        }
        void JDManagerSignals::ObjectChangeSignal::disconnectSlot(const Signal<const std::vector<JDObjectPair>&>::SlotFunction& slot)
        {
            signal.disconnectSlot(slot);
        }

        void JDManagerSignals::clearContainer()
        {
            objectRemovedFromDatabase.container.clear();
            objectAddedToDatabase.container.clear();
            objectOverrideChangeFromDatabase.container.clear();
            objectChangedFromDatabase.container.clear();
        }
        void JDManagerSignals::emitIfNotEmpty()
        {
            objectRemovedFromDatabase.emitSignalIfNotEmpty();
            objectAddedToDatabase.emitSignalIfNotEmpty();
            objectOverrideChangeFromDatabase.emitSignalIfNotEmpty();
            objectChangedFromDatabase.emitSignalIfNotEmpty();
        }

        void JDManagerSignals::addToQueue(Signals signal, bool onlyOnce)
        {
            if (onlyOnce)
            {
				if (std::find(m_signalQueue.begin(), m_signalQueue.end(), signal) != m_signalQueue.end())
					return;
			}
            m_signalQueue.push_back(signal);    
        }
        void JDManagerSignals::emitQueue()
        {
            // Copy the queue
            std::vector< Signals> signalQueue = m_signalQueue;
            m_signalQueue.clear();
            for (auto& signal : signalQueue)
            {
                switch (signal)
                {
				case signal_databaseFileChanged:
					databaseFileChanged.emitSignal();
					break;
				case signal_objectRemovedFromDatabase:
					objectRemovedFromDatabase.emitSignalIfNotEmpty();
                    objectRemovedFromDatabase.container.clear();
					break;
				case signal_objectAddedToDatabase:
					objectAddedToDatabase.emitSignalIfNotEmpty();
                    objectAddedToDatabase.container.clear();
					break;
				case signal_objectChangedFromDatabase:
					objectChangedFromDatabase.emitSignalIfNotEmpty();
                    objectChangedFromDatabase.container.clear();
					break;
				case signal_objectOverrideChangeFromDatabase:
					objectOverrideChangeFromDatabase.emitSignalIfNotEmpty();
                    objectOverrideChangeFromDatabase.container.clear();
					break;
				case signal_databaseOutdated:
					databaseOutdated.emitSignal();
					break;
				default:
					break;
				}
			}
        }
    }
}