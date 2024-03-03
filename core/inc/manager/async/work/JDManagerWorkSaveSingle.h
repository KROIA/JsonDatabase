#pragma once

#include "JD_base.h"
#include "manager/async/JDManagerAsyncWork.h"

namespace JsonDatabase
{
	namespace Internal
	{
		class JDManagerAysncWorkSaveSingle : public JDManagerAysncWork
		{
		public:
			JDManagerAysncWorkSaveSingle(
				JDManager& manager,
				std::mutex& mtx,
				const JDObject &object);
			~JDManagerAysncWorkSaveSingle();

			bool hasSucceeded() const override;
			void process() override;
			std::string getErrorMessage() const override;
			WorkType getWorkType() const override;

		private:
			JDObject m_object;
			bool m_success;
		};
	}
}