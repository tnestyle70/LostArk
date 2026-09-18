#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "UIWindowDrag.h"

#include <memory>
#include <string>
#include <vector>

NS_BEGIN(Client)

class CUILayoutRuntime;
struct HUD_PLAYER_STATE;

/* Retail honor title window (honortitle.gfx honorTitleWnd) rebuilt as CUI_Sprite slots
(Data/UI/HonorTitle/HonorTitle_Layout.json, written by Tools/LpkPipeline/build_honor_title_data.py)
under LEVEL::STATIC, the same construction as CVehicleWindowView.

Scope is the project's completed-title list (Data/Titles/HonorTitles.json through
CHonorTitleCatalog): one HonorTitleRendererItem_big row per title (name, the using badge on the
worn one), the current-title line, applyTitleBtn / applyDeselectBtn and the close X / Esc. The
retail tabs (completed / prefix / suffix), search, filters, tendency points and the acquisition panel are
not placed. The list scrolls with the mouse wheel and the DefaultEFScrollBarSmall_V2 bar
(track, thumb drag, arrow / track clicks) like the retail one.

The window only decides *what* to ask for: Take_TitleRequest hands the title id (0 = take the
title off) to CMainApp, which submits it through CPlayerController::Request_HonorTitle so the
Server round trip and pending sequence stay in one place. The worn title itself comes back in the
player snapshot (HUD_PLAYER_STATE::iHonorTitleId). */
class CHonorTitleWindowView final
{
public:
	/* Cancel transient gestures without closing or moving the window. */
	void Cancel_Interaction() { m_Drag.Reset(); m_bDraggingThumb = false; m_bEscapeDownLastFrame = true; }
	CHonorTitleWindowView(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	~CHonorTitleWindowView();

public:
	bool_t Is_Open() const { return m_bOpen; }
	/* Screen-pixel rect of the panel while open: CMainApp clips the labels of the windows drawn
	underneath out of it, so no text ever shows through a window on top. */
	bool_t Get_ScreenRect(f32_t& fX, f32_t& fY, f32_t& fWidth, f32_t& fHeight) const;
	/* Opening selects the worn title's row and scrolls it into view (on the next Update). */
	void Toggle();
	void Close() { m_bOpen = false; }

	/* Per frame from Update_CombatHUD while its level/player gate passes. Hover / selected
	skins, badge, buttons, wheel scroll, clicks, Esc. No-op (hides) while closed. */
	void Update(f32_t fTimeDelta, const HUD_PLAYER_STATE& Player);
	/* LOA-font text pass -- after CImGuiLayer::EndFrame() like the other runtime windows. */
	void Render_Text();
	/* Forces every owned sprite invisible without touching m_bOpen. */
	void Hide();
	/* True once per apply / deselect click; outTitleId is the catalog title to wear, 0 to take
	the current one off. */
	bool_t Take_TitleRequest(uint32_t& outTitleId);

private:
	void Update_Chrome();
	void Update_Rows(const HUD_PLAYER_STATE& Player);
	/* Wheel, arrow buttons, track paging and thumb drag; lays the thumb out for m_iScroll. */
	void Update_Scroll();
	void Update_Buttons(const HUD_PLAYER_STATE& Player);
	bool_t Get_WindowOrigin(f32_t& fX, f32_t& fY) const;
	/* Retail px relative to the window origin -> reference-resolution px. */
	f32_t Ref_X(f32_t fRetailX) const;
	f32_t Ref_Y(f32_t fRetailY) const;
	f32_t Draw_Label(const wstring_t& strFont, const wstring& strText, f32_t fRetailX,
		f32_t fRetailY, f32_t fRetailPx, fvector_t vColor, const float2_t& vOrigin);
	int32_t Row_Count() const;
	int32_t Max_Scroll() const;

private:
	ComPtr<ID3D11Device>			m_pDevice;
	ComPtr<ID3D11DeviceContext>		m_pContext;
	unique_ptr<CUILayoutRuntime>	m_pView;
	vector<string>					m_SlotIds;
	/* Retail px -> reference px, read back from the HT_WinBg slot (the layout document owns
	how large the window is drawn). */
	f32_t							m_fRetailScale = 2.f / 3.f;
	CUIWindowDrag					m_Drag;

	bool_t m_bOpen = false;
	bool_t m_bSelectWornOnUpdate = false;
	bool_t m_bEscapeDownLastFrame = false;
	/* Index into CHonorTitleCatalog::Get_Titles(); -1 = nothing selected. */
	int32_t m_iSelectedTitle = -1;
	int32_t m_iHoveredTitle = -1;
	/* First catalog row shown in the top list slot. */
	int32_t m_iScroll = 0;
	/* Thumb drag: the row offset the thumb had when the button went down and the cursor's
	reference-px Y at that moment. */
	bool_t m_bDraggingThumb = false;
	f32_t m_fThumbDragMouseY = 0.f;
	int32_t m_iThumbDragScroll = 0;
	/* Player.iHonorTitleId as of the last Update, for the badge and the label pass. */
	uint32_t m_iWornTitleId = 0u;
	bool_t m_bApplyEnabled = false;
	bool_t m_bRemoveEnabled = false;
	bool_t m_bTitleRequested = false;
	uint32_t m_iRequestedTitleId = 0u;
};

NS_END
