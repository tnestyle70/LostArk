#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
#include "Shader_EffectArtistNativeGroup448.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP)
#include "Shader_EffectVehicleModelNative.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 512
#include "Shader_EffectArtistNativeGroup512.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 768
#include "Shader_EffectArtistNativeGroup768.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 832
#include "Shader_EffectArtistNativeGroup832.hlsli"
#endif

















































































































































































// BEGIN ADDITIONAL ARTIST GROUPS
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
#include "Shader_EffectArtistNativeGroup1600.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1664
#include "Shader_EffectArtistNativeGroup1664.hlsli"
#endif
// END ADDITIONAL ARTIST GROUPS

// BEGIN WORLD NATIVE GROUP
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2304
#include "Shader_EffectWorldNative.hlsli"
#endif
// END WORLD NATIVE GROUP
// BEGIN KOUKU NATIVE GROUP
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2304 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER))
#include "Shader_EffectKoukuNativeGroup2304.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2368 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER))
#include "Shader_EffectKoukuNativeGroup2368.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2432)
#include "Shader_EffectKoukuNativeGroup2432.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2496 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER))
#include "Shader_EffectKoukuNativeGroup2496.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2560 || defined(EFFECT_NATIVE_DECAL_CARRIER))
#include "Shader_EffectKoukuNativeGroup2560.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2624)
#include "Shader_EffectKoukuNativeGroup2624.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2688 || defined(EFFECT_NATIVE_TRAIL_CARRIER))
#include "Shader_EffectKoukuNativeGroup2688.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2752 || defined(EFFECT_NATIVE_DECAL_CARRIER))
#include "Shader_EffectKoukuNativeGroup2752.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2816 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER))
#include "Shader_EffectKoukuNativeGroup2816.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2880)
#include "Shader_EffectKoukuNativeGroup2880.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2944 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER))
#include "Shader_EffectKoukuNativeGroup2944.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3008 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER))
#include "Shader_EffectKoukuNativeGroup3008.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3072)
#include "Shader_EffectKoukuNativeGroup3072.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3136 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER))
#include "Shader_EffectKoukuNativeGroup3136.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3200 || defined(EFFECT_NATIVE_DECAL_CARRIER))
#include "Shader_EffectKoukuNativeGroup3200.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3264 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER))
#include "Shader_EffectKoukuNativeGroup3264.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3328 || defined(EFFECT_NATIVE_TRAIL_CARRIER))
#include "Shader_EffectKoukuNativeGroup3328.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3584 || defined(EFFECT_NATIVE_DECAL_CARRIER))
#include "Shader_EffectKoukuNativeGroup3584.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3648)
#include "Shader_EffectKoukuNativeGroup3648.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3712 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER))
#include "Shader_EffectKoukuNativeGroup3712.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3776 || defined(EFFECT_NATIVE_TRAIL_CARRIER))
#include "Shader_EffectKoukuNativeGroup3776.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3840 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER))
#include "Shader_EffectKoukuNativeGroup3840.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3904 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER))
#include "Shader_EffectKoukuNativeGroup3904.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3968 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER))
#include "Shader_EffectKoukuNativeGroup3968.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4032 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER))
#include "Shader_EffectKoukuNativeGroup4032.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER))
#include "Shader_EffectKoukuNativeGroup4096.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4160 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER))
#include "Shader_EffectKoukuNativeGroup4160.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER))
#include "Shader_EffectKoukuNativeGroup4224.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4288 || defined(EFFECT_NATIVE_DECAL_CARRIER))
#include "Shader_EffectKoukuNativeGroup4288.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4352)
#include "Shader_EffectKoukuNativeGroup4352.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4416 || defined(EFFECT_NATIVE_DECAL_CARRIER))
#include "Shader_EffectKoukuNativeGroup4416.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4480 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER))
#include "Shader_EffectKoukuNativeGroup4480.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4544 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER))
#include "Shader_EffectKoukuNativeGroup4544.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4608 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER))
#include "Shader_EffectKoukuNativeGroup4608.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4672)
#include "Shader_EffectKoukuNativeGroup4672.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4736)
#include "Shader_EffectKoukuNativeGroup4736.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4800 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER))
#include "Shader_EffectKoukuNativeGroup4800.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4864)
#include "Shader_EffectKoukuNativeGroup4864.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4928)
#include "Shader_EffectKoukuNativeGroup4928.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4992 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER))
#include "Shader_EffectKoukuNativeGroup4992.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 5056 || defined(EFFECT_NATIVE_DECAL_CARRIER))
#include "Shader_EffectKoukuNativeGroup5056.hlsli"
#endif
#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 5120 || defined(EFFECT_NATIVE_TRAIL_CARRIER))
#include "Shader_EffectKoukuNativeGroup5120.hlsli"
#endif
// END KOUKU NATIVE GROUP
#ifndef ARTIST_NATIVE_MODEL_ONLY
EFFECT_PS_OUT Shade_EffectArtistNative(uint profile, ARTIST_NATIVE_INPUT input)
{
    EFFECT_PS_OUT output=(EFFECT_PS_OUT)0;
    float4 nativeColor=0.f;
    bool opaqueCoverage=false;
    switch(profile)
    {
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
#include "Shader_EffectArtistNativeDispatchBase448.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 512
#include "Shader_EffectArtistNativeDispatchBase512.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 768
#include "Shader_EffectArtistNativeDispatchBase768.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 832
#include "Shader_EffectArtistNativeDispatchBase832.hlsli"
#endif
// BEGIN ADDITIONAL ARTIST CASES
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1600
#include "Shader_EffectArtistNativeDispatchAdditionalArtistCases1600.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 1664
#include "Shader_EffectArtistNativeDispatchAdditionalArtistCases1664.hlsli"
#endif
// END ADDITIONAL ARTIST CASES
// BEGIN WORLD NATIVE CASES
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2304
#include "Shader_EffectArtistNativeDispatchWorldNativeCases2304.hlsli"
#endif
// END WORLD NATIVE CASES
// BEGIN KOUKU NATIVE CASES
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2304 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases2304.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2368 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases2368.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2432
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases2432.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2560 || defined(EFFECT_NATIVE_DECAL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases2560.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3584
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases3584.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2688 || defined(EFFECT_NATIVE_TRAIL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases2688.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2752 || defined(EFFECT_NATIVE_DECAL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases2752.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2816 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases2816.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2880
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases2880.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2944 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases2944.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3008 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases3008.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3072
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases3072.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3136 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases3136.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3200 || defined(EFFECT_NATIVE_DECAL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases3200.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3264 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases3264.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3328 || defined(EFFECT_NATIVE_TRAIL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases3328.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3584 || defined(EFFECT_NATIVE_DECAL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases3584Part2.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2496 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases2496.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2624
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases2624.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3648
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases3648.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3712 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases3712.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3776 || defined(EFFECT_NATIVE_TRAIL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases3776.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3840 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases3840.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3904 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases3904.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4480 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases4480.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3968 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases3968.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4032 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases4032.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4096 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases4096.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4160 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases4160.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4224 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases4224.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4288 || defined(EFFECT_NATIVE_DECAL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases4288.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4352
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases4352.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4416 || defined(EFFECT_NATIVE_DECAL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases4416.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4544 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases4544.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4608 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases4608.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4672
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases4672.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4736
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases4736.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4800 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases4800.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4864
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases4864.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4928
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases4928.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 4992 || defined(EFFECT_NATIVE_DECAL_CARRIER) || defined(EFFECT_NATIVE_TRAIL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases4992.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 5056 || defined(EFFECT_NATIVE_DECAL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases5056.hlsli"
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 5120 || defined(EFFECT_NATIVE_TRAIL_CARRIER)
#include "Shader_EffectArtistNativeDispatchKoukuNativeCases5120.hlsli"
#endif
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
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 460u: return ArtistNative460(input);
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 448
    case 461u: return ArtistNative461(input);
#endif
#if !defined(EFFECT_NATIVE_PROFILE_GROUP)
    case 3828u: return ArtistNative3828(input);
    case 3831u: return ArtistNative3831(input);
    case 3832u: return ArtistNative3832(input);
    case 3833u: return ArtistNative3833(input);
#endif
    default: clip(-1.f); return 0.f;
    }
}
