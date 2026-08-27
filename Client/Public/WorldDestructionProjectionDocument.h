#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

NS_BEGIN(Client)

struct WORLD_DESTRUCTION_PROJECTION_GROUP final
{
	std::string strGroupId;
	std::string strMutationId;
	bool_t bRemovesGround = false;
	std::vector<uint64_t> MemberPlacementIds;
	std::vector<uint64_t> SuppressionAliasPlacementIds;
};

class CWorldDestructionProjectionDocument final
{
public:
	bool_t Load(const std::filesystem::path& path, std::string& outStatus);
	static bool_t Parse_Text(
		std::string_view text,
		CWorldDestructionProjectionDocument& outDocument,
		std::string& outStatus);
	void Clear();

	bool_t Is_Ready() const { return m_isReady; }
	const std::string& Get_AreaId() const { return m_strAreaId; }
	const std::string& Get_CombatRuntimeRevision() const
	{
		return m_strCombatRuntimeRevision;
	}
	const std::vector<WORLD_DESTRUCTION_PROJECTION_GROUP>& Get_Groups() const
	{
		return m_Groups;
	}
	const WORLD_DESTRUCTION_PROJECTION_GROUP* Find_Group(
		std::string_view groupId) const;

private:
	std::string m_strAreaId;
	std::string m_strCombatRuntimeRevision;
	std::vector<WORLD_DESTRUCTION_PROJECTION_GROUP> m_Groups;
	bool_t m_isReady = false;
};

NS_END
