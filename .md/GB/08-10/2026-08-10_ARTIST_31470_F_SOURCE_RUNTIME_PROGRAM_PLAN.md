# 2026-08-10 Artist 31470 F Source Runtime Program Plan

## 1. 목표와 종료 조건

검증된 Source Execution Semantics receipt를 매 프레임 문자열로 다시 해석하지 않고,
한 번 생성한 immutable typed program으로 옮긴다. 이 단계는 실행기 연결 전의 materialization
checkpoint다. source에 남은 blocker를 값으로 보정하지 않으며 다음 조건을 모두 만족할 때만 닫는다.

- 35 emitter와 399 ordered module reference를 stable ID와 원본 순서 그대로 보존한다.
- 629 distribution을 owning opcode의 typed payload와 adapter로 결합한다.
- literal, distribution, selected LOD, ActionCue parameter, renderer family를 typed field로 보존한다.
- exact source class별 handler ID와 handler receipt를 보존하고 unknown/custom class를 실행하지 않는다.
- source blocker union과 opcode blocker를 다시 계산하며 silent fallback은 0이다.
- parse 실패는 기존 immutable program pointer를 보존한다.
- Debug/Release C++ consumer와 Python mutation suite가 같은 candidate를 검증한다.
- `runtimeExecutionAdmission=false`, `productAdmission=false`를 유지한다.

## 2. 소유 경계

이 G는 다음 파일만 소유한다.

- `Client/Public/Effect_RuntimeAuthority.h`
- `Client/Private/Effect_RuntimeAuthority.cpp`
- `Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp`
- `Tools/EffectPipeline/build_artist_31470_source_runtime_program.py`
- `Tools/EffectPipeline/test_build_artist_31470_source_runtime_program.py`
- `Tools/ProjectAudit/Test-Artist31470SourceRuntimeProgram.ps1`
- `Tools/ProjectAudit/Invoke-ProjectAudit.ps1`
- generated candidate `Data/Effects/Imported/Artist/Candidates/skill.31470.source-runtime-program.candidate.json`

Source evidence, Geometry resource, Material evaluator, Playback, renderer, Presentation, Effect Tool,
Catalog publisher는 수정하지 않는다. 이 checkpoint가 Playback 실행 권위나 Product admission을
만들었다고 판정하지 않는다.

## 3. H 계약

`Effect_RuntimeAuthority.h`에는 다음 typed 계약을 추가한다.

- `EFFECT_RUNTIME_RENDERER_FAMILY`: Mesh, Sprite, Decal, Ribbon, Light, ScreenPost만 허용한다.
- `EFFECT_RUNTIME_LITERAL`: Boolean, Number, String을 분리하고 number는 finite만 허용한다.
- `EFFECT_RUNTIME_DISTRIBUTION`: lookup table, keys, range, operation, reference identity를 보존한다.
- `EFFECT_RUNTIME_DISTRIBUTION_ADAPTER`: payload와 evaluator capability, blocker를 결합한다.
- `EFFECT_RUNTIME_OPCODE`: exact source identity, typed payload, seed/default, ordered handler를 소유한다.
- `EFFECT_RUNTIME_EMITTER`: selected LOD, ActionCue parameter, renderer family, ordered opcode ID를 소유한다.
- `EFFECT_SOURCE_RUNTIME_PROGRAM`: source receipt 세 identity, program SHA, execution contract와 admission을 소유한다.
- `CEffectRuntimeAuthorityCodec::Parse_SourceRuntimeProgram`: parse -> validate -> stage -> commit을 수행한다.

## 4. CPP와 offline generator 책임

`build_artist_31470_source_runtime_program.py`는 strict duplicate-key JSON loader로 reviewed receipt를
읽고 source row를 그대로 typed program으로 투영한다. exact class가 등록된 경우에만 opcode 이름을
결정하며 blocked custom class에는 `EXACT_CLASS_BLOCKED`만 기록한다. 모든 blocker를 집합으로 다시
계산하고 `FINAL_INTEGRATION_PRODUCT_ADMISSION_REQUIRED`를 유지한다.

`Parse_SourceRuntimeProgram`은 schema/version/root key, SHA, renderer enum, literal token,
distribution vector width, emitter/opcode coverage와 order, handler receipt, blocker count와 summary를
다시 검증한다. 검증이 끝난 staged object만 caller pointer에 commit한다.

Harness는 정상 candidate와 version, duplicate, nonfinite, order/identity/blocker mutation 및
failed parse rollback을 검증한다. Artist 고정 분모는 production parser가 아니라 fixture harness에만 둔다.

## 5. 검증 순서

1. Python unit/mutation suite와 generator `--check`.
2. Engine Debug/Release -> `UpdateLib.bat` Debug/Release.
3. ClientFrontendHarness Debug/Release build.
4. Debug/Release `--effect-source-runtime-program`.
5. Debug/Release focused ProjectAudit.
6. strict JSON parse, `git diff --check`, process residue 0.

이미지 캡처, 육안 비교, 이미지 기반 자동 판정은 수행하지 않는다.

## 6. 다음 G에 넘기는 경계

이 program은 compile-once 입력이다. 후속 G는 reviewed Source custom-handler oracle을 결합해
blocked exact class를 capability receipt로만 승격하고, Playback이 raw SourceRecipe를 읽지 않도록
immutable compiled program을 연결한다. Geometry preScale/cache/bounds, Material binding/evaluator,
6 renderer, Effect Tool/catalog transaction은 각각 별도 검증 단위로 닫는다.
