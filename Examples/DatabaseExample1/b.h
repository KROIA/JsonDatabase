#ifndef B_H
#define B_H

#include "JDObjectInterface.h"

class B : public JsonDatabase::JDObjectInterface
{
        JD_OBJECT(B)
    public:
        B();
        B(const std::string &id);
        B(const B &other);
        ~B();

    private:
        std::string getObjectID() const override;
        void setObjectID(const std::string &id) override;

        bool load(const QJsonObject &obj) override;
        bool save(QJsonObject &obj) const override;

        std::string objID;
};

#endif // A_H
