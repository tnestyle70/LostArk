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
screens being migrated off that ImGui interim rendering. Only the fields the first migrated
screen (the raid entry preview) actually uses are read -- id, rect, layers[0] (path/tint/
additive/flipX). shine/animation.frames/keyframeAnimationPath stay unread until a screen that
uses them gets migrated.

Each slot becomes a real CUI_Sprite GameObject added to the caller's layer, instead of an
ImGui AddImage call -- it renders through the normal CRenderer/RENDERGROUP::UI pipeline. */
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
	void Set_SlotVisible(const string& strId, bool_t bVisible);
	/* Runtime tint override (0..1 RGBA) -- e.g. a dim backdrop's alpha, independent of the
	slot's authored layer tint. */
	void Set_SlotTint(const string& strId, const float4_t& vTint);
	/* Alpha-only override (0..1, RGB stays white) -- for a fade-in/out reveal timeline
	   (RaidClear's celebration layers) that isn't a hard on/off like Set_SlotVisible. */
	void Set_SlotAlpha(const string& strId, f32_t fAlpha);
	/* Resources-relative path -- loads (or reuses a cached) SRV directly, bypassing the
	Prototype/Component texture lookup this slot's own authored layer used, for content that
	changes after this view is built (a button's hover texture). Empty path reverts to the
	slot's authored texture. */
	void Set_SlotTexture(const string& strId, const string& strAssetPath);
	/* Overrides a slot's authored top-left position at runtime (size unchanged) -- for a menu
	that has to appear wherever the player clicked, keyed off its own authored relative offset
	from some anchor slot, rather than always drawing at its authored position (the Party
	context menu's own reason). Get_SlotRect reflects the override afterward. No-op if the slot
	doesn't exist. */
	void Set_SlotPosition(const string& strId, f32_t fX, f32_t fY);
	/* Drives "animation.frames" flipbook slots (a non-empty frame list plays instead of
	   Layers, same as CHUDRuntimeView's own AnimationFrames) -- swaps each such slot's texture
	   via Set_SlotTexture as its own clock crosses a frame boundary. No-op for every other
	   slot. Callers add this to their owning Level's Update(fTimeDelta). */
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

	f32_t Get_ResolutionWidth() const { return m_fResolutionWidth; }
	f32_t Get_ResolutionHeight() const { return m_fResolutionHeight; }

private:
	struct RUNTIME_SLOT
	{
		string				strId;
		f32_t				fX = 0.f, fY = 0.f, fSizeX = 0.f, fSizeY = 0.f;
		shared_ptr<CUI_Sprite>	pSprite;

		vector<string>		AnimationFramePaths;
		f32_t				fAnimationFPS = 10.f;
		bool_t				bAnimationLoop = true;
		f32_t				fAnimationElapsed = 0.f;
		int32_t				iAnimationFrame = -1;
	};

private:
	HRESULT Load();
	/* Registers strPath as a Texture prototype under LEVEL::STATIC (keyed by that same
	Resources-relative path) the first time this factory sees it, generalizing
	CMainApp::Ready_Prototype_For_LoadingChrome's one-document-only version. No-op (not an
	error) if already registered -- CGameInstance::Add_Prototype itself fails safely on a
	duplicate tag. */
	void Ensure_TexturePrototype(const wstring_t& strPath);

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
	-- these two are applied only when actually positioning a CUI_Sprite (Load/Set_SlotPosition). */
	f32_t							m_fScaleX = 1.f;
	f32_t							m_fScaleY = 1.f;

	vector<RUNTIME_SLOT>			m_Slots;
	vector<wstring_t>				m_RegisteredTexturePrototypes;
	/* For Set_SlotTexture's runtime swap path only -- the authored-layer path goes through the
	Prototype/CUI_Sprite path above instead. Shared utility (CUITextureCache) so this isn't a
	third copy of CHUDRuntimeView's own duplicate of the same DDS/WIC-load logic. */
	unique_ptr<CUITextureCache>	m_pTextureCache;
};

NS_END
