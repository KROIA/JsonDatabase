#include "utilities/JsonUtilities.h"
#include "utilities/JDObjectIDDomain.h"
#include "object/JDObjectInterface.h"
#include "object/JDObjectRegistry.h"
#include "manager/async/WorkProgress.h"
#include "manager/JDManager.h"

#include <QJsonDocument>

#ifdef JD_ENABLE_MULTITHREADING
#include <thread>
#endif


namespace JsonDatabase
{
	namespace Internal
	{
#ifdef JD_USE_QJSON
        bool JsonUtilities::getJsonArray(const std::vector<JDObject>& objs, std::vector<QJsonObject>& jsonOut)
#else
        bool JsonUtilities::getJsonArray(const std::vector<JDObject>& objs, JsonArray& jsonOut)
#endif
        {
            return getJsonArray(objs, jsonOut, nullptr, 0.0);
        }
#ifdef JD_USE_QJSON
        bool JsonUtilities::getJsonArray(const std::vector<JDObject>& objs, 
                                         std::vector<QJsonObject>& jsonOut, 
                                         WorkProgress* progress,
                                         double deltaProgress)
#else
        bool JsonUtilities::getJsonArray(const std::vector<JDObject>& objs,
            JsonArray& jsonOut,
            WorkProgress* progress,
            double deltaProgress)
#endif
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_2);
            
            
            bool success = true;

#ifdef JD_ENABLE_MULTITHREADING
            unsigned int threadCount = std::thread::hardware_concurrency();
            if(threadCount > 100)
                threadCount = 100;
            size_t objCount = objs.size();
            if (objCount > 100 && threadCount)
            {
                struct ThreadData
                {
                    size_t start;
                    size_t end;
                };
                std::vector<std::thread*> threads(threadCount, nullptr);
                std::vector<ThreadData> threadData(threadCount);
                size_t chunkSize = objCount / threadCount;
                size_t remainder = objCount % threadCount;
                size_t start = 0;
                jsonOut.resize(objCount);


                for (size_t i = 0; i < threadCount; ++i)
                {
                    threadData[i].start = start;
                    threadData[i].end = start + chunkSize;
                    start += chunkSize;
                    if (i == threadCount - 1)
                        threadData[i].end += remainder;
                    threads[i] = new std::thread([&threadData, &objs, i, &jsonOut]()
                        {
                            ThreadData& data = threadData[i];
                            for (size_t j = data.start; j < data.end; ++j)
                            {
#ifdef JD_USE_QJSON
                                QJsonObject data;
#else
                                JsonObject data;
#endif
                                objs[j]->saveInternal(data);
                                jsonOut[j] = std::move(data);
                            }
                        });
                }
                // Wait for all threads to finish
                for (size_t i = 0; i < threadCount; ++i)
                {
                    threads[i]->join();
                    if (progress)
                    {
                        progress->addProgress(deltaProgress * (threadData[i].end - threadData[i].start));
                    }
                    delete threads[i];
                }
            }
            else
#endif
            {
                jsonOut.reserve(objs.size());
                for (auto o : objs)
                {
#ifdef JD_USE_QJSON
                    QJsonObject data;
#else
                    JsonObject data;
#endif
                    success &= o->saveInternal(data);
                    jsonOut.emplace_back(std::move(data));
                    if(progress)
                        progress->addProgress(deltaProgress);
                }
            }
            return success;
        }
#ifdef JD_USE_QJSON
        bool JsonUtilities::serializeObject(JDObject obj, std::string& serializedOut)
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
#endif

#ifdef JD_USE_QJSON
        bool JsonUtilities::deserializeOverrideFromJson(const QJsonObject& json, JDObject obj, bool& hasChangedOut)
#else
		bool JsonUtilities::deserializeOverrideFromJson(const JsonValue& json, JDObject obj, bool& hasChangedOut)
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
        bool JsonUtilities::deserializeOverrideFromJson(const QJsonObject& json, JDObject obj)
#else
        bool JsonUtilities::deserializeOverrideFromJson(const JsonValue& json, JDObject obj)
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
        bool JsonUtilities::serializeObject(JDObject obj, std::string& serializedOut)
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_4);
            if (!obj) return false;

            JsonObject data;
            if (!obj->saveInternal(data))
                return false;

            serializedOut = JsonValue(data).serialize();
            return true;
        }
#endif

#ifndef JD_USE_QJSON
        bool JsonUtilities::deserializeJson(
            const JsonValue& json, 
            JDObject objOriginal, 
            JDObject& objOut,
            JDObjectIDDomain& idDomain,
            JDManager& manager)
#else
        bool JsonUtilities::deserializeJson(const QJsonObject& json, JDObject objOriginal,
            JDObject& objOut, JDObjectIDDomain& idDomain, JDManager& manager)
#endif
        {
            JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_3);
            if (objOriginal)
            {
                JDObjectIDptr ID;
                JDObjectID::IDType id;
#ifndef JD_USE_QJSON
                if (json.getInt(id, JDObjectInterface::s_tag_objID))
#else
                if (JDSerializable::getJsonValue(json, id, JDObjectInterface::s_tag_objID))
#endif
                {
                    //ID = id;
                    ID = idDomain.getExistingID(id);
                    if (!ID)
                    {
                        JD_CONSOLE_FUNCTION("Can't get existing id: "<<id);
						return false;
                    }
                }
                else
                {
#ifndef JD_USE_QJSON
                    JD_CONSOLE_FUNCTION("Objet has incomplete data. Key: \"" 
                    						<< JDObjectInterface::s_tag_objID << "\" is missed\n" 
                    						<< "Object: " << json);
#else
                    JD_CONSOLE_FUNCTION("Objet has incomplete data. Key: \""
                        << JDObjectInterface::s_tag_objID.toStdString() << "\" is missed\n");
#endif
                    return false;
                }
                if (objOriginal->equalData(json))
                {
                    objOut = objOriginal;
                    // objOriginal->setVersion(json); // Update version value from loaded object
                }
                else
                {
                    objOut = manager.createClone(objOriginal, json, ID);
                   // objOut = objOriginal->clone(json, ID);
                }
            }
            else
            {
                JDObject clone = JDObjectRegistry::getObjectDefinition(json);
                if (!clone)
                {
                    std::string className;
#ifndef JD_USE_QJSON
                    if (!json.getString(className, JDObjectInterface::s_tag_className))
#else
                    if(!JDSerializable::getJsonValue(json, className, JDObjectInterface::s_tag_className))
#endif
                    {
#ifndef JD_USE_QJSON
                        JD_CONSOLE_FUNCTION("Objet has incomplete data. Key: \""
                            <<JDObjectInterface::s_tag_className<< "\" is missed\n" 
                            <<"Object: "<<json);
#else
                        JD_CONSOLE_FUNCTION("Objet has incomplete data. Key: \""
                            << JDObjectInterface::s_tag_className.toStdString() << "\" is missed\n");
#endif
                        return false;
                    }

                    JD_CONSOLE_FUNCTION("Objecttype: " << className.c_str() << " is not known by this database. "
                        "Call: addObjectDefinition<" << className.c_str() << ">(); first\n");
                    return false;
                }

                JDObjectIDptr ID;
                JDObjectID::IDType id;
#ifndef JD_USE_QJSON
                if (json.getInt(id, JDObjectInterface::s_tag_objID))
#else
                if(JDSerializable::getJsonValue(json, id, JDObjectInterface::s_tag_objID))
#endif
                {
                    //ID = id;
                    bool success;
                    ID = idDomain.getNewID(id, success);
                    if(!success)
					{
						JD_CONSOLE_FUNCTION("Can't generate new id: "<<id);
						return false;
					}
                }
                else
                {
#ifndef JD_USE_QJSON
                    JD_CONSOLE_FUNCTION("Objet has incomplete data. Key: \""
                        << JDObjectInterface::s_tag_objID << "\" is missed\n"
                        << "Object: " << json);
#else
                    JD_CONSOLE_FUNCTION("Objet has incomplete data. Key: \""
                        << JDObjectInterface::s_tag_objID.toStdString() << "\" is missed\n");
#endif
                    return false;
                }
                objOut = manager.createClone(clone, json, ID);
                // objOut = clone->clone(json, ID);
            }
            return true;
        }

	}
}