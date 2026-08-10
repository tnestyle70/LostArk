# Artist F 31470 Reconstructed Runtime Program Builder 계획

날짜: 2026-08-10

브랜치: `codex/artist-f-reconstructed-runtime-program-builder-v1`

## 목표

독립 검증된 Source, Material, Geometry, 승인 정책을 하나의 immutable typed runtime-program
candidate로 결합한다. 이 단계는 Python offline materializer와 검증기만 소유한다. C++, Playback,
Renderer, Catalog, Resources는 수정하지 않으며 R3 executor가 연결되기 전까지 runtime과 Product
admission은 모두 false로 유지한다.

## 입력 권위

- 승인 정책: `baee9f64039b455dd17666c2cd1289b8d1777d28`
- Source evidence/candidate/normalized graph/action cue: `7da937aeaa34c088c694e8eb4f53ff1f7f848ef3`
- Source reconstructed capability: `2fa2a2701e0ec9a8f990c65fef3fabb9e5a3e408`
- Material evidence/runtime: `cde8f3bddea2f9415f682b387d2705fd25794075`
- Material reconstructed policy: `97597531215fa9c9873fe1be3ba8cd23db60031d`
- Geometry binding/receipt: `0aca792819fdda3f541bb7cec7451c5ed93c6467`
- Material runtime texture binding 72/72: `1a0b1a6834d562dac02db4f57dda54644d75695b`
  (`materialTextureBindings` 72행 = runtime-cook 68 + exact-DDS deployment 4, unresolved 0)
- exact-DDS transactional deployment: `01b8b8a8bace09a3576f116771daf4859aa485a3`
  (13번째 Material receipt가 commit/tree/blob와 4개 deployment row를 transitively 결합)

입력은 mutable worktree 파일이 아니라 Git commit/tree/blob ID로 읽는다. tracked JSON은 LF
canonical text와 canonical JSON hash를 함께 보존하고, duplicate key, BOM, nonfinite 값을 거부한다.

## 생성 계약

`Tools/EffectPipeline/build_artist_31470_reconstructed_runtime_program.py`가 다음 flat typed section을
생성하고 모든 행의 ID, owner, order, row SHA, section SHA와 program SHA를 다시 검증한다.

- emitter 35, exact action schedule 7
- ordered module 399, property 1,434, primitive leaf 1,572, literal 1,590
- distribution 629: INLINE 612, FLOAT_PARAMETER 8, VECTOR_PARAMETER 5,
  FLOAT_CURVE 1, EF_MULTIPLY 3
- seed policy 14, implicit default 14, PointLight field 8
- Material family/recipe/occurrence 23/27/34, input/static/render 729/94/162,
  policy 255, logical texture binding 72
- renderer texture diagnostic projection 57
- Geometry carrier/use 7/13

Literal은 `BOOL/F64/ENUM_STRING`, distribution은 다섯 closed variant로만 표현한다. nested
sample, ActionCue binding, field provenance, Material descriptor/SRV, geometry submesh/channel/bounds도
exact key와 JSON type으로 검증한다. raw JSON bag이나 canonical JSON string을 executable payload로
사용하지 않는다.

## 실행 경계

- Source reconstructed module 29개는 capability implementation, property consumption, literal,
  distribution, seed, ActionCue projection과 numeric sample 87개를 직접 소유한다.
- source handler 370개와 reconstructed handler 29개를 구분하고 unknown fallback을 허용하지 않는다.
- ActionCue schedule은 recipe의 full precision event time 7행을 35 emitter에 역결합한다.
- Required/Spawn/Lifetime typed rows에서 duration, delay, loop, lifetime, burst 31행을 재도출한다.
- Material approval 255행과 arithmetic 23행, Geometry approval 7행을 승인 receipt의 frozen projection과
  exact join한다.
- candidate Detail presentation 값은 `DIAGNOSTIC_ONLY_FORBIDDEN_CONSUMPTION`이다. ScreenPost, Light,
  Decal, Ribbon은 별도 typed adapter만 실행 권위 후보로 보존한다.
- Material logical texture 72행은 모두 `RESOLVED_EXACT_RUNTIME_ASSET`으로 정규화한다. 원본 receipt의
  status는 runtime-cook 68과 reconstructed exact-DDS deployment 4로 분리해 보존하고, 후자의 proposal와
  deployment row ID/SHA를 함께 보존한다. 해소된 unresolved/deployment-pending blocker만 제거하고
  R4 SRV consumer blocker와 기존 sampler/source-fidelity blocker는 유지한다. renderer slot 57행은
  preview projection으로만 둔다.
- `sourceExact=false`, `runtimeExecution=false`, `product=false`를 재도출한다.

## 검증 계획

1. deterministic build와 byte-identical `--check`
2. 실제 `validate_program`을 호출하는 coordinated reseal mutation 테스트
3. unknown semantic, owner/order/count, schedule, duration default, burst sentinel, seeded Lifetime,
   capability projection, approval 29/255/23/7, D3D 107/SRV 72, Geometry tuple/cache,
   unsafe resource path, runtime/Product flip 거부
4. 배포 4행 runtime asset 교환, status/basis/proposal/deployment row 위조·누락, 구 68+4 receipt A/B,
   R4 blocker 공동 제거와 전체 canonical reseal 거부
5. tracked LF/CRLF canonical equivalence
6. focused ProjectAudit 등록과 실행
7. JSON parse, `git diff --check`, 전체 ProjectAudit 결과 분리 기록

## 완료 조건

현재 schema와 13-input candidate가 독립 reviewer의 frozen-tree mutation audit를 통과하면 검증된
단일 commit을 push하고 R3 C++ parser/executor에 exact program identity를 인계한다.
