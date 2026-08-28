# 베른 프러스텀 컬링 진단·안정화 결과

## 2026-08-28 LEFT 평면 수치 결함 수정

### 최종 결과 요약

**베른 맵 소멸을 일으키는 LEFT 프러스텀 평면의 수치 결함을 정본 폴더에 수정 반영했다.** CPU 컬링이 화면 안의 배치를 잘못 제외한 문제이며, 이번 수정에서 depth test나 맵 표면 높이는 변경하지 않았다.

| 구분 | 마감 상태 |
|---|---|
| 베른 컬링 계산 수정 | 구현 완료 |
| 수정 코드 수치 회귀 및 Client Debug/Release 빌드 | 아래 실행 증거 기준 PASS |
| 공식 전체 build/regression | 실행 당시 별도 Valtan `finale` 검증 오류로 완료하지 못함 |
| 수정 후 베른 실제 화면 | 사용자 관찰 미수신. 시각 해결 PASS로 기록하지 않음 |
| Character Select 모서리 일렁임 | geometry 겹침 후보 분석과 검사 추가. 표면/깊이 수정은 미적용 |

사용자의 “결과 보고서만 작성하고 더 검증하지 않아도 된다”는 요청에 따라 추가 검증을 중단하고 이미 확보한 증거로 이 보고서를 마감했다. 진행 중이던 정본 Debug `-SkipBuild`는 신규 Bern/native/surface 검사 통과 뒤 별도 Valtan 회귀 구간에서 이 작업이 시작한 프로세스와 하위 프로세스만 종료했다. 따라서 **부분 검사 통과와 전체 회귀 완료를 구분**한다. 추가 빌드·Client 실행·자동 화면 검사는 수행하지 않는다.

### 이번 변경과 상태

생산 변경은 [MapAssetRenderUtils.h](C:/Users/user/Desktop/LostArk/Client/Public/MapAssetRenderUtils.h)와 [MapAssetRenderUtils.cpp](C:/Users/user/Desktop/LostArk/Client/Private/MapAssetRenderUtils.cpp)에 한정했다.

- far corner를 기준으로 near corner 두 개를 외적하던 `MakePlanes`를 제거했다.
- 실제 float View×Projection의 column에서 여섯 outward clip plane을 double 중간 연산으로 추출·정규화한다. shader에 전달하는 원래 View/Projection은 그대로 보존한다.
- `Build_CameraCullSnapshot`이 유한성·가역성·plane 검증을 마친 뒤에만 output을 commit한다. `Capture_CameraCullSnapshot`은 성공한 행렬과 평면을 cache하고 그때만 revision을 갱신한다.
- sphere가 완전히 분리된 `distance > effectiveRadius + tolerance`일 때만 raw reject한다. 입력 실패 시 decision과 placement grace는 부분 변경하지 않는다.
- plane의 unit-normal 허용 오차도 float 반올림 범위로 제한했다. 약간 확대된 normal이 접구를 제외하는 별도 입력 반례를 실패/rollback 검사에 고정했다.
- 기존 0.25m/대형 geometry margin, 3회 reject grace, 제품 `bypass=false`, `diagnostics=false`는 유지했다.

이번 변경에서 `MainApp`, `MapAssetObject`, `MapStaticBatchObject`, Engine, shader, depth format/test, near/far와 맵 배치는 수정하지 않았다. 새 ImGui/F1 진단 패널도 제품에 등록하지 않았다. 기존 보수적 invalid-input draw fallback은 유지한다. Camera shader binding은 기존 HRESULT를 불필요하게 E_FAIL로 바꾸지 않고 반환한다.

### 원인과 간헐성의 실행 증거

수정 전 LEFT는 `XMPlaneFromPoints(farTL, nearTL, nearBL)`이었다. near 0.1m / far 13,084.808594m에서 길고 거의 평행한 float 벡터의 외적이 작은 차이를 잃는다. 틀어진 normal로 far corner에 대한 상수항까지 계산해 평면 위치 오차가 크게 증폭된다. 앞 placement가 CPU에서 제외되면 그 뒤 geometry가 보이므로, 이 증상만으로 depth test 오류라고 결론내릴 수 없다.

한 고정 입력에서 기존 LEFT 거리는 **+45.7138748m**, 수정 후는 **-5.73005676m**였다. 독립 double clip reference 안에 있는 반경 0.5m sphere를 기존 0.25m margin까지 포함한 판정이 잘못 제외했다.

| 실제 native fixture | 기존 식 오판 | 생산 수정식 오판 |
|---|---:|---:|
| free/follow 카메라 773개, 광축 8m 앞 sphere | 268 | 0 |
| float `at-eye`를 포함한 follow 좌표 1,689개 | 626 | 0 |
| 같은 near/far의 미세 이동·각도 변화 72개 | 37 | 0 |

773개 사례의 최초 독립 실험은 267건이었다. 최종 harness가 follow 5개를 실제 float `at-eye` 계산으로 바꾸면서 기존 식 오판이 268건이 됐다. 같은 세트를 임의로 바꿔 성공률을 주장한 것이 아니며 최종 검증 수치는 위 표다.

동일 입력 100회는 동일 결과였다. 미세 변화에서는 기존 판정이 뒤집힌 두 그룹을 확인했다. 또 최초부터 rejected인 state는 즉시 제외되지만, 직전에 visible이었던 state는 세 번 더 렌더되고 네 번째 reject에서 제외된다. 따라서 간헐성은 **카메라 입력의 반올림 변화와 이전 visibility 이력**으로 재현된다. grace는 현재 평가 호출 횟수이며 camera revision은 frame 번호가 아니다.

LEFT 평면 결함이 화면 왼쪽 경계에서만 영향을 준다고 제한할 수 없다. 잘못 이동한 평면은 위처럼 광축 앞 sphere도 제외한다. 원거리·작은 radius·작은 margin이 위험을 바꿀 수 있지만, 거리 하나만으로 발생을 단정하지 않는다. far=2,000m에서 일부 실험이 통과했다는 사실도 모든 자세에서 안전하다는 임계값 증명은 아니다.

### 자동 검증

[MapFrustumContractHarness](C:/Users/user/Desktop/LostArk/Tools/MapFrustumContractHarness/Private/MapFrustumContractHarness.cpp)는 실제 생산 CPP를 컴파일한다. independent clip oracle과 기존 수식은 test 안에만 있다.

| 항목 | 실행 결과 |
|---|---|
| 하네스 x64 Debug / Release compile·link | 각각 exit 0 |
| 하네스 x64 Debug / Release 직접 재실행 | 각각 2,753 assertions, failures 0, exit 0 |
| 6개 plane, 접구, near/far 2,000·13,084.808594·40,000, outside reject | PASS |
| singular/NaN/Inf/zero revision·radius·잘못된 policy·normal | 실패 이유와 output/state rollback PASS |
| 앞의 두 plane 생성 뒤 세 번째 plane에서 실패하는 입력 | 부분 commit 없음 PASS |
| fresh/warm grace, bypass, landscape/large margin | PASS |
| 정적 Bern policy/소비자 연결 검사 | PASS |
| 신규 project/filters XML 및 build script PowerShell AST | PASS |
| 기존 MapEffect publisher/rollback unit tests | 6 tests PASS |
| wrapper 격리 fixture | 9건 PASS: native 0/37 보존, stdout/stderr 각각 24,130 byte 일치, 누락 EXE/DLL는 exit 2 |
| wrapper 60초 timeout | 미실행. 코드의 bounded wait/owned process cleanup만 검토 |
| 정본 전체 Debug build/regression | Server pre-build에서 중단: ValtanEncounter의 `finale`를 기존 WorldGameplay validator가 거부. exit 1 |
| 정본 Debug `-SkipBuild` 연결 검사 | Bern 정적 계약, native 2,753 assertions/0 failures, surface 21 tests PASS. 뒤의 별도 Valtan 회귀는 사용자 요청으로 중단하여 전체 결과 미완료 |
| 정본 Client x64 Debug | compile·link·post-build 배포 PASS, exit 0 |
| 정본 Client x64 Release | compile·link·post-build 배포 PASS, exit 0 |
| 정본 전체 Release build/regression | 초기 UpdateLib 공유 파일 점유로 중단. 점유 해제 뒤 재시도는 UpdateLib 통과 후 같은 Server `finale` validator 오류, exit 1 |
| Client 실행·GPU/화면 검사 | 미실행, 사용자 전용 |

실제 명령:

```powershell
powershell -ExecutionPolicy Bypass -File Tools/MapFrustumContractHarness/Run-MapFrustumContractHarness.ps1 -Configuration Debug
powershell -ExecutionPolicy Bypass -File Tools/MapFrustumContractHarness/Run-MapFrustumContractHarness.ps1 -Configuration Release
powershell -ExecutionPolicy Bypass -File Tools/ProjectAudit/Test-BernFrustumCullingContract.ps1
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release
```

신규 프로젝트는 [Framework.sln](C:/Users/user/Desktop/LostArk/Framework.sln)과 [정본 build/regression](C:/Users/user/Desktop/LostArk/Tools/Build/Invoke-BuildAndRegression.ps1)에 build·SkipBuild 존재 확인·실행 gate를 함께 등록했다. 같은 회귀에 Character Select surface synthetic tests도 연결했다. [빌드 사용서](C:/Users/user/Desktop/LostArk/Tools/Build/README.md)의 오래된 자동 Client smoke 설명을 실제 headless 실행 범위에 맞췄다.

기존 C4819/C4828 인코딩 및 LNK4099 PDB 경고는 남아 있다. 이번 작업은 기존 C++의 UTF-8/CRLF를 유지했고, 경고가 없다고 기록하지 않는다.

로컬 로그는 [.codex_tmp/bern_frustum_shadow_20260828](C:/Users/user/Desktop/LostArk/.codex_tmp/bern_frustum_shadow_20260828), [.codex_tmp/bern_frustum_sweep_20260828](C:/Users/user/Desktop/LostArk/.codex_tmp/bern_frustum_sweep_20260828)에 보존했다. wrapper fixture 결과는 [results.json](C:/Users/user/Desktop/LostArk/.codex_tmp/map_wrapper_verification_20260828/results.json)이다. 산출물·EngineSDK·로그는 커밋하지 않았다.

공식 회귀의 신규 gate 실행 및 중단 전 기록은 [Debug SkipBuild 로그](C:/Users/user/Desktop/LostArk/.codex_tmp/bern_frustum_sweep_20260828/official-debug-skipbuild-regression.log)에 보존했다. 사용자 중단 요청 전에 실행한 `git diff --check`도 통과했다. 중단 이후에는 결과 문서만 마무리했으며 검사를 다시 실행하지 않았다.

### 남은 사용자 확인과 별도 증상

이 하네스의 실행 범위는 builder/predicate다. `Capture`의 GameInstance/cache 연결은 코드 검토 및 Client 컴파일 범위이고, 실제 map bounds 생성·instance buffer 업로드·GPU draw·최종 화면을 자동 통과했다고 기록하지 않는다. 수치 결함 수정은 완료했지만 **베른 시각 소멸 해결의 최종 확인은 아직 없다**.

서버와 Client 빌드가 정상 실행 가능한 상태에서 사용자가 Visual Studio **Server + Client** profile → **Ctrl+F5**로 시작한다. Client cwd는 `Client/Default`다. Lobby에서 Server 승인 후 Bern에 들어가 동일 장소의 follow 이동/정지, F6 free 전환 후 이동/회전, F6 follow 복귀와 재진입을 확인한다. `bypass=false`인 그대로 재발 여부를 글로 남긴다. 기존 전환 로그는 기본 비활성이며 새 F1 메뉴는 없다.

Character Select는 [별도 RESULT](C:/Users/user/Desktop/LostArk/.md/GB/08-09/2026-08-09_CHARACTER_SELECT_CENTER_DEPTH_SEPARATION_RESULT.md)에 실제 geometry 검사 21 unit tests, 여섯 겹침 후보, 803개 ID와 기존 중앙 보정 보존을 기록했다. 이번에는 표면 Y, mip, shader, near/far를 바꾸지 않았고 camera depth/모서리 육안 해결은 미확인이다.

정본 폴더의 다른 Valtan 작업 변경은 보존했다. stage/commit/checkout하지 않았으며 이 결과를 다른 작업의 전체 검증 또는 사용자 visual PASS로 승격하지 않는다.

---

## 이전 안정화 이력

아래 08-25~08-28 기록은 당시의 구현·우회 상태 사용자 관찰이다. 위 수학 수정 뒤 정상 컬링 상태의 새 화면 검증을 대신하지 않는다.

## 구현 상태

- 베른 전용 프러스텀 우회/진단과 원인 격리: 완료
- 제품 기본 프러스텀 컬링 재활성화: 완료
- stable placement ID와 경계구·여섯 평면 거리 기록: 완료
- binary static model 전체 decoded vertex 기준 경계 재계산: 완료
- 컬링과 렌더의 동일 View/Projection 스냅샷 사용: 완료
- Landscape 및 대형 floor/bridge geometry 보수적 margin: 완료
- 3-frame rejection hysteresis: 완료
- 우회 상태 사용자 화면 판정: PASS(깜빡임 소멸)
- 정상 컬링 재활성화 후 사용자 화면 판정: 대기

## 확인된 원인

사용자가 플레이어를 멈추고 F6 자유 카메라만 움직인 상태에서도 깜빡임을 재현했고, 베른 프러스텀 우회를 켜자 깜빡임이 사라졌다. 따라서 내비게이션 높이 또는 플레이어 위치 변경이 아니라 맵 배치의 프러스텀 false rejection이 직접 원인이다.

우회 실행에서 `VISIBLE_TO_REJECTED` 전환 157,729건이 기록됐고, 이 중 반경 4m 이상 또는 Landscape로 분류한 대형 geometry가 20,176건이었다. 잔디뿐 아니라 `BREEZE_BRIDGE`, `FLOOR`, 성벽과 기둥 asset도 측면 평면 경계에서 반복 탈락했다.

기존 경로에는 세 문제가 겹쳐 있었다.

1. fallback과 static batch가 `Late_Update`의 Engine frustum으로 먼저 탈락했지만 실제 shader View/Projection은 뒤의 Render에서 다시 읽었다. 특히 follow camera가 `Late_Update`에서 갱신되는 프레임에는 컬링과 실제 렌더 카메라의 시점이 달라질 수 있었다.
2. binary static model은 embedded bounds가 있으면 전체 decoded vertex를 순회하지 않아, 잘못되거나 축소된 metadata가 대형 바닥·다리 경계구를 작게 만들 수 있었다.
3. 대형 지형에도 일반 배치와 같은 날카로운 구-평면 판정을 사용해 화면 가장자리의 작은 행렬·경계 오차가 즉시 한 프레임 소멸로 이어졌다.

현재는 final Render에서 한 View/Projection 스냅샷으로 평면 판정과 shader 바인딩을 함께 수행하고, 전체 decoded vertex 경계, 대형 geometry margin, 3-frame rejection hysteresis를 적용한다.

## 런타임 동작

진단 실행에서는 베른에 들어갈 때 로그를 새로 시작하고, 프러스텀 계산상 탈락한 배치도 실제로는 계속 렌더한다. 계산 상태가 visible에서 rejected로 바뀌면 다음 파일에 한 줄을 추가한다.

`Client/Bin/Debug/Diagnostics/BernFrustumCulling.log`

각 행은 `cameraRevision`, `assetId`, `groupId`, `placementId`, `center`, `baseRadius`, `margin`, `effectiveRadius`, `largeGeometry`, `planeDistances[6]`를 포함한다.

원인 확인 뒤 제품 기본값은 `bypass=false`, `diagnostics=false`로 전환했다. 따라서 평상시에는 프러스텀 컬링이 다시 동작하고 대량 진단 로그를 쓰지 않는다. 진단 구현과 기존 확인 로그는 후속 재현에 사용할 수 있도록 유지한다.

## 자동 검증

- Engine x64 Debug: PASS
- Client 변경 파일 compile: PASS
- Client x64 Debug link: PASS
- 2026-08-26 정상 컬링 재활성화 Client x64 Debug rebuild: PASS
- 정적 계약 하네스: PASS
- 변경 파일 대상 `git diff --check`: PASS

`UpdateLib.bat Debug`는 Engine DLL/LIB와 헤더를 복사했으나 첫 실행에서 기존 PDB 쓰기 공유 오류를 반환했다. 이어진 Client 빌드 post-build가 Engine DLL/PDB를 정상 배포했고 Client 링크는 PASS했다.

## 수동 검증 경계

에이전트는 Client를 실행하거나 화면을 대신 판정하지 않는다. 우회 상태에서 깜빡임이 사라졌다는 사용자 판정은 기록했다. 이제 정상 컬링을 다시 켠 빌드에서 같은 재현 위치에 플레이어를 멈추고 F6 자유 카메라만 움직였을 때 깜빡임이 재발하지 않는지 사용자가 확인해야 한다.

## 2026-08-28 PR 병합 전 교정

PR #247의 최신 `main` 합성 병합 트리에서 Bern 전용 안정화 값이 모두 0으로 바뀐 것을
회귀로 확인했다. 우회 실행으로 false rejection을 확인한 위 증거와 맞지 않으므로 제품값을
`baseMargin=0.25`, `largeObjectRadiusThreshold=4`,
`largeObjectAbsoluteMargin=2`, `largeObjectRelativeMargin=0.12`,
`rejectHysteresisFrames=3`으로 복원했다. 정적 계약도 필드 존재만 보던 방식에서
`MakeBernMapScope`의 이 다섯 수치를 정확히 검사하도록 강화했다.

자동 계약은 다시 통과했지만 정상 컬링 상태의 최종 화면 판정은 여전히 사용자 확인 항목이다.
