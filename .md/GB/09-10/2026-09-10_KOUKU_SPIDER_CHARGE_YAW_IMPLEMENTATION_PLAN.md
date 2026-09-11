# 쿠크 거미 돌진 몸 방향 보정 구현 계획

## G00. 현재 동작과 수정

Server charge는 window 시작에 target 방향을 잡아 7m EndPosition과 atan2 body yaw를 함께 확정한다. 거미 native clip의 몸 축은 이동 방향에 대해 90도 차이가 있으므로 기존 charge에 optional `chargeYawOffsetDegrees`를 추가한다. 기본값은 0이며 거미 ENTER_AREA logic만 +90을 저장한다. 이동 EndPosition, Navigation/Collision과 target 고정은 기존 식을 유지하고 body yaw만 offset을 합한다.

## G01. 소비 경로

Composition logic H/CPP의 parse/validate/save, ActionWorkbench Charge distance 옆 yaw 편집, projector Encounter logicWindow, Gameplay publisher PATTERNLOGICCHARGE, Server catalog/window validation과 실제 LogicRuntime motion yaw로 연결한다. yaw는 finite -360..360이고 양수 charge distance가 있을 때만 사용할 수 있다. 기존 bootstrap 5열은 yaw 0으로 읽고 새 6열은 명시 값을 읽는다. GameRoom과 다른 패턴의 공통 native axis는 변경하지 않는다. 사용자 저장 중인 JSON은 직접 쓰지 않고 대상 logicId의 정확한 patch만 root에 전달한다.

## G02. 검증

기존 Server charge contract에 +90 body yaw와 기존 7m 끝점·target 고정·접촉을 확인하고 기본 0 보존도 검사한다. 기존 projector charge test에서 필드 투영과 비정상 yaw/거리 없는 yaw 거부를 확인한다. 변경 C++ 최소 컴파일과 JSON/PowerShell/Python parse, diff check를 수행한다. 새 C++ 파일은 없어 프로젝트/filter 등록은 없다. Product 최종 빌드와 publisher 통합은 root가 담당하며 Client 화면 판정은 사용자에게 남긴다.
