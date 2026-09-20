# 차원술사·도화가 크기와 Alt V 초반 연출·최종 타격

## G00. 실제 기준과 작업 범위

2026-09-19 현재 dirty worktree의 다른 쿠크·렌더링 작업을 보존한다. CharacterCatalog의 presentationScale은 차원술사 1.5에서 1.05, 도화가 기본 1에서 1.5로 조정한다. Character presentation root만 변경하며 gameplay Transform과 스킬 크기의 배율로 사용하지 않는다.

## G01. 기존 ScreenPost의 수축 편집 계약

Effect_AuthoringDocument의 ScreenPost에 좌·우·상·하 수축 속도, 도착 위치 offset, 회전과 정사각형 옵션을 추가한다. 양수 speed는 기존 smoothstep 진행도의 지수로 적용해 모든 변이 같은 종료 시각에 도달하며 기본 1은 기존 동작이다. captureShrinkSeconds는 cube에서도 Timing 수명 이하를 허용하고 남은 수명에는 종료 모양을 유지한다. cube ModelCue 시작과 ScreenPost 종료 일치 검사는 유지한다.

기존 SourceRecipeIo의 parse/serialize, Validation, Effect Detail Apply/Save, Build_NativeScreenPost의 immutable snapshot, CEffectNativeScreenPostMaterial Bind와 기존 SceneImageCollapse pass를 연결한다. 기본 필드는 기존 문서 동작을 유지하며 지원하지 않는 profile에 capture 전용 값을 저장하면 거절한다. 정사각형·회전은 화면 pixel 비율을 사용하고 고정 Color/Bloom에 동일 UV를 적용한다. live scene의 배경 감쇠는 별도 선택 값으로 저장하며 native shader 원작 복원이라고 주장하지 않는다. 새 C++ 파일과 프로젝트 등록은 없다.

## G02. 큐브 원작 수식·환경 영향 조사

첨부 세 이미지는 액자, 기울어진 장면, 입체 큐브와 밝은 모서리를 보여 준다. 단일 이미지로 multiply 수식이나 cubemap을 확정하지 않는다. 실제 ALT178 재질, source Required/localSpace, capture 시점, camera와 first-pose handoff, SceneHDR/Bloom/tonemap 소비를 대조한다. 이미 양호한 카메라 복귀 후 연출을 전역 shader 보정으로 변경하지 않는다.

## G03. 최종 타격과 피해

기존 Shared/Server hitshape를 사용한다. 차원술사2050540, 도화가31930의 최종 폭발 시각·형상을 실제 authored 연출과 대조하고 각각 창술사34630의 67,652% 중 절반 33,826%를 적용한다. Client Effect가 damage를 판정하지 않는다. 데이터와 provenance receipt는 기존 publisher 절차로 동기화한다.

## G04. 검증·적용

변경 파일 인코딩·무관한 dirty diff를 보존한다. JSON 구조, 실제 Codec 저장/재로드와 잘못된 값 거부, 수축 시작/끝과 방향별 진행, 해당 domain publisher, 정상 증분 Debug Product Build, git diff --check를 수행한다. Client/UI 자율 실행과 화면 캡처를 하지 않는다. 실제 설치, 자동 수치 증거, 사용자 화면 판정은 RESULT에서 구분한다.

## G05. 원래 요청의 쿠크·마리오 재감사

Kouku 전수조사는 별도 원본 clip/shape/sound coverage와 실제 Server 소비를 비교한다. 마리오 조명은 local Server snapshot의 승인된 iMarioStage1~4 동안만 Gate3 무대의 dark profile·source character/direct light 차단을 해제하고, stage0/이탈에서는 기존 Gate3 상태를 복구한다. remote Mario 참가자가 공용 무대를 밝히지 않게 한다. 해당 기존 Level header/CPP·MainApp 함수의 dirty 변경을 보존한다. MarioOriginal 3종 native material override 누락은 조명 차단과 구분하고 원본 material 증거 없이 descriptor를 임의 복제하지 않는다.
