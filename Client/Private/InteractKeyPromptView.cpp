#include "InteractKeyPromptView.h"

#pragma push_macro("new")
#undef new
#include <DirectXColors.h>
#pragma pop_macro("new")

#include "Character.h"
#include "GameInstance.h"
#include "MapAssetCatalog.h"
#include "UILabelFont.h"
#include "UILayoutRuntime.h"
#include "UITextOcclusion.h"
#include "WorldGameplayDocument.h"
#include "WorldPlayerNameplateView.h"

#include <cmath>

namespace
{
	/* Retail stage px (1920x1080) of MasterKeyComponent. */
	constexpr f32_t RETAIL_HEIGHT = 1080.f;
	constexpr f32_t LINE_OFFSET_ABOVE_POINT = 50.f;		// updatePos: pivot bottom-left, (0, 50)
	constexpr f32_t TEXT_PX = 14.f;						// descriptionTF stringSize
	constexpr f32_t DESC_TOP_BELOW_ICON_CENTER = 29.f;	// descriptionTF y vs the icon's centre
	constexpr f32_t ICON_W = 70.f * 0.7428589f;			// iconType_mc 70x69 at (0.7429, 0.7536)
	constexpr f32_t ICON_H = 69.f * 0.7536011f;
	constexpr f32_t KEY_H = 18.f;						// inline Shared_GlobalInputDeviceKey_G
	constexpr f32_t KEY_W = 18.f * 34.f / 35.f;
	constexpr f32_t KEY_GAP = 4.f;						// the " " between {0} and {1}
	constexpr f32_t FX_SIZE = 300.f;					// effectMc show shapes, centred
	constexpr f32_t FX_FPS = 40.f;
	constexpr int32_t FX_FRAMES = 5;
	constexpr f32_t REF_WIDTH = 1280.f;
	constexpr f32_t REF_HEIGHT = 720.f;

	const wstring_t FONT_YG760 = TEXT("Font_YG760");
	const char* const SLOTS[] = { "IKP_Fx", "IKP_Icon", "IKP_Key" };
	const char* const FX_FRAME_ART[FX_FRAMES] = {
		"UI/Interact/ShowFx_0.png", "UI/Interact/ShowFx_1.png", "UI/Interact/ShowFx_2.png",
		"UI/Interact/ShowFx_3.png", "UI/Interact/ShowFx_4.png" };

	/* GameMsg tip.name.interactionkey_godown / climb / tightrope / check, ASCII-escaped. */
	const wchar_t* Action_Text(const uint8_t iAction)
	{
		switch (iAction)
		{
		case 0: return L"\xB0B4\xB824\xAC00\xAE30";
		case 1: return L"\xC62C\xB77C\xAC00\xAE30";
		case 2: return L"\xAC74\xB108\xAC00\xAE30";
		default: return L"\xD655\xC778\xD558\xAE30";
		}
	}
	const char* Action_Icon(const uint8_t iAction)
	{
		switch (iAction)
		{
		case 0: return "UI/Interact/Icon_godown.png";
		case 1: return "UI/Interact/Icon_climb.png";
		case 2: return "UI/Interact/Icon_singleLine.png";
		default: return "UI/Interact/Icon_check.png";
		}
	}
}

void Client::CInteractKeyPromptView::Initialize(
	ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext,
	const uint32_t iOwnerLevelIndex, const char* pAreaId)
{
	m_pView = std::make_unique<CUILayoutRuntime>(pDevice, pContext, iOwnerLevelIndex,
		TEXT("Layer_UI"), L"UI/Interact/InteractKey_Layout.json");
	for (const char* pSlot : SLOTS)
		m_pView->Set_SlotVisible(pSlot, false);
	m_Triggers.clear();
	if (nullptr == pAreaId)
		return;

	CWorldGameplayDocument world;
	std::string strStatus;
	const std::filesystem::path path = CMapAssetCatalog::Get_MapDataRoot().parent_path() /
		"World" / (std::string(pAreaId) + ".viewer.world.json");
	if (!world.Load(path, pAreaId, strStatus))
	{
		OutputDebugStringA(("[InteractKeyPrompt] viewer world unavailable: " + strStatus + "\n").c_str());
		return;
	}
	for (const WORLD_GAMEPLAY_PLACEMENT& Placement : world.Get_Placements())
	{
		if (WORLD_PLACEMENT_KIND::TRIGGER_BOX != Placement.eKind ||
			(!Placement.requiresInteract && Placement.strInteractAction.empty()))
			continue;
		TRIGGER Trigger{};
		Trigger.strPlacementId = Placement.placementId;
		Trigger.bShowWhileInside = !Placement.requiresInteract;
		Trigger.vCenter = Placement.position;
		Trigger.vHalfExtents = Placement.halfExtents;
		Trigger.fYawDegrees = Placement.yawDegrees;
		/* An authored action wins: a box whose own move does not describe what the player
		is doing (the Valtan ledge runs an encounter, not a move) names its icon instead. */
		if (!Placement.strInteractAction.empty())
		{
			if ("godown" == Placement.strInteractAction) Trigger.eAction = ACTION::GODOWN;
			else if ("climb" == Placement.strInteractAction) Trigger.eAction = ACTION::CLIMB;
			else if ("tightrope" == Placement.strInteractAction) Trigger.eAction = ACTION::TIGHTROPE;
			else if ("check" == Placement.strInteractAction) Trigger.eAction = ACTION::CHECK;
			else
			{
				OutputDebugStringA(("[InteractKeyPrompt] unknown interactAction: " +
					Placement.strInteractAction + "\n").c_str());
				continue;
			}
			m_Triggers.push_back(std::move(Trigger));
			continue;
		}
		for (const WORLD_TRIGGER_EVENT& Event : Placement.triggerEvents)
		{
			if (WORLD_TRIGGER_EVENT_KIND::MOVE_PLAYER != Event.eKind)
				continue;
			const f32_t fRise = Event.targetPosition.y - Placement.position.y;
			Trigger.eAction = fRise < -1.f ? ACTION::GODOWN : (fRise > 1.f ? ACTION::CLIMB : ACTION::TIGHTROPE);
			break;
		}
		m_Triggers.push_back(std::move(Trigger));
	}
}

void Client::CInteractKeyPromptView::Hide()
{
	if (m_bVisible && nullptr != m_pView)
		for (const char* pSlot : SLOTS)
			m_pView->Set_SlotVisible(pSlot, false);
	m_bVisible = false;
	m_strShownTriggerId.clear();
	m_fShowSeconds = -1.f;
}

void Client::CInteractKeyPromptView::Update(const f32_t fTimeDelta,
	const std::shared_ptr<CCharacter>& pLocalCharacter, const std::string& strOfferedTriggerId,
	const bool_t bShown)
{
	if (nullptr == m_pView || !bShown || nullptr == pLocalCharacter)
	{
		Hide();
		return;
	}
	const TRIGGER* pTrigger = nullptr;
	if (!strOfferedTriggerId.empty())
	{
		for (const TRIGGER& Trigger : m_Triggers)
			if (Trigger.strPlacementId == strOfferedTriggerId)
				pTrigger = &Trigger;
	}
	if (nullptr == pTrigger && nullptr != pLocalCharacter->Get_Transform())
	{
		/* No Server offer: a box that fires on entry still shows the icon it authored
		while the player stands in it. */
		float3_t vPlayer{};
		XMStoreFloat3(&vPlayer, pLocalCharacter->Get_Transform()->Get_State(STATE::POSITION));
		for (const TRIGGER& Trigger : m_Triggers)
		{
			if (!Trigger.bShowWhileInside)
				continue;
			const f32_t fRadians = XMConvertToRadians(Trigger.fYawDegrees);
			const f32_t fCos = cosf(fRadians), fSin = sinf(fRadians);
			const f32_t fDeltaX = vPlayer.x - Trigger.vCenter.x;
			const f32_t fDeltaZ = vPlayer.z - Trigger.vCenter.z;
			/* Into the box's own axes, the way the Server tests the same OBB. */
			const f32_t fLocalX = fDeltaX * fCos + fDeltaZ * fSin;
			const f32_t fLocalZ = -fDeltaX * fSin + fDeltaZ * fCos;
			if (fabsf(fLocalX) <= Trigger.vHalfExtents.x &&
				fabsf(fLocalZ) <= Trigger.vHalfExtents.z &&
				fabsf(vPlayer.y - Trigger.vCenter.y) <= Trigger.vHalfExtents.y + 2.f)
			{
				pTrigger = &Trigger;
				break;
			}
		}
	}
	if (nullptr == pTrigger)
	{
		Hide();
		return;
	}

	CGameInstance& Instance = CGameInstance::Get();
	const float4x4_t* pView = Instance.Get_Transform(D3DTS::VIEW);
	const float4x4_t* pProj = Instance.Get_Transform(D3DTS::PROJ);
	const float2_t vViewport = Instance.Get_ViewportSize();
	float3_t vHead{};
	float2_t vPoint{};
	if (nullptr == pView || nullptr == pProj || vViewport.y <= 0.f ||
		!CWorldPlayerNameplateView::Try_GetHeadAnchor(*pLocalCharacter, vHead) ||
		!CWorldPlayerNameplateView::Try_ProjectWorldPosition(vHead, *pView, *pProj, vViewport, vPoint))
	{
		Hide();
		return;
	}

	if (m_strShownTriggerId != pTrigger->strPlacementId)
	{
		/* A new offer replays the "show" glow and swaps the action icon. */
		m_strShownTriggerId = pTrigger->strPlacementId;
		m_eAction = pTrigger->eAction;
		m_fShowSeconds = 0.f;
		m_pView->Set_SlotTexture("IKP_Icon", Action_Icon(static_cast<uint8_t>(m_eAction)));
	}
	else if (m_fShowSeconds >= 0.f)
	{
		m_fShowSeconds += fTimeDelta;
	}

	/* Retail px -> screen px at this viewport; slots take reference units. */
	const f32_t fPx = vViewport.y / RETAIL_HEIGHT;
	const f32_t fToRefX = REF_WIDTH / vViewport.x;
	const f32_t fToRefY = REF_HEIGHT / vViewport.y;
	m_fTextPx = TEXT_PX * fPx;
	const f32_t fLineBottom = vPoint.y - LINE_OFFSET_ABOVE_POINT * fPx;
	const f32_t fLineTop = fLineBottom - KEY_H * fPx;
	m_fTextCenterY = (fLineTop + fLineBottom) * 0.5f;
	const f32_t fIconCenterY = fLineTop - DESC_TOP_BELOW_ICON_CENTER * fPx;

	/* "{0} {1}": the name, a space, the keycap, centred on the point together. */
	f32_t fScale = 1.f;
	const wstring_t strFont = UILabelFont::Resolve(FONT_YG760, m_fTextPx, fScale);
	const f32_t fTextW = Instance.Measure_Text(strFont, Action_Text(static_cast<uint8_t>(m_eAction))).x * fScale;
	const f32_t fLineW = fTextW + (KEY_GAP + KEY_W) * fPx;
	m_fTextCenterX = vPoint.x - fLineW * 0.5f + fTextW * 0.5f;
	const f32_t fKeyX = vPoint.x - fLineW * 0.5f + fTextW + KEY_GAP * fPx;

	m_pView->Set_SlotRect("IKP_Icon", (vPoint.x - ICON_W * 0.5f * fPx) * fToRefX,
		(fIconCenterY - ICON_H * 0.5f * fPx) * fToRefY, ICON_W * fPx * fToRefX, ICON_H * fPx * fToRefY);
	m_pView->Set_SlotRect("IKP_Key", fKeyX * fToRefX, fLineTop * fToRefY,
		KEY_W * fPx * fToRefX, KEY_H * fPx * fToRefY);
	m_pView->Set_SlotVisible("IKP_Icon", true);
	m_pView->Set_SlotVisible("IKP_Key", true);

	/* effectMc "show": five 40 fps frames centred on the icon, then gone. */
	const int32_t iFxFrame = m_fShowSeconds >= 0.f ? static_cast<int32_t>(m_fShowSeconds * FX_FPS) : FX_FRAMES;
	if (iFxFrame < FX_FRAMES)
	{
		m_pView->Set_SlotTexture("IKP_Fx", FX_FRAME_ART[iFxFrame]);
		m_pView->Set_SlotRect("IKP_Fx", (vPoint.x - FX_SIZE * 0.5f * fPx) * fToRefX,
			(fIconCenterY - FX_SIZE * 0.5f * fPx) * fToRefY, FX_SIZE * fPx * fToRefX, FX_SIZE * fPx * fToRefY);
		m_pView->Set_SlotVisible("IKP_Fx", true);
	}
	else
	{
		m_pView->Set_SlotVisible("IKP_Fx", false);
		m_fShowSeconds = -1.f;
	}
	m_bVisible = true;
}

void Client::CInteractKeyPromptView::Render_Text() const
{
	if (!m_bVisible)
		return;
	/* YG760 14 px white with the retail black blur (blurSize 2, strength 3) read as an outline. */
	CUITextLayerScope HudText(UI_TEXT_LAYER::HUD);
	const wchar_t* pText = Action_Text(static_cast<uint8_t>(m_eAction));
	const fvector_t vOutline = XMVectorSet(0.f, 0.f, 0.f, 0.85f);
	static constexpr f32_t OFFSETS[8][2] = {
		{ -1.f, 0.f }, { 1.f, 0.f }, { 0.f, -1.f }, { 0.f, 1.f },
		{ -1.f, -1.f }, { 1.f, -1.f }, { -1.f, 1.f }, { 1.f, 1.f } };
	for (const auto& Offset : OFFSETS)
		(void)UILabelFont::Draw_Centered(FONT_YG760, pText, m_fTextCenterX + Offset[0],
			m_fTextCenterY + Offset[1], m_fTextPx, vOutline);
	(void)UILabelFont::Draw_Centered(FONT_YG760, pText, m_fTextCenterX, m_fTextCenterY, m_fTextPx,
		DirectX::Colors::White);
}
