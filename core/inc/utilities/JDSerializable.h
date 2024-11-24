#pragma once

#include "JsonDatabase_base.h"
#include "JsonDatabase_Declaration.h"
#include "object/JDObjectID.h"
#include "Json/JsonValue.h"




namespace JsonDatabase
{
	namespace Utilities
	{
		class JSON_DATABASE_EXPORT JDSerializable
		{
		public:
			virtual ~JDSerializable() {}



			/**
			 * @brief 
			 * Gets called when the object is loaded from the database
			 * Eead back the data from the json object
			 * @param obj 
			 * @return true if all data was read successfully, otherwise false
			 */
			virtual bool load(const JsonObject& obj) = 0;

			/**
			 * @brief 
			 * Gets called when the object is saved to the database
			 * Write the data to the json object
			 * @param obj in which the data should be written
			 * @return true if all data was written successfully, otherwise false
			 */
			virtual bool save(JsonObject& obj) const = 0;

			// Helper functions to get values from a json object

			/**
			 * @brief 
			 * Gets a JsonObject value from a json object with a given key
			 * @param obj that contains the <key> for the searched value
			 * @param value is a out parameter that will contain the value if it was found
			 * @param key that is used to search for the value
			 * @return true if the value was found and written to <value>, otherwise false
			 */
			static bool getJsonValue(const JsonObject& obj, JsonObject& value, const std::string& key);

			/**
			 * @brief
			 * Gets a JsonArray value from a json object with a given key
			 * @param obj that contains the data
			 * @param value to store the array value
			 * @param key to search for the value
			 * @return true if the key was found and its array value was written to <value>, otherwise false
			 */
			static bool getJsonValue(const JsonObject& obj, JsonArray& value, const std::string& key);

			/**
			 * @brief 
			 * Gets a string value from a json object with a given key
			 * @param obj that contains the data
			 * @param value to store the string value
			 * @param key to search for the value
			 * @return true if the key was found and its string value was written to <value>, otherwise false
			 */
			static bool getJsonValue(const JsonObject& obj, QString& value, const std::string& key);

			/**
			 * @brief
			 * Gets a string value from a json object with a given key
			 * @param obj that contains the data
			 * @param value to store the string value
			 * @param key to search for the value
			 * @return true if the key was found and its string value was written to <value>, otherwise false
			 */
			static bool getJsonValue(const JsonObject& obj, std::string& value, const std::string& key);

			/**
			 * @brief 
			 * Gets a long value from a json object with a given key
			 * @param obj that contains the data
			 * @param value to store the long value
			 * @param key to search for the value
			 * @return true if the key was found and its long value was written to <value>, otherwise false
			 */
			static bool getJsonValue(const JsonObject& obj, long& value, const std::string& key);

			/**
			 * @brief 
			 * Gets a double value from a json object with a given key
			 * @param obj that contains the data
			 * @param value to store the double value
			 * @param key to search for the value
			 * @return true if the key was found and its double value was written to <value>, otherwise false
			 */
			static bool getJsonValue(const JsonObject& obj, double& value, const std::string& key);

			/**
			 * @brief
			 * Gets a float value from a json object with a given key
			 * @param obj that contains the data
			 * @param value to store the float value
			 * @param key to search for the value
			 * @return true if the key was found and its float value was written to <value>, otherwise false
			 */ 
			static bool getJsonValue(const JsonObject& obj, float& value, const std::string& key);

			/**
			 * @brief
			 * Gets a bool value from a json object with a given key
			 * @param obj that contains the data
			 * @param value to store the bool value
			 * @param key to search for the value
			 * @return true if the key was found and its bool value was written to <value>, otherwise false
			 */
			static bool getJsonValue(const JsonObject& obj, bool& value, const std::string& key);

		protected:

		};
	}
}
