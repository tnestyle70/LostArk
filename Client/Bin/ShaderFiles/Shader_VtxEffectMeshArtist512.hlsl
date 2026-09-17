#define EFFECT_SHADER_FAMILY 7
#define EFFECT_NATIVE_PROFILE_GROUP 512
#define EFFECT_NATIVE_MESH_CARRIER 1
#include "Shader_EffectCommon.hlsli"
#define EFFECT_NATIVE_CARRIER_COMMON_INCLUDED 1
#include "Shader_EffectSceneDepthInput.hlsli"
#include "Shader_EffectSceneColorInput.hlsli"
#define EFFECT_ARTIST_NATIVE_PROGRAMS_EXTERNAL 1
#include "Shader_EffectArtistNative.hlsli"
#ifndef EFFECT_NATIVE_DECLARATIONS_ONLY
#include "Shader_EffectArtistNativeSelectedGroup512.hlsli"
#endif
#include "Shader_EffectMeshFamilyCarrier.hlsli"
