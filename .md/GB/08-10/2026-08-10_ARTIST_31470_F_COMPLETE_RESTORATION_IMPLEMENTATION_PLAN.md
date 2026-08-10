# 2026-08-10 Artist 31470 F Complete Restoration Implementation Plan

기준일: 2026-08-10

최종 통합 시작 commit: `38ebe7cf7dceb5054bde93812907173cc0f98c67`

참고 materializer checkpoint: `4ffe1102ed9cf3e21f669da292fac1f143e18d8f` — BLOCK, 재사용하지 않고 final Source/Material schema 위에서 다시 작성

계획 갱신 branch: `codex/artist-f-restoration-final-plan`

최종 통합 branch: `codex/artist-f-reconstructed-integration-v1`

최종 통합 시작 commit: `38ebe7cf7dceb5054bde93812907173cc0f98c67`

2026-08-10 사용자 승인: source-era 동일성을 주장하지 않는 `RECONSTRUCTED_APPROVED_V1` 실행 경로를
진행한다. 이 승인은 Source 29행과 Material 255행의 historical evidence blocker를 삭제하거나
`SOURCE_EXACT`로 승격하는 허가가 아니다. 각 행을 stable policy/capability ID, explicit typed value,
fixed-input/output oracle, finite tolerance와 implementation identity에 결합한 뒤 execution blocker만
별도로 해소하는 허가다. R2 통합과 schema/policy 구현은 `IN_PROGRESS`이며, policy receipt와 corrected
typed materializer의 독립 PASS 전에는 R3 Playback과 Product admission을 열지 않는다.

문서 역할: 이 파일은 전체 범위와 G 순서, worktree 소유권, admission predicate를 소유하는 최종
구현 계획서다. 각 mutation lane은 기존 Source/Geometry/Material/Compiler PLAN을 갱신하거나 해당 G의
전용 구현 PLAN을 먼저 만들고, public H/CPP의 세부 선언·전체 코드는 그 lane 문서에서만 소유한다.
같은 코드를 이 master 문서에 복제하지 않는다.

## 이번 재개판의 핵심 판단

고정된 세 개의 구현 세션을 계속 동시에 돌리는 방식은 채택하지 않는다. 이 작업은 Source evidence,
Material evidence, Geometry resource처럼 파일과 판정 근거가 분리되는 구간에서는 병렬화 효과가 크지만,
Playback, Renderer, Catalog, Presentation, Effect Tool처럼 같은 immutable identity와 같은 C++ 파일을
연속으로 연결하는 구간에서는 병렬 구현이 merge와 재검증 비용을 더 크게 만든다.

따라서 사용자에게 보이는 작업은 최대 세 세션으로 유지하되, dependency wave마다 실제 활성 세션 수와
역할을 바꾼다.

| 세션 | 기본 역할 | mutation 경계 |
|---|---|---|
| 세션 1 | Source/evidence 또는 interface 동결 뒤 분리 가능한 family/corpus lane | 한 wave에서 배정받은 evidence/data/family 파일만 수정 |
| 세션 2 | Pipeline Implementation/Integration | Material corrective, typed runtime, shared C++ foundation, 결합과 전체 build 소유 |
| 세션 3 | Independent Review | frozen commit/tree read-only 재현, PASS/BLOCK만 판정, 파일 수정 금지 |

운영 원칙은 다음과 같다.

1. evidence처럼 독립된 입력은 세션 1과 2가 병렬로 닫는다.
2. 같은 Runtime Authority, Playback, Renderer, Catalog 파일을 잇는 구간은 세션 2 한 명만 쓴다.
3. 세션 3은 live dirty diff를 따라가지 않고 frozen SHA가 생길 때만 활성화한다.
4. 공통 renderer interface가 frozen된 뒤에만 세션 1과 2가 서로 다른 family 파일을 병렬 구현한다.
5. Artist F 35/35 뒤에는 세션 1이 4-class corpus, 세션 2가 Valtan corpus를 병렬 적용하고,
   공통 compiler/renderer 변경이 필요해지면 확장을 멈추고 통합 gate로 되돌아온다.

즉 단일 세션만으로도 구현은 가능하지만 이 프로젝트에서는 권장하지 않는다. 실제로 자체 test가 PASS한
Source `9b046d6`과 Material `e54a5a2`에서 independent review가 source-era 승격, coordinated reseal,
operand ownership과 NaN oracle 문제를 재현했다. 반대로 세 세션을 항상 동시에 구현에 투입하면 공유
C++ ownership과 hash regeneration이 병목이 된다. 최적 구조는 **한 명의 shared-runtime writer,
독립 가능한 한 명의 specialist, frozen checkpoint reviewer 한 명**을 wave별로 켜고 끄는 방식이다.

## 목표와 종료 증거

도화가 `31470 / 필법 : 한획긋기 / F`의 7 cue와 35 active occurrence를 하나의
Source Contract에서 immutable typed Cascade IR, geometry와 material binding, 여섯 renderer,
Effect Tool의 동일 prepared revision, Product Catalog까지 연결한다.

이 작업의 종료는 문서 생성이나 scaffold build가 아니다. 다음 조건을 모두 만족해야 한다.

```text
Source denominator        7 cue / 35 occurrence / 399 ordered module reference
Source property           1,434 top-level tagged property / 1,572 primitive leaf / 629 distribution
Renderer denominator      Mesh 13 / Sprite 16 / Decal 3 / Ribbon 1 / Light 1 / ScreenPost 1
Local reference           15 definition / 17 occurrence / PointLight 1 occurrence
Geometry                  7 WModel carrier / 13 Mesh occurrence
Material                  27 unique recipe / 34 rendered material occurrence / builtin Light 1 separate
Runtime authority         immutable compiled IR only
Silent fallback           0
Unknown evaluator run     0
Missing material fallback 0
Double geometry scale     0
Stale generated hash      0
Failed transaction leak   0
Product admission         35/35, atomic cue publish
```

이미지 캡처, 스크린샷, 육안 비교, 이미지 기반 자동 판정은 종료 증거로 사용하지 않는다.
fixed seed, fixed timestep, 명시적인 ActionCue 입력과 sample time, numeric tolerance를 가진
numeric/structural oracle와 컴파일·링크·실행 오류 검증만 사용한다.

## 현재 실제 기준점

R0 matrix와 접근 가능한 범위의 R1 acquisition은 끝났지만 source-era actual-output/state provider를
확보하지 못했다. 따라서 final typed materializer, Playback, 여섯 renderer는 시작하지 않았고 Product
admission은 `0/35`다. `4ffe1102`는 구현 출발점이 아니라 폐기한 BLOCK checkpoint다.

| 영역 | frozen checkpoint | 독립 판정과 현재 사용 정책 |
|---|---|---|
| Geometry evidence/resource | `0aca792819fdda3f541bb7cec7451c5ed93c6467` | PASS. 7 WModel v1.1 physical deploy와 7/7 decode를 재사용하되 runtime preScale consumer는 아직 0/7 |
| Material evidence/runtime corrective | `cde8f3bddea2f9415f682b387d2705fd25794075` | `627ddc76`, `ab76b7ec`, `d39097c3`의 admission/semantic validation gap을 supersede. evidence integrity PASS, source value는 render 0/89·static 23/94·sampler 0/72, execution readiness 0/255 BLOCK. exact sampler 0, rejected legacy 4, Product false |
| Generic compiler | `c4b00f14b32d27604ac677e9a9ea81b01ecaa551` | non-executable inspection core 범위 PASS, Product false |
| Runtime Authority foundation | `38ebe7cf7dceb5054bde93812907173cc0f98c67` | PASS. immutable authority foundation만 승인, typed executor는 미연결 |
| Source execution corrective | `c927e397811d4e5718efd27b187eb59775023685` | evidence integrity PASS, module 370 ready/29 blocked. current-only evidence를 source exact로 승격하지 않음 |
| Source provider acquisition | `7da937aeaa34c088c694e8eb4f53ff1f7f848ef3` | accessible-scope evidence PASS, execution readiness 0/29 BLOCK. 29 occurrence→15 class→7 native family, provider/pilot 0, VSS `PERMISSION_UNCHECKED`, Product false |
| Source runtime materializer | `4ffe1102ed9cf3e21f669da292fac1f143e18d8f` | BLOCK. deterministic Productfalse projection scaffold만 인정. typed evaluator/PointLight/seed/default 손실, evidence registry 미결합, C++ generic bag 수용과 EOL/order 문제 교정 필요 |

현재 frozen R1 검증의 사실은 다음과 같다.

```text
Source acquisition mutation                21/21 PASS
Source actual-output readiness             0/29 BLOCK
Source ownerless row                       0
Geometry physical resource                 7/7 deployed and decoded
Material source-value acquisition          12/12 PASS
Material runtime mutation                  23/23 PASS
Material WARP oracle plumbing              numeric 200 / state pilot 4 PASS
Material source value                      render 0/89, static 23/94, sampler 0/72
Material execution readiness               0/255 BLOCK
Runtime execution admission                false
Product admission                          0/35
```

위 WARP PASS는 evaluator/state consumer plumbing의 재현성만 증명하며 source-era 값을 제공하지 않는다.
Source full audit의 lane 외 14건과 Material acquisition 이전 full audit의 lane 외 12건도 전체 green으로
세탁하지 않는다. 현재 병목은 renderer 코드 양이나 typed schema 전달이 아니라 source-era actual-output/
state provider 부재다. 이를 current default나 runtime fallback으로 덮고 Playback부터 진행하는 것을 금지한다.

## 고정 fidelity와 admission 계약

모든 definition, occurrence, resource binding은 다음 네 축과 네 blocker set을 독립적으로 가진다.

```text
sourceFidelity
  SOURCE_EXACT, CURRENT_REVISION_EVIDENCE, RECONSTRUCTED_NUMERICALLY_VERIFIED,
  RECONSTRUCTED_GRAPH, UNRESOLVED를 구분한다.

artifactBindingIntegrity
  compiled expected identity와 실제 payload bytes, package/export/record, model/material hash가
  일치하는지를 판정한다. self-signed container hash만으로 true가 되지 않는다.

executionAdmission
  payload를 읽기 전에 blocker가 0이고 해당 opcode/evaluator/renderer handler와 numeric oracle이
  존재할 때만 true다.

productAdmission
  execution, geometry, material, renderer, Tool/Catalog transaction과 최종 regression이 모두
  통과한 occurrence만 true다. 35개 중 하나라도 false면 cue 전체를 publish하지 않는다.

evidenceBlockers
  historical source fidelity가 닫히지 않은 이유다. verified reconstruction이 성공해도 삭제하지 않는다.

artifactBindingBlockers
  expected package/export/record/model/material identity와 실제 bytes가 다를 때만 유지한다.

executionBlockers
  opcode/evaluator/default/resource/renderer의 실행 의미가 닫히지 않았을 때 유지한다.
  numeric reconstruction proof는 이 집합의 명시된 blocker만 해소할 수 있다.

productBlockers
  execution, prepared transaction, runtime resource, regression이 제품 조건을 충족하지 못할 때 유지한다.
  evidenceBlockers가 남아 있다는 이유만으로 자동 생성하지 않고 G00의 허용 fidelity matrix로 계산한다.
```

`SOURCE_EXACT`는 완화하지 않는다. historical source가 없는 target을 current Constant나 class
이름만으로 exact 또는 executable로 바꾸지 않는다. `RECONSTRUCTED_NUMERICALLY_VERIFIED`는 별도의
evaluator ID/version, oracle provenance, 입력 영역, expected sample, tolerance, 독립 구현 비교를
모두 가진 경우에만 허용한다. `evidenceBlockers`는 이 승격 뒤에도 보존하되 독립적으로 검증된
reconstruction은 대응 `executionBlockers`만 해소한다.

maximum-reconstruction admission branch는 사용자의 별도 명시 승인으로
`RECONSTRUCTED_APPROVED_V1_POLICY_CONSTRUCTION` 상태다. Material의 기존 23 arithmetic evaluator와
WARP replay만으로 source value나 execution row를 자동 승격하지 않는다. Source 29행과 Material 255행은
각각 explicit policy row와 실제 consumer capability가 생긴 경우에만 reconstructed execution candidate에
포함한다. policy/capability가 없는 current default, class 이름, 암묵적 texture/state fallback은 계속
실행 또는 Product admission에 사용할 수 없다.

Product predicate는 마지막에 수동으로 정하지 않는다. 네 축, 네 blocker set, 허용 fidelity matrix로
G00에서 고정하고 G11은 같은 predicate를 평가만 한다. 서로 다른 blocker set을 하나의 union으로
합쳐 source exact가 아닌 행을 영구 Product false로 만드는 방식은 금지한다.

## 세션과 worktree 구조

세 세션은 동시에 계속 일하는 고정 worker가 아니라 dependency wave별 역할이다.

| wave | 세션 1 | 세션 2 | 세션 3 | 실제 병렬도 |
|---|---|---|---|---:|
| R1 evidence acquisition | Source `7da937ae`: accessible scope PASS, readiness 0/29 BLOCK | Material `cde8f3bd`: evidence PASS, readiness 0/255 BLOCK | frozen SHA 독립 review | acquisition 종료, evidence 정본 동결 |
| R2 integration/schema freeze | Source 29-row reconstructed capability 지원 | PASS checkpoint 결합, approval/compiler/program schema freeze | combined tree review | shared writer 1, IN_PROGRESS |
| R3 runtime consumer | custom/source oracle 조사 또는 idle | Playback -> Geometry -> Material shared runtime를 직렬 구현 | 각 frozen checkpoint review | 구현 1 + review |
| R4 renderer family | core 변경 없이 분리 가능한 family lane | renderer foundation과 다른 family lane, 최종 결합 | family frozen review | interface freeze 뒤 구현 2 |
| R5 Tool/Product | regression/fixture 지원 | Catalog -> Presentation -> Effect Tool transaction과 35/35 | final tree review | shared writer 1 |
| R6 runtime eye smoke | occurrence/fixture 회귀 지원 | exact published Artist F Debug/Release 실행 | smoke 결과와 numeric 재현 review | shared writer 1 |
| R7 Artist F freeze | source/material final hash 확인 | smoke 재현을 numeric gate에 환류하고 pipeline 동결 | final frozen tree review | shared writer 1 |
| R8 corpus expansion | 4-class corpus 적용 | Valtan corpus 적용과 integration | partition/final review | 구현 2 + review 순차 |

한 wave 안에서는 파일 소유권을 바꾸지 않는다. 세션 1이 Source corrective를 끝낸 직후 같은 dirty
worktree에서 renderer를 시작하지 않고, PASS commit을 종료한 뒤 새 exact integration parent에서 새
worktree를 만든다. 세션 3은 구현 코드를 쓰지 않고 frozen tree만 읽는다.

세션 간 통신은 다음만 허용한다.

1. 최소 public schema/typed field와 stable ID 요청
2. frozen commit SHA, parent SHA, owned file, 실제 검증과 remaining blocker
3. 재현 가능한 PASS/BLOCK의 입력 mutation, 기대, 실제
4. 실제 integration conflict 또는 전역 build lease 요청

진행률 메시지, live dirty diff 중계, 다른 lane 내부 구현 설명은 보내지 않는다. 이 규칙이 지켜지면
독립 session은 병목이 아니라 잘못된 fidelity 승격과 coordinated reseal을 merge 전에 차단하는 gate다.
반대로 reviewer가 live patch를 계속 따라가거나 두 mutation session이 같은 shared C++를 수정하면
review와 merge가 병목이 되므로 해당 wave를 즉시 직렬화한다.

Checkpoint 보고는 다음 필드를 모두 가진다.

```text
base SHA
commit SHA
owned files
public contract changes
tests actually executed
Product admission
remaining blockers
integration order
```

## 파일과 단일 소유자

| 구분 | 절대 경로 | 단일 소유자와 역할 |
|---|---|---|
| 통합 | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/.md/GB/08-10/2026-08-10_ARTIST_31470_F_COMPLETE_RESTORATION_IMPLEMENTATION_PLAN.md` | Integration Captain의 전체 G와 gate 정본 |
| Source schema | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Client/Public/Effect_AuthoringDocument.h` | G00 Integration만 public transport를 변경하고 이후 additive request를 중재 |
| Source Codec | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Client/Private/Effect_DocumentCodec.cpp` | G00 Integration과 Source reviewed bridge만 수정 |
| Source generator | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Tools/LevelPlacementExtractor/build_artist_31470_source_contract.py` | Source Closure 단일 소유 |
| Source outputs | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Data/Effects/Imported/Artist/Candidates` | Source Closure가 candidate/receipt를 단 한 번 재생성 |
| Source registry | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Data/Effects/Contracts/ue3-cascade-source-v1.registry.json` | Source Closure 단일 소유 |
| Geometry evidence | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Tools/ModelAssetConverter` | Geometry Evidence lane이 decoder/cooker/golden만 소유 |
| Geometry Engine | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Engine/Private/BinaryAsset/Winters` | Geometry Evidence lane이 WModel decode contract를 소유 |
| Material evidence | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Tools/LevelPlacementExtractor/build_artist_31470_material_evidence_contract.py` | Material Evidence lane 단일 소유 |
| Material outputs | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Data/Effects/Imported/Artist/Materials` | Material Evidence lane receipt/recipe 단일 소유 |
| Compiler | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Client/Public/Effect_CascadeCompiler.h` | G04가 inspection core를 만들고 G06이 execution IR shape/admission을 최종 동결 |
| Compiler implementation | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Client/Private/Effect_CascadeCompiler.cpp` | G04 inspection adapter 뒤 G06만 execution adapter/opcode/receipt를 최종 수정 |
| Executor | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Client/Private/Effect_Playback.cpp` | G07만 finalized execution IR 소비와 Mesh hidden scale 제거를 소유 |
| Prepared render | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Client/Private/Effect_DocumentRenderer.cpp` | Renderer Foundation이 common dispatch/cache/upload를 소유 |
| Presentation | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Client/Private/Effect_PresentationService.cpp` | Prepared/Catalog transaction lane이 exact revision attach를 소유 |
| Effect Tool | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Client/Private/Effect_Tool.cpp` | Tool lane이 read-only Source mode와 Draft/Product Play 경계를 소유 |
| Common shader | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Client/Bin/ShaderFiles/Shader_EffectCommon.hlsli` | Renderer Foundation 단일 소유; family lane이 수정하지 않음 |
| Light public payload | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Engine/Public/Engine_Struct.h` | G06이 radius/falloff/color/brightness typed payload를 소유 |
| Deferred Light shader | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Engine/Bin/ShaderFiles/Shader_Deferred.hlsl` | G06이 typed falloff 소비와 attenuation contract를 소유 |
| Shared registrations | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Client/Default/Client.vcxproj` | 필요한 lane commit에 additive 등록하고 Integration이 충돌 해결 |
| Audit registration | `C:/Users/user/.codex/worktrees/artist-f-v2-integration/LostArk/Tools/ProjectAudit/Invoke-ProjectAudit.ps1` | Integration이 gate별 focused audit를 최종 등록 |

Source candidate는 실제로 직접 pin하는 Source/geometry/active-material-closure 입력만 재생성한다.
새 typed MaterialRecipe 전체를 Source candidate에 다시 복제하지 않는다. 최종 compiled receipt가 Source,
Geometry, Material contract hash와 compiler revision을 함께 결합해 불필요한 hash regeneration cycle을
막는다.

## 현재부터 완성까지의 실행 순서

아래 순서는 기존 G 전체를 다시 시작하는 계획이 아니라 R0 evidence PASS 뒤 execution-readiness를 열어
PASS된 Runtime foundation `38ebe7cf` 위에 최종 pipeline을 결합하는 critical path다.

### R0. 35/35 feasibility와 blocker owner 동결 — matrix PASS, execution BLOCK

Playback과 renderer 구현 전에 Source 29개 blocked module과 Material 255개 execution row를 행 단위로
지도화했다. 이 gate는 문서상 owner 이름만 채운 것이 아니라 각 행의 acquisition/oracle 경로와 실패
판정을 frozen receipt로 고정했다.

Source feasibility matrix의 각 행은 다음을 가진다.

```text
moduleOccurrenceId
exactSourceClass
family
requiredRuntimeOutputs
sourceEraPackageOrBinaryIdentity
currentRevisionEvidenceIdentity
nativeEntryOrDispatchIdentity
numericOracleInputDomain
numericOracleExpectedOutput
independentOracleImplementation
oracleProvider
pilotFixtureIds
pilotExpectedMutatedOutputs
numericTolerance
pilotDecision
fidelityDecision
executionDecision
owner
remainingBlockers
```

Source safe denominator는 29 blocked module이다. standard seeded 11, EF custom 15, EF multiply
distribution owner 3을 임의로 합치거나 denominator에서 제거하지 않는다. input digest parity는 output
oracle가 아니다. source-era identity 또는 실제 native particle output 비교가 없으면 READY로 승격하지
않고 `CURRENT_REVISION_CROSS_REVISION_ALIAS_EVIDENCE`와 blocker를 유지한다. 각 family는 최소 하나의
실제 pilot fixture로 particle/component output 변화와 tolerance를 증명해야 하며 `FEASIBLE`,
`VERIFIED_IRRELEVANT`, `BLOCKED` 중 하나로 판정한다.

Material feasibility matrix의 각 행은 다음을 가진다.

```text
materialRecipeId
materialOccurrenceIds
fieldId
fieldKind
bindingOriginAndOwner
instanceRecordIdentity
parentOrCdoIdentity
staticOrShaderMapIdentity
rendererConsumption
acquisitionPath
oracleProvider
pilotFixtureIds
numericOracleInputDomain
numericOracleExpectedOutput
numericTolerance
pilotDecision
fidelityDecision
executionDecision
owner
remainingBlockers
```

Material safe denominator는 27 recipe/34 occurrence, 162 render-state field 중 explicit 73/unresolved 89,
static input 94, strict sampler 72다. 이전 exact sampler 3 instance+1 parent는 texture binding/DDS identity만
exact하며 source-era Texture2D CDO와 TextureGroup filter configuration이 없는 full descriptor는 증명하지
못하므로 strict sampler에 포함한다. render-state 89, static 94, sampler 72를 별도 matrix로 유지하며
validator correction과 혼동하지 않는다.
omitted field는 instance -> Parent Material -> nested default -> class CDO 순서로만 해석한다. D3D
blend/depth/cull state, static permutation, sampler address/filter/sRGB를 offscreen WARP 또는 독립 state
oracle의 작은 pilot로 실제 관측하지 못한 행은 BLOCK으로 유지한다. source-revision ShaderCache 또는
controlled runtime capture가 필요한 행은 acquisition provider가 실제로 동작하기 전까지 FEASIBLE로도
승격하지 않는다.

Frozen 결과는 source value와 execution readiness를 분리한다. Material static 23행은 source MIC의 exact
`bOverride=true` 값을 확보했지만 actual state-output pilot과 final consumer가 없으므로 READY가 아니다.
WARP 200 numeric sample과 state pilot 4건도 source 값 provenance가 아니라 oracle plumbing 검증이다.

```text
Source matrix integrity                    PASS, ownerless 0
Source execution readiness                 0/29 BLOCK
Material matrix integrity                  PASS, ownerless/unknown 0
Material source value                      23/255
Material execution readiness               0/255 BLOCK
VSS                                        PERMISSION_UNCHECKED
NVIDIA DXCache                             548/561 readable, 13 share-locked
global exhaustion claim                    false
```

R0 evidence-integrity 합격 조건:

- Source blocked 29/29, Material render-state 89/89, static 94/94, sampler 72/72가 matrix에 존재한다.
- owner 없는 행 0, silent fallback 0, denominator shrink 0이다.
- 각 행/family는 필요한 expected output과 tolerance 계약을 기록하며 provider, pilot 또는 tolerance가
  없으면 `BLOCKED`로 보존한다.
- READY 후보는 source-era evidence 또는 실제 expected mutated output과 tolerance를 고정한 독립
  actual-output/state oracle pilot을 가진 경우에만 존재할 수 있다.
- current-only/cross-revision 행을 `SOURCE_EXACT`로 승격한 행 0이다.
- acquisition이 불가능한 행은 즉시 명시되어 Artist F 35/35의 hard blocker로 보고된다.
- R0 matrix와 resolver/validator mutation test가 frozen review PASS하기 전 R2 이후 shared runtime 구현을 시작하지 않는다.

위 evidence-integrity 조건은 `7da937ae`와 `cde8f3bd`에서 충족했다. 이는 원본 의미 복원 완료가 아니다.
execution readiness는 Source 0/29, Material 0/255이므로 별도 BLOCK이며 R2 이후를 열지 않는다.

### R1. Source/Material provider acquisition — evidence PASS, execution BLOCK

Source `7da937aeaa34c088c694e8eb4f53ff1f7f848ef3`은 접근 가능한 filesystem/backup/current install/
Git remote·LFS·unreachable object 범위를 조사하고 29 occurrence를 15 exact class와 7 native family로
축약했다. source-era actual-output provider와 standalone pilot은 각각 0이고 blocker delta는 `29 -> 29`다.
current wrapper/dataflow와 fixed input digest는 `CURRENT_REVISION_CROSS_REVISION_ALIAS_EVIDENCE`로만 보존한다.

Material `cde8f3bddea2f9415f682b387d2705fd25794075`는 `627ddc76`의 잘못 승인된 exact sampler 4건,
`ab76b7ec`의 row/evaluator validator gap, `d39097c3`의 acquisition/runtime top-level schema gap을
교정하고 typed contract v4와 runtime receipt v3를 재봉인했다. render-state source value는 0/89,
static은 exact override value 23/94, sampler full descriptor는 0/72다. static nonoverride semantics 미확정
43행과 GUID join 없음 28행을 분리하며 execution readiness는 전체 0/255다.

두 lane 모두 VSS는 비관리자 권한으로 `PERMISSION_UNCHECKED`이고 Material GPU cache는 NVIDIA 561개 중
13개가 share-locked다. 접근 가능한 범위는 소진했지만 `globalExhaustionClaim=false`다. safe source-era
standalone runtime/UCC/commandlet/debug capture 경로가 없으며 injection/hooking/anti-cheat 우회는 시도하거나
허용하지 않는다. current capture는 가능해져도 `CURRENT_REVISION_OBSERVED`일 뿐 source-era fidelity를 닫지 못한다.

현재 판정:

```text
Source acquisition evidence                PASS_ACCESSIBLE_SCOPE_EXHAUSTED
Source actual-output execution readiness   0/29 BLOCK
Material evidence integrity                PASS
Material source value                      23/255
Material execution readiness               0/255 BLOCK
ownerless/unknown row                       0
typed materializer                         BLOCK, not started
reconstructed branch                       RECONSTRUCTED_APPROVED_V1 / POLICY_CONSTRUCTION
Product admission                          false / 0/35
R2                                          IN_PROGRESS
R3-R8                                       GATED
```

source-era exact 경로의 R2 진입 predicate는 Source 29/29, Material render 89/89·static 94/94·sampler
72/72, unresolved execution row 0으로 유지한다. 사용자가 승인한 reconstructed 경로의 R2 진입
predicate는 frozen evidence-integrity PASS, `sourceExactAdmission=false`, 29+255 denominator 보존,
explicit per-row policy/capability/oracle owner, silent fallback 0이다. Corrected materializer는 이 정책을
입력으로 R2에서 새로 작성하고 R2 종료 시 독립 PASS해야 한다. R3 진입은 materializer의 field coverage
100%, unknown/ownerless 0, 29+255 reconstructed capability receipt와 Debug/Release parser mutation PASS를
추가로 요구한다.

### R1 재개 입력 계약

`SOURCE_EXACT` 재개에는 같은 revision의 `EFEngine.dll`, `LOSTARK.exe`, `Engine.u`, `Core.u`, `EFGame.u`,
target UPK, ShaderCache/material map, SystemSettings TextureGroup config와 single-revision identity manifest가
필요하다. 대안은 동일 build identity와 fixed seed/time/world/parameter input, pre/post full numeric state,
expected output과 tolerance를 가진 인증된 source-era capture다.

최대 reconstruction 분기는 사용자 승인으로 시작한다. 행별 explicit project policy와 independent
same-input output/state oracle을 통과한 값만 versioned `RECONSTRUCTED_APPROVED_*` 또는
`RECONSTRUCTED_NUMERICALLY_VERIFIED`로 표시하며, evidence blocker, `sourceExactAdmission=false`,
Product false를 보존한다. 이미지 비교는 numeric admission 입력이 아니며 R6 수동 smoke에서 발견한
차이를 occurrence별 fixture로 환류하는 용도로만 사용한다.

이하 R2~R8은 reconstructed approval receipt와 각 gate의 execution predicate를 순서대로 충족할 때만
진행한다. 현재 R2만 열려 있고 R3~R8은 앞 단계 PASS 전까지 `GATED`다.

### R2. 승인 checkpoint 통합과 final typed schema 동결

1. clean final-integration worktree를 PASS된 Runtime foundation `38ebe7cf`에서 만든다.
2. Geometry는 조상에 이미 포함된 evidence commit을 재적용하지 않고 `git cherry-pick -x 669acf07`,
   이어서 `git cherry-pick -x 0aca7928`을 각각 실행한다. `669acf07..0aca7928` range는 첫 commit을
   누락하므로 사용하지 않는다.
   Compiler `c4b00f14`와 Publisher `c90c462`는 patch-equivalent 조상에 이미 포함되므로 재적용하지 않는다.
3. Source는 `ae425aaf -> 9b046d61 -> c927e397 -> 7da937ae (evidence PASS, readiness 0/29)
   -> <future 29/29 readiness>` 전체 검증 단위를, Material은 `532d1f52 -> b6757a21 -> e54a5a2a
   -> 627ddc76 (superseded) -> ab76b7ec (validator BLOCK) -> d39097c3 (top-level schema BLOCK)
   -> cde8f3bd (evidence PASS, readiness 0/255)
   -> <future 255/255 readiness>`
   전체 검증 단위를 적용한다. 잘못된 중간 READY 상태에서는 build, publish, admission을 수행하지 않는다.
4. BLOCK materializer `4ffe1102`는 cherry-pick하지 않는다. final Source/Material schema를 받은 combined
   head에서 corrected materializer를 새로 작성한다.
5. Source 35/399/629, Geometry 7, Material 27/34를 결합한 six-hash compiler input을 한 번 생성한다.
6. exact class, opcode, distribution evaluator, renderer, GeometryBinding, MaterialBinding capability ID와
   version을 동결한다.
7. blocked payload는 읽기 전에 거부하고 runtime fallback은 0으로 유지한다.

`native-v14.source-contract-candidate.effect.json`, 그 receipt, `4ffe1102` runtime-program candidate는 현재
R1 readiness를 반영하지 않은 stale 산출물이므로 재사용하지 않는다. 재생성은 Source/Material readiness
receipt -> Source candidate/receipt -> corrected runtime program -> six-hash compiler input -> derived
Assembly/artifact -> Catalog/prewarm 순서로 정확히 한 번 수행한다.

R2 종료 조건은 corrected materializer independent PASS, field loss 0, stale output 0, unknown capability 0,
ownerless blocker 0이며 Product false다.

### R3. Actual typed executor와 resource consumer

이 구간은 shared C++ dependency가 촘촘하므로 세션 2 한 명이 다음 순서로 직렬 구현한다.

1. Playback이 immutable compiled program pointer만 소비하게 하고 raw SourceRecipe/module 문자열 scan을 0으로 만든다.
2. standard distribution, fixed seed/time, ActionCue, spawn/lifetime/location/velocity/size/color/SubUV와
   TypeData handler consumption receipt를 연결한다.
3. GeometryBinding expected tuple을 `CModel -> CMesh -> CMaterial`에 연결한다.
   `geometryPreScale=0.01`은 vertex와 bounds에 정확히 한 번 적용하고 Mesh StartSize hidden `x0.01`을 제거한다.
4. MaterialBinding 27 recipe/34 occurrence를 typed evaluator와 HLSL에 연결하고 explicit render state만 소비한다.
5. post-compile raw semantic I/O와 재compile은 0이며 실패 시 이전 prepared pointer/cache를 보존한다.

R3 종료 조건은 399 opcode/629 distribution consumption, Geometry 7/7, Material 34/34, deterministic
fixed seed/time, fallback 0, Debug/Release PASS다. Product는 아직 false다.

### R4. 여섯 renderer family

공통 render packet, vertex layout, MaterialBinding slot, family dispatch interface를 세션 2가 먼저 동결한다.
그 뒤에만 서로 다른 파일을 소유하는 두 family lane을 병렬화한다.

1. Mesh + Sprite: 13 + 16
2. Decal + Ribbon: 3 + 1
3. Light + ScreenPost: 1 + 1

각 family는 같은 compiled IR pointer를 소비하고 size/rotation/local-space/timing/material/render-state
numeric packet을 검증한다. raw `eKind`, raw SourceRecipe, legacy heuristic dispatch는 Product path에서 0이다.

### R5. Effect Tool, Catalog와 Artist F 35/35

1. v14 Source Contract는 Effect Tool에서 read-only inspection만 제공한다.
2. Product Play는 Authored disk reload가 아니라 exact catalog prepared revision을 attach한다.
3. publish -> catalog load -> prewarm -> equivalence -> target commit을 하나의 transaction으로 묶는다.
4. 35 occurrence 각각에 source/compiled identity, opcode, distribution, geometry, material, renderer,
   fixed sample expected/actual/tolerance를 기록한다.
5. `35/35`, Geometry `7/7`, Material `34/34`, fallback/blocker `0`일 때만 cue 전체 Product를 atomic publish한다.

### R6. Runtime 실행과 눈으로 확인

자동 admission이 끝난 뒤 Client를 `Client/Default` working directory에서 Debug와 Release로 실행한다.
도화가 F를 exact published revision으로 재생하고 complete/family/occurrence filter가 같은 IR pointer를
사용하는지, 이펙트가 실제 world/camera에서 누락·폭주·잘못된 scale 없이 재생되는지 수동 smoke한다.

정식 build는 저장소 root에서 `Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug`와 Release를
직렬 실행한다. 실제 실행은 `Framework.slnLaunch`의 `Server + Client` profile을 사용한다. Lobby에서
Server 승인을 받아 Character Select로 진입한 뒤 Artist thumbnail을 선택하고, Debug는 F1 Effect Tool의 Active Product Cue와
실제 F 입력을 모두 확인한다. Release는 F1 도구가 없으므로 Server command -> snapshot -> animation
event -> exact catalog prepared spawn의 실제 F 경로만 확인한다. `Lobby -> Test`는 캐릭터/네트워크 없는
Map Editor이므로 이 smoke에 사용하지 않는다.

R6 전에 `effect.artist.skill.31470`의 v13 identity carrier, Assembly, compiled artifact/receipt, format-3
Catalog entry, `PlayerSkills.json.effectId`, `sdm_sk_onestroke`의 generated `effectref=asset`이 같은 prepared
identity를 가져야 한다. 현재 이 Product chain은 존재하지 않으므로 R5가 생성한다. Geometry WModel 7개와
compiled MaterialBinding에서 재도출한 DDS 전부를 hash-bind하며, exact recovery 4개
`fx_a_noise_011`, `fx_e_ring_001_cl`, `fx_a_decal_014`, `fx_c_atypical_016`도 canonical Resources에
publish됐는지 확인한다.

수동 timeline 기준은 0 ms Ink 4, 1338 ms Weapon 1, 1380 ms Swing 15, 1451 ms Hit 12와
ZoomBlur 1, 1452 ms Distortion 1, 1453 ms Light 1이다. 총 action 2.833초 안에서 종료되어야 하며 Mesh
scale/pivot/basis, Sprite billboard/SubUV, Decal projection/depth, Ribbon continuity/tail, Light radius/lifetime,
Post lifetime, Material fallback/depth/cull/sampler, attachment/world-space를 family와 occurrence filter로 확인한다.

이 단계는 numeric/structural admission을 대체하지 않는다. 스크린샷 캡처와 이미지 기반 자동 비교는
하지 않는다. 수동 관찰에서 이상이 보이면 해당 occurrence ID와 compiled revision을 기록하고 R3~R5의
numeric oracle에 재현을 추가한 뒤 다시 admission한다.

### R7. Artist F 최종 동결과 회귀

R6 수동 smoke에서 발견된 모든 이상을 occurrence ID와 compiled revision으로 numeric/structural fixture에
환류한다. Debug/Release, focused/deep ProjectAudit, publish/prewarm rollback, no-I/O prepared attach를 다시
통과하고 Source `35/399/629`, Geometry `7/7`, Material `27/34`, renderer `13/16/3/1/1/1`, Product
`35/35`가 동일 frozen tree에서 유지될 때 Artist F compiler/runtime/renderer interface를 동결한다.
수동으로만 맞춘 값이나 Product path 전용 class/count 분기는 0이어야 한다.

### R8. 4-class와 Valtan 확장

Artist F runtime/compiler/renderer를 동결한 뒤 세션 1은 4-class corpus, 세션 2는 Valtan
Particle/Decal/Trail/Material/Camera corpus를 병렬 적용한다. production code에 class/count switch를
추가하지 않고 fixture denominator만 확장한다. 공통 handler가 부족하면 corpus 값을 근사하지 않고
확장을 멈춘 뒤 shared pipeline gate로 되돌아간다.

4-class source corpus는 Artist F 35개를 포함하지 않는 별도 분모다.

| class | skill/stage/clip | source occurrence | 기존 Product cue |
|---|---:|---:|---:|
| DimensionMaster | 12/15/22 | 872 | 19 |
| LanceMaster | 17/27/45 | 2,199 | 41 |
| Artist (`31470` 제외) | 9/15/16 | 841 | 14 |
| Warlord | 13/17/30 | 1,320 | 27 |
| 합계 | 51/74/113 | 5,232 | 101 |

portable inventory revision을 먼저 고정하고 Artist 9-skill canary -> DimensionMaster -> Warlord ->
LanceMaster 순으로 cue/document atomic admission한다. source 51문서와 기존 Product 101 cue는 서로 다른
분모이므로 합치지 않는다.

Valtan의 정본 분모는 action 170, stage 2,464, clip 2,378, notify 21,931이며 typed target은 Particle
6,159(`PlayParticleEffect` 6,026 + quarantined `DefaultParticle` 133), Decal 536, Trail 430, Material
606, Camera 1,022다. generic `Effect` 3,787은 typed target으로 승격하지 않고 격리한다. 세 Action LOA를
raw SHA로 pin한 뒤 Particle -> Decal -> Trail -> Material -> Camera 순으로 적용한다.

R1 acquisition audit는 끝났지만 execution gate가 BLOCK이므로 R2~R8은 아직 개방되지 않았다. provider가
들어와 R1 readiness를 통과한 뒤에도 R3 typed consumer와 R4 renderer가 큰 구현 구간으로 남으며, R6
Server+Client runtime smoke와 R7 final regression 전에는 화면에 보이는 legacy effect를 복원 완료로
판정하지 않는다.

## 기존 G 상세 계약

아래 G00~G11은 원래 계획의 상세 acceptance contract다. 현재 실행 순서는 앞의 R1~R8을 정본으로
사용하며, 이미 PASS한 G를 재구현하지 않는다.

## G00. Known-red base와 public handshake 동결 (historical)

Integration Captain이 `068fcc2` 위에 이 계획과 최소 public handshake를 고정한다.

고정 필드는 다음과 같다.

- `(sourceOccurrenceId, sourceSystemId)` composite emitter identity
- selected LOD node/path/provenance
- ordered module reference index, role, stable ID, exact class/alias lineage
- property `(storage, canonicalPropertyPath, referenceId)`
- distribution definition/occurrence/reference ID, typed payload variant, fidelity, blocker, admission
- PointLight typed component binding과 instance/current inherited default 분리
- GeometryBinding expected tuple
- MaterialRecipe/MaterialOccurrence ID와 contract hash
- renderer family와 source space
- canonical source document hash, compiled IR hash, catalog/compiler revision

G00 종료 조건:

- 모든 Wave 1 branch가 같은 G00 commit에서 시작한다.
- Product 0/35와 known-red baseline 세 항목을 기록한다.
- Source/Compiler가 공통 schema/Codec를 각각 수정하지 않는다.
- 부족한 field는 Integration에 최소 typed request로 제출하며 승인 전 다른 field에 smuggle하지 않는다.

## Wave 1A. Evidence와 generic compiler core 병렬

### G01. Source Evidence Final

Source Closure는 local distribution만이 아니라 selected-LOD 전체 semantic evidence와 reconstruction
oracle을 소유한다.

- 15 distribution target / 17 occurrence와 PointLight 1 occurrence
- 399 ordered module reference, 1,434 top-level property, 1,572 primitive leaf, 629 distribution
- `INSTANCE_EXPLICIT -> NESTED_ARCHETYPE_TEMPLATE -> CLASS_CDO -> PARENT_CDO_HIERARCHY -> EVALUATOR_DEFAULT`
  순서의 field provenance
- target000/001/007/009/014와 custom evaluator의 pre-payload rejection
- raw decoded evidence와 executable typed payload의 분리
- external native-tail 248, seeded module 14, Required local-space default 8, Decal default 3,
  Ribbon default 1, ScreenPost default 1과 selected-LOD/class default의 closure 또는 실행 무관 증명
- standard UE3 module은 tagged instance/archetype/CDO/script/native contract를 결합하고 custom EF class는
  별도 native/differential numeric oracle 없이는 standard handler로 분류하지 않음
- 각 property를 `EXECUTION_CONSUMED`, `VERIFIED_IRRELEVANT`, `UNRESOLVED` 중 하나로 분류하고 근거 receipt 고정
- evidence/artifact/execution/Product blocker를 분리해 definition/property/module/element/receipt/registry/compiler로 전파
- code-only checkpoint와 마지막 bounded source-era artifact 조사

Source-era artifact가 없으면 source fidelity는 `UNRESOLVED` 또는 `CURRENT_REVISION_EVIDENCE`로
동결한다. current package constant를 old child payload로 사용하지 않는다. 실행 의미는 별도의
`RECONSTRUCTED_NUMERICALLY_VERIFIED` receipt가 있는 경우에만 열 수 있으며, standard/native evaluator와
default/seed가 numeric output에 미치는 영향을 독립 sample로 고정한다. Geometry receipt가 확정되기 전
candidate/header를 최종 재생성하지 않는다.

합격:

- 15 definition/17 occurrence와 PointLight active reference all-consumed
- 399 module, 1,434 top-level property, 1,572 leaf, 629 distribution이 consumed 또는 verified-irrelevant이며
  unknown/unconsumed/silent-ignored 행 0
- payload decoded/unresolved, semantic ready/blocked, compiled/admitted 분모를 각각 보고
- target000/001/007/009/014, seed/native-tail/default/Light archetype-CDO mutation이 production resolver에서 실패
- code-only commit은 pure resolver/binder, temp-output generation, mutation test만 PASS로 주장
- checked-in `--check`, deep audit, Codec round-trip은 Gate 1 regeneration commit에서 실행
- Product false

### G02. Geometry Evidence Final — PASS `0aca7928`

Geometry lane은 WModel evidence와 Engine decode contract만 닫는다. `Effect_Playback`과
`Effect_DocumentRenderer`는 수정하지 않는다.

필수 작업:

- strict JSON integer version과 canonical EOL/raw hash domain
- Python/C++ 동일 tangent-W 허용 계약
- writer-independent immutable golden과 actual 7-carrier C++ channel/index/bounds/hash oracle
- actual Resources legacy v1.0 static/skinned/hasBounds/multi-submesh corpus C++ sweep
- full transactional rollback
- self-consistent metadata와 externally bound expected identity 분리
- pivot와 exact UPK→glTF가 없으면 source blocker 유지

합격:

- frozen commit 독립 PASS
- actual 7 carrier C++ Debug/Release 7/7
- legacy corpus 2,586 decode regression
- Engine -> UpdateLib -> Client Debug/Release
- Resources와 Effect runtime diff 0
- Product false

### G03. Material Evidence Final — `cde8f3bd` evidence PASS, execution 0/255 BLOCK

Material lane은 raw evidence와 typed recipe만 닫고 HLSL/runtime을 수정하지 않는다.

필수 작업:

- source-pack manifest를 통한 logical package와 raw export identity 결합
- 342 scalar / 19 vector / 71 texture의 raw array order/name/value/reference/owner 결합
- MIC Parent edge와 selected base graph exact identity
- Texture2D export/serial/tagged sampler와 DDS binding
- parent/default/render/static field owner lineage
- recipe composition digest와 34 occurrence identity 연결
- coordinated closure/receipt reseal, label/parentGraph/inputs/defaults/sampler owner swap 거부
- checked receipt/contract 재생성으로 현재 hash mismatch 제거
- MIC native `FStaticParameterSet`, Engine/EFGame Material CDO, Texture sampler default, installed shader cache와
  surviving expression/native material metadata를 조사해 static/full-render/full-cull/default/sampler/graph별
  reconstruction oracle receipt 생성
- render-state 89, static 94, strict sampler 72, graph 23/502 edge를 각각 행 단위로 보존하며 값이 없는
  행을 공통 fallback으로 채우지 않음

합격:

- recipe row 27/27, occurrence 34/34, unused/unexpected recipe 0
- sampler exact 0, rejected legacy 3 instance + 1 parent, direct unproven 71, strict sampler 72
- graph family 23, null 1,803, unresolved edge 502, Source-exact graph 0
- shallow/deep generator, raw UPK/DDS/manifest audit와 mutation tests
- 23 family의 independent expected sample 출처와 아직 구현되지 않은 evaluator를 별도 기록
- evidence commit에서는 evaluator implemented 0과 Product false를 정직하게 유지하되,
  Gate 1 실행-readiness matrix에서 owner 없는 material blocker 0

### G04. Generic Compiler Core — non-executable checkpoint PASS `c4b00f14`

Compiler lane은 Source 재생성 전 generic typed core만 구현한다.

필수 작업:

- System -> Emitter -> selected LOD -> ordered module -> property/distribution immutable IR
- opcode별 allowed/required schema와 실제 handler consumption receipt
- canonical document identity와 full-field deterministic IR hash
- admission-before-payload, unknown/duplicate/nonfinite/alias/LOD mutation rejection
- generic production invariant와 Artist fixture denominator 분리
- Source generated header read-only 소비

G04에서는 current generated candidate의 최종 adapter PASS를 주장하지 않는다. `7/35/399/629`와
`13/16/3/1/1/1`은 harness fixture에만 둔다. production에 Artist/31470/count hardcode를 넣지 않는다.

합격:

- raw module payload materialization 0
- generic inspection/compiler Debug/Release harness와 static audit
- Product false

## Gate 1. Frozen audit, 통합, Source 단일 재생성 — historical, R2가 supersede

각 G01-G04 lane은 stable commit을 push하고 mutation을 멈춘다. Independent Review는 commit SHA 하나만
검사하고 read-only PASS/BLOCK을 보낸다. 구현 helper와 같은 parser만 사용한 자기확인은 PASS가 아니다.

이 절의 옛 cherry-pick 순서는 사용하지 않는다. 실제 재개 순서는 R2를 정본으로 하며 base
`38ebe7cf`, Geometry `669acf07`과 `0aca7928`의 개별 cherry-pick, compiler/publisher 재-pick 금지,
`4ffe1102` 폐기와 combined head 재작성을 따른다. 이 historical 문구의 통합 금지는
`RECONSTRUCTED_APPROVED_V1` 승인 전 상태를 설명한다. 승인 뒤 실제 통합은 R2 절의 exact 14-commit
순서와 새 policy/materializer gate를 따른다.

Gate 1 종료 조건:

- stale generated output 0
- Source 7/35/399/629와 renderer 분모 보존
- Material 27/34와 Geometry 7 hash를 compiled input receipt가 결합
- actual regenerated candidate의 Codec round-trip
- occurrence별 remaining execution blocker matrix와 owner가 존재
- owner 없는 blocker가 하나라도 있으면 Renderer 단계로 진입하지 않음
- Product false, integration worktree clean

## Gate 1B. Execution-readiness와 runtime resource provisioning

Renderer나 runtime authority를 수정하기 전에 네 병렬 closure job으로 실제 실행 입력을 준비한다.

### G05-S. Source Semantic Reconstruction Readiness — `c927e397 -> 7da937ae`, evidence PASS/readiness 0/29 BLOCK

G01에서 남은 standard/native/default/seed/custom evaluator 행을 actual regenerated candidate 기준으로
다시 계산한다. 각 399 module과 1,434 top-level property는 실행 handler가 소비하거나 독립적으로
실행 무관임을 증명해야 한다. target007/014 custom EF evaluator, external native-tail, seed와 local-space,
Decal/Ribbon/Post/Light default에 evaluator ID와 numeric oracle을 부여한다. current-only evidence는
source exact로 승격하지 않는다.

접근 가능한 provider acquisition은 끝났고 source-era provider/pilot은 0이다. 아래 semantic 작업은 동일
revision artifact 또는 승인된 authenticated capture가 들어와 R1을 다시 열 때만 재개한다.

### G05-G. Geometry Candidate Cook and Resource Provisioning — PASS `0aca7928`

G02의 7개 WModel 1.1 candidate를 deterministic cook하고 expected tuple을 생성한다. candidate는 먼저
임시 staging directory에서 검증한 뒤 팀장 관리 물리 root인
`C:/Users/user/Desktop/LostArk/Client/Bin/Resources/Effect/Artist/Meshes`의 정확한 7개 target만
backup -> atomic replace한다. 실패하면 기존 7개를 복원한다. Git worktree의 Resources 유무를 제품
payload 존재 증거로 사용하지 않고 test process의 `LOSTARK_RESOURCE_ROOT`를 위 물리 root로 고정한다.

### G05-M. Material Reconstruction Oracle and Texture Provisioning — `cde8f3bd` evidence PASS/readiness 0/255 BLOCK

current Engine/EFGame CDO와 installed cache는 cross-revision 후보일 뿐 source value provider가 아니다.
`cde8f3bd`는 source MIC static override value 23건만 보존했고 render/sampler 및 execution readiness는
열지 않았다. 동일 revision provider 또는 승인된 authenticated capture가 들어올 때만 27 recipe의
static/render/cull/default/sampler와 23 arithmetic family expected numeric sample을 닫는다.
모든 34 occurrence가 참조하는 texture dependency를 열거하고 기존 Resources DDS 또는 새로 검증한 DDS를
명시적인 asset ID/hash로 stage한다. white/black/legacy-default texture를 missing input 대체값으로 쓰지 않는다.
물리 texture 교체가 필요하면 G05-G와 같은 backup -> validate -> atomic replace -> rollback을 사용한다.

### G05-P1. Derived Artifact and Publisher Schema Code

v14 Source Contract 자체는 계속 read-only/non-drawable이다. 이 lane은 v14 Source, Geometry,
Material contract와 semantic reconstruction receipt를 결합할 `lostark.effect-authoring` v13
derived authoring/Assembly identity carrier와 compiled artifact의 schema만 구현한다. v13 carrier는
runtime semantic authority가 아니며 생성물은 다음 identity를 저장한다.

```text
sourceContractHash
sourceSemanticClosureHash
geometryContractHash
materialContractHash
resourceBindingHash
compilerInputHash
```

이 병렬 lane은 generator, serializer, publisher validator, runtime catalog format 3 schema와 synthetic
zero-blocker fixture만 구현한다. G05-S/G/M의 최종 hash를 요구하거나 actual Artist F v13
document/Assembly/compiled artifact를 생성하지 않는다.
`Publish-Effects.ps1`은 verified v13 derived document와 compiled artifact를 함께 검증하고 v14를 직접
publish하지 않는다. runtime catalog format 3은 위 identity와 compiled receipt/revision을 보존한다.
actual generator는 execution blocker가 하나라도 있으면 v13 document/Assembly를 만들지 않고 이전
Product를 유지하며, G10 이후 모든 final hash가 동결된 Gate 10B에서 한 번 실행한다.

Gate 1B 종료 조건:

- unknown opcode/property, unconsumed property, silent ignored native/default field가 각각 0
- Source semantic, Geometry payload, Material recipe/resource, derived Product 입력에 owner 없는 blocker 0
- runtime Resources의 7 WModel과 모든 required DDS가 compiled expected identity와 일치
- G05-P1 synthetic zero-blocker fixture만 v13/schema/publisher round-trip을 통과
- actual Artist v13/compiled artifact는 아직 생성하지 않고 Product bit/publish는 false
- Source exact가 아닌 reconstruction 행의 evidence blocker는 보존

## G06. Runtime Authority Foundation — PASS `38ebe7cf`

G05-S/G/M/P1 frozen PASS 뒤 단일 owner가 중앙 runtime authority와 extension point를 먼저 고정한다.
이 G 뒤에는 family/geometry/material lane이 중앙 파일을 다시 수정하지 않는다.

소유 파일:

- `Effect_CascadeCompiler.h/.cpp`
- `Effect_Playback.h/.cpp`의 prepared input boundary
- `Effect_DocumentRenderer.h/.cpp`의 common cache, upload와 family dispatch boundary
- `Effect_PresentationService.cpp`
- `Effect_Catalog.cpp`
- `Effect_Object.h/.cpp`
- common shader binding layout
- `Publish-Effects.ps1`과 runtime catalog format 3 parser/stager
- `Engine/Public/Engine_Struct.h`
- `Engine/Bin/ShaderFiles/Shader_Deferred.hlsl`

필수 계약:

- immutable prepared identity `{catalogRevision, compilerRevision, assetId, sourceHash, compiledHash,
  geometryHash, materialHash, resourceBindingHash}`
- v14 Source/Geometry/Material에서 immutable execution IR을 만드는 최종 adapter, opcode shape와
  execution admission compile을 G06이 소유하고 이후 G07은 compiler 파일을 수정하지 않음
- 각 opcode에 required handler ID/version을 고정하고 G07-G10이 fixed extension point로 제출하는
  capability/consumption receipt가 없으면 execution admission을 열지 않음
- Source/Geometry/Material을 한 번 compile하고 prepared pointer만 consumer에 전달
- Catalog -> Presentation -> Object -> Playback -> Renderer가 같은 compiled document identity를 확인
- central family interface, geometry cache key, material evaluator handle와 shader slot 선할당
- typed Light descriptor에 radius/falloff/color/brightness를 명시하고 `CEffectObject`와 deferred shader가
  고정 linear attenuation으로 세탁하지 않을 public binding 고정
- publish/load/prewarm/equivalence 전체 stage -> validate -> commit과 실패 rollback
- combat Spawn의 compile/model/DDS/vector-field I/O 0
- compile A 뒤 raw B attach, revision/hash/pointer/Product mismatch 거부
- `v14 Source Contract -> immutable compiled execution IR`만 runtime semantics의 단일 권위이며,
  v13 derived document/Assembly는 Catalog identity와 authoring carrier일 뿐 Playback/Renderer가 raw
  semantics를 읽지 않음

G06은 아직 family output을 Product로 승인하지 않는다. 중앙 Renderer의 v14 dispatch는 compiled family
enum만 사용하고 unsupported family는 fail-close한다. Light public payload와 deferred shader 변경은 같은
SHA/worktree에서 `Engine -> UpdateLib -> Client` Debug/Release build lease를 사용한다.

합격:

- actual `Prepare_Catalog -> Find_Prepared -> PresentationService::Spawn -> EffectObject` harness
- compile 1, attach 1, runtime I/O 0
- Catalog/Object/Renderer/Presentation rollback에서 이전 catalog/cache/targets 보존
- 중앙 public API와 common shader 동결
- Product false

## G06B. Source Runtime Program Materializer — `4ffe1102` 폐기, R1 readiness 뒤 R2에서 새로 작성

Source execution receipt의 35 emitter/399 module/629 distribution을 immutable program으로 옮긴다.
`4ffe1102`를 cherry-pick하거나 mutation하지 않는다. R1 readiness가 모두 PASS한 combined head에서
denominator와 fail-closed 요구만 새 구현의 acceptance contract로 재적용하고 generic JSON bag은 runtime
authority로 승격하지 않는다.

추가 합격 조건:

- ParticleParameter 17 occurrence의 name/mode/four-range/fallback/oracle/provenance typed coverage 100%
- PointLight typed component field와 handler receipt coverage 100%
- seed/default closed typed variant coverage 14/14
- READY distribution 629/629 closed evaluator registry join
- exact class -> opcode -> handler -> allowed/required property schema와 consumption receipt
- source record/value, handler SHA, lookup shape, emitter/opcode order, nested key mutation transactional reject
- tracked LF/CRLF에서 동일 canonical program SHA
- Debug/Release actual candidate mutation harness
- runtime execution admission false, Product false

## Wave 2A. Typed executor, Geometry와 Material consumer 병렬

### G07. Typed Cascade Executor — remaining

G06이 최종 compile한 immutable execution IR을 strict consumer로 받아 `CEffectPlayback`의 v14
production simulation이 typed opcode만 소비하도록 바꾼다. G07은 `Effect_CascadeCompiler`를 수정하거나
별도의 IR 승격 경로를 만들지 않는다. G06이 동결한 handler extension point에 Playback capability와
consumption receipt만 등록한다.

필수 opcode/handler 범위:

- Constant, Uniform, ConstantCurve
- standard Float/Vector ParticleParameter의 Direct/Normal/Abs와 네 range
- oracle이 있는 custom EF evaluator의 별도 opcode
- spawn, burst, lifetime, delay, loop, size, rotation, location, velocity, acceleration, color
- local/world/source space와 axis lock
- SubUV, spawn-per-unit, location-on-ground
- TypeData Mesh/Decal/Ribbon/Light
- ActionCue parameter binding과 none binding
- fixed seed, fixed timestep, deterministic sample order
- `Effect_Playback.cpp`의 Mesh StartSize hidden `x0.01`을 제거하고 signed dimensionless axis reorder만 유지

합격:

- 399 ordered reference, 1,434 property, 1,572 leaf, 629 distribution handler receipt
- Playback raw `SourceRecipe.Modules`, 문자열 class/property scan 0
- unresolved/custom payload read 0
- fixed seed/time repeated run deterministic
- failed compile/stage에서 이전 playback/prepared revision 보존
- Debug/Release actual candidate harness
- Product false

### G08. GeometryBinding Runtime Consumer — remaining

G02/G05-G 결과를 기존 `CModel -> CMesh -> CMaterial` 경로와 G06 extension point에 연결한다.

필수 작업:

- expected tuple `{assetId, payloadHash, metadataIdentity, geometryPreScale=0.01,
  DIMENSIONLESS_AXIS_REORDER_ONLY}`와 decoded bytes 대조
- vertex와 embedded/derived bounds에 preScale 정확히 한 번 적용
- prepared model cache identity에 tuple 전체 포함
- G07에서 hidden `x0.01`이 제거됐음을 검증하고 signed dimensionless axis reorder와 carrier preScale이
  정확히 한 번씩만 적용되는지 확인
- Sprite/Decal cm-to-m 경계와 분리
- tangent-W/COLOR sidecar를 Mesh family shader가 소비할 immutable vertex binding까지 전달

합격:

- 7 carrier / 13 Mesh occurrence combined magnitude numeric oracle
- cache collision, wrong hash, double/zero/nonfinite scale, final bounds mutation 거부
- failed load/prewarm에서 prepared cache residue 0
- source fidelity와 external authentication 상태 보존
- Product false

### G09. Material Recipe Compiler and Evaluator — remaining

G03/G05-M의 27 recipe를 G06 extension point가 소비할 immutable MaterialBinding으로 compile한다.

필수 작업:

- exact scalar/vector/texture input과 sampler/render-state payload
- source-exact input과 reconstructed arithmetic family의 분리
- 23 family별 stable evaluator ID/version과 implemented state
- Add/Multiply/Panner를 포함한 실제 관측 연산을 typed evaluator/HLSL로 구현
- static permutation, blend/depth/cull/default/sampler는 source 또는 G05-M oracle이 있는 값만 compile
- strict sampler 72(instance 71 + parent 1)와 omitted default를 공통 fallback으로 채우지 않음

합격:

- 27 recipe / 34 occurrence handler consumption
- per-family independent numeric sample과 mutation test
- unknown expression/input/sampler/render state 실행 0
- static/render/cull/default/sampler execution blocker 0
- reconstructed graph를 `SOURCE_EXACT`로 승격한 행 0
- Product false

## G10. Renderer family와 Effect Tool 병렬 — remaining

G07-G09 통합·독립 감사 뒤 G06의 중앙 Runtime Authority/Foundation API를 다시 동결하고 family lane을
생성한다.

G06 Foundation이 이미 소유하고 family가 변경하지 않는 계약:

- compiled render packet
- common particle attributes와 CPU/GPU upload layout
- immutable geometry/material binding handles
- renderer family dispatch interface
- central dispatcher와 common shader binding slot
- family 구현의 별도 파일/project/filter 경계

중앙 header, dispatcher, common shader는 동결한다. family lane은 자기 파일과 자기
shader만 수정한다.

병렬 family:

| lane | 분모 | numeric/structural 종료 증거 |
|---|---:|---|
| Mesh + Sprite | 13 + 16 | size/rotation/local-space/lifetime, geometry/material binding, tangent/COLOR input, double-scale mutation |
| Decal + Ribbon | 3 + 1 | projection, signed source space, segment ordering, spawn-per-unit, trail lifetime, invalid topology rollback |
| Light + ScreenPost | 1 + 1 | exact child Brightness/flags, current inherited default 분리, `LIGHT_DESC` radius/falloff/color/brightness 전달, deferred attenuation numeric oracle, post weight/lifetime, unsupported recipe rejection |
| Effect Tool | 35 inspection rows | Source Evidence/IR read-only tree와 Draft/Product Play 경계 |

Effect Tool 계약:

- v14 `SOURCE_CONTRACT`는 read-only이며 Save, Apply, resource replace, execute를 비활성화한다.
- Source tree는 7 System -> 35 Emitter -> 35 selected LOD -> 399 ordered module -> 629 distribution 순서다.
- authored reconstruction만 Draft Preview와 Save를 사용한다.
- Product Play는 Authored 파일을 disk에서 다시 load하지 않고 exact catalog prepared revision을 attach한다.
- Complete/family/occurrence solo는 같은 IR pointer의 transient mask이며 compile/model/texture I/O가 0이다.
- Tool 전용 두 번째 renderer나 Catalog 우회를 만들지 않는다.

각 family frozen commit은 독립 감사를 받은 뒤 Integration이 `cherry-pick -x`한다. family merge 뒤 Tool은
exact prepared attach/no-I/O test만 작은 후속 commit으로 재검증한다.

G10 통합 종료 조건에는 Renderer와 Presentation의 raw `eKind`, raw SourceRecipe/module/default scan 0을
포함한다. compiled family enum과 typed render packet 이외의 dispatch는 v14 Product path에서 거부한다.

## Gate 10B. Actual Artist Derived Artifact 단일 생성

G05-P1은 schema/publisher code만 만들었으므로 actual Artist artifact 생성은 Source semantic,
Geometry/Material contract와 resource binding, G06 execution compiler, G07-G10 handler/renderer의 final
hash가 모두 동결된 뒤 Integration이 한 번만 수행한다.

생성 순서:

1. final v14 Source Contract와 Source semantic reconstruction receipt를 읽는다.
2. final Geometry/Material contract와 실제 Resources binding hash를 결합한다.
3. G06 offline compiler가 v14-derived immutable execution IR과 compiled receipt를 생성한다.
4. G05-P1 generator가 같은 six-hash identity를 가진 v13 derived authoring/Assembly carrier를 생성한다.
5. publisher가 v13 carrier와 compiled artifact의 identity/revision 일치를 검증해 catalog format 3을
   임시 stage하되 Product bit와 기존 catalog publish는 계속 false/미변경으로 둔다.

실행 의미의 단일 권위는 3번의 compiled IR이다. v13 carrier의 raw Assembly/SourceRecipe는 Catalog ID,
Effect Tool Draft/inspection과 provenance 연결에만 사용하며 Playback/Renderer가 실행하지 않는다.
execution blocker, unconsumed handler, resource hash mismatch 또는 six-hash 불일치가 하나라도 있으면
두 artifact와 catalog stage를 전부 rollback하고 이전 Product를 유지한다. 일부 hash만 바꿔 반복
재생성하지 않고 최종 입력 묶음 전체를 한 transaction으로 생성한다.

Gate 10B 종료 조건:

- actual Artist v13 carrier와 compiled IR/receipt의 six-hash identity가 동일
- 399/1,434/1,572/629 consumption receipt와 renderer 13/16/3/1/1/1이 compiled artifact에 고정
- actual Resources 7 WModel과 required DDS expected identity 일치
- raw v13 runtime semantic consumer 0
- Product false, 기존 published catalog/cache/targets 미변경

## G11. Artist F Admission Oracle와 Product publish — remaining

Integration이 새로운 feature를 만들지 않고 전체 oracle만 실행한다. expected 값은 compiler/renderer가
자기 출력으로 다시 만든 값이 아니라 raw evidence, 독립 decoder, hand-authored immutable golden 또는
native numeric oracle에서 온다.

각 35 occurrence row는 다음을 기록한다.

- source fidelity와 원본 blocker
- artifact binding integrity
- execution admission과 evaluator ID/version
- source/compiled document identity
- geometry/material binding과 hash
- selected LOD와 ordered opcode coverage
- renderer family
- fixed seed/timestep/ActionCue input/sample time
- expected/actual/tolerance
- Product blocker와 최종 decision

전체 consumption gate는 분모를 축소하지 않는다.

```text
ordered module reference 399/399
top-level property       1,434/1,434 consumed or verified-irrelevant
primitive leaf           1,572/1,572 consumed or verified-irrelevant
distribution             629/629 compiled or verified-irrelevant
unknown opcode           0
unknown property         0
unconsumed property      0
silent ignored native/default field 0
```

최종 자동 gate:

```text
Source contract shallow/deep and all mutation tests
Geometry actual 7-carrier Debug/Release C++ oracle and legacy corpus
Material 27/34 shallow/deep raw UPK/DDS/manifest oracle
Compiler/Executor Debug/Release actual candidate oracle
Six renderer family 35-occurrence numeric packet/state oracle
Effect Tool read-only/Draft/Product revision and no-I/O transaction oracle
Engine Debug/Release
UpdateLib Debug/Release
ClientFrontendHarness Debug/Release
Client Debug/Release
focused Artist ProjectAudit checks all PASS
full Invoke-ProjectAudit executed with the canonical physical Resources root
all Artist/Effect-related full-audit failures fixed
unrelated baseline failure ID/detail delta 0
JSON/XML parse
git diff --check
conflict marker and unmerged path 0
residual Client/Server process and listener check
```

Product predicate가 35/35 true일 때만 Artist F cue를 Catalog에 atomic publish한다. partial Product나
missing occurrence 축소는 허용하지 않는다. publish 뒤 exact revision의 load/prewarm/Spawn을 다시 실행하고
실패하면 이전 catalog/cache/targets를 유지한다.

publish 대상은 Gate 10B의 verified v13 derived authoring/Assembly identity carrier와 runtime catalog
format 3 compiled entry다. v14 Source Contract를 drawable로 바꾸거나 Catalog가 v14를 직접 읽게 하지
않으며 v13 raw semantics도 실행하지 않는다. publisher, catalog stage, prepared compiler와 runtime
Spawn은 같은 six-hash identity와 compiled IR pointer를 확인해야 한다.

G00에서 canonical Resources root를 지정해 full ProjectAudit baseline report를 생성하고 failure ID/detail을
고정한다. Artist/Effect와 직접 관련된 baseline failure는 G11까지 전부 닫는다. Map/Character 등 이 기능과
무관한 baseline failure가 남으면 full audit 전체를 PASS라고 쓰지 않고, exact same failure 집합이며 신규
failure가 0이라는 비교 증거를 남긴다.

## Build lease와 commit 규칙

Integration Captain이 전역 full-build lease 하나를 관리한다.

- Engine, UpdateLib, Client full build는 한 번에 한 worktree만 실행한다.
- Debug와 Release도 직렬화한다.
- Engine public 변경은 같은 SHA/worktree에서 `Engine -> UpdateLib -> Client`를 실행한다.
- 다른 worktree의 `EngineSDK`, `.lib`, generated header를 복사해 PASS를 만들지 않는다.
- Python/unit은 output path가 겹치지 않을 때만 병렬 실행한다.
- `__pycache__`, temp JSON, build/intermediate, ignored EngineSDK를 stage하지 않는다.
- lane commit은 하나의 검증 단위이며 frozen audit PASS 뒤 rebase하지 않는다.
- correction은 같은 lane의 후속 commit으로 만들고 새 SHA를 다시 감사한다.
- lane branch는 checkpoint push 용도이며 main에는 최종 Integration PR 하나만 연다.

## 도화가 F 이후 확장

G11의 Product 35/35와 동일 prepared runtime path가 닫히기 전에는 다른 class와 Valtan occurrence를
Product로 확장하지 않는다. 완료 뒤에는 production code에 Artist-specific count/switch를 추가하지 않고
같은 generic compiler, material evaluator, geometry binding, six-family renderer와 Tool contract를 사용한다.

확장 순서:

```text
도화가 F golden fixture
-> Artist 9-skill canary (31470 제외)
-> 차원술사
-> 워로드
-> 창술사
-> 발탄 Particle/Decal/Trail/Material/Camera occurrence
-> 전체 corpus regression과 공통 renderer 최적화
```

Artist의 `7/35/399/629`, renderer `13/16/3/1/1/1`, material `27/34`는 fixture harness에만 남긴다.
production compiler/renderer/catalog에 이 숫자를 hardcode하지 않는다.
