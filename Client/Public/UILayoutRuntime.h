#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <map>
#include <string>
#include <vector>

NS_BEGIN(Client)
class CUI_Sprite;
class CUITextureCache;

/* Real CUIObject-based runtime consumer of a lostark.ui-layout JSON document (the same
schema CHUDRuntimeView already parses for its own ImGui foreground-drawlist path), for
screens migrated off that ImGui interim rendering. Reads id, rect, rotation, ownerClass,
every layers[] entry (path/tint/additive/flipX), "animation.frames" flipbooks, and
"keyframeAnimationPath" documents (the extracted Scaleform identity-HUD assets -- multi-layer
per-frame asset/position/scale/rotation/alpha tracks with labels). shine/stages stay unread --
no migrated document uses them.

Each slot layer becomes a real CUI_Sprite GameObject added to the caller's layer, instead of
an ImGui AddImage call -- it renders through the normal CRenderer/RENDERGROUP::UI pipeline in
document order (slot by slot, layer by layer). A keyframe slot gets one sprite per document
layer, driven per frame by Update(). */
class CUILayoutRuntime final
{
public:
	CUILayoutRuntime(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iGameObjectLevelIndex, const wstring_t& strLayerTag,
		const wstring_t& strDocumentPath);
	~CUILayoutRuntime();

public:
	/* Reference-resolution rect of the authored slot with this JSON "id", or false if no such
	slot exists (unknown id, or its texture failed to load and the slot was skipped). */
	bool_t Get_SlotRect(const string& strId, f32_t& fX, f32_t& fY, f32_t& fWidth, f32_t& fHeight) const;
	/* Real show/hide (a hidden sprite is skipped entirely, no transparent-quad draw call) --
	tint/alpha/texture state is untouched, so showing again restores exactly what was set
	before. For a keyframe slot this gates whether Update() drives its layer sprites at all. */
	void Set_SlotVisible(const string& strId, bool_t bVisible);
	/* Runtime tint override (0..1 RGBA) on every authored static layer sprite. */
	void Set_SlotTint(const string& strId, const float4_t& vTint);
	/* Multiplies every static and keyframe layer's own tint without replacing authored RGB or
	key alpha. White restores the unmodified appearance. */
	void Set_SlotTintMultiplier(const string& strId, const float4_t& vTintMultiplier);
	/* Alpha-only override (0..1, RGB stays white) -- for a fade-in/out reveal timeline
	   (RaidClear's celebration layers) that isn't a hard on/off like Set_SlotVisible. */
	void Set_SlotAlpha(const string& strId, f32_t fAlpha);
	/* Resources-relative path -- loads (or reuses a cached) SRV directly, bypassing the
	Prototype/Component texture lookup this slot's own authored layer used, for content that
	changes after this view is built (a button's hover texture). Applies to the slot's BASE
	(first) layer only. Empty path reverts to the slot's authored texture. */
	void Set_SlotTexture(const string& strId, const string& strAssetPath);
	/* Overrides a slot's authored top-left position at runtime (size unchanged) -- for a menu
	that has to appear wherever the player clicked, keyed off its own authored relative offset
	from some anchor slot, rather than always drawing at its authored position (the Party
	context menu's own reason). Get_SlotRect reflects the override afterward. No-op if the slot
	doesn't exist. */
	void Set_SlotPosition(const string& strId, f32_t fX, f32_t fY);
	/* Overrides both a slot's authored top-left position AND size at runtime in one call --
	for an effect that grows/shrinks around a moving center point (a hit-flash glow's radius)
	instead of only ever repositioning at a fixed authored size. Get_SlotRect reflects the
	override afterward. No-op if the slot doesn't exist. */
	void Set_SlotRect(const string& strId, f32_t fX, f32_t fY, f32_t fWidth, f32_t fHeight);
	/* 1.f (default) draws the whole slot; a value in [0,1) clips it to that fraction of its own
	authored width (post-FlipX), for a gauge/health-bar drain that reveals the bar art's own left
	portion at native scale instead of a stretched/squished resize (CUI_Sprite::Set_FillRatio). */
	void Set_SlotFillRatio(const string& strId, f32_t fFillRatio);
	/* 1.f (default) draws the whole slot; a value in [0,1) keeps only the pie sector spanning
	that fraction of a full turn clockwise from 12 o'clock -- the skill-cooldown sweep overlay
	(CUI_Sprite::Set_ArcRatio). */
	void Set_SlotArcRatio(const string& strId, f32_t fArcRatio);
	/* Texture window sampled by the slot's base sprite (CUI_Sprite::Set_UVWindow) -- the
	minimap's map slot scrolls/zooms across its area image this way. */
	void Set_SlotUVWindow(const string& strId, f32_t fOffsetU, f32_t fOffsetV,
		f32_t fScaleU, f32_t fScaleV);
	/* Overrides a slot's authored "rotation" (screen-space clockwise degrees about its own rect
	center) at runtime -- for continuous data-driven rotation (DimensionMaster's clock hands and
	gear ornaments). Applies to every layer sprite of the slot. */
	void Set_SlotRotation(const string& strId, f32_t fDegrees);
	/* Show/hide every class-owned slot in one pass: a slot whose authored ownerClass is
	non-empty becomes visible only when it equals strOwnerClass. ownerClass-less slots are
	untouched -- the same per-frame filter CHUDRuntimeView::Render(strOwnerClass, ...) applied
	implicitly. Callers layer their own dynamic Set_SlotVisible overrides after this. */
	void Set_ActiveOwnerClass(const string& strOwnerClass);
	/* Every slot at once -- the whole-screen hide for a view whose owning system stops running
	(level gate fails), since these sprites keep their last state instead of simply not being
	drawn the way an ImGui pass did. */
	void Set_AllSlotsVisible(bool_t bVisible);
	/* Creates a slot (with one sprite showing strTexturePath at the given reference-resolution
	rect) at runtime if no slot with this id exists -- the Lobby's atomic fallback path, which
	must still show all four command buttons when an old/corrupt external Data checkout ships a
	partial document. No-op if the id already exists (the authored slot wins). */
	void Ensure_RuntimeSlot(const string& strId, f32_t fX, f32_t fY,
		f32_t fWidth, f32_t fHeight, const string& strTexturePath);
	/* Jumps a keyframe slot's document to the frame labelled strLabel and plays forward until
	the next authored label (or the document's end), then holds on that last frame -- mirrors
	the original asset's own gotoAndPlay(label) trigger, same contract as CHUDRuntimeView's
	version. Calling again (any label) restarts the window. False if the slot has no keyframe
	document or the label is unknown. */
	bool_t Play_KeyframeAnimation(const string& strId, const string& strLabel);
	/* Drives "animation.frames" flipbook slots and keyframe-animation slots forward by
	fTimeDelta -- flipbooks swap their sprite's texture on frame-boundary crossings; keyframe
	slots re-evaluate every document layer's active key into that layer's own sprite
	(texture/position/scale/rotation/alpha/additive/flip). Callers add this to their owning
	system's per-frame update. */
	void Update(f32_t fTimeDelta);
	/* Restarts a slot's "animation.frames" flipbook from frame 0 right now -- for a one-shot
	effect (loop=false) a caller wants to fire on a real event (a gauge reaching 100%), since
	Update() otherwise only ever advances the clock it already has. No-op if the slot doesn't
	exist or has no animation frames. */
	void Restart_Animation(const string& strId);
	/* Pins a slot's "animation.frames" flipbook to an exact frame index right now, for a caller
	driving it from a real data value (a gauge percent) instead of letting it free-run off its
	own elapsed-time clock -- two flipbooks meant to read as "the same percent" would otherwise
	drift out of phase. iFrame is clamped to the slot's real frame count. No-op if the slot
	doesn't exist or has no animation frames. */
	void Set_Animation_Frame(const string& strId, int32_t iFrame);
	/* Swaps a slot's "animation.frames" flipbook list at runtime and rewinds it to frame 0 --
	the raid-entry popup switches the boss-portrait commander movie per tab this way. An empty
	Frames list clears the flipbook so the slot falls back to its static Set_SlotTexture art.
	fFps <= 0 keeps the slot's current fps. No-op if the slot doesn't exist. */
	void Set_SlotAnimation(const string& strId, const vector<string>& Frames,
		f32_t fFps, bool_t bLoop);

	f32_t Get_ResolutionWidth() const { return m_fResolutionWidth; }
	f32_t Get_ResolutionHeight() const { return m_fResolutionHeight; }

private:
	struct KEYFRAME_ANIM_KEY
	{
		int32_t	iFrame = 0;
		/* Empty = hidden from this frame onward (the source SWF depth was removed here). */
		string	strAsset;
		f32_t	fX = 0.f, fY = 0.f;
		f32_t	fScaleX = 1.f, fScaleY = 1.f;
		f32_t	fRotationDeg = 0.f;
		f32_t	fAlpha = 1.f;
		bool_t	bAdditive = false;
		bool_t	bFlipX = false;
	};

	struct KEYFRAME_ANIM_LAYER
	{
		vector<KEYFRAME_ANIM_KEY> Keys;
	};

	struct KEYFRAME_ANIM_DOCUMENT
	{
		bool_t				isLoaded = false;
		bool_t				bLoop = false;
		f32_t				fFrameRate = 40.f;
		int32_t				iFrameCount = 0;
		map<string, int32_t>			Labels;
		vector<KEYFRAME_ANIM_LAYER>		Layers;
	};

	struct RUNTIME_SLOT
	{
		string				strId;
		string				strOwnerClass;
		f32_t				fX = 0.f, fY = 0.f, fSizeX = 0.f, fSizeY = 0.f;
		bool_t				bVisible = true;
		shared_ptr<CUI_Sprite>	pSprite;
		/* layers[1..] of a multi-layer slot, same rect as pSprite, drawn on top of it in
		authored order (a skill slot's background + frame pair). */
		vector<shared_ptr<CUI_Sprite>>	ExtraLayerSprites;

		vector<string>		AnimationFramePaths;
		f32_t				fAnimationFPS = 10.f;
		bool_t				bAnimationLoop = true;
		f32_t				fAnimationElapsed = 0.f;
		int32_t				iAnimationFrame = -1;

		/* Keyframe-animation slot state: one sprite per document layer, driven by Update()
		once Play_KeyframeAnimation starts a window. fKeyframeElapsedSeconds < 0 = never
		played (draws nothing, matching CHUDRuntimeView). */
		string				strKeyframeAnimationPath;
		f32_t				fKeyframeAnimationScale = 1.f;
		int32_t				iKeyframeWindowStart = 0;
		int32_t				iKeyframeWindowEnd = 0;
		f64_t				fKeyframeElapsedSeconds = -1.0;
		vector<shared_ptr<CUI_Sprite>>	KeyframeSprites;
	};

private:
	HRESULT Load();
	/* Registers strPath as a Texture prototype under LEVEL::STATIC (keyed by that same
	Resources-relative path) the first time this factory sees it, generalizing
	CMainApp::Ready_Prototype_For_LoadingChrome's one-document-only version. No-op (not an
	error) if already registered -- CGameInstance::Add_Prototype itself fails safely on a
	duplicate tag. */
	void Ensure_TexturePrototype(const wstring_t& strPath);
	shared_ptr<CUI_Sprite> Create_Sprite(
		f32_t fRectX, f32_t fRectY, f32_t fRectWidth, f32_t fRectHeight,
		const string& strTexturePath);
	const KEYFRAME_ANIM_DOCUMENT* Get_Or_Load_KeyframeAnimation(const string& strPath);
	void Update_KeyframeSlot(RUNTIME_SLOT& Slot, f32_t fTimeDelta);

private:
	ComPtr<ID3D11Device>			m_pDevice;
	ComPtr<ID3D11DeviceContext>		m_pContext;
	uint32_t						m_iGameObjectLevelIndex = 0;
	wstring_t						m_strLayerTag;
	wstring_t						m_strDocumentPath;
	f32_t							m_fResolutionWidth = 1280.f;
	f32_t							m_fResolutionHeight = 720.f;
	/* Reference-resolution -> viewport-pixels scale, computed once at Load() (not re-applied on
	later resize, matching CMainApp's own LoadingLayout chrome). Get_SlotRect/RUNTIME_SLOT stay
	in raw reference-resolution units -- matching CHUDRuntimeView's own Get_SlotRect contract,
	which every existing caller (RenderItemAnnounceText, etc.) already assumes and scales itself
	-- these two are applied only when actually positioning a CUI_Sprite. */
	f32_t							m_fScaleX = 1.f;
	f32_t							m_fScaleY = 1.f;

	vector<RUNTIME_SLOT>			m_Slots;
	vector<wstring_t>				m_RegisteredTexturePrototypes;
	map<string, KEYFRAME_ANIM_DOCUMENT>	m_KeyframeAnimationCache;
	/* For Set_SlotTexture's runtime swap path and every keyframe/flipbook frame texture -- the
	authored-layer path goes through the Prototype/CUI_Sprite path above instead. Shared utility
	(CUITextureCache) so this isn't a third copy of CHUDRuntimeView's own duplicate of the same
	DDS/WIC-load logic. */
	unique_ptr<CUITextureCache>	m_pTextureCache;
};

NS_END
