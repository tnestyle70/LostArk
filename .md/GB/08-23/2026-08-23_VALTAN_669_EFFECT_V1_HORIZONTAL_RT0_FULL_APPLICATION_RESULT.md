# 2026-08-23 Valtan 669 ledger 기반 3패턴 V1 Unified 제품 audition 결과

최종 화면 판정자: 사용자

대응 계획:

- [`Valtan 669 ledger 기반 3패턴 V1 Unified 제품 audition 구현 계획`](2026-08-23_VALTAN_669_EFFECT_V1_HORIZONTAL_RT0_FULL_APPLICATION_PLAN.md)

## 0. 현재 결론

Valtan 세 패턴의 여섯 V0 자산을 수정하지 않고, 같은 occurrence와 Composition을 가진 여섯
`.v1.unified` 자산을 병렬로 생성했다. material 대상 37행은 모두 stable occurrence Binding을 가진
`PROJECT_TUNED_APPROX`로 실행하며, Ribbon 3행과 Light 1행은 계산식을 발명하지 않고
`V0_PRESENTATION_CARRY`로 보존한다.

이 문서의 `세 패턴`은 하나의 3연 공격이 아니라 `VALTAN_WHIRLWIND`, `VALTAN_MAGIC_CHOICE` 도넛,
`VALTAN_ARMOR_BREAK_OPENING/WALL_CHARGE` 돌진이라는 서로 다른 세 Product pattern이다.

이 결과는 **원본 Material의 whole-tuple exact 복원 결과가 아니다.** 세 패턴 안에는 source Program이
`SOURCE_EXACT`로 색인된 행도 있지만, 같은 occurrence에서 Program, Runtime ABI, Descriptor, state와
carrier/pass가 함께 닫힌 행은 없다. 따라서 runtime `SOURCE_EXACT` 승격은 0행이고, 37행 전부를
사실대로 project-tuned base-coverage approximation으로 분류했다.

| 항목 | 상태 | 실제 증거 |
|---|---|---|
| 구현 | `IMPLEMENTED_IN_BRANCH` | opcode `1001/1002`, 두 Layout, 8 Adapter, 37 Descriptor/Binding 및 Sprite/Mesh renderer dispatch 구현 |
| 669행 inventory와 41행 materialization | `GENERATED` | 669행 application contract와 41행 materialization receipt 생성 |
| 여섯 `.v1.unified` 자산 | `GENERATED` | V0 6개를 immutable input으로 봉인하고 V1 6개 생성 |
| registry Binding | `GENERATED` | V1 material occurrence 37개, unbound material row 0 |
| Product cue V1 route | `IMPLEMENTED_DEBUG_ONLY_BEST_EFFORT` | 별도 alias sidecar 6행과 `Use V1 .effect.unified aliases` Debug toggle; 기본 Product와 V0 자산은 불변 |
| Debug 자동 검증 | `PASS` | focused Python, publisher, 최신 main 기준 Debug Client build와 통합 `ExpectedBindingCount=171` render harness 통과 |
| Release 자동 검증 | `NEEDS_REVALIDATION` | 최신 selector ABI/UV/SubUV 변경 전 pass는 stale이므로 재빌드·재실행 필요 |
| 정본 folder publish/build | `PENDING_INTEGRATION` | 현재 구현은 별도 worktree이며 `C:\Users\user\Desktop\LostArk` 정본에는 아직 통합하지 않음 |
| 사용자 V0/V1 A/B | `PENDING` | Client를 에이전트가 실행·조작하지 않았고 사용자 서면 판정 없음 |

## 1. 통합 기준

| 항목 | 실제 값 |
|---|---|
| 구현 branch | `codex/valtan-effect-v1-smd-cohorts` |
| 구현 branch HEAD | latest main `24cce2e0` 위의 `codex/valtan-effect-v1-smd-cohorts` 단일 검증 커밋 |
| 기준 main | `24cce2e0` (`PR #178` 대표 4스킬과 `PR #179` 문서 감사를 포함한 최신 main) |
| 4캐릭터 PR/merge | `PR #178` main 병합 및 이 branch rebase 완료 |
| Valtan PR/merge | 아직 없음 |
| 정본 folder HEAD | 아직 갱신하지 않음 |
| runtime registry publish 증거 | Effect publisher `156 effects / 171 bindings`; 공용 golden 3 + 대표 4스킬 131 + Valtan V1 37 |

source registry base는 공용 compiled Adapter만 소유하고, Program/Layout/Descriptor/Binding은 domain fragment
merge로 materialize한다. Valtan 데이터를 base registry에 하드코딩하거나 스킬 ID C++ switch로 연결하지
않았다.

## 2. 실제 적용 분모

| 판정 | 실제 수량 | 상태 |
|---|---:|---|
| 전체 pattern rows | 41 | `VERIFIED` |
| Sprite material | 25 | `BOUND_PROJECT_TUNED_APPROX` |
| Mesh material | 12 | `BOUND_PROJECT_TUNED_APPROX` |
| whole-tuple `SOURCE_EXACT` | 0 | source Program exact 색인과 runtime exact admission을 구분함 |
| `PROJECT_TUNED_APPROX` | 37 | `BOUND` |
| unbound material row | 0 | `VERIFIED` |
| Ribbon carry | 3 | `V0_PRESENTATION_CARRY` |
| Light carry | 1 | `V0_PRESENTATION_CARRY` |
| V1 assets | 6 | `GENERATED` |
| V1 alias routes | 6 | `GENERATED` |
| `VALTAN_DASH_CHARGE` rows/claim | 0 | 구현·검증 대상으로 주장하지 않음 |

Material receipt의 opcode 분포는 sRGB base coverage `1001` 17행, linear base coverage `1002` 20행이다.
두 opcode는 같은 one-texture 식을 사용하고 SRV color-space 해석만 구분한다. DDS를 직접 감사한 selected
texture 분포는 `base` 23행과 `mask` 14행이며, coverage는 alpha 9행과 RGB luminance 28행이다.

Layout ABI는 `t0/s5`, `base_coverage`, `RGBA` 한 lane과 scalar 한 개
`coverage-channel-selector.0`을 소비한다. selector 값은 `0 = alpha`, `1 = red`, `2 = RGB luminance`이고
범위 밖 또는 비정수 값은 fail-closed한다. source의 multi-texture equation, 별도 UV distortion/pan,
scene input, MRT distortion과 vertex WPO는 복원했다고 주장하지 않는다.

compiled Adapter는 Sprite/Mesh 각각 `alpha/additive × one/two-sided`, 총 8개다. 현재 37행이 실제로 선택한
variant는 Sprite additive one-sided, Sprite alpha one/two-sided, Mesh alpha one/two-sided의 5개이며, 나머지
3개는 같은 compiled category의 허용 variant다. masked, Decal, Trail/Ribbon 전용 Material 또는 WPO capability는
이번 slice에 포함하지 않았다.

## 3. 실제 구현 경계

### 3.1 병렬 V1 자산과 Composition

- V0 여섯 authored document와 `Valtan.patterneffectcues.json`은 immutable input으로 사용했다.
- V1 여섯 document는 V0 element order, timing, transform, attachment와 sourceRecipe를 유지한다.
- `Data/Animation/Authored/Valtan/Valtan.patterneffectv1aliases.json`이 V0→V1 정확히 6개를 연결한다.
- Product 기본 경로는 계속 V0다. Debug Valtan panel에서 toggle을 켰을 때만 같은 cue가 V1 target을 선택한다.
- V1 alias는 화면 경제성을 확인하기 위한 best-effort Debug-only 격리 경로다. V1 결과나 generation이
  실패해도 기본 Product cue와 V0 authored document를 교체하지 않는다.
- Loading/Character Select prewarm과 Effect Tool direct-authored owner index에 V1 target을 추가했다.
- V0 파일을 V1로 덮어쓰거나 기존 cue의 timing/attachment를 바꾸지 않았다.

### 3.2 Program, ABI와 renderer

- RuntimeMaterialV2에 opcode `1001`과 `1002`의 HLSL equation 및 Particle/Mesh dispatch를 추가했다.
- 두 opcode의 compiled Layout ABI는 `t0/s5`, `base_coverage`, `RGBA`, scalar selector 1개와 고정
  dynamic/static/render mask 계약으로 Python, schema와 C++ receipt에 함께 등록했다.
- Particle path는 carrier가 계산한 transformed current/next UV와 SubUV blend를 식에 전달해 두 sample을
  보간한다. Mesh path는 carrier-transformed UV를 current/next 동일값과 blend 0으로 전달한다.
- 37 occurrence마다 texture 선택, color space, Program/Layout/Descriptor/Adapter stable ID와
  `INLINE_MIRROR_REQUIRED` packet을 생성했다.
- Sprite/Mesh renderer는 registry에서 materialize한 execution descriptor를 기존 carrier draw에 전달한다.
- Adapter는 nominal packet state가 아니라 실제 `Begin(pass)` 뒤 fixed state/MRT receipt를 검증한다.
- one-sided Mesh의 negative determinant는 nominal pass `3/4`를 mirrored pass `5/6`으로 remap하는 runtime
  정책을 구현했다.

### 3.3 생성·수정된 계약 묶음

- authored/catalog: V1 effect document 6개, alias sidecar, EffectCatalog 6행
- evidence: 669행 horizontal application contract, 41행 materialization receipt
- registry: Valtan fragment의 Program 2, Layout 2, Descriptor 37, Binding 37과 공용 base Adapter 8개
- runtime: HLSL family/Particle/Mesh dispatch, C++ registry ABI/compatibility, renderer pipeline receipt
- Product audition: Valtan Debug panel toggle, V1 prewarm, cue-document alias resolve
- tooling: V1 materializer와 11 tests, horizontal ledger builder/tests, registry builder/schema/tests,
  Effect publisher alias validation
- generated runtime: Effect catalog 156 entries, compiled authored V1 document 6개, registry Binding 171개

## 4. 자동 검증

| 명령 또는 gate | 구성 | 결과 | 실제 증거/경계 |
|---|---|---|---|
| `test_materialize_valtan_three_pattern_v1.py` | Python | `PASS` | 11/11; V0 immutability, 6 V1, 37 inline mirror, audited base/mask·coverage selector, 4 carry, fail-closed 검증 |
| `materialize_valtan_three_pattern_v1.py --check` | Python | `PASS` | worktree의 generated artifact byte-current |
| `test_build_valtan_effect_v1_horizontal_rt0_application.py` | Python | `PASS` | 12/12; 669 denominator와 focused pattern ledger 검증 |
| `test_build_effect_material_program_registry.py` | Python | `PASS` | 20/20; 대표 StandardColor와 opcode `1001/1002`, scalar selector Layout ABI, 16 Adapter 및 invalid tuple fail-closed |
| `test_effect_tool_valtan_saved_rows.py` | Python | `PASS` | 8/8 |
| `Publish-Effects.ps1 -Mode Validate` | Data | `PASS` | 156 entries / 171 bindings |
| `Publish-Effects.ps1 -Mode Publish` | Data | `PASS` | 156 effects와 runtime visual sidecar publish |
| Particle/Mesh direct FXC | Shader | `PASS_WITH_EXISTING_WARNINGS` | 두 HLSL compile 성공; 기존 warning만 존재 |
| Client x64 Debug build/link | Debug | `PASS` | worktree `Client\Bin\Debug\Client.exe` 생성 |
| Effect render focused harness | Debug | `PASS` | exit code 0, `ExpectedBindingCount=171`, `actualBindingCount=171`, 대표 131 registry receipt와 golden+대표 actual Adapter draw 8종; Valtan 화면 fidelity 판정은 아님 |
| Effect render focused harness | Release | `NEEDS_REVALIDATION` | 최신 ABI 전 `ExpectedBindingCount=40` pass는 stale; Release 재빌드 뒤 다시 실행해야 함 |
| Client x64 Release | Release | `NEEDS_REVALIDATION` | 최신 ABI 전 build 결과는 stale; 최종 Release link 재실행 필요 |
| full `Invoke-BuildAndRegression.ps1` | Debug/Release | `NOT_RUN_AS_FULL_GATE` | focused Client/shader/harness만 기록 |
| dedicated JSON/XML sweep | Data/Project | `PASS` | 변경·신규 JSON 21개 parse, `Client.vcxproj/.filters` XML parse 통과 |
| `git diff --check` | Git | `PASS_WITH_LINE_ENDING_WARNINGS` | whitespace error 0; 기존 LF→CRLF warning 존재 |

Debug harness는 `ExpectedBindingCount=171`, `actualBindingCount=171`으로 공용 golden 3행, 대표 4스킬
131행과 Valtan 37행을 한 generation에서 load/validate했다. actual WARP draw probe는 golden과 대표
4스킬 Adapter를 검증하며 Valtan 세 패턴 37행의 최종 화면 fidelity를 대신 증명하지 않는다. 해당 경계는
사용자의 Product A/B로 닫는다.

기존 `test_materialize_valtan_carrier_v1.py`는 이 변경과 무관한 Entrance/Arena84 source inventory gap 때문에
test 본문 진입 전에 실패한다. 이 오래된 carrier inventory 실패를 이번 V1 slice의 PASS로 숨기거나, 이번
변경이 해결했다고 기록하지 않는다.

## 5. 남은 자동·통합 경계

### 5.1 negative determinant actual draw

one-sided Mesh의 mirrored pass `5/6` remap 코드는 존재하지만, 이번 Valtan one-sided Mesh 4행은 모두 positive
determinant다. 따라서 negative-scale CModel occurrence를 이용한 **bound actual-draw harness 증거는 아직 없다.**
현재 결과는 runtime policy 구현과 positive determinant rows의 packet/state 검증까지만 증명한다.

### 5.2 Release와 정본 folder

| 항목 | 상태 | 다음 조치 |
|---|---|---|
| Debug render harness | `PASS` | 최신 main과 통합 171 Binding 기준 exit code 0 |
| Release Client/harness | `NEEDS_REVALIDATION` | Debug 완료 뒤 최신 ABI로 Release build/link와 171 Binding harness 재실행 |
| 4캐릭터 공용 diff 통합 | `PASS` | `PR #178` main 병합 상태를 rebase-consume |
| Valtan branch rebase | `PASS` | `24cce2e0` 기준 공용 registry/HLSL/C++ 충돌 해소와 전 focused gate 재실행 |
| Valtan PR | `PENDING` | 공용 파일과 Valtan data를 검증 단위로 commit/push 후 생성 |
| canonical `git pull --ff-only` | `PENDING` | 두 PR 병합 후 `C:\Users\user\Desktop\LostArk`에서 수행 |
| canonical publish/build | `PENDING` | 정본 main에서 Effect publish와 Debug Client build |

현재 worktree의 Debug EXE는 구조 검증 산출물이며 최종 사용자 실행 정본이 아니다. 다른 세션의 dirty 변경을
덮거나 정본 folder를 강제로 branch switch/reset하지 않는다.

## 6. 사용자 수동 A/B

| 대상 | V0 관찰 | V1 관찰 | 사용자 판정 | 결함 stable ID |
|---|---|---|---|---|
| `VALTAN_WHIRLWIND` | `미관찰` | `미관찰` | `PENDING` | `미기록` |
| `VALTAN_MAGIC_CHOICE` | `미관찰` | `미관찰` | `PENDING` | `미기록` |
| `VALTAN_ARMOR_BREAK_OPENING/WALL_CHARGE` | `미관찰` | `미관찰` | `PENDING` | `미기록` |
| V0/V1 Sprite SOLO | `미관찰` | `미관찰` | `PENDING` | `미기록` |
| V0/V1 Mesh SOLO | `미관찰` | `미관찰` | `PENDING` | `미기록` |
| Ribbon 3/Light 1 carry | `미관찰` | `미관찰` | `PENDING` | `미기록` |

사용자 서면 판정 전에는 manual first pixel, visual PASS 또는 Product admission을 기록하지 않는다. 정본 main
통합 뒤 사용자는 동일한 pattern을 V1 toggle OFF/ON으로 각각 재생하고, timing/attachment parity와
Sprite/Mesh coverage, blend/depth/cull, Ribbon/Light carry를 비교한다.

## 7. 다음 의사결정

| 결정 | 상태 | 판정 기준 |
|---|---|---|
| V1 수평 확대 | `UNDECIDED` | 세 패턴에서 사각형/coverage/state 문제가 의미 있게 줄고 Composition parity가 유지됨 |
| V1 stable Binding + Descriptor 손튜닝 hybrid | `UNDECIDED` | 공용 draw는 유효하나 색·밝기·texture 선택의 occurrence별 조정이 필요함 |
| V0/PNG A/B 손튜닝 | `UNDECIDED` | 세 패턴 모두 개선이 작거나 approximation exception 비용이 더 큼 |

V1 toggle의 기본값이 false이므로 화면 결과가 나쁘거나 registry generation이 실패해도 기존 V0 Product cue가
기본 경로다. 여섯 V1 자산과 receipt는 진단 입력으로 남길 수 있으며, V0 cue와 authored document를 되돌릴
Product mutation은 없다.
