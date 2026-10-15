#pragma once

// CPU 정점 데이터의 DirectX 수학 타입과 고정 폭 정수를 사용한다.
#include "Engine_Typedef.h"
// shader semantic·format·입력 slot·byte offset으로 D3D11 input layout을 선언한다.
#include <d3d11.h>
// CPU 구조체 필드 위치와 GPU 입력 offset이 일치하는지 offsetof로 검사한다.
#include <cstddef>

// 각 Elements는 shader 입력 layout 생성에 쓰는 정적 배열이다. 구조체 값은 CPU vertex/instance buffer의 내용이며 GPU 자원 소유자는 별도 VIBuffer/Model이다.
// 필드 순서·format·stride는 하나의 계약이므로 설명 정리에서 값을 재배치하지 않는다.
namespace Engine
{
	// 위치만 가진 정점이다. slot 0에서 float3 POSITION을 읽는다.
	typedef struct tagVertexPosition
	{
		DirectX::XMFLOAT3			vPosition;

		static constexpr uint32_t		iNumElements = { 1 };
		static constexpr D3D11_INPUT_ELEMENT_DESC		Elements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },			
		};
	}VTXPOS;

	
	// 2D texture 좌표를 함께 전달하는 정점이다. 위치는 12byte, UV는 그 다음 8byte다.
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

	// cube texture 조회용 3성분 좌표를 가진 정점이다. TEXCOORD의 의미는 2D UV와 다르다.
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

	// 조명 계산에 쓸 법선과 2D UV를 가진 정점이다. 공간 변환과 정규화는 소비 shader가 담당한다.
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


	// 정적 mesh의 위치·접선 basis·UV 세 묶음·정점 색상이다. COLOR는 packed RGBA8을 GPU에서 0~1로 읽는다.
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
	// 실제 구조체 크기/필드 offset을 GPU layout 선언과 대조한다. size는 vertex buffer의 stride로 소비된다.
	static_assert(sizeof(VTXMESH) == 76);
	static_assert(offsetof(VTXMESH, vTexcoord2) == 68);
	static_assert(offsetof(VTXMESH, vPosition) == 0);
	static_assert(offsetof(VTXMESH, vNormal) == 12);
	static_assert(offsetof(VTXMESH, vTangent) == 24);
	static_assert(offsetof(VTXMESH, vBinormal) == 36);
	static_assert(offsetof(VTXMESH, vTexcoord) == 48);
	static_assert(offsetof(VTXMESH, vTexcoord1) == 56);
	static_assert(offsetof(VTXMESH, color0Rgba8) == 64);

	// slot 1에서 instance마다 한 번 진행하는 정적 mesh record다. slot 0은 VTXMESH의 정점 입력을 읽는다.
	// 이 instance layout의 slot 0에는 TEXCOORD2가 없으므로 일반 VTXMESH의 모든 semantic을 그대로 제공한다고 가정하지 않는다.
	typedef struct tagVertexMeshInstance
	{
		// 배치의 월드 변환과 법선용 inverse-transpose다. 배치의 행렬 준비와 GPU instance payload 생성은 Client 맵 배치 경로가 맡는다.
		float4x4_t World = {};
		float4x4_t WorldInvTranspose = {};
		// instance별 lightmap UV와 광량 scale, 정적 shadow UV, 원본 바람 입력을 각 float4로 전달한다.
		float4_t vLightmapScaleBias = {};
		float4_t vLightmapAverageScale = {};
		float4_t vLightmapDirectionalScale = {};
        float4_t vStaticShadowScaleBias = {};
        float4_t vSourceWindOwnerPosition = {};
        float4_t vSourceWindDimensionsAndRadius = {};

		static constexpr uint32_t iNumElements = { 21 };

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
            { "INSTANCE_SOURCE_WIND", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,
                1, 192, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
            { "INSTANCE_SOURCE_WIND", 1, DXGI_FORMAT_R32G32B32A32_FLOAT,
                1, 208, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		};
	} VTXMESHINSTANCE;
	// instance record는 224byte다. 다음 offset 검사는 shader가 읽는 후반 float4 입력 위치도 고정한다.
	static_assert(sizeof(VTXMESHINSTANCE) == 224);
    static_assert(offsetof(VTXMESHINSTANCE, vSourceWindOwnerPosition) == 192);
    static_assert(offsetof(VTXMESHINSTANCE, vSourceWindDimensionsAndRadius) == 208);
    static_assert(offsetof(VTXMESHINSTANCE, vStaticShadowScaleBias) == 176);
	static_assert(offsetof(VTXMESHINSTANCE, vLightmapScaleBias) == 128);
	static_assert(offsetof(VTXMESHINSTANCE, vLightmapAverageScale) == 144);
	static_assert(offsetof(VTXMESHINSTANCE, vLightmapDirectionalScale) == 160);


	// 본 palette index 네 개와 weight 네 개를 가진 skinned mesh 정점이다. index는 정수 입력, weight는 float 입력이다.
	// 정적 VTXMESH와 stride/UV 위치가 다르므로 서로 같은 buffer layout으로 사용하지 않는다.
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
	// 104byte stride와 skinning/추가 UV 필드 위치를 확인한다. 이 구조는 weight를 자동 정규화하거나 index를 검증하지 않는다.
	static_assert(sizeof(VTXANIMMESH) == 104);
	static_assert(offsetof(VTXANIMMESH, vBlendIndices) == 56);
	static_assert(offsetof(VTXANIMMESH, vBlendWeights) == 72);
	static_assert(offsetof(VTXANIMMESH, vTexcoord1) == 88);
	static_assert(offsetof(VTXANIMMESH, vTexcoord2) == 96);

	// instance 변환의 basis 세 축과 translation을 네 float4로 보관한다. 입력 layout은 이를 사용하는 buffer가 선택한다.
	typedef struct tagVertexInstanceModel
	{
		DirectX::XMFLOAT4			vRight;
		DirectX::XMFLOAT4			vUp;
		DirectX::XMFLOAT4			vLook;
		DirectX::XMFLOAT4			vTranslation;		
	}VTXINSTANCE_MODEL;


	// instance basis/translation 뒤에 두 성분의 수명 데이터를 보관한다. 수명의 각 성분 해석과 갱신은 소비자가 담당한다.
	typedef struct tagVertexInstanceParticle
	{
		DirectX::XMFLOAT4			vRight;
		DirectX::XMFLOAT4			vUp;
		DirectX::XMFLOAT4			vLook;
		DirectX::XMFLOAT4			vTranslation;
		DirectX::XMFLOAT2			vLifeTime;	
	}VTXINSTANCE_PARTICLE;
	
	// 실제 정점 멤버 없이 layout만 제공한다. slot 0은 사각형 위치/UV, slot 1은 particle의 네 float4와 수명 float2다.
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

	// point 입자 layout이다. slot 0의 위치에 slot 1의 WORLD0~3과 수명 TEXCOORD를 결합한다.
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
