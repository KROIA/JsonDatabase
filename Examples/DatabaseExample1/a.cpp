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



#ifdef JD_USE_QJSON
bool A::load(const QJsonObject& obj)
#else
bool A::load(const JsonDatabase::JsonObject& obj)
#endif
{
    qDebug() << "Loading A";
    int value;
    getJsonValue(obj,value,"AValue");
    if(value != 1)
        qDebug() << "AValue is: "<<value;
    return true;
}
#ifdef JD_USE_QJSON
bool A::save(QJsonObject& obj) const
#else
bool A::save(JsonDatabase::JsonObject& obj) const
#endif
{
    qDebug() << "Saving A";
    obj["AValue"] = 1;
    return true;
}
