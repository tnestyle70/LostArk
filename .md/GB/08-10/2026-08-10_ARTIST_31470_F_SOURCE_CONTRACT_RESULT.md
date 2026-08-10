# 2026-08-10 Artist 31470 F Track A0 Source Contract Result

## 결과

도화가 F `31470 / 필법 : 한획긋기`의 실행 renderer를 구현한 것이 아니라, 원본 UE3 근거와
현재 손실을 native v14 `purpose=source_contract` 문서로 고정했다. 이 문서는 v13 Product와
Catalog가 소비하지 못하며 모든 실행 admission을 fail-closed 상태로 유지한다.

현재 상태는 다음과 같다.

- candidate envelope: `SOURCE_EXTRACTED`
- aggregate evidence: `SOURCE_EVIDENCE_PARTIAL`
- manual assembly: `MANUAL_MASTER_ASSEMBLY_PENDING`
- visual approval: `NOT_VISUAL_APPROVED`
- compiled runtime admission: false
- Product admission: false
- Product/Catalog publish: 0

`SOURCE_EXTRACTED`는 선택한 근거를 저장했다는 뜻이지 Cascade 의미가 전부 해석됐다는 뜻이
아니다. 아래 분모와 blocker가 0이 되기 전에는 runtime 복원 완료로 승격하지 않는다.

## 고정한 실행 분모

- base-enabled action cue: 7
- active occurrence: 35
- renderer:
  `MeshParticle 13 / SpriteParticle 16 / DecalParticle 3 / CascadeRibbon 1 /
  LightParticle 1 / ScreenPost 1`
- execution-disabled variant 제외: 93
- 선택 FIRST_LOD module reference order: 399
- 선택 module top-level tagged property: 1,434
- primitive leaf: 1,572
- distribution wrapper: 629
- cue local transform: 7/7
- source parameter override: unique 9, occurrence binding 11

원본 source receipt의 `inputSlot=R`은 바꾸지 않았고 현재 게임의 F 매핑은 Derived contract에만
기록했다. WModel에는 occurrence, Cascade module, material override를 넣지 않았다.

## 확인된 partial과 blocker

### LOD와 module closure

`913/913`은 선택 FIRST_LOD의 external module reference만 닫혔다는 수치다. active source 전체
closure 완료 수치가 아니다.

- 35 emitter 중 30개가 두 번째 LOD를 가진다.
- non-selected LOD 또는 filtered reference 880건은 실행 근거에서 차단했다.
- internal module native tail은 151 occurrence만 증명됐다.
- external module native tail 248 occurrence는 아직 증명되지 않았다.
- seeded module 14 occurrence 중 8개는 opaque metadata, 5개는 seed-array-only,
  1개는 class-default unresolved다.
- module coverage 399건은
  `source_decoded 13 / deterministic_conversion 108 / metadata_only 4 / unresolved 274`이다.

### local distribution과 Light

- referenced distribution target은 15 unique / 17 occurrence다.
- package filename/bytes/SHA가 source receipt에 고정된 target은 8/15이고, 그중 pinned child
  payload를 decode한 target은 7/15이며, 현재 exact source-era physical UPK가 존재하는 target은
  3/15다.
- raw record payload는 14 unique가 decoded이고 target014 1 unique / 2 occurrence는
  cross-revision unresolved다. 이 수치는 source-exact 수치가 아니다.
- semantic source-ready는 9 unique / 9 occurrence, semantic blocked는 6 unique / 8
  occurrence다. compiled execution allowed는 0/17이다.
- external module closure만 있는 target000-003/007-009는 receipt package identity가 없으므로
  decoded record를 `SOURCE_EXACT`로 승격하지 않았다.
- PointLightComponent 1 unique / 1 occurrence는 exact child `Brightness=10`과 두 explicit false
  flag를 보존했다. 현재 archetype/CDO의 `Radius=200`, `FalloffExponent=2`, white는 current-only
  evidence로 분리했고 source-era script/default 및 Light renderer blocker를 유지했다.

### WModel과 scale (별도 geometry receipt 소비)

이 source slice가 geometry generator/runtime을 구현했다는 뜻이 아니다. 별도 geometry lane이 만든
receipt를 tracked derived input으로 소비한다. 그 receipt는 7개 Mesh carrier에서 glTF와 현재
WModel의 position, normal, tangent XYZ, UV0,
topology 및 reversed winding 관계를 수치로 증명했다. 현재 cook의 geometry는 source cm 크기로
저장되므로 MeshParticle에만 `carrierGeometryPreScale=0.01`을 적용하고, Cascade StartSize는
signed dimensionless axis reorder로 유지한다. Sprite/Decal의 cm-to-m 변환과 섞지 않는다.

다음 blocker 때문에 geometry compiled admission은 0/7이다.

- tangent handedness 보존 0/7
- source `COLOR_0`가 존재하지만 차단된 모델 2개
- runtime bounds consumption 0/7
- exact UPK -> UModel/glTF provenance 미확정

### Material

active material identity는 28개다. Light builtin 1개를 제외한 27개는 모두
`COOKED_PARTIAL`이며 exact Material graph가 아니다. 후보의 34 rendered occurrence는
`BLOCKED_COOKED_PARTIAL_NO_TYPED_MATERIAL_RECIPE`, Light 1개는
`NON_RENDER_BUILTIN_NOT_APPLICABLE`이다. source material profile을 runtime enabled로 세탁하지
않았다.

## 구현한 계약

- runtime v13과 분리된 v14 `source_contract` parse/strict validation/serialize
- 629 distribution 모두의 typed `referenceId/occurrenceId`, `payloadStatus`, `fidelity`,
  `executionAdmission` public field. 이 중 local-reference 17 occurrence만 stable non-empty ID로
  binding되고, 나머지 612 inline distribution은 empty ID와 별도 inline fidelity를 사용한다.
- 17 distribution occurrence와 1 PointLight occurrence의 typed `localReferenceBindings` 및
  exact/current evidence 분리
- distribution definition/occurrence에서 instance explicit -> nested archetype/template -> class
  CDO -> parent CDO hierarchy -> evaluator 순서의 field provenance
- semantic-blocked distribution의 payload 비열람과 admission-before-payload 검증. inert transport
  numeric shape는 source/evaluator evidence로 소비하지 않음
- renderer type과 source space의 명시적 저장
- source graph/closure/material/evidence/local-reference/geometry receipt SHA pin
- selected LOD identity와 ordered module reference provenance
- cue source position, cue local transform, parameter override, transform composition order
- module/property coverage와 blocker
- local reference blocker의 property -> module -> element -> receipt/registry -> Product 집합 포함
  전파와 `blockerCount == 0` admission gate
- material/geometry/compiled-execution admission
- ParticleParameter의 `actionCue`/`none` binding 보존
- v13 JSON 또는 in-memory 문서가 v14 evidence field를 싣고 무음 소거하는 우회 차단
- Candidate를 Catalog, Publisher, Playback, Renderer에서 계속 차단

공용 `Effect_Playback`, `Effect_DocumentRenderer`, HLSL, `EffectCatalog`, Effect Tool은 이
슬라이스에서 수정하지 않았다.

## 미해결 native oracle과 Product 경계

- target000의 current nested archetype `Constant=1.0`은
  `CURRENT_REVISION_ARCHETYPE_EVIDENCE`일 뿐 source-era default가 아니다.
  `CLASS_DEFAULT_ARCHETYPE_UNPROVEN`과 source-era Engine CDO/script blocker를 유지한다.
- target001/009의 current effective NORMAL range `[0,100] -> [0,100]`은 네 range와 enum의
  source-era provenance가 없어 실행할 수 없다.
- target007/014의 `EFDistributionVectorMultiplyParticleParameter`는 custom evaluator와 runtime
  parameter source가 닫히지 않았다. class 이름에 particleparameter가 포함돼도 표준 evaluator로
  분류하지 않는다.
- target014의 old 66,494-byte package/SHA는 부재한다. current 66,557-byte recovery payload의
  `Constant=(1,1,.6)` 및 parent/reference quorum은 old child equality 근거로 쓰지 않았다.
  cross-revision, old absent-binding fallback, selected LOD class-default blocker를 모두 유지한다.
- target008은 standard ConstantCurve source payload를 보존하지만 curve compiler가 없다.
- Product admission은 blocker 39종이 남아 false이고 Catalog/Product 연결은 0/35다.
- source closure는 repo-tracked geometry receipt를 canonical text hash로 소비한다. raw WModel,
  glTF/bin 및 geometry generator 자체의 role-aware hash 검증은 별도 geometry commit의
  integration prerequisite다.

## G01/G05-S code-only semantic closure checkpoint

최종 checked-in Source output은 재생성하지 않았다. 대신 frozen input에서 temp semantic closure를
생성하고 별도 oracle로 다시 계산하는 code-only 경계를 추가했다.

### 분모와 결정

- selected LOD default field: `70 UNRESOLVED` (`Level` 35 + `Enabled` 35)
- ordered module: `399 UNRESOLVED`
- top-level property: `1,434 UNRESOLVED`
- primitive leaf: `1,572 UNRESOLVED`
- distribution: `629 UNRESOLVED`
- native-tail implicit slot: `151 VERIFIED_IRRELEVANT / 248 UNRESOLVED`
- local distribution: `15 definition / 17 occurrence`, stable definition/reference/occurrence ID
- 전체 distribution identity: `510 definition/reference / 629 occurrence`
- PointLight: 1 definition/occurrence, exact child 5 field와 current-only 3 field 모두 실행
  handler/oracle가 없어 `UNRESOLVED`
- seed: `14 UNRESOLVED`
- implicit default: `RequiredLocalSpace 8 / Decal 3 / Ribbon 1 / ScreenPost 1 / Light 1`
- Product: `0/35`

기존 `source_decoded`와 `deterministic_conversion`은 source/structural 상태이지 실행 consumption
증거가 아니다. 현재 registry의 197 field rule과 32 class는 모두 `runtimeImplemented=false`이고
opcode도 미할당이므로, handler receipt와 독립 numeric oracle 없이 `EXECUTION_CONSUMED`로
승격하지 않았다. requested 399/1,434/1,572/629의 full consumption은 아직 BLOCK이다.

### fail-closed 경계

- lookup/target이 없는 137 inline distribution은 legacy zero recipe를 실행 근거로 쓰지 않는다.
- operation reconstruction 409, lookup chunk/count reconstruction 각 257, explicit random operation
  82를 field-level로 분리하고 reconstruction fidelity를 source exact와 합치지 않는다.
- external module 248의 fidelity는 `CURRENT_REVISION_EVIDENCE`다. native-tail, seed, default를
  닫아도 historical source fidelity로 자동 승격하지 않는다.
- target000/001/007/009/014는 pre-payload rejection을 유지하며 target007 poison mutation을
  production binder regression에 추가했다.
- PointLight exact child에는 Brightness, 두 composite-shadow flag, LightGuid, LightMapGuid를
  보존했다. Radius/Falloff/Color current default와 GUID/flag 실행 무관성을 추측하지 않는다.
- selected-LOD Level/Enabled 실행 blocker 두 개는 semantic module과 opt-in coverage 399/399에
  전파한다. PointLight binding과 8 field의 고유 실행 blocker 다섯 개도 Light module coverage,
  compiled admission, element/global receipt와 registry까지 보존한다.
- 네 blocker set, artifact binding integrity, execution admission, Product admission은 각 행에서
  독립적으로 검증된다.
- module axis 재분류 mutation은 oracle이 거부하고, opt-in Source coverage transport는 검증된
  artifact/execution blocker 합집합을 보존한다.

### class-lineage bridge

Integration bridge `ccb60a519d1c9e2fb955652e862561b9b670438e`를 cherry-pick했다. Source
semantic row와 opt-in temp candidate의 399 module coverage는 모두 raw class와 정확히 같은
`exactSourceClass`와 `aliasId`를 가진다. 승인 alias table은 비어 있고 alias ID는 전부 empty다.
`efparticlemodule` prefix 또는 `_seeded` suffix를 자동 표준 alias로 만들지 않았으며 이에 해당하는
26 occurrence/12 exact class family는 exact handler/alias evidence가 생길 때까지 blocker를 유지한다.

현 bridge Codec는 `normalizedClass == Normalize_SourceModuleClass(raw class)`를 계속 요구하면서
empty alias mismatch도 허용한다. Source 규칙의 `normalizedClass=exactSourceClass.casefold()`와
충돌하므로 final candidate Codec round-trip 전 Integration이 이 실제 conflict를 해결해야 한다.
Source lane은 schema/Codec를 수정하지 않았다.

이 checkpoint는 pure resolver/binder, independent oracle, temp-output generation과 mutation test
PASS만 주장한다. G01의 consumed-or-irrelevant 종료, G05-S evaluator/numeric reconstruction,
Gate 1 checked output regeneration은 아직 완료가 아니다.

## 산출물 identity

아래 file SHA는 repo-tracked UTF-8 text의 LF canonical hash다.

- candidate file SHA-256:
  `8dbcd0c871d0a9f1698e65093b90462bb4d0f95b246c3c8db629d7d24d89f599`
- receipt file SHA-256:
  `90010f2d13799bca6e9531d582d98fc1bc759b494ba8f7473b2a4970be735b4b`
- registry file SHA-256:
  `599e9fa3940d2af9be0f61b1cf4b04ad2ed37e733e4cf289286c8599f66c9a44`
- registry contract SHA-256:
  `698583c9d234f31b588c6a0d468106391d7affbd964e92d98ae5e7fc45041969`
- generated header SHA-256:
  `33f6a0f654b0a1bbbe0f99f7cd2353a96c2ed89192e5f0143919ff0cceb8b9dc`

## 자동 검증

후속 frozen audit에서 Python 기본 JSON parser가 duplicate key의 마지막 값을 채택하는
반례가 발견됐다. production source closure, independent verifier, source-contract builder는
이제 `effect_source_contract_io.load_strict_json_object` 하나를 사용해 모든 nesting의 duplicate
key와 UTF-8 BOM을 canonical hash 계산 전에 거부한다.

- frozen input 5종 각각 top-level duplicate mutation: 세 CLI 모두 reject
- frozen input 5종 각각 nested duplicate mutation: 세 CLI 모두 reject
- semantic closure/source-contract unit: 56 tests PASS
- focused Source ProjectAudit: 91 tests PASS
- LF/CRLF tracked-text equivalence 계약은 유지

- code-only semantic closure temp write / deterministic `--check` / independent oracle:
  `399 / 1,434 / 1,572 / 629`, Product `0/35`, PASS
- focused semantic closure mutation suite: 39/39 PASS
- `Test-Artist31470SourceContract.ps1` shallow Source slice: 91/91 PASS
- `Tools/LevelPlacementExtractor` 전체 discovery: 304개 중 Source 포함 303 PASS,
  소유 범위 밖 Material render-state receipt mismatch 1 ERROR
- 전체 `Invoke-ProjectAudit.ps1`: `effect.artist-31470-source-contract` PASS. 전체 audit은
  map/resource, G09, Material/Geometry prerequisite, WFX/authored rollout 등 소유 범위 밖 13개 실패
- `-DeepSourceAudit`은 현재 세션에 필요한 외부 source path 7개가 제공되지 않아 실행하지 않았다.
  대신 frozen input 다섯 개의 canonical SHA와 source/local self hash는 builder와 독립 oracle 양쪽에서
  검증했다.
- Integration bridge producer evidence: Debug/Release harness compile PASS, class-lineage audit 6/6,
  기존 Source 52 PASS, Debug Client full build PASS. 현재 Source worktree에는 harness binary가 없어
  같은 C++ audit을 재실행하지 않았다.
- v14 opt-in class-lineage transport, stable ID/order, local raw-lineage, admission/integrity,
  default/reconstruction/random-stream, PointLight, blocker-axis laundering 반례: PASS
- Product publisher 지원 version: 5~12 유지
- EffectCatalog candidate reference: 0
- checked-in candidate/receipt/registry/header 변경: 0
- `git diff --check`: PASS, line-ending warning만 존재

빌드는 컴파일·링크 오류 확인에만 사용했다. 이미지, 스크린샷, GPU 화면 판정은 수행하지 않았다.
geometry generator의 raw glTF/bin/WModel 검증은 이 source commit에서 실행 완료로 주장하지 않으며
별도 geometry commit과 함께 integration할 때의 prerequisite로 남긴다.

## 다음 Track A 경계

1. Artist 전용 fixture assertion을 corpus 독립 audit으로 분리하고 v14 schema를 범용화한다.
2. 4클래스 51스킬/101 Product cue와 Valtan 193 ParticleSystem의 source denominator를 같은
   evidence envelope로 생성한다.
3. non-selected LOD, class default, seeded payload, external native tail과 local distribution
   target을 source가 존재하는 범위까지 닫는다.
4. WModel tangent/COLOR_0/bounds와 typed Material/RenderState recipe를 먼저 admission시킨다.
5. raw module 문자열을 hot path에서 찾지 않는 immutable typed Cascade IR compiler를 만든다.
6. MeshParticle, SpriteParticle, DecalParticle, Ribbon/Trail, Light/Post를 numeric golden으로
   통과시킨 뒤 기존 Catalog에 cue 단위로 admission한다.

원본 cooked package에 실제로 없는 Material arithmetic graph는 `SOURCE_EXACT`로 표기하지 않는다.
recoverable input/static permutation은 source exact로, 빠진 연산은 별도 reconstructed family
evaluator로 구분한다.

## Gate 1 최종 checked-in 재생성

G02 Geometry와 G03 Material evidence가 동결된 뒤 code-only semantic closure를 처음으로
checked-in 입력에 포함해 candidate, receipt, registry, generated header를 한 번에 다시
생성했다. 이 재생성은 실행 의미를 승격하지 않는다. 399 module, 1,434 top-level property,
1,572 primitive leaf, 629 distribution은 전부 분모와 blocker를 보존하며 semantic execution과
Product admission은 계속 `false` (`0/35`)다.

- semantic closure LF-canonical SHA-256:
  `73f2e93853c495d8489c1a234cb5dd02a1306f419614ff7fc6d4359ec1198b08`
- semantic closure self SHA-256:
  `75e1e584948951a6b288f9fedd1ab6abebec84ac2639b68ca7bedc1776fd256b`
- candidate LF-canonical SHA-256:
  `27cfb2b81ce688b9dd55cdc53afe6e5e709cefee390dab0e9f9c5d3e9e7f244e`
- receipt LF-canonical SHA-256:
  `989295f594ee95d207ccb75716a71c0a4daef867a335206744dea4deeb8a3d1e`
- registry LF-canonical SHA-256:
  `f01499af197be6d417a9efcf6a338f6819242ac58a1c4e361e3c4b0dbb7d0174`
- registry contract SHA-256:
  `201652eceeaa40258fec5574669b2e873b2305cbe33f859c0f17e58a15b2e553`
- generated header LF-canonical SHA-256:
  `ce05a0f3654122a48fc39527e4521805c114d13048f5bca59fcc34a0a2131825`

`Test-Artist31470SourceContract.ps1`은 이제 semantic builder `--check`, 독립 semantic
oracle, semantic closure를 명시 입력으로 받는 source-contract `--check`, 91개 unit/mutation
test를 같은 실행에서 검증한다. 최종 실행은 모두 PASS했다. 이미지·스크린샷·육안 판정은
실행하지 않았다.
