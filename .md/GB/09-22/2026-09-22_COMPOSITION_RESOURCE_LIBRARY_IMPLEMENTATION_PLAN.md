# Composition Resource Library 구현 계획서

## G00. 현재 실측과 목표

착수 branch는 `GB/collider-pattern-bug-fix`다. 기존 dirty 변경을 보존한다.
`CompositionAnimationResource.h`의 물리 목록은 21개 보스·몬스터만 등록하며 7개 플레이어 클래스가 빠져 있다.
`SequencerTool`은 그 목록을 기본 숨김으로 시작하고 Character owner에서는 아예 그리지 않는다.
`EffectAuthoringSequencer_Resources.cpp`는 V1 저장본의 category를 모두 `Saved Effects`로 덮어써 트리를 잃는다.

## G01. 설치 애니메이션과 기획 명칭

`CompositionAnimationResource.h`에 실제 `ANIMATION_PREVIEW_ASSETS`의 7개 클래스 이름을 추가한다.
기존 모델·AnimSet header reader와 실제 CModel preview를 사용한다. 표시 이름은 읽기 전용 clipmap과
현재 skillbinding/PlayerSkill 이름에서 가져오고 source clip identity와 package identity는 유지한다.
`SequencerTool.cpp`의 공용 browser는 Character/Boss/transform 범주의 모든 native clip을 접는 트리로 제공하고
현재 session의 typed Append 검사와 기존 Animation preview route를 유지한다.

## G02. 저장 이펙트 조직과 재생

`EffectAuthoringSequencer_Resources.cpp`는 V1 inventory를 `EffectResourceTree.json` 조직과 stable asset ID로 join한다.
저장된 기획 명칭과 category를 보존하고 미조직 항목은 실제 asset family 이름의 범주에 둔다.
선택 이펙트를 Preview/Append할 때 기존 V1/V2 callback과 document lifetime을 소비한다.
현재 model clip의 표시 이름에도 공용 기획 명칭 projection을 적용한다.

## G03. 검증

신규 C++ 파일이 없으므로 project/filter 등록은 추가하지 않는다. 기존 UTF-8 BOM 없음/CRLF를 유지한다.
변경 TU 최소 컴파일은 root가 통합 실행한다. 설치 WModel header의 7개 클래스 clip 수와 label 매칭,
단일 body 선택·Append owner guard·실제 preview 경로를 검사하고 `git diff --check`를 실행한다.
Client/UI 자율 실행과 최종 화면 판정은 하지 않는다. RESULT에 source, compile, 수동 확인 경계를 나눠 기록한다.
