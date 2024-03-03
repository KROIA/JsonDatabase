#pragma once

#include "JD_base.h"
#include "object/JDObjectInterface.h"
#include "utilities/JDSerializable.h"
#include "utilities/filesystem/FileChangeWatcher.h"
#include "utilities/JDUser.h"
#include "utilities/filesystem/AbstractRegistry.h"


#include <string>
#include <vector>
#include "Json/JsonValue.h"
#include <mutex>

namespace JsonDatabase
{
	namespace Internal
	{
		class JSONDATABASE_EXPORT JDObjectLocker : public Utilities::AbstractRegistry
		{
			//friend JDManagerObjectManager;
		public:
			enum Error
			{
				none,
				objIsNullptr,
				cantOpenTableFile,
				tableFileNotExist,
				corruptTableData,
				unableToLock,
				lockedByOther,
				programmingError,
			};

		protected:
		
			
			//bool setup(Error &err);
		public:
			JDObjectLocker(JDManager& manager, std::mutex& mtx);
			~JDObjectLocker();

			bool lockObject(const JDObject & obj, Error& err);
			bool unlockObject(const JDObject & obj, Error& err);
			bool unlockAllObjs(Error& err);
			bool isObjectLocked(const JDObject & obj, Error& err) const;
			bool isObjectLockedByMe(const JDObject & obj, Error& err) const;
			bool isObjectLockedByOther(const JDObject & obj, Error& err) const;

			
			struct LockData
			{
				JDObjectID::IDType objectID;
				Utilities::JDUser user;
				QDate lockDate;
				QTime lockTime;

				LockData() : objectID(JDObjectID::invalidID) {}
			};
			bool getLockedObjects(std::vector<LockData>& lockedObjectsOut, Error& err) const;
			int removeInactiveObjectLocks() const;


			const std::string& getErrorStr(Error err) const;

			struct JsonKeys
			{
				static const std::string objectID;
				static const std::string user;
				static const std::string lockDate;
				static const std::string lockTime;
			};

			void update();
		protected:
			ManagedFileChangeWatcher& getLockTableFileWatcher();
			
			//void onDatabasePathChange(const std::string& oldPath, const std::string& newPath, Error& err) const;


		private:
			class JSONDATABASE_EXPORT LockEntryObjectImpl : public LockEntryObject
			{
			public:
				LockEntryObjectImpl(const std::string& key);
				LockEntryObjectImpl(const std::string& key, const JDObject& obj, const JDManager& manager);
				~LockEntryObjectImpl();


				void setObject(const JDObject& obj, const JDManager& manager);
				std::string toString() const;
				

				bool load(const JsonObject& obj) override;
				bool save(JsonObject& obj) const override;
				static bool isValid(const JsonObject& lock);

			
				LockData data;
				JDObject obj;
			private:
			};


			void onCreateFiles() override;
			void onDatabasePathChangeStart(const std::string& newPath) override;
			void onDatabasePathChangeEnd() override;
			void onNameChange(const std::string& newName) override;

			JDManager& m_manager;
			std::mutex& m_mutex;
			std::string m_lockTableFile;
			unsigned int m_registryOpenTimeoutMs;
			mutable bool m_useSpecificDatabasePath;
			mutable std::string m_specificDatabasePath;
			
			mutable ManagedFileChangeWatcher m_lockTableWatcher;
		};
	}
}