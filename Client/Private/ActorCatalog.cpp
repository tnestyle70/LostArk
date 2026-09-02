#include "ActorCatalog.h"

#include "DataJson.h"
#include "ProjectDataRoot.h"

#include <algorithm>
#include <fstream>
#include <cctype>
#include <cmath>
#include <limits>
#include <set>

namespace
{
	using namespace Client;

	std::vector<CHARACTER_ACTOR_ENTRY> g_Characters;
	std::vector<BOSS_ACTOR_ENTRY> g_Bosses;
	std::vector<NPC_ACTOR_ENTRY> g_Npcs;
	std::vector<MONSTER_ACTOR_ENTRY> g_Monsters;
	std::string g_Status = "Actor catalog is not initialized.";
	bool_t g_isInitialized = false;

	bool_t ReadText(const std::filesystem::path& path, std::string& outText)
	{
		std::ifstream input(path, std::ios::binary);
		if (!input.is_open())
			return false;
		outText.assign(
			std::istreambuf_iterator<char>(input),
			std::istreambuf_iterator<char>());
		return input.good() || input.eof();
	}

	bool_t ReadDocument(
		const std::filesystem::path& relativePath,
		DATA_JSON_VALUE& outRoot)
	{
		const std::filesystem::path path =
			CProjectDataRoot::Resolve(relativePath);
		std::string text;
		std::string error;
		return !path.empty() && ReadText(path, text) &&
			CDataJson::Parse(text, outRoot, error) && outRoot.Is_Object();
	}

	bool_t ReadRequiredString(
		const DATA_JSON_VALUE& object,
		const char_t* pName,
		std::string& outValue)
	{
		const DATA_JSON_VALUE* pValue = object.Find(pName);
		if (nullptr == pValue || !pValue->Is_String() ||
			pValue->Get_String().empty())
		{
			return false;
		}
		outValue = pValue->Get_String();
		return true;
	}

	bool_t ReadRequiredNumber(
		const DATA_JSON_VALUE& object,
		const char_t* pName,
		f32_t& outValue)
	{
		const DATA_JSON_VALUE* pValue = object.Find(pName);
		if (nullptr == pValue || !pValue->Is_Number() ||
			!std::isfinite(pValue->Get_Number()))
		{
			return false;
		}
		outValue = static_cast<f32_t>(pValue->Get_Number());
		return true;
	}

	bool_t ReadCombatVisualWorldScale(
		const DATA_JSON_VALUE& object,
		float3_t& outScale)
	{
		const DATA_JSON_VALUE* scale = object.Find("worldScale");
		if (nullptr == scale)
			return true;
		if (!scale->Is_Array() || scale->Get_Array().size() != 3u)
			return false;
		f32_t components[3]{};
		for (size_t index = 0u; index < 3u; ++index)
		{
			const DATA_JSON_VALUE& component = scale->Get_Array()[index];
			if (!component.Is_Number() ||
				!std::isfinite(component.Get_Number()) ||
				component.Get_Number() <= 0.0 || component.Get_Number() > 100.0)
			{
				return false;
			}
			components[index] = static_cast<f32_t>(component.Get_Number());
			if (components[index] <= 0.f)
				return false;
		}
		outScale = float3_t(components[0], components[1], components[2]);
		return true;
	}

	bool_t ReadRequiredU32(
		const DATA_JSON_VALUE& object,
		const char_t* pName,
		std::uint32_t& outValue)
	{
		const DATA_JSON_VALUE* pValue = object.Find(pName);
		if (nullptr == pValue || !pValue->Is_Number())
			return false;
		const double value = pValue->Get_Number();
		if (!std::isfinite(value) || value < 0.0 ||
			value > static_cast<double>(
				(std::numeric_limits<std::uint32_t>::max)()) ||
			std::floor(value) != value)
		{
			return false;
		}
		outValue = static_cast<std::uint32_t>(value);
		return true;
	}

	bool_t IsStableId(const std::string& value)
	{
		if (value.empty() || value.size() > 64u)
			return false;
		return std::all_of(value.begin(), value.end(),
			[](const unsigned char character)
			{
				return 0 != std::isalnum(character) || character == '_' ||
					character == '-' || character == '.';
			});
	}

	bool_t IsResourceId(const std::string& value)
	{
		if (!value.starts_with("Character/") ||
			!std::filesystem::path(value).is_relative())
		{
			return false;
		}
		for (const auto& part : std::filesystem::path(value))
			if (part == L"..")
				return false;
		return std::filesystem::path(value).extension() == L".wmodel";
	}

	LostArk::Shared::CHARACTER_CLASS_ID ParseClass(const std::string& value)
	{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		if (value == "LANCE_MASTER") return CHARACTER_CLASS_ID::LANCE_MASTER;
		if (value == "GUNSLINGER") return CHARACTER_CLASS_ID::GUNSLINGER;
		if (value == "SLAYER") return CHARACTER_CLASS_ID::SLAYER;
		if (value == "ARTIST") return CHARACTER_CLASS_ID::ARTIST;
		if (value == "DESTROYER") return CHARACTER_CLASS_ID::DESTROYER;
		if (value == "DIMENSIONMASTER") return CHARACTER_CLASS_ID::DIMENSIONMASTER;
		if (value == "WARLORD") return CHARACTER_CLASS_ID::WARLORD;
		return CHARACTER_CLASS_ID::END;
	}

	bool_t ParseCharacters(const DATA_JSON_VALUE& root)
	{
		const DATA_JSON_VALUE* pSchema = root.Find("schema");
		const DATA_JSON_VALUE* pVersion = root.Find("formatVersion");
		const DATA_JSON_VALUE* pEntries = root.Find("characters");
		if (nullptr == pSchema || !pSchema->Is_String() ||
			pSchema->Get_String() != "lostark.character-catalog" ||
			nullptr == pVersion || !pVersion->Is_Number() ||
			pVersion->Get_Number() != 3.0 ||
			nullptr == pEntries || !pEntries->Is_Array())
		{
			return false;
		}

		std::set<std::string> archetypes;
		std::set<LostArk::Shared::CHARACTER_CLASS_ID> classes;
		std::vector<CHARACTER_ACTOR_ENTRY> staged;
		for (const DATA_JSON_VALUE& value : pEntries->Get_Array())
		{
			if (!value.Is_Object())
				return false;
			CHARACTER_ACTOR_ENTRY entry;
			std::string networkClassId;
			if (!ReadRequiredString(value, "archetypeId", entry.archetypeId) ||
				!ReadRequiredString(value, "networkClassId", networkClassId) ||
				!ReadRequiredString(value, "assetId", entry.assetId) ||
				!ReadRequiredString(value, "bodyModel", entry.bodyModel) ||
				!ReadRequiredString(value, "animationSetId", entry.animationSetId) ||
				!ReadRequiredString(value, "runtimeStatus", entry.runtimeStatus))
			{
				return false;
			}
			entry.networkClassId = ParseClass(networkClassId);
			const DATA_JSON_VALUE* pEquipment = value.Find("equipmentModels");
			const DATA_JSON_VALUE* pWeapons = value.Find("weaponModels");
			const DATA_JSON_VALUE* pAnimSetModel =
				value.Find("animationSetModel");
			if (LostArk::Shared::CHARACTER_CLASS_ID::END == entry.networkClassId ||
				!IsResourceId(entry.bodyModel) ||
				nullptr == pAnimSetModel ||
				(!pAnimSetModel->Is_Null() && !pAnimSetModel->Is_String()) ||
				nullptr == pEquipment || !pEquipment->Is_Array() ||
				nullptr == pWeapons || !pWeapons->Is_Array() ||
				pWeapons->Get_Array().size() > 4u ||
				(entry.runtimeStatus != "supported" &&
				 entry.runtimeStatus != "reserved") ||
				!archetypes.insert(entry.archetypeId).second ||
				!classes.insert(entry.networkClassId).second)
			{
				return false;
			}
			if (pAnimSetModel->Is_String())
			{
				entry.animationSetModel = pAnimSetModel->Get_String();
				if (!IsResourceId(entry.animationSetModel))
					return false;
			}
			for (const DATA_JSON_VALUE& equipment : pEquipment->Get_Array())
			{
				if (!equipment.Is_String() || !IsResourceId(equipment.Get_String()))
					return false;
				entry.equipmentModels.push_back(equipment.Get_String());
			}
			for (const DATA_JSON_VALUE& weapon : pWeapons->Get_Array())
			{
				if (!weapon.Is_String() || !IsResourceId(weapon.Get_String()))
					return false;
				entry.weaponModels.push_back(weapon.Get_String());
			}
			const bool_t hasEquipment = !entry.equipmentModels.empty();
			const bool_t hasWeapon = !entry.weaponModels.empty();
			if (entry.runtimeStatus == "supported" &&
				!hasWeapon)
			{
				return false;
			}
			if (entry.runtimeStatus == "reserved" &&
				(hasEquipment || hasWeapon))
			{
				return false;
			}
			staged.push_back(std::move(entry));
		}
		g_Characters = std::move(staged);
		return !g_Characters.empty();
	}

	bool_t ParseBosses(const DATA_JSON_VALUE& root)
	{
		const DATA_JSON_VALUE* pSchema = root.Find("schema");
		const DATA_JSON_VALUE* pVersion = root.Find("formatVersion");
		const DATA_JSON_VALUE* pEntries = root.Find("bosses");
		if (nullptr == pSchema || !pSchema->Is_String() ||
			pSchema->Get_String() != "lostark.boss-catalog" ||
			nullptr == pVersion || !pVersion->Is_Number() ||
			pVersion->Get_Number() != 6.0 ||
			nullptr == pEntries || !pEntries->Is_Array() ||
			3u != root.Get_Object().size())
		{
			return false;
		}

		std::set<std::string> archetypes;
		std::vector<BOSS_ACTOR_ENTRY> staged;
		for (const DATA_JSON_VALUE& value : pEntries->Get_Array())
		{
			if (!value.Is_Object() || 15u != value.Get_Object().size())
				return false;
			BOSS_ACTOR_ENTRY entry;
			const DATA_JSON_VALUE* pClips = value.Find("presentationClips");
			const DATA_JSON_VALUE* pArmor = value.Find("armorModels");
			const DATA_JSON_VALUE* pArmorParts = value.Find("armorParts");
			const DATA_JSON_VALUE* pCombatObjectVisuals =
				value.Find("combatObjectVisuals");
			if (!ReadRequiredString(value, "archetypeId", entry.archetypeId) ||
				!ReadRequiredString(value, "visualAssetId", entry.visualAssetId) ||
				!ReadRequiredNumber(
					value, "presentationScale", entry.presentationScale) ||
				!ReadRequiredNumber(value, "bodyModelPreScale", entry.bodyModelPreScale) ||
				!ReadRequiredNumber(value, "weaponModelPreScale", entry.weaponModelPreScale) ||
				!ReadRequiredString(value, "bodyModel", entry.bodyModel) ||
				!ReadRequiredString(value, "weaponModel", entry.weaponModel) ||
				!ReadRequiredString(value, "animationSetId", entry.animationSetId) ||
				!ReadRequiredString(value, "serverProfileId", entry.serverProfileId) ||
				!ReadRequiredString(value, "clientPresentationId", entry.clientPresentationId) ||
				!ReadRequiredString(value, "presentationStatus", entry.presentationStatus) ||
				nullptr == pClips || !pClips->Is_Object() ||
				6u != pClips->Get_Object().size() ||
				!ReadRequiredString(*pClips, "idle", entry.presentationClips.idle) ||
				!ReadRequiredString(*pClips, "chase", entry.presentationClips.chase) ||
				!ReadRequiredString(*pClips, "patternWindup", entry.presentationClips.patternWindup) ||
				!ReadRequiredString(*pClips, "patternActive", entry.presentationClips.patternActive) ||
				!ReadRequiredString(*pClips, "patternRecovery", entry.presentationClips.patternRecovery) ||
				!ReadRequiredString(*pClips, "dead", entry.presentationClips.dead) ||
				!IsStableId(entry.archetypeId) ||
				!IsResourceId(entry.bodyModel) ||
				!IsResourceId(entry.weaponModel) ||
				!IsResourceId(entry.animationSetId) ||
				entry.presentationScale <= 0.f ||
				entry.presentationScale > 100.f ||
				!std::isfinite(entry.bodyModelPreScale) ||
				entry.bodyModelPreScale <= 0.f || entry.bodyModelPreScale > 100.f ||
				!std::isfinite(entry.weaponModelPreScale) ||
				entry.weaponModelPreScale <= 0.f || entry.weaponModelPreScale > 100.f ||
				nullptr == pArmor || !pArmor->Is_Array() ||
				pArmor->Get_Array().size() > 4u ||
				pArmor->Get_Array().size() > MAX_BOSS_ARMOR_PARTS ||
				nullptr == pArmorParts || !pArmorParts->Is_Array() ||
				pArmorParts->Get_Array().size() != pArmor->Get_Array().size() ||
				nullptr == pCombatObjectVisuals ||
				!pCombatObjectVisuals->Is_Array() ||
				pCombatObjectVisuals->Get_Array().size() >
					MAX_BOSS_COMBAT_OBJECT_VISUALS ||
				!archetypes.insert(entry.archetypeId).second ||
				(entry.presentationStatus != "complete" &&
				 entry.presentationStatus != "fallback"))
			{
				return false;
			}
			for (const DATA_JSON_VALUE& armor : pArmor->Get_Array())
			{
				if (!armor.Is_String() || !IsResourceId(armor.Get_String()))
					return false;
				entry.armorModels.push_back(armor.Get_String());
			}
			std::set<std::string> armorPartIds;
			std::set<std::uint32_t> armorStateMasks;
			for (const DATA_JSON_VALUE& armorPartValue : pArmorParts->Get_Array())
			{
				BOSS_ARMOR_PART_ENTRY armorPart;
				if (!armorPartValue.Is_Object() ||
					3u != armorPartValue.Get_Object().size() ||
					!ReadRequiredString(armorPartValue, "partId", armorPart.partId) ||
					!ReadRequiredU32(
						armorPartValue, "stateMask", armorPart.stateMask) ||
					!ReadRequiredString(
						armorPartValue, "modelAssetId", armorPart.modelAssetId) ||
					!IsStableId(armorPart.partId) ||
					0u == armorPart.stateMask ||
					0u != (armorPart.stateMask & (armorPart.stateMask - 1u)) ||
					!IsResourceId(armorPart.modelAssetId) ||
					!armorPartIds.insert(armorPart.partId).second ||
					!armorStateMasks.insert(armorPart.stateMask).second)
				{
					return false;
				}
				entry.armorParts.push_back(std::move(armorPart));
			}
			std::set<std::string> combatObjectArchetypeIds;
			std::set<std::string> clientVisualIds;
			for (const DATA_JSON_VALUE& visual :
				pCombatObjectVisuals->Get_Array())
			{
				BOSS_COMBAT_OBJECT_VISUAL_ENTRY entryVisual;
				const DATA_JSON_VALUE* pEffectAssetId =
					visual.Is_Object() ? visual.Find("effectAssetId") : nullptr;
				const DATA_JSON_VALUE* pEffectV2Group =
					visual.Is_Object() ? visual.Find("effectV2Group") : nullptr;
				const DATA_JSON_VALUE* pHitEffectAssetId =
					visual.Is_Object() ? visual.Find("hitEffectAssetId") : nullptr;
				const size_t expectedVisualFields = 3u +
					(nullptr != pEffectV2Group ? 1u : 0u) +
					(visual.Is_Object() && nullptr != visual.Find("worldScale") ? 1u : 0u) +
					(nullptr != pHitEffectAssetId ? 1u : 0u);
				if (!visual.Is_Object() ||
					expectedVisualFields != visual.Get_Object().size() ||
					!ReadRequiredString(
						visual, "combatObjectArchetypeId",
						entryVisual.combatObjectArchetypeId) ||
					!ReadRequiredString(
						visual, "clientVisualId", entryVisual.clientVisualId) ||
					!ReadRequiredString(
						visual, "effectAssetId", entryVisual.effectAssetId) ||
					(nullptr != pHitEffectAssetId &&
						(!pHitEffectAssetId->Is_String() ||
						 pHitEffectAssetId->Get_String().empty())) ||
					!IsStableId(entryVisual.combatObjectArchetypeId) ||
					!IsStableId(entryVisual.clientVisualId) ||
					!IsStableId(entryVisual.effectAssetId) ||
					(nullptr != pHitEffectAssetId &&
						!IsStableId(pHitEffectAssetId->Get_String())) ||
					!ReadCombatVisualWorldScale(visual, entryVisual.worldScale) ||
					!combatObjectArchetypeIds.insert(
						entryVisual.combatObjectArchetypeId).second ||
					!clientVisualIds.insert(entryVisual.clientVisualId).second)
				{
					return false;
				}
				entryVisual.activeEffectKind =
					BOSS_COMBAT_OBJECT_ACTIVE_EFFECT_KIND::EFFECT_V1;
				if (nullptr != pEffectV2Group)
				{
					const DATA_JSON_VALUE* pGroupId = pEffectV2Group->Is_Object() ?
						pEffectV2Group->Find("groupId") : nullptr;
					const DATA_JSON_VALUE* pPlaybackRate = pEffectV2Group->Is_Object() ?
						pEffectV2Group->Find("playbackRate") : nullptr;
					const DATA_JSON_VALUE* pVisualHitMs = pEffectV2Group->Is_Object() ?
						pEffectV2Group->Find("visualHitMs") : nullptr;
					const DATA_JSON_VALUE* pServerHitId = pEffectV2Group->Is_Object() ?
						pEffectV2Group->Find("serverHitId") : nullptr;
					if (!pEffectV2Group->Is_Object() ||
						4u != pEffectV2Group->Get_Object().size() ||
						nullptr == pGroupId || !pGroupId->Is_String() ||
						!IsStableId(pGroupId->Get_String()) ||
						nullptr == pPlaybackRate || !pPlaybackRate->Is_Number() ||
						!std::isfinite(pPlaybackRate->Get_Number()) ||
						pPlaybackRate->Get_Number() <= 0.0 ||
						pPlaybackRate->Get_Number() > 16.0 ||
						nullptr == pVisualHitMs || !pVisualHitMs->Is_Number() ||
						pVisualHitMs->Get_Number() < 1.0 ||
						pVisualHitMs->Get_Number() > 60000.0 ||
						std::floor(pVisualHitMs->Get_Number()) !=
							pVisualHitMs->Get_Number() ||
						nullptr == pServerHitId || !pServerHitId->Is_String() ||
						!IsStableId(pServerHitId->Get_String()))
					{
						return false;
					}
					entryVisual.activeEffectKind =
						BOSS_COMBAT_OBJECT_ACTIVE_EFFECT_KIND::EFFECT_V2_GROUP;
					entryVisual.effectV2Group.groupId = pGroupId->Get_String();
					entryVisual.effectV2Group.playbackRate =
						static_cast<f32_t>(pPlaybackRate->Get_Number());
					entryVisual.effectV2Group.visualHitMs =
						static_cast<uint32_t>(pVisualHitMs->Get_Number());
					entryVisual.effectV2Group.serverHitId = pServerHitId->Get_String();
				}
				if (nullptr != pHitEffectAssetId)
					entryVisual.hitEffectAssetId =
						pHitEffectAssetId->Get_String();
				entry.combatObjectVisuals.push_back(std::move(entryVisual));
			}
			staged.push_back(std::move(entry));
		}
		g_Bosses = std::move(staged);
		return !g_Bosses.empty();
	}

	bool_t ParseNpcs(const DATA_JSON_VALUE& root)
	{
		const DATA_JSON_VALUE* pSchema = root.Find("schema");
		const DATA_JSON_VALUE* pVersion = root.Find("formatVersion");
		const DATA_JSON_VALUE* pEntries = root.Find("npcs");
		if (nullptr == pSchema || !pSchema->Is_String() ||
			pSchema->Get_String() != "lostark.npc-catalog" ||
			nullptr == pVersion || !pVersion->Is_Number() ||
			pVersion->Get_Number() != 2.0 ||
			nullptr == pEntries || !pEntries->Is_Array())
		{
			return false;
		}

		std::set<std::string> archetypes;
		std::set<std::string> presentations;
		std::vector<NPC_ACTOR_ENTRY> staged;
		for (const DATA_JSON_VALUE& value : pEntries->Get_Array())
		{
			const DATA_JSON_VALUE* pActionClips =
				value.Is_Object() ? value.Find("actionClips") : nullptr;
			const DATA_JSON_VALUE* pCutinWindow =
				value.Is_Object() ? value.Find("cutinWindow") : nullptr;
			const DATA_JSON_VALUE* pShaderProfile =
				value.Is_Object() ? value.Find("shaderProfile") : nullptr;
			size_t expectedFields = 6u;
			if (nullptr != pActionClips)
				++expectedFields;
			if (nullptr != pCutinWindow)
				++expectedFields;
			if (nullptr != pShaderProfile)
				++expectedFields;
			if (!value.Is_Object() ||
				expectedFields != value.Get_Object().size())
			{
				return false;
			}
			NPC_ACTOR_ENTRY entry;
			const DATA_JSON_VALUE* pAnimSet = value.Find("animationSetId");
			if (!ReadRequiredString(value, "archetypeId", entry.archetypeId) ||
				!ReadRequiredString(value, "clientPresentationId",
					entry.clientPresentationId) ||
				!ReadRequiredString(value, "modelAssetId", entry.modelAssetId) ||
				!ReadRequiredString(value, "idleClip", entry.idleClip) ||
				!ReadRequiredString(value, "runtimeStatus", entry.runtimeStatus) ||
				!IsResourceId(entry.modelAssetId) ||
				nullptr == pAnimSet ||
				(!pAnimSet->Is_Null() && !pAnimSet->Is_String()) ||
				entry.runtimeStatus != "supported" ||
				!archetypes.insert(entry.archetypeId).second ||
				!presentations.insert(entry.clientPresentationId).second)
			{
				return false;
			}
			if (pAnimSet->Is_String())
			{
				entry.animationSetId = pAnimSet->Get_String();
				if (!IsResourceId(entry.animationSetId))
					return false;
			}
			if (nullptr != pActionClips)
			{
				if (!pActionClips->Is_Object() ||
					pActionClips->Get_Object().empty())
				{
					return false;
				}
				for (const auto& [actionId, clip] :
					pActionClips->Get_Object())
				{
					std::vector<std::string> chain;
					if (clip.Is_String())
					{
						chain.push_back(clip.Get_String());
					}
					else if (clip.Is_Array())
					{
						for (const DATA_JSON_VALUE& element :
							clip.Get_Array())
						{
							if (!element.Is_String() ||
								element.Get_String().empty())
							{
								return false;
							}
							chain.push_back(element.Get_String());
						}
					}
					if (actionId.empty() || chain.empty() ||
						chain.front().empty() ||
						!entry.actionClips.emplace(
							actionId, std::move(chain)).second)
					{
						return false;
					}
				}
			}
			if (nullptr != pCutinWindow)
			{
				const DATA_JSON_VALUE* pStart =
					pCutinWindow->Is_Object() ?
					pCutinWindow->Find("startMs") : nullptr;
				const DATA_JSON_VALUE* pEnd =
					pCutinWindow->Is_Object() ?
					pCutinWindow->Find("endMs") : nullptr;
				if (nullptr == pActionClips ||
					!pCutinWindow->Is_Object() ||
					nullptr == pStart || !pStart->Is_Number() ||
					pStart->Get_Number() < 0.0 ||
					(nullptr != pEnd &&
						(!pEnd->Is_Number() || pEnd->Get_Number() < 0.0)))
				{
					return false;
				}
				const size_t windowFields = nullptr != pEnd ? 2u : 1u;
				if (windowFields != pCutinWindow->Get_Object().size())
					return false;
				entry.cutinStartMs =
					static_cast<std::uint32_t>(pStart->Get_Number());
				entry.cutinEndMs = nullptr != pEnd ?
					static_cast<std::uint32_t>(pEnd->Get_Number()) : 0u;
				if (0u != entry.cutinEndMs &&
					entry.cutinEndMs <= entry.cutinStartMs)
				{
					return false;
				}
			}
			if (nullptr != pShaderProfile)
			{
				if (!pShaderProfile->Is_String() ||
					pShaderProfile->Get_String() != "esther")
				{
					return false;
				}
				entry.shaderProfile = pShaderProfile->Get_String();
			}
			staged.push_back(std::move(entry));
		}
		g_Npcs = std::move(staged);
		return !g_Npcs.empty();
	}

	bool_t ParseMonsters(const DATA_JSON_VALUE& root)
	{
		const DATA_JSON_VALUE* pSchema = root.Find("schema");
		const DATA_JSON_VALUE* pVersion = root.Find("formatVersion");
		const DATA_JSON_VALUE* pEntries = root.Find("monsters");
		if (nullptr == pSchema || !pSchema->Is_String() ||
			pSchema->Get_String() != "lostark.monster-catalog" ||
			nullptr == pVersion || !pVersion->Is_Number() ||
			pVersion->Get_Number() != 2.0 ||
			nullptr == pEntries || !pEntries->Is_Array())
		{
			return false;
		}

		std::set<std::string> archetypes;
		std::set<std::string> presentations;
		std::vector<MONSTER_ACTOR_ENTRY> staged;
		for (const DATA_JSON_VALUE& value : pEntries->Get_Array())
		{
			if (!value.Is_Object() || 9u != value.Get_Object().size())
				return false;
			MONSTER_ACTOR_ENTRY entry;
			const DATA_JSON_VALUE* pClips = value.Find("presentationClips");
			const DATA_JSON_VALUE* pAttacks = value.Find("attackPresentations");
			if (!ReadRequiredString(value, "archetypeId", entry.archetypeId) ||
				!ReadRequiredString(value, "clientPresentationId",
					entry.clientPresentationId) ||
				!ReadRequiredString(value, "modelAssetId", entry.modelAssetId) ||
				!ReadRequiredNumber(value, "modelScale", entry.modelScale) ||
				!ReadRequiredNumber(value, "modelYawDegrees",
					entry.modelYawDegrees) ||
				!ReadRequiredNumber(value, "hitDurationSeconds",
					entry.hitDurationSeconds) ||
				nullptr == pAttacks || !pAttacks->Is_Array() ||
				pAttacks->Get_Array().empty() ||
				pAttacks->Get_Array().size() > 8u ||
				nullptr == pClips || !pClips->Is_Object() ||
				4u != pClips->Get_Object().size() ||
				!ReadRequiredString(*pClips, "idle",
					entry.presentationClips.idle) ||
				!ReadRequiredString(*pClips, "chase",
					entry.presentationClips.chase) ||
				!ReadRequiredString(*pClips, "hit",
					entry.presentationClips.hit) ||
				!ReadRequiredString(*pClips, "dead",
					entry.presentationClips.dead) ||
				!ReadRequiredString(value, "runtimeStatus", entry.runtimeStatus) ||
				!IsResourceId(entry.modelAssetId) ||
				entry.modelScale <= 0.f || entry.modelScale > 100.f ||
				std::abs(entry.modelYawDegrees) > 360.f ||
				entry.hitDurationSeconds < 0.05f ||
				entry.hitDurationSeconds > 2.f ||
				entry.runtimeStatus != "supported" ||
				!archetypes.insert(entry.archetypeId).second ||
				!presentations.insert(entry.clientPresentationId).second)
			{
				return false;
			}
			std::set<std::string> attackClips;
			for (const DATA_JSON_VALUE& attack : pAttacks->Get_Array())
			{
				MONSTER_ACTOR_ENTRY::ATTACK_PRESENTATION presentation;
				if (!attack.Is_Object() || 2u != attack.Get_Object().size() ||
					!ReadRequiredString(attack, "clip", presentation.clip) ||
					!ReadRequiredNumber(
						attack, "playbackRate", presentation.playbackRate) ||
					presentation.playbackRate < 0.1f ||
					presentation.playbackRate > 4.f ||
					!attackClips.insert(presentation.clip).second)
				{
					return false;
				}
				entry.attackPresentations.push_back(std::move(presentation));
			}
			staged.push_back(std::move(entry));
		}
		g_Monsters = std::move(staged);
		return !g_Monsters.empty();
	}
}

bool_t Client::CActorCatalog::Initialize()
{
	if (g_isInitialized)
		return true;
	DATA_JSON_VALUE characters;
	DATA_JSON_VALUE bosses;
	DATA_JSON_VALUE npcs;
	DATA_JSON_VALUE monsters;
	if (!ReadDocument(L"Actors/CharacterCatalog.json", characters) ||
		!ReadDocument(L"Actors/BossCatalog.json", bosses) ||
		!ReadDocument(L"Actors/NpcCatalog.json", npcs) ||
		!ReadDocument(L"Actors/MonsterCatalog.json", monsters) ||
		!ParseCharacters(characters) || !ParseBosses(bosses) ||
		!ParseNpcs(npcs) || !ParseMonsters(monsters))
	{
		g_Characters.clear();
		g_Bosses.clear();
		g_Npcs.clear();
		g_Monsters.clear();
		g_Status = "Actor catalog contract mismatch.";
		return false;
	}
	g_isInitialized = true;
	g_Status = "Actor catalogs ready.";
	return true;
}

const Client::NPC_ACTOR_ENTRY* Client::CActorCatalog::Find_Npc(
	const std::string_view archetypeId)
{
	if (!Initialize())
		return nullptr;
	for (const NPC_ACTOR_ENTRY& entry : g_Npcs)
		if (entry.archetypeId == archetypeId)
			return &entry;
	return nullptr;
}

const std::vector<Client::NPC_ACTOR_ENTRY>& Client::CActorCatalog::Get_Npcs()
{
	Initialize();
	return g_Npcs;
}

const Client::MONSTER_ACTOR_ENTRY* Client::CActorCatalog::Find_Monster(
	const std::string_view archetypeId)
{
	if (!Initialize())
		return nullptr;
	for (const MONSTER_ACTOR_ENTRY& entry : g_Monsters)
		if (entry.archetypeId == archetypeId)
			return &entry;
	return nullptr;
}

const std::vector<Client::MONSTER_ACTOR_ENTRY>&
Client::CActorCatalog::Get_Monsters()
{
	Initialize();
	return g_Monsters;
}

const Client::CHARACTER_ACTOR_ENTRY* Client::CActorCatalog::Find_Character(
	const LostArk::Shared::CHARACTER_CLASS_ID networkClassId)
{
	if (!Initialize())
		return nullptr;
	for (const CHARACTER_ACTOR_ENTRY& entry : g_Characters)
		if (entry.networkClassId == networkClassId)
			return &entry;
	return nullptr;
}

const Client::BOSS_ACTOR_ENTRY* Client::CActorCatalog::Find_Boss(
	const std::string_view archetypeId)
{
	if (!Initialize())
		return nullptr;
	for (const BOSS_ACTOR_ENTRY& entry : g_Bosses)
		if (entry.archetypeId == archetypeId)
			return &entry;
	return nullptr;
}

const Client::BOSS_COMBAT_OBJECT_VISUAL_ENTRY*
Client::CActorCatalog::Find_BossCombatObjectVisual(
	const std::string_view bossArchetypeId,
	const std::string_view combatObjectArchetypeId,
	const std::string_view clientVisualId)
{
	const BOSS_ACTOR_ENTRY* boss = Find_Boss(bossArchetypeId);
	if (nullptr == boss)
		return nullptr;
	for (const BOSS_COMBAT_OBJECT_VISUAL_ENTRY& visual :
		boss->combatObjectVisuals)
	{
		if (visual.combatObjectArchetypeId == combatObjectArchetypeId &&
			visual.clientVisualId == clientVisualId)
		{
			return &visual;
		}
	}
	return nullptr;
}

const std::string& Client::CActorCatalog::Get_Status()
{
	return g_Status;
}
