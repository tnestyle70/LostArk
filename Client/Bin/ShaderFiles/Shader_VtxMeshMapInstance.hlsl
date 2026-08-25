#include "Engine_Shader_Defines.hlsli"

float4x4 g_ViewMatrix;
float4x4 g_ProjMatrix;

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
float g_SpecularIntensity = 1.f;
float g_SpecularPower = 50.f;

float2 g_UVScale = float2(1.f, 1.f);
float2 g_UVOffset = float2(0.f, 0.f);

float g_Opacity = 1.f;
float g_OpacityPower = 1.f;

float4 g_ColorTint = 1.f;

/* Water presentation. These come from Data/Maps/Authoring/<Area>.mapwater.json,
   which the publisher fills from the source MaterialInstanceConstant chain, so
   the names below match the original parameter names rather than being tuned
   here. An asset that is not water leaves every value at the identity defaults
   and never reaches the water passes. */
Texture2D g_DetailNormalTexture;
Texture2D g_ReflectionTexture;

vector g_vCamPosition;
float g_ElapsedTime = 0.f;

uint g_HasDetailNormalTexture = 0;
uint g_HasReflectionTexture = 0;

float g_WaterOpacity = 1.f;
float g_WaterOpacityPower = 1.f;
float g_WaterFresnelIntensity = 0.f;
float g_WaterFresnelPower = 1.f;
float g_WaterScreenDistortionIntensity = 0.f;
float g_WaterNormalIntensity = 0.f;
float g_WaterDetailNormalIntensity = 0.f;
float g_WaterReflectionIntensity = 0.f;
float g_WaterDiffuseTiling = 1.f;
float4 g_WaterDiffuseColor = float4(1.f, 1.f, 1.f, 1.f);
float4 g_WaterReflectionColor = float4(1.f, 1.f, 1.f, 1.f);
/* xy is tiling, zw is panning speed in UV units per second. */
float4 g_WaterNormalTilingPanning = float4(1.f, 1.f, 0.f, 0.f);
float4 g_WaterDetailNormalTilingPanning = float4(1.f, 1.f, 0.f, 0.f);
float4 g_WaterReflectionTilingPanning = float4(1.f, 1.f, 0.f, 0.f);

struct VS_IN
{
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float3 vTangent : TANGENT;
    float3 vBinormal : BINORMAL;
    float2 vTexcoord : TEXCOORD0;

    float4 vWorld0 : WORLD0;
    float4 vWorld1 : WORLD1;
    float4 vWorld2 : WORLD2;
    float4 vWorld3 : WORLD3;

    float4 vWorldInvTranspose0 : WORLDINVTRANSPOSE0;
    float4 vWorldInvTranspose1 : WORLDINVTRANSPOSE1;
    float4 vWorldInvTranspose2 : WORLDINVTRANSPOSE2;
    float4 vWorldInvTranspose3 : WORLDINVTRANSPOSE3;
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
};

VS_OUT VS_MAIN(VS_IN input)
{
    VS_OUT output;

    const float4x4 world = float4x4(
		input.vWorld0,
		input.vWorld1,
		input.vWorld2,
		input.vWorld3);

    const float4x4 worldInvTranspose = float4x4(
		input.vWorldInvTranspose0,
		input.vWorldInvTranspose1,
		input.vWorldInvTranspose2,
		input.vWorldInvTranspose3);

    const float4 worldPosition =
		mul(float4(input.vPosition, 1.f), world);

    output.vPosition =
		mul(mul(
			worldPosition,
			g_ViewMatrix),
			g_ProjMatrix);

    const float3 normal = normalize(
		mul(
			float4(input.vNormal, 0.f),
			worldInvTranspose).xyz);

    const float3 tangentLinear =
		mul(
			float4(input.vTangent, 0.f),
			world).xyz;

    const float3 tangent = normalize(
		tangentLinear -
		normal * dot(
			tangentLinear,
			normal));

    const float3 sourceBinormal =
		mul(
			float4(input.vBinormal, 0.f),
			world).xyz;

    const float handedness =
		dot(
			cross(normal, tangent),
			sourceBinormal) < 0.f ?
			-1.f : 1.f;

    const float3 binormal =
		normalize(
			cross(normal, tangent)) *
		handedness;

    output.vNormal =
		float4(normal, 0.f);

    output.vTangent =
		float4(tangent, 0.f);

    output.vBinormal =
		float4(binormal, 0.f);

    output.vTexcoord =
		input.vTexcoord *
		g_UVScale +
		g_UVOffset;

    output.vWorldPos =
		worldPosition;

    output.vProjPos =
		output.vPosition;

    return output;
}

VS_OUT VS_MAIN_SKY(VS_IN input)
{
    VS_OUT output = VS_MAIN(input);

    output.vPosition.z =
		output.vPosition.w * 0.99999f;

    output.vProjPos =
		output.vPosition;

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

    float4 diffuse =
		g_DiffuseTexture.Sample(
			LinearSampler,
			input.vTexcoord);

    diffuse *= g_ColorTint;

    if (diffuse.a < 0.3f)
        discard;

    float3 normal =
		normalize(input.vNormal.xyz);

    if (0 != g_HasNormalTexture)
    {
        const float4 encodedNormal =
			g_NormalTexture.Sample(
				LinearSampler,
				input.vTexcoord);

        float3 tangentNormal;

        if (encodedNormal.b <= 0.0001f)
        {
            const float2 tangentXY =
				encodedNormal.rg * 2.f - 1.f;

            const float tangentZ =
				sqrt(saturate(
					1.f -
					dot(
						tangentXY,
						tangentXY)));

            tangentNormal =
				float3(
					tangentXY,
					tangentZ);
        }
        else
        {
            tangentNormal =
				normalize(
					encodedNormal.xyz *
					2.f - 1.f);
        }

        const float3x3 tangentToWorld =
			float3x3(
				normalize(
					input.vTangent.xyz),
				normalize(
					input.vBinormal.xyz) *
					-1.f,
				normal);

        normal =
			normalize(
				mul(
					tangentNormal,
					tangentToWorld));
    }

    float specularMask =
		g_SpecularIntensity;

    if (0 != g_HasSpecularTexture)
    {
        const float3 specular =
			g_SpecularTexture.Sample(
				LinearSampler,
				input.vTexcoord).rgb;

        specularMask *=
			dot(
				specular,
				float3(
					0.299f,
					0.587f,
					0.114f));
    }

    output.vDiffuse =
		diffuse;

    output.vNormal =
		float4(
			normal * 0.5f + 0.5f,
			specularMask);

    output.vDepth =
		float4(
			input.vProjPos.z /
				input.vProjPos.w,
			input.vProjPos.w /
				1000.f,
			g_SpecularPower,
			0.f);

    output.vPickPos =
		input.vWorldPos;

    output.vEmissive = 0.f;

    if (0 != g_HasEmissiveTexture)
    {
        const float3 emissive =
			g_EmissiveTexture.Sample(
				LinearSampler,
				input.vTexcoord).rgb;

        output.vEmissive =
			float4(
				emissive *
				g_EmissiveIntensity,
				0.f);
    }

    return output;
}

struct PS_OUT_FORWARD
{
    float4 vColor : SV_TARGET0;
};

PS_OUT_FORWARD PS_MAIN_ALPHA(
	VS_OUT input)
{
    PS_OUT_FORWARD output;

    float4 color =
		g_DiffuseTexture.Sample(
			LinearSampler,
			input.vTexcoord);

    color.rgb *=
		g_ColorTint.rgb;

    float opacityMask = 1.f;

    if (0 != g_HasOpacityTexture)
    {
        const float3 opacitySample =
			g_OpacityTexture.Sample(
				LinearSampler,
				input.vTexcoord).rgb;

        opacityMask =
			pow(
				saturate(
					dot(
						opacitySample,
						float3(
							0.299f,
							0.587f,
							0.114f))),
				g_OpacityPower);
    }

    color.a =
		saturate(
			color.a *
			opacityMask *
			g_Opacity *
			g_ColorTint.a);

    if (color.a < 0.001f)
        discard;

    output.vColor = color;
    return output;
}

/* RT1 is the scene distortion buffer that MRT_SceneHDR binds alongside scene
   colour. Shader_Deferred's final pass reads it as a screen-space UV offset,
   clamped to +-0.05, so writing here is how a water surface refracts what is
   behind it without a second scene-colour copy. */
struct PS_OUT_WATER
{
    float4 vColor : SV_TARGET0;
    float4 vDistortion : SV_TARGET1;
};

float2 Water_PannedUV(float2 baseUV, float4 tilingPanning)
{
    return baseUV * tilingPanning.xy + tilingPanning.zw * g_ElapsedTime;
}

/* Returns a tangent-space normal whose XY is scaled by the source intensity.
   The source water presets use very small intensities on purpose: a windless
   river is nearly flat and the ripple reads through the reflection and the
   distortion rather than through shading. */
float3 Water_SampleNormal(
	Texture2D normalTexture, float2 baseUV, float4 tilingPanning, float intensity)
{
    const float3 packed =
		normalTexture.Sample(
			LinearSampler,
			Water_PannedUV(baseUV, tilingPanning)).xyz * 2.f - 1.f;

    return float3(packed.xy * intensity, 1.f);
}

PS_OUT_WATER PS_MAIN_WATER(
	VS_OUT input)
{
    PS_OUT_WATER output;

    float3 tangentNormal = float3(0.f, 0.f, 1.f);

    if (0 != g_HasNormalTexture)
    {
        tangentNormal =
			Water_SampleNormal(
				g_NormalTexture,
				input.vTexcoord,
				g_WaterNormalTilingPanning,
				g_WaterNormalIntensity);
    }

    if (0 != g_HasDetailNormalTexture)
    {
        const float3 detail =
			Water_SampleNormal(
				g_DetailNormalTexture,
				input.vTexcoord,
				g_WaterDetailNormalTilingPanning,
				g_WaterDetailNormalIntensity);

        tangentNormal =
			float3(
				tangentNormal.xy + detail.xy,
				1.f);
    }

    tangentNormal = normalize(tangentNormal);

    const float3x3 tangentBasis =
		float3x3(
			normalize(input.vTangent.xyz),
			normalize(input.vBinormal.xyz),
			normalize(input.vNormal.xyz));

    const float3 worldNormal =
		normalize(mul(tangentNormal, tangentBasis));

    const float3 viewDirection =
		normalize(g_vCamPosition.xyz - input.vWorldPos.xyz);

    const float fresnel =
		saturate(
			pow(
				saturate(1.f - saturate(dot(worldNormal, viewDirection))),
				max(g_WaterFresnelPower, 0.0001f)) *
			g_WaterFresnelIntensity);

    float4 color =
		g_DiffuseTexture.Sample(
			LinearSampler,
			input.vTexcoord * g_WaterDiffuseTiling);

    color.rgb *= g_WaterDiffuseColor.rgb * g_ColorTint.rgb;

    if (0 != g_HasReflectionTexture)
    {
        const float3 reflection =
			g_ReflectionTexture.Sample(
				LinearSampler,
				Water_PannedUV(
					input.vTexcoord + tangentNormal.xy,
					g_WaterReflectionTilingPanning)).rgb;

        color.rgb +=
			reflection *
			g_WaterReflectionColor.rgb *
			g_WaterReflectionIntensity *
			fresnel;
    }

    /* PROJECT_RECONSTRUCTED: the source opacity graph is not in the cooked
       package, so the authored opacity and opacity_power set the body alpha and
       the fresnel term closes the grazing edge. The two source values are used
       exactly; only the way they combine is ours. */
    float alpha =
		saturate(
			pow(
				saturate(g_WaterOpacity),
				max(g_WaterOpacityPower, 0.0001f)));

    alpha =
		saturate(
			alpha + (1.f - alpha) * fresnel);

    alpha *= g_ColorTint.a;

    if (alpha < 0.001f)
        discard;

    output.vColor = float4(color.rgb, alpha);

    output.vDistortion =
		float4(
			tangentNormal.xy * g_WaterScreenDistortionIntensity * 0.05f,
			0.f,
			alpha);

    return output;
}

PS_OUT_FORWARD PS_MAIN_SKY(
	VS_OUT input)
{
    PS_OUT_FORWARD output;

    const float4 color =
		g_DiffuseTexture.Sample(
			LinearSampler,
			input.vTexcoord);

    output.vColor =
		float4(
			color.rgb *
				g_ColorTint.rgb,
			1.f);

    return output;
}

void PS_MAIN_SHADOW(
	VS_OUT input)
{
    float4 diffuse =
		g_DiffuseTexture.Sample(
			LinearSampler,
			input.vTexcoord) *
		g_ColorTint;

    if (diffuse.a < 0.3f)
        discard;
}

technique11 DefaultTechnique
{
    pass DefaultPass
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(
			BS_Default,
			float4(0.f, 0.f, 0.f, 0.f),
			0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }

    pass MirroredPass
    {
        SetRasterizerState(RS_Cull_CW);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(
			BS_Default,
			float4(0.f, 0.f, 0.f, 0.f),
			0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }

    pass TwoSidedOpaquePass
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(
			BS_Default,
			float4(0.f, 0.f, 0.f, 0.f),
			0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }

    pass AlphaBackPass
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(
			BS_AlphaBlend,
			float4(0.f, 0.f, 0.f, 0.f),
			0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_ALPHA();
    }

    pass AlphaFrontPass
    {
        SetRasterizerState(RS_Cull_CW);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(
			BS_AlphaBlend,
			float4(0.f, 0.f, 0.f, 0.f),
			0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_ALPHA();
    }

    pass AlphaTwoSidedPass
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(
			BS_AlphaBlend,
			float4(0.f, 0.f, 0.f, 0.f),
			0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_ALPHA();
    }

    pass SkyBackPass
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(
			BS_Default,
			float4(0.f, 0.f, 0.f, 0.f),
			0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN_SKY();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_SKY();
    }

    pass SkyFrontPass
    {
        SetRasterizerState(RS_Cull_CW);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(
			BS_Default,
			float4(0.f, 0.f, 0.f, 0.f),
			0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN_SKY();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_SKY();
    }

    pass SkyTwoSidedPass
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(
			BS_Default,
			float4(0.f, 0.f, 0.f, 0.f),
			0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN_SKY();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_SKY();
    }

    pass AdditiveBackPass
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(
			BS_Additive,
			float4(0.f, 0.f, 0.f, 0.f),
			0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_ALPHA();
    }

    pass AdditiveFrontPass
    {
        SetRasterizerState(RS_Cull_CW);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(
			BS_Additive,
			float4(0.f, 0.f, 0.f, 0.f),
			0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_ALPHA();
    }

    pass AdditiveTwoSidedPass
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(
			BS_Additive,
			float4(0.f, 0.f, 0.f, 0.f),
			0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_ALPHA();
    }

    pass ShadowBackPass
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(
			BS_Default,
			float4(0.f, 0.f, 0.f, 0.f),
			0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_SHADOW();
    }

    pass ShadowFrontPass
    {
        SetRasterizerState(RS_Cull_CW);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(
			BS_Default,
			float4(0.f, 0.f, 0.f, 0.f),
			0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_SHADOW();
    }

    pass ShadowTwoSidedPass
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(
			BS_Default,
			float4(0.f, 0.f, 0.f, 0.f),
			0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_SHADOW();
    }

	/* Water is appended after the shadow passes so every existing pass index
	   keeps its value. The source master declares bDisableDepthTest = false, so
	   the depth test stays on and only the depth write is dropped, exactly like
	   the alpha passes. */
    pass WaterBackPass
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(
			BS_AlphaBlend,
			float4(0.f, 0.f, 0.f, 0.f),
			0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_WATER();
    }

    pass WaterFrontPass
    {
        SetRasterizerState(RS_Cull_CW);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(
			BS_AlphaBlend,
			float4(0.f, 0.f, 0.f, 0.f),
			0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_WATER();
    }

    pass WaterTwoSidedPass
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(
			BS_AlphaBlend,
			float4(0.f, 0.f, 0.f, 0.f),
			0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_WATER();
    }
}
