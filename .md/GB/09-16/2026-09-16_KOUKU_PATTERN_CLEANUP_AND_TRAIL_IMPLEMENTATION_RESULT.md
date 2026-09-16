# 쿠크 Pattern 삭제와 저주의식·Trail 수정 결과

## G00. 소스 반영과 제품 적용 경계

Pattern 삭제 UI·참조 보호, Effect Tool의 저장 원본 애니메이션 선택, 공통 Trail geometry 수정이 이번 소스 변경이다. 사용자 Composition과 Effect Resources를 대신 삭제·이동·재저장하거나 publish하지 않았다. 같은 브랜치의 직전 Complete Play 복구와 사용자의 두 RenderingProfiles 변경을 보존했다. 새 제품 C++ 파일이나 프로젝트 항목은 없다.

Product 빌드는 사용자가 직접 한다는 지시를 유지했다. 격리 컴파일과 아래 수치 검사는 실행했지만 Client/UI 실행·조작·캡처, Save/reload 실사용, 최종 시각 판정은 수행하지 않았다. 현재 소스 반영을 Product 설치 또는 화면 PASS로 기록하지 않는다.

1/3관문 공통화는 [별도 설계](2026-09-16_KOUKU_COMMON_PATTERN_DESIGN.md)까지 조사했다. 공통 분류·source schema·gate별 projection·양쪽 Flow 실행·기존 Pattern 이동은 아직 구현하지 않았다. 단순히 Add from All Patterns의 gate 필터만 해제하면 Server target 검사와 MAP 절대좌표가 어긋나므로 그 우회는 적용하지 않았다.

## G01. Pattern 삭제

Patterns 트리 상단의 Delete Selected Pattern, Pattern 우클릭 Delete, 기존 Details 삭제를 하나의 확인창으로 연결했다. 표시명과 stable ID, 연결 owner를 함께 보여 준다. Flow PATTERN entry, Bundle member, Parent timeline/Pattern box, Logic의 followupPatternId·clonePatternId·directionPatternIds·patternIds, Summon DirectionPatternIds와 occurrence PatternSpawns를 검사한다. 참조가 있으면 owner에서 연결을 먼저 제거하도록 안내하고 삭제를 차단한다. PlayAllPatternIds는 파생 목록으로 삭제 candidate에서 재구성한다.

Delete from Draft는 최신 Composition과 게시 상태를 확인하고 candidate를 검증한 뒤 교체한다. 모달을 연 뒤 draft가 바뀌면 새 상태를 다시 확인하도록 거절한다. Commit이 draft를 교체하기 전에 stable ID를 값으로 복사해 borrowed string_view 수명 결함을 없앴다. 실패는 기존 draft와 선택을 유지하며 성공 후 preview와 source chooser의 삭제된 ID를 정리한다. 공유 Animation/Effect/Logic 원본을 연쇄 삭제하지 않는다.

실제 Workbench TU의 격리 Debug 컴파일이 성공했다. 실제 Collect/Delete/Request 메서드를 추출하고 실제 Composition 구조체를 사용한 native probe는68개 검사를 통과했다. Commit validation/normalization, storage freshness, preview transport, 파생 PlayAll helper는 경계 stub이다. 따라서 이 결과는 저장·재로드 또는 ImGui 실사용 증거가 아니다. 증거는 `out/PatternDelete20260916/run.log`, `out/KoukuCompletePlay20260916/KoukuSaydonActionWorkbench.delete.compile.log`다.

사용자 경로는 Patterns → 선택 후 Delete Selected Pattern 또는 우클릭 Delete → 참조 owner 정리 → Delete from Draft → Save → Publish All Patterns다. 이번 작업에서 실제 사용자 Pattern은 삭제하지 않았다.

## G02. 저주의식의 원본 애니메이션

설치 모델과 저장 SourceModelPreview를 대조했다. G1_SAYDON과 G3_SAYDON은 같은 MN_RPCT_05 WModel 및 .017 preScale을 사용한다. 저주의식 원본은 rpct00_att_battle_27_01/clip88, 마리오는 rpct00_att_battle_29_02/clip106이다. 저장 저주의식 source는 정상이었으나 Workbench에서 같은 Effect를 쓰는 마리오 P33을 선택한 상태에서 Open/Play가 현재 Pattern provider를 자동 호출해 다른 clip으로 바꿨다. 인덱스 오매핑이나 서로 다른 세이튼 모델 문제가 아니었다. 실측 근거는 `out/KoukuCursePreview20260916/installed-clip-check.json`이다.

Effect Tool은 이제 저장 source를 기본으로 사용한다. Use current Pattern animation을 명시적으로 누르면 해당 Effect와 Pattern/occurrence의 animation·start·duration·loop snapshot을 검증 후 고정한다. 다른 창의 선택 변경은 이를 바꾸지 않는다. Use saved source animation 또는 성공한 문서 재오픈은 저장 원본으로 돌아간다. 로드 실패와 미저장 취소는 기존 선택을 유지한다. Play All/Group과 source bone sampling은 같은 resolver를 소비한다. pin 뒤 Pattern 편집을 반영하려면 버튼을 다시 눌러 snapshot을 갱신한다.

Effect_Tool_Workspace.cpp, Effect_Tool_DocumentIo.cpp와 관련 소비자 Effect_Tool_CatalogPreview.cpp, MainApp.cpp의 격리 Debug 컴파일을 통과했다. 기존 코드 페이지 C4828 경고는 남았다. 교차 검토에서 새 문서의 성공 commit 뒤에만 pin이 초기화되는 경로와 pose/bone 공통 소비를 확인했다. 저주의식/P61 authored 데이터는 수정하지 않았다.

실제 Resolve_KoukuPatternPreviewContext 본문과 실제 EFFECT/preview 타입으로 구성한 native probe는55개 검사를 통과했다. 저장 source 보존, provider 자동 호출0회, 같은 Effect의 pin 적용, 다른 Effect의 pin 무시, 검증 거절 시 optional source와 timing 복원, pin 해제 뒤 기본값 복귀를 확인했다. Validate는 accept/reject를 주입하는 경계 stub이므로 codec 자체·Tool 초기화·Load/Save·UI·GPU 재생 검사가 아니다. 증거는 `out/EffectPatternPreview20260916/run.log`와 `manifest.json`이다.

사용자 경로는 Effect Tool → 저주의식 열기 → Animation: saved Effect source 아래 rpct00_att_battle_27_01 확인 → Play All이다. 특정 Pattern의 사용 모습이 필요하면 Boss Tool의 해당 Effect box 선택 → Effect Tool의 Use current Pattern animation → Play All을 사용한다.

## G03. Trail geometry

현재 sampling은60Hz다. 기존 설치 CModel 샘플225프레임을 현행 Hermite/tessellation과 함께 검사했을 때, camera/tangent cross로 독립 계산한 이웃 폭 축이 뒤집히는 구간이 확인됐다. camera4개에서 뒤집힘은5/53/42/0개였다. 이는 shader의 원본 색·폭 식을 바꿀 근거가 아니므로 공통 CPU strip 생성 경계를 수정했다.

카메라를 향하는 비beam centerline의 평행·왕복·중복점 tangent와 폭 축 fallback을 보강하고 이웃 edge의 방향을 연속화한다. ground-facing과 beam은 기존 폭 축 계산을 유지한다. 완전퇴화 구간은 연결을 끊어 생략된 구간을 큰 quad로 잇지 않는다. baked AnimationTrail의 EdgePairs에는 centerline tessellation을 적용하지 않아 빈 Points.front 접근을 막는다. 저작 width·UV·색·time·payload, source shader,60Hz sampling과 Resources는 보존한다.

교차 검토에서 폭 축의 부호만 연속화하면 단면 재질의 triangle winding이 뒤집힐 수 있음을 추가로 확인했다. 실제 spinning.card.throw는 단면 CascadeRibbon24개를 사용한다. camera-facing 비beam의 ALPHA/ADDITIVE_ONE_SIDED_DEPTH_READ에만 triangle별 winding을 기존 clockwise front 방향으로 보정했다. vertex/UV/material과 ground-facing·beam·baked geometry는 이 보정 대상이 아니다. 원본 재질을 양면으로 바꾸지 않았다.

최종 실제 TU 격리 Debug 컴파일이 성공했다. 현재 생산 helper와 tessellation guard·index 블록을 추출한 native probe에서221,747개 검사를 통과했다. 기존 모델 샘플225프레임×camera4개에서 폭 축 반전5/53/42/0개가 모두0개로 줄었고43,620개 triangle의 front 방향을 확인했다. sharp reversal fixture의4개 triangle 교정, 카메라 평행·왕복·완전중복점·퇴화 구간 비연결·baked empty Points와48개 재질/geometry 조합의 보정 범위도 검사했다. 근거는 `out/TrailGeometry20260916/probe-result.json`, `probe-extraction.json`, `compile.log`이며 교차 검토 fixture는 `out/KoukuCursePreview20260916/trail-winding-review.json`이다. 과거 실제 CModel 샘플과 현재 코드 수치 검증이며 최신 게임 화면 재현이나 GPU visual PASS가 아니다.

## G04. 남은 확인

사용자가 Product 빌드 후 새 Client로 삭제 확인창과 Save/Publish, 저주의식 원본 애니메이션, 쿠크·발탄 Trail을 직접 확인해야 한다. 공통화 설계의 실제 구현과 양쪽 관문 Flow 실행은 별도 미완료 범위다. 이번 변경에서 authored JSON/XML을 수정하지 않았다.

최종 `git diff --check`를 통과했다. 기존 인코딩/줄 끝 경고와 기존 C4828 경고는 성공 여부와 구분했으며 무관한 파일을 재인코딩하지 않았다.
