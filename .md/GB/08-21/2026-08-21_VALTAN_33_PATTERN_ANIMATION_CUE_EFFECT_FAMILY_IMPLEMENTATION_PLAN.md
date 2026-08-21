# 2026-08-21 Valtan 33 Pattern Animation·Cue·Effect Family 구현 계획

## 0. 결론

이 문서는 기존 Whirlwind canary 전용 계획을 발탄 33개 pattern 전체 계획으로 확장한 정본이다.
구현 결과는 캐릭터 Q/W/E/R/T와 같은 Effect 저작 경로를 사용한다.

    Valtan Pattern
      -> Server semantic stage
      -> ordered animation clip occurrence
      -> effect cue occurrence
      -> unified Effect document
      -> family/group
      -> editable elements

별도의 Valtan 전용 Effect renderer, editor, catalog, hot-reload manager는 만들지 않는다. 복구된
All Effects, Open Editor, Save, 선택 Effect Hot Reload와 기존
EffectCatalog -> EffectDocumentRenderer -> EffectPresentationService 경로를 그대로 확장한다.

### 0.1 Git 입력 정본과 적용 우선순위

계획 작성 시점의 실측은 다음과 같다.

| 항목 | 실측 |
|---|---|
| origin/main | dd382bf1c817, PR #130 merge |
| animation sequence merge | PR #127, merge 459da808 |
| ordered audition/pattern merge | PR #128, merge 0b70f06e |
| 현재 구현 branch 기준 | codex/valtan-all-effect-families-0821 @ dd382bf1c817 |
| origin/main -> HEAD | 구현 시작 시 동일 |
| 작업 격리 | 별도 worktree에서 구현하며 원래 worktree의 DimensionMaster 2050100 손튜닝 diff는 수정·stage하지 않음 |

PR #130은 복구 commit 410543ca와 이 계획서를 main에 병합했다. 따라서 발탄 구현은 main에 병합된
animation sequence와 pattern 변경, 복구된 캐릭터 authoring tool을 모두 가진 dd382bf1 기준선에서
별도 기능 branch로 시작한다.

입력의 지위는 다음처럼 고정한다.

1. PR #127 merge 459da808에서 Animation 담당자가 저작한 Valtan.patternbindings 행과 ordered clip
   chain이 제품 animation 입력 정본이다. 순서를 generic Effect builder가 교체하지 않는다.
2. source action/clip sequence, notify, ParticleSystem, emitter, selected LOD 자료는 animation chain을
   Effect occurrence로 분해하는 source evidence다.
3. ValtanEncounter.json의 pattern/stage/actionId/duration/hit 계약은 Server gameplay 권위다.
   Pattern 담당자가 만든 기존 cue와 generic Effect 문서는 coverage 참고 자료이지 source-exact
   Effect family의 정답이 아니다.
4. 사용자가 지정한 telegraph, 색, 크기, DDS 조합은 exact source join이 증명되지 않으면
   PROJECT_AUTHORED 또는 PROJECT_TUNED_OVERRIDE로 분리한다. source-exact라고 이름 붙이지 않는다.

Animation 담당자의 병합 결과를 "33 pattern의 sequence가 모두 확정됐다"고 과장하지 않는다. 해당
RESULT에서 직접 저작했다고 명시한 pattern은 다음 13개다.

- SWING, DOWN_SMASH, IMPRISON_ROAR, EARTHQUAKE_SMASH, PARRY
- MAGIC_CHOICE, FOUR_SLASH, HIGH_JUMP, GROUND_WAVE_SMASH
- WHIRLWIND, RED_BLADE_WAVE, FRONT_BACK_FRONT, FIST_IN_OUT

이 13개도 실제 PR #127 diff의 행 단위 provenance를 보존하며, merged patternbindings를 제품 입력으로
소비한다. Valtan.clipseq/Valtan.clipcuts는 read-only 근거이지 제품 mapping을 자동 교체하는 정본이
아니다. PR #127에서 값이 바뀌지 않은 행과 나머지 20개는
현재 단일 clip 또는 부분 chain을 보존한 채 source sequence를 다시 review한다. 특히 DASH_CHARGE는
Animation 담당 RESULT에서도 forced motion 거리와 ACTIVE duration 결합 때문에 보류한 항목이므로,
Effect timing만 늘리지 않고 G06-2의 animation/forced-motion 검증을 함께 통과시킨다.

현재 patternbindings 124행은 한 담당의 단일 산출물이 아니다. 459da808 뒤 Pattern PR #128 계열에서
DASH_CHARGE groggy/part-break와 ARMOR_BREAK_OPENING part-break 3행이 추가됐다. 이 3행은 current
gameplay coverage 참고 행으로 표시하고 Animation 담당 정본과 같은 provenance로 취급하지 않는다.
Whirlwind active-2/active-3는 최종 PR #129 merge resolution에서 제거돼 현재 파일에는 없다.

선행 복구 commit 410543ca는 다음 경계를 닫았다.

- PR #129의 캐릭터·Whirlwind 변경 보존
- 캐릭터 Q/W/E/R/T All Effects tree 복구
- Open Editor, Play Full Effect, Save, hand tuning 복구
- 저장한 선택 Effect만 다음 spawn부터 교체하는 Hot Reload
- reload 실패 시 이전 catalog/prepared GPU revision 보존
- 실행 중인 Effect instance는 이전 revision을 끝까지 사용

과거의 무거운 PublishTransaction, SHA admission, ProductCue approval, 저장마다 full
Publish-Effects를 수행하는 경로는 복구하지 않았다. Debug Client/Server build, focused
ClientFrontendHarness incremental-prewarm 10건이 통과했고 failure는 0이다. 현재 Hot Reload 대상은
direct-authored Player Effect이므로 G05에서 같은 rollback 계약을 Valtan product cue까지 일반화한다.

### 0.2 확정된 사용자 결정

- 발탄도 캐릭터 스킬과 동일한 unified Effect 문서와 동일한 손튜닝 UI를 사용한다.
- BA 1/2/3 unified처럼 한 pattern stage 안에 여러 animation occurrence와 여러 Effect occurrence를
  허용한다.
- 1 stage = 1 cue, 1 actionId = 1 Effect, 1 effectAssetId = 1회 사용 제약을 제거한다.
- 하나의 Effect family는 독립 runtime asset 종류가 아니라 unified document 내부 group과 element다.
- 도끼 target decal, axe projectile, ground impact, Valtan landing도 별도 Parallel Actors tree가 아니라
  같은 pattern의 cue occurrence와 unified Effect 내부에서 편집한다.
- 신규 pattern에서 standalone Light와 명백한 공용 Dust/Debris/Stone/Foot/Rock/Smoke family는
  이번 source-exact 완료 분모에서 defer할 수 있다.
- 단, 선택한 attack-defining ParticleSystem 내부의 dust처럼 보이는 emitter는 임의로 제외하지 않고
  selected LOD 전체 closure를 유지한다.
- 이미 완료된 Whirlwind active 9/9에는 dust와 Light가 포함돼 있으므로 삭제하거나 축소하지 않는다.
- Client Effect 변경만으로 Server damage/hit actor를 만들지 않는다. High Jump의 3회 axe drop은
  이번 slice에서는 presentation occurrence로 연결하고, 실제 피해가 필요하면 별도 Server-authority
  contract로 승격한다.
- 최종 visual fidelity는 사용자가 직접 판정한다. 자동 검증 성공을 visual PASS로 기록하지 않는다.

### 0.3 완료 정의

33 pattern 완료는 단순히 Effect JSON 파일이 존재한다는 뜻이 아니다. 각 source occurrence가 다음
disposition 중 정확히 하나를 가져야 한다.

| disposition | 의미 |
|---|---|
| EXECUTABLE_CORE | source identity, resource, material carrier, timing, attachment가 제품에서 실행됨 |
| PROJECT_AUTHORED | Server semantic stage에 의도적으로 새로 만든 telegraph/presentation |
| PROJECT_TUNED_OVERRIDE | source resource closure를 보존하면서 사용자가 선택한 값으로 override |
| DEFERRED_BY_SCOPE_LIGHT | standalone Light이며 이번 범위에서 명시적으로 defer |
| DEFERRED_BY_SCOPE_GENERIC_DUST | explicit shared ambience dust family라 명시적으로 defer |
| UNRESOLVED_DECAL | PlayDecal event는 있으나 payload identity가 없음 |
| UNRESOLVED_PROJECTILE | projectile event는 있으나 presentation payload 또는 authority가 미확정 |
| UNRESOLVED_EFFECT | Effect event는 있으나 target payload가 미확정 |
| UNREACHABLE_SOURCE_OCCURRENCE | source family는 있으나 merged animation mapping에서 해당 clip/segment에 도달하지 못함 |

discovered source denominator는 executable + project-authored/override + deferred + unresolved의 합과
정확히 같아야 한다. 조용히 누락한 occurrence는 0이어야 한다. 각 rollout batch는 이 closure와
손튜닝 보존 검증을 통과한 뒤에만 merge한다.

### 0.4 2026-08-21 구현 체크포인트

현재 구현은 고정 5-slot 또는 발탄 전용 renderer가 아니다. 캐릭터와 같은
`EffectCatalog -> unified v13 Effect -> EffectDocumentRenderer -> 기존 material/profile/shader ->
EffectPresentationService` 경로를 사용한다. `base/noise/mask/model`은 element의 재질 resource role이며
family 개수 제한이 아니다. source ParticleSystem과 emitter/selected LOD closure 수만큼 group과 element가
생긴다. 초기 admission의 신규 HLSL 수는 0이다.

| 항목 | 현재 구현 실측 |
|---|---:|
| v2 animation action rows / stable clip occurrences | 124 / 128 |
| v2 product cue occurrences | 99 |
| source patterns / source sequences / source occurrences | 33 / 227 / 9,434 |
| reviewed selected sequences | 21 |
| source systems / selected-LOD carriers | 130 / 1,044 |
| reviewed reachable completion carriers | 436 |
| drawable-ready direct carriers | 551 |
| missing runtime resource / adapter-blocked carriers | 350 / 125 |
| deferred standalone Light / explicit generic Dust carriers | 6 / 12 |

Portal Rush immutable source candidates 3문서 24 elements는 ordinary v13 codec, drawable validation,
renderer prepare와 nonzero draw를 모두 통과했다. project-authored 우선 후보는 9문서 24 desired
elements이며 기존 hand-tuned 6행을 보존하고 18행만 missing append한다. 9문서 18 visible elements도
prepare/draw를 모두 통과했다. 이 후보들은 아직 canonical Authored/Catalog/cue에 적용하지 않았으며,
transactional projection과 전체 source candidate draw gate가 끝난 뒤에만 반영한다.

All Effects의 제품 구조는 다음과 같다.

    Valtan Pattern
      -> semantic Stage
      -> ordered Clip Occurrence
      -> Product Cue Occurrence
      -> Unified Effect
      -> source ParticleSystem/hand-authored Family Group
      -> Mesh/Sprite/Decal/AnimationTrail/TrailGhost/Ribbon Elements

Open Editor, Play Full Effect, Save, selected same-revision Hot Reload는 이 exact occurrence를 유지한다.
cue source start/play rate/local transform이 animation과 Effect에 같은 clock으로 적용되며, 마지막
non-loop pose 이후 natural Effect tail은 wall clock으로 끝까지 진행한다. 실패 rollback은 이전
Valtan clip pose/time/playing/visible/snapshot을 복원하고, unsaved load Cancel은 기존 Player target을
바꾸지 않는다.

## 1. 현재 실측

### G00-1. 제품 data 분모

| 분모 | 현재 값 |
|---|---:|
| Encounter patterns | 33 |
| Encounter semantic stages | 127 |
| animation action bindings | 124 |
| product Effect cues | 99 |
| cue 없는 stages | 28 |
| cue가 참조하는 unique Effect documents | 99 |
| 99 documents의 elements | 3109 |
| sourceNode가 있는 elements | 17 |
| executable sourceRecipe elements | 5 |
| sourcePresentation elements | 9 |

현재 99개 문서의 실질 상태는 다음과 같다.

- Whirlwind active 1개: 5 base + 3 baked AnimationTrail + 1 first-edge Light, 9/9
- Floor Wipe 4개: sourceNode 8개가 있으나 recipe/attachment가 꺼진 부분 수작업 상태
- 나머지 94개: clip aggregate로 생성된 generic skeleton

따라서 99 cues가 있다는 사실은 99개 Effect family가 source-exact로 닫혔다는 뜻이 아니다.
Whirlwind도 active만 닫혔고 WINDUP cue가 없으며 recovery는 generic이므로 pattern 전체 완료가 아니다.

source reference inventory는 31/33 patterns에 actionbinding이 있고 ENTRANCE_WHIRLWIND와
ARENA_BREAK_84가 비어 있다. raw event 9175개는 particle 4812, unresolved Effect 3475,
PlayDecal 529, trail 359이며 PlayDecal payload 529개는 모두 unresolved다. 1차 분류의 unique template은
core 608, optional ambience 414, unresolved 618이다. branch 선택 전 source emitter 962는 상한일 뿐
제품 구현 denominator가 아니다. full key와 reviewed branch를 선택한 뒤 실제 batch denominator를
다시 고정한다. 이 raw corpus를 authored document마다 복제하지 않고 immutable source index를
정규화 참조한다.

### G00-2. 현행 구조 blocker

1. Valtan.patternbindings formatVersion 1은 actionId -> string 또는 ordered string array만 저장한다.
   stable clip occurrence ID, source segment, play length/rate, loop 정책이 없다.
2. Valtan.patterneffectcues formatVersion 1은 encounter tuple, actionId, effectAssetId를 각각 유일하게
   강제해 사실상 stage당 cue 하나만 허용한다.
3. CValtan runtime은 actionId -> vector<Cue> container는 이미 가지지만, stage age를 ordered clip
   source time과 loop epoch로 변환하지 않는다.
4. 기존 All Effects Valtan view는 RuntimeClipNames.front()만 표시하고 첫 clip만 replay한다.
5. build_valtan_stage_effects.py는 clip name에 붙은 모든 system을 합치고 source branch/notify/emitter
   occurrence를 잃는다. 고정 scale/lifetime과 filename heuristic으로 generic 문서를 다시 만들어
   사용자의 손튜닝을 덮을 수 있다.
6. Character Select의 일부 Valtan preparation gate는 고정 99 cue count에 의존한다. cue occurrence 수와
   unique prepared Effect asset 수는 다르므로 v2에서 깨진다.

### G00-3. 반드시 먼저 교정할 animation/effect join

| pattern | 현행 문제 | 계획 |
|---|---|---|
| FRONT_BACK_FRONT | 제품 active는 19_01이고 19_06/2_03 source family는 merged mapping에서 unreachable | 우선 unreachable로 기록하고 별도 animation mapping delta를 검증 |
| DASH_CHARGE | 4_01을 WINDUP/CHARGE에서 재시작해 2.45초 exact Dash notify에 도달하지 못함 | reviewed source segment와 stage wall mapping을 명시 |
| MAGIC_CHOICE | inner/outer branch가 같은 clip aggregate document에 섞임 | branch ID와 end family를 분리 |
| FLOOR_WIPE_130 | exact six-direction Atk09_02는 15_03인데 제품 chain에 15_03이 없음 | unreachable로 기록하고 별도 animation mapping delta를 검증 |
| ARENA_BREAK_84 | 3 stages 모두 binding/cue 없음 | environment-only 여부를 확정하고 명시적 disposition 부여 |

Animation 담당 자료의 시간 우선순위는 Valtan.clipcuts의 MonsterMoveNextStage cut, animnotify length,
model clip duration 순서다. 이 reference를 제품 v2로 옮길 때 Server stage age와 forced motion을 함께
검증하며, reference file 자체를 제품 runtime 정본으로 승격하지 않는다.

source evidence에만 존재하는 19_06, 2_03, 15_03을 Effect 작업이 patternbindings에 바로 삽입하지
않는다. 먼저 UNREACHABLE_SOURCE_OCCURRENCE로 inventory하고, clipseq/clipcuts, model replay,
Server duration/hit/forced-motion을 함께 확인한 별도 animation mapping delta가 승인된 뒤에만 reachable
occurrence로 승격한다.

## 2. 목표 데이터와 런타임 계약

### G01. Boss ordered clip occurrence formatVersion 2

Valtan.patternbindings.json을 action별 ordered clip object 배열로 정규화한다.

| 필드 | 의미 |
|---|---|
| actionId | Server semantic action stable ID |
| clipOccurrenceId | document 전체에서 유일한 저장 identity |
| clip | animation 담당자가 연결한 실제 model clip 이름 |
| mappingBasis | ANIMATION_PR_127, PATTERN_PR_REFERENCE, CURRENT_PRODUCT_BASELINE, SOURCE_REVIEWED_DELTA, PROJECT_AUTHORED 중 하나 |
| sourceStartMs | 선택한 source clip segment의 시작, 기본 0 |
| playMs | 기존 character 계약과 같은 source-local 최대 재생 길이, 0이면 segment 끝까지 |
| playRate | source time / wall time 비율, 0.05..16 |
| loop | 명시적 반복 여부, reachable chain의 마지막 clip에만 허용 |

vector index는 저장 ID로 사용하지 않는다. clipIndex는 parse 뒤 파생 ordinal로만 보관한다.
mappingBasis는 runtime 분기값이 아니라 row-level provenance다. PR #127에서 실제 변경·검토된 행은
ANIMATION_PR_127, PR #128이 추가한 3행은 PATTERN_PR_REFERENCE, 그 밖의 병합 전 기존 행은
CURRENT_PRODUCT_BASELINE로 기록한다. source evidence만으로 sequence를 바꾸지 않으며, 별도 검토를
통과한 delta만 SOURCE_REVIEWED_DELTA로 승격한다.
sourceStartMs 기본값 0은 기존 캐릭터 동작을 바꾸지 않는다. CActionPresentationTimeline을 확장해
source segment 시작, source duration cutoff, playRate, loop epoch를 함께 해석한다.

playMs가 0이면 effective source end는 실제 model duration이고 effective duration은
modelDuration - sourceStartMs다. 0이 아니면 effective duration은 playMs와 남은 model duration 중
작은 값이다.

비-loop chain이 Server stage보다 먼저 끝나면 마지막 pose를 clamp한다. 마지막 clip을 암묵적으로
처음부터 반복하지 않는다. stage duration을 넘는 chain, 음수/범위 밖 segment, 중간 clip loop,
duplicate clipOccurrenceId는 publish 전에 거부한다.

formatVersion 1은 read-only migration을 지원한다. string/array 순서를 보존하고 deterministic
occurrence ID, sourceStartMs 0, playMs 0, playRate 1을 사용한다. legacy 반복은 명시 migration으로만
보존하며 writer와 Git 정본은 formatVersion 2만 저장한다.

### G02. Pattern Effect cue formatVersion 2

cue는 semantic stage가 아니라 stable animation occurrence에 직접 연결한다.

| 필드 | 의미 |
|---|---|
| bindingId | authoring binding stable ID |
| occurrenceId | runtime cue occurrence stable ID |
| patternId/stageId/actionId | Encounter authority join |
| clipOccurrenceId | G01 ordered clip occurrence join |
| effectAssetId | unified Effect document ID, 반복 사용 가능 |
| anchor/follow/stop/localTransform | 기존 attachment contract |
| sourceStartMs/sourceEndMs | referenced clip의 absolute source-local cue window |
| repeatPolicy | once 또는 each_loop; animation loop와 Effect 재생 반복을 분리 |

bindingId와 occurrenceId만 document-global unique다. actionId와 effectAssetId의 반복을 허용한다.
같은 action 안의 unique key는 actionId + clipOccurrenceId + occurrenceId다.

CActionPresentationTimeline은 cue source time에서 clip sourceStartMs를 뺀 뒤 playRate와 앞선 clip wall
duration을 적용해 Server stage wall offset으로 변환한다. repeatPolicy=each_loop인 cue만 loop epoch마다 다시
발생하고 once는 첫 epoch에서 한 번만 spawn한다. runtime instance key는 Server pattern sequence + stage +
occurrenceId이며 each_loop일 때만 loop epoch를 추가한다. 533ms animation clip을 2133ms 동안 반복하지만
409-sample baked Effect는 한 번만 유지하는 Whirlwind는 repeatPolicy=once를 고정한다.

각 cue는 referenced clipOccurrenceId가 같은 actionId에 속하고, cue sourceStartMs가 clip segment
sourceStartMs 이상이며, sourceEndMs가 sourceStartMs보다 크고 effective source end 이하임을 요구한다.
변환한 wall offset은 finite/nonnegative여야 한다. non-loop cue end는 Server stage duration 이내여야
하고 loop epoch는 stage edge로 bounded한다. natural end를 뜻하는 nullable end를 도입한다면 0이나
임의 큰 수로 표현하지 않고 schema에서 별도 명시한다.

late snapshot은 이미 끝난 occurrence를 새로 spawn하지 않고, 아직 살아 있으면 elapsed local time으로
catch-up한다. stage edge, sequence 변경, death/despawn은 해당 owner occurrence만 stop한다.

v1 cue는 action이 단일 clip일 때만 첫 occurrence로 lossless migration한다. ordered multi-clip action의
v1 cue는 explicit migration row가 없으면 fail-close한다.

### G03. Source occurrence와 family identity

Valtan.patterneffects.json은 clip aggregate가 아니라 다음 full identity로 source evidence를 저장한다.

    patternId + semantic stage/actionId + clipOccurrenceId + sourceActionId
    + branchId/stagePath/sourceStageIndex + notifyId/time/duration
    + ParticleSystem + emitter + selected LOD + ordered module occurrence

family identity는 DDS filename이나 pattern 이름이 아니라 UE3 parent material + renderer/carrier shape +
named texture-role set + render profile/static switches다.

첫 33-pattern admission에서는 신규 HLSL profile을 0개로 유지한다. 기존 typed profile과
effect.ue3.grouped-translucent.v1로 실행 가능한 family를 우선 연결한다. 반복되는 고영향 family에
공식 수식 증거가 있을 때만 후속 별도 batch에서 profile을 추가한다.

### G04. Immutable import와 editable product 분리

1. Data/Effects/Imported/Valtan/Converted에는 pinned source로부터 결정적으로 재생성 가능한 occurrence
   receipt와 immutable recipe를 둔다.
2. Data/Effects/Authored/effect.valtan.*.effect.json은 All Effects가 여는 제품 손튜닝 문서다.

기본 제품 단위는 캐릭터 BA unified와 같은 persisted clipOccurrenceId 하나당 독립 unified Effect
문서 하나다. 그 clip에서 admission된 notify/system/emitter family를 문서 안의 group/element로 묶고,
notify local time은 element timing으로 보존한다. 의도적으로 같은 모양을 반복할 때만 동일 asset을
여러 cue occurrence가 재사용한다.

generic importer는 build_imported_effect_documents.py의 SourceIndex, selected_lod_partitions,
emitter_detail, build_source_recipe, choose_resources를 Valtan driver로 승격한다.
build_valtan_stage_effects.py의 clip aggregation은 canonical writer에서 제거하고 migration audit로만 쓴다.

| compiler 소유 | author 소유 |
|---|---|
| stable source identity/provenance | visible |
| sourceNode/sourceRecipe module graph | local transform/scale |
| attachment evidence | color/emissive/detail multiplier |
| immutable resource closure | explicit PROJECT_TUNED resource override |

builder는 missing element만 stable ID로 추가한다. 기존 author-owned field를 bulk overwrite/delete하지
않는다. source identity drift는 SOURCE_REBASE_REQUIRED로 실패한다. 같은 입력 두 번은 byte-identical이고,
sentinel hand tuning 뒤 재실행해도 author-owned field가 보존돼야 한다.

ordinary sprite/mesh/decal sourceRecipe에는 VisualProgram을 만들지 않는다. baked AnimationTrail만
ADAPTER_PACKET_V1 supplemental sidecar를 쓴다. 기존 Whirlwind 409-sample, 1.2-second clamp,
3 Trail + 1 Light packet은 회귀 canary로 동결한다.

### G05. All Effects와 선택 Hot Reload

    All Effects
      Valtan
        Phase
          Pattern
            Semantic Stage
              Ordered Clip Occurrence
                Product Cue Occurrence
                  Unified Effect Document
                    Family/Group
                      Elements

Open Editor, Play Full Effect, Save, transform/color/Detail/resource tuning은 캐릭터와 같은 widget과
renderer를 사용한다. Valtan-only editor 창은 만들지 않는다. 동일 effectAssetId를 세 cue가 재사용하면
mapping count는 3으로 보이되 문서는 한 번 reload한다.

Effect Tool의 PlayerProduct 전용 Count/Can/Try helper를 owner-neutral Product helper로 일반화하고
Valtan cue mapping을 합친다. 선택 Save는 catalog document와 matching prepared GPU target을
stage -> validate -> commit한다.

- active instance는 old revision 유지
- next spawn은 new revision 사용
- 실패하면 이전 catalog/prepared target 유지
- unrelated Effect는 reload하지 않음
- full Publish/Client restart는 source/schema/catalog batch publish 때만 수행

고정 cue count 99 gate를 제거하고 dedup된 unique effectAssetId target set을 preparation 정본으로 쓴다.

## 3. 33 pattern rollout matrix

cue는 현재 cue 수 / semantic stage 수다. G는 generic skeleton, P는 partial, X는 exact canary다.

| # | pattern | source action | 현재 animation/핵심 family | cue | 상태 |
|---:|---|---|---|---:|---|
| 1 | SWING | 420601,660 | idle -> 1_01+1_02; sweep/trail | 3/3 | G |
| 2 | DOWN_SMASH | 420602,661 | idle -> 2_01; smash/wave/trail | 3/3 | G |
| 3 | IMPRISON_ROAR | 420603 | 5_01 start/loop/end; roar/wave/energy/eye | 3/3 | G |
| 4 | DASH_CHARGE | 420604 | 4_01 -> charge/groggy -> 4_02; shield/trail/impact | 3/5 | G |
| 5 | EARTHQUAKE_SMASH | 420605,662 | 7_01 -> 7_02; center/ring waves | 4/4 | G |
| 6 | PARRY | 420606,607 | 9_01 start/loop/end; counter/wave/trail | 3/3 | G |
| 7 | MAGIC_CHOICE | 420608 | 5_02 branches; inner/outer donut | 4/4 | G mixed |
| 8 | FOUR_SLASH | 420609 | 10_01+10_02; four slash families | 3/3 | G |
| 9 | HIGH_JUMP | 420610 | 8_01 start/loop/end; target/axe/landing | 3/4 | G |
| 10 | STOMP | 420611 | 11_01; stomp/wave | 3/3 | G |
| 11 | BIND_CHARGE_SMASH | 420612-614 | 12_01 -> 13_01 -> 13_03 -> 12_02 | 3/4 | G |
| 12 | GROUND_WAVE_SMASH | 420615 | 15_01 -> 15_02+03+04 -> 15_05 | 2/3 | G |
| 13 | SUPER_SMASH | 420619,620,656,657 | 12_01 -> 12_02 -> 12_05; triple impact | 2/3 | G |
| 14 | JUMP_SPIN | 420621,663 | 20_01 -> 20_02 -> 20_03 -> 20_04 | 3/4 | G |
| 15 | PORTAL_RUSH | 420622 | 18_01 portal -> 18_02 rush -> 18_03 impact | 4/4 | G, clean candidate |
| 16 | CHARGE_GRAB_ROAR | 420623,631,632 | 21_01 -> 21_02 -> 21_03 -> 21_04 | 3/4 | G |
| 17 | WHIRLWIND | 420633 | 20_02 -> 20_03 -> 20_04 | 2/3 | active X 9/9 |
| 18 | BACKSTEP_ATTACK | 420635,664 | 20_03 -> 20_02 -> 7_03 | 2/3 | G |
| 19 | RED_BLADE_WAVE | 420636 | 9_01 -> 12_10 projectile -> 12_11 | 3/3 | G |
| 20 | FRONT_BACK_FRONT | 420637,666 | 제품 19_01, source 19_01 -> 19_06 -> 2_03 | 3/3 | G, misgrouped |
| 21 | FIST_IN_OUT | 420638 | 19_02 -> 19_04 -> idle; circle/ring | 3/4 | G |
| 22 | LEDGE_ROAR | 420639 | 5_01 start/loop/end; body fire/dash/wave | 3/3 | G |
| 23 | TRIPLE_COUNTER | 420640-647 | groggy/counter loop chain, 3 counter hits | 6/7 | G |
| 24 | ARMOR_BREAK_OPENING | 420627,628,654,655 | parts/groggy/parts end | 1/4 | G |
| 25 | FLOOR_WIPE_130 | 420630 | 1_01 -> 1_02 -> 5_02 loop/end -> 15_04; 15_03 누락 | 4/5 | P |
| 26 | FOUR_PILLARS_105 | 420610 | 8_01 start/loop/end; ring/cone | 2/4 | G |
| 27 | ENTRANCE_WHIRLWIND | 미확정, 420633 inferred reuse | 20_02 -> 20_03 -> 20_04 | 2/3 | G, PROJECT reuse |
| 28 | ARENA_BREAK_109 | 420629 | 12_01 -> 12_02 -> 12_03 -> hold/reveal | 5/6 | G |
| 29 | ARENA_BREAK_84 | 미확정 | WINDUP/IMPACT/RECOVERY binding 없음 | 0/3 | empty |
| 30 | MAGIC_ORB_STAGGER_76 | 420617,618 | groggy start/loop/end; magic/counter | 2/3 | G |
| 31 | CENTER_GRAB_COUNTER_64 | 420623,631 | 21_01 -> 21_02 -> 21_03 -> 21_04 | 4/5 | G |
| 32 | ARENA_BREAK_33 | 420629 | 12_01 -> 12_02 -> 12_03; landing/spin | 2/4 | G |
| 33 | GHOST_TRANSITION_15 | 420616,624-626,634,651-653,658-659,665 | portal/ghost/clone/wave | 6/7 | G |

현재 cue 없는 28 stages:

- DASH_CHARGE: GROGGY, PART_BREAK
- HIGH_JUMP: AIRBORNE
- BIND_CHARGE_SMASH: RECOVERY
- GROUND_WAVE_SMASH: RECOVERY
- SUPER_SMASH: IMPACTS
- JUMP_SPIN: LAND
- CHARGE_GRAB_ROAR: CHARGE
- WHIRLWIND: WINDUP
- BACKSTEP_ATTACK: SWEEP
- FIST_IN_OUT: WINDUP
- TRIPLE_COUNTER: RECOVERY
- ARMOR_BREAK_OPENING: GROGGY, RECOVERY, PART_BREAK
- FLOOR_WIPE_130: RECOVERY
- FOUR_PILLARS_105: YELLOW_ZONE, RECOVERY
- ENTRANCE_WHIRLWIND: WINDUP
- ARENA_BREAK_109: DROP
- ARENA_BREAK_84: WINDUP, IMPACT, RECOVERY
- MAGIC_ORB_STAGGER_76: RECOVERY
- CENTER_GRAB_COUNTER_64: TARGET_EXPLOSION
- ARENA_BREAK_33: LANDING, SPIN
- GHOST_TRANSITION_15: OUTER

cue가 없다는 이유로 무조건 particle을 만들지 않는다. NO_VISUAL인지, source occurrence가 있는지,
project-authored telegraph가 필요한지 검토하고 explicit disposition을 기록한다.

## 4. 사용자 우선 6 pattern

### G06-1. Portal Rush

18_01 Portal02_01/02, 18_02 Dash01_1, 18_03-1 Atk01_02, recovery residual을 각 persisted
clip occurrence의 unified 문서와 내부 family로 만든다. RUSHES 반복이 실제 제품 animation epoch와
일치한다고 검증된 경우에만 같은 unified asset을 여러 cue occurrence가 재사용한다.
multi-occurrence, asset reuse, tree, selected Hot Reload의 첫 end-to-end admission으로 쓴다.

### G06-2. Dash Charge

exact core는 4_01 약 2.45초의 Par_S_RPBF_Dash_01_1과 TrailGhost다. halfsphere/hemisphere가
전방 방패 형태의 근거다. animation occurrence가 notify에 도달하도록 source segment/wall mapping을
교정한다. exact resource는 atypical_055, ring_004, shockwave_02 계열이다. atypical_042_ycl과
line_003_xcl은 Dash exact가 아니다. 다만 사용자 확정 요구에 따라 `fx_c_line_003_xcl.dds` 붉은
전방 경로를 mandatory `PROJECT_TUNED_OVERRIDE` telegraph로 연결한다. exact provenance는
FLOOR_WIPE 420630 / Atk09_02에 그대로 남겨 두고 Dash source-exact shield family와 섞지 않는다.
벽 충돌은 Server stage/impact edge를 소비하며 Client가 판정하지 않는다.

### G06-3. Magic Choice Donut

- inner: Atk03_04 + shared Atk03_06 -> end Atk03_02
- outer: Atk03_05 + shared Atk03_06 -> end Atk03_03
- ring_002 exact join은 outer end Atk03_03 내부

inner/outer branch를 같은 aggregate doc에 섞지 않는다. 성장 ring 2개는 source payload 미해결 시
PROJECT_TUNED telegraph로 두고 attack fill과 provenance를 분리한다. 두 boundary는
`fx_c_ring_002.dds` base와 `fx_c_ring_004_cl.dds` mask 조합을 독립 element로 소유한다. 작은 ring은
timeline scale growth, 안팎 ring은 nonzero UV sweep을 손튜닝한다. ring_004의 exact 근거는 Dash
shield material 문맥이므로 Magic Choice 사용은 PROJECT_TUNED로 표시한다.

### G06-4. Floor Wipe 130 Six Direction

Atk09_01 prep, source clip 15_03 약 0.262초의 Atk09_02 exact six-direction, 5_02_end의 Atk09_03
impact를 분리한다. line_003_xcl은 Atk09_02 exact다. d009/d011/d032는 source material slot 역할대로
보존하고 filename으로 의미를 재지정하지 않는다. project-authored center impact는 d009 diffuse,
d032 mask, d028 noise, d011 dissolve role을 명시하고 색/UV/scale/lifetime을 손튜닝한다. d028도
Atk09_02 source evidence가 있으므로 generic dust로 제외하지 않는다. 15_03은 우선 unreachable evidence로 남기고,
별도 animation mapping delta와 Server timing 검증이 통과해야 product occurrence로 승격한다.

### G06-5. Front Back Front, 3연격

source candidate chain은 19_01 -> 19_06 -> 2_03이지만 merged product mapping은 19_01만 소유한다.
19_01에는 약 1.170, 2.254, 3.224, 4.220초에
네 visual wave가 있지만 gameplay hitCount는 3이다. Hit 1/2/3을 독립 occurrence로 만들고 네 번째
wave는 evidence 검토 전 auxiliary/recovery visual로 표시한다. 19_06/2_03은 unreachable inventory로
보존하며 별도 animation mapping delta가 승인된 뒤 Atk02_08, trail, Atk02_04, unresolved decal을
reachable recovery occurrence로 분리한다. HIGH_03, b_decal_001, h_wave_04는 exact join이 아니므로
사용 시 PROJECT_TUNED_OVERRIDE다. 최종 ground supplemental의 `fx_i_shockwave_02_ycl.dds`도
420637 source join이 없으므로 청록색 PROJECT_TUNED element로 분리한다.

### G06-6. High Jump, 도끼 3회와 착지

source-exact는 takeoff Atk06_03, Valtan land Atk06_04, branch final Atk06_05/08이다.
AIRBORNE의 Effect/PlayDecal payload는 미해결이다. target decal 3회와 Valtan landing 보강은
PROJECT_AUTHORED 후보로 연결하지만, 현재 52개 Valtan Effect WModel에서 filename-stable axe/weapon
payload가 확인되지 않았다. 따라서 axe beat 1/2/3은 `UNRESOLVED_PROJECTILE`로 유지하고 anonymous
mesh를 도끼로 추측하거나 Client damage actor를 만들지 않는다. 검증된 projectile presentation과
Server authority가 확보된 뒤 같은 cue occurrence 아래 승격한다. fx_h_atypical_01_1은 exact join이
아니므로 사용 시 PROJECT_TUNED_OVERRIDE다. 현 Server LAND 1회 damage는 바꾸지 않는다.

## 5. Rollout 순서

source candidate index는 33 pattern을 한 번에 만들고 product admission은 나눈다.

### G07-0. B0, Whirlwind canary 1

- WHIRLWIND: active 9/9, 409 samples, 1.2-second clamp 보존. WINDUP과 recovery만 별도 closure.

### G07-1. B1, 사용자 우선 6

- PORTAL_RUSH, DASH_CHARGE, MAGIC_CHOICE, FLOOR_WIPE_130, FRONT_BACK_FRONT, HIGH_JUMP

### G07-2. B2, 기본 공격군 9

- SWING, DOWN_SMASH, IMPRISON_ROAR, EARTHQUAKE_SMASH, PARRY, FOUR_SLASH, STOMP,
  BIND_CHARGE_SMASH, GROUND_WAVE_SMASH

### G07-3. B3, 복합 공격군 9

- SUPER_SMASH, JUMP_SPIN, CHARGE_GRAB_ROAR, BACKSTEP_ATTACK, RED_BLADE_WAVE, FIST_IN_OUT,
  LEDGE_ROAR, TRIPLE_COUNTER, ARMOR_BREAK_OPENING

### G07-4. B4, phase/mechanic군 8

- FOUR_PILLARS_105, ENTRANCE_WHIRLWIND, ARENA_BREAK_109, ARENA_BREAK_84,
  MAGIC_ORB_STAGGER_76, CENTER_GRAB_COUNTER_64, ARENA_BREAK_33, GHOST_TRANSITION_15

각 batch는 source closure, product join, non-destructive reconcile, publisher, harness, Debug/Release build를
통과한 뒤 독립 commit/PR로 올린다.

## 6. 변경 예상 파일

### G08-1. Data

    Data/Animation/Authored/Valtan/Valtan.patternbindings.json
    Data/Animation/Authored/Valtan/Valtan.actionbindings.json
    Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json
    Data/Animation/Authored/Valtan/Valtan.patterneffects.json
    Data/Animation/Reference/Valtan/Valtan.clipseq
    Data/Animation/Reference/Valtan/Valtan.clipcuts
    Data/Effects/Imported/Valtan/Converted/*
    Data/Effects/Authored/effect.valtan.*.effect.json
    Data/Effects/EffectCatalog.json
    Data/Effects/Contracts/effect-family-manifest.v1.json
    Data/Effects/VisualPrograms/effect-visual-program-corpus.v1.json
    Data/Effects/VisualPrograms/effect-visual-program-runtime.v1.json

ValtanEncounter.json은 gameplay authority join/validation input이다. 이번 presentation slice에서는 기본적으로
수정하지 않는다. Pattern 담당 stage 구성은 Effect 분해 참고로 읽고 unrelated gameplay를 되돌리지 않는다.

### G08-2. Tools

    Tools/EffectPipeline/build_valtan_action_bindings.py
    Tools/EffectPipeline/build_valtan_stage_effects.py
    Tools/EffectPipeline/build_valtan_whirlwind_effect_canary.py
    Tools/EffectPipeline/validate_boss_pattern_effects.py
    Tools/EffectPipeline/Schemas/lostark.boss-pattern-effects.schema.json
    Tools/EffectPipeline/build_imported_effect_documents.py
    Tools/EffectPipeline/build_effect_family_manifest.py
    Tools/EffectPipeline/project_ue3_material_families.py
    Tools/EffectPipeline/promote_effect_family_occurrences.py
    Tools/EffectPipeline/build_effect_visual_program_corpus.py
    Tools/EffectPipeline/build_effect_visual_program_runtime.py
    Tools/EffectPipeline/Publish-Effects.ps1
    Tools/EffectPipeline/tests/*Valtan*

새 generic source importer/reconciler가 필요하면 Python 파일과 deterministic test를 같은 commit에 둔다.

### G08-3. Client

복구 commit 뒤 다음 기존 파일을 확장한다.

    Client/Public/AnimationSkillBindingDocument.h
    Client/Private/AnimationSkillBindingDocument.cpp
    Client/Public/ActionPresentationTimeline.h
    Client/Private/ActionPresentationTimeline.cpp
    Client/Public/ValtanPatternEffectCueDocument.h
    Client/Private/ValtanPatternEffectCueDocument.cpp
    Client/Public/ValtanPatternTree.h
    Client/Private/ValtanPatternTree.cpp
    Client/Public/Valtan.h
    Client/Private/Valtan.cpp
    Client/Public/Effect_Tool.h
    Client/Private/Effect_Tool.cpp
    Client/Private/CharacterSelectArenaSpawnGate.h
    Client/Private/Level_CharacterSelect.cpp
    Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp

Effect_Catalog, Effect_DocumentRenderer, Effect_PresentationService, Effect_ProductPrewarmQueue는 복구
commit API가 부족할 때만 최소 확장한다. 새 Valtan manager/renderer C++ 파일은 만들지 않는다.
따라서 이 계획 자체는 새 vcxproj/filter 등록을 요구하지 않는다.

### G08-4. 문서

    CLAUDE.md
    .md/TEAM/ANIMATION_TOOL_OWNER_HANDOFF.md
    이 PLAN과 대응 RESULT

## 7. 자동 검증 계약

### G09-1. Schema와 migration

- 33 patterns, 127 stages, 124 current bindings parse
- ARENA_BREAK_84 3 binding holes를 silent fallback 없이 명시
- duplicate/unknown occurrence/action/stage/effect asset reject
- invalid sourceStartMs/playMs/playRate/loop reject
- cue clipOccurrence가 같은 action 소유인지 검증
- cueStart >= clip.sourceStart, cueEnd > cueStart, cueEnd <= effective source end
- wall offset finite/nonnegative, non-loop end <= stage duration, loop epoch stage-bounded
- clip reorder 뒤 stable ID join
- v1 single-clip migration PASS, multi-clip ambiguous migration reject
- parse/validate/stage 실패 시 이전 document 유지

### G09-2. Timeline과 runtime

- source segment/rate -> Server wall offset
- same stage 2~N cues, same asset 3 occurrences
- once cue의 loop 중 duplicate spawn 0, each_loop cue의 epoch별 재발생
- late snapshot expired skip/live catch-up
- sequence/stage/death owner stop
- ordered clip chain 전체 replay
- FRONT_BACK_FRONT 4 visual waves vs gameplay hit 3 분리
- DASH 2.45-second notify reachability
- FLOOR_WIPE 15_03은 현재 UNREACHABLE_SOURCE_OCCURRENCE로 검증하고,
  별도 SOURCE_REVIEWED_DELTA가 들어올 때만 reachability를 검증

### G09-3. Source closure와 reconcile

- pinned action/package/graph SHA와 full source join
- executable + deferred + unresolved = discovered
- dropped/duplicate full-key 0
- filename-only family inference 0, 신규 HLSL profile 0
- hand-tuned sentinel 보존, 두 번 실행 byte-identical
- source drift reject, check mode mutation 0
- admitted occurrence의 renderer/pass와 prepared resource가 실제 선택됨
- 전체 action-time sweep에서 각 core/project occurrence가 최소 한 sample draw > 0
- submitted transform/lifetime이 finite이며 resource/prepare/GPU 실패 시 이전 state rollback

### G09-4. Product와 Hot Reload

- patterneffects <-> cue <-> catalog <-> doc <-> receipt 양방향 closure
- orphan 0, prewarm = unique assets
- Valtan Save 다음 spawn 새 revision
- active old revision 유지, failure rollback, unrelated revision 불변
- Whirlwind 9/9/409 samples/1.2-second clamp 불변
- ordinary sourceRecipe VisualProgram 생성 0

### G09-5. 실행 범주

1. focused Python tests와 validate_boss_pattern_effects.py
2. Publish-Effects Validate/Check와 batch Publish
3. visual-program corpus/runtime builders check
4. Test-EffectPipeline.ps1과 ClientFrontendHarness
5. 변경 영향 범위의 Debug/Release 정본 build/regression
6. JSON parse와 git diff --check

## 8. 사용자 수동 검증 인계

    F1 -> Effect Tool -> All Effects -> Valtan -> Phase -> Pattern
       -> Stage -> Clip Occurrence -> Product Cue -> Open Editor

사용자는 캐릭터 Q/W/E/R/T tree 보존, Valtan Save 후 restart 없는 다음 replay 반영, active instance 안정,
ordered animation/cue 순서, occurrence별 손튜닝, shared asset reuse, Whirlwind 회귀를 직접 확인한다.
에이전트는 Client/UI를 자율 실행하거나 visual PASS를 대신 판정하지 않는다.

## 9. Commit과 PR 순서

1. 선행 복구: All Effects/Open Editor/Save/selected Hot Reload와 캐릭터 회귀 복원
2. boss clip occurrence v2, cue v2, timeline, tree, dynamic prewarm gate
3. all-33 source index와 non-destructive reconcile
4. source-unreachable clip의 reviewed animation mapping delta와 forced-motion regression
5. B0 Whirlwind migration
6. B1 사용자 우선 6
7. B2 기본 공격군 9
8. B3 복합 공격군 9
9. B4 phase/mechanic군 8
10. 최종 closure, RESULT, handoff, full regression

각 commit은 data/schema/runtime/consumer/harness를 끊지 않는 vertical slice다. main에는 직접 작업하지
않고 codex branch에서 push/PR한다. 다른 작업자의 미커밋 파일은 자동 add하지 않는다.

## 10. Whirlwind checkpoint의 지위

- action 420633
- effect.valtan.pattern.420633.active
- base 5 + baked AnimationTrail 3 + first-edge Light 1 = 9/9
- baked history 409 samples, playback clamp 1.2 seconds
- formatVersion 3 Client sidecar always-load

이 수치는 회귀 canary일 뿐 발탄 전체 완료가 아니다. 최종 완료는 33 pattern 각각의 source denominator,
disposition, animation occurrence, product cue, editable document, 자동 검증, 사용자 visual 판정을 분리한다.
