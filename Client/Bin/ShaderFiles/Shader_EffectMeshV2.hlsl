#include "Shader_EffectV2_Common.hlsli"

struct VS_IN
{
	float3 vPosition : POSITION;
	float3 vNormal : NORMAL;
	float3 vTangent : TANGENT;
	float3 vBinormal : BINORMAL;
	float2 vTexcoord : TEXCOORD0;
};

PS_EFFECT_IN VS_MAIN(VS_IN input)
{
	PS_EFFECT_IN output;
	const float4 worldPosition = mul(float4(input.vPosition, 1.f), g_WorldMatrix);
	output.vPosition = mul(mul(worldPosition, g_ViewMatrix), g_ProjMatrix);
	output.vProjPos = output.vPosition;
	output.vTexcoord = input.vTexcoord;
	output.vWorldNormal = normalize(mul(input.vNormal, (float3x3)g_WorldMatrix));
	output.vWorldPosition = worldPosition.xyz;
	output.vInstanceColor = float4(1.f, 1.f, 1.f, 1.f);
	return output;
}

PS_EFFECT_IN VS_OUTLINE(VS_IN input)
{
	PS_EFFECT_IN output;
	float4 worldPosition = mul(float4(input.vPosition, 1.f), g_WorldMatrix);
	const float3 worldNormal = normalize(mul(input.vNormal, (float3x3)g_WorldMatrix));
	worldPosition.xyz += worldNormal * g_OutlineWidth;
	output.vPosition = mul(mul(worldPosition, g_ViewMatrix), g_ProjMatrix);
	output.vProjPos = output.vPosition;
	output.vTexcoord = input.vTexcoord;
	output.vWorldNormal = worldNormal;
	output.vWorldPosition = worldPosition.xyz;
	output.vInstanceColor = float4(1.f, 1.f, 1.f, 1.f);
	return output;
}

technique11 DefaultTechnique
{
	EFFECT_V2_PASSES(VS_MAIN)
	EFFECT_V2_OUTLINE_PASS(VS_OUTLINE)
}
