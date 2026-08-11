#include "imgui.h"

#include "PartyWindowView.h"

#include "UITextureCache.h"

Client::CPartyWindowView::CPartyWindowView(ComPtr<ID3D11Device> pDevice)
	: m_pTextureCache{ make_unique<CUITextureCache>(pDevice) }
	/* Placeholder title -- there is no real party name until a Server party system exists. */
	, m_strPartyTitle{ "\xed\x8c\x8c\xed\x8b\xb0" }
{
	/* UI-only seed roster (no party Shared protocol yet) so the layout can be checked against
	the reference visually. Includes one DimensionMaster entry to exercise the no-symbol-art
	fallback the same way Character Select already does. */
	m_Members.push_back(PARTY_MEMBER{
		"\xed\x95\x9c\xeb\xb6\x93\xed\x94\xbd\xed\x95\x98\xec\x86\x8c",
		"UI/ClassSelect/Warlord/IdentitySymbol.png", 1.f, true });
	m_Members.push_back(PARTY_MEMBER{
		"A\xea\xb8\x89\xed\x95\x9c\xec\x9a\xb0",
		"UI/ClassSelect/LanceMaster/IdentitySymbol.png", 1.f, false });
	m_Members.push_back(PARTY_MEMBER{
		"\xeb\x8f\x84\xed\x99\x94\xea\xb0\x80\xeb\x8b\x98",
		"UI/ClassSelect/Artist/IdentitySymbol.png", 1.f, false });
	m_Members.push_back(PARTY_MEMBER{
		"\xec\xb0\xa8\xec\x9b\x90\xec\x88\xa0\xec\x82\xac\xeb\x8b\x98",
		"", 1.f, false });
}

Client::CPartyWindowView::~CPartyWindowView()
{
}

void Client::CPartyWindowView::Render()
{
	if (m_Members.empty())
		return;

	constexpr f32_t REF_WIDTH = 1280.f;
	constexpr f32_t REF_HEIGHT = 720.f;
	constexpr f32_t PANEL_X = 20.f;
	/* Reference has the title bar sitting about a third of the way down the screen, not
	flush against the top-left corner. */
	constexpr f32_t PANEL_Y = REF_HEIGHT / 3.f;

	/* Native sizes of the extracted party art (Client/Bin/Resources/UI/Party) -- the title
	bar is drawn shorter than its native 28px height (reference shows it noticeably thinner
	than a member row). */
	constexpr f32_t TITLE_WIDTH = 248.f;
	constexpr f32_t TITLE_HEIGHT = 19.2f;
	constexpr f32_t TITLE_TO_LIST_GAP = 15.f;
	constexpr f32_t SYMBOL_SIZE = 34.f;
	constexpr f32_t SYMBOL_GAP = 2.f;
	constexpr f32_t HP_BAR_WIDTH = 207.f;
	constexpr f32_t HP_BAR_HEIGHT = 26.f;
	constexpr f32_t HP_FILL_INSET = 0.5f;
	constexpr f32_t ROW_GAP = 13.5f;
	constexpr f32_t NUMBER_INSET_X = 6.f;
	constexpr f32_t LEADER_MARK_WIDTH = 20.f;
	constexpr f32_t LEADER_MARK_HEIGHT = 16.f;

	/* No ImGui window here on purpose: Begin()/End() carries window-background/border painting
	that kept showing up as an unwanted black panel behind the whole group no matter which
	NoBackground/BgAlpha/style-color combination was pushed. RenderStanceSkillIcons() and
	RenderSkillCooldowns() in MainApp.cpp already draw the rest of the combat HUD straight onto
	the foreground draw list with no window at all and have never had this problem, so the party
	roster now follows the same proven pattern instead of fighting window style flags. */
	ImGuiViewport* pViewport = ImGui::GetMainViewport();
	const f32_t fScaleX = pViewport->WorkSize.x / REF_WIDTH;
	const f32_t fScaleY = pViewport->WorkSize.y / REF_HEIGHT;
	ImDrawList* pDrawList = ImGui::GetForegroundDrawList(pViewport);

	const f32_t fPanelScreenX = pViewport->WorkPos.x + PANEL_X * fScaleX;
	const f32_t fPanelScreenY = pViewport->WorkPos.y + PANEL_Y * fScaleY;

	const ImVec2 vOrigin(fPanelScreenX, fPanelScreenY);

	const ImVec2 vTitleMin = vOrigin;
	const ImVec2 vTitleMax(
		vTitleMin.x + TITLE_WIDTH * fScaleX, vTitleMin.y + TITLE_HEIGHT * fScaleY);
	if (ID3D11ShaderResourceView* pTitleBg =
		m_pTextureCache->Get_Or_Load("UI/Party/Party Name.png"))
	{
		pDrawList->AddImage(pTitleBg, vTitleMin, vTitleMax);
	}
	{
		const ImVec2 vTitleSize = ImGui::CalcTextSize(m_strPartyTitle.c_str());
		pDrawList->AddText(
			ImVec2(
				vTitleMin.x + 10.f * fScaleX,
				vTitleMin.y + (vTitleMax.y - vTitleMin.y - vTitleSize.y) * 0.5f),
			IM_COL32(255, 255, 255, 255), m_strPartyTitle.c_str());
	}

	ID3D11ShaderResourceView* pHpBgSRV =
		m_pTextureCache->Get_Or_Load("UI/Party/Party Hp Bg.png");
	ID3D11ShaderResourceView* pHpFillSRV =
		m_pTextureCache->Get_Or_Load("UI/Party/Party HP.png");
	ID3D11ShaderResourceView* pLeaderMarkSRV =
		m_pTextureCache->Get_Or_Load("UI/Party/Party Leader Mark.png");

	f32_t fRowY = vTitleMax.y + TITLE_TO_LIST_GAP * fScaleY;
	for (size_t index = 0; index < m_Members.size(); ++index)
	{
		const PARTY_MEMBER& Member = m_Members[index];

		const ImVec2 vSymbolMin(vOrigin.x, fRowY);
		const ImVec2 vSymbolMax(
			vSymbolMin.x + SYMBOL_SIZE * fScaleX,
			vSymbolMin.y + SYMBOL_SIZE * fScaleY);
		if (!Member.strClassSymbolPath.empty())
		{
			if (ID3D11ShaderResourceView* pSymbolSRV =
				m_pTextureCache->Get_Or_Load(Member.strClassSymbolPath))
			{
				pDrawList->AddImage(pSymbolSRV, vSymbolMin, vSymbolMax);
			}
		}

		const ImVec2 vBarMin(
			vSymbolMax.x + SYMBOL_GAP * fScaleX, fRowY);
		const ImVec2 vBarMax(
			vBarMin.x + HP_BAR_WIDTH * fScaleX,
			vBarMin.y + HP_BAR_HEIGHT * fScaleY);
		if (nullptr != pHpBgSRV)
			pDrawList->AddImage(pHpBgSRV, vBarMin, vBarMax);
		if (nullptr != pHpFillSRV)
		{
			const f32_t fHpRatio = (std::min)(1.f, (std::max)(0.f, Member.fHpRatio));
			const ImVec2 vFillMin(
				vBarMin.x + HP_FILL_INSET * fScaleX, vBarMin.y + HP_FILL_INSET * fScaleY);
			const ImVec2 vFillMax(
				vFillMin.x + (HP_BAR_WIDTH - HP_FILL_INSET * 2.f) * fScaleX * fHpRatio,
				vBarMax.y - HP_FILL_INSET * fScaleY);
			const ImVec2 vFillUvMax(fHpRatio, 1.f);
			pDrawList->AddImage(
				pHpFillSRV, vFillMin, vFillMax, ImVec2(0.f, 0.f), vFillUvMax);
		}

		/* Each digit's own native size (Party No.1.png is 7x15, No.4 is 10x15, ...) --
		forcing them all into one fixed box stretched No.1 nearly 1.5x wider than drawn,
		which read as oversized/distorted next to the reference. NUMBER_SHRINK draws them
		smaller still, matching how small the reference's digits are against the bar. */
		constexpr f32_t NUMBER_SHRINK = 0.7f;
		struct NUMBER_ART { const char_t* pPath; f32_t fWidth; f32_t fHeight; };
		constexpr NUMBER_ART NUMBER_ARTS[] = {
			{ "UI/Party/Party No.1.png", 7.f, 15.f },
			{ "UI/Party/Party No.2.png", 9.f, 15.f },
			{ "UI/Party/Party No.3.png", 9.f, 15.f },
			{ "UI/Party/Party No.4.png", 10.f, 15.f },
		};
		const NUMBER_ART* pNumberArt =
			index < std::size(NUMBER_ARTS) ? &NUMBER_ARTS[index] : nullptr;
		f32_t fTextX = vBarMin.x + NUMBER_INSET_X * fScaleX;
		if (nullptr != pNumberArt)
		{
			if (ID3D11ShaderResourceView* pNumberSRV =
				m_pTextureCache->Get_Or_Load(pNumberArt->pPath))
			{
				const f32_t fNumberWidth = pNumberArt->fWidth * NUMBER_SHRINK;
				const f32_t fNumberHeight = pNumberArt->fHeight * NUMBER_SHRINK;
				const ImVec2 vNumberMin(
					fTextX,
					vBarMin.y + (vBarMax.y - vBarMin.y) * 0.5f - fNumberHeight * 0.5f * fScaleY);
				const ImVec2 vNumberMax(
					vNumberMin.x + fNumberWidth * fScaleX,
					vNumberMin.y + fNumberHeight * fScaleY);
				pDrawList->AddImage(pNumberSRV, vNumberMin, vNumberMax);
				fTextX = vNumberMax.x + 4.f * fScaleX;
			}
		}
		{
			const ImVec2 vNameSize = ImGui::CalcTextSize(Member.strNickname.c_str());
			pDrawList->AddText(
				ImVec2(
					fTextX,
					vBarMin.y + (vBarMax.y - vBarMin.y - vNameSize.y) * 0.5f),
				IM_COL32(255, 255, 255, 255), Member.strNickname.c_str());
		}

		if (Member.isLeader && nullptr != pLeaderMarkSRV)
		{
			/* Flush with the bar's left edge (not centered on the corner) so the crown
			never overhangs past the bar's left side, only above it. */
			const ImVec2 vLeaderMin(
				vBarMin.x, vBarMin.y - LEADER_MARK_HEIGHT * fScaleY * 0.5f);
			const ImVec2 vLeaderMax(
				vLeaderMin.x + LEADER_MARK_WIDTH * fScaleX,
				vLeaderMin.y + LEADER_MARK_HEIGHT * fScaleY);
			pDrawList->AddImage(pLeaderMarkSRV, vLeaderMin, vLeaderMax);
		}

		fRowY = vBarMax.y + ROW_GAP * fScaleY;
	}
}
