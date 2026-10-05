# Bern 공간 청크와 HLOD 결과

## G00. 변경 범위

Bern의 원본 asset/mirror 단위 instancing을 32m XZ 구역으로 나누고, 호환되는 정적 불투명 표면의 서로 다른 mesh들을 기존 CModel/CMaterial 경로 안에서 병합한다. 가까이서는 병합된 원본 index를 한 번의 DrawIndexed로 제출하고, 멀리서는 같은 청크의 축약 index를 사용한다. 한 공간 구역에 여러 재질 그룹이 있으면 여러 draw다.

원본 placement ID, picking geometry, MapTool 저장본, 그림자와 Server actor는 유지한다. 물·투명·masked·wind 표면은 병합 HLOD 대상에서 제외하며 반복 식생은 기존 instancing과 공간 후보 탐색을 사용한다. NPC 애니메이션은 정적 geometry로 합치지 않는다. 청크 구현은 rendering option과 맵 authoring JSON을 변경하지 않았다. 사용자 후속 요청의 클래스 무비 제외 데이터 변경은 G09에 별도로 기록한다.

## G01. 구현

- Layer는 선택적으로 제공한 bounds의 BVH를 소유하고 dirty 알림만 refit한다. 카메라 통과 후보를 원래 membership 순서로 제출하며 invalid camera/bounds는 fail-open한다. 활성 shadow light의 caster는 camera rejection으로 버리지 않는다.
- VTXMESH 76byte와 source index 4byte를 정점으로 사용하고, source별 VTXMESHINSTANCE 224byte를 structured buffer에 보존한다. 원본 VS_MAIN과 source BG bank pixel shader를 재사용한다. RNM·static shadow는 최대 8개 원본 SRV 조합으로 유지한다.
- CModel 병합 생성은 typed surface 입력이 동일한지 검사한다. 기존 adjacent instancing의 엄격한 legacy texture 비교는 유지하고 신규 chunk 경로만 사용하지 않는 legacy texture identity를 비교에서 제외한다.
- 병합 전체의 world-space geometry에 meshoptimizer를 적용한다. source/UV/조명 경계를 유지하고 원본 정점을 이동하지 않는 축약 index를 만든다. 목표 비율은 0.35, 허용 world error는 0.05m이며 이득이 없으면 원본을 사용한다. 화면상 오차 계산에는 깊이와 화면 측면 위치를 함께 반영하며 1pixel 이내일 때만 원거리 index를 선택한다.
- 파생 GPU geometry는 총 384MiB 상한을 둔다. draw 절감 후보 대비 예상 메모리 비율 순서로 준비한다. 파생 mesh는 원본 picking을 복제하지 않는다. CPU decode cache는 준비 종료 시 해제한다.
- 위치·가시성·suppression 변경은 관련 mesh claim을 무효화한다. 공유 claim의 모든 소속 batch가 원본 제출로 복귀한다. 매 프레임 Late_Update에서 활동 상태를 고정해 diagnostics lease나 UI 변경 때문에 source와 chunk가 서로 다른 제출 결정을 하지 않게 한다. 무효화된 청크는 맵을 다시 로드할 때 재생성한다.
- 시간에 따라 변하는 native 재질은 대표 원본 batch의 경과 시간을 사용한다. 파생 geometry 생성 시 원본 재질은 불변이라는 계약이며 향후 live material 편집을 추가하면 같은 invalidation을 연결해야 한다.

## G02. World Level Tool과 계측

Debug의 World Level Tool에서 `Runtime chunks / HLOD`를 열면 생성된 청크, 대표 material 이름, 동일한 활성 shader 입력으로 묶인 원본 placement/asset, 근거리·원거리 index 수와 활성 상태를 확인한다. 청크 경계 overlay, 선택 청크 Focus와 원본 placement Inspect를 제공한다. 숫자 chunk ID는 현재 로드에 한정된 진단 ID이며 저장 identity가 아니다. 원본 inspect 요청은 stable placement ID를 사용한다.

`Merged chunk draws`와 `Distant HLOD`는 실행 중 비교용이며 데이터 파일을 저장하지 않는다. 같은 controls를 World Scene Tool에서도 제공한다. 공간 분할과 Layer BVH는 유지된 상태에서 병합 draw와 HLOD를 각각 분리해서 비교한다.

Profiler v3는 chunk 수, 활동 상태, near/far draw, 병합된 원본 draw 후보 수, 실제 제출 index, 원본 index, 파생 GPU bytes와 invalidation을 기록한다. `Map.Chunk.Render`, `Map.Chunk.Prepare` scope를 제공한다. `mapChunkSourceDraws`는 그 청크에 들어 있는 원본 batch/mesh 수이며 카메라별 실제 절감 draw 수와 같다고 가정하지 않는다. 전체 drawCalls의 A/B 차이가 실제 절감의 근거다.

## G03. 완료한 자동 검증

| 검증 | 실제 결과 | 증거 |
|---|---|---|
| Layer 실제 코드 native fixture | Debug/Release 각각 45,769 assertions; 경계·random camera·grace·dirty·shadow·제거·clone 통과 | `out/SpatialLayer20261004` |
| 공간 후보 synthetic 장면 | 15,794 objects 중 16 callback; actual Layer traversal | 같은 fixture; 실제 Bern FPS가 아님 |
| Batch ownership/mutation fixture | 99 checks; 부분 mesh claim·fallback·그림자·bounds | `out/SpatialLayer20261004` |
| Profiler JSON roundtrip | 120 checks; legacy missing 값·future field·실패 시 기존 capture 보존 | `out/SpatialLayer20261004/counters/verification10.json` |
| World Level bounds clip | 실제 homogeneous clipping 함수 157,584 checks; near/side/far/w·behind-camera·NaN 검사 통과 | `out/SpatialLayer20261004/overlay-result.log` |
| Chunk FXC | fx_5_0 /O1 컴파일 성공 | `out/BernChunkHlod20261004/shader/validation-summary.json` |
| GPU shader parity | WARP/하드웨어 각각 960 MRT 비교; WARP exact, 하드웨어 최대 절대차 1.49e-7; D3D 오류·경고 0 | `out/BernChunkHlod20261004/shader/{warp,hardware}/gpu-parity.json` |
| CModel/CMesh 실제 병합 API | WARP synthetic 단계 485 checks; 2개 다른 mesh, 730 vertices, 3936→1374 indices; 오차 0.0231062m; 파생 picking BVH 없음 | `out/BernChunkHlod20261004/engine/api-probe.log` |
| 원본 geometry 보존 | 원래 CMesh GPU vertex buffer와 병합 buffer의 76byte 전 채널 비교; source ID·triangle 소속·실패 transaction 확인 | 같은 API fixture |
| 실제 Bern CModel 경로 | 88 WModel, 200 instances, 서로 다른 geometry/RNM 12그룹 모두 생성; 651,852→552,126 indices(15.30% 감소), 최대 오차 0.0499882m | `out/BernChunkHlod20261004/engine/bern-native-results.json` |
| 실제 Bern 파생 비용 | 준비 전용 실행에서 12그룹 생성 340.207ms, source model load 697.747ms, 파생 GPU buffer 22,949,112byte(21.89MiB) | `out/BernChunkHlod20261004/engine/api-probe.log`; GPU readback·source load는 build 시간에서 제외 |
| 실제 shader/material/draw 연결 | 실제 CShader와 Bind_Material → Bind_StaticClusterSources → Begin(0~2) → Render_StaticCluster(near/far); Bern 12그룹에서 DrawIndexed 72회, 3,611,934 indices 제출; D3D11 debug ERROR/CORRUPTION 없음 | `out/BernChunkHlod20261004/engine/binding-integration.log` |
| 수정 CPP | Engine Model/Mesh/Material/GameObject/Layer 및 Client 연결 파일 격리 컴파일 통과 | 각 out 하위 compile log |
| project/manifest | Client/Engine vcxproj+filters XML 및 BuildDomains JSON parse 통과 | 2026-10-04 실행 |
| Debug x64 Product | Engine/Shared/Server/Client 컴파일·링크·배포 PASS, 307.707초 | `out/BuildPipeline/runs/20261004T100751141Z-debug-product.json` |
| Release x64 Product | Engine/Shared/Server/Client 컴파일·링크·배포 PASS, 202.390초 | `out/BuildPipeline/runs/20261004T101139101Z-release-product.json` |
| Debug/Release shader closure | 두 구성 모두 PASS; active FxCompile 256, Client consumers 171, Product Effect WARP V1/V2 각각 1,352 pixels | `Test-CompiledShaderClosure.ps1 -Configuration Debug/Release -Modules Product` 각각 실행 |

GPU fixture는 창·Client·Present 없이 실행했다. 실제 게임 화면 검증을 대신하지 않는다. Shader parity와 triangle 감소는 서로 다른 검증이며 triangle 65.1% 감소를 FPS 65.1% 상승으로 표현하지 않는다.

최종 draw 연결 fixture는 D3D11 debug device를 사용하며 scene getter 세 개와 사용하지 않는 character resolver만 대체했다. 별도로 복사한 DLL/CSO로 실제 제품 shader/material/model 코드를 실행했다. 이 실행의 12그룹 생성은 429.419ms, source load는 1,204.62ms이며 최신 `bern-native-results.json`에 기록된다. 준비 전용 실행 시간과 debug-device 실행 시간을 같은 표본으로 섞지 않는다.

실제 Bern fixture의 89개 source batch/mesh를 12개 청크로 묶은 것은 전부 표시했을 때의 구조적 제출 수다. 기존 adjacent bank 또는 카메라 culling을 반영한 게임의 순 draw 절감 수는 아니다. WARP process private bytes 변화에는 소프트웨어 GPU와 allocator가 포함되므로 이를 실제 그래픽카드 VRAM 또는 지속 CPU RAM 증가량으로 적지 않는다.

## G04. 설치 데이터 실측과 비교 절차

설치 Bern은 50,021 placements, 1,291개 고유 WModel이다. 실제 생성 조건에 가까운 오프라인 inventory 계산에서는 1,970개 병합 후보가 6,959개 원본 batch/mesh를 포함했다. 서로 다른 geometry를 합치는 후보는 1,114개다. 이는 전체 맵·typed JSON 기반 시뮬레이션이며 실제 CModel 생성 성공 수, 현재 카메라 draw 또는 FPS 측정이 아니다. 근거는 `out/BernChunkHlod20261004/inventory/enabled-coverage-summary.json`이다. 생성 성공·실패 수와 준비 시간은 런타임 Stage 결과와 World Level Tool로 확인한다.

이 후보의 실제 WModel 정점으로 오프라인 meshoptimizer 계산을 실행했다. 3,972,521 vertices, 10,163,139 원본 indices에서 1,066개 그룹만 far 후보가 되었고 904개는 원본을 유지했다. far/near 혼합 index는 8,821,161개로 후보 전체에서 13.2% 줄었다. 따라서 작은 synthetic mesh의 65.1% 축약률을 베른 전체에 적용하지 않는다. 이 계산의 NumPy float32 transform과 생산 코드의 DirectX 연산은 같다고 보장하지 않으므로 런타임 최종 축약 수치와 구분한다.

오프라인 /O2 CPU probe에서 seam lock 0.265초, 축약 3.871초, binary 입력/할당 포함 총 5.383초였다. 별도 warm-cache 준비는 WModel 404개 읽기 0.224초, channel decode 0.423초, 배열 assembly 0.714초였다. 생산 Stage의 재질 검사·metadata 검증·GPU 생성 시간은 포함하지 않는다. 현재 준비는 Bern 활성화의 동기 작업이므로 로딩 시간 증가를 함께 평가해야 한다. `cpu-input-preparation.json`, `cpu-probe-optimized.json`에 원본 실측을 보존했다.

기존 하 캡처의 308개 양수 frame interval 평균은 47.744522ms(20.9448FPS), 전체 draw 평균은 1001.566이다. 새 버전의 동일 조건 캡처가 없으므로 이 값과 아직 개선율을 계산하지 않았다.

1. Debug Bern에서 같은 카메라·해상도·렌더링 옵션을 유지한다. 로딩과 셰이더 warmup이 끝난 뒤 측정한다.
2. World Level Tool의 병합을 끄고 도구/overlay를 닫는다. F7에서 Reset 후 `Bern_original` 독립 캡처를 저장한다.
3. 병합 ON, HLOD OFF로 같은 위치/동선을 반복해 `Bern_chunk`를 저장한다.
4. 병합 ON, HLOD ON으로 `Bern_hlod`를 저장한다. 먼 구역이 보이는 동일 시네마틱 동선도 별도로 비교한다.
5. `Tools/Profiler/analyze_capture.py`로 평균·p95 frame interval, 전체 draw, map CPU, GPU timestamp, triangle·메모리와 모드 counter를 비교한다. GPU timestamp는 CPU 공급 대기를 포함할 수 있으며 GPU 이용률이라고 쓰지 않는다.

포트폴리오에는 현재 확인된 구조·동치 검증·geometry 감소를 적을 수 있다. 실제 프레임 개선율은 위 게임 캡처와 화면 확인 후 기록한다.

## G05. 사용자 확인이 남은 항목

사용자가 Debug Bern에 입장한 화면을 제공했고 World Level Tool의 1,987 prepared chunks 및 All chunks를 켠 경계 표시를 확인했다. 사용자는 컷신과 Debug 플레이에서 유의미한 프레임 상승이 없었다고 보고했다. 이어 저장한 병합 OFF/ON 게임 캡처는 G07에서 비교했다. 현재 성능 목표는 달성했다고 판정하지 않는다. HLOD 전환과 근거리 복귀의 최종 화면 품질 확인도 남아 있다.

빌드는 기존 작업 트리의 무관한 미커밋 rendering 변경을 보존한 상태에서 수행했다. 전체 빌드에는 기존 인코딩/변환 경고가 남아 있다. Debug 배포 도중 검증용 프로세스가 Engine.dll을 잠시 참조해 복사 재시도가 있었지만 fixture 종료 뒤 배포가 성공했다. Client나 사용자 프로세스를 실행·종료하지 않았다.

## G06. 사용자 실행 후 표시 교정

첫 화면의 Show chunk bounds ON / All chunks OFF / 선택 없음 조합은 아무 경계도 표시하지 않았다. 선택이 없으면 전체 경계를 표시하도록 고치고 렌더링 제어와 경계선 제어를 설명하는 문구를 추가했다. 경계 draw list는 게임 main viewport를 명시해 분리된 도구 창의 viewport와 투영 좌표가 섞이지 않게 했다.

기존 Merged near 상태는 claim 활성 여부만 확인해 카메라 밖의 청크에도 표시되었다. Debug 행에 실제 Render_StaticCluster 성공 여부를 추가해 Not submitted와 구분하고, 250ms마다 한 프레임의 성공 near/HLOD draw 및 제출 index를 표시한다. source group 수는 실제 절감된 draw 수가 아니라는 설명도 추가했다. 이 표시 교정은 FPS 최적화로 계산하지 않는다.

코드 조사에서 원본 batch의 Update/Late_Update와 shadow 제출이 계속 유지되며, 일부 mesh만 병합된 batch는 원본 instance 가시성/업로드와 render 준비도 수행하는 것을 확인했다. shadow가 켜지면 Layer의 caster는 camera rejection을 우회한다. 또 청크 후보는 예상 bytes당 제출 감소 순서이므로 목록 처음의 12/12 indices 그룹은 작은 geometry이고 HLOD 축약이 없다. 이 사실만으로 사용자 프레임 정체의 지배 원인을 확정하지 않는다.

표시 교정은 실행 중인 Client를 건드리지 않고 별도 out의 WorldLevelTool, MainApp_WorldLevel, MapStaticChunkObject translation unit 컴파일과 git diff --check를 통과했다. 당시 제품 링크는 미실행이었으며 후속 G11의 Debug/Release 제품에 포함했다. 새 표시의 사용자 화면 확인은 남아 있다. 교정 전 EXE의 Merged chunk draws OFF/ON 비교 캡처는 G07의 근거로 보존한다.

## G07. 실제 병합 OFF/ON 캡처 결과

`병합off_20261004_194143_994_frame417_63752_1.json` 238 frames와 `병합on_20261004_194106_805_frame179_63752_0.json` 179 frames를 `Tools/Profiler/analyze_capture.py`로 분석했다. 원본 파일은 변경하지 않았다. SHA256을 포함한 분석 결과는 `out/BernChunkHlod20261004/merge-on-off-analysis.json`에 있다.

| 프레임당 평균 | OFF | ON |
|---|---:|---:|
| 양수 frame interval | 64.129ms | 68.325ms |
| 평균 interval의 역수 FPS | 15.594 | 14.636 |
| 전체 draw | 1,151.571 | 1,105.497 |
| 제출 indices | 3,553,658 | 4,247,253 |
| Map.Batch.Render CPU | 17.040ms | 13.980ms |
| Map.Chunk.Render CPU | 0ms | 5.388ms |
| batch + chunk 렌더 CPU 합계 | 17.040ms | 19.367ms |
| 최종 카메라 제출 CPU | 2.682ms | 2.291ms |
| 원본 batch Update + LateUpdate | 3.472ms | 3.423ms |
| Ambient.Advance CPU | 8.915ms | 9.284ms |
| ImGui.BuildAndSubmit CPU | 5.757ms | 8.973ms |

두 캡처 모두 chunkCount=1,987이며 OFF에서는 enabledCount/nearDraw=0, ON에서는 enabledCount=1,987·nearDraw 평균151.291이다. invalidation은 두 캡처 모두0이므로 모드가 적용되지 않은 결과가 아니다. ON의 source group 평균631.894를 실제 기존 draw631회로 해석하면 안 된다. 기존 카메라 culling과 adjacent instancing을 포함한 전체 draw의 관측 감소는 약4.00%다. HLOD enabled/far draw는 두 파일 모두0으로, 요청한 병합 단독 비교이며 HLOD 효과는 이 두 파일로 판정하지 않는다.

현재 병합은 실제 데이터에서 draw가 조금 줄어도 추가 렌더 CPU 비용으로 그 이득이 상쇄되는 결과를 보였다. 전체 제출 index도 약19.52% 늘었다. 원본 단위 가시성 대신 합쳐진 청크 bounds로 포함된 전체 geometry를 제출하는 구조와 일치하는 위험이며, 카메라 차이가 있으므로 증가량 전부를 병합 원인으로 확정하지 않는다. batch Update/LateUpdate는 그대로 남았고 환경 이펙트 업데이트도9ms 안팎으로 유지됐다. 그림자는 두 파일 모두OFF이므로 이번 관측의 그림자 비용을 원인으로 들지 않는다.

두 파일은 같은 Debug/RTX4070/1920x1080/D3D debug layer 및 저장된 렌더링 옵션을 사용하고 모드 counter는 전 프레임 일관된다. 다만 export camera는 약1.734m 차이 나며 ImGui render window 수가 다르다. CPU scope 누락은0이고 detailed CPU scopes는OFF다. GPU pipeline 통계는 각각 유효 GPU 프레임234/179개에 기록됐으며 OFF의 마지막4개 pending 프레임의0을 계측OFF로 해석하지 않는다. 전체 FPS 차이를 엄밀한 병합 회귀율로 보고하지 않으며, 병합 성능 개선을 입증하지 못했다는 판정과 관측된 비용 교환을 남긴다.

다음 성능 변경은 생성 청크 수를 늘리는 기준으로 결정하지 않는다. 기존 instancing이 이미 줄인 실제 제출 수, 청크의 가시성 범위로 늘어난 geometry, batch와 chunk를 합친 CPU 비용을 함께 기준으로 삼아야 한다. 가까운 원본 instancing과 먼 청크 대체의 선택, 원본 정적 batch의 매 프레임 순회 축소, 별도 ambient update 병목은 각각 측정 가능한 변경 단위로 분리한다.

## G08. 실제 카메라 기준 병합 선택과 정적 batch 갱신 축소

CModel::Get_StaticMeshSelectedIndexCount는 실제 instanced draw와 같은 CMesh LOD 선택기를 소비한다. 청크는 최종 카메라로 원본 batch의 visible instance와 mesh LOD를 준비한 뒤, 원본 최소3draw를 대체하고 선택한 청크 index가 보이는 원본 index 이하일 때만 제출한다. 기존 lighting-bank로 합칠 수 있는 single-mesh 원본은 인접하지 않아도 한 draw로 계산해 절감을 보수적으로 판정한다. 정확한 전체 render queue 절감치 또는 FPS 예측값은 아니다.

source와 chunk 중 누가 먼저 선택을 요청하든 frame당 한 번 확정하고, 원본 visibility 준비는 frame generation과 camera revision으로 재사용한다. 같은 프레임에 grace를 두 번 진행하지 않으며 실패는 원본으로 복귀한다. Debug 목록 조회는 선택 함수를 호출하지 않는다. Map.Chunk.Select scope로 새 선택 비용도 계측한다.

Bern의 static batch는 MapPlacementRuntime 공통 clock을 받아 각 객체의 Update/LateUpdate 목록에서 제외한다. runtime이 시간과 placement/batch counter를 한 번 갱신하고 ordinary·adjacent bank·chunk·shadow가 같은 경과 시간을 소비한다. MapTool authoring Reload가 만든 기존 독립 batch는 자기 clock과 callback을 계속 사용하며 중복 계수하지 않는다.

| 검증 | 실행 결과 | 증거 |
|---|---|---|
| 실제 Model/Mesh/LOD index 선택 | 2,018 assertions PASS | out/BernChunkHlod20261004/adaptive-helper/result.log |
| 실제 resolver/claim/Late/visibility wrapper, 재질·가시 입력 fixture | 순서·frame cache·fallback 등26 assertions PASS | 같은 폴더 admission-result.log |
| 공통 clock·업데이트 phase·authoring reload·visibility grace | 3,600frame의 기존 시간과 exact parity 및 경계 검사 PASS | out/BernBatchClock20261004/probe.log |
| 수정 CPP 개별 컴파일 | Model, MapStaticChunkObject, batch/runtime/Bern Debug·Release 관련 컴파일 PASS | adaptive-helper/compile-model.log, diagnostics/compile.log, BernBatchClock20261004/compile.log |

원본 visibility/GPU instance 업로드는 선택에 여전히 필요하고32m 분할 자체의 비용도 남는다. 서로 다른 표면 재질을 atlas로 구운 거대한 구역 proxy, 다단계 HLOD, ambient update 최적화는 이번 수정에 구현되지 않았다. 이 변경은 관측된 geometry 증가를 제한하고 반복 batch update를 제거한 것이며 실제 FPS 상승은 새 캡처로 확인해야 한다.

## G09. 차원술사 클래스 무비의 머리 껍질

사용자 스크린샷의 회색/흰 저해상도 머리 외피와 기존 편집 내역을 대조했다. 기존 world.object.classselect.dimensionmaster.a12260.p0는 계속 제외되어 있었다. 별도 a753.p1과 a754.p7은 같은808vertex 전신 shell(머리 영역126vertices)을 사용하는 primary-palette 복제본이다. 실제 hair인 a753.p0의16,956vertices와 구분했다.

a753.p1은 native program702의 opacity가 intro에서0→0.94998, loop에서0.7이다. 0df4567904의 native primitive opacity 복원(2026-10-04 03:54:09 KST)으로 기존의 잘못된 zero 입력에 가려진 형상이 드러나는 코드 경로를 확인했다. a754.p7은 같은 형상의 dormant 복제본이다. 공통 shader를 되돌리지 않고 아래2개 stable ID만 DIMENSIONMASTER.excludedWorldObjectIds에 추가했다.

- world.object.classselect.dimensionmaster.a753.p1
- world.object.classselect.dimensionmaster.a754.p7

Data/Camera/ClassSelection.cinematics.json의 제외 목록은23→25다. 기존7,014,285bytes는 삽입부 외 exact 보존했다. 두 movie source writer lock·최신 hash 확인·ReplaceFileW·backup 검증을 거쳤으며 설치 SHA256은 a6c9e65f878ea108b52e84c5a4f95d6c382a8c58fb037a7140f47f52415c6514이다. Debug/Release 모두 CProjectDataRoot를 통해 같은 Data/Camera 파일을 직접 읽으므로 별도 publisher가 필요하지 않다. 원본 백업·geometry·timeline·consumer 증거는 out/DimensionMasterMovieShell20261004/install-receipt.json에 있다. 실행 중 무비 Reload와 사용자 최종 머리 화면 확인은 수행하지 않았다.

## G10. 다른 캐릭터 생성 후 첫 캐릭터의 베른 재입장

사용자 재현은 첫 캐릭터 생성 → 다른 캐릭터 생성 → 첫 캐릭터 재선택 → Bern 입장 시 Lobby 복귀다. Release client-session-66008.jsonl의 generation5/6은 Server entry.accepted 뒤 각각7.828/7.843초 main-pump 정체가 있었고, 그 정체가 끝난4ms 뒤 character.restore-timeout / CLIENT_IDENTITY_COMMIT_FAILED를 기록했다. Server 수신은 계속되고 저장 슬롯도 유지되어 있었다.

Level_Bern::Update는 Try_Send_CharacterRestore 직후 이전 frame의 fTimeDelta를5초 제한에 누적했다. 로딩/activation이 길면 방금 보낸 요청이 즉시 만료되는 구조였다. optional steady_clock 시작값으로 실제 복원 대기를 측정하도록 수정하고, 복귀 전환이 이미 pending이면 같은 recovery를 반복 처리하지 않게 했다. 실제5초 제한·응답 identity 검증·대기 중 입력 차단·실패 시 슬롯 보존 계약은 유지했다.

기존/수정 block을 실제 CPP에서 추출한 native fixture15개가 통과했다. 기존7.843초 delta의 즉시 timeout을 재현하고, 수정 후4.999초 대기 유지·실제5초 timeout·정상 응답·송신 실패·중복 recovery 방지를 확인했다. Level_Bern.cpp의 Debug/Release 개별 컴파일과 scoped diff-check도 통과했다. 증거는 out/BernRestoreTimeout20261004/validation-receipt.json이다. 실제 사용자의 캐릭터 생성/재입장 화면 검증은 남아 있다.

Bern 입장 컷신은 같은 Level_Bern의 공통 cue clock에 BERN_ENTRANCE_PLAYBACK_RATE=0.5를 적용했다. 첫 입장과 Debug replay 모두 명목16초 cue가32초에 재생되며 기존 frame delta 상한0.1초 때문에 매우 느린 프레임에서는 실제 시간이 더 길 수 있다. 카메라 key·FOV·보간·ESC 종료·저장 cue JSON은 변경하지 않았다.

## G11. 후속 수정의 Debug·Release 제품 배포

G06 표시 교정과 G08~G10의 소스를 동결한 뒤 공식 Invoke-BuildAndRegression.ps1의 Product profile로 Debug, Release를 순차 빌드했다. MaxCompilerProcesses=8이며 clean/rebuild로 무관한 산출물을 삭제하지 않고 정상 의존성 빌드로 수정된 CPP와 소비자를 재컴파일했다. Client/UI를 실행하거나 사용자 프로세스를 종료하지 않았다.

| 최종 검사 | 결과 | 근거 |
|---|---|---|
| Debug x64 Product | 컴파일·링크·배포 PASS,169.609초 | out/BuildPipeline/runs/20261004T110358704Z-debug-product.json |
| Release x64 Product | 컴파일·링크·배포 PASS,166.741초 | out/BuildPipeline/runs/20261004T110658159Z-release-product.json |
| Debug shader closure | PASS,256 producers/171 Client consumers, WARP V1/V2 각1,352pixels | out/BernChunkHlod20261004/final-debug-shader-closure.log |
| Release shader closure | 같은 범위 PASS | out/BernChunkHlod20261004/final-release-shader-closure.log |
| 설치 movie JSON | SHA 일치, 기존 문서와 두 exclusion 외 구조 동일 | out/BernChunkHlod20261004/final-data-validation.json |
| 프로젝트와 manifest | Client/Engine vcxproj·filters4개 XML, BuildDomains JSON parse PASS | 같은 validation JSON |
| 전체 작업 트리 diff 검사 | git diff --check PASS | 2026-10-04 최종 실행 |

Debug Client.exe는20:03:56 KST, Release Client.exe는20:06:56 KST에 갱신됐다. 각 구성의 Engine.dll과 compiled shader 배포도 완료됐다. 기존 인코딩·수치 변환 경고와 Debug 외부 DirectXTK PDB 경고는 남아 있으며 빌드 오류는0이다. Product의 실행 데이터 존재/Navigation/Item·Valtan reward 검사도 통과했지만 다른 domain을 일괄 publish하지 않았다.

사용자 확인 순서는 새 EXE에서 첫 캐릭터 생성 → 다른 슬롯 캐릭터 생성 → 첫 캐릭터 재선택 → Bern 입장, 캐릭터 생성의 차원술사 class movie, Bern 입장 컷신0.5배다. 저장 캐릭터는 process-session이므로 EXE 재시작 뒤 새로 만드는 흐름으로 확인한다. 성능은 같은 카메라에서 도구/경계선을 닫고 Reset한 병합 OFF·ON 및 HLOD 캡처로 비교한다. 최종 실제 재입장 성공·머리 화면·컷신 체감·FPS 개선은 사용자가 아직 확인하지 않았으며 자동 검증으로 대체하지 않는다.

## G12. 2026-10-05 재확인과 사용될 수 없는 청크 생성 제거

사용자는 작은 청크와 동일 재질 병합에서 프레임 개선을 체감하지 못했다고 다시 보고했다. 보존된 G07 병합 OFF/ON 파일을 `Tools/Profiler/analyze_capture.py`로 다시 읽었으며 이전 수치와 일치했다. 전체 draw는 약4%만 줄고 batch+chunk 렌더 CPU는17.040→19.367ms, 제출 indices는355만→425만이었다. 환경 이펙트 갱신도 약9ms가 그대로 남았다. 청크 수나 청크 안 원본 group 수는 이미 instancing·culling된 실제 draw 절감 수와 다르다. G08의 카메라별 index 제한·정적 batch 공통 clock 이후 동일 조건 새 캡처는 없어 그 수정의 실제 FPS 효과를 이번 재분석으로 판정하지 않는다.

DirectionalLight OFF/ON의9월30일 캡처도 재확인했다. 프레임 간격69.240/68.235ms, Render.Lights CPU0.933/0.921ms, NONBLEND CPU21.392/21.130ms였다. 현재 live compare OFF는 directional diffuse/specular RGB를0으로 만들며 light pass 자체를 제거하지 않는다. ambient·RNM·local light와 맵 제출은 계속된다. 따라서 OFF 실험을 모든 조명 계산을 제거한 실험으로 해석하지 않는다. 다만 이 캡처에서 전체 light CPU scope 자체가 약1ms라는 점은 맵 제출·갱신 비용이 더 큰 설명임을 뒷받침한다. 구형 capture에는 directional flag가 없으므로 파일명과 사용자 설명으로 구분하며, export camera 동일과 전체 history 동일을 혼동하지 않는다.

현재 설치 Bern mapmaterials의23,200행 중21,363행은 원본 RNM average/directional bakedLighting을,16,463행은 staticShadow를 사용한다. `Shader_MapMaterialSurface.hlsli`는 그 사전계산 texture를 읽어 간접광을 계산한다. 이미 존재하는 이 조명 베이크와, 서로 다른 재질·조명 texture까지 atlas로 합쳐 큰 구역 proxy를 만드는 geometry/material bake는 다른 범위다. 전체 material 행 수를 현재 화면의 가시 표면 비율로 해석하지 않는다.

실제 추가 수정은 `MapStaticChunkObject.cpp` 하나다. G08은 원본3draw 이상을 대체해야 청크를 선택하지만 Stage는2member도 생성했다. 원본2member가3draw로 늘어날 수는 없어 이 청크들은 매번 선택 실패하면서 파생 GPU buffer·Layer membership·LateUpdate·BVH/선택 비용만 남겼다. 생성과 선택의 최소값을 같은 `MINIMUM_CHUNK_SOURCE_DRAWS=3`으로 연결해 파생 geometry 생성 전에 제외했다. 원본 batch와 배치 ID·picking·그림자·32m 구획·LOD·재질 호환성 검사·실패 fallback은 보존했다. 새로운 material 불변 cache는 추가하지 않았다.

기존 설치 inventory1,970개 중2member는805개(40.86%)이며 이 그룹의 파생 GPU bytes 상한 추정 합은76,511,680bytes(72.97MiB)였다. 이는 이전 전체맵 inventory에 대한 구조적 계산이고 실제 런타임 할당·FPS 측정이 아니다. 확보한 예산으로 기존 순위의 다른 적격 그룹이 생성될 수 있으므로 전체 GPU 메모리가 그만큼 감소한다고 주장하지 않는다.

수정 CPP의 Debug 격리 컴파일과 실제 resolver/claim/Late/Prepare 본문을 사용하는 기존 native fixture26검사가 통과했다. 기존3draw 선택,2visible fallback, geometry 증가 거부, LOD, frame/camera cache, 실패·무효화 경계가 유지됐다. 새 제품 파일·project/filter·맵 데이터·렌더링 옵션 변경은 없다. 재분석·inventory·컴파일·fixture 증거는 `out/BernChunkAdmission20261005`에 보존했다. 통합 Product 빌드는 현재 루트 작업의 결과를 별도로 기록하며 Client 실행·화면 판정·새 FPS 측정은 수행하지 않았다.

## G13. 10-05 최종 Debug·Release 제품 반영

G12의 생성/선택 최소3draw 조건은 캐릭터 선택 복귀 준비 수정과 함께 Debug·Release Product로 컴파일·링크했다. 최종 Release 결과는 `20261004T210803760Z-release-product.json`, 최종 Debug 결과는 `20261004T210827276Z-debug-product.json`이며 둘 다 PASS다. CPP·헤더 소비자만 재컴파일했고 CSO 변경과 map/Resources publish는 없다. 전체 실행·검증 기록은 [캐릭터 슬롯 결과 G07](../10-03/2026-10-03_CHARACTER_SLOT_STATE_RESTORE_RESULT.md#g07-10-05-베른-후속-최적화와-함께-수행한-최종-제품-검증)에 둔다. 텍스처 품질별 잔류 비용과 실제 mip inventory는 [mip 품질 결과 G05](2026-10-04_SYSTEM_OPTION_TEXTURE_MIP_QUALITY_RESULT.md#g05-10-05-텍스처-품질과-베른-성능-재조사)에 둔다. 새 사용자 성능 캡처와 최종 화면 판정은 아직 없으며 후보805개 제외를 실제 FPS 개선율이나 확정 메모리 절감량으로 환산하지 않는다.

## G14. 2026-10-05 Layer 공간 탐색의 반복 CPU 작업 제거

정적 batch 내부 visibility cache 위에서 `CLayer::Submit_FinalCamera`가 같은 카메라에도 BVH 평면 검사·후보 vector 재작성·원래 순서 sort를 반복하고 있었다. `Layer.cpp/.h`는 카메라 평면·그림자 상태가 같고 bounds/topology 변경이 없으며 reject grace가 끝난 경우 후보만 재사용한다. 소비자 callback은 매 프레임 계속 호출한다. 움직이는 카메라는 부모가 충분히 안쪽으로 포함된 평면 mask를 자손에 넘겨 중복 검사를 제거한다. Shadow caster의 강제 통과는 포함 판정으로 전파하지 않는다. 인코딩 UTF-8(BOM 없음)·CRLF를 보존했다.

기존 native fixture를 `out/SpatialLayer20261005`에서 재사용했다. 이전과 수정 실제 Layer.cpp, 정확한 header 및 기존 GameObject bounds/link/invalidation을 사용하고 GameInstance·Profiler·GPU는 stub이다. Debug/Release 각각46,570 assertions, 이전/수정 전체 callback ID·순서 digest `14811484266623696230` 일치. Random 및 perspective camera, DirectXCollision oracle, dirty 이동, duplicate dirty, invalid bounds/camera, remove/clone, shadow 전환, grace 진행과 복구를 통과했다. `git diff --check`도 통과했다.

15794개 synthetic bounds를 사용하는5회 교차 실행 중앙값은 아래와 같다. 시간에는 fixture callback·digest·assertion이 포함된다. GPU·제품 UI 실행과 실제 Bern FPS 실측이 아니며 draw 수는 그대로다.

| 조건 | Debug 이전→수정 | Release 이전→수정 |
|---|---:|---:|
|16개 표시, 정지100회 합계|0.7788→0.2104ms|0.1237→0.0631ms|
|전체 표시, 정지200회 합계|1118.160→79.0658ms|200.149→9.3276ms|
|전체 표시, 이동200회 합계|1107.190→512.973ms|196.824→150.883ms|

측정과 각 회차 로그는 `out/SpatialLayer20261005/comparison.json` 및 같은 폴더에 있다. 제품 통합 빌드는 별도 검증에 기록한다.

## G15. 반복 모델의 실제 제출 결합 확대

G07의 1,151.571→1,105.497 draw(-4%)는 이전 청크 OFF/ON 비교이며, 프로젝트의 최초 비인스턴싱 상태 대비 총 개선율이 아니다. 이번 변경 이후 실행 캡처는 아직 없으므로 그 수치를 새 변경의 성과로 재사용하지 않는다.

기존 결합은 단일 mesh와 최대8개의 연속 source batch에 제한됐다. `CModel::Can_ShareStaticInstanceStateWith`는 동일 CMesh·CMaterial·pretransform을 사용하는 정적 모델을 확인한다. Client는 셀별 가시성 payload를 보존하고 같은 mesh별 LOD·claim·mirror·render profile·시간인 연속 후보를 묶는다. 이 exact 경로는 조명 bank가 필요 없어8개 제한 없이 기존 shader로 mesh당 한 번 제출한다. world/wind/RNM instance payload는 그대로 복사한다. 짧은 exact prefix가 더 큰 RNM bank를 분할하지 않도록 AA+B 형태에서는 기존 bank에 양보한다.

서로 다른 RNM을 사용하는 같은 geometry도 모든 mesh의 표면이 기존 source BG bank와 호환하면8개까지 mesh별로 묶는다. 이전 single-mesh 제한을 제거하고 mesh별 실제 material slot을 바인딩한다. 업로드 이전 실패는 원본을 유지하고, mesh를 하나라도 제출한 뒤 실패하면 원본 전체를 다시 제출하지 않는다. 청크 선택의 원본 draw 하한도 multi-mesh bank와 exact instancing 기준으로 갱신했다. 따라서 새 인스턴싱으로 이미 싸진 원본을 청크가 불필요하게 대체하지 않는다.

Profiler 전체 작업량/요약과 JSON에 `mapIdenticalInstanceSourceDraws/Draws`, `mapLightingBankSourceDraws/Draws`를 추가했다. SourceDraws는 그 프레임의 결합 전 개별 제출 수이며 이전 제품 대비 차이나 전체 scene 절감량이 아니다. JSON 원래 counter 순서는 유지하고 뒤에 추가했다.

실제 production 함수의 dependency 경계에 stub을 둔 native fixture 결과:

| 검증 | 결과 | 근거 |
|---|---|---|
|동일 모델20batch×3mesh|60→3 제출,66 assertions PASS|out/BernInstanceCoalescing20261005/results.json|
|서로 다른 RNM8batch×3mesh|24→3 제출,Debug/Release 각110 assertions PASS|out/BernMultiMeshBank20261005/verification.log|
|실제 Model compatibility/material slot|각35 assertions PASS; slot2,0,1·나중mesh실패·morph·device|같은 폴더 model_contract_probe|
|청크 원본 비용 하한|31 assertions PASS; mesh ordinal·LOD·multi/exact veto|같은 폴더 admission fixture|

이 표는 GPU 화면이나 베른 실행 FPS 검증이 아니다. opaque/masked를 geometry별로 묶으면 submesh 사이 제출 순서가 달라질 수 있다. 깊이가 정확히 같은 겹침에 draw 순서로 재질을 선택하는 authoring은 안정적인 별도 overlay 계약이 필요하다. 이번 fixture의 instance payload/LOD 동등성을 모든 화면 픽셀의 동일성으로 확대하지 않는다.

설치 데이터에서 전mesh source BG인 반복 multi-mesh 모델145종/재질변형1,082개/표시 배치2,273개를 확인했다. 7mesh PILLAR02처럼 overlay가 섞인 모델은 RNM bank 대상에 포함하지 않는다. TOWER07은17배치가12개 조명변형과15개32m batch로 갈라지는 반복 사례지만 사용자 화면의 파란 첨탑과 같은 대상인지는 확인하지 않았다. 전맵 데이터 분석은 현재 camera-visible draw와 다르다.

Bern `Stage_PlacementRuntime`은 source BG deferred group만 기존 eligible slot 안에서 modelRelativePath·mirror·원래 key 순으로 배치한다. unsupported group의 위치/상대순서, 32m culling group, placement payload와 stable ID, picking lookup 및 rollback은 유지한다. 모델 복제나 GPU buffer를 추가하지 않으며 전역 Renderer 정렬도 아니다. 실제 production 정렬블록은 Debug/Release 각각18,618 assertions로18,610group/49,027stable placement ID 및 기대 순서 보존을 통과했다(`out/BernStageOrder20261005/results.json`).

설치 데이터 전체를 같은 LOD·청크 미사용으로 가정한 offline 제출 계산은 기존 asset 순서18,526→이번 source BG 제한 geometry 순서16,823(-1,703/9.19%)다. 런타임 SRV pointer 및 현재 camera-visible draw를 실행한 수치가 아니다. 짧은 exact-prefix의 bank 양보와 hidden group 정렬 후 visible filtering까지 반영했다(`out/BernMaterialChunkAudit20261005/stage-order.json`). TOWER07의 같은 조건 후보는18→10draw이며 이보다 작은8draw는 아직 구현하지 않은 material별 추가 정렬의 가정이다.

## G16. NPC와 환경 계산 공유

`CNpc::Initialize`에서 기존 authored town NPC의 animation culling 허용 경계에 `Enable_AnimationSampleReuse()`를 연결했다. 기존 CAnimation은 cooked channel 내용과 bone index가 같고 정확한 track time이 같을 때 local sample을 재사용한다. 서로 다른 phase를 강제로 변경하지 않으며 각 actor의 clock·blend·root suppression·combined matrix·skin palette와 Server 권위를 유지한다. Player/몬스터/Esther는 이번 opt-in 대상이 아니다.

게시 Bern44명의 실제 설치 idle channel을 분석하면28개의 exact sample group이며 중복6그룹에22명이 속한다. 이는 동시 재사용 hit 수가 아니다. 실제 설치3clip/379bones의 production Animation/Channel/Bone 및 skin palette fixture는 Debug/Release 각각2,474,255 assertions를 통과했다. 동일dt로 진행한1,800pair-frame에서1,800reuse를 확인했고, 독립phase·offscreen clock·root 처리를 함께 비교했다. 증거는 `out/NpcSampleReuse20261005`에 있다. 기존 `Animation.Channels.Reuse/Evaluate` 계측은 그대로 사용한다.

`Effect_Playback.cpp`의 particle-root inverse 호출3곳은16float 전체 bit가 같으면 thread-local 단일-entry 값을 재사용한다. 실제 함수 비교로 Debug/Release 각각34,383matrix·13,442velocity·80,000parallel checks가 bitwise 일치했다. Bern의 정적world-emitter 최대3,442slot을 사용하는 해당 연산 benchmark는 Debug109.287→9.532ns, Release10.033→5.739ns였다. 매 particle마다 root가 바뀌는 입력은 비교 비용으로 약8~12% 느려져 모든 이펙트/FPS가 빨라졌다고 주장하지 않는다. 증거는 `out/BernAmbientInverse20261005`에 있다.

물은 기존 shader의 elapsed-time 기반 UV/normal panning이며 CPU skeleton animation을 물체마다 계산하는 구조가 아니다. 식생은 공통 Bern batch clock과 shader wind 계산을 이미 사용하고 개별 owner/position/dimension payload를 유지한다. 이번 제출 결합도 그 값을 변경하지 않는다. RNM 환경광은 이미 bake된 입력이므로 추가 light bake만으로 render submission/animation/particle 비용이 사라지지 않는다.

상용 엔진의 대응 구조도 같은 구분이다. [Epic HLOD 문서](https://dev.epicgames.com/documentation/unreal-engine/hierarchical-level-of-detail-overview-in-unreal-engine)는 proxy geometry와 atlas material을 함께 생성한다. [Mesh Drawing Pipeline](https://dev.epicgames.com/documentation/en-us/unreal-engine/mesh-drawing-pipeline-in-unreal-engine)은 static draw state 캐시·compatible instancing을 별도로 다룬다. 현재 구현은 정확한 원본 instancing과 동일 surface 청크 병합이며, 임의의 서로 다른 shader/material을 atlas로 bake하는 범용 proxy cooker는 완료 범위가 아니다.

## G17. 10-05 반복 모델·NPC 공유 최종 제품 검증

정상 Product runner를 Debug/Release 각각1회 실행해 Engine→Shared→Server→Client 컴파일·링크·배포를 통과했다. Debug 기록은 `out/BuildPipeline/runs/20261004T213715153Z-debug-product.json`(161,729ms), Release는 `out/BuildPipeline/runs/20261004T214112459Z-release-product.json`이다. Client 출력은 각각 OBJ230/221, binaries2, CSO0이며 tracking identity 변경은 없었다. 청소/Rebuild/tracking 강제삭제는 수행하지 않았다. 기존 C4819/C4828 및 Release DirectXTK PDB 경고가 있으나 빌드는 성공했다.

수정 C++13개 파일의 UTF-8(BOM 없음)과 파일별 개행을 보존했다. 기존 신규/untracked `MapStaticChunkObject.cpp`는 LF이고 나머지12개는 CRLF다. 두 제품 빌드 동안13개 파일의 hash가 동결본과 일치했다. Engine/Client 프로젝트와 filters4개의 XML parse, 해당 변경의 `git diff --check`, 신규 청크/문서 whitespace 검증을 통과했다. `out/BernInstanceCoalescing20261005/validation.json`에 실행 증거를 기록했다.

제품 결과는 `Client/Bin/Debug`와 `Client/Bin/Release`에 반영됐다. Resources/맵 데이터/팀장 rendering option 게시·수정은 없었다. 저장소 규칙대로 Client·UI는 실행하지 않았으며 실제 Bern draw/FPS·화면 확인은 미실행이다. 따라서 synthetic60→3/24→3이나 전맵후보9.19%를 실제 베른 성능 개선율로 보고하지 않는다. 이전의 무관한 미커밋 작업이 함께 있는 상태여서 일괄 commit/push는 하지 않았다.


## G18. 10-05 Occlusion kernel·실제 Bern geometry 교차 검증

`CModel::Try_GetStaticOcclusionMesh`는 기존 immutable picking geometry의 pretransform 적용 position과 원본 index를 복사 없이 차용한다. 44byte vertex stride에서 position XYZ만 읽고, NONANIM·원본 geometry·수정되지 않은 VB만 허용한다. 기존 `Prepare_StaticPickGeometry`가 초기화 때 검증한 triangle 수와 원본 IB 수를 비교해 NaN·잘못된 index가 제거된 입력을 거부한다. 실패하면 출력은 보존한다. 실제 Model.cpp의 MSVC 격리 컴파일과 두 production 함수 본문을 사용하는 storage fixture26검사를 통과했다. 이 fixture의 모델/mesh 저장소는 대역이며 전체 Client model loader 실행으로 표현하지 않는다.

실제 `COcclusionCuller`와 Intel MOC를 연결한 창 없는 D3D11 hardware/WARP 교차 검증을 수행했다. 첫 검사에서 NDC winding 부호가 D3D `FrontCounterClockwise=false`와 반대여서 GPU가 버린 면을 occluder로 사용하는 오류를 재현했다. kernel의 BACK/FRONT 부호를 수정하고 같은 fixture를 다시 통과했다. 최종은 viewport와 같은 pixel center의 MOC, 가장 먼 triangle w, 앞쪽으로 bias한 query depth와 확장 bounds를 사용한다. 저해상도 triangle별 축소안은 최종 구현이 아니다.

두 장치에서 각각171개 geometry/camera 조건을1920×1080 및1919×1077로 검사했다. 합계684case·531,468bounds query 중255,436개가 hidden으로 판정됐다. 해당 geometry를 GPU의 원본 occluder depth에 실제 그렸을 때 잘못 숨겨진 visible pixel은0개이며 D3D 오류도0개였다. 벽의 내부 diagonal, 작은 틈, 얇은 면, 기울어진 깊이, mirror, BACK/FRONT/NONE, near/far 교차와144개 camera orbit 변형을 포함한다. 고정 sample 집합의 교차 검증이며 모든 가능한 scene의 수학적 증명이나 제품 화면 판정은 아니다.

설치 Bern 원본1,755geometry mesh에서 변형 없는 SOURCE_BG17,414model occurrence/20,777mesh occurrence를 읽고, 사용자10월4일 캡처의 서로 다른 export-time camera6개를 적용했다. masked BG는 가림당하는 대상으로 포함하고, 실제 불투명 source만 가림 근거로 썼다. 원본32m asset/mirror batch의 tight AABB 전체가 가려져야 제외한다. 후보 순위는 projected sphere area/√triangle 수, 최대64occluder batch·총100,000input triangle·callback당16,384input triangle과callback 사이1.5ms soft limit 정책을 적용했다. 아래는 hardware 실행20회 중 마지막 sample의 batch/geometry 판정과20회 평균 kernel 비용이다.

| Camera 위치 XYZ | 적격 frustum batch→남은 batch | 숨긴 instance | 숨긴 원본 triangle 상한 | depth 재생성 평균 | bounds 검사 평균 |
|---|---:|---:|---:|---:|---:|
|129.750, 53.469, -14.250|76→68|8|4,954|1.593ms|0.009ms|
|126.876, 54.286, -60.232|158→126|40|28,730|1.565ms|0.025ms|
|73.986, 59.074, -118.822|124→118|6|4,498|1.549ms|0.013ms|
|125.636, 54.189, -70.166|137→88|62|40,182|1.632ms|0.038ms|
|127.330, 54.127, -69.801|123→68|77|42,910|1.976ms|0.042ms|
|130.922, 53.490, -5.175|75→72|3|1,002|1.549ms|0.007ms|

20회 반복의 hidden instance 범위는 camera 순서대로8,40,6,62~63,77~78,3~4였다. 마지막 callback 실행 시간이 제한을 넘을 수 있으므로1.5ms는 엄격한 상한이 아니다. fixture 전체맵 탐색·group 생성은 별도로0.459~0.558ms였으며 이를 제품 Renderer 비용으로 대체하지 않는다. hardware/WARP의 마지막6scene 각각에서 잘못 숨겨진 GPU visible pixel은0개였다.

이 표는 production kernel을 실행한 installed-geometry 정책 fixture이고 production Renderer/Client object graph 실행이 아니다. 초기 frustum margin·reject grace·chunk claim·인접 instancing을 재생하지 않았으며, mesh당8,192triangle 이상은 실제 LOD1/2 가능성이 있어 occluder에서 보수적으로 전부 제외했다. 제외 triangle 수는 LOD 전 원본 상한이고 실제 GPU primitive 또는 draw 감소가 아니다. 고정 camera에서도 매회 depth를 새로 만든 비용을 측정했다. 제품의 완전 동일 camera/owner/bounds/revision cache 경로 비용과 실제 Bern FPS·메모리·사용자 화면은 측정하지 않았다.

입력 camera 경로/hash, current scene binary, 실제 kernel object/hash, 두 장치 JSON, adapter 검사와 한계는 `out/BernOcclusion20261005/verification-receipt.json`, `input-summary.json`, `production-hardware.json`, `production-warp.json`, `synthetic-*.json`에 보존했다. Resources·맵 데이터·저작 rendering option을 수정하지 않았고 Client/UI를 실행하지 않았다. 제품 통합 빌드와 Client callback/cache 자체 검증은 다음 결과에서 구분한다.

## G19. Bern 가림·거리 컬링의 제품 연결과 cache

`CRenderer::Render_NonBlend`는 G-buffer를 채우기 전에 Bern 정적 batch의 opt-in descriptor를 받아 stable filter를 적용한다. 기본 GameObject는 opt-out이며 캐릭터·NPC·다른 map owner를 이 경로에 강제로 넣지 않는다. 실제 CPU depth는 검증된 원본 LOD0 불투명 SOURCE_BG 삼각형만 사용한다. masked BG는 가려질 수 있으나 가림 근거로 사용하지 않는다. instance별 보수적인 tight world AABB의 visible 합집합은 기존 GPU payload 성공과 함께 commit한다. 그림자 queue와 picking은 원본을 유지하고 남은 NONBLEND 목록은 기존 adjacent instancing을 계속 소비한다.

전체100,000 input triangles·callback당16,384·최대64기여 batch와 callback 사이1.5ms soft budget을 사용한다. 입력 수와 rasterized 수를 구분하며 partial depth는 놓친 제거 기회만 허용한다. 가림 결과 cache는 view/projection/viewport/settings revision/weak ownership/full descriptor와 geometry revision이 모두 같을 때만 재사용한다. unsupported camera·불완전 입력·할당/준비 실패는 원본 queue를 유지한다. 제외 counter는 실제 queue 교체 뒤에 기록한다. 정확한 카메라·정적 배치가 유지되면 depth를 재생성하지 않으며 cachehit를 따로 센다.

Distance culling은 Bern의 작은 정적 소품·식생을 대상으로 sphere 반지름0.5/1.5/3m 이하에 각각35/45/60m의 거리와 투영 지름24pixel 조건을 함께 사용한다. 숨김 진입은 거리1.1배와 pixel0.9배, 유지에는 기본 조건을 사용한다. 큰 지형·큰 건물·landscape·gameplay entity는 이 조건으로 없애지 않는다. invalid/near-plane/지원하지 않는 projection은 표시를 유지하며, 같은 aspect의 resize도 viewport cache key로 재계산한다. DistanceTested는 이번 계산 작업, RejectedInstances/Indices는 성공한 committed 가시성 기준이고 cachehit에도 현재 제외량을 기록한다. 인덱스는 원본 상한이며 실제 LOD/GPU 시간과 다르다.

설치 전체48,456개 적격 placement를 조사한6카메라의 frustum visible 수는871/886/536/770/746/684, 반지름3m 이하 후보는542/463/395/348/324/407이었다. 거리 조건만으로는35/14/8/21/21/24개였으나 화면 지름24pixel까지 함께 만족한 제외는 모두0개였다. 따라서 기본 distance 설정의 성능 향상은 이 자료로 입증되지 않았다. F7의 거리 scale·pixel 상한은 실행 세션 A/B용이고 팀장 quality JSON을 덮어쓰지 않는다. 강한 정책을 기본으로 몰래 바꾸지 않았다.

Client 실제 production 함수 native fixture는 Debug/Release 각각26,318 assertions로 tight AABB/signed scale/shear·masked 구분·LOD/cull·revision·hysteresis·viewport 변경·업로드 실패 시 payload 보존을 통과했다. 당시 Renderer 실제 함수의 dependency 대역 fixture38검사는 stable queue·unsupported·실패 보존·settings/camera/static revision invalidation을 통과했다. actual Renderer/GameInstance와 Client3CPP 최소 컴파일도 통과했다. 근거는 `out/BernLargeProxy20261005/client-source-freeze.json`, `client-probe-*-run.log`, `distance-all-coverage.json`, `out/BernOcclusion20261005/renderer_probe_result.json`이다. G18의 정책 fixture hash는 최종 viewport/counter 보완 이전이며 최종 제품 소스는 다음 build freeze를 따른다.

## G20. 큰 atlas 후보의 완료 경계와 첫 통합 빌드

작은 fine chunk의 Bern Stage 호출을 제거했다. 큰64m atlas 후보는 `atlasEnabled=false`이면 준비·owner 생성·추가 source index 추적을 수행하지 않는다. CModel/xatlas geometry·visible source IB compaction·six-channel atlas bake·입력 SHA 기반 geometry/atlas 파생 cache는 구현했지만 현재 품질 조건에서 저장6카메라의 유효 source가0개여서 제품 기본 경로에 활성화하지 않았다. Loader의 사용하지 않는 proxy shader prototype 등록도 제거했다. 후보 활성화에는 해당 shader 등록과 실제 quality/coverage 확인이 추가로 필요하며 완성된 실사용 proxy로 표현하지 않는다.

재질 atlas는 원본 TBN과 UV·vertex color·instance RNM/정적 그림자를 유지한다. 정적 diffuse indirect는 bake할 수 있지만 시점 의존 RNM specular는 runtime 카메라 입력으로 계산한다. 반사/parallax/rim/subspecular/masked/wind/시간 의존 재질은 이 후보의 bake 대상이 아니다. 개인 LocalAppData의 파생 cache만 쓰고 Resources나 저작 데이터는 교체하지 않는다. 후보 shader의 hardware/WARP 각256MRT 검사, native material24검사와 baker/cache/padding/context-state4114검사를 통과했다. 이는 제품의 시각 품질·coverage·실FPS 확인이 아니다. 근거는 `out/BernProxyShader20261005`, `out/BernAtlasProxy20261005/baker_results.json`, `out/BernLargeProxy20261005/policy-coverage.json`이다.

컬링 연결 직후 정상 Product Debug 컴파일·링크·배포는 PASS했다. 기록은 `out/BuildPipeline/runs/20261004T223711650Z-debug-product.json`, 상세 log는 `out/BernOcclusion20261005/product-debug.log`이다. 신규/수정 product source90파일은 이 빌드 전후 hash가 동일했고4project XML parse와 diff check를 통과했다. `product-source-freeze.json`에 보존했다. 이 시점 Release는 별도로 완료하지 않았으며 사용자가 후속 요청한 공통 worker와 맵 CPU 준비 병렬화를 포함한 최종 Debug/Release 검증에서 다시 확인한다. Client/UI·실Bern FPS는 실행하지 않았다.

## G21. 공통 worker 풀과 Layer의 큰 CPU 작업 분배

사용자가 요청한 매 프레임 CPU 병렬화를 기존 파티클 Windows thread pool을 Engine `Run_CpuJobs`로 승격해 연결했다. 최대3helper(작은 CPU에서는 processors−2 이하)와 caller가 같은 coarse index 목록을 소비하고 모든 callback을 join한 뒤 반환한다. 기존 `Run_EffectParticleUpdates`는 thin wrapper로 이 풀을 사용하며 기존 최대1helper·emitters4개/particles512개·의존 event 제외 조건을 유지한다. 별도의 중복 프레임 pool, fiber scheduler나 D3D deferred context를 추가하지 않았다.

`CGameObject::Try_PrepareFinalCameraCpuJob`은 opt-in 경계이고 기본 객체는 참여하지 않는다. Layer는 기존 final-camera BVH에서 카메라와 교차한 CPU 후보만 별도로 보존하고 owner snapshot을 수집한 뒤 비용128instance 목표·최소8/최대64batch 묶음으로 dispatch한다. 화면 밖 shadow caster와 grace 후보의 기존 owner 제출은 유지하되 상세 CPU job 수집에는 넣지 않는다. 총512cost 미만 또는 단일 range는 caller 직렬 실행이다. cachehit는 job을 만들지 않는다. join 뒤에는 기존 후보 순서로 Submit_FinalCamera를 호출하므로 GPU Map/Unmap·render queue·원래 adjacent instancing 순서를 유지한다. pool nested 호출은 caller/worker TLS guard로 직렬 처리하고 외부 동시 제출은 mutex로 분리한다. callback 예외는 이미 실행 중인 모든 callback을 기다린 뒤 rethrow하며 partial writes의 transaction은 소비자가 소유한다. 이 동기 경계는 [Windows callback join 계약](https://learn.microsoft.com/en-us/windows/win32/api/threadpoolapiset/nf-threadpoolapiset-waitforthreadpoolworkcallbacks)을 사용한다.

공통 pool production TU와 실제 particle wrapper를 링크한 Debug/Release native 검사는 각각1,017,520 assertions PASS였다. caller48개/worker144개의 중첩 호출, 외부4caller×80batch 분리와16회 예외의 join 후 전달을 확인했다. 추가로 모든 lane 진입 뒤 caller/worker에서 실패시킨2조건의 join을 확인했다. API 실패/적은CPU 등을 주입한5경로64검사도 모두 PASS했다. Layer production CPP와 같은 pool을 직접 컴파일한 최종 fixture는 Debug/Release 각각68,834검사로 unsupported/empty/cachehit0·496cost 직렬/512cost 병렬·coarse range 경계·각job1회·원래 제출 순서·예외 후 owner fallback을 통과했다. 혼합27객체에서 그림자8개를 포함한 owner19개를 유지하고 CPU 준비만11개로 좁혔다. 그림자 전환·dirty refit·정지 cache·grace·6면 접선·완전 내포 plane mask와512객체×12 perspective의 독립 DirectXCollision oracle도 대조했다. 실제 Engine_RenderTypes.h의 기본 OFF를 확인하고 OFF의 준비 호출·worker callback·준비 scope0, ON→OFF에서 이전 job 미실행과 owner/shadow 제출 보존도 확인했다. dependency의 GameObject/GameInstance/Profiler는 대역이며 Client/UI를 실행한 검증이 아니다.

근거는 `out/CpuJobPool20261005/result-Debug.json`, `result-Release.json`, `fallback-result.json`, `out/BernWorker20261005/layer/validation-result.json`이다. 합성 CPU workload는 pool에서 약11.29~11.31→2.99~3.03ms였으나 실제 맵 계산·전체 frame/FPS 성능으로 사용하지 않는다. Layer 합성 workload 시간은 동시 작업에 따라 변동해 성능 판정에 사용하지 않는다. 첫 사용 시간에는 고의 callback 계산까지 포함하므로 pool startup 비용으로 표기하지 않는다. 실제 Bern 계산 비교는 다음 항목에서 분리한다.

F7에는 기본 OFF인 `Bern parallel visibility preparation` 세션 switch를 추가하고 capture에 저장 시점 값을 기록한다. OFF는 Layer의 job 수집·계측 scope도 생략한다. prepared batch/coarse job/caller 완료/worker 완료/submitted assistant 수와 `Map.Visibility.Prepare/Dispatch/Join/Worker` CPU scope를 제공한다. Assistants는 실제 일한 코어 수가 아니라 제출한 보조 callback 수이며 worker 작업 수를 함께 확인한다. Dispatch는 caller 계산과 join을 포함하므로 worker 합계와 더하지 않는다.

NPC 전체 Update에는 network/collider/effect/sound가, 환경 occurrence 전체 Update에는 source events/provider/collision/model cue와 렌더 연결이 있으므로 통째로 worker에 넘기지 않았다. 일반 NPC pose와 여러 환경 occurrence 사이의 순수 particle phase를 분리해 일괄 dispatch하는 경로는 이번 완료 범위가 아니다. 기존 NPC exact sample reuse와 particle-root inverse 공유는 유지한다. 로딩 worker의 COM·취소 I/O 계약은 이번 동기 프레임 pool과 별개다.


## G22. 실제 맵 CPU 준비 분리와 병렬 활성화 판단

`MapStaticBatchObject`는 owner Stage → 배치별 독점 CPU 계산 → owner GPU commit으로 분리했다. worker는 frustum·거리 판정과224byte instance payload·LOD/occlusion bounds를 임시 결과로만 만든다. plane 검증 cache4개는 thread-local이며 camera 전역 cache는 owner만 갱신한다. camera 본문은 동기 join까지 차용하고 CPU 완료/실패 시 pointer를 비우며 revision은 별도로 보관한다. READY 결과의 GPU Map 재시도는 CPU 계산과 grace를 반복하지 않는다. frustum와 거리 hysteresis도 업로드 성공 후 payload와 함께 commit한다. authoring 변경은 준비 상태를 무효화한다.

카메라 밖 broad reject는 상세 preparation 할당 전에 빈 payload로 바로 commit한다. 이 경로는 GPU Map 없이 성공하며 source별 frustum/hysteresis는 건드리지 않는다. Layer의 CPU 준비 목록은 기존 BVH의 카메라 교차 leaf만 사용하고 shadow-only leaf의 기존 Submit은 보존한다. 정지 cache는 job·scratch reserve·CPU 재계산 없이 현재 frame 준비를 확정하며 rebuild/cachehit counter를 한 번만 기록한다. inspection lease와 viewport도 거리 가시성 cache의 입력이다.

원래 production `Prepare_FinalCameraVisibility/Upload_VisibleInstances`, 변경된 Stage/Compute/Upload, 실제 `Evaluate_FrustumVisibility` 및 plane helper, 실제 공통 pool을 사용한 native fixture는 제품 hot-file과 같은 Debug `/O2 /MDd /D_DEBUG`, Release `/O2 /MD`에서 각각 **6,966,511검사 PASS**였다.12번의 camera/settings 변경에서 GPU payload byte·source index·LOD sphere/tight bounds/scale·occlusion AABB·frustum grace·거리 상태가 원본과 같았다. 할당 실패 주입, 미실행 Stage의 owner fallback, Map 실패 시 이전 payload/frustum 보존과 READY 재시도, CPU 실패 상태의 다음 frame 복구, 동일 aspect resize와 inspection 전환, 빈 commit의 무할당 및 counter1회를 확인했다. 일반 CPU 수학 입력에서 자연 발생하는 예외는 없었으므로 CPU 실패 recovery 검사는 저장된 FAILED 상태를 주입한 검증이다.

기존 distance/occlusion 함수 fixture도 Debug/Release 각26,318검사를 다시 통과했다. Batch/Chunk/Placement/MapAssetRenderUtils4CPP 최소 컴파일과 UTF-8(BOM 없음)/CRLF·scoped diff check를 통과했다. 제품 Debug/Release 전체 빌드는 통합 담당자의 최종 빌드 항목에서 별도로 판정한다.

설치된 opaque non-skinned 맵 inventory의18,407그룹/48,498instance와 저장6카메라를 사용해27sample 중 처음3개를 버리고5실행 모드 순서를 교대했다. shadow 전체 상한에서 CPU 준비 후보는848~1,149그룹이었다. 최종 최적화 Debug의 전체 shadow 상한 p50은 원본2.79~3.40ms, 새 direct OFF2.24~2.76ms,3helper2.31~2.99ms였다. Release는 각각2.14~2.79ms,2.00~2.55ms,2.02~2.53ms였다. 화면 후보만의 Release는 원본0.139~0.216ms, direct OFF0.152~0.219ms,3helper0.133~0.229ms로 view별 개선과 회귀가 섞였다. 정지 Release는 원본0.772ms/direct OFF0.841ms/3helper0.850ms였으며 warmup 이후 job은 만들지 않았다.1helper도 안정적인 우위를 보이지 않았다.

따라서 저렴한 broad empty 처리의 이득과 worker 분배의 이득을 구분한다. **병렬 ON의 안정적인 전체 순이득은 입증되지 않았으며 Debug/Release 기본값은 OFF**, F7 opt-in과 계측은 유지한다. OFF에서는 Layer가 gather 자체를 생략해 추가 고정비를 피한다. 이전 비최적화 `/Od` Debug 수치는 진단 로그로만 보존하고 제품 개선 근거로 사용하지 않는다.

수치는 CPU 준비/commit 대역에 한정된다. D3D Map·Profiler·GameInstance는 dependency 대역이며 GPU 비용과 실제 Layer BVH traversal은 포함하지 않는다. Layer AABB predicate와 batch margin으로 계산한 CPU 후보를 timing 밖에서 재사용했고 실제 Layer CPP 검사는 G21에 따로 남긴다. camera revision을 바꾸어 재계산을 유도했으며 연속 이동 capture를 재생한 것은 아니다. 원본 world sphere/camera는 실제 입력이지만 payload의 world/material identity는 결정적인 검사 입력으로 치환했다. Client/UI·실FPS를 실행하거나 개선을 확정하지 않았다. 근거는 `out/BernVisibilityWorkers20261005/result.json`, `source-freeze.json`, `production-provenance.json`, `debug-run.log`, `release-run.log`이다.

## G23. 컬링·공통 worker 최종 Debug/Release 제품 검증

기본 OFF와 Layer의 수집 생략까지 포함한 정상 Product runner를 Debug/Release에서 실행해 Engine→Shared→Server→Client 컴파일·링크·배포를 통과했다. Debug는171,654ms이며 `out/BuildPipeline/runs/20261004T231207424Z-debug-product.json`에 기록했다. Client는154OBJ와2개 binary가 갱신됐다. 최종 Release는13,098ms이며 `20261004T231239360Z-release-product.json`이다. 기존 유효 산출물을 재사용하고 Engine9OBJ를 다시 컴파일·링크했으며 Client는0OBJ/배포 binary1개 갱신이었다. 최종 실행에 앞서 같은 작업 트리의 Release Product `20261004T231049631Z-release-product.json`도 PASS 상태였으며, 이번 최종 증분을 모든 TU의 신규 컴파일로 설명하지 않는다. Clean/Rebuild나 tracking 삭제를 사용하지 않았다.

첫 Debug 시도는 Layer의 새 설정 접근에 필요한 완전한 타입 include 누락으로 실패했다. `Engine_RenderTypes.h`를 직접 include하고 source를 다시 동결한 뒤 위 두 제품 검증을 통과했다. 최초 실패 로그는 `out/BernWorker20261005/product-debug-incomplete-type.log`에 보존했다. 기존 문자 집합·수치 변환·외부 library PDB 경고는 남고 최종 빌드 오류는0이다. 데이터 publisher는 실행하지 않았다.

최종 source90개는 두 빌드 전후 hash가 동일했다. Engine/Client project·filters4개 XML parse, BuildDomains JSON parse, 전체 `git diff --check`, worker/Layer/Client CPU 변경의 UTF-8(BOM 없음)·CRLF를 확인했다. 최신 설정 기본값을 반영한 Renderer 실제 함수 fixture41검사도 PASS다. Layer는 실제 설정 헤더를 포함해 Debug/Release 각68,834검사를 통과했다. 영수증은 `out/BernWorker20261005/final-validation.json`, source 동결은 `product-source-freeze.json`, 제품 로그는 같은 디렉터리의 `product-debug.log`와 `product-release.log`다.

현재 적용 범위는 다음과 같다.

| 항목 | 제품 상태 | 확인된 효과와 남은 경계 |
|---|---|---|
| Occlusion culling | Bern NONBLEND 연결·기본 ON | 실제 geometry/kernel 제거 및 GPU 보수성 검사 PASS. 제품 FPS·사용자 화면은 미측정 |
| Distance/screen-size culling | Bern 작은 소품에 연결·기본 ON | 저장6camera에서 기본24pixel 조건의 추가 제외0. 개선을 입증하지 않음 |
| 기존 mesh LOD | 기존 조건과 실제 선택 유지 | 대상 확대·인스턴스별 LOD 재배치 미구현. >=8,192tri/보수적 화면오차 등 기존 한계 유지 |
| 공통 CPU worker | Engine pool과 기존 particle 소비자 연결 | 맵 준비 병렬 경로 구현·정확성 PASS. 안정적 순이득 미입증으로 Debug/Release 기본 OFF |
| 반복 geometry instancing·CPU 재사용 | 기존 adjacent coalescing·NPC exact sample·particle inverse 개선 유지 | 실제 Bern draw/FPS 개선율은 새 capture 없음 |
| 작은 fine chunk / 큰 atlas proxy | 작은 청크 생성 중단 / atlas 기본 비활성 | 작은 owner 고정비 제거. atlas는 현재 품질 조건의 적격 source0으로 활성화 미완료 |
| 모코코 슬롯 복귀 | 앞선 loading/model-ready gate 유지 | 선택 슬롯·저장 avatar 준비 후 전환 구현. 실제 첫 화면 판정 미실행 |

새 Client/UI·게임 화면·FPS 캡처를 실행하지 않았다. CPU 계산 fixture·GPU kernel 검사·제품 빌드 성공을 실제 플레이의 프레임 상승으로 대신 보고하지 않는다. 텍스처 품질은 기존 mip sampler 연결을 유지하며 draw/geometry·재질/pass·particle 계산과 VRAM residency를 제거하는 옵션이 아니라는 G05 조사 결론도 그대로다. 다른 세션과 사용자의 광범위한 미커밋 변경을 보존했고 일괄 stage/commit/push는 하지 않았다.


## G24. 사용자 원경·이동·컷신 캡처의 실제 병목

사용자는 앞선 변경 뒤 약20→30FPS 상승을 보고했다. 같은 카메라의 분리된 ON/OFF 실험은 아니므로 그 증가를 occlusion 하나의 효과로 확정하지 않는다. 이어 저장한 아래3종은 같은 Debug process39956의 연속 capture다. 원본 JSON의 SHA256과 분석은 `out/BernCutscene20261005/findings.json`과 `analysis-all.json`에 보존했다.

| 저장 이름 | frame 수 | 평균 frame interval | interval 역수 FPS | 평균 CPU |
|---|---:|---:|---:|---:|
| 베른성_줌아웃 | 239 | 61.641ms | 16.223 | 61.133ms |
| 베른성_컷신연출 | 384 | 121.409ms | 8.237 | 120.641ms |
| 베른성_줌아웃이동 | 139 | 88.047ms | 11.358 | 87.029ms |

각각 다른 카메라 작업량이며 개선 전후 A/B 표가 아니다. 컷신 파일의 처음14frame(240~253)은 이전 정지 장면이고, 이동 파일에도761~769의 큰 draw 증가 구간이 포함된다. export metadata는 저장 시점의 카메라·설정만 설명한다. 세 파일 모두 Debug·iterator debug2·D3D debug layer ON·RTX4070·1920×1080·프레임 제한0이며 GPU sample 유효율100%, CPU/GPU 누락0, 상세 draw trace OFF다. 모든 frame에서 shadow draw는0이다.

컷신 진입 경계 frame253→254의 실제 draw는2,216→10,004회, VS invocation은2.831M→11.958M으로 늘었다. 반면 전체 PS invocation은30.006M→28.446M으로 줄었다. NONBLEND CPU는42.123→254.011ms이고, 그 안의 Map.Batch.Draw는23.30→130.96ms다. 따라서 이 구간을 단순히 조명·픽셀 연산량 증가나 가림 계산 비용으로 설명할 수 없다. 가장 느린 연속20frame(254~273)은 평균9,636.65draw, CPU272.743ms, NONBLEND215.143ms다. 부모인 Map.Batch.Render177.542ms 안에 Draw118.535ms·Material27.977ms·Pass26.224ms가 포함되므로 이들을 부모에 다시 더하지 않는다. 가시성 계산7.747ms와 occlusion4.286ms는 별도지만 제출 경로보다 작다.

해당20frame에서 기존 exact 인스턴싱은 평균655.7개, lighting bank는1,917.6개의 source draw를 한 번에 제출하도록 줄이고 있다. 인스턴싱이 없는 상태는 아니다. 다만 그 이후에도 실제draw가 약9,637회 남고, 큰 구역을 내려다보는 이 구간에서 occlusion의 추가 제외는 평균14.5 source draws에 그친다. 건물 뒤에 확실히 가려진 후보가 적으면 가림 기법만으로 원경 제출량이 크게 줄지 않는다.

LOD는 실제로 선택되지만 coverage가 좁다. frame254에서 LOD 계측 대상2,691draw 중 low-LOD index가 준비된 draw는29개, 실제LOD1은9개·LOD2는3개였다. 이 counter는 SOURCE_BG·재질 admission·screenLod 입력이 있는 instanced 경로만 세므로 전체10,004draw의 분모로 혼용하지 않는다. 첨부 첫 원경 이미지는 해당 시점의 JSON이 없어 LOD0 여부를 이미지에서 확정하지 않는다. 두 번째 이미지의29.6FPS 표시와도 같은 작업량의 A/B 비교가 아니다.

`Map.Batch.Draw`는 CModel/CMesh의 bounds 확인·LOD 선택·IA 상태·DrawIndexedInstanced·Profiler 기록을 포함한다. 이를 드라이버 draw 함수 하나의 시간으로 부르지 않는다. actual Debug D3D device의 GetDesc10,000회는 warm median0.226ms였고, 현행 Profiler의9,000회 draw 계측 경로는3.897ms였다. 이 두 항목만 지워 수십~백ms 제출 비용을 해결할 수 있다는 근거는 없다. 근거는 `out/BernCutsceneAudit20261005/verification-receipt.json`이다. GPU timestamp 또한 CPU 명령 공급 공백을 포함할 수 있어 긴 GPU elapsed를 곧바로 GPU 연산 포화로 해석하지 않는다.

컷신 실행은 기존 camera cue의 pose/FOV 적용이며 별도 맵 actor 대량 생성 경로가 아니다. 저장 시점 projection으로 계산한 vertical FOV는 컷신60도, 줌아웃 이동약32.64도였다. 이것은 프레임별 camera 재생 증거는 아니지만, 줌 거리만으로 두 장면의 visible workload가 같다고 볼 수 없음을 보여준다. camera cue·렌더링 옵션·D3D debug layer를 임의로 변경하지 않았다.

fallback object1,222개도 전부 빠뜨린 불투명 인스턴싱 대상은 아니다. 실제 catalog의 source.*→SOURCE_CHARACTER 변환을 반영한 설치 inventory는 batch18,550/fallback1,222로 capture와 일치했다. fallback은 native carrier1,215개(혼합 모드 포함), profile-only6개, 기존 determinant 유효성 실패1개다. water/transparent 등의 표시 계약을 무시하고 이를 불투명 배치에 넣지 않는다. 상세 자료는 `out/BernCutsceneGrouping20261005/audit.json`이다.


## G25. 재질 호환 순서와 조명 bundle 기준 병합

MapPlacementRuntime의 기존 SOURCE_BG 지원 슬롯을 같은 geometry/model path·mirror 안에서 실제 CModel::Can_BatchStaticLightingWith와 동일한17개 render profile 값 기준으로 묶는다. 최초 등장 순서를 유지하며 비지원 슬롯은 원래 자리에 남긴다. 별도의 축약 재질 key를 만들어 세부 입력을 누락하지 않고 기존 Engine predicate를 소비한다. stage용 model clone과 bounds/static-instance 입력은 같은 asset에서 재사용한다. source placement ID·TRS·저장 데이터·다른 맵의 재질 계약은 유지한다.

Lighting bank는 앞선8개 batch 제한을 서로 다른 RNM texture bundle8개 제한으로 바꿨다. 각 model의 모든 submesh에서3개 RNM SRV를 비교한다. 같은 bundle을 쓰는 batch는 이미 할당된 slot을 사용한다. bundle1개는 일반 SOURCE_BG pass24~26과 원래224byte payload의 W를 유지하고,2~3개는 small-bank pass30~32,4~8개는 pass27~29를 사용한다. 원래 buffer·material·profile·mirror·LOD·시간·overflow 경계는 유지하며 exact-prefix handoff도 batch8개에서 끊지 않는다.

설치 전체 맵의 authored visible·유효 determinant·동일LOD 조건을 둔 정적 추정은 기존16,820draw → material cohort15,135 → 고유 bundle bank15,023이었다. 두 변경 합계1,797draw(10.68%) 후보 감소이며 실제 카메라·occlusion·GPU SRV identity를 재생한 runtime 측정이 아니다. 원래 순서에서 bundle 제한만 바꾸면58draw 감소에 그쳤다. 호환성의 레거시 제한을 무조건 풀어도 이 inventory에서 추가 감소0이므로 그런 완화는 하지 않았다. 근거는 `out/BernCutsceneGrouping20261005/audit.json`이다.

actual Stage block의 dependency fixture는 Debug/Release 각각16,601검사로 순서·stable ID·지원하지 않는 슬롯·모델 재사용을 확인했다. actual bank/Model11함수 fixture는 각각153,947검사로25개 같은 RNM batch의3meshdraw,40batch의 여러 slot,8/9bundle 경계, 모든 submesh 비교, original payload, prefix·overflow·buffer/Map 실패와 partial draw fallback을 확인했다. GPU upload/draw와 모델 predicate 경계는 대역인 부분을 영수증에 명시했다. 근거는 `stage-results.json`, `out/BernUniqueLightingBank20261005/verification-receipt.json`이다. 실제 Bern FPS 개선을 이 정적 추정이나 fixture 시간으로 확정하지 않는다.

## G26. 실제 LOD 대상 확대와 개인 파생 cache

생성 하한을8,192→4,096triangle로 낮추고 StaticMeshLod의 공유 상수를 생성기와 CMesh 준비 양쪽에서 사용한다. 원래19개 vertex attributes·UV 가중치100·border lock·오차0.001/0.002·목표0.55/0.30·최소 감소0.85와 화면오차0.25pixel은 유지한다. 원경이라는 이유만으로 고정된 낮은LOD를 강제로 선택하지 않는다. wind는 기존 주석과 달리 실제 admission에 빠진 부분이 있어 CModel 생성과 Client 현재 material 선택 양쪽에서 명시적으로 제외했다. alpha clip bit64를 wind bit로 혼동하지 않는다.

fresh material metadata에서 opaque SOURCE_BG·비wind 적격 후보는 고유mesh5→28, asset-mesh 변형43→189, authored visible occurrence51→224였다. 이 수는 생성 후보이며28개 모두가 실제 낮은LOD 생성에 성공하거나 현재 카메라에서 선택됐다는 의미가 아니다. 설치 표본에서 LANCE01A5,344tri는4,282/3,086, PILLAR078,032는5,858/4,776, BRIDGE8,104는6,384로 생성됐다. 작은652~1,698tri7개는 계속 대상 밖이고, 추가4,176tri표본은 감소 기준을 통과하지 않아 negative cache를 검증했다.

개인 LocalAppData/LostArk/StaticMeshLod/v2에 SHA256로 원본 transformed vertex76byte·index·알고리즘/상수 revision을 식별한 positive/negative 파생 cache를 둔다. 길이·header·checksum·index/range·원본 index prefix를 검증하고 실패하면 원래 생성기로 돌아간다. unique temp와 원자적 교체를 사용하며 Resources나 맵 원본을 변경하지 않는다. 새4,096하한에서 동일한3positive+1negative표본의 재계산은 Debug36.607/Release34.324ms, warm cache는4.271/4.351ms였다. cold write는45.068/44.198ms로 더 비쌌다. 기존8,192하한은 이 표본을 건너뛰었으므로 전체 로딩이 이전 버전보다 빨라졌다는 비교로 쓰지 않는다.

actual 생성·선택·준비·decoder 함수 fixture는 Debug/Release 각각32,418검사 PASS였다. WARP index buffer 원본/범위/near-far 선택,19채널 입력 변화,8가지 손상·동시4writer·I/O 실패 fallback을 확인했다. 실제2CPP 최소 컴파일도 양 빌드에서 통과했다. 근거는 `out/BernLodExpansion20261005/cache-validation.json`, `metadata-coverage.json`, `production-compile.json`이다. 화면 품질과 전체맵LOD 선택률은 다음 사용자 capture에서 확인한다.

## G27. 큰 atlas 청크의 새 컷신 후보 재평가

이전6개 export 카메라에서는 atlas density 후보가0이었지만 Bern 입장 sequence의16개 실제 authored key까지 확장하면 첫 key에 후보가 있다. 기존16texel/m 조건에서 source1,647개·cohort352개, 새 LOD4,096하한보다 작은 원본이라는 증명을 적용하면1,617개, 완전한 batch-mesh 멤버만 취하면1,560개·331cohort다. 저장3종의 export 카메라에서는 여전히0이었다.

기존64m owner·256source/64MiB geometry·1,024atlas·전체384MiB·build16/live8draw 기준을 유지하면 첫 key의 기존 grouping 최대6개 약351.42MiB에 절약 상한200draw, 면적 grouping 약370.56MiB에234draw였다. 고정6개 후보354.34MiB는16개 key 가운데6개에서220/187/87/62/26/64draw 상한이고 나머지10개 및3개 export는0이다. 실제 xatlas padding/packing·bake·시간별 가시성/LOD·GPU 표시를 완료한 값이 아니라 triangle 면적의 필요조건으로 계산한 상한이다.

따라서 넓은 카메라에서 후보 자체가 없다는 결론은 수정하지만, 약350MiB를 추가해 수백draw 상한을 얻는 후보를9,000~10,000draw 병목의 검증된 해법으로 활성화하지 않는다. atlas 기본OFF와 작은 청크 Stage 중단을 유지한다. broader material 호환 batching과 실제 제작한 HLOD 자산은 남은 과제다. 근거는 `out/BernCutscene20261005/proxy-new-policy-coverage.json`이다.

## G28. 재질 묶음·LOD 변경의 Debug/Release 제품 검증

정상 Product runner로 Engine→Shared→Server→Client 컴파일·링크·배포를 완료했다. Debug210,815ms는 `out/BuildPipeline/runs/20261004T235006219Z-debug-product.json`, Release185,502ms는 `20261004T235321754Z-release-product.json`에 PASS로 기록했다. Client는 각각230/221OBJ와2개 binary가 갱신됐다. Clean/Rebuild·tracking 삭제나 Client 실행은 하지 않았다.

92개 소스의 hash가 두 제품 빌드 전후 같았고 project/filter4개 XML parse와 git diff --check를 통과했다. 증거는 `out/BernCutscene20261005/product-source-freeze.json`, `product-validation.json`, `product-debug.log`, `product-release.log`다. 이후 사용자가 요청한 최적화 A/B Workbench는 별도의10-05 PLAN/RESULT에서 이어가며 그 후속 변경을 이 빌드가 검증했다고 표시하지 않는다. 큰 dirty worktree의 무관한 변경은 보존했고 일괄 stage·commit·push는 수행하지 않았다.
