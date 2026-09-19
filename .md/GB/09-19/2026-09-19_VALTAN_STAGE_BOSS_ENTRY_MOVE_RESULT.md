# 발탄 Stage_Boss를 밟으면 Stage_Boss_ArenaEntry 위치로 이동 — 결과

작성: 2026-09-19. 소스 수정과 구문 검사까지이며 **빌드, 게시, 게임 실행은 하지 않았다.**

## 0. 가정과 확정하지 못한 것

- **"Stage_Boss를 밟으면 Stage_Boss_ArenaEntry로 이동"의 목적지를 ArenaEntry 박스의 중심(사용자가 방금 옮긴 위치)으로 해석했다.** 사용자가 바꾼 것이 그 박스 위치뿐이고 "위치 트리거 위치 수정해서 저장했어 … 저기로 이동"이라고 말했기 때문이다. 대안은 ArenaEntry 트리거가 저작한 이동 목적지 `(147.75, 23.0176, -117.25)`(보스 방 안쪽)다. 이쪽이 맞다면 `Place_PlayerAtValtanArenaEntry` 한 곳만 바꾸면 된다.
- **Stage_Boss가 원래 이동 트리거였는지는 확인하지 못했다.** 문서로 확인된 것은 08-28 계획서의 진행 순서(`Stage_3 → Stage_Boss_ArenaEntry → Stage_Boss`, ArenaEntry 도착점이 Stage_Boss 박스 안)와 09-17 결과 문서의 "Stage_Boss를 사용자가 툴에서 이동" 한 줄이다. 그 이동 이후로 ArenaEntry 도착점이 더는 Stage_Boss 박스 안이 아니게 되었다는 것은 좌표로 확인했다(아래 2절). 사용자가 "원래"라고 한 시점의 정확한 구조는 이력에서 찾지 못했다(추정).
- 보스 시작(`activateEncounter`)은 **Stage_Boss 트리거가 계속 맡는다.** 트리거는 action 하나만 가질 수 있어서, Stage_Boss를 이동 트리거로 바꾸면 보스 시작을 다른 곳에 새로 만들어야 한다. 그 방식은 저작 데이터와 이름 의미를 바꾸므로 택하지 않았다(3절).

## 1. 사용자가 옮긴 것 (HEAD 대비, float32 반올림 오차는 제외)

Gameplay.world.json(`Data/Worlds/LV_LUT_HEARTRB_ED/`, 사용자 저장 19:45:46)에서 실제 변경은 하나였다.

- `Stage_Boss_ArenaEntry` 박스 위치: `(139.75, 25.7308, -112.75)` → `(141.85, 25.7308, -107.55)`
- 같은 박스 회전(yaw): `0°` → `45.5°`
- 이동 action(목적지 `(147.75, 23.0176, -117.25)`, 0.8초, 호 0)은 그대로다.
- MapTool 저장으로 `revision`이 574에서 645로 올랐고 다른 모든 배치의 좌표가 float32 왕복으로 미세하게 달라졌다(오차 0.01 이내라 의미 없음).

이후 다른 작업(Stage_3 fork)이 같은 파일의 `Stage_3` 목적지를 `(100.42, 20.53, -86.95)`로 저장했고 `revision`이 646이 되었다. 이번 작업은 이 파일을 수정하지 않았고, 사용자의 ArenaEntry 저장값은 그대로다.

## 2. 원래 구조, 현재 동작, 어긋난 지점

- 서버 자동 발동 규칙표(`AUTO_ENTRY_RULES`)에서 발탄 `activateEncounter`는 밟으면 자동이다. Stage_Boss는 그 대상이고 **플레이어를 옮기지 않았다.** 밟은 플레이어는 복도(박스 위치 `(129.65, 23.02, -96.85)`)에 그대로 서 있고 보스만 생성된다.
- Debug에서는 `Stage_Boss`가 `Build_ValtanStageBypassMove` 표에 있어 G 전용이었고, 보스 시작 뒤 `Place_PlayerAtValtanAuditionBait`로 하드코딩 좌표 `(154.296, 22.970, -125.219)`(159 bar 벽 돌진 유도 지점, 보스 중심에서 4.5m)로 옮겨졌다. Release에는 이 이동이 없었다.
- ArenaEntry(`movePlayer`)는 G로 발동하고 보스 방 안쪽 `(147.75, 23.02, -117.25)`로 0.8초 이동시킨다. 08-28 설계에서는 이 도착점이 Stage_Boss 박스 안이라 ArenaEntry 이동이 곧 보스 시작이었다. 09-17에 Stage_Boss 박스를 복도로 옮긴 뒤로는 도착점이 Stage_Boss 박스에서 27m 이상 떨어져 이 연결이 없다.
- 사용자의 기대("Stage_Boss를 밟으면 ArenaEntry로")와 코드가 어긋난 곳: Release는 이동이 없고, Debug는 ArenaEntry가 아닌 하드코딩 좌표로 갔다.

## 3. 선택한 방식과 버린 대안

**선택: 서버 코드에서 Stage_Boss의 보스 시작 뒤에 발동한 플레이어를 ArenaEntry 박스 중심으로 보낸다.**

- `ServerTriggerSystem.cpp`의 `Run_Action`에 발탄 `Stage_Boss`(`activateEncounter`) 전용 분기를 넣었다. 보스 시작을 부른 뒤 `Place_PlayerAtValtanArenaEntry`가 활성 트리거 `Stage_Boss_ArenaEntry`를 찾아 그 박스 중심으로 `Begin_MovePlayer`한다. 진입 발동(Release)과 G 발동(Debug), Debug playback이 모두 `Run_Action`을 거치므로 Debug와 Release가 같은 경로다.
- 이동 시간과 호는 ArenaEntry 박스가 저작한 값(0.8초, 0)을 쓴다. ArenaEntry 박스가 없으면 보스 시작만 한다.
- 목적지 XZ는 게시본의 ArenaEntry 박스 위치이므로 MapTool로 박스를 옮기면 착지가 따라간다(게시 필요).
- **보스가 이미 떠 있어 시작이 거절되어도 뒤에 온 플레이어를 보낸다.** 8인 레이드가 한 명씩 걸어 들어가지 않게 하려는 것이다. 이때 트리거는 "보내졌다"를 발동으로 친다.
- 착지 높이는 서버 네비 바닥으로 잡는다(`Set_GroundSampler`, `GameRoom.cpp`가 `Sample_Position`으로 연결). 사용자의 새 박스는 Y가 25.73이고 그 자리의 네비 바닥은 22.84라 2.9m 어긋난다. 박스 Y를 착지 Y로 쓰면 공중에 뜬다. 샘플러가 없거나 거절하면 박스 Y로 착지한다.
- Debug의 `Stage_Boss` 뒤 하드코딩 유도 지점 이동은 `Run_Trigger`에서 제거했다. `Place_PlayerAtValtanAuditionBait`와 지름길 표의 `Stage_Boss` 행은 패턴 audition(`GameRoom_ValtanAudition.cpp` 6곳)이 쓰므로 남겼다.

**버린 대안**
- (a) Stage_Boss를 `movePlayer`로 바꾸고 도착 지점에 새 `activateEncounter` 박스를 추가(Stage_MiniBoss 방식): 데이터 구조와 트리거 이름 의미가 바뀌고, 코드·테스트·문서의 `Stage_Boss` 참조를 다 고쳐야 하며, 사용자의 저작 파일에 새 배치를 넣어야 한다.
- (b) 데이터 스키마에 "도착 후 이동" 필드 추가: bootstrap 형식과 publisher 변경이 필요해 과하다.
- (c) 사용자의 ArenaEntry Y를 바닥 높이로 고침: 사용자 저작 값을 임의로 바꾸는 것이라 하지 않았다. 코드가 바닥 높이를 직접 구하므로 필요하지 않다.

## 4. 바꾼 파일

- `Server/Private/ServerTriggerSystem.cpp`: 상수 2개(`VALTAN_BOSS_START_TRIGGER_ID`, `VALTAN_ARENA_ENTRY_TRIGGER_ID`), `Run_Action`의 Stage_Boss 분기, 새 함수 `Place_PlayerAtValtanArenaEntry`, `Run_Trigger`의 Debug 유도 지점 호출 제거와 주석 갱신.
- `Server/Public/ServerTriggerSystem.h`: `Set_GroundSampler`, `m_GroundSampler`, `Place_PlayerAtValtanArenaEntry` 선언.
- `Server/Private/GameRoom.cpp`: `Set_FireLog` 바로 아래에서 네비 바닥 높이 샘플러 연결(6줄).
- `Server/Private/ServerGameplayContractTests_WorldTriggers.cpp`: 새 블록. **실행하지 않았다.** 다음을 단언한다.
  - 진입: 두 플레이어 모두 ArenaEntry 박스 중심 XZ와 샘플 바닥 Y로 이동, 보스 시작은 세 번 불리되 두 번째부터는 거절.
  - 바쁜 플레이어(넉백)는 이동하지 않음.
  - 이동 완료 후 위치가 박스 중심.
  - 샘플러가 없으면 박스 Y를 사용.
  - ArenaEntry 박스가 없으면 이동 없이 보스 시작만.
  - Debug에서는 밟으면 안내만, G가 보스를 시작하고 ArenaEntry로 이동.
- `.md/GB/gotchas.md`: 항목 하나 추가.
- Client, Shared, 데이터, protocol은 바꾸지 않았다(94 그대로).
- `2026-09-19_TRIGGER_REPEAT_AND_G_KEY_RESULT.md`의 30줄과 41줄이 "Debug에서 Stage_Boss는 보스 시작 뒤 벽 돌진 유도 지점으로 옮긴다"고 서술한다. **이 서술은 이번 변경으로 틀려졌다.** 다른 작업이 함께 편집 중인 문서라 고치지 않았다.

## 5. 검증 (실행한 것과 하지 않은 것)

- **데이터/소스로 직접 확인**
  - 사용자 저장본의 ArenaEntry 위치·yaw가 그대로다.
  - 착지 XZ `(141.85, -107.55)`의 게시 네비(`LV_LUT_HEARTRB_ED.navgrid`) 셀은 걸을 수 있고 바닥은 22.836이다. 그 셀 주변 2m 이내 81칸 중 76칸이 걸을 수 있다.
  - 서버 `Contains` 규칙을 그대로 옮겨 계산하면 착지점을 포함하는 발탄 트리거는 `Stage_Boss_ArenaEntry` 하나뿐이다. **착지 즉시 연쇄 발동은 없다.** ArenaEntry는 G 전용이라 안내만 뜬다.
  - 착지 시 플레이어 중심 높이는 23.736이고 ArenaEntry 박스와의 수직 여유는 0.405m다. 박스의 걸을 수 있는 영역 3,278 샘플 전부에서 G가 통한다(바닥 22.73~25.88).
- **구문 검사만 통과(`cl /Zs`, 산출물 없음)**: `ServerTriggerSystem.cpp`, `GameRoom.cpp`, `GameRoom_PartyWorld.cpp`, `GameRoom_ValtanAudition.cpp`, `ServerGameplayContractTests_WorldTriggers.cpp` 각각 Debug와 Release, 전부 exit=0.
- **실행 안 함:** 빌드, 링크, `Server.exe --contract-test`, 게임에서 Stage_Boss를 밟거나 G를 누르는 확인. 새 테스트가 통과하는지는 모른다.
- **확인하지 못한 것:** 보스 생성과 같은 틱에 이동이 시작될 때 발탄 입장 컷신·패턴 시작과의 상호작용(카메라, 첫 패턴 시각). 소스로 추적하지 않았다.

## 6. 반영에 필요한 것

1. **`Publish-WorldGameplay.ps1 -Mode Publish`가 필요하다.** 지금 게시본(`VALTAN_ARENA.worldbootstrap`, 18:56)에는 사용자가 옮긴 ArenaEntry 위치가 없다. 게시하지 않으면 서버는 옛 위치 `(139.75, 25.73, -112.75)`로 보낸다.
2. Server 재빌드와 재시작. Client는 바뀌지 않았다.
3. Server 재시작 후 발탄에서 확인: Release는 `Stage_Boss`를 밟으면, Debug는 밟은 뒤 G를 누르면 보스가 나오고 ArenaEntry 박스 자리로 0.8초 이동하는지.

## 7. 사용자가 결정할 것

1. 목적지가 ArenaEntry **박스 중심**(현재)인지, 그 트리거의 **이동 목적지** `(147.75, 23.02, -117.25)`(보스 방 안쪽)인지.
2. Debug에서 Stage_Boss를 G 전용으로 둘지(지금), Release처럼 밟으면 자동으로 할지. 후자는 지름길 표의 `Stage_Boss` 행을 패턴 audition용 별도 조회로 분리해야 한다.
3. ArenaEntry 박스의 Y(25.73)를 그 자리 바닥(22.84)에 맞출지. 지금도 G는 되지만 수직 여유가 0.4m뿐이다. 착지는 바닥에 정확히 선다.
4. 표식 규칙(`Level_ValtanArena::Load_TriggerMarkers`는 `movePlayer`만 표시)에 따르면 Stage_Boss는 `activateEncounter`라 표식이 뜨지 않는다. Stage_Boss도 표식을 띄울지.
