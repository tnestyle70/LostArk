# 2026-08-10 Artist 31470 F Material Evidence Contract Result

## 결과

도화가 F `31470`의 Material 첫 slice를 `27 recipe / 34 occurrence` typed evidence contract로
닫았다. 이 결과는 Material 입력 근거를 닫은 것이며 실행 셰이더 복원 완료가 아니다. 모든 arithmetic
family는 계속 `RECONSTRUCTED_GRAPH`, evaluator는 `implemented=false`, static permutation selection과
full render/cull state는 unresolved다. 따라서 executable/Product admission은 recipe와 occurrence 모두
0이다.

Client runtime, HLSL, `Effect_DocumentRenderer`, Effect Tool, source contract, distribution closure,
geometry/WModel은 수정하지 않았다.

## 구현 파일

- `Tools/LevelPlacementExtractor/extract_artist_31470_material_render_state.py`
  - raw UPK 19개에서 source/base export 48개, graph expression export 925개, Texture2D export 4개를
    직접 해독한다.
  - package raw SHA/size, export index/path/class, serial offset/size/SHA, tagged-property offset/type/value,
    encoded value bytes/SHA와 record SHA를 생성한다.
  - omitted field는 `OMITTED_FROM_EXPORT / UNRESOLVED_DEFAULT_PROVENANCE`로 남긴다.
  - tracked generator/parser는 EOL-canonical source SHA, UPK는 raw artifact SHA domain을 쓴다.
- `Data/Effects/Imported/Artist/Materials/skill.31470.material-render-state-evidence.receipt.json`
  - 27 binding, 48 unique raw source/base export, 925 graph expression, 4 Texture2D, 19 raw package의
    generated/self-digested format 3 receipt다.
- `Tools/LevelPlacementExtractor/build_artist_31470_material_evidence_contract.py`
  - active inventory, material closure, exact DDS receipt, raw render-state receipt를 pure builder로 join한다.
  - exact input/sampler/render/partial-cull과 reconstructed graph를 필드별로 분리한다.
  - blocker set이 비어 있을 때만 admission을 열 수 있게 하고 이번 slice에서는 0을 유지한다.
- `Data/Effects/Imported/Artist/Materials/skill.31470.typed-material-evidence-contract.json`
  - 27 recipe, 34 occurrence, 23 arithmetic family, 4 exact sampler binding의 generated format 2 contract다.
- `Tools/LevelPlacementExtractor/test_build_artist_31470_material_evidence_contract.py`
  - 실제 checked evidence를 기준으로 pure builder와 deep raw regeneration mutation 28개를 검증한다.
- `Tools/ProjectAudit/Test-Artist31470MaterialEvidenceContract.ps1`
  - shallow canonical/self check와 deep raw UPK/DDS/external manifest check를 제공한다.
- `Tools/ProjectAudit/Invoke-ProjectAudit.ps1`
  - `effect.artist-31470-material-evidence-contract` check를 추가했다.

## 확정 분모

| 항목 | 결과 |
|---|---:|
| 전체 rendered effect occurrence | 35 |
| engine builtin Material occurrence | 1 |
| typed Material occurrence | 34 |
| unique Material recipe | 27 |
| source MIC / raw Material | 25 / 2 |
| scalar override | 342 |
| vector override | 19 |
| direct texture override | 71 |
| exact direct sampler / unproven direct sampler | 3 / 68 |
| exact parent-default sampler | 1 |
| exact sampler 전체 | 4 |
| surviving parent default / static-switch parent default | 297 / 94 |
| raw graph expression export | 925 |
| unique arithmetic family | 23 |
| cooked-stripped null expression slot | 1,803 |
| unresolved graph edge | 502 |

`LightParticle` occurrence 자체는 렌더된다. 다만 `enginematerials.defaultparticle`는 source Material
recipe가 아닌 engine builtin이므로 27 recipe와 34 typed Material occurrence 분모에서만 분리했다.

초기 audit의 343 scalar / 20 vector는 데이터 차이가 아니었다. material payload가 없는 builtin row에
PowerShell `@($null).Count`를 적용하면 1이 되는 특성 때문에 각각 phantom 1이 더해졌다. Python
generator는 `material is not None`과 list type을 먼저 검증하며 authoritative 분모를 342/19로 고정한다.
Stable field identity는 `(sourceMaterialPath, fieldKind, serializedArrayIndex,
casefold(parameterName), bindingOrigin)`이다. blank/duplicate parameter는 0이다.

## Material·occurrence identity closure

- canonical `sourceMaterialPath`는 raw Material object path의 exact suffix, physical package SHA,
  export index, raw export evidence ID와 함께 고정한다.
- MIC의 raw `Parent` reference는 selected parent graph의 exact physical package SHA, export index,
  object path와 결합한다. 유효한 두 Material row의 canonical path를 맞바꾸거나 parent graph만
  맞바꾸는 실제-UPK 재생성 공격은 모두 실패한다.
- 34 occurrence는 `activeElementId`, cue, renderer type, source system/emitter, source material path,
  recipe ID, raw Material/base graph/family evidence를 하나의 per-occurrence identity로 갖는다.
  Validator는 각 identity와 aggregate digest를 다시 계산하며 sealed occurrence swap도 거부한다.

## raw render-state 근거

23 unique base Material/DecalMaterial export에서 다음 explicit field를 해독했다.

| explicit field | unique base export count |
|---|---:|
| `BlendMode` | 23 |
| `LightingModel` | 21 |
| `TwoSided` | 9 |
| `bDisableDepthTest` | 1 |
| `OpacityMaskClipValue` | 1 |
| `bUseOneLayerDistortion` | 1 |

25 MIC 중 `OverrideTwoSided` explicit은 13개, `bHasStaticPermutationResource` explicit은 24개다.
Base `TwoSided`와 MIC override를 적용해 partial-cull exact recipe는 18개지만 full cull exact recipe는
0이다. Omitted `TwoSided`, depth, lighting 값은 false나 class default로 만들지 않았다.
93개 explicit render/static 관련 tagged field는 decoded value만 신뢰하지 않고 encoded bytes/SHA,
record SHA, export evidence와 고정 fixture를 함께 검사한다. 원래 raw hash를 유지한 채 valid `BlendMode`나
`TwoSided` 값만 바꾸는 receipt/contract 공격도 거부한다.

`bHasStaticPermutationResource`는 resource 존재 flag일 뿐 selected static parameter 값이 아니다.
Parent graph의 surviving switch default 94개도 `PARENT_DEFAULT_NOT_INSTANCE_SELECTION`으로 분리했다.
따라서 `SOURCE_EXACT_STATIC_PERMUTATION` recipe는 0이다.

## exact input과 parent default 경계

- 342 scalar, 19 vector, 71 direct texture override는 exact source package/export와 raw parameter-array
  record SHA뿐 아니라 serialized element order/name/value/package reference까지 일치하는
  `SOURCE_EXACT_INPUT`이다.
- surviving parent default 297개와 static-switch parent default 94개는 exact parent package/export,
  raw MIC inheritance edge, raw expression export/path/serial, default 또는 texture property record,
  같은 kind/name의 closer override 부재를 모두 증명한 경우에만 exact다.
- parent default는 항상 `bindingOrigin=PARENT_DEFAULT`를 유지한다. raw Material 자체의 default는
  `SELF_DEFAULT`로 분리한다.
- exact DDS/sampler 4건은 `INSTANCE_OVERRIDE=3`, `PARENT_DEFAULT=1`이다. 네 번째
  `fx_tex_01.fx_c_atypical_016 / dissolve_texture`를 direct override로 세지 않는다.
- 4건 모두 raw Texture2D의 class/export/reference/serial offset·size·SHA와 tagged sampler field를
  DDS receipt에 join한다. Parent-default 1건은 raw expression의 Texture reference도 함께 요구한다.
- 나머지 direct texture override 68개에는 address/filter/color-space 값을 만들지 않고
  `SAMPLER_EVIDENCE_MISSING`을 유지한다.

## arithmetic graph와 admission

Exact `(parent package SHA, Material path)` 기준 family는 23개다. 23/23 모두
`graphProvenance=RECONSTRUCTED_GRAPH`, `sourceExactGraph=false`다. Aggregate cooked evidence는 null
expression slot 1,803개와 unresolved input edge 502개다. 각 family의 evaluator는 별도 stable ID,
`RECONSTRUCTED_ARITHMETIC_FAMILY`, `sourceExact=false`, `implemented=false`를 가진다.
두 aggregate는 closure summary의 복사가 아니라 23개 raw `Expressions` 배열과 925 expression input
reference에서 family별로 다시 계산한다. family 사이에 count를 재분배해 aggregate만 유지하는 변조도 거부한다.
각 `familyId`는 export index를 포함한 exact graph identity 전체와 raw base evidence 전체의 canonical
digest에서 재계산한다. Recipe의 family ID/evidence도 selected graph와 일치해야 하며
sealed exact-identity 또는 recipe-family swap은 실패한다.

Aggregate blocker는 다음 8개다.

- `COOKED_STRIPPED_ARITHMETIC_GRAPH`
- `FULL_CULL_MODE_UNRESOLVED`
- `FULL_RENDER_STATE_UNRESOLVED`
- `PRODUCT_RUNTIME_MATERIAL_COMPILER_UNIMPLEMENTED`
- `RECONSTRUCTED_ARITHMETIC_EVALUATOR_UNIMPLEMENTED`
- `RENDER_STATE_DEFAULT_PROVENANCE_UNRESOLVED`
- `SAMPLER_BINDINGS_INCOMPLETE`
- `STATIC_PERMUTATION_SELECTIONS_UNRESOLVED`

Recipe blocker set은 occurrence에 lossless 상속된다. Recipe/occurrence admission은 blocker count가 0일
때만 true가 될 수 있고 현재 executable/Product는 각각 0/27, 0/34다.

## hash domain과 fail-closed 검사

- generated/repo tracked JSON은 UTF-8 text의 EOL만 LF로 정규화해 exact bytes를 비교한다.
  LF/CRLF는 동등하지만 key reorder, 공백, 숫자 token `1`/`1.0`, JSON type 변화는 stale로 거부한다.
- self digest는 자기 digest field를 제외한 sorted canonical JSON serialization을 사용한다.
- external source-pack manifest는 JSON이어도 raw bytes SHA를 사용한다.
- UPK, DDS, export serial, tagged-property record는 모두 raw bytes SHA를 사용한다.
- duplicate JSON object key, duplicate/blank parameter, missing path/hash/export, parent cycle, invalid typed
  render value, stripped-edge 소실을 모두 거부한다.
- 35개 active element의 `sourceMaterials`와 `materialParameterEvidence.sourceMaterialPath`를 같은 값으로
  검증하고 stable join digest를 고정한다. 27 recipe는 34 typed occurrence에서 모두 사용되며 unused와
  unexpected material은 각각 0이다.

## 검증 결과

### focused unit test

```powershell
cd Tools/LevelPlacementExtractor
python -m unittest -q test_build_artist_31470_material_evidence_contract
```

결과: shallow `Ran 28 tests ... OK (skipped=1)`, deep `Ran 28 tests ... OK`.

공격 fixture는 static flag의 selected permutation 세탁, partial `TwoSided`의 full cull 승격, DDS hash와
sampler origin 변경, parent-default shadowing, extra exact sampler 세탁, parent cycle, cooked stripped edge
제거, reconstructed evaluator의 exact 승격, recipe/occurrence/aggregate blocker loss, occurrence join loss,
canonical Material/parentGraph 실제-UPK swap, independent occurrence identity mutation, sealed occurrence/family
swap, 원래 raw hash를 재사용한 render enum/bool substitution, tracked LF/CRLF와 external raw JSON 차이를 포함한다.

### shallow audit

```powershell
powershell -ExecutionPolicy Bypass -File `
  Tools/ProjectAudit/Test-Artist31470MaterialEvidenceContract.ps1
```

결과:

```text
PASS: Artist F 31470 Material evidence mode=shallow recipes=27 occurrences=34 inputs=342/19/71 samplers=3+1 graphs=23 stripped=1803/502 runtime=false product=false
```

### deep raw audit

로컬 source UPK root, fresh UModel DDS root, source-pack manifest를 세 개 모두 명시해 실행했다.

```text
PASS: Artist F 31470 Material evidence mode=deep recipes=27 occurrences=34 inputs=342/19/71 samplers=3+1 graphs=23 stripped=1803/502 runtime=false product=false
```

Deep audit에서 raw render receipt 27 binding/48 source/base export/925 expression/4 Texture2D/19 package가
`--check` 재생성 일치했고,
4개 DDS와 source Texture2D UPK, external manifest의 raw size/SHA가 모두 일치했다.
같은 deep 실행에서 `fx_e_me_ht_03_4_ma`/`fx_e_pa_fd_18_2_tr` canonical row swap과 parentGraph-only
swap을 실제 source UPK에 대해 재생성했으며 둘 다 fail-closed로 거부됐다.

### 전체 ProjectAudit

`Tools/ProjectAudit/Invoke-ProjectAudit.ps1`도 실행했다. 새
`effect.artist-31470-material-evidence-contract` check는 `passed=true`였고 위 shallow PASS detail을
보존했다. 전체 audit 자체는 base `0a90a1d`에 이미 존재하는 독립 실패 11개 때문에 exit 1이었다.
그중 현재 slice와 직접 겹치는 기존 실패는 stale Artist source-contract candidate이며, 그 밖에는 map
runtime root, Character Select manifest/data visibility, 기존 Effect G09/runtime 문서, WFX/rollout 및 로컬
runtime asset 부재가 포함된다. 첫 실행에서 Material unit-test stderr가 outer PowerShell capture에
ErrorRecord로 전달되던 문제는 focused audit 내부에서 output을 capture하고 exit code로 판정하게 고쳤다.
재실행 report에서 Material check가 통과하고 전체 실패 수가 12에서 기존 11로 돌아온 것을 확인했다.

## 미완료 경계

- MIC native `FStaticParameterSet` selected values 해독과 source-era static permutation 증명
- source-era Engine/EFGame default/archetype 근거를 포함한 omitted render-state closure
- 나머지 direct texture 68개와 필요한 parent texture의 sampler evidence
- 23 arithmetic family evaluator 구현과 cooked stripping을 넘는 numeric oracle
- typed compiler/runtime/HLSL/renderer 소비와 27/27 recipe, 34/34 occurrence execution admission
- Product publish

이 RESULT는 Material evidence contract 첫 slice만 완료로 기록한다. 위 항목이 닫히기 전에는 완전 복원이나
Product material 완료로 판정하지 않는다.

## WIP checkpoint: exact-evidence lineage

후속 integrity review에서 genuine-looking `recordSha256`/`serialSha256` 교환과 closure·receipt 공동
재봉인, recipe input 전체 재배치가 shallow 경계에서 수용되는 반례가 확인됐다. 현재 WIP는 다음 기반만
부분 구현한 중간 체크포인트다.

- 외부 source-pack manifest의 raw byte fixture와 `(physicalPackage, SHA) -> logicalPackage` 정본
- canonical Material path를 `objectPath` 또는 `logicalPackage.objectPath`로 제한하는 경계
- raw export/expression evidence ID 재계산과 instance array decoded projection/lineage 초안
- raw input/render/Texture2D fixture 확장을 위한 builder 구조

아직 parent-default lineage, exact sampler owner, render lineage, recipe composition digest, occurrence 소비,
formatVersion 3 생성물·audit·공격 fixture 연결이 끝나지 않았다. 따라서 이 체크포인트의 generator/unit/deep
audit는 완료 검증으로 기록하지 않으며, 기존 checked contract의 Product admission 0도 열지 않는다. 현재
검증 범위는 두 Python 파일의 `py_compile`과 `git diff --check`뿐이다.
