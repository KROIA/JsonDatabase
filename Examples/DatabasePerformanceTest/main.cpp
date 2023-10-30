#include <QCoreApplication>
#include <iostream>
#include "Person.h"


#define CONCURENT_TEST

#ifdef JD_PROFILING
#include "easy/profiler.h"
#endif


#ifdef CONCURENT_TEST
#include <thread>
#include <chrono>
void threadFunction1();
void threadFunction2();
void threadFunction3();
void threadFunction4();
void threadFunction5();

JDManager* manager1 = nullptr;
JDManager* manager2 = nullptr;
JDManager* manager3 = nullptr;
JDManager* manager4 = nullptr;
JDManager* manager5 = nullptr;
#endif




using namespace JsonDatabase;

std::vector<JDObjectInterface*> globalTable;

bool compareTables(const std::vector<JDObjectInterface*>& t1, const std::vector<JDObjectInterface*>& t2);

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);

    globalTable = createPersons();

#ifdef CONCURENT_TEST

    manager1 = new JDManager("database", "Persons", "sessionID", "USER 1");
    manager2 = new JDManager("database", "Persons", "sessionID", "USER 2");
    manager3 = new JDManager("database", "Persons", "sessionID", "USER 2");
    manager4 = new JDManager("database", "Persons", "sessionID", "USER 2");
    manager5 = new JDManager("database", "Persons", "sessionID", "USER 2");

    manager1->addObjectDefinition<Person>();
    manager2->addObjectDefinition<Person>();
    manager3->addObjectDefinition<Person>();
    manager4->addObjectDefinition<Person>();
    manager5->addObjectDefinition<Person>();

    manager1->addObject(globalTable);
    manager2->saveObjects();

    // Create and start the first thread
    std::thread t1(threadFunction1);
    std::thread t2(threadFunction2);
    std::thread t3(threadFunction3);
    std::thread t4(threadFunction4);
    std::thread t5(threadFunction5);

    // Wait for the threads to finish for up to 10 seconds
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();

   
    manager1->loadObjects();
    qDebug() << "Tables equal: " << compareTables(globalTable, manager1->getObjects());
    std::cout << "Finish";

    manager1->saveProfilerFile();


    delete manager5;
    delete manager4;
    delete manager3;
    delete manager2;
    delete manager1;

#else
    JDManager manager("database", "Persons", "sessionID", "USER");
    manager.addObjectDefinition<Person>();

    manager.addObject(globalTable);

    manager.saveObjects();
    manager.loadObjects();


    qDebug() << "Objects loaded: " << manager.getObjectCount();
    qDebug() << "Tables equal: " << compareTables(createPersons(), manager.getObjects());

    manager.saveProfilerFile();
#endif
    
    

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

#ifdef CONCURENT_TEST
// Function for the first thread
void threadFunction1() {
    auto start = std::chrono::high_resolution_clock::now();
    while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count() < 5) {
        std::cout << "Thread 1 is running..." << std::endl;

        manager1->loadObjects();
        std::this_thread::sleep_for(std::chrono::milliseconds(4)); // Simulate some work
        manager1->saveObjects();
    }
}

// Function for the second thread
void threadFunction2() {
    JDObjectInterface* obj = globalTable[0];
    auto start = std::chrono::high_resolution_clock::now();
    while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count() < 5) {
        std::cout << "Thread 2 is running..." << std::endl;

        manager2->loadObject(obj);
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Simulate some work
        manager2->saveObject(obj);
    }
}

void threadFunction3() {
    auto start = std::chrono::high_resolution_clock::now();
    while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count() < 5) {
        std::cout << "Thread 3 is running..." << std::endl;

        manager3->loadObjects();
        std::this_thread::sleep_for(std::chrono::milliseconds(5)); // Simulate some work
        manager3->saveObjects();
    }
}

void threadFunction4() {
    auto start = std::chrono::high_resolution_clock::now();
    while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count() < 5) {
        std::cout << "Thread 4 is running..." << std::endl;

        manager4->loadObjects();
        std::this_thread::sleep_for(std::chrono::milliseconds(20)); // Simulate some work
        manager4->saveObjects();
    }
}

void threadFunction5() {
    auto start = std::chrono::high_resolution_clock::now();
    while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count() < 5) {
        std::cout << "Thread 5 is running..." << std::endl;

        manager5->loadObjects();
        std::this_thread::sleep_for(std::chrono::milliseconds(15)); // Simulate some work
        manager5->saveObjects();
    }
}


#endif