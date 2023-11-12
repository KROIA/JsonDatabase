#pragma once

#include "JD_base.h"
#include "JDDeclaration.h"
#include "JDSerializable.h"

#include <QJsonObject>
#include <string>



namespace JsonDatabase
{
    

class JSONDATABASE_EXPORT JDObjectInterface: protected JDSerializable
{
        friend JDManager;
        friend Internal::JsonUtilities;
    public:
        JDObjectInterface();
        JDObjectInterface(const std::string &id);
        JDObjectInterface(const JDObjectInterface &other);
        virtual ~JDObjectInterface();

        // Creates a copy of the original object as a new instance
        static std::vector<JDObjectInterface*> reinstantiate(const std::vector<JDObjectInterface*> &objList);
        static size_t getJsonIndexByID(const std::vector<QJsonObject>& jsons, const std::string objID);


        virtual JDObjectInterface* clone() const = 0;
        virtual JDObjectInterface* clone(const QJsonObject &obj, const std::string &uniqueID) const = 0;
        virtual const std::string& className() const = 0;

        const std::string& getObjectID() const;
    protected:
        void setObjectID(const std::string &id);

        void setVersion(int version);
        void setVersion(const QJsonObject& obj);
        bool equalData(const QJsonObject &obj) const;
        bool loadInternal(const QJsonObject &obj);
        bool saveInternal(QJsonObject &obj);
        bool getSaveData(QJsonObject &obj) const;


    class JSONDATABASE_EXPORT AutoObjectAddToRegistry
    {
    public:
        AutoObjectAddToRegistry(JDObjectInterface* obj);
        int addToRegistry(JDObjectInterface* obj);
    };

    private:

        std::string m_objID;
        int m_version; // ObjectVersion

    public:
        static const QString s_tag_objID;
        static const QString s_tag_objVersion;
        static const QString s_tag_className;
        static const QString s_tag_data;
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
    classNameVal(const std::string &id); 

#define JD_OBJECT_DECL_CLONE(classNameVal) \
    classNameVal* clone() const override; \
    classNameVal* clone(const QJsonObject &reader, const std::string &uniqueID) const override; 

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
    classNameVal* classNameVal::clone(const QJsonObject &reader, const std::string &uniqueID) const\
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
