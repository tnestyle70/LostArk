#include "DurabilityHudView.h"

#include "UITextOcclusion.h"
#include "UILayoutRuntime.h"

#include <cmath>

namespace
{
	/* One row per part: the layout slot, and for each state the Resources-relative art and its
	source size. Both come from durability.gfx's own DefineSubImage regions, so the two frames
	keep their real sizes instead of being stretched to a single box. */
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

	/* Without bracers the account uses a different glove overlay and silhouette frame. */
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
	/* These GameObjects live under LEVEL::STATIC, so without this the whole widget is on
	screen from construction (Lobby included) until the first Update(). */
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

void Client::CDurabilityHudView::Set_PartState(const PART ePart, const PART_STATE eState)
{
	const uint32_t iPart = ETOUI(ePart);
	if (iPart >= ETOUI(PART::END) || m_PartStates[iPart] == eState)
		return;
	m_PartStates[iPart] = eState;
	Apply_Part(ePart);
}

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

	/* Capture the authored scale before this class first resizes the part. Later frames must
	not derive it from the previous state's output, or different-sized art scales repeatedly. */
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

void Client::CDurabilityHudView::Update()
{
	if (nullptr == m_pView)
		return;
	m_pView->Set_SlotVisible(BASE_SLOT_ID, true);
	for (uint32_t iPart = 0; iPart < ETOUI(PART::END); ++iPart)
		Apply_Part(static_cast<PART>(iPart));
}
