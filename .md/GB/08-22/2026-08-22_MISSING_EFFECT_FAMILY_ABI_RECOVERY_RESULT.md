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

두 generated file은 새 texture parity와 input/receipt hash를 반영했다. exact cooked variant 수 5와
Product runtime 수 0은 유지된다. 불완전한 sampler 후보를 더 이상 자동 보완하지 않으므로 complete
preview sampler와 authoring preview candidate 수는 0이다. Glass variant에는 exact NoDensity VS
sidecar 한 개가 추가되어 content-addressed DXBC blob 총수는 6개다.

새 VS blob `defae822...995b.dxbc`는 6,500 bytes이며 `Client.vcxproj/.filters`의
`96.DataFiles/Effects/CookedShaders`에 등록했다. exact material-map receipt가 VF 증거 추가로 바뀌었으므로
source-value target manifest의 upstream raw/seal identity도 같은 변경 단위에서 갱신했다.

## 5. 실행한 검증

### PASS

```text
extract_ue3_material_texture_sampler_closure.py
  targets=5 bindings=24 textures=23 samplerExact=0

materialize_ue3_exact_cooked_shader_variants.py
  variants=5 blobs=6 previewCandidates=0 runtime=0

test_extract_ue3_material_texture_sampler_closure.py
  Ran 8 tests, OK

test_evaluate_ue3_material_uniform_expressions.py
  Ran 10 tests, OK

test_extract_ue3_material_shader_maps.py
  Ran 28 tests, OK

test_materialize_ue3_exact_cooked_shader_variants.py
  Ran 13 tests, OK

extract_ue3_material_texture_sampler_closure.py --check
  PASS

materialize_ue3_exact_cooked_shader_variants.py --check
  PASS
```

### 미실행

- C++/HLSL 변경이 없으므로 이 변경 단위에서 Client Debug/Release build는 아직 실행하지 않았다.
- raw Glasshole authoring canary를 아직 admission하지 않아 first draw와 사용자 visual 판정을 요청하지 않았다.

## 6. 다음 수직 슬라이스

G03-6은 sampler의 source-exact 부분 증거, Glass 대상 scalar packing과 source emitter VF/NoDensity VS
선택을 닫았다. 남은 일은 같은 조사를 반복하는 것이 아니라 exact PS DXBC의 198 instruction을
`Shade_Ue3Glasshole02` HLSL로 번역하는 G03-7이다.

1. raw PS의 CB0/CB2, `t0..t7/s0..s7`, 여덟 PS varying과 RT0/2/3/4/5 계산을 line-order translation으로 보존한다.
2. raw DXBC와 translated HLSL을 동일 deterministic/source-value 입력으로 WARP 실행해 constant·spatial parity를 봉인한다.
3. Product/Tool draw에는 raw DXBC를 직접 승격하지 않고 translated RT0-only Glass shader만 사용한다.
4. scene depth, clip position, tangent-space view vector, alpha blend와 read-only depth를 typed renderer 입력으로 연결한다.
5. 차원술사 W의 `authored.source-particle.40e1b48e2f0f88dcfeff1549` 한 행만 기본 OFF canary로 실행한다.
6. Debug/Release build와 focused first-draw harness 뒤 사용자가 Effect Tool/인게임 화면을 판정한다.

이 canary가 통과하면 같은 family/permutation을 쓰는 차원술사 F occurrence에 동일 HLSL 계약을 확대하고,
그 다음 DragonMasked, MakeFlow02, LocalDecal 순으로 같은 복원 절차를 반복한다.

## 7. 사용자 수동 검증 상태

```text
Glasshole single occurrence visual    PENDING — canary 미연결
Lance V screw                          PENDING — ShaderMap 미복구
Lance T dragon                         PENDING — DragonMasked ABI 미복구
Warlord T LocalDecal                   PENDING — Product carrier 미복구
Artist T CModel dragon animation       PENDING — 별도 animation slice
```

사용자의 서면 관찰 전에는 어느 항목도 visual PASS로 기록하지 않는다.

## 8. G03-6 source ABI 조사 결과

### 8.1 sampler CDO

공식 v975 Engine package에서 `Default__Texture`와 `Default__Texture2D` CDO를 직접 복구했다.
`Default__Texture`는 `SRGB=true`, `Filter=TF_Linear`를 명시하며 `Default__Texture2D`는 이를
override하지 않는다. 같은 package의 `TextureFilter` enum은 `TF_Nearest`, `TF_Linear`, `TF_MAX`
세 값뿐이므로 기존 `TF_Default` 후보는 이 revision에 존재하지 않는다.

검증용 package는 임시 다운로드 위치에서 다음 안정 evidence 경로로 source-exact bytes를 복사했다.

```text
C:/Users/user/Desktop/Resource_LostArk/01_Extracted/Effect/ARTIST/
31470_TrackA_20260812/OfficialRefShaderCacheV974/NE1FENCQ4UNE9ZPRENOQS.v975.u
bytes   1,397,047
sha256  3b6de4c2cf785174d3cbeb8c9b31bdec846bf51737bcee0887160dcf64d58c3f
```

Glasshole의 7 texture는 color-space와 filter selector를 모두 확정할 수 있다. Address는 14축 중
명시된 `AddressV=Clamp` 한 축만 exact이며, 6 texture의 LODGroup은 Effects, cybernoise 한 개의
native LODGroup은 미확정이다. 보호된 TextureLODSettings 때문에 `TF_Linear`에서 최종 D3D filter로
가는 mapping도 아직 source-exact가 아니다. 따라서 full sampler admission은 계속 false다.

### 8.2 scalar CB packing

Glasshole exact PS native wire는 scalar group `1,7,8,9,10,11,12,13`을 CB0 `c14..c21`에
연결한다. source-value scalar count는 56이고 선택된 각 group의 네 lane이 모두 실제 scalar 범위 안에
있어 이 target에는 partial-group padding 문제가 없다.

```text
c14 [2.5, -15, -7.5, 1]
c15 [0, 0, -7, 0.2]
c16 [0, 2, 1, 0.3]
c17 [0, 0, 1, 0.1]
c18 [0, 2, -0.8, 1]
c19 [1, 0, 0, 0]
c20 [0, 0, 0, 2]
c21 [5, -0, 0, 0.8]
```

128-byte little-endian payload SHA-256은
`23e40670db53319d0c8a013b9f63866c4ed6aa16afdb4837db3a9c0c242e356d`다. Epic packed scalar
ABI와 일치하지만 LostArk v975 evaluator 본체를 동일 revision source로 확보하지 못했으므로
`EPIC_PACKED_SCALAR_ABI_CORROBORATED_AND_GLASSHOLE_PADDING_FREE`로 한정하고 일반 source-exact
승격은 하지 않는다.

### 8.3 actual emitter VF와 pass

현재 Product canary occurrence의 sourceRecipe에는 Required `boffsetcenter=true`,
`screenalignment=psa_rectangle`와 4-lane DynamicParameter `updateflags=15`가 함께 존재한다. 이 조합은
ShaderMap 후보 중 `FParticleOffsetCenterDynamicParameterVertexFactory`를 선택한다.

같은 VF 행의 bounded NoDensity BasePass VS는 다음 exact blob이다.

```text
type    TBasePassVertexShaderFNoLightMapPolicyFNoDensityPolicy
id      2dd6d96a7e6c974fac82106409a5b9b8
bytes   6,500
sha256  defae822f429760d30e0bfd31cef8c1217af8d7e43f794be6b42e85e6552995b
profile vs_5_0
CB      CB0[27] + CB1[5]
```

VS output은 Glass PS의 여덟 TEXCOORD input을 모두 공급한다. 다만 raw VS 실행에는 UE3 vertex stream과
CB0/CB1 packing이 더 필요하다. 첫 canary는 exact VS를 oracle blob으로 보존하고 translated sprite
bridge가 varying을 재구성하는 경계로 진행한다. scene fog/density와 Product pass는 승인하지 않는다.
