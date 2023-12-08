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



#if JD_ACTIVE_JSON == JD_JSON_QT
bool B::load(const QJsonObject& obj)
#elif JD_ACTIVE_JSON == JD_JSON_GLAZE || JD_ACTIVE_JSON == JD_JSON_INTERNAL
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
#if JD_ACTIVE_JSON == JD_JSON_QT
bool B::save(QJsonObject& obj) const
#elif JD_ACTIVE_JSON == JD_JSON_GLAZE || JD_ACTIVE_JSON == JD_JSON_INTERNAL
bool B::save(JsonDatabase::JsonObject& obj) const
#endif
{
    qDebug() << "Saving B";
    obj["BValue"] = 2;
    return true;
}
