# Effect V1 horizontal Sprite canary 구현 결과

## 1. 작업 경계와 기준

Track A는 `origin/main@6f047be2319a17ad3854a0551e825b27647b7e23`에서 시작했다.
Binding 0 registry spine의 구현 커밋은
`a2ffeb3bbfaf3b212973ff823f1ada63a8cbfec7`이다.

고정한 canary identity는 다음과 같다.

| 역할 | stable ID |
|---|---|
| golden control | `effect.artist.skill.31470.unified` |
| first occurrence | `sprite.2b3dc6842507e910` |
| second shadow proof fixture | `sprite.c65181324417a1a8` |

child-parent, G00 inventory/docs, 추가 DXBC 추출, 4캐릭터/Valtan cohort,
DimensionMaster BA/A, Artist R decal, Mesh/Decal/Trail/Glass/WPO/Presentation은
변경하지 않았다. 새 Artist F shader나 skill-ID renderer switch도 만들지 않았다.

## 2. Binding 0 registry spine

`Data/Effects/MaterialPrograms/effect-material-program-registry.v1.json`을 source 정본으로
추가하고 publisher가 catalog format version 4 안에 immutable registry payload와 revision을
봉인하게 했다. Binding 0 baseline은 program/layout/descriptor/adapter 정의를 모두
포함하지만 `bindings`는 정확히 빈 배열이다.

Client는 catalog를 `parse -> validate -> stage -> commit`으로 적재한다. registry handle,
revision, generation, binding count를 catalog에서 prewarm target과 prepared document까지
같은 immutable snapshot으로 전달한다. binding이 없는 occurrence는 기존 inline
`Material.Execution` draw path를 그대로 사용한다.

binding이 존재하는 경우에는 registry가 만든 packet과 inline golden packet을 다음 전체
계약으로 bit-exact 비교한다.

- execution의 모든 field와 ordered lane
- ordered scalar/vector/sampler
- 모든 float의 bit pattern
- standard color와 adapter/carrier/pass/state identity

불일치, dangling ID, duplicate ownership, revision/generation 혼합은 fail-closed다. binding이
있는 occurrence를 조용히 inline으로 되돌리는 fallback은 없다.

## 3. 실제 Sprite draw adapter

기존 `SpriteParticle/RuntimeMaterialV2` 경로를 typed adapter로 승격했다. bound occurrence의
`Render_Particles`는 metadata만 확인하지 않고 다음 실제 draw 상태를 검증한다.

- Sprite carrier와 compiled adapter ID
- opcode 6, shader pass 1, shader/VIBuffer identity
- `Shader::Begin` 뒤 실제 rasterizer/depth-stencil/blend/sample-mask/stencil state
- `MRT_SceneHDR`의 실제 RT0/RT1/DSV identity와 RT2~RT7 null

Engine의 output scope는 `Begin_MRT("MRT_SceneHDR")`가 설정한 D3D11 context/target을
capture하고 draw 직전에 다시 조회한다. 따라서 다른 MRT 재바인딩이나 metadata-only 연결은
실패한다.

## 4. 자동 검증 상태

### 4.1 Binding 0 PASS

| 항목 | 결과 |
|---|---|
| registry focused Python | 8 tests PASS |
| direct-authored runtime validator | 6 tests PASS |
| JSON/XML parse | PASS |
| Effect publisher Validate | 145 Effects, 0 bindings PASS |
| Engine → UpdateLib → Client Debug | PASS |
| Engine → UpdateLib → Client Release | PASS |
| Server Debug build + `--contract-test` | failures 0 PASS |
| `git diff --check` | PASS |

Binding 0 산출물 identity는 다음과 같다.

| 산출물 | SHA-256 |
|---|---|
| Debug `Client.exe` | `bfb370ffc3c9e438536cb0b7db745a876e7577ea99ee621156be8c4ccc8a64e3` |
| Release `Client.exe` | `b28df1a625f92278e954917b737fb6a7bb8cf4ce4b3753f60787dcf40739368d` |
| Binding 0 `EffectCatalog.runtime.json` | `90e0559a62f47a57079a067f2a8846cac9dfe0ab2e9bf13f808017dc912edd4d` |

### 4.2 아직 닫지 않은 gate

- focused WARP runner의 Artist F Full35, unified canary, Lance BA1 fixed-step 통계
- first occurrence 한 건을 실제 source/runtime catalog에 연결한 Binding 1 candidate
- Binding 0/1 통계 exact 비교와 candidate Debug/Release build
- Server Release: Track A와 무관한 기존 `GameRoom.cpp`의 Release compile error
  (`FINAL_ARENA_PATTERN_ID`, `FINAL_ARENA_STAGE_ID`, `FINAL_ARENA_ACTION_ID` 미정의)를
  별도 경계에서 확인 중이다.
- 사용자 first-pixel A/B

삭제된 광역 `ClientFrontendHarness`의 역사 수치는 현재 증거로 재사용하지 않았다. 위 통계는
제품 `Late_Update -> GameInstance::Render -> MRT_SceneHDR` 경로를 타는 focused headless
runner에서 새로 측정한 값만 기록한다.

## 5. 수동 화면 판정

Client/UI는 에이전트가 실행하거나 조작하지 않았고 visual PASS도 선언하지 않았다. Binding 1
candidate와 자동 A/B가 완료된 뒤 `Framework.slnLaunch`의 `Server + Client` profile,
`Client/Default` working directory, Artist F 경로와 두 commit/catalog identity를 최종 인계한다.
