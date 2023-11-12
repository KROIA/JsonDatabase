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
				JDObjectInterface* object);
			~JDManagerAysncWorkSaveSingle();

			bool hasSucceeded() const;
			JDObjectInterface * getObject() const;
			void process() override;

		private:
			JDObjectInterface* m_object;
			bool m_success;
		};
	}
}