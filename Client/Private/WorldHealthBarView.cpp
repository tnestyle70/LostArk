#include "WorldHealthBarView.h"

#include "Character.h"
#include "GameInstance.h"
#include "Model.h"
#include "Npc.h"
#include "Transform.h"
#include "UILayoutRuntime.h"
#include "UITextOcclusion.h"
#include "UILabelFont.h"
#include "Valtan.h"
#include "WorldPlayerNameplateView.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_set>

namespace
{
	constexpr const char* SLOTS[] = {
		"Health_Frame", "Health_EnemyFill", "Health_PlayerFill", "Health_ShieldFill" };
	constexpr f32_t HEAD_GAP = 3.f;

	size_t HealthBarPositionGroup(const Client::HUD_WORLD_HEALTH_BAR_STATE& state)
	{
		if (state.isPlayer) return 0u;
		const auto& id = state.strArchetypeId;
		if (id == "BOSS_KAKULSAYDON_G1_SAYDON" || id == "BOSS_KAKULSAYDON_G3_SAYDON" ||
			id == "BOSS_KAKULSAYDON_BINGO_SAYDON") return 2u;
		if (id == "BOSS_KAKULSAYDON_G1_KOUKU" || id == "BOSS_KAKULSAYDON_G2_KOUKU") return 3u;
		if (id == "BOSS_VALTAN" || id == "BOSS_VALTAN_GHOST") return 4u;
		return 1u;
	}

	bool HealthBarHidden(const Client::HUD_WORLD_HEALTH_BAR_STATE& state, const bool cardMazeActive)
	{
		const auto& id = state.strArchetypeId;
		if (id == "BOSS_KAKULSAYDON_G2_BIG_SAYDON") return true;
		const bool cardSoldier = id == "MONSTER_KOUKU_CARD_HEART" || id == "MONSTER_KOUKU_CARD_DIAMOND" ||
			id == "MONSTER_KOUKU_CARD_CLUB" || id == "MONSTER_KOUKU_CARD_SPADE";
		return cardMazeActive && (state.isPlayer || cardSoldier);
	}

	bool IsFinite(const float3_t& value)
	{
		return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
	}

	bool BoundsHead(const std::shared_ptr<Engine::CModel>& model,
		const float4x4_t& world, float3_t& position)
	{
		float3_t minimum{}, maximum{};
		if (!model || !model->Try_GetBindGeometryBounds(minimum, maximum) ||
			!IsFinite(minimum) || !IsFinite(maximum)) return false;
		const f32_t limit = (std::numeric_limits<f32_t>::max)();
		float3_t low(limit, limit, limit), high(-limit, -limit, -limit);
		const matrix_t root = XMLoadFloat4x4(&world);
		/* Bounds already include WModel preScale. Only the presentation root belongs
		here; transforming all corners also handles Valtan's source-axis rotation. */
		for (uint32_t corner = 0u; corner < 8u; ++corner)
		{
			const vector_t local = XMVectorSet(corner & 1u ? maximum.x : minimum.x,
				corner & 2u ? maximum.y : minimum.y, corner & 4u ? maximum.z : minimum.z, 1.f);
			float3_t point{};
			XMStoreFloat3(&point, XMVector3TransformCoord(local, root));
			if (!IsFinite(point)) return false;
			low.x = (std::min)(low.x, point.x); low.y = (std::min)(low.y, point.y);
			low.z = (std::min)(low.z, point.z); high.x = (std::max)(high.x, point.x);
			high.y = (std::max)(high.y, point.y); high.z = (std::max)(high.z, point.z);
		}
		position = float3_t((low.x + high.x) * 0.5f, high.y + 0.05f, (low.z + high.z) * 0.5f);
		return IsFinite(position);
	}

	bool NpcHead(const std::shared_ptr<Engine::CModel>& model,
		const float4x4_t& world, float3_t& position)
	{
		// Skinned NPC bind vertices can use a different basis/scale from the
		// animated pose. Anchor to the same head bone that draws the actor.
		if (!model || !model->Has_Bone("bip001-head"))
			return BoundsHead(model, world, position);
		// CModel's combined bone matrix already includes the model preScale.
		XMStoreFloat3(&position, XMVector3TransformCoord(
			model->Get_BoneMatrix("bip001-head").r[3], XMLoadFloat4x4(&world)));
		return IsFinite(position);
	}
}

Client::CWorldHealthBarView::CWorldHealthBarView(
	ComPtr<ID3D11Device> device, ComPtr<ID3D11DeviceContext> context)
	: m_Device(std::move(device)), m_Context(std::move(context))
{
}

Client::CWorldHealthBarView::~CWorldHealthBarView() = default;

Client::CWorldHealthBarView::BAR::~BAR()
{
	if (view) view->Release_Sprites();
	if (armorBreakView) armorBreakView->Release_Sprites();
}

std::unique_ptr<Client::CWorldHealthBarView::BAR> Client::CWorldHealthBarView::Create_Bar() const
{
	auto bar = std::make_unique<BAR>();
	bar->view = std::make_unique<CUILayoutRuntime>(m_Device, m_Context,
		ETOUI(LEVEL::STATIC), TEXT("Layer_HeadHealthBars"), L"UI/HeadStatus/HealthBar_Layout.json");
	bar->view->Set_AllSlotsVisible(false);
	bar->view->Set_UISortLayer(UI_TEXT_LAYER::WORLD - 1);
	for (size_t index = 0u; index < bar->rects.size(); ++index)
	{
		auto& rect = bar->rects[index];
		if (!bar->view->Get_SlotRect(SLOTS[index], rect.x, rect.y, rect.width, rect.height) ||
			rect.width <= 0.f || rect.height <= 0.f) return {};
	}
	return bar;
}

bool Client::CWorldHealthBarView::Try_GetHeadAnchor(
	const HUD_WORLD_HEALTH_BAR_STATE& state, float3_t& position)
{
	const auto actor = state.pPresentation.lock();
	if (!actor) return false;
	if (const auto character = std::dynamic_pointer_cast<CCharacter>(actor))
		return !character->Is_WorldPresentationHidden() && !character->Is_ShipPresentation() &&
			CWorldPlayerNameplateView::Try_GetHeadAnchor(*character, position);
	if (const auto npc = std::dynamic_pointer_cast<CNpc>(actor))
		return npc->Is_PresentationVisible() && npc->Get_Transform() &&
			NpcHead(npc->Get_Model(), *npc->Get_Transform()->Get_WorldMatrixPtr(), position);
	if (const auto valtan = std::dynamic_pointer_cast<CValtan>(actor))
	{
		float4x4_t root{};
		return valtan->Is_PresentationVisible() && valtan->Try_Get_PresentationRootMatrix(&root) &&
			BoundsHead(valtan->Get_BodyModel(), root, position);
	}
	return false;
}

bool Client::CWorldHealthBarView::Set_Offsets(const std::array<float2_t, 5>& offsets)
{
	for (const auto& offset : offsets)
		if (!std::isfinite(offset.x) || !std::isfinite(offset.y) ||
			std::abs(offset.x) > 1280.f || std::abs(offset.y) > 1280.f) return false;
	m_Offsets = offsets;
	return true;
}

bool Client::CWorldHealthBarView::Set_ValtanArmorBreakTuning(
	const float2_t& offset, const float2_t& scale, const bool showDebug)
{
	if (!std::isfinite(offset.x) || !std::isfinite(offset.y) ||
		std::abs(offset.x) > 1280.f || std::abs(offset.y) > 1280.f ||
		!std::isfinite(scale.x) || !std::isfinite(scale.y) ||
		scale.x < 0.1f || scale.x > 3.f || scale.y < 0.1f || scale.y > 3.f) return false;
	m_ValtanArmorBreakOffset = offset;
	m_ValtanArmorBreakScale = scale;
	m_ShowValtanArmorBreakDebug = showDebug;
	return true;
}

void Client::CWorldHealthBarView::Update(const f32_t timeDelta,
	const std::vector<HUD_WORLD_HEALTH_BAR_STATE>& states, const bool allowed, const bool cardMazeActive)
{
	/* Hide before sampling so offscreen, dead and rejected actors cannot leave a
	last-frame image behind. Missing entities release only this view's own sprites. */
	m_BreakTexts.clear();
	for (auto& [id, bar] : m_Bars)
	{
		bar->view->Set_AllSlotsVisible(false);
		if (bar->armorBreakView) bar->armorBreakView->Set_AllSlotsVisible(false);
	}
	std::unordered_set<LostArk::Shared::NET_ENTITY_ID> live;
	for (const auto& state : states)
		if (state.iCurrentHp > 0u && !state.isLocal && !state.pPresentation.expired()) live.insert(state.iNetEntityId);
	for (auto it = m_Bars.begin(); it != m_Bars.end();)
		if (live.find(it->first) == live.end()) it = m_Bars.erase(it); else ++it;
	if (!allowed) return;

	auto& game = CGameInstance::Get();
	const auto* view = game.Get_Transform(D3DTS::VIEW);
	const auto* projection = game.Get_Transform(D3DTS::PROJ);
	const auto viewport = game.Get_ViewportSize();
	if (!view || !projection || viewport.x <= 0.f || viewport.y <= 0.f) return;
	for (const auto& state : states)
	{
		if (HealthBarHidden(state, cardMazeActive) || state.isLocal || state.iNetEntityId == LostArk::Shared::INVALID_NET_ENTITY_ID ||
			state.iMaximumHp == 0u || state.iCurrentHp == 0u) continue;
		float3_t head{};
		float2_t screen{};
		if (!Try_GetHeadAnchor(state, head) || !CWorldPlayerNameplateView::Try_ProjectWorldPosition(
			head, *view, *projection, viewport, screen)) continue;
		auto found = m_Bars.find(state.iNetEntityId);
		if (found == m_Bars.end())
		{
			auto bar = Create_Bar();
			if (!bar) continue;
			found = m_Bars.emplace(state.iNetEntityId, std::move(bar)).first;
		}
		auto& bar = *found->second;
		if (const auto valtan = std::dynamic_pointer_cast<CValtan>(state.pPresentation.lock()))
		{
			const bool ready = valtan->Is_ArmorBreakAvailable() || m_ShowValtanArmorBreakDebug;
			if (ready && !bar.armorBreakView)
			{
				bar.armorBreakView = std::make_unique<CUILayoutRuntime>(m_Device, m_Context,
					ETOUI(LEVEL::STATIC), TEXT("Layer_ValtanArmorBreak"), L"UI/HeadStatus/ValtanArmorBreak_Layout.json");
				bar.armorBreakView->Set_UISortLayer(UI_TEXT_LAYER::WORLD - 1);
				bar.armorBreakView->Set_AllSlotsVisible(false);
				auto& base = bar.armorBreakBaseRect;
				bar.hasArmorBreakBaseRect = bar.armorBreakView->Get_SlotRect("Valtan_ArmorBreakReady",
					base.x, base.y, base.width, base.height) && base.width > 0.f && base.height > 0.f;
			}
			if (ready && bar.armorBreakView && bar.hasArmorBreakBaseRect)
			{
				const auto& base = bar.armorBreakBaseRect;
				const f32_t width = base.width * m_ValtanArmorBreakScale.x;
				const f32_t height = base.height * m_ValtanArmorBreakScale.y;
				bar.armorBreakView->Set_SlotRect("Valtan_ArmorBreakReady",
					screen.x * bar.armorBreakView->Get_ResolutionWidth() / viewport.x - width * 0.5f + m_ValtanArmorBreakOffset.x,
					screen.y * bar.armorBreakView->Get_ResolutionHeight() / viewport.y + m_ValtanArmorBreakOffset.y,
					width, height);
				bar.armorBreakView->Set_SlotVisible("Valtan_ArmorBreakReady", true);
				bar.armorBreakView->Update(timeDelta);
			}
			const f32_t remaining = valtan->Get_ArmorBreakFeedbackRemainingSeconds();
			if (remaining > 0.f)
				m_BreakTexts.push_back({ float2_t(screen.x, screen.y - 2.f * viewport.y / 720.f),
					(std::min)(1.f, remaining / 0.3f) });
		}
		const auto& frame = bar.rects[0];
		const auto& offset = m_Offsets[HealthBarPositionGroup(state)];
		const f32_t x = screen.x * bar.view->Get_ResolutionWidth() / viewport.x - frame.width * 0.5f + offset.x;
		const f32_t y = screen.y * bar.view->Get_ResolutionHeight() / viewport.y - frame.height - HEAD_GAP -
			(state.isPlayer ? CWorldPlayerNameplateView::Stack_Top_RefPx() : 0.f) +
			offset.y;
		for (size_t index = 0u; index < bar.rects.size(); ++index)
			bar.view->Set_SlotPosition(SLOTS[index], x + bar.rects[index].x - frame.x,
				y + bar.rects[index].y - frame.y);

		/* Match the HUD's additive shield representation. A shield above full HP
		shares the same total width instead of clipping or pretending HP was lost. */
		const double hp = static_cast<double>((std::min)(state.iCurrentHp, state.iMaximumHp));
		const double shield = static_cast<double>(state.iShield);
		const double total = (std::max)(static_cast<double>(state.iMaximumHp), hp + shield);
		const f32_t hpRatio = static_cast<f32_t>(hp / total);
		const f32_t shieldRatio = static_cast<f32_t>(shield / total);
		const size_t fill = state.isPlayer && !state.isEnemyPlayer ? 2u : 1u;
		bar.view->Set_SlotVisible(SLOTS[0], true);
		bar.view->Set_SlotVisible(SLOTS[fill], hpRatio > 0.f);
		bar.view->Set_SlotFillRatio(SLOTS[fill], hpRatio);
		const auto& shieldRect = bar.rects[3];
		bar.view->Set_SlotPosition(SLOTS[3], x + shieldRect.x - frame.x + shieldRect.width * hpRatio,
			y + shieldRect.y - frame.y);
		bar.view->Set_SlotFillRatio(SLOTS[3], shieldRatio);
		bar.view->Set_SlotVisible(SLOTS[3], shieldRatio > 0.f);
		bar.view->Update(timeDelta);
	}
}

void Client::CWorldHealthBarView::Render_Text() const
{
	if (m_BreakTexts.empty()) return;
	auto& game = CGameInstance::Get();
	const f32_t ratio = game.Get_ViewportSize().y / 720.f;
	f32_t scale = 1.f;
	const auto font = UILabelFont::Resolve(TEXT("Font_YG760"), 26.f * ratio, scale);
	const wchar_t* text = L"\uD30C\uAD34";
	const auto size = game.Measure_Text(font, text);
	CUITextLayerScope worldText(UI_TEXT_LAYER::WORLD);
	for (const auto& feedback : m_BreakTexts)
	{
		const float2_t position(feedback.position.x - size.x * scale * 0.5f, feedback.position.y);
		game.Draw_Text(font, text, float2_t(position.x + ratio, position.y + ratio),
			XMVectorSet(0.f, 0.f, 0.f, feedback.alpha), 0.f, float2_t(0.f, 0.f), scale);
		game.Draw_Text(font, text, position, XMVectorSet(0.4f, 1.f, 0.73f, feedback.alpha),
			0.f, float2_t(0.f, 0.f), scale);
	}
}
