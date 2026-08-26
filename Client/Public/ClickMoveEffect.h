#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
NS_END

NS_BEGIN(Client)

/* Client-only cosmetic right-click move marker. Not a combat Effect Catalog
   asset -- it carries no skill/boss semantics, so it is a small standalone
   ground quad pair driven directly by CPlayerController, the same pattern
   CSkillGroundTargetPreview already uses for its own non-combat ground marker.

   Real source: Lost Ark's own click-move burst (ark.ui.cursorEffect.
   CursorEffectFrame / CursorEffect_Default_0, package cursoreffect.gfx) is a
   12-frame, 40fps Scaleform animation: a ring quad (depth 3) growing 1.0 ->
   0.6 -> 0.7 -> 0.8 -> 0.9 -> 1.0 scale then fading, plus an ADD-blended glow
   accent quad (depth 5) growing 0.2 -> 1.0 in lockstep then fading on its own
   (brighter) alpha curve. The real bitmap fill (cursoreffect_i5.tga, a crunch
   -compressed external GFx image) could not be pixel-decoded -- Lost Ark's
   crunch bitstream variant defeated both a self-built crn_decomp.h decoder
   and the independent texture2ddecoder package, confirmed against two
   unrelated assets this session. Only the real motion/timing survives; the
   ring/glow textures here are placeholder grayscale coverage masks. */
class CClickMoveEffect final : public CGameObject
{
public:
	static constexpr const wchar_t* PROTOTYPE_TAG =
		L"Prototype_GameObject_ClickMoveEffect";
	static constexpr const wchar_t* RING_SHADER_TAG =
		L"Prototype_Component_Shader_SkillGroundTargetPreview";
	static constexpr const wchar_t* GLOW_SHADER_TAG =
		L"Prototype_Component_Shader_ClickMoveGlow";

private:
	CClickMoveEffect(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	CClickMoveEffect(const CClickMoveEffect& prototype);

public:
	virtual ~CClickMoveEffect();
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

	bool_t Initialize_Textures();
	void Play(const float3_t& worldPosition);

private:
	HRESULT Render_Quad(
		const shared_ptr<Engine::CShader>& shader,
		const shared_ptr<Engine::CTexture>& texture,
		const float3_t& position,
		f32_t diameter,
		const float4_t& tint);

private:
	shared_ptr<Engine::CShader> m_pRingShader;
	shared_ptr<Engine::CShader> m_pGlowShader;
	shared_ptr<Engine::CVIBuffer_Rect> m_pRect;
	shared_ptr<Engine::CTexture> m_pRingTexture;
	shared_ptr<Engine::CTexture> m_pGlowTexture;

	float3_t m_WorldPosition{};
	f32_t m_fElapsedSeconds = 0.f;
	bool_t m_isActive = false;

public:
	static unique_ptr<CClickMoveEffect> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
