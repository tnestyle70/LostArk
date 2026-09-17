
















































































































































































// BEGIN ADDITIONAL ARTIST GROUPS
#include "Shader_EffectArtistNativeGroup1600.hlsli"
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
// BEGIN ADDITIONAL ARTIST CASES
#include "Shader_EffectArtistNativeDispatchAdditionalArtistCases1600.hlsli"
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
    default: clip(-1.f); return 0.f;
    }
}

bool Has_EffectArtistNativeProfile(uint profile)
{
    switch (profile)
    {
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1600u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1601u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1602u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1603u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1604u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1605u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1606u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1607u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1608u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1609u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1610u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1611u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1612u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1613u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1614u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1615u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1616u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1617u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1618u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1619u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1620u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1621u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1622u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1623u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1624u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1625u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1626u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1627u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1628u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1629u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1630u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1631u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1632u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1633u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1634u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1635u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1636u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1637u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1638u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1639u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1640u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1641u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1642u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1643u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1644u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1645u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1646u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1647u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1648u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1649u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1650u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1651u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1652u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1653u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1654u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1655u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1656u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1657u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1658u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1659u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1660u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1661u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1662u: return true;
#endif
#endif
#if !defined(EFFECT_NATIVE_MESH_CARRIER) && !defined(EFFECT_NATIVE_SCREEN_POST_CARRIER)
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
    case 1663u: return true;
#endif
#endif
    default: return false;
    }
}
