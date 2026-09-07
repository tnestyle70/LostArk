# 2026-09-07 쿠크 룰렛 임시 보행면 구현 계획

## G00. 목표와 현재 기준점

사용자는 룰렛이 나타났을 때 캐릭터가 윗면을 걷고, 룰렛이 사라지면 원래 지면으로 돌아가기를 요청했다. 서버가 지면 높이를 확정하고 기존 player snapshot으로 Client에 전달한다. Client mesh collision이나 시각 offset으로 플레이어 정답을 바꾸지 않는다.

현재 `CServerNavigation`은 4m 기본 격자와 정적 `m_Heights`를 소비한다. 동적 조건은 BlockCounts/VoidCounts만 변경하며 높이 overlay는 없다. `Update_Players`는 이동 전후에 `Resolve_TraversalStep`을 호출하지만 정지한 플레이어는 지면 높이를 다시 읽지 않는다. `.navregions`는 고정된 Mario 영역 세 개이며 이번 기능의 동적 바닥으로 사용하지 않는다. 쿠크 기본 최대 보행 높이차는 1m다.

룰렛은 `world.object.kouku.roulette → world.sequence.instance.8 → MAP_PLACEMENT 40`이다. template `sequence.LV_LUT_MIDNIGHTC_ED.5`는 33,773ms이며 위치와 크기는 고정, 회전은 Y축뿐이고 마지막 키는 숨김이다. placement는 위치 `(-0.319,1.9,737.531)`, scale `(4,4,4)`다. WORLD cue는 Boss Spawn 기준으로 offset `(0,0.58,0)`을 사용한다. 실제 설치 모델의 주평면 raw Y는 약 2.6313, 원주는 약 250.1~250.4이며 Map model pre-scale 0.01을 적용한 로컬 보행면은 높이 0.026313m, 반지름 2.5m다. 장식의 최고점을 지면으로 오인하지 않는다.

기준 브랜치는 `codex/kouku-gate-pattern-bundles`이며 기존 사용자·다른 기능의 미커밋 변경을 보존한다. 초기 조사 시 WorldSequence revision396이었고 구현 승인 뒤 Composition은 revision93으로 갱신됐다. 사용자가 편집 중이므로 지금 source는 수정하지 않는다. root가 저장·종료를 확인한 뒤 bytes를 다시 읽고 백업하며 compare-and-swap으로 해당 instance의 optional 필드와 WorldSequence revision만 병합한다. Composition 및 기본 Nav 원본은 수정하지 않는다.

## G01. 저장과 저작 UI

`WorldSequenceDocument.h/.cpp`의 기존 instance에 optional `walkableSurface`를 추가한다. 값은 `radiusM`, `localHeightM` 두 필드다. 단일 MAP_PLACEMENT binding, WORLD anchor, STOP completion, 정지한 위치·크기와 Y축 회전만 지원한다. 평면은 target local coordinate의 미터 단위이며 placement scale은 publisher가 적용한다. 잘못된 값은 load/save의 기존 검증에서 이유를 보존해 거부한다. 필드가 없으면 기존 동작을 유지한다.

`WorldObjectTool.cpp`의 실제 Motion Detail에 Walkable Surface toggle, Radius, Local Height를 둔다. Tool 저장은 기존 document와 transactional Save를 사용한다. 지원하지 않는 binding에는 생성 기능을 제공하지 않는다. `Publish-MapAuthoring.ps1`은 같은 optional 필드와 평면 조건을 검증하며 원본 내용을 runtime WorldSequence에 보존한다.

## G02. Product 투영과 Server 정의

`project_kouku_saydon_composition.py`는 WorldSequence 원본의 지원 면을 읽고, 기존 `_load_region_world`로 동일 placement/anchor/scale을 resolve한다. 원판의 중심, 반지름, 표면 Y와 visible 구간을 기존 WORLD cue의 서버 고정 tick 기준 시간으로 투영한다. native animation, 움직이는 위치·scale, 기울어진 면은 조용히 근사하지 않고 거부한다.

`GameplayCatalog.h/.cpp`와 `Publish-GameplayBalance.ps1`은 기존 BOSS_PATTERN_WORLD_SEQUENCE에 optional support 정의를 연결한다. bootstrap에는 별도 typed 부가 행을 사용하여 기존 WORLD 행·Shared protocol 형식을 보존한다. 서버는 mesh 경로나 Client object를 읽지 않는다.

## G03. Navigation의 유효 지면 높이

`ServerNavigation.h/.cpp`에 원형 support 목록과 transactional 교체 함수를 둔다. 소유권은 room이 발급한 run/member/pattern/cue 키다. 기본 walkability가 없거나 동적 blocker가 막은 셀은 여전히 통과할 수 없다. 유효한 면 중 기본 바닥보다 높은 면을 지지 높이로 선택한다.

`Sample_Position`, `Cell_ToPoint`, `Is_CellTraversalAllowed`, `Has_LineOfSight`, `Resolve_TraversalStep`이 같은 effective height를 사용한다. 원의 실제 경계는 XZ로 검사하며, 같은 셀 내부에서도 높은 원판을 가로지르는 이동이 기존 최대 높이차 정책을 우회하지 못하게 한다. 기본 baked height는 변경하지 않는다. support 목록 변화는 Nav revision을 증가시키고 기존 path invalidation을 호출한다.

## G04. Room 수명과 플레이어

`GameRoom.h/.cpp`에서 기존 pinned Pattern과 WORLD cue 시간표를 이동 전에 평가한다. 패턴 시작 준비도 이 단계로 옮겨 첫 tick부터 보행면과 cue가 같은 기준을 사용한다. 기존 WORLD cue run/member 소유권, 시작 tick, 수명, visibility 구간을 재사용하며 새로운 socket 명령을 만들지 않는다.

정상 지면 위에 있는 플레이어는 정지 중에도 support 높이를 적용받는다. 패턴 bind, 붙잡힘, 낙하, 특수 이동의 Y 권위는 유지한다. 정상 종료, cue 수명 종료, owner STOP, abort, room reset에서 면을 제거하고 원래 바닥으로 재지지한다. 기존 snapshot이 Y를 전달하므로 Client 렌더 경로는 추가하지 않는다.

## G05. 검증과 완료 경계

기존 테스트 파일에 현재 기능을 검증하는 좁은 사례만 추가한다. 원형 경계, 같은 셀 경계 통과, 기존 step height 거부, Blocked/NO_SURFACE 보존, A*/LOS/이동 높이, 활성·만료·owner 정리와 정지 플레이어 재지지를 확인한다. 새 harness 프로젝트나 별도 runtime을 만들지 않는다.

변경 C++의 최소 컴파일, JSON/XML parse, 변경 도메인 publisher 및 `git diff --check`를 확인한다. 새 C++ 파일이 없으므로 project/filter 신규 등록은 없다. 빌드와 배포는 root와 조율하고 Client/Server 종료를 먼저 확인한다. Client/UI를 실행하거나 조작·캡처하지 않는다. 최종 시각 결과는 사용자가 `F1 → Saved Patterns → 룰렛 Complete Play`에서 확인한다. 실제 수행한 자동 검증과 남은 화면 확인은 대응 RESULT에 기록한다.
