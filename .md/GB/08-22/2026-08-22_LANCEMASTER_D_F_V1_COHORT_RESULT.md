# 창술사 D·F Typed RT0 Base V1 Cohort 결과

branch: `codex/v1-cohort-lance`

base: `main@dc280ff55cc49d7f92c0d84737fd939bbf440dca`

master plan: `2026-08-22_FOUR_CHARACTER_VALTAN_EFFECT_V1_FULL_MIGRATION_MASTER_PLAN.md`

## 1. 결론

창술사 D는 이미 동작하는 MeshParticle + MissileTrail profile15 행을 변경 없는 대조군으로
봉인했다. 창술사 F는 현재 Product 186행과 손튜닝값을 그대로 유지하고, MakeFlow02 MeshParticle
한 행의 `material.sourceProfile`만 effective parent profile36으로 승격했다.

```text
D control
  authored.source-particle.c7469f2311b49e44ed801be8
  carrier disposition     KEEP
  material action         NONE

F canary
  authored.source-particle.dfc359983bf57e958f75740d
  carrier disposition     KEEP
  material action         PROMOTE_EFFECTIVE_PARENT_PROFILE36
```

candidate terminal은 carrier/role/row 판정이므로 D와 F 모두 `KEEP`이다. F의 source-profile 교체는
별도 material action이다. `ADD/REPLACE/RETIRE` carrier는 모두 0이며 source 전량 복구나 Product
bulk mutation을 하지 않았다.

자동 구현 상태는 `IMPLEMENTED_AUTOMATED`다. 화면 결과는 사용자가 아직 판정하지 않았으므로
`PROJECT_RECONSTRUCTED_BOUNDED / USER_REVIEW_PENDING`이며 `V1_COMPLETE`가 아니다.

## 2. 보존 경계

### D control

- Product 문서 88행 전체의 canonical SHA-256을 동결했다.
- stable ID, Mesh carrier `fm_m_ring_001.wmodel`, transform, timing, source recipe를 변경하지 않았다.
- `effect.ue3.missiletrail-two-emissive.v1`과 effective source material profile15를 유지했다.
- EffectCatalog, skillbinding, `flm_sk_crescentsweep` animevent Product join이 현재 상태인지 검사한다.

### F canary

- Product 문서 186행 중 target 한 행만 소유한다.
- 다른 185행의 canonical SHA-256을 동결했다.
- target의 stable ID, Mesh carrier `fm_o_swing_02.wmodel`, source material child, transform,
  start `1.4053s`, particle lifetime, source recipe를 변경하지 않았다.
- target의 `material.sourceProfile` 외 material/render state도 동결했다.
- EffectCatalog, skillbinding, `flm_sk_crushingblow` animevent Product join을 유지했다.

## 3. F MakeFlow02 effective-parent 계약

정확한 child MIC는
`fx_m_mi_02.fx_mi.fx_k_me_makeflow_02_13_tr`, effective parent는
`fx_m_mi_02.fx_m.fx_k_me_makeflow_02_tr`다. 기존 child-only profile은 parent의 inherited
`color_tex`와 8개 scalar default를 누락했으므로 profile36이 요구하는 다섯 lane과 28개 scalar로
승격했다.

| 순서 | parameter | Product DDS | 근거 |
|---:|---|---|---|
| 0 | `opacity_tex` | `fx_m_spatter_001_xyclamp.dds` | child override |
| 1 | `diff_tex1` | `fx_h_atypical_01_1.dds` | child override |
| 2 | `diff_tex2` | `fx_i_atypical_03_2_xcl.dds` | child override |
| 3 | `color_tex` | `fx_l_environment_001.dds` | effective parent inherited |
| 4 | `flowtex` | `fx_d_noise_006.dds` | child override |

다섯 texture는 현재 grouped-source profile36과 기존 MakeFlow RT0 HLSL 경로를 재사용한다. 신규
C++, HLSL, shader opcode, `.vcxproj`, `.filters`는 필요하지 않았다.

## 4. DynamicParameter bounded 판정

원본 particle module에는 네 `ParamName`이 모두 문자열 `none`으로 저장돼 있다. 이 값을 임의의
의미 이름으로 바꾸거나 source-exact라고 승격하지 않았다.

```text
source names             none / none / none / none
runtime semantics        unbound / unbound / unbound / unbound
profile36 fallback       1.0 / 0.0 / 0.0 / 0.0
channel 3                suppressed, unused, default zero
```

따라서 flow strength는 기본 1, opacity pan V/U는 기본 0을 사용하며 네 번째 channel은 소비하지
않는다. 이 부분은 `PROJECT_RECONSTRUCTED_BOUNDED`이고 사용자 화면 승인 전에는 원본 동등성을
주장하지 않는다.

## 5. deterministic materializer와 검증 계약

`materialize_lancemaster_34110_34150_v1_cohort.py`는 다음을 fail-close한다.

- D 전체 문서와 control 행 drift
- F envelope, non-target 185행, target의 carrier/transform/timing/source recipe drift
- MakeFlow family row, exact child package binding, 다섯 texture와 28 scalar declaration drift
- anonymous DynamicParameter 네 channel drift
- 11개 runtime Mesh/DDS content identity drift
- EffectCatalog/skillbinding/animevent Product join drift

legacy child-only profile fixture에서는 target의 `material.sourceProfile`만 교체하고 D와 F non-target을
byte/semantic 보존한다. 실패는 Product 파일을 쓰기 전에 중단한다.

## 6. 자동 검증

### PASS

```text
python -m unittest Tools.EffectPipeline.test_materialize_lancemaster_34110_34150_v1_cohort -v
  Ran 7 tests, OK

python Tools/EffectPipeline/materialize_lancemaster_34110_34150_v1_cohort.py --mode check
  stable / D KEEP / F KEEP / profile36 / five lanes / unbound x4

modified JSON parse
  PASS: authored F document and generated receipt

git diff --check
  PASS: whitespace error 없음
```

`Publish-Effects.ps1 -Mode Validate`는 artifact check 뒤 장시간 실행돼 bounded cohort 종료 전에 완료
판정을 얻지 못했다. 중복 실행은 중단했으며 PASS로 기록하지 않는다. Publish, 전체 EffectPipeline,
project registration과 Debug/Release build는 최신 main 통합 브랜치에서 재실행한다. 따라서 이 commit은
authored + materializer + receipt 단위이고 runtime sealed catalog 생성물은 포함하지 않는다.

## 7. 사용자 수동 검증

에이전트는 Client나 Effect Tool을 실행·조작하지 않았고 화면 PASS를 대신 선언하지 않는다. 사용자는
통합된 빌드를 직접 실행해 다음을 확인한다.

1. `effect.lancemaster.skill.34110.unified`를 열고
   `authored.source-particle.c7469f2311b49e44ed801be8`만 Solo한다.
2. D의 ring/missile trail이 기존과 동일하게 보이는지 대조군으로 확인한다.
3. `effect.lancemaster.skill.34150.unified`를 열고
   `authored.source-particle.dfc359983bf57e958f75740d`만 Solo한다.
4. F MakeFlow mesh가 first pixel을 내고 사각형/잘못된 coverage가 아니라 flow 경계를 보이는지 확인한다.
5. 색·알파·UV 흐름과 1.4053초 시점의 전체 F composition을 확인한다.
6. 필요하면 Effect Detail의 기존 transform/scale/timing 손튜닝값만 조정하고 family ABI는 유지한다.

```text
D control first pixel                 PENDING
F MakeFlow first pixel                PENDING
F coverage / color / UV flow          PENDING
F full composition                    PENDING
```

사용자 승인 뒤에만 이 canary를 `V1_COMPLETE`로 올리고 같은 MakeFlow family occurrence로 데이터
확장을 검토한다.
