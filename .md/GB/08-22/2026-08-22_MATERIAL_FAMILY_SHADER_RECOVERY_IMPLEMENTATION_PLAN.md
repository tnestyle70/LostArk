# 2026-08-22 Material Family Shader Recovery 구현 계획서

## 0. 이 계획서가 소유하는 것과 판정자

이 문서는 원본 Material program evidence를 분류하는 G00과, 그 증거를 공용 runtime
capability로 올리는 순서를 소유한다. named mapping을 runtime ABI closure로 부르지 않는다.

`gotchas.md 12.6`에 따라 목표보다 판정자를 먼저 쓴다.

```text
G00 판정자   대상 스킬의 모든 source row가 program / named mapping / runtime ABI /
             Product admission의 서로 다른 증거 축으로 분류되고,
             분류 불명 row가 0이다.
G01 판정자   Program/Layout/Descriptor/Adapter registry가 parse -> validate -> stage ->
             atomic commit하고 모르는 tuple은 fail-closed다.
G02 판정자   Sprite RT0 도화가 F canary 1행이 기존 inline packet과 동일하고 사용자가
             기존 화면과 A/B한다.
G03 판정자   다른 캐릭터 또는 Valtan 1행이 같은 compiled Adapter에서 Descriptor만으로
             갈려 공용성을 증명한다.
G04 판정자   같은 tuple cohort를 확장한 뒤 Mesh RT0 -> LocalDecal -> Trail -> Glass/MRT ->
             WPO -> Presentation 순서로 capability를 연다.
G05 판정자   occurrence별 V0 A/B 후 사용자가 KEEP/REPLACE/ADD/RETIRE를 서면 판정한다.
```

에이전트는 G02, G05의 화면 판정을 대신하지 않는다. 빌드, 구조화된 로그, draw/resource
수치 진단과 실행 준비까지 수행하고 멈춘다.

이 계획서는 `NATIVE_PARITY`를 완료 조건에 넣지 않는다. `V1_COMPLETE`의 정의는 다음으로
고정한다.

```text
V1_COMPLETE
= 올바른 carrier
+ family별 RT0 Base HLSL
+ texture/channel/scalar/DynamicParameter 배선
+ blend/depth
+ attachment/timing
+ Effect Tool 편집·저장
+ 사용자 육안 승인
```

이 완료식은 occurrence 단위다. capability 구현만으로 다른 occurrence를 자동 Product로
승격하지 않는다.

## 1. 현재 실측

### 1.1 Family와 DXBC는 서로 다른 단위다

`Data/Effects/Contracts`의 shader-map, cooked, translation, named ABI, child-parent 다섯
evidence 계약을 함께 실측한 수치다.

```text
family (parent material)          205
cooked material-map 분모          193
DXBC blob 추출 family             180
고유 family program               169
HLSL 함수                         169
```

family 하나가 DXBC 여러 개를 갖고, DXBC 하나가 family 여러 개를 담당한다. 저장 계약에서
둘을 같은 키로 쓰지 않는다.

### 1.2 번역은 닫혔고 binding이 안 닫혔다

```text
effect-family-cooked-pixel-shaders.v1.json   193 대상 / 180 EXTRACTED / 13 BLOCKED
effect-family-hlsl-translations.v1.json      169 프로그램 / NUMERIC_MISMATCH 0
effect-family-named-abi.v1.json              180 대상 / 162 RESOLVED / 18 BLOCKED
```

named receipt의 `NAMED_LANE_IDENTITY_ONLY`는 register와 저작 이름의 관계만 증명한다.
literal HLSL program과 named mapping이 둘 다 있어도 occurrence source value, sampler,
DynamicParameter, carrier/VF/pass/MRT dispatch가 없으면 runtime ABI closure는 0이다.

### 1.3 도화가 F는 family를 쓰지 않았다

`Data/Effects/Authored/effect.artist.skill.31470.unified.effect.json` 17행 전수다.

```text
sourceProfile.enabled      false × 17
참조 parent material        13종
carrier(kind)              trail 1 / particle 14 / decal 2
execution backend          runtimeMaterialV2 8 / artistVisualV4 7 / localDecal 2
(backend, opcode) 조합      12종 / 17행
textureLaneCount           2~6
```

PR #156에서 형성되어 PR #162 기준 그대로인 authored corpus는 420 문서 / 7,572 element이며
enabled `material.execution`은 50행이다.
도화가 F 17행은 전부 execution 경로다. golden이 성립한 이유는 family가 아니라 행마다 확정된
carrier·lane·register·sampler·renderState다.

`gotchas.md 12.5`는 이 사실을 "스킬 수만큼 셰이더가 필요하다"로 결론냈다. 그 전제였던
`셰이더를 사람이 한 벌씩 쓴다`는 1.2의 자동 번역으로 무너졌다. 결론의 유효한 부분만 남긴다.
**Program은 skill이 아니라 equation identity를 키로 만들고, Adapter는 capability를 키로
만든다.** occurrence 차이는 Descriptor와 Composition이 소유한다.

### 1.4 execution과 sourceProfile은 상호 배타다

`Client/Private/Effect_DocumentRenderer.cpp`의 `Stage_AuthoredMaterialExecution`이
`Execution.iOpcode == 0u || Element.Material.SourceMaterial.bEnabled`를 실패로 처리한다.
도화가 F가 `sourceProfile.enabled=false`인 이유가 이 게이트다.

`EFFECT_MATERIAL_TEXTURE_LANE_DESC`와 codec에는 이미 `iTextureRegister`와
`iSamplerRegister`가 있다. 그러나 legacy staging은 `Staged.iSamplerRegister = 5u + iLane`을
만들고 generic validation도 `sampler = 5 + textureRegister`를 강제한다. 따라서 필드 추가가
아니라 Layout/Adapter별 register policy 분리가 필요한 상태다. 기존 execution 문서의 legacy
규칙은 유지하고 새 registry tuple만 exact layout을 소비해야 한다.

backend와 carrier의 대응도 같은 함수가 강제한다.

```text
LOCAL_DECAL        -> EFFECT_ELEMENT_KIND::DECAL 만
ARTIST_VISUAL_V4   -> MESH 또는 PARTICLE 만
```

### 1.5 대상 스킬 실측 — 창술사 D / F

`material.sourceMaterialPath`의 exact child receipt를 먼저 적용하고, occurrence의 selected child
static permutation과 `sourceRecipe.rendererShape` carrier까지 cooked program과 대조한 결과다.

`effect.lancemaster.skill.34110.unified.effect.json` (D, skillId 34110) — 88 element
(particle 83, decal 5), source row 83.

```text
runtimeShaderProfileId   grouped-translucent 74 / procedural-center-glow 8 /
                         missiletrail-two-emissive 1

parent leaf                        n   cooked      named snapshot             carrier  instr
fx_mm_simple_01_ad                15   BLOCKED     MISSING                    -        -
fx_mm_basic_01_tr                 12   EXTRACTED   NAMED_ONLY                 sprite   58
bfx_i_pa_glow_01_ad                8   EXTRACTED   NAMING_MISSING             sprite   -
fx_mm_simple_03_tr                 8   BLOCKED     MISSING                    -        -
fx_m_me_watertrail_01_tr           6   EXTRACTED   NAMED_ONLY                 mesh     103
fx_m_me_trail_02_tr                6   EXTRACTED   NAMED_ONLY                 mesh     89
fx_m_pa_trail_01_4_tr              5   MISSING     MISSING                    -        -
fx_c_pa_lensflare_01_ad          4+3   EXTRACTED   NAMED_ONLY                 sprite   42
fx_mm_dissolve_01_ad               3   EXTRACTED   NAMED_ONLY                 sprite   46
fx_mm_basic_01_ad                  2   EXTRACTED   NAMED_ONLY                 sprite   26
fx_d_pa_shine_02_ad                2   EXTRACTED   NAMED_ONLY                 sprite   56
fx_mm_extraalpha_01_ad             2   EXTRACTED   NAMED_ONLY                 sprite   41
fx_m_pa_noise_01_tr                2   EXTRACTED   NAMED_ONLY                 sprite   65
fx_d_pa_sqc_01_tr                  2   BLOCKED     MISSING                    -        -
fx_e_me_rock_01_ma                 2   EXTRACTED   NAMED_ONLY                 mesh     98
fx_m_pa_spritewave_01_tr           1   EXTRACTED   NAMED_ONLY                 mesh     119

PROGRAM_EXACT 27 / 83, 그중 named mapping evidence 보유 19 / 83이다. family 대표 program은
있지만 target child 또는 carrier가 달라 permutation pending인 행은 31 / 83이다. 나머지
25행은 DXBC blocker다. 이 수는 G00 builder가 실데이터에서 다시 계산한다.
```

`effect.lancemaster.skill.34150.unified.effect.json` (F, skillId 34150) — 186 element 전부
particle, source row 186.

```text
runtimeShaderProfileId   grouped-translucent 168 / circle 18

parent leaf                        n   cooked      named snapshot             carrier  instr
fx_k_me_flowtrail_01_ts_tr        48   EXTRACTED   NAMED_ONLY                 mesh     78
fx_m_pa_trail_01_4_tr             24   MISSING     MISSING                    -        -
fx_c_pa_lensflare_01_ad         18+9   EXTRACTED   NAMED_ONLY                 sprite   42
bfx_d_pa_circ_01_ad               18   MISSING     MISSING                    -        -
fx_k_me_makeflow_02_tr            18   EXTRACTED   NAMED_ONLY                 mesh     92
fx_mm_basic_01_ad                 12   EXTRACTED   NAMED_ONLY                 sprite   26
fx_mm_distortion_01_ad             9   EXTRACTED   NAMING_MISSING             sprite   -
fx_j_me_flamesurface_01_ma         9   EXTRACTED   NAMED_ONLY                 mesh     119
fx_mm_simple_01_ad                 9   BLOCKED     MISSING                    -        -
fx_mm_extraalpha_01_tr             6   EXTRACTED   NAMED_ONLY                 sprite   60
bfx_c_pa_lightflare_01_ddt_ad      3   EXTRACTED   NAMED_ONLY                 sprite   25
fx_mm_dissolve_01_tr               3   EXTRACTED   NAMED_ONLY                 sprite   84

PROGRAM_EXACT 104 / 186, 그중 named mapping evidence 보유 95 / 186이다. family 대표 program은
있지만 target child 또는 carrier가 달라 permutation pending인 행은 55 / 186이다. 나머지
27행은 DXBC 9 + parent-only 18이다. 이 수는 G00 builder가 실데이터에서 다시 계산한다.
```

두 문서 모두 `execution.backend`를 가진 행이 0이다. 즉 전부 generic 경로다.

`instr` 열은 `named-abi`가 기록한 대표 pixel program instruction count다. 표에 두 번 나타나는
`fx_c_pa_lensflare_01_ad`는 #164의 두 exact child가 같은 known family를 가리키고 그 family가
single-permutation이므로 G00에서는 occurrence 34개의 한 parent key로 합쳐진다.

### 1.6 창술사 F의 용 UV 흐름은 program/time evidence가 있다

`fx_m_mi_02.fx_m.fx_k_me_flowtrail_01_ts_tr` — 48행, 단일 family로 이 스킬 최대 지분이다.

```text
carrier                mesh
child MIC              fx_m_mi_n_00.fx_mi.fx_n_me_flowtrail_02_12_ts_tr
instruction            78
translated function    Shade_Ue3_fx_k_me_flowtrail_01_ts_tr
constant buffer        cb0 20 float4 / cb2 4 float4
texture slot           t2/s0 noise_tex
                       t3/s1 diff_tex
                       t0/s2 (parameterName 없음, referencedTexture)
                       t1/s3 opacity_tex
scalar lane            18 (named 18)
vector lane            13 (named 13)
timeDependent          cb0[15].x  cb0[15].y  cb0[16].y  cb0[7]  cb0[9]
```

이 행들은 시간 의존 expression이다. WARP replay의 1×1 texture는 UV 위치가 달라도 같은
texel을 반환하므로 화면 motion을 증명하지 못한다. receipt가 증명한 것은 CB 행이 시간에 따라
변하고 원본/번역 program의 RT0 값이 같은 것까지다.

같은 program/mapping evidence를 가진 두 번째 family가 재사용 후보였다.

```text
fx_k_me_makeflow_02_tr   18행 mesh 101 instr
  t0/s0 flowtex   t1/s1 diff_tex1   t3/s2 diff_tex2   t2/s3 opacity_tex
  scalar 19 / vector 9 / timeDependent cb0[12].x cb0[12].y cb0[4] cb0[6] cb0[7]
```

texture 이름과 lane 수가 다르고 sampler register 배치도 다르다. 그러나 이것은 Mesh RT0
capability의 후속 cohort다. 최초 수직 canary는 Sprite RT0 도화가 F로 바뀌었으며, 이 두
family를 먼저 직접 배선하지 않는다.

### 1.7 과거 join blocker 47행의 최신 상태

초기 G00은 두 경로를 `PARENT_ONLY` 47행으로 셌다. #164 receipt를 exact source child로
소비한 뒤 trail 29행은 parent join이 닫혔고, circ 18행만 parent-only로 남는다.

```text
fx_m_mi_m_00.fx_mi.fx_m_pa_trail_01_4_tr        D 5 + F 24 = 29행
  #164 parent     fx_m_mi_03.fx_mi.fx_m_pa_trail_01_tr
  program         family 대표 DXBC와 named mapping은 있음
  target child    fx_m_mi_m_00.fx_mi.fx_m_pa_trail_01_4_tr
  selected child  fx_m_mi_s_00.fx_mi.fx_s_pa_trail_03_01_tr
  status          PROGRAM_PERMUTATION_PENDING_NAMED_MAPPING_ONLY

bfx_m.bfx_d_pa_circ_01_ad                        F 18행
  index 등록      있음 (corpus 전체 occurrence 120)
  resolvedBy      LEAF_NAME_AMBIGUOUS
  cookedEvidence  BASE_MATERIAL_ID_UNRESOLVED
```

두 번째 경로에는 같은 leaf의 **완전 수식 경로가 따로 존재하지만**, 동일 material로 자동
합치지 않는다.

```text
bfx_m.bfx_d_pa_circ_01_ad                 corpus 120행  LEAF_NAME_AMBIGUOUS  미해결
bfx_m_mi_00.bfx_m.bfx_d_pa_circ_01_ad     corpus  17행  DECLARED_PACKAGE_EXPORT
                                                        COOKED_MATERIAL_MAPS_PRESENT
```

짧은 표기의 group.object와 package export identity가 유일한지 확인되기 전에는 같은 family로
가정하지 않는다. G01의 최우선 parent blocker다.

## 2. 고정 용어

G00은 다음 여덟 occurrence 상태를 사용한다. switch fallback으로 모르는 상태를
정상값처럼 처리하지 않는다.

```text
PROGRAM_EXACT_NAMED_MAPPING_ONLY
                              selected child/static permutation과 carrier가 cooked row와 같고
                              literal translation + named identity가 있다.
PROGRAM_EXACT_NAMED_MAPPING_MISSING
                              selected program은 같지만 named identity가 blocked다.
PROGRAM_PERMUTATION_PENDING_NAMED_MAPPING_ONLY
                              family 대표 program은 있으나 target child/carrier가 다르다.
PROGRAM_PERMUTATION_PENDING_NAMED_MAPPING_MISSING
                              위 상태이며 named identity도 blocked다.
SHADERMAP_FOUND_DXBC_MISSING  shader map은 있으나 permutation 선택이 막혀 blob이 없다.
PARENT_RESOLVED_PROGRAM_MISSING
                              #164가 신규 parent를 해소했지만 cooked denominator 밖이다.
PARENT_ONLY                   shader map parent는 있으나 cooked evidence가 없다.
UNKNOWN                       exact child/parent가 계약 어디에도 없다.
```

translation set이 cooked EXTRACTED denominator와 다르면 status로 낮추지 않고 전체 build를
실패시킨다. 모든 행은 별도로 `runtimeAbiClosure=NOT_PROVEN`,
`productAdmission=NOT_ADMITTED`를 기록한다. 여기서 Program exact는 selected DXBC에 대응하는
literal translation artifact까지이며 별도 numeric replay receipt나 runtime 실행을 뜻하지 않는다.

`carrier`는 두 층으로 분리한다. 한 단어로 쓰지 않는다.

```text
Carrier identity           원본이 Sprite/Mesh/Decal/Trail 중 무엇인지 맞다.
Carrier execution closure  그 family의 셰이더가 요구하는 VF input이 전부 전달된다.
                           (instance transform, particle color, dynamic parameter,
                            age/lifetime, normal/tangent)
```

1.5의 `carrier` 열은 identity가 아니라 **원본 permutation 선택에 사용된 carrier**다.
execution closure는 G02에서 family마다 따로 판정한다.

## 3. 변경할 파일

```text
새 파일
  Tools/EffectPipeline/build_effect_family_shader_inventory.py
  Tools/EffectPipeline/test_build_effect_family_shader_inventory.py
  Data/Effects/Contracts/effect-family-shader-inventory.v1.json      (생성물)
```

G00 PR에는 C++/HLSL을 넣지 않는다. Program/Layout/Descriptor/Adapter registry와 Sprite RT0
canary는 G00 증거가 main에 들어간 뒤 별도 계획·PR에서 project/filter 등록까지 닫는다.

## 4. 데이터와 호출 흐름

```text
Data/Effects/Authored/<doc>.effect.json
  element.material.sourceMaterialPath + sourceProfile.parentMaterialPath
        │
        ▼ exact child join (#164) + selected permutation/carrier 대조
Data/Effects/Contracts/effect-family-shader-inventory.v1.json
  occurrence status 8종 -> family별 evidenceClasses/MIXED 집계
  program exact / representative pending / named / runtime / Product 증거 축
        │
        ▼ (후속 registry PR)
Program + Layout + occurrence Descriptor + compiled Adapter
        │
        ▼ (런타임)
CEffectCatalog generation -> CEffectDocumentRenderer 기존 draw 경로
```

publisher는 기존 `Tools/EffectPipeline/Publish-Effects.ps1` 하나만 사용한다. 두 번째
runtime 경로를 만들지 않는다.

## 5. G00 — Family Shader Inventory

### 5.1 목표와 종료 증거

대상 스킬의 모든 source row를 §2의 occurrence status로 분류하고 계약 파일로 고정한다.
분류 불명 row 0이 종료 증거다.

대상은 다음 문서로 시작한다.

```text
effect.lancemaster.skill.34110.unified.effect.json     D
effect.lancemaster.skill.34150.unified.effect.json     F
effect.artist.skill.31470.unified.effect.json          golden 대조군
```

golden을 포함하는 이유는 `execution` 행을 별도 계수하고 source/execution 동시 enabled를
거부해 두 저작 경로를 혼동하지 않는지 검증하기 위해서다.

### 5.2 build_effect_family_shader_inventory.py

한 줄 책임: source child를 child-parent receipt로 해소하고, occurrence의 static permutation과
carrier를 shader-map/cooked/translation/named evidence와 join해 독립 evidence 축을 생성한다.

```text
입력
  Data/Effects/Authored/<대상 문서>
  Data/Effects/Contracts/effect-family-shader-map-index.v1.json
  Data/Effects/Contracts/effect-family-cooked-pixel-shaders.v1.json
  Data/Effects/Contracts/effect-family-hlsl-translations.v1.json
  Data/Effects/Contracts/effect-family-named-abi.v1.json
  Data/Effects/Contracts/effect-child-parent-resolution.v1.json

처리
  parse     sourceProfile 행과 execution 행을 분리 계수
  validate  schema/version/self-hash, upstream raw pin, exact denominator, 중복, LF를 검증
  stage     sourceMaterialPath exact child join -> selected child/permutation + carrier 대조
            RESOLVED receipt만 parent를 교정하고, 그 외에는 authored parent key를 보존한다.
            BLOCKED/부재 child를 추측 remap하지 않으며 leaf fallback도 없다.
  commit    전체 성공 시에만 계약 파일 하나를 원자 교체

출력 행
  parentMaterialPath, occurrenceCount, documents[], sourceParentMaterialPaths[],
  sourceMaterialPaths[], parentResolutions[], status, evidenceClasses[],
  programExactOccurrenceCount, programPermutationPendingOccurrenceCount,
  programEvidence, namedMappingEvidence, runtimeAbiClosure, productAdmission,
  carrier, childMaterialPath, permutationSelection/count, dxbcSha256,
  translatedFunctionName, instructionCount,
  scalarLaneCount, vectorLaneCount, textureSlots[], timeDependentRegisters[],
  blocker
```

leaf fallback을 금지하는 이유는 동명 leaf가 서로 다른 material/permutation을 가질 수 있기
때문이다. lensflare는 leaf가 같아서가 아니라 두 exact child receipt가 같은 known family를
가리키고 그 family가 single-permutation이라서만 합친다.

### 5.3 실패 처리

```text
입력 계약 파일 없음          즉시 실패. 이전 계약 파일 보존
parentMaterialPath 없음      UNKNOWN 으로 분류. 조용히 버리지 않는다
schema/self-hash/raw pin drift 즉시 실패. 이전 계약 파일 보존
translation missing/extra/중복 전체 denominator 오류로 실패
selected child/carrier 불일치 PROGRAM_PERMUTATION_PENDING 으로 분리
program exact, named 없음     PROGRAM_EXACT_NAMED_MAPPING_MISSING 으로 분리
named mapping 존재          runtimeAbiClosure와 productAdmission은 계속 미승격
child receipt BLOCKED/부재    authored parent key를 유지하되 추측 remap/admission 금지
```

### 5.4 test_build_effect_family_shader_inventory.py

```text
정상 join 1건이 PROGRAM_EXACT_NAMED_MAPPING_ONLY 로 분류되되 runtime 미승격인가
selected child 또는 carrier mismatch가 PROGRAM_PERMUTATION_PENDING 으로 남는가
single-permutation family도 carrier mismatch면 pending으로 남는가
같은 family 안 exact/pending occurrence가 MIXED로 보존되는가
두 exact child alias가 같은 known single-permutation family로만 합쳐지는가
exact child-key RESOLVED만 remap하고 BLOCKED/부재는 추측 remap하지 않는가
new family RESOLVED가 denominator 밖이면 PARENT_RESOLVED_PROGRAM_MISSING인가
child receipt의 rowSha/duplicate/dependency pin/new-family 의미 drift를 거부하는가
RESOLVED child의 known family/evidence가 receipt families 집계와 다르면 거부하는가
named mapping blocked 1건이 exact program을 잃지 않고 별도 상태로 남는가
cooked BLOCKED 1건이 SHADERMAP_FOUND_DXBC_MISSING 으로 분류되는가
같은 leaf 다른 package 두 family 가 두 행으로 남는가
Artist F execution 행을 source와 분리하고 둘이 동시에 enabled면 거부하는가
schema/hash/raw pin/denominator/duplicate/CRLF가 각각 fail-closed인가
중간 실패와 --check stale에서 기존 계약 파일이 그대로인가
```

### 5.5 종료 명령

```text
python Tools/EffectPipeline/build_effect_family_shader_inventory.py
python -m unittest Tools.EffectPipeline.test_build_effect_family_shader_inventory
git diff --check
```

기대: 대상은 25 unique parent family / 269 source occurrence이며 Artist F는 source 0 /
execution 17이다. occurrence 기준 exact named 114, exact naming-missing 17, permutation pending
named 86, DXBC missing 34, parent-only 18이고 runtime ABI/Product는 0이다. mixed family가 3개라
status family coverage 합은 unique family 수와 같지 않다.

## 6. Evidence blocker backlog — registry G01과 분리

### 6.1 목표와 종료 증거

G00이 `SHADERMAP_FOUND_DXBC_MISSING`, `PARENT_ONLY`, `UNKNOWN`으로 분류한 family는
blocker를 데이터 레벨에서 재현한다. named mapping 실패는 program 실패와 분리한다.

현재 D/F source 269행은 `exact 131 + representative pending 86 + Program 없음 52`로
정확히 닫힌다. 아래 표는 이 가운데 shader-map/cooked Program 자체가 없는 52행이다.

```text
parent leaf                 행    status                        blocker
fx_mm_simple_01_ad          24    SHADERMAP_FOUND_DXBC_MISSING  pixel pass ambiguous
bfx_d_pa_circ_01_ad         18    PARENT_ONLY                   short-path resolver denominator stale
fx_mm_simple_03_tr           8    SHADERMAP_FOUND_DXBC_MISSING  pixel pass ambiguous
fx_d_pa_sqc_01_tr            2    SHADERMAP_FOUND_DXBC_MISSING  child static set unresolved
                            ──
                            52
```

별도로 86행은 family 대표 Program은 있지만 target child/static permutation 또는 carrier가
대표 추출 행과 달라 occurrence exact가 아니다. 이 cohort에서는 trail 29행이 가장 큰 단일
회수 대상이며, representative Program을 그대로 복사하지 않고 target permutation을 추가로
추출해야 한다.

### 6.2 각 blocker의 처리 방침

```text
LEAF_NAME_AMBIGUOUS — bfx_d_pa_circ_01_ad (F 18행, corpus 120행)   [최우선]
  원본 package 실측상 bfx_m.bfx_d_pa_circ_01_ad 는 group.object와 정확히 일치하는 export가
  하나이며, 완전 경로 쪽 material과는 다른 GUID다. 둘을 alias로 합치지 않는다.
  shader-map resolver가 2-segment 경로를 leaf 검색으로 낮추기 전에 group.object exact match를
  적용하도록 별도 denominator PR에서 고치고 downstream 계약을 순서대로 재생성한다.
  이 한 건이 4캐릭터 120행을 좌우하므로 다른 parent-only blocker보다 먼저 판정한다.

PROGRAM_PERMUTATION_PENDING — fx_m_pa_trail_01_4_tr (29행)
  #164가 fx_m_mi_03.fx_mi.fx_m_pa_trail_01_tr parent로 정확히 해소했다.
  family 대표 DXBC/named mapping도 있으므로 parent나 대표 수식을 다시 찾는 작업이 아니다.
  target child fx_m_mi_m_00...trail_01_4_tr 의 static parameter set과 실제 carrier를 추출해
  해당 pixel program을 cooked denominator에 추가하고 occurrence exact 여부를 다시 판정한다.

no sample pair / empty texture denominator
  texture-free pixel program에는 합법이다. named mapping은 빈 texture set을 그대로 기록한다.
  이것을 runtime ABI closure나 Distortion Adapter admission으로 바꾸지는 않는다.

pixel pass ambiguous — fx_mm_simple_01_ad 24행, fx_mm_simple_03_tr 8행
  VF 후보마다 다른 pixel pass 를 가리킨다. 대상 스킬이 실제로 쓰는 carrier 를
  저작 문서에서 읽어 VF 를 한 개로 좁힐 수 있는지 확인한다.
  좁혀지지 않으면 임의 선택하지 않고 BLOCKED 로 남긴다.

child static set unresolved — fx_d_pa_sqc_01_tr (2행)
  근거가 0이면 family 를 만들지 않는다. gotchas 12.6 에 따라 판정자 없는 목표로
  전환하지 않고 backlog 로 분리한다.
```

### 6.3 종료 증거

`Data/Effects/Contracts/effect-family-shader-inventory.v1.json`의 blocker 열이 위 방침과
1:1로 일치하고, 추측으로 status가 올라간 행이 0이다.

## 7. G02 — Sprite RT0 golden canary

첫 수직 canary는 창술사 FlowTrail mesh가 아니라 도화가 F의 Sprite RT0 occurrence 하나다.
이미 보이는 도화가 F를 golden control로 쓰면 registry 이식 전후 packet과 화면을 비교할 수
있고, 새 수식의 미지수와 새 dispatch의 미지수를 동시에 열지 않는다.

```text
1  Program/Layout/Descriptor/Adapter ID를 immutable catalog generation에 stage한다.
2  기존 inline execution과 registry resolve 결과의 resource signature와 packet snapshot을 비교한다.
3  불일치, unknown ID, partial binding은 기존 generation을 유지하고 fail-closed한다.
4  Client Debug build와 focused harness 뒤 사용자가 도화가 F를 기존 화면과 A/B한다.
5  사용자 서면 판정 전에는 visual PASS나 Product admission을 기록하지 않는다.
```

## 8. G03 — 다른 캐릭터로 Adapter 공용성 증명

도화가 F golden canary가 통과한 뒤 차원술사 F의 Sprite occurrence 하나를 같은 compiled
Sprite Adapter에 연결한다. equation과 Layout은 달라도 carrier/VF/pass capability가 같으면
Adapter 코드는 바뀌지 않아야 한다.

```text
허용 변경   Program/Layout/Descriptor registry row
금지 변경   skillId/elementId 분기, 두 번째 renderer/runtime 경로, Adapter 코드 복제
종료 증거   동일 Adapter ID, packet snapshot valid, Client build, 사용자 수동 A/B 준비
```

## 9. G04 — tuple cohort와 capability 확장

G03 이후 같은 Program × Layout × Adapter tuple만 Descriptor 추가로 확장한다. 다른 equation은
Program, 다른 ABI shape는 Layout, 다른 carrier/VF/pass/MRT는 Adapter를 새로 연다.

```text
Sprite RT0 cohort
-> Mesh RT0
-> LocalDecal
-> Trail/Ribbon
-> Glass/Scene/MRT
-> WPO vertex
-> Light/ScreenPost/ModelCue presentation
```

`PROGRAM_PERMUTATION_PENDING`, `SHADERMAP_FOUND_DXBC_MISSING`, `PARENT_ONLY`, `UNKNOWN`은
capability 확장 중 추측으로 승격하지 않는다. child-parent 신규 family 78개도 map/cooked
denominator publisher가 닫힌 뒤에만 Program 후보가 된다.

## 10. G05 — Product 승격과 V0 A/B

### 10.1 절차

```text
1  capability registry로 열린 occurrence를 V0 손튜닝 문서와 나란히 재생한다
2  사용자가 행 단위로 KEEP / REPLACE / ADD / RETIRE 를 판정한다
3  REPLACE 로 판정된 행만 V0 의 transform/timing 을 이식한다
4  Effect Tool 에서 위치·크기·색·타이밍을 손튜닝하고 Save/Reload 한다
5  사용자 서면 승인 후 Product 문서로 승격한다
```

### 10.2 이식에서 반드시 확인할 것

`gotchas.md 12.2`에 따라 `sourceRecipe.enabled`를 먼저 본다.

```text
sourceRecipe.enabled = true   재생이 원본 모듈을 따라가고 Detail.Particle 이 무시된다
Detail.Transform              게이트 없음. 크기·위치 튜닝은 먹는다
Detail.Particle               게이트 있음. particle 수 튜닝은 안 먹는다
```

튜닝이 안 먹으면 셰이더가 아니라 이 플래그를 본다.

### 10.3 종료 증거

사용자의 서면 판정과 승인. 에이전트가 `visual PASS`를 대신 기록하지 않는다.

## 11. 이 계획서가 하지 않는 것

```text
named mapping만으로 runtime ABI closure 또는 Product admission 선언
근거 0인 family의 추측 복원
Sprite canary 전에 Mesh/Trail/Glass/WPO 수평 대량 확장
같은 skillId의 baseline/candidate/unified 문서 중복 합산
Client/UI 자율 실행, 화면 캡처, 에이전트의 visual 판정
```

## 12. 전체 검증

```text
1  python -m unittest Tools.EffectPipeline.test_build_effect_family_shader_inventory
2  python Tools/EffectPipeline/build_effect_family_shader_inventory.py
3  python Tools/EffectPipeline/build_effect_family_shader_inventory.py --check
4  JSON parse와 artifact/raw provenance 확인
5  git diff --check
```

G00은 Python/JSON evidence PR이라 Engine/Client 빌드와 Effect Tool smoke 대상이 아니다.
registry/Client PR부터 정본 Client build와 사용자 전용 수동 A/B를 별도로 실행한다.

## 13. 브랜치와 커밋 경계

shader-map/cooked/translation evidence는 PR #160/#161, named mapping은 PR #162, child-parent
receipt는 PR #164로 main에 들어갔다. G00은 다섯 계약의 raw/artifact identity와 dependency
pin을 직접 검증한다. #164는 source child exact join의 직접 입력이며, RESOLVED 행만 parent를
교정한다. evidence G00 PR과 runtime registry PR을 섞지 않고 각 브랜치에서 exact path만
stage한다. 다른 세션의 authored 문서와 Valtan publish output은 반입하지 않는다.
