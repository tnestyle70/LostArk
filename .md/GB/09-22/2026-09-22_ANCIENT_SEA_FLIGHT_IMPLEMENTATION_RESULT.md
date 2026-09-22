# 고대의 바다 비행 구현 결과

## G00. 소스 및 게시 상태

고대의 바다 vehicle 9523의 기존 E skill 98523 / `npc_sk_look`을 Server 권위 비행으로 연결했다. 별도 모델이나 애니메이션 리소스는 추가하지 않았다. 기존 미커밋 수정은 유지했고 branch 변경·commit·Client/UI 실행은 하지 않았다.

- E 첫 입력: 1.8초 이륙 후 날개 반복 유지. 다시 E: 현재 고도에서 하강·착륙 후 지상 이동 복귀.
- FLYING에서 WASD 수평 이동, Space hold 상승, 왼쪽/오른쪽 Ctrl hold 하강. Space+Ctrl은 상쇄된다. 고도는 검증 지면 기준 0.1~20m이며 실제 착지는 E로 한다. hover 1.2m, 수평 8m/s, 수직 4m/s는 profile 저작값이다.
- 용 방향은 최대 150도/초로 회전하며 속도는 지수 보간한다. Server navigation XZ와 collision 경계를 유지하므로 공중에서 벽·전투 전이 게이트를 우회하는 기능은 아니다. 350ms 입력 단절은 감속 정지하고 강제 하차는 마지막 검증 지면으로 복귀한다.
- Bern/Character Select의 기존 탈것 허용을 유지하며 Valtan/Kouku Arena에서는 고대의 바다만 예외로 허용한다. 사망·경직·구속·변신·Mario·CardMaze 등 기존 탑승 제약은 유지한다.

`Publish-VehicleProfiles.ps1 -Mode Publish`를 정상 경로로 실행해 `Server/Bin/DataFiles/Vehicles/Vehicles.bootstrap`을 v3으로 게시했다. 7 vehicle / 25 skill, 게시 SHA256은 `154F5CCE9C78581D23EF6A22A7538564C924D4775BD63713D31DC7EAEADCAA38`이다. 실행 중 Server 메모리를 자동 변경하는 경로는 없으며 새 Server 시작이 필요하다.

## G01. public 입력·복제·정본 계약

| 경계 | 실제 계약 |
|---|---|
| Shared protocol | 104. C2S_MOVE에 PLAYER_MOVE_INTENT U8과 vertical F32 추가. GROUND_GOAL은 기존 목표 XZ, vertical 0. VEHICLE_FLIGHT는 길이<=1의 world XZ 입력과 [-1,1] 수직 축이다. Client 위치를 전송하지 않는다. |
| Snapshot | vehicleId 다음에 GROUNDED/TAKEOFF/FLYING/LANDING phase U8, phaseStartTick U32, phaseDurationSeconds F32. 비행은 9523 + VEHICLE_SKILL만 허용하며 잘못된 phase/시계/시간을 write/read에서 거절한다. 양쪽 protocol 104 빌드가 필요하다. |
| Controller | CPlayerController -> IPlayerCommandSink::Request_VehicleFlightInput -> network sink. WASD 축 조합이 바뀔 때 현재 camera XZ basis를 취득하고 같은 조합 hold 중에는 world 방향을 유지해 camera heading 피드백 회전을 방지한다. |
| 입력 소유권 | UI/text/focus/F6/capture 경계는 별도 release gate를 관찰한다. 복귀 시 held 키가 자동 재활성화되지 않는다. 100ms heartbeat, 변경 33ms 간격, zero 입력은 즉시 전송. 실패한 전송은 sequence를 소비하지 않는다. |
| Presentation 정본 | Data/Actors/VehicleCatalog.json의 고대의 바다 E flightWindow: loopStartSeconds=1.8, loopEndSeconds=2.7666667, landingStartSeconds=3.6666667. 0<start<end<=landing<실제 clip duration. |
| Server 정본 | Data/Vehicles/VehicleProfiles.json의 flight는 hoverHeight/maximumHeight/speed/verticalSpeed 네 값만 소유한다. 이륙·착륙 시간을 중복 저장하지 않는다. |
| Publisher | E stable skillId와 presentation을 조인하고 installed WModel 또는 대응 authored boneclip의 실제 길이로 takeoff=start, landing=clipDuration-landingStart를 파생한다. 원본 160/30초와 profile actionDurationMs=5333의 밀리초 반올림을 구분하고, 구간 파생에는 실제 clip 길이를 사용한다. |
| 게시 보존 | 입력 JSON 읽기 전후 SHA256, 교체 직전 입력/목적 파일 SHA256, stage/backup/실패 시 rollback을 사용한다. 잘못된 입력은 기존 게시물을 보존한다. |

수정 파일은 Shared packet message/type, Server ServerPlayer/VehicleCatalog/GameRoom_VehicleRiding/GameRoom_PlayerCommands/GameRoom_Replication, Client ActorCatalog/Controller/typed command sink/NetworkManager/CombatHUD/ClientReplication/Character/Part_Vehicle, 두 Vehicle JSON과 publisher 및 기존 protocol/vehicle contract test다. 신규 C++ 파일은 없으며 project/filter 등록 변경은 없다.

## G02. 애니메이션·머리·카메라·편집 소비자

Part_Vehicle은 같은 원본 clip을 이륙/반복/착륙 구간으로 seek하고 반복 경계의 마지막 0.12초를 시작 pose와 보간한다. rider도 동일 구간과 phase clock을 소비하며 누락 rider clip은 별도 valid animation index로 격리한다. Server가 고도를 소유하므로 기존 clip의 bip001 local Z 상승 변위를 rest 값으로 제거해 고도를 이중 적용하지 않는다.

실제 설치 skeleton의 bip001-neck/neck1/neck2/head와 b_m_00을 읽어 부모 basis에서 제한된 CCD 보정을 적용한다. 매 프레임 원본 pose를 다시 만든 뒤 보정하며 회전 속도·상하 이동을 부드럽게 반영한다. 합성 anchor만 검사한 결과가 아니다.

통합 담당이 Camera_Free와 CharacterModelWorkbench를 연결했다. 좌클릭 drag는 플레이어 follow target 기준 공전이며 heading 최단 회전과 위치/시선 지수 보간, pitch/radius 제한을 사용한다. UI/포커스/연출에서 시작된 held mouse는 공전에 사용되지 않는다. F6·하차·target 교체는 orbit 상태를 초기화한다.

Action Workbench의 고대의 바다 E Details에서 세 flightWindow 값을 실제 clip 길이 안에서 편집하고 기존 stable skill subtree에 저장한다. Save Flight Logic이 별도 draft를 보존하며 build_vehicle_skills.py도 flightWindow를 보존한다. 저장 후 domain publisher를 실행하면 Server 시간이 같은 구간에서 다시 파생된다. 실행 중 도구 Reload나 Server 재시작을 자동 수행한 것은 아니다.

## G03. 실행한 검증

| 검증 | 결과 / 증거 |
|---|---|
| production 12 TU + Server vehicle test TU /Zs | PASS. flight-syntax.log / test-syntax.log. 기존 C4819 경고는 남아 있으며 오류 0. |
| Shared + NetworkProtocolHarness 독립 compile/run | 1240 PASS, failures 0. malformed axis/phase/timing, old protocol, snapshot byte 크기 포함. protocol-result.log. |
| 실제 installed WModel + CModel pose API + Part 메서드 본문 | 1155 PASS, UI=0/GPU=0. 25회 loop cockpit seam 최대 0.0000220618m, 실제 head CCD 변위 0.0189826m, 상승·하강 부호, 장시간 window, 착륙/해제 확인. pose-result.log. |
| 실제 Controller 입력 메서드 + 실제 capture gate | 통합 담당 probe 375 PASS. hold 방향 고정, 새 조합 basis, 대각선, 양방향 수직, UI/F6/capture release gate, heartbeat/실패 재시도/sequence 보존. input-probe-result.log. |
| 실제 Camera 메서드 | 통합 담당 probe 41 PASS, UI=0. camera-probe-result.log. |
| Workbench / Camera 개별 C++ compile | 통합 담당 PASS. CharacterModelWorkbench-compile.log / Camera_Free-compile.log. |
| catalog formatter roundtrip | 통합 담당 7 vehicle 의미상 동일 PASS. vehicle-catalog-roundtrip.json. |
| Publisher 실제 설치 길이 + 정상 게시 | Validate/Publish PASS, 7 vehicle / 25 skill. Server row=1.8 / 1.6666666333333331 / 1.2 / 20 / 8 / 4. |
| Publisher 격리 입력 실패/편집 소비 | 15 PASS. 역순 window/중복 timing 정본/초저속 착륙/다른 탈것 flight/누락 clip/루트 탈출 6사례는 기존 bootstrap SHA 보존. window start를 2초로 바꾸면 파생 Server takeoff만 2초로 반영. publisher-result.log. |
| 변경 범위 git diff --check | PASS. |

모든 위 로그는 `out/AncientSeaFlight20260922/` 아래에 있다. probe 중 Client/Bin의 기존 DLL과 Debug import mismatch를 발견해 Engine/Bin/Debug 및 해당 Debug 의존성으로 실제 pose 검사를 실행했다. 잘못된 0.01 scale로 측정한 중간값은 최종 결과에서 제외하고 제품 modelPreScale 0.0001 / Y -90도와 동일한 basis를 사용했다.

## G04. 통합 검증 및 남은 확인

새 Server Debug EXE(2026-09-22 17:46 빌드)에서 `--vehicle-riding-contract-test`를 실제 실행하여45개 조건 PASS, failures0을 확인했다. E toggle, 원본 action 종료 이후 지속, Space/Ctrl, 최고 고도, 입력 timeout/replay, typed WASD navigation 이동/회전 제한, 착륙/강제 하차, Valtan/Kouku 예외를 포함한다. 증거는 `out/AncientSeaFlight20260922/server-vehicle-contract.log`다. 기존 사용자 Server를 조작한 것이 아닌 별도 headless contract 실행이다. 최종 Product 빌드는 통합 담당 진행 중이다.

Client/UI는 실행하지 않았다. 실제 화면에서 이륙 높이, 날개 연결 느낌, rider 좌석, 머리 회전 강도, drag 공전, raid presentation은 사용자 확인이 남는다. 수치상 실제 모델·본 검증을 GPU 화면 표시 완료로 기록하지 않는다.

## G05. 사용자 마무리 요청 시점의 재확인

19:13에는 현재 Server Debug EXE(18:19 빌드)에서 같은 headless 계약을 다시 실행해
45 PASS, failures0, exit0을 확인했다. 로그와 EXE SHA는
`out/AncientSeaFlight20260922/server-vehicle-contract-final.log` 및
`server-vehicle-contract-final-receipt.json`이다. 입력/캡처/pose 본문은 기존 probe와,
publisher·Vehicle JSON·게시 bootstrap은 기존 검증 자료와 동일함을 재확인했다.
G01의 actionDurationMs 표기는 실제 검증 데이터와 같은5333으로 정정했다.

19:04 정식 Product 전체 PASS 뒤 동시 변경과 셰이더 생성물 정규화를 포함한
최신 정상 Product가 진행 중이다. 정리 시점에 Engine/Shared/Server는 PASS,
Client FXC는 진행 중이며 최종 링크 및 사용자 화면 확인은 아직 완료로 기록하지 않는다.
로그는 `out/CharacterMaterials20260922/product-final-build.console.log`다.
비행 프로토콜104는 새 Server/Client가 함께 사용해야 하며 실행 중 Server를 자동으로
재시작하거나 Client/UI를 실행한 것은 아니다.
