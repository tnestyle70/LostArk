# 2026-08-23 Valtan 669 ledger 기반 3패턴 V1 Unified 제품 audition 구현 계획

기준 branch: `codex/valtan-effect-v1-smd-cohorts`

기준 main: `4593e539` (`PR #175` 공용 S6/M3/D14 spine 병합)

최종 화면 판정자: 사용자

관련 정본:

- [`Effect Tuple Cohort Inventory 구현 계획`](2026-08-23_EFFECT_TUPLE_COHORT_INVENTORY_IMPLEMENTATION_PLAN.md)
- [`Effect Tuple Cohort Inventory 결과`](2026-08-23_EFFECT_TUPLE_COHORT_INVENTORY_RESULT.md)
- [`4캐릭터·Valtan Effect V1 전체 마이그레이션 마스터 계획`](../08-22/2026-08-22_FOUR_CHARACTER_VALTAN_EFFECT_V1_FULL_MIGRATION_MASTER_PLAN.md)
- [`Effect Family Runtime ABI 복원 가이드`](../../TEAM/EFFECT_FAMILY_RUNTIME_ABI_RESTORATION_GUIDE.md)

## 0. 이번 실험에서 한 번에 판정할 것

이번 변경은 669행 전체를 즉시 Product V1로 승격하는 작업이 아니다. 669행 ledger를 분모와 근거로
유지하면서, 사용자가 실제 Product 화면에서 경제성을 판정할 수 있는 세 패턴 41행을 별도
`.v1.unified` 자산으로 처음부터 끝까지 닫는 실험이다.

여기서 `세 패턴`은 하나의 3연 공격을 뜻하지 않는다. `VALTAN_WHIRLWIND`, `VALTAN_MAGIC_CHOICE` 도넛,
`VALTAN_ARMOR_BREAK_OPENING/WALL_CHARGE` 돌진이라는 서로 다른 세 Product pattern이다.

```text
Valtan 669 ledger
  -> Whirlwind 12 + Magic Choice 19 + Armor Break Wall Charge 10 추출
  -> material draw 37행을 PROJECT_TUNED_APPROX로 명시하고 whole-tuple SOURCE_EXACT는 0행으로 고정
  -> Ribbon 3 + Light 1은 material이라고 위장하지 않고 기존 presentation을 그대로 carry
  -> V0 여섯 자산을 복사한 병렬 .v1.unified 여섯 자산 생성
  -> V1 occurrence 37행 전부 stable registry Binding
  -> 실제 Sprite/Mesh RT0 draw
  -> V0 cue를 보존한 6행 V0→V1 alias sidecar와 Debug A/B route
  -> build 후 사용자가 동일 패턴과 V0/V1 자산을 A/B
  -> V1 확대 또는 V0/PNG 손튜닝 전환 결정
```

이번 실험의 자동 종료 조건은 `V1 파일이 생김`이 아니다. V1 자산의 material 대상 37행에서
unbound V0 fallback이 정확히 0이고, 각 행의 fidelity가 사실대로 저장되고, 실제 draw까지 도달해야 한다.

사용자 육안 승인 전에는 `V1_PRODUCT_APPROVED`, `SOURCE_EXACT 화면 일치`, `복원 완료`를 기록하지 않는다.

## 1. 왜 S6/M3/D14와 교집합 0이어도 이 실험은 가능하나

`S6`, `M3`, `D14`는 Sprite/Mesh/Decal이라는 carrier 이름이 아니라 도화가 F에서 증명한 특정
`Program × Layout × Adapter` tuple이다. 세 패턴 41행과 exact 교집합이 0이라는 것은 도화가 F 계산식을
발탄에 그대로 붙일 수 없다는 뜻이지, 발탄 carrier나 source 자료가 0이라는 뜻은 아니다.

PR #175에서 재사용하는 것은 다음 공용 실행 spine이다.

```text
domain fragment merge
-> stable occurrence Binding
-> Program/Layout/Descriptor/Adapter strict validation
-> EFFECT_MATERIAL_EXECUTION_DESC materialize
-> inline mirror 비교
-> Sprite/Mesh/Decal actual draw
-> parse/validate/stage 실패 시 이전 generation 유지
```

이번 branch가 새로 닫는 것은 발탄용 계산식과 packet이다.

```text
발탄 source evidence 또는 명시적 project category
-> 발탄 Program opcode와 실제 HLSL 함수
-> 해당 opcode의 Layout ABI receipt
-> occurrence별 Descriptor
-> source 또는 project render-state Adapter
-> .v1.unified occurrence Binding
```

compiled HLSL 함수와 ABI receipt 없이 opcode allowlist나 JSON metadata만 추가하는 것은 금지한다.

## 2. 두 fidelity만 사용한다

### 2.1 `SOURCE_EXACT`

다음 항목이 모두 같은 occurrence에서 닫힐 때만 사용한다.

```text
exact DXBC/HLSL Program semantic
x texture/sampler/CB/DynamicParameter Layout
x child override/parent default Descriptor 값
x carrier/VF/stage semantic
x pass/blend/depth/cull/MRT/scene input Adapter receipt
```

ledger의 `program.fidelity = SOURCE_EXACT` 10행은 계산식 후보일 뿐 자동 runtime exact가 아니다.
scene depth, MRT, WPO 또는 register 의미가 남으면 `SOURCE_EXACT`로 승격하지 않는다.

### 2.2 `PROJECT_TUNED_APPROX`

전체 exact tuple이 닫히지 않아도 source carrier와 drawable resource가 확인된 37 material 행을 화면에
올리기 위한 명시적 프로젝트 식이다.

- 스킬 ID C++ switch를 만들지 않는다.
- V1 `effectAssetId + elementId` stable occurrence에 Binding한다.
- source material path, 선택 texture, 무시한 source input, project scalar/vector와 render-state 근거를
  Descriptor provenance에 남긴다.
- source exact, family representative 또는 translated라고 기록하지 않는다.
- Mesh WPO를 재생하지 못하면 `vertexFidelity = PROJECT_TUNED_STATIC`으로 기록한다.
- scene depth/MRT/distortion을 생략하면 생략한 capability를 approximation delta로 기록한다.

두 상태의 합은 항상 다음과 같아야 한다.

```text
SOURCE_EXACT + PROJECT_TUNED_APPROX = 37 material rows
unbound material row = 0
```

최종 materialization audit에서 whole-tuple `SOURCE_EXACT`는 0행, `PROJECT_TUNED_APPROX`는 37행으로
확정됐다. source Program 단독 exact 색인은 runtime exact admission으로 승격하지 않는다.

## 3. 고정 대상과 카테고리

### 3.1 패턴 분모

| Product pattern | stage | 행 | 현재 자산 |
|---|---|---:|---|
| `VALTAN_WHIRLWIND` | `SPIN` | 9 | `effect.valtan.pattern.420633.active` |
| `VALTAN_WHIRLWIND` | `RECOVERY` | 3 | `effect.valtan.carrier-v1.attack.whirlwind.recovery.clip-01` |
| `VALTAN_MAGIC_CHOICE` | `INNER` | 4 | `effect.valtan.carrier-v1.attack.magic-choice.inner.clip-01` |
| `VALTAN_MAGIC_CHOICE` | `OUTER` | 2 | `effect.valtan.carrier-v1.attack.magic-choice.outer.clip-01` |
| `VALTAN_MAGIC_CHOICE` | `RECOVERY` | 13 | `effect.valtan.carrier-v1.attack.magic-choice.recovery.clip-01` |
| `VALTAN_ARMOR_BREAK_OPENING` | `WALL_CHARGE` | 10 | `effect.valtan.carrier-v1.mechanic.armor-break-opening.charge.clip-01` |
| 합계 |  | 41 | 여섯 V0 자산 |

`VALTAN_DASH_CHARGE`는 현재 Product cue/catalog/effect occurrence가 없어 0행이다. 이번 실험은 Wall Charge를
돌진 control로 사용하며 Dash Charge를 구현·검증했다고 주장하지 않는다.

### 3.2 carrier 분모

| 경계 | 행 | V1 처리 |
|---|---:|---|
| SpriteParticle | 25 | exact가 닫히면 exact, 아니면 Sprite base-coverage approximation Binding |
| MeshParticle | 12 | exact가 닫히면 exact, 아니면 Mesh base-coverage approximation Binding |
| Trail/Ribbon | 3 | V0 presentation을 동일 element/timing으로 carry, material registry Binding 없음 |
| Light | 1 | V0 presentation을 동일 element/timing으로 carry, material registry Binding 없음 |
| LocalDecal | 0 | 이번 세 패턴의 검증 대상이 아니며 D14/Decal 완료를 주장하지 않음 |
| 합계 | 41 | material 37 + presentation boundary 4 |

Ribbon과 Light를 V1 자산에서 삭제하지 않는다. 또한 Sprite나 Mesh로 바꾸지 않는다. 이 네 행은
`PRESENTATION_CARRIED_V0`로 표시하고 현재 carrier 경로를 그대로 소비한다.

### 3.3 사람이 관리하는 project Program category

실제 runtime category는 carrier별 식을 복제하지 않고, 같은 one-texture 식을 SRV color-space 해석에 따라
opcode 두 개로 나눈다.

```text
opcode 1001 = PROJECT_TUNED_BASE_COVERAGE_SRGB       17행
opcode 1002 = PROJECT_TUNED_BASE_COVERAGE_LINEAR     20행

radiance = selectedTexture.rgb * carrier/global color * emissiveIntensity
coverage = selector(selectedTexture) * carrier/global alpha
RT1 distortion = deterministic zero
```

37행의 selected texture는 DDS 채널을 직접 감사해 `base` 23행과 `mask` 14행으로 정했다. coverage scalar
`coverage-channel-selector.0`은 `0 = alpha`, `1 = red`, `2 = RGB luminance`인 ABI이며, 실제 분포는 alpha
9행과 RGB luminance 28행이다. selector가 정수가 아니거나 0~2 범위를 벗어나면 shader가 fail-closed한다.

Sprite는 carrier가 계산한 transformed current/next UV와 SubUV blend를 그대로 받아 두 sample을 보간한다.
Mesh는 carrier-transformed UV를 current/next 동일값과 blend 0으로 전달한다. 따라서 V1 식이 raw local UV로
되돌아가 기존 atlas/SubUV transform을 잃지 않는다. 단, source material의 별도 UV distortion/pan/WPO를
복원한 것은 아니다.

alpha/additive와 one/two-sided depth policy는 Program이 아니라 compiled Adapter variant다. masked Adapter나
두 번째 mask lane을 이번 slice에서 발명하지 않는다. 사용하지 않은 source texture와 식은 Descriptor
provenance에 approximation delta로 남고, 37행 모두 `PROJECT_TUNED_APPROX`다.

## 4. V0와 병렬인 여섯 V1 자산

저장소의 established suffix 순서에 맞춰 stable ID는 `<v0-id>.v1.unified`, 파일은
`<v0-id>.v1.unified.effect.json`으로 만든다.

| 보존할 V0 ID | 새 V1 ID |
|---|---|
| `effect.valtan.pattern.420633.active` | `effect.valtan.pattern.420633.active.v1.unified` |
| `effect.valtan.carrier-v1.attack.whirlwind.recovery.clip-01` | `effect.valtan.carrier-v1.attack.whirlwind.recovery.clip-01.v1.unified` |
| `effect.valtan.carrier-v1.attack.magic-choice.inner.clip-01` | `effect.valtan.carrier-v1.attack.magic-choice.inner.clip-01.v1.unified` |
| `effect.valtan.carrier-v1.attack.magic-choice.outer.clip-01` | `effect.valtan.carrier-v1.attack.magic-choice.outer.clip-01.v1.unified` |
| `effect.valtan.carrier-v1.attack.magic-choice.recovery.clip-01` | `effect.valtan.carrier-v1.attack.magic-choice.recovery.clip-01.v1.unified` |
| `effect.valtan.carrier-v1.mechanic.armor-break-opening.charge.clip-01` | `effect.valtan.carrier-v1.mechanic.armor-break-opening.charge.clip-01.v1.unified` |

V1 생성 불변식:

- V0 여섯 파일의 byte hash를 builder input으로 봉인하고 수정하지 않는다.
- V1은 V0 element를 같은 순서와 timing/transform/attachment/sourceRecipe로 1:1 복제한다.
- V1 `effectAssetId`만 새 stable ID로 바꾸고 material 37행에 typed execution과 Binding을 추가한다.
- 같은 `elementId`를 유지하되 Binding target identity는 새 V1 `effectAssetId + elementId`다.
- Ribbon 3, Light 1에는 material execution을 추가하지 않는다.
- V1 자산은 여섯 catalog row로 별도 등록하고 V0 catalog row를 제거·교체하지 않는다.

## 5. 데이터와 runtime 변경 단위

### G1. 669 ledger에서 41행 audition contract 생성

새 contract는 669 ledger를 복제하지 않고 다음 slice와 실행 판정만 소유한다.

```text
Data/Effects/Contracts/valtan-three-pattern-v1-materialization.receipt.json
Tools/EffectPipeline/materialize_valtan_three_pattern_v1.py
Tools/EffectPipeline/test_materialize_valtan_three_pattern_v1.py
```

각 행은 원본 occurrence, V1 target identity, pattern/stage/cue, carrier, source Program evidence,
runtime fidelity, project category, selected/ignored input, Adapter, Binding ID, presentation carry 상태와
manual review를 가진다.

고정 검산:

```text
rows = 41
V1 assets = 6
Sprite = 25
Mesh = 12
Ribbon = 3
Light = 1
materialBindings = 37
presentationCarries = 4
SOURCE_EXACT + PROJECT_TUNED_APPROX = 37
VALTAN_DASH_CHARGE rows = 0
```

### G2. 병렬 `.v1.unified` 자산과 catalog 생성

builder가 V0 여섯 문서를 read-only input으로 읽고 V1 여섯 문서를 stage한 뒤 전부 검증된 경우에만
한 generation으로 commit한다. 중간 한 문서라도 element join 또는 resource validation에 실패하면 여섯
파일과 catalog를 부분 publish하지 않는다.

변경 대상:

```text
Data/Effects/Authored/<six-v0-ids>.v1.unified.effect.json
Data/Effects/EffectCatalog.json
```

V0와 V1의 Composition parity는 element count, order, start/life, transform, attachment와 sourceRecipe hash로
검사한다. material execution과 effectAssetId 외 delta가 생기면 builder가 거부한다.

### G3. 발탄 exact/approx Program과 ABI packet

기존 단일 registry를 확장하며 두 번째 runtime registry를 만들지 않는다.

```text
Data/Effects/MaterialPrograms/Fragments/valtan-three-pattern-v1.material-program-fragment.v1.json
Client/Bin/ShaderFiles/Shader_EffectUe3MaterialFamilies.hlsli
Client/Bin/ShaderFiles/Shader_VtxEffectParticle.hlsl
Client/Bin/ShaderFiles/Shader_VtxEffectMeshPreview.hlsl
Client/Public/Effect_MaterialProgramRegistry.h
Client/Private/Effect_MaterialProgramRegistry.cpp
Tools/EffectPipeline/build_effect_material_program_registry.py
Tools/EffectPipeline/Schemas/lostark.effect-material-program-registry.schema.json
Tools/EffectPipeline/test_build_effect_material_program_registry.py
```

compiled HLSL에는 opcode `1001/1002`가 공유하는 project Program 함수를 추가한다. 두 Layout은 `t0/s5`,
`base_coverage`, `RGBA`, scalar 1개(`coverage-channel-selector.0`)와 고정 dynamic/static/render mask를 Python,
schema, C++ compiled ABI receipt에서 같은 값으로 연다. 광고한 opcode와 실제 dispatch의 교집합만 registry가
admit한다.

V1 material element 37개에는 generated inline mirror를 materialize하고 fragment Binding과 float32 bit-exact로
비교한다. `REGISTRY_AUTHORITATIVE`로 source packet 증거를 건너뛰지 않는다.

### G4. Sprite/Mesh actual draw와 pipeline receipt

Sprite 25행과 Mesh 12행을 기존 renderer의 RuntimeMaterialV2 draw에 전달한다. 별도 Valtan renderer나
스킬 switch를 추가하지 않는다.

Adapter receipt가 확인할 항목:

- carrier와 vertex layout
- shader opcode와 Layout ABI
- texture/sampler identity와 ordered lane
- actual `Begin(pass)` 뒤 blend/depth/rasterizer state
- active MRT와 deterministic zero distortion
- Mesh mirrored determinant의 one-sided pass policy
- one/two-sided Mesh의 project alpha state와 positive determinant actual draw
- issued draw와 world/snapshot identity

source state가 확인된 행은 그 state를 우선한다. source state가 미확정인 행은 project-selected state를
Descriptor에 명시하고 exact로 승격하지 않는다.

### G5. V0 cue를 보존한 여섯 V1 audition alias

`Valtan.patterneffectcues.json`의 cue ID, target, timing, transform, attachment는 한 글자도 바꾸지 않는다.
대신 다음 V0 effect asset 여섯 개를 같은 이름의 `.v1.unified` asset으로 매핑하는 별도 sidecar를 둔다.

```text
effect.valtan.pattern.420633.active
effect.valtan.carrier-v1.attack.whirlwind.recovery.clip-01
effect.valtan.carrier-v1.attack.magic-choice.inner.clip-01
effect.valtan.carrier-v1.attack.magic-choice.outer.clip-01
effect.valtan.carrier-v1.attack.magic-choice.recovery.clip-01
effect.valtan.carrier-v1.mechanic.armor-break-opening.charge.clip-01
```

alias 정본은 `Data/Animation/Authored/Valtan/Valtan.patterneffectv1aliases.json`이다. Product의 기본 선택은
계속 V0이고, Debug Valtan panel의 A/B switch가 켜진 경우에만 같은 cue가 V1 target을 선택한다. prewarm은
V0/V1 두 target을 모두 준비한다. validator는 원본 cue hash 불변, alias source의 실제 cue join, target의
`.v1.unified` suffix와 Catalog 존재를 검증한다.

V0 자산은 catalog에 남으므로 Effect Editor에서 V0/V1 ID를 각각 열어 SOLO/Play All A/B할 수 있다.

## 6. 자동 검증 게이트

### 6.1 데이터와 publisher

- 669 ledger check mode와 artifact hash 일치
- 41행 slice 누락·중복 0
- V0 여섯 파일 hash 무변경
- V0/V1 여섯 쌍 element 41행 1:1 parity
- material 37행 각각 Binding 정확히 1개
- Ribbon 3/Light 1 Binding 0, presentation carry 정확히 1개
- all Binding target이 V1 asset이고 V0 asset target은 0
- exact 행은 whole-tuple evidence 전부 존재
- approx 행은 project category, selected input, ignored input, state provenance 존재
- alias mapping 정확히 6개, V0 cue 파일 delta 0, timing/attachment delta 0
- `VALTAN_DASH_CHARGE` Product claim 0
- malformed fragment/V1 asset/alias route는 fail-closed하고 기존 V0 cue 계약 유지

### 6.2 shader와 actual draw

- Python/schema/C++ compiled opcode·Layout 교집합 완전 일치
- fragment packet과 inline mirror float32 bit-exact
- Sprite 25행 모두 bind, SRV/sampler, pass, issued draw 도달
- Mesh 12행 모두 bind, CModel pass, determinant policy, issued draw 도달
- RT1 deterministic zero 또는 명시적 MRT capability 일치
- Ribbon/Light는 기존 presentation 경로에서 disposition 유지
- Binding 0 V0 golden과 기존 Artist F S6/M3/D14 회귀 무변경
- invalid packet, missing SRV, wrong carrier, wrong pass는 해당 V1 occurrence fail-closed

### 6.3 실행 명령

실제 repository script 이름과 parameter는 구현 시 `Get-Help`와 현재 CI 호출자로 다시 확인해 RESULT에
실행한 명령만 기록한다. 최소 게이트는 다음 순서다.

```powershell
py -3 Tools/EffectPipeline/test_build_valtan_effect_v1_horizontal_rt0_application.py
py -3 Tools/EffectPipeline/test_materialize_valtan_three_pattern_v1.py
py -3 Tools/EffectPipeline/test_build_effect_material_program_registry.py

powershell -ExecutionPolicy Bypass -File Tools/EffectPipeline/Publish-Effects.ps1 -Validate
powershell -ExecutionPolicy Bypass -File Tools/EffectPipeline/Publish-Effects.ps1

powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release

git diff --check
```

Effect render focused harness는 Debug와 Release를 모두 build/run하고, Client x64 Debug/Release link까지
통과해야 한다. 실행하지 않은 명령은 PASS로 기록하지 않는다.

## 7. 사용자가 정본 EXE에서 수행할 A/B

에이전트는 Client/UI를 실행·조작하거나 visual PASS를 대신 판정하지 않는다. 두 PR 병합 후
`C:\Users\user\Desktop\LostArk`의 정본 main에서 publish/build가 완료된 다음 사용자에게 다음 경로를
전달한다.

### 7.1 Product V1 패턴

```text
C:\Users\user\Desktop\LostArk\Framework.sln
-> Visual Studio의 Server + Client profile
-> Ctrl+F5
-> Lobby에서 Valtan 진입
-> Debug Valtan panel의 Focused authored pattern
-> Use V1 .effect.unified aliases 체크
-> VALTAN_WHIRLWIND 선택
-> Reset + Play Selected Pattern
-> VALTAN_MAGIC_CHOICE 반복
-> VALTAN_ARMOR_BREAK_OPENING 반복
```

체크가 꺼져 있으면 V0, 켜져 있으면 같은 Product cue가 V1 alias를 사용한다. 각 패턴을 두 모드로 한 번씩
재생한다. `VALTAN_DASH_CHARGE`는 선택·검증 목록에 넣지 않는다.

### 7.2 V0/V1 자산 직접 비교

Effect Editor에서 표의 V0 ID와 `.v1.unified` ID를 각각 열고 같은 순서로 확인한다.

```text
Play All
-> Sprite Family/Solo
-> Mesh Family/Solo
-> Ribbon/Light carry 확인
```

사용자가 기록할 항목:

- Sprite가 사각형 quad로 보이는지, alpha/mask edge가 살아 있는지
- additive glow가 너무 밝거나 alpha처럼 탁한지
- Mesh가 통째 사각 면으로 보이는지, coverage/mask가 맞는지
- Mesh scale, mirrored cull, depth sorting이 무너지지 않았는지
- V0와 spawn timing, attachment, transform, lifetime이 같은지
- Ribbon 3과 Light 1이 사라지거나 이중 재생되지 않는지
- Whirlwind, Donut, Wall Charge의 전체 실루엣과 조작 타이밍이 V0보다 나아졌는지

관찰은 `patternId/stageId/effectAssetId/elementId`로 41행 contract에 연결한다. 사용자의 서면 판정이
있기 전에는 manual review를 `PENDING`에서 바꾸지 않는다.

## 8. 병렬 4캐릭터 세션과 merge 경계

두 세션 모두 registry schema/builder, compiled HLSL dispatch와 C++ ABI receipt를 건드릴 수 있으므로 같은
정본 folder에서 섞어 commit하지 않는다. 채팅 동기화가 없어도 다음 순서를 고정하면 된다.

```text
4캐릭터 V1 branch는 자기 worktree에서 완료 및 PR
-> 4캐릭터 PR을 main에 먼저 병합
-> 이 Valtan branch를 최신 origin/main에 rebase
-> 공유 registry/HLSL/C++ diff를 latest main 계약에 맞춰 재생성
-> Valtan focused + Artist F + 4캐릭터 회귀 전부 재실행
-> Valtan PR 생성·병합
-> 마지막에만 C:\Users\user\Desktop\LostArk 정본 main을 --ff-only pull
-> 정본 folder에서 publish/build
```

이 branch에는 Valtan 여섯 V1 자산, Valtan fragment, 41행 contract, V1 alias sidecar와 그것을 실행하는 데
필요한 generic opcode delta만 둔다. 4캐릭터 stable ID나 4캐릭터 registry fragment를 수정하지 않는다.

`C:\Users\user\Desktop\LostArk` 정본 folder는 두 PR이 main에 병합되기 전까지 pull, branch switch, 파일 복사,
publish/build 대상으로 사용하지 않는다. 기존 dirty 사용자 파일과 겹치는지 merge 전 이름 교집합을 먼저
검사한다.

## 9. rollback

### 9.1 화면 결과가 나쁠 때의 Product rollback

Debug A/B switch를 끄면 즉시 V0 target을 사용한다. V1 여섯 자산과 ledger/fragment는 catalog에 남겨 SOLO
진단과 후속 튜닝에 사용할 수 있다. cue target, timing, Server pattern과 V0 자산은 애초에 수정하지 않았으므로
Product 복귀를 위한 데이터 되돌리기가 없다.

### 9.2 runtime 실패 rollback

V1 registry 또는 cue generation의 parse/validate/stage가 실패하면 새 generation을 commit하지 않고 이전
runtime catalog/registry를 유지한다. 일부 V1 asset만 섞어 publish하지 않는다.

### 9.3 Git rollback

정본 main을 reset하지 않는다. Valtan PR의 검증 단위 commit을 revert하고 정본 folder에서 `git pull --ff-only`
후 재publish/build한다. 사용자 dirty 파일을 checkout/reset으로 되돌리지 않는다.

## 10. 이 실험 뒤의 의사결정

다음 셋 중 하나를 사용자가 선택한다.

1. 세 패턴에서 사각형/coverage/state 문제가 의미 있게 줄고 timing parity가 유지됨
   - 같은 project Program/Layout/Adapter cohort를 669 ledger의 동형 행으로 수평 확대한다.
2. 공용 실행 구조는 동작하지만 occurrence별 색·밝기·mask 튜닝이 많이 필요함
   - V1 stable Binding은 유지하고 Descriptor만 PNG A/B/손튜닝하는 hybrid로 간다.
3. 세 패턴 모두 V0 대비 개선이 작거나 exception 비용이 더 큼
   - V1 alias 사용을 중단하고 발탄 Product는 V0/PNG A/B 손튜닝으로 전환한다.

어느 결과라도 S6/M3/D14를 발탄 exact라고 위장하거나, V1 실패 행을 조용히 V0 fallback으로 섞어 성공처럼
보이지 않는다. 이 41행 실험은 V1 수평 확대의 마지막 경제성 판단 자료다.
