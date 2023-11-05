#include <QCoreApplication>
#include "JDManager.h"
#include "JDObjectInterface.h"
#include <QDebug>
#include "a.h"
#include "b.h"
#include "c.h"

using namespace JsonDatabase;


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);


    JDManager manager("database", "MyData", "sessionID", "USER");
    //JDManager manager("C:\\Users\\alexk\\Documents\\Privat\\Softwareentwicklung\\QT\\Projekte\\JsonDatabase\\example\\bin\\database","sessionID","USER");
    //manager.addObjectDefinition<A>();
    //manager.addObjectDefinition<B>();
    //manager.addObjectDefinition<C>();
    A* a1 = new A("a1");
    A* a2 = new A("a2");

    B* b1 = new B("b1");
    B* b2 = new B("b2");

    C* c1 = new C("c1");
    C* c2 = new C("c2");


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
    manager.saveObjects();

    

    JDManager manager2("database", "MyData", "sessionID2", "USER");
    //manager2.addObjectDefinition<A>();
    //manager2.addObjectDefinition<B>();
    //manager2.addObjectDefinition<C>();
    manager2.loadObjects();
    qDebug() << "Objects loaded: " << manager2.getObjectCount();
    std::vector<JDObjectInterface*> objs1 = manager.getObjects();
    std::vector<JDObjectInterface*> objs2 = manager2.getObjects();

    return a.exec();
}
