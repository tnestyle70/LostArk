#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <string>
#include <vector>

NS_BEGIN(Client)

/* The retail option table's control kinds (EFTable_SystemOption.ComponentType), as the row
document names them. Decoration kinds carry text or a rule and no value. */
enum class SYSTEM_OPTION_CONTROL : uint32_t
{
	COMBOBOX,
	SLIDER,
	CHECKBOX,
	HOTKEY,
	COLOR,
	SCALE_LIST,
	BUTTON,
	GROUP_TITLE,
	SUBTITLE,
	SEPARATOR,
	TEXT_INPUT,
	TAB_TITLE,
	COUNT_LIST,
	RADIO_PAIR,
	SPINNER,
	UNKNOWN,
};

struct SYSTEM_OPTION_CHOICE
{
	string strKey;
	wstring strText;
};

/* One row of one screen, straight from Data/UI/SystemOption/SystemOptionRows.json (itself a
projection of EFTable_SystemOption). Geometry is retail's own relative rule: the row hangs off
the row whose primaryKey is iAttachTarget, below it (iDirection 3) or to its right (2), offset
by the margins. */
struct SYSTEM_OPTION_ROW
{
	string strId;
	int32_t iPrimaryKey = 0;
	int32_t iOrder = 0;
	SYSTEM_OPTION_CONTROL eControl = SYSTEM_OPTION_CONTROL::UNKNOWN;
	int32_t iGroup = 0;
	/* UserOption.xml's Tag for this row; empty for decoration and for rows retail saves
	elsewhere (resolution). */
	string strSaveTag;
	wstring strTitle;
	wstring strTooltip;
	f32_t fDefault = 0.f;
	f32_t fMinimum = 0.f;
	f32_t fMaximum = 0.f;
	wstring strUnit;
	bool_t bShowSliderValue = true;
	int32_t iSnapInterval = -1;
	int32_t iAttachTarget = 0;
	int32_t iDirection = 3;
	f32_t fMarginX = 0.f;
	f32_t fMarginY = 0.f;
	f32_t fMarginLength = 0.f;
	f32_t fWidth = 0.f;
	f32_t fHeight = 0.f;
	/* false = retail does not show the row (the DirectX setting, voice chat). */
	bool_t bEnabled = true;
	/* GROUP_TITLE with the favourite star. */
	bool_t bFavourite = false;
	vector<SYSTEM_OPTION_CHOICE> Choices;
};

struct SYSTEM_OPTION_TAB
{
	int32_t iTabId = 0;
	int32_t iCategory = 0;
	int32_t iOrder = 0;
	wstring strTitle;
	/* Tab-column parent, when the screen sits under one (game play); empty otherwise. */
	string strParentId;
	wstring strParentLabel;
	vector<SYSTEM_OPTION_ROW> Rows;
};

/* Loader for the row document: parse -> validate -> commit as a whole, nothing partial. Only
the tabs the document marks `included` are kept, in the retail tab-column order. */
class CSystemOptionRowsDocument final
{
public:
	bool_t Load(string& strOutStatus);
	const vector<SYSTEM_OPTION_TAB>& Get_Tabs() const { return m_Tabs; }
	const SYSTEM_OPTION_TAB* Find_Tab(int32_t iTabId) const;

	static SYSTEM_OPTION_CONTROL Parse_Control(const string& strName);

private:
	vector<SYSTEM_OPTION_TAB> m_Tabs;
};

NS_END
