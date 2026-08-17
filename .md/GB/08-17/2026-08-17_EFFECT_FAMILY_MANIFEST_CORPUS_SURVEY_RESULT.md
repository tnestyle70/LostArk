# Effect 부모 Material family 전수 조사 결과

branch: `feature/effect-cooked-shader-recovery-and-transform-fix`
관련: `2026-08-17_EFFECT_CASCADE_MATERIAL_GRAPH_RECONSTRUCTION_RESULT.md` (Codex, W 4 family)

## 1. 구현한 것

```text
Tools/EffectPipeline/build_effect_family_manifest.py        신규
Tools/EffectPipeline/test_build_effect_family_manifest.py   신규 (18 tests)
Data/Effects/Contracts/effect-family-manifest.v1.json       신규 산출물 (644 KB)
```

저작 corpus에 대해 읽기 전용이다. 쓰는 파일은 manifest 하나뿐이며 Codex가 작업 중인
shader/materializer/W 문서와 겹치지 않는다.

`--mode build`로 생성하고 `--mode check`로 corpus와 어긋났는지 검사한다. manifest와 각
family row는 canonical SHA-256으로 봉인한다.

## 2. Codex 수치와의 대조

| 항목 | Codex | 이번 실측 |
|---|---:|---:|
| 문서 | 101 | 202 |
| Element | 2,945 | 5,105 |
| Source Profile | 2,878 | 2,918 |
| grouped-translucent | 2,532 | 2,556 |
| **부모 Material** | **169** | **169** |
| **아직 grouped인 부모** | **144** | **144** |
| **Sprite/Mesh 혼용** | **18** | **18** |

문서·Element 차이는 범위 차이다. source catalog는 203행(`.unified` 101 + 기타 102)이고
Codex는 unified만, 이번 조사는 catalog에 등재된 문서 전부를 셌다. **판단에 쓰는 family 축
세 숫자는 완전히 일치**하므로 분류 전제는 검증됐다.

## 3. 증거 확보 상태

```text
family                                        169
  부모 Material props 증거 있음               151
  named texture parameter까지 확보            136
추출한 named parameter
  texture parameter                           545
  scalar parameter                          3,728
```

admission 분류는 `family identity`와 `실행 가능한 admission`을 분리한다.

```text
READY_FOR_SINGLE_FAMILY_IMPLEMENTATION        102   blocker 없음
VARIANT_SPLIT_REQUIRED                         49   carrier/role/renderProfile 분기
EVIDENCE_REQUIRED                              10
EVIDENCE_REQUIRED_AND_VARIANT_SPLIT_REQUIRED    8
```

blocker는 의견이 아니라 corpus의 사실이다.

```text
CARRIER_SPLIT_SPRITE_AND_MESH    한 parent를 Sprite와 Mesh가 함께 사용
NAMED_ROLE_SET_VARIANCE          같은 parent인데 바인딩된 슬롯 집합이 다름
RENDER_PROFILE_VARIANCE          같은 parent인데 one-sided/two-sided가 섞임
PARENT_MATERIAL_EVIDENCE_ABSENT  props 추출본 없음
```

## 4. 레버리지

grouped 2,556 occurrence를 부모 빈도순으로 누적하면 다음과 같다.

```text
top  5 family ->   910 / 2,556   35.6%
top 10 family -> 1,362 / 2,556   53.3%
top 20 family -> 1,756 / 2,556   68.7%
top 50 family -> 2,254 / 2,556   88.2%
```

70개 parent가 2개 이상 class에서 쓰인다. family 하나가 여러 class를 동시에 덮는다.

### 4.1 blocker가 하나도 없는 family

```text
  grp skills cls  expr   sw  tex scal  parent
   73     30   4    46    0    1    3  fx_c_pa_lensflare_01_ad
   63     23   4    18    9    1    0  fx_mm_light_01_ad
   31     11   3    32    4    4    6  fx_e_pa_twinkle_01_ad
   25     10   2    43    1    2    3  fx_e_me_rock_01_ma
   20      8   4    95    7    3   10  fx_m_pa_smoke_01_tr
   ...
  READY 합계  591 / 2,556  (23.1%)
```

`fx_mm_light_01_ad`는 **expression slot 18개**로 corpus에서 가장 단순하고 4 class 23 skill이
쓴다. `fx_c_pa_lensflare_01_ad`는 **static switch가 0개**라 permutation이 하나뿐이다.
이 둘이 지금 가장 싼 관통점이다.

### 4.2 최다 빈도 family는 거의 다 왔다

```text
fx_mastermaterial.fx_mm.fx_mm_simple_01_ad
  grouped occurrence  359   skill 75   class 4   sprite 전용
  role set            1종 (base 하나뿐)
  BLEND_Additive, twoSided=false, isMasked=false
  expression slot 54, static switch 12, referencedTextures 2
  texture 2개 : emissive_tex, uv_noise_tex
  scalar 8개  : uv_panning_x 0.0        uv_panning_y 0.0
                uv_noise_panning_x 0.1  uv_noise_panning_y 0.1
                uv_noise_tilling 1.0    uv_noise_intensity 0.0
                emissive_tex_desturation 0.0
                dynamic_parameter_explanation 1.0
  blocker             RENDER_PROFILE_VARIANCE 하나뿐
                      additive_one_sided 279 / additive_two_sided 89
```

material은 texture 2개를 선언하지만 368 occurrence 전부 `base` 하나만 바인딩한다.
`uv_noise_intensity` 기본값이 `0.0`이라 noise 경로가 꺼져 있으므로 정합한다.

corpus의 14%를 차지하는 최대 family인데 남은 blocker가 **formula 차이가 아니라 raster
state 분기 하나**다. 기본값도 전부 중립(panning 0, noise intensity 0)이라 현재 grouped
경로와 실제 차이가 크지 않을 가능성이 높다. **먼저 눈으로 확인할 값어치가 가장 크다.**
맞으면 14%를 공짜로 건너뛰고, 틀리면 게임 전체에서 가장 큰 한 방이다.

## 5. Codex의 W 4 family 위치

```text
family            순위/144   grouped occ   skills
Glasshole            54           8          4
Slice                96           2          2
Crackhole           118           1          1
FluidNinja          119           1          1
CustomParticle      120           1          1
                              ────────
                              11 / 2,556  =  0.4%
```

W 화면을 통과시키는 목표로는 정확한 선택이다. 다만 corpus 일괄 적용 레버리지는 0.4%이므로
두 트랙을 같은 목표로 취급하지 않는다.

```text
Codex 트랙   W 하나를 눈으로 통과시킨다        4 family, 11 occurrence
전수 트랙    게임 전체의 사각형을 줄인다       top 10, 1,362 occurrence
```

Crackhole은 expression slot 251, **static switch 29개**에 scalar 이름이 `01`, `02`, `12`,
`15` 같은 숫자라 이름으로 뜻이 나오지 않는다. 144개 중 이것 하나만 DXBC oracle이 실제로
필요하다. 마지막에 두거나 빼는 편이 낫다.

## 6. 발탄은 이 축에 없다

```text
valtan authored 문서                75
  element                          193
  enabled sourceProfile              3
  grouped                            1
  catalog 등재                       0
```

발탄 Effect는 원본 추출이 아니라 손으로 저작한 문서다. 부모 Material이 사실상 없으므로
family 복원 대상이 아니고 catalog에도 없어 제품 경로에 오르지 않는다. 기존대로 Model View와
수동 저작 트랙으로 유지한다.

occurrence의 class 분포는 다음과 같다.

```text
lancemaster      1,494
warlord            632
dimensionmaster    538
artist             254
```

## 7. 증거가 없는 8종

```text
  60  artist,lancemaster,warlord            bfx_d_pa_shine_01_ad
  54  4 class 전부                          fx_d_pa_shine_02_ad
  27  warlord                               ch.realpbr.base.realpbr_wp_dead_msk
   9  artist,lancemaster,warlord            fx_d_pa_shine_02_tr
   1  artist                                fx_d_pa_dark_05_tr
   1  dimensionmaster                       fx_l_me_icesurfacee_01_tr
   1  dimensionmaster                       fx_j_me_splitline_01_1_ad
   1  warlord                               fx_m_pa_ring_11_ad
```

실질 공백은 shine 계열 3종(123 occurrence)과 warlord PBR 1종이다. 나머지 4종은 1건짜리다.
shine 3종은 추가 추출로 닫을 수 있는지 확인할 값어치가 있다.

## 8. 자동 검증 — 실행함

```text
build_effect_family_manifest.py --mode build     PASS  (169 families)
build_effect_family_manifest.py --mode check     PASS  (idempotent)
test_build_effect_family_manifest.py             PASS  18/18
git diff --check                                 PASS
```

테스트는 props 파싱, admission 분류, manifest 봉인, 누적 커버리지 단조성, 증거 루트 부재 시
degrade, 그리고 **커밋된 manifest가 corpus와 일치하는지**를 검사한다.

## 9. 이번에 발견한 파서 함정

`props.txt` 한 파일 안에 서로 다른 두 레이아웃이 섞여 있다.

```text
texture parameter    여러 줄, '=' 양쪽에 공백
    CollectedTextureParameters[0] =
    {
        Texture = Texture2D'fx_j_voronoi_tile_01'
        Name = slice_flow_texture

scalar parameter     한 줄, '=' 양쪽에 공백 없음
    CollectedScalarParameters[0] = { Value=-3.14, Name=slice_rot, Group=None }
```

한쪽 레이아웃만 보고 쓴 정규식은 다른 쪽을 **조용히 0개로 떨어뜨린다.** 초판이 정확히 이
실수를 했고 texture parameter 545개를 전부 놓쳤다. 중괄호를 포함하지 않는 본문만 매칭해
가장 안쪽 항목을 잡도록 고쳤다. 회귀 테스트의 샘플은 실제 파일에서 그대로 복사했다.

`Path.stem`도 `.props.txt`에서 `.props`만 벗겨내므로 증거 색인이 전부 빗나간다. 초판이 이
실수로 `증거 0/144`라는 잘못된 결론을 냈다.

## 10. signature 클러스터링 — 일괄 구현이 가능한가

"family가 서로 비슷하니 몇 개 식으로 묶으면 되지 않나"를 실측했다.

```text
증거 있는 grouped family        136
고유 parameter signature        117
```

**거의 안 겹친다.** 묶어도 136 -> 117이므로 signature 하나가 곧 식 하나다. 다만 빈도
분포가 가팔라서 상위 몇 개가 크게 덮는다.

```text
top  5 signature -> 1,084 / 2,402   45.1%
top 10 signature -> 1,520 / 2,402   63.3%
top 20 signature -> 1,845 / 2,402   76.8%
```

그리고 parameter 이름이 뜻을 알려주는지로 나누면 작업 성격이 갈린다.

```text
self-describing 이름   1,858 occurrence (72.7%)   손으로 식 작성 가능
숫자 이름                501 occurrence (19.6%, 37 family)   DXBC oracle 필요
named parameter 없음      43 occurrence
증거 없음                154 occurrence
```

숫자 이름 최대 덩어리는 Crackhole 하나가 아니라 `fx_d_pa_master_01_tr`(142) +
`fx_d_pa_master_01_ad`(107) = 249 occurrence다. 이 구간은 이름으로 추측하지 않고
fail-closed로 남긴다.

## 11. 승격 도구

```text
Tools/EffectPipeline/promote_effect_family_occurrences.py        신규
Tools/EffectPipeline/test_promote_effect_family_occurrences.py   신규 (12 tests)
```

family 하나가 구현될 때마다 조건이 정확히 일치하는 occurrence만 승격한다. 기본 모드는
report이며 `--mode apply` 없이는 아무것도 쓰지 않는다. 쓰는 필드는
`material.sourceProfile.runtimeShaderProfileId` 하나뿐이고 visible/timing/transform/
particle/resource와 손튜닝은 건드리지 않는다.

거절 사유는 부모 이름 일치 여부가 아니라 occurrence의 사실이다.

```text
SOURCE_PROFILE_DISABLED
PARENT_MISMATCH
ALREADY_PROMOTED
NON_GROUPED_CURRENT_PROFILE:<id>        기존 typed 저작물은 덮지 않는다
CARRIER_NOT_ALLOWED:<carrier>
ROLE_SET_NOT_ALLOWED:<slots>
RENDER_PROFILE_NOT_ALLOWED:<profile>
EXECUTION_PACKET_OWNS_THIS_ELEMENT
EXCLUDED_BY_REQUEST
```

### 11.1 `fx_mm_simple_01_ad` 승격 대상 실측

```text
PROMOTE   359   문서 74개, class 4개
SKIP        9   전부 NON_GROUPED_CURRENT_PROFILE:effect.ue3.reconstructed-standard.v1
```

9개는 Artist Track A가 이미 typed profile을 준 occurrence다. 도구가 저작물을 덮지 않고
거절한 결과이며 의도한 동작이다.

## 12. 진행 중 바뀐 두 가지 (2026-08-17 오후)

### 12.1 publish 차단이 풀렸다

Codex가 `sourceProfile.enabled=false` 플립을 되돌렸다. clip3 문서의 `execution.enabled`는
0개이고 `Publish-Effects.ps1 -Mode Validate`가
`PASS: validated 97 Effect catalog entries`로 통과한다. §9의 blocker는 해소됐다.

### 12.2 적용 방식이 opcode에서 typed profile로 바뀌었다

Codex가 `Effect_MaterialTemplate.h`의 `EFFECT_SOURCE_RUNTIME_SHADER_PROFILE_IDS`를
17 -> 21로 늘려 다음을 등록 중이다.

```text
effect.ue3.glasshole-02.v1
effect.ue3.fluidninja-01.v1
effect.ue3.customparticle-01.v1
effect.ue3.crackholev2-01.v1
```

opcode/execution packet 경로는 `Effect_DocumentCodec.cpp:11017`의 불변식 때문에 v13
문서에서 **로드 자체가 실패**한다. 따라서 typed profile이 유일하게 성립하는 경로이고,
승격 도구도 그 축으로 만들었다. 두 번째 런타임 경로를 만들지 않는다는 경계와도 맞다.

## 13. 다음 작업

manifest가 있으므로 family를 구현할 때마다 조건이 정확히 일치하는 occurrence를 바로 뽑을 수
있다. 아직 하지 않은 것은 다음과 같다.

```text
family registry (runtime profile / opcode 매핑)   미구현
자동 승격 도구                                     미구현
HLSL family 구현                                   Codex 소유, 이번 변경에 없음
```

승격 도구를 만들 때는 사용자 수정값, source resource, visible/timing/transform을 보존하고
조건이 닫히지 않으면 grouped 유지 또는 fail-closed한다는 경계를 그대로 적용한다.

publish는 여전히 막혀 있다. `Publish-Effects.ps1 -Mode Validate`가
`effect.dimensionmaster.skill.2050120.clip3.unified`의 `sourceProfile.enabled=false`에서
계속 실패한다. 상세는
`2026-08-17_EFFECT_AUTHORED_SAVE_TO_PRODUCT_STALENESS_RESULT.md` §5.1과 §7.
