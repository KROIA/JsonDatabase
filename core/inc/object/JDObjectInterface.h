#pragma once

#include "JD_base.h"
#include "JDDeclaration.h"
#include "utilities/JDSerializable.h"
#include "JDObjectID.h"
#include "utilities/Signal.h"
#include <memory>

#if JD_ACTIVE_JSON == JD_JSON_QT
#include <QJsonObject>
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
#include "Json/JsonValue.h"
#endif
#include <string>



namespace JsonDatabase
{
    
    
    
    
class JSONDATABASE_EXPORT JDObjectInterface: protected Utilities::JDSerializable
{
        friend JDManager;
        friend Internal::JDManagerObjectManager;
        friend Utilities::JsonUtilities;
        friend Internal::JDObjectManager;

        friend class AutoObjectAddToRegistry;

    public:
        JDObjectInterface();
        //JDObjectInterface(const JDObjectIDptr& id);

        
        JDObjectInterface(const JDObjectInterface &other);
        virtual ~JDObjectInterface();

        JDObject deepClone() const;
        template<typename T>
        std::shared_ptr<T> deepClone() const;

        JDObject shallowClone() const;
        template<typename T>
        std::shared_ptr<T> shallowClone() const;
        

        // Creates a copy of the original object as a new instance
        //static std::vector<JDObject> reinstantiate(const std::vector<JDObject> &objList);
#if JD_ACTIVE_JSON == JD_JSON_QT
        static size_t getJsonIndexByID(const std::vector<QJsonObject>& jsons, const JDObjectIDptr& objID);
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
        static size_t getJsonIndexByID(const JsonArray& jsons, const JDObjectIDptr& objID);
#endif


        bool loadFrom(const JDObject& source);
        bool loadFrom(const JDObjectInterface* source);

        bool isManaged() const;


        virtual const std::string& className() const = 0;

        JDObjectIDptr getObjectID() const;

        /*
            The shallow ID is a backup id for the case the object is unmanaged but was copied from an managed object.
            This id has a weak bound to the object if it is managed.
            Always try to use
            JDObjectIDptr getObjectID() const;
            to get the id of the object. Only if it returns an empty pointer use
            const JDObjectID::IDType& getShallowObjectID() const;
        */
        const JDObjectID::IDType& getShallowObjectID() const;
    protected:
        //void setObjectID(const JDObjectIDptr&id);

#if JD_ACTIVE_JSON == JD_JSON_QT
        bool equalData(const QJsonObject &obj) const;
        bool loadInternal(const QJsonObject &obj);
        bool saveInternal(QJsonObject &obj);
        bool getSaveData(QJsonObject &obj) const;
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
        bool equalData(const JsonObject& obj) const;
        bool loadInternal(const JsonObject& obj);
        bool saveInternal(JsonObject& obj);
        bool getSaveData(JsonObject& obj) const;
#endif


    class JSONDATABASE_EXPORT AutoObjectAddToRegistry
    {
    public:
        AutoObjectAddToRegistry(JDObject obj);
        int addToRegistry(JDObject obj);
    };

    private:

        void setManager(Internal::JDObjectManager* manager);
        Internal::JDObjectManager* getManager() const;

        virtual JDObjectInterface* deepClone_internal() const = 0;
        virtual JDObjectInterface* shallowClone_internal() const = 0;
#if JD_ACTIVE_JSON == JD_JSON_QT
        //virtual JDObjectInterface* clone_internal(const QJsonObject& obj, const JDObjectIDptr& uniqueID) const = 0;
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
        //virtual JDObjectInterface* clone_internal(const JsonValue& obj, const JDObjectIDptr& uniqueID) const = 0;
#endif



        Internal::JDObjectManager* m_manager;

        /*
            The shallow ID is a backup id for the case the object is unmanaged but was copied from an managed object.
            This id has a weak bound to the object if it is managed.
            Always try to use 
            JDObjectIDptr getObjectID() const;
            to get the id of the object. Only if it returns an empty pointer use 
            const JDObjectID::IDType& getShallowObjectID() const;
        */
        JDObjectID::IDType m_shallowID;
        //JDObjectIDptr m_objID;
        //Signal<const JDObjectInterface*> m_onDelete;

    public:
#if JD_ACTIVE_JSON == JD_JSON_QT
        static const QString s_tag_objID;
        static const QString s_tag_className;
        static const QString s_tag_data;
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
        static const std::string s_tag_objID;
        static const std::string s_tag_className;
        static const std::string s_tag_data;
#endif
    private:
        
};

template<typename T>
std::shared_ptr<T> JDObjectInterface::deepClone() const
{
    JD_OBJECT_PROFILING_FUNCTION(JD_COLOR_STAGE_4);
    T* cloned = dynamic_cast<T*>(deepClone_internal());
    std::shared_ptr<T> clone(cloned);
    return clone;
}
template<typename T>
std::shared_ptr<T> JDObjectInterface::shallowClone() const
{
    JD_OBJECT_PROFILING_FUNCTION(JD_COLOR_STAGE_4);
    T* cloned = dynamic_cast<T*>(shallowClone_internal());
    std::shared_ptr<T> clone(cloned);
    return clone;
}



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
    //classNameVal(const JsonDatabase::JDObjectIDptr &id); 

#if JD_ACTIVE_JSON == JD_JSON_QT
#define JD_OBJECT_DECL_CLONE(classNameVal) \
    classNameVal* deepClone_internal() const override; \
    classNameVal* shallowClone_internal() const override; \
    //classNameVal* clone_internal(const QJsonObject &reader, const JsonDatabase::JDObjectIDptr &uniqueID) const override; 
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
#define JD_OBJECT_DECL_CLONE(classNameVal) \
    classNameVal* deepClone_internal() const override; \
    classNameVal* shallowClone_internal() const override; \
    //classNameVal* clone_internal(const JsonDatabase::JsonValue &reader, const JsonDatabase::JDObjectIDptr &uniqueID) const override;
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
    //classNameVal::classNameVal(const JsonDatabase::JDObjectIDptr &id) \
    //    : JDObjectInterface(id) \
    //{} 

#if JD_ACTIVE_JSON == JD_JSON_QT
#define JD_OBJECT_IMPL_CLONE(classNameVal) \
    classNameVal* classNameVal::deepClone_internal() const \
    { \
        JD_OBJECT_PROFILING_FUNCTION(JD_COLOR_STAGE_5); \
        classNameVal* c = new classNameVal(*this); \
       /* c->setObjectID(this->getObjectID()); */ \
        return c; \
    } \
    classNameVal* classNameVal::shallowClone_internal() const \
    { \
        JD_OBJECT_PROFILING_FUNCTION(JD_COLOR_STAGE_5); \
        classNameVal* c = new classNameVal(); \
       /* c->setObjectID(this->getObjectID()); */ \
        return c; \
    } \
    //classNameVal* classNameVal::clone_internal(const QJsonObject &reader, const JsonDatabase::JDObjectIDptr &uniqueID) const\
    //{ \
    //    classNameVal* obj = new classNameVal(); \
    //    obj->setObjectID(uniqueID); \
    //    obj->loadInternal(reader); \
    //    return obj; \
    //} 
#elif JD_ACTIVE_JSON == JD_JSON_INTERNAL
#define JD_OBJECT_IMPL_CLONE(classNameVal) \
    classNameVal* classNameVal::deepClone_internal() const \
    { \
        JD_OBJECT_PROFILING_FUNCTION(JD_COLOR_STAGE_5); \
        classNameVal* c = new classNameVal(*this); \
        /* //c->setObjectID(this->getObjectID()); */ \
        return c; \
    } \
    classNameVal* classNameVal::shallowClone_internal() const \
    { \
        JD_OBJECT_PROFILING_FUNCTION(JD_COLOR_STAGE_5); \
        classNameVal* c = new classNameVal(); \
        /* //c->setObjectID(this->getObjectID()); */ \
        return c; \
    } \
    //classNameVal* classNameVal::clone_internal(const JsonDatabase::JsonValue &reader, const JsonDatabase::JDObjectIDptr &uniqueID) const\
    //{ \
    //    classNameVal* obj = new classNameVal(); \
    //    obj->setObjectID(uniqueID); \
    //    obj->loadInternal(reader); \
    //    return obj; \
    //} 
#endif

#define JD_OBJECT_IMPL_CLASSNAME(classNameVal) \
    const std::string &classNameVal::className() const \
    { \
        static std::string name = #classNameVal; \
        return name; \
    } 


#define JD_OBJECT_IMPL_AUTOREGISTRY(classNameVal) \
    classNameVal::AutoObjectAddToRegistry classNameVal::s__autoObjectRegistrator(std::shared_ptr<classNameVal>(new classNameVal()));


#define JD_OBJECT_IMPL(classNameVal) \
    JD_OBJECT_IMPL_CONSTRUCTOR_ID(classNameVal) \
    JD_OBJECT_IMPL_CLONE(classNameVal) \
    JD_OBJECT_IMPL_CLASSNAME(classNameVal) \
    JD_OBJECT_IMPL_AUTOREGISTRY(classNameVal) 


}
