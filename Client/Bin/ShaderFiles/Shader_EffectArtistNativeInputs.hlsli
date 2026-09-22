#ifndef ARTIST_NATIVE_MODEL_ONLY
#include "Shader_EffectSceneDepthInput.hlsli"
#include "Shader_EffectSceneColorInput.hlsli"
#include "Shader_EffectNativeScreenUV.hlsli"
#endif

float4 g_ArtistSourceMaterialParameters[32];
float g_ArtistSourceMaterialTime = 0.f;
float4 g_KoukuSourceAmbient;
// Native2893 lit paper mesh only. Capacity matches CLight_Manager::Replace_SceneLights (16).
uint g_KoukuDoveDirectionalCount = 0u;
float4 g_KoukuDoveDirectionalDirections[16];
float4 g_KoukuDoveDirectionalColors[16];
float4 g_KoukuSourceActorPosition;
float4x4 g_KoukuSourceProjection;
float4 g_ArtistSourceMacroUV;
float4 g_ArtistSourceWorldToLocal[3];
float4 g_ArtistSourceLocalToWorld[3];
float4 g_ArtistSourceWorldToView[3];

