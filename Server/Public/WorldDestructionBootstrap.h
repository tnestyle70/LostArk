#pragma once

#include "WorldDestructionRuntime.h"

#include <cstdint>
#include <filesystem>
#include <string>

namespace LostArk::Server
{
	struct WORLD_PICKUP_DESCRIPTOR final
	{
		std::string strPlacementId, strWallGroupId;
		float fStartX = 0.f, fStartY = 0.f, fStartZ = 0.f;
		float fLandingX = 0.f, fLandingY = 0.f, fLandingZ = 0.f;
		std::uint32_t iFallDurationTicks = 0u;
		float fPickupRadiusM = 0.f;
	};
	// Required Valtan World authoring output; parse/validate before replacing descriptors.
	bool Load_ValtanWorldPickups(std::vector<WORLD_PICKUP_DESCRIPTOR>& descriptors,
		std::string& status);
	bool Load_WorldPickupsFromFile(const std::filesystem::path& path,
		std::vector<WORLD_PICKUP_DESCRIPTOR>& descriptors, std::string& status);

	class CWorldDestructionBootstrap final
	{
	public:
		bool Load_ValtanArena();
		bool Load_FromFile(const std::filesystem::path& path);

		const WORLD_DESTRUCTION_DESCRIPTOR_GRAPH& Get_DescriptorGraph() const
		{
			return m_DescriptorGraph;
		}
		const std::string& Get_AreaId() const { return m_strAreaId; }
		const std::string& Get_EncounterId() const { return m_strEncounterId; }
		const std::string& Get_CombatRuntimeRevision() const
		{
			return m_strCombatRuntimeRevision;
		}
		const std::string& Get_Status() const { return m_strStatus; }
		std::uint32_t Get_FixedTickHz() const { return m_iFixedTickHz; }

	private:
		WORLD_DESTRUCTION_DESCRIPTOR_GRAPH m_DescriptorGraph;
		std::string m_strAreaId;
		std::string m_strEncounterId;
		std::string m_strCombatRuntimeRevision;
		std::string m_strStatus;
		std::uint32_t m_iFixedTickHz = 0u;
	};
}
