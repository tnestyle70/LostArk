#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix;
float4x4 g_ViewMatrix;
float4x4 g_ProjMatrix;

float g_Time;
float4 g_Tint = float4(1.f, 1.f, 1.f, 1.f);
float2 g_UVScale = float2(1.f, 1.f);
float2 g_BasePan = float2(0.f, 0.f);
float g_NoiseStrength = 0.f;
float g_NoiseScale = 1.f;
float2 g_NoisePan = float2(0.f, 0.f);
float g_EmissiveIntensity = 1.f;
float g_DissolveAmount = 0.f;
float g_DissolveSoftness = 0.1f;

Texture2D g_BaseTexture;
Texture2D g_NoiseTexture;
Texture2D g_MaskTexture;
Texture2D g_EmissiveTexture;
Texture2D g_DissolveTexture;
uint g_HasBase = 0;
uint g_HasNoise = 0;
uint g_HasMask = 0;
uint g_HasEmissive = 0;
uint g_HasDissolve = 0;

RasterizerState RS_EffectV2
{
	FillMode = Solid;
	CullMode = None;
	FrontCounterClockwise = false;
};

struct PS_EFFECT_IN
{
	float4 vPosition : SV_POSITION;
	float2 vTexcoord : TEXCOORD0;
};

float4 PS_EFFECT_V2(PS_EFFECT_IN input) : SV_TARGET0
{
	const float2 uv = input.vTexcoord * g_UVScale + g_BasePan * g_Time;

	float2 distortion = float2(0.f, 0.f);
	if (0 != g_HasNoise)
	{
		const float2 noiseUV = uv * g_NoiseScale + g_NoisePan * g_Time;
		distortion = (g_NoiseTexture.Sample(LinearSampler, noiseUV).rg * 2.f - 1.f) *
			g_NoiseStrength;
	}
	const float2 baseUV = uv + distortion;

	float4 base = float4(1.f, 1.f, 1.f, 1.f);
	if (0 != g_HasBase)
		base = g_BaseTexture.Sample(LinearSampler, baseUV);

	float mask = 1.f;
	if (0 != g_HasMask)
		mask = g_MaskTexture.Sample(LinearSampler, uv).r;

	float3 emissive = float3(0.f, 0.f, 0.f);
	if (0 != g_HasEmissive)
		emissive = g_EmissiveTexture.Sample(LinearSampler, baseUV).rgb * g_EmissiveIntensity;

	float dissolve = 1.f;
	if (0 != g_HasDissolve)
	{
		const float threshold = g_DissolveTexture.Sample(LinearSampler, uv).r;
		dissolve = smoothstep(
			g_DissolveAmount - g_DissolveSoftness,
			g_DissolveAmount + g_DissolveSoftness,
			threshold);
	}

	const float alpha = base.a * mask * dissolve * g_Tint.a;
	if (alpha <= 0.001f)
		discard;
	const float3 color = base.rgb * g_Tint.rgb + emissive;
	return float4(color, alpha);
}

#define EFFECT_V2_PASSES(VS_FUNC) \
	pass AlphaDepth \
	{ \
		SetRasterizerState(RS_EffectV2); \
		SetDepthStencilState(DSS_ReadOnly, 0); \
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff); \
		VertexShader = compile vs_5_0 VS_FUNC(); \
		GeometryShader = NULL; \
		PixelShader = compile ps_5_0 PS_EFFECT_V2(); \
	} \
	pass AdditiveDepth \
	{ \
		SetRasterizerState(RS_EffectV2); \
		SetDepthStencilState(DSS_ReadOnly, 0); \
		SetBlendState(BS_Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff); \
		VertexShader = compile vs_5_0 VS_FUNC(); \
		GeometryShader = NULL; \
		PixelShader = compile ps_5_0 PS_EFFECT_V2(); \
	} \
	pass AlphaNoDepth \
	{ \
		SetRasterizerState(RS_EffectV2); \
		SetDepthStencilState(DSS_ZNone, 0); \
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff); \
		VertexShader = compile vs_5_0 VS_FUNC(); \
		GeometryShader = NULL; \
		PixelShader = compile ps_5_0 PS_EFFECT_V2(); \
	} \
	pass AdditiveNoDepth \
	{ \
		SetRasterizerState(RS_EffectV2); \
		SetDepthStencilState(DSS_ZNone, 0); \
		SetBlendState(BS_Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff); \
		VertexShader = compile vs_5_0 VS_FUNC(); \
		GeometryShader = NULL; \
		PixelShader = compile ps_5_0 PS_EFFECT_V2(); \
	}
