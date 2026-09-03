# 2026-09-03 Valtan Composition Animation Tool Effect 연결 구현 계획서

작성 기준 브랜치 `GB/valtan-bugfix-koukusaydon-pattern`, HEAD `de3cc494`.
같은 날 `2026-09-03_VALTAN_GRIP_AND_PATTERN_REGRESSION_IMPLEMENTATION_PLAN.md`가
왼손 부착과 도끼/사자후/워프 회귀를 소유한다. 이 문서는 그와 겹치지 않고
`CActionCompositionWorkbench`(Composition Animation Tool)에서 Effect를 실제로 붙이는
경로만 소유한다.

읽은 정본: `AGENTS.md`, `CLAUDE.md`, `.md/GB/gotchas.md`, `.md/GB/gotchas.local.md`,
`.md/GB/계획서하네스규칙.local.md`, `.md/GB/local.md`, `.md/TEAM/README.md`.

## 0. 질문과 요구 사항

| # | 질문 / 요구 사항 | 답 | 담당 G |
|---|---|---|---|
| Q5 | 3연속 공격 기준으로 재생되는 이펙트가 없다 | `VALTAN_THREE`는 `STEP_01`에 V1 cue 하나뿐이고 실제 타격이 있는 `STEP_02`/`STEP_03`은 비어 있다. V2 binding도 없다 | G11 |
| Q6 | 3연속 공격 - 카운터 기준으로도 기존에 만들었던 이펙트가 없다 | 맞다. `effect.valtan.carrier-v1.reactive.triple-counter.first.clip-01`이 커밋 `0e2e84b0`에서 파일째 삭제되고 catalog에서도 빠졌다. 남은 V2 binding은 counter 표시용 pulse뿐이고 실제 내려치기 stage(`FAIL_1/2/3`)에는 아무것도 없다 | G11 |
| Q7 | 바닥 쾅 찍을 때 나는 effect를 V2 group에서 가져와 두 패턴에 적용할 수 있나 | 가능하다. 그 effect는 `boss.valtan.impact` V2 group이고 자식 17개다. Workbench의 `Append V2 Stage Binding` 버튼이 이미 GROUP 추가를 지원한다 | G11 |
| Q8 | Composition Animation Tool이 이 역할인데 save도 못 하고 pattern에 적용하면 tree가 다 깨진다 | 저장소가 지금 `split authoring Product drift` 상태다. 이 상태에서는 canonical admission 자체가 실패해 tree가 비고 모든 mutation과 Save가 잠긴다 | G10 |
| Q9 | 사자후도 처음 이펙트 말고 뒤에 포효하는 이펙트만 쓰고 싶다 | `boss.valtan.shout`(첫 확산 링)과 `boss.valtan.shout.burst`(포효)가 이미 별도 group이다. 앞 binding 하나만 제거하면 된다 | G12 |

## 1. 지금 저장소가 실제로 어떤 상태인가

### 1.1 Product drift로 Valtan 저작 전체가 잠겨 있다

```powershell
powershell -ExecutionPolicy Bypass -File Tools/ValtanPipeline/Publish-ValtanTuningRuntimeSet.ps1 -Mode Validate
```

실행 결과.

```json
{"command":"VALIDATE","ok":false,
 "errors":[{"errorCode":"VALIDATION_FAILED",
   "message":"split authoring Product drift; run PublishV2: Data/Encounters/Valtan/ValtanEncounter.json"}],
 "sourceRevision":"fd0f31d39bc89db5f698f4d0a74b1153fd67c48f9a2eddf9c26e61e81a914a28"}
```

drift 지점을 실측하면 `VALTAN_HIGH_JUMP / AIRBORNE`의 도끼 volley 한 블록이다.

| field | 현재 Product | source가 투영하는 값 |
|---|---|---|
| `spawnCount` | 1 | 3 |
| `spawnIntervalMs` | 0 | 1333 |
| `arenaRandomCount` | 0 | 4 |
| `arenaRandomRadiusM` | 0 | 14.0 |
| `arenaHeightToleranceM` | 0 | 1.0 |
| `arenaAnchorPolicy` | NONE | BOSS_SPAWN_POSITION |
| `maximumTotalObjects` | 4 | 36 |

`Data/Valtan/Valtan.gameplay.json`의 저작 source는 이미 3웨이브 + 랜덤 4로 복구돼 있고
(`git diff --stat` 기준 12 insert / 7 delete), 생성물 `ValtanEncounter.json`만 아직 옛 값이다.
즉 **source를 손으로 고친 뒤 projector를 돌리지 않은 상태**다.

### 1.2 그 결과 Tool이 왜 전부 잠기는가

`Publish-ValtanTuningRuntimeSet.ps1 -Mode SourceManifest`는 통과한다
(`splitJoinValidated: true`, `authoringRevision: null`). 그래서 Save가 막히는 지점은
split join이 아니라 그 뒤의 canonical 그래프 admission이다.

```text
CActionCompositionWorkbench::Reload_Canonical
  -> CBalanceTool 의 canonical 재적재
  -> validate_repository 가 Product drift 로 실패
  -> m_eAdmission != ADMITTED
  -> bMutationAdmitted = Can_MutateValtanView(m_eAdmission) && bEffectivePatternReady == false
```

`Client/Private/ActionCompositionWorkbench.cpp:11674`

```cpp
const bool_t bMutationAdmitted =
    Can_MutateValtanView(m_eAdmission) && bEffectivePatternReady;
...
const bool_t bPatternMutationAdmitted = bMutationAdmitted;
```

`bMutationAdmitted`가 false면 다음이 전부 비활성이 된다.

| 증상 | 근거 |
|---|---|
| tree가 비거나 깨져 보인다 | admission 실패 시 이전 snapshot을 read-only로만 보존한다 (`:1865`, `:1887`, `:1960`) |
| `Append V2 Stage Binding` 버튼이 눌리지 않는다 | `bV2AppendAdmitted`가 `bMutationAdmitted`를 요구 (`:11274`) |
| `Append V1 Effect to Pattern Draft`도 눌리지 않는다 | `bEffectAppendAdmitted`가 `bPatternMutationAdmitted`를 요구 (`:11209`) |
| Save가 처음부터 거부된다 | `Save_Reload`의 첫 gate가 `VALTAN_VIEW_ADMISSION::ADMITTED`를 요구 (`:4686`) |

그래서 "save도 못 하고, 동작하는지도 모르겠고, pattern에 적용하면 tree가 다 깨진다"는
Tool 자체의 결함이 아니라 **저장소가 잠긴 상태에서 Tool을 연 결과**다.

### 1.3 잠금을 푸는 명령

```powershell
powershell -ExecutionPolicy Bypass -File Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1 -Mode PublishV2
```

그 뒤 다시 Validate가 `ok: true`가 되어야 한다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/ValtanPipeline/Publish-ValtanTuningRuntimeSet.ps1 -Mode Validate
```

이 명령은 생성물을 쓴다. 지금 worktree에는 다른 세션의 미커밋 변경
(`Server/Private/GameRoom.cpp`, `ServerGameplayContractTests.cpp`,
`Tools/ValtanPipeline/test_valtan_rock_pillar_group_contract.py`, 09-02 문서 2개, `gotchas.md`)이
있으므로 실행 전에 소유자를 확인한다.

## 2. 3연속 공격과 카운터의 현재 Effect 실측

### 2.1 `VALTAN_THREE` — 3연속 내려치기

gameplay.

| stage | duration | hit |
|---|---|---|
| `STEP_01` | 1800 | NONE |
| `STEP_02` | 1200 | NONE |
| `STEP_03` | 2067 | `CONE 75도 / 15m`, `EXPLICIT_OFFSETS [500, 1350]`, `damage.valtan.ground-wave-smash` |

presentation.

| stage | clip | effectCues |
|---|---|---|
| `STEP_01` | `mesh_att_battle_2_01` 1800ms | `effect.valtan.project-tuned.sequence.three` 1개 |
| `STEP_02` | `mesh_att_battle_2_02` 1200ms | 없음 |
| `STEP_03` | `mesh_att_battle_2_03` 2067ms | 없음 |

V2 binding은 `VALTAN_THREE` 행이 하나도 없다.

`effect.valtan.project-tuned.sequence.three.effect.json`은 살아 있고 catalog에도 있으며
element 6개(cyan ring decal 3, sky-wave particle 3)다. 다만 `STEP_01`은 hit이 없는
준비 구간이라, 실제 타격 두 번(`STEP_03`의 500ms/1350ms)에는 아무 표현도 없다.

### 2.2 `VALTAN_TRIPLE_COUNTER` — 3연속 내려치기 카운터

gameplay.

| stage | duration | 성격 |
|---|---|---|
| `SETUP` | 2000 | WINDUP |
| `COUNTER_1` | 1800 | WINDUP, counterProxy `BOSS_FORWARD_ARC 180도`, `COUNTER_HIT` 분기 |
| `FAIL_1` | 1667 | ACTIVE, `CIRCLE 12m`, hit `[900]`, `damage.valtan.triple-counter` |
| `COUNTER_2` / `FAIL_2` / `COUNTER_3` / `FAIL_3` | 동일 반복 | |

presentation의 `effectCues`는 **모든 stage가 비어 있다**.

V2 binding은 `COUNTER_1/2/3`에만 있고 전부
`boss.valtan.project-tuned.sequence.trash.pulse-group`(자식 1개, 카운터 표시용 pulse)이다.

| bindingId | stage | clock |
|---|---|---|
| `binding.valtan.project-tuned.counter-pulse.013` | `COUNTER_1` | `CLIP_OCCURRENCE : 0 : ONCE` |
| `...014` | `COUNTER_1` | `600` |
| `...015` | `COUNTER_1` | `200` (clip.02) |
| `...016` `...017` `...018` | `COUNTER_2` | `0` / `600` / `200` |
| `...019` `...020` `...021` | `COUNTER_3` | `0` / `600` / `200` |

즉 **실제 내려치기(`FAIL_1/2/3`, 900ms 타격)에는 표현이 하나도 없다.**

### 2.3 사라진 이펙트의 정체

커밋 `0e2e84b0 valtan-pattern-bugfix`가 다음을 지웠다.

```text
Data/Effects/Authored/effect.valtan.carrier-v1.reactive.triple-counter.first.clip-01.effect.json   16555줄 삭제
Data/Effects/Authored/effect.valtan.project-tuned.sequence.trash.effect.json                        12673줄 -> 1 element
Data/Effects/EffectCatalog.json                                                                     위 두 항목 제거
```

`effect.valtan.carrier-v1.reactive.triple-counter.first.clip-01`이 사용자가 말한
"기존에 만들었던 이펙트"다. 파일이 없으므로 catalog 항목만 되살릴 수 없고,
복구하려면 그 커밋의 blob에서 파일을 되살려야 한다.

```powershell
git show 0e2e84b0^:Data/Effects/Authored/effect.valtan.carrier-v1.reactive.triple-counter.first.clip-01.effect.json > <복원 경로>
```

`effect.valtan.project-tuned.sequence.trash`는 파일이 남아 있지만 element 1개로 줄었고
catalog에서 빠져 현재 orphan이다.

## 3. 바닥 쾅 이펙트의 정체와 위치

사용자가 말한 "바닥 쾅 찍을 때 발생하는 effect"는 `boss.valtan.impact` V2 group이다.

`Data/Effects/V2/Groups/boss.valtan.impact.effectv2group.json`, 자식 17개.

| leaf | 개수 | startMs | 성격 |
|---|---|---|---|
| `boss.valtan.hit_1` | 1 | 0 | 타격 섬광 |
| `boss.valtan.hit_2` | 1 | 0 | 타격 섬광 |
| `boss.valtan.hit_3` | 3 | 0 / 30 / 60 | 전방으로 1.5, 3.0 밀려나가는 잔상 |
| `boss.valtan.decal_1` | 1 | 0 | 바닥 자국 |
| `boss.valtan.decal_2` | 1 | 0 | 바닥 자국 |
| `boss.valtan.spread_1` | 5 | 0 / 50 / 100 / 150 / 200 | scale 1.0 -> 1.5로 커지며 전방 확산 |
| `boss.valtan.spread_2` | 5 | 0 / 40 / 80 / 120 / 160 | scale 1.0 -> 1.2로 후방 확산 |

현재 이 group을 쓰는 binding은 넷이다.

| scope | clock |
|---|---|
| `VALTAN_SEQUENCE_FOUR / STEP_01` | `CLIP_OCCURRENCE 1233 / 2233 / 3233 / 4200 : ONCE` |
| `VALTAN_STRUGGLING / STEP_04` | 같은 네 시각 : ONCE |
| `VALTAN_FRONT_BACK_FRONT / SMASHES` | 같은 네 시각 : EACH_LOOP |
| `VALTAN_GROUND_ROAR / STEP_01` | `STAGE : 0 : ONCE` |

즉 "4연속 내려치기 4번"에 1233ms 간격으로 붙어 있다. 3연속에도 같은 group을 그대로 쓸 수 있다.

Resources 실물 경로는 각 leaf 문서
`Data/Effects/V2/Authored/boss.valtan.<leaf>.effectv2.json`이 소유하고,
그 안의 asset ID는 `Client/Bin/Resources` 상대 경로다.
`CRuntimeAssetRoot::Resolve()`가 해석하므로 코드에 경로를 박지 않는다.

## 4. Composition Animation Tool이 지금 할 수 있는 것과 못 하는 것

### 4.1 두 개의 Append 버튼

`Client/Private/ActionCompositionWorkbench.cpp:11216` `Append V1 Effect to Pattern Draft`

```text
전제  pResourcePattern, pResourceStage, pEffectTargetClip(Animation box 선택),
      m_pBalanceTool, bPatternMutationAdmitted, stage role != WAIT, V1 선택
저장  Valtan.presentation.json 의 effectCues 한 행
기본  anchor root / follow / natural / once / OWNER_RELATIVE / sourceStartMs = clip 시작
```

`Client/Private/ActionCompositionWorkbench.cpp:11276` `Append V2 Stage Binding`

```text
전제  pResourcePattern, pResourceStage, bMutationAdmitted, asset 선택, V2(LEAF 또는 GROUP)
입력  Stage-local start (ms) 하나
저장  Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json 한 행
```

V2 Append가 강제로 박는 값 (`Client/Private/EffectV2_Catalog.cpp:1171`).

```cpp
Binding.eClockBasis     = EFFECT_V2_CLOCK_BASIS::STAGE;   // clip occurrence 지정 불가
Binding.strClipOccurrenceId.clear();
Binding.iStartMs        = SourceKey.iStartMs;
Binding.eRepeatPolicy   = ONCE;                            // EACH_LOOP 불가
Binding.strAnchorSlotId = "b_effectroot";
Binding.eFollowPolicy   = SNAPSHOT_AT_START;
Binding.eRotationBasis  = TARGET_YAW;
Binding.LocalTransform  = {};
Binding.eStopPolicy     = NATURAL;
```

### 4.2 mutation API 전체

`Client/Public/EffectV2_Catalog.h:94`

| 동작 | 함수 | 가능 |
|---|---|---|
| 추가 | `Stage_AppendBossValtanStageBinding` | LEAF / GROUP 모두 |
| 제거 | `Stage_RemoveBossValtanStageBinding` | key가 `bindingId`뿐이라 CLIP_OCCURRENCE 행도 대상 |
| 복제 | `Stage_DuplicateBossValtanStageBinding` | 가능 |
| 시작 시각 변경 | `Stage_UpdateBossValtanStageBindingStart` | 타임라인 드래그로 연결됨 |
| group 자식 편집/분할 | 없음 | group 문서 mutation API 자체가 없다 |

### 4.3 Save 트랜잭션

`Client/Private/ActionCompositionWorkbench.cpp:4685` `Save_Reload`가 세 owner를 한 번에 커밋한다.

```text
bSavePattern   = Balance draft dirty        -> Valtan.gameplay.json / Valtan.presentation.json
bSaveSound     = Pattern Sound draft dirty  -> Valtan.patternsoundcues 계열
bSaveEffectV2  = CEffectV2Catalog::Has_BossValtanBindingDraft()
                                            -> BOSS_VALTAN.effectv2bindings.json
```

세 개가 모두 clean이면 `"There are no staged Pattern, Sound, or Effect V2 changes to save."`로
조용히 성공 반환한다. V2 binding만 바꾼 경우 `BuildValtanDraftPatch`가 0 operation을 만들고
`commit_typed_authoring_patch`가 빈 operations 배열을 정상 처리하므로
**V2 binding 단독 Save는 구조적으로 지원된다.**

Save 직전 gate 순서는 다음과 같다.

```text
1  m_eAdmission == ADMITTED                        아니면 "Save is not ready"
2  pinned authoring/canonical revision 이 현재와 동일  아니면 STALE_PRESERVED 로 내려가고 거부
3  세 owner 중 하나라도 dirty                        아니면 "no staged changes"
4  m_pBossTool, m_pAnimationTool 존재
5  Animation / Sound 의존성 검증
6  owner draft stage (baseline/candidate 쌍)
7  CommitCanonicalDraft (writer lock, split 복원, Product 재투영, CAS)
8  Boss Tool 재적재 -> Workbench Reload_Canonical
```

지금은 1번에서 막히고 있다.

## 5. G10 — 저장소 잠금 해제와 재발 방지

### 5.1 목표와 종료 증거

Composition Animation Tool이 다시 ADMITTED로 열리고 Append/Save가 활성화돼야 한다.
종료 증거는 `Publish-ValtanTuningRuntimeSet.ps1 -Mode Validate`가 `ok: true`를 반환하는 것이다.

### 5.2 절차

```powershell
# 1. drift 확인
powershell -ExecutionPolicy Bypass -File Tools/ValtanPipeline/Publish-ValtanTuningRuntimeSet.ps1 -Mode Validate

# 2. source -> Product 재투영
powershell -ExecutionPolicy Bypass -File Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1 -Mode PublishV2

# 3. 재확인
powershell -ExecutionPolicy Bypass -File Tools/ValtanPipeline/Publish-ValtanTuningRuntimeSet.ps1 -Mode Validate
```

2번이 바꾸는 파일은 `Data/Encounters/Valtan/ValtanEncounter.json`과
그 외 generated Product다. `Data/Valtan/*.json` 저작 source는 건드리지 않는다.

### 5.3 재발 방지

지금 상황의 근본 원인은 "저작 source를 손으로 고칠 수 있는데
Product 재투영이 자동으로 붙어 있지 않다"는 것이다. 두 가지를 붙인다.

```text
G10-A  Workbench 가 admission 실패 사유를 화면에 그대로 노출한다
       현재는 "이전 스냅샷을 read-only 로 보존" 문구만 나오고
       "run PublishV2" 라는 실제 복구 명령이 사용자에게 보이지 않는다
       CBalanceTool 이 pipeline diagnostic 문자열을 그대로 m_strStatus 로 전달하고
       Workbench 가 그 문자열에 PublishV2 안내를 붙인다

G10-B  Tools/Build/Invoke-BuildAndRegression.ps1 의 변경 domain 검증에
       Valtan source 가 dirty 일 때 -Mode Validate 를 실행하는 단계를 넣는다
       실패 시 build 를 멈추고 PublishV2 를 안내한다
```

G10-A는 Client 한 파일, G10-B는 빌드 스크립트 한 곳이다.
두 항목 모두 새 런타임 경로를 만들지 않는다.

## 6. G11 — 3연속 공격과 카운터에 바닥 쾅 이펙트 연결

### 6.1 목표와 종료 증거

`VALTAN_THREE`의 실제 타격 두 번과 `VALTAN_TRIPLE_COUNTER`의 내려치기 세 번에
`boss.valtan.impact` group이 재생돼야 한다.
자동 종료 증거는 Save 성공과 `-Mode Validate` 통과까지이고,
최종 화면 판정은 사용자가 직접 한다.

### 6.2 저작 조작 (코드 변경 없음)

G10이 끝나 Tool이 ADMITTED가 되면 다음 순서로 조작한다.

```text
Composition Animation Tool -> Patterns 에서 VALTAN_THREE 선택
Resources 탭 -> Effect -> V2 Effect Groups -> boss.valtan.impact 선택
Sequencer 에서 대상 Stage 박스를 선택
Stage-local start (ms) 입력
Append V2 Stage Binding
```

권장 시작 시각은 Server hit 시각과 맞춘다.

| pattern | stage | duration | Server hit | 권장 startMs |
|---|---|---|---|---|
| `VALTAN_THREE` | `STEP_02` | 1200 | 없음(예비 타격 연출) | 사용자 판단 |
| `VALTAN_THREE` | `STEP_03` | 2067 | 500, 1350 | 500 과 1350 두 행 |
| `VALTAN_TRIPLE_COUNTER` | `FAIL_1` | 1667 | 900 | 900 |
| `VALTAN_TRIPLE_COUNTER` | `FAIL_2` | 1667 | 900 | 900 |
| `VALTAN_TRIPLE_COUNTER` | `FAIL_3` | 1667 | 900 | 900 |

`STEP_03`처럼 한 stage에 두 번 필요하면 첫 행을 추가한 뒤
그 박스를 선택하고 `Duplicate`로 두 번째 시각을 만든다.

`VALTAN_THREE`의 stage들은 clip occurrence가 각각 하나뿐이라
STAGE clock과 clip clock이 사실상 같다. `VALTAN_TRIPLE_COUNTER`의 `FAIL_*`도
occurrence가 하나이므로 STAGE clock으로 충분하다.

### 6.3 삭제된 V1 이펙트를 되살릴 경우

`effect.valtan.carrier-v1.reactive.triple-counter.first.clip-01`을 다시 쓰려면
파일 복원과 catalog 재등록을 한 변경 단위로 묶는다.

```text
1  git show 0e2e84b0^:Data/Effects/Authored/effect.valtan.carrier-v1.reactive.triple-counter.first.clip-01.effect.json
   를 같은 경로로 복원
2  Data/Effects/EffectCatalog.json 에 effectAssetId / payloadKind / authoringPath 3필드 항목 복원
3  Tools/EffectPipeline/Validate-EffectSources.ps1 로 dependency closure 검증
4  Workbench 에서 Append V1 Effect to Pattern Draft 로 원하는 Animation box 에 연결
```

V1 복원과 V2 group 연결은 배타적이지 않다. 다만 같은 시각에 둘 다 붙이면
표현이 겹치므로 한쪽만 고르는 것을 권한다.
이 결정은 사용자 화면 판정 영역이라 계획서에서 정하지 않는다.

### 6.4 V2 Append의 한계와 회피

`Append V2 Stage Binding`은 anchor `b_effectroot`, `SNAPSHOT_AT_START`, `TARGET_YAW`,
identity transform, `ONCE`로 고정된다. 3연속 내려치기처럼 보스 발밑에서
정면으로 퍼지는 표현은 이 기본값이 그대로 맞다
(`VALTAN_SEQUENCE_FOUR`의 기존 4행이 같은 group을 쓰고 있다).

다음이 필요하면 이 G의 저작 조작으로는 불가능하다.

```text
clip occurrence 기준 clock
EACH_LOOP
b_effectroot 이외의 anchor
group 자식별 yaw / scale / translation 변경
```

앞의 셋은 G13의 Append 확장, 마지막 하나는 group 문서를 새로 저작해야 한다.

### 6.5 검증

```powershell
powershell -ExecutionPolicy Bypass -File Tools/EffectPipeline/Validate-EffectSources.ps1
powershell -ExecutionPolicy Bypass -File Tools/ValtanPipeline/Publish-ValtanTuningRuntimeSet.ps1 -Mode Validate
python Tools/ValtanPipeline/test_valtan_pattern_master_v2.py
```

## 7. G12 — 사자후에서 뒤쪽 포효만 남기기

### 7.1 두 group의 실제 내용

| group | 자식 | 성격 | 사용자 표현 |
|---|---|---|---|
| `boss.valtan.shout` | 20 | `shout.comet_1` 9 + `shout.comet_2` 9가 yaw 0/45/60/90/135/180/225/270/315로 startMs 200~1650 계단식, `shout.converge_1` 0ms, `blur_3` 0ms | 처음 이펙트 |
| `boss.valtan.shout.burst` | 6 | `shout.fog_1~4`, `shout.emit_1`, `blur_4` 전부 0ms | 후에 포효하는 이펙트 |

### 7.2 피자 패턴에 걸린 두 행

| bindingId | resource | scope | clock |
|---|---|---|---|
| `binding.valtan.project-tuned.six-pizza.001.shout-loop` | GROUP `boss.valtan.shout` | `VALTAN_SIX_PIZZA_106 / STEP_06` | `CLIP_OCCURRENCE : 0 : EACH_LOOP` |
| `binding.valtan.project-tuned.six-pizza.002.shout-burst` | GROUP `boss.valtan.shout.burst` | `VALTAN_SIX_PIZZA_106 / STEP_07` | `CLIP_OCCURRENCE : 733 : ONCE` |

`STEP_06`은 duration 8000ms이고 clip이 `LOOP_TO_STAGE_END`라
`EACH_LOOP` binding이 8초 내내 20개 자식을 반복 재생한다.

### 7.3 조작

```text
Composition Animation Tool -> VALTAN_SIX_PIZZA_106 선택
Sequencer 에서 STEP_06 의 V2 Group | boss.valtan.shout 박스 선택
Remove
Save
```

`Validate_NoLeafGroupClockOverlap`(`Client/Private/EffectV2_Catalog.cpp:601`)은
같은 leaf가 같은 유효 clock에 중복될 때만 거부하므로 이 조합은 걸리지 않는다.

같은 방식이 다른 사자후 패턴에도 적용된다. `boss.valtan.shout`가 걸린 scope는
`VALTAN_BIND_SLOT/STEP_01`(3행), `VALTAN_ROAR_CHARGE/STEP_02`,
`VALTAN_IMPRISON_ROAR/ROAR`, `VALTAN_STRUGGLING/STEP_09`,
`VALTAN_GHOST_TRANSITION_15/FOUR_DIRECTIONS`, `VALTAN_TERRAIN_DESTRUCTION/STEP_10`이다.
어느 패턴에서 앞 링을 뺄지는 패턴별로 따로 판단한다.

### 7.4 자식 일부만 쓰고 싶을 때

`boss.valtan.shout`의 20개 중 일부만 원하면 새 group 문서를 만든다.

```text
경로     Data/Effects/V2/Groups/boss.valtan.shout.roar-only.effectv2group.json
필수     schema "lostark.effect-v2-group", formatVersion 2,
         groupId 가 파일명과 일치, children 은 LEAF 만 (중첩 group 금지)
검증     Isolate_InvalidCrossReferences 가 leaf 실재를 검사한다
사용     Workbench Resources -> V2 Effect Groups 에서 선택 후 Append
```

group 자식의 yaw/startMs는 Tool에서 만들 수 없으므로 이 파일은 손으로 저작한다.

## 8. G13 — Tool을 "진짜 이 역할"로 만들기 위한 확장

사용자가 원하는 최종 상태는 "effect와 resource를 Tool 안에서 고르고
Sequencer 편집만으로 끝난다"이다. 그러려면 아래 세 가지가 더 필요하다.
이 G는 범위와 소유자만 정하고 이번 변경 단위에서 구현하지 않는다.

```text
G13-A  Append V2 Stage Binding 에 clock basis, clipOccurrenceId, repeatPolicy,
       anchorSlotId, followPolicy, rotationBasis, localTransform 입력을 추가한다
       소유자는 CEffectV2Catalog::Stage_AppendBossValtanStageBinding 의 인자 확장이며
       Mutate_BossValtanStageBinding 의 APPEND 분기에서 고정값을 지운다

G13-B  선택한 binding 의 anchor/follow/transform/repeat 를 사후 편집하는
       typed mutation 을 추가한다. 지금은 startMs 만 바꿀 수 있다

G13-C  group 문서 stage/commit API 와 자식 편집 패널을 추가한다
       group 을 복제해 자식 일부만 남기는 조작이 Tool 안에서 끝나야
       "사자후에서 앞 링만 빼기" 같은 요구가 파일 편집 없이 닫힌다
```

세 항목 모두 저장 owner는 이미 있는 `BOSS_VALTAN.effectv2bindings.json`과
`Data/Effects/V2/Groups/*.effectv2group.json`이다.
새 런타임 경로나 두 번째 Effect 재생기를 만들지 않는다.

## 9. 진행 순서

```text
G10  저장소 잠금 해제. 이게 끝나기 전에는 G11 / G12 조작이 UI 에서 비활성이다
G11  3연속 공격과 카운터에 boss.valtan.impact 연결. 저작 조작이라 코드 커밋 없음
G12  사자후 앞 링 binding 제거. 저작 조작이라 코드 커밋 없음
G13  Tool 확장. 사용자 결정 후 별도 커밋
```

G11과 G12는 같은 `BOSS_VALTAN.effectv2bindings.json` 한 파일을 바꾸므로
한 Save 트랜잭션으로 함께 커밋해도 된다.

## 10. 이 계획서가 하지 않는 것

```text
Client 또는 UI 자율 실행과 화면 캡처
사용자 서면 판정 없는 visual PASS 기록
다른 세션의 미커밋 변경 정리
PublishV2 자동 실행 (worktree 소유자 확인 후 사용자가 실행)
삭제된 V1 이펙트의 자동 복원 (사용자가 V1 복원과 V2 group 중 선택)
group 자식 편집 UI 구현 (G13-C 로 범위만 기록)
```
