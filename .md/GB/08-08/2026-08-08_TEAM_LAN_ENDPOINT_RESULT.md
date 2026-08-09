# 팀 LAN endpoint 공유 기본값 구현 결과

작성일: 2026-08-08

대응 계획서:
`.md/GB/08-08/2026-08-08_TEAM_LAN_ENDPOINT_IMPLEMENTATION_PLAN.md`

## 완료 상태

- Server 코드 기본 bind와 x64 shared debugger argument를 `0.0.0.0`으로 변경했다.
- Client 코드 기본 host와 x64 shared debugger environment를 `192.168.200.103`으로 변경했다.
- 현재 PC의 Git 제외 `.vcxproj.user`도 Server `0.0.0.0`, Client `192.168.200.103`으로 맞췄다.
- `Framework.slnLaunch`의 기존 `Server + Client` 순서는 유지했다.
- `AGENTS.md`, `CLAUDE.md`, 팀 게임플레이 사용서와 ProjectAudit을 같은 계약으로 갱신했다.
- `Tools/Network/TeamLanEndpoint.json`을 endpoint와 `2026-08-20 23:59 KST` 만료일 정본으로 추가했다.
- 모든 에이전트가 세션 시작 시 실행할 `Sync-TeamLanEndpoint.ps1`을 추가했다. 스크립트는 기존
  `.vcxproj.user`의 다른 property를 보존하고 Server/Client debugger property만 stage 후 commit한다.
- endpoint IPv4를 가진 PC는 `server-host`로 판정해 `Server + Client` profile을 안내하고, 다른 PC는
  `client`로 판정해 Client project만 시작하도록 안내한다. 명시적 `-Role Server`도 주소 소유 검사를
  통과해야 하므로 잘못된 PC가 중앙 Server 역할을 가장하지 않는다.
- Server 주소를 가진 PC에서는 Debug/Release `Server.exe`의 TCP 7777 LocalSubnet 방화벽 규칙을
  확인하며, 규칙이 없고 관리자 권한이 없으면 필요한 조치를 경고한다.
- world 입장 Server approval과 Character Select Preview 경계는 변경하지 않았다.

## 자동 검증

- Server/Client vcxproj XML parse: PASS
- Server x64 Debug build: PASS
- Client x64 Debug compile/link: PASS
- `Server.exe --contract-test`: `failures : 0`
- ProjectAudit: `78 checks` PASS
- 기본 bind runtime smoke: `0.0.0.0:7777` listener 확인
- `Test-NetConnection 192.168.200.103 -Port 7777`: PASS
- session sync, Server 종료 상태: PASS, `Endpoint status now: not-listening`
- session sync, Server listener 활성 상태: PASS, `Endpoint status now: reachable`
- session sync role auto-detect on Server PC: PASS, `Machine role: server-host`
- session sync explicit Client role: PASS, `Visual Studio start target: Client project only`
- bounded Server exit code: `0`
- smoke 종료 뒤 잔류 TCP 7777 listener: `0`

Client 첫 full link는 기존 실행 중이던 Debug `Client.exe`의 출력 파일 점유로 `LNK1104`가 발생했다.
해당 실행 process만 종료한 뒤 동일 build를 다시 실행해 link PASS를 확인했다.

## 수동 검증 경계

다른 팀원 PC에서 실제 Client UI의 Test/Server Play/Bern/Valtan 명령으로 Server approval까지
확인하는 LAN 수동 smoke는 팀 실행 대기 상태다. 팀원 PC에 기존 `.vcxproj.user`가 있어
`127.0.0.1`을 덮어쓰면 `192.168.200.103`으로 맞추고 project Reload 또는 IDE 재시작이 필요하다.
