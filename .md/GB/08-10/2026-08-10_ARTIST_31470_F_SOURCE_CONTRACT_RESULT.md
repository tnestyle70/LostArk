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
- 14 unique는 payload와 ParticleParameter 실행 필드를 decode했다.
- 원본 package revision이 없는 1 unique / 2 Mesh occurrence는 unresolved로 남겼다.
- PointLightComponent 1 unique / 1 occurrence는 decode했다.
- 이 closure의 compiled execution allowed reference는 0이다.

### WModel과 scale

7개 Mesh carrier에서 glTF와 현재 WModel의 position, normal, tangent XYZ, UV0,
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
- renderer type과 source space의 명시적 저장
- source graph/closure/material/evidence/local-reference/geometry receipt SHA pin
- selected LOD identity와 ordered module reference provenance
- cue source position, cue local transform, parameter override, transform composition order
- module/property coverage와 blocker
- material/geometry/compiled-execution admission
- ParticleParameter의 `actionCue`/`none` binding 보존
- v13 JSON 또는 in-memory 문서가 v14 evidence field를 싣고 무음 소거하는 우회 차단
- Candidate를 Catalog, Publisher, Playback, Renderer에서 계속 차단

공용 `Effect_Playback`, `Effect_DocumentRenderer`, HLSL, `EffectCatalog`, Effect Tool은 이
슬라이스에서 수정하지 않았다.

## 산출물 identity

- candidate file SHA-256:
  `7f4d6827fac6eab2e58d43e72c401d92bda1462e344bc5b4f16245d09dbb50be`
- receipt file SHA-256:
  `f69e1cddcd176cfcda24033a4fef64de6a4c2b5c46238564ff4bb6f10a3aa6a7`
- registry file SHA-256:
  `49ff19a8a30db7ffa2573141f4e2afbe76e743f30de917a0b819b76a59217b81`
- registry contract SHA-256:
  `28f75bc5b8e1f56be42d962636c7dd43c932892036655ef3a414392115c252f4`
- generated header SHA-256:
  `f4d3f92154c31b99c39d379dff9cfbecd384c55a85bf417a470ad9e236173bc7`

## 자동 검증

- `Test-Artist31470SourceContract.ps1`: PASS
- generator `--check`: PASS, checked-in output byte identity
- focused Python tests: 28/28 PASS
- inventory/material/action/socket 추가 tests: 13/13 PASS
- changed JSON parse: 16/16 PASS
- ClientFrontendHarness Debug `--effect-source-contract`: 5/5 PASS
- ClientFrontendHarness Release `--effect-source-contract`: 5/5 PASS
- Product publisher 지원 version: 5~12 유지
- EffectCatalog candidate reference: 0
- `git diff --check`: PASS, line-ending warning만 존재

빌드는 컴파일·링크 오류 확인에만 사용했다. 이미지, 스크린샷, GPU 화면 판정은 수행하지 않았다.

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
