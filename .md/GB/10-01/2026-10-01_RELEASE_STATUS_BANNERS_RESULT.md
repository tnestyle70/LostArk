# Release Lobby·Character Select 상시 상태 문구 제거 결과

## G01. 실제 반영

`CMainApp::RenderLobbyButtonText`의 Release 전용 상시 상태 draw 블록을 제거했다.
`Choose a stage directly or open Character Select to change class.` 같은 상태 문장은
Lobby 화면 하단에 그리지 않는다. 서버 목록과 접속·종료·환경설정 등 제품 label은 유지했다.

`CLevel_CharacterSelect::Render_ProductStatus`의 정의·선언·호출을 제거했다.
`Server Arena active. Select a class thumbnail, then test its skill keys.` 및 연결 중
Server 안내 문장은 Character Select 화면 상단에 그리지 않는다.

두 상시 overlay가 공유하던 일반 상태·오류 문자열의 화면 출력 전체를 제거한 변경이다.
상태 문자열 저장, `CLevel_Lobby::Get_ProductStatus`, Debug 상태 패널, 입장 실패·복귀 처리,
캐릭터 생성 모달의 별도 유효성 검사·오류 안내는 유지했다. F1 이동·진단 소비자가 필요한
상태를 계속 조회할 수 있다. JSON·Resources·프로젝트 등록은 변경하지 않았다.

## G02. 실행한 확인

기존 미커밋 변경을 포함한 적용 직전 bytes와 비교해 MainApp.cpp 47줄,
Level_CharacterSelect.cpp 55줄, Level_CharacterSelect.h 1줄만 삭제했음을 확인했다.
백업은 `out/ReleaseStatusBanners20261001/replace-backup/`에 있다.
세 파일의 UTF-8과 CRLF를 유지했으며 `git diff --check`가 통과했다.
`Render_ProductStatus`의 잔류 선언·호출·정의가 없고 Lobby 상태 API와
`CreateCharacterModal_StatusText` 표시 경로, 두 Debug 상태 출력이 남아 있음을 확인했다.

F1 이동·쿠크 최초 진입음 수정과 함께 최종 정상 Product Debug/Release Build를 수행했고
모두 PASS다. 영수증은 out/BuildPipeline/runs/20260930T194308285Z-debug-product.json과
20260930T194348823Z-release-product.json이다. 산출물 갱신·증분 재사용과 상세 검증 경계는
같은 폴더의 2026-10-01_RELEASE_F1_LEVEL_NAVIGATION_RESULT.md G03에 기록했다.
Client를 실행·조작하거나 화면을 캡처하지 않았으며 실제 화면 확인은 사용자 검증으로 남는다.
