#include "Engine_Shader_Defines.hlsli"
float4x4    g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

texture2D   g_DiffuseTexture[2];
texture2D   g_MaskTexture;



/* 정점셰이더 */
/* 정점에대한 필수적인 변환작업 */
struct VS_IN
{
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
    float4 vProjPos : TEXCOORD2;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;
    
    
    matrix      matWV, matWVP;
    
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix); 
    
    Out.vPosition = mul(vector(In.vPosition, 1.f), matWVP);
    Out.vNormal = normalize(mul(vector(In.vNormal, 0.f), g_WorldMatrix));
    Out.vTexcoord = In.vTexcoord;
    Out.vWorldPos = mul(vector(In.vPosition, 1.f), g_WorldMatrix);
    Out.vProjPos = Out.vPosition;
    
    return Out;
}

/* 투영변환 (Z나누기) */
/* 뷰포트 변환(윈도우좌표로 변환) */
/* 래스터라이즈(픽셀의 정보를 생성한다.) */

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
    float4 vProjPos : TEXCOORD2;
};

struct PS_OUT
{
    float4 vDiffuse : SV_TARGET0;
    float4 vNormal : SV_TARGET1;
    float4 vDepth : SV_TARGET2;
    float4 vPickPos : SV_TARGET3;
    float4 vEmissive : SV_TARGET4;
};

/* 픽셀 셰이더 */
/* 전달받은 픽셀의 정보를 바탕으로하여 픽셀의 색을 결정한다 */

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out;
    
    vector      vSourMtrlDiffuse = g_DiffuseTexture[0].Sample(LinearSampler, In.vTexcoord * 50.f);
    vector      vDestMtrlDiffuse = g_DiffuseTexture[1].Sample(LinearSampler, In.vTexcoord * 50.f);
    vector vMask = g_MaskTexture.Sample(PointSampler, In.vTexcoord);
    
    vector vMtrlDiffuse = vSourMtrlDiffuse * (1.f - vMask) + vDestMtrlDiffuse * vMask;
    
  
    Out.vDiffuse = vector(vMtrlDiffuse.rgb, 1.f);
    Out.vNormal = vector((In.vNormal.xyz * 0.5f) + 0.5f, 1.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 1000.f, 0.f, 0.f);
    Out.vPickPos = In.vWorldPos;
    Out.vEmissive = 0.f;
   
    
    return Out;    
}

technique11 DefaultTechnique
{
    pass DefaultPass
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }    
}



