
#include "Engine_Shader_Defines.hlsli"

float4x4    g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
texture2D   g_Texture;
texture2D g_DepthTexture;

/* CUI_Sprite's own runtime tint/flip -- default values are this shader's own fallback for a
consumer that never binds them (every existing DefaultTechnique/Effect pass draws unaffected),
not a contract callers must always set. */
float4 g_TintColor = float4(1.f, 1.f, 1.f, 1.f);
bool g_FlipX = false;

/* LinearSampler's AddressU/V = WRAP is correct for tiled 3D world textures but wrong here:
this shader only backs CUI_Sprite's screen-space UI quads, whose texcoords span exactly
0..1. With WRAP, bilinear filtering near v=0/v=1 blends in a few texels from the texture's
opposite edge, showing as a thin seam of that edge's color across the sprite's boundary --
visible as a solid line where a gradient texture (e.g. a vignette fading from opaque at one
edge to transparent at the other) has a stark edge-to-edge color difference. CLAMP holds the
edge texel instead of wrapping. */
sampler UISampler = sampler_state
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};



/* �������̴� */ 
/* ���������� �ʼ����� ��ȯ�۾� */ 
struct VS_IN
{
    float3 vPosition : POSITION;
    float2 vTexcoord : TEXCOORD0;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float4 vWordlPos : TEXCOORD1;
    float4 vProjPos : TEXCOORD2;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out;
    
    
    matrix      matWV, matWVP;
    
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix); 
    
    Out.vPosition = mul(vector(In.vPosition, 1.f), matWVP);
    Out.vTexcoord = In.vTexcoord;
    Out.vWordlPos = mul(vector(In.vPosition, 1.f), g_WorldMatrix);
    Out.vProjPos = Out.vPosition;
    
    return Out;
}

/* ������ȯ (Z������) */
/* ����Ʈ ��ȯ(��������ǥ�� ��ȯ) */ 
/* �����Ͷ�����(�ȼ��� ������ �����Ѵ�.) */ 

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float4 vWordlPos : TEXCOORD1;
    float4 vProjPos : TEXCOORD2;
};

struct PS_OUT
{
    float4 vColor : SV_TARGET0;
};

/* �ȼ� ���̴� */ 
/* ���޹��� �ȼ��� ������ ���������Ͽ� �ȼ��� ���� �����Ѵ� */ 

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out;

    Out.vColor = g_Texture.Sample(UISampler, In.vTexcoord);

    return Out;
}

/* CUI_Sprite's own pass -- g_TintColor/g_FlipX default to white/false, so an existing caller
that never binds them (the loading background, progress bar fills) samples identically to
PS_MAIN above. */
PS_OUT PS_MAIN_UI(PS_IN In)
{
    PS_OUT Out;

    float2 vTexcoord = In.vTexcoord;
    if (g_FlipX)
        vTexcoord.x = 1.f - vTexcoord.x;

    Out.vColor = g_Texture.Sample(UISampler, vTexcoord) * g_TintColor;

    return Out;
}



PS_OUT PS_MAIN_SOFTEFFECT(PS_IN In)
{
    PS_OUT Out;
    
    Out.vColor = g_Texture.Sample(LinearSampler, In.vTexcoord);  
    
    ;
    
    float2 vTexcoord;
    
    vTexcoord.x = (In.vProjPos.x / In.vProjPos.w) * 0.5f + 0.5f;
    vTexcoord.y = (In.vProjPos.y / In.vProjPos.w) * -0.5f + 0.5f;
    
    vector      vDepthDesc = g_DepthTexture.Sample(LinearSampler, vTexcoord);
    
    float       fOldZ = vDepthDesc.y * 1000.f;
    
    Out.vColor.a = Out.vColor.a * saturate(fOldZ - In.vProjPos.w);
    
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

    pass Effect
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_SOFTEFFECT();
    }

    /* Plain alpha-blended textured quad, no depth-based soft fade -- for screen-space UI
    sprites (CUI_Sprite) that have no depth texture to sample. */
    pass UIBlend
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_UI();
    }

    /* Same as UIBlend but additive -- for a UI sprite authored with a Scaleform additive
    blendMode (a glow/burst layer whose backing would otherwise show as an opaque box under
    normal alpha blend). */
    pass UIBlendAdditive
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_UI();
    }
}



