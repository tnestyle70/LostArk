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

inline constexpr std::array<const char*, 6u>
	COMPOSITION_ANIMATION_TARGET_ASSET_NAMES = {
		"Valtan", "Valtan_Ghost_MN_RPBF_02",
		"MN_RPCT_00", "MN_RPCT_05", "MN_RPCT_06", "MN_RPCZ_00" };

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
