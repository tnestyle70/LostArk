#include "ClickMoveEffect.h"

#include "GameInstance.h"
#include "RuntimeAssetRoot.h"
#include "Shader.h"
#include "Texture.h"
#include "VIBuffer_Rect.h"

#include <array>
#include <cmath>
#include <filesystem>

namespace
{
	/* Real timing captured from ark.ui.cursorEffect.CursorEffectFrame's
	   CursorEffect_Default_0 (12-frame sprite, cursoreffect.gfx, frameRate
	   40.0). Index 0 is the sprite's frame 2 (frame 1 only places an
	   always-alpha-0 core, never visible). Ring is depth 3 / straight alpha;
	   glow is depth 5 / ADD blend, both keyed off the same clock. */
	struct CLICK_MOVE_KEYFRAME final
	{
		f32_t fTimeSeconds;
		f32_t fRingScale;
		f32_t fRingAlpha;
		f32_t fGlowScale;
		f32_t fGlowAlpha;
	};

	constexpr f32_t SWF_FRAME_SECONDS = 1.f / 40.f;

	constexpr std::array<CLICK_MOVE_KEYFRAME, 10> g_Keyframes{ {
		{ 0.f * SWF_FRAME_SECONDS, 1.00f, 1.0000f, 0.20f, 1.0000f },
		{ 1.f * SWF_FRAME_SECONDS, 0.60f, 1.0000f, 0.36f, 1.0000f },
		{ 2.f * SWF_FRAME_SECONDS, 0.70f, 1.0000f, 0.52f, 1.0000f },
		{ 3.f * SWF_FRAME_SECONDS, 0.80f, 1.0000f, 0.68f, 1.0000f },
		{ 4.f * SWF_FRAME_SECONDS, 0.90f, 1.0000f, 0.84f, 1.0000f },
		{ 5.f * SWF_FRAME_SECONDS, 1.00f, 1.0000f, 1.00f, 1.0000f },
		{ 6.f * SWF_FRAME_SECONDS, 1.00f, 0.5625f, 1.00f, 0.7500f },
		{ 7.f * SWF_FRAME_SECONDS, 1.00f, 0.2500f, 1.00f, 0.5000f },
		{ 8.f * SWF_FRAME_SECONDS, 1.00f, 0.0625f, 1.00f, 0.2500f },
		{ 9.f * SWF_FRAME_SECONDS, 1.00f, 0.0000f, 1.00f, 0.0000f },
	} };

	/* Native shapeBounds ratio: ring (shapeId 3) 2080x2060 twips, glow
	   (shapeId 6) 760x740 twips -- glow's native size is 0.365x the ring's.
	   Ring base diameter matches this project's own ground-target "point"
	   marker scale (PlayerSkillTargeting.json targetPreview.diameter=6.0). */
	constexpr f32_t RING_BASE_DIAMETER = 6.f;
	constexpr f32_t GLOW_BASE_DIAMETER = RING_BASE_DIAMETER * 0.365f;
	constexpr f32_t TOTAL_DURATION_SECONDS =
		g_Keyframes.back().fTimeSeconds;

	/* Real color could not be recovered -- cursoreffect_i5.tga is a crunch-
	   compressed external GFx image and its bitstream did not decode with
	   any available tool this session. Placeholder tint only. */
	constexpr float4_t RING_TINT{ 0.55f, 0.95f, 0.55f, 1.f };
	constexpr float4_t GLOW_TINT{ 0.80f, 1.f, 0.80f, 1.f };

	void Sample_Keyframes(
		f32_t fElapsedSeconds,
		f32_t& outRingScale,
		f32_t& outRingAlpha,
		f32_t& outGlowScale,
		f32_t& outGlowAlpha)
	{
		if (fElapsedSeconds <= g_Keyframes.front().fTimeSeconds)
		{
			const auto& first = g_Keyframes.front();
			outRingScale = first.fRingScale;
			outRingAlpha = first.fRingAlpha;
			outGlowScale = first.fGlowScale;
			outGlowAlpha = first.fGlowAlpha;
			return;
		}
		for (size_t i = 1; i < g_Keyframes.size(); ++i)
		{
			const auto& prev = g_Keyframes[i - 1];
			const auto& next = g_Keyframes[i];
			if (fElapsedSeconds <= next.fTimeSeconds)
			{
				const f32_t span = next.fTimeSeconds - prev.fTimeSeconds;
				const f32_t t = span > 0.f ?
					(fElapsedSeconds - prev.fTimeSeconds) / span : 1.f;
				outRingScale = prev.fRingScale +
					(next.fRingScale - prev.fRingScale) * t;
				outRingAlpha = prev.fRingAlpha +
					(next.fRingAlpha - prev.fRingAlpha) * t;
				outGlowScale = prev.fGlowScale +
					(next.fGlowScale - prev.fGlowScale) * t;
				outGlowAlpha = prev.fGlowAlpha +
					(next.fGlowAlpha - prev.fGlowAlpha) * t;
				return;
			}
		}
		const auto& last = g_Keyframes.back();
		outRingScale = last.fRingScale;
		outRingAlpha = last.fRingAlpha;
		outGlowScale = last.fGlowScale;
		outGlowAlpha = last.fGlowAlpha;
	}
}

Client::CClickMoveEffect::CClickMoveEffect(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CGameObject(std::move(pDevice), std::move(pContext))
{
}

Client::CClickMoveEffect::CClickMoveEffect(
	const CClickMoveEffect& prototype)
	: CGameObject(prototype)
{
}

Client::CClickMoveEffect::~CClickMoveEffect() = default;

HRESULT Client::CClickMoveEffect::Initialize_Prototype()
{
	return S_OK;
}

HRESULT Client::CClickMoveEffect::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)) ||
		FAILED(__super::Add_Component(
			ETOUI(LEVEL::STATIC), RING_SHADER_TAG,
			TEXT("Com_RingShader"), m_pRingShader)) ||
		FAILED(__super::Add_Component(
			ETOUI(LEVEL::STATIC), GLOW_SHADER_TAG,
			TEXT("Com_GlowShader"), m_pGlowShader)) ||
		FAILED(__super::Add_Component(
			ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_VIBuffer_Rect"),
			TEXT("Com_VIBuffer"), m_pRect)))
	{
		return E_FAIL;
	}
	return S_OK;
}

bool_t Client::CClickMoveEffect::Initialize_Textures()
{
	if (nullptr != m_pRingTexture && nullptr != m_pGlowTexture)
		return true;

	const filesystem::path ringPath = CRuntimeAssetRoot::Resolve(
		"Effect/UI/ClickMoveEffect/click_move_ring.dds");
	const filesystem::path glowPath = CRuntimeAssetRoot::Resolve(
		"Effect/UI/ClickMoveEffect/click_move_glow.dds");
	if (ringPath.empty() || glowPath.empty())
		return false;

	shared_ptr<CTexture> stagedRing = CTexture::Create(
		m_pDevice, m_pContext, ringPath.c_str(), 1u);
	shared_ptr<CTexture> stagedGlow = CTexture::Create(
		m_pDevice, m_pContext, glowPath.c_str(), 1u);
	if (nullptr == stagedRing || nullptr == stagedGlow)
		return false;

	m_pRingTexture = std::move(stagedRing);
	m_pGlowTexture = std::move(stagedGlow);
	return true;
}

void Client::CClickMoveEffect::Play(const float3_t& worldPosition)
{
	if (nullptr == m_pRingTexture || nullptr == m_pGlowTexture)
		return;
	m_WorldPosition = worldPosition;
	m_fElapsedSeconds = 0.f;
	m_isActive = true;
}

void Client::CClickMoveEffect::Late_Update(f32_t fTimeDelta)
{
	if (!m_isActive)
		return;
	m_fElapsedSeconds += fTimeDelta;
	if (m_fElapsedSeconds > TOTAL_DURATION_SECONDS)
	{
		m_isActive = false;
		return;
	}
	CGameInstance::Get().Add_RenderObject(
		RENDERGROUP::BLEND,
		static_pointer_cast<CGameObject>(shared_from_this()));
}

HRESULT Client::CClickMoveEffect::Render()
{
	if (!m_isActive)
		return S_OK;

	f32_t ringScale = 0.f, ringAlpha = 0.f, glowScale = 0.f, glowAlpha = 0.f;
	Sample_Keyframes(m_fElapsedSeconds, ringScale, ringAlpha,
		glowScale, glowAlpha);

	const float4_t ringTint{
		RING_TINT.x, RING_TINT.y, RING_TINT.z, RING_TINT.w * ringAlpha };
	const float4_t glowTint{
		GLOW_TINT.x, GLOW_TINT.y, GLOW_TINT.z, GLOW_TINT.w * glowAlpha };

	if (FAILED(Render_Quad(m_pGlowShader, m_pGlowTexture, m_WorldPosition,
			GLOW_BASE_DIAMETER * glowScale, glowTint)) ||
		FAILED(Render_Quad(m_pRingShader, m_pRingTexture, m_WorldPosition,
			RING_BASE_DIAMETER * ringScale, ringTint)))
	{
		return E_FAIL;
	}
	return S_OK;
}

HRESULT Client::CClickMoveEffect::Render_Quad(
	const shared_ptr<Engine::CShader>& shader,
	const shared_ptr<Engine::CTexture>& texture,
	const float3_t& position,
	const f32_t diameter,
	const float4_t& tint)
{
	if (nullptr == shader || nullptr == texture || nullptr == m_pRect ||
		!std::isfinite(diameter) || diameter <= 0.f)
	{
		return E_INVALIDARG;
	}
	float4x4_t world{};
	XMStoreFloat4x4(&world,
		XMMatrixScaling(diameter, diameter, 1.f) *
		XMMatrixRotationX(XM_PIDIV2) *
		XMMatrixTranslation(position.x, position.y + 0.035f, position.z));
	if (FAILED(shader->Bind_Matrix("g_WorldMatrix", &world)) ||
		FAILED(shader->Bind_Matrix(
			"g_ViewMatrix", CGameInstance::Get().Get_Transform(D3DTS::VIEW))) ||
		FAILED(shader->Bind_Matrix(
			"g_ProjMatrix", CGameInstance::Get().Get_Transform(D3DTS::PROJ))) ||
		FAILED(shader->Bind_RawValue("g_TintLinear", &tint, sizeof(tint))) ||
		FAILED(texture->Bind_ShaderResource(shader, "g_CoverageTexture", 0u)) ||
		FAILED(shader->Begin(0u)) ||
		FAILED(m_pRect->Bind_Resources()) ||
		FAILED(m_pRect->Render()))
	{
		return E_FAIL;
	}
	return S_OK;
}

unique_ptr<Client::CClickMoveEffect> Client::CClickMoveEffect::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto instance = unique_ptr<CClickMoveEffect>(
		new CClickMoveEffect(std::move(pDevice), std::move(pContext)));
	if (FAILED(instance->Initialize_Prototype()))
		return nullptr;
	return instance;
}

shared_ptr<CPrototype> Client::CClickMoveEffect::Clone(void* pArg)
{
	auto instance = shared_ptr<CClickMoveEffect>(
		new CClickMoveEffect(*this));
	if (FAILED(instance->Initialize(pArg)))
		return nullptr;
	return instance;
}
