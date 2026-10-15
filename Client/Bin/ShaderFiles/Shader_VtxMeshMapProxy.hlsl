// Baking always reads source material mips without the user's runtime MinLOD.
// CShader's quality policy matches sampler names; these private names opt out.
#define MaterialAnisotropicSampler MapProxyBakeMaterialSampler
#define SurfaceAnisotropicSampler MapProxyBakeSurfaceSampler
#define SurfaceMirrorUSampler MapProxyBakeMirrorUSampler
#define SurfaceMirrorVSampler MapProxyBakeMirrorVSampler
#define SurfaceMirrorUVSampler MapProxyBakeMirrorUVSampler
#define SourceCharacterSampler MapProxyBakeCharacterSampler
#define SourceCharacterStampSampler MapProxyBakeStampSampler
#define SourceMapMonsterStateSampler MapProxyBakeMonsterSampler
#define SourceMapSkyCloudSampler MapProxyBakeSkySampler
#define LinearSampler MapProxyBakeLinearSampler
#define MAP_CHUNK_ONLY 1
#include "Shader_VtxMeshMapInstance.hlsl"

// Original VTXMESHINSTANCE, 14 float4 rows / 224 bytes.
struct MAP_PROXY_SOURCE
{
    float4 world0, world1, world2, world3;
    float4 inverse0, inverse1, inverse2, inverse3;
    float4 lightmapScaleBias, averageScale, directionalScale, shadowScaleBias;
    float4 windOwner, windDimensions;
};
StructuredBuffer<MAP_PROXY_SOURCE> g_MapProxySources;
// x = SourceBG flags & 1023; y = baked bit0, shadow bit1, channel bits2..5.
StructuredBuffer<uint2> g_MapProxyMetadata;

Texture2D g_MapProxyDiffuseAtlas;
Texture2D g_MapProxyNormalAtlas;
Texture2D g_MapProxySpecularAtlas;
Texture2D g_MapProxyIndirectAtlas;
Texture2D g_MapProxyAverageAtlas;
Texture2D g_MapProxyDirectionalAtlas;
SamplerState MapProxyAtlasSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
    MinLOD = 0;
    MaxLOD = 3;
};

// Slot 0 is the original 76-byte vertex. Slot 1 is 12 bytes per vertex.
struct VS_PROXY_IN
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float2 uv : TEXCOORD0;
    float2 lightmapUV : TEXCOORD1;
    float4 color : COLOR0;
    float2 uv2 : TEXCOORD2;
    float2 atlasUV : ATLASUV0;
    uint sourceIndex : SOURCEINDEX0;
};

struct VS_PROXY_OUT
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float2 rawUV : TEXCOORD0;
    float4 worldPosition : TEXCOORD1;
    float4 projection : TEXCOORD2;
    float2 lightmapUV : TEXCOORD3;
    float2 shadowUV : TEXCOORD4;
    nointerpolation float4 averageScale : TEXCOORD5;
    nointerpolation float4 directionalScale : TEXCOORD6;
    float2 atlasUV : TEXCOORD7;
    nointerpolation uint sourceIndex : TEXCOORD8;
    float4 color : COLOR0;
};

VS_PROXY_OUT VS_PROXY(VS_PROXY_IN input)
{
    const MAP_PROXY_SOURCE source = g_MapProxySources[input.sourceIndex];
    VS_IN original = (VS_IN)0;
    original.vPosition = input.position;
    original.vNormal = input.normal;
    original.vTangent = input.tangent;
    original.vBinormal = input.binormal;
    original.vTexcoord = input.uv;
    original.vLightmapUV = input.lightmapUV;
    original.vColor = input.color;
    original.vWorld0 = source.world0;
    original.vWorld1 = source.world1;
    original.vWorld2 = source.world2;
    original.vWorld3 = source.world3;
    original.vWorldInvTranspose0 = source.inverse0;
    original.vWorldInvTranspose1 = source.inverse1;
    original.vWorldInvTranspose2 = source.inverse2;
    original.vWorldInvTranspose3 = source.inverse3;
    original.vLightmapScaleBias = source.lightmapScaleBias;
    original.vLightmapAverageScale = source.averageScale;
    original.vLightmapDirectionalScale = source.directionalScale;
    original.vStaticShadowScaleBias = source.shadowScaleBias;
    original.vSourceWindOwnerPosition = source.windOwner;
    original.vSourceWindDimensionsAndRadius = source.windDimensions;
    const VS_OUT transformed = VS_MAIN(original);
    VS_PROXY_OUT output;
    output.position = transformed.vPosition;
    output.normal = transformed.vNormal.xyz;
    output.tangent = transformed.vTangent.xyz;
    output.binormal = transformed.vBinormal.xyz;
    output.rawUV = transformed.vRawTexcoord;
    output.worldPosition = transformed.vWorldPos;
    output.projection = transformed.vProjPos;
    output.lightmapUV = transformed.vLightmapUV;
    output.shadowUV = transformed.vStaticShadowUV;
    output.averageScale = transformed.vLightmapAverageScale;
    output.directionalScale = transformed.vLightmapDirectionalScale;
    output.atlasUV = input.atlasUV;
    output.sourceIndex = input.sourceIndex;
    output.color = transformed.vColor;
    return output;
}

VS_PROXY_OUT VS_PROXY_BAKE(VS_PROXY_IN input)
{
    VS_PROXY_OUT output = VS_PROXY(input);
    // D3D texture Y increases downward; xatlas UVs are used in that same basis.
    output.position = float4(input.atlasUV.x * 2.f - 1.f, 1.f - input.atlasUV.y * 2.f, 0.f, 1.f);
    return output;
}

struct PS_PROXY_BAKE_OUT
{
    float4 diffuse : SV_TARGET0;     // RGBA8: normalized HDR diffuse, original alpha.
    float4 normal : SV_TARGET1;      // RGBA16F: tangent normal, native specular power.
    float4 specular : SV_TARGET2;    // RGBA16F: specular RGB, diffuse HDR scale.
    float3 indirect : SV_TARGET3;    // R11G11B10F: diffuse RNM + static emissive only.
    float4 average : SV_TARGET4;     // RGBA8: scaled RNM average, static shadow.
    float3 directional : SV_TARGET5; // R11G11B10F: scaled RNM coefficients.
};

PS_PROXY_BAKE_OUT PS_PROXY_BAKE(VS_PROXY_OUT input)
{
    PS_PROXY_BAKE_OUT output = (PS_PROXY_BAKE_OUT)0;
    // Admission excludes view-dependent diffuse, rim/subspecular, animation,
    // alpha clipping, unlit and vertex deformation before this pass is used.
    const MAP_SURFACE_SAMPLE surface = EvaluateMapSourceBGSurface(input.rawUV,
        input.color, input.worldPosition.xyz, input.tangent, input.binormal, input.normal);
    const float scale = max(1.f, max(surface.diffuse.r, max(surface.diffuse.g, surface.diffuse.b)));
    output.diffuse = float4(surface.diffuse.rgb / scale, surface.diffuse.a);
    output.normal = float4(surface.tangentNormal, g_SurfaceSpecularPower);
    output.specular = float4(surface.specular, scale);
    // specular.a >= 1 is also the coverage mask for atlas padding. Clear is 0.
    const bool baked = g_HasBakedLighting != 0u && input.averageScale.w != 0.f;
    float3 average = 0.f, coefficients = 0.f;
    if (baked)
    {
        average = g_BakedAverageTexture.Sample(SurfaceLightmapSampler, input.lightmapUV).rgb * input.averageScale.rgb;
        coefficients = g_BakedDirectionalTexture.Sample(SurfaceLightmapSampler, input.lightmapUV).rgb * input.directionalScale.rgb;
    }
    output.indirect = surface.diffuse.rgb * average *
        MapSourceDirectionalLightmapWeight(surface.tangentNormal, coefficients) +
        EvaluateMapSurfaceEmissive(input.rawUV, 8u);
    output.average = float4(average, EvaluateMapStaticShadow(input.shadowUV));
    output.directional = coefficients;
    return output;
}

PS_OUT PS_PROXY(VS_PROXY_OUT input)
{
    PS_OUT output = (PS_OUT)0;
    const float4 diffuse = g_MapProxyDiffuseAtlas.Sample(MapProxyAtlasSampler, input.atlasUV);
    const float4 normalPower = g_MapProxyNormalAtlas.Sample(MapProxyAtlasSampler, input.atlasUV);
    const float4 specularScale = g_MapProxySpecularAtlas.Sample(MapProxyAtlasSampler, input.atlasUV);
    const float3 indirect = g_MapProxyIndirectAtlas.Sample(MapProxyAtlasSampler, input.atlasUV).rgb;
    const float4 averageShadow = g_MapProxyAverageAtlas.Sample(MapProxyAtlasSampler, input.atlasUV);
    const float3 coefficients = g_MapProxyDirectionalAtlas.Sample(MapProxyAtlasSampler, input.atlasUV).rgb;
    const uint2 metadata = g_MapProxyMetadata[input.sourceIndex];
    const uint flags = metadata.x & 1023u;
    const bool baked = (metadata.y & 1u) != 0u;
    const bool hasShadow = (metadata.y & 2u) != 0u;
    const float3 tangentNormal = normalize(normalPower.xyz);
    const float3x3 tangentToWorld = float3x3(MapGeometryNormalizeOrZero(input.tangent),
        MapGeometryNormalizeOrZero(input.binormal), MapGeometryNormalizeOrZero(input.normal));
    const float3 worldNormal = normalize(mul(tangentNormal, tangentToWorld));
    output.vDiffuse = diffuse;
    if (g_SurfaceDebugView == 4u) output.vDiffuse.rgb = 0.f;
    output.vNormal = float4(worldNormal * .5f + .5f, 0.f);
    output.vDepth = float4(input.projection.z / input.projection.w,
        input.projection.w / 1000.f, normalPower.w, 8.f);
    output.vPickPos = input.worldPosition;
    float packedNormal = EncodeMapSurfaceGeometricNormal(input.normal, baked);
    if (hasShadow)
        packedNormal = asfloat((asuint(packedNormal) & 0x007fffffu) |
            ((127u + ((metadata.y >> 2u) & 15u)) << 23u));
    output.vPickPos.w = packedNormal;
    float3 radiance = indirect;
    if (baked && (flags & 4u) != 0u)
    {
        // The original view-dependent RNM specular remains live. Baking it into
        // indirect would freeze the lobe at the camera used during preparation.
        const float3 tangentView = normalize(mul(tangentToWorld, g_vCamPosition.xyz - input.worldPosition.xyz));
        const float3 reflected = reflect(-tangentView, tangentNormal);
        const float3 lobes = saturate(float3(
            dot(reflected.yz, float2(0.81649658f, 0.57735027f)),
            dot(reflected, float3(-0.70710678f, -0.40824829f, 0.57735027f)),
            dot(reflected, float3(0.70710678f, -0.40824829f, 0.57735027f))));
        radiance += specularScale.rgb * averageShadow.rgb * dot(coefficients, pow(lobes, normalPower.w + 1.f));
    }
    output.vEmissive = float4(radiance, hasShadow ? 1.f - averageShadow.a : 0.f);
    output.vMaterialSpecular = specularScale;
    output.vCharacterSurface = float4(0.f, 0.f, 0.f, float(flags));
    return output;
}

// 공유 low geometry는 원본 material atlas와 UV32/J32에서 native 조명을 다시 평가한다.
Texture2D g_OriginalTangent;
Texture2D g_OriginalBinormal;
Texture2D g_OriginalNormal;
Texture2D g_NativeStoneExtra;
Texture2D g_MapProxyUvJacobian;
PS_OUT PS_PROXY_SHARED(VS_PROXY_OUT input)
{
    PS_OUT output = (PS_OUT)0;
    const float4 diffuse = g_MapProxyDiffuseAtlas.Sample(MapProxyAtlasSampler, input.atlasUV);
    const float4 normalPower = g_MapProxyNormalAtlas.Sample(MapProxyAtlasSampler, input.atlasUV);
    const float4 specularScale = g_MapProxySpecularAtlas.Sample(MapProxyAtlasSampler, input.atlasUV);
    float3 indirect = 0.f; // compact admission requires static emissive == 0
    float4 averageShadow = 0.f; // 좌표는 색처럼 mip/filter하지 않는다.
    float3 coefficients = 0.f; // live RNM is evaluated below

    input.tangent=normalize(g_OriginalTangent.Sample(MapProxyAtlasSampler,input.atlasUV).xyz);
    input.binormal=normalize(g_OriginalBinormal.Sample(MapProxyAtlasSampler,input.atlasUV).xyz);
    input.normal=normalize(g_OriginalNormal.Sample(MapProxyAtlasSampler,input.atlasUV).xyz);
    uint aw,ah;g_OriginalNormal.GetDimensions(aw,ah);
    const uint2 pixel=min(uint2(saturate(input.atlasUV)*uint2(aw,ah)),uint2(aw-1u,ah-1u));
    uint2 metadata=uint2(round(float2(g_OriginalTangent.Load(int3(pixel,0)).w,g_OriginalBinormal.Load(int3(pixel,0)).w)));
    metadata.y*=65536u; // compact finite half state0/1 to native family flag




    // 원본 atlas는 local T/B/N이다. actual placement마다 같은 원본 instance 행렬로 방향을 옮긴다.
    const MAP_PROXY_SOURCE instance=g_MapProxySources[input.sourceIndex];
    const float4x4 world=float4x4(instance.world0,instance.world1,instance.world2,instance.world3);
    const float4x4 inverse=float4x4(instance.inverse0,instance.inverse1,instance.inverse2,instance.inverse3);
    input.tangent=MapGeometryNormalizeOrZero(mul(float4(input.tangent,0),world).xyz);
    input.binormal=MapGeometryNormalizeOrZero(mul(float4(input.binormal,0),world).xyz);
    input.normal=MapGeometryNormalizeOrZero(mul(float4(input.normal,0),inverse).xyz);
    // 실제 source triangle의 UV affine map을 low atlas texel 중심에서 연장한다.
    // UV island 사이의 값 평균/화면 quad 차분은 사용하지 않는다.
    const float2 rawCenterUV=g_MapProxyAverageAtlas.Load(int3(pixel,0)).xy;
    const float4 jacobian=g_MapProxyUvJacobian.Load(int3(pixel,0));
    const float2 delta=input.atlasUV-(float2(pixel)+.5f)/float2(aw,ah);
    const float2 rawUV=rawCenterUV+float2(dot(jacobian.xy,delta),dot(jacobian.zw,delta));
    const float2 atlasDx=ddx(input.atlasUV),atlasDy=ddy(input.atlasUV);
    const float2 rawDx=float2(dot(jacobian.xy,atlasDx),dot(jacobian.zw,atlasDx));
    const float2 rawDy=float2(dot(jacobian.xy,atlasDy),dot(jacobian.zw,atlasDy));
    const float2 lightmapUV=rawUV*instance.lightmapScaleBias.xy+instance.lightmapScaleBias.zw;
    const float2 shadowUV=rawUV*instance.shadowScaleBias.xy+instance.shadowScaleBias.zw;
    const bool hasLighting=g_HasBakedLighting!=0u&&instance.averageScale.w!=0.f;
    averageShadow=float4(0.f,0.f,0.f,(g_HasStaticShadow!=0u?EvaluateMapStaticShadowDistance(g_StaticShadowTexture.SampleGrad(SourceStaticShadowSampler,shadowUV,rawDx*instance.shadowScaleBias.xy,rawDy*instance.shadowScaleBias.xy).r):1.f));coefficients=0.f;
    if(hasLighting){averageShadow.rgb=g_BakedAverageTexture.SampleGrad(SurfaceLightmapSampler,lightmapUV,rawDx*instance.lightmapScaleBias.xy,rawDy*instance.lightmapScaleBias.xy).rgb*instance.averageScale.rgb;
        coefficients=g_BakedDirectionalTexture.SampleGrad(SurfaceLightmapSampler,lightmapUV,rawDx*instance.lightmapScaleBias.xy,rawDy*instance.lightmapScaleBias.xy).rgb*instance.directionalScale.rgb;}
    metadata.y=(metadata.y&65536u)|(hasLighting?1u:0u)|(g_HasStaticShadow!=0u?2u:0u)|((g_StaticShadowChannel&15u)<<2u);
    if((metadata.y&65536u)!=0u){
        const float3 mixed=SourceStoneUnit(g_NativeStoneExtra.Sample(MapProxyAtlasSampler,input.atlasUV).xyz);
        const float3 weights=SourceStoneDirectionalWeights(mixed);
        indirect+=diffuse.rgb*averageShadow.rgb*dot(coefficients,weights*weights);
    }else indirect+=diffuse.rgb*specularScale.a*averageShadow.rgb*MapSourceDirectionalLightmapWeight(normalize(normalPower.xyz),coefficients);

    if((metadata.y&65536u)!=0u)
    {
        const bool baked=(metadata.y&1u)!=0u,hasShadow=(metadata.y&2u)!=0u;
        const float4 extra=g_NativeStoneExtra.Sample(MapProxyAtlasSampler,input.atlasUV);
        const float3x3 tangentToWorld=float3x3(MapGeometryNormalizeOrZero(input.tangent),MapGeometryNormalizeOrZero(input.binormal),MapGeometryNormalizeOrZero(input.normal));
        const float3 bakedNormal=SourceStoneUnit(extra.xyz);
        const float3 view=SourceStoneUnit(mul(tangentToWorld,g_vCamPosition.xyz-input.worldPosition.xyz));
        const float3 reflected=2.f*dot(bakedNormal,view)*bakedNormal-view;
        const float3 weights=SourceStoneDirectionalWeights(reflected);
        const float3 specular=baked?averageShadow.rgb*specularScale.rgb*dot(coefficients,pow(weights,normalPower.w+1.f)):0.f;
        output.vDiffuse=diffuse;
        output.vNormal=float4(SourceStoneUnit(mul(normalPower.xyz,tangentToWorld))*.5f+.5f,0.f);
        output.vDepth=float4(input.projection.z/input.projection.w,input.projection.w/1000.f,normalPower.w,7.f);
        output.vPickPos=input.worldPosition;
        float packed=EncodeMapSurfaceGeometricNormal(input.normal,baked);
        if(hasShadow)packed=asfloat((asuint(packed)&0x007fffffu)|((127u+((metadata.y>>2u)&15u))<<23u));
        output.vPickPos.w=packed;
        output.vEmissive=float4(indirect+specular,hasShadow?1.f-averageShadow.a:0.f);
        output.vMaterialSpecular=float4(specularScale.rgb,extra.w);
        output.vCharacterSurface=float4(diffuse.rgb,0.f);
        output.vCharacterGeometry=float4(mul(extra.xyz,tangentToWorld),(metadata.x&1024u)!=0u?1.f:0.f);
        return output;
    }
    const uint flags = metadata.x & 1023u;
    const bool baked = (metadata.y & 1u) != 0u;
    const bool hasShadow = (metadata.y & 2u) != 0u;
    const float3 tangentNormal = normalize(normalPower.xyz);
    const float3x3 tangentToWorld = float3x3(MapGeometryNormalizeOrZero(input.tangent),
        MapGeometryNormalizeOrZero(input.binormal), MapGeometryNormalizeOrZero(input.normal));
    const float3 worldNormal = normalize(mul(tangentNormal, tangentToWorld));
    output.vDiffuse = diffuse;
    if (g_SurfaceDebugView == 4u) output.vDiffuse.rgb = 0.f;
    output.vNormal = float4(worldNormal * .5f + .5f, 0.f);
    output.vDepth = float4(input.projection.z / input.projection.w,
        input.projection.w / 1000.f, normalPower.w, 8.f);
    output.vPickPos = input.worldPosition;
    float packedNormal = EncodeMapSurfaceGeometricNormal(input.normal, baked);
    if (hasShadow)
        packedNormal = asfloat((asuint(packedNormal) & 0x007fffffu) |
            ((127u + ((metadata.y >> 2u) & 15u)) << 23u));
    output.vPickPos.w = packedNormal;
    float3 radiance = indirect;
    if (baked && (flags & 4u) != 0u)
    {
        // The original view-dependent RNM specular remains live. Baking it into
        // indirect would freeze the lobe at the camera used during preparation.
        const float3 tangentView = normalize(mul(tangentToWorld, g_vCamPosition.xyz - input.worldPosition.xyz));
        const float3 reflected = reflect(-tangentView, tangentNormal);
        const float3 lobes = saturate(float3(
            dot(reflected.yz, float2(0.81649658f, 0.57735027f)),
            dot(reflected, float3(-0.70710678f, -0.40824829f, 0.57735027f)),
            dot(reflected, float3(0.70710678f, -0.40824829f, 0.57735027f))));
        radiance += specularScale.rgb * averageShadow.rgb * dot(coefficients, pow(lobes, normalPower.w + 1.f));
    }
    output.vEmissive = float4(radiance, hasShadow ? 1.f - averageShadow.a : 0.f);
    output.vMaterialSpecular = specularScale;
    output.vCharacterSurface = float4(0.f, 0.f, 0.f, float(flags));
    return output;
}



VertexShader MapProxyBakeVS = compile vs_5_0 VS_PROXY_BAKE();
VertexShader MapProxyVS = compile vs_5_0 VS_PROXY();
PixelShader MapProxyBakePS = compile ps_5_0 PS_PROXY_BAKE();
PixelShader MapProxyPS = compile ps_5_0 PS_PROXY();

PixelShader MapProxySharedPS = compile ps_5_0 PS_PROXY_SHARED();

technique11 DefaultTechnique
{
    pass Bake
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = MapProxyBakeVS;
        GeometryShader = NULL;
        PixelShader = MapProxyBakePS;
    }
    pass Back
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = MapProxyVS;
        GeometryShader = NULL;
        PixelShader = MapProxyPS;
    }
    pass Front
    {
        SetRasterizerState(RS_Cull_CW);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = MapProxyVS;
        GeometryShader = NULL;
        PixelShader = MapProxyPS;
    }
    pass TwoSided
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = MapProxyVS;
        GeometryShader = NULL;
        PixelShader = MapProxyPS;
    }
    pass SharedBack
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = MapProxyVS;
        GeometryShader = NULL;
        PixelShader = MapProxySharedPS;
    }
    pass SharedFront
    {
        SetRasterizerState(RS_Cull_CW);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = MapProxyVS;
        GeometryShader = NULL;
        PixelShader = MapProxySharedPS;
    }
    pass SharedTwoSided
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = MapProxyVS;
        GeometryShader = NULL;
        PixelShader = MapProxySharedPS;
    }
}
