#ifndef C_H
#define C_H

#include "JDObjectInterface.h"

class C : public JsonDatabase::JDObjectInterface
{
        JD_OBJECT(C)
    public:
        C();
        C(const std::string &id);
        C(const C &other);
        ~C();

    private:
        std::string getObjectID() const override;
        void setObjectID(const std::string &id) override;

        bool load(const QJsonObject &obj) override;
        bool save(QJsonObject &obj) const override;

        std::string objID;
};

#endif // A_H
