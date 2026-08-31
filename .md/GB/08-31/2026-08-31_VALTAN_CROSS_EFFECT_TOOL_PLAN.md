# VALTAN_CROSS Effect Tool 구현 계획

## 목표

- `VALTAN_CROSS`의 `mesh_att_battle_2_01` 내려찍기 직후 보스 기준 네 축으로 석재가 근거리부터 원거리까지 순차 생성되고 자연 소멸하는 편집 가능한 Authored Effect를 추가한다.
- 제품 런타임의 새 반복 스키마를 만들지 않고 현재 Effect v13의 Mesh Particle, world-space birth, Element root motion 계약을 재사용한다.
- 다른 세션이 수정 중인 Action Composition Workbench와 V2 binding 파일은 변경하지 않는다.

## 정본과 경계

- Pattern/model clock 정본: `Data/Valtan/Valtan.presentation.json`
- Effect Tool draft ownership: `Data/Effects/ValtanPatternAuthoringEffects.json`
- Effect 본문 정본: `Data/Effects/Authored/effect.valtan.sequence.cross.effect.json`
- 석재 WModel: `Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_001.wmodel`
- 석재 source material: `fx_m_mi_05.fx_mi.fx_e_me_ht_03_4_ma`
- Draft Effect 내부 시작점: source notify impact `1617ms`
- 이번 1차 상태: `VALTAN_CROSS -> effect.valtan.sequence.cross -> DRAFT_ATTACHED`
- Server damage는 이번 변경 범위가 아니다. 현재 `VALTAN_CROSS`의 `NONE` hit를 Effect mesh에서 역산하지 않는다.

## 구현 구조

1. `0/90/180/270` 네 방향마다 Mesh Particle Element 하나를 둔다. `360`은 `0`과 중복이므로 만들지 않는다.
2. 각 emitter의 authored local 값은 반경 1.5, 속도 6m/s다. Pattern의 기존 `GAMEPLAY_FOOTPRINT` 승격 시 1.5 world scale을 적용하면 약 2.25m에서 시작해 약 6.75m까지 전파된다.
3. 각 emitter는 시작 시 하나를 burst하고 이후 초당 10개를 방출한다. 0.55초 emission window와 최대 active 6개로 축마다 정확히 6개가 약 2.25~6.75m 구간을 순차 채운다.
4. `particle.localSpace=false`로 저장하여 이미 태어난 돌은 emitter를 따라가지 않고 태어난 world pose에 남는다.
5. particle lifetime과 end size로 각 돌이 개별적으로 소멸한다.
6. Draft preview는 Effect를 Pattern t=0부터 재생하므로 네 emitter의 `Start Delay`를 1.617초로 둔다. 제품 승격 때는 `stage 0ms + 내부 1.617초` 또는 `cue 1617ms + 내부 0초` 중 하나만 선택하여 이중 지연을 막고, impact 시점 boss root를 snapshot한다.

## Effect Tool 편집 축

- Element Transform Position: 첫 생성 반경
- Element Transform Velocity: 파동 전파 방향과 속도
- Timing Life Time: emitter 이동/방출 시간
- Particle Spawn Rate / Fixed Burst / Max Particles: 축별 돌 개수와 간격
- Particle Life Min/Max / Start Size / End Size: 돌 유지·소멸과 크기
- WModel slot: 석재 외형 교체
- Particle Local Space: 반드시 끔

## 검증

- JSON parse와 Effect source validator
- `VALTAN_CROSS` cue 및 네 emitter topology 실행형 contract test
- Valtan split source validation과 Product projection
- `git diff --check`
- 최종 크기·간격·방향은 사용자가 Effect Tool에서 직접 재생해 판정한다.
