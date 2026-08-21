#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix, g_WorldInvTransposeMatrix, g_ViewMatrix, g_ProjMatrix;
Texture2D g_DiffuseTexture;
Texture2D g_NormalTexture;
Texture2D g_EmissiveTexture;
Texture2D g_SpecularTexture;
Texture2D g_OpacityTexture;
uint g_HasNormalTexture = 0;
uint g_HasEmissiveTexture = 0;
uint g_HasSpecularTexture = 0;
uint g_HasOpacityTexture = 0;
float g_EmissiveIntensity = 1.f;
float4 g_EmissiveColor = 1.f;
uint g_HasFullSurfaceEmissiveOverride = 0;
float4 g_FullSurfaceEmissiveColor = 1.f;
float g_FullSurfaceEmissiveIntensity = 0.f;
/* 0: diffuse luminance weights the whole surface (skill glow).
   1: normal-map bump strength times specular weights only creases and
   metal, so a hit flash reads the shape instead of washing it out. */
uint g_FullSurfaceEmissiveMaskMode = 0;
float g_SpecularIntensity = 1.f;
float g_SpecularPower = 50.f;
float2 g_UVScale = float2(1.f, 1.f);
float2 g_UVOffset = float2(0.f, 0.f);
float g_Opacity = 1.f;
float g_OpacityPower = 1.f;
float4 g_ColorTint = 1.f;
/* Presentation-only Valtan vortex treatment. 0=none, 1=dark aperture,
   2=red ring, 3=masked red cloud disc. The map-object path binds its bounded
   state per draw and restores profile 0/strength 0 before returning. */
uint g_PresentationVortexProfile = 0;
float g_PresentationVortexStrength = 0.f;
/* The source game's dye contract: the mask's channels select colour regions
of a mostly achromatic diffuse and each region multiplies its tint in. */
Texture2D g_DyeMaskTexture;
uint g_HasDyeMask = 0;
float4 g_DyeDiffuseColor = 1.f;
float4 g_DyeRegionA = 1.f;
float4 g_DyeRegionB = 1.f;
float4 g_DyeRegionC = 1.f;

struct VS_IN
{
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float3 vTangent : TANGENT;
    float3 vBinormal : BINORMAL;
    float2 vTexcoord : TEXCOORD0;
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
    /* Radial masks must remain centered while the authored texture pans. */
    float2 vRawTexcoord : TEXCOORD3;
};

VS_OUT VS_MAIN(VS_IN input)
{
    VS_OUT output;
    matrix worldView = mul(g_WorldMatrix, g_ViewMatrix);
    matrix worldViewProjection = mul(worldView, g_ProjMatrix);
    output.vPosition = mul(float4(input.vPosition, 1.f), worldViewProjection);
    float3 normal = normalize(
        mul(float4(input.vNormal, 0.f), g_WorldInvTransposeMatrix).xyz);
    float3 tangentLinear =
        mul(float4(input.vTangent, 0.f), g_WorldMatrix).xyz;
    float3 tangent = normalize(
        tangentLinear - normal * dot(tangentLinear, normal));
    float3 sourceBinormalLinear =
        mul(float4(input.vBinormal, 0.f), g_WorldMatrix).xyz;
    float handedness =
        dot(cross(normal, tangent), sourceBinormalLinear) < 0.f ? -1.f : 1.f;
    float3 binormal = normalize(cross(normal, tangent)) * handedness;
    output.vNormal = float4(normal, 0.f);
    output.vTangent = float4(tangent, 0.f);
    output.vBinormal = float4(binormal, 0.f);
    output.vTexcoord = input.vTexcoord * g_UVScale + g_UVOffset;
    output.vWorldPos = mul(float4(input.vPosition, 1.f), g_WorldMatrix);
    output.vProjPos = output.vPosition;
    output.vRawTexcoord = input.vTexcoord;
    return output;
}

VS_OUT VS_MAIN_SKY(VS_IN input)
{
    VS_OUT output = VS_MAIN(input);
    output.vPosition.z = output.vPosition.w * 0.99999f;
    output.vProjPos = output.vPosition;
    return output;
}

struct PS_OUT
{
    float4 vDiffuse : SV_TARGET0;
    float4 vNormal : SV_TARGET1;
    float4 vDepth : SV_TARGET2;
    float4 vPickPos : SV_TARGET3;
    float4 vEmissive : SV_TARGET4;
};

PS_OUT PS_MAIN(VS_OUT input)
{
    PS_OUT output;
    float4 diffuse = g_DiffuseTexture.Sample(LinearSampler, input.vTexcoord);
    diffuse *= g_ColorTint;
    if (diffuse.a < 0.3f)
        discard;
    if (0 != g_HasDyeMask)
    {
        float3 mask = g_DyeMaskTexture.Sample(LinearSampler, input.vTexcoord).rgb;
        float3 tint = g_DyeDiffuseColor.rgb;
        tint *= lerp(1.f.xxx, g_DyeRegionA.rgb, mask.r);
        tint *= lerp(1.f.xxx, g_DyeRegionB.rgb, mask.g);
        tint *= lerp(1.f.xxx, g_DyeRegionC.rgb, mask.b);
        diffuse.rgb *= tint;
    }
    float3 normal = normalize(input.vNormal.xyz);
    if (0 != g_HasNormalTexture)
    {
        float4 encodedNormal =
            g_NormalTexture.Sample(LinearSampler, input.vTexcoord);
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
            LinearSampler, input.vTexcoord).rgb;
        specularMask *= dot(specular, float3(0.299f, 0.587f, 0.114f));
    }
    output.vNormal = float4(normal * 0.5f + 0.5f, specularMask);
    output.vDepth = float4(
        input.vProjPos.z / input.vProjPos.w,
        input.vProjPos.w / 1000.f, g_SpecularPower, 0.f);
    output.vPickPos = input.vWorldPos;
    output.vEmissive = 0.f;
    if (0 != g_HasEmissiveTexture)
    {
        float3 emissive = g_EmissiveTexture.Sample(
            LinearSampler, input.vTexcoord).rgb;
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

struct PS_OUT_FORWARD
{
    float4 vColor : SV_TARGET0;
};

float PresentationVortexRadial(float2 rawTexcoord)
{
    return length(rawTexcoord - float2(0.5f, 0.5f)) * 2.f;
}

float PresentationVortexRadialEdgeMask(float2 rawTexcoord)
{
    /* Every midpoint and corner of the source square is exactly transparent.
       The authored texture may pan, but the silhouette stays camera-centred. */
    const float radial = PresentationVortexRadial(rawTexcoord);
    return 1.f - smoothstep(0.86f, 1.f, radial);
}

PS_OUT_FORWARD PS_MAIN_ALPHA(VS_OUT input)
{
    PS_OUT_FORWARD output;
    const float4 textureColor =
        g_DiffuseTexture.Sample(LinearSampler, input.vTexcoord);
    float4 color = textureColor;
    color.rgb *= g_ColorTint.rgb;
    float opacityMask = 1.f;
    if (0 != g_HasOpacityTexture)
    {
        float3 opacitySample =
            g_OpacityTexture.Sample(LinearSampler, input.vTexcoord).rgb;
        opacityMask = pow(saturate(dot(
            opacitySample, float3(0.299f, 0.587f, 0.114f))),
            g_OpacityPower);
    }
    const float authoredAlpha = saturate(
        color.a * opacityMask * g_Opacity * g_ColorTint.a);

    const float vortexStrength = saturate(g_PresentationVortexStrength);
    if (1 == g_PresentationVortexProfile && vortexStrength > 0.f)
    {
        /* A nearly black alpha-blended disc is required to remove light from
           the arena sky. The soft burgundy shoulder retains cloud detail. */
        const float radial = PresentationVortexRadial(input.vRawTexcoord);
        const float radialEdge =
            PresentationVortexRadialEdgeMask(input.vRawTexcoord);
        const float core = 1.f - smoothstep(0.18f, 0.72f, radial);
        const float shoulder =
            smoothstep(0.14f, 0.38f, radial) *
            (1.f - smoothstep(0.66f, 0.94f, radial));
        const float textureDetail = saturate(dot(
            textureColor.rgb, float3(0.299f, 0.587f, 0.114f)) * 1.35f);
        const float radialMask =
            saturate(core + shoulder * 0.42f) *
            lerp(0.72f, 1.f, textureDetail) * radialEdge;
        const float rimBlend = smoothstep(0.16f, 0.92f, radial);
        color.rgb = lerp(
            float3(0.004f, 0.001f, 0.003f),
            float3(0.075f, 0.004f, 0.012f), rimBlend) *
            lerp(0.72f, 1.f, textureDetail);
        /* The procedural disc owns its silhouette. Diffuse/opacity texture
           alpha may contain a hollow center and must not erase the aperture. */
        const float apertureOpacity = saturate(
            g_Opacity * g_ColorTint.a);
        color.a = saturate(
            apertureOpacity * 2.2f * radialMask * vortexStrength);
    }
    else if (2 == g_PresentationVortexProfile && vortexStrength > 0.f)
    {
        /* Preserve the panning source texture as moving breakup, but form the
           overall ring from the unpanned UV so its center never drifts. */
        const float2 centeredUV = input.vRawTexcoord - float2(0.5f, 0.5f);
        const float radial = PresentationVortexRadial(input.vRawTexcoord);
        const float radialEdge =
            PresentationVortexRadialEdgeMask(input.vRawTexcoord);
        const float annulus =
            smoothstep(0.25f, 0.43f, radial) *
            (1.f - smoothstep(0.72f, 0.94f, radial));
        const float safeAngleX = radial > 0.00001f ?
            centeredUV.x : 1.f;
        const float angle = atan2(centeredUV.y, safeAngleX);
        const float angularBreak =
            abs(sin(angle * 7.f + radial * 19.f));
        const float textureDetail = saturate(dot(
            textureColor.rgb, float3(0.299f, 0.587f, 0.114f)) * 1.6f);
        const float brokenMask = smoothstep(
            0.18f, 0.72f,
            textureDetail * 0.72f + angularBreak * 0.28f);
        const float ringMask = annulus * brokenMask * radialEdge;
        color.rgb = float3(0.46f, 0.008f, 0.022f) *
            lerp(0.42f, 0.88f, textureDetail);
        color.a = saturate(
            g_Opacity * g_ColorTint.a * 0.9f * ringMask * vortexStrength);
    }
    else if (3 == g_PresentationVortexProfile && vortexStrength > 0.f)
    {
        /* Generic cloud tiles are useful only as moving breakup. A procedural
           burgundy disc owns the colour and circular silhouette. */
        const float2 centeredUV = input.vRawTexcoord - float2(0.5f, 0.5f);
        const float radial = PresentationVortexRadial(input.vRawTexcoord);
        const float radialEdge =
            PresentationVortexRadialEdgeMask(input.vRawTexcoord);
        const float safeAngleX = radial > 0.00001f ? centeredUV.x : 1.f;
        const float angle = atan2(centeredUV.y, safeAngleX);
        const float textureDetail = saturate(dot(
            textureColor.rgb, float3(0.299f, 0.587f, 0.114f)));
        const float spiral = 0.5f + 0.5f *
            sin(angle * 5.f - radial * 19.f + textureDetail * 3.f);
        const float breakup = smoothstep(
            0.12f, 0.88f, textureDetail * 0.68f + spiral * 0.32f);
        const float hollowShoulder = lerp(
            0.32f, 1.f, smoothstep(0.12f, 0.46f, radial));
        const float cloudMask = radialEdge * hollowShoulder *
            lerp(0.34f, 0.92f, breakup);
        const float burgundy = saturate(
            breakup * 0.62f + spiral * 0.22f + (1.f - radial) * 0.16f);
        color.rgb = lerp(
            float3(0.012f, 0.0005f, 0.002f),
            float3(0.16f, 0.005f, 0.018f), burgundy);
        color.a = saturate(
            g_Opacity * g_ColorTint.a * 0.82f * cloudMask * vortexStrength);
    }
    else
    {
        /* NONE deliberately remains the original map-material path. */
        color.a = authoredAlpha;
    }
    if (color.a < 0.001f)
        discard;
    output.vColor = color;
    return output;
}

PS_OUT_FORWARD PS_MAIN_SKY(VS_OUT input)
{
    PS_OUT_FORWARD output;
    float4 color = g_DiffuseTexture.Sample(LinearSampler, input.vTexcoord);
    output.vColor = float4(color.rgb * g_ColorTint.rgb, 1.f);
    return output;
}

void PS_MAIN_SHADOW(VS_OUT input)
{
    float4 diffuse =
        g_DiffuseTexture.Sample(LinearSampler, input.vTexcoord) *
        g_ColorTint;
    if (diffuse.a < 0.3f)
        discard;
}

struct PS_OUT_DEFERRED_EMISSIVE_OVERLAY
{
    float3 vEmissive : SV_TARGET4;
};

PS_OUT_DEFERRED_EMISSIVE_OVERLAY PS_MAIN_DEFERRED_EMISSIVE_OVERLAY(
    VS_OUT input)
{
    clip((float)g_HasEmissiveTexture - 0.5f);

    const float3 emissive = g_EmissiveTexture.Sample(
        LinearSampler, input.vTexcoord).rgb;
    clip(max(emissive.r, max(emissive.g, emissive.b)) - (1.f / 255.f));

    PS_OUT_DEFERRED_EMISSIVE_OVERLAY output;
    output.vEmissive =
        emissive * g_EmissiveColor.rgb * g_EmissiveIntensity;
    return output;
}

/* The source crack mesh is nearly coplanar with the intact arena stone.
   A one-unit negative bias makes only this read-only overlay win equal-depth
   comparisons without moving the authored world transform. */
RasterizerState RS_DeferredEmissiveOverlay
{
    FillMode = Solid;
    CullMode = Back;
    FrontCounterClockwise = false;
    DepthBias = -1;
    DepthBiasClamp = 0.f;
    SlopeScaledDepthBias = -1.f;
};

/* MRT_GameObject binds five targets. Preserve the finished opaque G-buffer
   and update RGB of Target_Emissive only. */
BlendState BS_DeferredEmissiveOverlay
{
    BlendEnable[0] = false;
    BlendEnable[1] = false;
    BlendEnable[2] = false;
    BlendEnable[3] = false;
    BlendEnable[4] = false;
    RenderTargetWriteMask[0] = 0x00;
    RenderTargetWriteMask[1] = 0x00;
    RenderTargetWriteMask[2] = 0x00;
    RenderTargetWriteMask[3] = 0x00;
    RenderTargetWriteMask[4] = 0x07;
};

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
    pass MirroredPass
    {
        SetRasterizerState(RS_Cull_CW);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
    pass TwoSidedOpaquePass
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
    pass AlphaBackPass
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_ALPHA();
    }
    pass AlphaFrontPass
    {
        SetRasterizerState(RS_Cull_CW);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_ALPHA();
    }
    pass AlphaTwoSidedPass
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_ALPHA();
    }
    pass SkyBackPass
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN_SKY();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_SKY();
    }
    pass SkyFrontPass
    {
        SetRasterizerState(RS_Cull_CW);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN_SKY();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_SKY();
    }
    pass SkyTwoSidedPass
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN_SKY();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_SKY();
    }
    pass AdditiveBackPass
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_ALPHA();
    }
    pass AdditiveFrontPass
    {
        SetRasterizerState(RS_Cull_CW);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_ALPHA();
    }
    pass AdditiveTwoSidedPass
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_ALPHA();
    }
    pass ShadowBackPass
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_SHADOW();
    }
    pass ShadowFrontPass
    {
        SetRasterizerState(RS_Cull_CW);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_SHADOW();
    }
    pass ShadowTwoSidedPass
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_SHADOW();
    }
    pass DeferredEmissiveOverlayPass
    {
        SetRasterizerState(RS_DeferredEmissiveOverlay);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_DeferredEmissiveOverlay,
            float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DEFERRED_EMISSIVE_OVERLAY();
    }
}
