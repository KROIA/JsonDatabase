#pragma once

#include "JsonDatabase.h"
#include <vector>
#include <string>

using namespace JsonDatabase;
using std::string;

class Person : public JDObjectInterface
{
    
    JD_OBJECT(Person)
public:
    
    /*
    std::string firstName, lastName, gender;
    unsigned int age;
    std::string email, phone, education, occupation;
    unsigned int experience, salary, ;
    std::string martialStatus;
    numberOfChildren
    
    */
    Person(string fn, string ln, string g, string age,
        string email, string ph, string edu, string occ,
        string exp, string sal, string mart, string chc);
    Person();
    ~Person();


    bool operator==(const Person& other);
    bool operator!=(const Person& other);


    std::string firstName, lastName, gender;
    std::string age;
    std::string email, phone, education, occupation;
    std::string experience, salary, numberOfChildren;
    std::string martialStatus;
private:
   
#if JD_ACTIVE_JSON == JD_JSON_QT
    bool load(const QJsonObject& obj) override;
    bool save(QJsonObject& obj) const override;
#elif JD_ACTIVE_JSON == JD_JSON_GLAZE || JD_ACTIVE_JSON == JD_JSON_INTERNAL
    bool load(const JsonObject& obj) override;
    bool save(JsonObject& obj) const override;
#endif
    


    //static size_t instanceCounter;
};





extern std::vector<JDObject> createPersons();