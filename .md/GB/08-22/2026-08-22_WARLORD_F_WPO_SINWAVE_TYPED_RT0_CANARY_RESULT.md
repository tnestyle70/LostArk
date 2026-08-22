# 워로드 F WPO SinWave typed RT0 canary 결과

branch: `codex/v1-cohort-warlord`

base: `main@dc280ff55cc49d7f92c0d84737fd939bbf440dca`

## 1. 결론

워로드 F `17140`의 원본 Product 56행과 사용자 손튜닝은 수정하지 않았다. 원본에서 살아 있는
WPO SinWave 전기 Mesh 두 행만 별도 direct-authored Tool 문서로 복제하고, source contract가 역할을
증명한 두 texture lane만 소비하는 class-neutral `RuntimeMaterialV2 opcode 22` RT0 계산을 연결했다.

```text
Product effect       effect.warlord.skill.17140.unified
Tool candidate       effect.warlord.skill.17140.wpo-sinwave-v1.unified
visual canary        authored.source-particle.8c0d6ab070c1a6c83479e590
data-only reuse      authored.source-particle.59e6ffa8852fba74279b6ae9
carrier              fm_d_electric_05_vertexcolor.wmodel
child MIC            fx_d_me_worldpositionoffset_sinwave_01_04_ad
parent Material      fx_d_me_worldpositionoffset_sinwave_01_ad
runtime              RuntimeMaterialV2 / opcode 22 / additive one-sided RT0
fidelity             PROJECT_RECONSTRUCTED / TYPED_RT0_BASE_ONLY
```

두 번째 occurrence는 첫 packet과 byte-for-byte 같은 data row다. 두 번째를 위해 C++이나 HLSL 분기를
추가하지 않았다. 따라서 이번 cohort가 확인하려는 확장 단위는 스킬명이 아니라
`child + parent + mesh carrier + render pass + proven lane set`이다.

## 2. Product 동결과 판정

| 항목 | 결과 |
|---|---|
| Product element 수 | `56` 유지 |
| Product byte SHA-256 | `52d8c29d3af8e8fdf27f32bd8882b25b56c6fab737a83a5bf87bbcc6ebec81ff` 유지 |
| source 190행 bulk restore | 수행하지 않음 |
| Product animevent/cue | 기존 `effect.warlord.skill.17140.unified` 유지 |
| Tool 후보 Product 연결 | 없음 |
| carrierDisposition | 미부여 (`null`) |
| reviewState | `USER_REVIEW_PENDING` |
| candidateAction | `ADD_OR_REPLACE_PENDING` |
| terminal KEEP/REPLACE/ADD/RETIRE | 사용자 승인 전 미부여 |

이번 후보는 `EffectCatalog.json`에 direct-authored v13 source로만 등록했다. Warlord animevent에는 후보 ID를
넣지 않았으므로 인게임 Product F를 교체하지 않는다.

## 3. 증명된 입력과 열린 입력

### 3.1 이번 RT0가 소비하는 입력

| source parameter | 원본 group | runtime asset | channel | runtime lane |
|---|---|---|---|---:|
| `21.map_c` | `01_alpha` | `fx_i_thunder_02_ycl.dds` | `R` | 0 |
| `02.map_e` | `02_emission` | `fx_d_atypical_049.dds` | `RGB` | 1 |

child scalar 중 alpha power/strength/UV와 emission power/desaturation/UV/pan, child emission color
`[5,5,5,1]`, source DynamicParameter Y의 `dissolve[0-1]`, ParticleColor RGBA만 packet에 넣었다.

### 3.2 `PENDING_EVIDENCE`

- parent default `06.map = fx_d_noise_009`
- parent default `12.map_f = fx_c_noise_002`
- `umodel_dependency = fx_d_symbol_030_cl`
- source DXBC equation과 native texture register wire
- source WPO vertex equation, native MeshParticle VF/pass
- source-exact sampler CDO/address/filter

위 항목은 존재를 추정해 빈 lane이나 다른 캐릭터 DDS로 채우지 않았다. 현재 VS는 기존 typed mesh
carrier의 정점 위치를 사용하므로 이름의 WPO는 source identity이고, 원본 sin-wave vertex displacement
복원 완료 주장이 아니다.

## 4. 구현

- `Shader_EffectUe3MaterialFamilies.hlsli`
  - opcode 22 packet shape를 고정한다.
  - thunder R에서 coverage/dissolve를 만들고 atypical049 RGB에서 additive radiance를 만든다.
  - invalid mask/count/finite value는 clip으로 fail-close한다.
- `Shader_VtxEffectMeshPreview.hlsl`
  - typed mesh 경로에서 opcode 22 하나를 class-neutral 함수로 dispatch한다.
- `Effect_DocumentRenderer.cpp`
  - 정확한 두 stable ID/sourceNode만 allowlist한다.
  - exact child/parent/profile, electric WModel, 세 resource binding, mesh sourceRecipe, Dynamic module,
    additive pass와 packet의 lane/channel/scalar/vector/mask 전부를 검증한다.
  - allowlist 밖 opcode 22 또는 tuple drift는 generic shader로 보내지 않고 stage를 거부한다.
- `materialize_warlord_17140_wpo_sinwave_canary.py`
  - Product를 read-only hash input으로 사용한다.
  - 두 행의 material execution 면만 candidate에 투영한다.
  - source contract와 runtime WModel/DDS hash를 다시 검사하고 deterministic receipt를 생성한다.
- focused Python test
  - Product freeze, 두 lane, candidate/catalog/no-cue, data-only reuse, disposition, mutation reject를 고정한다.

## 5. 자동 검증

### PASS

```text
test_materialize_warlord_17140_wpo_sinwave_canary.py
  Ran 10 tests, OK

materialize_warlord_17140_wpo_sinwave_canary.py --check
  current; changed=false; productRows=56; candidateRows=2; opcode=22

Sync-EffectDataProject.ps1 -Check
  PASS after registration update

fresh-checkout materializer replay
  PASS; HLSL/C++ implementation identity uses CRLF-to-LF canonical bytes

Publish-Effects.ps1 -Mode Validate
  PASS; validated 207 Effect catalog entries and visual-program sidecar

fxc.exe /E PS_MAIN /T ps_5_0 Shader_VtxEffectMeshPreview.hlsl
  PASS; output 187436 bytes
  기존 공용 함수의 X4000 경고만 존재하고 신규 compile error는 없음
```

Effect Publish와 Debug/Release 통합 빌드는 captain 통합 브랜치에서 재실행한다. 이 작업에서는
사용자 화면을 실행하지 않았고 visual PASS를 주장하지 않는다.

## 6. 사용자 수동 검증

자동 검증은 visual PASS를 승인하지 않는다. 사용자는 Effect Tool에서 Warlord F 아래
`effect.warlord.skill.17140.wpo-sinwave-v1.unified`를 열고 다음 순서로 확인한다.

1. 첫 행 `8c0d...`만 Solo하여 electric mesh가 사각형 카드가 아니라 의미 있는 전기 형태로 보이는지 본다.
2. alpha 경계, 밝기, dissolve 진행, 위치/크기/회전을 확인한다.
3. 두 번째 행 `59e6...`만 Solo하여 같은 family 계산이 별도 코드 없이 재사용되는지 본다.
4. 두 행을 함께 켜 원래 composition timing을 확인한다.

사용자 서면 승인 전에는 `V1_COMPLETE`, visual PASS 또는 Product 승격으로 기록하지 않는다.

## 7. 남은 정확한 visual blocker

가장 큰 blocker는 원본 WPO vertex displacement가 아직 실행되지 않는다는 점이다. RT0 색·coverage가
의미 있어도 전기 mesh의 진동 폭/속도/축은 정적인 typed carrier와 다를 수 있다. 다음으로 sampler와
누락 parent texture lane 때문에 표면 미세 왜곡과 두 번째 emission 층이 다를 수 있다. 첫 pixel에서
형태가 의미 있으면 사용자가 손튜닝으로 배치/밝기를 맞춘 뒤 후보의 ADD/REPLACE 여부를 결정한다.
형태 자체가 의미 없으면 Product에는 영향 없이 후보를 폐기하고 WPO VS/DXBC evidence 단계로 돌아간다.
