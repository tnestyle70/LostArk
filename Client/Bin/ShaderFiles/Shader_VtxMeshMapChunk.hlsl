#define MAP_CHUNK_ONLY 1
#include "Shader_VtxMeshMapInstance.hlsl"

// Byte-for-byte VTXMESHINSTANCE: fourteen float4 rows, 224 bytes. Keep matrix
// rows explicit so StructuredBuffer layout cannot change matrix packing.
struct MAP_CLUSTER_SOURCE
{
    float4 vWorld0;
    float4 vWorld1;
    float4 vWorld2;
    float4 vWorld3;
    float4 vWorldInvTranspose0;
    float4 vWorldInvTranspose1;
    float4 vWorldInvTranspose2;
    float4 vWorldInvTranspose3;
    float4 vLightmapScaleBias;
    float4 vLightmapAverageScale;
    float4 vLightmapDirectionalScale;
    float4 vStaticShadowScaleBias;
    float4 vSourceWindOwnerPosition;
    float4 vSourceWindDimensionsAndRadius;
};

StructuredBuffer<MAP_CLUSTER_SOURCE> g_MapClusterSources;

// Slot 0 keeps the original 76-byte VTXMESH. Slot 1 is a four-byte source
// index per vertex; source transforms and lighting stay in the shared table.
struct VS_CHUNK_IN
{
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float3 vTangent : TANGENT;
    float3 vBinormal : BINORMAL;
    float2 vTexcoord : TEXCOORD0;
    float2 vLightmapUV : TEXCOORD1;
    float4 vColor : COLOR0;
    float2 vTexcoord2 : TEXCOORD2;
    uint iSourceIndex : SOURCEINDEX0;
};

VS_OUT VS_MAIN_CHUNK(VS_CHUNK_IN input)
{
    const MAP_CLUSTER_SOURCE source = g_MapClusterSources[input.iSourceIndex];
    VS_IN original;
    original.vPosition = input.vPosition;
    original.vNormal = input.vNormal;
    original.vTangent = input.vTangent;
    original.vBinormal = input.vBinormal;
    original.vTexcoord = input.vTexcoord;
    original.vLightmapUV = input.vLightmapUV;
    original.vColor = input.vColor;
    original.vWorld0 = source.vWorld0;
    original.vWorld1 = source.vWorld1;
    original.vWorld2 = source.vWorld2;
    original.vWorld3 = source.vWorld3;
    original.vWorldInvTranspose0 = source.vWorldInvTranspose0;
    original.vWorldInvTranspose1 = source.vWorldInvTranspose1;
    original.vWorldInvTranspose2 = source.vWorldInvTranspose2;
    original.vWorldInvTranspose3 = source.vWorldInvTranspose3;
    original.vLightmapScaleBias = source.vLightmapScaleBias;
    original.vLightmapAverageScale = source.vLightmapAverageScale;
    original.vLightmapDirectionalScale = source.vLightmapDirectionalScale;
    original.vStaticShadowScaleBias = source.vStaticShadowScaleBias;
    original.vSourceWindOwnerPosition = source.vSourceWindOwnerPosition;
    original.vSourceWindDimensionsAndRadius = source.vSourceWindDimensionsAndRadius;
    return VS_MAIN(original);
}

VertexShader MapChunkVS = compile vs_5_0 VS_MAIN_CHUNK();
PixelShader MapChunkSourceBgPS = compile ps_5_0 PS_MAIN_SOURCE_BG_BANK();

technique11 DefaultTechnique
{
    pass SourceBGChunkBackPass
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = MapChunkVS;
        GeometryShader = NULL;
        PixelShader = MapChunkSourceBgPS;
    }

    pass SourceBGChunkFrontPass
    {
        SetRasterizerState(RS_Cull_CW);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = MapChunkVS;
        GeometryShader = NULL;
        PixelShader = MapChunkSourceBgPS;
    }

    pass SourceBGChunkTwoSidedPass
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = MapChunkVS;
        GeometryShader = NULL;
        PixelShader = MapChunkSourceBgPS;
    }
}
