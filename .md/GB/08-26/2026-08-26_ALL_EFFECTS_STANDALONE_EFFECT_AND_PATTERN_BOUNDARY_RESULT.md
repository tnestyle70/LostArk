# All Effects 발탄 Runtime Product·서버 권위·독립 Effect 경계 구현 결과

## 0. 결과

기존 발탄 권위 Effect는 삭제되지 않았다. 현재 정본에는 Product cue 51개와 unique Effect asset 48개가
남아 있고, 이 asset들과 combat-object 도끼 Effect는 `EffectCatalog.json` 및 drawable authored 문서로
resolve된다.

문제는 이전 All Effects projection이 이 실제 runtime inventory를 숨기고 Pattern마다 빈 aggregate
sidecar 하나만 `(not created)`로 표시한 것이었다. 현재 구현은 이 projection을 바로잡았다.

```text
Pattern                                      [Play Server]
  Runtime Product Effects
    published Product cue                    [Open Editor] [Play Effect + Animation]
    STAGE_CLOCK Product cue                  [Open Editor] [Play Effect]
    combat-object visual                     [Open Editor] [Play Effect + Animation]
  Unpublished Pattern Draft                  [Open Draft]

Independent Effect
  도넛                                       [Open Editor] [Play Effect] [Play Server Owner]
  플레이어 추적 도끼                        [Open Editor] [Play Effect + Owner Animation]
                                              [Play Server Owner]
```

Pattern `Play Server`는 그대로 Valtan Arena의 기존 Server fixed-tick audition 경로를 사용한다. 로컬
Effect/animation preview와 Server pattern 검증을 다시 분리했으며, sidecar draft는 published runtime
Effect를 대체하지 않는다.

## 1. 실제 구현 상태

### 1.1 Pattern Runtime Product Effects 복원

`Render_ValtanPatternNode()`는 각 Pattern의 Stage에서 다음 runtime row를 수집한다.

- clip-bound `Stage.ProductCues`
- `STAGE_CLOCK` Product cue
- `Stage.CombatObjectEffects`

row는 `effectAssetId`로 catalog의 exact authored Effect 문서를 resolve한다. `Open Editor`로 문서를 열 수
있고, clip-bound Product는 `Play Effect + Animation`, stage-clock Product는 `Play Effect`, combat-object
visual은 owner timeline과 Effect offset을 사용한 로컬 재생을 제공한다.

`Open Editor`는 clip/owner animation binding을 준비하더라도 synchronized animation을 time 0에서 pause한다.
따라서 단순 열기에서 animation이 먼저 흘러가는 문제를 막고 Play 버튼과 동작을 구분했다. unsaved-document
modal을 거친 pending load도 같은 preview intent와 pause/play 구분을 유지한다.

Pattern 검색도 Pattern 이름만 보지 않고 Stage, clip occurrence, Product cue, Stage effect,
combat-object effect를 포함한다.

### 1.2 Server Play 유지

Pattern row의 `Play Server`는 `CValtanPatternAuditionService::Submit()`에 stable pattern ID를 제출한다.
Arena 밖, Server 미연결, boss 미활성 또는 audition busy 상태에서는 fail closed한다.

Server는 다음을 계속 소유한다.

- pattern/action/stage와 fixed tick
- target 추적, CHASE, 사거리와 공격 전 delay
- damage/hit, combat-object wave와 실제 위치
- replicated presentation stage
- 현재 publish된 Product cue와 combat-object presentation 발생 시점

Character Select local clock 또는 Effect Tool 자체 timeline을 Server 합격 근거로 추가하지 않았다.

### 1.3 Independent 도넛·도끼

Independent root row는 두 Effect를 유지한다.

```text
valtan.independent-effect.donut-in-out
  owner: VALTAN_FIST_IN_OUT / INNER / SERVER_PATTERN_STAGE

valtan.independent-effect.target-axe
  owner: VALTAN_HIGH_JUMP / AIRBORNE / SERVER_COMBAT_OBJECT
```

- 도넛은 `Open Editor`, Effect-only `Play Effect`, 실제 owner를 실행하는 `Play Server Owner`를 제공한다.
- 도끼는 `Open Editor`, AIRBORNE stage offset을 반영한 `Play Effect + Owner Animation`, 실제 추적/wave/hit을
  실행하는 `Play Server Owner`를 제공한다.
- owner Pattern이 visible 2+26 목록 밖에 있어도 exact stable ID를 resolve하면 `Play Server Owner`가 실제
  audition service를 사용할 수 있다.

Independent root와 Pattern runtime row에 같은 asset이 모두 보이는 것은 의도된 결과다. 전자는 독립 저작
진입점이고 후자는 실제 Pattern 소비 occurrence다.

### 1.4 FIST 계약 보존

`VALTAN_FIST_IN_OUT`은 과거 4-stage animation sequence로 복원하지 않았다. 현재 명시 계약을 그대로
유지했다.

```text
stage count: 1
stage: INNER, 2600ms
animation.mode: NONE
Product timingBasis: STAGE_CLOCK
clipOccurrenceId: 없음
gameplay ring: 8~16
hit offset: +1600ms
```

도넛의 몸 animation이 없는 것은 손실이 아니라 현재 저작 계약이다. Effect 모양은 `Play Effect`, 실제
Server timing과 hit은 `Play Server Owner`로 각각 확인한다.

### 1.5 sidecar 초안 분리

기존 `New Effect` 2문서 CAS transaction은 유지했다. 다만 Pattern의 유일한 Effect로 가장하지 않고,
binding이 있을 때만 `Unpublished Pattern Draft` section에 표시한다.

이 draft는 다음 runtime 정본에 자동 publish되지 않는다.

- `Valtan.patterneffectcues.json`
- `Valtan.patternbindings.json`
- `EffectCatalog.json`
- gameplay/presentation Product

따라서 Pattern `Play Server`는 draft가 아니라 현재 published binding만 소비한다.

## 2. PR #232 통합

### 2.1 선행 gameplay/client 수정

dirty working tree에 merge 전체를 강제하지 않고 PR #232의 변경을 commit별로 대조해 현재 분기에 반영했다.

- held-LMB basic attack은 최초 hold 180ms, 반복 100ms scheduler와 성공 제출 뒤 commit 경계를 그대로 반영했다.
- Character Select audition의 authored cooldown을 덮어쓰던 고정 3초 override를 제거했다.
- Lance Master, Artist, Dimension Master, Warlord의 max HP 50,000과 provenance를 같이 반영했다.
- Animation Tool의 예전 no-blend hunk는 현재 코드에 같은 호출 지점이 없어 덮어쓰지 않았다. 현재 master
  timeline은 `CValtan` Product sampler에서 clip edge를 직접 시작하고 track position을 기록한 뒤 blend 0으로
  sample하므로 PR의 목적을 이미 포함하는 후속 구현을 보존했다.

### 2.2 한국어 P2 Pattern 반영

PR #232의 animation-chain promotion stable ID와 표시 이름을 split gameplay/presentation 및 생성 closure에
반영했다. 20개 manual audition 중 15개는 한국어 이름으로 승격되었고, PR에서 유지한 5개는 기존
`[P2 Animation]` 이름을 보존한다.

주요 확인 대상은 다음과 같다.

```text
VALTAN_SIX_PIZZA_106       중앙이동 후 6방향 공격 후 피자 패턴
VALTAN_ATTACK_WHIRLWIND    점프찍기 후 휠윈드
VALTAN_ROAR_CHARGE         사자후 후 위로 모아치기
VALTAN_TERRAIN_DESTRUCTION 2페이즈 지형파괴 패턴
VALTAN_TRASH               버러지 패턴
VALTAN_COUNTER             카운터 쳐야 하는 내려치기
VALTAN_STRUGGLING          3페이즈 전 발악패턴
```

`Valtan.presentation.debug.json`의 검증 chain은 repeat 12까지 확장되었고, promotion closure는
20 Pattern / 99 source occurrence / 99 stage / 7 projected artifact를 기준으로 검증된다. All Effects의
animator 목록은 `ManualAuditions`에서 현재 stable ID와 이름을 읽으므로 별도 한국어 복사본을 만들지 않았다.

## 3. 변경 파일

### All Effects 경계

- `Client/Public/Effect_Tool.h`
- `Client/Private/Effect_Tool.cpp`
- `Tools/EffectPipeline/test_effect_tool_valtan_all_effects_contract.py`
- 기존 `ValtanPatternAuthoringEffectDocument`와 sidecar transaction은 draft 전용으로 유지

### PR #232 P2 promotion

- `Data/Valtan/Valtan.animation-chain-promotions.json`
- `Data/Valtan/Valtan.presentation.debug.json`
- `Data/Valtan/Valtan.gameplay.json`
- `Data/Valtan/Valtan.presentation.json`
- promotion publisher가 관리하는 Encounter/pattern binding/receipt closure
- `Tools/ValtanPipeline/test_valtan_animation_chain_promotion.py`

### PR #232 gameplay/client 선행 변경

- `Client/Public/PlayerController.h`
- `Client/Private/PlayerController.cpp`
- `Server/Private/GameRoom.cpp`
- `Data/Balance/PlayerProfiles.json`
- `Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json`

working tree에는 다른 세션의 변경이 함께 있으므로 이 RESULT는 stage/commit 소유권을 주장하지 않는다.

## 4. 자동 검증

### PASS — All Effects focused contract

```text
python -B Tools/EffectPipeline/test_effect_tool_valtan_all_effects_contract.py
  9/9 PASS
```

이 검증은 다음을 포함한다.

- Product cue 51개 / unique asset 48개 유지
- `effect.valtan.*` catalog 58개 이상과 cue/도끼 authored drawable document identity
- Pattern `Runtime Product Effects`, `Open Editor`, local Play, `Play Server` 경계
- independent 도넛/도끼 owner resolution과 `Play Server Owner`
- FIST `NONE + STAGE_CLOCK`, clip occurrence 없음
- sidecar draft의 2문서 CAS와 runtime 비승격
- core 6 + animator 20 projection
- `Open Editor`와 pending load가 owner animation을 time 0에서 pause하는 경계

기존 saved-row 회귀 suite도 현재 계약에 맞춰 의미 없는 수량 snapshot을 제거하고 다시 실행했다.

```text
python -B Tools/EffectPipeline/test_effect_tool_valtan_saved_rows.py
  32/32 PASS (7 intentional skips)
```

FIST outer/recovery의 empty `playbackMode:NONE` tombstone 2개가 runtime projection에서 빠지는 이유,
모든 canonical Product Effect의 drawable invariant, HIGH_JUMP의 1933ms AIRBORNE offset과 owner timeline,
독립 Effect의 local/server 버튼 분리를 직접 검증한다.

### PASS — PR #232 P2 promotion closure

```text
python -B Tools/ValtanPipeline/promote_valtan_animation_chains.py --repo-root . --mode Validate
  patternCount: 20
  sourceOccurrenceCount: 99
  stageCount: 99
  projectedArtifactCount: 7

python -B Tools/ValtanPipeline/test_valtan_animation_chain_promotion.py
  6/6 PASS
```

한국어 stable ID는 PR 이후 추가된 RootMotion, sound, Server contract test 참조까지 전파했다. 이어서 실제
소비자 closure를 다음과 같이 확인했다.

```text
Valtan Pattern Master PublishV2
  changed Product: 1 (ordered rotations)

Valtan Pattern Master ValidateV2
  managedPatterns: 27
  projectedArtifacts: 9
  PASS

Valtan tuning/balance runtime set Validate
  patterns: 53
  stages: 227
  audition rows: 52
  PASS

Valtan RootMotion fresh bake + check
  patterns: 44
  animation-bearing stages: 98
  samples: 5346
  PASS
```

즉 manifest와 split source만 이름이 바뀐 상태가 아니라, ordered rotation Product와 runtime balance set,
RootMotion consumer까지 동일 stable ID closure를 소비한다.

### PASS — Effect Tool focused compile

```text
Client/Private/Effect_Tool.cpp | Debug x64 selected-file ClCompile
  PASS
```

Pattern runtime row, independent owner preview, `Open Editor` pause 분기를 포함한 최신 소스가 focused compile을
통과했다.

### PASS — Debug Client/Server build와 runtime contract

```text
Client/Default/Client.vcxproj | Debug x64
  PASS -> Client/Bin/Debug/Client.exe

Server/Default/Server.vcxproj | Debug x64
  PASS -> Server/Bin/Debug/Server.exe

Tools/NetworkProtocolHarness/Default/NetworkProtocolHarness.vcxproj | Debug x64
  PASS

NetworkProtocolHarness.exe
  failures: 0

Server.exe --contract-test
  failures: 0
```

첫 link 시 실행 중이던 기존 `Client.exe`와 `Server.exe`가 각각 출력 파일을 점유해 `LNK1104`가 났다.
해당 두 프로세스만 종료하고 동일 build를 다시 실행해 최종 link까지 통과했다. 에이전트가 새 Client/Server를
실행하지는 않았으므로 사용자는 수동 smoke 전에 `Server + Client` profile을 다시 시작해야 한다.

전체 working tree `git diff --check`도 PASS다. 기존 line-ending 안내와 DirectXTK PDB link warning은 있었지만
compile/link 실패는 없다.

## 5. 사용자 수동 시각 검증 대기

자동 검증은 데이터와 호출 계약을 확인했지만 실제 화면 occurrence의 visual PASS는 사용자 확인 전이다.

1. 사용자가 Debug `Server + Client` profile을 실행한다.
2. Valtan Arena에 입장하고 실제 `boss.valtan.center`를 활성화한다.
3. `F1 -> Effect Tool -> All Effects -> Character / Boss: Valtan`을 연다.
4. Pattern을 펼쳐 `(not created)`만 보이던 자리에 `Runtime Product Effects`와 실제 Effect row가 나오는지
   확인한다.
5. `Open Editor`가 Effect를 열되 animation을 time 0에서 멈추는지, `Play Effect + Animation`이 연결
   sequence를 재생하는지 확인한다.
6. Pattern `Play Server`가 실제 Server pattern 전체를 재생하는지 확인한다.
7. 도넛은 `Play Effect`와 `Play Server Owner`, 도끼는 `Play Effect + Owner Animation`과
   `Play Server Owner`를 각각 비교한다.
8. animator 목록에서 한국어 P2 이름과 `2페이즈 지형파괴 패턴`을 확인한다.

에이전트는 Client를 자율 실행하지 않았고 visual PASS를 기록하지 않았다.

## 6. 현재 판정

```text
권위 Product Effect inventory        보존 확인
Pattern runtime Effect UI            구현 및 focused contract PASS
Independent 도넛/도끼 owner 경계     구현 및 focused contract PASS
FIST NONE + STAGE_CLOCK              보존 및 focused contract PASS
PR #232 한국어 P2 promotion          Validate 및 harness PASS
V2 Product/balance/RootMotion closure PASS
Effect_Tool.cpp focused compile       PASS
전체 Client/Server Debug build/link   PASS
Network/Server contract               failures 0
saved-row regression suite            32/32 PASS (7 intentional skips)
git diff --check                      PASS
Valtan Arena EXE 시각 검증            사용자 확인 대기
```

현재 구현은 기존 권위 Effect를 aggregate 초안으로 대체하지 않는다. 자동 구현·데이터·build 계약은 닫혔고,
PR 제출 전 남은 것은 사용자의 Arena 화면 occurrence/visual 판정과 shared dirty working tree에서 실제 PR에 넣을
변경 단위 선별이다.

## 7. 2026-08-27 선택 Effect 삭제와 재발 방지 보강

사용자가 Pattern 아래의 Product Effect 전체를 없애려고 마지막 Element까지 지웠지만, Effect 문서는 drawable
Element를 최소 하나 요구하므로 그 동작은 올바른 삭제 단위가 아니었다. All Effects에 다음 소유권-aware 경계를
추가했다.

- `Create Effect` 오른쪽 `Delete Effect`와 explicit confirmation modal
- Product row: 선택 Pattern의 exact cue 연결만 split presentation에서 unlink, shared asset/catalog/file과 다른
  Pattern 연결 보존
- `DRAFT_ATTACHED`: deterministic aggregate sidecar row와 exact authored file을 CAS transaction으로 삭제
- unsaved 작업, stale pattern/effect/cue set, V1 alias, combat-object-only row, Product에 등록된 Draft 파일 삭제 거부
- 삭제 transaction 뒤 render-frame pattern/binding pointer를 재사용하지 않는 즉시 return 경계

Model View가 같은 Valtan asset을 새 target generation으로 교체했을 때 synchronized clip이 비워진 뒤 영구적으로
animation이 사라지던 P1도 함께 막았다. Pattern Draft는 generic sequence update 전에 새 generation으로 동일
Pattern timeline을 다시 stage한다. Create는 Effect+sidecar durable commit 이후 auto-open 실패를 rollback하지 않고
원인을 `files remain committed`로 분리 보고한다.

현재 실행 중인 Debug Client/Server는 사용자의 Effect 튜닝 세션이므로 빌드와 링크를 수행하지 않았다. 현 시점에
실행한 자동 검증은 다음과 같다.

```text
python Tools/EffectPipeline/test_effect_tool_valtan_all_effects_contract.py
  16/16 PASS

python Tools/EffectPipeline/test_effect_tool_valtan_saved_rows.py
  32/32 PASS (7 intentional skips)

python Tools/EffectPipeline/test_valtan_pattern_effect_unlink.py
  7/7 PASS

python Tools/ValtanPipeline/test_valtan_pattern_tree_contract.py
  19/19 PASS

PowerShell AST parse
  Project-ValtanPatternMaster.ps1 PASS
  Remove-ValtanPatternEffectLink.ps1 PASS

focused git diff --check
  PASS (line-ending notice only)
```

실제 현재 `VALTAN_ARENA_BREAK_109` 선택 Product row에 대한 unlink `Validate`도 read-only로 PASS했고 Data는
변경하지 않았다. focused compile과 최종 Client Debug build는 실행 중 EXE를 사용자가 종료한 다음의 검증
항목이다. 이번 세션에서 발생한 ordered entry strict join, entry-required Map Effect prewarm, stale provider
obj/중복 build, publisher wrapper exit code, Model View target generation 및 Create auto-open commit 경계는 공용
`.md/GB/gotchas.md`와 `CLAUDE.md`에 재발 방지 계약으로 반영했다. visual PASS는 새 EXE에서 사용자가 직접
확인하기 전까지 대기다.

비평에서 확인한 네 P1도 같은 변경에서 닫았다.

- Draft create/delete는 표시 캐시가 아니라 잠근 source catalog와 fresh Valtan Product graph로 Product 소유권을
  다시 검사한다.
- target generation 교체 복구는 animation pose뿐 아니라 기존 Effect fixed-step clock도 같은 full seek 경로로
  0초에 재결합한다.
- Product unlink는 non-killing async process로 실행하고 180초 이후에도 rollback 가능한 child를 종료하지 않는다.
- source atomic replace backup은 post-replace byte 검증 전에는 삭제하지 않으며, 주입된 검증 실패에서도 baseline과
  Product를 복구한다.

확인 modal의 pending/current stable identity가 달라진 경우에도 아무것도 삭제하지 않는 fail-closed 검사를
추가했다. 이 보강은 사용자가 현재 실행 중인 이전 EXE에서 저장하고 있는 Effect 파일이나 Product data에 실제
unlink/delete를 수행하지 않았다.

## 8. 2026-08-27 Valtan Arena 입장 차단 해제

사용자 화면의 실패는 Server 병목이 아니라 Client activation 순서 문제였다. current level이 `LOADING`인
prospective Valtan `Initialize`에서 `effect.valtan.environment.red-vortex-sky`의 level-placement admission을
요청해 target `VALTAN_ARENA`와 current level이 달랐고, clone 전 거부가 전체 Level 초기화 실패로 승격됐다.

Arena 입장 검증을 우선하라는 사용자 결정에 따라 Valtan Map Effect source에서 붉은 Vortex world 배치만
제거하고 publisher로 runtime 문서를 갱신했다. 바닥 균열 surface overlay와 발탄 전투/Server 로직은 유지했다.
authored Effect와 resource는 비활성 자산으로 남아 Product 입장에서는 소비되지 않는다.

```text
source/runtime presentation 수       1 / 1
source/runtime EFFECT_DOCUMENT 수    0 / 0
source/runtime 문서 동일성           PASS
Publish-MapAuthoring.ps1             PASS
Map Effect lifecycle contract        8/8 PASS
Map Effect publisher contract        6/6 PASS
focused git diff --check             PASS
```

데이터-only 변경이라 Client 재빌드는 필요하지 않다. Server와 Client는 확인 시점에 모두 종료 상태였고 7777
listener도 없었다. 사용자가 `Server + Client`를 새로 실행해 Lobby의 `Valtan`으로 진입하는 수동 smoke와 시각
판정은 아직 수행 전이다.
