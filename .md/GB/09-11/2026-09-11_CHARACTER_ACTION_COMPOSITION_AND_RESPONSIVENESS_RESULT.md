# 일반 클릭 이동 예측 구현 결과

갱신일: 2026-09-12. 대응 계획서는 `2026-09-11_CHARACTER_ACTION_COMPOSITION_AND_RESPONSIVENESS_IMPLEMENTATION_PLAN.md`이다.
이번 후속 요청은 G05의 일반 클릭 이동 반응 개선이며 다른 G의 캐릭터 도구·스킬 취소·외형 변경은 구현 완료 범위가 아니다.

## 현재 상태

일반 이동 가능한 자기 캐릭터가 Server 응답을 기다리지 않고 이동 표시를 시작하도록 소스를 반영했다.
typed 이동 명령 송신 직후 기존 navigation으로 경로를 준비하고 RUN을 요청한다. Engine의 ObjectUpdate가
Level 입력보다 먼저 실행되므로 실제 위치 전진과 모델 갱신은 다음 ObjectUpdate부터 진행한다.

최초 구현은 09-12 23:04 Debug x64 제품 빌드까지 적용됐으나, 사용자가 EXE에서 경로 우회·상하 이동,
좌우 클릭의 위치 튐·즉시 얼굴 회전 회귀를 보고했다. 아래 최초 helper/packet 검사 성공은 이 회귀가 없다는
증거가 아니었다. 실제 경로·Character·카메라 소비자를 대상으로 후속 교정을 진행했다.

**후속 교정 소스와 격리 컴파일·CPU 검증, 09-13 00:42 최종 Debug 제품 빌드까지 완료했다.**
후속 Effect scene player 등록 수정과 함께 현재 제품 dependency 빌드를 확인했다.
`out/BuildPipeline/runs/20260912T154243343Z-debug-product.json`과
`out/FestivalEffect20260913/product-build.log`가 근거다. 빌드 직전 기존 Client/Server가 종료된 것을
확인했으며 에이전트가 프로그램을 종료하거나 Client/UI를 실행·조작·캡처하지 않았다.
새 빌드의 체감과 화면 판정은 사용자 확인이 필요하다.

작업 기준은 `codex/kouku-authored-finale-popup`, HEAD `5168899d671013b48061f0d7b18ea81a86f7e042`와
기존 dirty working copy다. 다른 기능의 수정이 다수 함께 있어 자동 stage/commit/push하지 않았다.

## G05 변경 파일과 동작

| 파일 | 실제 변경과 책임 |
|---|---|
| `Client/Private/PlayerController.cpp` | 기존 `IPlayerCommandSink::Request_MoveGoal` 송신 성공 직후 같은 sequence/goal로 예측 요청. 송신 실패는 예측을 시작하지 않음. Debug teleport 승인 시 미처리 예측을 폐기 |
| `Client/Public/Character.h`, `Client/Private/Character.cpp` | 자기 캐릭터만 예측 상태를 소유. 기존 `CNavPathFollower`를 임시로 준비한 뒤 성공 시 교체. 즉시 locomotion 요청과 프레임 위치 표시, snapshot 보정 연결 |
| `Client/Public/LocalMovePrediction.h` | 최신 클릭 sequence, 처리 ACK, snapshot freshness, 경유점 외삽, 오차 보정 상태. socket/navigation query를 소유하지 않는 표준 C++ helper |
| `Client/Private/ClientReplication.cpp` | 일반 player snapshot과 지연 class replacement 모두 예측 snapshot을 먼저 전달하고 기존 action/animation snapshot을 적용 |
| `Shared/Public/Network/PacketMessages.h`, `Shared/Private/Network/PacketMessages.cpp`, `Shared/Public/Network/PacketType.h` | protocol 81. player별 처리 sequence·실효 속도·이동 가능 여부·활성 목표 여부·다음 경유점 22 byte를 기존 player wire 끝에 추가. finite/속도/상태/boolean 검증 |
| `Server/Private/GameRoom.cpp` | 기존 이동 검증 결과를 read-only snapshot으로 제공. 태세 배율 포함 실효 속도와 현재 Server 경유점 사용. 기존 navigation/collision/스킬 판정 유지 |
| `Client/Private/NetworkManager.cpp`, `Server/Public/ClientSession.h`, `Server/Private/ClientSession.cpp` | 양쪽 gameplay socket에 TCP_NODELAY 적용. 실패 이유는 기존 연결 오류 처리로 전달 |
| `Client/Default/Client.vcxproj`, `Client/Default/Client.vcxproj.filters` | 새 helper header를 기존 PlayerController filter에 등록 |
| 기존 NetworkProtocol/CharacterSelectIsolation harness, `ServerGameplayContractTests.cpp` | 관련 protocol·예측 상태·Server snapshot assertion 추가. 새 독립 harness 없음 |

`iLastProcessedMoveSequence`는 승인 sequence가 아니다. Server가 이동 요청을 거절해도 처리 sequence는 진행하므로
Client는 `hasMoveGoal`과 권위 위치로 정지·기존 이동 지속을 구분한다. 최신 미처리 클릭이 남아 있으면 이전 IDLE 응답으로
로컬 경로와 RUN을 취소하지 않는다. 더 최신 클릭은 이전 클릭을 대체한다.

Server 처리 전 표시에는 기존 Client navigation을 사용한다. 처리 후에는 Server 위치·다음 경유점·속도에서 짧게 외삽하며
다음 경유점을 넘지 않는다. 외삽 상한은 150ms, 추정 편도 지연 상한은 100ms, 입력/snapshot 무응답 상한은 350ms다.
최초 구현은 작은 표시 차이를 80ms 동안 줄이고 2m 초과 차이를 즉시 맞췄다. 사용자 회귀 뒤 이 보정과
회전·경로 소비자를 아래 G05-01과 같이 교정했다.
이동 불가·피격/강제 이동·사망·패턴 구속·마리오에서는 예측을 해제한다. class/world 교체는 Character 객체 수명으로
이전 예측을 폐기하고 다른 player는 기존 2 tick 보간을 사용한다.

## 최초 구현에서 실행한 검증

로그와 임시 OBJ/PDB는 Git 제외 `out/LocalMovePrediction20260912/`에 있다.

| 검증 | 결과와 증거 |
|---|---|
| 팀 LAN 동기화 | `server-host`, `192.168.0.14:7777` reachable, firewall ready. 설정 스크립트 실행 |
| Shared Debug x64 빌드 | PASS. `shared-build.log` |
| 기존 NetworkProtocolHarness Debug x64 빌드 | PASS. `protocol-build.log` |
| `NetworkProtocolHarness.exe --move-prediction-only` | 101 PASS, 실패 없음. roundtrip, 거절/정지, 유효/잘못된 속도·경유점·상태, truncated/atomic decode 등. `protocol-test.log` |
| 기존 CharacterSelectIsolationHarness Debug x64 빌드 | PASS. `presentation-build.log` |
| `CharacterSelectIsolationHarness.exe --presentation-contract-only` | PASS. 첫 Update 위치 전진, 이전 IDLE와 최신 클릭, 재클릭, ACK/거절, 속도/경유점 상한, stale/timeout, 큰 위치 차이/잠금/reset, sequence wrap. `presentation-test.log` |
| Client 변경 C++ 최소 컴파일 | Character, PlayerController, ClientReplication, NetworkManager Debug x64 모두 PASS. 제품과 동일한 MSVC 14.44 C++20 옵션으로 별도 OBJ/PDB 사용. `client-compile/results.json`, 각 `.log` |
| Server 변경 C++ 최소 컴파일 | GameRoom, ClientSession, ServerGameplayContractTests Debug x64 모두 PASS. `server-compile/` 로그. 추가 Server assertion의 실제 실행은 아직 하지 않음 |
| Client project/filter XML | 두 파일 parse PASS |
| 변경 범위 whitespace | `git diff --check` PASS |
| 최초 제품 Debug x64 빌드 | 09-12 23:04 PASS. `out/BuildPipeline/runs/20260912T140418074Z-debug-product.json`. 이후 사용자 EXE 검토에서 아래 회귀가 보고됨 |

0.3초는 사용자가 보고한 체감이다. 원인 조사에서 확인한 30Hz 2 tick 보간 약 66.7ms에 통신/Server tick 대기가 더해지는
구조와 첫 RMB 즉시 송신을 구분했다. TCP_NODELAY의 실제 개선량이나 최종 화면 입력 지연을 자동 측정한 것으로 기록하지 않는다.
상태 helper 검사는 실제 Client 화면에서의 다음 프레임 표시 성공을 대신하는 증거가 아니다.

## G05-01. 사용자 실행에서 발견한 회귀 교정

| 원인 | 실제 수정 |
|---|---|
| Server가 현재 얼굴 방향으로 전진해 반대 클릭 때 돌아가는 궤적, Client는 목표 직선으로 예측 | `GameRoom_PlayerSimulation.cpp`의 일반 MOVE에서 위치를 다음 경유점 방향으로 전진. Server의 최단 540도/초 회전과 collision·ground 검증은 유지. 같은 시각 다른 작업의 GameRoom 분할 뒤 실제 소비자에 수정이 보존됨을 확인 |
| Client 격자 검사에서 연속된 두 셀 경계 통과를 대각선으로 오판 | `NavGrid.cpp`의 segment walkability를 실제 격자 경계 순서로 검사. 막힌 모서리·경계·높이 제한을 보존 |
| Client 경로는 기본 step 0.6, Server는 맵별 navpolicy를 사용 | 기존 publisher가 양쪽에 배포한 동일 `.navpolicy`를 `MapNavigationContract → Loader → CNavigation`에 연결. prototype/Clone이 숫자를 보존하며 Character가 `Get_MaxStepHeight()`로 경로를 준비. 월드별 상수·일괄 1.0 상향·새 wire 필드는 없음 |
| 먼 경유점 Y를 향해 이동 도중 미리 올라가거나 내려감 | `NavPathFollower.cpp`는 XZ 속도로 이동하고 현재 XZ의 지면 Y를 샘플링. `LocalMovePrediction`도 먼 경유점 Y를 외삽하지 않고 Character가 현재 지면을 적용 |
| ACK 위치 오차를 고정 80ms에 소거해 급격히 밀리고 2m에서 snap | 일반 연속 위치의 correction 기간을 `max(80ms, 오차 / max(1m/s, 이동속도))`로 결정. 10m 초과 잘못된 표시와 실제 Server 위치 불연속은 reset |
| 예측 경로가 얼굴을 즉시 회전시키고, ACK와 Character가 각도를 이중 보간 | Character의 공통 `Update_PresentationYaw`가 최단 각도를 720도/초로 소비. 위치는 즉시 전진하며 helper는 목표 방향만 제공. ACK 위치 보정에 yaw를 섞지 않음 |
| 일반 예측에서 SPACE/스킬의 기존 2 tick 보간으로 돌아갈 때 표시 위치 되감김 | 최초 SKILL handoff에만 현재 표시 잔여 offset을 120ms에 줄임. 서버 스킬 위치·시간은 그대로이며 강제 상태·teleport·취소 때 잔여 상태를 폐기 |
| KoukuSaydon/Character Select profile의 followResponse 0이 즉시 카메라 이동을 선택 | 두 JSON과 fallback을 12로 변경. 기존 exponential follow에 실제 dt를 적용하고 cinematic override·target/world/free 전환 상태를 분리. 일반 SPACE 이동도 같은 카메라 감쇠를 사용 |

`CNavigation::Create_NavGrid`의 새 네 번째 maxStep 인자는 기본값 0.6으로 기존 명시적 raw/editor 호출을
보존한다. 제품 맵 Loader만 유효한 배포 정책을 요구하고 정책 오류는 기존 Loader 실패 이유로 전달한다.
Client/Server 정책 파일은 실제로 존재하므로 publisher를 재실행하거나 생성물을 직접 편집하지 않았다.
SPACE는 현재 카메라 복귀 키가 아니라 typed skill 입력이다. 이번 수정은 Client 스킬 이동 예측을 추가하지 않는다.

### 후속 재현과 검증 증거

- Server 실제 MOVE 블록의 CPU 재현: 속도 6m/s·30Hz·180도 반대 클릭에서 이전 첫 tick은
  X +0.0618/Z +0.1902로 반대 방향에 전진했다. 교정 뒤 X 0/Z -0.2, 5/10 tick은 Z -1/-2이며
  yaw 18/90/180도 회전은 유지됐다. 이 검사는 full GameRoom simulation 대신 실제 이동 블록을 추출한 검사다.
- 실제 설치 맵 375개 경로: 현재 지면 대비 0.005m 초과 높이 차이 64 → 0.
  첫 격자 교정에서 Server 직선 대비 Client 우회 9 → 4, 남은 Bern 4개는 정책 0.6/1.0 차이로 확인해
  위 정책 소비자를 추가했다. 최종 정책 연결은 실제 reader → prototype → Clone → getter → path 요청을
  소비하며 같은 375개에서 불필요 우회 0, 높이 오차 0을 확인했다. 정책 실패·기존 값 보존 13개도 PASS.
- 막힌 모서리·정확한 대각·내부/외부 경계·거의 모서리·동적 벽 해제 8개 CPU 검사 PASS.
- 실제 Character 메서드 9개 추출의 33개 검사는 즉시 위치·RUN, 179↔-179 최단 회전, 60Hz에서 최대
  12도/프레임, 현재 지면, SKILL 첫 프레임 연속성과 강제 상태 폐기, 정책 getter 전달을 검사해 PASS.
- 30Hz ACK·60Hz update·표시 차이 3m에서 이중 yaw 보간은 반대 목표 각도 오차가 2초 뒤에도 66.78도였다.
  Character를 단일 회전 소유자로 둔 비교는 동일 X 이동을 유지하며 0.25초에 목표 180도 회전을 마쳤다.
- 카메라 실제 메서드 추출 22개 검사 PASS: 10/30/60/144Hz, 일반 이동·8m dash, 큰 warp, dt 0,
  cinematic override/복귀. `out/CameraFollowResponsiveness20260912/validation.json`과 compile 로그.
- 기존 primitive 검사에 ACK 보정 속도·2m 초과 연속 오차·실제 teleport·최신 클릭·먼 경유점 Y·반복 ACK 방향
  사례를 추가했다. 12개 그룹 PASS, 새 회귀 3개 그룹에는 57개 assertion이 있다. 변경 전 기준본에서는
  새 3개 그룹이 모두 실패했다. 과거 먼 경유점 Y 보간을 정답으로 둔 assertion도 현재 지면 계약으로 교정했다.
- 최종 Character/ClientReplication, Camera_Free/ArenaCameraProfile, MapNavigationContract/Loader와
  분할된 GameRoom_PlayerSimulation의 전체 번역 단위 격리 Debug x64 컴파일 PASS.
  실제 Engine navigation 네 번역 단위와 Cell은 CPU probe 링크까지 PASS. 제품 링크를 대신한 증거는 아니다.
- 두 camera JSON, 현재 Client project/filter·Engine/Server project XML parse와 변경 범위
  `git diff --check` PASS. 최종 focused·camera receipt의 source SHA가 현재 파일과 일치함을 확인했다.

주요 근거는 `out/LocalMovePrediction20260912/navigation/`,
`out/LocalMovePredictionRegression20260912/{Compile,Focused}/`다. 임시 추출 CPU 검사는 out 안에만 두고
새 제품 runtime이나 영구 독립 harness를 추가하지 않았다. 변경 기능의 격리 compile/test와 제품 최종 링크,
사용자 화면 판정은 서로 다른 상태로 기록한다.

최종 focused receipt는 `out/LocalMovePredictionRegression20260912/Focused/validation-summary.json`,
정책을 포함한 경로 receipt는 `out/LocalMovePrediction20260912/navigation/policy/policy_regression_receipt.json`이다.

## 남은 적용과 사용자 확인

1. 다른 빌드와 제품 프로세스가 종료된 상태에서 Engine → Shared → Server → Client의 최종 Debug 제품 빌드를 완료했다.
2. 최초 구현의 통신 형식은 81이며 이번 후속 교정에서는 그대로다. Server와 Client 모두 교정된 빌드로 재시작해야 한다.
3. 이 PC는 `server-host`이므로 사용자가 Visual Studio의 `Server + Client` profile에서 `Ctrl+F5`로 실행한다.
4. Lobby → Character Select 또는 Bern/KoukuSaydon에서 정지 중 우클릭, 이동 중 반대 방향 재클릭, 벽 근처 클릭,
   SPACE·스킬·피격 직후 이동을 확인한다. 직선 이동·상하 높이·좌우 전환·최단 얼굴 회전·카메라 추적을
   사용자가 관찰하고, 서버 제한 상태에서 임의로 이동하거나 보정으로 크게 되감기는지도 확인한다.

이번 범위는 일반 클릭의 즉시 표시와 서버 경유점 예측이다. 서버와 동일한 전체 nav/collision 고정 tick 재시뮬레이션,
스킬 취소 deadline, 전역 inputOrder, action BUFFERED/APPLIED 결과는 포함하지 않는다. Client navigation과 Server collision의
차이가 큰 장소에서는 보정이 보일 수 있으므로 실제 조작감·시각 결과는 사용자 확인 뒤 판단한다.
