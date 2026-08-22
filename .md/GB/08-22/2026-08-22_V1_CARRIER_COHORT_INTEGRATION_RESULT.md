# V1 Carrier Cohort 통합 결과

branch: `codex/v1-carrier-cohort`

integration base: `origin/main@48c53f20` (PR #151 창술사 D/F profile39/40 확장 포함)

implementation merge: PR `#152`, `main@dc436ae8fdf4b9d68251785c0c18aa1f8d2062e7`

master plan: `2026-08-22_FOUR_CHARACTER_VALTAN_EFFECT_V1_FULL_MIGRATION_MASTER_PLAN.md`

## 1. 결론

요청한 순서인 `창술사 D Mesh control -> 워로드 F WPO Mesh -> 차원술사 F Fluid Sprite ->
창술사 F MakeFlow`를 하나의 carrier cohort로 통합했다. source 전량이나 과거 invisible 행을 되살리지
않았으며, 현재 V0 Product와 손튜닝을 기준선으로 보존한 채 의미가 확정된 네 계약만 다뤘다.

```text
IMPLEMENTED_AUTOMATED
  carrier/family packet, deterministic materializer, publisher, Debug/Release

USER_REVIEW_PENDING
  Effect Tool Solo와 전체 스킬 composition의 화면 판정
```

자동 검증은 모두 통과했지만 사용자가 화면을 아직 승인하지 않았으므로 이번 cohort 전체를
`V1_COMPLETE` 또는 visual PASS로 선언하지 않는다.

## 2. KEEP / REPLACE / ADD / RETIRE 정본

네 단어는 파일 수정 방식이나 material 변경 이름이 아니라 Product carrier의 terminal 판정이다.

| 판정 | 의미 | 승인 전 Product 처리 |
|---|---|---|
| `KEEP` | 현재 stable 행의 carrier와 semantic role이 source evidence와 일치 | 같은 행에서 material ABI만 승격하고 손튜닝 유지 |
| `REPLACE` | 역할은 맞지만 Sprite/Mesh/Decal/Trail carrier 또는 attachment가 틀림 | Tool 후보를 먼저 만들고 사용자 승인 뒤 predecessor와 원자 교체 |
| `ADD` | V0에 없지만 기존 행과 겹치지 않는 의미 있는 source 역할 | default-off Tool 후보로 시작하고 사용자 승인 뒤 추가 |
| `RETIRE` | 중복, 근거 부재 또는 RT0에서 의미 없는 source 역할 | Product에 넣지 않고 receipt에 사유만 보존 |

`PENDING_EVIDENCE`와 `USER_REVIEW_PENDING`은 terminal 판정이 아니다. 특히 material sourceProfile을
바꿨다는 이유로 carrier를 `REPLACE`라 부르지 않는다. bulk append/regenerate, texture 교환으로 carrier
종류를 가장하는 일, 사용자 승인 전 predecessor 삭제는 금지한다.

## 3. 이번 cohort의 실제 판정

| 순서 | 대상 / stable ID | carrier | material action | Product 상태 | 자동 상태 |
|---:|---|---|---|---|---|
| 1 | 창술사 D `c7469f2311b49e44ed801be8` | `KEEP` | 없음, profile15 control | 88행 byte/semantic 동결 | control join PASS |
| 2 | 워로드 F `8c0d6ab070c1a6c83479e590`, `59e6ffa8852fba74279b6ae9` | 미부여 (`reviewState=USER_REVIEW_PENDING`) | Tool-only opcode22 RT0 | Product 56행 byte 동결 | canary + data-only reuse PASS |
| 3 | 차원술사 F `1ae3416ac205fee634b746a9`, `ed33fb10661afb8854e76957` | `KEEP`, `KEEP` | 기존 opcode17 유지 | 기존 8행과 손튜닝 유지 | RT0 witness PASS |
| 4 | 창술사 F `dfc359983bf57e958f75740d` | `KEEP` | `PROMOTE_EFFECTIVE_PARENT_PROFILE36` | target sourceProfile만 변경·publish | five-lane join PASS |

이번 cohort에서 승인된 carrier `REPLACE/ADD/RETIRE`는 0이다. 워로드 F 후보의 다음 행동은
`ADD_OR_REPLACE_PENDING`이며 사용자 화면 승인 뒤에만 terminal 판정을 부여한다.

## 4. 대상별 구현 결과

### 4.1 창술사 D source Mesh control

- carrier: `Effect/LanceMaster/Meshes/fm_m_ring_001.wmodel`
- material: `effect.ue3.missiletrail-two-emissive.v1`, effective profile15
- Product, EffectCatalog, skillbinding, animevent를 변경하지 않았다.
- 이번 cohort의 no-code 대조군이며 기존 first pixel이 유지되는지를 사용자가 확인한다.

### 4.2 워로드 F WPO SinWave Mesh

Product `effect.warlord.skill.17140.unified` 56행과 byte SHA-256
`52d8c29d3af8e8fdf27f32bd8882b25b56c6fab737a83a5bf87bbcc6ebec81ff`를 동결했다.
별도 Tool 문서 `effect.warlord.skill.17140.wpo-sinwave-v1.unified`에 정확히 두 source occurrence만
투영했다.

- carrier: `fm_d_electric_05_vertexcolor.wmodel`
- child/parent: `fx_d_me_worldpositionoffset_sinwave_01_04_ad` /
  `fx_d_me_worldpositionoffset_sinwave_01_ad`
- executor: class-neutral `RuntimeMaterialV2 opcode 22`, Mesh preview dispatch
- proven lane 0: `21.map_c -> fx_i_thunder_02_ycl.dds`, coverage R
- proven lane 1: `02.map_e -> fx_d_atypical_049.dds`, emission RGB
- admission: exact stable ID/sourceNode/child/parent/carrier/resource/module/packet allowlist, drift fail-close
- second occurrence: 추가 C++/HLSL 0인 data-only packet reuse

원본 WPO vertex displacement, parent `06.map`, `12.map_f`, symbol dependency와 source-exact sampler는
`PENDING_EVIDENCE`다. 따라서 현재 결과는 `PROJECT_RECONSTRUCTED / TYPED_RT0_BASE_ONLY`이며 이름의
WPO가 native vertex parity 완료를 뜻하지 않는다. Tool 후보는 authoring catalog에는 있지만 animevent와
active Product runtime에는 의도적으로 연결하지 않았다.

### 4.3 차원술사 F Fluid01 Sprite

raw F source에서 child `fx_w_pa_fd_01_3_tr`, parent `fx_mm_fluid_01_tr`를 쓰는 Sprite는 Product에
이미 남아 있는 위 두 행뿐이다. 둘 다 opcode17 typed packet과 올바른 Sprite carrier를 유지한다.

- authored -> EffectCatalog -> sealed runtime -> animevent join PASS
- DXT1/ATI2 DDS와 opcode17 식을 소비한 32x32 reference-positive first-pixel witness:
  - canary: `773 / 1024` nonzero
  - data-only reuse: `774 / 1024` nonzero
- 다른 Product 6행, transform, size, timing과 HLSL/C++는 변경하지 않았다.

이 수치는 화면 품질 승인이 아니라 texture/packet/timing이 전부 0으로 닫히지 않는다는 구조 증거다.

### 4.4 창술사 F MakeFlow Mesh

현재 `fm_o_swing_02.wmodel` Mesh carrier와 stable 행, 1.4053초 timing, transform과 다른 Product
185행을 유지했다. child-only sourceProfile을 exact child와 effective parent가 합쳐진 profile36으로
승격했다.

| lane | parameter | DDS | ownership |
|---:|---|---|---|
| 0 | `opacity_tex` | `fx_m_spatter_001_xyclamp.dds` | child override |
| 1 | `diff_tex1` | `fx_h_atypical_01_1.dds` | child override |
| 2 | `diff_tex2` | `fx_i_atypical_03_2_xcl.dds` | child override |
| 3 | `color_tex` | `fx_l_environment_001.dds` | parent inherited |
| 4 | `flowtex` | `fx_d_noise_006.dds` | child override |

28 effective scalar를 연결했다. 원본 DynamicParameter 이름 네 개가 모두 `none`이므로 추측 이름을
만들지 않고 `unbound x4`, profile36 fallback `[1, 0, 0, 0]`, channel 3 suppressed로 봉인했다.
신규 C++/HLSL 없이 기존 MakeFlow RT0 family를 재사용했다.

## 5. 통합 중 닫은 재현성 문제

격리 worktree에서는 통과했지만 fresh Windows checkout에서 워로드 materializer가 HLSL의 CRLF를
LF와 다른 byte identity로 오판했고, 창술사 materializer도 exact profile36이 이미 적용된 Product의
CRLF target block을 stale로 오판했다. JSON은 semantic object로 비교하고 HLSL/C++ 구현 receipt는
`CRLF_TO_LF` canonical bytes로 봉인한다. 필드/식/리소스 drift는 계속 fail-close하며 단순 줄바꿈만
stale로 처리하지 않는다.

PR #151의 profile39/40과 profile36이 섞이지 않도록 MakeFlow 검증에는
`MAKEFLOW_02 enum -> runtime profile36 -> five-lane texture stage -> RT0 HLSL` bridge hash를 추가했다.
현재 D는 profile39 14행, F는 profile39 12행/profile40 48행/profile36 1행이며 각 selector는 배타적이다.

## 6. 자동 검증

```text
DimensionMaster Fluid01 focused                  8 / 8 PASS
LanceMaster D/F focused                         8 / 8 PASS
Warlord F WPO focused                          10 / 10 PASS
typed source-profile join audit                  4 / 4 PASS
Effect materializer --check                     PASS x3
Sync-EffectDataProject -Check                    PASS, 1865 files / 210 filters
Publish-Effects -Mode Validate                   PASS, 207 active Effects
Publish-Effects -Mode Publish                    PASS, 207 active Effects
EffectPipeline full contract                     PASS, 130 tests
visual-program artifact                          PASS, 17 programs / 135 rows
Client x64 Debug compile/link                    PASS
Client x64 Release compile/link                  PASS
```

창술사 F 변경은 새 sealed authored document
`effect.lancemaster.skill.34150.unified.0fc9cebe4d53be9184aa1cc275adf8e1742fb65079a15b43e8df4e6617adb4b3.effect.json`
으로 publish됐다. 워로드 후보는 Tool-only라 active runtime 207건 수에 들어가지 않는 것이 정상이다.
기존 FXC X3577/X4000, C4819와 DirectXTK LNK4099 경고는 남지만 compile/link error는 없다.

## 7. 사용자 수동 검증 순서

에이전트는 Client와 Effect Tool을 실행하지 않았고 visual PASS를 대신 선언하지 않는다. 통합된 main
빌드에서 다음을 직접 확인한다.

1. `effect.lancemaster.skill.34110.unified`
   - `authored.source-particle.c7469f2311b49e44ed801be8` Solo
   - D ring/missile-trail control이 기존과 같은지 확인
2. `effect.warlord.skill.17140.wpo-sinwave-v1.unified`
   - `8c0d...` Solo 후 `59e6...` Solo, 마지막에 두 행 동시 재생
   - 전기 mesh 형상, alpha 경계, emission, dissolve, 위치/크기 확인
3. `effect.dimensionmaster.skill.2050230.unified`
   - `1ae3...`와 `ed33...`를 각각 Solo
   - 두 Fluid Sprite의 first pixel, 원형 경계, 색, timing 확인
4. `effect.lancemaster.skill.34150.unified`
   - `dfc359983bf57e958f75740d` Solo
   - MakeFlow mesh의 coverage, color, UV 흐름과 1.4053초 전체 composition 확인

판정 결과는 각 대상별로 `보임/안 보임`, `carrier 의미 있음/틀림`, `경계`, `색`, `UV/시간`,
`배치 재튜닝 필요`를 기록한다. 이 결과로 워로드 후보의 terminal `KEEP/REPLACE/ADD/RETIRE`와 네
대상의 `V1_COMPLETE` 승격 여부를 결정한다.
