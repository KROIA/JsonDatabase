#include "utilities/JsonUtilities.h"
#include "object/JDObjectInterface.h"
#include "object/JDObjectRegistry.h"
#include "manager/async/WorkProgress.h"

#include <QJsonDocument>

namespace JsonDatabase
{
	namespace Internal
	{
#ifdef JD_USE_QJSON
        bool JsonUtilities::getJsonArray(const std::vector<JDObjectInterface*>& objs, std::vector<QJsonObject>& jsonOut)
#else
        bool JsonUtilities::getJsonArray(const std::vector<JDObjectInterface*>& objs, JsonArray& jsonOut)
#endif
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            jsonOut.reserve(objs.size());

            bool success = true;

            for (auto o : objs)
            {
#ifdef JD_USE_QJSON
                QJsonObject data;
#else
                JsonValue data;
#endif
                success &= o->saveInternal(data);
                jsonOut.emplace_back(data);
            }
            return success;
        }
#ifdef JD_USE_QJSON
        bool JsonUtilities::getJsonArray(const std::vector<JDObjectInterface*>& objs, 
                                         std::vector<QJsonObject>& jsonOut, 
                                         WorkProgress* progress,
                                         double deltaProgress)
#else
        bool JsonUtilities::getJsonArray(const std::vector<JDObjectInterface*>& objs,
            JsonArray& jsonOut,
            WorkProgress* progress,
            double deltaProgress)
#endif
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            jsonOut.reserve(objs.size());

            bool success = true;

            for (auto o : objs)
            {
#ifdef JD_USE_QJSON
                QJsonObject data;
#else
                JsonValue data;
#endif
                success &= o->saveInternal(data);
                jsonOut.emplace_back(data);
                progress->addProgress(deltaProgress);
            }
            return success;
        }
#ifdef JD_USE_QJSON
        bool JsonUtilities::serializeObject(JDObjectInterface* obj, std::string& serializedOut)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_4);
            if (!obj) return false;

            QJsonObject data;
            if (!obj->saveInternal(data))
                return false;

            return serializeJson(data, serializedOut);
        }
        bool JsonUtilities::serializeJson(const QJsonObject& obj, std::string& serializedOut)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
            QJsonDocument document;
            document.setObject(obj);
            QByteArray bytes = document.toJson(QJsonDocument::Indented);

            serializedOut = bytes.constData();
            return true;
        }
        bool JsonUtilities::deserializeJson(const QJsonObject& json, JDObjectInterface* objOriginal, JDObjectInterface*& objOut)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
            if (objOriginal)
            {
                JDObjectID ID;
                JDSerializable::getJsonValue(json, ID, JDObjectInterface::s_tag_objID);
                if (objOriginal->equalData(json))
                {
                    objOut = objOriginal;
                   // objOriginal->setVersion(json); // Update version value from loaded object
                }
                else
                {
                    objOut = objOriginal->clone(json, ID);
                }
            }
            else
            {
                JDObjectInterface* clone = JDObjectRegistry::getObjectDefinition(json);
                if (!clone)
                {
                    std::string className;
                    JDSerializable::getJsonValue(json, className, JDObjectInterface::s_tag_className);

                    JD_CONSOLE_FUNCTION("Objecttype: " << className.c_str() << " is not known by this database. "
                        "Call: addObjectDefinition<" << className.c_str() << ">(); first\n");
                    return false;
                }

                JDObjectID ID;
                JDSerializable::getJsonValue(json, ID, JDObjectInterface::s_tag_objID);

                objOut = clone->clone(json, ID);
            }
            return true;
        }
#endif

#ifdef JD_USE_QJSON
        bool JsonUtilities::deserializeOverrideFromJson(const QJsonObject& json, JDObjectInterface* obj, bool& hasChangedOut)
#else
		bool JsonUtilities::deserializeOverrideFromJson(const JsonValue& json, JDObjectInterface* obj, bool& hasChangedOut)
#endif
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
            if (!obj->equalData(json))
                hasChangedOut = true;
            if (!obj->loadInternal(json))
            {
                JD_CONSOLE_FUNCTION("Can't load data in object: " << obj->getObjectID() << " classType: " << obj->className() + "\n");
                return false;
            }
            return true;
        }
#ifdef JD_USE_QJSON
        bool JsonUtilities::deserializeOverrideFromJson(const QJsonObject& json, JDObjectInterface* obj)
#else
        bool JsonUtilities::deserializeOverrideFromJson(const JsonValue& json, JDObjectInterface* obj)
#endif
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
            if (!obj->loadInternal(json))
            {
                JD_CONSOLE_FUNCTION("Can't load data in object: " << obj->getObjectID() << " classType: " << obj->className() + "\n");
                return false;
            }
            return true;
        }

#ifndef JD_USE_QJSON
        bool JsonUtilities::serializeObject(JDObjectInterface* obj, std::string& serializedOut)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_4);
            if (!obj) return false;

            JsonValue data;
            if (!obj->saveInternal(data))
                return false;

            serializedOut = data.serialize();
            return true;
        }
        bool JsonUtilities::deserializeJson(const JsonValue& json, JDObjectInterface* objOriginal, JDObjectInterface*& objOut)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
            if (objOriginal)
            {
                JDObjectID ID;
                int id;
                if(json.getInt(id, JDObjectInterface::s_tag_objID))
					ID = id;
                else
                {
                    JD_CONSOLE_FUNCTION("Objet has incomplete data. Key: \"" 
                    						<< JDObjectInterface::s_tag_objID << "\" is missed\n" 
                    						<< "Object: " << json);
                    return false;
                }
                if (objOriginal->equalData(json))
                {
                    objOut = objOriginal;
                    // objOriginal->setVersion(json); // Update version value from loaded object
                }
                else
                {
                    objOut = objOriginal->clone(json, ID);
                }
            }
            else
            {
                JDObjectInterface* clone = JDObjectRegistry::getObjectDefinition(json);
                if (!clone)
                {
                    std::string className;
                    if (!json.getString(className, JDObjectInterface::s_tag_className))
                    {
                        JD_CONSOLE_FUNCTION("Objet has incomplete data. Key: \""
                            <<JDObjectInterface::s_tag_className<< "\" is missed\n" 
                            <<"Object: "<<json);
                        return false;
                    }

                    JD_CONSOLE_FUNCTION("Objecttype: " << className.c_str() << " is not known by this database. "
                        "Call: addObjectDefinition<" << className.c_str() << ">(); first\n");
                    return false;
                }

                JDObjectID ID;
                int id;
                if (json.getInt(id, JDObjectInterface::s_tag_objID))
                    ID = id;
                else
                {
                    JD_CONSOLE_FUNCTION("Objet has incomplete data. Key: \""
                        << JDObjectInterface::s_tag_objID << "\" is missed\n"
                        << "Object: " << json);
                    return false;
                }
                objOut = clone->clone(json, ID);
            }
            return true;
        }

#endif
	}
}