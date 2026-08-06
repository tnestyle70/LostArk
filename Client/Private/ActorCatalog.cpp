#include "ActorCatalog.h"

#include "DataJson.h"
#include "ProjectDataRoot.h"

#include <fstream>
#include <set>

namespace
{
	using namespace Client;

	std::vector<CHARACTER_ACTOR_ENTRY> g_Characters;
	std::vector<BOSS_ACTOR_ENTRY> g_Bosses;
	std::vector<NPC_ACTOR_ENTRY> g_Npcs;
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
			pVersion->Get_Number() != 2.0 ||
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
			if (LostArk::Shared::CHARACTER_CLASS_ID::END == entry.networkClassId ||
				!IsResourceId(entry.bodyModel) ||
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
			pVersion->Get_Number() != 1.0 ||
			nullptr == pEntries || !pEntries->Is_Array())
		{
			return false;
		}

		std::set<std::string> archetypes;
		std::vector<BOSS_ACTOR_ENTRY> staged;
		for (const DATA_JSON_VALUE& value : pEntries->Get_Array())
		{
			if (!value.Is_Object())
				return false;
			BOSS_ACTOR_ENTRY entry;
			const DATA_JSON_VALUE* pClips = value.Find("presentationClips");
			if (!ReadRequiredString(value, "archetypeId", entry.archetypeId) ||
				!ReadRequiredString(value, "visualAssetId", entry.visualAssetId) ||
				!ReadRequiredString(value, "bodyModel", entry.bodyModel) ||
				!ReadRequiredString(value, "weaponModel", entry.weaponModel) ||
				!ReadRequiredString(value, "animationSetId", entry.animationSetId) ||
				!ReadRequiredString(value, "serverProfileId", entry.serverProfileId) ||
				!ReadRequiredString(value, "clientPresentationId", entry.clientPresentationId) ||
				!ReadRequiredString(value, "presentationStatus", entry.presentationStatus) ||
				nullptr == pClips || !pClips->Is_Object() ||
				!ReadRequiredString(*pClips, "idle", entry.presentationClips.idle) ||
				!ReadRequiredString(*pClips, "chase", entry.presentationClips.chase) ||
				!ReadRequiredString(*pClips, "patternWindup", entry.presentationClips.patternWindup) ||
				!ReadRequiredString(*pClips, "patternActive", entry.presentationClips.patternActive) ||
				!ReadRequiredString(*pClips, "patternRecovery", entry.presentationClips.patternRecovery) ||
				!ReadRequiredString(*pClips, "dead", entry.presentationClips.dead) ||
				!IsResourceId(entry.bodyModel) ||
				!IsResourceId(entry.weaponModel) ||
				!archetypes.insert(entry.archetypeId).second ||
				(entry.presentationStatus != "complete" &&
				 entry.presentationStatus != "fallback"))
			{
				return false;
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
			pVersion->Get_Number() != 1.0 ||
			nullptr == pEntries || !pEntries->Is_Array())
		{
			return false;
		}

		std::set<std::string> archetypes;
		std::set<std::string> presentations;
		std::vector<NPC_ACTOR_ENTRY> staged;
		for (const DATA_JSON_VALUE& value : pEntries->Get_Array())
		{
			if (!value.Is_Object() || 5u != value.Get_Object().size())
				return false;
			NPC_ACTOR_ENTRY entry;
			if (!ReadRequiredString(value, "archetypeId", entry.archetypeId) ||
				!ReadRequiredString(value, "clientPresentationId",
					entry.clientPresentationId) ||
				!ReadRequiredString(value, "modelAssetId", entry.modelAssetId) ||
				!ReadRequiredString(value, "idleClip", entry.idleClip) ||
				!ReadRequiredString(value, "runtimeStatus", entry.runtimeStatus) ||
				!IsResourceId(entry.modelAssetId) ||
				entry.runtimeStatus != "supported" ||
				!archetypes.insert(entry.archetypeId).second ||
				!presentations.insert(entry.clientPresentationId).second)
			{
				return false;
			}
			staged.push_back(std::move(entry));
		}
		g_Npcs = std::move(staged);
		return !g_Npcs.empty();
	}
}

bool_t Client::CActorCatalog::Initialize()
{
	if (g_isInitialized)
		return true;
	DATA_JSON_VALUE characters;
	DATA_JSON_VALUE bosses;
	DATA_JSON_VALUE npcs;
	if (!ReadDocument(L"Actors/CharacterCatalog.json", characters) ||
		!ReadDocument(L"Actors/BossCatalog.json", bosses) ||
		!ReadDocument(L"Actors/NpcCatalog.json", npcs) ||
		!ParseCharacters(characters) || !ParseBosses(bosses) ||
		!ParseNpcs(npcs))
	{
		g_Characters.clear();
		g_Bosses.clear();
		g_Npcs.clear();
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

const std::string& Client::CActorCatalog::Get_Status()
{
	return g_Status;
}
