#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <map>
#include <string>
#include <vector>

struct ImDrawList;
struct ImDrawCmd;
struct ImVec2;

NS_BEGIN(Client)

/* Runtime (Release-safe) presentation for the HUD Layout Tool's authored JSON. The tool
(_DEBUG only) edits Data/UI/HUD/HUD_Layout.json; this class is the first actual in-game
consumer of that data -- it loads the document once, then every frame draws whichever slots
match the local player's class and current charge stage, via ImGui's foreground draw list.
This intentionally mirrors CHUDLayoutTool's own per-slot draw logic (same source data, same
visual rules) instead of inventing a second layout format. */
class CHUDRuntimeView final
{
public:
	enum class DRAW_TARGET
	{
		/* Always on top of every ImGui window (the combat HUD's own use, independent of any
		specific window). */
		FOREGROUND,
		/* The currently-open ImGui window's draw list -- art lands behind that window's own
		widgets instead of over every window in the game. Call Render() between that window's
		Begin()/End() when using this. */
		CURRENT_WINDOW,
		/* Always behind every ImGui window and the 3D scene alike -- a level's own backdrop
		(the lobby's title background, ...), never covering any window's UI. */
		BACKGROUND,
	};

	/* strDocumentPath: Data-root-relative layout JSON to load ("UI/HUD/HUD_Layout.json" if
	omitted). CHUDLayoutTool authors any lostark.ui-layout document at any such path (Combat
	HUD, Screen UI, Loading Screen, Skill Window, Lobby, ...); a second CHUDRuntimeView
	instance pointed at a different path is how a second document gets a runtime consumer
	without a second copy of this parse/draw logic. */
	CHUDRuntimeView(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext,
		const wstring& strDocumentPath = L"UI/HUD/HUD_Layout.json",
		DRAW_TARGET eDrawTarget = DRAW_TARGET::FOREGROUND);
	~CHUDRuntimeView();

public:
	/* strOwnerClass: HUD document class name ("LanceMaster", "Gunslinger", ...). Slots with no
	owner (shared/common chrome) always draw; slots owned by a different class are skipped.
	iStage: 0 = base only, 1 = base + stage-1 layers, 2 = base + stage-1 + stage-2 layers
	(monotonic reveal -- a slot visible from stage N stays visible at every stage after N). */
	void Render(const string& strOwnerClass, int32_t iStage);

	/* Resources-relative path (e.g. "UI/ClassSelect/Common/Category.png") -> cached SRV, or
	nullptr if it can't be resolved/loaded. For callers that draw a handful of images outside
	this document's own slot layout (Level_CharacterSelect's category list, ...) instead of
	duplicating DDS/WIC loading, or standing up a second CHUDRuntimeView, for that. */
	ID3D11ShaderResourceView* Load_Texture(const string& strPath);

	/* Reference-resolution rect of the authored slot with this JSON "id" (e.g. "Skill_Q"), or
	false if no such slot exists. For callers that need to draw a dynamic overlay (a skill
	cooldown sweep, ...) aligned to a slot this view already owns the authored position of,
	instead of duplicating the JSON's numbers a second time. */
	bool_t Get_SlotRect(const string& strId, f32_t& fX, f32_t& fY, f32_t& fWidth, f32_t& fHeight) const;

	/* Jumps a slot's "keyframeAnimationPath" document (a lostark.identity-keyframe-animation
	document -- a Scaleform identity-HUD extraction baked into per-layer keyframes) to the frame
	labelled strLabel and plays forward in real time until the next authored label (or the
	document's end), then holds on that last frame -- mirrors the original asset's own
	stanceMc.gotoAndPlay(label) trigger. No-op if the slot has no keyframeAnimationPath, the
	document fails to load, or the label is unknown. */
	bool_t Play_KeyframeAnimation(const string& strSlotId, const string& strLabel);

	/* Overrides a slot's authored "rotation" (degrees, clockwise, about its own rect centre) at
	runtime -- for continuous data-driven rotation (DimensionMaster's minute hand tracks its real
	cyclic identity gauge value directly via MovieClip.rotation in the source, not a frame-based
	clip) that a fixed JSON value or the frame-indexed keyframe system can't express. No-op if the
	slot doesn't exist. */
	bool_t Set_SlotRotation(const string& strSlotId, f32_t fDegrees);
	bool_t Set_SlotVisible(const string& strSlotId, bool_t bVisible);

private:
	struct TEXTURE_LAYER
	{
		string	strPath;
		float	vTint[4] = { 1.f, 1.f, 1.f, 1.f };
		bool_t	bAdditive = false;
		bool_t	bFlipX = false;
	};

	struct KEYFRAME_ANIM_KEY
	{
		int32_t	iFrame = 0;
		/* Empty = hidden from this frame onward (the source SWF depth was removed here). */
		string	strAsset;
		f32_t	fX = 0.f, fY = 0.f;
		f32_t	fScaleX = 1.f, fScaleY = 1.f;
		f32_t	fRotationDeg = 0.f;
		f32_t	fAlpha = 1.f;
		/* SWF blendMode 8 (ADD) on this piece's own placement -- glow/burst fragments are
		authored expecting this; drawing them with normal alpha blend instead shows their baked
		black backing as an opaque box. */
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

	struct TEXTURE_SIZE
	{
		f32_t fWidth = 0.f, fHeight = 0.f;
	};

	struct HUD_SLOT
	{
		string					strId;
		string					strOwnerClass;
		f32_t					fX = 0.f, fY = 0.f, fSizeX = 0.f, fSizeY = 0.f;
		f32_t					fRotation = 0.f;
		int32_t					iBaseFromStage = 0;
		int32_t					iShineFromStage = 1;
		vector<TEXTURE_LAYER>	Layers;
		string					strShineTexture;
		bool_t					bShineAdditive = false;
		/* A non-empty list plays as a looping flipbook instead of drawing Layers -- CHUDLayoutTool
		already authors this same "animation.frames"/"animation.fps" pair for its own canvas
		preview, so a slot built for a login/lobby background frame sequence just needs a runtime
		consumer, not a new document field. */
		vector<string>			AnimationFrames;
		f32_t					fAnimationFPS = 10.f;
		/* true (default) repeats for as long as this view is on screen, e.g. a lobby background.
		false plays once from the view's first render and then stops drawing the slot entirely,
		e.g. a boot logo intro that should reveal whatever is layered beneath it once it ends. */
		bool_t					bAnimationLoop = true;
		/* Sentinel: unset until this slot's first Render() call, which stamps it with
		ImGui::GetTime() so playback starts from frame 0 the first time this slot is actually
		drawn -- using the raw (app-launch-relative) clock directly would let engine/asset init
		time before this view's first frame eat into or skip past a non-looping slot entirely. */
		f64_t					dAnimationStartSeconds = -1.0;

		/* Data-root-relative lostark.identity-keyframe-animation document (empty = this slot has
		none; draws its Layers/shine as usual). Set once at Load() time. */
		string					strKeyframeAnimationPath;
		f32_t					fKeyframeAnimationScale = 1.f;
		/* Play_KeyframeAnimation() playback window; negative dStartSeconds = nothing playing
		(falls back to the slot's own Layers). Holds on iWindowEnd-1 once reached, rather than
		looping, since a stance icon should settle on its arrival pose until the next trigger. */
		f64_t					dKeyframeAnimStartSeconds = -1.0;
		int32_t					iKeyframeAnimWindowStart = 0;
		int32_t					iKeyframeAnimWindowEnd = 0;

		/* Runtime on/off for slots that only make sense conditionally (LanceMaster's gauge glow,
		visible only while that segment is full) -- set via Set_SlotVisible(), defaults shown. */
		bool_t					bForceHidden = false;
	};

private:
	HRESULT Load();
	ID3D11ShaderResourceView* Get_Or_Load_Texture(const string& strPath);
	/* Native pixel size of a texture already resident in m_TextureCache (0,0 if not loaded/found).
	Keyframe layers size their quad off each piece's own bitmap instead of stretching to a shared
	slot rect, unlike the fixed-rect Layers/shine path above. */
	TEXTURE_SIZE Get_Texture_Size(const string& strPath);
	const KEYFRAME_ANIM_DOCUMENT* Get_Or_Load_KeyframeAnimation(const string& strPath);
	void Draw_Image_Quad(ImDrawList* pDrawList, ID3D11ShaderResourceView* pSRV,
		const ImVec2 Corners[4], uint32_t iTint, bool_t bAdditive, bool_t bFlipX);
	static void Enable_Additive_Blend(const ImDrawList* pParentList, const ImDrawCmd* pCmd);

private:
	wstring							m_strDocumentPath;
	DRAW_TARGET						m_eDrawTarget = DRAW_TARGET::FOREGROUND;
	ComPtr<ID3D11Device>			m_pDevice;
	ComPtr<ID3D11DeviceContext>		m_pContext;
	ComPtr<ID3D11BlendState>		m_pAdditiveBlendState;
	map<string, ComPtr<ID3D11ShaderResourceView>>	m_TextureCache;
	map<string, TEXTURE_SIZE>		m_TextureSizeCache;
	map<string, KEYFRAME_ANIM_DOCUMENT>	m_KeyframeAnimationCache;

	vector<HUD_SLOT>	m_Slots;
	f32_t				m_fResolutionWidth = 1280.f;
	f32_t				m_fResolutionHeight = 720.f;
};

NS_END
