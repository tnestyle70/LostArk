
#include "Engine_Shader_Defines.hlsli"

float4x4    g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
float4   g_vColor = 1.f;

struct VS_IN
{
    float3 vPosition : POSITION;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;    
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;
    
    
    matrix      matWV, matWVP;
    
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix); 
    
    Out.vPosition = mul(vector(In.vPosition, 1.f), matWVP);
  
    return Out;
}



struct PS_IN
{
    float4 vPosition : SV_POSITION;
  
};

struct PS_OUT
{
    float4 vColor : SV_TARGET0;
};

/* 픽셀 셰이더 */ 
/* 전달받은 픽셀의 정보를 바탕으로하여 픽셀의 색을 결정한다 */ 

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out;
    
    Out.vColor = g_vColor;  
    
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



