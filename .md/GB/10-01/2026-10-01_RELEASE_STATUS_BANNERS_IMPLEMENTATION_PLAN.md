# Release Lobby·Character Select 상시 상태 문구 제거

## G01. 현재 출력과 변경 범위

사용자가 Release Lobby와 Character Select에 계속 표시되는 개발용 상태 문구를 제거하도록
요청했다. Lobby 초기 문구는 `Choose a stage directly or open Character Select to change class.`이며
Character Select 활성 문구는 `Server Arena active. Select a class thumbnail, then test its skill keys.`다.

`Client/Private/MainApp.cpp`의 `CMainApp::RenderLobbyButtonText` 끝에 있는 Release 전용
상태 문자열 draw 블록을 제거한다. 서버 목록·선택·접속·종료·환경설정의 제품 label은 유지한다.
`CLevel_Lobby::Get_ProductStatus`, 입장 실패 처리와 상태 문자열 저장은 유지하므로 F1과
진단 소비자는 같은 상태를 계속 읽는다.

`Client/Private/Level_CharacterSelect.cpp`의 `CLevel_CharacterSelect::Render`에서
`Render_ProductStatus` 호출을 제거하고 더 이상 사용하지 않는 함수 정의와
`Client/Public/Level_CharacterSelect.h`의 private 선언을 삭제한다. Debug 선택 패널과
캐릭터 생성 모달의 별도 유효성 검사·오류 안내는 유지한다.

두 renderer가 표시하던 일반 상태와 오류 문장은 해당 상시 overlay에서 함께 사라진다.
문자열 내용으로 성공·실패를 추측하는 필터나 새 상태 체계는 만들지 않는다. Server 승인,
입장·복귀·캐릭터 생성 동작은 변경하지 않는다. 기존 파일별 인코딩과 다른 세션의 변경을 보존한다.

## G02. 적용과 검증

기존 H/CPP만 수정하므로 vcxproj와 filters 추가 등록, JSON 변경·publish가 필요 없다.
기존 diff와 제거 영역을 대조하고 `Render_ProductStatus` 잔류 선언·호출이 없는지 확인한다.
상태 저장/API와 생성 모달 안내가 남아 있는지 확인하며 `git diff --check`를 실행한다.
상위 작업에서 정상 Release Product Build를 수행하고 결과만 RESULT에 기록한다.
Client 실행과 실제 화면 문구 제거 확인은 사용자가 수행한다.
