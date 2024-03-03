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

        bool load(const JsonDatabase::JsonObject& obj) override;
        bool save(JsonDatabase::JsonObject& obj) const override;
};

#endif // A_H
