# LostArk 팀 게임플레이 인터페이스 사용서

작성일: 2026-08-03
정본 브랜치: `codex/baren-player-replication`

## 1. 한 줄 계약

제품 게임플레이는 항상 다음 한 방향으로 흐른다.

```text
Input/UI intent
-> Client command service 또는 IPlayerCommandSink
-> Shared C2S message
-> Server GameRoom 30 Hz authority
-> Shared S2C snapshot
-> ClientReplication
-> Character/Valtan presentation 또는 CombatHUDViewModel
```

Client는 입력을 빠르게 제출하지만 위치, damage, cooldown, HP, boss phase를 확정하지 않는다. Server가 확정한 snapshot만 제품 화면의 정답이다.

Lobby의 Bern/Valtan/Training은 `Local Preview`와 `Multiplayer`를 명시적으로 선택하며 기본 선택은 Local이다. Local은 canonical `Gameplay.world.json`의 player spawn에 선택 class의 presentation-only Character 하나만 만들고 socket, server ID, registry, command sink, skill/damage/boss authority를 만들지 않는다. Multiplayer는 입력한 서버 IPv4 또는 `localhost`와 port가 연결되고 `S2C_ENTER_ACCEPTED`를 받은 뒤에만 진입한다. 연결 실패·거부 또는 5초 이내 승인 부재는 Lobby에 남고, 진입 후 disconnect는 Lobby로 복귀하며 Local로 자동 전환하지 않는다.

같은 PC 테스트는 Server 기본 bind `127.0.0.1`을 사용한다. LAN 공동 플레이는 Server를 `--bind-address 0.0.0.0` 또는 특정 사설 IPv4로 실행하고 Client Lobby에 Server PC의 사설 IPv4를 입력한다. 자동 서버 탐색은 현재 범위가 아니다.

## 2. 팀원이 먼저 읽을 파일

| 담당 | 시작 파일 | 데이터 정본 |
|---|---|---|
| UI | `Client/Public/CombatHUDViewModel.h`, `LobbyCommandService.h`, `SceneTransitionService.h` | `Data/UI`, `Data/Balance` |
| Player/Input | `Client/Public/PlayerController.h`, `PlayerCommandSink.h` | `Data/Balance/PlayerSkills.json` |
| Character/Animation | `Client/Public/Character.h`, `CharacterSpec.h`, `AnimationTargetService.h` | `Data/Actors`, `Data/Animation` |
| Server/Player | `Server/Public/GameRoom.h`, `PlayerSkillSystem.h`, `ServerNavigation.h` | `Data/Balance`, `Data/Navigation`, `Data/Worlds` |
| Boss | `Server/Public/ValtanBrain.h`, `ServerWorldEntity.h` | `Data/Balance/BossProfiles.json`, `Data/Encounters` |
| Map/Encounter | `Client/Public/MapTool.h`, `WorldGameplayDocument.h` | `Data/Maps/Authoring`, `Data/Worlds`, `Data/Navigation` |
| 통합/검증 | `AGENTS.md`, `CLAUDE.md` | `Tools/Build`, `Tools/ProjectAudit` |

### 2.1 기능 담당자의 수직 슬라이스 책임

<!-- team-contract: vertical-slice-feature-owner; roles-are-not-file-permissions -->

기능 담당자는 수직 슬라이스의 구현 책임자다. 위 표는 어디서 시작하고 어떤 정본을 읽는지
알려주는 표이지, 해당 행 밖의 파일을 수정하지 못하게 하는 권한표가 아니다.

```text
요청 기능
-> authoring/balance JSON과 stable ID
-> 필요한 Shared command/snapshot
-> Server validation·authority·room state
-> Client replication·presentation·ViewModel
-> 실제 UI/Character/Map 소비자
-> protocol/server/client harness와 실패 경로
```

- Player/Input 기능이 Server 판정을 요구하면 그 작업자가 Shared message와 `GameRoom`/skill
  system까지 함께 구현한다.
- Character/Animation 기능이 새 action을 요구하면 balance/action ID, Server 승인 상태,
  snapshot과 presentation mapping을 함께 닫는다.
- UI 기능에 새 runtime 값이 필요하면 UI에서 socket을 읽지 않고 Server snapshot과
  `CCombatHUDViewModel`까지 확장한다.
- Map/Encounter 기능이 spawn이나 encounter truth를 바꾸면 world authoring, publisher,
  Server bootstrap/runtime과 Client presentation을 함께 검증한다.
- 다른 담당자의 미커밋 줄을 덮어쓰거나 public interface를 우회하는 것은 계속 금지한다.
  필요한 교차 영역 수정 자체는 금지하지 않는다.

## 3. 플레이어 입력과 스킬

현재 제품 입력은 다음과 같다.

| 입력 | stable ID | 결과 |
|---|---:|---|
| 우클릭 | move sequence | `C2S_MOVE`로 목표 X/Z 제출 |
| Q | `34060` | 열공참 사용 의도 제출 |
| W | `34100` | 청룡출수 사용 의도 제출 |

`CPlayerController`는 edge input, sequence, aim만 만든다. `IPlayerCommandSink`가 전송 구현을 숨기므로 Controller에서 `CNetworkManager`를 include하지 않는다. Character를 직접 이동하거나 `Play_Skill`을 호출하지 않는다.

스킬 서버 흐름은 다음과 같다.

```text
CPlayerController::Update
-> IPlayerCommandSink::Request_UseSkill
-> CNetworkManager::Send_UseSkill
-> C2S_USE_SKILL { clientSequence, skillId, aimX, aimZ }
-> CServerApp::On_SessionFrame
-> CGameRoom::Handle_UseSkill
-> CPlayerSkillSystem::Try_Start / Update
-> S2C_WORLD_SNAPSHOT
```

Client payload에는 PlayerId와 NetEntityId가 없다. Server가 SessionId로 player를 찾고 sequence, class, 생존, 현재 action, cooldown, resource를 검사한다. 승인하면 이동 목표를 취소하고 action tick, skill ID, cooldown end tick을 Server 상태에 기록한다.

## 4. 이동, Navigation, collider 경계

우클릭 피킹은 입력 목표를 얻기 위한 Client 표현 계층이다. 제품 위치의 정답은 Server Navigation이다.

- 일반 이동: Server가 navgrid에서 시작/목표를 projection하고 8방향 A* path를 만든다.
- 높이: 각 Server nav point의 Y를 사용한다.
- 스킬 이동: `movementDistance`를 action duration에 분배하고 매 tick 다음 위치를 navgrid로 projection한다.
- 보스 이동: `CValtanBrain`이 같은 Server Navigation으로 target까지 path를 계산한다.
- Client `CNavigation`, mesh picking, animation root motion은 Server 위치를 확정하지 않는다.

현재 서버 충돌 계약은 walkable nav cell 경계다. 동적 capsule-vs-capsule, projectile, knockback obstacle collision은 아직 public 계약이 아니므로 Character collider에서 임의로 서버 판정을 대신하지 않는다. 추가할 때는 Server collision owner, shape ID, broad/narrow phase, snapshot correction, harness를 한 변경 단위로 닫는다.

## 5. Character와 Animation

Character는 Server action을 시각화한다.

```text
CClientReplication::Apply_WorldSnapshot
-> CCharacter::Apply_NetworkState
-> CCharacter::Apply_NetworkAction
-> approved skill: Play_Skill
-> locomotion: RUN / IDLE
```

`Set_Locomotion()`만으로 스킬 계약 전체가 닫히는 것은 아니다. 이 함수는 이동 중 RUN, 정지 시 IDLE을 고르는 표현 함수다. 스킬 중에는 locomotion animation 전환을 보류한다. 실제 skill 시작은 snapshot의 `action`, `skillId`, `actionStartTick`이 바뀌었을 때만 `Apply_NetworkAction()`이 수행한다.

Character/Animation 담당자는 clip mapping, part, notify, blend와 재생 결과를 소유한다. damage, cooldown, resource, hit 여부, 위치 정답은 수정하지 않는다. `Logic_*`에서 DirectInput, socket, packet을 읽거나 `Play_Skill`을 직접 호출하지 않는다.

## 6. UI와 밸런스 데이터

UI가 바로 사용할 읽기 경계는 `CCombatHUDViewModel`이다.

`Get_Player()`가 제공하는 값:

- 최신 server tick
- current/max HP
- current/max resource
- server action
- skill ID, input slot, 표시 이름, action ID
- cooldown duration tick, cooldown end tick, 표시 damage

`Get_Boss()`가 제공하는 값:

- archetype ID와 표시 이름
- current/max HP
- phase
- server action과 action ID

쿨타임 남은 tick은 `max(0, cooldownEndTick - serverTick)`이며 UI가 별도 timer를 정답으로 만들지 않는다. 표시 damage는 데이터 정의를 읽은 값이고 실제 피해 적용은 Server만 한다.

밸런스 정본:

| 파일 | 수정하는 값 | 주 소비자 |
|---|---|---|
| `Data/Balance/PlayerProfiles.json` | class별 max HP/resource/move speed | Server spawn, HUD snapshot |
| `Data/Balance/PlayerSkills.json` | slot, 이름, cooldown, action/hit time, cost, 이동 거리, range, damage 참조 | Server skill, UI definition |
| `Data/Balance/DamageProfiles.json` | 실제 정수 damage | Server 판정, UI 표시 |
| `Data/Balance/BossProfiles.json` | boss HP, engage range, speed, phase threshold | Server boss, UI 이름 |
| `Data/Encounters/Valtan/ValtanEncounter.json` | state/action/pattern timing/range/damage 참조 | Server Valtan brain |

UI 담당자는 JSON을 매 프레임 읽지 않는다. `CCombatHUDViewModel::Initialize_Definitions()`가 정의를 준비하고 `CClientReplication`이 snapshot마다 runtime 상태를 적용한다. UI 코드에서 packet, socket, Character, boss GameObject를 직접 조회하지 않는다.

Git 관리 대상 데이터는 Visual Studio Client 프로젝트의 `96.DataFiles` 필터에서 원본을 바로 연다. 이 항목들은 `None` 링크이며 복사본이나 runtime 배포본이 아니다. 수치 튜닝 절차와 무중단 reload를 아직 활성화하지 않은 이유는 `BALANCE_TUNING_AND_HOT_RELOAD_CONTRACT.md`를 따른다.

## 7. Valtan Boss

제품 발탄의 transform, target, action, phase, HP, damage는 Server authority다.

```text
CGameRoom::Tick
-> CValtanBrain::Update
-> nearest living player acquire
-> IDLE / CHASE / PATTERN_WINDUP / PATTERN_ACTIVE / PATTERN_RECOVERY / DEAD
-> damage exactly once in active window
-> S2C_WORLD_SNAPSHOT
-> CValtan presentation + CCombatHUDViewModel
```

`BossProfiles.json`은 boss 기본 수치, `ValtanEncounter.json`은 pattern timeline, `DamageProfiles.json`은 피해량을 소유한다. Client `CValtan`의 로컬 AI는 Development preview 외 제품 정답이 아니다.

수업용 `CMonster`와 `astar/Monster`는 제거 대상 레거시다. Monster runtime/catalog/schema, placeholder enum을 다시 만들지 않는다. 잡몹 요구가 승인되면 실제 archetype, spawn, brain, replication, harness를 별도 수직 슬라이스로 설계한다.

## 8. MapTool과 gameplay 저장

정적 visual 배치와 gameplay 배치는 분리한다.

- visual authoring: `Data/Maps/Authoring/<AreaId>/`
- gameplay authoring: `Data/Worlds/<AreaId>/Gameplay.world.json`
- navigation authoring: `Data/Navigation/<AreaId>.navgrid.json`
- Server world 생성물: `Server/Bin/DataFiles/World/*.worldbootstrap`

gameplay kind는 `playerSpawn`, `npc`, `boss`만 지원한다. placement에는 stable placement ID, kind, encounter ID, position, yaw, enabled를 저장한다. NPC/boss는 stable archetype ID를 소유하지만 `playerSpawn`의 `archetypeId`는 `null`이며 실제 class는 session/player selection이 소유한다. NetEntityId, pointer, Prototype tag, vector index, runtime HP/phase를 저장하지 않는다.

Map/Encounter 담당자가 좌표를 수정하면 navigation publish가 활성 playerSpawn/boss 좌표의 walkable cell과 높이 오차를 검사한다. 생성된 Server bootstrap/navgrid를 직접 편집하지 않는다.

Area별 optional layer, 현재 Bern/Valtan/Training 데이터 보유 현황, Monster/wave/trigger와 NPC presentation의 미지원 경계는 `AREA_DATA_LAYER_GUIDE.md`를 정본으로 사용한다.

## 9. 데이터 변경 절차

```powershell
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/WorldPipeline/Publish-WorldGameplay.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/NavigationPipeline/Publish-ServerNavigation.ps1 -Mode Validate
```

세 publisher는 parse → validate → stage → commit을 따른다. unknown field, schema/version 오류, 중복 stable ID, 잘못된 참조, non-finite 위치, navigation 밖 spawn을 정상값으로 보정해 숨기지 않고 실패시킨다.

새 스킬을 추가할 때는 다음을 함께 변경한다.

1. `PlayerSkills.json`과 참조 `DamageProfiles.json`
2. Character presentation의 stable action/skill mapping
3. 필요한 Shared message/snapshot 확장
4. Server validation/action/damage 처리
5. `CCombatHUDViewModel` 소비 확인
6. protocol harness와 `Server.exe --contract-test`

## 10. Asset과 Git

`Client/Bin/Resources`는 `Fonts, Character, Deploy, Effect, Map, UI` 여섯 root만 허용한다. asset ID는 Resources 상대 경로이며 절대 경로, drive-qualified 경로, `..` 탈출을 금지한다.

대용량 runtime payload는 Git에 올리지 않는다. Drive의 immutable pack으로 배포하고 Git에는 다음만 올린다.

- `Data/AssetPacks.lock.json`
- `Data/AssetManifests/<immutable-pack>.manifest.json`
- source/catalog/authoring JSON과 운영 문서

팩을 바꿀 때는 `Tools/AssetPipeline/README.md`의 Snapshot → Verify → Publish → Hydrate 순서를 사용한다. 개별 파일 덮어쓰기나 팀원별 절대 경로 하드코딩을 금지한다.

팀원이 branch를 pull한 뒤 최초 실행하는 순서는 다음과 같다.

```text
git lfs pull
→ lock과 같은 version의 Resource ZIP SHA-256 확인
→ 외부 pack root에 압축 해제
→ Manage-ResourcePack Hydrate
→ Manage-ResourcePack Verify
→ Debug 전체 회귀
→ 담당 public interface에서 작업 시작
```

Git commit과 Resource ZIP은 한 쌍의 인계 단위다. commit은 `Data/AssetPacks.lock.json`으로 필요한 pack version을 선언하고, Drive에는 정확히 그 immutable version ZIP만 둔다.

기능은 `main`이 아닌 별도 branch/PR로 전달한다. 코드, 소비 데이터, project/filter 등록, harness, RESULT를 같은 검증 단위로 묶는다. build output, `EngineSDK`, `.vs`, `.codex_tmp`, `_work`, `imgui.ini`, Resources payload를 stage하지 않는다.

## 11. 완료 검증

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -DeepAssetHash
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release -DeepAssetHash
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-OfflineClientSmoke.ps1 -Configuration Debug
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-OfflineClientSmoke.ps1 -Configuration Release
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-NetworkEndpointSmoke.ps1 -Configuration Debug
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-NetworkEndpointSmoke.ps1 -Configuration Release
```

자동화 순서는 Engine → UpdateLib → Shared/Protocol Harness → Server build/contract test → Client build → 제품/개발 smoke → balance/world/navigation validate → ProjectAudit이다.

최소 성공 증거:

- Protocol Harness `failures : 0`
- Server gameplay contract `failures : 0`
- Debug/Release lobby, Bern, Valtan 제품 smoke 성공
- Debug/Release `dev.training.ground` 서버 연결 smoke 성공
- Server 미기동 상태의 Bern, Valtan, Training local preview smoke 성공
- Debug Development scenario smoke 성공
- ProjectAudit와 필요한 deep asset hash 성공
- `git diff --check` 성공
- 잔류 Client/Server/7777 listener 없음

## 12. 현재 완료와 다음 경계

완료:

- 서버 권위 우클릭 이동과 Navigation path
- Q/W skill command, server approval, action/damage/cooldown/resource
- snapshot 기반 Character skill/locomotion 표현
- HUD용 player/boss runtime ViewModel
- Valtan 추적, pattern, damage, phase, death
- world gameplay와 navigation 배치 정합성 검사
- `dev.training.ground` 최소 Area, class-neutral player spawn, RCArena 10종 admission, 서버 navigation
- Lobby의 Lance Master/Gunslinger/Slayer/Artist 네 선택 slot, 네 class Loader/Server profile, class별 runtime HUD

별도 수직 슬라이스:

- Gunslinger/Slayer/Artist 고유 skill/action/damage balance와 animation mapping
- runtime HUD widget의 최종 아트 레이아웃 연결
- 추가 스킬
- party/raid admission과 roster
- 동적 collider, projectile, knockback/피격 판정
- 잡몹 및 추가 boss pattern

이 항목들은 현재 인터페이스를 우회해 임시 구현하지 않는다.
