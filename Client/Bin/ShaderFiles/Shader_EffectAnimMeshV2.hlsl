#include "Shader_EffectV2_Common.hlsli"

matrix g_BoneMatrices[512];

struct VS_IN
{
	float3 vPosition : POSITION;
	float3 vNormal : NORMAL;
	float3 vTangent : TANGENT;
	float3 vBinormal : BINORMAL;
	float2 vTexcoord : TEXCOORD0;
	uint4 vBlendIndices : BLENDINDEX;
	float4 vBlendWeights : BLENDWEIGHT;
};

PS_EFFECT_IN VS_MAIN(VS_IN input)
{
	PS_EFFECT_IN output;
	const matrix boneMatrix =
		g_BoneMatrices[input.vBlendIndices.x] * input.vBlendWeights.x +
		g_BoneMatrices[input.vBlendIndices.y] * input.vBlendWeights.y +
		g_BoneMatrices[input.vBlendIndices.z] * input.vBlendWeights.z +
		g_BoneMatrices[input.vBlendIndices.w] * input.vBlendWeights.w;
	const float4 skinnedPosition = mul(float4(input.vPosition, 1.f), boneMatrix);
	const float3 skinnedNormal = mul(float4(input.vNormal, 0.f), boneMatrix).xyz;
	const float4 worldPosition = mul(skinnedPosition, g_WorldMatrix);
	output.vPosition = mul(mul(worldPosition, g_ViewMatrix), g_ProjMatrix);
	output.vTexcoord = input.vTexcoord;
	output.vWorldNormal = normalize(mul(skinnedNormal, (float3x3)g_WorldMatrix));
	output.vWorldPosition = worldPosition.xyz;
	return output;
}

technique11 DefaultTechnique
{
	EFFECT_V2_PASSES(VS_MAIN)
}
