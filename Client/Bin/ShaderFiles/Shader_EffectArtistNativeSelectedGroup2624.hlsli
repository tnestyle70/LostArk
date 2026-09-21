
















































































































































































// BEGIN ADDITIONAL ARTIST GROUPS
// END ADDITIONAL ARTIST GROUPS

// BEGIN WORLD NATIVE GROUP
// END WORLD NATIVE GROUP
// BEGIN KOUKU NATIVE GROUP
#include "Shader_EffectKoukuNativeGroup2624.hlsli"
// END KOUKU NATIVE GROUP
#ifndef ARTIST_NATIVE_MODEL_ONLY
EFFECT_PS_OUT Shade_EffectArtistNative(uint profile, ARTIST_NATIVE_INPUT input)
{
    EFFECT_PS_OUT output=(EFFECT_PS_OUT)0;
    float4 nativeColor=0.f;
    bool opaqueCoverage=false;
    switch(profile)
    {
// BEGIN ADDITIONAL ARTIST CASES
// END ADDITIONAL ARTIST CASES
// BEGIN WORLD NATIVE CASES
// END WORLD NATIVE CASES
// BEGIN KOUKU NATIVE CASES
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases2624.hlsli"
// END KOUKU NATIVE CASES
    default: clip(-1.f); return output;
    }
    output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity, opaqueCoverage ? 1.f : nativeColor.a);
    output.Distortion=0.f;
    if(g_ColorClip>0.f) clip(output.SceneColor.a-g_ColorClip);
    return output;
}
#endif

float4 Shade_ArtistModelNative(uint profile, ARTIST_NATIVE_INPUT input)
{
    switch(profile)
    {
    default: clip(-1.f); return 0.f;
    }
}

bool Has_EffectArtistNativeProfile(uint profile)
{
    switch (profile)
    {
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2624) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 2638u: return true;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2624) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 2624u: return true;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2624) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 2625u: return true;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2624) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
    case 2626u: return true;
#endif
#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2624) && !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_DECAL_CARRIER) && !defined(EFFECT_NATIVE_TRAIL_CARRIER)
    case 2627u: return true;
#endif
    default: return false;
    }
}
