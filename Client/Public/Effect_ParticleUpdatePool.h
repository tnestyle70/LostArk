#pragma once

// 작업 개수와 callback의 독립 작업 index를 size_t로 전달한다.
#include <cstddef>

// 실행 시간을 기록할 측정기를 호출 동안 차용하므로 전방 선언만 사용한다.
namespace Engine
{
class CProfiler;
}

namespace Client
{
// EffectPlayback이 선택한 독립 emitter 상태를 동기식으로 갱신한다. 호출자는 context·작업 목록·측정기를 반환까지 유지한다.
// 같은 pool 참여 스레드의 중첩 호출은 직렬 실행한다. callback 실패는 실행 중인 작업이 끝난 뒤 다시 전달한다.
// 완료한 particle 쓰기는 되돌리지 않는다. index별 상태를 독립시킬 책임은 작업을 구성하는 호출자에게 있다.
void Run_EffectParticleUpdates(
	std::size_t count,
	void* context,
	void (*execute)(void*, std::size_t),
	Engine::CProfiler* profiler);
}
