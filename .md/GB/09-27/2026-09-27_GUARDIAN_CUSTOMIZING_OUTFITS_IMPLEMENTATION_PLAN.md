# 가디언 나이트 기존 다섯 의상 슬롯 구현 계획

## G00. 현재 실측과 범위

`CustomizingCostumes.json`의 GuardianKnight는 HR00 의상, original00 의상, HR00 무기,
original00 무기 네 행이다. 제품 화면의 의상 셀은 다섯 개이며 두 무기가 같은 줄에 섞여 있다.
선택은 `Wear_CustomizingSet -> CEquipmentPresentationService::Apply_Preview ->
CCharacter::Apply_EquipmentPreview`의 기존 slot 단위 transaction을 사용한다.

실제 원본 package inventory에서 PC_DDK_01/02/03의 upper/lower/arm/shoulder/helmet을
확인했다. PC_DDK_04/05는 자체 mesh가 없는 MIC variant package다. 최종 의상은 현재
class_select_hr00, 기존 original_00, 신규 source_ddk_01/02/03의 다섯 개다. 이는 현재
기본 외형에 대한 네 대안이며 원본에 없는 geometry나 임의 색상 의상을 만들지 않는다.

## G01. 원본 의상과 모델

기존 UModel LostArk v7로 세 package의 다섯 실제 mesh와 의존 재질·texture를 추출한다.
`normalize_character_equipment_gltf.py`가 설치 GuardianKnight의 master skeleton 순서와
inverse bind로 정규화하고 기존 ModelAssetConverter로 cook한다. 실제 positive weight 본,
rest pose, 추가 UV와 named material slot을 검사한다. 신규 모델 stable asset ID는
`Character/GuardianKnight/Equipment/source_ddk_XX/<part>.wmodel`이다.

## G02. 재질과 장착 계약

각 원본 mesh의 실제 MIC와 Base/Light shader·uniform 계약을 추출해 이미 지원하는 exact
SOURCE_CHARACTER family에 연결한다. 원본 texture와 parameter를 기존 CharacterCatalog의
root modelMaterialOverrides에 stable model/material key로 추가한다. body와 눈·머리 재질은
다른 작업 소유이며 변경하지 않는다. 신규 permutation이 있으면 재사용 가능성을 먼저
실측하고 필요한 기존 native renderer 연결을 함께 검증한다.

EquipmentPresentationCatalog에 세 full outfit을 UPPER primary slot과
UPPER/LOWER/HANDS/SHOULDER/HEAD 점유로 등록한다. 무기는 기존 b_wp_1 socket의 두
SOCKETED set을 유지하되 선택 UI에 노출하지 않는다. outfit 선택은 기존 무기를 보존한다.
기본 body의 의상 관련 hidden mesh mask와 실제 교체된 part visibility도 소비자에서 확인한다.

## G03. 제품 선택 UI

사용자 명시 정정에 따라 기존 의상 셀 다섯 개만 사용한다. 이전 행 2/3의 무기 항목을 실제
의상으로 바꾸고 4번 의상을 추가한다. 별도 무기 선택 UI, 슬롯, schema, 저장 field를
추가하지 않는다. 협업 작업자는 기존 costume/icon JSON의 GuardianKnight 배열만 수정하며
다른 class와 기존 C++ 소비자를 유지한다. 신규 C++ 파일·프로젝트 등록은 없다.

## G04. 설치와 전달

후보와 입력 hash를 보관하고 최신 디스크의 무관한 field를 유지한 채 stable ID로 병합한다.
교체 직전 freshness, 백업, atomic replace와 실패 시 자기 변경 rollback을 지킨다.
신규 Resources 및 변경된 runtime resource는 기존 상대 경로 그대로
`C:/Users/user/Desktop/GBResources`에도 전달한다. 설치/전달 hash를 대조한다.

## G05. 검증

JSON parse, catalog class/slot/socket/texture closure, 실제 설치 master palette와 inverse bind,
다섯 의상의 기존 장착 소비자와 슬롯 교체를 headless로 검사한다. 원본 모델과
같은 native material descriptor를 실제 CModel/CMaterial에 전달하는지 확인한다.
통합 담당자가 Debug/Release Product 빌드를 실행한다. Client/UI 자동 실행은 하지 않으며
다섯 의상 선택 뒤 실제 화면의 최종 판정은 사용자 확인으로 남긴다.

## G06. 추가 코드 정본

기존 native1472 cohort와 SourceCharacterMaterialParameters::Configure에 들어가는1526의
완전한 추가 함수·packing은 아래와 같다. 기존 함수와 다른 작업의 native600은 보존한다.
source ID에 한정한 engine BRDF reciprocal finite guard도 generator에서 동일하게 재생성한다.
기존 project/filter cohort를 쓰므로 신규 C++/shader 파일 등록은 없다.

### base1526.hlsli

```hlsl
// source.character.equipment-native-1526.v1 / source program a68bdca4b6e4b34bb222fafbd8cc25bd
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1526(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[15]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[20].w=(g_SourceCharacterTime.xxxx).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0, r16=0.0, r17=0.0, r18=0.0, r19=0.0, r20=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.xyzw, s6, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 2: mul r1.xyzw, r0.xyzw, cb0[16].xyzw
    r1.xyzw = ((r0.xyzw)*(source[16].xyzw)).xyzw;
    // 3: add r0.xy, r0.ywyy, r0.xzxx
    r0.xy = ((r0.ywyy)+(r0.xzxx)).xy;
    // 4: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 5: add r0.yz, r1.yywy, r1.xxzx
    r0.yz = ((r1.yywy)+(r1.xxzx)).yz;
    // 6: add r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)+(r0.yyyy)).y;
    // 7: add r0.y, r0.y, l(-1.000000)
    r0.y = ((r0.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 8: mad_sat r0.x, r0.x, r0.y, l(1.000000)
    r0.x = (saturate((r0.xxxx)*(r0.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t2.xyzw, s1, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 10: mul_sat r0.x, r0.x, r1.w
    r0.x = (saturate((r0.xxxx)*(r1.wwww))).x;
    // 11: add r0.x, r0.x, l(-0.333300)
    r0.x = ((r0.xxxx)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).x;
    // 12: lt r0.x, r0.x, l(0.000000)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 13: discard_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) { output.discarded = true; return output; }
    // 14: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 15: frc r0.x, v4.x
    r0.x = (frac(v4.xxxx)).x;
    // 16: mul r0.x, r0.x, l(0.125000)
    r0.x = ((r0.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 17: mul r2.y, cb0[7].y, cb0[15].y
    r2.y = ((source[7].yyyy)*(source[15].yyyy)).y;
    // 18: mov r0.y, v4.y
    r0.y = (v4.yyyy).y;
    // 19: mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 20: add r0.xy, r0.xyxx, r2.xyxx
    r0.xy = ((r0.xyxx)+(r2.xyxx)).xy;
    // 21: frc r0.z, cb0[7].x
    r0.z = (frac(source[7].xxxx)).z;
    // 22: add r0.w, -r0.z, cb0[7].x
    r0.w = ((-(r0.zzzz))+(source[7].xxxx)).w;
    // 23: mul r2.z, r0.w, l(0.125000)
    r2.z = ((r0.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 24: add r0.xy, r0.xyxx, r2.zwzz
    r0.xy = ((r0.xyxx)+(r2.zwzz)).xy;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r0.xyxx, t5.xyzw, s5, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 26: mul r0.x, r0.z, r2.w
    r0.x = ((r0.zzzz)*(r2.wwww)).x;
    // 27: add r0.y, -cb0[7].w, l(1.000000)
    r0.y = ((-(source[7].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 28: mul r0.y, r0.y, cb0[20].w
    r0.y = ((r0.yyyy)*(source[20].wwww)).y;
    // 29: mul r0.y, r0.y, l(6.283185)
    r0.y = ((r0.yyyy)*(float4(6.283185,6.283185,6.283185,6.283185))).y;
    // 30: sincos r0.y, null, r0.y
    r0.y = (sin(r0.yyyy)).y;
    // 31: add r0.y, r0.y, l(1.000000)
    r0.y = ((r0.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 32: mul r0.z, cb0[7].z, l(1.500000)
    r0.z = ((source[7].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 33: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 34: mad r0.y, r0.y, l(0.500000), cb0[7].z
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[7].zzzz)).y;
    // 35: mul r0.yzw, r2.xxyz, r0.yyyy
    r0.yzw = ((r2.xxyz)*(r0.yyyy)).yzw;
    // 36: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 37: add r2.xyz, -r1.xyzx, r1.wwww
    r2.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 38: mad r2.xyz, cb0[19].xxxx, r2.xyzx, r1.xyzx
    r2.xyz = ((source[19].xxxx)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 39: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 40: add r3.xyz, -r2.xyzx, r1.wwww
    r3.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 41: mad r2.xyz, cb0[19].yyyy, r3.xyzx, r2.xyzx
    r2.xyz = ((source[19].yyyy)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 42: mul r3.xyz, cb0[5].xyzx, cb0[5].wwww
    r3.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 43: max r4.xyz, r3.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r4.xyz = (max(r3.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 44: max r3.xyz, r3.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 45: min r3.xyz, r3.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 46: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 47: add r4.xyz, -r3.xyzx, r4.xyzx
    r4.xyz = ((-(r3.xyzx))+(r4.xyzx)).xyz;
    // 48: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 49: log r6.xyz, |r5.xzyx|
    r6.xyz = (log2(abs(r5.xzyx))).xyz;
    // 50: lt r5.xyz, |r5.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r5.xyz = (asfloat((uint4)((abs(r5.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 51: mul r1.w, r6.y, cb0[18].y
    r1.w = ((r6.yyyy)*(source[18].yyyy)).w;
    // 52: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 53: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 54: movc r1.w, r5.y, l(0), r1.w
    r1.w = ((asuint(r5.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 55: mad r3.xyz, r1.wwww, r4.xyzx, r3.xyzx
    r3.xyz = ((r1.wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 56: mul r4.xyz, cb0[3].xyzx, cb0[3].wwww
    r4.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 57: max r7.xyz, r4.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r7.xyz = (max(r4.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 58: max r4.xyz, r4.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r4.xyz = (max(r4.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 59: min r4.xyz, r4.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 60: min r7.xyz, r7.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r7.xyz = (min(r7.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 61: add r7.xyz, -r4.xyzx, r7.xyzx
    r7.xyz = ((-(r4.xyzx))+(r7.xyzx)).xyz;
    // 62: mad r4.xyz, r1.wwww, r7.xyzx, r4.xyzx
    r4.xyz = ((r1.wwww)*(r7.xyzx)+(r4.xyzx)).xyz;
    // 63: mul r7.xyz, cb0[4].xyzx, cb0[4].wwww
    r7.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 64: max r8.xyz, r7.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r8.xyz = (max(r7.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 65: max r7.xyz, r7.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r7.xyz = (max(r7.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 66: min r7.xyz, r7.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r7.xyz = (min(r7.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 67: min r8.xyz, r8.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r8.xyz = (min(r8.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 68: add r8.xyz, -r7.xyzx, r8.xyzx
    r8.xyz = ((-(r7.xyzx))+(r8.xyzx)).xyz;
    // 69: mad r7.xyz, r1.wwww, r8.xyzx, r7.xyzx
    r7.xyz = ((r1.wwww)*(r8.xyzx)+(r7.xyzx)).xyz;
    // 70: add r8.xyz, -r4.xyzx, r7.xyzx
    r8.xyz = ((-(r4.xyzx))+(r7.xyzx)).xyz;
    // 71: sample_b_indexable(texture2d)(float,float,float,float) r9.xyz, v4.xyxx, t4.xyzw, s3, l(0.000000)
    r9.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 72: mad r4.xyz, r9.xxxx, r8.xyzx, r4.xyzx
    r4.xyz = ((r9.xxxx)*(r8.xyzx)+(r4.xyzx)).xyz;
    // 73: add r8.xyz, r3.xyzx, -r4.xyzx
    r8.xyz = ((r3.xyzx)+(-(r4.xyzx))).xyz;
    // 74: mad r3.xyz, -r9.xxxx, r7.xyzx, r3.xyzx
    r3.xyz = ((-(r9.xxxx))*(r7.xyzx)+(r3.xyzx)).xyz;
    // 75: mul r7.xyz, r7.xyzx, r9.xxxx
    r7.xyz = ((r7.xyzx)*(r9.xxxx)).xyz;
    // 76: mad r3.xyz, r9.yyyy, r3.xyzx, r7.xyzx
    r3.xyz = ((r9.yyyy)*(r3.xyzx)+(r7.xyzx)).xyz;
    // 77: mad r4.xyz, r9.yyyy, r8.xyzx, r4.xyzx
    r4.xyz = ((r9.yyyy)*(r8.xyzx)+(r4.xyzx)).xyz;
    // 78: mul r7.xyz, cb0[6].xyzx, cb0[6].wwww
    r7.xyz = ((source[6].xyzx)*(source[6].wwww)).xyz;
    // 79: max r8.xyz, r7.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r8.xyz = (max(r7.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 80: max r7.xyz, r7.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r7.xyz = (max(r7.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 81: min r7.xyz, r7.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r7.xyz = (min(r7.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 82: min r8.xyz, r8.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r8.xyz = (min(r8.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 83: add r8.xyz, -r7.xyzx, r8.xyzx
    r8.xyz = ((-(r7.xyzx))+(r8.xyzx)).xyz;
    // 84: mad r7.xyz, r1.wwww, r8.xyzx, r7.xyzx
    r7.xyz = ((r1.wwww)*(r8.xyzx)+(r7.xyzx)).xyz;
    // 85: add r8.xyz, -r4.xyzx, r7.xyzx
    r8.xyz = ((-(r4.xyzx))+(r7.xyzx)).xyz;
    // 86: mad r4.xyz, r9.zzzz, r8.xyzx, r4.xyzx
    r4.xyz = ((r9.zzzz)*(r8.xyzx)+(r4.xyzx)).xyz;
    // 87: add r7.xyz, -r3.xyzx, r7.xyzx
    r7.xyz = ((-(r3.xyzx))+(r7.xyzx)).xyz;
    // 88: mad r3.xyz, r9.zzzz, r7.xyzx, r3.xyzx
    r3.xyz = ((r9.zzzz)*(r7.xyzx)+(r3.xyzx)).xyz;
    // 89: add r3.xyz, r3.xyzx, -cb0[8].xyzx
    r3.xyz = ((r3.xyzx)+(-(source[8].xyzx))).xyz;
    // 90: dp3 r2.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 91: add r7.xyz, -r4.xyzx, r2.wwww
    r7.xyz = ((-(r4.xyzx))+(r2.wwww)).xyz;
    // 92: mad r7.xyz, cb0[19].xxxx, r7.xyzx, r4.xyzx
    r7.xyz = ((source[19].xxxx)*(r7.xyzx)+(r4.xyzx)).xyz;
    // 93: mul r1.xyz, r1.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // 94: dp3 r2.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 95: add r4.xyz, -r7.xyzx, r2.wwww
    r4.xyz = ((-(r7.xyzx))+(r2.wwww)).xyz;
    // 96: mad r4.xyz, cb0[19].yyyy, r4.xyzx, r7.xyzx
    r4.xyz = ((source[19].yyyy)*(r4.xyzx)+(r7.xyzx)).xyz;
    // 97: mad r7.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 98: mad r8.xyz, cb0[10].wwww, cb0[10].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((source[10].wwww)*(source[10].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 99: mul r7.xyz, r7.xyzx, r8.xyzx
    r7.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 100: mul r4.xyz, r4.xyzx, r7.xyzx
    r4.xyz = ((r4.xyzx)*(r7.xyzx)).xyz;
    // 101: mul r8.xyz, r2.xyzx, r4.xyzx
    r8.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 102: dp3 r2.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 103: mad r2.xyz, -r4.xyzx, r2.xyzx, r2.wwww
    r2.xyz = ((-(r4.xyzx))*(r2.xyzx)+(r2.wwww)).xyz;
    // 104: mad r2.xyz, cb0[19].xxxx, r2.xyzx, r8.xyzx
    r2.xyz = ((source[19].xxxx)*(r2.xyzx)+(r8.xyzx)).xyz;
    // 105: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 106: add r4.xyz, -r2.xyzx, r2.wwww
    r4.xyz = ((-(r2.xyzx))+(r2.wwww)).xyz;
    // 107: mad r2.xyz, cb0[19].yyyy, r4.xyzx, r2.xyzx
    r2.xyz = ((source[19].yyyy)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 108: mul r2.xyz, r7.xyzx, r2.xyzx
    r2.xyz = ((r7.xyzx)*(r2.xyzx)).xyz;
    // 109: mad r0.yzw, r0.yyzw, l(0.000000, 2.000000, 2.000000, 2.000000), -r2.xxyz
    r0.yzw = ((r0.yyzw)*(float4(0.000000,2.000000,2.000000,2.000000))+(-(r2.xxyz))).yzw;
    // 110: mad r0.xyz, r0.xxxx, r0.yzwy, r2.xyzx
    r0.xyz = ((r0.xxxx)*(r0.yzwy)+(r2.xyzx)).xyz;
    // 111: add r0.w, r0.y, r0.x
    r0.w = ((r0.yyyy)+(r0.xxxx)).w;
    // 112: add r0.w, r0.z, r0.w
    r0.w = ((r0.zzzz)+(r0.wwww)).w;
    // 113: mul r0.w, r0.w, l(0.333330)
    r0.w = ((r0.wwww)*(float4(0.333330,0.333330,0.333330,0.333330))).w;
    // 114: max r0.w, r0.w, cb0[21].y
    r0.w = (max(r0.wwww,source[21].yyyy)).w;
    // 115: min r0.w, r0.w, cb0[21].x
    r0.w = (min(r0.wwww,source[21].xxxx)).w;
    // 116: add r2.x, -r0.w, l(1.000000)
    r2.x = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 117: mad r0.w, r1.w, r2.x, r0.w
    r0.w = ((r1.wwww)*(r2.xxxx)+(r0.wwww)).w;
    // 118: mul_sat r2.w, r1.w, cb2[3].w
    r2.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 119: add r1.w, r0.w, l(-1.000000)
    r1.w = ((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 120: mad r1.w, cb0[21].w, r1.w, l(1.000000)
    r1.w = ((source[21].wwww)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 121: mul r4.xyz, r0.xyzx, r1.wwww
    r4.xyz = ((r0.xyzx)*(r1.wwww)).xyz;
    // 122: mul r2.x, r6.x, cb0[20].y
    r2.x = ((r6.xxxx)*(source[20].yyyy)).x;
    // 123: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 124: movc r2.x, r5.x, l(0), r2.x
    r2.x = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).x;
    // 125: add_sat r2.x, r2.x, cb0[20].z
    r2.x = (saturate((r2.xxxx)+(source[20].zzzz))).x;
    // 126: add r2.y, -r2.x, l(1.000000)
    r2.y = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 127: mul r5.xyw, r2.yyyy, cb0[14].xyxz
    r5.xyw = ((r2.yyyy)*(source[14].xyxz)).xyw;
    // 128: mul r4.xyz, r4.xyzx, r5.xywx
    r4.xyz = ((r4.xyzx)*(r5.xywx)).xyz;
    // 129: mad r0.xyz, r1.wwww, r0.xyzx, -r4.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(-(r4.xyzx))).xyz;
    // 130: mad r0.xyz, r2.xxxx, r0.xyzx, r4.xyzx
    r0.xyz = ((r2.xxxx)*(r0.xyzx)+(r4.xyzx)).xyz;
    // 131: mul_sat r0.w, r0.w, r2.x
    r0.w = (saturate((r0.wwww)*(r2.xxxx))).w;
    // 132: add r4.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 133: mul r0.xyz, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.xyzx)*(r4.xyzx)).xyz;
    // 134: mad_sat r4.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 135: mad r0.xyz, r4.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r0.xyz = ((r4.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 136: mad r5.xyw, r4.xyxz, l(2.040400, 2.040400, 0.000000, 2.040400), l(-0.332400, -0.332400, 0.000000, -0.332400)
    r5.xyw = ((r4.xyxz)*(float4(2.040400,2.040400,0.000000,2.040400))+(float4(-0.332400,-0.332400,0.000000,-0.332400))).xyw;
    // 137: mad r6.xyw, r4.xyxz, l(-4.795100, -4.795100, 0.000000, -4.795100), l(0.641700, 0.641700, 0.000000, 0.641700)
    r6.xyw = ((r4.xyxz)*(float4(-4.795100,-4.795100,0.000000,-4.795100))+(float4(0.641700,0.641700,0.000000,0.641700))).xyw;
    // 138: mad r5.xyw, r0.wwww, r5.xyxw, r6.xyxw
    r5.xyw = ((r0.wwww)*(r5.xyxw)+(r6.xyxw)).xyw;
    // 139: mad r0.xyz, r5.xywx, r0.wwww, r0.xyzx
    r0.xyz = ((r5.xywx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 140: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 141: max r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = (max(r0.xyzx,r0.wwww)).xyz;
    // 142: mov_sat r4.w, cb0[22].x
    r4.w = (saturate(source[22].xxxx)).w;
    // 143: mad r5.xyw, -r4.wwww, l(0.080000, 0.080000, 0.000000, 0.080000), r4.xyxz
    r5.xyw = ((-(r4.wwww))*(float4(0.080000,0.080000,0.000000,0.080000))+(r4.xyxz)).xyw;
    // 144: mul r1.w, r4.w, l(0.080000)
    r1.w = ((r4.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 145: mov o3.xyzw, r4.xyzw
    output.targets[3].xyzw = (r4.xyzw).xyzw;
    // 146: mad r5.xyw, r2.wwww, r5.xyxw, r1.wwww
    r5.xyw = ((r2.wwww)*(r5.xyxw)+(r1.wwww)).xyw;
    // 147: mul_sat r1.w, r5.y, l(50.000000)
    r1.w = (saturate((r5.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 148: add r2.x, cb0[22].w, -cb0[23].x
    r2.x = ((source[22].wwww)+(-(source[23].xxxx))).x;
    // 149: mad r2.x, r9.x, r2.x, cb0[23].x
    r2.x = ((r9.xxxx)*(r2.xxxx)+(source[23].xxxx)).x;
    // 150: add r2.y, -r2.x, cb0[23].z
    r2.y = ((-(r2.xxxx))+(source[23].zzzz)).y;
    // 151: mad r2.x, r9.y, r2.y, r2.x
    r2.x = ((r9.yyyy)*(r2.yyyy)+(r2.xxxx)).x;
    // 152: add r2.y, -r2.x, cb0[24].x
    r2.y = ((-(r2.xxxx))+(source[24].xxxx)).y;
    // 153: mad r2.x, r9.z, r2.y, r2.x
    r2.x = ((r9.zzzz)*(r2.yyyy)+(r2.xxxx)).x;
    // 154: mul r2.x, r6.z, r2.x
    r2.x = ((r6.zzzz)*(r2.xxxx)).x;
    // 155: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 156: min r2.x, r2.x, l(1.000000)
    r2.x = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 157: movc r2.x, r5.z, l(0), r2.x
    r2.x = ((asuint(r5.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).x;
    // 158: max r2.x, r2.x, cb0[0].x
    r2.x = (max(r2.xxxx,source[0].xxxx)).x;
    // 159: min r2.z, r2.x, l(1.000000)
    r2.z = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 160: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 161: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 162: dp2 r3.w, r2.xyxx, r2.xyxx
    r3.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 163: mul r6.xy, r2.xyxx, cb0[18].xxxx
    r6.xy = ((r2.xyxx)*(source[18].xxxx)).xy;
    // 164: add r2.x, -r3.w, l(1.000000)
    r2.x = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 165: max r2.x, r2.x, l(0.000000)
    r2.x = (max(r2.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 166: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 167: add r6.z, r2.x, l(0.000010)
    r6.z = ((r2.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 168: dp3 r2.x, r6.xyzx, r6.xyzx
    r2.x = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 169: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 170: div r6.xyz, r6.xyzx, r2.xxxx
    r6.xyz = ((r6.xyzx)/(r2.xxxx)).xyz;
    // 171: dp3 r2.x, r6.xyzx, r6.xyzx
    r2.x = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 172: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 173: mul r8.xyz, r2.xxxx, r6.xyzx
    r8.xyz = ((r2.xxxx)*(r6.xyzx)).xyz;
    // 174: dp3 r2.x, v5.xyzx, v5.xyzx
    r2.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 175: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 176: mul r10.xyz, r2.xxxx, v5.xyzx
    r10.xyz = ((r2.xxxx)*(v5.xyzx)).xyz;
    // 177: dp3 r2.x, r8.xyzx, r10.xyzx
    r2.x = (dot((r8.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 178: deriv_rtx_coarse r11.x, r2.x
    r11.x = (ddx_coarse(r2.xxxx)).x;
    // 179: deriv_rty_coarse r11.y, r2.x
    r11.y = (ddy_coarse(r2.xxxx)).y;
    // 180: dp2 r2.y, r11.xyxx, r11.xyxx
    r2.y = (dot((r11.xyxx).xy,(r11.xyxx).xy).xxxx).y;
    // 181: sqrt r2.y, r2.y
    r2.y = (sqrt(r2.yyyy)).y;
    // 182: mad r2.y, r2.y, l(0.300000), r2.z
    r2.y = ((r2.yyyy)*(float4(0.300000,0.300000,0.300000,0.300000))+(r2.zzzz)).y;
    // 183: mov o2.zw, r2.zzzw
    output.targets[2].zw = (r2.zzzw).zw;
    // 184: min r11.y, r2.y, l(1.000000)
    r11.y = (min(r2.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 185: add r2.y, -r11.y, l(1.000000)
    r2.y = ((-(r11.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 186: max r12.xyz, r5.xywx, r2.yyyy
    r12.xyz = (max(r5.xywx,r2.yyyy)).xyz;
    // 187: add r12.xyz, -r5.xywx, r12.xyzx
    r12.xyz = ((-(r5.xywx))+(r12.xyzx)).xyz;
    // 188: mul r12.xyz, r1.wwww, r12.xyzx
    r12.xyz = ((r1.wwww)*(r12.xyzx)).xyz;
    // 189: mul r13.xyz, r2.xxxx, r8.xyzx
    r13.xyz = ((r2.xxxx)*(r8.xyzx)).xyz;
    // 190: mad r13.xyz, r13.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r10.xyzx
    r13.xyz = ((r13.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r10.xyzx))).xyz;
    // 191: add r1.w, r13.z, l(1.000000)
    r1.w = ((r13.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 192: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 193: add r2.y, r2.x, l(1.000000)
    r2.y = ((r2.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 194: mov_sat r2.x, r2.x
    r2.x = (saturate(r2.xxxx)).x;
    // 195: log r2.x, r2.x
    r2.x = (log2(r2.xxxx)).x;
    // 196: mul r2.x, r2.x, cb0[1].y
    r2.x = ((r2.xxxx)*(source[1].yyyy)).x;
    // 197: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 198: mad_sat r2.x, r2.x, cb0[1].w, cb0[1].z
    r2.x = (saturate((r2.xxxx)*(source[1].wwww)+(source[1].zzzz))).x;
    // 199: mul r2.x, r2.x, cb0[24].y
    r2.x = ((r2.xxxx)*(source[24].yyyy)).x;
    // 200: add_sat r11.x, -r1.w, r2.y
    r11.x = (saturate((-(r1.wwww))+(r2.yyyy))).x;
    // 201: sample_indexable(texture2d)(float,float,float,float) r2.yz, r11.xyxx, t7.zxyw, s8
    r2.yz = ((float4(0.0,0.0,0.0,0.0)).zxyw).yz;
    // 202: add r1.w, r0.w, r11.x
    r1.w = ((r0.wwww)+(r11.xxxx)).w;
    // 203: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 204: mul r11.xzw, r2.zzzz, r5.xxyw
    r11.xzw = ((r2.zzzz)*(r5.xxyw)).xzw;
    // 205: mad r11.xzw, r12.xxyz, r2.yyyy, r11.xxzw
    r11.xzw = ((r12.xxyz)*(r2.yyyy)+(r11.xxzw)).xzw;
    // 206: div r2.y, l(1.000000, 1.000000, 1.000000, 1.000000), r2.z
    r2.y = r2.z != 0.f ? 1.f / r2.z : 0.f;
    // 207: add r2.y, r2.y, l(-1.000000)
    r2.y = ((r2.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 208: mad r12.xyz, r5.xywx, r2.yyyy, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((r5.xywx)*(r2.yyyy)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 209: dp3 r2.y, r5.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.y = (dot((r5.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 210: mad r5.xyz, r2.yyyy, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r5.xyz = ((r2.yyyy)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 211: mad r14.xyz, -r11.xzwx, r12.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r14.xyz = ((-(r11.xzwx))*(r12.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 212: mul r11.xzw, r11.xxzw, r12.xxyz
    r11.xzw = ((r11.xxzw)*(r12.xxyz)).xzw;
    // 213: mul r12.xyz, r4.xyzx, r14.xyzx
    r12.xyz = ((r4.xyzx)*(r14.xyzx)).xyz;
    // 214: add r2.y, -r2.w, l(1.000000)
    r2.y = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 215: mul r12.xyz, r2.yyyy, r12.xyzx
    r12.xyz = ((r2.yyyy)*(r12.xyzx)).xyz;
    // 216: dp3 r2.z, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.z = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 217: dp3 r3.w, v1.xyzx, v1.xyzx
    r3.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 218: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 219: mul r15.xyz, r3.wwww, v1.xyzx
    r15.xyz = ((r3.wwww)*(v1.xyzx)).xyz;
    // 220: dp3 r3.w, v0.xyzx, v0.xyzx
    r3.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 221: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 222: mul r16.xyz, r3.wwww, v0.xyzx
    r16.xyz = ((r3.wwww)*(v0.xyzx)).xyz;
    // 223: mul r17.xyz, r15.zxyz, r16.yzxy
    r17.xyz = ((r15.zxyz)*(r16.yzxy)).xyz;
    // 224: mad r17.xyz, r15.yzxy, r16.zxyz, -r17.xyzx
    r17.xyz = ((r15.yzxy)*(r16.zxyz)+(-(r17.xyzx))).xyz;
    // 225: mul r17.xyz, r17.xyzx, v1.wwww
    r17.xyz = ((r17.xyzx)*(v1.wwww)).xyz;
    // 226: dp3 r18.y, r17.xyzx, r8.xyzx
    r18.y = (dot((r17.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 227: dp3 r17.y, r17.xyzx, r13.xyzx
    r17.y = (dot((r17.xyzx).xyz,(r13.xyzx).xyz).xxxx).y;
    // 228: dp3 r18.x, r16.xyzx, r8.xyzx
    r18.x = (dot((r16.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 229: dp3 r17.x, r16.xyzx, r13.xyzx
    r17.x = (dot((r16.xyzx).xyz,(r13.xyzx).xyz).xxxx).x;
    // 230: dp2 r16.z, r18.xyxx, cb0[26].xyxx
    r16.z = (dot((r18.xyxx).xy,(source[26].xyxx).xy).xxxx).z;
    // 231: mul r17.zw, cb0[26].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r17.zw = ((source[26].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 232: dp2 r16.x, r18.xyxx, r17.zwzz
    r16.x = (dot((r18.xyxx).xy,(r17.zwzz).xy).xxxx).x;
    // 233: dp2 r19.x, r17.xyxx, r17.zwzz
    r19.x = (dot((r17.xyxx).xy,(r17.zwzz).xy).xxxx).x;
    // 234: dp2 r19.z, r17.xyxx, cb0[26].xyxx
    r19.z = (dot((r17.xyxx).xy,(source[26].xyxx).xy).xxxx).z;
    // 235: dp3 r16.y, r15.xyzx, r8.xyzx
    r16.y = (dot((r15.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 236: dp3 r19.y, r15.xyzx, r13.xyzx
    r19.y = (dot((r15.xyzx).xyz,(r13.xyzx).xyz).xxxx).y;
    // 237: mov r16.w, l(1.000000)
    r16.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 238: dp4 r15.x, cb0[27].xyzw, r16.xyzw
    r15.x = (dot((source[27].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).x;
    // 239: dp4 r15.y, cb0[28].xyzw, r16.xyzw
    r15.y = (dot((source[28].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).y;
    // 240: dp4 r15.z, cb0[29].xyzw, r16.xyzw
    r15.z = (dot((source[29].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).z;
    // 241: mul r17.xyzw, r16.yzzx, r16.xyzz
    r17.xyzw = ((r16.yzzx)*(r16.xyzz)).xyzw;
    // 242: dp4 r20.x, cb0[30].xyzw, r17.xyzw
    r20.x = (dot((source[30].xyzw).xyzw,(r17.xyzw).xyzw).xxxx).x;
    // 243: dp4 r20.y, cb0[31].xyzw, r17.xyzw
    r20.y = (dot((source[31].xyzw).xyzw,(r17.xyzw).xyzw).xxxx).y;
    // 244: dp4 r20.z, cb0[32].xyzw, r17.xyzw
    r20.z = (dot((source[32].xyzw).xyzw,(r17.xyzw).xyzw).xxxx).z;
    // 245: add r15.xyz, r15.xyzx, r20.xyzx
    r15.xyz = ((r15.xyzx)+(r20.xyzx)).xyz;
    // 246: mul r3.w, r16.y, r16.y
    r3.w = ((r16.yyyy)*(r16.yyyy)).w;
    // 247: mov r18.z, r16.y
    r18.z = (r16.yyyy).z;
    // 248: mad r3.w, r16.x, r16.x, -r3.w
    r3.w = ((r16.xxxx)*(r16.xxxx)+(-(r3.wwww))).w;
    // 249: mad r15.xyz, cb0[33].xyzx, r3.wwww, r15.xyzx
    r15.xyz = ((source[33].xyzx)*(r3.wwww)+(r15.xyzx)).xyz;
    // 250: max r15.xyz, r15.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r15.xyz = (max(r15.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 251: mul r15.xyz, r15.xyzx, cb0[25].xyzx
    r15.xyz = ((r15.xyzx)*(source[25].xyzx)).xyz;
    // 252: mul r15.xyz, r15.xyzx, cb0[26].zzzz
    r15.xyz = ((r15.xyzx)*(source[26].zzzz)).xyz;
    // 253: mad r15.xyz, r15.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[25].wwww
    r15.xyz = ((r15.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[25].wwww)).xyz;
    // 254: dp3 r3.w, r15.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r15.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 255: add r15.xyz, -r3.wwww, r15.xyzx
    r15.xyz = ((-(r3.wwww))+(r15.xyzx)).xyz;
    // 256: mad r15.xyz, r15.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r3.wwww
    r15.xyz = ((r15.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r3.wwww)).xyz;
    // 257: dp3 r3.w, r15.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r15.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 258: mad r4.w, r11.y, l(2.000000), l(2.000000)
    r4.w = ((r11.yyyy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 259: div r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)/(r4.wwww)).w;
    // 260: mad r3.w, r2.z, l(5.000000), r3.w
    r3.w = ((r2.zzzz)*(float4(5.000000,5.000000,5.000000,5.000000))+(r3.wwww)).w;
    // 261: add_sat r3.w, r2.w, r3.w
    r3.w = (saturate((r2.wwww)+(r3.wwww))).w;
    // 262: mad r5.w, r3.w, l(-2.000000), l(3.000000)
    r5.w = ((r3.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 263: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 264: mul r3.w, r3.w, r5.w
    r3.w = ((r3.wwww)*(r5.wwww)).w;
    // 265: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 266: mul r3.w, r3.w, l(1.500000)
    r3.w = ((r3.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 267: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 268: mul r15.xyz, r3.wwww, r15.xyzx
    r15.xyz = ((r3.wwww)*(r15.xyzx)).xyz;
    // 269: mul r12.xyz, r12.xyzx, r15.xyzx
    r12.xyz = ((r12.xyzx)*(r15.xyzx)).xyz;
    // 270: mul r14.xyz, r14.xyzx, r15.xyzx
    r14.xyz = ((r14.xyzx)*(r15.xyzx)).xyz;
    // 271: mul r12.xyz, r0.xyzx, r12.xyzx
    r12.xyz = ((r0.xyzx)*(r12.xyzx)).xyz;
    // 272: mul r3.w, r11.y, l(5.000000)
    r3.w = ((r11.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 273: mul r5.w, r11.y, r11.y
    r5.w = ((r11.yyyy)*(r11.yyyy)).w;
    // 274: mul r1.w, r1.w, r5.w
    r1.w = ((r1.wwww)*(r5.wwww)).w;
    // 275: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 276: add r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)+(r1.wwww)).w;
    // 277: mov o5.y, r0.w
    output.targets[5].y = (r0.wwww).y;
    // 278: add_sat r0.w, r1.w, l(-1.000000)
    r0.w = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 279: sample_l_indexable(texturecube)(float,float,float,float) r15.xyzw, r19.xyzx, t8.xyzw, s7, r3.w
    r15.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r19.xyzx).xyz, (r3.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 280: mul r15.xyz, r15.xyzx, r15.wwww
    r15.xyz = ((r15.xyzx)*(r15.wwww)).xyz;
    // 281: mul r15.xyz, r15.xyzx, cb0[25].xyzx
    r15.xyz = ((r15.xyzx)*(source[25].xyzx)).xyz;
    // 282: mul r15.xyz, r15.xyzx, cb0[26].zzzz
    r15.xyz = ((r15.xyzx)*(source[26].zzzz)).xyz;
    // 283: mad r15.xyz, r15.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[25].wwww
    r15.xyz = ((r15.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[25].wwww)).xyz;
    // 284: dp3 r1.w, r15.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r15.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 285: add r15.xyz, -r1.wwww, r15.xyzx
    r15.xyz = ((-(r1.wwww))+(r15.xyzx)).xyz;
    // 286: mad r15.xyz, r15.xyzx, l(0.800000, 0.800000, 0.800000, 0.000000), r1.wwww
    r15.xyz = ((r15.xyzx)*(float4(0.800000,0.800000,0.800000,0.000000))+(r1.wwww)).xyz;
    // 287: dp3 r1.w, r15.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r15.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 288: div r1.w, r1.w, r4.w
    r1.w = ((r1.wwww)/(r4.wwww)).w;
    // 289: mad r1.w, r2.z, l(5.000000), r1.w
    r1.w = ((r2.zzzz)*(float4(5.000000,5.000000,5.000000,5.000000))+(r1.wwww)).w;
    // 290: add_sat r1.w, r2.w, r1.w
    r1.w = (saturate((r2.wwww)+(r1.wwww))).w;
    // 291: mad r2.z, r1.w, l(-2.000000), l(3.000000)
    r2.z = ((r1.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 292: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 293: mul r1.w, r1.w, r2.z
    r1.w = ((r1.wwww)*(r2.zzzz)).w;
    // 294: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 295: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 296: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 297: mul r15.xyz, r1.wwww, r15.xyzx
    r15.xyz = ((r1.wwww)*(r15.xyzx)).xyz;
    // 298: mul r16.xyz, r11.xzwx, r15.xyzx
    r16.xyz = ((r11.xzwx)*(r15.xyzx)).xyz;
    // 299: mad r1.w, r0.w, r5.x, r5.y
    r1.w = ((r0.wwww)*(r5.xxxx)+(r5.yyyy)).w;
    // 300: mad r1.w, r1.w, r0.w, r5.z
    r1.w = ((r1.wwww)*(r0.wwww)+(r5.zzzz)).w;
    // 301: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 302: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 303: mad r5.xyz, r16.xyzx, r0.wwww, r12.xyzx
    r5.xyz = ((r16.xyzx)*(r0.wwww)+(r12.xyzx)).xyz;
    // 304: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 305: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 306: mul r12.xyz, r1.wwww, v6.xyzx
    r12.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 307: dp3 r1.w, r12.xyzx, r8.xyzx
    r1.w = (dot((r12.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 308: dp3 r2.z, -r12.xyzx, r8.xyzx
    r2.z = (dot((-(r12.xyzx)).xyz,(r8.xyzx).xyz).xxxx).z;
    // 309: dp3 r3.w, r12.xyzx, r13.xyzx
    r3.w = (dot((r12.xyzx).xyz,(r13.xyzx).xyz).xxxx).w;
    // 310: mad r8.xy, r3.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r8.xy = ((r3.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 311: mad r8.zw, r2.zzzz, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r8.zw = ((r2.zzzz)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 312: mul r8.xyzw, r8.xyzw, r8.xyzw
    r8.xyzw = ((r8.xyzw)*(r8.xyzw)).xyzw;
    // 313: mad r12.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r12.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 314: mul r12.xy, r12.xyxx, r12.xyxx
    r12.xy = ((r12.xyxx)*(r12.xyxx)).xy;
    // 315: mul r12.yzw, r12.yyyy, cb0[36].xxyz
    r12.yzw = ((r12.yyyy)*(source[36].xxyz)).yzw;
    // 316: mad r12.xyz, r12.xxxx, cb0[35].xyzx, r12.yzwy
    r12.xyz = ((r12.xxxx)*(source[35].xyzx)+(r12.yzwy)).xyz;
    // 317: mul r12.xyz, r12.xyzx, cb0[37].wwww
    r12.xyz = ((r12.xyzx)*(source[37].wwww)).xyz;
    // 318: mul r12.xyz, r4.xyzx, r12.xyzx
    r12.xyz = ((r4.xyzx)*(r12.xyzx)).xyz;
    // 319: mul r0.xyz, r0.xyzx, r12.xyzx
    r0.xyz = ((r0.xyzx)*(r12.xyzx)).xyz;
    // 320: mul r0.xyz, r0.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 321: mul r0.xyz, r14.xyzx, r0.xyzx
    r0.xyz = ((r14.xyzx)*(r0.xyzx)).xyz;
    // 322: mad r0.xyz, -r0.xyzx, r2.wwww, r0.xyzx
    r0.xyz = ((-(r0.xyzx))*(r2.wwww)+(r0.xyzx)).xyz;
    // 323: mad r0.xyz, r5.xyzx, l(0.400000, 0.400000, 0.400000, 0.000000), r0.xyzx
    r0.xyz = ((r5.xyzx)*(float4(0.400000,0.400000,0.400000,0.000000))+(r0.xyzx)).xyz;
    // 324: mul r5.xyz, r8.yyyy, cb0[36].xyzx
    r5.xyz = ((r8.yyyy)*(source[36].xyzx)).xyz;
    // 325: mad r5.xyz, cb0[35].xyzx, r8.xxxx, r5.xyzx
    r5.xyz = ((source[35].xyzx)*(r8.xxxx)+(r5.xyzx)).xyz;
    // 326: mul r5.xyz, r5.xyzx, cb0[37].wwww
    r5.xyz = ((r5.xyzx)*(source[37].wwww)).xyz;
    // 327: mul r5.xyz, r0.wwww, r5.xyzx
    r5.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // 328: mul r5.xyz, r15.xyzx, r5.xyzx
    r5.xyz = ((r15.xyzx)*(r5.xyzx)).xyz;
    // 329: mul r5.xyz, r5.xyzx, r11.xzwx
    r5.xyz = ((r5.xyzx)*(r11.xzwx)).xyz;
    // 330: mad r0.xyz, r5.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r0.xyzx
    r0.xyz = ((r5.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r0.xyzx)).xyz;
    // 331: mul r5.xyz, r5.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))).xyz;
    // 332: dp3 o4.x, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 333: add r0.w, r9.y, r9.x
    r0.w = ((r9.yyyy)+(r9.xxxx)).w;
    // 334: add_sat r0.w, r9.z, r0.w
    r0.w = (saturate((r9.zzzz)+(r0.wwww))).w;
    // 335: mad r3.xyz, r0.wwww, r3.xyzx, cb0[8].xyzx
    r3.xyz = ((r0.wwww)*(r3.xyzx)+(source[8].xyzx)).xyz;
    // 336: mul r3.xyz, r3.xyzx, cb0[18].wwww
    r3.xyz = ((r3.xyzx)*(source[18].wwww)).xyz;
    // 337: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t6.xyzw, s4, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 338: mul r9.xyz, r3.xyzx, r5.xyzx
    r9.xyz = ((r3.xyzx)*(r5.xyzx)).xyz;
    // 339: dp3 r0.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 340: mad r3.xyz, -r5.xyzx, r3.xyzx, r0.wwww
    r3.xyz = ((-(r5.xyzx))*(r3.xyzx)+(r0.wwww)).xyz;
    // 341: mad r3.xyz, cb0[19].xxxx, r3.xyzx, r9.xyzx
    r3.xyz = ((source[19].xxxx)*(r3.xyzx)+(r9.xyzx)).xyz;
    // 342: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 343: add r5.xyz, -r3.xyzx, r0.wwww
    r5.xyz = ((-(r3.xyzx))+(r0.wwww)).xyz;
    // 344: mad r3.xyz, cb0[19].yyyy, r5.xyzx, r3.xyzx
    r3.xyz = ((source[19].yyyy)*(r5.xyzx)+(r3.xyzx)).xyz;
    // 345: dp3 r0.w, r6.xyzx, r10.xyzx
    r0.w = (dot((r6.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 346: mul_sat r1.w, r0.w, cb0[19].z
    r1.w = (saturate((r0.wwww)*(source[19].zzzz))).w;
    // 347: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 348: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 349: mul_sat r2.z, r10.z, cb0[19].z
    r2.z = (saturate((r10.zzzz)*(source[19].zzzz))).z;
    // 350: add r2.w, -|r10.z|, l(1.000000)
    r2.w = ((-(abs(r10.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 351: mul r0.w, r0.w, r2.w
    r0.w = ((r0.wwww)*(r2.wwww)).w;
    // 352: add r2.z, -r2.z, l(1.000000)
    r2.z = ((-(r2.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 353: add_sat r2.z, r2.z, -cb0[19].w
    r2.z = (saturate((r2.zzzz)+(-(source[19].wwww)))).z;
    // 354: log r2.w, r2.z
    r2.w = (log2(r2.zzzz)).w;
    // 355: lt r2.z, r2.z, l(0.000001)
    r2.z = (asfloat((uint4)((r2.zzzz)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 356: mul r2.w, r2.w, cb0[20].x
    r2.w = ((r2.wwww)*(source[20].xxxx)).w;
    // 357: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 358: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 359: movc r1.w, r2.z, l(0), r1.w
    r1.w = ((asuint(r2.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 360: mad r5.xyz, r1.wwww, cb0[12].xyzx, -cb0[12].xyzx
    r5.xyz = ((r1.wwww)*(source[12].xyzx)+(-(source[12].xyzx))).xyz;
    // 361: mul r1.w, r1.w, cb0[11].w
    r1.w = ((r1.wwww)*(source[11].wwww)).w;
    // 362: mad r5.xyz, cb0[12].wwww, r5.xyzx, cb0[12].xyzx
    r5.xyz = ((source[12].wwww)*(r5.xyzx)+(source[12].xyzx)).xyz;
    // 363: mad r5.xyz, r1.wwww, cb0[11].xyzx, r5.xyzx
    r5.xyz = ((r1.wwww)*(source[11].xyzx)+(r5.xyzx)).xyz;
    // 364: mad r3.xyz, r3.xyzx, r7.xyzx, r5.xyzx
    r3.xyz = ((r3.xyzx)*(r7.xyzx)+(r5.xyzx)).xyz;
    // 365: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 366: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 367: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 368: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 369: mul r5.xyz, r1.wwww, cb0[13].xyzx
    r5.xyz = ((r1.wwww)*(source[13].xyzx)).xyz;
    // 370: movc r5.xyz, r0.wwww, l(0,0,0,0), r5.xyzx
    r5.xyz = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.xyzx)).xyz;
    // 371: add r3.xyz, r3.xyzx, r5.xyzx
    r3.xyz = ((r3.xyzx)+(r5.xyzx)).xyz;
    // 372: mad r1.xyz, cb0[18].zzzz, r1.xyzx, r3.xyzx
    r1.xyz = ((source[18].zzzz)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 373: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 374: mul r3.xyz, r8.wwww, cb0[36].xyzx
    r3.xyz = ((r8.wwww)*(source[36].xyzx)).xyz;
    // 375: mad r3.xyz, r8.zzzz, cb0[35].xyzx, r3.xyzx
    r3.xyz = ((r8.zzzz)*(source[35].xyzx)+(r3.xyzx)).xyz;
    // 376: mul r3.xyz, r3.xyzx, cb0[37].wwww
    r3.xyz = ((r3.xyzx)*(source[37].wwww)).xyz;
    // 377: mul_sat r5.xyz, cb0[17].xyzx, cb0[17].wwww
    r5.xyz = (saturate((source[17].xyzx)*(source[17].wwww))).xyz;
    // 378: mul r2.xzw, r2.xxxx, r5.xxyz
    r2.xzw = ((r2.xxxx)*(r5.xxyz)).xzw;
    // 379: mul r5.xyz, r5.xyzx, cb0[24].yyyy
    r5.xyz = ((r5.xyzx)*(source[24].yyyy)).xyz;
    // 380: dp3_sat o5.x, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[5].x = (saturate(dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx)).x;
    // 381: mul r2.xyz, r2.yyyy, r2.xzwx
    r2.xyz = ((r2.yyyy)*(r2.xzwx)).xyz;
    // 382: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 383: mul r2.xyz, r4.xyzx, r2.xyzx
    r2.xyz = ((r4.xyzx)*(r2.xyzx)).xyz;
    // 384: mad r1.xyz, r2.xyzx, l(0.600000, 0.600000, 0.600000, 0.000000), r1.xyzx
    r1.xyz = ((r2.xyzx)*(float4(0.600000,0.600000,0.600000,0.000000))+(r1.xyzx)).xyz;
    // 385: add r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 386: dp3 o4.y, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 387: mad o0.xyz, r4.xyzx, cb0[37].xyzx, r1.xyzx
    output.targets[0].xyz = ((r4.xyzx)*(source[37].xyzx)+(r1.xyzx)).xyz;
    // 388: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 389: dp3 r0.x, r18.xyzx, r18.xyzx
    r0.x = (dot((r18.xyzx).xyz,(r18.xyzx).xyz).xxxx).x;
    // 390: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 391: mul r0.xyz, r0.xxxx, r18.xyzx
    r0.xyz = ((r0.xxxx)*(r18.xyzx)).xyz;
    // 392: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 393: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 394: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 395: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 396: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 397: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 398: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 399: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 400: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 401: ftou r0.x, cb0[34].z
    r0.x = (asfloat((uint4)(source[34].zzzz))).x;
    // 402: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 403: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 404: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 405: mov o5.z, l(0.450000)
    output.targets[5].z = (float4(0.450000,0.450000,0.450000,0.450000)).z;
    // 406: ret
    return output;
}
```

### light1526.hlsli

```hlsl
// source.character.equipment-native-1526.v1 / source program 26eada8cd67300489567bd8748f26356
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1526(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[11]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[16].w=(g_SourceCharacterTime.xxxx).x;
    source[21]=float4(input.lightColor,1.0);
    source[22].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0;
    // 1: dp3 r0.x, v7.xyzx, v7.xyzx
    r0.x = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xyz, r0.xxxx, v7.xyzx
    r0.xyz = ((r0.xxxx)*(v7.xyzx)).xyz;
    // 4: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 5: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 6: mul r1.xyz, r0.wwww, v5.xyzx
    r1.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 8: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 9: mul r3.xy, r2.xyxx, cb0[14].xxxx
    r3.xy = ((r2.xyxx)*(source[14].xxxx)).xy;
    // 10: dp2 r1.w, r2.xyxx, r2.xyxx
    r1.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 11: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 12: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 13: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 14: add r3.z, r1.w, l(0.000010)
    r3.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 15: dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 16: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 17: div r2.xyz, r3.xyzx, r1.wwww
    r2.xyz = ((r3.xyzx)/(r1.wwww)).xyz;
    // 18: dp3 r1.w, r2.xyzx, r2.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 19: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 20: mul r2.xyz, r1.wwww, r2.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)).xyz;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t1.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 22: mul r4.xyzw, r3.xyzw, cb0[12].xyzw
    r4.xyzw = ((r3.xyzw)*(source[12].xyzw)).xyzw;
    // 23: add r4.xy, r4.ywyy, r4.xzxx
    r4.xy = ((r4.ywyy)+(r4.xzxx)).xy;
    // 24: add r1.w, r4.y, r4.x
    r1.w = ((r4.yyyy)+(r4.xxxx)).w;
    // 25: add r3.xy, r3.ywyy, r3.xzxx
    r3.xy = ((r3.ywyy)+(r3.xzxx)).xy;
    // 26: add r2.w, r3.y, r3.x
    r2.w = ((r3.yyyy)+(r3.xxxx)).w;
    // 27: add r1.w, r1.w, l(-1.000000)
    r1.w = ((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 28: mad_sat r1.w, r2.w, r1.w, l(1.000000)
    r1.w = (saturate((r2.wwww)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000)))).w;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 30: mul_sat r1.w, r1.w, r3.w
    r1.w = (saturate((r1.wwww)*(r3.wwww))).w;
    // 31: add r1.w, r1.w, l(-0.333300)
    r1.w = ((r1.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 32: lt r1.w, r1.w, l(0.000000)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 33: discard_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) { output.discarded = true; return output; }
    // 34: ne r1.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[22].x
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[22].xxxx)) * 0xffffffffu)).w;
    // 35: if_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) {
    // 36: div r4.xy, v8.xyxx, v8.wwww
    r4.xy = ((v8.xyxx)/(v8.wwww)).xy;
    // 37: mad r4.xy, r4.xyxx, cb2[0].xyxx, cb2[0].wzww
    r4.xy = ((r4.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 38: sample_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t6.xyzw, s0
    r4.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 39: mul r4.xyz, r4.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 40: else
    } else {
    // 41: mov r4.xyz, l(1.000000,1.000000,1.000000,0)
    r4.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 42: endif
    }
    // 43: add r5.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 45: lt r7.xyz, |r6.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r7.xyz = (asfloat((uint4)((abs(r6.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 46: log r6.xyz, |r6.xzyx|
    r6.xyz = (log2(abs(r6.xzyx))).xyz;
    // 47: mul r1.w, r6.x, cb0[16].y
    r1.w = ((r6.xxxx)*(source[16].yyyy)).w;
    // 48: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 49: movc r1.w, r7.x, l(0), r1.w
    r1.w = ((asuint(r7.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 50: add_sat r1.w, r1.w, cb0[16].z
    r1.w = (saturate((r1.wwww)+(source[16].zzzz))).w;
    // 51: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 52: mul r8.xyz, r2.wwww, cb0[10].xyzx
    r8.xyz = ((r2.wwww)*(source[10].xyzx)).xyz;
    // 53: mul r9.xyz, cb0[3].xyzx, cb0[3].wwww
    r9.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 54: max r10.xyz, r9.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r10.xyz = (max(r9.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 55: min r10.xyz, r10.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r10.xyz = (min(r10.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 56: max r9.xyz, r9.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r9.xyz = (max(r9.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 57: min r9.xyz, r9.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r9.xyz = (min(r9.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 58: mul r2.w, r6.y, cb0[14].y
    r2.w = ((r6.yyyy)*(source[14].yyyy)).w;
    // 59: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 60: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 61: movc r2.w, r7.y, l(0), r2.w
    r2.w = ((asuint(r7.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 62: add r6.xyw, -r10.xyxz, r9.xyxz
    r6.xyw = ((-(r10.xyxz))+(r9.xyxz)).xyw;
    // 63: mad r6.xyw, r2.wwww, r6.xyxw, r10.xyxz
    r6.xyw = ((r2.wwww)*(r6.xyxw)+(r10.xyxz)).xyw;
    // 64: mul r7.xyw, cb0[4].xyxz, cb0[4].wwww
    r7.xyw = ((source[4].xyxz)*(source[4].wwww)).xyw;
    // 65: max r9.xyz, r7.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r9.xyz = (max(r7.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 66: min r9.xyz, r9.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r9.xyz = (min(r9.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 67: max r7.xyw, r7.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r7.xyw = (max(r7.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 68: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 69: add r7.xyw, -r9.xyxz, r7.xyxw
    r7.xyw = ((-(r9.xyxz))+(r7.xyxw)).xyw;
    // 70: mad r7.xyw, r2.wwww, r7.xyxw, r9.xyxz
    r7.xyw = ((r2.wwww)*(r7.xyxw)+(r9.xyxz)).xyw;
    // 71: sample_b_indexable(texture2d)(float,float,float,float) r9.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r9.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 72: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 73: mad r6.xyw, r9.xxxx, r7.xyxw, r6.xyxw
    r6.xyw = ((r9.xxxx)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 74: mul r7.xyw, cb0[5].xyxz, cb0[5].wwww
    r7.xyw = ((source[5].xyxz)*(source[5].wwww)).xyw;
    // 75: max r10.xyz, r7.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r10.xyz = (max(r7.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 76: min r10.xyz, r10.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r10.xyz = (min(r10.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 77: max r7.xyw, r7.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r7.xyw = (max(r7.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 78: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 79: add r7.xyw, -r10.xyxz, r7.xyxw
    r7.xyw = ((-(r10.xyxz))+(r7.xyxw)).xyw;
    // 80: mad r7.xyw, r2.wwww, r7.xyxw, r10.xyxz
    r7.xyw = ((r2.wwww)*(r7.xyxw)+(r10.xyxz)).xyw;
    // 81: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 82: mad r6.xyw, r9.yyyy, r7.xyxw, r6.xyxw
    r6.xyw = ((r9.yyyy)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 83: mul r7.xyw, cb0[6].xyxz, cb0[6].wwww
    r7.xyw = ((source[6].xyxz)*(source[6].wwww)).xyw;
    // 84: max r10.xyz, r7.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r10.xyz = (max(r7.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 85: min r10.xyz, r10.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r10.xyz = (min(r10.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 86: max r7.xyw, r7.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r7.xyw = (max(r7.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 87: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 88: add r7.xyw, -r10.xyxz, r7.xyxw
    r7.xyw = ((-(r10.xyxz))+(r7.xyxw)).xyw;
    // 89: mad r7.xyw, r2.wwww, r7.xyxw, r10.xyxz
    r7.xyw = ((r2.wwww)*(r7.xyxw)+(r10.xyxz)).xyw;
    // 90: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 91: mad r6.xyw, r9.zzzz, r7.xyxw, r6.xyxw
    r6.xyw = ((r9.zzzz)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 92: dp3 r3.w, r6.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r6.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 93: add r7.xyw, -r6.xyxw, r3.wwww
    r7.xyw = ((-(r6.xyxw))+(r3.wwww)).xyw;
    // 94: mad r6.xyw, cb0[15].xxxx, r7.xyxw, r6.xyxw
    r6.xyw = ((source[15].xxxx)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 95: dp3 r3.w, r6.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r6.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 96: add r7.xyw, -r6.xyxw, r3.wwww
    r7.xyw = ((-(r6.xyxw))+(r3.wwww)).xyw;
    // 97: mad r6.xyw, cb0[15].yyyy, r7.xyxw, r6.xyxw
    r6.xyw = ((source[15].yyyy)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 98: mad r7.xyw, cb0[8].wwww, cb0[8].xyxz, l(1.000000, 1.000000, 0.000000, 1.000000)
    r7.xyw = ((source[8].wwww)*(source[8].xyxz)+(float4(1.000000,1.000000,0.000000,1.000000))).xyw;
    // 99: mad r10.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 100: mul r7.xyw, r7.xyxw, r10.xyxz
    r7.xyw = ((r7.xyxw)*(r10.xyxz)).xyw;
    // 101: mul r6.xyw, r6.xyxw, r7.xyxw
    r6.xyw = ((r6.xyxw)*(r7.xyxw)).xyw;
    // 102: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 103: add r10.xyz, -r3.xyzx, r3.wwww
    r10.xyz = ((-(r3.xyzx))+(r3.wwww)).xyz;
    // 104: mad r3.xyz, cb0[15].xxxx, r10.xyzx, r3.xyzx
    r3.xyz = ((source[15].xxxx)*(r10.xyzx)+(r3.xyzx)).xyz;
    // 105: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 106: add r10.xyz, -r3.xyzx, r3.wwww
    r10.xyz = ((-(r3.xyzx))+(r3.wwww)).xyz;
    // 107: mad r3.xyz, cb0[15].yyyy, r10.xyzx, r3.xyzx
    r3.xyz = ((source[15].yyyy)*(r10.xyzx)+(r3.xyzx)).xyz;
    // 108: mul r10.xyz, r3.xyzx, r6.xywx
    r10.xyz = ((r3.xyzx)*(r6.xywx)).xyz;
    // 109: dp3 r3.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 110: mad r3.xyz, -r6.xywx, r3.xyzx, r3.wwww
    r3.xyz = ((-(r6.xywx))*(r3.xyzx)+(r3.wwww)).xyz;
    // 111: mad r3.xyz, cb0[15].xxxx, r3.xyzx, r10.xyzx
    r3.xyz = ((source[15].xxxx)*(r3.xyzx)+(r10.xyzx)).xyz;
    // 112: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 113: add r6.xyw, -r3.xyxz, r3.wwww
    r6.xyw = ((-(r3.xyxz))+(r3.wwww)).xyw;
    // 114: mad r3.xyz, cb0[15].yyyy, r6.xywx, r3.xyzx
    r3.xyz = ((source[15].yyyy)*(r6.xywx)+(r3.xyzx)).xyz;
    // 115: mul r3.xyz, r7.xywx, r3.xyzx
    r3.xyz = ((r7.xywx)*(r3.xyzx)).xyz;
    // 116: mul r3.w, cb0[7].z, l(1.500000)
    r3.w = ((source[7].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 117: add r4.w, -cb0[7].w, l(1.000000)
    r4.w = ((-(source[7].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 118: mul r4.w, r4.w, cb0[16].w
    r4.w = ((r4.wwww)*(source[16].wwww)).w;
    // 119: mul r4.w, r4.w, l(6.283185)
    r4.w = ((r4.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 120: sincos r4.w, null, r4.w
    r4.w = (sin(r4.wwww)).w;
    // 121: add r4.w, r4.w, l(1.000000)
    r4.w = ((r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 122: mul r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)*(r4.wwww)).w;
    // 123: mad r3.w, r3.w, l(0.500000), cb0[7].z
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[7].zzzz)).w;
    // 124: frc r4.w, cb0[7].x
    r4.w = (frac(source[7].xxxx)).w;
    // 125: add r5.w, -r4.w, cb0[7].x
    r5.w = ((-(r4.wwww))+(source[7].xxxx)).w;
    // 126: mul r10.z, r5.w, l(0.125000)
    r10.z = ((r5.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 127: mov r10.xw, l(0,0,0,0)
    r10.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 128: mul r10.y, cb0[7].y, cb0[11].y
    r10.y = ((source[7].yyyy)*(source[11].yyyy)).y;
    // 129: frc r5.w, v4.x
    r5.w = (frac(v4.xxxx)).w;
    // 130: mul r6.x, r5.w, l(0.125000)
    r6.x = ((r5.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 131: mov r6.y, v4.y
    r6.y = (v4.yyyy).y;
    // 132: add r6.xy, r6.xyxx, r10.xyxx
    r6.xy = ((r6.xyxx)+(r10.xyxx)).xy;
    // 133: add r6.xy, r6.xyxx, r10.zwzz
    r6.xy = ((r6.xyxx)+(r10.zwzz)).xy;
    // 134: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, r6.xyxx, t5.xyzw, s5, l(0.000000)
    r10.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 135: mul r6.xyw, r3.wwww, r10.xyxz
    r6.xyw = ((r3.wwww)*(r10.xyxz)).xyw;
    // 136: mul r3.w, r4.w, r10.w
    r3.w = ((r4.wwww)*(r10.wwww)).w;
    // 137: mad r6.xyw, r6.xyxw, l(2.000000, 2.000000, 0.000000, 2.000000), -r3.xyxz
    r6.xyw = ((r6.xyxw)*(float4(2.000000,2.000000,0.000000,2.000000))+(-(r3.xyxz))).xyw;
    // 138: mad r3.xyz, r3.wwww, r6.xywx, r3.xyzx
    r3.xyz = ((r3.wwww)*(r6.xywx)+(r3.xyzx)).xyz;
    // 139: add r3.w, r3.y, r3.x
    r3.w = ((r3.yyyy)+(r3.xxxx)).w;
    // 140: add r3.w, r3.z, r3.w
    r3.w = ((r3.zzzz)+(r3.wwww)).w;
    // 141: mul r3.w, r3.w, l(0.333330)
    r3.w = ((r3.wwww)*(float4(0.333330,0.333330,0.333330,0.333330))).w;
    // 142: max r3.w, r3.w, cb0[17].y
    r3.w = (max(r3.wwww,source[17].yyyy)).w;
    // 143: min r3.w, r3.w, cb0[17].x
    r3.w = (min(r3.wwww,source[17].xxxx)).w;
    // 144: add r4.w, -r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 145: mad r3.w, r2.w, r4.w, r3.w
    r3.w = ((r2.wwww)*(r4.wwww)+(r3.wwww)).w;
    // 146: add r3.w, r3.w, l(-1.000000)
    r3.w = ((r3.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 147: mad r3.w, cb0[17].w, r3.w, l(1.000000)
    r3.w = ((source[17].wwww)*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 148: mul r6.xyw, r3.xyxz, r3.wwww
    r6.xyw = ((r3.xyxz)*(r3.wwww)).xyw;
    // 149: mul r6.xyw, r6.xyxw, r8.xyxz
    r6.xyw = ((r6.xyxw)*(r8.xyxz)).xyw;
    // 150: mad r3.xyz, r3.wwww, r3.xyzx, -r6.xywx
    r3.xyz = ((r3.wwww)*(r3.xyzx)+(-(r6.xywx))).xyz;
    // 151: mad r3.xyz, r1.wwww, r3.xyzx, r6.xywx
    r3.xyz = ((r1.wwww)*(r3.xyzx)+(r6.xywx)).xyz;
    // 152: mul r3.xyz, r5.xyzx, r3.xyzx
    r3.xyz = ((r5.xyzx)*(r3.xyzx)).xyz;
    // 153: mad_sat r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = (saturate((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 154: mov_sat r1.w, cb0[18].x
    r1.w = (saturate(source[18].xxxx)).w;
    // 155: mul_sat r2.w, r2.w, cb2[3].w
    r2.w = (saturate((r2.wwww)*(passValues[3].wwww))).w;
    // 156: add r3.w, cb0[18].w, -cb0[19].x
    r3.w = ((source[18].wwww)+(-(source[19].xxxx))).w;
    // 157: mad r3.w, r9.x, r3.w, cb0[19].x
    r3.w = ((r9.xxxx)*(r3.wwww)+(source[19].xxxx)).w;
    // 158: add r4.w, -r3.w, cb0[19].z
    r4.w = ((-(r3.wwww))+(source[19].zzzz)).w;
    // 159: mad r3.w, r9.y, r4.w, r3.w
    r3.w = ((r9.yyyy)*(r4.wwww)+(r3.wwww)).w;
    // 160: add r4.w, -r3.w, cb0[20].x
    r4.w = ((-(r3.wwww))+(source[20].xxxx)).w;
    // 161: mad r3.w, r9.z, r4.w, r3.w
    r3.w = ((r9.zzzz)*(r4.wwww)+(r3.wwww)).w;
    // 162: mul r3.w, r6.z, r3.w
    r3.w = ((r6.zzzz)*(r3.wwww)).w;
    // 163: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 164: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 165: movc r3.w, r7.z, l(0), r3.w
    r3.w = ((asuint(r7.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 166: max r3.w, r3.w, cb0[0].x
    r3.w = (max(r3.wwww,source[0].xxxx)).w;
    // 167: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 168: mad r5.xyz, v5.xyzx, r0.wwww, r0.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 169: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 170: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 171: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 172: dp3_sat r4.w, r2.xyzx, r5.xyzx
    r4.w = (saturate(dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 173: dp3 r5.w, r2.xyzx, r0.xyzx
    r5.w = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 174: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 175: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 176: dp3_sat r6.x, r2.xyzx, r1.xyzx
    r6.x = (saturate(dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx)).x;
    // 177: dp3_sat r5.x, r0.xyzx, r5.xyzx
    r5.x = (saturate(dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx)).x;
    // 178: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 179: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 180: add r5.x, r5.x, l(1.000000)
    r5.x = ((r5.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 181: add r0.w, -r0.w, r5.x
    r0.w = ((-(r0.wwww))+(r5.xxxx)).w;
    // 182: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 183: mad r5.xyz, -r3.xyzx, r2.wwww, r3.xyzx
    r5.xyz = ((-(r3.xyzx))*(r2.wwww)+(r3.xyzx)).xyz;
    // 184: mul r5.xyz, r5.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 185: mul r6.y, r3.w, r3.w
    r6.y = ((r3.wwww)*(r3.wwww)).y;
    // 186: mul r6.z, r6.y, r6.y
    r6.z = ((r6.yyyy)*(r6.yyyy)).z;
    // 187: mad r6.w, r4.w, r6.z, -r4.w
    r6.w = ((r4.wwww)*(r6.zzzz)+(-(r4.wwww))).w;
    // 188: mad r4.w, r6.w, r4.w, l(1.000000)
    r4.w = ((r6.wwww)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 189: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 190: mul r4.w, r4.w, l(3.141593)
    r4.w = ((r4.wwww)*(float4(3.141593,3.141593,3.141593,3.141593))).w;
    // 191: div r4.w, r6.z, r4.w
    r4.w = ((r6.zzzz)/(r4.wwww)).w;
    // 192: mad r6.z, -r3.w, r3.w, l(1.000000)
    r6.z = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 193: mad r6.w, r5.w, r6.z, r6.y
    r6.w = ((r5.wwww)*(r6.zzzz)+(r6.yyyy)).w;
    // 194: mad r6.y, r6.x, r6.z, r6.y
    r6.y = ((r6.xxxx)*(r6.zzzz)+(r6.yyyy)).y;
    // 195: mul r5.w, r5.w, r6.y
    r5.w = ((r5.wwww)*(r6.yyyy)).w;
    // 196: mad r5.w, r6.x, r6.w, r5.w
    r5.w = ((r6.xxxx)*(r6.wwww)+(r5.wwww)).w;
    // 197: rcp r5.w, r5.w
    r5.w = (1.0/(r5.wwww)).w;
    // 198: mul r4.w, r4.w, r5.w
    r4.w = ((r4.wwww)*(r5.wwww)).w;
    // 199: mul r5.w, r1.w, l(0.080000)
    r5.w = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 200: mad r6.yzw, -r1.wwww, l(0.000000, 0.080000, 0.080000, 0.080000), r3.xxyz
    r6.yzw = ((-(r1.wwww))*(float4(0.000000,0.080000,0.080000,0.080000))+(r3.xxyz)).yzw;
    // 201: mad r6.yzw, r2.wwww, r6.yyzw, r5.wwww
    r6.yzw = ((r2.wwww)*(r6.yyzw)+(r5.wwww)).yzw;
    // 202: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 203: mul r1.w, r0.w, r0.w
    r1.w = ((r0.wwww)*(r0.wwww)).w;
    // 204: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 205: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 206: mul_sat r1.w, r6.z, l(50.000000)
    r1.w = (saturate((r6.zzzz)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 207: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 208: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 209: max r7.xyz, r6.yzwy, r3.wwww
    r7.xyz = (max(r6.yzwy,r3.wwww)).xyz;
    // 210: add r7.xyz, -r6.yzwy, r7.xyzx
    r7.xyz = ((-(r6.yzwy))+(r7.xyzx)).xyz;
    // 211: mad r6.yzw, -r0.wwww, r6.yyzw, r6.yyzw
    r6.yzw = ((-(r0.wwww))*(r6.yyzw)+(r6.yyzw)).yzw;
    // 212: mad r6.yzw, r1.wwww, r7.xxyz, r6.yyzw
    r6.yzw = ((r1.wwww)*(r7.xxyz)+(r6.yyzw)).yzw;
    // 213: dp3 r0.w, r6.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r6.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 214: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 215: mul r1.w, r4.w, l(0.500000)
    r1.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 216: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 217: min r0.w, r0.w, r1.w
    r0.w = (min(r0.wwww,r1.wwww)).w;
    // 218: mul r7.xyz, r6.yzwy, r0.wwww
    r7.xyz = ((r6.yzwy)*(r0.wwww)).xyz;
    // 219: add r6.yzw, -r6.yyzw, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(r6.yyzw))+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 220: mad r5.xyz, r5.xyzx, r6.yzwy, r7.xyzx
    r5.xyz = ((r5.xyzx)*(r6.yzwy)+(r7.xyzx)).xyz;
    // 221: mul r5.xyz, r6.xxxx, r5.xyzx
    r5.xyz = ((r6.xxxx)*(r5.xyzx)).xyz;
    // 222: mul r5.xyz, r5.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 223: mul_sat r6.xyz, cb0[13].xyzx, cb0[13].wwww
    r6.xyz = (saturate((source[13].xyzx)*(source[13].wwww))).xyz;
    // 224: mad r1.xyz, r2.xyzx, cb0[1].xxxx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(source[1].xxxx)+(r1.xyzx)).xyz;
    // 225: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 226: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 227: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 228: dp3_sat r0.x, r0.xyzx, -r1.xyzx
    r0.x = (saturate(dot((r0.xyzx).xyz,(-(r1.xyzx)).xyz).xxxx)).x;
    // 229: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 230: mul r0.x, r0.x, cb0[1].y
    r0.x = ((r0.xxxx)*(source[1].yyyy)).x;
    // 231: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 232: mad_sat r0.x, r0.x, cb0[1].w, cb0[1].z
    r0.x = (saturate((r0.xxxx)*(source[1].wwww)+(source[1].zzzz))).x;
    // 233: mul r0.x, r0.x, cb0[20].y
    r0.x = ((r0.xxxx)*(source[20].yyyy)).x;
    // 234: mul r0.xyz, r3.xyzx, r0.xxxx
    r0.xyz = ((r3.xyzx)*(r0.xxxx)).xyz;
    // 235: mul r0.xyz, r6.xyzx, r0.xyzx
    r0.xyz = ((r6.xyzx)*(r0.xyzx)).xyz;
    // 236: add r0.w, -r2.w, l(1.000000)
    r0.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 237: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 238: mad r0.xyz, r5.xyzx, r4.xyzx, r0.xyzx
    r0.xyz = ((r5.xyzx)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 239: mul r0.xyz, r0.xyzx, l(0.450000, 0.450000, 0.450000, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.450000,0.450000,0.450000,0.000000))).xyz;
    // 240: mul o0.xyz, r0.xyzx, cb0[21].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[21].xyzx)).xyz;
    // 241: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 242: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 243: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 244: ret
    return output;
}
```

### configure1526.h

```cpp
    if (staged.program == 0u && family == "source.character.equipment-native-1526.v1")
    {
        [&]() {
        staged.program = 1526u;
        staged.baseTextureMask = 127u;
        staged.baseConstants[2] = vector(parameter("selectioncolor"));
        staged.baseConstants[3] = vector(parameter("basecolor_color"));
        staged.baseConstants[4] = vector(parameter("diffusecolor_a"));
        staged.baseConstants[5] = vector(parameter("diffusecolor_b"));
        staged.baseConstants[6] = vector(parameter("diffusecolor_c"));
        staged.baseConstants[7] = vector(parameter("state"));
        staged.baseConstants[8] = vector(parameter("emissive_color"));
        staged.baseConstants[9] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.baseConstants[10] = vector(parameter("fx_color_intensity_actiontool"));
        staged.baseConstants[11] = vector(parameter("transcolor"));
        staged.baseConstants[12] = vector(parameter("buffcolor"));
        staged.baseConstants[13] = vector(parameter("hit_color"));
        staged.baseConstants[14] = vector(parameter("occlusion_color"));
        staged.baseConstants[15] = vector(append(Value{},Value{},1u));
        staged.baseConstants[16] = vector(parameter("mask_variation_visible"));
        staged.baseConstants[17] = vector(parameter("ssstintcolor"));
        staged.baseConstants[18] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("emissive_intensity")[0]);
        staged.baseConstants[19] = float4_t(parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0]);
        staged.baseConstants[20] = float4_t(parameter("trans_rim_hard")[0],parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],Value{}[0]);
        staged.baseConstants[21] = float4_t(parameter("auto_pbr_oc_max")[0],parameter("auto_pbr_oc_min")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0]);
        staged.baseConstants[22] = float4_t(parameter("pbr_specular")[0],parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0],multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0]);
        staged.baseConstants[23] = float4_t(parameter("roughness_power")[0],parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_c")[0]);
        staged.baseConstants[24] = float4_t(multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0],parameter("ssslocalthickness")[0],0.f,0.f);
        staged.lightTextureMask = 111u;
        staged.lightConstants[2] = vector(parameter("selectioncolor"));
        staged.lightConstants[3] = vector(parameter("basecolor_color"));
        staged.lightConstants[4] = vector(parameter("diffusecolor_a"));
        staged.lightConstants[5] = vector(parameter("diffusecolor_b"));
        staged.lightConstants[6] = vector(parameter("diffusecolor_c"));
        staged.lightConstants[7] = vector(parameter("state"));
        staged.lightConstants[8] = vector(parameter("fx_color_intensity_buffsettool"));
        staged.lightConstants[9] = vector(parameter("fx_color_intensity_actiontool"));
        staged.lightConstants[10] = vector(parameter("occlusion_color"));
        staged.lightConstants[11] = vector(append(Value{},Value{},1u));
        staged.lightConstants[12] = vector(parameter("mask_variation_visible"));
        staged.lightConstants[13] = vector(parameter("ssstintcolor"));
        staged.lightConstants[14] = float4_t(parameter("normaltex_intensity")[0],parameter("metallic_power")[0],parameter("pbr_add_basecolor_to_emissive")[0],parameter("emissive_intensity")[0]);
        staged.lightConstants[15] = float4_t(parameter("fx_color_desaturation_actiontool")[0],parameter("fx_color_desaturation_buffsettool")[0],parameter("transcolor_rimlight ")[0],parameter("trans_rim_inradius")[0]);
        staged.lightConstants[16] = float4_t(parameter("trans_rim_hard")[0],parameter("occlusion_power")[0],parameter("occlusion_brightness")[0],Value{}[0]);
        staged.lightConstants[17] = float4_t(parameter("auto_pbr_oc_max")[0],parameter("auto_pbr_oc_min")[0],parameter("occlusionbasecolor_blend_intensity")[0],bounded(parameter("occlusionbasecolor_blend_intensity"),Value{0.f,0.f,0.f,0.f},Value{1.f,0.f,0.f,0.f})[0]);
        staged.lightConstants[18] = float4_t(parameter("pbr_specular")[0],parameter("dye_roughness_power_tensition")[0],parameter("roughness_power_a")[0],multiply(parameter("roughness_power_a"),parameter("dye_roughness_power_tensition"))[0]);
        staged.lightConstants[19] = float4_t(parameter("roughness_power")[0],parameter("roughness_power_b")[0],multiply(parameter("roughness_power_b"),parameter("dye_roughness_power_tensition"))[0],parameter("roughness_power_c")[0]);
        staged.lightConstants[20] = float4_t(multiply(parameter("roughness_power_c"),parameter("dye_roughness_power_tensition"))[0],parameter("ssslocalthickness")[0],0.f,0.f);
        }();
    }
```

## G07. 추가 데이터 정본

기존 visualSets 끝에 다음 세 stable ID를 추가하며 기존 행은 유지한다.

```json
[
  {
    "visualSetId": "character.guardian_knight.source_ddk_01.outfit",
    "classId": "GUARDIANKNIGHT",
    "categoryId": "APPAREL_OUTFIT",
    "catalogStatus": "READY_ALTERNATIVE",
    "primarySlot": "UPPER",
    "occupiedSlots": [
      "UPPER",
      "LOWER",
      "HANDS",
      "SHOULDER",
      "HEAD"
    ],
    "parts": [
      {
        "partId": "upper",
        "partRole": "UPPER",
        "attachmentMode": "SKINNED",
        "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_01/upper.wmodel",
        "socketBoneId": null,
        "socketYawDegrees": 0.0,
        "requiredStance": null,
        "hiddenMeshMask": 0
      },
      {
        "partId": "lower",
        "partRole": "LOWER",
        "attachmentMode": "SKINNED",
        "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_01/lower.wmodel",
        "socketBoneId": null,
        "socketYawDegrees": 0.0,
        "requiredStance": null,
        "hiddenMeshMask": 0
      },
      {
        "partId": "arm",
        "partRole": "HANDS",
        "attachmentMode": "SKINNED",
        "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_01/arm.wmodel",
        "socketBoneId": null,
        "socketYawDegrees": 0.0,
        "requiredStance": null,
        "hiddenMeshMask": 0
      },
      {
        "partId": "shoulder",
        "partRole": "SHOULDER",
        "attachmentMode": "SKINNED",
        "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_01/shoulder.wmodel",
        "socketBoneId": null,
        "socketYawDegrees": 0.0,
        "requiredStance": null,
        "hiddenMeshMask": 0
      },
      {
        "partId": "helmet",
        "partRole": "HEAD",
        "attachmentMode": "SKINNED",
        "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_01/helmet.wmodel",
        "socketBoneId": null,
        "socketYawDegrees": 0.0,
        "requiredStance": null,
        "hiddenMeshMask": 0
      }
    ]
  },
  {
    "visualSetId": "character.guardian_knight.source_ddk_02.outfit",
    "classId": "GUARDIANKNIGHT",
    "categoryId": "APPAREL_OUTFIT",
    "catalogStatus": "READY_ALTERNATIVE",
    "primarySlot": "UPPER",
    "occupiedSlots": [
      "UPPER",
      "LOWER",
      "HANDS",
      "SHOULDER",
      "HEAD"
    ],
    "parts": [
      {
        "partId": "upper",
        "partRole": "UPPER",
        "attachmentMode": "SKINNED",
        "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_02/upper.wmodel",
        "socketBoneId": null,
        "socketYawDegrees": 0.0,
        "requiredStance": null,
        "hiddenMeshMask": 0
      },
      {
        "partId": "lower",
        "partRole": "LOWER",
        "attachmentMode": "SKINNED",
        "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_02/lower.wmodel",
        "socketBoneId": null,
        "socketYawDegrees": 0.0,
        "requiredStance": null,
        "hiddenMeshMask": 0
      },
      {
        "partId": "arm",
        "partRole": "HANDS",
        "attachmentMode": "SKINNED",
        "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_02/arm.wmodel",
        "socketBoneId": null,
        "socketYawDegrees": 0.0,
        "requiredStance": null,
        "hiddenMeshMask": 0
      },
      {
        "partId": "shoulder",
        "partRole": "SHOULDER",
        "attachmentMode": "SKINNED",
        "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_02/shoulder.wmodel",
        "socketBoneId": null,
        "socketYawDegrees": 0.0,
        "requiredStance": null,
        "hiddenMeshMask": 0
      },
      {
        "partId": "helmet",
        "partRole": "HEAD",
        "attachmentMode": "SKINNED",
        "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_02/helmet.wmodel",
        "socketBoneId": null,
        "socketYawDegrees": 0.0,
        "requiredStance": null,
        "hiddenMeshMask": 0
      }
    ]
  },
  {
    "visualSetId": "character.guardian_knight.source_ddk_03.outfit",
    "classId": "GUARDIANKNIGHT",
    "categoryId": "APPAREL_OUTFIT",
    "catalogStatus": "READY_ALTERNATIVE",
    "primarySlot": "UPPER",
    "occupiedSlots": [
      "UPPER",
      "LOWER",
      "HANDS",
      "SHOULDER",
      "HEAD"
    ],
    "parts": [
      {
        "partId": "upper",
        "partRole": "UPPER",
        "attachmentMode": "SKINNED",
        "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_03/upper.wmodel",
        "socketBoneId": null,
        "socketYawDegrees": 0.0,
        "requiredStance": null,
        "hiddenMeshMask": 0
      },
      {
        "partId": "lower",
        "partRole": "LOWER",
        "attachmentMode": "SKINNED",
        "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_03/lower.wmodel",
        "socketBoneId": null,
        "socketYawDegrees": 0.0,
        "requiredStance": null,
        "hiddenMeshMask": 0
      },
      {
        "partId": "arm",
        "partRole": "HANDS",
        "attachmentMode": "SKINNED",
        "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_03/arm.wmodel",
        "socketBoneId": null,
        "socketYawDegrees": 0.0,
        "requiredStance": null,
        "hiddenMeshMask": 0
      },
      {
        "partId": "shoulder",
        "partRole": "SHOULDER",
        "attachmentMode": "SKINNED",
        "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_03/shoulder.wmodel",
        "socketBoneId": null,
        "socketYawDegrees": 0.0,
        "requiredStance": null,
        "hiddenMeshMask": 0
      },
      {
        "partId": "helmet",
        "partRole": "HEAD",
        "attachmentMode": "SKINNED",
        "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_03/helmet.wmodel",
        "socketBoneId": null,
        "socketYawDegrees": 0.0,
        "requiredStance": null,
        "hiddenMeshMask": 0
      }
    ]
  }
]
```

기존 root modelMaterialOverrides 끝에 아래34개 (modelAssetId, materialName) 행을 추가한다.
source package가 다른 동명 texture는 각 행의 sourceMaterial과 assetId를 함께 유지한다.

```json
[
  {
    "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_01/upper.wmodel",
    "materialName": "pc_ddk_01_upper1_mi",
    "sourceMaterial": "PC_DDK_01.mat.pc_ddk_01_upper1_mi",
    "family": "source.character.equipment-native-1526.v1",
    "parameters": {
      "auto_pbr_oc_max": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "auto_pbr_oc_min": [
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896
      ],
      "basecolor_color": [
        0.19888770580291748,
        0.10462038964033127,
        0.11046764254570007,
        1.0
      ],
      "buffcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "diffusecolor_a": [
        0.15100732445716858,
        0.0795094221830368,
        0.08264787495136261,
        1.0
      ],
      "diffusecolor_b": [
        0.06629589945077896,
        0.031623076647520065,
        0.03696228936314583,
        1.0
      ],
      "diffusecolor_c": [
        0.06190747395157814,
        0.03423020616173744,
        0.038472745567560196,
        1.0
      ],
      "dye_roughness_power_tensition": [
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448
      ],
      "emissive_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "emissive_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "fx_color_desaturation_actiontool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_desaturation_buffsettool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_intensity_actiontool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "fx_color_intensity_buffsettool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "hit_color": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "mask_variation_visible": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "metallic_power": [
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896
      ],
      "normaltex_intensity": [
        1.2000000476837158,
        1.2000000476837158,
        1.2000000476837158,
        1.2000000476837158
      ],
      "occlusion_brightness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "occlusion_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusionbasecolor_blend_intensity": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "pbr_add_basecolor_to_emissive": [
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806
      ],
      "pbr_specular": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "roughness_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "roughness_power_a": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_b": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_c": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "selectioncolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "ssslocalthickness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "ssstintcolor": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "state": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "trans_rim_hard": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "trans_rim_inradius": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "transcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "transcolor_rimlight ": [
        1.0,
        1.0,
        1.0,
        1.0
      ]
    },
    "textures": [
      {
        "expressionIndex": 0,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_upper_01_n.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 1,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_upper_01_d.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 2,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_upper_01_orm.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 3,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_upper_01_cm.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 4,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_upper_01_e.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 5,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 6,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_upper_01_m.dds",
        "colorSpace": "srgb"
      }
    ]
  },
  {
    "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_01/upper.wmodel",
    "materialName": "pc_ddk_01_upper2_mi",
    "sourceMaterial": "PC_DDK_01.mat.pc_ddk_01_upper2_mi",
    "family": "source.character.realpbr-avatar-ddk.v1",
    "parameters": {
      "auto_pbr_oc_max": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "auto_pbr_oc_min": [
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896
      ],
      "basecolor_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "buffcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "diffusecolor_a": [
        0.0427197702229023,
        0.04051397740840912,
        0.04972192645072937,
        1.0
      ],
      "diffusecolor_b": [
        0.12506848573684692,
        0.13037224113941193,
        0.13259179890155792,
        1.0
      ],
      "diffusecolor_c": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "dye_roughness_power_tensition": [
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448
      ],
      "fx_color_desaturation_actiontool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_desaturation_buffsettool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_intensity_actiontool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "fx_color_intensity_buffsettool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "hit_color": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "mask_variation_visible": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "metallic_power": [
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896
      ],
      "normaltex_intensity": [
        1.2000000476837158,
        1.2000000476837158,
        1.2000000476837158,
        1.2000000476837158
      ],
      "occlusion_brightness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "occlusion_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusionbasecolor_blend_intensity": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "pbr_add_basecolor_to_emissive": [
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806
      ],
      "pbr_specular": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "roughness_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "roughness_power_a": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_b": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_c": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "selectioncolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "ssslocalthickness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "ssstintcolor": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "state": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "trans_rim_hard": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "trans_rim_inradius": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "transcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "transcolor_rimlight ": [
        1.0,
        1.0,
        1.0,
        1.0
      ]
    },
    "textures": [
      {
        "expressionIndex": 0,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_upper_02_n.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 1,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_upper_02_d.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 2,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_upper_02_orm.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 3,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_upper_02_cm.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 4,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 5,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_upper_02_m.dds",
        "colorSpace": "srgb"
      }
    ]
  },
  {
    "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_01/upper.wmodel",
    "materialName": "pc_ddk_01_upper3_mi",
    "sourceMaterial": "PC_DDK_01.mat.pc_ddk_01_upper3_mi",
    "family": "source.character.realpbr-avatar-ddk.v1",
    "parameters": {
      "auto_pbr_oc_max": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "auto_pbr_oc_min": [
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896
      ],
      "basecolor_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "buffcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "diffusecolor_a": [
        0.0427197702229023,
        0.04051397740840912,
        0.04972192645072937,
        1.0
      ],
      "diffusecolor_b": [
        0.12506848573684692,
        0.13037224113941193,
        0.13259179890155792,
        1.0
      ],
      "diffusecolor_c": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "dye_roughness_power_tensition": [
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448
      ],
      "fx_color_desaturation_actiontool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_desaturation_buffsettool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_intensity_actiontool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "fx_color_intensity_buffsettool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "hit_color": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "mask_variation_visible": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "metallic_power": [
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896
      ],
      "normaltex_intensity": [
        1.2000000476837158,
        1.2000000476837158,
        1.2000000476837158,
        1.2000000476837158
      ],
      "occlusion_brightness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "occlusion_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusionbasecolor_blend_intensity": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "pbr_add_basecolor_to_emissive": [
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806
      ],
      "pbr_specular": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "roughness_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "roughness_power_a": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_b": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_c": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "selectioncolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "ssslocalthickness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "ssstintcolor": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "state": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "trans_rim_hard": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "trans_rim_inradius": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "transcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "transcolor_rimlight ": [
        1.0,
        1.0,
        1.0,
        1.0
      ]
    },
    "textures": [
      {
        "expressionIndex": 0,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_upper_03_n.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 1,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_upper_03_d.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 2,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_upper_03_orm.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 3,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_upper_03_cm.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 4,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 5,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_upper_03_m.dds",
        "colorSpace": "srgb"
      }
    ]
  },
  {
    "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_01/upper.wmodel",
    "materialName": "pc_ddk_01_upper4_mi",
    "sourceMaterial": "PC_DDK_01.mat.pc_ddk_01_upper4_mi",
    "family": "source.character.realpbr-avatar-ddk.v1",
    "parameters": {
      "auto_pbr_oc_max": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "auto_pbr_oc_min": [
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896
      ],
      "basecolor_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "buffcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "diffusecolor_a": [
        0.0427197702229023,
        0.04051397740840912,
        0.04972192645072937,
        1.0
      ],
      "diffusecolor_b": [
        0.12506848573684692,
        0.13037224113941193,
        0.13259179890155792,
        1.0
      ],
      "diffusecolor_c": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "dye_roughness_power_tensition": [
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448
      ],
      "fx_color_desaturation_actiontool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_desaturation_buffsettool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_intensity_actiontool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "fx_color_intensity_buffsettool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "hit_color": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "mask_variation_visible": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "metallic_power": [
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896
      ],
      "normaltex_intensity": [
        1.2000000476837158,
        1.2000000476837158,
        1.2000000476837158,
        1.2000000476837158
      ],
      "occlusion_brightness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "occlusion_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusionbasecolor_blend_intensity": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "pbr_add_basecolor_to_emissive": [
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806
      ],
      "pbr_specular": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "roughness_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "roughness_power_a": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_b": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_c": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "selectioncolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "ssslocalthickness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "ssstintcolor": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "state": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "trans_rim_hard": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "trans_rim_inradius": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "transcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "transcolor_rimlight ": [
        1.0,
        1.0,
        1.0,
        1.0
      ]
    },
    "textures": [
      {
        "expressionIndex": 0,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_upper_04_n.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 1,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_upper_04_d.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 2,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_upper_04_orm.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 3,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_upper_04_cm.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 4,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 5,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_upper_04_m.dds",
        "colorSpace": "srgb"
      }
    ]
  },
  {
    "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_01/upper.wmodel",
    "materialName": "pc_ddk_01_base_upper_mi",
    "sourceMaterial": "PC_DDK_01.mat.pc_ddk_01_base_upper_mi",
    "family": "source.character.classic-skin.v1",
    "parameters": {
      "1.use_dyeing_sp": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "beckmannspecular_constant_max": [
        3.3499999046325684,
        3.3499999046325684,
        3.3499999046325684,
        3.3499999046325684
      ],
      "buffcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "diffusecolor": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "fresnel_radius": [
        0.949999988079071,
        0.949999988079071,
        0.949999988079071,
        0.949999988079071
      ],
      "fresnel_rimlightintensity": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "fx_color_desaturation_actiontool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_desaturation_buffsettool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_intensity_actiontool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "fx_color_intensity_buffsettool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "half_lambert_skin": [
        1.0,
        0.1173890009522438,
        0.1080550029873848,
        1.0
      ],
      "hit_color": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "ibl_color_bottom": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "ibl_color_top": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "ibl_exposer": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "ibl_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "ibl_normal_smooth": [
        0.699999988079071,
        0.699999988079071,
        0.699999988079071,
        0.699999988079071
      ],
      "ibl_reflect_lodbias": [
        80.0,
        80.0,
        80.0,
        80.0
      ],
      "ibl_skinlodscale": [
        0.800000011920929,
        0.800000011920929,
        0.800000011920929,
        0.800000011920929
      ],
      "metalicness_power": [
        0.20000000298023224,
        0.20000000298023224,
        0.20000000298023224,
        0.20000000298023224
      ],
      "normaltex_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "orennayar": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "orennayar_brightness": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "pbr_specular_intensity": [
        10.0,
        10.0,
        10.0,
        10.0
      ],
      "pbr_specular_power": [
        6.0,
        6.0,
        6.0,
        6.0
      ],
      "roughness_power": [
        3.0,
        3.0,
        3.0,
        3.0
      ],
      "selectioncolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "shadowfactor": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "skin_metalicness_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "skin_normal_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "skin_roughness_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "skin_specular_intensity": [
        0.4000000059604645,
        0.4000000059604645,
        0.4000000059604645,
        0.4000000059604645
      ],
      "skin_specular_intensity_min": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "skin_specular_power": [
        20.0,
        20.0,
        20.0,
        20.0
      ],
      "skin_specular_power_min": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "specular_power_limit": [
        0.9200000166893005,
        0.9200000166893005,
        0.9200000166893005,
        0.9200000166893005
      ],
      "state": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "state_noise": [
        1.0,
        0.0,
        0.0,
        1.0
      ],
      "trans_rim_hard": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "trans_rim_inradius": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "transcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "transcolor_rimlight ": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "var_base_skincolor_ui": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "var_base_skinnormalintensity_ui": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "var_base_skinspecularintensity_ui": [
        0.6000000238418579,
        0.6000000238418579,
        0.6000000238418579,
        0.6000000238418579
      ],
      "var_base_skinspecularpower_ui": [
        0.5,
        0.5,
        0.5,
        0.5
      ]
    },
    "textures": [
      {
        "expressionIndex": 0,
        "assetId": "Character/SourceMaterials/pc_dk_av_basebody/pc_dk_av_base_upper_n_loc_int.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 1,
        "assetId": "Character/SourceMaterials/pc_dk_av_basebody/pc_dk_av_base_upper_cm.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 2,
        "assetId": "Character/SourceMaterials/pc_dk_av_basebody/pc_dk_av_base_upper_s.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 3,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_dk_av_base_upper_d.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 4,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/hdr07_1.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 5,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 6,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/brdf_beckmann_spec.dds",
        "colorSpace": "linear"
      }
    ]
  },
  {
    "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_01/lower.wmodel",
    "materialName": "pc_ddk_01_lower1_mi",
    "sourceMaterial": "PC_DDK_01.mat.pc_ddk_01_lower1_mi",
    "family": "source.character.realpbr-avatar-ddk.v1",
    "parameters": {
      "auto_pbr_oc_max": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "auto_pbr_oc_min": [
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896
      ],
      "basecolor_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "buffcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "diffusecolor_a": [
        0.06261279433965683,
        0.03549452871084213,
        0.03950466215610504,
        1.0
      ],
      "diffusecolor_b": [
        0.09944385290145874,
        0.09944385290145874,
        0.09944385290145874,
        1.0
      ],
      "diffusecolor_c": [
        0.022012995555996895,
        0.022012995555996895,
        0.022012995555996895,
        1.0
      ],
      "dye_roughness_power_tensition": [
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448
      ],
      "fx_color_desaturation_actiontool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_desaturation_buffsettool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_intensity_actiontool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "fx_color_intensity_buffsettool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "hit_color": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "mask_variation_visible": [
        0.0,
        1.0,
        0.0,
        0.0
      ],
      "metallic_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "normaltex_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_brightness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "occlusion_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusionbasecolor_blend_intensity": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "pbr_add_basecolor_to_emissive": [
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806
      ],
      "pbr_specular": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "roughness_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "roughness_power_a": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_b": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_c": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "selectioncolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "ssslocalthickness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "ssstintcolor": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "state": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "trans_rim_hard": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "trans_rim_inradius": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "transcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "transcolor_rimlight ": [
        1.0,
        1.0,
        1.0,
        1.0
      ]
    },
    "textures": [
      {
        "expressionIndex": 0,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_lower_01_n.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 1,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_lower_01_d.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 2,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_lower_01_orm.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 3,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_lower_01_cm.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 4,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 5,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_upper_01_m.dds",
        "colorSpace": "srgb"
      }
    ]
  },
  {
    "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_01/arm.wmodel",
    "materialName": "pc_ddk_01_arm_mi",
    "sourceMaterial": "PC_DDK_01.mat.pc_ddk_01_arm_mi",
    "family": "source.character.realpbr-avatar-ddk.v1",
    "parameters": {
      "auto_pbr_oc_max": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "auto_pbr_oc_min": [
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896
      ],
      "basecolor_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "buffcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "diffusecolor_a": [
        0.44197267293930054,
        0.44197267293930054,
        0.44197267293930054,
        1.0
      ],
      "diffusecolor_b": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "diffusecolor_c": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "dye_roughness_power_tensition": [
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448
      ],
      "fx_color_desaturation_actiontool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_desaturation_buffsettool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_intensity_actiontool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "fx_color_intensity_buffsettool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "hit_color": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "mask_variation_visible": [
        1.0,
        1.0,
        0.0,
        0.0
      ],
      "metallic_power": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "normaltex_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_brightness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "occlusion_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusionbasecolor_blend_intensity": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "pbr_add_basecolor_to_emissive": [
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806
      ],
      "pbr_specular": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "roughness_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "roughness_power_a": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_b": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_c": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "selectioncolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "ssslocalthickness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "ssstintcolor": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "state": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "trans_rim_hard": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "trans_rim_inradius": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "transcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "transcolor_rimlight ": [
        1.0,
        1.0,
        1.0,
        1.0
      ]
    },
    "textures": [
      {
        "expressionIndex": 0,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_arm_n.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 1,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_arm_d.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 2,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_arm_orm.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 3,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_arm_cm.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 4,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 5,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_arm_m.dds",
        "colorSpace": "srgb"
      }
    ]
  },
  {
    "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_01/arm.wmodel",
    "materialName": "pc_ddk_01_base_upper_mi",
    "sourceMaterial": "PC_DDK_01.mat.pc_ddk_01_base_upper_mi",
    "family": "source.character.classic-skin.v1",
    "parameters": {
      "1.use_dyeing_sp": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "beckmannspecular_constant_max": [
        3.3499999046325684,
        3.3499999046325684,
        3.3499999046325684,
        3.3499999046325684
      ],
      "buffcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "diffusecolor": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "fresnel_radius": [
        0.949999988079071,
        0.949999988079071,
        0.949999988079071,
        0.949999988079071
      ],
      "fresnel_rimlightintensity": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "fx_color_desaturation_actiontool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_desaturation_buffsettool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_intensity_actiontool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "fx_color_intensity_buffsettool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "half_lambert_skin": [
        1.0,
        0.1173890009522438,
        0.1080550029873848,
        1.0
      ],
      "hit_color": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "ibl_color_bottom": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "ibl_color_top": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "ibl_exposer": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "ibl_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "ibl_normal_smooth": [
        0.699999988079071,
        0.699999988079071,
        0.699999988079071,
        0.699999988079071
      ],
      "ibl_reflect_lodbias": [
        80.0,
        80.0,
        80.0,
        80.0
      ],
      "ibl_skinlodscale": [
        0.800000011920929,
        0.800000011920929,
        0.800000011920929,
        0.800000011920929
      ],
      "metalicness_power": [
        0.20000000298023224,
        0.20000000298023224,
        0.20000000298023224,
        0.20000000298023224
      ],
      "normaltex_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "orennayar": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "orennayar_brightness": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "pbr_specular_intensity": [
        10.0,
        10.0,
        10.0,
        10.0
      ],
      "pbr_specular_power": [
        6.0,
        6.0,
        6.0,
        6.0
      ],
      "roughness_power": [
        3.0,
        3.0,
        3.0,
        3.0
      ],
      "selectioncolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "shadowfactor": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "skin_metalicness_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "skin_normal_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "skin_roughness_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "skin_specular_intensity": [
        0.4000000059604645,
        0.4000000059604645,
        0.4000000059604645,
        0.4000000059604645
      ],
      "skin_specular_intensity_min": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "skin_specular_power": [
        20.0,
        20.0,
        20.0,
        20.0
      ],
      "skin_specular_power_min": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "specular_power_limit": [
        0.9200000166893005,
        0.9200000166893005,
        0.9200000166893005,
        0.9200000166893005
      ],
      "state": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "state_noise": [
        1.0,
        0.0,
        0.0,
        1.0
      ],
      "trans_rim_hard": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "trans_rim_inradius": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "transcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "transcolor_rimlight ": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "var_base_skincolor_ui": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "var_base_skinnormalintensity_ui": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "var_base_skinspecularintensity_ui": [
        0.6000000238418579,
        0.6000000238418579,
        0.6000000238418579,
        0.6000000238418579
      ],
      "var_base_skinspecularpower_ui": [
        0.5,
        0.5,
        0.5,
        0.5
      ]
    },
    "textures": [
      {
        "expressionIndex": 0,
        "assetId": "Character/SourceMaterials/pc_dk_av_basebody/pc_dk_av_base_upper_n_loc_int.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 1,
        "assetId": "Character/SourceMaterials/pc_dk_av_basebody/pc_dk_av_base_upper_cm.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 2,
        "assetId": "Character/SourceMaterials/pc_dk_av_basebody/pc_dk_av_base_upper_s.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 3,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_dk_av_base_upper_d.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 4,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/hdr07_1.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 5,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 6,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/brdf_beckmann_spec.dds",
        "colorSpace": "linear"
      }
    ]
  },
  {
    "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_01/shoulder.wmodel",
    "materialName": "pc_ddk_01_shoulder_mi",
    "sourceMaterial": "PC_DDK_01.mat.pc_ddk_01_shoulder_mi",
    "family": "source.character.realpbr-avatar-ddk-plate.v1",
    "parameters": {
      "1.use_emissive_flickerspeed_fixed": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "auto_pbr_oc_max": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "auto_pbr_oc_min": [
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896
      ],
      "basecolor_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "buffcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "diffusecolor_a": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "diffusecolor_b": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "diffusecolor_c": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "dye_roughness_power_tensition": [
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448
      ],
      "emissive_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "emissive_flicker_speed": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "emissive_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "emissive_intensitymin": [
        2.0,
        2.0,
        2.0,
        2.0
      ],
      "fx_color_desaturation_actiontool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_desaturation_buffsettool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_intensity_actiontool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "fx_color_intensity_buffsettool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "hit_color": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "mask_variation_visible": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "metallic_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "normaltex_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_brightness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "occlusion_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusionbasecolor_blend_intensity": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "pbr_add_basecolor_to_emissive": [
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806
      ],
      "pbr_specular": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "roughness_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "roughness_power_a": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_b": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_c": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "selectioncolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "ssslocalthickness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "ssstintcolor": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "state": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "trans_rim_hard": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "trans_rim_inradius": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "transcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "transcolor_rimlight ": [
        1.0,
        1.0,
        1.0,
        1.0
      ]
    },
    "textures": [
      {
        "expressionIndex": 0,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_shoulder2_n.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 1,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_shoulder2_d.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 2,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_shoulder2_orm.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 3,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_shoulder2_cm.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 4,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_shoulder2_e.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 5,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 6,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_shoulder2_m.dds",
        "colorSpace": "srgb"
      }
    ]
  },
  {
    "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_01/shoulder.wmodel",
    "materialName": "pc_ddk_01_shoulder1_mi",
    "sourceMaterial": "PC_DDK_01.mat.pc_ddk_01_shoulder1_mi",
    "family": "source.character.equipment-native-198.v1",
    "parameters": {
      "1.use_emissive_flickerspeed_fixed": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "auto_pbr_oc_max": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "auto_pbr_oc_min": [
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896
      ],
      "basecolor_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "buffcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "diffusecolor_a": [
        0.33884570002555847,
        0.33884570002555847,
        0.33884570002555847,
        1.0
      ],
      "diffusecolor_b": [
        1.0,
        0.15066486597061157,
        0.38099581003189087,
        1.0
      ],
      "diffusecolor_c": [
        0.46775442361831665,
        0.011768095195293427,
        0.009492557495832443,
        1.0
      ],
      "dye_roughness_power_tensition": [
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448
      ],
      "emissive_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "emissive_flicker_speed": [
        0.4000000059604645,
        0.4000000059604645,
        0.4000000059604645,
        0.4000000059604645
      ],
      "emissive_intensity": [
        1.5,
        1.5,
        1.5,
        1.5
      ],
      "emissive_intensitymin": [
        0.800000011920929,
        0.800000011920929,
        0.800000011920929,
        0.800000011920929
      ],
      "fx_color_desaturation_actiontool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_desaturation_buffsettool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_intensity_actiontool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "fx_color_intensity_buffsettool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "hit_color": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "mask_variation_visible": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "metallic_power": [
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896
      ],
      "normaltex_intensity": [
        1.2000000476837158,
        1.2000000476837158,
        1.2000000476837158,
        1.2000000476837158
      ],
      "occlusion_brightness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "occlusion_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusionbasecolor_blend_intensity": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "pbr_add_basecolor_to_emissive": [
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806
      ],
      "pbr_specular": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "roughness_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "roughness_power_a": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_b": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_c": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "selectioncolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "ssslocalthickness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "ssstintcolor": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "state": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "trans_rim_hard": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "trans_rim_inradius": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "transcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "transcolor_rimlight ": [
        1.0,
        1.0,
        1.0,
        1.0
      ]
    },
    "textures": [
      {
        "expressionIndex": 0,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_upper_01_n.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 1,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_upper_01_d.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 2,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_upper_01_orm.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 3,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_shoulder1_cm.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 4,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_upper_01_e.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 5,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 6,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_upper_01_m.dds",
        "colorSpace": "srgb"
      }
    ]
  },
  {
    "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_01/helmet.wmodel",
    "materialName": "pc_ddk_01_helmet_mi",
    "sourceMaterial": "PC_DDK_01.mat.pc_ddk_01_helmet_mi",
    "family": "source.character.equipment-native-198.v1",
    "parameters": {
      "1.use_emissive_flickerspeed_fixed": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "auto_pbr_oc_max": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "auto_pbr_oc_min": [
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896
      ],
      "basecolor_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "buffcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "diffusecolor_a": [
        0.2725498080253601,
        0.2725498080253601,
        0.2725498080253601,
        1.0
      ],
      "diffusecolor_b": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "diffusecolor_c": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "dye_roughness_power_tensition": [
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448
      ],
      "emissive_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "emissive_flicker_speed": [
        0.699999988079071,
        0.699999988079071,
        0.699999988079071,
        0.699999988079071
      ],
      "emissive_intensity": [
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896
      ],
      "emissive_intensitymin": [
        3.0,
        3.0,
        3.0,
        3.0
      ],
      "fx_color_desaturation_actiontool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_desaturation_buffsettool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_intensity_actiontool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "fx_color_intensity_buffsettool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "hit_color": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "mask_variation_visible": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "metallic_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "normaltex_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_brightness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "occlusion_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusionbasecolor_blend_intensity": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "pbr_add_basecolor_to_emissive": [
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806
      ],
      "pbr_specular": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "roughness_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "roughness_power_a": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_b": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_c": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "selectioncolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "ssslocalthickness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "ssstintcolor": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "state": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "trans_rim_hard": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "trans_rim_inradius": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "transcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "transcolor_rimlight ": [
        1.0,
        1.0,
        1.0,
        1.0
      ]
    },
    "textures": [
      {
        "expressionIndex": 0,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_helmet_n.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 1,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_helmet_d.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 2,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_helmet_orm.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 3,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_helmet_cm.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 4,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_helmet_e.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 5,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 6,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_01/textures/pc_ddk_01_helmet_m.dds",
        "colorSpace": "srgb"
      }
    ]
  },
  {
    "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_02/upper.wmodel",
    "materialName": "pc_ddk_02_upper_mi",
    "sourceMaterial": "PC_DDK_02.mat.pc_ddk_02_upper_mi",
    "family": "source.character.realpbr-avatar-ddk-plate.v1",
    "parameters": {
      "1.use_emissive_flickerspeed_fixed": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "auto_pbr_oc_max": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "auto_pbr_oc_min": [
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896
      ],
      "basecolor_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "buffcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "diffusecolor_a": [
        0.3314794898033142,
        0.17134490609169006,
        0.15017971396446228,
        1.0
      ],
      "diffusecolor_b": [
        0.07592611759901047,
        0.0041161770932376385,
        0.0,
        1.0
      ],
      "diffusecolor_c": [
        0.25413429737091064,
        0.07300678640604019,
        0.029493482783436775,
        1.0
      ],
      "dye_roughness_power_tensition": [
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448
      ],
      "emissive_color": [
        0.1491657793521881,
        0.028071079403162003,
        0.009070482105016708,
        1.0
      ],
      "emissive_flicker_speed": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "emissive_intensity": [
        2.0,
        2.0,
        2.0,
        2.0
      ],
      "emissive_intensitymin": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "fx_color_desaturation_actiontool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_desaturation_buffsettool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_intensity_actiontool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "fx_color_intensity_buffsettool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "hit_color": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "mask_variation_visible": [
        1.0,
        0.0,
        0.0,
        0.0
      ],
      "metallic_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "normaltex_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_brightness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "occlusion_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_power": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "occlusionbasecolor_blend_intensity": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "pbr_add_basecolor_to_emissive": [
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806
      ],
      "pbr_specular": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "roughness_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "roughness_power_a": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_b": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_c": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "selectioncolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "ssslocalthickness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "ssstintcolor": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "state": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "trans_rim_hard": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "trans_rim_inradius": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "transcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "transcolor_rimlight ": [
        1.0,
        1.0,
        1.0,
        1.0
      ]
    },
    "textures": [
      {
        "expressionIndex": 0,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_02/textures/pc_ddk_02_upper_n.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 1,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_02/textures/pc_ddk_02_upper_d.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 2,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_02/textures/pc_ddk_02_upper_orm.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 3,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_02/textures/pc_ddk_02_upper_cm.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 4,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_02/textures/pc_ddk_02_upper_e.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 5,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 6,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_02/textures/pc_ddk_02_upper_m.dds",
        "colorSpace": "srgb"
      }
    ]
  },
  {
    "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_02/upper.wmodel",
    "materialName": "pc_ddk_02_upper1_mi",
    "sourceMaterial": "PC_DDK_02.mat.pc_ddk_02_upper1_mi",
    "family": "source.character.realpbr-avatar-ddk.v1",
    "parameters": {
      "auto_pbr_oc_max": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "auto_pbr_oc_min": [
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896
      ],
      "basecolor_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "buffcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "diffusecolor_a": [
        0.40982574224472046,
        0.20471012592315674,
        0.17677432298660278,
        1.0
      ],
      "diffusecolor_b": [
        0.060771241784095764,
        0.003438194515183568,
        0.0,
        1.0
      ],
      "diffusecolor_c": [
        0.2508402466773987,
        0.07176145166158676,
        0.02899118699133396,
        1.0
      ],
      "dye_roughness_power_tensition": [
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448
      ],
      "fx_color_desaturation_actiontool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_desaturation_buffsettool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_intensity_actiontool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "fx_color_intensity_buffsettool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "hit_color": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "mask_variation_visible": [
        1.0,
        0.0,
        1.0,
        0.0
      ],
      "metallic_power": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "normaltex_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_brightness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "occlusion_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_power": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "occlusionbasecolor_blend_intensity": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "pbr_add_basecolor_to_emissive": [
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806
      ],
      "pbr_specular": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "roughness_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "roughness_power_a": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_b": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_c": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "selectioncolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "ssslocalthickness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "ssstintcolor": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "state": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "trans_rim_hard": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "trans_rim_inradius": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "transcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "transcolor_rimlight ": [
        1.0,
        1.0,
        1.0,
        1.0
      ]
    },
    "textures": [
      {
        "expressionIndex": 0,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_02/textures/pc_ddk_02_upper1_n.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 1,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_02/textures/pc_ddk_02_upper1_d.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 2,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_02/textures/pc_ddk_02_upper1_orm.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 3,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_02/textures/pc_ddk_02_upper1_cm.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 4,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 5,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_02/textures/pc_ddk_02_upper1_m.dds",
        "colorSpace": "srgb"
      }
    ]
  },
  {
    "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_02/upper.wmodel",
    "materialName": "pc_ddk_02_upper2_mi",
    "sourceMaterial": "PC_DDK_02.mat.pc_ddk_02_upper2_mi",
    "family": "source.character.realpbr-avatar-ddk.v1",
    "parameters": {
      "auto_pbr_oc_max": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "auto_pbr_oc_min": [
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896
      ],
      "basecolor_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "buffcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "diffusecolor_a": [
        0.32972902059555054,
        0.1701383739709854,
        0.14799802005290985,
        1.0
      ],
      "diffusecolor_b": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "diffusecolor_c": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "dye_roughness_power_tensition": [
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448
      ],
      "fx_color_desaturation_actiontool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_desaturation_buffsettool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_intensity_actiontool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "fx_color_intensity_buffsettool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "hit_color": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "mask_variation_visible": [
        0.0,
        1.0,
        0.0,
        0.0
      ],
      "metallic_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "normaltex_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_brightness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "occlusion_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusionbasecolor_blend_intensity": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "pbr_add_basecolor_to_emissive": [
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806
      ],
      "pbr_specular": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "roughness_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "roughness_power_a": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_b": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_c": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "selectioncolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "ssslocalthickness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "ssstintcolor": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "state": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "trans_rim_hard": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "trans_rim_inradius": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "transcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "transcolor_rimlight ": [
        1.0,
        1.0,
        1.0,
        1.0
      ]
    },
    "textures": [
      {
        "expressionIndex": 0,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_02/textures/pc_ddk_02_upper2_n.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 1,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_02/textures/pc_ddk_02_upper2_d.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 2,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_02/textures/pc_ddk_02_upper2_orm.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 3,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_02/textures/pc_ddk_02_upper2_cm.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 4,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 5,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_02/textures/pc_ddk_02_upper2_m.dds",
        "colorSpace": "srgb"
      }
    ]
  },
  {
    "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_02/upper.wmodel",
    "materialName": "pc_ddk_02_upper3_mi",
    "sourceMaterial": "PC_DDK_02.mat.pc_ddk_02_upper3_mi",
    "family": "source.character.realpbr-avatar-ddk.v1",
    "parameters": {
      "auto_pbr_oc_max": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "auto_pbr_oc_min": [
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896
      ],
      "basecolor_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "buffcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "diffusecolor_a": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "diffusecolor_b": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "diffusecolor_c": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "dye_roughness_power_tensition": [
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448
      ],
      "fx_color_desaturation_actiontool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_desaturation_buffsettool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_intensity_actiontool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "fx_color_intensity_buffsettool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "hit_color": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "mask_variation_visible": [
        0.0,
        1.0,
        1.0,
        0.0
      ],
      "metallic_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "normaltex_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_brightness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "occlusion_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_power": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "occlusionbasecolor_blend_intensity": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "pbr_add_basecolor_to_emissive": [
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806
      ],
      "pbr_specular": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "roughness_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "roughness_power_a": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_b": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_c": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "selectioncolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "ssslocalthickness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "ssstintcolor": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "state": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "trans_rim_hard": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "trans_rim_inradius": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "transcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "transcolor_rimlight ": [
        1.0,
        1.0,
        1.0,
        1.0
      ]
    },
    "textures": [
      {
        "expressionIndex": 0,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_02/textures/pc_ddk_02_upper3_n.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 1,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_02/textures/pc_ddk_02_upper3_d.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 2,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_02/textures/pc_ddk_02_upper3_orm.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 3,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/flat_black.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 4,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 5,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_02/textures/pc_ddk_02_upper3_m.dds",
        "colorSpace": "srgb"
      }
    ]
  },
  {
    "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_02/upper.wmodel",
    "materialName": "pc_dk_av_base_upper_mi",
    "sourceMaterial": "PC_DK_AV_BASEBODY.mat.pc_dk_av_base_upper_mi",
    "family": "source.character.classic-skin.v1",
    "parameters": {
      "1.use_dyeing_sp": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "beckmannspecular_constant_max": [
        3.3499999046325684,
        3.3499999046325684,
        3.3499999046325684,
        3.3499999046325684
      ],
      "buffcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "diffusecolor": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "fresnel_radius": [
        0.949999988079071,
        0.949999988079071,
        0.949999988079071,
        0.949999988079071
      ],
      "fresnel_rimlightintensity": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "fx_color_desaturation_actiontool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_desaturation_buffsettool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_intensity_actiontool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "fx_color_intensity_buffsettool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "half_lambert_skin": [
        1.0,
        0.1173890009522438,
        0.1080550029873848,
        1.0
      ],
      "hit_color": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "ibl_color_bottom": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "ibl_color_top": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "ibl_exposer": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "ibl_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "ibl_normal_smooth": [
        0.699999988079071,
        0.699999988079071,
        0.699999988079071,
        0.699999988079071
      ],
      "ibl_reflect_lodbias": [
        80.0,
        80.0,
        80.0,
        80.0
      ],
      "ibl_skinlodscale": [
        0.800000011920929,
        0.800000011920929,
        0.800000011920929,
        0.800000011920929
      ],
      "metalicness_power": [
        0.20000000298023224,
        0.20000000298023224,
        0.20000000298023224,
        0.20000000298023224
      ],
      "normaltex_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "orennayar": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "orennayar_brightness": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "pbr_specular_intensity": [
        10.0,
        10.0,
        10.0,
        10.0
      ],
      "pbr_specular_power": [
        6.0,
        6.0,
        6.0,
        6.0
      ],
      "roughness_power": [
        3.0,
        3.0,
        3.0,
        3.0
      ],
      "selectioncolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "shadowfactor": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "skin_metalicness_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "skin_normal_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "skin_roughness_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "skin_specular_intensity": [
        0.4000000059604645,
        0.4000000059604645,
        0.4000000059604645,
        0.4000000059604645
      ],
      "skin_specular_intensity_min": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "skin_specular_power": [
        20.0,
        20.0,
        20.0,
        20.0
      ],
      "skin_specular_power_min": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "specular_power_limit": [
        0.9200000166893005,
        0.9200000166893005,
        0.9200000166893005,
        0.9200000166893005
      ],
      "state": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "state_noise": [
        1.0,
        0.0,
        0.0,
        1.0
      ],
      "trans_rim_hard": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "trans_rim_inradius": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "transcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "transcolor_rimlight ": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "var_base_skincolor_ui": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "var_base_skinnormalintensity_ui": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "var_base_skinspecularintensity_ui": [
        0.6000000238418579,
        0.6000000238418579,
        0.6000000238418579,
        0.6000000238418579
      ],
      "var_base_skinspecularpower_ui": [
        0.5,
        0.5,
        0.5,
        0.5
      ]
    },
    "textures": [
      {
        "expressionIndex": 0,
        "assetId": "Character/SourceMaterials/pc_dk_av_basebody/pc_dk_av_base_upper_n_loc_int.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 1,
        "assetId": "Character/SourceMaterials/pc_dk_av_basebody/pc_dk_av_base_upper_cm.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 2,
        "assetId": "Character/SourceMaterials/pc_dk_av_basebody/pc_dk_av_base_upper_s.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 3,
        "assetId": "Character/SourceMaterials/pc_dk_av_basebody/pc_dk_av_base_upper_d.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 4,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/hdr07_1.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 5,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 6,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/brdf_beckmann_spec.dds",
        "colorSpace": "linear"
      }
    ]
  },
  {
    "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_02/lower.wmodel",
    "materialName": "pc_ddk_02_lower_mi",
    "sourceMaterial": "PC_DDK_02.mat.pc_ddk_02_lower_mi",
    "family": "source.character.realpbr-avatar-ddk.v1",
    "parameters": {
      "auto_pbr_oc_max": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "auto_pbr_oc_min": [
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896
      ],
      "basecolor_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "buffcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "diffusecolor_a": [
        0.32972902059555054,
        0.1701383739709854,
        0.14799802005290985,
        1.0
      ],
      "diffusecolor_b": [
        0.07592611759901047,
        0.0041161770932376385,
        0.0,
        1.0
      ],
      "diffusecolor_c": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "dye_roughness_power_tensition": [
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448
      ],
      "fx_color_desaturation_actiontool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_desaturation_buffsettool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_intensity_actiontool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "fx_color_intensity_buffsettool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "hit_color": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "mask_variation_visible": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "metallic_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "normaltex_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_brightness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "occlusion_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_power": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "occlusionbasecolor_blend_intensity": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "pbr_add_basecolor_to_emissive": [
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806
      ],
      "pbr_specular": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "roughness_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "roughness_power_a": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_b": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_c": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "selectioncolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "ssslocalthickness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "ssstintcolor": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "state": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "trans_rim_hard": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "trans_rim_inradius": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "transcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "transcolor_rimlight ": [
        1.0,
        1.0,
        1.0,
        1.0
      ]
    },
    "textures": [
      {
        "expressionIndex": 0,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_02/textures/pc_ddk_02_lower_n.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 1,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_02/textures/pc_ddk_02_lower_d.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 2,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_02/textures/pc_ddk_02_lower_orm.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 3,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_02/textures/pc_ddk_02_lower_cm.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 4,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 5,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/null.tga",
        "colorSpace": "srgb"
      }
    ]
  },
  {
    "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_02/lower.wmodel",
    "materialName": "pc_ddk_02_lower1_mi",
    "sourceMaterial": "PC_DDK_02.mat.pc_ddk_02_lower1_mi",
    "family": "source.character.realpbr-avatar-ddk.v1",
    "parameters": {
      "auto_pbr_oc_max": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "auto_pbr_oc_min": [
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896
      ],
      "basecolor_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "buffcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "diffusecolor_a": [
        0.1546904295682907,
        0.08489446341991425,
        0.0752115324139595,
        1.0
      ],
      "diffusecolor_b": [
        0.320430189371109,
        0.1851804107427597,
        0.1664169281721115,
        1.0
      ],
      "diffusecolor_c": [
        0.060771241784095764,
        0.030605049803853035,
        0.028875907883048058,
        1.0
      ],
      "dye_roughness_power_tensition": [
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448
      ],
      "fx_color_desaturation_actiontool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_desaturation_buffsettool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_intensity_actiontool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "fx_color_intensity_buffsettool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "hit_color": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "mask_variation_visible": [
        1.0,
        1.0,
        1.0,
        0.0
      ],
      "metallic_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "normaltex_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_brightness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "occlusion_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_power": [
        0.20000000298023224,
        0.20000000298023224,
        0.20000000298023224,
        0.20000000298023224
      ],
      "occlusionbasecolor_blend_intensity": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "pbr_add_basecolor_to_emissive": [
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806
      ],
      "pbr_specular": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "roughness_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "roughness_power_a": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_b": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_c": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "selectioncolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "ssslocalthickness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "ssstintcolor": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "state": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "trans_rim_hard": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "trans_rim_inradius": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "transcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "transcolor_rimlight ": [
        1.0,
        1.0,
        1.0,
        1.0
      ]
    },
    "textures": [
      {
        "expressionIndex": 0,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_02/textures/pc_ddk_02_lower1_n.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 1,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_02/textures/pc_ddk_02_lower1_d.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 2,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_02/textures/pc_ddk_02_lower1_orm.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 3,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_02/textures/pc_ddk_02_lower1_cm.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 4,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 5,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/null.tga",
        "colorSpace": "srgb"
      }
    ]
  },
  {
    "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_02/lower.wmodel",
    "materialName": "pc_dk_av_base_lower_mi",
    "sourceMaterial": "PC_DK_AV_BASEBODY.mat.pc_dk_av_base_lower_mi",
    "family": "source.character.classic-skin.v1",
    "parameters": {
      "1.use_dyeing_sp": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "beckmannspecular_constant_max": [
        3.3499999046325684,
        3.3499999046325684,
        3.3499999046325684,
        3.3499999046325684
      ],
      "buffcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "diffusecolor": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "fresnel_radius": [
        0.949999988079071,
        0.949999988079071,
        0.949999988079071,
        0.949999988079071
      ],
      "fresnel_rimlightintensity": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "fx_color_desaturation_actiontool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_desaturation_buffsettool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_intensity_actiontool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "fx_color_intensity_buffsettool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "half_lambert_skin": [
        1.0,
        0.1173890009522438,
        0.1080550029873848,
        1.0
      ],
      "hit_color": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "ibl_color_bottom": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "ibl_color_top": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "ibl_exposer": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "ibl_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "ibl_normal_smooth": [
        0.699999988079071,
        0.699999988079071,
        0.699999988079071,
        0.699999988079071
      ],
      "ibl_reflect_lodbias": [
        80.0,
        80.0,
        80.0,
        80.0
      ],
      "ibl_skinlodscale": [
        0.800000011920929,
        0.800000011920929,
        0.800000011920929,
        0.800000011920929
      ],
      "metalicness_power": [
        0.20000000298023224,
        0.20000000298023224,
        0.20000000298023224,
        0.20000000298023224
      ],
      "normaltex_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "orennayar": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "orennayar_brightness": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "pbr_specular_intensity": [
        10.0,
        10.0,
        10.0,
        10.0
      ],
      "pbr_specular_power": [
        6.0,
        6.0,
        6.0,
        6.0
      ],
      "roughness_power": [
        3.0,
        3.0,
        3.0,
        3.0
      ],
      "selectioncolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "shadowfactor": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "skin_metalicness_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "skin_normal_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "skin_roughness_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "skin_specular_intensity": [
        0.4000000059604645,
        0.4000000059604645,
        0.4000000059604645,
        0.4000000059604645
      ],
      "skin_specular_intensity_min": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "skin_specular_power": [
        20.0,
        20.0,
        20.0,
        20.0
      ],
      "skin_specular_power_min": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "specular_power_limit": [
        0.9200000166893005,
        0.9200000166893005,
        0.9200000166893005,
        0.9200000166893005
      ],
      "state": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "state_noise": [
        1.0,
        0.0,
        0.0,
        1.0
      ],
      "trans_rim_hard": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "trans_rim_inradius": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "transcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "transcolor_rimlight ": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "var_base_skincolor_ui": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "var_base_skinnormalintensity_ui": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "var_base_skinspecularintensity_ui": [
        0.6000000238418579,
        0.6000000238418579,
        0.6000000238418579,
        0.6000000238418579
      ],
      "var_base_skinspecularpower_ui": [
        0.5,
        0.5,
        0.5,
        0.5
      ]
    },
    "textures": [
      {
        "expressionIndex": 0,
        "assetId": "Character/GuardianKnight/Equipment/source_materials/pc_dk_av_basebody/pc_dk_av_base_lower_n.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 1,
        "assetId": "Character/GuardianKnight/Equipment/source_materials/pc_dk_av_basebody/pc_dk_av_base_lower_cm.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 2,
        "assetId": "Character/GuardianKnight/Equipment/source_materials/pc_dk_av_basebody/pc_dk_av_base_lower_s.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 3,
        "assetId": "Character/GuardianKnight/Equipment/source_materials/pc_dk_av_basebody/pc_dk_av_base_lower_d.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 4,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/hdr07_1.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 5,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 6,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/brdf_beckmann_spec.dds",
        "colorSpace": "linear"
      }
    ]
  },
  {
    "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_02/arm.wmodel",
    "materialName": "pc_ddk_02_arm_mi",
    "sourceMaterial": "PC_DDK_02.mat.pc_ddk_02_arm_mi",
    "family": "source.character.realpbr-avatar-ddk.v1",
    "parameters": {
      "auto_pbr_oc_max": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "auto_pbr_oc_min": [
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896
      ],
      "basecolor_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "buffcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "diffusecolor_a": [
        0.32972902059555054,
        0.1701383739709854,
        0.14799802005290985,
        1.0
      ],
      "diffusecolor_b": [
        0.07592611759901047,
        0.0041161770932376385,
        0.0,
        1.0
      ],
      "diffusecolor_c": [
        0.03867260739207268,
        0.0,
        0.0,
        1.0
      ],
      "dye_roughness_power_tensition": [
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448
      ],
      "fx_color_desaturation_actiontool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_desaturation_buffsettool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_intensity_actiontool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "fx_color_intensity_buffsettool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "hit_color": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "mask_variation_visible": [
        1.0,
        1.0,
        1.0,
        0.0
      ],
      "metallic_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "normaltex_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_brightness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "occlusion_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusionbasecolor_blend_intensity": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "pbr_add_basecolor_to_emissive": [
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806
      ],
      "pbr_specular": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "roughness_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "roughness_power_a": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_b": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_c": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "selectioncolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "ssslocalthickness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "ssstintcolor": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "state": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "trans_rim_hard": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "trans_rim_inradius": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "transcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "transcolor_rimlight ": [
        1.0,
        1.0,
        1.0,
        1.0
      ]
    },
    "textures": [
      {
        "expressionIndex": 0,
        "assetId": "Character/SourceMaterials/pc_ddk_02/pc_ddk_02_arm_n.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 1,
        "assetId": "Character/SourceMaterials/pc_ddk_02/pc_ddk_02_arm_d.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 2,
        "assetId": "Character/SourceMaterials/pc_ddk_02/pc_ddk_02_arm_orm.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 3,
        "assetId": "Character/SourceMaterials/pc_ddk_02/pc_ddk_02_arm_cm.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 4,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 5,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/null.tga",
        "colorSpace": "srgb"
      }
    ]
  },
  {
    "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_02/arm.wmodel",
    "materialName": "pc_dk_av_base_upper_mi",
    "sourceMaterial": "PC_DK_AV_BASEBODY.mat.pc_dk_av_base_upper_mi",
    "family": "source.character.classic-skin.v1",
    "parameters": {
      "1.use_dyeing_sp": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "beckmannspecular_constant_max": [
        3.3499999046325684,
        3.3499999046325684,
        3.3499999046325684,
        3.3499999046325684
      ],
      "buffcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "diffusecolor": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "fresnel_radius": [
        0.949999988079071,
        0.949999988079071,
        0.949999988079071,
        0.949999988079071
      ],
      "fresnel_rimlightintensity": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "fx_color_desaturation_actiontool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_desaturation_buffsettool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_intensity_actiontool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "fx_color_intensity_buffsettool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "half_lambert_skin": [
        1.0,
        0.1173890009522438,
        0.1080550029873848,
        1.0
      ],
      "hit_color": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "ibl_color_bottom": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "ibl_color_top": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "ibl_exposer": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "ibl_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "ibl_normal_smooth": [
        0.699999988079071,
        0.699999988079071,
        0.699999988079071,
        0.699999988079071
      ],
      "ibl_reflect_lodbias": [
        80.0,
        80.0,
        80.0,
        80.0
      ],
      "ibl_skinlodscale": [
        0.800000011920929,
        0.800000011920929,
        0.800000011920929,
        0.800000011920929
      ],
      "metalicness_power": [
        0.20000000298023224,
        0.20000000298023224,
        0.20000000298023224,
        0.20000000298023224
      ],
      "normaltex_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "orennayar": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "orennayar_brightness": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "pbr_specular_intensity": [
        10.0,
        10.0,
        10.0,
        10.0
      ],
      "pbr_specular_power": [
        6.0,
        6.0,
        6.0,
        6.0
      ],
      "roughness_power": [
        3.0,
        3.0,
        3.0,
        3.0
      ],
      "selectioncolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "shadowfactor": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "skin_metalicness_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "skin_normal_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "skin_roughness_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "skin_specular_intensity": [
        0.4000000059604645,
        0.4000000059604645,
        0.4000000059604645,
        0.4000000059604645
      ],
      "skin_specular_intensity_min": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "skin_specular_power": [
        20.0,
        20.0,
        20.0,
        20.0
      ],
      "skin_specular_power_min": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "specular_power_limit": [
        0.9200000166893005,
        0.9200000166893005,
        0.9200000166893005,
        0.9200000166893005
      ],
      "state": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "state_noise": [
        1.0,
        0.0,
        0.0,
        1.0
      ],
      "trans_rim_hard": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "trans_rim_inradius": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "transcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "transcolor_rimlight ": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "var_base_skincolor_ui": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "var_base_skinnormalintensity_ui": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "var_base_skinspecularintensity_ui": [
        0.6000000238418579,
        0.6000000238418579,
        0.6000000238418579,
        0.6000000238418579
      ],
      "var_base_skinspecularpower_ui": [
        0.5,
        0.5,
        0.5,
        0.5
      ]
    },
    "textures": [
      {
        "expressionIndex": 0,
        "assetId": "Character/SourceMaterials/pc_dk_av_basebody/pc_dk_av_base_upper_n_loc_int.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 1,
        "assetId": "Character/SourceMaterials/pc_dk_av_basebody/pc_dk_av_base_upper_cm.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 2,
        "assetId": "Character/SourceMaterials/pc_dk_av_basebody/pc_dk_av_base_upper_s.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 3,
        "assetId": "Character/SourceMaterials/pc_dk_av_basebody/pc_dk_av_base_upper_d.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 4,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/hdr07_1.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 5,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 6,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/brdf_beckmann_spec.dds",
        "colorSpace": "linear"
      }
    ]
  },
  {
    "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_02/shoulder.wmodel",
    "materialName": "pc_ddk_02_shoulder_mi",
    "sourceMaterial": "PC_DDK_02.mat.pc_ddk_02_shoulder_mi",
    "family": "source.character.equipment-native-198.v1",
    "parameters": {
      "1.use_emissive_flickerspeed_fixed": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "auto_pbr_oc_max": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "auto_pbr_oc_min": [
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896
      ],
      "basecolor_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "buffcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "diffusecolor_a": [
        0.32972902059555054,
        0.17935532331466675,
        0.12010139971971512,
        1.0
      ],
      "diffusecolor_b": [
        0.13609854876995087,
        0.012028954923152924,
        0.00752399442717433,
        1.0
      ],
      "diffusecolor_c": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "dye_roughness_power_tensition": [
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448
      ],
      "emissive_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "emissive_flicker_speed": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "emissive_intensity": [
        2.0,
        2.0,
        2.0,
        2.0
      ],
      "emissive_intensitymin": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "fx_color_desaturation_actiontool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_desaturation_buffsettool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_intensity_actiontool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "fx_color_intensity_buffsettool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "hit_color": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "mask_variation_visible": [
        0.0,
        1.0,
        0.0,
        0.0
      ],
      "metallic_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "normaltex_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_brightness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "occlusion_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_power": [
        0.20000000298023224,
        0.20000000298023224,
        0.20000000298023224,
        0.20000000298023224
      ],
      "occlusionbasecolor_blend_intensity": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "pbr_add_basecolor_to_emissive": [
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806
      ],
      "pbr_specular": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "roughness_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "roughness_power_a": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_b": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_c": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "selectioncolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "ssslocalthickness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "ssstintcolor": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "state": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "trans_rim_hard": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "trans_rim_inradius": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "transcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "transcolor_rimlight ": [
        1.0,
        1.0,
        1.0,
        1.0
      ]
    },
    "textures": [
      {
        "expressionIndex": 0,
        "assetId": "Character/SourceMaterials/pc_ddk_02/pc_ddk_02_shoulder_n.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 1,
        "assetId": "Character/SourceMaterials/pc_ddk_02/pc_ddk_02_shoulder_d.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 2,
        "assetId": "Character/SourceMaterials/pc_ddk_02/pc_ddk_02_shoulder_orm.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 3,
        "assetId": "Character/SourceMaterials/pc_ddk_02/pc_ddk_02_shoulder_cm.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 4,
        "assetId": "Character/SourceMaterials/pc_ddk_02/pc_ddk_02_shoulder_e.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 5,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 6,
        "assetId": "Character/SourceMaterials/pc_ddk_02/pc_ddk_02_shoulder_m.dds",
        "colorSpace": "srgb"
      }
    ]
  },
  {
    "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_02/helmet.wmodel",
    "materialName": "pc_ddk_02_helmet_mi",
    "sourceMaterial": "PC_DDK_02.mat.pc_ddk_02_helmet_mi",
    "family": "source.character.equipment-native-198.v1",
    "parameters": {
      "1.use_emissive_flickerspeed_fixed": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "auto_pbr_oc_max": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "auto_pbr_oc_min": [
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896
      ],
      "basecolor_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "buffcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "diffusecolor_a": [
        0.657434344291687,
        0.3976881504058838,
        0.31601694226264954,
        1.0
      ],
      "diffusecolor_b": [
        0.08286987245082855,
        0.011270656250417233,
        0.008701778016984463,
        1.0
      ],
      "diffusecolor_c": [
        0.6523700952529907,
        0.3940831124782562,
        0.3157627582550049,
        1.0
      ],
      "dye_roughness_power_tensition": [
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448
      ],
      "emissive_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "emissive_flicker_speed": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "emissive_intensity": [
        2.0,
        2.0,
        2.0,
        2.0
      ],
      "emissive_intensitymin": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "fx_color_desaturation_actiontool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_desaturation_buffsettool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_intensity_actiontool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "fx_color_intensity_buffsettool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "hit_color": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "mask_variation_visible": [
        1.0,
        1.0,
        1.0,
        0.0
      ],
      "metallic_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "normaltex_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_brightness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "occlusion_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_power": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "occlusionbasecolor_blend_intensity": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "pbr_add_basecolor_to_emissive": [
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806
      ],
      "pbr_specular": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "roughness_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "roughness_power_a": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_b": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_c": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "selectioncolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "ssslocalthickness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "ssstintcolor": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "state": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "trans_rim_hard": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "trans_rim_inradius": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "transcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "transcolor_rimlight ": [
        1.0,
        1.0,
        1.0,
        1.0
      ]
    },
    "textures": [
      {
        "expressionIndex": 0,
        "assetId": "Character/SourceMaterials/pc_ddk_02/pc_ddk_02_helmet_n.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 1,
        "assetId": "Character/SourceMaterials/pc_ddk_02/pc_ddk_02_helmet_d.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 2,
        "assetId": "Character/SourceMaterials/pc_ddk_02/pc_ddk_02_helmet_orm.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 3,
        "assetId": "Character/SourceMaterials/pc_ddk_02/pc_ddk_02_helmet_cm.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 4,
        "assetId": "Character/SourceMaterials/pc_ddk_02/pc_ddk_02_helmet_e.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 5,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 6,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/null.tga",
        "colorSpace": "srgb"
      }
    ]
  },
  {
    "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_03/upper.wmodel",
    "materialName": "pc_ddk_03_upper_mi",
    "sourceMaterial": "PC_DDK_03.mat.pc_ddk_03_upper_mi",
    "family": "source.character.equipment-native-183.v1",
    "parameters": {
      "auto_pbr_oc_max": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "auto_pbr_oc_min": [
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896
      ],
      "basecolor_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "buffcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "diffusecolor_a": [
        0.3462119400501251,
        0.3462119400501251,
        0.3462119400501251,
        1.0
      ],
      "diffusecolor_b": [
        0.9723398685455322,
        0.9256473183631897,
        0.9186214208602905,
        1.0
      ],
      "diffusecolor_c": [
        0.5635151267051697,
        0.5635151267051697,
        0.5635151267051697,
        1.0
      ],
      "dye_roughness_power_tensition": [
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448
      ],
      "fx_color_desaturation_actiontool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_desaturation_buffsettool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_intensity_actiontool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "fx_color_intensity_buffsettool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "hit_color": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "metallic_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "normaltex_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_brightness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "occlusion_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusionbasecolor_blend_intensity": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "pbr_add_basecolor_to_emissive": [
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806
      ],
      "pbr_specular": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "roughness_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "roughness_power_a": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_b": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_c": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "selectioncolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "ssslocalthickness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "ssstintcolor": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "state": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "trans_rim_hard": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "trans_rim_inradius": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "transcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "transcolor_rimlight ": [
        1.0,
        1.0,
        1.0,
        1.0
      ]
    },
    "textures": [
      {
        "expressionIndex": 0,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_03/textures/pc_ddk_03_upper_n.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 1,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_03/textures/pc_ddk_03_upper_d.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 2,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_03/textures/pc_ddk_03_upper_orm.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 3,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_03/textures/pc_ddk_03_upper_cm.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 4,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.dds",
        "colorSpace": "srgb"
      }
    ]
  },
  {
    "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_03/upper.wmodel",
    "materialName": "pc_ddk_03_upper1_mi",
    "sourceMaterial": "PC_DDK_03.mat.pc_ddk_03_upper1_mi",
    "family": "source.character.realpbr-avatar-ddk.v1",
    "parameters": {
      "auto_pbr_oc_max": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "auto_pbr_oc_min": [
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896
      ],
      "basecolor_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "buffcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "diffusecolor_a": [
        0.34989503026008606,
        0.34989503026008606,
        0.34989503026008606,
        1.0
      ],
      "diffusecolor_b": [
        0.9658146500587463,
        0.9239933490753174,
        0.9157501459121704,
        1.0
      ],
      "diffusecolor_c": [
        0.5604991316795349,
        0.5604991316795349,
        0.5604991316795349,
        1.0
      ],
      "dye_roughness_power_tensition": [
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448
      ],
      "fx_color_desaturation_actiontool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_desaturation_buffsettool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_intensity_actiontool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "fx_color_intensity_buffsettool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "hit_color": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "mask_variation_visible": [
        1.0,
        0.0,
        0.0,
        0.0
      ],
      "metallic_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "normaltex_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_brightness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "occlusion_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusionbasecolor_blend_intensity": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "pbr_add_basecolor_to_emissive": [
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806
      ],
      "pbr_specular": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "roughness_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "roughness_power_a": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_b": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_c": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "selectioncolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "ssslocalthickness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "ssstintcolor": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "state": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "trans_rim_hard": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "trans_rim_inradius": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "transcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "transcolor_rimlight ": [
        1.0,
        1.0,
        1.0,
        1.0
      ]
    },
    "textures": [
      {
        "expressionIndex": 0,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_03/textures/pc_ddk_03_upper_1_n.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 1,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_03/textures/pc_ddk_03_upper_1_d.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 2,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_03/textures/pc_ddk_03_upper_1_orm.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 3,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_03/textures/pc_ddk_03_upper_1_cm.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 4,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 5,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_03/textures/pc_ddk_03_upper_1_m.dds",
        "colorSpace": "srgb"
      }
    ]
  },
  {
    "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_03/upper.wmodel",
    "materialName": "pc_ddk_03_upper2_mi",
    "sourceMaterial": "PC_DDK_03.mat.pc_ddk_03_upper2_mi",
    "family": "source.character.realpbr-avatar-ddk.v1",
    "parameters": {
      "auto_pbr_oc_max": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "auto_pbr_oc_min": [
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896
      ],
      "basecolor_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "buffcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "diffusecolor_a": [
        0.11926401406526566,
        0.11926401406526566,
        0.11926401406526566,
        1.0
      ],
      "diffusecolor_b": [
        0.835527777671814,
        0.8122414946556091,
        0.7817506790161133,
        1.0
      ],
      "diffusecolor_c": [
        0.5604991316795349,
        0.5604991316795349,
        0.5604991316795349,
        1.0
      ],
      "dye_roughness_power_tensition": [
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448
      ],
      "fx_color_desaturation_actiontool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_desaturation_buffsettool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_intensity_actiontool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "fx_color_intensity_buffsettool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "hit_color": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "mask_variation_visible": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "metallic_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "normaltex_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_brightness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "occlusion_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusionbasecolor_blend_intensity": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "pbr_add_basecolor_to_emissive": [
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806
      ],
      "pbr_specular": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "roughness_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "roughness_power_a": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_b": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_c": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "selectioncolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "ssslocalthickness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "ssstintcolor": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "state": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "trans_rim_hard": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "trans_rim_inradius": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "transcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "transcolor_rimlight ": [
        1.0,
        1.0,
        1.0,
        1.0
      ]
    },
    "textures": [
      {
        "expressionIndex": 0,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_03/textures/pc_ddk_03_upper_2_n.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 1,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_03/textures/pc_ddk_03_upper_2_d.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 2,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_03/textures/pc_ddk_03_upper_2_orm.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 3,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_03/textures/pc_ddk_03_upper_2_cm.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 4,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 5,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_03/textures/pc_ddk_03_upper_2_m.dds",
        "colorSpace": "srgb"
      }
    ]
  },
  {
    "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_03/upper.wmodel",
    "materialName": "pc_dk_av_base_upper_mi",
    "sourceMaterial": "PC_DK_AV_BASEBODY.mat.pc_dk_av_base_upper_mi",
    "family": "source.character.classic-skin.v1",
    "parameters": {
      "1.use_dyeing_sp": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "beckmannspecular_constant_max": [
        3.3499999046325684,
        3.3499999046325684,
        3.3499999046325684,
        3.3499999046325684
      ],
      "buffcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "diffusecolor": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "fresnel_radius": [
        0.949999988079071,
        0.949999988079071,
        0.949999988079071,
        0.949999988079071
      ],
      "fresnel_rimlightintensity": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "fx_color_desaturation_actiontool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_desaturation_buffsettool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_intensity_actiontool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "fx_color_intensity_buffsettool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "half_lambert_skin": [
        1.0,
        0.1173890009522438,
        0.1080550029873848,
        1.0
      ],
      "hit_color": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "ibl_color_bottom": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "ibl_color_top": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "ibl_exposer": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "ibl_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "ibl_normal_smooth": [
        0.699999988079071,
        0.699999988079071,
        0.699999988079071,
        0.699999988079071
      ],
      "ibl_reflect_lodbias": [
        80.0,
        80.0,
        80.0,
        80.0
      ],
      "ibl_skinlodscale": [
        0.800000011920929,
        0.800000011920929,
        0.800000011920929,
        0.800000011920929
      ],
      "metalicness_power": [
        0.20000000298023224,
        0.20000000298023224,
        0.20000000298023224,
        0.20000000298023224
      ],
      "normaltex_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "orennayar": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "orennayar_brightness": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "pbr_specular_intensity": [
        10.0,
        10.0,
        10.0,
        10.0
      ],
      "pbr_specular_power": [
        6.0,
        6.0,
        6.0,
        6.0
      ],
      "roughness_power": [
        3.0,
        3.0,
        3.0,
        3.0
      ],
      "selectioncolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "shadowfactor": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "skin_metalicness_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "skin_normal_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "skin_roughness_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "skin_specular_intensity": [
        0.4000000059604645,
        0.4000000059604645,
        0.4000000059604645,
        0.4000000059604645
      ],
      "skin_specular_intensity_min": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "skin_specular_power": [
        20.0,
        20.0,
        20.0,
        20.0
      ],
      "skin_specular_power_min": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "specular_power_limit": [
        0.9200000166893005,
        0.9200000166893005,
        0.9200000166893005,
        0.9200000166893005
      ],
      "state": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "state_noise": [
        1.0,
        0.0,
        0.0,
        1.0
      ],
      "trans_rim_hard": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "trans_rim_inradius": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "transcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "transcolor_rimlight ": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "var_base_skincolor_ui": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "var_base_skinnormalintensity_ui": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "var_base_skinspecularintensity_ui": [
        0.6000000238418579,
        0.6000000238418579,
        0.6000000238418579,
        0.6000000238418579
      ],
      "var_base_skinspecularpower_ui": [
        0.5,
        0.5,
        0.5,
        0.5
      ]
    },
    "textures": [
      {
        "expressionIndex": 0,
        "assetId": "Character/SourceMaterials/pc_dk_av_basebody/pc_dk_av_base_upper_n_loc_int.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 1,
        "assetId": "Character/SourceMaterials/pc_dk_av_basebody/pc_dk_av_base_upper_cm.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 2,
        "assetId": "Character/SourceMaterials/pc_dk_av_basebody/pc_dk_av_base_upper_s.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 3,
        "assetId": "Character/SourceMaterials/pc_dk_av_basebody/pc_dk_av_base_upper_d.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 4,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/hdr07_1.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 5,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 6,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/brdf_beckmann_spec.dds",
        "colorSpace": "linear"
      }
    ]
  },
  {
    "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_03/lower.wmodel",
    "materialName": "pc_ddk_03-3_lower_mi",
    "sourceMaterial": "PC_DDK_03.mat.pc_ddk_03-3_lower_mi",
    "family": "source.character.realpbr-avatar-ddk.v1",
    "parameters": {
      "auto_pbr_oc_max": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "auto_pbr_oc_min": [
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896
      ],
      "basecolor_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "buffcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "diffusecolor_a": [
        0.3344578146934509,
        0.3488648235797882,
        0.3066347539424896,
        1.0
      ],
      "diffusecolor_b": [
        5.077051810076227e-06,
        0.12474094331264496,
        0.041451890021562576,
        1.0
      ],
      "diffusecolor_c": [
        0.016988052055239677,
        0.017936432734131813,
        0.01606770046055317,
        1.0
      ],
      "dye_roughness_power_tensition": [
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448
      ],
      "fx_color_desaturation_actiontool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_desaturation_buffsettool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_intensity_actiontool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "fx_color_intensity_buffsettool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "hit_color": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "mask_variation_visible": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "metallic_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "normaltex_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_brightness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "occlusion_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusionbasecolor_blend_intensity": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "pbr_add_basecolor_to_emissive": [
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806
      ],
      "pbr_specular": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "roughness_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "roughness_power_a": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_b": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_c": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "selectioncolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "ssslocalthickness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "ssstintcolor": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "state": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "trans_rim_hard": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "trans_rim_inradius": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "transcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "transcolor_rimlight ": [
        1.0,
        1.0,
        1.0,
        1.0
      ]
    },
    "textures": [
      {
        "expressionIndex": 0,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_03/textures/pc_ddk_03_lower_n.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 1,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_03/textures/pc_ddk_03_lower_d.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 2,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_03/textures/pc_ddk_03_lower_orm.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 3,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_03/textures/pc_ddk_03_lower_cm.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 4,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 5,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/null.tga",
        "colorSpace": "srgb"
      }
    ]
  },
  {
    "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_03/lower.wmodel",
    "materialName": "pc_ddk_03-3_lower1_mi",
    "sourceMaterial": "PC_DDK_03.mat.pc_ddk_03-3_lower1_mi",
    "family": "source.character.realpbr-avatar-ddk.v1",
    "parameters": {
      "auto_pbr_oc_max": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "auto_pbr_oc_min": [
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896
      ],
      "basecolor_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "buffcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "diffusecolor_a": [
        0.3344578146934509,
        0.3488648235797882,
        0.3066347539424896,
        1.0
      ],
      "diffusecolor_b": [
        5.077051810076227e-06,
        0.12474094331264496,
        0.041451890021562576,
        1.0
      ],
      "diffusecolor_c": [
        0.016988052055239677,
        0.017936432734131813,
        0.01606770046055317,
        1.0
      ],
      "dye_roughness_power_tensition": [
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448
      ],
      "fx_color_desaturation_actiontool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_desaturation_buffsettool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_intensity_actiontool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "fx_color_intensity_buffsettool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "hit_color": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "mask_variation_visible": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "metallic_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "normaltex_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_brightness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "occlusion_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusionbasecolor_blend_intensity": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "pbr_add_basecolor_to_emissive": [
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806
      ],
      "pbr_specular": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "roughness_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "roughness_power_a": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_b": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_c": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "selectioncolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "ssslocalthickness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "ssstintcolor": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "state": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "trans_rim_hard": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "trans_rim_inradius": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "transcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "transcolor_rimlight ": [
        1.0,
        1.0,
        1.0,
        1.0
      ]
    },
    "textures": [
      {
        "expressionIndex": 0,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_03/textures/pc_ddk_03_lower_1_n.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 1,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_03/textures/pc_ddk_03_lower_1_d.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 2,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_03/textures/pc_ddk_03_lower_1_orm.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 3,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_03/textures/pc_ddk_03_lower_1_cm.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 4,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 5,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/null.tga",
        "colorSpace": "srgb"
      }
    ]
  },
  {
    "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_03/lower.wmodel",
    "materialName": "pc_dk_av_base_lower_mi",
    "sourceMaterial": "PC_DK_AV_BASEBODY.mat.pc_dk_av_base_lower_mi",
    "family": "source.character.classic-skin.v1",
    "parameters": {
      "1.use_dyeing_sp": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "beckmannspecular_constant_max": [
        3.3499999046325684,
        3.3499999046325684,
        3.3499999046325684,
        3.3499999046325684
      ],
      "buffcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "diffusecolor": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "fresnel_radius": [
        0.949999988079071,
        0.949999988079071,
        0.949999988079071,
        0.949999988079071
      ],
      "fresnel_rimlightintensity": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "fx_color_desaturation_actiontool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_desaturation_buffsettool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_intensity_actiontool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "fx_color_intensity_buffsettool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "half_lambert_skin": [
        1.0,
        0.1173890009522438,
        0.1080550029873848,
        1.0
      ],
      "hit_color": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "ibl_color_bottom": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "ibl_color_top": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "ibl_exposer": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "ibl_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "ibl_normal_smooth": [
        0.699999988079071,
        0.699999988079071,
        0.699999988079071,
        0.699999988079071
      ],
      "ibl_reflect_lodbias": [
        80.0,
        80.0,
        80.0,
        80.0
      ],
      "ibl_skinlodscale": [
        0.800000011920929,
        0.800000011920929,
        0.800000011920929,
        0.800000011920929
      ],
      "metalicness_power": [
        0.20000000298023224,
        0.20000000298023224,
        0.20000000298023224,
        0.20000000298023224
      ],
      "normaltex_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "orennayar": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "orennayar_brightness": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "pbr_specular_intensity": [
        10.0,
        10.0,
        10.0,
        10.0
      ],
      "pbr_specular_power": [
        6.0,
        6.0,
        6.0,
        6.0
      ],
      "roughness_power": [
        3.0,
        3.0,
        3.0,
        3.0
      ],
      "selectioncolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "shadowfactor": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "skin_metalicness_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "skin_normal_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "skin_roughness_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "skin_specular_intensity": [
        0.4000000059604645,
        0.4000000059604645,
        0.4000000059604645,
        0.4000000059604645
      ],
      "skin_specular_intensity_min": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "skin_specular_power": [
        20.0,
        20.0,
        20.0,
        20.0
      ],
      "skin_specular_power_min": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "specular_power_limit": [
        0.9200000166893005,
        0.9200000166893005,
        0.9200000166893005,
        0.9200000166893005
      ],
      "state": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "state_noise": [
        1.0,
        0.0,
        0.0,
        1.0
      ],
      "trans_rim_hard": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "trans_rim_inradius": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "transcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "transcolor_rimlight ": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "var_base_skincolor_ui": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "var_base_skinnormalintensity_ui": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "var_base_skinspecularintensity_ui": [
        0.6000000238418579,
        0.6000000238418579,
        0.6000000238418579,
        0.6000000238418579
      ],
      "var_base_skinspecularpower_ui": [
        0.5,
        0.5,
        0.5,
        0.5
      ]
    },
    "textures": [
      {
        "expressionIndex": 0,
        "assetId": "Character/GuardianKnight/Equipment/source_materials/pc_dk_av_basebody/pc_dk_av_base_lower_n.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 1,
        "assetId": "Character/GuardianKnight/Equipment/source_materials/pc_dk_av_basebody/pc_dk_av_base_lower_cm.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 2,
        "assetId": "Character/GuardianKnight/Equipment/source_materials/pc_dk_av_basebody/pc_dk_av_base_lower_s.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 3,
        "assetId": "Character/GuardianKnight/Equipment/source_materials/pc_dk_av_basebody/pc_dk_av_base_lower_d.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 4,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/hdr07_1.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 5,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 6,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/brdf_beckmann_spec.dds",
        "colorSpace": "linear"
      }
    ]
  },
  {
    "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_03/arm.wmodel",
    "materialName": "pc_ddk_03_arm_mi",
    "sourceMaterial": "PC_DDK_03.mat.pc_ddk_03_arm_mi",
    "family": "source.character.realpbr-avatar-ddk.v1",
    "parameters": {
      "auto_pbr_oc_max": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "auto_pbr_oc_min": [
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896
      ],
      "basecolor_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "buffcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "diffusecolor_a": [
        0.3440256714820862,
        0.3440256714820862,
        0.3440256714820862,
        1.0
      ],
      "diffusecolor_b": [
        0.9658146500587463,
        0.9239933490753174,
        0.9157501459121704,
        1.0
      ],
      "diffusecolor_c": [
        0.5604991316795349,
        0.5604991316795349,
        0.5604991316795349,
        1.0
      ],
      "dye_roughness_power_tensition": [
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448
      ],
      "fx_color_desaturation_actiontool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_desaturation_buffsettool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_intensity_actiontool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "fx_color_intensity_buffsettool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "hit_color": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "mask_variation_visible": [
        0.0,
        0.0,
        1.0,
        0.0
      ],
      "metallic_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "normaltex_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_brightness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "occlusion_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusionbasecolor_blend_intensity": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "pbr_add_basecolor_to_emissive": [
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806
      ],
      "pbr_specular": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "roughness_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "roughness_power_a": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_b": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_c": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "selectioncolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "ssslocalthickness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "ssstintcolor": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "state": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "trans_rim_hard": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "trans_rim_inradius": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "transcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "transcolor_rimlight ": [
        1.0,
        1.0,
        1.0,
        1.0
      ]
    },
    "textures": [
      {
        "expressionIndex": 0,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_03/textures/pc_ddk_03_arm_n.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 1,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_03/textures/pc_ddk_03_arm_d.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 2,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_03/textures/pc_ddk_03_arm_orm.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 3,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_03/textures/pc_ddk_03_arm_cm.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 4,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 5,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_03/textures/pc_ddk_03_arm_m.dds",
        "colorSpace": "srgb"
      }
    ]
  },
  {
    "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_03/arm.wmodel",
    "materialName": "pc_dk_av_base_upper_mi",
    "sourceMaterial": "PC_DK_AV_BASEBODY.mat.pc_dk_av_base_upper_mi",
    "family": "source.character.classic-skin.v1",
    "parameters": {
      "1.use_dyeing_sp": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "beckmannspecular_constant_max": [
        3.3499999046325684,
        3.3499999046325684,
        3.3499999046325684,
        3.3499999046325684
      ],
      "buffcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "diffusecolor": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "fresnel_radius": [
        0.949999988079071,
        0.949999988079071,
        0.949999988079071,
        0.949999988079071
      ],
      "fresnel_rimlightintensity": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "fx_color_desaturation_actiontool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_desaturation_buffsettool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_intensity_actiontool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "fx_color_intensity_buffsettool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "half_lambert_skin": [
        1.0,
        0.1173890009522438,
        0.1080550029873848,
        1.0
      ],
      "hit_color": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "ibl_color_bottom": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "ibl_color_top": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "ibl_exposer": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "ibl_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "ibl_normal_smooth": [
        0.699999988079071,
        0.699999988079071,
        0.699999988079071,
        0.699999988079071
      ],
      "ibl_reflect_lodbias": [
        80.0,
        80.0,
        80.0,
        80.0
      ],
      "ibl_skinlodscale": [
        0.800000011920929,
        0.800000011920929,
        0.800000011920929,
        0.800000011920929
      ],
      "metalicness_power": [
        0.20000000298023224,
        0.20000000298023224,
        0.20000000298023224,
        0.20000000298023224
      ],
      "normaltex_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "orennayar": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "orennayar_brightness": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "pbr_specular_intensity": [
        10.0,
        10.0,
        10.0,
        10.0
      ],
      "pbr_specular_power": [
        6.0,
        6.0,
        6.0,
        6.0
      ],
      "roughness_power": [
        3.0,
        3.0,
        3.0,
        3.0
      ],
      "selectioncolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "shadowfactor": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "skin_metalicness_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "skin_normal_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "skin_roughness_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "skin_specular_intensity": [
        0.4000000059604645,
        0.4000000059604645,
        0.4000000059604645,
        0.4000000059604645
      ],
      "skin_specular_intensity_min": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "skin_specular_power": [
        20.0,
        20.0,
        20.0,
        20.0
      ],
      "skin_specular_power_min": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "specular_power_limit": [
        0.9200000166893005,
        0.9200000166893005,
        0.9200000166893005,
        0.9200000166893005
      ],
      "state": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "state_noise": [
        1.0,
        0.0,
        0.0,
        1.0
      ],
      "trans_rim_hard": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "trans_rim_inradius": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "transcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "transcolor_rimlight ": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "var_base_skincolor_ui": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "var_base_skinnormalintensity_ui": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "var_base_skinspecularintensity_ui": [
        0.6000000238418579,
        0.6000000238418579,
        0.6000000238418579,
        0.6000000238418579
      ],
      "var_base_skinspecularpower_ui": [
        0.5,
        0.5,
        0.5,
        0.5
      ]
    },
    "textures": [
      {
        "expressionIndex": 0,
        "assetId": "Character/SourceMaterials/pc_dk_av_basebody/pc_dk_av_base_upper_n_loc_int.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 1,
        "assetId": "Character/SourceMaterials/pc_dk_av_basebody/pc_dk_av_base_upper_cm.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 2,
        "assetId": "Character/SourceMaterials/pc_dk_av_basebody/pc_dk_av_base_upper_s.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 3,
        "assetId": "Character/SourceMaterials/pc_dk_av_basebody/pc_dk_av_base_upper_d.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 4,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/hdr07_1.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 5,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 6,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/brdf_beckmann_spec.dds",
        "colorSpace": "linear"
      }
    ]
  },
  {
    "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_03/shoulder.wmodel",
    "materialName": "pc_ddk_03_shoulder_mi",
    "sourceMaterial": "PC_DDK_03.mat.pc_ddk_03_shoulder_mi",
    "family": "source.character.realpbr-avatar-ddk.v1",
    "parameters": {
      "auto_pbr_oc_max": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "auto_pbr_oc_min": [
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896
      ],
      "basecolor_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "buffcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "diffusecolor_a": [
        0.3440256714820862,
        0.3440256714820862,
        0.3440256714820862,
        1.0
      ],
      "diffusecolor_b": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "diffusecolor_c": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "dye_roughness_power_tensition": [
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448
      ],
      "fx_color_desaturation_actiontool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_desaturation_buffsettool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_intensity_actiontool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "fx_color_intensity_buffsettool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "hit_color": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "mask_variation_visible": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "metallic_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "normaltex_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_brightness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "occlusion_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusionbasecolor_blend_intensity": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "pbr_add_basecolor_to_emissive": [
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806
      ],
      "pbr_specular": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "roughness_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "roughness_power_a": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_b": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_c": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "selectioncolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "ssslocalthickness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "ssstintcolor": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "state": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "trans_rim_hard": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "trans_rim_inradius": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "transcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "transcolor_rimlight ": [
        1.0,
        1.0,
        1.0,
        1.0
      ]
    },
    "textures": [
      {
        "expressionIndex": 0,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_03/textures/pc_ddk_03_shoulder_n.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 1,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_03/textures/pc_ddk_03_shoulder_d.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 2,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_03/textures/pc_ddk_03_shoulder_orm.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 3,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_03/textures/pc_ddk_03_shoulder_cm.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 4,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 5,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_03/textures/pc_ddk_03_shoulder_m.dds",
        "colorSpace": "srgb"
      }
    ]
  },
  {
    "modelAssetId": "Character/GuardianKnight/Equipment/source_ddk_03/helmet.wmodel",
    "materialName": "pc_ddk_03_helmet_mi",
    "sourceMaterial": "PC_DDK_03.mat.pc_ddk_03_helmet_mi",
    "family": "source.character.realpbr-avatar-ddk.v1",
    "parameters": {
      "auto_pbr_oc_max": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "auto_pbr_oc_min": [
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896,
        0.30000001192092896
      ],
      "basecolor_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "buffcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "diffusecolor_a": [
        0.3440256714820862,
        0.3440256714820862,
        0.3440256714820862,
        1.0
      ],
      "diffusecolor_b": [
        0.9658146500587463,
        0.9239933490753174,
        0.9157501459121704,
        1.0
      ],
      "diffusecolor_c": [
        0.5604991316795349,
        0.5604991316795349,
        0.5604991316795349,
        1.0
      ],
      "dye_roughness_power_tensition": [
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448,
        0.15000000596046448
      ],
      "fx_color_desaturation_actiontool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_desaturation_buffsettool": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "fx_color_intensity_actiontool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "fx_color_intensity_buffsettool": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "hit_color": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "mask_variation_visible": [
        1.0,
        0.0,
        0.0,
        0.0
      ],
      "metallic_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "normaltex_intensity": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_brightness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "occlusion_color": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusion_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "occlusionbasecolor_blend_intensity": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "pbr_add_basecolor_to_emissive": [
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806,
        0.05000000074505806
      ],
      "pbr_specular": [
        0.5,
        0.5,
        0.5,
        0.5
      ],
      "roughness_power": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "roughness_power_a": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_b": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "roughness_power_c": [
        5.0,
        5.0,
        5.0,
        5.0
      ],
      "selectioncolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "ssslocalthickness": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "ssstintcolor": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "state": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "trans_rim_hard": [
        1.0,
        1.0,
        1.0,
        1.0
      ],
      "trans_rim_inradius": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "transcolor": [
        0.0,
        0.0,
        0.0,
        1.0
      ],
      "transcolor_rimlight ": [
        1.0,
        1.0,
        1.0,
        1.0
      ]
    },
    "textures": [
      {
        "expressionIndex": 0,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_03/textures/pc_ddk_03_helmet_n.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 1,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_03/textures/pc_ddk_03_helmet_d.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 2,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_03/textures/pc_ddk_03_helmet_orm.dds",
        "colorSpace": "linear"
      },
      {
        "expressionIndex": 3,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_03/textures/pc_ddk_03_helmet_cm.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 4,
        "assetId": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.dds",
        "colorSpace": "srgb"
      },
      {
        "expressionIndex": 5,
        "assetId": "Character/GuardianKnight/Equipment/source_ddk_03/textures/pc_ddk_03_helmet_m.dds",
        "colorSpace": "srgb"
      }
    ]
  }
]
```
