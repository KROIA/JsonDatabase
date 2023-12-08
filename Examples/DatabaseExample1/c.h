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
#if JD_ACTIVE_JSON == JD_JSON_QT
        bool load(const QJsonObject &obj) override;
        bool save(QJsonObject &obj) const override;
#elif JD_ACTIVE_JSON == JD_JSON_GLAZE || JD_ACTIVE_JSON == JD_JSON_INTERNAL
        bool load(const JsonDatabase::JsonObject& obj) override;
        bool save(JsonDatabase::JsonObject& obj) const override;
#endif
};

#endif // A_H
