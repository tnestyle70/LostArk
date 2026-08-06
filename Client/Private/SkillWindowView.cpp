#include "imgui.h"

#include "SkillWindowView.h"

#include "DataJson.h"
#include "HUDRuntimeView.h"
#include "ProjectDataRoot.h"
#include "UITextureCache.h"

#include <fstream>

namespace
{
	using LostArk::Shared::CHARACTER_CLASS_ID;

	/* File stem under Data/UI/SkillWindow/ and Resources/UI/Skill/. Mirrors the same PascalCase
	class ids MainApp.cpp already uses for HUD_Layout.json ("Yinyangshi" for ARTIST included),
	so one class keeps one name across every UI document instead of inventing a second one here. */
	const char* Get_ClassIdStem(const CHARACTER_CLASS_ID characterClass)
	{
		switch (characterClass)
		{
		case CHARACTER_CLASS_ID::LANCE_MASTER: return "LanceMaster";
		case CHARACTER_CLASS_ID::GUNSLINGER: return "Gunslinger";
		case CHARACTER_CLASS_ID::SLAYER: return "Slayer";
		case CHARACTER_CLASS_ID::ARTIST: return "Yinyangshi";
		case CHARACTER_CLASS_ID::DIMENSIONMASTER: return "DimensionMaster";
		default: return "";
		}
	}

	const char* Get_ClassDisplayName(const CHARACTER_CLASS_ID characterClass)
	{
		switch (characterClass)
		{
		case CHARACTER_CLASS_ID::LANCE_MASTER: return "Lance Master";
		case CHARACTER_CLASS_ID::GUNSLINGER: return "Gunslinger";
		case CHARACTER_CLASS_ID::SLAYER: return "Slayer";
		case CHARACTER_CLASS_ID::ARTIST: return "Artist";
		case CHARACTER_CLASS_ID::DIMENSIONMASTER: return "Dimension Master";
		default: return "Unknown";
		}
	}

	const DATA_JSON_VALUE* Required(
		const DATA_JSON_VALUE& object,
		const char* name,
		const DATA_JSON_TYPE type)
	{
		const DATA_JSON_VALUE* value = object.Find(name);
		return nullptr != value && value->Get_Type() == type ? value : nullptr;
	}
}

Client::CSkillWindowView::CSkillWindowView(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pTextureCache{ make_unique<CUITextureCache>(pDevice) }
	, m_pBackgroundView{ make_unique<CHUDRuntimeView>(
		pDevice, pContext, L"UI/SkillWindow/SkillWindow_Layout.json", true) }
{
}

Client::CSkillWindowView::~CSkillWindowView()
{
}

const vector<Client::CSkillWindowView::SKILL_ROSTER_ENTRY>*
Client::CSkillWindowView::Get_Roster(
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
{
	const int32_t classKey = static_cast<int32_t>(characterClass);
	const auto cached = m_Rosters.find(classKey);
	if (m_Rosters.end() != cached)
		return &cached->second;
	if (m_RosterLoadStatus.end() != m_RosterLoadStatus.find(classKey))
		return nullptr;

	const string strClassId = Get_ClassIdStem(characterClass);
	if (strClassId.empty())
	{
		m_RosterLoadStatus[classKey] = "Unknown character class";
		return nullptr;
	}

	const filesystem::path relativePath =
		filesystem::path(L"UI/SkillWindow") / (strClassId + ".skillroster.json");
	const filesystem::path resolvedPath = CProjectDataRoot::Resolve(relativePath);

	ifstream stream(resolvedPath, ios::binary);
	if (!stream.is_open())
	{
		m_RosterLoadStatus[classKey] =
			strClassId + ".skillroster.json not published yet";
		return nullptr;
	}

	const string text(
		(istreambuf_iterator<char>(stream)),
		istreambuf_iterator<char>());

	DATA_JSON_VALUE root;
	string parseError;
	if (!CDataJson::Parse(text, root, parseError) || !root.Is_Object())
	{
		m_RosterLoadStatus[classKey] = "Skill roster document failed to parse";
		return nullptr;
	}

	const DATA_JSON_VALUE* formatVersion = Required(root, "formatVersion", DATA_JSON_TYPE::NUMBER);
	const DATA_JSON_VALUE* skillArray = Required(root, "skills", DATA_JSON_TYPE::ARRAY);
	if (nullptr == formatVersion || 1.0 != formatVersion->Get_Number() ||
		nullptr == skillArray)
	{
		m_RosterLoadStatus[classKey] = "Skill roster document is not formatVersion 1";
		return nullptr;
	}

	/* Staged into a local first: a document that fails halfway must not leave a partial
	roster behind for Render to iterate. */
	vector<SKILL_ROSTER_ENTRY> staged;
	for (const DATA_JSON_VALUE& value : skillArray->Get_Array())
	{
		const DATA_JSON_VALUE* id = Required(value, "skillId", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* name = Required(value, "displayName", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* stance = Required(value, "stance", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* icon = Required(value, "iconPath", DATA_JSON_TYPE::STRING);
		if (nullptr == id || nullptr == name || nullptr == stance || nullptr == icon)
		{
			m_RosterLoadStatus[classKey] = "Skill roster document is missing a required field";
			return nullptr;
		}

		SKILL_ROSTER_ENTRY entry{};
		entry.iSkillId = static_cast<LostArk::Shared::SKILL_ID>(id->Get_Number());
		entry.strDisplayName = name->Get_String();
		entry.strStance = stance->Get_String();
		entry.strIconPath = icon->Get_String();
		if (LostArk::Shared::INVALID_SKILL_ID == entry.iSkillId || entry.strDisplayName.empty())
		{
			m_RosterLoadStatus[classKey] = "Skill roster document has an invalid skill entry";
			return nullptr;
		}
		staged.push_back(move(entry));
	}

	auto committed = m_Rosters.emplace(classKey, move(staged)).first;
	return &committed->second;
}

void Client::CSkillWindowView::Render(
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
{
	if (!m_bOpen)
		return;
	/* Pinned to fill the main viewport every frame (not just on first use, and no move/resize)
	so this window's own top-left is always the viewport's WorkPos -- the same origin
	CHUDRuntimeView's slot math (Slot.fX/fY off pViewport->WorkPos) already assumes. That is
	what lets a slot placed in CHUDLayoutTool's "Skill Window" tab land in the same spot here
	that it did there, instead of drifting with wherever a movable window happened to be. */
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos, ImGuiCond_Always);
	ImGui::SetNextWindowSize(viewport->WorkSize, ImGuiCond_Always);

	const string strTitle =
		string("Combat Skills - ") + Get_ClassDisplayName(characterClass) + "###SkillWindow";
	constexpr ImGuiWindowFlags windowFlags =
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
	if (!ImGui::Begin(strTitle.c_str(), &m_bOpen, windowFlags))
	{
		ImGui::End();
		return;
	}

	/* Authored background art (panel frames, tripod plate, ...) draws first, into this
	window's own draw list, so it sits behind everything the rest of this function draws. */
	if (nullptr != m_pBackgroundView)
		m_pBackgroundView->Render("", 0);

	const vector<SKILL_ROSTER_ENTRY>* pRoster = Get_Roster(characterClass);
	if (nullptr == pRoster)
	{
		const int32_t classKey = static_cast<int32_t>(characterClass);
		const auto status = m_RosterLoadStatus.find(classKey);
		ImGui::TextDisabled("%s",
			m_RosterLoadStatus.end() != status ? status->second.c_str() : "Loading...");
		ImGui::End();
		return;
	}

	/* Single scale applied to every atlas-sourced piece in this window (icon, row background,
	...) uniformly. These pieces are all cropped from the same shared atlas, so their sizes are
	already proportioned to each other at native scale; scaling only one of them independently
	is what warped the row background's circle into an ellipse earlier. Change this one number
	to resize everything together instead of touching any single element's size. */
	constexpr float uiScale = 0.8f;
	constexpr float rowBgWidth = 622.f * uiScale;
	constexpr float rowBgHeight = 62.f * uiScale;
	/* The icon must not exceed the row's own height (the native art is 64x64 icon on a 62-tall
	bar, a 2px mismatch that becomes visible once everything is scaled up together), so the row
	height is the icon's sizing authority, not its own native 64. */
	constexpr float iconSize = rowBgHeight;

	/* ImGuiCol_ChildBg is fully transparent by default, so without an explicit opaque fill
	here the tripod background art (drawn earlier, straight to this window's own draw list,
	before any child) shows through this child's empty space instead of being covered by it --
	that was the faint tripod ghost bleeding into the skill list. */
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.06f, 0.06f, 0.08f, 1.f));
	ImGui::BeginChild("##SkillList", ImVec2(rowBgWidth + 30.f, -8.f), true);
	ImGui::PopStyleColor();
	ImGui::TextDisabled("Skill List (%d)", static_cast<int32_t>(pRoster->size()));
	ImGui::Separator();
	for (const char* stanceFilter : { "LONG", "SHORT", "NONE" })
	{
		bool_t hasHeader = false;
		for (const SKILL_ROSTER_ENTRY& entry : *pRoster)
		{
			if (entry.strStance != stanceFilter)
				continue;
			if (!hasHeader)
			{
				ImGui::TextColored(
					ImVec4(0.6f, 0.8f, 1.f, 1.f),
					"%s",
					string("LONG") == stanceFilter ? "Long Stance" :
					string("SHORT") == stanceFilter ? "Short Stance" : "Shared");
				hasHeader = true;
			}

			ImGui::PushID(entry.iSkillId);
			ID3D11ShaderResourceView* pIcon =
				m_pTextureCache->Get_Or_Load(entry.strIconPath);
			const bool_t isSelected = m_SelectedSkillId == entry.iSkillId;

			/* The whole row is one click target (not just the icon), matching the reference --
			clicking the bar, the name, or the icon all select this skill. Everything below is
			drawn straight to the draw list at fixed offsets from rowMin instead of through
			separate ImGui widgets, so nothing here carries ImGui's own button frame/border
			(that showed up as a blue outline around each icon that the reference does not have)
			and nothing but this one InvisibleButton owns the click or the row's layout height. */
			const ImVec2 rowMin = ImGui::GetCursorScreenPos();
			ImDrawList* pDrawList = ImGui::GetWindowDrawList();

			ID3D11ShaderResourceView* pRowBg =
				m_pTextureCache->Get_Or_Load("UI/SkillWindow/SkillPanel.png");
			if (nullptr != pRowBg)
			{
				pDrawList->AddImage(pRowBg, rowMin,
					ImVec2(rowMin.x + rowBgWidth, rowMin.y + rowBgHeight));
			}
			if (isSelected)
			{
				ID3D11ShaderResourceView* pRowSelected =
					m_pTextureCache->Get_Or_Load("UI/SkillWindow/PanelSelected.png");
				if (nullptr != pRowSelected)
				{
					pDrawList->AddImage(pRowSelected, rowMin,
						ImVec2(rowMin.x + rowBgWidth, rowMin.y + rowBgHeight));
				}
			}
			if (nullptr != pIcon)
			{
				pDrawList->AddImage(pIcon, rowMin,
					ImVec2(rowMin.x + iconSize, rowMin.y + iconSize));
			}
			const ImU32 textColor = ImGui::GetColorU32(ImGuiCol_Text);
			pDrawList->AddText(
				ImVec2(rowMin.x + iconSize + 6.f, rowMin.y + rowBgHeight * 0.5f - 7.f),
				textColor, entry.strDisplayName.c_str());

			/* Placeholder values -- this project has no skill-point investment system yet
			(Data/Balance carries cooldown/damage/etc. but no point cost or level concept),
			so these are display-only stand-ins, same as the tripod/rune panels, until that
			system exists to source real numbers from. */
			const ImU32 disabledColor = ImGui::GetColorU32(ImGuiCol_TextDisabled);
			pDrawList->AddText(
				ImVec2(rowMin.x + rowBgWidth - 90.f, rowMin.y + rowBgHeight * 0.5f - 7.f),
				disabledColor, "Req 1");
			pDrawList->AddText(
				ImVec2(rowMin.x + rowBgWidth - 40.f, rowMin.y + rowBgHeight * 0.5f - 7.f),
				disabledColor, "Lv 1");

			if (ImGui::InvisibleButton("##row", ImVec2(rowBgWidth, rowBgHeight)))
				m_SelectedSkillId = entry.iSkillId;
			ImGui::PopID();
		}
	}
	ImGui::EndChild();

	ImGui::SameLine();

	/* Matches the tripod plate's own scaled width (260 native * uiScale) instead of staying at
	the old fixed 260 -- this child was still sized for the pre-scale art after the tripod plate
	and nodes shrank by uiScale, leaving it too big for what it now holds. */
	ImGui::BeginChild("##TripodPanel", ImVec2(260.f * uiScale, -8.f), true, ImGuiWindowFlags_NoBackground);
	/* No hardcoded tripod art here anymore -- the tripod plate and all 8 node glows are
	slots in Data/UI/SkillWindow/SkillWindow_Layout.json, placed with CHUDLayoutTool's
	"Skill Window" tab and drawn once for the whole window by m_pBackgroundView->Render()
	above. This child only reserves layout space and shows the disabled-state text. */
	ImGui::TextDisabled("Tripod (display only, not applied yet)");
	if (LostArk::Shared::INVALID_SKILL_ID != m_SelectedSkillId)
	{
		for (const SKILL_ROSTER_ENTRY& entry : *pRoster)
		{
			if (entry.iSkillId != m_SelectedSkillId)
				continue;
			ImGui::TextUnformatted(entry.strDisplayName.c_str());
			break;
		}
	}
	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.06f, 0.06f, 0.08f, 1.f));
	ImGui::BeginChild("##RunePanel", ImVec2(0.f, -8.f), true);
	ImGui::PopStyleColor();
	ImGui::TextDisabled("Rune (display only, not applied yet)");
	ImGui::Separator();
	ImGui::BeginDisabled();
	static char runeSearchBuffer[64] = "";
	ImGui::InputTextWithHint("##RuneSearch", "Rune name or effect", runeSearchBuffer, sizeof(runeSearchBuffer));
	ImGui::EndDisabled();
	ImGui::TextDisabled("Rune equipping is out of scope for this pass.");
	ImGui::EndChild();

	ImGui::End();
}
