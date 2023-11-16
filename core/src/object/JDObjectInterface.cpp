#include "object/JDObjectInterface.h"
#include "object/JDObjectRegistry.h"
#include <QVariant>


namespace JsonDatabase
{

const QString JDObjectInterface::s_tag_objID = "objID";
//const QString JDObjectInterface::s_tag_objVersion = "objVersion";
const QString JDObjectInterface::s_tag_className = "class";
const QString JDObjectInterface::s_tag_data = "Data";


JDObjectInterface::AutoObjectAddToRegistry::AutoObjectAddToRegistry(JDObjectInterface* obj)
{
    addToRegistry(obj);
}
int JDObjectInterface::AutoObjectAddToRegistry::addToRegistry(JDObjectInterface* obj)
{
    return JDObjectRegistry::registerType(obj);
}


JDObjectInterface::JDObjectInterface()
    : m_objID(0)
    //, m_version(0)
{

}
JDObjectInterface::JDObjectInterface(const JDObjectID& id)
    : m_objID(id)
   // , m_version(0)
{

}
JDObjectInterface::JDObjectInterface(const JDObjectInterface &other)
    : m_objID(other.m_objID)
   // , m_version(other.m_version)
{

}
JDObjectInterface::~JDObjectInterface()
{

}

std::vector<JDObjectInterface*> JDObjectInterface::reinstantiate(const std::vector<JDObjectInterface*>& objList)
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_4);
    std::vector<JDObjectInterface*> ret;
	ret.reserve(objList.size());
    for (auto it = objList.begin(); it != objList.end(); ++it)
    {
		ret.push_back((*it)->clone());
	}
	return ret;
}
size_t JDObjectInterface::getJsonIndexByID(const std::vector<QJsonObject>& jsons, const JDObjectID &objID)
{
    for (size_t i = 0; i < jsons.size(); ++i)
    {
        JDObjectID id;
        if (JDSerializable::getJsonValue(jsons[i], id, s_tag_objID))
        {
            if (id == objID)
                return i;
        }
    }
    return std::string::npos;
}

const JDObjectID &JDObjectInterface::getObjectID() const
{
    return m_objID;
}
void JDObjectInterface::setObjectID(const JDObjectID& id)
{
    m_objID = id;
}

/*void JDObjectInterface::setVersion(int version)
{
	m_version = version;
}
void JDObjectInterface::setVersion(const QJsonObject& obj)
{
	if(obj.contains(s_tag_objVersion))
		m_version = obj[s_tag_objVersion].toInt(0);
}*/
bool JDObjectInterface::equalData(const QJsonObject& obj) const
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_4);
    QJsonObject data1;
    QJsonObject data2;
    bool equal = getJsonValue(obj, data1, s_tag_data);
    {
        JD_GENERAL_PROFILING_BLOCK("user save", JD_COLOR_STAGE_5);
        equal &= save(data2);
    }
    {
        JD_GENERAL_PROFILING_BLOCK("UserEqual", JD_COLOR_STAGE_5);
        equal &= data1 == data2;
    }

    return equal;
}
bool JDObjectInterface::loadInternal(const QJsonObject &obj)
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_4);
    
    
    if(!obj.contains(s_tag_objID) /* ||
       !obj.contains(s_tag_objVersion)*/)
        return false;
    QJsonObject data;
    bool success = getJsonValue(obj, data, s_tag_data);
    
    {
        JD_GENERAL_PROFILING_BLOCK("UserLoad", JD_COLOR_STAGE_5);
        success &= load(data);
    }
    setObjectID(obj[s_tag_objID].toInt());
    //m_version = obj[s_tag_objVersion].toInt(0);

   // if(m_version <= 0)
   //     success = false;
    return success;
}


bool JDObjectInterface::saveInternal(QJsonObject &obj)
{
    //++m_version;
    return getSaveData(obj);
}
bool JDObjectInterface::getSaveData(QJsonObject& obj) const
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_4);
    obj[s_tag_objID] = getObjectID().get();
    //obj[s_tag_objVersion] = QJsonValue(m_version);
    obj[s_tag_className] = className().c_str();
    QJsonObject data;
    bool ret;
    {
        JD_GENERAL_PROFILING_BLOCK("UserSave", JD_COLOR_STAGE_5);
        ret = save(data);
    }

    obj[s_tag_data] = data;
    return ret;
}
/*void JDObjectInterface::incrementVersionValue()
{
    ++m_version;
}*/

}
