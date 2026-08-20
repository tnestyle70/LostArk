#pragma once

#include <cstdint>
#include <string_view>

namespace Client
{
	class IWorldEntityCommandSink
	{
	public:
		virtual ~IWorldEntityCommandSink() = default;
		virtual bool Request_SpawnWorldEntity(
			std::string_view placementId) = 0;
		virtual bool Request_DespawnAllWorldEntities(
			std::uint32_t requestSequence) = 0;
	};
}
