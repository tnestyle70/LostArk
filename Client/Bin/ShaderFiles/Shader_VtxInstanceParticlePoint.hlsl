
#include "Engine_Shader_Defines.hlsli"

float4x4    g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
texture2D   g_Texture;
vector      g_vCamPosition;



/* 정점셰이더 */ 
/* 정점에대한 필수적인 변환작업 */ 
struct VS_IN
{
    float3 vPosition : POSITION;    
    row_major float4x4 TransformMatrix : WORLD;    
    float2 vLifeTime : TEXCOORD0;
};

struct VS_OUT
{
    float4 vPosition : POSITION;  
    float2 vPSize : PSIZE;
    float2 vLifeTime : TEXCOORD0;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;   
   
    vector vPosition = mul(vector(In.vPosition, 1.f), In.TransformMatrix); 
    
   
    Out.vPosition = mul(vPosition, g_WorldMatrix);    
    Out.vPSize = float2(length(In.TransformMatrix._11_12_13), length(In.TransformMatrix._21_22_23));
    Out.vLifeTime = In.vLifeTime;
    
    return Out;
}

struct GS_IN
{
    float4 vPosition : POSITION;
    float2 vPSize : PSIZE;
    float2 vLifeTime : TEXCOORD0;
};

struct GS_OUT
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;    
    float2 vLifeTime : TEXCOORD1;
};

[maxvertexcount(6)]
void GS_MAIN(point GS_IN In[1], inout TriangleStream<GS_OUT> OutStream)
{
    GS_OUT Out[4];
    
    float4 vLook = g_vCamPosition - In[0].vPosition;
    float3 vRight = normalize(cross(float3(0.f, 1.f, 0.f), vLook.xyz)) * In[0].vPSize.x * 0.5f;
    float3 vUp = normalize(cross(vLook.xyz, vRight)) * In[0].vPSize.y * 0.5f;
    
    matrix matVP = mul(g_ViewMatrix, g_ProjMatrix);
    
    Out[0].vPosition = mul(vector(In[0].vPosition.xyz + vRight + vUp, 1.f), matVP);
    Out[0].vTexcoord = float2(0.f, 0.f);
    Out[0].vLifeTime = In[0].vLifeTime;
    
    Out[1].vPosition = mul(vector(In[0].vPosition.xyz - vRight + vUp, 1.f), matVP);
    Out[1].vTexcoord = float2(1.f, 0.f);
    Out[1].vLifeTime = In[0].vLifeTime;
    
    Out[2].vPosition = mul(vector(In[0].vPosition.xyz - vRight - vUp, 1.f), matVP);
    Out[2].vTexcoord = float2(1.f, 1.f);
    Out[2].vLifeTime = In[0].vLifeTime;
    
    Out[3].vPosition = mul(vector(In[0].vPosition.xyz + vRight - vUp, 1.f), matVP);
    Out[3].vTexcoord = float2(0.f, 1.f);
    Out[3].vLifeTime = In[0].vLifeTime;
    
    
    OutStream.Append(Out[0]);
    OutStream.Append(Out[1]);
    OutStream.Append(Out[2]);
    OutStream.RestartStrip();
    
    OutStream.Append(Out[0]);
    OutStream.Append(Out[2]);
    OutStream.Append(Out[3]);
    OutStream.RestartStrip();
}

/* 투영변환 (Z나누기) */
/* 뷰포트 변환(윈도우좌표로 변환) */ 
/* 래스터라이즈(픽셀의 정보를 생성한다.) */ 

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float2 vLifeTime : TEXCOORD1;
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
        GeometryShader = compile gs_5_0 GS_MAIN();
        PixelShader = compile ps_5_0 PS_MAIN();
    }    
}



