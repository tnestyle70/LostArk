# 2026-08-10 Artist 31470 F Source Runtime Program Result

## 결과

Source Execution Semantics receipt를 immutable typed runtime-program candidate로 materialize하는
checkpoint를 구현했다. candidate는 35 emitter, 399 ordered opcode, 629 distribution을 보존하며
renderer 분모는 Mesh 13, Sprite 16, Decal 3, Ribbon 1, Light 1, ScreenPost 1이다.

현재 source checkpoint 기준 opcode는 370 ready / 29 blocked다. blocked custom class나 distribution을
표준 handler로 alias하지 않았고 silent fallback은 0이다. 따라서 이 결과의 admission은 다음과 같다.

- source identity self-consistency: true
- typed program materialized: true
- runtime execution admission: false
- Product admission: false

## 구현 결과

- generated candidate:
  `Data/Effects/Imported/Artist/Candidates/skill.31470.source-runtime-program.candidate.json`
- schema/version: `lostark.effect-source-runtime-program` / 1
- program SHA-256: `b1737ec7aabc9013bd8b3aa9c1966d6d8fb3e7cf5d7cdddec31079eaa46e16a7`
- handler receipt count: 41
- emitter/opcode/distribution: 35 / 399 / 629
- ready/blocked opcode: 370 / 29
- silent fallback: 0

`CEffectRuntimeAuthorityCodec::Parse_SourceRuntimeProgram`은 strict schema/type/SHA 검증 뒤 staged
immutable pointer만 commit한다. renderer 문자열은 실제 source의 `MeshParticle`, `SpriteParticle`,
`DecalParticle`, `CascadeRibbon`, `LightParticle`, `ScreenPost`만 enum으로 변환한다. literal과
distribution의 nonfinite 값, 중복 ID, 순서/coverage drift, blocker/handler 불일치를 거부한다.

## 자동 검증

- Python unit/mutation: 9/9 PASS
- generator `--check`: PASS
- Debug focused audit: PASS
  - Python 9/9
  - C++ `--effect-source-runtime-program` 7/7
- Release focused audit: PASS
  - Python 9/9
  - C++ `--effect-source-runtime-program` 7/7
- Engine x64 Debug/Release build: PASS
- `UpdateLib.bat` Debug/Release: PASS
- ClientFrontendHarness x64 Debug/Release build: PASS
- full `Invoke-ProjectAudit.ps1`: 신규 `effect.artist-31470-source-runtime-program` PASS. 전체는 이
  G 밖의 기존 map/data visibility, G09, Source execution wrapper, missing WModel harness,
  WFX/rollout/actor resource gate 12건으로 exit 1
- `git diff --check`: PASS
- build process residue: 0

검증은 numeric/structural oracle과 compile/link/runtime parser error만 사용했다. 이미지 캡처,
육안 비교, 이미지 기반 자동 검증은 수행하지 않았다.

## 남은 blocker

1. 이 candidate는 frozen Source receipt의 370/29 상태를 반영한다. Source follow-up custom-handler
   oracle의 seeded 11건 승격은 독립 PASS 후 재생성해야 한다.
2. 남은 EF custom handler 15 occurrence와 EF multiply distribution 소유 module 3 occurrence는
   native/evaluator oracle 없이는 실행할 수 없다.
3. Playback, Renderer, Presentation은 아직 이 typed program을 sole semantic authority로 소비하지 않는다.
4. Geometry preScale/cache/bounds, Material evaluator/render state, 6 renderer capability receipt가 아직 없다.
5. Effect Tool exact prepared revision과 catalog publish/prewarm rollback transaction이 아직 연결되지 않았다.

따라서 이 commit은 typed materialization checkpoint이며 Artist F 복원 완료나 Product 35/35가 아니다.
후속 통합은 independent frozen review PASS 뒤에만 진행한다.
