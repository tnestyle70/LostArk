# 쿠크 카드미로 — 현재 프레임워크 기반 설계 검토서

작성: 2026-09-08. 범위: 사용자 요구와 현재 코드·데이터를 대조한 설계 설명. 이번 작업은 게임 코드, 저작 JSON, Resources, 배포물을 수정하지 않는다. 아래 신규 계약은 제안이며 현재 구현됐다는 뜻이 아니다. 아직 식별되지 않은 모델을 임의 연결한 전체 코드 PLAN을 만들지 않는다.

## G00. 현재 있는 것과 이번에 연결할 것

### 사용자가 확정한 규칙

- 처치 목표인 **문양 몬스터**와 길을 가로지르는 **행진 병사**는 서로 다른 몬스터다. 파일명이나 같은 외형 여부와 관계없이 게임플레이 역할과 처치 집계를 분리한다.
- 시작 시 망원경 담당 1명, 문양 몬스터를 처치하는 3명이다.
- 각 처치 담당자는 발밑 문양과 같은 문양의 몬스터 3마리를 잡는다. 목표 몬스터는 통로에 랜덤 생성한다.
- 첫 문양 몬스터를 **타격했을 때** 행진을 시작한다. 첫 처치나 카메라 종료가 시작 조건은 아니다.
- 행진 출발 방향 순서는 3시→9시, 6시→12시, 9시→3시, 12시→6시다. 시계방향으로 바뀌는 것은 출발 측이며 병사가 맵 외곽을 원형으로 도는 것이 아니다.
- 먼저 3마리를 처치한 사람은 중앙 무적 구역으로 이동하여 망원경으로 함께 전체 맵을 볼 수 있다.

### 현재 파일에서 확인한 사실

| 항목 | 확인 결과 | 직접 근거 |
|---|---|---|
| F1 카드미로 이동 | 기존 쿠크 아레나에 버튼과 목적지 `(0.09, -0.01, 1351.48)` 존재 | `Client/Private/Level_KakulSaydonArena.cpp`, `Get_DebugGates()` |
| 외곽 배치 | `Miro_1`~`Miro_36` 존재. 전부 `triggerBox`, `enabled:false`, `events:[]` | `Data/Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json` |
| 쿠크 몬스터 생성 | 저작 spawnGroups 0개. 배포 bootstrap도 groups 0 / profiles 0 | 같은 Area의 `SpawnGroups.world.json`, `Server/Bin/DataFiles/World/KAKULSAYDON_ARENA.spawngroupsbootstrap` |
| 일반 몬스터 정의 | MonsterCatalog와 MonsterProfiles 각각 발탄 계열 5행. 쿠크 카드 병사 등록 없음 | `Data/Actors/MonsterCatalog.json`, `Data/Balance/MonsterProfiles.json` |
| 몬스터 공통 실행 경로 | Server 생성·AI·HP·사망과 Client 표현 경로 존재. 쿠크도 publisher 대상임 | `SpawnGroupBootstrap`, `SpawnGroupRuntime`, `GameRoom::Spawn_Monster`, `MonsterBrain`, `ClientReplication` |
| 미로 HUD | MAZE 모드와 Q 뿅망치 이미지·모션 바인딩 존재 | `Data/UI/KoukuSaydon/KoukuHudModes.json`, `Data/Animation/Authored/KoukuSaydon/Clown.interactionbindings.json` |
| 미로 공격 | `Handle_InteractionSlot()`은 action/쿨타임/모션 상태를 시작하지만 MAZE 몬스터 hit/kill 판정은 연결하지 않음 | `Server/Private/GameRoom.cpp:3449` |
| 카드 문양 타입 | HEART/SPADE/CLUB/DIAMOND 존재. 현재 player 문양 상태는 룰렛과 공유 중 | `Shared/Public/Network/PacketMessages.h:691`, `Server/Private/KoukuSaydonLogicRuntime.cpp` |
| 망원경 카메라 | 기존 카메라 샷 저작·재생은 존재. 현재 샷 24개에 미로 전용 샷은 없음 | Area `camerashots.json`, `Find_ActiveCameraShot()` |
| 통신 | 조사 당시 protocol 68 | `Shared/Public/Network/PacketType.h:59` |

따라서 **네비가 있는 맵에 시퀀스 몇 개만 배치해서 완성되는 기능은 아니다.** 기존 몬스터·입력·카메라 경로를 재사용하면서 미로 참가자, 최초 타격, 개인별 목표, 행진, 관전 자격을 서버에 추가해야 한다.

### 리소스 확인 결과

원본 추출 DB `.codex_tmp/monster_source_20260806/EFTable_Npc.db`의 `Npc` 테이블을 read-only SQL로 확인했다. 이 DB는 식별 참고 자료이며 제품 런타임 입력은 아니다.

| 원본 NPC 행 예시 | DB Comment1 | Model 참조 |
|---|---|---|
| 480644 | 카드 병사_스패이드 | `EFDLChar_MN_PPCH_01.MN_PPCH_01` |
| 480645 | 클로버 카드 병사_노말 | `EFDLChar_MN_PPCH_00-2.MN_PPCH_00-2` |
| 480646 | 하트 카드 병사_노말 | `EFDLChar_MN_PPCH_00.MN_PPCH_00` |
| 480647 | 다이아 카드 병사_노말 | `EFDLChar_MN_PPCH_00-1.MN_PPCH_00-1` |
| 480708 / 480709 | 카드미로 세토 로밍형 / 돌진형 | `EFDLChar_MN_PPCT_00.MN_PPCT_00` |

같은 PPCH 모델을 참조하면서 AI가 다른 NPC 행이 여럿 있다. 위 네 행을 곧바로 이번 목표/행진 배역의 정확한 원본 spawn 행이라고 확정하지 않는다. 모델 계열과 문양을 식별한 증거이며, 두 배역의 정확한 원본 행은 해당 이벤트/스폰 참조와 추가 대조해야 한다.

현재 `Client/Bin/Resources` 전역 파일명 검색에서 `MN_PPCH`/`WP_MN_PPCH` 경로가 나오지 않았다. 다른 이름으로 쿠킹됐을 가능성까지 부정하는 검사는 아니다. 따라서 팀장이 추가했다는 카드 병사 모델의 **현재 물리 경로와 모델 대응표는 아직 확보되지 않았다.** 없는 것으로 단정하거나 세토를 정답으로 대신 넣지 않는다.

반면 `Character/KoukuSaton/MN_PPCT_00/MN_PPCT_00.wmodel`은 실제 존재하고 `ModelAssetConverter info`에서 skeleton, material 4개, animation 68개를 확인했다. `Seto_idle_normal_1`, `Seto_walk_normal_1`, `Seto_run_battle_1`, `Seto_att_battle_21_02`, `Seto_dead_1`도 존재한다. 현재는 World Object의 세토 모델로 등록되어 있으며, 이것만으로 피격·사망 가능한 Server 몬스터가 되지는 않는다.

`MN_RHOC_00`/`MN_RHOC_00-1`은 원본 DB상 뒤집힌 빈 카드/조커 카드다. 이 둘을 문양 병사로 오인하지 않는다. 망원경 이름의 Effect DDS도 존재하지만, 그 텍스처만으로 망원경 아이템 획득과 관전 기능이 구현됐다고 판정하지 않는다.

## G01. 맵 배치와 네비게이션

### 재사용할 정본

- 바닥·카드 외형: 기존 Map catalog/placement.
- 보행: 기존 `Data/Navigation/LV_LUT_MIDNIGHTC_ED.navsource/.navpaint` → Navigation publisher → Server `.navgrid`.
- 트리거: 기존 `Data/Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json`.
- 몬스터 스폰 좌표: 기존 `SpawnGroups.world.json`의 anchor 계약.

현재 배포된 기본 grid는 `524×800`, cellSize `4m`, origin `(-1960,-1780)`, 높이 통과 한계 `1m`다. detail region은 Mario2/3/4뿐이다. 미로 시작점의 cell은 walkable=1, 높이 약 -0.01m다. 그러나 `Miro_14`, `Miro_32`, `Miro_23` 중심의 기본 grid cell은 walkable=0이었다. 이 결과는 **세 지점의 cell 조회**이며 미로 전체 길찾기·충돌 검증 PASS가 아니다.

기존 `Miro_*` 상자에 무작정 소환 이벤트를 넣지 않는다. 상자는 이벤트 없는 초안이고, 그 중심이 적의 발 위치인지 카드 외곽인지도 역할에 맞춰 대조해야 한다. 스폰 anchor는 길 위의 정확한 높이·여유 반경을 가진 별도 저작 지점이다. 기존 상자의 ID/좌표를 유지하고, 실제 사용할 위치만 명시적으로 참조하거나 anchor로 작성한다.

### MapTool에서 필요한 저작 항목

1. 참가자 시작 위치 4개와 중앙 안전 구역.
2. 중앙 망원경 상호작용 위치·범위. 한 번 사라지는 공유 아이템으로 만들지 않고, 첫 담당자와 후속 완료자가 각자 사용 가능한 규칙으로 연결한다.
3. **문양 몬스터 후보 지점**: 통로별 anchor 목록. 생성 위치는 이 목록에서 Server가 고른다.
4. **행진 route**: 출발 anchor, 통로를 따라가는 waypoint, 도착/퇴장 anchor. 동·서·남·북은 카메라 회전과 독립적인 미로 기준으로 저장한다.
5. 완료자가 중앙으로 진입하는 구간과 전체 성공 후 복귀 목적지.
6. 망원경 전체보기 샷: 기존 카메라 샷 문서에 이름과 eye/lookAt/FOV/전환 시간을 저작한다.

검사는 시작 위치→후보 지점→중앙→출구의 연결성, 각 route의 통과 폭·높이, 벽을 통과하는 지름길 여부까지 포함한다. point 하나의 walkable 값으로 통과 가능 판정을 끝내지 않는다. 특히 4m grid가 좁은 통로를 구분하지 못하면 현재 상세 region 기능으로 **카드미로만** 보완하는 방안을 선택한다. 전체 맵 재베이크가 기본 절차는 아니며 이번에는 어떠한 네비 변경도 하지 않았다.

행진의 화면 방향은 저작된 미로 기준의 3/6/9/12시로 표시한다. 현재 카메라의 오른쪽 벡터로 매 프레임 출발 방향을 바꾸지 않는다. 중앙 안전 구역을 가로지르는 route가 있다면 병사의 경로와 중앙 피해 무효를 함께 검증한다. 안전 구역을 물리벽으로 만들어 병사를 쌓이게 해서는 안 된다.

## G02. 서버가 소유할 미로 진행 상태

### 책임과 수명

기존 `CGameRoom`이 미로 실행 상태를 소유한다. 미로 규칙은 쿠크 전용의 한 의미 단위로 분리하고, 별도 월드·두 번째 몬스터 저장소·Client 전용 가짜 HP는 만들지 않는다. 제품 쿠크는 shared world이므로 참가자는 같은 방 전체를 무조건 사용하지 않고 **시작 승인된 파티의 4명**으로 고정한다. 최초 버전은 같은 미로 공간에서 동시 실행 1개만 허용한다는 설계안을 권장한다.

| 상태 | 의미 | 다음 전이 |
|---|---|---|
| 비활성 | 미로가 진행 중이 아님 | 승인된 시작 명령 |
| 준비 | 참가자, 배포된 gameplay 정의, 위치, 초기 몬스터를 검사·stage | 전체 준비 성공 |
| 첫 타격 대기 | 망원경 담당 1명과 처치 담당 3명 확정, 문양 표시·목표 생성, 행진은 아직 없음 | 유효한 목표 타격 |
| 진행 | 방향 순서대로 행진, 개인 처치 집계, 완료자 관전 합류 | 세 명의 목표 완료 및 성공 조건 충족 |
| 성공 | 더 이상 행진 생성 안 함, 해당 실행의 객체 정리·복귀 | 비활성 |
| 중단 | 실패 이유 보존, 해당 실행의 입력/카메라/객체 정리 | 비활성 또는 명시적인 재시작 |

개인 상태는 역할, 배정 문양, 처치 수, 목표 완료 여부, 현재 안전 구역 안인지, 관전 권한, 관전 중인지를 나눠 보관한다. `3/3`과 `중앙 도착`과 `관전 중`을 같은 bool로 합치지 않는다.

실행 식별자는 Server가 발급한 `runId`, 참가자 식별자는 `PlayerId`다. 각 목표와 행진 entity는 자신의 `runId`와 역할을 가진다. 이전 실행에서 날아온 입력·hit·exit 결과는 새 실행에 반영하지 않는다. 모델 asset path, ImGui list index, nickname을 권위 식별자로 쓰지 않는다.

초기 망원경 획득은 Server가 유효한 첫 사용자 한 명에게 담당자 권한을 확정하고, 동일 tick의 중복 획득은 같은 역할을 두 명에게 주지 않는다. 후속 완료자는 최초 담당자 자리와 별도로 관전 권한을 얻는다. 1명만 관전 가능한 전역 bool을 두면 사용자 요구를 충족하지 못한다.

저작 데이터 로드는 `parse → validate → stage → commit`이다. Server 시작 준비에서도 참가자·몬스터 profile·생성 지점과 예약 생성을 먼저 검증하고, 실패하면 기존 위치·역할·게임 진행을 보존한다. 참여/시작 요청의 성공·거절 이유는 요청자에게 회신한다.

모델 파일·텍스처·clip 검사는 저작/publish와 Client presentation 준비의 책임이다. Server가 Resources 경로나 CModel을 읽어 준비 여부를 판정하지 않는다. 모든 참가자의 표현 준비까지 시작 조건으로 삼는다면, 별도의 준비 단계와 제한 시간 안에서 stable run/정의 ID에 대한 Client 준비 결과만 받고 Server가 시작을 확정하는 계약이 추가로 필요하다. 기존에 그런 4인 준비 응답이 있다는 뜻은 아니다. 준비 실패 시 서버 게임 상태를 먼저 바꾸는 부분 시작은 허용하지 않는다.

## G03. 두 몬스터를 기존 생성 경로로 연결

### 정의와 실제 생성

`MonsterCatalog`는 모델·재질·idle/run/attack/hit/dead 표현을, `MonsterProfiles`는 HP·속도·피격 반경·공격 수치를 소유한다. 목표와 행진을 별도 archetype/역할로 등록하되 모델이 같은 경우에는 동일 asset ID를 재사용할 수 있다. 새 `CMonster`나 World Object 전용 전투 클래스를 만들지 않는다.

실제 생성·복제는 `CSpawnGroupRuntime → CGameRoom::Spawn_Monster → SERVER_WORLD_ENTITY → spawn/snapshot/despawn → CClientReplication → CMonsterPresentationAssetService → CNpc/CModel`을 유지한다. `CNpc`를 사용하는 것은 기존 몬스터 presentation 구현을 재사용한다는 뜻이지 Server kind를 NPC로 위장한다는 뜻이 아니다.

### 문양 목표

현재 SpawnGroup entry는 `anchorId` 하나만 가진다. 따라서 랜덤 후보 풀은 기존 기능이 아니며 저작 parser·publisher·Server bootstrap/runtime을 함께 확장해야 한다.

제안 계약은 entry의 위치 선택을 `고정 anchor` 또는 `명시적인 후보 anchor 목록` 중 정확히 하나로 저장하는 것이다. 좌표는 anchor 한 곳에만 보관한다. Server가 후보 순서를 섞고 다음 조건을 충족하는 지점을 선택한다.

- 같은 미로 구역의 실제 보행면이고, 투영 후에도 원래 통로에서 벗어나지 않는다.
- 해당 참가자가 도달할 수 있다. 벽 너머의 가까운 점으로 스냅된 결과를 승인하지 않는다.
- 중앙 안전 구역과 겹치지 않는다.
- 플레이어 몸체·다른 적·예약된 같은 tick 스폰과 겹치지 않는다.
- 후보를 다 검사했는데 자리가 없으면 이유를 표시하고 제한된 재시도 또는 준비 실패로 처리한다. 무한 랜덤 재추첨하지 않는다.

첫 구현의 권장 게임 규칙은 **세 명에게 서로 다른 문양을 배정하고, 각 문양의 목표 3마리를 중복 없는 후보 위치에 배치**하는 것이다. 네 문양 중 무엇을 쓸지는 설정으로 고른다. 이것은 진행이 막히지 않는 제안이지 원본 게임의 확정 규칙은 아니다. 목표를 처음부터 모두 낼지 순차 보충할지는 설정 정책으로 분리한다. 순차 보충이라면 `남은 필요 수 - 살아 있는 해당 목표 수 - 예약 스폰 수`로만 보충하여 무한 증식을 막는다.

목표를 정지시킬지 배회시킬지는 별도 behavior다. 랜덤 생성이라는 사용자 요구를 자동으로 플레이어 추적 AI로 바꾸지 않는다. 기존 `CMonsterBrain` 기본 추적을 그대로 적용하면 목표들이 한 사람에게 몰려 미로 탐색이 사라진다.

### 행진 병사

행진 병사는 플레이어를 추격하는 AI가 아니라 **저장한 통로를 따라 끝까지 이동하는 behavior**를 기존 MonsterBrain 경로에 추가한다. Server navigation과 collision을 소비하고, Client는 승인 위치와 애니메이션만 보간한다. 달리기 모션의 root 이동과 Server 위치를 동시에 누적하지 않는다.

| 순번 | 출발 측 | 진행·퇴장 측 |
|---|---|---|
| 0 | 3시 | 9시 |
| 1 | 6시 | 12시 |
| 2 | 9시 | 3시 |
| 3 | 12시 | 6시 |

위 순서를 한 주기로 반복한다. route별 발사 간격, 한 줄의 병사 수, 줄 간격, 이동 속도, 최대 생존 수는 저작 수치다. 이번 조사로 원본 값이 확정된 것이 아니므로 임의의 초/속도를 원본값으로 기록하지 않는다.

현재 SpawnGroup `REPEAT`는 기존 적이 모두 사라진 다음에 다시 시작한다. 이 기능만 사용하면 연속 행진의 방향 주기와 어긋날 수 있다. 고정 주기 생성 정책을 기존 SpawnGroup runtime에 명시적으로 추가하고, 그 runtime만 생성 시계를 소유하게 한다. 미로 진행 상태는 첫 타격 때 해당 group을 활성화하고 성공/중단 때 중지한다. 양쪽에서 각각 병사를 생성하는 이중 타이머는 만들지 않는다.

최초 3시 방향은 첫 유효 타격이 처리된 fixed tick의 생성 단계에서 시작한다. 후속 방향은 Server tick deadline을 사용한다. 실수 시간 매 프레임 누적이나 Client 시간으로 판단하지 않는다. 생성 제한에 걸리면 무한 밀린 생성을 한 프레임에 몰아내지 않고 제한 사유를 남긴다. 끝에 도착한 entity는 퇴장 처리하되 목표 처치로 집계하지 않는다.

병사-병사 끼임과 목표 몸체에 가로막히는 경우를 검사한다. 필요하면 미로 역할 사이에 한정된 충돌 정책을 명시한다. 이를 해결하려고 모든 몬스터의 collision을 끄지 않는다. 행진 피해는 전후 tick 이동 구간의 swept contact로 판정하여 빠른 병사가 플레이어를 건너뛰어도 접촉을 놓치지 않게 한다.

## G04. 망치 타격, 첫 타격 시작, 개인 처치 집계

### 기존 입력 연결점

`CPlayerController::Poll_SkillSlots()`는 MAZE 등 상호작용 HUD에서 일반 class skill 대신 `IPlayerCommandSink::Request_InteractionSlot()`을 호출한다. Server `Handle_InteractionSlot()`이 슬롯, 중복 sequence, HP, action, 쿨타임을 검사한다. 이 흐름을 유지한다.

UI JSON의 `56411`은 현재 미로 뿅망치 표시 ID다. 이 값이 `PlayerSkills.json`의 제품 damage 계약으로 등록되어 있는 것은 아니다. 아이콘이 있다는 이유로 기존 class skill 호출에 숫자만 넣지 않는다. 미로 전용 공격의 timing/shape/대상 정책을 Server가 읽는 저작 계약에 추가하고 기존 typed hit resolver로 HP를 처리한다.

권장 흐름은 다음과 같다.

1. Server가 미로 참가자·처치 역할·유효한 망치 입력을 승인한다.
2. action 시작 tick과 Server yaw를 기준으로 공격 발생 시각과 판정 영역을 계산한다. Client bone 위치를 피해 정답으로 보내지 않는다.
3. 같은 run의 목표만 판정하고, 한 공격 occurrence에서 같은 entity를 한 번만 맞춘다.
4. `CServerCombatHitRuntime::Apply_PlayerToWorld()`의 결과와 실제 HP 감소량을 즉시 미로 규칙에 전달한다.
5. 처음 승인된 목표 타격이면 행진 시작을 한 번 확정한다. 한 방에 죽여도 최초 타격과 처치 모두 처리한다.
6. `KILLED`이며 아직 집계하지 않은 entity일 때만 처치자에게 1을 더한다.
7. 세 번째 처치에서 목표 완료와 관전 자격을 복제한다. 아직 중앙 밖이면 자동 무적이나 자동 순간이동을 주지 않는다.

`DAMAGE_EVENT`를 나중에 뒤져서 처치자를 추정하지 않는다. 현재 event에는 target/amount/position/outgoing이 있지만 sourcePlayerId가 없고 event 수에도 상한이 있다. 기존 hit 입력에는 sourcePlayerId가 있고 반환값은 `NOT_ADMITTED/ABSORBED/LANDED/KILLED`이므로, 그 성공 결과에서 의미 있는 미로 hit/kill 통지를 생성한다. 일반 스킬·투사체 경로도 같은 정책을 통과시키거나 미로에서 명시적으로 차단하여 우회 처치를 방지한다.

처치 숫자는 매 프레임 `HP==0`을 세지 않는다. 죽은 모델은 일정 시간 남아 있으므로 그렇게 구현하면 1마리가 여러 번 집계된다. `runId + NetEntityId`로 한 번만 인정하고, 목표 역할이 아닌 행진 병사·세토·소품 despawn은 제외한다.

**권장 오답 정책:** 다른 문양 목표에는 피해/진행을 주지 않고 안내만 표시한다. 이렇게 하면 다른 사람이 목표를 소진해서 해당 문양 담당자가 진행 불가능해지는 일이 없다. 이는 사용자에게서 확정받은 원본 규칙이 아니라 초기 구현안이며, 오답 피해·패널티를 원하면 별도 명시해야 한다. 첫 타격 시작도 이 제안에서는 '자기 문양에 대한 유효한 타격'이다.

## G05. 중앙 무적과 망원경 시점

### 상호작용

기존 `Request_InteractTrigger → C2S_INTERACT_TRIGGER → CGameRoom::Handle_InteractTrigger → CServerTriggerSystem::Activate_Interact`를 확장한다. 중앙 상자는 해당 플레이어에게만 offer를 주고, 요청 시 동일 run/참가자/살아 있음/실제 범위/사용 자격을 다시 검사한다. 카메라 Play 명령을 방 전체에 보내는 방식으로 망원경을 구현하지 않는다.

현재 trigger action callback은 종류와 targetId 중심이고 망원경 role/state를 변경하는 전용 action은 없다. caller가 이미 알고 있는 playerId를 실제 소비자까지 전달하는 typed action을 추가해야 한다. UI에서 player 좌표를 중앙으로 고치거나 권한 bool을 직접 켜지 않는다.

기존 `ServerTriggerSystem::Evaluate_Entries()`는 상자에 새로 들어온 순간에만 사용 안내를 제공하고, `Handle_InteractTrigger()`는 사용 후 안내를 철회한다. 망원경에는 사용 자격 변경과 관전 종료 시에도 Server가 현재 범위·생존·역할을 재검사해 안내를 다시 제공하는 처리가 필요하다. 중앙 안에서 관전을 껐다가 다시 켜거나, 중앙에 있는 동안 사용 자격을 얻었을 때 밖으로 나갔다 들어와야만 하는 상황을 방지한다. 관전 중·권한 상실·미로 종료 때는 안내를 철회하고 같은 안내를 매 tick 중복 송신하지 않는다.

### 안전 구역

권장 규칙은 최초 망원경 담당자 또는 목표 완료자가 중앙 구역 **안에 있을 때** 보호받고, 관전 중에는 이동/공격을 잠그는 것이다. 미완료자는 중앙에서 공격만 하며 버티는 우회를 허용하지 않는 안이다. 안전 구역의 미완료자 보호 여부와 보호 범위는 사용자 결정 대상이며 원본 규칙으로 단정하지 않는다.

보호는 Server hit admission 전에 적용하여 HP 감소뿐 아니라 knockback/knockdown도 막는다. 일반 공격, 미로 접촉, 별도 기믹 피해 등 실제 피해 경로를 점검한다. Client에서 HP를 되돌리는 보정은 사용하지 않는다. 추락·실패·퇴장까지 무효화하는 무제한 invincible flag와 구분한다.

동일 tick에 중앙으로 들어오는 경우에는 이동 확정 위치 기준으로 보호 여부를 검사한 뒤 위험물 접촉/피해를 평가한다. 이전 프레임의 중앙 여부를 그대로 사용하지 않는다. Server의 판정 기준과 Client 표시 기준은 같은 상태 snapshot을 소비한다.

### 카메라와 표시

Server는 `관전 허용/관전 중`이라는 의미 상태만 결정한다. eye/lookAt/FOV는 Client가 기존 `camerashots.json`에서 가져온다. 기존 `Find_ActiveCameraShot()`의 현재 box/sequence 조건에 '본인 미로 관전 상태' 조건을 연결하여 중앙에 서 있기만 한 모든 사람의 카메라가 올라가지 않게 한다.

- 관전 입장: 현재 player follow pose에서 미로 전체보기 샷으로 blend.
- 관전 유지: 중앙 기준의 고정 맵 시야. 플레이어가 좌우로 움직일 때마다 카메라 기준을 바꾸지 않는다.
- 나머지 세 명: 기존 보행 시점과 입력 유지. Mario의 좌우 일직선 제약은 적용하지 않는다.
- 관전 종료: 본인 현재 follow pose로 복귀. 시작 당시 저장한 다른 구역 카메라로 돌아가지 않는다.
- 강제 종료: 미로 종료, disconnect, Level 종료, 권한 상실 시 카메라 owner와 입력 잠금을 반드시 정리한다.
- 기존 F6 free camera와 별개다. F6가 관전 권한을 만들거나 서버 진행 상태를 변경하지 않는다.

전체보기는 기존 월드 객체를 같은 카메라로 보는 것이 기본이다. 별도 RenderTexture 미니맵을 새로 만들 필요는 없다. 상단 시점에서 outer card가 가리는 문제나 화면 비율에 따른 잘림은 사용자가 조정할 카메라/표현 검증 항목으로 남긴다.

발밑 문양은 미로 전용 presentation을 연결한다. 현재 머리 위 룰렛 카드 Effect를 위치만 임의로 낮추고 정답이라고 하지 않는다. 같은 `MECHANIC_CARD_SYMBOL` enum은 재사용할 수 있지만 미로 배정 상태와 룰렛 상태의 owner/수명은 분리하여 룰렛의 clear가 미로 문양을 지우지 않게 한다.

## G06. 정본 파일과 변경 연결점

아래 표는 수정 대상 책임을 설명하는 설계이며, 이번에 실제 생성·변경한 C++ 목록이 아니다. 경로 기준 root는 `C:/Users/USER/source/졸업팀폴/LostArk`다.

| 기존 파일 | 필요한 확장 책임 |
|---|---|
| `Data/Actors/MonsterCatalog.json` | 확인된 목표/행진 모델과 실제 clip의 Client 표현 등록 |
| `Data/Balance/MonsterProfiles.json` | 몬스터 HP·속도·피격/공격 수치 |
| `Data/Worlds/LV_LUT_MIDNIGHTC_ED/SpawnGroups.world.json` | 후보 anchor, 고정 route 지점, 두 그룹, 고정 주기·위치 선택 정책 |
| `Data/Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json` | 시작·망원경·중앙 도착·퇴장 trigger 연결. 기존 Miro 초안 보존 |
| `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.camerashots.json` | 전체보기 샷, 복귀 blend, 미로 관전용 활성 조건 |
| `Client/Public/SpawnGroupDocument.h`, `Client/Private/SpawnGroupDocument.cpp` | 랜덤 anchor 선택과 고정 주기 저작 parse/validate/save |
| `Client/Private/MapTool.cpp`, `Client/Public/MapTool.h` | 미로 배치·설정 UI. Server 상태 직접 수정 금지 |
| `Tools/WorldPipeline/Publish-WorldGameplay.ps1` | 신규 저작 필드와 미로 참조 검증·원자 배포 |
| `Server/Public/SpawnGroupBootstrap.h`, `Server/Private/SpawnGroupBootstrap.cpp` | 같은 스키마의 Server 생성물 읽기 |
| `Server/Public/SpawnGroupRuntime.h`, `Server/Private/SpawnGroupRuntime.cpp` | 랜덤 위치 선택, 고정 주기, 실행별 취소/종료 |
| `Server/Public/MonsterBrain.h`, `Server/Private/MonsterBrain.cpp` | 목표용 동작과 경로 행진. 기존 일반 추적 보존 |
| `Server/Public/GameRoom.h`, `Server/Private/GameRoom.cpp` | 실행/참가자 수명, 명령 검증, 기존 entity 생성·복제·정리 연결 |
| `Server/Public/ServerTriggerSystem.h`, `Server/Private/ServerTriggerSystem.cpp` | 플레이어별 망원경 자격 검사, 관전 종료·자격 변경 시 사용 안내 재제공 |
| `Server/Public/ServerCombatHitRuntime.h`, `Server/Private/ServerCombatHitRuntime.cpp` | 타격 결과 통지 연결, 보호/역할 hit admission |
| `Server/Private/PlayerSkillSystem.cpp`, 기존 combat-object hit 호출자 | 공통 판정 정책으로 우회 없이 연결 |
| `Client/Public/PlayerCommandSink.h`, `Client/Private/PlayerController.cpp`, 기존 Network sink | 상호작용·관전 종료·디버그 명령 전달 |
| `Shared/Public/Network/PacketMessages.h`, `PacketType.h`, 대응 codec, ServerApp queue | 미로 상태/결과·명령 versioned wire와 malformed 거절 |
| `Client/Private/ClientReplication.cpp`, `Level_KakulSaydonArena.cpp`, `CombatHUDViewModel.cpp` | 참가자 상태 수신, 본인 카메라·입력, 문양과 0/3~3/3 표시 |
| `Client/Private/MainApp_SequenceViewer.cpp`, `Data/Maps/SequenceViewer.labels.json` | F1에서 한글 이름·구역·원인 표시 및 기존 실행 경로 연결 |

미로 고유 규칙은 선택적 새 저작 문서 `Data/Worlds/LV_LUT_MIDNIGHTC_ED/CardMaze.world.json`을 추가하는 안을 권장한다. 여기에 참가 인원/역할, 목표 수, 기존 group/trigger/shot 참조, 문양 정책, 성공·오답·보호 정책 및 미로 망치 판정을 둔다. 좌표·모델 경로·몬스터 HP·행진 wave 수치를 복제하지 않는다. 미로 망치 값은 기존 UI 쿨타임/입력 상수와 두 정본이 되지 않게 Server 저작값을 승인 snapshot 및 HUD에 투영하는 계약으로 함께 전환한다.

새 문서는 **현재 parser가 읽지 않는다.** 소비자와 publisher를 함께 구현할 때만 파일을 추가한다. Server의 미로 규칙을 별도 `.h/.cpp`로 분리한다면 실제 GameRoom 소유·호출을 먼저 확정하고 `Server/Default/Server.vcxproj`와 `.filters`에 필요한 `ClInclude/ClCompile`만 등록한다. Client에 새 C++ 파일이 생기는 경우도 동일하다. 새 Data 원본은 Client 프로젝트 `96.DataFiles`의 `None`으로 등록하고 Bin에 수동 복제하지 않는다.

Shared에는 한 번의 연출 이벤트만 보내지 않고 현재 `runId/phase`, 참가자별 `role/symbol/killCount/completed/observing`를 복원 가능한 상태로 보낸다. 개인 문양 공개 범위는 게임 규칙에 맞게 나누고, 관전자가 필요한 정보는 Server가 정한 범위에서 제공한다. entity 자체의 HP/위치/모션은 기존 world snapshot을 재사용한다. 새 프로토콜은 구현 시점의 현재 버전에서 일괄 증가시키며 Server/Client/codec이 서로 다른 선언을 쓰지 않게 한다.

## G07. 누구나 조정할 수 있는 도구 구성

MapTool의 쿠크 `World Gameplay` 아래 **Card Maze / 카드미로** 묶음을 추가하는 안이다. 버튼·필드 제목은 짧은 영어, 기능 설명·목록 별명은 한국어를 지원한다. stable ID는 별명 변경과 독립적이다.

| 묶음 | 표시·수정할 내용 |
|---|---|
| Overview | 미로 ID, 데이터 상태, 누락 리소스·끊긴 참조, 4인 시작 조건 |
| Players | 4개 시작 위치, 망원경 담당/처치 담당 구분 |
| Symbol Targets | 문양별 실제 모델, 목표 수, 통로 후보 지점 목록과 지도 표시 |
| Marching Soldiers | 방향 순서 4개, route/출발·퇴장, 간격·수·속도·최대 생존 수 |
| Safe Zone | 중앙 범위와 보호 자격, 망원경 사용 범위 |
| Telescope Camera | 기존 카메라 샷 선택·편집, 본인 입장/복귀 미리보기 |
| Validation | 네비 접근 불가, 모델/clip 누락, 후보 부족, route 단절, 잘못된 문양/참조 |

F1에서는 쿠크 탭에 '카드미로 시작', '전체보기 카메라', '현재 진행 상태'를 식별하기 쉬운 이름으로 제공한다. 테스트용 강제 첫 타격/방향별 생성/개인 완료는 Debug 전용 Server 명령으로 만들고 정상 진행과 구분해 표시한다. 다른 Area에서는 목록을 보여주되 서버 실행은 해당 아레나 승인 입장 뒤에 수행한다. Test의 카메라/모델 Preview가 실제 4인 진행 성공을 뜻하지 않게 표시한다.

기존 Sequence Viewer `Stop`은 연출 정지이지 몬스터·개인 목표·무적·카메라를 모두 초기화하는 기능이 아니다. **미로 Reset은 별도 명시적 동작**으로 제공하고 영향받는 참가자를 표시한다. 실행 중 다른 사람의 진행을 단순 시퀀스 Replay로 초기화하지 않는다.

## G08. 성공, 실패, 검증 순서

성공 판정의 권장안은 '처치 담당 3명이 각각 3/3을 달성하고 중앙 도착'이다. 완료자가 중앙으로 이동한다는 사용자 요구에 맞춘 제안이며, 마지막 처치 즉시 종료를 원하면 조건을 바꾼다. 초기 망원경 담당자는 처치 목표가 없다.

사망/연결 종료/파티 이탈에 대한 최초 구현안은 이유를 남기고 **해당 미로 실행만 중단·정리**하는 것이다. 자동 역할 교체나 미확인 재접속 복구를 구현된 기능처럼 약속하지 않는다. 같은 실행 재접속을 지원할 경우 새 session과 기존 PlayerId의 안전한 재결합, 전체 상태 재송신, 관전 owner 복구까지 별도 검증해야 한다.

정리 시 순서는 생성 중지 → 미로 전용 예약/타격 무효화 → 해당 run entity despawn → 참가자 보호·관전·입력 상태 해제 → 살아 있는 플레이어의 검증된 복귀 → 완료/중단 결과 전달이다. 다른 쿠크 구역의 보스·사용자 저작·다른 파티 state를 지우지 않는다. 이전 HP를 저장해 되돌리거나 사망자를 조용히 부활시키지 않는다.

### 구현을 시작할 때의 순서

1. PPCH 모델의 실제 전달 위치/정확한 배역을 확정한다. WModel·texture·idle/run/hit/dead·공격 clip이 실제로 읽히는지 검사한다. 세토는 사용자가 원한 추가 배역인지 확인한 뒤에만 넣는다.
2. 미로 후보 지점과 모든 행진 route의 Server 보행·충돌을 확인한다. 필요한 구간만 저작 보완한다.
3. 미로 전용 대상 1마리를 기존 MonsterCatalog/SpawnGroup으로 생성하고 망치 입력으로 실제 Server HP 감소·사망을 확인한다.
4. 4인 참가·문양 배정·개인 3마리 집계를 연결한다. 중복 hit/죽음과 다른 문양 처치를 검증한다.
5. 첫 유효 타격의 동일 tick에 3시 방향을 활성화하고 4방향 주기·퇴장·생성 상한을 연결한다.
6. 중앙 보호와 망원경 본인 전용 카메라·후속 완료자 합류를 연결한다.
7. 전원 성공·중단·재시작·F1 상태 확인을 연결한다. 사용자 4대 화면 검증 후 실제 플레이 완료로 판정한다.

### 해당 기능에서 확인할 항목

- 첫 타격 전 행진 0마리, 첫 유효 타격/즉사 타격 후 시작 1회, 동시 타격에도 중복 활성화 없음.
- 각 문양 목표가 접근 가능한 통로에 생성됨. 후보 부족 시 명확한 실패와 준비 전 상태 보존.
- 목표 1마리 사망은 1회만 집계. 죽은 모델 유지·despawn·연출 Stop으로 추가 점수 없음.
- 다른 문양/다른 역할/다른 run/비참가자의 입력은 정책대로 거절.
- 순서가 3→6→9→12시 출발로 반복되고 상대편에 퇴장. 빠른 접촉, 벽, 교차 병사, 중앙 통과 검증.
- 최초 담당자만 처음부터 전체보기. 3/3 완료자가 중앙에서 사용하면 관전자 수가 늘어도 다른 플레이어 카메라에 영향 없음.
- 중앙 안에서 관전 종료 후 다시 사용 가능. 자격 획득/상실 때 안내 갱신, 중복 안내·중복 획득 없음.
- 중앙 진입 tick과 피해 tick의 순서, 관전 중 서버 이동·공격 거절, 종료 후 정상 입력 복원.
- 잘못된 JSON/참조/클립, 중복 명령, stale runId, 네트워크 지연·종료에서 부분 commit/무한 대기 없음.
- 저장/재로드·관련 publisher parse/validate, 변경 Shared/Server/Client 최소 컴파일·기존 focused 검사, `git diff --check`. 실제 Client 화면은 사용자 확인.

### 이번 조사에서 실제 확인한 범위

코드와 JSON/원본 DB 읽기, 등록 수와 모델 경로 검색, 설치 세토 WModel info, 배포 navgrid header와 지정 5지점 cell 읽기를 수행했다. 구현·publish·빌드·Server 게임 실행·Client/UI 실행·화면 검증은 하지 않았다. 이번 설계가 작성됐다는 이유로 카드미로 구현이 완료되거나 필요한 모델이 모두 준비됐다고 기록하지 않는다.

설계 독립 검토에서 나온 망원경 안내 재제공 누락은 실제 `ServerTriggerSystem.cpp`와 `GameRoom.cpp`에서 재확인하여 반영했다. 기존 파일 경로와 UTF-8을 확인했고, 새 문서의 `git diff --no-index --check` 및 작업 트리 `git diff --check`에서 whitespace 오류는 없었다. C++/JSON/XML 변경이 없어 이번 문서 작업에 대한 컴파일·변경 데이터 parse는 대상이 아니다.

### 구현 확정 전에 정할 게임 규칙

사용자 요구 밖의 값을 원본 사실로 채우지 않았다. 다음은 본문의 권장안을 채택할지 결정할 항목이다: 서로 다른 문양 3개 배정, 최초 목표 일괄 생성 또는 순차 보충, 오답 무효/패널티, 미완료자의 중앙 보호 여부, 행진 접촉 피해/밀침과 생존 여부, 마지막 처치 또는 중앙 도착 성공, 사망/이탈 중단 정책. 간격·속도·카메라 수치는 저작 값으로 조정한다. 이 선택과 실제 모델 대응이 확정되면 전체 반영 코드 PLAN과 데이터 변경을 작성한다.
