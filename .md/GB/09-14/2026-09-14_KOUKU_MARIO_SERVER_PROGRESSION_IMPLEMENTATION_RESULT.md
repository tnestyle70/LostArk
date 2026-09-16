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


## G07. 2026-09-16 서버 랜덤 두 패턴 — 소스 반영, 설치 대기

이번 요청은 Sequencer Play에서 지정된 여섯 후보 중 서로 다른 두 패턴을 Server가 골라 실제 완료 순서로 재생하는 범위다. 별도 질문에 아직 답변이 없어 마지막 요청의 범위를 두 패턴 완료 후 종료로 해석해 후보를 준비했다. 다음 마리오 자동 진입은 이번 설치 후보에서 연결하지 않으며, 기존 Success를 다시 연결하는 계약과 솔로 복귀 대기는 코드에서 유지한다.

### 소스 반영

Workbench의 Sequencer Play는 활성 PATTERN_COMPLETION_COUNT Logic을 발견하면 기존 Request_SelectedServerPlay를 사용한다. Server 선택 ID와 현재 실행 시작 tick으로 패턴 선택·커서를 갱신한다. 진행 중 반복 Server Play는 기존 실행을 보존하며 거절하고, Stop 대기/거절에는 follow 상태를 유지한다. dirty 또는 활성 editor 입력이 있으면 이번 실행의 자동 선택을 중단하여 미적용 입력을 보존한다. 일반 Pattern preview 경로의 start/scrub도 이 추적 실행 중에는 거절한다. 별도 Resource/Row preview의 기존 동작을 전역 변경한 것은 아니다.

Server의 shuffle/완료 누적을 재사용하고 Success 없는 체인은 마지막 실제 PATTERN_COMPLETED에서 portal 및 같은 tick 대기 entry를 정리한 뒤 기존 정상 종료로 처리한다. 취소는 ABORTED다. Success 있는 체인의 복귀 및 후속 진행은 유지한다. Bundle member의 iStartTick은 최초 schedule이 아닌 현재 boss의 실제 시작 tick을 복제하도록 교정했다.

Client codec, Python projector, Gameplay publisher, Server Catalog 및 Brain은 Success 0개 또는 FOLLOWUP_PATTERN 1개를 같은 규칙으로 검증한다. P52 Parent Summon은 명시 lifetime을 기존 확장 Stage로 게시하므로 authored Stage0만으로 거절하지 않는다. untimed candidate, 다른 Gate/body, 중첩 chain 및 부적절한 outcome의 거절은 유지한다.

### 후보 데이터와 검증

1122 source 스냅샷의 out-only 후보는 logic55의 completionCount를2로, 후보를 P38/P39/P40/P43/P52/P46으로, P34.logic.1의 Success를 빈 배열로 바꿨다. 나머지 저작은 보존했다. 실제 prepare_publication와 projected_outputs에서 root+여섯 후보 모두 published=true/unavailableReason 없음, 전체 Product63개를 확인했다. P52는 기존 확장으로 0→1 Stage가 됐다.

| 후보 | source의 실제 전체 길이 |
| --- | --- |
| P38 무지개댄스 | 18485ms |
| P39 알비온 감전장판 | 15864ms |
| P40 십자화염폭발 | 7267ms |
| P43 백스텝불뿜기 화염링 | 12554ms (Stage 합6434ms와 구분) |
| P52 분신소환 기분나빠 Parent Summon | 13009ms |
| P46 백스텝 후 감전빔 | 3600ms |

P40은 현재 animation Stage2개만 있으며 presentation/summon/world/logic occurrence는 없다. 이번 랜덤 재생 연결을 십자 화염 시각 복원 완료로 기록하지 않는다.

- Workbench, CompositionDocument와 Server GameRoom_KoukuAudition/KoukuSaydonBrain/GameplayCatalog/ServerGameplayContractTests_KoukuProduct의 격리 Debug 컴파일 성공. Product 링크는 미실행이다.
- 기존 Python 집중 테스트2개와 관련 parent/clone 회귀2개 성공. 두 후보/Parent/optional Success, 실패 outcome·untimed·nested chain 거절을 검사했다.
- 새 Server 시나리오는 두 패턴 종료·취소·P52·기존 후속 진행·현재 시작 tick을 추가하고 컴파일했다. 실제 Server 시나리오 실행은 미실행이다. 현재 Run_KoukuProduct는 --contract-test 경로에 포함되고 전용 좁은 switch는 없다.
- source/후보 JSON parse, PowerShell publisher AST parse, git diff --check 성공. 새 C++ 파일이나 project/filter 등록은 없다.

근거는 out/MarioRandomTwo20260916/compile-results.json 및 Workbench compile log, out/KoukuMarioRandomTwo20260916/compile/{server,client}.log, candidate-probe/receipt.json이다. candidate-probe/source.exact.candidate.json과 Install-ExactMarioRandomTwo.ps1은 네 줄만 바꾼1122→1123 후보와 설치 준비물이며 실행하지 않았다. 설치기는 실행 중 Client와 baseline byte 변경을 거절하고 기존 writer lock/CAS를 사용한다.

### 아직 적용하지 않은 경계와 재개 순서

최종 확인에서 실제 source는 다른 저장으로1123이 됐고 Client PID58036, Server PID57572가 계속 실행 중이다. 이번 source 후보를 설치하거나 runtime publish, Product build, Server 재시작, Client 화면 조작을 하지 않았다. 열린 draft 보존을 위해 사용자 Save 및 두 프로그램 종료를 요청했으며 답변 대기다. 기존1122 기반 설치 후보는 최신 저장본과 맞지 않으므로 그대로 실행하면 CAS 거절되어야 한다.

저장·종료 확인 후 최신 source bytes에서 위 세 필드와 revision만 다시 후보로 만들고 검증/설치한다. 이어 `Tools/Build/Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon -ExpectedKoukuSaydonSourceRevision <설치 revision>`으로 정식 게시하고 `Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug` Product Build를 실행한다. 현재 실행 경로는 Client/Bin/Debug/Client.exe, Server/Bin/Debug/Server.exe이며 두 EXE 모두 이번 변경을 아직 링크하지 않았다.

사용자 확인 경로는 Lobby → KoukuSaydon → F1에서 3관문 보스 활성화 → Composition/Sequencer의 세이튼_마리오_1페이즈(P34) 선택 → Play다. 게시와 새 Product가 준비된 뒤 root의4650ms에서 랜덤 두 패턴이 시작하고, 각 실제 완료마다 다음 패턴으로 넘어간다. Server CMD의 [MarioPatternChain] count=2와 completed=1/2,2/2를 확인한다. 화면·효과·패턴 선택 전환은 사용자 확인 전이며 visual PASS로 기록하지 않는다.


## G08. 마리오 접촉 즉시 이동·입장 — 소스 검증, 설치 대기

Server30Hz에 별도 느린 collider polling은 없었다. P34 포탈 표시는2998ms지만 판정은4635ms부터 시작했고, 보이는 MAP 위치와 BOSS_CURRENT 판정 기준이 달랐다. 중앙 boss body radius1m+player0.45m에 막힌 상태에서는 halfExtent1m 박스의 가장 먼 꼭짓점1.414m에도 못 닿는 조건이 있었다. 일반 동작 중 NONE 제한과 실패한 진입의 inside 캐시도 재시도를 지연했다. Client 위치 보간2tick과 damage event 즉시 반영 차이는 체감 차이를 추가하며 이번 수정으로 네트워크 지연 자체가0이 되는 것은 아니다.

Mario entry는 Shared BODY_CIRCLE_XZ와 기존 collision margin1mm로 접촉을 평가한다. 정상 전투 동작은 취소하고 입장할 수 있으며 dead/grabbed/bound/arena ejection·이미 Mario·이동 중 상태는 거절한다. 목적지 검증과 candidate 상태 구성이 성공한 뒤에만 기존 action/object를 정리하고 commit한다. Mario1~4 실제 lane exit/arrival의 자동 move는 같은 정책으로 일반 동작을 취소하며, 실패 시 inside 캐시를 지워 다음 tick에 재시도한다. 같은 이동의 반복 초기화와 무관한 trigger 변경은 막았다.

P34 collider/Logic을 눈에 보이는 portal의 MAP TRS·2998ms 시작·21160ms 창에 맞추는 후보를 준비했다. 고정 MAP Collider를 Client codec/runtime 및 Python projector의 WORLD region까지 연결했다. 전체52 Mario placement 조사에서 활성 move18개 중15개의 once를 해제하고 두 interact(화살표) 트리거도 자동 접촉으로 바꾸는 후보를 준비했다.18개 모두 repeatable이 된다. 전체 world8795 원본과 installed의 관련52행이 같았고 진입 Y 범위에 의한 지연은 발견하지 않았다.

### 실행한 검증

- 현행 VS18/v14314.44/SDK26100으로 Shared+Server를 out 전용 격리 빌드했다. 기존 --debug-teleport-contract-test는6789 PASS/실패0이며 새 entry contact·6동작×4stage·실패 보존·Mario1/Mario4 callback 사례를 포함한다. WorldTriggers에 추가한 generic 회귀는 컴파일했고 그 별도 suite는 실행하지 않았다.
- Workbench/CompositionDocument/PresentationPlayer3개 Client TU의 격리 Debug 컴파일 성공. Product BIN/기본 intermediate 변경0을 확인했다. 새 C++ 파일 등록은 없다.
- actual publisher의 out-only World Validate는112placements/5groups PASS. Combined1140→1141/World8795→8796 후보의 actual prepare_publication/projected_outputs는 Product63개와 P34/랜덤6개 모두 available이다.
- MAP Collider 집중 Python 검증 성공. 기존 linked-collider 전체 live fixture는 별개 P32.presentation1의 end18801>lifetime0 오류로 실패했고, frozen P1 기반 동일 두 subcase는 성공했다. 이를 전체 suite PASS로 기록하지 않는다.

증거는 out/MarioContact20260916/{focused-test-receipt,isolated-build-receipt,compiled-sources}.json, debug-teleport-contract.log, out/MarioContactClient20260916/compile-results.json, out/MarioCollisionAudit20260916/, out/MarioColliderAudit20260916/entry/, out/KoukuMarioRandomTwo20260916/combined-candidate/다. 예상 missing-anchor 로그는 실패 시 기존 상태 보존 fixture다.

### 남은 적용 경계

사용자는 저장·종료 확인에 “아직 편집 중이야”라고 답했다. 따라서 후보 설치·runtime publish·Product 링크·프로세스 종료를 하지 않았다. 사용자가 이후 저장한 Composition을 기준으로 후보를 다시 생성해야 하며1140 baseline 설치기를 그대로 실행하면 안 된다. 현재 실행 Client/Server는 이번 소스 변경을 사용하지 않는다. 사용자 화면·실제 입력 확인은 미실행이다.
