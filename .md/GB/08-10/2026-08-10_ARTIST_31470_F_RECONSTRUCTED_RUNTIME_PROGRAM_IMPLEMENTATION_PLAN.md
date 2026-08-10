# Artist F 31470 F Reconstructed Runtime Program 구현 계획

날짜: 2026-08-10

branch: `codex/artist-f-reconstructed-integration-v1`

기준 commit: Runtime Authority foundation `38ebe7cf7dceb5054bde93812907173cc0f98c67` 위에
Geometry, Source, Material frozen evidence 14개 commit을 결합한 tree
`e7116d5c352bc6846e6c10681df503176b480788`

폐기 checkpoint: `4ffe1102ed9cf3e21f669da292fac1f143e18d8f`

## 목표와 종료 증거

사용자가 승인한 `RECONSTRUCTED_APPROVED_V1` 정책과 frozen Source, Material, Geometry evidence를
하나의 immutable typed runtime program으로 materialize한다. historical fidelity는 끝까지
`sourceExact=false`로 보존하고, 실행 blocker는 row별 policy와 구현 capability receipt가 존재하는
경우에만 해소한다.

이 G의 종료 증거는 다음과 같다.

```text
emitter                         35/35
ordered module                  399/399
top-level property              1,434/1,434
primitive leaf                  1,572/1,572
distribution                    629/629
seed policy                     14/14
implicit default                14/14
PointLight field                8/8
Material recipe/occurrence      27/34
Material policy row             255/255
Geometry carrier/use            7/13
unknown/ownerless/silent row     0
sourceExact                      false
runtime execution admission      false until all handler receipts are integrated
Product admission                false
```

Python builder와 Debug/Release C++ parser가 같은 denominator, order, owner, enum, value variant,
blocker union을 재도출해야 한다. self digest를 다시 봉인한 unknown field, row swap, owner swap,
fidelity promotion, stale input SHA, EOL 차이는 fail-closed해야 한다.

## 현재 실측과 금지 경계

- Runtime foundation은 format-3 derived entry의 immutable identity와 execution contract만 소유하며
  typed payload는 아직 materialize하지 않는다.
- Source frozen receipt는 35 emitter, 399 module, 1,434 property, 1,572 leaf, 629 distribution을 보존한다.
  29 blocked module과 custom distribution 3개는 reconstructed capability receipt 전에는 읽지 않는다.
- Material frozen receipt는 23 evaluator, 27 recipe, 34 occurrence, 255 policy row를 보존한다.
  255행 모두 explicit reconstructed value/capability가 결합되기 전에는 실행하지 않는다.
- Geometry frozen binding은 carrier 7개를 소유하지만 source Mesh occurrence 13개와의 use join 및
  runtime preScale consumer는 아직 없다.
- `4ffe1102`의 free-form literal bag, free-form evaluator ID, canonical JSON seed/default, 6-field
  PointLight projection, checkout raw SHA identity는 재사용하지 않는다.
- Product path에서 raw SourceRecipe, class prefix/suffix alias, unknown field skip, default texture/state,
  Mesh hidden scale 보정은 금지한다.

## 변경 파일과 역할

### `Tools/EffectPipeline/build_artist_31470_reconstructed_runtime_program.py`

새 generic offline builder다. strict duplicate-key JSON loader로 frozen input과 approval/capability receipt를
읽고 exact key/type/set/order/owner join을 검증한다. 입력 admission bool을 신뢰하지 않고 program의
artifact binding과 execution blocker union을 재계산한다.

### `Tools/EffectPipeline/test_build_artist_31470_reconstructed_runtime_program.py`

actual frozen input을 사용하는 mutation test다. 399/629 denominator, ParticleParameter typed fields,
PointLight 8행, seed/default variants, Material 255행, Geometry 7/13행, policy/capability/blocker propagation과
LF/CRLF parity를 공격한다.

### `Data/Effects/Imported/Artist/Candidates/skill.31470.reconstructed-runtime-program.candidate.json`

최종 builder가 생성하는 immutable offline input이다. generated receipt나 C++ parser가 이 파일의 bool을
그대로 신뢰하지 않고 모든 section digest와 admission predicate를 재계산한다. capability가 덜 결합된
중간 candidate는 Git에 기록하지 않는다.

### `Client/Public/Effect_RuntimeAuthority.h`

기존 derived catalog identity에 closed runtime program enum/variant/row struct와 parser entry를 추가한다.
public struct는 실행 데이터를 소유하지만 Playback 상태, GPU resource, mutable authoring document는
소유하지 않는다.

### `Client/Private/Effect_RuntimeAuthority.cpp`

program JSON을 `parse -> validate -> stage -> commit`한다. 모든 nested object의 exact key/type를 확인하고
parent/owner/order/row-set digest와 blocker union을 재도출한 뒤 local staged shared pointer만 반환한다.
실패 시 caller의 기존 pointer와 revision을 변경하지 않는다.

### `Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp`

actual Artist candidate를 Debug/Release parser에 넣고 identity, denominator, typed coverage, Product false를
검사한다. source+coverage 동시 reseal, valid handler swap, value-kind 변경, stale policy, A/B identity,
transaction rollback을 실제 C++ parser에서 공격한다.

### `Tools/ProjectAudit/Test-Artist31470ReconstructedRuntimeProgram.ps1`

Python generator `--check`, mutation test, Debug/Release harness와 tracked hash/EOL 검사를 한 focused gate로
묶는다. `Invoke-ProjectAudit.ps1`에는 이 check를 정확히 한 번 등록한다.

### PLAN과 RESULT

이 문서는 구현 전 범위와 검증 계약을 소유한다. RESULT는 실제 diff, 실행한 build/harness, 남은
handler/renderer/Product blocker와 수동 미검증을 분리해 기록한다.

## typed section 계약

program root는 exact input artifact manifest, emitter/module/property/leaf/distribution, seed/default,
PointLight, Material, Geometry, handler registry, policy binding, execution contract를 각각 독립 section으로
가진다. stable ID와 row SHA는 owning parent ID와 ordered field projection에서 재도출한다.

Distribution은 inline raw 612, float parameter 8, vector parameter 5, float curve 1, blocked EF multiply 3의
closed variant다. ParticleParameter는 parameter name, mode, 네 range, constant, binding, ActionCue index/offset,
fallback, oracle/provenance를 typed field로 소유한다.

Seed 14행, implicit default 14행, PointLight 8행은 canonical JSON string이 아니라 closed struct/enum으로
materialize한다. LightGuid와 LightMapGuid 두 행은 `VERIFIED_IRRELEVANT`와 irrelevance oracle ID를 가진다.

Material은 23 evaluator, 27 recipe, 34 occurrence, ordered input 729, static 94, render state 162,
policy row 255를 owner 기준으로 양방향 join한다. reconstructed evaluator를 `SOURCE_EXACT`로 승격하지 않는다.

Geometry는 carrier 7과 Mesh use 13을 분리한다. use row는 source emitter/TypeDataMesh/source mesh object를
carrier binding에 결합하고 size semantics를 `DIMENSIONLESS_AXIS_REORDER_ONLY`로 고정한다.

## admission과 rollback

artifact binding admission은 모든 input canonical SHA, schema, stable ID, owner join, row SHA가 일치할 때만
true다. Source, Material, Geometry execution admission은 각 domain의 capability/consumer receipt와 blocker가
0일 때만 true다. runtime execution과 Product admission은 builder 입력으로 받을 수 없으며 predicate에서
재도출한다.

어느 검증에서든 실패하면 candidate, existing prepared pointer, catalog revision, cache를 교체하지 않는다.
이 G에서는 Playback/Renderer를 연결하지 않으므로 최종 output의 runtime execution과 Product는 false다.

## 구현 순서와 검증

1. approval, Source capability, Material policy receipt의 frozen schema를 결합한다.
2. Python pure builder와 actual-input mutation test를 먼저 통과시킨다.
3. complete candidate를 한 번 생성하고 `--check`로 deterministic equality를 확인한다.
4. C++ closed structs/parser를 추가하고 project/filter 등록을 확인한다.
5. ClientFrontendHarness Debug/Release actual candidate와 mutation suite를 실행한다.
6. focused ProjectAudit, JSON/XML parse, `git diff --check`를 실행한다.
7. independent frozen-tree PASS 뒤에만 R3 Playback consumer를 시작한다.

Engine public header는 이 G에서 수정하지 않는다. Client와 ClientFrontendHarness만 Debug/Release로
직렬 빌드하며 다른 worktree의 EngineSDK/compiled output을 PASS 근거로 사용하지 않는다.
