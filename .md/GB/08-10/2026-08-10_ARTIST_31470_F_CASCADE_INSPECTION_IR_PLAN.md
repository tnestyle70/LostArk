# 2026-08-10 Artist 31470 F Cascade Inspection IR Plan

## 목표

도화가 F Source Contract를 실행값으로 승격하지 않고 구조적으로 검사할 수 있는 immutable typed IR을 추가한다. 이번 변경은 compiler/source-inspection checkpoint이며 Product 실행기, renderer, material evaluator, Effect Tool을 연결하지 않는다.

## 고정 경계

- `CEffectCascadeCompiler`는 Source Contract의 `ModuleCoverage`와 `ModuleReferenceOrder`만 구조적으로 소비한다.
- raw module literal/distribution payload는 읽거나 실행값으로 materialize하지 않는다.
- compiler 결과는 항상 `bExecutable=false`, `bProductAdmission=false`다.
- 기존 Product Playback, DocumentRenderer, PresentationService의 raw 실행 의미는 변경하지 않는다.
- 기존 v14 `normalizedClass`는 exact source class 또는 alias lineage로 승격하지 않는다.
- Source owner의 receipt-bound typed distribution admission/class lineage adapter가 reviewed commit으로 도착하기 전에는 연결하지 않는다.

## 구현 범위

1. System/occurrence/element composite identity와 source emitter node/selected LOD lineage를 보존한다.
2. ordered module reference에 exhaustive typed role, source reference index/path/SHA/stable ID를 보존한다.
3. opcode별 allowed/required property schema와 handler consumption receipt를 만든다.
4. property의 canonical path/reference ID, storage/status/provenance/blocker requirement를 typed matrix로 검증한다.
5. raw distribution은 payload 미접근·execution disallowed evidence로 격리한다.
6. renderer type/source space와 metadata-only geometry binding을 inspection evidence로 보존한다.
7. public IR 전체를 포함한 deterministic inspection hash와 compiler-computed canonical document identity를 검증한다. checksum은 source authentication으로 취급하지 않는다.
8. 지원하지 않는 legacy class는 alias로 정상화하지 않고 migration gap report로 분류한다.
9. Client와 ClientFrontendHarness project/filter에 compiler를 등록하고 focused ProjectAudit을 추가한다.

## 합격 조건

- production compiler에 Artist asset ID 또는 `7/35/399/629`, renderer 분모 hardcode가 없다. 분모는 harness fixture에서만 확인한다.
- fixture는 7 systems, 35 emitters, 399 ordered opcodes, 629 isolated distributions와 renderer `13/16/3/1/1/1`을 확인한다.
- unknown storage/class/opcode, duplicate role/path/reference/index, property swap, nonfinite geometry, forged LOD/node/alias/geometry lineage를 거부한다.
- unresolved module/property가 허용된 provenance token만으로 promoted 되지 않는다.
- raw B에 A identity를 재사용하거나 임의로 만든 IR/hash를 입력 identity로 인증할 수 없다.
- Product/runtime hook은 0이며 compiled inspection 결과의 Product admission은 0/35다.
- Debug/Release ClientFrontendHarness `--effect-source-contract`가 25/25를 통과한다.
- Debug/Release focused audit와 Client build, `git diff --check`가 통과한다.
- 이미지 캡처나 육안 합격 판정은 수행하지 않는다.
