#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <memory>
#include <string>
#include <vector>

NS_BEGIN(Client)

class CUILayoutRuntime;

/* One contribution row of the award page: the title the contribution earned
("Fierce Bloodsport" / "Steadfast Fighter" -- EFTable_MvpTitle joined to
EFTable_GameMsg sys.mvp.title_*), the contribution's own name (sys.mvp.desc_*)
and the formatted amount (EFTable_Mvp.CalcType 0 = absolute, 1 = percent). */
struct MVP_RESULT_STAT
{
	wstring_t	strTitle;
	wstring_t	strDescription;
	wstring_t	strValue;
	bool_t		bHighlight = false;
};

/* One character shown on the page. The MVP owns the wide left area, up to three
party members own the narrow right columns. Medals are EFTable_MvpMedalDescription
IconIndex values, 1..17. */
struct MVP_RESULT_ENTRY
{
	wstring_t				strCharacterName;
	/* No server-name field: MvpResultFrame and MvpResultOtherStatItem both declare a
	   serverNameTF and neither ever assigns it, so retail leaves it blank and its
	   authored box overlaps the guild name it sits on. */
	wstring_t				strGuildName;
	vector<MVP_RESULT_STAT>	Stats;
	vector<int32_t>			Medals;
};

struct MVP_RESULT_DATA
{
	wstring_t				strContentName;
	MVP_RESULT_ENTRY		Mvp;
	vector<MVP_RESULT_ENTRY> Party;
};

/* Raid-clear MVP award page -- the screen that follows the clear mark and hands
out the contribution titles.

Source: EFUI_MVP's mvp.gfx, DefineSprite 275 "MvpResultFrame", driven by
ark.ui.mvp.MvpResultFrame. Image layers are authored in
Data/UI/MVP/MvpResult_Layout.json; this view owns the intro timeline and every
text run, because the retail document creates its labels at runtime through
MvpResultFrame.mvpData rather than authoring them as symbols.

Coordinate note: MainTimeline places the document with widgetX 980 / widgetY 641
and pivotType "center", so its frame-1 bounding box (the 2600x1080 dim, local
x -20..2580, centre 1280) lands on 980. Frame-local x therefore maps to screen
x - 300 on the movie's own 1920x1080 stage, then a uniform 2/3 onto this
project's 1280x720 authoring canvas. Every constant below is that conversion
applied to a measured local coordinate.

This view is presentation only. It never decides who the MVP is or what a
contribution is worth; it draws whatever MVP_RESULT_DATA it is handed. */
class CMvpResultView final
{
public:
	CMvpResultView(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iGameObjectLevelIndex);
	~CMvpResultView();

public:
	/* Restarts the intro timeline from frame 0. */
	void Show(const MVP_RESULT_DATA& Data);
	void Hide();
	bool_t Is_Visible() const { return m_bVisible; }

	void Update(f32_t fTimeDelta);
	/* Text only -- the image layers are CUI_Sprite GameObjects the layout runtime
	already submitted, so they draw without this call. */
	void Render();

private:
	void Apply_Timeline();
	void Draw_Label(const wchar_t* pFontTag, const wstring_t& strText,
		f32_t fCenterX, f32_t fCenterY, f32_t fFontPx,
		const float2_t& vOrigin, fvector_t vColor) const;
	void Render_MvpSide() const;
	void Render_PartyColumn(size_t iColumn) const;
	/* szOwner keeps each list's runtime slot ids distinct ("Mvp", "Party0"...). */
	/* bStampIn plays RollingRepositionList's item tween. Only the MVP's own list has
	   it: mvpBadgeList sets enableTween true with presetIndex 5, while each party
	   column's badgeList sets enableTween false, so those badges simply appear. */
	void Render_Medals(const vector<int32_t>& Medals, const char* szOwner,
		f32_t fCenterLocalX, f32_t fCenterLocalY, f32_t fPitchLocal,
		f32_t fIconLocal, size_t iMaxShown, bool_t bStampIn) const;

private:
	/* A slot position captured once, so per-frame motion can be absolute. */
	struct AUTHORED_SLOT
	{
		string	strId;
		f32_t	fX = 0.f;
		f32_t	fY = 0.f;
	};

	unique_ptr<CUILayoutRuntime>	m_pView;
	/* One render-target stand-in per column. */
	AUTHORED_SLOT				m_PartyAuthored[3];
	/* Medal slots are created on demand and then kept, so Show() has to hide
	   them again or a replay would skip their delay. */
	mutable vector<string>		m_MedalSlotIds;
	MVP_RESULT_DATA					m_Data;
	bool_t							m_bVisible = false;
	/* Seconds since Show(); the retail intro is 135 frames at the movie's 40fps. */
	f32_t							m_fElapsedSeconds = 0.f;
	/* One-shot burst: rewound once, when the intro reaches its entry frame. */
	bool_t						m_bParticleBoomStarted = false;
	bool_t						m_bSuccessBurstStarted = false;
	bool_t						m_bBadgeEffectStarted = false;
	/* Cached glyph height of a fixed metric string per font, so a label's scale
	does not change with the characters it happens to contain. */
	f32_t							m_fYoonMetricHeight = 0.f;
	f32_t							m_fYG760MetricHeight = 0.f;
};

NS_END
