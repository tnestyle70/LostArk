#pragma once

// DLL 밖의 Client에서도 Run_CpuJobs를 사용할 수 있도록 ENGINE_DLL 내보내기를 선언한다.
#include "Engine_Macro.h"
// 작업 개수와 인덱스에는 size_t, 보조 실행자 수에는 고정 폭 정수를 사용한다.
#include <cstddef>
#include <cstdint>

NS_BEGIN(Engine)
// 실행 중 측정기를 차용할 뿐 헤더에서 내부에 접근하지 않으므로 전방 선언한다.
class CProfiler;

struct CPU_JOB_STATS final
{
    // 정상 반환한 작업 수다. 성공한 호출에서는 호출 스레드와 worker의 합이 count다.
    std::size_t CallerJobs = 0u;
    std::size_t WorkerJobs = 0u;
    // 제출한 보조 callback 수다. 실행 시 이미 남은 작업이 없어도 이 수에는 포함된다.
    std::uint32_t Assistants = 0u;
};

// Layer의 최종 카메라 CPU 준비와 Client의 particle 갱신에서 독립 인덱스 작업을 나눠 실행한다.
// 호출 스레드도 작업을 수행하고 제출한 worker가 모두 끝난 뒤 반환한다.
// context, profiler, scope 이름은 반환까지 호출자가 유지하며 각 인덱스는 독립된 변경 영역을 사용한다.
// callback에서 D3D immediate context를 사용하거나 다른 스레드의 이 pool 호출 완료를 기다리지 않는다.
// 중첩 호출은 현재 스레드에서 직렬 실행한다. pool 제출끼리는 mutex로 직렬화하지만,
// pool을 거치지 않는 직렬 경로는 다른 외부 호출과 동시에 실행될 수 있다.
// 병렬 경로는 진행 중인 callback을 모두 join한 뒤 첫 예외를 다시 던진다. 직렬 경로는 즉시 전파한다.
// 이미 완료한 쓰기는 되돌리지 않으므로 전체 반영이 필요하면 호출자가 임시 결과를 소유한다.
ENGINE_DLL CPU_JOB_STATS Run_CpuJobs(std::size_t count, void* context,
    void (*execute)(void*, std::size_t), std::uint32_t maxAssistants,
    CProfiler* profiler, const char* workerScope, const char* joinScope);
NS_END
