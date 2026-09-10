#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
Texture2D g_DiffuseTexture;
Texture2D g_NormalTexture;
Texture2D g_SpecularTexture;
Texture2D g_EmissiveTexture;
uint g_HasNormalTexture = 0;
uint g_HasSpecularTexture = 0;
uint g_HasEmissiveTexture = 0;
float g_SpecularIntensity = 1.f;
float g_SpecularPower = 50.f;
float4 g_EmissiveColor = 1.f;
float g_EmissiveIntensity = 1.f;
uint g_HasFullSurfaceEmissiveOverride = 0;
float4 g_FullSurfaceEmissiveColor = 1.f;
float g_FullSurfaceEmissiveIntensity = 0.f;
/* 0: diffuse luminance weights the whole surface (skill glow).
   1: normal-map bump strength times specular weights only creases and
   metal, so a hit flash reads the shape instead of washing it out. */
uint g_FullSurfaceEmissiveMaskMode = 0;
/* The source game's dye contract: the mask's channels select colour regions
of a mostly achromatic diffuse and each region multiplies its tint in. */
Texture2D g_DyeMaskTexture;
uint g_HasDyeMask = 0;
uint g_DyeIsHair = 0;
/* Skin and eyes are tinted without a region mask in the source game, so this multiplies the
sampled diffuse directly. Identity unless the creation screen chose a colour. */
float4 g_DiffuseTint = 1.f;
float4 g_DyeDiffuseColor = 1.f;
float4 g_DyeRegionA = 1.f;
float4 g_DyeRegionB = 1.f;
float4 g_DyeRegionC = 1.f;
float4 g_EffectModelCueColorMultiply = 1.f;
float g_EffectModelCueOpacity = 1.f;
matrix g_BoneMatrices[512];

/* Model textures carry full mip chains. Anisotropic sampling also preserves
   detail on slanted body and equipment surfaces at gameplay distance. */
sampler MaterialAnisotropicSampler = sampler_state
{
    Filter = ANISOTROPIC;
    MaxAnisotropy = 16;
    AddressU = WRAP;
    AddressV = WRAP;
    AddressW = WRAP;
};

#include "Shader_SourceCharacterMaterial.hlsli"

// Optional source skeletal material shares this shader's existing skinned VS.
uint g_ArtistModelCueProfile = 0u;
float4 g_ArtistModelAmbient = 0.f;
uint g_SourceTextureClampUMask = 0u;
uint g_SourceTextureClampVMask = 0u;
Texture2D g_SourceTexture0;
Texture2D g_SourceTexture1;
Texture2D g_SourceTexture2;
Texture2D g_SourceTexture3;
Texture2D g_SourceTexture4;
Texture2D g_SourceTexture5;
Texture2D g_SourceTexture6;
Texture2D g_SourceTexture7;
Texture2D g_SourceTexture8;

SamplerState LinearClampUSampler { Filter=MIN_MAG_MIP_LINEAR; AddressU=Clamp; AddressV=Wrap; };
SamplerState LinearClampVSampler { Filter=MIN_MAG_MIP_LINEAR; AddressU=Wrap; AddressV=Clamp; };
SamplerState LinearClampUVSampler { Filter=MIN_MAG_MIP_LINEAR; AddressU=Clamp; AddressV=Clamp; };
#define ARTIST_NATIVE_MODEL_ONLY
#include "Shader_EffectArtistNative.hlsli"
#undef ARTIST_NATIVE_MODEL_ONLY
// Native skeletal T material uses the same committed Target_Depth adapter.
Texture2D g_EffectSceneDepthTexture;
SamplerState EffectSliceDepthSampler
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Clamp;
    AddressV = Clamp;
};
#define LANCE_VA_NATIVE_MODEL_ONLY
#include "Shader_EffectLanceMasterVANative.hlsli"
#undef LANCE_VA_NATIVE_MODEL_ONLY

struct VS_IN
{
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float3 vTangent : TANGENT;
    float3 vBinormal : BINORMAL;
    float2 vTexcoord : TEXCOORD0;
    float2 vTexcoord1 : TEXCOORD1;
    float2 vTexcoord2 : TEXCOORD2;
    uint4 vBlendIndices : BLENDINDEX;
    float4 vBlendWeights : BLENDWEIGHT;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float4 vTangent : TANGENT;
    float4 vBinormal : BINORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
    float4 vProjPos : TEXCOORD2;
    float4 vSourceExtraUV : TEXCOORD3;
};

VS_OUT VS_MAIN(VS_IN input)
{
    VS_OUT output;

    matrix boneMatrix =
        g_BoneMatrices[input.vBlendIndices.x] * input.vBlendWeights.x +
        g_BoneMatrices[input.vBlendIndices.y] * input.vBlendWeights.y +
        g_BoneMatrices[input.vBlendIndices.z] * input.vBlendWeights.z +
        g_BoneMatrices[input.vBlendIndices.w] * input.vBlendWeights.w;

    float4 position = mul(float4(input.vPosition, 1.f), boneMatrix);
    float4 normal = mul(float4(input.vNormal, 0.f), boneMatrix);
    float4 tangent = mul(float4(input.vTangent, 0.f), boneMatrix);
    float4 binormal = mul(float4(input.vBinormal, 0.f), boneMatrix);

    matrix worldView = mul(g_WorldMatrix, g_ViewMatrix);
    matrix worldViewProjection = mul(worldView, g_ProjMatrix);

    output.vPosition = mul(position, worldViewProjection);
    output.vNormal = normalize(mul(normal, g_WorldMatrix));
    output.vTangent = normalize(mul(tangent, g_WorldMatrix));
    output.vBinormal = normalize(mul(binormal, g_WorldMatrix));
    output.vTexcoord = input.vTexcoord;
    output.vWorldPos = mul(position, g_WorldMatrix);
    output.vProjPos = output.vPosition;
    output.vSourceExtraUV = float4(input.vTexcoord1, input.vTexcoord2);
    return output;
}

struct PS_OUT
{
    float4 vDiffuse : SV_TARGET0;
    float4 vNormal : SV_TARGET1;
    float4 vDepth : SV_TARGET2;
    float4 vPickPos : SV_TARGET3;
    float4 vEmissive : SV_TARGET4;
    float4 vMaterialSpecular : SV_TARGET5;
    float4 vCharacterSurface : SV_TARGET6;
    float4 vCharacterGeometry : SV_TARGET7;
};

PS_OUT Evaluate_Material(
    VS_OUT input, bool alphaClip, float alphaClipThreshold, bool frontFace)
{
    PS_OUT output = (PS_OUT)0;
    if (g_SourceCharacterProgram != 0u)
    {
        SOURCE_CHARACTER_GBUFFER source = EvaluateSourceCharacterGeometry(input.vTexcoord,
            input.vSourceExtraUV, input.vWorldPos.xyz, input.vTangent.xyz, input.vBinormal.xyz,
            input.vNormal.xyz, input.vProjPos, input.vPosition, g_ViewMatrix, g_ProjMatrix, frontFace);
        output.vDiffuse = source.diffuse;
        output.vNormal = source.normal;
        output.vDepth = source.depth;
        output.vPickPos = source.pickPosition;
        output.vEmissive = source.indirect;
        output.vMaterialSpecular = source.extraUV;
        output.vCharacterSurface = source.surfaceUVTangent;
        output.vCharacterGeometry = source.geometricNormal;
        // Existing hit/skill presentation stays independent of material light.
        if (g_HasFullSurfaceEmissiveOverride != 0u)
        {
            float3 camera = -mul((float3x3)g_ViewMatrix, g_ViewMatrix[3].xyz);
            float weight = g_FullSurfaceEmissiveMaskMode == 1u ?
                pow(1.f - saturate(dot(normalize(input.vNormal.xyz),
                    normalize(camera - input.vWorldPos.xyz))), 3.f) : 1.f;
            output.vEmissive.rgb += g_FullSurfaceEmissiveColor.rgb *
                g_FullSurfaceEmissiveIntensity * weight;
        }
        return output;
    }
    float4 diffuse = g_DiffuseTexture.Sample(MaterialAnisotropicSampler, input.vTexcoord);
    if (alphaClip && diffuse.a < alphaClipThreshold)
        discard;
    if (!alphaClip)
        diffuse.a = 1.f;
    if (0 != g_HasDyeMask)
    {
        float3 mask = g_DyeMaskTexture.Sample(MaterialAnisotropicSampler, input.vTexcoord).rgb;
        if (0 != g_DyeIsHair)
        {
            /* A hair texture is not a colour. Its red and blue sit at a constant marker and
               only green varies, carrying the strand shading -- measured on every class'
               hair map, green's spread is 86-99 against 10-38 for red and 17-56 for blue.
               So the authored hair colour replaces the sample and green shades it. */
            /* Two-tone: the second colour reaches up the strand from its tip. The strand
               runs along v, so v is how far down it a pixel sits; range is how much of it
               the second colour claims and strength is how much of it actually blends.
               A hairstyle authored without a second colour repeats the first, so the blend
               changes nothing whatever the sliders say. */
            float fRange = max(g_DyeRegionA.a, 0.001f);
            float fAlong = saturate((input.vTexcoord.y - (1.f - fRange)) / fRange);
            float3 hair = lerp(g_DyeDiffuseColor.rgb, g_DyeRegionA.rgb,
                fAlong * saturate(g_DyeDiffuseColor.a));
            diffuse.rgb = hair * mask.g;
        }
        else
        {
            float3 tint = g_DyeDiffuseColor.rgb;
            tint *= lerp(1.f.xxx, g_DyeRegionA.rgb, mask.r);
            tint *= lerp(1.f.xxx, g_DyeRegionB.rgb, mask.g);
            tint *= lerp(1.f.xxx, g_DyeRegionC.rgb, mask.b);
            diffuse.rgb *= tint;
        }
    }

    /* Skin and eyes: identity everywhere the creation screen has not chosen a colour. */
    diffuse.rgb *= g_DiffuseTint.rgb;

    float3 normal = normalize(input.vNormal.xyz);
    if (0 != g_HasNormalTexture)
    {
        /* Monster normal maps ship as BC5 (RG only); a plain xyz decode reads
           z = -1 and flips the normal into the surface. */
        float4 encodedNormal =
            g_NormalTexture.Sample(MaterialAnisotropicSampler, input.vTexcoord);
        float3 tangentNormal;
        if (encodedNormal.b <= 0.0001f)
        {
            float2 tangentXY = encodedNormal.rg * 2.f - 1.f;
            float tangentZ = sqrt(saturate(1.f - dot(tangentXY, tangentXY)));
            tangentNormal = float3(tangentXY, tangentZ);
        }
        else
        {
            tangentNormal = normalize(encodedNormal.xyz * 2.f - 1.f);
        }
        float3x3 tangentToWorld = float3x3(
            normalize(input.vTangent.xyz),
            normalize(input.vBinormal.xyz) * -1.f,
            normal);
        normal = normalize(mul(tangentNormal, tangentToWorld));
    }

    output.vDiffuse = diffuse;
    float specularMask = g_SpecularIntensity;
    if (0 != g_HasSpecularTexture)
    {
        float3 specular = g_SpecularTexture.Sample(
            MaterialAnisotropicSampler, input.vTexcoord).rgb;
        specularMask *= dot(specular, float3(0.299f, 0.587f, 0.114f));
    }
    output.vNormal = float4(normal * 0.5f + 0.5f, specularMask);
    output.vDepth = float4(
        input.vProjPos.z / input.vProjPos.w,
        input.vProjPos.w / 1000.f,
        g_SpecularPower,
        0.f);
    output.vPickPos = input.vWorldPos;
    output.vEmissive = 0.f;
    if (0 != g_HasEmissiveTexture)
    {
        float3 emissive = g_EmissiveTexture.Sample(
            MaterialAnisotropicSampler, input.vTexcoord).rgb;
        output.vEmissive = float4(
            emissive * g_EmissiveColor.rgb * g_EmissiveIntensity, 0.f);
    }
    if (0 != g_HasFullSurfaceEmissiveOverride)
    {
        if (1 == g_FullSurfaceEmissiveMaskMode)
        {
            /* Hit flash: only the silhouette rim glows, so the body keeps its
               own colour and shading. */
            const float3 cameraPosition =
                -mul((float3x3)g_ViewMatrix, g_ViewMatrix[3].xyz);
            const float3 toCamera =
                normalize(cameraPosition - input.vWorldPos.xyz);
            const float rim = pow(1.f - saturate(dot(normal, toCamera)), 3.f);
            output.vEmissive.rgb += g_FullSurfaceEmissiveColor.rgb *
                rim * g_FullSurfaceEmissiveIntensity * diffuse.a;
        }
        else
        {
            const float textureDetail = saturate(0.35f +
                dot(diffuse.rgb, float3(0.299f, 0.587f, 0.114f)) * 0.65f);
            output.vEmissive.rgb +=
                g_FullSurfaceEmissiveColor.rgb *
                g_FullSurfaceEmissiveIntensity * textureDetail * diffuse.a;
        }
    }
    return output;
}

PS_OUT PS_MAIN(VS_OUT input, bool frontFace : SV_IsFrontFace)
{
    return Evaluate_Material(input, true, 0.3f, frontFace);
}

PS_OUT PS_MAIN_OPAQUE(VS_OUT input, bool frontFace : SV_IsFrontFace)
{
    return Evaluate_Material(input, false, 0.f, frontFace);
}

PS_OUT PS_MAIN_EFFECT_MODEL_CUE_MASKED(VS_OUT input, bool frontFace : SV_IsFrontFace)
{
    return Evaluate_Material(input, true, 0.333f, frontFace);
}

float4 PS_MAIN_EFFECT_MODEL_CUE_TRANSLUCENT(VS_OUT input) : SV_TARGET0
{
    float4 diffuse = g_DiffuseTexture.Sample(MaterialAnisotropicSampler, input.vTexcoord);
    diffuse.rgb *= g_EffectModelCueColorMultiply.rgb;
    diffuse.a = saturate(
        diffuse.a * g_EffectModelCueColorMultiply.a * g_EffectModelCueOpacity);
    if (diffuse.a <= 0.f)
        discard;
    return diffuse;
}

float4 PS_MAIN_EFFECT_MODEL_CUE_NATIVE(VS_OUT input, bool frontFace : SV_IsFrontFace) : SV_TARGET0
{
    ARTIST_NATIVE_INPUT nativeInput = (ARTIST_NATIVE_INPUT)0;
    float3 camera = -mul((float3x3)g_ViewMatrix, g_ViewMatrix[3].xyz);
    SOURCE_CHARACTER_NATIVE_INPUT basis = MakeSourceCharacterInput(input.vTexcoord,
        input.vSourceExtraUV, input.vWorldPos.xyz, input.vTangent.xyz,
        input.vBinormal.xyz, input.vNormal.xyz, camera, input.vProjPos,
        mul(g_ViewMatrix,g_ProjMatrix), float3(0.f,1.f,0.f), 0.f, 1.f, frontFace);
    nativeInput.uv = input.vTexcoord;
    nativeInput.uv1 = input.vTexcoord;
    nativeInput.uvNext = input.vTexcoord;
    nativeInput.sourceBasisX = basis.values[0].xyz;
    nativeInput.sourceBasisZ = basis.values[1].xyz;
    nativeInput.handedness = basis.values[1].w;
    float3 t = SourceCharacterSafeUnit(input.vTangent.xyz);
    float3 n = SourceCharacterSafeUnit(input.vNormal.xyz);
    t = SourceCharacterSafeUnit(t - n * dot(t,n));
    float3 b = SourceCharacterSafeUnit(-input.vBinormal.xyz);
    float3 view = SourceCharacterSafeUnit(camera-input.vWorldPos.xyz);
    nativeInput.tangentView = float3(dot(t,view),dot(b,view),dot(n,view));
    nativeInput.tangentUp = float3(t.y,b.y,n.y);
    nativeInput.sourceWorldPosition = input.vWorldPos.xzy * 100.f;
    nativeInput.sourceActorPosition = g_WorldMatrix[3].xzy * 100.f;
    nativeInput.sourceCameraPosition = camera.xzy * 100.f;
    [unroll] for(uint i=0u;i<4u;++i) nativeInput.sourceProjection[i] = basis.projection[i];
    nativeInput.projectionW = input.vProjPos.w;
    nativeInput.projectionZ = input.vProjPos.z;
    nativeInput.screenUV = input.vPosition.xy;
    nativeInput.color = 1.f;
    nativeInput.vertexColor = 1.f;
    nativeInput.frontFace = frontFace;
    nativeInput.ambientColor = g_ArtistModelAmbient.rgb;
    // Zero sky inputs reflect the current scene contract; native material math is unchanged.
    float4 color;
    if ((g_ArtistModelCueProfile >= 560u && g_ArtistModelCueProfile <= 659u) ||
        (g_ArtistModelCueProfile >= 720u && g_ArtistModelCueProfile <= 819u) ||
        g_ArtistModelCueProfile == 1360u)
    {
        LANCE_VA_NATIVE_INPUT lanceInput = (LANCE_VA_NATIVE_INPUT)0;
        lanceInput.uv = nativeInput.uv;
        lanceInput.uv1 = input.vSourceExtraUV.xy;
        lanceInput.uvNext = nativeInput.uvNext;
        lanceInput.subUVBlend = nativeInput.subUVBlend;
        lanceInput.sourceWorldPosition = nativeInput.sourceWorldPosition;
        lanceInput.sourceBasisX = nativeInput.sourceBasisX;
        lanceInput.sourceBasisZ = nativeInput.sourceBasisZ;
        lanceInput.handedness = nativeInput.handedness;
        lanceInput.vertexColor = nativeInput.vertexColor;
        lanceInput.screenUV = nativeInput.screenUV;
        lanceInput.projectionW = nativeInput.projectionW;
        lanceInput.projectionZ = nativeInput.projectionZ;
        lanceInput.tangentView = nativeInput.tangentView;
        lanceInput.color = nativeInput.color;
        lanceInput.dynamicParameter = nativeInput.dynamicParameter;
        lanceInput.frontFace = nativeInput.frontFace;
        lanceInput.tangentUp = nativeInput.tangentUp;
        lanceInput.sourceCameraPosition = nativeInput.sourceCameraPosition;
        lanceInput.sourceActorPosition = nativeInput.sourceActorPosition;
        lanceInput.skyUpperColor = nativeInput.skyUpperColor;
        lanceInput.skyLowerColor = nativeInput.skyLowerColor;
        lanceInput.ambientColor = nativeInput.ambientColor;
        lanceInput.skyIntensity = nativeInput.skyIntensity;
        [unroll] for(uint i=0u;i<4u;++i) lanceInput.sourceProjection[i] = nativeInput.sourceProjection[i];
        if (g_ArtistModelCueProfile == 1360u)
        {
            // Skeletal cues consume the authored material value, not particle state.
            lanceInput.dynamicParameter = g_LanceVASourceMaterialParameters[8u];
            // Source clip-space W and the reconstructed scene depth are centimetres.
            // Derive UV from clip coordinates; SV_POSITION is in viewport pixels.
            lanceInput.screenUV = input.vProjPos.xy / input.vProjPos.w * float2(.5f,-.5f) + .5f;
            lanceInput.projectionW *= 100.f;
            lanceInput.projectionZ *= 100.f;
        }
        color = Shade_LanceVAModelNative(g_ArtistModelCueProfile,lanceInput);
    }
    else color = Shade_ArtistModelNative(g_ArtistModelCueProfile,nativeInput);
    color.rgb *= g_EffectModelCueColorMultiply.rgb;
    color.a = saturate(color.a*g_EffectModelCueColorMultiply.a*g_EffectModelCueOpacity);
    clip(color.a - 1e-6f);
    return color;
}

void PS_MAIN_SHADOW(VS_OUT input)
{
    float4 diffuse = g_DiffuseTexture.Sample(MaterialAnisotropicSampler, input.vTexcoord);
    if (diffuse.a < 0.3f)
        discard;
}

float3 g_CutinLightDirection = float3(-0.45f, -0.75f, 0.35f);

float4 PS_MAIN_SCREEN_CUTIN(VS_OUT input) : SV_TARGET0
{
    float4 diffuse = g_DiffuseTexture.Sample(MaterialAnisotropicSampler, input.vTexcoord);
    if (diffuse.a < 0.3f)
        discard;

    float3 normal = normalize(input.vNormal.xyz);
    if (0 != g_HasNormalTexture)
    {
        float4 encodedNormal =
            g_NormalTexture.Sample(MaterialAnisotropicSampler, input.vTexcoord);
        float3 tangentNormal;
        if (encodedNormal.b <= 0.0001f)
        {
            float2 tangentXY = encodedNormal.rg * 2.f - 1.f;
            float tangentZ = sqrt(saturate(1.f - dot(tangentXY, tangentXY)));
            tangentNormal = float3(tangentXY, tangentZ);
        }
        else
        {
            tangentNormal = normalize(encodedNormal.xyz * 2.f - 1.f);
        }
        float3x3 tangentToWorld = float3x3(
            normalize(input.vTangent.xyz),
            normalize(input.vBinormal.xyz) * -1.f,
            normal);
        normal = normalize(mul(tangentNormal, tangentToWorld));
    }

    const float3 cameraPosition =
        -mul((float3x3)g_ViewMatrix, g_ViewMatrix[3].xyz);
    const float3 toCamera = normalize(cameraPosition - input.vWorldPos.xyz);
    /* Portrait studio lighting rides the camera (the character can face any world
       direction): key from the camera's upper left, softer fill from the lower right,
       plus the hemisphere/rim terms. g_CutinLightDirection stays as an extra world key. */
    const float3 cameraRight = normalize(float3(g_ViewMatrix._11, g_ViewMatrix._21, g_ViewMatrix._31));
    const float3 cameraUp = normalize(float3(g_ViewMatrix._12, g_ViewMatrix._22, g_ViewMatrix._32));
    const float3 cameraLook = normalize(float3(g_ViewMatrix._13, g_ViewMatrix._23, g_ViewMatrix._33));
    const float3 keyLight = normalize(-cameraLook + cameraUp * 0.6f - cameraRight * 0.5f);
    const float3 fillLight = normalize(-cameraLook - cameraUp * 0.15f + cameraRight * 0.7f);
    const float3 worldLight = normalize(-g_CutinLightDirection);
    const float diffuseLight =
        saturate(dot(normal, keyLight)) * 0.85f +
        saturate(dot(normal, fillLight)) * 0.35f +
        saturate(dot(normal, worldLight)) * 0.25f;
    const float hemisphere = 0.5f + saturate(normal.y) * 0.2f;
    const float rim = pow(1.f - saturate(dot(normal, toCamera)), 3.f) * 0.15f;

    float3 color = diffuse.rgb * (hemisphere + diffuseLight) + rim;
    if (0 != g_HasEmissiveTexture)
    {
        float3 emissive =
            g_EmissiveTexture.Sample(MaterialAnisotropicSampler, input.vTexcoord).rgb;
        color += emissive * g_EmissiveColor.rgb * g_EmissiveIntensity;
    }
    return float4(color, 1.f);
}

VertexShader EffectSourceModelVS = compile vs_5_0 VS_MAIN();
PixelShader EffectSourceModelPS = compile ps_5_0 PS_MAIN_EFFECT_MODEL_CUE_NATIVE();

technique11 DefaultTechnique
{
    pass DefaultPass
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }

    pass Shadow
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_SHADOW();
    }

    /* Effect ModelCue assets carry an explicit source-material alpha policy.
       OPAQUE must not reinterpret a packed diffuse alpha channel as cutout. */
    pass EffectModelCueOpaque
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_OPAQUE();
    }

    /* Exact UE3 monster_dead_msk_high_realpbr contract used only by the
       Dimension Summon ModelCue.  RS_Default preserves its one-sided source
       state while diffuse alpha owns the 0.333 opacity-mask threshold. */
    pass EffectModelCueMaskedExact
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_EFFECT_MODEL_CUE_MASKED();
    }

    /* Source translucent skeletal projectiles render after scene lighting.
       They write only SceneHDR, keep depth read-only, and preserve the source
       diffuse alpha plus the model-cue opacity/tint contract. */
    pass EffectModelCueTranslucent
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_EFFECT_MODEL_CUE_TRANSLUCENT();
    }

    pass ScreenCutin
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_SCREEN_CUTIN();
    }
    // Appended index 6 preserves all existing effect and portrait pass IDs.
    pass SourceCharacterTwoSided
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
    // Appended index 7: recovered translucent skeletal material, existing skinning input.
    pass EffectModelCueNative
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = EffectSourceModelVS;
        GeometryShader = NULL;
        PixelShader = EffectSourceModelPS;
    }
    // Appended index 8 preserves the source hair material's two-sided rasterizer.
    pass EffectModelCueNativeTwoSided
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = EffectSourceModelVS;
        GeometryShader = NULL;
        PixelShader = EffectSourceModelPS;
    }
}
