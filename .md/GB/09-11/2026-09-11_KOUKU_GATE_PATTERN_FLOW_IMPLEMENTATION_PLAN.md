# 쿠크 관문별 Pattern Flow와 Complete Play 구현 계획

## G00. 현재 경로와 변경 범위

기존 Sequence Workbench의 Complete Play는 선택 관문의 Sequence를 source 순서대로 재생한다.
MainApp은 실제 preview completion을 받아 다음 Sequence를 시작하지만 마지막에는 preview만 정지한다.
Boss Tool과 F1은 게시된 전체 tree를 공유한다. 기존 Play All은 개별 Pattern 배열을 사용하므로
Bundle의 동시 actor 구성을 보존하지 못하고 관문 선택도 실행 범위에 적용하지 않는다.

현재 브랜치 `codex/kouku-gate1-sequence-playback`, HEAD `2d7b96693fb93c397a26632c221d5d29bf4a2ae1`의
대규모 기존 미커밋 작업을 보존한다. 이번 변경만 최소 단위로 덧붙이며 자동 stage/commit하지 않는다.

## G01. 저장과 Boss Tool

기존 KoukuSaydon Composition에 optional 관문별 Pattern Flow를 추가한다. 각 row는 stable row ID와
Pattern 또는 Bundle stable ID를 저장한다. 관문 불일치·누락·실행 불가 항목은 명시 오류로 보존한다.
기존 Composition Document의 parse/validate 및 CAS Save_Atomic을 재사용하고 source revision을 올린다.
Boss Tool에는 All Patterns와 Pattern Flow 탭, 관문 선택, Add From All Patterns, Pattern/Bundle 선택,
순서 변경·삭제·저장·로드·게시와 재생을 연결한다. 미사용 보조 Pattern은 전체 목록에 유지하되
저장 Flow에는 사용자가 선택한 항목만 넣는다. 새 C++ 파일과 project/filter 등록은 필요하지 않다.

## G02. 관문별 재생

PatternAuditionService의 기존 PLAY_SELECTED/PLAY_BUNDLE typed 명령을 재사용한다.
Flow는 immutable row 순서와 게시 revision을 pin하고 정확히 대응하는 Server COMPLETED 뒤에만
다음 row를 제출한다. Bundle 멤버 동시 시작·follow-up·완료·damage는 기존 Server 권위다.
거부·취소·연결 종료·다른 실행으로 교체되면 대기 순서를 버리고 실패 이유를 남긴다.
Composition Play All은 선택 관문의 실행 가능한 Bundle을 하나의 row로 다루며 해당 member를 중복 실행하지 않는다.

F1에는 Saved Pattern Flow와 All Patterns를 구분한다. Complete Play는 선택 관문 Sequence를
처음부터 끝까지 재생한 뒤 기존 preview/camera 소유권을 반환하고 플레이어 follow 시점으로 전환하여
그 관문의 저장 Flow를 시작한다. Stop이나 preview 실패는 전투 시작으로 처리하지 않는다.

## G03. 중앙 이동 후 카드 decal 기준점

댄스타임 P6의 startMs=0, followBoss=false 카드 장판은 첫 actor pose를 고정한다.
Server가 패턴 시작에서 중앙 position/yaw를 확정해도 CNpc의 2 tick 보간 지연으로 이전 pose가
사용될 수 있다. ClientReplication의 새로운 Kouku pattern sequence edge에서만 authoritative
snapshot pose를 즉시 적용하고 보간 buffer를 재설정한다. 일반 이동과 stage 진행은 기존 보간을 유지한다.

## G04. 검증과 실행 경계

초기 G1 Flow는 Pattern 1→2→6→7→29→30이다. 무력화 실패3·성공4·가짜세이튼5는
독립 전투 row에서 제외하고 원래 서버 follow-up/clone 연결을 유지한다. G2는 Bundle
1→2→3→Bundle6→Bundle7→Pattern25→Pattern21→Bundle9→Bundle10→Bundle4→Pattern27이다.
마지막 row 외에는 1000ms를 기다린다. 이후 Boss Tool에서 사용자가 순서를 저장해 바꾼다.
빈 Flow는 Complete Play를 시작하기 전에 사유를 표시하며 임의 전체 패턴으로 대체하지 않는다.
G1 Sequence 정본은 별도 Sequence Composition의 `KAKULSAYDON_G1_PATTERN_1`(팝업북),
`KAKULSAYDON_G1_PATTERN_2`(1관문 피날레) 순서다. 같은 ID인 전투 Pattern과 문서 owner가 다르다.

## G05. 추가 요청: Workbench 트리와 바닥 이펙트

첨부 화면은 Model View=All인데 Gate 행만 표시된다. Action Workbench는 모델 필터로
전체 패턴을 숨기지 않고 Gate→Parent→Bundle→Pattern 트리를 모두 표시한다. 최초 로드가
실패했으면 Saved라고 표시하지 않고 실제 사유와 재로드 버튼을 제공한다. Sequence workspace의
관문 선택은 유지한다. 기존 미커밋 저작값은 재로드 시 dirty guard로 보존한다.

내려찍기 P29와 거미카운터 P15는 사용자 관찰상 바닥 효과가 누락됐다. 기존 bone basis100과
preScale 변환 적용 여부, 발생 좌표·시간·크기, decal/sprite의 실제 carrier 소비를 조사한다.
임의 x100 보정이나 연기 발생 성공을 바닥 효과 성공으로 대신하지 않는다. 실측 원인과 수정,
자동 검증 및 사용자 화면 확인 경계를 RESULT에 분리한다.

변경한 저장 문서의 parse·roundtrip·실패 보존, 관문별 Pattern/Bundle 순서, 정확한 lifecycle에만
다음 row 진행, Stop/실패와 Sequence 마지막 완료 구분을 기존 focused 검사로 확인한다.
필요한 Debug C++ compile/link, 해당 Composition publish, JSON/XML parse와 git diff --check를 수행한다.
Client/UI는 실행·조작·캡처하지 않는다. 사용자에게 Server + Client profile과 F1 실제 클릭 경로를 전달하고
두 Sequence 종료→플레이어 시점→저장 Flow, Gate2 Bundle 동시 멤버, 중앙 카드 장판 화면 확인을 남긴다.
