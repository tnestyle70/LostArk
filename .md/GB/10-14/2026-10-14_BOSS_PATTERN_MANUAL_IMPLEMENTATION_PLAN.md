# 보스 패턴 직접 구현 안내 — 구조 분리 이후

2026-10-14. **G0 구조 분리는 소스에 반영됐고, G1 이후 패턴 코드는 수동 구현 후보다.**
사용자가 편집 중임을 확인해 추가 빌드·계약 검사를 중단했다. 편집 정리 후 최신 코드로 전체 빌드와 회귀를 재개한다.

| 순서 | 읽을 문서 | 현재 상태 |
|---|---|---|
| G0 파일 구조 | [실제 변경·검증 RESULT](2026-10-14_BOSS_SIMULATION_RESPONSIBILITY_SPLIT_RESULT.md), [전체 코드 PLAN](2026-10-14_BOSS_SIMULATION_RESPONSIBILITY_SPLIT_PLAN.md) | 제품 소스 반영. Engine·Shared·Server 빌드 통과, Client 편집 중 오류로 전체 빌드 미완료 |
| G1 진짜 세이튼 / G2 조커 | [교체 블록·전체 C++](2026-10-14_KOUKU_RANDOM_MANUAL_IMPLEMENTATION_PLAN.md) | 네 자리 본체 선택과 일곱 카드 배치/접촉의 공통 이동값. 제품 미적용 |
| G3 잡기 / G4 피자 망치 | [손 추적 후보·실제 Room 재현](2026-10-14_KOUKU_COLLIDER_MANUAL_IMPLEMENTATION_PLAN.md) | 잡기 위치 불일치의 후보. 현재 피자 bundle은 정상 생존 상태에서 피해·넉백 확인 |
| G5 발탄 포탈 | [4회 연결 JSON·방향 근거](2026-10-14_VALTAN_PORTAL_MANUAL_IMPLEMENTATION_PLAN.md) | 4회 연결 미적용 후보. 원본 carrier route anchor 확장은 아직 구현 전 |

G1부터 직접 작성할 때는 파일의 전체 전문을 다시 덮어쓰기보다 문서의 정확한 기준 블록을 찾아 추가·교체한다.
G3 문서의 LogicRuntime 전체 파일은 G2와 손 추적을 누적 병합한 후보다. 각 후보는 개별 컴파일했지만 누적본 새 빌드는 사용자 요청으로 아직 하지 않았다.
원본 hash와 별도 후보를 `out/BossSimulationRefactor20261014`에 보존했고, 위 PLAN에는 필요한 전체 코드와 데이터 블록을 담았다.

구조 정리의 상태는 RESULT, 아직 제품에 반영하지 않은 패턴 코드는 각 PLAN이 정본이다.
새로운 Q/W/E/R/A/S 및 빙고 Space 기믹은 포함하지 않는다.

## 쿠크 Collider 저작부터 서버 피해·넉백까지

2026-10-14 현재 소스를 읽어 정리한 학습 메모다. 이 문서 작성으로 제품 코드·데이터를 수정하지 않았다.
사용자가 직접 구현할 때 현재 함수의 책임과 입력을 찾기 위한 안내이며, 실제 플레이 검증 결과를 뜻하지 않는다.
기존 상세 조사와 사례는 [10-12 학습 기록](C:/Users/tnest/Desktop/LostArk/.md/GB/10-12/2026-10-12_KOUKU_COLLIDER_AND_RANDOM_PATTERN_LEARNING_RESULT.md)을 참고한다. 아래는 분리 후 현재 파일 위치를 우선한다.

## 1. 전체 연결

`ImGui 편집 → Composition 저장 → Encounter projection → bootstrap 게시 → GameplayCatalog typed 정의 → Brain/LogicRuntime → 서버 HP·이동 → 복제` 순서다.
**Brain은 Composition이나 Encounter JSON을 직접 읽지 않는다.** 이미 파싱·검증한 `BOSS_PATTERN_DEFINITION`을 받아 패턴과 단계 상태를 진행한다.

| 단계 | 현재 진입점 | 이 단계가 책임지는 것 |
|---|---|---|
| Collider 정의 생성 | [Presentation.cpp:914](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench_Presentation.cpp:914) `Create_PresentationResource` | 재사용할 shape·크기 등의 정의와 stable resource ID를 후보에 추가한다. |
| 타임라인 배치 | [Presentation.cpp:1010](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench_Presentation.cpp:1010) `Append_PresentationCandidate` | 특정 패턴의 occurrence ID, 시작·지속 시간, 정의 참조를 만든다. |
| 저작 저장 | [Document.cpp:202](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench_Document.cpp:202) `Save` | 후보를 검증하고 `Save_Atomic`으로 현재 디스크 저장본과 병합한다. |
| 실행 정의 투영 | [projector.py:5000](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:5000) `project_encounter` | 저작 정의·배치를 결합해 실행용 논리 창·판정 영역·결과로 변환한다. |
| 서버 행 게시 | [KoukuBootstrapRows.ps1:708](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/KoukuBootstrapRows.ps1:708) `Add-KoukuBootstrapRows` | Encounter의 구조를 서버가 읽는 typed bootstrap 행으로 내보낸다. |
| 서버 파싱 | [GameplayCatalog.cpp:1773](C:/Users/tnest/Desktop/LostArk/Server/Private/GameplayCatalog.cpp:1773) `Load_BootstrapBytes` | 행의 enum·숫자·참조를 검증하고 typed 정의를 구성한다. |
| 패턴 진행 | [KoukuSaydonBrain.cpp:1258](C:/Users/tnest/Desktop/LostArk/Server/Private/KoukuSaydonBrain.cpp:1258) `Begin_Pattern` | 유효한 패턴 정의로 보스의 실행 상태를 시작한다. |
| 판정·결과 실행 | [LogicRuntime.cpp:1750](C:/Users/tnest/Desktop/LostArk/Server/Private/KoukuSaydonLogicRuntime.cpp:1750) `Update` | 현재 tick의 논리 창과 플레이어 상태로 접촉·성공·실패·피해 결과를 처리한다. |

## 2. ImGui에서 생성한 Collider가 저장되는 방식

Collider resource는 재사용하는 모양이고, presentation occurrence는 그 모양을 특정 시간·위치에 사용하는 배치다.
[발생 인스턴스 구조체](C:/Users/tnest/Desktop/LostArk/Client/Public/KoukuSaydonCompositionDocument.h:515)는 resource 참조와 TRS, anchor, bone, 논리 연결을 함께 보관한다.
버튼을 눌러 표시되는 Collider만 생성했다고 서버 피해가 생기지는 않는다. 판정할 Logic occurrence와 결과를 연결해야 한다.

- [Set_ColliderLogicValues:303](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench_Collider.cpp:303)는 논리 정의·시간 구간을 검증하고 Collider의 `logicOccurrenceId`를 연결한다.
- [Set_ColliderTriggerDamage:456](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench_Collider.cpp:456)는 접촉 피해용 `ENTER_AREA` 논리와 피해 결과를 구성한다.
- [Render_ColliderBoxDetails의 호출:877](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench_Collider.cpp:877)에서 공용 `Render_PresentationAnchor`를 Collider 모드로 사용한다.
- [Render_PresentationAnchor:2269](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench_Presentation.cpp:2269)는 BOSS·MAP·WORLD 선택, BODY/WEAPON, bone, follow, 회전 기준을 편집한다.
- [bone 회전 선택:2491](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench_Presentation.cpp:2491)의 `BONE`과 `TARGET_YAW`는 본의 회전까지 따를지, 대상 yaw 기준을 사용할지 구분한다.

현재 저작 정본은 [KoukuSaydonComposition.json](C:/Users/tnest/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json)이다.
[Read_PresentationOccurrence:1396](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonCompositionDocument.cpp:1396)가 `bone`, `boneTarget`, `boneRotation`, `anchorKind`, `logicOccurrenceId` 등을 다시 읽는다.
[직렬화 구간:7229](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonCompositionDocument.cpp:7229)은 같은 follow·bone·anchor 필드를 JSON으로 저장한다.
[Save_Atomic:7468](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonCompositionDocument.cpp:7468)은 최신 디스크 재파싱·병합과 임시 파일 검증 후 교체한다. 실패한 후보를 정상 문서로 확정하지 않는다.

## 3. 한글 displayName과 실행 ID는 다르다

[Parse_Text:4292](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonCompositionDocument.cpp:4292)에서 JSON 문서를 읽고, [4447~4449 부근](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonCompositionDocument.cpp:4447)에서 `logicId`, `displayName`, `logicType`을 각각 별도 필드로 옮긴다.
[DataJson의 문자열 처리:213](C:/Users/tnest/Desktop/LostArk/Client/Private/DataJson.cpp:213)는 문자열을 읽으며, escape 처리와 UTF-8 변환은 [238 이후](C:/Users/tnest/Desktop/LostArk/Client/Private/DataJson.cpp:238)에 있다.
따라서 `displayName`에 한글을 저장할 수 있다. 그 한글 문장을 분석해서 게임 행동을 결정하는 구조는 아니다.

| 값 | 역할 | 실제 소비 |
|---|---|---|
| `displayName` | 사람이 목록과 타임라인에서 읽는 이름 | [Logic 목록:2344](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench_Logic.cpp:2344)는 `displayName + "##" + logicId`로 표시한다. |
| `logicId` | 재사용하는 Logic 정의의 stable ID | 목록 선택도 이름 대신 이 ID를 저장하며, projector가 정의를 찾는다. |
| `occurrenceId` / `logicOccurrenceId` | 특정 패턴 안의 시간 배치와 Collider 연결 | 같은 정의를 여러 번 사용해도 각 배치와 연결을 구분한다. |
| `logicType`, 결과 kind | 실행할 판정·결과의 명시적 종류 | projector와 서버 enum 분기에서 처리한다. |

[Timeline.cpp:924](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench_Timeline.cpp:924)는 ID로 정의를 찾은 뒤 `displayName`을 라벨로 사용한다.
표시 이름만 바꾸는 작업과 stable ID 또는 논리 종류를 바꾸는 작업은 영향 범위가 다르다.

## 4. anchor는 저작 값과 서버 값을 나누어 읽는다

[`_project_collider_regions`:4683](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:4683)가 Collider occurrence를 실행 영역으로 바꾸며, [4725](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:4725)와 [4758](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:4758)에서 anchor를 결정한다.

| 저작 기준 | 실행 anchor | 의미 |
|---|---|---|
| `MAP` | `WORLD` | 서버 월드 좌표를 사용한다. LINEAR/track이 있으면 움직일 수 있으므로 WORLD 자체가 항상 정지를 뜻하지 않는다. |
| `BOSS`, follow 켬 | `BOSS_CURRENT` | 현재 서버 보스 위치·yaw를 기준으로 로컬 영역을 변환한다. |
| `BOSS`, follow 끔 + 지원 overlap 논리 | `BOSS_START` → 캡처 후 `WORLD` | 시작 기준 위치·yaw를 한 번 저장해 이후 보스 이동과 분리한다. |
| `WORLD` | 주로 `WORLD`, 조건부 `BOSS_SPAWN` | 저작 WORLD는 WorldObject 참조다. 해당 오브젝트의 배치·동작을 샘플링/track으로 투영한다. |
| WorldObject의 배치 생략 + spawn anchor | `BOSS_SPAWN` | 현재 움직이는 보스가 아니라 저장된 보스 spawn 위치를 기준으로 한다. |

서버 변환 함수는 [Resolve_LogicRegionTransform:263](C:/Users/tnest/Desktop/LostArk/Server/Private/KoukuSaydonLogicRuntime.cpp:263)이다. `BOSS_CURRENT`의 XZ는 현재 보스 yaw로 회전하고 현재 위치를 더한다.
`BOSS_SPAWN`은 spawn 위치를 더한다. 두 경우의 Y 가산은 코드상 CYLINDER 또는 LINEAR 영역 조건이 있으므로 모든 모양에 같은 3D 규칙을 가정하면 안 된다.
[Capture_StartRegions:1125](C:/Users/tnest/Desktop/LostArk/Server/Private/KoukuSaydonLogicRuntime.cpp:1125)는 BOSS_START 영역을 캡처하고 `FrozenRegions`에 월드 기준으로 보관한다.
기본 캡처 시각은 Logic 창 시작이다. 연결한 `anchorPresentationOccurrenceId`가 있으면 [projector:4774](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:4774)가 그 presentation의 시작을 `captureStartMs`로 내보낸다.
이는 과거 보스 pose를 재구성하는 기능이 아니다. 지정 시각 이상이 되어 처음 처리할 때 제공된 보스 상태로 캡처하며, [Update의 사전 캡처:1947](C:/Users/tnest/Desktop/LostArk/Server/Private/KoukuSaydonLogicRuntime.cpp:1947)가 Logic 창보다 이른 캡처도 처리한다.
`OBJECT_CONTACT`의 BOSS Collider는 follow를 켜야 한다. WORLD Collider의 bone 사용도 이 논리에서는 금지하며, [projector:4716](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:4716)가 계약 위반을 거절한다.

## 5. bone에서 서버 판정 영역으로 넘어가는 경계

Server가 Client의 `CModel`과 본 포인터를 빌려 매 프레임 읽는 구조가 아니다.
[`_project_bone_collider_track`:4423](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:4423)과 이어지는 `_build_bone_collider_track`이 설치된 모델·clip을 샘플링해 숫자 track으로 만든다.
WorldObject는 [4737 이후](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:4737)에서 참조와 수명을 확인하고, 연속 판정이면 track, 그 외에는 판정 시점의 변환을 투영한다.
본 샘플에 occurrence의 전체 TRS가 이미 포함된 `BONE` 경로는 [4759 이후](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:4759)에서 위치·yaw의 이중 적용을 방지한다.
따라서 손 주변 표시가 맞아도 실제 판정이 어긋나면 bone·anchor·track·root motion과 실행 시간의 각 입력을 함께 비교해야 한다.

## 6. Encounter → bootstrap → Catalog

투영 결과는 [KoukuSaydonEncounter.json](C:/Users/tnest/Desktop/LostArk/Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json)이며, `_project_logic_window`는 [4870](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:4870)에 있다.
[Publish-GameplayBalance.ps1:3565](C:/Users/tnest/Desktop/LostArk/Tools/GameplayPipeline/Publish-GameplayBalance.ps1:3565)가 Encounter를 읽고 `Add-KoukuBootstrapRows`를 호출한다.
행 생성은 [REGION:1509](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/KoukuBootstrapRows.ps1:1509), [CAPTURE:1516](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/KoukuBootstrapRows.ps1:1516), [WORLD:1563](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/KoukuBootstrapRows.ps1:1563), [OUTCOME/PUSH:1771](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/KoukuBootstrapRows.ps1:1771)에서 확인할 수 있다.
Server 기본 입력은 `Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap`이다. `LOSTARK_SERVER_DATA_ROOT` 설정에 따라 실제 data root가 달라질 수 있다.
[GameplayCatalog::Load:1419](C:/Users/tnest/Desktop/LostArk/Server/Private/GameplayCatalog.cpp:1419)가 bootstrap 경로를 선택하고, [REGION:3323](C:/Users/tnest/Desktop/LostArk/Server/Private/GameplayCatalog.cpp:3323), [OUTCOME:3529](C:/Users/tnest/Desktop/LostArk/Server/Private/GameplayCatalog.cpp:3529), [PUSH:3656](C:/Users/tnest/Desktop/LostArk/Server/Private/GameplayCatalog.cpp:3656)에서 typed 값을 읽는다.
게시 쿠크 재로드인 [Load_PublishedKoukuProduct:1608](C:/Users/tnest/Desktop/LostArk/Server/Private/GameplayCatalog.cpp:1608)도 이 bootstrap 경로를 사용한다. JSON 파일만 저장했다고 실행 중 서버가 새 정의를 읽었다고 판단하지 않는다.

## 7. 접촉 판정에서 실제 피해·넉백까지

현재 audition tick의 [GameRoom_KoukuAudition.cpp:1580](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_KoukuAudition.cpp:1580)은 stage root motion을 먼저 적용한 뒤 `LogicRuntime::Update`, 플레이어 target 갱신, 출력 적용, Brain 단계 진행을 수행한다.
LogicRuntime은 시작 시 [Build:129](C:/Users/tnest/Desktop/LostArk/Server/Private/KoukuSaydonLogicRuntime.cpp:129)로 창 시간과 실행 ledger를 만들고, 각 tick에는 typed 정의와 그 ledger를 함께 사용한다.
[Contains_LogicRegion:371](C:/Users/tnest/Desktop/LostArk/Server/Private/KoukuSaydonLogicRuntime.cpp:371)이 변환된 영역과 플레이어를 판정한다. 이동 영역은 해당 경로의 swept 판정도 사용한다.
[접촉 결과 호출:2377](C:/Users/tnest/Desktop/LostArk/Server/Private/KoukuSaydonLogicRuntime.cpp:2377)은 실제 접촉 중심을 `Apply_Results`로 넘긴다.
[Apply_Result:1438](C:/Users/tnest/Desktop/LostArk/Server/Private/KoukuSaydonLogicRuntime.cpp:1438)는 피해 종류와 push 거리·시간·방향·강제 여부 등을 `SERVER_COMBAT_HIT`에 구성한다.
`AWAY_FROM_CONTACT`에는 접촉 중심이 필요하며 없으면 해당 결과를 적용하지 않는다. `BOSS_FORWARD`는 보스 yaw와 방향 offset을 사용한다.
[Apply_WorldToPlayer:664](C:/Users/tnest/Desktop/LostArk/Server/Private/ServerCombatHitRuntime.cpp:664)는 사망·TimeStop·grab·면역 등의 서버 조건을 확인하고 HP와 damage event를 갱신한다.
이어 [788](C:/Users/tnest/Desktop/LostArk/Server/Private/ServerCombatHitRuntime.cpp:788)의 `Arm_PlayerHitReaction`이 넉백 상태를 설정하고, [Advance_PlayerKnockback:1419](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_PlayerSimulation.cpp:1419)가 실제 서버 위치를 진행시킨다.
그 결과는 [Broadcast_WorldSnapshot:494](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_Replication.cpp:494) 등 기존 복제 경로로 Client에 전달된다. ImGui Collider 표시 자체가 HP나 위치를 직접 바꾸지 않는다.

확인은 **저장된 occurrence/Logic ID → Encounter region·anchor·결과 → 게시 bootstrap 행 → Catalog 정의 → 현재 tick의 영역과 player → hit 승인 → 넉백 상태** 순서로 이어가면 된다.
이 메모는 연결과 좌표 의미를 확인한 읽기 전용 조사다. 새 랜덤 패턴 후보와 검증 범위는 [수동 랜덤 가이드](C:/Users/tnest/Desktop/LostArk/out/BossSimulationRefactor20261014/manual-random/MANUAL_RANDOM_NOTES.md)에 별도로 남겼다.
