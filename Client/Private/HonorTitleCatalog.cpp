#include "HonorTitleCatalog.h"

#include "DataJson.h"
#include "ProjectDataRoot.h"

#include <Windows.h>

#include <fstream>

namespace
{
	std::vector<Client::HONOR_TITLE_ENTRY> g_Titles;
	Client::HONOR_TITLE_STRINGS g_Strings;

	bool_t ConvertUtf8ToWide(const std::string& strUtf8, wstring& outWide)
	{
		outWide.clear();
		if (strUtf8.empty())
			return true;
		const int32_t iLength = ::MultiByteToWideChar(
			CP_UTF8, MB_ERR_INVALID_CHARS, strUtf8.c_str(), -1, nullptr, 0);
		if (iLength <= 1)
			return false;
		outWide.assign(static_cast<size_t>(iLength - 1), L'\0');
		::MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, outWide.data(), iLength);
		return true;
	}

	bool_t ReadText(const DATA_JSON_VALUE& Object, const char* pKey, wstring& outText)
	{
		const DATA_JSON_VALUE* pText = Object.Find(pKey);
		return nullptr != pText && pText->Is_String() &&
			ConvertUtf8ToWide(pText->Get_String(), outText);
	}
}

bool_t Client::CHonorTitleCatalog::Load(std::string& outStatus)
{
	const filesystem::path DataPath = CProjectDataRoot::Resolve(L"Titles/HonorTitles.json");
	ifstream Stream(DataPath, ios::binary);
	if (DataPath.empty() || !Stream.is_open())
	{
		outStatus = "Missing honor title document Data/Titles/HonorTitles.json";
		return false;
	}
	const string Text((istreambuf_iterator<char>(Stream)), istreambuf_iterator<char>());
	DATA_JSON_VALUE Root;
	string Error;
	if (!CDataJson::Parse(Text, Root, Error) || !Root.Is_Object())
	{
		outStatus = "HonorTitles.json parse failed: " + Error;
		return false;
	}
	const DATA_JSON_VALUE* pSchema = Root.Find("schema");
	const DATA_JSON_VALUE* pVersion = Root.Find("formatVersion");
	if (nullptr == pSchema || !pSchema->Is_String() || "lostark.honor-titles" != pSchema->Get_String() ||
		nullptr == pVersion || !pVersion->Is_Number() || 1.0 != pVersion->Get_Number())
	{
		outStatus = "HonorTitles.json header is not lostark.honor-titles formatVersion 1";
		return false;
	}

	/* Staged first: a document that fails halfway leaves the previous set in place. */
	HONOR_TITLE_STRINGS stagedStrings;
	const DATA_JSON_VALUE* pStrings = Root.Find("strings");
	if (nullptr == pStrings || !pStrings->Is_Object() ||
		!ReadText(*pStrings, "windowTitle", stagedStrings.strWindowTitle) ||
		!ReadText(*pStrings, "apply", stagedStrings.strApply) ||
		!ReadText(*pStrings, "remove", stagedStrings.strRemove) ||
		!ReadText(*pStrings, "current", stagedStrings.strCurrent) ||
		!ReadText(*pStrings, "using", stagedStrings.strUsing) ||
		!ReadText(*pStrings, "change", stagedStrings.strChange) ||
		!ReadText(*pStrings, "none", stagedStrings.strNone))
	{
		outStatus = "HonorTitles.json strings block is incomplete";
		return false;
	}

	const DATA_JSON_VALUE* pTitles = Root.Find("titles");
	if (nullptr == pTitles || !pTitles->Is_Array())
	{
		outStatus = "HonorTitles.json has no titles array";
		return false;
	}
	std::vector<HONOR_TITLE_ENTRY> staged;
	for (const DATA_JSON_VALUE& Value : pTitles->Get_Array())
	{
		const DATA_JSON_VALUE* pId = Value.Is_Object() ? Value.Find("titleId") : nullptr;
		HONOR_TITLE_ENTRY Entry{};
		if (nullptr == pId || !pId->Is_Number() || pId->Get_Number() < 1.0 ||
			!ReadText(Value, "name", Entry.strName) || Entry.strName.empty())
		{
			outStatus = "HonorTitles.json carries an invalid title row";
			return false;
		}
		Entry.iTitleId = static_cast<LostArk::Shared::HONOR_TITLE_ID>(pId->Get_Number());
		for (const HONOR_TITLE_ENTRY& Existing : staged)
		{
			if (Existing.iTitleId == Entry.iTitleId)
			{
				outStatus = "HonorTitles.json repeats title id " + std::to_string(Entry.iTitleId);
				return false;
			}
		}
		staged.push_back(std::move(Entry));
	}
	if (staged.empty())
	{
		outStatus = "HonorTitles.json lists no titles";
		return false;
	}

	g_Titles = std::move(staged);
	g_Strings = std::move(stagedStrings);
	outStatus = "Loaded " + std::to_string(g_Titles.size()) + " honor titles";
	return true;
}

const std::vector<Client::HONOR_TITLE_ENTRY>& Client::CHonorTitleCatalog::Get_Titles()
{
	return g_Titles;
}

const Client::HONOR_TITLE_ENTRY* Client::CHonorTitleCatalog::Find(
	const LostArk::Shared::HONOR_TITLE_ID iTitleId)
{
	if (LostArk::Shared::INVALID_HONOR_TITLE_ID == iTitleId)
		return nullptr;
	for (const HONOR_TITLE_ENTRY& Entry : g_Titles)
	{
		if (Entry.iTitleId == iTitleId)
			return &Entry;
	}
	return nullptr;
}

const wstring* Client::CHonorTitleCatalog::Find_Name(const LostArk::Shared::HONOR_TITLE_ID iTitleId)
{
	const HONOR_TITLE_ENTRY* pEntry = Find(iTitleId);
	return nullptr != pEntry ? &pEntry->strName : nullptr;
}

const Client::HONOR_TITLE_STRINGS& Client::CHonorTitleCatalog::Get_Strings()
{
	return g_Strings;
}
