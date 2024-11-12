#include <QCoreApplication>
#include "JsonDatabase.h"
#include <QDebug>
#include <QObject>
#include "a.h"
#include "b.h"
#include "c.h"

using namespace JsonDatabase;

bool saveDone = false;

void onSaveCallback(bool success)
{
	qDebug() << "Save finished: " << success;
    saveDone = true;
}

Log::LogObject logger("Main");

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);


    JDManager manager;
    manager.setup("database", "MyData", "USER");
    //JDManager manager("C:\\Users\\alexk\\Documents\\Privat\\Softwareentwicklung\\QT\\Projekte\\JsonDatabase\\example\\bin\\database","sessionID","USER");
    //manager.addObjectDefinition<A>();
    //manager.addObjectDefinition<B>();
    //manager.addObjectDefinition<C>();
    JDObject a1(new A());
    JDObject a2(new A());

    JDObject b1(new B());
    JDObject b2(new B());

    JDObject c1(new C());
    JDObject c2(new C());

    QObject::connect(&manager, &JDManager::onSaveObjectsDone, [](bool s) {onSaveCallback(s); });


    manager.addObject(a1);
    manager.addObject(a2);
    manager.addObject(b2);
    manager.addObject(c1);
    manager.addObject(b1);
    manager.addObject(c2);
    manager.saveObjects();
    manager.loadObjects();
    //manager.setDatabasePath("C:\\Users\\alexk\\Documents\\Privat\\Softwareentwicklung\\QT\\Projekte\\JsonDatabase\\example\\bin\\database2");
    manager.setDatabasePath("database2");
    manager.saveObjectsAsync();

    while (!saveDone)
    {
        manager.update();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    

    JDManager manager2;
    manager2.setup("database", "MyData", "USER");
    //manager2.addObjectDefinition<A>();
    //manager2.addObjectDefinition<B>();
    //manager2.addObjectDefinition<C>();
    manager2.loadObjects();
    qDebug() << "Objects loaded: " << manager2.getObjectCount();
    std::vector<JDObject> objs1 = manager.getObjects();
    std::vector<JDObject> objs2 = manager2.getObjects();

    return a.exec();
}
