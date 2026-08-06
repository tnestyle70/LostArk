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
		case CHARACTER_CLASS_ID::WARLORD: return "Warlord";
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
		case CHARACTER_CLASS_ID::WARLORD: return "Warlord";
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

	constexpr float iconSize = 64.f; // matches the skill icon PNGs' native 64x64 pixels

	ImGui::BeginChild("##SkillList", ImVec2(400.f, -8.f), true);
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

			/* Row background art (user-cut from the game's own skill window atlas) draws
			first; the selected-row overlay draws on top of that when this row is the current
			selection. Both keep SkillPanel.png's own aspect ratio (scaled to the row height,
			not stretched to whatever width the row happens to be) so the bar does not warp --
			it just does not reach the row's full width. Neither participates in hit-testing --
			the ImageButton drawn afterward still owns the click. */
			{
				ID3D11ShaderResourceView* pRowBg =
					m_pTextureCache->Get_Or_Load("UI/SkillWindow/SkillPanel.png");
				const ImVec2 rowMin = ImGui::GetCursorScreenPos();
				const float rowBarWidth = iconSize * (622.f / 62.f);
				const ImVec2 rowMax(
					rowMin.x + (min)(rowBarWidth, ImGui::GetContentRegionAvail().x),
					rowMin.y + iconSize);
				if (nullptr != pRowBg)
					ImGui::GetWindowDrawList()->AddImage(pRowBg, rowMin, rowMax);
				if (isSelected)
				{
					ID3D11ShaderResourceView* pRowSelected =
						m_pTextureCache->Get_Or_Load("UI/SkillWindow/PanelSelected.png");
					if (nullptr != pRowSelected)
						ImGui::GetWindowDrawList()->AddImage(pRowSelected, rowMin, rowMax);
				}
			}

			if (nullptr != pIcon)
			{
				if (ImGui::ImageButton("##icon", pIcon, ImVec2(iconSize, iconSize)))
					m_SelectedSkillId = entry.iSkillId;
			}
			else if (ImGui::Button("##icon", ImVec2(iconSize, iconSize)))
			{
				m_SelectedSkillId = entry.iSkillId;
			}
			ImGui::SameLine();
			ImGui::TextUnformatted(entry.strDisplayName.c_str());
			ImGui::PopID();
		}
	}
	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild("##TripodPanel", ImVec2(260.f, -8.f), true, ImGuiWindowFlags_NoBackground);
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

	ImGui::BeginChild("##RunePanel", ImVec2(0.f, -8.f), true);
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
