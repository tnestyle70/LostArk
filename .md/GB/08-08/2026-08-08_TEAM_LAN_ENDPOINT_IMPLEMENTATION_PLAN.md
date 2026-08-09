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
