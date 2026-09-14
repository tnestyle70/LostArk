# 마리오 Server 진행 구현 결과

## G00. 적용 계약

Composition550에 typed PATTERN_COMPLETION_COUNT duration, ENTER_AREA→MARIO_ENTER, Success followup을연결했다. Server가회차1..4와진입commit을소유한다. 실제패턴COMPLETED만완료수를올리며취소·실패·Timeout은성공으로소비하지않는다. root entry collider/clock/anchor는child패턴중유지하고protocol83 Bundle memberstate를Client retainedpresentation이소비한다. latejoin도같은시계를사용한다.

P34후보는실제stage가있는P38무지개댄스/P39알비온/P43백스텝화염이며3개완료후Success→P33마리오2페이즈,그stagger성공→P42로연결했다. P40/P45십자화염폭발은stage0이라아직후보가아니다. 기존효과·커튼저작은보존했다. Gate3 SavedFlow는Mario×2→Showtime→Mario×2다.

F1 단독Test는`Mario test stage (0 = live)`와`Mario test seed`를설정하고Complete Play(Server)를사용한다. stage0은현재server상태,1..4는한번의명시시작값이다. 같은seed로후보순서를재현하며SavedFlow는강제Test값을사용하지않는다. Client로컬카운트/트리거우회는없다.

## G01. 검증과 배포

Shared codec와Server/Client변경TUs의Debug CL,Server정규Build PASS. source550 projector와GameplayBalance publish PASS(43productpatterns/319stages). 최초전체Server contract실행은Mario4의구형published navgrid때문에1failure였다. authoring은ground-12.8인데installed cell이walk0/height18.588이었으며Navigation publisher로동기화했다. 이거절조건을완화하지않았다.

최종새navigation/최신Server로재실행한결과는통합RESULT G06에기록한다. 실제3pattern완료chain·seed재현·typed4회차진입·retainedstate·legacy/invalid/truncatedcodec과Bernnavigation검사를현재Server suite가포함한다. Client화면조작은사용자가수행하며Server/Client83을함께재시작해야한다.

## G02. 남은 범위

십자화염폭발stage작성,사용자의CompletePlay전과정화면판정과테스트stage별진입확인은미완료다. 결과/근거는out/KoukuMario20260914/와통합out/CharacterEffectArena20260914/final-server-contract.log를사용한다.
