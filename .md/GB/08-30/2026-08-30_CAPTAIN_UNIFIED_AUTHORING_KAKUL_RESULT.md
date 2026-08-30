# 캡틴 통합 저작 환경·쿠크세이튼 수직 슬라이스 결과

## 1. 결론

이번 변경은 `C:/Users/user/Desktop/CodexWorkTree/LostArk-captain-unified`의
`codex/captain-unified-authoring-kakul` 브랜치에서 구현·검증했다. 기준 commit은 Action
Presentation Workbench가 들어간 `c75e3623`이다.

완료된 핵심은 다음과 같다.

1. Debug 도구를 하나 열 때 다른 도구가 닫히던 단일 선택 구조를 제거했다.
2. `Resource Files`가 물리 `Resources`, Git 정본 `Data`, publish된 `Client/Bin/DataFiles`를 한 트리에서
   보여 주고 기존 Effect V1/V2, Animation/Workbench, Boss, Map, UI, Camera 도구를 동시에 연다.
3. Valtan의 `Complete Play`는 어느 도구에서 눌러도 동일한 stable pattern ID를 기존
   `CValtanPatternAuditionService -> Server CGameRoom -> replicated presentation` 경로로 제출한다.
4. Workbench에서 Valtan stage duration, high-jump 공중 체공 clock, hit collider, push 거리·시간·방향,
   knockdown/down time을 typed field로 보고 수정한다.
5. Valtan 경기장 상태는 임의 Client checkbox가 아니라 정확한 다섯 Server preset과 replicated
   destruction state로 통합했다.
6. Valtan room은 party 4인 상한과 별개로 실제 network seat 8개, nav-valid spawn 8개를 가진다.
7. KakulSaydon 추출 자료의 map/intake 정본과 fail-closed admission validator를 통합했다.
8. 팀 endpoint를 `192.168.0.14:7777`로 갱신했다.
9. Debug Core와 Release Core는 최종 통과했다.

완료로 가장하지 않는 경계도 명확하다.

- KakulSaydon은 아직 Server Product Level이 아니다. World, nav, encounter/boss, pattern, playable sound
  mapping이 없다.
- 4 human + 4 AI 구성을 위한 network seat는 8개지만 네 명의 Server AI brain/skill/party actor는 아직 없다.
- Workbench의 Gameplay Product와 combat-object Sound source는 각각 whole-file atomic save지만, 두 domain을
  하나의 cross-domain transaction으로 함께 rollback하는 저장은 아직 아니다.
- Debug FullDiagnostic는 코드 회귀가 아니라 원본 물리 Resources에서 Ghost Valtan body/AnimSet과 texture가
  삭제돼 정본 runner 전체 PASS를 선언할 수 없다.
- 원본 `C:/Users/user/Desktop/LostArk`는 2,269건의 대규모 dirty 상태라 이번 commit을 자동 병합하지 않았다.

## 2. 이미지에 보인 다른 작업의 읽기 전용 대조

다른 Codex 작업에는 메시지를 보내지 않았다. 제목과 최근 결과를 읽기 전용으로 대조하고 실제 Git/파일
상태를 다시 측정했다.

| 작업 | 물리 상태 | 이번 통합 판정 |
|---|---|---|
| `LostArk 전체 구조와 병목 정리` | `C:/w/lostark-structure-cleanup-0829`, tracked dirty 2,655건 | 수천 건 삭제 WIP라 통병합하지 않았다. build profile, Effect project entry 제거, 문서 계약처럼 현재 코드로 재현 가능한 부분만 반영했다. |
| `쿠크세이튼 전체 리소스 정본 반영` | `C:/Users/user/.codex/worktrees/4fc7/LostArk`, detached `9174f17c`, 총 103건 | 검증된 map/intake JSON과 validator/test만 해시 대조 후 이식했다. 미완성 Product Level은 이식하지 않았다. |
| `Find all LostArk worktrees` | 08-29 조사 중 다른 정리 작업이 동시에 폴더를 이동·삭제 | 과거 63개 등록 결과를 현재값으로 재사용하지 않고 08-30에 다시 측정했다. 현재 등록은 29개다. prune/휴지통 비우기는 하지 않았다. |
| `이펙트 애니메이션 툴 통합` | 설계 결과는 V1/V2/local/server 경로 분리를 핵심 병목으로 지적 | 두 Effect 도구를 삭제·복제하지 않고, orchestration layer의 `Complete Play`를 실제 Server pattern 경로에 연결했다. |
| `에스더 발탄 사운드 추출` | 원본 물리 Resources에 WAV가 존재 | Valtan Server-hit sound 계약은 Workbench 기준점에 포함됐다. 에스더 신규 WAV 3개는 아직 Esther runtime cue에 연결되지 않았다. |

`lostark-structure-cleanup-0829`의 대규모 삭제와 효과 레거시 절단은 완료 commit이 아니며, 현재 branch에
그대로 합치면 Kakul/Workbench 변경과 무관한 수천 파일을 잃을 수 있다. 이를 “다른 세션 통합 완료”로
표현하지 않는다.

## 3. 현재 물리 폴더와 worktree 상태

### 3.1 이번 작업에 직접 관련된 네 경로

| 경로 | branch / HEAD | 현재 상태 |
|---|---|---|
| `C:/Users/user/Desktop/LostArk` | `codex/release-runtime-finalization` / `9174f17c` | 총 2,269: modified 77, deleted 2,170, untracked 22 |
| `C:/Users/user/.codex/worktrees/4fc7/LostArk` | detached / `9174f17c` | 총 103: modified 47, deleted 1, untracked 55 |
| `C:/Users/user/Desktop/CodexWorkTree/LostArk-action-presentation-tool` | `codex/action-presentation-integrated-tool` / `c75e3623` | clean |
| `C:/Users/user/Desktop/CodexWorkTree/LostArk-captain-unified` | `codex/captain-unified-authoring-kakul` / `c75e3623` + 이번 변경 | commit 전 tracked 44, untracked status root 13 |

원본의 2,170개 삭제를 이번 작업이 만든 것이 아니다. reset, checkout, stage, commit, worktree prune으로
원본을 정리하지 않았다.

### 3.2 2026-08-30 현재 Git 등록 worktree 29개

`dirty`는 빠른 tracked-only 측정값이다. untracked는 별도 정밀 감사가 필요한 경로가 있다.

| 경로 | branch / HEAD | dirty |
|---|---|---:|
| `C:/Users/user/Desktop/LostArk` | `codex/release-runtime-finalization` / `9174f17c` | 2,247 |
| `C:/Users/user/.codex/worktrees/4fc7/LostArk` | detached / `9174f17c` | 48 |
| `C:/Users/user/.codex/worktrees/dm-ba-effect-pr/LostArk` | `codex/effect-selected-direct-publisher` / `d676115f` | 0 |
| `C:/Users/user/.codex/worktrees/effect-manual-authoring/LostArk` | `codex/effect-manual-authoring-canary-retirement` / `81f24a8a` | 0 |
| `C:/Users/user/.codex/worktrees/pr-split-profiler/LostArk` | `codex/four-client-client-hotpaths` / `3fe9ebac` | 0 |
| `C:/Users/user/.codex/worktrees/pr-split-serverperf/LostArk` | `codex/artist-e-crane-modelcue` / `b8653bfd` | 1 |
| `C:/Users/user/.codex/worktrees/pr-split/LostArk` | `codex/dimensionmaster-ba-two-stage` / `3dcd0528` | 0 |
| `C:/Users/user/.codex/worktrees/valtan-authoring-pr/LostArk` | `codex/valtan-authoring-data-contract` / `3cd2f4d2` | 0 |
| `C:/Users/user/.codex/worktrees/valtan-balance-donut-sandbox/LostArk` | `codex/valtan-balance-donut-sandbox` / `07cbd888` | 35 |
| `C:/Users/user/.codex/worktrees/valtan-hot-runtime` | `codex/valtan-visual-review-handoff` / `a2f6c4d3` | 46 |
| `C:/Users/user/Desktop/CodexWorkTree/LostArk-action-presentation-tool` | `codex/action-presentation-integrated-tool` / `c75e3623` | 0 |
| `C:/Users/user/Desktop/CodexWorkTree/LostArk-captain-unified` | `codex/captain-unified-authoring-kakul` / `c75e3623` | 44 |
| `C:/Users/user/Desktop/CodexWorkTree/LostArk-pipeline-cleanup` | `codex/pipeline-cleanup` / `68cabd25` | 3,859 |
| `C:/w/lostark-structure-cleanup-0829` | `codex/project-structure-cleanup` / `68cabd25` | 2,655 |
| `C:/w/net-recovery` | `codex/lan-discovery-session-recovery` / `0a08b084` | 0 |
| `C:/w/p247` | `codex/post-merge-valtan-party-regression` / `a182b0cf` | 0 |
| `C:/w/valtan-arena-next` | `codex/valtan-arena-navigation-next` / `a6871a2f` | 65 |
| `C:/w/valtan-combat-completion` | `codex/valtan-combat-ghost-completion` / `d77d7e02` | 976 |
| `C:/w/valtan-flow-verify-0828` | detached / `1a9bce42` | 1,174 |
| `C:/w/valtan-half-ring-clock` | `codex/valtan-half-ring-clock` / `a6871a2f` | 65 |
| `C:/w/valtan-patterns-0828` | `codex/valtan-pattern-completion-0828` / `1a9bce42` | 0 |
| `C:/w/valtan-playall-sync` | `codex/valtan-play-all-timeline-sync` / `1a9bce42` | 15 |
| `C:/w/valtan-sector-wave-stone` | `codex/valtan-sector-wave-stone` / `d77d7e02` | 44 |
| `C:/w/vsp` | `codex/valtan-server-pattern-play` / `e507902b` | 0 |
| `C:/w/vw` | `codex/valtan-whirlwind-spin-binding` / `b3b36d33` | 0 |
| `C:/wt/la-balance-return` | `codex/balance-tool-character-select-return` / `52fc2e91` | 2 |
| `C:/wt/la-hf210` | `codex/hotfix-main-gameplay-bootstrap-compat` / `0ff4b3de` | 2 |
| `C:/wt/la-pr218` | detached / `bd6a8a42` | 2 |
| `C:/wt/la-v19fix` | `codex/hotfix-valtan-bootstrap-v19-admission` / `7dab9277` | 0 |

현재 이 목록은 08-29 전수조사의 63개보다 작다. 다른 정리 작업이 휴지통 이동과 Git metadata 변경을
동시에 수행했던 기록이 있으므로 `git worktree prune`, branch 삭제, 휴지통 비우기는 별도 보존 감사 전
실행하면 안 된다.

## 4. 통합 Tool 구현

### 4.1 비독점 workspace

`CMainApp`의 하나짜리 active tool enum을 도구별 visibility 상태로 바꿨다. 다음 도구를 동시에 열 수 있다.

- Resource Files
- Action Presentation Workbench / Animation Tool
- Effect Tool V1
- Effect Tool V2
- Boss Tool
- Map Tool
- HUD/UI Tool
- Camera Tool
- Rendering/Balance 도구

Map, Animation, Camera가 동시에 update될 때 같은 입력을 모두 소비하지 않도록 explicit input owner를 둔다.
현재 owner는 Developer Tools에서 선택한다. 실제 ImGui focus가 owner를 자동 전환하는 단계는 남아 있다.

### 4.2 Resource Files

트리는 최초 open 또는 수동 Refresh에서 최대 50,000개 항목을 한 번 스캔한다. 매 frame recursive scan하지
않는다.

표시 root는 다음과 같다.

- Physical Resources: `Character`, `Effect`, `Sound`, `Map`, `Deploy`, `UI`, `Fonts`
- Source Data: `Animation`, `Valtan`, `Encounters`, `Actors`, `Effects`, `Effects/V2`, `Sound`, `Maps`,
  `Worlds`, `Navigation`, `UI`, `Balance`, `Items`, `Rendering`, `ResourceIntake`
- Published DataFiles: `Map`, `World`, `Navigation`, `Rendering`

선택 항목은 workspace-relative path와 domain을 유지한다. KakulSaydon은 사용자 컬렉션명으로 묶지만
`LV_LUT_MIDNIGHTC_ED`, `MN_RPCT_*`, `MN_RPCZ_00` 같은 stable ID와 WModel 내부 상대 경로를 물리적으로
바꾸지 않는다.

### 4.3 Complete Play

다음 버튼이 모두 같은 함수로 들어간다.

- Global Complete Play
- Animation/Workbench Complete Play
- Effect V1 Complete Play
- Effect V2 Complete Play
- Boss Complete Play
- Map/HUD/UI/Camera에서 선택 pattern의 Complete Play

실제 경로는 다음과 같다.

```text
stable pattern ID
  -> CBossTool::Play_ServerPattern
  -> CValtanPatternAuditionService
  -> C2S typed audition command
  -> Server CGameRoom fixed tick
  -> animation/effect/sound/combat-object/world event replication
```

Level이 Valtan Arena가 아니거나 Server가 연결되지 않았거나 replicated Valtan이 없으면 실패 이유를
표시한다. local clip/model preview는 `Local`로 명시하며 Complete Play 성공으로 취급하지 않는다.

### 4.4 Workbench typed inspector와 Save

Workbench가 표시·편집하는 Server gameplay field:

- stage duration
- collider shape: NONE, CIRCLE, SECTOR, RECT
- radius, inner radius, angle, length, half width
- hit count, first delay, interval
- signed push range와 push time
- knockdown과 down time
- high-jump AIRBORNE duration

push 방향은 signed range, 속도는 `abs(range) / time`에서 파생한다. high-jump의 공중 대기 시간은
combat-object `lifeMs`나 hit `atMs`와 섞지 않는다. stable pattern/stage/damage/attachment ID는 읽기
전용이다.

사용자 버튼은 `Save` 하나다. 내부적으로 validate, authoring save, Product candidate 생성, 가능한 경우
Server/Client two-phase apply를 수행한다. 다만 현재 구현은 Gameplay source/Product와
`Valtan.combatobjectsoundcues.json`을 하나의 joined filesystem transaction으로 묶지 않는다. Sound 저장이
마지막에 실패하면 Gameplay Product는 이미 저장될 수 있다. active runtime은 실패 시 보존되지만,
cross-domain authoring atomicity는 후속 P1이다.

## 5. Valtan 경기장과 8인 admission

### 5.1 벽·파편·collision·navigation

Workbench/Boss/Resource Files의 Active 표시는 replicated destruction state의 읽기 전용 사실이다.
독립 checkbox를 임의 mutation command로 만들지 않았다.

변경 명령은 다음 다섯 Server preset뿐이다.

1. Fresh: 모든 벽과 바닥 복구
2. Walls Gone / Phase 2 Circle: 벽 전체 제거, 바닥 유지
3. Break 3 o'clock
4. Break 9 o'clock
5. Break 3 + 9

Server는 mesh/debris, collision receiver, dynamic navigation blocker를 같은 destruction revision에서
commit하고 Client는 결과를 관찰한다. 자동 계약은 99개 exact wall footprint, 105 destruction group,
224 binding을 검증했다.

### 5.2 Valtan 8 seat

- `MAX_PARTY_MEMBERS = 4`
- `MAX_VALTAN_RAID_PLAYERS = 8`
- `player_1`부터 `player_8`까지 정확히 8개 enabled spawn
- 첫 8명은 실제 `CGameRoom::Join` 성공
- 9번째는 `ROOM_FULL`
- 한 명 leave 뒤 정확한 released spawn으로 replacement admission
- snapshot codec은 8 participant round-trip

추가 spawn 네 개는 authoring navsource의 walkable cell과 Server collisionBox expanded OBB 검사를 통과한다.
이번 변경은 network seat와 snapshot/admission을 닫은 것이며 4개 bot AI를 만든 것은 아니다.

## 6. Effect와 Sound 실제 상태

### 6.1 Effect project 병목

- Client project의 `Data/Effects` 개별 entry: `0`
- 전체 `None` entry: `190`
- `Content` entry: `0`
- `Sync-EffectDataProject.ps1`: 물리적으로 없음
- `Test-EffectDataProjectRegistration.ps1`: 물리적으로 없음
- 2,405개 Effect entry 동기화 경로는 제거 상태다.

Effect source validator 결과:

- direct Product source 197
- source bytes 70,860,294
- 실제 resource file 1,030 / 78,253,144 bytes
- generated artifact 0
- 허용된 local untracked resource 3

현재 Git `Data/Effects` 자체에는 `Authored` 468개, `V2` 80개와 Imported/Corrections/Contracts 등 큰 역사
자료가 남아 있다. validator의 Product direct source 197개와 폴더 파일 468개를 같은 수로 해석하면 안
된다. 구조 정리 worktree의 대규모 삭제는 완료 commit이 아니므로 이번 branch에서 추가 삭제하지 않았다.

### 6.2 EffectRenderContractHarness

완전히 삭제된 것은 아니다. 현재는 제품 CPP를 직접 포함하지 않는 단일 CPP optional WARP/CSO probe로
축소돼 있고, `Framework.sln`과 중앙 Product/Core/FullDiagnostic profile에서는 제외된다. 따라서 일상
빌드 병목은 아니다. shader closure와 exit-propagation 전용 스크립트가 명시적으로 호출할 수 있는 격리
도구로 남아 있다.

### 6.3 Sound 물리 폴더

원본 `C:/Users/user/Desktop/LostArk/Client/Bin/Resources/Sound` 실측:

| 도메인 | WAV | bytes | 상태 |
|---|---:|---:|---|
| `Asther` | 7 | 2,916,390 | Common 4 + 신규 Sillian 3 |
| `Valtan` | 252 | 151,688,728 | Valtan catalog/preview 물리 입력 |
| `KakulSaydon` | 0 | 0 | playable WAV mapping 없음 |

에스더 신규 세 파일은 cast 1개, shot 2개이며 아직 Esther product cue에 연결되지 않았다.

Valtan `patternsoundcues`와 catalog에는 과거 실측 기준 52 pattern, 190 action, 511 occurrence,
131 logical event가 있다. 130 event가 243 WAV variant로 연결되고 `G_Voltan1_Attack13_Loop1`은 variant가
없다. 이번 Workbench 기준점은 Server combat-object hit가 실제 commit된 tick에 stable hit ID와 world
transform을 보내고 Client가 sound binding을 exact join하는 계약을 포함한다.

## 7. KakulSaydon 정리와 admission

### 7.1 물리 Resources

원본 물리 폴더 실측:

| 경로 | files | bytes |
|---|---:|---:|
| `Map/LV_LUT_MIDNIGHTC_ED` | 1,415 | 343,242,379 |
| `Effect/KakulSaydon` | 697 | 100,169,184 |
| `UI/KakulSaydon` | 3,742 | 212,573,563 |
| 직접 Character 6 root | 52 | 506,671,752 |
| `Sound/KakulSaydon` | 0 | 0 |

직접 Character root는 `MN_RPCT_00`, `MN_RPCT_05`, `MN_RPCT_06`, `MN_RPCZ_00`,
`WP_MN_RPCT_05`, `WP_MN_RPCT_06`이다. `Character/KakulSaydon`은 비어 있다. 이를 무조건 한 폴더로
move하면 Actor catalog와 WModel dependency identity가 깨지므로 Resource Files에서 KakulSaydon virtual
collection으로 묶었다.

Effect/UI는 현재 사용자가 요청한 `KakulSaydon` 물리 폴더에 있고, Map은 canonical Area ID를 보존한다.
빈 alias 폴더를 asset으로 인정하지 않는다.

원본에는 WEM 1,189개 2,523,758,626 bytes와 PCK 184개 18,999,539,477 bytes, 합계 약 21.523GB의
Wwise corpus가 있지만 event→media mapping이 해소되지 않았다. UPK의 476 AkEvent 이름을 WAV라고
가장하거나 임의 파일을 `Sound/KakulSaydon`에 넣지 않았다.

### 7.2 통합한 map/intake 데이터

- Area/stable identity: `LV_LUT_MIDNIGHTC_ED`
- map asset 292
- placement 2,951
- source mesh 164
- negative scale 857
- reflected placement 837
- Debug MapTool development row
- Authoring, Imported receipt/inventory, published `.mapassets/.mapplacements`
- bounded fail-closed admission report와 7개 회귀 테스트

integration worktree에는 Git 관리 DataFiles만 있고 ignored physical Kakul pack을 복제하지 않았다. 따라서
validator의 현재 결과는 다음과 같다.

- integration worktree: Resource Collection BLOCKED, geometry WModel 292개 missing, Product 42 findings
- 원본 physical folder: Resource Collection PERMITTED, 하지만 원본 branch에 MapCatalog row가 아직 없어
  Development Geometry BLOCKED, Product 38 findings

두 상태를 합치면 Development map preview dependency closure가 되지만, 원본의 2,269 dirty를 보존하기
위해 자동 복사/merge하지 않았다.

### 7.3 Product Level을 만들지 않은 이유

다음 정본이 없다.

- `Data/Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json`
- `Data/Navigation/LV_LUT_MIDNIGHTC_ED.navsource/.navpaint`
- Server/Client published navgrid/policy/blockers
- `WORLD_ID`, Client Level registry/loader/transition closure
- Server shared simulation registration
- boss placement/profile/encounter/pattern
- playable Kakul Sound Catalog mapping

random world position은 nav/collision 검증을 우회하므로 만들지 않았다. 위 closure 없이 Kakul lobby 버튼,
Server boss spawn, Complete Play를 추가하면 “보이지만 제품이 아닌 두 번째 runtime”이 된다. 현재는 MapTool
Development metadata와 Resource Files collection까지만 admitted다.

## 8. Git ignore / attributes

현재 실제 판정:

| 예시 | Git 상태 |
|---|---|
| `Client/Bin/Resources/.../*.wmodel` | ignored |
| `Client/Bin/Resources/.../*.dds` | ignored |
| `Client/Bin/Resources/.../*.wem` | ignored |
| `Intermediate/.../*.obj` | ignored |
| root의 probe `.obj` | `*.obj`로 ignored |
| `Client/Bin/DataFiles/Map/*.mapassets` | trackable |

`.gitattributes`에는 일반 `*.dds` LFS 규칙이 있지만 `Client/Bin/Resources` 하위 DDS는 상위
`.gitignore`가 먼저 적용돼 Git 대상이 아니다. Product DataFiles는 예외로 다시 열려 있다.

따라서 Kakul WModel/DDS/UI raster/WEM/PCK는 이번 commit에 섞이지 않고, map/intake JSON과 published
DataFiles만 Git 변경이다.

## 9. Debug / Release 화면 계약

자동 source contract로 다음을 확인했다.

- Debug 시작 Level도 Release와 같은 `LOBBY`
- Lobby 제품 명령은 `Test`, `Character Select`, `Valtan`, `Bern` 네 개
- Debug에서 F1 Developer Tools와 ImGui authoring surface 활성
- Release에서 F1 Developer Tools와 authoring ImGui surface 미포함
- Release에서 제품 네 버튼은 그대로 유지

Client 화면을 에이전트가 실행하거나 캡처하지 않았다. 따라서 위 항목은 compile/source admission PASS이며
실제 배치·클릭·visual fidelity 판정은 사용자 수동 검증 대기다.

## 10. 빌드 구조와 병목

### 10.1 profile 실측

| profile | projects | CPP entries | unique CPP | duplicate compile |
|---|---:|---:|---:|---:|
| Product | 4 | 259 | 259 | 0 |
| Core | 6 | 261 | 261 | 0 |
| FullDiagnostic | 12 | 300 | 270 | 30 |

기본 `Framework.sln` Build는 Engine, Shared, Server, Client만 빌드한다. 하네스는 명시 profile에서만
빌드한다. `ValtanFourPlayerHarness`는 물리적으로 퇴역했고 8인 admission은 Server contract와 protocol
contract에서 직접 검증한다.

FullDiagnostic의 남은 30개 중복 compile은 주로 broad harness가 제품 CPP를 직접 포함하는 구조다.
ActionPresentationTimeline은 제품 CPP 15개를 다시 컴파일한다. Product/Core에서는 이 중복이 0이므로
일상 build가 30분이어야 할 구조는 아니다.

남은 큰 build 병목:

1. Server pre-build publisher들이 MSBuild Inputs/Outputs 증분 경계를 갖지 않아 no-op build에서도 실행된다.
2. Engine/Client의 configuration별 Intermediate와 별도 진단 IntDir가 중복된다.
3. FullDiagnostic broad harness 30 duplicate CPP.
4. HLSL producer와 CSO closure는 정상이나 cold cache에서는 shader compile 비용이 크다.
5. DirectXTK PDB가 없다는 LNK4099와 기존 CP949 C4819는 경고이며 이번 실패 원인은 아니다.

### 10.2 Intermediate 실측

원본에는 이름이 `Intermediate`인 폴더가 12개, 총 6,789,213,687 bytes 있었다.

- root `Intermediate`: 5,288,285,238 bytes
- excluded EffectRender probe Intermediate: 661,828,258 bytes
- Server: 365,702,550 bytes
- ActionPresentationTimeline: 218,027,285 bytes
- 나머지 Shared/active harness: 약 255MB

root에서 8월 25~27일 일회성 matrix와 EffectRender Intermediate 24개 경로, 5,902,929,968 bytes를 정확히
검증해 삭제 대상으로 잡았다. 실행 환경이 recursive delete를 정책상 차단해 실제 삭제는 0건이다.

반드시 보존할 root 항목:

- `ValtanTuningCandidates`
- `ValtanFlowReloadNextFix`
- reserved `ValtanProductProjection` 및 현재 tuning authoring/runtime state

삭제 후보는 `CodexFinalBuildMatrix`, `CodexValtanScaleFinal`, `CodexValtanFinal`,
`ClientValtanRevisionVerify`, `CodexVerify`, `Validation`, `CodexG02`, `g01-*`, `ContractCompile`,
`CodexValtanSplitResume`, `GrabCompile`, 과거 abort/network/camera/grab/race 검증 폴더와
`Tools/EffectRenderContractHarness/Intermediate`다. 모두 Git ignored 재생성 산출물이다.

## 11. 자동 검증 결과

### 11.1 최종 build

| 명령 | 결과 | wall time |
|---|---|---:|
| Debug Product full integration build | PASS | 99.033s |
| Debug Core 최종 | PASS | 117.449s |
| Release Core 최종 incremental | PASS | 140.003s |
| Release Core 첫 cold-ish build | PASS | 약 5m 50s |

Debug/Release Core 모두 Product build, CSO closure, Valtan split pattern Validate, balance/world/nav/rendering,
NetworkProtocol, 실제 Character Select private/shared isolation을 통과했다.

### 11.2 변경 domain

- release surface: 3 PASS
- build profile: 7 PASS
- team endpoint: 3 PASS
- Valtan 8 seat: 3 PASS
- Workbench: 10 PASS
- Kakul admission: 7 PASS
- Effect Tool saved-row: 35 PASS, 7 intentional skip
- Valtan split master: 73 PASS in 293.590s
- Animation Tool master: 12 PASS
- Server gameplay contract: 최종 `failures: 0`, 44.437s
- Character Select Debug: Core 23.470s, Party2 19.295s, Party4 19.389s
- ActionPresentationTimeline: PASS, 9.926s
- PointLightFalloff: PASS, 0.284s
- Physics: PASS, 0.231s
- WModelGeometry: PASS, 0.603s
- Rendering draft/runtime: PASS
- World gameplay/nav/wall/destruction: PASS

Server 광역 계약 첫 실행은 과거 `Load exactly four enabled Valtan player spawns` 기대값 한 줄 때문에
1건 실패했다. 이를 `MAX_VALTAN_RAID_PLAYERS` 상수 기반으로 바꾸고 static regression을 추가한 뒤
Debug Server rebuild와 광역 계약을 다시 실행해 0건으로 닫았다.

### 11.3 FullDiagnostic 물리 blocker

정본 FullDiagnostic runner는 원본 Resources의 다음 항목이 없어 중간에서 fail-closed했다.

```text
Character/Valtan/Ghost/MN_RPBF_02.wmodel
Character/Valtan/Ghost/MN_RPBF_02_AnimSet.wmodel
Character/Valtan/Ghost/textures/mn_rpbf_01_ghost_d.dds
Character/Valtan/Ghost/textures/mn_rpbf_01_n.dds
Character/Valtan/Ghost/textures/mn_rpbf_01-1_d_loc_int.dds
Character/Valtan/Ghost/textures/mn_rpbf_01-1_n_loc_int.dds
Character/Valtan/Ghost/textures/mn_rpbf_01-2_d_loc_int.dds
Character/Valtan/Ghost/textures/mn_rpbf_01-2_n_loc_int.dds
```

과거 확인값:

- body: 42,166,008 bytes, SHA-256 prefix `EC69F9E46B73A65C949048B1D7FF0ACF`
- AnimSet: 46,540,308 bytes, SHA-256
  `cb21ca2cb57a439966789bfee4a0ee9d91ee9ce521f7f9c006fd4cc2824a3998`

문서에 남은 `C:/LostArkExtract/GhostValtan_20260822`와
`C:/LostArkExtract/GhostValtanCook_20260822`도 현재 물리 디스크에 없다. 가짜 model이나 normal Valtan
복사본으로 gate를 우회하지 않았다.

이 gate 전의 floor crack, Valtan master/effect/balance/world/nav/destruction 검증은 통과했고, gate 뒤
native/Server/party harness는 개별 실행해 모두 통과했다. 따라서 코드 회귀와 Ghost physical closure
누락은 분리돼 있지만 정본 FullDiagnostic 전체 결과는 `BLOCKED`, PASS가 아니다.

## 12. endpoint와 수동 실행

정본 endpoint는 `192.168.0.14:7777`이다.

- Wi-Fi 2가 `192.168.0.14`를 실제 소유
- Client default/debugger contract 갱신
- Server bind는 `0.0.0.0:7777`
- 격리 harness `127.0.0.1` 유지
- endpoint test 3 PASS

`Sync-TeamLanEndpoint.ps1`는 local debugger 설정까지 갱신했지만 TCP 7777 LocalSubnet firewall rule을
추가하는 단계는 관리자 권한이 필요해 완료하지 못했다. Server PC에서 관리자 PowerShell로 다음을 한 번
실행해야 한다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Network/Sync-TeamLanEndpoint.ps1
```

Server가 현재 `not-listening`인 것은 설정 실패가 아니다. 사용자가 실행하기 전 상태다.

## 13. 사용자 수동 검증 순서

원본은 대규모 dirty이므로 먼저 이번 integration commit을 안전하게 반영해야 한다. 원본을 clean하게
보존하거나 다른 변경을 commit한 뒤 cherry-pick한다. 그 전에는 원본에서 `git add .`를 실행하지 않는다.

반영 후:

1. Server PC 관리자 PowerShell에서 endpoint sync를 실행한다.
2. `Framework.sln` x64 Debug의 `Server + Client` profile을 사용해 사용자가 `Ctrl+F5`한다.
3. Lobby의 제품 버튼 네 개와 F1 Developer Tools를 확인한다.
4. Valtan 진입 후 Resource Files, Effect V1, Effect V2, Workbench, Boss, Map을 동시에 연다.
5. 같은 stable pattern을 각 도구의 Complete Play에서 눌러 모두 Server lifecycle로 실행되는지 본다.
6. 도끼 착지의 effect와 Server-hit sound, world transform을 확인한다.
7. Workbench에서 stage duration/high-jump AIRBORNE/collider/push/down을 수정하고 Save한 뒤 같은 pattern을
   다시 Complete Play한다.
8. Fresh, Walls Gone, 3시, 9시, 3+9 preset마다 mesh/debris, collision, 이동 가능한 nav 결과를 본다.
9. 이번 변경은 AI 봇을 구현하지 않았으므로 8개의 실제 네트워크 Client로 8번째 admission과 9번째 ROOM_FULL을 확인한다. human/bot 조합 검증은 Winters AI 수직 슬라이스가 구현된 뒤 별도로 수행한다.
10. x64 Release에서는 Lobby 제품 버튼 네 개만 있고 F1/ImGui가 나오지 않는지 확인한다.

KakulSaydon은 이번 변경에서 Product lobby/server 진입 대상이 아니다. MapTool의 Development row와
Resource Files collection만 확인한다. Server Level처럼 보이면 오히려 admission 경계 위반이다.

## 14. 다음 수직 슬라이스

우선순위는 다음과 같다.

1. 원본 dirty 2,269건을 소유 작업별 commit/보존하고 이번 integration commit을 cherry-pick한다.
2. Ghost Valtan exact body/AnimSet/textures를 원본 근거에서 재추출해 FullDiagnostic를 다시 실행한다.
3. Gameplay + Sound joined Save를 stage/validate/commit/rollback 한 transaction으로 닫는다.
4. Server publisher에 MSBuild Inputs/Outputs를 주고 no-op pre-build 비용을 제거한다.
5. FullDiagnostic broad harness의 제품 CPP 30개 재컴파일을 작은 presentation core library 또는 좁은
   contract executable로 수렴시킨다.
6. Kakul world/nav/placement spawn admission을 먼저 만든 뒤 boss/profile/pattern/sound를 별도 수직
   슬라이스로 추가한다.
7. 4 Server AI actor의 brain, skill command, revive, party/raid ownership과 8인 실제 raid regression을
   구현한다.
