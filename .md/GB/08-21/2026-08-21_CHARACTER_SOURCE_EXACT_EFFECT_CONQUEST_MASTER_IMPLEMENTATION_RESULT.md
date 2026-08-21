# 2026-08-21 Character Source-Exact Effect Conquest Master 구현 결과

기준 커밋: `812d23a4`
구현 브랜치: `codex/effect-family-conquest`
상태: 진행 중 living RESULT
최종 화면 판정자: 사용자

Fluid01 추가 checkpoint는 기준 `b705abb7`, 격리 브랜치 `codex/dm-fluid01`에서 검증했다.

이 문서는 마스터 PLAN의 실제 반영 상태만 기록한다. 도화가·워로드·창술사·차원술사
전체 복원은 아직 진행 중이며, 이 버전에서는 family inventory, action-facing, opt-in standard
color/coverage 수직 슬라이스를 구현했다.

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
- 이 수직 슬라이스는 synthetic DDS canary를 첫 소비자로 둔 `PROJECT_TUNED/TYPED_PRESENTATION`이다.
  차원 F/W Product occurrence와 원본 화면 유리 파편은 아직 연결하지 않았고 source-exact native DXBC,
  refraction, normal/noise/mask/dissolve 다중 lane도 후속 revision 경계다.
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
- screen-overlay v1은 synthetic canary만 소비한다. 차원 F/W Product effectref/catalog/occurrence에는
  아직 연결하지 않았고, 실제 화면 유리 파편과 refraction 품질은 사용자 visual 판정 전이다.
- Server contract-test는 이 변경이 건드리지 않은 Valtan scripted mechanic과 floor-collapse 기준선에서
  Debug 22건, Release 11건 실패했다. Server 빌드 자체는 양 configuration 모두 통과했지만 이 실행을
  회귀 PASS로 기록하지 않는다.
- Fluid01 opcode 17도 현재 authored 두 행과 renderer/harness까지의 vertical slice다. checked-in sealed
  runtime catalog publish와 사용자 Client 화면 판정 전에는 `COMPLETE`로 올리지 않는다.
- screen-space textured shard overlay의 renderer와 rollback 계약은 닫혔지만 아직 synthetic canary다.
  다음 단위는 exact shard texture/role과 F/W Product timing·ownership을 typed descriptor로 연결하고,
  RGBNoise/ZoomBlur와의 실제 ordered composition을 sealed runtime에서 검증해야 한다.
- full Effect publisher Validate는 현재 main의 `effect.valtan.red-blade-wave.active` 중복 소유자
  검증에서 중단된다. 이 세션은 해당 Valtan 데이터를 수정하지 않았다.
- 이 기준점의 전체 EffectPipeline Python suite는 Valtan canonical occurrence 기대값 `128` 대 실제
  `137`, migration 기대 tuple과 binding-gap 기대가 불일치한다. Fluid01 변경은 Valtan 파일을 수정하지
  않았고 focused Fluid01 tests/Debug·Release WARP는 모두 통과했다.
- 실제 Client의 동/서/남/북 cast 방향, 검격 형태·타이밍·색감은 사용자 화면 판정 대기다.
- 워로드 W의 `fm_a_hemisphere_012` 하나가 보이는 방패 세 판을 포함하는지는 사용자
  화면 판정 후 독립 좌/중/우 cohort 추가 여부를 결정한다.

## 4. 다음 구현 단위

1. Lance/Artist dragon의 독립 UV·dissolve typed family와 실제 12 occurrence admission을 완료한다.
2. F raw 69행의 sphere/hemisphere/world shard/light/RGBNoise/ZoomBlur를 role-select하고 screen-overlay Product timing에 연결한다.
3. Fluid01 mesh와 W의 동일 exact variant cohort를 carrier별 admission하고 Artist T ribbon·V attractor/camera family를 닫는다.
4. full Effect publisher transaction, 전체 Debug/Release focused harness 뒤 사용자가 실제 Client 화면을 판정한다.
