#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "CharacterSpec.h"
#include "Network/PacketMessages.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

NS_BEGIN(Client)

class CCharacter;
class CCharacterPortraitRenderer;
class CUILayoutRuntime;
struct HUD_PLAYER_STATE;

/* Retail character info window (P): the equipment tab of characterinfo.gfx rebuilt as real
CUI_Sprite slots (Data/UI/CharacterInfo/CharacterInfo_Layout.json, CHUDLayoutTool's
"Character Info" document) under LEVEL::STATIC, the same construction as CInventoryView.

Every rect in the layout is the retail 1920x1080 placement scaled by 2/3 around a window
origin, so the text pass below keeps working in retail px relative to that origin (the CI_WinBg
slot) and converts once. The live portrait is the original's CharInfoTexture render target:
the replicated local character's own parts are drawn a second time into m_pPortrait* with the
binary mesh shaders' forward ScreenCutin passes and a camera parked in front of the character,
then shown through the CI_Preview slot (CUILayoutRuntime::Set_SlotTextureSRV). Dragging the
portrait orbits that camera; nothing about the character itself changes.

What is real data: nickname (replicated CCharacter), class (HUD snapshot), max HP (HUD snapshot)
and attack power (PlayerProfiles.json via CCombatHUDViewModel). Everything the project has no
Server truth for yet (item level, combat power, the six combat stats, expedition level, title/
guild/estate rows, ark passive points, which equipment icons fill the slots) comes from
Data/UI/CharacterInfo/CharacterInfoDisplay.json so the numbers are editable data, not code. */
class CCharacterInfoWindowView final
{
public:
	CCharacterInfoWindowView(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	~CCharacterInfoWindowView();

public:
	bool_t Is_Open() const { return m_bOpen; }
	void Toggle() { m_bOpen = !m_bOpen; }
	void Close() { m_bOpen = false; }

	/* Per frame while the combat HUD's own level/player gate passes (Update_CombatHUD). Drives
	slot visibility, hover art, clicks, title-bar drag and the portrait orbit; remembers the
	character for Render_Portrait. No-op (hides) while closed. */
	void Update(f32_t fTimeDelta, const std::shared_ptr<CCharacter>& pLocalCharacter,
		const HUD_PLAYER_STATE& Player);
	/* Draws the live portrait into the window's own render target. Must run inside the frame
	after CGameInstance::Render_Begin and before CGameInstance::Render (the UI sprites sample the
	target during that world render); restores every pipeline binding it touches. */
	HRESULT Render_Portrait();
	/* LOA-font text pass -- after CImGuiLayer::EndFrame() like CInventoryView::Render_Text. */
	void Render_Text();
	/* Forces every owned sprite invisible without touching m_bOpen (see CInventoryView::Hide). */
	void Hide();
	/* True once per click on the avatar-page bottom-left button (the retail equipAvatarBookButton);
	CMainApp opens the avatar book with it. */
	bool_t Take_AvatarBookRequest();
	/* While another runtime window (the avatar book) is drawn over this one: this window stops
	registering itself as the router's top window and its own labels skip the covered area. */
	void Set_Covered(bool_t bCovered) { m_bCovered = bCovered; }
	/* CharacterInfoDisplay.json data the avatar book shares: which avatar-page slot is the head /
	outfit slot (-1 = none) and the catalog item id worn in that avatar kind per class ("" = none). */
	int32_t Get_AvatarSlotIndex(EQUIPMENT_SLOT_KIND eKind) const;
	const wstring& Get_ClassDisplayName(LostArk::Shared::CHARACTER_CLASS_ID eClass) const;
	const string& Get_EquippedAvatarItemId(LostArk::Shared::CHARACTER_CLASS_ID eClass,
		EQUIPMENT_SLOT_KIND eKind) const;

private:
	struct CLASS_DISPLAY
	{
		wstring strDisplayName;
		string strSymbolPath;
		/* Data/Items/ItemCatalog.json item id per equipment slot (weapon, helmet, shoulder, top,
		pants, gloves, necklace, earring1, earring2, ring1, ring2, stone, bracelet). An empty or
		unknown id leaves that slot as its silhouette; icon and grade art come from the catalog. */
		std::unordered_map<string, string> EquippedItemIds;
		/* Same for the avatar page: "head" (AVATAR_HEAD) and "outfit" (AVATAR_ARMOR). The icon
		shows only when the character's spec actually carries that avatar part. */
		std::unordered_map<string, string> EquippedAvatarItemIds;
	};

	struct STAT_ROW
	{
		wstring strName;
		wstring strValue;
	};

	struct DISPLAY_DATA
	{
		bool_t isLoaded = false;
		wstring strItemLevel;
		wstring strCombatPower;
		wstring strCharacterLevel;
		wstring strExpeditionLevel;
		wstring strTitle;
		wstring strGuild;
		wstring strEstate;
		wstring strPresetName;
		wstring strHonor;
		vector<STAT_ROW> CombatStats;
		vector<wstring> ArkPassivePoints;
		f32_t fPortraitDistance = 3.2f;
		f32_t fPortraitEyeHeight = 1.05f;
		f32_t fPortraitLookHeight = 0.95f;
		f32_t fPortraitFovDegrees = 35.f;
		f32_t fPortraitYawDegrees = 0.f;
		/* Which of the eight avatar page slots (CI_AvatarFrame_k) is the head / outfit slot. */
		int32_t iAvatarSlotHead = -1;
		int32_t iAvatarSlotOutfit = -1;
		std::unordered_map<LostArk::Shared::CHARACTER_CLASS_ID, CLASS_DISPLAY> Classes;
	};

private:
	void Load_DisplayData();
	void Update_Visibility();
	void Update_Chrome();
	void Update_Tabs();
	void Update_EquipToggle();
	void Update_Buttons();
	void Update_EquipmentIcons(LostArk::Shared::CHARACTER_CLASS_ID eClass);
	/* Avatar page: icon per equipped avatar part and the retail eye toggle that hides/shows
	that part in the portrait only (m_iPreviewHiddenAvatarKinds). */
	void Update_AvatarSlots(const std::shared_ptr<CCharacter>& pLocalCharacter,
		LostArk::Shared::CHARACTER_CLASS_ID eClass);
	void Update_PortraitDrag();
	void Update_PanelDrag();
	/* Keeps the whole window inside the reference resolution (a title-bar drag could otherwise
	park it off screen, where it toggles with P but is never seen). */
	void Clamp_ToScreen();
	bool_t Get_WindowOrigin(f32_t& fX, f32_t& fY) const;
	/* Retail px relative to the window origin -> reference-resolution px. */
	f32_t Ref_X(f32_t fRetailX) const;
	f32_t Ref_Y(f32_t fRetailY) const;
	void Draw_Label(const wstring_t& strFont, const wstring& strText, f32_t fRetailX,
		f32_t fRetailY, f32_t fRetailPx, fvector_t vColor, const float2_t& vOrigin);
	static void Set_HoverArt(CUILayoutRuntime& View, const string& strId, bool_t bHovered,
		const char* pNormalPath, const char* pOverPath);

private:
	ComPtr<ID3D11Device>			m_pDevice;
	ComPtr<ID3D11DeviceContext>		m_pContext;
	unique_ptr<CUILayoutRuntime>	m_pView;
	vector<string>					m_SlotIds;
	DISPLAY_DATA					m_Display;

	bool_t m_bOpen = false;
	bool_t m_bCovered = false;
	bool_t m_bAvatarBookRequested = false;
	int32_t m_iSelectedTab = 0;
	bool_t m_bAvatarMode = false;
	bool_t m_bDraggingPanel = false;
	bool_t m_bDraggingPortrait = false;
	f32_t m_fLastDragMouseX = 0.f;
	f32_t m_fLastDragMouseY = 0.f;
	f32_t m_fPortraitYawDegrees = 0.f;
	/* Eye toggles on the avatar page: bit (1 << ETOUI(EQUIPMENT_SLOT_KIND)) per avatar kind the
	portrait draws without. Preview only (CCharacter::Render_PreviewParts); the real character
	keeps its avatar until an actual unequip. Cleared when the window closes. */
	uint32_t m_iPreviewHiddenAvatarKinds = 0u;

	std::weak_ptr<CCharacter>		m_pPortraitCharacter;
	LostArk::Shared::CHARACTER_CLASS_ID m_ePortraitClass =
		LostArk::Shared::CHARACTER_CLASS_ID::END;
	wstring							m_strNickName;
	uint32_t						m_iMaximumHp = 0;
	uint32_t						m_iAttackPower = 0;
	bool_t							m_bHasAttackPower = false;

	unique_ptr<CCharacterPortraitRenderer>	m_pPortrait;
	bool_t m_bPortraitValid = false;
};

NS_END
