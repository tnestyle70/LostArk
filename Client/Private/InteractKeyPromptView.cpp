#include "InteractKeyPromptView.h"

#include "Character.h"
#include "GameInstance.h"
#include "MapAssetCatalog.h"
#include "UILayoutRuntime.h"
#include "WorldGameplayDocument.h"

#include <cmath>

namespace
{
	constexpr const char* SLOT_KEY = "IKP_Key";
	/* The keycap shows once the player is this close (metres, XZ) to a gated box; the
	   Server's own prompt needs the player inside the box. */
	constexpr f32_t SHOW_RANGE = 10.f;
	/* Drawn a little above the box centre so it reads as "over" the spot, not on the floor. */
	constexpr f32_t LIFT = 1.2f;
}

void Client::CInteractKeyPromptView::Initialize(
	ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext,
	const uint32_t iOwnerLevelIndex, const char* pAreaId)
{
	m_pView = std::make_unique<CUILayoutRuntime>(pDevice, pContext, iOwnerLevelIndex,
		TEXT("Layer_UI"), L"UI/Interact/InteractKey_Layout.json");
	m_pView->Set_SlotVisible(SLOT_KEY, false);
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
			!Placement.requiresInteract || !Placement.isEnabled)
			continue;
		m_Triggers.push_back(TRIGGER{ Placement.placementId, Placement.position });
	}
}

void Client::CInteractKeyPromptView::Update(
	const std::shared_ptr<CCharacter>& pLocalCharacter, const bool_t bShown)
{
	if (nullptr == m_pView)
		return;
	const std::shared_ptr<CTransform> pTransform =
		nullptr != pLocalCharacter ? pLocalCharacter->Get_Transform() : nullptr;
	if (!bShown || m_Triggers.empty() || nullptr == pTransform)
	{
		m_pView->Set_SlotVisible(SLOT_KEY, false);
		return;
	}

	float3_t vPlayer{};
	XMStoreFloat3(&vPlayer, pTransform->Get_State(STATE::POSITION));
	const TRIGGER* pNearest = nullptr;
	f32_t fNearest = SHOW_RANGE * SHOW_RANGE;
	for (const TRIGGER& Trigger : m_Triggers)
	{
		const f32_t fDx = Trigger.vPosition.x - vPlayer.x;
		const f32_t fDz = Trigger.vPosition.z - vPlayer.z;
		const f32_t fDistSq = fDx * fDx + fDz * fDz;
		if (fDistSq < fNearest)
		{
			fNearest = fDistSq;
			pNearest = &Trigger;
		}
	}
	if (nullptr == pNearest)
	{
		m_pView->Set_SlotVisible(SLOT_KEY, false);
		return;
	}

	CGameInstance& Instance = CGameInstance::Get();
	const float4x4_t* pView = Instance.Get_Transform(D3DTS::VIEW);
	const float4x4_t* pProj = Instance.Get_Transform(D3DTS::PROJ);
	const f32_t fRefW = m_pView->Get_ResolutionWidth();
	const f32_t fRefH = m_pView->Get_ResolutionHeight();
	if (nullptr == pView || nullptr == pProj || fRefW <= 0.f || fRefH <= 0.f)
	{
		m_pView->Set_SlotVisible(SLOT_KEY, false);
		return;
	}
	const vector_t vWorld = XMVectorSet(pNearest->vPosition.x, pNearest->vPosition.y + LIFT,
		pNearest->vPosition.z, 1.f);
	const vector_t vViewPos = XMVector3TransformCoord(vWorld, XMLoadFloat4x4(pView));
	if (XMVectorGetZ(vViewPos) <= 0.f)
	{
		m_pView->Set_SlotVisible(SLOT_KEY, false);
		return;
	}
	float3_t ndc{};
	XMStoreFloat3(&ndc, XMVector3TransformCoord(vViewPos, XMLoadFloat4x4(pProj)));
	if (!std::isfinite(ndc.x) || !std::isfinite(ndc.y) ||
		ndc.x < -1.f || ndc.x > 1.f || ndc.y < -1.f || ndc.y > 1.f)
	{
		m_pView->Set_SlotVisible(SLOT_KEY, false);
		return;
	}
	f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
	if (!m_pView->Get_SlotRect(SLOT_KEY, fX, fY, fW, fH))
		return;
	const f32_t fCenterX = (ndc.x * 0.5f + 0.5f) * fRefW;
	const f32_t fCenterY = (0.5f - ndc.y * 0.5f) * fRefH;
	m_pView->Set_SlotRect(SLOT_KEY, std::round(fCenterX - fW * 0.5f), std::round(fCenterY - fH * 0.5f), fW, fH);
	m_pView->Set_SlotVisible(SLOT_KEY, true);
}
