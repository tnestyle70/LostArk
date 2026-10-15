// EffectPlayback이 사용하는 동기식 particle 작업 실행 경계를 구현한다.
#include "Effect_ParticleUpdatePool.h"
// 호출 스레드와 보조 worker가 함께 사용하는 공용 CPU 실행자에 작업을 위임한다.
#include "CpuJobPool.h"
// 현재 세션의 particle worker 사용 설정을 읽는다.
#include "GameInstance.h"
// RenderOptimizationSettings의 ParticleWorkersEnabled 설정 필드 계약을 사용한다.
#include "Engine_RenderTypes.h"
// 호출 스레드/worker의 처리 작업 수와 제출한 보조 callback 수를 기록한다.
#include "Profiler.h"

// EffectPlayback의 병렬 갱신 조건을 통과한 작업을 받는다. 옵션은 읽기만 하며 보조 실행자를 최대 한 명 요청한다.
// Run_CpuJobs가 모든 callback을 마쳐야 후속 emitter 간 위치 결합을 진행할 수 있다. 예외는 이 함수가 잡지 않는다.
void Client::Run_EffectParticleUpdates(
    const std::size_t count,
    void* context,
    void (*execute)(void*, std::size_t),
    Engine::CProfiler* profiler)
{
    // particle 경로는 보조 실행자 한 명 정책을 사용한다. 실제 제출 수는 공용 pool의 CPU/작업 수 제한을 함께 따른다.
    const uint32_t assistants =
        Engine::CGameInstance::Get().Get_RenderOptimizationSettings().ParticleWorkersEnabled ? 1u : 0u;
    const auto stats = Engine::Run_CpuJobs(count, context, execute, assistants, profiler,
        "Effect.Particle.Update.Worker", "Effect.Particle.Update.Join");
    // 정상 반환한 통계만 합산한다. callback 예외가 전달되면 이 계측 구간에는 도달하지 않는다.
    if (profiler)
    {
        profiler->Add_Counter(Engine::EProfilerCounter::EffectParticleCallerJobs, stats.CallerJobs);
        profiler->Add_Counter(Engine::EProfilerCounter::EffectParticleWorkerJobs, stats.WorkerJobs);
        profiler->Add_Counter(Engine::EProfilerCounter::EffectParticleAssistants, stats.Assistants);
    }
}
