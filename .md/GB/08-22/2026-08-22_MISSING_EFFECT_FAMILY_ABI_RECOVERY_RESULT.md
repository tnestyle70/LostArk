# Missing Effect Family ABI 복원 결과

branch: `codex/missing-effect-family-recovery`

base: `origin/main@7fb8f8139f62657914228070ebe2a9860287b577`

plan: `2026-08-22_MISSING_EFFECT_FAMILY_ABI_RECOVERY_IMPLEMENTATION_PLAN.md`

## 1. 현재 결론

unknown family 복원은 불가능하지 않다. 차원술사 W glass 계열의 기존 작업은 byte/hash 또는 DXBC
추출에서 실패한 것이 아니라, exact PS 뒤쪽의 sampler/default, scalar CB packing, actual VF/pass와
Product binding에서 멈춘 상태였다.

이번 변경 단위에서는 첫 canary인 Glasshole02가 요구하는 runtime DDS의 마지막 두 누락을 source-exact
bytes로 배포했다. 그 결과 W의 exact family 5개 전부가 runtime DDS parity를 통과한다.

```text
exact ShaderMap/DXBC target                 5 / 5
source-exact texture binding target         5 / 5
runtime DDS parity target                   5 / 5
source-exact sampler target                 0 / 5
source-value texture/sampler target         0 / 5
actual VF/pass target                       0 / 5
Product runtime target                      0 / 5
visual admission                            false
```

따라서 이번 결과는 texture byte closure 완료이며 Glasshole 원본 material 또는 Product 복원 완료가 아니다.

## 2. family 감사 결과

### 2.1 차원술사 W glass

Glasshole02, FluidNinja01, CustomParticle01, CrackholeV2와 SpriteWave는 exact ShaderMap join,
content-addressed PS DXBC, native binding wire, structural WARP replay, source-value uniform expression과
texture binding까지 성공해 있다. Slice01만 effective native static-set/permutation 증거가 없어 exact
map 선택 전에서 차단된다.

현재 Product W clip3은 과거 21행이 아니라 13행이다. 현재 살아 있는 exact-target cohort는 Glass 1,
Fluid 1, Slice 1, Helix 4행이다. 과거 receipt의 10 occurrence를 현재 Product element 수로 오해하지 않는다.

첫 canary stable ID는 다음이다.

```text
authored.source-particle.40e1b48e2f0f88dcfeff1549
```

### 2.2 창술사 V/T와 도화가 T

- 창술사 V screw는 Sprite가 아니라 `fm_m_helix_006.wmodel` MakeFlow02 MeshParticle twin이다.
- 창술사 T dragon도 Sprite가 아니라 두 WModel을 쓰는 DragonMasked MeshParticle이다.
- DragonMasked 원본은 `BLEND_Masked`, one-sided, clip `0.166`인데 현재 grouped alpha two-sided다.
- 도화가 T effect의 선회선은 MakeFlow03 MeshParticle 3행이다.
- 미르세김 용 본체는 effect element가 아니라 CModel/animation owner다.

따라서 MakeFlow, DragonMasked와 CModel 용 애니메이션은 서로 다른 수직 슬라이스다.

### 2.3 워로드 F/T

- 워로드 F Product 56행은 모두 Particle이며 원본 Light/ScreenPost는 현재 Product에서 빠졌다.
- F의 DecMaster 이름 5행도 carrier는 SpriteParticle이므로 texture를 바꿔 LocalDecal로 만들 수 없다.
- 풀배럴 T BA3 원본에는 emitter55~58 LocalDecal 네 행이 있으나 현재 Product에는 0개다.
- 풀배럴 복원은 LocalDecal carrier와 DecMaster01 material equation을 분리해야 한다.

## 3. 배포한 source-exact runtime DDS

두 파일은 Git 제외 공유 runtime root에 배포했다.

| runtime asset ID | bytes | SHA-256 |
|---|---:|---|
| `Effect/DimensionMaster/Textures/FX_TEX_02/fx_d_atypical_094_ycl.dds` | 65,664 | `8097e1011480df43f56ad42a0ab849c74b9d8a29c17c867556f1df68dd071041` |
| `Effect/DimensionMaster/Textures/FX_TEX_04/fx_f_aura_004_1.dds` | 32,896 | `80a7797447d457de7e56594951e2d91c12899d144a7e0c730a4a8da14fdca896` |

source는 다음 exact Glasshole family export cohort다.

```text
C:/Users/user/Desktop/Resource_LostArk/05_Reports/EffectExtraction/
FourClassMaterials/export/175266c16bb27e04/
```

배포 대상은 다음 공유 정본이다.

```text
C:/Users/user/Desktop/LostArk/Client/Bin/Resources/
```

복사 전 target 부재를 확인했고 복사 후 byte 수와 SHA-256을 다시 검사했다. 기존 파일을 덮어쓰지 않았다.

## 4. source 변경

### 4.1 texture/sampler closure validator

`extract_ue3_material_texture_sampler_closure.py`의 tracked corpus invariant를 과거 missing 상태 4/5에서
현재 source-exact runtime parity 5/5로 갱신했다. sampler와 runtime/visual admission false guard는
그대로 유지했다.

동시에 tracked repository input의 절대 checkout 경로를 receipt digest에 넣던 부분을 repository-relative
path로 교정했다. 외부 immutable source 경로는 absolute provenance를 유지한다. 따라서 같은 commit을
main checkout과 별도 worktree에서 재생성해도 tracked input path 때문에 receipt가 달라지지 않는다.

### 4.2 focused test

tracked receipt test가 다음을 고정한다.

- summary `runtimeDdsParityTargetCount == 5`
- Glasshole target `runtimeDdsParityAdmission == true`
- tracked extractor path는 `Tools/EffectPipeline/...` repository-relative
- runtime/visual admission은 false

### 4.3 generated receipts

- `skill.2050120.clip3.exact-texture-sampler-closure.receipt.json`
- `ue3-exact-cooked-shader-variants.v1.json`

두 generated file은 새 texture parity와 input/receipt hash를 반영했다. exact cooked variant 수 5,
preview candidate 수 2, Product runtime 수 0은 유지된다.

## 5. 실행한 검증

### PASS

```text
extract_ue3_material_texture_sampler_closure.py
  targets=5 bindings=24 textures=23 samplerExact=0

materialize_ue3_exact_cooked_shader_variants.py
  variants=5 blobs=5 previewCandidates=2 runtime=0

test_extract_ue3_material_texture_sampler_closure.py
  Ran 5 tests, OK

test_materialize_ue3_exact_cooked_shader_variants.py
  Ran 9 tests, OK

extract_ue3_material_texture_sampler_closure.py --check
  PASS

materialize_ue3_exact_cooked_shader_variants.py --check
  PASS
```

### 미실행

- C++/HLSL 변경이 없으므로 이 변경 단위에서 Client Debug/Release build는 아직 실행하지 않았다.
- raw Glasshole authoring canary를 아직 admission하지 않아 first draw와 사용자 visual 판정을 요청하지 않았다.

## 6. 다음 blocker

다음 세 항목을 독립적으로 조사한다.

1. source revision Texture2D CDO와 TextureLODSettings를 통한 Address/SRGB/`TF_Default` 해석
2. native scalar expression group의 CB0 lane order와 padding
3. Glasshole Sprite occurrence의 actual ParticleVF/BasePass varying, scene-depth와 renderer-owned CB row

세 항목이 닫힌 뒤 raw exact PS를 단일 Authored canary로 연결한다. 이때 Product 기본 경로는 계속 false로
유지하고, structural/source-value WARP parity와 Debug/Release first draw를 먼저 통과시킨다.

## 7. 사용자 수동 검증 상태

```text
Glasshole single occurrence visual    PENDING — canary 미연결
Lance V screw                          PENDING — ShaderMap 미복구
Lance T dragon                         PENDING — DragonMasked ABI 미복구
Warlord T LocalDecal                   PENDING — Product carrier 미복구
Artist T CModel dragon animation       PENDING — 별도 animation slice
```

사용자의 서면 관찰 전에는 어느 항목도 visual PASS로 기록하지 않는다.
