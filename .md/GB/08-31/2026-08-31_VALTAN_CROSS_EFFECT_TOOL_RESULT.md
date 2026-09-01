# VALTAN_CROSS Effect Tool 구현 결과

## 완료 상태

- `effect.valtan.sequence.cross`를 direct-authored Product Effect로 등록하고 `VALTAN_CROSS/STEP_01` cue에 연결했다.
- draft sidecar 행은 제거했다. Server hit/damage와 Effect V2 binding은 추가하지 않았다.
- 다른 세션이 수정 중인 Action Composition Workbench와 `BOSS_VALTAN.effectv2bindings.json`은 건드리지 않았다.
- Client를 실행하거나 화면을 조작하지 않았다. 실제 크기, 간격, 지면 높이와 소멸 품질은 사용자의 Effect Tool 육안 판정이 남아 있다.

## 구현 데이터

- Effect ID: `effect.valtan.sequence.cross`
- Product owner: `VALTAN_CROSS/STEP_01`, cue `cue.valtan.sequence.cross.step-01`
- WModel: `Effect/Valtan/Meshes/FX_SM_00/fm_d_stoneparts_003.wmodel`
- Base texture: `Effect/Valtan/Textures/FX_TEX_02/fx_d_fluid_020.dds`
- Noise texture: `Effect/Valtan/Textures/FX_TEX_02/fx_d_stoneparts_002.dds`
- Mask texture: `Effect/Valtan/Textures/FX_TEX_04/fx_h_noise_001.dds`
- Source material evidence: `fx_m_mi_n_00.fx_mi.fx_n_me_dissolve_01_04_ma`
- Project render profile: `opaque_back_depth_write`
- Project tint: `(0.10, 1.40, 1.70, 1.00)`
- Model import pre-scale: `0.01`
- Pattern impact anchor: `1.617초`

WModel과 세 texture/material 조합은 기존 Valtan authored `attack.swing`의 `particlespriteemitter_44`에서 확인했다. 원본 material 실행식은 현재 generic `effect.standard`로 완전히 복원되지 않았으므로, 낮은 Base/Mask alpha와 manual particle 수명 alpha가 겹쳐 반투명 먼지처럼 보이던 상태를 그대로 두지 않았다. CROSS는 `opaque_back_depth_write`를 명시하고 원본 흑백 패턴에 청록 tint를 적용하는 `PROJECT_TUNED` 표현을 사용한다. 실험 중 추가된 standalone Mesh와 단발 Mesh Particle은 정식 네 축 계약에서 제거했다.

## 네 방향 생성 계약

`360도`는 `0도`와 같은 방향이므로 Element를 다섯 개 만들지 않는다. 아래 네 Element가 십자 네 팔을 각각 소유한다.

| Element | 표시 각도 | 시작 위치 | Element 속도 |
|---|---:|---:|---:|
| `cross.rock-wave.arm.000` | 0 | `(1.5, 0.05, 0)` | `(6, 0, 0)` |
| `cross.rock-wave.arm.090` | 90 | `(0, 0.05, 1.5)` | `(0, 0, 6)` |
| `cross.rock-wave.arm.180` | 180 | `(-1.5, 0.05, 0)` | `(-6, 0, 0)` |
| `cross.rock-wave.arm.270` | 270 | `(0, 0.05, -1.5)` | `(0, 0, -6)` |

각 Element는 하나의 돌이 아니라 한 방향의 moving emitter다. Element root가 축 바깥쪽으로 이동하는 동안 `spawnRatePerSecond=20`, `burstCount=0`, `maxParticles=10`으로 돌을 낳는다. `particle.localSpace=false`이므로 이미 태어난 particle은 그 시점의 world birth root를 보존하고, emitter만 다음 생성 위치로 이동한다. particle 자체의 initial velocity와 acceleration은 0이라 생성된 돌이 뒤따라 이동하지 않는다.

- 생성 간격: `1 / rate = 0.05초`
- authored 공간 간격: `speed / rate = 6 / 20 = 0.3`
- emitter 방출/이동 시간: `0.50초`
- 축별 최대 돌: `10개`
- particle life: `0.65~0.8초`
- 크기: `modelPreScale=0.01`, `startSize=endSize=(1, 1)`
- 소멸: 크기 축소와 dissolve 없이 particle life 종료 시 제거

`maxParticles`는 총 발생 횟수가 아니라 동시 active 상한이다. particle 최소 수명 `0.65초`를 emitter life `0.50초`보다 길게 유지해 방출 중 slot이 다시 열리지 않게 했다. 60Hz fixed tick에서는 rate accumulator가 step마다 `20 / 60 = 1/3`씩 증가한다. t=0 burst가 없으므로 step `3, 6, ..., 30`, 즉 `50, 100, ..., 500ms`에 정확히 10개가 생긴다. `GAMEPLAY_FOOTPRINT` scale 1.5 적용 후 birth 반경은 `2.70, 3.15, ..., 6.75m`다.

네 방향은 원본 CROSS notify가 공간 transform까지 보존한 값이 아니라 현재 프로젝트 좌표계에 맞춘 `PROJECT_TUNED` 초안이다. 원본 `mesh_att_battle_2_01`에서 확정되는 것은 내려찍기 HIT 약 `1.600초`와 impact FX 약 `1.617초`다.

## Effect Tool 편집 지점

- `Transform > Position`: 첫 돌 반경과 지면 높이
- `Transform > Velocity`: 해당 축 전파 속도
- `Timing > Start Delay`: Product cue가 지연을 소유하므로 `0`
- `Timing > Life Time`과 `Transform Motion Duration`: 방출·이동 window, 둘 다 `0.50`
- `Particle > Spawn Rate / Fixed Burst / Max Particles`: 돌 개수와 시간 간격
- `Particle > Life / Start Size / End Size`: 유지 시간과 크기. 현재 Start/End는 모두 `1`
- `Particle > Local Space`: 반드시 꺼진 상태 유지
- `Mesh Model`: 네 축 모두 `fm_d_stoneparts_003.wmodel`
- `Material > Render Profile`: 검은 돌 본체의 불투명도를 보존하려면 `opaque_back_depth_write`
- `Color Multiply`: RGB를 모두 낮추면 청록까지 사라진다. 현재 `(0.10, 1.40, 1.70, 1.00)`이 검정/청록 기준선이다.

Product cue가 `sourceStartMs=1617`을 소유하고 Effect 내부 `Start Delay=0`을 사용한다. cue는 boss `root`를 `snapshot`하고 `once/natural`로 재생하므로 아직 생성되지 않은 돌도 이동 중인 boss를 따라가지 않는다.

## 변경 파일

- `.md/GB/08-31/2026-08-31_VALTAN_CROSS_EFFECT_TOOL_PLAN.md`
- `.md/GB/08-31/2026-08-31_VALTAN_CROSS_EFFECT_TOOL_RESULT.md`
- `Data/Effects/Authored/effect.valtan.sequence.cross.effect.json`
- `Data/Effects/EffectCatalog.json`
- `Data/Effects/ValtanPatternAuthoringEffects.json`
- `Data/Valtan/Valtan.presentation.json`
- `Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json` (V2 publisher 생성물)
- `Tools/EffectPipeline/test_valtan_cross_rock_wave_effect.py`
- `Tools/EffectPipeline/test_valtan_pattern_authoring_effect_document.py`
- `Tools/EffectPipeline/test_effect_tool_valtan_all_effects_contract.py`
- `Tools/Build/Invoke-BuildAndRegression.ps1`
- `Tools/Build/test_build_profile_contract.py`

## 자동 검증

- JSON parse: PASS
- `Project-ValtanPatternMaster.ps1 -Mode PublishV2`: PASS, changed 1 / artifacts 7
- `Project-ValtanPatternMaster.ps1 -Mode Validate`: PASS, managed 38 / legacy 25 / projected artifacts 9 / combat objects 4
- `Validate-EffectSources.ps1 -AllowLocalResources`: PASS, direct sources 177 / unbound references 0
- stoneparts 전환 후 CROSS + All Effects focused unittest: 41/41 PASS
- CROSS/sidecar/combat-lifetime focused unittest: 24/24 PASS
- `test_effect_tool_valtan_all_effects_contract.py`: 37/37 PASS
- `test_validate_effect_sources.py`: 42/42 PASS
- `test_build_profile_contract.py`: PASS; CROSS exact contract를 FullDiagnostic gate에 등록
- `git diff --check`: PASS

C++ 변경이 없으므로 Product build는 실행하지 않았다.

## 수동 검증과 다음 단계

사용자는 Effect Tool의 `All Effects -> Valtan -> VALTAN_CROSS(십자 돌 공격)`에서 Product cue `effect.valtan.sequence.cross`를 선택하고 Pattern Play로 확인한다. 다음 항목은 아직 PASS로 기록하지 않는다.

1. 내려찍기 프레임과 첫 돌 생성 시점
2. 네 축의 중심 정렬과 boss 전방 의미
3. 돌 크기, 지면 관통, 생성 간격
4. 축마다 열 돌의 행이 너무 짧거나 긴지
5. 크기 축소 없이 particle life 종료 시 제거되는 시점이 자연스러운지

Product cue와 impact snapshot root 연결은 완료했다. 사용자 육안 승인 뒤 크기·간격·지면 높이만 authored Effect에서 조정한다. Server의 CROSS hit/damage가 필요하면 별도 Server authority 수직 슬라이스로 추가하고 Effect mesh에서 damage 범위를 역산하지 않는다. 땅구르기 후 사자후의 네 방향 폭발 돌은 이번 CROSS의 단순 생성·소멸 emitter와 분리하여, 돌 발생과 폭발 payload를 가진 별도 Effect로 구현한다.

## 2026-09-01 사용자 확정본 Dissolve 갱신

- 사용자가 Effect Tool에서 확정한 네 Mesh Particle의 모델, transform, scale, 속도, `1.5` world 고정 간격, 검정/청록 material 조합은 보존했다.
- 네 Element 모두 `dissolve` 슬롯에 `Effect/Valtan/Textures/FX_TEX_04/fx_h_noise_001.dds`를 연결했다.
- `detail.timing.dissolveStartNormalized`를 `0.65`로 설정했다. 현재 particle lifetime `2.0초` 기준 각 돌이 생성된 뒤 `1.3초`부터 마지막 `0.7초` 동안 dissolve된다.
- render profile은 `opaque_back_depth_write`를 유지한다. 공용 Effect shader가 dissolve R 채널과 수명 진행률로 `clip()`하므로 alpha pass나 CROSS 전용 shader/pass를 추가하지 않았다.
- 이 값은 각 particle의 개별 lifetime 기준이다. 네 방향의 돌은 생성 순서를 유지한 채 순차적으로 소멸하며, 모든 돌의 절대 시각 동시 소멸 계약은 아니다.
- `Validate-EffectSources.ps1`: PASS, direct sources `177`, unbound references `0`.
- `python -m unittest Tools.EffectPipeline.test_valtan_cross_rock_wave_effect`: PASS, `5/5`.
- 최종 dissolve의 화면 형태와 체감 속도는 사용자가 새로 로드한 Effect Tool preview에서 판정한다.
