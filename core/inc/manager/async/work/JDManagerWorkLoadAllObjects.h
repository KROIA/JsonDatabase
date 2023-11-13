#pragma once

#include "manager/async/JDManagerAsyncWork.h"

namespace JsonDatabase
{
	namespace Internal
	{
		class JDManagerAysncWorkLoadAllObjects : public JDManagerAysncWork
		{
		public:
			JDManagerAysncWorkLoadAllObjects(
				JDManager& manager,
				std::mutex& mtx,
				int mode);
			~JDManagerAysncWorkLoadAllObjects();


			bool hasSucceeded() const override;
			void process() override;
			std::string getErrorMessage() const override;
		private:
			bool m_success;
			int m_loadMode;
		};
	}
}