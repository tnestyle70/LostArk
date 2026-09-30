#include "DungeonTimerView.h"

#include "GameInstance.h"
#include "UILayoutRuntime.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cwchar>

namespace
{
	constexpr f32_t REF_WIDTH = 1280.f;
	constexpr f32_t REF_HEIGHT = 720.f;
	const char* const EMBLEM_SLOT = "DungeonTimer_Emblem";

	/* raidImageMc's own size in the movie. Every offset below is in that same
	space and is scaled by the authored slot width divided by this. */
	constexpr f32_t NATIVE_EMBLEM_WIDTH = 241.f;

	/* Normal and decimal-warning strings share the timeTF centre and font size.
	The authored timeTF bounds are x 19..225, y 131..175.8 in emblem space. */
	constexpr f32_t TIME_CENTER_X = 122.f;
	constexpr f32_t TIME_CENTER_Y = 153.4f;
	/* timeTF fontHeight 680 twips. */
	constexpr f32_t TIME_FONT_PX = 34.f;

	/* The hidden "setting" clip's own text: a = warning, b = normal. */
	constexpr std::uint32_t NORMAL_COLOR_RGB = 0xFFD200u;
	constexpr std::uint32_t WARNING_COLOR_RGB = 0xCC3300u;

	fvector_t To_Color(const std::uint32_t iRgb)
	{
		return XMVectorSet(
			static_cast<f32_t>((iRgb >> 16) & 0xFFu) / 255.f,
			static_cast<f32_t>((iRgb >> 8) & 0xFFu) / 255.f,
			static_cast<f32_t>(iRgb & 0xFFu) / 255.f,
			1.f);
	}

	/* TranslateTime.getTime with timeType "min": both parts zero padded to two
	digits and joined by ":" with no spaces around it. */
	wstring Format_MinuteSecond(const f32_t fSeconds)
	{
		const int32_t iTotal = static_cast<int32_t>(std::ceil((std::max)(fSeconds, 0.f)));
		const int32_t iMinutes = iTotal / 60;
		const int32_t iSeconds = iTotal % 60;
		wchar_t Buffer[16] = {};
		swprintf_s(Buffer, L"%02d:%02d", iMinutes, iSeconds);
		return Buffer;
	}
}

Client::CDungeonTimerView::CDungeonTimerView(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const uint32_t iGameObjectLevelIndex)
{
	m_pView = std::make_unique<CUILayoutRuntime>(pDevice, pContext, iGameObjectLevelIndex,
		TEXT("Layer_UI"), L"UI/KoukuSaydon/DungeonTimer_Layout.json");
	m_pView->Set_AllSlotsVisible(false);
	m_bSlotFound = m_pView->Get_SlotRect(EMBLEM_SLOT,
		m_fEmblemX, m_fEmblemY, m_fEmblemWidth, m_fEmblemHeight);
	if (!m_bSlotFound || m_fEmblemWidth <= 0.f)
	{
		OutputDebugStringA(
			"[DungeonTimer] DungeonTimer_Layout.json emblem slot missing -- the timer stays hidden.\n");
		m_bSlotFound = false;
	}
}

Client::CDungeonTimerView::~CDungeonTimerView()
{
}

void Client::CDungeonTimerView::Hide()
{
	if (!m_bVisible)
		return;
	m_pView->Set_AllSlotsVisible(false);
	m_bVisible = false;
}

void Client::CDungeonTimerView::Update(const f32_t fTimeDelta, const HUD_DUNGEON_TIMER_STATE& State)
{
	if (!m_bSlotFound || !State.isVisible ||
		!std::isfinite(State.fSeconds) || State.fSeconds < 0.f)
	{
		Hide();
		return;
	}

	m_fSeconds = State.fSeconds;
	m_fWarningSeconds = State.fWarningSeconds;
	m_pView->Set_AllSlotsVisible(true);
	m_bVisible = true;
	m_pView->Update(fTimeDelta);
}

void Client::CDungeonTimerView::Render() const
{
	if (!m_bVisible)
		return;
	const float2_t vViewport = CGameInstance::Get().Get_ViewportSize();
	if (vViewport.x <= 0.f || vViewport.y <= 0.f)
		return;

	/* Reference units -> viewport pixels, and movie units -> reference units. */
	const f32_t fScaleX = vViewport.x / REF_WIDTH;
	const f32_t fScaleY = vViewport.y / REF_HEIGHT;
	const f32_t fMovieScale = m_fEmblemWidth / NATIVE_EMBLEM_WIDTH;

	/* Measured from one fixed string, not from the text being drawn: SpriteFont
	measures the actual glyphs, so "10:00" and "9.87" come back different heights
	and the number would change size as it counts down. */
	const float2_t vMetric =
		CGameInstance::Get().Measure_Text(TEXT("Font_YoonGasiIIM"), L"00:00");
	if (vMetric.y <= 0.f)
		return;

	const auto draw = [&](const f32_t fMovieCenterX, const f32_t fMovieCenterY,
		const f32_t fMovieFontPx, const wstring& strText, const std::uint32_t iRgb)
	{
		if (strText.empty())
			return;
		const float2_t& vMeasured = vMetric;
		/* The sprite font is baked at its own pixel height, so scale it until one
		line is the authored point size. */
		const f32_t fDrawScale =
			(fMovieFontPx * fMovieScale * fScaleY) / vMeasured.y;
		const float2_t vPosition(
			(m_fEmblemX + fMovieCenterX * fMovieScale) * fScaleX,
			(m_fEmblemY + fMovieCenterY * fMovieScale) * fScaleY);
		/* CCustomFont::Draw takes the origin as a 0..1 anchor fraction. */
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), strText.c_str(),
			vPosition, To_Color(iRgb), 0.f, float2_t(0.5f, 0.5f), fDrawScale);
	};

	if (m_fSeconds >= m_fWarningSeconds)
	{
		draw(TIME_CENTER_X, TIME_CENTER_Y, TIME_FONT_PX,
			Format_MinuteSecond(m_fSeconds), NORMAL_COLOR_RGB);
		return;
	}

	// One string shares a baseline, glyph size and centering across the decimal point.
	wchar_t raw[32] = {};
	swprintf_s(raw, L"%.2f", (std::max)(m_fSeconds, 0.f));
	draw(TIME_CENTER_X, TIME_CENTER_Y, TIME_FONT_PX, raw, WARNING_COLOR_RGB);
}
