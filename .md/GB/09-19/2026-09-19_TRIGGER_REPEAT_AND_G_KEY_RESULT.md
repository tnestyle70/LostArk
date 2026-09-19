# 트리거 G 키 발동과 재발동 RESULT

작성일: 2026-09-19 (같은 날 사용자 정정으로 개정)
브랜치: `feature/maharaka-island-level` (마하라카 작업과 같은 작업 폴더, 이 문서의 변경은 마하라카 파일과 무관)
범위: 트리거를 밟기만 해서는 발동하지 않고 **볼륨 안에서 G를 눌렀을 때만** 발동하게 한다. 몇 번이든, 어떤 플레이어든 다시 발동한다. 스크립트 흐름의 트리거만 예외로 진입 발동을 남긴다.

## 1. 결론 먼저

- 사용자 정정: "트리거 밟으면 바로 이동하는 게 아니라 G 키를 눌렀을 때 이동되게". 처음 작업은 "걸어 들어가도 발동, G도 발동"이었고 이것은 **틀렸다.** 이번에 바로잡았다.
- 지금 규칙: 트리거는 진입해도 발동하지 않는다. 진입하면 Server가 `[ G ]` 안내를 제안하고, 그 안에서 G를 누르면 Server가 플레이어의 현재 위치로 볼륨을 다시 판정해 발동한다. 반복 횟수 제한과 플레이어 간 소진은 없다. 0.3초 디바운스는 유지한다. **(서버가 안내를 제안하는 동작은 그대로다. 그 안내를 화면에 그리는 표시는 2026-09-19 저녁에 제거함, 11절)**
- **예외**(진입 발동 유지)는 스크립트 흐름 트리거뿐이고, 한 표 `AUTO_ENTRY_RULES`(`ServerTriggerSystem.cpp:15-41`)에 모아 뒀다. 5절에 전체 목록과 이유가 있다.
- **같은 날 오후 개정은 10절이 정본이다.** 이 문서의 "Kouku Mario 레인은 밟으면 발동"이라는 서술은 10절로 바뀌었다.
- 처음 원인(저작 `triggerOnce`가 방 전체 래치가 되는 문제)의 수정은 그대로 유지한다.
- **컴파일은 구문 검사(`cl /Zs`)까지만 했다. 전체 빌드·링크, 계약 테스트 실행, 실제 게임 확인은 하지 않았다.** 7절의 표를 그대로 믿을 것.
- 발견: **베른에는 활성 트리거가 하나도 없다.** 게시본과 저작 원본 모두 베른의 트리거 1개(`valtan`, changeLevel)는 `enabled:false`다. 베른에 새로 깐 트리거는 MapTool 저장 + 게시가 끝나야 Server가 본다(8절).

## 2. 원인 (첫 번째 문제: 한 번 밟으면 다음 사람이 재발동하지 않음)

증거는 코드와 게시된 데이터를 직접 읽은 것이다. 실행 재현은 하지 않았다.

1. 저작 기본값이 once다. `Client/Public/WorldGameplayDocument.h:138` `isTriggerOnce = true`, MapTool 체크박스 `MapTool_WorldGameplayPanels.cpp:530`. publisher가 bootstrap 12번째 필드로 싣고(`Publish-WorldGameplay.ps1:745`) Server가 `placement.isTriggerOnce`로 읽는다(`WorldBootstrap.cpp:310`).
2. Server의 래치 (변경 전 `ServerTriggerSystem.cpp`): 발동에 성공하면 `trigger.hasFired = true`(151-152, 313-315). `hasFired`는 **트리거 하나당 하나**이며 플레이어별이 아니다. 이후 진입 판정은 `wasInside || (isTriggerOnce && hasFired)`이면 건너뛴다(261). 상호작용 제안(252-253), `Activate_Interact`(181), Debug 재생(212, `ALREADY_USED`)도 같은 래치를 본다.
3. 래치는 `Initialize`나 `Reset_SequenceActivation`에서만 풀린다. 방이 비면 초기화하는 `Reset_ReplayableArenaWhenEmpty`(`GameRoom_WorldEntities.cpp:336`)는 Character Select·Valtan·Kouku만 대상이라 **Bern·수련장·마하라카 방은 서버를 다시 켜기 전까지 한 번 소진되면 돌아오지 않는다.**
4. 몬스터 소환 트리거는 래치와 별개로 `CSpawnGroupRuntime::Activate`가 `DORMANT`에서만 성공해(`SpawnGroupRuntime.cpp:47-52`) 한 번만 됐다. 저작 SpawnGroup 11개는 전부 `ONCE`다.
5. 진입 순간 플레이어가 스킬·피격으로 바쁘면 `Begin_MovePlayer`가 거부하는데(`eAction != NONE`) 진입 edge는 이미 소비돼 있었다.

## 3. 원인 (두 번째 문제: 밟으면 알아서 이동함)

- 진입이 곧 발동이었다(`Evaluate_Entries`가 진입 edge에서 `Run_Action`을 바로 실행). 인터랙션이 필요한 박스(`requiresInteract`)만 예외였고, MapTool로 깐 일반 이동 트리거는 전부 진입 발동이었다.
- **Debug Valtan에는 추가 자동 이동이 있었다(현재는 `Stage_Boss` 한 행만 남았다, 14절).** Debug 빌드의 복도 지름길(`Build_ValtanStageBypassMove`)은 원래 `Stage_2`·`Stage_3`·`Stage_Boss`를 밟는 순간 트리거 본래 동작 대신 다음 단계 앞으로 옮겼다. `Stage_2`는 웨이브라 13절에서, `Stage_3`은 저작 절벽 이동을 덮어써서 별도 문서(`2026-09-19_VALTAN_STAGE3_MOVE_RESULT.md`)에서 지름길 표에서 뺐다. 사용자가 Debug에서 본 "그냥 밟으니 넘어가진다"는 이 경우일 수 있다고 봤고, 나중에 사용자가 발탄 `Stage_2`에서 웨이브 대신 앞으로 나간다고 알려 줘서 실제로 `Stage_2`가 그 경우였음을 확인했다(13절).
- G는 Server가 **제안한 상호작용 박스**에만 응답했고, 제안이 없으면 Client가 아무것도 보내지 않았다.
- 안내 표시: 제안(`S2C_INTERACT_PROMPT`)은 어느 방에서든 Client의 HUD 뷰모델에 저장되지만 `[ G ]`를 그리는 곳은 **Kouku 레벨뿐**이었다(`Level_KakulSaydonArena.cpp:2136`). Bern·Valtan에는 그리는 코드가 없었다.

## 4. 변경

| 파일 | 변경 |
|---|---|
| `Server/Private/ServerTriggerSystem.cpp` `AUTO_ENTRY_RULES` (15-41) | **예외 표.** (월드, 액션 종류, 선택적 id 접두사) 행. 나열된 것만 진입 발동. 행을 지우면 그 종류가 G 전용, 추가하면 진입 발동 |
| 같은 파일 `Fires_OnEntry` (296) | 진입 발동 여부: 저작 `requiresInteract`는 항상 G 전용, Debug Valtan 지름길 트리거도 G 전용, 그 외는 표로 판정 |
| 같은 파일 `Evaluate_Entries` (398) | 진입 발동 대상이 아닌 박스는 **제안만** 하고 아무것도 실행하지 않음. 나갈 때 제안 철회. 진입 발동 대상만 종전 경로(방 소유 Mario 진입, `RETRY_WHILE_INSIDE`, 바쁨 재시도) |
| 같은 파일 `Run_Trigger` (328) | G가 실행하는 것: 저작 동작. Debug의 Valtan 지름길 표는 현재 `Stage_Boss` 한 행뿐이라 그 트리거만 Debug에서 G 전용이다(14절). `Stage_Boss`의 ArenaEntry 이동은 `Run_Action`이 진입과 G 모두에서 처리한다. 진입은 이 함수를 부르지 않는다 |
| 같은 파일 `Activate_Interact` (202) / `Activate_Here` (244) | 제안된 박스 ID 또는 `@here`로 G 처리. 진입 발동 대상인 박스는 G가 건드리지 않는다. Server 위치로 볼륨 재판정, 플레이어당 9틱 디바운스, `Source=KEY` 로그 |
| `Server/Public/ServerTriggerSystem.h` | `Set_WorldId`, `Fires_OnEntry`, `Run_Trigger`, `Is_KeyDebounced`, `m_eWorldId`. 첫 작업의 "진입 edge 재무장(`KeyRequested`)"은 삭제 |
| `Server/Private/GameRoom.cpp` (98, 713, 1016) / `GameRoom.h` (1150) | 방이 월드를 트리거 시스템에 알림. 진입과 G가 **같은** 대상 처리(`Activate_TriggerTarget`)를 쓰도록 Tick의 람다 본문을 옮김(Debug 발탄 보스 HP 보정이 G 경로에도 적용됨) |
| `Server/Private/GameRoom_PartyWorld.cpp` (304) | G 처리기가 같은 대상 처리를 사용. 성공해도 제안을 철회하지 않음(플레이어가 아직 안에 있고 반복 가능. 나가면 철회) |
| `Client/Private/Level_Bern.cpp` (716) / `Level_ValtanArena.cpp` (1745) | 제안이 있으면 화면 중앙 아래에 `[ G ]`를 그림. Kouku의 기존 블록과 같은 방식이며 ASCII만 사용. **새 파일·`.vcxproj` 변경 없음** **(2026-09-19 저녁: 이 표시는 제거됨, 11절)** |
| 유지 | 첫 작업의 `triggerOnce` 래치 제거(`ServerTriggerSystem.cpp:83`), `Activate_Repeat`(소환 그룹 재발동), `@here`(`PacketMessages.h`) 및 `Client/Private/PlayerController.cpp`의 G 전송, `[Trigger] Fire` 로그 |

프로토콜 번호는 바꾸지 않았다. Client는 제안된 박스가 있으면 그 ID를, 없으면 `@here`를 보낸다. Server는 어느 쪽이든 그 플레이어의 현재 위치로 다시 판정한다. Server와 Client를 함께 빌드·재시작해야 한다.

## 5. 예외 목록 (진입 발동 유지)

`AUTO_ENTRY_RULES` 한 표. 각 행은 지우면 그 종류가 G 전용이 된다.

| 행 (월드 · 종류 · 접두사) | 트리거 | 개수 | 이유 |
|---|---|---|---|
| 모든 월드 · `playSequence` | Kouku `1Stage_Final`, `Mario*_Intro`, `Mario*_Trigger_*`(시퀀스형), `paper.1/2` 등 | 24 | 진입이 곧 컷신·기믹 시작이다. G를 요구하면 컷신·레이드 진행이 멈춘다 |
| 모든 월드 · `claimCardMazeTelescope` | Kouku `cardmaze.telescope` | 1 | 카드미로 뿅망치가 유일한 발동이다. **진입도 G도 발동하지 않는다**(코드에서 제외) |
| `KAKULSAYDON_ARENA` · `movePlayer` · 접두사 `Mario` | `Mario1_Trigger_1/3/5`, `Mario2_Trigger_2/4/7`, `Mario3_Trigger_4/5/6/8/10/12`, `Mario4_Tigger_5/6/7/13` | 16 | 방의 Mario 진입 처리기(`Begin_MarioTriggerMove`, `MARIO_LANES`)가 소유한 레인 체인이다. 진입 발동이 전제다 |
| `VALTAN_ARENA` · `activateSpawnGroup` | `Stage_1`, `Stage_2`, `Stage_MiniBoss_Spawn` | 3 | 복도 웨이브. 이동이 아니다. Debug에서도 본래 동작으로 밟으면 발동한다(13절, `Stage_2`를 지름길에서 뺌) |
| `VALTAN_ARENA` · `activateEncounter` | `Stage_Boss` | 1 | 보스 시작. 이동이 아니다. **Debug에서는 `Stage_Boss`가 지름길이라 G 전용** |

**G 전용이 된 것** (게시본 기준):

| 월드 | 트리거 |
|---|---|
| `VALTAN_ARENA` | `movePlayer` 5개 전부: `player.spawn.editor`, `player_Move.1`, `Stage_3`, `Stage_Boss_ArenaEntry`, `Stage_MiniBoss`. **Debug에서는 `Stage_Boss`도** (Release 5개, Debug 6개) |
| `KAKULSAYDON_ARENA` | `jump.1`, `jump.2`, `jump.3` (Mario 레인이 아니므로 종전에는 기본 이동이었다). 저작 게이트가 있는 `Mario4_Tigger_2`, `Mario4_Tigger_3`은 종전대로 G 전용 |
| 그 밖의 월드 | 베른·Character Select·수련장·마하라카에는 활성 트리거가 없다. 앞으로 추가되는 모든 `movePlayer`·`changeLevel`·`activateSpawnGroup`·`activateEncounter`는 위 예외에 해당하지 않으면 G 전용 |

집계 (게시본, `Server/Bin/DataFiles/World/*.worldbootstrap`의 활성 triggerBox): Kouku 46개 = 진입 발동 41 + G 전용 5. Valtan 9개 = 진입 발동 4 + G 전용 5 (Debug는 2 + 7). 나머지 월드는 0개.

**한 줄로 바꾸는 법**: Valtan 이동만이 아니라 스폰·보스 시작도 G로 하려면 `VALTAN_ARENA` 행 두 개를 지운다. Kouku Mario를 G 전용으로 하려면 `KAKULSAYDON_ARENA` 행을 지운다(Mario 레인 진행이 G를 요구하게 된다). `jump.*`까지 진입 발동으로 되돌리려면 그 행의 접두사를 `nullptr`로 바꾼다.

**결정한 것 하나**: Valtan `movePlayer` 5개는 예외에 넣지 않았다. 사용자의 요청("밟으면 바로 이동하는 게 아니라 G")이 이동을 가리키고, `player_Move.1`·`player.spawn.editor`가 이동 시험용 이름이라서다. 그 결과 복도의 `Stage_MiniBoss`·`Stage_3`·`Stage_Boss_ArenaEntry` 이동에는 G가 필요하다. `Stage_MiniBoss`의 도착지는 `Stage_MiniBoss_Spawn` 안이라 도착하면 루가루 소환은 자동으로 이어진다.

## 6. 재시도 논리

"막힌 진입을 다음 틱에 다시 시도"하는 로직은 **진입 발동 대상(위 표)에만** 남았다. G 전용 박스는 애초에 그 경로에 들어오지 않는다. 이동 중(`TRIGGER_MOVE`) 접촉은 여전히 재시도에서 제외한다.

## 7. 검증 (단계별)

| 단계 | 내용 | 결과 |
|---|---|---|
| (1) 코드 경로 | 진입 → 제안 → G 요청 → 볼륨 재판정 → 동작, Debug 지름길, Client 표시 경로를 끝까지 읽음 | 완료 |
| (2) 데이터 실측 | 게시된 bootstrap 6개(활성 triggerBox 55개)를 규칙 표대로 분류. 저작 원본(`Data/Worlds/*/Gameplay.world.json`)과 게시본이 같음을 확인 | 완료. 5절의 집계. 순환 없음 |
| (3) 컴파일 | `cl.exe /Zs`(구문·의미 검사, 산출물을 쓰지 않음). Server: `ServerTriggerSystem`, `GameRoom`, `GameRoom_PartyWorld`, 테스트 5개(WorldTriggers·KoukuProduct·SpawnGroups·WorldPlayback·DebugTeleport)를 Debug·Release로. Client: `Level_Bern`, `Level_ValtanArena`, `PlayerController`을 Debug·Release로 | 오류 0. **전체 빌드·링크는 실행하지 않았다** (VS가 켜져 있어 MSBuild 금지) |
| (3) `git diff --check`, 인코딩 | 변경 파일 전부 | 문제 없음. 줄바꿈은 기존 그대로(CRLF), U+FFFD·BOM 없음 |
| (4) 계약 테스트 | 아래 목록을 추가·수정했다. **`Server.exe --contract-test`는 실행하지 않았다.** 어설션을 손으로 추적했을 뿐이다 | **미실행** |
| (5) 실제 게임 | 실행하지 않았다. `[ G ]` 위치·모양도 확인하지 못했다 | **미확인, 사용자 확인 필요** **(2026-09-19 저녁: 이 표시는 제거됨, 11절)** |

계약 테스트 변경 (모두 기존 파일):

- `ServerGameplayContractTests_WorldTriggers.cpp`: 새 블록 4개. (a) once로 저작된 박스를 한 플레이어가 발동한 뒤 다른 플레이어와 같은 플레이어의 재방문이 다시 발동(Valtan 소환, 진입 발동), (b) 바쁜 접촉이 끝난 뒤 같은 접촉으로 이동이 시작(Kouku Mario), (c) **Bern에서 진입하면 제안만 하고 아무것도 실행하지 않음 → G가 실행 → 디바운스 → 9틱 뒤 재발동 → 다른 플레이어의 G는 독립 → 이동 박스 진입은 이동하지 않고 제안된 ID로 G가 이동 → 밖·죽은 플레이어·모르는 ID의 G는 거부 → 진입 발동 대상 박스(시퀀스)는 진입에 발동하고 G는 건드리지 않음 → 로그가 G 전용 박스에는 `Source=KEY`만**, (d) 월드×id 규칙: 이동 박스는 Kouku의 `Mario*` id만 진입에 발동, `jump.*`와 changeLevel은 세 월드 모두 G 전용.
  기존 블록: changeLevel(진입은 제안만, G가 전송), Debug Valtan 지름길(진입은 제안만, G가 도약), 나머지는 월드 설정과 `Mario*` id 부여.
- `ServerGameplayContractTests_KoukuProduct.cpp`: 단독 트리거 시스템에 Kouku 월드 설정 한 줄.
- `ServerGameplayContractTests_SpawnGroups.cpp`: `Activate_Repeat` 블록, 실제 Valtan 방의 `Stage_Boss` 시작을 Debug에서는 G(`Activate_Here`)로, Release에서는 진입으로 실행.
- `ServerGameplayContractTests_WorldPlayback.cpp`, `ServerGameplayContractTests_DebugTeleport.cpp`: 래치를 검증하던 픽스처가 옵트인(`Set_HonourTriggerOnce`)하도록 한 줄씩.

## 8. 사용자 확인 절차

1. **Server와 Client를 같은 소스로 다시 빌드하고 재시작한다.** 데이터 게시는 필요 없다(bootstrap을 바꾸지 않았다). Client만 바꾸면 서버가 옛 진입 발동이고, Server만 바꾸면 G가 안 나간다.
2. **베른에서 확인하려면 먼저 트리거가 Server에 있어야 한다.** 지금 게시본과 저작 원본 모두 베른에는 활성 트리거가 없다. MapTool에서 트리거를 `enabled`로 저장한 뒤 `Tools/Build/Invoke-BuildDomainOwner.ps1 -Owner Server`(또는 `Publish-WorldGameplay.ps1`)로 게시하고 Server를 다시 켠다. 그렇지 않으면 아무 일도 없는 것이 정상이다.
3. Server CMD에는 발동마다 이런 줄이 찍힌다.

   ```
   [Trigger] Fire Trigger=<placementId> Player=<playerId> Source=ENTER
   [Trigger] Fire Trigger=<placementId> Player=<playerId> Source=KEY
   ```

   `KEY`는 G, `ENTER`는 진입 발동(5절의 예외만)이다.
4. 확인 순서:
   - 트리거(예: 베른에 깐 이동 트리거, 또는 Valtan `player_Move.1`) 안에 아무것도 누르지 않고 선다. **이동하지 않아야 하고 `[Trigger] Fire` 줄이 없어야 하며** (`[ G ]` 화면 표시는 11절에서 제거했으므로 Bern·Valtan에서는 안내가 뜨지 않는다. 그 표시는 다른 팀원이 작업한다.)
   - G를 누른다. `Source=KEY` 줄이 찍히고 동작이 실행돼야 한다. 0.3초 안의 두 번째 G는 무시된다.
   - 볼륨 밖에서 G를 누른다. 아무 일도 없어야 한다.
   - 다른 플레이어로 같은 트리거에서 G. 각자 발동해야 한다.
   - Debug Valtan: `Stage_3` 앞에서 밟기만 하면 이동하지 않고, G를 누르면 저작 절벽 이동이 실행된다(Debug도 같다, 지름길 표에서 뺐다). `Stage_2`는 밟으면 웨이브가 발동한다(13절). Debug에서 지름길 때문에 G 전용인 것은 `Stage_Boss`뿐이다(14절).
   - 예외(개정 후): 컷신(playSequence), Valtan 복도 웨이브·보스 시작만 밟으면 발동한다. Kouku Mario 레인은 10절대로 G다.

## 9. 알려진 한계·부작용

- **`[ G ]`의 실제 위치·모양과 다른 UI와의 겹침은 확인하지 못했다.** Kouku와 같은 좌표(화면 가로 중앙, 세로 62%)를 썼다. 안내 표시는 Kouku·Bern·Valtan 레벨에만 있다. Character Select·수련장·Debug Map Editor(`Test`) 레벨에는 그리는 코드를 넣지 않았다. 현재 그곳에는 활성 트리거가 없다. **(2026-09-19 저녁: 이 표시는 제거됨, 11절)**
- 제안은 한 번에 하나만 저장된다(Client HUD 뷰모델 구조). 겹친 두 G 전용 박스에 동시에 서면 마지막 제안만 보인다. 이 경우 G는 제안된 박스 하나만 실행한다(제안 없이 `@here`가 나갈 때만 겹친 박스를 모두 실행한다).
- Valtan 복도에서는 이동 트리거마다 G가 필요해졌다(`Stage_MiniBoss`, `Stage_3`, `Stage_Boss_ArenaEntry`). 스폰·보스 시작은 진입 발동이다. 원하지 않으면 5절 한 줄 변경으로 되돌린다.
- Kouku의 playSequence 트리거는 밟을 때마다 **방 전체에** 연출이 다시 재생된다. 두 번째 플레이어가 밟아도 첫 플레이어의 화면에서 재생된다.
- Client의 Kouku 입구 마커(`Level_KakulSaydonArena.cpp:4003`, once인 playSequence 마커)는 첫 재생 뒤 사라진다. 표시만 어긋나며 바꾸지 않았다. `jump.*` 마커는 진입해도 발동하지 않는다는 점이 안내(`[ G ]`)와 다를 수 있다.
- MapTool의 "Trigger Once" 체크박스는 저장되지만 Server가 무시한다. UI 문구는 그대로다.
- `hasFired`, `Reset_SequenceActivation`, Debug의 `ALREADY_USED`는 제품 경로에서 사실상 비활성이다. 래치를 검증하는 테스트만 `Set_HonourTriggerOnce(true)`로 옵트인한다.
- Valtan `Stage_Boss`(activateEncounter)는 보스 엔티티가 방에서 사라진 뒤 다시 발동하면 다시 소환될 수 있다. 처치된 보스가 엔티티로 남는지는 확인하지 않았다.
- 컴파일은 구문 검사만 했고 실행 테스트와 게임 확인은 아직이다. 7절의 (4), (5)가 통과하기 전에는 "고쳐졌다"고 말할 수 없다.

## 10. 2026-09-19 오후 개정: 플레이어 이동 트리거는 전부 G, 발탄 표식, G 키캡

사용자 요청(원문 요지): 웨이브·컷신·"마리오로 이동"은 자동이 맞다. 플레이어가 스스로 이동하는 것(발탄 시작 지점, 마리오에서 뛰어내리기·올라가기·넘어가기)은 G를 눌러 상호작용해야 한다. 발탄 트리거 박스도 쿠크처럼 표시하고 여러 명이 각자 쓸 수 있어야 한다. G 키캡 그림이 뜨고 G를 눌러야 발동한다.

### 10.1 가정 (확정하지 못한 것)

- "마리오로 이동(입장)"은 `Mario*_Intro` 컷신 트리거와 `Update_MarioControlState`의 OBB 판정이라고 봤다. `Mario*_go` 이동 박스는 게시본에서 비활성이다. 그래서 활성 `movePlayer` 중 마리오의 것은 전부 "넘어가기·뛰어내리기·올라가기·마지막 출구"로 보고 G로 바꿨다.
- 마지막 출구(`Mario1_Trigger_5`, `Mario2_Trigger_7`, `Mario3_Trigger_12`, `Mario4_Tigger_13`)도 사용자 말의 "플레이어가 이동하는 것"으로 보고 G로 했다. 자동으로 남기려면 `AUTO_ENTRY_RULES`에 그 id만 잡는 접두사 행을 넣으면 된다.
- 데이터(`Gameplay.world.json`)는 바꾸지 않았다. 분류는 서버 코드 표 한 곳이 소유하므로 게시가 필요 없고 Server 재빌드·재시작만 필요하다.

### 10.2 분류 결과

- 자동으로 남긴 것: 모든 `playSequence`(쿠크 24개: `Mario*_Intro`, 마리오 안 컷신, `paper.1/2` 등), 카드미로 망원경 상자(진입도 G도 아닌 상자), Valtan `activateSpawnGroup` 3개(`Stage_1`, `Stage_2`, `Stage_MiniBoss_Spawn`), `Stage_Boss`(`activateEncounter`).
- G로 바꾼 것(쿠크 마리오 이동 16개): `Mario1_Trigger_1/3/5`, `Mario2_Trigger_2/4/7`, `Mario3_Trigger_4/5/6/8/10/12`, `Mario4_Tigger_5/6/7/13`. 원래 G였던 것: `jump.1~3`, `Mario4_Tigger_2/3`(`requiresInteract`), Valtan `player.spawn.editor`, `player_Move.1`, `Stage_3`, `Stage_MiniBoss`, `Stage_Boss_ArenaEntry`.
- 근거: 이 id들은 전부 `movePlayer` 이벤트이고 `MARIO_LANES`(`GameRoom_Internal.h:105`)의 lane 출구·도착이다.

### 10.3 바꾼 것

- `ServerTriggerSystem.cpp` `AUTO_ENTRY_RULES`에서 Kouku `Mario*` 이동 행 삭제. 규칙 주석 갱신.
- G 경로가 방 소유 진입 핸들러를 거치도록 `Activate_Interact`/`Activate_Here`에 `moveEntry` 인자와 `Run_KeyTrigger` 추가(`ServerTriggerSystem.h/.cpp`), `Handle_InteractTrigger`(`GameRoom_PartyWorld.cpp`)가 `Begin_MarioTriggerMove`를 넘긴다. 이유: 마지막 출구는 그 핸들러가 목적지를 복귀 위치로 바꾸므로, G가 우회하면 저작 좌표로 이동한다.
- `ClickMoveEffect.cpp`: 발탄에서도 `effect.world.move_destination`을 로딩에서 준비한다.
- `Level_ValtanArena.h/.cpp`, `MainApp.cpp`: 쿠크와 같은 표식. 처음에는 활성 트리거 9개 전부에 띄웠으나 12절에서 이동 트리거 5개로 좁혔다. 중심은 `Gameplay.world.json`에서 읽는다. 하드코딩 좌표는 없다.
- `InteractKeyPromptView.h/.cpp`: 서버가 제안한 박스 id 위에 G 키캡을 그린다. `Level_ValtanArena`가 이 뷰를 가지고, `Level_KakulSaydonArena`는 제안 id를 넘긴다. 키캡이 그려지는 동안에는 텍스트 `[ G ]`를 생략한다. **(2026-09-19 저녁: 이 표시는 제거됨, 11절)**
- 테스트(실행 안 함): `ServerGameplayContractTests_WorldTriggers.cpp`(이동 박스의 진입은 안내만, G로 발동, 두 플레이어 각자 이동, 한 명의 방 거절이 다른 사람을 막지 않음, 규칙표 갱신), `ServerGameplayContractTests_DebugTeleport.cpp`·`ServerGameplayContractTests_KoukuProduct.cpp`(마리오 진입 발동을 G로).
- 프로토콜 번호, Shared 메시지, 데이터 파일은 바꾸지 않았다.

### 10.4 여러 명이 각자 쓰는 것 (코드 추적 결과)

- 진입 안내(`PlayersInside`), 디바운스(`m_LastKeyActivationTick`), 이동 상태(`TriggerMove`)는 모두 플레이어별이다. 게시 경로에서 `hasFired` 래치는 꺼져 있다. 한 사람이 쓴 이동 박스는 다른 사람도, 같은 사람도 다시 쓴다.
- 발탄에는 트리거 시스템 밖에 이동 박스를 막는 서버 로직이 없다(`Stage_*` 검색 결과는 주석과 Debug 지름길뿐이다). 다만 Debug의 발탄 타임라인 audition 중에는 방이 플레이어 트리거 평가를 끈다(`GameRoom.cpp` `evaluatePlayerTriggers`).
- 마리오 stage·lane 조건은 각자의 `iMarioStage`로 판단하므로 첫 번째 사람 때문에 잠기지 않는다.
- **남는 한계**: 마리오 마지막 출구는 복귀 좌표에 다른 플레이어가 서 있으면(`Is_PlayerPositionClear`) 그 사람이 비킬 때까지 G가 거절된다. 기록된 복귀 위치가 없으면 모두 같은 3관문 스폰으로 돌아오므로 동시에 끝낸 여러 명이 겹칠 수 있다. 확인만 했고 바꾸지 않았다. 웨이브·보스는 진행 중/이미 있으면 거절하는 조건이 그대로다(요청대로 유지).
- 파티 전원 강제 이동이나 "한 명이 누르면 모두 이동"은 요청에 명시되지 않아 만들지 않았다.

### 10.5 검증 티어 (거짓 없이)

- 소스 직접 읽기로 확인: 규칙표, G 경로, 핸들러 호출, 플레이어별 상태, 키캡·표식 코드, 테스트 수정. (이 중 키캡 코드는 11절에서 제거)
- 구문 검사(`cl /Zs`, 산출물 없음)만 통과: 서버 `ServerTriggerSystem.cpp`, `GameRoom_PartyWorld.cpp`, `GameRoom.cpp`, 수정한 테스트 3개(Debug), 서버 Release 2개. 클라이언트 `Level_ValtanArena.cpp`, `InteractKeyPromptView.cpp`, `ClickMoveEffect.cpp`(Debug·Release), `Level_KakulSaydonArena.cpp`, `MainApp.cpp`(Debug), 이 헤더를 포함하는 다른 번역 단위 9개(Debug, 그중 `LevelRegistry.cpp`와 `MainApp_WorldLevel.cpp`는 Release도). `MainApp_SequenceViewer.cpp`는 스크립트에 `/utf-8`이 없어 실패했고 `/utf-8`을 붙이면 통과한다(수정하지 않은 파일).
- **빌드·링크 안 함, 계약 테스트 실행 안 함, Server·Client 실행 안 함, 화면 확인 안 함.** VS가 켜져 있어 빌드와 publisher는 실행하지 않았다.
- 게임에서 확인하기 전에는 "G로 이동된다", "발탄 표식이 뜬다", "키캡이 뜬다"를 단정할 수 없다. (키캡 표시는 11절에서 제거)

### 10.6 사용자가 할 일

1. VS를 끈 뒤 Product 빌드: `powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug` (Release도 쓰면 `-Configuration Release`). 데이터 게시는 필요 없다.
2. Server를 다시 켜고 Client를 다시 실행한다. Server와 Client를 같은 소스로 빌드해야 한다.
3. 확인: 쿠크 마리오에서 이동 박스 안에 서서 아무것도 누르지 않으면 이동하지 않는다(우리가 넣었던 G 키캡·안내 표시는 11절에서 제거했다). G(Debug는 ↑도)를 누르면 이동한다. 마지막 출구에서 G를 누르면 3관문 복귀 위치로 간다. 발탄은 이동 트리거 5개 위에만 표식이 뜨고(12절), 시작 지점 박스에서 G로 이동한다. 두 명이 같은 박스에서 각자 G를 눌러 각자 이동한다.

## 11. 2026-09-19 저녁: G 안내 표시(키캡·텍스트) 제거

사용자 요청: "G키 아이콘 표시는 이미 다른 팀원이 하고 있어서 내가 할 필요가 없다. 우리가 추가한 표시를 지워라." G키 입력과 서버 동작은 유지하고 화면에 그리는 표시만 뺐다.

### 11.1 제거한 것

- `InteractKeyPromptView.h/.cpp`: 서버가 제안한 박스 id 위에 키캡을 그리던 변경(`strOfferedId` 인자, `Is_Showing()`, `bGated`, `m_bShowing`) 전부. 이 두 파일의 미커밋 변경은 전부 우리 추가분이어서 HEAD 내용으로 되돌렸다. 원래(커밋 7f224e07) 있던 `requiresInteract` 상자용 키캡은 그대로다.
- `Level_KakulSaydonArena.cpp`: 제안 id를 키캡 뷰에 넘기던 인자와, 키캡이 그려지면 텍스트 `[ G ]`/`[ Up ]`을 생략하던 조건(`!Is_Showing()`). 쿠크의 원래 동작으로 복원했다. 이 파일의 미커밋 변경은 이 두 hunk뿐이어서 HEAD와 diff가 없다.
- `Level_Bern.cpp`: 이전 fork가 넣은 텍스트 `[ G ]` 그리기 블록. HEAD와 diff가 없다.
- `Level_ValtanArena.cpp/.h`: 키캡 뷰 멤버·`Initialize`·`Update`·`#include "InteractKeyPromptView.h"`와 텍스트 `[ G ]` 블록.

### 11.2 일부러 남긴 것

- G 입력 경로(`PlayerController.cpp`의 `Submit_InteractIfOffered`, `@here`)와 프로토콜 94, 서버 쪽 전부(자동/G 분류, `Activate_Interact/Activate_Here/Run_KeyTrigger`, 안내 발신, 테스트). 이번 요청은 서버를 바꾸지 않는다.
- 서버 안내를 저장하는 `CCombatHUDViewModel::Get_InteractPromptTriggerId`. 지금 이를 그리는 곳은 쿠크 레벨의 기존 표시뿐이며, 다른 팀원의 표시 작업이 이 접근자를 쓰면 된다.
- 쿠크의 기존 키캡·`[ G ]`·`[ Up ]` 표시.
- 발탄 트리거 표식(`Load_TriggerMarkers`, `Submit_TriggerMarkers`, `MainApp.cpp` 호출, `ClickMoveEffect.cpp`)과 스퀘어홀·몬스터 변경.

### 11.3 확인 단계

- 소스 diff로 확인: 위 4개 파일이 HEAD와 diff 0, 발탄 diff에서 키캡·`[ G ]` 줄 없음, `PlayerController.cpp`·`MainApp.cpp`·`ClickMoveEffect.cpp` diff는 수정 전과 동일, 서버·Shared는 이번 작업에서 건드리지 않음.
- 구문 검사(`cl /Zs`, 산출물 없음)만 통과: 수정한 `InteractKeyPromptView.cpp`, `Level_KakulSaydonArena.cpp`, `Level_Bern.cpp`, `Level_ValtanArena.cpp`와 이 헤더를 포함하는 `LevelRegistry.cpp`, `MainApp.cpp`, `MainApp_WorldLevel.cpp`, `MainApp_RenderingLighting.cpp`, `MainApp_SequenceViewer.cpp`, `CharacterPreviewPanel.cpp`, `Effect_Tool_V2.cpp`, `MapTool_Area.cpp`, `ValtanBossTool.cpp`, `Animation_Tool_CompositionSounds.cpp`. Release 14개 전부 exit=0. Debug는 13개 exit=0이고 `MainApp_SequenceViewer.cpp`만 검사 스크립트에 `/utf-8`이 없어 exit=2였으며(수정하지 않은 파일, 이전 절과 같은 현상) `/utf-8`을 붙이면 exit=0이다.
- 빌드하지 않았다. 게임에서 확인하지 않았다. 반영하려면 Client를 다시 빌드해야 하며 Server는 바뀌지 않았다.

## 12. 2026-09-19 밤: 발탄 표식은 출발 이동 트리거에만

사용자 요청(원문 요지): 표식은 `Player_Move.1` 같은 출발 이동 트리거 위에만 뜨는 것이 맞다. 미니보스로 들어가는 쪽의 도착하는 곳에도 표식이 뜨는데 왜 그렇게 했는가. 표식이 필요한 것은 이동 트리거 위뿐이다. 스크린샷 3장(진단 입력일 뿐 최종 판정이 아니다)에서는 구덩이 양쪽, 곧 건너기 전과 건넌 뒤에 표식이 하나씩 보인다.

### 12.1 데이터로 확정한 분류 (`Data/Worlds/LV_LUT_HEARTRB_ED/Gameplay.world.json`, 활성 triggerBox 9개)

- 표식 유지(이동 트리거 5개, 좌표는 x, y, z, 단위 m):
  - `player.spawn.editor`: (28.11, 10.24, -28.87) -> (28.96, 10.35, -35.94), 수평 7.13m.
  - `player_Move.1`: (30.68, 10.04, -28.02) -> (31.53, 10.34, -37.15), 수평 9.18m.
  - `Stage_MiniBoss`: (49.15, 10.06, -64.61) -> (50.87, 10.14, -81.02), 수평 16.50m.
  - `Stage_3`: (97.76, 15.51, -87.99) -> (100.30, 20.55, -87.27), 수평 2.64m, 위로 5.04m.
  - `Stage_Boss_ArenaEntry`: (139.75, 25.73, -112.75) -> (147.75, 23.02, -117.25), 수평 9.18m.
- 표식 제외(자동 트리거 4개, 이동이 아님): `Stage_1`, `Stage_2`, `Stage_MiniBoss_Spawn`(activateSpawnGroup 3개)과 `Stage_Boss`(activateEncounter).
- 도착 쪽 박스의 근거: `Stage_MiniBoss_Spawn`의 위치 (50.87, 10.14, -81.02)가 `Stage_MiniBoss`의 목적지와 정확히 같다(거리 0.00m, 목적지가 박스 안). 이것이 미니보스 도착점에 표식이 뜬 원인이다. 나머지 이동 트리거의 목적지 8m 안에 다른 박스가 있는 경우는 `player.spawn.editor`(`Stage_1`까지 4.09m)와 `player_Move.1`(`Stage_1`까지 2.83m)뿐이며 둘 다 목적지가 그 박스 밖이다. 서버 복도 주석(다음 authored 트리거 직전에 멈춘다)과 일치한다.

### 12.2 바꾼 것

- `Level_ValtanArena.cpp`의 `Load_TriggerMarkers`: 이벤트가 정확히 하나이고 `WORLD_TRIGGER_EVENT_KIND::MOVE_PLAYER`인 박스만 표식으로 고른다. 삽입은 한 블록(475바이트)이고 나머지 바이트는 그대로다.
- 명시 id 목록 대신 종류 규칙을 택했다. 요청이 "이동 트리거 위에만"이라 종류가 기준이고, MapTool에서 이동 트리거를 추가해도 목록을 따로 고치지 않아도 되기 때문이다. 대신 도착 쪽에 이동 트리거를 두면 그곳에도 표식이 뜬다(`gotchas.md`에 남겼다).
- 헤더, `MainApp.cpp`, `ClickMoveEffect.cpp`, 서버, Shared, 데이터, 쿠크 표식은 바꾸지 않았다. 표식 개수 상수나 배열 크기는 원래 없었다(벡터).
- 문서: 이 문서의 10.3, 10.6에 있던 "9개 전부" 서술과 `gotchas.md`에 한 줄.

### 12.3 가정과 확정하지 못한 것

- `player.spawn.editor`는 이동 트리거이고 시작 위치에 있는 출발 트리거라 표식을 남겼다. 이름의 `editor` 때문에 사용자가 표식을 원하지 않을 가능성은 데이터로 판단할 수 없다.
- `changeLevel` 트리거는 지금 발탄에 없어서 규칙에 넣지 않았다. 생기면 표식이 뜨지 않는다.
- 스크린샷의 구덩이 양쪽 표식이 어느 트리거 쌍인지는 카메라 축척을 몰라 거리로 확정하지 못했다. 미니보스 쪽은 위의 좌표 일치로 확정했고, 다른 쌍이 있는지는 사용자가 화면에서 확인해야 한다.

### 12.4 검증 티어 (거짓 없이)

- 데이터로 확인: 활성 트리거 9개의 종류·좌표·목적지, 규칙 시뮬레이션 결과(표식 5개, 제외 4개).
- 소스 diff로 확인: `Level_ValtanArena.cpp`의 변경이 삽입 블록 하나뿐이고 CRLF와 한글 바이트 수는 그대로다.
- 구문 검사(`cl /Zs`, 산출물 없음)만 통과: `Level_ValtanArena.cpp` Debug와 Release.
- **빌드·링크 안 함, Client 실행 안 함, 화면 확인 안 함.** VS(devenv)가 켜져 있어 빌드는 하지 않았다. 표식이 실제로 어디에 뜨는지는 사용자가 화면에서 확인한다.

### 12.5 반영하려면

- Client를 다시 빌드한다(서버·데이터는 바뀌지 않았다). 그다음 발탄에 다시 들어가서 이동 트리거 5개 위에만 표식이 있는지, 미니보스 도착점과 웨이브·보스 박스에는 없는지 본다.

## 13. 2026-09-19 밤: 발탄 `Stage_2`가 웨이브 대신 앞으로 이동하던 것 (Debug 지름길)

### 13.1 원인 (소스와 게시본으로 확인)

- `Data/Worlds/LV_LUT_HEARTRB_ED/Gameplay.world.json`과 게시된 `VALTAN_ARENA.worldbootstrap`의 `Stage_2`는 `activateSpawnGroup`(`spawn.valtan.stage03`, 위치 [89.28, 14.63, -95.47], 반크기 [2.4, 1, 0.5])다. 웨이브가 맞다. `SpawnGroups`의 그 그룹은 선행 그룹 조건이 없고 2웨이브다.
- `GameRoom.cpp:105-107`이 `VALTAN_ARENA` 방이면 `Initialize`의 세 번째 인자(Debug 지름길)를 켠다. `_DEBUG`가 아닌 빌드에서는 `ServerTriggerSystem.cpp`가 이 인자를 무시하고 끈다. 즉 Debug 서버의 발탄 방은 별도 옵션 없이 항상 지름길이 켜진다.
- 지름길 표 `DESTINATIONS`(`ServerTriggerSystem.cpp`)에 `Stage_2`가 있었다. 그러면 `Fires_OnEntry`가 진입 발동을 막고(밟기만 하면 제안만) `Run_Trigger`가 웨이브 대신 (94.762, 15.511, -90.633)으로 0.55초 도약시켰다. 이 좌표는 다음 트리거 `Stage_3` 상자([97.76, 15.51, -87.99]) 바로 앞이다. 사용자가 본 "`player_Move.1`처럼 앞으로 나간다"와 같은 경로다.
- Client에는 `Stage_2`를 이동으로 처리하는 경로가 없다(`Client/Private`, `Client/Public` 검색 결과 없음).
- 기존 테스트가 이 동작을 "의도된 계약"으로 단언하고 있었다(Stage_2를 지름길 대표로 사용). 그래서 이 동작은 실수로 생긴 것이 아니라 Debug 지름길이 웨이브까지 덮은 것이다.

### 13.2 바꾼 것

- `ServerTriggerSystem.cpp`: 지름길 표에서 `Stage_2` 행을 뺐다. `Stage_1`·`Stage_MiniBoss`와 같은 방식이다. 이제 지름길은 `Stage_3`(이동)과 `Stage_Boss`(보스 시작 뒤 유도 지점)뿐이다. `Stage_2`는 `AUTO_ENTRY_RULES`의 발탄 스폰 그룹 행에 따라 Debug에서도 밟으면 웨이브가 발동한다. Release 경로는 바뀌지 않았다.
- `ServerGameplayContractTests_WorldTriggers.cpp`: 지름길 대표를 `Stage_2`에서 `Stage_3`(이동 트리거)로 옮기고 목적지 단언을 (126.450, -94.750)으로 고쳤다. Release 쪽 단언은 "진입은 제안만, G가 저작 이동 실행"으로 바꿨다. `Stage_2` 웨이브가 Debug 지름길이 켜져도 본래 동작으로 실행되는 단언 블록을 추가했다. 테스트는 실행하지 않았다.
- 데이터 파일, Client, Shared, 프로토콜(94)은 바꾸지 않았다.

### 13.3 남는 위험과 확정하지 못한 것

- `Stage_2`의 지름길은 `Stage_3` 앞까지 약 7m를 건너뛰던 것이라, 빼도 진행 경로는 짧은 걷기가 늘 뿐이다. `Stage_3`·`Stage_Boss`는 웨이브 완료를 조건으로 하지 않는다(`Activate_Encounter`는 보스 entity 존재 여부만 본다). 따라서 웨이브를 못 잡아도 소스상 진행이 막히는 조건은 찾지 못했다.
- 웨이브 몬스터(`spawn.valtan.stage03`, 1웨이브 8마리·2웨이브 9마리)는 앵커 [87, 15.8, -80.1], [98.4, 16.0, -81.3], [92.3, 16.0, -81.5]에 나온다. `Stage_3` 상자([97.76, 15.51, -87.99], 반크기 Z 3.8) 가장자리에서 약 3~11m 거리라 전투 중에 `Stage_3`을 밟게 될 수 있다. 피격으로 행동이 잠기면 G 도약이 그동안 거절될 수 있다(`Begin_MovePlayer`는 행동이 `NONE`일 때만 시작). 영구히 막히는지는 확인하지 못했다.
- 원래 주석의 "안 죽는 audition 몬스터"에 해당하는 일반 몬스터 무적 처리는 서버 소스에서 찾지 못했다(무적 플래그는 보스 전용). Debug에서 웨이브 몬스터가 실제로 죽는지는 확인하지 못했다.
- `Stage_2`와 `Stage_3` 사이가 걸어서 이어지는지(네비 격자)는 확인하지 않았다. 정적 충돌 박스는 그 구간 14m 안에 없다.
- 방이 비어 초기화되면(`Reset_ReplayableArenaWhenEmpty`, `GameRoom_WorldEntities.cpp:348`) 트리거 시스템을 지름길 인자 없이 다시 초기화해 지름길이 꺼진다. 서버를 처음 켠 뒤 첫 세션에서만 `Stage_3`·`Stage_Boss` 지름길이 있고, 모두 나갔다 다시 들어오면 없다. 이번 요청과 별개의 기존 동작이라 바꾸지 않았다.

### 13.4 검증 티어 (거짓 없이)

- 데이터·게시본으로 확인: `Stage_2`의 종류·좌표·대상 그룹, `spawn.valtan.stage03`의 웨이브 구성.
- 소스로 직접 확인: 지름길 켜짐 조건(`GameRoom.cpp:105-107`), 표 내용, `Fires_OnEntry`·`Run_Trigger` 경로, Client에 별도 이동 경로가 없음.
- 구문 검사만: 아래 절차의 `cl /Zs` 결과(수정한 Server 파일 Debug·Release).
- **빌드·링크 안 함, 테스트 실행 안 함, 서버·Client 실행 안 함, 화면 확인 안 함.** 지금 사용자가 Server와 Client를 켜 둔 상태라 빌드하지 않았다. 웨이브가 실제로 나오는지는 사용자가 화면에서 확인한다.

### 13.5 반영하려면

- Server를 다시 빌드하고 재시작한다(Client와 데이터는 바뀌지 않았다). 재시작 뒤 발탄에서 `Stage_2` 상자에 들어가면 G 없이 웨이브가 나와야 한다. 나오지 않으면 서버 로그의 `[Trigger] Fire Trigger=Stage_2 ... Source=ENTER` 줄 유무를 먼저 본다.


## 14. 이후 변경 정리 (2026-09-19 밤, 위 서술이 바뀐 부분)

- 지름길 표(`Build_ValtanStageBypassMove`의 `DESTINATIONS`)는 이제 `Stage_Boss` 한 행뿐이다. `Stage_2`는 13절에서, `Stage_3`은 `2026-09-19_VALTAN_STAGE3_MOVE_RESULT.md`에서 뺐다.
- 13절이 "`Stage_3`(이동)과 `Stage_Boss`가 남았다", "`Stage_3` 도약 단언 (126.450, -94.750)"이라고 쓴 것은 그 시점의 상태다. 현재 테스트는 `Stage_3`이 Debug에서도 저작 이동 (100.42, 20.53, -86.95)을 실행한다고 단언한다.
- `Stage_Boss`는 보스 시작 뒤 플레이어를 `Stage_Boss_ArenaEntry` 박스 중심으로 보낸다. 옛 Debug 하드코딩 유도 지점 이동은 `Run_Trigger`에서 뺐다. 상세는 `2026-09-19_VALTAN_STAGE_BOSS_ENTRY_MOVE_RESULT.md`.
- G 키캡·텍스트 안내 표시는 11절대로 제거된 상태다.
