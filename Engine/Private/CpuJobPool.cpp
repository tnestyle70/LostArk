// 외부에 공개한 동기식 CPU 작업 분배 함수와 통계 구조체의 구현이다.
#include "CpuJobPool.h"

// 전용 Windows thread pool의 생성, callback 제출, 완료 대기를 사용한다.
#include <Windows.h>
// CPU 수, 요청한 보조 실행자 수, 실제 작업 수 중 작은 값으로 병렬도를 제한한다.
#include <algorithm>
// 작업 인덱스 배분과 실패 통지, worker에 넘길 현재 batch 포인터를 동기화한다.
#include <atomic>
// worker에서 포착한 첫 예외를 호출 스레드의 join 이후 다시 전달한다.
#include <exception>
// 서로 다른 외부 호출이 같은 pool의 현재 batch를 동시에 교체하지 못하게 한다.
#include <mutex>
// 작업이 있는데 실행 callback이 없는 잘못된 입력을 invalid_argument로 보고한다.
#include <stdexcept>

// worker 실행 시간과 호출 스레드의 join 대기를 각각 측정한다.
#include "Profiler.h"

namespace
{
    // 각 참여 스레드의 중첩 깊이다. callback이 같은 pool을 재호출할 때 mutex 재진입 대기를 피한다.
    thread_local std::uint32_t g_CpuJobDepth = 0u;

    // 정상 반환과 예외 탈출 모두에서 참여 깊이를 복원하는 스코프 표식이다.
    struct CPU_JOB_LANE final
    {
        CPU_JOB_LANE() noexcept
        {
            ++g_CpuJobDepth;
        }
        ~CPU_JOB_LANE()
        {
            --g_CpuJobDepth;
        }
    };

    // 작업 하나, 병렬도 0, 중첩 호출 또는 pool 준비 실패에서 사용하는 직렬 경로다.
    // callback의 예외는 즉시 전파하며 앞서 완료한 작업은 되돌리지 않는다.
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

    // Run_CpuJobs의 스택에 놓이는 한 호출의 공유 상태다. join이 끝날 때까지 pool과 worker가 차용한다.
    struct CPU_JOB_BATCH final
    {
        // 입력은 호출 동안 불변이다. context와 측정기/문자열은 소유하지 않는다.
        std::size_t count;
        void* context;
        void (*execute)(void*, std::size_t);
        Engine::CProfiler* profiler;
        const char* workerScope;
        const char* joinScope;
        // next를 CAS로 선점하여 각 인덱스를 한 번만 실행한다. 호출 스레드의 통계는 한 스레드만 쓴다.
        std::atomic_size_t next{0u};
        std::atomic_size_t workerJobs{0u};
        std::size_t callerJobs = 0u;
        // 첫 실패만 error를 기록한다. error의 읽기는 모든 callback의 join 뒤에 수행한다.
        std::atomic_bool failed{false};
        std::exception_ptr error;

        void RecordException() noexcept
        {
            // 실패 표식을 처음 바꾼 실행자만 예외를 쓴다. 호출자는 이 실행자까지 join한 뒤 읽는다.
            if (!failed.exchange(true, std::memory_order_acq_rel))
            {
                error = std::current_exception();
            }
        }

        // 호출 스레드와 worker가 같은 인덱스 큐를 비운다. 실패 관측 뒤 새 작업을 중단하되 이미 실행 중인 작업은 마친다.
        void Drain(const bool worker) noexcept
        {
            CPU_JOB_LANE lane;
            try
            {
                while (!failed.load(std::memory_order_acquire))
                {
                    // count에서 멈춰 SIZE_MAX인 입력에서도 다음 인덱스의 덧셈이 순환하지 않게 한다.
                    std::size_t index = next.load(std::memory_order_relaxed);
                    while (index < count && !next.compare_exchange_weak(index, index + 1u,
                        std::memory_order_relaxed))
                    {
                    }
                    if (index >= count)
                    {
                        return;
                    }

                    execute(context, index);
                    if (worker)
                    {
                        workerJobs.fetch_add(1u, std::memory_order_relaxed);
                    }
                    else
                    {
                        ++callerJobs;
                    }
                }
            }
            catch (...)
            {
                RecordException();
            }
        }
    };

    // 함수 정적 객체 하나가 전용 OS pool을 소유한다. 각 호출의 batch는 반환 전까지 차용한다.
    class CCpuJobPool final
    {
    public:
        // CPU 여유를 남겨 최대 세 worker를 준비한다. 어느 생성 단계에서 실패해도 Run은 직렬 경로를 사용할 수 있다.
        CCpuJobPool()
        {
            const DWORD processors = GetActiveProcessorCount(ALL_PROCESSOR_GROUPS);
            m_iMaximumAssistants = processors > 2u ? (std::min)(3u,
                static_cast<std::uint32_t>(processors - 2u)) : 0u;
            if (!m_iMaximumAssistants)
            {
                return;
            }

            m_pPool = CreateThreadpool(nullptr);
            if (!m_pPool)
            {
                return;
            }

            SetThreadpoolThreadMaximum(m_pPool, m_iMaximumAssistants);
            if (!SetThreadpoolThreadMinimum(m_pPool, m_iMaximumAssistants))
            {
                CloseThreadpool(m_pPool);
                m_pPool = nullptr;
                return;
            }
            InitializeThreadpoolEnvironment(&m_Environment);
            SetThreadpoolCallbackPool(&m_Environment, m_pPool);
            m_pWork = CreateThreadpoolWork(&RunWorker, this, &m_Environment);
            if (!m_pWork)
            {
                DestroyThreadpoolEnvironment(&m_Environment);
                CloseThreadpool(m_pPool);
                m_pPool = nullptr;
            }
        }

        // 외부 제출과 겹치지 않게 잠근 뒤 대기 callback을 취소하고 실행 중인 callback을 기다려 OS 자원을 닫는다.
        ~CCpuJobPool()
        {
            std::lock_guard lock(m_SubmissionMutex);
            if (!m_pWork)
            {
                return;
            }

            WaitForThreadpoolWorkCallbacks(m_pWork, TRUE);
            CloseThreadpoolWork(m_pWork);
            DestroyThreadpoolEnvironment(&m_Environment);
            CloseThreadpool(m_pPool);
        }

        // 한 batch를 독점 제출한다. 반환이나 예외 전에 모든 worker가 끝나므로 호출자의 스택 batch가 살아 있다.
        Engine::CPU_JOB_STATS Run(CPU_JOB_BATCH& batch, const std::uint32_t requested)
        {
            std::unique_lock lock(m_SubmissionMutex);

            if (!m_pWork)
            {
                return RunSerial(batch.count, batch.context, batch.execute);
            }

            const auto assistants = static_cast<std::uint32_t>((std::min)({
                std::size_t(requested), std::size_t(m_iMaximumAssistants), batch.count - 1u}));
            // worker가 입력 초기화를 관측하도록 release/acquire로 batch를 게시한다.
            m_pActiveBatch.store(&batch, std::memory_order_release);
            // 대기와 현재 batch 해제를 한 번만 수행한다. 중간 예외로 스코프를 나가도 worker 수명을 먼저 끝낸다.
            struct CALLBACK_JOIN final
            {
                CCpuJobPool& pool;
                bool joined = false;
                void Wait() noexcept
                {
                    if (joined)
                    {
                        return;
                    }

                    WaitForThreadpoolWorkCallbacks(pool.m_pWork, FALSE);
                    pool.m_pActiveBatch.store(nullptr, std::memory_order_release);
                    joined = true;
                }
                ~CALLBACK_JOIN()
                {
                    Wait();
                }
            } join{*this};
            for (std::uint32_t i = 0u; i < assistants; ++i)
            {
                SubmitThreadpoolWork(m_pWork);
            }
            // 호출 스레드도 남은 인덱스를 처리한 다음 worker의 종료를 기다린다.
            batch.Drain(false);
            {
                Engine::CProfilerScope wait(batch.profiler, batch.joinScope);
                join.Wait();
            }
            if (batch.error)
            {
                std::rethrow_exception(batch.error);
            }

            return {batch.callerJobs, batch.workerJobs.load(std::memory_order_relaxed), assistants};
        }

    private:
        // Windows callback의 context는 pool이다. 게시된 batch를 차용하며 C++ 예외를 OS callback 밖으로 내보내지 않는다.
        static void CALLBACK RunWorker(PTP_CALLBACK_INSTANCE, void* context, PTP_WORK) noexcept
        {
            auto& pool = *static_cast<CCpuJobPool*>(context);
            CPU_JOB_BATCH* batch = pool.m_pActiveBatch.load(std::memory_order_acquire);
            if (!batch)
            {
                return;
            }

            try
            {
                Engine::CProfilerScope profile(batch->profiler, batch->workerScope);
                batch->Drain(true);
            }
            catch (...)
            {
                batch->RecordException();
            }
        }

        // pool의 유일한 활성 batch를 보호한다. worker는 이 mutex를 잡지 않아 호출 스레드의 join과 교착하지 않는다.
        std::mutex m_SubmissionMutex;
        std::atomic<CPU_JOB_BATCH*> m_pActiveBatch{nullptr};
        // pool이 직접 소유하는 OS 자원이다. m_pWork가 없으면 Run은 직렬로 동작한다.
        PTP_POOL m_pPool = nullptr;
        TP_CALLBACK_ENVIRON m_Environment{};
        PTP_WORK m_pWork = nullptr;
        std::uint32_t m_iMaximumAssistants = 0u;
    };
}

NS_BEGIN(Engine)

// 입력과 중첩 여부로 직렬 경로를 먼저 결정한다. 정상 병렬 호출만 process 수명의 단일 pool에 제출한다.
CPU_JOB_STATS Run_CpuJobs(const std::size_t count, void* context,
    void (*execute)(void*, std::size_t), const std::uint32_t maxAssistants,
    CProfiler* profiler, const char* workerScope, const char* joinScope)
{
    if (!count)
    {
        return {};
    }

    if (!execute)
    {
        throw std::invalid_argument("CPU job callback is missing.");
    }

    if (count == 1u || maxAssistants == 0u || g_CpuJobDepth != 0u)
    {
        return RunSerial(count, context, execute);
    }

    // 이 함수의 정적 pool은 프로세스 종료까지 유지하며 OS worker 준비 실패는 직렬 경로로 처리한다.
    static CCpuJobPool pool;
    CPU_JOB_BATCH batch{count, context, execute, profiler,
        workerScope ? workerScope : "CPU.Jobs.Worker", joinScope ? joinScope : "CPU.Jobs.Join"};
    return pool.Run(batch, maxAssistants);
}

NS_END
