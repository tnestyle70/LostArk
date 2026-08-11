# 2026-08-10 Artist 31470 F Runtime Authority Foundation Plan

## 목표

G05-P1이 정의한 format 3 derived runtime entry를 Client가 raw authoring document로 해석하지 않고,
별도의 immutable compiled authority identity로 parse/validate/stage/commit한다. 이 변경 단위는 중앙
runtime authority의 첫 단계이며 typed opcode program, Playback/Renderer 실행, Product admission을 열지
않는다.

## 고정 경계

- format 2 catalog와 기존 `EFFECT_DOCUMENT_DESC` 경로는 그대로 유지한다.
- format 3 `IMMUTABLE_COMPILED_IR` entry는 별도 authority map에만 저장한다.
- v13 carrier와 Assembly의 raw field는 runtime semantics로 읽지 않는다.
- embedded compiled artifact, compiled IR, receipt와 six-hash identity를 서로 대조한다.
- JSON integer token과 floating-point token을 구분해 `3.0`, `1.0` version laundering을 거부한다.
- parse/validation 실패는 기존 catalog revision, pointer, legacy document를 보존한다.
- 외부 expected receipt authentication과 typed program adapter가 없으므로 runtime execution과 Product는
  계속 false다.

## 구현 범위

1. `CEffectRuntimeAuthorityCodec`
   - format/schema/exact key/stable ID/lowercase SHA 검증
   - six-hash identity, artifact, IR, receipt의 cross-layer identity 검증
   - canonical compiled IR SHA 및 embedded artifact/receipt pretty-byte SHA 검증
   - handler receipt에서 execution contract 재도출
   - opcode/resource binding 내부의 execution-admission field 주입 거부
2. `CDataJson`
   - parsed number의 integer/float lexical domain 보존
   - object insertion order 보존
   - Python publisher와 같은 canonical/pretty serialization에 필요한 read-only metadata 제공
3. `CEffectCatalog`
   - format 3 exact root와 payload kind 검증
   - immutable authority의 별도 transactional stage/commit/snapshot/rollback
   - legacy drawable lookup과 compiled authority lookup 분리
4. Harness/Audit
   - Product promotion, IR hash, floating version, failed reload mutation
   - raw drawable document 비생성
   - prior pointer/revision rollback
   - Debug/Release 및 format 2/Cascade 회귀

## 합격 조건

- Debug/Release runtime-authority harness가 모두 통과한다.
- Debug/Release Engine, UpdateLib, ClientFrontendHarness, full Client 빌드가 통과한다.
- 기존 Cascade compiler audit와 format 2 Effect publisher 회귀가 통과한다.
- format 3 entry가 committed되어도 `Contains/Find` drawable document에는 나타나지 않는다.
- `bExternalIdentityAuthenticated`, `bTypedProgramMaterialized`,
  `bRuntimeExecutionAdmission`, `bProductAdmission`은 모두 false다.
- 이미지 캡처, 육안 판정, 이미지 기반 자동 검증은 수행하지 않는다.
