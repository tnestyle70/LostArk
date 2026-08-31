#include "PartyWindowView.h"

#include "GameInstance.h"
#include "ReplicatedPlayerHealth.h"
#include "UILayoutRuntime.h"

#include <algorithm>

namespace
{
	/* Same six playable classes CLASS_LIST_ENTRIES in Level_CharacterSelect.cpp maps to their
	UI/ClassSelect/<Class>/ folder -- kept as its own small table here rather than reusing that
	file's anonymous-namespace helper, since this is the only thing this view needs from it. */
	const char_t* Get_ClassSelectFolderName(
		const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
	{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		switch (characterClass)
		{
		case CHARACTER_CLASS_ID::LANCE_MASTER: return "LanceMaster";
		case CHARACTER_CLASS_ID::GUNSLINGER: return "Gunslinger";
		case CHARACTER_CLASS_ID::SLAYER: return "Slayer";
		case CHARACTER_CLASS_ID::ARTIST: return "Artist";
		case CHARACTER_CLASS_ID::DIMENSIONMASTER: return "DimensionMaster";
		case CHARACTER_CLASS_ID::WARLORD: return "Warlord";
		default: return nullptr;
		}
	}

	string Row_SlotId(const char_t* pPrefix, const size_t iRow)
	{
		return string(pPrefix) + std::to_string(iRow);
	}

	bool_t Convert_NicknameToWide(const string& strUtf8, wstring_t& outWide)
	{
		if (strUtf8.empty())
			return false;
		const int32_t iLength = ::MultiByteToWideChar(
			CP_UTF8, 0, strUtf8.c_str(), -1, nullptr, 0);
		if (iLength <= 1)
			return false;
		outWide.assign(static_cast<size_t>(iLength - 1), L'\0');
		::MultiByteToWideChar(
			CP_UTF8, 0, strUtf8.c_str(), -1, outWide.data(), iLength);
		return true;
	}
}

Client::CPartyWindowView::CPartyWindowView(
	ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pView{ make_unique<CUILayoutRuntime>(
		pDevice, pContext, ETOUI(LEVEL::STATIC), TEXT("Layer_UI"),
		L"UI/Party/PartyWindow_Layout.json") }
	// "161기 최후의 4인"
	, m_strPartyTitle{ "161\xea\xb8\xb0 \xec\xb5\x9c\xed\x9b\x84\xec\x9d\x98 4\xec\x9d\xb8" }
{
	/* A CUI_Sprite is visible from construction, unlike the old drawlist path that simply drew
	nothing while the roster was empty. */
	Hide_AllRows();
}

Client::CPartyWindowView::~CPartyWindowView()
{
}

void Client::CPartyWindowView::Hide_AllRows()
{
	if (nullptr == m_pView)
		return;
	m_pView->Set_SlotVisible("PartyWindow_TitleBg", false);
	for (size_t iRow = 0; iRow < MAX_ROWS; ++iRow)
	{
		m_pView->Set_SlotVisible(Row_SlotId("PartyWindow_Symbol_", iRow), false);
		m_pView->Set_SlotVisible(Row_SlotId("PartyWindow_HpBg_", iRow), false);
		m_pView->Set_SlotVisible(Row_SlotId("PartyWindow_HpFill_", iRow), false);
		m_pView->Set_SlotVisible(Row_SlotId("PartyWindow_Number_", iRow), false);
		m_pView->Set_SlotVisible(Row_SlotId("PartyWindow_LeaderMark_", iRow), false);
	}
}

void Client::CPartyWindowView::Sync_From_Roster(
	const LostArk::Shared::S2C_PARTY_ROSTER& Roster,
	const CReplicatedPlayerHealth& Health)
{
	m_Members.clear();
	m_Members.reserve(Roster.Members.size());
	for (size_t index = 0; index < Roster.Members.size(); ++index)
	{
		const LostArk::Shared::PARTY_ROSTER_MEMBER& Member = Roster.Members[index];
		const char_t* pFolderName = Get_ClassSelectFolderName(Member.eCharacterClass);
		const REPLICATED_PLAYER_HEALTH health = Health.Find(Member.iNetEntityId);
		m_Members.push_back(PARTY_MEMBER{
			Member.strNickname,
			nullptr != pFolderName ?
				string("UI/ClassSelect/") + pFolderName + "/IdentitySymbol.png" : string{},
			health.Get_Ratio(),
			health.hasSnapshot,
			0 == index });
	}
}

void Client::CPartyWindowView::Render()
{
	if (nullptr == m_pView)
		return;
	if (m_Members.empty())
	{
		Hide_AllRows();
		return;
	}

	m_pView->Set_SlotVisible("PartyWindow_TitleBg", true);

	for (size_t iRow = 0; iRow < MAX_ROWS; ++iRow)
	{
		const string strSymbol = Row_SlotId("PartyWindow_Symbol_", iRow);
		const string strHpBg = Row_SlotId("PartyWindow_HpBg_", iRow);
		const string strHpFill = Row_SlotId("PartyWindow_HpFill_", iRow);
		const string strNumber = Row_SlotId("PartyWindow_Number_", iRow);
		const string strLeader = Row_SlotId("PartyWindow_LeaderMark_", iRow);

		if (iRow >= m_Members.size())
		{
			m_pView->Set_SlotVisible(strSymbol, false);
			m_pView->Set_SlotVisible(strHpBg, false);
			m_pView->Set_SlotVisible(strHpFill, false);
			m_pView->Set_SlotVisible(strNumber, false);
			m_pView->Set_SlotVisible(strLeader, false);
			continue;
		}

		const PARTY_MEMBER& Member = m_Members[iRow];

		/* A class with no symbol art keeps its row (bar/number/nickname) but shows no symbol,
		matching the old path's own empty-path check. */
		if (Member.strClassSymbolPath.empty())
		{
			m_pView->Set_SlotVisible(strSymbol, false);
		}
		else
		{
			m_pView->Set_SlotVisible(strSymbol, true);
			m_pView->Set_SlotTexture(strSymbol, Member.strClassSymbolPath);
		}

		m_pView->Set_SlotVisible(strHpBg, true);
		m_pView->Set_SlotVisible(strNumber, true);

		/* Before the accepted world snapshot (or outside the replicated world) there is no HP
		to show, so the fill stays hidden over its own background art. */
		if (Member.hasHealthSnapshot)
		{
			const f32_t fHpRatio = std::clamp(Member.fHpRatio, 0.f, 1.f);
			m_pView->Set_SlotVisible(strHpFill, true);
			m_pView->Set_SlotFillRatio(strHpFill, fHpRatio);
		}
		else
		{
			m_pView->Set_SlotVisible(strHpFill, false);
		}

		m_pView->Set_SlotVisible(strLeader, Member.isLeader);
	}
}

void Client::CPartyWindowView::RenderText()
{
	if (nullptr == m_pView || m_Members.empty())
		return;

	const float2_t vViewportSize = CGameInstance::Get().Get_ViewportSize();
	const f32_t fRefWidth = m_pView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pView->Get_ResolutionHeight();
	if (fRefWidth <= 0.f || fRefHeight <= 0.f)
		return;
	const f32_t fScaleX = vViewportSize.x / fRefWidth;
	const f32_t fScaleY = vViewportSize.y / fRefHeight;
	const f32_t fUiScale = (std::min)(fScaleX, fScaleY);

	/* Left-anchored, vertically centered on the given reference-resolution point -- the same
	placement the old drawlist computed by hand from CalcTextSize. */
	const auto Fn_DrawLeft = [&](f32_t fX, f32_t fCenterY, const wchar_t* pText,
		f32_t fTargetHeight, const fvector_t& vColor)
	{
		const float2_t vMeasured =
			CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), pText);
		if (vMeasured.y <= 0.f)
			return;
		const f32_t fScale = (fTargetHeight / vMeasured.y) * fUiScale;
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), pText,
			float2_t(fX * fScaleX, fCenterY * fScaleY),
			vColor, 0.f, float2_t(0.f, 0.5f), fScale);
	};

	f32_t fTitleX = 0.f, fTitleY = 0.f, fTitleW = 0.f, fTitleH = 0.f;
	if (m_pView->Get_SlotRect(
		"PartyWindow_TitleBg", fTitleX, fTitleY, fTitleW, fTitleH))
	{
		wstring_t strTitle;
		if (Convert_NicknameToWide(m_strPartyTitle, strTitle))
		{
			Fn_DrawLeft(fTitleX + 10.f, fTitleY + fTitleH * 0.5f, strTitle.c_str(),
				13.f, Colors::White);
		}
	}

	for (size_t iRow = 0; iRow < MAX_ROWS && iRow < m_Members.size(); ++iRow)
	{
		f32_t fBarX = 0.f, fBarY = 0.f, fBarW = 0.f, fBarH = 0.f;
		if (!m_pView->Get_SlotRect(
			Row_SlotId("PartyWindow_HpBg_", iRow), fBarX, fBarY, fBarW, fBarH))
		{
			continue;
		}

		/* Nickname starts just past the join-order digit art, exactly as before -- the digit's
		own slot rect supplies the width instead of a hardcoded per-digit table. */
		f32_t fNumberX = 0.f, fNumberY = 0.f, fNumberW = 0.f, fNumberH = 0.f;
		const f32_t fNameX = m_pView->Get_SlotRect(
			Row_SlotId("PartyWindow_Number_", iRow),
			fNumberX, fNumberY, fNumberW, fNumberH) ?
			fNumberX + fNumberW + 4.f : fBarX + 6.f;

		wstring_t strNickname;
		if (!Convert_NicknameToWide(m_Members[iRow].strNickname, strNickname))
			continue;
		Fn_DrawLeft(fNameX, fBarY + fBarH * 0.5f, strNickname.c_str(),
			13.f, Colors::White);
	}
}
