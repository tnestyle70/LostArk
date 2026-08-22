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
	float fPreviewScale = 0.01f;
	float fPreviewYawDegrees = -90.f;
	bool bPlaybackOnly = false;
	const char* pBossArchetypeId = nullptr;
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
	},
	ANIMATION_PREVIEW_ASSET
	{
		"monster.480001.mn-padd-01",
		"[Monster] Normal 01 - MN_PADD_01 (36 clips)",
		"Monster_480001_MN_PADD_01",
		"Character/Monster/NPC_480001_MN_PADD_01/NPC_480001_MN_PADD_01.wmodel",
		L"Prototype_Component_Model_AnimationPreview_Monster_480001",
		false,
		0.01f,
		-90.f,
		true
	},
	ANIMATION_PREVIEW_ASSET
	{
		"monster.480002.mn-sjfc-00-4",
		"[Monster] Normal 02 - MN_SJFC_00_4 (29 clips)",
		"Monster_480002_MN_SJFC_00_4",
		"Character/Monster/NPC_480002_MN_SJFC_00_4/NPC_480002_MN_SJFC_00_4.wmodel",
		L"Prototype_Component_Model_AnimationPreview_Monster_480002",
		false,
		0.01f,
		-90.f,
		true
	},
	ANIMATION_PREVIEW_ASSET
	{
		"monster.480003.mn-0019-05",
		"[Monster] Normal 03 - MN_0019_05 (25 clips)",
		"Monster_480003_MN_0019_05",
		"Character/Monster/NPC_480003_MN_0019_05/NPC_480003_MN_0019_05.wmodel",
		L"Prototype_Component_Model_AnimationPreview_Monster_480003",
		false,
		0.01f,
		-90.f,
		true
	},
	ANIMATION_PREVIEW_ASSET
	{
		"monster.480005.lugaru",
		"[Mid Boss] Commander Lugaru - MN_RPRS_02 (91 clips)",
		"Monster_480005_Lugaru_MN_RPRS_02",
		"Character/Monster/NPC_480005_MN_RPRS_02/NPC_480005_MN_RPRS_02.wmodel",
		L"Prototype_Component_Model_AnimationPreview_Monster_480005_Lugaru",
		false,
		0.01f,
		-90.f,
		true
	},
	ANIMATION_PREVIEW_ASSET
	{
		"boss.valtan",
		"[Boss] Valtan - MN_RPBF_01 + AnimSet (173 clips)",
		"Valtan",
		"Character/Valtan/MN_RPBF_01.wmodel",
		L"Prototype_Component_Model_AnimationPreview_Boss_Valtan",
		false,
		0.0001f,
		-90.f,
		true,
		"BOSS_VALTAN"
	},
	ANIMATION_PREVIEW_ASSET
	{
		"boss.valtan.ghost",
		"[Boss] Ghost Valtan - MN_RPBF_02 (140 clips)",
		"Valtan_Ghost_MN_RPBF_02",
		"Character/Valtan/Ghost/MN_RPBF_02.wmodel",
		L"Prototype_Component_Model_AnimationPreview_Boss_Valtan_Ghost",
		false,
		0.01f,
		-90.f,
		true
	}
};

}
