# 2026-09-06 KoukuSaydon 관문 1·3 거부 수정, 뿅망치 배율·회전, 광기 수치 확장, 광대 아바타 교체 구현 계획서

> 문서 종류: 구현 계획서 (버그 수정 + 튜닝 + 수치 확장 + Debug 아바타 교체)
>
> 상태: G01~G04 구현·자동 검증 완료 / PR325 최신 main 통합 완료 / G05(Logic 런타임) 후속, 사용자 화면 확인 전
>
> 기준 브랜치: `codex/kouku-scale-1p7` (관문 스폰·HUD·G05 재생 위에 얹는다)
>
> 선행 문서: [Arena 관문 스폰 계획](../09-05/2026-09-05_KOUKU_SAYDON_ARENA_GATE_SPAWN_AND_BOSS_HUD_IMPLEMENTATION_PLAN.md), [광기 게이지 계획(보스 변신 가정, 이 문서가 대체)](../09-05/2026-09-05_KOUKU_SAYDON_MADNESS_GAUGE_AND_COLORLESS_FORM_IMPLEMENTATION_PLAN.md), [무력화 반사 계획](../09-05/2026-09-05_KOUKU_SAYDON_SHIELD_STAGGER_AND_COMMON_GROGGY_PATTERN_IMPLEMENTATION_PLAN.md)

## 0. 목표와 종료 증거

1. **관문 1·3 버튼이 성공한다.** Server 보스 spawn이 저작 좌표를 4m navgrid 셀 중심으로 옮기지 않는다.
2. **뿅망치가 맞는 크기로 보인다.** 빙고 세이튼 `weaponModelPreScale 0.0001`, 대형 세이튼도 뿅망치를 들고 `0.001`. F1 튜닝 슬라이스에 대형 세이튼 yaw 행이 생기고 Save가 `yawDegrees`를 쓴다.
3. **광기 게이지 수치가 플레이어에 존재한다.** `SERVER_PLAYER`·`PLAYER_SNAPSHOT`(protocol 59)·`CCombatHUDViewModel`이 `current/maximum`을 나른다. PR325 통합에서 main의 기존 HUD 바에 수치를 연결했다. 증가·자동 변신·모드별 스킬 구현은 후속이다.
4. **F1 `Change to Clown` / `Return to Player`가 플레이어 몸체를 무채색 세이튼(MN_RPCT_03)으로 바꾸고 되돌린다.** 바뀐 몸체로 idle/run이 재생되고 우클릭 이동이 그대로 된다. 교체는 Server가 소유한 `eMadnessForm`을 snapshot으로 받아 같은 entity presentation을 transactionally 교체한다(class 변경과 같은 경로).
5. **G05 결정값을 고정한다.** 진짜 쿠크 찾기 = 플레이어 시야각 45°, 방패 반사 = 보스 forward 45° cone 안 공격은 공격자에게 반사, 댄스타임 timeout = 최대 HP 퍼센트 피해(50000의 10% = 5000). 구현은 Logic 런타임 슬라이스로 후속.

종료 증거: Debug Product 빌드 PASS, `NetworkProtocolHarness` failures 0, `Server.exe --contract-test` failures 0(관문 1 spawn 정확 좌표 + 플레이어 좌표 충돌 없음 검사 추가), python 테스트 OK, balance/world publisher 성공, `git diff --check` PASS. 화면 확인(관문 1·3 이동, 뿅망치 크기, 광대 몸체와 이동)은 사용자가 한다.

## 1. 현재 실측

### 1.1 관문 1·3 거부 원인 (재현 계산 완료)

| 항목 | 값 |
|---|---|
| Client 메시지 | `Server rejected placement: player would overlap a blocker.` = `DEBUG_TELEPORT_RESULT::REJECTED_COLLISION` (`PlayerController.cpp:838`) |
| Server 판정 | `Apply_DebugTeleportToPosition` → `Refresh_PlayerBlockingBodies()` → `CServerCollisionSystem::Is_PlayerPositionClear` (`GameRoom.cpp:3357`) |
| 차단 body | BOSS는 profile `collisionRadius`(세이튼 1.0). 허용 거리 = 0.45(플레이어) + 1.0 + 0.001 = **1.451m** |
| 보스 spawn 좌표 | `Build_WorldEntity`가 `m_ServerNavigation.Project_Point` 결과로 **덮어쓴다** (`GameRoom.cpp:10079~10091`). `Project_Point`는 `Cell_ToPoint` = 셀 중심 |
| `LV_LUT_MIDNIGHTC_ED.navgrid` | 520×720, **cellSize 4.0m**, origin (-1944, -1724) (다른 Area는 0.5~1.0m) |
| 관문 1·3 | 세이튼 (-0.07, 942.33) → 셀 중심 **(-2.0, 1.32, 942.0)**. 플레이어 (-2.84, 941.02)와 **1.29m** < 1.451 → 거부 |
| 빙고 | 세이튼 (-0.6, 1147.44) → (-2.0, 1146.0). 플레이어 (-3.40, 1147.44)와 2.01m → 통과 |
| 2관문 | 대형 (10.74, 316.45) → (10.0, 318.0) 8.88m, 쿠크 (6.36, 321.29) → (6.0, 322.0) 3.25m → 통과 |

즉 보스가 저작 위치에서 최대 2.8m 밀리고, 관문 1·3만 플레이어 좌표와 겹친다. collisionBox·SpawnGroup·Summon entity는 이 world에 없다(placements 32 = triggerBox 17, playerSpawn 9, boss 6).

### 1.2 뿅망치·대형 세이튼 현재 데이터

| row | bodyModel | bodyModelPreScale | weaponModel | weaponModelPreScale |
|---|---|---|---|---|
| `BOSS_KAKULSAYDON_BINGO_SAYDON` | MN_RPCT_05 | 0.017 | `WP_MN_RPCT_06` | **0.01** (사용자: 100배 큼) |
| `BOSS_KAKULSAYDON_G2_BIG_SAYDON` | MN_RPCT_06 | 0.1 | **null** (뿅망치 없음 → 안 보이는 이유) | null |

무기 행렬 = `Scaling(debug) × BoneMatrix(b_wp_1) × world` (`Npc.cpp:378~391`), weapon preScale은 `CModel::Create` admission transform에 굽는다 (`KoukuSaydonPresentationAssetService.cpp:396~399`). 튜닝 슬라이스(`MainApp.cpp:6608~6800`)는 대형 세이튼 scale/offset, 빙고 hammer scale만 다루며 `yawDegrees`는 없다. world placement 저작값은 `yawDegrees`(대형 세이튼 315).

### 1.3 플레이어 수치·아바타 경로 (재사용)

| 계약 | 위치 |
|---|---|
| `SERVER_PLAYER::iCurrentIdentity/iMaximumIdentity` | `ServerPlayer.h:185~186`; 리셋 `GameRoom.cpp:695, 2269, 3048`; snapshot `9672~9673` |
| `PLAYER_SNAPSHOT` identity 필드·검증 | `PacketMessages.h:717~718`, `PacketMessages.cpp:118`(validator), `2342~2343`(write), `2540~2541`(read) |
| protocol 버전 | `PacketType.h:39` = 58 |
| Debug typed 명령 형태 | `C2S_DEBUG_TELEPORT_TO_POSITION{iRequestSequence, eWorldId, xyz}` → `RoomCommand` → `Handle_DebugTeleportToPosition` → `S2C_..._RESULT{iRequestSequence, eWorldId, eResult, xyz}`; Client `CNetworkManager::Send_/Try_Consume_`, `IPlayerCommandSink`, `CPlayerController` |
| 플레이어 presentation | `CClientReplication::Create_Character(class, nick, pos, yaw, local, out)` → `CCharacterCatalog::Find_Spec(class)` → `CCharacter` (`ClientReplication.cpp:1937~`) |
| class 변경 교체 transaction | `Replace_CharacterClass(snapshot)` (`ClientReplication.cpp:3733~3810`): 새 CCharacter stage → registry Replace → 옛 객체 제거, 실패 시 복원 |
| 교체 edge | `record->eCharacterClass != player.eCharacterClass` (`ClientReplication.cpp:3165`) |
| `CHARACTER_SPEC` | 정적 상수(`Logic_<Class>.cpp`), `AnimationClips[IDLE, RUN, HIT, DEAD, …]`, 무기/장비 목록, `pCreateLogic`; skill binding 문서 누락은 격리(`Character.cpp:139~146`) |
| 퀵슬롯 class | `CPlayerController`가 `pSpec->eCharacterClass`로 skill을 찾는다 (`PlayerController.cpp:568`) |
| MN_RPCT_03 | 165본, `rpct00_*` 249 clip. `rpct00_idle_battle_1`, `rpct00_run_battle_1`, `rpct00_walk_normal_1` 존재. Kouku body admission = `Scaling(0.017)`만; CNpc·CCharacter 모두 `Rotation(0, yaw, 0)` |

### 1.4 Logic 박스 현재 상태 (G05 근거)

- 저작 문서 `KoukuSaydonComposition.json`(revision 40)의 Logic 정의는 `logicId/displayName/logicType(DURATION|TRIGGER|RESULT)`뿐이며 판정 값 필드가 없다 (`KoukuSaydonCompositionDocument.h:49~57`). 댄스타임(`KAKULSAYDON_G1_PATTERN_6`)은 DURATION 4개의 timeout이 `_최대체력10%/20%/50%감소` RESULT 이름을 가리킬 뿐이다.
- projector는 Logic을 Product에 투영하지 않고(`KoukuSaydonEncounter.json`에 `logic` 0건), publisher·Server에 LOGIC 행이 없다.
- 무력화 반사 계획서는 stage optional key 방식(G02 brain 분기)이며 아직 구현 전이다. 두 설계를 합치는 Logic 런타임은 별도 슬라이스다.

## 2. 요구사항 ↔ 반영

| # | 사용자 요구 | 반영 | G |
|---|---|---|---|
| R1 | 관문 1·3 `player would overlap a blocker` 수정 | `Build_WorldEntity`: placement XZ가 `Is_PointWalkableExact`이면 그 XZ와 `Sample_Position` 높이를 유지, 아니면 기존 `Project_Point`. 계약 테스트로 (-0.07, 942.33) 유지와 플레이어 좌표 clear 확인 | G01 |
| R2 | 빙고 뿅망치 100배 축소 | `BINGO_SAYDON.weaponModelPreScale 0.01 → 0.0001` | G02 |
| R3 | 대형 세이튼 뿅망치 = 빙고 새 값의 10배 | `G2_BIG_SAYDON.weaponModel = WP_MN_RPCT_06`, `weaponModelPreScale 0.001` (body 0.1이라 유효 배율은 빙고보다 크다. 슬라이스로 조정) | G02 |
| R4 | 대형 세이튼·쿠크·세이튼 rotation 행 + Save | 슬라이스가 아레나 boss placement 5개(`KOUKU_TUNE_BOSSES`) 각각에 `yaw offset` 행을 두고 live `CNpc::Set_DebugPresentationYawOffset`, Save가 각 placement `yawDegrees`를 patch | G02 |
| R4-b | 뿅망치 rotation 조절·저장 | `BossCatalog.json` v8 `weaponModelPreRotationDegrees [pitch, yaw, roll]`(무기 row 필수, 무기 없는 row null). Client `CActorCatalog` 16속성, Kouku/Valtan 무기 admission이 회전을 굽고, `CNpc::Set_DebugWeaponRotationOffset`로 live 조절, Save가 배열을 patch. v7을 고정하던 소비자(publisher, `valtan_presentation_generation.py`, `valtan_native_animation_inventory.py`, `validate_effect_v2.py`, python 테스트 4개) 전부 v8 | G02 |
| R4-c | 대형 세이튼 preview를 저장된 scale로 | `Animation_Tool.cpp`의 "대형" 100배 상수 대신 catalog `bodyModelPreScale / MN_RPCT_06 preview 0.017`(현재 5.88배), preview 뿅망치 admission도 catalog 배율·회전 | G02 |
| R5 | 광기 게이지 수치 확장, 상수 0/10000 | `iCurrentMadness/iMaximumMadness` Server·snapshot(v59)·HUD ViewModel. 최대치는 `SERVER_PLAYER::MADNESS_GAUGE_MAXIMUM = 10000` 상수, 현재 0. 기존 HUD 바 연결 완료, 증가 로직은 후속 | G03 |
| R6 | F1 `Change to Clown` / `Return to Player` | Debug typed 명령 → Server `eMadnessForm` → snapshot → Client가 `Spec_KoukuSaydonClown`(MN_RPCT_03, idle/run)으로 같은 entity 교체. class·스킬 슬롯은 유지 | G04 |
| R7 | 진짜 쿠크 찾기 45° 시야각, 방패 반사 45° cone, 댄스타임 최대HP% 피해 | Logic 런타임 슬라이스 결정값으로 고정(§7). 구현 후속 | G05 |

## 3. 변경할 파일

새 C++ 파일은 없다(광대 spec은 `CharacterCatalog.cpp` 파일 정적 상수). `.vcxproj` 변경 없음.

| G | 파일 | 역할 |
|---|---|---|
| G01 | `Server/Private/GameRoom.cpp` | `Build_WorldEntity` 보스 위치: exact-walkable 유지 |
| G01 | `Server/Private/ServerGameplayContractTests.cpp` | 관문 1 세이튼 정확 좌표 + `Is_PlayerPositionClear(-2.84,1.32,941.02)` 검사 |
| G02 | `Data/Actors/BossCatalog.json` | 빙고 0.0001, 대형 weapon 추가 0.001 |
| G02 | `Tools/KoukuSaydonPipeline/test_kouku_saydon_runtime_inputs.py` | weaponless 집합 5→4, 대형 뿅망치 기대값 |
| G02 | `Client/Public/Npc.h`, `Client/Private/Npc.cpp` | `Set_DebugPresentationYawOffset`, 두 transform 적용점에 yaw offset |
| G02 | `Client/Public/MainApp.h`, `Client/Private/MainApp.cpp` | yaw 행·Save `yawDegrees` patch·대형 뿅망치 슬라이더 |
| G03 | `Shared/Public/Network/PacketType.h` | v59, `C2S_DEBUG_SET_MADNESS_FORM`, `S2C_DEBUG_SET_MADNESS_FORM_RESULT` |
| G03 | `Shared/Public/Network/PacketMessages.h`, `Shared/Private/Network/PacketMessages.cpp` | `PLAYER_MADNESS_FORM`, snapshot 3필드, 명령/결과 write/read/validate |
| G03 | `Tools/NetworkProtocolHarness/Private/NetworkProtocolHarness.cpp` | snapshot round-trip·invalid, 명령/결과 round-trip |
| G03 | `Server/Public/ServerPlayer.h`, `Server/Private/GameRoom.cpp` | 필드, 리셋 3곳, snapshot |
| G03 | `Client/Public/CombatHUDViewModel.h`, `Client/Private/CombatHUDViewModel.cpp` | `HUD_PLAYER_STATE` 필드 복사 |
| G04 | `Server/Public/RoomCommand.h`, `Server/Private/ServerApp.cpp`, `Server/Public/GameRoom.h`, `Server/Private/GameRoom.cpp` | 명령 라우팅, `Handle_DebugSetMadnessForm`(Debug 전용) |
| G04 | `Client/Public/NetworkManager.h`, `Client/Private/NetworkManager.cpp` | send/consume + 수신 queue |
| G04 | `Client/Public/PlayerCommandSink.h`, `Client/Public/NetworkPlayerCommandSink.h`, `Client/Private/NetworkPlayerCommandSink.cpp` | sink 메서드 |
| G04 | `Client/Public/PlayerController.h`, `Client/Private/PlayerController.cpp` | `Request_DebugMadnessForm`, 결과 소비·status |
| G04 | `Client/Public/CharacterSpec.h`(없음) / `Client/Public/Character.h`, `Client/Private/Character.cpp` | `CHARACTER_DESC::eCharacterClass` override, `Get_CharacterClass()` |
| G04 | `Client/Public/CharacterCatalog.h`, `Client/Private/CharacterCatalog.cpp` | `Spec_KoukuSaydonClown`, `Find_ClownSpec()` |
| G04 | `Client/Public|Private/KoukuSaydonPresentationAssetService.*` | `Ensure_ClownBodyPrototype(level)` |
| G04 | `Client/Public/NetObjectRegistry.h`, `Client/Public|Private/ClientReplication.*` | record `eMadnessForm`, `Create_Character(form)`, 교체 edge |
| G04 | `Client/Private/PlayerController.cpp` | 퀵슬롯 class를 `Get_CharacterClass()`로 |
| G04 | `Client/Private/MainApp.cpp` | 버튼 2개 + Madness/form 표시 |

## 4. 데이터와 호출 흐름

### 4.1 G01 보스 spawn 위치

```text
Build_WorldEntity(placement)
→ nav 로드됨:
   ├─ Is_PointWalkableExact(px, pz) && Sample_Position(px, pz, ground) → 위치 = (px, ground.y, pz)   ← 새 규칙(저작 좌표 유지)
   └─ 아니면 Project_Point(px, pz) → 셀 중심 (기존 fallback), 실패 시 "Boss placement is outside server navigation"
```

다른 Area(0.5~1m 셀)도 같은 규칙을 타지만 이동량이 ≤0.7m였고 Valtan placement는 기존 테스트가 셀 중심 값을 고정하지 않는다(`Project_Point`를 쓰는 테스트는 자기 좌표를 스스로 계산).

### 4.2 G03/G04 wire

```text
enum PLAYER_MADNESS_FORM : uint8 { NORMAL = 0, CLOWN = 1, END }
PLAYER_SNAPSHOT += iCurrentMadness u32, iMaximumMadness u32, eMadnessForm u8
  검증: iCurrentMadness <= iMaximumMadness, eMadnessForm < END
C2S_DEBUG_SET_MADNESS_FORM { iRequestSequence u32 (≠0), eWorldId u16, eForm u8 }
S2C_DEBUG_SET_MADNESS_FORM_RESULT { iRequestSequence, eWorldId, eResult u8, eActiveForm u8 }
DEBUG_MADNESS_FORM_RESULT { ACCEPTED, REJECTED_DISABLED, REJECTED_SESSION, REJECTED_WRONG_WORLD,
                            REJECTED_STALE_SEQUENCE, REJECTED_PLAYER_STATE, REJECTED_SAME_FORM, END }
```

Server: `Handle_DebugSetMadnessForm` — Release는 `REJECTED_DISABLED`; 같은 world·newer sequence·살아 있고 GRABBED/pattern-bound 아님·다른 form일 때 `player.eMadnessForm = eForm`. 위치·HP·class·cooldown은 건드리지 않는다. spawn/class 변경/부활은 `iCurrentMadness = 0, iMaximumMadness = MADNESS_GAUGE_MAXIMUM(10000), eMadnessForm = NORMAL`로 초기화한다.

Client:

```text
S2C snapshot → CClientReplication::Apply_Snapshot
→ record.eCharacterClass != player.eCharacterClass || record.eMadnessForm != player.eMadnessForm
   → Replace_CharacterPresentation(player)  (기존 Replace_CharacterClass 본문, form 인자 추가)
      → Create_Character(class, form, …): form == CLOWN ?
           Ensure_ClownBodyPrototype + Find_ClownSpec() (desc.eCharacterClass = class)
         : Ensure_Prototypes(class) + Find_Spec(class)
→ CCharacter: m_eCharacterClass = desc override(END이면 spec class); Get_CharacterClass()
→ CPlayerController 퀵슬롯·LMB: pCharacter->Get_CharacterClass()  (광대 몸체에서도 원래 class 스킬 제출)
→ Kouku Arena Update: Rebind_LocalCharacter로 같은 Server player의 이동·행동 sequence 보존
→ 카메라: 교체 뒤 m_LocalCharacterHandle 갱신 → Level의 기존 재바인딩 경로 그대로
```

`Spec_KoukuSaydonClown`: asset `"KoukuSaydonClown"`(binding 문서 없음 → 스킬 clip 격리), body `Prototype_Component_Model_KoukuSaydonClown`, shader `Prototype_Component_Shader_VtxAnimMeshBinary`, 무기·장비 0, clips `{ rpct00_idle_battle_1, rpct00_run_battle_1, 나머지 nullptr }`(HIT/DEAD 등은 현재 pose 유지), `pCreateLogic nullptr`.

## 5. G별 구현 범위

### G01 — 관문 1·3 spawn 좌표 유지
- `Build_WorldEntity` 위 규칙. 상태 문자열은 유지.
- 계약 테스트(기존 arenaRoom 블록 안): built 세이튼 `fPositionX == -0.07f && fPositionZ == 942.33f`, `Refresh_PlayerBlockingBodies()` 뒤 `m_ServerCollisionSystem.Is_PlayerPositionClear(-2.84f, 1.32f, 941.02f, INVALID)` true.

### G02 — 뿅망치·yaw 튜닝
- catalog 두 row 텍스트 patch(전체 재직렬화 금지). python 기대값 갱신. `Publish-GameplayBalance.ps1` 재실행(catalog은 Client가 직접 읽지만 publisher 검증을 지난다).
- `CNpc`: `m_fDebugPresentationYawOffset`, `m_fDebugUnadjustedYaw`; `Apply_ImmediateTransform`/`Update_NetworkTransform`이 `yaw + offset`으로 회전; `Set_DebugPresentationYawOffset`이 즉시 재회전.
- 슬라이스: 5개 placement의 yaw offset과 Save `yawDegrees`(0~360 정규화)를 연결한다. 실시간 보정은 저장 baseline + 편집 offset을 현재 Server yaw에 대한 차이로 전달하여 Reload 뒤에도 목표 각도를 유지한다.
- 뿅망치 슬라이더 2개(`Big Saydon hammer x`, `Bingo hammer x`)는 실행 중 prototype의 catalog scale을 기준으로 한다. 회전은 실행 중 catalog 회전과 저장 baseline을 분리하고 `inverse(R(catalog)) * R(baseline + offset)`을 socket 전에 적용한다. live와 Save 후 재실행은 같은 목표 Euler 회전을 사용한다.

### G03 — 광기 수치 확장
- Shared v59 + 검증 + 하네스(round-trip, `current > maximum` 거부, `eMadnessForm >= END` 거부).
- Server 필드·리셋·snapshot. Client ViewModel.

### G04 — 광대 아바타 Debug 교체
- Shared 명령/결과 + 하네스 round-trip.
- Server 라우팅·핸들러·계약 테스트(Debug: NORMAL→CLOWN 수락, 같은 form 거부, 사망 거부; 값이 snapshot에 실림).
- Client 위 흐름. F1 `KoukuSaydon Arena` 아래 `Change to Clown` / `Return to Player`(현재 form에 따라 하나만 활성) + `Madness cur/max | form` 텍스트.

### G05 — Logic 런타임 (후속, 결정값 고정)
- Logic 정의에 typed 판정 필드 추가: DURATION `judgement { kind: SHIELD_REFLECT_CONE|FIND_REAL_CONE|DANCE_STEP, angleDegrees }`, RESULT `outcome { kind: MAX_HP_PERCENT_DAMAGE|WIPE|STAGGER_SUCCESS, percent }`.
- projector → Product `logics`/`logicOccurrences` → publisher `PATTERNLOGIC` 행 → Server `BOSS_PATTERN_DEFINITION::Logics` → `CKoukuSaydonBrain`이 재생 clock으로 window를 평가.
- 결정값: 방패 반사 = `dot(bossForward, normalize(attacker − boss)) ≥ cos 45°`면 그 hit의 `iRawDamage`를 `Apply_WorldToPlayer`로 공격자에게 되돌리고 boss 피해 0, cone 밖 hit은 무력화 누적 → 성공 시 GROGGY; 진짜 찾기 = window 종료 시 `dot(playerForward, normalize(realSaydon − player)) ≥ cos 45°`면 성공, 아니면 RESULT WIPE(전멸), Debug collider 표시 옵션; 댄스타임 timeout = 살아 있는 전 플레이어에게 `round(iMaximumHp × percent / 100)` 피해(50000·10% = 5000).
- 보류 이유: 저작 문서·Workbench(`KoukuSaydonCompositionDocument.cpp`, `KoukuSaydonActionWorkbench.cpp`)가 다른 세션에서 동시 편집 중이고, 진짜 찾기는 Summon 런타임(가짜 세이튼 4슬롯 spawn)이 선행한다.

## 6. 검증 요약

| 단계 | 명령 / 절차 | 기대 |
|---|---|---|
| G02 데이터 | `Publish-GameplayBalance.ps1`, `python -m unittest Tools/KoukuSaydonPipeline/test_kouku_saydon_runtime_inputs.py` | 성공 |
| 빌드 | `Invoke-BuildAndRegression.ps1 -Configuration Debug` | PASS |
| 하네스 | `NetworkProtocolHarness.exe` | failures 0 |
| Server | `Server.exe --contract-test` | failures 0 (G01·G04 검사 포함) |
| diff | `git diff --check` | 0 |
| runtime (사용자) | Server+Client 재시작(v59) → F1 `1관문 - 세이튼` → 이동 성공, 세이튼이 (-0.07, 942.33)에 선다 → `빙고` 뿅망치 크기 → `2관문` 대형 세이튼 뿅망치·yaw 행 조절·Save → `Change to Clown` → 광대 몸체로 우클릭 이동, `Return to Player` 복귀 | 눈으로 확인 |

## 7. 고정된 결정과 미결

- 고정: 광기 게이지는 **플레이어** 수치, 가득 차면 **플레이어**가 광대(MN_RPCT_03)로 변한다(이전 계획서의 "보스 변신" 가정은 폐기). 변신 지속 시간·게이지 증가 원인·광대 상태 스킬 제한은 Logic 런타임과 함께 정한다. 기존 HUD 바는 PR325 통합에서 연결했다.
- 고정(G05): 시야각 45°, 반사 cone 45°, timeout 최대 HP 퍼센트 피해.
- 미결: 광대 몸체의 forward가 class 몸체와 다르면(-90° admission 차이) 사용자 확인 뒤 `Spec_KoukuSaydonClown` admission에 회전을 더한다. 대형 세이튼 뿅망치 유효 배율은 슬라이스로 확정한다.
