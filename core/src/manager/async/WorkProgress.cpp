#include "manager/async/WorkProgress.h"

namespace JsonDatabase
{
    namespace Internal
    {
        WorkProgress::WorkProgress()
            : m_scalar(1)
            , m_progress(0)
            , m_subProgress(0)
            , m_taskText("")
            , m_comment("")
        {

        }
        WorkProgress::~WorkProgress()
        {

        }

        void WorkProgress::setProgress(double percent)
        {
            m_subProgress = percent * m_scalar;
            if(m_progress + m_subProgress > 1)
                m_subProgress = 1 - m_progress;
        }
        void WorkProgress::addProgress(double percent)
        {
            m_subProgress += percent * m_scalar;
            if (m_progress + m_subProgress > 1)
                m_subProgress = 1 - m_progress;
        }
        void WorkProgress::startNewSubProgress(double range)
        {
            setScalar(range);
            m_progress += m_subProgress;
            m_subProgress = 0;
        }
        void WorkProgress::setCompleted()
        {
			m_progress = 1;
            m_subProgress = 0;
        }
        void WorkProgress::setSubProgressCompleted()
        {
            setProgress(1);
        }
        void WorkProgress::setTaskName(const std::string& name)
        {
            m_taskText = name;
        }
        void WorkProgress::setComment(const std::string& comment)
        {
            m_comment = comment;
        }

        void WorkProgress::setScalar(double scalar)
        {
            m_scalar = scalar;
        }
        double WorkProgress::getScalar() const
        {
            return m_scalar;
        }




        double WorkProgress::getProgress() const
        {
            return m_progress + m_subProgress;
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