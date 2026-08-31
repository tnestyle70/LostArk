#include "PlayableCharacterPreviewContract.h"

#include "ActorCatalog.h"
#include "AnimationPreviewAssets.h"
#include "CharacterCatalog.h"
#include "CharacterSpec.h"

#include <array>
#include <cmath>
#include <set>
#include <string_view>

namespace
{
	LostArk::Shared::PLAYER_STANCE_ID Resolve_FallbackStance(
		const LostArk::Shared::CHARACTER_CLASS_ID CharacterClass)
	{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		using LostArk::Shared::PLAYER_STANCE_ID;
		switch (CharacterClass)
		{
		case CHARACTER_CLASS_ID::LANCE_MASTER:
			return PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
		case CHARACTER_CLASS_ID::WARLORD:
			return PLAYER_STANCE_ID::WARLORD_NORMAL;
		default:
			return PLAYER_STANCE_ID::NONE;
		}
	}

	uint32_t Count_VisibleWeapons(
		const Client::CHARACTER_SPEC& Spec,
		const LostArk::Shared::PLAYER_STANCE_ID Stance)
	{
		using LostArk::Shared::PLAYER_STANCE_ID;
		uint32_t Count = 0u;
		for (uint32_t Index = 0u; Index < Spec.iNumWeapons; ++Index)
		{
			const Client::WEAPON_PART_SPEC& Weapon = Spec.pWeapons[Index];
			if (PLAYER_STANCE_ID::NONE == Weapon.eRequiredStance ||
				Weapon.eRequiredStance == Stance)
			{
				++Count;
			}
		}
		return Count;
	}

	const Client::CHARACTER_SPEC* Find_SpecByAssetName(
		const std::string_view AssetName)
	{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		constexpr std::array Classes =
		{
			CHARACTER_CLASS_ID::LANCE_MASTER,
			CHARACTER_CLASS_ID::GUNSLINGER,
			CHARACTER_CLASS_ID::SLAYER,
			CHARACTER_CLASS_ID::ARTIST,
			CHARACTER_CLASS_ID::DIMENSIONMASTER,
			CHARACTER_CLASS_ID::WARLORD
		};
		for (const CHARACTER_CLASS_ID CharacterClass : Classes)
		{
			const Client::CHARACTER_SPEC* Spec =
				Client::CCharacterCatalog::Find_Spec(CharacterClass);
			if (nullptr != Spec && nullptr != Spec->pAssetName &&
				AssetName == Spec->pAssetName)
			{
				return Spec;
			}
		}
		return nullptr;
	}
}

bool_t Client::CPlayableCharacterPreviewContract::Stage(
	const ANIMATION_PREVIEW_ASSET& Asset,
	PLAYABLE_CHARACTER_PREVIEW_COMPOSITION& OutComposition)
{
	if (nullptr == Asset.pAssetName)
		return false;
	const CHARACTER_SPEC* Spec = Find_SpecByAssetName(Asset.pAssetName);
	if (nullptr == Spec)
		return false;
	const CHARACTER_ACTOR_ENTRY* Actor =
		CActorCatalog::Find_Character(Spec->eCharacterClass);
	if (nullptr == Actor)
		return false;
	return Stage(Asset, *Spec, *Actor, OutComposition);
}

bool_t Client::CPlayableCharacterPreviewContract::Stage(
	const ANIMATION_PREVIEW_ASSET& Asset,
	const CHARACTER_SPEC& Spec,
	const CHARACTER_ACTOR_ENTRY& Actor,
	PLAYABLE_CHARACTER_PREVIEW_COMPOSITION& OutComposition)
{
	if (!Asset.bPlayableClassBody || nullptr == Asset.pAssetName ||
		nullptr == Asset.pModelAssetId || nullptr == Asset.pPrototypeTag ||
		nullptr == Spec.pAssetName || nullptr == Spec.pBodyModelTag ||
		nullptr == Spec.pShaderTag || nullptr == Spec.pWeaponShaderTag ||
		Spec.eCharacterClass != Actor.networkClassId ||
		Asset.pAssetName != std::string_view(Spec.pAssetName) ||
		0 != wcscmp(Asset.pPrototypeTag, Spec.pBodyModelTag) ||
		Actor.assetId != Asset.pAssetName ||
		Actor.bodyModel != Asset.pModelAssetId ||
		Actor.runtimeStatus != "supported" ||
		Actor.equipmentModels.size() != Spec.iNumEquipment ||
		Actor.weaponModels.size() != Spec.iNumWeapons ||
		0u == Spec.iNumWeapons ||
		(0u != Spec.iNumEquipment && nullptr == Spec.pEquipment) ||
		nullptr == Spec.pWeapons)
	{
		return false;
	}

	std::set<std::wstring_view> PartTags;
	for (uint32_t Index = 0u; Index < Spec.iNumEquipment; ++Index)
	{
		const EQUIPMENT_PART_SPEC& Part = Spec.pEquipment[Index];
		if (nullptr == Part.pPartTag || nullptr == Part.pModelTag ||
			Part.ePresentationSlot >= EQUIPMENT_PRESENTATION_SLOT::WEAPON ||
			!PartTags.insert(Part.pPartTag).second)
		{
			return false;
		}
	}
	for (uint32_t Index = 0u; Index < Spec.iNumWeapons; ++Index)
	{
		const WEAPON_PART_SPEC& Part = Spec.pWeapons[Index];
		if (nullptr == Part.pPartTag || nullptr == Part.pModelTag ||
			nullptr == Part.pSocketBone || '\0' == Part.pSocketBone[0] ||
			!std::isfinite(Part.fSocketYawDegrees) ||
			!PartTags.insert(Part.pPartTag).second)
		{
			return false;
		}
	}

	PLAYABLE_CHARACTER_PREVIEW_COMPOSITION Staged;
	Staged.pSpec = &Spec;
	Staged.pActor = &Actor;
	Staged.eOwnerKind = PLAYABLE_PREVIEW_OWNER_KIND::CHARACTER;
	/* CCharacter::Ready_PartObjects supplies the body model as skeleton/palette
	   provider to every equipment part and as socket provider to every weapon. */
	Staged.iBodyPalettePartCount = Spec.iNumEquipment;
	Staged.iBodySocketPartCount = Spec.iNumWeapons;
	Staged.eFallbackStance = Resolve_FallbackStance(Spec.eCharacterClass);
	Staged.iFallbackVisibleWeaponPartCount =
		Count_VisibleWeapons(Spec, Staged.eFallbackStance);
	OutComposition = Staged;
	return true;
}
