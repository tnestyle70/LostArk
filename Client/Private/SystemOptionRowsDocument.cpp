#include "SystemOptionRowsDocument.h"

#include "DataJson.h"
#include "ProjectDataRoot.h"

#include <algorithm>
#include <fstream>

namespace
{
	bool_t ConvertUtf8ToWide(const string& strUtf8, wstring& outWide)
	{
		outWide.clear();
		if (strUtf8.empty())
			return true;
		const int32_t iLength = ::MultiByteToWideChar(
			CP_UTF8, 0, strUtf8.c_str(), -1, nullptr, 0);
		if (iLength <= 1)
			return false;
		outWide.assign(static_cast<size_t>(iLength - 1), L'\0');
		::MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, outWide.data(), iLength);
		return true;
	}

	wstring Read_Text(const Client::DATA_JSON_VALUE& Object, const char* pKey)
	{
		const Client::DATA_JSON_VALUE* pValue = Object.Find(pKey);
		wstring strWide;
		if (nullptr != pValue && pValue->Is_String())
			(void)ConvertUtf8ToWide(pValue->Get_String(), strWide);
		return strWide;
	}

	string Read_String(const Client::DATA_JSON_VALUE& Object, const char* pKey)
	{
		const Client::DATA_JSON_VALUE* pValue = Object.Find(pKey);
		return (nullptr != pValue && pValue->Is_String()) ? pValue->Get_String() : string();
	}

	f32_t Read_Number(const Client::DATA_JSON_VALUE& Object, const char* pKey, const f32_t fFallback)
	{
		const Client::DATA_JSON_VALUE* pValue = Object.Find(pKey);
		return (nullptr != pValue && pValue->Is_Number()) ?
			static_cast<f32_t>(pValue->Get_Number()) : fFallback;
	}

	int32_t Read_Integer(const Client::DATA_JSON_VALUE& Object, const char* pKey, const int32_t iFallback)
	{
		const Client::DATA_JSON_VALUE* pValue = Object.Find(pKey);
		return (nullptr != pValue && pValue->Is_Number()) ?
			static_cast<int32_t>(pValue->Get_Number()) : iFallback;
	}

	bool_t Read_Boolean(const Client::DATA_JSON_VALUE& Object, const char* pKey, const bool_t bFallback)
	{
		const Client::DATA_JSON_VALUE* pValue = Object.Find(pKey);
		return (nullptr != pValue && pValue->Is_Boolean()) ? pValue->Get_Boolean() : bFallback;
	}
}

Client::SYSTEM_OPTION_CONTROL Client::CSystemOptionRowsDocument::Parse_Control(const string& strName)
{
	struct ENTRY final { const char* pName; SYSTEM_OPTION_CONTROL eControl; };
	static const ENTRY Entries[] =
	{
		{ "COMBOBOX",    SYSTEM_OPTION_CONTROL::COMBOBOX },
		{ "SLIDER",      SYSTEM_OPTION_CONTROL::SLIDER },
		{ "CHECKBOX",    SYSTEM_OPTION_CONTROL::CHECKBOX },
		{ "HOTKEY",      SYSTEM_OPTION_CONTROL::HOTKEY },
		{ "COLOR",       SYSTEM_OPTION_CONTROL::COLOR },
		{ "SCALE_LIST",  SYSTEM_OPTION_CONTROL::SCALE_LIST },
		{ "BUTTON",      SYSTEM_OPTION_CONTROL::BUTTON },
		{ "GROUP_TITLE", SYSTEM_OPTION_CONTROL::GROUP_TITLE },
		{ "SUBTITLE",    SYSTEM_OPTION_CONTROL::SUBTITLE },
		{ "SEPARATOR",   SYSTEM_OPTION_CONTROL::SEPARATOR },
		{ "TEXT_INPUT",  SYSTEM_OPTION_CONTROL::TEXT_INPUT },
		{ "TAB_TITLE",   SYSTEM_OPTION_CONTROL::TAB_TITLE },
		{ "COUNT_LIST",  SYSTEM_OPTION_CONTROL::COUNT_LIST },
		{ "RADIO_PAIR",  SYSTEM_OPTION_CONTROL::RADIO_PAIR },
		{ "SPINNER",     SYSTEM_OPTION_CONTROL::SPINNER },
	};
	for (const ENTRY& Entry : Entries)
	{
		if (strName == Entry.pName)
			return Entry.eControl;
	}
	return SYSTEM_OPTION_CONTROL::UNKNOWN;
}

bool_t Client::CSystemOptionRowsDocument::Load(string& strOutStatus)
{
	const filesystem::path Path =
		CProjectDataRoot::Resolve(L"UI/SystemOption/SystemOptionRows.json");
	ifstream Stream(Path, ios::binary);
	if (!Stream.is_open())
	{
		strOutStatus = "SystemOptionRows.json is missing.";
		return false;
	}
	const string Text((istreambuf_iterator<char>(Stream)), istreambuf_iterator<char>());
	DATA_JSON_VALUE Root;
	string Error;
	if (!CDataJson::Parse(Text, Root, Error) || !Root.Is_Object())
	{
		strOutStatus = "SystemOptionRows.json parse failed: " + Error;
		return false;
	}
	const DATA_JSON_VALUE* pSchema = Root.Find("schema");
	if (nullptr == pSchema || !pSchema->Is_String() ||
		pSchema->Get_String() != "lostark.system-option-rows")
	{
		strOutStatus = "SystemOptionRows.json has an unexpected schema.";
		return false;
	}
	const DATA_JSON_VALUE* pTabs = Root.Find("tabs");
	if (nullptr == pTabs || !pTabs->Is_Array())
	{
		strOutStatus = "SystemOptionRows.json has no tabs array.";
		return false;
	}

	/* Staged: the whole document parses and validates, or nothing changes. */
	vector<SYSTEM_OPTION_TAB> Staged;
	for (const DATA_JSON_VALUE& TabValue : pTabs->Get_Array())
	{
		if (!TabValue.Is_Object() || !Read_Boolean(TabValue, "included", false))
			continue;
		SYSTEM_OPTION_TAB Tab{};
		Tab.iTabId = Read_Integer(TabValue, "tabId", -1);
		Tab.iCategory = Read_Integer(TabValue, "category", -1);
		Tab.iOrder = Read_Integer(TabValue, "order", 0);
		Tab.strTitle = Read_Text(TabValue, "title");
		Tab.strParentId = Read_String(TabValue, "parentId");
		Tab.strParentLabel = Read_Text(TabValue, "parentLabel");
		if (Tab.iTabId < 0 || Tab.strTitle.empty())
		{
			strOutStatus = "SystemOptionRows.json: a tab has no id or title.";
			return false;
		}
		const DATA_JSON_VALUE* pRows = TabValue.Find("rows");
		if (nullptr == pRows || !pRows->Is_Array())
		{
			strOutStatus = "SystemOptionRows.json: tab " + std::to_string(Tab.iTabId) + " has no rows.";
			return false;
		}
		for (const DATA_JSON_VALUE& RowValue : pRows->Get_Array())
		{
			if (!RowValue.Is_Object())
				continue;
			SYSTEM_OPTION_ROW Row{};
			Row.strId = Read_String(RowValue, "id");
			Row.iPrimaryKey = Read_Integer(RowValue, "primaryKey", 0);
			Row.iOrder = Read_Integer(RowValue, "order", 0);
			Row.eControl = Parse_Control(Read_String(RowValue, "type"));
			Row.iGroup = Read_Integer(RowValue, "group", 0);
			Row.strSaveTag = Read_String(RowValue, "saveTag");
			Row.strTitle = Read_Text(RowValue, "title");
			Row.strTooltip = Read_Text(RowValue, "tooltip");
			Row.fDefault = Read_Number(RowValue, "default", 0.f);
			Row.fMinimum = Read_Number(RowValue, "minimum", 0.f);
			Row.fMaximum = Read_Number(RowValue, "maximum", 0.f);
			Row.strUnit = Read_Text(RowValue, "unit");
			Row.bShowSliderValue = Read_Boolean(RowValue, "showSliderValue", true);
			Row.iSnapInterval = Read_Integer(RowValue, "snapInterval", -1);
			Row.fWidth = Read_Number(RowValue, "width", 0.f);
			Row.fHeight = Read_Number(RowValue, "height", 0.f);
			Row.bEnabled = Read_Boolean(RowValue, "enabled", true);
			Row.bFavourite = Read_Boolean(RowValue, "favourite", false);
			if (const DATA_JSON_VALUE* pAttach = RowValue.Find("attach");
				nullptr != pAttach && pAttach->Is_Object())
			{
				Row.iAttachTarget = Read_Integer(*pAttach, "target", 0);
				Row.iDirection = Read_Integer(*pAttach, "direction", 3);
				Row.fMarginX = Read_Number(*pAttach, "marginX", 0.f);
				Row.fMarginY = Read_Number(*pAttach, "marginY", 0.f);
				Row.fMarginLength = Read_Number(*pAttach, "marginLength", 0.f);
			}
			if (const DATA_JSON_VALUE* pChoices = RowValue.Find("choices");
				nullptr != pChoices && pChoices->Is_Array())
			{
				for (const DATA_JSON_VALUE& ChoiceValue : pChoices->Get_Array())
				{
					if (!ChoiceValue.Is_Object())
						continue;
					SYSTEM_OPTION_CHOICE Choice{};
					Choice.strKey = Read_String(ChoiceValue, "key");
					Choice.strText = Read_Text(ChoiceValue, "text");
					Row.Choices.push_back(std::move(Choice));
				}
			}
			if (Row.strId.empty() || 0 == Row.iPrimaryKey)
			{
				strOutStatus = "SystemOptionRows.json: tab " + std::to_string(Tab.iTabId) +
					" has a row without id or primaryKey.";
				return false;
			}
			Tab.Rows.push_back(std::move(Row));
		}
		Staged.push_back(std::move(Tab));
	}
	if (Staged.empty())
	{
		strOutStatus = "SystemOptionRows.json marks no tab as included.";
		return false;
	}
	std::sort(Staged.begin(), Staged.end(),
		[](const SYSTEM_OPTION_TAB& a, const SYSTEM_OPTION_TAB& b) { return a.iOrder < b.iOrder; });

	m_Tabs = std::move(Staged);
	strOutStatus = "SystemOptionRows.json loaded: " + std::to_string(m_Tabs.size()) + " tabs.";
	return true;
}

const Client::SYSTEM_OPTION_TAB* Client::CSystemOptionRowsDocument::Find_Tab(const int32_t iTabId) const
{
	for (const SYSTEM_OPTION_TAB& Tab : m_Tabs)
	{
		if (Tab.iTabId == iTabId)
			return &Tab;
	}
	return nullptr;
}

void Client::CSystemOptionRowsDocument::Set_ResolutionChoices(const vector<SYSTEM_OPTION_CHOICE>& Choices)
{
	for (SYSTEM_OPTION_TAB& Tab : m_Tabs)
		for (SYSTEM_OPTION_ROW& Row : Tab.Rows)
			if (Row.strId == "combobox_resolution")
				Row.Choices = Choices;
}
