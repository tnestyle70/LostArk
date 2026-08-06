#pragma once

#include <array>

namespace Client
{

struct ANIMATION_PREVIEW_ASSET final
{
	const char* pId = nullptr;
	const char* pLabel = nullptr;
	const char* pAssetName = nullptr;
	const char* pModelAssetId = nullptr;
	const wchar_t* pPrototypeTag = nullptr;
	bool bPlayableClassBody = false;
};

// Debug authoring targets reuse the same CModel path as the playable
// DimensionMaster body. The core and summon remain optional preview-only models;
// the Animation Tool drives whichever admitted model target is selected.
inline constexpr std::array ANIMATION_PREVIEW_ASSETS =
{
	ANIMATION_PREVIEW_ASSET
	{
		"lancemaster.character",
		"LanceMaster Character",
		"LanceMaster",
		"Character/LanceMaster/LanceMaster.wmodel",
		L"Prototype_Component_Model_LanceMaster",
		true
	},
	ANIMATION_PREVIEW_ASSET
	{
		"gunslinger.character",
		"Gunslinger Character",
		"GunSlinger",
		"Character/GunSlinger/GunSlinger.wmodel",
		L"Prototype_Component_Model_GunSlinger",
		true
	},
	ANIMATION_PREVIEW_ASSET
	{
		"slayer.character",
		"Slayer Character",
		"Slayer",
		"Character/Slayer/Slayer.wmodel",
		L"Prototype_Component_Model_Slayer",
		true
	},
	ANIMATION_PREVIEW_ASSET
	{
		"artist.character",
		"Artist Character",
		"Artist",
		"Character/Artist/Artist.wmodel",
		L"Prototype_Component_Model_Artist",
		true
	},
	ANIMATION_PREVIEW_ASSET
	{
		"dimensionmaster.character",
		"DimensionMaster Character (154 clips)",
		"DimensionMaster",
		"Character/DimensionMaster/DimensionMaster_Character.wmodel",
		L"Prototype_Component_Model_DimensionMaster",
		true
	},
	ANIMATION_PREVIEW_ASSET
	{
		"dimensionmaster.dimension-core",
		"Dimension Core (sk_super_instance)",
		"DimensionMaster_DimensionCore",
		"Character/DimensionMaster/DimensionMaster_DimensionCore.wmodel",
		L"Prototype_Component_Model_AnimationPreview_DimensionCore"
	},
	ANIMATION_PREVIEW_ASSET
	{
		"dimensionmaster.dimension-summon",
		"Dimension Summon (2 clips)",
		"DimensionMaster_DimensionSummon",
		"Character/DimensionMaster/DimensionMaster_DimensionSummon.wmodel",
		L"Prototype_Component_Model_AnimationPreview_DimensionSummon"
	},
	ANIMATION_PREVIEW_ASSET
	{
		"warlord.character",
		"Warlord Character (193 clips)",
		"Warlord",
		"Character/Warlord/Warlord.wmodel",
		L"Prototype_Component_Model_Warlord",
		true
	}
};

}
