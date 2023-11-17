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
#ifdef JD_USE_QJSON
        bool load(const QJsonObject &obj) override;
        bool save(QJsonObject &obj) const override;
#else
        bool load(const JsonDatabase::JsonObject& obj) override;
        bool save(JsonDatabase::JsonObject& obj) const override;
#endif
};

#endif // A_H
