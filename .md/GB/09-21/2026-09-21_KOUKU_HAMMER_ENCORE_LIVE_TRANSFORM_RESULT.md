# 카드미로 플레이어 망치와 빙고 앵콜 시작 회전 편집 결과

## G01. 실제 구현

`Client/Private/MainApp.cpp`의 Card Maze Player Hammer는 TRS와 Reset을 별도
`hammerEdited`로 모은 뒤 기존 활성 Level의 profile setter에 즉시 전달한다. 종전에는
일반 camera preview의 `!Is_PresentationOverrideActive()` 조건에 같이 묶여 카드미로
카메라 중 변경값이 draft에만 남았다. 이제 망치 profile 적용은 그 조건을 사용하지 않으며
`Set_FollowEnabled`도 호출하지 않아 카드미로 카메라 소유권과 F6 상태를 유지한다.
기존 `CCharacter::Update_PresentationRootMatrix`가 매 Update에서 실제 손 부착 망치 part의
local TRS를 읽는다. 새 Character/Server 경로나 profile schema는 만들지 않았다.

Reset 바로 옆에 `Save player hammer` 버튼을 추가했다. 기존 map별
`CArenaCameraProfile::Save`와 source baseline 검사를 재사용하며 저장 실패 시 draft와
기존 파일을 보존한다. 활성 map이면 profile을 적용하고, 비활성 map이면 다음 입장용 저장으로
기존 메시지가 구분한다. 이 버튼은 선택 map의 기존 camera profile을 저장하므로 같은 profile의
다른 편집값도 함께 저장한다.

기존 `Render_KoukuEncoreRotation`을 관문 버튼 위에서 제거하고,
`pAuditionPlacementId == boss.kakulsaydon.bingo.saydon`인 빙고 앵콜 행 바로 아래에 배치했다.
live yaw preview, Save Encore Rotation, Reload Saved Rotation, Reset Rotation Preview와
기존 freshness/atomic rollback 구현은 유지했다. Save 대상은
`Data/Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json`의 해당 stable placement
`yawDegrees`와 revision이다. Server가 사용할 시작 회전은 World Gameplay publish와
Server world reload 후 반영되며, 현재 미리보기는 Client presentation이다.

## G02. 실행한 검증

| 검증 | 실제 실행 결과 |
|---|---|
| MainApp Debug TU 실제 컴파일 | `out/KoukuHammerEncore20260921/client.cmd`, exit 0, MainApp.obj 생성. 기존 Camera_Free.h 인코딩 경고만 존재 |
| production profile/parser/save 검사 | 현재 ArenaCameraProfile.cpp, DataJson.cpp, ProjectDataRoot.cpp와 기존 CardMaze profile probe를 다시 컴파일하고 격리 root에서 168개 검사 PASS |
| 검사 범위 | 망치 TRS 저장/로드, 구형 optional 기본값, 0 scale/범위/NaN 거절, stale write와 malformed load의 기존 상태 보존, map별 파일 격리 |
| 변경 줄 검사 | MainApp.cpp와 대응 PLAN/RESULT 범위 `git diff --check` PASS |
| 인코딩 | MainApp 기존 UTF-8 BOM 없음과 CRLF 유지 |

로그는 `out/KoukuHammerEncore20260921/client.log`, `profile-check.log`이며 probe의 쓰기 대상은
같은 out 폴더의 `isolated-profile-check`뿐이다. 새 C++ 파일이 없어 vcxproj/filter 등록 변경은
필요 없다. 기존 MainApp preparation status 관련 dirty hunk는 보존했다.

## G03. 배포와 사용자 화면 확인 경계

망치·앵콜 작업 자체에서 live profile/Gameplay.world JSON 수정이나 해당 domain publish,
Client/UI 실행, 화면 캡처는 하지 않았다. 병행한 네비게이션 5구역의 데이터 publish는
`2026-09-21_KOUKU_FINE_NAVIGATION_RESULT.md`에 별도로 기록했다.

사용자가 저장 후 Client와 Server를 직접 종료했다. 에이전트의 정상 Debug Product 빌드는
Engine·Shared·Server까지 성공했으나 Client 셰이더 단계에서 사용자 VS 빌드와 중복됨을
확인해 에이전트 소유 빌드만 중단했다. 사용자가 직접 빌드 중임을 확인하고 대기를 요청했으므로
추가 빌드는 시작하지 않는다. 독립 TU 컴파일과 Client EXE의 통합 링크 완료를 구분하며,
현재 통합 링크 완료 판정은 보류한다.

새 Client 빌드 후 사용자가 F1 → Player Follow Camera → Card Maze Player Hammer에서 카드미로
진행 중 Pos/Rotation/Size와 Save를 확인한다. F1 → KoukuSaydon Arena의 빙고 앵콜 버튼 바로
아래에서 yaw를 조절해 현재 Saydon 표현과 Save를 확인한다. 실제 화면 결과는 아직 미확인이다.
