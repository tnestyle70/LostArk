# 로컬 F5 endpoint 수정 결과

## 구현

WSA 10060은 Lobby TCP 접속 이전 단계에서 팀 IP 192.168.0.22에 도달하지 못한 오류다.
이전 로컬 debugger host를 세션 LAN sync가 덮어썼다. 공유 endpoint는 그대로 유지하고
Sync-TeamLanEndpoint.ps1에 명시적 개인 Local/Team 선택과 기본 Saved 보존을 추가했다.
현재 Client/Server Debug·Release user 설정은 localhost다. 사용자 환경변수와 작업 디렉터리를
보존하며 선택은 Git 제외 Client.vcxproj.user에 기록했다. NPC·맵 데이터와 C++ 변경은 없다.

## 실행한 검증

- Local 선택 후 옵션 없는 재동기화: Local / server-host / 127.0.0.1:7777 유지.
- 독립 검토에서 발견한 ValidateSet 대소문자 보존 문제 수정. `-EndpointMode local` 후
  기본 Saved 실행 성공 및 canonical Local 저장 확인.
- MSBuild Debug Client 유효 환경값: LOSTARK_SERVER_HOST=127.0.0.1.
- MSBuild Debug Server 유효 인자: --bind-address 127.0.0.1.
- MSBuild Release Client 유효 환경값: LOSTARK_SERVER_HOST=127.0.0.1.
- 기존 Debug Server를 headless / loopback / smoke-timeout-ms 10000으로 실행.
  모든 world 초기화 뒤 Listening 로그 확인, 실제 TCP 접속 성공, exit 0 정상 종료.
  로그: out/LocalF5Endpoint20260927/server.stdout.log 및 server.stderr.log.
- 검증 후 7777 listener 없음. git diff --check 통과.

## 사용자 실행 / 미검증

현재 열려 있는 VS는 debugger 값을 캐시할 수 있으므로 solution Reload 또는 IDE 재시작 후
Debug / x64, Server + Client profile을 선택하고 F5를 누른다. Client Only는 Server가 이미
실행 중일 때만 사용한다. 팀 접속 복귀는 sync -EndpointMode Team이다.
설정/script 변경이므로 C++ 재빌드·데이터 게시는 수행하지 않았다. F5 자체는 IDE 설정에 따라
빌드할 수 있다. Client UI를 실행하지 않았으므로 실제 Lobby 입장·화면 결과는 사용자 확인이다.
