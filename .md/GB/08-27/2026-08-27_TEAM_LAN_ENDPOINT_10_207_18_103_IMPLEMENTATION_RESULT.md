# 팀 LAN endpoint 10.207.18.103 전환 구현 결과

작성일: 2026-08-27

상태: 설정·문서·Release 실행 검증 완료, 다른 PC의 LAN 접속 확인 대기

## 완료 내용

- 팀 LAN endpoint 정본을 Server PC가 소유한 `10.207.18.103:7777`로 변경했다.
- Server listener 기본 계약 `0.0.0.0:7777`과 명시적 환경변수 우선순위를 유지했다.
- Client의 compiled fallback, x64 Debug/Release debugger environment와 port-only 연결 overload가
  모두 공용 endpoint resolver를 사용하도록 정리했다.
- `AGENTS.md`, `CLAUDE.md`, 팀 README와 LAN 실행·진단 사용서를 새 endpoint 기준으로 갱신했다.
- 격리 harness의 명시적 `127.0.0.1`, Server bind용 `0.0.0.0`과 기존 4인 admission 계약은
  변경하지 않았다.

## 자동 검증

| 검증 | 결과 |
|---|---|
| Server PC의 endpoint 소유권 | PASS, `Wi-Fi 2`가 `10.207.18.103/24` Preferred 상태로 소유 |
| `TeamLanEndpoint.json` JSON parse | PASS, `10.207.18.103:7777` |
| `Client.vcxproj` XML parse | PASS |
| 현재 코드·설정·공용 문서의 이전 endpoint literal 검사 | PASS, 0건 |
| `Sync-TeamLanEndpoint.ps1` | PASS, `Machine role: server-host` |
| Server debugger bind | PASS, `0.0.0.0:7777` |
| Client debugger endpoint | PASS, `10.207.18.103:7777` |
| Server firewall | PASS, TCP 7777 `LocalSubnet` ready |
| Git 제외 Client/Server `.vcxproj.user` 동기화 | PASS |
| Client x64 Release build | PASS, 새 `Client.exe` 생성 및 실행 확인 |
| Server listener | PASS, `0.0.0.0:7777`에서 수신 중 |
| 로컬 Client의 정본 endpoint 접속 | PASS, `10.207.18.103 -> 10.207.18.103:7777` `Established` |
| `git diff --check` | PASS |

이 변경 직전 같은 PR revision에서 Client x64 Debug/Release build, NetworkProtocolHarness
Debug/Release와 Server Release contract test가 모두 통과했다. 최종 `.103` literal을 포함한 Release
재빌드와 Server/Client 실행 뒤 실제 TCP connection도 확인했다.

## 남은 수동 확인

Server PC의 listener와 같은 PC Client 접속은 확인했다. 다른 PC에서 다음을 사용자가 확인한다.

1. `Test-NetConnection 10.207.18.103 -Port 7777` 성공
2. IDE를 재시작하거나 project를 reload한 뒤 Lobby의 `Server: 10.207.18.103:7777` 표시
3. Bern 또는 Valtan에 여러 Client로 진입해 4인 동시 접속 확인

각 Client가 DHCP로 받은 자기 주소는 서로 달라도 정상이다. 모든 Client는 Server PC의 단일 주소
`10.207.18.103:7777`을 사용하며, Server 플레이 중에는 이 주소를 소유한 `Wi-Fi 2` 연결을 유지한다.
