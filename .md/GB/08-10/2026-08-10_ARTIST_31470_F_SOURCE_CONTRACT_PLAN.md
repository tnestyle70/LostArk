# 2026-08-10 Artist 31470 F Track A0 Source Contract Plan

## 목표

도화가 F `31470 / 필법 : 한획긋기 / sdm_sk_onestroke`의 UE3 Cascade 근거를
Track A renderer가 소비할 수 있는 typed Source Contract로 고정한다. 이 변경 단위는 원본을
실행하거나 Product로 게시하지 않는다. 종료선은 다음 다섯 항목이다.

1. source package, action cue, emitter/LOD/module, material, WModel/DDS의 분모와 partial closure
2. runtime v13과 분리된 v14 `source_contract` schema와 Codec 검증
3. machine-readable renderer/module/property registry와 생성 헤더
4. 7 cue / 35 element 숫자 golden candidate
5. Product, Catalog, runtime, visual approval 차단 receipt

## 현재 코드와 데이터 경계

- WModel은 geometry-only다. Cascade occurrence나 material override를 WModel에 넣지 않고
  `.wparticle` 같은 두 번째 model runtime도 만들지 않는다.
- `EFFECT_AUTHORING_FORMAT_VERSION`은 계속 13이다. v14는
  `EFFECT_SOURCE_CONTRACT_FORMAT_VERSION`이며 `purpose=source_contract`만 허용한다.
- `Load/Parse`는 v14를 `Validate_SourceContract`로 검증할 수 있다. 일반 `Validate`,
  `Validate_Drawable`, Playback, Renderer, Catalog, Publisher는 v14를 실행하지 않는다.
- Imported `skill.31470.source-receipt.json`의 원본 슬롯 `R`은 변경하지 않는다. 현재 게임의 F
  매핑은 Derived action/inventory/candidate receipt에 `F`로 기록한다.
- material identity 28개 중 27개는 `COOKED_PARTIAL`, Light 1개는 non-render builtin이다.
  Source Contract 안의 runtime source material profile은 모두 disabled다.
- 이 슬라이스의 `913/913`은 선택 FIRST_LOD external module reference 분모다. non-selected LOD,
  external native tail, local distribution target, class default, WModel tangent/COLOR_0/bounds가
  모두 닫혔다는 뜻으로 사용하지 않는다. aggregate evidence는 blocker가 남아 있는 동안
  `SOURCE_EVIDENCE_PARTIAL`이다.

## typed renderer inventory

활성 action cue 7개가 선택하는 base target은 정확히 35개다.

| renderer | count | Source Contract 의미 |
|---|---:|---|
| `MeshParticle` | 13 | WModel geometry와 Cascade mesh occurrence |
| `SpriteParticle` | 16 | billboard/SubUV sprite occurrence |
| `DecalParticle` | 3 | TypeDataDecal occurrence와 signed source projection 근거 |
| `CascadeRibbon` | 1 | TypeDataRibbon occurrence; AnimTrail과 별도 |
| `LightParticle` | 1 | 활성 Light occurrence; inherited defaults는 unresolved |
| `ScreenPost` | 1 | ZoomBlur source occurrence; runtime shader 미구현 |

비활성 action variant에서 제외한 legacy element는 93개다. 128행 전체 inventory를 base F 한 번의
Complete Effect로 합치지 않는다.

## 파일 역할

- `Client/Public/Effect_AuthoringDocument.h`: v13 runtime과 분리된 renderer/source-space/coverage
  descriptor를 선언한다.
- `Client/Public/Effect_Distribution.h`: source ParticleParameter의 `actionCue`/`none` binding을
  lossless하게 보존한다.
- `Client/Private/Effect_DocumentCodec.cpp`: v14 strict parse, source-only validate, serialize를
  소유한다. runtime validation은 v14를 거부한다.
- `Data/Effects/Contracts/ue3-cascade-source-v1.registry.json`: 실제 35개 element에서 관측한
  renderer/module/property storage와 literal kind의 machine-readable 정본이다.
- `Client/Public/Generated/Effect_SourceContractRegistry.generated.h`: Codec가 registry identity와
  shape를 검증하는 생성 헤더다.
- `Tools/LevelPlacementExtractor/build_artist_31470_source_contract.py`: source closure를 선택하고
  candidate, receipt, registry, generated header를 원자적인 생성 단위로 만든다.
- `Tools/ProjectAudit/Test-Artist31470SourceContract.ps1`: 생성 byte identity, 수치, R/F 경계,
  Catalog/Product 차단을 검사한다.

## 구현 순서

1. raw action과 skeletal socket을 재현 가능한 extractor로 닫는다.
2. normalized graph, external module closure, active material closure를 repo 데이터로 pin한다.
3. base-enabled cue만 선택해 35개 occurrence와 93개 제외 행을 만든다.
4. renderer와 source space를 명시하고 모든 source module/property에 coverage를 한 번씩 부여한다.
5. registry canonical SHA를 candidate의 모든 element와 receipt에 pin한다.
6. Codec는 v14 Source Contract를 읽되 runtime/Product 경로가 소비하지 못하도록 fail-close한다.
7. generator `--check`, focused unit test, source-only Debug Codec build/load, ProjectAudit를
   실행한다. Client 실행과 시각 검증은 하지 않는다.

## 이 변경에서 금지하는 작업

- `Effect_Playback`, `Effect_DocumentRenderer`, HLSL, Effect Catalog, Effect Tool 변경
- Mesh/Sprite size, seed, SubUV 동작을 v13 Product에 전역 적용
- Publisher의 v14 admission, Product/Assembly/Component 생성, `PlayerSkills.effectId` 연결
- WModel converter, vertex payload, culling/bounds 변경
- Decal/Ribbon/Light/Post의 실행·shader parity 완료 주장
- 수동 화면 검증 전 `VISUAL_APPROVED` 또는 복원 완료 표기

## 완료 조건

- generator와 checked-in 네 output은 UTF-8 EOL-normalized `--check`에서 동일하다.
- source receipt는 R, Derived contract는 F다.
- renderer count는 `13/16/3/1/1/1`, active 35, excluded 93이다.
- Debug source harness가 후보를 Load하고 `Validate_SourceContract`를 통과한다.
- Publisher는 계속 version 5~12만 허용하고 Catalog에 후보 ID가 없다.
- Playback/Renderer/HLSL/Catalog/Tool의 scoped diff는 0이다.
- candidate 상태는 `SOURCE_EXTRACTED`, aggregate evidence는 `SOURCE_EVIDENCE_PARTIAL`,
  assembly는 `MANUAL_MASTER_ASSEMBLY_PENDING`, visual은 `NOT_VISUAL_APPROVED`이며
  runtime/Product admission은 false다.

## 2026-08-10 distribution/default closure 보강

이 재개 단위는 Source Closure만 소유한다. WModel/scale, Material IR, compiler executor,
Playback/renderer, Effect Tool, Catalog/Publisher 구현은 바꾸지 않는다.

1. 15 distribution definition과 17 occurrence, PointLight definition/occurrence를 분리한다.
   각 occurrence는 stable `referenceId`, `definitionId`, `occurrenceId`, 원본 module
   reference order와 property identity를 보존한다.
2. UE3 값 선택은 property별로
   `INSTANCE_EXPLICIT -> NESTED_ARCHETYPE_TEMPLATE -> CLASS_CDO ->
   PARENT_CDO_HIERARCHY -> EVALUATOR_DEFAULT` 순서만 허용한다.
3. package identity receipt pin, pinned payload decode, 현재 exact physical source package를
   각각 별도 축으로 기록한다. identity가 없는 external closure record와 current/recovery
   package는 `SOURCE_EXACT`로 승격하지 않는다.
4. semantic-blocked distribution은 executable payload를 `UNRESOLVED` variant로 scrub하고,
   admission gate가 payload reader보다 먼저 실패하게 한다. raw decoded evidence와 current
   default evidence는 closure에 별도로 남긴다.
5. v14 SourceRecipe에는 distribution별 fidelity/admission과 18개 local-reference binding,
   property blocker를 typed public field로 둔다. v13 field smuggling은 Codec이 거부한다.
6. blocker token은 occurrence/property/module/element/receipt/registry/Product로 집합 포함
   전파한다. admission은 blocker 집합이 비었을 때만 true이며 이 slice에서는 Product가
   계속 false다.
7. 외부 graph/UPK/script는 raw SHA-256, repo-tracked derived UTF-8 JSON은 LF canonical SHA-256,
   generated JSON/header `--check`는 EOL-normalized compare를 사용한다. geometry generator는
   별도 lane 소유이며 이 slice는 tracked geometry receipt link까지만 검증한다.

검증은 production pure resolver/binder mutation test, generated round-trip, v13 rejection,
source shallow/deep audit, JSON parse와 `git diff --check`로 닫는다. 사진, 스크린샷,
Client 실행은 하지 않는다.
