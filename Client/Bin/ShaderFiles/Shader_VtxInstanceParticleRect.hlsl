
#include "Engine_Shader_Defines.hlsli"

float4x4    g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
texture2D   g_Texture;



/* 정점셰이더 */ 
/* 정점에대한 필수적인 변환작업 */ 
struct VS_IN
{
    float3 vPosition : POSITION;
    float2 vTexcoord : TEXCOORD0;
    
    float4 vRight : TEXCOORD1;
    float4 vUp : TEXCOORD2;
    float4 vLook : TEXCOORD3;
    float4 vTranslation : TEXCOORD4;
    float2 vLifeTime : TEXCOORD5;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float4 vWordlPos : TEXCOORD1;
    float2 vLifeTime : TEXCOORD2;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;
    
    matrix TransformMatrix = float4x4(In.vRight, In.vUp, In.vLook, In.vTranslation);
    
    vector vPosition = mul(vector(In.vPosition, 1.f), TransformMatrix);
    
    
    matrix      matWV, matWVP;
    
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix); 
    
    Out.vPosition = mul(vPosition, matWVP);
    Out.vTexcoord = In.vTexcoord;
    Out.vWordlPos = mul(vPosition, g_WorldMatrix);
    Out.vLifeTime = In.vLifeTime;
    
    return Out;
}

/* 투영변환 (Z나누기) */
/* 뷰포트 변환(윈도우좌표로 변환) */ 
/* 래스터라이즈(픽셀의 정보를 생성한다.) */ 

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float4 vWordlPos : TEXCOORD1;
    float2 vLifeTime : TEXCOORD2;
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
    
    vector      vColor = g_Texture.Sample(LinearSampler, In.vTexcoord);  
    
    if (vColor.a < 0.3f)
        discard;
    
    Out.vColor = 1.f;
    Out.vColor.a = saturate(In.vLifeTime.y - In.vLifeTime.x);
    
    return Out;    
}

technique11 DefaultTechnique
{
    pass DefaultPass
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }    
}



