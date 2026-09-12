# 쿠크 Pattern Start Offset 구현 계획

## G00. 목표와 현재 경계

사용자는 세이튼 등장 이펙트를 패턴 0ms부터 재생하고 등장 애니메이션을 늦추는
Pattern Start Offset 추가를 승인했다. 마리오 회차 로직은 애니메이션·로직·이펙트 저작 후
진행하도록 보류한다. 이번 변경은 마리오 데이터와 회차 상태를 수정하지 않는다.

현재 첫 Stage와 clip 길이가 같으면 Workbench 드래그 상한이 0이다. publisher와 제품
animation binding loader도 startOffsetMs=0을 요구한다. UI만 수정하면 실제 재생은 거절된다.

기준 HEAD는 2f289461461434bcc0442bae5a71b795e7b34c62, 브랜치는
codex/sequencer-camera-load-performance다. 다른 작업의 카메라, enterCombatOnFinish,
World Object 편집과 렌더링 변경이 진행 중이다. 기존 diff를 보존하고 자동 stage/commit하지 않는다.

## G01. 편집과 저장

새 pattern JSON 필드는 만들지 않는다. 기존 첫 Stage의 가장 이른 animation startOffsetMs를
Pattern Start Offset ms로 표시한다. 값을 바꾸면 첫 Stage의 animation offset과 Stage 길이를
같은 차이만큼 변경한다. 후속 Stage는 기존 누적 시계로 함께 이동한다. Effect, Logic,
World, Summon, Camera의 pattern-relative 시간과 원본 clip의 sourceStartMs는 유지한다.
최종 duration과 기존 모든 row 검증이 성공한 candidate만 commit하며 실패 시 draft를 보존한다.

Workbench H/CPP의 실제 setter와 Pattern Detail을 연결한다. 저장은 기존 CompositionDocument를
재사용한다. 기존 0ms 파일은 동일하게 동작한다. 비어 있는 첫 Stage는 원인을 표시하고 거절한다.

## G02. Publisher와 실제 재생

PRODUCT의 stage당 animation 정확히 하나와 sourceStartMs=0은 유지한다. 첫 Stage만
nonzero startOffset을 허용하고, 이후 Stage의 기존 stage-entry 정책은 유지한다.
첫 Stage duration에 지연이 포함되므로 Server stage/protocol을 새로 만들지 않는다.
게시 binding이 offset을 보존하고 Client loader의 runtime row가 그 값을 실제 보관한다.

ClientReplication과 CNpc가 Server action age에서 offset을 뺀 시계로 clip을 재생한다.
offset 이전에는 target clip의 첫 pose를 유지하고, 경계를 넘은 frame은 초과 시간만 진행한다.
늦은 snapshot, loop, 재시작, 중단, hold-at-end와 기존 0offset 동작을 보존한다.

## G03. Preview와 본 좌표

일반 preview, bundle preview, source attachment sampler와 publisher bone collider bake는
첫 animation 이전에 같은 첫 pose를 사용한다. 사전 이펙트가 본을 참조해도 아직 시작하지 않은
animation이라는 이유로 실패하지 않아야 한다. 이후 clip gap과 기존 end policy는 보존한다.
사용자 Client/UI 실행이나 화면 캡처는 하지 않는다.

## G04. 검증과 전달

기존 pipeline test에 offset 경계, 첫 pose, 후속 stage 정책, serialization/projection 보존을 검증한다.
Workbench의 실제 setter를 통한 값 변경/실패 보존 및 실제 runtime window의 지연 전·경계·후·late
sample을 비UI 검사로 확인한다. 변경 C++의 최소 Debug 컴파일과 필요한 최종 링크를 수행하며
다른 작업의 build/publisher와 동시에 같은 output을 쓰지 않는다.

새 C++ 파일이 없으므로 vcxproj/filters 등록은 불필요하다. 기존 C++ UTF-8 BOM 없음과 CRLF를
유지한다. 변경 JSON/XML이 있으면 parse하고 git diff --check를 실행한다. 사용자가 저장해 둔
패턴의 지연 값을 임의 튜닝하거나 Data 전체를 재작성하지 않는다. 실제 검증과 실행파일 반영,
수동 화면 확인의 남은 경계는 대응 RESULT에 기록한다.
