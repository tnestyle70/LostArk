// 부위별 장비 상태를 내구도 HUD 이미지로 바꾼다.
#include "DurabilityHudView.h"

// 실루엣과 부위 이미지를 HUD 정렬 layer에 배치한다.
#include "UITextOcclusion.h"
// 문서 슬롯의 이미지·사각형·표시 상태를 조작한다.
#include "UILayoutRuntime.h"

// 저작 배율이 유한한지 isfinite로 검사한다.
#include <cmath>

namespace
{
	// 부위별 slot ID와 손상·파괴 이미지의 원본 크기다. 상태마다 다른 종횡비를 유지한다.
	struct PART_ART
	{
		const char*	pSlotId;
		const char*	pDamagedAsset;
		f32_t		fDamagedWidth, fDamagedHeight;
		const char*	pDestroyedAsset;
		f32_t		fDestroyedWidth, fDestroyedHeight;
	};

	constexpr PART_ART PART_TABLE[] = {
		{ "Durability_Weapon",
		  "UI/Durability/weapon_damaged.png",    14.f, 55.f,
		  "UI/Durability/weapon_destroyed.png",  15.f, 55.f },
		{ "Durability_Helmet",
		  "UI/Durability/helmet_damaged.png",    15.f, 16.f,
		  "UI/Durability/helmet_destroyed.png",  16.f, 17.f },
		{ "Durability_Top",
		  "UI/Durability/top_damaged.png",       40.f, 52.f,
		  "UI/Durability/top_destroyed.png",     40.f, 52.f },
		{ "Durability_Gloves",
		  "UI/Durability/gloves_damaged.png",    48.f, 28.f,
		  "UI/Durability/gloves_destroyed.png",  48.f, 28.f },
		{ "Durability_Bottoms",
		  "UI/Durability/bottoms_damaged.png",   32.f, 56.f,
		  "UI/Durability/bottoms_destroyed.png", 31.f, 57.f },
		{ "Durability_Shoulder",
		  "UI/Durability/shoulder_damaged.png",  43.f, 17.f,
		  "UI/Durability/shoulder_destroyed.png",45.f, 19.f },
		{ "Durability_Life",
		  "UI/Durability/life_damaged.png",      31.f, 40.f,
		  "UI/Durability/life_destroyed.png",    31.f, 40.f },
		{ "Durability_Bracer",
		  "UI/Durability/bracer_damaged.png",    46.f, 18.f,
		  "UI/Durability/bracer_destroyed.png",  46.f, 18.f },
	};
	static_assert(ETOUI(Client::CDurabilityHudView::PART::END) ==
		sizeof(PART_TABLE) / sizeof(PART_TABLE[0]),
		"every PART needs one row of source art");

	// 팔찌를 사용하지 않을 때 장갑과 실루엣이 참조하는 별도 이미지다.
	constexpr const char* GLOVES_PLAIN_DAMAGED = "UI/Durability/gloves_plain_damaged.png";
	constexpr const char* GLOVES_PLAIN_DESTROYED = "UI/Durability/gloves_plain_destroyed.png";
	constexpr f32_t GLOVES_PLAIN_DAMAGED_WIDTH = 48.f;
	constexpr f32_t GLOVES_PLAIN_DAMAGED_HEIGHT = 28.f;
	constexpr f32_t GLOVES_PLAIN_DESTROYED_WIDTH = 49.f;
	constexpr f32_t GLOVES_PLAIN_DESTROYED_HEIGHT = 29.f;

	constexpr const char* BASE_WITH_BRACER = "UI/Durability/base_bracer.png";
	constexpr const char* BASE_PLAIN = "UI/Durability/base_plain.png";
	constexpr const char* BASE_SLOT_ID = "Durability_Base";
}

Client::CDurabilityHudView::CDurabilityHudView(
	ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pView{ std::make_unique<CUILayoutRuntime>(
		pDevice, pContext, ETOUI(LEVEL::STATIC), TEXT("Layer_UI"),
		L"UI/Durability/DurabilityUI.json") }
{
	m_pView->Set_UISortLayer(UI_TEXT_LAYER::HUD);
	// STATIC sprite는 생성 직후 보일 수 있으므로 MainApp의 첫 표시 판정 전에는 숨긴다.
	Hide();
}

Client::CDurabilityHudView::~CDurabilityHudView()
{
}

void Client::CDurabilityHudView::Hide()
{
	if (nullptr != m_pView)
		m_pView->Set_AllSlotsVisible(false);
}

// 설정이 실제로 바뀐 때만 실루엣과 장갑을 교체한다.
void Client::CDurabilityHudView::Set_BracerEnabled(const bool_t bEnabled)
{
	if (m_bBracerEnabled == bEnabled)
		return;
	m_bBracerEnabled = bEnabled;
	if (nullptr == m_pView)
		return;
	m_pView->Set_SlotTexture(BASE_SLOT_ID, bEnabled ? BASE_WITH_BRACER : BASE_PLAIN);
	Apply_Part(PART::GLOVES);
}

// 유효한 부위의 변경된 상태만 저장하고 해당 이미지를 갱신한다.
void Client::CDurabilityHudView::Set_PartState(const PART ePart, const PART_STATE eState)
{
	const uint32_t iPart = ETOUI(ePart);
	if (iPart >= ETOUI(PART::END) || m_PartStates[iPart] == eState)
		return;
	m_PartStates[iPart] = eState;
	Apply_Part(ePart);
}

// 부위 상태로 이미지와 원본 크기를 고른다. 슬롯·배율 조회 실패는 현재 이미지를 더 바꾸지 않는다.
void Client::CDurabilityHudView::Apply_Part(const PART ePart)
{
	if (nullptr == m_pView)
		return;
	const uint32_t iPart = ETOUI(ePart);
	if (iPart >= ETOUI(PART::END))
		return;
	const PART_ART& Art = PART_TABLE[iPart];
	const PART_STATE eState = m_PartStates[iPart];
	if (PART_STATE::NORMAL == eState)
	{
		m_pView->Set_SlotVisible(Art.pSlotId, false);
		return;
	}

	const bool_t bDestroyed = PART_STATE::DESTROYED == eState;
	const char* pAsset = bDestroyed ? Art.pDestroyedAsset : Art.pDamagedAsset;
	f32_t fSourceWidth = bDestroyed ? Art.fDestroyedWidth : Art.fDamagedWidth;
	f32_t fSourceHeight = bDestroyed ? Art.fDestroyedHeight : Art.fDamagedHeight;
	if (PART::GLOVES == ePart && !m_bBracerEnabled)
	{
		pAsset = bDestroyed ? GLOVES_PLAIN_DESTROYED : GLOVES_PLAIN_DAMAGED;
		fSourceWidth = bDestroyed ?
			GLOVES_PLAIN_DESTROYED_WIDTH : GLOVES_PLAIN_DAMAGED_WIDTH;
		fSourceHeight = bDestroyed ?
			GLOVES_PLAIN_DESTROYED_HEIGHT : GLOVES_PLAIN_DAMAGED_HEIGHT;
	}

	// 처음의 저작 배율을 저장한다. 이전 상태의 출력 크기로 다시 나누면 상태 전환마다 크기가 달라진다.
	f32_t fSlotX = 0.f, fSlotY = 0.f, fSlotWidth = 0.f, fSlotHeight = 0.f;
	if (!m_pView->Get_SlotRect(Art.pSlotId, fSlotX, fSlotY, fSlotWidth, fSlotHeight))
		return;
	if (0.f == m_PartAuthoredScales[iPart])
	{
		const f32_t fAuthoredScale = fSlotWidth / Art.fDamagedWidth;
		if (!std::isfinite(fAuthoredScale) || fAuthoredScale <= 0.f)
			return;
		m_PartAuthoredScales[iPart] = fAuthoredScale;
	}
	const f32_t fScale = m_PartAuthoredScales[iPart];
	m_pView->Set_SlotTexture(Art.pSlotId, pAsset);
	m_pView->Set_SlotRect(Art.pSlotId, fSlotX, fSlotY,
		fSourceWidth * fScale, fSourceHeight * fScale);
	m_pView->Set_SlotVisible(Art.pSlotId, true);
}

// MainApp이 손상 부위가 있는 허용 Level에서 호출해 현재 부위 상태 전체를 다시 표시한다.
void Client::CDurabilityHudView::Update()
{
	if (nullptr == m_pView)
		return;
	m_pView->Set_SlotVisible(BASE_SLOT_ID, true);
	for (uint32_t iPart = 0; iPart < ETOUI(PART::END); ++iPart)
		Apply_Part(static_cast<PART>(iPart));
}
