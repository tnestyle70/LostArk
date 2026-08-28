# 팀 LAN endpoint 10.207.18.103 전환 구현 계획

작성일: 2026-08-27

상태: 구현 및 자동 검증 완료, 다른 PC의 LAN 접속 확인 대기

## 목표

공유 Server를 실행하는 PC의 팀 핫스팟 주소 `10.207.18.103:7777`을 단일 Client endpoint로
사용한다. 각 Client가 DHCP로 받은 서로 다른 주소는 자신의 출발지 주소일 뿐 접속 endpoint로
사용하지 않는다. Server listener bind `0.0.0.0:7777`, 환경변수 우선순위와 격리 harness의
명시적 `127.0.0.1` 계약은 유지한다.

## 변경 범위

- `Tools/Network/TeamLanEndpoint.json`의 `serverHost`
- `Client/Private/NetworkManager.cpp`의 환경변수 미지정 시 기본 host
- `Client/Public/NetworkManager.h`의 port-only 연결 overload를 공용 host resolver로 통합
- `Client/Default/Client.vcxproj`의 x64 Debug/Release debugger environment
- endpoint 정본을 안내하는 공용 LAN·설정 문서
- Git 제외 Client/Server `.vcxproj.user`는 sync script로 갱신

## 검증

1. 현재 PC가 `10.207.18.103/24`을 Preferred 상태로 소유하는지 확인
2. JSON과 project XML parse
3. `Sync-TeamLanEndpoint.ps1`의 `server-host`, bind, firewall과 Client endpoint 결과 확인
4. 현재 코드·설정·공용 문서에서 이전 endpoint literal이 남지 않았는지 검사
5. Client x64 Debug/Release build
6. NetworkProtocolHarness Debug/Release와 Server Release contract test
7. `git diff --check`
8. Server 시작 뒤 다른 Client PC에서 `10.207.18.103:7777` TCP probe와 4인 진입 수동 확인

현재 PC는 같은 핫스팟에서 `10.207.18.103/24`과 두 번째 Wi-Fi 주소 `10.207.18.213/24`을
소유하지만 Windows의 우선 인터페이스와 팀 정본은 `10.207.18.103`으로 고정한다. Server 플레이
중에는 이 주소를 소유한 `Wi-Fi 2` 어댑터 연결을 유지한다.
