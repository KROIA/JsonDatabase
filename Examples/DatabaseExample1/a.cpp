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



#if JD_ACTIVE_JSON == JD_JSON_QT
bool A::load(const QJsonObject& obj)
#elif JD_ACTIVE_JSON == JD_JSON_GLAZE || JD_ACTIVE_JSON == JD_JSON_INTERNAL
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
#if JD_ACTIVE_JSON == JD_JSON_QT
bool A::save(QJsonObject& obj) const
#elif JD_ACTIVE_JSON == JD_JSON_GLAZE || JD_ACTIVE_JSON == JD_JSON_INTERNAL
bool A::save(JsonDatabase::JsonObject& obj) const
#endif
{
    qDebug() << "Saving A";
    obj["AValue"] = 1;
    return true;
}
