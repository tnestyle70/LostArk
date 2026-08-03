# LostArk 통합 프레임워크·팀 하네스 최종 현황

작성일: 2026-08-03
작업 브랜치: `codex/baren-player-replication`
문서 성격: 현재 구현, 운영 정본, 검증 증거, 다음 수직 슬라이스의 경계를 함께 기록하는 RESULT

## 1. 결론

이번 정리 단계에서 리소스 루트, 제품/개발 레벨, Loader, MapTool publish, world gameplay publish, Server 30 Hz, protocol/smoke/build harness, 팀·AI 작업 규칙을 하나의 실행 계약으로 묶었다.

다음 항목은 실제 코드와 자동 검증까지 닫혔다.

- `Client/Bin/Resources` 평탄화와 immutable 외부 팩 lock/manifest
- 수업용 Level/Object/Monster/astar 및 `Resources/LostArk` wrapper의 active 경로 제거
- Lobby 시작, F1 Developer Tools와 F6 follow/free camera, 나머지 F키 전역 전환 제거
- 제품 씬 Lobby/Bern/Valtan과 stable scenario catalog
- 제품 map load scope와 Loader/Map runtime의 동일 scope 소비
- Bern shard-set MapTool authoring publish의 원자 교체와 rollback
- Bern/Valtan gameplay JSON에서 Server bootstrap 세트로의 원자 publish와 rollback
- Server 누적 deadline 기반 fixed 30 Hz
- Q/W skill command, Server approval, damage/cooldown/resource와 action snapshot
- Server navgrid 기반 player path, skill 이동 projection, Valtan chase
- Valtan Server brain의 pattern damage, phase, death
- snapshot을 소비하는 Character action presentation과 `CCombatHUDViewModel`
- Lobby 네 클래스 선택, 선택 클래스 우선 binary load, remote class 최초 스폰 시 on-demand admission
- Server snapshot을 표시하는 class-aware runtime HUD overlay
- 포트 7777의 실제 Server PID, Client exit code, 양쪽 process cleanup을 확인하는 회귀 harness
- 매 실행 전 구성별 report 폴더를 비워 stale PASS가 남지 않는 결과 계약
- ProjectAudit의 deep resource hash, failure injection, legacy/경계 검사

다만 nickname 정책 정리, 클래스별 고유 skill/action, 최종 아트 HUD layout, 파티/레이드 입장, 동적 collider·projectile·knockback은 아직 구현 완료가 아니다. 이 문서는 그 기능을 완료로 기록하지 않는다.

## 2. 현재 Git 상태와 통합 규칙

현재 브랜치는 `codex/baren-player-replication`이다. 인수인계 당시 worktree에는 여러 세션의 변경 237개가 함께 있었으므로 임의로 되돌리거나 중간 commit하지 않았다. 이후 실제 코드·데이터·문서를 대조해 이 RESULT가 설명하는 하나의 통합 변경 단위만 남겼으며, Resources payload와 build/intermediate 산출물은 Git 범위에서 제외했다.

통합 규칙은 다음과 같다.

1. `main`은 정본이며 직접 작업하지 않는다.
2. 사람 작업은 팀 브랜치 관례, Codex 작업은 `codex/<topic>` 브랜치를 사용한다.
3. 한 커밋에는 한 실행 계약만 넣는다. 코드, 소비 JSON/schema, project/filter 등록, harness, PLAN/RESULT를 같은 변경 단위로 묶는다.
4. 다른 담당자의 미커밋 파일과 무관한 변경을 되돌리거나 정리하지 않는다.
5. `Client/Bin/Resources`, EXE/DLL/PDB/CSO, `EngineSDK`, `.vs`, `.codex_tmp`, `_work`, `imgui.ini`는 커밋하지 않는다.
6. runtime resource 변경은 payload가 아니라 `Data/AssetPacks.lock.json`과 immutable manifest를 Git에 둔다.
7. 커밋 전 `git diff --check`, JSON/XML parse, 관련 harness, Debug/Release 회귀 결과를 확인한다.
8. Release에서 실행하지 않은 Development tool smoke를 과거 report로 PASS 처리하지 않는다.

문서 보관 규칙은 다음과 같다.

- 계획: `.md/GB/<MM-DD>/YYYY-MM-DD_<TOPIC>_PLAN.md`
- 결과: `.md/GB/<MM-DD>/YYYY-MM-DD_<TOPIC>_RESULT.md`
- 같은 작업 문서가 있으면 새 파일을 만들지 않고 갱신한다.
- `.md/계획서작성규칙.local.md`, local gotcha는 개인 파일이며 커밋하지 않는다.
- `AGENTS.md`는 공통 행동·경계 정본, `CLAUDE.md`는 프로젝트 구조·빌드·운영 설명이다.

## 3. 리소스 관리 현황

허용된 runtime root는 정확히 다음 여섯 개다.

```text
Client/Bin/Resources/
  Character/
  Deploy/
  Effect/
  Fonts/
  Map/
  UI/
```

마지막 lock 기준 전체는 7,922 files, 4,567,118,211 bytes다.

| domain | files | bytes |
|---|---:|---:|
| Character | 341 | 827,738,929 |
| Deploy | 123 | 62,577,095 |
| Effect | 1 | 32,896 |
| Fonts | 4 | 23,106,912 |
| Map | 7,364 | 3,652,251,526 |
| UI | 89 | 1,410,853 |

현재 pack 정본은 다음과 같다.

- pack: `lostark-resources@2026.08.03.3`
- lock: `Data/AssetPacks.lock.json`
- manifest: `Data/AssetManifests/lostark-resources-2026.08.03.3.manifest.json`
- content SHA-256: `3a5729752af3ab549e58ccf9b1923e4b3d68cbd0c500a4defae7997c8fa70b8d`

`Manage-ResourcePack.ps1`의 계약은 `Snapshot -> Verify -> Publish -> Hydrate`다. Snapshot은 manifest와 lock을 한 트랜잭션으로 취급하며 lock commit 전 강제실패 시 orphan manifest와 임시 파일을 지운다. Publish와 Hydrate는 전체 payload를 staging에서 검증한 뒤 승격한다. 상세 명령은 `Tools/AssetPipeline/README.md`가 정본이다.

Effect 원본 선별은 `Cook-SelectedEffectAsset.ps1`이 `Data/Effects/Cooked` staging에 텍스처와 SHA-256 manifest만 만든다. 이 도구는 런타임 `Resources`를 직접 수정하지 않는다. mesh 입력은 거부하며, mesh는 `ModelAssetConverter`의 `.wmodel` 통합 경로로 만든 complete asset만 resource pack 계약을 통해 승인한다.

금지 항목은 `Resources/LostArk`, `Models`, `Textures`, `SourceData`, `Sound`, raw 추출물, 절대/drive 경로 asset ID, `..` 탈출이다.

수업용 `astar` 루트, `_work/resource-layout-backup/legacy-map-packs/dev-lol-annie`, 구형 `Resources/LostArk` 절대 경로와 제거된 converter를 기록하던 `Data/Effects/Editor/IntakeTest`는 저장소에 존재하지 않는다. 삭제로 복구 불가능하게 숨기지 않고 기존 외부 격리소 `C:\Users\user\Desktop\LostArk_Legacy_Quarantine_20260803`에 보존했다.

## 4. 데이터 정본과 저장 포맷

| 영역 | 정본 | 포맷/계약 |
|---|---|---|
| Level/Scenario | `Data/Levels/LevelCatalog.json` | `lostark.level-catalog` v1 |
| Map high-level catalog | `Data/Maps/MapCatalog.json` | `lostark.map-catalog` v1 |
| Character/Boss/NPC | `Data/Actors/*.json` | stable archetype와 Resources-relative asset ID |
| UI/HUD | `Data/UI/HUD/HUD_Layout.json`, `Data/UI/ScreenUI/ScreenUI.json` | `lostark.ui-layout` v1 |
| visual placement authoring | `Data/Maps/Authoring/<AreaId>/<AreaId>.mapplacements` | placement document v2 |
| visual placement runtime | `Client/Bin/DataFiles/Map/*.mapplacements`, `*.mapset` | 대용량 line format, stable ID |
| gameplay placement | `Data/Worlds/<AreaId>/Gameplay.world.json` | `lostark.world-gameplay` v1 |
| encounter | `Data/Encounters/Valtan/ValtanEncounter.json` | server gameplay profile |
| player/boss balance | `Data/Balance/*.json` | player, skill, damage, boss 수치와 stable 참조 |
| server navigation authoring | `Data/Navigation/<AreaId>.navgrid.json` | walkable cell, height, neighbor 입력의 생성 정본 |
| animation authored/reference | `Data/Animation/Authored`, `Data/Animation/Reference` | 작성 데이터와 추출 참조 분리 |

UI와 gameplay 설정은 JSON만 사용한다. 신규 `.cfg`와 runtime cfg reader는 ProjectAudit에서 거부한다. visual map placement는 수만 행을 다루므로 검증된 전용 line format을 유지하되, MapTool 작성본과 runtime 생성물을 분리한다.

Git 관리 대상 `Data` 파일 42개는 Client 프로젝트의 `96.DataFiles` 아래 `None` 항목으로 연결했다. Actors, Animation, AssetPacks, Balance, Encounters, Levels, Maps, Navigation, UI, Worlds 필터에서 원본을 바로 열 수 있지만 runtime 복사나 두 번째 정본을 만들지는 않는다. ignored Effect 추출 원본은 이 프로젝트 목록에 포함하지 않는다.

## 5. Map과 MapTool

현재 제품 맵은 두 개다.

| Area ID | catalog | placements | assets |
|---|---|---:|---:|
| `LV_BER_BERNCASTLE` | shard-set | 50,017 | catalog rows 3,021 / unique 1,003 |
| `LV_LUT_HEARTRB_ED` | single | 13,103 | 269 |

MapTool의 visual `Save`는 `Data/Maps/Authoring`만 갱신한다. 현재 저장소에는 Training authoring placement가 있고 Bern/Valtan runtime map은 기존 import/publish 결과다. 제품 runtime 파일은 `Publish-MapAuthoring.ps1`만 교체할 수 있다.

Bern처럼 shard-set인 경우 publish는 다음을 보장한다.

- 기존 placement ID의 shard 소속 보존
- 신규 placement를 해당 asset을 포함한 shard에 결정적으로 배치
- 모든 shard placement와 `.mapset`을 staging 후 한 트랜잭션으로 승격
- 중간 강제실패 시 모든 기존 파일 복원
- malformed row, non-finite transform, ID domain 위반, 중복 ID, root 밖 shard path 거부

MapTool gameplay 저장은 `Data/Worlds/<AreaId>/Gameplay.world.json`이다.

- Bern: revision 2, player spawn 4개
- Valtan: revision 2, player spawn 4개 + boss 1개
- 지원 kind: `playerSpawn`, `npc`, `boss`
- Monster kind/catalog/schema: 없음

`Publish-WorldGameplay.ps1`은 두 제품 world를 모두 stage/validate한 뒤 Server bootstrap 세트를 원자 교체한다. 생성된 `.worldbootstrap`은 직접 편집하지 않는다. 상세 계약은 `Tools/MapPipeline/README.md`를 따른다.

`Publish-ServerNavigation.ps1`은 제품 navgrid를 검증하고 활성 playerSpawn/boss가 walkable cell 안에 있으며 Y 오차가 허용 범위 안인지 확인한다. Valtan spawn 네 곳은 실제 nav cell center/height로 정렬했다. Server 생성 navgrid는 직접 편집하지 않는다.

## 6. UI Tool과 JSON 현황

HUD Tool은 두 JSON 문서를 편집·저장한다.

- Combat HUD: 60 slots
- Screen UI: 24 slots
- 합계: 84 slots

저장은 JSON schema/version, slot ID 중복, class ID, resource-relative path, animation frame path를 검사하고 임시 파일에서 원자 승격한다. ImGui는 편집 명령 UI이며 매 프레임 JSON을 다시 읽지 않는다.

`CCombatHUDViewModel`은 `CClientReplication`에서 Server snapshot을 받아 다음 읽기 전용 상태를 제공한다.

- local player current/max HP와 resource
- server tick과 action
- skill input slot, 이름, action ID, cooldown duration/end tick, 표시 damage
- Valtan 이름, current/max HP, phase, server action/action ID

UI 담당자는 이 ViewModel을 소비하며 packet/snapshot을 직접 파싱하거나 Character를 조회하지 않는다. cooldown 남은 시간은 Server tick과 cooldown end tick으로 표시하고 실제 damage/cooldown 판정은 Server가 소유한다.

Bern/Valtan/Training에는 이 ViewModel을 읽는 runtime HUD overlay가 연결되어 선택 클래스, HP/resource, action, class별 skill/cooldown/damage, boss 상태를 표시한다. 최종 아트 layout과 party roster는 아직 남아 있다. 비-Lance class는 Server skill 계약이 없으므로 HUD가 스킬을 꾸며내지 않고 미지원 상태를 표시한다.

2026-08-03 UI authoring/runtime 경계를 재검토했다. `CHUDLayoutTool`은 `Resources/UI` 이미지
palette와 thumbnail, drag/drop, rect/rotation, layer order, hover preview, 두 `Data/UI` JSON의
save/load까지 구현되어 있다. 반면 이 JSON을 제품 `CUIObject` image widget으로 stage/commit하는
runtime factory, reference resolution 기반 2D hit-test router, stable command binding schema는
아직 없다. 따라서 현재 상태를 “ImGui UI가 제품 이미지 UI와 picking으로 자동 전환된다”고
표현하지 않는다. 목표 규칙과 완료 검증은 `AGENTS.md`, `CLAUDE.md`,
`.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`에 정본별 역할로 반영했다.

## 7. Level, Lobby, ImGui 이동 현황

엔진 enum은 `STATIC`, `LOADING`, `LOBBY`, `BERN`, `VALTAN_ARENA`, `DEVELOPMENT` 여섯 값이다. 사용자가 체감하는 제품 씬은 다음 세 개다.

1. Lobby
2. Bern
3. Valtan Arena

`STATIC`과 `LOADING`은 인프라이고 `DEVELOPMENT`는 여러 test/tool을 stable scenario ID로 실행하는 공용 레벨이다. 새 `TEST` enum이나 test world를 만들지 않는다.

제품/개발 scenario는 다음과 같다.

| stable ID | level | 용도 |
|---|---|---|
| `front.lobby` | LOBBY | 제품 시작 |
| `world.bern` | BERN | 제품 월드 |
| `raid.valtan.arena` | VALTAN_ARENA | 제품 레이드 arena |
| `dev.training.ground` | DEVELOPMENT | 서버 연결 수련장 |
| `dev.map.active` | DEVELOPMENT | 전체 active map + MapTool |
| `asset.character.lance-master` | DEVELOPMENT | Character/Animation tool |
| `render.hdr-readback` | DEVELOPMENT | render/effect tool |
| `effect.preview` | DEVELOPMENT | effect tool |
| `ui.hud.layout` | DEVELOPMENT | HUD layout tool |

공식 전역 기능키는 F1과 F6 두 개다. Debug의 F1은 Developer Tools 허브를 열고 F6는 gameplay camera를 follow/free mode로 전환한다. free camera는 WASD 이동과 Tab mouse-look을 사용하며 free mode에서는 gameplay command를 보내지 않는다. F2~F5와 F7~F12 전역 전환은 금지했다. 자동화는 `--smoke --scenario=<stable-id>`를 사용하고 Release에는 Development ImGui tool을 ship하지 않는다.

Lobby Loader는 camera prototype만 준비하며 Map/Deploy/Effect/Character bundle을 로드하지 않는다. Release에서도 ImGui character select panel은 동작한다.

현재 Lobby UI의 실제 상태는 다음과 같다.

- 선택 클래스: Lance Master, Gunslinger, Slayer, Artist 4개
- 목적지 버튼: Bern, Valtan, Training 3개
- nickname 입력: 아직 보이며 빈 값이면 Client가 `Player`로 치환
- Development/Test 버튼: Lobby에는 없음
- 서버 `S2C_ENTER_ACCEPTED` 이후에만 `CSceneTransitionService`로 제품 씬 이동

네 클래스와 Training 진입은 닫혔다. nickname field 제거/서버 fallback 정책은 아직 다음 단계다.

## 8. Loader 계약

Level 추가는 enum만 늘리는 작업이 아니다. catalog, registry, loader, factory, smoke를 한 변경 단위로 추가해야 한다.

Loader의 고정 순서는 다음과 같다.

```text
parse -> validate -> stage -> commit
                         \-> failure: rollback all staged state
```

제품 level은 `LevelCatalog.json`의 정확한 `assetDomains`, `tools`, `mapLoadBounds` parity를 코드와 ProjectAudit에서 검사한다. Bern/Valtan Loader와 `CMapPlacementRuntime`은 같은 `MAP_LOAD_SCOPE`를 소비한다. Loader가 읽고 범위 필터링한 map catalog/placement stage는 runtime으로 handoff해 같은 50,017행 문서를 다시 parse하지 않는다.

Character binary는 Area 진입 때 네 클래스를 모두 디코드하지 않는다. `CPlayableCharacterAssetService`를 단일 admission 경계로 사용해 Lobby에서 선택한 클래스만 Loader가 먼저 준비하고, 다른 플레이어 클래스는 replication spawn에서 최초 한 번만 같은 서비스로 추가한다. 이 변경으로 Debug Bern smoke가 전체 class 선로드 당시 timeout에서 16.9초 PASS로 회복됐다.

실패 계약은 다음과 같다.

- worker factory에서 modal `MessageBox`를 띄우지 않는다.
- 부분 prototype/resource는 rollback한다.
- world load 실패 시 network session을 닫고 partial target resource를 지운 뒤 Lobby recovery를 시도한다.
- Lobby load 자체가 실패하면 Release ImGui에서 `Retry Lobby`를 제공한다.
- 종료는 cooperative cancellation 5초, `CancelSynchronousIo` 후 5초 bounded join 순서다.
- 그래도 worker가 끝나지 않으면 worker만 `TerminateThread`하지 않고 `ERROR_TIMEOUT`으로 process fail-fast한다.

## 9. Server, Client, Character 경계

Server는 누적 없는 `sleep_for(33ms)`를 사용하지 않는다. `1.0 / 30.0` fixed step과 누적 deadline을 사용하고 lag가 커지면 deadline을 재동기화한다.

현재 닫힌 player 경계는 우클릭 move goal과 LanceMaster 긴 창 quick slot 9개까지다.

```text
Raw input -> CPlayerController -> IPlayerCommandSink
          -> C2S command -> RoomCommand queue -> GameRoom
          -> authoritative snapshot -> CClientReplication -> presentation
```

Q/W/E/R/A/S/T/V/Alt+V는 `Data/Balance/PlayerSkills.json`의 `inputSlot`을 `CPlayerSkillCatalog`로 해석해 `C2S_USE_SKILL`로 제출한다. payload에는 PlayerId/NetEntityId가 없고 Server가 SessionId로 player를 결정한다. `CPlayerSkillSystem`이 sequence, class, action, resource, cooldown을 검사하고 nav projection 이동과 damage를 한 번만 적용한 뒤 action/skill/cooldown/HP/resource를 snapshot으로 복제한다.

Character/Animation의 `Logic_*`는 presentation-only다. DirectInput, socket, packet을 읽거나 `Play_Skill`을 직접 호출하지 않는다. `CCharacter::Apply_NetworkAction`만 Server action tick 변경을 받아 승인된 skill animation을 시작한다. 아직 계약이 없는 추가 스킬은 로컬에서 우회 재생하지 않는다.

Actor catalog의 현재 지원 상태는 다음과 같다.

| class | catalog status | 제품 선택/복제 |
|---|---|---|
| LanceMaster | supported | 지원 |
| Gunslinger | supported | 이동/복제/HUD 지원, 고유 스킬 미정 |
| Slayer | supported | 이동/복제/HUD 지원, 고유 스킬 미정 |
| Artist | supported | 이동/복제/HUD 지원, 고유 스킬 미정 |

Valtan actor model과 presentation clip은 `BossCatalog.json`을 Loader와 replication이 소비한다. Server는 Valtan transform/action/target/pattern phase/HP/damage/death를 소유하고 Client는 semantic clip과 HUD 상태를 표현한다. 잡몹은 현재 범위가 아니며 수업용 Monster 계약은 제거 상태를 유지한다.

## 10. 팀 역할과 금지 경계

| 담당 | 소유하는 정본 | 제출 인터페이스 | 금지 |
|---|---|---|---|
| Framework/Integration | AGENTS/CLAUDE, catalog/registry/loader, build/audit | stable contract와 harness | 팀원 feature 내부를 임의 재작성 |
| UI | UI JSON, UI 상태 모델, layout authoring | `CLobbyCommandService`, `CSceneTransitionService`, 향후 gameplay command | packet/socket/`Change_Level`, Character 직접 변경 |
| Character/Animation | `CHARACTER_SPEC`, clip mapping, presentation callback | `CAnimationTargetService`와 복제 action 표현 | DirectInput/socket/damage/cooldown 판정, local skill 우회 |
| Player/Input | quarter-view input와 command 생성 | `CPlayerController -> IPlayerCommandSink` | `CNetworkManager` 직접 include, 위치 직접 확정 |
| Map/Encounter | catalog와 placement, navigation, boss/NPC 배치 | authoring JSON/placement와 publish tool | pointer/prototype/vector index 저장, runtime entity ID 저장 |
| Server/Boss | world/entity/action truth, fixed tick | Shared stable ID와 snapshot/event | Client GameObject, asset path, clip, ImGui 참조 |
| Transport/G10 | byte/frame/session I/O | `RoomCommand` enqueue와 frame send | `GameRoom` 상태 직접 변경 |

Catalog는 생성 가능한 정의를, placement는 instance ID와 Transform을 소유한다. pointer, prototype tag, vector index는 저장 ID가 아니다.

## 11. AI Agent 작업 규율

1. 코드 전에 실제 호출자, 소유자, 데이터 정본, 실패 소비자를 찾는다.
2. 소비자가 없는 interface, 빈 미래용 catalog, placeholder enum을 만들지 않는다.
3. 함수는 하나의 의미 단위를 갖고 이름으로 그 단위를 설명한다.
4. `Manager`, `Data`, `Handle`, `Temp` 같은 포괄 이름만으로 새 상태를 숨기지 않는다.
5. 파일/네트워크/레벨 실패 이유와 rollback 결과를 보존한다.
6. public 계약에는 정상, version/ID/path 오류, 중복, 중간 실패 rollback 검증을 함께 추가한다.
7. 문서 수정만으로 finding을 닫지 않는다. 가능한 finding은 audit/harness/smoke로 바꾼다.
8. 비평 에이전트의 답도 실제 코드·데이터와 대조한 뒤 반영한다.
9. 구현 완료, 자동 검증 완료, 수동 검증 완료, 다음 단계는 별도로 보고한다.
10. 다른 팀원의 dirty 변경을 자동 정리·stage·commit하지 않는다.

## 12. 이번에 실행 계약으로 바꾼 비평 항목

- Server 30 Hz: 누적 없는 33 ms sleep 제거, exact fixed step/deadline 적용
- Process harness: 기존 7777 listener 거부, 실제 listener PID와 spawned Server PID 일치 확인
- Client result: report 존재뿐 아니라 Client exit code 0 확인
- Cleanup: harness가 만든 Client/Server process를 `finally`에서 종료
- Report truth: 구성별 report 디렉터리를 실행 전에 초기화해 stale PASS 제거
- Level metadata: scenario별 `assetDomains`와 `tools`의 exact parity를 code/audit에서 확인하고 실제 Loader/tool 선택에 사용
- Map shard publish: malformed/path escape뿐 아니라 promotion 중 강제실패 후 기존 세트 hash 보존 확인
- World publish: 두 world 중간 promotion 강제실패 후 전부 rollback 확인
- Resource Snapshot: manifest 생성 직후 강제실패에서 orphan/temporary 0 확인
- Legacy: `astar`, course sample backup, 구형 effect smoke, `Resources/LostArk`, legacy mesh cook, cfg reader의 부재 확인

## 13. 최종 자동 검증

정본 명령은 다음과 같다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -DeepAssetHash
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release -DeepAssetHash
```

이번 작업에서는 Debug와 Release의 Engine -> UpdateLib -> Shared/Protocol Harness -> Server -> Client 전체 build를 모두 성공시켰다. 이후 harness의 report cleanup과 PowerShell fixture를 보강하고 같은 EXE로 두 구성을 다시 실행했다.

### Debug 최종 실행

| 검증 | 결과 | 시간 |
|---|---:|---:|
| Protocol Harness | PASS | failures 0 |
| Server Gameplay Contract | PASS | failures 0 |
| `front.lobby` | PASS | 61 ms |
| `world.bern` | PASS | 16,944 ms |
| `raid.valtan.arena` | PASS | 17,484 ms |
| `dev.training.ground` Lance Master | PASS | 12,910 ms |
| `dev.training.ground` Gunslinger | PASS | 12,996 ms |
| `dev.training.ground` Slayer | PASS | 12,977 ms |
| `dev.training.ground` Artist | PASS | 13,072 ms |
| `dev.map.active` | PASS | 2,907 ms |
| `asset.character.lance-master` | PASS | 12,724 ms |
| `render.hdr-readback` | PASS | 1,053 ms |
| `effect.preview` | PASS | 974 ms |
| `ui.hud.layout` | PASS | 126 ms |
| Gameplay/Navigation Validate | PASS | 4 players, 2 skills, 3 damage, 1 boss / Valtan 62x63, Training 32x32 |
| ProjectAudit + deep asset hash | PASS | 58 checks |

### Release 최종 실행

| 검증 | 결과 | 시간 |
|---|---:|---:|
| Protocol Harness | PASS | failures 0 |
| Server Gameplay Contract | PASS | failures 0 |
| `front.lobby` | PASS | 60 ms |
| `world.bern` | PASS | 3,585 ms |
| `raid.valtan.arena` | PASS | 3,490 ms |
| `dev.training.ground` Lance Master | PASS | 2,667 ms |
| `dev.training.ground` Gunslinger | PASS | 2,650 ms |
| `dev.training.ground` Slayer | PASS | 2,667 ms |
| `dev.training.ground` Artist | PASS | 2,682 ms |
| Development ImGui tool smoke | SKIP | Release 미배포 계약 |
| Gameplay/Navigation Validate | PASS | 4 players, 2 skills, 3 damage, 1 boss / Valtan 62x63, Training 32x32 |
| ProjectAudit + deep asset hash | PASS | 58 checks |

중간 Debug 오류창은 `CWorldBootstrap::Load`가 재사용되는 line buffer를 `string_view`로 보관해 area ID가 이후 placement 행으로 변질된 것이 원인이었다. area ID를 staging 문자열로 소유하도록 수정했고 Server 기동, contract test, Debug/Release 전체 회귀에서 재발하지 않았다.

Deep ProjectAudit 58개에는 resource deep verify, Snapshot rollback, level parity, product scope, shard/world publish rollback, JSON-only data, F1/F6 input, Data project/filter parity, transition boundary, actor catalog, 선택 class 우선/on-demand Loader, 네 class runtime Training matrix, Loader termination, Monster 제외, gameplay balance/navigation publish, command→Server truth, HUD ViewModel 경계, 수련장 map/spawn/navigation 계약과 PR #34/#35 정본화 검사가 포함된다.

기존 경고는 third-party PDB 미포함 `LNK4099`, 혼재 인코딩 `C4819`, 일부 narrowing warning이다. Visual Studio 전역 vcpkg target의 `pwsh.exe` 탐색 메시지는 Windows PowerShell fallback 뒤 exit 0인 환경 메시지다.

## 14. 다음 수직 슬라이스: 아직 닫히지 않은 항목

다음 작업은 현재 정리와 섞지 않고 별도 PLAN/harness로 진행한다.

1. nickname 입력 제거와 Server fallback display name 확정
2. Gunslinger/Slayer/Artist 고유 skill/action/damage/animation mapping
3. runtime HUD overlay를 최종 아트 layout widget으로 교체
4. Server 소유 party ID/roster와 좌상단 party HUD
5. 동일 party를 동일 raid instance로 원자 입장시키고 실패 시 전원 rollback
6. 동적 collider, projectile, knockback/피격 판정의 Server 계약
7. revision과 tick-boundary commit을 포함하는 Server-authoritative balance Hot Reload

잡몹과 추가 보스는 실제 요구와 별도 계획·하네스가 승인될 때까지 placeholder 계약을 만들지 않는다.

## 15. 인계 판단

현재 단계의 framework, asset/data publish, player Q/W combat, Server Navigation, Valtan brain, HUD ViewModel은 자동 검증 기준으로 닫혔다. 다음 담당자는 Level/Loader/resource/map/gameplay 경계를 다시 만들지 말고 `.md/TEAM/README.md`에서 연결한 역할별 public 계약 위에서 기능을 확장해야 한다.

## 16. 팀 최초 세팅과 Resource ZIP 인계

팀원은 Git pull과 Resource pack hydrate를 한 세팅 단위로 수행한다. 최초 세팅·일일 문서 갱신 규칙은 `AGENTS.md`, 실제 명령은 `CLAUDE.md`, 역할별 시작점은 `.md/TEAM/README.md`가 정본이다.

현재 배포 산출물은 다음과 같다.

- pack: `lostark-resources@2026.08.03.3`
- ZIP: `lostark-resources-2026.08.03.3.zip`
- ZIP bytes: `2,452,286,024`
- ZIP SHA-256: `61d40a7cb6afda50f7b0f536d247c8c777b4317ae890a2ba52cad855342b239e`
- 원본 manifest: 7,922 files, 4,567,118,211 bytes
- archive 구조 검사: `lostark-resources/2026.08.03.3/{manifest.json,payload,READY}` 확인

ZIP은 Git에 커밋하지 않고 팀 Drive에 checksum 파일과 함께 올린다. 팀원은 외부 pack root에 압축 해제한 뒤 `Manage-ResourcePack.ps1 -Mode Hydrate`, `-Mode Verify` 순서로 검증한다. 저장소 또는 기존 `Client/Bin/Resources` 위에 ZIP을 직접 덮어쓰지 않는다.

## 17. 최소 수련장 Area 계약

`dev.training.ground`는 새 Level enum 없이 `LEVEL::DEVELOPMENT`를 사용하는 서버 연결 수련장으로 추가했다.

- Area/World: `LV_DEV_TRAINING_GROUND` / `WORLD_ID::TRAINING_GROUND`
- visual: RCArena 10종 admission, stable numeric ID를 가진 placement 18개
- gameplay: 특정 클래스에 결합되지 않은 `playerSpawn` 4개, `archetypeId: null`
- navigation: 32×32, cell size 1.0, origin (-16, -16), 높이 0의 결정적 grid
- Client: `CClientReplication`, `CPlayerController`, `CNetworkPlayerCommandSink`, HUD ViewModel 사용
- Server: 독립 `CGameRoom`, navigation projection/path, 30 Hz action/snapshot 사용
- Lobby: `Enter Training`은 `S2C_ENTER_ACCEPTED` 이후에만 DEVELOPMENT로 전환
- 자동 증거: protocol v5 round-trip, class-neutral bootstrap, nav bounds rejection, Q skill 승인과 cooldown HUD smoke

`Resources/Map/LoL/Annie` 3파일은 `C:\Users\user\Desktop\LostArk_Legacy_Quarantine_20260803\Resources\Map_LoL_Annie`로 복구 가능하게 격리했고 `.3` lock과 ZIP에는 포함되지 않는다.

## 18. 팀 문서 입구와 네 클래스 Lobby roster

살아 있는 팀 public 계약은 `.md/TEAM/README.md`를 단일 입구로 통합했다. 담당 인터페이스는 `TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`, Bern/Valtan/Training의 레이어 보유 현황과 optional/missing 경계는 `AREA_DATA_LAYER_GUIDE.md`가 소유한다. 날짜별 PLAN/RESULT는 당시 증거이므로 기존 위치에 보존한다.

Lobby는 Lance Master, Gunslinger, Slayer, Artist 네 slot을 모두 표시하고 선택한다. `.3` pack에는 네 class의 body/equipment/weapon binary가 있고 Character/Server catalog도 네 class를 지원하므로 모두 Bern/Valtan/Training에 진입할 수 있다. Loader는 선택 class를 우선 로드하고 remote class는 첫 spawn에서 한 번만 admission한다. 비-Lance profile 수치는 고유 skill 계약 전의 명시적 training baseline이며 누락 class를 LanceMaster로 대체하는 silent identity fallback은 만들지 않았다.

일반 Monster, wave/증분 spawn, trigger, Area별 balance override, 제품 NPC presentation은 아직 구현되지 않았다. gameplay placement에서 NPC/boss 행이 없으면 생성하지 않고, Valtan deploy pair가 둘 다 없으면 skip하는 현재 동작과 미지원 기능을 구분해 Area 가이드에 기록했다.

## 19. Data 탐색과 밸런스 튜닝 경계

팀원이 pull 직후 수치 정본을 찾을 수 있도록 Client 프로젝트의 `96.DataFiles` 필터에 Git 관리 대상 `Data` 파일을 `None` 항목으로 연결했다. `Data/Balance`를 수정한 뒤 `Publish-GameplayBalance.ps1 -Mode Validate`, `-Mode Publish`, Server 재기동, `dev.training.ground` 확인 순서로 튜닝한다. runtime HUD는 Server snapshot과 class별 정의를 함께 보여 주므로 HP/resource/cooldown/damage/boss 상태를 인게임에서 검증할 수 있다.

실행 중 Hot Reload는 아직 켜지 않았다. Client만 JSON을 다시 읽으면 Server 판정과 표시 revision이 갈라지고 진행 중 action/cooldown/boss phase의 교체 정책도 없기 때문이다. 활성화 조건은 balance revision, 별도 Server stage, room tick 경계 commit, 진행 중 상태 pinning, snapshot revision, Client 동기 commit, rollback harness이며 정본은 `.md/TEAM/BALANCE_TUNING_AND_HOT_RELOAD_CONTRACT.md`다.

## 20. PR #34/#35 최종 통합

최종 통합 브랜치는 `2fff20e`를 계약 기준점으로 삼아 최신 `main`, PR #34의 `origin/CY`, PR #35의 `origin/feature/character-classes`를 모두 merge commit으로 흡수했다. 따라서 두 담당 브랜치의 원본 커밋과 저자 이력은 최종 Git graph에 남는다.

파일 내용은 다음 기준으로 reconciliation했다.

- PR #34의 F8/F9 직접 `Change_Level`은 제거하고 F1/F6, Lobby command, `CSceneTransitionService` 계약을 유지했다.
- PR #35의 `animnotify win=` 분류, NPC package-group/root-motion 조사 문서, Engine/Bone animation crossfade는 보존했다.
- F3, 삭제된 TEST_LEVEL2, 구형 Loader, DirectInput 로컬 skill/stance 재생, 소비자가 없는 `SkillData` runtime은 복원하지 않았다.
- Lance Master stance는 Shared command → Server approval → snapshot 계약을 별도 수직 슬라이스로 추가하기 전까지 활성화하지 않는다.

통합 HEAD에서 Debug/Release 전체 자동화를 다시 실행했다. 두 구성 모두 Engine/UpdateLib/Shared/Server/Client 빌드, Protocol Harness와 Server Gameplay Contract failures 0, Bern/Valtan, 네 클래스 Training smoke를 통과했다. Debug는 Development tool smoke까지 통과했고 Release는 도구 미배포 계약에 따라 skip했다. 최종 Deep ProjectAudit는 58/58이다.

## 21. 담당 영역 하네스의 수직 기능 책임 교정

기존 담당 표와 “다른 담당 영역을 필요 없이 수정하지 않는다”는 규칙은 merge 충돌과 계층 우회를
막기 위한 것이었지만, AI agent가 이를 배타적 파일 권한으로 해석하면 Client command만 만들고
Server authority 구현을 남기는 반쪽 기능이 생길 수 있었다.

현재 규칙은 다음과 같이 교정했다.

- 역할은 작업 시작점, 데이터 정본, authority와 public interface를 나타낸다.
- 기능 담당자는 필요한 Data, Shared, Server, Client, UI와 harness를 한 수직 슬라이스로 구현한다.
- 서버 판정이 필요한 기능에서 Server 파일 수정은 범위 위반이 아니라 완료 조건이다.
- UI의 socket 직접 호출, Character의 damage 자체 판정처럼 계층을 우회하는 구현은 계속 금지한다.
- 다른 팀원의 미커밋 변경을 덮어쓰지 않는 Git 안전 경계도 그대로 유지한다.

`ProjectAudit`의 `team.vertical-slice-ownership` 검사가 `AGENTS.md`, `CLAUDE.md`, 팀 README와
인터페이스 핸드북에 이 의미가 함께 박제되어 있는지 확인한다. 교정 후 ProjectAudit는 60/60으로
통과했다.

## 22. LAN endpoint와 Character resource pack 갱신

Server의 안전한 기본 bind `127.0.0.1`은 유지하고 LAN 실행에서만
`Server.exe --bind-address 0.0.0.0`을 사용한다. Client는 `LOSTARK_SERVER_HOST` process
환경값을 읽고 값이 없거나 `0.0.0.0`이면 `127.0.0.1`로 돌아간다. 개인 사설 IPv4는
`.vcxproj.user` 같은 Git 제외 로컬 설정에만 저장한다.

Character 폴더 갱신을 포함한 현재 Resources는 새 immutable pack으로 잠갔다.

- pack: `lostark-resources@2026.08.03.4`
- manifest: 9,180 files, 5,153,765,021 bytes
- publish root: `C:\Users\user\Desktop\LostArk_Team_ResourcePacks\lostark-resources\2026.08.03.4`
- `Manage-ResourcePack.ps1 -Mode Publish` 내부 Verify: PASS
- Deep ProjectAudit: 58/58 PASS

Resources payload와 개인 IP는 Git에 포함하지 않고 `.4` manifest와 lock만 공유한다.

## 23. 최종 Git 통합과 종료 시 검증

최종 브랜치는 `origin/main` 위에 현재 Character Select/Level/Tool/Map 변경을 고정한 뒤 열린 담당자
브랜치를 merge commit으로 흡수했다.

- PR #39의 weapon pre-transform 수정은 PR #40에 이미 포함되어 중복 적용하지 않았다.
- PR #40의 데이터 기반 LanceMaster 9-skill binding은 보존했다.
- PR #41의 color texture sRGB decode와 MapTool navigation bake 수정은 보존했다.
- PR #38은 담당자 이력과 독립적인 `CustomFont` centering 수정만 보존했다. 제품 ImGui HUD와
  실제 class ID를 바꾸지 않는 display-only relabel은 현재 계약과 맞지 않아 제외했다. Loading JSON이
  참조하는 `UI/Loading/*` 8개 asset도 `.4` pack에 없어서 Loading/UI_Sprite 활성화를 제외했다.

최종 자동·실행 증거:

- Debug Engine/Shared/Server/Client build: PASS
- Protocol Harness / Server Gameplay Contract: `failures : 0`
- gameplay balance: 4 players, 9 skills, 10 damage profiles, 1 boss
- Deep ProjectAudit: 58/58 PASS
- Server listener: `0.0.0.0:7777`, preferred private IPv4의 port `7777` LAN connect PASS
- bounded Server 종료 후 7777 listener: 0
- Release 전체 회귀: 사용자 요청으로 진행 중 중단, PASS로 기록하지 않음
