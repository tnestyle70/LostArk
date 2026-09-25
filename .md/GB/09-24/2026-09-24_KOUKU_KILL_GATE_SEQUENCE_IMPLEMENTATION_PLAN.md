# Kill Boss 뒤 관문 연출 누락 수정 계획

## G00. 실측된 두 경로

Kill Boss는 현재 관문의 boss HP만 0으로 만들고 정상 사망 처리를 사용한다.
Complete Play raid가 살아 있으면 `Advance_KoukuRaidGate`가 다음 관문의 intro를 재생한다.
그러나 단독 보스 소환/패턴에서 처치한 뒤에는 raid owner가 없어 `Advance_Gate`의
legacy spawn/teleport 경로로 진입한다. 이 경로는 4관문만 준비·intro에 연결돼 있으며
1~3관문은 시퀀스를 건너뛴다. Source In의 stale Debug OBJ 문제와 구분한다.

## G01. GateProgress와 RaidFlow

동의가 끝난 ADVANCE/RESTART는 관문 번호와 무관하게 기존
`Begin_KoukuRaidPreparation -> READY -> Begin_KoukuRaidCinematic`을 사용한다.
기존 Bingo 전용 vote flag를 일반 gate vote flag로 확장하고 요청 관문·제안자·전원 동의를
검증한다. 준비 실패 전에는 위치·boss·player 상태를 변경하지 않는다.
준비 성공 뒤 다음 intro의 authored arrival이 이동을 소유한다.

G3 입구 deck에서 전투 구역으로 들어가는 별도 ENTER_GATE3 동의는 이미 intro가 끝난
뒤의 입장 계약이므로 기존 participant 제한과 destination 검증을 유지한다.
G3 죽음 뒤 자동 false clear/Encore 및 Bingo ending은 기존 raid owner를 유지한다.

## G02. 검증

기존 Kouku raid native 계약에 단독 G1/G2 처치 후 진행과 G1/G2/G3 재시작의 준비·intro를
추가한다. 부분 동의/READY 실패에서 위치·생성 상태를 보존하며 완료된 준비가 CINEMATIC을
방송하는지 확인한다. Debug/Release Server·Client 최소 컴파일과 diff 검사를 실행한다.
프로토콜·새 C++ 파일·프로젝트 등록 변경은 없다. 실제 연출 화면은 사용자가 확인한다.
