# 2026-08-10 Artist 31470 F Generic Cascade Compiler Core Result

## 결과

도화가 F Source Contract를 실행하지 않고 검사하는 immutable typed Cascade inspection IR을 revision 2로
강화했다. 이 변경은 G04 generic compiler core이며 실행 compiler 완료가 아니다. 결과는 항상
`bExecutable=false`, `bProductAdmission=false`이고 Product admission은 0/35다.

기존 Product Playback, DocumentRenderer, PresentationService, Catalog, EffectObject, 여섯 renderer family,
Geometry/Material runtime consumer는 수정하지 않았다. raw SourceRecipe executor도 그대로이며 이 inspection
IR을 제품 실행 권위로 사용하지 않는다.

## 구현한 계약

- `CEffectCascadeCompiler::Compile_SourceInspection`
  - domain-separated 전체 document serialization hash를 caller identity와 대조한다.
  - System → occurrence/emitter → selected LOD → ordered module → property/distribution 구조를 immutable IR로 만든다.
  - selected LOD의 emitter path/node, selected path/node, array index를 canonical lineage ID와 stable reference에
    결합하고 node reuse, noncanonical suffix, free-form provenance promotion을 거부한다.
  - source execution admission true를 typed class/payload/handler closure 전에 거부한다.
  - raw module literal/distribution payload는 읽지 않고 distribution evidence를
    `bRawPayloadRead=false`, `bExecutionAllowed=false`로 격리한다.
- opcode-specific handler receipt
  - 각 allowed property는 `{property key, storage, handlerFieldId, required, result}` receipt를 가진다.
  - source와 coverage가 함께 만든 unknown property도 handler schema에 없으면 거부한다.
  - required/consumed/handler receipt count를 별도로 재계산하고 raw payload/executable count는 0만 허용한다.
  - source receipt의 property blocker를 IR에 보존한다. decoded/metadata property는 blocker를 금지하고,
    deterministic distribution과 unresolved property는 canonical blocker 집합을 필수로 요구한다.
  - distribution 격리 evidence는 source property blocker에
    `SOURCE_TYPED_DISTRIBUTION_ADAPTER_PENDING`만 합성한 정확한 집합이어야 한다.
- exact class와 alias lineage
  - G00 bridge의 optional `strExactSourceClass/strAliasId`를 typed receipt로 보존한다.
  - transport가 없으면 `RECEIPT_NORMALIZED_ONLY`와 pending blocker를 유지한다.
  - exact와 normalized가 같으면 `EXACT_SOURCE_CLASS`로 구조 보존한다.
  - exact와 normalized가 다르면 alias가 없을 때 `ALIAS_REQUIRED_EXECUTION_UNAPPROVED`, alias가 있어도
    `EXPLICIT_ALIAS_EXECUTION_UNAPPROVED`로 보존하고 실행 blocker를 유지한다.
  - well-shaped alias string을 명시적 registry/evaluator로 세탁하지 않는다.
- `Matches_InputIdentity`
  - count와 hash만 다시 보는 대신 system/emitter/LOD/module/property/distribution/class/handler receipt 전체를
    typed schema로 재검증한 뒤 inspection hash를 비교한다.
  - forged handler field receipt, role/reference drift, property/storage/provenance mutation과 self-shaped IR을 거부한다.
- production compiler에는 Artist ID 또는 `7/35/399/629`, renderer 분모 hardcode가 없다. fixture count는
  ClientFrontendHarness에만 있다.
- legacy migration gap은 module 세 종류와 실제 distribution class 두 종류를 구분한다:
  `particlemodulecollision`, `particlemodulesizemultiplyvelocity`, `particlemodulesubuvmovie`,
  `distributionfloatsoundparameter`, `distributionvectorconstant`.

## 현재 fixture 실측

checked Source candidate를 독립 JSON walk로 다시 계산한 결과는 다음과 같다.

```text
systems            7
emitters           35
ordered modules    399
distributions      629
exact raw class    373
normalized delta    26
```

26개 normalized-difference reference는 EF custom/seeded class lineage이며 G04에서 executable alias로
승격하지 않는다.

## 자동 검증

실행 완료:

- `Test-Artist31470SourceContract.ps1`: 52 tests PASS, 7/35/399, Product false
- independent fixture JSON walk: 7/35/399/629, exact 373, alias-required 26 PASS
- Engine x64 Debug/Release build: PASS
- `UpdateLib.bat Debug/Release`: PASS
- ClientFrontendHarness x64 Debug/Release build: PASS
- Debug/Release `--effect-source-contract`: 각각 48/48 PASS, failures 0
- Debug/Release `Test-EffectCascadeCompiler.ps1 -HarnessPath ...`: PASS
- Debug/Release `Test-EffectSourceClassLineage.ps1 -HarnessPath ...`: 각각 6/6 mutation PASS
- Client/ClientFrontendHarness project와 filter XML parse: PASS
- `git diff --check`: PASS

G04는 Engine public 계약을 변경하지 않았고 G00 bridge의 Debug Client full build가 선행 통과했으므로,
full Client 재빌드는 이 lane에서 중복 실행하지 않았다. 최종 통합 worktree가 모든 lane 결합 뒤
Debug/Release Client build와 전체 회귀를 소유한다.

이미지 캡처, 육안 검증, 이미지 기반 자동 판정은 수행하지 않았다.

## 남은 blocker

1. Source generated candidate는 아직 optional exact class/alias transport를 채우지 않았고 G01의 receipt-bound
   regeneration이 필요하다.
2. 26 custom/seeded normalized delta에는 G05-S/G06의 reviewed alias/evaluator registry와 numeric oracle가 없다.
3. typed distribution payload, executor, Geometry/Material consumer, 여섯 renderer, Effect Tool prepared revision,
   Catalog transaction은 각각 G05-G10 소유이며 이 commit에서 실행하지 않는다.
4. canonical document FNV hash는 deterministic mutation binding이지 historical source authentication이 아니다.

따라서 이 checkpoint는 G04 generic non-executable compiler/inspection core로만 통합 가능하며 도화가 F
복원 완료 또는 Product admission 근거가 아니다.
