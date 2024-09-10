#include "object/JDObjectInterface.h"
#include "object/JDObjectRegistry.h"
#include "object/JDObjectManager.h"



namespace JsonDatabase
{

    const std::string JDObjectInterface::s_tag_objID = "objID";
    const std::string JDObjectInterface::s_tag_className = "class";
    const std::string JDObjectInterface::s_tag_data = "Data";

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
    , m_shallowID(JDObjectID::invalidID)
{

}

JDObjectInterface::JDObjectInterface(const JDObjectInterface &other)
    : m_manager(nullptr)
    , m_shallowID(other.m_shallowID)
{

}
JDObjectInterface::~JDObjectInterface()
{

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



size_t JDObjectInterface::getJsonIndexByID(const JsonArray& jsons, const JDObjectID::IDType& objID)
{
    for (size_t i = 0; i < jsons.size(); ++i)
    {
        JDObjectID::IDType id;
        const JsonObject* obj = jsons[i].get_if<JsonObject>();
        if (!obj)
            continue;
        const auto& it = obj->find(s_tag_objID);
        if(it == obj->end())
			continue;
        const JsonDatabase::JsonValue& value = it->second;
        
#if JD_ID_TYPE_SWITCH == JD_ID_TYPE_STRING
        const std::string* idPtr = value.get_if<std::string>();
        if (!idPtr)
            continue;
        id = *idPtr;
#elif JD_ID_TYPE_SWITCH == JD_ID_TYPE_LONG
        const long* idPtr = value.get_if<long>();
        if (!idPtr)
        {
            const double* idPtrD = value.get_if<double>();
            if (!idPtrD)
                continue;
            id = static_cast<long>(*idPtrD);
        }
        else
            id = *idPtr;
#else 
    #error "Invalid JD_ID_TYPE_SWITCH value"
#endif 
        if (id == objID)
            return i;
    }
    return std::string::npos;
}
JDObjectID::IDType JDObjectInterface::getIDFromJson(const JsonObject& obj)
{
	const auto& it = obj.find(s_tag_objID);
	if(it == obj.end())
		return JDObjectID::invalidID;
	const JsonDatabase::JsonValue& value = it->second;
    #if JD_ID_TYPE_SWITCH == JD_ID_TYPE_STRING
    const std::string* idPtr = value.get_if<std::string>();
		if (!idPtr)
			return JDObjectID::invalidID;
		return *idPtr;
        #elif JD_ID_TYPE_SWITCH == JD_ID_TYPE_LONG
    const long* idPtr = value.get_if<long>();
    if (!idPtr)
    {
		const double* idPtrD = value.get_if<double>();
		if (!idPtrD)
			return JDObjectID::invalidID;
		return static_cast<long>(*idPtrD);
	}
    return *idPtr;
		#else 
	#error "Invalid JD_ID_TYPE_SWITCH value"
	#endif
}


bool JDObjectInterface::loadFrom(const JDObject& source)
{
    bool success = true;
    JsonObject data;
    success &= source->save(data);
    if (success)
        success &= load(data);
    return success;
}
bool JDObjectInterface::loadFrom(const JDObjectInterface* source)
{
    bool success = true;
    JsonObject data;
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

bool JDObjectInterface::isLocked() const
{
    if(m_manager)
        return m_manager->isLocked();
    return false;
}
bool JDObjectInterface::lock()
{
    if (m_manager)
        return m_manager->lock();
    return false;
}
bool JDObjectInterface::unlock()
{
    if (m_manager)
        return m_manager->unlock();
    return false;
}

Utilities::JDUser JDObjectInterface::getLockOwner(bool& isLocked) const
{
    if(m_manager)
		return m_manager->getLockOwner(isLocked);
    isLocked = false;
    return Utilities::JDUser();
}

bool JDObjectInterface::saveToDatabase()
{
    if (m_manager)
        return m_manager->saveToDatabase();
    return false;
}
void JDObjectInterface::saveToDatabaseAsync() 
{
    if (m_manager)
        m_manager->saveToDatabaseAsync();
}

bool JDObjectInterface::loadFromDatabase() 
{
    if (m_manager)
        return m_manager->loadFromDatabase();
    return false;
}
void JDObjectInterface::loadFromDatabaseAsync()
{
    if (m_manager)
		m_manager->loadFromDatabaseAsync();
}

bool JDObjectInterface::equalData(const JsonObject& obj) const
{
    JD_OBJECT_PROFILING_FUNCTION(JD_COLOR_STAGE_4);
    bool equal = true;

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
}
bool JDObjectInterface::loadInternal(const JsonObject& obj)
{
    JD_OBJECT_PROFILING_FUNCTION(JD_COLOR_STAGE_4);
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
}

bool JDObjectInterface::saveInternal(JsonObject& obj)
{
    return getSaveData(obj);
}
bool JDObjectInterface::getSaveData(JsonObject& obj) const
{
    JD_OBJECT_PROFILING_FUNCTION(JD_COLOR_STAGE_4);
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
        std::shared_ptr<JsonObject> data = std::make_shared<JsonObject>();
        ret = save(*data);
        *obj[s_tag_data] = std::move(data);
    }
    return ret;
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
    {
        if (id.get()->get() != JDObjectID::invalidID)
            m_shallowID = id->get();
    }
	m_manager = manager;
}
Internal::JDObjectManager* JDObjectInterface::getManager() const
{
    return m_manager;
}
}
