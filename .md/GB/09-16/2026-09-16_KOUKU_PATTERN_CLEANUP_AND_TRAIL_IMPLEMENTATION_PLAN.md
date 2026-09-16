# 쿠크 Pattern 삭제와 Effect 원본 애니메이션·Trail 수정

## G00. Pattern 삭제

기존 ActionWorkbench::Delete_Pattern과 저장 경로를 확장한다. 트리 상단과 우클릭에서 삭제를 열고 표시명·stable ID·참조 owner를 확인하는 공통 modal을 제공한다. Parent timeline/Pattern occurrence, Bundle, Flow, Logic follow-up·candidate, Summon directional/clone/pattern spawn 참조를 모두 검사한다. 참조 중인 패턴은 실제 owner를 안내하고 삭제하지 않는다. 삭제를 위해 관련 gameplay를 조용히 연쇄 변경하지 않는다. 참조가 없고 현재 saved baseline이 유효하면 draft candidate에서 제거하고 Validate/Commit 후 기존 Save/Publish로 반영한다. Commit 전에 ID를 값으로 보존해 string_view 수명이 끝난 뒤 접근하지 않는다. 실제 사용자의 패턴을 에이전트가 대신 삭제하지 않는다.

## G01. Effect Tool 애니메이션 원본과 명시 선택

저주의식 Effect의 저장된 SourceModelPreview는 rpct00_att_battle_27_01,4667ms다. 현재 Workbench에서 같은 Effect를 사용하는 마리오 P33이 선택돼 있으면 Resolve_KoukuPatternPreviewContext가 그것을 매번 소비해 다른 clip으로 바꾼다. Effect 문서를 여는 기본 동작은 저장된 source animation을 사용한다. 사용자가 Use current Pattern animation을 명시적으로 누르면 해당 Pattern/occurrence와 source sampling snapshot을 고정하고, 다른 편집창 선택 변화가 재생 clip을 바꾸지 않게 한다. Use saved source animation으로 복귀한다. 문서 로드 성공 commit은 context를 초기화하고 실패·미저장 확인 대기는 현재 상태를 보존한다. 저주의식 리소스와 P61 source 데이터를 다시 쓰지 않는다.

## G02. 공통 Trail strip의 연속성

현재 runtime sampling은60Hz이며 shader 수정이나 tick 상향의 근거가 없다. 공통 Render_Trails에서 camera와 tangent의 cross로 만든 폭 축이 이웃점에서 반전되는 결함과 baked EdgePairs 경로에서 빈 Points.front를 읽는 경계를 수정한다. 유효 이웃 폭 축을 연속화하며 카메라 평행·중복·왕복점에서 tangent와 side fallback을 사용한다. 완전퇴화 구간은 연결을 끊는다. 원본 width/time/UV/payload와 shader 식은 보존한다. 실제 설치 CModel 샘플과 synthetic 경계로 finite geometry·축 연속성·퇴화 분리·baked empty Points를 검사한다. 사용자 화면 판정과 수치 재현을 구분한다.

교차 검토로 추가 확인한 단면 winding도 같은 변경에서 처리한다. 폭 축 연속화는 camera-facing 비beam에 제한하고 ground-facing/beam은 기존 계산을 유지한다. 대상 단면 재질의 triangle winding만 기존 front 방향으로 보정하며 원본 재질을 양면으로 바꾸지 않는다.

## G03. 검증과 기존 변경 보존

같은 브랜치의 직전 Complete Play·애니메이션 로더 수정과 사용자 RenderingProfiles 변경을 보존한다. 새 제품 C++ 파일이나 project/filter 등록 없이 기존 소비자를 수정한다. 기존 인코딩/BOM/줄 끝을 유지하고 변경 TU 격리 Debug 컴파일, 각 기능의 focused 검사와 git diff --check를 수행한다. Client/UI 실행·조작·캡처는 하지 않는다. Product 빌드는 사용자가 직접 한다는 현재 지시를 따른다.

1/3관문 공통 Pattern의 별도 설계는 같은 날짜 COMMON_PATTERN_DESIGN에서 정의하며 모델 공유와 관문별 위치·실행 target을 분리한다.
