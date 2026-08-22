#include "Shader_Ue3Glasshole02.hlsli"

/*
 * Runtime carrier for the translated UE3 Glasshole02 RT0 equation.
 *
 * The particle renderer already folds OffsetCenter into the instance World
 * matrix and represents PSA_Rectangle with independent World X/Y scale.  This
 * vertex shader therefore consumes the engine VTXEFFECT_PARTICLE stream
 * without applying a second pivot or forcing square dimensions.
 *
 * Ue3Glasshole02MaterialConstants remains b0, the source scene constants
 * remain b2, and FX11 assigns these two matrices to the otherwise free globals
 * buffer.  The pixel equation and t0..t7/s0..s7 declarations live in the
 * verified Shader_Ue3Glasshole02 translation include.
 */
float4x4 g_ViewMatrix;
float4x4 g_ProjMatrix;
float g_Glasshole02LocalTimeSeconds = 0.0f;

struct GLASSHOLE02_VS_IN
{
    float3 position : POSITION;
    float2 uv : TEXCOORD0;
    float4 world0 : WORLD0;
    float4 world1 : WORLD1;
    float4 world2 : WORLD2;
    float4 world3 : WORLD3;
    float4 color : COLOR0;
    float4 dynamicParameter : DYNAMIC0;
    float4 uvTransform : UVTRANSFORM0;
    float4 uvTransformNext : UVTRANSFORM1;
    float2 particleData : PARTICLEDATA0;
};

struct GLASSHOLE02_VS_OUT
{
    float4 tangentToWorld0 : TEXCOORD10;
    float4 tangentToWorld1 : TEXCOORD11;
    float4 particleUv : TEXCOORD0;
    float4 particleColor : TEXCOORD1;
    float4 dynamicParameter : TEXCOORD2;
    float4 fogAndSelection : TEXCOORD4;
    float4 cameraToParticle : TEXCOORD6;
    float4 screenPosition : TEXCOORD5;
    float4 position : SV_POSITION;
};

float3 NormalizeGlasshole02Axis(float3 axis, float3 fallbackAxis)
{
    const float lengthSquared = dot(axis, axis);
    return lengthSquared > 1.0e-12f ?
        axis * rsqrt(lengthSquared) : fallbackAxis;
}

GLASSHOLE02_VS_OUT VS_MAIN(GLASSHOLE02_VS_IN input)
{
    GLASSHOLE02_VS_OUT output;
    const float4x4 world = float4x4(
        input.world0, input.world1, input.world2, input.world3);
    const float4 worldPosition = mul(float4(input.position, 1.0f), world);
    const float4 viewPosition = mul(worldPosition, g_ViewMatrix);
    const float4 clipPosition = mul(viewPosition, g_ProjMatrix);

    /* Normalize only the orientation carrier.  Position retains the separate
       rectangle X/Y scale already authored into the instance matrix. */
    const float3 worldAxisX = NormalizeGlasshole02Axis(
        input.world0.xyz, float3(1.0f, 0.0f, 0.0f));
    const float3 worldAxisY = NormalizeGlasshole02Axis(
        input.world1.xyz, float3(0.0f, 1.0f, 0.0f));
    const float3 worldAxisZ = NormalizeGlasshole02Axis(
        input.world2.xyz, float3(0.0f, 0.0f, 1.0f));
    const float3 viewAxisX = NormalizeGlasshole02Axis(
        mul(float4(worldAxisX, 0.0f), g_ViewMatrix).xyz,
        float3(1.0f, 0.0f, 0.0f));
    const float3 viewAxisY = NormalizeGlasshole02Axis(
        mul(float4(worldAxisY, 0.0f), g_ViewMatrix).xyz,
        float3(0.0f, 1.0f, 0.0f));
    const float3 viewAxisZ = NormalizeGlasshole02Axis(
        mul(float4(worldAxisZ, 0.0f), g_ViewMatrix).xyz,
        float3(0.0f, 0.0f, 1.0f));
    const float handedness =
        dot(cross(viewAxisX, viewAxisY), viewAxisZ) >= 0.0f ? 1.0f : -1.0f;

    output.tangentToWorld0 = float4(
        viewAxisX.x, viewAxisY.x, viewAxisZ.x, 0.0f);
    output.tangentToWorld1 = float4(
        viewAxisX.z, viewAxisY.z, viewAxisZ.z, handedness);
    output.particleUv = float4(
        input.uv * input.uvTransform.xy + input.uvTransform.zw,
        input.particleData);
    output.particleColor = input.color;
    output.dynamicParameter = input.dynamicParameter;
    output.fogAndSelection = float4(0.0f, 0.0f, 0.0f, 1.0f);

    /* UE3's source VS projects (cameraWorld - vertexWorld) into its sprite
       frame.  Camera origin is zero in view space, so this is the same vector
       without introducing another runtime camera-position constant. */
    const float3 particleToCameraView = -viewPosition.xyz;
    output.cameraToParticle = float4(
        dot(viewAxisX, particleToCameraView),
        dot(viewAxisY, particleToCameraView),
        dot(viewAxisZ, particleToCameraView),
        1.0f);
    output.screenPosition = clipPosition;
    output.position = clipPosition;
    return output;
}

float4 PS_MAIN(Ue3Glasshole02PixelInput input) : SV_Target0
{
    /* The renderer must evaluate the source uniform-expression AST and bind
       CB0 for this same nonnegative local time.  Consuming the anchor here
       makes a static time-zero CB upload detectable at the runtime seam while
       leaving the translated material equation itself unchanged. */
    clip(g_Glasshole02LocalTimeSeconds);
    /* g_Ue3Glasshole02SceneDepth is bound to Target_Depth.  Its .x lane is
       source NDC depth; CB2[1]=(0,0,1/P43,P33/P43) reconstructs the same view
       depth carried by Target_Depth.y * 1000 without changing the DXBC math. */
    /* Calling the verified full evaluator and selecting only sceneColor lets
       the compiler remove the source BasePass MRT2/3/4/5 tail while preserving
       the instruction order of the RT0 equation. */
    return EvaluateUe3Glasshole02(input).sceneColor;
}

RasterizerState RS_Glasshole02TwoSided
{
    FillMode = Solid;
    CullMode = None;
    FrontCounterClockwise = false;
};

DepthStencilState DSS_Glasshole02ReadOnly
{
    DepthEnable = true;
    DepthWriteMask = zero;
    DepthFunc = less_equal;
};

BlendState BS_Glasshole02Alpha
{
    BlendEnable[0] = true;
    SrcBlend = Src_Alpha;
    DestBlend = Inv_Src_Alpha;
    BlendOp = Add;
    SrcBlendAlpha = One;
    DestBlendAlpha = Inv_Src_Alpha;
    BlendOpAlpha = Add;
    RenderTargetWriteMask[0] = 0x0f;
};

technique11 DefaultTechnique
{
    pass Glasshole02AlphaTwoSidedDepthRead
    {
        SetRasterizerState(RS_Glasshole02TwoSided);
        SetDepthStencilState(DSS_Glasshole02ReadOnly, 0);
        SetBlendState(
            BS_Glasshole02Alpha, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
}
