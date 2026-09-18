#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <memory>
#include <string>
#include <vector>

NS_BEGIN(Client)

class CUILayoutRuntime;
class CCharacter;

/* The retail interaction key over a gated trigger box: the shared "G" keycap
(EFUI_SHAREIMAGE Shared_GlobalInputDeviceKey_G) drawn in screen space over the box the player
is walking up to, so the box is visible before the Server's prompt fires.

Presentation only. Which boxes are interact-gated comes from the area's published
viewer.world.json (requiresInteract); whether an interaction is actually offered stays with
the Server's S2C interact prompt, which CPlayerController answers. */
class CInteractKeyPromptView final
{
public:
	void Initialize(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iOwnerLevelIndex, const char* pAreaId);
	/* Places the keycap over the nearest interact-gated box within range of the local
	   player, or hides it. bShown false hides it outright (award page, cutscene). */
	void Update(const std::shared_ptr<CCharacter>& pLocalCharacter, bool_t bShown);

private:
	struct TRIGGER
	{
		std::string strPlacementId;
		float3_t vPosition{};
	};

private:
	std::unique_ptr<CUILayoutRuntime>	m_pView;
	std::vector<TRIGGER>				m_Triggers;
};

NS_END
