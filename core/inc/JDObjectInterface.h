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

    public:
        JDObjectInterface();
        JDObjectInterface(const std::string &id);
        JDObjectInterface(const JDObjectInterface &other);
        virtual ~JDObjectInterface();



        virtual JDObjectInterface* clone() const = 0;
        virtual JDObjectInterface* clone(const QJsonObject &obj, const std::string &uniqueID) const = 0;
        virtual const std::string& className() const = 0;

        const std::string& getObjectID() const;
    protected:
        void setObjectID(const std::string &id);


        bool loadInternal(const QJsonObject &obj);
        bool saveInternal(QJsonObject &obj);


    class AutoObjectAddToRegistry
    {
    public:
        AutoObjectAddToRegistry(JDObjectInterface* obj);
        int addToRegistry(JDObjectInterface* obj);
    };

    private:

        std::string m_objID;
        int m_version; // ObjectVersion

        static const QString m_tag_objID;
        static const QString m_tag_objVersion;
        static const QString m_tag_className;
        static const QString m_tag_data;

        
};


/**
 * \macro JD_OBJECT(classNameVal)
 *        Implements the virtual clone() function of the JDObjectInterface
 *        Implements the virtual className() function of the JDObjectInterface
 */
#define JD_OBJECT(classNameVal) \
public: \
classNameVal(const classNameVal &other); \
classNameVal(const std::string &id); \
classNameVal* clone() const override; \
classNameVal* clone(const QJsonObject &reader, const std::string &uniqueID) const override; \
const std::string &className() const override; \
private: \
static AutoObjectAddToRegistry s__autoObjectRegistrator;


#define JD_OBJECT_IMPL(classNameVal) \
classNameVal::classNameVal(const std::string &id) \
    : JDObjectInterface(id) \
{} \
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
} \
const std::string &classNameVal::className() const \
{ \
    static std::string name = #classNameVal; \
    return name; \
} \
classNameVal::AutoObjectAddToRegistry classNameVal::s__autoObjectRegistrator(new classNameVal());



}
