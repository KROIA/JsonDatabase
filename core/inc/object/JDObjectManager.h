#pragma once

#include "JD_base.h"
#include "JDDeclaration.h"
#include "JDObjectID.h"

#ifdef JD_USE_QJSON
#include <QJsonObject>
#else
#include "Json/JsonValue.h"
#endif


namespace JsonDatabase
{
	namespace Internal
	{
		class JSONDATABASE_EXPORT JDObjectManager
		{
			friend JDManagerObjectManager;

			JDObjectManager(const JDObject& obj, const JDObjectIDptr& id);
			~JDObjectManager();
		public:
			

			const JDObjectIDptr &getID() const;
			const JDObject &getObject() const;
			Lockstate getLockstate() const;
			ChangeState getChangeState() const;

#ifdef JD_USE_QJSON
			static bool getJsonArray(const std::vector<JDObject>& objs, std::vector<QJsonObject>& jsonOut);
			static bool getJsonArray(const std::vector<JDObject>& objs, std::vector<QJsonObject>& jsonOut,
				WorkProgress* progress);
#else
			static bool getJsonArray(const std::vector<JDObject>& objs, JsonArray& jsonOut);
			static bool getJsonArray(const std::vector<JDObject>& objs, JsonArray& jsonOut,
				WorkProgress* progress);
#endif
			
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
			
#ifdef JD_USE_QJSON
			static ManagedLoadStatus managedLoad(
				const QJsonObject& json, 
				JDObjectManager* manager, 
				ManagedLoadContainers& containers, 
				const ManagedLoadMode &loadMode,
				const ManagedLoadMisc &misc);


			bool loadAndOverrideData(const QJsonObject& json);
			bool loadAndOverrideDataIfChanged(const QJsonObject& json, bool &hasChangesOut);

			static JDObjectManager* instantiateAndLoadObject(const QJsonObject& json, const JDObjectIDptr& id);
			static JDObjectManager* cloneAndLoadObject(const JDObject& original, const QJsonObject& json, const JDObjectIDptr& id);
#else
			static ManagedLoadStatus managedLoad(
				const JsonValue& json, 
				JDObjectManager* manager, 
				ManagedLoadContainers& containers/*,
				const ManagedLoadMode& loadMode*/,
				const ManagedLoadMisc& misc);

			bool loadAndOverrideData(const JsonValue& json);
			bool loadAndOverrideDataIfChanged(const JsonValue& json, bool& hasChangesOut);

			static JDObjectManager* instantiateAndLoadObject(const JsonValue& json, const JDObjectIDptr& id);
			static JDObjectManager* cloneAndLoadObject(const JDObject &original, const JsonValue& json, const JDObjectIDptr& id);
#endif

#ifdef JD_USE_QJSON
			static ManagedLoadStatus managedLoadExisting_internal(
				const QJsonObject& json, 
				JDObjectManager* manager,
				ManagedLoadContainers& containers,
				const ManagedLoadMode& loadMode/*,
				const ManagedLoadMisc& misc*/ );

			static ManagedLoadStatus managedLoadNew_internal(
				const QJsonObject& json,
				ManagedLoadContainers& containers/*,
				const ManagedLoadMode& loadMode*/,
				const ManagedLoadMisc& misc);


			static bool deserializeOverrideFromJsonIfChanged_internal(const QJsonObject& json, JDObject obj, bool& hasChangedOut);
			static bool deserializeOverrideFromJson_internal(const QJsonObject& json, JDObject obj);
#else
			static ManagedLoadStatus managedLoadExisting_internal(
				const JsonValue& json,
				JDObjectManager* manager,
				ManagedLoadContainers& containers,
				const ManagedLoadMode& loadMode/*,
				const ManagedLoadMisc& misc*/);

			static ManagedLoadStatus managedLoadNew_internal(
				const JsonValue& json,
				ManagedLoadContainers& containers,
				const ManagedLoadMode& loadMode,
				const ManagedLoadMisc& misc);

			static bool deserializeOverrideFromJsonIfChanged_internal(const JsonValue& json, JDObject obj, bool& hasChangedOut);
			static bool deserializeOverrideFromJson_internal(const JsonValue& json, JDObject obj);
#endif

			JDObject m_obj;
			JDObjectIDptr m_id;
			Lockstate m_lockstate;
			ChangeState m_changestate;
		};
	}
}