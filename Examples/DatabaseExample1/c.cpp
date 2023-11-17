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



#ifdef JD_USE_QJSON
bool C::load(const QJsonObject &obj)
#else
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
#ifdef JD_USE_QJSON
bool C::save(QJsonObject &obj) const
#else
bool C::save(JsonDatabase::JsonObject& obj) const
#endif
{
    qDebug() << "Saving C";
    obj["CValue"] = 3;
    return true;
}
