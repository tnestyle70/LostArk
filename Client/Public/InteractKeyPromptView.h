#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <memory>
#include <string>
#include <vector>

NS_BEGIN(Client)

class CUILayoutRuntime;
class CCharacter;

/* The retail interaction key prompt (EFUI_MASTERKEY MasterKeyComponent) while the Server is
offering an interact-gated trigger box: the action icon (iconType_mc, ~52 px), under it the
action name followed by the G keycap as one centred line (GameMsg
sys.tip.masterkey_string_combination "{0} {1}"), and the "show" glow on appear.

Placement follows MasterKeyFrame.updatePos: the host passes one screen point and the line's
bottom sits 50 px above it (PIVOT_TYPE_BOTTOM_LEFT, 0, 50, descriptionTF). The retail capture
puts that point at the player's head (the prompt sits right over the nameplate while the
player stands in the trigger ring), so the local character's head anchor is used.

Which action a box shows is not authored on our trigger boxes; it is read from the box's own
move: down -> godown, up -> climb, level -> tightrope, no move -> check. Presentation only: the
Server's prompt decides when it shows and the G press still goes through CPlayerController. */
class CInteractKeyPromptView final
{
public:
	void Initialize(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iOwnerLevelIndex, const char* pAreaId);
	/* strOfferedTriggerId: the box the Server is offering now (empty = none). */
	void Update(f32_t fTimeDelta, const std::shared_ptr<CCharacter>& pLocalCharacter,
		const std::string& strOfferedTriggerId, bool_t bShown);
	/* Action name line; HUD text layer. Call from the Level's Render. */
	void Render_Text() const;

private:
	enum class ACTION : uint8_t { GODOWN, CLIMB, TIGHTROPE, CHECK };
	struct TRIGGER
	{
		std::string strPlacementId;
		ACTION eAction = ACTION::CHECK;
	};
	void Hide();

private:
	std::unique_ptr<CUILayoutRuntime>	m_pView;
	std::vector<TRIGGER>				m_Triggers;
	std::string							m_strShownTriggerId;
	ACTION								m_eAction = ACTION::CHECK;
	f32_t								m_fShowSeconds = -1.f;
	/* Screen px of the text line (centre x, centre y) and its line height, set in Update. */
	f32_t								m_fTextCenterX = 0.f;
	f32_t								m_fTextCenterY = 0.f;
	f32_t								m_fTextPx = 0.f;
	f32_t								m_fTextOffsetX = 0.f;
	bool_t								m_bVisible = false;
};

NS_END
