# 팀 LAN endpoint 10.207.18.151 전환 구현 계획

작성일: 2026-08-27

상태: 구현 및 자동 검증 완료, Server host listener 확인 대기

## 목표

공유 Server endpoint를 `10.207.18.151:7777`로 전환해 모든 Client가 같은 주소를 사용하게 한다.
Server listener bind `0.0.0.0:7777`, 환경변수 우선순위와 격리 하네스의 명시적
`127.0.0.1` 계약은 유지한다.

## 변경 범위

- `Tools/Network/TeamLanEndpoint.json`의 `serverHost`
- `Client/Private/NetworkManager.cpp`의 환경변수 미지정 시 기본 host
- `Client/Public/NetworkManager.h`의 port-only 연결 overload를 공용 host resolver로 통합
- `Client/Default/Client.vcxproj`의 x64 Debug/Release debugger environment
- endpoint 정본을 안내하는 공용 문서의 현재 주소 표기
- Git 제외 `Client.vcxproj.user`는 sync script로 갱신

## 검증

1. JSON과 project XML parse
2. `Sync-TeamLanEndpoint.ps1` 실행 결과와 Client user setting 확인
3. 현재 코드·설정·공용 문서에서 이전 endpoint literal이 남지 않았는지 검사
4. Client x64 Debug/Release build
5. NetworkProtocolHarness Debug/Release와 Server Release contract test
6. `10.207.18.151:7777` probe 결과 기록
7. `git diff --check`

현재 실측에서 `10.207.18.151`은 ICMP 응답하지만 TCP 7777은 아직 수신 중이 아니다. 설정
반영은 완료할 수 있으며 실제 접속은 해당 주소를 소유한 PC에서 Server를 시작한 뒤 확인한다.
