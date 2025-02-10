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

    const QIcon &getIcon() const override
    {
        static QIcon icon(":/JDIcons/user.png");
        return icon;
    }

    bool operator==(const Person& other);
    bool operator!=(const Person& other);

    std::string getToolTip() const override { return firstName +std::string(" ")+lastName + std::string("\n") + email; }
    std::string getDisplayName() const override { return firstName + " " + lastName; }


    JDObjectValue<std::string> firstName, lastName, gender;
    JDObjectValue<std::string> age;
    JDObjectValue<std::string> email, phone, education, occupation;
    JDObjectValue<std::string> experience, salary, numberOfChildren;
    JDObjectValue<std::string> martialStatus;
private:
   

    // bool load(const JsonObject& obj) override;
    // bool save(JsonObject& obj) const override;
    
};





extern std::vector<JDObject> createPersons();