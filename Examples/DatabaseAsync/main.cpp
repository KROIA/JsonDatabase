//#define GLAZE_EXAMPLE


#ifdef GLAZE_EXAMPLE
#include "glaze/glaze.hpp"

#include <iostream>
#include <unordered_map>
#include <variant>
#include <vector>
#include <string>


using JsonValue = glz::json_t;
using JsonObject = glz::json_t::object_t;
using JsonArray = glz::json_t::array_t;

// Interface for serializable objects
class Serializable
{
public:

    // Function gets called when the data of the object gets loaded back
    virtual bool load(const JsonObject& obj) = 0;

    // Function gets called when a object gets saved to json
    virtual bool save(JsonObject& obj) const = 0;
};

// Implement dummy Obj1 
class Obj1 : public Serializable
{
public:
    Obj1(int val, const std::string& str)
        : value1(val), text1(str)
    {}

    bool load(const JsonObject& obj) override
    {
        // Get the values back from the loaded JsonObject 
        value1 = obj.at("value").get<double>();
        text1 = obj.at("text").get<std::string>();
        return true;
    }

    bool save(JsonObject& obj) const override
    {
        // Saves the data to the json structure
        obj["value"] = value1;
        obj["text"] = text1;

        JsonArray list{
            value1,
            value1 + 1,
            value1 + 2
        };
        obj["list"] = list;
        return true;
    }
    int value1;
    std::string text1;
};
class Obj2 : public Serializable
{
public:
    Obj2(int val, const std::string& str, int val2, const std::string& str2)
        : value1(val), text1(str), obj1(val2, str2)
    {}

    bool load(const JsonObject& obj) override
    {
        value1 = obj.at("value").get<double>();
        text1 = obj.at("text").get<std::string>();

        obj1.load(obj.at("obj1").get<JsonObject>());
        return true;
    }
    bool save(JsonObject& obj) const override
    {
        obj["value"] = (value1);
        obj["text"] = (text1);

        // Save nasted object
        JsonObject buff; // Create a json container
        obj1.save(buff); // save the nasted object
        obj["obj1"] = buff; // Add the data from the nasted object to this json structure
        return true;
    }

    int value1;
    std::string text1;
    Obj1 obj1;
};

// Serializes all objects
std::string serialized(const std::vector< Serializable*>& objs);

// Loads all objects back
// For simplicity the objs list must contain the same amount and objects to match the 
// json data
bool deserialize(const std::vector<Serializable*>& objs, const std::string& buffer);

void test(const JsonArray& array)
{

}
int main()
{
    JsonArray jsonData;
    test(jsonData);
    // Create multiple test objects
    std::vector< Serializable*> objects1{
        new Obj1(1, "text 1"),
        new Obj1(2, "text 2"),
        new Obj2(3, "text 3", 10, "1000"),
        new Obj1(4, "text 4"),
    };

    // Save the objects
    std::string data = serialized(objects1);

    // Prints the correct parsed structure
    std::cout << "serialized 1:\n" << data << "\n";

    // Create the same objects as before but with different data to be able to 
    // verify that loading was successful
    std::vector< Serializable*> objects2{
        new Obj1(0, ""),
        new Obj1(0, ""),
        new Obj2(0, "", 0, ""),
        new Obj1(0, ""),
    };
    // Try to load the objects back from the created string
    if (deserialize(objects2, data)) // is true if loading was successful
    {
        // Save the loaded objects back to check for any difference
        // expected(data2 == data)
        std::string data2 = serialized(objects2);
        std::cout << "serialized 2:\n" << data2 << "\n";

        if (data == data2)
            std::cout << "PASS\n";
        else
            std::cout << "FAIL\n";
    }
    return 0;
}
void read(const std::string& buffer, JsonArray &array)
{
	// Try to load back to an array
	auto err = glz::read_json(array, buffer);
}
std::string write(const JsonArray& array)
{
    std::string buffer;
    glz::write < glz::opts{ .prettify = true } > (array, buffer);
    return buffer;
}
std::string serialized(const std::vector< Serializable*>& objs)
{
    JsonArray jsonList;
    jsonList.reserve(objs.size());

    for (auto& obj : objs)
    {
        // collect all json object structures
        JsonObject data;
        obj->save(data);
        jsonList.push_back((data));
    }
    // parse the array to a string
    //std::string buffer;
    return write(jsonList);
    //glz::write < glz::opts{ .prettify = true } > (jsonList, buffer);
    //return buffer;
}

bool deserialize(const std::vector<Serializable*>& objs, const std::string& buffer)
{
    JsonArray readed;
    read(buffer, readed);
    // Try to load back to an array
   // auto err = glz::read_json(readed, buffer);

    // Check the loaded object count 
    if (readed.size() != objs.size())
    {
        std::cout << "\nsize mismatch: " << readed.size() << " != " << objs.size() << "\n";
        return false;
    }

    bool success = true;
    for (size_t i = 0; i < objs.size(); ++i)
    {
        // Load each object back
        auto el = readed[i].get<JsonObject>();
        success &= objs[i]->load(el);
    }
    return success;
}
#else
#include <QApplication>
#include <iostream>
#include "UIWrapper.h"
#include "JsonDatabase.h"



int main(int argc, char* argv[])
{
	QApplication a(argc, argv);
	Profiler::startProfiler();
	bool ret;
	{
		UIWrapper wrapper;
		ret = a.exec();
	}
	Profiler::stopProfiler("profiler.prof");
	return ret;
}
#endif