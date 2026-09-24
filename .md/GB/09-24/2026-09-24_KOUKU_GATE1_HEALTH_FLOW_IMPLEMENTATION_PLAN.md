# 쿠크 1관문 HP 구간 반복 Flow 구현 계획

## G00. 목표와 현재 기준점

현재 G1은 `patternFlows`의 28개 entry를 HP 조건 없이 순서대로 반복한다. P1/P2/P6/P7 기믹과 일반 패턴이 같은 목록에 섞여 있다. 기존 BUNDLE은 여러 actor의 동시 재생이고 Parent pattern은 동적 counter 후속을 자식으로 수용하지 않으므로 일반 패턴 반복은 기존 raid flow scheduler에서 처리한다.

사용자가 확정한 기믹 목표는 130줄 무력화, 110줄 진짜 세이튼, 85줄 댄스, 60줄 무력화, 50줄 룰렛, 30줄 진짜 세이튼이다. 최초 변경은 저장·검증·서버·UI 구조까지였으나, 같은 날 사용자가 실제 F1 목록과 HP 기믹이 반영되지 않았다고 지적하고 적용을 요청했다. 이번 후속 변경은 현재 저장본의 G1에 실제 반복 구간과 기믹을 저장하고 게시한다.

## G01. Flow 그룹 저장과 검증

`KoukuSaydonCompositionDocument.h/.cpp`의 기존 flow에 optional `entryGroups`를 추가한다. 그룹은 stable `groupId`, `displayName`, 기존 entry의 `startEntryId`와 `endEntryId`를 갖는다. Optional `repeatUntilHealthBars`는 정수 HP 줄 임계이며, `transitionAt`은 `PATTERN_END` 또는 `GROUP_END`로 완료 경계를 명시한다. 임계가 없는 그룹은 순서 표시만 한다.

그룹은 entry 순서 기준으로 서로 겹치지 않는 연속 범위를 참조한다. 참조 누락·중복 ID·역전·범위 초과·legacy `loopStartEntryId`와 반복 그룹의 혼용은 실패한다. parse → validate → stage → commit 및 기존 디스크 freshness 검증을 유지한다. 필드가 없으면 기존 문서와 재생 동작을 보존한다.

Python composition projector, raid projection, PowerShell publisher가 같은 그룹을 검증하고 `RAIDFLOWGROUP` bootstrap row로 게시한다. `GameplayCatalog`는 게시된 stable ID를 entry index 범위로 검증한 뒤 보관한다. 원본 JSON과 runtime bootstrap의 직접 편집은 하지 않는다.

## G02. Server 권위 HP 전환

`GameRoom_KoukuRaidFlow.cpp`의 기존 완료 receipt 처리에서 현재 primary boss HP와 profile의 최대 HP·줄 수를 정수 비교한다. 현재 패턴의 dynamic counter 후속을 포함한 `COMPLETED`가 도착하기 전에는 다음 entry를 시작하지 않는다. 임계 미도달 상태에서 그룹 끝은 그룹 처음으로 돌아가고, 도달하면 저장된 전환 경계에 따라 그룹 다음으로 이동한다. 보스 사망·관문 reset·world 전환은 기존 raid lifecycle이 우선한다.

새 HP 시뮬레이션이나 Client HP 판정을 만들지 않는다. `GameRoom.h`와 Shared packet 형식은 변경하지 않고 기존 flow index와 pinned catalog를 사용한다.

## G03. F1 목록과 실행 경계

`KoukuSaydonBossTool`은 그룹을 접을 수 있는 flow 목록으로 표시하고 기존 flow editor에서 그룹 이름·범위·HP 조건을 편집한다. HP 반복 flow의 `Play Saved Pattern Flow`는 Complete Play callback으로 전달해 flat Client audition 순서 실행을 방지한다. MainApp callback 연결은 통합 담당자가 수행한다.

실제 저장된 그룹에서 HP 기믹 요약을 구성하고 반복 구간은 청색 framed box, 일회 기믹은 금색 framed box로 구분한다. 반복 구간은 처음에는 접고 기믹은 펼쳐 전체 HP 순서를 읽을 수 있게 한다. 그룹 제목은 펼침·접기만 수행하며 기존 Pattern/Bundle 행의 stable ID 선택과 Load Pattern 실행을 유지한다.

Release의 F1 Load Pattern/Complete Play에 필요한 typed Kouku audition와 raid request만 Debug 전용 거절에서 해제한다. session/world/revision/target/epoch 검증을 유지하고 다른 debug teleport·madness 기능의 guard는 건드리지 않는다.

## G04. 검증과 완료 조건

기존 raid projection focused tests에 stable group round trip, 누락 참조·겹침·역전·임계 값·잘못된 전환 경계 거부, PowerShell row 정렬 및 검증을 추가한다. native scheduler/카탈로그 소비자를 Debug와 Release에서 검사하고 변경 C++의 최소 컴파일을 수행한다. JSON parse, 인코딩·개행 유지, `git diff --check`를 기록한다. 최초 구조의 빌드·native 검증은 완료했으며, 후속 데이터 변경에는 기존 schema 검증과 공식 publisher를 사용한다. Product build/commit은 root 담당이고 Client UI는 실행하지 않는다.

## G05. 실제 G1 HP 구간 저장과 게시

현재 `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json` revision 2237의 G1만 변경한다. 저장된 후보의 전체 문서를 복사하지 않고 최신 문서의 G1 flow에 stable entry ID 단위로 병합한다. 기존 28개 entry의 ID와 모든 필드, P81/P118 counter 성공 후속, 다른 관문과 모든 pattern은 보존한다. 새 저장 revision은 2238이다.

일반 구간은 P58/P102/P81/P82/P47/P100/P80/P103/P48/P79/P78/P83 12개에 기존 P101 추적 8개와 P78 뒤 P104 1개를 포함한 21개 entry다. 이를 160→130, 130→110, 110→85, 85→60, 60→50, 50→30, 30→0줄의 7개 반복 그룹에 배치한다. 그룹 사이에는 P1/P2/P6/P1/P7/P2 기믹과 각 기믹 뒤 기존 P104를 둔다. 총 159 entry와 13 group이며 전환은 `PATTERN_END`다. 카운터 성공 후 동적 무력화는 기존 COMPLETED 안에서 끝난 뒤 HP 전환한다.

최신 bytes/hash와 revision을 검증하고 후보의 전체 schema 및 기존 entry 보존 검사를 마친 뒤 원본 백업, 재확인, 원자 교체를 수행한다. 동시 저장이 확인되면 해당 파일을 덮어쓰지 않는다. 게시 실패 시 공식 owner의 runtime rollback을 유지하며 source 복구도 자신이 저장한 hash일 때만 수행한다.

공식 게시 명령은 `powershell -NoProfile -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon -ExpectedKoukuSaydonSourceRevision 2238`이다. 이 owner는 기존 projector, Map, Kouku World, Retail Gameplay의 입력·출력 검사와 transaction을 사용한다. 생성 JSON의 G1 159 entry/13 group, `Gameplay.bootstrap`의 source revision과 `RAIDFLOWGROUP` 13행을 확인한다. 실행 중 Client·Server 종료/재실행/Reload는 수행하지 않으며 디스크 게시와 현재 메모리 반영을 구분한다. 이 단계는 C++/프로젝트 등록 변경이 필요하지 않다.
