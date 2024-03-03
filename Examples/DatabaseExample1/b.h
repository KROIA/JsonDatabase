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

        bool load(const JsonDatabase::JsonObject& obj) override;
        bool save(JsonDatabase::JsonObject& obj) const override;
};

#endif // A_H
