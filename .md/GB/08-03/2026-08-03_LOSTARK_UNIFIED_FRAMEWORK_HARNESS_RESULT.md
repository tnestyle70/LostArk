# LostArk 통합 프레임워크·팀 하네스 최종 현황

작성일: 2026-08-03
작업 브랜치: `codex/baren-player-replication`
문서 성격: 현재 구현, 운영 정본, 검증 증거, 다음 수직 슬라이스의 경계를 함께 기록하는 RESULT

## 1. 결론

이번 정리 단계에서 리소스 루트, 제품/개발 레벨, Loader, MapTool publish, world gameplay publish, Server 30 Hz, protocol/smoke/build harness, 팀·AI 작업 규칙을 하나의 실행 계약으로 묶었다.

다음 항목은 실제 코드와 자동 검증까지 닫혔다.

- `Client/Bin/Resources` 평탄화와 immutable 외부 팩 lock/manifest
- 수업용 Level/Object/Monster/astar 및 `Resources/LostArk` wrapper의 active 경로 제거
- Lobby 시작, F1 단일 Developer Tools 허브, F2~F12 레벨 이동 제거
- 제품 씬 Lobby/Bern/Valtan과 stable scenario catalog
- 제품 map load scope와 Loader/Map runtime의 동일 scope 소비
- Bern shard-set MapTool authoring publish의 원자 교체와 rollback
- Bern/Valtan gameplay JSON에서 Server bootstrap 세트로의 원자 publish와 rollback
- Server 누적 deadline 기반 fixed 30 Hz
- Q/W skill command, Server approval, damage/cooldown/resource와 action snapshot
- Server navgrid 기반 player path, skill 이동 projection, Valtan chase
- Valtan Server brain의 pattern damage, phase, death
- snapshot을 소비하는 Character action presentation과 `CCombatHUDViewModel`
- 포트 7777의 실제 Server PID, Client exit code, 양쪽 process cleanup을 확인하는 회귀 harness
- 매 실행 전 구성별 report 폴더를 비워 stale PASS가 남지 않는 결과 계약
- ProjectAudit의 deep resource hash, failure injection, legacy/경계 검사

다만 네 클래스 선택, nickname 완전 선택화, 실제 runtime HUD widget 연결, 파티/레이드 입장, 동적 collider·projectile·knockback은 아직 구현 완료가 아니다. 이 문서는 그 기능을 완료로 기록하지 않는다.

## 2. 현재 Git 상태와 통합 규칙

현재 브랜치는 `codex/baren-player-replication`이고 worktree에는 여러 세션과 담당자의 대규모 미커밋 변경이 함께 있다. 최종 감사 기준 `git status --short` 항목은 237개였다. 따라서 이번 작업에서는 자동 stage/commit을 하지 않았다.

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

마지막 lock 기준 전체는 7,741 files, 3,994,538,522 bytes다.

| domain | files | bytes |
|---|---:|---:|
| Character | 157 | 254,562,408 |
| Deploy | 123 | 62,577,095 |
| Effect | 1 | 32,896 |
| Fonts | 4 | 23,106,912 |
| Map | 7,367 | 3,652,848,358 |
| UI | 89 | 1,410,853 |

현재 pack 정본은 다음과 같다.

- pack: `lostark-resources@2026.08.03.1`
- lock: `Data/AssetPacks.lock.json`
- manifest: `Data/AssetManifests/lostark-resources-2026.08.03.1.manifest.json`
- content SHA-256: `0a8b647da3bb78586a121031e58993199fbd8cf63778a256175a29dccd3edad2`

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
| server navigation | `Data/Navigation/<AreaId>.navgrid` | walkable cell, height, neighbor 입력 |
| animation authored/reference | `Data/Animation/Authored`, `Data/Animation/Reference` | 작성 데이터와 추출 참조 분리 |

UI와 gameplay 설정은 JSON만 사용한다. 신규 `.cfg`와 runtime cfg reader는 ProjectAudit에서 거부한다. visual map placement는 수만 행을 다루므로 검증된 전용 line format을 유지하되, MapTool 작성본과 runtime 생성물을 분리한다.

## 5. Map과 MapTool

현재 제품 맵은 두 개다.

| Area ID | catalog | placements | assets |
|---|---|---:|---:|
| `LV_BER_BERNCASTLE` | shard-set | 50,017 | catalog rows 3,021 / unique 1,003 |
| `LV_LUT_HEARTRB_ED` | single | 13,103 | 269 |

MapTool의 visual `Save`는 `Data/Maps/Authoring`만 갱신한다. 현재 저장소에는 아직 이 authoring 디렉터리가 없으므로, 현재 runtime map은 기존 import/publish 결과다. 제품 runtime 파일은 `Publish-MapAuthoring.ps1`만 교체할 수 있다.

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

실제 제품 HUD widget이 이 ViewModel을 최종 시각 배치에 연결하는 작업, party roster, 네 클래스 class owner 정규화는 아직 남아 있다.

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
| `dev.map.active` | DEVELOPMENT | 전체 active map + MapTool |
| `asset.character.lance-master` | DEVELOPMENT | Character/Animation tool |
| `render.hdr-readback` | DEVELOPMENT | render/effect tool |
| `effect.preview` | DEVELOPMENT | effect tool |
| `ui.hud.layout` | DEVELOPMENT | HUD layout tool |

F1은 유일한 전역 Developer Tools 단축키다. F2~F12 레벨·맵·카메라 전환은 금지했다. Debug에서는 F1 허브가 Development scenario를 선택하고, 자동화는 `--smoke --scenario=<stable-id>`를 사용한다. Release에는 Development ImGui tool을 ship하지 않는다.

Lobby Loader는 camera prototype만 준비하며 Map/Deploy/Effect/Character bundle을 로드하지 않는다. Release에서도 ImGui character select panel은 동작한다.

현재 Lobby UI의 실제 상태는 다음과 같다.

- 선택 클래스: LanceMaster 1개
- 목적지 버튼: Bern, Valtan 2개
- nickname 입력: 아직 보이며 빈 값이면 Client가 `Player`로 치환
- Development/Test 버튼: Lobby에는 없음
- 서버 `S2C_ENTER_ACCEPTED` 이후에만 `CSceneTransitionService`로 제품 씬 이동

Artist/Slayer/Gunslinger 선택, nickname field 제거/서버 fallback, Lobby의 Test destination은 아직 다음 단계다.

## 8. Loader 계약

Level 추가는 enum만 늘리는 작업이 아니다. catalog, registry, loader, factory, smoke를 한 변경 단위로 추가해야 한다.

Loader의 고정 순서는 다음과 같다.

```text
parse -> validate -> stage -> commit
                         \-> failure: rollback all staged state
```

제품 level은 `LevelCatalog.json`의 정확한 `assetDomains`, `tools`, `mapLoadBounds` parity를 코드와 ProjectAudit에서 검사한다. Bern/Valtan Loader와 `CMapPlacementRuntime`은 같은 `MAP_LOAD_SCOPE`를 소비한다. Loader가 읽고 범위 필터링한 map catalog/placement stage는 runtime으로 handoff해 같은 50,017행 문서를 다시 parse하지 않는다.

실패 계약은 다음과 같다.

- worker factory에서 modal `MessageBox`를 띄우지 않는다.
- 부분 prototype/resource는 rollback한다.
- world load 실패 시 network session을 닫고 partial target resource를 지운 뒤 Lobby recovery를 시도한다.
- Lobby load 자체가 실패하면 Release ImGui에서 `Retry Lobby`를 제공한다.
- 종료는 cooperative cancellation 5초, `CancelSynchronousIo` 후 5초 bounded join 순서다.
- 그래도 worker가 끝나지 않으면 worker만 `TerminateThread`하지 않고 `ERROR_TIMEOUT`으로 process fail-fast한다.

## 9. Server, Client, Character 경계

Server는 누적 없는 `sleep_for(33ms)`를 사용하지 않는다. `1.0 / 30.0` fixed step과 누적 deadline을 사용하고 lag가 커지면 deadline을 재동기화한다.

현재 닫힌 player 경계는 우클릭 move goal과 LanceMaster Q/W skill까지다.

```text
Raw input -> CPlayerController -> IPlayerCommandSink
          -> C2S command -> RoomCommand queue -> GameRoom
          -> authoritative snapshot -> CClientReplication -> presentation
```

Q/W는 stable skill ID `34060`/`34100`을 `C2S_USE_SKILL`로 제출한다. payload에는 PlayerId/NetEntityId가 없고 Server가 SessionId로 player를 결정한다. `CPlayerSkillSystem`이 sequence, class, action, resource, cooldown을 검사하고 nav projection 이동과 damage를 한 번만 적용한 뒤 action/skill/cooldown/HP/resource를 snapshot으로 복제한다.

Character/Animation의 `Logic_*`는 presentation-only다. DirectInput, socket, packet을 읽거나 `Play_Skill`을 직접 호출하지 않는다. `CCharacter::Apply_NetworkAction`만 Server action tick 변경을 받아 승인된 skill animation을 시작한다. 아직 계약이 없는 추가 스킬은 로컬에서 우회 재생하지 않는다.

Actor catalog의 현재 지원 상태는 다음과 같다.

| class | catalog status | 제품 선택/복제 |
|---|---|---|
| LanceMaster | supported | 지원 |
| Gunslinger | reserved | 미지원 |
| Slayer | reserved | 미지원 |
| Artist | reserved | 미지원 |

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
| `front.lobby` | PASS | 62 ms |
| `world.bern` | PASS | 18,897 ms |
| `raid.valtan.arena` | PASS | 17,835 ms |
| `dev.map.active` | PASS | 3,111 ms |
| `asset.character.lance-master` | PASS | 14,812 ms |
| `render.hdr-readback` | PASS | 968 ms |
| `effect.preview` | PASS | 982 ms |
| `ui.hud.layout` | PASS | 128 ms |
| Gameplay/Navigation Validate | PASS | 1 player, 2 skills, 3 damage, 1 boss / 62x63 |
| ProjectAudit + deep asset hash | PASS | 49 checks |

### Release 최종 실행

| 검증 | 결과 | 시간 |
|---|---:|---:|
| Protocol Harness | PASS | failures 0 |
| Server Gameplay Contract | PASS | failures 0 |
| `front.lobby` | PASS | 61 ms |
| `world.bern` | PASS | 3,704 ms |
| `raid.valtan.arena` | PASS | 3,924 ms |
| Development ImGui tool smoke | SKIP | Release 미배포 계약 |
| Gameplay/Navigation Validate | PASS | 1 player, 2 skills, 3 damage, 1 boss / 62x63 |
| ProjectAudit + deep asset hash | PASS | 49 checks |

중간 Debug 오류창은 `CWorldBootstrap::Load`가 재사용되는 line buffer를 `string_view`로 보관해 area ID가 이후 placement 행으로 변질된 것이 원인이었다. area ID를 staging 문자열로 소유하도록 수정했고 Server 기동, contract test, Debug/Release 전체 회귀에서 재발하지 않았다.

ProjectAudit 49개에는 resource deep verify, Snapshot rollback, level parity, product scope, shard/world publish rollback, JSON-only data, F1-only input, transition boundary, actor catalog, Loader termination, Monster 제외, gameplay balance/navigation publish, command→Server truth, HUD ViewModel 경계가 포함된다.

기존 경고는 third-party PDB 미포함 `LNK4099`, 혼재 인코딩 `C4819`, 일부 narrowing warning이다. Visual Studio 전역 vcpkg target의 `pwsh.exe` 탐색 메시지는 Windows PowerShell fallback 뒤 exit 0인 환경 메시지다.

## 14. 다음 수직 슬라이스: 아직 닫히지 않은 항목

다음 작업은 현재 정리와 섞지 않고 별도 PLAN/harness로 진행한다.

1. Lobby 선택을 정확히 LanceMaster/Artist/Slayer/Gunslinger 네 클래스로 확장
2. Destroyer/Yinyangshi 노출 제거와 Artist canonical ID 정규화
3. nickname 입력 제거와 Server fallback display name 확정
4. 네 클래스 catalog -> Loader -> clone -> remote presentation 완성
5. 실제 runtime HUD widget을 `CCombatHUDViewModel`에 연결하고 class filter 적용
6. Server 소유 party ID/roster와 좌상단 party HUD
7. 동일 party를 동일 raid instance로 원자 입장시키고 실패 시 전원 rollback
8. 나머지 클래스/스킬을 현재 action command 계약으로 확장
9. 동적 collider, projectile, knockback/피격 판정의 Server 계약

잡몹과 추가 보스는 실제 요구와 별도 계획·하네스가 승인될 때까지 placeholder 계약을 만들지 않는다.

## 15. 인계 판단

현재 단계의 framework, asset/data publish, player Q/W combat, Server Navigation, Valtan brain, HUD ViewModel은 자동 검증 기준으로 닫혔다. 다음 담당자는 Level/Loader/resource/map/gameplay 경계를 다시 만들지 말고 `2026-08-03_TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`의 역할별 public 계약 위에서 기능을 확장해야 한다.
