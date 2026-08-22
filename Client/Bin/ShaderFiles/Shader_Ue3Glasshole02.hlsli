#ifndef SHADER_UE3_GLASSHOLE02_HLSLI
#define SHADER_UE3_GLASSHOLE02_HLSLI

// Source translation boundary:
//   fx_m_mi_j_00.fx_m.fx_j_pa_glasshole_02_tr
//   FParticleOffsetCenterDynamicParameterVertexFactory / NoDensity BasePass
//
// The register arrays deliberately retain the cooked shader's CB layout.  The
// translation harness supplies exactly the same bytes to this HLSL and to the
// original DXBC; semantic names may be introduced only after that comparison.
cbuffer Ue3Glasshole02MaterialConstants : register(b0)
{
    float4 g_Ue3Glasshole02CB0[22];
};

cbuffer Ue3Glasshole02SceneConstants : register(b2)
{
    float4 g_Ue3Glasshole02CB2[4];
};

Texture2D<float4> g_Ue3Glasshole02CrackNormal : register(t0);
Texture2D<float4> g_Ue3Glasshole02AtypicalMask : register(t1);
Texture2D<float4> g_Ue3Glasshole02SceneDepth : register(t2);
Texture2D<float4> g_Ue3Glasshole02InnerHole : register(t3);
Texture2D<float4> g_Ue3Glasshole02Dust : register(t4);
Texture2D<float4> g_Ue3Glasshole02Environment : register(t5);
Texture2D<float4> g_Ue3Glasshole02Aura : register(t6);
Texture2D<float4> g_Ue3Glasshole02EnvironmentOverlay : register(t7);

SamplerState g_Ue3Glasshole02SceneDepthSampler : register(s0);
SamplerState g_Ue3Glasshole02CrackNormalSampler : register(s1);
SamplerState g_Ue3Glasshole02AtypicalMaskSampler : register(s2);
SamplerState g_Ue3Glasshole02InnerHoleSampler : register(s3);
SamplerState g_Ue3Glasshole02DustSampler : register(s4);
SamplerState g_Ue3Glasshole02EnvironmentSampler : register(s5);
SamplerState g_Ue3Glasshole02AuraSampler : register(s6);
SamplerState g_Ue3Glasshole02EnvironmentOverlaySampler : register(s7);

struct Ue3Glasshole02PixelInput
{
    float4 tangentToWorld0 : TEXCOORD10;
    float4 tangentToWorld1 : TEXCOORD11;
    float4 particleUv : TEXCOORD0;
    float4 particleColor : TEXCOORD1;
    float4 dynamicParameter : TEXCOORD2;
    float4 fogAndSelection : TEXCOORD4;
    centroid float4 cameraToParticle : TEXCOORD6;
    centroid float4 screenPosition : TEXCOORD5;
    bool isFrontFace : SV_IsFrontFace;
};

struct Ue3Glasshole02PixelOutput
{
    float4 sceneColor : SV_Target0;
    float4 encodedNormal : SV_Target2;
    float4 sceneMetadata : SV_Target3;
    float4 unusedTarget4 : SV_Target4;
    float4 unusedTarget5 : SV_Target5;
};

// This is a readable, instruction-order-preserving translation of the 198
// instructions in the cooked Glasshole02 PS.  r0-r3 are intentionally kept as
// explicit float4 temporaries so component lifetime and mad/log2/exp2 order can
// be compared directly with the DXBC disassembly.
Ue3Glasshole02PixelOutput EvaluateUe3Glasshole02(
    Ue3Glasshole02PixelInput input)
{
    float4 r0 = 0.0f;
    float4 r1 = 0.0f;
    float4 r2 = 0.0f;
    float4 r3 = 0.0f;

    // 000-044: radial polar field, curve falloff, and dynamic distortion.
    r0.xy = input.particleUv.yx - float2(0.5f, 0.5f);
    r0.z = max(abs(r0.y), abs(r0.x));
    r0.z = 1.0f / r0.z;
    r0.w = min(abs(r0.y), abs(r0.x));
    r0.z = r0.z * r0.w;
    r0.w = r0.z * r0.z;
    r1.x = mad(r0.w, 0.020835f, -0.085133f);
    r1.x = mad(r0.w, r1.x, 0.180141f);
    r1.x = mad(r0.w, r1.x, -0.330299f);
    r0.w = mad(r0.w, r1.x, 0.999866f);
    r1.x = r0.w * r0.z;
    r1.x = mad(r1.x, -2.0f, 1.570796f);
    const bool polarUsesComplement = abs(r0.y) < abs(r0.x);
    r1.x = polarUsesComplement ? r1.x : 0.0f;
    r0.z = mad(r0.z, r0.w, r1.x);
    const bool polarYIsNegative = r0.y < -r0.y;
    r0.w = polarYIsNegative ? asfloat(0xc0490fdbu) : 0.0f;
    r0.z = r0.w + r0.z;
    r0.w = min(r0.y, r0.x);
    const bool polarCrossesNegativeAxis = r0.w < -r0.w;
    r1.x = max(r0.y, r0.x);
    r0.x = dot(r0.xy, r0.xy);
    r0.x = sqrt(r0.x);
    const bool polarHasPositiveComponent = r1.x >= -r1.x;
    const bool polarQuadrantFlip =
        polarHasPositiveComponent && polarCrossesNegativeAxis;
    r0.y = polarQuadrantFlip ? -r0.z : r0.z;
    r0.y = r0.y * g_Ue3Glasshole02CB0[17].z;
    r0.z = -r0.x + 0.5f;
    r0.z = dot(r0.zz, g_Ue3Glasshole02CB0[17].ww);
    r0.y = mad(r0.y, 0.318309886f, r0.z);
    r0.y = r0.y + g_Ue3Glasshole02CB0[18].x;
    r1.x = r0.y * g_Ue3Glasshole02CB0[18].w;
    r0.y = r0.x + r0.x;
    r0.y = log2(r0.y);
    r0.y = r0.y * g_Ue3Glasshole02CB0[18].y;
    r0.y = exp2(r0.y);
    r0.y = r0.y * input.dynamicParameter.w;
    const bool radialDistanceWasNegative = r0.x < 0.0f;
    r0.x = mad(-r0.x, 2.0f, 1.0f);
    r0.x = max(r0.x, 0.0f);
    r0.y = radialDistanceWasNegative ? 0.0f : r0.y;
    r0.z = input.dynamicParameter.z + g_Ue3Glasshole02CB0[18].z;
    r0.y = mad(r0.z, -0.4f, r0.y);
    r0.z = input.dynamicParameter.w * g_Ue3Glasshole02CB0[19].z;
    r1.y = mad(r0.y, g_Ue3Glasshole02CB0[19].x, r0.z);

    // 045-054: paired environment/dust lookup and powered radiance.
    r0.yz = r1.xy + g_Ue3Glasshole02CB0[10].xy;
    r1.xy = r1.xy + g_Ue3Glasshole02CB0[11].xy;
    r1.xyz = g_Ue3Glasshole02Environment.SampleLevel(
        g_Ue3Glasshole02EnvironmentSampler, r1.xy, -1.0f).xyz;
    r0.yzw = g_Ue3Glasshole02Dust.SampleLevel(
        g_Ue3Glasshole02DustSampler, r0.yz, -1.0f).xyz;
    r2.xyz = r1.xyz * r0.yzw;
    r0.yzw = r1.xyz + r0.yzw;
    r1.xyz = max(abs(r2.xyz), float3(0.000001f, 0.000001f, 0.000001f));
    r1.xyz = log2(r1.xyz);
    r1.xyz = r1.xyz * g_Ue3Glasshole02CB0[20].w;
    r1.xyz = exp2(r1.xyz);

    // 055-079: secondary polar coordinate around the authored hole centre.
    r2.xy = input.particleUv.xy - g_Ue3Glasshole02CB0[4].xy;
    r1.w = max(abs(r2.y), abs(r2.x));
    r1.w = 1.0f / r1.w;
    r2.z = min(abs(r2.y), abs(r2.x));
    r1.w = r1.w * r2.z;
    r2.z = r1.w * r1.w;
    r2.w = mad(r2.z, 0.020835f, -0.085133f);
    r2.w = mad(r2.z, r2.w, 0.180141f);
    r2.w = mad(r2.z, r2.w, -0.330299f);
    r2.z = mad(r2.z, r2.w, 0.999866f);
    r2.w = r1.w * r2.z;
    r2.w = mad(r2.w, -2.0f, 1.570796f);
    const bool holePolarUsesComplement = abs(r2.y) < abs(r2.x);
    r2.w = holePolarUsesComplement ? r2.w : 0.0f;
    r1.w = mad(r1.w, r2.z, r2.w);
    const bool holeYIsNegative = r2.y < -r2.y;
    r2.z = holeYIsNegative ? asfloat(0xc0490fdbu) : 0.0f;
    r1.w = r1.w + r2.z;
    r2.z = min(r2.y, r2.x);
    r2.x = max(r2.y, r2.x);
    const bool holeHasPositiveComponent = r2.x >= -r2.x;
    const bool holeCrossesNegativeAxis = r2.z < -r2.z;
    const bool holeQuadrantFlip =
        holeHasPositiveComponent && holeCrossesNegativeAxis;
    r1.w = holeQuadrantFlip ? -r1.w : r1.w;
    r2.x = mad(r1.w, 0.159154943f, 0.5f);

    // 080-107: crack-normal twist, atypical mask, aura, and base color.
    r3.xy = mad(
        input.particleUv.xy,
        g_Ue3Glasshole02CB0[6].xy,
        g_Ue3Glasshole02CB0[7].xy);
    r1.w = g_Ue3Glasshole02CrackNormal.SampleBias(
        g_Ue3Glasshole02CrackNormalSampler, r3.xy, 0.0f).x;
    r1.w = mad(r1.w, 2.0f, -1.0f);
    r3.xy = mad(
        input.particleUv.xy,
        g_Ue3Glasshole02CB0[6].xy,
        g_Ue3Glasshole02CB0[8].xy);
    r2.w = g_Ue3Glasshole02CrackNormal.SampleBias(
        g_Ue3Glasshole02CrackNormalSampler, r3.xy, 0.0f).x;
    r2.w = mad(r2.w, 2.0f, -1.0f);
    r3.xy = input.dynamicParameter.xy + float2(-0.5f, -0.9f);
    r2.w = r2.w * r3.y;
    r1.w = mad(r3.y, r1.w, r2.w);
    r1.w = r1.w * g_Ue3Glasshole02CB0[15].z;
    r3.yz = input.particleUv.xy - g_Ue3Glasshole02CB0[5].xy;
    r2.w = dot(r3.yz, r3.yz);
    r2.w = sqrt(r2.w);
    r3.y = mad(-r2.w, 2.0f, 1.0f);
    r2.w = mad(r2.w, 3.0f, -r3.x);
    r2.w = max(r2.w, -0.1f);
    r2.y = min(r2.w, 10.0f);
    r3.xy = mad(r3.yy, r1.ww, r2.xy);
    const float4 atypicalSample = g_Ue3Glasshole02AtypicalMask.SampleLevel(
        g_Ue3Glasshole02AtypicalMaskSampler, r3.xy, -1.0f);
    r2.y = atypicalSample.z;
    r2.z = atypicalSample.x;
    r3.xyz = r0.yzw * r2.z;
    r3.xyz = r3.xyz * 3.0f;
    r1.xyz = mad(g_Ue3Glasshole02CB0[21].xxx, r1.xyz, r3.xyz);
    r2.x = mad(r2.x, 7.0f, g_Ue3Glasshole02CB0[13].x);
    r2.w = r2.z + g_Ue3Glasshole02CB0[13].y;
    r3.xyz = g_Ue3Glasshole02Aura.SampleBias(
        g_Ue3Glasshole02AuraSampler, r2.xw, 0.0f).xyz;
    r3.xyz = r2.zzz * r3.xyz;
    r3.xyz = saturate(r3.xyz * 4.0f);
    r3.xyz = r3.xyz * r3.xyz;
    r1.xyz = mad(r1.xyz, g_Ue3Glasshole02CB0[12].xyz, r3.xyz);

    // 108-140: reflection overlay, scene-depth intersection, and opacity.
    r0.w = dot(input.cameraToParticle.xyz, input.cameraToParticle.xyz);
    r0.w = rsqrt(r0.w);
    r2.x = r0.w * input.cameraToParticle.x;
    r2.w = r0.w * input.cameraToParticle.y;
    r0.y = mad(r0.y, 0.2f, r2.x);
    r0.z = mad(r0.z, 0.2f, r2.w);
    r0.yzw = g_Ue3Glasshole02EnvironmentOverlay.SampleBias(
        g_Ue3Glasshole02EnvironmentOverlaySampler, r0.yz, 0.0f).xyz;
    r0.yzw = r0.yzw + r1.xyz;
    r1.x = dot(r0.yzw, float3(0.3f, 0.59f, 0.11f));
    r1.xyz = r1.xxx - r0.yzw;
    r0.yzw = mad(g_Ue3Glasshole02CB0[21].www, r1.xyz, r0.yzw);
    r1.xy = input.screenPosition.xy / input.screenPosition.w;
    r1.xy = mad(
        r1.xy,
        g_Ue3Glasshole02CB2[0].xy,
        g_Ue3Glasshole02CB2[0].wz);
    r1.x = g_Ue3Glasshole02SceneDepth.SampleLevel(
        g_Ue3Glasshole02SceneDepthSampler, r1.xy, 0.0f).x;
    r1.x = min(r1.x, 0.999f);
    r1.y = mad(
        r1.x,
        g_Ue3Glasshole02CB2[1].z,
        -g_Ue3Glasshole02CB2[1].w);
    r1.x = mad(
        r1.x,
        g_Ue3Glasshole02CB2[1].x,
        g_Ue3Glasshole02CB2[1].y);
    r1.y = 1.0f / r1.y;
    r1.x = r1.y + r1.x;
    r1.x = r1.x - input.screenPosition.w;
    r1.xy = saturate(
        r1.xx * float2(0.034482759f, 0.066666667f));
    r1.y = -r1.y + 1.0f;
    r1.z = r1.y * r1.y;
    const bool intersectionEnvelopeIsZero = r1.y < 0.000001f;
    r1.z = r1.z * r1.z;
    r1.z = r1.z * r1.z;
    r3.xyz = r1.zzz * float3(10.0f, 10.0f, 20.0f);
    r1.yzw = intersectionEnvelopeIsZero ? 0.0f : r3.xyz;
    r0.yzw = mad(r2.yyy, r0.yzw, r1.yzw);
    r0.yzw = r0.yzw * input.particleColor.xyz;
    r1.y = -r2.y + 1.0f;
    r1.z = saturate(r1.y + r2.z);
    r1.x = r1.x * r1.z;
    r1.x = saturate(r1.x * input.particleColor.w);

    Ue3Glasshole02PixelOutput output = (Ue3Glasshole02PixelOutput)0;
    output.sceneColor.w = r1.x * g_Ue3Glasshole02CB0[0].x;

    // 141-170: inner-hole UV deformation and final fog/selection composition.
    r1.x = log2(r0.x);
    const bool radialEnvelopeIsZero = r0.x < 0.000001f;
    r1.x = r1.x * g_Ue3Glasshole02CB0[14].x;
    r1.x = exp2(r1.x);
    r1.x = r1.x * g_Ue3Glasshole02CB0[14].z;
    r1.z = -input.particleUv.x + 0.5f;
    r1.w = -input.particleUv.y + 0.5f;
    r1.xz = r1.zw * r1.xx;
    r1.xz = radialEnvelopeIsZero ? 0.0f : r1.xz;
    r1.xz = r1.xz + input.particleUv.xy;
    r1.xz = mad(
        g_Ue3Glasshole02CB0[14].ww,
        r1.xz,
        g_Ue3Glasshole02CB0[2].xy);
    r1.xz = r1.xz + g_Ue3Glasshole02CB0[3].xy;
    const float4 crackNormalSample = g_Ue3Glasshole02CrackNormal.SampleBias(
        g_Ue3Glasshole02CrackNormalSampler, input.particleUv.xy, 0.0f);
    r2.y = crackNormalSample.x;
    r2.z = crackNormalSample.y;
    r2.yz = mad(r2.yz, 2.0f, -1.0f);
    r2.yz = r1.yy * r2.yz;
    r1.xz = mad(g_Ue3Glasshole02CB0[15].ww, r2.yz, r1.xz);
    r0.x = mad(g_Ue3Glasshole02CB0[16].x, 0.5f, -0.25f);
    r1.x = mad(r0.x, r2.x, r1.x);
    r1.z = mad(r0.x, r2.w, r1.z);
    const float4 innerHoleSample = g_Ue3Glasshole02InnerHole.SampleBias(
        g_Ue3Glasshole02InnerHoleSampler, r1.xz, 0.0f);
    r1.x = innerHoleSample.x;
    r1.z = innerHoleSample.y;
    r1.w = innerHoleSample.z;
    r1.xzw = max(
        abs(r1.xzw),
        float3(0.000001f, 0.000001f, 0.000001f));
    r1.xzw = log2(r1.xzw);
    r1.xzw = r1.xzw * g_Ue3Glasshole02CB0[16].y;
    r1.xzw = exp2(r1.xzw);
    r2.xyz = r1.xzw * g_Ue3Glasshole02CB0[16].z;
    r0.x = dot(r2.xyz, float3(0.3f, 0.59f, 0.11f));
    r1.xzw = mad(-g_Ue3Glasshole02CB0[16].zzz, r1.xzw, r0.xxx);
    r1.xzw = mad(g_Ue3Glasshole02CB0[16].www, r1.xzw, r2.xyz);
    r1.xzw = r1.xzw * g_Ue3Glasshole02CB0[9].xyz;
    r0.xyz = mad(r1.yyy, r1.xzw, r0.yzw);
    r0.xyz = r0.xyz + g_Ue3Glasshole02CB0[1].xyz;
    output.sceneColor.xyz = mad(
        r0.xyz,
        input.fogAndSelection.www,
        input.fogAndSelection.xyz);

    // 171-197: UE3 GBuffer normal encoding and the remaining BasePass MRTs.
    r0.x = dot(input.tangentToWorld1.xyz, input.tangentToWorld1.xyz);
    r0.x = rsqrt(r0.x);
    r0.xyz = r0.xxx * input.tangentToWorld1.xyz;
    r0.w = dot(input.tangentToWorld0.xyz, input.tangentToWorld0.xyz);
    r0.w = rsqrt(r0.w);
    r1.x = r0.w * input.tangentToWorld0.z;
    r1.y = r0.w * input.tangentToWorld0.x;
    r1.z = r0.w * input.tangentToWorld0.y;
    r0.y = r0.y * r1.y;
    r0.x = mad(r0.x, r1.z, -r0.y);
    r1.z = r0.z;
    r1.y = r0.x * input.tangentToWorld1.w;
    r0.x = dot(r1.xyz, r1.xyz);
    r0.x = rsqrt(r0.x);
    r0.xyz = r0.xxx * r1.xyz;
    r0.w = dot(float3(1.0f, 1.0f, 1.0f), abs(r0.xyz));
    r0.xy = r0.xy / r0.ww;
    const bool encodedNormalUsesLowerHemisphere = 0.0f >= r0.z;
    const bool2 encodedNormalIsNonNegative = r0.xy >= 0.0f;
    r1.xy = encodedNormalIsNonNegative
        ? float2(1.0f, 1.0f)
        : float2(-1.0f, -1.0f);
    r1.xy = mad(-abs(r0.yx), r1.xy, r1.xy);
    r0.xy = encodedNormalUsesLowerHemisphere ? r1.xy : r0.xy;
    output.encodedNormal.xy = mad(
        r0.xy,
        float2(0.5f, 0.5f),
        float2(0.5f, 0.5f));
    output.encodedNormal.zw = float2(1.0f, 0.0f);
    output.sceneMetadata.xyz = g_Ue3Glasshole02CB2[3].xyz;
    output.sceneMetadata.w = 0.0f;
    output.unusedTarget4 = 0.0f;
    output.unusedTarget5 = 0.0f;
    return output;
}

#endif
