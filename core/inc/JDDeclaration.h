#pragma once
#include "JD_base.h"

namespace JsonDatabase
{
    enum LoadMode
    {
        newObjects = 1,
        changedObjects = 2,
        removedObjects = 4,
        allObjects = 7,

        overrideChanges = 8,
    };
    
    class JDManager;
    class JDSerializable;
    class JDObjectInterface;

    typedef std::shared_ptr<JDObjectInterface> JDObject;
    // Pair of JDObjectInterface objects which have the same ID
    typedef std::pair<JDObject, JDObject> JDObjectPair;
    // Create a template alias for derived classes of Object
    template<typename T>
    using JDderivedObject = std::enable_if_t<std::is_base_of<JDObjectInterface, T>::value, std::shared_ptr<T>>;


    class JDObjectID;
    using JDObjectIDptr = std::shared_ptr<JDObjectID>;


    class JDObjectIDDomainInterface;
    class JDObjectIDDomain;

    

    
    
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

        class JDObjectContainer;
        class JDObjectManager;
        enum Lockstate
        {
            locked,
            unlocked
        };
        enum ChangeState
        {
			changed,
			unchanged
		};

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
