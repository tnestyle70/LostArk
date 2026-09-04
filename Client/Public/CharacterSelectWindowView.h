#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <memory>

NS_BEGIN(Client)

class CUILayoutRuntime;

/* Retail-style full-screen character-select window, opened by the Lobby's
   "게임 시작" product button and shown over the Lobby (Phase 1 of the start-sequence
   rework -- see .md/TJ/09-03/2026-09-03_캐릭터선택창_PLAN.md). Traced from the real
   client's characterselect.gfx: wallpaper, bottom gradient bar, the horizontal
   character card bar (all six slots empty -- each empty slot is the source's own
   addSlotMc "신규 캐릭터 생성" card, whose click is what RequestNewCharacter maps to
   in the original), the center "게임 시작" plate and the bottom-left "서버 선택"
   back button. No account system exists, so the card bar carries no real characters
   and "게임 시작" stays disabled.

   Same ownership pattern as CRaidEntryPreviewView: every slot is a real CUI_Sprite
   on LEVEL::STATIC's Layer_UI via CUILayoutRuntime, and this view only reports the
   user's intent -- CMainApp consumes it and submits the matching Lobby command
   (the view never touches CLobbyCommandService or sockets itself). */
class CCharacterSelectWindowView
{
public:
	CCharacterSelectWindowView(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iOwnerLevelIndex);
	~CCharacterSelectWindowView();

public:
	void Open();
	void Close();
	bool_t Is_Open() const { return m_isOpen; }

	/* 이번 프레임의 사용자 의도. NEW_CHARACTER = 빈 슬롯 카드 클릭(캐릭터 생성 화면 진입),
	   CLOSE = 서버 선택/ESC(로비 복귀). CMainApp이 Consume_Intent로 한 번 가져가 소비한다. */
	enum class INTENT { NONE, NEW_CHARACTER, CLOSE };
	INTENT Consume_Intent();

	/* Drives visibility, hover texture swaps and click hit-testing for one frame while
	   open (sprites themselves draw through the normal engine UI pipeline). Claims the
	   mouse for the whole frame -- the window is modal over the Lobby. */
	void Update(f32_t fTimeDelta);

	/* Draw_Text submits immediately (SpriteBatch), so this must run after
	   CImGuiLayer::EndFrame() -- same reasoning as every other Render_XText split. */
	void RenderText();

private:
	void Hide_AllSlots();

private:
	unique_ptr<CUILayoutRuntime> m_pView;
	bool_t m_isOpen = false;
	/* The opening click must not also hit whatever lands under the cursor inside the
	   window on that same frame. */
	bool_t m_hasJustOpened = false;
	bool_t m_wasEscapeDown = false;
	INTENT m_eIntent = INTENT::NONE;
	/* Card index the pointer is over this frame (-1 none) -- RenderText brightens that
	   card's label to match the hover art swap. */
	int32_t m_iHoveredCard = -1;
};

NS_END
