#pragma once

#include "JD_base.h"
#include "JDDeclaration.h"
#include "JDSerializable.h"
#include "JDObjectID.h"
#include "utilities/Signal.h"
#include <memory>

#ifdef JD_USE_QJSON
#include <QJsonObject>
#else
#include "Json/JsonValue.h"
#endif
#include <string>



namespace JsonDatabase
{
    
    
    
    
class JSONDATABASE_EXPORT JDObjectInterface: protected JDSerializable
{
        friend JDManager;
        friend Internal::JDManagerObjectManager;
        friend Internal::JsonUtilities;
        friend Internal::JDObjectManager;

        friend class AutoObjectAddToRegistry;

    public:
        JDObjectInterface();
        //JDObjectInterface(const JDObjectIDptr& id);

        
        JDObjectInterface(const JDObjectInterface &other);
        virtual ~JDObjectInterface();

        JDObject clone() const;
        template<typename T>
        std::shared_ptr<T> clone() const;
        

        // Creates a copy of the original object as a new instance
        //static std::vector<JDObject> reinstantiate(const std::vector<JDObject> &objList);
#ifdef JD_USE_QJSON
        static size_t getJsonIndexByID(const std::vector<QJsonObject>& jsons, const JDObjectIDptr& objID);
#else
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
        AutoObjectAddToRegistry(JDObject obj);
        int addToRegistry(JDObject obj);
    };

    private:

        void setManager(Internal::JDObjectManager* manager);
        Internal::JDObjectManager* getManager() const;

        virtual JDObjectInterface* clone_internal() const = 0;
#ifdef JD_USE_QJSON
        //virtual JDObjectInterface* clone_internal(const QJsonObject& obj, const JDObjectIDptr& uniqueID) const = 0;
#else
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

template<typename T>
std::shared_ptr<T> JDObjectInterface::clone() const
{
    T* cloned = dynamic_cast<T*>(clone_internal());
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

#ifdef JD_USE_QJSON
#define JD_OBJECT_DECL_CLONE(classNameVal) \
    classNameVal* clone_internal() const override; \
    //classNameVal* clone_internal(const QJsonObject &reader, const JsonDatabase::JDObjectIDptr &uniqueID) const override; 
#else
#define JD_OBJECT_DECL_CLONE(classNameVal) \
    classNameVal* clone_internal() const override; \
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

#ifdef JD_USE_QJSON
#define JD_OBJECT_IMPL_CLONE(classNameVal) \
    classNameVal* classNameVal::clone_internal() const \
    { \
        classNameVal* c = new classNameVal(*this); \
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
#else
#define JD_OBJECT_IMPL_CLONE(classNameVal) \
    classNameVal* classNameVal::clone_internal() const \
    { \
        classNameVal* c = new classNameVal(*this); \
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
