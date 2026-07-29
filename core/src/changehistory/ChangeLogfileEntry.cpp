#include "changehistory/ChangeLogfileEntry.h"

namespace JsonDatabase
{
	ChangeLogfileEntry::ChangeLogfileEntry()
	{

	}
	ChangeLogfileEntry::~ChangeLogfileEntry()
	{

	}


	bool ChangeLogfileEntry::load(const JsonObject& obj)
	{
        JD_UNUSED(obj);
        return false;
	}
	bool ChangeLogfileEntry::save(JsonObject& obj) const
	{
        JD_UNUSED(obj);
        return false;
	}



    /*
    ChangeLogfileEntry::ChangeLogFileEntryRegistry::ChangeLogfileEntry::ChangeLogFileEntryRegistry()
    {

    }
    ChangeLogfileEntry::ChangeLogFileEntryRegistry& ChangeLogfileEntry::ChangeLogFileEntryRegistry::getInstance() {
        static ChangeLogfileEntry::ChangeLogFileEntryRegistry instance;
        return instance;
    }

    ChangeLogfileEntry::ChangeLogFileEntryRegistry::Error ChangeLogfileEntry::ChangeLogFileEntryRegistry::registerType(const ChangeLogfileEntry& obj) {
        Error error = Error::none;
        ChangeLogfileEntry::ChangeLogFileEntryRegistry& instance = getInstance();

        if (obj)
        {
            const std::string& className = obj->className();
            if (className.size() == 0)
            {
                error = Error::emptyClassName;
                return error;
            }
            if (instance.m_registry.find(className) != instance.m_registry.end())
            {
                error = Error::typeAlreadyRegistered;
                return error;
            }

            instance.m_registry.insert(std::pair<std::string, ChangeLogfileEntry>(className, obj));
        }
        else
            error = Error::objIsNullptr;
        return error;
    }


    const ChangeLogfileEntry& ChangeLogfileEntry::ChangeLogFileEntryRegistry::getObjectDefinition(const JsonObject& json)
    {
        JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
        std::string className;
        bool match = json.contains(ChangeLogfileEntryInterface::s_tag_className);
        if (match)
        {
            className = json.at(ChangeLogfileEntryInterface::s_tag_className).get<std::string>();

            return getObjectDefinition(className);
        }
        static const ChangeLogfileEntry nullObj = nullptr;
        return nullObj;
    }

    std::string ChangeLogfileEntry::ChangeLogFileEntryRegistry::getObjectTypeString(const JsonObject& json)
    {
        std::string className;
        className = json.at(ChangeLogfileEntryInterface::s_tag_className).get<std::string>();
        return className;
    }

    const ChangeLogfileEntry& ChangeLogfileEntry::ChangeLogFileEntryRegistry::getObjectDefinition(const std::string& className)
    {
        const std::map<std::string, ChangeLogfileEntry>& registry = getRegisteredTypes();
        auto it = registry.find(className);
        if (it == registry.end())
        {
            static const ChangeLogfileEntry nullObj = nullptr;
            return nullObj;
        }
        return it->second;
    }

    const std::map<std::string, ChangeLogfileEntry>& ChangeLogfileEntry::ChangeLogFileEntryRegistry::getRegisteredTypes()
    {
        return getInstance().m_registry;
    }*/
}