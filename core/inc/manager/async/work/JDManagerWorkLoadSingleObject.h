#pragma once

#include "JsonDatabase_base.h"
#include "manager/async/JDManagerAsyncWork.h"

namespace JsonDatabase
{
    namespace Internal
    {
		class JDManagerAysncWorkLoadSingleObject : public JDManagerAysncWork
		{
		public:
			JDManagerAysncWorkLoadSingleObject(
				JDManager& manager,
				std::mutex& mtx,
				const JDObject& object);
			~JDManagerAysncWorkLoadSingleObject();

			bool hasSucceeded() const override;
			const JDObject& getObject() const;
			void process() override;
			std::string getErrorMessage() const override;
			WorkType getWorkType() const override;


		private:
			JDObject m_object;
			bool m_success;
		};
	}
}