#include "JDObjectInterface.h"
#include <QVariant>


namespace JsonDatabase
{

const QString JDObjectInterface::m_tag_objID = "objID";
const QString JDObjectInterface::m_tag_objVersion = "objVersion";
const QString JDObjectInterface::m_tag_className = "class";

JDObjectInterface::JDObjectInterface()
{
    m_version = 0;
}
JDObjectInterface::JDObjectInterface(const JDObjectInterface &other)
    : m_version(other.m_version)
{

}
JDObjectInterface::~JDObjectInterface()
{

}



bool JDObjectInterface::loadInternal(const QJsonObject &obj)
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_1)
    if(!obj.contains(m_tag_objID) ||
       !obj.contains(m_tag_objVersion))
        return false;
    QJsonObject data;
    bool success = getJsonValue(obj, data,  "Data");
    success &= load(data);
    setObjectID(obj[m_tag_objID].toString().toStdString());
    m_version = obj[m_tag_objVersion].toInt(0);

    if(m_version <= 0)
        success = false;
    return success;
}


bool JDObjectInterface::saveInternal(QJsonObject &obj)
{
    JD_PROFILING_FUNCTION(COLOR_STAGE_1)
    ++m_version;
    obj[m_tag_objID] = getObjectID().c_str();
    obj[m_tag_objVersion] = QJsonValue(m_version);
    obj[m_tag_className] = className().c_str();
    QJsonObject data;
    bool ret = save(data);
    obj["Data"] = data;
    return ret;
}


}
