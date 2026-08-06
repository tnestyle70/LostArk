# MapTool Bern Spawn / Change Level 구현 결과

## 1. 완료 상태

| 항목 | 상태 | 실제 결과 |
|---|---|---|
| MapTool option 정리 | 완료 | Player Spawn, NPC authoring, Boss, Trigger Box를 UI에 표시 |
| Bern editor camera | 완료 | `player.spawn.bern.entry`를 focus, radius 35로 framing |
| surface picking | 완료 | depth-tested `Target_PickPos`만 사용하고 Y=0 plane fallback 제거 |
| player spawn 저장 | 완료 | position/yaw만 저장하며 archetype/encounter를 null로 정규화 |
| Server-authority Bern spawn | 완료 | Bern World JSON → publisher → bootstrap → GameRoom spawn 계약 확인 |
| Trigger Box 편집 | 완료 | position/yaw/half extents/once와 None/Move Player/Change Level action 선택 |
| Bern→Valtan changeLevel | 구현 완료 | Server OBB enter → source leave → target enter → S2C_ENTER_ACCEPTED → Client typed transition |
| NPC authoring 저장 | 완료 | NPC kind/archetype/position/yaw/enabled 배치·저장 유지 |
| NPC 제품 presentation | 미완료 | NpcCatalog 0 rows, Client replication이 Boss presentation만 지원 |
| Monster spawn trigger | 미완료 | Monster catalog/profile/brain/replication/presentation/spawn group 수직 슬라이스가 없어 placeholder를 추가하지 않음 |
| 수동 제품 smoke | 사용자 검증 대기 | MapTool camera/pick/save와 실제 Bern→Valtan 플레이 확인 필요 |

## 2. 정본 데이터

- Bern player entry: `player.spawn.bern.entry`, `(144.8, 42.7, -70.3)`, yaw `180`.
- Bern transfer trigger: `trigger.bern.to-valtan`, `(144.8, 42.7, -60.3)`, half extents `(2,2,2)`, once, target `VALTAN_ARENA`.
- 네 World gameplay 문서는 formatVersion 3으로 승격했다.
- generated Server world bootstrap은 version 4이며 action을 `type payloadCount payload...` 형식으로 저장한다.

## 3. 런타임 연결

```text
Gameplay.world.json v3
-> Publish-WorldGameplay.ps1
-> LOSTARK_WORLD_BOOTSTRAP v4
-> CWorldBootstrap strict stage/commit
-> CServerTriggerSystem OBB enter edge
-> CGameRoom LEVEL_CHANGED leave
-> CServerApp target room REGISTER/ENTER
-> S2C_ENTER_ACCEPTED + target player spawn/snapshot
-> CLevelTransitionService
-> LEVEL::VALTAN_ARENA load
```

Client는 trigger overlap으로 직접 `Change_Level`을 호출하지 않는다. Server가 target room 입장을 승인한 뒤 기존 accepted message를 소비할 때만 level transition request를 제출한다.

## 4. MapTool 동작

- Area switch camera는 preferred playerSpawn, 첫 enabled playerSpawn, map bounds 순서로 fallback한다.
- World gameplay placement와 movePlayer target pick은 모두 첫 visible rendered triangle surface만 허용한다.
- Player Spawn/NPC/Boss는 position과 yaw를 inspector에서 편집한다.
- Trigger Box의 scale은 Server OBB 계약과 같은 half extents다.
- Trigger action은 정확히 하나일 때만 enabled로 저장할 수 있다.
- NPC 버튼은 authoring 저장까지 사용할 수 있으나 제품 publish/runtime 완료 표시는 하지 않는다.

## 5. 자동 검증

| 검증 | 결과 |
|---|---|
| 네 Gameplay JSON parse | PASS |
| `Publish-WorldGameplay.ps1 -Mode Validate` | PASS, Bern 5 / Valtan 5 / Training 4 / Character Select 5 |
| publisher failure injection after promote 2 | PASS, failure rejected and four previous files preserved |
| Server x64 Debug build | PASS, 0 errors |
| `Server/Bin/Debug/Server.exe --contract-test` | PASS, failures 0 |
| Debug Server 250 ms smoke | PASS, four rooms ready/listening |
| Client x64 Debug build | PASS, 0 errors |
| Server x64 Release build | PASS, 0 errors |
| `Server/Bin/Release/Server.exe --contract-test` | PASS, failures 0 |
| Client x64 Release build | PASS, 0 errors |
| ProjectAudit | PASS, 69 checks |
| `git diff --check` | PASS |

Client build에는 기존 C4819 인코딩 경고와 DirectXTK PDB LNK4099 경고가 남지만 오류는 0이다. MapTool H/CPP는 기존 CP949 인코딩으로 복원해 유지했다.

## 6. 사용자 수동 검증

1. Debug Server를 실행한다.
2. Client의 F1 Developer Tools에서 Map Tool을 열고 Bern Area를 선택한다.
3. 카메라가 Bern entry 주변에서 시작하는지 확인한다.
4. Player Spawn 또는 Trigger Box를 arm하고 map geometry가 보이는 지점을 클릭한다. 빈 배경 클릭이 Y=0에 배치되지 않는지 확인한다.
5. Trigger Box 선택 후 Action=`Change Level`, Target World=`VALTAN_ARENA`, Enabled를 설정하고 저장한다.
6. publish/Server 재시작 후 Bern에서 `trigger.bern.to-valtan` 위치로 이동해 Valtan Arena로 전환되고 Valtan player spawn에서 생성되는지 확인한다.

## 7. 남은 경계

- NPC 제품 배치를 완료하려면 실제 NpcCatalog row, per-level model admission, `CNpc` replication presentation, NPC snapshot 처리와 smoke를 같은 변경 단위에서 구현해야 한다.
- Monster spawn은 MonsterCatalog/Profile, SpawnGroups authoring, Server brain/entity authority, Shared replication, Client presentation이 승인될 때 typed trigger action으로 확장한다.
- 실제 화면과 네트워크를 함께 실행하는 수동 Bern→Valtan smoke는 사용자가 수행한다.
