# 2026-08-21 Character Source-Exact Effect 및 두루미 복원 결과

## 0. 결과 요약

이 문서는 같은 shared dirty worktree에서 진행 중인 캐릭터 Effect 복원의 실제 구현 상태를
`구현 반영`, `자동 검증`, `사용자 수동 검증`으로 나눠 기록한다. 다른 세션이 손튜닝한 authored
행은 파일 단위로 되돌리거나 교체하지 않았다.

| 대상 | 구현 반영 | 자동 검증 | 사용자 화면 |
|---|---|---|---|
| DimensionMaster S 2050220 | sprite 2행 selective restore | JSON/identity, runtime row 1, Release build PASS | 미실행 |
| DimensionMaster W 2050120 clip 3 | vertical mesh 4행 selective restore | JSON/identity, runtime row 1, Release build PASS | 미실행 |
| DimensionMaster R 2050180 | swing mesh 3행 selective restore | JSON/identity, runtime row 1, Release build PASS | 미실행 |
| DimensionMaster D 2050240 clip 2 | 기존 cone mesh와 사용자 튜닝 보존, 새 행 없음 | JSON/identity, runtime row 1 PASS | 사용자가 튜닝 중, PASS 아님 |
| DimensionMaster F 2050230 | exact cloud 두 행용 bounded profile 34 구현 | C++/HLSL, runtime row 1, Release build PASS | 미실행 |
| Artist E 31480 crane | exact model/animation/projectile cue 연결 | receipt/shader, runtime row 1, Release build PASS | 미실행 |
| Artist E 31480 glow stroke | FlowRibbon 3행, profile 35, exact color/dynamic curve와 붓 끝 anchor | post-save identity/resource/float32 anchor PASS | 미실행 |
| Artist Z 31050 | 회전·축소 음양 emitter39, Simple02 profile 38 | post-save identity/role/scalar PASS | 미실행 |
| Artist T 31950 미르 새김 | MakeFlow03 mesh 3행, profile 16 | post-save identity/role/scalar PASS | 미실행 |
| Artist R 31210 BA4 | source event occurrence 68행 | unique ID/source node/event 분모 PASS | 미실행 |
| DimensionMaster T 2050500 | Authored 보존, exact MASKED pass 3 | focused T isolation PASS, current JSON read-only | 미실행 |
| LanceMaster V 34610 | 공용 MakeFlow/WaterTrail/ParticleMaster/SpriteWave admission 확장 | shader/TU PASS, missing mesh·TrailGhost는 fail-close | 미실행 |
| Warlord T 17240 | 공용 family admission + RGB-noise screenPost 1행 | source identity/consumer/shader PASS | 미실행 |

캐릭터 slice 전용 새 harness는 추가하지 않았다. 기존 publisher, corpus/runtime check, shader compile,
Client build를 재사용하는 계획을 유지한다.

### 0.1 사용자 저장 뒤 현재 기준선

- Artist E `31480`은 현재 `7 elements + 1 modelCue`다. 사용자가 정리한 non-target 32행은
  복원하지 않았다. FlowRibbon 3행과 crane cue는 모두 보존됐다.
- 세 ribbon은 `b_wp_1 + [0.464471906, 0, 0]`을 사용한다. receipt의
  `0.4644718933105469`와의 차이는 float32 직렬화뿐이며 bit contract가 같다.
- Artist Z는 `9 elements`, Artist T는 `23 elements`, Artist R BA4는 `68 elements`이며
  이번 exact 대상 row는 전부 cardinality 1과 유일한 source identity를 유지한다.
- 공용 profile은 Simple01 `33`, WaterTrail `14`, MakeFlow03 mesh `16`, MakeFlow02 mesh `36`,
  MakeFlow03 sprite `37`, Simple02 `38`까지 확장했다. Glasshole s-family는 named output lane이
  닫히지 않아 계속 fail-close한다.
- 전역 Effect Validate/Publish는 사용자 authored 저장과 경쟁하지 않는다. 최종 4-field runtime
  catalog publish는 별도 통합 단계다.

## 1. DimensionMaster selective restore

### 1.1 S 2050220

대상 문서:
`Data/Effects/Authored/effect.dimensionmaster.skill.2050220.unified.effect.json`

기존 사용자 15행은 유지하고 다음 source sprite 두 행만 추가했다.

```text
authored.source-particle.c3dbce9e43ebe73960d43c29
  fx_r_pa_spritewave_24_02_tr
  base       fx_m_trail_004_cl.dds
  dissolve   fx_e_ring_039.dds
  emissive   fx_d_atypical_028.dds
  noise      fx_m_noise_008.dds

authored.source-particle.7b61af226536a9545acd0c89
  fx_r_pa_spritewave_30_01_tr
  base       fx_m_trail_004_cl.dds
  dissolve   fx_r_symbol_swp_01_cl.dds
  noise      fx_d_noise_002.dds
```

따라서 S의 검격 carrier 판정은 `mesh particle`이 아니라 `sprite particle`이다. 질문에 나온
`fx_f_symbol_042.dds`는 이 S occurrence의 근거가 아니어서 추가하지 않았다.

보존 증거:

```text
restore 전 사용자 행 수: 15
restore 후 행 수: 17
기존 15행 semantic hash:
15becfd7181046de2e5f301b96aa1e09c57a060ebeed44a3e0eab8dcfb5f520d
추가한 두 stable ID cardinality: 각각 1
```

### 1.2 W 2050120 clip 3

대상 문서:
`Data/Effects/Authored/effect.dimensionmaster.skill.2050120.clip3.unified.effect.json`

기존 사용자 10행은 유지하고 다음 네 vertical mesh occurrence만 추가했다.

```text
authored.source-particle.0d483237146fa5d618d39a7f  start 0.25s
authored.source-particle.da2a381f72e08dbbe43e42f3  start 0.35s
authored.source-particle.0b38a1bdf89659a65e1ed571  start 0.45s
authored.source-particle.798b11b002a959023824d209  start 0.55s
```

공통 source packet은 다음과 같다.

```text
mesh       Effect/DimensionMaster/Meshes/fm_m_helix_011.wmodel
child      fx_k_me_spritewave_01_45_ad
parent     fx_m_pa_spritewave_01_ad
base       fx_m_spark_001.dds
dissolve   fx_h_noise_001.dds
noise      fx_a_noise_008_n.dds
```

보존 증거:

```text
restore 전 사용자 행 수: 10
restore 후 행 수: 14
기존 10행 semantic hash:
2c68e474bf7b7b3b0d187cfecdaa585120bd9608e460ba80ec7917627e12d5da
추가한 네 stable ID cardinality: 각각 1
```

### 1.3 R 2050180

대상 문서:
`Data/Effects/Authored/effect.dimensionmaster.skill.2050180.unified.effect.json`

한 mesh가 캐릭터 축을 계속 도는 표현 대신, 원본 source occurrence의
`fm_h_swing_05.wmodel` 세 행을 독립적으로 복원했다.

```text
authored.source-particle.cc4d20091ad0ed409617f51f  0.50s
authored.source-particle.333341329ff3992ea8c7f0a1  0.70s
authored.source-particle.f2e42e062ca87d93f0a477e3  1.05s
```

세 행 모두 revolution은 0이며 child `fx_w_pa_spritewave_01_05_tr`, base
`fx_m_trail_007.dds`, dissolve `fx_i_atypical_03_1_ycl.dds`, noise
`fx_d_noise_030.dds`를 쓴다. 시작부의 회전 helix는 별도 carrier이므로 이 세 검격을 대신하지 않는다.
현재 문서는 15행이며 세 stable ID는 각각 정확히 한 번 존재한다.

### 1.4 D 2050240 clip 2

대상 문서:
`Data/Effects/Authored/effect.dimensionmaster.skill.2050240.clip2.unified.effect.json`

다음 기존 행은 이미 cone mesh/diffuse/noise packet을 소유한다.

```text
id         authored.source-particle.feb86706b66f889097dcf610
mesh       Effect/DimensionMaster/Meshes/fm_d_cone_007.wmodel
child      fx_o_me_master_01_04_ds_ad
parent     fx_d_pa_master_01_ad
base       fx_a_atypical_048.dds
noise      fx_i_shockwave_02_ycl.dds
```

이 행의 transform은 사용자가 문서 작업과 동시에 계속 조정하고 있어 이번 작업에서 고정값으로
되돌리지 않았다. 현재 존재/리소스 identity만 확인했으며, 시각 완성이나 튜닝 완료로 판정하지 않았다.

## 2. DimensionMaster F 사각 card와 profile 34

### 2.1 원인

cloud card 두 행은 TypeData sprite이며 다음 exact 조합을 쓴다.

```text
authored.source-particle.1ae3416ac205fee634b746a9
authored.source-particle.ed33fb10661afb8854e76957
child      fx_m_mi_w_00.mi.fx_w_pa_fd_01_3_tr
parent     fx_mastermaterial.fx_mm.fx_mm_fluid_01_tr
base       fx_d_cloud_035.dds
emissive   fx_o_glass_01.dds
```

이 DDS의 불투명 alpha와 전체 card coverage를 generic grouped-translucent 식이 그대로 사용하면서
사각 경계가 드러났다. 원본 parent의 transition/noise/opacity equation을 실행하지 않은 상태이므로
noise texture 하나의 누락만으로 설명할 수 없다.

ground card `authored.source-particle.46bdb5dbec23f48e6c3f1a87`는 별도 glasshole 계열이다.
`fx_s_atypical_006.dds`만으로는 source crack normal과 radial graph가 닫히지 않으므로 cloud 두 행과
같은 수정으로 완료 처리하지 않았다.

### 2.2 반영

`Client/Public/Effect_MaterialTemplate.h`는 strict `ARTIST_MM_FLUID01` 판정에 위 exact child를
추가했다. `Client/Private/Effect_DocumentRenderer.cpp`는 이 child, sprite carrier, 정확히 두
resource가 모두 맞을 때만 source material profile 34를 선택한다.

profile 34는 다음만 구현한다.

- exact base/emissive 두 texture를 linear source lane 0/1에 고정
- source dynamic parameter의 transition, alpha power, fresnel alpha semantic 연결
- transition luminance, soft threshold, line gate, radial envelope로 coverage 생성
- 불투명 DDS alpha가 quad 전체를 노출하지 않도록 fail-close

이는 native `fx_mm_fluid_01` graph가 아니다. parent noise closure, native sampler/register/pass ABI,
ground glasshole의 crack-normal/radial closure는 남아 있다.

## 3. Artist E 31480

### 3.1 두루미 model cue

새 receipt:
`Data/Effects/Imported/Artist/CurrentCombat/skill.31480.crane-model-cook.receipt.json`

```text
source mesh:
  SK_SDM_RCC_00.Mesh.SK_SDM_RCC_00_SK_FX_01
source package SHA-256:
  504c8594e528e7c687be8b9e8a2f2cecd042df84f3ae60789fbaa8329f68da86
runtime WModel:
  Effect/Artist/Meshes/SK_SDM_RCC_00_SK_FX_01/SK_SDM_RCC_00_SK_FX_01.wmodel
runtime WModel SHA-256:
  cb96358dddf1bf749da1bc555973c8977ceaf21cf61a2bb4cd06ae5ba3ea6936
runtime clip:
  rcc_sk_flyinheaven
spawn/duration/velocity:
  1.0s / 1.11111116s / local +Z 9m/s
```

`effect.artist.skill.31480.unified.effect.json`에는 `artist_31480_crane_projectile` model cue가 있다.
기존 `CModel -> CMaterial` 경로에서 translucent alpha, source opacity/color reconstruction,
마지막 frame hold, local velocity를 사용한다. source mesh, animation, projectile timing/distance는 exact
identity로 기록했지만 material 식은 reconstructed이고 visual fidelity는 사용자 판정 대상이다.

### 3.2 glow ribbon

audit에서 원본 SHA가 일치하는 `YINYANGSHI.loa`의 action 31480 stage 0
`SK_FlyinHeaven`을 기준으로 다음을 확인했다.

```text
cue start/duration: 0.3006150126s / 0.899999976s
source follow: weapon rig B_Body_03
runtime anchor: b_wp_1 + [0.464471906, 0, 0]
visible carrier: TypeDataRibbon emitter 1, 11, 17
source-disabled cue: _01_02, _01_03 (복원 제외)

emitter 1/11:
  fx_k_flowrib_01_03_tr -> fx_k_flowrib_01_tr
  fx_i_atypical_03_ycl / fx_l_environment_001 / fx_d_noise_002
  rate 50

emitter 17:
  fx_k_flowrib_01_01_tr -> fx_k_flowrib_01_tr
  fx_k_auraline_02 / fx_d_noise_002
  SpawnPerUnit 1 / UnitScalar 30
```

초기 구현에서는 기존 36 element와 crane model cue를 보존하고 다음 세 stable row만 selective splice했다.

```text
authored.source-particle.8467a9b69e22208c6d4ea99e  emitter 1
authored.source-particle.a006dbe9b5650a0b13880aa5  emitter 11
authored.source-particle.c6d4247396f641316d28767c  emitter 17
```

사용자 저장 뒤 현재 문서는 `7 elements + 1 modelCue`이고 세 ID는 각각 정확히 한 번 존재한다.
FlowRibbon profile 35가 source color/alpha/width 및 `x_tiling/y_tiling/dissolve/disort` curve를 소비한다.
세 행은 runtime `b_wp_1`과 float32 anchor `[0.464471906, 0, 0]`을 사용하며, 이는 receipt의
`[0.4644718933105469, 0, 0]`과 같은 float32 계약이다. 현재 SHA-256은
`d88a42de717c875414c55a09ba26c6503ddea05c03c276941e1b348eb30a8180`이다.

historical `preserved36=True`는 최초 selective splice의 안전성 증거일 뿐 현재 cardinality가 아니다.
사용자가 제거한 non-target 32행은 의도를 추측해 복원하지 않았다. native UE3 VF/pass와 weapon 자체의
skeletal animation은 여전히 bounded 경계이고 visual fidelity는 사용자 미검증이다.

## 4. 넓은 스킬 범위의 미완료

아래 대상은 일부 source carrier와 공용 family가 연결됐지만 전체 occurrence의 제품 draw를 완료하지
않았다. 04:09 이후 authored 저장본은 최종 4-field full Publish 전이므로 현재 runtime에 게시됐다고
기록하지 않는다.

```text
DimensionMaster T 2050500
LanceMaster V 34610 clip 1/2/3
Warlord T 17240 BA1/BA2/BA3
```

DimensionMaster T는 exact MASKED pass 3/TRANSLUCENT pass 4와 단일 occurrence 경계를 유지한다.
LanceMaster V는 missing mesh 8 occurrence와 TrailGhost 2개, Warlord T는 unresolved Light와 film/zoom
post가 fail-close다. 문서 존재나 family 부분 입장을 전체 스킬 복원 완료로 세지 않는다.

## 5. 자동 검증 증거

### 5.1 PASS

| 검증 | 결과 |
|---|---|
| S/W/R/D/F/Artist E authored JSON parse | PASS |
| S restore stable ID 2개 cardinality | 각각 1 |
| W restore stable ID 4개 cardinality | 각각 1 |
| R restore stable ID 3개 cardinality | 각각 1 |
| D cone stable ID cardinality | 1 |
| Artist E glow ribbon stable ID 3개 cardinality | 각각 1 |
| Artist E 기존 36행 deep equality와 unique ID 검사 | `preserved36=True`, PASS |
| Artist E authored JSON `git diff --check` | PASS |
| Debug ClientFrontendHarness `--effect-document-codec-sha`의 31480 v13 C++ codec/canonical hash | PASS, `25f390781a8565f105adfd954e7d73d1dc6d3013a25c196d21432569bc264557` |
| Artist crane receipt JSON parse와 identity/hash 검사 | PASS |
| crane용 `Shader_VtxAnimMeshBinary.hlsl` compile checkpoint | PASS |
| crane 변경 뒤 Client Debug `/t:ClCompile` checkpoint | PASS |
| profile 34 C++ 변경 뒤 Client Debug `/t:ClCompile` | exit 0 |
| profile 34 threshold normalization 뒤 `Shader_VtxEffectParticle.hlsl` `fx_5_0` 최종 compile | exit 0 |
| 04:09 Full `Publish-Effects.ps1 -Mode Publish` | exit 0, Effects 192, Components 1, visual-program sidecar 파일 생성 |
| Runtime Artist E 31480 row/content SHA | row 1, `4eb78357f10d3d51675703341427252d25435adfeaf716e2d5ea34107766b0fb` |
| Runtime DimensionMaster S row/content SHA | row 1, `a29b5ecb080cd73882bc1ba4db0ad526f29c7c22319410a080e6d95622112e93` |
| Runtime DimensionMaster W clip 3 row/content SHA | row 1, `0b8a63af3edd42a7bb0cfc25b066b823ec40e90ad1b8b39478332a52e4ce1783` |
| Runtime DimensionMaster R row/content SHA | row 1, `ab47c56d2e6963c7261872df394d52e7704c511804f4f68d57d1f80adf2c1e81` |
| Runtime DimensionMaster F row/content SHA | row 1, `2c633f89e8aefff57957a3b83907eb66afe7ea142c746a28be54f40de002ec56` |
| Runtime DimensionMaster D clip 2 row/content SHA | row 1, `4de22367cfd2c2a3f0df0a64fd8095ded72ff785ed1e4ef0266d697b21d1fc62` |
| Client Release full Build | exit 0, `Client.exe` 생성, warnings only |
| 현재 공통 Effect HLSL full/VS/PS 10조합 | exit 0, errors 0 |
| 현재 Catalog/Renderer/MainApp Debug 선택 TU | exit 0, errors 0 |
| 격리 Engine/Shared rebuild + Client Debug relink | exit 0, errors 0 |
| 격리 ClientFrontendHarness build/relink | exit 0, errors 0 |
| 격리 `--effect-dm-r-boundary-fast` | `failures: 0`, exit 0 |
| 현재 Artist E/Z/T/R post-save identity | 7+cue / 9 / 23 / 68, exact 대상 보존 PASS |

profile 34 이전 상태에서 Effect HLSL 5개 `fx_5_0` compile이 통과했고, 이후 threshold normalization이
들어간 `Shader_VtxEffectParticle.hlsl`도 별도로 다시 컴파일해 exit 0을 확인했다. 따라서 profile 34의
현재 Particle shader compile gate는 PASS다.

### 5.2 Runtime 단순화 뒤 Publish 경계

04:09 snapshot에서 full publish 실행 자체는 성공했다.

```text
Publish-Effects.ps1 -Mode Publish
exit 0
PASS: published 192 Effects, 1 Components, visual-program sidecar
catalog LastWrite:  2026-08-21 04:09:04
sidecar LastWrite:  2026-08-21 04:09:04
```

그러나 이 게시본의 direct 191행은 모두 전환용 legacy 6-field이고 현재 authored 저장보다 오래됐다.
runtime 단순화 뒤 publisher 정본은 `payloadKind/effectAssetId/authoringFormatVersion/authoredDocumentPath`
4필드다. loader는 기존 6필드를 전환 입력으로 읽되 direct SHA/dependencies는 사용하지 않는다.
formatVersion 3 VisualProgram sidecar는 `visualProgramSidecarRequired=false`여도 항상 parse/validate/stage한다.
따라서 marker `true`는 더 이상 제품 9/9의 완료 조건이 아니다.

Debug Effect Tool, selected publish와 ProductCue approval/admission 계약은 제거됐다. 현재 Artist/Valtan
authored 저장본을 반영하는 final full Publish는 다른 저작 저장이 자연스럽게 끝난 뒤 별도 통합 단계에서
한 번 실행한다. 이번 단계에서는 경쟁하는 global Validate/Publish를 실행하지 않았다.

### 5.3 남은 경계

- 모든 authored 저장 종료 뒤 4-field Full Effect Publish와 Client 재시작
- 전체 Client link는 별도 통합 빌드에서 확인. 현재 선택 TU/HLSL은 오류 0
- Debug Server 재시작과 새 gameplay/item bootstrap 적용
- S/W/R/F/Artist E Product draw 및 실제 visual fidelity
- DimensionMaster T visual, LanceMaster V/Warlord T의 fail-close occurrence 후속 복원

Client 제품 Release full Build의 historical checkpoint는 exit 0이다. 현재 리비전은 공통 HLSL 10조합과
Catalog/Renderer/MainApp 선택 TU를 다시 컴파일해 오류 0을 확인했다. 실행 중 Client를 종료하지 않고
별도 출력 경로에서 Engine/Shared를 같은 스냅샷으로 재빌드한 뒤 Client Debug와 frontend harness도
오류 0으로 링크했다. R focused 실행은 `failures: 0`이었다. 실제 주 Client 출력 교체와 재시작은 최종
4-field publish 통합 단계다.

## 6. 수동 검증 경계

에이전트는 Client를 실행·조작하거나 화면을 캡처하지 않았다. 따라서
`manual first pixel`, `eye smoke`, `visual PASS`, 원본과 같은 형태·색·타이밍·밀도·궤적은 모두
미판정이다.

자동 검증이 끝난 뒤 사용자가 직접 다음 대상들을 분리해 확인해야 한다.

```text
DimensionMaster S: 검격 sprite 두 layer의 존재와 방향
DimensionMaster W: 0.25~0.55초 네 vertical mesh occurrence
DimensionMaster R: 0.50/0.70/1.05초 세 검격, character-axis 상시 회전 부재
DimensionMaster D: 현재 사용자 transform의 cone mesh/diffuse/noise
DimensionMaster F: cloud 두 card의 사각 경계, 별도 ground glasshole card
Artist E: body-follow glow ribbon 세 carrier와 1.0초 두루미 model cue
```

사용자의 서면 관찰 전에는 어느 항목도 시각 복원 완료로 기록하지 않는다.
