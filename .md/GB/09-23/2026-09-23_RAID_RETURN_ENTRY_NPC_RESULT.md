# 레이드 종료 후 입장 NPC 앞으로 복귀

## G01. 원인과 연결

Handle_ReturnToBern이 발탄과 쿠크 모두 npc.bern.beda.guide로 고정되어 있었다.
입장 확인/투표에는 실제 NPC ID가 있었지만 world transfer와 대상 SERVER_PLAYER에
보존하지 않아 아일라라로 입장해도 베다 앞으로 돌아왔다.

SERVER_WORLD_TRANSFER_REQUEST → ROOM_COMMAND → Join/Stage_PlayerEntry → SERVER_PLAYER에
서버 검증된 strRaidReturnNpcPlacementId를 전달한다. 파티는 Transfer_PartyTo의 각 staged
player에도 동일한 입장 NPC ID를 전달한다. Handle_ReturnToBern은 이 ID를 기존 spawn
override로 사용하며 Bern admission이 현재 placement 앞 2.5m를 네비게이션에 투영한다.
복귀 투영에는 NPC의 높이도 전달한다. 알려지지 않은 NPC/잘못된 대상 world는 거부한다.
복귀 후 visit 정보는 비워진다. 로비 직접 입장처럼 원래 NPC가 없는 경우만 기존 기본
베다 출구를 유지한다. 클리어 조건·보상 inventory 전달·파티 원자 이동은 유지했다.

Server 내부 상태만 추가했다. Shared packet/schema, Client, 저작 JSON, Resources 변경 없음.
기존 프로젝트 파일만 수정했으므로 vcxproj/filters 등록과 publisher 실행은 필요 없다.

## G02. 실행 검증

- Debug Product 빌드 PASS: out/BuildPipeline/runs/20260923T004646321Z-debug-product.json.
- Server.exe --npc-raid-return-contract-test: 18개 검사 PASS, failures0.
- 실제 Bern/발탄/쿠크 catalog와 navigation을 읽고 두 NPC × 두 레이드의 admission,
  완료 복귀 라우팅, 높이/근접성, visit 상태 해제, 잘못된 NPC 거절을 확인했다.
- 현재 게시 nav 기준 베다 앞 (142.25,42.5027,-69.75), 아일라라 앞
  (144.75,50.1319,-145.75). 이 숫자를 하드코딩하지 않으며 배치와 네비를 따라 계산한다.
- git diff --check PASS. 기존 사용자 데이터 수정은 보존했다.

## G03. 사용자 확인

새 Server로 다시 실행한 뒤 베른 NPC에서 새 레이드를 입장하고 종료 버튼으로 돌아온다.
이전 실행 중 레이드에는 입장 NPC 정보가 없으므로 새로 입장해야 한다.
실제 플레이/파티 화면은 사용자 확인 대상이며 자동 검사를 화면 PASS로 기록하지 않는다.
상주 Server/Client를 자동 실행하지 않았고 추가 commit/push도 하지 않았다.
