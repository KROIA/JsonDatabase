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



#ifdef JD_USE_QJSON
bool B::load(const QJsonObject& obj)
#else
bool B::load(const JsonDatabase::JsonObject& obj)
#endif
{
    qDebug() << "Loading B";
    int value;
    getJsonValue(obj,value,"BValue");
    if(value != 2)
        qDebug() << "BValue is: "<<value;
    return true;
}
#ifdef JD_USE_QJSON
bool B::save(QJsonObject& obj) const
#else
bool B::save(JsonDatabase::JsonObject& obj) const
#endif
{
    qDebug() << "Saving B";
    obj["BValue"] = 2;
    return true;
}
