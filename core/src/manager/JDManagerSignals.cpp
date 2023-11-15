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
            , lockedObjectsChanged("LockedObjectsChanged")
            , objectRemovedFromDatabase("RemovedFromDatabase")
            , objectAddedToDatabase("AddedToDatabase")
            , objectOverrideChangeFromDatabase("OverrideChangeFromDatabase")
            , objectChangedFromDatabase("ChangedFromDatabase")
            , databaseOutdated("DatabaseOutdated")

            , onLoadObjectDone("onLoadObjectDone")
            , onLoadObjectsDone("onLoadObjectsDone")
            , onSaveObjectDone("onSaveObjectDone")
            , onSaveObjectsDone("onSaveObjectsDone")
        { }

        /*
          -----------------------------------------------------------------------------------------------
          ------------------ S I G N A L   D E F I N I T I O N S ----------------------------------------
          -----------------------------------------------------------------------------------------------
        */

        DEFINE_SIGNAL_CONNECT_DISCONNECT(databaseFileChanged, )
        DEFINE_SIGNAL_CONNECT_DISCONNECT(lockedObjectsChanged, )
        DEFINE_SIGNAL_CONNECT_DISCONNECT(objectRemovedFromDatabase, const JDObjectContainer&)
        DEFINE_SIGNAL_CONNECT_DISCONNECT(objectAddedToDatabase, const JDObjectContainer&)
        DEFINE_SIGNAL_CONNECT_DISCONNECT(objectChangedFromDatabase, const std::vector<JDObjectPair>&)
        DEFINE_SIGNAL_CONNECT_DISCONNECT(objectOverrideChangeFromDatabase, const JDObjectContainer&)
        DEFINE_SIGNAL_CONNECT_DISCONNECT(databaseOutdated, )

        DEFINE_SIGNAL_CONNECT_DISCONNECT(onLoadObjectDone,  bool, JDObjectInterface*)
      
        DEFINE_SIGNAL_CONNECT_DISCONNECT(onLoadObjectsDone, bool)
        DEFINE_SIGNAL_CONNECT_DISCONNECT(onSaveObjectDone,  bool, JDObjectInterface*)
        DEFINE_SIGNAL_CONNECT_DISCONNECT(onSaveObjectsDone, bool)

        /*
          -----------------------------------------------------------------------------------------------
          ------------------ S I G N A L   D E F I N I T I O N S ----------------------------------------
          -----------------------------------------------------------------------------------------------
        */

        JDManagerSignals::ContainerSignal::ContainerSignal(const std::string& name)
            : signal(name) {}
        void JDManagerSignals::ContainerSignal::reserve(size_t size)
        {
            JDM_UNIQUE_LOCK;
            container.reserve(size);
        }
        void JDManagerSignals::ContainerSignal::addObjs(const std::vector<JDObjectInterface*>& objs)
        {
            JDM_UNIQUE_LOCK;
            container.addObject(objs);
        }
        void JDManagerSignals::ContainerSignal::addObj(JDObjectInterface* obj)
        {
            JDM_UNIQUE_LOCK;
            container.addObject(obj);
        }
        void JDManagerSignals::ContainerSignal::clear()
        {
            JDM_UNIQUE_LOCK;
			container.clear();
        }
        void JDManagerSignals::ContainerSignal::emitSignalIfNotEmpty()
        {
            JDObjectContainer cpy;
            {
                JDM_UNIQUE_LOCK;
                cpy = container;
                container.clear();
            }
            if (cpy.size() == 0)
                return;
            signal.emitSignal(cpy);
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
        void JDManagerSignals::ObjectChangeSignal::reserve(size_t size)
        {
            JDM_UNIQUE_LOCK;
			container.reserve(size);
        }
        void JDManagerSignals::ObjectChangeSignal::addPairs(const std::vector<JDObjectPair>& pairs)
        {
            JDM_UNIQUE_LOCK;
            container.insert(container.end(), pairs.begin(), pairs.end());
        }
        void JDManagerSignals::ObjectChangeSignal::addPair(const JDObjectPair& pair)
        {
            JDM_UNIQUE_LOCK;
			container.push_back(pair);
        }
        void JDManagerSignals::ObjectChangeSignal::clear()
        {
            JDM_UNIQUE_LOCK;
            container.clear();
        }
        void JDManagerSignals::ObjectChangeSignal::emitSignalIfNotEmpty()
        {
            std::vector<JDObjectPair> cpy;
            {
				JDM_UNIQUE_LOCK;
				cpy = container;
                container.clear();
			}
            if (cpy.size() == 0)
                return;
            signal.emitSignal(cpy);
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
            
            objectRemovedFromDatabase.clear();
            objectAddedToDatabase.clear();
            objectOverrideChangeFromDatabase.clear();
            objectChangedFromDatabase.clear();
        }
        void JDManagerSignals::emitIfNotEmpty()
        {
            
            objectRemovedFromDatabase.emitSignalIfNotEmpty();
            objectOverrideChangeFromDatabase.emitSignalIfNotEmpty();
            objectAddedToDatabase.emitSignalIfNotEmpty();
            objectChangedFromDatabase.emitSignalIfNotEmpty();
        }

        void JDManagerSignals::addToQueue(Signals signal, bool onlyOnce)
        {
            JDM_UNIQUE_LOCK_P_M(m_signalsMutex);
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
            JDM_UNIQUE_LOCK_P_M(m_signalsMutex);
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
                case Signals::signal_onLoadObjectDone:
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
                case Signals::signal_onSaveObjectDone:
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
            JDM_UNIQUE_LOCK_P_M(m_signalsMutex);
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
                case Signals::signal_onLoadObjectsDone:
                {
                    AsyncLoadObjectsData* data = new AsyncLoadObjectsData();
                    data->success = success;
                    QueueSignal sig;
                    sig.data = data;
                    sig.signal = signal;
                    m_signalQueue.push_back(sig);
                    break;
                }
                case Signals::signal_onSaveObjectsDone:
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
                    JDM_UNIQUE_LOCK_P_M(m_signalsMutex);
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
                    case signal_lockedObjectsChanged:
                        lockedObjectsChanged.emitSignal();
                        break;
                    case signal_objectRemovedFromDatabase:
                        objectRemovedFromDatabase.emitSignalIfNotEmpty();
                        break;
                    case signal_objectAddedToDatabase:
                        objectAddedToDatabase.emitSignalIfNotEmpty();
                        break;
                    case signal_objectChangedFromDatabase:
                        objectChangedFromDatabase.emitSignalIfNotEmpty();
                        break;
                    case signal_objectOverrideChangeFromDatabase:
                        objectOverrideChangeFromDatabase.emitSignalIfNotEmpty();
                        break;
                    case signal_databaseOutdated:
                        databaseOutdated.emitSignal();
                        break;
                    case signal_onLoadObjectDone:
                    {
                        AsyncLoadObjectData* data = (AsyncLoadObjectData*)signal.data;
                        onLoadObjectDone.emitSignal(data->success, data->object);
                        delete data;
                        break;
                    }
                    case signal_onLoadObjectsDone:
                    {
                        AsyncLoadObjectsData* data = (AsyncLoadObjectsData*)signal.data;
                        onLoadObjectsDone.emitSignal(data->success);
                        delete data;
                        break;
                    }
                    case signal_onSaveObjectDone:
                    {
                        AsyncSaveObjectData* data = (AsyncSaveObjectData*)signal.data;
                        onSaveObjectDone.emitSignal(data->success, data->object);
                        delete data;
                        break;
                    }
                    case signal_onSaveObjectsDone:
                    {
                        AsyncSaveObjectsData* data = (AsyncSaveObjectsData*)signal.data;
                        onSaveObjectsDone.emitSignal(data->success);
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