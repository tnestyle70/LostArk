#ifndef EFFECT_SCENE_DEPTH_INPUT_HLSLI
#define EFFECT_SCENE_DEPTH_INPUT_HLSLI

Texture2D g_EffectSceneDepthTexture;
SamplerState EffectSliceDepthSampler
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Clamp;
    AddressV = Clamp;
};

#endif
