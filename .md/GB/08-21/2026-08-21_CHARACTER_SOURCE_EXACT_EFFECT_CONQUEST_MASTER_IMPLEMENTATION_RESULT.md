# 2026-08-21 Character Source-Exact Effect Conquest Master 구현 결과

기준 커밋: `812d23a4`
구현 브랜치: `codex/effect-family-conquest`
상태: 진행 중 living RESULT
최종 화면 판정자: 사용자

Fluid01 추가 checkpoint는 기준 `b705abb7`, 격리 브랜치 `codex/dm-fluid01`에서 검증했다.
차원 F ScreenOverlay Product checkpoint는 기준 `709ebfcb`, 격리 브랜치
`codex/dm-f-overlay-product`에서 검증했다.

이 문서는 마스터 PLAN의 실제 반영 상태만 기록한다. 도화가·워로드·창술사·차원술사
전체 복원은 아직 진행 중이며, 이 버전에서는 family inventory, action-facing, opt-in standard
color/coverage와 Lance dragon masked material 수직 슬라이스를 구현했다.

## 1. 자동 완료된 범위

### 1.1 family-first restoration inventory

- `CarrierVariant × MaterialVariant(nullable) × RenderVariant × CompositionVariant`를 제품
  execution closure로 고정했다.
- provenance, evidence, executor, runtime admission, user review를 다른 축으로 보존했다.
- 도화가 F 17행, 차원술사 A 4행, 워로드 W 1행 총 22 occurrence를 machine-readable
  contract로 봉인했다.
- grayscale/DXT1을 암묵적 `base.a`로 coverage에 연결하거나, 없는 emissive lane에
  intensity만 설정하거나, default SRV로 required lane을 대체하는 계약을 validator가 거부한다.

### 1.2 차원술사 A `2050210`

- 기존 사용자 튜닝 9행을 ordered canonical identity로 보존했다.
- 현재 `.25s` `a58f...` 행을 유지하고 source occurrence `.60/.90/1.30s` 세 행만
  selective append해 보이는 MakeFlow 검격을 정확히 네 번으로 만들었다.
- 원본 `.25s` base occurrence는 evidence-only로 남기고 현재 튜닝 행을 중복 spawn하지 않았다.
- outer cue는 `follow=follow`, 각 occurrence는 시작 시점 snapshot으로 구성했다.

### 1.3 공용 `ACTION_FACING`

- animevent format v6에 `orientation=anchor|action_facing`을 추가했다.
- position follow/snapshot과 orientation authority를 분리했다.
- `action_facing`은 Server snapshot의 스킬 edge yaw를 actionStartTick과 함께 한 번만 캡고,
  actor translation/positive scale을 유지하며 local transform을 정확히 한 번 합성한다.
- mirrored/non-finite action-facing anchor는 실패 즉시 기존 prepared state를 보존한다.
- 차원술사 A와 워로드 W `17060`이 이 계약을 소비한다.
- 현재 admission은 ACTIVE 스킬만 대상이다. HOLD action은 Server-latched historical facing
  field가 추가되기 전에 이 모드로 승격하지 않는다.

### 1.4 워로드 A `17090` authored subset save

- immutable Track-A 증거는 계속 Mesh 14/Sprite 2와 chain06 8/chain07 4를 정확히 검증한다.
- 일반 authored Load/Save에서는 exact count를 강제하지 않고, 남겨 둔 chain 행 각각의 stable ID,
  mesh, material profile, source recipe, 회전과 DynamicParameter identity를 검증한다.
- 따라서 Effect Tool에서 합법적인 네 행 subset을 저장하면 atomic temporary reload가 더 이상
  원본 12행 cardinality를 이유로 거부하지 않는다.
- 중복 ID, source allowlist 밖 ID, 알 수 없는 mesh, 변조된 burst recipe는 저장을 거부하고 기존
  디스크 문서를 그대로 유지한다.

### 1.5 도화가 A/D/R 저위험 복원

- A `31460`은 정확히 여덟 검격 행의 `uv_noise_velue`만 `0`으로 override했다. noise DDS를
  삭제하거나 source reset으로 되돌리지 않으며 base/dissolve/source recipe는 보존한다.
- D `31490`은 기존 56행 authored 문서를 바꾸지 않고 `sdm_sk_cloudtiger` animevent effectref와
  direct-v13 source catalog join을 추가했다. sealed runtime 생성은 full publisher 단계에 남아 있다.
- R `31210` BA1에서 symbol14를 particle base/noise로 잘못 덮었던 두 override를 compiler 값으로
  복원했다. BA4에는 `fx_o_symbol_14.dds`를 쓰는 독립 LocalDecal 한 행을 추가해 중심 기준
  `-720deg/s` 회전, 0.8초 alpha fade, 비특이 `1 -> 0.01` scale 소멸을 구성했다.

### 1.6 워로드 T `17240`

- BA1의 원인 행 `authored.source-screenpost.dd9606e656d90f42cbf5ed32`만 제거하고 다른 24행은
  deep-equal로 보존했다.
- BA3의 현재 사용자 튜닝 8행은 그대로 두고 source emitter `55/56/58/57`의 decal 네 행만
  receipt 순서로 selective append했다.
- 네 행은 `fx_k_turtlediff_02` base와 원본 noise/mask/emissive/dissolve lane, 0.12초 start,
  2초 life, 4.5 크기, source recipe identity를 보존한다.

### 1.7 창술사 E `34560` cone donor

- W `34550`에서 사용자가 기준으로 지정한 `fm_d_cone_005` occurrence
  `authored.source-particle.622ad9be48bb849d36fa5b81`를 exact donor로 봉인했다.
- E clip3의 기존 네 행은 deep-equal로 유지하고, impact 시점 0.3초에 donor carrier/material/source
  recipe 한 행만 `DONOR_TRANSPLANT`로 append했다.
- donor의 `97.25deg` Y rotation과 local scale은 첫 화면 기준값이며, 사용자가 Effect Tool에서
  해당 독립 occurrence만 후속 튜닝할 수 있다.

### 1.8 Glasshole02 첫 class-neutral family canary

- 차원 W `2050120.clip3`의 exact K-child SpriteParticle
  `authored.source-particle.40e1b48e2f0f88dcfeff1549` 한 건만 RuntimeMaterialV2 opcode 16으로
  승격했다.
- aura `RGBA/sRGB`, crack normal `RG/linear`, inner-hole `RGB/sRGB` 세 lane과 각각의
  linear/wrap sampler, 32 scalar, 2 vector, dynamic/particle/static/render mask를 exact execution
  tuple로 봉인했다.
- 기존 bounded Glasshole02 profile-29 수식을 class-neutral HLSL family로 옮겼다. exact cooked
  DXBC는 실제 SpriteParticle VF와 six-slot MRT/sampler ABI가 닫히지 않았으므로 oracle-only다.
- A `2050210`과 D `2050240.clip2`의 J-child는 같은 parent 이름만으로 승격하지 않았다. 두
  occurrence는 effective static set을 별도로 회수할 때까지 기존 reconstructed profile을 유지한다.
- opcode 16 packet의 carrier/material/resource/channel/color-space/sampler/scalar/mask가 하나라도
  다르면 staging에서 occurrence를 거부하고 직전 prepared document를 보존한다.

### 1.9 도화가 S `31420` 풀끝 소멸

- 기존 source-owned particle 한 행은 canonical identity까지 그대로 보존했다.
- `PROJECT_TUNED` Sprite 두 행만 독립 append했다. body는 `fx_o_grass_04`를 coverage(mask),
  `fx_o_grass_03`을 dissolve로 사용하고 tip은 두 역할을 교차해 DXT1의 고정 alpha를 색 alpha로
  오해하지 않는다.
- tip은 기존 S의 `fx_d_fluid_007` emissive texture와 HDR intensity `6 -> 0`을 사용한다.
  실제 PointLight는 추가하지 않았고, body/tip의 alpha·emissive·dissolve가 절대 시각
  `1.5318s`에 함께 종료된다.

### 1.10 opt-in `effect.standard_color_v1`

- 기존 generic effect 의미를 바꾸지 않고 `standardColorV1` backend를 명시적으로 선택한
  sprite-backed PARTICLE, DECAL, TRAIL만 새 계약에 admission한다.
- packet version/opcode, base-radiance/coverage/dissolve lane, source channel, linear/sRGB,
  sampler, carrier-alpha lifetime, base-radiance emissive, missing-lane fail-close를 typed packet으로
  봉인했다.
- GPU ABI는 `Header={packetVersion=1, opcode=1, laneCount, textureMask}`,
  `BaseCoverage={baseLane, baseChannel, coverageLane, coverageChannel}`,
  `Dissolve={mode, lane, channel, missingLanePolicy}`,
  `Policies={emissiveMode, lifetimeEnvelope, requiredMask, boundMask}`,
  `Scalars.x=dissolveSoftness`다. 나머지 scalar 성분과 generic packed input은 0이어야 한다.
- required lane은 실제 texture/SRV format과 sampler readback까지 일치해야 commit된다. 따라서
  BC1/DXT1의 존재하지 않는 alpha를 coverage로 선언하거나 default SRV로 대체하는 경로는
  기존 prepared stage를 보존한 채 해당 stage만 실패한다.
- 전용 `Shader_EffectStandardColorV1.hlsli`가 세 carrier에서 같은 식을 소비한다. legacy
  `Shader_EffectCommon.hlsli`와 generic backend는 수정하지 않았다.
- 아직 Product occurrence를 이 family로 migration하지 않았다. class별 source-exact material은
  사용자 화면 판정과 occurrence admission을 거쳐 별도 변경으로 연결한다.

### 1.11 도화가 D `31490` BLACK_TIGER_STROKE typed 수직 슬라이스

- role receipt의 sourceOrder `45/46/47/48/51/52/81/82/84/110/111/113`에 해당하는
  SpriteParticle 정확히 12행만 기존 56행 사이에 삽입했다. 기존 56행은 filter 뒤 deep-equal이며
  각 JSON object의 key order, 숫자 표기와 들여쓰기 byte도 보존한다.
- source child5 네 행과 child6 여덟 행을 각각 28/24 scalar packet으로 분리했다. 두 variant는
  source parent `fx_m_pa_spritewave_01_ad`, exact base/noise/dissolve texture와 DynamicParameter
  module identity를 유지한다.
- class-neutral `RuntimeMaterialV2` opcode 18 consumer를 추가했다. main RGB는 linear radiance,
  main R은 DXT1 silhouette coverage, noise RG는 UV warp, dissolve R은 gate, ParticleColor alpha는
  독립 lifetime envelope로 소비하고 additive pass 2에서 그린다.
- child5의 source-owned `ParticleColor.rgb=-0.5`는 기존 SpriteWave 계열과 같은 signed magnitude
  정책(`abs(rgb)`, exact zero neutral)으로 소비해 검은 카드 collapse를 막는다.
- opcode 18은 위 12 stable ID와 두 exact packet에서만 admission된다. allowlist 밖 opcode,
  channel/material/scalar/sampler/render packet drift는 stage 전에 fail-close한다.
- fidelity는 `TYPED_SOURCE_RECONSTRUCTION/PARTIAL/PENDING`이다. native DXBC 실행 또는 원본
  shader source-exact 복원으로 기록하지 않았고 사용자 first-pixel 판정도 아직 대기다.

### 1.12 typed textured screen-overlay v1 capability

- 기존 RGBNoise, ZoomBlur, FilmNoise의 profile/pass 번호와 수식을 바꾸지 않고 별도
  `ScreenOverlays` presentation channel을 추가했다. Manager는 frame마다 overlay expectation,
  accepted/suppressed/failed 통계를 독립적으로 계산하며 provider 하나가 잘못된 SRV/typed descriptor를
  제출하면 전체 overlay frame을 rollback하고 다음 frame에서 정상 복구한다.
- Renderer는 기존 ScreenPost가 끝난 scene ping-pong 뒤에 `PresentationTexturedOverlay` pass 14를
  stable `sourceOrder` 순서로 실행한다. 결과 target parity는 ScreenPost 수와 overlay 수를 함께 계산하고,
  overlay가 0행이면 기존 final target과 완전히 같다.
- Client `CEffectScreenOverlayPresentation`은 `lostark.effect-screen-overlay` format v1 JSON을
  parse → validate → DDS stage → atomic generation commit 순으로 처리한다. asset ID는 Resources-relative
  `Effect/.../*.dds`만 허용하고 `provenance=PROJECT_TUNED`를 필수로 요구한다. linear/sRGB view,
  R/G/B/A coverage availability, point/linear,
  clamp/wrap, start/lifetime alpha, position/scale/rotation/angular velocity/UV drift/tint를 typed field로
  봉인한다.
- invalid version, missing asset, 알 수 없는 coverage/sampler, SRV color-space/channel 불일치는 기존
  committed presentation과 재생 clock을 유지한다. `Cancel`, `Stop`, lifetime expiry와 overlay enable
  suppression은 이미 queue된 provider가 있어도 다음 submission에서 0 draw로 정리된다.
- capability 자체는 synthetic DDS canary로 먼저 닫은 `PROJECT_TUNED/TYPED_PRESENTATION`이다.
  차원 F 첫 Product consumer는 1.14에서 연결했다. source-exact native DXBC, refraction,
  normal/noise/mask/dissolve 다중 lane과 W rollout은 계속 후속 revision 경계다.
### 1.13 차원술사 F `2050230` Fluid01 SpriteParticle family

- F의 exact Product SpriteParticle
  `authored.source-particle.1ae3416ac205fee634b746a9`와
  `authored.source-particle.ed33fb10661afb8854e76957` 두 행만 RuntimeMaterialV2 opcode 17로
  승격했다. Glasshole02는 opcode 16, 도화가 D BLACK_TIGER_STROKE는 opcode 18을 독립적으로 유지한다.
- 두 행의 child는 `fx_m_mi_w_00.mi.fx_w_pa_fd_01_3_tr`, parent는
  `fx_mastermaterial.fx_mm.fx_mm_fluid_01_tr`이다. transition `fx_d_cloud_035` RGB/linear,
  emissive `fx_o_glass_01` RGB/linear, noise01 `fx_bg_softriver_02_n` RG/linear, noise02
  `fx_bg_softriver_01_n` RG/linear 네 lane과 linear/wrap sampler를 exact tuple로 봉인했다.
- source evidence의 scalar 22개, vector 0개, texture mask `0x0f`, dynamic consumed/suppressed
  `0x07/0x08`, particle color consumed `0x0f`, pass 3
  `RS_Default/DSS_ReadOnly/BS_EffectAlpha`를 packet으로 보존했다.
- bounded profile-34 식을 class-neutral HLSL Fluid01 family로 옮겨 transition/noise/distortion,
  emissive, particle color와 lifetime coverage를 평가한다. 이 식은 현재 source evidence 기반의
  bounded 복원이며 native cooked DXBC 복원으로 주장하지 않는다.
- C++ admission은 두 stable ID와 두 sourceNode, sprite carrier, child/resource bindings, 네 lane의
  role/asset/register/channel/color-space/sampler, 22 scalar와 모든 mask가 모두 일치할 때만 opcode 17을
  승인한다. 공용 resource reuse signature에도 sourceNode, render profile, SourceRecipe enabled/renderer
  shape를 포함해 opcode 16/17 strict admission의 no-rebuild 우회를 막았다. Fluid01 두 번째 행의
  sourceNode를 변조한 mid-stage restage와 Glasshole02 carrier shape 변조는 모두 거부하고 직전 prepared
  draw를 유지한다. opcode 18 도화가 D 실행은 별도 exact allowlist와 WARP canary가 소유한다.
- selective materializer는 두 target material만 교체한다. 같은 F 문서의 나머지 행은 canonical
  deep-equal이며 도화가 F authored 문서는 byte/canonical golden으로 고정했다.
- 이번 단위는 world Fluid01 SpriteParticle까지만 닫았다. 화면을 가로지르는 textured glass shard
  overlay, F raw source의 sphere/hemisphere/world shard/light/post 재조합과 Fluid01 mesh rollout은
  구현하지 않았다.

### 1.14 CircleSurface/Vortex reconstructed source atom

- reconstructed source execution의 strict allowlist를 32 class에서 34 class로 확장해
  `particlemodulelocationcirclesurface`와 `particlemodulevortex`를 admission했다. 실제 source의
  `efparticlemodule...` 이름은 기존 `SourceClass_Matches`가 `ef` prefix와 `_seeded` suffix를
  정규화하므로 Product/character별 분기 없이 같은 실행 atom으로 join한다.
- CircleSurface는 실행 코드가 소비하는 `startlocation` 3-vector, `startradius/startrot/velocityscale`
  scalar distribution 네 개를 정확히 요구하고, `splitcirclecount` integer `[0,4096]`,
  `surfaceaxis`, `bhalfmode`, `bnegativeaxis`, `velocity`, `benabled` literal type을 검증한다.
  Vortex는 정확히 하나의 scalar `poweracceleration` distribution과 optional numeric `power`, boolean
  `benabled`를 검증한다. 추가/누락 distribution, 잘못된 width, fractional split과 unknown class는
  reconstructed plan commit 전에 fail-close한다.
- 실행 증명은 7개 particle을 260cm 반경의 7-sector 원에 배치하고 radial velocity를 적용한 뒤,
  30/60/120FPS 입력이 같은 fixed-step Vortex position/velocity에 도달하는지 비교한다. unknown,
  fractional CircleSurface, wrong-width/missing Vortex 문서는 기존 playback에 재-stage해 모두 거부하고
  직전 evaluated frame을 보존했다.
- 차원 F `authored.source-particle.0f7aa9cd0601769f9e51f5cc`의 exact 7-burst,
  CircleSurface `FX_PC_SWP_03:export:58@ref:11`, Vortex `FX_PC_SWP_03:export:184@ref:12`와 도화가 V의
  Vortex stable row 세 개를 resource-independent raw JSON canary로 봉인했다. 격리 worktree에
  Git 비관리 DDS가 없어도 source identity/consumed field drift를 검출한다.
- 이 단위는 Product authored 문서와 runtime catalog를 수정하지 않았다. 위 Product particle 행은
  이미 ordinary portable source carrier로 spawn/update를 실행하므로 이번 변경을 신규 Product
  admission이나 도화가 V attractor 완성으로 승격하지 않는다.
### 1.15 창술사 `34630/34650` dragon masked class-neutral family

- `34630` clip1~4와 `34650` clip1의 기존 Product 문서에서 body 6행/head 6행, 정확히 12행만
  selective materialize했다. 비대상 top-level element JSON byte와 대상의 material/resources 외 필드는
  보존하며 동일 입력 재실행은 byte-idempotent다.
- source parent `fx_m_mi_00.fx_m.fx_d_me_master_01_ph_msk`, body/head child, 12 stable ID,
  sourceNode, body/head WModel, ParameterDynamic stable ID와 render packet을 exact allowlist로 봉인했다.
  allowlist 밖 opcode 19와 sourceNode/rendererShape/renderProfile/resource/lane/scalar drift는 fail-close한다.
- RuntimeMaterialV2 opcode 19는 normal RG, alpha R, emission RGB, diffuse RGB, specular RGB 다섯
  `linear/wrap` lane을 독립 소비한다. normal/emission/diffuse/specular UV는 고정이고 alpha UV만
  `10x10`, `panY=-0.125/s`다.
- DynamicParameter W는 dissolve threshold, ParticleColor alpha는 별도 lifetime envelope다.
  normalized life만 바꿔 dissolve를 재적용하지 않는다. focused WARP readback에서 body 386픽셀,
  head 312픽셀, alpha-pan signature 변화, 32개 공통 stationary radiance 픽셀, Dynamic-W 변화,
  ParticleColor alpha 감쇠와 normalized-life 불변을 확인했다.
- disabled native source profile의 parent/profile 증거를 codec이 버리던 공백을 닫았다. minimal
  `{enabled:false}`는 기존처럼 유지하고, `profileId`가 있는 disabled evidence는 parse/serialize에서
  보존해 Tool roundtrip 뒤에도 parent allowlist가 유지된다.
- 복원된 native pixel DXBC는 deferred five-MRT이고 source mesh VF/native binding array가 닫히지
  않았다. 따라서 이 구현은 `TYPED_HLSL_SEMANTIC_REPLAY/PARTIAL/AUTHORING_ONLY/PENDING`이며,
  one-sided alpha explicit clip/depth-read SceneColor replay를 source masked depth-write/GBuffer와
  동일하다고 주장하지 않는다.
- 팀 관리 runtime 입력 중 body spec은
  `Effect/LanceMaster/Textures/fx_d_atypical_010.dds` SHA-256 `93fa9c93...`, head spec은
  `Effect/LanceMaster/Textures/sk_flm_gdr_02_s.dds` SHA-256 `a626399c...`다. DDS는 커밋하지 않는다.

### 1.16 도화가 V reviewed wisp cohort와 TargetAttractor

- 도화가 V `31910`의 현재 손튜닝 41행 canonical hash를 보존하고, sealed donor와 Track-A receipt,
  `PALE_YELLOW_RED_ORBIT` role allowlist가 모두 일치하는 source particle 다섯 행만 앞에 삽입했다.
  결과는 46행이며 기존 41행은 canonical deep-equal이다. historical donor의 추가 decal은 복원하지 않았다.
- source occurrence/recipe/resource는 `SOURCE_EVIDENCE_DONOR_TRANSPLANT`, 중앙 집결 motion은
  `PROJECT_TUNED`로 분리했다. emitter17 `b637bf78...` 한 행만 14 source particle을 대상으로
  root-local centre, normalized active interval, signed tangential/radial acceleration, maximum speed,
  convergence radius와 arrival damping을 사용한다. runtime catalog publish와 사용자 승인은 아직 대기다.
- codec은 nested `targetAttractor`를 optional/default-omitted로 parse/serialize하고 particle-only,
  finite range, ±1000m target offset, ordered interval, known target-space를 검증한다. 실패는 기존
  parsed/staged state를 보존한다.
  Effect Tool에는 source module 편집과 구분되는 PROJECT_TUNED attractor controls를 추가했다.
- playback은 source module velocity와 매 step의 transient VelocityOverLife scale을 그대로 둔 채 별도
  world-space steering velocity를 합성한다. acceleration/speed/radius와 world +Y tangent는 Element pitch와
  비균일 scale에 종속되지 않으며, `ELEMENT_LOCAL`은 non-local particle도 현재 ElementWorld를 추종한다.
  마지막 radial step에서는 tangent를 제거해 capture radius 바깥의 영구 공전을 막는다.
- materializer는 receipt와 모든 evidence를 Product write 전에 검증하고, invalid receipt는 destination bytes를
  보존한다. fresh 41→46 삽입은 기존 41행 raw array tail과 indentation까지 보존한다.
- synthetic 하네스는 30/60/120FPS bit-stable frame, tangential curve와 centre capture, transient source
  velocity scale 불변, root/element target-space, pitch·비균일 scale, moving Element target, oversized offset
  rollback과 Reset clear를 검증한다. 실제 Product 행도 Resource root를 read-only로 지정해 source burst
  14개 전부가 0.25초 시점에 root 5cm 안으로 capture되는 것을 확인했다.

### 1.17 차원술사 F `2050230` 첫 Product ScreenOverlay consumer

- actual animevent `pc_sp_m_00_sk_sk_chronorecoil`의 0ms effectref
  `effect.dimensionmaster.skill.2050230.unified`와 direct EffectCatalog row를 그대로 따라 첫 Product
  screen-overlay presentation을 선택적으로 연결했다. class/skill switch나 별도 overlay manager는
  만들지 않아 이후 W도 같은 effect asset binding 계약을 재사용할 수 있다.
- 기존 Product effect 문서의 8 Elements는 수정하지 않았다. 원문 byte SHA
  `afab680bd36b4efcc4baf654c4848a1f3571a29cbc338b0ed58fec940de60e09`, Elements canonical SHA
  `a3e3b1348614c57e78fcf4d8b3feff89d43559fa985ad5f877673c94839747ee`를 focused harness와 receipt/결과 문서로
  봉인했다.
- raw 69행에는 `fx_d_fragment_005.dds`를 쓰는 world SpriteParticle이 0.7초에 7개 burst되고,
  RGBNoise와 ZoomBlur도 0.7초에 시작한다. screen-space textured shard occurrence는 source에서
  증명되지 않았으므로 texture 선택 근거만 world source로 보존하고, 5개 screen placement·회전·alpha·
  sRGB 해석은 전부 `PROJECT_TUNED/PARTIAL`로 명시했다. DDS는 8320-byte DXT1/BC1,
  SHA-256 `193a597baf328508763b0e6712dc702d604fd1e3b22a311494c26f45470f992c`이며 coverage는 존재하는
  `R` channel만 사용한다.
- publisher는 presentation 원문과 실제 DDS의 byte count/SHA를 direct runtime row 안의 typed binding으로
  봉인한다. catalog load는 schema, provenance, presentation ID, 원문 SHA, ordered field, resource closure를
  모두 stage한 뒤 전체 map을 한 번에 commit한다. Product prewarm도 Resources root의 DDS identity와
  typed GPU stage를 검증한 immutable template만 commit하므로 spawn-time disk I/O가 없다.
- 각 Product `CEffectObject`는 template의 playback instance를 복제하고 같은 lifecycle과 playback-rate delta를
  공유한다. overlay delta clock은 world document duration clamp와 분리되어, 짧은 world carrier가 먼저 끝나도
  overlay tail의 natural end 전에는 EffectObject가 완료되지 않는다. absolute Tool sample은 같은 시간으로 seek한다.
  기존 RGBNoise/ZoomBlur/FilmNoise submission은 수식·순서 변경 없이 먼저 실행되고 overlay는 그 뒤
  기존 Engine `ScreenOverlays` channel에 제출된다. natural end, cancel, death/visibility clear, level clear,
  reset은 같은 EffectObject lifecycle로 0 draw까지 닫힌다.
- malformed 다섯 번째 resource/stage는 직전 presentation generation, ID, clock과 5행을 보존한다.
  actual cue/catalog join, `.69/.70/.72/.78s` deterministic active count, sourceOrder `1000..1004`,
  post 2행 뒤 overlay append와 target parity, cancel/reset/natural clear를 Debug/Release focused harness가
  검증한다. 사용자 Client first-pixel과 source-exact 판정은 여전히 `PENDING`이다.

### 1.18 Effect Tool Presentation Light/ScreenPost family

- `EFFECT_AUTHORING_FAMILY`에 `PRESENTATION_LIGHT`와 `PRESENTATION_SCREEN_POST`를 서로 다른 family로
  추가했다. 기존 Mesh/Sprite/MeshParticle/SpriteParticle/LocalDecal/Trail을 포함한 여덟 family가
  element kind와 mesh carrier binding으로 정확히 resolve된다. Light/Post가 더 이상 tree에서 `END`로
  사라지지 않으므로 count, 선택, solo/isolation, visible, delete, Apply/Revert/atomic Save 흐름을 그대로
  사용한다.
- Light는 기존 typed payload의 range, intensity, HDR color, ambient, falloff를 편집한다. ScreenPost는
  RGBNoise/ZoomBlur/FilmNoise profile, intensity, secondary, frequency, HDR tint, nonzero seed를 편집한다.
  변경된 payload는 `RECONSTRUCTED_PROFILE` status를 유지하며 global Preview ScreenPost는 계속 비영속
  A/B gate다.
- Presentation carrier는 WModel/DDS/material lane이 없으므로 drawable element resource UI를 표시하지
  않고 resource binding을 거부한다. source evidence가 없는 Light/Post를 Tool에서 임의 생성하거나
  duplicate/preset seed로 복제하지 않는다. 이미 typed payload가 enabled인 row만 수치 편집할 수 있고,
  disabled/unresolved source row는 hide/delete만 허용한다. 신규 enabled payload는 후속 source materializer가
  parse→validate→stage→commit으로 만들어야 한다.
- focused harness는 synthetic authored Light+ScreenPost를 ordinary codec으로 저장·재로드한 뒤 typed field
  수정 persistence, invalid Post profile의 disk-preserving rejection, Post 삭제 후 Light 보존, family isolation을
  검증한다. Client/UI는 자율 실행하지 않았으므로 실제 Tool open/editor 화면과 사용자 조작 평가는
  `PENDING`이다.

### 1.19 도화가 E `31480` animated crane 제품 경로 canary

- `31480 -> sdm_sk_flyinheaven -> effect.artist.skill.31480.unified` 연결을 skill binding과 정확히 하나의
  root-follow animevent effectref로 봉인했다. source catalog뿐 아니라 실제
  `Client/Bin/DataFiles/Effect/EffectCatalog.runtime.json`의 direct-v13 row가 SHA-256
  `d9fc1196...` content-addressed sealed 문서를 선택하고, 이 문서가 authored 문서와 codec semantic
  identity가 같은지 검증한다.
- crane ModelCue는 `SK_SDM_RCC_00_SK_FX_01.wmodel`, clip `rcc_sk_flyinheaven`, 1초 start,
  1.11111116초 duration, translucent surface, 0.01 scale, +Z 9 velocity를 유지한다. WModel은
  1,133,036 bytes, SHA-256 `cb96358d...`로 고정했고 실제 clip은 30 TPS, 32 ticks, animation 5개다.
- 실제 CModel을 0.10초와 0.65초에 샘플해 43개 finite bone palette와 최대 component delta
  `199.123`을 확인했다. 같은 두 시점을 ordinary translucent ModelCue renderer로 WARP draw/readback해
  각각 752/228 nonzero RGB pixel과 서로 다른 frame hash를 확인했다.
- 잘못된 clip restage는 실패하고 직전 정상 late-frame draw를 byte-equivalent readback으로 보존한다.
  따라서 현재 증거는 clip 부재, 정지 bone palette, renderer 미실행, authored/runtime join 누락을 원인에서
  제외한다. 실제 Client에서 두루미가 정지해 보이는지와 카메라/크기/가시성 평가는 사용자 화면 검증
  `PENDING`이며, 이 canary 자체는 제품 데이터나 renderer를 임의 변경하지 않는다.

## 2. 실행한 검증

| 검증 | 결과 |
|---|---|
| Dimension A selective materializer | 9 tests PASS, `--check` PASS |
| family restoration inventory | 23 tests PASS, 22 occurrence/check PASS |
| legacy cue projection | 7 tests PASS |
| rollout orientation focused test | PASS |
| level component animevent v6 parser | 29 tests PASS |
| Debug Engine → UpdateLib → ClientFrontendHarness | PASS |
| Debug `--effect-action-facing-fast` | 8/8 PASS |
| Debug Client | 0 errors PASS |
| Release Engine → UpdateLib → ClientFrontendHarness | PASS |
| Release `--effect-action-facing-fast` | 8/8 PASS |
| Release Client | exit 0, errors 0, 00:09:28.08 |
| Debug/Release Warlord authored subset save | 각 7/7 PASS |
| Debug/Release Client after subset codec change | 각 errors 0 PASS |
| Artist A/D/R + Warlord T focused contracts | 17 tests PASS, 각 materializer `--check` PASS |
| Lance E W-cone selective donor | 4 tests, `--check`, authored codec parse PASS |
| Glasshole02 K01 materializer | 6 tests, `--check`, Debug authored codec parse PASS |
| Debug Glasshole02 K01 HLSL/Client | shader compile + Client errors 0 PASS |
| Debug/Release `--effect-glass-family-fast` | 각 11 PASS; draw 1, issued 1, RGB pixels 283, semantic-channel·carrier-shape reject/rollback PASS |
| Artist S grass-tip selective materializer | 7 tests PASS, `--check` PASS, 4 DDS SHA-256 PASS, Debug/Release codec SHA `03e4e9f8...` 일치 |
| Debug `--effect-standard-color-v1-fast` codec + WARP | failures 0 |
| Release `--effect-standard-color-v1-fast` codec + WARP | failures 0 |
| StandardColorV1 carrier draw | PARTICLE/DECAL/TRAIL nonzero pixels PASS |
| StandardColorV1 semantic matrix | R-vs-A, linear-vs-sRGB, base-radiance emissive PASS |
| StandardColorV1 envelope closure | lifetime/dissolve independence PASS |
| StandardColorV1 BC1 fail-close | R coverage draw, unavailable A rejection, prior stage preservation PASS |
| StandardColorV1 Debug/Release Client | errors 0 PASS |
| Artist D BLACK_TIGER_STROKE materializer | 9 tests PASS, `--check` PASS; 56 baseline raw object byte 보존 |
| Artist D product join verifier | 5 tests PASS, authored/cue/catalog closure PASS |
| Debug Artist D WARP focused harness | opcode allowlist/channel negative, child5/6 draw, sparse coverage, alpha/life/warp sensitivity 전부 PASS |
| Debug Client integration build | 외부 격리 OutDir/IntDir, errors 0 PASS |
| Debug/Release Engine → UpdateLib → ClientFrontendHarness, screen-overlay v1 | 각 errors 0 PASS |
| Debug/Release `--effect-screen-overlay-fast` | 각 13/13 PASS, failures 0 |
| screen-overlay ordered WARP draw | 각 RGB pixel 2968, center `80/128/0` PASS |
| screen-overlay linear/sRGB ABI | 각 center red `128/55` PASS |
| screen-overlay transaction/fail-close | invalid version/provenance/resource/channel preserve, invalid provider rollback/recovery PASS |
| screen-overlay clear/suppression | disable, queued cancel, stop, lifetime expiry 모두 0 draw PASS |
| overlay 이후 Debug/Release `--effect-standard-color-v1-fast` | 각 failures 0 PASS |
| screen-overlay Engine ABI 이후 Debug/Release Client | 각 errors 0 PASS, UI 미실행 |
| Debug/Release Shared + NetworkProtocolHarness build/run | 각 errors 0, failures 0 PASS |
| Debug/Release Server build | 각 errors 0 PASS |
| Debug/Release Server `--contract-test` | Debug 22건, Release 11건 FAIL. 모두 기존 Valtan scripted mechanic/rotation/charge/root-motion/floor-collapse 계열이며 overlay 완료 증거로 승격하지 않음 |
| Fluid01 selective materializer | 7 tests PASS, `--mode check` PASS; target 2행 외 F deep-equal, Artist F golden PASS |
| Debug/Release ClientFrontendHarness build | 각 exit 0 PASS; 기존 C4828/DirectXTK LNK4099 warning만 존재 |
| Debug/Release `--effect-fluid01-family-fast` | 각 failures 0; draw 2, pass `3/3`, mask `0x0f/0x0f`, PS invocation 2452, RGB pixels 1021, second-row sourceNode reject/rollback PASS |
| 전체 `Test-EffectPipeline.ps1` | publisher fixture 본체 PASS; Python 82 tests 중 이번 Fluid01 7 tests PASS, 기존 Valtan cardinality/gap 기대값 drift 3 failure + 2 error로 최종 exit 1 |
| Debug/Release ClientFrontendHarness CircleSurface/Vortex build | 각 exit 0 PASS |
| Debug/Release `--effect-source-circle-vortex-fast` | 각 6/6 PASS; 7-count/260cm/radial velocity, 30/60/120FPS Vortex trajectory, strict reject/rollback, Dimension F와 Artist V canary PASS |
| Debug/Release `--effect-source-null-cdo-size-fast` | 각 2/2 PASS |
| Debug 전체 `--effect-executor-fast` 보조 실행 | 신규 CircleSurface/Vortex 6 assertions는 PASS. 격리 worktree에 Git 비관리 `Client/Bin/Resources`가 없어 기존 resource-dependent 항목들이 실패했으므로 전체 회귀 PASS로 기록하지 않음 |
| Lance dragon selective materializer | 8 tests PASS, `--check` PASS, artifact SHA-256 `94b4f972...` |
| Debug `--effect-dragon-flow-fast` | 15 checks PASS; body 386/head 312 RGB pixels, alpha-pan/Dynamic-W/lifetime 분리 PASS |
| Release `--effect-dragon-flow-fast` | 15 checks PASS; Debug와 같은 pixels/sums/signatures, failures 0 |
| Lance dragon failed restage | second sourceNode malformed reject, 이전 body/head draw와 RGBA readback byte 보존 PASS |
| Lance dragon Client Debug/Release | 각 compile/link/FXC errors 0 PASS; Client/UI 미실행 |
| Artist V wisp selective materializer | 8 tests PASS, `--check` PASS; invalid receipt write-before-validate와 raw-tail indentation regression 포함 |
| Debug `--effect-target-attractor-fast` | 12/12 PASS; 30/60/120FPS 동일 frame, VelocityOverLife 불변, world metric, moving Element, actual V source burst 14 전원 capture, invalid parse/stage rollback, Reset clear |
| Release `--effect-target-attractor-fast` | Debug와 동일한 12/12 PASS, failures 0 |
| Artist V attractor 이후 Debug/Release Client | 각 errors 0 PASS; Release warnings 2173은 기존 FXC/C4819/DirectXTK 계열, Client/UI 미실행 |
| 차원 F Product overlay publisher | Publish/Validate 각 206 Effects PASS; embedded presentation/DDS identity와 actual cue join PASS |
| screen-overlay runtime validator focused unit | 5 tests PASS; valid binding, payload hash, resource closure, ordered field와 prior fixture preservation |
| Debug/Release `--effect-dm-f-product-overlay-fast` | 각 18/18 PASS; 기존 8행 raw identity, actual cue/catalog, DDS identity, `.69/.70/.72/.78s` timeline, fifth-resource rollback, Product GPU prewarm, 짧은 world carrier보다 긴 overlay tail, lifecycle와 post→overlay order |
| Debug/Release `--effect-screen-overlay-fast` 재검증 | 각 13/13 PASS; WARP pixels 2968, linear/sRGB `128/55`, transaction/clear/order PASS |
| Debug/Release Client | 각 errors 0, link PASS; UI/Client 자율 실행 안 함 |
| Debug/Release `--effect-tool-presentation-family-fast` | 각 8/8 PASS; eight-family resolve, presentation-only drawable/playback preview, typed Light/Post atomic persistence, invalid profile rollback, delete/isolation PASS |
| Presentation Tool 변경 후 Debug/Release Client | 각 errors 0, link PASS; Tool/UI 자율 실행 안 함 |
| Debug/Release `--effect-artist-e-fast` | 각 18/18 PASS; 실제 skill/effectref/runtime sealed join, 세 FlowRibbon draw, 30 TPS/32 tick crane clip, 43-bone palette 변화, 752/228 pixel distinct frame, invalid clip rollback PASS |
| `Sync-EffectDataProject.ps1 -Check` | `files=1811`, `filters=201` PASS |
| `git diff --check` | PASS |

Release Client 경고 2173건은 기존 FXC X4717/X4000, C4819, DirectXTK LNK4099 계열이며
`ACTION_FACING` 컴파일/링크 오류는 없었다.

## 3. 아직 완료로 올리지 않는 경계

- checked-in runtime catalog는 아직 차원 A의 기존 9행 sealed document를 가리킨다. 신규
  세 행은 `AUTHORING_ONLY/AUTHORED_NOT_PUBLISHED`이다.
- 도화가 D effectref/source catalog와 A/R/S, 워로드 T authored 변경도 full publish 전에는
  checked-in sealed runtime catalog에서 실행되지 않는다. D의 tiger 12행 typed Product 문서와
  renderer admission은 준비됐지만 sealed catalog publish와 사용자 first-pixel 승인은 별도다.
- Glasshole02 opcode 16 authored packet도 checked-in runtime catalog가 pre-promotion sealed document를
  가리키므로 현재 `AUTHORING_ONLY/AUTHORED_NOT_PUBLISHED`이다.
- screen-overlay v1의 첫 실제 Product consumer는 차원 F `2050230`에 연결했다. 다만 screen-space
  carrier가 source-proven이 아니므로 이 5행은 `PROJECT_TUNED/PARTIAL`이며, 실제 화면 유리 파편과
  refraction 품질은 사용자 visual 판정 전이다. W Product rollout도 아직 하지 않았다.
- Server contract-test는 이 변경이 건드리지 않은 Valtan scripted mechanic과 floor-collapse 기준선에서
  Debug 22건, Release 11건 실패했다. Server 빌드 자체는 양 configuration 모두 통과했지만 이 실행을
  회귀 PASS로 기록하지 않는다.
- Fluid01 opcode 17도 현재 authored 두 행과 renderer/harness까지의 vertical slice다. checked-in sealed
  runtime catalog publish와 사용자 Client 화면 판정 전에는 `COMPLETE`로 올리지 않는다.
- CircleSurface/Vortex strict seam과 도화가 V root-local attractor 실행/authoring은 닫혔다. 다만 V의
  camera/post/light composition, checked-in sealed runtime publish와 사용자 화면 판정은 구현 완료로
  올리지 않는다. attractor는 원본 module exact가 아니라 명시적 `PROJECT_TUNED` layer다.
- screen-space textured shard overlay의 Product ownership, resource identity, rollback과 RGBNoise/ZoomBlur
  뒤 ordered composition은 자동 검증으로 닫혔다. 다음 단위는 source-proven screen carrier 또는 사용자
  승인 tuning, scene-color refraction과 normal/noise/mask/dissolve multi-lane, F raw world composition 및 W
  reuse다.
- Lance dragon opcode 19의 다섯 Product 문서도 sealed runtime catalog publish와 사용자 first-pixel
  승인이 남아 있다. `34610`과 Artist T는 같은 용이라는 이유만으로 이 exact variant에 admission하지
  않았다.
- full Effect publisher Publish/Validate는 이 기준점에서 206 Effects PASS했다. 생성된 runtime catalog에는
  이미 통합된 다른 character/Valtan 미publish 변경도 함께 포함되므로 이 세션은 Valtan source/cue/binding을
  수정하거나 그 결과를 자기 완료 증거로 소유하지 않는다.
- 실제 Client의 동/서/남/북 cast 방향, 검격 형태·타이밍·색감은 사용자 화면 판정 대기다.
- 워로드 W의 `fm_a_hemisphere_012` 하나가 보이는 방패 세 판을 포함하는지는 사용자
  화면 판정 후 독립 좌/중/우 cohort 추가 여부를 결정한다.

## 4. 다음 구현 단위

1. 도화가 T ribbon과 도화가 V camera/post/light composition을 닫는다.
2. F raw 69행의 sphere/hemisphere/world shard/light/post와 scene-color refraction·multi-lane을 확장하고 W consumer를 rollout한다.
3. Fluid01 mesh/J-child, 창술사 `34610`, Q/A 등 source-evidence가 닫힌 variant를 carrier별로 확장한다.
4. 차원술사 T의 Server-authoritative ground targeting과 승인 target-root damage/effect를 닫는다.
5. 이 세션의 authored/source catalog를 한 번의 Effect publisher transaction으로 sealed runtime에 반영하고 전체 Debug/Release focused harness 뒤 사용자가 실제 Client 화면을 판정한다.
