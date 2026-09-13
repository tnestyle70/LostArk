#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <memory>
#include <string>
#include <vector>

NS_BEGIN(Client)

class CUILayoutRuntime;
class CCharacter;
class CCharacterPortraitRenderer;

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
/* The class emblem a card shows: the watermark behind the MVP and the small
icon above a party column's name. Both come from the same class_N label -- the
MVP page's classMc gotoAndStop()s that label -- but from different art: the big
one is MvpClassIcon_NN out of shareimagev2, the small one is either a shape
mvp.gfx defines itself or an imported goldClassIcon.

The offset is the icon's own placement inside Shared_MvpClassBigSymbol; the
forty emblems differ in size (178x179 to 193x181 among the playable four) and
are not registered on a common origin. Empty when the class has no mapping. */
struct MVP_CLASS_EMBLEM
{
	/* The class_N label number. Part of the runtime slot id, because
	   Ensure_RuntimeSlot keeps the texture a slot was created with. */
	int32_t		iClassKey = 0;
	wstring_t	strBigAsset;
	wstring_t	strSmallAsset;
	f32_t		fOffsetX = 0.f;
	f32_t		fOffsetY = 0.f;
	f32_t		fWidth = 0.f;
	f32_t		fHeight = 0.f;
	bool_t Is_Valid() const { return !strBigAsset.empty() && fWidth > 0.f; }
};

struct MVP_RESULT_ENTRY
{
	wstring_t				strCharacterName;
	/* No server-name field: MvpResultFrame and MvpResultOtherStatItem both declare a
	   serverNameTF and neither ever assigns it, so retail leaves it blank and its
	   authored box overlaps the guild name it sits on. */
	wstring_t				strGuildName;
	MVP_CLASS_EMBLEM		Emblem;
	vector<MVP_RESULT_STAT>	Stats;
	vector<int32_t>			Medals;
};

/* What the authored page has room for: three contribution rows on a card and
three party columns beside the MVP. The layout owns these numbers; whatever
composes the page has to agree with them. */
inline constexpr size_t MVP_RESULT_MAX_STATS = 3u;
inline constexpr size_t MVP_RESULT_MAX_PARTY_COLUMNS = 3u;

/* One coloured span of the headline above the MVP. contentNameTF is a single
HTML field in retail and its pieces carry their own <FONT COLOR> in GameMsg --
the gate is #A9D0F5, most difficulties have a colour of their own, and anything
without one takes the field's white -- so the page carries runs, not a string. */
struct MVP_TEXT_RUN
{
	wstring_t	strText;
	float4_t	vColor = float4_t(1.f, 1.f, 1.f, 1.f);
};

struct MVP_RESULT_DATA
{
	/* the difficulty, the raid name and the gate, laid out as one centred line. */
	vector<MVP_TEXT_RUN>	ContentName;
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
	/* The character one panel shows. mvp.gfx leaves host render targets there
	   (MvpPlayerTexture and MvpSubPlayerTexture_0..2) rather than any art, so the
	   page draws each model into an off-screen target and hands the slot that
	   target's SRV. Slot 0 is the MVP panel, 1..3 the party columns left to
	   right. Passing nullptr leaves that panel empty. */
	void Set_StageCharacter(size_t iSlot, const shared_ptr<CCharacter>& pCharacter);
	/* Draws the staged characters into their own targets and hands each panel
	   slot the result. Must run in CMainApp's portrait phase -- after
	   Render_Begin and before the world/UI pass that samples them -- because the
	   target manager keeps a single saved binding and nesting a second pass
	   inside the renderer's own frame overwrites it. */
	void Render_Portraits();
	/* Text only -- the image layers are CUI_Sprite GameObjects the layout runtime
	already submitted, so they draw without this call. */
	void Render();

private:
	void Apply_Timeline();
	/* Holds the staged characters on the award page's celebration clip. */
	void Drive_StageCelebration();
	/* Resources-relative cue, played through CGameInstance::Play_Sound. */
	static void Play_Cue(const wchar_t* pRelativePath);
	/* True when the player asked to close: Esc, or a click on the exit button the
	page draws as "나가기[Esc]". */
	bool_t Poll_ExitRequest();
	/* bLatinDisplay routes a title-size Latin label (the word "MVP", a
	   percentage) to the re-rasterised display atlases instead of magnifying the
	   family one. Never pass it for text that can contain Korean. */
	void Draw_Label(const wchar_t* pFontTag, const wstring_t& strText,
		f32_t fCenterX, f32_t fCenterY, f32_t fFontPx,
		const float2_t& vOrigin, fvector_t vColor,
		bool_t bLatinDisplay = false) const;
	/* The headline's runs, laid out as one centred line. */
	void Draw_ContentName(f32_t fCenterX, f32_t fCenterY, f32_t fFontPx,
		f32_t fAlpha) const;
	/* One emblem, as a runtime image slot. szOwner keeps the slot ids distinct
	   the way the medal rows do. */
	void Render_ClassEmblem(const MVP_CLASS_EMBLEM& Emblem, const char* szOwner,
		const wstring_t& strAsset, f32_t fLeftLocalX, f32_t fTopLocalY,
		f32_t fWidthLocal, f32_t fHeightLocal, f32_t fAlpha) const;
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
	/* Same lifetime problem as the medals: created on demand, so Show() has
	   to hide them again or a replay would keep the previous card's emblem. */
	mutable vector<string>		m_EmblemSlotIds;
	MVP_RESULT_DATA					m_Data;
	bool_t							m_bVisible = false;
	/* Seconds since Show(); the retail intro is 135 frames at the movie's 40fps. */
	f32_t							m_fElapsedSeconds = 0.f;
	/* One-shot burst: rewound once, when the intro reaches its entry frame. */
	bool_t						m_bParticleBoomStarted = false;
	bool_t						m_bSuccessBurstStarted = false;
	bool_t						m_bBadgeEffectStarted = false;
	/* How many badge stamp cues have fired, so each medal gets one as it lands. */
	size_t						m_iBadgeCuesPlayed = 0u;
	/* Press edge for the Esc close, so a held key does not re-close a replay. */
	bool_t						m_bEscapeDownLastFrame = false;
	/* One per panel: the same off-screen character draw the character info window
	   and the avatar book already use, which owns its target and draws the parts
	   with their forward ScreenCutin passes. Built on first use, because a panel
	   with no character never needs one. */
	ComPtr<ID3D11Device>		m_pStageDevice;
	ComPtr<ID3D11DeviceContext>	m_pStageContext;
	unique_ptr<CCharacterPortraitRenderer>	m_pStagePortrait[4];
	/* Not owned: the level hands over whichever character the panel should show. */
	weak_ptr<CCharacter>		m_pStageCharacter[4];
};

NS_END
