
sampler LinearSampler = sampler_state
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = WRAP;
    AddressV = WRAP;
};

sampler PointSampler = sampler_state
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = WRAP;
    AddressV = WRAP;
};

RasterizerState RS_Default
{
    FillMode = Solid;
    CullMode = Back;
    FrontCounterClockwise = false;
};

RasterizerState RS_Wireframe
{
    FillMode = Wireframe;
    CullMode = Back;
    FrontCounterClockwise = false;
};

RasterizerState RS_Cull_CW
{
    FillMode = Solid;
    CullMode = front;
    FrontCounterClockwise = false;
};

RasterizerState RS_Cull_None
{
    FillMode = Solid;
    CullMode = None;
    FrontCounterClockwise = false;
};

DepthStencilState DSS_Default
{
    DepthEnable = true;
    DepthWriteMask = all;
    DepthFunc = less_equal;
};

DepthStencilState DSS_ZNone
{
    DepthEnable = false;
    DepthWriteMask = zero;    
};

DepthStencilState DSS_ReadOnly
{
    DepthEnable = true;
    DepthWriteMask = zero;
    DepthFunc = less_equal;
};

BlendState BS_Default
{
    BlendEnable[0] = false;
};



BlendState BS_AlphaBlend
{
    BlendEnable[0] = true;
    SrcBlend = Src_Alpha;
    DestBlend = Inv_Src_Alpha;
    BlendOp = Add;
};

BlendState BS_Additive
{
    BlendEnable[0] = true;
    SrcBlend = Src_Alpha;
    DestBlend = One;
    BlendOp = Add;
};

BlendState BS_Blend
{
    BlendEnable[0] = true;
    BlendEnable[1] = true;

    SrcBlend = one;
    DestBlend = one;
    BlendOp = Add;

};


// RT0 keeps original HDR radiance. RT2 carries independently weighted bloom.
// Threshold and knee are renderer-owned; intensity is bound by the effect owner.
// A negative shader-only sentinel selects the scene benchmark intensity.
float g_fEffectBloomIntensity = -1.f;
float g_fSceneBloomIntensity = 1.f;
float g_fEffectBloomThreshold = 1.f;
float g_fEffectBloomSoftKnee = 0.5f;

float3 Extract_SceneBloom(float3 radiance)
{
    radiance = min(max(radiance, 0.f), 60000.f);
    const float threshold = max(g_fEffectBloomThreshold, 0.f);
    const float knee = max(g_fEffectBloomSoftKnee, 0.f);
    const float brightness = max(radiance.r, max(radiance.g, radiance.b));
    float soft = clamp(brightness - threshold + knee, 0.f, 2.f * knee);
    soft = soft * soft / (4.f * knee + 0.00001f);
    return radiance * (max(brightness - threshold, soft) / max(brightness, 0.00001f));
}

float4 Write_SceneBloom(float4 sceneColor)
{
    return float4(min(Extract_SceneBloom(sceneColor.rgb) *
        clamp(g_fEffectBloomIntensity >= 0.f ? g_fEffectBloomIntensity :
            g_fSceneBloomIntensity, 0.f, 16.f), 60000.f), sceneColor.a);
}

struct SCENE_COLOR_BLOOM_OUT
{
    float4 SceneColor : SV_TARGET0;
    float4 BloomContribution : SV_TARGET2;
};

SCENE_COLOR_BLOOM_OUT Write_SceneColorAndBloom(float4 color)
{
    SCENE_COLOR_BLOOM_OUT output;
    output.SceneColor = color;
    output.BloomContribution = Write_SceneBloom(color);
    return output;
}

// Mutable only within one pixel-shader invocation, never shared between draws.
static uint g_EffectSceneReadMode = 0u;
static bool g_EffectSceneSampleUsed = false;
Texture2D g_EffectStartingSceneBloomTexture;

// Auxiliary bloom evaluations reuse material coverage but must not discard the
// original HDR fragment when their zero-radiance baseline changes a clip value.
void Effect_Clip(float value) { if (g_EffectSceneReadMode == 0u) clip(value); }
void Effect_Clip(float2 value) { if (g_EffectSceneReadMode == 0u) clip(value); }
void Effect_Clip(float3 value) { if (g_EffectSceneReadMode == 0u) clip(value); }
void Effect_Clip(float4 value) { if (g_EffectSceneReadMode == 0u) clip(value); }
