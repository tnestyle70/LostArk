#include "AssetPreparationBatch.h"

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <exception>
#include <objbase.h>
#include <process.h>

namespace
{
	struct PREPARATION_BUDGET final
	{
		std::mutex mutex;
		std::condition_variable changed;
		std::deque<const void*> waiting;
		size_t active = 0u;
		size_t children = 0u;
		bool exclusive = false;
	};

	PREPARATION_BUDGET g_PreparationBudget;
	thread_local uint32_t g_PreparationDepth = 0u;
	thread_local bool g_PreparationExclusive = false;

	size_t PreparationLimit() noexcept
	{
		static const size_t limit = []
		{
			const DWORD processors = GetActiveProcessorCount(ALL_PROCESSOR_GROUPS);
			return (std::min)(size_t{ 4u }, processors > 2u ? size_t{ processors - 2u } : size_t{ 1u });
		}();
		return limit;
	}

	// Only executing preparation callbacks hold a permit. In particular an
	// Effect producer releases it before waiting for the main-thread commit ACK.
	class CPreparationPermit final
	{
	public:
		CPreparationPermit(const bool exclusive, const std::atomic_bool* cancellation,
			const std::atomic<HRESULT>& result)
		{
			const auto stopped = [&]
			{
				return FAILED(result.load(std::memory_order_acquire)) ||
					(cancellation && cancellation->load(std::memory_order_acquire));
			};
			if (stopped())
				return;
			if (g_PreparationDepth != 0u)
			{
				++g_PreparationDepth;
				m_Entered = true;
				return;
			}
			auto& budget = g_PreparationBudget;
			std::unique_lock lock(budget.mutex);
			budget.waiting.push_back(this);
			try
			{
				for (;;)
				{
					if (stopped())
					{
						budget.waiting.erase(std::find(budget.waiting.begin(), budget.waiting.end(), this));
						budget.changed.notify_all();
						return;
					}
					if (budget.waiting.front() == this && !budget.exclusive &&
						(exclusive ? budget.active == 0u : budget.active < PreparationLimit()))
					{
						budget.waiting.pop_front();
						++budget.active;
						budget.exclusive = exclusive;
						g_PreparationDepth = 1u;
						g_PreparationExclusive = exclusive;
						m_Entered = true;
						m_OwnsPermit = true;
						budget.changed.notify_all();
						return;
					}
					budget.changed.wait_for(lock, std::chrono::milliseconds(10));
				}
			}
			catch (...)
			{
				budget.waiting.erase(std::find(budget.waiting.begin(), budget.waiting.end(), this));
				budget.changed.notify_all();
				throw;
			}
		}

		~CPreparationPermit()
		{
			if (!m_Entered)
				return;
			--g_PreparationDepth;
			if (!m_OwnsPermit)
				return;
			g_PreparationExclusive = false;
			auto& budget = g_PreparationBudget;
			std::scoped_lock lock(budget.mutex);
			--budget.active;
			budget.exclusive = false;
			budget.changed.notify_all();
		}

		explicit operator bool() const noexcept { return m_Entered; }
		CPreparationPermit(const CPreparationPermit&) = delete;
		CPreparationPermit& operator=(const CPreparationPermit&) = delete;

	private:
		bool m_Entered = false;
		bool m_OwnsPermit = false;
	};

	bool ReserveChild()
	{
		std::scoped_lock lock(g_PreparationBudget.mutex);
		if (g_PreparationBudget.children >= PreparationLimit() - 1u)
			return false;
		++g_PreparationBudget.children;
		return true;
	}

	void ReleaseChild() noexcept
	{
		std::scoped_lock lock(g_PreparationBudget.mutex);
		--g_PreparationBudget.children;
	}
}

struct Client::CAssetPreparationBatch::RUN_STATE final
{
	CAssetPreparationBatch& owner;
	const size_t taskCount;
	const std::atomic_bool* cancellation;
	const std::function<HRESULT(size_t)>& prepare;
	const bool exclusive;
	std::atomic_size_t next{ 0u };
	std::atomic<HRESULT> result{ S_OK };
	std::atomic_size_t failedTask{ (std::numeric_limits<size_t>::max)() };

	~RUN_STATE()
	{
		// In particular, keep prepare and every caller-owned output alive while
		// unwinding after allocation or thread-start bookkeeping failures.
		owner.Join_Workers();
	}

	bool Cancelled() const noexcept
	{
		return cancellation && cancellation->load(std::memory_order_acquire);
	}

	void Fail(const HRESULT failure, const size_t index) noexcept
	{
		HRESULT expected = S_OK;
		if (result.compare_exchange_strong(expected, failure, std::memory_order_acq_rel))
			failedTask.store(index, std::memory_order_release);
	}

	void Run() noexcept
	{
		size_t index = (std::numeric_limits<size_t>::max)();
		try
		{
			while (SUCCEEDED(result.load(std::memory_order_acquire)) && !Cancelled())
			{
				index = next.fetch_add(1u, std::memory_order_relaxed);
				if (index >= taskCount)
					return;
				CPreparationPermit permit(exclusive, cancellation, result);
				if (!permit)
					return;
				const HRESULT prepared = prepare(index);
				if (FAILED(prepared))
				{
					Fail(prepared, index);
					return;
				}
			}
		}
		catch (const std::bad_alloc&) { Fail(E_OUTOFMEMORY, index); }
		catch (...) { Fail(E_FAIL, index); }
	}

	static unsigned __stdcall ThreadMain(void* parameter)
	{
		struct CHILD_COMPLETION final
		{
			~CHILD_COMPLETION() { ReleaseChild(); }
		} completion;
		auto& state = *static_cast<RUN_STATE*>(parameter);
		const HRESULT apartment = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
		if (SUCCEEDED(apartment))
		{
			state.Run();
			CoUninitialize();
		}
		else
			state.Fail(apartment, (std::numeric_limits<size_t>::max)());
		return FAILED(state.result.load(std::memory_order_acquire)) ? 1u : 0u;
	}
};

Client::CAssetPreparationBatch::RESULT Client::CAssetPreparationBatch::Run(
	ID3D11Device* device, const size_t taskCount,
	const std::atomic_bool* cancellation, const std::function<HRESULT(size_t)>& prepare)
{
	RESULT output;
	if (!device || !prepare)
	{
		output.result = E_INVALIDARG;
		return output;
	}
	{
		std::scoped_lock lock(m_WorkerMutex);
		if (m_Running)
		{
			output.result = HRESULT_FROM_WIN32(ERROR_BUSY);
			return output;
		}
		m_Running = true;
	}
	struct RUN_COMPLETION final
	{
		CAssetPreparationBatch& owner;
		~RUN_COMPLETION()
		{
			std::scoped_lock lock(owner.m_WorkerMutex);
			owner.m_Running = false;
		}
	} completion{ *this };
	const bool exclusive = (device->GetCreationFlags() & D3D11_CREATE_DEVICE_SINGLETHREADED) != 0u;
	// Upgrading a normal parent's permit would either overlap another caller
	// or deadlock waiting for that parent. Reject this incompatible nesting.
	if (exclusive && g_PreparationDepth != 0u && !g_PreparationExclusive)
	{
		output.result = E_INVALIDARG;
		return output;
	}
	RUN_STATE state{ *this, taskCount, cancellation, prepare, exclusive };
	try
	{
		if (taskCount != 0u && !state.Cancelled())
		{
			// A nested batch borrows this thread's callback permit and executes
			// serially, so nesting cannot wait on permits held by its own parents.
			const size_t limit = exclusive || g_PreparationDepth != 0u ?
				1u : (std::min)(PreparationLimit(), taskCount);
			output.workerCount = 1u;
			{
				std::scoped_lock lock(m_WorkerMutex);
				m_Workers.reserve(limit - 1u);
				for (size_t index = 1u; index < limit; ++index)
				{
					if (state.Cancelled() || !ReserveChild())
						break;
					const HANDLE worker = reinterpret_cast<HANDLE>(_beginthreadex(
						nullptr, 0u, &RUN_STATE::ThreadMain, &state, 0u, nullptr));
					if (!worker)
					{
						ReleaseChild();
						break; // The owner and existing children drain the remaining queue.
					}
					m_Workers.push_back(worker);
					++output.workerCount;
				}
			}
			state.Run();
		}
	}
	catch (const std::bad_alloc&)
	{
		state.Fail(E_OUTOFMEMORY, (std::numeric_limits<size_t>::max)());
	}
	catch (...)
	{
		state.Fail(E_FAIL, (std::numeric_limits<size_t>::max)());
	}
	Join_Workers();
	output.result = state.result.load(std::memory_order_acquire);
	output.failedTask = state.failedTask.load(std::memory_order_acquire);
	if (SUCCEEDED(output.result) && state.Cancelled())
		output.result = HRESULT_FROM_WIN32(ERROR_CANCELLED);
	return output;
}

void Client::CAssetPreparationBatch::Join_Workers() noexcept
{
	for (;;)
	{
		HANDLE worker = nullptr;
		{
			std::scoped_lock lock(m_WorkerMutex);
			if (m_Workers.empty())
				return;
			worker = m_Workers.back();
		}
		// The caller owns its existing bounded shutdown deadline and can cancel
		// the tracked children's synchronous I/O while the owner waits here.
		if (WAIT_OBJECT_0 != WaitForSingleObject(worker, INFINITE))
		{
			const DWORD error = GetLastError();
			OutputDebugStringA("[AssetPreparation] Worker join failed; terminating the process.\n");
			if (!TerminateProcess(GetCurrentProcess(),
				ERROR_SUCCESS == error ? ERROR_INVALID_HANDLE : error))
				std::terminate();
			__assume(0);
		}
		{
			// Cancellation and close use the same lock; no stale/reused handle
			// can escape to a concurrent shutdown request.
			std::scoped_lock lock(m_WorkerMutex);
			CloseHandle(worker);
			m_Workers.pop_back();
		}
	}
}

void Client::CAssetPreparationBatch::Cancel_SynchronousIo() noexcept
{
	std::scoped_lock lock(m_WorkerMutex);
	for (const HANDLE worker : m_Workers)
		CancelSynchronousIo(worker);
}
