# 로딩 완료 뒤 로비 잔상 제거 구현 계획

## G00. 현재 호출 순서와 결함

`CMainApp::Update`의 runtime UI 갱신은 `Is_RuntimeUIScreenSuppressed()`가 false일 때만 실행된다.
Loading 동안 `Update_LobbyButtons`도 생략되므로 STATIC layer에 남은 로비 sprite의 visible이
true로 유지된다. `Close_RuntimeWindowsForLoading`은 현재 로비 view를 숨기지 않는다.

같은 Update 끝의 `Apply_LevelRequest`는 Loading의 Update/Late_Update가 렌더 큐를 만든 다음
목적 Level로 교체한다. Loading 소멸자는 이미 큐에 있는 loading sprite도 숨기지만 목적 Level은
아직 한 번도 Update/Late_Update하지 않았다. 이 프레임에 기존 로비 sprite만 다시 노출될 수 있다.

## G01. MainApp.cpp 프레임 경계

`Apply_LevelRequest`를 다음 `CMainApp::Update` 시작으로 옮긴다. 이전 프레임의 요청만 소비하고,
성공한 목적 Level이 같은 프레임의 UI 갱신·Engine Update·Late_Update·Render를 함께 소유하게 한다.
현재 Level의 Update 도중 교체하지 않는 기존 계약은 유지한다. 준비를 기다리는 요청과 실패의
복구 경로도 기존 서비스가 소유한다.

`Close_RuntimeWindowsForLoading`에서 `m_pLobbyBackgroundView`의 모든 slot을 숨기고
`m_bLobbyWasActive`를 false로 만들고 Lobby의 character-select modal도 닫는다. 로비 복귀 시 기존 logo 초기화가 다시 적용된다.
새 C++ 파일, public API, 데이터, 프로젝트 등록 변경은 없다. 기존 UTF-8/CRLF를 유지한다.

## G02. 검증

현재 함수의 호출 순서와 단일 소비 지점을 검토하고 Debug/Release Client 컴파일을 확인한다.
`git diff --check`를 수행한다. Client 실행과 화면 판정은 사용자가 직접 하며,
Lobby→Kouku/Character Select 입장의 완료 프레임과 실패 후 Lobby 복귀를 확인한다.
