# 유령 발탄 Character Select 미리보기 재질 경로 복구

## G01. 실제 미리보기 consumer와 목표

첨부 화면에서 유령 발탄의 머리와 상체는 매우 옅고 도끼와 손·발 일부가 진하게 보인다. 기존 [Valtan 편집 결과](2026-09-22_VALTAN_EDITOR_VISIBILITY_RESULT.md)의 수치 검증은 전투 `CBody_Valtan`의 native84/pass10 경로다. Character Select의 `boss.valtan.ghost`는 `pBossArchetypeId=nullptr`이므로 `CharacterPreviewPanel::Select_Asset`의 generic `CPart_Body` 경로를 사용한다.

현재 `CPart_Body::Resolve_TranslucentSourcePass`는 native6/7/18/99만 등록하고 native84를 누락했다. 그래서 ghost 3개 mesh가 NONBLEND/pass0 deferred geometry에 들어간다. 전투는 동일 재질을 BLEND/pass10으로 제출한다. 기존 모델·재질의 색과 alpha를 보존하면서 두 consumer가 같은 원본 translucent pass를 선택하도록 연결한다.

## G02. 변경 파일과 함수

`Client/Private/Part_Body.cpp`의 기존 anonymous namespace에 one-sided pass 상수 10을 둔다. `Resolve_TranslucentSourcePass`는 SOURCE_CHARACTER/native84에 10을 반환하고 기존 두 면 hair/eye 프로그램의 9 선택을 유지한다. 모델 이름이나 현재 Level로 분기하지 않는다.

호출 흐름은 `Initialize`의 translucent mesh 인식 → `Late_Update`의 BLEND 등록 → 기본 `Render_Pass(0)`의 중복 제외 → `Render_Translucent`의 scene light/base material/light material/bone binding → pass10 mesh 제출이다. 생성 rollback, weapon socket, donor clip, 본 자세 및 데이터 정본을 바꾸지 않는다. 기존 H 계약과 project/filter 등록을 그대로 사용한다.

## G03. 검증

변경 TU를 Debug 설정으로 컴파일한다. 설치된 실제 ghost WModel와 cinematic donor 및 현재 `scene.character-select.warm-high-key.v1`의 조명으로 기존 pass0과 pass10을 비교한다. 수치 출력은 각 mesh의 covered/colored/RGB/alpha와 nonfinite를 구분하며 직접 consumer 선택의 누락을 별도로 확인한다. 기존 hair/eye의 pass9와 일반 body의 pass0 경로를 확인한다.

Client/UI는 실행하거나 캡처하지 않는다. headless 수치 검증은 사용자의 실제 카메라·현재 실행 메모리·최종 화면 판정을 대신하지 않는다. 전체 제품 빌드와 설치는 통합 작업에서 다루며 RESULT에 실행한 범위를 적는다.
