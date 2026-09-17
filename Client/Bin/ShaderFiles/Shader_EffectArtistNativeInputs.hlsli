#ifndef ARTIST_NATIVE_MODEL_ONLY
#include "Shader_EffectSceneDepthInput.hlsli"
#include "Shader_EffectSceneColorInput.hlsli"
#include "Shader_EffectNativeScreenUV.hlsli"
#endif

float4 g_ArtistSourceMaterialParameters[32];
float g_ArtistSourceMaterialTime = 0.f;
float4 g_KoukuSourceAmbient;
float4 g_KoukuSourceActorPosition;
float4x4 g_KoukuSourceProjection;
float4 g_ArtistSourceMacroUV;
float4 g_ArtistSourceWorldToLocal[3];

