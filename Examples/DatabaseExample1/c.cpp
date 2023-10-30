#include "c.h"
#include <QDebug>

C::C()
  : JDObjectInterface()
{}
C::C(const std::string &id)
  : JDObjectInterface()
{ objID = id;}
C::C(const C &other)
: JDObjectInterface(other)
{}
C::~C()
{

}


std::string C::getObjectID() const
{
    return objID;
}
void C::setObjectID(const std::string &id)
{
    objID = id;
}

bool C::load(const QJsonObject &obj)
{
    qDebug() << "Loading C";
    int value;
    getJsonValue(obj,value,"CValue");
    if(value != 3)
        qDebug() << "CValue is: "<<value;
    return true;
}
bool C::save(QJsonObject &obj) const
{
    qDebug() << "Saving C";
    obj["CValue"] = 3;
    return true;
}
