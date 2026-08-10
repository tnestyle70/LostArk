# 2026-08-10 Artist 31470 F Cascade Inspection IR Result

## 결과

도화가 F Source Contract를 실행하지 않고 검사하는 immutable typed Cascade inspection IR checkpoint를 추가했다. 이 변경은 실행 compiler 완료가 아니다. 결과는 의도적으로 항상 `bExecutable=false`, `bProductAdmission=false`이며 Product admission은 0/35다.

기존 Product Playback, DocumentRenderer, PresentationService, 여섯 renderer family의 실행 코드는 변경하지 않았다. 따라서 raw executor도 그대로이며 새 inspection IR로 되돌아오거나 실행되는 경로는 없다.

## 구현한 계약

- `CEffectCascadeCompiler::Compile_SourceInspection`
  - compiler가 직렬화한 전체 document로 canonical identity를 계산하고 caller의 expected identity와 대조한다.
  - System → occurrence → element composite identity, source emitter node, selected LOD, ordered module reference를 보존한다.
  - module role을 `REQUIRED/MODULE/SPAWN/TYPE_DATA` typed enum으로 보존하고 opcode/role 조합을 검증한다.
  - opcode-specific allowed/required property schema와 handler required/consumed reference receipt를 생성한다.
  - canonical property path와 reference ID, typed storage/status/provenance/blocker requirement를 검증한다.
  - distribution payload를 읽지 않고 `bRawPayloadRead=false`, `bExecutionAllowed=false` evidence로 격리한다.
  - renderer/source-space와 metadata-only geometry evidence를 보존하되 runtime consumer readiness를 false로 유지한다.
  - public IR의 모든 inspection field와 receipt를 deterministic hash에 포함하고 self-verification한다.
- existing `normalizedClass`는 receipt normalization으로만 보존한다. `strExactSourceClass`, `strAliasId`, exact lineage는 비워 두고 `SOURCE_EXACT_CLASS_LINEAGE_ADAPTER_PENDING` blocker를 유지한다.
- checksum은 인증 근거가 아니므로 `CANONICAL_DOCUMENT_CHECKSUM_NOT_AUTHENTICATION` blocker를 유지한다.
- collision, sizemultiplyvelocity, subuvmovie, soundparameter, vectorconstant는 silent alias 없이 explicit legacy migration gap으로 분류한다.
- fixture 전용 분모 `7/35/399/629`와 renderer `13/16/3/1/1/1`은 ClientFrontendHarness에만 존재한다.

## 자동 검증

Debug와 Release에서 같은 25개 gate를 실행했다.

1. fixture 7 systems / 35 emitters / 399 opcodes / 629 isolated distributions 및 renderer 분모
2. deterministic canonical identity와 inspection hash
3. raw B가 A canonical identity를 재사용하는 경우 거부
4. 임의 default IR과 valid-looking ID/hash의 `Matches_InputIdentity` 거부
5. source emitter node와 LOD lineage의 hash binding
6. source+coverage simultaneous unknown property 거부
7. EF class mutation 거부
8. `SOURCE_TAGGED`에서 `SOURCE_EXACT` provenance promotion 거부
9. unresolved ColorScale module/property aggregate promotion 거부
10. forged selected LOD path 거부
11. valid-looking emitter node package mismatch 거부
12. module stable/alias lineage drift 거부
13. opcode alias schema mismatch 거부
14. `REQUIRED` opcode의 valid-looking wrong role 거부
15. duplicate property path/reference 거부
16. unknown storage 거부
17. duplicate module reference index 거부
18. nonfinite geometry evidence 거부
19. geometry asset ID와 mesh ResourceBinding mismatch 거부
20. 다섯 legacy migration gap의 explicit classification
21. Source Contract codec roundtrip
22. legacy v14 field rejection
23. geometry scale contract preservation
24. ordered reference preservation
25. native source field preservation

실행 결과:

- ClientFrontendHarness x64 Debug build: PASS
- Debug `ClientFrontendHarness.exe --effect-source-contract <Artist F candidate>`: 25/25 PASS, `failures : 0`
- ClientFrontendHarness x64 Release build: PASS
- Release `ClientFrontendHarness.exe --effect-source-contract <Artist F candidate>`: 25/25 PASS, `failures : 0`
- `Test-EffectCascadeCompiler.ps1` with Debug harness: PASS
- `Test-EffectCascadeCompiler.ps1` with Release harness: PASS
- Client x64 Debug build: PASS
- Client x64 Release build: PASS
- `git diff --check`: PASS
- 이미지 캡처/육안 검증: 수행하지 않음

## 남은 blocker

1. Source typed adapter: reviewed Source commit의 exact raw class/alias lineage, receipt-bound typed distribution execution admission/reference ID, source-authenticated provenance를 연결해야 한다.
2. typed opcode executor: 현재 Product raw executor는 그대로이며 hot path를 compiled opcode/payload만 소비하도록 바꾸지 않았다.
3. geometry consumer: `geometryPreScale=0.01`, model cache identity, bounds와 tangent/color channel consumption이 runtime에서 닫히지 않았다.
4. material closure: rendered material 27개의 typed recipe, arithmetic family evaluator, HLSL/render-state 소비가 없다.
5. renderer closure: Mesh/Sprite/Decal/Ribbon/Light/Post 여섯 family가 typed IR을 소비하지 않는다.
6. Effect Tool: exact catalog revision/prepared hash attach, Product Play의 authored disk reload 제거, IR stable ID/order 기반 filter가 없다.
7. catalog transaction: publish/load/prewarm/no-I/O와 failed stage rollback을 compiled authority로 연결하지 않았다.
8. legacy class migration gap 다섯 종류는 실행 opcode schema가 없다.

따라서 이 커밋은 Source inspection 구조와 공격 검증을 고정하는 비실행형 WIP checkpoint이며 도화가 F 복원 완료 또는 renderer 완료 근거가 아니다.
