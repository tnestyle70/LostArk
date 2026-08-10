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
4. semantic-blocked distribution은 admission gate가 payload reader보다 먼저 실패하게 하고
   transport를 `UNRESOLVED` variant로 유지한다. transport의 inert numeric shape를 source 값이나
   evaluator oracle로 소비하지 않으며 raw decoded evidence와 current default evidence는 closure에
   별도 fidelity로 남긴다.
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

## 2026-08-10 G01/G05-S code-only semantic closure checkpoint

최종 candidate/receipt/registry/header를 다시 생성하기 전에 다음 pure code 경계를 먼저 고정한다.

1. `artist_31470_source_semantic_closure.py`는 frozen input 다섯 개의 canonical SHA를 검증하고
   35 selected LOD, 399 ordered module, 1,434 top-level property, 1,572 primitive leaf,
   629 distribution을 stable occurrence ID로 열거한다.
2. 모든 행은 `sourceFidelity`, `artifactBindingIntegrity`, `executionAdmission`,
   `productAdmission`과 네 blocker set을 독립적으로 가진다. handler receipt와 독립 numeric
   oracle이 없는 구조 decode를 `EXECUTION_CONSUMED`로 표기하지 않는다.
3. 612 inline distribution과 17 local-reference occurrence는 510 stable definition/reference ID와
   629 stable occurrence ID를 가진다. lookup/target이 없는 137행은 class-default 값을 0으로
   만들지 않고 `DISTRIBUTION_CLASS_DEFAULT_VALUE_UNRESOLVED`로 차단한다.
4. operation reconstruction 409행, lookup chunk/count reconstruction 각 257행, explicit random
   operation 82행을 source field와 분리한다. reconstruction fidelity는 `RECONSTRUCTED_GRAPH`이며
   numeric execution evidence는 계속 `UNVERIFIED`다.
5. external native-tail 248, seed 14, Required local-space default 8, Decal 3, Ribbon 1,
   ScreenPost 1, Light 1, selected-LOD Level/Enabled 70 field와 PointLight exact/current field를
   dedicated row로 두고 첫 property에 blocker를 몰아넣지 않는다. LOD와 PointLight field의
   실행 blocker는 module aggregate, opt-in coverage, compiled/receipt/registry admission까지
   집합 포함으로 전파한다.
6. raw `Module.strClassName`은 `exactSourceClass`에 그대로 저장한다. 승인 alias table은 비어
   있으며 `efparticlemodule` prefix와 `_seeded` suffix를 자동 alias로 만들지 않는다.
   따라서 26 module occurrence는 exact class 전용 handler/alias 근거가 생길 때까지 실행을 막는다.
7. `verify_artist_31470_source_semantic_closure.py`는 builder를 import하지 않고 raw tree에서
   분모, ID, class lineage, fidelity, family별 blocker axis/admission/integrity,
   native/default/seed/Light 결정을 다시 계산한다. self hash를 다시 봉인한 axis 이동이나
   fallback evaluator/oracle mutation도 거부한다.
8. 기존 Source generator는 `--source-semantic-closure` opt-in에서 이 oracle을 먼저 실행하고,
   temp candidate의 399 module coverage에 `exactSourceClass`와 `aliasId`를 모두 기록한다.
   module coverage blocker는 검증된 artifact/execution axis 합집합을 보존한다. opt-in이 없는
   checked-in `--check` 결과는 바꾸지 않는다.

이 checkpoint의 합격 주장은 pure resolver/binder/oracle과 temp-output mutation PASS까지만이다.
399/1,434/1,572/629 실행 consumption과 G05-S numeric reconstruction 완료는 주장하지 않는다.
Geometry/Material Gate dependency를 결합하고 class-lineage Codec conflict를 해결한 뒤 Source owner가
최종 네 output을 한 번만 재생성한다.

## 2026-08-10 G05-S source execution semantics adapter

기존 semantic closure의 `UNRESOLVED` 행을 곧바로 runtime admission으로 바꾸지 않고, source
evidence와 runtime compiler 사이에 pure typed adapter receipt를 하나 둔다.

1. 입력은 checked-in semantic closure, v14 candidate/receipt, local-reference closure,
   external-module closure, ActionCue recipe 여섯 개다. JSON 의미 hash와 generator/tool의
   LF-canonical text hash를 receipt에 기록한다.
2. 35 occurrence의 399 module, 1,434 top-level property, 1,572 primitive leaf, 629 distribution을
   candidate typed literal/distribution payload와 stable ID로 1:1 결합한다. payload 이후 raw
   source 문자열을 다시 검색하지 않도록 module/property/leaf별 handler capability ID를 낸다.
3. selected LOD의 Level/Enabled 70 field는 `FIRST_LOD_ONLY` source identity가 adapter build에서
   이미 선택되므로 `VERIFIED_IRRELEVANT`로 닫는다. `lodvalidity`와 editor-only
   `b3ddrawmode`도 별도 irrelevance oracle을 사용한다.
4. 612 inline distribution은 source tagged field와 reconstruction field를 분리하고, current UE3
   raw-distribution default와 cooked lookup shape로 operation/chunk/count를 재계산한다. fixed
   time/random vector 세 세트의 independent numeric oracle을 저장하되 reconstructed field를
   `SOURCE_EXACT`로 승격하지 않는다.
5. local distribution 17 occurrence는 instance -> archetype -> CDO -> evaluator default 순서를
   보존한다. standard parameter/curve 14 occurrence는 ActionCue type/name을 결합해 direct,
   normal, constant fallback 또는 curve 값을 계산한다. custom EF multiply 3 occurrence는 current
   payload를 진단용으로 보존하되 exact evaluator oracle이 없어 계속 `BLOCKED`다.
6. external module 248 occurrence는 installed current package의 178 unique export를 다시 decode해
   `serialSize == propertyStreamEnd`를 검사한다. 결과는 native tail의 current-revision
   irrelevance 증거이며 historical `SOURCE_EXACT` 증거가 아니다.
7. seed 14 occurrence는 source array 또는 32-byte struct body에서 int32 seed를 보존한다. Engine/
   EFGame current seeded CDO 8종을 직접 decode해 공통 seed selection policy를 검증한다. source-era
   script identity는 pin되지 않았으므로 current reconstruction으로만 기록한다.
8. Required local-space 8, Decal 3, Ribbon 1, Light 1은 current CDO/default chain을 typed value로
   물질화한다. ScreenPost 1은 implicit source field가 없는 renderer projection이므로
   `VERIFIED_IRRELEVANT`다. PointLight exact child와 current default field는 분리하며 GUID 두 개는
   runtime irrelevance oracle로 닫는다.
9. `efparticlemodule*` 및 `_seeded` exact class 26 occurrence/13 class family는 alias하지 않는다.
   exact handler numeric oracle이 없으면 module blocker를 유지한다. custom distribution을 소유한
   standard module 3개도 함께 차단한다.
10. 생성기 `--check`, builder를 import하지 않는 독립 verifier, self-reseal mutation suite,
    shallow/deep ProjectAudit를 통과해야 한다. Product admission은 항상 false이며 이미지,
    스크린샷, 육안 판정을 완료 조건으로 사용하지 않는다.

이 adapter의 합격 범위는 source-side typed input과 evaluator receipt다. runtime compiler가 capability를
결합하고 custom handler/evaluator oracle을 제공하기 전에는 35/35 실행 또는 Product 복원 완료를
주장하지 않는다.
