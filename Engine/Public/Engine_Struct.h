#ifndef Engine_Struct_h__
#define Engine_Struct_h__

#include "Engine_Typedef.h"

#include <cstddef>

namespace Engine
{
	typedef struct tagEngineDesc
	{
		HINSTANCE	hInstance = {};
		HWND		hWnd = {};
		WINMODE		eWinMode = { WINMODE::END };
		D3D_DRIVER_TYPE eDriverType = D3D_DRIVER_TYPE_HARDWARE;
		bool_t		bNonInteractiveErrors = false;
		uint32_t	iNumLevels = {};
		uint32_t	iWinSizeX{}, iWinSizeY{};
	}ENGINE_DESC;

	typedef struct tagLightDesc
	{
		LIGHT		eType;
		XMFLOAT4	vDirection;
		XMFLOAT4	vPosition;
		float		fRange;
		float		fFalloffExponent = 1.f;
		XMFLOAT4	vDiffuse, vAmbient, vSpecular;
	}LIGHT_DESC;
	static_assert(sizeof(LIGHT_DESC) == 92u);
	static_assert(offsetof(LIGHT_DESC, fRange) == 36u);
	static_assert(offsetof(LIGHT_DESC, fFalloffExponent) == 40u);
	static_assert(offsetof(LIGHT_DESC, vDiffuse) == 44u);

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
	}HEIGHT_FOG_SETTINGS;

	typedef struct tagKeyFrame
	{
		XMFLOAT3	vScale;
		XMFLOAT4	vRotation;
		XMFLOAT3	vTranslation;
		float		fTrackPosition;
	}KEYFRAME;

	typedef struct tagVertexPosition
	{
		XMFLOAT3			vPosition;

		static constexpr uint32_t		iNumElements = { 1 };
		static constexpr D3D11_INPUT_ELEMENT_DESC		Elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },			
		};
	}VTXPOS;

	
	typedef struct tagVertexPositionTexcoord
	{
		XMFLOAT3			vPosition;
		XMFLOAT2			vTexcoord;

		static constexpr uint32_t		iNumElements = { 2 };
		static constexpr D3D11_INPUT_ELEMENT_DESC		Elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
	}VTXTEX;

	typedef struct tagVertexCube
	{
		XMFLOAT3			vPosition;
		XMFLOAT3			vTexcoord;

		static constexpr uint32_t		iNumElements = { 2 };
		static constexpr D3D11_INPUT_ELEMENT_DESC		Elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
	}VTXCUBE;

	typedef struct tagVertexPositionNormalTexcoord
	{
		XMFLOAT3			vPosition;
		XMFLOAT3			vNormal;
		XMFLOAT2			vTexcoord;

		static constexpr uint32_t		iNumElements = { 3 };
		static constexpr D3D11_INPUT_ELEMENT_DESC		Elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
	}VTXNORTEX;


	typedef struct tagVertexMesh
	{
		XMFLOAT3			vPosition;
		XMFLOAT3			vNormal;
		XMFLOAT3			vTangent;
		XMFLOAT3			vBinormal;
		XMFLOAT2			vTexcoord;

		static constexpr uint32_t		iNumElements = { 5 };
		static constexpr D3D11_INPUT_ELEMENT_DESC		Elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
	}VTXMESH;
	static_assert(sizeof(VTXMESH) == 56);
	static_assert(offsetof(VTXMESH, vPosition) == 0);
	static_assert(offsetof(VTXMESH, vNormal) == 12);
	static_assert(offsetof(VTXMESH, vTangent) == 24);
	static_assert(offsetof(VTXMESH, vBinormal) == 36);
	static_assert(offsetof(VTXMESH, vTexcoord) == 48);

	typedef struct tagVertexMeshInstance
	{
		float4x4_t World = {};
		float4x4_t WorldInvTranspose = {};

		static constexpr uint32_t iNumElements = { 13 };

		static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
				0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,
				0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT,
				0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,
				0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,
				0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },

			{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,
				1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT,
				1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT,
				1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT,
				1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },

			{ "WORLDINVTRANSPOSE", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,
				1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLDINVTRANSPOSE", 1, DXGI_FORMAT_R32G32B32A32_FLOAT,
				1, 80, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLDINVTRANSPOSE", 2, DXGI_FORMAT_R32G32B32A32_FLOAT,
				1, 96, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLDINVTRANSPOSE", 3, DXGI_FORMAT_R32G32B32A32_FLOAT,
				1, 112, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		};
	} VTXMESHINSTANCE;


	typedef struct tagVertexAnimationMesh
	{
		XMFLOAT3			vPosition;
		XMFLOAT3			vNormal;
		XMFLOAT3			vTangent;
		XMFLOAT3			vBinormal;
		XMFLOAT2			vTexcoord;
		XMUINT4				vBlendIndices;
		XMFLOAT4			vBlendWeights;

		static constexpr uint32_t		iNumElements = { 7 };
		static constexpr D3D11_INPUT_ELEMENT_DESC		Elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDINDEX", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 56, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 72, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
	}VTXANIMMESH;

	typedef struct tagVertexInstanceModel
	{
		XMFLOAT4			vRight;
		XMFLOAT4			vUp;
		XMFLOAT4			vLook;
		XMFLOAT4			vTranslation;		
	}VTXINSTANCE_MODEL;


	typedef struct tagVertexInstanceParticle
	{
		XMFLOAT4			vRight;
		XMFLOAT4			vUp;
		XMFLOAT4			vLook;
		XMFLOAT4			vTranslation;
		XMFLOAT2			vLifeTime;	
	}VTXINSTANCE_PARTICLE;
	
	typedef struct tagVertexInstanceParticleRect
	{
		static constexpr uint32_t		iNumElements = { 7 };
		static constexpr D3D11_INPUT_ELEMENT_DESC		Elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },

			{ "TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 4, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 5, DXGI_FORMAT_R32G32_FLOAT, 1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		};
	}VTXINSTANCE_PARTICLE_RECT;

	typedef struct tagVertexInstanceParticlePoint
	{
		static constexpr uint32_t		iNumElements = { 6 };
		static constexpr D3D11_INPUT_ELEMENT_DESC		Elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },			

			{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		};
	}VTXINSTANCE_PARTICLE_POINT;







	

	
}


#endif // Engine_Struct_h__
