#pragma once

#include "JD_base.h"
#include "JDDeclaration.h"
#include "JDSerializable.h"
#include "JDObjectID.h"
#include <memory>

#ifdef JD_USE_QJSON
#include <QJsonObject>
#else
#include "Json/JsonValue.h"
#endif
#include <string>



namespace JsonDatabase
{
    
using JDObject = std::shared_ptr<JDObjectInterface>;

class JSONDATABASE_EXPORT JDObjectInterface: protected JDSerializable
{
        friend JDManager;
        friend Internal::JsonUtilities;

        JDObjectInterface();
        JDObjectInterface(const JDObjectIDptr& id);
    public:


        
        JDObjectInterface(const JDObjectInterface &other);
        virtual ~JDObjectInterface();

        // Creates a copy of the original object as a new instance
        static std::vector<JDObjectInterface*> reinstantiate(const std::vector<JDObjectInterface*> &objList);
#ifdef JD_USE_QJSON
        static size_t getJsonIndexByID(const std::vector<QJsonObject>& jsons, const JDObjectIDptr&objID);
#else
        static size_t getJsonIndexByID(const JsonArray& jsons, const JDObjectIDptr& objID);
#endif


        virtual JDObject clone() const = 0;
#ifdef JD_USE_QJSON
        virtual JDObject clone(const QJsonObject &obj, const JDObjectID &uniqueID) const = 0;
#else
        virtual JDObjectInterface* clone(const JsonValue& obj, const JDObjectIDptr& uniqueID) const = 0;
#endif
        virtual const std::string& className() const = 0;

        const JDObjectIDptr getObjectID() const;
    protected:
        void setObjectID(const JDObjectIDptr&id);

#ifdef JD_USE_QJSON
        bool equalData(const QJsonObject &obj) const;
        bool loadInternal(const QJsonObject &obj);
        bool saveInternal(QJsonObject &obj);
        bool getSaveData(QJsonObject &obj) const;
#else
        bool equalData(const JsonValue& obj) const;
        bool loadInternal(const JsonValue& obj);
        bool saveInternal(JsonObject& obj);
        bool getSaveData(JsonObject& obj) const;
#endif


    class JSONDATABASE_EXPORT AutoObjectAddToRegistry
    {
    public:
        AutoObjectAddToRegistry(JDObjectInterface* obj);
        int addToRegistry(JDObjectInterface* obj);
    };

    private:

        JDObjectIDptr m_objID;

    public:
#ifdef JD_USE_QJSON
        static const QString s_tag_objID;
        static const QString s_tag_className;
        static const QString s_tag_data;
#else
        static const std::string s_tag_objID;
        static const std::string s_tag_className;
        static const std::string s_tag_data;
#endif
    private:
        
};


/**
 * \macro JD_OBJECT(classNameVal)
 *        Implements the virtual clone() function of the JDObjectInterface
 *        Implements the virtual className() function of the JDObjectInterface
 */
#define JD_OBJECT(classNameVal) \
    public: \
    JD_OBJECT_DECL_CONSTRUCTOR_COPY(classNameVal) \
    JD_OBJECT_DECL_CONSTRUCTOR_ID(classNameVal) \
    JD_OBJECT_DECL_CLONE(classNameVal) \
    JD_OBJECT_DECL_CLASSNAME(classNameVal) \
    JD_OBJECT_DECL_AUTOREGISTRY(classNameVal) 



#define JD_OBJECT_DECL_CONSTRUCTOR_COPY(classNameVal) \
    classNameVal(const classNameVal &other); 

#define JD_OBJECT_DECL_CONSTRUCTOR_ID(classNameVal) \
    classNameVal(const JsonDatabase::JDObjectIDptr &id); 

#ifdef JD_USE_QJSON
#define JD_OBJECT_DECL_CLONE(classNameVal) \
    classNameVal* clone() const override; \
    classNameVal* clone(const QJsonObject &reader, const JsonDatabase::JDObjectIDptr &uniqueID) const override; 
#else
#define JD_OBJECT_DECL_CLONE(classNameVal) \
    classNameVal* clone() const override; \
    classNameVal* clone(const JsonDatabase::JsonValue &reader, const JsonDatabase::JDObjectIDptr &uniqueID) const override;
#endif

#define JD_OBJECT_DECL_CLASSNAME(classNameVal) \
    const std::string &className() const override; 

#define JD_OBJECT_DECL_AUTOREGISTRY(classNameVal) \
    private: \
    static AutoObjectAddToRegistry s__autoObjectRegistrator;


// Implementations 

#define JD_OBJECT_IMPL_CONSTRUCTOR_COPY(classNameVal) \
    classNameVal::classNameVal(const classNameVal &other) \
        : JDObjectInterface(other) \
    {} 


#define JD_OBJECT_IMPL_CONSTRUCTOR_ID(classNameVal) \
    classNameVal::classNameVal(const JsonDatabase::JDObjectIDptr &id) \
        : JDObjectInterface(id) \
    {} 

#ifdef JD_USE_QJSON
#define JD_OBJECT_IMPL_CLONE(classNameVal) \
    classNameVal* classNameVal::clone() const \
    { \
        classNameVal *c = new classNameVal(*this); \
        c->setObjectID(this->getObjectID()); \
        return c; \
    } \
    classNameVal* classNameVal::clone(const QJsonObject &reader, const JsonDatabase::JDObjectIDptr &uniqueID) const\
    { \
        classNameVal *obj = new classNameVal(uniqueID); \
        obj->loadInternal(reader); \
        return obj; \
    } 
#else
#define JD_OBJECT_IMPL_CLONE(classNameVal) \
    classNameVal* classNameVal::clone() const \
    { \
        classNameVal *c = new classNameVal(*this); \
        c->setObjectID(this->getObjectID()); \
        return c; \
    } \
    classNameVal* classNameVal::clone(const JsonDatabase::JsonValue &reader, const JsonDatabase::JDObjectIDptr &uniqueID) const\
    { \
        classNameVal *obj = new classNameVal(uniqueID); \
        obj->loadInternal(reader); \
        return obj; \
    } 
#endif

#define JD_OBJECT_IMPL_CLASSNAME(classNameVal) \
    const std::string &classNameVal::className() const \
    { \
        static std::string name = #classNameVal; \
        return name; \
    } 


#define JD_OBJECT_IMPL_AUTOREGISTRY(classNameVal) \
    classNameVal::AutoObjectAddToRegistry classNameVal::s__autoObjectRegistrator(new classNameVal());


#define JD_OBJECT_IMPL(classNameVal) \
    JD_OBJECT_IMPL_CONSTRUCTOR_ID(classNameVal) \
    JD_OBJECT_IMPL_CLONE(classNameVal) \
    JD_OBJECT_IMPL_CLASSNAME(classNameVal) \
    JD_OBJECT_IMPL_AUTOREGISTRY(classNameVal) 


}
