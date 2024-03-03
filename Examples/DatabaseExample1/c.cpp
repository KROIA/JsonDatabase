#include "c.h"
#include <QDebug>

JD_OBJECT_IMPL(C);
C::C()
  : JDObjectInterface()
{}
C::C(const C &other)
: JDObjectInterface(other)
{}
C::~C()
{

}



bool C::load(const JsonDatabase::JsonObject& obj)
{
    qDebug() << "Loading C";
    long value;
    getJsonValue(obj,value,"CValue");
    if(value != 3)
        qDebug() << "CValue is: "<<value;
    return true;
}
bool C::save(JsonDatabase::JsonObject& obj) const
{
    qDebug() << "Saving C";
    obj["CValue"] = long(3);
    return true;
}
