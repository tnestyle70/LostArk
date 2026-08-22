# Effect V1 수평 Sprite canary 구현 계획

## 0. 문서 목적

이 계획은 스킬 하나를 다시 손으로 꾸미는 작업이 아니다. 현재 화면상 문제가 없는 도화가 F
`effect.artist.skill.31470.unified`를 golden control로 사용해 다음 공용 수직 tuple을 처음으로 끝까지
닫는다.

```text
Program
× Layout
× occurrence Descriptor
× compiled Carrier Adapter
= 기존 inline Material.Execution과 동등한 실행 packet
```

첫 종료점은 SpriteParticle 한 element가 registry와 기존 inline 경로에서 동시에 resolve되고, 두 결과가
자동으로 완전 동등하며, 사용자가 기존 도화가 F와 비교해 화면 변화가 없음을 확인할 수 있는 상태다.
이 단계에서는 다른 스킬의 외형을 개선하거나 Product cohort를 일괄 승격하지 않는다.

## 1. 시작 기준과 선행 PR

구현 branch는 다음 두 증거 PR을 각각 최신 `main`에 병합한 뒤 새로 만든다.

1. child-parent resolution evidence
   - source child와 effective parent를 fail-closed로 연결한다.
   - 420 authored documents, 7,572 elements, 809 orphan child, 621 source packages를 고정 분모로 검증한다.
   - duplicate/ambiguous join, 입력 identity, row hash, atomic write와 실패 시 기존 receipt 보존을 검증한다.
2. G00 family shader inventory와 문서
   - Program exact, named mapping, runtime ABI closure, Product admission을 서로 다른 축으로 기록한다.
   - `DXBC 있음`이나 `named ABI mapping 있음`을 runtime 연결 또는 Product 완료로 과장하지 않는다.

각 PR은 한 계약만 담아 병합하고 다음 작업은 직전 merge 뒤의 최신 `origin/main`에서 새 branch로 시작한다.

## 2. 이번 구현의 고정 범위

### G01. Program/Layout/Descriptor/Adapter registry 최소 구현

authoring 정본은 `Data/Effects/MaterialPrograms/effect-material-program-registry.v1.json`이다. publisher가
검증한 immutable registry를 `Client/Bin/DataFiles/Effect/EffectCatalog.runtime.json`에 포함한다. runtime
catalog는 v4를 발행하고 Client loader는 기존 v2/v3를 빈 registry로 계속 읽을 수 있어야 한다.

registry의 네 책임은 다음처럼 분리한다.

| 영역 | 소유 내용 | 소유하지 않는 내용 |
|---|---|---|
| Program | pixel equation identity, backend/opcode | texture asset, occurrence 값, draw state |
| Layout | t/s/CB/dynamic lane 의미와 개수 | 실제 texture와 scalar 값 |
| Descriptor | occurrence별 texture, scalar/vector, sampler/channel | 실행 가능한 shader/pass/VF 선택 |
| Binding | effect asset ID + stable element ID → Program/Layout/Descriptor/Adapter 네 ID | 이름 추측이나 vector index |
| compiled Adapter | carrier, VF/input layout, shader, pass, MRT, blend/depth/raster, draw dispatch | JSON이 지정하는 임의 실행 코드 |

JSON의 `adapterId`는 C++에 컴파일된 adapter allowlist만 선택한다. JSON이 shader path, pass, VF, MRT 또는
draw 함수를 임의로 구성하게 만들지 않는다. registry는 새 전역 Manager가 아니라
`CEffectCatalog::Load`가 parse → validate → stage → commit한 immutable catalog generation에 속한다.

최소 변경 파일은 다음과 같다.

- `Data/Effects/MaterialPrograms/effect-material-program-registry.v1.json`
- `Tools/EffectPipeline/Schemas/lostark.effect-material-program-registry.schema.json`
- registry builder/validator와 focused test
- `Tools/EffectPipeline/Publish-Effects.ps1`
- `Client/Bin/DataFiles/Effect/EffectCatalog.runtime.json`
- `Client/Public/Effect_MaterialProgramRegistry.h`
- `Client/Private/Effect_MaterialProgramRegistry.cpp`
- `Client/Public/Effect_Catalog.h`
- `Client/Private/Effect_Catalog.cpp`
- `Client/Public/Effect_DocumentRenderer.h`
- `Client/Private/Effect_DocumentRenderer.cpp`
- `Client/Private/Effect_PresentationService.cpp`
- `Client/Default/Client.vcxproj`
- `Client/Default/Client.vcxproj.filters`

`Effect_MaterialProgramRegistry`는 immutable ID lookup과 staged validation만 소유한다. texture 생성,
shader 생성, draw, Product admission, skill timing은 소유하지 않는다. unknown/duplicate ID, dangling
reference, unsupported adapter, 수량 불일치와 invalid register는 load 실패로 처리하고 이전 catalog
generation을 유지한다.

registry generation과 document가 찢어지지 않게 같은 immutable generation handle과 catalog revision을
`Prepare_TargetSet → EFFECT_RENDER_PREWARM_TARGET → Build_PreparedDocument → PREPARED_DOCUMENT`까지
전달한다. `Stage_ElementResource`가 준비 중간에 최신 전역 catalog를 다시 조회하면 안 된다.

### G01 종료 검증: registry-only draw-path 무변경

첫 registry PR에는 Program/Layout/Descriptor 정의가 있어도 element Binding을 0개로 둔다. 따라서 기존
모든 element는 현재 inline `Material.Execution` 경로만 사용한다.

자동 종료 증거는 다음과 같다.

- v2/v3 catalog → empty registry compatibility
- v4 empty-binding catalog의 parse/validate/stage/commit
- invalid schema/version/ID/duplicate/dangling adapter 실패와 이전 generation 보존
- publisher 전후 effect entry 및 authored document identity 동일
- binding count 0, registry-resolved element count 0
- 기존 prepared execution snapshot corpus byte/field 동등
- 기존 fixed-step execution harness의 Artist F Full35와 legacy Lance BA1 sentinel에서
  `attempted/submitted/suppressed/failed/committed` 카운터 동등
- Debug/Release Client build
- `Publish-Effects.ps1 -Mode Validate`, 관련 focused tests, `git diff --check`

이 검증은 구조상 기존 draw path가 바뀌지 않았다는 자동 증거다. “기존 화면 무변경”의 최종 판정은
binding 0 build에서 사용자가 Artist F와 Lance BA1을 직접 실행해 관찰한 뒤에만 닫는다. 사용자 관찰
전에는 G01을 `AUTOMATED_NO_DRAW_PATH_CHANGE`로만 기록하고 visual PASS를 선언하지 않는다.

### G02. 도화가 F Sprite canary dual-resolve

첫 canary는 다음 stable occurrence다.

```text
effectAssetId = effect.artist.skill.31470.unified
elementId     = sprite.2b3dc6842507e910
carrier       = SpriteParticle
program       = RuntimeMaterialV2 opcode 6
pass          = 1
output        = SceneColor RT0 + deterministic zero Distortion RT1
state         = alpha / two-sided / depth-read
```

현재 inline execution의 핵심 packet은 그대로 golden mirror로 보존한다.

```text
lane 0 = Effect/Artist/Textures/fx_d_noise_003.dds, t0/s5, SRGB, linear-wrap
lane 1 = Effect/Artist/Textures/fx_c_noise_002.dds, t1/s6, SRGB, linear-wrap
scalars = [30, 1.5, 0.100000001]
dynamicConsumedMask = 15
```

첫 compiled adapter ID는 다음 의미를 고정한다.

```text
effect.adapter.sprite-particle.scene-color-rt0.zero-distortion-rt1.alpha-two-sided.v1
```

direct-authored v13 문서의 `Renderer.eType`이 `END`일 수 있으므로 carrier 판정은 그 값 하나를
신뢰하지 않는다. element `kind=particle`, `sourceRecipe.rendererShape=sprite`, model 부재와 실제 준비
경로를 함께 검증한다.

dual-resolve 호출 흐름은 다음과 같다.

```text
EffectCatalog Binding lookup
→ Program + Layout + Descriptor + compiled Adapter resolve
→ EFFECT_MATERIAL_EXECUTION_RESOURCE materialize
→ 기존 inline Material.Execution과 exact compare
→ Stage_AuthoredMaterialExecution
→ GPU resource/input stage
→ Build_MaterialExecutionSnapshot
→ registry materialized snapshot과 exact compare
→ prepared resource commit
```

여기서 exact는 epsilon 비교가 아니다. execution의 모든 field, ordered texture lane/scalar/vector row,
sampler descriptor와 float의 `bit_cast<uint32_t>` 값까지 같아야 한다. prepared element에는 resolved
adapter ID와 immutable registry generation/revision을 저장하고, `Render_Particles`의 실제 pass/MRT/state
선택 직전에도 그 adapter allowlist와 일치함을 확인한다. metadata만 resolve하고 legacy dispatch가 다른
선택을 하는 상태는 성공이 아니다.

Binding이 있는 canary에서 registry 누락, unknown adapter, packet mismatch 또는 snapshot mismatch가 나면
fail-closed한다. inline 경로로 조용히 되돌아가 성공처럼 보이게 하지 않는다. Binding이 없는 나머지
element는 기존 inline 경로를 그대로 사용한다.

연결 기준 함수는 다음이다.

- `CEffectCatalog::Load`, `Find`, `Clear`: immutable registry generation load/lookup/lifetime
- `CEffectPresentationService::Prepare_TargetSet`와 incremental preparation: canary가 실제 prewarm 경로를 소비
- `CEffectDocumentRenderer::Stage_ElementResource`: stable occurrence binding resolve
- `Stage_AuthoredMaterialExecution`: materialized execution stage
- `Build_MaterialExecutionSnapshot`: 최종 실행 snapshot 동등성
- `Build_PreparedDocument`: 실패 시 전체 staged preparation rollback
- `Render_Particles`: 새 전용 draw가 아니라 검증된 기존 SpriteParticle draw 소비

이 PR은 새 HLSL이나 새 전용 도화가 F shader를 만들지 않는다. 이미 사용 중인 opcode 6 program과
SpriteParticle draw를 registry ID로 재표현한다.

### G02 자동 종료 검증

- registry materialized execution과 inline execution의 모든 field exact equality
- texture asset/register/sampler/color-space 및 scalar/vector/dynamic mask equality
- compiled adapter의 carrier/VF/pass/MRT/render-state allowlist equality
- `Build_MaterialExecutionSnapshot` equality
- same document의 두 번째 동형 element `sprite.c65181324417a1a8` shadow resolve로 공용성 확인
- missing/duplicate/dangling/mismatched canary fixture fail-close와 이전 prepared document 보존
- Product cue와 prewarm 모두 같은 committed catalog generation 소비
- Artist F Full35와 legacy Lance BA1 fixed-step sentinel의
  `attempted/submitted/suppressed/failed/committed`가 binding 0 baseline과 동등
- Debug/Release Client build와 effect publisher Validate
- 관련 C++ execution/snapshot harness, Python registry tests, `git diff --check`

두 번째 element는 첫 canary와 같은 Program/Layout/Adapter를 재사용할 수 있음을 증명하되, 첫 사용자
A/B 전에 Product cohort로 일괄 전환하지 않는다.

## 3. 사용자 도화가 F A/B

자동 검증이 모두 통과한 빌드만 사용자에게 넘긴다. A는 PR D의 binding 0 merge commit과 그 commit에서
빌드한 Client.exe identity이고, B는 PR E canary commit과 그 commit에서 빌드한 Client.exe identity다.
RESULT에는 두 commit SHA, executable timestamp/SHA-256, runtime catalog SHA-256을 기록한다. 사용자는
Visual Studio `Framework.sln`의 `Server + Client` profile에서 두 identity를 직접 실행하고 비교한다.
에이전트는 Client/UI를 조작하거나 화면을 캡처하지 않는다.

사용자가 확인할 항목은 다음 네 가지다.

1. F 입력부터 첫 visible frame까지의 지연이 변하지 않았는가.
2. 휘두름과 hit sprite의 위치, 방향, 크기, lifetime이 변하지 않았는가.
3. 색, coverage, noise 움직임과 alpha가 기존 golden control과 같은가.
4. 반복 사용, 취소, level 재진입 뒤 missing draw나 잔상이 없는가.

사용자 판정은 `USER_APPROVED`, `USER_REJECTED` 또는 관찰 내용으로만 기록한다. 자동 snapshot equality가
사용자 visual PASS를 대신하지 않는다. 승인 뒤에만 같은 tuple cohort의 Descriptor 확장 PR로 넘어간다.

## 4. 이번 slice에서 수정하지 않는 구조적 회귀 backlog

다음 세 문제는 중요하지만 Sprite registry closure와 원인·검증 분모가 다르다. 이번 PR에 코드 변경을
섞지 않고 재현 가능한 후속 수직 슬라이스로 고정한다.

### DimensionMaster BA cue/control parity

- 목표: DimensionMaster BA1/2/3/4의 입력 cue, action edge, combo window, playback delay를 Artist
  BA1/2/3/4와 같은 구조로 만든다.
- asset mapping: DimensionMaster BA1/2/4는 BA1 unified를, BA3은 BA3 unified를 재생한다.
- 종료 증거: 같은 입력 timeline에서 Artist와 DimensionMaster의 action start, cue spawn, next-input
  acceptance와 recovery 경계가 수치로 동등하고 사용자가 조작감을 승인한다.

### DimensionMaster A casting sample regression

- recovery를 제외한 cast end가 `0.8`이면 sample은 `0.0, 0.2, 0.4, 0.6`만 재생한다.
- `0.8` 또는 recovery frame을 cast sample로 다시 포함하지 않는다.
- 종료 증거: Server action clock, animation cursor와 presentation sample index의 focused timeline test.

### Artist R decal regression

- LocalDecal projector의 carrier/VF/pass/depth intersection과 descriptor를 G04에서 별도 복원한다.
- 단순히 바닥 sprite를 보이게 한 상태를 decal V1 완료로 기록하지 않는다.

이전의 Artist A/S, DimensionMaster F/W, LanceMaster D/F 결과도 현재는 “V0 수준의 visible effect가
생성됨”까지만 인정한다. Program/Layout/Descriptor/Adapter, Product admission과 사용자 semantic A/B가
닫히지 않았으므로 이번 Artist F 증명을 근거로 자동 V1 승격하지 않는다.

## 5. PR 순서와 중단 조건

```text
PR A  계획과 deferred regression backlog
PR B  child-parent evidence
PR C  G00 inventory/docs
PR D  registry 최소 구현, Binding 0
PR E  Artist F stable canary dual-resolve + 자동 동등성
USER  Artist F A/B
```

각 PR은 직전 PR이 병합된 최신 main에서 새 branch로 시작한다. PR D의 no-binding snapshot이 기존과
다르거나 PR E의 exact comparison이 실패하면 다음 단계로 진행하지 않는다. 사용자 A/B가 거절되면
같은 stable element와 mismatch receipt로 돌아가 수정하고, 다른 캐릭터나 Valtan cohort를 열지 않는다.

## 6. 이 slice가 증명하는 것과 증명하지 않는 것

성공하면 “도화가 F를 특별 취급해서 보이게 했다”가 아니라 다음 확장 규칙이 처음으로 실행 증거를
갖는다.

```text
equation 같음 + ABI 같음 + carrier/VF/pass 같음
→ Program/Layout/Adapter 재사용
→ occurrence별 Descriptor와 Binding만 추가
```

아직 증명하지 않는 것은 Mesh RT0, LocalDecal, Trail/Ribbon, Glass/MRT, WPO와 별도 presentation이다.
또한 registry 구조의 성공은 다른 occurrence의 source fidelity, Composition, Product admission 또는
사용자 승인을 자동 보장하지 않는다. 다음 확장은 Artist F의 두 번째 동형 element, 그 뒤 다른
캐릭터 또는 Valtan의 동일 Sprite tuple 한 occurrence 순으로 진행한다.
