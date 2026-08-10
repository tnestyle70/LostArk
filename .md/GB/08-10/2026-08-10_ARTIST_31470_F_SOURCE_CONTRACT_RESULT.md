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
  `source_decoded 13 / deterministic_conversion 108 / metadata_only 2 / unresolved 276`이다.

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
- semantic-blocked distribution의 evaluator 입력 scrub와 admission-before-payload 검증
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

## 산출물 identity

아래 file SHA는 repo-tracked UTF-8 text의 LF canonical hash다.

- candidate file SHA-256:
  `b88a37f569c8bbbbab323e1c4257b26b11a95ff0e02456b77d54f1147fda3914`
- receipt file SHA-256:
  `832fe22fae20568166c430dacfacc579fb60b5f53e685dfae6c3746e8dc44be6`
- registry file SHA-256:
  `bb59cbb547e6b6916c4a86f146a708e11e97cc3238d04126b8c4c15204d28014`
- registry contract SHA-256:
  `88d801cdb94064b981d6fd629406e4d999521be0356777f136e99171ef922b1d`
- generated header SHA-256:
  `e6d971e550d7d8425e3d2699983d5dcc862eb04c6474f979f1718e46d951073f`

## 자동 검증

- `Test-Artist31470SourceContract.ps1` shallow source slice: PASS
- 같은 script의 `-DeepSourceAudit`: closure/evidence/contract `--check` PASS
- focused source/extractor Python tests: 65/65 PASS
- `Tools/LevelPlacementExtractor` 전체 discovery: 263/263 PASS
- changed JSON parse: 5/5 PASS
- ClientFrontendHarness Debug compile: PASS
- ClientFrontendHarness Debug `--effect-source-contract`: 15/15 PASS
- v14 serialize round-trip, v13/in-memory field smuggling, reference ID/order mutation,
  admission-before-payload, evaluator scrub, blocker laundering 반례: PASS
- Product publisher 지원 version: 5~12 유지
- EffectCatalog candidate reference: 0
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
