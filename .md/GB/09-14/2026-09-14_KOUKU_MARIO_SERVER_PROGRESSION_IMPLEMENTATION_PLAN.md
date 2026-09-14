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
