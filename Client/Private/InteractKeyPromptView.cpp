#include "InteractKeyPromptView.h"

// 행동 문구의 White 색상 상수를 사용한다.
#include <DirectXColors.h>

// 캐릭터의 Transform과 머리 위치를 조회한다.
#include "Character.h"
// 현재 View/Projection·viewport와 글자 폭을 조회한다.
#include "GameInstance.h"
// Area의 World 문서를 찾기 위한 Data 경로를 구한다.
#include "MapAssetCatalog.h"
// 화면 크기에 맞는 글꼴을 선택하고 중앙 정렬 문구를 그린다.
#include "UILabelFont.h"
// 아이콘·키·등장 효과 슬롯을 만들고 위치·이미지·표시를 갱신한다.
#include "UILayoutRuntime.h"
// 글자를 HUD 계층으로 표시해 상위 창의 가림 규칙을 사용한다.
#include "UITextOcclusion.h"
// viewer world의 trigger 배치와 MOVE_PLAYER 이벤트를 읽는다.
#include "WorldGameplayDocument.h"
// 캐릭터 머리 anchor와 월드→화면 투영을 재사용한다.
#include "WorldPlayerNameplateView.h"

// 상자 로컬 축 검사에 삼각함수와 절댓값을 사용한다.
#include <cmath>

namespace
{
	// MasterKeyComponent의 1920×1080 기준 픽셀 치수다.
	constexpr f32_t RETAIL_HEIGHT = 1080.f;
	constexpr f32_t LINE_OFFSET_ABOVE_POINT = 50.f;		// 투영점에서 글자 하단까지의 위쪽 간격이다.
	constexpr f32_t TEXT_PX = 14.f;						// descriptionTF의 원본 글자 크기다.
	constexpr f32_t DESC_TOP_BELOW_ICON_CENTER = 29.f;	// 아이콘 중심에서 설명 상단까지의 간격이다.
	constexpr f32_t ICON_W = 70.f * 0.7428589f;			// iconType_mc 원본 크기에 저작 배율을 적용한다.
	constexpr f32_t ICON_H = 69.f * 0.7536011f;
	constexpr f32_t KEY_H = 18.f;						// G 키 이미지의 원본 높이다.
	constexpr f32_t KEY_W = 18.f * 34.f / 35.f;
	constexpr f32_t KEY_GAP = 4.f;						// 행동 문구와 G 키 사이의 공백이다.
	constexpr f32_t FX_SIZE = 300.f;					// 아이콘 중심에 놓는 등장 효과 크기다.
	constexpr f32_t FX_FPS = 40.f;
	constexpr int32_t FX_FRAMES = 5;
	constexpr f32_t REF_WIDTH = 1280.f;
	constexpr f32_t REF_HEIGHT = 720.f;

	const wstring_t FONT_YG760 = TEXT("Font_YG760");
	const char* const SLOTS[] = { "IKP_Fx", "IKP_Icon", "IKP_Key" };
	const char* const FX_FRAME_ART[FX_FRAMES] = {
		"UI/Interact/ShowFx_0.png", "UI/Interact/ShowFx_1.png", "UI/Interact/ShowFx_2.png",
		"UI/Interact/ShowFx_3.png", "UI/Interact/ShowFx_4.png" };

	// 행동별 GameMsg 문구를 wide escape로 보관한다.
	const wchar_t* Action_Text(const uint8_t iAction)
	{
		switch (iAction)
		{
		case 0: return L"\xB0B4\xB824\xAC00\xAE30";
		case 1: return L"\xC62C\xB77C\xAC00\xAE30";
		case 2: return L"\xAC74\xB108\xAC00\xAE30";
		case 4: return L"\xC774\xB3D9\xD558\xAE30";
		case 5: return L"\xB808\xBC84 \xB2F9\xAE30\xAE30";
		case 6: return L"\xC815\xBC15\xD558\xAE30";
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
		case 4: return "UI/Interact/Icon_move.png";
		case 5: return "UI/Interact/Icon_lever.png";
		case 6: return "UI/Interact/Icon_check.png";
		default: return "UI/Interact/Icon_check.png";
		}
	}
}

// Level의 Area ID로 viewer.world.json을 읽고 표시용 상자 값만 복사한다.
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
		// 명시한 interactAction을 이동 높이에서 추정한 행동보다 우선한다.
		if (!Placement.strInteractAction.empty())
		{
			if ("godown" == Placement.strInteractAction) Trigger.eAction = ACTION::GODOWN;
			else if ("climb" == Placement.strInteractAction) Trigger.eAction = ACTION::CLIMB;
			else if ("tightrope" == Placement.strInteractAction) Trigger.eAction = ACTION::TIGHTROPE;
			else if ("check" == Placement.strInteractAction) Trigger.eAction = ACTION::CHECK;
			else if (0 == Placement.strInteractAction.rfind("dock:", 0) && Placement.strInteractAction.size() > 5u)
			{
				// dock:<name>의 UTF-8 장소명을 wide 문구로 바꾼다. 변환할 수 없는 행은 건너뛴다.
				const std::string strLabel = Placement.strInteractAction.substr(5u);
				const int iWide = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, strLabel.c_str(),
					static_cast<int>(strLabel.size()), nullptr, 0);
				if (iWide <= 0)
					continue;
				Trigger.strLabel.assign(static_cast<size_t>(iWide), L'\0');
				MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, strLabel.c_str(),
					static_cast<int>(strLabel.size()), Trigger.strLabel.data(), iWide);
				Trigger.eAction = ACTION::DOCK;
			}
			else if ("move" == Placement.strInteractAction) Trigger.eAction = ACTION::MOVE;
			else if ("lever" == Placement.strInteractAction) Trigger.eAction = ACTION::LEVER;
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

// 유효한 대상과 머리 투영점을 찾은 뒤 화면 슬롯과 텍스트 위치를 함께 갱신한다.
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
		// 제안 ID에 대응하는 상자가 없으면 자동 발동 상자 내부의 아이콘 표시를 검사한다.
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
			// 위치를 yaw의 역방향으로 돌려 상자의 로컬 X/Z 범위와 비교한다.
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
	// 카메라·머리 anchor·투영이 준비되지 않으면 이전 표시를 숨긴다.
	if (nullptr == pView || nullptr == pProj || vViewport.y <= 0.f ||
		!CWorldPlayerNameplateView::Try_GetHeadAnchor(*pLocalCharacter, vHead) ||
		!CWorldPlayerNameplateView::Try_ProjectWorldPosition(vHead, *pView, *pProj, vViewport, vPoint))
	{
		Hide();
		return;
	}

	if (m_strShownTriggerId != pTrigger->strPlacementId)
	{
		// 대상 ID가 바뀔 때만 등장 효과를 재시작하고 아이콘·문구를 교체한다.
		m_strShownTriggerId = pTrigger->strPlacementId;
		m_eAction = pTrigger->eAction;
		m_strLabel = pTrigger->strLabel;
		m_fShowSeconds = 0.f;
		m_pView->Set_SlotTexture("IKP_Icon", Action_Icon(static_cast<uint8_t>(m_eAction)));
	}
	else if (m_fShowSeconds >= 0.f)
	{
		m_fShowSeconds += fTimeDelta;
	}

	// 1080p 치수를 화면 픽셀로, 이미지 슬롯 위치를 1280×720 기준 좌표로 바꾼다.
	const f32_t fPx = vViewport.y / RETAIL_HEIGHT;
	const f32_t fToRefX = REF_WIDTH / vViewport.x;
	const f32_t fToRefY = REF_HEIGHT / vViewport.y;
	m_fTextPx = TEXT_PX * fPx;
	const f32_t fLineBottom = vPoint.y - LINE_OFFSET_ABOVE_POINT * fPx;
	const f32_t fLineTop = fLineBottom - KEY_H * fPx;
	m_fTextCenterY = (fLineTop + fLineBottom) * 0.5f;
	const f32_t fIconCenterY = fLineTop - DESC_TOP_BELOW_ICON_CENTER * fPx;

	// 문구·공백·G 키 전체 폭을 합쳐 투영점 중앙에 한 줄로 배치한다.
	f32_t fScale = 1.f;
	const wstring_t strFont = UILabelFont::Resolve(FONT_YG760, m_fTextPx, fScale);
	const wchar_t* pActionText = ACTION::DOCK == m_eAction && !m_strLabel.empty() ?
		m_strLabel.c_str() : Action_Text(static_cast<uint8_t>(m_eAction));
	const f32_t fTextW = Instance.Measure_Text(strFont, pActionText).x * fScale;
	const f32_t fLineW = fTextW + (KEY_GAP + KEY_W) * fPx;
	m_fTextCenterX = vPoint.x - fLineW * 0.5f + fTextW * 0.5f;
	const f32_t fKeyX = vPoint.x - fLineW * 0.5f + fTextW + KEY_GAP * fPx;

	m_pView->Set_SlotRect("IKP_Icon", (vPoint.x - ICON_W * 0.5f * fPx) * fToRefX,
		(fIconCenterY - ICON_H * 0.5f * fPx) * fToRefY, ICON_W * fPx * fToRefX, ICON_H * fPx * fToRefY);
	m_pView->Set_SlotRect("IKP_Key", fKeyX * fToRefX, fLineTop * fToRefY,
		KEY_W * fPx * fToRefX, KEY_H * fPx * fToRefY);
	m_pView->Set_SlotVisible("IKP_Icon", true);
	m_pView->Set_SlotVisible("IKP_Key", true);

	// 등장 효과는 40fps의 다섯 장을 표시한 뒤 숨긴다.
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

bool_t Client::CInteractKeyPromptView::Try_Get_DockPoint(float3_t& vOutCenter, f32_t& fOutYawDegrees) const
{
	for (const TRIGGER& Trigger : m_Triggers)
	{
		if (ACTION::DOCK != Trigger.eAction)
			continue;
		vOutCenter = Trigger.vCenter;
		fOutYawDegrees = Trigger.fYawDegrees;
		return true;
	}
	return false;
}

void Client::CInteractKeyPromptView::Render_Text() const
{
	if (!m_bVisible)
		return;
	// YG760 흰 글자 주변 여덟 위치에 검은 글자를 그려 외곽선을 만든다.
	CUITextLayerScope HudText(UI_TEXT_LAYER::HUD);
	const wchar_t* pText = ACTION::DOCK == m_eAction && !m_strLabel.empty() ?
		m_strLabel.c_str() : Action_Text(static_cast<uint8_t>(m_eAction));
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
