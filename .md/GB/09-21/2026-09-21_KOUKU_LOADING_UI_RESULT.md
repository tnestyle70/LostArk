# 로딩 화면 도구·자막 숨김 결과

Client/Private/MainApp.cpp:3330에 LEVEL::LOADING 전용 종료 guard를 추가했다. 로딩 Level이 자신의 이미지와 제목·팁·실패 복구 표시를 그린 다음, 이미 시작한 ImGui frame을 CancelFrame으로 닫고 UIInputRouter::End_Frame과 Render_End를 호출한다. 이에 따라 기존 guard 밖에서 제출되던 ImGui/debug 오버레이와 MapTool/시네마틱 자막은 로딩 화면에 제출되지 않는다. 일반 게임/컷신의 기존 표시 정책은 유지한다.

원인은 MainApp의 runtime 글자 guard는 LOADING을 소비하지만 ImGui EndFrame과 별도 자막 호출이 그 밖에 있었던 것이다. Engine ImGuiLayer의 최종 DX11/platform submit 및 Font_Manager에 별도 loading 조건은 없다. 이 기존 누락은 이번 쿠크 저작/Complete Play diff에서 삭제된 조건은 아니다.

기존 dirty 파일 백업: out/KoukuLoadingUi20260921/MainApp.cpp.before. 변경 범위: scope.diff의 10줄 추가. 대상 git diff --check PASS, 현재 파일 UTF-8 BOM 여부/CRLF 유지. 사용자 지시에 따라 추가 컴파일·제품 빌드·UI 실행은 하지 않았다. Debug 빌드와 실제 로딩 화면 확인은 사용자 대기 상태다.

후속 사용자 빌드 확인: MainApp의 로딩 guard가 유지됐으며, 08:45:14 소스 수정 뒤 MainApp.obj가 08:46:12, Debug Client.exe가 08:46:14에 갱신됐다. Client/Default/x64/Debug/Client.log에 MainApp 컴파일과 EXE 링크·런타임 배포가 기록됐다. 코드가 이전 내용으로 덮인 흔적은 없고 실제 로딩 화면 확인만 남았다. 에이전트가 새 빌드를 실행한 것은 아니다.
