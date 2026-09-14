# 알비온 nav 랜덤 장판과 Logic 실행 연결 구현 계획

## G00. 현재 Logic과 실제 소비자

현재 기준은 `codex/sequence-capture-focus`, HEAD `29ad2df5`다. 다른 작업의 미커밋 변경을 보존한다. P39의 logic53(랜덤 플레이어 탐색), logic54(하늘 순간이동), summon4(추적 파란 장판)는 이름만 있고 실행 종류가 없다. 기존 ALBION_BLUE_CIRCLE logic44는 구현됐지만 패턴에 배치되지 않았다. P34 logic58(괴기스러운 인형 랜덤 스폰)도 이름만 있으며, 현재 큰 인형은 World 트랙의 고정 위치를 사용한다. 마리오는 이번 요청에서 감사하고 알비온 장판의 실행 연결을 변경한다.

## G01. 기존 CombatObject 생성 경로 확장

`BOSS_PATTERN_MECHANIC_TRIGGER`와 Composition Logic에 `arenaRandomCount`, `arenaRandomRadiusM`, `arenaHeightToleranceM`, `arenaMinimumSpacingM`, `randomPlayerOnly`를 추가한다. 기존 필드가 없는 문서는 종전 플레이어별 생성 의미를 유지한다. 신규 설정은 nav 랜덤 5개와 살아 있는 준비 완료 플레이어 중 무작위로 선택한 한 명의 생성 시점 위치 1개다. 발탄의 현행 도끼와 같이 위치를 생성 순간 고정하며 계속 따라다니는 이동은 추가하지 않는다.

`Commit_KoukuMechanicTriggers`는 기존 `Resolve_ArenaRandomVolleyOrigins`와 `CombatObjectRuntime`의 stage/commit을 사용한다. 이펙트 전용 객체에는 damage hit이 없으므로 공용 resolver에 명시 간격 입력을 추가하고 발탄의 기존 damage 직경 기반 기본값을 보존한다. 위치·같은 지면·개수 검증에 실패하면 전체 새 volley를 취소하고 기존 객체와 ID를 유지한다. 한 명의 대상 선택도 Server가 소유하며 마리오 참가자는 본 무대의 대상으로 삼지 않는다.

## G02. 저장·게시·실제 Effect 소비

Client Composition H/CPP의 typed parse/validate/serialize와 Workbench Logic 입력, Python projector, GameplayBalance publisher 및 Server bootstrap reader를 같은 필드로 연결한다. 기존 `effect.kouku.albion.bluecircle.warning.impact.runtime`을 BossCatalog의 combat visual로 사용한다. 원본의 2초 예고 뒤 폭발과 7초 객체 수명을 유지하며 Shared/Client presentation의 새 경로는 만들지 않는다.

P39의 실제 파란 장판 시각에서 logic44를 실행하도록 연결하고, 단일 보스 부착 합본 Effect의 중복 재생을 제거한다. 다른 animation·부채꼴·십자 연출과 미정의 Logic은 보존한다. 현재 Client가 실행 중이므로 정본 변경안을 먼저 out에 준비하고, 사용자 편집이 저장·종료된 것을 확인한 뒤 최신 파일에 대상 항목만 반영한다. 파일 전체 외부 교체로 미저장 draft나 freshness 검사를 우회하지 않는다.

## G03. 검증과 빌드

기존 projector 회귀와 Server support-surface 계약에서 5+1 개수, 대상 선택, nav 높이·간격, 생성 후 고정, 만료·취소·원자 rollback을 검사한다. JSON/XML parse, scoped publisher의 실제 P39 편입과 visual ID, `git diff --check`를 확인한다. 새 C++ 파일이 없으므로 project/filter 등록은 추가하지 않는다.

공유 출력이 사용 중이면 out의 격리 컴파일로 수정 TU를 먼저 검사한다. 사용자 편집 저장·Client/Server 종료 뒤 정상 증분 `Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product`와 필요한 Kouku domain 게시를 수행한다. Client/UI 실행·캡처와 최종 화면 판정은 사용자가 수행하며 RESULT에 소스 반영·게시·바이너리·사용자 화면 상태를 구분한다.

## G04. Sequencer Reset 옆 Server Play Pattern

Action Workbench의 Pattern/Parent와 Bundle transport에서 Reset 바로 오른쪽에 `Play Pattern`을 추가한다. 현재 왼쪽 Play는 local preview이므로 일반 transport의 해당 표시를 `Play Preview`로 명확히 한다. 별도 Sequence workspace는 기존 Sequence/전투 진입 계약을 유지하고 gameplay Pattern 요청을 만들지 않는다.

새 공통 요청 함수는 미저장 변경, 진행 중 Publish, stale source 및 잘못된 선택을 거부하고 현재 저장 문서와 draft의 동일성을 다시 확인한다. 기존 BossTool의 read-only Product inventory loader로 게시 revision과 선택 Pattern/Bundle의 실제 실행 가능 상태를 확인한 후에만 기존 pending Pattern/Bundle typed request를 기록한다. MainApp의 기존 소비자가 BossTool과 Server audition service로 전달하므로 Collider/Logic은 Server에서 실행된다. 버튼을 눌렀다고 자동 Save/Publish하거나 local collider simulation을 추가하지 않는다.

기존 Complete Play (Server)도 같은 요청 함수로 통일하고, 실패 이유는 Workbench 상태에 남긴다. 새 버튼 tooltip은 현재 실행 대상의 표시명과 stable Pattern/Bundle ID를 보여 준다. Resources 선택은 Append 대상만 바꾸는 기존 의미를 유지하고 현재 timeline owner와 혼동하지 않도록 설명한다. 새 C++ 파일과 프로젝트 등록은 없다. 변경 Workbench TU를 MSVC14.44/SDK10.0.26100.0의 격리 out에 컴파일하고 요청 함수의 selection/revision/실패 보존과 Reset 인접 UI 연결을 좁게 검증한다. 실제 Client/UI 실행과 최종 사용자 화면 검증은 수행하지 않는다.

## G05. 여러 패턴의 이펙트 게시본 로드 실패

게시550의 P39.presentation.2/.3/.4는 WORLD anchor인데 worldId가 비어 있다. Product reader는 해당 행을 거절하고 전체 replacement를 commit하지 않으므로 정상인 P34 오망성과 P38 무지개댄스까지 재생할 Product가 없어질 수 있다. Client 저작 validator와 Python publisher가 같은 WORLD 필수 ID 계약을 검사하도록 수정한다. 오류를 정상 pivot으로 치환하거나 전체 로드 검증을 제거하지 않는다.

사용자가 World는 캐릭터에 부착하지 않는 맵 절대 좌표라고 명시했다. UI의 World/Fixed position은 저장 MAP이며, 특정 World Object 부착인 저장 WORLD와 구분한다. 사용자가 직접 저장한 revision552의 P39 세 이펙트는 MAP·followBoss=false·절대위치(-2.157323,1.317626,942.926086)다. 이 저장 위치를 보존한 최신 후보553에서 기존 파란 장판 직접 행만 제거하고 두 부채꼴은 그대로 둔다. 정본과 실행 중 draft에는 적용하지 않는다. 실제 codec과 publisher에서 원래 잘못된 행의 거부, 정상 Boss/Map/World 행의 허용, 실패 시 문서 보존을 검증한다. 작은 오망성의 투명 노이즈 문제는 해당 asset과 설치 shader를 별도로 대조하며, 전체 Product 로드 결함과 구분한다.

## G06. 쇼타임 기본 무기 교체

기존 양손 총 World object를 유지하고 두 총의 정확한 objectId에만 기본 무기 숨김을 연결한다.
WorldSequencePlayer는 실제 본을 제공한 body model과 visible 총 clone을 해당 object 재생 수명에
묶어 등록한다. NpcPresentationAssetService가 같은 body에 살아 있는 visible 총이 하나라도
있는지 조회한다. CNpc와 Model View 무기 part는 렌더 시 그 결과를 사용하므로 왼손 종료가
오른손 총의 숨김을 풀지 않으며 실패·Stop·pool 반환·Clear 때 등록 해제로 원래 무기가 복구된다.
NPC/Preview actor별 실제 model identity를 사용하고 Pattern 이름·clip·timer로 추측하지 않는다.
새 JSON·packet·런타임 경로는 추가하지 않는다. 변경 TU는 out에 격리 컴파일하고 등록·양손·종료·
다른 owner 재사용을 좁게 확인한다. 새 C++ 파일이 없으므로 project/filter 등록은 없다.
