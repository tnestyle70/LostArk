#include "CpuJobPool.h"

#pragma push_macro("new")
#undef new
#include <Windows.h>
#include <algorithm>
#include <atomic>
#include <exception>
#include <mutex>
#include <stdexcept>
#pragma pop_macro("new")

#include "Profiler.h"

namespace
{
    thread_local std::uint32_t g_CpuJobDepth = 0u;

    struct CPU_JOB_LANE final
    {
        CPU_JOB_LANE() noexcept { ++g_CpuJobDepth; }
        ~CPU_JOB_LANE() { --g_CpuJobDepth; }
    };

    Engine::CPU_JOB_STATS RunSerial(const std::size_t count, void* context,
        void (*execute)(void*, std::size_t))
    {
        CPU_JOB_LANE lane;
        Engine::CPU_JOB_STATS stats;
        for (std::size_t index = 0u; index < count; ++index)
        {
            execute(context, index);
            ++stats.CallerJobs;
        }
        return stats;
    }

    struct CPU_JOB_BATCH final
    {
        std::size_t count;
        void* context;
        void (*execute)(void*, std::size_t);
        Engine::CProfiler* profiler;
        const char* workerScope;
        const char* joinScope;
        std::atomic_size_t next{0u};
        std::atomic_size_t workerJobs{0u};
        std::size_t callerJobs = 0u;
        std::atomic_bool failed{false};
        std::exception_ptr error;

        void RecordException() noexcept
        {
            // The winning writer also joins before the owner reads error.
            if (!failed.exchange(true, std::memory_order_acq_rel))
                error = std::current_exception();
        }

        void Drain(const bool worker) noexcept
        {
            CPU_JOB_LANE lane;
            try
            {
                while (!failed.load(std::memory_order_acquire))
                {
                    // Stop at count without wrapping a size_t cursor at SIZE_MAX.
                    std::size_t index = next.load(std::memory_order_relaxed);
                    while (index < count && !next.compare_exchange_weak(index, index + 1u,
                        std::memory_order_relaxed)) {}
                    if (index >= count) return;
                    execute(context, index);
                    if (worker) workerJobs.fetch_add(1u, std::memory_order_relaxed);
                    else ++callerJobs;
                }
            }
            catch (...) { RecordException(); }
        }
    };

    class CPU_JOB_POOL final
    {
    public:
        CPU_JOB_POOL()
        {
            const DWORD processors = GetActiveProcessorCount(ALL_PROCESSOR_GROUPS);
            m_MaximumAssistants = processors > 2u ? (std::min)(3u,
                static_cast<std::uint32_t>(processors - 2u)) : 0u;
            if (!m_MaximumAssistants) return;
            m_Pool = CreateThreadpool(nullptr);
            if (!m_Pool) return;
            SetThreadpoolThreadMaximum(m_Pool, m_MaximumAssistants);
            if (!SetThreadpoolThreadMinimum(m_Pool, m_MaximumAssistants))
            {
                CloseThreadpool(m_Pool);
                m_Pool = nullptr;
                return;
            }
            InitializeThreadpoolEnvironment(&m_Environment);
            SetThreadpoolCallbackPool(&m_Environment, m_Pool);
            m_Work = CreateThreadpoolWork(&RunWorker, this, &m_Environment);
            if (!m_Work)
            {
                DestroyThreadpoolEnvironment(&m_Environment);
                CloseThreadpool(m_Pool);
                m_Pool = nullptr;
            }
        }

        ~CPU_JOB_POOL()
        {
            std::lock_guard lock(m_SubmissionMutex);
            if (!m_Work) return;
            WaitForThreadpoolWorkCallbacks(m_Work, TRUE);
            CloseThreadpoolWork(m_Work);
            DestroyThreadpoolEnvironment(&m_Environment);
            CloseThreadpool(m_Pool);
        }

        Engine::CPU_JOB_STATS Run(CPU_JOB_BATCH& batch, const std::uint32_t requested)
        {
            std::unique_lock lock(m_SubmissionMutex);
            if (!m_Work) return RunSerial(batch.count, batch.context, batch.execute);
            const auto assistants = static_cast<std::uint32_t>((std::min)({
                std::size_t(requested), std::size_t(m_MaximumAssistants), batch.count - 1u}));
            m_ActiveBatch.store(&batch, std::memory_order_release);
            struct CALLBACK_JOIN final
            {
                CPU_JOB_POOL& pool;
                bool joined = false;
                void Wait() noexcept
                {
                    if (joined) return;
                    WaitForThreadpoolWorkCallbacks(pool.m_Work, FALSE);
                    pool.m_ActiveBatch.store(nullptr, std::memory_order_release);
                    joined = true;
                }
                ~CALLBACK_JOIN() { Wait(); }
            } join{*this};
            for (std::uint32_t i = 0u; i < assistants; ++i)
                SubmitThreadpoolWork(m_Work);
            batch.Drain(false);
            {
                Engine::CProfilerScope wait(batch.profiler, batch.joinScope);
                join.Wait();
            }
            if (batch.error) std::rethrow_exception(batch.error);
            return {batch.callerJobs, batch.workerJobs.load(std::memory_order_relaxed), assistants};
        }

    private:
        static void CALLBACK RunWorker(PTP_CALLBACK_INSTANCE, void* context, PTP_WORK) noexcept
        {
            auto& pool = *static_cast<CPU_JOB_POOL*>(context);
            CPU_JOB_BATCH* batch = pool.m_ActiveBatch.load(std::memory_order_acquire);
            if (!batch) return;
            try
            {
                Engine::CProfilerScope profile(batch->profiler, batch->workerScope);
                batch->Drain(true);
            }
            catch (...) { batch->RecordException(); }
        }

        std::mutex m_SubmissionMutex;
        std::atomic<CPU_JOB_BATCH*> m_ActiveBatch{nullptr};
        PTP_POOL m_Pool = nullptr;
        TP_CALLBACK_ENVIRON m_Environment{};
        PTP_WORK m_Work = nullptr;
        std::uint32_t m_MaximumAssistants = 0u;
    };
}

Engine::CPU_JOB_STATS Engine::Run_CpuJobs(const std::size_t count, void* context,
    void (*execute)(void*, std::size_t), const std::uint32_t maxAssistants,
    CProfiler* profiler, const char* workerScope, const char* joinScope)
{
    if (!count) return {};
    if (!execute) throw std::invalid_argument("CPU job callback is missing.");
    if (count == 1u || maxAssistants == 0u || g_CpuJobDepth != 0u)
        return RunSerial(count, context, execute);
    static CPU_JOB_POOL pool;
    CPU_JOB_BATCH batch{count, context, execute, profiler,
        workerScope ? workerScope : "CPU.Jobs.Worker", joinScope ? joinScope : "CPU.Jobs.Join"};
    return pool.Run(batch, maxAssistants);
}
