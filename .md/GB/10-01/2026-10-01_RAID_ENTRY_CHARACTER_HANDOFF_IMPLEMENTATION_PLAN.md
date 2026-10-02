# 레이드 전환 중 캐릭터 생성 이벤트 소유권 수정 계획

## G00. 목표와 실측

베른에서 발탄·쿠크 입장이 승인된 뒤 로딩을 마쳤지만 캐릭터 없이 자유 카메라로
남는 간헐적 현상을 수정한다. 차원술사 수정 commit `8b58818af`를 보존하고
같은 수정본을 포함한 새 Release ZIP을 만든다.

`CMainApp::Apply_LevelRequest`는 비동기 캐릭터 준비 취소가 끝나지 않으면
전환 요청을 유지하고 다음 프레임을 기다린다. 기존 `CLevel_Bern::Update`는
승인을 소비한 첫 프레임에만 반환한다. 다음 프레임의 기존 replication은
이미 승인된 목적지의 PLAYER_SPAWNED를 소비할 수 있다. 기존 레벨이 정리되면
새 레이드는 WORLD_SNAPSHOT만 받으며 아직 없는 player를 생성하지 못한다.
두 레이드의 카메라는 local character가 없을 때 follow를 끈다.

서버의 초기 reliable batch는 ENTER_ACCEPTED 다음에 자신의 PLAYER_SPAWNED를
포함한다. Client snapshot 병합도 spawn 경계를 보존한다. 수정 대상은 서버
송신 순서가 아니라 이전 레벨의 새 입장 세대 소비다.

## G01. 변경 파일과 호출 흐름

- `Client/Public/ClientReplication.h`: replication 인스턴스가 소유한 world inbound
  generation을 private 상태로 보관한다. world ID가 같아도 재입장은 다른 세대다.
- `Client/Private/ClientReplication.cpp`: Initialize 성공 때 현재 세대를 캡처하고
  Reset에서 해제한다. Update는 기존 disconnect 정리를 유지하되, 살아 있는
  연결의 세대가 다르면 이벤트·typed 결과·캐릭터 준비를 소비하기 전에 반환한다.
- `Tools/Network/test_client_receive_dispatch.py` 또는 인접 기존 network 검사:
  실제 생산 함수 경로로 지연 전환의 stale consumer, 새 consumer, 같은 world
  재입장, disconnect 정리를 검증한다.
- `.md/GB/gotchas.md`: 레벨 전환 대기 중 inbound 세대 소유권 주의점을 기록한다.
- 대응 RESULT: 실제 검사·빌드·배포와 사용자 화면 확인 경계를 기록한다.

흐름은 서버 승인 → NetworkManager 세대 증가 → 기존 replication 소비 차단 →
비동기 준비 정리 → 새 레벨 Initialize의 세대 캡처 → 새 레벨의 초기 spawn 소비다.
새 C++ 파일·프로토콜·데이터 schema·project/filter 등록은 필요하지 않다.
사용자 렌더링 설정과 기존 F6 수동 전환 동작은 유지한다.

## G02. 검증과 Release ZIP

수정 전 소비 경로가 동일 회귀 조건에서 실패하는지 확인하고 수정 후 network
검사와 Client Debug/Release 컴파일·링크를 확인한다. 이전 Guide 데이터·Server
수정도 새 Release에 포함한다. 변경 줄·인코딩·git diff --check를 확인한다.

기존 portable 배포 도구로 Desktop의 새 ZIP을 생성한다. 기존 ZIP을 덮어쓰지
않으며 payload hash, ZIP CRC, launcher --check와 빌드 receipt를 검증한다.
실행 중인 EXE/DLL 링크 잠금은 사용자 종료 또는 명시적 종료 승인 후 처리한다.
Client/UI는 자동 실행하지 않고 실제 반복 입장 화면 확인은 사용자 검증으로
구분한다. Resources는 기존 배포 계약의 외부 Resources를 사용한다.

## G03. 장비 강화 창과 월드 이름표 겹침

사용자가 추가 요청한 강화 창 위 차원술사 이름표 겹침도 같은 Release에 포함한다.
현재 `CMainApp::Register_UITextOccluders`는 강화 창 전체 폭948 대신 중앙
`ItemUpgrade_PanelBg` 폭442만 이름표 가림 영역으로 등록한다. 실제 슬롯 rect와
Font clip 소비를 대조하여 강화 창 전체가 WORLD 이름표보다 앞에 표시되도록
기존 `CUITextOcclusion` 경로를 보완한다. 강화 창 밖 이름표와 강화 창 자체의
텍스트는 유지한다. 창 닫힘·결과 화면과 해상도 스케일을 확인한다.
