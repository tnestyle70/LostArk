#include "SoundCueCatalog.h"

#include "DataJson.h"
#include "ProjectDataRoot.h"

#include <algorithm>
#include <fstream>

std::unordered_map<std::string, Client::CSoundCueCatalog::EVENT_VARIANTS>
	Client::CSoundCueCatalog::s_ClassEvents;
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
}

bool_t Client::CSoundCueCatalog::Load(std::string& strOutStatus)
{
	CLASS_EVENTS Staged;
	if (!Parse_Catalog(Staged, strOutStatus))
		return false;
	s_ClassEvents = std::move(Staged);
	s_bLoaded = true;
	strOutStatus = "Loaded CharacterSoundCatalog.json transactionally.";
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
