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
        JDObjectInterface(const JDObjectInterface &other);
        virtual ~JDObjectInterface();



        virtual JDObjectInterface* clone() const = 0;
        virtual JDObjectInterface* clone(const QJsonObject &obj, const std::string &uniqueID) const = 0;
        virtual std::string className() const = 0;

        virtual std::string getObjectID() const = 0;
    protected:
        virtual void setObjectID(const std::string &id) = 0;


        bool loadInternal(const QJsonObject &obj);
        bool saveInternal(QJsonObject &obj);


    private:


        int m_version; // ObjectVersion

        static const QString m_tag_objID;
        static const QString m_tag_objVersion;
        static const QString m_tag_className;
        static const QString m_tag_data;

        class AutoObjectAddToRegistry
        {
        public:
            AutoObjectAddToRegistry(JDObjectInterface* obj)
            {
                addToRegistry(obj);
            }

            int addToRegistry(JDObjectInterface* obj);

        private:

        };
};


/**
 * \macro JD_OBJECT(classNameVal)
 *        Implements the virtual clone() function of the JDObjectInterface
 *        Implements the virtual className() function of the JDObjectInterface
 */
#define JD_OBJECT(classNameVal) \
public: \
classNameVal* clone() const override \
{ \
    classNameVal *c = new classNameVal(*this); \
    c->setObjectID(this->getObjectID()); \
    return c; \
} \
classNameVal* clone(const QJsonObject &reader, const std::string &uniqueID) const override\
{ \
    classNameVal *obj = new classNameVal(uniqueID); \
    obj->loadInternal(reader); \
    return obj; \
} \
std::string className() const override \
{ \
    return #classNameVal; \
} \
private: \





}
