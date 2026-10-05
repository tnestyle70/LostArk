#include "Effect_ParticleUpdatePool.h"
#include "CpuJobPool.h"
#include "GameInstance.h"
#include "Engine_RenderTypes.h"
#include "Profiler.h"

void Client::Run_EffectParticleUpdates(
    const std::size_t count,
    void* context,
    void (*execute)(void*, std::size_t),
    Engine::CProfiler* profiler)
{
    // Keep the measured one-helper particle policy; other consumers choose
    // their own coarse grain and assistant count on the shared CPU executor.
    const uint32_t assistants = Engine::CGameInstance::Get().Get_RenderOptimizationSettings().ParticleWorkersEnabled ? 1u : 0u;
    const auto stats = Engine::Run_CpuJobs(count, context, execute, assistants, profiler,
        "Effect.Particle.Update.Worker", "Effect.Particle.Update.Join");
    if (profiler)
    {
        profiler->Add_Counter(Engine::EProfilerCounter::EffectParticleCallerJobs, stats.CallerJobs);
        profiler->Add_Counter(Engine::EProfilerCounter::EffectParticleWorkerJobs, stats.WorkerJobs);
        profiler->Add_Counter(Engine::EProfilerCounter::EffectParticleAssistants, stats.Assistants);
    }
}
