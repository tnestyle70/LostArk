#define EFFECT_SHADER_FAMILY 7
#define EFFECT_NATIVE_PROFILE_GROUP 4736
#define EFFECT_NATIVE_PARTICLE_CARRIER 1
#include "Shader_EffectCommon.hlsli"
#define EFFECT_NATIVE_CARRIER_COMMON_INCLUDED 1
#include "Shader_EffectSceneColorInput.hlsli"
#include "Shader_EffectSceneDepthInput.hlsli"
#define EFFECT_ARTIST_NATIVE_PROGRAMS_EXTERNAL 1
#include "Shader_EffectArtistNative.hlsli"
#ifndef EFFECT_NATIVE_DECLARATIONS_ONLY
#include "Shader_EffectArtistNativeSelectedGroup4736.hlsli"
#endif
#include "Shader_EffectParticleFamilyCarrier.hlsli"
