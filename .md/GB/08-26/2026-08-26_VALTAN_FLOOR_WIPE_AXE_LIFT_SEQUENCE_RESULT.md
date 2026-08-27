# 발탄 130 전멸기 도끼 회수 시퀀스 구현 결과

## 1. 완료한 Product animation 계약

`VALTAN_FLOOR_WIPE_130`의 빠져 있던 정방향 도끼 회수 clip을 presentation 정본에 연결했다.
역재생은 사용하지 않았고, Server stage duration과 Effect cue stable ID도 바꾸지 않았다.

```text
WINDUP       1800ms  mesh_att_battle_5_02_loop 반복
FIRST_SMASH   800ms  mesh_att_battle_5_02_end 534ms + 마지막 포즈 266ms 유지
INTERVAL     2000ms  mesh_att_battle_5_04 500ms -> mesh_att_battle_15_02 1000ms
                      + 마지막 머리 위 포즈 500ms 유지
SECOND_SMASH  500ms  mesh_att_battle_15_03 500ms exact
RECOVERY     1500ms  mesh_att_battle_15_04 반복
```

source action 420630 sequence 2의 순서를 사용했다. source stage marker는 `5_04` 400ms,
`15_03` 450ms지만 Product는 hard-cut runtime의 끝 포즈 연결과 고정 stage budget을 위해 각각
500ms까지 재생한다.

## 2. 데이터·생성물 연결

- `Data/Valtan/Valtan.presentation.json`의 FIRST_SMASH, INTERVAL, SECOND_SMASH occurrence와
  `HOLD_LAST_POSE`/`EXACT` end policy를 갱신했다.
- V2 projector로 `Data/Animation/Authored/Valtan/Valtan.patternbindings.json`을 재생성했다.
  retired V1 `Data/Valtan/Valtan.pattern.json`은 수정하지 않았다.
- `Data/Animation/RootMotion/Valtan.rootmotion.json`을 현재 binding 전체에서 재생성했다.
  FLOOR_WIPE에는 `INTERVAL / mesh_att_battle_5_04 / 2000ms / final -0.2214m`와
  `SECOND_SMASH / mesh_att_battle_15_03 / 500ms / final -0.5560m` 두 stage curve가 생성됐다.
- Animation Tool contract test는 여섯 clip 순서와 각 stage의 play budget을 고정한다.
- root-motion explicit multi-clip migration 목록에는 FLOOR_WIPE INTERVAL을 추가했다.
- root-motion admission은 explicit finite multi-clip chain에 한해 최종 변위뿐 아니라 33ms sampled curve의
  최대 수평 이동도 본다. 이 보정으로 중간에 0.2717m 이동하고 최종 -0.0170m로 돌아오는 기존
  ARENA_BREAK WIDE_REVEAL curve를 잃지 않는다. single/natural clip의 기존 final-displacement 판정은 유지한다.
- 팀 인수인계서의 FLOOR_WIPE wall contract와 실제 clip flow를 같은 값으로 갱신했다.

root-motion 생성기는 단일 패턴 부분 갱신이 아니라 현재 binding 전체를 투영한다. full bake 결과 HEAD source에는
이미 존재하지만 기존 생성물에서 빠져 있던 18개 `VALTAN_SEQUENCE_*` pattern의 46개 stage도 함께 투영됐다.
나머지는 FLOOR_WIPE 변경과 ARENA_BREAK WIDE_REVEAL의 full-chain 재계산이며, 총 20개 pattern block이 HEAD와
달라졌다. 다른 pattern source를 이 작업에서 수정하거나 되돌리지는 않았으며, 현재 source 전체에 대한
`--check`가 통과하는 생성 상태를 유지했다. 동시 HIGH_JUMP와 DASH_CHARGE 변경은 각각 zero-root 및 authored
motion override라 이 root-motion diff에는 기여하지 않는다.

## 3. Server 재생 계약

`VALTAN_FLOOR_WIPE_130`은 새 weighted rotation 후보로 추가하지 않았다. 현재 정본에 이미
`HEALTH_BAR_CROSSING 130`, `triggerOrder: 1`, `oncePerEncounter: true`,
`ABORT_ENCOUNTER_REQUIRE_RESET`인 강제 mechanic으로 연결돼 있고, Server는 일반 weighted 선택보다
`PendingPatternIds`의 강제 mechanic을 먼저 소비한다. 같은 ID를 130~109 selection set에도 넣으면 130줄 이후
랜덤으로 재등장할 수 있으므로 현재 강제 경로를 유지하는 것이 올바른 통합이다.

Gameplay publisher를 Publish 모드로 다시 실행해 로컬
`Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap`에 현재 pattern과 animation root motion을 투영했다.
Server가 소비하는 다섯 stage는 다음과 같다.

```text
WINDUP       1800ms  damage 없음
FIRST_SMASH   800ms  SIX_DIRECTIONS 14.0m / half-width 2.2m / 1 pulse
INTERVAL     2000ms  도끼 회수 root motion 62 samples
SECOND_SMASH  500ms  CIRCLE 100.0m / 전멸 damage / 1 pulse / root motion 17 samples
RECOVERY     1500ms  완료 후 IDLE 및 pattern invulnerability 해제
```

Server contract에는 Effect Tool의 `Play Server Pattern`과 동일한 stable-ID 요청으로 실제 `CGameRoom`을
30Hz tick하는 회귀 검증을 추가했다. 이 검증은 `WINDUP -> FIRST_SMASH -> INTERVAL -> SECOND_SMASH ->
RECOVERY -> IDLE` 순서, 두 damage pulse, 2000/500ms root-motion 원문 일치, 실행 중 무적과 완료 후 해제,
`PENDING -> ACTIVE -> COMPLETED` lifecycle 및 pinned gameplay revision을 함께 확인한다. 기존 계약은 별도로
131줄 초과에서 130줄 이하로 내려갈 때 `FORCED_HEALTH_BAR` source로 이 pattern이 queue·선택되는 것을 검증한다.

현재 떠 있는 Debug Server PID 36584는 Publish 전 실행 파일과 bootstrap을 이미 로드한 process이며 표준
`Server/Bin/Debug/Server.exe` 링크도 점유한다. 사용자 process를 중지하지 않고 Debug Server를 별도 TEMP
OutDir에 빌드해 새 FLOOR_WIPE 계약을 실행했다. 실제 플레이 검토 전에는 Server를 한 번 재시작해야 이번
bootstrap과 Server test build 기준을 소비한다.

## 4. 자동 검증

| 검증 | 결과 |
|---|---|
| `Project-ValtanPatternMaster.ps1 -Mode ValidateV2` | PASS, errors 0, managed 27 / legacy 26 / artifacts 9 |
| `build_valtan_rootmotion.py --check` | PASS, 45 patterns / 98 stages / 5,247 samples |
| `test_animation_tool_valtan_pattern_master` | PASS, 8/8 |
| `test_valtan_pattern_tree_contract` | PASS, 17/17 |
| `Publish-ValtanTuningRuntimeSet.ps1 -Mode Validate` | PASS |
| `Publish-GameplayBalance.ps1 -Mode Validate` | PASS, 53 boss patterns / 225 stages |
| `Publish-GameplayBalance.ps1 -Mode Publish` | PASS, Server bootstrap 갱신 |
| Debug Server isolated build | PASS; 실행 중인 표준 Debug Server는 중지하지 않음 |
| 새 `VALTAN_FLOOR_WIPE_130` room-tick contract | PASS, 5 stages / 2 damage pulses / lifecycle 완료 |
| Release Server isolated build + 전체 `--contract-test` | PASS, failures 0 / exit 0 |
| JSON parse | PASS |

전체 `Test-ValtanPatternMaster.ps1`은 40개 중 39개가 통과했다. 남은 1개
`test_v1_migration_is_staged_and_excluded_from_current_split_authority`는 retired V1과 현재 split source의
동시 HIGH_JUMP/ENCOUNTER 변경 차이에서 실패했으며, animation binding 비교는 이 실패 지점이 아니다.

`test_build_valtan_rootmotion`은 4/4 통과했다. explicit returning/static/single 경계와 이 작업의
`floor-wipe-130.interval`, 기존 current binding의 `arena-break-109.wide-reveal`을 explicit multi-clip
목록에 함께 고정했다. ARENA_BREAK runtime 계약 자체는 수정하지 않았다.

Effect source-occurrence inventory `--check`는 기존 reviewed FLOOR_WIPE WINDUP의 source/product clip-name
불일치로 실패한다. 사용자가 별도로 저작 중인 부채꼴 Effect와 occurrence 검토 범위이므로 이 animation 작업에서는
Effect 문서와 selection manifest를 수정하지 않았다.

전체 Debug `Server.exe --contract-test`에서는 위 새 FLOOR_WIPE 계약이 통과했고, 실행 중인 다른 Debug Server가
같은 `Local\\LostArk.Server.ValtanRuntimeActivation.Debug` mutex를 보유해 runtime gameplay owner 계약 1건만
실패했다. 이는 FLOOR_WIPE 데이터나 재생 실패가 아니라 계약 테스트 process가 첫 owner mutex를 획득할 수 없어서
발생한 환경 충돌이다. 해당 durable-state fixture는 contract process ID별 TEMP 경로를 시작 시 비우므로 stale
runtime JSON과도 무관하다.

현재 source의 Release Server도 별도 TEMP Out/Int에 빌드하고 `LOSTARK_SERVER_DATA_ROOT`를 표준
`Server/Bin/DataFiles`로 지정해 전체 `--contract-test`를 실행했으며 `failures : 0`, exit 0으로 통과했다.
새 FLOOR_WIPE room-tick block은 Debug 전용 contract 구간에 있어 Release suite에는 포함되지 않는다. 따라서
현재 검증 근거는 `Debug에서 새 5-stage 계약 PASS`와 `Release에서 전체 제품 계약 PASS`를 분리해 기록한다.

## 5. 수동 화면 검증 경계

현재 animation runtime에는 clip-edge blend가 없다. WModel pose 표본상
`5_02_end@534ms -> 5_04@0ms`의 major-skeleton 차이는 약 0.37m로, 빠진 회수 동작을 연결해 기존 직결보다
줄었지만 완전한 무점프 전환이라고 자동 판정할 수는 없다. `15_02@end -> 15_03@start`는 사실상 같은 포즈다.

Client와 UI는 실행하지 않았다. 사용자가 실제 Pattern 재생에서 다음 순서를 육안 확인해야 visual PASS가 닫힌다.

1. 첫 강타 후 266ms 동안 지면 포즈를 유지한다.
2. `5_04`로 도끼를 정방향으로 들어 올린다.
3. `15_02` 끝의 머리 위 포즈를 유지한다.
4. `15_03`으로 두 번째 강타하고 `15_04` recovery로 이어진다.

반복 재생 경로는 `F1 > Balance Tool > Gimmicks > 6방향 후 전멸 패턴 > Play Server Pattern`이다.
현재 합본 Effect가 SECOND_SMASH 시점의 원형 전멸 표현까지 소유하기 전에는 기존 SECOND_SMASH 보조 cue를
제거하지 않는다. 사용자가 부채꼴과 4.6초 wipe impact를 합본 Effect에서 확인한 뒤 단일 cue 정리를 별도로
결정한다.
