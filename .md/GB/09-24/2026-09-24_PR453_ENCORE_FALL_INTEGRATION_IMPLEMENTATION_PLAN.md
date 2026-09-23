# PR453 앵콜·낙사 통합 구현 계획

## G00. 기존 관문 흐름과 통합 기준

현재 HEAD의 Composition revision2235, G3 처치 뒤 Server5초 대기, 쿠크5m 낙사면,
1·3관문 펜스와 현재 관문 시작점 부활을 보존한다. PR453의 P97/P10 앵콜 단축,
공통 Sample 기반 가짜 클리어 UI, Mario 사망 복귀와 도박장 보행 이탈을 연결한다.
생성물은 손으로 합치지 않고 통합 담당자가 기존 publisher로 재생성한다.

## G01. Composition 정본과 Server 충돌

`KoukuSaydonComposition.json`은 자동 병합된 stable ID 행을 양쪽 원본과 대조하고
revision을2236으로 올린다. 기존 최신 패턴·피해·흐름은 유지하며 앵콜의5000ms 단축과
자막 스타일만 PR453에서 받는다. 새 JSON/CPP 파일이나 project/filter 등록은 없다.

`GameRoom_PlayerCommands.cpp`는 현재 승인 관문의 부활 목적지를 우선한다. 관문이
아직 승인되지 않은 도박장 보행 테스트만 낙하 전 검증한 중앙 pin을 사용한다. Mario의
사망 복귀가 완료되지 않은 상태에서는 기존 revive 명령을 거절한다.
`GameRoom_PlayerSimulation.cpp`는5m 낙사면과 authored TriggerMove 보호를 함께 유지한다.
1·3관문 펜스는 별도 Mario 내부에 적용하지 않는다. 실제3관문 상태의 Mario 사망과
도박장 중력 하강은 기존 debug-teleport 계약 테스트에서 검사한다. 보행용 바닥 이탈은
일반 넉백의bounded/fence/swept-surface 판정 전에 실행하지 않는다. Mario의명시적인
unbounded push만별도rail 이탈검사를거치고, 나머지는기존forced mover를유지한다.

## G02. 앵콜 표현 UI의 수명과 실패 격리

`KoukuSaydonPresentationPlayer.cpp`의 session UI는 생성 중 실패와 Stop에서
기존 `CUILayoutRuntime::Release_Sprites`를 호출해 Layer 소유권을 해제한다. 새 session이나
역방향 재진입에서 필요한 경우 재생성하고, 현재 occurrence 시간의 기존 keyframe 샘플을 쓴다.

`UI_Sprite.cpp`는 캡션 font 유효성을 admission에서 확인하고 scene UI의 그리기 실패를
해당 sprite에서 진단·격리한다. `Renderer.cpp`의 SCENE_UI 실패는 전체 scene 실패로
확대하지 않는다. UI는 SceneHDR의 유리 후처리 전에 그리며 Debug/Release 공통이다.

## G03. 검증과 남은 화면 경계

변경 JSON parse, 양쪽 Composition stable ID 비교, 기존 앵콜 trim 검사와 diff check를
수행한다. 통합 담당자가 publisher와 Debug/Release Product 빌드, 기존 focused Server
검사를 실행한다. Client/UI를 실행하거나 화면 PASS를 대신 기록하지 않는다.
