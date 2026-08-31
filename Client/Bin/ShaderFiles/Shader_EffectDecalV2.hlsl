#include "Shader_EffectV2_Common.hlsli"

float4x4 g_ViewMatrixInverse;
float4x4 g_ProjMatrixInverse;
float4x4 g_DecalWorldInverse;
Texture2D g_NormalTexture;
float2 g_DecalSize = float2(1.f, 1.f);
float g_DecalDepth = 1.f;
float g_DecalEdgeFade = 0.f;
float3 g_DecalUp = float3(0.f, 1.f, 0.f);
float g_DecalNormalCutoff = -1.f;

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

VS_OUT VS_MAIN(VS_IN input)
{
	VS_OUT output;
	output.vPosition = float4(input.vPosition.x * 2.f, input.vPosition.y * 2.f, 0.f, 1.f);
	output.vTexcoord = input.vTexcoord;
	return output;
}

PS_EFFECT_OUT Decal_Shade(VS_OUT input, uniform bool bPremultiply)
{
	const float4 depth = g_DepthTexture.Sample(PointSampler, input.vTexcoord);
	const float viewZ = depth.y * 1000.f;
	float4 worldPosition;
	worldPosition.x = input.vTexcoord.x * 2.f - 1.f;
	worldPosition.y = input.vTexcoord.y * -2.f + 1.f;
	worldPosition.z = depth.x;
	worldPosition.w = 1.f;
	worldPosition *= viewZ;
	worldPosition = mul(worldPosition, g_ProjMatrixInverse);
	worldPosition = mul(worldPosition, g_ViewMatrixInverse);

	const float3 local = mul(worldPosition, g_DecalWorldInverse).xyz;
	const float2 halfSize = max(g_DecalSize, float2(0.0001f, 0.0001f)) * 0.5f;
	const float halfDepth = max(g_DecalDepth, 0.0001f) * 0.5f;
	clip(halfSize.x - abs(local.x));
	clip(halfSize.y - abs(local.z));
	clip(halfDepth - abs(local.y));
	if (g_DecalNormalCutoff > -1.f)
	{
		const float3 sceneNormal =
			g_NormalTexture.Sample(PointSampler, input.vTexcoord).xyz * 2.f - 1.f;
		clip(dot(normalize(sceneNormal), normalize(g_DecalUp)) - g_DecalNormalCutoff);
	}

	PS_EFFECT_IN effectInput;
	effectInput.vPosition = input.vPosition;
	effectInput.vTexcoord = float2(
		local.x / (halfSize.x * 2.f) + 0.5f,
		0.5f - local.z / (halfSize.y * 2.f));
	effectInput.vWorldNormal = float3(0.f, 0.f, 0.f);
	effectInput.vWorldPosition = worldPosition.xyz;
	effectInput.vInstanceColor = float4(1.f, 1.f, 1.f, 1.f);
	effectInput.vProjPos = float4(0.f, 0.f, 0.f, 0.f);
	PS_EFFECT_OUT output = PS_EFFECT_V2(effectInput);

	if (g_DecalEdgeFade > 0.f)
	{
		const float3 normalized = float3(
			abs(local.x) / halfSize.x, abs(local.y) / halfDepth, abs(local.z) / halfSize.y);
		const float edge = 1.f - max(normalized.x, max(normalized.y, normalized.z));
		const float fade = saturate(edge / g_DecalEdgeFade);
		output.vSceneColor.a *= fade;
		output.vDistortion *= fade;
	}
	if (bPremultiply)
		output.vSceneColor.rgb *= output.vSceneColor.a;
	return output;
}

PS_EFFECT_OUT PS_MAIN(VS_OUT input)
{
	return Decal_Shade(input, false);
}

PS_EFFECT_OUT PS_MAIN_MULTIPLY(VS_OUT input)
{
	return Decal_Shade(input, true);
}

technique11 DefaultTechnique
{
	pass Alpha
	{
		SetRasterizerState(RS_EffectV2);
		SetDepthStencilState(DSS_ZNone, 0);
		SetBlendState(BS_EffectV2Alpha, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN();
	}
	pass Additive
	{
		SetRasterizerState(RS_EffectV2);
		SetDepthStencilState(DSS_ZNone, 0);
		SetBlendState(BS_EffectV2Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN();
	}
	pass Multiply
	{
		SetRasterizerState(RS_EffectV2);
		SetDepthStencilState(DSS_ZNone, 0);
		SetBlendState(BS_EffectV2Multiply, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN_MULTIPLY();
	}
}
