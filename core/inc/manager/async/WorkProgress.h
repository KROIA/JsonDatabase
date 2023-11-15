#pragma once


#include "JD_base.h"
#include <string>

namespace JsonDatabase
{
    namespace Internal
    {
        class JSONDATABASE_EXPORT WorkProgress
        {
        public:
            
            void setProgress(double percent);
            void setTaskName(const std::string& name);

            double getProgress() const;
            const std::string& getTaskText() const;

        protected:
            double m_progress; // Percent value
            std::string m_taskText;
        };
    }
}