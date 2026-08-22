# 2026-08-22 MM_LIGHT01 family와 창술사 V carrier 복원 결과

branch: `codex/lance-df-v1-restore`

선행: [`창술사 D·F V1 family 확장`](2026-08-22_LANCEMASTER_D_F_V1_FAMILY_EXPANSION_RESULT.md)

## 1. 이번 범위

Valtan은 codex 세션이 carrier 재구성을 진행 중이므로 이 branch에서 건드리지 않는다.
이번 변경은 4캐릭터 쪽 두 갈래만 닫는다.

1. `MM_LIGHT01` family — 새 HLSL 없이 기존 evaluator 재사용
2. 창술사 V(34610 적룡질풍격)의 미복원 source occurrence를 carrier-exact Tool 후보로 생성

## 2. MM_LIGHT01 — 새 셰이더 0줄

`fx_mm_light_01`은 corpus에서 가장 단순한 master material이다.

```text
texture parameter : emissive_tex 1개
scalar parameter  : 0개
vector parameter  : 0개
blend             : _ad = Additive / _tr = Translucent, 둘 다 단면
carrier           : sprite 전용
occurrence        : _ad 64 / _tr 5
```

scalar가 하나도 없으므로 이 family의 출력은 `fx_mm_simple_01`(opcode 33)의 모든 입력이 중립일 때의
결과와 같다. `Build_Simple01Constants`의 fallback이 pan 0, noise intensity 0, desaturation 0을 만들고
noise lane은 texture mask로 gate되어 꺼진다. 따라서 가이드 9절의 첫 행
`equation과 layout이 같고 texture/scalar만 다름 -> 기존 HLSL·adapter 재사용`을 적용해
**새 HLSL 없이 opcode 33을 공유**한다.

carrier 계약만 이 family용으로 따로 둔다. simple_01은 additive 전용이지만 light_01은 `_tr` 변종이
alpha이므로 sprite carrier + `emissive_tex` 필수만 요구하고 blend는 render state로 넘긴다.

결과: classifier가 아는 element가 **59행 추가**(`MM_LIGHT01`), 새 셰이더 코드 0줄.

## 3. 창술사 V — carrier-exact 복원 후보

### 3.1 실측

```text
변환된 source emitter        172개  (conversion receipt: unsupported 0, missingResource 4)
Product 3개 문서가 소유       38개
화면에 없는 source occurrence 134개
```

`authored-baseline` 문서(98행)는 이 비교의 기준이 아니다. 그 문서는 standalone mesh/sprite에
`parentMaterialPath`가 없는 **이전 세대 저작물**이고, `.unified`가 source 변환 계보다. 두 문서를
before/after로 읽으면 안 된다.

### 3.2 빠진 134행의 carrier 구성

| kind | shape | render profile | 행 |
|---|---|---|---:|
| particle | sprite | additive_two_sided | 42 |
| particle | sprite | alpha_two_sided | 42 |
| particle | mesh | alpha_two_sided | 38 |
| particle | mesh | additive_two_sided | 6 |
| light | light | alpha_two_sided | 4 |
| decal | decal | alpha_two_sided | 1 |
| screenPost | screenPost | alpha_two_sided | 1 |

mesh asset 6종이 사용된다: `fm_d_plane_003`(11), `fm_o_swing_02`(11), `fm_h_swing_03`(6),
`fm_c_square_001`(5), `fm_n_flm_ydr_00_sm`(2), `fm_a_stone_001`(1).

### 3.3 생성물

`Data/Effects/Authored/effect.lancemaster.skill.34610.restoration-candidate.effect.json`

- catalog에 등록하지 않는다. **Tool 전용**이며 Product 문서 3개는 한 바이트도 바뀌지 않았다.
- element kind, `rendererShape`, `renderProfile`, `resources`(mesh asset 포함)를 source 변환본에서
  **그대로 복사**한다. 추론하지 않는다.
- stable ID는 source element ID의 SHA-1 앞 16자로 결정한다(`restore.lancemaster.34610.<hash>`).
- resource 224개 전부 `Client/Bin/Resources` 아래 실재한다.

### 3.4 material 신원은 채우지 않았다

134행의 `sourceProfile.enabled`는 `false`다. imported 문서가 child MIC 경로
(`material.sourceMaterialPath`)만 갖고 parent/profileId enrichment는 별도 contract 단계 산출물인데,
그 산출물이 저장소에 없다.

corpus에서 같은 child material의 enrichment를 수확해 복사하는 방법을 검토했으나 기각했다.

```text
candidate 134행 기준
  단일 enrichment 존재 :  46행
  여러 버전 충돌       :  54행   (fx_a_pa_db_01_1_ad 는 16버전)
  corpus 에 없음       :  34행
```

같은 child 경로에 최대 16개의 서로 다른 sourceProfile이 존재하므로 무엇이 맞는지 결정할 수 없다.
가이드 17절의 `근거 없이 가정하지 않는다`에 따라 비워 두고, enrichment를 다음 단계로 남긴다.
따라서 이 134행은 현재 grouped 경로로 그려진다. 형태와 carrier를 먼저 보고, material 정확도는
승인 이후 단계다.

## 4. 자동 검증 (실행함)

| 검증 | 결과 |
|---|---|
| Client x64 Debug build | PASS (`Client.exe` 링크) |
| `Publish-Effects.ps1 -Mode Validate` | PASS 207 entries |
| `audit_typed_source_profile_join.py` | classifier pair 40, dead identity join 0 |
| `test_build_lancemaster_34610_source_restoration_candidates.py` | 6 tests OK |
| `test_audit_typed_source_profile_join.py` | 4 tests OK |
| `Sync-EffectDataProject.ps1` + registration harness | PASS files=1863 |
| `git diff --check` | 경고 없음 |

carrier 복사 불변식은 test로 고정했다. 복원 element의 `kind`, `rendererShape`, `renderProfile`,
`resources`가 source 변환본과 전부 일치하지 않으면 실패한다.

## 5. 수동 검증 (사용자 전용)

```text
Effect Tool -> effect.lancemaster.skill.34610.restoration-candidate  (Tool 전용, 134행)
Solo Element 로 하나씩 보고 Product 에 넣을 것만 승인한다.
```

승인 전까지 Product 3개 문서는 변경하지 않는다. `manual first pixel`, `visual PASS`는 기록하지 않는다.

## 6. 다음 단계

1. 승인된 후보만 Product 문서로 이동
2. 134행의 source material enrichment — corpus 수확이 아니라 정본 contract 경로로
3. 남은 grouped family: `fx_mm_distortion_01_ad`, `fx_j_me_flamesurface_01_ma`, `fx_mm_simple_03_tr`
