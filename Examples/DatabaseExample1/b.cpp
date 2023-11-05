#include "b.h"
#include <QDebug>

JD_OBJECT_IMPL(B);
B::B()
  : JDObjectInterface()
{}
B::B(const B &other)
: JDObjectInterface(other)
{}
B::~B()
{

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
