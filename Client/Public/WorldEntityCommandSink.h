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
		/* Debug authoring entry. The Server stages the normal world-transfer
		transaction and S2C_ENTER_ACCEPTED remains the only level authority. */
		virtual bool Request_EnterKakulSaydonArena(
			std::uint32_t requestSequence) = 0;
		/* Debug authoring navigation. The Server resolves the stable playerSpawn
		waypoint against its own Kakul navigation and snapshots the result. */
		virtual bool Request_StageTeleport(
			std::uint32_t requestSequence,
			std::string_view placementId) = 0;
	};
}
