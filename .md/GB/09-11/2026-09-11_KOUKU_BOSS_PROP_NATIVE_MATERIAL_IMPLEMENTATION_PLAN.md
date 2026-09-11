# 쿠크 보스·컷신·소품 원본 재질 구현 계획

## G00. 실측과 목표

전투 쿠크·세이튼은 `CKoukuSaydonPresentationAssetService::Ensure_Prototypes`에서 경로만으로 CModel을 생성한다. 컷신 `MN_RPCT_00`도 DeployPropRuntime의 같은 경로 기반 생성이므로 현재 native material override를 소비하지 않는다. 기존 CharacterCatalog의 `modelMaterialOverrides`와 공의 World Object `materialProfile`은 실제 CModel → CMaterial → SourceCharacter Base/Light 경로까지 연결되어 있다.

현재 승인된 공 program 21을 보존한다. 쿠크/세이튼은 원본 MIC의 static shader map, texture expression, parameter를 조사해 동일한 program을 공유하는 재질만 묶는다. 피부·발광·불투명 몬스터 분기를 공 profile 복사로 대체하지 않는다. 원본 runtime texture는 Resources의 물리 입력이고 Git에 추가하지 않는다.

## G01. 원본 shader와 입력

`Shader_SourceCharacterPrograms.hlsli`와 `SourceCharacterMaterialParameters.h`에 신규 원본 Base/Light pair와 named parameter packing을 추가한다. 기존 program 1~21은 바꾸지 않는다. Model/Material의 허용 program 범위와 실제 shader dispatch를 함께 연결한다. 각 원본 texture의 실제 sRGB flag와 expression index를 적용한다.

## G02. 모델 생성 소비자

BossCatalog에 optional 모델별 공유 `modelMaterialOverrides`를 추가해 전투용 보스, 컷신 body, 소품이 같은 Resources-relative 모델 ID로 입력을 소비하게 한다. 별도 gameplay actor나 모델 runtime은 만들지 않는다. ActorCatalog는 `parse → validate → stage → commit` 후 `Build_ModelLoadDescription`으로 공유 material override를 전달한다. Boss Ensure_Prototypes, Character 모델을 쓰는 Deploy, World Object가 실제 descriptor를 CModel에 전달한다. 기존 World Object 명시 profile은 동일 slot의 최종 override로 유지한다.

## G03. 검증과 종료 경계

원본 material map과 texture closure, 변경 JSON parse, 실제 ActorCatalog/CModel load와 Shader native pair 수치 비교, 필요한 C++와 FX 컴파일, `git diff --check`를 확인한다. Product 빌드와 publisher는 통합 담당 root가 한 번 실행한다. 새 C++ 파일은 없어 project/filter 등록을 추가하지 않는다. 실제 실행과 사용자 최종 화면 판정은 RESULT에서 미실행으로 분리하며 Client/UI 실행·캡처를 하지 않는다.
