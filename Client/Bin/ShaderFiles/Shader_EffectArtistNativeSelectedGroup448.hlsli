#include "Shader_EffectArtistNativeGroup448.hlsli"

















































































































































































// BEGIN ADDITIONAL ARTIST GROUPS
// END ADDITIONAL ARTIST GROUPS

// BEGIN WORLD NATIVE GROUP
// END WORLD NATIVE GROUP
// BEGIN KOUKU NATIVE GROUP
// END KOUKU NATIVE GROUP
#ifndef ARTIST_NATIVE_MODEL_ONLY
EFFECT_PS_OUT Shade_EffectArtistNative(uint profile, ARTIST_NATIVE_INPUT input)
{
    EFFECT_PS_OUT output=(EFFECT_PS_OUT)0;
    float4 nativeColor=0.f;
    bool opaqueCoverage=false;
    switch(profile)
    {
#include "Shader_EffectArtistNativeDispatchBase448.hlsli"
// BEGIN ADDITIONAL ARTIST CASES
// END ADDITIONAL ARTIST CASES
// BEGIN WORLD NATIVE CASES
// END WORLD NATIVE CASES
// BEGIN KOUKU NATIVE CASES
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
    case 460u: return ArtistNative460(input);
    case 461u: return ArtistNative461(input);
    default: clip(-1.f); return 0.f;
    }
}

bool Has_EffectArtistNativeProfile(uint profile)
{
    switch (profile)
    {
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 462u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 463u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 464u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 465u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 466u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 467u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 468u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 469u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 470u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 471u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 472u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 473u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 474u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 475u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 476u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 477u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 478u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 479u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 480u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 481u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 482u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 483u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 484u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 485u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 486u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 487u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 488u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 489u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 490u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 491u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 492u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 493u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 494u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 495u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 496u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 497u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 498u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 499u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 500u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 501u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 502u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 503u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 504u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 505u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 506u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 507u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 508u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 509u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 510u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 511u: return true;
#endif
#endif
    default: return false;
    }
}
