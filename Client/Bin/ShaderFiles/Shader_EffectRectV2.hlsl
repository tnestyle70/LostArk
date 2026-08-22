#include "Shader_EffectV2_Common.hlsli"

struct VS_IN
{
	float3 vPosition : POSITION;
	float2 vTexcoord : TEXCOORD0;
};

PS_EFFECT_IN VS_MAIN(VS_IN input)
{
	PS_EFFECT_IN output;
	const float4 worldPosition = mul(float4(input.vPosition, 1.f), g_WorldMatrix);
	output.vPosition = mul(mul(worldPosition, g_ViewMatrix), g_ProjMatrix);
	output.vTexcoord = input.vTexcoord;
	output.vWorldNormal = float3(0.f, 0.f, 0.f);
	output.vWorldPosition = worldPosition.xyz;
	return output;
}

technique11 DefaultTechnique
{
	EFFECT_V2_PASSES(VS_MAIN)
}
