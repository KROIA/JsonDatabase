#pragma once


#include "JD_base.h"
#include <string>
#include <vector>

namespace JsonDatabase
{
    namespace Internal
    {
        class JSONDATABASE_EXPORT WorkProgress
        {
        public:
            WorkProgress();
            ~WorkProgress();

            void setProgress(double percent);
            void addProgress(double percent);
            void setTaskName(const std::string& name);
            void setComment(const std::string& comment);


            double getProgress() const;
            const std::string& getTaskText() const;
            const std::string& getComment() const;

        protected:

            double m_progress; // Percent value
            std::string m_taskText;
            std::string m_comment;
        };
    }
}