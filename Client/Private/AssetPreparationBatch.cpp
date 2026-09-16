#include "AssetPreparationBatch.h"

#include <algorithm>
#include <exception>
#include <objbase.h>
#include <process.h>

struct Client::CAssetPreparationBatch::RUN_STATE final
{
	CAssetPreparationBatch& owner;
	const size_t taskCount;
	const std::atomic_bool* cancellation;
	const std::function<HRESULT(size_t)>& prepare;
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
	RUN_STATE state{ *this, taskCount, cancellation, prepare };
	try
	{
		if (taskCount != 0u && !state.Cancelled())
		{
			const DWORD processors = GetActiveProcessorCount(ALL_PROCESSOR_GROUPS);
			const size_t available = processors > 2u ? processors - 2u : 1u;
			const size_t limit = (device->GetCreationFlags() & D3D11_CREATE_DEVICE_SINGLETHREADED) ?
				1u : (std::min)({ size_t{ 4u }, available, taskCount });
			output.workerCount = 1u;
			{
				std::scoped_lock lock(m_WorkerMutex);
				m_Workers.reserve(limit - 1u);
				for (size_t index = 1u; index < limit; ++index)
				{
					if (state.Cancelled())
						break;
					const HANDLE worker = reinterpret_cast<HANDLE>(_beginthreadex(
						nullptr, 0u, &RUN_STATE::ThreadMain, &state, 0u, nullptr));
					if (!worker)
						break; // The owner and existing children drain the remaining queue.
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
