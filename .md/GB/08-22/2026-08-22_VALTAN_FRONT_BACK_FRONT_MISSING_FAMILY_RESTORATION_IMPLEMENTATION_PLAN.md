# 2026-08-22 Valtan 전체 Render Contract 복구와 3연격 Missing Family 복원 구현 계획서

## 0. 목표와 현재 실측

우선순위를 다음처럼 고정한다.

1. 모든 발탄 패턴 Effect를 4캐릭터와 같은 손튜닝 가능 상태로 복구한다.
2. 그 뒤 3연격을 첫 missing-family 육안 canary로 사용한다.

### 2026-08-22 carrier-first Product 재기준선 최종 결정

사용자 최종 결정에 따라 기존 clip-name aggregate V0 행은 더 이상 Product 기준선으로 보존하지
않는다. 다만 ID 모양만 보고 삭제하지 않고, 아래 네 항목이 모두 맞는 행만 legacy로 확정한 뒤
활성 Product에서 전량 제거한다.

```text
sourceNode == ""
+ element id의 .emNN ordinal
+ Product owner가 가리키는 원본 ParticleSystem exact join
+ N번째 material resourceBinding objectPath == element sourceMaterialPath
```

현재 boss-root cue Product만 보면 96문서 / 3,328 element 중 strict legacy가 3,022행이고,
Red Blade combat-object Product owner까지 포함하면 97문서 / 3,343 element 중 3,032행이다.
초기에는 나머지 311행을 보존할 예정이었지만, 최종 구현 결정은 더 엄격하다. 기존 boss-root
owner의 모든 행을 제거하고 exact join 행만 새 clip owner로 재구성한다. 기존 문서는
`elements=[]` evidence shell로 남기며 preimage hash와 이동/삭제 분모는 immutable receipt가
소유한다. 예외는 이미 검증된 Whirlwind active와 별도 combat-object owner뿐이다.

새 Product carrier 정본은 reviewed branch에서만 만든다.

| 분모 | 수치 | 처리 |
|---|---:|---|
| reviewed patterns | 24 / 33 | source branch가 사람 검토로 고정된 범위 |
| exact core clip occurrences | 45 clips / 24 patterns | reviewed 4중 exact join |
| exact carrier projections | 660 | Sprite 455 / Mesh 173 / Decal 32 |
| common-translucent Product projections | 657 | 보호 Whirlwind exact alias 3행 제외 |
| blocked projections | 917 | resource/adapter/dust/light blocker ledger-only |
| 미승격 patterns | 9 | 미검토 branch 7 + action binding 부재 2 |

660행은 occurrence full key와 carrier key로 각각 고유한 element를 만들고, clip-local cue clock을
사용한다. Mesh는 exact `Effect/Valtan/Meshes/**/*.wmodel` 한 개와 `modelPreScale=0.01`을,
Sprite/Decal은 meshModel 0개와 modelPreScale 필드 없음까지 고정한다. system 안에 mesh 하나가 있다는
이유로 다른 Sprite/Decal emitter에 그 mesh를 복사하는 fallback은 금지한다.

이번 carrier-first 단계에서는 아직 typed family가 닫히지 않은 657행도 원본
`sourceMaterialPath`, texture/resource lane, sourceRecipe를 보존한 채 공통
`alpha_two_sided_depth_read` RT0로 임시 실행한다. 이 상태의 명칭은
`CARRIER_V1_COMMON_TRANSLUCENT`이며 family 복원 또는 `V1_COMPLETE`가 아니다. 이후 exact
child/parent family가 닫히면 element identity와 손튜닝 transform/timing은 유지하고 material
executor만 교체한다.

Decal은 exact EF TypeDataDecal closure와 Base DDS가 닫힌 32행만 실제 Decal carrier로
승격한다. 반면 Light, ScreenPost 또는 runtime carrier adapter/resource가 닫히지 않은 행은 common
translucent로 위장하지 않는다. carrier shape와 source identity는 1,577행 전체 ledger에 남기되
drawable element로 만들지 않는다. 이 경계는 "미해석 material은 임시 translucent"와
"미구현 carrier를 임의 quad/mesh로 바꾸지 않음"을 동시에 만족시킨다.

2026-08-21에 추가된 source-occurrence/source-wave writer가 기존 Valtan WModel 계약을 다시
빠뜨렸다. 현재 canonical `effect.valtan.*` 113문서의 meshModel carrier는 2,130개이며,
`Effect/.../*.wmodel` 150개와 `Character/Valtan/ValtanWeapon.wmodel` 1개가 명시적
`modelPreScale` 없이 런타임 기본값 `1.0`을 사용한다. Effect WModel의 정답은 `0.01`, 캐릭터
도끼 모델의 정답은 `1.0`이다. 따라서 missing family 연구보다 이 151개를 먼저 계약 교정하고,
모든 이후 writer와 publisher에 같은 검사를 연결한다.

이번 변경의 첫 수직 슬라이스는 발탄 `VALTAN_FRONT_BACK_FRONT` 3연격이다. 원본
`Att_Battle_19_01` 한 clip occurrence를 유지한 채, clip 내부의 네 원본 visual beat를
각각 timeline cue로 재생한다. Server의 실제 damage hit는 기존 3회 권위를 그대로
유지하고 네 번째 visual beat는 source auxiliary presentation으로만 남긴다.

이 작업의 핵심은 `Translucent`, `Additive`, `Masked` 같은 blend 이름을 generic shader에
붙이는 것이 아니다. Product에서 사용하는 material variant의 정본 키는 다음 합성이다.

```text
exact child material
+ exact parent material and inherited defaults
+ static switch set
+ carrier kind / vertex factory
+ render pass and render state
+ recovered pixel equation
+ named texture/sampler/scalar/vector/dynamic-parameter ABI
```

따라서 같은 Translucent라도 SpriteWave, MakeFlow, WaterTrail, Crack, Ground Decal은 서로
다른 family다. 다만 carrier-first 재기준선에서는 exact carrier/resource/sourceMaterialPath가 닫힌
Sprite·Mesh·검증된 Decal만 명시적인 `CARRIER_V1_COMMON_TRANSLUCENT`로 실행할 수 있다. 이것을
family 복원으로 부르지 않으며, runtime carrier adapter 자체가 없는 Light/ScreenPost는 generic
plane이나 임의 mesh로 바꾸지 않고 fail-close한다.

### 2026-08-22 최종 범위 교정

사용자 최종 결정에 따라 이 세션은 Glasshole과 캐릭터 공용 family 작업을 중복하지 않는다.
첫 구현 단위는 3연격에서 실제 손튜닝을 막는 다음 세 source family의 증거, HLSL 번역,
Tool-only runtime ABI와 canary admission까지다. Firework/Smoke는 inventory와 번역 증거만
보존하고 이 수직 슬라이스의 renderer admission에는 포함하지 않는다.

| family | source identity | 실제 carrier | 첫 발탄 canary |
|---|---|---|---|
| Masked dissolve stone | child `fx_m_mi_n_00.fx_n_me_dissolve_04_011_ma` / parent `fx_m_mi_00.fx_m.fx_d_pa_dissolve_01_ma` | Mesh Particle + `fx_sm_00.fm_a_stone_001` | sourceOrder 7 |
| Ground decal | child `fx_m_mi_n_00.fx_mi.fx_n_de_ground_04_30_tr` / parent `fx_m_mi_04.fx_m.fx_d_de_ground_04_tr` | TypeDataDecal projector | sourceOrder 14 |
| Valtan crack translucent | child `fx_m_mi_o_00.fx_mi.fx_o_me_crack_01_01_tr` / parent `fx_m_mi_00.fx_m.fx_d_me_crack_01_tr` | Mesh Particle + source emitter mesh join | `Atk_04_11/12/13` 7개 |

여기서 Crack은 캐릭터 공용 `fx_j_me_localcrack_01_tr`와 다른 parent/equation이다. Ground도
`flocalvertexfactory`로 뽑은 임의 mesh PS가 아니라 원본 decal VF/pass를 선택해야 한다.
이 두 구분을 target manifest와 negative test로 고정한다.

`source exact`라는 완료 명칭은 아래 여덟 항목이 모두 닫힌 variant에만 사용한다.

```text
exact child/direct Material serial
+ parent/default inheritance and effective static set
+ selected cooked map and pixel program
+ translated HLSL versus original DXBC WARP A/B
+ source-value CB0 row packing
+ texture register / sampler register / address / filter / color space
+ actual carrier VF/pass and render state
+ class-neutral Product occurrence projection
```

세 family의 공식 pixel equation은 회수·번역하고 원본 DXBC와 WARP 수치 오차 0으로 닫는다.
그 뒤에도 sampler default와 실제 source VF/pass는 열린 경계이므로 첫 결과는
`SOURCE_EXACT_PRODUCT`가 아니라 `TOOL_RENDERER_RUNTIME_ADMITTED_BOUNDED_RT0`이다.
Product, 실제 VF/pass와 visual admission은 계속 false로 둔다.

### 2026-08-22 V1 완료선과 첫 확장 실험

위의 `source exact`/native parity 연구선은 V1 완료 조건이 아니다. 도화가 F에서 실제로 성공한
제품 기준을 발탄에도 그대로 적용하고, 다음 합성이 사용자 육안 승인을 받으면 `V1_COMPLETE`로
기록한다.

```text
정확한 element와 carrier
+ child/parent texture lane, channel, scalar, DynamicParameter 배선
+ family별 RT0 Base HLSL
+ blend/depth
+ attachment/timing
+ Effect Tool 저장·재로드 가능한 손튜닝
+ 사용자 서면 visual 승인
= V1_COMPLETE
```

native ShaderMap/VF, 전체 MRT, distortion parity는 선택적인 `NATIVE_PARITY` 후속 연구다. 단,
`CARRIER_V1_COMMON_TRANSLUCENT`는 손튜닝 시작점을 만드는 중간 상태일 뿐 V1 완료 조건의 generic
shader fallback은 아니다. 필수 RT0 family 식이나 carrier를 표현할 수 없으면
`V1_COMPLETE`로 승격하지 않고 ledger의 `BLOCKED_REQUIRED`를 유지한다.

첫 구조 검증은 기존 9-row hardcoded Tool canary를 Product로 켜는 작업이 아니다. exact child
`fx_m_mi_m_00.fx_mi.fx_m_me_watertrail_01_46_tr`, parent
`fx_m_mi_03.fx_m.fx_m_me_watertrail_01_tr`, mesh `fx_sm_01.fm_m_sphere_006`을 공유하는 다음 두
occurrence를 별도의 저장 가능한 audition Effect로 만든다.

| audition | source occurrence | 목적 |
|---|---|---|
| 3연격 | `Atk_01_02` sourceOrder 15 | 첫 발탄 source family 손튜닝 canary |
| 4연격 | `effect.valtan.four-slash.active.clip-02`의 `source.7e08a4a792dbc4be1e1f` | 같은 family executor의 스킬 비의존 재사용 증명 |

3연격 문서는 catalog와 실제 ordered clip occurrence cue를 같은 transaction에서 추가해 All
Effects Product tree에 연결한다. source time `3.214798927s`는 정수 반올림으로 뭉개지 않고
`sourceStartMs=3214`와 element local delay `0.000798927s`로 분리한다. 동시에 잘못된 60-row
aggregate `front-back-front.windup`은 catalog와 cue에서 제거하고 증거 문서와 60-row ledger로만
보존한다.

4연격에는 같은 source WATERTRAIL element가 기존 Product cue로 이미 한 번 실행된다. 별도 cue를
추가하면 같은 occurrence가 두 번 그려지므로 재사용 문서는 authoring-only witness로 둔다. 4연격
Product 승격은 기존 element를 receipt-aware in-place 교체하는 별도 원자 작업에서만 허용한다.
자동 검증이 실패하면 3연격 문서, catalog 교체, cue 교체, 두 audition 문서와 ledger를 전부
rollback한다. 사용자 서면 승인 전 상태는 family/carrier first-pixel canary이지 `V1_COMPLETE`가
아니다.
두 occurrence는 동일한 `effect.ue3.watertrail-01.v1` profile, 동일 RT0 HLSL 분기, 동일 renderer
코드를 사용해야 하며, Four Slash 전용 shader/profile/opcode/C++ effect-ID 분기는 0개여야 한다.

`front-back-front.windup`의 60 row는 source 감사용 legacy aggregate다. 이 문서를 Product
catalog/cue에서 실제로 분리하고, 60/60 row마다 아래 중 하나를 기록하는 별도 selection ledger를
둔다.

```text
V1_REQUIRED_VISIBLE       현재 V1 audition/Product에 들어가며 반드시 가시 기여해야 함
V1_OUT_OF_SCOPE_NON_RT0   distortion/light/post처럼 현재 V1 완료선 밖임
USER_RETIRED              사용자가 Solo/mute 비교 후 제외를 승인함
BLOCKED_REQUIRED          필수 후보이나 family/carrier가 아직 닫히지 않음
```

unknown, failed-but-removed, silent drop은 허용하지 않는다. Product에 들어간 N개는 N개 모두
attempted/submitted/committed draw와 사용자 Solo 기여가 있어야 하며, 동일 material/mesh라는 이유로
자동 dedup하지 않는다. 첫 WaterTrail 실험 성공은 확장 구조의 `GO` 증거일 뿐 3연격 전체 완료가
아니다.

## G-1. 전체 발탄 WModel 100배 회귀의 즉시 복구

하나의 path-aware render-contract 모듈을 정본으로 둔다.

```text
Effect/.../*.wmodel                    -> modelPreScale 0.01
Character/Valtan/ValtanWeapon.wmodel  -> modelPreScale 1.0
meshModel 없음                         -> modelPreScale 필드 없음
```

다음 경계를 같은 변경 단위로 닫는다.

- `build_valtan_source_occurrence_inventory.py::carrier_element_seed`가 seed를 반환하기 전에 계약을
  기록한다.
- 별도 root writer인 Whirlwind canary와 source-wave candidate가 복사된 legacy seed를 다시
  정규화한다.
- 기존 canonical 113문서는 transform, particle size, colour, timing, family packet을 건드리지 않고
  `modelPreScale`만 원자적으로 교정한다.
- `Publish-Effects.ps1 -Mode Validate/Publish`가 canonical 발탄 전체를 검사해 누락을 거부한다.
- unit test는 Effect WModel, 캐릭터 도끼 예외, meshModel 중복·미지원 경로, 바닥 world-aligned
  Sprite Particle의 `billboard=false` 보존, repository 전수 분모를 고정한다.

Sprite와 Local Decal은 이미 metre 크기이므로 함께 `0.01`을 곱하지 않는다. Mesh의
`transform.scale`도 손튜닝 값이므로 바꾸지 않는다. 도넛 안쪽 경계의 7→18 성장은 canonical
Local Decal의 `decal.size + linearLerp.endScale`이고 이 회귀 교정과 독립이다. 6방향 중심 원형
가이드는 현재 4.5×4.5, 충격은 6×6 고정 크기를 유지한다.

현재 Product에는 다음 두 층이 동시에 있다.

- Project Tuned aggregate `effect.valtan.front-back-front.active`: 3 particle + 3 decal의
  사람이 만든 근사 표현이다.
- Source wave 4개: 각 25 element, 총 100 element이며 `Atk_04_11/12/13`의 원본 carrier를
  네 시점 `1169/2253/3224/4220 ms`에 분리했다. 모든 material `sourceProfile.enabled`는
  아직 false라 drawable proof는 제출 성공만 증명하고 원본 material 식은 증명하지 않는다.

실제 바닥 충격과 검격 보조층인
`FX_MN_RPBF_00_N.Par_N_RPBF_Atk_01_02`는 네 시점
`1174/2225/3214/4206 ms`에 존재하지만 Product에 아직 연결되지 않았다. 이 system은
15 sprite, 3 mesh, 2 Local Decal, 총 20 carrier다. 현재 전체가
`UNRESOLVED_RUNTIME_ADAPTER`인 직접 원인은 Artist F용 portable carrier validator가
Lifetime과 Spawn module을 항상 요구하기 때문이다. 원본 Cascade emitter는 이 module을
생략하거나 외부 package의 module을 재사용할 수 있어, missing family 문제 이전에 source
module dependency closure가 먼저 필요하다.

## G00. 기준선, 변형 inventory와 실패 경계

### 원본 carrier matrix

| 순서 | carrier | exact child material | mesh | 현재 첫 blocker | 복원 경로 |
|---:|---|---|---|---|---|
| 0 | Sprite | `fx_d_pa_spla_05_01_tr` | - | portable Lifetime 누락 | dependency closure 뒤 family 판정 |
| 1 | Sprite | `fx_b_pa_distort_01_1_ad` | - | portable Lifetime 누락 | distortion family 판정 |
| 2 | Sprite | `fx_a_pa_cd_01_2_tr` | - | Spawn/Lifetime closure | dependency closure |
| 3 | Sprite | `fx_b_pa_smoke_03_tr` | - | runtime base lane 없음 | named texture role 복구 |
| 4 | Mesh Particle | `fx_k_me_makeflow_03_27_tr` | `fm_d_plane_003` | Spawn/Lifetime closure | 기존 MakeFlow 식 재사용 후보 |
| 5 | Sprite | `fx_k_pa_worldoffset_02_01_tr` | - | Spawn closure | WorldOffset variant |
| 6 | Sprite | `fx_a_pa_firework_01_ad` | - | runtime base lane 없음 | procedural/no-texture 판정 |
| 7 | Mesh Particle | `fx_n_me_dissolve_04_011_ma` | `fm_a_stone_001` | Masked executor 없음 | 새 MaskedDissolve variant |
| 8 | Sprite | `fx_e_pa_ht_18_1_tr` | - | Spawn closure | 기존 Shine 식 재사용 후보 |
| 9 | Sprite | `fx_k_pa_turbpa_01_tr` | - | portable Lifetime 누락 | source profile 판정 |
| 10 | Sprite | `fx_m_pa_spritewave_01_7_tr` | - | Spawn/Lifetime closure | 기존 SpriteWave 식 exact 재사용 |
| 11 | Sprite | `fx_a_pa_cd_01_3_tr` | - | Spawn closure | dependency closure |
| 12 | Sprite | `fx_a_pa_db_01_1_ad` | - | Spawn/Lifetime closure | 기존 Simple01 식 재사용 후보 |
| 13 | Local Decal | `fx_d_de_unlit_01_02_tr` | projector | parent/typed decal 식 미확정 | fail-close 유지 |
| 14 | Local Decal | `fx_n_de_ground_04_30_tr` | projector | Product decal 식 없음 | source-complete GroundDecal variant |
| 15 | Mesh Particle | `fx_m_me_watertrail_01_46_tr` | `fm_m_sphere_006` | Spawn/Lifetime closure | 기존 WaterTrail 식 exact 재사용 |
| 16 | Sprite | `fx_e_pa_fd_04_2_tr` | - | runtime base lane 없음 | named texture role 복구 |
| 17 | Sprite | `fx_a_pa_gl_01_3_ad` | - | Spawn/Lifetime closure | Simple additive parent 판정 |
| 18 | Sprite | `fx_e_pa_dist_04_1_ad` | - | Spawn closure | distortion variant 판정 |
| 19 | Sprite | `fx_a_pa_firework_01_ad` | - | runtime base lane 없음 | 6번과 같은 variant 판정 |

### 변경하지 않는 계약

- `ValtanEncounter.json`의 hit count, damage timing, action ID를 바꾸지 않는다.
- `Valtan.patternbindings.json`의 한 clip occurrence를 여러 animation clip으로 쪼개지 않는다.
- current Project Tuned aggregate를 source family admission과 별개로 먼저 삭제하지 않는다.
- raw DXBC가 있다는 이유만으로 sampler, VF, pass, constant ABI가 닫히지 않은 material을
  Product에 허용하지 않는다.
- Light와 generic Dust는 이번 slice에서 계속 제외한다.
- 다른 세션이 수정하는 캐릭터 authored document와 untracked 파일을 읽기 witness 이상으로
  사용하지 않고 stage/commit하지 않는다.
- Client와 UI는 에이전트가 실행·조작하지 않는다. 사용자가 최종 화면을 판정한다.

## G01. Cascade 외부 module dependency closure

### 수정 후보

```text
Tools/EffectPipeline/build_valtan_source_occurrence_inventory.py
Tools/EffectPipeline/materialize_artist_31470_portable_particle_carriers.py
Tools/EffectPipeline/test_build_valtan_source_occurrence_inventory.py
Data/Effects/Imported/Valtan/Valtan.source-occurrence-inventory.v1.json
```

Artist 31470 전용 strict validator를 느슨하게 만들어 누락을 숨기지 않는다. 먼저 20 carrier의
selected LOD가 참조하는 외부 module package를 exact node ID로 수집한다. package graph가
없으면 `MISSING_SOURCE_MODULE_PACKAGE`로 남기고 합성 기본값을 쓰지 않는다. graph가 있으면
Required/Lifetime/Spawn/TypeDataMesh cardinality를 원본 순서대로 다시 계산한다.

UE3에서 실제로 module 생략이 허용되고 class default가 source property로 증명된 경우에만
`SOURCE_CLASS_DEFAULT` provenance를 가진 canonical Lifetime/Spawn packet을 만들 수 있다.
Detail 값만 보고 임의 module을 합성하지 않는다. 이 단계의 종료 조건은 20개를 억지로
drawable로 만드는 것이 아니라 각 carrier가 `READY`, `MISSING_EXTERNAL_GRAPH`,
`MISSING_RUNTIME_RESOURCE`, `UNSUPPORTED_DECAL` 중 정확히 하나로 나뉘는 것이다.

## G02. 공용 source material family의 소비와 발탄 admission

### 이 세션의 수정 후보

```text
Tools/EffectPipeline/build_valtan_front_back_front_family_inventory.py
Tools/EffectPipeline/extract_valtan_front_back_front_exact_material_variants.py
Tools/EffectPipeline/test_extract_valtan_front_back_front_exact_material_variants.py
Tools/EffectPipeline/project_valtan_front_back_front_exact_families.py
Tools/EffectPipeline/test_project_valtan_front_back_front_exact_families.py
Data/Effects/Imported/Valtan/FrontBackFrontFamilyRestoration/
  Valtan.front-back-front-source-exact-family-targets.v1.json
  Valtan.front-back-front-source-exact-family-receipt.v1.json
```

Glasshole 세션의 전용 식과 Product gate는 건드리지 않는다. 이 세션은 세 family만 동일한
translated-HLSL 원칙으로 결합하되, 실제 Product 경로 대신 3연격 exact Authored 문서에서
명시적으로 켜는 Tool gate를 사용한다. generic fallback을 임시 연결하지 않고 canary draw가
실패하면 해당 occurrence를 fail-close한다.

`Valtan.source-material-evidence.json`의 child/parent/default/override를 이름 기반으로 병합하고,
carrier별 static set, texture sampler, dynamic parameter, render state를 variant key로 봉인한다.
다음 기존 strict family는 새 shader를 만들지 않고 exact contract가 일치할 때만 재사용한다.

```text
SpriteWave  -> fx_m.fx_m_pa_spritewave_01_tr
MakeFlow03  -> fx_m.fx_k_me_makeflow_03_tr
WaterTrail  -> fx_m.fx_m_me_watertrail_01_tr
Shine       -> fx_m.fx_f_pa_shine_01_0_tr
Simple01    -> fx_mm.fx_mm_simple_01_ad
```

캐릭터에서 같은 child를 썼다는 사실만으로 profile 전체를 복사하지 않는다. Valtan exact child의
texture/scalar/vector와 sampler를 source evidence에서 다시 패킹하고, strict evaluator가 요구하는
parent identity와 named lane contract가 일치할 때만 `sourceProfile.enabled=true` 또는 단일
`RuntimeMaterialV2` owner로 승격한다.

Valtan Crack, MaskedDissolve, GroundDecal만 이번 runtime slice의 명시적 복원 대상이다.
raw DXBC와 번역 HLSL을 함께 oracle로 사용하고 Tool에는 검증된 exact occurrence packet만
들어간다. 향후 Product admission도 같은 증거를 요구한다.
`Translucent/Additive/Masked` suffix 추측 fallback은 금지한다.

## G03. Local Decal typed family의 Valtan canary 소비

Local Decal projector geometry와 material equation을 분리한다. `TypeDataDecal`은 quad particle로
바꾸지 않고 기존 `Render_Decal` projector 경로를 사용한다.

- `fx_n_de_ground_04_30_tr`는 exact parent
  `fx_m_mi_04.fx_m.fx_d_de_ground_04_tr`, 6 named texture와 source scalar/vector가 모두
  존재하므로 첫 Valtan canary다. selected PS는 실제 decal VF/pass에서 가져오며 local-mesh
  structural candidate로 대체하지 않는다.
- `fx_d_de_unlit_01_02_tr`는 parent/equation closure가 끝날 때까지 fail-close한다.
- Artist F occurrence hash에 묶인 6-SRV Local Decal adapter를 발탄에 재사용하지 않는다.
- projector transform, radius, lifetime, dissolve가 material packet과 별도 검증되어야 한다.

공용 renderer ABI는 하나만 유지한다. 기반 merge 전에는 증거와 translated HLSL을 닫고, 기반
merge 후 같은 renderer의 typed GroundDecal carrier를 확장한다. wrong parent, local-mesh PS
오선택, missing texture role, duplicate lane, unsupported TypeDataDecal module, staging 중간 실패
rollback을 Valtan fixture로 고정한다.

## G04. 3연격 Product cue의 원자적 교체

가족별 admission이 끝난 뒤 한 transaction에서만 다음을 수행한다.

1. `Atk_04` 네 source wave cue `1169/2253/3224/4220 ms`를 유지한다.
2. `Atk_01_02` 네 impact/decal cue `1174/2225/3214/4206 ms`를 같은 clip occurrence에 추가한다.
3. 첫 세 쌍은 왼쪽/오른쪽/위에서 아래 hit presentation, 네 번째 쌍은 auxiliary source visual로
   명시한다.
4. 원본 source element와 겹치는 Project Tuned aggregate particle/decal만 같은 transaction에서
   catalog/cue에서 제거한다.
5. source family가 하나라도 required canary를 통과하지 못하면 aggregate와 기존 cue를 그대로
   보존하고 전체를 rollback한다.

All Effects는 clip 하나 아래 ordered cue occurrence 8개를 시간순으로 보여 준다. 사용자는 각
cue와 전체 clip을 따로 재생·저장할 수 있고, 저장은 동일한 Product document에 반영된다.

## G05. 캐릭터 family 연구 결과를 쓰는 방식

창술사 V/ALT_V/T의 dragon/helix, 차원술사 W/F glass, 워로드 Full Barrel/F, 도화가 F/Mir는
발탄 데이터로 복사하는 대상이 아니라 공용 evaluator의 witness다. 각 세션에서 얻은 결과는
다음 조건을 만족할 때만 공용 코드에 합친다.

- 동일한 parent equation과 static set이다.
- carrier/VF와 pass가 같다.
- named sampler/texture/scalar/vector ABI가 같다.
- golden fixture 또는 WARP oracle로 식이 검증됐다.
- 기존 캐릭터 occurrence hash나 skill ID를 evaluator가 요구하지 않는다.

Glasshole/Crackhole/FluidNinja/CustomParticle/SpriteWave의 raw DXBC 확보는 복원 불가능을 뜻하지
않는다. 현재 차원술사 W의 병목은 DXBC 추출이 아니라 scalar lane, sampler/address/color space,
실제 VF/pass와 Product admission의 결합이다. 도화가 F는 일부 occurrence에서 이 결합을
typed HLSL 식으로 번역하고 golden fixture를 만든 점이 다르다.

이 협력은 같은 파일을 동시에 고치는 방식이 아니다. 캐릭터 세션은 공용 family ABI와 해당
캐릭터 witness를 소유하고, 이 세션은 Valtan source packet/cue/admission fixture를 소유한다.
공용 변경이 main에 merge되면 이 branch에서 pull/rebase한 뒤 exact ABI를 소비한다. 사용자가
금지한 다른 세션 메시지 전송은 하지 않으며, 미merge 작업을 추측해 복제하지 않는다.

## G06. 자동 검증과 사용자 수동 확인

다음 순서로 실제 통과 결과만 RESULT에 기록한다.

1. family inventory schema/identity/hash/rollback unit tests
2. source occurrence inventory check와 20-carrier denominator 검증
3. materializer dry-run, wrong-parent/missing-lane negative tests, idempotence
4. Effect publisher Validate와 Valtan focused harness
5. Engine Debug, UpdateLib Debug, ClientFrontendHarness Debug
6. action-facing Valtan fast regression
7. Client Debug isolated build/link
8. scoped `git diff --check`

자동 검증 뒤 사용자가 직접 다음을 확인한다.

1. All Effects에서 Valtan `FRONT_BACK_FRONT -> SMASHES -> mesh_att_battle_19_01`을 연다.
2. source wave와 impact/decal cue를 하나씩 재생해 giant quad나 화면 전체 덮임이 없는지 본다.
3. 전체 clip에서 왼쪽, 오른쪽, 위에서 아래의 방향과 네 번째 auxiliary visual timing을 확인한다.
4. 바닥 spike/stone과 Ground Decal이 바닥 projector에 붙고 dissolve하는지 확인한다.
5. 실제 Server+Client 패턴에서 animation과 effect가 같은 clip timeline을 공유하는지 확인한다.

사용자의 서면 관찰 전에는 visual fidelity를 PASS로 기록하지 않는다.

## G07. 완료 단위와 명칭

첫 결과의 정직한 명칭은 `FRONT_BACK_FRONT admitted family slice`다. 사전 검토에서 분리된
증거 부족 4개는 source closure 결과로 identity를 다시 확인하며 숫자를 코드에 임의 고정하지
않는다. 다음이 모두 성립할 때 admitted slice를 완료하고, 남은 carrier까지 같은 기준을
충족한 뒤에만 `원본 모든 carrier 완전 복원`이라고 기록한다.

- 20 `Atk_01_02` carrier와 100 `Atk_04` occurrence의 source denominator가 보존된다.
- supported family는 exact variant로 렌더되고 unsupported family는 generic plane 대신 fail-close한다.
- Local Decal은 projector + typed GroundDecal 식을 사용한다.
- Server 3-hit authority와 한 animation clip binding은 불변이다.
- aggregate retirement와 8 Product cue 연결이 원자적이고 rollback 가능하다.
- 자동 검증과 Debug build가 통과하고 수동 visual 상태는 별도로 기록된다.
