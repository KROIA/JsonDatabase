#ifndef B_H
#define B_H

#include "JDObjectInterface.h"

class B : public JsonDatabase::JDObjectInterface
{
        JD_OBJECT(B)
    public:
        B();
        ~B();

    private:

        bool load(const QJsonObject &obj) override;
        bool save(QJsonObject &obj) const override;

};

#endif // A_H
