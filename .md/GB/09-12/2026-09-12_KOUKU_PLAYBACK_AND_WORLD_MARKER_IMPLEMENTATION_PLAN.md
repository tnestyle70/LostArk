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
