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
};

// Debug authoring targets reuse the same CModel path as the playable
// Dimensionist body. The core and summon remain optional preview-only models;
// the Animation Tool drives whichever admitted model target is selected.
inline constexpr std::array ANIMATION_PREVIEW_ASSETS =
{
	ANIMATION_PREVIEW_ASSET
	{
		"dimensionist.character",
		"Dimensionist Character (154 clips)",
		"Dimensionist",
		"Character/Dimensionist/Dimensionist_Character.wmodel",
		L"Prototype_Component_Model_Dimensionist"
	},
	ANIMATION_PREVIEW_ASSET
	{
		"dimensionist.dimension-core",
		"Dimension Core (sk_super_instance)",
		"Dimensionist_DimensionCore",
		"Character/Dimensionist/Dimensionist_DimensionCore.wmodel",
		L"Prototype_Component_Model_AnimationPreview_DimensionCore"
	},
	ANIMATION_PREVIEW_ASSET
	{
		"dimensionist.dimension-summon",
		"Dimension Summon (2 clips)",
		"Dimensionist_DimensionSummon",
		"Character/Dimensionist/Dimensionist_DimensionSummon.wmodel",
		L"Prototype_Component_Model_AnimationPreview_DimensionSummon"
	}
};

}
