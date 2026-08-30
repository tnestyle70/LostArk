# F1 Complete Play 아레나 보존과 Anim Bench·Sequencer 저작 구현 계획서

## 목표

F1 Developer Tools에서 등록된 발탄 패턴을 스크롤 목록으로 선택하고 `Complete Play`를 누르면,
현재 Server가 확정한 벽·Debris·collision·Nav 상태를 그대로 보존한 채 보스와 해당 패턴만 다시 시작한다.
동시에 Action Presentation Workbench의 기존 joined view를 실제 정본 데이터에 연결해, 이미 지원하는
패턴 gameplay 수치 저장과 presentation sequence 저작의 완료·미완료 경계를 명확히 한다. 현재의
작은 세로형 ImGui 창을 Master/Viewport·Transport/Sequencer/Detail/Resource Files가 동시에 보이는
저작 작업공간으로 교체하고, Animation·Effect·Sound·Camera·World owner를 침범하지 않는 실제
Sequencer를 단계적으로 완성한다.

## 2026-08-30 재개 기준선과 반영 판정

- 작업 브랜치: `codex/anim-bench-sequencer-authoring`
- 기준 HEAD: `d4b88f88a63fdb72e3bf0919635385636600761a`
- 기준 상태: `origin/main`과 동일 tree, clean. 과거 worktree를 폴더 단위로 병합하지 않는다.
- F1 Developer Tools, 다중 Tool 표시, Resource Files, Complete Play, Valtan Arena Active와 joined
  Animation/Effect/Sound/Camera/World inspector는 구현 완료다.
- `WORLD_ID::KAKULSAYDON_ARENA`와 `LEVEL::KAKULSAYDON_ARENA`, Character Select의 typed Server
  admission, Kakul SL01~SL05 typed marker command와 replicated character 이동 코드 경로는 존재하고
  정적 계약을 통과한다. 그러나 2026-08-30 사용자 실측에서 SL01~SL05 이동이 동작하지 않았으므로
  runtime 완료는 `NOT_DONE`이다. 정적 계약을 실제 이동 PASS로 승격하지 않는다.
- F1에서 임의 map/level을 직접 선택해 `Change_Level`하거나 local character transform을 바꾸는 기능은 없다.
  향후 level 선택기는 Server-admitted destination만 typed route로 제출하고, MapTool의 Development Area
  선택과 분리한다.
- Kakul 물리 폴더에는 WModel 11개가 있고 Workbench selector는 body profile 4개
  (`MN_RPCT_00`, `MN_RPCT_05/07`, `MN_RPCT_06`, `MN_RPCZ_00`)를 연다. 나머지 weapon WModel은
  body timeline target이 아니라 attachment dependency다. 전체 preview selector 18개 중 Valtan Ghost
  `MN_RPBF_02` body/AnimSet은 물리 payload가 없어 unavailable로 취급한다.
- Kakul slot Detail의 clip/start/play/rate/loop 편집 코드는 있으나 네 authored sparse document의
  `bindings`는 모두 비어 있다. 저장·reload로 반영됐다는 현재 증거는 0건이다.
- Effect Tool은 stable `groupId`, manual group tree, Play Group, element/group solo·mute, marked multi
  duplicate/delete를 지원한다. Create/Rename/Assign Marked/Ungroup/Reorder와 저장되는 collapse 상태는
  아직 없다.
- Valtan pattern sound와 combat-object sound join/playback은 구현됐고 focused contract를 통과한다.
  일반 Animation Event의 Sound는 raw payload 편집뿐이며 catalog picker, waveform, typed drag/drop은 없다.
- Anim Bench는 `460x720` 단일 세로 스크롤 창이다. Kakul은 같은 폭 안에서 좌측 목록을 270px로
  고정하므로 오른쪽 Detail이 눌린다. 기존 `imgui.ini`에는 `FirstUseEver` 기본 크기 변경도 적용되지 않는다.
- 실제 Product owner와 별개인 범용 Sequencer document/class/codec/runtime/harness는 만들지 않는다.
  현재의 Valtan seek slider와 별도 `Animation Sequence Intake` 선형 목록만으로는 Sequencer 완료
  증거가 아니며, 실제 owner 문서의 stable ID join과 typed Save adapter가 완료 기준이다.

이번 재개에서 완료 판정은 다음처럼 분리한다.

| 범위 | 현재 판정 | 이번 목표 |
|---|---|---|
| F1 Valtan Complete Play/Arena 보존 | 구현·자동 검증 완료 | 회귀 보존, 수동 시각·청음 대기 |
| Kakul 입장/SL01~SL05/캐릭터 이동 | 정적 계약 PASS, 사용자 runtime 실패 | marker 재추출·경계 승인 뒤 별도 수직 슬라이스 |
| F1 일반 map/level 선택 | 미구현 | Server-admitted 목적지 선택기로 별도 수직 슬라이스 |
| Kakul/기타 model preview | 부분 구현 | availability와 attachment 관계 표시, missing payload fail-closed |
| Effect grouping 편집 | 다른 작업자 진행 범위 | 이 브랜치에서는 현 계약 보존·Workbench 연결만 수행 |
| Anim Bench 넓은 작업공간/고정 Detail | 미구현 | 첫 UI 수직 슬라이스 |
| 멀티트랙 Sequencer | joined lane 부분 구현 | 실제 owner adapter와 전용 full-width track pane을 우선 구현 |

이번 세션의 구현 중심은 Valtan Product pattern을 기준으로 한 Anim Workbench다. Kakul의 고정 5 marker를
추측으로 보정하지 않는다. 실제 레벨의 더 세분화된 경계는 placement/package 좌표를 다시 추출한 뒤
사용자 또는 MapTool 작업자가 화면에서 확인해 stable ID와 표시명을 승인하는 별도 작업으로 분리한다.

## 2026-08-30 구현 종료 판정

이번 브랜치에서 계획한 Valtan 수직 슬라이스는 구현과 자동 검증을 완료했다.

- F1 pattern scroll, boss-only Complete Play와 explicit arena preset: 완료
- 최소 `1180x760` 3-pane, Persistent Detail, full-width Sequencer, Data Files: 완료
- gameplay blank/release/Counter/Groggy typed Save: 완료
- animation sequence slot Add/Remove/Edit와 model/dependency CAS Save: 완료
- Pattern Sound deterministic Add/exact Remove/Edit와 역방향 Encounter/Animation commit CAS: 완료
- 실제 Server-replicated primary Valtan joined/combat Sound reload와 Complete Play freshness gate: 완료
- Debug/Release Product/Core, focused native, domain validation: 완료
- Client 화면·사운드·release 방향과 occurrence 최종 판정: 사용자 수동 검증 대기

Kakul `SL01~SL05` runtime 이동, map 경계 재추출·육안 네이밍, Effect grouping 편집, generic Pattern/Stage Add와
Kakul Product boss pattern은 구현 완료로 승격하지 않는다. 세부 증거와 수동 검증 순서는 대응 RESULT를 따른다.

이번 브랜치의 커밋 범위는 F1 Valtan replay/arena 보존, Valtan Anim Workbench 레이아웃, 실제
Gameplay·Animation·Pattern Sound typed owner와 joined Sequencer다. G04 Kakul Product 승격, G07 Effect
grouping, G13 broad harness 정리와 G14 compile critical-path 분리는 계획 문맥만 보존하며 이번 완료로
기록하지 않는다.

## 시작 상태 실측

- F1은 `CBossTool::Get_ServerPatternOptions`로 Product pattern inventory를 이미 읽고,
  `CBossTool::Play_ServerPattern`으로 Server typed audition을 제출한다.
- 현재 F1 선택 UI는 dropdown이며 상시 보이는 스크롤 목록은 아니다.
- `PLAY_PATTERN_ID`의 Valtan Arena 분기가 `Reset_ValtanAuditionState`를 호출해 World destruction,
  Encounter props, collision, Nav blocker까지 Fresh로 되돌린다.
- Character Select에는 이미 보스·attachment·combat object만 교체하는
  `Reset_ValtanBossOnlyAuditionState`가 존재한다.
- Workbench는 pattern/stage, collider/hit/push/knockdown, combat object, animation occurrence,
  Effect/Sound/Camera/World lane을 이미 결합해 표시한다. gameplay stage 수치는 Balance Tool의
  Product save 경계로 저장하지만, Product animation occurrence 자체는 아직 읽기 전용이다.
- Effect Tool의 `Play All`과 `Solo`는 미저장 element까지 확인하는 local authoring preview다.
  stable pattern ID가 있는 Server 재생은 각 Tool의 별도 `Complete Play`가 담당한다.

## G00. 실패를 먼저 고정하는 Server 회귀

Server gameplay contract에서 `BOTH_SIDES_BROKEN` 아레나를 만든 뒤 `PLAY_PATTERN_ID`를 제출한다.
요청 전후의 destruction group exact state, navigation revision, collision revision을 비교하고,
보스 pending pattern만 선택 ID로 바뀌는지 검사한다. 현재 구현에서 실패하고 수정 뒤 통과하는
동적 회귀를 완료 조건으로 사용한다.

## G01. Complete Play reset 경계 교정

`PLAY_PATTERN_ID`는 Valtan Arena와 Character Select 모두 기존 boss-only reset을 사용한다.
보스 상태, boss attachment, boss-owned combat object, audition lifecycle은 초기화하되 다음 상태는
절대 변경하지 않는다.

- ordinary/outer wall과 floor destruction group
- Debris/Encounter prop active state
- Server collision active state와 revision
- Server Nav blocker active state와 revision

이전 패턴의 Debug pillar 예약 상태는 boss-only reset에서 함께 해제해 새 패턴으로 새지 않게 한다.
Fresh/Phase 2/3시/9시/양쪽 preset은 명시적으로 눌렀을 때만 기존 full arena transaction을 사용한다.

## G02. F1 패턴 스크롤 선택

기존 shared selected pattern과 Server inventory를 유지하고 dropdown을 고정 높이 스크롤 목록으로 바꾼다.
`ImGuiListClipper`로 전체 등록 패턴을 비용 없이 표시하고 선택 행을 유지한다. `Complete Play`는 선택된
semantic pattern ID만 서버에 제출하며 arena preset을 암시적으로 호출하지 않는다.

## G03. Workbench 실제 저작 경계

Workbench는 임의 JSON 편집기가 아니라 Product 정본의 typed inspector다.

- gameplay/balance: 기존 stage duration, collider, hit schedule, push/knockdown 등 지원 필드를 표시·저장한다.
- animation sequence: 현재 Product occurrence와 clip binding을 pattern/stage/action/occurrence stable ID로
  선택·편집하고, parse -> validate -> stage -> commit/publish 경계가 존재하는 필드만 저장한다.
- presentation: Effect V1/V2, Sound, Camera/Shake, World/Combat Object는 joined lane과 정확한 owner Tool
  deep-link를 유지한다. 별도 정본 저장 API가 없는 필드는 읽기 전용이라고 UI에 표시하고 가짜 저장을 만들지 않는다.
- 한 번의 사용자 `Save`는 지원하는 domain validator와 publisher/apply를 내부에서 실행하며 부분 저장 실패 시
  기존 Product 상태를 보존한다.

Product animation occurrence 저장 계약이 현재 코드에 없다면 이번 변경에서는 먼저 해당 계약의 최소 수직
슬라이스와 회귀를 추가하고, 원본 extraction/reference 문서를 직접 덮어쓰지 않는다.

세부 typed slice는 다음을 포함한다.

- `RELEASE_GRABBED_PLAYERS`: speed/duration과 별개의 `yawOffsetDegrees`를 Source, Product,
  Server bootstrap/runtime, Workbench draft 전체에 연결한다.
- Sector Effect: Server가 선택한 `arena.center.facing`은 유지하고 cue의 local Y rotation만 stable
  occurrence ID로 수정한다.
- High Jump: AIRBORNE stage duration과 alive player당 도끼 수를 typed gameplay draft로 저장한다.
- 버러지 잡기: `CATCH_PRE_IMPACT`의 실제 추출 Shot3 cue가 세 Product pattern에 모두 결합되는지 검증한다.
- Esther: Server snapshot의 action edge를 identity로 사용해 player `ESTHER_CAST`와 Esther NPC strike의
  최소 근거 cue만 재생한다. exact notify 근거가 없는 나머지 WAV를 임의 타이밍에 연결하지 않는다.

## G04. KakulSaydon 추출 액션 시퀀스 저작

쿠크세이튼은 물리 WModel, 추출 action 근거, Server Product world와 typed 입장·이동까지 존재하지만
boss/NPC/pattern Product 수직 슬라이스는 아직 없다. 따라서 Valtan Complete Play를 가짜로 복제하지 않고
다음 두 문서를 분리한다.

- `Data/Animation/Reference/KakulSaydon/*.actionreference.json`: generator가 만든 immutable
  `REFERENCE_ONLY` action/stage/slot 기본 sequence, 한글 표시명, physical-model join과 holdout 증거.
- `Data/Animation/Authored/KakulSaydon/*.actionbindings.json`: 사용자가 바꾼 exact slot만 기록하는
  `PROJECT_AUTHORED` sparse override. 초기 문서는 비어 있다.

Resource Files에서 프로필 문서나 물리 Character를 선택하면 같은 Workbench가 정확한
`MN_RPCT_00/05/06/07/RPCZ_00` profile을 받고, 07은 실제 `MN_RPCT_05` body를 사용한다. Workbench는
한글 action -> stage -> slot을 나열하고, reference default를 즉시 채워 보여 주며, 현재 WModel clip을
선택해 timing/loop와 함께 원자 저장한다. Load/Save는 stale revision, 잘못된 identity, 중복, 없는 clip,
잘못된 수치에서 기존 메모리와 파일을 보존한다. 이 화면과 버튼은 `Local Extracted Action Preview`이며
Kakul Server Product pattern으로 표시하지 않는다.

현재 네 authored sparse document는 모두 빈 `bindings`다. 첫 완료 증거는 대표 slot 하나를 UI Detail에서
수정하고 저장한 뒤 process reload에서 exact value를 다시 읽으며, 실패 fixture에서는 기존 파일과 메모리가
보존되는 실행형 회귀다. 이 증거 없이 Detail 구현 완료라고 기록하지 않는다.

## G05. 검증과 종료 경계

- Python Workbench/F1 정적 계약
- Kakul generator 재현성, reference/authored strict schema와 sparse override 계약
- Esther/Valtan sound asset closure와 replicated occurrence dedup 계약
- Server gameplay contract 동적 회귀
- 변경 domain publisher validation
- Debug Product/Core 빌드와 회귀
- Release Product/Core 빌드와 회귀
- `git diff --check`, JSON/XML parse, 변경 파일만 stage

Client/UI는 에이전트가 실행하거나 조작하지 않는다. 자동 검증 뒤 사용자가 `Server + Client`로 진입해
F1에서 벽 preset을 선택하고 패턴 Complete Play 전후의 벽·Debris·Nav 시각 상태와 패턴 재생을 직접 판정한다.

## G06. KakulSaydon Arena 수직 슬라이스

복구된 물리 payload는 `LV_LUT_MIDNIGHTC_ED` Area ID를 정본으로 사용한다. `KakulSaydon`은
Resource Files와 F1에서 쓰는 사람이 읽는 collection 이름일 뿐 Map asset ID나 Server world ID를
대체하지 않는다. 아래 Product Level 코드 경계는 현재 main에 구현돼 focused admission/client-level
정적 계약을 통과했지만, 사용자 runtime 실측에서 SL01~SL05 이동은 실패했다. 따라서 Client 전용 레벨이나
임의 텔레포트로 우회하지 않고, runtime 원인과 marker 근거를 별도 수직 슬라이스에서 다시 닫는다.

- `Data/Maps/MapCatalog.json`의 product map, authoring placement, navigation, gameplay 경로
- 추출 placement의 실제 좌표 군집과 package/actor 이름에 근거한 stage marker 문서
- stable player spawn과 stage marker가 포함된 `Gameplay.world.json`
- MapTool bake 형식의 navsource/navpaint와 Client/Server 동일 runtime navgrid
- `WORLD_ID::KAKULSAYDON_ARENA`, Server shared simulation, world bootstrap
- `LEVEL::KAKULSAYDON_ARENA`, registry, loader, transition, 실제 level class
- Character Select에서 typed Server admission을 요청하는 KakulSaydon 진입 버튼
- F1 `Valtan / KakulSaydon` 탭과 현재 world에서만 활성화되는 stage 이동 명령

Mario 1~4나 관문 이름은 추출 원본에 그 identity와 위치 근거가 있을 때만 stable marker로 만든다.
단순 좌표 군집을 Mario라고 추측하지 않는다. exact 근거가 없는 구간은 package/cluster 이름으로
표시하고 UI에서 `추출 좌표 기준`임을 명시한다. stage 이동은 Client transform 변경이 아니라 Server가
현재 player를 해당 Area의 nav-valid stable marker로 옮긴 snapshot 결과만 소비한다.

현재 5개 marker는 실제 레벨 전체 경계를 대표하지 않는다. placement/package 좌표를 재추출하고,
MapTool에서 사용자가 직접 확인해 이름을 붙이는 authoring 흐름이 정본이다. 그 전에는 SL01~SL05를
제품 관문 이름이나 이동 완료 증거로 사용하지 않는다.

남은 제품 범위는 Kakul runtime 이동 교정, boss/NPC spawn, pattern gameplay,
animation/effect/sound presentation과 실제 사용자 visual/audio 판정이다. Level 입장이나 정적 test 통과를
이 남은 vertical slice 완료로 확대 해석하지 않는다.

## G07. 독립 Effect Asset/Instance와 공통 Authoring Scene

Effect V1/V2의 descriptor는 애니메이션 occurrence와 별개로 생성·저장·preview 가능해야 한다.
통합 화면은 다음 identity를 분리한다.

- Effect Asset: V1/V2 owner tool이 소유하는 reusable definition ID
- Effect Instance: Scene/Sequence가 소유하는 stable instance ID, effect asset ID, transform, pivot
- Binding: pattern/action/stage가 독립 effect asset 또는 instance occurrence를 참조하는 관계

World pivot은 Area transform을, Model pivot은 stable preview actor와 bone/anchor ID를 사용한다.
pointer, vector index, Prototype tag는 저장하지 않는다. V1/V2의 기존 local Play All/Solo는 미저장
Asset/Instance preview로 유지하고, `Complete Play`만 saved semantic pattern을 Server에 제출한다.
공통 Authoring Scene은 선택된 실제 Level의 map, actor, camera를 공유하되 tool별 draft ownership은
합치지 않는다.

기존 manual group 조회/solo/mute와 다중 duplicate/delete는 보존한다. Create/Rename/Assign/Ungroup,
Reorder와 collapse 저장은 다른 작업자가 Effect owner Tool에서 진행하는 범위다. 이 브랜치는 그 파일을
중복 수정하지 않고, 추후 확정된 stable Effect asset/instance/group ID와 typed owner Save를 Workbench에서
선택·deep-link하는 소비자만 맡는다.

## G08. Sequencer와 통합 Detail

Sequencer는 또 하나의 Effect/Animation 런타임이나 별도 Product 정본이 아니라 existing owner domain을
시간축에서 조율하는 joined editor다. 선택 identity는 기존 owner의 stable pattern/stage/action/occurrence
ID를 그대로 사용하고 다음 track을 단계적으로 지원한다.

- Animation clip/action section
- independent Effect instance section
- Sound point/window section
- Camera section
- Map/World presentation section

선택된 row는 오른쪽 Detail에서 exact owner data를 편집한다. Animation row는 pattern binding owner,
Sound row는 pattern sound owner, gameplay state/branch는 Valtan gameplay transaction을 호출한다. Effect,
Camera, World처럼 이번 브랜치에 Save adapter가 없는 owner는 exact path와 stable occurrence를 표시하고
owner Tool로 이동한다. Server Product pattern과 실제로 결합된 row만 `Complete Play` coverage에 포함한다.

Anim Bench의 목표 배치는 전체 viewport를 사용하는 다음 `3 + 1` pane workspace다.

```text
┌ Master / Outliner ┬ Preview + Transport ┬ Persistent Detail ┐
│ target/action     │ selected actor/map   │ exact selected ID │
│ track hierarchy   │ play/seek/loop       │ owner typed editor│
├───────────────────┴──────────────────────┴───────────────────┤
│ Sequencer: time ruler + Animation/Effect/Sound/Camera/World │
├─────────────────────────────────────────────────────────────┤
│ Resource Files: typed search/filter/selection/owner open    │
└─────────────────────────────────────────────────────────────┘
```

- 창 기본값은 최소 `1180x760`과 viewport 기반 크기를 사용하고 기존 좁은 `imgui.ini`도 minimum size와
  layout version migration 또는 명시적 Reset Layout으로 교정한다.
- 좌측 Master/Outliner는 pointer나 vector index가 아니라 `(selectionKind, stableId)`를 제출한다.
- 오른쪽 Detail은 항상 보이며 Kakul slot, Valtan stage/hit/combat object, sequence section 중 정확히 하나의
  선택을 편집한다. 기존 typed editor와 owner Save를 재사용하고 generic JSON editor를 만들지 않는다.
- 중앙 Preview/Transport는 local authored preview와 Server Actual 상태를 명확히 분리한다.
- 하단 Resource Files는 현재 `CMainApp`의 private scan을 typed shared index/selection 서비스로 분리한 뒤
  Resources-relative asset ID만 section/owner tool에 전달한다.
- 탭은 pane 내부의 작은 범주 전환에만 쓰고 Anim Bench 전체를 작은 tab body에 다시 넣지 않는다.

Sequencer 구현 순서는 다음과 같다.

1. stable `(patternId, stageId, actionId)` 선택과 Animation/Effect/Sound/Camera/World joined row를 전용
   full-width track pane에 배치한다.
2. Animation occurrence와 Pattern Sound row를 각 실제 owner document의 typed atomic Save에 연결한다.
3. Counter/Groggy gameplay edge는 Server-authority owner transaction으로 연결하고 Animation notify가 직접
   gameplay 결과를 확정하지 못하게 한다.
4. Effect instance, Camera와 Map/World presentation은 owner Tool의 stable deep-link를 먼저 제공하고,
   해당 domain의 검증된 Save API가 있을 때만 inline adapter를 추가한다.
5. Server semantic pattern과 명시적으로 결합된 row만 Complete Play coverage에 참여시킨다.

첫 UI 수직 슬라이스는 넓은 레이아웃 shell, persistent selection/Detail, 실제 owner joined track과 typed
Save 계약이다. 별도 consumer가 없는 generic sequence JSON은 완료 증거로 추가하지 않는다.

### G08-A. Valtan pattern·Counter·Groggy typed authoring

상용 엔진의 Montage/Timeline 편집기처럼 Animation sequence를 named slot/section에 놓되, gameplay 상태
전환은 Animation clip이나 notify가 직접 확정하지 않는다. 이 저장소에서는 다음 네 owner를 분리한다.

| Detail 범주 | 저장 정본 | 역할 |
|---|---|---|
| Pattern/Stage/Counter/Groggy | `Data/Valtan/Valtan.gameplay.json` | Server stage, event, branch, duration과 상태 전환 |
| Animation sequence slot | `Data/Animation/Authored/Valtan/Valtan.patternbindings.json` | stable action ID에 clip occurrence chain 연결 |
| Effect/Sound/Camera/World | 각 typed owner document | stage/action/clip occurrence에 presentation cue 연결 |
| Workbench Sequencer | 별도 저장 정본 없음 | 위 실제 owner들을 stable ID와 시간축으로 join하고 각 typed Save를 호출 |

Persistent Detail은 선택한 `(patternId, stageId, actionId)` 하나에 다음을 세로로 모두 표시한다.

- stage kind, Server duration, animation source/wall duration, 내부 blank/hold milliseconds
- ordered animation slot과 각 `clipOccurrenceId`, clip, source start, play window, play rate, loop
- hit/collider/schedule/damage/reaction과 기존 typed gameplay 수치
- Counter enabled, Counter success target stage/action, timeout/default target
- Groggy stage 여부와 paired groggy ENTER/EXIT flag
- 결합된 Effect/Sound/Shake/Camera/World cue와 정확한 owner document/status

UI의 `Counter: true`는 느슨한 boolean 하나를 저장하지 않는다. 다음 구조가 전부 유효할 때만 true로
표시하고, Add/Enable도 같은 transaction에서 exact 구조를 stage한다.

1. 선택 stage에 `ENTER SET_BOSS_FLAG boss.flag.counterable = 1`
2. 같은 stage에 `EXIT SET_BOSS_FLAG boss.flag.counterable = 0`
3. `COUNTER_HIT -> selected Groggy actionId` branch
4. 대상 action이 같은 pattern의 stable stage이며 `stageKind == GROGGY`
5. 대상 Groggy stage에 paired `boss.flag.groggy` ENTER/EXIT
6. Server fixed-tick counter 판정과 snapshot action edge가 선택 target으로 전환

`Add Counter`는 위 묶음을 deterministic stable event ID로 만들고, Disable은 자신이 소유한 묶음만
제거한다. 다른 authoring event/branch를 지우지 않는다. duplicate, 다른 pattern target, 없는 action,
unpaired flag, 잘못된 stage kind, stale revision과 publish/apply 실패는 기존 draft/file/runtime을 보존한다.

`Add Animation Slot`은 선택 stage의 stable `actionId`에 새 `clipOccurrenceId`를 추가한다. model의 실제
clip selector만 허용하고 `sourceStartMs`, `playMs`, `playRate`, `loop`를 typed Detail에서 편집한다.
stage wall duration과 animation wall 합계의 차이는 `Blank / Hold`로 명시해 도끼 투척·공중 loop처럼
Server stage 시간이 animation source 구간보다 긴 경우를 숨기지 않는다. clip occurrence를 추가해도
Counter/Groggy gameplay branch는 자동 생성하지 않으며, 반대 방향도 마찬가지다.

Pattern/Stage 자체의 Add는 stable ID, entry/default/branch closure, selection/eligibility, animation binding,
publisher와 Server runtime rollback을 한 번에 닫는 후속 수직 슬라이스다. 기존 pattern의 stage/slot 연결을
먼저 완성한 뒤 generic 빈 pattern을 만들어 partial Product 상태를 허용하지 않는다.

## G09. 확장 검증

- Kakul admission gate `server-product-level`
- stage marker identity/중복/Area/nav coverage validator와 Server teleport protocol 회귀
- Effect Asset과 Instance가 animation 없이 생성·저장·재로드·재생되는 계약
- Animation/Pattern Sound owner parse -> validate -> stage -> atomic commit, stale stable ID, duplicate occurrence,
  invalid time과 replace 실패 rollback
- Workbench minimum/pane/layout-version 정적 계약과 선택 ID -> persistent Detail routing 계약
- Kakul 대표 sparse binding save -> process reload exact value와 실패 rollback 실행형 회귀
- Sound section asset ID closure와 겹치는 window의 독립 playback handle 회귀
- Valtan/Kakul F1 탭이 다른 world 명령을 제출하지 않는 실패 경로
- Debug/Release Core, `git diff --check`, JSON/XML parse

Kakul 화면 결과, stage 위치, map/effect/animation fidelity는 최종적으로 사용자가 직접 판정한다.

## G10. 물리 정본과 커밋 증거 경계

현재 작업은 `C:\Users\user\Desktop\LostArk` 한 물리 폴더에서만 통합한다. 다른 worktree의 파일을
폴더 단위로 덮어쓰지 않고, 현재 branch의 누적 커밋과 다른 작업자의 미커밋 파일을 먼저 보존한다.
개발 중 dirty build는 컴파일 진단으로만 사용하고 최종 PASS 증거로 승격하지 않는다.

최종 검증은 다음 identity를 같은 실행 증거에 묶는다.

- exact Git HEAD와 dirty 여부
- Data·source·shader input fingerprint
- domain publisher/validator receipt와 재사용 여부
- Debug/Release Client.exe, Server.exe와 대응 PDB의 SHA-256, 크기, 수정 시각
- 각 publish, compile/link, harness 단계의 경과 시간

`-SkipBuild`는 위 source/input과 binary receipt가 정확히 일치할 때만 허용한다. source, Data, project,
shader가 바뀌었거나 EXE/PDB pair가 다른 경우에는 stale binary 검증을 거부한다.

## G11. Effect V1/V2와 Drive Resource 보존

Effect Tool V2는 삭제 대상이 아니다. 현재 정본은 V2 파일 80개, Authored 75개, Binding 4개,
TextureSlotUsage 1개이며 Esther NPC와 Valtan hand 제품 소비자가 존재한다. V2 validator와 runtime consumer,
Tool을 통합 Workbench의 authoring owner로 보존한다.

V1은 Catalog 171 / Authored 171의 exact closure를 보존한다. 삭제 대상은 현재 Product consumer가 없는
과거 후보 생성·복구·증거용 apply/build/materialize/promote/replay corpus다. caller와 Data consumer를
확인하지 않은 Effect asset이나 물리 resource는 삭제하지 않는다.

`Client/Bin/Resources`는 팀장의 Drive 물리 정본이다. Git index에서는 추적하지 않지만 실제 파일은
그 자리에 보존한다. EXE/Data 전달 ZIP은 실행 바이너리와 generated DataFiles만 포함하고 Resource payload는
포함하지 않는다. 설치기는 허용 경로, SHA-256, staging, rollback을 검증하며 Resource 경로를 덮어쓰려는
ZIP을 거부한다.

## G12. Domain publisher와 build receipt

하나의 build domain graph가 Valtan Product, Effect V2, World Gameplay, Navigation, Destruction,
Gameplay Balance, Item, Reward의 입력·도구·출력을 소유한다. 같은 fingerprint의 domain은 한 번만
publish 또는 validate하고 이후 Product/Core 단계는 receipt를 재사용한다.

Server/Client project의 기존 pre-build hook은 Visual Studio 단독 빌드 안전망으로 보존한다. 중앙 runner가
receipt를 실제로 검증한 경우에만 명시적 MSBuild property로 중복 hook을 건너뛴다. Effect V2와
`ValidateV2`를 제거하거나 빈 성공으로 대체하지 않는다.

목표 실행 흐름은 다음과 같다.

```text
Git HEAD + dirty/source diff
-> changed domain fingerprint
-> domain별 publish/materialize/validate 1회
-> receipt의 output hash 검증
-> Engine/Shared/Server/Client compile/link
-> 영향 harness
-> CSO closure
-> EXE/PDB/source identity evidence
-> 사용자 visual/audio smoke
```

## G13. Broad harness 제거와 assertion 이관

기본 Solution Build는 Engine, Shared, Server, Client 네 제품만 유지한다. 강한 계약인
NetworkProtocol, CharacterSelectIsolation, WModelGeometry는 보존하고 변경 domain에서만 실행한다.
Physics, PointLight, ValtanPatternAudition은 domain-only 진단으로 제한한다.

- MapFrustumContractHarness: water assertion을 MapPipeline으로 이관한 뒤 solution, runner, 문서와 물리
  프로젝트를 제거한다.
- ActionPresentationTimelineHarness: sound/effect/camera/monster/party assertions를 작은 기존 domain
  harness로 이관하고 broad project를 제거한다.
- EffectRenderContractHarness: compiled shader/WARP/resource-root/stage-commit의 고유 assertion을
  Product CSO closure와 V2 focused contract로 이관한 뒤 물리 project와 runner를 제거한다.

assertion 이관은 문자열 존재 검사만으로 대체하지 않는다. 가능한 항목은 실제 parser/evaluator 또는 native
실행 계약으로 유지하고, 남은 harness가 제품 CPP를 직접 재컴파일하면 정확한 수와 후속 library 경계를
RESULT에 남긴다.

## G14. Tools와 제품 compile critical path 정리

이번 변경에서 caller 0이 확인된 HUD `.cfg` 일회성 DataMigration과 빈 ProjectAudit/Profiler 잔재를
정리한다. EffectPipeline과 LevelPlacementExtractor의 대량 복원 corpus는 active validator와 Kakul/map
추출 consumer의 dependency closure를 먼저 분리한 뒤 의미 단위로 제거한다. `MaterialEvaluatorHarness`,
`ModelAssetConverter/Bin`, Valtan tuning transaction 경로를 이름만 보고 삭제하지 않는다.

다음 compile 병목은 별도 검증 가능한 수직 슬라이스로 분리한다.

- Server 제품에서 Full-only `ServerGameplayContractTests.cpp`와
  `WorldDestructionBootstrapContractTests.cpp`를 contract executable/library로 분리
- Release Client critical path에서 Tool/Preview/Diagnostic TU를 editor target 또는 library로 분리
- `Effect_Tool.cpp`, `Effect_DocumentRenderer.cpp`, `Effect_DocumentCodec.cpp`, `MapTool.cpp`,
  `Animation_Tool.cpp`를 실제 책임 단위로 분해
- ActorX FBX 변환 구현을 CharacterAnimationIntake 흐름으로 통일하고 Sound intake owner를 분리
- UpdateLib와 Client SDK/runtime 배포의 중복 copy owner를 하나로 통일

디스크 캐시 정리는 compile 최적화와 분리한다. `Intermediate/ValtanTuningAuthoring`,
`ValtanTuningCandidates`, `ValtanTuningRuntime`은 일반 cache가 아니므로 삭제하지 않는다. 정확한 active
IntDir가 아닌 stale output만 경로를 재확인한 뒤 정리한다.

## G15. 최종 자동 검증과 사용자 인계

모든 구현 변경을 의미 단위 커밋으로 닫은 뒤 같은 clean HEAD에서 Debug/Release Product와 Core를 실행한다.
모든 domain이 바뀐 경우에만 FullDiagnostic을 확장한다. 실행하지 않은 publisher나 harness를 PASS로 쓰지
않고, Client/UI 화면과 소리는 사용자가 다음 경로에서 직접 판정한다.

- 여섯 캐릭터 Character Select와 4캐릭터 우선 스킬 smoke
- Valtan Arena의 F1 wall preset, 선택 pattern Complete Play, slot/Effect/Sound
- 4인 party 입장과 보스 사망 뒤 presentation
- Kakul Arena 진입, SL01~SL05 Server teleport, Workbench action binding

최종 RESULT는 `구현 완료`, `자동 검증`, `사용자 수동 검증 대기`, `남은 구조 병목`을 서로 분리한다.
