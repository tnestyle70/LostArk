#pragma once

#pragma push_macro("new")
#undef new
#include <d3d11.h>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <mutex>
#include <vector>
#pragma pop_macro("new")

namespace Client
{
	class CAssetPreparationBatch final
	{
	public:
		struct RESULT final
		{
			HRESULT result = S_OK;
			size_t failedTask = (std::numeric_limits<size_t>::max)();
			uint32_t workerCount = 0u;
		};

		// Synchronous: every child is joined before returning. The callback may
		// run concurrently and must own a distinct output slot for each task.
		// The caller keeps this batch and cancellation flag alive until Run exits.
		RESULT Run(ID3D11Device* device, size_t taskCount,
			const std::atomic_bool* cancellation,
			const std::function<HRESULT(size_t)>& prepare);
		void Cancel_SynchronousIo() noexcept;

	private:
		struct RUN_STATE;
		void Join_Workers() noexcept;
		std::mutex m_WorkerMutex;
		std::vector<HANDLE> m_Workers;
		bool m_Running = false;
	};
}
