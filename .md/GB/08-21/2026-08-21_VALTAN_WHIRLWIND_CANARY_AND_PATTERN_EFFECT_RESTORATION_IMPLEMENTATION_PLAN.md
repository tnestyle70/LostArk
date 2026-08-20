# 2026-08-21 Valtan Whirlwind Canary 및 Pattern Effect 복원 구현 계획

## 0. 목표와 완료 경계

이 계획의 첫 완료 단위는 발탄 휠윈드 action `420633`, source stage `2`, runtime clip
`mesh_att_battle_20_03`의 첫 LOD carrier 아홉 개를 모두 제품 실행 가능한 source/typed packet으로
만드는 것이다. 그 다음 별도 gate에서 full Effect Publish로 Runtime EffectCatalog에 입장시킨다.

- AnimationTrail 3개
- Dust sprite 2개
- 기존 실행 중인 Whirlwind sprite/mesh 3개
- Point Light 1개

아홉 개가 같은 source occurrence, emitter, selected LOD, material/resource, attachment,
Server action age를 보존하는 canary가 된 뒤 같은 importer와 admission 규칙을 다른 발탄
pattern에 확장한다. pattern별 authoring view는 보기 편한 분류일 뿐 정본 identity가 아니다.

완료 보고는 다음을 분리한다.

1. 구현: source identity와 제품 runtime 연결
2. 자동 검증: JSON/schema/publisher, source/resource identity, Debug/Release build
3. 수동 검증: 사용자가 제품 Valtan에서 직접 확인한 첫 픽셀과 fidelity

사용자 결정에 따라 새 전용 harness는 만들지 않는다. Debug Effect Tool/selected Save Hot Reload는
runtime 단순화 계약에서 제거됐으며, authoring 반영은 full publisher와 Client 재시작을 사용한다.

### 0.1 2026-08-21 최종 기준선

| gate | 현재 상태 |
|---|---|
| Whirlwind source/typed VisualProgram | 5 base + 3 baked Trail + 1 first-edge Light, `9/9` |
| Authoring cue | `effect.valtan.pattern.420633.active`, `0..2133ms` 존재 |
| Runtime EffectCatalog product row | `1`, payload 일치. 현재 게시본은 전환용 legacy 6-field |
| 새 Client 실행 carrier | formatVersion 3 sidecar always-load로 `9/9` 구조 연결 |
| 최종 Publish | 모든 authored 저장 종료 뒤 4-field catalog로 별도 실행 |
| Gameplay/Server | combined bootstrap v13 Publish, Server Release/contract PASS |
| Item startup input | `Server/Bin/DataFiles/Items/Items.bootstrap` 생성 |
| Debug Server | 실행 중 프로세스의 출력 점유 때문에 link만 미완료, 재시작 필요 |
| Visual fidelity | 사용자 미판정, visual PASS 없음 |

`9/9` source/typed denominator와 Runtime EffectCatalog row `1`은 각각 검증됐다. formatVersion 3 loader는
marker와 무관하게 sidecar를 항상 로드하므로 새 Client code의 구조적 실행 분모는 `9/9`다. 다만 현재
게시 catalog는 legacy 6-field이고 다른 authored 저장보다 오래됐으므로 최종 4-field full Publish와 Client
재시작이 남는다. 어느 자동 상태도 실제 화면 fidelity PASS를 뜻하지 않는다.

## 1. 현재 실측

### G00-1. Server와 Client의 권위 분리

발탄 gameplay action, phase, pattern, stage, action age는 Server가 소유한다. Client는 snapshot의
stable pattern/action/stage/sequence와 age를 소비해 authored effect cue를 생성한다. 따라서
texture 이름이나 Client animation clip만으로 pattern을 추측하지 않는다.

정확한 join key는 다음 전체 튜플이다.

```text
patternId
+ semantic actionId
+ source actionId
+ source branch/stage path
+ sourceStageIndex
+ notifyId/time/duration
+ ParticleSystem
+ emitter
+ selected LOD
```

### G00-2. 휠윈드 아홉 carrier 분모

정본 mapping은 `Data/Animation/Authored/Valtan/Valtan.patterneffects.json`의
`valtan.whirlwind.420633.active`다. 구현 전 authored document는 다음 세 개만 실행했다.

| carrier | kind | source material | 현재 상태 |
|---|---|---|---|
| notify006/emitter7336 | sprite | `fx_d_pa_atta_05_11_ad` | 실행 |
| notify006/emitter7334 | mesh | `fx_s_me_missiletrail_01_1_ts_tr` | 실행 |
| notify006/emitter7335 | mesh | `fx_s_me_missiletrail_01_1_ts_tr` | 실행 |

구현 전 빠진 여섯 개와 이번 admission 경계는 다음과 같다.

| carrier | kind | source material/component | blocker의 본질 |
|---|---|---|---|
| notify004/emitter5259 | AnimationTrail | `fx_d_pa_trail_07_05_tr` | baked edge typed packet + bounded material |
| notify004/emitter5260 | AnimationTrail | `fx_d_pa_atta_09_01_ad` | baked edge typed packet + bounded material |
| notify004/emitter5258 | AnimationTrail | `fx_o_pa_ribbonmaster_03_04_tr` | baked edge typed packet + bounded material |
| notify005/emitter7034 | sprite | `fx_e_pa_fd_04_1_tr` | finite material + DynamicParameter admission |
| notify005/emitter7035 | sprite | `fx_c_pa_aura_02_tr` | exact runtime DDS binding |
| notify009/emitter6823 | light | `PointLightComponent_1335` | first-edge inferred typed attachment |

### G00-3. 기존 공용 런타임

새 Trail renderer나 새 Light manager는 만들지 않는다.

- AnimationTrail의 typed packet과 일반 source material rendering 경로는 이미 존재하지만, 현
  runtime의 root translation history + camera-facing width는 이번 source carrier의 정확한 geometry가
  아니다. `MN_RPBF_00_AnimNotify_Trails`의 `animnotify_trails_479`에는 409개의
  `(relativeTime, firstEdge, control, secondEdge)` baked sample이 보존돼 있다.
- Point Light는 authored light detail을 평가해 transient presentation light로 제출하는 경로가
  이미 존재한다.
- 공용 source recipe parser는 emitter delay/duration/loop, burst, module order, literal,
  distribution lookup/random-lock을 보존한다.

따라서 이번 변경은 기존 경로의 입력/admission을 source evidence로 닫는다.

## 2. 구현

### G01. Source graph와 mapping 재고정

`FX_CM_02` particle graph를 현 extractor로 다시 생성한다. 현 extractor는
`PointLightComponent`를 포함하고 graph invariant를 검증하지만, 기존 외부 receipt는 그 기능
도입 전 산출물이라 component를 누락했다.

fresh extraction의 필수 분모는 다음과 같다.

```text
graphObjectCount                  7046
particleSystemCount               194
pointLightComponentCount           32
pointLightComponentReferenceCount  32
invalidPointLightComponentTarget    0
propertyErrorCount                  0
```

새 graph SHA를 mapping에 고정하고, 다른 package graph와 action document pin은 바꾸지 않는다.
mapping의 carrier disposition/blocker/count는 실제 새 admission 결과와 같은 변경에서 갱신한다.

### G02. AnimationTrail 3개

기존 LanceMaster 전용으로 고정된 supplemental builder를 class-neutral input row 방식으로
일반화한다. Lance 4개와 Valtan 3개가 같은 packet schema를 사용하되 selector와 provenance는
각 occurrence identity를 보존한다.

각 Valtan row는 다음을 만족해야 한다.

- target effect asset: `effect.valtan.pattern.420633.active`
- target element: emitter5259/5260/5258 각각 하나
- source notify: `action-420633/stage-002/notify-004`
- kind/renderer: `trail` / `ANIM_TRAIL`
- typed module: 정확히 하나의 `ParticleModuleTypeDataAnimTrail`
- material/resources: 각 source emitter의 exact material closure와 cooked DDS
- timing: notify local time과 duration
- geometry carrier: `animnotify_trails_479`의 409개 baked edge pair를 immutable packet에 보존
- sampling: source sample 범위 `0..3.1999977s` 중 stage 2 notify duration `1.2s`에서 clamp
- renderer: camera-facing 폭 재구성이 아니라 sampled `firstEdge/secondEdge`를 직접 strip vertex로 제출
- attachment/control edge는 root history로 조용히 대체하지 않는다.

AnimationTrail을 Cascade Ribbon이나 ordinary sprite recipe로 해석하지 않는다.
세 target element는 base document에서 `visible=false`, generic `sourceRecipe.enabled=false`를
유지한다. `ANIMATION_TRAIL_BAKED_EDGE_HISTORY_V1` supplemental projection만 target copy를
활성화한다. geometry sample과 DDS/object/hash 연결은 source-exact지만, 세 UE3 parent material을
`effect.ue3.grouped-translucent.v1`로 실행하는 shader 의미는 `BOUNDED_RECONSTRUCTION`이다.
따라서 제품 admission은 exact geometry와 reconstructed material을 같은 말로 합치지 않는다.

### G03. Dust 2개

#### G03-1. emitter7034

도화가 E `31480`에서 같은 exact material `fx_e_pa_fd_04_1_tr`이 이미 사용하는
`effect.ue3.grouped-translucent.v1` profile을 재사용한다.

- dissolve: `fx_tex_03.fx_e_noise_008`
- base/emissive: `fx_tex_03.fx_e_fluid_007`
- DynamicParameter source names:
  `dissolve_density`, `alpha_power`, `emissive_tiling`, `lamp_time`
- source recipe의 curve/lookup table은 그대로 보존한다.

material family를 skill 또는 boss 이름으로 복제하지 않는다.

#### G03-2. emitter7035

`fx_c_pa_aura_02_tr`의 existing common profile `effect.ue3.aura.v1`과 다음 exact source
texture를 연결한다.

- `fx_tex_00.fx_a_glow_009`
- `fx_tex_high_00.fx_a_cloud_026`

동일 source object/hash가 이미 cook된 runtime payload를 재사용하거나 기존 cook pipeline으로
Valtan namespace에 배치한다. filename 유사도나 임의 색 texture를 사용하지 않는다.

### G04. Point Light 1개

fresh `FX_CM_02` graph의 exact instance를 읽는다.

- component: `light.par_mp_light_05_l...pointlightcomponent_1335`
- explicit brightness: `10.0`
- nested effect archetype radius: `200.0 UE unit`
- class default falloff: `2.0`
- inherited default light color: white

기존 `light.point.reconstructed.v1` detail로 투영하고 source parameter마다 explicit,
archetype, class-default provenance를 기록한다. source notify가 직렬화한 anchor label은
`EndControl_01`이지만 현재 Valtan WModel은 87 bones, socket 0이며 이 이름을 제공하지 않는다.
따라서 이를 exact socket으로 주장하거나 root로 대체하지 않고, 같은 notify004 baked history의
`firstEdge` lane에 붙이는 `SIBLING_TEMPLATE_INFERRED` attachment로 경계를 봉인한다.
`LIGHT_BAKED_EDGE_ATTACHMENT_V1` packet은 source start `0.07953999936580658s`, duration
`1.120460033416748s`, end `1.2000000327825546s`와 별도의 history clamp
`1.2000000476837158s`를 보존한다. base light target은 hidden/disabled이고 projected copy만 기존
transient light runtime에 제출된다.

### G05. 제품 admission과 Server semantic time

9/9 document가 검증된 뒤에만 EffectCatalog와 Valtan pattern binding을 제품 경로로 승격한다.

- catalog asset ID는 `effect.valtan.pattern.420633.active`
- cue는 `VALTAN_WHIRLWIND` / `SPIN` / `valtan.attack.whirlwind.active`
- cue 제품 window는 origin/main의 stage clip-chain v12 계약과 같은 `0..2133ms`다.
- spawn age는 Server snapshot action age를 사용한다.
- source notify time은 authored local schedule로 유지한다.
- source clip 전체 duration을 Server action duration으로 덮어쓰지 않는다.

즉 `2133ms`는 Server semantic SPIN stage의 lifetime이고, baked history `1.2s` clamp는 source
carrier local sample 범위다. 두 시간을 같은 값으로 정규화하지 않는다.

#### G05-1. origin/main clip chain과 local gameplay timing의 결합

origin/main `70c9bc1c`의 발탄 clip-chain 계약은 파일 전체 교체가 아니라 현재 dirty worktree 위에
semantic hunk로 합친다. `Valtan.patternbindings.json`의 `clip`은 string 또는 최대 16개의 ordered
array이고, array를 소유하는 pattern은 한 Server stage age 안에서 순서대로 소비한다. 현재 실측에서
Whirlwind active 자체는 array가 아니라 단일 `mesh_att_battle_20_03`이다. Whirlwind의 `2133ms`는
ordered clip 3개를 뜻하는 값이 아니라 Server `SPIN` semantic stage와 effect cue의 window다.

Gameplay bootstrap은 local `comboAdvanceMs` v12와 upstream Valtan `hitDelayMs` v12 중 하나를
덮어쓰지 않고 combined v13으로 올린다.

- `SKILLSTAGE`는 기존 `comboAdvanceMs`, `inputOpenMs`, `inputCloseMs` 순서를 유지한다.
- `PATTERNSTAGE`는 `hitIntervalMs` 뒤, `serverDamageProfileId` 앞에 `hitDelayMs`를 추가한다.
- `NONE` hit shape는 `hitDelayMs == 0`, hit stage는 `hitDelayMs < durationMs`를 요구한다.
- `CValtanBrain`의 hit pulse threshold는
  `hitDelayMs + appliedHitCount * hitIntervalMs`다.
- Balance Tool의 load/edit/normalize/save와 Server loader/contract fixture가 같은 v13 순서를 쓴다.
- provenance receipt는 전체 파일을 upstream 것으로 바꾸지 않고 canonical updater가 stable field
  key union을 만들어 현 skill/receipt entry를 보존한다.

### G06. 나머지 발탄 pattern 확장

기존 `build_valtan_stage_effects.py`의 clip/material-first 추정 경로를 정본으로 사용하지 않는다.
G00의 full join key와 `build_imported_effect_documents.py`의 emitter/LOD recipe atom을 사용해
pattern별 document를 재생성한다.

우선 적용군은 사용자 관찰과 source identity가 모두 있는 다음 pattern이다.

1. 3연 공격 slash/trail와 마지막 ground decal/wave
2. 돌진 windup line, 이동 shield/mesh/trail, impact
3. donut outer/inner ring timeline과 attack fill
4. 중앙 원 + 6방향 telegraph
5. whirlwind canary와 같은 typed carrier를 공유하는 나머지 pattern

project-authored telegraph는 source-derived particle와 별도 provenance를 가지며 Server semantic
stage와 연결한다. 사용자가 지목한 DDS는 후보가 아니라 source material/parameter join이 맞을
때만 최종 binding으로 승인한다.

## 3. 변경 예상 파일

```text
Data/Animation/Authored/Valtan/Valtan.patterneffects.json
Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json
Data/Animation/Authored/Valtan/Valtan.patternbindings.json
Data/Effects/Authored/effect.valtan.pattern.420633.active.effect.json
Data/Effects/Imported/Valtan/Valtan.420633.whirlwind-baked-edge-history.v1.json
Data/Effects/EffectCatalog.json
Data/Effects/VisualPrograms/effect-visual-program-corpus.v1.json
Data/Effects/VisualPrograms/effect-visual-program-runtime.v1.json
Data/Encounters/Valtan/ValtanEncounter.json
Tools/EffectPipeline/build_valtan_whirlwind_baked_edge_history.py
Tools/EffectPipeline/build_valtan_whirlwind_effect_canary.py
Tools/EffectPipeline/Schemas/lostark.boss-pattern-effects.schema.json
Tools/EffectPipeline/build_effect_visual_program_corpus.py
Tools/EffectPipeline/build_effect_visual_program_runtime.py
Tools/EffectPipeline/build_valtan_stage_effects.py
Tools/EffectPipeline/validate_boss_pattern_effects.py
Tools/EffectPipeline/Publish-Effects.ps1
Tools/GameplayPipeline/Publish-GameplayBalance.ps1
Server/Public/GameplayCatalog.h
Server/Private/GameplayCatalog.cpp
Server/Private/ValtanBrain.cpp
Client/Public/BalanceTool.h
Client/Private/BalanceTool.cpp
Client/Public/Valtan.h
Client/Private/Valtan.cpp
```

`Publish-Effects.ps1`은 Validate/Publish만 소유한다. selected transaction과 ProductCue approval/admission은
제거됐고 420633 canary를 제품 검증에서 제외하지 않는다. visual-program schema/runtime/C++의 typed
Light attachment는 이 계획의 mapping과 같은 packet identity를 사용한다.

새 C++ 파일은 추가하지 않았으므로 `.vcxproj`/`.vcxproj.filters` 등록 변경은 없다. 새 Python 파일은
409-sample history를 결정적으로 생성하는 data builder이며 별도 실행형 harness가 아니다.

## 4. 검증

새 harness를 추가하지 않고 다음을 실행한다.

1. fresh `FX_CM_02` graph invariant와 SHA 검증
2. Valtan pattern mapping schema/cross-document 검증
3. Whirlwind builder `--check`: carrier 9, base-visible 5, typed supplemental 4,
   product-executable 9, fail-closed 0, duplicate 0
4. Effect publisher Validate/Publish 후 runtime document JSON parse
5. VisualProgram corpus/runtime builder check
6. Client x64 Debug/Release build
7. `git diff --check`
8. final 4-field Publish와 Client 재시작 뒤 사용자가 제품 Valtan에서 결과를 직접 판정

자동 검증 성공은 visual fidelity PASS를 뜻하지 않는다.

### 4.1 2026-08-21 자동 검증 checkpoint

- baked history `--check`: 409 samples, source end `3.200000047683716s`, playback clamp
  `1.2000000476837158s`, artifact SHA
  `46bf2e83ace7d798a2ff34489cc4eb223a716ac75159d799f6dd306707112a64`
- Whirlwind canary `--check`: base-visible `5/9` + baked-edge Trail `3/9` + first-edge Light
  `1/9`, product-executable `9/9`, blocker `0`
- Light의 `EndControl_01 -> FIRST_EDGE`는 exact socket이 아니라 sibling-template 기반
  `SIBLING_TEMPLATE_INFERRED` 연결이다. 이를 source-exact attachment로 승격하지 않는다.
- boss mapping validator와 canary Python unit `15/15`: PASS
- focused Valtan frontend binding/cue와 Balance Tool round-trip: failures `0`
- visual-program corpus `--check`: schedules `35`, rows `135`, artifact SHA
  `5407580676ca5d9123a762b384f24184abcbf61dea13c423634236378f297873`
- visual-program runtime `--check`: programs `14`, rows `135`, artifact SHA
  `03e6d76db542d2dd78c200da40128203d89bbfaf0ca7eac86154a8210227fe32`
- 위 corpus/runtime SHA는 04:09 게시 checkpoint다. 현재 post-save 감사에서는 source runtime과
  published sidecar가 byte-identical이고 420633 program/supplemental identity도 불변이지만, corpus input
  38개 중 schema raw SHA 1건이 stale하다. 최종 full Publish 전에 이 global provenance를 재생성한다.
- combined gameplay bootstrap v13 Validate/Publish: 6 profiles, 136 skills, 33 patterns,
  124 stages, PASS. `SKILLSTAGE.comboAdvanceMs`와 `PATTERNSTAGE.hitDelayMs`를 모두 보존한다.
- origin/main의 string/ordered-array clip binding runtime을 통합했다. 현재 Whirlwind active binding은
  단일 `mesh_att_battle_20_03`이고 authoring cue는 `0..2133ms`다.
- `Items.bootstrap` v2를 생성해 Server의 `Missing item bootstrap` 시작 blocker를 제거했다.
- Valtan world destruction Validate: PASS
- Client Debug `/t:ClCompile`: errors `0`, PASS
- Server Release build와 `Server.exe --contract-test`: failures `0`
- 04:09 Full `Publish-Effects.ps1 -Mode Publish`: exit `0`, Effects `192`, Components `1`, visual-program
  sidecar 파일을 게시했다. catalog와 sidecar LastWrite는 `2026-08-21 04:09:04`다.
- runtime `EffectCatalog.runtime.json`의 `effect.valtan.pattern.420633.active` row는 `1`이고 content
  SHA-256은 `0c27dee05f03e053f64587ca30e0a73e0be192fd9784b8150fa76179cd2c838d`다.
- 04:09 catalog의 marker는 `false`지만 현재 formatVersion 3 loader는 marker와 무관하게 sidecar를 항상
  parse/validate/stage한다. source/published sidecar는 byte-identical이고 420633 supplemental 4개가 유지된다.
- 현재 direct 191행은 legacy 6-field 전환 게시본이다. 최종 gate는 모든 authored 저장이 끝난 뒤 4-field
  full Publish와 Client 재시작이며 marker `true`는 요구하지 않는다.
- Client Release full Build: exit `0`, `Client.exe` 생성, warnings only.
- Debug Server는 실행 중인 프로세스가 출력 파일을 점유해 compile 뒤 link만 `LNK1104`였다.
  프로세스를 종료하지 않았다. 같은 소스의 Release link와 contract test는 통과했으며, 사용자가
  Debug Server를 재시작해야 새 bootstrap/v13이 실제 실행 프로세스에 적용된다.
- 새 전용 harness는 추가하지 않았다. 기존 builder check, validator, unit, frontend/contract test를
  재사용했다.
- Client를 실행·조작하지 않았고 visual PASS는 없다.
