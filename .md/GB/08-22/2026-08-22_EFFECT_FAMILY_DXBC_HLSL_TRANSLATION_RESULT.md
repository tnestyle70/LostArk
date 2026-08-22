# 2026-08-22 Effect family 전수 조사와 DXBC → HLSL 번역 결과

## 0. 이 문서의 범위

요청은 두 가지였다. 모든 Effect의 missing family를 전수 조사하고, 그 DXBC를 바탕으로
family별 HLSL을 복원하는 것이다. 이 문서는 그 두 작업의 실제 실행 결과만 기록한다.

이 세션은 **수평 자동화**를 소유한다. 같은 날 다른 세션이 진행 중인
`Glasshole02` 단일 family의 renderer 수직 슬라이스(`Shader_Ue3Glasshole02*`,
`materialize_ue3_glasshole02_runtime_canary_contract.py`, Tool-only canary)는 이 세션에서
건드리지 않았고 복제하지도 않았다. 두 작업은 같은 결론에 다른 축으로 도달한다.

- 다른 세션: 한 family를 `DXBC → 전용 HLSL → renderer canary → 빌드 → 육안 검증`까지 닫는다.
- 이 세션: 그 첫 단계인 `DXBC → 검증된 HLSL`을 현재 169개 고유 pixel program에 대해
  자동화한다. family 분모는 205, shader-map 보유 분모는 193, 추출 family는 180이다.

renderer 배선, sampler state, render state, Product admission은 이 세션이 열지 않았다.

## 1. 전수 조사 실측

### 1.1 authored corpus의 두 모집단

PR #156에서 형성되어 PR #162 기준 그대로인 `Data/Effects/Authored`는 420개 문서,
7,572개 element다.

| 모집단 | element 수 | 의미 |
|---|---:|---|
| source parent 보존 | 2,746 | `sourceProfile.parentMaterialPath`를 가진 현재 shader-map 분모다. |
| authored detail / parent 유실 | 4,826 | parent가 없지만 4,793행은 child MIC 경로를 보존한다. |

이 문서의 최초 결론인 "parent가 없으면 원본 근사"는 틀렸다. child-parent resolution은
809개 orphan child 중 744개를 해석해 4,391 element의 parent를 복구했다. 따라서 두 번째
모집단은 근사 판정이 아니라 **join key 복구 대기**다. Valtan도 family 경로 밖이라고
단정하지 않는다.

### 1.2 family와 실행 중인 executor

source parent 보존 2,746 occurrence는 **205개 parent material family**로 모인다.

| 상태 | family | occurrence |
|---|---:|---:|
| typed executor 있음 | 40 | 1,518 |
| runtime executor pending | 165 | 1,228 |

typed executor가 있는 40개도 세 증거 상태로 갈린다. 이 구분이 요청의 핵심이었다.

| formulaProvenance | family | occurrence |
|---|---:|---:|
| `BOUNDED_PARENT_PROPS_RECONSTRUCTION` | 28 | 1,274 |
| `DXBC_REPLAYED_TRANSLATION` | 1 | 86 |
| `EXECUTOR_OPCODE_NOT_MAPPED` | 11 | 158 |

typed executor가 있다는 사실은 Product 복원을 뜻하지 않는다. `formulaProvenance`가
equation의 근거만 분리하며, runtime executor pending 1,228 occurrence는 여전히 공용
Program/Layout/Descriptor/Adapter 경로가 필요하다.

도구와 정본은 다음과 같다.

```text
Tools/EffectPipeline/build_effect_missing_family_inventory.py
Tools/EffectPipeline/test_build_effect_missing_family_inventory.py
Data/Effects/Contracts/effect-missing-family-inventory.v1.json
```

executor 목록은 문서에 다시 적지 않고 `Client/Public/Effect_MaterialTemplate.h`의
`Resolve_EffectStrictTypedSourceProfile`, opcode는 `Effect_DocumentRenderer.cpp`의 switch,
bounded 여부는 `Shader_EffectCommon.hlsli`의 profile 분기에서 파싱한다. 인벤토리가 코드와
어긋날 수 없다.

## 2. DXBC 회수는 미착수였지 불가능이 아니었다

조사 시작 시점의 상태는 "173 missing family 전부 DXBC join 0건"이었다. 이것을 증거 부재로
읽으면 안 된다는 것이 실측 결과다.

`fx_mastermaterial`, `fx_m_mi_00~05`, `fx_m_mi_d/g/h/j/k/l/m/n/o/p/q/r/s/t/u/w/x/y/z_00`,
`bfx_m_mi_00`을 포함한 공용 material package가 DimensionMaster v3 source pack에 이미
staging되어 있다. parent Material export의 tagged property stream 뒤 `tail[16:32]`에서 base
Material GUID를 읽고, pinned RefShaderCache v974를 한 번만 순회하면 된다.

```text
Tools/EffectPipeline/build_effect_family_shader_map_index.py
Data/Effects/Contracts/effect-family-shader-map-index.v1.json
```

| 항목 | 결과 |
|---|---|
| parent material | 205 |
| cooked material map 보유 | **193 / 205** |
| base Material GUID 미해석 | 12 |
| material-map context | 4,057 |
| 미해석 12건 | `defaultmaterial`, `defaultparticle`, `ch.realpbr.*` 등 fx 외 material |

현재 shader-map index의 193개 map 보유 family와 12개 미해석 family가 정확한 분모다.
이 수치는 child-parent resolution으로 찾은 신규 parent 78개를 아직 포함하지 않는다. 그
확장은 별도 denominator 변경이며 이 RESULT가 자동으로 admission하지 않는다.

## 3. family별 cooked pixel shader 추출

permutation 선택을 사람이 pin하지 않고 자동화했다. authored corpus가 가장 많이 쓰는 child
MaterialInstanceConstant를 찾아 native `FStaticParameterSet`을 디코드하고, 그
engine-equality identity와 정확히 하나 일치하는 material map만 고른다. 그다음 corpus가 쓰는
carrier(sprite/mesh)로 VF/pass를 구조 선택해 packed DXBC slice를 꺼낸다.

```text
Tools/EffectPipeline/extract_effect_family_cooked_pixel_shaders.py
Data/Effects/Contracts/effect-family-cooked-pixel-shaders.v1.json
Data/Effects/CookedShaders/*.dxbc
```

| 항목 | 결과 |
|---|---:|
| 대상 family | 193 |
| 추출 성공 | **180** |
| 덮은 occurrence | 2,178 / 2,746 |
| permutation 선택 근거 `CHILD_MIC_ENGINE_EQUALITY` | 145 |
| 단일 permutation family | 35 |
| carrier sprite / mesh | 129 / 51 |
| 소요 | descriptor table 1회 순회 포함 53초 |

막힌 13건은 조용히 대체하지 않고 이유를 남긴다.

| blocker | 건수 | 대표 |
|---|---:|---|
| `structural pixel pass reference is ambiguous` | 5 | `fx_mm_simple_01_ad` (261 occurrence) |
| `child static set unresolved and the family has several permutations` | 7 | `fx_m_pa_smoke_01_tr` |
| `structural VF candidate is absent` | 1 | sprite structural candidate 부재 |

`fx_mm_simple_01_ad`가 가장 큰 미해결이다. VF 후보마다 서로 다른 pixel pass를 가리키므로
VF 확정 없이는 한 프로그램을 고를 수 없다. 임의 선택하지 않고 남겼다.

## 4. DXBC → HLSL 번역과 수치 A/B

### 4.1 번역기

회수된 cooked pixel shader의 명령어 집합은 산술·비교·sample 수십 종뿐이고 대부분 제어
흐름이 없다. family마다 사람이 옮길 이유가 없어서 번역기를 만들었다.

```text
Tools/EffectPipeline/translate_ue3_dxbc_to_hlsl.py
Tools/EffectPipeline/test_translate_ue3_dxbc_to_hlsl.py
```

DXBC 명령 하나가 HLSL 문장 하나가 된다. 모르는 opcode는 근사하지 않고 즉시 실패한다.
개발 중 실제로 조용히 틀린 HLSL을 만들었던 지점은 전부 unit test로 고정했다.

- **동시 쓰기**: `mul r0.xyz, r0.xxxx, r1.xyzx`는 세 lane 모두 *갱신 전* `r0.x`를 곱한다.
  component 순차 대입은 두 번째 lane부터 새 값을 먹인다. 회수한 5개 프로그램 중 3개가
  이것 때문에 원본과 달랐다. 다중 component 목적지는 lane별 임시로 staging한다.
- **swizzle 위치 색인**: `v2.yxyy`는 rank가 아니라 절대 component로 읽는다.
- **정수 도메인**: 비교는 `0xffffffff/0` lane mask를 쓰고 `and`/`movc`는 그 mask를 소비한다.
  float으로 취급하면 안 된다.
- **정수 immediate**: `l(5)`는 float 5.0이 아니라 비트 패턴 5다. `bfi`가 5 대신
  `0x40A00000`으로 마스킹하고 있었다. `asfloat(5u)`로 적어 두 도메인을 모두 지킨다.
- **early `ret`**: 중간 `ret`은 무시하지 않고 거부한다. 무시하면 원본이 건너뛰는 코드를
  실행한다.

### 4.2 원본 대조

```text
Tools/EffectPipeline/verify_ue3_dxbc_hlsl_translation.py
```

원본 cooked DXBC와 번역본 컴파일 결과를 **같은 WARP device, 같은 constant buffer, 같은
texture, 같은 sampler, 같은 carrier vertex shader**로 실행하고 모든 render target을
비교한다. 번역이 명령 단위이므로 컴파일러가 다시 재결합하지 못하도록
`D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_IEEE_STRICTNESS`로 컴파일한다.

169개 고유 프로그램을 seed `1,2,3,4,5`로 실행한 결과다. 즉 845회 program/seed 대조다.

| 등급 | 프로그램 |
|---|---:|
| `NUMERIC_EXACT` (오차 정확히 0) | **153** |
| `NUMERIC_WITHIN_PRINT_PRECISION` (≤ 1e-4) | 16 |
| `NUMERIC_MISMATCH` | **0** |
| 번역 실패 | **0** |

검증된 명령 수는 14,022개다. 16건의 잔차는 번역 결함이 아니라 디스어셈블러가 float
immediate를 소수점 6자리로 출력하기 때문이다. `1/255`가 `0.003922`로 찍혀 약 1e-5의
상대 오차가 남는다. 이 한계는 숨기지 않고 별도 등급으로 남겼다.

### 4.3 산출물

```text
Data/Effects/TranslatedShaders/Shade_Ue3_<parent leaf>[_<digest>].hlsli   (169개, 58,178줄)
Data/Effects/Contracts/effect-family-hlsl-translations.v1.json
```

같은 parent leaf가 서로 다른 DXBC를 가질 수 있어 충돌 cohort 8개는 full digest suffix로
분리한다. report의 169 digest, 169 function name, 실제 HLSLI 169개는 verifier가 exact set으로
검사하며 extra 파일도 허용하지 않는다.

## 5. 전수 조사 대비 도달점

| 항목 | 값 |
|---|---:|
| runtime executor pending family | 165 |
| 그중 cooked DXBC가 join된 family | **142** |
| runtime executor pending occurrence | 1,228 |
| 그중 cooked DXBC가 join된 occurrence | **940** |
| 전체 EXTRACTED family / 고유 program | **180 / 169** |

이 표는 program evidence 도달점이다. 번역본이 있어도 Layout, occurrence Descriptor,
carrier/VF Adapter, pass/MRT와 Product admission이 없으면 런타임 복원으로 승격하지 않는다.

## 6. 자동 검증

실행하고 통과한 것만 적는다.

```text
python -m unittest Tools.EffectPipeline.test_translate_ue3_dxbc_to_hlsl \
  Tools.EffectPipeline.test_verify_effect_family_hlsl_translations                    51 tests OK
python -m unittest Tools.EffectPipeline.test_build_effect_missing_family_inventory   21 tests OK
python Tools/EffectPipeline/build_effect_missing_family_inventory.py --check         PASS
python Tools/EffectPipeline/build_effect_family_shader_map_index.py --check           205/193
python Tools/EffectPipeline/verify_effect_family_cooked_pixel_shaders.py             193/180/13
python Tools/EffectPipeline/verify_effect_family_hlsl_translations.py                169/169
python Tools/EffectPipeline/verify_ue3_dxbc_hlsl_translation.py <169 blobs>          845/845 match
git diff --check                                                                    clean
```

Engine/Client 빌드는 실행하지 않았다. 이 세션은 C++과 `Client/Bin/ShaderFiles`를 한 줄도
바꾸지 않았기 때문이다. 같은 branch의 103개 수정 파일은 다른 세션의 Valtan 작업이며
stage하거나 되돌리지 않았다.

## 7. 이 결과가 admission하지 않는 것

- runtime admission 없음. 번역본은 `Data/Effects/TranslatedShaders`의 저작 산출물이고
  `Client/Bin/ShaderFiles`에 들어가지 않았다. renderer도 이들을 참조하지 않는다.
- vertex factory 확정 없음. `select_structural_vf_pass_candidate`는 구조 선택이며
  `actualVfPassAdmission`은 계속 false다.
- sampler state, address mode, color space, blend/depth/cull, MRT 배선은 별도 증거다.
- uniform expression → named material parameter 매핑은 아직 CB row index 그대로다.
  `cb0[11].z`가 어느 파라미터인지는 이 세션이 닫지 않았다.
- **육안 검증 없음.** 사용자의 서면 관찰 전에는 어떤 visual PASS도 기록하지 않는다.

## 8. 다음 단계 순서

1. PR #162의 180-family named-lane receipt(162 resolved / 18 blocked)를 소비하되, 이것이
   runtime ABI closure가 아님을 G00 contract에서 고정한다.
2. G00 inventory로 source child/static permutation과 carrier까지 대표 추출 행과 일치한
   occurrence만 exact program으로 세고, 대표 program만 있는 행은 permutation pending으로
   분리한다. named mapping / runtime ABI / Product admission도 서로 다른 축으로 유지한다.
3. Program/Layout/Descriptor/Adapter registry를 최소 구현하고 Sprite RT0 canary 하나를
   수직으로 닫는다.
4. 사용자 A/B 뒤 다른 캐릭터 또는 Valtan 한 occurrence로 공용성을 증명한 다음 같은 tuple
   cohort를 확장한다.

---

# 9. FlowTrail01 evidence canary — 대표 스킬 하나로 좁힌 증거 복원

## 9.1 무엇을 복원이라고 부르는가

`family 복원 = DXBC가 GPU에서 실제로 실행하는 연산을 HLSL로 옮기는 것`이 맞다. 다만
번역만으로는 복원이 아니다. 번역본은 정확하지만 **익명**이다. `cb0[11].z`를 읽는 이유는
원본이 그러기 때문이지, 그 lane이 무엇인지 알아서가 아니다. 도화가 F와 Glasshole02가
추가로 닫은 것은 다음 세 가지이고, 이 절은 같은 것을 FlowTrail01에 대해 닫는다.

```text
원본 DXBC 연산            -> 번역된 HLSL              (program evidence)
uniform expression 이름   -> named CB lane            (named mapping evidence)
texture expression        -> observed t/s register    (named mapping evidence)
FMaterialUniformExpressionTime -> 시간 반응 증거       (equation/time evidence)
```

이 네 줄은 runtime ABI packet closure가 아니다. occurrence source value, 실제 sampler state,
carrier/VF input, pass/MRT와 renderer dispatch는 이후 registry 수직 canary에서 따로 닫는다.

## 9.2 대상 선정과 근거

| 항목 | 값 |
|---|---|
| family | `fx_m_mi_02.fx_m.fx_k_me_flowtrail_01_ts_tr` (`FX_K_Me_FlowTrail_01_Ts_Tr`) |
| occurrence | 53 (lancemaster 48, warlord 5) |
| **대표 스킬** | **Lance Master `34150`, `F` slot, ACTIVE** — 53개 중 48개 |
| 요구 stance | `LANCE_MASTER_LONG_SPEAR` / action 2333 ms, hit 1345 ms |
| **solo 눈 검증 대상** | element `authored.source-particle.1dda1a259e98ed79e8fbb978` |
| child material | `fx_m_mi_n_00.fx_mi.fx_n_me_flowtrail_02_12_ts_tr` (48개 중 36개) |
| carrier / VF | mesh, `flocalvertexfactory` — **VF 후보가 정확히 하나** |
| render profile | `alpha_two_sided_depth_read` 단일 |
| mesh | `Effect/LanceMaster/Meshes/fm_k_helix_01.wmodel` |
| source emitter | `FX_PC_FLM_03.par_n_flm_earthqs_trail_01_04` |
| 프로그램 | 78 instruction, cb0 20 rows, texture 4, sampler 4 |

이 family를 고른 이유는 세 가지다.

1. missing family 중 **단일 스킬 집중도가 가장 높다**(48/53). 눈 판정이 모호해지지 않는다.
2. **VF 후보가 하나뿐**이다. Glass의 최대 미결 블로커인
   `NATIVE_EMITTER_VERTEX_FACTORY_ABI_UNPROVEN`은 particle emitter VF가 2~4개 후보를
   가질 때 생긴다. LocalVertexFactory mesh family는 그 모호성이 구조적으로 없다.
3. 다른 세션이 진행 중인 차원술사 Glasshole02와 **클래스·스킬·material·texture가 전부
   다르다**. 같은 파일을 건드리지 않는다.

## 9.3 named CB lane identity mapping

```text
Tools/EffectPipeline/build_effect_family_named_abi.py
Data/Effects/Contracts/effect-family-named-abi.v1.json
```

material map의 uniform expression set은 저작자의 파라미터 이름을 그대로 직렬화하고 있고,
native shader object의 binding wire는 각 expression이 앉는 CB0 byte offset과 texture/sampler
register를 직접 준다. 둘을 join하면 lane에 이름이 붙는다. 이 단계가 증명하는 것은
register와 저작 이름의 identity이며 occurrence value upload까지 닫지는 않는다.

```text
cb0[15].x  noise_v_pan        foldedmath(noise_v_pan, time)     <-- TIME
cb0[15].y  noise_u_pan        foldedmath(noise_u_pan, time)     <-- TIME
cb0[16].y  wave_pan_speed     foldedmath(wave_pan_speed, time)  <-- TIME
cb0[7]     noise_u/v_pan      appendvector(folded(..., time))   <-- TIME
cb0[9]     wave_pan_speed     appendvector(const, folded(...))  <-- TIME
cb0[17].z  diff_pow           scalarparameter
cb0[19].y  distortion_str     scalarparameter
cb0[3]     diff_high_color    vectorparameter
cb0[4]     diff_low_color     vectorparameter
```

전 corpus로 확장한 결과는 다음과 같다.

| 항목 | 결과 |
|---|---:|
| EXTRACTED family 분모 | **180** |
| named mapping 결과 | **162 resolved / 18 structured BLOCKED** |
| admission | `NAMED_LANE_IDENTITY_ONLY` — runtime ABI와 Product는 **0건** |

named 결과 수는 receipt를 재생성할 때마다 다음 식으로 다시 계산한다. 과거 receipt의
resolution 수를 문서 상수로 고정하지 않는다.

```text
resolved = summary.resolvedNamedMappingCount
blocked  = summary.blockedCount
assert resolved + blocked == 180
```

18 blocked는 모두 `NATIVE_BINDING_ARRAY_CANDIDATE_AMBIGUOUS`이며 receipt가 exact parent set과
candidate count를 구조화해 고정한다.

## 9.4 texture register는 순서대로가 아니다

FlowTrail01의 실제 배선이다.

| register | 역할 | source object | 출처 |
|---|---|---|---|
| `t0` / `s2` | (이름 없는 parent texture) | `fx_c_line_004_ycl` | parent `ReferencedTextures[2]` |
| `t1` / `s3` | `opacity_tex` | `fx_a_atypical_002_cl` | child MIC override |
| `t2` / `s0` | `noise_tex` | `fx_m_flow_02_n` | parent 기본값 |
| `t3` / `s1` | `diff_tex` | `fx_j_ylinestream_01_ycl` | child MIC override |

expression 순서대로 `t0,t1,t2,t3`에 꽂았다면 네 장 전부 틀린 자리에 들어간다.
expression 순서와 register 순서가 다른 family 수는 최종 named receipt의 `textureSlots`를
다시 계측해야 한다. 이전 receipt의 비율은 현재 증명값으로 사용하지 않는다.

여기서 실제 결함 하나를 찾았다. authored 문서의 이 occurrence는 texture를 **2장만**
가진다(`diff_tex`, `opacity_tex`). 누락된 두 장은 parent 소유라서 intake가 놓쳤다.

- `noise_tex` = `fx_m_flow_02_n` — parent 기본값이라 child override가 없다
- `t0` = `fx_c_line_004_ycl` — parameter가 아닌, parent graph에 박힌 texture

두 DDS 모두 실물이 있다. 다만 LanceMaster 폴더가 아니라
`Effect/DimensionMaster/Textures/FX_TEX_05/fx_m_flow_02_n.dds`,
`Effect/Artist/Textures/fx_c_line_004_ycl.dds`에 있다. **이것은 이 family만의 문제가
아니라 intake 계약의 문제다.** authored 문서는 MIC override만 기록하고 parent 기본값과
비-parameter texture를 버린다.

## 9.5 시간 가변 authored-reconstructed CB0 A/B

```text
Tools/EffectPipeline/verify_effect_family_time_varying_parity.py
Data/Effects/Contracts/effect-family-time-varying-parity.v1.json
```

occurrence의 `reconstructed_profile` 파라미터 값으로 cooked expression tree를 game time
`0.0 / 0.25 / 0.5 / 1.0 / 2.0`초에서 평가해 CB0를 만들고, 같은 WARP device에서 원본 DXBC와
현재 translator로 다시 만든 번역본을 각각 실행해 비교한다. 이 receipt는 checked-in HLSLI
identity를 증명하지 않으며, 그 identity는 bulk translation verifier와 G00이 별도로 고정한다.
두 가지를 함께 요구한다.

| gate | 결과 |
|---|---|
| `AUTHORED_RECONSTRUCTED_CB0_VALUE_PARITY` | **오차 정확히 `0.000e+00`** — reconstructed profile에서 원본 DXBC와 재생성 번역이 일치 |
| `OFFLINE_EVALUATOR_CB0_CHANGES_WITH_TIME` | `cb0[7]`, `cb0[9]`, `cb0[15]`, `cb0[16]` 네 행이 시간에 따라 변함 |

두 번째 gate가 "spawn 때 한 번만 업로드" 버그를 실행 가능한 회귀로 고정한다. 이 네 행을
매 프레임 다시 올리지 않으면 수식이 완벽해도 noise와 wave가 정지한다.

정직한 한계: replay device는 1×1 texture를 바인딩하므로 UV panning이 **출력에는** 나타날
수 없다. 그래서 motion 판정을 출력이 아니라 CB0 행에서 한다. receipt에
`outputMotionCaveat`로 남겼다.

## 9.6 지금 상태와 남은 것

FlowTrail01 evidence canary에서 닫힌 것.

- 원본 cooked program 78 instruction 확보
- HLSL 번역, 합성 입력 3 seed에서 오차 0
- **occurrence의 reconstructed profile 값, 5개 game time에서 오차 0**
- final named receipt가 `NAMED_LANE_IDENTITY_ONLY`로 승인한 CB lane과 texture register identity
- time-parity receipt가 시간에 따라 바뀐다고 관찰한 CB0 행 4개

여기서 `exact`는 pixel program equation 대조에만 붙인다. named mapping과 시간 반응은 각각
별도 evidence이며 runtime ABI packet이나 draw closure가 아니다.

닫히지 않은 것.

- **runtime 배선 없음.** C++과 `Client/Bin/ShaderFiles`를 이 세션에서 바꾸지 않았다.
  후속 registry PR은 먼저 도화가 F Sprite RT0 golden canary를 이식하고 다른 캐릭터 또는
  Valtan 한 occurrence로 공용성을 증명한다. FlowTrail01은 그 뒤 Mesh RT0 cohort의 evidence
  입력이지 최초 runtime canary가 아니다.
- occurrence source value와 parent 기본 texture를 Descriptor에 채우는 규칙.
- sampler state(address/filter/color space), blend·depth·cull, MRT 중 RT0만 쓰는 근거.
- **육안 검증 없음.** 사용자가 Lance Master `34150`(`F`)를 직접 눌러 판정하기 전에는
  어떤 visual PASS도 기록하지 않는다.
