# 2026-08-22 워로드 A 사슬 family와 도화가 Z carrier 복원 결과

branch: `codex/lance-df-v1-restore`

선행: [`MM_LIGHT01과 창술사 V carrier 복원`](2026-08-22_MM_LIGHT01_AND_LANCEMASTER_V_CARRIER_RESTORATION_RESULT.md)

## 1. 워로드 A(17090 갈고리 사슬) — masked mesh family

### 1.1 원본 증거

`fx_m_mi_00.fx_m.fx_d_me_chain_01_ma`의 parent evidence는 이 corpus에서 가장 특이하다.

```text
blend              BLEND_Masked (1)
twoSided           true
isMasked           true
expressionSlots    132
staticSwitchCount  0
textureParameters  []                                   <- 하나도 없음
scalarParameters   worldpositionoffset_bias    50.0
                   worldpositionoffset_uvscale  1.0     <- WPO 두 개가 전부
```

즉 이 material은 **텍스처를 하나도 소유하지 않고**, masked cutout과 world position offset만 기여한다.
화면에 보이는 사슬 무늬는 element의 `base` binding(`fx_d_atypical_028.dds`)에서 온다.
Product 12행은 `fm_d_berchain_06.wmodel` 9개와 `fm_d_berchain_07.wmodel` 4개를 쓰는 mesh particle이다.

### 1.2 무엇이 틀려 있었나

| 축 | 원본 | 변환 결과 | 결과 |
|---|---|---|---|
| blend | `BLEND_Masked` | `alpha_two_sided_depth_read` | 이진 cutout이 soft alpha로 바뀜 |
| cutout 임계값 | masked | `detail.color.clip = 0.0` | cutout이 **아예 없음** |
| 표면 alpha | DDS에 alpha 채널 없음(`alphaFlag=false`) | grouped가 alpha로 추정 | 실루엣이 뭉개짐 |

사슬 링이 단단한 금속이 아니라 반투명 유령으로 그려지고 있었다.

### 1.3 구현 — `MESH_MASKED_CHAIN01` (opcode 41)

```text
carrier   mesh particle + base binding 존재
lane 0    element base binding을 linear 로 stage (named source texture가 없으므로)
coverage  artwork luminance (DDS에 alpha 채널이 없다)
cutout    clip(coverage - 0.3333)   <- UE3 masked 기본 OpacityMaskClipValue
shape     1.0                       <- masked 는 이진 coverage
```

`0.3333`은 UE3 engine 기본값이며 이 parent가 override를 직렬화하지 않는다는 사실을 근거로 쓴다.
source에서 측정한 값이 아니라는 점을 여기 명시한다.

### 1.4 WPO는 구현하지 않았다

`worldpositionoffset_bias`와 `worldpositionoffset_uvscale`은 packing하지 않았다.

- WPO는 vertex stage 출력이고 이 작업의 RT0 base는 pixel stage다.
- 두 scalar의 **이름만** 증거에 있고, 132 slot짜리 parent expression graph가 없다.
  변위 함수를 지어내면 사슬의 기하가 근거 없이 바뀐다.

`NATIVE_PARITY` backlog로 남긴다. 사슬이 늘어나거나 출렁이는 움직임은 이 항목이 닫히기 전까지 없다.

## 2. 도화가 Z(31050 저무는 달) — family 확장 불가, carrier 복원으로

### 2.1 family 확장을 시도했으나 증거가 없다

live 10행 중 4행은 이미 typed(`PARTICLE_MASTER_01`, `SIMPLE01` 2, `SIMPLE02`)다.
남은 grouped 3행의 parent evidence를 조회한 결과는 다음과 같다.

| parent material | corpus occ | evidence |
|---|---:|---|
| `bfx_m.bfx_d_pa_circ_01_ad` | 84 | **ABSENT** |
| `fx_m.fx_d_markring_02_tr` | - | manifest에 없음 |
| `fx_m_mi_03.fx_m.fx_d_pa_ring_12_ad` | - | manifest에 없음 |

texture parameter도 scalar parameter도 blend 정보도 없다. RT0 식을 세울 근거가 없으므로
family를 만들지 않는다. 세 행은 grouped에 남긴다.

### 2.2 진짜 결손은 material이 아니라 element다

```text
변환된 source emitter   23개
Product 2개 문서가 소유  10개
화면에 없는 것          13개
```

빠진 13개에 이 스킬의 **주 연출이 들어 있다**.

| source system | 행 | 성격 |
|---|---:|---|
| `fx_pc_sdm_00.par_o_harmonyofyin_01_01` | 8 | 저무는 달 본체 |
| `fx_pc_sdm_00.par_o_trailtest_01` | 2 | trail |
| `fx_cm_02.light.par_mp_light_03` | 1 | light |
| `fx_post.fx_par.par_c_filmnoise_01` | 1 | screenPost |
| `fx_post.fx_par.par_c_zoomblur_03` | 1 | screenPost |

carrier 구성은 mesh particle 4, sprite particle 6, screenPost 2, light 1이다.
screenPost와 light는 material family가 아니라 presentation 계약이므로 별도 축이다.

## 3. 생성물

| 파일 | 행 | 상태 |
|---|---:|---|
| `effect.artist.skill.31050.restoration-candidate.effect.json` | 13 | Tool 전용 |
| `effect.warlord.skill.17090.restoration-candidate.effect.json` | 2 | Tool 전용 |
| `effect.lancemaster.skill.34610.restoration-candidate.effect.json` | 134 | Tool 전용(기존) |

세 후보 모두 catalog에 등록하지 않았고 Product 문서는 한 바이트도 바뀌지 않았다.
`kind`, `rendererShape`, `renderProfile`, `resources`(mesh asset 포함)를 변환본에서 그대로 복사한다.

생성기는 skill별 스크립트를 `Tools/EffectPipeline/build_source_restoration_candidates.py` 하나로
일반화했다. 34610 후보는 refactor 전후로 byte 단위 동일하며 stable ID 계보가 유지된다.

## 4. 자동 검증 (실행함)

| 검증 | 결과 |
|---|---|
| Client x64 Debug build | PASS |
| `Publish-Effects.ps1 -Mode Validate` | PASS 207 entries |
| `audit_typed_source_profile_join.py` | classifier pair 41, dead identity join 0 |
| `test_build_source_restoration_candidates.py` | 7 tests OK |
| `test_audit_typed_source_profile_join.py` | 4 tests OK |
| `Test-EffectDataProjectRegistration.ps1` | PASS files=1868 |
| `git diff --check` | 경고 없음 |

후보가 catalog에 등록되지 않았다는 것도 test로 고정했다.

## 5. 수동 검증 (사용자 전용)

```text
워로드 A : Effect Tool -> effect.warlord.skill.17090.unified
           사슬 링 12행이 반투명 유령이 아니라 단단한 실루엣으로 잘리는지
           (움직임/늘어남은 WPO 미구현이므로 아직 없다)

도화가 Z : Effect Tool -> effect.artist.skill.31050.restoration-candidate  (13행)
           harmonyofyin 8행이 저무는 달 연출로 보이는지, Product 에 넣을 것을 고른다
```

## 6. 다음 경계

- WPO vertex stage — 사슬 움직임. DXBC 없이는 식을 세울 수 없다
- `bfx_d_pa_circ_01_ad`(corpus 84행) parent evidence 회수 — 지금 manifest가 ABSENT
- screenPost/light presentation 계약 — material family와 다른 축
