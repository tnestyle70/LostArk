# 마하라카 닉네임·동행·낙사 결과

## G00. 적용한 연결

Maharaka와 Colosseum의 CLevel_Development에 기존 이름표·파티 초대·채팅 말풍선을 연결하고,
MainApp의 제품 파티·채팅 화면 대상에 포함했다. server-replicated nickname/health/roster를 소비한다.
Maharaka의 섬 이동 확인창은 기존 RaidEntryPreviewView 이미지 UI를 재사용하며,
모달 입력 소유와 UI 위 gameplay 클릭 차단을 유지한다.

Bern의 island.dock.to.maharaka와 Maharaka의 island.exit.to.bern G는 Server가 위치·상태·파티장을
검사하여 전원 확인을 요청한다. target enum의 기존 발탄0/쿠크1은 유지하고 섬 입장2/복귀3을 추가했다.
전원 수락 후 solo도 같은 reliable batch admission을 사용하고 모든 초기 송신 준비 전에 출발하지 않는다.
인벤토리·재화가 비어 있어도 신규 입장 지급으로 대체하지 않으며 각각의 닉네임·내구도·배 복귀 기록을 보존한다.
원래 파티가 없는 solo는 별도1인 파티를 만들지 않아 섬 안에서 정상 초대를 받을 수 있다.

Waterpang fall만 시작 지면보다2m 낮은 평면에서 기존 점프대 생환을 확정한다.
발탄의45tick 사망과 쿠크의 원래 높이 판정은 바꾸지 않았다. 기본 점프·탄도 진행을 조기 낙사로 처리하지 않는다.

## G01. 실제 검증

Debug Server 제품 빌드가 성공한 실행 파일에서 listener 없이 실제 CGameRoom/CServerApp와
fake transport session을 사용했다. --maharaka-ai-contract-test exit0, 전체239개 assertion PASS.

- 실제 G가 즉시 출발 대신 확인창을 열고, 거절 시 전원이 기존 방에 남는다.
- 1·2·4인 모두 한 명의 outbound FIFO 포화에서 방/파티가 보존되고, 해소 후 전원 입장한다.
- 실제 왕복으로 닉네임·빈 인벤토리·빈 재화·session binding·개인 배/선착장 기록이 보존된다.
- 섬2m 낙사 생환과 발탄45tick·쿠크5m 판정의 경계 전후를 검사했다.
- 기존 Q3발/R1발/W비행 무피격·착탄1회, AI20명,3분 종료·snapshot·전원 이동·재입장 회귀도 함께 통과했다.

증거: out/MaharakaParty20261001/party-fall-debug.log, contract-debug.json.
변경 JSON/XML22개 parse 및 diff whitespace도 통과했다. Client/UI는 실행하지 않았으며
닉네임 배치·확인창·낙하 체감의 실제 화면 판정은 새 실행 파일로 사용자 확인이 필요하다.

## G02. 최종 제품 전달

2026-10-01 후속 정상 Server [Debug 빌드](../../../out/GuidePersonal20261001/server-guide-final-debug-build.log)와
[Release 빌드](../../../out/GuidePersonal20261001/server-guide-final-release-build.log)가 실제 제품 EXE
링크까지 성공했다(각 경고 0/오류 0). 이전 Server 실행 파일 점유에 따른 링크 보류는 해소됐다.

최신 Release `--maharaka-ai-contract-test`는 **199 PASS, 0 FAIL, exit 0**이며,
`--npc-raid-return-contract-test`도 **54 PASS, 0 FAIL, exit 0**이다.
[마하라카 로그](../../../out/GuidePersonal20261001/maharaka-ai-release-complete.log),
[NPC 입장·귀환 로그](../../../out/GuidePersonal20261001/npc-raid-return-release-complete.log),
[종료값 JSON](../../../out/GuidePersonal20261001/contracts-release-complete.json)을 증거로 보존한다.
실제 섬 이동 transaction·실패 보존·닉네임/인벤토리·2m 낙사 및 경기 종료/재입장이 포함된다.

최종 이동 수정·실패 알림 연결을 포함하는 Client 전체 Product 빌드는 아직 대기 중이다.
Server 링크·계약 PASS를 최신 Client 링크나 사용자 화면 확인 완료로 기록하지 않는다.

## G03. 최종 파티 입장 제품 빌드

이동 보완과 모든 최신 Client/Server 소스를 포함한 정상 Product Debug/Release가 모두 성공했다.
앞선 EXE 잠금·최종 링크/제품 빌드 대기는 해소됐으며 실행 파일과 실제 로그는
`../09-27/2026-09-27_GUIDE_AI_TOOL_IMPLEMENTATION_RESULT.md`의 G09에 기록했다.
이 결과는 기존 기능별 검증을 대체하거나 실제 Client 화면·다인 플레이·성능 확인으로 확대하지 않는다.
