# Release Effect 로딩 응답성과 compiled shader 폐쇄 구현 결과

기준일: 2026-08-27

브랜치: `codex/release-effect-loading-responsiveness`

기준 commit: `88dfd58abfb28fc52078d3bcf72f2b4d35eee7b1`

## 0. 현재 admission 상태

| 항목 | 상태 | 근거 |
|---|---|---|
| CSO build/deploy/runtime 계약 | 구현 완료 | runtime source compiler 제거, active producer 총 23개 closure 검증 |
| Character Select/Valtan Effect 준비 worker stage | 구현 완료 | single-slot command mailbox, capacity-2 result channel, exact target ACK 사용 |
| main-thread bounded commit과 실제 진행률 | 구현 완료 | worker device-stage, renderer swap/per-key receipt commit, fake 90% 제거 |
| Debug focused build/harness | PASS | 무캐시 CSO 재생성, Effect/PointLight/closure PASS |
| Release focused build/harness | PASS | 무캐시 CSO 재생성, Effect/PointLight/closure PASS |
| Debug 정본 full regression | 기준선 실패 분리 | 이번 변경 밖의 기존 gate drift와 Server/CS simulation failure가 존재 |
| Release 정본 full regression | 기준선 실패 분리 | build/Effect closure는 PASS, 기존 source assertion 2건과 CS combat timeout으로 exit 1 |
| 사용자 Release/Debug visual smoke | 미실행 | 사용자 직접 `Framework.sln` 실행·육안 판정 필요 |

사용자의 화면 확인 전에는 Character Select 최종 화면, `응답 없음` 제거와 visual fidelity를 PASS로
기록하지 않는다.

## 1. 원인 결론

문제가 보인 시점에는 Lobby의 Server 승인과 Character Select Loading Level 진입이 이미 끝나 있었다.
직접 병목은 `CShader::Initialize_Prototype()`가 실행 시 HLSL을 다시 컴파일하고, Product Effect renderer
core의 큰 shader들을 메인 스레드에서 연속 생성한 것이다.

- Debug/Release의 Loader thread 구조가 달랐던 것이 아니다.
- Release는 같은 source compile에 최적화를 적용해 한 shader가 훨씬 오래 걸렸다.
- `CMainApp::Update()`가 반환하지 않으므로 Win32 message pump, loading animation과 진행률 poll이 함께
  멈춰 Windows가 `응답 없음`으로 표시했다.
- 기존 Loading UI의 시간 기반 90% 표시는 실제 Effect 단계와 무관해 어느 작업에서 기다리는지도
  숨겼다.

따라서 `127.0.0.1`, Character Select 승인 `E_FAIL`, packet parse가 이번 정지의 직접 원인은 아니었다.

## 2. 실제 구현

### 2.1 build-time CSO와 fail-closed runtime

- Engine runtime의 `D3DX11CompileEffectFromFile` 정상 경로를 제거했다.
- 실행 module 옆의 같은 basename `.cso`만 읽어 `D3DX11CreateEffectFromMemory`로 Effect를 만든다.
- 파일 크기는 1 byte 이상 256 MiB 이하로 제한한다.
- Effect/technique/pass/signature/input-layout을 stage별로 검증하고 논리 HLSL ID, module 경로, CSO 경로,
  byte 수, HRESULT와 경과 시간을 진단에 남긴다.
- 누락·빈 파일·손상 bytecode를 source compile로 우회하지 않는다.
- Engine의 `Shader_Cell.cso`, `Shader_Deferred.cso`는 `UpdateLib.bat`과 Client AfterBuild에서 구성별
  Client output에 복사한다.
- Client가 소비하는 V2 Effect shader를 포함해 구성별 `FxCompile` producer를 프로젝트에 등록했다.

`Tools/Build/Test-CompiledShaderClosure.ps1`는 다음을 검사한다.

1. Debug/Release x64에서 활성화되는 producer output이 유일하고 실제 존재하는가
2. runtime과 두 하네스에 source compiler 호출이 다시 들어오지 않았는가
3. Client, EffectRenderContractHarness와 PointLightFalloffContractHarness의 module-adjacent CSO가
   producer와 SHA-256까지 같은가
4. 구성별 project/filter와 deploy 경로가 producer/consumer closure를 이루는가

Effect harness에는 Client 비링크 `ProjectReference`를 추가했다. 따라서 clean clone의 직접 project 빌드나
solution 병렬 빌드에서도 Client의 전체 CSO 생성과 deploy가 끝난 뒤에만 하네스가 `Shader_*.cso`를
수집한다. Client EXE나 import output을 하네스에 링크하지는 않는다.

### 2.2 Loader worker stage와 main-thread commit

`CEffectLoadPreparationJob`은 latest-value single-slot command mailbox, capacity-2 result channel과 exact
one-target ACK를 소유한다.
Loading owner는 target snapshot을 worker에 넘기고 다음을 worker에서 수행한다.

- Effect 문서 읽기와 parse/validate
- catalog/registry/overlay immutable stage
- budget/duration 산출
- renderer core와 target GPU resource의 device-only 준비

worker는 D3D11 immediate context method를 호출하지 않는다. target document/projection, prepared-target
index, receipt와 queue terminal state는 main만 publish한다. 단, revision-independent이고 device-scoped인
immutable renderer core와 그 실패 latch는 중복 빌드를 막기 위한 cache 예외로 worker가 mutex 아래
populate할 수 있다. 이 cache는 Effect target/catalog commit이 아니며 이후 target stage가 exact core
identity를 CAS한다.

main은 queue token과 catalog/registry generation, source identity, device/core identity를 다시 검증한 뒤
준비된 결과를 commit한다. 새 catalog document/projection을 먼저 삽입한 뒤 renderer CAS가 실패하는
경우에는 exact commit receipt로 그 삽입만 rollback한다. budget/duration/overlay receipt는 전역 map 세 개를
target마다 복사하지 않고 해당 key의 node/값만 미리 stage한 뒤 O(log N) per-key commit한다. stale result는
기존 target 상태를 유지한 채 거부한다.

ACK 전에는 worker가 준비 결과를 계속 소유한다. main이 자신의 참조를 먼저 해제하고 ACK하므로 교체된
renderer map/session의 무거운 마지막 소멸도 worker에서 일어난다. cancel/rebase/close는 bounded join과
fail-fast 종료 계약을 따른다.

현재 이 worker 준비 경로의 제품 범위는 `CHARACTER_SELECT`와 `VALTAN_ARENA` Loading이다. runtime class
change, Bern과 Debug Valtan spawn의 기존 lazy 경로까지 모두 worker화했다고 주장하지 않는다.

### 2.3 진행률, activation과 종료 순서

- 시간 경과만으로 90%를 표시하던 fake progress를 제거했다.
- worker phase, 완료/전체, 현재 target ID와 상태 문자열을 thread-safe snapshot으로 노출한다.
- `completed == total`만으로 100%를 표시하지 않는다. Level worker 성공, target terminal 상태, main commit
  비어 있음과 최종 readiness가 모두 확인된 경로만 100%를 publish한다.
- ACK 직후에는 `Finalizing...`을 표시하고 epoch/close 완료 전의 가짜 완료 정체를 막는다.
- Loader wait의 `WAIT_FAILED`도 무시하지 않고 전용 진단과 fail-fast로 처리한다.
- 종료는 ObjectManager가 살아 있을 때 `Clear_All()`을 먼저 수행하고, Engine이 Loading Level을 파괴해
  worker를 join한 뒤 Effect prepared cache/catalog를 해제한다. worker와 global cache clear의 race를
  제거했다.

## 3. 회귀 방지 하네스

EffectRenderContractHarness에 다음 계약을 추가했다.

- load job 정상/취소/ACK/close와 result lifetime
- queue stage/commit과 stale token rollback
- catalog parse/validate/stage/commit, duplicate와 source identity CAS
- renderer two-phase stage/commit과 stale device/generation 거부
- compiled shader 누락/손상 fast failure
- compiled renderer core 최대 10초와 renderer target swap 최대 100,000 us admission

`Invoke-BuildAndRegression.ps1`은 Debug/Release마다 Client 뒤 Effect/PointLight 하네스를 빌드·배포하고,
compiled shader closure를 통과한 뒤 두 실행형 하네스를 실행한다. stale local CSO만 남아 있는 상태는
closure hash gate가 실패한다.

최신 main의 Effect Data project에는 이미 tracked V2 데이터와 project/filter registration 사이 drift가
있었다. 정본 `Sync-EffectDataProject.ps1`로 현재 2,395 files/219 filters를 동기화하고 `-Check`를 gate로
유지했다.

## 4. 자동 검증 증거

### 4.1 Debug fresh-output CSO 재생성

정확히 23개 producer output을 별도 임시 backup으로 이동한 뒤 Engine → UpdateLib → Client를 다시
빌드했다.

```text
CSO_REGEN_CONFIGURATION=Debug
CSO_REGEN_PRODUCERS=23
CSO_REGEN_BYTES=61613221
ENGINE_CSO_BUILD_SECONDS=1.048
CLIENT_CSO_BUILD_SECONDS=76.971
```

생성 뒤 Debug closure 결과:

```text
Compiled shader closure PASS for Debug|x64
active FxCompile producers : 23
Client consumers : 22
EffectRenderContractHarness consumers : 22
PointLightFalloffContractHarness consumers : 1
```

Engine CSO를 다시 만든 뒤 아직 갱신하지 않은 PointLight harness 복사본은 실제로 hash mismatch로
거부됐다. PointLight를 다시 빌드한 뒤 closure가 PASS했으므로 stale output 검출도 실증했다.

### 4.2 Debug Effect/PointLight focused 결과

```text
EffectRenderContractHarness Debug build: 7.576 s
effectLoadJobContractValidated: true
effectPrewarmQueueTransactionValidated: true
effectCatalogLoadStageTransactionValidated: true
effectRendererStageTransactionValidated: true
compiledShaderFastFailureValidated: true
compiledShaderMaximumFailureMs: 0
coreBuildCount: 1
coreBuildMaximumMicroseconds: 766090
targetPrepareCount: 2
targetPrepareMaximumMicroseconds: 626588
targetCommitCount: 1
targetCommitMaximumMicroseconds: 28
```

기존 조사에서 source shader 하나의 Debug compile은 약 25초였고 Release compile은 2분을 넘겨 수동
중단됐다. 변경 뒤 Debug에서는 compiled core 전체가 0.766초였고 renderer target validation/swap은
28 us였다. 이 28 us는 catalog/receipt/queue를 포함한 전체 main-thread consume 시간이 아니다. 이는 같은
장비의 조사/구현 전후 실측이며 다른 장비의 절대 성능을 보장하는 수치는 아니다.

PointLightFalloffContractHarness도 Debug PASS했다.

### 4.3 Debug full regression에서 분리된 기준선 실패

Debug full 명령은 400.412초 실행됐고 Engine/UpdateLib/Shared/Server/Client 및 두 shader harness build,
compiled shader closure, Effect source/project validation, Valtan pipeline 49 tests와 protocol harness를
통과했다. 그러나 전체 admission은 PASS가 아니다.

현재 branch 변경과 무관한 기준 HEAD 파일에서 다음 실패가 확인됐다.

1. `test_valtan_floor_emissive_contract.py`가 제거된 member `m_fEmissiveIntensity` 문자열을 기대하지만
   제품 코드는 local staged `emissiveIntensity`를 사용한다.
2. `test_ground_target_preview_prototype_scope.py`가 인자 없는 `Commit_Submission()` marker를 기대하지만
   제품 helper는 typed stage 인자를 받는다.
3. Server gameplay contract의 `Start all four Valtan fight pages...`가 실패한다.
4. Character Select isolation은 두 private session의 입장·snapshot 격리는 PASS했지만 client A가
   spawned monster의 34650 hit reach로 이동하는 단계에서 4초 timeout이 반복된다.

이 네 경로와 관련 source/test 파일에는 이번 branch diff가 없다. 특히 네 번째 결과는 Server 연결과
Character Select 입장 자체의 실패가 아니라 입장 뒤 Server movement/combat simulation의 기준선 실패다.
다른 세션의 실제 compiler response file도 확인했으며 출력은 별도
`LostArk-worktrees/valtan-jump-balance-fix` 아래였다. 따라서 위 Debug 실패는 동일 출력 폴더의 병렬 링크나
publisher 간섭으로 설명되지 않는다.

### 4.4 Release fresh-output와 focused 결과

Release output에 있던 기존 CSO 26개를 별도 임시 backup으로 이동한 뒤 Engine → UpdateLib → Client →
Effect harness → PointLight harness를 다시 빌드했다. 증분 시간은 admission으로 사용하지 않았다.

```text
CSO_REGEN_CONFIGURATION=Release
CSO_REGEN_PREEXISTING_MOVED=26
CSO_REGEN_ENGINE_COUNT=2
CSO_REGEN_CLIENT_COUNT=23
CSO_REGEN_CLIENT_BYTES=5181205
CSO_REGEN_EFFECT_COUNT=23
CSO_REGEN_POINT_COUNT=1
ENGINE_CSO_BUILD_SECONDS=1.32
UPDATELIB_SECONDS=0.522
CLIENT_CSO_BUILD_SECONDS=521.627
EFFECT_HARNESS_BUILD_SECONDS=71.889
POINT_HARNESS_BUILD_SECONDS=6.628
```

Release closure와 두 실행형 harness는 PASS했다.

```text
Compiled shader closure PASS for Release|x64
active FxCompile producers : 23
Client consumers : 22
EffectRenderContractHarness consumers : 22
PointLightFalloffContractHarness consumers : 1

effectLoadJobContractValidated: true
effectPrewarmQueueTransactionValidated: true
effectCatalogLoadStageTransactionValidated: true
effectRendererStageTransactionValidated: true
compiledShaderFastFailureValidated: true
compiledShaderMaximumFailureMs: 0
coreBuildMaximumMicroseconds: 183772
targetPrepareMaximumMicroseconds: 68920
targetCommitMaximumMicroseconds: 0
PointLightFalloffContractHarness: PASS
```

`targetCommitMaximumMicroseconds`는 전체 Loading result consume 시간이 아니라
`Commit_VisualProgramTargetStage()`의 renderer validation/swap 구간만 측정한다. 따라서 이 수치를 전체
main-thread frame 점유 시간이라고 기록하지 않는다. 전체 경로는 target당 세 전역 map 복사를 제거하고
per-key receipt commit으로 제한했으며, 실제 창 응답성과 end-to-end frame pacing은 사용자 smoke 경계다.

### 4.5 Release full regression에서 분리된 기준선 실패

정본 Release 명령은 Engine/UpdateLib/Shared/Server/Client와 두 shader harness를 빌드하고 Release compiled
shader closure, Valtan pipeline 49 tests, protocol과 Valtan four-player live harness까지 통과했다. 전체
명령은 다음 현재 기준선 실패 때문에 exit 1이었다.

1. `test_valtan_floor_emissive_contract.py`의 제거된 member 문자열 assertion
2. `test_ground_target_preview_prototype_scope.py`의 인자 없는 helper marker assertion
3. Character Select 두 private session 입장/snapshot PASS 뒤, client A의 34650 hit-reach 접근 4초 timeout

세 번째는 한 번의 wrapper 안에서 같은 admission을 두 번 실행해 동일하게 실패했다. Release Server 접속,
world 승인, Character Select private session 생성과 snapshot parse는 모두 timeout 이전에 PASS했다. 이번
branch는 해당 Server/Shared/Data/harness 파일을 수정하지 않는다.

### 4.6 최종 focused 재검증

catalog exact rollback과 per-key receipt 보완 뒤 Debug/Release Client와 Effect harness를 다시 빌드했다.
최초 Release 실행에서 하네스가 version 13 문서에도 visual-program projection이 필수라고 가정한 테스트
오류를 검출했다. 제품 계약은 version 13 projection을 optional로 허용한다. 하네스의 공통 document
rollback은 optional projection을 허용하도록 교정하고, version 15
`effect.artist.skill.31950.unified`를 사용해 document와 projection의 동시 commit/rollback을 별도로
검증했다.

```text
Release Client + EffectRenderContractHarness build: 오류 0
Release EffectRenderContractHarness: PASS
  effectLoadJobContractValidated: true
  effectPrewarmQueueTransactionValidated: true
  effectCatalogLoadStageTransactionValidated: true
  effectRendererStageTransactionValidated: true
  compiledShaderMaximumFailureMs: 16
  coreBuildMaximumMicroseconds: 233591
  targetPrepareMaximumMicroseconds: 97659
  targetCommitMaximumMicroseconds: 0

Debug Client + EffectRenderContractHarness build: 오류 0
Debug EffectRenderContractHarness: PASS
  effectLoadJobContractValidated: true
  effectPrewarmQueueTransactionValidated: true
  effectCatalogLoadStageTransactionValidated: true
  effectRendererStageTransactionValidated: true
  compiledShaderMaximumFailureMs: 15
  coreBuildMaximumMicroseconds: 777242
  targetPrepareMaximumMicroseconds: 603648
  targetCommitMaximumMicroseconds: 24

Compiled shader closure Debug/Release: PASS
  active FxCompile producers: 23
  Client consumers: 22
  EffectRenderContractHarness consumers: 22
  PointLightFalloffContractHarness consumers: 1

Validate-EffectSources.ps1: PASS
  directSourceCount: 196
  resourceFileCount: 1028
Sync-EffectDataProject.ps1 -Check: PASS (files=2395, filters=219)
project/filter XML parse: PASS
build PowerShell parse: PASS
git diff --check: PASS
```

두 focused 구성에서 renderer target commit admission은 100,000 us 이내였다. 이 수치는 앞서 명시한
대로 renderer validation/swap 구간이며 전체 main-thread frame 시간으로 확대 해석하지 않는다.

## 5. 남은 검증

자동:

- 이 변경 범위의 focused Debug/Release build, 실행형 harness, shader closure와 정적 검사는 완료됐다.
- 정본 full regression의 위 기준선 실패는 이 PR에서 assertion 완화나 Server 우회로 숨기지 않음

사용자 수동:

1. `Framework.sln`에서 `Release | x64`를 정본 순서로 빌드한다.
2. Server + Client profile을 시작한다.
3. Lobby → Character Select를 누른다.
4. loading progress가 실제 target/phase로 갱신되고 창이 응답하는지 확인한다.
5. Character Select visual map과 character 화면에 최종 진입하는지 확인한다.
6. Lobby 복귀 뒤 같은 revision 두 번째 진입을 확인한다.
7. 필요하면 같은 경로를 Debug에서도 확인한다.

이 사용자 관찰이 PASS한 뒤 PR의 visual/manual admission을 완료하고 merge 여부를 확정한다.
