#include "JDObjectInterface.h"
#include "JDObjectRegistry.h"
#include <QVariant>


namespace JsonDatabase
{

const QString JDObjectInterface::m_tag_objID = "i";
const QString JDObjectInterface::m_tag_objVersion = "o";
const QString JDObjectInterface::m_tag_className = "c";
const QString JDObjectInterface::m_tag_data = "D";


int JDObjectInterface::AutoObjectAddToRegistry::addToRegistry(JDObjectInterface* obj)
{
    return JDObjectRegistry::registerType(obj);
}


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
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_4);
    
    
    if(!obj.contains(m_tag_objID) ||
       !obj.contains(m_tag_objVersion))
        return false;
    QJsonObject data;
    bool success = getJsonValue(obj, data, m_tag_data);
    
    {
        JD_GENERAL_PROFILING_BLOCK("UserLoad", JD_COLOR_STAGE_5);
        success &= load(data);
    }
    setObjectID(obj[m_tag_objID].toString().toStdString());
    m_version = obj[m_tag_objVersion].toInt(0);

    if(m_version <= 0)
        success = false;
    return success;
}


bool JDObjectInterface::saveInternal(QJsonObject &obj)
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_4)
    ++m_version;
    obj[m_tag_objID] = getObjectID().c_str();
    obj[m_tag_objVersion] = QJsonValue(m_version);
    obj[m_tag_className] = className().c_str();
    QJsonObject data;
    bool ret;
    {
        JD_GENERAL_PROFILING_BLOCK("UserSave", JD_COLOR_STAGE_5);
        ret = save(data);
    }
     
    obj[m_tag_data] = data;
    return ret;
}


}
