/* WinSock2 ahead of everything else (Client_Defines pulls the socket headers). */
#include <WinSock2.h>

#include "VehicleWindowView.h"

#include <DirectXColors.h>

#include "ActorCatalog.h"
#include "Character.h"
#include "CombatHUDViewModel.h"
#include "DataJson.h"
#include "GameInstance.h"
#include "MainApp.h"
#include "PlayerController.h"
#include "ProjectDataRoot.h"
#include "UIInputRouter.h"
#include "UILabelFont.h"
#include "UITextOcclusion.h"
#include "UILayoutRuntime.h"

#include <cmath>
#include <fstream>

namespace
{
	/* vehicle.gfx is authored at 1920x1080; the layout document scales it onto the 1280x720
	reference (2/3 x the document's own enlargement), so every retail px offset below goes
	through m_fRetailScale, read from the VH_WinBg slot against this retail width. */
	constexpr f32_t TEXT_BOOST = 1.15f;
	constexpr int32_t ROW_COUNT = 6;
	/* VehicleListItem text placement (retail px inside the row): name (99,10), description
	(99,32), additionalStat right-aligned to x 427 -- see the 09-14 extraction doc 3.2. */
	constexpr f32_t ROW_X = 14.f;
	constexpr f32_t ROW_Y0 = 60.f;
	constexpr f32_t ROW_PITCH = 60.f;
	constexpr f32_t ROW_TEXT_X = 99.f;
	constexpr f32_t ROW_NAME_Y = 10.f;
	constexpr f32_t ROW_DESC_Y = 32.f;
	constexpr f32_t ROW_FONT_PX = 14.f;
	constexpr f32_t TITLE_X = 22.f;
	constexpr f32_t TITLE_Y = 12.f;
	constexpr f32_t TITLE_PX = 18.f;
	/* Header band of the stretched WindowBG_V2 (drag handle), retail px. */
	constexpr f32_t WINDOW_WIDTH = 480.f;
	constexpr f32_t TITLE_BAR_HEIGHT = 49.f;
	/* Rows run to the window's right padding (build_vehicle_ui.py ROW_W); text stops short of
	the row edge by this much. */
	constexpr f32_t ROW_W = WINDOW_WIDTH - ROW_X * 2.f;
	constexpr f32_t ROW_TEXT_RIGHT_PAD = 10.f;
	constexpr f32_t HINT_X = 17.f;
	/* Under the last row (build_vehicle_ui.py HINT_Y / BUTTON_Y). */
	constexpr f32_t HINT_Y = ROW_Y0 + ROW_PITCH * ROW_COUNT + 10.f;
	constexpr f32_t BUTTON_Y = HINT_Y + 30.f;
	constexpr f32_t BUTTON_W = 103.f;
	constexpr f32_t BUTTON_H = 36.f;
	constexpr f32_t MOUNT_BUTTON_X = 259.f;
	constexpr f32_t CLOSE_BUTTON_X = 365.f;
	constexpr f32_t BUTTON_PX = 16.f;

	const wstring_t FONT_YOON = TEXT("Font_YoonGasiIIM");
	const wstring_t FONT_YG760 = TEXT("Font_YG760");

	const fvector_t COLOR_TITLE = XMVectorSet(1.f, 247.f / 255.f, 226.f / 255.f, 1.f);        // #fff7e2
	const fvector_t COLOR_NAME = XMVectorSet(213.f / 255.f, 172.f / 255.f, 102.f / 255.f, 1.f);  // #d5ac66 (every row, user request)
	const fvector_t COLOR_DESC = XMVectorSet(145.f / 255.f, 156.f / 255.f, 166.f / 255.f, 1.f);  // #919ca6
	const fvector_t COLOR_MOUNTED = XMVectorSet(35.f / 255.f, 201.f / 255.f, 1.f, 1.f);         // #23c9ff
	const fvector_t COLOR_DISABLED = XMVectorSet(120.f / 255.f, 120.f / 255.f, 120.f / 255.f, 1.f);

	const char* ART_BTN_NORMAL = "UI/Vehicle/Vehicle_Btn_Normal.png";
	const char* ART_BTN_OVER = "UI/Vehicle/Vehicle_Btn_Over.png";
	const char* ART_BTN_DOWN = "UI/Vehicle/Vehicle_Btn_Down.png";
	const char* ART_BTN_DISABLED = "UI/Vehicle/Vehicle_Btn_Disabled.png";
	const char* ART_CLOSE_NORMAL = "UI/Vehicle/Vehicle_Close_Normal.png";
	const char* ART_CLOSE_OVER = "UI/Vehicle/Vehicle_Close_Over.png";

	bool_t ConvertUtf8ToWide(const string& strUtf8, wstring& outWide)
	{
		outWide.clear();
		if (strUtf8.empty())
			return true;
		const int32_t iLength = ::MultiByteToWideChar(
			CP_UTF8, 0, strUtf8.c_str(), -1, nullptr, 0);
		if (iLength <= 1)
			return false;
		outWide.assign(static_cast<size_t>(iLength - 1), L'\0');
		::MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, outWide.data(), iLength);
		return true;
	}

	/* GameMsg strings carry Scaleform HTML (<FONT COLOR=...>, <br>); this window draws plain
	labels, so tags go and line breaks become a space. */
	string Strip_Markup(const string& strText)
	{
		string strOut;
		strOut.reserve(strText.size());
		size_t i = 0;
		while (i < strText.size())
		{
			const char c = strText[i];
			if ('<' == c)
			{
				const size_t iEnd = strText.find('>', i);
				if (string::npos == iEnd)
					break;
				if (0 == strText.compare(i, 3, "<br") || 0 == strText.compare(i, 3, "<BR"))
					strOut.push_back(' ');
				i = iEnd + 1;
				continue;
			}
			strOut.push_back(('\n' == c || '\r' == c) ? ' ' : c);
			++i;
		}
		return strOut;
	}

	string Row_Slot(const int32_t iRow, const char* pSuffix)
	{
		return "VH_Row" + std::to_string(iRow) + "_" + pSuffix;
	}
}

Client::CVehicleWindowView::CVehicleWindowView(
	ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pView{ std::make_unique<CUILayoutRuntime>(
		pDevice, pContext, ETOUI(LEVEL::STATIC), TEXT("Layer_UI"),
		L"UI/Vehicle/Vehicle_Layout.json") }
{
	/* Draw order for this window's panel; the same number orders its labels
	(Register_UITextOccluders) and its clicks. */
	m_pView->Set_UISortLayer(UI_TEXT_LAYER::WINDOW_VEHICLE);
	m_SlotIds = m_pView->Get_SlotIds();
	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	if (m_pView->Get_SlotRect("VH_WinBg", fX, fY, fWidth, fHeight) && fWidth > 0.f)
		m_fRetailScale = fWidth / WINDOW_WIDTH;
	Load_Catalog();
	/* Same reason as CInventoryView: LEVEL::STATIC sprites are visible from construction. */
	Hide();
}

Client::CVehicleWindowView::~CVehicleWindowView() = default;

void Client::CVehicleWindowView::Load_Catalog()
{
	m_Rows.clear();
	const filesystem::path DataPath =
		CProjectDataRoot::Resolve(L"UI/Vehicle/VehicleUiCatalog.json");
	ifstream Stream(DataPath, ios::binary);
	if (!Stream.is_open())
	{
		OutputDebugStringA("[VehicleWindow] VehicleUiCatalog.json missing -- the list stays empty.\n");
		return;
	}
	const string Text((istreambuf_iterator<char>(Stream)), istreambuf_iterator<char>());
	DATA_JSON_VALUE Root;
	string Error;
	if (!CDataJson::Parse(Text, Root, Error) || !Root.Is_Object())
	{
		OutputDebugStringA(("[VehicleWindow] VehicleUiCatalog.json parse failed: " + Error + "\n").c_str());
		return;
	}

	const auto ReadText = [](const DATA_JSON_VALUE& Object, const char* pKey, wstring& outText)
	{
		const DATA_JSON_VALUE* pText = Object.Find(pKey);
		if (nullptr != pText && pText->Is_String())
			(void)ConvertUtf8ToWide(Strip_Markup(pText->Get_String()), outText);
	};
	if (const DATA_JSON_VALUE* pStrings = Root.Find("strings"); nullptr != pStrings && pStrings->Is_Object())
	{
		ReadText(*pStrings, "title", m_strTitle);
		ReadText(*pStrings, "mount", m_strMount);
		ReadText(*pStrings, "dismount", m_strDismount);
		ReadText(*pStrings, "close", m_strClose);
		ReadText(*pStrings, "mountedSuffix", m_strMountedSuffix);
		ReadText(*pStrings, "quickslotHint", m_strHint);
	}
	const DATA_JSON_VALUE* pVehicles = Root.Find("vehicles");
	if (nullptr == pVehicles || !pVehicles->Is_Array())
		return;
	for (const DATA_JSON_VALUE& Value : pVehicles->Get_Array())
	{
		if (!Value.Is_Object())
			continue;
		const DATA_JSON_VALUE* pId = Value.Find("vehicleId");
		if (nullptr == pId || !pId->Is_Number())
			continue;
		VEHICLE_ROW Row{};
		Row.iVehicleId = static_cast<uint32_t>(pId->Get_Number());
		/* Only vehicles the actor catalog can actually spawn get a row; the UI catalog is
		display data joined to that contract, not a second list of rideable things. */
		if (nullptr == CActorCatalog::Find_Vehicle(Row.iVehicleId))
			continue;
		if (const DATA_JSON_VALUE* pIcon = Value.Find("iconAsset"); nullptr != pIcon && pIcon->Is_String())
			Row.strIconAsset = pIcon->Get_String();
		ReadText(Value, "name", Row.strName);
		ReadText(Value, "description", Row.strDescription);
		if (const DATA_JSON_VALUE* pSkills = Value.Find("skills"); nullptr != pSkills && pSkills->Is_Array())
		{
			for (const DATA_JSON_VALUE& Skill : pSkills->Get_Array())
			{
				const DATA_JSON_VALUE* pSlot = Skill.Is_Object() ? Skill.Find("slot") : nullptr;
				const DATA_JSON_VALUE* pSkillId = Skill.Is_Object() ? Skill.Find("skillId") : nullptr;
				const DATA_JSON_VALUE* pCooldown = Skill.Is_Object() ? Skill.Find("cooldownMs") : nullptr;
				const DATA_JSON_VALUE* pAsset = Skill.Is_Object() ? Skill.Find("iconAsset") : nullptr;
				if (nullptr == pSlot || !pSlot->Is_String() || nullptr == pSkillId || !pSkillId->Is_Number() ||
					nullptr == pAsset || !pAsset->Is_String())
					continue;
				VEHICLE_SKILL_UI Entry{};
				Entry.strSlot = pSlot->Get_String();
				Entry.iSkillId = static_cast<uint32_t>(pSkillId->Get_Number());
				Entry.iCooldownMs = (nullptr != pCooldown && pCooldown->Is_Number()) ?
					static_cast<uint32_t>(pCooldown->Get_Number()) : 0u;
				Entry.strIconAsset = pAsset->Get_String();
				Row.Skills.push_back(std::move(Entry));
			}
		}
		m_Rows.push_back(std::move(Row));
		if (static_cast<int32_t>(m_Rows.size()) >= ROW_COUNT)
			break;
	}
}

void Client::CVehicleWindowView::Update(const f32_t fTimeDelta,
	const std::shared_ptr<CCharacter>& pLocalCharacter, const HUD_PLAYER_STATE& Player)
{
	/* Hit tests below belong to this window; the router refuses a press that lands on the
	window in front and lets only one widget take any one press. */
	CUIPointerScope PointerScope(this);
	(void)fTimeDelta;
	if (!m_bOpen)
	{
		Hide();
		m_Drag.Reset();
		return;
	}

	for (const string& strId : m_SlotIds)
		m_pView->Set_SlotVisible(strId, true);

	/* Header-band drag first: while a drag is live the rows / buttons below see no click. */
	m_Drag.Update(*m_pView, m_SlotIds, Ref_X(0.f), Ref_Y(0.f),
		WINDOW_WIDTH * m_fRetailScale, TITLE_BAR_HEIGHT * m_fRetailScale, "VH_Close");
	Update_Chrome();
	Update_Rows(pLocalCharacter, Player);
	Update_Buttons(Player);

	/* Anything over the window belongs to the window -- keeps a click on the panel from
	turning into a gameplay move command underneath. */
	CUIInputRouter& Router = CUIInputRouter::Get();
	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	if (m_pView->Get_SlotRect("VH_WinBg", fX, fY, fWidth, fHeight) &&
		Router.Is_Hovered(fX, fY, fWidth, fHeight,
			m_pView->Get_ResolutionWidth(), m_pView->Get_ResolutionHeight()))
	{
		Router.Claim_Mouse_This_Frame();
	}
}

void Client::CVehicleWindowView::Update_Chrome()
{
	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pView->Get_ResolutionHeight();
	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	if (!m_pView->Get_SlotRect("VH_Close", fX, fY, fWidth, fHeight))
		return;
	const bool_t bHovered = Router.Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight);
	m_pView->Set_SlotTexture("VH_Close", bHovered ? ART_CLOSE_OVER : ART_CLOSE_NORMAL);
	if (bHovered && Router.Is_Clicked(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
	{
		CMainApp::Play_UIButtonClickSound();
		Close();
	}
}

void Client::CVehicleWindowView::Update_Rows(const std::shared_ptr<CCharacter>& pLocalCharacter,
	const HUD_PLAYER_STATE& Player)
{
	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pView->Get_ResolutionHeight();

	if (m_iSelectedRow >= static_cast<int32_t>(m_Rows.size()))
		m_iSelectedRow = m_Rows.empty() ? -1 : 0;
	m_iHoveredRow = -1;
	m_bMountEnabled = false;
	m_bSelectedIsMounted = false;
	m_iMountedVehicleId = Player.iVehicleId;

	for (int32_t iRow = 0; iRow < ROW_COUNT; ++iRow)
	{
		const bool_t bHasRow = iRow < static_cast<int32_t>(m_Rows.size());
		for (const char* pSuffix : { "Bg", "Over", "Selected", "Equip", "Icon", "Slot", "Star" })
			m_pView->Set_SlotVisible(Row_Slot(iRow, pSuffix), bHasRow);
		if (!bHasRow)
			continue;

		const VEHICLE_ROW& Row = m_Rows[static_cast<size_t>(iRow)];
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		const bool_t bHovered = m_pView->Get_SlotRect(Row_Slot(iRow, "Bg"), fX, fY, fWidth, fHeight) &&
			Router.Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight);
		if (bHovered)
		{
			m_iHoveredRow = iRow;
			if (Router.Is_Clicked(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight) && iRow != m_iSelectedRow)
			{
				CMainApp::Play_UIButtonClickSound();
				m_iSelectedRow = iRow;
				/* The H key mounts the window's current choice too. */
				CPlayerController::Set_PreferredVehicleId(Row.iVehicleId);
			}
			/* A click on the icon itself also picks the vehicle up for a quick slot. */
			f32_t fIconX = 0.f, fIconY = 0.f, fIconWidth = 0.f, fIconHeight = 0.f;
			if (m_pView->Get_SlotRect(Row_Slot(iRow, "Slot"), fIconX, fIconY, fIconWidth, fIconHeight) &&
				Router.Is_Clicked(fIconX, fIconY, fIconWidth, fIconHeight, fRefWidth, fRefHeight))
			{
				m_bIconPicked = true;
				m_iPickedVehicleId = Row.iVehicleId;
			}
		}
		const bool_t bSelected = iRow == m_iSelectedRow;
		const bool_t bMounted = Row.iVehicleId == Player.iVehicleId;
		m_pView->Set_SlotVisible(Row_Slot(iRow, "Over"), bHovered && !bSelected);
		m_pView->Set_SlotVisible(Row_Slot(iRow, "Selected"), bSelected);
		m_pView->Set_SlotVisible(Row_Slot(iRow, "Equip"), bMounted);
		if (!Row.strIconAsset.empty())
			m_pView->Set_SlotTexture(Row_Slot(iRow, "Icon"), Row.strIconAsset);

		if (bSelected)
		{
			m_bSelectedIsMounted = bMounted;
			const VEHICLE_ACTOR_ENTRY* pVehicle = CActorCatalog::Find_Vehicle(Row.iVehicleId);
			m_bMountEnabled = bMounted || (nullptr != pVehicle && nullptr != pLocalCharacter &&
				nullptr != pVehicle->Find_Rider(pLocalCharacter->Get_CharacterClass()));
		}
	}
}

void Client::CVehicleWindowView::Update_Buttons(const HUD_PLAYER_STATE& Player)
{
	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fRefWidth = m_pView->Get_ResolutionWidth();
	const f32_t fRefHeight = m_pView->Get_ResolutionHeight();
	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;

	const bool_t bCanRequest = m_bMountEnabled && Player.isValid && !Player.isPreview;
	if (m_pView->Get_SlotRect("VH_MountBtn", fX, fY, fWidth, fHeight))
	{
		const bool_t bHovered = Router.Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight);
		m_pView->Set_SlotTexture("VH_MountBtn",
			!bCanRequest ? ART_BTN_DISABLED : (bHovered ? ART_BTN_OVER : ART_BTN_NORMAL));
		if (bHovered && Router.Is_Clicked(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight) && bCanRequest &&
			m_iSelectedRow >= 0 && m_iSelectedRow < static_cast<int32_t>(m_Rows.size()))
		{
			CMainApp::Play_UIButtonClickSound();
			m_pView->Set_SlotTexture("VH_MountBtn", ART_BTN_DOWN);
			m_bRidingRequested = true;
			m_iRequestedVehicleId = m_bSelectedIsMounted ?
				0u : m_Rows[static_cast<size_t>(m_iSelectedRow)].iVehicleId;
		}
	}
	if (m_pView->Get_SlotRect("VH_CloseBtn", fX, fY, fWidth, fHeight))
	{
		const bool_t bHovered = Router.Is_Hovered(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight);
		m_pView->Set_SlotTexture("VH_CloseBtn", bHovered ? ART_BTN_OVER : ART_BTN_NORMAL);
		if (bHovered && Router.Is_Clicked(fX, fY, fWidth, fHeight, fRefWidth, fRefHeight))
		{
			CMainApp::Play_UIButtonClickSound();
			Close();
		}
	}
}

bool_t Client::CVehicleWindowView::Take_RidingRequest(uint32_t& outVehicleId)
{
	if (!m_bRidingRequested)
		return false;
	m_bRidingRequested = false;
	outVehicleId = m_iRequestedVehicleId;
	return true;
}

bool_t Client::CVehicleWindowView::Take_IconPick(uint32_t& outVehicleId, string& outIconPath)
{
	if (!m_bIconPicked)
		return false;
	m_bIconPicked = false;
	outVehicleId = m_iPickedVehicleId;
	const string* pIcon = Find_IconAsset(m_iPickedVehicleId);
	outIconPath = nullptr != pIcon ? *pIcon : string();
	return true;
}

const string* Client::CVehicleWindowView::Find_IconAsset(const uint32_t iVehicleId) const
{
	for (const VEHICLE_ROW& Row : m_Rows)
	{
		if (Row.iVehicleId == iVehicleId)
			return &Row.strIconAsset;
	}
	return nullptr;
}

const std::vector<VEHICLE_SKILL_UI>* Client::CVehicleWindowView::Find_Skills(const uint32_t iVehicleId) const
{
	for (const VEHICLE_ROW& Row : m_Rows)
	{
		if (Row.iVehicleId == iVehicleId)
			return &Row.Skills;
	}
	return nullptr;
}

void Client::CVehicleWindowView::Render_Text()
{
	if (!m_bOpen || nullptr == m_pView)
		return;
	f32_t fOriginX = 0.f, fOriginY = 0.f;
	if (!Get_WindowOrigin(fOriginX, fOriginY))
		return;

	const float2_t vTopLeft(0.f, 0.f);
	const float2_t vTopCenter(0.5f, 0.f);
	const float2_t vCenter(0.5f, 0.5f);

	Refit_Descriptions();
	Draw_Label(FONT_YOON, m_strTitle, WINDOW_WIDTH * 0.5f, TITLE_Y, TITLE_PX, COLOR_TITLE, vTopCenter);

	for (size_t i = 0; i < m_Rows.size(); ++i)
	{
		const VEHICLE_ROW& Row = m_Rows[i];
		const f32_t fRowY = ROW_Y0 + ROW_PITCH * static_cast<f32_t>(i);
		const f32_t fNameWidth = Draw_Label(FONT_YG760, Row.strName,
			ROW_X + ROW_TEXT_X, fRowY + ROW_NAME_Y, ROW_FONT_PX, COLOR_NAME, vTopLeft);
		if (0u != Row.iVehicleId && Row.iVehicleId == m_iMountedVehicleId)
		{
			Draw_Label(FONT_YG760, m_strMountedSuffix, ROW_X + ROW_TEXT_X + fNameWidth,
				fRowY + ROW_NAME_Y, ROW_FONT_PX, COLOR_MOUNTED, vTopLeft);
		}
		Draw_Label(FONT_YG760, Row.strDescriptionFit, ROW_X + ROW_TEXT_X, fRowY + ROW_DESC_Y,
			ROW_FONT_PX, COLOR_DESC, vTopLeft);
	}

	Draw_Label(FONT_YG760, m_strHint, HINT_X, HINT_Y, ROW_FONT_PX, COLOR_DESC, vTopLeft);
	Draw_Label(FONT_YOON, m_bSelectedIsMounted ? m_strDismount : m_strMount,
		MOUNT_BUTTON_X + BUTTON_W * 0.5f, BUTTON_Y + BUTTON_H * 0.5f, BUTTON_PX,
		m_bMountEnabled ? Colors::White : COLOR_DISABLED, vCenter);
	Draw_Label(FONT_YOON, m_strClose,
		CLOSE_BUTTON_X + BUTTON_W * 0.5f, BUTTON_Y + BUTTON_H * 0.5f, BUTTON_PX,
		Colors::White, vCenter);
}

void Client::CVehicleWindowView::Hide()
{
	for (const string& strId : m_SlotIds)
		m_pView->Set_SlotVisible(strId, false);
}

bool_t Client::CVehicleWindowView::Get_WindowOrigin(f32_t& fX, f32_t& fY) const
{
	f32_t fWidth = 0.f, fHeight = 0.f;
	return m_pView->Get_SlotRect("VH_WinBg", fX, fY, fWidth, fHeight);
}

f32_t Client::CVehicleWindowView::Ref_X(const f32_t fRetailX) const
{
	f32_t fX = 0.f, fY = 0.f;
	(void)Get_WindowOrigin(fX, fY);
	return fX + fRetailX * m_fRetailScale;
}

f32_t Client::CVehicleWindowView::Ref_Y(const f32_t fRetailY) const
{
	f32_t fX = 0.f, fY = 0.f;
	(void)Get_WindowOrigin(fX, fY);
	return fY + fRetailY * m_fRetailScale;
}

f32_t Client::CVehicleWindowView::Draw_Label(const wstring_t& strFont, const wstring& strText,
	const f32_t fRetailX, const f32_t fRetailY, const f32_t fRetailPx, const fvector_t vColor,
	const float2_t& vOrigin)
{
	if (strText.empty())
		return 0.f;
	const float2_t vViewport = CGameInstance::Get().Get_ViewportSize();
	if (vViewport.x <= 0.f || vViewport.y <= 0.f)
		return 0.f;
	const f32_t fScaleX = vViewport.x / m_pView->Get_ResolutionWidth();
	const f32_t fScaleY = vViewport.y / m_pView->Get_ResolutionHeight();
	const f32_t fUiScale = (std::min)(fScaleX, fScaleY);
	/* Baked size nearest the on-screen line height, drawn 1:1 when it matches (no blur) --
	same rule as CCharacterInfoWindowView::Draw_Label, whole-pixel top-left anchoring included. */
	f32_t fScale = 1.f;
	const wstring_t strUseFont = UILabelFont::Resolve(strFont,
		fRetailPx * m_fRetailScale * fUiScale * TEXT_BOOST, fScale);
	const float2_t vMeasured = CGameInstance::Get().Measure_Text(strUseFont, strText.c_str());
	const float2_t vPosition(
		std::round(Ref_X(fRetailX) * fScaleX - vMeasured.x * fScale * vOrigin.x),
		std::round(Ref_Y(fRetailY) * fScaleY - vMeasured.y * fScale * vOrigin.y));
	const float2_t vTopLeft(0.f, 0.f);
	CGameInstance::Get().Draw_Text(strUseFont, strText.c_str(),
		float2_t(vPosition.x + 1.f, vPosition.y + 1.f),
		XMVectorSet(0.f, 0.f, 0.f, 0.75f), 0.f, vTopLeft, fScale);
	CGameInstance::Get().Draw_Text(strUseFont, strText.c_str(), vPosition, vColor, 0.f, vTopLeft, fScale);
	/* Screen px -> retail px so the caller can place a follow-up label right after this one. */
	return vMeasured.x * fScale / (fScaleX * m_fRetailScale);
}

wstring Client::CVehicleWindowView::Fit_Label(const wstring_t& strFont, const wstring& strText,
	const f32_t fRetailPx, const f32_t fMaxRetailWidth) const
{
	const float2_t vViewport = CGameInstance::Get().Get_ViewportSize();
	if (strText.empty() || vViewport.x <= 0.f || vViewport.y <= 0.f)
		return strText;
	const f32_t fScaleX = vViewport.x / m_pView->Get_ResolutionWidth();
	const f32_t fScaleY = vViewport.y / m_pView->Get_ResolutionHeight();
	const f32_t fUiScale = (std::min)(fScaleX, fScaleY);
	f32_t fScale = 1.f;
	const wstring_t strUseFont = UILabelFont::Resolve(strFont,
		fRetailPx * m_fRetailScale * fUiScale * TEXT_BOOST, fScale);
	/* Same measure Draw_Label draws with, brought back to retail px. */
	const auto RetailWidth = [&](const wstring& str) -> f32_t
	{
		return CGameInstance::Get().Measure_Text(strUseFont, str.c_str()).x * fScale /
			(fScaleX * m_fRetailScale);
	};
	if (RetailWidth(strText) <= fMaxRetailWidth)
		return strText;
	const wstring strEllipsis = L"...";
	wstring strFit = strText;
	while (!strFit.empty())
	{
		strFit.pop_back();
		if (RetailWidth(strFit + strEllipsis) <= fMaxRetailWidth)
			break;
	}
	return strFit + strEllipsis;
}

void Client::CVehicleWindowView::Refit_Descriptions()
{
	/* Fit_Label uses both viewport axes to select the font and measure the text. */
	const float2_t viewport = CGameInstance::Get().Get_ViewportSize();
	if (viewport.x <= 0.f || viewport.y <= 0.f ||
		(viewport.x == m_vFitViewport.x && viewport.y == m_vFitViewport.y))
		return;
	m_vFitViewport = viewport;
	const f32_t fMaxWidth = ROW_W - ROW_TEXT_X - ROW_TEXT_RIGHT_PAD;
	for (VEHICLE_ROW& Row : m_Rows)
		Row.strDescriptionFit = Fit_Label(FONT_YG760, Row.strDescription, ROW_FONT_PX, fMaxWidth);
}

bool_t Client::CVehicleWindowView::Get_ScreenRect(
	f32_t& fX, f32_t& fY, f32_t& fWidth, f32_t& fHeight) const
{
	if (!Is_Open() || nullptr == m_pView)
		return false;
	f32_t fRefX = 0.f, fRefY = 0.f, fRefWidth = 0.f, fRefHeight = 0.f;
	if (!m_pView->Get_SlotRect("VH_WinBg", fRefX, fRefY, fRefWidth, fRefHeight))
		return false;
	const float2_t vViewport = CGameInstance::Get().Get_ViewportSize();
	const f32_t fScaleX = vViewport.x / m_pView->Get_ResolutionWidth();
	const f32_t fScaleY = vViewport.y / m_pView->Get_ResolutionHeight();
	fX = fRefX * fScaleX;
	fY = fRefY * fScaleY;
	fWidth = fRefWidth * fScaleX;
	fHeight = fRefHeight * fScaleY;
	return true;
}
