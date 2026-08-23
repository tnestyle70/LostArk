#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix;
float4x4 g_ViewMatrix;
float4x4 g_ProjMatrix;

float g_Time;
float4 g_vCamPosition = float4(0.f, 0.f, 0.f, 1.f);
float4 g_RimColor = float4(1.f, 1.f, 1.f, 1.f);
float g_RimPower = 3.f;
float g_RimIntensity = 0.f;
float g_GhostAlpha = 0.f;
float4 g_ColorMul = float4(1.f, 1.f, 1.f, 1.f);
float4 g_ColorOffset = float4(0.f, 0.f, 0.f, 0.f);
float g_ColorClip = 0.f;
uint g_ColorClipChannel = 1;
float g_BloomIntensity = 1.f;
float g_DistortionIntensity = 0.f;
float2 g_UVStart = float2(0.f, 0.f);
float2 g_UVSpeed = float2(0.f, 0.f);
float2 g_UVTileCount = float2(1.f, 1.f);
float g_NoiseStrength = 0.f;
float g_NoiseScale = 1.f;
float2 g_NoisePan = float2(0.f, 0.f);
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

BlendState BS_EffectV2Alpha
{
	BlendEnable[0] = true;
	BlendEnable[1] = true;
	SrcBlend[0] = Src_Alpha;
	DestBlend[0] = Inv_Src_Alpha;
	BlendOp[0] = Add;
	SrcBlendAlpha[0] = One;
	DestBlendAlpha[0] = Inv_Src_Alpha;
	BlendOpAlpha[0] = Add;
	SrcBlend[1] = One;
	DestBlend[1] = One;
	BlendOp[1] = Add;
	SrcBlendAlpha[1] = One;
	DestBlendAlpha[1] = One;
	BlendOpAlpha[1] = Add;
	RenderTargetWriteMask[1] = 0x03;
};

BlendState BS_EffectV2Additive
{
	BlendEnable[0] = true;
	BlendEnable[1] = true;
	SrcBlend[0] = Src_Alpha;
	DestBlend[0] = One;
	BlendOp[0] = Add;
	SrcBlendAlpha[0] = One;
	DestBlendAlpha[0] = One;
	BlendOpAlpha[0] = Add;
	SrcBlend[1] = One;
	DestBlend[1] = One;
	BlendOp[1] = Add;
	SrcBlendAlpha[1] = One;
	DestBlendAlpha[1] = One;
	BlendOpAlpha[1] = Add;
	RenderTargetWriteMask[1] = 0x03;
};

BlendState BS_EffectV2Opaque
{
	BlendEnable[0] = false;
	BlendEnable[1] = true;
	SrcBlend[1] = One;
	DestBlend[1] = One;
	BlendOp[1] = Add;
	SrcBlendAlpha[1] = One;
	DestBlendAlpha[1] = One;
	BlendOpAlpha[1] = Add;
	RenderTargetWriteMask[1] = 0x03;
};

struct PS_EFFECT_IN
{
	float4 vPosition : SV_POSITION;
	float2 vTexcoord : TEXCOORD0;
	float3 vWorldNormal : NORMAL;
	float3 vWorldPosition : TEXCOORD1;
	float4 vInstanceColor : COLOR0;
};

struct PS_EFFECT_OUT
{
	float4 vSceneColor : SV_TARGET0;
	float4 vDistortion : SV_TARGET1;
};

PS_EFFECT_OUT PS_EFFECT_V2(PS_EFFECT_IN input)
{
	PS_EFFECT_OUT output;
	const float2 uv = input.vTexcoord * g_UVTileCount + g_UVStart + g_UVSpeed * g_Time;

	float fresnel = 0.f;
	if (dot(input.vWorldNormal, input.vWorldNormal) > 0.f)
	{
		const float3 N = normalize(input.vWorldNormal);
		const float3 V = normalize(g_vCamPosition.xyz - input.vWorldPosition);
		fresnel = pow(1.f - saturate(abs(dot(N, V))), max(g_RimPower, 0.01f));
	}

	float2 noise = float2(0.5f, 0.5f);
	float2 warp = float2(0.f, 0.f);
	if (0 != g_HasNoise)
	{
		const float2 noiseUV = uv * g_NoiseScale + g_NoisePan * g_Time;
		noise = g_NoiseTexture.Sample(LinearSampler, noiseUV).rg;
		warp = (noise * 2.f - 1.f) * g_NoiseStrength;
	}
	const float2 baseUV = uv + warp;

	float4 base = float4(1.f, 1.f, 1.f, 1.f);
	if (0 != g_HasBase)
		base = g_BaseTexture.Sample(LinearSampler, baseUV);

	float mask = 1.f;
	if (0 != g_HasMask)
		mask = g_MaskTexture.Sample(LinearSampler, uv).r;

	float dissolve = 1.f;
	if (0 != g_HasDissolve)
	{
		const float threshold = g_DissolveTexture.Sample(LinearSampler, uv).r;
		dissolve = smoothstep(
			g_DissolveAmount - g_DissolveSoftness,
			g_DissolveAmount + g_DissolveSoftness,
			threshold);
	}

	float4 color;
	color.rgb = max(base.rgb * g_ColorMul.rgb * input.vInstanceColor.rgb + g_ColorOffset.rgb,
		float3(0.f, 0.f, 0.f));
	color.a = saturate(base.a * mask * dissolve * g_ColorMul.a * input.vInstanceColor.a +
		g_ColorOffset.a);
	color.a *= lerp(1.f, fresnel, saturate(g_GhostAlpha));
	if (color.a <= 0.001f)
		discard;
	const float clipValue = (0 == g_ColorClipChannel) ?
		max(color.r, max(color.g, color.b)) : color.a;
	if (g_ColorClip > 0.f && clipValue <= g_ColorClip)
		discard;

	color.rgb += g_RimColor.rgb * fresnel * g_RimIntensity;
	if (0 != g_HasEmissive)
		color.rgb += g_EmissiveTexture.Sample(LinearSampler, baseUV).rgb * g_BloomIntensity;

	const float2 distortion = (0 != g_HasNoise) ?
		(noise * 2.f - 1.f) * g_DistortionIntensity * color.a : float2(0.f, 0.f);

	output.vSceneColor = color;
	output.vDistortion = float4(distortion, 0.f, 0.f);
	return output;
}

#define EFFECT_V2_PASSES(VS_FUNC) \
	pass AlphaDepth \
	{ \
		SetRasterizerState(RS_EffectV2); \
		SetDepthStencilState(DSS_ReadOnly, 0); \
		SetBlendState(BS_EffectV2Alpha, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff); \
		VertexShader = compile vs_5_0 VS_FUNC(); \
		GeometryShader = NULL; \
		PixelShader = compile ps_5_0 PS_EFFECT_V2(); \
	} \
	pass AdditiveDepth \
	{ \
		SetRasterizerState(RS_EffectV2); \
		SetDepthStencilState(DSS_ReadOnly, 0); \
		SetBlendState(BS_EffectV2Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff); \
		VertexShader = compile vs_5_0 VS_FUNC(); \
		GeometryShader = NULL; \
		PixelShader = compile ps_5_0 PS_EFFECT_V2(); \
	} \
	pass AlphaNoDepth \
	{ \
		SetRasterizerState(RS_EffectV2); \
		SetDepthStencilState(DSS_ZNone, 0); \
		SetBlendState(BS_EffectV2Alpha, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff); \
		VertexShader = compile vs_5_0 VS_FUNC(); \
		GeometryShader = NULL; \
		PixelShader = compile ps_5_0 PS_EFFECT_V2(); \
	} \
	pass AdditiveNoDepth \
	{ \
		SetRasterizerState(RS_EffectV2); \
		SetDepthStencilState(DSS_ZNone, 0); \
		SetBlendState(BS_EffectV2Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff); \
		VertexShader = compile vs_5_0 VS_FUNC(); \
		GeometryShader = NULL; \
		PixelShader = compile ps_5_0 PS_EFFECT_V2(); \
	} \
	pass Opaque \
	{ \
		SetRasterizerState(RS_Default); \
		SetDepthStencilState(DSS_Default, 0); \
		SetBlendState(BS_EffectV2Opaque, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff); \
		VertexShader = compile vs_5_0 VS_FUNC(); \
		GeometryShader = NULL; \
		PixelShader = compile ps_5_0 PS_EFFECT_V2(); \
	}
