# 쿠크 재생 복구와 입구 표식 수정

## G00. 실제 원인과 범위

사용자 화면의 `Saved Composition and published Boss Patterns differ`는 저장 revision332와
실행용331의 차이로 발생한다. 추가된 것은 빈3관문 쇼타임 초안이며 기존 실행 패턴은 같다.
초안은 보존하고 공식 KoukuSaydon publisher로 실행 문서를 맞춘다. revision 검증을 없애거나
서버 권위 재생을 Client 로컬 재생으로 바꾸지 않는다.

현재 `codex/character-select-200fps` 작업 트리에는 성능·이펙트 복구 변경이 함께 있다.
기존 diff를 보존하며 자동 stage/commit하지 않는다. Client와 Server는 사용자가 실행 중이다.

## G01. Complete Play와 Sequencer

MainApp의 F1 Complete Play에서 기존 Publish Saved Patterns 요청을 바로 제출할 수 있게 한다.
재생 준비는 열려 있는 편집기의 이전 revision 대신 실제 저장 문서와 게시 문서를 비교한다.
미저장 draft와 게시 중 상태의 차단은 유지한다. 게시 완료 후 목록 갱신과 Server audition의
기존 exact revision 계약을 사용한다.

Composition Sequencer의 preview 요청, MainApp transport, Presentation/World 실행과 실패
상태 전파를 실제 호출 흐름으로 점검한다. 발견한 재현 가능한 실패만 수정하며 독립 Sequence
문서를 전투 Composition 또는 게시된 SHADOW 문서로 대체하지 않는다. 동일요청 WORLD 문서
검증을1회로 묶고, Begin 뒤 첫 own-clock Update에 포함된 준비 delta를 제외한다.
G2 fade/light의 빈 WORLD 앵커를 MAP으로 바로잡고 Play 하단에 실패 status를 표시한다.

## G02. 클릭과 입구 트리거 표식

ClickMoveEffect는 우클릭 제출 성공 시 mouse_click만 생성한다. 쿠크의 move_destination은
같은 클릭 위치에 생성하던 분기를 제거하고 Level의 입구 jump/paper 트리거5개 중심에 붙인다.
실제 Gameplay.world.json의 stable placement와 collider 중심을 소비하며 좌표를 복제하지 않는다.
기존 EffectPresentationService의 prepared handle과 외부 sampling을 재사용하고 Level 종료 때
정리한다. 새 Effect runtime이나 GameObject 종류는 만들지 않는다.

mouse_click의 LocalDecal2개는 생성 시계, 첫 샘플, 크기·알파 곡선을 mesh/sprite와 대조한다.
사용자가 바꾼 life 값은 보존하고, 지연을 만드는 실제 계약을 확인한 뒤 해당 부분만 수정한다.

## G03. 검증과 실행 적용

각 변경 CPP의 인코딩과 줄 끝을 유지한다. 변경 기능에 맞는 기존 focused contract와 JSON parse,
publisher 검증, diff check 및 최소 컴파일·링크를 수행한다. 기존 프로젝트 파일 안에서 구현하며
새 C++ 파일이 필요하면 실제 호출자와 project/filter 등록을 함께 확인한다.

실행 중인 사용자 Client/Server는 종료하지 않는다. 출력 잠금이 있으면 독립 컴파일을 먼저 끝내고
최종 기본 EXE 링크에 필요한 종료를 사용자에게 알린다. 화면 실행·캡처·visual PASS는 사용자 몫이다.
실제 수정, 검증, 배포된 EXE와 사용자 재실행 필요 여부는 같은 이름의 RESULT에 구분한다.

## G04. 2026-09-16 우클릭 최초 입력에서만 클릭 표식 생성

일반 우클릭 이동은 목적지 갱신마다 `Request_MoveToPoint`를 호출하고, 이 함수가 매번
`CClickMoveEffect::Play`로 기존 표식을 중단·재생한다. 이동의 50ms 재전송과 목적지 변화
검사는 유지하며, 표식 생성 여부만 물리 우클릭의 최초 down edge로 제한한다.

`PlayerController.h/.cpp`의 기존 이동 함수에 기본값 true인 `playClickEffect` 인자를 추가한다.
일반 이동 caller는 별도로 관찰한 raw press edge를 전달한다. 이 상태는 모든 early return보다
먼저 갱신하여 UI 차단 해제·잡힘 해제·타기팅 종료가 같은 hold를 새 클릭으로 만들지 않는다.
새 캐릭터 session에서 초기화하고 동일 player의 presentation rebind에서는 보존한다.
Bern NPC 접근의 두 기존 caller는 기본값으로 한 번의 클릭 표식을 유지한다. 송신 실패에는
표식을 생성하지 않는다. 새 C++ 파일이나 project/filter 등록, Resources 변경은 없다.

변경 CPP 최소 컴파일, 기존 입력 경계 검사와 diff check를 확인한 뒤 정식 증분 Product Build를
수행한다. 실행 중 Client/Server의 미저장 작업은 보존하고, 출력 잠금으로 제품 빌드가 차단되면
격리 컴파일과 미적용 EXE를 구분한다. 사용자가 hold 이동·놓고 재클릭·NPC 접근을 직접 확인한다.

후속 사용자 요청으로 이번 종료 범위는 소스 수정과 컴파일 확인까지다. 정식 Product 재시도,
실행 중 Client/Server 종료와 실행 파일 교체는 수행하지 않는다.


## G05. 2026-09-16 Complete Play 보스 준비·애니메이션 복구

현재 사용자는 마리오1페이즈의 G3 세이튼 미생성 거부와 1관문 Live 중 애니메이션 누락을 보고했다. 서버는 선택 패턴의 대상 placement가 없으면 Parent/Summon 실행 전에 거부한다. 선택 패턴의 gate가 준비되지 않았으면 BossTool이 기존 Level::Debug_ActivateGate를 요청하고 보스 spawn·플레이어 이동·맵·조명·HUD의 승인 완료를 기다린다. 이후 고정한 source revision과 연결 generation을 다시 검사한 뒤 같은 audition 요청을 보낸다. 준비 중 중복 재생·다른 관문·Stop·실패·레벨 변경은 지연 요청을 취소한다. 서버의 missing boss 검증은 유지하며 보스만 임의 생성하지 않는다.

1관문 binding 문서에는 현재 targetedCombatVisuals가 있으나 PresentationAssetService의 root 허용 필드에 빠져 전체 애니메이션 문서를 거부한다. 기존 소비자가 이미 지원하는 optional field를 허용하며 잘못된 schema/version/revision/clip의 거부를 유지한다. Effect 재생 성공과 실제 보스 animation binding 성공을 따로 확인한다.

PatternAuditionService::Reset은 레벨 초기화 사유를 실행 snapshot과 Flow 상태에 동시에 남긴다. 이후 단독 재생이 ACTIVE여도 이전 Flow 문구가 계속 표시된다. Reset 시 Flow 상태를 비우고, 성공적으로 제출한 단독 재생에서 이전 Flow 결과를 정리한다. 제출 실패에는 이전 결과를 보존하며 Stop/control 요청과 실제 Flow 진행은 그대로 유지한다.

기존 C++ 파일과 필요한 기존 테스트만 수정하며 새 제품 파일·project/filter 등록은 없다. 현재 RenderingProfiles의 사용자 변경은 보존한다. 변경 TU 컴파일, 실제 binding parser 및 Server 생성·거부 보존 검증, diff check와 정상 Product 증분 빌드를 수행한다. 실행 중 Client/Server의 저장·종료와 화면 조작은 사용자에게 남기며, 출력 잠금이 있으면 먼저 격리 컴파일을 끝낸다.

후속 사용자 지시로 최종 Product 빌드는 사용자가 직접 수행한다. 에이전트는 변경 CPP5개 격리 Debug 컴파일과 집중 기능 검증까지 수행하며 표준 EXE를 링크하거나 Client를 실행하지 않는다.
