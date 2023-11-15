#include "manager/async/WorkProgress.h"

namespace JsonDatabase
{
    namespace Internal
    {
        WorkProgress::WorkProgress()
            : m_progress(0)
        {

        }
        WorkProgress::~WorkProgress()
        {

        }

        void WorkProgress::setProgress(double percent)
        {
            m_progress = percent;
            if(m_progress > 1)
                m_progress = 1;
        }
        void WorkProgress::addProgress(double percent)
        {
            m_progress += percent;
            if (m_progress > 1)
                m_progress = 1;
        }
        void WorkProgress::setTaskName(const std::string& name)
        {
            m_taskText = name;
        }
        void WorkProgress::setComment(const std::string& comment)
        {
            m_comment = comment;
        }




        double WorkProgress::getProgress() const
        {
            return m_progress;
        }
        const std::string& WorkProgress::getTaskText() const
        {
            return m_taskText;
        }
        const std::string& WorkProgress::getComment() const
        {
            return m_comment;
        }

    }
}