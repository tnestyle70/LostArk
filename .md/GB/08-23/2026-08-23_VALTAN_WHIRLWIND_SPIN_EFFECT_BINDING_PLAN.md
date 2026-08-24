# 2026-08-23 Valtan Whirlwind SPIN Effect·Trail·Slot 계획

## 목표

`VALTAN_WHIRLWIND`를 실제 제품 stage 순서인 `WINDUP -> SPIN -> RECOVERY`로 재생하고,
SPIN을 정확히 `1400 ms` 동안 두 바퀴 도는 animation/effect occurrence로 고정한다.
사용자가 만든 청록/검정 Mesh Particle과 도끼 Trail은 같은 SPIN occurrence에서 재생하며,
순차 재생뿐 아니라 Effect Tool timeline scrub에서도 도끼의 과거 궤적이 복원되어야 한다.

Effect Tool에는 사용자가 실수로 직접 연결한 선택적 DDS 슬롯을 실제로 삭제하는 기능을 추가한다.
compiler/source-owned 슬롯은 삭제하지 않고 기존 Reset to Source 계약을 유지한다.

## 현재 정본

- pattern: `VALTAN_WHIRLWIND`
- stage: `WINDUP 1333 ms -> SPIN 1400 ms -> RECOVERY 1467 ms`
- SPIN action: `valtan.attack.whirlwind.active`
- SPIN occurrence: `valtan.attack.whirlwind.active.clip.01`
- source clip: `mesh_att_battle_20_03`, 실제 길이 `0.533333 s`
- authored playback: `playMs 0`, `playRate 0.761904762`, `loop true`
- SPIN damage: 시작 지연 `0 ms`, `350 ms` 간격 4회
- Product Effect ID: `effect.valtan.carrier-v1.attack.whirlwind.recovery.clip-01`
- Product cue ID: `cue.valtan.carrier-v1.attack.whirlwind.recovery.clip-01`

Effect/cue의 `recovery` 문자열은 이미 저장 계약에 사용 중인 stable ID이므로 이름 변경 대상으로
취급하지 않는다. 화면 표시 이름만 `VALTAN_WHIRLWIND / SPIN / carrier V1`로 교정한다.

## G01. SPIN animation과 pattern timing

수정 파일:

- `Data/Encounters/Valtan/ValtanEncounter.json`
- `Data/Animation/Authored/Valtan/Valtan.patternbindings.json`

SPIN stage를 `1400 ms`로 줄이고 4회 damage interval을 `350 ms`로 맞춘다. 실제 `0.533333 s`
source clip은 `playRate 0.761904762`와 explicit loop를 사용해 wall clock 기준 `0.7 s`마다 반복하고,
SPIN 한 번에 정확히 두 바퀴를 재생한다. `sourceEndMs: 1400`처럼 source clip 범위를 wall duration으로
오인하는 값을 만들지 않는다.

## G02. Product cue stage window

수정 파일:

- `Client/Public/ValtanPatternTree.h`
- `Client/Private/ValtanPatternTree.cpp`
- `Client/Private/Effect_Tool.cpp`
- `Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json`

Product cue view에 encounter가 resolve한 `stageDurationMs`를 전달한다. Effect Tool의 Product Play는
animation loop나 natural Effect tail이 더 길어도 timeline/visibility를 SPIN stage 끝 `1400 ms`에서
정확히 닫는다.

사용자 carrier cue는 natural source 수명을 `1.0666667 s` 이내로 맞춘다. 기존 sealed
`effect.valtan.pattern.420633.active` cue의 identity/policy는 이 수동 carrier 튜닝 범위에서 바꾸지 않는다.

## G03. Trail follow, scrub, smoothing

수정 파일:

- `Client/Public/Effect_Tool.h`
- `Client/Private/Effect_Tool.cpp`
- `Client/Private/Effect_DocumentRenderer.cpp`
- `Data/Effects/Authored/effect.valtan.carrier-v1.attack.whirlwind.recovery.clip-01.effect.json`
- `Data/Effects/Authored/effect.valtan.carrier-v1.attack.whirlwind.recovery.clip-01.v1.unified.effect.json`

`whirlwind.trail.20.axe.main`은 Product cue root placement와 별개로 Element-local `b_wp_r_01`
follow를 소유한다. 순차 재생, loop wrap, Sample Time scrub, preview restore에서 source clip의 과거
bone pose를 동기식으로 재생하고 `CEffectObject::Set_SampleTimeWithTransformHistory`에 전달한다.

일반 authored Trail에는 다음 수동 축을 노출하고 renderer가 실제 소비한다.

- `Trail UV Repeat Distance`: 누적 world distance 기준 U 좌표
- `Trail Curve Step`: segment 거리 tessellation 간격

두 값이 `0`이면 기존 동작을 보존한다. Whirlwind Trail은 `maxPoints 128`, sample interval
`1/120 s`, minimum distance `0.0025`, UV repeat distance `0.35`, curve step `0.025`를 사용한다.
Trail과 particle의 source end는 약 `1.0666667 s`다. `playRate 0.761904762`를 통과한 wall end는
`1.4 s`다. Debug V1 alias도 표시 이름과 최대 source 수명을 같은 SPIN 경계로 맞춘다.

## G04. 선택적 authored 슬롯 삭제와 잘못 연결한 Emissive 정리

수정 파일:

- `Client/Private/Effect_Tool.cpp`
- `Data/Effects/Authored/effect.valtan.carrier-v1.attack.four-slash.active.clip-01.effect.json`

직접 저작한 Element에서 Base와 Mesh가 아닌 선택적 resource binding을 고르면
`Delete Selected Slot`을 표시한다. 다음 소유권이 있으면 새 빈 슬롯 생성과 삭제를 모두 거부하고
기존 source override/reset 계약만 사용한다.

- compiler material lane
- source material texture/profile/recipe/presentation
- material execution
- imported/runtime adapter packet

삭제는 staged document에서 정확한 binding 한 개를 erase한 뒤 transactional commit하며,
`Save Changes`를 해야 파일에 영구 반영된다는 상태 메시지를 보여준다.

사용자가 실수로 Emissive를 연결한 다음 두 Element는 Base-only로 되돌린다.

- `valtan.clip01.hit-spark.01`: `fx_h_hit_01.dds`
- `impact.fragments.hit_007`: `fx_a_hit_007.dds`

## 검증

```powershell
python -B -m unittest `
  Tools.EffectPipeline.test_effect_tool_valtan_saved_rows.EffectToolValtanSavedRowsTests.test_effect_detail_has_one_working_owner_per_manual_tuning_axis `
  Tools.EffectPipeline.test_effect_tool_valtan_saved_rows.EffectToolValtanSavedRowsTests.test_whirlwind_carrier_v1_plays_on_spin_clip `
  Tools.EffectPipeline.test_effect_tool_valtan_saved_rows.EffectToolValtanSavedRowsTests.test_optional_authored_slot_delete_and_hit_base_only_contract `
  Tools.EffectPipeline.test_effect_tool_valtan_saved_rows.EffectToolValtanSavedRowsTests.test_whirlwind_trail_uses_element_local_axe_follow

powershell -NoProfile -ExecutionPolicy Bypass -File `
  Tools/EffectPipeline/Publish-Effects.ps1 -Mode Validate `
  -ResourceRoot Client/Bin/Resources

powershell -NoProfile -ExecutionPolicy Bypass -File `
  Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Validate

MSBuild.exe Client/Default/Client.vcxproj /t:Build `
  /p:Configuration=Debug /p:Platform=x64 /m /nologo

git diff --check
```

화면 결과는 사용자가 새 Client process에서 Valtan Effect Tool을 열어 직접 판정한다. 자동 검증은
stage/clip/cue/resource 소유권과 build 계약까지만 증명하고 visual fidelity PASS를 대신 선언하지 않는다.
