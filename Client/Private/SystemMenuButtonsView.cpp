#include "SystemMenuButtonsView.h"

// 텍스트 렌더 시 viewport 크기를 조회한다.
#include "GameInstance.h"
// 유효한 메뉴 클릭 효과음을 재생한다.
#include "UISoundPlayer.h"
// 프레임 포인터 선점 여부와 아이콘 rect의 hover/click을 검사한다.
#include "UIInputRouter.h"
// 아이콘 아래 문구를 화면 크기에 맞춰 중앙 정렬한다.
#include "UILabelFont.h"
// 메뉴 슬롯의 rect·표시·tint 상태를 읽고 갱신한다.
#include "UILayoutRuntime.h"

namespace
{
	struct BUTTON
	{
		const char* pSlotId;
		Client::CSystemMenuButtonsView::INTENT eIntent;
		const wchar_t* pLabel;
	};
	// stable slot ID를 캐릭터 선택/환경설정 의도와 문구에 연결한다.
	constexpr BUTTON BUTTONS[] = {
		{ "SMB_CharacterSelect", Client::CSystemMenuButtonsView::INTENT::RETURN_TO_CHARACTER_SELECT,
			L"\xCE90\xB9AD\xD130 \xC120\xD0DD" },
		{ "SMB_Options", Client::CSystemMenuButtonsView::INTENT::OPEN_OPTIONS,
			L"\xD658\xACBD\xC124\xC815" },
	};
	constexpr f32_t LABEL_PX = 12.f;	// viewport 높이에 따라 조절할 기준 픽셀 크기다.
	constexpr f32_t LABEL_GAP = 4.f;	// 아이콘 아래쪽과 문구 사이의 기준 간격이다.
	const wstring_t LABEL_FONT = TEXT("Font_YoonGasiIIM");
	// hover 전용 이미지 대신 tint 배율을 높여 아이콘을 밝힌다.
	const float4_t TINT_NORMAL = float4_t(1.f, 1.f, 1.f, 1.f);
	const float4_t TINT_HOVER = float4_t(1.45f, 1.45f, 1.45f, 1.f);
}

void Client::CSystemMenuButtonsView::Initialize(
	ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext,
	const uint32_t iOwnerLevelIndex)
{
	m_pView = std::make_unique<CUILayoutRuntime>(pDevice, pContext, iOwnerLevelIndex,
		TEXT("Layer_UI"), L"UI/SystemMenu/SystemMenuButtons_Layout.json");
}

// 상단 UI의 입력 소비를 존중한다. hover 플래그를 Level에 돌려주고 이 함수가 mouse claim을 직접 하지는 않는다.
Client::CSystemMenuButtonsView::INTENT Client::CSystemMenuButtonsView::Update(const bool_t bVisible)
{
	m_bPointerOver = false;
	if (nullptr == m_pView)
		return INTENT::NONE;
	if (bVisible != m_bShown)
	{
		for (const BUTTON& Button : BUTTONS)
			m_pView->Set_SlotVisible(Button.pSlotId, bVisible);
		m_bShown = bVisible;
	}
	if (!bVisible)
		return INTENT::NONE;

	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pView->Get_ResolutionHeight();
	INTENT eIntent = INTENT::NONE;
	for (const BUTTON& Button : BUTTONS)
	{
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		if (!m_pView->Get_SlotRect(Button.pSlotId, fX, fY, fW, fH))
			continue;
		// 먼저 포인터를 소비한 창·modal이 있으면 아이콘 hover와 클릭을 처리하지 않는다.
		const bool_t bHovered = !Router.Is_MouseClaimedThisFrame() &&
			Router.Is_Hovered(fX, fY, fW, fH, fRefWidth, fRefHeight);
		m_pView->Set_SlotTintMultiplier(Button.pSlotId, bHovered ? TINT_HOVER : TINT_NORMAL);
		if (!bHovered)
			continue;
		m_bPointerOver = true;
		if (INTENT::NONE == eIntent && Router.Is_Clicked(fX, fY, fW, fH, fRefWidth, fRefHeight))
		{
			Client::Play_UISound(Client::UI_SOUND_CUE::BUTTON_CLICK);
			eIntent = Button.eIntent;
		}
	}
	return eIntent;
}

// 저작 rect를 viewport 픽셀로 바꾸고 아이콘별 문구를 그린다. 유효한 viewport/기준 크기가 필요하다.
void Client::CSystemMenuButtonsView::Render_Text() const
{
	if (nullptr == m_pView || !m_bShown)
		return;
	const float2_t vViewport = CGameInstance::Get().Get_ViewportSize();
	const f32_t fRefWidth = m_pView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pView->Get_ResolutionHeight();
	if (vViewport.x <= 0.f || vViewport.y <= 0.f || fRefWidth <= 0.f || fRefHeight <= 0.f)
		return;
	const f32_t fScaleX = vViewport.x / fRefWidth;
	const f32_t fScaleY = vViewport.y / fRefHeight;
	for (const BUTTON& Button : BUTTONS)
	{
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		if (!m_pView->Get_SlotRect(Button.pSlotId, fX, fY, fW, fH))
			continue;
		(void)UILabelFont::Draw_Centered(LABEL_FONT, Button.pLabel,
			(fX + fW * 0.5f) * fScaleX, (fY + fH + LABEL_GAP + LABEL_PX * 0.5f) * fScaleY,
			LABEL_PX * fScaleY, DirectX::Colors::White);
	}
}
