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
        

        bool load(const QJsonObject &obj) override;
        bool save(QJsonObject &obj) const override;


};

#endif // A_H
