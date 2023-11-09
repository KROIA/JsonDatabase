#include "JDObjectInterface.h"
#include "JDObjectRegistry.h"
#include <QVariant>


namespace JsonDatabase
{

const QString JDObjectInterface::m_tag_objID = "objID";
const QString JDObjectInterface::m_tag_objVersion = "objVersion";
const QString JDObjectInterface::m_tag_className = "class";
const QString JDObjectInterface::m_tag_data = "Data";


JDObjectInterface::AutoObjectAddToRegistry::AutoObjectAddToRegistry(JDObjectInterface* obj)
{
    addToRegistry(obj);
}
int JDObjectInterface::AutoObjectAddToRegistry::addToRegistry(JDObjectInterface* obj)
{
    return JDObjectRegistry::registerType(obj);
}


JDObjectInterface::JDObjectInterface()
    : m_objID("")
    , m_version(0)
{

}
JDObjectInterface::JDObjectInterface(const std::string& id)
    : m_objID(id)
    , m_version(0)
{

}
JDObjectInterface::JDObjectInterface(const JDObjectInterface &other)
    : m_version(other.m_version)
{

}
JDObjectInterface::~JDObjectInterface()
{

}


const std::string& JDObjectInterface::getObjectID() const
{
    return m_objID;
}
void JDObjectInterface::setObjectID(const std::string& id)
{
    m_objID = id;
}

void JDObjectInterface::setVersion(int version)
{
	m_version = version;
}
void JDObjectInterface::setVersion(const QJsonObject& obj)
{
	if(obj.contains(m_tag_objVersion))
		m_version = obj[m_tag_objVersion].toInt(0);
}
bool JDObjectInterface::equalData(const QJsonObject& obj) const
{
    QJsonObject data1;
    QJsonObject data2;
    bool equal = getJsonValue(obj, data1, m_tag_data);
    equal = save(data2);
    equal &= data1 == data2;

    return equal;
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
    ++m_version;
    return getSaveData(obj);
}
bool JDObjectInterface::getSaveData(QJsonObject& obj) const
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_4);
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
