// 미니게임 문양과 남은 초를 화면에 표시한다.
#include "DungeonTimerView.h"

// viewport와 폰트 크기를 조회하고 시간 문자열을 그린다.
#include "GameInstance.h"
// 문양 JSON을 읽어 sprite의 표시와 애니메이션을 갱신한다.
#include "UILayoutRuntime.h"

// 음수 초를 막는 max를 사용한다.
#include <algorithm>
// 입력의 isfinite 검사와 남은 초의 ceil 계산을 사용한다.
#include <cmath>
// 시간을 wide 문자열로 만드는 swprintf_s의 표준 입출력·문자 헤더를 포함한다.
#include <cstdio>
#include <cwchar>

namespace
{
	constexpr f32_t REF_WIDTH = 1280.f;
	constexpr f32_t REF_HEIGHT = 720.f;
	const char* const EMBLEM_SLOT = "DungeonTimer_Emblem";

	// 원본 문양의 폭이다. 원본 글자 좌표는 저작 슬롯 폭 / 이 값으로 환산한다.
	constexpr f32_t NATIVE_EMBLEM_WIDTH = 241.f;

	// 일반·경고 표시가 공유하는 문양 기준 글자 중심이다.
	constexpr f32_t TIME_CENTER_X = 122.f;
	constexpr f32_t TIME_CENTER_Y = 153.4f;
	// 원본 글자 높이 680 twip을 픽셀로 환산한 값이다.
	constexpr f32_t TIME_FONT_PX = 34.f;

	// 일반·경고 상태의 원본 글자 RGB 값이다.
	constexpr std::uint32_t NORMAL_COLOR_RGB = 0xFFD200u;
	constexpr std::uint32_t WARNING_COLOR_RGB = 0xCC3300u;

	// 24비트 RGB의 각 채널을 0~1로 바꾸고 alpha 1을 붙인다.
	fvector_t To_Color(const std::uint32_t iRgb)
	{
		return XMVectorSet(
			static_cast<f32_t>((iRgb >> 16) & 0xFFu) / 255.f,
			static_cast<f32_t>((iRgb >> 8) & 0xFFu) / 255.f,
			static_cast<f32_t>(iRgb & 0xFFu) / 255.f,
			1.f);
	}

	// 0 이상으로 제한해 올림한 초를 두 자리 분:초로 만든다.
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

// 지정 Level의 Layer_UI에 문양을 만들고 필수 슬롯이 없으면 계속 숨긴다.
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

// 표시 가능한 입력만 글자 상태로 복사한다. 유효하지 않은 입력은 이전 표시를 숨긴다.
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

// 보관한 시간을 화면 글자로 그린다. viewport·폰트 조회 실패는 이번 출력을 생략한다.
void Client::CDungeonTimerView::Render() const
{
	if (!m_bVisible)
		return;
	const float2_t vViewport = CGameInstance::Get().Get_ViewportSize();
	if (vViewport.x <= 0.f || vViewport.y <= 0.f)
		return;

	// 1280×720 문서 좌표는 viewport 픽셀로, 원본 문양 좌표는 저작 슬롯 크기로 환산한다.
	const f32_t fScaleX = vViewport.x / REF_WIDTH;
	const f32_t fScaleY = vViewport.y / REF_HEIGHT;
	const f32_t fMovieScale = m_fEmblemWidth / NATIVE_EMBLEM_WIDTH;

	// 고정 문자열의 높이로 배율을 계산해 남은 시간의 숫자가 바뀌어도 글자 크기를 유지한다.
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
		// 원본 글자 높이에 문양·viewport 배율을 적용하고 등록 폰트 높이로 나눈다.
		const f32_t fDrawScale =
			(fMovieFontPx * fMovieScale * fScaleY) / vMeasured.y;
		const float2_t vPosition(
			(m_fEmblemX + fMovieCenterX * fMovieScale) * fScaleX,
			(m_fEmblemY + fMovieCenterY * fMovieScale) * fScaleY);
		// Draw_Text의 0.5, 0.5는 문자열 중심을 vPosition에 맞추는 정규화 원점이다.
		CGameInstance::Get().Draw_Text(TEXT("Font_YoonGasiIIM"), strText.c_str(),
			vPosition, To_Color(iRgb), 0.f, float2_t(0.5f, 0.5f), fDrawScale);
	};

	// 경고 전에는 분:초, 경고 구간에는 소수 둘째 자리 초로 표시한다.
	if (m_fSeconds >= m_fWarningSeconds)
	{
		draw(TIME_CENTER_X, TIME_CENTER_Y, TIME_FONT_PX,
			Format_MinuteSecond(m_fSeconds), NORMAL_COLOR_RGB);
		return;
	}

	// 소수점을 포함한 한 문자열로 그려 숫자의 기준선·크기·중심을 맞춘다.
	wchar_t raw[32] = {};
	swprintf_s(raw, L"%.2f", (std::max)(m_fSeconds, 0.f));
	draw(TIME_CENTER_X, TIME_CENTER_Y, TIME_FONT_PX, raw, WARNING_COLOR_RGB);
}
