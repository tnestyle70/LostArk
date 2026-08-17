# Effect Cascade Material Family Reconstruction 구현 계획

## 0. 목표와 현재 범위

차원술사 W(`skillId=2050120`) clip3에서 사용자가 직접 핵심 Element만 남긴
현재 저장본을 기준으로, 큰 사각형으로 보이는 Sprite Particle을 원본 부모
Material family 단위로 분류한다. 이번 구현 단위에서는 `Glasshole`, `FluidNinja`,
`CustomParticle`, `Crackhole` 네 family와 기존 `Slice`를 source-exact가 아닌
opt-in `family-lite` evaluator로 W의 대표 6개 occurrence에 먼저 적용한다. 사용자가
대표 occurrence의 first pixel과 실루엣을 승인한 family만 동일한 source identity와
실행 계약을 만족하는 다른 스킬 occurrence로 확장한다.

현재 기준선은 다음과 같다.

- visible Element 21개: Sprite Particle 16개, Mesh Particle 5개
- 이미 잘 보이는 `fragment` 계열은 변경하지 않는다.
- `voronoi`, `flownoise`, `dustparticle`, `caustic`을 같은 Sprite 문제로 묶지 않는다.
- 전역 Sprite 크기 배율이나 전역 alpha threshold를 추가하지 않는다.
- 사용자가 `445823...` Slice occurrence에 저장한 Source Trim
  `Count 1.43 / Size 0.25 / Life 1.65`를 모두 보존한다. family projector는
  크기를 소유하거나 `1.00`으로 되돌리지 않는다.
- 공유 worktree의 SpriteWave opcode/HLSL/materializer WIP는 이번 수동 분류 작업에서
  다시 생성하거나 일괄 적용하지 않는다.

첫 종료 조건은 stripped graph의 식을 추측해 `source-exact`로 선언하는 것이 아니다.
원본 named texture/scalar/vector evidence를 보존하면서, 대표 occurrence가 항상
그려지는 단순 cutout/envelope를 제공하는 것이다. `Color Clip`, Source Trim Size,
emissive/color를 사용자가 occurrence별로 조절해 V0를 만든다. Product admission은
필수 role이 닫히지 않으면 계속 fail-closed하고, 시각 확장은 사용자 승인 뒤에만 한다.

이번 실패는 Material family라는 분류 단위의 실패가 아니다. stripped graph에서
추측한 수식을 first-pixel 확인 전에 여러 occurrence로 승격한 순서의 실패다. 따라서
W 6개 대표본과 Claude 세션의 상위 `fx_mm_simple_01_ad` 대표본은 같은
`대표 1개 확인 -> 수치 튜닝 -> 동일 family 확장` 순서를 사용하되 서로의 데이터를
자동으로 덮어쓰지 않는다.

현재 corpus 분모는 다음과 같이 분리한다.

- 전체 Authored: 332문서, 8,137 Element
- 현재 Product direct-authored: 101문서, 2,945 Element
- Product source profile: 2,878개, 이 중 grouped-translucent 2,532개
- Product source parent 169종, grouped가 남은 parent 144종

parent 이름은 후보 family를 결정하지만 실행 승격은
`parent + renderer shape + blend + required named-role closure`를 모두 만족해야 한다.

## 1. 확정된 크기 계약

SourceRecipe Sprite의 최종 쿼드 크기는 다음 순서로 계산된다.

```text
raw Cascade StartSize(cm)
  * 0.01 (Client meter 변환, 정확히 한 번)
  * Detail.Particle.SourceScale.Size (사용자 Source Trim)
  * SizeOverLife 계열 모듈
  * Element/Root scale
```

`Effect_Playback.cpp`의 `UE3_StartSizeToClient()`가 Sprite의 raw cm를 `0.01`로
변환하고, Source Trim은 Source module 평가가 끝난 `vBaseSize`에 한 번 더 곱한다.
Sprite에는 Mesh의 `modelPreScale`이 적용되지 않는다. 따라서 현재 증상은 전역
10배/100배 변환 버그가 아니다.

대표 원본 크기는 다음과 같다.

| 계열 | raw Cascade | Client source 크기 | 현재 Trim |
|---|---:|---:|---:|
| Fragment 001 | `0.5~1×2~4 cm` | `0.005~0.01×0.02~0.04 m` | `1.00` |
| Fragment 002 | `3×40~80 cm` | `0.03×0.40~0.80 m` | `1.00` |
| Voronoi Slice | `200×300 cm` | `2×3 m` | `0.25` |
| FlowNoise Crackhole | `35×400 cm` | `0.35×4 m` | `1.00` |
| Dust Glasshole | `140×650 cm` | `1.4×6.5 m` | `1.00` |
| Caustic FluidNinja | `80~140×120~150 cm` | `0.8~1.4×1.2~1.5 m` | `1.00` |
| Caustic CustomParticle | `160~180×180~220 cm` | `1.6~1.8×1.8~2.2 m` | `1.00` |

큰 구조 레이어는 원본부터 큰 쿼드다. 원작에서는 family의 mask, opacity,
radial/cast envelope가 쿼드 안에서 최종 모양을 깎는다. 그 식이 없을 때만 큰
사각형이 그대로 드러난다.

## 2. Sprite 세 분류

### 2.1 최종 실루엣 carrier

`fx_a_fragment_001`, `fx_a_fragment_002_cl`처럼 simple/light parent에서
단일 `emissive_tex`로 쓰이고 RGB 경계가 검은 텍스처다. DDS alpha가 불투명해도
additive 또는 base-luminance alpha에서 검은 경계가 사라지고, 원본 크기도 작다.
이 계열은 기존 grouped 경로와 원본 크기를 유지한다.

### 2.2 alpha를 가진 중간 carrier

`fx_j_dustparticle_tile_02`는 자체 alpha가 있으므로 단순 근사도 어느 정도
형태를 보인다. 그러나 원본 역할은 Glasshole의 `aura_texture`이고,
`cracknormal_tex`, `in_hole_texture`, radial/edge/twist 식과 함께 쓰인다.
V0에서는 Source Trim으로 튜닝할 수 있지만 정확 경로는 Glasshole family다.

### 2.3 그래프 전용 타일

`fx_j_voronoi_tile_01`, `fx_j_flownoise_tile_04`, `fx_j_caustic_tile_02/04`는
대부분의 픽셀과 경계가 채워진 512×512 타일이다. 이들은 각각
`slice_flow_texture`, Crackhole map, FluidNinja/CustomParticle `diff_tex`이며,
별도 mask/opacity/flow/envelope가 최종 alpha를 만든다. slot 0을 최종 alpha로
사용하는 grouped fallback을 허용하지 않는다.

## 3. family별 구현 순서

### G00. 현재 W 수동 정리본 고정

- visible 21개와 Sprite 16개의 stable Element ID를 기준선으로 사용한다.
- Material Instance, parent, named texture role, SourceRecipe StartSize를 기록한다.
- 사용자의 visibility 제거와 Source Trim을 되돌리지 않는다.
- imported 전체 문서에서 Element를 다시 일괄 materialize하지 않는다.

### G01. Slice family-lite와 사용자 크기 보존

- `445823f15363035cab17dc91`은 기존 `effect.ue3.slice.v1`을 유지한다.
- `slice_flow_texture=fx_j_voronoi_tile_01.dds`와 analytic slice envelope가
  최종 경계를 만든다.
- `Size 0.25`는 source-exact 값이 아닌 사용자 V0 tuning으로 기록하되 그대로 보존한다.
- Count/Size/Life 수동 보정과 다른 Element의 모든 Source Trim은 projector가 소유하지 않는다.

### G02. Glasshole — 세로 검/오라

- `aura_texture=fx_j_dustparticle_tile_02`
- `cracknormal_tex=fx_j_normal_bc5_09`
- `in_hole_texture=fx_m_cybernoise_02`
- `alpha_tile`, `aura_pow/str`, `twist`, `edge`, `radial_size`를 고정 입력으로
  갖는 Glasshole family를 만든다.
- 현재 누락된 cracknormal named resource를 올리고, aura alpha에 inner-hole과
  edge envelope를 합성한다.

### G03. Crackhole — 세로 균열과 왜곡

- `01.map_e=fx_j_flownoise_tile_04`
- `06.map_f=fx_j_environment_tile_02`
- normal/noise와 `mask_tex_l/r`를 exact named role로 올린다.
- `flownoise`의 불투명 밝기 전체를 final alpha로 사용하지 않는다.
- 좌/우 mask, thickness/depth, distortion을 합쳐 Crackhole의 최종 alpha와
  distortion을 별도로 출력한다.

### G04. FluidNinja — Caustic 왜곡층

- `diff_tex=fx_j_caustic_tile_02`
- `flow_1_tex=fx_j_noise_tile_06`
- `flow_2_tex=fx_m_normalperlinnoise_01`
- `mask_tex=fx_j_wave_03_ycl`, `opacity_tex=fx_r_gra_03_ycl`
- 현재 `mask` slot에 flow texture를 넣은 grouped 조합을 교체한다.
- 최종 alpha는 `mask_tex * opacity_tex * particle alpha`, 색/왜곡은
  caustic과 두 flow의 UV 식으로 계산한다.

### G05. CustomParticle — Caustic cast/radial층

- `diff_tex=fx_j_caustic_tile_04`를 유지한다.
- `cast_center`, `cast_dirinout`, `cast_fov`, `cast_particle`, `cast_speed`로
  bounded envelope를 만든다.
- 원본 `a_noise_01_tex`가 unresolved이므로 임의 흰 texture로 대체하지 않는다.
  해당 입력 없이 식을 닫을 수 없으면 occurrence만 fail-closed한다.

### G06. 일반화된 authoring 진단

툴의 진단 분류는 렌더 결과를 자동 변경하지 않고 다음 세 badge만 제공한다.

```text
DIRECT_FINAL
ALPHA_INTERMEDIATE
GRAPH_ONLY
```

분류 근거는 파일명 하나가 아니라 다음 조합이다.

1. MIC의 named texture role과 parent family
2. 별도 mask/opacity/flow texture 존재 여부
3. DDS의 RGB/alpha 경계 점유율
4. blend mode
5. source world size와 aspect ratio

`GRAPH_ONLY`인데 executable typed family가 없으면 사각형 fallback 대신 해당
occurrence를 숨기고 이유를 표시한다. Source Trim은 사용자가 명시적으로 선택하는
프로젝트 tuning으로 남긴다.

### G07. 공용 family registry와 전수 적용

하나의 compiler-owned registry가 다음 계약을 소유한다.

```text
profileId / exact parentMaterialPath
allowed renderer shape and blend
required / optional named texture roles
scalar / vector / switch lanes
alpha source and bounded evaluator
admission failure policy
```

source contract builder와 materializer가 이 registry로 occurrence를 분류하고,
runtime 문서에는 resolved profile과 named role만 기록한다. Runtime은 별도 JSON
registry나 동적 shader를 읽지 않고 기존 fixed profile dispatch를 사용한다.

전수 적용은 다음 순서로 제한한다.

1. 전체 occurrence를 read-only manifest로 분류한다.
2. family 구현과 required-role closure가 닫힌 occurrence만 후보로 만든다.
3. stable Element ID 기준으로 profile/named role만 surgical patch한다.
4. cardinality, visible, timing, transform, SourceRecipe와 사용자 override fingerprint를
   유지한다.
5. W 대표 occurrence의 사용자 육안 판정 뒤 동일 family occurrence를 opt-in 승격한다.
6. family가 섞인 parent나 role이 빠진 occurrence는 자동 승격하지 않는다.
7. Claude 세션의 14% 우선군은 `fx_mm_simple_01_ad` 대표 occurrence를 별도로
   검증하고, W의 네 저빈도 family와 완료율을 합산하지 않는다.

## 4. 검증과 사용자 판정

자동 검증은 최소 범위로 유지한다.

- target Element cardinality와 stable ID
- named resource 존재와 role 중복/누락
- finite scalar/vector/Dynamic Parameter
- JSON parse와 HLSL/Client Debug compile
- missing typed family의 fail-closed
- family manifest와 targeted migration dry-run
- 사용자 override 및 cardinality fingerprint 불변
- `git diff --check`

대형 corpus 하네스나 새 admission 체계는 만들지 않는다. 화면 판정은 사용자가
Effect Tool에서 occurrence를 Solo 재생해 수행한다.

수동 판정 순서는 다음과 같다.

1. hard square rim이 없어졌는가
2. 실루엣이 reference 역할과 맞는가
3. world 크기와 aspect ratio가 맞는가
4. 흐름/왜곡 방향이 맞는가
5. 밝기와 수명

크기를 먼저 줄여 V0를 얻는 것은 허용한다. Source Trim은 사용자 저작값이며
family projector가 원본 크기라는 이유로 자동 복귀시키지 않는다.

현재 패치 확인은 stale Product runtime이 아니라 아래 Authoring 경로에서 한다.

```text
Effect Tool
  -> Data Files / DimensionMaster
  -> effect.dimensionmaster.skill.2050120.clip3.unified
  -> Open for Editing
  -> 대상 Element Solo 또는 Play All
```

`All Effects -> Product Play`는 publish 전의 51-Element runtime 문서를 재생하므로
현재 21-Element W 저장본과 family-lite 수식의 화면 증거로 사용하지 않는다.
