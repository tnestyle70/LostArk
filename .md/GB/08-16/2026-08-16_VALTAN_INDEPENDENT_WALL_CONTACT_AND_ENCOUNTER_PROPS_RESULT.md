# 2026-08-16 발탄 독립 벽 접촉 파괴와 반복 프롭 결과

## 0. 결론

발탄 아레나의 파괴 계약을 "스테이지가 정해 둔 벽 묶음이 한꺼번에 무너진다"에서 "실제로 닿은 벽
하나가 무너진다"로 바꿨다.

- 벽 그룹을 21개 묶음에서 99개 독립 단위로 분리했다.
- `COLLIDER_CONTACT` trigger를 추가해 보스 몸과 저작된 도끼 펄스가 실제로 도달한 벽만 부순다.
- 한 판에 네 번 반복되는 네 기둥은 일방향 파괴 그룹으로 표현할 수 없어 별도 `CEncounterPropRuntime`과
  `S2C_ENCOUNTER_PROP_SYNC`로 분리했다. protocol version은 19에서 20으로 올렸다.
- 109 아레나 붕괴 도약의 착지 좌표를 패턴이 소유하는 anchor 하나로 모아 착지·시네마틱 lookAt·벽
  방사 방향이 각자 좌표를 들고 어긋나지 않게 했다.

판정 권위는 계속 Server fixed tick이다. Client의 축/뼈 collider는 표현용 mirror이며 이번 변경에서도
damage나 파괴 권위를 갖지 않는다.

## 1. 실제 구현

### 독립 벽 단위 분리

`Data/Encounters/Valtan/ValtanWorldEvents.json`의 group과 mutation을 각각 21개에서 99개로,
binding을 22개에서 178개로 늘렸다. 같은 분리를 authoring 문서 네 개에 동시에 적용해야 해서
`Tools/WorldPipeline/Split-ValtanIndependentWallGroups.ps1`을 새로 만들었다. 이 스크립트는
world events, destruction simulation, gameplay world, navigation blocker를 staging에 쓴 뒤
전부 성공했을 때만 교체한다.

navigation blocker는 region 97개로 재작성됐다. 인접한 독립 벽이 남아 있는 동안에는 열린 벽의
cell만 노출되고 나머지 장벽은 닫힌 상태를 유지한다.

### 접촉 기반 파괴

`WORLD_DESTRUCTION_TRIGGER_KIND`에 `COLLIDER_CONTACT`를 추가했다. 이 binding은 의도적으로
pattern, stage, action을 갖지 않는다. 일치시킬 스케줄이 없고 기하만 본다.

- `CServerCollisionSystem::Collect_BossCircleContacts` — 이전 위치와 현재 위치 사이에서 보스 몸이
  실제로 스친 collision box를 전부 수집한다. 가장 이른 하나만 고르지 않는다. 몸이 벽 하나보다
  넓으면 여러 벽에 동시에 닿을 수 있기 때문이다.
- `CServerCollisionSystem::Collect_BossPatternHitContacts` — 저작된 공격 펄스 하나를 아직 온전한
  벽 OBB에 투영한다.
- `CWorldDestructionRuntime::Prepare_ContactTrigger` — 같은 접촉이 두 번 적용되지 않도록 contact
  sequence만 전진시킨다. 파괴는 일방향이므로 이미 부서진 벽에 대한 반복은 `NO_CHANGE`다.
- `CGameRoom::Apply_WorldDestructionBodyContact` / `_PatternHitContact` / `_Contacts`가 tick에서 이를 소비한다.

어떤 공격이 "물리적인 도끼 접촉"인지는 새 문서
`Data/Encounters/Valtan/ValtanWallContactActions.json`이 소유한다. 패턴 11개의 (patternId, stageId,
actionId) 3-튜플을 정확히 명시하며, 포효·마법·바닥 기믹처럼 물리 접촉이 아닌 damage stage는
의도적으로 목록에 넣지 않았다. 카탈로그 쪽 표현은 `BOSS_PATTERN_HIT::bWallContact`다.

109 외곽 링 30개 벽은 일반 접촉으로부터 보호된다. 즉 아레나 붕괴 연출로만 열리고 평소 돌진이나
걷기로는 열리지 않는다.

### 반복 프롭 런타임

벽 파괴 그룹은 `INTACT`에서 한 번 벗어나면 돌아오지 않는다. 네 기둥은 같은 슬롯 네 개가 한 판에서
네 번 올라오고 부서지므로 파괴 그룹을 되감는 대신 `Server/Public|Private/EncounterPropRuntime.*`을
새로 만들었다.

- 슬롯 ID가 복제되는 encounter identity다. Client projection이 이를 기존 내부
  `DEPLOY_ITR_02326` placement 네 개로 해석하며 placement ID가 Server 권위가 되지 않는다.
- `Prepare_Spawn` / `Prepare_Break` / `Prepare_DueRemoval`은 사본에 준비하고 전체가 검증됐을 때만
  commit한다. 절반만 적용된 cycle은 client에 도달할 수 없다.
- occurrence sequence가 전진해야만 상태가 바뀌므로 한 패턴이 같은 cycle을 두 번 만들 수 없고,
  다음 cycle은 같은 슬롯을 재사용한다.

전송은 `S2C_ENCOUNTER_PROP_SYNC` 하나이며 현재 슬롯 집합 전체를 싣는다. 로그가 아니라 전량 교체
상태이므로 늦게 들어온 참가자가 과거 spawn이나 shatter를 재생하지 않아도 정확하다.

기둥 파괴(shatter)의 제품 trigger는 아직 식별되지 않았다. 현재는 Debug audition 전용
`m_iPillarAuditionBreakTick` / `m_bPillarAuditionCycleArmed`만 이 값을 쓴다.

### 인트로 패턴과 Server 도약 anchor

`ValtanEncounter.json`에 `introPatternId: VALTAN_ENTRANCE_WHIRLWIND`와 해당 패턴을 추가했다.
encounter epoch당 한 번, 최초 engage에서만 실행되며 `SERVER_WORLD_ENTITY::bIntroPatternConsumed`가
소비 여부를 소유한다. 늦게 들어온 참가자는 재생하지 않고, room이 비거나 Debug reset일 때만 초기화된다.

109 아레나 붕괴 패턴에는 `serverMotion`을 추가했다.

```json
"serverMotion": {
  "kind": "LEAP_TO_ANCHOR",
  "anchorId": "anchor.valtan.arena-break-109.landing",
  "landingPosition": [156.03, 22.99751, -122.06],
  "apexHeight": 12.0
}
```

`Publish-GameplayBalance.ps1`이 `introPatternId`와 `serverMotion`의 필드 집합, kind 값, 좌표 3개,
apex 범위, anchorId 중복을 검증한다. Server는 이를 `BOSS_PATTERN_MOTION`으로 컴파일하고
`fLeapLandingX/Y/Z`가 보스 placement가 아니라 이 anchor에서만 복사된다.

겸해서 유령 전환 15 패턴의 `PORTAL` 단일 stage를 `PORTAL_OPEN`(1180ms)과 `GHOST_APPEAR`(620ms)로
분리하고, 109 도약의 TAKEOFF/DROP/RECOVERY 지속시간을 원본 타이밍에 맞춰 조정했다.

### 시네마틱 하늘 레이어와 사망 컷

`VALTAN_CINEMATIC_SKY_CUE`와 `VALTAN_CINEMATIC_SKY_STATE`를 추가했다. 카메라와 같은 권위 튜플
(patternId, stageIndex, stageActionId, actionStartTick)에서 해석하므로 시간축이 하나다. 저작된
window가 현재 action age를 덮지 않으면 비활성이고, collision과 navigation에는 절대 닿지 않는다.

**붉은 구름과 검은 구멍의 asset ID는 현재 빈 문자열이다.** 빈 ID는 해당 레이어를 숨긴다는 뜻이며
이펙트 담당자가 채울 자리로 남겨 뒀다. 이 상태에서 화면에 하늘 연출이 보이지 않는 것은 정상이다.

보스 사망 클리어 컷은 key로 삼을 패턴이 없어 `VALTAN_CINEMATIC_CAMERA_INPUT::isBossDead`와
`Find_DeathCue()`로 따로 선택한다. 시작 tick은 여전히 권위 action start tick이다.

### Debug 환경 타임라인과 진단

`CLevel_ValtanArena`에 `ENVIRONMENT_TIMELINE_STEP`, `Start_EnvironmentTimeline`,
`Advance_EnvironmentTimeline`을 추가했다. 기존 audition operation 위에서 한 번만 순서대로 도는
chapter 실행이며, 입구에서 한 번 reset한 뒤 저작된 bar만 넘어가므로 환경이 녹화본처럼 누적된다.

`WORLD_DESTRUCTION_RUNTIME_DIAGNOSTICS`를 모든 destruction 메시지에 실어 Server가 소유한
collision/navigation 카운터를 그대로 보낸다. Debug 패널이 벽 상태에서 통행 가능 여부를 역추론하지
않는다.

### 로비 로드 실패 사유 보존

`Report_LoadFailure`에 `detail`을 추가했다. 거부한 stage가 먼저 보고하고 뒤이어 일반 activation
실패가 따라오므로, 나중의 빈 detail이 유일한 원인 문자열을 지우지 않도록 빈 값은 무시한다.
Lobby 복귀는 socket이 이미 닫힌 상태라 이 문자열이 없으면 연결 끊김과 구분되지 않았다.
`CLevel_Lobby::Update`가 이를 소비해 `Stage loading failed: ...`로 표시한다.

### Client Server host 우선순위

`CNetworkManager::Resolve_ServerHost`에서 `LOSTARK_SERVER_HOST` 환경 변수를 Debug local endpoint
파일보다 먼저 본다. VS debugger 환경이 팀 endpoint 정본이므로, 로컬 편의 파일이 명시된 host를
조용히 덮어쓰면 안 된다. 값이 없거나 `0.0.0.0`일 때만 local 파일과 기본값 `192.168.200.103`으로
내려간다.

## 2. 변경 파일

### 추가

- `Data/Encounters/Valtan/ValtanWallContactActions.json`
- `Server/Public/EncounterPropRuntime.h`
- `Server/Private/EncounterPropRuntime.cpp`
- `Tools/WorldPipeline/Split-ValtanIndependentWallGroups.ps1`

### 수정

- `Shared/Public/Network/PacketType.h`, `PacketMessages.h`, `Shared/Private/Network/PacketMessages.cpp`
- `Server/Public/GameRoom.h`, `GameplayCatalog.h`, `ServerCollisionSystem.h`, `ServerNavigation.h`,
  `ServerWorldEntity.h`, `WorldDestructionRuntime.h`
- `Server/Private/GameRoom.cpp`, `GameplayCatalog.cpp`, `ServerCollisionSystem.cpp`,
  `ServerNavigation.cpp`, `ValtanBrain.cpp`, `WorldDestructionBootstrap.cpp`,
  `WorldDestructionRuntime.cpp`
- `Server/Private/ServerGameplayContractTests.cpp`, `WorldDestructionBootstrapContractTests.cpp`
- `Server/Default/Server.vcxproj`, `Server.vcxproj.filters`
- `Client/Public/ClientReplication.h`, `ClientReplicationEvent.h`, `LevelTransitionService.h`,
  `Level_ValtanArena.h`, `ValtanCinematicCameraController.h`, `ValtanCinematicCameraDocument.h`,
  `WorldDestructionDebrisPresentationRuntime.h`
- `Client/Private/ClientReplication.cpp`, `EncounterPatternReference.cpp`,
  `LevelTransitionService.cpp`, `Level_Loading.cpp`, `Level_Lobby.cpp`, `Level_ValtanArena.cpp`,
  `NetworkManager.cpp`, `ValtanCinematicCameraController.cpp`, `ValtanCinematicCameraDocument.cpp`,
  `ValtanPresentationAssetService.cpp`
- `Client/Default/Client.vcxproj`, `Client.vcxproj.filters`
- `Data/Encounters/Valtan/ValtanEncounter.json`, `ValtanWorldEvents.json`, `ValtanCinematicCamera.json`
- `Data/Animation/Authored/Valtan/Valtan.patternbindings.json`
- `Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json`
- `Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.destructionsimulation.json`,
  `LV_LUT_HEARTRB_ED.deployplacements`
- `Data/Navigation/LV_LUT_HEARTRB_ED.navblockers`
- `Data/Worlds/LV_LUT_HEARTRB_ED/Gameplay.world.json`
- `Client/Bin/DataFiles/Map/LV_LUT_HEARTRB_ED.deployplacements`,
  `Client/Bin/DataFiles/Navigation/LV_LUT_HEARTRB_ED.navblockers`,
  `Client/Bin/DataFiles/World/LV_LUT_HEARTRB_ED.worlddestruction.json`,
  `LV_LUT_HEARTRB_ED.worlddestructionpresentation.json`
- `Tools/WorldPipeline/Publish-WorldGameplay.ps1`, `Publish-ValtanWorldDestruction.ps1`
- `Tools/GameplayPipeline/Publish-GameplayBalance.ps1`
- `Tools/NetworkProtocolHarness/Private/NetworkProtocolHarness.cpp`
- `Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp`

`Client/Bin/DataFiles`의 네 문서는 직접 편집하지 않았다. publisher가 생성한 결과를 그대로 커밋했다.

## 3. 자동 검증 상태

| 검증 | 결과 |
|---|---|
| `Publish-WorldGameplay.ps1 -Mode Validate` | PASS, VALTAN_ARENA 124 placements / 3 spawn groups |
| `Publish-ValtanWorldDestruction.ps1 -Mode Validate` | PASS, groups=99 bindings=111 emitters=99 outer109=30groups/30walls/360fragments impact159=10 contacts=69 |
| `Publish-ValtanWorldDestruction.ps1 -Mode ContractTest` | PASS, revision `d281ea05...e6d8` |
| `Publish-GameplayBalance.ps1 -Mode Validate` | PASS, 6 profiles / 132 skills / 108 damage profiles / 32 boss patterns / 121 stages |
| `Publish-ServerNavigation.ps1 -Mode Validate` | PASS, LV_LUT_HEARTRB_ED 392x312 cellSize 0.5 walkable 21381 |
| `Publish-ServerNavigation.ps1 -Mode ContractTest` | PASS |
| Debug `Server.exe --contract-test` | PASS, `failures : 0` |
| 변경 JSON 전체 parse | PASS, 9/9 |
| `git diff --check` | PASS |

Server contract test에는 이번 계약을 직접 덮는 항목이 포함되어 있다.

- `Accept one exact ACTIVE axe wall-contact row`
- `Reject a wall-contact row whose action does not exactly join its stage`
- `Reject duplicate axe wall-contact ownership atomically`
- `Prepare one exact wall from its independent direct collider contact`
- `Protect every 109 outer wall from ordinary collider contact`
- `Load ninety-nine independent Valtan source-wall destruction units`
- `Expose only the selected wall cells while adjacent independent walls keep the full barrier closed`
- `Initialize the four pillar slots hidden in canonical slot order`
- `Raise the four pillars once and ignore the repeated raise edge`
- `Raise the same four slots again on the next pattern occurrence`
- `Reset the pillars to hidden and refuse a transaction from the old epoch`
- `Compile a 109 landing anchor that is separate from the boss placement`
- `Land the 109 leap exactly on the compiled anchor at IMPACT`
- `Enter the real Debug Stage_Boss trigger and run the entrance sweep first`

사용한 `Server.exe`는 Debug 구성이며 마지막 소스 수정 이후에 빌드된 바이너리다.

## 4. 아직 실행하지 않은 검증

| 항목 | 상태 |
|---|---|
| Engine / Client / Server Debug·Release 전체 빌드 | 미실행 — 사용자가 수행 |
| `NetworkProtocolHarness` 실행 | 미실행 — 빌드 산출물 없음 |
| `ClientFrontendHarness` 실행 | 미실행 — 빌드 산출물 없음 |
| `Invoke-BuildAndRegression.ps1` 정본 회귀 | 미실행 |

protocol version을 19에서 20으로 올렸으므로 Server와 Client를 **함께** 다시 빌드해야 한다. 한쪽만
갱신하면 frame이 거부된다.

## 5. 수동 화면 검증 경계

에이전트는 Client를 실행하거나 조작하지 않았고 화면을 캡처하지 않았다. 아래는 사용자가 직접
확인해야 하며, 관찰 전에는 visual PASS로 기록하지 않는다.

1. 발탄 아레나 진입 후 최초 등장 회전 돌진이 한 번만 재생되는지
2. 보스가 걷거나 돌진해서 벽에 닿을 때 닿은 벽 **하나만** 무너지는지
3. 저작된 도끼 공격이 벽에 닿을 때 부서지고, 포효·마법 stage에서는 부서지지 않는지
4. 109 붕괴 도약이 anchor 좌표에 정확히 착지하고 외곽 링 30개가 그때 열리는지
5. 네 기둥이 올라오고 부서지는 cycle이 같은 슬롯에서 반복되는지
6. Debug 환경 타임라인으로 bar를 순서대로 넘길 때 환경 변화가 누적되는지
7. 로드가 거부됐을 때 Lobby에 거부 사유 문자열이 표시되는지

하늘 연출(붉은 구름, 검은 구멍)은 asset ID가 비어 있으므로 이번 검증 대상이 아니다.

## 6. 남은 경계

- 기둥 shatter의 제품 trigger가 아직 식별되지 않았다. 현재는 Debug audition만 cycle을 구동한다.
- 105 하늘 레이어의 asset ID가 비어 있다. 이펙트 담당자 작업이 남아 있다.
- `PointLight` 계열 조명 작업은 이번 변경에 포함하지 않았다.

## 7. origin/main 병합 (2026-08-16)

`origin/main`이 `bf95a47`(PR #105 LanceMaster identity gauge, PR #106 Esther gauge/Sillian summon)로
앞서 나가면서 이 브랜치와 충돌했다. `git merge origin/main`을 실행해 브랜치 위에서 해결했다.

실제 conflict는 두 파일이다. 나머지는 auto-merge됐다.

| 파일 | 충돌 내용 | 해결 |
|---|---|---|
| `Server/Public/GameRoom.h` | 같은 위치에 `EncounterPropRuntime.h`와 `EstherSkillSystem.h` include 추가 | 두 include 모두 유지 |
| `Server/Private/ServerGameplayContractTests.cpp` | 같은 위치에 서로 독립적인 test block 추가 | 각각 별도 `{ }` scope로 분리해 둘 다 유지 |

`NETWORK_PROTOCOL_VERSION`은 양쪽이 모두 `19 -> 20`으로 올려 auto-merge가 `20`을 남겼다. 병합
결과의 `PACKET_TYPE` 나열은 `C2S_USE_ESTHER_SKILL`과 `S2C_ENCOUNTER_PROP_SYNC`를 함께 가지므로
어느 쪽 `20`과도 다른 세 번째 wire 형태다. 같은 번호를 두면 main만 가진 v20 Server와 이 병합
Client가 handshake를 통과한 뒤 packet ordinal이 어긋나므로 `21`로 올렸다.
`NetworkProtocolHarness`의 `World Destruction Protocol V20 Packet Types` 검사도 `21`로 맞췄다.

병합 후 실제로 실행한 검증은 다음과 같다.

| 검증 | 결과 |
|---|---|
| `Shared` x64 Debug 빌드 | PASS |
| `Server` x64 Debug 빌드 (pre-build publisher 4종 포함) | PASS |
| Debug `Server.exe --contract-test` | PASS, `failures : 0` (기둥 cycle과 Esther gauge test 모두 포함) |
| `NetworkProtocolHarness` x64 Debug 빌드·실행 | PASS, `failures : 0` |
| `Client` x64 Debug 빌드 | PASS (기존 C4819 / LNK4099 경고만) |
| `git diff --check` | PASS |

`ClientFrontendHarness`는 `failures : 9`로 실패했다. 실패 항목은 전부 Effect 저작/Source Authoring
Overlay 계열이고, `git diff 33c0371` 기준으로 이번 병합이 `ClientFrontendHarness.cpp`와 Effect 소스를
한 줄도 바꾸지 않았다. 병합 이전부터 존재하던 실패이며 이번 변경의 회귀가 아니다. 원인 조사는
Effect 담당 작업으로 남는다.

Engine은 양쪽 모두 건드리지 않아 기존 `EngineSDK`로 빌드했다. Release 구성과 실제 Server/Client
동시 실행 smoke는 여전히 사용자가 수행한다.
