#pragma once

#include <string_view>

namespace Client
{
	class IWorldEntityCommandSink
	{
	public:
		virtual ~IWorldEntityCommandSink() = default;
		virtual bool Request_SpawnWorldEntity(
			std::string_view placementId) = 0;
	};
}
