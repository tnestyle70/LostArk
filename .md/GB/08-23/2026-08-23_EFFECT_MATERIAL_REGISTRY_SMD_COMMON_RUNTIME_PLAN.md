# 2026-08-23 Effect Material Registry S/M/D 공용 Runtime 계획

기준 branch: `codex/effect-v1-visual-matrix`

기준 main: `d6b27084`

최종 화면 판정자: 사용자

Valtan 669 전수 적용은 이 공용 PR이 main에 병합된 뒤 별도 branch/PR에서 진행한다.

## 0. 목표

Valtan과 4캐릭터가 함께 소비할 도메인 무관 공용 runtime capability만 먼저 닫는다.

```text
복수 Program
x 복수 Layout/Descriptor
x compiled Sprite/Mesh/Decal Adapter
x 복수 Binding
-> 기존 EFFECT_MATERIAL_EXECUTION_DESC
-> 기존 Effect_DocumentRenderer actual draw
```

이 변경에는 Valtan occurrence, Valtan opcode 23+, Valtan ledger 또는
`REGISTRY_AUTHORITATIVE` Binding을 넣지 않는다. 공용 PR의 모든 Binding은 근거 유무와 관계없이
`INLINE_MIRROR_REQUIRED`만 허용한다.

## 1. 현재 병목

현재 registry JSON schema는 여러 배열을 표현할 수 있지만 compiled runtime은 다음 한 tuple만 허용한다.

```text
Artist F sprite.2b3dc6842507e910
runtimeMaterialV2 opcode 6
SpriteParticle
alpha two-sided depth-read
pass 1
```

다음 하드코딩을 제거해야 한다.

- adapter count 정확히 1
- adapter ID 정확히 Artist F Sprite alpha-two
- Program opcode 정확히 6
- carrier 정확히 SpriteParticle
- renderer stage/draw 검증이 Sprite/opcode6/pass1만 허용
- EffectRenderContractHarness expected binding count가 0 또는 1

## 2. 고정 안전 경계

- JSON은 shader path, vertex layout, pass, MRT, raster/depth/blend state를 작성하지 못한다.
- Adapter ID는 C++ compiled allowlist의 고정 descriptor로만 resolve한다.
- main HLSL에 실제 dispatch가 존재하는 Program/backend/opcode만 허용한다.
- 모든 공용 PR Binding은 `INLINE_MIRROR_REQUIRED`이며 authored execution과 float32 bit-exact여야 한다.
- inline packet이 없거나 disabled이면 registry load 전체를 거부한다.
- Binding target carrier와 Adapter carrier가 다르면 Python publisher와 C++ loader 모두 거부한다.
- Binding 없는 occurrence는 기존 V0 경로를 유지한다.
- parse/validate/stage 중 실패하면 새 catalog/registry generation을 commit하지 않는다.

## 3. compiled capability

사람이 관리하는 carrier capability는 세 개다.

```text
S = SpriteParticle Standard RT0
M = MeshParticle CModel RT0
D = LocalDecal Projector RT0
```

Adapter data variants:

| carrier | state | shader pass |
|---|---|---:|
| S | alpha two-sided depth-read | 1 |
| M | alpha two-sided depth-read | 1 |
| D | alpha one-sided scene-depth projector | 3; localDecal opcode 14만 허용 |

Program allowlist:

```text
SpriteParticle x RuntimeMaterialV2 opcode 6
MeshParticle CModel x RuntimeMaterialV2 opcode 3
LocalDecal Projector x LocalDecal opcode 14
```

위 세 Program/backend/opcode만 compiled Layout ABI receipt와 actual bound draw를 함께 가진다.
opcode 17, 19, 23+와 다른 equation은 이 PR에서 public Program 또는 Adapter admission으로
허용하지 않는다. Mesh masked pass 7 관련 fixed-state 검사가 남더라도 이는
`synthetic fixed-state receipt only`이며 CModel/MRT/registry admission 증거가 아니다.

## 4. inline exact 공용 증명

새 근거를 발명하지 않고 Artist F의 이미 enabled인 typed packet 세 개를 registry로 dual-resolve한다.

| carrier | element | backend/opcode | adapter |
|---|---|---|---|
| S | `sprite.2b3dc6842507e910` | runtimeMaterialV2/6 | alpha two-sided |
| M | `mesh.062366ee9f9655d3` | runtimeMaterialV2/3 | alpha two-sided |
| D | `decal.f3b5c3b63b4a7e34` | localDecal/14 | alpha one-sided projector |

각 inline execution을 Program/Layout/Descriptor로 분해하고 registry materialization 결과와 모든 field 및
float32 bit를 비교한다. 이 세 Binding은 새로운 시각식을 추가하지 않으며 기존 packet의 공용 구조 승격이다.

## 5. 변경 파일

### 데이터와 publisher

- `Data/Effects/MaterialPrograms/effect-material-program-registry.v1.json`
- `Tools/EffectPipeline/Schemas/lostark.effect-material-program-registry.schema.json`
- `Tools/EffectPipeline/build_effect_material_program_registry.py`
- `Tools/EffectPipeline/test_build_effect_material_program_registry.py`
- `Data/Effects/MaterialPrograms/Fragments/artist-f-golden.material-program-fragment.v1.json`

### Client runtime

- `Client/Public/Effect_MaterialProgramRegistry.h`
- `Client/Private/Effect_MaterialProgramRegistry.cpp`
- `Client/Private/Effect_Catalog.cpp`
- `Client/Private/Effect_DocumentRenderer.cpp`

### 실행형 검증

- `Tools/EffectRenderContractHarness/Private/EffectRenderContractHarness.cpp`
- 필요하면 해당 harness runner/result contract

새 C++ 파일은 없다. 새 Git 관리 Data fragment는 `Client.vcxproj`와 `.filters`의
`96.DataFiles/Effects/MaterialPrograms/Fragments`에 등록한다.

## 6. 실제 호출 흐름

```text
EffectCatalog parse
-> registry Program/Layout/Descriptor/Adapter/Binding validate
-> Binding packet materialize
-> authored inline execution bit-exact compare
-> immutable registry generation stage
-> Effect_DocumentRenderer prepare
-> binding carrier/adapter/execution snapshot compare
-> existing Stage_AuthoredMaterialExecution
-> existing Bind_Common 또는 Bind_MaterialInputs
-> existing Render_Particles / Render_Mesh / Render_Decal
-> compiled pass/state/MRT actual pipeline validation
-> issued draw
```

새 renderer 또는 두 번째 material execution path를 만들지 않는다.

## 7. 검증

### Python/publisher

- 3 Program / 3 Layout / 3 Descriptor / 3 Adapter / 3 Binding
- S/M/D target identity와 carrier 일치
- inline packet bit-exact
- duplicate/dangling/unknown adapter 거부
- carrier mismatch 거부
- backend/opcode/Adapter mismatch 거부
- disabled/missing inline packet 거부
- invalid mask/register/sampler/count/non-finite 거부

### C++/WARP harness

- registry 3 Binding resolve
- S/M/D 각 prepared snapshot bit-exact
- S/M/D 각 실제 shader pass 선택
- texture/sampler bind
- Sprite instance draw, Mesh CModel draw, Decal projector draw
- active MRT와 compiled raster/depth/blend state 검증
- Sprite golden은 기존 bound actual draw와 packet/snapshot receipt를 유지
- 동일 Artist F Mesh/Decal occurrence의 Binding 0/1 isolated actual draw에서 carrier, pass,
  draw count, world hash와 packet/snapshot 동등성 유지
- carrier/profile-invalid catalog reload는 이전 generation 유지

### build

- EffectPipeline focused tests
- EffectRenderContractHarness x64 Debug/Release와 실행
- Client x64 Debug/Release
- `Publish-Effects.ps1` validate/check
- `git diff --check`, JSON/XML parse

## 8. 완료와 미완료

공용 PR 완료:

- 도메인 무관 S/M/D registry runtime spine
- Artist F inline exact 3행으로 actual draw 증명
- 다른 도메인이 같은 tuple과 증명된 packet을 추가할 수 있는 구조

공용 PR에서 의도적으로 미완료:

- Valtan 669 ledger와 packet 생성
- 4캐릭터 전수 ledger와 packet 생성
- inline이 없는 occurrence의 source packet oracle
- Valtan op23+ HLSL
- masked Mesh opcode 19 actual bound CModel draw와 registry admission
- Ribbon, Glass/MRT, WPO vertex, Presentation
- 사용자 Product visual approval
