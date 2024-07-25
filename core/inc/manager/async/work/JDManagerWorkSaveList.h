#pragma once

#include "JsonDatabase_base.h"
#include "manager/async/JDManagerAsyncWork.h"
#include <vector>

#include "Logger.h"

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
				const std::vector<JDObject>& objects,
				Log::Logger::ContextLogger *parentLogger);
			~JDManagerAysncWorkSaveList();

			bool hasSucceeded() const override;
			void process() override;
			std::string getErrorMessage() const override;
			WorkType getWorkType() const override;


		private:
			Log::Logger::ContextLogger* m_logger = nullptr;
			std::vector<JDObject> m_objects;
			bool m_success;
		};
	}
}