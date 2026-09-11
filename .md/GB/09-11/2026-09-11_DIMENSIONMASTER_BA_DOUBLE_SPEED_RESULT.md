# 차원술사 BA 표현 배율 2배와 전체 애니메이션 연결 복구 결과

## G00. 실제 변경

최신 main의 DimensionMaster.skillbindings.json은 skill 2050010을 `_01`, `_03`, `_04`의 3단계로 연결한다. PR #357의 중복 `_02` 단계 제거를 보존하고 세 clip에 playRate 2.0을 설정했다. 첫 clip의 playMs 1400과 나머지 원본 clip 재생 범위, 다른 skill binding과 Server action/판정/입력창, root motion 수치는 최신 main 그대로다.

선택된 Product cue는 각각 effect.dimensionmaster.skill.2050010.ba0/ba2/ba3.full.restore다. Character::Start_Clip의 model speed와 Update_EffectCues의 EFFECT_SPAWN_DESC가 같은 clip 배율을 소비한다. EffectPresentationService는 배율을 EffectObject에 전달하고 EffectObject는 delta*rate로 Playback을 진행한다. 이로써 스폰 시점·burst·파티클 진행과 수명이 같은 2배 시계로 동작한다. 발생량이나 source spawnRate를 별도로 곱하지 않았다. `_02`와 ba1의 원본 저작 자료는 삭제하지 않았다.

## G01. 충돌과 전체 애니메이션 미재생 경로

PR #358의 시작점 e26cd2b2는 BA 4단계였고, 최신 main db8c2d2d는 PR #357을 통해 Server/Client 모두 3단계로 바뀌었다. 양쪽이 같은 binding을 수정한 것이 유일한 Git 충돌이었다.

AnimationSkillBindingDocument.cpp의 Validate는 COMBO clip 수와 Server 소유 comboStages 수 불일치를 `COMBO clip count must match the Server-owned combo stage count.`로 거절한다. Character.cpp의 Load_ClipChains는 검증 실패 시 전체 chain commit 전에 반환한다. 따라서 3단계 catalog와 4단계 binding이 섞이면 차원술사의 다른 스킬도 재생되지 않는 경로가 발생한다. 최신 3단계로 정렬해 이 실패 조건을 제거했다. 이전 PR HEAD의 문서/Balance는 각각 4단계이고 main의 문서/Balance는 각각 3단계여서 각 Git 버전 단독으로는 이 불일치가 없다. 사용자 실행 당시 로그가 없어 이 경로를 신고 현상의 확정 원인으로 처리하지 않는다.

## G02. 검증과 적용

- 충돌 표식 제거 후 JSON parse와 3단계 playRate 2.0 확인 완료.
- 실제 모델의 136 animation과 전체 14개 binding / 23개 clip 참조를 대조해 이름·단계·원본 재생 범위·배율 검사 PASS. 기존 stage-split 회귀 검사 7개 PASS. 최신 main과 비교한 의미 변경은 BA 세 clip의 playRate 2.0뿐이다.
- Kouku 공식 owner가 포함하는 Gameplay publisher PASS. 실행 bootstrap도 BA stage 0~2이며 원본 3단계와 일치한다. 관련 JSON 9개/XML 8개 parse 및 PR diff --check PASS. 구조 검사 증거는 `out/PR358Integration20260911/dimensionmaster-binding-model-diagnostic.json`, `integration-data-check.json`, `parse-check.json`이다.
- Debug/Release 진단 JSONL 215개에는 Character 로드 실패 문자열이 없었지만 해당 오류는 OutputDebugStringA로만 출력되므로 과거 실행의 성공 근거는 아니다.
- 통합 Product Debug는 Engine/Shared/Server 빌드 후 Client 컴파일 도중 사용자 Visual Studio 빌드와 같은 Debug 출력을 쓰는 것이 확인되어 에이전트 빌드만 중단했다. 최종 Client 컴파일과 사용자 신고 현상의 해소 여부는 사용자 빌드·실행 확인 전이다. PR 갱신 후 사용자가 확인하고 병합한다.
- Client/UI 실행 및 화면 검증 미실행. 새 빌드 적용 후 차원술사 LMB와 각 스킬 슬롯을 사용자가 확인한다. Server의 콤보 진행·판정 시간은 기존 그대로인 presentation 배율 변경이다.
