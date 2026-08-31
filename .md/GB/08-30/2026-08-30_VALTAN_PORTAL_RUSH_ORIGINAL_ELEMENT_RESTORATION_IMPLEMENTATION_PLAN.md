# 발탄 Portal Rush 원작 Element 복원 구현 계획서

## 목표

`VALTAN_PORTAL_RUSH / PORTAL` Product에서 삭제된 원작 Portal Element 13개를 복원한다.
현재 남아 있는 원작 Element 1개와 합쳐 Git 이력상 검증된 14개 구성을 되살리고,
Effect Tool의 기존 catalog/cue/animation 연결로 다시 재생할 수 있게 한다.

현재 Anim Bench 세션이 수정 중인 Pattern restart, target anchor, sequencer와 Server motion 파일은
건드리지 않는다. 별도 복제본인 `effect.valtan.project-tuned.sequence.warp.portal`도 이번 변경 범위가 아니다.

## 현재 실측과 복원 근거

- 대상 asset은 `effect.valtan.carrier-v1.attack.portal-rush.portal.clip-01`이다.
- `3dd2a3e0`은 원작
  `FX_MN_RPBF_00_N.Par_N_RPBF_Potal_02_01/02` source occurrence를 실행 가능한 Element 14개로
  materialize한 마지막 정상 carrier다.
- `f82489a9`에서 13개가 삭제돼 현재는 `source.db4398c6dbd27ca0622f` 하나만 남아 있다.
- 정상본은 sprite 11개와 mesh 3개이며, 원작 group `_02_01` 9개와 `_02_02` 5개로 구성된다.
- 정상본이 참조하는 고유 DDS/WModel 12개는 현재 Drive 기반 `Client/Bin/Resources`에 모두 존재한다.
- `EffectCatalog.json`의 exact authored row, `Valtan.patterneffectcues.json`의
  `VALTAN_PORTAL_RUSH / PORTAL` Product cue와 `Valtan.patternbindings.json`의
  `mesh_att_battle_18_01` clip 연결은 이미 유지돼 있다.

퇴역한 Imported candidate나 generator를 되살리지 않는다. 현재 authored JSON 하나에 삭제된 원작
Element만 복원하고 현행 catalog/cue/runtime 경로가 그대로 소비하게 한다.

## G01. 원작 Portal Element 복원

### 수정 파일

```text
Data/Effects/Authored/effect.valtan.carrier-v1.attack.portal-rush.portal.clip-01.effect.json
```

복원 뒤 exact Element 순서는 다음과 같다.

```text
source.1b776620aaf4f38756f2
source.1c9d5fc0aa230299b12a
source.215a59ea8258f7e63813
source.2d9b128f179cfb10a0ef
source.516ebd8ab92481cddfb9
source.542d45705d0ab10e26c4
source.8bdb145e59f215ec3ca4
source.905f8d81b798a5ba69d0
source.b34062a699ad2c3749ec
source.baf3b1214e15af4bdd50
source.c15e35d8637a09e93cd6
source.c8364a880f2d2bac929b
source.cb18c0a3eb4715d141e5
source.db4398c6dbd27ca0622f
```

모두 `visible=true`, `kind=particle`, `sourceRecipe.enabled=true`인 원작 source row를 보존한다.
catalog, pattern cue, animation binding, local transform과 Pattern runtime clock은 수정하지 않는다.

## G02. 재삭제 방지 회귀 계약

### 신규 파일

```text
Tools/EffectPipeline/test_valtan_portal_rush_original_elements.py
```

focused test는 다음을 검증한다.

- catalog가 exact asset을 exact authored path로 resolve한다.
- Product cue가 `VALTAN_PORTAL_RUSH / PORTAL`과 exact asset을 연결하고, animation binding이
  `mesh_att_battle_18_01` clip occurrence를 제공한다.
- schema/version/effectAssetId와 Element ID/sourceNode가 중복 없이 exact 14개다.
- 원작 group 분포, sprite/mesh 분포, timing과 source recipe가 유지된다.
- 모든 Element가 material template과 하나 이상의 Resources-relative resource를 가진다.
- 절대 경로, drive-qualified 경로, 역슬래시 또는 `..` 탈출 resource ID를 거부한다.

## G03. 자동 검증

```powershell
python -B Tools/EffectPipeline/test_valtan_portal_rush_original_elements.py
python -B Tools/EffectPipeline/test_effect_tool_valtan_all_effects_contract.py
python -B Tools/EffectPipeline/test_effect_tool_valtan_saved_rows.py
powershell -ExecutionPolicy Bypass `
  -File Tools/EffectPipeline/Validate-EffectSources.ps1 `
  -RepositoryRoot . `
  -ResourceRoot Client/Bin/Resources
git diff --check
```

JSON parse, catalog/cue projection과 referenced DDS/WModel 실물을 함께 확인한다. Client/UI는 에이전트가
실행하지 않는다.

## G04. 사용자 수동 확인

```text
F1
→ Effect Tool
→ All Effects
→ Character / Boss: Valtan
→ VALTAN_PORTAL_RUSH
→ PORTAL
→ [PRODUCT] effect.valtan.carrier-v1.attack.portal-rush.portal.clip-01
→ Open Editor
→ Elements 14개 확인
→ Play Effect + Animation
```

자동 검증 완료는 document/cue/resource/tool projection 준비 완료를 뜻한다. 원작과 같은 pixel fidelity와
위치·방향은 사용자 확인 전 PASS로 기록하지 않는다.
