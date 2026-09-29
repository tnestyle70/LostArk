#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Network/PacketType.h"

#include <memory>
#include <string>

NS_BEGIN(Client)

class CCharacter;
class CCharacterPortraitRenderer;
class CPlayableCharacterAssetService;
class CUILayoutRuntime;

/* Retail-style full-screen character-select window, opened by the Lobby's
   "server select" product button and shown over the Lobby. Traced from the real client's
   characterselect.gfx: wallpaper, bottom gradient bar, the horizontal character card bar,
   the center "game start" plate, the bottom-left "server select" back button and, bottom
   right, the rename and options icon buttons.

   The first cards seat the local roster (CCharacterRoster: class + nickname, no account
   system); the remaining cards are the source's addSlotMc "new character" card. Selecting a
   seated card enables "game start", which enters Bern as that character; the rename icon
   edits the selected card's nickname in a small dialog.

   Above the card bar the seated characters stand side by side. Each is a real CCharacter on a
   LEVEL::LOBBY layer, hidden from the world pass and drawn through CCharacterPortraitRenderer
   into its CharSel_StagePortrait_N slot. The class models are prepared one at a time off the
   main thread once the window first opens, so a character appears when its model is ready.

   Same ownership pattern as CRaidEntryPreviewView: every slot is a real CUI_Sprite on
   LEVEL::STATIC's Layer_UI via CUILayoutRuntime, and this view only reports the user's
   intent -- CMainApp consumes it and submits the matching Lobby command (the view never
   touches CLobbyCommandService or sockets itself). */
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

	/* This frame's intent, taken once by CMainApp through Consume_Intent.
	   NEW_CHARACTER = an empty card (enter character creation), START_CHARACTER = "game start"
	   on a seated card (Get_StartCharacter names it), OPEN_OPTIONS = the options icon,
	   CLOSE = server select / ESC. */
	enum class INTENT { NONE, NEW_CHARACTER, START_CHARACTER, OPEN_OPTIONS, CLOSE };
	INTENT Consume_Intent();
	int32_t Get_CreationSlot() const { return m_iCreationSlot; }
	/* The seated card "game start" was pressed on. */
	void Get_StartCharacter(LostArk::Shared::CHARACTER_CLASS_ID& outClass,
		std::string& outNickname, std::string& outAppearanceJson, std::string& outCharacterId) const;

	/* Drives visibility, hover texture swaps and click hit-testing for one frame while
	   open (sprites themselves draw through the normal engine UI pipeline). Claims the
	   mouse for the whole frame -- the window is modal over the Lobby. bInputBlocked keeps
	   everything drawn but ignores the pointer and ESC (the options window is on top). */
	void Update(f32_t fTimeDelta, bool_t bInputBlocked = false);

	/* Queues this frame's stage portraits and points their slots at them. Runs between
	   CGameInstance::Render_Begin and the world render, like every other portrait. */
	void Render_Portraits();
	/* Called every frame the Lobby is not the current level: a pending class preparation is
	   cancelled without committing, and the characters (gone with the Lobby's layers) are
	   forgotten so the next visit prepares them again. */
	void Release_Stage();

	/* Draw_Text submits immediately (SpriteBatch), so this must run after
	   CImGuiLayer::EndFrame() -- same reasoning as every other Render_XText split. */
	void RenderText();

private:
	void Hide_AllSlots();
	void Update_Cards(bool_t bAcceptClicks);
	void Update_IconButtons(bool_t bAcceptClicks);
	void Update_RenameDialog();
	void Open_RenameDialog();
	void Close_RenameDialog();
	void Render_RenameDialogText(f32_t fScaleX, f32_t fScaleY, f32_t fUiScale);
	void Update_Stage();
	void Spawn_StageCharacter(int32_t iIndex);

private:
	static constexpr int32_t STAGE_COUNT = 6;

	ComPtr<ID3D11Device> m_pDevice;
	ComPtr<ID3D11DeviceContext> m_pContext;
	unique_ptr<CUILayoutRuntime> m_pView;
	bool_t m_isOpen = false;
	/* The opening click must not also hit whatever lands under the cursor inside the
	   window on that same frame. */
	bool_t m_hasJustOpened = false;
	bool_t m_wasEscapeDown = false;
	bool_t m_wasInputBlocked = false;
	INTENT m_eIntent = INTENT::NONE;
	/* Card index the pointer is over this frame (-1 none) -- RenderText brightens that
	   card's label to match the hover art swap. */
	int32_t m_iHoveredCard = -1;
	/* Seated card picked by the last click (-1 none). */
	int32_t m_iSelectedCard = -1;
	int32_t m_iCreationSlot = -1;
	bool_t m_bRenameHovered = false;
	bool_t m_bOptionHovered = false;

	bool_t m_isRenameOpen = false;
	std::wstring m_strRenameDraft;
	std::string m_strRenameStatus;

	/* Stage characters, by card index. Preparation starts on the first open and runs one class
	at a time; a class that fails is skipped for the rest of this Lobby visit. */
	bool_t m_bStageRequested = false;
	bool_t m_bStageCatalogsReady = false;
	unique_ptr<CPlayableCharacterAssetService> m_pStageAssets;
	int32_t m_iStagePreparingIndex = -1;
	std::weak_ptr<CCharacter> m_StageCharacters[STAGE_COUNT];
	bool_t m_bStageFailed[STAGE_COUNT] = {};
	unique_ptr<CCharacterPortraitRenderer> m_pStagePortraits[STAGE_COUNT];
	/* The authored portrait rects, read once: Render_Portraits moves a slot's drawn rect to crop
	it, so the layout's own rect is kept here as the source of truth. */
	struct STAGE_RECT { f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f; bool_t bValid = false; };
	STAGE_RECT m_StageSlotRects[STAGE_COUNT];
};

NS_END
