#include "Effect_ParticleUpdatePool.h"
#include "Profiler.h"

#include <Windows.h>
#include <algorithm>
#include <atomic>
#include <exception>
#include <mutex>
#include <stdexcept>

namespace
{
	struct PARTICLE_UPDATE_BATCH final
	{
		std::size_t count;
		void* context;
		void (*execute)(void*, std::size_t);
		Engine::CProfiler* profiler;
		std::atomic_size_t next{ 0u };
		std::atomic_bool failed{ false };
		std::exception_ptr error;

		void Record_Exception() noexcept
		{
			// Only the first failing lane writes error. The caller reads it after
			// all callbacks have joined, including the lane that claimed failure.
			if (!failed.exchange(true, std::memory_order_acq_rel))
				error = std::current_exception();
		}

		void Drain() noexcept
		{
			try
			{
				while (!failed.load(std::memory_order_acquire))
				{
					const std::size_t index = next.fetch_add(1u, std::memory_order_relaxed);
					if (index >= count)
						return;
					execute(context, index);
				}
			}
			catch (...)
			{
				Record_Exception();
			}
		}
	};

	class PARTICLE_UPDATE_POOL final
	{
	public:
		PARTICLE_UPDATE_POOL()
		{
			const unsigned processors = static_cast<unsigned>(
				GetActiveProcessorCount(ALL_PROCESSOR_GROUPS));
			// The Debug playback workload benefits from one assisting lane. More
			// workers contend on checked-library bookkeeping and regress this batch.
			m_WorkerCount = processors > 2u ? 1u : 0u;
			if (0u == m_WorkerCount)
				return;
			m_Pool = CreateThreadpool(nullptr);
			if (nullptr == m_Pool)
				return;
			SetThreadpoolThreadMaximum(m_Pool, m_WorkerCount);
			if (!SetThreadpoolThreadMinimum(m_Pool, m_WorkerCount))
			{
				CloseThreadpool(m_Pool);
				m_Pool = nullptr;
				return;
			}
			InitializeThreadpoolEnvironment(&m_Environment);
			SetThreadpoolCallbackPool(&m_Environment, m_Pool);
			m_Work = CreateThreadpoolWork(&Run_Worker, this, &m_Environment);
			if (nullptr == m_Work)
			{
				DestroyThreadpoolEnvironment(&m_Environment);
				CloseThreadpool(m_Pool);
				m_Pool = nullptr;
			}
		}

		~PARTICLE_UPDATE_POOL()
		{
			// Every public call is synchronous. At process teardown the lock also
			// excludes another batch while pending callbacks are cancelled/drained.
			std::lock_guard lock(m_SubmissionMutex);
			if (nullptr == m_Work)
				return;
			WaitForThreadpoolWorkCallbacks(m_Work, TRUE);
			CloseThreadpoolWork(m_Work);
			DestroyThreadpoolEnvironment(&m_Environment);
			CloseThreadpool(m_Pool);
		}

		void Run(PARTICLE_UPDATE_BATCH& batch)
		{
			std::unique_lock lock(m_SubmissionMutex);
			if (nullptr == m_Work)
			{
				for (std::size_t index = 0u; index < batch.count; ++index)
					batch.execute(batch.context, index);
				return;
			}
			m_ActiveBatch.store(&batch, std::memory_order_release);
			struct CALLBACK_JOIN final
			{
				PARTICLE_UPDATE_POOL& pool;
				bool joined = false;
				void Wait() noexcept
				{
					if (joined)
						return;
					WaitForThreadpoolWorkCallbacks(pool.m_Work, FALSE);
					pool.m_ActiveBatch.store(nullptr, std::memory_order_release);
					joined = true;
				}
				~CALLBACK_JOIN() { Wait(); }
			} join{ *this };
			const std::size_t callbacks = (std::min)(
				static_cast<std::size_t>(m_WorkerCount), batch.count - 1u);
			for (std::size_t index = 0u; index < callbacks; ++index)
				SubmitThreadpoolWork(m_Work);

			// The frame owner participates instead of waiting for worker startup.
			batch.Drain();
			{
				Engine::CProfilerScope wait(batch.profiler, "Effect.Particle.Update.Join");
				join.Wait();
			}
			if (batch.error)
				std::rethrow_exception(batch.error);
		}

	private:
		static void CALLBACK Run_Worker(PTP_CALLBACK_INSTANCE, void* context, PTP_WORK) noexcept
		{
			auto& pool = *static_cast<PARTICLE_UPDATE_POOL*>(context);
			PARTICLE_UPDATE_BATCH* const batch =
				pool.m_ActiveBatch.load(std::memory_order_acquire);
			if (nullptr == batch)
				return;
			try
			{
				Engine::CProfilerScope worker(batch->profiler, "Effect.Particle.Update.Worker");
				batch->Drain();
			}
			catch (...)
			{
				batch->Record_Exception();
			}
		}

		std::mutex m_SubmissionMutex;
		std::atomic<PARTICLE_UPDATE_BATCH*> m_ActiveBatch{ nullptr };
		PTP_POOL m_Pool = nullptr;
		TP_CALLBACK_ENVIRON m_Environment{};
		PTP_WORK m_Work = nullptr;
		DWORD m_WorkerCount = 0u;
	};
}

void Client::Run_EffectParticleUpdates(
	const std::size_t count,
	void* context,
	void (*execute)(void*, std::size_t),
	Engine::CProfiler* profiler)
{
	if (0u == count)
		return;
	if (nullptr == execute)
		throw std::invalid_argument("Effect particle update callback is missing.");
	if (1u == count)
	{
		execute(context, 0u);
		return;
	}
	static PARTICLE_UPDATE_POOL pool;
	PARTICLE_UPDATE_BATCH batch{ count, context, execute, profiler };
	pool.Run(batch);
}
