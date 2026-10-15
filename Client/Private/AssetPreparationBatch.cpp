// 독립 준비 작업의 실행·결과·취소 계약을 구현한다.
#include "AssetPreparationBatch.h"

// worker 수 상한과 공용 허가 대기열의 검색·제거에 사용한다.
#include <algorithm>
// 외부 atomic 취소를 다시 확인할 10ms 조건 대기 간격을 정한다.
#include <chrono>
// 공용 실행 허가의 반환을 기다리는 producer를 깨운다.
#include <condition_variable>
// 최상위 실행 허가 요청을 FIFO 순서로 보관한다.
#include <deque>
// worker 예외를 HRESULT로 바꾸고 수명 보장 실패 시 terminate를 사용한다.
#include <exception>
// 할당 실패 예외인 std::bad_alloc의 직접 선언이다.
#include <new>
// 새 자식 worker의 COM MTA 초기화·해제를 수행한다.
#include <objbase.h>
// CRT 상태가 준비된 자식 worker를 _beginthreadex로 시작한다.
#include <process.h>

NS_BEGIN(Client)

// 공용 예산과 허가 구현은 이 CPP에서만 사용한다.
namespace
{
	// process 안의 모든 batch가 공유한다. waiting은 stack의 허가 객체 주소를 식별값으로만 보관한다.
	// active는 최상위 허가 수, children은 예약된 자식 몫이며 같은 mutex로 갱신한다.
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
	// 현재 스레드의 중첩 깊이와 가장 바깥 허가의 독점 여부다. 자식 스레드에 상속되는 값은 아니다.
	thread_local uint32_t g_PreparationDepth = 0u;
	thread_local bool g_PreparationExclusive = false;

	// 논리 CPU 두 개를 남기되 최소 1, 최대 4로 정한 process 공용 한도를 최초 호출에 계산한다.
	size_t PreparationLimit() noexcept
	{
		static const size_t limit = []
		{
			const DWORD processors = GetActiveProcessorCount(ALL_PROCESSOR_GROUPS);
			return (std::min)(size_t{ 4u }, processors > 2u ? size_t{ processors - 2u } : size_t{ 1u });
		}();
		return limit;
	}

	// callback을 실행하는 범위에서만 허가를 소유한다. 대기 중인 producer는 활성 허가를 차지하지 않는다.
	// Effect producer의 main-thread commit 응답 대기는 Run이 끝나 허가가 해제된 뒤 호출자에서 수행한다.
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
			{
				return;
			}

			// 부모 callback이 가진 허가의 수명 안에서만 중첩되므로 전역 active를 다시 늘리지 않는다.
			if (g_PreparationDepth != 0u)
			{
				++g_PreparationDepth;
				m_bEntered = true;
				return;
			}

			auto& budget = g_PreparationBudget;
			std::unique_lock lock(budget.mutex);
			// FIFO 앞의 요청만 입장한다. 독점 요청은 active가 0일 때, 일반 요청은 한도 미만일 때 허용한다.
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
						m_bEntered = true;
						m_bOwnsPermit = true;
						budget.changed.notify_all();
						return;
					}
					// 허가 해제 알림뿐 아니라 외부 atomic 취소도 확인하기 위해 10ms마다 조건을 다시 읽는다.
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

		// 중첩은 깊이만 줄이고, 실제 허가 소유자만 active와 독점 상태를 해제해 다음 대기자를 깨운다.
		~CPreparationPermit()
		{
			if (!m_bEntered)
			{
				return;
			}
			--g_PreparationDepth;
			if (!m_bOwnsPermit)
			{
				return;
			}
			g_PreparationExclusive = false;
			auto& budget = g_PreparationBudget;
			std::scoped_lock lock(budget.mutex);
			--budget.active;
			budget.exclusive = false;
			budget.changed.notify_all();
		}

		explicit operator bool() const noexcept
		{
			return m_bEntered;
		}
		CPreparationPermit(const CPreparationPermit&) = delete;
		CPreparationPermit& operator=(const CPreparationPermit&) = delete;

	private:
		bool m_bEntered = false;
		bool m_bOwnsPermit = false;
	};

	// 동시에 여러 batch가 시작해도 전체 자식 수가 한도에서 호출자 몫 1개를 뺀 값을 넘지 않게 예약한다.
	bool ReserveChild()
	{
		std::scoped_lock lock(g_PreparationBudget.mutex);
		if (g_PreparationBudget.children >= PreparationLimit() - 1u)
		{
			return false;
		}
		++g_PreparationBudget.children;
		return true;
	}

	// 스레드 생성 실패 또는 ThreadMain의 종료에서 예약한 자식 몫을 돌려준다.
	void ReleaseChild() noexcept
	{
		std::scoped_lock lock(g_PreparationBudget.mutex);
		--g_PreparationBudget.children;
	}
}

// Run의 stack에 놓이고 모든 참여 스레드가 차용한다. next는 작업 배분용 atomic이며 출력 객체를 소유하지 않는다.
struct CAssetPreparationBatch::RUN_STATE final
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
		// 할당이나 스레드 시작 관리 중 예외로 빠져나가도 자식부터 join한다.
		// 이에 따라 Run이 참조하는 callback과 호출자 출력의 수명 안에서 자식 접근을 끝낸다.
		owner.Join_Workers();
	}

	// 호출자가 소유한 취소 표식을 읽는다. 이미 실행 중인 callback을 강제로 끝내지는 않는다.
	bool Cancelled() const noexcept
	{
		return cancellation && cancellation->load(std::memory_order_acquire);
	}

	// S_OK를 먼저 바꾼 실패 하나만 기록한다. 가장 작은 task index를 고르는 정책은 아니며 결과는 join 뒤 읽는다.
	void Fail(const HRESULT failure, const size_t index) noexcept
	{
		// result와 failedTask를 한 번에 공개하는 연산은 아니다. 외부 결과는 모든 join 뒤 읽는다.
		HRESULT expected = S_OK;
		if (result.compare_exchange_strong(expected, failure, std::memory_order_acq_rel))
		{
			failedTask.store(index, std::memory_order_release);
		}
	}

	// 작업 번호를 한 번씩 확보하고 허가를 받은 뒤 callback을 실행한다. 실패/취소 이후 새 작업 시작을 줄인다.
	// 이미 실행 중인 callback이나 완료된 출력은 되돌리지 않는다. 정리와 commit 여부는 호출자 책임이다.
	void Run() noexcept
	{
		size_t index = (std::numeric_limits<size_t>::max)();
		try
		{
			while (SUCCEEDED(result.load(std::memory_order_acquire)) && !Cancelled())
			{
				// 작업 번호만 배분한다. 출력 공개는 callback의 동기화와 join이 담당한다.
				index = next.fetch_add(1u, std::memory_order_relaxed);
				if (index >= taskCount)
				{
					return;
				}

				CPreparationPermit permit(exclusive, cancellation, result);
				if (!permit)
				{
					return;
				}

				const HRESULT prepared = prepare(index);
				if (FAILED(prepared))
				{
					Fail(prepared, index);
					return;
				}
			}
		}
		catch (const std::bad_alloc&)
		{
			Fail(E_OUTOFMEMORY, index);
		}
		catch (...)
		{
			Fail(E_FAIL, index);
		}
	}

	// 생성한 자식만 MTA COM을 초기화하고 해제한다. Run 호출자 스레드의 COM 준비는 여기서 대신하지 않는다.
	static unsigned __stdcall ThreadMain(void* parameter)
	{
		struct CHILD_COMPLETION final
		{
			~CHILD_COMPLETION()
			{
				ReleaseChild();
			}
		} completion;
		auto& state = *static_cast<RUN_STATE*>(parameter);
		const HRESULT apartment = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
		if (SUCCEEDED(apartment))
		{
			state.Run();
			CoUninitialize();
		}
		else
		{
			state.Fail(apartment, (std::numeric_limits<size_t>::max)());
		}
		return FAILED(state.result.load(std::memory_order_acquire)) ? 1u : 0u;
	}
};

// 동일 batch의 동시 Run은 ERROR_BUSY다. 서로 다른 batch는 전역 예산을 공유하며 자기 작업만 join한다.
// 명시적 실패는 HRESULT로 모으지만 이미 callback이 만든 결과의 rollback까지 수행하지 않는다.
CAssetPreparationBatch::RESULT CAssetPreparationBatch::Run(
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
		if (m_bRunning)
		{
			output.result = HRESULT_FROM_WIN32(ERROR_BUSY);
			return output;
		}
		m_bRunning = true;
	}

	struct RUN_COMPLETION final
	{
		CAssetPreparationBatch& owner;
		~RUN_COMPLETION()
		{
			std::scoped_lock lock(owner.m_WorkerMutex);
			owner.m_bRunning = false;
		}
	} completion{ *this };

	const bool exclusive = (device->GetCreationFlags() & D3D11_CREATE_DEVICE_SINGLETHREADED) != 0u;
	// 일반 부모의 허가를 독점으로 바꾸면 다른 실행과 겹치거나 자기 부모를 기다릴 수 있다.
	// 따라서 독점 허가가 아닌 부모 안에서 SINGLETHREADED device batch를 시작하면 거부한다.
	if (exclusive && g_PreparationDepth != 0u && !g_PreparationExclusive)
	{
		output.result = E_INVALIDARG;
		return output;
	}

	// 이 참조 묶음보다 먼저 자식이 끝나도록 정상 경로와 소멸 경로 양쪽에서 join한다.
	RUN_STATE state{ *this, taskCount, cancellation, prepare, exclusive };
	try
	{
		if (taskCount != 0u && !state.Cancelled())
		{
			// 중첩 batch는 현재 스레드의 부모 허가를 빌리고 자식 스레드를 만들지 않는다.
			// 부모가 가진 허가를 기다리지 않도록 중첩 작업은 현재 스레드에서 순차 처리한다.
			const size_t limit = exclusive || g_PreparationDepth != 0u ?
				1u : (std::min)(PreparationLimit(), taskCount);
			output.workerCount = 1u;
			{
				std::scoped_lock lock(m_WorkerMutex);
				// HANDLE 저장 공간을 먼저 확보해 생성한 자식의 소유 기록을 잃지 않게 한다.
				m_Workers.reserve(limit - 1u);
				for (size_t index = 1u; index < limit; ++index)
				{
					if (state.Cancelled() || !ReserveChild())
					{
						break;
					}
					const HANDLE worker = reinterpret_cast<HANDLE>(_beginthreadex(
						nullptr, 0u, &RUN_STATE::ThreadMain, &state, 0u, nullptr));
					if (!worker)
					{
						ReleaseChild();
						break; // 호출자와 이미 생성한 자식이 남은 작업을 처리한다.
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
	// 모든 출력 접근이 끝난 뒤 첫 실패와 취소 결과를 호출자에게 돌려준다.
	Join_Workers();
	output.result = state.result.load(std::memory_order_acquire);
	output.failedTask = state.failedTask.load(std::memory_order_acquire);
	if (SUCCEEDED(output.result) && state.Cancelled())
	{
		output.result = HRESULT_FROM_WIN32(ERROR_CANCELLED);
	}
	return output;
}

// Run 소유 스레드가 자식 종료를 기다린 뒤 HANDLE을 닫는다. wait 자체 실패는 수명을 보장할 수 없어 process를 종료한다.
void CAssetPreparationBatch::Join_Workers() noexcept
{
	for (;;)
	{
		HANDLE worker = nullptr;
		{
			std::scoped_lock lock(m_WorkerMutex);
			if (m_Workers.empty())
			{
				return;
			}
			worker = m_Workers.back();
		}
		// 이 join 자체에는 시간 제한이 없다. Loader 등 호출자가 별도의 종료 제한 시간을 관리한다.
		// 호출자는 여기서 대기 중인 batch의 자식 동기 I/O에 취소를 요청할 수 있다.
		if (WAIT_OBJECT_0 != WaitForSingleObject(worker, INFINITE))
		{
			const DWORD error = GetLastError();
			OutputDebugStringA("[AssetPreparation] Worker join failed; terminating the process.\n");
			if (!TerminateProcess(GetCurrentProcess(),
				ERROR_SUCCESS == error ? ERROR_INVALID_HANDLE : error))
			{
				std::terminate();
			}
			__assume(0);
		}
		{
			// 취소 요청과 HANDLE 닫기를 같은 lock으로 보호한다.
			// 닫혀 재사용된 HANDLE에 다른 스레드의 취소 요청이 전달되지 않게 한다.
			std::scoped_lock lock(m_WorkerMutex);
			CloseHandle(worker);
			m_Workers.pop_back();
		}
	}
}

// Loader와 비동기 준비 서비스의 종료 요청에서 호출한다. 성공 여부를 모으거나 취소 완료를 기다리는 함수는 아니다.
void CAssetPreparationBatch::Cancel_SynchronousIo() noexcept
{
	std::scoped_lock lock(m_WorkerMutex);
	for (const HANDLE worker : m_Workers)
	{
		CancelSynchronousIo(worker);
	}
}

NS_END
