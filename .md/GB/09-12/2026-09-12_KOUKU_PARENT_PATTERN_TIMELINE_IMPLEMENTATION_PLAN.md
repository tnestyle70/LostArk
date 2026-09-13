# Kouku Parent 타임라인과 Pattern 참조 행 확장

## G00 목표와 현재 상태

Parent에서 기존 Animation, Logic, Effect, Collider, World, Scene Profile 행을 편집하고 Pattern 행에 독립 공격을 배치한다. Parent의 공통 판정 수명은 자식 공격 종료와 독립이며, 정해진 구간 끝에서 자식의 남은 공격은 취소한다. 마리오와 쇼타임에 같은 편집 구조를 사용한다.

현재 Parent는 folderId/gateId/displayName만 소유한다. Bundle은 서로 다른 보스의 동시 재생이며 같은 보스를 중복할 수 없다. 서버 보스는 한 Pattern/action clock을 사용하고 완료 시 LogicLedger를 정리한다. 현재 브랜치 codex/sequencer-camera-load-performance의 대규모 미커밋 변경을 보존한다. 자동 stage/commit과 Client/UI 실행·캡처는 하지 않는다.

## G01 저장 계약과 Composition Document

기존 Parent에 optional timelinePatternId를 추가한다. 이것은 같은 폴더·관문에 속한 기존 Pattern 문서를 실행 타임라인으로 가리킨다. 기존 폴더는 그대로 읽힌다. 실행 타임라인은 동일 모델·대상 보스의 Pattern occurrence를 stable pattern ID로 참조하며 원본 세부 행을 저작 문서에 복사하지 않는다.

Pattern의 optional durationMs는 명시 전체 시간이며 없는 문서는 기존 Stage 합계를 사용한다. patternOccurrences는 occurrenceId, patternId, startMs, durationMs, repeat를 저장한다. nextPatternOccurrenceOrdinal이 새 배치 ID를 발급한다. 원본 내부 편집은 공유 정의에 반영하고 배치 시간·반복 수정은 그 occurrence만 변경한다. 잘못된 참조, 순환·중첩, 모델/관문/대상 불일치, 본체 애니메이션 겹침은 실패 이유를 유지한다.

수정 파일은 기존 KoukuSaydonCompositionDocument.h/.cpp와 ActionWorkbench.h/.cpp다. 별도 모델 런타임이나 새 C++ 파일은 추가하지 않는다. parse/validate/Serialize와 CAS Save를 확장하고 손상된 항목 때문에 다른 저장 내용을 지우지 않는다.

## G02 실행 시간과 게시

이번 실행 계약은 저작한 고정 시간 배치와 반복이다. Preview와 publisher가 같은 참조를 하나의 기존 Pattern 실행 시간으로 전개한다. 자식 Stage, animation, logic, world, scene, presentation과 서로를 가리키는 ID를 배치별로 재매핑한다. 자식의 종료가 Parent 공통 행을 제거하지 않는다. 재생 공백은 파생 실행 데이터에서만 idle로 채운다.

자식이 자연 종료하기 전에 배치 창이 닫히면 미발생 이벤트를 제거하고 진행 중 window를 판정 없이 취소한다. 이를 위한 cancelAtEnd를 실제 Gameplay publisher/bootstrap/Server Logic 소비까지 연결한다. 자식의 댄스 HUD도 자식 구간 밖으로 남지 않는다. Pattern 전체를 조기에 끝내는 Logic, 동적 FOLLOWUP_PATTERN, Pattern 단위 강제 초기화·BossMotion 등 현재 고정 배치로 의미를 보존할 수 없는 자식은 명시적으로 실행 불가 사유를 표시한다. 이를 조용히 무시하거나 Parent 전체 동작으로 승격하지 않는다.

Python projector의 optional schema와 dependency closure, saved inventory, 생성 Pattern/Presentation을 함께 갱신한다. Parent에 고정 시간 참조를 배치하는 것과 상황별 무작위 공격 선택은 구분한다. 후자는 이번에 별도 AI 경로로 추가하지 않는다.

전개한 Parent Product의 fixedTimeline은 PATTERNFIXEDTIMELINE bootstrap 행으로 Server에 전달한다. Stage마다 밀리초를 tick으로 올림하여 합산하지 않고 Parent 시작부터 누적 밀리초의 경계를 직접 비교한다. 30Hz에서 같은 tick으로 압축되는 stage는 사전에 거부한다.

## G03 편집기와 실제 소비자

Parent 선택에서 타임라인 생성·열기로 기존 행 편집기를 사용한다. Pattern 행의 Append, 선택, 시작/길이/반복 편집, 삭제, 원본 패턴 열기와 Parent 복귀를 연결한다. Save/Reload, Preview/Scrub, Publish 및 F1 선택은 동일 timelinePatternId를 사용한다. Preview는 미저장 초안의 전개 실패를 이전 저장본으로 우회하지 않는다.

### G03-01. Parent 선택과 Append 진입 보완

사용자가 Parent 선택 뒤 빈 sequencer와 `Publish All Patterns` 옆 Append 버튼 부재를 보고했다. 기존 코드는 timelinePatternId가 없는 Parent를 FOLDER 선택으로 유지하면서 Render_Timeline에서 즉시 반환하고, 생성·Append는 Details 안에서만 제공했다. 기본 Debug EXE에도 해당 Details 버튼 문자열이 존재하므로 단순한 구버전 EXE 문제로 분류하지 않는다.

`KoukuSaydonActionWorkbench.cpp`의 Render_PatternsAndResources에서 Publish 오른쪽에 `Append Pattern`을 표시한다. 유효한 Parent 또는 Parent backing Pattern을 선택하면 활성화하며, 같은 관문의 독립 Pattern을 고르는 팝업으로 연결한다. 이미 타임라인이 있으면 같은 actor·target boss를 필터링한다. 없는 경우에는 선택한 자식의 actor로 15000ms backing timeline과 첫 Pattern occurrence를 한 candidate에 stage한 뒤 기존 Try_ExpandPatternDocument와 Commit_Candidate를 통과시킨다. 실패 시 folder, Pattern, ordinal과 기존 draft를 모두 보존한다.

기존 Append_PatternBox의 ownerId는 backing Pattern ID와 Parent folder ID를 모두 받는다. Create_ParentTimeline과 공용 Stage_ParentTimeline 준비 함수를 사용해 중복 생성 경로를 만들지 않는다. 성공하면 backing Pattern과 새 박스를 선택하므로 기존 Pattern lane/상세 편집기를 즉시 소비한다. 선택만으로 문서를 변경하지 않으며, 미생성 Parent의 Render_Timeline에는 생성 안내와 `Create Parent Timeline` 버튼을 표시한다. 기존 타임라인 선택, 자식 원본 편집·Parent 복귀·저장·게시 계약은 유지한다.

공개 헤더에는 기존 Append_PatternBox 계약 설명과 private Render_AppendPatternButton 선언만 추가한다. 새 C++ 파일이나 프로젝트 등록은 없다. 기존 Workbench CPU probe를 out에서 확장하여 미생성 Parent의 첫 append와 실패 rollback, 이미 생성된 Parent 재열기, 자식 원본 선택, Save/Reload를 검증한다. Client UI는 실행하지 않고 최종 Product 빌드와 사용자 버튼·화면 확인을 구분한다.

### G03-02. Publish 중 Sequencer 입력과 저장 경계

`Publish All Patterns` 직후 박스 선택과 타임라인 이동이 멈춘다는 보고를 조사했다. 실제 publisher는 아직 GameplayBalance의 Composition 검증을 진행 중이며, `Render_Timeline`의 `canInteract = !publishing && !WantTextInput`이 선택, ruler scrub, 박스 이동·트림과 다중 선택을 모두 차단한다. 프로세스 완료를 관찰하면 handle을 해제하는 기존 경로와 별개의 입력 제한이다.

기존 `Commit_Candidate`는 검증 후 메모리 초안만 교체하고, publisher는 저장된 원본을 읽는다. `KoukuSaydonActionWorkbench.cpp`에서 publish 중에도 타임라인 선택·scrub·박스 편집, Parent 생성·Append와 초안 lifetime 편집을 허용한다. 원본을 바꾸는 Save, Reload 및 중복 Publish/Server Complete Play 제한은 유지한다. 진행 중에는 타임라인에 경과 시간과 저장 대기 상태를 따로 표시하여 편집 상태 메시지가 publisher 진행 상태를 숨기지 않게 한다. 성공·실패 뒤에도 초안, 선택, 커서와 dirty 상태를 유지하고, 성공 안내에는 게시 도중의 편집을 다시 Save/Publish해야 한다는 사실을 포함한다.

새 제품 C++ 파일과 저장 schema는 추가하지 않는다. 기존 Workbench CPU probe에서 UI 없는 자식 프로세스의 실행/성공/실패 상태를 사용해 초안 편집, 게시 중 Save 거부와 원본 보존, 완료 후 dirty/선택/커서 보존과 저장 재개를 확인한다. 실제 UI 입력 경로의 잠금 해제는 소스 분기와 컴파일로 확인하며 Client를 실행하거나 화면 판정을 대신하지 않는다.

### G03-03. 게시 실행 단위 공통 입력 재사용과 모델 선택 읽기

사용자가 G05 조사에서 확인한 6분 게시 병목 수정을 요청했다. 범위는 `Publish All Patterns` 내부이며 게임 렌더링 런타임을 변경하지 않는다. 모델 이름이나 한국어 표시명에 특례를 넣지 않고, 게시 과정이 읽는 모든 모델과 공통 JSON에 동일한 수명·변경 감지 규칙을 적용한다.

`project_kouku_saydon_composition.py`의 한 게시 실행 안에서 공통 JSON, catalog join, 모델, pose와 동일 변환 결과를 재사용한다. cache는 호출 세션에만 존재하고 다른 게시나 root로 새지 않는다. 실제 입력의 변경을 읽기 및 최종 출력 준비 경계에서 검사한다. 개별 Pattern/Bundle 오류 격리와 dependency closure 검증, 중복 ID와 잘못된 시간·참조 거부를 보존한다. 후보 검증을 생략하는 무조건 Skip 스위치로 단축하지 않는다. GameplayBalance 후단의 검증도 같은 최적화된 projector를 사용한다.

기존 공통 `verify_dimensionmaster_summon_bind_pose.py::read_wmodel`에 실제 publisher가 필요한 선택 읽기를 연결한다. 기본 호출은 기존 전체 geometry/animation decode를 유지한다. source trim 검사에는 skeleton과 animation metadata, 본 판정 계산에는 필요한 clip key만 decode한다. 선택하지 않은 vertex/key의 Python 객체를 만들지 않되 section/span, bone-reference와 duplicate 검사는 유지한다. 생략된 geometry/채널은 `None`으로 구별하고 그 값이 필요한 sampling 함수는 명시적으로 거부한다. 기존 `CModel/CMesh` 런타임, WModel 저장 형식과 asset ID는 변경하지 않는다.

원본 projector/reader는 `out/KoukuPublishPerformance20260912/before`에 보존했다. 현재 입력을 고정한 격리 snapshot에서 이전 코드와 수정 코드의 `prepare_publication/projected_outputs`를 각각 실행하여 시간, JSON/model 읽기 횟수와 최종 출력 bytes 동일성을 비교한다. 기존 projector·model reader 테스트로 기본 전체 읽기, 선택 읽기, 손상 입력, source 변경과 rollback 경계를 확인한다. 원본 Product 파일을 성능 측정용으로 교체하지 않고 실제 실행한 검증만 RESULT에 기록한다. 변경은 기존 Python 파일과 관련 테스트이므로 C++ 프로젝트 등록은 없다.

같은 게시 체인의 `Publish-WorldGameplay.ps1::Read-ProjectJson`도 한 실행 안에서 같은 Area 시퀀스를 trigger마다 다시 parse한다. 이 읽기를 실행 단위로 공유하고 원본 text 및 파일 version을 보관한다. 반복 읽기는 변경된 파일을 거부하며, 출력 준비·교체 전후에는 원본 text를 다시 대조한다. 캐시를 사용한 결과가 새 입력과 섞이면 기존 publish rollback으로 되돌린다. 공통 parser이며 특정 World/model/문자열에 캐시 예외를 만들지 않는다. out에 격리한 기존 publisher와 수정 publisher의 모든 World 출력 bytes 및 실패 rollback을 비교한다.

`BuildDomains.json`의 Kouku Product와 Gameplay Balance tool 목록에 실제 Python import인 공통 WModel reader를 포함하고, Gameplay Balance에도 light validator를 포함한다. 도구 변경이 기존 receipt의 재사용을 무효화하도록 한다. 별도 지속 cache나 검증을 생략하는 실행 옵션은 추가하지 않는다.

## G04 작은 오망성

별이 그려지는 기존 작은 오망성 V1 이펙트에 기존 오망성 바닥 요소를 식별해 추가한다. 실제 중앙 V1은 해골·연기 위주이므로 바닥 별이 있는 boss.kouku.disarm의 decal을 사용한다. 기존 별 궤적의 실측 범위와 시간에 맞춰 바닥 크기·수명을 정하고 원본 후보 표기를 작은 오망성으로 바꾼다. 공용 원본과 다른 패턴의 튜닝은 보존한다. 효과 세부 조사·변경·수치 근거는 대응 작은 오망성 PLAN/RESULT에 기록한다.

## G05 검증과 완료 경계

기존 Composition/publisher 검사에 레거시 roundtrip, Parent 참조, 반복, 시간 겹침 거부, namespace와 내부 참조, deadline 취소, 잘못된 항목 격리를 추가한다. 필요한 C++ 소스만 컴파일하고 실제 연결을 최소 링크로 확인한다. 실제 저장 문서의 공식 Kouku publisher와 변경 JSON parse, git diff --check를 수행한다. 코드 결과와 사용자 화면 판정을 RESULT에서 분리한다.

LAN 설정은 server-host, 192.168.0.14:7777, 방화벽 준비 상태다. 사용자가 Server + Client profile을 직접 시작하고 Parent 편집·재생 및 작은 오망성 바닥의 최종 크기/표시를 확인한다.

### G03-04. Pattern Resources와 편집 대상 선택 분리

현재 `3관문_쿠크세이튼_마리오`는 folder.15의 backing Pattern37을 가진 15000ms Parent다. 기존 Composition Patterns의 선택은 편집 대상을 전환하며 Append는 Parent 선택만 받는다. 기존 저장 계약은 유지하고 Workbench H/CPP에서 Pattern Resources 탭과 공용 `Render_PatternTree`를 연결한다. Resource 모드의 leaf 선택은 session의 `m_strAppendPatternId`만 바꾸고 Gate/Parent/Bundle 선택, cursor, preview와 생성 목적지는 보존한다. 두 창은 같은 draft의 Gate → Parent → Bundle → Pattern을 그린다.

상단과 Details의 Append Pattern은 같은 Pattern Resources를 열고 기존 shell view request로 숨겨진 Resources도 표시한다. 실제 추가 버튼은 현재 Parent와 source의 관문/actor/target/lifetime을 매번 검사하고, 기존 `Append_PatternBox → Try_ExpandPatternDocument → Commit_Candidate`로 stage/commit한다. 실패 사유를 보여주고 기존 초안·ordinal을 보존한다. 미생성 Parent의 첫 append와 기존 time overlap 거부도 유지한다. 새 C++ 파일, project/filter 등록, JSON/schema 변경은 없다.

기존 CPU Workbench probe를 격리 Data 사본에서 재사용하여 대상·커서 보존, source 재선택, 잘못된 대상·관문·끝 시점 거부, 실제 append·Save/Reload와 실패 rollback을 확인한다. 현재 소스를 최소 컴파일하고 실행 중 기본 EXE가 잠겨 있으면 종료하지 않고 링크 산출물을 out에 보존한다. Client/UI 실행·조작·화면 판정은 사용자 확인으로 남긴다.
