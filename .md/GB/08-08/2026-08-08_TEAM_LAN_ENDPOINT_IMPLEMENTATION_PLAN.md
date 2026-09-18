# 팀 LAN endpoint 공유 기본값 구현 계획

작성일: 2026-08-08

## 목표

서로 다른 PC에서 같은 LAN으로 검증할 때 팀원이 저장소를 pull하고 Visual Studio의 공유
`Server + Client` profile에서 `Ctrl+F5`를 누르면 Server가 모든 IPv4 adapter의 TCP 7777을
수신하고 Client가 팀장 PC `192.168.200.103:7777`로 접속하게 한다.

## 현재 실측과 변경 범위

- 기존 Server 기본 bind와 Client 기본 host는 `127.0.0.1`이어서 다른 PC가 기본 실행만으로는
  접속할 수 없다.
- `Framework.slnLaunch`는 이미 Server 다음 Client 순서로 두 실제 project를 시작한다.
- Server 기본 bind, Client 기본 host, x64 shared debugger setting, 로컬 `.vcxproj.user`,
  팀 계약 문서와 ProjectAudit을 하나의 endpoint 계약으로 갱신한다.
- `Tools/Network/TeamLanEndpoint.json`에 endpoint와 `2026-08-20 23:59 KST` 만료를 저장하고,
  세션 시작 스크립트가 각 PC의 Git 제외 `.vcxproj.user`를 정본과 동기화한다.
- world 입장 승인, 5초 승인 timeout, Character Select의 socket 없는 Preview 계약은 변경하지 않는다.

## 구현 순서

1. Server 기본 listener를 `0.0.0.0:7777`로 변경한다.
2. Client 기본 endpoint를 `192.168.200.103:7777`로 변경한다.
3. Server/Client x64 shared debugger 설정과 현재 로컬 `.vcxproj.user`를 같은 값으로 맞춘다.
4. `AGENTS.md`, `CLAUDE.md`, 팀 게임플레이 사용서의 정본 계약을 갱신한다.
5. `Sync-TeamLanEndpoint.ps1`이 기존 user XML의 다른 설정을 보존하면서 필요한 debugger property만
   stage하고 검증 후 교체하게 한다. endpoint IPv4를 가진 PC는 `server-host`, 나머지는 `client`로
   자동 판정하고 Server host만 TCP 7777 LocalSubnet 방화벽을 확인한다.
6. ProjectAudit가 endpoint 정본, 만료일, source와 shared debugger 값의 불일치를 거부하게 한다.

## 검증

- Server/Client project XML parse
- Server와 Client x64 Debug build
- `Server.exe --contract-test`
- ProjectAudit
- 인자 없는 Server에 smoke timeout만 주어 실제 `0.0.0.0:7777` listener 확인
- `192.168.200.103:7777` TCP connect 확인
- 세션 시작 sync를 Server 종료 상태와 listener 활성 상태에서 각각 실행
- bounded Server 종료 후 잔류 7777 listener 없음 확인
- `git diff --check`


## G01. 2026-09-19 현재 팀 endpoint 교체

사용자가 새 Server 주소 `10.16.127.103:7777` 적용을 요청했다. 현재 Server PC의
`Wi-Fi 2`가 `10.16.127.103/24`를 소유한 것을 확인하고 이전 공용 주소
`192.168.0.14:7777`에서 전환한다. 위 2026-08-08 절과 다른 날짜의 PLAN/RESULT는
당시 구현 증거로 보존하며, 현재 정본은 `Tools/Network/TeamLanEndpoint.json`을 따른다.

root는 endpoint JSON, Client compiled fallback, x64 Debug/Release debugger 설정과
배포 실행기의 접속 환경값을 같은 주소로 맞춘다. 문서 담당은 `AGENTS.md`, `CLAUDE.md`,
팀 README, 게임플레이 인터페이스 사용서와 네트워크 연결 가이드의 현재 endpoint를
함께 갱신한다. 가이드에 남아 있던 `.4`·`.103` 축약은 전체 IPv4로 명확하게 적는다.
Server bind `0.0.0.0`, port `7777`, 만료 `2026-09-30 23:59 KST`, 환경변수 우선순위,
격리 하네스의 명시적 loopback 계약은 유지한다.

JSON/XML parse, sync의 역할·방화벽·endpoint 결과, 필요한 Release 빌드와 배포 파일
일치를 확인한다. 문서는 literal 정합성과 `git diff --check`로 검증한다. Server/Client/UI는
문서 작업에서 실행하지 않으며, 다른 PC의 TCP 연결과 실제 입장은 사용자 확인으로 남긴다.


## G02. 2026-09-19 현재 PC의 로컬 4클라이언트 Release endpoint 교체

사용자가 현재 PC에서 Release Client 네 개를 검증하기 위해 접속 주소를
`192.168.0.14:7777`로 즉시 교체하도록 요청했다. 직전 G01의 `10.16.127.103:7777`
적용 이력은 보존하고, 이번 현재 endpoint만 정본과 활성 공용 문서에 반영한다.

root가 `TeamLanEndpoint.json`의 `serverHost`, `NetworkManager.cpp` compiled fallback,
`Client.vcxproj`의 x64 Debug/Release 환경값을 변경하고 sync와 필요한 Release 빌드를
수행한다. 문서 작업은 `AGENTS.md`, `CLAUDE.md`, 팀 README, 게임플레이 인터페이스
사용서와 네트워크 연결 가이드의 현재 주소를 맞추고, 가이드의 직전 주소를
`10.16.127.103`으로 명확히 구분한다. Server bind `0.0.0.0`, port `7777`, 만료일,
명시적 환경변수 우선순위와 격리 하네스 loopback은 유지한다.

JSON/XML와 활성 문서의 주소 일치, 현재 PC의 주소 소유, sync의 역할·방화벽·probe,
`git diff --check`를 확인한다. Release 빌드 결과와 사용자의 실제 4클라이언트 검증은
RESULT에서 각각 기록한다. 이번 요청에는 ZIP 재생성이 포함되지 않으며, 기존 v3
`.103` 배포 ZIP은 이전 배포물로 그대로 보존한다.
