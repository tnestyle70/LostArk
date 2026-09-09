#ifndef EFFECT_NATIVE_SCREEN_UV_HLSLI
#define EFFECT_NATIVE_SCREEN_UV_HLSLI
// Engine vertex-factory adapter: project coordinates are (UE.x,UE.z,-UE.y).
float2 ALTVNativeScreenUV(float2 pixelPosition)
{
    uint width, height;
    g_EffectSceneDepthTexture.GetDimensions(width, height);
    return pixelPosition / float2(width, height);
}
#endif
