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
