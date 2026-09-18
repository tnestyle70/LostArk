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


## G01. 2026-09-19 현재 endpoint 반영

### 완료한 변경

현재 팀 endpoint를 `10.16.127.103:7777`로 안내하도록 `AGENTS.md`, `CLAUDE.md`,
`.md/TEAM/README.md`, `TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`, `네트워크연결가이드.md`를
갱신했다. 가이드의 현재 주소 `.4`·`.103` 축약을 전체 IPv4로 바꾸었고 직전 endpoint는
`192.168.0.14:7777`로 구분했다. 임시 다른 Server 확인 명령은 이전 IP를 그대로 실행하지
않도록 확인할 IPv4를 입력받는다. 위 2026-08-08 결과와 다른 날짜의 기록은 보존했다.

Server bind `0.0.0.0:7777`, 환경변수 우선순위, LocalSubnet 방화벽, 격리 테스트의
loopback과 2026-09-30 만료 계약은 바꾸지 않았다. 대응 기존 PLAN에 G01 변경 범위를 추가했다.

### 문서 검증과 남은 경계

- 활성 공용 문서 5개의 현재 endpoint가 모두 `10.16.127.103`인 것을 검사했다.
- 구주소 `192.168.0.14`는 연결 가이드의 직전 endpoint 설명 2곳에만 남는다.
- `.4`·`.103` 축약 잔존 0건, 가이드 JSON 예시와 현재 `TeamLanEndpoint.json` 전체 객체 일치 PASS.
- 이 문서 변경의 `git diff --check` PASS.

### 코드·설정과 Release 빌드·sync 검증

`TeamLanEndpoint.json`의 serverHost, `NetworkManager.cpp`의 compiled fallback과
`Client.vcxproj`의 x64 Debug/Release debugger 환경값을 `10.16.127.103`으로 맞췄다.
Sync 뒤 Git 제외 Client user project의 host와 Server user project의 bind도 다시 확인했다.

- Release Product 빌드 PASS. Client 단계 29,491ms, 전체 32,911ms다.
- 새 Client 산출물은 `NetworkManager.obj` 1개와 `Client.exe` 1개다.
  Engine/Shared/Server의 object 재생성은 0개다.
- 빌드 증거는 `out/BuildPipeline/runs/20260918T172954232Z-release-product.json`이다.
- root의 재Sync 실측은 `Machine role: server-host`, TCP 7777 `LocalSubnet` firewall ready,
  Server bind `0.0.0.0:7777`, Client endpoint `10.16.127.103:7777`,
  `Endpoint status now: reachable`이다.
- reachable은 사용자가 켜 둔 Server에 대한 probe 결과다. 에이전트가 Server/Client/UI를
  실행한 결과로 기록하지 않는다.

### 남은 배포·수동 확인

새 endpoint를 사용하는 배포 실행기·설치기와 ZIP은 생성 중이며 이 절에서 완료로 기록하지 않는다.
다른 PC의 TCP 연결·실제 입장·다인 화면 확인도 아직 수행하지 않았다.

### 새 endpoint ZIP 생성 완료

- 배포: `C:\Users\user\Desktop\LostArk-Release-20260919-v2\LostArk-Release-20260919-10.16.127.103.zip`
- 크기: 97351354 bytes; SHA-256: `77cc6119ada839d64439c92bb96cc72deedb5a3675cd8646d91766456789ba77`.
- main `911fd4af9`에 로컬 endpoint 변경을 반영한 Release와 새 런처/설치 스크립트를 포함한다.
- 필수 Data JSON 1,188개, Client DataFiles 150개, Server DataFiles 159개를 포함하며 Resources는 별도 Drive 전달이다.
- ZIP CRC와 원본/패키지 SHA-256 일치 PASS. 런처 `--check`에서 `Server=10.16.127.103`, `ClientStarted=False` 확인.
- endpoint 계약 테스트 3개 PASS. Client/Server/UI를 새로 실행하거나 설치하지 않았다.


## G02. 2026-09-19 현재 PC의 로컬 4클라이언트 Release endpoint 반영

### 반영한 현재 계약

사용자의 새 요청에 따라 현재 endpoint를 `192.168.0.14:7777`로 변경했다. root는
`TeamLanEndpoint.json`, Client compiled fallback과 x64 Debug/Release debugger 환경값을
맞췄고 sync를 수행했다. 활성 공용 문서 5개도 같은 현재 주소를 사용하며, 네트워크
연결 가이드의 직전 endpoint 2곳은 `10.16.127.103`으로 구분했다. 위 G01과 기존 날짜의
실행·배포 기록은 당시 주소와 결과를 보존했다.

### 확인한 설정과 문서

- 현재 PC의 `Wi-Fi 2`가 `192.168.0.14/24`, `AddressState=Preferred`를 소유함을 읽기 전용으로 확인했다.
- root의 sync 실측은 `Machine role: server-host`, TCP 7777 `LocalSubnet` firewall ready,
  Server bind `0.0.0.0:7777`, Client endpoint `192.168.0.14:7777`, probe `not-listening`이다.
- `not-listening`은 sync 시점에 Server가 수신 중이지 않다는 결과이며 설정 실패가 아니다.
- 활성 문서의 현재 주소와 가이드의 JSON 예시가 현재 `TeamLanEndpoint.json`과 일치한다.
- 이번 문서 변경의 `git diff --check` PASS.

### 빌드·배포·사용자 확인 경계

현재 주소를 포함하는 Debug/Release Product 빌드가 모두 PASS했다. Debug 182713ms,
Release 249917ms이며 정본 Engine/Shared/Server/Client를 모두 빌드·배포했다.
증거는 `out/BuildPipeline/runs/20260918T190723672Z-debug-product.json` 및
`out/BuildPipeline/runs/20260918T191133844Z-release-product.json`다. source fallback과 Debug/Release
debugger 값은192.168.0.14로 일치하며 JSON/XML parse PASS. 기존 코드 페이지·PDB 경고는
남았지만 이번 빌드의 컴파일·링크 오류는0이다. 사용자가 Ctrl+F5 중 보고한 오류의
구체 메시지는 받지 않았으므로 원인을 동시 빌드로 확정하지 않는다. 빌드 완료 뒤
사용자가 재실행하기로 했다.
현재 PC의 실제 4클라이언트 접속·입장·패턴 화면 검증은 사용자가 수행한다. 문서 작업에서
Server/Client/UI는 실행하지 않았다. 이번 요청에는 ZIP 재생성이 없으므로 기존 v3 `.103`
ZIP은 수정하지 않으며, 그 ZIP이 새 `.14` compiled fallback을 포함한다고 설명하지 않는다.
