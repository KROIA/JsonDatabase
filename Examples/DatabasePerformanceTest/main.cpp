#include <QCoreApplication>
#include <iostream>
#include "Person.h"


#define CONCURENT_TEST

#ifdef JD_PROFILING
#include "easy/profiler.h"
#endif

#include <atomic>
#include <qmutex.h>
QMutex mutex;


#ifdef CONCURENT_TEST
#include <thread>
#include <chrono>

#define USE_LOADS_SAVES
#define THREAD_END_SECONDS 4
#define USE_ZIP_FORMAT false


void threadFunction1();
void threadFunction2();
void threadFunction3();
void threadFunction4();
void threadFunction5();

void collisionChecker();

JDManager* manager1 = nullptr;
JDManager* manager2 = nullptr;
JDManager* manager3 = nullptr;
JDManager* manager4 = nullptr;
JDManager* manager5 = nullptr;
#endif




using namespace JsonDatabase;

std::vector<JDObjectInterface*> globalTable;

bool compareTables(const std::vector<JDObjectInterface*>& t1, const std::vector<JDObjectInterface*>& t2);
bool lockRandomPerson(JDManager *manager, JDObjectInterface*& obj);
bool unlockPerson(JDManager* manager, JDObjectInterface*& obj);

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);

    globalTable = createPersons();

#ifdef CONCURENT_TEST
    JDManager::startProfiler();

    manager1 = new JDManager("database", "Persons", "sessionID1", "USER 1");
    manager2 = new JDManager("database", "Persons", "sessionID2", "USER 2");
    manager3 = new JDManager("database", "Persons", "sessionID3", "USER 3");
    manager4 = new JDManager("database", "Persons", "sessionID4", "USER 4");
    manager5 = new JDManager("database", "Persons", "sessionID5", "USER 5");

    //manager1->addObjectDefinition<Person>();
    //manager2->addObjectDefinition<Person>();
    //manager3->addObjectDefinition<Person>();
    //manager4->addObjectDefinition<Person>();
    //manager5->addObjectDefinition<Person>();

    manager1->enableZipFormat(USE_ZIP_FORMAT);
    manager2->enableZipFormat(USE_ZIP_FORMAT);
    manager3->enableZipFormat(USE_ZIP_FORMAT);
    manager4->enableZipFormat(USE_ZIP_FORMAT);
    manager5->enableZipFormat(USE_ZIP_FORMAT);

    manager1->addObject(globalTable);
    manager1->saveObjects();

    manager1->enableZipFormat(false);
    manager1->setDatabaseName("secondPersons");
    manager1->saveObjects();

    manager1->enableZipFormat(USE_ZIP_FORMAT);
    manager1->setDatabaseName("Persons");

    

    // Create and start the first thread
    std::thread t1(threadFunction1);
   std::thread t2(threadFunction2);
   std::thread t3(threadFunction3);
   std::thread t4(threadFunction4);
   std::thread t5(threadFunction5);
   
   std::thread t6(collisionChecker);


    // Wait for the threads to finish for up to 10 seconds
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    t6.join();

   
    manager2->loadObjects();
    qDebug() << "Tables equal: " << compareTables(globalTable, manager2->getObjects());
    manager2->enableZipFormat(false);
    manager2->setDatabaseName("secondPersons");
    manager2->saveObjects();
    std::cout << "Finish";

    


    delete manager5;
    delete manager4;
    delete manager3;
    delete manager2;
    delete manager1;

#else
    JDManager manager("database", "Persons", "sessionID", "USER");

    manager.addObject(globalTable);

    manager.saveObjects();
    manager.loadObjects();


    qDebug() << "Objects loaded: " << manager.getObjectCount();
    qDebug() << "Tables equal: " << compareTables(createPersons(), manager.getObjects());

#endif
    JDManager::stopProfiler("Profile.prof");
    

    return a.exec();
}



bool compareTables(const std::vector<JDObjectInterface*>& t1, const std::vector<JDObjectInterface*>& t2)
{
    if (t1.size() != t2.size())
        return false;

    size_t matchCount = 0;
    for (size_t i = 0; i < t1.size(); ++i)
    {
        Person* p1 = dynamic_cast<Person*>(t1[i]);
        if (!p1)
            return false;
        for (size_t j = 0; j < t2.size(); ++j)
        {
            Person* p2 = dynamic_cast<Person*>(t2[j]);
            if (!p2)
                return false;

            if (*p1 == *p2)
            {
                matchCount++;
            }
        }
    }

    if (matchCount != t1.size())
        return false;

    return true;
}
bool lockRandomPerson(JDManager* manager, JDObjectInterface*& obj)
{
    mutex.lock();
    if (obj == nullptr)
    {
        //int randomIndex = rand() % globalTable.size();
        int randomIndex = 100;
        JDObjectInterface* target = globalTable[randomIndex];
        if (manager->lockObj(target))
        {
            obj = target;
            mutex.unlock();
            return true;
        }
    }
    mutex.unlock();
    return false;
}
bool unlockPerson(JDManager* manager, JDObjectInterface*& obj)
{
    mutex.lock();
    if (obj != nullptr)
    {
        if (manager->unlockObj(obj))
        {
            obj = nullptr;
            mutex.unlock();
            return true;
        }
    }
    mutex.unlock();
    return false;
}

#ifdef CONCURENT_TEST
// Function for the first thread
void threadFunction1() {
    auto start = std::chrono::high_resolution_clock::now();
    bool hasLocked = false;
    JDObjectInterface* lockedPerson = nullptr;

    while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count() < THREAD_END_SECONDS) {
        std::cout << "Thread 1 is running..." << std::endl;

#ifdef USE_LOADS_SAVES
        for(int i=0; i<20; ++i)
        manager1->loadObjects();
        std::this_thread::sleep_for(std::chrono::milliseconds(4)); // Simulate some work
        manager1->saveObjects();
#endif
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        if (rand() % 10 == 0)
        {
            if (hasLocked)
            {
                JDObjectInterface* wasLocked = lockedPerson;
                hasLocked = !unlockPerson(manager1, lockedPerson);
                if (!hasLocked)
                    std::cout << "Unlocked: " << wasLocked->getObjectID();

            }
            else
            {
                hasLocked = lockRandomPerson(manager1, lockedPerson);
                if (hasLocked)
                    std::cout << "Locked: " << lockedPerson->getObjectID();
            }
        }
    }
}

// Function for the second thread
void threadFunction2() {
    JDObjectInterface* obj = globalTable[0];
    bool hasLocked = false;
    JDObjectInterface* lockedPerson = nullptr;
    auto start = std::chrono::high_resolution_clock::now();
    while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count() < THREAD_END_SECONDS) {
        std::cout << "Thread 2 is running..." << std::endl;

#ifdef USE_LOADS_SAVES
        for (int i = 0; i < 20; ++i)
        manager2->loadObjects();
        std::this_thread::sleep_for(std::chrono::milliseconds(4)); // Simulate some work
        manager2->saveObjects();
#endif
        //std::this_thread::sleep_for(std::chrono::milliseconds(1));
        //bool wasInLoop = false;
        if (rand() % 10 == 0)
        {
            //wasInLoop = true;
            //EASY_NONSCOPED_BLOCK("test", profiler::colors::DeepPurple900);
            if (hasLocked)
            {
                JDObjectInterface* wasLocked = lockedPerson;
                hasLocked = !unlockPerson(manager1, lockedPerson);
                if (!hasLocked)
                    std::cout << "Unlocked: " << wasLocked->getObjectID()<< "\n";

            }
            else
            {
                hasLocked = lockRandomPerson(manager1, lockedPerson);
                if (hasLocked)
                    std::cout << "Locked: " << lockedPerson->getObjectID()<< "\n";
            }

            
            
        }
        //std::this_thread::sleep_for(std::chrono::milliseconds(10));
        /*if (wasInLoop)
        {
            EASY_END_BLOCK;
        }*/
    }
}

void threadFunction3() {
    auto start = std::chrono::high_resolution_clock::now();
    bool hasLocked = false;
    JDObjectInterface* lockedPerson = nullptr;
    while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count() < THREAD_END_SECONDS) {
        std::cout << "Thread 3 is running..." << std::endl;

#ifdef USE_LOADS_SAVES
        for (int i = 0; i < 20; ++i)
        manager3->loadObjects();
        std::this_thread::sleep_for(std::chrono::milliseconds(4)); // Simulate some work
        manager3->saveObjects();
#endif
       // std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if (rand() % 10 == 0)
        {
            if (hasLocked)
            {
                JDObjectInterface* wasLocked = lockedPerson;
                hasLocked = !unlockPerson(manager1, lockedPerson);
                if (!hasLocked)
                    std::cout << "Unlocked: " << wasLocked->getObjectID();

            }
            else
            {
                hasLocked = lockRandomPerson(manager1, lockedPerson);
                if (hasLocked)
                    std::cout << "Locked: " << lockedPerson->getObjectID();
            }
        }
    }
}

void threadFunction4() {
    auto start = std::chrono::high_resolution_clock::now();
    bool hasLocked = false;
    JDObjectInterface* lockedPerson = nullptr;
    while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count() < THREAD_END_SECONDS) {
        std::cout << "Thread 4 is running..." << std::endl;

#ifdef USE_LOADS_SAVES
        for (int i = 0; i < 20; ++i)
        manager4->loadObjects();
        std::this_thread::sleep_for(std::chrono::milliseconds(4)); // Simulate some work
        manager4->saveObjects();
#endif
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        if (rand() % 5 == 0)
        {
            if (hasLocked)
            {
                JDObjectInterface* wasLocked = lockedPerson;
                hasLocked = !unlockPerson(manager1, lockedPerson);
                if (!hasLocked)
                    std::cout << "Unlocked: " << wasLocked->getObjectID() << "\n";

            }
            else
            {
                hasLocked = lockRandomPerson(manager1, lockedPerson);
                if (hasLocked)
                    std::cout << "Locked: " << lockedPerson->getObjectID() << "\n";
            }
        }
    }
}

void threadFunction5() {
    auto start = std::chrono::high_resolution_clock::now();
    bool hasLocked = false;
    JDObjectInterface* lockedPerson = nullptr;
    while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count() < THREAD_END_SECONDS) {
        std::cout << "Thread 5 is running..." << std::endl;

#ifdef USE_LOADS_SAVES
        for (int i = 0; i < 20; ++i)
        manager5->loadObjects();
        std::this_thread::sleep_for(std::chrono::milliseconds(4)); // Simulate some work
        manager5->saveObjects();
#endif
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        if (rand() % 5 == 0)
        {
            if (hasLocked)
            {
                JDObjectInterface* wasLocked = lockedPerson;
                hasLocked = !unlockPerson(manager1, lockedPerson);
                if (!hasLocked)
                    std::cout << "Unlocked: " << wasLocked->getObjectID() << "\n";

            }
            else
            {
                hasLocked = lockRandomPerson(manager1, lockedPerson);
                if (hasLocked)
                    std::cout << "Locked: " << lockedPerson->getObjectID() << "\n";
            }
        }
    }
}




void collisionChecker() {
    auto start = std::chrono::high_resolution_clock::now();
    bool hasLocked = false;
    JDObjectInterface* lockedPerson = nullptr;
    while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count() < THREAD_END_SECONDS+1) {
        
        std::vector<std::string>  files = FileReadWriteLock::getFileNamesInDirectory("database", ".clk");
        size_t wCount = 0;
        size_t rCount = 0;
        for (const std::string& file : files)
        {
            if (file.find(".clk") == std::string::npos)
                continue;

            // Check the filename to see if it matches the file we want to lock
            size_t pos = file.find_last_of("_");
            if (pos == std::string::npos)
                continue;
            std::string fileName = file.substr(0, pos);

            if (fileName != "Persons")
                continue;

            // Check the access type
            size_t pos2 = file.find_last_of("-");
            std::string accessType = file.substr(pos + 1, pos2 - pos - 1);
            FileReadWriteLock::Access access = FileReadWriteLock::stringToAccessType(accessType);
            switch (access)
            {
            case FileReadWriteLock::Access::readWrite:
            case FileReadWriteLock::Access::write:
            {
                // Already locked for writing by a other process
                ++wCount;
                break;
            }
            case FileReadWriteLock::Access::read:
            {
                ++rCount;
                break;
            }
            case FileReadWriteLock::Access::unknown:
            {
                int a = 0;

            }
            }
        }

        if (wCount > 1)
        {
            int a = 0;
            std::cout << "Collision detected\n";
        }
        if (rCount > 0 && wCount > 0)
        {
            std::cout << "Collision detected reader and writer\n";
        }

        


    }
}

#endif