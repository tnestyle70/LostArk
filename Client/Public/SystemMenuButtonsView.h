#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <memory>

NS_BEGIN(Client)

class CUILayoutRuntime;

/* The two icon buttons at the bottom right of a world: the options window (the same one Escape
opens) and the way back to character select. The art is the retail game menu's own bottom bar
icons (ExitMenuBottomButton_icon_9 and _31); rects come from
Data/UI/SystemMenu/SystemMenuButtons_Layout.json and draw as CUI_Sprite through CUILayoutRuntime.

Presentation and hit-testing only: the view reports which icon was clicked and the owning Level
carries it out, so nothing here talks to the network or changes a Level. */
class CSystemMenuButtonsView final
{
public:
	enum class INTENT { NONE, OPEN_OPTIONS, RETURN_TO_CHARACTER_SELECT };

public:
	void Initialize(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iOwnerLevelIndex);
	/* Shows or hides both icons; a hidden view reads no input. Returns this frame's click. */
	INTENT Update(bool_t bVisible);
	/* True while the cursor is over an icon, so the owning Level keeps that click away from
	gameplay (a left click would otherwise also attack). */
	bool_t Is_PointerOver() const { return m_bPointerOver; }
	/* The caption under each icon, in the LOA font; call from the Level's text pass. */
	void Render_Text() const;

private:
	unique_ptr<CUILayoutRuntime> m_pView;
	bool_t m_bShown = true;
	bool_t m_bPointerOver = false;
};

NS_END
