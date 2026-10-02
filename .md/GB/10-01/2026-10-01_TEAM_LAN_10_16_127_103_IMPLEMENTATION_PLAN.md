# 팀 LAN 주소 변경과 새 Release ZIP 구현 계획

## G00. 목표와 실측

사용자가 지정한 `10.16.127.103:7777`로 Client 기본값·Visual Studio 설정·portable
런처를 맞추고 이전 Guide/레이드/강화 수정이 포함된 새 Release ZIP을 만든다.
현재 PC의 Wi-Fi 2가 이 IPv4/24를 실제 소유한다. Client·Server는 종료된 상태이며
초기 TCP probe는 연결 거부다. IP 소유권 확인과 서버 실행/입장 승인을 구분한다.

## G01. 변경 범위와 연결

- `Tools/Network/TeamLanEndpoint.json`의 serverHost를 새 주소로 변경한다. 기존
  09-30에 끝난 임시 계약은 이번 작업일10-01 23:59 KST까지 갱신하고 공용 문서도
  일치시킨다. AllowExpired로 우회하지 않는다.
- `Client/Private/NetworkManager.cpp`의 기본 접속 주소와 `Client.vcxproj`의
  x64 Debug/Release 환경 변수를 변경한다. 명시적 환경 변수 우선은 유지한다.
- Sync-TeamLanEndpoint.ps1의 Team 모드로 Git 제외 Client/Server .vcxproj.user를
  동기화한다. Server는0.0.0.0:7777로 수신한다. .vcxproj.filters는 소스 분류 파일이며
  접속 주소 항목이 없어 재배치하지 않는다.
- portable builder의 HOST, 대응 endpoint 검사 fixture, 실행 README를 변경한다.
  생성되는 launcher contract·manifest가 같은 주소를 사용해야 한다.
- AGENTS/CLAUDE/팀 사용서의 현재 계약을 갱신한다. 기존 ZIP의 hash·이전 주소 등
  역사 기록은 과거 배포본 사실로 보존한다.

## G02. 검증과 전달

JSON/XML parse, 설정 동기화와 TCP7777 LocalSubnet 방화벽 상태, 현재 endpoint
참조 일치, NetworkProtocolHarness와 관련 Server 검증을 확인한다. Client Debug와
정식 Product Release Build를 수행한다. 지원되는 headless probe로 실제 새 주소의
서버 protocol/입장 승인을 확인하며 Client/UI는 실행하지 않는다. 검사 용도로 시작한
Server만 종료하고 기존 사용자 프로세스는 임의 종료하지 않는다.

새 이름 `LostArk-Release-20261001-10.16.127.103.zip`으로 패키징하고 ZIP CRC,
manifest hash·binary pins·launcher --check의 endpoint를 검증한다. Resources는 기존
외부 폴더를 사용한다. 다른 PC에서의 실제 LAN 플레이와 화면 확인은 구분해 기록한다.
