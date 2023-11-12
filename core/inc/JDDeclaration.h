#pragma once
#include "JD_base.h"

namespace JsonDatabase
{
    class JDObjectInterface;
    // Pair of JDObjectInterface objects which have the same ID
    typedef std::pair<JDObjectInterface*, JDObjectInterface*> JDObjectPair;

    class JDManager;
    class JDSerializable;
    class FileLock;

    class JDObjectContainer;
    
    namespace Internal
    {
        class FileChangeWatcher;
        class JsonUtilities;
    }
}
