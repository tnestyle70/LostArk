# 2026-09-05 KoukuSaydon Arena 관문별 스폰·플레이어 배치·보스 HUD 구현 계획서

> 문서 종류: 구현 계획서 (범위와 변경 단위 합의용)
>
> 상태: §7 추가 범위 및 §8 재검토 수정 반영. 실제 검증 상태는 대응 RESULT를 따른다.
>
> 기준 브랜치: `codex/kouku-scale-1p7` (같은 worktree에 다른 세션의 Logic/Summon 미커밋 변경이 있으므로 그 파일의 hunk는 보존한다)
>
> 선행 문서: [Logic/Summon Resources 결과](2026-09-05_KOUKU_SAYDON_LOGIC_RESOURCE_CATEGORY_RESULT.md), [무력화 반사 계획](2026-09-05_KOUKU_SAYDON_SHIELD_STAGGER_AND_COMMON_GROGGY_PATTERN_IMPLEMENTATION_PLAN.md)

## 0. 사용자 요구사항과 반영 내용

사용자가 대화에서 말한 요구를 그대로 옮기고, 각 항목이 이번 변경에서 어떻게 반영되는지 적는다.
"이번 변경"은 G01~G04이고, G05는 방향 확인 뒤 별도 변경 단위로 진행한다.

| # | 사용자 요구 (원문 요지) | 반영 내용 | 변경 단위 |
|---|---|---|---|
| R1 | F1의 `Authoring Sources` 표를 지우고 그 자리에 `KoukuSaydon Arena` 창을 열고 닫을 수 있게 | `RenderDebugResourceFiles()`와 그 전용 상태(`DEBUG_AUTHORING_SOURCE`, `m_DebugAuthoringSources`, `RefreshDebugAuthoringSources`, `OpenDebugAuthoringSource`)를 제거하고 같은 위치에 `KoukuSaydon Arena` CollapsingHeader를 둔다. Level이 KoukuSaydon Arena가 아니면 안내문만 표시한다 | G03 |
| R2 | `1관문 - 세이튼` 버튼: 세이튼이 `(-0.07, 1.32, 942.33)`에 Server 권위로 스폰, 플레이어는 약간 떨어진 곳에 배치, 발탄과 같은 보스 UI(이름 `세이튼` + 체력바) | 세계 JSON에 disabled boss placement `boss.kakulsaydon.g1.saydon`(archetype `BOSS_KAKULSAYDON_G1_SAYDON`, MN_RPCT_05, 0.017 배율)을 추가하고 버튼이 Server에 활성화를 요청한다. 플레이어는 사용자가 이미 Server 승인으로 얻은 `(-2.84, 1.32, 941.02)`로 typed teleport한다. HUD focus를 이 archetype으로 두어 `세이튼` 이름과 체력바가 뜬다 | G01~G03 |
| R3 | 스폰된 보스는 **대기(queue) 상태**여야 하고, Boss Tool의 `Start Full Pattern`/`Play Isolated`를 눌렀을 때만 재생. 발탄처럼 스폰 즉시 패턴이 돌면 안 됨 | 관문 보스는 Server에서 brain 없이 IDLE로만 유지한다(발탄 brain 경로에서 제외). Kouku 경로는 원래 `AUDITION_ONLY`라 명시 명령 전에는 어떤 패턴도 시작하지 않는다 | G02 |
| R4 | 콜라이더·배율은 지금 쿠크(1.7배 = `bodyModelPreScale 0.017`)처럼 | 세이튼(G1/G3/빙고)과 2관문 쿠크는 `0.017`, Server `collisionRadius 1.0`(현재 쿠크와 동일). Client Debug wire는 Server 복제 radius를 그대로 그린다 | G01 |
| R5 | `2관문 - 대형 세이튼, 쿠크` 버튼: 대형 세이튼은 배율 `1.f`(원본 기준 100배)로 `(10.74, 2.68, 316.45)`, 쿠크는 `(6.36, 10.56, 321.29)`, 플레이어는 `(3.38, 10.56, 323.92)`, HUD 이름은 `쿠크` | disabled placement 2개(`boss.kakulsaydon.g2.big-saydon` MN_RPCT_06 `bodyModelPreScale 1.0`, `boss.kakulsaydon.g2.kouku` MN_RPCZ_00 `0.017`). HUD focus는 `BOSS_KAKULSAYDON_G2_KOUKU`(`쿠크`). transform/scale 조절 UI는 이번 범위 밖 | G01~G03 |
| R6 | `3관문 - 세이튼`: `(-0.07, 1.32, 942.33)`에 세이튼, 플레이어 약간 떨어진 곳, HUD `세이튼` | `boss.kakulsaydon.g3.saydon`(`BOSS_KAKULSAYDON_G3_SAYDON`)을 1관문과 같은 좌표에 두고 같은 플레이어 좌표를 쓴다. 1관문과 별도 archetype·별도 HP | G01~G03 |
| R7 | `1마리오`: 플레이어만 `(-1150, -11.52, -909.28)` | 보스 없이 teleport만 보내고 HUD focus를 해제한다 | G03 |
| R8 | `2~4마리오`, `카드미로`: 네비 없어 보류 | 비활성 버튼과 "navigation 미보유로 보류" 안내만 둔다 | G03 |
| R9 | `빙고`: `(-0.6, 0.0, 1147.44)`에 세이튼이 뿅망치를 든 채 스폰, 이름 `앵콜을 외친 쿠크세이튼`, 플레이어 약간 떨어진 곳 | `boss.kakulsaydon.bingo.saydon`(`BOSS_KAKULSAYDON_BINGO_SAYDON`, MN_RPCT_05 + weapon `WP_MN_RPCT_06` 소켓 `b_wp_1`). 망치는 rest pose로 손 본을 따라간다(패턴 연동 전까지 clip 동기화 없음). 플레이어는 `(-3.4, 0.0, 1147.44)`로 teleport하며 Server navigation 검증 결과를 상태창에 표시한다 | G01~G03 |
| R10 | 보스 체력은 발탄과 동일, 1~3관문·빙고 총 4개 보스 UI가 이름과 함께 개별 체력 | `BossProfiles.json`에 5 profile(1관문 세이튼, 2관문 대형 세이튼, 2관문 쿠크, 3관문 세이튼, 빙고 세이튼)을 `maximumHp 600000`, `maximumHealthBars 160`(발탄 값)으로 추가. HUD는 focus archetype 하나만 표시하고 각 entity가 자기 HP를 가진다 | G01, G03 |
| R11 | F1에서 KoukuSaydon을 열면 발탄처럼 `Saved Patterns (N)` 목록이 나오고, `1관문` 같은 관문 카테고리로 나눠 선택한 뒤 `Complete Play` | `RenderCompletePlayControls`에 `KoukuSaydon Complete Play` 헤더를 추가한다. 관문 콤보(1관문/2관문/3관문/빙고)로 Product 패턴을 거르고 `Complete Play`는 기존 `CKoukuSaydonBossTool::Play_PatternById`, `Complete Play All`은 `Play_All`을 호출한다. 현재 Product는 Gate1 composition뿐이라 `1관문`만 목록이 있다 | G04 |
| R12 | Boss Tool은 추후. 다만 `Start Full Pattern`·`Play Isolated`가 스폰된 보스를 재생하는 흐름이 핵심 | 이번 변경은 기존 Boss Tool 버튼 라벨만 `Play Isolated`/`Start Full Pattern`으로 바꾼다. **재생 대상은 아직 기존 1관문 쿠크(`boss.kakulsaydon.g1.kouku`) 고정**이다. 스폰된 관문 보스가 자기 패턴을 재생하려면 G05(패턴 actor → 보스 archetype 라우팅)가 필요하다 | G04 / G05 |

## 1. 현재 실측

### 1.1 KoukuSaydon Arena에는 지금 HUD가 없다

- `CMainApp::Update_CombatHUD`, `Update_BossHealthBar`, `RenderBossHealthBarText`의 `isSupportedLevel`은 `BERN, VALTAN_ARENA, DEVELOPMENT, CHARACTER_SELECT`만 포함한다 ([MainApp.cpp:1735](../../Client/Private/MainApp.cpp), 3785, 4029). KAKULSAYDON_ARENA에서는 플레이어 HUD와 보스 체력바가 전부 숨겨진다.
- 보스 제목은 `L"마수군단장 발탄"` 하드코딩이다 (`MainApp.cpp:4075`). 이름 정본은 `Data/Balance/BossProfiles.json`의 `displayName`이며 `CCombatHUDViewModel::Apply_Boss`가 `strDisplayName`으로 이미 실어 준다.
- `CCombatHUDViewModel::Apply_Boss`는 snapshot의 모든 primary BOSS마다 호출되므로 여러 보스가 있으면 마지막 entity가 이긴다. 보스별 선택 규칙이 없다.

### 1.2 Server는 KoukuSaydon Arena에서 boss 활성화 명령을 받지 않는다

- `Handle_SpawnWorldEntity`는 `CHARACTER_SELECT_ARENA`만 허용하고 placement는 `disabled BOSS && archetype == BOSS_VALTAN`만 통과한다 ([GameRoom.cpp:4914](../../Server/Private/GameRoom.cpp)).
- `Handle_DespawnAllWorldEntities`도 `CHARACTER_SELECT_ARENA` 전용이며 room의 entity 전부를 지운다 (`GameRoom.cpp:3845`).
- `Build_WorldEntity`는 boss placement에 대해 `Find_Boss(archetype)` profile, `profile.strEncounterId == placement.encounterId`, `Find_BossPatterns(encounterId)` 비어 있지 않음을 요구한다 (`GameRoom.cpp:9769`). Kouku encounter `ENCOUNTER_KAKULSAYDON_G1`은 PIZZA 패턴을 가지므로 새 boss profile이 같은 encounter를 쓰면 이 조건을 만족한다.
- `Update_WorldEntities`는 `CKoukuSaydonBrain::Is_GateOneBoss`(archetype·placement exact)만 Kouku brain으로 보내고, 나머지 BOSS는 전부 발탄 brain 경로로 보낸다 (`GameRoom.cpp:14486`). 새 archetype을 그대로 두면 발탄 경로가 Kouku encounter 패턴에서 선택을 시도한다. 사용자 요구 R3(대기 상태)를 위해 명시적으로 제외해야 한다.
- `Apply_DebugTeleportToPosition`은 world 일치, walkable, 높이 1 m 이내, blocking body 비겹침을 검사한다. KAKULSAYDON_ARENA에서 이미 동작한다(사용자 스크린샷 `Server moved player to (-2.84, 1.32, 941.02)`).
- `Reset_ReplayableArenaWhenEmpty`는 KAKULSAYDON_ARENA도 대상이며 `Initialize_WorldEntities`가 enabled placement만 다시 만든다. 마지막 플레이어가 나가면 Debug로 활성화한 관문 보스는 사라진다.

### 1.3 Client presentation은 쿠크 archetype 하나만 안다

- `CKoukuSaydonPresentationAssetService::Ensure_Prototypes`는 `BOSS_KAKULSAYDON_G1_KOUKU`만 받고 weapon이 있으면 거부한다. binding 문서도 이 archetype 헤더로만 검증한다.
- `CClientReplication` spawn(2350행), snapshot(3444행), 생존 판정(584·2117행)이 모두 `KOUKU_SAYDON_BOSS_ARCHETYPE` 문자열 exact 비교다. 다른 BOSS는 발탄 `CValtan` 경로로 떨어져 실패한다.
- `CNpc`는 body 모델 하나만 그린다. 미리보기 패널은 `WP_MN_RPCT_06`을 `b_wp_1` 본 행렬 × 월드로 별도 `CPart_Body`에 붙인다 (`CharacterPreviewPanel.cpp:221`). `MN_RPCT_05`와 `MN_RPCT_06` 모두 `b_wp_1`을 가진다(wmodel 실측).
- clip 이름 실측: MN_RPCT_05 `rpct00_idle_battle_1 / rpct00_run_battle_1 / rpct00_dead_1`, MN_RPCT_06 `mn_rpct_06_sk.ao_idle_battle_1 / …evt2_rpct_move_01 / …dmg_critical_*`(run·dead clip 없음), MN_RPCZ_00 `rpcz00_idle_battle_1 / run_battle_1 / dead_1`.

### 1.4 데이터 publisher의 현재 제약

| publisher | 제약 | 이번 변경 |
|---|---|---|
| `Publish-WorldGameplay.ps1` 901~908행 | boss placement의 encounter profile `bossArchetypeId`가 placement archetype과 같아야 함 | `Get-EncounterProfiles`가 `BossProfiles.json`에서 같은 encounterId를 가진 archetype 집합을 붙이고 그 집합 소속으로 검사 |
| `Publish-GameplayBalance.ps1` 1348~1381행 | `BOSS_KAKULSAYDON_G1_KOUKU`만 weapon null 허용, 나머지 boss는 weapon 필수 | `BOSS_KAKULSAYDON_*` family는 weapon null 허용, weapon이 있으면 preScale 범위만 검사. G1_KOUKU exact 블록은 유지 |
| `Publish-GameplayBalance.ps1` 553·602~617행 | BossProfiles 모든 field가 provenance receipt에 있어야 하고 count 일치 | `Update-BalanceProvenanceReceipt.ps1`을 먼저 실행해 새 profile field를 `PROJECT_TUNED`로 등록. Kouku 전용 note 루프를 `boss:BOSS_KAKULSAYDON_*`로 확장 |
| `Publish-ServerNavigation.ps1` 538~553행 | 모든 boss placement(disabled 포함)가 walkable cell 위, 높이차 0.25 m 이내 | 새 placement 5개를 검증. 실패하면 Server nav 높이로 Y를 맞추고 RESULT에 기록 |
| `test_kouku_saydon_runtime_inputs.py` 87행 | weapon null인 boss가 정확히 `BOSS_KAKULSAYDON_G1_KOUKU` 하나 | weapon null 집합을 Kouku family 5개로, 빙고는 hammer 보유로 갱신 |

### 1.5 재사용하는 기존 계약

| 계약 | 위치 | 용도 |
|---|---|---|
| `IWorldEntityCommandSink::Request_SpawnWorldEntity(placementId)` / `Request_DespawnAllWorldEntities(seq)` | `Client/Public/WorldEntityCommandSink.h` | 관문 보스 활성화·정리 |
| `S2C_WORLD_ENTITY_SPAWN_RESULT` → `CNetworkManager::Try_Consume_WorldEntitySpawnResult` | `NetworkManager.h:314` | 버튼 결과 표시 |
| `IPlayerCommandSink::Request_DebugTeleportToPosition` + `CPlayerController::Update_DebugPlayerPlacement`의 결과 소비 | `PlayerController.cpp:728~870` | 좌표 지정 플레이어 배치 |
| `S2C_WORLD_ENTITY_SPAWNED` + `CClientReplication::Apply_WorldEntitySpawn` CNpc 경로 | `ClientReplication.cpp:2350` | 관문 보스 표현 |
| `CCombatHUDViewModel::Apply_Boss` → `CMainApp::Update_BossHealthBar` / `RenderBossHealthBarText` | `MainApp.cpp:3776, 4029` | 보스 이름·체력바 |
| `CKoukuSaydonBossTool::Reload / Play_PatternById / Play_All` + `CKoukuSaydonPatternAuditionService` | `KoukuSaydonBossTool.cpp` | F1 Complete Play |

## 2. 변경할 파일

새 C++ 파일은 없다. `.vcxproj`/`.filters` 등록 변경도 없다.

| G | 파일 | 역할 |
|---|---|---|
| G01 | `Data/Actors/BossCatalog.json` | archetype 5개 presentation row |
| G01 | `Data/Balance/BossProfiles.json` | Server profile 5개 |
| G01 | `Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json` | updater 실행 산출 (수동 편집 없음) |
| G01 | `Data/Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json` | disabled boss placement 5개, revision +1 |
| G01 | `Tools/WorldPipeline/Publish-WorldGameplay.ps1` | encounter ↔ boss archetype 집합 join |
| G01 | `Tools/GameplayPipeline/Publish-GameplayBalance.ps1` | Kouku family weapon 규칙 |
| G01 | `Tools/GameplayPipeline/Update-BalanceProvenanceReceipt.ps1` | Kouku family note |
| G01 | `Tools/KoukuSaydonPipeline/test_kouku_saydon_runtime_inputs.py` | 기대값 갱신 |
| G02 | `Server/Public/KoukuSaydonBrain.h`, `Server/Private/KoukuSaydonBrain.cpp` | `Is_ArenaBoss` |
| G02 | `Server/Private/GameRoom.cpp` | Arena boss 대기 경로, spawn/despawn 명령 KAKULSAYDON 허용 |
| G02 | `Server/Private/ServerGameplayContractTests.cpp` | Arena boss spawn/despawn/idle 검사 |
| G03 | `Client/Public/Npc.h`, `Client/Private/Npc.cpp` | optional socketed weapon |
| G03 | `Client/Public/KoukuSaydonPresentationAssetService.h`, `Client/Private/KoukuSaydonPresentationAssetService.cpp` | family archetype prototype + weapon prototype |
| G03 | `Client/Private/ClientReplication.cpp` | family archetype spawn/snapshot/despawn |
| G03 | `Client/Public/CombatHUDViewModel.h`, `Client/Private/CombatHUDViewModel.cpp` | boss focus archetype |
| G03 | `Client/Public/PlayerController.h`, `Client/Private/PlayerController.cpp` | 좌표 지정 Debug teleport |
| G03 | `Client/Public/Level_KakulSaydonArena.h`, `Client/Private/Level_KakulSaydonArena.cpp` | 관문 table, 활성화 명령, 결과 소비 |
| G03 | `Client/Public/MainApp.h`, `Client/Private/MainApp.cpp` | HUD gate, 제목, Authoring Sources 제거, `KoukuSaydon Arena` 창 |
| G04 | `Client/Public/KoukuSaydonBossTool.h`, `Client/Private/KoukuSaydonBossTool.cpp` | 목록 accessor, 버튼 라벨 |
| G04 | `Client/Private/MainApp.cpp` | KoukuSaydon Complete Play 헤더 |

## 3. 데이터와 호출 흐름

### 3.1 관문 table (Client Debug 정본, `Level_KakulSaydonArena.cpp` anonymous namespace)

좌표는 사용자가 Move Player로 얻은 값이다. yaw는 보스가 플레이어 배치 지점을 바라보도록 계산한 값이며 Server 관례 `forward = (sin yaw, cos yaw)`를 따른다.

| 버튼 | placement (disabled) | archetype / body / preScale | 보스 위치·yaw | 플레이어 teleport | HUD focus (displayName) |
|---|---|---|---|---|---|
| `1관문 - 세이튼` | `boss.kakulsaydon.g1.saydon` | `BOSS_KAKULSAYDON_G1_SAYDON` / MN_RPCT_05 / 0.017 | (-0.07, 1.32, 942.33), 245 | (-2.84, 1.32, 941.02) | `BOSS_KAKULSAYDON_G1_SAYDON` (`세이튼`) |
| `2관문 - 대형 세이튼, 쿠크` | `boss.kakulsaydon.g2.big-saydon` | `BOSS_KAKULSAYDON_G2_BIG_SAYDON` / MN_RPCT_06 / 1.0 | (10.74, 2.68, 316.45), 315 | (3.38, 10.56, 323.92) | `BOSS_KAKULSAYDON_G2_KOUKU` (`쿠크`) |
| | `boss.kakulsaydon.g2.kouku` | `BOSS_KAKULSAYDON_G2_KOUKU` / MN_RPCZ_00 / 0.017 | (6.36, 10.56, 321.29), 311 | | |
| `3관문 - 세이튼` | `boss.kakulsaydon.g3.saydon` | `BOSS_KAKULSAYDON_G3_SAYDON` / MN_RPCT_05 / 0.017 | (-0.07, 1.32, 942.33), 245 | (-2.84, 1.32, 941.02) | `BOSS_KAKULSAYDON_G3_SAYDON` (`세이튼`) |
| `1마리오` | 없음 | | | (-1150, -11.52, -909.28) | focus 해제 |
| `2마리오`, `3마리오`, `4마리오`, `카드미로` | 보류 (비활성 버튼) | | | | |
| `빙고 - 앵콜을 외친 쿠크세이튼` | `boss.kakulsaydon.bingo.saydon` | `BOSS_KAKULSAYDON_BINGO_SAYDON` / MN_RPCT_05 + `WP_MN_RPCT_06`@`b_wp_1` / 0.017 | (-0.6, 0.0, 1147.44), 270 | (-3.4, 0.0, 1147.44) | `BOSS_KAKULSAYDON_BINGO_SAYDON` (`앵콜을 외친 쿠크세이튼`) |

### 3.2 Server profile 첫 값 (`BossProfiles.json`)

| archetype | encounterId | displayName | maximumHp / bars | attackPower | collisionRadius | engage / move | phasePolicy |
|---|---|---|---|---|---|---|---|
| `BOSS_KAKULSAYDON_G1_SAYDON` | `ENCOUNTER_KAKULSAYDON_G1` | 세이튼 | 600000 / 160 | 100 | 1.0 | 100 / 1.0 | AUTHORED_PATTERN_EVENT |
| `BOSS_KAKULSAYDON_G2_BIG_SAYDON` | 동일 | 대형 세이튼 | 600000 / 160 | 100 | 4.0 | 100 / 1.0 | 동일 |
| `BOSS_KAKULSAYDON_G2_KOUKU` | 동일 | 쿠크 | 600000 / 160 | 100 | 1.0 | 100 / 1.0 | 동일 |
| `BOSS_KAKULSAYDON_G3_SAYDON` | 동일 | 세이튼 | 600000 / 160 | 100 | 1.0 | 100 / 1.0 | 동일 |
| `BOSS_KAKULSAYDON_BINGO_SAYDON` | 동일 | 앵콜을 외친 쿠크세이튼 | 600000 / 160 | 100 | 1.0 | 100 / 1.0 | 동일 |

`armorPlates`는 전부 `[]`. HP/bars는 발탄 값이다(R10). 대형 세이튼 `collisionRadius 4.0`은 배율 1.0에 맞춘 첫 값이며 플레이어 teleport 지점(약 10.6 m 거리)과 겹치지 않는다.

### 3.3 BossCatalog row (`BossCatalog.json` v7)

| archetype | bodyModel | weaponModel / weaponModelPreScale | bodyModelPreScale | presentationClips |
|---|---|---|---|---|
| `BOSS_KAKULSAYDON_G1_SAYDON` | `Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05.wmodel` | null / null | 0.017 | idle `rpct00_idle_battle_1`, chase `rpct00_run_battle_1`, windup/active/recovery `rpct00_att_battle_6_04 / 6_02 / 6_03`, dead `rpct00_dead_1` |
| `BOSS_KAKULSAYDON_G2_BIG_SAYDON` | `Character/KoukuSaton/MN_RPCT_06/MN_RPCT_06.wmodel` | null / null | 1.0 | idle `mn_rpct_06_sk.ao_idle_battle_1`, chase `…evt2_rpct_move_01`, windup/active/recovery `…att_battle_1_01 / 1_02 / 1_05`, dead `…dmg_critical_end_1` |
| `BOSS_KAKULSAYDON_G2_KOUKU` | `Character/KoukuSaton/MN_RPCZ_00/MN_RPCZ_00.wmodel` | null / null | 0.017 | G1 쿠크와 동일 |
| `BOSS_KAKULSAYDON_G3_SAYDON` | MN_RPCT_05 | null / null | 0.017 | G1 세이튼과 동일 |
| `BOSS_KAKULSAYDON_BINGO_SAYDON` | MN_RPCT_05 | `Character/KoukuSaton/WP_MN_RPCT_06/WP_MN_RPCT_06.wmodel` / 1.0 | 0.017 | G1 세이튼과 동일 |

`serverProfileId`/`clientPresentationId`는 `boss.kakulsaydon.<gate>.<name>.server.v1` / `.client.v1`, `visualAssetId`는 `KOUKUSAYDON_<body>`, `animationSetId`는 body 경로, `presentationStatus complete`, `armorModels/armorParts/combatObjectVisuals` 빈 배열이다. Client `CActorCatalog`는 이미 weapon null을 허용한다.

### 3.4 실행 흐름

```text
F1 KoukuSaydon Arena 버튼 (CMainApp::RenderKoukuSaydonArenaControls)
→ CLevel_KakulSaydonArena::Debug_ActivateGate(gateIndex)
   ├─ IWorldEntityCommandSink::Request_DespawnAllWorldEntities(seq)   (이전 관문 보스 정리)
   ├─ 각 placement: Request_SpawnWorldEntity(placementId)
   ├─ CPlayerController::Request_DebugTeleportToPosition(KAKULSAYDON_ARENA, x, y, z)
   └─ CCombatHUDViewModel::Set_BossFocusArchetype(hudArchetype)
Server (같은 room tick, 명령 순서대로)
   ├─ Handle_DespawnAllWorldEntities: KAKULSAYDON이면 bootstrap disabled placement에서 온 entity만 제거
   ├─ Handle_SpawnWorldEntity: disabled BOSS + Kouku encounter 검증 → Build_WorldEntity → Broadcast_WorldEntitySpawned → SPAWNED/ALREADY_EXISTS/REJECTED
   ├─ Apply_DebugTeleportToPosition: navigation/높이/충돌 검증 → 플레이어 이동
   └─ Update_WorldEntities: Is_ArenaBoss → PinnedDefinitionRevision만 갱신, brain 없음 (IDLE 유지 = queue 상태)
Client
   ├─ Apply_WorldEntitySpawn: Kouku family → CKoukuSaydonPresentationAssetService::Ensure_Prototypes(archetype) → CNpc(+weapon)
   ├─ snapshot: Kouku family → CNpc::Apply_NetworkState + CCombatHUDViewModel::Apply_Boss (focus archetype만 통과)
   ├─ Update_BossHealthBar/RenderBossHealthBarText: KAKULSAYDON_ARENA 포함, 제목 = displayName
   └─ Level::Update: Try_Consume_WorldEntitySpawnResult → 상태 문자열 (F1 창에 표시)
```

패턴 재생은 이번 변경에서 기존 경로 그대로다: `Complete Play`/`Play Isolated`/`Start Full Pattern` → `CKoukuSaydonPatternAuditionService` → `C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST` → Server `Find_KoukuSaydonAuditionBoss()`(= 기존 1관문 쿠크 `boss.kakulsaydon.g1.kouku`).

## 4. G별 구현 범위

### G01 — 데이터: catalog, profile, world placement, publisher

- `BossCatalog.json`: §3.3 row 5개를 `BOSS_KAKULSAYDON_G1_KOUKU` 뒤에 추가.
- `BossProfiles.json`: §3.2 profile 5개를 쿠크 profile 뒤에 추가.
- `Gameplay.world.json`(LV_LUT_MIDNIGHTC_ED): §3.1 placement 5개를 `boss.kakulsaydon.g1.kouku` 뒤에 `"enabled": false`로 추가, `revision` 1867 → 1868.
- `Publish-WorldGameplay.ps1` `Get-EncounterProfiles`: `Data/Balance/BossProfiles.json`을 읽어 encounterId별 `bossArchetypeIds` 집합을 profile에 붙이고, boss placement 검사를 `placement.archetypeId ∈ profile.bossArchetypeIds`로 바꾼다. encounter 자체의 `bossArchetypeId`는 그대로 집합에 포함된다.
- `Publish-GameplayBalance.ps1` 1348~1381행: `$isKoukuSaydonFamily = archetypeId -like 'BOSS_KAKULSAYDON_*'`. G1_KOUKU exact 블록 유지. family이면 weapon null 허용, weapon 문자열이면 `weaponModelPreScale` (0, 100] 검사. 그 외 boss는 기존 규칙.
- `Update-BalanceProvenanceReceipt.ps1` 190행 루프: `targetId -like 'boss:BOSS_KAKULSAYDON_*'`로 확장.
- `test_kouku_saydon_runtime_inputs.py`: weapon null 집합과 빙고 hammer 기대값.

검증: `Update-BalanceProvenanceReceipt.ps1` → `Publish-GameplayBalance.ps1` → `Publish-WorldGameplay.ps1`(또는 `Invoke-BuildDomainOwner.ps1 -Owner Server`) → `Publish-ServerNavigation.ps1 -Mode Validate` → `PYTHONPATH=. python -m unittest Tools.KoukuSaydonPipeline.test_kouku_saydon_runtime_inputs` → 생성 bootstrap에서 `BOSS\tBOSS_KAKULSAYDON_G1_SAYDON` 행과 world bootstrap의 disabled boss 행 5개 grep.

### G02 — Server: Arena boss 대기 경로와 Debug 명령

**`KoukuSaydonBrain.h/.cpp`**: `static bool Is_ArenaBoss(WORLD_ID, const SERVER_WORLD_ENTITY&)`. KAKULSAYDON_ARENA + BOSS + owner 없음 + encounter `ENCOUNTER_KAKULSAYDON_G1` + archetype prefix `BOSS_KAKULSAYDON_`. `Is_GateOneBoss`는 유지(패턴 audition 대상).

**`GameRoom.cpp`**

- `Update_WorldEntities` 14486행 뒤: `Is_ArenaBoss && !Is_GateOneBoss` → `entity.PinnedDefinitionRevision = m_GameplayCatalog.Get_ActiveRevision(); continue;` (brain 없음, IDLE 유지). 발탄 경로로 내려가지 않는다.
- `Handle_SpawnWorldEntity`: world 허용 집합 `{CHARACTER_SELECT_ARENA, KAKULSAYDON_ARENA}`. placement 검사를 world별로 나눈다. CHARACTER_SELECT: 기존 `BOSS_VALTAN`. KAKULSAYDON: `!isEnabled && BOSS && encounter == KOUKUSAYDON_G1_ENCOUNTER_ID && archetype prefix BOSS_KAKULSAYDON_`. spawn group 경로는 CHARACTER_SELECT만.
- `Handle_DespawnAllWorldEntities`: KAKULSAYDON이면 `Find_Placement(entity.strPlacementId)`가 disabled인 entity(와 그 dependent child)만 제거하고 enabled 쿠크·NPC는 유지. spawn group reset은 CHARACTER_SELECT만.
- contract test: `ServerGameplayContractTests.cpp` Kouku 블록 뒤에 (a) KAKULSAYDON room에서 `C2S_SPAWN_WORLD_ENTITY{boss.kakulsaydon.g1.saydon}` → `SPAWNED`, entity IDLE, `Is_GateOneBoss false`, `Is_ArenaBoss true`; (b) 몇 tick `Update_WorldEntities` 뒤에도 `strPatternId` 비어 있음; (c) despawn-all 뒤 `boss.kakulsaydon.g1.kouku`만 남음; (d) enabled placement ID로 spawn 요청 → `REJECTED`.

검증: Debug Product 빌드(Server), `Server.exe --contract-test` failures 0.

### G03 — Client: presentation, HUD, 관문 창

**`Npc.h/.cpp`**: `NPC_DESC`에 `wstring_t strWeaponModelTag`, `const char_t* pWeaponSocketBone`, `f32_t fWeaponScale = 1.f`. `Ready_Components`에서 tag가 있으면 `Add_Component(Com_WeaponModel)` 후 `Refresh_BoneCombinedMatrices()`(rest pose). `Render`에서 body 뒤에 weapon: `g_WorldMatrix = Scale(fWeaponScale) × body->Get_BoneMatrix(socket) × world`, `Bind_BoneMatrices` + 각 mesh Render. 소켓 본이 없으면 `Ready_Components` 실패로 fail-closed.

**`KoukuSaydonPresentationAssetService`**: 허용 archetype을 `BOSS_KAKULSAYDON_` prefix + catalog `clientPresentationId` prefix `boss.kakulsaydon.`로 일반화. model tag `Prototype_Component_Model_KoukuSaydon_<archetype>`, weapon tag `…_<archetype>_Weapon`(catalog weapon이 있을 때만, `weaponModelPreScale`로 pre-transform). binding 로드는 `BOSS_KAKULSAYDON_G1_KOUKU`에서만. `Get_WeaponSocketBone()`은 `b_wp_1` 상수.

**`ClientReplication.cpp`**: file-static `Is_KoukuSaydonFamilyBoss(archetype, encounter, owner)`. spawn 2350행·snapshot 3444행·생존 584/2117행을 이 helper로 교체. spawn 시 `NPC_DESC` weapon 필드를 service에서 채운다. `Apply_WorldEntityDespawn`에서 family boss의 `pNpc` 제거 뒤 `CCombatHUDViewModel::Get().Clear_BossIfArchetype(archetype)`.

**`CombatHUDViewModel`**: `Set_BossFocusArchetype(string)`, `Clear_BossFocus()`, `Get_BossFocusArchetype()`, `Clear_BossIfArchetype(string_view)`. `Apply_Boss`는 focus가 비어 있지 않고 archetype이 다르면 무시. focus는 Debug session 상태이며 Level 전환 시 `Clear_Boss`와 함께 비운다.

**`PlayerController`**: `bool_t Request_DebugTeleportToPosition(WORLD_ID, f32_t x, f32_t y, f32_t z)`(Debug). pending이 없고 local character·sink가 있으면 즉시 `m_pCommandSink->Request_DebugTeleportToPosition(seq, x, y, z)`를 보내고 기존 pending/결과 소비 경로를 그대로 쓴다. 결과 문자열은 기존 `Get_DebugPlayerPlacementStatus()`.

**`Level_KakulSaydonArena`**: `KAKUL_DEBUG_GATE` table(§3.1), `Debug_ActivateGate(size_t, string& status)`, `Get_DebugGates()`, `Get_DebugGateStatus()`. `Update`에서 `Try_Consume_WorldEntitySpawnResult`를 소비해 `placementId: SPAWNED/ALREADY_EXISTS/ACTIVATED/REJECTED`를 상태에 누적. 명령 sequence는 Level 멤버 `m_iNextDebugGateRequestSequence`.

**`MainApp`**: `Update_CombatHUD`/`Update_BossHealthBar`/`RenderBossHealthBarText`/`RenderCombatHUDText`의 level gate에 `KAKULSAYDON_ARENA` 추가. 보스 제목: archetype `BOSS_VALTAN`이면 기존 `마수군단장 발탄`, 아니면 `boss.strDisplayName`(UTF-8 → wide). `RenderDebugResourceFiles`와 authoring source 전용 상태·함수 제거(`RefreshDebugResourceFiles`/`OpenDebugResourceFile`/`m_DebugResourceFiles`는 다른 소비자가 있으면 유지). 새 `RenderKoukuSaydonArenaControls()`를 같은 호출 위치(7061행)에 둔다: CollapsingHeader `KoukuSaydon Arena`, Level 검사, 관문 버튼 6개(보류 4개 비활성), `Despawn Arena Bosses`, `HUD focus: <archetype>` 표시, Level 상태·teleport 상태 표시.

검증: Debug Product 빌드(Client), `git diff --check`. 실행 확인은 사용자(§5).

### G04 — F1 KoukuSaydon Complete Play와 Boss Tool 라벨

- `CKoukuSaydonBossTool`: `Get_ProductPatterns()`, `Get_PlayAllPatternIds()`, `Get_SourceRevision()`, `Has_SavedComposition()` accessor; `Play_All` public. 버튼 라벨 `Play Selected` → `Play Isolated`, `Play All` → `Start Full Pattern`.
- `CMainApp::RenderCompletePlayControls` 끝에 CollapsingHeader `KoukuSaydon Complete Play (Server Boss Replay)`: `Load/Reload KoukuSaydon Inventory` → `m_pKoukuSaydonBossTool->Reload`; 관문 콤보 `1관문 / 2관문 / 3관문 / 빙고`(Product는 encounter G1뿐이므로 `1관문`만 목록, 나머지는 "저장된 composition 없음"); `Saved Patterns (N)` Selectable 목록(`patternId | displayName`); `Complete Play` → `Play_PatternById(selected, sourceRevision)`; `Complete Play All` → `Play_All`. 둘 다 Level이 KAKULSAYDON_ARENA일 때만 활성. 상태는 audition snapshot `strStatus`.

검증: Debug Product 빌드, 사용자 실행 확인.

### G05 (다음 변경 단위, 이번 범위 밖) — 패턴을 스폰된 관문 보스에서 재생

- composition pattern `actorProfileId`(MN_RPCZ_00 / MN_RPCT_05 / MN_RPCT_06)를 Product pattern `bossArchetypeId`로 투영하고 publisher가 `PATTERN` 행에 실어 Server catalog가 pattern별 대상 archetype을 안다.
- Server `Evaluate_KoukuSaydonPatternAudition`/`Update_KoukuSaydonBoss`: 요청 pattern의 archetype과 같은 **살아 있는 Arena boss**(관문 버튼으로 스폰된 entity)를 대상으로 잡고, 대상이 없으면 `REJECTED_NO_BOSS`. despawn-all은 진행 중 audition을 ABORTED로 닫는다.
- Client: `KoukuSaydon.patternbindings.json`을 archetype별로 나누고 `CKoukuSaydonPresentationAssetService`가 각 body 모델의 clip으로 검증. `ClientReplication` snapshot 경로가 family boss 전부에 action binding을 적용.
- Boss Tool: `Start Full Pattern`은 선택 관문의 Play All, `Play Isolated`는 선택 패턴을 그 관문 보스에서 재생.

## 5. 검증 요약

| 단계 | 명령 / 절차 | 기대 |
|---|---|---|
| G01 | receipt updater → balance publisher → world publisher → navigation validate → python 테스트 | bootstrap에 BOSS 행 5개, world bootstrap disabled boss 5개, navigation 검증 통과 |
| G02 | Debug 빌드, `Server.exe --contract-test` | Arena boss 검사 4건 통과, 기존 Kouku/발탄 검사 유지 |
| G03/G04 | Debug Product 빌드, `git diff --check` | 오류 0 |
| runtime (사용자) | Lobby → KoukuSaydon → F1 `KoukuSaydon Arena` → `1관문 - 세이튼` | 세이튼이 (-0.07, 1.32, 942.33)에 IDLE로 서 있고 플레이어가 (-2.84, 1.32, 941.02)로 이동, HUD `세이튼` 600000 HP · 160줄, 스킬로 HP 감소 |
| runtime (사용자) | `2관문` / `3관문` / `1마리오` / `빙고` | 대형 세이튼 1.0 배율 + 쿠크(HUD `쿠크`), 3관문 세이튼, 1마리오 이동만, 빙고 세이튼이 망치를 들고 HUD `앵콜을 외친 쿠크세이튼` |
| runtime (사용자) | F1 `KoukuSaydon Complete Play` → `1관문` → PIZZA → `Complete Play` | 기존 1관문 쿠크(`boss.kakulsaydon.g1.kouku`)가 PIZZA를 재생. 스폰된 세이튼은 G05 전까지 대기만 한다 |

에이전트는 데이터 publish·빌드·contract test까지 실행하고, Client 실행과 화면 판정은 사용자가 한다.

## 7. 검토 후 추가 반영 (사용자 추가 요청 · Codex 비평)

사용자 검토 뒤 들어온 요청과 Codex 비평을 실제 코드로 대조해 같은 변경 단위에 넣었다. G05는 "보류"에서
"이번 변경"으로 바뀌었다(사용자: 지금 저작된 패턴은 사실상 세이튼 패턴뿐이므로 관문 세이튼에서 재생돼야 함).

| 출처 | 요구/지적 | 반영 |
|---|---|---|
| 사용자 | 대형 세이튼 10배 축소, 뿅망치 1/100 | `bodyModelPreScale 0.1`, 빙고 `weaponModelPreScale 0.01` |
| 사용자 | HUD 표시 | G03 gate 4곳에 KAKULSAYDON_ARENA 포함 |
| 사용자 | 누른 관문 버튼 비활성, 다른 관문 누르면 그 보스 HUD | Level `m_iActiveDebugGate`, MainApp이 활성 관문과 Server 응답 대기 중 버튼을 비활성 |
| 사용자 | Workbench Stage 순서 이동 | Stage와 animation box는 묶여 있음(occurrence offset이 Stage 기준). `< Earlier` / `Later >` 버튼과 Left/Right 키가 `Move_SelectedStage`로 한 칸 이동 후 같은 선택 유지. 드래그 삽입선은 기존 드래그(box 이동/트림)와 충돌해 채택하지 않음 |
| 사용자 | MN_RPCT_03을 Resources Animation에 | 다른 세션이 `COMPOSITION_ANIMATION_TARGET_ASSET_NAMES`·preview asset 표에 등록했고 Workbench 물리 clip 목록은 그 표에서 만들어지므로 이번 빌드부터 표시. Pattern actor로의 Append는 arena boss body가 아니라 미지원 |
| 사용자 | 세이튼 패턴을 관문 세이튼에서 재생 (G05) | projector가 BossCatalog `bodyModel` join으로 pattern별 `bossArchetypeIds`를 투영, publisher `PATTERNBOSS` 행, Server `BOSS_PATTERN_DEFINITION::AuditionBossArchetypeIds`, audition scope가 관문 보스 placement를 이름 짓고 body 불일치는 `REJECTED_UNSUPPORTED_PATTERN`, Client audition service `Set_TargetBoss`(관문 버튼이 설정), presentation binding을 archetype별로 body clip 기준 필터 |
| 사용자 | 대형 세이튼 위치·배율·뿅망치 배율 임시 튜닝 + Save/Reload | F1 `KoukuSaydon Arena` 하단 `Big Saydon / Hammer Tuning (temporary)`: live multiplier(Client 표현 전용) + Save(BossCatalog/world JSON 숫자만 patch) + Reload. 값 확정 뒤 제거 예정 |
| Codex P1 | Server 로더가 G1_KOUKU만 part 없는 보스로 허용 | `GameplayCatalog.cpp`의 `isKoukuSaydonGateOne`을 `ENCOUNTER_KAKULSAYDON_G1` + `BOSS_KAKULSAYDON_*` family로 정의(부위·패턴 선택·페이즈 검사 3곳이 같은 변수) |
| Codex P1 | Client `ActorCatalog`가 무기 없는 보스를 G1_KOUKU로 한정 → 전체 catalog 초기화 실패 | `hasEmbeddedBodyOnly`를 family(`BOSS_KAKULSAYDON_*` + `boss.kakulsaydon.*.client.v1`)로 확장 |
| Codex P2 | 관문 연속 클릭 시 보스/플레이어 불일치 | `Debug_ActivateGate`가 명령을 보내기 전에 이전 teleport pending을 검사해 거부하고, F1 버튼도 pending 동안 비활성 |
| Codex P2 | Release Server에서 spawn/despawn은 허용, teleport는 거부 | KAKULSAYDON spawn/despawn을 `#ifdef _DEBUG`로 teleport와 같은 경계 |
| Codex | 90 tick idle 테스트가 tick을 올리지 않음 | 테스트가 `m_iServerTick`을 tick마다 증가 |
| Codex | 안내 문구가 실제 지원보다 앞섬 | G05 반영 뒤 "Product가 그 보스 body를 나열할 때만 재생"으로 문구 교정 |

## 6. 방향 확인이 필요한 항목 (검토 완료)

1. 패턴 재생 대상: 이번 변경은 기존 1관문 쿠크 고정이고, 스폰된 관문 보스 재생은 G05로 이어서 진행하는 순서가 맞는지.
2. HP를 네 보스 모두 발탄 값(600000 / 160줄)으로 두는지, 대형 세이튼 `collisionRadius 4.0` 첫 값이 괜찮은지.
3. 보스 yaw를 플레이어 배치 지점을 바라보게 두는 규칙과, 빙고 플레이어 지점 `(-3.4, 0.0, 1147.44)`(Server navigation이 거부하면 상태창에 사유 표시) 사용.
4. 빙고 망치는 `WP_MN_RPCT_06`를 `b_wp_1` rest pose로 붙인다(패턴 연동 전까지 clip 동기화 없음).
5. `Authoring Sources` 패널은 숨김이 아니라 코드 제거로 처리한다.

## 8. 재검토에서 닫은 경계

기존 G05의 관문 보스 재생을 실제 room tick까지 연결하고, 전환 중 새 Play 차단 및 Level 재진입 대상 초기화를 반영했다.
관문 버튼 active 상태는 spawn/teleport 응답 성공 뒤 확정한다. RAW/source action 0의 publisher/native reader 계약을 맞춘다.
임시 튜닝 저장은 반복 저장에 같은 값을 쓰고 외부 수정과 다중 파일 실패를 보존하며, 화면용 배율과 collider mirror를 분리한다.
세이튼의 댄스·방패 무력화·진짜 세이튼 찾기는 현재 DRAFT이며 이번 수정에서 Product로 승격하지 않는다.
현재 피자 Product를 사용하는 자동 검증과 사용자가 수행할 Boss Tool 화면 검증은 RESULT에서 분리한다.
