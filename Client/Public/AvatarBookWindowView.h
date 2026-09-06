#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "CharacterSpec.h"
#include "Network/PacketMessages.h"

#include <chrono>
#include <memory>
#include <string>
#include <vector>

NS_BEGIN(Client)

class CCharacter;
class CCharacterInfoWindowView;
class CCharacterPortraitRenderer;
class CUILayoutRuntime;
struct HUD_PLAYER_STATE;
struct ITEM_DEFINITION;

/* Retail avatar book (avatarbook.gfx AvatarBookWindowContent 1532x800) rebuilt as
CUI_Sprite slots (Data/UI/AvatarBook/AvatarBook_Layout.json) under LEVEL::STATIC, opened from the
character info window's avatar-page button. Same construction as CCharacterInfoWindowView: every
rect is the retail placement scaled by 2/3 around the AB_WinBg origin, the text pass works in
retail px and converts once, and the live mannequin is a CCharacterPortraitRenderer target shown
through the AB_Preview slot.

Try-on model (the retail flow): the grid lists the avatar items the catalog has for the current
class (grade "avatar"); clicking one puts it on the mannequin, the preview slots' eye toggles take
it off again, all as a preview mask on the portrait only. Revert resets the preview to what the
character really wears, auto-equip wears everything, save commits the preview to the world
character; a double click on a grid avatar equips it and a double click on a worn mannequin slot
unequips it -- CCharacter::Set_AvatarPartVisible is the only path that changes the real character.
Collections/favorites/shop/craft/tint/auction have no Server data yet and are press feedback. */
class CAvatarBookWindowView final
{
public:
	CAvatarBookWindowView(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	~CAvatarBookWindowView();

public:
	bool_t Is_Open() const { return m_bOpen; }
	void Open();
	void Close() { m_bOpen = false; }

	/* Per frame while the combat HUD's own level/player gate passes (after the character info
	window's Update, which owns the slot-index/item-id data this window shares). */
	void Update(f32_t fTimeDelta, const std::shared_ptr<CCharacter>& pLocalCharacter,
		const HUD_PLAYER_STATE& Player, const CCharacterInfoWindowView& InfoView);
	/* Inside the frame after Render_Begin and before CGameInstance::Render (see the portrait
	renderer). */
	HRESULT Render_Portrait();
	/* LOA-font text pass, after CImGuiLayer::EndFrame(). */
	void Render_Text();
	void Hide();

private:
	struct AVATAR_ENTRY
	{
		const ITEM_DEFINITION* pItem = nullptr;
		EQUIPMENT_SLOT_KIND eKind = EQUIPMENT_SLOT_KIND::DEFAULT;
		int32_t iPreviewSlot = -1;   /* mannequin slot index (AB_SlotBg_k), -1 = none */
		int32_t iShowCheck = -1;     /* eye toggle beside that slot (AB_ShowCheck_k), -1 = none */
	};

	struct DISPLAY_DATA
	{
		vector<wstring> TabNames;
		wstring strCategoryCombo;
		wstring strAlignCombo;
		wstring strSearchPlaceholder;
		wstring strFavoriteCount;
		wstring strAvatarPoint;
		f32_t fPortraitDistance = 3.4f;
		f32_t fPortraitEyeHeight = 1.05f;
		f32_t fPortraitLookHeight = 0.95f;
		f32_t fPortraitFovDegrees = 35.f;
		f32_t fPortraitYawDegrees = 0.f;
		int32_t iSlotHead = -1;
		int32_t iSlotOutfit = -1;
		int32_t iShowCheckHead = -1;
		int32_t iShowCheckOutfit = -1;
	};

private:
	void Load_DisplayData();
	void Rebuild_Entries(const std::shared_ptr<CCharacter>& pLocalCharacter,
		LostArk::Shared::CHARACTER_CLASS_ID eClass, const CCharacterInfoWindowView& InfoView);
	void Reset_PreviewToReal(const std::shared_ptr<CCharacter>& pLocalCharacter);
	void Update_Visibility();
	void Update_PanelDrag();
	/* Keeps the whole window inside the reference resolution (a title-bar drag could otherwise
	park it off screen, where it toggles with P but is never seen). */
	void Clamp_ToScreen();
	void Update_Chrome();
	void Update_Tabs();
	void Update_Grid();
	void Update_PreviewSlots(const std::shared_ptr<CCharacter>& pLocalCharacter);
	void Update_Buttons(const std::shared_ptr<CCharacter>& pLocalCharacter);
	void Update_PortraitDrag();
	/* True when this click on strId is the second within DOUBLE_CLICK_SECONDS. */
	bool_t Register_Click(const string& strId);
	bool_t Get_WindowOrigin(f32_t& fX, f32_t& fY) const;
	f32_t Ref_X(f32_t fRetailX) const;
	f32_t Ref_Y(f32_t fRetailY) const;
	void Draw_Label(const wstring_t& strFont, const wstring& strText, f32_t fRetailX,
		f32_t fRetailY, f32_t fRetailPx, fvector_t vColor, const float2_t& vOrigin);

private:
	ComPtr<ID3D11Device>			m_pDevice;
	ComPtr<ID3D11DeviceContext>		m_pContext;
	unique_ptr<CUILayoutRuntime>	m_pView;
	vector<string>					m_SlotIds;
	DISPLAY_DATA					m_Display;
	unique_ptr<CCharacterPortraitRenderer>	m_pPortrait;

	bool_t m_bOpen = false;
	bool_t m_bJustOpened = false;
	bool_t m_bDraggingPanel = false;
	bool_t m_bDraggingPortrait = false;
	f32_t m_fLastDragMouseX = 0.f;
	f32_t m_fLastDragMouseY = 0.f;
	f32_t m_fPortraitYawDegrees = 0.f;
	int32_t m_iSelectedTab = 0;
	int32_t m_iSelectedCell = -1;
	int32_t m_iSelectedCamera = 0;
	/* Bit (1 << ETOUI(EQUIPMENT_SLOT_KIND)) per avatar kind the mannequin draws without. */
	uint32_t m_iPreviewHiddenAvatarKinds = 0u;

	vector<AVATAR_ENTRY>			m_Entries;
	std::chrono::steady_clock::time_point m_LastClickTime{};
	string							m_strLastClickId;
	std::weak_ptr<CCharacter>		m_pPortraitCharacter;
	LostArk::Shared::CHARACTER_CLASS_ID m_eClass = LostArk::Shared::CHARACTER_CLASS_ID::END;
	wstring							m_strClassName;
	wstring							m_strNickName;
	bool_t							m_bPortraitValid = false;
};

NS_END
