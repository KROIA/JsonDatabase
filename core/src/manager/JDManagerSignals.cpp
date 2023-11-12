#include "manager/JDManager.h"
#include "utilities/JDUniqueMutexLock.h"

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

        JDManagerSignals::JDManagerSignals(JDManager& manager, std::mutex& mtx)
            : m_manager(manager)
            , m_mutex(mtx)
            , databaseFileChanged("DatabaseFileChanged")
            , objectRemovedFromDatabase("RemovedFromDatabase")
            , objectAddedToDatabase("AddedToDatabase")
            , objectOverrideChangeFromDatabase("OverrideChangeFromDatabase")
            , objectChangedFromDatabase("ChangedFromDatabase")
            , databaseOutdated("DatabaseOutdated")

            , loadObject("onLoadObjectDone")
            , loadObjects("onLoadObjectsDone")
            , saveObject("onSaveObjectDone")
            , saveObjects("onSaveObjectsDone")
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
        DEFINE_SIGNAL_CONNECT_DISCONNECT(databaseOutdated, )

        DEFINE_SIGNAL_CONNECT_DISCONNECT(loadObject,  bool, JDObjectInterface*)
        DEFINE_SIGNAL_CONNECT_DISCONNECT(loadObjects, bool)
        DEFINE_SIGNAL_CONNECT_DISCONNECT(saveObject,  bool, JDObjectInterface*)
        DEFINE_SIGNAL_CONNECT_DISCONNECT(saveObjects, bool)

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
            JDM_UNIQUE_LOCK_P;
            if (onlyOnce)
            {
                for (size_t i = 0; i < m_signalQueue.size(); ++i)
                {
                    if (m_signalQueue[i].signal == signal)
                        return;
                }
			}
            QueueSignal sig;
            sig.data = nullptr;
            sig.signal = signal;
            m_signalQueue.push_back(sig);
        }
        void JDManagerSignals::addToQueue(Signals signal, bool success, JDObjectInterface* obj, bool onlyOnce)
        {
            JDM_UNIQUE_LOCK_P;
            if (onlyOnce)
            {
                for (size_t i = 0; i < m_signalQueue.size(); ++i)
                {
                    if (m_signalQueue[i].signal == signal)
					return;
                }
            }
            switch (signal)
            {
                case Signals::signal_loadObject:
                {
                    AsyncLoadObjectData *data = new AsyncLoadObjectData();
                    data->success = success;
                    data->object = obj;
                    QueueSignal sig;
                    sig.data = data;
                    sig.signal = signal;
                    m_signalQueue.push_back(sig);
                    break;
                }
                case Signals::signal_saveObject:
                {
                    AsyncSaveObjectData* data = new AsyncSaveObjectData();
                    data->success = success;
                    data->object = obj;
                    QueueSignal sig;
                    sig.data = data;
                    sig.signal = signal;
                    m_signalQueue.push_back(sig);
                    break;
                }
            }
        }
        void JDManagerSignals::addToQueue(Signals signal, bool success, bool onlyOnce)
        {
            JDM_UNIQUE_LOCK_P;
            if (onlyOnce)
            {
                for (size_t i = 0; i < m_signalQueue.size(); ++i)
                {
                    if (m_signalQueue[i].signal == signal)
                        return;
                }
            }
            switch (signal)
            {
                case Signals::signal_loadObjects:
                {
                    AsyncLoadObjectsData* data = new AsyncLoadObjectsData();
                    data->success = success;
                    QueueSignal sig;
                    sig.data = data;
                    sig.signal = signal;
                    m_signalQueue.push_back(sig);
                    break;
                }
                case Signals::signal_saveObjects:
                {
                    AsyncSaveObjectsData* data = new AsyncSaveObjectsData();
                    data->success = success;
                    QueueSignal sig;
                    sig.data = data;
                    sig.signal = signal;
                    m_signalQueue.push_back(sig);
                    break;
                }
            }
        }
        void JDManagerSignals::emitQueue()
        {
            // Copy the queue
            std::vector< QueueSignal> signalQueue;
            bool hasWork = m_signalQueue.size();
            while (hasWork)
            {
                {
                    JDM_UNIQUE_LOCK_P;
                    signalQueue = m_signalQueue;
                    m_signalQueue.clear();
                }
                hasWork = signalQueue.size() > 0;
                if(!hasWork)
                    return;
                for (auto& signal : signalQueue)
                {
                    switch (signal.signal)
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
                    case signal_loadObject:
                    {
                        AsyncLoadObjectData* data = (AsyncLoadObjectData*)signal.data;
                        loadObject.emitSignal(data->success, data->object);
                        delete data;
                        break;
                    }
                    case signal_loadObjects:
                    {
                        AsyncLoadObjectsData* data = (AsyncLoadObjectsData*)signal.data;
                        loadObjects.emitSignal(data->success);
                        delete data;
                        break;
                    }
                    case signal_saveObject:
                    {
                        AsyncSaveObjectData* data = (AsyncSaveObjectData*)signal.data;
                        saveObject.emitSignal(data->success, data->object);
                        delete data;
                        break;
                    }
                    case signal_saveObjects:
                    {
                        AsyncSaveObjectsData* data = (AsyncSaveObjectsData*)signal.data;
                        saveObjects.emitSignal(data->success);
                        delete data;
                        break;
                    }
                    default:
                        break;
                    }
                }
            }
        }
    }
}