#pragma once

#include "JD_base.h"
#include "JDDeclaration.h"


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


		private:
			JDObject m_obj;
			JDObjectIDptr m_id;
			Lockstate m_lockstate;
			ChangeState m_changestate;
		};
	}
}