# F1 Complete Play 아레나 보존과 Anim Workbench·Sequencer 저작 구현 결과

## 결론

`codex/anim-bench-sequencer-authoring` 브랜치에서 발탄 중심의 F1 Complete Play와 Anim Workbench 저작
수직 슬라이스를 구현했다. 작은 세로 탭이던 Animation Tool은 최소 `1180x760`의 Master/Preview/Persistent
Detail, full-width Sequencer, Data Files 구조로 바뀌었다. Workbench는 별도 범용 sequence JSON을 만들지
않고 실제 gameplay, animation binding, Pattern Sound owner를 stable ID로 join한다.

사용자가 먼저 요구한 다음 세 사례는 Source JSON, Product projection, Server 소비, Workbench Detail과 Save까지
연결됐다.

- `VALTAN_HIGH_JUMP/AIRBORNE`: 6500ms Server stage/blank wall-clock과 alive player당 axe 수 편집
- `VALTAN_TRASH`, `VALTAN_TRASH_CATCH_IF` `STEP_07`: Pattern Sound와 Counter enable/disable → exact Groggy 전환
- `VALTAN_CATCH_BREATH/STEP_04`: 실제 `ARENA_EJECTION`, 24m/s, 500ms, yaw 0도를 표시하고 180도 draft 저장

Debug/Release Product와 Core, focused native harness, 변경 domain validator가 모두 통과했다. Client/UI는
에이전트가 실행·조작하지 않았으므로 최종 layout, 음향, 0도/180도 release 방향과 패턴 체감은 사용자의
수동 판정으로 남아 있다.

## 브랜치와 기준선

- 물리 정본: `C:\Users\user\Desktop\LostArk`
- 브랜치: `codex/anim-bench-sequencer-authoring`
- 시작 기준: `d4b88f88a63fdb72e3bf0919635385636600761a`
- 세션 시작 LAN 판정: `server-host`
- 팀 endpoint: `192.168.0.14:7777`
- Server bind/firewall 설정: 준비 완료
- endpoint probe `not-listening`: 작업 차단 아님; 사용자 smoke 전에 Server 실행 필요

## F1 Complete Play와 아레나 보존

- F1 Boss Tool은 등록된 발탄 Product pattern을 고정 높이 스크롤 목록으로 표시한다.
- `Valtan Complete Play (Server Boss Replay)`는 선택한 semantic pattern ID만 typed Server audition으로 제출한다.
- pattern replay는 boss, attachment, boss-owned combat object와 audition lifecycle만 재설정한다.
- 기존 wall, floor, debris, collision, navigation state와 revision은 보존한다.
- arena 전체 변경은 다음 explicit preset으로만 수행한다.
  - `Fresh / Restore Entire Arena`
  - `Circle / Remove All Walls`
  - `Break 3 O'Clock Floor`
  - `Break 9 O'Clock Floor`
  - `Final / Break 3 + 9 O'Clock Floors`
- 요청 sequence와 pattern ID가 일치하는 Server 결과를 상태로 표시한다.

## Anim Workbench 화면

Workbench 기본 창은 viewport-aware size와 최소 `1180x760`을 사용하고 `Reset Workbench Layout`을 제공한다.

```text
Master / Outliner | Preview + Transport | Persistent Detail
-----------------------------------------------------------
full-width Sequencer: Animation / Effect / Sound / Camera /
                       Shake / World / Combat Object
-----------------------------------------------------------
Data Files: 실제 owner 경로, load 상태, stable selection
```

- Persistent Detail은 선택한 `patternId/stageId/actionId/occurrenceId`의 typed owner를 세로로 표시한다.
- Sequencer는 별도 runtime이나 generic JSON 정본이 아니라 실제 owner row를 시간축으로 join한다.
- Animation은 clip/sourceStart/playMs/playRate/loop/NONE과 Add/Remove Sequence Slot을 편집한다.
- Pattern Sound는 exact clip occurrence와 검증된 Sound event로 Add하고, exact
  `bindingId + occurrenceId`로 Remove한다.
- Effect V1/V2, Camera/Shake, World처럼 이번 변경에 검증된 inline Save adapter가 없는 owner는 실제 path와
  stable occurrence를 표시하고 owner Tool deep-link를 제공한다.
- raw JSON text editor와 consumer 없는 generic sequence document는 추가하지 않았다.

## 실제 저작 owner와 저장 경계

| Detail 범주 | 실제 owner | 이번 상태 |
|---|---|---|
| Gameplay stage/release/Counter/Groggy | `Data/Valtan/Valtan.gameplay.json` | typed draft, publish/apply, Server revision gate |
| Animation sequence slot | `Data/Animation/Authored/Valtan/Valtan.patternbindings.json` | Add/Remove/Edit, model timing, atomic CAS Save |
| Pattern Sound | `Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json` | deterministic Add/Exact Remove/Edit, atomic CAS Save |
| Combat-object Sound | typed combat-object Sound owner | gameplay Product transaction과 원자 교체 |
| Effect/Camera/Shake/World | 각 Product owner | joined row와 owner deep-link; 무소유 inline Save 없음 |

Animation Save는 현재 CModel clip duration과 모든 Encounter-required action을 검사한다. 변경 animation occurrence와
결합된 Effect/Sound/Shake가 같은 action, clip occurrence, stage wall, loop policy 안에 남는지도 검사한다.
Encounter/Effect/Sound/Shake exact bytes를 commit 직전까지 잠그고 재검사하므로 동시 owner 변경은 stale Save로
거부된다.

Pattern Sound Save도 역방향 dependency를 닫았다. Encounter와 patternbindings exact bytes를 snapshot한 뒤
commit 직전에 shared-read lock과 CAS를 수행한다. Animation clip 길이가 동시에 짧아지거나 Encounter stage가
바뀌어 기존 `startMs`가 무효가 되는 경쟁에서는 Sound 파일과 draft baseline을 보존한 채 Save를 거부한다.

## Counter와 Groggy

Workbench의 `Counter: true`는 느슨한 boolean이 아니다. 다음 구조를 한 저작 단위로 검증한다.

1. WINDUP ENTER의 `boss.flag.counterable = 1`
2. 같은 stage EXIT의 `boss.flag.counterable = 0`
3. `COUNTER_HIT` branch가 같은 pattern의 stable Groggy action을 가리킴
4. 대상 stage가 `stageKind == GROGGY`
5. Groggy ENTER/EXIT가 `boss.flag.groggy`를 paired set/clear
6. Server counter 판정이 `COUNTER_HIT` outcome으로 해당 action edge를 확정

현재 실제 admission은 다음과 같다.

- `VALTAN_TRASH/STEP_07`
  - counter proxy: boss-local forward 1m, right 0m, radius 2.25m
  - success: `valtan.sequence.center-trash-rush-if.groggy`
  - Groggy: 4433ms
- `VALTAN_TRASH_CATCH_IF/STEP_07`
  - counter proxy: boss-local forward 1m, right 0m, radius 2.25m
  - success: `valtan.sequence.rush-if.groggy`
  - Groggy: 4433ms

Disable은 counter authoring이 소유한 flag/branch 묶음을 제거하되 dormant counter proxy를 보존한다. 따라서 다시
Enable하면 같은 stable geometry와 Groggy target을 복원한다.

## 첫 Pattern Sound admission

두 Trash `STEP_07`에는 실제 다음 row가 결합됐다.

- `G_Voltan2_Attack12_Cast2`: clip source +1ms
- `G_Voltan2_Attack04_ShotVox2`: clip source +900ms

Add는 선택 action의 exact clip occurrence와 Sound catalog event만 허용한다. stable ID는
`cue.sound.authoring.<action-qualified-clipOccurrenceId>.<lowest-free-ordinal>` 규칙으로 결정되며 occurrence ID도
같은 identity에 맞춰 생성한다. Remove는 exact 두 ID가 모두 일치해야 한다. 없는 clip/event, duplicate ID,
model source end 또는 Server stage wall을 벗어난 startMs, stale dependency, replace failure는 기존 source와
draft를 보존한다.

## High Jump blank와 Catch Breath release

- `VALTAN_HIGH_JUMP/AIRBORNE` Source와 Product의 현재 baseline은 6500ms다.
- Detail의 `Server wall / blank timeline ms`는 boss stage clock을 편집한다.
- loop animation을 더 오래 유지할 수 있지만 spawned axe lifetime과 axe-local hit `atMs`를 바꾸지는 않는다.
- alive player당 axe 수는 같은 typed Product draft에서 별도로 조정한다.

`VALTAN_CATCH_BREATH/STEP_04`의 baseline은 다음과 같다.

- releaseMode: `ARENA_EJECTION`
- speed: `24 m/s`
- duration: `500 ms`
- yawOffsetDegrees: `0`

Server의 0도는 기존 backward release 방향이다. `Set 180 deg Draft`는 그 결과를 boss facing 정면 방향으로
뒤집어 저장 비교할 수 있게 한다. 최종 방향은 Client root motion이 아니라 Server release action과 snapshot이
확정한다. Source baseline은 비교를 위해 0도로 유지했다.

## 실제 F1 보스 최신성

저장 후 reload 대상은 Development preview만이 아니라 `CClientReplication`이 소유한 primary
`BOSS_VALTAN`, `INVALID_NET_ENTITY_ID` Server-replicated presentation이다.

- Animation/Pattern Sound Save: animation/effect/sound/shake joined cache 전체를 stage하고 성공 시에만 교체
- Combat-object Sound Save: primary Valtan의 typed cache를 reload
- active consumer reload 실패: 이전 cache를 복원하고 freshness gate reject
- ordinary boss despawn 또는 consumer 부재: reject latch를 지우지 않음
- successful authoritative reload/spawn 또는 `Reset_World`: freshness admit
- stale presentation 또는 최신 gameplay candidate가 Server-active revision이 아님: Complete Play 차단
- 직접 Play/Repeat, isolated Flow slot, Flow Start/Retry, Next Queue/Retry가 모두 같은 gate를 통과
- Next Clear와 Flow Stop은 새 pattern을 시작하지 않으므로 stale 상태에서도 복구 명령으로 허용

따라서 Source Save 성공을 곧바로 실제 Server replay 적용 완료로 오인하지 않는다.

## 자동 검증

### Focused 계약

- Workbench/Counter/F1/Pattern tree/Pattern Sound/Animation Tool/Boss flow Python: 115/115 PASS
- saved gameplay revision과 counter proxy 직접 회귀: 2/2 PASS
- 합계: 117/117 PASS

### Native Valtan focused harness

Debug와 Release 모두 통과했다.

- audition service: 23/23
- pattern flow: 12/12
- tuning command: 14/14
- presentation, Encounter, Pattern Sound, Animation binding contract: PASS
- Pattern Sound add 566 → 567 → exact remove 566 round-trip: PASS
- Animation/Encounter mid-Save dependency mutation rollback: PASS
- presentation freshness reject → despawn 보존 → reset/success admit: PASS

### Domain validator

- `Project-ValtanPatternMaster.ps1 -Mode ValidateV2`: PASS
  - managed 33, legacy 26, projected artifact 9
- `Publish-ValtanTuningRuntimeSet.ps1 -Mode Validate`: PASS
- `Publish-GameplayBalance.ps1 -Mode Validate -SkipValtanSplitProjection`: PASS
  - boss pattern 57, stages 255, audition timeline 52
- `Publish-WorldGameplay.ps1 -Mode Validate`: PASS
  - Area 5개, Valtan spawn group 3개
- `Publish-ValtanWorldDestruction.ps1 -Mode Validate`: PASS
  - groups 105, bindings 224

### Product/Core

| 구성 | Product | Core |
|---|---|---|
| Debug x64 | PASS | PASS |
| Release x64 | PASS | PASS |

Build evidence:

- `out/BuildPipeline/runs/20260830T135713088Z-debug-product-2b427982.json`
- `out/BuildPipeline/runs/20260830T135743659Z-release-product-e98a0acd.json`
- `out/BuildPipeline/runs/20260830T135848320Z-debug-core-af56d07a.json`
- `out/BuildPipeline/runs/20260830T135944487Z-release-core-afc47e3e.json`

Core에서 protocol, Character Select private/shared world isolation, Product CSO closure, WARP readback,
navigation/destruction와 gameplay balance 계약도 함께 통과했다.

## 알려진 별도 baseline test 부채

최종 Product/Core에는 영향이 없고 모두 PASS했지만, 광역 개별 테스트 전체 실행에는 이번 변경과 무관한 두 stale
fixture가 남아 있다.

- `test_split_products_drive_client_and_server_builds`: 퇴역한 `ValidateValtanSplitProducts` 이름을 기대하지만
  현재 Client project 정본은 `ValidateClientBuildDomains`다.
- `test_world_publisher_rejects_static_ghost_and_preserves_runtime`: 임시 repo fixture가 현재 필수 Kakul
  `StageMarkers.json`을 복사하지 않는다.

이번 변경으로 늘어난 focused harness Client source count는 build surface gate를 14개 exact source로 갱신해
Product/Core 진입에서 검증한다.

## 사용자 수동 검증 순서

1. `Framework.sln` Debug의 `Server + Client` profile을 실행한다.
2. Lobby → Valtan으로 진입한다.
3. F1 → `Action Presentation Workbench` → Valtan을 연다.
4. 넓은 3-pane, 오른쪽 Persistent Detail, 아래 full-width Sequencer와 Data Files가 동시에 보이는지 확인한다.
5. `VALTAN_HIGH_JUMP/AIRBORNE`를 선택한다.
   - `Server wall / blank timeline ms`를 변경하고 Save한다.
   - Server-active revision 상태를 확인한 뒤 Complete Play로 공백 체감을 비교한다.
6. `VALTAN_TRASH/STEP_07`과 `VALTAN_TRASH_CATCH_IF/STEP_07`을 각각 선택한다.
   - Counter enable/disable과 Groggy target을 확인한다.
   - Pattern Sound row를 Add/Edit/Remove하고 Save한다.
   - Complete Play에서 counter 성공 전환과 두 Sound cue를 직접 확인한다.
7. `VALTAN_CATCH_BREATH/STEP_04`를 선택한다.
   - baseline yaw 0도를 먼저 Complete Play한다.
   - `Set 180 deg Draft` → Save → Server-active 확인 → Complete Play로 방향을 비교한다.
8. arena preset을 바꾼 뒤 다른 pattern을 Complete Play해 기존 wall/debris/collision/Nav 상태가 보존되는지 확인한다.

사용자의 서면 관찰 전에는 layout, visual, audio, occurrence와 방향을 `manual PASS`로 기록하지 않는다.

## 이번 결과에서 제외한 범위

- Kakul `SL01~SL05` 이동은 사용자 runtime 실패 상태다. 실제 map 경계 재추출, MapTool 육안 네이밍과
  stable marker 승인이 필요한 별도 수직 슬라이스다.
- Effect group Create/Rename/Assign/Ungroup는 사용자 요청대로 다른 작업자 범위다.
- generic Pattern/Stage Add는 entry/default/branch closure와 모든 owner rollback이 닫히기 전까지 추가하지 않았다.
- 모든 schema를 무제한 편집하는 범용 JSON editor는 만들지 않았다. 실제 runtime owner별 typed Row만 편집한다.
- Kakul/기타 model의 Product boss pattern 승격과 시각 fidelity는 이번 발탄 Workbench 결과가 아니다.
