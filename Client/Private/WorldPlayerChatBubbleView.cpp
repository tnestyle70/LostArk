#include "WorldPlayerChatBubbleView.h"

#pragma push_macro("new")
#undef new
#include <DirectXColors.h>
#pragma pop_macro("new")

#include "Character.h"
#include "GameInstance.h"
#include "Transform.h"
#include "UILabelFont.h"
#include "UILayoutRuntime.h"
#include "WorldPlayerNameplateView.h"

#include <algorithm>
#include <cmath>

namespace
{
	/* Same head anchor as the nameplate; the bubble stacks above the name from there. */
	constexpr f32_t HEAD_OFFSET = 2.2f;
	constexpr f32_t RETAIL_STAGE_HEIGHT = 1080.f;
	/* Retail: name plate top is 37 px above the anchor (HeadStatusNamePlate at -37); the balloon
	is the next stack element, so its bottom (tail tip) lands there. */
	constexpr f32_t STACK_BOTTOM_Y = -37.f;

	/* HeadStatus_Balloon normalBG: 72x50 art, DefineScalingGrid centre x 18.75..20.5 /
	y 17.75..19.75 px; everything right of / below the centre is the fixed tail side. */
	constexpr f32_t ART_W = 72.f;
	constexpr f32_t ART_H = 50.f;
	constexpr f32_t GRID_X0 = 18.75f;
	constexpr f32_t GRID_X1 = 20.5f;
	constexpr f32_t GRID_Y0 = 17.75f;
	constexpr f32_t GRID_Y1 = 19.75f;
	/* Balloon.as: _tailMargin 13, _defaultBGWidth 200, nocustomwordwrap text at (2,2) inside a
	host-set padding; the body interior of the art is ~8 px in from the sides and ~6 px from
	the top, so those are the paddings used here. */
	constexpr f32_t TAIL_H = 13.f;
	constexpr f32_t PAD_X = 8.f;
	constexpr f32_t PAD_TOP = 6.f;
	constexpr f32_t PAD_BOTTOM = 6.f;
	constexpr f32_t MAX_TEXT_W = 200.f;
	constexpr f32_t TEXT_PX = 12.f;
	constexpr f32_t TEXT_BOOST = 1.15f;
	/* Balloon.as defaultStrLength: a message this short is centred, longer ones are left aligned. */
	constexpr size_t CENTER_UP_TO_CHARS = 15;
	constexpr size_t BUBBLE_SLOTS = 4;
	constexpr const char* PIECES[9] = { "tl", "t", "tr", "l", "c", "r", "bl", "b", "br" };

	const wstring_t FONT_YG760 = TEXT("Font_YG760");
	const fvector_t COLOR_TEXT = XMVectorSet(238.f / 255.f, 238.f / 255.f, 238.f / 255.f, 1.f);   // #EEEEEE

	string Piece_Slot(const size_t iBubble, const size_t iPiece)
	{
		return "CB_" + std::to_string(iBubble) + "_" + PIECES[iPiece];
	}
}

void Client::CWorldPlayerChatBubbleView::Initialize(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const uint32_t iOwnerLevelIndex)
{
	m_pView = std::make_unique<CUILayoutRuntime>(
		pDevice, pContext, iOwnerLevelIndex, TEXT("Layer_UI"),
		L"UI/HeadStatus/ChatBubble_Layout.json");
	for (size_t i = 0; i < BUBBLE_SLOTS; ++i)
		Hide_Bubble(i);
}

void Client::CWorldPlayerChatBubbleView::Hide_Bubble(const size_t iBubble)
{
	if (nullptr == m_pView)
		return;
	for (size_t iPiece = 0; iPiece < 9; ++iPiece)
		m_pView->Set_SlotVisible(Piece_Slot(iBubble, iPiece), false);
}

void Client::CWorldPlayerChatBubbleView::Wrap_Text(const wstring_t& strFont, const f32_t fFontScale,
	const std::wstring& strText, const f32_t fMaxWidth, std::vector<std::wstring>& outLines) const
{
	outLines.clear();
	CGameInstance& gameInstance = CGameInstance::Get();
	std::wstring strLine;
	size_t iLastSpace = std::wstring::npos;
	for (const wchar_t ch : strText)
	{
		if (L'\n' == ch)
		{
			outLines.push_back(strLine);
			strLine.clear();
			iLastSpace = std::wstring::npos;
			continue;
		}
		strLine.push_back(ch);
		if (L' ' == ch)
			iLastSpace = strLine.size() - 1;
		const f32_t fWidth = gameInstance.Measure_Text(strFont, strLine.c_str()).x * fFontScale;
		if (fWidth <= fMaxWidth || strLine.size() <= 1)
			continue;
		if (std::wstring::npos != iLastSpace && iLastSpace > 0)
		{
			outLines.push_back(strLine.substr(0, iLastSpace));
			strLine.erase(0, iLastSpace + 1);
		}
		else
		{
			outLines.push_back(strLine.substr(0, strLine.size() - 1));
			strLine.erase(0, strLine.size() - 1);
		}
		iLastSpace = std::wstring::npos;
	}
	if (!strLine.empty() || outLines.empty())
		outLines.push_back(strLine);
}

void Client::CWorldPlayerChatBubbleView::Render(
	const CClientReplication& Replication,
	const std::vector<REPLICATED_PLAYER_VIEW>& Players)
{
	CGameInstance& gameInstance = CGameInstance::Get();
	const float4x4_t* const pViewMatrix =
		gameInstance.Get_Transform(D3DTS::VIEW);
	const float4x4_t* const pProjectionMatrix =
		gameInstance.Get_Transform(D3DTS::PROJ);
	const float2_t vViewportSize = gameInstance.Get_ViewportSize();
	if (nullptr == pViewMatrix || nullptr == pProjectionMatrix ||
		vViewportSize.x <= 0.f || vViewportSize.y <= 0.f)
	{
		return;
	}
	/* Art px -> screen px follows the viewport height (1080 stage); slots live in the
	document's reference resolution. */
	const f32_t fS = vViewportSize.y / RETAIL_STAGE_HEIGHT;
	const f32_t fRefWidth = nullptr != m_pView ? m_pView->Get_ResolutionWidth() : 1280.f;
	const f32_t fRefHeight = nullptr != m_pView ? m_pView->Get_ResolutionHeight() : 720.f;
	const f32_t fToRefX = fRefWidth / vViewportSize.x;
	const f32_t fToRefY = fRefHeight / vViewportSize.y;

	f32_t fFontScale = 1.f;
	const wstring_t strFont = UILabelFont::Resolve(FONT_YG760, TEXT_PX * fS * TEXT_BOOST, fFontScale);
	const f32_t fLineHeight = gameInstance.Measure_Text(strFont, L"\xAC00").y * fFontScale;

	size_t iBubble = 0;
	for (const REPLICATED_PLAYER_VIEW& player : Players)
	{
		if (iBubble >= BUBBLE_SLOTS)
			break;
		std::string bubbleText;
		if (!Replication.Try_Get_ActiveChatBubble(player.iNetEntityId, bubbleText))
			continue;
		const std::shared_ptr<CCharacter> pCharacter = player.pCharacter.lock();
		if (nullptr == pCharacter)
			continue;
		const std::shared_ptr<Engine::CTransform> pTransform = pCharacter->Get_Transform();
		if (nullptr == pTransform)
			continue;

		float3_t vHeadPosition{};
		XMStoreFloat3(&vHeadPosition, pTransform->Get_State(STATE::POSITION));
		vHeadPosition.y += HEAD_OFFSET;
		float2_t vAnchor{};
		if (!CWorldPlayerNameplateView::Try_ProjectWorldPosition(
			vHeadPosition, *pViewMatrix, *pProjectionMatrix, vViewportSize, vAnchor))
		{
			continue;
		}
		std::wstring bubbleWide;
		if (!CWorldPlayerNameplateView::Try_ConvertUtf8(bubbleText, bubbleWide))
			continue;

		Wrap_Text(strFont, fFontScale, bubbleWide, MAX_TEXT_W * fS, m_Lines);
		f32_t fTextW = 0.f;
		for (const std::wstring& strLine : m_Lines)
			fTextW = (std::max)(fTextW, gameInstance.Measure_Text(strFont, strLine.c_str()).x * fFontScale);
		const f32_t fTextH = fLineHeight * static_cast<f32_t>(m_Lines.size());

		/* Bubble box in screen px: never narrower/shorter than the art (the two fixed slices
		must fit), the tail hangs below the body. */
		const f32_t fLeftW = GRID_X0 * fS, fRightW = (ART_W - GRID_X1) * fS;
		const f32_t fTopH = GRID_Y0 * fS, fBottomH = (ART_H - GRID_Y1) * fS;
		const f32_t fW = (std::max)(ART_W * fS, fTextW + PAD_X * 2.f * fS);
		const f32_t fH = (std::max)(ART_H * fS, fTextH + (PAD_TOP + PAD_BOTTOM + TAIL_H) * fS);
		const f32_t fLeft = std::round(vAnchor.x - fW * 0.5f);
		const f32_t fTop = std::round(vAnchor.y + STACK_BOTTOM_Y * fS - fH);

		if (nullptr != m_pView)
		{
			const f32_t fXs[4] = { 0.f, fLeftW, fW - fRightW, fW };
			const f32_t fYs[4] = { 0.f, fTopH, fH - fBottomH, fH };
			const f32_t fUs[4] = { 0.f, GRID_X0 / ART_W, GRID_X1 / ART_W, 1.f };
			const f32_t fVs[4] = { 0.f, GRID_Y0 / ART_H, GRID_Y1 / ART_H, 1.f };
			for (size_t iRow = 0; iRow < 3; ++iRow)
			{
				for (size_t iCol = 0; iCol < 3; ++iCol)
				{
					const string strSlot = Piece_Slot(iBubble, iRow * 3 + iCol);
					m_pView->Set_SlotRect(strSlot,
						(fLeft + fXs[iCol]) * fToRefX, (fTop + fYs[iRow]) * fToRefY,
						(fXs[iCol + 1] - fXs[iCol]) * fToRefX, (fYs[iRow + 1] - fYs[iRow]) * fToRefY);
					m_pView->Set_SlotUVWindow(strSlot, fUs[iCol], fVs[iRow],
						fUs[iCol + 1] - fUs[iCol], fVs[iRow + 1] - fVs[iRow]);
					m_pView->Set_SlotVisible(strSlot, true);
				}
			}
		}

		const bool_t bCenter = 1 == m_Lines.size() && bubbleWide.size() <= CENTER_UP_TO_CHARS;
		f32_t fLineY = fTop + PAD_TOP * fS;
		for (const std::wstring& strLine : m_Lines)
		{
			const f32_t fLineW = gameInstance.Measure_Text(strFont, strLine.c_str()).x * fFontScale;
			const f32_t fLineX = bCenter ?
				fLeft + (fW - fLineW) * 0.5f : fLeft + PAD_X * fS;
			const float2_t vPosition(std::round(fLineX), std::round(fLineY));
			gameInstance.Draw_Text(strFont, strLine.c_str(),
				float2_t(vPosition.x + 1.f, vPosition.y + 1.f),
				XMVectorSet(0.f, 0.f, 0.f, 0.6f), 0.f, float2_t(0.f, 0.f), fFontScale);
			gameInstance.Draw_Text(strFont, strLine.c_str(), vPosition, COLOR_TEXT, 0.f,
				float2_t(0.f, 0.f), fFontScale);
			fLineY += fLineHeight;
		}
		++iBubble;
	}
	for (size_t i = iBubble; i < BUBBLE_SLOTS; ++i)
		Hide_Bubble(i);
}
