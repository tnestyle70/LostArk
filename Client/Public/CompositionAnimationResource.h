#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

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
	// Display-only; native clip and package identity remain the append key.
	std::string strDisplayName;
};

struct COMPOSITION_ANIMATION_SEQUENCE_RESOURCE final
{
	std::string strStableId;
	std::string strDisplayName;
	std::string strProfileId;
	std::string strTargetAssetName;
	std::vector<COMPOSITION_ANIMATION_RESOURCE> Clips;
};

inline constexpr std::array<const char*, 28u>
	COMPOSITION_ANIMATION_TARGET_ASSET_NAMES = {
		"LanceMaster", "GunSlinger", "Slayer", "Artist", "DimensionMaster", "Warlord", "GuardianKnight",
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

// Optional source labels never determine which clips exist or can be played.
std::unordered_map<std::string, std::string> Read_CompositionAnimationDisplayNames(
    const std::string& assetName);

inline std::vector<std::string> CompositionAnimationCategory(const std::string& assetName)
{
    for (std::size_t i = 0u; i < 7u; ++i)
        if (assetName == COMPOSITION_ANIMATION_TARGET_ASSET_NAMES[i])
            return {"Character", assetName == "GunSlinger" ? "Gunslinger" : assetName};
    if (assetName == "MN_RPCZ_00-1") return {"Character Transform", "Mario / Clown"};
    if (assetName == "Valtan") return {"Boss", "Valtan"};
    if (assetName == "Valtan_Ghost_MN_RPBF_02") return {"Boss", "Ghost Valtan"};
    if (assetName == "MN_RPCT_00") return {"Boss", "Saydon"};
    if (assetName == "MN_RPCT_03") return {"Boss", "Saydon (colorless)"};
    if (assetName == "MN_RPCT_05") return {"Boss", "Saydon (Gate 3 / Encore)"};
    if (assetName == "MN_RPCT_06") return {"Boss", "Large Saydon"};
    if (assetName == "MN_RPCZ_00") return {"Boss", "Kouku"};
    return {"Other models", assetName};
}

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
