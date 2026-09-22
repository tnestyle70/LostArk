#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace Client
{

// Physical clip identity includes its body and source package. The same clip
// name on two rigs remains two selectable resources.
struct COMPOSITION_ANIMATION_RESOURCE final
{
	std::string strTargetAssetName;
	std::string strModelAssetId;
	std::string strSourceAssetId;
	std::string strProfileId;
	std::string strRuntimeClip;
	std::uint32_t iDurationMs = 0u;
	std::string strEndPolicy = "EXACT";
	// Runtime ticks and cooked clock retain precision when a Sequence overrides
	// iDurationMs. Save admission never depends on the selected preview rig.
	float fDurationTicks = 0.f;
	float fTicksPerSecond = 0.f;
};

struct COMPOSITION_ANIMATION_SEQUENCE_RESOURCE final
{
	std::string strStableId;
	std::string strDisplayName;
	std::string strProfileId;
	std::string strTargetAssetName;
	std::vector<COMPOSITION_ANIMATION_RESOURCE> Clips;
};

inline constexpr std::array<const char*, 21u>
	COMPOSITION_ANIMATION_TARGET_ASSET_NAMES = {
		"Valtan", "Valtan_Ghost_MN_RPBF_02",
		"MN_RPCT_00", "MN_RPCT_03", "MN_RPCT_05", "MN_RPCT_06", "MN_RPCZ_00", "MN_RPCZ_00-1",
		"Monster_480001_MN_PADD_01",
		"Monster_480002_MN_SJFC_00_4",
		"Monster_480003_MN_0019_05",
		"Monster_480005_Lugaru_MN_RPRS_02",
		"Monster_Valtan_SJFC_Elite",
		"Monster_Kouku_CMDUP_02",
		"Monster_Kouku_REUP_04",
		"Monster_Kouku_RHKP_06",
		"Monster_Kouku_CMDGR_03",
		"Monster_Card_Heart",
		"Monster_Card_Diamond",
		"Monster_Card_Club",
		"Monster_Card_Spade" };

inline bool Is_CompositionAnimationTargetAsset(const std::string_view name)
{
	for (const char* candidate : COMPOSITION_ANIMATION_TARGET_ASSET_NAMES)
	{
		if (name == candidate)
			return true;
	}
	return false;
}

}
