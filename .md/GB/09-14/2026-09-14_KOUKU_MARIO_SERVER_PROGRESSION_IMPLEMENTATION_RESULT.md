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

## G03. 추가 요청: 솔로 복귀와 0키

솔로 입장 회차는 Server member가 참가자의 player/session/entity를 고정하고 실제 3관문 착지를 확인한다. 세 랜덤 패턴 완료 후 입장자가 남아 있으면 대기하고, 미입장이거나 이미 복귀했다면 다음 fixed tick에 기존 P33 Success를 시작한다. 복귀 완료가 먼저여도 세 패턴을 모두 기다린다. 입장자의 사망·퇴장은 이유가 있는 ABORTED로 처리한다. 실제 Leave 경로도 [MarioSoloAbort] 로그에 원인을 남기며, 마지막 퇴장은 기존 방 초기화와 receipt 정리를 유지한다. 2인 이상은 기존 세 패턴 완료 진행을 유지한다.

숫자열 0은 기존 바인딩이 없었으며 Mario controls의 새 눌림을 `Request_MarioReturn`으로 보낸다. Server는 현재 stage의 마지막 authored exit, 지면과 충돌을 검증하고 `Begin_MovePlayer`로 복귀시킨다. 요청에는 좌표가 없으며 실패 시 위치·진행·형태를 유지한다. terminal 이동이 Intro 안에서 시작해도 자동 재입장하지 않는다. 동일 sequence의 재전송은 이동을 재시작하지 않는다. 네 마지막 트리거 목적지는 `stage.kakul.sl05`로 교정했고 World revision8794를 publisher로 배포했다.

현재 P33에는 아이언메이든 감금 판정이 없다. P37의 WORLD 아이언메이든 연출과 다른 패턴의 잡기 판정은 변경하지 않았다. 현재 네 단계 모두 같은 복귀 경로를 쓰며 네 번째 입장 후 카운트5는 추가 입장을 열지 않는다. 다음 실험 회차는 기존 reset/test stage를 사용한다.

### 검증 상태

Shared protocol84 마리오 이동·점프·복귀 하네스 217개 PASS. 하네스의 구형 Bundle aggregate 초기화를 현재 멤버 계약에 맞게 수정한 뒤 컴파일 및 실행했다. source JSON/XML parse와 `git diff --check` PASS. Server 계약 테스트에는 7개 진행 시나리오와 네 stage의 실제 Intro→복귀 motion, 중복·상태·충돌·navigation 실패 검증을 추가했다.

최종 Product 빌드와 새 Server 계약 테스트 실행은 현재 실행 중인 Client/Server의 저장·종료 후 수행한다. 실행 중인 두 프로그램은 사용자가 시작한 것이며 에이전트는 Client를 실행·조작하거나 화면 판정을 하지 않았다. 완료 증거는 이 항목을 실제 빌드 결과로 갱신한다.

## G06. 숫자열 0의 편집창 포커스 차단 수정

기존 바인딩은 DIK_0, 즉 위쪽 숫자열 0이었다. 숫자패드 0으로 연결된 문제가 아니다.
MainApp의 WantCaptureKeyboard가 일반 F1 편집창 포커스만으로 true이면 Mario의 공통
keyboard block이 복귀 새 눌림도 차단했다. 실제 사용자 입력 상태를 캡처하지 않았으므로
이는 코드와 전후 집중 검사로 재현한 차단 경로이며 사용자 화면의 단일 원인을 확정한 것은 아니다.

PlayerController의 복귀 전용 조건만 일반 편집창 포커스를 통과한다. foreground·Mario active·
gameplay camera는 필요하고, 텍스트/활성 widget/속박/자유 및 연출 카메라/ground targeting은
차단한다. 일반 이동·점프·스킬의 keyboard gate는 보존했다. 동작 중·Mario 밖·연결 없음·편집
차단은 기존 Mario return status에 이유를 남긴다. Server typed 요청과 action/navigation/
collision/중복 sequence 검증, 기존 Begin_MovePlayer의 권위는 그대로다.

실제 Update_MarioControls/Update_MarioReturn과 관련 현재 함수 본문을 추출하고 외부 입력·
ImGui·command sink만 대체한 전후 프로브가 각각 34검사, 합계 68검사/실패 0이다. 같은 일반
F1 focus의 0 새 눌림은 전송 0→1로 바뀌었다. 실제 text·active widget·background·free camera·
bound/action/dead·pending·0 홀드·차단 중 눌림/해제 재입력·응답 상관관계·timeout 보호도 통과했다.
수정 PlayerController TU Debug 격리 컴파일 PASS이며 기존 문자 집합 경고는 남는다.

이미 실행 중인 제품과 같은 Server.exe에 `--debug-teleport-contract-test`를 별도 headless
프로세스로 실행한 결과 실패 0이다. 네 stage의 실제 Intro→return motion→3관문 착지와
잘못된 state/world·중복·목적지 충돌·navigation 보존을 포함한다. Server 소스 수정은 없으며
새 Client 입력 검사는 위 프로브와 구분한다. 증거는 `out/DollSustain15_20260914/`의
`mario-before-probe.result.log`, `mario-after-probe.result.log`, `mario-probe-receipt.json`,
`mario-PlayerController.compile.log`, `mario-existing-server-test.log`다.

최종 새 Client Product 링크와 실제 사용자 0키 입력은 아직 미완료다. Client PID45256과
Server PID63540은 사용자 실행 프로세스이며 저장·종료를 요청했다. 사용자 종료 후 빌드하며
에이전트는 Client 실행·조작·캡처를 하지 않는다. 새 빌드 후 Mario에 진입해 동작이 끝난 상태에서
위쪽 숫자열 0을 새로 눌러 3관문 복귀를 확인한다. 실패 이유는 기존 Mario return 상태줄을 따른다.

### G06 최종 제품 빌드

Client/Server가 종료된 뒤 정규 Debug Product Build와 배포가 통과했다. 근거는
`out/BuildPipeline/runs/20260914T040700948Z-debug-product.json`이다. 이 항목이 앞의 Product
링크 대기 상태를 갱신한다. 실제 사용자 0키 입력과 3관문 착지 화면은 아직 확인하지 않았다.
