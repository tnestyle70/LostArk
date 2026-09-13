#pragma once

#include "Engine_Typedef.h"
#include "Engine_RenderFwd.h"
#include <d3d11.h>
#include "Engine_Enum.h"
#include <wrl/client.h>
#include <cstddef>

namespace Engine
{
	// The renderer owns the active scene cube; a staged value retains its SRV
	// without changing current lighting until the profile transaction commits.
	struct RENDER_ENVIRONMENT_STATE final
	{
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pCube;
		wstring_t strCubePath;
		float4_t vColor = { 1.f, 1.f, 1.f, 0.f };
		float4_t vRotationIntensity = { 0.f, 1.f, 1.f, 0.f };
	};

	typedef struct tagLightDesc
	{
		LIGHT		eType;
		DirectX::XMFLOAT4	vDirection;
		DirectX::XMFLOAT4	vPosition;
		float		fRange;
		float		fFalloffExponent = 1.f;
		DirectX::XMFLOAT4	vDiffuse, vAmbient, vSpecular;
		float		fSpotInnerCos = 1.f;
		float		fSpotOuterCos = 1.f;
		LIGHT_RECEIVER eReceiver = LIGHT_RECEIVER::ALL;
        uint32_t staticShadowChannel = 0u;
	}LIGHT_DESC;
	static_assert(sizeof(LIGHT_DESC) == 108u);
	static_assert(offsetof(LIGHT_DESC, fRange) == 36u);
	static_assert(offsetof(LIGHT_DESC, fFalloffExponent) == 40u);
	static_assert(offsetof(LIGHT_DESC, vDiffuse) == 44u);
	static_assert(offsetof(LIGHT_DESC, fSpotInnerCos) == 92u);
	static_assert(offsetof(LIGHT_DESC, fSpotOuterCos) == 96u);

	typedef struct tagShadowSettings
	{
		bool_t	bEnabled = false;
		f32_t	fOrthographicWidth = 40.f;
		f32_t	fOrthographicHeight = 40.f;
		f32_t	fNear = 0.1f;
		f32_t	fFar = 150.f;
		f32_t	fDepthBias = 0.0015f;
		f32_t	fNormalBias = 0.02f;
		f32_t	fStrength = 0.7f;
	}SHADOW_SETTINGS;

	typedef struct tagShadowLightDesc
	{
		float4_t			vEye = float4_t(0.f, 20.f, -20.f, 1.f);
		float4_t			vAt = float4_t(0.f, 0.f, 0.f, 1.f);
		SHADOW_SETTINGS		Settings = {};
	}SHADOW_LIGHT_DESC;

	typedef struct tagRenderQualitySettings
	{
		bool_t	bSSAOEnabled = true;
		f32_t	fSSAORadius = 0.75f;
		f32_t	fSSAOBias = 0.025f;
		f32_t	fSSAOIntensity = 1.f;
		f32_t	fSSAOPower = 1.25f;
		f32_t	fSSAODistanceFade = 60.f;
		bool_t	bBloomEnabled = true;
		f32_t	fBloomThreshold = 1.f;
		f32_t	fBloomSoftKnee = 0.5f;
		f32_t	fBloomIntensity = 0.8f;
		f32_t	fBloomScatter = 1.f;
		f32_t	fExposure = 2.f;
		f32_t	fWhitePoint = 11.2f;
		f32_t	fGamma = 2.2f;
		bool_t	bFXAAEnabled = false;
		f32_t	fFXAASubpixel = 0.75f;
		f32_t	fFXAAEdgeThreshold = 0.166f;
		f32_t	fFXAAEdgeThresholdMin = 0.0833f;
	}RENDER_QUALITY_SETTINGS;

	enum class MATERIAL_DEBUG_VIEW : uint32_t
	{
		FINAL,
		BASE_COLOR,
		NORMAL,
		DIRECT_SPECULAR,
		REFLECTION_DELTA,
		ROUGHNESS,
		METALLIC,
		AMBIENT_OCCLUSION,
		END,
	};

	/* Session-only comparison state; never serialized with scene quality. */
	struct MATERIAL_RENDER_SETTINGS
	{
		bool_t bUseSourceMaterials = true;
		MATERIAL_DEBUG_VIEW eDebugView = MATERIAL_DEBUG_VIEW::FINAL;
	};

	/* Height fog is a screen space term applied where the deferred combine
	   already reconstructs world position, so terrain, buildings and
	   characters all receive it from one place. The blend group and effects
	   draw after that pass and stay clear of the fog on purpose.
	   fTopHeight is the world height the fog fades out at; fHeightFalloff is
	   the exponential rate below it. The drift fields let the fog breathe
	   without any per frame CPU work. */
	typedef struct tagHeightFogSettings
	{
		bool_t		bEnabled = false;
		float4_t	vColor = float4_t(0.55f, 0.62f, 0.72f, 1.f);
		f32_t		fDensity = 0.35f;
		f32_t		fHeightFalloff = 0.08f;
		f32_t		fTopHeight = 24.f;
		f32_t		fStartDistance = 0.f;
		f32_t		fMaximumOpacity = 0.9f;
		f32_t		fDriftSpeed = 0.f;
		f32_t		fDriftHeightAmplitude = 0.f;
		f32_t		fDriftDensityAmplitude = 0.f;
		/* Coverage turns the blanket into drifting cloud banks. 1 keeps the
		   whole map fogged; lower values thin it to patches whose total area
		   matches the fraction. The wind vector moves the pattern through
		   world XZ, so the banks travel without any CPU simulation. */
		f32_t		fCoveragePercent = 1.f;
		f32_t		fWindDirectionX = 1.f;
		f32_t		fWindDirectionZ = 0.f;
		f32_t		fWindSpeed = 0.f;
		f32_t		fPatchScale = 0.01f;
		f32_t		fPatchSoftness = 0.15f;
        // Optional source exponential model; distances/heights are runtime metres.
        bool_t bSourceExponential = false;
        float4_t vInscatteringColor = float4_t(0.f, 0.f, 0.f, 0.f);
        // xyz points toward the directional light; w is terminator-angle cosine.
        float4_t vFogLightDirection = float4_t(0.f, 1.f, 0.f, 0.f);
	}HEIGHT_FOG_SETTINGS;
}
