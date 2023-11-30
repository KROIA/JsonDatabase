#include "object/JDObjectInterface.h"
#include "object/JDObjectRegistry.h"
#include "object/JDObjectManager.h"

#ifdef JD_USE_QJSON
#include <QVariant>
#endif

namespace JsonDatabase
{

#ifdef JD_USE_QJSON
    const QString JDObjectInterface::s_tag_objID = "objID";
    const QString JDObjectInterface::s_tag_className = "class";
    const QString JDObjectInterface::s_tag_data = "Data";
#else
    const std::string JDObjectInterface::s_tag_objID = "objID";
    const std::string JDObjectInterface::s_tag_className = "class";
    const std::string JDObjectInterface::s_tag_data = "Data";
#endif

JDObjectInterface::AutoObjectAddToRegistry::AutoObjectAddToRegistry(JDObject obj)
{
    addToRegistry(obj);
}
int JDObjectInterface::AutoObjectAddToRegistry::addToRegistry(JDObject obj)
{
    return Internal::JDObjectRegistry::registerType(obj);
}


JDObjectInterface::JDObjectInterface()
    : m_manager(nullptr)
    //, m_objID(nullptr)
  //  , m_onDelete("onDelete")
{

}
/*JDObjectInterface::JDObjectInterface(const JDObjectIDptr& id)
    : m_objID(id)
{

}*/
JDObjectInterface::JDObjectInterface(const JDObjectInterface &other)
    : m_manager(nullptr)
  //  , m_objID(other.m_objID)
   // , m_onDelete("onDelete")
{

}
JDObjectInterface::~JDObjectInterface()
{
 //   m_onDelete.emitSignal(this);
#ifdef JD_DEBUG
    if (JDObjectID::isValid(getObjectID()))
    {
        JD_CONSOLE("Delete object with ID: " << getObjectID()->get());
    }
    else
    {
        JD_CONSOLE("Delete object with ID: invalid");
    }
#endif
}

/*/
std::vector<JDObject> JDObjectInterface::reinstantiate(const std::vector<JDObject>& objList)
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_4);
    std::vector<JDObject> ret;
	ret.reserve(objList.size());
    for (auto it = objList.begin(); it != objList.end(); ++it)
    {
		ret.push_back((*it)->clone());
	}
	return ret;
}*/
#ifdef JD_USE_QJSON
size_t JDObjectInterface::getJsonIndexByID(const std::vector<QJsonObject>& jsons, const JDObjectIDptr&objID)
{
    for (size_t i = 0; i < jsons.size(); ++i)
    {
        int id;
        if (JDSerializable::getJsonValue(jsons[i], id, s_tag_objID))
        {
            if (id == objID->get())
                return i;
        }
    }
    return std::string::npos;
}
#else
size_t JDObjectInterface::getJsonIndexByID(const JsonArray& jsons, const JDObjectIDptr& objID)
{
    for (size_t i = 0; i < jsons.size(); ++i)
    {
        int id;
        if (jsons[i].getInt(id, s_tag_objID))
        {
            if (id == objID->get())
                return i;
        }
    }
    return std::string::npos;
}
#endif

bool JDObjectInterface::loadFrom(const JDObject& source)
{
    bool success = true;
#ifdef JD_USE_QJSON
    QJsonObject data;
#else
    JsonObject data;
#endif
    success &= source->save(data);
    if (success)
        success &= load(data);
    return success;
}
bool JDObjectInterface::loadFrom(const JDObjectInterface* source)
{
    bool success = true;
#ifdef JD_USE_QJSON
    QJsonObject data;
#else
    JsonObject data;
#endif
    success &= source->save(data);
    if (success)
        success &= load(data);
    return success;
}

bool JDObjectInterface::isManaged() const
{
	return m_manager != nullptr;
}

JDObjectIDptr JDObjectInterface::getObjectID() const
{
    if(m_manager)
        return m_manager->getID();
    return nullptr;
}
/*void JDObjectInterface::setObjectID(const JDObjectIDptr& id)
{
    m_objID = id;
}*/

#ifdef JD_USE_QJSON
bool JDObjectInterface::equalData(const QJsonObject& obj) const
#else
bool JDObjectInterface::equalData(const JsonValue& obj) const
#endif
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_4);
#ifdef JD_USE_QJSON
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
#else
    bool equal = true;
    const JsonObject& valueObject = std::get<JsonObject>(obj.getConstVariant());
    auto it = valueObject.find(s_tag_data);
    if(it == valueObject.end())
		return false;

    const JsonObject &data1 = valueObject.find(s_tag_data)->second.getObject();
    JsonObject data2;

    {
        JD_GENERAL_PROFILING_BLOCK("user save", JD_COLOR_STAGE_5);
        equal &= save(data2);
    }
    {
        JD_GENERAL_PROFILING_BLOCK("UserEqual", JD_COLOR_STAGE_5);
        equal &= data1 == data2;
    }

    return equal;
#endif
    
}
#ifdef JD_USE_QJSON
bool JDObjectInterface::loadInternal(const QJsonObject &obj)
#else
bool JDObjectInterface::loadInternal(const JsonValue& obj)
#endif
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_4);
    
#ifdef JD_USE_QJSON
    if(!obj.contains(s_tag_objID))
        return false;
    QJsonObject data;
    bool success = getJsonValue(obj, data, s_tag_data);
    
    {
        JD_GENERAL_PROFILING_BLOCK("UserLoad", JD_COLOR_STAGE_5);
        success &= load(data);
    }
    //setObjectID(obj[s_tag_objID].toInt());
    return success;
#else
    if (!obj.contains(s_tag_objID))
        return false;
    JsonObject data;
    bool success = obj.getObject(data, s_tag_data);

    {
        JD_GENERAL_PROFILING_BLOCK("UserLoad", JD_COLOR_STAGE_5);
        success &= load(data);
    }
/*#ifdef JD_USE_QJSON
    setObjectID(obj.getInt(s_tag_objID));
#else
    int id;
    if (!obj.getInt(id, s_tag_objID))
        return false;
    setObjectID(id);
#endif*/
    return success;
#endif
}

#ifdef JD_USE_QJSON
bool JDObjectInterface::saveInternal(QJsonObject &obj)
#else
bool JDObjectInterface::saveInternal(JsonObject& obj)
#endif
{
    return getSaveData(obj);
}
#ifdef JD_USE_QJSON
bool JDObjectInterface::getSaveData(QJsonObject& obj) const
#else
bool JDObjectInterface::getSaveData(JsonObject& obj) const
#endif
{
    JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_4);
#ifdef JD_USE_QJSON
    obj[s_tag_objID] = getObjectID()->get();
    obj[s_tag_className] = className().c_str();
    QJsonObject data;
    bool ret;
    {
        JD_GENERAL_PROFILING_BLOCK("UserSave", JD_COLOR_STAGE_5);
        ret = save(data);
    }

    obj[s_tag_data] = data;
    return ret;
#else
    obj.reserve(3);
    obj[s_tag_objID] = getObjectID()->get();
    obj[s_tag_className] = className();
    obj[s_tag_data] = JsonObject();
    bool ret;
    {
        JD_GENERAL_PROFILING_BLOCK("UserSave", JD_COLOR_STAGE_5);
        JsonObject& data = std::get<JsonObject>(obj[s_tag_data].getVariant());
        ret = save(data);
    }
    return ret;
#endif
}


void JDObjectInterface::setManager(Internal::JDObjectManager* manager)
{
	m_manager = manager;
}
}
