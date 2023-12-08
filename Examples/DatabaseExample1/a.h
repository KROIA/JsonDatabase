#ifndef A_H
#define A_H

#include "JsonDatabase.h"

class A : public JsonDatabase::JDObjectInterface
{
    JD_OBJECT(A);
private:
    public:
        A();
        ~A();

    private:
#if JD_ACTIVE_JSON == JD_JSON_QT
        bool load(const QJsonObject& obj) override;
        bool save(QJsonObject& obj) const override;
#elif JD_ACTIVE_JSON == JD_JSON_GLAZE || JD_ACTIVE_JSON == JD_JSON_INTERNAL
        bool load(const JsonDatabase::JsonObject& obj) override;
        bool save(JsonDatabase::JsonObject& obj) const override;
#endif

};

#endif // A_H
