# 쿠크 1관문 HP 구간 반복 Flow 구현 계획

## G00. 목표와 현재 기준점

현재 G1은 `patternFlows`의 28개 entry를 HP 조건 없이 순서대로 반복한다. P1/P2/P6/P7 기믹과 일반 패턴이 같은 목록에 섞여 있다. 기존 BUNDLE은 여러 actor의 동시 재생이고 Parent pattern은 동적 counter 후속을 자식으로 수용하지 않으므로 일반 패턴 반복은 기존 raid flow scheduler에서 처리한다.

사용자가 확정한 기믹 목표는 130줄 무력화, 110줄 진짜 세이튼, 85줄 댄스, 60줄 무력화, 50줄 룰렛, 30줄 진짜 세이튼이다. 실제 일반 패턴 묶음과 전환 시점은 사용자 답변 후 authoring 데이터에 반영한다. 이 문서는 그 답변과 독립적인 저장·검증·서버·UI 구조만 먼저 구현한다.

## G01. Flow 그룹 저장과 검증

`KoukuSaydonCompositionDocument.h/.cpp`의 기존 flow에 optional `entryGroups`를 추가한다. 그룹은 stable `groupId`, `displayName`, 기존 entry의 `startEntryId`와 `endEntryId`를 갖는다. Optional `repeatUntilHealthBars`는 정수 HP 줄 임계이며, `transitionAt`은 `PATTERN_END` 또는 `GROUP_END`로 완료 경계를 명시한다. 임계가 없는 그룹은 순서 표시만 한다.

그룹은 entry 순서 기준으로 서로 겹치지 않는 연속 범위를 참조한다. 참조 누락·중복 ID·역전·범위 초과·legacy `loopStartEntryId`와 반복 그룹의 혼용은 실패한다. parse → validate → stage → commit 및 기존 디스크 freshness 검증을 유지한다. 필드가 없으면 기존 문서와 재생 동작을 보존한다.

Python composition projector, raid projection, PowerShell publisher가 같은 그룹을 검증하고 `RAIDFLOWGROUP` bootstrap row로 게시한다. `GameplayCatalog`는 게시된 stable ID를 entry index 범위로 검증한 뒤 보관한다. 원본 JSON과 runtime bootstrap의 직접 편집은 하지 않는다.

## G02. Server 권위 HP 전환

`GameRoom_KoukuRaidFlow.cpp`의 기존 완료 receipt 처리에서 현재 primary boss HP와 profile의 최대 HP·줄 수를 정수 비교한다. 현재 패턴의 dynamic counter 후속을 포함한 `COMPLETED`가 도착하기 전에는 다음 entry를 시작하지 않는다. 임계 미도달 상태에서 그룹 끝은 그룹 처음으로 돌아가고, 도달하면 저장된 전환 경계에 따라 그룹 다음으로 이동한다. 보스 사망·관문 reset·world 전환은 기존 raid lifecycle이 우선한다.

새 HP 시뮬레이션이나 Client HP 판정을 만들지 않는다. `GameRoom.h`와 Shared packet 형식은 변경하지 않고 기존 flow index와 pinned catalog를 사용한다.

## G03. F1 목록과 실행 경계

`KoukuSaydonBossTool`은 그룹을 접을 수 있는 flow 목록으로 표시하고 기존 flow editor에서 그룹 이름·범위·HP 조건을 편집한다. HP 반복 flow의 `Play Saved Pattern Flow`는 Complete Play callback으로 전달해 flat Client audition 순서 실행을 방지한다. MainApp callback 연결은 통합 담당자가 수행한다.

Release의 F1 Load Pattern/Complete Play에 필요한 typed Kouku audition와 raid request만 Debug 전용 거절에서 해제한다. session/world/revision/target/epoch 검증을 유지하고 다른 debug teleport·madness 기능의 guard는 건드리지 않는다.

## G04. 검증과 완료 조건

기존 raid projection focused tests에 stable group round trip, 누락 참조·겹침·역전·임계 값·잘못된 전환 경계 거부, PowerShell row 정렬 및 검증을 추가한다. native scheduler/카탈로그 소비자를 Debug와 Release에서 검사하고 변경 C++의 최소 컴파일을 수행한다. JSON parse, 인코딩·개행 유지, `git diff --check`를 기록한다. Product build/publish/commit은 root 담당이며 Client UI는 실행하지 않는다. 실제 묶음 선택 전에는 authoring 패턴 순서를 바꾸지 않는다.
