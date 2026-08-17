# Effect Cascade Material Graph Reconstruction 결과

## 0. 핵심 결론

사용자가 발견한 “경계를 깎고 크기를 줄이면 모양이 나온다”는 관찰은 맞다.
그러나 원인은 모든 Sprite Particle의 공통 size 버그가 아니다.

```text
잘 보이는 Fragment
  = 작은 원본 크기 + 검은 RGB 경계 + simple/light 최종 carrier

사각형으로 보인 Tile
  = 큰 원본 구조 크기 + 쿼드 전면을 채운 graph input
    + 누락된 mask/opacity/flow/envelope 식
```

즉 복원할 공식은 Sprite 전체의 공통 배율이 아니라 Material family별 최종 alpha
공식이다. Source Trim은 빠른 V0 tuning으로 유효하지만 source-exact 값은 아니다.

## 1. 첨부 화면에서 확인된 현상

- 첫 reference는 작은 유리 조각, 길게 선 중심 검/균열, 별도 광량과 왜곡이 합쳐진
  복합 화면이다. 한 Sprite가 전체를 만드는 화면이 아니다.
- Slice 분기 적용 뒤의 화면은 hard rectangular rim이 사라졌다. 이는 analytic
  envelope가 실제로 작동한다는 사용자 관찰과 일치한다.
- 같은 occurrence를 `Size × 0.25`로 줄인 현재 저장값은 좁은 검격형 실루엣을
  읽힌다. shader cutout 이후 occurrence 크기가 별도 조절 축이라는 증거다.
- 이 결과만으로 Voronoi를 작은 유리 조각 원본이라고 확정하지 않는다. 현재 작은
  조각의 가장 강한 후보는 원본부터 작은 `fragment_001/002` carrier다.

이 절은 사용자가 제공한 화면에 대한 관찰 기록이며 최종 visual PASS가 아니다.

## 2. 현재 W 수동 정리본

정본은
`Data/Effects/Authored/effect.dimensionmaster.skill.2050120.clip3.unified.effect.json`이다.

- visible Element: 21개
- Sprite Particle: 16개
- Mesh Particle: 5개
- 현재 Voronoi Slice Sprite: 1개(`445823...`)
- Source Trim이 기본값이 아닌 Sprite: 위 Slice 1개뿐
- 저장된 Trim: `Count 1.43 / Size 0.25 / Life 1.65`

사용자가 제거한 Element는 다시 복원하거나 imported 전체 문서에서 재생성하지 않았다.

## 3. 크기 데이터 교차검증

`Effect_Playback.cpp`에서 SourceRecipe Sprite 크기는 raw UE3 cm에 `0.01`을
정확히 한 번 곱한다. Source Trim은 변환과 source module 평가가 끝난
`Particle.vBaseSize`에 추가로 한 번 곱한다. Mesh의 model pre-scale은 Sprite에
적용되지 않는다.

```text
최종 쿼드 폭/높이
  = abs(raw StartSize cm * 0.01)
  * Source Trim Size
  * SizeOverLife
  * Element/Root scale
```

| Element/asset | raw source | 저장된 source 크기 | 현재 유효 시작 크기 |
|---|---:|---:|---:|
| `fragment_001` | `0.5~1×2~4 cm` | UI mirror `0.01×0.02 m` | runtime `0.005~0.01×0.02~0.04 m` |
| `fragment_002_cl` | `3×40~80 cm` | UI mirror `0.03×0.40 m` | runtime `0.03×0.40~0.80 m` |
| `voronoi_tile_01` | `200×300 cm` | `2×3 m` | `0.5×0.75 m` (`×0.25`) |
| `flownoise_tile_04` | `35×400 cm` | `0.35×4 m` | 동일 |
| `dustparticle_tile_02` | `140×650 cm` | `1.4×6.5 m` | 동일 |
| `caustic_tile_02` | `80~140×120~150 cm` | UI mirror `1.2×1.2 m` | runtime `0.8~1.4×1.2~1.5 m` |
| `caustic_tile_04` | `160~180×180~220 cm` | UI mirror `1.8×1.8 m` | runtime `1.6~1.8×1.8~2.2 m` |

`Size × 0.25`는 숨겨진 원본 값의 복구가 아니라 사용자가 저장한 authored multiplier다.
빠른 V0로 유지할 수 있지만 다른 Sprite에 일괄 적용하면 원본 공간 구조를 다시 잃는다.
표의 UI mirror `startSize/endSize`는 converter가 distribution의 대표 표본과 over-life
끝값을 접어 둔 approximation이다. SourceRecipe가 켜진 현재 실제 권위는 raw cooked
distribution이며 runtime 범위가 고정 mirror보다 우선한다.

## 4. DDS와 현재 grouped 식의 차이

현재 grouped translucent 식은 별도 mask/emissive가 없으면 대략
`base.a * luminance(base.rgb)`를 alpha로 사용한다. carrier 바깥쪽에는
`saturate(edgeDistance * 128)` feather가 있지만 UV 약 1/128 경계만 깎는다.
쿼드 내부 전체가 밝은 texture의 실루엣은 만들지 못한다.

| DDS | 형식/크기 | 원본 역할 | 실측 경계 특성 | 판정 |
|---|---|---|---|---|
| `fx_a_fragment_001` | DXT1 128² | 단일 `emissive_tex` | RGB `>0.1` 10.5%, border 0% | 최종 carrier |
| `fx_a_fragment_002_cl` | DXT5 64×256 | 단일 `emissive_tex` | RGB `>0.1` 24.0%, border 0% | 최종 carrier |
| `fx_j_voronoi_tile_01` | DXT1 512² | `slice_flow_texture` | RGB `>0.1` 98.5%, border 93% | graph-only field |
| `fx_j_flownoise_tile_04` | DXT1 512² | Crackhole `01.map_e` | RGB `>0.1` 100%, border 100% | graph-only field |
| `fx_j_caustic_tile_02` | DXT1 512² | FluidNinja `diff_tex` | RGB `>0.1` 97%, border 87% | graph-only field |
| `fx_j_dustparticle_tile_02` | DXT5 512² | Glasshole `aura_texture` | alpha `>0.1` 28.5%, border 21.9% | alpha 중간 carrier |
| `fx_j_caustic_tile_04` | DXT5 512² | CustomParticle `diff_tex` | alpha nonzero 99.6%, border alpha `>0.1` 46.6% | graph-only field |

Alpha가 불투명한지 하나만으로는 구분할 수 없다. Fragment 001도 alpha는 전부
불투명이지만 RGB 경계가 검고 additive이므로 사각형이 보이지 않는다. 반대로
FlowNoise/Voronoi/Caustic은 RGB 자체가 쿼드 전면에 존재해 final alpha 식 없이
그리면 반드시 사각형이 된다.

현재 잘못 연결된 grouped 조합까지 포함한 effective alpha support 실측도 같은
결론을 낸다.

| 현재 조합 | alpha support `>0.05` | border `>0.05` |
|---|---:|---:|
| Fragment 001 | 13.5% | 0.2% |
| Fragment 002 | 37.9% | 0% |
| FlowNoise + 현재 environment mask | 100% | 100% |
| Dust | 23.4% | 17.5% |
| Caustic 02 + 현재 normalperlin mask | 100% | 100% |
| Caustic 04 | 37.0% | 20.5% |

## 5. 남은 Sprite 16개 분류

| # | asset/family | source 시작→끝 크기 | 분류/처리 |
|---:|---|---:|---|
| 1 | Bloodcliff Circle | `3.5×6 → 5.25×9` | typed `circle.v1` 유지 |
| 2 | Voronoi Slice | `2×3 → 1.6×2.4` | graph-only, `slice.v1`, Trim 0.25 |
| 3 | FlowNoise Crackhole | `0.35×4 → 0.875×4.8` | graph-only, Crackhole 필요 |
| 4 | Dust Glasshole | `1.4×6.5 → 1.12×5.2` | alpha 중간, Glasshole 필요 |
| 5 | Caustic FluidNinja | `1.2×1.2 → 1.2×0` | graph-only, mask/opacity 필요 |
| 6 | Shine grouped | `1×0.7 → 0.1584×4.578` | composite, 이후 family 판정 |
| 7 | Shine typed | `1.4×6` | typed `shine.v1` 유지 |
| 8 | RGB Split | `10×10` | composite, RGBSplit family 후보 |
| 9 | Basic Glow | `1×1.8 → 1×9` | 단순 emissive 후보 |
| 10 | Fragment 001 | `0.01×0.02 → 0.01×0` | 최종 carrier, 유지 |
| 11 | Lens Flare | `4×1.5 → 0` | 전용 lens role, 유지/육안 판정 |
| 12 | Fragment 002 | `0.03×0.4` | 최종 carrier, 유지 |
| 13 | Fragment 002 delayed | `0.03×0.4` | 타이밍이 다른 별도 carrier, 유지 |
| 14 | Caustic CustomParticle | `1.8² → 2.4204²` | graph-only, cast envelope 필요 |
| 15 | Ground Dust Glasshole | `5×5 → 3.5×3.5` | alpha 중간, Glasshole 재사용 |
| 16 | Ring Noise | `1.25×0.01 → 4×0.032` | composite, Ring family 후보 |

같은 DDS나 같은 resource binding이라고 Element를 합치지 않는다. 크기, timing,
SourceRecipe와 family 역할이 다르면 별개 occurrence다.

## 6. 문제 family별 정확한 누락

### 6.1 Slice

`fx_j_voronoi_tile_01`은 최종 모양이 아니라 `slice_flow_texture`다. 현재
`effect.ue3.slice.v1`이 analytic slice envelope를 만들면서 hard square rim이
사라졌다. 이 방향은 유지한다.

### 6.2 Crackhole

현재 active binding은 `flownoise`, normal, environment 세 장뿐이다. 원본 Material
증거에는 `mask_tex_l/r`, thickness/depth, mask noise, distortion이 따로 있다.
`flownoise`를 final alpha로 쓰지 않고 좌/우 mask와 crack envelope를 복구해야 한다.

### 6.3 Glasshole

`dustparticle`은 `aura_texture`다. 원본은 추가로
`cracknormal_tex=fx_j_normal_bc5_09`, `in_hole_texture=fx_m_cybernoise_02`,
radial/edge/twist 식을 사용한다. 현재 resource에는 cracknormal named input이
빠져 있다.

### 6.4 FluidNinja

`caustic_tile_02`는 `diff_tex`이고, 두 flow texture와 별도로
`mask_tex=fx_j_wave_03_ycl`, `opacity_tex=fx_r_gra_03_ycl`이 있다. 현재 grouped의
`mask` slot에는 실제 final mask가 아니라 `flow_2_tex`가 들어가므로 alpha support가
100%가 되는 것이 정상적인 실패다.

### 6.5 CustomParticle

`caustic_tile_04`는 `diff_tex`이고 cast/radial 식이 최종 범위를 만든다.
`a_noise_01_tex`는 원본 증거에서도 unresolved이므로 임의 texture로 채워
source-exact라고 부르지 않는다.

## 7. 일반화 공식

Sprite 여부나 파일명으로 렌더 경로를 고르지 않는다.

```text
DIRECT_FINAL
  단일 main/emissive/lens texture
  + simple/basic/light parent
  + 검은 RGB 또는 안전한 alpha 경계
  -> grouped/simple 허용

ALPHA_INTERMEDIATE
  자체 alpha는 있으나 normal/inner/radial/edge 입력이 별도 존재
  -> V0 grouped 가능, exact는 typed family

GRAPH_ONLY
  flow/noise/caustic/voronoi/map 역할
  + 별도 mask/opacity/envelope
  + 경계 점유율이 높음
  -> typed family 없으면 fail-closed
```

최종 family 출력은 최소 다음 계약을 가져야 한다.

```text
color = family diffuse/flow/caustic expression
alpha = explicit mask or opacity
      * procedural/radial/slice envelope
      * Cascade particle alpha
distortion = family normal/flow expression * bounded strength
```

이 규칙은 차원술사만의 예외가 아니다. 같은 parent Material family를 쓰는 다른
캐릭터도 같은 evaluator를 재사용할 수 있다. 다만 자동 classifier는 진단 badge일
뿐이며 filename heuristic으로 shader를 자동 승인하지 않는다.

## 8. family-lite 전환 결과

일괄 family 개념 전체가 실패한 것은 아니다. 실패 범위는 stripped graph를 바탕으로
만든 네 reconstructed 수식을 first-pixel 확인 전에 여러 occurrence에 승격한 이번
국소 구현이다. W 대표 6개에는 다음 수리와 보존 경계를 반영했다.

- Slice: 카드 경계 feather를 추가하고 RGB/alpha의 shape 이중 적용을 제거했다.
- Glasshole: 카드 경계 feather를 추가하고 authored radial 값의 0을 임의로 1로
  바꾸지 않도록 했다.
- CustomParticle: `mask_bias=-1`을 DDS alpha에 직접 더해 전 픽셀 alpha가 0이 되던
  수식을 제거하고, cast envelope가 cutout을 소유하게 했다.
- Crackhole: round-emission의 `0.05`를 본체 thickness `5`의 override로 오인하던
  배선을 제거했다.
- FluidNinja: 별도 `mask_tex`와 `opacity_tex`를 사용하는 현재 lite 식을 유지했다.
- 기존 fragment 3개와 non-target Element는 profile/data를 변경하지 않았다.
- Slice의 `Count 1.43 / Size 0.25 / Life 1.65`를 보존했다.

이 구현은 bit-for-bit cooked material 복원이나 source-exact 결과가 아니다. 원본
named role과 일부 scalar를 사용한 bounded family-lite이며, 최종 크기·clip·밝기는
대표 Element를 사용자가 눈으로 확인하면서 조절한다.

## 9. 자동 검증과 수동 검증 경계

실행한 자동 검증:

- `Shader_VtxEffectParticle.hlsl`의 `PS_MAIN`을 `fxc ps_5_0`으로 컴파일
- family projector focused unit test 7개 PASS
- Product direct 101문서/2,945 Element census: 후보 18, admitted 18,
  rejected 0, pending 0
- projector write: authored 문서 변경 0, receipt만 갱신
- HLSL/Python/registry 대상 `git diff --check` PASS
- Client x64 Debug 빌드에서 Effect shader와 C++ 컴파일까지 PASS. 최종 링크는
  사용자가 실행 중인 `Client/Bin/Debug/Client.exe`가 출력 파일을 점유해
  `LNK1104`로 중단됐다. 컴파일 오류나 assertion은 아니며, 실행 중 Client를
  사용자가 종료한 뒤 동일 빌드의 링크 재확인이 남아 있다.

사용자 수동 검증은 아직 PASS가 아니다. 현재 결과는 아래 경로에서 확인해야 한다.

```text
Effect Tool
  -> Data Files / DimensionMaster
  -> effect.dimensionmaster.skill.2050120.clip3.unified
  -> Open for Editing
  -> Slice / Crackhole / Glasshole 2개 / FluidNinja / CustomParticle Solo
```

`All Effects -> Product Play`는 02:48에 publish된 51-Element 구버전을 재생한다.
현재 Authored는 21 Element이므로 Product Play 결과로 이번 패치를 판정하지 않는다.

## 10. 실제 완료와 남은 경계

완료된 항목:

- 현재 W 수동 정리본 21개/16 Sprite inventory 고정
- fragment와 문제 tile의 DDS pixel support 비교
- raw Cascade size와 Client `×0.01` 변환 교차검증
- Source Trim이 별도 authored multiplier임을 확인
- Slice occurrence를 `effect.ue3.slice.v1`과 named
  `slice_flow_texture`로 연결
- 네 family profile과 W 대표 6개 named-role binding을 연결
- zero-alpha/100배 thickness/shape 이중 적용의 정적 결함 수정
- 수동 Source Trim을 projector correction 대상에서 제거

아직 완료하지 않은 항목:

- W 대표 6개의 사용자 first-pixel 및 형태 판정
- family별 Cutoff/Source Trim/색·밝기 눈 튜닝
- 대표 승인 후 동일 family occurrence의 opt-in 확장
- cooked DXBC source-exact replay
- W 전체 visual PASS
- Product republish

공유 worktree에는 이전 SpriteWave opcode/HLSL/materializer WIP가 남아 있다. 현재
사용자가 정리한 visibility와 Source Trim을 보존하기 위해 materializer의 write mode를
다시 실행하지 않았다.

## 11. 이번 확인 범위

Client/UI를 에이전트가 실행하거나 조작하지 않았고 visual PASS를 대신 판정하지
않았다. 대형 corpus 하네스와 Product publish는 실행하지 않았다. 자동 검증은
family projector focused test, HLSL compile, 대상 diff 검사로 제한했다.
