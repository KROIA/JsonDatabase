#include "manager/async/WorkProgress.h"


namespace JsonDatabase
{
    namespace Internal
    {

        void WorkProgress::setProgress(double percent)
        {
            m_progrss = percent;
        }
        void WorkProgress::setTaskName(const std::string& name)
        {
            m_taskName = name;
        }

        double WorkProgress::getProgress() const
        {
            return m_progress;
        }
        const std::string& WorkProgress::getTaskText() const
        {
            return m_taskText;
        }
    }
}