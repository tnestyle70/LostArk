#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

NS_BEGIN(Client)

class CWorldDestructionProjectionDocument;

struct WORLD_DESTRUCTION_DEBRIS_EMITTER final
{
	uint64_t iSourceRuntimePlacementId = 0u;
	std::vector<uint64_t> SuppressionAliasPlacementIds;
	float3_t vSpawnOffset{};
	float3_t vDirection = { 0.f, 1.f, 0.f };
	f32_t fSpeedMetersPerSecond = 0.f;
	f32_t fGravityScale = 1.f;
	f32_t fLifetimeSeconds = 4.f;
};

struct WORLD_DESTRUCTION_DEBRIS_PROFILE final
{
	std::string strGroupId;
	std::string strMutationId;
	std::string strBindingId;
	std::vector<WORLD_DESTRUCTION_DEBRIS_EMITTER> Emitters;
};

class CWorldDestructionDebrisPresentationDocument final
{
public:
	bool_t Load(const std::filesystem::path& path, std::string& outStatus);
	static bool_t Parse_Text(
		std::string_view text,
		CWorldDestructionDebrisPresentationDocument& outDocument,
		std::string& outStatus);
	bool_t Validate_Against(
		const CWorldDestructionProjectionDocument& projection,
		std::string& outStatus) const;
	void Clear();

	bool_t Is_Ready() const { return m_isReady; }
	const std::string& Get_AreaId() const { return m_strAreaId; }
	const std::string& Get_CombatRuntimeRevision() const
	{
		return m_strCombatRuntimeRevision;
	}
	const std::vector<WORLD_DESTRUCTION_DEBRIS_PROFILE>& Get_Profiles() const
	{
		return m_Profiles;
	}
	const WORLD_DESTRUCTION_DEBRIS_PROFILE* Find_Group(
		std::string_view groupId) const;
	const WORLD_DESTRUCTION_DEBRIS_PROFILE* Find_Profile(
		std::string_view groupId) const { return Find_Group(groupId); }

private:
	std::string m_strAreaId;
	std::string m_strCombatRuntimeRevision;
	std::vector<WORLD_DESTRUCTION_DEBRIS_PROFILE> m_Profiles;
	bool_t m_isReady = false;
};

NS_END
