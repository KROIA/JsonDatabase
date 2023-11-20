#pragma once
#include "JD_base.h"

namespace JsonDatabase
{
    
    
    class JDManager;
    class JDSerializable;
    class JDObjectInterface;
    class JDObjectID;

    class JDObjectContainer;

    // Pair of JDObjectInterface objects which have the same ID
    typedef std::pair<JDObjectInterface*, JDObjectInterface*> JDObjectPair;

    
    namespace Internal
    {
        class FileChangeWatcher;
        class JsonUtilities;
        class FileLock;
        class FileReadWriteLock;

        class JDManagerFileSystem;
        class JDManagerObjectManager;
        class JDManagerSignals;
        class JDObjectLocker;

        class JDManagerAsyncWorker;
        class JDManagerAysncWork;
        class WorkProgress;
        enum class WorkType
        {
            loadAllObjects,
            loadSingleObject,
            saveSingleObject,
            saveAllObjects
        };

        class JDManagerAysncWorkLoadAllObjects;
        class JDManagerAysncWorkLoadSingleObject;
        class JDManagerAysncWorkSaveSingle;
        class JDManagerAysncWorkSaveList;

    }
}
