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



#if JD_ACTIVE_JSON == JD_JSON_QT
bool C::load(const QJsonObject &obj)
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
bool C::load(const JsonDatabase::JsonObject& obj)
#endif
{
    qDebug() << "Loading C";
    int value;
    getJsonValue(obj,value,"CValue");
    if(value != 3)
        qDebug() << "CValue is: "<<value;
    return true;
}
#if JD_ACTIVE_JSON == JD_JSON_QT
bool C::save(QJsonObject &obj) const
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
bool C::save(JsonDatabase::JsonObject& obj) const
#endif
{
    qDebug() << "Saving C";
    obj["CValue"] = 3;
    return true;
}
