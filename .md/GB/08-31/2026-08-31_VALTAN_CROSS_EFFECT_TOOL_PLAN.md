# VALTAN_CROSS Effect Tool 구현 계획

## 목표

- `VALTAN_CROSS`의 `mesh_att_battle_2_01` 내려찍기 직후 보스 기준 네 축으로 석재가 근거리부터 원거리까지 순차 생성되고 자연 소멸하는 편집 가능한 Authored Effect를 추가한다.
- 현재 Effect v13의 Mesh Particle과 world-space birth 계약을 확장해, 기존 문서는 그대로 재생되고 opt-in 행만 균일한 중심 간격을 쓰게 한다.
- `VALTAN_CROSS/STEP_01` Product cue가 이 Effect를 boss root snapshot으로 한 번 재생하도록 승격한다.

## 정본과 경계

- Pattern/model clock 정본: `Data/Valtan/Valtan.presentation.json`
- Product Effect 등록 정본: `Data/Effects/EffectCatalog.json`
- Effect Tool draft sidecar: `Data/Effects/ValtanPatternAuthoringEffects.json`에서 승격된 CROSS 행을 제거한다.
- Effect 본문 정본: `Data/Effects/Authored/effect.valtan.sequence.cross.effect.json`
- 석재 WModel: `Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_001.wmodel`
- 석재 source material: `fx_m_mi_05.fx_mi.fx_e_me_ht_03_4_ma`
- Product cue 시작점: source notify impact `1617ms`
- Effect 내부 시작 지연: `0ms`; cue와 내부 지연을 동시에 적용하지 않는다.
- 완료 상태: `VALTAN_CROSS/STEP_01 -> cue.valtan.sequence.cross.step-01 -> effect.valtan.sequence.cross`
- Server damage는 이번 변경 범위가 아니다. 현재 `VALTAN_CROSS`의 `NONE` hit를 Effect mesh에서 역산하지 않는다.

## 구현 구조

1. `0/90/180/270` 네 방향마다 Mesh Particle Element 하나를 둔다. `360`은 `0`과 중복이므로 만들지 않는다.
2. 각 emitter의 authored local 값은 반경 1.5, 속도 6m/s다. `GAMEPLAY_FOOTPRINT` world scale 1.5를 적용한 실제 birth 반경은 2.70m부터 6.75m까지다.
3. t=0 burst 없이 초당 20개를 0.50초 동안 방출하고 최대 active를 10으로 둔다. 60Hz fixed-step accumulator는 3 step마다 하나를 생성하므로 정확히 50ms 간격, step 3~30의 10개 birth를 만든다.
4. `particle.localSpace=false`로 저장하여 이미 태어난 돌은 emitter를 따라가지 않고 태어난 world pose에 남는다.
5. particle lifetime과 end size로 각 돌이 개별적으로 소멸한다.
6. Product cue `sourceStartMs=1617`만 impact 지연을 소유한다. 네 emitter의 `Start Delay=0`, cue `followPolicy=snapshot`, `repeatPolicy=once`, `stopPolicy=natural`로 이중 지연과 이동 boss 추종을 막는다.
7. optional `particle.fixedCenterSpacingWorldUnits`가 `0`보다 크면 direct-authored world-space POINT particle만 이를 사용한다. 첫 birth의 world origin과 직선 이동 방향을 고정하고 이후 birth root를 순번 × 간격에 배치한다. 이 모드에서 `Initial Position Min/Max`는 모두 `0`이어야 하며, `0` 또는 필드 누락은 기존 rate/root-motion 재생을 보존한다.

## Effect Tool 편집 축

- Element Transform Position: 첫 생성 반경
- Element Transform Velocity: 파동 전파 방향과 속도
- Timing Life Time: emitter 이동/방출 시간
- Particle Spawn Rate / Fixed Burst / Max Particles: 축별 돌 개수와 간격
- Particle Fixed Center Spacing: 돌 중심 사이의 고정 world 간격. `0`은 기존 방식이며, 양수 편집 시 랜덤 Initial Position 범위를 자동으로 `0`으로 정리한다.
- Particle Life Min/Max / Start Size / End Size: 돌 유지·소멸과 크기
- WModel slot: 석재 외형 교체
- Particle Local Space: 반드시 끔

## 검증

- JSON parse와 Effect source validator
- `VALTAN_CROSS` cue 및 네 emitter topology 실행형 contract test
- 필드 누락/0의 기존 동작, 양수 간격의 직선 world-space 배치, 잘못된 local-space/random-position 조합 거부
- Valtan split source validation과 Product projection
- `git diff --check`
- 최종 크기·간격·방향은 사용자가 Effect Tool에서 직접 재생해 판정한다.
