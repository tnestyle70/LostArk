
#include "Engine_Shader_Defines.hlsli"

float4x4    g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
float4x4    g_ViewMatrixInverse, g_ProjMatrixInverse;
float4x4    g_LightViewMatrix, g_LightProjMatrix;
texture2D   g_Texture;
texture2D   g_DiffuseTexture, g_ShadeTexture;
texture2D   g_DepthTexture;
texture2D   g_SpecularTexture;
texture2D   g_LightDepthTexture;

vector      g_vCamPosition;



vector      g_vLightDir;
vector      g_vLightPos;
float       g_fLightRange;
vector      g_vLightDiffuse;
vector      g_vLightAmbient;

vector      g_vMtrlAmbient = 1.f;

texture2D   g_NormalTexture;

struct VS_IN
{
    float3 vPosition : POSITION;
    float2 vTexcoord : TEXCOORD0;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;    
    float2 vTexcoord : TEXCOORD0;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;
    
    
    matrix      matWV, matWVP;
    
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix); 
    
    Out.vPosition = mul(vector(In.vPosition, 1.f), matWVP);
    Out.vTexcoord = In.vTexcoord;
  
    return Out;
}



struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
};

struct PS_OUT_BACKBUFFER
{
    float4 vBackBuffer : SV_TARGET0;
};

struct PS_OUT_LIGHT
{
    float4 vShade : SV_TARGET0;
    float4 vSpecular : SV_TARGET1;
};

/* 픽셀 셰이더 */ 
/* 전달받은 픽셀의 정보를 바탕으로하여 픽셀의 색을 결정한다 */ 

PS_OUT_BACKBUFFER PS_MAIN_DEBUG(PS_IN In)
{
    PS_OUT_BACKBUFFER Out;
    
    Out.vBackBuffer = g_Texture.Sample(LinearSampler, In.vTexcoord);
    
    return Out;    
}

PS_OUT_LIGHT PS_MAIN_DIRECTIONAL(PS_IN In)
{
    PS_OUT_LIGHT Out;
    
    vector          vNormalDesc = g_NormalTexture.Sample(LinearSampler, In.vTexcoord);
    
    /* 0 ~ 1 => -1 ~ 1 */
    vector vNormal = vector(vNormalDesc.xyz * 2.f - 1.f, 0.f);    
    

    
    Out.vShade = g_vLightDiffuse * (saturate(dot(normalize(g_vLightDir) * -1.f, normalize(vNormal)))
    + (g_vLightAmbient * g_vMtrlAmbient));
    
    
    vector vReflect = reflect(g_vLightDir, vNormal);
    
    
    vector      vDepthDesc = g_DepthTexture.Sample(LinearSampler, In.vTexcoord);
    float fViewZ = vDepthDesc.y * 1000.f;
    
    vector vWorldPos;
    
    /* 로컬위치 * 월드행렬 * 뷰행렬 * 투영행렬 / w -> 투영공간상의 위치부터 구한다. */ 
    vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
    vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
    vWorldPos.z = vDepthDesc.x;
    vWorldPos.w = 1.f;
    
    /* 로컬위치 * 월드행렬 * 뷰행렬 * 투영행렬 */ 
    vWorldPos = vWorldPos * fViewZ;
    
    /* 로컬위치 * 월드행렬 * 뷰행렬 */ 
    vWorldPos = mul(vWorldPos, g_ProjMatrixInverse);
    
    /* 로컬위치 * 월드행렬  */ 
    vWorldPos = mul(vWorldPos, g_ViewMatrixInverse);
    
    vector vLook = vWorldPos - g_vCamPosition;
    
    Out.vSpecular = pow(saturate(dot(normalize(vReflect) * -1.f, normalize(vLook))), 50.f);    
    
    return Out;
}

PS_OUT_LIGHT PS_MAIN_POINT(PS_IN In)
{
    PS_OUT_LIGHT Out;
    
    vector vNormalDesc = g_NormalTexture.Sample(LinearSampler, In.vTexcoord);
    
    /* 0 ~ 1 => -1 ~ 1 */
    vector vNormal = vector(vNormalDesc.xyz * 2.f - 1.f, 0.f);
    
    
    vector vDepthDesc = g_DepthTexture.Sample(LinearSampler, In.vTexcoord);
    float fViewZ = vDepthDesc.y * 1000.f;
    
    vector vWorldPos;
    
    /* 로컬위치 * 월드행렬 * 뷰행렬 * 투영행렬 / w -> 투영공간상의 위치부터 구한다. */ 
    vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
    vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
    vWorldPos.z = vDepthDesc.x;
    vWorldPos.w = 1.f;
    
    /* 로컬위치 * 월드행렬 * 뷰행렬 * 투영행렬 */ 
    vWorldPos = vWorldPos * fViewZ;
    
    /* 로컬위치 * 월드행렬 * 뷰행렬 */ 
    vWorldPos = mul(vWorldPos, g_ProjMatrixInverse);
    
    /* 로컬위치 * 월드행렬  */ 
    vWorldPos = mul(vWorldPos, g_ViewMatrixInverse);
    
    
    vector vLightDir = vWorldPos - g_vLightPos;
    
    float fAtt = saturate((g_fLightRange - length(vLightDir)) / g_fLightRange);
    
    Out.vShade = (g_vLightDiffuse * (saturate(dot(normalize(vLightDir) * -1.f, normalize(vNormal)))
    + (g_vLightAmbient * g_vMtrlAmbient))) * fAtt;
    
    
    vector vReflect = reflect(vLightDir, vNormal);
    
    
    vector vLook = vWorldPos - g_vCamPosition;
    
    Out.vSpecular = pow(saturate(dot(normalize(vReflect) * -1.f, normalize(vLook))), 50.f) * fAtt;
    
    return Out;
}



PS_OUT_BACKBUFFER PS_MAIN_COMBINED(PS_IN In)
{
    PS_OUT_BACKBUFFER Out;
    
    vector vDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    if (0.f == vDiffuse.a)
        discard;
    
    vector vShade = g_ShadeTexture.Sample(LinearSampler, In.vTexcoord);
    
    vector vSpecular = g_SpecularTexture.Sample(LinearSampler, In.vTexcoord);
    
    
    Out.vBackBuffer = vDiffuse * vShade + vSpecular;
    
    vector vDepthDesc = g_DepthTexture.Sample(LinearSampler, In.vTexcoord);
    float fViewZ = vDepthDesc.y * 1000.f;
    
    vector vWorldPos;
    
    /* 로컬위치 * 월드행렬 * 뷰행렬 * 투영행렬 / w -> 투영공간상의 위치부터 구한다. */ 
    vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
    vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
    vWorldPos.z = vDepthDesc.x;
    vWorldPos.w = 1.f;
    
    /* 로컬위치 * 월드행렬 * 뷰행렬 * 투영행렬 */ 
    vWorldPos = vWorldPos * fViewZ;
    
    /* 로컬위치 * 월드행렬 * 뷰행렬 */ 
    vWorldPos = mul(vWorldPos, g_ProjMatrixInverse);
    
    /* 로컬위치 * 월드행렬  */ 
    vWorldPos = mul(vWorldPos, g_ViewMatrixInverse);
    
    vWorldPos = mul(vWorldPos, g_LightViewMatrix);
    vWorldPos = mul(vWorldPos, g_LightProjMatrix);
    
    float2 vTexcoord;
    
    /* -1, 1 ~ 1, -1 */ 
    /*  0, 0 ~ 1,  1 */
    
    vTexcoord.x = (vWorldPos.x / vWorldPos.w) * 0.5f + 0.5f;
    vTexcoord.y = (vWorldPos.y / vWorldPos.w) * -0.5f + 0.5f;
    
    float4      vOldZ = g_LightDepthTexture.Sample(LinearSampler, vTexcoord);
       
    
    // if (현재 그리는 픽셀의 광원기준 깊이가 >= 이전에 광원기준으로 그려져있던 픽셀의 깊이보다. )
    if (vWorldPos.w - 0.1f >= vOldZ.x * 1000.f)
    {
        Out.vBackBuffer = Out.vBackBuffer * 0.3f;

    }
    
    return Out;
}


technique11 DefaultTechnique
{
    pass Debug
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();        
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DEBUG();
    }   

    pass Directional
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Blend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DIRECTIONAL();
    }

    pass Point
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Blend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_POINT();
    }

    pass Combined
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ZNone, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_COMBINED();
    }


}



