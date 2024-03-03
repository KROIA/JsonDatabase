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
            void startNewSubProgress(double range);
            void setCompleted();
            void setSubProgressCompleted();
            void setTaskName(const std::string& name);
            void setComment(const std::string& comment);

            void setScalar(double scalar);
            double getScalar() const;

            double getProgress() const;
            const std::string& getTaskText() const;
            const std::string& getComment() const;

        protected:
            double m_scalar; // 0.0 - 1.0
            double m_subProgress; 
            double m_progress; // Percent value
            std::string m_taskText;
            std::string m_comment;
        };
    }
}