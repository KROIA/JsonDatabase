#include "a.h"
#include <QDebug>


JD_OBJECT_IMPL(A);
A::A()
  : JDObjectInterface()
{}
A::A(const A &other)
: JDObjectInterface(other)
{}
A::~A()
{

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
