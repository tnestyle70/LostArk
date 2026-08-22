# 2026-08-23 Effect Tuple Cohort Inventory 구현 계획

기준 branch: `codex/effect-tuple-cohort-inventory`

기준 main: `0d199aac942fe8c39ced3a6adb6fce2bee7402ed` (`PR #169` merge, `PR #168` Binding 0 포함)

상위 계획:

- [`4캐릭터·Valtan Effect V1 전체 마이그레이션 마스터 계획`](../08-22/2026-08-22_FOUR_CHARACTER_VALTAN_EFFECT_V1_FULL_MIGRATION_MASTER_PLAN.md)
- [`Effect Family Shader Inventory 구현 계획`](../08-22/2026-08-22_EFFECT_FAMILY_SHADER_INVENTORY_IMPLEMENTATION_PLAN.md)
- [`Effect Family Shader Inventory 결과`](../08-22/2026-08-22_EFFECT_FAMILY_SHADER_INVENTORY_RESULT.md)

작업 lane: `Track B — 대량 확장을 위한 evidence inventory`

실제 Pixel 연결 lane: 별도 `Track A — Artist F horizontal Sprite runtime spine`

최종 화면 판정자: 사용자

## 0. 이번 구현의 목표

이번 구현은 4캐릭터와 Valtan의 authored Effect를 전수 조사해 각 element occurrence가 다음 중
어디에서 막혀 있는지 하나의 결정적 계약으로 만든다.

```text
Occurrence
  × Program evidence
  × Layout evidence
  × Adapter evidence
  × Descriptor evidence
  × Composition reachability
  × Product/user review
```

이 계약은 화면을 그리는 runtime 구현이 아니다. Track B는
`Program × Layout identity × Adapter`의 정적 증거를 cohort로 묶어 재사용 후보와 blocker 우선순위를
만든다. 모든 cohort의 `runtimeVerified`와 `runtimeDescriptorExpansionEligible`은 false이며,
Descriptor-only 확장은 Track A의 compiled draw 증명과 후속 occurrence 검증 뒤에만 가능하다.

종료 증거는 다음 세 파일이다.

- `Tools/EffectPipeline/build_effect_tuple_cohort_inventory.py`
- `Tools/EffectPipeline/test_build_effect_tuple_cohort_inventory.py`
- `Data/Effects/Contracts/effect-tuple-cohort-inventory.v1.json`

## 1. 현재 실측

### 1.1 authored 분모

`Data/Effects/Authored/*.effect.json` 중 document `effectAssetId`가 아래 prefix인 파일을 대상으로 한다.

```text
effect.artist.
effect.dimensionmaster.
effect.lancemaster.
effect.warlord.
effect.valtan.
```

현재 exact 분모는 다음과 같다.

| domain | document | occurrence |
|---|---:|---:|
| Artist | 35 | 615 |
| DimensionMaster | 60 | 2,169 |
| LanceMaster | 102 | 2,588 |
| Warlord | 61 | 1,414 |
| Valtan | 158 | 780 |
| 합계 | 416 | 7,566 |

문서 version은 현재 `10~13`이며 filename은 전부 `<effectAssetId>.effect.json`과 일치한다. 빈 Valtan
문서 `107`개도 document 분모에 남기고, `visible=false`와 material evidence가 없는 element도 occurrence에서
제외하지 않는다.

### 1.2 Product reachability 분모

현재 Product consumer union은 다음 세 source에서 만들어진다.

- Artist, DimensionMaster, LanceMaster, Warlord `*.animevents`: 99 assets
- `Valtan.patterneffectcues.json`: 44 assets
- `BossCatalog.json`의 Valtan `combatObjectVisuals`: 2 assets

중복 제거 결과는 `145 assets / 2,554 elements`이며 현재 runtime Effect catalog와 정확히 같다.
`effect.valtan.sky-axe.active`, `effect.valtan.red-blade-wave.active`를 빠뜨리지 않는다. runtime catalog에
있다는 사실은 Product V1 admission이나 사용자 승인을 뜻하지 않는다.

### 1.3 carrier 분모

현재 C++의 coarse GPU render family와 같은 규칙으로 전수 분류한다.

| carrier | occurrence | 결정 규칙 |
|---|---:|---|
| `SPRITE` | 4,816 | sprite, meshModel 없는 particle |
| `MESH` | 2,539 | mesh, meshModel 있는 particle |
| `DECAL` | 83 | decal |
| `RIBBON` | 19 | trail |
| `PRESENTATION` | 109 | light, screenPost |

이 값은 몸의 대분류일 뿐 Vertex Factory, pass, scene input, MRT를 닫은 Adapter 증거가 아니다.
또한 source TypeData의 native carrier authority도 아니다. 현재 particle 중 TypeDataMesh이지만 meshModel이
없는 `12`행과 TypeDataRibbon이지만 current coarse bucket은 Sprite인 `10`행을 별도 source-evidence/blocker로
남긴다. coarse runtime bucket을 source-exact carrier로 승격하지 않는다.

fine renderer 검산값은 standalone mesh `1,354`, legacy standalone sprite `1,902`, mesh particle `1,197`,
sprite particle `2,892`, decal particle/local decal 합계 `83`, valid Cascade ribbon `4`, AnimTrail `3`,
legacy authored trail `12`, particle-kind/TypeDataRibbon mismatch `10`, light `50`, screen post `59`다.

### 1.4 material evidence 분모

- typed `material.execution.enabled`: 50
- `material.sourceProfile.enabled`: 2,732
- 둘 다 없는 occurrence: 4,784

현재 runtime packet 상한은 실제 codec 기준 texture lane `6`, packed scalar `52`, packed vector `3`이다.
source profile 자체 허용량과 runtime packet 수용량을 혼동하지 않는다.

위 `50 / 2,732 / 4,784`는 Descriptor 값 materialization 분모다. `sourceProfile.enabled=false`여도
raw parent/source material metadata는 Program과 Layout evidence join에 사용하고, enabled profile만
Descriptor 값을 공급한다. 따라서 `material evidence 없음`은 `Program/Layout evidence 없음`과 같은 뜻이
아니다.

## 2. 변경 파일과 책임

### 2.1 `build_effect_tuple_cohort_inventory.py`

모든 입력을 strict parse하고 identity/hash/dependency pin을 검증한 뒤 7,566개 occurrence의 여섯 축과
cohort를 계산한다. `parse -> validate -> stage -> atomic replace`를 소유하며 기존 artifact를 부분
교체하지 않는다.

재사용 가능한 공통 코드는 기존 builder에서 import하지 않고 이번 도구의 명시적 계약으로 보존한다.
대형 기존 script의 target-specific 전역 상태와 결합하지 않도록 canonical JSON, SHA-256, strict JSON,
input tracking, content-addressed variant registry만 작은 공통 의미 단위로 옮긴다.

### 2.2 `test_build_effect_tuple_cohort_inventory.py`

fixture repository를 만들어 정상 생성, stale check, identity drift, overpromotion, ambiguous composition,
transaction rollback을 실행한다. 단순 schema snapshot이 아니라 evidence가 부족한 행을 cohort로 올리지
않는 동작을 검증한다.

### 2.3 `effect-tuple-cohort-inventory.v1.json`

생성 전용 정본이다. 사람이 직접 편집하지 않는다. 입력 identity, 정책, summary, content-addressed
candidate/variant, occurrence, cohort, blocker bucket, canary와 artifact self hash를 보존한다.

### 2.4 project/filter 등록

Python/JSON/Markdown만 추가한다. C++ 파일이 없으므로 `.vcxproj`와 `.vcxproj.filters`는 변경하지 않는다.

## 3. 입력 정본

### 3.1 Program·Layout evidence

- `effect-family-shader-map-index.v1.json`
- `effect-family-cooked-pixel-shaders.v1.json`
- `effect-family-hlsl-translations.v1.json`
- `effect-family-named-abi.v1.json`
- `effect-child-parent-resolution.v1.json`
- `effect-family-shader-inventory.v1.json`
- `ue3-exact-cooked-shader-variants.v1.json`
- `character-effect-restoration-targets.v1.json`
- `Data/Effects/CookedShaders/*.{dxbc,asm}`의 exact set/hash
- `Data/Effects/TranslatedShaders/*.hlsli`의 exact set/hash

`PR #166` G00 artifact는 LanceMaster D/F와 Artist F control의 269개 source occurrence만 소유한다.
전체 7,566개 분모로 확장하거나 parent 대표 program을 다른 child에 전파하지 않고 cross-check canary로만
사용한다.

family translation/G00 set은 `169`개 program을 소유하지만 CookedShaders에는 exact cooked variant 연구가
소유하는 추가 DXBC `2`개가 있어 전체 `171`개다. 두 추가 blob을 미소유 파일로 오판하지 않고 169 family
set과 `ue3-exact-cooked-shader-variants`의 candidate ownership을 분리해 검증한다. exact variant 계약도
actual VF/pass, Product runtime, visual admission은 모두 `0`이므로 verified Adapter 증거로 사용하지 않는다.

### 3.2 authored·catalog evidence

- 선택된 `Data/Effects/Authored/*.effect.json` 416개
- `Data/Effects/EffectCatalog.json`
- `Client/Bin/DataFiles/Effect/EffectCatalog.runtime.json`

catalog row가 가리키는 published authored 문서의 canonical element identity를 검증한다. stale publish는
행을 삭제하는 이유가 아니라 Product technical blocker이며, 같은 stable ID의 다른 내용으로 승인하지
않는다.

authoring catalog의 target-prefix row는 `257`개지만 physical authored occurrence에 join되는 행은
`256 assets / 4,812 elements`다. 나머지 하나인 `effect.artist.skill.31470`은
`IMMUTABLE_RECONSTRUCTED_RUNTIME_PROGRAM`이고 authored occurrence가 아니므로 unified document에 임의
결합하지 않는다.

### 3.3 Composition evidence

- `Data/Balance/PlayerSkills.json`
- 4캐릭터 `*.skillbindings.json`
- 4캐릭터 `*.animevents`
- `Valtan.patternbindings.json`
- `Valtan.patterneffectcues.json`
- `Data/Actors/BossCatalog.json`
- `Data/Encounters/Valtan/ValtanEncounter.json`
- `Data/Animation/Reference/Valtan/Valtan.animnotify`
- `Data/Encounters/Valtan/ValtanCombatObjects.json`

하나의 Effect가 여러 cue에서 사용되면 occurrence를 복제하지 않고 `compositionVariantIds[]`로 one-to-many
관계를 보존한다. Character text cue는 line/index가 아니라 clip, startms, effect asset, anchor/follow/stop,
transform, optional orientation의 normalized semantic payload를 hash한다. skillbindings의 clip이 정확히 하나의
skillId로 resolve되고 PlayerSkills의 class/action과 맞는지도 검증한다. Valtan의 추가 세 입력은 Product ID를
늘리지 않지만 pattern action/stage/duration, clip duration, combat-object owner join을 닫는다.

## 4. stable identity와 출력 구조

### 4.1 occurrence identity

identity key는 `(effectAssetId, element.id)`다.

```text
occurrenceId
  = "occurrence."
  + sha256(canonical({effectAssetId, stableId: element.id}))
```

array index는 저장 identity가 아니다. 진단을 위한 `elementOrder`만 보존한다. 같은 문서의 duplicate
element ID와 duplicate effectAssetId는 fatal이다. filename prefix와 parsed effectAssetId prefix 중 하나만
대상인 XOR도 fatal로 처리해 target 파일이 이름 또는 내부 ID만 바꿔 scope에서 빠져나가는 것을 막는다.

### 4.2 top-level 구조

```text
schema / formatVersion / identity / inputs / policies
summary
carrierVariants
programCandidates / programEvidence
layoutCandidates / layoutEvidence
adapterCandidates
descriptorVariants
compositionVariants
occurrences
cohorts / blockerBuckets / canaries
transaction / artifactSha256
```

candidate와 variant ID는 canonical payload content hash다. Descriptor와 Composition은 tuple cohort
identity에 넣지 않는다.

```text
tupleCohortId
  = hash(
      programCandidateId
      × layoutIdentityId(candidate 또는 evidence)
      × adapterCandidateId
    )
```

## 5. G00-B3 축별 분류

### 5.1 Program

상태는 다음으로 제한한다.

- `TYPED_RUNTIME_PROGRAM_DECLARED`
- `DXBC_OCCURRENCE_EXACT`
- `DXBC_OCCURRENCE_EXACT_UNTRANSLATED`
- `DXBC_FAMILY_REPRESENTATIVE_ONLY`
- `BOUNDED_SOURCE_PROFILE_ONLY`
- `NO_PROGRAM_EVIDENCE`
- `NOT_APPLICABLE_PRESENTATION`

`programCandidateId`는 typed Program 또는 literal HLSL translation이 존재하는 occurrence-exact DXBC에만
발급한다. `DXBC_OCCURRENCE_EXACT_UNTRANSLATED`는 exact blob/evidence를 보존하지만 실행 가능한 Program
candidate가 없으므로 cohort에 들어가지 않는다. family representative, bounded source profile,
no-program도 evidence만 보존한다.

occurrence exact에는 두 경로가 있다. Family 경로는 `SINGLE_PERMUTATION_FAMILY` 또는 현재 child와 같은
`CHILD_MIC_ENGINE_EQUALITY`이고 source carrier까지 일치해야 한다. Exact-variant 경로는
`material.sourceMaterialPath`가 정확히 join되고 `SpriteParticle ↔ SPRITE_PARTICLE`,
`MeshParticle ↔ MESH_PARTICLE`의 fine renderer가 일치해야 한다. renderer가 맞고 translation이 있으면
`DXBC_OCCURRENCE_EXACT`, 없으면 `DXBC_OCCURRENCE_EXACT_UNTRANSLATED`다. renderer mismatch는
`EXACT_VARIANT_RENDERER_KIND_MISMATCH`와 representative evidence만 만들며, 별도로 증명된 family-exact
Program을 강등하지 않는다.

standalone mesh/sprite는 element kind 자체를 native carrier evidence로 사용하고 particle/trail은
sourceRecipe rendererShape와 TypeData evidence를 요구한다. 현재 V0 coarse carrier는 별도 Carrier/Adapter
축이며 source native carrier의 대용 증거가 아니다. sealed child-parent receipt가 이미
`LEAF_NAME_SEARCH`로 resolve한 265개 행은 receipt identity 그대로 소비하며 다시 leaf 검색하지 않는다.
`BLOCKED` child는 guessed parent나 새 leaf fallback으로 바꾸지 않고 authored parent, `rowSha256`, blocker
payload를 occurrence/evidence에 보존한다.

현재 projection의 검산값은 typed `50`, occurrence exact translated `2,148`, occurrence exact untranslated
`8`, family representative `3,671`, bounded source-only `525`, no Program evidence `1,055`, presentation
`109`다. child-parent가 `BLOCKED`인 12행은 authored parent evidence가 있어도 occurrence-exact로 승격하지 않고
representative로 유지한다.

### 5.2 Layout

상태는 다음으로 제한한다.

- `TYPED_PACKET_CLOSED`
- `EXACT_VARIANT_NATIVE_WIRE_ONLY_REQUIRES_PACKET_TRANSLATION`
- `NAMED_NATIVE_WIRE_ONLY_WITHIN_COUNT_CAPS`
- `NAMED_NATIVE_WIRE_ONLY_REQUIRES_COUNT_EXTENSION`
- `SOURCE_NAMES_ONLY`
- `UNRESOLVED`
- `NOT_APPLICABLE_PRESENTATION`

typed Layout은 texture register/sampler topology, count/mask, packed scalar/vector topology만 소유하고 asset과
값을 제외한다. Layout은 Program translation 및 Descriptor enabled 여부와 독립적으로 투영한다.
renderer-matched exact variant의 native binding wire는 translation 유무와 무관하게 exact Layout evidence가
되지만 typed runtime packet은 아니다. named ABI도 raw source metadata를 기준으로 전 occurrence에 투영한다.
native evidence가 cohort에 들어가는 것은 별도로 translated exact Program candidate가 있을 때뿐이며,
Descriptor 대량 확장 가능 상태로 표시하지 않는다.

named ABI의 current packet 적합성은 source의 전체 `uniformExpressionCounts`가 아니라 실제 resolve된
`textureSlots`, `scalarLanes`, `vectorLanes`로 판정한다. 현재 162개 resolved family 중 `69`개가
`6/52/3` count 상한 안에 있고 `93`개는 count 확장이 필요하다. 이는 packet topology 적합 판정이 아니다.
69개 중 non-empty texture layout은 현재 codec의 `textureRegister=i`, `samplerRegister=5+i`를 그대로
만족하지 않으므로 계속 `RUNTIME_PACKET_NOT_MATERIALIZED`와 topology blocker를 가진다. source expression
중 native binding lane에 선택되지 않은 항목을 runtime packet 요구량으로 잘못 세지 않는다.

현재 occurrence projection의 검산값은 typed packet `50`, exact-variant native wire `20`, named count-caps
`2,430`, named count-extension `3,153`, source names only `673`, unresolved `1,131`, presentation `109`다.
전체 exact Program `2,156 = 2,148 translated + 8 untranslated`의 Layout 분포는 exact-variant native wire
`20`, count-caps `960`, count-extension `1,002`, source names `106`, unresolved `68`이다.

### 5.3 Adapter

상태는 다음으로 제한한다.

- `TYPED_STATIC_DISPATCH_CANDIDATE`
- `RENDER_PROFILE_STATIC_CANDIDATE`
- `UNRESOLVED`
- `PRESENTATION_SEPARATE`

coarse carrier만으로 Adapter를 묶지 않는다. standalone mesh/sprite, mesh/sprite particle, decal particle,
animation trail, Cascade ribbon 등 실제 draw를 나누는 fine renderer kind/VF candidate와 render profile,
backend/opcode, pass index, 정적 render state로 candidate identity를 만든다.
현재 main에는 이 전체 분모를 닫는 compiled registry/draw harness가 없으므로 어떤 행도
`RUNTIME_PROVEN`으로 기록하지 않는다. VF, stage input, scene input, MRT, WPO와 compiled dispatch 미증명을
각 blocker로 보존한다.

현재 검산값은 typed static candidate `50`, render-profile static candidate `7,407`, presentation separate
`109`다. 다만 source body와 current authored body가 충돌하는 TypeDataMesh/no-meshModel 12행과
TypeDataRibbon/particle-kind 10행은 candidate를 발급하지 않고 `UNRESOLVED`로 둔다. 최종 검산값은 typed
`50`, render-profile `7,385`, unresolved `22`, presentation `109`다. static candidate 수는 runtime proof
수가 아니다.

### 5.4 Descriptor

상태는 다음으로 제한한다.

- `TYPED_VALUES_CLOSED`
- `SOURCE_VALUES_PRESENT_UNPACKED`
- `RESOURCE_ONLY_NO_MATERIAL_VALUES`
- `MISSING`
- `NOT_APPLICABLE_PRESENTATION`

typed Descriptor는 Layout ID, ordered texture asset/channel/color space/sampler, packed scalar/vector와
artist/color 값을 소유한다. source profile 값은 named ABI packing과 sampler closure 전까지
`SOURCE_VALUES_PRESENT_UNPACKED`이며 typed closed로 승격하지 않는다.

현재 검산값은 typed values `50`, source values unpacked `2,732`, resource-only `4,575`, missing `100`,
presentation `109`다.

### 5.5 Composition

상태는 다음으로 제한한다.

- `PRODUCT_BOUND_CUE`
- `RUNTIME_PUBLISHED_WITHOUT_CONSUMER`
- `CATALOG_DECLARED_ONLY`
- `AUTHORED_ONLY`

`scopeBits`는 `authored`, `catalogDeclared`, `runtimePublished`, `productConsumed`를 독립 보존한다.

### 5.6 Product와 사용자 검증

기술 join 상태는 `PRODUCT_JOIN_CLOSED`, `PUBLISHED_ELEMENT_STALE`,
`RUNTIME_PUBLISHED_UNCONSUMED`, `CATALOG_NOT_PUBLISHED`, `AUTHORED_NOT_CATALOGED`로 분리한다.

사용자 상태는 legacy golden review와 이번 horizontal V1 review를 분리한다. 기존 restoration receipt는
exact occurrence ID와 현재 authored element SHA가 맞을 때만 `legacyGoldenReview`로 전달한다. 이 값의
`APPROVED`는 과거 도화가 F 기준 화면 승인이지 Track A registry/dual-resolve/Adapter A/B 승인이 아니다.
`horizontalV1Review`는 별도 사용자 receipt 전까지 `NOT_RECORDED`이고 Artist F canary에는
`PENDING_USER_A_B`를 표시한다. Product consumer와 runtime publish가 같아도 V1 admission 및 사용자 visual
PASS를 자동 생성하지 않는다.

기존 receipt의 legacy `runtimeAdmission`과 carrier/material/render/composition `familyTuple`도 새 runtime
proof로 가져오지 않는다. 현재 exact hash join 결과의 legacy 사용자 상태만 APPROVED `17`, PENDING `5`,
NOT_RECORDED `7,544`로 투영하며 horizontal V1 review summary와 섞지 않는다.

## 6. cohort와 blocker

### 6.1 cohort 종류

- `TYPED_EXECUTION_COHORT`: typed Program + typed packet Layout + static Adapter candidate
- `NATIVE_EVIDENCE_COHORT`: translated occurrence-exact Program candidate + exact-variant/named native Layout
  evidence + static Adapter candidate

두 종류 모두 현재 `runtimeVerified=false`, `runtimeDescriptorExpansionEligible=false`다. typed cohort는
구조적으로 같은 Descriptor 재사용 후보라는 사실만 별도 표기하고, compiled Adapter 증거 전에는 실제
확장 가능 상태로 올리지 않는다. native evidence cohort는 typed packet까지 없으므로 구조적 Descriptor
확장 후보도 아니다. exact-untranslated 8행은 exact Layout evidence가 있어도 Program candidate가 없으므로
cohort가 아니다. Program/Layout/Adapter 중 required identity가 없으면 cohort를 만들지 않고 blocker bucket에만
넣는다.

현재 artifact identity는
`STATIC_EVIDENCE_CANDIDATES_ONLY_NO_RUNTIME_OR_PRODUCT_ADMISSION`으로 고정하고
`runtimeVerifiedCohortCount=0`, `runtimeDescriptorExpansionEligibleCohortCount=0`을 불변식으로 검증한다.

### 6.2 주요 blocker

Program:

- `PARENT_UNRESOLVED`
- `SHADERMAP_ABSENT`
- `DXBC_EXTRACTION_BLOCKED`
- `OCCURRENCE_STATIC_PERMUTATION_NOT_EXTRACTED`
- `COOKED_CARRIER_MISMATCH`
- `LITERAL_TRANSLATION_MISSING`
- `PROGRAM_EVIDENCE_ABSENT`
- `PROGRAM_EQUATION_EVIDENCE_ABSENT`
- `CHILD_PARENT_RESOLUTION_BLOCKED`
- `EXACT_VARIANT_RENDERER_KIND_MISMATCH`

Layout/Descriptor:

- `NAMED_ABI_BLOCKED`
- `RUNTIME_PACKET_NOT_MATERIALIZED`
- `SOURCE_REGISTER_BINDING_UNRESOLVED`
- `CURRENT_PACKET_CAPACITY_EXCEEDED`
- `SCALAR_VECTOR_PACKING_UNRESOLVED`
- `TEXTURE_REGISTER_SAMPLER_TOPOLOGY_UNMATERIALIZED`
- `TEXTURE_PARAMETER_TO_SLOT_UNRESOLVED`
- `SAMPLER_STATE_UNPROVEN`
- `EXACT_VARIANT_PACKET_TRANSLATION_REQUIRED`
- `SOURCE_VALUE_REPLAY_UNPROVEN`
- `MATERIAL_EVIDENCE_ABSENT`

Exact variant의 별도 admission 경계도 다음 blocker로 보존한다.

- `EXACT_VARIANT_ACTUAL_VF_PASS_UNPROVEN`
- `EXACT_VARIANT_RUNTIME_ADMISSION_UNPROVEN`
- `EXACT_VARIANT_VISUAL_ADMISSION_UNPROVEN`

Adapter:

- `VERTEX_FACTORY_UNPROVEN`
- `STAGE_INPUT_SEMANTICS_UNPROVEN`
- `SCENE_INPUTS_UNPROVEN`
- `OUTPUT_TOPOLOGY_MRT_UNPROVEN`
- `WPO_VERTEX_PROGRAM_UNPROVEN`
- `COMPILED_DRAW_DISPATCH_UNPROVEN`

Composition/Product blocker는 ambiguous cue/binding, missing consumer, stale published element를 별도 코드로
보존한다.

## 7. canary

### 7.1 G00 occurrence safety

`PR #166` G00이 실제 소유하는 sourceProfile 분모는 LanceMaster 34110의 `83`행과 34150의 `186`행,
합계 `269`행이다. Artist 31470은 같은 입력 문서지만 sourceProfile `0`, typed execution `17`인 대조군이다.
따라서 B3 source-evidence projection의 269행/status count와 G00 per-document source/execution count가
정확히 일치해야 한다. B3의 typed Program status를 G00의 source status로 억지 변환하지 않고, G00이
소유하지 않는 전체 7,566개 분모도 비교하지 않는다.

### 7.2 Artist F horizontal golden

`effect.artist.skill.31470.unified`의
`sprite.2b3dc6842507e910`, `sprite.c65181324417a1a8`이 존재하고 같은 Program/Layout/Adapter candidate를
resolve하는지 검사한다. 기존 receipt가 현재 element SHA와 맞을 때만 사용자 승인 정보를 연결한다.
이 Track B artifact는 Track A 병합 여부와 무관하게 `runtimeVerified=false`를 유지한다. Track A runtime
receipt를 새 입력 계약으로 소비하는 후속 revision 없이는 compiled Adapter proof를 자동 추론하지 않는다.

### 7.3 Product consumer/runtime set

현재 `99 character + 44 Valtan pattern + 2 BossCatalog visual = 145 assets / 2,554 elements`가 runtime
catalog 대상 집합과 정확히 일치하는 현재 snapshot을 기록한다. 향후 정상적인 Product 증감은 inventory
생성을 막지 않고 drift status/blocker와 새 summary로 나타내며, 이 수치는 현재 repository regression
test가 소유한다.

## 8. fail-closed와 transaction

다음은 fatal이며 기존 output을 보존한다.

- duplicate JSON key, non-finite number, wrong schema/version, filename/asset identity mismatch
- duplicate effectAssetId/element ID/candidate hash collision
- evidence artifact self hash 또는 dependency pin drift
- owner contract와 맞지 않는 DXBC/ASM/HLSLI set 또는 hash 불일치
- unknown cooked permutation selection과 sealed receipt 밖의 새 leaf fallback 시도
- malformed typed count/mask, duplicate lane/register/packed index, missing sampler
- ambiguous duplicate catalog/runtime/cue row
- G00 evidence canary 또는 Artist F stable golden identity mismatch
- axis summary 합이 7,566이 아닌 결과
- nondeterministic ordering 또는 artifact self hash 불일치

unresolved parent, blocked child receipt, blocked DXBC/named ABI, exact-untranslated Program, missing Product
join, stale publish, no material evidence는 fatal이 아니다. occurrence와 sealed receipt/exact evidence를
보존하되 candidate/cohort 승격만 막는다. 새 leaf 추측, blocker receipt 유실, untranslated DXBC를 실행
Program으로 승격하는 것은 fail-closed 오류다. well-formed Product consumer/runtime set의 정상적인 증감도
fatal이 아니며 현재 145/2,554 snapshot과의 차이를 canary status로 기록한다.

## 9. 구현 순서

### G00-B3-01 strict input와 identity

strict JSON, canonical hash, input tracker, artifact self hash/dependency pin, authored discovery와 stable
occurrence ID를 구현한다.

종료 조건: 416 docs / 7,566 occurrences가 deterministic하게 stage되고 identity mutation fixture가 실패한다.

### G00-B3-02 여섯 축 projection

carrier와 Program/Layout/Adapter/Descriptor/Composition/Product projection을 구현한다. family 대표 DXBC와
named ABI를 typed closure로 과장하지 않는 status/blocker 불변식을 먼저 둔다.

종료 조건: 모든 축의 status 합이 7,566이고 presentation 109행은 material axes가 NOT_APPLICABLE이다.

### G00-B3-03 cohort와 canary

content-addressed candidate/variant registry, typed/native evidence cohort, blocker bucket, 세 canary와 summary를
구현한다.

종료 조건: runtime verified cohort는 0이며 Descriptor 값만 바꾼 fixture는 tuple cohort ID를 바꾸지 않는다.

### G00-B3-04 transaction·검증·문서

`--check`, atomic replace, rollback test, real repository 생성과 결과 문서를 닫는다.

종료 조건: unit test, real build, real `--check`, `py_compile`, JSON/self hash, deterministic byte comparison,
`git diff --check`가 통과한다.

## 10. 자동 검증

```powershell
python Tools/EffectPipeline/test_build_effect_tuple_cohort_inventory.py
python Tools/EffectPipeline/build_effect_tuple_cohort_inventory.py
python Tools/EffectPipeline/build_effect_tuple_cohort_inventory.py --check
python -m py_compile Tools/EffectPipeline/build_effect_tuple_cohort_inventory.py Tools/EffectPipeline/test_build_effect_tuple_cohort_inventory.py
git diff --check
```

생성 도구는 Python/JSON evidence만 변경하므로 Engine/Client build를 이 PR의 완료 증거로 과장하지 않는다.
Track A의 실제 Sprite Adapter build와 사용자 Artist F A/B는 별도 lane의 종료 조건이다.

## 11. 이번 PR의 비범위

- HLSL, C++, shader registry, renderer Adapter 구현
- EffectCatalog 또는 authored Effect 수정
- Composition/timing/attachment 변경
- Valtan stable ID Descriptor materialization
- Client 자율 실행, 화면 캡처, visual PASS 판정
- Product admission 또는 cohort bulk publish

Track B는 evidence inventory와 static cohort까지만 담당한다. Track A는 Artist F의
registry/dual-resolve/actual Sprite draw와 사용자 canary A/B를 닫는다. 그 뒤 후속 Track A 확장 또는 Track C가
inventory의 typed cohort에서 cross-domain occurrence를 선택해 Descriptor 재사용을 증명하고,
Composition·실제 스킬 timing·사용자 Product admission을 occurrence별로 닫는다. Track B artifact가 이를 자동
승격하지 않는다.
