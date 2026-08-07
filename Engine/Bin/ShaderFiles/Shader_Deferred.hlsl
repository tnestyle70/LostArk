
#include "Engine_Shader_Defines.hlsli"

float4x4    g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
float4x4    g_ViewMatrixInverse, g_ProjMatrixInverse;
float4x4    g_LightViewMatrix, g_LightProjMatrix;
texture2D   g_Texture;
texture2D   g_DiffuseTexture, g_ShadeTexture;
texture2D   g_DepthTexture;
texture2D   g_SpecularTexture;
texture2D   g_EmissiveTexture;
texture2D   g_LightDepthTexture;
texture2D   g_SceneHDRTexture;
texture2D   g_PostProcessTexture;
texture2D   g_BloomTexture;
texture2D   g_DistortionTexture;

float2      g_vBloomTexelSize;
float2      g_vInverseSceneSize;
uint        g_iBloomEnabled;
float       g_fBloomThreshold;
float       g_fBloomSoftKnee;
float       g_fBloomIntensity;
float       g_fBloomScatter;
float       g_fToneMapExposure;
float       g_fToneMapWhitePoint;
float       g_fToneMapGamma;
uint        g_iFXAAEnabled;
float       g_fFXAASubpixel;
float       g_fFXAAEdgeThreshold;
float       g_fFXAAEdgeThresholdMin;
float       g_fPresentationTime;
float       g_fPresentationIntensity;
float       g_fPresentationSecondaryIntensity;
float       g_fPresentationFrequency;
uint        g_iPresentationSeed;
float4      g_vPresentationTint;

/* Post effects must not wrap energy from one screen edge to the opposite edge. */
sampler PostProcessSampler = sampler_state
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
    AddressW = CLAMP;
};

vector      g_vCamPosition;



vector      g_vLightDir;
vector      g_vLightPos;
float       g_fLightRange;
vector      g_vLightDiffuse;
vector      g_vLightAmbient;
vector      g_vLightSpecular;

vector      g_vMtrlAmbient = 1.f;

texture2D   g_NormalTexture;

struct VS_IN
{
    float3 vPosition : POSITION;
    float2 vTexcoord : TEXCOORD0;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;    
    float2 vTexcoord : TEXCOORD0;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;
    
    
    matrix      matWV, matWVP;
    
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix); 
    
    Out.vPosition = mul(vector(In.vPosition, 1.f), matWVP);
    Out.vTexcoord = In.vTexcoord;
  
    return Out;
}



struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
};

struct PS_OUT_BACKBUFFER
{
    float4 vBackBuffer : SV_TARGET0;
};

struct PS_OUT_LIGHT
{
    float4 vShade : SV_TARGET0;
    float4 vSpecular : SV_TARGET1;
};

/* 픽셀 셰이더 */
/* 전달받은 픽셀의 정보를 바탕으로하여 픽셀의 색을 결정한다 */

PS_OUT_BACKBUFFER PS_MAIN_DEBUG(PS_IN In)
{
    PS_OUT_BACKBUFFER Out;
    
    Out.vBackBuffer = g_Texture.Sample(LinearSampler, In.vTexcoord);
    
    return Out;    
}

PS_OUT_LIGHT PS_MAIN_DIRECTIONAL(PS_IN In)
{
    PS_OUT_LIGHT Out;
    
    vector          vNormalDesc = g_NormalTexture.Sample(LinearSampler, In.vTexcoord);
    
    /* 0 ~ 1 => -1 ~ 1 */
    vector vNormal = vector(vNormalDesc.xyz * 2.f - 1.f, 0.f);    
    

    
    Out.vShade = g_vLightDiffuse * (saturate(dot(normalize(g_vLightDir) * -1.f, normalize(vNormal)))
    + (g_vLightAmbient * g_vMtrlAmbient));
    
    
    vector vReflect = reflect(g_vLightDir, vNormal);
    
    
    vector      vDepthDesc = g_DepthTexture.Sample(LinearSampler, In.vTexcoord);
    float fViewZ = vDepthDesc.y * 1000.f;
    
    vector vWorldPos;
    
    /* 로컬위치 * 월드행렬 * 뷰행렬 * 투영행렬 / w -> 투영공간상의 위치부터 구한다. */
    vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
    vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
    vWorldPos.z = vDepthDesc.x;
    vWorldPos.w = 1.f;
    
    /* 로컬위치 * 월드행렬 * 뷰행렬 * 투영행렬 */
    vWorldPos = vWorldPos * fViewZ;
    
    /* 로컬위치 * 월드행렬 * 뷰행렬 */
    vWorldPos = mul(vWorldPos, g_ProjMatrixInverse);
    
    /* 로컬위치 * 월드행렬  */
    vWorldPos = mul(vWorldPos, g_ViewMatrixInverse);
    
    vector vLook = vWorldPos - g_vCamPosition;
    
    const float specularPower = vDepthDesc.z > 0.f ? vDepthDesc.z : 50.f;
    Out.vSpecular = g_vLightSpecular *
        pow(saturate(dot(normalize(vReflect) * -1.f, normalize(vLook))),
            specularPower) * vNormalDesc.a;
    
    return Out;
}

PS_OUT_LIGHT PS_MAIN_POINT(PS_IN In)
{
    PS_OUT_LIGHT Out;
    
    vector vNormalDesc = g_NormalTexture.Sample(LinearSampler, In.vTexcoord);
    
    /* 0 ~ 1 => -1 ~ 1 */
    vector vNormal = vector(vNormalDesc.xyz * 2.f - 1.f, 0.f);
    
    
    vector vDepthDesc = g_DepthTexture.Sample(LinearSampler, In.vTexcoord);
    float fViewZ = vDepthDesc.y * 1000.f;
    
    vector vWorldPos;
    
    /* 로컬위치 * 월드행렬 * 뷰행렬 * 투영행렬 / w -> 투영공간상의 위치부터 구한다. */
    vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
    vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
    vWorldPos.z = vDepthDesc.x;
    vWorldPos.w = 1.f;
    
    /* 로컬위치 * 월드행렬 * 뷰행렬 * 투영행렬 */
    vWorldPos = vWorldPos * fViewZ;
    
    /* 로컬위치 * 월드행렬 * 뷰행렬 */
    vWorldPos = mul(vWorldPos, g_ProjMatrixInverse);
    
    /* 로컬위치 * 월드행렬  */
    vWorldPos = mul(vWorldPos, g_ViewMatrixInverse);
    
    
    vector vLightDir = vWorldPos - g_vLightPos;
    
    float fAtt = saturate((g_fLightRange - length(vLightDir)) / g_fLightRange);
    
    Out.vShade = (g_vLightDiffuse * (saturate(dot(normalize(vLightDir) * -1.f, normalize(vNormal)))
    + (g_vLightAmbient * g_vMtrlAmbient))) * fAtt;
    
    
    vector vReflect = reflect(vLightDir, vNormal);
    
    
    vector vLook = vWorldPos - g_vCamPosition;
    
    const float specularPower = vDepthDesc.z > 0.f ? vDepthDesc.z : 50.f;
    Out.vSpecular = g_vLightSpecular *
        pow(saturate(dot(normalize(vReflect) * -1.f, normalize(vLook))),
            specularPower) * vNormalDesc.a * fAtt;
    
    return Out;
}



PS_OUT_BACKBUFFER PS_MAIN_COMBINED(PS_IN In)
{
    PS_OUT_BACKBUFFER Out;
    
    vector vDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    if (0.f == vDiffuse.a)
        discard;
    
    vector vShade = g_ShadeTexture.Sample(LinearSampler, In.vTexcoord);
    
    vector vSpecular = g_SpecularTexture.Sample(LinearSampler, In.vTexcoord);
    vector vEmissive = g_EmissiveTexture.Sample(LinearSampler, In.vTexcoord);
    
    
    vector vLitColor = vDiffuse * vShade + vSpecular;
    
    vector vDepthDesc = g_DepthTexture.Sample(LinearSampler, In.vTexcoord);
    float fViewZ = vDepthDesc.y * 1000.f;
    
    vector vWorldPos;
    
    /* 로컬위치 * 월드행렬 * 뷰행렬 * 투영행렬 / w -> 투영공간상의 위치부터 구한다. */
    vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
    vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
    vWorldPos.z = vDepthDesc.x;
    vWorldPos.w = 1.f;
    
    /* 로컬위치 * 월드행렬 * 뷰행렬 * 투영행렬 */
    vWorldPos = vWorldPos * fViewZ;
    
    /* 로컬위치 * 월드행렬 * 뷰행렬 */
    vWorldPos = mul(vWorldPos, g_ProjMatrixInverse);
    
    /* 로컬위치 * 월드행렬  */
    vWorldPos = mul(vWorldPos, g_ViewMatrixInverse);
    
    vWorldPos = mul(vWorldPos, g_LightViewMatrix);
    vWorldPos = mul(vWorldPos, g_LightProjMatrix);
    
    float2 vTexcoord;
    
    /* -1, 1 ~ 1, -1 */ 
    /*  0, 0 ~ 1,  1 */
    
    vTexcoord.x = (vWorldPos.x / vWorldPos.w) * 0.5f + 0.5f;
    vTexcoord.y = (vWorldPos.y / vWorldPos.w) * -0.5f + 0.5f;
    
    float4      vOldZ = g_LightDepthTexture.Sample(LinearSampler, vTexcoord);
       
    
    // if (현재 그리는 픽셀의 광원기준 깊이가 >= 이전에 광원기준으로 그려져있던 픽셀의 깊이보다. )
    if (vWorldPos.w - 0.1f >= vOldZ.x * 1000.f)
    {
        vLitColor *= 0.3f;

    }

    /* Emissive and Effect HDR energy are light sources, not receivers.  They
       must remain available to bloom even when the carrier is in shadow. */
    Out.vBackBuffer = vLitColor + vEmissive;
    return Out;
}


float3 Sanitize_HDR(float3 vColor)
{
    // FP16 SceneHDR can contain INF after heavy additive overlap.  Keep the
    // post chain finite before division in bright-pass and Hable tone mapping.
    return min(max(vColor, 0.f), 60000.f);
}

float3 Extract_Bloom(float3 vColor)
{
    float fThreshold = max(g_fBloomThreshold, 0.f);
    float fSoftKnee = max(g_fBloomSoftKnee, 0.f);

    float fBrightness = max(vColor.r, max(vColor.g, vColor.b));
    float fSoft = clamp(
        fBrightness - fThreshold + fSoftKnee,
        0.f, 2.f * fSoftKnee);
    fSoft = fSoft * fSoft / (4.f * fSoftKnee + 0.00001f);

    float fContribution = max(fBrightness - fThreshold, fSoft) /
        max(fBrightness, 0.00001f);
    return vColor * fContribution;
}

PS_OUT_BACKBUFFER PS_MAIN_BLOOM_EXTRACT(PS_IN In)
{
    PS_OUT_BACKBUFFER Out;
    float3 vScene = Sanitize_HDR(g_PostProcessTexture.Sample(
        PostProcessSampler, In.vTexcoord).rgb);
    Out.vBackBuffer = float4(Extract_Bloom(vScene), 1.f);
    return Out;
}

float3 Blur_Bloom(float2 vTexcoord, float2 vDirection)
{
    float2 vStep = g_vBloomTexelSize * vDirection *
        max(g_fBloomScatter, 0.25f);
    float3 vResult = g_PostProcessTexture.Sample(
        PostProcessSampler, vTexcoord).rgb * 0.2270270270f;

    vResult += g_PostProcessTexture.Sample(
        PostProcessSampler, vTexcoord + vStep).rgb * 0.1945945946f;
    vResult += g_PostProcessTexture.Sample(
        PostProcessSampler, vTexcoord - vStep).rgb * 0.1945945946f;
    vResult += g_PostProcessTexture.Sample(
        PostProcessSampler, vTexcoord + vStep * 2.f).rgb * 0.1216216216f;
    vResult += g_PostProcessTexture.Sample(
        PostProcessSampler, vTexcoord - vStep * 2.f).rgb * 0.1216216216f;
    vResult += g_PostProcessTexture.Sample(
        PostProcessSampler, vTexcoord + vStep * 3.f).rgb * 0.0540540541f;
    vResult += g_PostProcessTexture.Sample(
        PostProcessSampler, vTexcoord - vStep * 3.f).rgb * 0.0540540541f;
    vResult += g_PostProcessTexture.Sample(
        PostProcessSampler, vTexcoord + vStep * 4.f).rgb * 0.0162162162f;
    vResult += g_PostProcessTexture.Sample(
        PostProcessSampler, vTexcoord - vStep * 4.f).rgb * 0.0162162162f;
    return vResult;
}

PS_OUT_BACKBUFFER PS_MAIN_BLOOM_BLUR_H(PS_IN In)
{
    PS_OUT_BACKBUFFER Out;
    Out.vBackBuffer = float4(
        Blur_Bloom(In.vTexcoord, float2(1.f, 0.f)), 1.f);
    return Out;
}

PS_OUT_BACKBUFFER PS_MAIN_BLOOM_BLUR_V(PS_IN In)
{
    PS_OUT_BACKBUFFER Out;
    Out.vBackBuffer = float4(
        Blur_Bloom(In.vTexcoord, float2(0.f, 1.f)), 1.f);
    return Out;
}

float Presentation_Hash(float2 vPosition, uint iSeed)
{
    float fSeed = (float)(iSeed & 0x00ffffffu);
    return frac(sin(dot(vPosition + fSeed,
        float2(12.9898f, 78.233f))) * 43758.5453f);
}

PS_OUT_BACKBUFFER PS_MAIN_SCENE_RESOLVE(PS_IN In)
{
    PS_OUT_BACKBUFFER Out;
    float2 vDistortion = clamp(g_DistortionTexture.Sample(
        PostProcessSampler, In.vTexcoord).rg, -0.05f, 0.05f);
    float3 vScene = Sanitize_HDR(g_SceneHDRTexture.Sample(
        PostProcessSampler, In.vTexcoord + vDistortion).rgb);
    Out.vBackBuffer = float4(vScene, 1.f);
    return Out;
}

PS_OUT_BACKBUFFER PS_MAIN_RGB_NOISE(PS_IN In)
{
    PS_OUT_BACKBUFFER Out;
    float fTick = floor(g_fPresentationTime *
        max(g_fPresentationFrequency, 0.0001f) * 60.f);
    float fNoise = Presentation_Hash(
        floor(In.vTexcoord * float2(320.f, 180.f)) + fTick,
        g_iPresentationSeed) - 0.5f;
    float fOffset = clamp(g_fPresentationIntensity, 0.f, 8.f) *
        fNoise * 0.015f;
    float3 vColor;
    vColor.r = g_PostProcessTexture.Sample(PostProcessSampler,
        clamp(In.vTexcoord + float2(fOffset, 0.f), 0.f, 1.f)).r;
    vColor.g = g_PostProcessTexture.Sample(PostProcessSampler,
        In.vTexcoord).g;
    vColor.b = g_PostProcessTexture.Sample(PostProcessSampler,
        clamp(In.vTexcoord - float2(fOffset, 0.f), 0.f, 1.f)).b;
    vColor += fNoise * clamp(g_fPresentationSecondaryIntensity,
        0.f, 8.f) * g_vPresentationTint.rgb;
    Out.vBackBuffer = float4(Sanitize_HDR(vColor), 1.f);
    return Out;
}

PS_OUT_BACKBUFFER PS_MAIN_ZOOM_BLUR(PS_IN In)
{
    PS_OUT_BACKBUFFER Out;
    float2 vFromCenter = In.vTexcoord - float2(0.5f, 0.5f);
    float fStrength = clamp(g_fPresentationIntensity, 0.f, 8.f) * 0.025f;
    float3 vColor = 0.f;
    [unroll]
    for (int iSample = 0; iSample < 8; ++iSample)
    {
        float fSample = (float)iSample / 7.f;
        float2 vUV = clamp(In.vTexcoord - vFromCenter *
            fStrength * fSample, 0.f, 1.f);
        vColor += g_PostProcessTexture.Sample(
            PostProcessSampler, vUV).rgb;
    }
    vColor *= 0.125f;
    float3 vSource = g_PostProcessTexture.Sample(
        PostProcessSampler, In.vTexcoord).rgb;
    float fMix = saturate(g_fPresentationSecondaryIntensity > 0.f ?
        g_fPresentationSecondaryIntensity : g_fPresentationIntensity);
    Out.vBackBuffer = float4(Sanitize_HDR(
        lerp(vSource, vColor * g_vPresentationTint.rgb, fMix)), 1.f);
    return Out;
}

PS_OUT_BACKBUFFER PS_MAIN_FILM_NOISE(PS_IN In)
{
    PS_OUT_BACKBUFFER Out;
    float fTick = floor(g_fPresentationTime *
        max(g_fPresentationFrequency, 0.0001f) * 60.f);
    float fNoise = Presentation_Hash(
        floor(In.vTexcoord * float2(1024.f, 1024.f)) + fTick,
        g_iPresentationSeed) * 2.f - 1.f;
    float fScan = sin((In.vTexcoord.y + g_fPresentationTime) *
        max(g_fPresentationFrequency, 1.f) * 256.f);
    float3 vSource = g_PostProcessTexture.Sample(
        PostProcessSampler, In.vTexcoord).rgb;
    float3 vNoise = (fNoise * clamp(g_fPresentationIntensity, 0.f, 8.f) +
        fScan * clamp(g_fPresentationSecondaryIntensity, 0.f, 8.f)) *
        g_vPresentationTint.rgb;
    Out.vBackBuffer = float4(Sanitize_HDR(vSource + vNoise), 1.f);
    return Out;
}

/* Hable(Uncharted 2) 필름 커브. 씬 컬러는 선형이고 1을 넘을 수 있다. */
float3 Tonemap_Hable(float3 vColor)
{
    const float A = 0.15f;
    const float B = 0.50f;
    const float C = 0.10f;
    const float D = 0.20f;
    const float E = 0.02f;
    const float F = 0.30f;

    return ((vColor * (A * vColor + C * B) + D * E) /
            (vColor * (A * vColor + B) + D * F)) - E / F;
}

float3 Resolve_FinalLDR(float2 vTexcoord)
{
    float3 vScene = Sanitize_HDR(g_SceneHDRTexture.Sample(
        PostProcessSampler, clamp(vTexcoord, 0.f, 1.f)).rgb);
    float3 vBloom = float3(0.f, 0.f, 0.f);
    if (0u != g_iBloomEnabled)
    {
        vBloom = Sanitize_HDR(g_BloomTexture.Sample(
            PostProcessSampler, clamp(vTexcoord, 0.f, 1.f)).rgb);
    }

    float3 vSceneWithBloom = vScene + vBloom * g_fBloomIntensity;
    float3 vWhiteScale = max(Tonemap_Hable(
        max(g_fToneMapWhitePoint, 1.f).xxx), 0.00001f);
    float3 vMapped = Tonemap_Hable(vSceneWithBloom *
        max(g_fToneMapExposure, 0.01f)) / vWhiteScale;
    return pow(saturate(vMapped),
        1.f / max(g_fToneMapGamma, 1.f));
}

float Final_Luminance(float3 vColor)
{
    return dot(vColor, float3(0.299f, 0.587f, 0.114f));
}

float3 Resolve_FinalFXAA(float2 vTexcoord)
{
    float3 vCenter = float3(0.f, 0.f, 0.f);
    vCenter = Resolve_FinalLDR(vTexcoord);
    if (0u == g_iFXAAEnabled || g_fFXAASubpixel <= 0.f)
        return vCenter;

    float2 vTexel = max(g_vInverseSceneSize, 0.000001f);
    float3 vNW = Resolve_FinalLDR(vTexcoord + vTexel * float2(-1.f, -1.f));
    float3 vNE = Resolve_FinalLDR(vTexcoord + vTexel * float2( 1.f, -1.f));
    float3 vSW = Resolve_FinalLDR(vTexcoord + vTexel * float2(-1.f,  1.f));
    float3 vSE = Resolve_FinalLDR(vTexcoord + vTexel * float2( 1.f,  1.f));

    float fLumaM = Final_Luminance(vCenter);
    float fLumaNW = Final_Luminance(vNW);
    float fLumaNE = Final_Luminance(vNE);
    float fLumaSW = Final_Luminance(vSW);
    float fLumaSE = Final_Luminance(vSE);
    float fLumaMin = min(fLumaM,
        min(min(fLumaNW, fLumaNE), min(fLumaSW, fLumaSE)));
    float fLumaMax = max(fLumaM,
        max(max(fLumaNW, fLumaNE), max(fLumaSW, fLumaSE)));
    float fLumaRange = fLumaMax - fLumaMin;
    float fRequiredRange = max(g_fFXAAEdgeThresholdMin,
        fLumaMax * g_fFXAAEdgeThreshold);
    if (fLumaRange < fRequiredRange)
        return vCenter;

    float2 vDirection;
    vDirection.x = -((fLumaNW + fLumaNE) - (fLumaSW + fLumaSE));
    vDirection.y =  ((fLumaNW + fLumaSW) - (fLumaNE + fLumaSE));
    float fDirectionReduce = max(
        (fLumaNW + fLumaNE + fLumaSW + fLumaSE) * 0.03125f,
        0.0078125f);
    float fReciprocalDirection = 1.f /
        (min(abs(vDirection.x), abs(vDirection.y)) + fDirectionReduce);
    vDirection = clamp(vDirection * fReciprocalDirection,
        -8.f, 8.f) * vTexel;

    float3 vBlendA = 0.5f * (
        Resolve_FinalLDR(vTexcoord + vDirection * (-1.f / 6.f)) +
        Resolve_FinalLDR(vTexcoord + vDirection * ( 1.f / 6.f)));
    float3 vBlendB = vBlendA * 0.5f + 0.25f * (
        Resolve_FinalLDR(vTexcoord + vDirection * -0.5f) +
        Resolve_FinalLDR(vTexcoord + vDirection *  0.5f));
    float fLumaB = Final_Luminance(vBlendB);
    float3 vEdgeColor =
        (fLumaB < fLumaMin || fLumaB > fLumaMax) ? vBlendA : vBlendB;
    return lerp(vCenter, vEdgeColor, saturate(g_fFXAASubpixel));
}

/* SceneHDR을 백버퍼로 옮기는 유일한 지점. 톤매핑, 감마, FXAA는 UI 전에 여기서 처리한다. */
PS_OUT_BACKBUFFER PS_MAIN_FINAL(PS_IN In)
{
    PS_OUT_BACKBUFFER Out;
    Out.vBackBuffer = float4(Resolve_FinalFXAA(In.vTexcoord), 1.f);

    return Out;
}


technique11 DefaultTechnique
{
    pass Debug
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();        
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DEBUG();
    }   

    pass Directional
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Blend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DIRECTIONAL();
    }

    pass Point
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Blend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_POINT();
    }

    pass Combined
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_COMBINED();
    }

    /* DEFERRED::FINAL == 4. 반드시 Combined 다음 순서를 유지할 것. */
    pass Final
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_FINAL();
    }

    /* FINAL stays at index 4; bloom passes are invoked before it by CRenderer. */
    pass BloomExtract
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_BLOOM_EXTRACT();
    }

    pass BloomBlurHorizontal
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_BLOOM_BLUR_H();
    }

    pass BloomBlurVertical
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_BLOOM_BLUR_V();
    }

    pass SceneResolve
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_SCENE_RESOLVE();
    }

    pass PresentationRGBNoise
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_RGB_NOISE();
    }

    pass PresentationZoomBlur
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_ZOOM_BLUR();
    }

    pass PresentationFilmNoise
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_FILM_NOISE();
    }

}



