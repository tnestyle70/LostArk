# 팀 LAN 192.168.200.139 적용 계획

## G00. 현재 상태와 목표

사용자 요청에 따라 Server/Client의 팀 endpoint를 `192.168.200.139:7777`로 맞춘다.
현재 PC의 `Wi-Fi 2`가 `192.168.200.139/24`를 Preferred 상태로 소유한다.
기존 공유 주소는 `192.168.0.14`, 개인 VS Client 주소는 `10.16.127.103`이며
Debug 직접 실행의 개인 loopback override도 활성화되어 있다.
기존 임시 계약은 2026-10-02에 만료됐다. 새 계약은 요청일인 2026-10-03
23:59:59 KST까지로 갱신하며 `-AllowExpired`를 사용하지 않는다.

## G01. 수정 파일과 소비 흐름

- `Tools/Network/TeamLanEndpoint.json`: 주소와 만료일 정본.
- `Client/Private/NetworkManager.cpp`: `Resolve_ServerHost`의 기본 주소 상수만 변경한다.
  기존 파일 인코딩·줄바꿈과 명시적 환경변수 우선순위를 유지한다.
- `Client/Default/Client.vcxproj`: x64 Debug/Release debugger 환경값 두 곳을 변경한다.
- `Tools/ReleasePackaging/build_portable.py`, `test_package_tools.py`, `README_실행방법.md`:
  앞으로 생성할 portable launcher와 정상 fixture·안내의 주소를 맞춘다.
- AGENTS, CLAUDE, 팀 README/interface handbook/네트워크/Runtime 전달 가이드:
  현재 public 주소·기간을 맞추고 과거 ZIP/PLAN/RESULT 기록을 보존한다.
- Git 제외 개인 Client/Server `.vcxproj.user`: 기존 sync의 명시적 Team 모드로 적용한다.
- Git 추적 `Client/Default/LocalServerEndpoint.user.json`: `enabled`만 false로 바꾸어
  Debug 직접 실행도 새 compiled fallback을 사용하도록 한다.

Server 코드·공용 project의 기존 bind `0.0.0.0:7777`과 `Framework.slnLaunch`의
`Server + Client` profile은 이미 요청을 지원한다. 새 C++ 파일·project/filter 등록은 없다.
다른 작업의 `.gitignore`, 미추적 파일과 사용자 데이터를 보존한다.

## G02. 교체 설정

```json
{
  "schema": "lostark.team-lan-endpoint",
  "version": 1,
  "serverBindAddress": "0.0.0.0",
  "serverHost": "192.168.200.139",
  "port": 7777,
  "activeThroughKst": "2026-10-03T23:59:59+09:00"
}
```

```xml
<LocalDebuggerEnvironment>LOSTARK_SERVER_HOST=192.168.200.139</LocalDebuggerEnvironment>
```

## G03. 적용·검증

개인 설정을 저장소 `out/Network/20261003-192-168-200-139`에 백업한다.
주소·public 계약 갱신 후 `Sync-TeamLanEndpoint.ps1 -EndpointMode Team`을 실행한다.
주소 소유에 따른 server-host 판정, Server bind, Client 주소와 TCP7777 LocalSubnet
방화벽 상태를 확인한다. 권한 부족은 즉시 보고하고 권한을 우회하지 않는다.

Endpoint 계약 테스트, JSON/XML/packaging Python 구조 검사, `git diff --check`와
표준 Product Debug/Release Build를 수행한다. 게임 Client/UI는 실행하지 않는다.
설정 반영, 빌드, Server listen 상태와 다른 PC의 실제 접속 검증을 RESULT에서 구분한다.
