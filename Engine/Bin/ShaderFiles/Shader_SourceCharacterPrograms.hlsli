// CMaterial이 바인딩한 program·register 상수·texture를 native base/light 코드와 연결한다.
// program은 재질이 공유하는 번호이며 캐릭터 class 이름으로 계산을 분기하지 않는다.
#ifndef SOURCE_CHARACTER_PROGRAMS_INCLUDED
#define SOURCE_CHARACTER_PROGRAMS_INCLUDED

uint g_SourceCharacterProgram = 0u;
uint g_SourceCharacterRow = 0u;
float g_SourceCharacterTime = 0.f;
#ifdef SOURCE_CHARACTER_LIGHT_PASS
float4 g_SourceCharacterLightConstants[64];
#else
float4 g_SourceCharacterBaseConstants[64];
#endif
#ifndef SOURCE_CHARACTER_LIGHT_PASS
// hair base pass의 coverage 선택이다. caller가 native masked 상수를 준비하며 지원하지 않는 program의 보조 cutoff는 geometry adapter가 처리한다.
uint g_SourceCharacterHairMaskedPass = 0u;
uint g_SourceCharacterEnvironmentEnabled = 0u;
TextureCube g_SourceCharacterEnvironmentCube;
float4 g_SourceCharacterEnvironmentColor = float4(1.f, 1.f, 1.f, 0.f);
float4 g_SourceCharacterEnvironmentRotation = float4(0.f, 1.f, 1.f, 0.f);
#endif
Texture2D g_SourceCharacterTexture0;
Texture2D g_SourceCharacterTexture1;
Texture2D g_SourceCharacterTexture2;
Texture2D g_SourceCharacterTexture3;
Texture2D g_SourceCharacterTexture4;
Texture2D g_SourceCharacterTexture5;
Texture2D g_SourceCharacterTexture6;
Texture2D g_SourceCharacterTexture7;
Texture2D g_SourceCharacterTexture8;
Texture2D g_SourceCharacterTexture9;
Texture2D g_SourceCharacterTexture10;
Texture2D g_SourceCharacterTexture11;
Texture2D g_SourceCharacterTexture12;
Texture2D g_SourceCharacterTexture13;
Texture2D g_SourceCharacterTexture14;
Texture2D g_SourceCharacterTexture15;
SamplerState SourceCharacterSampler
{
    Filter = ANISOTROPIC;
    MaxAnisotropy = 16;
    AddressU = Wrap; AddressV = Wrap; AddressW = Wrap;
};
SamplerState SourceCharacterStampSampler
{
    Filter = ANISOTROPIC;
    MaxAnisotropy = 16;
    AddressU = Clamp; AddressV = Clamp; AddressW = Clamp;
};
SamplerState SourceCharacterLookupSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp; AddressV = Clamp; AddressW = Clamp;
};
SamplerState SourceMapMonsterStateSampler
{
    Filter = ANISOTROPIC; MaxAnisotropy = 16;
    AddressU = Clamp; AddressV = Wrap; AddressW = Wrap;
};
uint g_SourceMapMonsterBakedEnabled = 0u;
Texture2D g_SourceMapMonsterAverageTexture;
Texture2D g_SourceMapMonsterDirectionalTexture;

struct SOURCE_CHARACTER_NATIVE_INPUT
{
    float3 sourceCameraPosition;
    float4 sourcePrimitiveBounds;
    bool hasBakedLighting;
    float3 bakedAverage;
    float3 bakedCoefficients;
    float4 values[10];
    float4 projection[4];
    float shadow;
    float3 lightColor;
};
struct SOURCE_CHARACTER_NATIVE_OUTPUT
{
    float4 targets[6];
    bool discarded;
};
float4 SourceCharacterAppend(float4 a,float4 b,uint count)
{
    // 생성 입력의 count는 1..4다. 명시적 vector 생성으로 선택되지 않은 b[i-count]의 unsigned underflow 평가를 피한다.
    if (count == 1u) return float4(a.x, b.xyz);
    if (count == 2u) return float4(a.xy, b.xy);
    if (count == 3u) return float4(a.xyz, b.x);
    return a; // count4는 a의 네 성분을 그대로 쓴다.
}
float4 SourceCharacterBitInsert(uint4 width,uint4 offset,uint4 insert,uint4 base)
{
    // DXBC BFI처럼 width·offset의 하위 5bit만 쓴다. width32는 0이 되어 1u<<32를 평가하지 않는다.
    width &= 31u;
    offset &= 31u;
    uint4 mask = ((1u << width) - 1u) << offset;
    return asfloat((base & ~mask) | ((insert << offset) & mask));
}

#ifdef SOURCE_CHARACTER_LIGHT_PASS
#include "Shader_SourceCharacterLightPrograms.hlsli"
#else
#include "Shader_SourceCharacterBasePrograms.hlsli"
#endif
#endif
