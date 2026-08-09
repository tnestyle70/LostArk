#pragma once

#include "SpawnGroupBootstrap.h"

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace LostArk::Server
{
	class CSpawnGroupRuntime final
	{
	public:
		using ACTIVE_COUNT_QUERY = std::function<std::uint32_t(const std::string&)>;
		using SPAWN_CALLBACK = std::function<bool(const std::string&,
			const SPAWN_GROUP_ENTRY&, const SPAWN_GROUP_ANCHOR&,
			const MONSTER_RUNTIME_PROFILE&, std::uint32_t)>;

		bool Initialize(const CSpawnGroupBootstrap& bootstrap, std::string& outStatus);
		bool Activate(const std::string& spawnGroupId);
		bool Activate_Immediate(const std::string& spawnGroupId,
			const CSpawnGroupBootstrap& bootstrap,
			const SPAWN_CALLBACK& spawn);
		void Update(float fixedDeltaSeconds, const CSpawnGroupBootstrap& bootstrap,
			const ACTIVE_COUNT_QUERY& activeCount, const SPAWN_CALLBACK& spawn);
		bool Is_Completed(const std::string& spawnGroupId) const;
		bool Is_ActiveOrCompleted(const std::string& spawnGroupId) const;

	private:
		enum class GROUP_STATE { DORMANT, RUNNING, COMPLETED };

		struct RUNTIME_GROUP final
		{
			const SPAWN_GROUP_DEFINITION* pDefinition = nullptr;
			GROUP_STATE eState = GROUP_STATE::DORMANT;
			std::size_t iWaveIndex = 0;
			std::uint64_t iElapsedMs = 0;
			std::vector<std::uint32_t> SpawnedByEntry;
		};

		RUNTIME_GROUP* Find(const std::string& spawnGroupId);
		const RUNTIME_GROUP* Find(const std::string& spawnGroupId) const;
		static void Begin_Wave(RUNTIME_GROUP& group);

	private:
		std::vector<RUNTIME_GROUP> m_Groups;
	};
}
