#include "SoundCueCatalog.h"

#include "DataJson.h"
#include "ProjectDataRoot.h"

#include <fstream>

std::unordered_map<std::string,
	std::unordered_map<std::string, std::vector<std::string>>>
	Client::CSoundCueCatalog::s_ClassEvents;
bool_t Client::CSoundCueCatalog::s_bLoaded = false;

namespace
{
	const std::vector<std::string> g_EmptyVariants;
}

bool_t Client::CSoundCueCatalog::Load(std::string& strOutStatus)
{
	s_ClassEvents.clear();
	s_bLoaded = false;

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
			"CharacterSoundCatalog.json is not a valid JSON object." : ParseError;
		return false;
	}

	const DATA_JSON_VALUE* pClasses = Root.Find("classes");
	if (nullptr == pClasses || !pClasses->Is_Object())
	{
		strOutStatus = "CharacterSoundCatalog.json is missing \"classes\".";
		return false;
	}

	for (const auto& [strClassName, ClassValue] : pClasses->Get_Object())
	{
		if (!ClassValue.Is_Object())
			continue;

		std::unordered_map<std::string, std::vector<std::string>> Events;
		for (const auto& [strEventName, EventValue] : ClassValue.Get_Object())
		{
			if (!EventValue.Is_Array())
				continue;

			std::vector<std::string> Variants;
			for (const DATA_JSON_VALUE& Entry : EventValue.Get_Array())
			{
				if (Entry.Is_String())
					Variants.push_back(Entry.Get_String());
			}
			Events.emplace(strEventName, std::move(Variants));
		}
		s_ClassEvents.emplace(strClassName, std::move(Events));
	}

	s_bLoaded = true;
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
