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
#ifdef JD_USE_QJSON
        bool load(const QJsonObject& obj) override;
        bool save(QJsonObject& obj) const override;
#else
        bool load(const JsonDatabase::JsonObject& obj) override;
        bool save(JsonDatabase::JsonObject& obj) const override;
#endif

};

#endif // A_H
