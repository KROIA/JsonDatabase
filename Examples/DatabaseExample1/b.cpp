#include "b.h"
#include <QDebug>

B::B()
  : JDObjectInterface()
{}
B::B(const std::string &id)
  : JDObjectInterface()
{ objID = id;}
B::B(const B &other)
: JDObjectInterface(other)
{}
B::~B()
{

}


std::string B::getObjectID() const
{
    return objID;
}
void B::setObjectID(const std::string &id)
{
    objID = id;
}

bool B::load(const QJsonObject &obj)
{
    qDebug() << "Loading B";
    int value;
    getJsonValue(obj,value,"BValue");
    if(value != 2)
        qDebug() << "BValue is: "<<value;
    return true;
}
bool B::save(QJsonObject &obj) const
{
    qDebug() << "Saving B";
    obj["BValue"] = 2;
    return true;
}
