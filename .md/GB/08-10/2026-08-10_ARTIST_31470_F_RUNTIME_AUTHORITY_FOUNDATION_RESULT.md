# 2026-08-10 Artist 31470 F Runtime Authority Foundation Result

## 결과

format 3 compiled Effect entry를 기존 authoring/runtime document와 분리된 immutable authority로 적재하는
중앙 경계를 구현했다. Catalog는 embedded artifact, canonical compiled IR, receipt와 six-hash identity가
일치할 때만 새 authority pointer를 commit한다. v13 carrier와 Assembly는 identity carrier일 뿐이며 raw
semantic executor로 전달되지 않는다.

이 결과는 G06 foundation의 identity/catalog 단계다. typed opcode payload를 materialize하거나
Playback, Renderer, Presentation, EffectObject가 compiled program을 실행하도록 바꾸지 않았다. 따라서
execution admission과 Product admission은 열리지 않았다.

## 구현된 계약

- `EFFECT_COMPILED_RUNTIME_DOCUMENT`
  - artifact/compiler/six-hash/IR/receipt identity
  - handler execution receipts와 aggregate execution contract
  - opcode/resource-binding denominator
  - self-consistency와 external authentication 상태의 명시적 분리
- `CEffectRuntimeAuthorityCodec`
  - exact schema/version/key/type 검증
  - integer token과 float token 구분
  - canonical compiled IR SHA 재계산
  - publisher-compatible pretty artifact/receipt SHA 재계산
  - entry/artifact/IR/receipt identity 및 admission cross-check
  - raw opcode/resource binding의 reserved execution field 거부
- `CEffectCatalog`
  - format 2 legacy document와 format 3 immutable authority map 분리
  - parse -> validate -> stage -> commit
  - snapshot/restore/clear에 authority map 포함
  - failed reload에서 기존 revision/pointer 보존
- public inspection API
  - `Find_RuntimeAuthority`, `Contains_RuntimeAuthority`,
    `Get_RuntimeAuthorityAssetIds`
  - 기존 `Find`, `Contains`, `Get_EffectAssetIds`는 drawable legacy document만 반환

## 자동 검증

- Engine x64 Debug build: PASS
- `UpdateLib.bat Debug`: PASS
- ClientFrontendHarness x64 Debug build: PASS
- Debug `--effect-runtime-authority`: 7/7 PASS, failures 0
- Engine x64 Release build: PASS
- `UpdateLib.bat Release`: PASS
- ClientFrontendHarness x64 Release build: PASS
- Release `--effect-runtime-authority`: 7/7 PASS, failures 0
- Client x64 Debug full build: PASS, errors 0
- Client x64 Release full build: PASS, errors 0
- Debug/Release `Test-EffectRuntimeAuthority.ps1`: PASS
- Debug/Release `Test-EffectCascadeCompiler.ps1`: PASS
- legacy `Tools/EffectPipeline/Test-EffectPipeline.ps1`: PASS
- full `Invoke-ProjectAudit.ps1`: 신규 `effect.runtime-compiled-authority` PASS. 전체는 이 lane 밖의
  기존 11개 map/G09/WModel-harness/WFX/rollout/actor resource gate로 exit 1
- changed project/filter XML parse: PASS
- `git diff --check`: PASS

검증한 mutation은 다음과 같다.

- canonical JSON의 `1` 대 `1.0`, `-0.0`, exponent token domain
- format 3 catalog version `3.0`
- Product admission false -> true
- compiled IR canonical SHA 불일치
- failed catalog reload 뒤 revision/pointer rollback
- format 3 authority가 raw drawable `EFFECT_DOCUMENT_DESC`로 노출되지 않음

이미지 캡처, 육안 판정, 이미지 기반 자동 검증은 수행하지 않았다.

## 남은 blocker

1. embedded artifact/receipt는 내부적으로 self-consistent하지만 final upstream expected identity와 compiler
   receipt를 runtime에서 인증하는 adapter가 아직 없다.
2. compiled program의 opcode/resource binding은 이 단계에서 count와 identity만 검증한다. typed payload
   materialization과 handler capability registry는 G07-G10이 소유한다.
3. Catalog -> Presentation -> EffectObject -> Playback -> Renderer exact pointer attach와 no-I/O prewarm
   transaction은 아직 연결하지 않았다.
4. Geometry preScale/cache/bounds, Material evaluator/shader state, 여섯 renderer family consumer가 아직
   execution receipt를 제출하지 않는다.
5. 따라서 현재 상태는
   `externalAuthenticated=false`, `typedProgramMaterialized=false`,
   `runtimeExecutionAdmission=false`, `productAdmission=false`다.

이 commit은 Product 복원 완료가 아니라, raw authoring 의미가 compiled authority를 우회하지 못하게 하는
검증 가능한 Runtime Authority foundation checkpoint다.
