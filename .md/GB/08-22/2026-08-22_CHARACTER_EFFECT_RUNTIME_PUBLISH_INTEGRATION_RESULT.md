# 캐릭터 Effect authored → sealed runtime 선별 통합 결과

## 1. 최종 선별 경계

기준은 `origin/main@7fb8f8139f62657914228070ebe2a9860287b577`이고 통합 브랜치는
`codex/captain-effects-no-glasshole`이다. 닫힌 PR #141의 구현은
`codex/effect-family-conquest@9480a68ff82f1e4cd44788191861ddceaed8a037`에서 계약 단위로
다시 검토했다. 창술사 Q/A 미완성 작업은 stash 번호가 아닌 exact object
`4489abbdb6b303b3ff9871935c71055e70b29efe`를 read-only로 확인한 뒤 필요한 제품 변경만
적용했다.

다음은 의도적으로 제외했다.

- source-exact 복원을 별도로 진행 중인 Glasshole02 bounded opcode/canary/visual program
- 차원술사 `2050120.clip3` Glasshole authored 문서와 runtime mapping
- #141이 추가했던 `ClientFrontendHarness` project/source/assertion 변경 전체
- 발탄 authored/candidate/pattern cue/binding/world data

최종 diff에서 `Tools/ClientFrontendHarness` 및 `2050120.clip3` authored 문서는
`origin/main`과 byte diff가 없다. Glasshole 구현 symbol
`RUNTIME_MATERIAL_V2_UE3_GLASSHOLE02`, `glasshole02-k01`, `effect-glass-family-fast`도 없다.
`Level_ValtanArena.cpp`의 차원술사 T 공용 preview clone 초기화 외에 발탄 소유 데이터는
수정하지 않았다.

## 2. main 승격 대상

### 2.1 sealed runtime catalog

`EffectCatalog.runtime.json`의 205개 Effect를 검증·publish했다. Glasshole를 제외한
실제 authored document identity 변경은 17개다.

| 대상 | 변경된 runtime Effect |
|---|---|
| 도화가 | `31210.ba1`, `31210.ba4`, `31420`, `31460`, 신규 `31490`, `31910`, `31950` |
| 차원술사 | `2050210`, `2050230` |
| 창술사 | `34560.clip3`, `34630.clip1~4`, `34650.clip1` |
| 워로드 | `17240.ba1`, `17240.ba3` |

도화가 A/S/R/D/T/V, 워로드 T, 창술사 E와 dragon cohort, 차원술사 A/F가 실제
sealed runtime을 소비한다. E `31480`과 F `31470`은 publish 전부터 authored/runtime이
semantic-equal이어서 catalog identity 교체 개수에는 포함되지 않지만 관련 runtime 연결은
함께 빌드했다.

### 2.2 runtime family와 gameplay

다음 계약을 data/publisher/C++/HLSL/consumer 단위로 유지했다.

- StandardColorV1, BLACK_TIGER_STROKE
- ScreenOverlay, Fluid01
- CircleSurface, Vortex, TargetAttractor
- Lance dragon masked family, Artist CascadeRibbon
- typed Light/ScreenPost authoring과 Product playback
- action-facing occurrence, 차원술사 T Server-authoritative ground target
- 차원술사 BA1 0.7초 Server clock, 2.0x animation/effect, 압축 root motion

창술사 Q `34040`는 대체 BA mesh를 새로 만들 필요가 없었다. Product에 이미 있는
clip1 WaterTrail mesh 1행과 clip2 MissileTrail mesh 4행의 WModel/DDS/source recipe를 그대로
사용했다. 문제였던 profile 13의 named scalar, texture lane, dynamic parameter semantic을
공용 renderer/HLSL에 연결했다. A `34140`은 Product transform을 임의로 바꾸지 않고
Tool tree/Selection에 `skillId`, stance, stage/clip/effect ID를 노출해 exact row를 선택할 수
있게 했다.

## 3. #141의 46건 병목 분리

PR #141은 203 files, +162,964/-18,409 lines의 단일 diff였다. 당시 Debug 정본
통합에서 실패한 46건의 대표 군은 다음이었다.

- DimensionMaster source material admission과 semantic module 보존
- legacy GPU occurrence
- source overlay CAS/round-trip
- Valtan placement source contract

이 실패는 한 실행 파일이 Effect family, resource, Tool, 발탄의 역사적 assertion을
모두 동시에 재생하면서 발생한 통합 결합이었다. 이번 브랜치는 해당 프로젝트의
신규 변경을 전부 제외했고 이 46건을 `0 PASS`로 바꿔 기록하지 않았다. 대신
실제 제품 consumer인 Engine/Client build, Effect/gameplay publisher, NetworkProtocolHarness,
Server contract를 최종 admission으로 다시 실행했다.

이 과정에서 실제로 재현된 Effect pipeline 회귀는 두 건이었고 둘 다 수정했다.

1. isolated BossCatalog fixture가 발탄 combat object Effect 두 행을 다시 기존 fixture ID로
   덮어써 `cataloguedWithoutOwner` 두 건을 만들었다. 각 moving row가 정확한 combat
   object Effect ID를 소유하게 고정했다.
2. restoration inventory test가 publish 전 `19 ADMITTED + 3 AUTHORING_ONLY`를 기대했지만
   최종 catalog은 22개 전부 `ADMITTED/CLOSED`였다. 정본 상태와 fail-closed negative를
   현재 계약에 맞게 갱신했다.

## 4. 생성물과 자동 검증

| 항목 | 결과 |
|---|---|
| Effect publisher Validate/Publish | 205 Effects, 0 Components PASS |
| visual-program artifact | 17 programs, 135 rows, `productMutation=false`, SHA-256 `0efa773900da495869f71b4f3d0bba787455ca3fb10eb790c4ca432762191054` PASS |
| sealed runtime mapping | Glasshole 제외 17 Effect identity 교체 PASS |
| `Test-EffectPipeline.ps1` | fixture 전체 + Python 110 tests PASS |
| restoration inventory focused | 23 tests, 22 `ADMITTED/CLOSED` PASS |
| Effect data project check | files 1833, filters 207 PASS |
| gameplay balance Validate | 6 profiles, 231 skills, 108 damage profiles PASS |
| balance runtime-set Validate | Bern/Valtan/Training/Character Select world·spawn group·item validation PASS |
| Debug full build | Engine → UpdateLib → Shared → NetworkProtocolHarness → Server → Client PASS |
| Release full build | Engine → UpdateLib → Shared → NetworkProtocolHarness → Server → Client PASS |
| NetworkProtocolHarness Debug/Release | 각 failures 0 PASS |
| `Server.exe --contract-test` Debug/Release | 각 failures 0 PASS |
| Glasshole bounded symbol/document check | 전용 symbol 없음, `2050120.clip3` origin/main identity PASS |
| ClientFrontendHarness diff check | origin/main 대비 diff 없음; 최종 admission에 사용하지 않음 |

Release Client에는 기존 FXC X4717/X4000, C4819, DirectXTK LNK4099 warning이 남지만
신규 compile/link error는 0이다. Client/UI는 에이전트가 실행·조작하지 않았다.

## 5. 보류한 변경과 이유

| 보류 대상 | 보존 위치 | 이유 |
|---|---|---|
| source-exact Glasshole02 | `codex/missing-effect-family-recovery@ff3eb7d2` | 전용 Tool canary 계약이 완료되어 이 no-Glass 통합 후 별도 PR로 승격 |
| #141 광역 frontend assertions | PR #141 / 원본 branch | 46건 광역 결합을 제품 승격 gate로 다시 포함하지 않음 |
| 창술사 A 사용자 transform tuning | 기존 Product row `d629659d...` | 정확한 값은 사용자 화면 판정 후에만 변경 |

창술사 Q의 BA mesh 생성은 보류가 아니라 `불필요`다. 기존 exact mesh 1+4행을
사용했기 때문에 추가 mesh를 만들면 source identity를 중복하게 된다.

## 6. 사용자 수동 확인 경로

`Framework.slnLaunch`의 `Server + Client` profile을 사용하고 Client working directory는
`Client/Default`로 둔다. 사용자가 `Ctrl+F5`로 실행한 뒤 Lobby → Character Select에서
다음을 직접 확인한다.

1. 창술사 Q `34040`: 1타 WaterTrail 1행, 2타 MissileTrail 4행의 검격, 방향, 색감.
2. 창술사 A `34140`: Tool에서 skill/stance/stage/clip/effect ID로 `d629659d...`를
   선택한 뒤 저장·publish·재시작한 다음 cast에서만 position/rotation이 반영되는지.
3. 차원술사 LMB BA1: 찌르기와 unified Effect가 약 0.7초에 같이 끝나고
   추가 입력 없이 BA2로 자동 진행하지 않는지.
4. 차원술사 T `2050500`: 11m clamp, invalid red marker, LMB confirm, RMB cancel,
   Server-approved target의 Effect/damage 일치.
5. 도화가 A/S/R/D/E/T/V, 워로드 T, 창술사 E/V/ALT_V/T, 차원술사 A/F의
   크기·방향·UV·색·dissolve·screen overlay.

자동 검증은 연결과 rollback, build, 수치 계약까지만 PASS다. first pixel,
원본 색감과 occurrence visual fidelity는 사용자 서면 판정 전까지 `PENDING`이다.
