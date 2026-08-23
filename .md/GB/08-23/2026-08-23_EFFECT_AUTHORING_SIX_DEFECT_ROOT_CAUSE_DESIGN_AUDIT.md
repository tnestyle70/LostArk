# 2026-08-23 Effect Authoring 여섯 결함 원인 감사·실행 설계

작성 기준:

- base: origin/main 59c9c082c2deddae2c4839341000600b9d071361
- 계획 branch: codex/effect-tool-six-defect-plan
- 상태: 실코드·데이터·독립 비평 완료, 구현 시작 전
- 수동 화면 판정: 사용자 전용

이 문서는 다음 여섯 요구를 제품 경로에서 닫기 위한 root-cause/design audit다.
`.md/GB/local.md`가 `*_PLAN.md`에 요구하는 최종 H/CPP 전문은 아직 쓰지 않는다. G00의
각 domain 증거와 아래 선택 gate가 고정된 뒤, 서로 독립 배포 가능한 G별 `*_PLAN.md`에
정확한 삽입 블록과 최종 전문을 작성하고 나서 구현한다.

1. 다른 Effect의 Element를 현재 Effect로 독립 복제한다.
2. Sprite 회전과 source particle 이동 방향을 분리해 편집한다.
3. 차원술사 A의 네 검격 occurrence를 전체 animation 시간에 맞춰 복원한다.
4. 도화가 R의 바닥 symbol LocalDecal과 중심 소멸 연출을 복원한다.
5. 차원술사 BA1의 정확한 약 0.2초 timing과 BA1~4 stage 연결을 복원한다.
6. 워로드 방패 mesh가 80도 조준 방향으로 이동하고 보이는지 방향과 cull을 분리해 수정한다.

## 0. 먼저 고정할 결론

여섯 문제는 하나의 shader 버그가 아니다. 소유 계층이 네 종류로 나뉜다.

| 계층 | 문제 | 정본 |
|---|---|---|
| Effect authoring transaction | Import/Copy Element | Effect Tool + authored Effect document |
| Particle simulation | 이동 방향 편집 | SourceRecipe를 소비하는 EffectPlayback |
| Effect composition/material | 차원술사 A, 도화가 R | occurrence group + Material Program/Layout/Descriptor/Adapter |
| Action presentation timeline | 차원술사 BA, 워로드 방향 | Server stage edge + skillbinding + animevent + action-facing root |

구현은 기존 경로만 확장한다.

- 두 번째 Effect loader, 두 번째 particle runtime, Client-local combo clock을 만들지 않는다.
- Product effect는 EffectCatalog와 effectref=asset cue를 계속 사용한다.
- Server가 combo stage와 action edge를 소유한다.
- Material tuple은 carrier만 같다는 이유로 기존 opcode를 재사용하지 않는다.
- Client와 Effect Tool의 화면은 에이전트가 실행·판정하지 않는다.

shader 파일의 역할도 미리 고정한다.

- 재사용 가능한 pixel material 계산식과 packet 검증은 `.hlsli`에 둔다.
- carrier의 VS/PS entry, technique/pass, raster/depth/blend dispatch가 바뀔 때만 `.hlsl`을 바꾼다.
- Program 식이 이미 `.hlsli`에 있고 carrier branch도 존재하면 JSON/registry/CPU ABI만 닫고 shader를 중복 작성하지 않는다.

최신 main의 PR #178은 이 판단을 한 단계 전진시켰다.

- StandardColorV1/opcode 1, 6 Layout, 8 compiled carrier Adapter와 131 Binding이 추가됨
- Sprite/Mesh/LocalDecal의 실제 draw 배선은 대표 4스킬 V1 audition으로 자동 검증됨
- 131행은 모두 PROJECT_TUNED_APPROX이며 SOURCE_EXACT는 0
- V1은 REGISTRY_BOUND_AUDITION_ONLY이고 active Product cue 승격은 publisher가 거부함

따라서 이 설계는 이미 닫힌 StandardColor carrier/ABI를 다시 만들지 않는다. 이를 진단 control로
재사용하되, 원본 material 식 복원이나 현재 Product 연결까지 끝났다고 간주하지 않는다.

이번 작업의 완료식은 다음이다.

    Product closure
    = input/Server edge
    × skillbinding clip
    × animevent cue
    × runtime EffectCatalog
    × occurrence cohort
    × resource/material tuple
    × renderer draw

한 항이라도 0이면 JSON 행이 있어도 제품 화면은 0이다.

## 1. 실제 Product 분모

### 1.1 Import/Copy의 첫 소비 사례

- source: 워로드 T 풀배럴 캐넌 계열의 cone/screw mesh Element
- target: 워로드 E 대쉬 어퍼 파이어 Effect
- 선택 단위: texture나 vector index가 아니라 source effectAssetId + stable elementId
- 결과 단위: target effectAssetId + 새 stable elementId

실제 donor는 Effect Tool에서 source Effect를 열고 stable ID를 선택한 뒤 고정한다. cone 또는 screw라는 이름만으로 자동 선택하지 않는다.

### 1.2 창술사 particle 방향

- skill: 34110 반월섬, D, LANCE_MASTER_LONG_SPEAR
- Product document: Data/Effects/Authored/effect.lancemaster.skill.34110.unified.effect.json
- material-only control: effect.lancemaster.skill.34110.v1.unified, REGISTRY_BOUND_AUDITION_ONLY
- texture: Effect/LanceMaster/Textures/fx_a_fragment_001.dds
- 동일 texture를 쓰는 row: 8개
- 짧은 row: d6a31b4ae13598e5067ca575, 96fd6a1f328f38e8ed4e9872, bedc716599fe37caab055e58
- 연속 row: aa9446a4dffd00d43183729d, c1c45fcb669bf12e1fddc8b4, fc70a03a6c5b1d946cbed7ff, fdb6b3c6b5200d9e83830288, 92310eb44fc2dc56d53ce77a

texture 이름은 occurrence identity가 아니다. 먼저 다섯 연속 row의 실제 velocity를 덤프하고 관찰 대상 stable ID를 고정한다.

### 1.3 차원술사 A

- skill: 2050210
- clip: pc_sp_m_00_sk_sk_willowrend
- Product cue: clip 시작 0ms에 effect.dimensionmaster.skill.2050210.unified 한 번
- source hit time: 0.25, 0.60, 0.90, 1.30초
- imported canonical: 117 rows
- current Product: 12 rows

outer cue 한 번은 맞다. 잘못된 것은 내부 네 occurrence 중 뒤 세 개가 완전한 검격 묶음이 아니라 emitter 한 행씩만 남았다는 점이다.

### 1.4 도화가 R

- skill: 31210 필법: 콩콩이
- 마지막 clip: sdm_sk_skykongkong_02
- Product document: effect.artist.skill.31210.ba4.unified
- target row: decal.artist.31210.ba4.symbol14.v1
- texture: Effect/Artist/Textures/fx_o_symbol_14.dds
- source event time: 0.5333초

row와 Product join은 이미 존재한다. 이 문제를 다시 row append 문제로 다루지 않는다.

### 1.5 차원술사 BA

- skill: 2050010, LMB COMBO
- clips: pc_sp_m_00_sk_att_battle_1_01~04
- intended Effect assets: ba1, ba2, ba3, ba4
- current Product mapping: ba1, ba1, ba3, ba1
- current BA1 binding: playMs 1400 / playRate 2 = wall 700ms

현재 저장소에는 animation 담당자가 승인한 약 0.2초의 exact frame receipt가 없다. 200ms라는 숫자를 추측해 넣지 않는다.

### 1.6 워로드 80도 방패

실측 후보는 다음과 같다.

- S 17040 배쉬: fm_e_shield_001.wmodel 존재
- D 17100 방패 격동: Product authored document에 mesh 0개
- X 17820 전장의 방패: clip1/2 mesh 0개, clip5에 stationary half-cylinder 한 개

가장 강한 후보는 S 17040이다.

- effect: effect.warlord.skill.17040.unified
- element: authored.source-particle.a39dfb1b27ccb2397e53bdd3
- mesh: Effect/Warlord/Meshes/FX_SM_00/fm_e_shield_001.wmodel
- material: fx_e_me_sy_10_2_tr
- render profile: alpha_one_sided_depth_read
- source velocity: 1200cm/s와 velocity-over-life
- Product cue: Warlord.animevents의 wgl_sk_bash, orientation 없음

G00 재현 로그가 다른 skill/element를 지목하면 17040 데이터는 건드리지 않고 분모를 교체한다.

## 2. 이전 수정이 반복 실패한 이유

### 2.1 행 존재를 동작 완료로 판정했다

기존 검증은 다음을 주로 확인했다.

- JSON row가 존재한다.
- timestamp가 네 개다.
- scale이 invertible하다.
- alpha end 값이 0이다.
- Product catalog에 asset ID가 있다.

하지만 다음을 검사하지 않았다.

- timestamp마다 완전한 occurrence cohort가 활성화되는가.
- prepared resource와 material tuple이 닫혔는가.
- renderer가 올바른 carrier/pass를 골랐는가.
- 실제 draw와 nonzero pixel이 발생했는가.
- Server stage clock과 clip wall clock이 일치하는가.
- action yaw가 particle root basis에 적용됐는가.

### 2.2 서로 다른 authority를 번갈아 수정했다

- 차원술사 A는 imported 117행, authored baseline, 수동 24행, Product 12행을 번갈아 수정했다.
- 차원술사 BA는 binding, Server duration, root motion을 한 transaction으로 묶지 않았다.
- 도화가 R은 source material lineage 없이 generic LocalDecal 한 행만 추가했다.
- particle 방향은 SourceRecipe가 velocity를 소유하는데 quad rotation과 Effect-global yaw를 수정했다.

### 2.3 잘못된 결과를 테스트가 정답으로 고정했다

- 차원술사 A test는 total 12 / append 3을 성공으로 고정했다.
- 차원술사 BA의 과거 test는 ba1/ba1/ba3/ba1을 expected로 고정했다.
- 도화가 R test는 row append와 lerp 값만 검사했다.
- Material ledger의 blocker가 남아도 Product draw를 별도로 막지 않았다.

따라서 이번에는 수정 전에 acceptance denominator를 먼저 작성하고, 기존 잘못된 expected를 제거한다.

## 3. 공통 수식과 불변식

### 3.1 Effect occurrence

한 occurrence의 identity는 다음 tuple이다.

    O = (
      effectAssetId,
      elementId,
      carrier,
      program,
      layout,
      descriptor,
      timeline,
      attachment
    )

mesh나 texture만 같아도 O는 같지 않다. Import는 O의 value를 복제하되 destination identity만 새로 발급한다.

### 3.2 animation/effect 시간

    clipWallTime = (sourceTime - sourceTrimStart) / clipPlayRate
    elementWallTime = cueWallTime + elementLocalDelay / effectPlaybackRate

Character가 cue와 함께 clip playRate를 Effect playback에 전달하므로 Effect 내부 delay와 tail도
같은 rate로 압축된다. inner delay를 보존하는 Effect asset을 cue offset에서도 다시 늦추면
double delay다. 반대로 Product cue를 네 번 복제하면 document의 core/tail까지 네 번 중복된다.

### 3.3 particle 이동과 quad 회전

Sprite roll은 quad basis만 바꾼다.

    quadBasis = Billboard(camera) × Roll

이동은 source module이 만든 velocity를 적분한다.

    position(t + dt) = position(t) + velocityEffective(t) × dt

따라서 Roll을 바꿔 velocity를 고치지 않는다.

### 3.4 per-occurrence velocity basis correction

raw particle-root velocity를 vP, particle-root world rotation을 P, 선택 basis world rotation을 B, authored correction을 R, 반전 부호를 s라 한다.

    vB  = vP × P × inverse(B)
    vB2 = s × vB × R
    vP2 = vB2 × B × inverse(P)

- s는 +1 또는 -1이다.
- P/B/R은 rotation-only orthonormal basis다.
- non-uniform scale을 velocity 크기에 섞지 않는다.
- correction은 source velocity에 한 번만 적용한다.
- target-attractor velocity는 target 방향 계약을 유지한 채 별도로 합성한다.

### 3.5 action-facing root

ACTION_FACING이면 sampled anchor의 translation과 scale은 유지하고 yaw basis만 Server action yaw로 교체한다.

    Root = Local × Scale(anchor) × Ry(actionYaw) × Translation(anchor)

alpha와 blend는 이 식의 방향을 바꿀 수 없다. cull은 방향이 맞은 geometry가 카메라에 보이는지만 결정한다.

### 3.6 transaction

모든 authoring 변경은 다음 순서를 지킨다.

    parse source/target
    → validate identities/resources/dependencies
    → allocate new stable IDs
    → stage complete candidate
    → canonical round-trip
    → drawable/prepared preview validation
    → commit active document

실패 시 target document, selection, preview, disk가 모두 기존 상태여야 한다.

## 4. G00 domain별 Product denominator

### 목표

데이터를 바꾸기 전에 여섯 문제의 실제 소비 경로와 수치 실패를 재현한다. 하나의 mega
harness에 Server, Character, Effect, WARP를 mock으로 합치지 않는다. 각 authority의 기존
실행 seam에서 focused baseline을 만들고 마지막에 receipt만 join한다.

### 변경 위치

- Import: codec/Tool transaction focused fixture
- particle와 차원술사 A: EffectPlayback fixed-step frame fixture
- 도화가 R: EffectRenderContractHarness WARP resource/pass/draw/readback fixture
- 차원술사 BA: Product join validator + ServerGameplayContractTests + Client timeline fixture
- 워로드: Product join/action-facing fixture + EffectPlayback transform/cull WARP fixture

EffectRenderContractHarness는 Character, ClientReplication, Server action edge를 소유하지 않는다.
그 계층을 harness에 새 mock runtime으로 복제하지 않는다. 새 compile item이 실제 focused fixture에
필요할 때만 vcxproj와 filters를 같은 변경에서 갱신한다.

### 구조화 trace

각 domain receipt를 합치면 최소 다음 필드를 추적할 수 있어야 한다.

    class, skillId, inputSlot, comboStage
    actionStartTick, actionYaw, sampledOwnerYaw
    clipName, clipWallTime
    effectAssetId, elementId, groupId
    carrier, meshId, textureIds
    materialProgramId, layoutId, adapterId, blockerCodes
    localVelocity, worldVelocity, worldTransform
    selectedPass, cullState, drawCount, nonzeroPixelCount

### 종료 증거

- 차원술사 A: 네 sample time에서 활성 group 누계가 현재 0→1→2→3→4가 아님을 재현
- 도화가 R: row/published resource 존재와 zero/nonzero pixel을 분리 기록
- 차원술사 BA: 현재 Server 700ms, clip wall, internal effect delay/tail과 관찰된 hold를 분리 기록
- 워로드: 0/80/90/180/-90도에서 world velocity와 expected forward의 dot을 기록
- 창술사: fx_a_fragment_001 다섯 continuous row를 stable ID별로 구분

각 G는 자기 focused baseline만 선행 gate로 삼는다. 도화가 pixel probe나 BA timing receipt가
Import/particle 구현을 막지 않는다. 해당 domain 로그가 없으면 그 G의 hardcoded target ID만
승인하지 않는다.

## 5. G01 Import/Copy Element

### 현재 원인

same-Effect Duplicate는 Effect_Tool.cpp의 Try_DuplicateSelectedElement에서 Element value를 깊은 복사하고 새 ID를 발급한 뒤 Try_CommitDocument를 호출한다.

반면 교차 Effect의 Use Selected as New Layer Seed는 Effect_DocumentCodec.cpp의 Build_GenericAuthoredElementStartingCopy에서 다음을 지운다.

- Renderer
- ActionCueAttachment
- TransformInheritance
- SourceRecipe
- SourcePresentation
- source mesh type-data rotation

Create 단계도 SourceRecipe를 다시 지운다. 따라서 기존 seed는 동일 Effect를 복제하는 기능이 아니라 generic migration lowering이다.

### 새 공용 clone 계약

Effect_DocumentCodec.h/.cpp에 direct-authored v13→direct-authored v13 전용 pure stage 함수를
추가한다. source modal은 `!bSourceContract`, unique asset/element identity를 만족하는 ordinary
authored document만 받는다. runtime projection이나 migration reference는 reject한다.

권장 책임 이름:

- EFFECT_INDEPENDENT_ELEMENT_CLONE_REQUEST
- Build_IndependentElementCloneStage

request는 다음 identity를 소유한다.

- sourceEffectAssetId
- sourceElementId
- targetEffectAssetId
- requestedTargetElementId 또는 target ID allocator policy
- requestedTargetGroupId
- dependency policy

복제 대상:

- kind; renderer carrier는 kind/resource/material execution에서 다시 유도
- ResourceBindings
- Material, SourceMaterial, Execution packet
- Detail 전체
- SourceRecipe와 emitter modules
- ActionCueAttachment
- 유효한 dependency closure

새로 발급할 대상:

- destination elementId
- destination provenance
- bound clone이면 registry binding의 destination key와 새 descriptor ID

독립 clone에서 제거할 것은 same-Effect Duplicate와 동일하게 SourcePresentation evidence뿐이다. runtime value를 지우지 않는다.

same-Effect Duplicate와 cross-Effect Import가 공유하는 것은 이 pure value builder다. commit
policy까지 공유하지 않는다. 기존 Duplicate의 partial-draft authoring 의미는 유지하고 Import는
아래 strict transaction만 사용한다.

### dependency 규칙

- TransformInheritance가 있으면 terminal master와 companion closure를 함께 import하고 모든 ID를 remap
- SourceRecipe portable event의 generator/receiver route closure도 별도로 수집하고 remap
- 같은 document Duplicate는 기존 master/route를 참조하고, cross-document Import만 closure를 복제
- closure를 완전하게 지원하지 못하면 field를 clear하지 말고 명시적으로 reject
- closure에 particle-simulation row가 있을 때만 source/target ParticleSystem의 네 semantic field가 다르면 reject
- target anchor/cue가 attachment를 만족하지 못하면 reject
- 외부 asset 파일은 복사하지 않고 Resources-relative ID를 재사용

### Material registry 규칙

Material.Execution은 value copy한다. registry pointer, old effect/element key, mutable `sourceNode`
provenance에서 destination binding을 추론하지 않는다. 현재 registry authority는 explicit
`Fragments/*.material-program-fragment.v1.json` 행뿐이다.

- source가 unbound이면 destination도 명시적 UNBOUND로 두고 inline Execution만 복제
- source가 bound이면 Program/Layout/compiled Adapter ID는 immutable capability로 공유 가능
- source Descriptor는 공유하지 않고 target-owned 새 descriptor ID로 value-copy
- 새 binding은 `(targetEffectAssetId, newElementId)`와 새 descriptor를 참조
- Effect JSON inline mirror와 target fragment candidate가 bit-exact하지 않으면 Import reject
- bound sidecar를 만들 수 없는 상태는 `BOUND_DESTINATION_FRAGMENT_REQUIRED`로 reject하며, unbound clone을 bound 완료로 표시하지 않음

bound source의 same-Effect Duplicate와 cross-Effect Import 모두 target Effect가 소유하는
explicit binding fragment sidecar를 함께 stage한다. publisher가 source
binding을 자동 복제하는 경로를 만들지 않는다. 이후 material/resource 손튜닝도 inline Execution과
destination Descriptor를 같은 transaction에서 갱신한다.

예상 변경:

- Tools/EffectPipeline/build_effect_material_program_registry.py
- Tools/EffectPipeline/test_build_effect_material_program_registry.py
- target-owned `Data/Effects/MaterialPrograms/Fragments/*.material-program-fragment.v1.json`
- Effect Tool의 fragment staging/paired-save codec

### strict commit과 paired save

기존 Try_CommitDocument는 non-drawable draft를 active document에 넣는 의도적 authoring API다.
Import는 이를 직접 호출하지 않고 별도 Try_CommitDrawableImportCandidate를 사용한다.

    모든 dirty draft guard
    → source/target/fragment baseline hash 재확인
    → canonical serialize/parse
    → Validate + Validate_Drawable
    → merged material registry와 inline mirror preflight
    → 기존 preview와 분리된 playback/renderer prepare
    → active document + pending fragment + selection + prepared preview 한 번에 swap

실패 시 active document, dirty bit, selection/isolation, filter, play time, canary state, preview
handle과 disk가 모두 그대로다. 기존 Stage_WorldPreview가 preview object를 먼저 release할 수 있으므로
candidate prepare는 현재 preview와 분리한다.

Save Changes도 기존 Effect JSON 단일 save를 그대로 쓰지 않는다. Effect와 fragment temp를 모두
round-trip/merge 검증하고 두 optimistic baseline hash를 확인한 뒤 journal 기반 paired promote를
수행한다. 두 번째 replace 실패나 process 중단은 다음 실행 recovery에서 두 파일을 모두 old 또는
모두 new generation으로 복원한다. Save I/O 실패 때 메모리 candidate는 unsaved/dirty로 남지만
disk 두 파일은 이전 generation을 유지한다.

### Tool UI

Effect_Tool.h/.cpp에 별도 Import/Copy Element modal을 둔다.

1. target Effect는 열린 상태로 유지
2. source Effect를 별도 staging document로 선택
3. source stable Element를 선택
4. dependency/material/ParticleSystem preflight 결과 표시
5. bound/unbound destination 상태와 sidecar 변경을 표시
6. Import를 누르면 strict 검증된 메모리 bundle만 commit
7. Save Changes에서 Effect+fragment paired-save

m_SourcePreviewDocument나 active document 교체 경로를 import source storage로 재사용하지 않는다.

### 자동 검증

positive:

- resources/material/execution/source recipe/attachment가 bit-exact
- source와 copy를 각각 수정해도 서로 독립
- 새 stable ID와 round-trip save/reload
- same-Effect Duplicate와 cross-Effect Import가 같은 clone helper 사용
- source event route closure remap
- bound donor의 새 descriptor/binding과 merged-registry inline bit-exact

negative:

- duplicate target ID
- invalid/missing asset
- source identity mismatch
- malformed/unsupported source version, source==target routing, source TOCTOU
- dangling/cyclic inheritance
- missing/cyclic portable event route와 group-ID collision
- unavailable attachment anchor
- particle closure에서만 ParticleSystem mismatch
- sidecar existing key/descriptor conflict
- non-drawable candidate
- preview preparation failure
- 첫/둘째 file promote 실패와 recovery/rollback 실패

Import 클릭 전 모든 negative case에서 source/target canonical bytes, selection, preview, disk가
불변이어야 한다. Save fault에서는 memory만 dirty로 유지하고 두 disk artifact는 같은 이전
generation이어야 한다.

## 6. G02 Particle 이동 방향 편집

### 현재 원인

SourceRecipe가 활성인 row는 Effect Tool의 fallback initial velocity UI가 disabled다. 수정 가능한 Source Trim Rotation은 particle spin/quad 표현이고, document-global Emission Direction Yaw는 Effect 전체 particle에 적용된다.

runtime 순서는 다음과 같다.

    source modules
    → SourceScale speed
    → legacy document-global emission modifier
    → velocity-over-life/update
    → target attractor
    → position integration/frame export

global yaw는 occurrence별 수정 seam이 아니며 absolute velocity-over-life가 이후 velocity를 다시
교체할 수 있다. 먼저 target row가 정확한 180도 반전이고 absolute update가 없는지 확인한다.
그 경우 기존 per-element SourceScale.speed=-1로 닫고 새 public schema를 만들지 않는다. 이
최소 경로로 닫히지 않을 때만 아래 correction block을 연다.

### schema

EFFECT_PARTICLE_DESC에 optional velocityBasisCorrection을 추가한다.

- enabled
- space: EMITTER_LOCAL, EFFECT_ROOT_LOCAL, WORLD
- rotationDegrees: 기존 transform과 같은 `[pitchX, yawY, rollZ]`
- invertVector

identity 값은 serializer에서 생략해 기존 document hash와 동작을 유지한다.

행벡터 기준으로 P는 particle root의 world rotation, B는 선택 basis의 world rotation이다.

- EMITTER_LOCAL: B=P
- EFFECT_ROOT_LOCAL: B=Effect RootWorld의 rotation-only part
- WORLD: B=identity
- localSpace=true의 P는 current ElementWorld, false는 particle SpawnRootWorld
- Euler 합성은 기존 `XMMatrixRotationRollPitchYaw(pitch, yaw, roll)`과 동일
- non-uniform scale과 shear는 orthonormal rotation 추출에서 제거

### runtime 적용

Effect_Playback에 한 개의 pure effective-velocity helper를 둔다. correction은 모든 source
update와 absolute velocity-over-life가 끝난 뒤, target attractor와 position integration 직전에
평가한다.

- stored `vVelocity/vBaseVelocity/vVelocityScale`은 raw source state로 유지하고 절대 수정하지 않음
- 매 tick raw source velocity × velocity scale에서 corrected value를 새로 계산
- Apply_TargetAttractor에는 corrected source와 별도 attractor contribution을 합성
- spawn-event payload는 `already occurrence-corrected world velocity`로 표시
- receiver는 inherited component에 receiver correction을 다시 적용하지 않고 자기 source component만 correction
- Rebuild_Frame의 vWorldVelocity와 integration이 같은 effective value를 사용
- position/location/direct-location은 회전하지 않음
- billboard roll은 전혀 읽지 않음

좌표 변환은 기존 UE3→Client 식 (x,y,z)→(x,z,-y)를 한 번만 거친다.

authoring enum/non-finite angle은 codec/publisher stage에서 reject한다. 동적으로 들어오는 RootWorld
또는 attachment basis가 finite/decomposable하지 않으면 해당 Element update를 시작하기 전에
fail-isolate하고 지난 valid state/frame을 유지한다. codec rollback과 runtime isolation을 같은
acceptance로 섞지 않는다.

### Tool UI

SourceRecipe-owned controls의 disabled block 밖에 Velocity Direction Override를 둔다.

- basis 선택
- yaw/pitch/roll
- invert
- selected fixed-step time과 first-live particle stable ordinal의 raw/effective/world velocity

help에는 Element Rotation, Billboard Roll, Source Trim Rotation, Velocity Override의 차이를 명시한다.

### 실제 34110 적용

1. V0 Product와 PR #178의 V1 audition에서 continuous 다섯 row를 같은 stable ID로 sample
2. material만 바뀐 V1에서도 velocity가 같은지 확인해 방향과 pixel visibility를 분리
3. action-forward dot이 음수 또는 측면인 target stable ID를 고정
4. 정확히 180도 반전이면 기존 SourceScale.speed=-1을 먼저 사용
5. 그 경로로 닫히지 않을 때만 emitter-local correction을 사용
6. 다른 일곱 row가 bit-identical인지 검사
7. 수치 PASS 뒤 사용자가 육안 방향을 판정

PR #178 V1은 V0 raw source seal과 occurrence 값을 복제한다. V0 34110을 고치면
`materialize_representative_four_v1_standard_color.py`를 결정적으로 다시 실행해 V1 document,
receipt/catalog seal과 필요한 fragment output을 함께 갱신한다. V1을 stale audition으로 남기지 않는다.

### 자동 검증

- identity bit-identical
- emitter-local yaw/invert exact vector
- rotated effect-root basis
- world basis
- localSpace on/off
- absolute velocity-over-life 이후에도 correction 유지
- fixed tick 반복에서 correction 비누적 canary
- 다른 Element 불변
- billboard roll 변경 시 velocity bit-identical
- parent-only/receiver-only/both와 legacy global-yaw nonzero event route
- unknown space/default omission round-trip, seek/rewind deterministic replay, hot reload mid-particle
- target-attractor on/off transition
- authoring non-finite는 stage reject; dynamic singular root/anchor는 Element fail-isolation

변경 파일:

- Client/Public/Effect_AuthoringDocument.h
- Client/Private/Effect_DocumentCodec.cpp
- Client/Private/Effect_Tool.cpp
- Client/Public/Effect_Playback.h
- Client/Private/Effect_Playback.cpp
- Tools/EffectPipeline/Publish-Effects.ps1와 focused validator fixture
- EffectRenderContractHarness.cpp
- 승인된 34110 authored row
- representative-four V1 materializer와 생성된 34110 V1/receipt/catalog/fragment

Engine public header는 바꾸지 않으므로 이 G 자체 때문에 Engine ABI를 추가하지 않는다.

## 7. G03 차원술사 A 네 검격

### 현재 원인

현재 materializer는 네 검격을 네 occurrence group으로 만들지 않고 MakeFlow emitter_2 행 네 개를 visualCardinality=4로 센다.

source SwingHit 한 번은 19 children이다.

- Mesh 11
- Sprite 8

source envelope는 첫 hit 31 rows, 후속 hit 각각 23 rows다. 이 중 매 hit의 검격 core가
19 children(11M/8S)이고 나머지는 hit별 dust/light/RGB/zoom 동반 row다. 현재 Product는 첫
0.25초에 9 rows, 뒤 세 time에는 각 1 row뿐이다.

### 변경 원칙

- outer animevent cue는 0ms 한 번 유지
- source times 0.25/0.60/0.90/1.30 유지
- imported 117 rows를 canonical source로 사용
- 기존 손튜닝은 source row를 대체하는 override map으로 보존
- 한 emitter를 group 대표로 취급하지 않음
- Server damage hit 수는 현재 한 번 유지

acceptance denominator는 두 층으로 고정한다.

- slash core: 19/19/19/19, 매 hit 11M/8S
- complete source envelope: 31/23/23/23

검격 네 번의 구조 PASS는 core로 판정하고, 사용자가 요구한 완전한 source effect 완료는
envelope까지 닫혀야 한다. 두 숫자를 하나의 `visualCardinality=4`로 축약하지 않는다.

### occurrence receipt

새 receipt 또는 기존 materializer output에 다음을 명시한다.

- hit01~hit04 stable group ID
- group source time
- child stable IDs
- carrier mix 11M/8S
- child resource/material hash
- envelope membership
- Product override provenance

### Material gate

SwingHit 76 rows의 base 감사 결과는 다음 세 상태다.

- exact-ready 36
- occurrence static permutation 필요 32
- source-profile reconstruction 필요 8

구조 복원과 Material admission을 같은 성공 플래그로 섞지 않는다.

1. active Product와 분리된 occurrence candidate/receipt에서 4×19 core와 31/23/23/23 envelope를 복원
2. 각 child를 closed tuple 또는 explicit blocker로 분류
3. active Product에는 closed tuple row만 commit
4. blocked child는 ledger/receipt에 남기며 generic Program으로 추정 대체하지 않음
5. 네 hit의 필수 visible child와 동반 envelope가 모두 draw-admitted되기 전에는 visual complete로 기록하지 않음

`G03 structure receipt`와 `G03 admitted Product/material batch`는 별도 commit/PR이다. imported
117 baseline hash가 drift하거나 override target이 ambiguous/missing이면 candidate 생성 자체를
거부한다. 같은 materializer rerun은 byte-identical이어야 한다.

### harness

fixed-step sample:

- 0.25 직전: hit 0
- 0.25: hit 1
- 0.60: hit 2
- 0.90: hit 3
- 1.30: hit 4

각 시점에 다음을 검사한다.

- 새 group만 한 번 activate
- slash core 19와 11M/8S mix
- 새 complete envelope count가 차례로 31/23/23/23
- distinct element/particle identity
- core/root history가 재시작하지 않음
- previous tail이 자연 소멸
- 네 visual hit 때문에 Server damage가 네 번 발생하지 않음
- incomplete core/envelope, authored+receipt write fault에서 active Product 불변

변경 파일:

- Data/Effects/Authored/effect.dimensionmaster.skill.2050210.unified.effect.json
- Tools/EffectPipeline/materialize_dimensionmaster_2050210_occurrences.py
- 대응 test
- occurrence receipt/material program fragments
- Effect publisher/runtime catalog
- EffectRenderContractHarness.cpp

## 8. G04 도화가 R LocalDecal과 중심 소멸

### 단정된 현재 상태

현재 row의 시간식은 이미 다음과 같다.

    u = clamp((time - 0.5333) / 0.8, 0, 1)
    position = center
    scale = lerp(1, 0.01, u)
    alpha = 1 - u
    yaw = -720 × (time - 0.5333)

EffectPlayback도 rotation/scale/color lerp를 평가한다. 따라서 같은 row를 또 추가하거나 start delay를 0으로 바꾸지 않는다. 0.5333초는 source final hit time이다.

### 확인된 결함과 아직 미확정인 원인

현재 symbol row는 source occurrence를 복원한 것이 아니다.

- fx_o_symbol_14의 실제 source 사용은 다른 Artist skill의 SpriteParticle emissive lane에서 발견됨
- R row는 이를 one-texture generic LocalDecal로 새로 구성
- Material ledger는 NO_PROGRAM_EVIDENCE / PROGRAM_EQUATION_EVIDENCE_REQUIRED
- Program/Layout/Descriptor/Adapter가 닫히지 않음
- 기존 test는 row와 lerp 숫자만 검사하고 renderer/depth/pixel을 검사하지 않음

Artist F의 D14는 6-lane source ABI이므로 R에 재사용하지 않는다.

다만 `tuple 미결 = zero pixel의 직접 원인`으로 아직 단정하지 않는다. unbound LocalDecal도
generic Shade_Effect fallback에 도달할 수 있다. resource, projector/depth, pass/state, material
admission 중 실제 0이 무엇인지는 아래 probe가 결정한다.

### 첫 단계: zero-pixel probe

현재 exact row를 바꾸기 전에 WARP render harness에서 다음을 고정한다.

- ResourceRoot에서 DDS load 성공
- DDS RGBA extrema와 사용 channel
- projector inverse와 volume clip
- Target_Depth SRV
- selected decal pass와 실제 raster/depth/blend state
- shader invocation
- DECAL_RECT draw count
- nonzero output pixel

PR #178은 이 문서를 감사하던 중 main에 merge되어 이전 pass-state 모순을 이미 닫았다.

- LocalDecal alpha-two-sided compiled Adapter 존재
- shader/pass 1, RS_Cull_None, DSS_ZNone, BS_EffectAlpha를 actual authority로 고정
- StandardColorV1/opcode 1과 LocalDecal compatibility 존재
- 대표 V1에서 compiled Adapter actual draw를 검증

따라서 R에서 새 Adapter나 decal shader를 다시 만들지 않는다. R-specific descriptor/pixel과 active
Product promotion만 아직 미검증이다. 기존 generic row와 같은 geometry로 다음을 비교한다.

| 관찰 | 판정 | 다음 변경 |
|---|---|---|
| unbound generic은 0, inline/registry StandardColor candidate는 nonzero | material admission/descriptor가 직접 원인 | closed tuple과 Product promotion |
| candidate도 draw 0 | projector/resource/admission | equation보다 앞 단계 수정 |
| draw는 있으나 output만 0 | DDS channel/descriptor/equation | lane/channel probe |
| WARP candidate는 nonzero, Product만 0 | cue/catalog/promotion join | Product closure 수정 |

### PROJECT_TUNED Material tuple

사용자 의도는 source-faithful R 복원과 별개인 symbol LocalDecal이므로, 아래 식을 채택하면
사용자 승인과 함께 PROJECT_TUNED equation provenance로 저장한다. DDS RGB/A readback은 channel
적합성과 실행 가능성만 증명하며 material equation의 source 근거는 아니다.

probe 결과와 PROJECT_TUNED 승인이 DDS RGB+A 표준 색 식을 지지하면 기존 StandardColor V1
capability를 LocalDecal carrier에 연결한다.

권장 식:

    rgbOut = sample(symbol14).rgb × color.rgb × emissive
    alphaOut = sample(symbol14).a × color.a × lifetimeEnvelope

권장 layout:

- baseRadiance: symbol14 RGB
- coverage: symbol14 A
- lifetime envelope: carrier alpha
- no dissolve lane

이 식이 probe와 맞지 않으면 source material 계산식을 추가 복원한 뒤 별도 Program을 배정하되
D14를 이름만 바꿔 복사하지 않는다.

main에 이미 있는 다음 capability를 그대로 재사용한다.

- Program: `effect.program.standard-color-v1.opcode-1.v1`
- Layout: `effect.layout.standard-color-v1.2-lane.173d7774780fd60d.v1`
- lane 0/1: 같은 symbol14 DDS의 RGB linear / A linear
- Adapter: `effect.adapter.local-decal.projector.scene-color-rt0.zero-distortion-rt1.alpha-two-sided.v1`

active Product document에 inline StandardColor를 임시로 넣는 probe는 publisher를 통과하지 못한다.
unbound inline candidate는 WARP fixture 안에서만 사용한다. 저장 가능한 비교 후보는 별도 `.v1`
EffectAssetId의 REGISTRY_BOUND_AUDITION_ONLY document로 만든다.

G04를 다음 네 gate로 닫는다.

1. G04-A: 현재 Product row의 zero-pixel 원인 WARP probe
2. G04-B: 별도 Artist R `.v1` audition candidate
3. G04-C: 사용자가 Effect Tool에서 V0/V1을 직접 비교
4. G04-D: 승인 결과에 따라 Product 승격 계약

G04-B는 다음을 한 commit으로 갖는다.

- 기존 Program/Layout/LocalDecal two-sided Adapter reference
- symbol14를 RGB/A 두 lane에 둔 새 R-owned Descriptor
- 새 `.v1` document의 inline mirror와 explicit Binding
- deterministic materializer/receipt와 source raw SHA seal
- audition catalog metadata와 WARP pixel oracle

현재 publisher는 StandardColorV1-bound active Product를 문서 template과 Binding 양쪽에서 명시적으로
거부한다. G04-D는 131개 기존 audition의 제한을 풀어버리지 않고 다음 중 하나를 선택한다.

- source-equivalent equation/evidence를 복원한 Product Program
- stable effect/element와 승인 receipt를 요구하는 일반
  `REGISTRY_BOUND_PROJECT_TUNED_PRODUCT` admission

두 번째를 택해도 SOURCE_EXACT로 승격하지 않는다. 승인된 R symbol row만 Product로 허용하고,
audition marker 일부 누락, source SHA drift, marker+binding 동시 제거 우회, 미승인 active cue를 모두
publisher가 거부해야 한다.

### 중심 소멸의 두 의미와 범위

1. symbol 한 장이 중심 기준으로 축소·소멸: 현재 scale/alpha 식을 실제 draw sample로 증명
2. 여러 particle 조각이 중심으로 모임: 현재 68 particle row 중 targetAttractor.enabled는 0개

G04-01은 사용자 문장과 현재 row가 직접 나타내는 첫 번째 self-shrink/fade로 고정한다. 두 번째가
실제 추가 요구라는 사용자 관찰이 있을 때만 G04-02를 새로 열고, marked stable cohort를 먼저
선택한 뒤 그 row들에만 target attractor overlay를 추가한다.

    direction_i = normalize(center - position_i)
    velocity_i = sourceVelocity_i + attractorVelocity_i

전체 68 rows에 일괄 적용하지 않는다. decal self-scale을 particle convergence로 기록하지도 않는다.

### harness

- 0.52초: symbol draw 0
- 0.54/0.93/1.32초: DECAL_RECT 1
- scale/alpha 단조 감소
- 유한 inverse projector
- correct depth SRV/pass/state
- PS invocation과 nonzero pixel
- missing DDS/depth, singular transform, wrong descriptor는 stage 실패와 prepared state rollback
- pass1 actual-state oracle, harness inline probe와 registry-bound audition 경로의 상호 배타성
- descriptor tuning 뒤 stale binding/inline mirror reject
- audition marker/source seal drift와 active Product 연결 negative

변경 파일:

- G04-B: 새 Artist R `.v1` authored document
- G04-B: Data/Effects/MaterialPrograms/Fragments의 새 descriptor/binding fragment
- G04-B: Data/Effects/EffectCatalog.json의 audition metadata
- G04-B: deterministic materializer/receipt와 focused tests
- EffectRenderContractHarness.cpp

G04-D에서만 active `effect.artist.skill.31210.ba4.unified.effect.json`, Product approval receipt,
EffectCatalog/Publish-Effects/Effect Tool admission과 focused Product tests를 바꾼다.

이 경로에서는 registry builder, C++ registry, renderer, `.hlsli/.hlsl`을 바꾸지 않는다. R probe가
기존 StandardColor 식으로 요구를 못 닫는다고 증명할 때만 새 equation G를 별도로 열어 재사용
함수를 `.hlsli`에 추가한다.

## 9. G05 차원술사 BA1~4

### 현재 원인 1: 잘못된 stage asset mapping

현재 Product cue는 clip01→ba1, clip02→ba1, clip03→ba3, clip04→ba1이다. ba2와 ba4는 실제 authored asset이 있어도 animevent에서 참조되지 않아 runtime publish에서 prune된다.

먼저 다음을 복구한다.

- clip01→ba1
- clip02→ba2
- clip03→ba3
- clip04→ba4
- 각 cue local start 0

각 Effect document가 이미 내부 delay를 소유하므로 source notify offset을 cue에 또 더하지 않는다.

### 현재 원인 2: 700ms는 author receipt가 아니다

현재 700ms는 과거 1400ms를 playRate 2로 절반 압축한 project tuning이다. checked-in authoritative timing에는 약 0.2초 exact frame receipt가 없다.

따라서 presentation과 Server authority를 분리해 두 입력을 먼저 정본화한다.

animation exact receipt:

- source clip FPS
- source start frame
- source end frame 또는 playMs
- intended playRate
- root motion sample end frame
- animation 담당 승인 provenance

Server balance/project-tuned approval:

- BA1 actionDurationMs와 fixed-tick rounding policy
- hit time
- comboAdvanceMs
- inputOpenMs/inputCloseMs
- field별 balance provenance

    wallMs = min(sourceClipMs, playMs) / playRate

앞 200ms를 trim해 rate 1로 재생하는 것과 1400ms 전체를 rate 7로 압축하는 것은 다른
animation이다. 현재 player skillbinding은 `{clip, playMs, playRate}`만 표현하고 nonzero source
start를 저장하지 못한다. receipt의 sourceStart가 0이 아니면 숫자를 잃어버리지 말고
AnimationSkillBindingDocument schema/parser/serializer/runtime timeline migration을 먼저 추가한다.

### 한 transaction으로 바꿀 값

두 승인이 모두 들어오면 다음을 동시에 stage한다.

- PlayerSkills.json top-level BA1 action/hit/input/advance mirror
- PlayerSkills.json comboStages[0]의 같은 field
- DimensionMaster.skillbindings.json playMs/playRate와, 필요하면 명시적 source trim
- DimensionMaster.rootmotion.json stage0 sample range
- balance provenance receipt
- Server contract test expected

frame→integer ms→Server fixed tick의 반올림 방향과 boundary 포함 규칙을 receipt에 고정한다.
animation frame은 presentation 근거이지 Server hit/input window를 단독 승인하지 않는다.

binding만 200ms로 자르고 Server를 700ms로 두면 약 500ms final-pose hold가 생긴다. Server만
줄이면 Client clip/effect/root motion이 잘린다. Effect 내부 row도
`cueWall + localDelay/effectPlaybackRate`로 샘플되므로 required delay/tail이 새 wall interval 밖으로
잘리는지 함께 검사한다.

BA2~4는 별도 author receipt가 없으면 현재 timing을 유지한다. 구조만 Artist BA처럼 네 stage array와 네 distinct Effect asset으로 맞춘다.

### cross-domain validator

다음을 하나의 join으로 검사한다.

- class/skill/inputSlot
- stage count
- stage clip array
- clip wall duration
- Server duration/advance/input window
- root motion last sample
- stage Effect asset
- runtime EffectCatalog reachability
- Effect required internal delay/tail wall time

명시적 hold policy가 없으면 clipWall < serverDuration을 거부한다.

negative:

- ba2/ba4 누락 또는 ba1 재사용
- double cue offset
- playRate/Server mismatch
- root motion overrun
- stage count mismatch
- failed publish의 partial commit
- 잘못된 class/skill/clip, FPS=0, start>end
- frame→ms/tick rounding mismatch
- top-level/comboStages[0] mirror drift
- required Effect delay가 trimmed wall interval 밖
- skill/binding/rootmotion/provenance multi-file transaction fault

runtime harness는 BA1 boundary 직전/정확히/직후, combo stage edge, clip index, effect occurrence once, no freeze/restart, natural tail survival을 검사한다.

변경 파일:

- Data/Animation/Authored/DimensionMaster/DimensionMaster.animevents
- Data/Animation/Authored/DimensionMaster/DimensionMaster.skillbindings.json
- Data/Animation/Authored/DimensionMaster/DimensionMaster.rootmotion.json
- nonzero source start가 필요할 때만 AnimationSkillBindingDocument H/CPP와 소비 timeline
- Data/Balance/PlayerSkills.json
- 공식/project provenance receipt
- Tools/GameplayPipeline/Publish-GameplayBalance.ps1
- Effect publisher Product closure tests
- ServerGameplayContractTests.cpp

generic Character.cpp와 PlayerSkillSystem.cpp는 현재 올바른 authority를 소비한다. 새 harness가 runtime 결함을 재현하기 전에는 수정하지 않는다.

## 10. G06 워로드 방패 80도

### 첫 판정

17040 identity만 확정됐다고 곧바로 cue를 바꾸지 않는다. 먼저 production/cull-none에서 현재
world velocity dot과 pixel을 측정하고, cue orientation NONE/ACTION_FACING A/B를 같은 action
edge로 비교한다.

공용 ACTION_FACING 코드는 이미 있다.

- Server action edge가 yaw를 소유
- snapshot의 fYawDegrees와 actionStartTick을 Client가 받음
- Character가 action edge에서 yaw를 capture
- AnimationEffectCueDocument가 anchor yaw basis를 action yaw로 교체

새 packet field나 두 번째 facing runtime을 선제 추가하지 않는다. 두 cull 조건 모두 현재 dot이
틀리고 ACTION_FACING에서만 `dot > 0.999`가 되면 Warlord.animevents의 wgl_sk_bash Product cue에
`orientation=action_facing`을 추가한다.

### 방향과 cull을 분리한다

17040 shield source material은 실제로 twoSided=false다. alpha_one_sided를 곧바로 two-sided로 바꾸면 source 계약을 훼손한다.

검증 순서:

1. production one-sided 상태에서 world velocity가 action forward와 일치하는지 검사
2. 동일 world transform을 diagnostic cull-none으로 한 번 비교
3. 양쪽 dot이 틀리고 ACTION_FACING A/B만 맞으면 cue orientation defect
4. dot은 맞고 production에서만 zero-pixel이면 winding/cull defect
5. 양쪽 모두 zero-pixel이면 material/descriptor/admission defect
6. source mesh winding 또는 occurrence-specific render profile만 근거 있게 수정

    dot(normalize(worldVelocityXZ), expectedActionForward) > 0.999

sample yaw:

- 0
- 80
- 90
- 180
- -90

직전 presentation yaw는 반대 방향으로 두어 action-facing을 쓰는지 확인한다. mid-flight owner yaw가 바뀌어도 발사 방향이 휘지 않아야 하며 follow가 필요하면 translation만 갱신한다.

### Material gate

17040 shield ledger는 현재 다음 상태다.

- DXBC_FAMILY_REPRESENTATIVE_ONLY
- OCCURRENCE_STATIC_PERMUTATION_REQUIRED
- opcode/layout 미할당
- runtime packet 미구현

따라서 action-facing이 맞아도 exact material fidelity는 별도 blocker다. family 대표 HLSLI를 이 occurrence의 계산식으로 간주하지 않는다.

1차 변경 파일:

- Data/Animation/Authored/Warlord/Warlord.animevents
- Tools/EffectPipeline/publish_four_class_authored_rollout.py
- Tools/EffectPipeline/verify_legacy_product_cue_projection.py
- 대응 test

legacy verifier가 현재 허용하는 action-facing delta 수도 세 번째 explicit 17040 delta로 같은
commit에서 갱신한다. 그렇지 않으면 새 cue가 정상이어도 전체 Product contract가 거부한다.

2차는 cull/material probe 결과가 요구할 때만 다음을 수정한다.

- effect.warlord.skill.17040.unified.effect.json
- material program/layout/descriptor
- mesh shader/renderer
- EffectRenderContractHarness.cpp

negative:

- invalid/non-finite facing
- stale action edge
- singular anchor
- missing WModel/DDS
- invalid descriptor
- cull 변경이 다른 one-sided occurrence에 전파됨

모두 fail-closed와 기존 prepared state 보존을 검사한다.

## 11. 구현 순서와 commit gate

여섯 public 계약을 한 branch/PR의 완료 플래그로 묶지 않는다. BA 승인이나 material ABI 연구가
generic Tool 기능 배포를 막지 않도록 다음 독립 branch/PR 단위로 진행하고, merge된 최신 main에서
마지막 통합 build를 만든다.

### G00 domain baselines

- 각 G의 기존 focused seam에서만 재현
- target stable IDs와 수치 join receipt 고정

### G01-A independent clone + strict memory transaction

- codec 공용 helper
- same-document Duplicate migration
- cross-document Tool UI
- unbound clone 상태와 strict drawable preview

### G01-B bound material paired-save

- target descriptor/explicit fragment binding
- Effect+fragment journal/recovery transaction

### G02 diagnosed velocity overlay

- existing SourceScale.speed로 닫히는지 먼저 판정
- 필요할 때만 schema/codec/publisher/UI/playback/harness
- 34110 target occurrence 데이터

### G03-A DimensionMaster A occurrence receipt

- 4×19 core와 31/23/23/23 envelope candidate
- active Product 불변

### G03-B admitted material batches

- closed tuple row만 Product commit
- blocker 분리
- fixed-step activation

### G04-A Artist R zero-pixel cause

- zero-pixel probe

### G04-B Artist R LocalDecal tuple

- probe가 요구할 때만 PROJECT_TUNED program/layout/descriptor/compiled adapter
- self-shrink/fade

multi-particle convergence는 사용자 확인 뒤 별도 G04-C다.

### G05-01 DimensionMaster BA product mapping

- ba1/ba2/ba3/ba4 reachability 먼저 복구

### G05-02 DimensionMaster BA exact timing

- animation receipt와 Server balance 승인이 모두 들어온 뒤 binding/Server/root motion/provenance 동시 migration

### G06 confirmed Warlord defect

- A/B가 증명한 Product cue만 변경
- yaw covariance
- occurrence-specific cull/material follow-up

### G07-01 integration publish/build

- 모든 publisher/harness
- Debug/Release Server+Client build
- 사용자 수동 판정 경로 인계

각 PR은 자체 publisher/harness/Debug·Release 영향 범위를 닫는다. 모든 독립 PR이 merge된 뒤 최신
main에서 전체 Debug/Release Server+Client regression과 사용자 수동 판정 경로를 한 번 인계한다.

## 12. 자동 검증 명령

구현 후 최소 다음을 실행한다.

    powershell -ExecutionPolicy Bypass -File Tools/EffectPipeline/Publish-Effects.ps1 -Mode Validate
    powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Validate
    powershell -ExecutionPolicy Bypass -File Tools/EffectPipeline/Sync-EffectDataProject.ps1 -Check
    $effectContractBindingCount = @((Get-Content -Raw -Encoding UTF8 Data/Effects/MaterialPrograms/effect-material-program-registry.v1.json | ConvertFrom-Json).bindings).Count
    Get-ChildItem Data/Effects/MaterialPrograms/Fragments -Filter *.json | ForEach-Object { $effectContractBindingCount += @((Get-Content -Raw -Encoding UTF8 $_.FullName | ConvertFrom-Json).bindings).Count }
    powershell -ExecutionPolicy Bypass -File Tools/EffectRenderContractHarness/Run-EffectRenderContractHarness.ps1 -Configuration Debug -ExpectedBindingCount $effectContractBindingCount
    powershell -ExecutionPolicy Bypass -File Tools/EffectRenderContractHarness/Run-EffectRenderContractHarness.ps1 -Configuration Release -ExpectedBindingCount $effectContractBindingCount
    powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
    powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release
    git diff --check

추가 fixture는 다음 실패를 반드시 포함한다.

- invalid version/enum/path
- duplicate stable ID
- missing resource
- material registry mismatch
- partial publish fault injection
- stage/prepared preview failure rollback
- exact boundary 직전/정확히/직후
- 동일 texture를 쓰는 비대상 occurrence 불변

## 13. 사용자 수동 판정 경로

자동 검증이 끝난 뒤 에이전트는 Server+Client 실행 준비와 정확한 경로만 보고한다.

### 창술사

- Lobby → Character Select → Lance Master
- 장창 stance → D 반월섬
- 선택된 fragment cohort가 action forward로 흐르는지 판정

### 차원술사 A

- Lobby → Character Select → DimensionMaster
- A 사용
- 0.25/0.60/0.90/1.30 cadence로 네 개의 완전한 검격 envelope인지 판정

### 도화가 R

- Lobby → Character Select → Artist
- R 콤보 마지막 sdm_sk_skykongkong_02
- ground symbol draw, 중심 축소, 회전, alpha 소멸 판정
- particle convergence가 별도 요구면 marked cohort가 중앙으로 모이는지 판정

### 차원술사 BA

- LMB BA1~4
- BA1 exact author timing, final-pose hold 없음, ba1~ba4 distinct effect 판정

### 워로드

- G00가 확정한 skill을 0/80/90/180/-90 방향으로 사용
- 17040이 확정된 경우 S 배쉬가 첫 대상
- 방패 mesh의 발사 방향과 one-sided 가시성을 각각 판정
- D/X는 같은 defect로 가정하지 않고 denominator 확인용으로 한 번씩 확인

사용자의 서면 관찰 전에는 manual first pixel, visual PASS, fidelity 완료로 기록하지 않는다.

## 14. G05-02 구현 전 필요한 두 authority 승인

차원술사 BA1의 약 0.2초 exact frame receipt가 현재 main에 없다.

첫 gate는 animation presentation receipt다.

- animation 담당자가 저장한 source start/end frame + FPS
- 동일 의미의 sourceStartMs/playMs/playRate와 승인 provenance
- 해당 값이 담긴 merge 예정 branch/commit

두 번째 gate는 Server balance/project-tuned 승인과 field provenance다.

- actionDuration/hit/input/advance의 승인 값
- frame→ms→fixed tick rounding 정책
- PlayerSkills top-level/stage mirror를 함께 바꾼다는 승인

두 입력이 없어도 G00, G01, G02, G03, G04, G05-01, G06은 진행할 수 있다. 다만
animation receipt만으로 Server 수치를 추정하거나 200ms를 임의 입력해 전체 완료로 처리하지 않는다.

## 15. 독립 비평에서 기각한 가정

세 read-only 비평을 실제 코드와 다시 대조해 다음 가정을 폐기했다.

- 기존 Try_CommitDocument를 Import rollback 경로로 그대로 사용
- authored inline Execution만 보고 publisher가 destination registry binding을 자동 생성
- 하나의 EffectRenderContractHarness가 Server→Character→Effect→pixel 전 경로를 재현
- animation frame receipt 하나로 Server hit/input/combo 수치까지 승인
- row 존재 또는 DDS RGB/A readback만으로 Artist R의 zero-pixel 원인/equation을 확정
- Warlord target identity만 확인하면 action_facing을 즉시 적용
- 차원술사 A의 emitter 네 행을 네 complete slash occurrence로 계산

모두 위 G의 strict transaction, authority별 fixture, dual approval, carrier ABI decision과
occurrence receipt로 교체했다.

## 16. 비범위

- 모든 particle에 global 방향 보정 적용
- 모든 one-sided material을 two-sided로 변경
- Artist F D14를 모든 decal의 공용 식으로 사용
- 차원술사 A visual 4회에 맞춰 Server damage를 4회로 변경
- Client가 BA combo stage를 자체 증가
- source asset file을 Effect마다 물리 복제
- Client/UI 자동 실행과 화면 캡처
- material blocker를 generic fallback으로 숨김

## 17. 완료 정의

각 문제는 다음 상태를 따로 기록한다.

- 구현 완료: 코드/data/publisher가 연결됨
- 자동 검증 완료: positive/negative/rollback/boundary harness PASS
- 빌드 완료: Debug/Release Server+Client PASS
- 수동 검증 대기: 사용자가 아직 화면을 보지 않음
- 수동 검증 완료: 사용자가 서면으로 관찰 결과를 승인

여섯 항목 모두 자동 검증까지 닫히고, BA1의 두 authority 승인이 반영되고, 사용자가 수동 화면을
승인해야 전체 완료다. 각 G 구현 전에는 이 감사서의 확정 gate를 입력으로 `.md/GB/local.md`
형식의 별도 full-code `*_PLAN.md`를 작성한다.
