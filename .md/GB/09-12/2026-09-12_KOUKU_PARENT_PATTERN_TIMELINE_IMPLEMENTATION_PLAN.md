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

## G04 작은 오망성

별이 그려지는 기존 작은 오망성 V1 이펙트에 기존 오망성 바닥 요소를 식별해 추가한다. 실제 중앙 V1은 해골·연기 위주이므로 바닥 별이 있는 boss.kouku.disarm의 decal을 사용한다. 기존 별 궤적의 실측 범위와 시간에 맞춰 바닥 크기·수명을 정하고 원본 후보 표기를 작은 오망성으로 바꾼다. 공용 원본과 다른 패턴의 튜닝은 보존한다. 효과 세부 조사·변경·수치 근거는 대응 작은 오망성 PLAN/RESULT에 기록한다.

## G05 검증과 완료 경계

기존 Composition/publisher 검사에 레거시 roundtrip, Parent 참조, 반복, 시간 겹침 거부, namespace와 내부 참조, deadline 취소, 잘못된 항목 격리를 추가한다. 필요한 C++ 소스만 컴파일하고 실제 연결을 최소 링크로 확인한다. 실제 저장 문서의 공식 Kouku publisher와 변경 JSON parse, git diff --check를 수행한다. 코드 결과와 사용자 화면 판정을 RESULT에서 분리한다.

LAN 설정은 server-host, 192.168.0.14:7777, 방화벽 준비 상태다. 사용자가 Server + Client profile을 직접 시작하고 Parent 편집·재생 및 작은 오망성 바닥의 최종 크기/표시를 확인한다.
