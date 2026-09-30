#pragma once

#include "CombatHUDViewModel.h"
#include "Engine_Defines.h"

#include <array>
#include <memory>
#include <unordered_map>

namespace Client
{
class CUILayoutRuntime;

/* One JSON image layout per visible replicated actor. Health comes from the HUD
read model; this view owns neither gameplay values nor the actors' lifetimes. */
class CWorldHealthBarView final
{
public:
	CWorldHealthBarView(ComPtr<ID3D11Device> device, ComPtr<ID3D11DeviceContext> context);
	~CWorldHealthBarView();
	void Update(f32_t timeDelta, const std::vector<HUD_WORLD_HEALTH_BAR_STATE>& states,
		bool allowed, bool cardMazeActive);
	/* Pair order: ally, normal monster, KoukuSaydon, Kouku, Valtan.
	Reference pixels; each offset moves the complete HP/shield group. */
	bool Set_Offsets(const std::array<float2_t, 5>& offsets);
	// Reference-pixel head offset and non-accumulating layout scale; debug changes visibility only.
	bool Set_ValtanArmorBreakTuning(const float2_t& offset, const float2_t& scale, bool showDebug);
	// Called in the existing world-text pass, after UI occluders are registered.
	void Render_Text() const;
	// Shared by the mechanic gauge so both bars follow the visible, interpolated actor.
	static bool Try_GetHeadAnchor(const HUD_WORLD_HEALTH_BAR_STATE& state, float3_t& position);

private:
	struct RECT
	{
		f32_t x = 0.f, y = 0.f, width = 0.f, height = 0.f;
	};
	struct BAR
	{
		~BAR();
		std::unique_ptr<CUILayoutRuntime> view;
		std::unique_ptr<CUILayoutRuntime> armorBreakView;
		std::array<RECT, 4> rects;
		RECT armorBreakBaseRect;
		bool hasArmorBreakBaseRect = false;
	};
	std::unique_ptr<BAR> Create_Bar() const;

	struct BREAK_TEXT { float2_t position; f32_t alpha = 1.f; };
	std::vector<BREAK_TEXT> m_BreakTexts;
	std::array<float2_t, 5> m_Offsets{};
	float2_t m_ValtanArmorBreakOffset{ -24.f, -40.f };
	float2_t m_ValtanArmorBreakScale{ 1.f, 1.f };
	bool m_ShowValtanArmorBreakDebug = false;
	ComPtr<ID3D11Device> m_Device;
	ComPtr<ID3D11DeviceContext> m_Context;
	std::unordered_map<LostArk::Shared::NET_ENTITY_ID, std::unique_ptr<BAR>> m_Bars;
};
}
