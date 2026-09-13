#pragma once

#include "Engine_Typedef.h"
#include <d3d11.h>
#include <cstddef>

namespace Engine
{
	typedef struct tagVertexPosition
	{
		DirectX::XMFLOAT3			vPosition;

		static constexpr uint32_t		iNumElements = { 1 };
		static constexpr D3D11_INPUT_ELEMENT_DESC		Elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },			
		};
	}VTXPOS;

	
	typedef struct tagVertexPositionTexcoord
	{
		DirectX::XMFLOAT3			vPosition;
		DirectX::XMFLOAT2			vTexcoord;

		static constexpr uint32_t		iNumElements = { 2 };
		static constexpr D3D11_INPUT_ELEMENT_DESC		Elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
	}VTXTEX;

	typedef struct tagVertexCube
	{
		DirectX::XMFLOAT3			vPosition;
		DirectX::XMFLOAT3			vTexcoord;

		static constexpr uint32_t		iNumElements = { 2 };
		static constexpr D3D11_INPUT_ELEMENT_DESC		Elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
	}VTXCUBE;

	typedef struct tagVertexPositionNormalTexcoord
	{
		DirectX::XMFLOAT3			vPosition;
		DirectX::XMFLOAT3			vNormal;
		DirectX::XMFLOAT2			vTexcoord;

		static constexpr uint32_t		iNumElements = { 3 };
		static constexpr D3D11_INPUT_ELEMENT_DESC		Elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
	}VTXNORTEX;


	typedef struct tagVertexMesh
	{
		DirectX::XMFLOAT3			vPosition;
		DirectX::XMFLOAT3			vNormal;
		DirectX::XMFLOAT3			vTangent;
		DirectX::XMFLOAT3			vBinormal;
		DirectX::XMFLOAT2			vTexcoord;
		DirectX::XMFLOAT2			vTexcoord1 = {};
		uint32_t			color0Rgba8 = { 0xffffffffu };
		DirectX::XMFLOAT2			vTexcoord2 = {};

		static constexpr uint32_t		iNumElements = { 8 };
		static constexpr D3D11_INPUT_ELEMENT_DESC		Elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 56, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 64, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 2, DXGI_FORMAT_R32G32_FLOAT, 0, 68, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
	}VTXMESH;
	static_assert(sizeof(VTXMESH) == 76);
	static_assert(offsetof(VTXMESH, vTexcoord2) == 68);
	static_assert(offsetof(VTXMESH, vPosition) == 0);
	static_assert(offsetof(VTXMESH, vNormal) == 12);
	static_assert(offsetof(VTXMESH, vTangent) == 24);
	static_assert(offsetof(VTXMESH, vBinormal) == 36);
	static_assert(offsetof(VTXMESH, vTexcoord) == 48);
	static_assert(offsetof(VTXMESH, vTexcoord1) == 56);
	static_assert(offsetof(VTXMESH, color0Rgba8) == 64);

	typedef struct tagVertexMeshInstance
	{
		float4x4_t World = {};
		float4x4_t WorldInvTranspose = {};
		float4_t vLightmapScaleBias = {};
		float4_t vLightmapAverageScale = {};
		float4_t vLightmapDirectionalScale = {};
        float4_t vStaticShadowScaleBias = {};

		static constexpr uint32_t iNumElements = { 19 };

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
			{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT,
				0, 56, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM,
				0, 64, D3D11_INPUT_PER_VERTEX_DATA, 0 },

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
			{ "INSTANCE_LIGHTMAP", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,
				1, 128, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "INSTANCE_LIGHTMAP", 1, DXGI_FORMAT_R32G32B32A32_FLOAT,
				1, 144, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "INSTANCE_LIGHTMAP", 2, DXGI_FORMAT_R32G32B32A32_FLOAT,
				1, 160, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
            { "INSTANCE_SHADOW", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,
                1, 176, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		};
	} VTXMESHINSTANCE;
	static_assert(sizeof(VTXMESHINSTANCE) == 192);
    static_assert(offsetof(VTXMESHINSTANCE, vStaticShadowScaleBias) == 176);
	static_assert(offsetof(VTXMESHINSTANCE, vLightmapScaleBias) == 128);
	static_assert(offsetof(VTXMESHINSTANCE, vLightmapAverageScale) == 144);
	static_assert(offsetof(VTXMESHINSTANCE, vLightmapDirectionalScale) == 160);


	typedef struct tagVertexAnimationMesh
	{
		DirectX::XMFLOAT3			vPosition;
		DirectX::XMFLOAT3			vNormal;
		DirectX::XMFLOAT3			vTangent;
		DirectX::XMFLOAT3			vBinormal;
		DirectX::XMFLOAT2			vTexcoord;
		DirectX::XMUINT4				vBlendIndices;
		DirectX::XMFLOAT4			vBlendWeights;
		DirectX::XMFLOAT2			vTexcoord1 = {};
		DirectX::XMFLOAT2			vTexcoord2 = {};

		static constexpr uint32_t		iNumElements = { 9 };
		static constexpr D3D11_INPUT_ELEMENT_DESC		Elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDINDEX", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 56, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 72, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 88, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 2, DXGI_FORMAT_R32G32_FLOAT, 0, 96, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
	}VTXANIMMESH;
	static_assert(sizeof(VTXANIMMESH) == 104);
	static_assert(offsetof(VTXANIMMESH, vBlendIndices) == 56);
	static_assert(offsetof(VTXANIMMESH, vBlendWeights) == 72);
	static_assert(offsetof(VTXANIMMESH, vTexcoord1) == 88);
	static_assert(offsetof(VTXANIMMESH, vTexcoord2) == 96);

	typedef struct tagVertexInstanceModel
	{
		DirectX::XMFLOAT4			vRight;
		DirectX::XMFLOAT4			vUp;
		DirectX::XMFLOAT4			vLook;
		DirectX::XMFLOAT4			vTranslation;		
	}VTXINSTANCE_MODEL;


	typedef struct tagVertexInstanceParticle
	{
		DirectX::XMFLOAT4			vRight;
		DirectX::XMFLOAT4			vUp;
		DirectX::XMFLOAT4			vLook;
		DirectX::XMFLOAT4			vTranslation;
		DirectX::XMFLOAT2			vLifeTime;	
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
