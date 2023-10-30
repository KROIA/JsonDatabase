#ifndef A_H
#define A_H

#include "JDObjectInterface.h"

class A : public JsonDatabase::JDObjectInterface
{
    JD_OBJECT(A);
private:
    public:
        A();
        A(const std::string &id);
        A(const A &other);
        ~A();

    private:
        std::string getObjectID() const override;
        void setObjectID(const std::string &id) override;

        bool load(const QJsonObject &obj) override;
        bool save(QJsonObject &obj) const override;

        std::string objID;
};

#endif // A_H
