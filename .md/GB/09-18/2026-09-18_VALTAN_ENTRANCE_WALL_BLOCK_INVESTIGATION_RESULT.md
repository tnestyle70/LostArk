# 2026-09-18 발탄 입장 컷신 입구 막힘 벽 조사 결과

조사만 했다. 파일 수정·삭제·게시·Map Tool Save는 하지 않았다.

## 1. 입구를 막는 벽의 정체

입장 컷신 `source.entrance.cut01`의 카메라는 (132.81, 25.90, -98.33)에서 출발해 yaw 135°로
아레나 중심 쪽 (155.62, 25.31, -121.59)으로 들어간다. 아레나 중심 (156.03, -122.06) 기준 방위는 약 -44°다.
이 궤적 위에 있는 원본이 아닌 배치는 **109 외곽 링 deploy 벽뿐**이다.

- 위치: 중심 (156.03, -122.06), 반지름 16.10, y 23.04에 링 모양으로 360°를 빈틈없이 닫는다.
- 구성: 30그룹 × 2배치 = 60배치.
  - 본체 `DEPLOY_ITR_02306` 30개. ID `1090000000000001`~`030`, 12° 간격.
  - gap-filler `DEPLOY_ITR_02307` 30개. ID `1091000000000001`~`030`, 6° 어긋남, scale 1.52.
- 출처: `Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.deployplacements`에만 있다.
  `Data/Maps/Imported/.../LV_LUT_HEARTRB_ED.deployplacements`(원본 85행)에는 없다.
  저작본은 151행이며 차이 66행 = 링 60 + 바닥 레일/벽돌 6.
- 서버 충돌 박스: 그룹마다 `collision.valtan.wallgroup.sector03.<id>`와 `.receiver` 두 개.
  halfExtents는 [2.294, 2.334, 0.838]로, 벽 한 장은 대략 폭 4.6m, 높이 4.7m, 두께 1.7m다.

입구 방위에 걸린 그룹은 다음과 같다(측정값).

| 그룹 | 본체 방위 | filler 방위 | 비고 |
|---|---|---|---|
| `outerwall109.1090000000000011` | -30° | -36° | |
| `outerwall109.1090000000000025` | -42° | -48° | 카메라 궤적이 정확히 지난다. 원본 카메라 두 번째 키 (145.14, -110.89)와 본체 (145.26, -110.10)의 거리는 0.8m다. |
| `outerwall109.1090000000000026` | -54° | -60° | |

추론: cut01 시작 시점(fovY 29.395, 16:9에서 수평 약 50°)에서 약 16.8m 앞 링을 보면 방위 -16°~-72° 범위가 화면에 들어간다.
따라서 스크린샷의 벽은 010/011/025/026/027 그룹 일대로 보인다. 스크린샷의 정확한 ms는 확인하지 못했다.

visual 맵 배치 가운데 에디터가 추가한 78개(`editor`) 중 입장 궤적 8m 안에 있는 것은 0개다.

같은 방위에 원본 벽 2개도 있다. 둘 다 원본 추출본에 있던 배치다.
- `destroyable.group.valtan.entrance.frontwallA`: `DEPLOY_ITR_02315`, (145.15, -115.00), 반지름 12.97, -57°
- `destroyable.group.valtan.entrance.frontwallB`: `DEPLOY_ITR_02316`, (143.73, -114.55), 반지름 14.41, -58.6°
- 파괴 조건: `VALTAN_ENTRANCE_WHIRLWIND / SWEEP`의 COLLISION_IMPACT, 또는 COLLIDER_CONTACT.
- 카메라 궤적과의 가로 거리는 약 3m다.

## 2. 넣은 시점과 이유

`git log -S`로 확인했다. 작성자는 모두 KCY다.

| 커밋 | 날짜 | 추가 |
|---|---|---|
| `23007084` feat(valtan): 109-bar arena collapse with server-authoritative leap | 08-14 | 02306 1~24번. 커밋 메시지: "the ring leaves the entrance corridor and the 159 passage open" |
| `33c0371d` feat(valtan): break each wall by real collider contact and cycle the four pillars | 08-16 | 02306 25~30번. 25~28은 입구 통로(-42~-78°), 29~30은 159 통로(162°·150°)를 채움. 이유는 메시지에 없음. |
| `20779077` wip(valtan): safety snapshot before merging the boss combat runtime | 08-21 | 02307 1~30번(gap-filler). `Tools/WorldPipeline/sync_valtan_109_outer_wall_gap_fillers.py` |

용도는 `VALTAN_ARENA_BREAK_109 / IMPACT`, 즉 2페이즈 진입 109줄 도약 착지 때 외곽 링이 날아가는 연출이다
(08-16 RESULT 50행, 08-27 PLAN/RESULT).
`VALTAN_DASH_CHARGE / CHARGE` 충돌로도 부서진다.
일반 접촉으로는 부서지지 않도록 보호돼 있다.

사용자 기억 판정: "벽이 날아가는 연출용으로 세우면서 입구를 막았다"는 맞다.
다만 연출은 포효가 아니라 **109 도약 착지의 외곽 링 붕괴**다.
처음(08-14)에는 입구를 열어 뒀고, 이틀 뒤(08-16) 25~28번으로 막았다.

## 3. 원본 게임의 입구

- 원본 추출본 `out/ValtanCameraReplace20260917/ORIGINAL_CAMERAS_V2.json`,
  SCENE06A "발탄 등장(부활)"(`978T90T8XHW4FTFB8IWP8IMIWNW6C4.upk`, main_sequence matinee_0)의 cut01 카메라는
  (132.81, 25.90, -98.33) → (145.14, 26.10, -110.89) → … 로 링 025 자리를 통과한다.
  원본에서는 그 자리에 벽이 없어야 성립하는 궤적이다.
- 원본 deploy에 외곽 링이 없다(위 1절).
- 원본에도 입구 벽 frontwallA/B는 있다. 원본 입장 연출에서 이 둘이 언제 사라졌는지는 확인하지 못했다.

## 4. 입구 조각을 지우면 깨지는 것

| 대상 | 근거 | 결과 |
|---|---|---|
| 서버 파괴 publisher | `Tools/WorldPipeline/Publish-ValtanWorldDestruction.ps1:58-60, 68, 71, 1386-1389` 외곽 그룹 30 / 멤버 60 / 이미터 30 / filler 각도 π/30 고정 | 개수가 달라지면 publish 실패 |
| 초기 상태 | 같은 파일 `:790`, `initialState`는 `INTACT`만 허용 | "처음부터 부서진 상태"는 데이터로 표현할 수 없음 |
| gap-filler 동기화 | `sync_valtan_109_outer_wall_gap_fillers.py` `SOURCE_COUNT=30`, 각도 표 | OutOfSyncError |
| 서버 파괴 그룹 | `Data/Encounters/Valtan/ValtanWorldEvents.json` groups[20..22], mutation 3, binding 6 | 함께 지워야 함. 이 세션의 수정 금지 파일이다. |
| 서버 충돌 | `Data/Worlds/LV_LUT_HEARTRB_ED/Gameplay.world.json` collisionBox 2개/그룹 | 함께 지워야 함. float32 오염 이력이 있어 Map Tool Save 금지. |
| 네비 | `Data/Navigation/LV_LUT_HEARTRB_ED.navblockers` 2443/2554/2602행 REGION `BLOCK_WHILE_INTACT` | 함께 지워야 함. `Publish-ServerNavigation.ps1` 필요. |
| Map Tool 파괴 미리보기 | `LV_LUT_HEARTRB_ED.destructionsimulation.json` | 함께 지워야 함 |
| 입장 흐름 | `Stage_Boss_ArenaEntry` 트리거 (139.75, 25.73, -112.75)가 movePlayer로 (147.75, 23.02, -117.25)까지 0.8초 이동 | 벽이 없어도 동작한다. 다만 입구로 걸어서 드나들 수 있게 된다. |
| 109 연출 | 입구 쪽 벽이 없으므로 그 방위에서는 날아가는 벽이 없음 | 08-14 원래 설계와 같은 모습 |

그 밖의 제약:
- Map Tool은 deploy 배치를 편집하지 못한다(`Client/Private/MapTool.cpp:1178`,
  "DeployProp authoring is excluded until its source/stage contract is complete.").
- `Publish-MapAuthoring.ps1`의 `-Scope`는 `Area | WorldSequences | Lights` 셋뿐이다.
  deployplacements는 `Area`에서만 게시되고, `Area`는 `maplights.json`·`mapmaterials.json`·`mapeffects.json`·camerashots도 함께 게시한다(`:3213`, `:3239`, `:3324`, `:3436-3440`).
  조명만 빼고 deploy만 게시하는 Scope는 없다. 조명은 팀장 담당이다.

## 5. 해결 방안

### A. 입구 조각 삭제(실제 게임까지 열림)
- 대상: 그룹 025 한 개, 넓게 열려면 011/025/026 세 개.
- 수정: 위 4절 표의 파일 전부. 여기에 publisher 기대 개수(30→27 등)와 sync 스크립트 상수를 바꿔야 한다.
- 게시: `Publish-MapAuthoring.ps1 -AreaId LV_LUT_HEARTRB_ED -Scope Area`(조명 동반), `Publish-ValtanWorldDestruction.ps1`, `Publish-WorldGameplay.ps1`, `Publish-ServerNavigation.ps1 -AreaId LV_LUT_HEARTRB_ED`, 그리고 Server 재시작.
- 위험: 전투 데이터 수정(수정 금지 파일), 조명 게시 충돌, 서버 계약 검사 다수 수정.
- 되돌리기: git으로 원복 후 같은 게시를 다시 한다.

A의 변형으로, 08-16에 추가된 25~30번만 빼서 08-14 원래 설계로 되돌리는 방법이 있다. 부담은 A와 같다.

### B. Map Tool 컷신 미리보기 동안만 숨김(게임은 그대로)
- 방법: 편집 세션이 Play할 때 해당 deploy 배치를 `CDeployPropRuntime::Set_State(id, DEPLOY_PROP_STATE::DESPAWNED)`로 숨기고, Stop·Area 변경 때 원래 상태로 되돌린다.
- 대상 ID는 camerashots의 cutscene에 선택 필드로 저장하는 방식이 적합하다. 이 경우 parser와 publisher 검증에 필드 추가가 필요하다.
- 영향: Map Tool 미리보기만 바뀐다. 전투·서버·조명은 무관하다. C++ 수정과 Client 빌드가 필요하다.
- 확인하지 못한 것: `DESPAWNED → INTACT` 복귀를 `Set_State`가 허용하는지. 서버 파괴 스냅샷이 이미 부순 벽을 INTACT로 되살리지 않도록 복원 기준을 "숨기기 전 상태"로 잡아야 한다.
- 기존 `CWorldSequencePlayer::Set_PlacementSuppressed`는 visual 맵 배치 전용이다. world sequence는 deploy를 애니메이션 대상으로만 다루는데, ITR_02306은 `STATIC`이라 해당하지 않는다. 그래서 데이터만으로는 불가능하다.

### C. 초기 상태 변경 또는 입장 소용돌이에서 먼저 부수기
- `initialState`는 `INTACT`만 허용되고, 109가 외곽 30그룹 전부에 닿아야 한다는 검사와 binding 계열 화이트리스트(`:1357`)가 있다.
- publisher와 전투 데이터를 함께 바꿔야 하므로 A보다 가볍지 않다. 권하지 않는다.

### D. 외곽 링 전체 제거(원본 상태)
- 109 붕괴 연출 자체가 사라진다. 권하지 않는다.

## 6. 추천

목적이 "Map Tool에서 원본 입장 컷신을 제대로 확인"이라면 **B**를 권한다.
컷신 편집 작업의 "실제 전투 패턴은 바꾸지 않는다" 조건과 맞고, 조명·전투 데이터를 건드리지 않는다.

실제 게임에서도 입구를 열고 싶다면 A(그룹 025만)를 쓴다.
다만 전투 데이터 수정 승인과, 조명이 함께 게시되는 문제를 팀장과 먼저 정리해야 한다.

## 7. 확인하지 못한 것

- 스크린샷의 정확한 cut과 ms.
- 08-16에 입구 통로를 막은 이유(커밋 메시지·문서에 없음).
- 원본 입장 연출에서 frontwallA/B의 처리.
- `CDeployPropRuntime::Set_State`의 `DESPAWNED → INTACT` 복귀 가능 여부.
