# VALTAN_CROSS Effect Tool 구현 결과

## 완료 상태

- `VALTAN_CROSS`에 연결되는 Effect Tool 전용 `DRAFT_ATTACHED` Authored Effect를 추가했다.
- 이번 변경은 저작 초안이다. Product Effect cue, Server hit/damage, V2 binding은 변경하지 않았다.
- 다른 세션이 수정 중인 Action Composition Workbench와 `BOSS_VALTAN.effectv2bindings.json`은 건드리지 않았다.
- Client를 실행하거나 화면을 조작하지 않았다. 실제 크기, 간격, 지면 높이와 소멸 품질은 사용자의 Effect Tool 육안 판정이 남아 있다.

## 구현 데이터

- Effect ID: `effect.valtan.sequence.cross`
- Draft owner: `VALTAN_CROSS`
- WModel: `Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_001.wmodel`
- Base texture: `Effect/Valtan/Textures/FX_TEX_00/fx_a_environ_002.dds`
- Source material: `fx_m_mi_05.fx_mi.fx_e_me_ht_03_4_ma`
- Model import pre-scale: `0.01`
- Pattern impact anchor: `1.617초`

WModel과 texture/material 조합은 기존 Valtan authored `earthquake-smash.impact`와 `attack.swing` 석재 particle에서 확인한 조합을 사용했다. 물리 Resource pack에는 `fm_a_stone_002/004/010.wmodel`과 파쇄 보조용 `fm_d_stoneparts_003.wmodel`도 있으나, 이번 CROSS 1차 초안은 직접 근거가 가장 강한 `_001`만 사용한다.

## 네 방향 생성 계약

`360도`는 `0도`와 같은 방향이므로 Element를 다섯 개 만들지 않는다. 아래 네 Element가 십자 네 팔을 각각 소유한다.

| Element | 표시 각도 | 시작 위치 | Element 속도 |
|---|---:|---:|---:|
| `cross.rock-wave.arm.000` | 0 | `(1.5, 0.05, 0)` | `(6, 0, 0)` |
| `cross.rock-wave.arm.090` | 90 | `(0, 0.05, 1.5)` | `(0, 0, 6)` |
| `cross.rock-wave.arm.180` | 180 | `(-1.5, 0.05, 0)` | `(-6, 0, 0)` |
| `cross.rock-wave.arm.270` | 270 | `(0, 0.05, -1.5)` | `(0, 0, -6)` |

각 Element는 하나의 돌이 아니라 한 방향의 moving emitter다. Element root가 축 바깥쪽으로 이동하는 동안 `spawnRatePerSecond=10`, `burstCount=1`, `maxParticles=6`으로 돌을 낳는다. `particle.localSpace=false`이므로 이미 태어난 particle은 그 시점의 world birth root를 보존하고, emitter만 다음 생성 위치로 이동한다. particle 자체의 initial velocity와 acceleration은 0이라 생성된 돌이 뒤따라 이동하지 않는다.

- 생성 간격: `1 / rate = 0.1초`
- authored 공간 간격: `speed / rate = 6 / 10 = 0.6`
- emitter 방출/이동 시간: `0.55초`
- 축별 최대 돌: `6개`
- particle life: `0.65~0.8초`
- 소멸: `endSize=0`

`maxParticles`는 총 발생 횟수가 아니라 동시 active 상한이다. 방출 중 먼저 태어난 돌이 죽어 빈 slot이 다시 열리지 않도록 particle 최소 수명 `0.65초`를 emitter life `0.55초`보다 길게 유지했다. 60Hz fixed tick에서는 첫 burst와 rate spawn 사이 간격이 나머지보다 약 한 프레임 짧을 수 있다. 화면에서 첫 간격만 좁아 보이면 rate-only 방식 또는 첫 위치 보정으로 다음 iteration에서 조정한다.

네 방향은 원본 CROSS notify가 공간 transform까지 보존한 값이 아니라 현재 프로젝트 좌표계에 맞춘 `PROJECT_TUNED` 초안이다. 원본 `mesh_att_battle_2_01`에서 확정되는 것은 내려찍기 HIT 약 `1.600초`와 impact FX 약 `1.617초`다.

## Effect Tool 편집 지점

- `Transform > Position`: 첫 돌 반경과 지면 높이
- `Transform > Velocity`: 해당 축 전파 속도
- `Timing > Start Delay`: 내려찍기 정렬, 현재 `1.617`
- `Timing > Life Time`과 `Transform Motion Duration`: 방출·이동 window, 둘 다 `0.55`
- `Particle > Spawn Rate / Fixed Burst / Max Particles`: 돌 개수와 시간 간격
- `Particle > Life / Start Size / End Size`: 유지 시간, 크기, 소멸
- `Particle > Local Space`: 반드시 꺼진 상태 유지
- `Mesh Model`: `_001`을 다른 Valtan stone WModel로 교체 가능

Draft preview는 pattern t=0에서 Effect와 animation을 함께 시작하므로 내부 `Start Delay=1.617`을 사용한다. Product 승격 시에는 `stage 0ms + 내부 1.617초` 또는 `cue 1617ms + 내부 0초` 중 하나만 사용해야 한다. impact root snapshot도 같은 시점에 고정해 아직 생성되지 않은 돌이 이동 중인 boss를 따라가지 않게 한다.

## 변경 파일

- `.md/GB/08-31/2026-08-31_VALTAN_CROSS_EFFECT_TOOL_PLAN.md`
- `.md/GB/08-31/2026-08-31_VALTAN_CROSS_EFFECT_TOOL_RESULT.md`
- `Data/Effects/Authored/effect.valtan.sequence.cross.effect.json`
- `Data/Effects/ValtanPatternAuthoringEffects.json`
- `Tools/EffectPipeline/test_valtan_cross_rock_wave_effect.py`
- `Tools/EffectPipeline/test_valtan_pattern_authoring_effect_document.py`
- `Tools/EffectPipeline/test_effect_tool_valtan_all_effects_contract.py`

## 자동 검증

- `Validate-EffectSources.ps1 -AllowLocalResources`: PASS
- `test_valtan_cross_rock_wave_effect.py`: 4/4 PASS
- `test_valtan_pattern_authoring_effect_document.py`: 5/5 PASS
- `test_effect_tool_valtan_all_effects_contract.py`: 35/35 PASS
- `Project-ValtanPatternMaster.ps1 -Mode ValidateV2`: PASS, managed 34 / legacy 26 / projected artifacts 9
- `Test-ValtanPatternMaster.ps1`: 75개 중 74 PASS, 1 FAIL
  - 실패: clean worktree에 Git 비추적 runtime fixture `Server/Bin/DataFiles/World`가 없어 `test_world_publisher_rejects_static_ghost_and_preserves_runtime`의 사전 조건이 성립하지 않음
  - 이번 Effect 데이터나 CROSS contract 실패가 아니다.
- `git diff --check`: PASS

C++ 변경이 없으므로 Product build는 실행하지 않았다.

## 수동 검증과 다음 단계

사용자는 Effect Tool의 `All Effects -> Valtan -> VALTAN_CROSS(십자 돌 공격)`에서 `effect.valtan.sequence.cross` Draft를 열어 Pattern Play로 확인한다. 다음 항목은 아직 PASS로 기록하지 않는다.

1. 내려찍기 프레임과 첫 돌 생성 시점
2. 네 축의 중심 정렬과 boss 전방 의미
3. 돌 크기, 지면 관통, 생성 간격
4. 여섯 돌의 행이 너무 짧거나 긴지
5. `endSize=0` 소멸이 자연스러운지

육안 승인 뒤 Product cue와 impact snapshot root를 연결한다. Server의 CROSS hit/damage가 필요하면 별도 Server authority 수직 슬라이스로 추가하고 Effect mesh에서 damage 범위를 역산하지 않는다. 땅구르기 후 사자후의 네 방향 폭발 돌은 이번 CROSS의 단순 생성·소멸 emitter와 분리하여, 돌 발생과 폭발 payload를 가진 별도 Effect로 구현한다.
