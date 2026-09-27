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
	void Update(f32_t timeDelta, const std::vector<HUD_WORLD_HEALTH_BAR_STATE>& states, bool allowed);

private:
	struct RECT
	{
		f32_t x = 0.f, y = 0.f, width = 0.f, height = 0.f;
	};
	struct BAR
	{
		~BAR();
		std::unique_ptr<CUILayoutRuntime> view;
		std::array<RECT, 4> rects;
	};
	std::unique_ptr<BAR> Create_Bar() const;
	static bool Try_GetHeadAnchor(const HUD_WORLD_HEALTH_BAR_STATE& state, float3_t& position);

	ComPtr<ID3D11Device> m_Device;
	ComPtr<ID3D11DeviceContext> m_Context;
	std::unordered_map<LostArk::Shared::NET_ENTITY_ID, std::unique_ptr<BAR>> m_Bars;
};
}
