# 팀 LAN endpoint 10.207.18.151 전환 구현 결과

작성일: 2026-08-27

상태: 설정·문서·자동 검증 완료, Server host 실행 확인 대기

## 완료 내용

- 팀 LAN endpoint 정본을 `10.207.18.151:7777`로 변경했다.
- Server listener 기본 계약 `0.0.0.0:7777`과 명시적 환경변수 우선순위를 유지했다.
- Client의 compiled fallback, x64 Debug/Release debugger environment와 port-only 연결 overload가
  모두 공용 endpoint resolver를 사용하도록 정리했다.
- `AGENTS.md`, `CLAUDE.md`, 팀 README와 LAN 실행·진단 사용서를 새 endpoint 기준으로 갱신했다.
- 격리 harness의 명시적 `127.0.0.1`과 Server bind용 `0.0.0.0`은 변경하지 않았다.
- 플레이어 수 계약은 변경하지 않았다. 기존 Server의 4인 admission 계약과 contract test를 그대로
  사용한다.

## 자동 검증

| 검증 | 결과 |
|---|---|
| `TeamLanEndpoint.json` JSON parse | PASS, `10.207.18.151:7777` |
| `Client.vcxproj` XML parse | PASS |
| 현재 코드·설정·공용 문서의 이전 endpoint literal 검사 | PASS, 0건 |
| `Sync-TeamLanEndpoint.ps1` | PASS, `Machine role: client`, Client endpoint `10.207.18.151:7777` |
| Git 제외 `Client.vcxproj.user` 동기화 | PASS, `LOSTARK_SERVER_HOST=10.207.18.151` |
| Client x64 Debug build | PASS, exit code 0 |
| Client x64 Release build | PASS, exit code 0 |
| NetworkProtocolHarness x64 Debug | PASS, `failures : 0` |
| NetworkProtocolHarness x64 Release | PASS, `failures : 0` |
| `Server.exe --contract-test` Release | PASS, `failures : 0` |
| `git diff --check` | PASS |

저장소에 endpoint 전용 `ProjectAudit` 실행 파일이나 스크립트는 현재 존재하지 않아 검증 결과로
기록하지 않았다. 대신 정본 JSON/XML parse, live surface literal 검사, protocol harness와 Server
contract test를 실행했다.

## 남은 수동 확인

현재 작업 PC는 `10.207.18.151`을 소유하지 않아 sync 결과가 `client`로 판정됐다.
`10.207.18.151`은 ICMP에 응답하지만 TCP 7777 probe는 `not-listening`이다. 해당 주소를 가진 PC에서
sync script를 실행해 `server-host`를 확인하고 Server를 시작한 뒤 아래를 확인해야 한다.

1. Server 로그의 `Listening on 0.0.0.0:7777`
2. Client PC의 `Test-NetConnection 10.207.18.151 -Port 7777` 성공
3. IDE를 재시작하거나 project를 reload한 뒤 Lobby의 `Server: 10.207.18.151:7777` 표시
4. 사용자가 직접 Bern 또는 Valtan에 여러 Client로 진입해 4인 동시 접속 확인

Server가 시작되기 전 `not-listening`은 설정 반영 실패가 아니다. Lobby에 이전 주소가 계속 보이면
실행 중이던 Client나 Visual Studio에 남은 debugger environment를 먼저 종료·재시작한다.
