# 2026-08-22 DXBC 전수 해석과 SOURCE_EXACT 전환 구현 계획서

## 0. 용어 정리 — family, DXBC, carrier는 서로 다른 축이다

세 단어가 같은 것을 가리킨다는 오해가 이 작업의 진행을 계속 어긋나게 했다. 실제 관계는
다음과 같다.

```text
family   = 원본 UE3 parent Material 자산 하나          (예: fx_d_me_crack_01_tr)
             └─ 그 material을 상속한 child MIC 수십 개  (예: fx_o_me_crack_01_01_tr)
                  └─ 그 MIC를 쓰는 authored element 수백 개

DXBC     = 그 family가 cook될 때 생성된 pixel shader 프로그램의 컴파일 결과 바이트코드
             family 1개 -> permutation 여러 개 -> 우리가 고른 program 1개

carrier  = 그 프로그램을 실행시킬 정점 공급자. Mesh / Sprite / Decal / Trail
             carrier 안에서 다시 VF(vertex factory)가 갈린다
```

즉 `family ≠ DXBC`다. family는 자산이고 DXBC는 그 자산의 계산식이다. 그리고
`DXBC = shader`는 맞다. 정확히는 **컴파일된 pixel shader**이며, 원본이 픽셀마다 어떤 곱셈과
어떤 sample을 어떤 순서로 했는지가 명령어 단위로 들어 있다. 그래서 이것을 oracle이라 부른다.

## 1. 현재 실측 — "DXBC가 없다"는 절반만 맞다

### 1.1 이미 있는 것

| 항목 | 값 | 위치 |
|---|---:|---|
| authored parent material family | 205 | `effect-family-shader-map-index.v1.json` |
| cooked map / pixel shader 추출 | 193 / 180 | `effect-family-cooked-pixel-shaders.v1.json` |
| 현재 family 고유 program | 169 | cooked receipt의 EXTRACTED digest 집합 |
| 번역 HLSL 함수 파일 | 169 | `Data/Effects/TranslatedShaders/*.hlsli` |
| 원본 DXBC 대비 수치 A/B `NUMERIC_MISMATCH` | **0** | 2026-08-22 translation RESULT |
| named lane identity | 162 resolved / 18 blocked / 180 | `effect-family-named-abi.v1.json` |

DXBC는 이미 뜯려 있고 번역까지 끝나 있다. 계산식은 손에 있다.
bulk WARP 대조는 169 program × 5 seed = 845회 모두 일치하지만 1×1 constant texture를 쓰므로
UV 이동, 실제 texture 의미, sampler state나 화면 motion을 증명하지 않는다.

### 1.2 그런데 화면에 안 나오는 이유 세 가지

**(a) 렌더러가 그 번역본을 안 쓴다.**
제품 렌더러의 typed executor는 40 family / 1,518 occurrence이고, 그중 원본 수식 번역
(`DXBC_REPLAYED_TRANSLATION`)은 **1 family / 86 occurrence**다. bounded reconstruction은
28 family / 1,274 occurrence, opcode 미매핑은 11 family / 158 occurrence다. runtime executor
pending은 165 family / 1,228 occurrence다.
Glasshole02와 Valtan core-three canary 4개가 번역본을 실제로 실행하지만 전부 **Tool 전용,
default-off**이며 Product admission이 없다.

**(b) family마다 손으로 배선했다.**
지금 방식은 family 하나당 `Shader_Ue3<Family>.hlsli` + `Shader_VtxEffect<Family>.hlsl` +
전용 C++ 런타임 + `Resolve_EffectStrictTypedSourceProfile`의 if 블록을 사람이 쓴다.
Valtan core-three 3개에 C++ 1,312줄 + HLSL 2,171줄이 들어갔다. 이 단가로는 180 extracted family가
불가능하다.

**(c) 산출물 exact set을 고정해야 한다.**
현재 family receipt가 소비하는 169 DXBC와 169 HLSLI는 main에 추적됐다. CookedShaders의
tracked extra 2개는 현재 bulk denominator 밖의 과거 canary이며 family program 수에 더하지
않는다. cheap verifier는 receipt의 169 digest와 HLSLI exact set을 검사한다.

### 1.3 그리고 지금까지 안 보였던 가장 큰 이유 — join key 유실

기존 인벤토리는 authored corpus를 이렇게 갈랐다.

```text
parent 보존   2,746 element   parentMaterialPath 보유 -> 현재 family 분모
parent 유실   4,826 element   child MIC가 있으면 join key 복구 대상
```

**두 번째 줄을 근사로 분류한 것이 틀렸다.** 4,826개 중 **4,793개가
`material.sourceMaterialPath`(child MIC)를
그대로 들고 있다.** MIC는 자기 `Parent` 오브젝트 참조를 직렬화하므로, 부모는 이미
staging된 같은 source package 안에서 tagged property 한 번만 읽으면 나온다. 잃어버린 것은
증거가 아니라 **join key 하나**였다.

현재 shader-map index는 parent 보존 2,746행만 본다. child-parent resolution이 복구한 신규
parent 78개는 denominator 확장 전까지 map/cooked 계약에 자동 합류하지 않는다.

스킬별 수는 같은 skillId의 baseline/candidate/unified 문서를 합치면 중복된다. 따라서 현재
분모는 PR #156에서 형성되어 PR #162 기준 그대로인 contract의 420 authored 문서 /
7,572 element와 catalog-admitted 문서 256개를
분리해 읽고, 스킬별 Product 수는 catalog가 가리키는 문서만 다시 계산한다.

## 2. 등급 사다리와 이 계획이 옮기려는 것

```text
SOURCE_EXACT          DXBC를 번역한 식을 실제 렌더러가 실행한다
BOUNDED_TRANSLATED    파라미터 이름·그룹·기본값으로 재구성한 식        <- 지금 대부분
PROJECT_RECONSTRUCTED 증거 없이 프로젝트 판단으로 만든 식
```

SOURCE_EXACT occurrence 하나를 닫으려면 다음 일곱 축이 전부 있어야 한다. DXBC는 그중
program 축 하나다.

```text
1. family join key      element -> parent material
2. cooked program       parent -> permutation -> DXBC
3. 번역과 수치 A/B      DXBC -> HLSL, 원본과 오차 0
4. named mapping        cb0[i].w -> 저작자 파라미터 이름, 관찰된 t/s register
5. runtime ABI packet   occurrence value, texture, sampler, DynamicParameter를 실제로 채운다
6. renderer adapter     carrier/VF/pass/MRT로 program을 dispatch한다
7. composition/product  timing/transform과 사용자 A/B 뒤 실제 cue로 admission한다
```

## 3. 사용자가 지목한 연출에 대한 정직한 답

`이거 복원해야 도화가 달, 차원술사 유리조각, 워로드 F/D, 창술사 적룡필살 UV 흐름이
가능한 것 아니냐` — **대체로 맞다. 하나만 다르다.**

| 연출 | DXBC가 병목인가 | 실제 상태 |
|---|---|---|
| 차원술사 유리 조각 | **program에는 그렇다** | Glasshole02 program canary가 있으나 공용 registry와 Product admission은 별도다. |
| 창술사 T 적룡필살 UV 흐름 | **program에는 그렇다** | 현재 catalog가 가리키는 clip 문서 기준으로 occurrence를 다시 resolve해야 한다. |
| 워로드 F/D | **program에는 그렇다** | 과거 합산 element 수를 쓰지 않고 현재 Product 문서의 tuple을 다시 분류한다. |
| 도화가 Z 저무는 달 도깨비불 | **셰이더만의 문제는 아니다** | source occurrence intake와 Composition 완전성을 먼저 확인한다. |

## V1. join key 전수 복구 — 완료

### 목표

authored corpus에서 parent를 잃은 element 전부에 대해 child MIC → parent Material 링크를
staged source pack에서 복구하고, 실제 family 분모를 확정한다. authored 문서는 건드리지
않는다. 문서 patch는 별도 마이그레이션이다.

### 추가 파일

```text
Tools/EffectPipeline/build_effect_child_parent_resolution.py
Tools/EffectPipeline/test_build_effect_child_parent_resolution.py
Data/Effects/Contracts/effect-child-parent-resolution.v1.json
```

### 해결 방식

1. `collect_orphan_children` — `sourceProfile.parentMaterialPath`가 없고
   `sourceMaterialPath`만 있는 element를 child 경로별로 모은다.
2. `resolve_child` — declaring package를 열어 export를 찾고, MIC면 `Parent` 참조를 읽어
   root `Material`에 닿을 때까지 걷는다. 깊이 상한 8로 자기참조·순환은 fail-closed.
3. `LeafIndex` — UE3는 import outer chain이 있는 만큼만 쓰므로 `Parent`가
   `group.leaf` 두 토막으로 오는 경우가 있다. 이때만 전 package leaf 검색으로 답하고,
   서로 다른 오브젝트로 갈리면 골라잡지 않고 ambiguous로 막는다.
   sweep은 pending leaf를 모아 한 번에 돌린다(현재 contract는 전체 2회).
4. `canonical_object_path` — 같은 material이 `package.group.leaf`와 `group.leaf` 두 철자로
   분모에 두 번 잡히던 문제를 없앤다. 실제로 선언한 package로 재수식해 오브젝트 기준으로
   합친다.

### 결과 실측

```text
orphan child 경로            809
  RESOLVED                   744   (DECLARED_PACKAGE_EXPORT 479 / LEAF_NAME_SEARCH 265)
  BLOCKED                     65   (child leaf ambiguous 19 / disagreeing 34 / 모든 package에 없음 12)
복구된 element             4,391
복구된 canonical parent      223   (기존 분모에 있던 145 / 신규 78)
그중 DXBC 이미 추출됨      3,646 element
```

이 값은 join-key evidence다. `recoveredElementsWithExtractedDxbc`도 program blob 존재만
뜻하며 named mapping, runtime ABI, Adapter 또는 Product admission으로 승격하지 않는다.
Valtan을 family 경로 밖이라고 단정했던 이전 결론은 폐기하지만, Valtan occurrence가 곧
renderable하다는 뜻도 아니다.

### 검증

```text
python -m unittest Tools.EffectPipeline.test_build_effect_child_parent_resolution   contract tests OK
python Tools/EffectPipeline/build_effect_child_parent_resolution.py --check         PASS
git diff --check                                                                    clean
```

## V2. 신규 family의 DXBC 추출과 기존 blocker 해소

### 대상

V1이 만든 분모에서 아직 cooked program이 없는 곳이다.

```text
A. 현재 retained parent denominator             205 family
B. child-parent가 찾은 신규 parent 후보          78 family
C. 복구됐지만 현재 extracted DXBC가 없는 행      746 element
```

가장 큰 단일 blocker cohort 중 하나는 `fx_mastermaterial.fx_mm.fx_mm_simple_01_ad`다.
blocker는 `structural pixel pass reference is ambiguous` — VF 후보마다 다른 pixel pass를
가리켜 프로그램 하나를 고를 수 없다. occurrence 수는 retained와 recovered를 중복 없이
합치는 확장 publisher에서 다시 산출한다.

### 순서

1. `build_effect_family_shader_map_index.py` 입력을 V1 contract까지 확장해 신규 78 family의
   base Material GUID를 해석한다.
2. `extract_effect_family_cooked_pixel_shaders.py`를 같은 분모로 재실행한다.
3. Valtan canary 3 family(`fx_d_pa_dissolve_01_ma`, `fx_d_me_crack_01_tr`,
   `fx_d_de_ground_04_tr`)는 이미 추출·번역돼 있으나 일반 index 밖에 있다. 별도 재추출이
   아니라 index에 병합한다.
4. 기존 blocked 13건은 VF 확정 결과를 입력으로 받아 다시 시도한다. 임의 선택하지 않는다.

### 종료 증거

`extract_effect_family_cooked_pixel_shaders.py`의 extracted 수와 덮은 element 수 증가,
`translate_ue3_dxbc_to_hlsl.py` + `verify_ue3_dxbc_hlsl_translation.py`에서 신규 프로그램
`NUMERIC_MISMATCH = 0`.

## V3. named mapping evidence 확장

`build_effect_family_named_abi.py`는 180 EXTRACTED family를 분모로 CB lane 이름과 관찰된
texture/sampler register를 join한다. 결과 등급 `NAMED_LANE_IDENTITY_ONLY`는 이름과 register
identity만 증명한다. source value, sampler state, carrier/VF, pass/MRT 또는 runtime binding을
admission하지 않는다. 현재 receipt는 162 resolved / 18 structured BLOCKED이며, blocked는
`NATIVE_BINDING_ARRAY_CANDIDATE_AMBIGUOUS`로만 남는다.

FlowTrail01에서 드러난 parent 기본 texture와 비-parameter texture 누락은 Descriptor intake
문제다. named mapping 성공을 runtime ABI packet closure로 기록하지 않는다.

## V4. Program / Layout / Descriptor / Adapter registry 최소 구현

```text
Program     pixel equation과 provenance
Layout      CB/texture/sampler register shape와 shader input/output shape
Descriptor  occurrence가 실제로 넣는 texture, scalar/vector, DynamicParameter 값
Adapter     carrier/VF/pass/state/MRT를 소유하는 컴파일된 renderer dispatch
```

JSON이 executable Adapter를 정의하지 않는다. immutable `CEffectCatalog` generation이 authored
registry를 parse → validate → stage → atomic commit하고, C++ compiled Adapter registry가 기존
`CEffectDocumentRenderer` draw 경로를 선택한다. 모르는 ID, tuple 불일치와 부분 binding은
fail-closed다. named mapping receipt만으로 Descriptor 또는 Adapter를 자동 생성하지 않는다.

## V5. capability-first 수직 canary

첫 capability는 `Sprite RT0 / Standard coverage`다. 도화가 F의 한 occurrence를 golden
control로 골라 기존 inline execution과 registry 결과의 packet snapshot을 비교하고, 사용자가
기존 화면과 A/B한다. 다음은 다른 캐릭터 또는 Valtan occurrence 하나로 같은 Adapter의 공용성을
증명한다. 그 뒤에만 같은 Program × Layout × Adapter tuple cohort를 Descriptor 추가로 확장한다.

Mesh RT0부터 먼저 수백 family를 여는 방식은 쓰지 않는다. 대표 occurrence 하나에서
Program × Layout × Descriptor × Adapter × Composition을 끝까지 닫은 뒤 cohort를 넓힌다.

## V6. capability 순서와 Product admission

```text
Sprite RT0 -> Mesh RT0 -> LocalDecal -> Trail/Ribbon -> Glass/Scene/MRT
           -> WPO vertex -> Light/ScreenPost/ModelCue presentation
```

각 capability는 자동 harness와 Client build가 통과해도 Product가 아니다. occurrence별
Composition, V0 A/B와 사용자의 서면 visual 판정이 있어야 admission한다. 에이전트는 실행
준비와 수치 진단까지만 하고 `manual first pixel`이나 `visual PASS`를 대신 기록하지 않는다.

## 7. 이 계획이 admission하지 않는 것

- V1은 join key만 복구한다. cooked program, VF, sampler state, runtime 배선, 시각 결과 중
  어느 것도 admission하지 않으며 authored 문서를 수정하지 않는다.
- 현재 bulk receipt의 169 DXBC와 169 HLSLI는 program evidence일 뿐 runtime registry가 아니다.
- 도화가 Z 저무는 달은 source occurrence intake와 Composition 완전성을 먼저 확인한다.
- WPO(world position offset)는 정점을 미는 출력이라 pixel DXBC 번역으로 열리지 않는다.
  vertex shader DXBC를 따로 회수해야 한다.
