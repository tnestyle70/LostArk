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

	/* Optional animation donor merged into this body's clip table through
	CModel::Attach_AnimationSet. It must be built against this body's rig: the
	attach is refused when the skeleton hash differs, and refused again when a
	clip name is already present, so a mismatched donor fails loudly. */
	const char* pAnimationSetAssetId = nullptr;

	/* Optional socketed weapon for a preview-only body. The piece rides one bone
	of the body's own skeleton, so it carries no animation and no clip list. All
	four fields are declared together or none of them are: a half-declared weapon
	is rejected rather than silently previewed without its piece. */
	const char* pWeaponModelAssetId = nullptr;
	const wchar_t* pWeaponPrototypeTag = nullptr;
	const char* pWeaponSocketBone = nullptr;
	/* Converts the weapon's authored units into the body's, and nothing else.
	This is not the preview scale: the socket bone matrix already carries the
	body's pre-transform, so the preview scale cancels out of the ratio. */
	float fWeaponScale = 1.f;
	const char* pWeaponMaterialProfileId = nullptr;
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
	/* KoukuSaydon extracted bodies are Development authoring references only.
	   They intentionally have no bossArchetypeId until a Server Product world,
	   encounter and pattern contract admits a concrete subset. */
	ANIMATION_PREVIEW_ASSET
	{
		"kakulsaydon.mn-rpct-00",
		"[KoukuSaydon Reference] MN_RPCT_00 (249 clips)",
		"MN_RPCT_00",
		"Character/KoukuSaton/MN_RPCT_00/MN_RPCT_00.wmodel",
		L"Prototype_Component_Model_AnimationPreview_KoukuSaydon_MN_RPCT_00",
		false,
		0.01f,
		-90.f,
		true
	},
	ANIMATION_PREVIEW_ASSET
	{
		"kakulsaydon.mn-rpct-05",
		"[KoukuSaydon Reference] MN_RPCT_05 / MN_RPCT_07 alias (249 clips)",
		"MN_RPCT_05",
		"Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05.wmodel",
		L"Prototype_Component_Model_AnimationPreview_KoukuSaydon_MN_RPCT_05",
		false,
		0.01f,
		-90.f,
		true
	},
	ANIMATION_PREVIEW_ASSET
	{
		"kakulsaydon.mn-rpct-06",
		"[KoukuSaydon Reference] MN_RPCT_06 (34 clips)",
		"MN_RPCT_06",
		"Character/KoukuSaton/MN_RPCT_06/MN_RPCT_06.wmodel",
		L"Prototype_Component_Model_AnimationPreview_KoukuSaydon_MN_RPCT_06",
		false,
		0.01f,
		-90.f,
		true
	},
	ANIMATION_PREVIEW_ASSET
	{
		"kakulsaydon.mn-rpcz-00",
		"[KoukuSaydon Reference] MN_RPCZ_00 (91 clips)",
		"MN_RPCZ_00",
		"Character/KoukuSaton/MN_RPCZ_00/MN_RPCZ_00.wmodel",
		L"Prototype_Component_Model_AnimationPreview_KoukuSaydon_MN_RPCZ_00",
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
		true,
		nullptr,
		/* Baked by Tools/ModelAssetConverter/bake_ghost_valtan_animset.py from
		the product AnimSet: the same 146 clips retagged onto this rig and
		rescaled to its units, keeping the product mesh_* names. With it
		attached both Valtan bodies answer to one clip vocabulary, so a chain
		authored on either plays on the other with no rename step. */
		"Character/Valtan/Ghost/MN_RPBF_02_AnimSet.wmodel",
		/* The ghost body carries the same 87-bone rig and the same b_wp_r_01
		hand socket as the product body, so it holds the product axe rather than
		a second authored asset. MN_RPBF_02 is authored 100x smaller than
		MN_RPBF_01 (b_wp_r_01 bind translation 0.34437 against 34.43719), which
		is why its preview scale is 0.01 against the product's 0.0001. Both
		bodies therefore render at one size, and the axe that needs 100 against
		the product body needs 1 here. */
		"Character/Valtan/ValtanWeapon.wmodel",
		L"Prototype_Component_Model_AnimationPreview_Boss_Valtan_Ghost_Weapon",
		"b_wp_r_01",
		1.f,
		"material.valtan.monster-base.v1"
	}
};

}
