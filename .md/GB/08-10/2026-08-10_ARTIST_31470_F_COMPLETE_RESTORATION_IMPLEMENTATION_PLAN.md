# 2026-08-10 Artist 31470 F Complete Restoration Implementation Plan

기준일: 2026-08-10

기준 commit: `068fcc2eb2c1008505564a2e97aa9e0abaac70c3`

통합 branch: `codex/artist-f-restoration-integration-v2`

문서 역할: 이 파일은 전체 범위와 G 순서, worktree 소유권, admission predicate를 소유하는 최종
구현 계획서다. 각 mutation lane은 기존 Source/Geometry/Material/Compiler PLAN을 갱신하거나 해당 G의
전용 구현 PLAN을 먼저 만들고, public H/CPP의 세부 선언·전체 코드는 그 lane 문서에서만 소유한다.
같은 코드를 이 master 문서에 복제하지 않는다.

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

`068fcc2`는 네 WIP checkpoint가 main에 병합된 clean tree이지만 green base가 아니다.
기존 checkpoint SHA는 history 추적용이며 새 lane에 cherry-pick하지 않는다.

| 영역 | main에 포함된 checkpoint | 실제 상태 |
|---|---|---|
| Source | `cdb052f` 계열과 후속 재생성 | shallow 52 tests PASS, 15/17 중 semantic ready 9 / blocked 8 occurrence, compiled 0/17, Product false |
| Geometry | `dd0d548` 계열 후속 WIP | self-consistency 7/7, external authentication 0/7, preScale consumer 0/7, 최종 독립 PASS 미완료 |
| Material | `b9212dda` 계열 후속 WIP | checked contract는 27/34 Product0, 현재 generator와 receipt hash 불일치 |
| Compiler | `d93bbd9` | non-executable inspection IR, static audit PASS, Product 0/35, actual current Source adapter 미연결 |

기준 tree에서 재현한 시작 결과는 다음과 같다.

```text
Test-Artist31470SourceContract.ps1
  PASS: 7 cue / 35 element / 399 module order / Product false

Test-Artist31470MaterialEvidenceContract.ps1
  FAIL: render-state receipt generator hash mismatch

Test-EffectCascadeCompiler.ps1
  PASS: non-executable static inspection boundary

ClientFrontendHarness actual Source candidate execution
  clean worktree에 EngineSDK와 새 binary가 없으므로 G04 build lease 뒤 재검증
  직전 merge audit의 known blocker는 provenance/path/reference identity rejection
```

canonical physical Resources root를 지정한 G00 full ProjectAudit는 93 check 중 9개가 실패했다.

| check | G11 판정 |
|---|---|
| `projects.data-source-visibility` | 이번 변경의 새 Data/소스 등록을 포함해 expected/project/filter를 일치시킴 |
| `effect.g09-authoring-world-runtime-boundary` | Runtime Authority/Renderer/Tool에서 닫음 |
| `effect.g09-cross-document-contract` | v13 derived Product와 compiled revision 계약으로 닫음 |
| `effect.artist-31470-source-contract` | standalone PASS와 full-audit stderr 판정 불일치를 Source audit wrapper에서 닫음 |
| `effect.artist-31470-material-evidence-contract` | G03에서 hash mismatch를 닫음 |
| `effect.artist-31470-wmodel-geometry-contract` | G02 build lease에서 harness를 만들고 닫음 |
| `effect.wfx-component-assembly` | F compiled Product 변경으로 새 회귀가 없음을 비교하고 F 범위 failure이면 닫음 |
| `effect.representative-authored-readiness` | 기존 4-class rollout 범위는 baseline으로 분리하고 F 영향이 있으면 닫음 |
| `effect.four-class-authored-clip-product-exact101` | Artist/31210 기존 stage mismatch는 F 완료 뒤 확장 범위로 보존하되 신규 failure/detail 변화 0 |

앞의 여섯 check와 이번 변경으로 detail이 바뀌는 Effect check는 Product publish 전에 PASS여야 한다.
후속 4-class 확장 check는 정확한 baseline ID/detail을 유지하고 F 변경으로 악화되지 않아야 한다.

현재 `C:/Users/user/Desktop/LostArk`는 다른 기능의 대규모 dirty worktree다. 이 작업은 그 tree를
stage, stash, reset, clean하거나 파일 전체 ours/theirs로 해결하지 않는다. 모든 mutation lane은
통합 branch의 exact Gate commit에서 새 worktree를 만든다.

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

Product predicate는 마지막에 수동으로 정하지 않는다. 네 축, 네 blocker set, 허용 fidelity matrix로
G00에서 고정하고 G11은 같은 predicate를 평가만 한다. 서로 다른 blocker set을 하나의 union으로
합쳐 source exact가 아닌 행을 영구 Product false로 만드는 방식은 금지한다.

## 세션과 worktree 구조

사용자에게 보이는 역할은 세 개만 유지한다.

| 세션 | 역할 | mutation 권한 |
|---|---|---|
| Source Closure | source evidence, resolver, generated Source output 단일 소유 | Source lane만 |
| Pipeline Implementation / Integration | Geometry, Material, Compiler, Executor, Renderer, Tool 내부 lane 조정과 통합 | 각 내부 worktree와 integration conflict commit |
| Independent Review | frozen commit/tree의 재현과 PASS/BLOCK | read-only |

계획의 G 번호는 새 사용자 채팅 이름이 아니라 위 세션 안에서 실행하는 bounded job이다. 전역 active
worker 예산은 최대 4다. mutation worker 4개가 동작할 때 Integration과 Review는 checkpoint 처리만
하고, 감사가 시작되면 해당 mutation worker를 멈춘다.

통신은 다음 네 종류만 허용한다.

1. 최소 public schema/typed field 요청
2. frozen commit SHA와 실제 실행한 검증
3. 재현 가능한 PASS/BLOCK의 명령, 기대, 실제
4. 실제 integration conflict 또는 full-build lease 요청

진행 중 추측, live dirty diff 중계, 다른 lane 내부 구현 로그는 보내지 않는다.

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

## G00. Known-red base와 public handshake 동결

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

### G02. Geometry Evidence Final

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

### G03. Material Evidence Final

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
- 27 static/full-render/full-cull/default blocker, 25 sampler blocker, 23 graph/502 edge를 각각
  source-closed 또는 reconstruction-ready로 만들며 값이 없는 행을 공통 fallback으로 채우지 않음

합격:

- recipe row 27/27, occurrence 34/34, unused/unexpected recipe 0
- sampler exact 3 instance + 1 parent, direct unproven 68
- graph family 23, null 1,803, unresolved edge 502, Source-exact graph 0
- shallow/deep generator, raw UPK/DDS/manifest audit와 mutation tests
- 23 family의 independent expected sample 출처와 아직 구현되지 않은 evaluator를 별도 기록
- evidence commit에서는 evaluator implemented 0과 Product false를 정직하게 유지하되,
  Gate 1 실행-readiness matrix에서 owner 없는 material blocker 0

### G04. Generic Compiler Core

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

## Gate 1. Frozen audit, 통합, Source 단일 재생성

각 G01-G04 lane은 stable commit을 push하고 mutation을 멈춘다. Independent Review는 commit SHA 하나만
검사하고 read-only PASS/BLOCK을 보낸다. 구현 helper와 같은 parser만 사용한 자기확인은 PASS가 아니다.

통합 순서:

1. G02 Geometry frozen PASS commit을 `cherry-pick -x`한다.
2. G03 Material frozen PASS commit을 `cherry-pick -x`한다.
3. G01 Source code-only frozen PASS commit을 `cherry-pick -x`한다.
4. Source Closure가 통합 head의 직접 dependency로 candidate/receipt/registry/header를 단 한 번 재생성한다.
5. G04 generic Compiler frozen PASS commit을 `cherry-pick -x`한다.
6. 통합 conflict는 별도 Integration commit으로 해결하고 lane PASS SHA를 바꾸지 않는다.
7. 새 EngineSDK와 Debug/Release harness를 이 exact integration SHA에서 만든다.

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

### G05-S. Source Semantic Reconstruction Readiness

G01에서 남은 standard/native/default/seed/custom evaluator 행을 actual regenerated candidate 기준으로
다시 계산한다. 각 399 module과 1,434 top-level property는 실행 handler가 소비하거나 독립적으로
실행 무관임을 증명해야 한다. target007/014 custom EF evaluator, external native-tail, seed와 local-space,
Decal/Ribbon/Post/Light default에 evaluator ID와 numeric oracle을 부여한다. current-only evidence는
source exact로 승격하지 않는다.

### G05-G. Geometry Candidate Cook and Resource Provisioning

G02의 7개 WModel 1.1 candidate를 deterministic cook하고 expected tuple을 생성한다. candidate는 먼저
임시 staging directory에서 검증한 뒤 팀장 관리 물리 root인
`C:/Users/user/Desktop/LostArk/Client/Bin/Resources/Effect/Artist/Meshes`의 정확한 7개 target만
backup -> atomic replace한다. 실패하면 기존 7개를 복원한다. Git worktree의 Resources 유무를 제품
payload 존재 증거로 사용하지 않고 test process의 `LOSTARK_RESOURCE_ROOT`를 위 물리 root로 고정한다.

### G05-M. Material Reconstruction Oracle and Texture Provisioning

G03에서 획득한 current Engine/EFGame CDO, static parameter, sampler, shader cache, raw expression evidence로
27 recipe의 static/render/cull/default/sampler와 23 arithmetic family expected numeric sample을 닫는다.
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

## G06. Runtime Authority Foundation

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

## Wave 2A. Typed executor, Geometry와 Material consumer 병렬

### G07. Typed Cascade Executor

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

### G08. GeometryBinding Runtime Consumer

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

### G09. Material Recipe Compiler and Evaluator

G03/G05-M의 27 recipe를 G06 extension point가 소비할 immutable MaterialBinding으로 compile한다.

필수 작업:

- exact scalar/vector/texture input과 sampler/render-state payload
- source-exact input과 reconstructed arithmetic family의 분리
- 23 family별 stable evaluator ID/version과 implemented state
- Add/Multiply/Panner를 포함한 실제 관측 연산을 typed evaluator/HLSL로 구현
- static permutation, blend/depth/cull/default/sampler는 source 또는 G05-M oracle이 있는 값만 compile
- 68 direct sampler와 omitted default를 공통 fallback으로 채우지 않음

합격:

- 27 recipe / 34 occurrence handler consumption
- per-family independent numeric sample과 mutation test
- unknown expression/input/sampler/render state 실행 0
- static/render/cull/default/sampler execution blocker 0
- reconstructed graph를 `SOURCE_EXACT`로 승격한 행 0
- Product false

## G10. Renderer family와 Effect Tool 병렬

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

## G11. Artist F Admission Oracle와 Product publish

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
-> 차원술사
-> 창술사
-> 워로드
-> 발탄 Particle/Decal/Trail/Material/Camera occurrence
-> 전체 corpus regression과 공통 renderer 최적화
```

Artist의 `7/35/399/629`, renderer `13/16/3/1/1/1`, material `27/34`는 fixture harness에만 남긴다.
production compiler/renderer/catalog에 이 숫자를 hardcode하지 않는다.
