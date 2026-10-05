#pragma once

#include "Engine_Macro.h"
#include <cstddef>
#include <cstdint>

namespace Engine
{
class CProfiler;

struct CPU_JOB_STATS final
{
    // Successfully returned callbacks; their sum equals count on success.
    std::size_t CallerJobs = 0u;
    std::size_t WorkerJobs = 0u;
    // Submitted assisting callbacks, including one that found no remaining job.
    std::uint32_t Assistants = 0u;
};

// Synchronous, bounded CPU fork/join. Each index owns independent mutable data.
// Inputs, profiler and scope names remain alive until return. Callbacks must not
// access the D3D immediate context or wait for another thread to call this pool.
// Nested calls on participating lanes run serially. External callers serialize.
// On an exception, already-running callbacks finish before the first exception
// is rethrown; completed writes are not rolled back. A caller owns any staging.
ENGINE_DLL CPU_JOB_STATS Run_CpuJobs(std::size_t count, void* context,
    void (*execute)(void*, std::size_t), std::uint32_t maxAssistants,
    CProfiler* profiler, const char* workerScope, const char* joinScope);
}
