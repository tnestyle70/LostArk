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
	};
}
