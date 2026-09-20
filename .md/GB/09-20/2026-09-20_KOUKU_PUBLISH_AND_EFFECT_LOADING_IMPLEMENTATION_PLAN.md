# 쿠크 게시와 입장 이펙트 준비 시간 개선

## G00. 현재 기준과 변경 경계

`origin/main`의 `9f2a9feb0dff04164f23bbc78ac50222c91aa326`으로 main을 fast-forward하고
사용자가 지정한 `GB/koukubugfix-bingo`를 생성했다. 기존 Encounter와 patternbindings의
미커밋 변경은 safety stash로 보존한 뒤 그대로 복원했다. 이 두 저장본을 성능 실험으로
교체하지 않는다. 실행 중인 Client와 Server, 편집 중 메모리 초안은 유지한다.

현재 게시 receipt의 실제 action 시간은 Kouku projection 179597ms,
Gameplay balance 240316ms, world gameplay 4714ms다. map의 80354ms receipt는
전날 기록이므로 이번 실행 시간에 더하지 않는다. 별도 PowerShell 5.1 측정에서
각 domain fingerprint는 204/61/72/702ms, source revision JSON 읽기는 208ms였다.
전체 검증·hash·JSON 자체를 병목으로 단정하거나 생략하지 않는다.

## G01. 게시 단계의 실제 비용 제거

`Tools/KoukuSaydonPipeline`의 projection과 `Tools/GameplayPipeline`의 게시기를
별도 out snapshot에서 같은 입력으로 실행한다. Python 함수별 profile과 PowerShell
단계 시간을 나누어 반복 직렬화·정렬·검증의 실제 지배 비용을 찾는다. 기존 코드가
만들던 결과와 후보 결과의 byte 또는 의미 동등성을 확인하고, 입력 freshness,
validation, 원자적 교체와 실패 rollback은 유지한다.

함수 profile에서 큰 비용을 확인한 `_publication_candidate`는 선택 Pattern만 복사하고,
`_expand_parent_patterns`는 하나의 private 문서를 소유해 순서대로 확장한다. memo의
identity는 전체 field를 포함한 compact JSON으로 계산하며 제품 JSON 형식은 유지한다.
native root curve는 pinned actor·clip·vertical scale에 묶고 Object collider 모델과
pose는 같은 publication·repository root 안에서 재사용한다. 모델과 sequence의 강한
참조를 유지하며 서로 다른 sequence 객체는 같은 ID라도 별도 계산한다. pinned World
문서의 digest도 한 번만 계산한다. animation key 검색은 매번 timestamp 배열을
만들지 않고 기존 정렬 배열에서 동일한 이분 탐색을 수행한다.

Gameplay의 worldTrack key loop는 배열을 한 번 만들고 숫자 type/finite/range 검사와
invariant 서식을 한 순회로 처리한다. grip 트랙의 고정 조건은 첫 key에서 검사하고
모든 key의 grip 존재 일치·좌표 수·숫자·범위 검사는 유지한다.
모든 변경은 기존 산출물과 동등해야 하며 캐시가 입력 hash 재확인을 대신하지 않는다.

숫자 허용·서식 동등성은 `Tools/GameplayPipeline/Test-GameplayWorldTrackNumbers.ps1`에서
실제 publisher 함수를 읽어 확인하고, collider cache는 기존
`test_world_object_collider.py`에 bake 수명과 같은 ID의 다른 sequence 회귀를 추가한다.

`Tools/Build/Invoke-BuildDomainOwner.ps1`은 기존 Workbench 로그에 owner lock 대기,
domain 시작·완료 시간과 전체 시간을 기록한다. 새로운 publisher나 별도 runtime
경로를 추가하지 않는다. Save는 기존 저장 책임을 유지한다.

## G02. Debug와 Release의 이펙트 준비

기준 구현은 09-18 `KOUKU_PATTERN_RUNTIME_REPAIR_RESULT` G10이다. 당시 closure는
V1 119개와 V2 45개였다. 현재 문서에서 대상을 다시 수집해 수량을 확인한다.
기존 `CLoader::EffectThreadMain`의 worker가 이미 map/character와 병렬로 실행되므로
worker 존재 여부와 CPU parse/decode, GPU resource 준비, Level activation을 구분한다.

기존 실제 Debug PID51692 로그는 느린 쿠크 V1 116개 준비 구간333646ms,
클래스·marker 포함161개 구간398001ms를 기록했다. 현재 V1은 이미3개 window와
공용 budget4로 병렬 준비한다. 따라서 Debug는 G10에서 추가한 전체 쿠크
V1/V2/World gate만 기존 lazy 소비로 복귀시키고, 클래스·marker·BossCatalog의
기존 필수 선준비는 유지한다. Release는 G10 전체 closure 준비를 그대로 유지한다.
기록 임계값50ms와 V2/World 시간 누락 때문에 이 수치를164개 전체 또는 총입장
시간으로 부르지 않는다. 다음 사용자 실행에서 누락 구간을 구분할 최소 시간 로그를
기존 준비 경계에 연결한다.

기존 비UI 진단을 재사용해 같은 설치 입력의 Debug CPU 처리와 worker 수에 따른 시간을
측정한다. Release는 published 전체 Effect 선준비와 필수 실패의 입장 거절을 유지한다.
Debug는 실측과 thread ownership에 맞춰 기존 lazy 경로 또는 기존 worker 개선으로
입장 시간을 줄인다. 부분 CPU 측정을 전체164 GPU 준비나 아레나 진입 시간으로
보고하지 않는다. 실제 실행에서 남는 시간은 단계별 수치 진단으로 기록한다.

## G03. 검증과 결과

publisher는 실제 저장 입력의 전후 시간, 출력 동등성, 실패 시 보존을 확인한다.
C++은 기존 파일 인코딩을 유지하고 정상 증분 Product Debug/Release를 수행한다.
출력 EXE/DLL이 실행 중 프로세스에 점유됐으면 임의 종료하지 않고 필요한 최소 컴파일을
먼저 끝낸 뒤 사용자의 종료를 요청한다. 새 C++ 파일은 계획하지 않으며 기존 프로젝트
등록을 유지한다. 변경한 JSON/XML parse와 `git diff --check`를 확인한다.

RESULT는 소스 구현, 실제 측정, Product 설치, 사용자 아레나 확인을 분리한다.
사용자는 새 바이너리에서 Lobby → KoukuSaydon 진입과 Workbench의 Save → Publish를
확인한다. 에이전트가 Client/UI를 자율 실행하거나 화면 판정을 대신하지 않는다.

## G04. Debug Complete Play 선택 리소스 준비 장벽

Debug 입장에서는 전체 raid 선준비를 계속 생략하되, 사용자가 Complete Play를 누른 선택의
모든 이펙트가 준비된 다음 기존 typed Server 요청을 제출한다. Pattern, Bundle, Pattern Flow는
게시 Encounter의 후속·분신·소환 Pattern ID를 닫고, projected presentation의 모든 시간대와
Fear/targeted visual, WORLD group/NEXT를 수집한다. Stage 종료보다 늦은 row도 제외하지 않는다.

BossTool의 기존 Gate 승인 대기에 resource phase를 추가한다. V1은 기존 worker queue의
실제 prepared/current/failure probe를 사용하고 V2, actor, WORLD는 기존 main-thread 준비기를
한 항목씩 진행한다. queued 또는 settled만으로 재생을 허용하지 않는다. Gate 20초와 resource
20분 제한을 분리하고, 실패·취소·리비전 변경은 서버 시작 없이 종료한다.

Sequence + Pattern Flow는 전체 raid를 진행하므로 Release에서 사용하는 전체 dependency
collector를 재사용해 요청 전에 준비한다. 기존 raid PREPARING의 모든 Debug 참가자도 같은
실제 준비가 끝나야 READY를 회신한다. Server의 Debug PREPARING만 20분으로 확장하고 Release의
기존 10초, 전투·컷씬 clock, owner STOP/FAILED/이탈/리비전 검사는 유지한다. 사용자 메모리
초안이나 authoring JSON은 수정하지 않는다.

기존 소스 인코딩·줄바꿈을 보존하고 새 C++ 파일은 추가하지 않는다. 실제 collector를 격리
콘솔에서 실행해 게시 입력과 누락·미래 tail·리비전 불일치를 확인하고, Client Debug/Release와
Server 변경 TU를 최소 컴파일한다. Product 통합과 Server focused 실행은 전체 작업 owner가
취합하고 Client/파티 화면 확인은 사용자가 한다.
