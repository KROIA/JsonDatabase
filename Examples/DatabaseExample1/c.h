#ifndef C_H
#define C_H

#include "JsonDatabase.h"

class C : public JsonDatabase::JDObjectInterface
{
        JD_OBJECT(C)
    public:
        C();
        ~C();

    private:

        bool load(const QJsonObject &obj) override;
        bool save(QJsonObject &obj) const override;
};

#endif // A_H
