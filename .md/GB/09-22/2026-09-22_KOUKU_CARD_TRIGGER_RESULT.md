# 세이튼 카드 Trigger 연결 결과

## G00. 실제 원인과 보존한 입력

작업 시점 Composition revision2166에서 사용자의 `KAKULSAYDON_G1_PATTERN_78.logic.9~12`는 definition125~128을 참조하며 시작 시간은 각각7643/9848/11997/14252ms다. definition들은 `logicType=TRIGGER`만 있고 triggerKind가 없어 실행 의미가 없었다. 동시에 기존 logic.1의 definition73은 DURATION과4000ms 간격을 유지했다.

후보는 네 기존 ID·표시명·시작 시간·box 길이를 보존하며 definition125~128에 `triggerKind=PURSUIT_PROJECTILES`, 각 heart/spade/clover/diamond 하나, 속도3m/s·생성반경3m·접촉반경0.5m·기존10% CONTACT hit·lifetimeMs0·spawnIntervalMs0·countPerWave1을 연결한다. old logic.1은 enabled만false로 바꾼다. trigger 길이는 새 카드 수명으로 사용하지 않는다.

live Composition은 이 하위 작업에서 쓰지 않았다. 통합 병합 함수는 `out/KoukuCardTriggers20260922/prepare_card_triggers.py::prepare(document)`이며 최신 저장본의 무관한 필드를 그대로 복사하고 해당 stable ID만 수정한다. 통합 담당자가 최신 source 재확인·백업·원자적 교체와 publish를 소유한다.

## G01. 기존 런타임 소비자 연결

- `KoukuSaydonCompositionDocument.cpp`는 TRIGGER pursuit의 typed validation, parse, serialize와 resource reference 검증을 연결했다. Trigger의 반복 interval은0만 허용하며 DURATION 회전카드 기능은 유지했다. maxDistanceM은 triggerKind를 먼저 읽고 기존 pursuit 전용 member로 분리한다.
- `KoukuSaydonActionWorkbench.cpp`의 기존 추적 카드 편집기를 공용 lambda로 옮겨 Trigger 선택과 동일 속도/접촉/수명 편집을 제공한다. Trigger는 box 시작에 한 번 발생한다는 설명을 표시한다.
- projector는 같은 pursuit payload와 Client visual template에 투영한다. 일반 mechanicTriggers에 중복 생성하지 않는다.
- `KoukuSaydonPresentationPlayer_LogicPreview.cpp`는 triggerKind도 수집·재생하고 이미 지난1ms Trigger도 최초 관찰 시 누락하지 않는다. DURATION의 기존 생성 창은 유지한다. 제품 Preview의 Server-clock 라우팅은 통합 collider 작업의 소유 범위다.
- `KoukuSaydonLogicRuntime.cpp`는 interval0의1ms box가30Hz 올림에서 startTick=endTick이 되더라도 최소 한 birth tick을 보존한다.
- `GameRoom_BossSimulation.cpp`와 Preview의 homing 원점은 기존270도에서90도로 바뀐다. 실제 Saydon +X 전방이며 기존 뒤쪽의 정확한180도 반대다. random 회전카드는 이 조건에 들어가지 않는다.
- persistent 카드에서 authoring 검증용600000ms 전체 수명 CONTACT hit만 runtime end0으로 바꾼다. 명시적으로 짧은 window는 보존한다. 기존 CombatObject의 end0 접촉 판정과 room-owned lifetime을 재사용해10분 이후에도 접촉 피해가 살아 있다.

새 C++ 파일·packet·프로젝트 등록은 없다. 기존 UTF-8 BOM 없음과CRLF를 보존했다.

## G02. 실행한 검증

1. focused Python pursuit 기존3개와 신규 Trigger1개:4 tests / OK. 신규 검증은913ms의1ms box, 단발·lifetime0, generic mechanic 중복 없음, 유한 lifetime일 때도 Trigger 반복 interval 거부를 확인했다. 기존 bootstrap 검증이 실제 함수의 새 의존성 `New-KoukuAttackHitRows`를 fixture에 가져오지 않던 문제도 함께 바로잡았다.
2. fresh native Client codec:15 checks / failures0. 후보 parse·Validate·exact roundtrip, 네 Trigger의 단발 영구 카드 값, repeating Trigger 거절, finite maxDistance의 pursuit lane roundtrip을 확인했다. 옛9월15일 dependency object 재사용 시 ABI access violation이 있어 DataJson/ProjectDataRoot/AnimationActionDocument를 현재 source로 다시 컴파일한 뒤 통과했다.
3. 실제 P78 candidate: `_publication_candidate -> validate_document -> validate_publishable -> project_encounter/project_presentation` 성공. 네 행의 원래 startMs, 각 visual1개, count1, lifetime0, interval0을 확인했다. `projected-p78-pursuits.json`에 보존했다.
4. 변경6개 C++ TU의 격리 Debug 최소 컴파일 성공: CompositionDocument, ActionWorkbench, LogicPreview, LogicRuntime, BossSimulation, SupportSurface tests. Product EXE를 링크·실행하지 않았다.
5. `git diff --check` 성공. 로그와 격리 obj/EXE는 Git 제외 `out/KoukuCardTriggers20260922/`에 있다.

## G03. 통합에서 확인할 검증

기존 `ServerGameplayContractTests_KoukuSupportSurface.cpp`에 실제 CGameRoom/CombatObject 검사를 추가했다.1ms Trigger birth, +X 전방, full-life CONTACT만end0, 짧은1000ms 창 보존,601초 뒤 접촉 피해1건과despawn을 검증한다. 이 하위 작업에서는 테스트 TU 컴파일까지 끝났고 실행 결과는 통합 빌드/RESULT에 기록해야 한다.

publish와 설치된 Product build, 실행 중 Server의 새 코드·데이터 로드는 통합 작업 범위다. Client/UI 자율 실행·조작·화면 캡처는 하지 않았다. Trigger 타이밍·추적·발생 위치와 최종 화면은 사용자가 확인한다.

## G04. Product 빌드 후 SupportSurface 회귀 확인

통합 Product Debug 빌드 뒤 실제 Server SupportSurface 실행에서 이번에 추가한1ms Trigger, 방향,601초 뒤 접촉 피해·despawn 검사들은 모두PASS였다. 기존 네 assertion 실패는 다음 fixture 불일치로 확인해 테스트만 수정했다.

-9월17일 `dfec8fe8bd`부터 pursuit는 `combatpresentation.kouku.pursuit.started` birth pulse를 이미 큐에 넣는다. finite expiry와 swept contact 테스트 두 곳만 전체 lifecycle event 개수를 terminal burst 개수와 같다고 가정했다. 다른 distance/finite 테스트처럼 birth와terminal을 구별하되, 전체24=12+12 또는2=1+1의 정확한 개수도 함께 검증한다.
-9월19일 `34ec277efc`에서 Showtime tracker 시작 위치를 각 플레이어 위치에서 authored arena center로 바꿨다.9월16일 fixture가 이전 시작점6/10에서의 X6.1/10.2를 그대로 기대했다. 현재 계약대로 두 tracker의 중앙 출생을 확인하고 각각0.1/0.2m의 이동량·자기 target까지 거리 감소를 검증한다. 같은 tick 재호출은 두 tracker의 X/Z와 고정/추적 object 수가 모두 그대로인지 검사한다.

위 네 assertion 교정에는 제품 코드를 수정하지 않았다. 교정 뒤 실행과 isolated Server data root 설정 결과는 통합 로그를 따른다.
