#include "object/JDObjectInterface.h"
#include "object/JDObjectRegistry.h"
#include "object/JDObjectManager.h"

/*#if JD_ACTIVE_JSON == JD_JSON_QT
#include <QVariant>
#endif*/

namespace JsonDatabase
{

#if JD_ACTIVE_JSON == JD_JSON_QT
    const QString JDObjectInterface::s_tag_objID = "objID";
    const QString JDObjectInterface::s_tag_className = "class";
    const QString JDObjectInterface::s_tag_data = "Data";
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
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
    , m_shallowID(0)
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
    , m_shallowID(other.m_shallowID)
  //  , m_objID(other.m_objID)
   // , m_onDelete("onDelete")
{
    //loadFrom(&other);
}
JDObjectInterface::~JDObjectInterface()
{
/*#ifdef JD_DEBUG
    if (isManaged())
    {
        if (JDObjectID::isValid(getObjectID()))
        {
            JD_CONSOLE("Delete managed object with ID: " << getObjectID()->get() << "\n");
        }
        else
        {
            JD_CONSOLE("Delete managed object with ID: invalid\n");
        }
    }
    else
    {
        if (JDObjectID::isValid(getObjectID()))
        {
            JD_CONSOLE("Delete unmanaged object with ID: " << getObjectID()->get() << "\n");
        }
        else
        {
            JD_CONSOLE("Delete unmanaged object with ID: "<<m_shallowID<<"\n");
        }
    }
#endif*/
}

JDObject JDObjectInterface::deepClone() const
{
    JD_OBJECT_PROFILING_FUNCTION(JD_COLOR_STAGE_4);
    JDObjectInterface *instance = deepClone_internal();
	return JDObject(instance);
}
JDObject JDObjectInterface::shallowClone() const
{
    JD_OBJECT_PROFILING_FUNCTION(JD_COLOR_STAGE_4);
    JDObjectInterface *instance = shallowClone_internal();
	return JDObject(instance);
}

/*/
std::vector<JDObject> JDObjectInterface::reinstantiate(const std::vector<JDObject>& objList)
{
    JD_OBJECT_PROFILING_FUNCTION(JD_COLOR_STAGE_4);
    std::vector<JDObject> ret;
	ret.reserve(objList.size());
    for (auto it = objList.begin(); it != objList.end(); ++it)
    {
		ret.push_back((*it)->clone());
	}
	return ret;
}*/
/*#if JD_ACTIVE_JSON == JD_JSON_QT
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
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL*/
size_t JDObjectInterface::getJsonIndexByID(const JsonArray& jsons, const JDObjectIDptr& objID)
{
    for (size_t i = 0; i < jsons.size(); ++i)
    {
        
        const JsonObject* obj = jsons[i].get_if<JsonObject>();
        if (!obj)
            continue;
        const auto& it = obj->find(s_tag_objID);
        if(it == obj->end())
			continue;
        const auto& value = it->second;
        
        const int *idPtr = value.get_if<int>();
        int id;
        if (!idPtr)
        {
            const double* idPtrD = value.get_if<double>();
            if(!idPtrD)
				continue;
            id = static_cast<int>(*idPtrD);
        }
		else
			id = *idPtr;


        if (id == objID->get())
            return i;
    }
    return std::string::npos;
}
//#endif

bool JDObjectInterface::loadFrom(const JDObject& source)
{
    bool success = true;
#if JD_ACTIVE_JSON == JD_JSON_QT
    QJsonObject data;
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
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
#if JD_ACTIVE_JSON == JD_JSON_QT
    QJsonObject data;
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
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
const JDObjectID::IDType& JDObjectInterface::getShallowObjectID() const
{
    return m_shallowID;
}
/*void JDObjectInterface::setObjectID(const JDObjectIDptr& id)
{
    m_objID = id;
}*/

#if JD_ACTIVE_JSON == JD_JSON_QT
bool JDObjectInterface::equalData(const QJsonObject& obj) const
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
bool JDObjectInterface::equalData(const JsonObject& obj) const
#endif
{
    JD_OBJECT_PROFILING_FUNCTION(JD_COLOR_STAGE_4);
#if JD_ACTIVE_JSON == JD_JSON_QT
    QJsonObject data1;
    QJsonObject data2;
    bool equal = getJsonValue(obj, data1, s_tag_data);
    {
        JD_OBJECT_PROFILING_BLOCK("user save", JD_COLOR_STAGE_5);
        equal &= save(data2);
    }
    {
        JD_OBJECT_PROFILING_BLOCK("UserEqual", JD_COLOR_STAGE_5);
        equal &= data1 == data2;
    }

    return equal;
 #elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
    bool equal = true;
   // const JsonObject& valueObject = obj.get<JsonObject>();

    auto it = obj.find(s_tag_data);
    if(it == obj.end())
		return false;


    const JsonObject &data1 = obj.find(s_tag_data)->second.get<JsonObject>();

    JsonObject data2;

    {
        JD_OBJECT_PROFILING_BLOCK("user save", JD_COLOR_STAGE_5);
        equal &= save(data2);
    }
    {
        JD_OBJECT_PROFILING_BLOCK("UserEqual", JD_COLOR_STAGE_5);
        equal &= data1 == data2;
    }

    return equal;
#endif
    
}
#if JD_ACTIVE_JSON == JD_JSON_QT
bool JDObjectInterface::loadInternal(const QJsonObject &obj)
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
bool JDObjectInterface::loadInternal(const JsonObject& obj)
#endif
{
    JD_OBJECT_PROFILING_FUNCTION(JD_COLOR_STAGE_4);
    
#if JD_ACTIVE_JSON == JD_JSON_QT
    QJsonObject data;
    bool success = getJsonValue(obj, data, s_tag_data);
    
    {
        JD_OBJECT_PROFILING_BLOCK("UserLoad", JD_COLOR_STAGE_5);
        success &= load(data);
    }
    //setObjectID(obj[s_tag_objID].toInt());
    return success;
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
    const JsonObject *data = obj.at(s_tag_data).get_if<JsonObject>();
    bool success = true;

    if (data)
    {
        JD_OBJECT_PROFILING_BLOCK("UserLoad", JD_COLOR_STAGE_5);
        success &= load(*data);
    }
    else
        success = false;
    return success;
#endif
}

#if JD_ACTIVE_JSON == JD_JSON_QT
bool JDObjectInterface::saveInternal(QJsonObject &obj)
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
bool JDObjectInterface::saveInternal(JsonObject& obj)
#endif
{
    return getSaveData(obj);
}
#if JD_ACTIVE_JSON == JD_JSON_QT
bool JDObjectInterface::getSaveData(QJsonObject& obj) const
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
bool JDObjectInterface::getSaveData(JsonObject& obj) const
#endif
{
    JD_OBJECT_PROFILING_FUNCTION(JD_COLOR_STAGE_4);
#if JD_ACTIVE_JSON == JD_JSON_QT
    JDObjectIDptr id = getObjectID();
    JDObjectID::IDType idVal = JDObjectID::invalidID;
    if (id)
        idVal = id->get();
    else
        idVal = m_shallowID;
    obj[s_tag_objID] = idVal;
    obj[s_tag_className] = className().c_str();
    QJsonObject data;
    bool ret;
    {
        JD_OBJECT_PROFILING_BLOCK("UserSave", JD_COLOR_STAGE_5);
        ret = save(data);
    }

    obj[s_tag_data] = data;
    return ret;
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
    //obj.reserve(3);
    JDObjectIDptr id = getObjectID();
    JDObjectID::IDType idVal = JDObjectID::invalidID;
    if(id)
        idVal = id->get();
    else
        idVal = m_shallowID;
    *obj[s_tag_objID] = idVal;
    *obj[s_tag_className] = className();
    bool ret;
    {
        JD_OBJECT_PROFILING_BLOCK("UserSave", JD_COLOR_STAGE_5);
       // JsonObject& data = std::get<JsonObject>(obj[s_tag_data].getVariant());
        std::shared_ptr<JsonObject> data = std::make_shared<JsonObject>();
        ret = save(*data);
        *obj[s_tag_data] = std::move(data);
    }
    return ret;
#endif
}


void JDObjectInterface::setManager(Internal::JDObjectManager* manager)
{
    JDObjectIDptr id = nullptr;
    if (m_manager)
    {
        if (!manager)
            id = m_manager->getID();
        else
            id = manager->getID();
    }
    else
        if (manager)
        {
            id = manager->getID();
        }
    if (id.get())
        m_shallowID = id->get();
	m_manager = manager;
}
Internal::JDObjectManager* JDObjectInterface::getManager() const
{
    return m_manager;
}
}
