#pragma once

#include "JD_base.h"
#include <thread>
#include <functional>
#include <condition_variable>


// Profiling
#ifdef BUILD_WITH_EASY_PROFILER
// Enable profiling of this lib
#define TH_PROFILING

// Change the base color for the profiler. base color can be found in profiler::colors from profiler_clolors.h
#define TH_PROFILING_BASE_COLOR Blue
#endif

#ifdef TH_PROFILING
#include "easy/profiler.h"

// Special expantion tecniques are required to combine the color name
#define CONCAT_SYMBOLS_IMPL(x, y) x##y
#define CONCAT_SYMBOLS(x, y) CONCAT_SYMBOLS_IMPL(x, y)
#define TH_PROFILING_COLOR(stage) profiler::colors:: CONCAT_SYMBOLS(TH_PROFILING_BASE_COLOR , stage)

#define TH_PROFILING_BLOCK_C(text, colorStage) EASY_BLOCK(text, colorStage)
#define TH_PROFILING_FUNCTION_C(colorStage) EASY_FUNCTION(colorStage)
#define TH_PROFILING_BLOCK(text, colorStage) TH_PROFILING_BLOCK_C(text, TH_PROFILING_COLOR(colorStage))
#define TH_PROFILING_FUNCTION(colorStage) TH_PROFILING_FUNCTION_C(TH_PROFILING_COLOR(colorStage))
#define TH_PROFILING_THREAD(name) EASY_THREAD(name)
#define TH_PROFILING_END_BLOCK EASY_END_BLOCK
#else
#define TH_PROFILING_BLOCK(text, color)
#define TH_PROFILING_FUNCTION(color)
#define TH_PROFILING_END_BLOCK
#define TH_PROFILING_THREAD(name)
#endif

namespace JsonDatabase
{
    class ThreadWorker;
    class JSONDATABASE_EXPORT ThreadWork
    {
        friend ThreadWorker;
    public:
        ThreadWork();
        virtual ~ThreadWork();

        int getThreadIndex() const;

    protected:
        virtual void process(int threadIndex) = 0;

    private:
        int m_threadIndex;
    };

    class JSONDATABASE_EXPORT ThreadWorker {
    public:
        ThreadWorker(const std::string &name = "ThreadWorker");
        ThreadWorker(ThreadWork* work, const std::string& name = "ThreadWorker");
        ThreadWorker(const std::vector<ThreadWork*>& workload, const std::string& name = "ThreadWorker");
        ThreadWorker(const std::function<void()>& executionFunction, const std::string& name = "ThreadWorker");
        ThreadWorker(const std::vector<std::function<void()>>& executionFunctions, const std::string& name = "ThreadWorker");
        ~ThreadWorker();

        void setWaitingFunc(const std::function<void()>& func);

        void addFunction(const std::function<void()>& function);
        void addWork(ThreadWork* work);

        void signalExecution(bool blocking = false);
        void waitForThreads();
        void stop();

        unsigned int getActiveThreadCount() const;
        bool threadFinished(unsigned int index = 0) const;
        size_t getThreadCount() const;

    private:
        class JSONDATABASE_EXPORT ThreadData
        {
        public:
            ThreadData();
            ThreadData(const ThreadData& other);

            ThreadWork* work;
            std::function<void()>* func;
            std::thread* thread;
            //void* param;
            std::atomic<bool> shouldExecute;
            std::atomic<bool> threadExited;
            std::atomic<bool> threadCrashed;
            int threadIndex = 0;
            //std::mutex *mutex = nullptr;
        };
        void threadFunction(ThreadData &data);

        std::string m_name;
        std::vector<ThreadData*> m_threads;
        std::mutex m_mutex;
        std::condition_variable m_conditionVar;
        std::function<void()>* m_sleepWorkFunc = nullptr; // function gets called while waiting for threads

        std::atomic<bool> m_run = true;
    };
}