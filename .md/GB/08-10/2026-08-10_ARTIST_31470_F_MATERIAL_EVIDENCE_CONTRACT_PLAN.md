# 2026-08-10 Artist 31470 F Material Evidence Contract Plan

## 목표

도화가 F `31470`의 실제 렌더 대상 `27 material recipe / 34 occurrence`를 하나의 typed
Material evidence contract로 고정한다. 이 단계는 원본 입력과 실행 가능한 셰이더를 같은 것으로
취급하지 않는다. 정확한 UPK 입력, DDS/sampler, static permutation, render state, cull, cooked-stripped
산술 그래프의 fidelity를 필드별로 분리하고 blocker가 하나라도 있으면 execution/Product admission을
열지 않는다.

## 실측 분모

- active element 35개 중 `LightParticle` occurrence는 렌더되지만,
  `enginematerials.defaultparticle` 1개는 source Material recipe 분모에서 제외하는 engine builtin이다.
- 렌더 대상 occurrence는 34개, unique material은 27개다.
- 27개는 `MaterialInstanceConstant` 25개와 raw `Material` 2개다.
- instance scalar override 342개, vector override 19개, non-null texture override 71개다.
- 기존 343/20 관찰은 builtin row의 누락 필드를 PowerShell `@($null).Count == 1`로 센 오류다.
- exact DDS/sampler binding은 4개지만 origin은 instance override 3개와 parent default 1개다.
  따라서 direct texture override 71개 중 exact sampler는 3개, sampler-unproven은 68개다.
- exact `(parent package SHA, Material path)` 기준 arithmetic family는 23개다. 이 23개 graph의
  cooked-stripped null expression slot 합계는 1,803개, unresolved input edge 합계는 502개다.
  23/23 모두 `RECONSTRUCTED_GRAPH`이며 Source-exact graph admission은 금지한다.

## fidelity 경계

| 등급 | 허용 근거 | 금지 경계 |
|---|---|---|
| `SOURCE_EXACT_INPUT` | exact package/export의 identity, scalar/vector/texture override, surviving parent default | cooked-stripped 연산 의미로 승격 금지 |
| `SOURCE_EXACT_SAMPLER` | pinned DDS receipt의 asset/binding별 4건만 | 다른 texture override에 sampler default 전파 금지 |
| `SOURCE_EXACT_STATIC_PERMUTATION` | MIC selected static parameter payload가 해독된 경우만 | `bHasStaticPermutationResource`를 선택값으로 사용 금지 |
| `SOURCE_EXACT_RENDER_STATE` | raw Material export에 명시적으로 직렬화된 필드만 | omitted 값을 false/default로 간주 금지 |
| `SOURCE_EXACT_PARTIAL_CULL` | explicit `TwoSided` 또는 exact MIC `OverrideTwoSided`만 | full cull mode 정확성 주장 금지 |
| `RECONSTRUCTED_ARITHMETIC_FAMILY` | 별도 evaluator ID와 구현 상태 | Source exact graph/HLSL로 승격 금지 |

## 파일별 구현

### `Tools/LevelPlacementExtractor/extract_artist_31470_material_render_state.py`

기존 material closure가 가리키는 27개 source export와 unique base Material export를 raw UPK에서
다시 읽는다. package byte count/raw SHA, export index/path/class, serial offset/size/SHA, tagged-property
stream 범위, 각 선택 필드의 tag/value offset·type·value·raw record SHA를 기록한다. 누락 필드는
`OMITTED_FROM_EXPORT`로 남긴다. tracked parser와 generator의 EOL-canonical source SHA 및 placeholder 기반 재현 command를
receipt에 기록한다. 출력 self digest는 자기 필드를 제외한 canonical JSON SHA다.

### `Data/Effects/Imported/Artist/Materials/skill.31470.material-render-state-evidence.receipt.json`

위 raw extractor의 checked-in derived receipt다. shallow audit은 canonical JSON/self digest와 typed
contract join을 검증하고, deep audit은 명시적으로 받은 source package root에서 raw bytes를 다시
읽어 `--check`한다. 사람 손으로 render-state 값을 작성하지 않는다.

### `Tools/LevelPlacementExtractor/build_artist_31470_material_evidence_contract.py`

pure builder가 active inventory, active material closure, exact DDS receipt, render-state receipt를
입력으로 받는다. `material is not None`과 list type을 먼저 검사하여 builtin null을 분모에서
제외한다. stable field identity는
`(sourceMaterialPath, fieldKind, serializedArrayIndex, casefold(parameterName), origin)`이다.

builder는 다음을 수행한다.

1. 27 recipe/34 occurrence를 case-insensitive stable key로 join한다.
2. package/path/hash/export, parameter blank/duplicate, parent cycle을 거부한다.
3. scalar/vector/texture override와 surviving parent default를 `SOURCE_EXACT_INPUT`으로 분리한다.
   Parent default는 exact parent package/export, exact inheritance edge, 같은 이름의 closer override 부재를
   모두 필드별로 증명하고 `bindingOrigin=PARENT_DEFAULT`를 보존할 때만 포함한다.
4. exact sampler 4건을 `INSTANCE_OVERRIDE=3`, `PARENT_DEFAULT=1`로 검증하고 나머지 68 direct
   texture override에는 sampler 값을 만들지 않는다.
5. parent static-switch default와 MIC selected permutation을 별도 배열에 둔다. 후자는 현재 비어 있고
   `bHasStaticPermutationResource`가 true여도 blocker를 유지한다.
6. explicit render-state 필드와 omitted/default-unproven 필드를 분리한다. partial cull만 허용한다.
7. cooked partial graph와 reconstructed evaluator ID를 분리한다. evaluator는 `implemented=false`다.
8. recipe blocker union을 occurrence에 그대로 상속하고, blocker count가 0일 때만 executable/Product
   admission을 열 수 있게 검증한다. 이번 slice의 admission은 0/27, 0/34다.
9. repo tracked JSON은 UTF-8 byte sequence의 EOL만 LF로 정규화한 exact canonical text hash/check를
   사용한다. key order, 공백, 숫자 표기와 JSON type은 그대로 비교하여 `1`과 `1.0`, key reorder를
   동등 처리하지 않는다. self hash만 정렬 canonical serialization을 사용할 수 있다. 외부 JSON,
   UPK, DDS는 raw hash만 사용한다.

### `Data/Effects/Imported/Artist/Materials/skill.31470.typed-material-evidence-contract.json`

27 recipe와 34 occurrence, 필드별 provenance/fidelity, sampler origin 분모, static/render/graph/evaluator
blocker, aggregate admission을 가진 generated contract다.

### `Tools/LevelPlacementExtractor/test_build_artist_31470_material_evidence_contract.py`

실제 문서 baseline과 pure in-memory mutation을 함께 검증한다. missing/corrupt path/hash, duplicate/blank
parameter, parent cycle, fake static selection, `TwoSided`의 full-cull 세탁, DDS/sampler hash와 origin,
stripped edge blocker, reconstructed evaluator의 exact 승격, blocker-set loss, occurrence join loss,
tracked LF/CRLF equivalence와 raw artifact CRLF mutation 차단을 포함한다.

### `Tools/ProjectAudit/Test-Artist31470MaterialEvidenceContract.ps1`

shallow mode에서 두 generator의 self/check와 unit test, 27/34 및 342/19/71, exact sampler 3+1,
Product 0을 검증한다. deep mode는 source package root, exact DDS root, source-pack manifest를 명시적으로
받아 raw UPK/DDS/external JSON SHA를 다시 검증한다.

### `Tools/ProjectAudit/Invoke-ProjectAudit.ps1`

focused audit를 호출해 `effect.artist-31470-material-evidence-contract` 결과에 연결한다.

## 금지 범위

- Client runtime, HLSL, `Effect_DocumentRenderer`, Effect Tool 변경
- source contract, local-reference/distribution closure, geometry/WModel 변경
- active material closure의 기존 근거를 값 수정으로 보정
- 이미지 캡처·육안 승인·visual approval 상태 변경
- reconstructed evaluator 또는 cooked partial graph의 Source exact 승격

## 완료 조건

- raw render-state receipt가 source UPK deep `--check`로 재생성 일치한다.
- typed generator `--check`가 checkout LF/CRLF에는 무관하지만 다른 byte/token 변화는 거부한다.
- recipe 27/27, rendered occurrence 34/34 join이 닫힌다.
- scalar/vector/direct texture override가 정확히 342/19/71이고 blank/duplicate가 0이다.
- exact sampler는 전체 4, 그중 instance 3/parent default 1이며 direct unproven은 68이다.
- static permutation, full render state, cooked arithmetic evaluator blocker가 유지되어 Product admission은 0이다.
- focused unit tests, ProjectAudit entry, JSON parse, `git diff --check`가 통과한다.
- 구현·검증·RESULT를 하나의 독립 커밋으로 만든다.
