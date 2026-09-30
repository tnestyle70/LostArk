#include "SoundCueCatalog.h"

#include "DataJson.h"
#include "ProjectDataRoot.h"

#include <algorithm>
#include <fstream>

std::unordered_map<std::string, Client::CSoundCueCatalog::EVENT_VARIANTS>
	Client::CSoundCueCatalog::s_ClassEvents;
std::unordered_map<std::string,
	std::unordered_map<uint8_t, Client::CSoundCueCatalog::EVENT_VARIANTS>>
	Client::CSoundCueCatalog::s_ClassVoiceEvents;
std::unordered_map<std::string, uint8_t>
	Client::CSoundCueCatalog::s_ClassDefaultVoiceType;
bool_t Client::CSoundCueCatalog::s_bLoaded = false;

namespace
{
	const std::vector<std::string> g_EmptyVariants;
	using CLASS_EVENTS = std::unordered_map<std::string,
		Client::CSoundCueCatalog::EVENT_VARIANTS>;

	bool_t Parse_Catalog(
		CLASS_EVENTS& OutClasses,
		std::string& strOutStatus)
	{
		const std::filesystem::path Path =
			CProjectDataRoot::Resolve("Sound/CharacterSoundCatalog.json");
		std::ifstream Input(Path, std::ios::binary);
		if (Path.empty() || !Input)
		{
			strOutStatus = "CharacterSoundCatalog.json not found.";
			return false;
		}
		const std::string Text{
			std::istreambuf_iterator<char>(Input),
			std::istreambuf_iterator<char>() };

		DATA_JSON_VALUE Root;
		std::string ParseError;
		if (!CDataJson::Parse(Text, Root, ParseError) || !Root.Is_Object())
		{
			strOutStatus = ParseError.empty() ?
				"CharacterSoundCatalog.json is not a valid JSON object." :
				ParseError;
			return false;
		}
		const DATA_JSON_VALUE* pClasses = Root.Find("classes");
		if (nullptr == pClasses || !pClasses->Is_Object())
		{
			strOutStatus = "CharacterSoundCatalog.json is missing \"classes\".";
			return false;
		}

		CLASS_EVENTS Staged;
		for (const auto& [strClassName, ClassValue] : pClasses->Get_Object())
		{
			if (!ClassValue.Is_Object())
			{
				strOutStatus =
					"CharacterSoundCatalog.json class row is not an object: " +
					strClassName;
				return false;
			}
			Client::CSoundCueCatalog::EVENT_VARIANTS Events;
			for (const auto& [strEventName, EventValue] :
				ClassValue.Get_Object())
			{
				if (!EventValue.Is_Array())
				{
					strOutStatus =
						"CharacterSoundCatalog.json event row is not an array: " +
						strClassName + "/" + strEventName;
					return false;
				}
				std::vector<std::string> Variants;
				Variants.reserve(EventValue.Get_Array().size());
				for (const DATA_JSON_VALUE& Entry : EventValue.Get_Array())
				{
					if (!Entry.Is_String() || Entry.Get_String().empty())
					{
						strOutStatus =
							"CharacterSoundCatalog.json contains an invalid asset ID: " +
							strClassName + "/" + strEventName;
						return false;
					}
					Variants.push_back(Entry.Get_String());
				}
				/* An empty array is the authored unresolved state produced when an
				   animation event has no extracted WAV yet. Keep it addressable so
				   unrelated classes do not fail a class snapshot; the typed consumer
				   that requires playback assets rejects an empty referenced event. */
				if (!Events.emplace(strEventName, std::move(Variants)).second)
				{
					strOutStatus =
						"CharacterSoundCatalog.json contains a duplicate event: " +
						strClassName + "/" + strEventName;
					return false;
				}
			}
			if (!Staged.emplace(strClassName, std::move(Events)).second)
			{
				strOutStatus =
					"CharacterSoundCatalog.json contains a duplicate class: " +
					strClassName;
				return false;
			}
		}
		if (Staged.empty())
		{
			strOutStatus = "CharacterSoundCatalog.json has no class rows.";
			return false;
		}
		OutClasses = std::move(Staged);
		return true;
	}

	/* class -> voice type -> event -> the catalog variants recorded for that
	   type. Every listed asset must already be a catalog variant of the same
	   event, so the voice document can only narrow. */
	using CLASS_VOICE_EVENTS = std::unordered_map<std::string,
		std::unordered_map<uint8_t, Client::CSoundCueCatalog::EVENT_VARIANTS>>;

	bool_t Parse_VoiceTypeName(const std::string& strName, uint8_t& iOutType)
	{
		if (strName.size() != 5u || 0 != strName.compare(0u, 4u, "Type") ||
			strName[4] < '1' || strName[4] > '8')
		{
			return false;
		}
		iOutType = static_cast<uint8_t>(strName[4] - '0');
		return true;
	}

	bool_t Parse_VoiceTypes(
		const CLASS_EVENTS& Catalog,
		CLASS_VOICE_EVENTS& OutClasses,
		std::unordered_map<std::string, uint8_t>& OutDefaults,
		std::string& strOutStatus)
	{
		const std::filesystem::path Path =
			CProjectDataRoot::Resolve("Sound/CharacterVoiceTypes.json");
		std::ifstream Input(Path, std::ios::binary);
		if (Path.empty() || !Input)
		{
			strOutStatus = "CharacterVoiceTypes.json not found.";
			return false;
		}
		const std::string Text{
			std::istreambuf_iterator<char>(Input),
			std::istreambuf_iterator<char>() };

		DATA_JSON_VALUE Root;
		std::string ParseError;
		if (!CDataJson::Parse(Text, Root, ParseError) || !Root.Is_Object())
		{
			strOutStatus = ParseError.empty() ?
				"CharacterVoiceTypes.json is not a valid JSON object." :
				ParseError;
			return false;
		}
		const DATA_JSON_VALUE* pVersion = Root.Find("formatVersion");
		if (nullptr == pVersion || !pVersion->Is_Number() ||
			1 != static_cast<int32_t>(pVersion->Get_Number()))
		{
			strOutStatus = "CharacterVoiceTypes.json formatVersion is not 1.";
			return false;
		}
		const DATA_JSON_VALUE* pClasses = Root.Find("classes");
		if (nullptr == pClasses || !pClasses->Is_Object())
		{
			strOutStatus = "CharacterVoiceTypes.json is missing \"classes\".";
			return false;
		}

		CLASS_VOICE_EVENTS Staged;
		std::unordered_map<std::string, uint8_t> StagedDefaults;
		for (const auto& [strClassName, ClassValue] : pClasses->Get_Object())
		{
			const DATA_JSON_VALUE* pSelected =
				ClassValue.Is_Object() ? ClassValue.Find("selectedVoiceType") : nullptr;
			const DATA_JSON_VALUE* pEvents =
				ClassValue.Is_Object() ? ClassValue.Find("events") : nullptr;
			uint8_t iDefaultType = 0u;
			if (nullptr == pSelected || !pSelected->Is_String() ||
				!Parse_VoiceTypeName(pSelected->Get_String(), iDefaultType) ||
				nullptr == pEvents || !pEvents->Is_Object())
			{
				strOutStatus =
					"CharacterVoiceTypes.json class row needs selectedVoiceType Type1..8 and events: " +
					strClassName;
				return false;
			}
			const auto CatalogClass = Catalog.find(strClassName);
			if (Catalog.end() == CatalogClass)
			{
				strOutStatus =
					"CharacterVoiceTypes.json names a class absent from the catalog: " +
					strClassName;
				return false;
			}
			std::unordered_map<uint8_t, Client::CSoundCueCatalog::EVENT_VARIANTS> Types;
			for (const auto& [strEventName, TypesValue] : pEvents->Get_Object())
			{
				const auto CatalogEvent = CatalogClass->second.find(strEventName);
				if (CatalogClass->second.end() == CatalogEvent ||
					!TypesValue.Is_Object() || TypesValue.Get_Object().empty())
				{
					strOutStatus =
						"CharacterVoiceTypes.json event is not a catalog event with voice types: " +
						strClassName + "/" + strEventName;
					return false;
				}
				const std::vector<std::string>& Known = CatalogEvent->second;
				for (const auto& [strTypeName, AssetsValue] : TypesValue.Get_Object())
				{
					uint8_t iType = 0u;
					if (!Parse_VoiceTypeName(strTypeName, iType) ||
						!AssetsValue.Is_Array() || AssetsValue.Get_Array().empty())
					{
						strOutStatus =
							"CharacterVoiceTypes.json voice type row must be Type1..8 with assets: " +
							strClassName + "/" + strEventName + "/" + strTypeName;
						return false;
					}
					std::vector<std::string> Variants;
					Variants.reserve(AssetsValue.Get_Array().size());
					for (const DATA_JSON_VALUE& Entry : AssetsValue.Get_Array())
					{
						if (!Entry.Is_String() ||
							Known.end() == std::find(Known.begin(), Known.end(), Entry.Get_String()))
						{
							strOutStatus =
								"CharacterVoiceTypes.json lists an asset the catalog does not: " +
								strClassName + "/" + strEventName;
							return false;
						}
						Variants.push_back(Entry.Get_String());
					}
					if (!Types[iType].emplace(strEventName, std::move(Variants)).second)
					{
						strOutStatus =
							"CharacterVoiceTypes.json contains a duplicate event: " +
							strClassName + "/" + strEventName;
						return false;
					}
				}
			}
			if (Types.end() == Types.find(iDefaultType))
			{
				strOutStatus =
					"CharacterVoiceTypes.json selectedVoiceType has no events: " +
					strClassName;
				return false;
			}
			if (!Staged.emplace(strClassName, std::move(Types)).second)
			{
				strOutStatus =
					"CharacterVoiceTypes.json contains a duplicate class: " +
					strClassName;
				return false;
			}
			StagedDefaults[strClassName] = iDefaultType;
		}
		OutClasses = std::move(Staged);
		OutDefaults = std::move(StagedDefaults);
		return true;
	}
}

bool_t Client::CSoundCueCatalog::Load(std::string& strOutStatus)
{
	CLASS_EVENTS Staged;
	if (!Parse_Catalog(Staged, strOutStatus))
		return false;
	CLASS_VOICE_EVENTS StagedVoice;
	std::unordered_map<std::string, uint8_t> StagedDefaults;
	std::string strVoiceStatus;
	const bool_t bVoice = Parse_VoiceTypes(
		Staged, StagedVoice, StagedDefaults, strVoiceStatus);
	s_ClassEvents = std::move(Staged);
	s_ClassVoiceEvents = bVoice ? std::move(StagedVoice) : CLASS_VOICE_EVENTS{};
	s_ClassDefaultVoiceType = bVoice ?
		std::move(StagedDefaults) : std::unordered_map<std::string, uint8_t>{};
	s_bLoaded = true;
	strOutStatus = bVoice ?
		"Loaded CharacterSoundCatalog.json and CharacterVoiceTypes.json transactionally." :
		"Loaded CharacterSoundCatalog.json; voice types unavailable: " + strVoiceStatus;
	return true;
}

bool_t Client::CSoundCueCatalog::Load_ClassSnapshot(
	const std::string& strClassName,
	EVENT_VARIANTS& InOutEvents,
	std::string& strOutStatus)
{
	CLASS_EVENTS Staged;
	if (!Parse_Catalog(Staged, strOutStatus))
		return false;
	const auto Found = Staged.find(strClassName);
	if (Staged.end() == Found || Found->second.empty())
	{
		strOutStatus =
			"CharacterSoundCatalog.json has no requested class: " +
			strClassName;
		return false;
	}
	InOutEvents = Found->second;
	strOutStatus = "Loaded an immutable Character Sound class snapshot.";
	return true;
}

const std::vector<std::string>& Client::CSoundCueCatalog::Find_Variants(
	const std::string& strClassName,
	const std::string& strEventName)
{
	if (!s_bLoaded)
		return g_EmptyVariants;

	const auto ClassIterator = s_ClassEvents.find(strClassName);
	if (s_ClassEvents.end() == ClassIterator)
		return g_EmptyVariants;

	const auto EventIterator = ClassIterator->second.find(strEventName);
	if (ClassIterator->second.end() == EventIterator)
		return g_EmptyVariants;

	return EventIterator->second;
}

const std::vector<std::string>& Client::CSoundCueCatalog::Find_VoiceVariants(
	const std::string& strClassName,
	const std::string& strEventName,
	const uint8_t iVoiceType)
{
	if (!s_bLoaded)
		return g_EmptyVariants;

	const auto ClassIterator = s_ClassVoiceEvents.find(strClassName);
	if (s_ClassVoiceEvents.end() == ClassIterator)
		return g_EmptyVariants;

	uint8_t iType = iVoiceType;
	if (0u == iType)
	{
		const auto DefaultIterator = s_ClassDefaultVoiceType.find(strClassName);
		if (s_ClassDefaultVoiceType.end() == DefaultIterator)
			return g_EmptyVariants;
		iType = DefaultIterator->second;
	}
	const auto TypeIterator = ClassIterator->second.find(iType);
	if (ClassIterator->second.end() == TypeIterator)
		return g_EmptyVariants;

	const auto EventIterator = TypeIterator->second.find(strEventName);
	if (TypeIterator->second.end() == EventIterator)
		return g_EmptyVariants;

	return EventIterator->second;
}

std::vector<uint8_t> Client::CSoundCueCatalog::Collect_VoiceTypes(
	const std::string& strClassName)
{
	std::vector<uint8_t> types;
	if (!s_bLoaded)
		return types;
	const auto classIterator = s_ClassVoiceEvents.find(strClassName);
	if (s_ClassVoiceEvents.end() == classIterator)
		return types;
	types.reserve(classIterator->second.size());
	for (const auto& [voiceType, events] : classIterator->second)
		types.push_back(voiceType);
	std::sort(types.begin(), types.end());
	return types;
}

std::vector<std::string> Client::CSoundCueCatalog::Collect_VoiceEventNames(
	const std::string& strClassName)
{
	std::vector<std::string> names;
	if (!s_bLoaded)
		return names;
	const auto classIterator = s_ClassVoiceEvents.find(strClassName);
	if (s_ClassVoiceEvents.end() == classIterator)
		return names;
	for (const auto& [voiceType, events] : classIterator->second)
	{
		for (const auto& [eventName, variants] : events)
		{
			if (names.end() == std::find(names.begin(), names.end(), eventName))
				names.push_back(eventName);
		}
	}
	std::sort(names.begin(), names.end());
	return names;
}

std::vector<std::string> Client::CSoundCueCatalog::Collect_EventNames(
	const std::string& strClassName)
{
	std::vector<std::string> names;
	if (!s_bLoaded)
		return names;
	const auto classIterator = s_ClassEvents.find(strClassName);
	if (s_ClassEvents.end() == classIterator)
		return names;
	names.reserve(classIterator->second.size());
	for (const auto& [eventName, variants] : classIterator->second)
	{
		if (!variants.empty())
			names.push_back(eventName);
	}
	std::sort(names.begin(), names.end());
	return names;
}
