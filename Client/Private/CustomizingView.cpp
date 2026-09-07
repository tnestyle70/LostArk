#include "CustomizingView.h"

#include "Character.h"
#include "FaceCustomizeApplier.h"
#include "GameInstance.h"
#include "MainApp.h"
#include "UIInputRouter.h"
#include "UILayoutRuntime.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <iterator>

namespace
{
	constexpr f32_t REF_WIDTH = 1280.f;
	constexpr f32_t REF_HEIGHT = 720.f;

	/* Korean labels are written as wide escapes: this project's C++ sources are compiled
	without /utf-8 and a raw Korean byte in a narrow literal breaks the CP949 lead-byte
	handling. Values are the retail pccreate.* strings. */
	constexpr const wchar_t* LABEL_TABS[] = {
		L"\xAE30\xBCF8",					/* base */
		L"\xC5BC\xAD74",					/* face */
		L"\xBA38\xB9AC",					/* hair */
		L"\xB208",							/* eye */
		L"\xD53C\xBD80",					/* skin */
		L"\xAFB8\xBBF8\xAE30",				/* adorn */
		L"\xC74C\xC131",					/* voice */
	};
	constexpr int32_t FACE_TAB_INDEX = 1;

	constexpr const wchar_t* LABEL_DETAIL_DESC =
		L"\xC5BC\xAD74\xC758 \xC0C1\xC138\xD55C \xACE8\xACA9 \xC124\xC815";
	constexpr const wchar_t* LABEL_FOLD_DETAIL = L"\xC138\xBD80 \xC124\xC815";
	constexpr const wchar_t* LABEL_TOP_GUIDE =
		L"\xCE90\xB9AD\xD130 \xC0DD\xC131 \xD6C4 24\xC2DC\xAC04 \xC774\xB0B4 \xC678\xD615\xC744 "
		L"\xC790\xC720\xB86D\xAC8C \xBCC0\xACBD \xAC00\xB2A5\xD569\xB2C8\xB2E4.";
	constexpr const wchar_t* LABEL_DECIDE = L"\xC678\xD615 \xACB0\xC815";
	constexpr const wchar_t* LABEL_BACK = L"\xB4A4\xB85C\xAC00\xAE30";
	constexpr const wchar_t* LABEL_RESET_ALL = L"\xC804\xCCB4 \xCD08\xAE30\xD654";
	constexpr const wchar_t* LABEL_FACE_RANDOM = L"\xBB34\xC791\xC704 \xC5BC\xAD74";
	constexpr const wchar_t* LABEL_FACE_RESET = L"\xC5BC\xAD74 \xCD08\xAE30\xD654";
	constexpr const wchar_t* LABEL_GUIDE_ROTATE = L"\xCE90\xB9AD\xD130 \xD68C\xC804";
	constexpr const wchar_t* LABEL_GUIDE_ZOOM = L"\xD655\xB300/\xCD95\xC18C";
	constexpr const wchar_t* LABEL_NO_SLIDERS =
		L"\xC774 \xD074\xB798\xC2A4\xB294 \xC5BC\xAD74 \xC2AC\xB77C\xC774\xB354 "
		L"\xB370\xC774\xD130\xAC00 \xC544\xC9C1 \xC5C6\xC2B5\xB2C8\xB2E4.";
	constexpr const wchar_t* LABEL_NOT_READY = L"\xC900\xBE44 \xC911\xC778 \xD56D\xBAA9\xC785\xB2C8\xB2E4.";

	/* One row of CharCustom_Right_TabFaceDetailPart<N>: the slider id in
	<race>.facesliders.json, its retail row caption, and which part sprite it belongs to.
	fPartY is the row's y inside that part sprite, matching the same value
	build_customizing_ui.py placed the track and thumb slots at. */
	struct FACE_SLIDER_ROW
	{
		int32_t iPart;
		const char_t* pSliderId;
		const wchar_t* pLabel;
		f32_t fPartY;
	};

	constexpr const wchar_t* ROW_HORIZONTAL = L"\xAC00\xB85C";
	constexpr const wchar_t* ROW_VERTICAL = L"\xC138\xB85C";
	constexpr const wchar_t* ROW_GAP = L"\xAC04\xACA9";
	constexpr const wchar_t* ROW_HEIGHT = L"\xB192\xC774";
	constexpr const wchar_t* ROW_ANGLE = L"\xAC01\xB3C4";
	constexpr const wchar_t* ROW_PULL = L"\xB3CC\xCD9C";
	constexpr const wchar_t* ROW_SCALE = L"\xD06C\xAE30";

	constexpr FACE_SLIDER_ROW FACE_SLIDER_ROWS[] = {
		{ 0, "eye_scaleh", ROW_HORIZONTAL, 26.f },
		{ 0, "eye_scalev", ROW_VERTICAL, 56.f },
		{ 0, "eye_width", ROW_GAP, 119.f },
		{ 0, "eye_height", ROW_HEIGHT, 149.f },
		{ 0, "eye_angle", ROW_ANGLE, 211.f },
		{ 1, "eyebrow_height", ROW_HEIGHT, 26.f },
		{ 1, "eyebrow_angle", ROW_ANGLE, 56.f },
		{ 2, "cheekbone_pull", ROW_PULL, 26.f },
		{ 2, "cheekbone_height", ROW_HEIGHT, 56.f },
		{ 3, "jaw_scale", ROW_SCALE, 26.f },
		{ 3, "jaw_height", ROW_HEIGHT, 56.f },
		{ 3, "jaw_pull", ROW_PULL, 119.f },
		{ 3, "jaw_angle", ROW_ANGLE, 149.f },
		{ 4, "nose_scale", ROW_SCALE, 26.f },
		{ 4, "nose_height", ROW_HEIGHT, 56.f },
		{ 4, "nose_length", ROW_PULL, 86.f },
		{ 5, "mouthlip_height", ROW_HEIGHT, 26.f },
	};

	/* The group captions each part sprite draws above its rows, at their own y. */
	struct FACE_PART_HEADER
	{
		int32_t iPart;
		const wchar_t* pLabel;
		f32_t fPartY;
	};

	constexpr FACE_PART_HEADER FACE_PART_HEADERS[] = {
		{ 0, L"\xB208 \xD06C\xAE30", -1.f },
		{ 0, L"\xB208 \xC704\xCE58", 92.f },
		{ 0, L"\xD68C\xC804", 184.f },
		{ 1, L"\xB208\xC378 \xD615\xD0DC", -1.f },
		{ 2, L"\xAD11\xB300 \xD615\xD0DC", -1.f },
		{ 3, L"\xD131 \xD06C\xAE30", -1.f },
		{ 3, L"\xD131 \xBAA8\xC591", 92.f },
		{ 4, L"\xCF54 \xD615\xD0DC", -1.f },
		{ 5, L"\xC785 \xD615\xD0DC", -1.f },
	};

	constexpr const wchar_t* LABEL_PARTS[] = {
		L"\xB208",							/* eye */
		L"\xB208\xC378",					/* eyebrow */
		L"\xAD11\xB300",					/* cheekbone */
		L"\xD131",							/* jaw */
		L"\xCF54",							/* nose */
		L"\xC785",							/* mouth */
	};
	constexpr int32_t FACE_PART_COUNT = 6;

	/* Part sprite origin in reference units: CharCustom_Right_TabFace (-437,192) > detail
	(0,48) > content (34,51) > part (0,118), taken from 1920x1080 to 1280x720. */
	constexpr f32_t PART_ORIGIN_X = (1920.f - 437.f + 34.f) * 2.f / 3.f;
	constexpr f32_t PART_ORIGIN_Y = (192.f + 48.f + 51.f + 118.f) * 2.f / 3.f;
	constexpr f32_t PART_SCALE = 2.f / 3.f;

	constexpr const char_t* ASSET_TAB_BG = "UI/Customizing/tab_bg.png";
	constexpr const char_t* ASSET_TAB_BG_SELECTED = "UI/Customizing/tab_bg_selected.png";
	constexpr const char_t* ASSET_SUBTAB_NORMAL = "UI/Customizing/v2_tab2_normal.png";
	constexpr const char_t* ASSET_SUBTAB_OVER = "UI/Customizing/v2_tab2_over.png";
	constexpr const char_t* ASSET_BUTTON_NORMAL = "UI/Customizing/v2_btn_normal.png";
	constexpr const char_t* ASSET_BUTTON_OVER = "UI/Customizing/v2_btn_over.png";
	constexpr const char_t* ASSET_THUMB_NORMAL = "UI/Customizing/v2_slider_thumb.png";
	constexpr const char_t* ASSET_THUMB_OVER = "UI/Customizing/v2_slider_thumb_over.png";
	constexpr const char_t* ASSET_BACK_ICON = "UI/Customizing/back_icon.png";
	constexpr const char_t* ASSET_BACK_ICON_OVER = "UI/Customizing/back_icon_over.png";
	constexpr const char_t* ASSET_RESET_ICON = "UI/Customizing/resetall_icon.png";
	constexpr const char_t* ASSET_RESET_ICON_OVER = "UI/Customizing/resetall_icon_over.png";

	constexpr f32_t ORBIT_MIN_DISTANCE = 1.1f;
	constexpr f32_t ORBIT_MAX_DISTANCE = 6.f;
	constexpr f32_t ORBIT_MIN_PITCH = -12.f;
	constexpr f32_t ORBIT_MAX_PITCH = 40.f;
	/* Right panel and bottom bar own the pointer; a drag started anywhere else orbits. */
	constexpr f32_t PANEL_LEFT_EDGE = 983.f;
	constexpr f32_t BOTTOM_BAR_TOP_EDGE = 640.f;

	string Slider_TrackSlotId(const char_t* pSliderId)
	{
		return string("CC_Slider_") + pSliderId + "_Track";
	}
	string Slider_ThumbSlotId(const char_t* pSliderId)
	{
		return string("CC_Slider_") + pSliderId + "_Thumb";
	}

	/* Index of pSliderId inside the character's applier, or -1 when this class's document has
	no such slider (a race whose bones that slider needs are all missing, or no document). */
	int32_t Find_SliderIndex(const CCharacter& Character, const char_t* pSliderId)
	{
		const CFaceCustomizeApplier& Face = Character.Get_FaceCustomize();
		for (size_t i = 0; i < Face.Get_SliderCount(); ++i)
		{
			if (Face.Get_SliderId(i) == pSliderId)
				return static_cast<int32_t>(i);
		}
		return -1;
	}
}

Client::CCustomizingView::CCustomizingView(
	ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext,
	const uint32_t iGameObjectLevelIndex)
	: m_pView{ std::make_unique<CUILayoutRuntime>(pDevice, pContext,
		iGameObjectLevelIndex, TEXT("Layer_UI"),
		L"UI/Customizing/CustomizingUI.json") }
{
	Hide();
}

Client::CCustomizingView::~CCustomizingView() = default;

void Client::CCustomizingView::Open()
{
	m_bOpen = true;
	m_iSelectedTab = FACE_TAB_INDEX;
	m_iSelectedPart = 0;
	m_iDraggingRow = -1;
	m_bOrbitDragging = false;
	m_fOrbitYaw = 20.f;
	m_fOrbitPitch = 6.f;
	m_fOrbitDistance = 2.6f;
	m_bDecideRequested = false;
	m_bBackRequested = false;
}

void Client::CCustomizingView::Close()
{
	m_bOpen = false;
	m_iDraggingRow = -1;
	m_bOrbitDragging = false;
	Hide();
}

void Client::CCustomizingView::Hide()
{
	if (nullptr == m_pView)
		return;
	m_pView->Set_AllSlotsVisible(false);
}

bool_t Client::CCustomizingView::Get_SlotRect(const char_t* pSlotId,
	f32_t& outX, f32_t& outY, f32_t& outWidth, f32_t& outHeight) const
{
	return nullptr != m_pView &&
		m_pView->Get_SlotRect(pSlotId, outX, outY, outWidth, outHeight) &&
		outWidth > 0.f && outHeight > 0.f;
}

bool_t Client::CCustomizingView::Is_Hovered(
	const f32_t fX, const f32_t fY, const f32_t fWidth, const f32_t fHeight) const
{
	return CUIInputRouter::Get().Is_Hovered(fX, fY, fWidth, fHeight, REF_WIDTH, REF_HEIGHT);
}

bool_t Client::CCustomizingView::Is_Clicked(
	const f32_t fX, const f32_t fY, const f32_t fWidth, const f32_t fHeight) const
{
	return CUIInputRouter::Get().Is_Clicked(fX, fY, fWidth, fHeight, REF_WIDTH, REF_HEIGHT);
}

bool_t Client::CCustomizingView::Try_Consume_Decide()
{
	const bool_t bRequested = m_bDecideRequested;
	m_bDecideRequested = false;
	return bRequested;
}

bool_t Client::CCustomizingView::Try_Consume_Back()
{
	const bool_t bRequested = m_bBackRequested;
	m_bBackRequested = false;
	return bRequested;
}

float3_t Client::CCustomizingView::Get_CameraPositionOffset() const
{
	const f32_t fYaw = XMConvertToRadians(m_fOrbitYaw);
	const f32_t fPitch = XMConvertToRadians(m_fOrbitPitch);
	const f32_t fHorizontal = m_fOrbitDistance * std::cos(fPitch);
	const float3_t vLook = Get_CameraLookOffset();
	return float3_t(
		fHorizontal * std::sin(fYaw),
		vLook.y + m_fOrbitDistance * std::sin(fPitch),
		fHorizontal * std::cos(fYaw));
}

float3_t Client::CCustomizingView::Get_CameraLookOffset() const
{
	/* Zoomed all the way in the camera looks at the head, pulled back it settles on the
	chest, which is what the retail screen does as the wheel moves. */
	const f32_t fRange = ORBIT_MAX_DISTANCE - ORBIT_MIN_DISTANCE;
	const f32_t fNear = fRange > 0.f ?
		std::clamp((m_fOrbitDistance - ORBIT_MIN_DISTANCE) / fRange, 0.f, 1.f) : 0.f;
	return float3_t(0.f, 1.55f - 0.5f * fNear, 0.f);
}

void Client::CCustomizingView::Update(
	const f32_t fTimeDelta, const shared_ptr<CCharacter>& pCharacter)
{
	UNREFERENCED_PARAMETER(fTimeDelta);
	if (!m_bOpen || nullptr == m_pView)
		return;

	/* Full-screen modal-style ownership of the pointer, the same claim every other runtime
	screen makes while it is up, so a click here never also reaches gameplay. */
	CUIInputRouter::Get().Claim_Mouse_This_Frame();

	Update_Orbit();
	Update_Tabs();
	Update_FaceTab(pCharacter);
	Update_Buttons(pCharacter);
}

void Client::CCustomizingView::Update_Orbit()
{
	CUIInputRouter& Router = CUIInputRouter::Get();
	f32_t fMouseX = 0.f, fMouseY = 0.f;
	const bool_t bHasMouse = Router.Get_MousePosition(REF_WIDTH, REF_HEIGHT, fMouseX, fMouseY);
	const bool_t bOverScene = bHasMouse &&
		fMouseX < PANEL_LEFT_EDGE && fMouseY < BOTTOM_BAR_TOP_EDGE;

	if (m_bOrbitDragging)
	{
		if (!Router.Is_LeftDown())
		{
			m_bOrbitDragging = false;
		}
		else if (bHasMouse)
		{
			m_fOrbitYaw -= (fMouseX - m_fLastMouseX) * 0.4f;
			m_fOrbitPitch = std::clamp(
				m_fOrbitPitch + (fMouseY - m_fLastMouseY) * 0.25f,
				ORBIT_MIN_PITCH, ORBIT_MAX_PITCH);
		}
	}
	else if (bOverScene && Router.Is_LeftClickEdge())
	{
		m_bOrbitDragging = true;
	}
	m_fLastMouseX = fMouseX;
	m_fLastMouseY = fMouseY;

	const int32_t iWheel = CGameInstance::Get().Get_DIMouseMove(DIMM::WHEEL);
	if (0 != iWheel)
	{
		m_fOrbitDistance = std::clamp(
			m_fOrbitDistance - static_cast<f32_t>(iWheel) * 0.0025f,
			ORBIT_MIN_DISTANCE, ORBIT_MAX_DISTANCE);
	}
}

void Client::CCustomizingView::Update_Tabs()
{
	for (int32_t i = 0; i < static_cast<int32_t>(std::size(LABEL_TABS)); ++i)
	{
		const string strBgId = "CC_Tab" + std::to_string(i) + "_Bg";
		const string strGlowId = "CC_Tab" + std::to_string(i) + "_Glow";
		const string strIconId = "CC_Tab" + std::to_string(i) + "_Icon";
		const bool_t bSelected = i == m_iSelectedTab;
		const bool_t bSupported = FACE_TAB_INDEX == i;

		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!Get_SlotRect(strBgId.c_str(), fX, fY, fWidth, fHeight))
			continue;
		const bool_t bHovered = bSupported && Is_Hovered(fX, fY, fWidth, fHeight);

		m_pView->Set_SlotVisible(strBgId, true);
		m_pView->Set_SlotTexture(strBgId,
			bSelected ? ASSET_TAB_BG_SELECTED : ASSET_TAB_BG);
		m_pView->Set_SlotVisible(strGlowId, bHovered && !bSelected);
		m_pView->Set_SlotVisible(strIconId, true);
		/* A tab whose data slice does not exist yet is dimmed instead of pretending to be
		clickable -- the same honest state the retail disabled frame carries. */
		const float4_t vTint = bSupported ?
			float4_t(1.f, 1.f, 1.f, 1.f) : float4_t(0.45f, 0.45f, 0.45f, 1.f);
		m_pView->Set_SlotTint(strBgId, vTint);
		m_pView->Set_SlotTint(strIconId, vTint);

		if (bHovered && Is_Clicked(fX, fY, fWidth, fHeight))
		{
			CMainApp::Play_UIButtonClickSound();
			m_iSelectedTab = i;
		}
	}

	for (const char_t* pSlotId : { "CC_TopBannerA", "CC_TopBannerB", "CC_TopGuideBg",
		"CC_RightBg", "CC_BottomBg", "CC_GuideRotateIcon", "CC_GuideZoomIcon" })
	{
		m_pView->Set_SlotVisible(pSlotId, true);
	}
}

void Client::CCustomizingView::Apply_SliderVisibility()
{
	for (const FACE_SLIDER_ROW& Row : FACE_SLIDER_ROWS)
	{
		const bool_t bVisible = FACE_TAB_INDEX == m_iSelectedTab && Row.iPart == m_iSelectedPart;
		m_pView->Set_SlotVisible(Slider_TrackSlotId(Row.pSliderId), bVisible);
		m_pView->Set_SlotVisible(Slider_ThumbSlotId(Row.pSliderId), bVisible);
	}
}

void Client::CCustomizingView::Update_FaceTab(const shared_ptr<CCharacter>& pCharacter)
{
	const bool_t bFaceTab = FACE_TAB_INDEX == m_iSelectedTab;
	m_bHasFaceSliders = nullptr != pCharacter && pCharacter->Has_FaceSliders();
	m_pView->Set_SlotVisible("CC_FoldDetail_Bg", bFaceTab);
	m_pView->Set_SlotVisible("CC_FoldDetail_Arrow", bFaceTab);
	m_pView->Set_SlotVisible("CC_DetailDivision", bFaceTab);
	m_pView->Set_SlotVisible("CC_FaceRandomBtn", bFaceTab);
	m_pView->Set_SlotVisible("CC_FaceResetBtn", bFaceTab);
	for (int32_t i = 0; i < FACE_PART_COUNT; ++i)
	{
		const string strBgId = "CC_FaceSub" + std::to_string(i) + "_Bg";
		const string strSelectedId = "CC_FaceSub" + std::to_string(i) + "_Selected";
		m_pView->Set_SlotVisible(strBgId, bFaceTab);
		m_pView->Set_SlotVisible(strSelectedId, bFaceTab && i == m_iSelectedPart);
		if (!bFaceTab)
			continue;

		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!Get_SlotRect(strBgId.c_str(), fX, fY, fWidth, fHeight))
			continue;
		const bool_t bHovered = Is_Hovered(fX, fY, fWidth, fHeight);
		m_pView->Set_SlotTexture(strBgId, bHovered ? ASSET_SUBTAB_OVER : ASSET_SUBTAB_NORMAL);
		if (bHovered && Is_Clicked(fX, fY, fWidth, fHeight))
		{
			CMainApp::Play_UIButtonClickSound();
			m_iSelectedPart = i;
			m_iDraggingRow = -1;
		}
	}

	Apply_SliderVisibility();
	if (!bFaceTab)
	{
		m_iDraggingRow = -1;
		return;
	}

	CUIInputRouter& Router = CUIInputRouter::Get();
	if (!Router.Is_LeftDown())
		m_iDraggingRow = -1;

	f32_t fMouseX = 0.f, fMouseY = 0.f;
	const bool_t bHasMouse = Router.Get_MousePosition(REF_WIDTH, REF_HEIGHT, fMouseX, fMouseY);

	for (int32_t iRow = 0; iRow < static_cast<int32_t>(std::size(FACE_SLIDER_ROWS)); ++iRow)
	{
		const FACE_SLIDER_ROW& Row = FACE_SLIDER_ROWS[iRow];
		if (Row.iPart != m_iSelectedPart)
			continue;

		f32_t fTrackX = 0.f, fTrackY = 0.f, fTrackWidth = 0.f, fTrackHeight = 0.f;
		f32_t fThumbX = 0.f, fThumbY = 0.f, fThumbWidth = 0.f, fThumbHeight = 0.f;
		const string strTrackId = Slider_TrackSlotId(Row.pSliderId);
		const string strThumbId = Slider_ThumbSlotId(Row.pSliderId);
		if (!Get_SlotRect(strTrackId.c_str(), fTrackX, fTrackY, fTrackWidth, fTrackHeight) ||
			!Get_SlotRect(strThumbId.c_str(), fThumbX, fThumbY, fThumbWidth, fThumbHeight))
		{
			continue;
		}

		const int32_t iSlider = nullptr != pCharacter ?
			Find_SliderIndex(*pCharacter, Row.pSliderId) : -1;
		f32_t fWeight = 0.5f;
		if (iSlider >= 0)
			fWeight = pCharacter->Get_FaceCustomize().Get_Weight(static_cast<size_t>(iSlider));

		/* The whole bar accepts the press, not just the thumb, matching the retail slider. */
		const f32_t fHitY = fThumbY - 4.f;
		const f32_t fHitHeight = fThumbHeight + 8.f;
		const bool_t bHovered = Is_Hovered(fTrackX, fHitY, fTrackWidth, fHitHeight);
		if (iSlider >= 0 && bHovered && Router.Is_LeftClickEdge())
			m_iDraggingRow = iRow;

		if (iSlider >= 0 && m_iDraggingRow == iRow && bHasMouse && fTrackWidth > 0.f)
		{
			fWeight = std::clamp((fMouseX - fTrackX) / fTrackWidth, 0.f, 1.f);
			pCharacter->Set_FaceSliderWeight(static_cast<size_t>(iSlider), fWeight);
		}

		m_pView->Set_SlotTexture(strThumbId,
			(bHovered || m_iDraggingRow == iRow) ? ASSET_THUMB_OVER : ASSET_THUMB_NORMAL);
		m_pView->Set_SlotPosition(strThumbId,
			fTrackX + fWeight * fTrackWidth - fThumbWidth * 0.5f,
			fThumbY);
		m_pView->Set_SlotTint(strTrackId, iSlider >= 0 ?
			float4_t(1.f, 1.f, 1.f, 1.f) : float4_t(0.4f, 0.4f, 0.4f, 1.f));
		m_pView->Set_SlotTint(strThumbId, iSlider >= 0 ?
			float4_t(1.f, 1.f, 1.f, 1.f) : float4_t(0.4f, 0.4f, 0.4f, 1.f));
	}
}

void Client::CCustomizingView::Update_Buttons(const shared_ptr<CCharacter>& pCharacter)
{
	struct TEXT_BUTTON
	{
		const char_t* pSlotId;
		const char_t* pIdle;
		const char_t* pHover;
	};
	constexpr TEXT_BUTTON BUTTONS[] = {
		{ "CC_FaceRandomBtn", ASSET_BUTTON_NORMAL, ASSET_BUTTON_OVER },
		{ "CC_FaceResetBtn", ASSET_BUTTON_NORMAL, ASSET_BUTTON_OVER },
		{ "CC_CreateBtn", ASSET_BUTTON_NORMAL, ASSET_BUTTON_OVER },
		{ "CC_BackIcon", ASSET_BACK_ICON, ASSET_BACK_ICON_OVER },
		{ "CC_ResetAllIcon", ASSET_RESET_ICON, ASSET_RESET_ICON_OVER },
	};

	m_pView->Set_SlotVisible("CC_CreateBtn", true);
	m_pView->Set_SlotVisible("CC_BackIcon", true);
	m_pView->Set_SlotVisible("CC_ResetAllIcon", true);

	const auto Fn_RandomizeFace = [this, &pCharacter]()
	{
		if (nullptr == pCharacter)
			return;
		std::uniform_real_distribution<f32_t> Distribution(0.15f, 0.85f);
		const size_t iCount = pCharacter->Get_FaceCustomize().Get_SliderCount();
		for (size_t i = 0; i < iCount; ++i)
			pCharacter->Set_FaceSliderWeight(i, Distribution(m_Random));
	};

	for (const TEXT_BUTTON& Button : BUTTONS)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!Get_SlotRect(Button.pSlotId, fX, fY, fWidth, fHeight))
			continue;
		const bool_t bHovered = Is_Hovered(fX, fY, fWidth, fHeight);
		m_pView->Set_SlotTexture(Button.pSlotId, bHovered ? Button.pHover : Button.pIdle);
		if (!bHovered || !Is_Clicked(fX, fY, fWidth, fHeight))
			continue;

		CMainApp::Play_UIButtonClickSound();
		if (0 == std::strcmp(Button.pSlotId, "CC_FaceRandomBtn"))
			Fn_RandomizeFace();
		else if (0 == std::strcmp(Button.pSlotId, "CC_FaceResetBtn") ||
			0 == std::strcmp(Button.pSlotId, "CC_ResetAllIcon"))
		{
			if (nullptr != pCharacter)
				pCharacter->Reset_FaceSliders();
		}
		else if (0 == std::strcmp(Button.pSlotId, "CC_CreateBtn"))
			m_bDecideRequested = true;
		else if (0 == std::strcmp(Button.pSlotId, "CC_BackIcon"))
			m_bBackRequested = true;
	}
}

void Client::CCustomizingView::Render_Text()
{
	if (!m_bOpen || nullptr == m_pView)
		return;

	const float2_t vViewportSize = CGameInstance::Get().Get_ViewportSize();
	const f32_t fScaleX = vViewportSize.x / REF_WIDTH;
	const f32_t fScaleY = vViewportSize.y / REF_HEIGHT;
	const f32_t fUiScale = (std::min)(fScaleX, fScaleY);

	/* fSize is a reference-resolution pixel height; Draw_Text scales relative to the font's
	own measured height, the same way every other runtime screen's text pass does. */
	const auto Fn_Draw = [&](const wstring& strFont, f32_t fX, f32_t fY, f32_t fSize,
		const fvector_t& vColor, const wchar_t* pText, const float2_t& vPivot)
	{
		if (nullptr == pText || L'\0' == pText[0])
			return;
		const float2_t vMeasured = CGameInstance::Get().Measure_Text(strFont, pText);
		if (vMeasured.y <= 0.f)
			return;
		const f32_t fScale = (fSize / vMeasured.y) * fUiScale;
		CGameInstance::Get().Draw_Text(strFont, pText,
			float2_t(fX * fScaleX, fY * fScaleY), vColor, 0.f, vPivot, fScale);
	};

	const fvector_t vSectionColor = XMVectorSet(144.f / 255.f, 175.f / 255.f, 191.f / 255.f, 1.f);
	const fvector_t vRowColor = XMVectorSet(200.f / 255.f, 208.f / 255.f, 214.f / 255.f, 1.f);
	const fvector_t vDescColor = XMVectorSet(1.f, 1.f, 1.f, 1.f);
	const fvector_t vDimColor = XMVectorSet(0.45f, 0.45f, 0.45f, 1.f);

	/* Tab captions: CharCustom_Right_TabListRenderer's textField is $YoonGasiIIM 16 at 1080p. */
	for (int32_t i = 0; i < static_cast<int32_t>(std::size(LABEL_TABS)); ++i)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!Get_SlotRect(("CC_Tab" + std::to_string(i) + "_Bg").c_str(),
			fX, fY, fWidth, fHeight))
		{
			continue;
		}
		Fn_Draw(TEXT("Font_YoonGasiIIM"), fX + fWidth * 0.5f, fY + fHeight - 6.f, 11.f,
			FACE_TAB_INDEX == i ? vDescColor : vDimColor, LABEL_TABS[i], float2_t(0.5f, 0.5f));
	}

	f32_t fGuideX = 0.f, fGuideY = 0.f, fGuideWidth = 0.f, fGuideHeight = 0.f;
	if (Get_SlotRect("CC_TopGuideBg", fGuideX, fGuideY, fGuideWidth, fGuideHeight))
	{
		Fn_Draw(TEXT("Font_YG760"), fGuideX + fGuideWidth * 0.5f,
			fGuideY + fGuideHeight * 0.5f, 12.f,
			XMVectorSet(1.f, 210.f / 255.f, 0.f, 1.f), LABEL_TOP_GUIDE, float2_t(0.5f, 0.5f));
	}

	f32_t fFoldX = 0.f, fFoldY = 0.f, fFoldWidth = 0.f, fFoldHeight = 0.f;
	if (FACE_TAB_INDEX == m_iSelectedTab &&
		Get_SlotRect("CC_FoldDetail_Bg", fFoldX, fFoldY, fFoldWidth, fFoldHeight))
	{
		Fn_Draw(TEXT("Font_YG760"), fFoldX + 12.f, fFoldY + fFoldHeight * 0.5f, 11.f,
			vDescColor, LABEL_FOLD_DETAIL, float2_t(0.f, 0.5f));
	}

	if (FACE_TAB_INDEX != m_iSelectedTab)
	{
		f32_t fPanelX = 0.f, fPanelY = 0.f, fPanelWidth = 0.f, fPanelHeight = 0.f;
		if (Get_SlotRect("CC_RightBg", fPanelX, fPanelY, fPanelWidth, fPanelHeight))
		{
			Fn_Draw(TEXT("Font_YG760"), fPanelX + fPanelWidth * 0.5f, 200.f, 12.f,
				vDimColor, LABEL_NOT_READY, float2_t(0.5f, 0.5f));
		}
	}
	else
	{
		f32_t fDivX = 0.f, fDivY = 0.f, fDivWidth = 0.f, fDivHeight = 0.f;
		if (Get_SlotRect("CC_DetailDivision", fDivX, fDivY, fDivWidth, fDivHeight))
		{
			Fn_Draw(TEXT("Font_YG760"), fDivX, fDivY - 14.f, 11.f,
				m_bHasFaceSliders ? vDescColor : vDimColor,
				m_bHasFaceSliders ? LABEL_DETAIL_DESC : LABEL_NO_SLIDERS,
				float2_t(0.f, 0.5f));
		}

		for (int32_t i = 0; i < FACE_PART_COUNT; ++i)
		{
			f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
			if (!Get_SlotRect(("CC_FaceSub" + std::to_string(i) + "_Bg").c_str(),
				fX, fY, fWidth, fHeight))
			{
				continue;
			}
			Fn_Draw(TEXT("Font_YG760"), fX + fWidth * 0.5f, fY + fHeight * 0.5f, 11.f,
				i == m_iSelectedPart ? vDescColor : vRowColor,
				LABEL_PARTS[i], float2_t(0.5f, 0.5f));
		}

		for (const FACE_PART_HEADER& Header : FACE_PART_HEADERS)
		{
			if (Header.iPart != m_iSelectedPart)
				continue;
			Fn_Draw(TEXT("Font_YoonGasiIIM"), PART_ORIGIN_X + 2.f * PART_SCALE,
				PART_ORIGIN_Y + (Header.fPartY + 10.f) * PART_SCALE, 12.f,
				vSectionColor, Header.pLabel, float2_t(0.f, 0.5f));
		}
		for (const FACE_SLIDER_ROW& Row : FACE_SLIDER_ROWS)
		{
			if (Row.iPart != m_iSelectedPart)
				continue;
			Fn_Draw(TEXT("Font_YG760"), PART_ORIGIN_X + 12.f * PART_SCALE,
				PART_ORIGIN_Y + (Row.fPartY + 14.f) * PART_SCALE, 11.f,
				vRowColor, Row.pLabel, float2_t(0.f, 0.5f));
		}

		struct PANEL_BUTTON_LABEL { const char_t* pSlotId; const wchar_t* pLabel; };
		constexpr PANEL_BUTTON_LABEL PANEL_BUTTONS[] = {
			{ "CC_FaceRandomBtn", LABEL_FACE_RANDOM },
			{ "CC_FaceResetBtn", LABEL_FACE_RESET },
		};
		for (const PANEL_BUTTON_LABEL& Button : PANEL_BUTTONS)
		{
			f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
			if (!Get_SlotRect(Button.pSlotId, fX, fY, fWidth, fHeight))
				continue;
			Fn_Draw(TEXT("Font_YG760"), fX + fWidth * 0.5f, fY + fHeight * 0.5f, 11.f,
				vDescColor, Button.pLabel, float2_t(0.5f, 0.5f));
		}
	}

	f32_t fCreateX = 0.f, fCreateY = 0.f, fCreateWidth = 0.f, fCreateHeight = 0.f;
	if (Get_SlotRect("CC_CreateBtn", fCreateX, fCreateY, fCreateWidth, fCreateHeight))
	{
		Fn_Draw(TEXT("Font_YoonGasiIIM"), fCreateX + fCreateWidth * 0.5f,
			fCreateY + fCreateHeight * 0.5f, 15.f, vDescColor, LABEL_DECIDE,
			float2_t(0.5f, 0.5f));
	}

	struct ICON_LABEL { const char_t* pSlotId; const wchar_t* pLabel; };
	constexpr ICON_LABEL ICON_LABELS[] = {
		{ "CC_BackIcon", LABEL_BACK },
		{ "CC_ResetAllIcon", LABEL_RESET_ALL },
	};
	for (const ICON_LABEL& Icon : ICON_LABELS)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!Get_SlotRect(Icon.pSlotId, fX, fY, fWidth, fHeight))
			continue;
		Fn_Draw(TEXT("Font_YG760"), fX + fWidth * 0.5f, fY + fHeight + 8.f, 11.f,
			vDescColor, Icon.pLabel, float2_t(0.5f, 0.5f));
	}

	struct GUIDE_LABEL { const char_t* pSlotId; const wchar_t* pLabel; };
	constexpr GUIDE_LABEL GUIDE_LABELS[] = {
		{ "CC_GuideRotateIcon", LABEL_GUIDE_ROTATE },
		{ "CC_GuideZoomIcon", LABEL_GUIDE_ZOOM },
	};
	for (const GUIDE_LABEL& Guide : GUIDE_LABELS)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!Get_SlotRect(Guide.pSlotId, fX, fY, fWidth, fHeight))
			continue;
		Fn_Draw(TEXT("Font_YG760"), fX + fWidth + 10.f, fY + fHeight * 0.5f, 12.f,
			vRowColor, Guide.pLabel, float2_t(0.f, 0.5f));
	}
}
