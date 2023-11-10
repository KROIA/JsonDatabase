#pragma once

#include "JD_base.h"
#include "JDDeclaration.h"
#include "JDSerializable.h"


//#include <QJsonObject>
#include <string>



namespace JsonDatabase
{
    // Pair of JDObjectInterface objects which have the same ID
    typedef std::pair<JDObjectInterface*, JDObjectInterface*> JDObjectPair;

class JSONDATABASE_EXPORT JDObjectInterface: protected JDSerializable
{
        friend JDManager;

    public:
        JDObjectInterface();
        JDObjectInterface(const std::string &id);
        JDObjectInterface(const JDObjectInterface &other);
        virtual ~JDObjectInterface();



        virtual JDObjectInterface* clone() const = 0;
        virtual JDObjectInterface* clone(const simdjson::dom::object &obj, const std::string &uniqueID) const = 0;
        virtual const std::string& className() const = 0;

        const std::string& getObjectID() const;
    protected:
        void setObjectID(const std::string &id);

        void setVersion(int version);
        void setVersion(const simdjson::dom::object& obj);
        bool equalData(const simdjson::dom::object &obj) const;
        bool loadInternal(const simdjson::dom::object &obj);
        bool saveInternal(simdjson::dom::object &obj);
        bool getSaveData(simdjson::dom::object &obj) const;


    class JSONDATABASE_EXPORT AutoObjectAddToRegistry
    {
    public:
        AutoObjectAddToRegistry(JDObjectInterface* obj);
        int addToRegistry(JDObjectInterface* obj);
    };

    private:

        std::string m_objID;
        int m_version; // ObjectVersion

        static const std::string m_tag_objID;
        static const std::string m_tag_objVersion;
        static const std::string m_tag_className;
        static const std::string m_tag_data;

        
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
    classNameVal(const std::string &id); 

#define JD_OBJECT_DECL_CLONE(classNameVal) \
    classNameVal* clone() const override; \
    classNameVal* clone(const simdjson::dom::object &reader, const std::string &uniqueID) const override; 

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
    classNameVal::classNameVal(const std::string &id) \
        : JDObjectInterface(id) \
    {} 


#define JD_OBJECT_IMPL_CLONE(classNameVal) \
    classNameVal* classNameVal::clone() const \
    { \
        classNameVal *c = new classNameVal(*this); \
        c->setObjectID(this->getObjectID()); \
        return c; \
    } \
    classNameVal* classNameVal::clone(const simdjson::dom::object &reader, const std::string &uniqueID) const\
    { \
        classNameVal *obj = new classNameVal(uniqueID); \
        obj->loadInternal(reader); \
        return obj; \
    } 


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
