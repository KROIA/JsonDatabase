#pragma once

#include "JD_base.h"
#include "manager/async/JDManagerAsyncWork.h"
#include <vector>

namespace JsonDatabase
{
	namespace Internal
	{
		class JDManagerAysncWorkSaveList : public JDManagerAysncWork
		{
		public:
			JDManagerAysncWorkSaveList(
				JDManager& manager,
				std::mutex& mtx,
				const std::vector<JDObjectInterface*>& objects);
			~JDManagerAysncWorkSaveList();

			bool hasSucceeded() const override;
			void process() override;
			std::string getErrorMessage() const override;
			WorkType getWorkType() const override;


		private:
			std::vector<JDObjectInterface*> m_objects;
			bool m_success;
		};
	}
}