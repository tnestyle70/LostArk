#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "UIWindowDrag.h"

#include <array>
#include <memory>
#include <string>
#include <vector>

NS_BEGIN(Client)

class CCharacter;
class CUILayoutRuntime;
struct HUD_PLAYER_STATE;

/* One VehicleUiCatalog.json skills[] row: the key the Server binds it to, its id, the
cooldown the Server applies (for the HUD pie) and the retail icon cut for it. */
struct VEHICLE_SKILL_UI
{
	string strSlot;
	uint32_t iSkillId = 0u;
	uint32_t iCooldownMs = 0u;
	string strIconAsset;
};

/* Retail vehicle window (vehicle.gfx vehicleWnd) rebuilt as CUI_Sprite slots
(Data/UI/Vehicle/Vehicle_Layout.json, written by Tools/LpkPipeline/build_vehicle_ui.py) under
LEVEL::STATIC, the same construction as CCharacterInfoWindowView.

Scope is the project's catalog vehicles: one VehicleListItem row each (icon, name,
description, move speed, the "(mounted)" ribbon), the confirmBtn (mount / dismount) and the
closeBtnDummy, X / Esc to close. The retail tabs, search, sort, bookmarks and summon-motion panel
are not placed. Rows come from Data/UI/Vehicle/VehicleUiCatalog.json (names, descriptions, icons
from the retail tables) joined to CActorCatalog::Get_Vehicles(); a row whose vehicle has no
riding pose for the local class shows the disabled button, the same rule CPlayerController's H
key applies.

The window only decides *what* to ask for: Take_RidingRequest hands the vehicle id (0 = dismount)
to CMainApp, which submits it through CPlayerController::Request_VehicleRiding so the Server
round trip, pending sequence and result logging stay in one place. Selecting a row also becomes
the H key's preferred vehicle. */
class CVehicleWindowView final
{
public:
	CVehicleWindowView(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	~CVehicleWindowView();

public:
	bool_t Is_Open() const { return m_bOpen; }
	void Toggle() { m_bOpen = !m_bOpen; }
	void Close() { m_bOpen = false; }

	/* Per frame from Update_CombatHUD while its level/player gate passes. Hover / selected
	skins, button art, clicks, Esc. No-op (hides) while closed. */
	void Update(f32_t fTimeDelta, const std::shared_ptr<CCharacter>& pLocalCharacter,
		const HUD_PLAYER_STATE& Player);
	/* LOA-font text pass -- after CImGuiLayer::EndFrame() like the other runtime windows. */
	void Render_Text();
	/* Forces every owned sprite invisible without touching m_bOpen. */
	void Hide();
	/* True once per mount / dismount click; outVehicleId is the catalog vehicle to ride, 0 to
	dismount. */
	bool_t Take_RidingRequest(uint32_t& outVehicleId);
	/* One-shot: true once after a click on a row's icon -- the vehicle is picked up for the
	click-to-carry quick-slot binding (CQuickSlotDragView); the row is selected as well. */
	bool_t Take_IconPick(uint32_t& outVehicleId, string& outIconPath);
	/* Catalog icon asset for a vehicle (nullptr when the catalog has no row) -- the 5..0 quick
	slots show it. */
	const string* Find_IconAsset(uint32_t iVehicleId) const;
	/* The vehicle's Server-contract skills (VehicleProfiles.json SPACE/Q/W/E) with their icons
	and cooldowns, for the mounted HUD (nullptr when the catalog has no row). */
	const std::vector<VEHICLE_SKILL_UI>* Find_Skills(uint32_t iVehicleId) const;

private:
	struct VEHICLE_ROW
	{
		uint32_t iVehicleId = 0u;
		string strIconAsset;
		wstring strName;
		wstring strDescription;
		/* strDescription cut to the row's text column with a trailing ellipsis, for the
		viewport width it was measured at (m_fFitViewportWidth). */
		wstring strDescriptionFit;
		/* Mounted HUD actions (catalog `skills[]`: SPACE dash, Q/W/E). */
		std::vector<VEHICLE_SKILL_UI> Skills;
	};

private:
	void Load_Catalog();
	void Update_Rows(const std::shared_ptr<CCharacter>& pLocalCharacter,
		const HUD_PLAYER_STATE& Player);
	void Update_Buttons(const HUD_PLAYER_STATE& Player);
	void Update_Chrome();
	bool_t Get_WindowOrigin(f32_t& fX, f32_t& fY) const;
	/* Retail px relative to the window origin -> reference-resolution px. */
	f32_t Ref_X(f32_t fRetailX) const;
	f32_t Ref_Y(f32_t fRetailY) const;
	/* strText, or its longest prefix plus "..." that draws within fMaxRetailWidth at fRetailPx. */
	wstring Fit_Label(const wstring_t& strFont, const wstring& strText, f32_t fRetailPx,
		f32_t fMaxRetailWidth) const;
	void Refit_Descriptions();
	/* Draws one label and returns its on-screen width in retail px (for the mounted suffix). */
	f32_t Draw_Label(const wstring_t& strFont, const wstring& strText, f32_t fRetailX,
		f32_t fRetailY, f32_t fRetailPx, fvector_t vColor, const float2_t& vOrigin);

private:
	ComPtr<ID3D11Device>			m_pDevice;
	ComPtr<ID3D11DeviceContext>		m_pContext;
	unique_ptr<CUILayoutRuntime>	m_pView;
	vector<string>					m_SlotIds;
	vector<VEHICLE_ROW>				m_Rows;
	/* Retail px -> reference px, read back from the VH_WinBg slot (the layout document owns
	how large the window is drawn). */
	f32_t							m_fRetailScale = 2.f / 3.f;
	f32_t							m_fFitViewportWidth = 0.f;
	/* Title-band drag (the retail window moves by its header). */
	CUIWindowDrag					m_Drag;

	wstring m_strTitle;
	wstring m_strMount;
	wstring m_strDismount;
	wstring m_strClose;
	wstring m_strMountedSuffix;
	wstring m_strHint;

	bool_t m_bOpen = false;
	bool_t m_bEscapeDownLastFrame = false;
	int32_t m_iSelectedRow = 0;
	int32_t m_iHoveredRow = -1;
	/* Button state this frame, for the label pass: false = the selected vehicle has no riding
	pose for the local class (or no row at all). */
	bool_t m_bMountEnabled = false;
	bool_t m_bSelectedIsMounted = false;
	/* Player.iVehicleId as of the last Update, for the label pass. */
	uint32_t m_iMountedVehicleId = 0u;
	bool_t m_bRidingRequested = false;
	uint32_t m_iRequestedVehicleId = 0u;
	bool_t m_bIconPicked = false;
	uint32_t m_iPickedVehicleId = 0u;
};

NS_END
