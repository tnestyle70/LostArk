# 발탄 집결 입장과 첫 폭탄 갑옷 파괴 구현 계획

## G00. 현재 실측과 목표

2026-09-30의 기존 저장본을 기준으로 두 갑옷은 각각 mask 1/2이며 현재 파괴 창은
DESTROY_FIRST_ELIGIBLE로 폭탄마다 하나씩 제거한다. 사용자가 요청한 첫 성공 폭탄의
전체 갑옷 제거는 실제 Server part 상태, legacy plate 상태, reliable PART_BROKEN 및
Client 파편을 함께 변경한다. 일반 스킬과 닫힌 파괴 창의 갑옷 피해 차단은 보존한다.

Stage_Boss는 (129.65,23.02,-96.85)의 구 이동 marker와 직접 encounter 시작을 소유한다.
이를 비활성화하고 Stage_Boss_Assembly를 (123.38,23.05,-89.20)에 배치한다. 쿠크 원본
entry_aura/active의 동일 footprint를 사용한다. 현재 월드의 인간 참가자가 모두 집결한
Server 시간 10초 뒤 확인창·투표 없이 기존 encounter 입장 cinematic 경로를 시작한다. 사용자의 두 좌표와 같은 간격의 왼쪽 두 좌표는 아레나
도착 위치로 사용하며 집결 지점에 도달하기 전 최초 맵 입장 위치는 보존한다.

## G01. Server 상태와 데이터

ServerCombatHitRuntime에서 provenance가 검증된 primary 발탄 폭탄과 실제 part break에만
남은 갑옷을 같은 tick에 제거한다. 기존 PendingPartBreakEdges를 한 combined mask로
확장하고 상태 revision 및 legacy plate와 맞춘다. Client는 기존 mask별 파편 두 개를
동일 event에서 한 번씩 소비한다.

GameRoom_GateProgress는 두 아레나의 집결 시작 tick/인간 roster와 쿠크 raid epoch를 Server에서
소유한다. 발탄은 전원 집결, 쿠크3관문은 기존 leader aura 점유 조건을 유지한다. 이동·죽음·
인원/epoch 변경은 타이머를 초기화한다.300tick 뒤 기존 gate transition 완료 함수를 재사용하여
모든 도착 좌표의 navigation/collision과 현재 참가자를 검증한 뒤 commit하며 실패 시 위치를
보존한다. 초기 발탄 ADVANCE와 쿠크 ENTER_GATE3 수동 요청도300tick을 우회하지 못한다.
Bern 파티 이동, 관문 ADVANCE/RESTART/EXIT 등 무관한 동의 절차는 유지한다.

World Gameplay의 disabled slot 배치가 네 도착 좌표를 소유한다. 기존 Area publisher로
게시하며 generated bootstrap을 직접 수정하지 않는다. 신규 C++ 파일은 없으므로 project와
filter 등록 변경은 없다.

## G02. Client 입력과 표현

Level_ValtanArena는 기존 replicated player view와 Server tick으로 카운트다운을 표시한다.
집결 collider와 effect anchor는 같은 Gameplay.world.json 배치를 읽는다.10초 뒤 Client가
제안·수락을 전송하지 않으며 Server 자동 완료의 복제 상태와 기존 cinematic만 소비한다.
UI는 직접 socket·damage·player transform을 변경하지 않는다. 이전 Stage_Boss marker는
비활성 데이터로 제거하며 active aura 교체 실패는 이전 handle을 보존한다.

## G03. 검증

실제 room의 첫 폭탄 combined mask/중복 억제와 집결 시간·이탈·자동 완료·이동 admission을
focused contract로 검증한다. 변경 JSON parse, World publish/CheckPublished, git diff --check와
필요한 Server/Client 컴파일은 통합 담당과 한 번 수행한다. Client/UI를 실행하지 않으며 실제
카운트다운·자동 입장·cutscene·갑옷 파편 화면은 사용자 확인으로 남긴다.

## G04. 사용자 후속 요청: 두 아레나의 자동 집결 완료

이전10초 뒤 레이드 입장 확인창과 수락 단계를 없앤다. 이 변경은 실행 중인 Client/Server를
종료하거나 게시·빌드하지 않고 최신 저장본의 C++ 후보만 준비한다. 첫 폭탄의 갑옷 파괴는
사용자 화면 확인이 끝났으므로 재작업하지 않는다.

Server의 기존 Close_GateProgressVote가 소유한 완료 분기를 별도 공유 완료 함수로 추출한다.
자동 집결 완료와 남아 있는 일반 투표가 이 함수를 함께 호출한다. 자동 집결은 열린 vote를
만들거나 가짜 응답을 생성하지 않는다. 기존 S2C_GATE_PROGRESS_STATE의 closed result로
완료/입장 실패를 전달하며 proposal ID는0이다. 실패 때 기존 위치·보스 상태를 보존한다.

CRaidGateProgressView::Render_AssemblyCountdown(secondsLeft) static 함수가 쿠크의 기존
Font_YoonGasiIIM 표시를 공통화한다. 흰색 ‘잠시 후 다음 지점으로 이동됩니다’는 y72%, size0.625,
노란 ‘N초’는 y75.5%, size0.7이다. ValtanLevel의 주황 Show_Notice·입장 confirm/vote/request를
제거하고 같은 countdown HUD만 표시한다. KoukuLevel도 같은 helper를 사용하고 자동 proposal
전송을 제거한다. 사용자 HUD8fields JSON은 건드리지 않으며 신규파일/project 등록은 없다.

기존 BattleItems의 실제4인 집결 test는300tick 이전 거절·이탈 reset·기한 뒤 무요청 자동입장과
4목적지·입장 cinematic을 확인하도록 바꾼다. 연속 lifecycle fixture도 제안/응답 없이 집결로
시작한다. 쿠크 기존 Gate3 entry test는 실제 Server timer와 공유완료를 통해 roster변경·이탈·
사망 및 commit실패 보존,10초 뒤 자동진입을 검사한다. 실행은 사용자 실행파일 종료와 통합
담당의 최종 빌드/재실행 뒤에만 완료로 기록한다.

## G07. 진입 컷신의 플레이어 표시 억제

현재 발탄 진입은 HUD·이름표·원본 보스를 숨기지만 복제된 플레이어의 Character
presentation suppression을 설정하지 않아 컷신에 본체와 장비가 남는다.

Level_ValtanArena.h/.cpp에서 기존 CCharacter::Set_CinematicPresentationSuppressed를
현재 replicated player view에 적용한다. 입장 source cinematic과 그 camera return
구간에만 적용하고 정상 종료·시작 실패·취소·레벨 이탈에서는 해제한다. 같은 숨김 API가
소유하는 장비·탈것·그림자도 함께 억제한다. 전투 중 camera와 다른 보스 정책은 유지한다.
MainApp의 기존 후반 cinematic UI 동기화 지점에서 발탄도 같은 표시 상태를 재확인하여
snapshot에서 새로 만들어진 player가 한 프레임 표시되지 않도록 한다.

현재 dirty 변경과 C++ 파일별 인코딩을 보존하고 추가한 부분만 검토한다. 새 public
packet/데이터/렌더링 옵션/파일/project 등록은 없다. 실제 소비자와 종료 경계를 검사하고
Debug/Release 최소 컴파일 후 가능한 제품 링크까지 진행한다. GPU 화면에서 local/remote
플레이어의 숨김과 복귀는 사용자가 확인하며 CPU 검증을 화면 PASS로 기록하지 않는다.