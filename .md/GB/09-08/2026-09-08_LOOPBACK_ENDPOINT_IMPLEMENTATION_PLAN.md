<!-- Superseded: user changed the endpoint to 192.168.0.14; loopback code changes were deferred, not applied. -->
# 로컬 Server endpoint 전환 구현 계획

작성일: 2026-09-08

## G00. 현재 endpoint와 호출 경로

사용자가 요청한 같은 PC 실행 주소 `127.0.0.1:7777`을 정본, debugger와 코드 기본값에 적용한다.
현재 `TeamLanEndpoint.json`과 Client debugger는 `192.168.0.4`지만
`CNetworkManager::Resolve_ServerHost()`의 compiled fallback은 `192.168.0.1`이다.
Server의 Main, `CServerApp::Run`, `CTcpListener::Open(port)`과 debugger bind 기본값은 `0.0.0.0`이다.

`Sync-TeamLanEndpoint.ps1`은 이미 loopback 계약을 지원한다. 정본이 loopback이면 모든 PC를
`server-host`로 판정하고 Server/Client의 Git 제외 `.vcxproj.user`를 동기화하며 LAN 방화벽을 변경하지
않는다. loopback에는 임시 LAN 주소의 만료 조건이 적용되지 않는다.

## G01. 정본과 실제 소비자 변경

- `Tools/Network/TeamLanEndpoint.json`: `serverHost`와 `serverBindAddress`를 `127.0.0.1`로 변경한다.
- `Client/Private/NetworkManager.cpp`: 환경변수 미지정 시 host를 `127.0.0.1`로 변경한다.
- `Client/Default/Client.vcxproj`: x64 Debug/Release debugger environment를 맞춘다.
- `Server/Private/Main.cpp`, `Server/Public/ServerApp.h`, `Server/Public/TcpListener.h`:
  명시적 bind 인자가 없는 기존 호출 경로의 기본값을 `127.0.0.1`로 맞춘다.
- `Server/Default/Server.vcxproj`: 공유 debugger bind를 맞춘다.
- 기존 endpoint contract test는 loopback일 때 loopback bind, LAN일 때 all-adapter bind를 검사한다.
- `AGENTS.md`, `CLAUDE.md`, 팀 README와 네트워크 사용서의 현재 계약을 갱신한다.

`LOSTARK_SERVER_HOST`와 명시적 Server `--bind-address` 인자는 계속 기본값보다 우선한다.
새 C++ 파일이나 project/filter 등록은 없다. `Framework.slnLaunch`에는 주소가 없으므로 기존
`Server + Client` profile을 사용한다. 과거 PLAN/RESULT의 당시 LAN 실행 증거는 유지한다.

## G02. 적용과 종료 증거

1. JSON/project XML과 변경한 ASCII literal을 확인한다. 기존 C++ 인코딩을 유지한다.
2. sync를 실행해 `server-host`, 양쪽 `127.0.0.1:7777`과 생성된 user XML을 확인한다.
3. 기존 endpoint contract test, `git diff --check`를 확인한다.
4. 전체 기능 변경과 함께 필요한 Product 컴파일을 수행한다.
5. 실행 중인 Server/Client와 port 7777 listener 상태를 확인한다. Client/UI 실행은 사용자가 한다.

사용자는 Visual Studio project Reload 후 `Server + Client` profile에서 `Ctrl+F5`를 눌러
Server CMD의 `Listening on 127.0.0.1:7777`과 Lobby의 endpoint를 확인한다.
