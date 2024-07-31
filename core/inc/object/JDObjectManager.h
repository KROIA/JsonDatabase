#pragma once

#include "JsonDatabase_base.h"
#include "JsonDatabase_Declaration.h"
#include "JDObjectID.h"


#include "Json/JsonValue.h"

#include "Logger.h"



namespace JsonDatabase
{
	namespace Internal
	{
		class JSON_DATABASE_EXPORT JDObjectManager
		{
			friend JDManagerObjectManager;

			JDObjectManager(const JDObject& obj, const JDObjectIDptr& id, Log::LogObject* parentLogger);
			~JDObjectManager();
		public:
			

			const JDObjectIDptr &getID() const;
			const JDObject &getObject() const;
			Lockstate getLockstate() const;
			ChangeState getChangeState() const;


			static bool getJsonArray(const std::vector<JDObject>& objs, JsonArray& jsonOut);
			static bool getJsonArray(const std::vector<JDObject>& objs, JsonArray& jsonOut,
				WorkProgress* progress);
			
		private:
			enum ManagedLoadStatus
			{
				success,
				noLoadNeeded,
				loadFailed,
				loadFailed_UnknownObjectType,
				loadFailed_IncompleteData,
			};
			static const std::string &managedLoadStatusToString(ManagedLoadStatus status);

			struct ManagedLoadContainers
			{
				/*
					These objects are objects that are already in the database.
					These objects are loaded from the database.
				*/
				std::vector<JDObject>& overridingObjs;

				/*
					Pair of ID's and objects.
					The objects need to be added with the exact same ID's as the ones in the pair.
					These objects are new objects that need to be added to the database.
				*/
				std::vector<JDObjectID::IDType>& newObjIDs;
				std::vector<JDObject>& newObjInstances;

				/*
					These objects are used in the "objectChangedFromDatabase" signal. 
				*/
				std::vector<JDObjectPair>& changedPairs;

				/*
					These objects are new instances of objects that are already in the database.
					The data was different and the mode was not set to "override".
					These objects are the replacements for the orignal objects in the database.
				*/
				std::vector<JDObject> &replaceObjs;

				/*
					This is a complete list of all successfully loaded objects.
					The list is used to check for which objects got deleted from the database file.
				*/
				std::unordered_map<JDObject, JDObject>& loadedObjects;
			};
			struct ManagedLoadMode
			{
				bool newObjects;
				bool removedObjects;
				bool changedObjects;
				bool overridingObjects;
			};
			struct ManagedLoadMisc
			{
				JDObjectID::IDType id = JDObjectID::invalidID;
				//bool hasOverrideChangeFromDatabaseSlots;
			};
			

			static ManagedLoadStatus managedLoad(
				const JsonObject& json,
				JDObjectManager* manager, 
				ManagedLoadContainers& containers,
				const ManagedLoadMode& loadMode,
				const ManagedLoadMisc& misc,
				Log::LogObject* logger);

			bool loadAndOverrideData(const JsonObject& json);
			bool loadAndOverrideDataIfChanged(const JsonObject& json, bool& hasChangesOut);

			static JDObjectManager* instantiateAndLoadObject(const JsonObject& json, const JDObjectIDptr& id, Log::LogObject * parentLogger);
			static JDObjectManager* cloneAndLoadObject(const JDObject &original, const JsonObject& json, const JDObjectIDptr& id, Log::LogObject* parentLogger);

			static ManagedLoadStatus managedLoadExisting_internal(
				const JsonObject& json,
				JDObjectManager* manager,
				ManagedLoadContainers& containers,
				const ManagedLoadMode& loadMode,
				Log::LogObject *logger);

			static ManagedLoadStatus managedLoadNew_internal(
				const JsonObject& json,
				ManagedLoadContainers& containers/*,
				const ManagedLoadMode& loadMode*/,
				const ManagedLoadMisc& misc,
				Log::LogObject* logger);

			bool deserializeOverrideFromJsonIfChanged_internal(const JsonObject& json, JDObject obj, bool& hasChangedOut);
			bool deserializeOverrideFromJson_internal(const JsonObject& json, JDObject obj);

			Log::LogObject* m_logger = nullptr;
			JDObject m_obj;
			JDObjectIDptr m_id;
			Lockstate m_lockstate;
			ChangeState m_changestate;
		};
	}
}
