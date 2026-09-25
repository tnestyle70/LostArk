# 로딩 완료 프레임의 로비 잔상 수정 결과

## G01. 원인과 반영

Loading 동안 runtime UI update를 생략하면서 STATIC layer의 기존 Lobby sprite는
visible 상태로 남았다. 동시에 Update 끝에서 목적 Level로 바꾸면 Loading 소멸자가
로딩 화면을 숨긴 뒤 목적 Level은 아직 첫 Update/Late_Update를 하지 않은 프레임이
생긴다. 이때 남아 있던 Lobby sprite만 다시 보일 수 있었다.

`MainApp.cpp`의 `Apply_LevelRequest`는 다음 Update 시작에서 이전 프레임의 요청을
소비하도록 옮겼다. 목적 Level은 첫 Render 전에 UI 갱신과 Engine Update/Late_Update를
모두 수행한다. 실행 중인 Level의 Update 안에서 Level을 파괴하지 않는 계약은 유지한다.

`Close_RuntimeWindowsForLoading`에서 Lobby background 모든 slot을 숨기고
Lobby-active flag를 초기화하며 character-select modal을 닫는다. 로비 복귀 시
기존 초기화 경로가 다시 화면을 준비한다. 새 런타임 경로나 데이터는 추가하지 않았다.

## G02. 검증과 남은 경계

- 전환 요청의 단일 소비 지점과 실패 복구 경로를 코드로 확인했다.
- Debug Client ClCompile 성공. 로그: `out/KoukuMadnessRaid20260924-client-debug-compile.log`.
- 독립 코드 검토에서 추가 결함을 찾지 못했다.
- 최종 v37 기준 Debug/Release Server·Client ClCompile 모두 exit0.
  로그: `out/KoukuMadnessRaid20260924-{server,client}-{debug,release}-v37-compile.log`.
- 최종 Product 링크는 실행 중 Client 2개와 Server 1개의 사용자 저장·종료 확인을 기다린다.
- 사용자 Client를 실행·종료·조작하지 않았다. 실제 로딩 완료 프레임과 실패 시 Lobby 복귀
  화면은 새 실행 파일로 사용자가 확인해야 한다.
