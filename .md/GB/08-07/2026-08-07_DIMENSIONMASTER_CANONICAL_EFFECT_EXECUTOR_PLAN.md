# 차원술사 체험 모드 정본 Animation/Effect 실행 변환 구현 계획

## 1. 목표와 완료 판정

이번 변경은 구형 Effect 슬롯 이름을 현재 슬롯에 다시 붙이는 작업이 아니다. 현재 정본인
`PlayerSkills.json`과 `DimensionMaster.skillbindings.json`의 교집합을 기준으로
`Action Notify -> Particle graph -> external module closure -> Authored Effect -> WFX Component ->
Assembly -> Runtime Catalog`를 다시 생성한다.

완료 조건은 다음과 같다.

- 현재 체험 모드 `LMB/Q/W/E/R/A/S/D/F/T/V/ALT_V`의 skill ID와 body clip이 일치한다.
- 구형 `R=2050190`, `T=2050510`, `S=2050550`은 current slot으로 승인되지 않는다.
- 현재 12개 aggregate Effect와 LMB BA1~BA4가 각자 stable identity로 게시된다.
- Component ID와 파일 경로는 slot 이름이 아니라 `skill.<id>[.baN]`에 종속된다.
- T `2050500`의 Summon Model Cue는 원본 Action payload의 시간·위치·scale과 30fps clip을 소비한다.
- 슬롯별 Resource Index는 바이너리를 복사하지 않고 실제 Mesh/Texture/Model/Material 사용처를 역참조한다.
- 전체 문서가 parse/validate/stage/commit되고 실패 시 기존 Runtime Catalog를 유지한다.
- 관련 Python/C++ harness, Debug build, Effect publisher, ProjectAudit, Client smoke를 통과한다.

Material graph가 조리 원본에 남아 있지 않은 profile과 아직 해독하지 못한 MODEL_MATERIAL cue는
`RECONSTRUCTED_PROFILE` 또는 `UNSUPPORTED`로 남긴다. 이 항목을 픽셀 동일 완료로 기록하지 않는다.

## 2. 정본 연결

```text
Data/Balance/PlayerSkills.json
  (class, inputSlot) -> skillId -> effectId
              +
Data/Animation/Authored/DimensionMaster/DimensionMaster.skillbindings.json
  skillId -> ordered body clips
              +
원본 DIMENSIONMASTER.loa / DimensionMaster.animnotify
  clip/action stage -> notify time, anchor, local transform, presentation payload
              +
원본 UE3 Particle graph / external shared module
  ParticleSystem -> Emitter -> ordered Module occurrence -> Material/Mesh/Texture
              v
Imported lossless evidence
              v
Authored complete Effect
              v
WFX Component -> Assembly -> EffectCatalog.runtime.json
              v
Character body animation + one complete Effect playback
```

`inputSlot`은 표시 메타데이터다. 저장 identity와 재생 identity는 항상 skill/effect ID다. 따라서
슬롯 재배치가 일어나도 같은 원본 Effect가 다른 Effect로 변하지 않고, 예전 슬롯 문서가 현재 스킬을
대체하지도 않는다.

## 3. 현재 체험 모드 계약

| 슬롯 | skillId | body clip 정본 | Effect identity |
|---|---:|---|---|
| LMB | 2050010 | `battle_1_01~04` | `effect.dimensionmaster.skill.2050010` + BA1~BA4 |
| Q | 2050100 | `nailstrike_01` | `effect.dimensionmaster.skill.2050100` |
| W | 2050120 | `dimensionalleap_01~03` | `effect.dimensionmaster.skill.2050120` |
| E | 2050160 | `overslash_01~05` | `effect.dimensionmaster.skill.2050160` |
| R | 2050180 | `foldcut` | `effect.dimensionmaster.skill.2050180` |
| A | 2050210 | `willowrend` | `effect.dimensionmaster.skill.2050210` |
| S | 2050220 | `momentaryrift` | `effect.dimensionmaster.skill.2050220` |
| F | 2050230 | `chronorecoil` | `effect.dimensionmaster.skill.2050230` |
| D | 2050240 | `telekinesisthrust_01/04` | `effect.dimensionmaster.skill.2050240` |
| T | 2050500 | `dimensionprison` | `effect.dimensionmaster.skill.2050500` |
| V | 2050520 | `timewave` | `effect.dimensionmaster.skill.2050520` |
| ALT_V | 2050540 | `super_timewave` | `effect.dimensionmaster.skill.2050540` |

## 4. 구현 순서

### G01. Canonical admission과 stable identity

생성기는 하드코딩한 `EXPECTED_SLOTS`를 사용하지 않는다. `PlayerSkills.effectId`와 현재
skillbinding이 모두 존재하는 행만 제품 Effect로 승인한다. 추출 후보는 Data Files의 증거로 유지하되
All Effects와 Runtime Catalog를 대신하지 못한다.

Component stable ID와 물리 경로는 `effect.dimensionmaster.skill.<id>[.baN]`에서 파생한다. slot은
Component source metadata와 UI label에만 남긴다.

### G02. 현재 Action/Particle source 재생성

현재 body clip sequence로 원본 Action stage를 선택한다. 동일 action 안의 tripod/disabled stage를
전부 동시에 실행하지 않고, animnotify reference와 일치하는 stage만 실행한다. 원본 LOD Modules 배열의
외부 import reference는 원본 occurrence이므로 package 소유자가 달라도 배열 순서와 중복을 보존한다.

### G03. T Summon Model Cue

`PlaySkeletalMesh` source payload를 `MODEL_CUE` typed cue로 해독한다. 검증된 source skeletal mesh,
AnimSet, runtime WModel, runtime clip 매핑을 데이터 계약으로 결합한다. Action local position에는 UE 단위
`0.01`을 한 번만 적용하고 source scale을 그대로 보존한다. 현재 확인된 T 값은 위치
`[0, 0, 0.45]`, scale `[1.2, 1.2, 1.2]`, clip duration `131/30 = 4.366666...초`다.

`PlaySkeletalMeshMaterialParam`은 raw payload와 2.901422초 발생 시각을 보존하되 parameter 의미와
GPU consumer가 닫히기 전까지 실행 완료로 승격하지 않는다.

### G04. Animation cue 문서와 runtime 연결

`DimensionMaster.animevents`의 authored `effectref=asset` 행 중 현재 skillbindings에 없는 구형 clip/effect
행을 제거한다. loader를 느슨하게 만들어 오류를 숨기지 않는다. BA1~BA4와 이미 근거가 있는 current cue는
보존하고, 근거 없는 anchor 행을 새로 추측하지 않는다. 명시 cue가 없는 current skill은
`PlayerSkills.effectId`를 complete Effect root에서 한 번 시작하는 기존 fallback 경로를 사용한다.

### G05. Resource Index와 Tool 소비

`Data/Effects/ResourceIndex/DimensionMaster/<SLOT>/`에는 per-slot JSON index만 둔다. 기존
`Client/Bin/Resources/Effect/DimensionMaster` payload를 복제하지 않는다. index는 exact asset ID,
Material identity, Element/Emitter/Model Cue 역참조와 물리 존재 상태를 가진다.

All Effects의 기존 `Assembly -> Component -> Emitter -> Resources -> Modules` 트리는 전체/개별 Effect
선택과 source diagnosis를 담당한다. Effect Detail의 상단 slot은 선택 Element가 현재 사용하는 리소스를,
하단 Resource Library는 DimensionMaster 전체 후보를 보여준다. 별도 두 번째 Effect runtime은 만들지 않는다.

### G06. 재생성·게시·검증

```text
source receipt / normalized graph / module closure 생성
-> Imported 변환
-> canonical Authored materialize
-> WFX Component/Assembly 생성 및 identity verify
-> EffectCatalog publish validate/publish
-> per-slot Resource Index 재생성
-> Client project/filter Data 등록 동기화
-> Python/C++ harness
-> Debug build / Client smoke / ProjectAudit
```

## 5. 변경 파일 코드 정본

구현 위치, 함수별 책임, 생성 데이터 계약과 실행 검증 순서는 같은 날짜의
`2026-08-07_DIMENSIONMASTER_CANONICAL_EFFECT_EXECUTOR_CODE_PLAN.md`를 정본으로 삼는다.
실제 반영 코드는 다음 파일이며 중간 사본이나 별도 두 번째 runtime을 두지 않는다.

- admission/source: `build_dimensionmaster_base_effects.py`,
  `build_skill_effect_source_receipt.py`, `build_action_cue_recipe.py`
- materialize: `build_effect_source_material_contract.py`,
  `materialize_dimensionmaster_base_effects.py`
- hierarchy: `build_effect_components.py`
- resource index: `build_skill_effect_resource_index.py`
- publish/project: `Publish-Effects.ps1`, `Sync-EffectDataProject.ps1`
- runtime audit: `ClientFrontendHarness.cpp`, `Test-EffectToolFinal.ps1`,
  `Invoke-ProjectAudit.ps1`

각 신규 스크립트의 전체 코드는 해당 경로 자체가 정본이고, PLAN에는 복제본을 만들지 않는다.
최종 RESULT에는 실행한 명령과 실제 수치만 기록한다.

## 6. 수동 GPU 확인

자동 검증 뒤 Debug Client에서 다음만 판정한다.

1. All Effects의 `R`이 `2050180 / foldcut`이고 `2050190`이 아닌지 확인한다.
2. All Effects의 `T`가 `2050500 / dimensionprison`이며 Summon animation을 함께 재생하는지 확인한다.
3. 각 슬롯의 body clip과 complete Effect가 동일 action start에서 움직이는지 확인한다.
4. Resource/Material fallback, emitter 크기·방향·피벗 문제를 stable Component/Emitter/Profile ID로 기록한다.
5. 그 뒤에만 원작 PNG와 Material profile, Light/Camera/Post/Sound를 A/B 튜닝한다.
