#ifndef B_H
#define B_H

#include "JsonDatabase.h"

class B : public JsonDatabase::JDObjectInterface
{
        JD_OBJECT(B)
    public:
        B();
        ~B();

    private:
#ifdef JD_USE_QJSON
        bool load(const QJsonObject& obj) override;
        bool save(QJsonObject& obj) const override;
#else
        bool load(const JsonDatabase::JsonObject& obj) override;
        bool save(JsonDatabase::JsonObject& obj) const override;
#endif
};

#endif // A_H
