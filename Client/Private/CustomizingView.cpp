#include "CustomizingView.h"

#include "Character.h"
#include "FaceCustomizeApplier.h"
#include "GameInstance.h"
#include "MainApp.h"
#include "Model.h"
#include "Transform.h"
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
	constexpr const wchar_t* LABEL_FOLD_DETAIL = L"\xC5BC\xAD74 \xC138\xBD80 \xC124\xC815";
	constexpr const wchar_t* LABEL_FOLD_DEFAULT = L"\xAE30\xBCF8";
	/* sys.pccreate.customizing_label_face_desc / _face_form, read out of
	EFTable_GameMsg.db in data2.lpk. */
	constexpr const wchar_t* LABEL_FACE_DESC = L"\xC5BC\xAD74 \xD615\xD0DC \xBCC0\xACBD";
	constexpr const wchar_t* LABEL_FACE_FORM = L"\xAE30\xBCF8 \xC5BC\xAD74";
	/* pccreate.customizing_label_create_change_info, in the three runs its own markup has. */
	constexpr const wchar_t* LABEL_BOTTOM_GUIDE_HEAD =
		L"\xCE90\xB9AD\xD130 \xC0DD\xC131 \xD6C4 ";
	constexpr const wchar_t* LABEL_BOTTOM_GUIDE_ACCENT =
		L"24\xC2DC\xAC04 \xC774\xB0B4 \xC678\xD615\xC744 \xC790\xC720\xB86D\xAC8C \xBCC0\xACBD";
	constexpr const wchar_t* LABEL_BOTTOM_GUIDE_TAIL =
		L" \xAC00\xB2A5\xD569\xB2C8\xB2E4.";
	/* pccreate.customizing_title on rightTitleLabel. */
	constexpr const wchar_t* LABEL_RIGHT_TITLE = L"\xCE90\xB9AD\xD130 \xC0DD\xC131";
	constexpr const wchar_t* LABEL_DECIDE = L"\xC678\xD615 \xACB0\xC815";
	constexpr const wchar_t* LABEL_BACK = L"\xB4A4\xB85C\xAC00\xAE30";
	constexpr const wchar_t* LABEL_RESET_ALL = L"\xC804\xCCB4 \xCD08\xAE30\xD654";
	constexpr const wchar_t* LABEL_FACE_RANDOM = L"\xBB34\xC791\xC704 \xC5BC\xAD74";
	constexpr const wchar_t* LABEL_FACE_RESET = L"\xC5BC\xAD74 \xCD08\xAE30\xD654";
	constexpr const wchar_t* LABEL_GUIDE_ROTATE = L"\xCE90\xB9AD\xD130 \xD68C\xC804";
	/* sys.pccreate.customizing_guide_eyetracking: the middle of the three guides.
	Character creation shows all three; the salon frame shows fewer. */
	constexpr const wchar_t* LABEL_GUIDE_EYETRACK =
		L"\xB9C8\xC6B0\xC2A4 \xBC14\xB77C\xBCF4\xAE30";
	/* Left column captions, at the y the movie places each list at. */
	constexpr const wchar_t* LABEL_LEFT_COSTUME =
		L"\xC758\xC0C1 \xC785\xC5B4\xBCF4\xAE30";		/* costume try-on */
	constexpr const wchar_t* LABEL_LEFT_ACTION = L"\xC561\xC158";	/* action */
	constexpr const wchar_t* LABEL_LEFT_SAVE =
		L"\xCEE4\xC2A4\xD130\xB9C8\xC774\xC9D5 \xC800\xC7A5/\xBD88\xB7EC\xC624\xAE30";
	constexpr const wchar_t* LABEL_LEFT_RESET = L"\xCD08\xAE30\xD654";	/* reset */
	constexpr const wchar_t* LABEL_SAVE = L"\xC800\xC7A5";
	constexpr const wchar_t* LABEL_LOAD = L"\xBD88\xB7EC\xC624\xAE30";
	/* Left panel geometry from CharCustom_LeftPanel: labels at (35,50)/(35,171), the bottom
	page at (33,426) with its own save caption at (2,127). Values are gfx units. */
	constexpr f32_t LEFT_LABEL_X = 35.f;
	constexpr f32_t LEFT_COSTUME_Y = 50.f;
	constexpr f32_t LEFT_ACTION_Y = 171.f;
	/* The background picker is hidden during character creation, and
	CharacterCustomizingLeftPanel then slides bottomPage up to the caption's own y. */
	constexpr f32_t BOTTOM_PAGE_Y = 296.f;
	constexpr f32_t LEFT_SAVE_Y = BOTTOM_PAGE_Y + 127.f;
	/* bottomPage @ (33,426): the recommended-style caption sits at its (2,2). */
	/* bottomPage @ (33,426): the recommended-style caption sits at its (2,2). */
	constexpr f32_t GFX_TO_REF = 2.f / 3.f;
	/* CharCustom_Right_TabFaceDefaultContent is a 390x470 box, so expanding the first
	category pushes the second one down by its whole height. */
	constexpr f32_t FACE_DEFAULT_SECTION_HEIGHT = 470.f * GFX_TO_REF;
	constexpr int32_t FACE_PRESET_COUNT = 25;
	/* Retail point sizes, converted the same way every position in this file is. */
	constexpr f32_t SIZE_RIGHT_TITLE = 28.f * GFX_TO_REF;
	constexpr f32_t SIZE_BOTTOM_GUIDE = 16.f * GFX_TO_REF;
	constexpr f32_t SIZE_MOUSE_GUIDE = 14.f * GFX_TO_REF;
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

	/* Framing lifted from the retail class-select stage. Three camera actors stand at the
	character's spot there: a face camera 0.92 m out at 0.83 m looking 3.9 degrees up, a
	full-body camera 5.45 m out at 1.01 m looking 2.5 degrees up with a 40 degree FOV, and a
	wide stage camera at 12 m the customizing screen does not use. Distances are the retail
	centimetre values divided by 100, the same conversion the map import pipeline applies. */
	constexpr f32_t FULLBODY_DISTANCE = 2.50f;
	constexpr f32_t FULLBODY_LOOK_HEIGHT = 0.62f;
	constexpr f32_t FULLBODY_PITCH = 2.5f;
	constexpr f32_t FACE_DISTANCE = 0.65f;
	constexpr f32_t FACE_PITCH = 3.9f;
	/* The four class bodies are not the same height, so the face framing is measured off the
	model instead of being one constant. These are the rig's own eye bones, whose combined
	matrix already carries the admission scale each class is imported with:

	    LanceMaster 1.040   DimensionMaster 1.010   Warlord 0.917   Artist 0.862   (metres)

	FACE_DISTANCE was set against the tallest of them, so the distance travels with the
	measured height and a shorter class's head fills the same share of the frame. */
	constexpr const char_t* EYE_BONE_NAMES[] = { "b_fc_l_eye_ani", "b_fc_r_eye_ani" };
	constexpr f32_t REFERENCE_EYE_HEIGHT = 1.040f;
	constexpr f32_t FACE_LOOK_HEIGHT_FALLBACK = REFERENCE_EYE_HEIGHT;
	/* The retail full-body camera carries this on its own FOVAngle property. */
	constexpr f32_t CUSTOMIZING_FOV_Y_DEGREES = 40.f;
	/* How fast the two-state wheel toggle settles. */
	constexpr f32_t ZOOM_RESPONSE = 9.f;
	/* The two panels and the bottom bar own the pointer; a drag started between them orbits.
	The left panel is 402 gfx units wide, the right one starts at 1475. */
	constexpr f32_t PANEL_LEFT_EDGE = 983.f;
	constexpr f32_t PANEL_RIGHT_OF_LEFT_EDGE = 268.f;
	constexpr f32_t BOTTOM_BAR_TOP_EDGE = 640.f;
	/* Fraction of the half-width the subject is shifted left by. */
	f32_t Subject_ScreenShiftFraction()
	{
		constexpr f32_t HALF_WIDTH = REF_WIDTH * 0.5f;
		const f32_t fFreeCentre =
			(PANEL_RIGHT_OF_LEFT_EDGE + PANEL_LEFT_EDGE) * 0.5f;
		return (HALF_WIDTH - fFreeCentre) / HALF_WIDTH;
	}

	f32_t Camera_HorizontalTangent()
	{
		const float2_t vViewport = CGameInstance::Get().Get_ViewportSize();
		const f32_t fAspect = (vViewport.x > 0.f && vViewport.y > 0.f) ?
			(vViewport.x / vViewport.y) : (16.f / 9.f);
		return std::tan(XMConvertToRadians(CUSTOMIZING_FOV_Y_DEGREES * 0.5f)) * fAspect;
	}

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
	m_fOrbitYaw = 0.f;
	m_fZoomTarget = 0.f;
	m_fZoomBlend = 0.f;
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

f32_t Client::CCustomizingView::Get_FieldOfViewDegrees() const
{
	return CUSTOMIZING_FOV_Y_DEGREES;
}

f32_t Client::CCustomizingView::Get_FaceLookHeight() const
{
	return m_fMeasuredEyeHeight > 0.f ? m_fMeasuredEyeHeight : FACE_LOOK_HEIGHT_FALLBACK;
}

f32_t Client::CCustomizingView::Get_Distance() const
{
	const f32_t fFaceDistance =
		FACE_DISTANCE * (Get_FaceLookHeight() / REFERENCE_EYE_HEIGHT);
	return FULLBODY_DISTANCE + (fFaceDistance - FULLBODY_DISTANCE) * m_fZoomBlend;
}

f32_t Client::CCustomizingView::Get_Pitch() const
{
	return FULLBODY_PITCH + (FACE_PITCH - FULLBODY_PITCH) * m_fZoomBlend;
}

float3_t Client::CCustomizingView::Get_LateralOffset() const
{
	/* Shifting the eye and the look point by the same sideways vector moves the subject across
	the screen without turning the camera. The camera moves right, so the character reads as
	left of centre. */
	const f32_t fYaw = XMConvertToRadians(m_fCharacterYawDegrees + m_fOrbitYaw);
	const f32_t fLateral =
		Subject_ScreenShiftFraction() * Camera_HorizontalTangent() * Get_Distance();
	return float3_t(-std::cos(fYaw) * fLateral, 0.f, std::sin(fYaw) * fLateral);
}

f32_t Client::CCustomizingView::Get_LookHeight() const
{
	return FULLBODY_LOOK_HEIGHT +
		(Get_FaceLookHeight() - FULLBODY_LOOK_HEIGHT) * m_fZoomBlend;
}

float3_t Client::CCustomizingView::Get_CameraPositionOffset() const
{
	/* Yaw 0 stands the camera directly in front of the character's own facing. */
	const f32_t fYaw = XMConvertToRadians(m_fCharacterYawDegrees + m_fOrbitYaw);
	/* The retail pitch is the angle the camera looks UP at the character, so the eye sits
	that much below the point it aims at. */
	const f32_t fPitch = XMConvertToRadians(Get_Pitch());
	const f32_t fDistance = Get_Distance();
	const f32_t fHorizontal = fDistance * std::cos(fPitch);
	const float3_t vLateral = Get_LateralOffset();
	return float3_t(
		fHorizontal * std::sin(fYaw) + vLateral.x,
		Get_LookHeight() - fDistance * std::sin(fPitch),
		fHorizontal * std::cos(fYaw) + vLateral.z);
}

float3_t Client::CCustomizingView::Get_CameraLookOffset() const
{
	const float3_t vLateral = Get_LateralOffset();
	return float3_t(vLateral.x, Get_LookHeight(), vLateral.z);
}

void Client::CCustomizingView::Update(
	const f32_t fTimeDelta, const shared_ptr<CCharacter>& pCharacter)
{
	if (!m_bOpen || nullptr == m_pView)
		return;

	/* The turntable is measured from the character's own facing, so it has to be read every
	frame: the replicated transform is what decides which way the model is looking. */
	if (nullptr != pCharacter && nullptr != pCharacter->Get_Transform())
	{
		float3_t vLook{};
		XMStoreFloat3(&vLook,
			pCharacter->Get_Transform()->Get_State(Engine::STATE::LOOK));
		if (std::isfinite(vLook.x) && std::isfinite(vLook.z) &&
			(std::fabs(vLook.x) > 1e-4f || std::fabs(vLook.z) > 1e-4f))
		{
			m_fCharacterYawDegrees =
				XMConvertToDegrees(std::atan2(vLook.x, vLook.z));
		}
	}

	Update_SubjectMetrics(pCharacter);
	Apply_ListIcons(pCharacter);
	Update_Orbit(fTimeDelta);
	Update_Tabs();
	Update_FaceTab(pCharacter);
	Update_Buttons(pCharacter);
}

void Client::CCustomizingView::Update_Orbit(const f32_t fTimeDelta)
{
	CUIInputRouter& Router = CUIInputRouter::Get();
	f32_t fMouseX = 0.f, fMouseY = 0.f;
	const bool_t bHasMouse = Router.Get_MousePosition(REF_WIDTH, REF_HEIGHT, fMouseX, fMouseY);
	const bool_t bOverScene = bHasMouse &&
		fMouseX > PANEL_RIGHT_OF_LEFT_EDGE && fMouseX < PANEL_LEFT_EDGE &&
		fMouseY < BOTTOM_BAR_TOP_EDGE;

	if (m_bOrbitDragging)
	{
		if (!Router.Is_LeftDown())
		{
			m_bOrbitDragging = false;
		}
		else if (bHasMouse)
		{
			/* Yaw only: the retail guide offers rotate and zoom, no free pitch. */
			m_fOrbitYaw -= (fMouseX - m_fLastMouseX) * 0.4f;
		}
	}
	else if (bOverScene && Router.Is_LeftClickEdge())
	{
		m_bOrbitDragging = true;
	}
	m_fLastMouseX = fMouseX;
	m_fLastMouseY = fMouseY;

	/* One wheel notch swaps the whole framing in the original, so this is a toggle rather
	than a continuous distance. */
	const int32_t iWheel = CGameInstance::Get().Get_DIMouseMove(DIMM::WHEEL);
	if (iWheel > 0)
		m_fZoomTarget = 1.f;
	else if (iWheel < 0)
		m_fZoomTarget = 0.f;
	const f32_t fStep = std::clamp((max)(0.f, fTimeDelta), 0.f, 0.1f);
	m_fZoomBlend += (m_fZoomTarget - m_fZoomBlend) *
		(1.f - std::exp(-ZOOM_RESPONSE * fStep));
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

	for (const char_t* pSlotId : { "CC_LeftBg", "CC_RightBg", "CC_BottomBg",
		"CC_GuideRotateIcon", "CC_GuideEyeTrackIcon", "CC_GuideZoomIcon" })
	{
		m_pView->Set_SlotVisible(pSlotId, true);
	}
	/* CharCustom_TopPanel carries the paid appearance-change notice, which the creation flow
	never shows. */
	for (const char_t* pSlotId : { "CC_TopBannerA", "CC_TopBannerB", "CC_TopGuideBg" })
		m_pView->Set_SlotVisible(pSlotId, false);
	/* The left column's rolling lists: each CharCustom_LeftRenderer cell is its own plate
	under the shared V2Slot_border frame, and the panel art behind them is a translucent
	black sheet that reads as nothing on its own. */
	for (int32_t i = 0; i < 5; ++i)
	{
		for (const string& strId : { "CC_LeftDress" + std::to_string(i),
			"CC_LeftAction" + std::to_string(i) })
		{
			m_pView->Set_SlotVisible(strId, true);
			m_pView->Set_SlotVisible(strId + "_Plate", true);
		}
	}
	for (int32_t i = 0; i < 6; ++i)
	{
		const string strId = "CC_LeftSave" + std::to_string(i);
		m_pView->Set_SlotVisible(strId, true);
		m_pView->Set_SlotVisible(strId + "_Plate", true);
	}
	for (const char_t* pSlotId : { "CC_LeftSaveBtn", "CC_LeftLoadBtn" })
		m_pView->Set_SlotVisible(pSlotId, true);
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

void Client::CCustomizingView::Apply_ListIcons(const shared_ptr<CCharacter>& pCharacter)
{
	if (nullptr == pCharacter || nullptr == pCharacter->Get_Spec())
		return;
	const char_t* pAssetName = pCharacter->Get_Spec()->pAssetName;
	if (nullptr == pAssetName || m_strIconClassAssetId == pAssetName)
		return;
	m_strIconClassAssetId = pAssetName;

	if (!m_IconDocument.Load())
	{
		OutputDebugStringA(
			("[CustomizingView][Icons] " + m_IconDocument.Get_Status() + "\n").c_str());
		return;
	}
	const auto* pIcons = m_IconDocument.Find(m_strIconClassAssetId);
	if (nullptr == pIcons)
		return;

	/* A cell past the end of its list keeps the authored frame: the retail lists are
	longer than the visible rows and scroll, which this screen does not do yet. */
	const auto Fn_Fill = [&](const char_t* pPrefix, int32_t iCount,
		const std::vector<std::string>& Assets)
	{
		for (int32_t i = 0; i < iCount; ++i)
		{
			const string strId = string(pPrefix) + std::to_string(i) + "_Plate";
			m_pView->Set_SlotTexture(strId,
				static_cast<size_t>(i) < Assets.size() ? Assets[i] : string());
		}
	};
	Fn_Fill("CC_LeftAction", 5, pIcons->Actions);
	Fn_Fill("CC_FacePreset", FACE_PRESET_COUNT, pIcons->Presets);
	/* The recommended-style row stays empty: it is account content the client fetches,
	not a table, and the icon package that looked like it is the background picker. */
}

void Client::CCustomizingView::Update_SubjectMetrics(const shared_ptr<CCharacter>& pCharacter)
{
	if (nullptr == pCharacter)
		return;
	const shared_ptr<Engine::CModel> pModel = pCharacter->Get_BodyModel();
	if (nullptr == pModel)
		return;

	f32_t fSum = 0.f;
	int32_t iFound = 0;
	for (const char_t* pBoneName : EYE_BONE_NAMES)
	{
		if (!pModel->Has_Bone(pBoneName))
			continue;
		float3_t vBonePosition{};
		XMStoreFloat3(&vBonePosition, pModel->Get_BoneMatrix(pBoneName).r[3]);
		if (!std::isfinite(vBonePosition.y) || vBonePosition.y <= 0.f)
			continue;
		fSum += vBonePosition.y;
		++iFound;
	}
	if (0 == iFound)
		return;
	m_fMeasuredEyeHeight = fSum / static_cast<f32_t>(iFound);
}

void Client::CCustomizingView::Update_FaceCategoryHeader(
	const char_t* pSlotId, bool_t& isExpanded, const bool_t isVisible)
{
	if (!isVisible)
		return;
	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	if (!Get_SlotRect(pSlotId, fX, fY, fWidth, fHeight))
		return;
	if (!Is_Hovered(fX, fY, fWidth, fHeight) || !Is_Clicked(fX, fY, fWidth, fHeight))
		return;
	CMainApp::Play_UIButtonClickSound();
	isExpanded = !isExpanded;
	m_iDraggingRow = -1;
}

void Client::CCustomizingView::Capture_FaceDetailLayout()
{
	if (!m_FaceDetailAuthoredRects.empty() || nullptr == m_pView)
		return;
	std::vector<string> ids = { "CC_FoldDetail_Bg", "CC_FoldDetail_Arrow", "CC_DetailDivision" };
	for (int32_t i = 0; i < FACE_PART_COUNT; ++i)
	{
		ids.push_back("CC_FaceSub" + std::to_string(i) + "_Bg");
		ids.push_back("CC_FaceSub" + std::to_string(i) + "_Selected");
	}
	for (const FACE_SLIDER_ROW& Row : FACE_SLIDER_ROWS)
	{
		ids.push_back(Slider_TrackSlotId(Row.pSliderId));
		ids.push_back(Slider_ThumbSlotId(Row.pSliderId));
	}
	for (const string& id : ids)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (m_pView->Get_SlotRect(id, fX, fY, fWidth, fHeight))
			m_FaceDetailAuthoredRects.emplace_back(id, float2_t(fX, fY));
	}
}

void Client::CCustomizingView::Apply_FaceAccordionLayout()
{
	Capture_FaceDetailLayout();
	const f32_t fOffset =
		m_isFaceDefaultExpanded ? FACE_DEFAULT_SECTION_HEIGHT : 0.f;
	for (const auto& [id, vAuthored] : m_FaceDetailAuthoredRects)
		m_pView->Set_SlotPosition(id, vAuthored.x, vAuthored.y + fOffset);
}

void Client::CCustomizingView::Update_FaceTab(const shared_ptr<CCharacter>& pCharacter)
{
	const bool_t bFaceTab = FACE_TAB_INDEX == m_iSelectedTab;
	m_bHasFaceSliders = nullptr != pCharacter && pCharacter->Has_FaceSliders();
	Apply_FaceAccordionLayout();

	/* Two CharCustom_CategoryCheckBox headers: the preset half and the slider half. Clicking
	either folds it, and folding the first slides the second up into its place. */
	const bool_t bDefaultOpen = bFaceTab && m_isFaceDefaultExpanded;
	const bool_t bDetailOpen = bFaceTab && m_isFaceDetailExpanded;
	m_pView->Set_SlotVisible("CC_FoldDefault_Bg", bFaceTab);
	m_pView->Set_SlotVisible("CC_FoldDefault_Arrow", bFaceTab);
	m_pView->Set_SlotVisible("CC_DefaultDivision", bDefaultOpen);
	m_pView->Set_SlotVisible("CC_FaceRandomBtn", bDefaultOpen);
	m_pView->Set_SlotVisible("CC_FaceResetBtn", bDefaultOpen);
	for (int32_t i = 0; i < FACE_PRESET_COUNT; ++i)
	{
		const string strId = "CC_FacePreset" + std::to_string(i);
		m_pView->Set_SlotVisible(strId, bDefaultOpen);
		m_pView->Set_SlotVisible(strId + "_Plate", bDefaultOpen);
	}
	m_pView->Set_SlotVisible("CC_FoldDetail_Bg", bFaceTab);
	m_pView->Set_SlotVisible("CC_FoldDetail_Arrow", bFaceTab);
	m_pView->Set_SlotVisible("CC_DetailDivision", bDetailOpen);
	Update_FaceCategoryHeader("CC_FoldDefault_Bg", m_isFaceDefaultExpanded, bFaceTab);
	Update_FaceCategoryHeader("CC_FoldDetail_Bg", m_isFaceDetailExpanded, bFaceTab);

	for (int32_t i = 0; i < FACE_PART_COUNT; ++i)
	{
		const string strBgId = "CC_FaceSub" + std::to_string(i) + "_Bg";
		const string strSelectedId = "CC_FaceSub" + std::to_string(i) + "_Selected";
		m_pView->Set_SlotVisible(strBgId, bDetailOpen);
		m_pView->Set_SlotVisible(strSelectedId, bDetailOpen && i == m_iSelectedPart);
		if (!bDetailOpen)
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
	if (!bDetailOpen)
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
		{ "CC_LeftResetBtn", ASSET_BUTTON_NORMAL, ASSET_BUTTON_OVER },
		{ "CC_FaceRandomBtn", ASSET_BUTTON_NORMAL, ASSET_BUTTON_OVER },
		{ "CC_FaceResetBtn", ASSET_BUTTON_NORMAL, ASSET_BUTTON_OVER },
		{ "CC_CreateBtn", ASSET_BUTTON_NORMAL, ASSET_BUTTON_OVER },
		{ "CC_BackIcon", ASSET_BACK_ICON, ASSET_BACK_ICON_OVER },
		{ "CC_ResetAllIcon", ASSET_RESET_ICON, ASSET_RESET_ICON_OVER },
	};

	m_pView->Set_SlotVisible("CC_CreateBtn", true);
	m_pView->Set_SlotVisible("CC_LeftResetBtn", true);
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
			0 == std::strcmp(Button.pSlotId, "CC_ResetAllIcon") ||
			0 == std::strcmp(Button.pSlotId, "CC_LeftResetBtn"))
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
		const float2_t vPos(fX * fScaleX, fY * fScaleY);
		CGameInstance::Get().Draw_Text(strFont, pText,
			float2_t(vPos.x + 1.f, vPos.y + 1.f),
			XMVectorSet(0.f, 0.f, 0.f, 0.85f), 0.f, vPivot, fScale);
		CGameInstance::Get().Draw_Text(strFont, pText, vPos, vColor, 0.f, vPivot, fScale);
	};

	/* Lays consecutive coloured runs out as a single centred line: each run is measured in
	the same font at the same size, so the joins land exactly where one string would. */
	struct TEXT_RUN { const wchar_t* pText; vector_t vColor; };
	const auto Fn_DrawRuns = [&](const wstring& strFont, f32_t fCenterX, f32_t fCenterY,
		f32_t fSize, const TEXT_RUN* pRuns, size_t iNumRuns)
	{
		f32_t fTotalWidth = 0.f;
		f32_t fScale = 0.f;
		for (size_t i = 0; i < iNumRuns; ++i)
		{
			const float2_t vMeasured = CGameInstance::Get().Measure_Text(strFont, pRuns[i].pText);
			if (vMeasured.y <= 0.f)
				return;
			fScale = (fSize / vMeasured.y) * fUiScale;
			fTotalWidth += vMeasured.x * fScale;
		}
		f32_t fPenX = fCenterX * fScaleX - fTotalWidth * 0.5f;
		const f32_t fPenY = fCenterY * fScaleY;
		for (size_t i = 0; i < iNumRuns; ++i)
		{
			const float2_t vMeasured = CGameInstance::Get().Measure_Text(strFont, pRuns[i].pText);
			CGameInstance::Get().Draw_Text(strFont, pRuns[i].pText,
				float2_t(fPenX + 1.f, fPenY + 1.f), XMVectorSet(0.f, 0.f, 0.f, 0.85f),
				0.f, float2_t(0.f, 0.5f), fScale);
			CGameInstance::Get().Draw_Text(strFont, pRuns[i].pText, float2_t(fPenX, fPenY),
				pRuns[i].vColor, 0.f, float2_t(0.f, 0.5f), fScale);
			fPenX += vMeasured.x * fScale;
		}
	};

	const fvector_t vSectionColor = XMVectorSet(164.f / 255.f, 175.f / 255.f, 183.f / 255.f, 1.f);
	const fvector_t vRowColor = XMVectorSet(224.f / 255.f, 230.f / 255.f, 236.f / 255.f, 1.f);
	const fvector_t vDescColor = XMVectorSet(1.f, 1.f, 1.f, 1.f);
	const fvector_t vDimColor = XMVectorSet(0.45f, 0.45f, 0.45f, 1.f);

	/* Tab captions: CharCustom_Right_TabListRenderer keeps its textField at alpha 0 on the
	idle frame and only raises it on the over/selected frames, so only the selected tab (or
	the one under the pointer) shows a name. */
	for (int32_t i = 0; i < static_cast<int32_t>(std::size(LABEL_TABS)); ++i)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!Get_SlotRect(("CC_Tab" + std::to_string(i) + "_Bg").c_str(),
			fX, fY, fWidth, fHeight))
		{
			continue;
		}
		const bool_t bSelected = i == m_iSelectedTab;
		const bool_t bHovered = FACE_TAB_INDEX == i && Is_Hovered(fX, fY, fWidth, fHeight);
		if (!bSelected && !bHovered)
			continue;
		Fn_Draw(TEXT("Font_YoonGasiIIM"), fX + fWidth * 0.5f, fY + fHeight - 6.f, 11.f,
			FACE_TAB_INDEX == i ? vDescColor : vDimColor, LABEL_TABS[i], float2_t(0.5f, 0.5f));
	}

	/* Left column captions. The lists themselves have no data yet, so only their headings
	draw -- the panel art already carries the rest of the column. */
	Fn_Draw(TEXT("Font_YG760"), LEFT_LABEL_X * GFX_TO_REF, LEFT_COSTUME_Y * GFX_TO_REF,
		12.f, vSectionColor, LABEL_LEFT_COSTUME, float2_t(0.f, 0.f));
	Fn_Draw(TEXT("Font_YG760"), LEFT_LABEL_X * GFX_TO_REF, LEFT_ACTION_Y * GFX_TO_REF,
		12.f, vSectionColor, LABEL_LEFT_ACTION, float2_t(0.f, 0.f));
	Fn_Draw(TEXT("Font_YG760"), LEFT_LABEL_X * GFX_TO_REF, LEFT_SAVE_Y * GFX_TO_REF,
		12.f, vSectionColor, LABEL_LEFT_SAVE, float2_t(0.f, 0.f));
	{
		struct LEFT_BUTTON { const char_t* pSlotId; const wchar_t* pLabel; };
		constexpr LEFT_BUTTON LEFT_BUTTONS[] = {
			{ "CC_LeftSaveBtn", LABEL_SAVE },
			{ "CC_LeftLoadBtn", LABEL_LOAD },
		};
		for (const LEFT_BUTTON& Button : LEFT_BUTTONS)
		{
			f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
			if (!Get_SlotRect(Button.pSlotId, fX, fY, fWidth, fHeight))
				continue;
			Fn_Draw(TEXT("Font_YG760"), fX + fWidth * 0.5f, fY + fHeight * 0.5f, 11.f,
				vDescColor, Button.pLabel, float2_t(0.5f, 0.5f));
		}
	}
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (Get_SlotRect("CC_LeftResetBtn", fX, fY, fWidth, fHeight))
		{
			Fn_Draw(TEXT("Font_YG760"), fX + fWidth * 0.5f, fY + fHeight * 0.5f, 11.f,
				vDescColor, LABEL_LEFT_RESET, float2_t(0.5f, 0.5f));
		}
	}

	/* CharCustom_Bottom_CreateOutGamePage's own guide label: it sits on the bottom plate just
	above the decide button, not on the top panel. */
	f32_t fGuideX = 0.f, fGuideY = 0.f, fGuideWidth = 0.f, fGuideHeight = 0.f;
	if (Get_SlotRect("CC_BottomBg", fGuideX, fGuideY, fGuideWidth, fGuideHeight))
	{
		const fvector_t vAccent = XMVectorSet(1.f, 210.f / 255.f, 0.f, 1.f);
		const TEXT_RUN Runs[] = {
			{ LABEL_BOTTOM_GUIDE_HEAD, vDescColor },
			{ LABEL_BOTTOM_GUIDE_ACCENT, vAccent },
			{ LABEL_BOTTOM_GUIDE_TAIL, vDescColor },
		};
		Fn_DrawRuns(TEXT("Font_YG760"), fGuideX + fGuideWidth * 0.5f,
			fGuideY + fGuideHeight * 0.5f, SIZE_BOTTOM_GUIDE, Runs, std::size(Runs));
	}

	/* rightTitleLabel: centred over the right panel, 18 gfx units down from its top. */
	{
		f32_t fTitleX = 0.f, fTitleY = 0.f, fTitleWidth = 0.f, fTitleHeight = 0.f;
		if (Get_SlotRect("CC_RightBg", fTitleX, fTitleY, fTitleWidth, fTitleHeight))
		{
			Fn_Draw(TEXT("Font_YoonGasiIIM"), fTitleX + fTitleWidth * 0.5f,
				fTitleY + 18.f * GFX_TO_REF, SIZE_RIGHT_TITLE,
				XMVectorSet(1.f, 252.f / 255.f, 226.f / 255.f, 1.f),
				LABEL_RIGHT_TITLE, float2_t(0.5f, 0.f));
		}
	}

	f32_t fFoldX = 0.f, fFoldY = 0.f, fFoldWidth = 0.f, fFoldHeight = 0.f;
	if (FACE_TAB_INDEX == m_iSelectedTab &&
		Get_SlotRect("CC_FoldDefault_Bg", fFoldX, fFoldY, fFoldWidth, fFoldHeight))
	{
		Fn_Draw(TEXT("Font_YG760"), fFoldX + 12.f, fFoldY + fFoldHeight * 0.5f, 11.f,
			vDescColor, LABEL_FOLD_DEFAULT, float2_t(0.f, 0.5f));
	}
	/* The first category's own two lines sit above its preset grid: a white 14pt caption and
	a 16pt accent line, the same pair CharCustom_Right_TabFaceDefaultContent carries. */
	f32_t fDefX = 0.f, fDefY = 0.f, fDefWidth = 0.f, fDefHeight = 0.f;
	if (m_isFaceDefaultExpanded && FACE_TAB_INDEX == m_iSelectedTab &&
		Get_SlotRect("CC_DefaultDivision", fDefX, fDefY, fDefWidth, fDefHeight))
	{
		Fn_Draw(TEXT("Font_YG760"), fDefX + 2.f, fDefY - 14.f, 9.33f,
			vDescColor, LABEL_FACE_DESC, float2_t(0.f, 0.5f));
		Fn_Draw(TEXT("Font_YoonGasiIIM"), fDefX + 2.f, fDefY + 10.f, 10.67f,
			vSectionColor, LABEL_FACE_FORM, float2_t(0.f, 0.5f));
	}
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
		/* AnimatedButton_renew20_V2's own textField: $YG760 at 18 gfx px. */
		Fn_Draw(TEXT("Font_YG760"), fCreateX + fCreateWidth * 0.5f,
			fCreateY + fCreateHeight * 0.5f, 12.f, vDescColor, LABEL_DECIDE,
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
		{ "CC_GuideEyeTrackIcon", LABEL_GUIDE_EYETRACK },
		{ "CC_GuideZoomIcon", LABEL_GUIDE_ZOOM },
	};
	for (const GUIDE_LABEL& Guide : GUIDE_LABELS)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!Get_SlotRect(Guide.pSlotId, fX, fY, fWidth, fHeight))
			continue;
		Fn_Draw(TEXT("Font_YG760"), fX + fWidth + 10.f, fY + fHeight * 0.5f,
			SIZE_MOUSE_GUIDE, vRowColor, Guide.pLabel, float2_t(0.f, 0.5f));
	}
}
