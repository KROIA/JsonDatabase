#include "a.h"
#include <QDebug>

A::A()
  : JDObjectInterface()
{}
A::A(const std::string &id)
  : JDObjectInterface()
{ objID = id;}
A::A(const A &other)
: JDObjectInterface(other)
{}
A::~A()
{

}


std::string A::getObjectID() const
{
    return objID;
}
void A::setObjectID(const std::string &id)
{
    objID = id;
}

bool A::load(const QJsonObject &obj)
{
    qDebug() << "Loading A";
    int value;
    getJsonValue(obj,value,"AValue");
    if(value != 1)
        qDebug() << "AValue is: "<<value;
    return true;
}
bool A::save(QJsonObject &obj) const
{
    qDebug() << "Saving A";
    obj["AValue"] = 1;
    return true;
}
