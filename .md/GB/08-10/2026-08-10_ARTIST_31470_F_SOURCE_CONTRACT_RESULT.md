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

## G05-S source execution semantics adapter 결과

`skill.31470.source-execution-semantics.receipt.json`을 추가했다. 이 파일은 기존 candidate를
Product로 승인하는 문서가 아니라, source evidence를 다음 typed compiler가 소비할 수 있게
물질화한 fail-closed adapter receipt다.

### 분모와 결정

- occurrence: 35
- selected LOD field: `70 VERIFIED_IRRELEVANT`
- module: `370 READY_FOR_HANDLER / 29 BLOCKED`
- top-level property: `1,029 READY_FOR_HANDLER / 402 VERIFIED_IRRELEVANT / 3 BLOCKED`
- primitive leaf: `1,170 READY_FOR_HANDLER / 402 VERIFIED_IRRELEVANT`
- distribution: `626 READY_FOR_HANDLER / 3 BLOCKED`
- native tail: `399 VERIFIED_IRRELEVANT`
- seed: `14 READY_FOR_HANDLER`
- implicit default: `13 READY_FOR_HANDLER / 1 VERIFIED_IRRELEVANT`
- typed payload: literal 1,590 / distribution 629, unclassified 0, silent fallback 0
- Product admission: false

`allRowsClassifiedAndBound=true`지만 `allRowsConsumedOrIrrelevant=false`다. exact custom handler와
custom distribution 세 행이 실제 blocker이므로 이를 consumed로 세탁하지 않았다.

### 실제로 닫힌 의미

- installed current package 70개를 열고 external module 248 occurrence가 참조하는 178 unique
  export를 다시 decode했다. source closure와 tagged property가 모두 같고 각 export는
  `serialSize == propertyStreamEnd`였다. 따라서 native tail은 current-revision 범위에서 실행
  입력이 아님을 검증했다. historical source exact로는 승격하지 않았다.
- inline distribution 612개는 operation, lookup chunk/count, default minimum/maximum, lookup/key를
  typed descriptor로 보존했다. fixed time `0/.25/1`과 fixed random vector 세 세트의 값을 생성기와
  독립 verifier가 별도로 계산해 일치시켰다. operation 409, chunk/count 각 257의 reconstructed
  provenance는 source tagged field와 분리했다.
- standard local distribution/curve 14 occurrence를 닫았다. target000 두 occurrence는 current
  archetype chain의 `Constant=1`과 `ParameterName=None`을 결합해 constant fallback 1로,
  target001은 Spawn 0의 normal mapping으로 0, target009는 life 0.6000000238의 normal mapping으로
  0.6000000238을 계산했다. target002/004/005/006은 typed ActionCue direct input을, target003은
  scalar/vector type mismatch의 constant fallback을, target008은 cubic curve oracle을 사용한다.
- target014 current child의 `rotation`, `DPM_DIRECT`, `Constant=(1,1,0.6000000238)`은 별도 current
  observation으로 보존했다. old 66,494-byte package와 custom multiply evaluator가 없으므로 두
  occurrence를 실행값으로 승인하지 않았다. target007 한 occurrence도 같은 custom evaluator
  blocker를 유지했다.
- source seed array 5개와 32-byte struct body 8개에서 seed를 보존했고 class-default empty array
  1개를 구분했다. current Engine/EFGame seeded CDO 8종의 ParameterName/selection/reset policy를
  직접 decode해 같은 정책임을 확인했다. 이는 current script evidence이며 source-era exact가 아니다.
- Required local-space 8개는 current inherited false, Decal 3개와 Ribbon 1개는 current CDO value,
  Light 1개는 local PointLight default chain으로 물질화했다. ScreenPost implicit default는 source
  field가 아니어서 renderer projection irrelevance로 닫았다.
- PointLight는 exact instance Brightness/flags와 current Radius 200/Falloff 2/white를 분리했다.
  LightGuid/LightMapGuid는 runtime light simulation 입력이 아니므로 전용 irrelevance row다.

### 남은 blocker

- `efparticlemodule*` 또는 `_seeded`인 exact class 26 occurrence/13 class family는 자동 alias하지
  않았다. exact handler numeric oracle이 없어 `EXACT_SOURCE_CLASS_HANDLER_NUMERIC_ORACLE_REQUIRED`다.
- 위 custom distribution 3 occurrence를 소유하는 standard module 3개도 함께 차단돼 module 총
  blocker는 29개다.
- typed runtime compiler capability 결합과 최종 Product admission은 Integration/G06 이후 단계가
  소유한다. 이 source receipt만으로 35/35 실행 완료를 주장하지 않는다.

### 자동 검증

- focused unit/mutation: 22/22 PASS
- independent shallow oracle: modules 399, ready 370, blocked 29, distributions 629, blocked 3 PASS
- independent deep oracle: current UPK 70개, external module export 178개, Engine/EFGame CDO PASS
- generator deterministic deep `--check`: PASS
- focused shallow/deep ProjectAudit: PASS
- 전체 `Invoke-ProjectAudit.ps1`: 실행했으나 이 lane 밖의 publisher v14 expectation,
  four-class Artist/31210 stage mismatch, 미배포 Character resource 등 기존 통합 항목으로 exit 1.
  새 `effect.artist-31470-source-execution-semantics`는 focused audit에서 별도로 PASS
- 이미지, 스크린샷, GPU 화면, 육안 판정: 실행하지 않음

receipt LF/raw SHA-256은
`de15843dfa2f151371c1e26c472f2d42a0bfd7c7f8c8a41a3cdd2da08eaccb9a`, self SHA-256은
`7e1113dd05bcc9b51056cacc27da1805f7a6d26f65dda5b72c99d26c3141a71c`다.

## G05-S2 exact seeded handler와 blocker owner closure 결과 — 독립 감사 BLOCK

아래 내용은 commit `9b046d6171b5dcd73cec969f5ac3e346762db0d8`이 당시 생성한 결과다.
후속 독립 감사에서 current wrapper/script evidence에 source-era identity가 없고 fixed input digest
parity가 actual particle output oracle이 아니라는 문제가 확인됐다. 따라서 아래 `11 resolved`,
`381 READY / 18 BLOCKED`, capability grant 7 결론은 현재 실행 admission으로 무효이며, 감사 이력으로만
보존한다. 현재 정본은 뒤의 v2 corrective 결과다.

`skill.31470.custom-handler-oracle.receipt.json`을 추가했다. 이 receipt는 기존 G05-S의 29 blocked
module을 source ID 그대로 join하고, 근거가 생긴 표준 seeded 11건만 exact handler capability로
승격한다. class prefix/suffix 문자열 정규화는 사용하지 않았다.

### 표준 seeded 11건

- class family: Color 2, Lifetime 1, Location 2, LocationPrimitiveCylinder 3,
  MeshRotation 1, Size 1, Velocity 1
- current `Engine.u`: 7개 exact seeded class가 대응 base class를 상속하고 direct child가
  `RandomSeedInfo` struct property 하나이며 UFunction child는 0임을 검증
- current `EFEngine.dll`: 7개 exact seeded `Spawn` wrapper가 정확한 base
  `SpawnEx(..., FRandomStream*)` export로 dispatch함을 wrapper bytes와 direct/vtable target으로 검증
- fixed-seed numeric oracle: source first seed 또는 명시적 non-source oracle occurrence seed에서
  UE3 LCG random unit을 만들고 time `0/.25/1`, draw offset `0/4/8`의 typed distribution input을 계산
- capability grant: 7 family / 11 occurrence, `normalizedStringAliasAllowed=false`

### 남은 native/evaluator blocker

- EF custom module: 6 family / 15 occurrence `BLOCKED`
  - `efparticlemodulelocationonground` 2
  - `efparticlemodulelocationprimitivecylinderspin` 2
  - `efparticlemodulelocationprimitivecylinderspin_seeded` 3
  - `efparticlemoduletypedatadecal` 3
  - `efparticlemoduletypedatalight` 1
  - `efparticlemodulevelocityoverlifetime` 4
- custom distribution:
  `efdistributionvectormultiplyparticleparameter` 3 occurrence `BLOCKED`
- current script class shape와 installed export 부재는 기록했지만 source-era/native evaluator 동등성으로
  승격하지 않았다. exact native dispatch 또는 controlled numeric evaluator가 생기기 전까지 blocker다.

### blocker ownership과 공개 결합 필드

- base G05-S blocked module: 29
- exact seeded capability가 해결: 11
- 남은 blocked module: 18 = EF custom handler 15 + custom distribution owner module 3
- `moduleBlockerOwnership`: 29/29
- `distributionBlockerOwnership`: 3/3
- ownerless blocker: 0
- projected handler decision: `381 READY_FOR_HANDLER / 18 BLOCKED`
- distribution decision: `626 READY_FOR_HANDLER / 3 BLOCKED`
- Product admission: false

runtime/compiler가 소비할 공개 필드는 `capabilityGrants[].handlerCapabilityId`,
`exactSourceClass`, `baseHandlerCapabilityId`, `grant`, `requiredEvidenceDecision`과
`moduleBlockerOwnership[]`, `distributionBlockerOwnership[]`이다. runtime은 이 receipt의 exact class
join을 소비해야 하며 raw class 이름을 다시 normalize해서는 안 된다.

### 자동 검증

- focused unit/mutation: 21/21 PASS
- builder deterministic deep `--check`: PASS
- independent shallow oracle: ready 381, blocked 18, distribution blocked 3, ownerless 0 PASS
- independent deep oracle: source deep receipt, Engine/EFGame script package, EFEngine/LOSTARK bytes,
  14 script class serial, 7 native wrapper bytes/direct target PASS
- focused shallow/deep ProjectAudit: PASS
- duplicate top-level/nested JSON key, bool version, class/capability/occurrence reassignment,
  source seed/random stream mutation, custom blocker 제거, owner 제거/교체, Product 승격 mutation: 모두 reject
- 이미지, 스크린샷, GPU 화면, 육안 판정: 실행하지 않음

custom handler oracle LF-canonical text SHA-256은
`157e396df78270fe2aa00839f204b2a290841cc770c60b17eefc3ee4fc68eb67`, self SHA-256은
`f69e53b54c779d8faeaaef1d832ec75c0462281edcfa0f522dc2e074c17c3111`다.

## G05-S2 v2 actual-output feasibility corrective 결과

`skill.31470.custom-handler-oracle.receipt.json`을 format version 2로 다시 생성했다. 이 corrective는
source evidence integrity 후보를 만들었을 뿐 execution-readiness를 승인하지 않는다.

### current evidence 재도출

- current `Engine.u`의 standard seeded 7 class는 대응 base class, exact
  `RandomSeedInfo` struct-property direct child 1개, UFunction child 0개를 raw export child chain에서
  다시 계산했다.
- hash-pinned current `EFEngine.dll`의 7 wrapper는 random-stream owner call 반환 `RAX`가
  `[rsp+0x20]`에 저장되고 그 5-byte store 바로 다음 instruction이 exact base `SpawnEx` direct/vtable
  dispatch임을 actual wrapper bytes로 검증했다.
- 이 근거의 판정은 `CURRENT_REVISION_CROSS_REVISION_ALIAS_EVIDENCE`다. source-era handler binary
  identity 또는 actual mutated particle output 동등성을 주장하지 않는다.
- fixed seed/time/draw 결과는 `diagnosticFixedSeedInputs`로 이름을 바꿨다. input SHA parity는 output
  oracle로 인정하지 않고 `capabilityGrants`는 0이다.

### 29행 feasibility matrix

- standard seeded handler: 7 family / 11 occurrence `BLOCKED`
- EF custom handler: 6 family / 15 occurrence `BLOCKED`
- custom distribution owner module: 2 family / 3 occurrence `BLOCKED`
- module matrix: 29/29 stable occurrence row, ownerless 0
- custom distribution matrix: 3/3 stable distribution row
- actual native particle output oracle: 0
- actual native distribution output oracle: 0
- pilot fixture/provider/tolerance가 확보된 행: 0
- execution-readiness: `BLOCKED`, unresolved execution row 29

각 module row는 required runtime output, source/current/native identity, numeric input domain, expected
mutated output, independent oracle/provider, pilot fixture/output, tolerance, fidelity/execution decision,
final capability owner와 remaining blocker를 공개한다. provider, expected numeric value, pilot 또는
tolerance가 없는 행은 `null`/empty와 `BLOCKED`로 남겼으며 값을 추측하지 않았다.

### 현재 수치와 경계

- module decision: `370 READY_FOR_HANDLER / 29 BLOCKED`
- distribution decision: `626 READY_FOR_HANDLER / 3 BLOCKED`
- resolved Source blocker: 0/29
- capability grant: 0
- ownerless blocker: 0
- silent fallback: 0
- Product admission: false

따라서 이 commit은 Source evidence frozen review 후보일 수 있지만 Source actual-output execution
readiness PASS가 아니다. 29개 행의 provider/pilot/expected output/tolerance가 actual oracle로 닫히기
전에는 final materializer, Playback 또는 renderer 구현으로 진입할 수 없다.

### 자동 검증

- focused unit/coordinated-reseal mutation: 29/29 PASS
- builder deterministic deep `--check`: PASS
- independent shallow/deep oracle: module 370/29, distribution 626/3, ownerless 0 PASS
- current Engine/EFGame script package, EFEngine/LOSTARK binary identity, 14 script class serial,
  7 wrapper의 RAX→fifth argument→SpawnEx dataflow PASS
- focused shallow/deep ProjectAudit: PASS
- 전체 `Invoke-ProjectAudit.ps1`: 실행했으며 이 lane의
  `effect.artist-31470-custom-handler-oracle`은 PASS했다. 전체 audit은 source-contract의 기존
  publisher v14 expectation, geometry harness binary 부재, map/resource, G09, four-class rollout 등
  소유 범위 밖 14개 통합 항목으로 exit 1
- current-only fidelity의 `SOURCE_EXACT` 승격, fabricated provider/pilot/output/tolerance,
  final owner 제거, wrapper dataflow/RandomSeedInfo child 변경, blocker/Product 제거 coordinated reseal:
  모두 reject
- 이미지, 스크린샷, GPU 화면, 육안 판정: 실행하지 않음

v2 receipt LF-canonical text SHA-256은
`c436e69e40e7ad13079940e9ed88d1522942c10a135e19c1d19b020e86b797ac`, self SHA-256은
`0da627b3ed5b100014f2a2ac1fa3591d861c6a241befee65ca856b406dedaadc`다.

## 2026-08-10 R0-S3 source actual-output provider acquisition 결과

최종 계획 정본 `7ffb8a3bf123703ea451cbe53a178f449f102fbe`를 기준으로 frozen Source corrective
`c927e397811d4e5718efd27b187eb59775023685`에서 별도 acquisition worktree를 만들었다. 기능 runtime,
Playback, renderer, shared C++는 수정하지 않았다.

### blocker를 native family로 축약한 결과

- Source blocker occurrence: 29/29
- exact Source class: 15
- native family cluster: 7
  - UE3 standard seeded Spawn→base SpawnEx: 7 class / 11 occurrence
  - EF LocationOnGround: 1 / 2
  - EF CylinderSpin seeded+unseeded: 2 / 5
  - EF Decal TypeData: 1 / 3
  - EF PointLight TypeData: 1 / 1
  - EF VelocityOverLifetime: 1 / 4
  - EF custom vector-multiply particle parameter evaluator owner: 2 / 3
- source-era actual-output provider: 0
- standalone actual mutated-output pilot: 0
- resolved blocker delta: `29 -> 29`, resolved 0
- ownerless blocker: 0

각 class row에는 29개 stable occurrence ID, required mutated output, final owner, provider/pilot/tolerance와
decision을 보존했다. family grouping은 조사 단위를 7개로 줄였지만 29개 행을 READY로 합치지 않는다.

### 접근 가능한 획득 경로 조사

- C:가 유일한 filesystem volume이었다. `C:/Users/user`, Program Files, ProgramData와 현재 설치 외
  exact target scan에서 source-era `EFEngine.dll`, `LOSTARK.exe`, logical `Engine.u`/`EFGame.u`를 찾지
  못했다. 현재 설치는 `LOSTARK.ver 3.22.2.3,1546`, `EFEngine.dll 1.0.8767.0` 한 revision뿐이다.
- Desktop backup/resource root 36 ZIP과 1,813 archived UPK, Recycle Bin, OneDrive, 접근 가능한 USN
  journal에는 native/script provider가 없었다. FileHistory와 Windows.old는 존재하지 않았다.
- VSS inventory는 비관리자 token에서 `vssadmin`/`Win32_ShadowCopy` 접근 권한이 없어 확인하지 못했다.
  이를 `PERMISSION_UNCHECKED`로 기록했으며 exhausted로 계산하지 않았다. 권한 상승, snapshot mount,
  recovery와 쓰기는 수행하지 않았다.
- local/remote 79 heads/tags와 pre-purge bundle, GitHub Actions artifact 0/cache 0, Release 0,
  user container package none, local LFS 1,546 object/728,803,042 bytes를 확인했다. LFS는 PE 21/UPK 0/
  game-native 0이며 19,818,496-byte `b367…` candidate는 Assimp v6.0.4였다.
- unreachable UE3 candidate `7d19e7036aee09d6502f6f1527425aa3d52853ff`, 5,303,178 bytes,
  raw SHA-256 `f130d3d0b4a048832885cc0d13eb8a88b7d6eb0b1a46dba6181c4d1cd3c4239f`는
  Ver868/16 Engine12097의 cooked Level/map이었다. ShaderCache/Material/MIC/ParticleSystem asset export가
  0이므로 native/script output provider가 아니다. 나머지 큰 unreachable object도 gameplay SQLite,
  mapplacement와 opaque numeric buffer였으며 provider가 없었다.
- refs-outside blob 6,235개 중 sub-1MiB 5,980개/379,505,715 bytes도 전부 stream 분류했다.
  LFS pointer 1,232/other 4,748이며 PE/UE3 UPK/ZIP은 0이다. pointer tree 798 entry와 local store에
  실제 존재하는 unmapped pointer target 757개/137,947,876 bytes도 PE/UPK/ZIP 0이었다. 유일한 EXE
  이름은 기존 project tool `ModelAssetConverter.exe`였으므로 accessible Git object gap도 닫혔다.

### source package revision과 current callable 경계

- `FX_CM_01`, `FX_CM_02`는 source/current package identity가 같고 source archive exact copy가 각각
  3개다. `FX_PC_SDM_00`, `FX_PC_SDM_05`는 current와 revision이 다르지만 source exact copy가 각각
  3개다. `FX_PC_SDM_07` source identity는 66,494 bytes / `94398893…`, current는 66,557 bytes /
  `5a052189…`이며 source exact package copy는 0이다.
- package input의 존재는 native `Spawn/Update/GetValue` 구현이 아니다. 특히 package mismatch가 있는
  current runtime capture는 `SOURCE_EXACT`로 사용할 수 없다.
- current `EFEngine.dll`에는 표준 seeded wrapper 7개가 addressable하지만 standalone 호출에 필요한
  live object/emitter/world/distribution graph fixture가 없다. EF custom exact exported entry와 custom
  distribution exact `GetValue` entry는 0이다. live game injection/hooking은 current-only이고 독립적이지
  않으며 안전한 controlled source-era oracle도 아니므로 수행하지 않았다.

### 외부 artifact intake와 최종 진입 판정

`SOURCE_EXACT`를 다시 열 수 있는 최소 입력은 다음 둘 중 하나다.

1. 동일 revision의 `EFEngine.dll`, `LOSTARK.exe`, logical `Engine.u`, logical `EFGame.u`, target UPK 전체와
   single-revision identity manifest
2. 위 binary/package identity를 증명하고 fixed seed/time/world/parameter input과 pre/post full numeric
   particle/component state를 제공하는 authenticated source-era capture

두 대안 모두 raw bytes/length/SHA-256/version/signature, 29 occurrence→15 exact class→7 native family
mapping, expected mutated output과 absolute/relative tolerance가 필요하다. current-only identity, input digest,
wrapper dataflow, class 이름/상속, 이미지·스크린샷과 출처 없는 수치는 거부한다.

현재 결론은 다음과 같다.

- acquisition evidence: `PASS_ACCESSIBLE_SCOPE_EXHAUSTED` (VSS 1개는 permission-unchecked)
- Source execution readiness: `BLOCKED`
- Source exact readiness: `0/29`
- R2/final materializer/Playback/renderer 진입: `NO-GO`
- reconstructed-numerically-verified 최대 복원 분기: `NOT_STARTED`, 사용자 명시 승인 필요
- Product admission: false

이는 구현 실패를 숨긴 타협 완료가 아니다. accessible provider가 없다는 사실과 외부 입력 계약을
동결한 R0 결론이며, source exact artifact가 들어오거나 사용자가 별도 reconstructed fidelity 분기를
승인하기 전에는 35/35 또는 runtime 복원 완료를 주장하지 않는다.

### 자동 검증

- acquisition unit/coordinated-reseal mutation: 21/21 PASS
- deterministic builder `--check`: PASS
- 기존 custom handler shallow/deep independent oracle: module 370/29, distribution 626/3,
  actual output oracle 0, Product false PASS
- focused acquisition shallow/deep ProjectAudit: classes 15, native families 7, blocked 29,
  provider/pilot 0, VSS permission-unchecked, next stage NO-GO, Product false PASS
- 전체 `Invoke-ProjectAudit.ps1`: exit 1. 신규
  `effect.artist-31470-source-oracle-acquisition`은 실패 목록에 없고 PASS했다. 전체 audit은 map/runtime
  resource, project data visibility, G09 codec/runtime, 기존 Source publisher expectation과 nested stderr,
  geometry harness binary, WFX/representative/four-class rollout, Character resource 등 이 lane 밖 기존
  통합 항목 14개가 남아 실패했다.
- JSON strict load/self hash/input identities, class/family denominator, VSS status, current-only laundering,
  fabricated provider/pilot, fidelity/reconstruction approval, artifact intake, next-stage/Product coordinated
  reseal mutation: 모두 reject
- 이미지, 스크린샷, GPU 화면, 육안 판정: 실행하지 않음

acquisition receipt self SHA-256은
`c49f4dfcabc09c765b2127c620c5dbc8676d18819de008eb813e583b3f07e98d`, raw file SHA-256은
`05d8c90b2000fbc6aa699f63805784b2013a700aa94ee5e85c4d4a30459cafbc`다.
