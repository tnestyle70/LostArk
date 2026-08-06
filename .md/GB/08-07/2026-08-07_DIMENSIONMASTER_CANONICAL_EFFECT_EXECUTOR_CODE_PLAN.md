# 차원술사 체험 모드 정본 Effect 실행 변환 코드 계획

## 1. 코드 경계

이번 변경은 두 번째 Effect 런타임을 만들지 않는다. 기존
`CEffectPresentationService -> CEffectCatalog -> CEffectPlayback ->
CEffectDocumentRenderer` 경로가 다음 생성물을 소비하도록 정본 생성기와 검증 경계를 교정한다.

```text
PlayerSkills + skillbindings
  -> Action source / normalized Particle graph / module closure
  -> Imported lossless document
  -> Authored v11 document
  -> WFX Component
  -> Assembly
  -> EffectCatalog.runtime.json
```

## 2. G01 current roster admission

파일:

- `Tools/LevelPlacementExtractor/build_dimensionmaster_base_effects.py`
- `Tools/LevelPlacementExtractor/materialize_dimensionmaster_base_effects.py`
- `Data/Balance/PlayerSkills.json`
- `Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json`

`dimensionmaster_admitted_skills()`는 다음 조건을 모두 만족한 행만 반환한다.

1. `characterClass == DIMENSIONMASTER`
2. 비어 있지 않은 `effectId == effect.dimensionmaster.skill.<skillId>`
3. 같은 `skillId`의 skillbinding과 하나 이상의 실제 CModel clip 존재
4. 중복 skill/effect identity 없음

입력 슬롯은 UI label일 뿐 저장 identity로 쓰지 않는다. 현행 정본은
`LMB/Q/W/E/R/A/S/F/D/T/V/ALT_V` 12개이며 SPACE는 이번 Effect 경계 밖이다.

## 3. G02 source extraction과 손실 없는 변환

파일:

- `Tools/LevelPlacementExtractor/build_skill_effect_source_receipt.py`
- `Tools/LevelPlacementExtractor/build_action_cue_recipe.py`
- `Tools/LevelPlacementExtractor/build_imported_effect_documents.py`
- `Data/Effects/Imported/DimensionMaster/Modules/*.external-module-closure.json`

생성기는 현재 12개 normalized graph만 source manifest에 승인한다. 원본 LOD Modules 배열의
순서, 중복 occurrence, 외부 package import reference는 그대로 보존한다. 추측 리소스나 흰색
fallback을 만들지 않는다. unresolved source system과 closure request가 하나라도 있으면 해당
경계는 완료가 아니다.

## 4. G03 Authored v11 Material profile 재생성

파일:

- `Tools/LevelPlacementExtractor/build_effect_source_material_contract.py`
- `Tools/LevelPlacementExtractor/materialize_dimensionmaster_base_effects.py`

Authored를 다시 만들 때 이전 Authored의 profile을 복사하지 않는다. 각 스킬의 source receipt,
conversion receipt, class resource manifest를 조인하여 `sourceMaterialPath`, physical package,
render profile, texture bindings, Dynamic Parameter, SubUV 정보를 다시 구성한다. 같은 material
object path가 여러 package에 있으면 `sourcePhysicalPackage`가 일치하는 행만 EXACT로 승인한다.

완료 조건:

- current aggregate 12개와 BA1~BA4 모두 v11
- Particle sourceProfile enabled 수가 Particle 수와 동일
- A의 46 Particle / 21 공유 profile / 전용 shader 14 / Dynamic 32 / SubUV 2 회귀 유지

## 5. G04 T Model Cue

파일:

- `Tools/LevelPlacementExtractor/build_action_cue_recipe.py`
- `Data/Effects/Imported/DimensionMaster/DimensionMaster.model-cue-runtime-bindings.json`
- `Tools/LevelPlacementExtractor/materialize_dimensionmaster_base_effects.py`

`PlaySkeletalMesh` payload의 source cue, skeletal mesh, AnimSet, MI 목록, local transform을 typed
payload로 해독한다. 검증된 source/runtime binding과 retime receipt가 정확히 하나씩 조인될 때만
Model Cue를 만든다.

```text
effect: effect.dimensionmaster.skill.2050500
model: Character/DimensionMaster/DimensionMaster_DimensionSummon.wmodel
clip: sk_swp_dms_00_sk_sk_dimensionprison
start: 0.0s
duration: 131 / 30 = 4.3666667s
position: UE [0,0,45] -> project [0,0,0.45]
scale: [1.2,1.2,1.2]
asset pre-scale: [0.01,0.01,0.01]
asset pre-rotation: [0,-90,0]
```

2.901422초의 `PlaySkeletalMeshMaterialParam`은 semantic/GPU consumer가 구현되기 전까지
명시적 unsupported로 남긴다.

## 6. G05 Component, Assembly, catalog

파일:

- `Tools/LevelPlacementExtractor/build_effect_components.py`
- `Data/Effects/EffectCatalog.json`
- `Tools/EffectPipeline/Publish-Effects.ps1`

Component stable ID와 물리 디렉터리는 `skill.<id>[.baN]`에서 파생한다. 생성 뒤 Assembly를
다시 compile한 문서와 Authored JSON의 canonical 의미가 같지 않으면 publish하지 않는다.
카탈로그에서 빠진 과거 후보의 generated Assembly/WFX만 schema/source owner를 확인한 뒤
제거한다. 후보의 Imported/Authored 원본 증거는 삭제하지 않는다.

예상 게시 수:

```text
aggregate Effects 12
BA stage Effects 4
total Effects 16
WFX Components 182
Emitters 947
```

## 7. G06 Animation cue와 runtime fallback

파일:

- `Data/Animation/Authored/DimensionMaster/DimensionMaster.animevents`
- `Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp`

과거 후보 `2050110/2050150/2050190/2050200/2050510/2050550` asset cue를 제거한다.
BA1~BA4처럼 clip별 stage identity가 필요한 cue와 T의 source-confirmed cue는 명시 행을 유지한다.
명시 cue가 없는 current active skill은 기존 `Spawn_FallbackEffect`가 PlayerSkills의 complete
Effect를 root에서 한 번 시작한다. body animation이 Effect에 종속되는 구조가 아니라 같은
승인 action이 body clip과 Effect presentation을 각각 시작하는 구조다.

## 8. G07 Resource Index와 프로젝트 등록

파일:

- `Tools/LevelPlacementExtractor/build_skill_effect_resource_index.py`
- `Data/Effects/ResourceIndex/DimensionMaster/<SLOT>/*.json`
- `Tools/EffectPipeline/Sync-EffectDataProject.ps1`
- `Tools/EffectPipeline/Test-EffectDataProjectRegistration.ps1`

리소스 바이너리는 슬롯 폴더로 복사하지 않는다. Q~ALT_V 폴더에는 실제 Authored가 참조한
Resources-relative asset ID, Mesh/Texture/Model 구분, material identity, Element/Model Cue
역참조, 물리 파일 존재 여부를 저장한다. Effect Tool의 기존
`Assembly -> Component -> Emitter -> Resources -> Modules` 트리는 실행 구조를 보여주고,
동적 Resource Library가 이 index를 직접 필터링하는 UI는 별도 수직 슬라이스로 둔다.

## 9. 검증 순서

```text
Python unittest discover
Gameplay balance Validate
Imported/Authored materialize
WFX split + Assembly compile identity
Resource Index --require-complete
Effect project/filter sync harness
Publish-Effects Validate/Publish
ClientFrontendHarness --skill-binding-fast
ClientFrontendHarness --effect-runtime-fast
ClientFrontendHarness --effect-executor-fast
Test-EffectToolFinal
ProjectAudit
Engine -> UpdateLib -> Client Debug build
Client startup smoke
```

자동 검증과 GPU 수동 검증은 분리한다. 자동 PASS만으로 원작 픽셀 동일을 선언하지 않는다.
