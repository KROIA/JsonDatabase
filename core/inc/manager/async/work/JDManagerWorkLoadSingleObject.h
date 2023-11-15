#pragma once

#include "JD_base.h"
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
				JDObjectInterface* object);
			~JDManagerAysncWorkLoadSingleObject();

			bool hasSucceeded() const override;
			JDObjectInterface * getObject() const;
			void process() override;
			std::string getErrorMessage() const override;
			WorkType getWorkType() const override;


		private:
			JDObjectInterface* m_object;
			bool m_success;
		};
	}
}