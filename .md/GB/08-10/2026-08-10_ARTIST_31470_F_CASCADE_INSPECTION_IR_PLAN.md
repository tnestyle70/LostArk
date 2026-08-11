# 2026-08-10 Artist 31470 F Generic Cascade Compiler Core Plan

## 목표

도화가 F Source Contract를 실행값으로 승격하지 않고 구조적으로 검사할 수 있는 immutable typed IR
revision 3을 닫는다. 이번 변경은 G04 generic compiler/source-inspection checkpoint이며 Product 실행기,
renderer, material evaluator, Effect Tool을 연결하지 않는다.

## 고정 경계

- `CEffectCascadeCompiler`는 Source Contract의 `ModuleCoverage`, `ModuleReferenceOrder`, selected LOD와
  geometry metadata만 구조적으로 소비한다.
- raw module literal/distribution payload는 읽거나 실행값으로 materialize하지 않는다.
- reviewed adapter가 전달할 typed expected Source identity seam은 canonical document identity와 opaque
  receipt/candidate token을 함께 받는다. 최종 Source receipt가 아직 없으므로 현재 결과는
  `SELF_CONSISTENT_UNAUTHENTICATED`이고 mandatory blocker를 유지한다. document serialization hash는 input
  mutation binding일 뿐 historical source authentication으로 과장하지 않는다.
- compile은 local stage에만 쓰고 전체 검증 성공 뒤에만 caller의 immutable inspection pointer를 교체한다.
  실패하면 이전 committed inspection을 그대로 보존한다.
- disabled Source recipe도 element 분모에서 생략하지 않고 explicit quarantine emitter로 보존한다.
- compiler 결과는 항상 `bExecutable=false`, `bProductAdmission=false`다.
- 기존 Product Playback, DocumentRenderer, PresentationService의 raw 실행 의미는 변경하지 않는다.
- G00의 optional `exactSourceClass/aliasId` transport가 비어 있으면
  `RECEIPT_NORMALIZED_ONLY`와 blocker를 유지한다. exact class가 normalized schema key와 동일한 경우만
  `EXACT_SOURCE_CLASS`로 구조 승인한다. exact와 normalized가 다르고 alias가 없으면 exact EF/custom/seeded
  class를 `EXACT_CLASS_HANDLER_QUARANTINED`로 보존한다. alias evidence가 있더라도 명시적 registry가 없으므로
  `EXPLICIT_ALIAS_EXECUTION_UNAPPROVED` blocker로 실행을 거부한다.
- Source owner의 receipt-bound typed distribution execution admission/reference ID가 reviewed commit으로
  도착하기 전에는 payload adapter를 연결하지 않는다.

## 구현 범위

1. System/occurrence/element composite identity와 source emitter node/selected LOD lineage를 보존한다.
2. ordered module reference에 exhaustive typed role, source reference index/path/SHA/stable ID를 보존한다.
3. opcode별 allowed/required property schema를 고정하고 각 property에
   `{property key, storage, handlerFieldId, required, result}` consumption receipt를 만든다.
4. property의 canonical path/reference ID, storage/status/provenance/blocker requirement를 typed matrix로 검증한다.
   source receipt의 per-property blocker도 canonical token 집합으로 보존하고, required/prohibited matrix와
   distribution 격리 blocker의 합성을 재검증한다.
5. raw distribution은 payload 미접근·execution disallowed evidence로 격리한다.
6. renderer type/source space와 metadata-only geometry binding을 inspection evidence로 보존한다.
7. selected LOD의 emitter path/node, selected path/node, array index 전체를 canonical lineage ID와 stable
   reference에 결합하고 noncanonical suffix, node reuse, provenance promotion을 거부한다.
8. source execution admission true는 typed class/payload/handler closure 전에 거부한다.
9. public IR 전체를 포함한 deterministic inspection hash, typed expected identity kind/token, compiler-computed
   canonical document identity를 검증한다. checksum과 self-resealed token은 source authentication으로 취급하지 않는다.
10. exact class가 normalized와 같아도 typed schema가 없는 경우 표준 opcode로 추정하지 않는다. 전용
    schema-independent quarantine opcode가 reference/order/property를 보존하고 handler 의미를 부여하지 않는다.
11. 지원하지 않는 legacy module/distribution class는 alias로 정상화하지 않고 migration gap report로 분류한다.
12. disabled recipe count와 declared element count를 emitter receipt에서 재계산하고 35→34 silent shrink를 거부한다.
13. 기존 Client/ClientFrontendHarness project/filter 등록을 보존하고 focused ProjectAudit을 갱신한다.

## 합격 조건

- production compiler에 Artist asset ID 또는 `7/35/399/629`, renderer 분모 hardcode가 없다. 분모는 harness fixture에서만 확인한다.
- fixture는 7 systems, 35 emitters, 399 ordered opcodes, 629 isolated distributions와 renderer `13/16/3/1/1/1`을 확인한다.
- Gate1-shaped fixture가 `normalizedClass=lowercase exactSourceClass`를 채웠을 때 373 known typed class와 26
  schema-independent exact custom reference를 각각 `EXACT_SOURCE_CLASS`와
  `EXACT_CLASS_HANDLER_QUARANTINED`로 보존한다. 26개 class의 exact EF/custom/seeded 이름과 empty alias,
  handler-unavailable blocker를 모두 유지한다.
- recipe 하나를 disable해도 declared/emitter/opcode 분모는 35/35/399이며 disabled count 1과 quarantine blocker가 남는다.
- unknown storage/class/opcode, duplicate role/path/reference/index, property swap, nonfinite geometry,
  forged LOD/node/class/alias/handler receipt/geometry lineage를 거부한다.
- unresolved module/property가 허용된 provenance token만으로 promoted 되지 않는다.
- required property blocker 제거, blocker-prohibited property의 fabricated blocker 추가, compiled IR의 blocker
  제거를 모두 거부한다.
- raw B에 A identity를 재사용하거나 임의로 만든 IR/hash, handler consumption receipt를 입력 identity로
  인증할 수 없다.
- fixed expected identity 아래 blocker/module ID/record SHA/provenance를 coordinated reseal해도 거부하며,
  모든 compile 실패는 caller의 이전 committed inspection pointer와 hash를 보존한다.
- Product/runtime hook은 0이며 compiled inspection 결과의 Product admission은 0/35다.
- Debug/Release ClientFrontendHarness `--effect-source-contract`가 새 revision 3 mutation suite를 통과한다.
- Debug/Release Engine → UpdateLib → ClientFrontendHarness, focused audit와 `git diff --check`가 통과한다.
  최종 full Client build는 통합 worktree가 결합 뒤 수행한다.
- 이미지 캡처나 육안 합격 판정은 수행하지 않는다.
