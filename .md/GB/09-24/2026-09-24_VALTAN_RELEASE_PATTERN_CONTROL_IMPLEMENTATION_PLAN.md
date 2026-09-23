# Valtan Release 패턴 실행 연결 계획

## G00 — 현재 결함과 완료 범위

F1의 Load Pattern / Complete Play / 저장 flow는 기존 typed command를 사용하지만 Client BossTool과 Level 준비/응답 루프, Server audition 상태와 fixed tick/lifecycle가 각각 `_DEBUG`로 빠진다. Release에서도 같은 stable placement, gameplay revision, 요청 sequence, navigation/collision 검증을 거쳐 실행한다. 별도 재생 경로를 만들지 않는다.

## G01 — Server 권위 경로

Valtan audition 선언·상태·도움함수와 stable-ID play/restart/next/flow, owner 이탈·방 초기화·세대 pin·terminal lifecycle를 공통화한다. 외부 진입은 Valtan Arena에 한정한다. 기존 retired health/timeline opcode 거절을 유지하고 일반 wave/월드 재생/자살 명령은 열지 않는다. Arena teleport는 Valtan/Kouku world에서만 Release 허용하며 기존 목적지 검증 및 session 자기 player만 적용한다. spawn은 기존 disabled boss placement allowlist, despawn은 해당 boss 소유 트리만 사용한다.

## G02 — Client 제출과 응답

Level의 replicated boss 준비/제거와 arena preset 상태를 공통화한다. BossTool의 admission/Complete Play와 root MainApp의 공통 update를 연결한다. PlayerController의 고정 위치 제출·correlated result drain·disconnect 정리는 공통화하고 viewport 자유 picking은 Debug에 둔다. PatternAuditionService는 이미 공통이므로 재구현하지 않는다.

## G03 — 검증

기존 Valtan audition/flow native 검증을 양 구성에서 실행 가능하게 하고 Release 거절만 기대하던 테스트를 현재 공통 admission 계약으로 바꾼다. 잘못된 world/session/revision/legacy opcode가 상태를 바꾸지 않는 회귀 검사를 유지한다. C++ 신규 파일은 없으므로 프로젝트 등록 변경 없음. root가 Debug/Release Product를 빌드하고 관련 native 검사를 실행한다. Client 화면 확인은 사용자 담당이다.
