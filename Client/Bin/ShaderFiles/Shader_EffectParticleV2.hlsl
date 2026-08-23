#include "Shader_EffectV2_Common.hlsli"

struct VS_IN
{
	float3 vPosition : POSITION;
	float2 vTexcoord : TEXCOORD0;
	float4 vRight : WORLD0;
	float4 vUp : WORLD1;
	float4 vLook : WORLD2;
	float4 vTranslation : WORLD3;
	float4 vColor : COLOR0;
	float4 vDynamicParameter : DYNAMIC0;
	float4 vUVTransform : UVTRANSFORM0;
	float4 vUVTransformNext : UVTRANSFORM1;
	float2 vParticleData : PARTICLEDATA0;
};

PS_EFFECT_IN VS_MAIN(VS_IN input)
{
	PS_EFFECT_IN output;
	const float4x4 instanceWorld = float4x4(
		input.vRight, input.vUp, input.vLook, input.vTranslation);
	const float4 worldPosition = mul(float4(input.vPosition, 1.f), instanceWorld);
	output.vPosition = mul(mul(worldPosition, g_ViewMatrix), g_ProjMatrix);
	output.vTexcoord = input.vTexcoord * input.vUVTransform.xy + input.vUVTransform.zw;
	output.vWorldNormal = float3(0.f, 0.f, 0.f);
	output.vWorldPosition = worldPosition.xyz;
	output.vInstanceColor = input.vColor;
	return output;
}

technique11 DefaultTechnique
{
	EFFECT_V2_PASSES(VS_MAIN)
}
