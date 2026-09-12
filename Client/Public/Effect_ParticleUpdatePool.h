#pragma once

#include <cstddef>

namespace Engine { class CProfiler; }

namespace Client
{
// Synchronously updates independent emitter states. The caller keeps context,
// indexed work items and profiler alive until return; execute must not reenter
// this executor. Exceptions are rethrown only after every callback has finished.
void Run_EffectParticleUpdates(
	std::size_t count,
	void* context,
	void (*execute)(void*, std::size_t),
	Engine::CProfiler* profiler);
}
