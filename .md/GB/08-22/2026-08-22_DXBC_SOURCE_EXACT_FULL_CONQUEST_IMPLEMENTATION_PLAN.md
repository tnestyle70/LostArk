# 2026-08-22 DXBC 전수 해석과 SOURCE_EXACT 전환 구현 계획서

> **[2026-08-23 실행 기준선 동기화]** 이 계획서를 처음 쓸 때의 main은 `bc395406`
> (PR #156)이었다. 현재 구현 기준선은 `origin/main@4593e539`이며 PR #175의
> S6/M3/D14 공용 runtime까지 포함한다.
>
> - V1(join key 복구)은 PR #164, family shader inventory는 PR #166으로 merge됐다.
> - PR #168이 `CEffectMaterialProgramRegistry`를 만들고 PR #172가 도화가 F Sprite S6를
>   실제 draw에 연결했다. PR #175는 같은 spine을 Mesh M3와 LocalDecal D14까지 확장해
>   Program/Layout/Descriptor/Adapter/Binding을 각각 3개씩, 모두
>   `INLINE_MIRROR_REQUIRED`로 검증했다.
> - 따라서 아래의 예전 V5/V6 목표인 “registry를 새로 만든다”는 폐기한다. 현재 목표는
>   **같은 registry와 기존 `CEffectDocumentRenderer` draw 경로에 class-neutral Program을
>   전수 materialize하고, occurrence 소유 세션이 Descriptor/Binding fragment를 붙일 수 있는
>   public handoff를 닫는 것**이다. 두 번째 renderer는 만들지 않는다.
> - 현재 정본 실측은
>   [`2026-08-23_EFFECT_TUPLE_COHORT_INVENTORY_RESULT.md`](../08-23/2026-08-23_EFFECT_TUPLE_COHORT_INVENTORY_RESULT.md),
>   공용 runtime 완료 증거는
>   [`2026-08-23_EFFECT_MATERIAL_REGISTRY_SMD_COMMON_RUNTIME_RESULT.md`](../08-23/2026-08-23_EFFECT_MATERIAL_REGISTRY_SMD_COMMON_RUNTIME_RESULT.md)를 따른다.
> - 아래 V1의 수치와 검증은 완료 이력으로 보존한다. 이후 G00~G07이 최신 실행 계획이며,
>   V1 당시의 V2~V6 초안보다 우선한다.

> **[2026-08-23 사용자 정지 게이트]** 이번 PR은 전수 Program ledger 정본화와 기존
> S6/M3/D14의 class-neutral ID 이전까지만 병합한다. 169 SOURCE_EXACT Program backend,
> packet ABI 확장, Adapter 추가와 occurrence Binding은 여기서 시작하지 않는다. 다른 세션이
> 최신 `main`에서 4캐릭터 V1 실행 파일을 한 호흡으로 만들고 사용자가 실제 화면을 판정한 뒤,
> 그 서면 관찰을 입력으로 G01 이후 우선순위와 fidelity 경계를 다시 연다.

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

스킬별 수는 같은 skillId의 baseline/candidate/unified 문서를 합치면 중복된다. 최신 정본은
`effect-tuple-cohort-inventory.v1.json`의 **416 authored 문서 / 7,566 occurrence**다. Product는
catalog와 실제 consumer union을 exact join한 **4캐릭터 1,885 + Valtan 669 = 2,554 occurrence**다.
authored 7,566과 Product 2,554를 섞거나, 문서 수를 occurrence 수로 부르지 않는다.

Program 구현 분모도 occurrence 수와 다르다. Product translated exact 후보를 safe candidate identity로
중복 제거하면 `111`, 기존 typed runtime Program identity는 `17`, 번역 corpus는 `169`다. 여기에 아직
번역되지 않은 Product exact DXBC identity `1`개(4 occurrence)가 별도 blocker로 존재한다. 앞의 세 수는
각각 “Product에서 번역까지 닫힌 exact 계산식”, “이미 typed packet이 있는 프로그램”, “보유한 family
DXBC 번역 전체”이므로 단순 합산하지 않는다. v1 ledger의 identity는 typed면 `(backend, opcode)`,
literal이면 `dxbcSha256`이다. 서로 다른 DXBC를 정규화 식이 비슷하다는 이유로 먼저 합치지 않는다.

## 2. 등급 사다리와 이 계획이 옮기려는 것

```text
SOURCE_EXACT          DXBC를 번역한 식을 실제 렌더러가 실행한다
BOUNDED_TRANSLATED    파라미터 이름·그룹·기본값으로 재구성한 식        <- 지금 대부분
PROJECT_RECONSTRUCTED 증거 없이 프로젝트 판단으로 만든 식
```

SOURCE_EXACT occurrence 하나를 닫으려면 다음 일곱 축이 전부 있어야 한다. DXBC는 그중
Program 축 하나다.

```text
1. family join key      element -> parent material
2. cooked program       parent -> permutation -> DXBC
3. 번역과 수치 A/B      DXBC -> HLSL, 원본과 오차 0
4. named mapping        cb0[i].w -> 저작자 파라미터 이름, 관찰된 t/s register
5. runtime ABI packet   occurrence value, texture, sampler, DynamicParameter를 실제로 채운다
6. renderer adapter     carrier/VF/pass/MRT로 program을 dispatch한다
7. composition/product  timing/transform과 사용자 A/B 뒤 실제 cue로 admission한다
```

registry에서 자동 Binding을 발급하는 최소식은 다음으로 고정한다.

```text
ProgramClosed(o)     = exact equation + immutable provenance + allocated (backend, opcode)
LayoutClosed(o)      = CB/SRV/sampler/stage-input/output ABI가 typed packet으로 표현 가능
DescriptorClosed(o)  = texture/value/sampler 전부 source-backed이고 asset이 존재
AdapterClosed(o)     = carrier/VF/pass/state/MRT/scene/WPO가 compiled allowlist에 존재
CompositionClosed(o) = Product cue가 exact occurrence identity와 timing/transform을 소유

BindingEligible(o) = ProgramClosed(o)
                  && LayoutClosed(o)
                  && DescriptorClosed(o)
                  && AdapterClosed(o)
                  && CompositionClosed(o)
```

하나라도 거짓이면 행을 삭제하거나 S6/M3/D14로 근사하지 않고 terminal blocker를 남긴다. 자동
closure와 사용자의 visual approval도 분리한다. 즉 `BindingEligible=true`는 실행 가능한 기술 상태이지
색·크기·방향·UV·수명·합성의 사용자 `visual PASS`가 아니다.

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

## 4. 최신 분모, 우선순위와 세션 소유권

### 4.1 구현 분모

| 범위 | occurrence | 이 계획에서의 의미 |
|---|---:|---|
| 도화가 F | 17 | `effect.artist.skill.31470.unified`, typed runtime Program/Layout 대조군 |
| 대표 4스킬 Product slice | 131 | 다른 occurrence-ledger 세션이 stable row identity로 넘기는 첫 소비 범위 |
| 4캐릭터 Product | 1,885 | Artist 377 / DimensionMaster 270 / LanceMaster 775 / Warlord 463 |
| Valtan Product | 669 | 현재 cue/catalog가 소비하는 전수 occurrence |
| 전체 Product | 2,554 | 4캐릭터 1,885 + Valtan 669 |
| 전체 authored | 7,566 | 416문서의 분석 분모. Product rollout과 동일한 메모리 분모가 아님 |

대표 4스킬의 `131`은 **occurrence 행 수**다. PR #166의 historical G00도 LanceMaster
34110/34150에서 exact Program evidence 27+104=`131`을 남겼지만, 같은 숫자라는 이유로 새 4스킬
manifest의 행 identity를 추정하지 않는다. 다른 세션이 넘긴 `(effectAssetId, elementId)`와
`effect-tuple-cohort-inventory.v1.json`의 exact join으로만 소비 범위를 확정한다.

Program-level 분모는 다음처럼 따로 고정한다.

```text
Product occurrence-exact candidate identity   111
Product typed runtime candidate identity        17
family translated DXBC/HLSLI corpus            169
```

`111 + 17 = 128개 새 opcode`라고 미리 결론내리지 않는다. 같은 Program인지 판단하는 v1 safe key는
class/skill/family 이름이 아니라 typed `(backend, opcode)` 또는 literal `dxbcSha256`다.
169 corpus는 Product에 아직 안 쓰이는 식까지 포함하고, exact 111은 Product occurrence에서 exact로
지목된 식만 포함한다. `Data/Effects/CookedShaders`의 tracked extra exact-variant canary 2개는 별도
receipt 소유이며 169 family corpus에 더하지 않는다.

우선순위는 `S6/M3/D14 회귀 control -> 도화가 F 17 -> 대표 4스킬 131 -> Valtan 669 ->
전체 Product 2,554 -> authored 7,566 분석`이다. 뒤 단계의 행을 먼저 구현할 수는 있지만 앞 단계의
closure와 회귀 gate를 건너뛰어 Product 완료로 기록하지 않는다.

기존 milestone 이름은 다음처럼 최신화한다.

```text
V4 registry 최소 구현       PR #168에서 완료
V5 capability/runtime 일반화 PR #172/#175 control 완료, 남은 범위는 G00~G05
V6 Product admission         occurrence-ledger handoff 뒤 G06~G07
```

### 4.2 public handoff와 수정 경계

이 세션은 다음을 소유한다.

- 169 equation census, class-neutral Program ID와 backend-local opcode 배정
- Program별 HLSLI/HLSL materialization과 compiled allowlist
- Layout packet 표현력, compiled Adapter capability, registry builder/runtime/harness
- 다른 세션이 소비할 `programId/layoutId/adapterId`와 blocker vocabulary

다른 두 세션은 다음을 소유한다.

- 4스킬과 Valtan occurrence ledger의 stable row, Product cue와 Composition
- occurrence별 texture/value/sampler를 담는 Descriptor
- `Data/Effects/MaterialPrograms/Fragments/*.material-program-fragment.v1.json`의 Descriptor/Binding
- 막힌 행의 source evidence와 terminal blocker

즉 이 세션은 669행이나 131행을 다시 분류한 임시 ledger를 만들지 않는다. 다른 세션도
class/skill 전용 Program, shader path, pass를 fragment에서 발명하지 않는다. PR #175의 base는 현재
계약대로 compiled Adapter만 소유한다. 공용 Program/Layout은 이 세션의 class-neutral program-library
fragment에 한 번만 있고 occurrence domain fragment에는 Descriptor/Binding만 둔다.

## 5. `.hlsli`, `.hlsl`, DXBC와 registry의 역할

| 산출물 | 역할 | 금지 |
|---|---|---|
| `Data/Effects/CookedShaders/*.dxbc` | immutable cooked equation oracle이자 SOURCE_EXACT pixel-shader payload | hash 검증 없이 로드, 임의 permutation 선택, runtime 재컴파일 |
| `Data/Effects/TranslatedShaders/*.hlsli` | DXBC instruction을 literal하게 옮긴 사람이 읽는 식과 numeric A/B oracle | 번역 재컴파일 결과를 곧바로 원본 DXBC와 동일한 SOURCE_EXACT로 표기 |
| `Client/Bin/ShaderFiles/**/*.hlsl` | carrier/VF input bridge, technique/pass/MRT/state와 translated fallback entrypoint | 169 Program `switch(opcode)` mega-shader, exact pixel 식을 adapter마다 재컴파일 |
| `effect-material-program-registry.v1.json` | compiled Adapter ID의 integration-owned base | base에 domain Program/Layout/Descriptor/Binding을 직접 추가 |
| program-library fragment | class-neutral Program/Layout와 append-only allocation | class/skill 이름이나 executable shader path를 저작 |
| occurrence domain fragment | occurrence Descriptor/Binding | Program/opcode를 domain-local로 재발급 |

SOURCE_EXACT 주경로는 번역 HLSLI를 다시 컴파일하는 것이 아니다. Program ID를 원본 full DXBC SHA-256에
content-address하고, publisher가 exact bytes를 보존하며, 기존 exact-preview가 이미 증명한
`ID3D11Device::CreatePixelShader` seam을 registry Product 경로로 일반화한다. 같은 pixel DXBC는 Adapter가
달라도 Program ID와 pixel shader object를 재사용한다. carrier/native input signature를 맞추는 bridge와
pass/state/MRT만 Adapter-side HLSL permutation으로 분리한다.

번역 HLSLI를 include한 HLSL wrapper가 필요한 경우에는 한 wrapper가 HLSLI 하나만 소비하게 한다. 그러나
그 결과는 별도 `translatedHlslPermutationV1` backend와 `BOUNDED_TRANSLATED` fidelity다. 원본 DXBC 대비
output/ABI 동등성을 별도 봉인하기 전에는 SOURCE_EXACT로 승격하지 않는다.

기존 수동 canary인 다음 파일은 generated variant가 같은 packet/pass/RT 결과를 증명할 때까지 회귀
control로 유지한다.

```text
Client/Bin/ShaderFiles/Shader_Ue3Glasshole02.hlsli
Client/Bin/ShaderFiles/Shader_VtxEffectGlasshole02.hlsl
Client/Bin/ShaderFiles/Shader_Ue3ValtanCrack01.hlsli
Client/Bin/ShaderFiles/Shader_VtxEffectUe3ValtanCrack01.hlsl
Client/Bin/ShaderFiles/Shader_Ue3ValtanDissolve01.hlsli
Client/Bin/ShaderFiles/Shader_VtxEffectUe3ValtanDissolve01.hlsl
Client/Bin/ShaderFiles/Shader_Ue3ValtanGround04.hlsli
Client/Bin/ShaderFiles/Shader_VtxEffectUe3ValtanGround04.hlsl
```

## 6. ABI와 blocker 정본

### 6.1 현재 관찰값과 runtime 수용량

named ABI 정본의 관찰 상한은 `textureSlotCount=9 / scalarLaneCount=88 /
vectorLaneCount=16`이다. 이는 각 family가 요구하는 **row 수**이며 native register 번호가 아니다.
169 번역 HLSLI 선언을 직접 집계한 현재 최고 native register는 `t8 / s8 / v8`이다.

- texture slot 9개는 `t0..t8`과 일치하지만 slot count와 register identity는 별도 필드로 보존한다.
- scalar lane 88개를 sampler `s88`이나 constant register 88개로 해석하지 않는다.
- vector lane 16개를 stage input `v16`으로 해석하지 않는다.

현재 `CEffectMaterialProgramRegistry` 상한은 texture lane `6`, scalar row `52`, vector row `3`이고
`CEffectDocumentCodec`의 authored vector 허용량은 `8`이다. row-count 상한과 native register topology를
같은 숫자로 맞추지 않는다. G02가 explicit texture/sampler register와 stage semantic translation을
검증한 뒤 필요한 versioned packet만 확장한다.

### 6.2 terminal blocker

| blocker | 닫혀야 하는 증거 |
|---|---|
| `OCCURRENCE_STATIC_PERMUTATION_NOT_EXTRACTED` | child static permutation과 exact DXBC digest |
| `TEXTURE_REGISTER_SAMPLER_TOPOLOGY_UNMATERIALIZED` | ordered SRV와 sparse sampler register mapping |
| `SCALAR_VECTOR_PACKING_UNRESOLVED` | CB lane/component와 source value의 typed packing |
| `CURRENT_PACKET_CAPACITY_EXCEEDED` | versioned packet, parser 상한과 negative harness |
| `STAGE_INPUT_SEMANTICS_UNPROVEN` | VF가 native `v#`에 공급하는 semantic/mask |
| `OUTPUT_TOPOLOGY_MRT_UNPROVEN` | `o#`와 실제 RT/MRT bind 및 format |
| `SCENE_INPUTS_UNPROVEN` | scene color/depth SRV, projection/reconstruction 상수 |
| `WPO_VERTEX_PROGRAM_UNPROVEN` | pixel Program과 별도의 exact/typed vertex displacement path |
| `SAMPLER_STATE_UNPROVEN` | source filter/address/mip/bias와 compiled sampler receipt |
| `SOURCE_VALUE_REPLAY_UNPROVEN` | source-backed texture/scalar/vector 값과 asset 존재 |
| `COMPILED_DRAW_DISPATCH_UNPROVEN` | compiled Adapter allowlist와 issued draw receipt |
| `PRODUCT_CONSUMER_ABSENT` | exact Product cue/Composition join |

MRT, WPO, scene input, additive는 Program 식만 번역했다고 닫히지 않는다. 특히 WPO는 pixel DXBC의
출력이 아니므로 vertex program을 회수하거나 현재 typed vertex equation을 별도 capability로 증명한다.
Light/ScreenPost도 Sprite quad fallback으로 바꾸지 않고 presentation 전용 경계로 남긴다.

## G00. Program conquest ledger와 class-neutral allocation queue

### 목표와 파일

tuple cohort inventory를 in-process로 다시 빌드해 Program identity를 dedupe한다. JSON은 blocker와
consumer를 독립 검증할 수 있도록 7,566 occurrence의 최소 projection을 함께 보존하고, CSV는
Program-level review surface만 제공한다. 따라서 원본 occurrence payload 전체를 복제하지는 않지만
`occurrenceId -> Program/blocker/denominator` projection은 의도적으로 self-contained다.

```text
입력
  Tools/EffectPipeline/build_effect_tuple_cohort_inventory.py의 in-process 결과
  Data/Effects/Contracts/effect-tuple-cohort-inventory.v1.json (live 결과와 byte/hash current 검사)
  Data/Effects/Contracts/effect-family-hlsl-translations.v1.json
  Data/Effects/MaterialPrograms/effect-material-program-registry.v1.json
  Data/Effects/MaterialPrograms/Fragments/*.material-program-fragment.v1.json

추가
  Tools/EffectPipeline/build_effect_material_program_conquest_ledger.py
  Tools/EffectPipeline/test_build_effect_material_program_conquest_ledger.py
  Data/Effects/Contracts/effect-material-program-conquest-ledger.v1.json
  Data/Effects/Contracts/effect-material-program-conquest-ledger.v1.csv
```

JSON은 class-neutral Program-level 정본이고 CSV는 같은 내용을 사람이 검토하기 위한 deterministic
derived view다. 별도 JSON Schema를 복제하지 않고 builder의 fail-closed validator가 두 산출물을 함께
검사한다. 각 row는 typed `(backend,opcode)` 또는 literal `dxbcSha256` candidate identity, 원본
DXBC/HLSLI hash, authored/Product/4-character/Valtan consumer count, axis status, public registry allocation
state와 blocker를 보존한다. occurrence 최소 projection은 JSON에 보존하되 CSV에는 반복하지 않는다.

### ID와 allocator 불변식

```text
typed programId = effect.program.<backend-slug>.opcode-<decimal>.v<semantic-version>
exact programId = effect.program.ue3.ps.{64-lowercase-hex-dxbc-sha256}.v1
allocation key = (backend, opcode)
source identity = typed (backend, opcode) | literal dxbcSha256
optional proven equivalence = (normalizedEquationSha256, nativeOutputSignature)
```

allocation 전 safe source identity는 typed `(backend, opcode)`, literal `dxbcSha256`다. 위
`normalizedEquationSha256`은 향후 structural equivalence verifier가 별도 증명했을 때만 dedupe 보조키로
쓴다. v1 allocator는 서로 다른 DXBC digest를 자동 병합하지 않는다.

backend 역할도 고정한다.

```text
runtimeMaterialV2       기존 17 typed/reconstructed evaluator opcode
localDecal              현재 compiled LocalDecal packet/program
ue3CookedPixelShaderV1  content-addressed cooked DXBC를 raw CreatePixelShader로 여는 SOURCE_EXACT backend
translatedHlslPermutationV1  translated HLSLI를 별도 컴파일하는 non-exact fallback backend
```

`ue3CookedPixelShaderV1`의 opcode는 HLSL 내부 switch index가 아니다. full SHA Program ID가 immutable
pixel bytecode identity를 소유하고, append-only opcode는 registry packet의 작은 stable selector다. C++
compiled catalog가 `(backend, opcode, full DXBC hash)`를 sealed `.dxbc`에 exact-match하고 Layout/Adapter
compatibility는 별도 tuple로 검증한다. 새 backend token은 `EFFECT_MATERIAL_EXECUTION_BACKEND`, codec,
schema, publisher와 negative harness가 같은 G02/G03 변경 단위로 연다.

- Program ID에는 `artist`, `valtan`, class, skillId, filename, occurrence ID를 넣지 않는다.
- opcode namespace는 backend-local이다. `runtimeMaterialV2/14`와 `localDecal/14`는 다른 키다.
- 한 identity가 이미 배정됐으면 기존 ID/opcode를 재사용한다. equation hash나 output semantic이 바뀌면
  기존 opcode 의미를 바꾸지 않고 새 version/opcode를 append한다.
- 삭제는 번호 재사용이 아니라 tombstone이다. 병렬 세션은 임시 번호를 발급하지 않고
  class-neutral program-library fragment에 merge된 allocation만 public ID로 소비한다.
- PR #175가 연 S6/M3/D14 Program/Layout ID에는 `artist-f` naming debt가 있었다. 이번 정지-gate PR이
  이를 class-neutral ID로 atomic 이전한다. `effect.descriptor.artist-f.*`는 occurrence value owner이므로
  의도된 domain ID다.
- S6/M3/D14는 golden tuple이지 각 carrier의 기본식이 아니다. 같은 carrier라도 equation hash가 다르면
  새 Program이 필요하다.

### 데이터 흐름과 종료 증거

```text
build_effect_tuple_cohort_inventory.build_inventory()
-> authored 7,566 / Product 2,554 / four-character 1,885 / Valtan 669 projection
-> typed (backend,opcode) / literal DXBC / evidence-open source-class 분리
-> family HLSLI corpus 169 exact-set join
-> 기존 S6/M3/D14 allocation join, literal 169는 OPCODE_UNALLOCATED queue
-> deterministic JSON authority + derived CSV + self hash
```

종료 조건은 Product `2,554`, four-character `1,885`, Valtan `669`, translated exact Product identity `111`,
typed Product identity `17`, translated literal corpus `169`, public registry allocation `3`이 모두 재현되고
literal 169가 누락 없이 `OPCODE_UNALLOCATED` queue에 들어가는 것이다. 별도로 untranslated exact identity
`1`개가 DimensionMaster Product `4` occurrence를 소유하며 `P2_TRANSLATE_EXACT_DXBC` blocker로 남아야 한다.
대표 slice `131`과 Artist F `17`은 다른 세션 handoff와 exact join해 별도 canary로 검산한다. fixture는
typed identity mutation, literal hash collision, class/skill 이름을 넣은 public Program/Layout ID, stale
input hash, JSON/CSV divergence와 중간 write fault를 거부해야 한다.

## G01. 169 sealed DXBC Program materialization과 bridge codegen

### 파일

```text
수정
  Tools/EffectPipeline/translate_ue3_dxbc_to_hlsl.py
  Tools/EffectPipeline/verify_effect_family_hlsl_translations.py
  Tools/EffectPipeline/build_effect_material_program_registry.py
  Tools/EffectPipeline/test_build_effect_material_program_registry.py
  Data/Effects/MaterialPrograms/Fragments/artist-f-golden.material-program-fragment.v1.json

추가
  Data/Effects/MaterialPrograms/Fragments/class-neutral-program-library.material-program-fragment.v1.json
  Tools/EffectPipeline/build_effect_material_program_shaders.py
  Tools/EffectPipeline/test_build_effect_material_program_shaders.py
  Client/Bin/DataFiles/Effect/MaterialPrograms/Shaders/*.dxbc
  Client/Bin/ShaderFiles/Generated/EffectMaterialBridges/*.hlsl
  Client/Private/Effect_MaterialProgramCompiledCatalog.generated.inl
```

publisher는 `Data/Effects/CookedShaders/<sha>.dxbc`를 byte-for-byte 검증해 위 runtime DataFiles 경로로
stage한다. generated C++ table은 full SHA Program ID, backend-local opcode, expected byte count/hash와
deterministic runtime path를 연결하지만 domain 이름이나 임의 shader path를 JSON에서 읽지 않는다.
`.inl`은 `Client/Private/Effect_MaterialProgramRegistry.cpp`만 include하고 수동 편집하지 않는다.
generated bridge HLSL은 pixel equation을 포함하지 않고 carrier/native input signature와 pass/output state만
연결한다. 같은 pixel DXBC를 Adapter별로 복사하거나 다시 컴파일하지 않는다.

이번 정지-gate PR은 PR #175의 S6/M3/D14 Program/Layout ID만 Artist fragment 안에서 class-neutral로
atomic 이전한다. G01에서 program-library fragment를 처음 publish할 때 이 공용 Program/Layout을 Artist
fragment에서 library로 이동하고, Artist Descriptor/Binding은 cross-fragment reference로 같은 ID를 계속
소비한다. 어느 한쪽만 stage된 registry는 duplicate 또는 dangling ID로 실패해야 한다.

169개 식 전부는 sealed pixel Program과 translated replay evidence를 함께 보존한다. Product runtime
bridge는 Layout과 Adapter가 닫힌 식만 생성한다. 따라서 상태를 다음처럼 나눈다.

```text
SEALED_DXBC_PROGRAM_READY       exact bytes/hash와 CreatePixelShader 대상이 닫힘
TRANSLATED_EQUATION_REPLAY_EXACT HLSLI numeric verifier가 원본 DXBC와 일치
RUNTIME_BRIDGE_COMPILED         native input signature × compiled Adapter bridge가 닫힘
RUNTIME_BRIDGE_BLOCKED          Program은 있으나 Layout/Adapter blocker가 구조화됨
```

`SEALED_DXBC_PROGRAM_READY`나 `TRANSLATED_EQUATION_REPLAY_EXACT`만으로 Product-ready라고 부르지 않는다.
반대로 ABI가 막혔다고 equation 구현을 복사한 family-specific HLSL로 우회하지 않는다.

### mega-switch 금지

- `Shader_EffectUe3MaterialFamilies.hlsli`나 C++에 169-case `switch(opcode)`를 추가하지 않는다.
- 169 HLSLI를 한 translation unit에 include하지 않는다.
- skill/family마다 `Shader_VtxEffect<Family>.hlsl`을 손으로 복사하지 않는다.
- codegen은 같은 native-input/Adapter bridge template을 재사용하되 pixel shader는 full SHA별 sealed DXBC다.
- Adapter가 달라져도 DXBC가 같으면 Program ID, bytecode와 pixel shader object를 중복 생성하지 않는다.
- 실제 Product에서 reachable하지 않은 variant는 Client startup prewarm 목록에 넣지 않는다.

종료 조건은 corpus 169 전부가 `SEALED_DXBC_PROGRAM_READY`, full SHA Program ID와 append-only allocation을
가지고, runtime target의 bridge가 FX compile/CreatePixelShader/generated-table exact-set 검사를 통과하는
것이다. 기존 845회 translated numeric A/B의 `NUMERIC_MISMATCH=0`도 유지한다. translated fallback을
생성했다면 backend와 fidelity가 exact path와 분리됐는지도 검사한다.

## G02. Layout ABI packet 확장

### 파일과 책임

```text
수정
  Client/Public/Effect_AuthoringDocument.h
  Client/Private/Effect_DocumentCodec.cpp
  Client/Public/Effect_MaterialProgramRegistry.h
  Client/Private/Effect_MaterialProgramRegistry.cpp
  Tools/EffectPipeline/Schemas/lostark.effect-material-program-registry.schema.json
  Tools/EffectPipeline/build_effect_material_program_registry.py
  Tools/EffectPipeline/test_build_effect_material_program_registry.py
  Tools/EffectRenderContractHarness/Private/EffectRenderContractHarness.cpp
```

Layout은 ordered value가 아니라 register/semantic shape를 소유한다. texture lane은 explicit
`textureRegister`와 `samplerRegister`, CB row는 register/component/mask, stage input은 native `v#`와
carrier semantic, output은 `o#`와 RT target을 기록한다. Descriptor asset/value는 Layout에 넣지 않는다.

현재 6/52/3 상한 안에 드는 Layout은 기존 packet version을 유지한다. 넘는 식은 G00 native receipt를
근거로 versioned packet을 추가하고 codec/registry/harness를 같은 변경 단위에서 확장한다. `s88`을
dense sampler array로 만들거나 `v16`을 Descriptor vector count로 오독하는 확장은 금지한다.

종료 조건은 S6/M3/D14 packet snapshot bit-exact 회귀, t9/s88/v16 경계 fixture, unknown/sparse duplicate
register, count overflow, mask mismatch, non-finite value, missing sampler, wrong color space와 rollback 검사가
모두 닫히는 것이다. exact native wire만 있고 source value가 없는 행은 계속 Layout evidence일 뿐
Descriptor closure가 아니다.

## G03. class-neutral Program runtime와 compiled dispatch

### 파일과 호출 흐름

```text
수정
  Data/Effects/MaterialPrograms/effect-material-program-registry.v1.json
  Data/Effects/MaterialPrograms/Fragments/class-neutral-program-library.material-program-fragment.v1.json
  Data/Effects/MaterialPrograms/Fragments/artist-f-golden.material-program-fragment.v1.json
  Client/Public/Effect_MaterialProgramRegistry.h
  Client/Private/Effect_MaterialProgramRegistry.cpp
  Client/Private/Effect_Catalog.cpp
  Client/Public/Effect_MaterialTemplate.h
  Client/Private/Effect_DocumentRenderer.cpp
  Client/Default/Client.vcxproj
  Client/Default/Client.vcxproj.filters
```

G01에서 class-neutral program-library fragment로 옮긴 S6/M3/D14와 Artist Descriptor/Binding의
cross-fragment join을 그대로 유지한다. compiled Adapter base, runtime 결과와 binding identity는 바꾸지
않는다. 새 Program은 G00 allocation queue에서 append-only opcode를 발급받아 program-library fragment와
compiled catalog entry에 동시에 들어갔을 때만 load된다.

현재 `CEffectDocumentRenderer::Install_AuthoringExactPreviewVariants` 안의 reflection/hash 검증과
`CreatePixelShader`는 sealed cooked PS가 실행 가능하다는 기존 증거다. 이 설치 코어를 registry와
authoring preview가 함께 쓰는 private primitive로 분리하되, Product는 source-material lookup이나
`Set_AuthoringExactPreviewExecutionEnabled` toggle을 소비하지 않고 Program ID/Binding으로 선택한다.
authoring 전용 두 번째 draw path를 Product에 그대로 복사하지 않는다.

```text
compiled Adapter base + class-neutral Program/Layout library fragment
-> occurrence-domain Descriptor/Binding fragments
-> build_effect_material_program_registry.py strict merge
-> Program equation/provenance와 compiled catalog exact join
-> Layout ABI receipt validate
-> immutable generation stage
-> CEffectCatalog atomic commit
-> CEffectDocumentRenderer가 기존 EFFECT_MATERIAL_EXECUTION_DESC로 draw
```

`Effect_MaterialTemplate.h`의 기존 family-specific resolver는 아직 안 옮긴 행의 V0 fallback으로만 둔다.
새 family if-chain을 추가하지 않고, sealed Program이 동일한 actual draw를 증명한 resolver 블록만
해당 vertical slice에서 제거한다. 모르는 backend/opcode/Program ID를 default shader로 보내지 않는다.

종료 조건은 같은 Program을 Artist, 4스킬, Valtan Descriptor가 공유해도 shader/dispatch가 하나이고,
domain 이름 없이 resolve되는 것이다. duplicate Program semantic, compiled entry 누락, shader path drift,
wrong backend/opcode, partial generation load는 이전 generation 보존으로 실패해야 한다.

## G04. Adapter capability 확장

### capability 순서

```text
S/M/D existing standard RT0 controls
-> Sprite/Mesh additive state variants
-> LocalDecal scene-depth variants
-> Ribbon / AnimTrail topology
-> SceneColor / distortion MRT
-> WPO vertex variants
-> Light / ScreenPost presentation adapters
```

Adapter ID는 `carrier + VF + output topology + blend/depth/raster + scene inputs + vertex behavior`의 compiled
receipt다. Program 식이 같아도 alpha/additive, RT0/MRT, static mesh/mesh particle, Sprite/Ribbon,
pixel-only/WPO가 다르면 Adapter를 분리한다. 반대로 class/skill만 다르면 Adapter를 늘리지 않는다.

### 파일

```text
수정
  Client/Public/Effect_DocumentRenderer.h
  Client/Private/Effect_DocumentRenderer.cpp
  Client/Public/Effect_MaterialProgramRegistry.h
  Client/Private/Effect_MaterialProgramRegistry.cpp
  Client/Bin/ShaderFiles/Shader_VtxEffectParticle.hlsl
  Client/Bin/ShaderFiles/Shader_VtxEffectTrail.hlsl
  Client/Bin/ShaderFiles/Shader_VtxEffectDecal.hlsl
  Tools/EffectRenderContractHarness/Private/EffectRenderContractHarness.cpp
```

실제 필요 variant는 G00 conquest ledger의 Product reachable tuple만 추가한다. bridge identity는
`Adapter + native pixel input signature`이며 같은 cooked PS Program을 Adapter별로 복제하지 않는다.
MRT는 active render target count/format과 o-register, scene path는 실제 SRV slot, additive는 blend state,
WPO는 vertex output/world hash까지 harness로
검사한다. JSON이 pass/state를 정하지 못하며 compiled Adapter description과 불일치하면 catalog 전체 load를
거부한다.

G04 완료는 “모든 carrier가 S/M/D 중 하나”가 아니다. 각 ledger row가 compiled Adapter 또는 위 §6.2의
terminal capability blocker 중 하나를 갖는 상태다. Light 1행은 pixel material 분모에서 제외해
presentation adapter가 생기기 전까지 `NOT_APPLICABLE_PRESENTATION`으로 남긴다.

## G05. registry publish, capacity와 authority 전환

### 정본 경로

```text
source base
  Data/Effects/MaterialPrograms/effect-material-program-registry.v1.json
source fragments
  Data/Effects/MaterialPrograms/Fragments/*.material-program-fragment.v1.json
schema/builder
  Tools/EffectPipeline/Schemas/lostark.effect-material-program-registry.schema.json
  Tools/EffectPipeline/build_effect_material_program_registry.py
runtime output
  Client/Bin/DataFiles/Effect/EffectCatalog.runtime.json.materialPrograms
publisher
  Tools/EffectPipeline/Publish-Effects.ps1
```

base는 PR #175 계약대로 compiled Adapter public catalog만 소유한다. class-neutral program-library
fragment는 Program/Layout과 allocation을, occurrence-domain fragment는 Descriptor/Binding만 소유한다.
publisher는 base와 모든 fragment를 `parse -> validate -> stage -> atomic commit`하고 실패 시 기존 runtime
catalog를 유지한다. 새 fragment와 generated shader/`.inl`은 `Client/Default/Client.vcxproj`와
`.filters`에 정확히 한 번 등록하고 `Sync-EffectDataProject.ps1 -Check`의 기존 광범위한 baseline 누락과
이번 변경을 섞지 않는다.

현재 public cap은 Program `4096`, Layout `4096`, Descriptor `4096`, Binding `65536`이다. Product
2,554행은 occurrence당 Descriptor 하나여도 4096 안이지만 authored 7,566행은 그렇지 않다. 그러므로
이번 Product phase는 2,554를 runtime 상한으로 고정하고 authored 7,566은 offline census로 유지한다.
authored 전체를 runtime으로 열기 전에는 다음 중 하나를 별도 public 계약으로 닫는다.

1. descriptor semantic dedupe가 실제로 4096 이하임을 receipt로 증명한다.
2. versioned cap/partition을 parser, memory budget, malicious-count fixture와 함께 확장한다.

cap을 조용히 올리거나 4,096번째 뒤를 잘라내지 않는다.

PR #175의 세 Binding은 계속 `INLINE_MIRROR_REQUIRED`다. inline packet이 없는 새 occurrence를 위해
`REGISTRY_AUTHORITATIVE`를 여는 일은 source-backed Descriptor, old/new packet comparison, failure rollback,
actual draw harness가 한 vertical slice로 닫힌 뒤에만 한다. registry 확장 자체가 authority 전환을
강제하지 않는다.

## G06. occurrence binding과 Product slice

### 세션 간 handoff row

다른 세션은 다음 필드를 가진 row를 넘긴다.

```text
effectAssetId + elementId
programEvidenceId + equationSha256
layoutEvidenceId + native register/stage receipt
descriptor source values + resource asset IDs
adapter candidate + carrier/VF/pass/state/MRT/scene/WPO evidence
compositionVariantId + Product cue identity
terminal blockers
```

이 세션은 row를 G00 census와 join해 `programId/layoutId/adapterId` 또는 정확한 capability blocker를
돌려준다. occurrence 세션은 그 결과로 다음 domain fragment를 소유한다.

```text
Data/Effects/MaterialPrograms/Fragments/artist-f-golden.material-program-fragment.v1.json
Data/Effects/MaterialPrograms/Fragments/representative-four-skills.material-program-fragment.v1.json
Data/Effects/MaterialPrograms/Fragments/valtan-product.material-program-fragment.v1.json
```

fragment filename은 public merge handoff 이름이며 Program/opcode allocator가 아니다. 아직 merge되지 않은
fragment를 다른 세션이 존재한다고 추정해 소비하지 않는다.

### Valtan 669 정본화

Valtan Product 분모는 `effect-tuple-cohort-inventory.v1.json`의 productConsumed Valtan 669행이다.

| 축 | 현재 실측 |
|---|---:|
| carrier | Sprite 458 / Mesh 174 / Decal 33 / AnimTrail 3 / Light 1 |
| Program | exact translated 139 / 14 Program candidates, family representative 252, bounded 1, no evidence 276, presentation 1 |
| Layout | typed packet 0, native within cap 249, native extension 141, source names 1, unresolved 277, presentation 1 |
| Descriptor | source values unpacked 8 / resource-only 660 / presentation 1 |

Product lineage는 CarrierV1 657 + 보호된 Whirlwind 9 + 외부 owner Sky Axe 3 = 669다. 이 669를
먼저 정본 ledger로 유지하고 3패턴 임시 ledger를 만들지 않는다.

`Data/Effects/Imported/Valtan/CarrierV1/Valtan.carrier-v1-materialization-receipt.v1.json`의 두 배열은
grain이 다르다.

```text
reviewedProjectionLedger
  1,577 composite rows
  key = (occurrenceFullKey, carrierKey)
  executable core 660 / blocked 917
  unique occurrenceFullKey 247

reviewedSourceOnlyOccurrences
  carrier가 없는 별도 source occurrence 197
```

따라서 `1,577 + 197 = 1,774 occurrences`라고 쓰지 않는다. 하나의 분석 표로 합쳐야 한다면
`rowKind=PROJECTION|SOURCE_ONLY`를 두고 각 grain의 key와 분모를 따로 보존한다. 669 Product ledger와도
행 수를 직접 비교하지 않는다.

### 휠윈드·돌진·도넛 첫 slice

| 사용자 용어 | stable pattern | 현재 Product occurrence | 판정 |
|---|---|---:|---|
| 휠윈드 | `VALTAN_WHIRLWIND` | 12 = active 9 + recovery 3 | 669 ledger에서 exact join |
| 도넛 | `VALTAN_MAGIC_CHOICE` | 19 = inner 4 + outer 2 + recovery 13 | 669 ledger에서 exact join |
| 포탈 돌진 | `VALTAN_PORTAL_RUSH` | 28 | Dash와 다른 pattern |
| 돌진 | `VALTAN_DASH_CHARGE` | **0** | `EXACT_REVIEWED_OWNER_ABSENT` blocker |

`VALTAN_DASH_CHARGE`는 현재 Product 669, `reviewedProjectionLedger`,
`reviewedSourceOnlyOccurrences`, current cue가 모두 0행이다. 과거 cue는
`RETIRED_NO_EXACT_REVIEWED_CARRIER_OWNER`, authored active/recovery/windup은 비어 있고 part-break 18행은
catalog-unreachable이다. 사용자가 말한 “돌진”을 `PORTAL_RUSH`로 자동 치환하지 않는다. target이
Dash라면 이번 slice는 구현 0행과 blocker가 정답이고, target이 Portal Rush라면 stable ID를 명시한
28행 slice로 진행한다.

S6/M3/D14 Binding도 carrier 종류만 보고 발급하지 않는다. Program equation, Layout packet,
Descriptor values, compiled Adapter와 Composition이 모두 golden tuple과 exact할 때만 재사용한다. 현재
Valtan 669의 typed Layout은 0이고 Descriptor 660행이 resource-only이므로, 139 exact Program 행도 G02와
Descriptor session이 닫히기 전에는 Binding 후보일 뿐이다.

## G07. 자동 검증, build와 사용자 화면 경계

### 자동 검증

각 G는 focused test를 먼저 통과시키고 통합 checkpoint에서 다음을 실행한다.

```powershell
python Tools/EffectPipeline/build_effect_material_program_conquest_ledger.py --check
python Tools/EffectPipeline/test_build_effect_material_program_conquest_ledger.py
python Tools/EffectPipeline/verify_effect_family_hlsl_translations.py
python Tools/EffectPipeline/test_build_effect_material_program_shaders.py
python Tools/EffectPipeline/test_build_effect_material_program_registry.py
powershell -ExecutionPolicy Bypass -File Tools/EffectPipeline/Publish-Effects.ps1 -Mode Validate

powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release
$effectRuntimeCatalog = Get-Content -LiteralPath Client/Bin/DataFiles/Effect/EffectCatalog.runtime.json -Raw -Encoding UTF8 | ConvertFrom-Json
$expectedMaterialBindingCount = @($effectRuntimeCatalog.materialPrograms.bindings).Count
powershell -ExecutionPolicy Bypass -File Tools/EffectRenderContractHarness/Run-EffectRenderContractHarness.ps1 -Configuration Debug -ExpectedBindingCount $expectedMaterialBindingCount
powershell -ExecutionPolicy Bypass -File Tools/EffectRenderContractHarness/Run-EffectRenderContractHarness.ps1 -Configuration Release -ExpectedBindingCount $expectedMaterialBindingCount

git diff --check
```

Binding count는 publisher runtime output에서 읽어 같은 검증 단위에 고정하며 숫자를 추측하지 않는다.
Engine public header가 바뀌므로 Debug/Release 각각 Engine,
`UpdateLib.bat`, Shared/NetworkProtocolHarness, Server `--contract-test`, Client와 Effect harness를 통과해야
한다. Server gameplay packet을 바꾸지 않아도 사용자가 실제 Product cue를 볼 실행 경로까지 준비하기 위해
Server+Client 정본 build 결과를 남긴다.

focused negative fixture는 최소 다음을 포함한다.

- allocation duplicate/renumber/hash mutation과 unknown backend/opcode
- stale DXBC/HLSLI/generated wrapper hash와 169 exact-set drift
- textureSlotCount 9 / scalarLaneCount 88 / vectorLaneCount 16 row boundary, native t8/s8/v8
  register topology, overflow, duplicate mask와 non-finite value
- Program/Layout/Descriptor/Adapter/Composition 중 하나씩 빠진 Binding
- wrong carrier/VF/pass/MRT/state/scene/WPO compiled tuple
- missing resource, wrong color space/sampler와 fragment duplicate
- registry/catalog parse 중간 실패와 이전 generation rollback
- Descriptor 4096/4097, Binding 65536/65537 capacity boundary
- Valtan 669, projection 1,577, source-only 197 grain 혼합과 Dash/Portal substitution 거부

### 사용자 실행 경로

에이전트는 build와 구조화된 실행 증거까지만 만들고 Client/UI를 실행·조작하거나 화면을 캡처하지 않는다.
사용자가 다음 경로로 최종 판정한다.

1. Visual Studio에서 `Framework.sln`, `x64`, 검증할 `Debug` 또는 `Release`,
   `Server + Client` launch profile을 선택하고 `Ctrl+F5`로 실행한다. Client working directory는
   `Client/Default`, Server/Client endpoint는 `127.0.0.1:7777`이다.
2. 도화가 F는 `Lobby > Character Select > Artist`, follow camera에서 `F`
   (`skillId=31470`, `필법 : 한획긋기`)를 같은 위치/방향에서 재생한다.
3. 대표 4스킬은 occurrence-ledger 세션이 넘긴 class/slot/skill ID의 exact 실행 경로를 따른다.
4. Valtan은 `F1 > Effect Tool > All Effects > Valtan > <stable pattern> > Saved Unified Effects >
   Open Saved Effect > Play Saved Effect`로 휠윈드, Magic Choice, 명시된 Dash 또는 Portal Rush를 각각 연다.
5. first pixel, 발생 시점, 위치/방향, scale/수명, UV motion, 색/coverage/noise/alpha, blend/MRT/scene 결과,
   반복·취소·재진입 잔상을 기록한다.

사용자의 서면 관찰 전 상태는 `PENDING_USER_VISUAL_GATE`다. 자동 검증 성공을 `visual PASS`,
`V1_COMPLETE` 또는 occurrence admission으로 대신 기록하지 않는다.

## 7. phase 완료 기준과 금지사항

| 상태 | 완료 조건 |
|---|---|
| `PROGRAM_LEDGER_COMPLETE` | translated exact 111 / untranslated exact 1 / typed 17 / public 3 / corpus 169 identity와 allocation queue가 deterministic |
| `EQUATION_LIBRARY_COMPLETE` | 169 식 전부 sealed DXBC Program + translated replay exact |
| `RUNTIME_CAPABILITY_READY` | 대상 sealed Program × Layout × Adapter bridge CreatePixelShader/actual draw |
| `BINDING_CANDIDATE_READY` | 다섯 closure가 모두 true, fragment publish 전 |
| `PRODUCT_AUTOMATED_COMPLETE` | publisher/harness/Debug+Release/Server+Client build와 rollback 통과 |
| `PENDING_USER_VISUAL_GATE` | 자동 완료, 사용자의 화면 판정 전 |
| `USER_VISUAL_APPROVED` | 사용자가 stable occurrence별 관찰과 결론을 서면으로 남김 |

- V1은 join key 복구 이력이다. cooked Program, runtime ABI, Adapter, Product 또는 화면을 admission하지 않는다.
- 169 DXBC/HLSLI를 가지고 있다는 사실만으로 7,566 occurrence가 renderable한 것은 아니다.
- S6/M3/D14를 Sprite/Mesh/Decal의 보편 공식으로 쓰지 않는다.
- `Shader_EffectUe3MaterialFamilies.hlsli`, C++ 또는 JSON에 169-case mega-switch를 만들지 않는다.
- translated HLSLI 재컴파일 결과를 원본 cooked DXBC와 같은 SOURCE_EXACT Program으로 표기하지 않는다.
- class/skill/occurrence 이름으로 Program/opcode를 발급하거나 세션마다 번호를 다시 매기지 않는다.
- named ABI, exact program, carrier 중 하나만 보고 Descriptor/Binding을 자동 생성하지 않는다.
- MRT, scene input, additive, WPO, Ribbon과 presentation blocker를 alpha RT0 Sprite로 위장하지 않는다.
- Valtan `DASH_CHARGE` 0행을 `PORTAL_RUSH` 28행으로 바꾸지 않는다.
- 1,577 projection row와 197 source-only row를 occurrence 1,774개로 합산하지 않는다.
- Product 2,554가 현재 Descriptor cap 4,096 안이라는 사실로 authored 7,566 rollout을 승인하지 않는다.
- 다른 세션의 occurrence ledger/Descriptor/Binding을 이 branch에서 다시 작성하거나 merge 전 fragment를
  public 계약으로 소비하지 않는다.
