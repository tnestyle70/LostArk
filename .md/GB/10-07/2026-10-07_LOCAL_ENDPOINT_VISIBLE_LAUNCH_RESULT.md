# 개인 Client endpoint 및 화면 창 실행 결과

사용자가 이 PC의 Client 설정을 `172.30.1.96`으로 변경하고 Server CMD와 Client 창을
직접 띄우도록 명시적으로 요청했다. 현재 Wi-Fi가 `172.30.1.96/24`를 소유한다.

- `Client/Default/Client.vcxproj.user`의 `LOSTARK_SERVER_HOST`를
  `192.168.0.14`에서 `172.30.1.96`으로 변경했다. 다른 속성은 보존했다.
- `.vcxproj.filters`에는 endpoint 설정이 없으며 변경하지 않았다.
- Server 개인 설정은 이미 `--bind-address 0.0.0.0`이었다.
- 기존 Release Server를 `Server/Default`에서 화면에 보이는 `cmd.exe /k`로 실행했다.
  headless, smoke timeout, 출력 리디렉션을 사용하지 않았다.
- 기존 Release Client를 `Client/Default`에서 동일 host 환경변수로 실행했다.
  EXE 직접 실행은 `.vcxproj.user`를 읽지 않으므로 실행 환경에 별도로 지정했다.

Server PID33884의 `0.0.0.0:7777` listener와 `172.30.1.96:7777` TCP probe 성공을
확인했다. CMD는 Windows Terminal 창 제목과 window handle로 확인했다.
Client PID5180의 window handle 및 응답 상태, 시작 로그의
`Lobby.Start_Level`과 `Initialize` ready를 확인했다. 게임 입장·조작이나 화면 캡처,
다른 PC의 접속 검증은 수행하지 않았다. 두 프로세스는 사용자 사용을 위해 유지했다.

Client/Server 개인 XML과 Client filters XML parse, `git diff --check`를 확인했다.
빌드는 하지 않았다. 변경 전 개인 XML은
`out/Network/20261007-172-30-1-96/Client.vcxproj.user.before`에 보관했다.

이번 변경은 사용자가 지정한 개인 설정과 실행에 한정했다. 공유 endpoint JSON,
C++ fallback, 공용 project 및 portable 계약은 변경하지 않았다. 공유 계약의
2026-10-03 만료를 우회하지 않았으며 sync도 실행하지 않았다. 추후 갱신된 Team
sync를 실행하면 이 개인 host가 공유 정본 값으로 다시 동기화될 수 있다.

## Debug·Release 개인 VS 설정 명시

이후 사용자가 Debug Lobby의 `192.168.0.14:7777 / WSA10060` 실패를 제시하고
`vcx user filters` 수정 범위를 지정했다. `Client.vcxproj.user`의 host를
`Debug|x64`, `Release|x64` 각각의 조건부 PropertyGroup에 명시했다.
두 구성 모두 `LOSTARK_SERVER_HOST=172.30.1.96`이며 기존
`LostArkEndpointMode=Team`은 보존했다. `.vcxproj.filters`에는 endpoint나 debugger
환경변수 항목이 없으므로 주소를 넣지 않았다.

MSBuild의 `-getProperty:LocalDebuggerEnvironment`를 두 구성에 각각 실행하여
둘 다 `LOSTARK_SERVER_HOST=172.30.1.96`으로 평가되는 것을 확인했다. 이 명령은
속성 조회이며 제품 컴파일이나 실행을 하지 않는다. 개인 XML·filters XML parse와
`git diff --check`도 통과했다. 변경 전 개인 XML은
`out/Network/20261007-172-30-1-96/Client.vcxproj.user.before-explicit-configs`에
보관하고 교체 직전 byte 동일성을 확인한 뒤 원자적으로 교체했다.

당시 실행 중 VS의 시작 시각은10:06:48, 최초 개인 주소 수정은10:12:44였다.
IDE가 변경 전 debugger 설정을 보관했을 가능성이 있으므로 열린 VS에는 Client project
Reload 또는 VS 재시작이 필요하다. 이 기록에서 IDE 캐시를 직접 읽거나 UI를 조작하지는 않았다.
Client·Server도 추가 실행하지 않았으며 실제 사용자 접속 성공은 별도 확인이다.

최종 반영은 개인 `.vcxproj.user`에 한정했다. `NetworkManager.cpp`와
`LocalServerEndpoint.user.json`은 조사 시작 시 원본 byte로 유지했고 공유 endpoint,
코드 기본값과 filters는 변경하지 않았다. 직접 EXE 실행이 `.vcxproj.user`를 읽는 것으로
설명하지 않는다. 직접 실행은 기존처럼 실행 환경의 host를 별도로 지정해야 한다.
