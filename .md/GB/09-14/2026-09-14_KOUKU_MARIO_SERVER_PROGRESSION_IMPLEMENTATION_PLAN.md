# 쿠크 마리오 서버 진행 구현 계획

## G00. 현재 호출과 정본

Composition P34의 logic55는 judgementKind가 없는 DURATION이고 logic59는 결과가 없는 ENTER_AREA다. logic60도 outcomeKind 없는 RESULT다. 현재 fixed stage 20,227 ms 종료와 timeout wiring은 실제 랜덤 패턴이나 마리오 진입을 실행하지 않는다. `KoukuSaydonCompositionDocument → project_kouku_saydon_composition.py → Publish-GameplayBalance.ps1 → CGameplayCatalog → CGameRoom` 경로를 확장한다.

## G01. 완료 기준 랜덤 패턴

DURATION `PATTERN_COMPLETION_COUNT`는 후보 stable patternIds와 completionCount를 소유한다. 해당 box의 startMs에서 Server가 후보를 한 번 섞어 실제 패턴들을 같은 audition member에 연결한다. 각 pattern occurrence의 COMPLETED만 완료 수를 올리고 지정 수를 모두 마쳐야 Success FOLLOWUP_PATTERN을 연결한다. timeout은 정상 다음 단계가 아니다. 중단·실패는 성공으로 바꾸지 않는다. 원래 entry collider는 이 체인이 소유하며 체인이 끝날 때 닫힌다.

## G02. 마리오 진입과 카운트

ENTER_AREA Success의 typed MARIO_ENTER 결과는 Room에게 player ID를 제출한다. Room은 현재 열린 마리오 회차, 살아 있는 상태, navigation/collision, stage layout을 검증하고 기존 Mario Intro/go 경로로 진입시킨다. 같은 회차의 중복 진입은 카운트를 올리지 않는다. Room이 다음 stage 1..4를 소유하고 시작 위치 reset 때 초기화한다. Test는 기존 audition typed request의 명시적 시작 stage와 seed로 동일 경로를 사용한다.

## G03. 연결·검증

P34의 랜덤 패턴 후보는 P38 무지개댄스, P39 알비온, P43 뒤로뛰기 화염숨결/화염고리(현재 P40 십자화염폭발은 stage0으로 실행 불가)이다. Success는 P33 마리오 2페이즈로 연결하며 현재 커튼·애니메이션·이펙트 저작은 보존한다. F1 Boss Tool에서 test stage/seed를 고른 뒤 Complete Play(Server)를 누른다. 기존 C++ 파일만 수정하므로 project/filter 신규 등록은 없다. 실제 publisher와 Server contract test, Shared codec, 관련 Client 컴파일을 실행하며 Client/UI 실행이나 화면 판정은 사용자가 수행한다.
## G04. 동적 체인의 화면 소유권

기존 S2C_KOUKUSAYDON_BUNDLE_STATE member에 entry rootpattern, starttick, holdMs, stage, anchor를 복제한다. Client는 동일 Sample/Stop_Session 구현을 별도 root session으로 소비해 chain marker의 실제 entry 프레임을 유지한다. 실제 child animation은 Server snapshot대로 진행하고, portal 진입 소비/chain완료/중단/새generation 때 root session을 종료한다. Shared request와 state 형식 변경은 protocol83으로 구형 Client/Server 혼용을 거부한다.

## G05. 솔로 복귀와 0키 (추가 요청)

1인 진입 회차는 성공한 마리오 참가자 ID와 복귀 완료 상태를 Server member가 보관한다. 랜덤 패턴 세 개 종료 시 미진입이면 기존 Success를 실행하고, 진입했으면 실제 복귀 motion 완료까지 Success를 보류한다. 복귀를 먼저 마쳤다면 세 패턴 종료까지 기다린다. 사망·퇴장은 완료로 간주하지 않으며 중단 이유를 보존한다. 기존 P33에는 감금 판정이 없고 P37의 아이언메이든은 WORLD 연출이므로 다른 잡기 로직을 전역으로 변경하지 않는다.

Mario1~4의 마지막 movePlayer 트리거 목적지를 옛 SL04에서 기존 stage.kakul.sl05의 3관문 지면으로 교정하고 World publisher로 배포한다. 숫자열 0키는 현재 비어 있으며 마리오 조작 중 새 눌림만 typed MARIO_RETURN 명령으로 보낸다. Server는 참가 상태와 해당 stage의 마지막 트리거, 3관문 navigation/collision을 검증한 뒤 기존 Begin_MovePlayer 경로를 시작한다. 활성 UI 위젯·free camera·텍스트 입력 중에는 전송하지 않는다. 일반 편집창 포커스만 남은 경우는 G06의 복귀 전용 조건을 따른다. 실패 시 기존 위치와 동작을 유지하며 결과를 Client에 전달한다. 새 요청/응답은 protocol84로 구버전 혼용을 거부한다.

기존 파일 확장이므로 프로젝트 등록은 추가하지 않는다. Shared codec, Server의 복귀 전후·미진입·중복·잘못된 상태 검증을 추가하고 Product Debug 빌드 및 Server 계약 검증을 실행한다. Client 화면은 사용자가 직접 확인한다.

## G06. 숫자열 0의 편집창 포커스 차단

사용자는 위쪽 숫자열 0을 사용한다고 확인했다. 기존 DIK_0 바인딩과 실행 EXE의 복귀 코드,
네 stage의 실제 Server return 검사는 정상이다. MainApp의 ImGui WantCaptureKeyboard가
Object/Action 편집창 포커스를 유지하면 Update_MarioControls의 공통 keyboard block이 0도
소비하고 전송하지 않는 경로가 있다. Mario camera follow는 FollowEnabled를 끄지 않는다.

복귀 전용 새 눌림은 일반 편집창 포커스와 실제 텍스트/활성 widget 입력을 구분한다.
기존 이동·점프·스킬의 inputAllowed는 바꾸지 않고, 복귀만 foreground·gameplay camera·
active Mario·비텍스트·비활성 widget·비포획 조건에서 기존 typed Request_MarioReturn으로
제출한다. busy/blocked/outside Mario의 0 새 눌림에는 기존 상태줄에 이유를 표시한다.
Server의 현재 action·navigation·collision·중복 요청 검증과 이동 권위는 유지한다.

실제 PlayerController의 gate 분기를 집중 검증하고 변경 TU를 컴파일한다. 실행 중 Client와
Server를 종료하거나 입력을 대신하지 않으며 최종 실제 키 입력 결과는 사용자 확인으로 남긴다.

## G07. 2026-09-16 Play의 서버 랜덤 패턴 두 개

현재 logic55 `쿠크세이튼_마리오_랜덤패턴`은 세 후보/완료 세 번이다. 요청된 후보는 P38 무지개댄스, P39 알비온 감전장판, P40 십자화염폭발, P43 백스텝불뿜기 화염링, P52 분신소환 기분나빠 Parent Summon, P46 백스텝 후 감전빔이다. P52는 명시적인 parent lifetime과 typed Summon이 있고 기존 parent expansion이 실행 Stage를 만든다. 원본 Stage가 없다는 이유만으로 후보에서 배제하지 않고 실제 확장 결과를 검증한다.

`KoukuSaydonActionWorkbench`의 Sequencer Play는 completion-count Logic이 활성인 패턴에 한해 기존 `Request_SelectedServerPlay -> BossTool -> PatternAuditionService`로 제출한다. 저장·게시 revision과 미저장 draft 보호는 유지한다. 서버가 고른 실제 패턴 ID와 시계를 읽어 패턴 선택 및 Sequencer 커서를 따라가며 Client에서 후보를 추첨하거나 완료를 계산하지 않는다. Stop은 같은 typed Server service로 요청한다.

`GameRoom_KoukuAudition`의 기존 shuffle과 PATTERN_COMPLETED 누적을 재사용한다. 이번 확인 범위는 중복 없는 두 패턴의 실제 완료까지이며 다음 마리오 진입은 후속 범위다. Success가 없는 completion-count도 허용해 두 번째 완료 후 기존 run 종료 경로로 정리한다. Success가 연결된 기존 체인은 복귀·followup 동작을 보존한다. Client codec, Python projection, Gameplay publisher와 Server catalog가 같은 optional Success 계약을 검증한다.

기존 C++ 파일만 수정하므로 새 project/filter 등록은 없다. 실행 중 편집은 사용자 Save/종료 후 최신 JSON의 대상 필드만 수정하고 정상 domain publisher를 사용한다. Product Debug Build, 기존 집중 Server 계약 검사, JSON parse와 `git diff --check`를 확인한다. Client 실행·버튼 클릭·화면 판정은 사용자에게 남긴다.
## G08. 2026-09-16 마리오 접촉 즉시 이동

사용자는 마리오1~4 입장 콜라이더와 내부 이동 트리거가 닿는 즉시 작동하도록 수정하도록 요청했다. P34의 플레이어 중심점 검사는 보스 몸통 안에 가려지고, 일반 이동 트리거는 동작 중 실패한 접촉도 inside로 기록해 재진입 전까지 재시도하지 않는다.

- Shared WorldCollisionContract의 기존 이동 접촉 여유를 ServerCollisionSystem과 Mario 입장 body overlap이 함께 사용한다. 기존 보스 차단 전체를 해제하지 않는다.
- KoukuSaydonLogicRuntime의 Mario 전용 입장은 플레이어 몸체와 입장 영역의 접촉을 검사한다. GameRoom 입장은 일반 스킬·상호작용·피격 동작 종료를 기다리지 않으며, 목적지 검증이 성공한 후 기존 동작과 그 combat object를 취소하고 이동을 commit한다. 사망·낙하·잡힘·속박 등 부적합 상태와 실패 시 기존 상태 보존은 유지한다.
- ServerTriggerSystem은 GameRoom의 선택적 move-entry callback을 받는다. GameRoom은 기존 MARIO_LANES에 속하는 이동만 즉시 동작 중단 후 실행한다. 거절한 Mario 접촉은 inside에 소비하지 않아 안에 머무르는 동안 다시 검사한다. 같은 source 이동의 중복 시작과 비마리오 trigger의 기존 동작을 보존한다.
- P34 입장 collider의 MAP 위치·시작·지속 시간을 실제 portal occurrence와 맞춘다. Mario4의 두 requiresInteract 이동은 접촉 이동으로 통일하며 18개 이동을 반복 진입 가능하게 triggerOnce=false로 둔다. Client codec/Workbench/Presentation과 projector가 고정 MAP Collider를 같은 계약으로 소비하도록 연결한다. 앞선 랜덤 두 패턴 후보도 최신 저장본에서 함께 작성한다. 사용자 편집을 보존하며 저장·종료 후 정식 publisher로 설치한다.
- 기존 Server trigger/Mario contract에 실제 이동 충돌 경계의 접촉, 동작 중 즉시 진입, 실패 보존, 재시도, 동일 이동 중복 방지를 검증한다. 변경 TU 최소 컴파일과 정규 Product Build, JSON/publisher와 diff 검사를 수행하고 Client 화면은 사용자가 확인한다. 새 C++ 파일이나 project/filter 등록은 필요 없다.
