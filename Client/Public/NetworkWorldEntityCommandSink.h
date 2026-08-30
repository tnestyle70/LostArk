#pragma once

#include "WorldEntityCommandSink.h"

namespace Client
{
	class CNetworkWorldEntityCommandSink final :
		public IWorldEntityCommandSink
	{
	public:
		bool Request_SpawnWorldEntity(
			std::string_view placementId) override;
		bool Request_DespawnAllWorldEntities(
			std::uint32_t requestSequence) override;
		bool Request_EnterKakulSaydonArena(
			std::uint32_t requestSequence) override;
		bool Request_StageTeleport(
			std::uint32_t requestSequence,
			std::string_view placementId) override;
	};
}
