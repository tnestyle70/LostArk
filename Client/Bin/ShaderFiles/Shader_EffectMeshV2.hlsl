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
	const matrix worldViewProjection =
		mul(mul(g_WorldMatrix, g_ViewMatrix), g_ProjMatrix);
	output.vPosition = mul(float4(input.vPosition, 1.f), worldViewProjection);
	output.vTexcoord = input.vTexcoord;
	return output;
}

technique11 DefaultTechnique
{
	EFFECT_V2_PASSES(VS_MAIN)
}
