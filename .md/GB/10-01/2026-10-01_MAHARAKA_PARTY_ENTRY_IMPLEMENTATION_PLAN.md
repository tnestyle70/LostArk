# 마하라카 닉네임·파티 동행·낙사 높이

## G00. 현재 경로와 요청

사용자는 워터팡 아일랜드에서 인간 플레이어의 닉네임과 파티 초대, 파티를 유지하는 단체 입장 확인 UI, 더 이른 낙사 처리를 요청했다. 기존 Bern·Valtan의 이름표와 PartyInteraction을 같은 replication view로 연결한다. 제품 UI에 ImGui를 추가하지 않는다.

Maharaka는 CLevel_Development가 소유하며 현재 이름표·파티 UI 호출이 없다. 기존 CPartyInteractionView와 CWorldPlayerNameplateView를 재사용하고 CMainApp의 파티·채팅 표시 대상에 Maharaka와 Colosseum을 포함한다. 기존 도구·자유 카메라·컷신·모달 입력 소유를 존중한다.

## G01. 섬 왕복 입장 투표

현재 Bern의 island.dock.to.maharaka G 트리거는 서버 검증 뒤 즉시 solo transfer를 만든다. 이를 기존 RAID_ENTRY 제안·전원 수락 경로로 연결하고 target enum 끝에 Maharaka 입장과 바다 복귀를 추가한다. 기존 발탄·쿠크 enum 값과 동작을 보존한다. 레이드와 같은 확인창에 섬 입장 문구를 표시한다.

서버가 트리거의 실제 위치·world·살아 있는 상태와 파티장을 검증한다. 거절·timeout·roster 변경·송신 실패는 전원을 기존 방에 보존한다. 수락 후 기존 Transfer_PartyTo의 stage→송신 준비→commit 경로를 사용해 닉네임·장비·인벤토리·재화·파티를 옮긴다. 각 세션의 왕복 배 기록도 유지한다. 기존 solo 사용자는 같은 투표에서 한 명만 수락한다.

## G02. 워터팡 낙사

현재 워터팡 낙사는 공용45tick(1.5초) 대기 뒤 복귀하여 몸이 지나치게 아래로 내려간다. Maharaka의 실제 Waterpang fall만 시작 지면보다2m 아래의 높이 기준으로 조기에 해결한다. Waterpang의 살아서 점프대 복귀 계약은 유지한다. 발탄의 기존 시간 기준과 쿠크의 기존 높이 기준, 점프 중 정상 이동은 변경하지 않는다.

## G03. 검증과 전달

실제 room 계약으로 초대·투표·전원 이동·실패 시 보존·배 기록·낙사 높이 경계를 검사한다. 필요한 Client/Server/Shared 컴파일과 최종 Debug/Release Product Build를 수행한다. JSON/XML 및 diff whitespace를 검사하며 새 C++ 파일을 추가하는 경우 project/filter에 함께 등록한다. Client/UI는 실행하지 않고 새 실행 파일로 사용자가 화면을 확인한다. 기존 이동 보정·성능·물총 복원 및 Colosseum PvP 변경과 파일별 변경을 보존하며 PR·병합까지 완료한다.
