# 베른 카메라 이동 중 맵 소멸: 프러스텀 컬링 진단·수정 구현 계획서

- 갱신일: 2026-08-28
- 문서 종류: 구현 계획서. 최신 요청에 따라 정본 폴더에서 핵심 프러스텀 수학 수정과 실행형 회귀를 먼저 반영한다. 추가 ImGui/GPU 진단은 사용자 재확인 뒤 필요한 경우의 별도 단계다. Client 실행·UI 조작·화면 캡처와 최종 육안 판정은 사용자 전용이다.
- 기존 08-25 계획의 후속 범위이므로 파일명을 유지한다. 이전 구현·검증 이력은 [08-25 RESULT](C:/Users/user/Desktop/LostArk/.md/GB/08-25/2026-08-25_BERN_FRUSTUM_CULLING_DIAGNOSTIC_AND_STABILITY_RESULT.md)에 보존한다.
- 함께 진행할 별도 증상: [Character Select 모서리 표면 일렁임 계획](C:/Users/user/Desktop/LostArk/.md/GB/08-09/2026-08-09_CHARACTER_SELECT_CENTER_DEPTH_SEPARATION_IMPLEMENTATION_PLAN.md).

## G00. 현재 동작과 이번 수정의 목표

### 증상을 구분하는 기준

프러스텀 컬링은 카메라 밖이라고 판정한 **객체 또는 배치 인스턴스의 draw를 생략하는 CPU 판정**이다. 현재 맵 경로는 삼각형을 프러스텀 경계에서 잘라 새 mesh를 만들지 않는다. 프러스텀과 일부라도 겹치는 객체는 그리기 대상으로 남기며, 화면 경계·near/far 경계의 삼각형 clipping은 이후 GPU가 수행한다. 삼각형의 앞·뒷면 culling도 이 판정과 별개다. [Microsoft D3D11 rasterizer 계약](https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_rasterizer_desc)

따라서 화면 안의 지형이 덩어리째 사라지는 현상은 **잘못된 경계구/카메라 행렬/컬링 판정으로 발생할 수 있다**. 다만 draw 자체의 실패, GPU clipping, 면 방향이나 depth 때문에도 비슷하게 보이므로 현상 설명만으로 원인을 확정하지 않는다.

목표는 정상 컬링을 켠 상태에서, 실제 카메라 프러스텀과 교차하는 베른 배치가 잘못 제외되지 않도록 만드는 것이다. 전역 컬링 해제, 무제한 반경 확대, hysteresis 연장만으로 완료 처리하지 않는다.

앞 mesh가 사라지고 뒤 mesh가 보이는 증상도 depth test 오류만의 증거가 아니다. 앞 mesh의 draw를 CPU에서 생략하면 뒤 mesh가 자연스럽게 남는다. 따라서 depth format이나 LESS_EQUAL을 먼저 바꾸지 않고 아래 단계별 수치로 누락 지점을 확정한다.

### 후속 조사에서 재현한 수치 결함과 간헐성

수정 전 `MakePlanes`의 LEFT 평면은 far corner 4를 기준으로 near corner 0/3을 잇는 긴 두 벡터의 외적을 사용한다. 베른 near 0.1m / far 약 13,084.81m에서는 거의 평행한 큰 벡터를 float로 빼면서 작은 단면 정보가 사라진다. plane normal뿐 아니라 plane 상수항도 크게 틀어질 수 있다.

수정 전 코드와 같은 DirectXMath 연산을 실행한 독립 console에서 optical axis 8m 앞의 반경 0.5m sphere를 773 pose로 검사했다. 모든 중심은 shader clip reference 안이지만 기존 계산은 margin 0.25m를 포함해도 267 pose에서 reject했다. 한 재현에서는 LEFT distance가 올바른 약 -5.730m 대신 +45.714m가 된다. 이것은 실제 Client 실행 관찰이 아니라 동일 수학식의 native 반례이며, G03에서 생산 함수 직접 호출로 회귀를 고정한다.

같은 입력 100회는 bitwise 동일했다. 하지만 고정 target에서 yaw를 0.00001도 바꾸는 작은 차이만으로 LEFT distance가 -77.449m와 +45.714m 사이로 바뀌는 반례가 있다. random race가 없어도 카메라 값에 따른 수치 오차가 '될 때와 안 될 때'를 만들 수 있다. 또 기존 state가 처음부터 rejected면 즉시 빠지고, 직전에 visible이었다면 3회 유예 후 네 번째 reject에서 빠진다. 실제 생산 predicate의 fresh/warm state test로 이 차이를 별도 검증한다.

따라서 이번 우선 수정은 먼 corner를 역투영해 삼각형으로 평면을 만드는 방식을 제거하고, **실제 float View/Projection의 clip half-space를 double 중간 연산으로 직접 추출**하는 것이다. margin, near/far, 최종 depth test와 shader의 카메라 입력은 유지한다.

### 현재 코드에서 이미 반영된 항목

| 확인 대상 | 현재 상태 | 이번 계획에서의 취급 |
|---|---|---|
| [LevelRegistry.cpp](C:/Users/user/Desktop/LostArk/Client/Private/LevelRegistry.cpp)의 `MakeBernMapScope` | 전체 맵 scope, `bypass=false`, `diagnostics=false` | 최종 제품값 유지 |
| 베른 margin | 기본 0.25m, landscape 또는 반경 4m 이상은 `max(2m, radius × 0.12)`, reject grace 3회 | 이미 적용됨. 같은 처방을 다시 추가하지 않음 |
| [MapAssetRenderUtils.cpp](C:/Users/user/Desktop/LostArk/Client/Private/MapAssetRenderUtils.cpp) | Render에서 View/Projection 복사 → 평면 생성 → 같은 복사본 shader 바인딩 | 실제 실행 수치와 소비자까지 검증 |
| [MapAssetObject.cpp](C:/Users/user/Desktop/LostArk/Client/Private/MapAssetObject.cpp) | fallback 객체 하나가 reject되면 그 객체의 모든 submesh draw 생략 | 객체 전체 소멸과 연결되는 분기 |
| [MapStaticBatchObject.cpp](C:/Users/user/Desktop/LostArk/Client/Private/MapStaticBatchObject.cpp) | 배치 내부 placement별로 검사해 visible instance buffer 생성 | 배치 전체를 단일 sphere로 판정하는 코드가 아님 |
| [Model.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/Model.cpp)의 static binary bounds | decoded vertex 전체와 pretransform으로 local AABB 재계산 | metadata만 믿는 옛 문제는 이미 보완됨 |
| [MapPlacementRuntime.cpp](C:/Users/user/Desktop/LostArk/Client/Private/MapPlacementRuntime.cpp) 및 fallback bounds | 실제 world center, nonuniform·음수 scale의 절댓값 최댓값으로 world sphere 확대 | signed scale과 bottom-center anchor 보존 |
| [Camera_Free.cpp](C:/Users/user/Desktop/LostArk/Client/Private/Camera_Free.cpp) | follow는 Late_Update에서 위치 갱신 후 camera state refresh | 단순히 카메라 갱신 순서를 다시 옮기지 않음 |
| [GameInstance.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/GameInstance.cpp) | Object Update → Physics → PostPhysics → Level Update → Object Late_Update | 현재 호출 순서를 기준으로 검증 |

위 margin 행의 grace는 현재 `Evaluate_FrustumVisibility` 호출 횟수로 소비된다. 카메라 revision은 행렬이 같으면 증가하지 않으므로 frame 번호로 취급하면 안 된다. 제품 main map pass에서 placement당 1회 호출인지 함께 확인한다.

### 데이터와 카메라 기준

- Area는 `LV_BER_BERNCASTLE`이고 [MapCatalog.json](C:/Users/user/Desktop/LostArk/Data/Maps/MapCatalog.json)은 **shard-set**으로 선언한다.
- catalog 진입점은 [Imported mapset](C:/Users/user/Desktop/LostArk/Data/Maps/Imported/LV_BER_BERNCASTLE/LV_BER_BERNCASTLE.mapset), 배치 정본은 [Authoring mapplacements](C:/Users/user/Desktop/LostArk/Data/Maps/Authoring/LV_BER_BERNCASTLE/LV_BER_BERNCASTLE.mapplacements)다. 존재하지 않는 Bern 단일 root mapassets 파일을 전제로 도구를 만들지 않는다.
- 현재 50,017 placement, catalog row 3,021개, unique asset 1,003개다. 베른 full scope에는 landscape component 42개도 포함된다.
- [Level_Bern.cpp](C:/Users/user/Desktop/LostArk/Client/Private/Level_Bern.cpp)의 near는 0.1m, far는 `max(2000m, mapSpan × 8)`이다. mapSpan의 근거인 placement 위치 범위는 실제 모든 mesh vertex의 범위와 같지 않으므로 이것만으로 far를 줄이지 않는다. 이번 고정 fixture는 런타임의 visible/non-background 배치 범위를 따라 far=13,084.808594m를 사용한다.
- 현재 카메라/맵 입력을 바꿔서 재현을 없애는 대신 동일 입력에서 잘못 생략되는 첫 placement를 찾는다.

### 확인된 결함과 아직 확정되지 않은 원인

| 항목 | 확인 수준 | 조치 |
|---|---|---|
| 유한한 singular View/Projection이 수정 전 최초 검사 통과 가능 | 수정 전 코드 확인. inverse/corner/plane 검증이 없고 revision/output을 먼저 갱신함 | G02에서 stage 후 commit 및 실패 이유 추가 |
| 평면 거리 `>= effectiveRadius`면 reject | 코드 확인. 정확히 접하는 sphere도 outside 처리 | G02에서 완전히 분리된 경우만 reject하도록 경계 계약 수정 |
| 현재 invalid snapshot/bounds 때 소비자가 draw를 유지함 | 두 실제 소비자 확인 | invalid를 소멸의 직접 원인으로 단정하지 않음. 이번 수학 수정에서는 기존 소비자 유지. 재발하면 G01 추가 진단으로 분리 |
| NONBLEND가 객체 Render의 HRESULT를 버림 | [Renderer.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/Renderer.cpp)의 `Render_NonBlend` 확인 | 객체에서 E_FAIL만 반환해도 상위에 이유가 전달되지 않음. 추가 진단 시 map 소비자에서 직접 실패 기록 |
| 08-25 RESULT의 bypass 사용 시 깜빡임이 사라졌다는 사용자 기록 | 과거 관찰 | 컬링을 우선 조사할 근거. 이번 정상 컬링 재발의 확정 증거는 아님 |
| reject transition 개수가 많음 | 정상 화면 밖 배치도 포함 | 개수만으로 false reject 판정 금지 |
| 현재 사용자 발생 지점의 행렬·placement·draw 기록 | 아직 없음. 이번 세션에서 Client 미실행 | core 수정 후 사용자 확인, 재발 시 G01 추가 진단 |

접선 경계의 작은 결함이나 singular 입력 방어 부족만으로 화면 전체 소멸 원인이 설명됐다고 쓰지 않는다.

G00 종료 증거는 현재 호출 경로·policy·데이터 수치의 기록이다. 버그 수정 완료 증거가 아니다.

## G01. 반례를 고정하고 수정 범위를 줄인다

### 이번에 사용하는 실행 증거

[MapFrustumContractHarness](C:/Users/user/Desktop/LostArk/Tools/MapFrustumContractHarness/Private/MapFrustumContractHarness.cpp)는 다음 두 계산을 같은 입력에서 비교한다.

- 제거할 inverse-corner 외적은 **test 전용 negative control**로만 보존한다.
- 제품의 `Build_CameraCullSnapshot`과 `Evaluate_FrustumVisibility`를 실제 CPP에서 컴파일해 호출한다.
- 독립 oracle은 world point를 View와 Projection에 순서대로 double로 곱한 뒤 `-w<x,y<w`, `0<z<w`를 검사한다. 생산 평면 값을 oracle에 재사용하지 않는다.
- 동일한 카메라 값 반복, 미세 이동, 실제 float `at-eye` 양자화를 포함한 follow 입력, fresh/warm grace를 따로 검사한다.
- stdout에는 입력 집합별 기존/new 오판 수와 실패 assertion을 남긴다. Client 화면·GPU 픽셀·실제 map instance buffer 검증으로 확대 해석하지 않는다.

공유 수학 함수에서 반례가 확정됐으므로 이번 단위에는 두 생산 파일과 하네스·회귀 등록만 반영한다. `MainApp`, 두 map render 소비자, Engine renderer, shader, depth/near/far, 배치 데이터는 변경하지 않는다. 기본값 `bypass=false`를 유지한다.

### 재발 시 추가할 진단의 경계

수정된 빌드에서도 사용자가 소멸을 확인하면 그때 실제 asset/placement·카메라 행렬을 수집하고 다음 단계를 연결한다. **아래 ImGui/GPU 진단은 현재 제품에 반영되지 않았으며 새 F1 기능이 있다고 안내하지 않는다.**

1. queue → Render 진입 → raw cull/grace → 선택 instance → 실제 업로드 → material/pass → draw 제출을 구분한다. `MapVisibleInstances`는 업로드 전에 증가하고 NONBLEND는 객체 HRESULT를 버리므로, 기존 profiler 숫자만으로 정상 draw를 확정하지 않는다.
2. 기존 F1 Developer Tools 안에서 opt-in하는 진단과 Area별 JSON 기록을 연결한다. frame ID와 camera matrix revision을 구분하고 상세 수·파일 크기 상한, 누락 수, 최초 실패와 원 HRESULT를 보존한다.
3. 선택한 batch/submesh에만 비동기 pipeline/occlusion query를 붙인다. pending/error/unsupported는 0이 아니며 GPU 대기·Flush를 넣지 않는다. batch 결과를 단일 placement의 증거로 쓰지 않는다.
4. RS CullNone 또는 depth test/write 비활성화는 선택 대상 A/B 진단에만 허용하고 실제 draw 직후/실패/Area 종료에서 원 상태를 복원한다. 이 상태 변경의 headless WARP 테스트를 통과시키기 전에는 제품에 등록하지 않는다.
5. Character Select 임시 hide와 camera JSON은 겹친 표면을 식별할 때만 추가한다. Authoring/runtime 데이터를 즉석에서 바꾸지 않는다.

공통 카메라 JSON을 도입할 때는 `formatVersion=1`, `kind=LOSTARK_MAP_RENDER_CAPTURE`, `matrixConvention=row-vector-row-major-d3d-lh-z01`, Area와 frames를 사용한다. frame의 ID와 camera revision은 decimal uint64 문자열, View/Projection은 유한한 row-major 16개 값이다. 같은 행렬의 revision 반복은 정상이며 잘못된 version/Area/ID/행렬은 거부한다. Character Select 검사에는 이 입력을 검증하는 소비자가 있지만, 이번 수정에 자동 capture writer는 없다.

## G02. 잘못된 reject를 생산하는 계약을 수정한다

### snapshot 생성과 실패 처리

[MapAssetRenderUtils.h/CPP](C:/Users/user/Desktop/LostArk/Client/Private/MapAssetRenderUtils.cpp)에 명시적 View/Projection을 받는 `Build_CameraCullSnapshot`을 추가하고, 기존 `Capture_CameraCullSnapshot`과 G03 harness가 함께 호출한다. builder는 수학 계산과 candidate 검증만, Capture는 성공 뒤 revision/cache commit을 소유한다. 제품 카메라를 별도로 소유하지 않는다.

1. 입력 행렬의 finite 여부와 가역성을 검사한다.
2. View×Projection의 column으로 Direct3D clip half-space를 직접 추출하고 double로 정규화한다. 6개 plane의 finite·normal 길이를 검사한다. 먼 corner 외적은 사용하지 않고, 임의의 큰 determinant threshold로 정상 far/scale을 거부하지 않는다.
3. local candidate에 결과를 만들고 모든 검사 성공 뒤 output과 camera revision/cache를 commit한다.
4. 실패하면 이전 snapshot 출력·revision/cache·placement grace를 바꾸지 않고 원인을 반환한다.
5. builder/predicate 실패는 false와 선택적 failure reason을 반환하며 output/state를 부분 변경하지 않는다. 이번 수정은 기존 소비자의 보수적 draw fallback을 바꾸지 않는다. 잘못된 render 행렬의 draw 차단·원인 기록은 G01 추가 진단과 함께 별도 검증할 범위다. 캐시에 실패한 새 행렬을 commit하거나 이전 snapshot을 성공한 새 카메라처럼 반환하지 않는다.

### sphere 경계와 수치 오차

- 현재 outward plane convention을 유지한다. 정규화된 평면의 `distance > effectiveRadius + tolerance`일 때만 완전히 outside로 판정한다.
- 실제 near/far를 포함한 프러스텀에 접촉·교차하는 sphere는 그리기 대상으로 남긴다. 카메라 중심을 포함해도 near보다 작은 sphere는 프러스텀 밖일 수 있다. tolerance는 float 연산 오차 규모와 G03 fixture로 정하고 별도 거대한 월드 margin으로 사용하지 않는다.
- near/far, 큰 좌표, 작은 움직임의 반례를 fixture로 고정한다. 새 평면은 실제 shader의 row-vector View/Projection과 동일한 clip 영역을 나타내야 한다.
- 기존 margin과 3회 grace를 유지한 상태에서도 **raw predicate 자체의 false reject가 0**이어야 한다. grace로 불일치를 가려 통과시키지 않는다.

### 실제 bounds와 소비자

현재 bounds 공식을 다시 추가하지 않는다. G01에서 잘못된 bounds가 확인된 asset에 대해 decoded 모든 submesh vertex, pretransform, S×R×T, bottom-center anchor를 동일하게 적용해 원인을 고정한다. mirror scale을 양수 transform으로 바꾸거나 물리 asset을 삭제하지 않는다.

CModel의 bounds 생성이 원인이면 [Model.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/Model.cpp), world 변환이 원인이면 [MapPlacementRuntime.cpp](C:/Users/user/Desktop/LostArk/Client/Private/MapPlacementRuntime.cpp)와 fallback `Update_WorldCullBounds`의 해당 계산만 수정한다. Shader material/pass 실패면 G01에서 확인한 실제 instanced·fallback 소비자를 함께 고친다.

G02 종료 조건은 수정 전 실패한 fixture가 수정 후 통과하고, 제품 default `bypass=false`에서 false reject를 만들던 분기가 사라지는 것이다.

## G03. 실제 C++ 판정을 실행하는 하네스

### 새 파일과 프로젝트 등록

현재 [Test-BernFrustumCullingContract.ps1](C:/Users/user/Desktop/LostArk/Tools/ProjectAudit/Test-BernFrustumCullingContract.ps1)은 소스 문자열 검사다. [WModelGeometryContractHarness](C:/Users/user/Desktop/LostArk/Tools/WModelGeometryContractHarness/Private/WModelGeometryContractHarness.cpp)는 모델 binary 계약을 검사하지만 map snapshot/predicate를 호출하지 않는다. 기존 Effect harness는 다른 작업의 미커밋 변경도 있으므로 억지로 결합하지 않는다.

이 공백에만 다음 신규 파일을 추가한다. 실제 반영·검증 상태는 RESULT를 따른다.

| 신규 경로 | 책임 |
|---|---|
| `C:/Users/user/Desktop/LostArk/Tools/MapFrustumContractHarness/Private/MapFrustumContractHarness.cpp` | 생산 코드의 snapshot builder와 `Evaluate_FrustumVisibility` 호출, 수치 fixture 검증 |
| `C:/Users/user/Desktop/LostArk/Tools/MapFrustumContractHarness/Default/MapFrustumContractHarness.vcxproj` | x64 Debug/Release console, Engine 참조와 실제 Client utility CPP 컴파일 |
| `C:/Users/user/Desktop/LostArk/Tools/MapFrustumContractHarness/Default/MapFrustumContractHarness.vcxproj.filters` | 새 test CPP와 필요한 기존 utility CPP의 물리 경로만 등록 |
| `C:/Users/user/Desktop/LostArk/Tools/MapFrustumContractHarness/Run-MapFrustumContractHarness.ps1` | Engine DLL 경로, 구성, timeout, native 실패 exit code 보존 |

Client의 기존 H/CPP는 이미 등록돼 있으므로 중복 등록하지 않는다. 새 console 프로젝트는 [Framework.sln](C:/Users/user/Desktop/LostArk/Framework.sln)의 x64 구성과 [Invoke-BuildAndRegression.ps1](C:/Users/user/Desktop/LostArk/Tools/Build/Invoke-BuildAndRegression.ps1)에 build/실행/SkipBuild 존재 검사를 함께 등록한다. [빌드 사용서](C:/Users/user/Desktop/LostArk/Tools/Build/README.md)에 새 harness 명령만 추가한다. Engine public header를 바꾸게 되면 UpdateLib와 Client까지 검증한다.

### 반드시 실행할 사례

- 6개 plane 각각의 완전 내부, 정확한 접촉, 교차, 완전 외부.
- 카메라를 포함하며 near를 넘어 프러스텀과 교차하는 큰 sphere, 카메라를 포함하지만 near 안쪽에만 있는 작은 sphere, far/near 경계, 큰 음수 월드 좌표, 아주 작은 camera 이동.
- 회전한 카메라와 큰 음수 월드 좌표의 sphere 입력. nonuniform·mirror scale과 bottom-center anchor의 실제 CModel/placement bounds 함수는 이번 하네스 실행 범위가 아니며 해당 소비자를 바꾸지도 않는다.
- 고정한 동일 View/Projection·sphere 재생과 independent double precision clip-space reference 비교. sphere가 near와 교차하는 경우를 검사한다. 실제 mesh 삼각형 clipping과 map placement 생성은 이 하네스 범위가 아니다.
- NaN/Inf, finite singular matrix, 0 길이 plane, 잘못된 radius/revision 입력. builder output과 predicate의 placement grace가 부분 변경되지 않아야 한다.
- visible→3회 reject grace→실제 reject→재진입, 초기화한 state, bypass on/off. raw 판정과 최종 판정을 따로 검증한다.
- 동일한 sphere/snapshot을 독립 placement state에 입력한 경우의 결정 일치와 최초 rejected 판정.
- 동일 행렬 100회 반복의 결정 일치, 미세 camera perturbation에서 정상 내부 sphere 유지, fresh/warm state와 이동→정지→재진입에 해당하는 grace 순서 비교. 수치적 간헐성과 state 이력의 영향을 분리한다.

### 실제 소비자 검증의 경계

위 console이 직접 실행하는 것은 builder/predicate다. `Build_StaticInstance`, fallback `Update_WorldCullBounds`, transform 변경 reset, visible buffer 업로드, all-rejected 처리나 GPU draw는 이 harness만으로 PASS 처리하지 않는다.

현재 소비자 연결은 기존 정적 계약 검사와 Client 컴파일로 확인한다. 재발 시에는 G01의 추가 로그로 동일 placement의 입력 bounds·선택 수·buffer/draw 결과를 대조한다. 그 소비자의 계산을 수정하게 되면 기존 함수가 호출하는 범용 수학 부분을 같은 utility로 모아 harness가 **그 생산 함수**를 실행하도록 확장하거나, 해당 실제 객체를 호출하는 headless fixture를 연결한다. 수정한 분기의 실행 증거 없이 합성 sphere test만으로 완료하지 않는다. Capture의 revision/cache commit 순서는 이번에는 코드 검토 범위다. 실제 GameInstance/cache 실행과 진단 기록은 별도 검증이 필요하다.

독립 reference는 oracle로만 사용한다. Python에 같은 predicate를 복사해 통과시킨 결과를 실제 C++ 검증으로 대체하지 않는다. CPU test가 관찰하지 못하는 GPU 제출과 최종 시각 결과는 G01 로그·G04 사용자 확인으로 분리한다.

G03 종료 증거는 x64 Debug/Release 실행 실패 0, 정상 outside는 계속 reject, geometry 교차 false reject 0, 오류 입력의 상태 보존이다.

## G04. 회귀 검증과 사용자 확인

### 구현 후 자동 검증 순서

아래는 이번 수정의 자동 검증 명령이다. 성공/실패와 미실행은 RESULT에 실제 상태로 기록한다.

```powershell
Set-Location 'C:/Users/user/Desktop/LostArk'
powershell -ExecutionPolicy Bypass -File 'C:/Users/user/Desktop/LostArk/Tools/ProjectAudit/Test-BernFrustumCullingContract.ps1'
powershell -ExecutionPolicy Bypass -File 'C:/Users/user/Desktop/LostArk/Tools/Build/Invoke-BuildAndRegression.ps1' -Configuration Debug
powershell -ExecutionPolicy Bypass -File 'C:/Users/user/Desktop/LostArk/Tools/Build/Invoke-BuildAndRegression.ps1' -Configuration Release
git diff --check
```

정본 build script 순서는 Engine → UpdateLib → Shared·Server·Client·관련 harness 빌드 → contract/regression 실행이다. G03에서 새 harness를 이 경로에 등록하므로 build 후 실행되며, 없는 EXE를 먼저 호출하지 않는다. `Run-MapFrustumContractHarness.ps1 -Configuration <Debug|Release>` 단독 재실행은 해당 구성 빌드가 끝난 뒤에만 사용한다. 현재 스크립트는 Client.exe를 자동 실행하지 않는다. 위 두 configuration은 순차 실행하고 Debug/Release 성공을 각각 기록한다.

- 추가한 vcxproj/filters 및 변경한 JSON fixture를 parse한다. compiled shader closure는 정본 regression의 기존 실행 항목을 그대로 통과해야 한다.
- 베른 맵 데이터는 이번 기본 수정 범위에서 변경하지 않는다. 필요성이 입증되어 변경했다면 Character Select 계획 G01의 publisher Validate/Check 지원과 함께 domain 검증 후 publish한다.
- 옛 `Test-CameraFrustumFrameOrder.ps1`, `Invoke-ProjectAudit.ps1`은 현재 존재하지 않으므로 실행 목록에 넣지 않는다.
- normal culling 복귀값, 동일한 render/cull 행렬, margin/grace 유지의 정적 검사를 현재 소스와 맞게 갱신한다. 문자열 PASS만으로 수치·화면 회귀를 대체하지 않는다.

### 사용자가 직접 실행할 경로

1. 현재 PC LAN 역할은 `server-host`, 방화벽 준비 완료다. 조사 시 Server/Client는 실행 중이 아니고 endpoint는 not-listening이었다. Visual Studio에서 **Server + Client** profile을 선택하고 **Ctrl+F5**로 사용자가 시작한다. Client 작업 디렉터리는 `Client/Default`다.
2. 시작 Lobby → Character Select → class와 nickname 선택 → Create Character → Server 승인 후 Bern에 진입한다. 이미 created identity가 있는 동일 process에서는 Lobby → Bern을 사용한다.
3. 원래 소멸 지점에서 follow 이동/정지, F6 free 전환, Tab mouse-look 전환 후 마우스 회전과 WASD 이동, 다시 F6 follow 복귀를 확인한다.
4. 같은 건물·바닥·landscape를 화면 네 모서리에 걸치고 근접/원거리로 이동한다. 첫 진입과 재진입도 확인한다. free camera에서는 gameplay command를 보내지 않는다.
5. 사용자는 발생 위치·조작·사라진 대상과 재발 여부를 글로 남긴다. 이번 기본 빌드는 추가 ImGui/JSON capture가 없고 기존 전환 로그도 `diagnostics=false`여서 자동 생성되지 않는다. 재발 시에만 G01 추가 진단을 적용한다.
6. 반드시 `bypass=false`, `diagnostics=false`인 제품 설정에서 사용자 확인을 다시 받는다. Character Select/Valtan과 사용 가능한 Development 시나리오는 별도 수동 회귀로 기록한다. Release에서 제외된 도구를 PASS로 적지 않는다.

에이전트는 Client/UI를 실행·조작·촬영하지 않는다. 사용자가 분석을 요청한 첨부 이미지는 진단 입력으로 읽을 수 있으나 사용자 최종 육안 승인 대신 사용하지 않는다.

### 완료와 인계

| 구분 | 상태 기록 위치 | 완료 기준 |
|---|---|---|
| 수학 수정 | RESULT 실제 diff | G02 반영, 기존 margin/grace와 shader 입력 유지 |
| 자동 검증 | RESULT 명령·exit·로그 | Debug/Release native harness, Client build, 정본 회귀의 실패를 숨기지 않음 |
| 수동 화면 검증 | 사용자 서면 관찰 | 정상 컬링 상태에서 소멸 재발 없음 |
| Character Select | 별도 PLAN/RESULT | 겹침 후보와 camera depth·시각 증상을 분리해 검증 |
| 추가 ImGui/GPU 진단 | G01의 조건부 후속 범위 | 필요한 경우에만 실제 소비자·상태 복구 테스트와 함께 반영 |

작업 폴더는 정본 `C:/Users/user/Desktop/LostArk`다. 시작 branch는 `codex/valtan-flow-reload-next-fix`, HEAD는 `1a9bce42ac00f75b2e1b6fa24ed0de1071d4c050`였다. Valtan·Balance·Effect 등의 기존 dirty 변경은 보존하며, 다른 작업의 index/HEAD checkpoint 중에는 stage/commit/checkout하지 않는다. 이 수정은 기존 두 map utility 파일과 새 하네스/회귀/문서로 범위를 제한한다.

베른 resource ID root는 `Map/LV_BER_BERNCASTLE`, 물리 폴더는 [Bern Resources](C:/Users/user/Desktop/LostArk/Client/Bin/Resources/Map/LV_BER_BERNCASTLE)다. 기본안은 신규 runtime asset을 만들지 않아 Git dependency closure 추가도 없다. 로그·빌드 산출물·EngineSDK·개인 규칙 파일은 커밋하지 않는다.
