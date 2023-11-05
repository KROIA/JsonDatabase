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
