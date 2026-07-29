#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix;
float4x4 g_ViewMatrix;
float4x4 g_ProjMatrix;

float3 g_CameraPosition;
float3 g_LightDirection;

Texture2D g_DiffuseTexture;
Texture2D g_NormalTexture;
uint g_HasNormalTexture = 0;

RasterizerState RS_Preview
{
	FillMode = Solid;
	CullMode = None;
	FrontCounterClockwise = false;
};

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
	float3 vNormal : NORMAL;
	float3 vTangent : TANGENT;
	float3 vBinormal : BINORMAL;
	float2 vTexcoord : TEXCOORD0;
	float3 vWorldPosition : TEXCOORD1;
};

VS_OUT VS_MAIN(VS_IN input)
{
	VS_OUT output;
	const matrix worldView =
		mul(g_WorldMatrix, g_ViewMatrix);
	const matrix worldViewProjection =
		mul(worldView, g_ProjMatrix);

	output.vPosition = mul(
		float4(input.vPosition, 1.f),
		worldViewProjection);
	output.vNormal = normalize(mul(
		float4(input.vNormal, 0.f),
		g_WorldMatrix).xyz);
	output.vTangent = normalize(mul(
		float4(input.vTangent, 0.f),
		g_WorldMatrix).xyz);
	output.vBinormal = normalize(mul(
		float4(input.vBinormal, 0.f),
		g_WorldMatrix).xyz);
	output.vTexcoord = input.vTexcoord;
	output.vWorldPosition = mul(
		float4(input.vPosition, 1.f),
		g_WorldMatrix).xyz;
	return output;
}

float4 PS_MAIN(VS_OUT input) : SV_TARGET0
{
	const float4 albedo =
		g_DiffuseTexture.Sample(
			LinearSampler,
			input.vTexcoord);
	if (albedo.a < 0.3f)
		discard;

	float3 normal = normalize(input.vNormal);
	if (0 != g_HasNormalTexture)
	{
		const float3 tangentNormal =
			g_NormalTexture.Sample(
				LinearSampler,
				input.vTexcoord).xyz *
			2.f - 1.f;
		const float3x3 tangentToWorld =
			float3x3(
				normalize(input.vTangent),
				normalize(input.vBinormal) * -1.f,
				normal);
		normal = normalize(
			mul(tangentNormal, tangentToWorld));
	}

	const float3 light =
		normalize(-g_LightDirection);
	const float diffuseLight =
		saturate(dot(normal, light));
	const float hemisphere =
		0.38f + saturate(normal.y) * 0.18f;

	const float3 viewDirection =
		normalize(
			g_CameraPosition -
			input.vWorldPosition);
	const float rim = pow(
		1.f - saturate(
			dot(normal, viewDirection)),
		3.f) * 0.12f;

	const float3 color =
		albedo.rgb *
		(hemisphere + diffuseLight * 0.72f) +
		rim;
	return float4(color, albedo.a);
}

technique11 DefaultTechnique
{
	pass PreviewPass
	{
		SetRasterizerState(RS_Preview);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(
			BS_Default,
			float4(0.f, 0.f, 0.f, 0.f),
			0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN();
	}
}
