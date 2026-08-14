# Lobby Bern Canonical Endpoint RESULT

작성일: 2026-08-14

상태: 코드 반영 및 관련 자동 검증 완료. 사용자 Server+Client 수동 입장 확인 대기

## 1. 원인

Lobby 화면 실측에서 일반 제품 Level은 `192.168.200.103:7777`, Debug Test Map Editor는
`127.0.0.1:7777`을 사용했다. Bern 버튼의 `WSA 10060`은 Level 로드 오류가 아니라 팀
Server endpoint에 대한 TCP 연결 시간 초과였다.

기존 `Resolve_MapEditorServerHost()`가 `LOSTARK_MAPEDITOR_SERVER_HOST`를 별도로 읽으면서
Test만 loopback Server에 연결될 수 있었다. 따라서 Test 성공과 Bern 실패가 동시에 발생했다.

## 2. 반영 내용

- `Client/Private/NetworkManager.cpp`
  - `Resolve_MapEditorServerHost()`를 `Resolve_ServerHost()`의 호환 별칭으로 변경했다.
  - 옛 `LOSTARK_MAPEDITOR_SERVER_HOST` 값은 더 이상 endpoint를 분기하지 않는다.
- `Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp`
  - canonical Server를 `192.168.200.103`, 옛 Map Editor 변수를 `127.0.0.1`로 동시에
    설정하는 회귀 테스트를 추가했다.
  - 두 resolver가 반드시 canonical Server를 반환하는지 검증한다.
  - `--lobby-endpoint-fast` 빠른 모드를 추가하고 전체 하네스에도 등록했다.

Test의 Server 승인 후 Development Map Editor 진입 동작은 유지했다. Bern도 기존과 같이
Server의 `S2C_ENTER_ACCEPTED`를 받은 뒤 Loading Level을 통해 진입한다. 로컬 성공 처리나
offline fallback은 추가하지 않았다.

## 3. 자동 검증

### 통과

- ClientFrontendHarness x64 Debug 빌드: 오류 0
- `ClientFrontendHarness.exe --lobby-endpoint-fast`:

```text
[PASS] Lobby Test And Product Worlds Share Canonical Server Endpoint
failures : 0
```

- Client x64 Debug 빌드 및 링크: 오류 0
- `git diff --check`: 오류 없음

기존 SDK/source 인코딩과 DirectXTK PDB에 관한 경고는 남아 있지만 이번 endpoint 변경의
컴파일·링크를 막지 않았다.

### 전체 ProjectAudit

ProjectAudit은 현재 작업트리 전체 기준 37개 check 실패로 종료했다. 대표 원인은 현재
진행 중인 Valtan destruction/data source WIP와 실행 환경의 `python` 명령 부재다. 출력에는
Lobby endpoint resolver 변경과 직접 관련된 실패가 없었다. 관련 endpoint 회귀는 위 전용
하네스에서 별도로 통과했다. 전체 ProjectAudit을 PASS로 기록하지 않는다.

## 4. 수동 검증 전제와 순서

세션 시작 LAN 동기화 결과 이 PC 역할은 `client`이고, 검사 시점의
`192.168.200.103:7777` 상태는 `not-listening`이었다. 따라서 지금 Server가 꺼진 상태에서는
Bern뿐 아니라 단일 endpoint로 정리된 Test도 진입할 수 없다. 이는 의도한 fail-closed 동작이다.

1. 팀 Server PC에서 `Server + Client` profile로 Server listener를 켠다.
2. 이 PC에서는 Client project만 다시 시작한다.
3. Lobby에 서로 다른 `Test (Map Editor)` endpoint 줄이 사라졌는지 확인한다.
4. Test를 눌러 Development Map Editor 진입을 확인한다.
5. Lobby로 복귀한 뒤 Bern을 눌러 같은 Server 승인 후 Bern 진입을 확인한다.

사용자가 위 두 진입을 관찰하기 전에는 수동 visual/runtime PASS로 기록하지 않는다.

## 5. 같은 PC 로컬 Server 후속 설정

사용자 재검증에서 Bern 진입이 계속 실패했다. 추가 실측 결과 이 PC의 활성 IPv4는
`192.168.0.5`이고 팀 endpoint `192.168.200.103:7777`에는 listener가 없었다. Bern 제품
데이터 누락 여부를 분리하기 위해 현재 Debug `Server.exe --contract-test`를 실행했고 다음
Bern 계약이 모두 통과했다.

- Bern world bootstrap 로드
- Bern player spawn, `NPC_BEDA`, trigger, collision box 로드
- Bern collision swept movement 계약
- 전체 Server contract failures 0

따라서 현재 실패 원인은 Bern Level이나 world data가 아니라 실행 중인 Server가 없고 서로
다른 사설망 주소를 사용한 것이다. 저장소의 Git 제외 개인 설정 도구를 다음과 같이 적용했다.

```powershell
powershell -ExecutionPolicy Bypass -File \
  Tools/Network/LocalServerClient.user/Use-LocalLoopback.ps1 \
  -Configuration Debug -ProbeOnly
```

적용 결과:

```text
Server debugger bind: 127.0.0.1:7777
Client debugger endpoint: 127.0.0.1:7777
Local Server probe succeeded: 127.0.0.1:7777
```

`Client.vcxproj.user`와 `Server.vcxproj.user`만 바뀌며 Git 관리 파일은 아니다. Visual Studio가
기존 debugger 값을 캐시하므로 완전히 재시작한 뒤 `Server + Client` profile로 실행해야 한다.
Client project만 실행하면 127.0.0.1 listener가 없으므로 Test와 Bern 모두 실패한다.

로컬 설정 뒤 Client UI를 실행하지 않고 실제 TCP 입장 패킷을 Server에 보내는 smoke도
수행했다. `C2S_ENTER_WORLD(world=BERN, class=LANCE_MASTER)`에 대해 Server가 다음과 같이
유효한 승인을 반환했다.

```text
RESPONSE type=2 protocol=17 world=1 player=1 entity=101 total=18
BERN_LOOPBACK_ENTRY=PASS
```

즉 현재 빌드의 로컬 Server는 Bern 입장을 실제로 승인한다. Visual Studio 재시작 후에도
진입하지 않으면 Lobby 상태 문구와 Server 콘솔 출력으로 그 이후 Client loading 단계의
실패를 별도로 진단한다.

## 6. Bern 승인 후 Lobby 복귀 원인과 수정

로컬 Server 승인과 Bern 화면 진입 뒤 Lobby로 돌아오는 현상을 호출 순서로 재조사했다.
`Send_EnterWorld`는 요청 class를 `m_eLocalCharacterClass`에 저장하지만,
`S2C_ENTER_ACCEPTED` 처리에서 호출한 `Reset_WorldInboundState()`가 그 값을 다시
`CHARACTER_CLASS_ID::END`로 지우고 있었다. 이후 `CLoader::Ready_For_Bern()`은 END class로
`Ready_Character_Rendering`을 호출해 실패하며, MainApp의 activation-failure 복구가 연결을
닫고 Lobby를 다시 로드한다.

승인 처리 직전 요청 class를 보관하고 generation reset 직후 복원하도록 수정했다. 이전
world의 queue와 entity state는 계속 초기화하지만 새 world가 소비해야 하는 승인 요청 class는
유지한다.
