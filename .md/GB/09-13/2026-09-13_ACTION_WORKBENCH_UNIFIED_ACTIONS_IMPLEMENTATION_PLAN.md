# Action Workbench 통합 Composition Actions 구현 계획서

작성일: 2026-09-13. 사용자의 통합 구현 요청을 반영한다. 기존 `codex/kouku-donut-ball-motion`의 다른 세션 미커밋 변경을 보존하며 자동 stage/commit하지 않는다. 캐릭터의 이전 설계는 `09-11/2026-09-11_CHARACTER_ACTION_COMPOSITION_AND_RESPONSIVENESS_IMPLEMENTATION_PLAN.md` G01~G03이며, 이번 통합은 보스·오브젝트·연출까지 같은 창 소유자로 연결하는 작업이다.

## G00. 현재 정본과 호출자

`CSequencerTool`은 `ICompositionWorkbenchSession`의 창 배치와 가시성을 소유한다. Valtan과 Kouku 문서는 각 세션에 있고 Sequence는 같은 Kouku 편집기의 독립 문서를 사용한다. `CWorldObjectTool`은 World Sequence 문서와 원자 저장·연결 Composition 갱신을 소유한다. Character는 `PlayerSkills`와 skillbindings, animevents, HitShapes를 기존 제품 reader가 소비한다.

현재 Character 목록과 제품 스킬 통합 편집은 구현되지 않았다. `CEffectAuthoringSequencer::Select_CharacterSkill`은 애니메이션 중심 preview이며 Save Sequence는 제품 스킬 저장이 아니다. Server의 기존 player hit는 한 overlap에서 피해·카운터·무력화를 함께 적용한다. 이 소비자를 실제로 분리해야 세 가지 collider 저작이 의미를 갖는다.

## G01. 공통 창과 대상 선택

`Composition Patterns`를 `Composition Actions`로 바꾸고 최상단에 Boss / Character / Object / Sequence를 제공한다. Boss에는 기존 Valtan·Kouku 관문 트리를 그대로 연결한다. 창 ID는 유지해 기존 배치를 보존한다. 대상 전환은 프레임 경계에서 적용하고 이전 preview를 종료하되 문서·미저장 초안·선택은 세션에 보존한다.

수정 파일은 `CompositionWorkbenchSession.h`, `SequencerTool.h/.cpp`, `MainApp.h/.cpp`다. 공통 shell이 같은 Resources / Sequencer / Detail / Preview / toolbar를 그리고 기존 MainApp이 실제 재생 owner를 선택한다. Object와 Sequence의 이전 진입점은 통합 창의 해당 대상으로 연결한다. 별도 preview runtime은 추가하지 않는다.

## G02. Character의 실제 skill과 parent timeline

신규 `CharacterActionWorkbench.h/.cpp`가 실제 catalog의 여섯 class와 LMB, SPACE, ALT_V 및 ACTIVE slot을 나열한다. Parent 선택은 clip·Effect·Sound·Collider/Result의 연결을 같은 action 시간축에서 보여 준다. 실제 skill ID와 Server combo stage를 사용하며 빠진 binding은 해당 항목 오류로 보존한다.

`CharacterPreviewPanel`, `EffectAuthoringSequencer`, `AnimationSkillBindingDocument`의 기존 모델·재생·저장 owner를 확장한다. 편집은 실제 제품 입력에 저장하고 Effect sequence 저장을 제품 스킬 저장으로 위장하지 않는다. 지원하는 cue 편집은 기존 reader·writer까지 연결하고 외부 변경 충돌 시 draft와 파일을 보존한다. 원본 Effect의 표시 geometry와 Server 판정 collider를 구분하고 판정 정본은 HitShapes로 연결한다.

## G03. 독립 Collider → Logic → Result

기존 여섯 class HitShapes의 실제 caster/projectile hit를 DAMAGE / STAGGER / COUNTER 세 행으로 이관한다. 각 행은 stable colliderId·logicId·resultId와 Result kind, 독립 shape·시각을 가진다. AREA_OVERLAP 논리는 해당 Result로 연결한다. 기존 fallback hit도 기존 실행 위치·범위·시각을 명시 데이터로 옮기고 원작 복원으로 부르지 않는다.

신규 `CharacterActionCombatDocument.h/.cpp`가 읽기·행 편집·원자 저장을 소유한다. Gameplay publisher와 Server catalog/player skill 소비자를 같이 확장한다. damage row만 HP 예산을 분배하고 counter/stagger row는 각 수치만 적용하여 피해량 3배 또는 1/3 회귀를 막는다. 수치 정본은 PlayerSkills로 유지하고 존재하지 않던 카운터를 슬롯 이름만으로 추가하지 않는다. 중복 ID·잘못된 연결·종류·상한을 거부한다.

## G04. Object와 Sequence 재사용

`WorldObjectTool.h/.cpp`가 기존 document와 timeline을 유지한 채 공통 세션 인터페이스를 구현한다. Parent Object에서 연결 Motion의 Transform / Animation / Effect 행을 펼치고 기존 Save·Reload·Play·편집·연결 문서 충돌 처리를 재사용한다.

Sequence는 기존 독립 Composition 문서를 통합 shell에 연결한다. 현재 Animation, Effect, Sound, Camera, World, Light, Scene Profile 등의 lane과 Complete Play 및 전투 handoff 소비를 유지한다. 지원되지 않는 Server Logic을 연출 preview에서 실행한 것으로 표시하지 않는다. 새로운 도구나 두 번째 데이터 복사본은 만들지 않는다.

## G05. 등록과 검증

신규 Client H/CPP는 기존 Tool 필터의 `.vcxproj`와 `.vcxproj.filters`에 필요한 항목만 등록한다. 변경 JSON/XML parse, 문서 저장·재로드·외부 충돌 보존, publisher와 Server의 세 channel 독립 처리 및 기존 피해 합계 보존을 해당 focused 검사로 확인한다. 사용자가 실행 중 편집을 계속하므로 기본 Product 출력 대신 `out`의 별도 Debug 컴파일·링크와 `git diff --check`를 수행한다. 제품 교체·전체 publish는 편집 종료 뒤 source baseline을 재확인하여 적용한다.

현재 Client 45900 / Server 48036이 실행 중이다. 사용자가 편집 저장 후 직접 종료할 때까지 제품 EXE 교체를 하지 않는다. Client/UI 자율 실행·조작·캡처 없이 빌드·구조화된 검증과 실행 준비까지만 수행한다. 실제 완료·실행한 검사·남은 사용자 화면 확인은 대응 RESULT에 나눈다.

## G06. 사용자 재빌드 후 bootstrap 34 적용

사용자가 새 Client/Server를 직접 빌드한 뒤 `Gameplay bootstrap header is invalid`로 시작이 실패했다.
현재 실행 프로세스는 없고 기본 bootstrap은 33, 새 코드 요구 버전은 34다. 새 연출 요청의 코드·데이터
반영을 잠시 보류하고 이 오류를 우선 처리한다. 현재 Composition과 projection, 기존 bootstrap을 먼저
보존한 뒤 정식 Kouku projector로 두 Product 문서를 동기화하고 Gameplay publisher의 Publish를 실행한다.
사용자가 빌드한 EXE를 그대로 사용하여 Server의 정상 generation 초기화와 listener를 확인하고, 이번
명시 요청에 따라 Client 실행만 수행한다. UI 조작·화면 캡처는 하지 않는다. 헤더 숫자만 바꾸거나 버전
검사를 완화하지 않는다.

## G07. 사용자 수정 요청: Effect V1/V2 독립 창 복구

사용자는 Effect Tool을 Action Workbench에서 분리하고 `Open Effect Tool V1`,
`Open Effect Tool V2`를 각각 기존 독립 창으로 복구하는 작업을 연출 확장보다 먼저 요청했다.
이번 변경은 이 분리만 구현하며 연출 확장은 조사 상태로 둔다.

현재 두 Effect Tool의 독립 `Render()`와 기존 창 ID는 남아 있지만 MainApp의 Render 호출이 빠져 있고,
Open/visibility가 Sequencer의 Effect target으로 우회한다. `MainApp.cpp`에서 두 버튼·독립 Render와
visibility/input owner를 복구하고 `EnsureDebugTool(EFFECT/EFFECT_V2)`는 각 Effect owner만 연다.
Action의 Effect resource 선택은 기존 typed callback으로 독립 도구를 여는 기능을 유지한다.

`SequencerTool.h/.cpp`와 `CompositionWorkbenchSession.h`에서 Effect target/session/selector를 제거한다.
두 Effect Tool의 사용하지 않는 Composition pane adapter도 제거하되 native 창·resource 상세·Attach/
Group·저장·draft와 기존 Update 경로는 유지한다. Boss/Character/Object/Sequence 통합은 유지한다.

현재 실행 중인 EXE와 Data 저장본은 변경하지 않는다. 인코딩을 보존한 소스 수정, 실제 라우팅과
독립 visibility/draft 유지 검사, 영향받는 Client C++의 `out` 격리 컴파일·링크를 먼저 마친다.
이후 사용자가 편집을 저장하고 종료한 상태에서 정상 Product Build로 적용한다. 이번 UI/C++ 수정에는
Gameplay/Map/Effect publisher 실행이나 authoring 데이터 변환이 필요 없다.
