#ifndef LOSTARK_EFFECT_RUNTIME_MATERIAL_PACKET_HLSLI
#define LOSTARK_EFFECT_RUNTIME_MATERIAL_PACKET_HLSLI

// typed Effect evaluator가 공유하는 uniform과 sampler 선언이다. 어느 class의 스킬인지로 texture 의미를 추론하지 않는다.
// C++ ResourceStaging이 저작 Execution을 검증/pack하고 MaterialBinding이 아래 이름으로 Bind_RawValue한다.
// 이 파일 자체에는 색/coverage 수식이나 geometry/VF 선택이 없다. 실제 evaluator와 carrier가 별도로 소비한다.
// Effects 전역 uniform이다. 이 파일은 b register나 C++ struct 전체 memcpy 배치를 지정하지 않는다.
// Enabled/opcode는 이미 컴파일된 evaluator 선택과 유효성 검사의 입력이며 런타임 HLSL 컴파일 명령이 아니다.
uint g_RuntimeMaterialV2Enabled = 0u;
uint g_RuntimeMaterialV2Opcode = 0u;
// texture lane의 개수와 존재 bit다. SRV 선언/실제 texture 바인딩은 별도이며 여기에는 sampler만 선언한다.
uint g_RuntimeMaterialV2TextureLaneCount = 0u;
uint g_RuntimeMaterialV2TextureMask = 0u;
// DynamicParameter와 ParticleColor의 어떤 성분을 소비/억제하는지 나타낸다. 구체 bit 의미는 선택한 opcode/layout이 정한다.
uint g_RuntimeMaterialV2DynamicConsumedMask = 0u;
uint g_RuntimeMaterialV2DynamicSuppressedMask = 0u;
uint g_RuntimeMaterialV2ParticleColorPolicy = 0u;
uint g_RuntimeMaterialV2ParticleColorConsumedMask = 0u;
uint g_RuntimeMaterialV2ParticleColorSuppressedMask = 0u;
uint g_RuntimeMaterialV2ScalarCount = 0u;
uint g_RuntimeMaterialV2VectorCount = 0u;
// 입력 사용 계약을 검사하는 개수와 bit 집합이다. uint2는 두 32-bit 구간을 보관하며 임의 HLSL instruction 목록이 아니다.
uint g_RuntimeMaterialV2InputCount = 0u;
uint2 g_RuntimeMaterialV2InputConsumedMask = uint2(0u, 0u);
uint2 g_RuntimeMaterialV2InputSuppressedMask = uint2(0u, 0u);
// 저작 static 선택과 소비/억제 기록이다. 이 uniform을 쓴다는 사실만으로 새 shader permutation이 런타임 생성되지는 않는다.
uint g_RuntimeMaterialV2StaticInputCount = 0u;
uint g_RuntimeMaterialV2StaticSelectedMask = 0u;
uint g_RuntimeMaterialV2StaticConsumedMask = 0u;
uint g_RuntimeMaterialV2StaticSuppressedMask = 0u;
// render 입력의 소비/억제 계약을 검사한다. 실제 blend/depth/rasterizer state 적용은 C++ adapter와 shader pass가 맡는다.
uint g_RuntimeMaterialV2RenderInputCount = 0u;
uint g_RuntimeMaterialV2RenderConsumedMask = 0u;
uint g_RuntimeMaterialV2RenderSuppressedMask = 0u;
// 최대 52개 scalar를 float4 13행에 pack한다. packedIndex / 4가 행, packedIndex % 4가 xyzw 성분이다.
// 값의 단위와 수식 의미는 program별 descriptor가 정하며 모든 행을 공통 색/세기로 해석하지 않는다.
float4 g_RuntimeMaterialV2ScalarBlocks[13];
// 최대 3개 float4와 각 vector 성분의 소비/억제 mask다. C++은 입력 검증 뒤 같은 크기의 배열을 이름별로 업로드한다.
float4 g_RuntimeMaterialV2Vectors[3];
uint3 g_RuntimeMaterialV2VectorComponentConsumedMask = uint3(0u, 0u, 0u);
uint3 g_RuntimeMaterialV2VectorComponentSuppressedMask = uint3(0u, 0u, 0u);
// 재생 측에서 draw마다 전달하는 정규화 수명 입력이다. 초 단위 local time은 별도 g_EffectLocalTime으로 전달한다.
float g_RuntimeMaterialV2NormalizedLife = 0.f;

// 6개 texture lane의 sampler를 PS s5~s10에 고정한다. filter/address는 C++이 Execution에서 검증해 만든 실제 sampler state를 바인딩한다.
SamplerState g_RuntimeMaterialV2Sampler0 : register(s5);
SamplerState g_RuntimeMaterialV2Sampler1 : register(s6);
SamplerState g_RuntimeMaterialV2Sampler2 : register(s7);
SamplerState g_RuntimeMaterialV2Sampler3 : register(s8);
SamplerState g_RuntimeMaterialV2Sampler4 : register(s9);
SamplerState g_RuntimeMaterialV2Sampler5 : register(s10);

#endif
