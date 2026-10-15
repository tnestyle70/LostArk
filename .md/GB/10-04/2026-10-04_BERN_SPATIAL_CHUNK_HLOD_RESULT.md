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

## G29. 2026-10-11 저장 컷신과 네 차례 비평

사용자가 반복 최적화에도1600×900에서20FPS가 나온다고 보고하고 네 차례 비평을 거친
개선안8개를 요청했다.10월10일 `베른컷신_20261010_134406_565_frame1289_31168_0.json`,
`베른컷신_FHD_20261010_134512_291_frame2565_31168_1.json`을 다시 계산했고,
새 `베른컷신_20261011_213428_775_frame528_53220_0.json`의 진입 경계를 별도로 분석했다.
파일 위치는 모두 Client/Bin/ProfilerCaptures다. 조사 시점10월11일 파일은1개였다.

새 파일은 Release/RTX4050 Laptop/1600×900/D3D debug OFF,528frame이다.
CPU scope 누락0이며 마지막4frame만 GPU pending이다. 카메라·옵션은 export snapshot이고
frame별 cue time이 없으므로127frame의 급격한 제출 증가를 컷신 진입 경계로 분석했다.
frame N의 CPU와 대응하는 frame interval은N+1의 값을 사용했다.

| 프레임당 평균 | 직전101–125 | 진입127–146 | 진입127–226 |
|---|---:|---:|---:|
| CPU frame ms |10.123|56.300|49.957|
| 대응 다음 frame interval ms |10.284|56.980|50.527|
| interval 역수 FPS |97.24|17.55|19.79|
| draw |895|6,830|6,000|
| NONBLEND CPU ms |3.760|34.216|29.818|
| Map.Batch.Material ms |1.933|15.207|12.974|
| Map.Batch.Pass ms |0.443|3.466|2.998|
| Map.Batch.Draw ms |0.219|1.601|1.396|
| FinalCameraSubmission ms |1.245|10.592|9.135|
| NONBLEND VS invocation 백만 |1.184|7.269|6.310|
| NONBLEND PS invocation 백만 |8.031|8.742|8.759|
| 전체 PS invocation 백만 |23.126|18.913|18.668|
| distance 제외 instance |0|8,353|6,859|
| LOD1+ draw |0|330|250|
| LOD 계측 범위 index 감소 |0%|15.205%|14.187%|
| occlusion CPU ms |0.096|3.368|3.193|
| HLOD near/far draw |0/0|0/0|0/0|

중첩 CPU scope와 CPU/GPU 시간을 합산하지 않는다. 정지 시점의 distance 검사0은
visibility cache 재사용과 구분해야 한다. LOD 감소율의 분모는 해당 계측 경로뿐이다.
CPU scope는 driver/API 대기도 포함하고 PS 호출 수는 셰이더1회 비용을 나타내지 않는다.
따라서 CPU 제출 경로의 큰 비용은 확인됐지만 순수 CPU 연산 병목이나 GPU 무관을 확정하지 않는다.
10월10일 두 파일의 초반100frame CPU46.942/55.534ms, draw5,605/5,829는
DRAW_SUBMISSION_RESULT G22와 일치했다. 카메라 시각이 맞춰진 해상도A/B는 아니다.

네 차례 검토에서 수정한 판단은 다음과 같다.

1. **캡처로 원인 가설 비평:** 전체 PS는 감소했지만 NONBLEND PS는약9% 증가한다.
   같은 구간 VS는6.14배, draw는7.63배다. 픽셀 수만으로20FPS를 설명하는 가설을 버리고
   geometry와 재질·pass 제출, final-camera 준비 비용을 함께 대상으로 삼았다.
2. **실제 연결 범위 비평:** 거리 컬링과 개별 LOD는 실행됐다. radius>3m 구조물은
   거리 정책 밖이고 원거리 대체물은0이다. 스위치ON,IB축약 성공,큰 성 해결을 구분했다.
3. **과거 실패와 대안 비평:** 근거리 청크의 전체 geometry 증가,2member 생성/3draw 선택
   불일치,strict atlas 적격0,범용 RawValue cache의 비교 비용을 재검토했다.
   global bin/PVS/LOD bucket도 기존 기능 중복·추가draw·보수적 가시성 비용을 지적했다.
4. **활성화 경로 비평:** atlas는원본index를 유지하는 재질 병합이고 실제 geometry HLOD는
   Stage/Create_StaticCluster다. 두 경로를 동시에 켜면 claim을 덮는다. 기존 Stage를
   원거리 전용으로 연결하고 원본현재LOD보다엄격히작은index와최소3draw를 요구하도록 좁혔다.
   컴파일·native검사와 전체게임FPS·사용자화면 판정을 마지막까지 구분한다.

| 개선안 | 실제 변경 방향과 코드 연결 | 비평 후 남은 비용·완료 판정 |
|---|---|---|
|1. 큰 성의 geometry HLOD|MapPlacementRuntime→MapStaticChunkObject→CModel의 기존 경로에 성·탑의 원경 표현을 연결|atlas bool만 켜면 안 된다. 실제 대상·far 선택·원본 claim·총 제출 감소를 확인한다. 이번 G30은 기존 동일 재질 cluster의 원거리 연결이며 범용 skyline proxy 완성이 아니다|
|2. 재질 제출 준비 분리|MapStaticBatchObject의 매 mesh Bind_Material/Begin을 불변 material packet과 view/instance 상태로 분리|Shader.cpp의 작은 raw cache는 이미 있다. shared Effect 무효화·재질 revision을 보존하고 Material 실측 ms로 평가한다|
|3. 건축 세부의 거리 정책|radius 3m 기준으로 보호하는 큰 모델의 외곽과 문·창문·장식을 명시적 detail 단위로 구분|성 실루엣·지면까지 거리만으로 제거하지 않는다. stable asset/placement별 분류와 팝핑 검증이 필요하다|
|4. instance/submesh별 LOD bucket|현재 32m batch 전체 bounds를 공유하는 선택을 실제 원경 부품별로 좁힘|LOD별 분할은 draw를 늘릴 수 있다. geometry 절감이 추가 bind/pass 비용을 넘을 때만 분리한다|
|5. 가시 draw의 호환 묶음 확대|32m cell은 culling 단위로 유지하고 생존 mesh만 material/LOD/mirror/RNM별로 기존 제출 경로에 모음|geometry/material 정렬과 lighting bank가 이미 있다. 인접 중단 이유·추가 절감량을 먼저 집계하고 blend/overlay 순서를 보존한다|
|6. occlusion의 순이득 제한|Renderer의 후보 구성·정렬·raster·query 전체 비용과 제거 효과를 함께 제한|1.5ms는 raster callback 사이의 soft budget일 뿐이다. 이동 카메라에 과거 hidden 결과를 재사용하면 안 된다|
|7. 컷신 구간 후보·준비 캐시|기존 camera cue의 보수적인 구간별 PVS와 미리 준비한 자료를 기존 final-camera 경로에서 소비|전체 컷신 가시물 합집합을 매번 그리면 악화된다. 실제 frame의 frustum/LOD/거리 검사를 유지하고 자유 카메라는 원래 경로로 복귀한다|
|8. 원경 재질·depth 비용 절감|실제 비싼 GPU 재질 구간이 확인될 때 material LOD 또는 선별 depth 준비 적용|depth test/write는 이미 있다. 전체 depth prepass는 CPU draw를 늘리므로 이번 자료에서 우선순위가 낮다|

메모리14개 역사 표본의 가용 RAM은92.75–166.44MiB, system commit은96.67–96.77%였다.
Client private 약13GiB와 working set1.25–1.29GiB, VRAM2.55/예산5.03GiB는 서로 다른 지표다.
메모리 압박은 확인되지만 page fault·대기 사유·할당 소유자가 없어 컷신 실패의 단독 원인으로 확정하지 않는다.

## G30. 2026-10-11 원거리 geometry HLOD 연결과 Client 없는 CMD 검증

사용자가 HLOD를 켜고 Client EXE를 실행하지 않은 CMD 검증을 명시적으로 요청했다.
현재 Bern Load의 Stage_Proxy 호출을 실제 축약 geometry를 만드는 Stage로 교체했다.
기존 chunk shader/prototype/project 등록을 재사용한다. atlasEnabled는 별도 미완료
재질 atlas 후보의 설정이므로 false를 유지하며 두 Stage의 claim을 겹치지 않는다.

MapStaticChunkObject는 hasFarGeometry가 없는 생성 결과를 버린다. 선택 시에는
현재 카메라의 투영 오차1pixel을 만족한 far geometry만 허용한다. 최소3개로 보수적으로 계산한 원본
draw를 대체하고, 보이는 원본의 현재 개별 mesh LOD보다 index가 엄격히 작을 때만
claim을 켠다. 근거리 병합은 복구하지 않았다. near·HLOD OFF·생성/선택 거부·편집
무효화는 기존 원본 제출을 유지한다. 이미 claim한 뒤의 shader bind/Begin/draw 실패까지
원본 재제출을 보장하는 검사는 아니다. 새 C++ 파일과 project/filter 변경, 데이터 publish는 없다.

실행한 검증은 다음처럼 범위를 나눈다.

- 선택·소유권: 현재 제품 함수8개를 변경 없이 추출한 native Debug/Release 각각
  29시나리오·1,516assertion PASS. near→far→near, source/owner 조회 순서,
  최소 draw, strict index 감소, camera 경계, toggle, 편집 무효화와 owner 소멸을 검사했다.
  CModel·frustum·queue 입력은 대역이며 실제 Client 실행 검사가 아니다.
- 실제 자산·GPU: 기존 CMapAssetCatalog parser와 설치 Release Engine.dll을 사용했다.
  실제 게시 material override·RNM·원래 배치 TRS를 보존한 건축 표본25그룹/119모델을
  실제 CModel::Create_StaticCluster에 전달했다.25그룹 생성,22그룹 축약,5,591검사 PASS다.
  전체 표본의 원본804,144에서 near/far 혼합709,824indices로11.73% 줄었다.
  GPU near→far→near IB 선택·복원과 IAVertices/IAPrimitives 일치를 확인했다.
  단순 offscreen shader를 사용했으므로 제품 재질 화면 동치나 게임 FPS 검사가 아니다.
- 실제 컷신 카메라의 수치 평가:32개 고유 native 표본을 저작 첫 키,1600×900,FOV45에
  대조했다. 현재 distance/mesh LOD 뒤 최소3개 후보 draw를 가진22그룹 중2개가
  생성된 실제 오차와 strict index 감소를 함께 통과했다. Bern pillar02/06/07 그룹은
  28,536→27,330indices·0.825241pixel, pillar02/06 그룹은15,450→15,132·0.942295pixel이다.
  합계는43,986→42,462indices, 후보 draw6→2다. float64 visibility/LOD 재구성과
  path/mesh/LOD 호환 추정이며 실제 Client Stage 전체·SRV predicate·occlusion·grace를
  재생한 결과는 아니다. 따라서 모든 큰 성의 HLOD가 선택됐다고 확대하지 않는다.

재현 CMD는 저장소 루트에서 다음과 같다. 별도 창·swapchain·Present와 Client.exe는 없다.

```cmd
cmd /c out\BernHlodActivation20261011\selection\run-selection.cmd Release
cmd /c out\BernHlodActivation20261011\selection\run-selection.cmd Debug
cmd /c out\BernHlodActivation20261011\native\run-native-distance.cmd
```

증거는 같은 out 폴더의 selection/validation-result.json,
native/native-verification-receipt.json, native/cluster-gpu-distance-evidence.jsonl,
inventory/first-key-native-assessment-fov45.json에 있다. 첫17그룹의 독립 GPU 검사도
4,134assertion PASS,14그룹 축약이다. 이 두 표본은 겹치므로 합산한 전수 검사가 아니다.

정상 Product compile/link/deploy는 Release와 Debug 모두 PASS다.
Release 영수증은 out/BuildPipeline/runs/20261011T124741198Z-release-product.json이며
Client OBJ2·binary1·CSO0이 갱신됐다. Debug 영수증은
out/BuildPipeline/runs/20261011T125200854Z-debug-product.json이다. 이전 미커밋 Engine
header/CPP의 미반영 입력도 정상 증분 빌드가 소비해 Client OBJ217·binary2·CSO0을
갱신했으며 이를 이번 두 CPP 변경만의 컴파일 범위로 설명하지 않는다.
기존 C4819/C4828 및 외부 DirectXTK PDB 경고는 남아 있다.

실행 파일은 Client/Bin/Release/Client.exe와 Client/Bin/Debug/Client.exe다.
Client/UI를 실행하거나 촬영하지 않았다. Resources·저작/게시 JSON·렌더링 옵션은 변경하지
않았고 다른 세션의 미커밋 변경을 유지했다. 이번 연결로 HLOD 생성·원거리 선택 가능성을
확인했지만, 수천 draw의 큰 성 원경 전체나20FPS 문제의 해결은 아직 검증되지 않았다.
전체 Bern Stage의 실제 생성 수·추가 메모리·진입 로딩 시간도 Client 미실행으로
미측정이다. 기존384MiB 파생 GPU budget은 제한값이며 이번 실측 사용량이 아니다.
현 축약기의 강한 UV·source 경계 보존과 현재 개별 LOD보다 비싸면 제외하는 조건 때문에
기존 HLOD를 켜는 것만으로 큰 개선을 기대할 근거는 없다. 큰 성을 별도 원경 geometry와
적은 재질로 만드는 G29의1안, 반복 재질 제출을 줄이는2안이 여전히 남은 구조적 작업이다.

## G31. 베른 전체 LOD·거리 정책과 CPU/GPU 확장 범위 조사

사용자가 성 일부가 아니라 베른 전체의 LOD·Distance Culling과 NVIDIA GPU/CPU 확장
가능성을 다시 명확히 했다. 현재 구현은 모든 renderable이 같은 가시성 계약을 소비하는
구조가 아니다. 아래는 실제 설치 데이터와 현재 코드의 조사 결과이며, GPU culling이나
전체 장면 LOD를 이번 변경에서 구현했다는 기록이 아니다.

설치 맵의 placement는50,021개, 저작 visible은48,507개다. catalog16,743개에는 재질과
RNM 변형이 포함되며 WModel 경로는1,291개, 실제 placement 참조 경로는1,276개다.
고유(path,submesh)1,755개 중 현재 mesh LOD 재질 조건을 통과하는 것은840개,
512triangle 하한까지 통과하는 것은314개, 생성 성공은253개다. 생성 성공 가운데
저작 visible 배치에서 사용되는 것은249개다. 이 수는 맵 자산의 준비 coverage이며
컷신 특정 프레임의 선택률이나 전체 draw 감소율이 아니다.

| 객체·재질 범위 | 현재 연결 | 남은 공백 |
|---|---|---|
| 불투명 SOURCE_BG 정적 배치 | CPU frustum, 작은 prop distance, 일부 mesh LOD, G30 원거리 geometry HLOD | 큰 건물 외곽·세부와 전체 배치에 대한 원경 표현 coverage |
| foliage·grass·masked·wind | 기존 batch/frustum, 작은 객체 조건을 만족하면 distance | alpha/wind를 보존하는 LOD 및 원경 표현 |
| landscape | 기존 배치·frustum | 지형 경계 연결을 보존하는 전용 LOD |
| overlay·snow/ice·wet·vertexblend | 일부 기존 배치 | SOURCE_BG 한정 geometry LOD 밖의 재질별 처리 |
| 물·반투명·native SOURCE_CHARACTER 맵 객체 | 주로 MapAssetObject, frustum | 공통 distance/screen LOD 소비 경로 |
| Deploy | static intact의 frustum/shadow | 공통 거리·mesh LOD 정책 |
| 캐릭터·NPC·이펙트 | 각자의 기존 표시·simulation 경로 | 전체 장면의 표시 비용 정책에 등록하는 계약 |

family 사용량은 foliage14,336placement, grass12,326, opaque/masked21,038,
overlay3,105, landscape42다. 혼합 재질 객체가 중복되므로 합산하지 않는다.
50,021개 맵 배치와 별도 Deploy·캐릭터·이펙트까지 같은 분모로 혼용하지 않는다.

직접적인 공백은 MapStaticBatchObject.cpp의 StaticPropDistanceLimit에서radius>3m를
제외하고 Uses_MeshScreenLod가 SOURCE_BG/no wind/no alpha/no opacity texture로
제한되는 점이다. MapAssetObject::Should_CullCamera는frustum만 검사하며 실제 Render는
screen LOD를 받지 않는 CModel::Render다. DeployPropObject 역시 동일한 mesh LOD
경로에 연결되지 않는다. 큰 건물의 외곽을 보존하는 결정과 그 내부 세부까지 전부 원본으로
그리는 결정을 구분하는 데이터가 필요하다. radius 조건만 없애 성 전체를 숨기는 수정은
사용자가 요구한 전체 장면 LOD의 완성이 아니다.

원본 모델의 부재를 원인으로 단정할 근거는 없다. 설치 placement 참조 geometry 조사에서
geometry_errors.json은빈 배열이다. 다만 MODEL_MESH_DATA는mesh당vertices/indices
한 벌만 저장하고 MODEL_ASSET_DATA와WModel section에는선택 가능한 원본LOD chain이
없다. 따라서 현재 설치 런타임은 원본 UE LOD 체인을 보존·선택하는 계약이 없다.
원본 UE 패키지에도LOD가 없었다거나 모든 추출본이 최고LOD라는 뜻은 아니다.
필요한 자산은 기존 원본에서 만드는 재질별 축약LOD·원경proxy/impostor와 그 source
매핑이다. 현재 검사한 geometry HLOD는원본 경계·UV를강하게 보존해축약률이 작다.

CPU BVH/frustum/distance와 software occlusion은 이미 실행된다. 공통 CPU worker도
있지만 맵 가시성 준비 병렬화는 기존 G22에서 일관된 순이득이 입증되지 않아 기본OFF다.
이를 CPU culling 전체가 꺼졌다는 뜻으로 해석하지 않는다. CPU 확장은 기존 BVH와
stable placement ID를 유지하고 모든 표시 객체의 bounds·종류·LOD 정책을 연결하는
방향으로 가능하다. 판정은 비싼 material/pass 준비 이전에 제출을 줄여야 한다.

NVIDIA 장치의 native 검사는D3D11 feature level11_0에 성공했다. 표준 D3D11의
[compute shader](https://learn.microsoft.com/en-us/windows/win32/direct3d11/direct3d-11-advanced-stages-compute-shader)와
[DrawIndexedInstancedIndirect](https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11devicecontext-drawindexedinstancedindirect)를
사용할 수 있으므로 CUDA나DX12 전환이 선행 조건은 아니다. 현 제품에는GPU instance
compaction·indirect draw 호출·HiZ/HZB 연결이 없다. Shader_MeshLod.hlsl 파일은
프로젝트 None 항목으로 남아 있지만 호출 경로는 없고, Mesh.cpp는CPU LOD 선택 뒤
직접 DrawIndexedInstanced를 사용한다. shader 파일의 존재를 작동하는 GPU culling으로
계산하지 않는다. instance buffer도CPU dynamic vertex buffer이며 현재 depth target은
단일mip라서그대로HiZ가 되지 않는다.

G29의 네 차례 비평을 전체 베른 범위에 적용한 수정 방향8개는 다음과 같다.

| 방향 | 현재 소비자를 확장하는 방식 | 비평·검증 기준 |
|---|---|---|
|1. 전체 표시 객체의 가시성 계약 | 기존 Layer/Renderer에 stable ID·bounds·종류·LOD 가능 범위를 등록하고 동일한 최종 카메라를 사용 | 두 번째 모델 runtime을 만들지 않는다. fallback·Deploy·식생 등이 미등록인 채로 남지 않아야 한다 |
|2. CPU 거리 정책 확장 | 기존 BVH로 큰 후보를 먼저 제거하고 외곽·세부·지형·식생별 화면 크기/거리 정책 사용 | 큰 건물 외곽과 지면은 보존한다. 거리만으로 전체를 삭제하거나 display culling을 Server simulation 중단과 혼동하지 않는다 |
|3. 재질 family별 파생 LOD | 현재 CModel→CMaterial에 불투명·masked/wind·terrain별 적합한 축약 표현을 연결 | alpha coverage·wind bounds·terrain seam을 각각 검증한다. 같은 simplifier의 제외 조건을 무작정 풀지 않는다 |
|4. 원경 HLOD 제작·캐시 | 성만이 아니라 건물군·배경군을 적은 geometry와 재질로 표현하고 source stable ID로 대응 | 기존 Stage 활성화만으로 끝내지 않는다. 카메라에서 실제 선택되는 coverage·총draw·index·추가메모리·로딩을 확인한다 |
|5. 호환 제출 그룹과 재질 준비 | culling cell은 유지하되 살아남은 geometry/material/LOD/mirror/RNM 호환 객체를 묶고 불변 재질 준비를 재사용 | 기존 정렬·lighting bank·작은RawValue cache와 중복하지 않는다. transparent 순서와 shared Effect 상태를 보존한다 |
|6. 장면 단위 GPU instance culling | GPU 상주 bounds/transform으로 소수 compute dispatch에서 frustum/distance/LOD를 판정하고 생존ID를 모아 indirect 제출 | 매draw마다compute를하나씩실행하지 않는다. 같은프레임CPU readback 없이 소비하며 실제 CPU 제출 비용을 함께 측정한다 |
|7. GPU HiZ occlusion |6번의 생존 후보에 보수적인 depth hierarchy 가림 판정을 추가 | 현재 depth texture 존재만으로 완료가 아니다. camera cut·이동·disocclusion을 검증하고 가림이 적은 컷신에서 추가비용보다 이득이 있는지 확인한다 |
|8. 동일 컷신 기준의 완료 판정 | pose/FOV/viewport/시각과 설정을 맞춘 자료에서 범주별 원본→LOD/HLOD→제외→최종draw를 추적 | 현재 export camera는 과거 모든 프레임을 설명하지 않는다. 빌드/합성 검사를 게임FPS 상승으로 대신 기록하지 않는다 |

권장 연결 순서는 전체 객체 등록과CPU 후보 축소를 기반으로 원경 표현·재질 제출 수를
줄인 뒤GPU instance culling을연결하고, 추가이득이있는경우HiZ를붙이는 것이다.
D3D11에서는indirect를써도geometry/material별CPU command와state bind가남는다.
따라서GPU 컬링만켜면수천draw의Material/Pass 비용까지사라진다는기대는성립하지않는다.
원경에실제로보이는건물은culling으로없애는대신LOD/HLOD로싸게그려야한다.

전체 범위 자료는 out/BernHlodActivation20261011/inventory/full-bern-coverage.json에
현재 데이터 hash·분모와 이전 생성검사 재사용 범위를 구분해 보존한다.
G30 두CPP와PLAN/RESULT·gotchas·렌더링이펙트복원V2의scoped diff check는PASS다.
전체diff check에서나온KoukuSaydonPresentationAssetService.h, GameInstance.cpp/.h,
Sound_Manager.h의기존무관한공백오류는수정하지않았다. 제품JSON/XML은변경하지않았다.


## G32. 2026-10-11~12 베른 전체 정적 표시 경로 확장과 회귀 검토

사용자가 전체 적용 및 Debug/Release 반영을 승인했다. 아래는 10월 11일 저장본을 기준으로 구현한 범위다. 기존 G30/G31의 미구현 설명과 253/249 coverage는 그 조사 시점의 기록이며 현재 범위는 이 절을 따른다. Client/UI는 실행하지 않았다. 코드 반영과 무창 GPU 검증, 최종 제품 빌드, 실제 컷신 FPS/시각 검증을 구분한다.

저장된 진입 전/직후 비교에서 전체 CPU는 10.123→56.300ms, draw는 895→6,830회다. NONBLEND CPU는 3.760→34.216ms이며 그 안의 Material 설정은 1.933→15.207ms, Pass 적용은 0.443→3.466ms다. 별도 FinalCameraSubmission은 1.245→10.592ms다. 부모 NONBLEND와 자식 Material/Pass를 합산하지 않는다. NONBLEND VS는 1.184→7.269백만, NONBLEND PS는 8.031→8.742백만이며 전체 PS는 23.126→18.913백만이다. 가장 강하게 관측된 병목은 불투명 맵의 CPU 준비·재질 설정·반복 제출이며, 정점/geometry 증가도 함께 처리해야 한다. 호출 수만으로 GPU shader별 시간이나 순수 GPU 점유 병목을 확정하지 않는다.

### 8개 방향의 실제 반영과 경계

| 방향 | 반영한 코드·행동 | 보존하거나 아직 검증하지 않은 범위 |
|---|---|---|
|1. 공통 가시성 계약|MapAssetRenderUtils의 분류·거리·화면 LOD 입력을 정적 batch, Bern fallback MapAssetObject, intact static Deploy에 연결했다. Bern fallback의 안전한 정적 bounds는 기존 Layer BVH에 등록한다.|캐릭터·NPC·이펙트의 simulation이나 정렬을 정적 map 모델로 바꾸지 않는다. 기존 개별 표시 경로는 유지한다.|
|2. 거리 정책 확장|작은 소품의 기존 35/45/60m 정책과 화면 크기 조건을 공유하며 큰 정적 구조물은 120m 및 고정 2px 이하라는 두 조건을 사용한다. 초기 숨김 여유와 invalid-input 표시 보존을 둔다.|지형·물·반투명·변형 bounds가 미확정인 wind는 새 거리 제거에서 보존한다. 성 전체를 거리 하나로 삭제하지 않는다.|
|3. 재질별 LOD|CModel이 ordinary/instanced/indirect에 같은 현재 재질 admission과 원본 VB·파생 IB를 적용한다. 기존 v3에 별도 속성 보존 v4를 추가하고 fallback·static Deploy도 소비한다.|wind/source character/skinned/morph/forward 및 축약 실패는 원본이다. alpha 최종 화면 동등성을 기하 검사만으로 확정하지 않는다.|
|4. HLOD·파생 cache|source별 오차·bounds와 현재 가시 source subset IB를 사용한다. 각 source가 현재 mesh LOD보다 strict하게 작고 보수적인 원본 draw가 3개 이상일 때만 claim한다. SHA256 geometry cache와 64MiB decode cache를 추가했다.|384MiB 제품 GPU admission 상한을 유지한다. GPU 전역 병합 중인 cohort는 HLOD에서 제외해 중복 draw를 막고 GPU OFF에서는 복귀한다. 서로 다른 표면 재질의 새 atlas/impostor 제작은 포함하지 않는다.|
|5. 재질 제출 묶기|CMapGpuScene이 culling cell과 원본 placement를 유지하면서 같은 geometry·재질·mirror와 RNM 최대 8-bank를 전역 cohort로 묶는다. 대표 모델로 mesh당 재질/pass를 준비한다.|singleton, 비호환 재질, runtime transform 변경 cohort는 CPU 원본 경로다. transparent 순서는 바꾸지 않는다.|
|6. 장면 GPU culling|표준 D3D11 compute가 상주 source의 frustum·distance·LOD를 판정하고 instance payload를 압축해 기존 CModel의 DrawIndexedInstancedIndirect로 소비한다. 동일 프레임 readback은 없다.|실제 GPU survivor 수를 CPU 카운터에 추정 기입하지 않는다. 준비/dispatch 실패는 CPU 경로이며, GPU 제출을 확정한 뒤 material/draw 실패는 기존 HRESULT 실패로 보고한다.|
|7. GPU HiZ|현재 프레임의 실제 불투명 원본 geometry로 full-resolution depth와 max hierarchy를 만든다. 32 draw/80,000 triangles와 CPU soft budget 2ms, 장면 크기 조건을 둔다.|alpha/wind/morph와 축약 가능한 occluder는 제외한다. 이전 프레임 depth로 camera cut을 가리지 않는다. 실제 베른에서 HiZ 순이득은 미측정이다.|
|8. 같은 컷신 계측|F7에 GPU visibility/HiZ 세션 비교와 scope를 추가하고 JSON에 현재 스위치를 기록한다. 기존 MapBatch Material/Pass/Draw 고정 계측을 새 경로에도 유지한다.|CPU-only distance/LOD 수와 GPU indirect 명령의 분모를 구분한다. 다음 사용자 실행의 같은 컷신·FOV·1600×900 capture가 FPS 완료 판정이다.|

### 독립 비평과 수정한 회귀

1. 범위 비평: 재질 family를 일괄 허용하면 wind 경계, 지형 seam, forward 순서가 깨진다. wind를 새 distance/GPU 경로에서 보존하고 v4 border/속성 보존 및 caller의 실제 NONBLEND·opacity 조건을 추가했다. 다른 레벨의 fallback BVH는 변경하지 않는다.
2. 투영 수식 비평: GPU의 y축 error/깊이 근사는 off-axis와 z 오차를 과소평가했다. CPU와 같은 x/y·safe depth 상계와 부동소수 여유로 수정하고 near/화면 가장자리 GPU 회귀를 추가했다.
3. HLOD 선택 비평: 전체 cluster 오차와 전체 index만 비교하면 가까운 source까지 바꾸거나 현재 mesh LOD보다 비싸질 수 있다. source별 strict 비교·부분 선택·최소 draw 이득·실패 시 claim 미게시로 변경했다. 축약 없는 후보가 별도 far IB를 만들던 회귀도 전수 검사에서 고쳤다.
4. 제출 소유권 비평: 처음 보이는 RNM member가 bank 0이 아닐 때 model.front 계약 위반과 CPU/GPU 중복 제출 가능성을 발견했다. 항상 cohort 대표 모델을 바인딩하고, current GPU 결과를 소비하기로 확정한 뒤 실패를 member별 CPU 재시도로 바꾸지 않는다. 인접 CPU 병합은 현재 GPU-ready member를 소비하지 않는다.
5. 상태 변경 비용 비평: visible/suppression만 바꿔도 장면 전체 resident buffer를 재생성하던 경로를 active bit 갱신으로 줄였다. 실제 payload/transform 변경은 별도로 무효화하고, 계속 움직이는 cohort는 CPU로 둔다.
6. 계측 비평: 새 GPU 함수로 옮긴 뒤 기존 Material/Pass 계측이 사라지면 개선으로 오인한다. 같은 호출에 기존 고정 계측을 보존하고 GPU 추가 준비 비용은 별도 scope로 기록한다.

### 무창 native 검증

- 실제 CModel 23개 재질 표본: Debug/Release 각각 2,983 checks PASS, 선택·indices 행 동일. ordinary/instanced/fixed-indirect, 원본 복귀, invalid args, zero instances를 실제 NVIDIA D3D11 IA pipeline 값과 대조했다.
- 설치 모델 전체의 v3 생성: 9,493,301 checks PASS, 832개 크기 적합 mesh 중 666개 축약. v4: 4,208,455 checks PASS, 1,311개 크기 적합 mesh 중 97개 축약. 이는 서로 다른 profile 분모이므로 더해 적용률로 쓰지 않는다.
- 현재 재질 admission을 포함한 고유(path,submesh) LOD coverage는 321개, authored-visible은 315개다. wind 181개 고유 mesh는 원본을 유지한다. occurrence와 unique mesh를 혼용하지 않는다.
- 공통 distance/LOD 입력의 실제 함수 본문: 459 checks PASS. 큰 구조물 120m+2px, near plane, off-axis, hysteresis, shear/nonuniform scale, NaN과 실패 출력 보존을 검사했다.
- HLOD 실제 설치 후보 1,236개: 326,282 checks PASS, 949개 축약. 전체 후보 원본 12,437,742→10,507,194 indices는 전수 후보 geometry 비교이며 현재 화면 감소율이 아니다. Release/Debug 경계 26그룹 각각 7,692 checks PASS, cache 손상 재생성·warm reuse도 통과했다.
- HLOD Client 선택 함수 9개를 추출한 16개 시나리오/68 assertions PASS. 현재 mesh LOD strict 비교, partial member, GPU ON/OFF, IB 준비 실패·claim 수명을 검사했다. 실제 Layer/Client 실행과 구분한다.
- 현재 GPU module 실제 실행: 1,024 sources×224B, 5 LOD bins, active/frustum/distance/HiZ, near/off-axis, failed Stage rollback, actual depth triangle, actual indirect IA 소비 등 2,309 assertions PASS. D3D11 debug errors/warnings는 0/0이다.
- Client GPU lifecycle 실제 함수 추출 fixture: 36 assertions PASS. bank 1이 먼저 보이는 경우, 동일 프레임 bind 실패, CPU/GPU mixed readiness, 설정·active·stale frame을 확인했다. material backend는 대역이며 위 실제 GPU/CModel 검사와 별개다.

저장된 첫 키프레임을 FOV45/1600×900으로 재구성한 HLOD 후보는 16그룹/91 source다. 현재 mesh LOD 212,505→178,467 indices, draw 하한 54→16이다. 이것은 GPU 전역 cohort 제외·실제 occlusion·grace·현재 active 상태를 포함하지 않는 후보 평가다. 전체 원경의 실제 HLOD 활성 수 또는 FPS 개선으로 발표하지 않는다.

증거 정본은 `out/BernSceneOptimization20261011/lod-summary.json`, `hlod-summary.json`, `render/`의 native·lifecycle 기록이다. 각 기록은 입력/source/DLL hash와 검사 한계를 분리한다. 원본 Resources, authoring/published JSON, fog·FXAA·조명 등 저장된 렌더링 옵션은 이번 변경에서 바꾸지 않았다. 제품 빌드·배포의 최종 결과는 이어지는 절에 기록한다.

### Debug·Release 제품 반영 완료 (2026-10-12 KST)

`Tools/Build/Invoke-BuildAndRegression.ps1`의 Product 경로로 두 구성을 모두 완료했다. Client/UI는 실행하지 않았으며 Clean/Rebuild를 하지 않았다. Release는 컴파일 worker 2개를 사용했다. Debug는 같은 작업 폴더에서 진행 중인 다른 작업의 Product 빌드를 중단하지 않고 기다린 뒤, 그 빌드와 같은 worker 1개로 최종 증분 확인을 했다.

| 구성 | 최종 제품 receipt | 결과 |
|---|---|---|
| Release | `out/BuildPipeline/runs/20261011T151814499Z-release-product.json` | Engine·Shared·Server·Client PASS, 전체 486,177ms. Client OBJ 50개·CSO 3개·binary 2개 갱신. |
| Debug | `out/BuildPipeline/runs/20261011T152941498Z-debug-product.json` | Engine·Shared·Server·Client PASS. 직전 전체 Debug 제품 빌드 결과 위에서 최종 소스가 up-to-date임을 확인하고 배포를 다시 검증. |

직전 전체 Debug 빌드 receipt는 `out/BuildPipeline/runs/20261011T152928091Z-debug-product.json`이다. 이 빌드의 Client는 OBJ 276개·CSO 3개·binary 2개를 갱신했다. 최종 증분 빌드의 OBJ 갱신 0개를 미컴파일로 해석하지 않는다. 반대로 다른 작업의 광역 변경을 이번 베른 기능의 변경 수로 계산하지 않는다.

초기 Release Product 빌드에서는 신규 MapGpuScene의 `Engine_RenderTypes.h` 직접 include 누락이 드러났고 이를 추가한 뒤 최종 빌드를 통과했다. 초기 Engine Debug 단독 빌드의 OBJ 접근/D8040 실패도 재시도에서 해소했다. 원본 파일의 코드 페이지 경고 C4819/C4828과 외부 DirectXTK PDB 경고 LNK4099는 로그에 남아 있으며 warning 0을 주장하지 않는다. 이번 native GPU 검사의 D3D11 debug 오류/경고 0/0과 C++ 빌드 경고는 별개다.

최종 기능 소스 40개 SHA256가 동결한 manifest와 일치한다. Debug/Release 각각 Engine 산출물과 Client 실행 폴더의 `Engine.dll`, `Shader_GpuSceneVisibility.cso`, `Shader_GpuHiZCopy.cso`, `Shader_GpuHiZReduce.cso` 네 쌍의 SHA256 일치를 확인했다. Client EXE hash와 배포 쌍 전체 기록은 `out/BernSceneOptimization20261011/final-product-verification.json`에 저장했다. 제품 runner의 runtime 파일 존재·Navigation 참조·Item/Valtan reward 내용 검사도 통과했으며 데이터를 publish하지 않았다.

프로젝트/filters XML 4개와 BuildDomains JSON parse 및 이번 기능/문서의 scoped `git diff --check`는 PASS다. 전체 코드 PLAN에 복사된 줄 끝 공백은 PLAN에서만 정리했다. 기존 무관한 GameInstance.cpp/.h 등의 공백 변경은 보존했다. 실행 파일·DLL·CSO·EngineSDK와 out 검증 산출물은 소스 커밋 대상으로 추가하지 않았다.

완료 범위는 코드 연결, 독립 회귀 검토, native 검사, 두 구성의 제품 컴파일·배포다. 실제 베른 장면 전체의 GPU cohort 활성 범위, HiZ 순이익, 총 메모리/로딩 비용, 컷신 화면 및 FPS 개선폭은 아직 측정하지 않았다. 다음 사용자 실행에서 동일한 시작 pose·FOV·1600×900으로 저장한 캡처의 전체 CPU/프레임 시간, NONBLEND Material/Pass/Draw, FinalCameraSubmission과 GPU 추가 준비 시간을 함께 비교해야 성능 개선을 판정할 수 있다. CPU scope 시간에는 드라이버나 GPU 대기가 섞일 수 있으므로 이 기록만으로 순수 CPU 병목을 확정하지 않는다.

## G33. 2026-10-13 기본 카메라·컷신 회귀 검토

사용자는 베른 입장 컷신의 반복 수정 뒤에도 문제가 남고, 기본 카메라에서도 약40FPS로
낮아졌다고 보고했다. 이후 새 Profiler JSON이 저장되지 않은 사실을 확인했다. 이번 조사는
현재 소스와 수정 직전 사본을 대조한 코드 검토이며, 새40FPS 캡처를 분석한 결과가 아니다.
기본 카메라의 추가 제출 비용을 만드는 회귀와 컷신 원경의 남은 비용을 분리해 다룬다.

### 캡처 근거와 현재 측정 한계

`C:/Users/tnest/Desktop/LostArk/Client/Bin/ProfilerCaptures`는 조사 당시 비어 있었다.
Desktop·Downloads·Documents 및 다른 LostArk worktree를 확인했지만 이번 베른 캡처는
찾지 못했다. `CProfilerCaptureIO::Get_CaptureDirectory`는 실행 EXE의 상위 폴더에 있는
`ProfilerCaptures`를 사용한다. 파일명 입력은 이 폴더 안에서 저장할 이름을 정한다.
파일이 없는 원인을 이번 렌더링 수정으로 해결했다고 기록하지 않는다.

아래 수치는 이 문서 G29에 보존된10월11일 기록을 재인용했다. 이번에 원본 JSON을
다시 읽거나 통계를 재계산하지 않았다. 당시 조건은 Release·RTX4050 Laptop·1600×900·
D3D debug OFF다. 두 열은 같은 캡처 안의 서로 다른 카메라 구간이며 최적화 전후 A/B가 아니다.

| G29의 프레임당 평균 | 기본 카메라 직전101–125 | 컷신 초반127–146 |
|---|---:|---:|
| CPU frame |10.123ms|56.300ms|
| 대응 다음 frame interval |10.284ms|56.980ms|
| interval 역수 FPS |97.24|17.55|
| draw |895|6,830|
| NONBLEND CPU |3.760ms|34.216ms|
| Map.Batch.Material CPU |1.933ms|15.207ms|
| Map.Batch.Pass CPU |0.443ms|3.466ms|
| FinalCameraSubmission CPU |1.245ms|10.592ms|

현재40FPS의 평균·중앙값·p95, GPU 시간, 실제 옵션 및 카메라 조건은 확보하지 못했다.
따라서 과거97.24FPS와 사용자 보고40FPS의 차이를 이번 변경의 정량 회귀율로 계산하지
않는다. 아래 결함은 소스에서 확인했지만 각각이 실제40FPS에서 차지한 시간은 미측정이다.

### 실제 수정 파일과 함수

이번 변경 전 사본은 `out/BernRegression20261013/before`에 보존했다. 이 사본과 현재
파일의 `git diff --no-index`를 직접 대조했다. 저장소 전체에는 이전 작업의 미커밋 변경이
있으므로 HEAD 대비 전체 diff를 이번 수정량으로 계산하지 않는다. 아래 행 번호는 이 절
작성 시점의 현재 파일에서 확인했다. 새 C++ 파일·헤더·프로젝트 등록 변경은 없다.

| 실제 수정 파일 | 확인한 함수·행 | 변경 이유와 동작 |
|---|---|---|
| `Client/Private/MapStaticBatchObject.cpp` | `Submit_FinalCamera`,658행 | Layer가 보존한 그림자 전용 후보도 GPU NONBLEND로 등록하던 경로에 기존 보수적 batch sphere의 카메라 검사를 추가했다. 색상 후보만 `Mark_Submitted`하며 그림자 제출은 유지한다. 검사 실패는 표시를 보존하고 임시 frustum state로 CPU fallback의 grace를 진행하지 않는다. |
| `Client/Private/MapStaticBatchObject.cpp` | `Try_GetStaticOcclusionDesc`,401행 | 현재 GPU-ready cohort의 대표 callback을 개별 cell bounds로 CPU 가림에서 제거하지 않는다. CPU fallback batch는 기존 MOC descriptor와 인접 병합 경로를 사용한다. |
| `Client/Private/MapGpuScene.cpp` | `Prepare_Frame`,244행 | 현재 제출 cell이 없는 cohort는 재질·bank·mesh claim 검사를 건너뛴다. 화면 밖 대표 모델의 적격 검사는 실제 GPU cohort가 사용할 때 유지한다. 현재 개별 cell의 mesh draw 상한보다 indirect bin 수가 적지 않으면 CPU 가시성을 미리 준비하고 CPU로 돌린다. GPU 작업이 없으면 active upload·dispatch·HiZ를 생략한다. |
| `Client/Private/MapGpuScene.cpp` | `Is_Ready`,137행; `Try_Render`,314행 | 이번 프레임에 GPU를 선택한 `cohort.Active`를 확인한다. CPU fallback cohort를 GPU-ready로 간주하거나 `S_OK`로 소비해 원본을 누락하지 않고 `S_FALSE`로 기존 제출에 돌려준다. |
| `Engine/Private/Renderer.cpp` | `Prepare_GpuSceneVisibility`,1449행 | HiZ의128개 기준을 상주 `SourceInstances`에서 현재 제출된 `ActiveSources`로 바꾼다. 기존 bin 수8개·32draw·80,000triangle·2ms soft budget은 유지한다. |
| `Engine/Private/Renderer.cpp` | `Render_NonBlend`,1545행 | GPU 준비 성공 여부와 무관하게 CPU 잔여 대상의 기존 `Cull_StaticOcclusion`을 수행한다. GPU-ready callback은 객체 descriptor가 거절해 다른 cell까지 대표하는 GPU 제출을 잘못 제거하지 않는다. |

그림자 후보가 색상 제출에도 들어간 것은 Layer 자체의 오류가 아니다. Layer는 화면 밖의
그림자 caster를 의도적으로 callback에 남긴다. 이전 CPU 경로는 callback 내부에서 색상
가시성을 준비했지만 새 GPU 경로는 그 단계를 건너뛰어 화면 밖 batch의 재질·pass·indirect
준비까지 유발했다. GPU의 최종 frustum 제거가 픽셀을 숨겨도 앞선 CPU 작업을 취소하지는 못한다.

`Prepare_Frame`의 이전 전수 재질 비교·claim 조회는 최종 제출 여부를 검사하기 전에
수행됐다. 수정 뒤에도 resident member의 현재 제출 frame을 찾는 순회 자체는 남는다.
전수 순회와 active mask 초기화까지 제거했다고 기록하지 않는다. sparse cohort의 선택은
`submittedMembers × mesh 수`와 전체 indirect bin 수의 구조적 비교다. 기존 CPU 인접 병합,
instance culling 및 재질 준비 시간까지 모델링한 실제 성능 승리 조건은 아니다.

GPU는 모든 정적 batch를 소유하지 않는데 이전 Renderer는 GPU dispatch 하나만 성공해도
CPU MOC 전체를 생략했다. singleton·비호환 재질·CPU fallback이 가림을 잃는 경로를 이번에
복구했다. HiZ의 active 기준도 실제 최종 GPU survivor 수가 아니라 CPU가 제출한 유효
instance 수다. GPU frustum·distance·HiZ 제거 이후 개수를 측정한 값으로 해석하지 않는다.

### 조사만 수행한 연결 파일

다음 항목은 해당 함수 본문을 직접 읽었지만 이번 G33에서 수정하지 않았다. 기존 작업의
dirty 상태와 구분한다.

| 조사 파일 | 직접 읽은 위치 | 확인한 계약 |
|---|---|---|
| `Engine/Private/Layer.cpp` | `CLayer::Submit_FinalCamera`,441행 | `CameraIntersects || (shadowEnabled && ShadowCaster)`로 callback 후보를 보존하며 CPU 색상 준비 후보는 별도로 고른다. |
| `Client/Private/MapStaticChunkObject.cpp` | `Resolve_RenderSelection`,202행; `Resolve_ProxySelection`,277행 | `Is_GpuVisibilityActive`인 member를 HLOD/proxy 후보에서 제외한다. 현재 mesh LOD 대비 strict index 감소 및 최소 source draw 수 조건을 확인했다. |
| `Engine/Private/GpuSceneVisibility.cpp` | `Set_ActiveSources`,231행; `Prepare_Frame`,241행; `Dispatch`,258행 | active mask의 nonzero 개수를 `ActiveSources`에 기록하고 prepare callback 이후 통계를 소비한다. 같은 프레임 readback으로 빈 indirect bin을 찾는 경로는 없다. |
| `Client/Private/MapPlacementRuntime.cpp` | `Load_Area`,78행의 Bern stage·commit | 기존 batch로 HLOD를 준비한 뒤 GPU scene을 생성하고 같은 runtime에 commit한다. GPU OFF 복귀를 위해 chunk를 영구 삭제하지 않는다. |
| `Client/Private/Level_Bern.cpp` | `Initialize`,375행; `Update`,574행 | registry의 기존 `MapLoadScope`로 `Load_Area`하고 매 update에 공통 static batch frame을 진행한다. 컷신만 별도 맵 제출 runtime을 사용하지 않는다. |
| `Client/Private/ProfilerCaptureIO.cpp` | `Get_CaptureDirectory`,891행 | 실행 EXE 기준 캡처 저장 위치를 확인했다. 저장 실패의 실행 원인은 새 로그로 확인하지 못했다. |

### 남은 경계와 제품 빌드 상태

HLOD의 GPU 제외 조건은 아직 `Is_GpuVisibilityActive -> Can_Use`라는 cohort 적격 상태를
사용한다. 이번 프레임의 sparse CPU fallback 여부까지 반영하는 조건으로 바꾸지 않았다.
따라서 GPU 적격 cohort가 CPU fallback하더라도 HLOD 후보에서 빠질 수 있다. 이를 컷신
원경 HLOD 활성 문제까지 해결한 것으로 발표하지 않는다.

CPU MOC를 복구했으므로 같은 프레임에 CPU 잔여 대상의 MOC와 GPU cohort의 HiZ 비용이
함께 발생할 수 있다. 두 경로의 대상 소유권은 분리했지만 총 준비 시간이 항상 줄어든다는
보장은 없다. HiZ의 candidate 수집·정렬은2ms raster soft budget 밖에 있고 bin 기준은
여전히 resident 통계다. 기본 카메라와 컷신 각각에서 실제 추가 비용과 제거량을 측정해야 한다.

Release·Debug 정상 Product Build와 배포는 모두 PASS다. VS18 Insiders의 amd64
MSBuild18.11.0.42713, 기존 v143/MSVC14.44.35207, Windows SDK10.0.26100.0을
사용했다. Clean/Rebuild나 CSO tracking 우회는 하지 않았다. Client/UI·Server를
실행하지 않았으며 최종 확인 시 Client/Server 프로세스도 없었다.

| 구성 | 최종 제품 receipt | 확인 상태 |
|---|---|---|
| Release | `out/BuildPipeline/runs/20261013T102027662Z-release-product.json` | Engine·Shared·Server·Client PASS.132.226초, Client OBJ25·CSO0·binary2 갱신. |
| Debug | `out/BuildPipeline/runs/20261013T102043932Z-debug-product.json` | 네 프로젝트 PASS.3.817초, SkipBuild=false인 정상 증분 확인·배포. OBJ/PCH/CSO/binary 추가 갱신0. |

초기 Release는 `20261013T101648506Z-release-product.json`에 FAIL을 기록했다.
이번 베른 소스3개는 컴파일됐지만 별도 작업 중인 `BossLogicFlowView.h`의8행
`<string_vie` include로 Client가 실패했다. 해당 파일이 `<string_view>`로 교정된
최신 디스크 상태를 확인한 뒤 정상 Build를 다시 실행했다. 이 작업에서는 그 헤더를
수정하지 않았다. 초기 실행에서 Engine Renderer OBJ와 Client OBJ2 및 기존 Effect
셰이더 CSO1이 갱신됐다. 이 셰이더 변경을 이번 베른 수정으로 계산하지 않는다.

다른 작업의 선행 Debug Product(`20261013T101845630Z-debug-product.json`)가
현재 소스를 컴파일·링크한 뒤 이번 최종 Debug가 up-to-date와 배포를 확인했다.
선행 Debug는97.228초, Client OBJ29·binary2 갱신이다. 대량 기존 미커밋 변경을
포함하는 작업 폴더이므로 전체 OBJ 수를 이번 세 CPP 변경량으로 해석하지 않는다.
두 구성의 최종 receipt 모두 missingRuntimeInputs/invalidRuntimeInputs는 비어 있다.
기존 C4819/C4828과 외부 library PDB LNK4099 경고는 남아 있다.

최종3소스 SHA256는 검토한 candidate와 모두 일치한다. Debug/Release 각각 생산
Engine.dll과 Client 실행 폴더의 배포 Engine.dll SHA256가 일치하며 Client.exe도
존재한다. 증거는 `out/BernRegression20261013/source-hashes.json`,
`deployment-hashes.json`과 Release 최초/재시도·Debug 빌드 로그다.

실행 명령은 저장소 루트에서 다음과 같고 Debug도 Configuration만 바꿔 실행했다.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release -Profile Product -MSBuildPath 'C:\Program Files\Microsoft Visual Studio\18\Insiders\MSBuild\Current\Bin\amd64\MSBuild.exe' -MaxCompilerProcesses 2
```

이번 변경의 scoped `git diff --check`와 untracked MapGpuScene.cpp의 수정 전후
whitespace check는 PASS다. 전체 diff check는 기존 무관한12개 위치의 줄 끝 공백을
보고했으며 해당 파일을 정리하지 않았다. 기존 UTF-8 BOM 없음 인코딩을 유지했다.
제품 JSON/XML/HLSL/Resources와 저장 렌더링 옵션은 이번 작업에서 수정하지 않았다.
PLAN G34에는 실제3CPP 전체 코드를, gotchas/V2에는 재발 방지 원리를, CLAUDE에는
CPU/GPU 가림 소유권의 현재 계약만 갱신했다. 새 독립 하네스나 광역 진단은 추가하지
않았고 독립 코드 검토·제품 컴파일을 실제 화면/FPS 검증으로 대신하지 않는다.

다음 사용자 캡처는 기본 카메라와 입장 컷신을 별도 파일로 저장하고 동일한 구성·해상도·
카메라 조건을 비교한다. `Render.GpuScene.Prepare`, `Map.GpuScene.ActiveUpload`,
`Map.GpuScene.CpuFallback`, `Render.GpuScene.StaticDepthHiZ`, `Render.GpuScene.Dispatch`,
`Map.GpuScene.IndirectRender`와 기존 NONBLEND·Material·Pass·FinalCameraSubmission을
함께 확인한다. 최신 평균·p95 및 실제 FPS 회복은 그 캡처가 있어야 판정할 수 있다.


## G34. 2026-10-13 GPU 적격 배치의 native HLOD 연결과 비용 제한

GPU 적격 member의 native HLOD 무조건 제외를 제거하고, 같은 프레임의 원본 mesh와 HLOD
소유권을 연결했다. 기본 카메라 비용을 제한하는 사전 검사와 GPU 잔여 mesh의 제출을 함께 고쳤다.
이번 완료 범위는 코드 반영, 독립 검토, 실제 함수의 상태 검증, Release·Debug 제품 빌드·배포다.
사용자 새 profiler JSON은 없으며 40fps 회귀의 측정상 해소, 실제 Bern 컷신 HLOD 제출 수와
화면/FPS 개선은 아직 확인하지 않았다. 과거16그룹·91source 후보 수치를 이번 결과로 재사용하지 않는다.

### 확인한 원인과 실제 변경

`Resolve_RenderSelection`의 GPU 적격 제외 때문에 native HLOD buffer가 있어도 해당 member가
선택되지 않았다. 제외만 지우면 `Prepare_Frame`의 mesh claim 발견이 GPU scene 전체를 CPU로
돌리며, 기존 instance 공통 active bit는 submesh별 소유권을 표현하지 못한다. 또한 GPU cohort의
일부 member만 대체해도 원본 indirect draw가 남으므로 chunk 내부 원본 수를 절약으로 세면 안 된다.
이는 코드에서 확인한 연결 결함이며 보고된40fps의 단일 원인이라는 실측 결론은 아니다.

| 실제 수정 파일 | 함수·선언 | 적용한 책임 |
|---|---|---|
| `Client/Private/MapStaticChunkObject.cpp` | `Count_ReplacementDraws`, `Resolve_RenderSelection` | immutable source의1px·strict index 감소 및 draw 하한을 CPU VB 준비 전에 검사한다. GPU 적격도 후보에 포함하되 실제 visibility/LOD 탈락 후 다시 비용을 계산하고 IB 준비 성공 뒤에만 claim을 공개한다. |
| `Client/Public/MapStaticChunkObject.h` | `Count_ReplacementDraws` 선언 | 같은 owner가 후보 mask와 기존 representative scratch를 사용한다. 새 per-frame heap container를 만들지 않는다. |
| `Client/Private/MapGpuScene.cpp` | `Can_ReplaceHlodMesh`, `Get_HlodMeshIndexLimit`, `Build_Resident`, `Prepare_Frame`, `Try_Render` | cohort/mesh의 잠재 가시 peer를 자기 후보가 모두 덮을 때만 GPU draw1개를 절감 하한으로 센다. mesh별 active slot, 부분 claim, 빈 mesh 명령 생략, CPU fallback mask 정리를 연결한다. GPU per-instance LOD보다 HLOD가 무거워지지 않게 resident 최소 index와 비교한다. |
| `Client/Public/MapGpuScene.h` | HLOD 비용 조회, DRAW.IndexCount, COHORT.ActiveMeshes | CPU/HLOD와 GPU 제출을 조정하는 기존 owner의 계약을 확장한다. weak member와 고정 RNM bank 소유권은 유지한다. |
| `Client/Private/MapStaticBatchObject.cpp` | `Is_FinalCameraColorCandidate`, `Submit_FinalCamera`, `Render` | 색상 제출과 HLOD peer 비용 판단에 같은 보수적인 sphere/frustum을 사용한다. 전 mesh claim 때는 GPU cohort callback 이후 CPU VB 준비를 생략한다. 기존 그림자 제출을 유지한다. |
| `Client/Public/MapStaticBatchObject.h` | `Is_FinalCameraColorCandidate` 선언 | scene/chunk friend가 같은 색상 후보 기준을 소비한다. CPU frustum grace는 진전시키지 않는다. |
| `Engine/Public/GpuSceneVisibility.h` | STATISTICS.ActiveOwners, Set_ActiveSources optional owner count | mesh lane 수와 중복을 뺀 instance 수를 분리한다. shader/source ABI는 바꾸지 않는다. |
| `Engine/Private/GpuSceneVisibility.cpp` | `Set_ActiveSources` | owner 수를 upload 전에 검증하고 active lane 통계와 owner 통계를 함께 확정한다. 기존 단일 인자는 nonzero lane 수를 owner 기본값으로 사용한다. |
| `Engine/Private/Renderer.cpp` | `Prepare_GpuSceneVisibility` | HiZ의128개 진입 기준을 고유 ActiveOwners로 유지해 submesh 수 때문에 기본 카메라에서 가림 준비가 더 자주 켜지는 것을 막는다. |

### 선택·실패 경계

- 다른 chunk의 lazy claim을 비용 조회에서 호출하지 않는다. 자기 후보 밖 잠재 가시 peer가
  있으면 draw credit은0이며 모든 peer가 색상 후보 밖인 공집합도0이다.
- 최소3개 원본 draw를 대체할 때만 HLOD1draw를 추가한다. 현재 GPU-ready 여부가 아니라
  같은 카메라의 보수적인 색상 후보로 판단해 Layer 제출 순서에 의존하지 않는다.
- 후보가 준비돼도 실제 source가 현재 CPU LOD 및 모든 resident GPU LOD의 최소 index보다
  strict하게 작아야 한다. GPU OFF 또는 mesh LOD OFF는 해당 실제 원본 경로의 기준으로 돌아간다.
  이 하한은 보수적이므로 GPU의 현재 화면 LOD만 비교하는 방식보다 HLOD 적용 범위를 줄일 수 있다.
- HLOD 준비 실패는 원본을 남긴다. GPU mask/upload 실패는 stale frame을 사용하지 않고,
  기존 CPU 경로는 공개된 mesh claim을 건너뛴다. GPU draw를 소비한 뒤의 오류는 개별 CPU 재시도로 바꾸지 않는다.
- native HLOD에만 적용한다. 실험 atlas proxy의 기존 제외와 atlasEnabled=false는 유지한다.

### 읽기 검토한 연결 파일

`Client/Private/MapPlacementRuntime.cpp::Load_Area`의 chunk→GPU 준비 순서,
`Client/Private/Level_Bern.cpp`의 frame state 갱신,
`Engine/Private/Layer.cpp::Submit_FinalCamera/Prepare_FinalCameraCpuJobs`의 BVH·shadow 후보,
`Engine/Bin/ShaderFiles/Shader_GpuSceneVisibility.hlsl`의 ActiveSource 및 instance별 LOD 소비,
`Engine/Public/MeshLod.h`와 `Engine/Private/StaticMeshLod.*`의 화면 오차/LOD 범위를 대조했다.
이 파일들과 Bern 카메라·placement·저작 렌더링 JSON, Resources에는 이번 변경을 적용하지 않았다.

### 검증 증거

실제 후보10함수 본문을 변경 없이 추출한 임시 dependency fixture를 `out`에서만 실행했다.
Release 359 assertions, Debug 359 assertions PASS.
부분/전체 mesh claim, 다른 cohort GPU 유지, near 사전 탈락의 CPU visibility 호출0,
IB 준비 실패의 원본 보존, source-first lazy 선택1회, 후보 밖 peer/전부 화면 밖 peer의0 credit,
resident 최소 index equality 탈락, GPU upload 실패의 ready frame 초기화를 확인했다.
2~3instance×3mesh 혼합의45slot, 숨김·suppression, 고유 owner 분모, sparse fallback 구간 제거와
다음 프레임 복원도 검사했다. GPU backend·모델 호환·기하 오차/visibility 입력은 대역이며,
Build_Resident의 실제 GPU 할당이나 shader 실행, 설치 Bern asset의 coverage/FPS 검증은 아니다.
Stage의 최대8member 안에 들어가는7member 입력(2GPU cohort×3member+CPU geometry1개)에서도
draw credit3과 HLOD claim을 확인하고 두 GPU cohort의 미claim mesh가 그대로 제출 대상으로 남는지 검사했다.

증거는 `out/BernGpuHlod20261013/state-fixture/verification-result.json`,
구성별 `build-run-Release.log`, `build-run-Debug.log`, `provenance.json`이다.
독립 검토에서 발견한 all-peer-outside 공집합 credit 오류는 수정 후 같은 fixture에 포함했다.
9개 source는 candidate SHA와 일치하며 기존 UTF-8 BOM 없음과 파일별 줄바꿈을 보존했다.
각 파일의 기존 vcxproj/filters 등록은1개씩이고 네 프로젝트 XML parse를 통과했다.
9개 before/candidate의 `git diff --no-index --check`에 whitespace 오류가 없으며 요청 관련
source·PLAN·RESULT·정본 문서의 scoped `git diff --check`를 수행했다.
현재 작업으로 수정한 제품 JSON/XML은 없다. 다른 작업의 변경과 산출물은 commit/stage하지 않았다.

### 제품 빌드와 설치 결과

`Invoke-BuildAndRegression.ps1 -Profile Product -MaxCompilerProcesses 2`를 Release→Debug 순서로
실행했다. VS18 Insiders amd64 MSBuild를 명시했으며 Clean/Rebuild/SkipBuild는 사용하지 않았다.

| 구성 | 결과 | 전체 시간 | 갱신 object 수 | receipt |
|---|---|---|---|---|
| Release x64 Product | PASS | 325.243초 | Engine 2 OBJ, Shared 0 OBJ, Server 0 OBJ, Client 75 OBJ | [20261013T104818118Z-release-product.json](C:/Users/tnest/Desktop/LostArk/out/BuildPipeline/runs/20261013T104818118Z-release-product.json) |
| Debug x64 Product | PASS | 285.680초 | Engine 2 OBJ, Shared 0 OBJ, Server 0 OBJ, Client 83 OBJ | [20261013T105330853Z-debug-product.json](C:/Users/tnest/Desktop/LostArk/out/BuildPipeline/runs/20261013T105330853Z-debug-product.json) |

Engine→Shared→Server→Client compile/deploy와 Product runtime 검사를 통과했다.
기존 인코딩 C4819/C4828 및 third-party PDB LNK4099 경고는 남아 있다. 두 구성의 Client.exe와
배포된 Engine.dll 경로는 `Client/Bin/Release`, `Client/Bin/Debug`이며 Engine 원본/Client 배포
DLL hash를 대조했다. source/candidate hash와 배포 hash 증거는 같은 out 폴더에 보관한다.
Client·Server·UI는 자율 실행하지 않았다. 바이너리는 Git 전달/commit 대상에서 제외한다.

### 남은 실제 화면 확인

새 캡처로 동일 pose·FOV·해상도에서 기본 카메라와 입장 컷신을 각각 비교해야 한다.
`Map.Chunk.Select/Render`, `MapChunkFarDraws`, 원본 대비 제출 indices, GPU ActiveUpload/CPU fallback,
NONBLEND Material/Pass/Draw, FinalCameraSubmission 및 전체 frame interval을 함께 확인한다.
chunk 생성 수나 후보 수를 실제 HLOD draw 수로 대신하지 않는다. FPS 불변/회복과 컷신의 실제
HLOD 적용 범위는 사용자 실행·저장 파일이 확보된 뒤 판정한다.
현재32m 청크 밖에 같은 cohort의 잠재 가시 peer가 남으면 draw credit0이므로 모든 후보가
탈락할 가능성도 남는다. resident 최소 LOD 하한 역시 실제 화면 선택보다 엄격하다.
로컬 Geometry-v2 캐시777개에는 placement·bounds·cohort 식별자가 없어 저장된 Bern 카메라와
직접 연결할 수 없으며, 이 캐시 수를 현재 컷신의 nonzero HLOD 활성 증거로 사용하지 않았다.

## G35. 2026-10-13 사용자 확인과 거리·geometry 계측으로의 전환

후속 Release에서 기본 베른 카메라가 복구됐다고 사용자가 확인했다. 초·중반에 배경 성이 많이
그려질 때의 컷신 프레임 드랍은 여전히 남았고, 새 Profiler JSON은 저장되지 않았다고 답했다.
따라서 G34의 기본 카메라 미확인 경계는 사용자 확인으로 갱신하되, 컷신 성능 개선은 미확인이다.

현재 설치 자산을 CModel로 읽은 두 부분집합의 native HLOD 생성은 성공했다.
집8 source는5,727→4,665 indices, 계단6 source는3,819→3,417 indices였다.
집은 초기 세 keyframe에서8개가 frustum과 교차했지만1px·resident-min strict gate를 함께
통과한 source가0개였다. 계단은 초기1px를6개 모두 통과했으나 strict index 비교 통과는2개였다.
이것은 실제 자산에서 보수적 gate가 적용 범위를 제한하는 증거다. 전체 Stage grouping·GPU cohort·
현재 프레임 GPU LOD·게임 FPS를 재현한 결과는 아니다.
상세 입력과 한계는 `out/BernCutsceneHlod20261013/ACTUAL_SUBSET_RESULT.md`에 보존했다.

사용자가 이후 HLOD 조건 변경 대신 컷신의 일정 tick별 카메라 거리·변화·실제 draw/vertex/index
기록을 요청하여 선택 알고리즘 수정은 진행하지 않았다. 계측의 코드·검증·Release/Debug 빌드는
[컷신 geometry 결과](../10-13/2026-10-13_BERN_CUTSCENE_GEOMETRY_PROFILER_RESULT.md)에 둔다.

## G36. 2026-10-14 저장 캡처 확인과 실제 기하량 감소 기준

사용자가 저장한 두 JSON은 실제로 존재하며 전체 스트림 parse를 통과했다. 이번 목표는 후보 생성이나
draw 호출 병합이 아니라 동일 카메라의 실제 GPU 정점 처리량과 제출 index 감소다. 컷신의 목표는
기준 기하량의 1/8 이하이며, 기본 베른 카메라의 기존 동작과 성능은 별도로 보존해야 한다.

### 확인한 저장 파일과 계측 한계

- `Client/Bin/ProfilerCaptures/bern-distance_20261014_002751_254_frame88147_48928_0.json`:
  2,059,987,253 bytes, 100개 표본, 2,000,000개 source 행. `capture_budget_reached`로 종료했다.
  실제 cue 범위는 0.201457~5.107775초이므로 16초 컷신 전체를 저장한 것으로 해석하지 않는다.
  source cap으로 3,528,554행을 생략했지만 map draw 누락은 0개다. source 거리 목록의 일부 누락과
  실제 draw 집계의 완전성을 구분한다.
- `Client/Bin/ProfilerCaptures/profiler_20261014_003011_085_frame2906_48928_1.json`:
  207,784,243 bytes, 연속 264프레임. 거리 캡처와 별도 실행 시간축이므로 프레임 번호로 합치지 않는다.
- 거리 표본 수집 CPU 중앙값은 15.17925ms, p95는 30.7806ms이며 첫 표본은 236.8945ms다.
  상세 수집 비용이 포함된 FPS를 계측을 끈 정상 FPS로 대신하지 않는다.
- input 크기와 수정 시간은 parse 전후 동일했다. uint64 placement ID를 보존하기 위해 ijson의
  `use_float=False`를 사용했으며 입력 파일은 수정하지 않았다.

### 첫 거리 표본의 실제 제출

| 제출 경로 | map color index | draw 수 |
|---|---:|---:|
| CPU static batch | 11,015,559 | 5,394 |
| GPU indirect | 3,700,896 | 750 |
| CPU identical merge | 1,565,907 | 365 |
| Native HLOD | 364,080 | 51 |
| CPU lighting bank | 182,088 | 21 |
| 합계 | 16,828,530 | 6,581 |

LOD0가 13,688,961 indices, 즉 map color의 81.34%다. HLOD 출력은 2.16%지만 이 비율은
원본 대비 절감률이 아니다. GPU pipeline query의 VSInvocations는 8,952,634, Engine 전체
indices는 19,670,838이다. 전체 indices와 map color의 차이 2,842,308은 이 캡처에서 경로를
분류하지 못했으므로 그림자 등 특정 원인으로 단정하지 않는다. index 수, 정점 버퍼 용량,
VSInvocations, draw 호출 수는 서로 다른 측정값이다.

상위 10개 대표 asset의 index 합계는 map color의 5.48%다. 몇 개 성 모델만 바꾸면 전체 1/8이
된다는 근거가 없다. 병합/GPU/HLOD draw의 asset ID는 대표값이므로 모든 constituent 자산을
식별한 목록으로 해석하지 않는다.

### 기존 방법이 목표를 보장하지 못하는 코드 근거

`Model_StaticProxy.inl::Create_StaticProxy`는 atlas UV 재배치 뒤 source index 개수가 같은지
검사한다. 기하 단순화를 수행하지 않으므로 atlas 생성 자체는 정점 처리량 감소 증거가 아니다.
현재 `atlasEnabled=false`도 유지돼 있다.

`Model_StaticCluster.inl::Create_StaticCluster`는 source별로 단순화하고 경계·속성을 보존한다.
목표 ratio는 실제 결과의 보장이 아니다. 그 결과를 `MapStaticChunkObject::Resolve_RenderSelection`이
화면 오차, 실제 CPU LOD 또는 GPU resident 최소 LOD, cohort 전체 대체와 최소 draw 절감 조건으로
다시 거른다. 따라서 생성 성공, 선택 성공, 실제 원본 draw 제거, GPU 기하 감소는 각각 확인해야 한다.
이 근거로 과거 모든 시도의 원인을 하나로 확정한 것은 아니다.

### 실제 CModel GPU 반복 측정

`out/ProfilerInspection20261014/gpu-probe`에서 설치 CModel과 원본 placement/camera를 사용한
windowless D3D11 실험을 수행했다. 각 조건 50회, 원본/후보 쌍별 query가 모두 같은 값을 반환했다.
제품 소스·OBJ·DLL 21개 입력 hash는 전후 동일했다. Client나 게임 UI는 실행하지 않았다.

| 실제 입력/조건 | draw | index | VSInvocations |
|---|---:|---:|---:|
| CPU 실제 6사례, 현재 LOD 선택 | 원본과 동일 | 원본과 동일 | 원본과 동일 |
| 기둥 2-source 원본 | 2 | 49,632 | 14,712 |
| Native cluster ratio .35 / error .05m | 1 | 32,922 | 11,270 |
| Native cluster ratio .125 / error .05m | 1 | 32,892 | 11,261 |
| Native cluster ratio .125 / error .25m | 1 | 31,362 | 10,899 |
| Native cluster ratio .125 / error 1m | 1 | 30,984 | 10,834 |

모두 1/8 목표 실패다. CPU 4사례는 실제 출력 픽셀이 있고 원본/현재 LOD mask IoU가 1이다.
기둥 2사례와 cluster는 해당 카메라에서 출력 픽셀이 0이므로 외관 대체 품질을 판정할 수 없다.
2-source cluster는 제품의 최소 3개 원본 draw 조건도 통과하지 못하며 backend 실험으로만 분류한다.
position VS와 constant PS를 사용했으므로 제품 재질과 최종 FPS 검증을 대신하지 않는다.

첫 표본에서 실제 source ID가 알려진 CPU draw만 모은 재생 manifest는 1,105개 draw,
2,495,874 indices로 CPU indices의 19.55%다. 이 목록은 chunk에서 source 추적을 켠 배치에
편중되므로 일부 무출력 메시의 비율을 전체 맵으로 일반화하지 않는다.

분석 증거는 `out/ProfilerInspection20261014/distance-summary.json`, `raw-summary.json`,
`distance-draw-cost-summary.json`, `first-sample-replay-inputs.json`,
`gpu-probe/suite-summary.json`, `gpu-probe/verification.json`에 있다.
이번 확인만으로 제품의 기하량 감소 구현·전체 컷신 1/8 달성·FPS 개선·최종 화면 검증이 완료된
것은 아니다. 이 경계는 후속 실제 기하 후보와 전체 map 제출 비교 결과로 갱신한다.

### 실제 삼각형 감소 후보의 GPU 측정

기존 atlas/native cluster와 별도로, 원본 WModel의 실제 GPU VB를 읽어 원래 position/index와
바이트 단위로 대조한 뒤 Permissive/Sloppy simplifier가 만든 IB만 바꿔 측정했다. 원본 정점
속성을 유지하더라도 연결 삼각형이 달라지면 UV·normal 보간과 실루엣은 달라질 수 있다.
기존 WModel이나 제품 CModel 경로를 교체한 구현은 아니며 out 실험이다.

Permissive 1/16 후보에 vertex-cache 순서 최적화까지 적용한 결과는 다음과 같다.

| 실제 입력 | 원본→후보 index | 원본→후보 VSInvocations | 같은 카메라 실루엣 IoU |
|---|---:|---:|---:|
| pillar06 mesh3 | 24,816→1,551 | 7,356→889 | 출력 픽셀 0, 판정 불가 |
| house16 mesh0 | 6,072→378 | 1,641→187 | 0.97301 |
| house07 mesh4 | 7,026→438 | 2,127→204 | 0.69802 |
| tree 사례 | 19,530→1,218 | 8,961→868 | 0.45547 |
| house16 mesh1 | 4,320→267 | 1,288→134 | 0.69556 |

수치상 VS 1/8은 이 1/16 후보들에서 가능했지만, 여러 메시의 형상 손실이 커서 제품 적용
성공으로 판정하지 않았다. Draw는 모두 1→1이다. 목표 index를 1/8로 했을 때 Permissive
VS는 전부 1/8에 미달했다. index 감소율과 GPU 정점 처리 감소율을 같게 가정하면 안 된다.

유효 19개 후보는 각각 50회 paired query를 통과했다. Sloppy house16 mesh0의 1/8 후보는
퇴화 삼각형 1개가 생겨 실패로 남겼고 임의 삭제하거나 정상 후보로 제출하지 않았다.
cache 최적화 전후에는 삼각형 multiset·winding·정점 바이트·픽셀 mask가 동일했다.
25개 제품 입력 hash도 전후 동일했다.

이 측정은 Microsoft WARP의 실제 pipeline query다. RTX 4050에서 같은 VS 비율이 나온다는
보장은 아니며 제품 재질 shader·alpha·wind·최종 프레임·FPS를 검증한 결과도 아니다.
세부 비교는 `out/ProfilerInspection20261014/gpu-probe/candidate-comparison.json`, 재현과
280개 산출물 hash는 같은 폴더의 `candidate-verification.json`에 있다.

### 실제 전체 베른 맵의 동일 카메라 A/B

부분 메시 실험 다음에는 실제 Release OBJ의 `CLoader::Ready_MapArea`,
`CMapPlacementRuntime::Load_Area`, `CRenderer`와 설치된 shader/material을 사용하는
windowless offscreen 실행으로 범위를 넓혔다. 실제 RTX 4050 Laptop에서 1600×900으로
18,550개 batch, 48,799개 static placement, 1,222개 fallback을 로드했다. 제품의 모델
공유와 material variant 경로를 그대로 사용했다. Client와 게임 UI는 실행하지 않았다.

Level 소유 actor, sky, local map light, UI, physics, sound는 포함하지 않는다. 검은 하늘은
sky 생략 결과다. 캡처의 카메라 행렬은 같지만 과거 게임 프레임 전체를 재현한 것은 아니며,
새로 실행한 대조군/후보 사이에서만 동일 조건을 주장한다. 이 8프레임 비교에는 FPS 계측이 없다.

| 조건 | map draw | map index | 실제 VSInvocations |
|---|---:|---:|---:|
| 제품 LOD 원본 | 6,441 | 16,464,603 | 7,793,748 |
| 마지막 LOD 생성만 변경, 강제 선택 없음 | 6,380 | 17,000,559 | 7,962,820 |
| CPU 원거리 후보 강제 선택, 최소 view depth 20m | 6,442 | 14,103,081 | 6,878,803 |
| CPU 원거리 후보 강제 선택, 최소 view depth 50m | 6,454 | 14,325,087 | 6,960,368 |
| CPU 원거리 후보 강제 선택, 최소 view depth 80m | 6,480 | 15,166,191 | 7,269,765 |

최소 view depth는 bounds의 카메라 공간 Z이며 카메라와 중심 사이의 유클리드 거리가 아니다.
후보는 일반 메시의 마지막 두 LOD를 Permissive 1/8·1/16로 만들고 실제 반환 오차를 보존했다.
20/50/80m 선택 우회는 CPU 경로만 적용했고 GPU는 기존 shader의 오차 선택을 사용했다.
단, 사용 가능한 LOD와 HLOD 대체 승인 결과는 간접적으로 달라질 수 있다.

가장 강한 20m에서도 실제 VS 감소는 11.74%에 그쳤다. 50m는 10.69%, 80m는 6.72%다.
각 이미지에는 넓은 지붕과 첨탑의 구멍·파손이 있어 모두 기하량 1/8과 품질 기준에 실패했다.
화면 전체 occupancy IoU가 높더라도 국소 건축물 손실을 숨길 수 있어 성공 근거로 사용하지 않는다.
생성만 변경한 대조군은 오히려 VS가 증가했다. 따라서 후보 생성 코드 변경 자체를 기본 카메라
성능 보존으로 간주할 수도 없다. 제품 파일에는 이 후보를 반영하지 않았다.

출력은 `out/ProfilerInspection20261014/full-scene-probe/comparison.json`과 각 조건 폴더의
`baseline.png`, `summary.json`이다. 원본 Engine OBJ 메타데이터와 설치 CSO 260개 hash를
대조했고 원본 baseline DLL을 별도로 보존했다. out의 후보 DLL은 제품 배포 파일이 아니다.

### 텍스처 최상/하 비교에서 확인한 실제 기하량

제품 LOD OBJ로 돌아온 동일 DLL·EXE·카메라에서 실제 quality API의 `iTextureMinMip`만
0과 3으로 바꿨다. 저작 데이터나 개인 설정을 저장하지 않았고 모든 프레임의 dt는 0이다.
MinMip 0 이미지는 앞선 제품 baseline의 RGBA와 byte 단위로 같았다.

| 텍스처 설정 | map draw | map index | VS 중앙값 | Engine index |
|---|---:|---:|---:|---:|
| 최상, MinMip 0 | 6,441 | 16,464,603 | 7,793,748 | 17,573,211 |
| 하, MinMip 3 | 6,441 | 16,464,603 | 7,793,748 | 17,573,211 |

이 비교의 실제 정점·index 감소는 0%다. MinMip 3은 바닥·기둥의 텍스처를 흐리게 했지만
삼각형을 줄이지 않았다. `Shader.cpp`의 sampler `MinLOD` 변경은 VB/IB 단순화와 별개이며
상위 mip를 GPU 리소스에서 제거하는 구현도 아니다. 텍스처 처리 비용이 지배적이지 않으면
FPS 이득이 작을 수 있지만 이 비교에는 FPS 계측이 없어 성능 변화율을 주장하지 않는다.
상세 이미지 차이와 유효 query는 `full-scene-probe/texture-comparison.json`에 보존했다.

### 경계 보존·위치 변경 후보의 추가 실패 범위

지붕 파손을 단순히 열린 경계 붕괴로 단정하지 않도록 작은 두 house 사례에서 LockBorder와
`meshopt_simplifyWithUpdate`를 추가 비교했다. 신규 12개 후보 모두 원본/후보 50쌍의 실제
query가 반복 일치했고 비유한 위치·퇴화 삼각형은 0이었다. 제품 입력 25개 hash는 같았다.

LockBorder는 정확한 위치를 용접해 센 원본 open-boundary edge를 house07 572/572,
house16 632/632 보존했으나 house16 화면 IoU는 0.17794였다. 경계를 남기는 것만으로
내부 면이나 화면 실루엣을 보존하지 못한다. 모든 LockBorder 후보는 1/8 감축 목표에 실패했다.

WithUpdate 1/16의 house07은 438 indices / 204 VS / IoU 0.711992, house16은
378 indices / 187 VS / IoU 0.983079였다. house07의 손실은 여전히 크고 house16은
화면에 잘린 작은 부분집합이다. normal 길이 0.178~1.507, 일부 bbox 팽창도 남아
normal/tangent 후처리와 UV/lightmap 재투영 없이 제품 후보로 승인할 수 없다.
이 결과도 WARP의 단순 shader 실험이며 전체 RTX 맵 품질이나 FPS 성공을 대신하지 않는다.

증거는 `gpu-probe/shape-comparison.png`, `shape-candidate-comparison.json`,
`shape-candidate-verification.json`이다. 이번 조사에서 제품에 적용한 기하량 감소 수정은 없으며
Release/Debug 제품 빌드를 새로 수행한 것으로 기록하지 않는다.

### 저장 카메라 6개 × 품질/LOD/HLOD 16개 실제 이미지 비교

거리 캡처의 원래 view/projection을 그대로 추출해 0.201457, 1.001458, 1.993884,
3.007016, 4.010106, 5.107775초의 6개 카메라를 사용했다. 원본 캡처 SHA256·크기·mtime과
출력 행렬 round-trip을 확인했다. 5.108초 뒤의 미저장 구간이나 움직이는 전체 컷신 재현은 아니다.

각 시점에서 MinMip 0/1/2/3 × Mesh LOD ON/OFF × native HLOD ON/OFF를 실행해
96개 이미지를 생성했다. 앞뒤 대조 2개를 더한 총 98개 모두 GPU query/index가 유효하고
간접 draw 인수 누락·map draw 손실이 0이었다. 실제 적용 설정과 원본 RGBA를 보존했다.
Mesh LOD OFF는 CPU뿐 아니라 `Renderer -> GpuSceneVisibility -> Flags.y`를 통해
GPU indirect 선택도 LOD0로 고정한다. atlas는 모든 조건에서 OFF다.

제품맵은 한 번 로드했다. 시점마다 visibility 설정의 epoch를 갱신해 거리 판정 이력을
초기화하고 8회 원래 frame advance/update/render를 수행했다. dt=0에서도 frustum grace는
평가 프레임마다 만료한다. 그 뒤 16프레임 제출과 최종 GPU 완료를 wall time으로 측정했다.
상세 source 순회, GPU query 결과 복사, 이미지·JSON 저장은 측정 구간 밖이다. 마지막 프레임의
draw 기록 CPU 비용은 포함한다. animation 시간은 고정하며 Level actor/sky 등은 앞선 범위와 같다.

MinMip 0에서 같은 시점의 두 기능 모두 OFF와 ON을 비교한 실제 VS는 다음과 같다.
HLOD 추가 절감은 Mesh LOD ON을 유지한 채 HLOD만 ON으로 바꾼 비교다.

| cue 초 | LOD/HLOD 모두 OFF | 모두 ON | 두 기능의 VS 절감 | HLOD 추가 VS 절감 |
|---|---:|---:|---:|---:|
| 0.201 | 8,285,522 | 7,793,748 | 5.935% | 0.210% |
| 1.001 | 7,311,279 | 6,921,078 | 5.337% | 0.197% |
| 1.994 | 6,350,342 | 6,021,005 | 5.186% | 0.193% |
| 3.007 | 5,598,128 | 5,315,466 | 5.049% | 0.167% |
| 4.010 | 4,595,079 | 4,376,687 | 4.753% | 0.187% |
| 5.108 | 2,402,747 | 2,296,417 | 4.425% | 0.062% |

이번 실제 장면에서도 현재 LOD/HLOD는 1/8 목표에 크게 미달한다. 첫 시점 Engine index도
19,106,373에서 17,573,211로 줄었을 뿐이다. 설정의 존재나 정상 적용을 목표 달성과 혼동하지 않는다.
Mip만 바꾼 72쌍 중 64쌍은 index가 같고 8쌍은 작은 제출량 변화가 있었다. 최대 Engine index
차이는 약 0.266%이며 texture sampler 변경이 기하 단순화를 수행했다는 증거가 아니다.
이 작은 차이의 원인을 별도 분리 측정 없이 MOC·HiZ·alpha 중 하나로 단정하지 않는다.

FPS는 맵만 그리는 짧은 측정의 처리율이다. 앞뒤 대조 이미지의 RGBA hash와 모든 기하량이
정확히 같은데도 13.1626에서 17.4705 FPS로 32.73% 변했다. 따라서 Client FPS 또는
설정별 FPS 개선율·속도 순위의 근거로 사용하지 않는다. 이미지에도 이 한계를 명시했다.

각 이미지의 노란 띠에 FPS, 같은 이미지 프레임의 `VSInvocations`/Engine indices/map draw,
cue·설정·geometry frame을 표시했다. Vtx는 고유 VB 정점 수가 아니라 실제 VS 호출 수다.
VS/Engine indices는 offscreen 전체 렌더 패스, map draw/indices는 분류된 map color 범위다.
원본 1600×900을 가리지 않고 위쪽 140px에 표시했으며 합성은 전체 GPU 측정 종료 후 수행했다.

증거와 재실행 파일은 `out/ProfilerInspection20261014/full-scene-probe/` 아래에 있다.

- `batch-output/batch-results.json`, `validation.json`: 98조건 원시 측정·실제 설정·검증.
- `sweep-comparison.json`: 96조합 완전성, 같은 mip의 LOD/HLOD 효과, mip 단독 차이, 앞뒤 대조.
- `batch-output/images/camera-overview.png`: 현재 ON 설정의 6개 시점.
- `batch-output/images/contact-<cameraId>.png`: 시점별 16조건 비교, 총 6장.
- `batch-output/images/<caseId>.png`, `<caseId>-raw.png`: 표시 이미지와 원본 PNG 각각 98장.
- `batch-output/images/measurements.tsv`, `image-manifest.json`: 이미지와 수치·hash 연결.

원본 Engine OBJ 471개의 메타데이터는 바뀌지 않았다. 제품 소스와 저작 설정 변경 없이
수집·비교·표시를 완료한 것이며 기하량 감소 구현이나 새 Release/Debug 제품 빌드 완료는 아니다.

## G37. 2026-10-14 새 기하 재구성 후보와 전체 장면 예산

### 현재 완료 범위

기존 설정 비교를 반복하는 대신 원본 WModel의 모든 재질 mesh를 위치 기준으로 용접하고,
열린 지붕 면을 voxel shell로 재구성한 뒤 새 정점·삼각형·UV atlas를 생성했다.
원본 position/index를 유지하는 이전 LOD와 다른 실제 새 VB/IB다. 아래 수치는 out 전용
단일 HOUSE07 검증이며 제품 적용, 전체 베른 1/8 달성, 화면 최종 승인, Release/Debug
제품 빌드 완료를 의미하지 않는다. 작업은 계속 진행 중이다.

`out/BernGeometryProxy20261014/gpu-counts`의 windowless RTX 4050 검증은 실제 설치
`CModel` 5개 mesh 전체와 새 VB/IB를 같은 placement·저장 카메라 6개에서 비교했다.
각 조건 16쌍의 pipeline query가 동일했고 원본/후보 순서를 교대했다. 원본 native CModel의
index는 offline WModel parser와 완전히 같고 position 최대 오차는 0.0000002383m였다.
원본 WModel·연결 OBJ·고정 Engine DLL·후보 입력 hash는 실행 전후 같았다.

| 전체 HOUSE07 후보 | 실제 VS | 제출 index | draw | 원본 대비 VS |
|---|---:|---:|---:|---:|
| 원본 5개 mesh | 5,686 | 15,030 | 5 | 100% |
| 5cm shell, 1/8 index, atlas 773정점 | 1,452 | 1,878 | 1 | 25.54% |
| 같은 773정점, index cache 순서 정리 | 817 | 1,878 | 1 | 14.37% |
| 5cm shell closing3, atlas 458정점 | 1,021 | 1,404 | 1 | 17.96% |
| 같은 458정점, index cache 순서 정리 | 519 | 1,404 | 1 | 9.13% |

마지막 후보만 이 단일 모델의 VS와 index를 모두 1/8 이하로 줄였다. cache 정리는 삼각형
membership·winding·정점 데이터를 바꾸지 않고 draw 순서만 바꾼 것이다. 따라서 저장 VB
정점 수 또는 index 목표만으로 실제 VS 감소를 대신 판정하면 안 된다.

현재 비교 shader는 position-only, cull none, depth/alpha/원본 재질이 없는 기하 검증이다.
실제 GPU mask의 IoU는 보이는 세 시점에서 0.9004 / 0.9076 / 0.8917이고 나머지 세 시점은
이 placement가 화면 밖이다. 새 기하의 큰 실루엣은 남지만 최종 품질 합격으로 판정하지 않았다.
native 재질 bake·낮은 모델의 decode·전체 장면 검증을 이어서 수행해야 한다.

### 원본 재질을 함께 보존하는 데 필요한 조건

새 삼각형은 여러 원본 재질 경계를 가로지른다. 773정점 후보는 626개 삼각형 중 329개가
복수 원본 재질을 포함하므로 원본 UV나 재질 하나만 그대로 붙일 수 없다. 저해상도 atlas의
각 texel에서 원본 mesh/triangle/barycentric 좌표를 저장해 실제 native material로 다시
평가하는 경로를 `out/BernGeometryProxy20261014/native-bake`에서 검증 중이다.
재구성한 normal/tangent/binormal은 unit·직교 검증을 마쳤지만 원본 RNM tangent basis와
같지는 않으므로 원본 TBN을 별도로 보존해야 한다.

HOUSE07 mesh0의 기존 `Can_BakeStaticProxy` 탈락 원인은 실제 `diffuseSaturation=1.5`다.
wind, panning, subspecular, rim 때문이 아니었다. 기존 unsigned atlas는 채도 확대로 생길 수
있는 음수를 보존하지 못해 거절한다. 원본 재질값을 낮춰 통과시키지 않고 signed atlas 후보로
검증한다. 나머지 mesh1~4는 기존 admission을 통과했다. 이 관찰만으로 전체 bake 완료를
기록하지 않는다.

### BG만 줄이면 전체 1/8에 도달하지 못하는 근거

원래 첫 캡처의 실제 map color index 16,828,530을 설치 WModel/재질 정본과 결합했다.
대표 재질 분류 기준 foliage 3,282,699, grass 2,421,711, landscape 531,696으로 합계
6,236,106 indices, 37.06%다. 이 경로들을 모두 원본으로 보존하면 index 1/8 예산
2,103,566을 이미 넘는다. BG opaque/masked 전체도 45.82%뿐이다. merged/GPU draw의
대표 재질 귀속 한계는 `budget/cpu-budget-summary.json`에 기록했으며 이것을 VS 비용과
동일시하지 않는다.

grass 상위 물리 모델 세 개의 합은 실제 제출 index 2,223,738이다. 해당 diffuse의 많은
texel이 투명하고 모두 원본 wind 프로그램이 있어 불투명 voxel shell이나 이름 기반 삭제로
바꾸지 않는다. 원거리 alpha 표현과 바람 오차를 별도로 분석 중이다.

설치 placement 50,021개와 물리 WModel 1,276개를 분석했고 캡처의 서로 다른 placement
17,739개와 비교한 bounds 최대 오차는 중심 0.00003101m, 반경 0.00000353m였다.
원본 구의 가까운 표면까지 80m 이내인 확정 기하를 그대로 유지하면 최소 3,243,642 indices로
이미 1/8 예산을 넘는다. 너무 먼 배경만 대상으로 삼는 설계도 충분하지 않다. 미확정 GPU
survivor/혼재 배치 기여를 임의로 나눠 이 수치에 더하지 않았다.

### MapLoadScope와 임시 대체 상태의 경계

`MapLoadScope.h`의 `Contains`는 초기 placement 원점 X/Z의 로드 허용 범위이며
frustum culling 뒤에 낮은 모델을 선택하는 함수가 아니다. 현재 Bern descriptor는 전체 범위와
background 포함을 사용한다. 이 범위를 줄이는 것은 새 저폴리 배경을 만드는 작업이 아니며
일반 카메라가 필요로 하는 원본을 로드하지 않게 될 수 있다.

기존 `Set_InstanceSuppressed` / `Set_InstanceCameraPreviewSuppressed`는 native chunk의
`claim.valid`를 영구 무효화하고 해제 때 복원하지 않는다. 컷신 전용 대체는 이 API를 그대로
사용하지 않고 별도 가역 상태로 CPU/GPU 원본 draw와 depth 제공자를 함께 제어해야 한다.
실제 통합 지점과 실패 시 원본 보존 조건은 `runtime/runtime-audit.json`에 정리했다.
이 단락은 코드 조사 결과이며 해당 런타임 수정이 완료됐다는 뜻이 아니다.

## G38. 2026-10-14 성 기둥의 실제 GPU 감소와 기술소개서 산출물

### 성 기둥 두 종류의 새 기하 검증

PILLAR06/07은 열린 면을 두꺼운 voxel로 만드는 대신 원본 높이별 단면의 외곽을 연결한
닫힌 낮은 모델을 생성했다. 끝 첨탑의 높이를 보존하고 새 UV를 만들었다. 원본의 내부 면과
각인 형상은 새 기하에서 생략되므로 실제 재질 재투영과 화면 품질은 별도 검증 대상이다.
각인까지 동일한 geometry라고 설명하지 않는다.

`out/BernGeometryProxy20261014/gpu-counts/pillar06-axial16x40`과
`pillar07-axial16x36`에서 실제 설치 CModel의 전체 mesh를 비교했다. 원본 삼각형과 parser의
index는 완전히 같고 position 최대 오차는 각각 0.0000004659m / 0.0000006642m다.
저장된 동일 카메라 6개, 원본/후보 교대 16쌍에서 각 조건의 pipeline query가 동일했다.
입력 파일 hash도 검증 전후 같았다. shader는 position-only, cull none이며 depth/재질은
이 측정에 포함하지 않았다.

| 원본 물리 모델 | 원본 VS → 후보 VS | 원본 index → 후보 index | draw | VS 비율 | index 비율 |
|---|---:|---:|---:|---:|---:|
| PILLAR06 | 12,871 → 1,223 | 33,810 → 3,840 | 7 → 1 | 9.50% | 11.36% |
| PILLAR07 | 11,850 → 1,054 | 31,104 → 3,456 | 6 → 1 | 8.89% | 11.11% |

두 후보는 모두 이 단일 모델 기준 실제 VS와 index를 1/8 이하로 줄였다. PILLAR06은
삼각형 순서를 바꾸기 전 VS 1,621로 1/8을 약간 넘었고 cache 순서 정리 뒤 통과했다.
GPU 실루엣 IoU는 PILLAR06의 보이는 첫 구도에서 0.9966044, PILLAR07의 보이는
다섯 구도에서 0.9842143~0.9886871이다. 나머지 구도는 해당 placement가 화면 밖이며
그 구도의 빈 mask를 품질 일치 성공으로 세지 않았다.

### HOUSE07 원본 재질 평가와 남은 오차

native bake는 실제 `Bind_Material`과 source BG 평가 함수를 사용해 5개 원본 재질을
141,783개 atlas texel에 기록했다. 빠진 texel과 예상 밖 texel은 0, 출력은 모두 finite였다.
signed RGBA16F 6면과 원본 TBN/flags를 담는 추가 2면을 사용한 out 전용 검증이다.
실제 제품 GBuffer 형식·원본 cull·depth로 낮은 모델을 그려도 VS 519 / index 1,404를
유지했다. 제품 shader나 CModel에 이 경로를 설치했다는 뜻은 아니다.

4 mip을 사용하면 diffuse 평균 절대 오차가 약 0.019~0.023에서 0.0126~0.0153으로
줄었다. 반대 방향 source normal을 피하는 재투영은 normal 평균 오차를 추가로 줄였지만,
투영 거리가 늘고 일부 구도의 diffuse 오차가 커졌다. 실루엣 소실과 큰 normal 오차가
남아 있으므로 HOUSE07의 재질·화면 품질 통과로 판정하지 않는다. 원본 TBN의 oct
encoding에 대한 선형 보간/mip 오차도 source projection 오차와 나눠 조사한다.

### 사용자 지정 기술소개서 폴더

`기술소개서/베른컷신최적화`에 기존 카메라 비교 자료와 새 단일 모델 증거를 정리했다.
대표 합본은 다음 세 파일이다.

- `00_베른컷신_6시점_개선전_기준.png`: 기존 LOD/HLOD ON, mip0의 6구도.
- `02_새기하_단일모델_검증중/단일모델_기하감소와_실루엣_검증중.png`: HOUSE07.
- `02_새기하_단일모델_검증중/성기둥2종_GPU_기하감소_검증중.png`: PILLAR06/07.

기존 설정 96조건은 개선 전 조사 자료로 표시했다. `03_측정근거`에 측정 JSON을 복사했고
`자료안내.txt`에 범위·해석 제한, `산출물목록.json`에 상대 경로·크기·SHA-256을 저장했다.
원본 복사 hash를 확인했고 대표 합본을 직접 열어 글자·배치·기하 차이 표시를 확인했다.
현재 236개 파일이며 전체 베른 개선 후 이미지나 Client FPS 개선 증거로 표시하지 않았다.

이 단계도 out 전용 후보와 문서·이미지 정리다. 제품 런타임 적용, 전체 컷신 1/8,
일반 카메라 복귀 A/B, 새 Release/Debug 제품 빌드는 아직 완료하지 않았다.


## G39. 2026-10-14 임시 숨김 뒤 HLOD 복귀 결함 수정

### 재현과 제품 소스 변경

`CMapStaticBatchObject::Set_InstanceVisible`, `Set_InstanceSuppressed`,
`Set_InstanceCameraPreviewSuppressed`가 단순 표시 변경에도 `claim.valid=false`를 만들었다.
숨김을 해제해도 불변 HLOD geometry의 유효성이 돌아오지 않아 원본 제출이 늘었다.
실제 설치 CModel·MapPlacementRuntime의 베른 subset 133 placement/80 asset/8 chunk에서
확인했다. 이 subset은 실제 모델명 PILLAR06/PILLAR07/RANIAT_HOUSE07 일치 집합이며,
BernCastle과 Raniat의 같은 이름이 포함된다. 전체 베른이나 기본 카메라 화면 시험은 아니다.

| 동일 카메라 상태 | 유효 / 선택 chunk | GPU VS | Engine index | map draw |
|---|---:|---:|---:|---:|
| 시작 | 8 / 2 | 453,913 | 1,099,428 | 276 |
| 기존 코드 숨김 해제 | 0 / 0 | 454,543 | 1,101,336 | 284 |
| 수정 코드 숨김 해제 | 8 / 2 | 453,913 | 1,099,428 | 276 |

`MAP_CHUNK_CLAIM::Invalidate_FrameSelection`은 frameActive/activeMembers와 owner의
현재 선택 캐시만 지운다. 배치의 세 setter는 이 경로를 사용하고, `Late_Update`도 같은
초기화를 재사용한다. TRS/payload의 `Update_Instance`는 계속 불변 claim을 영구 무효화한다.
기존 visible CPU cache, shadow revision, 공간 bounds 처리와 final traversal 이전의 main
thread 호출 계약은 유지했다. 이미 제출된 GPU draw를 취소하는 API로 확대하지 않았다.

실제 수정 파일은 `Client/Public/MapStaticBatchObject.h`,
`Client/Private/MapStaticBatchObject.cpp`, `Client/Public/MapStaticChunkObject.h`,
`Client/Private/MapStaticChunkObject.cpp` 네 개다. 새 파일·project/filter 등록은 없다.
기존 인코딩과 줄바꿈을 보존했고 PLAN G39를 먼저 저장한 뒤 hash가 같은 소스에만 적용했다.

### 실행한 검증

동일 현재 소스에서 baseline/candidate 두 CPP를 각각 컴파일해 원래 제품 OBJ와 연결한
windowless RTX fixture를 실행했다. 새 Client/Server UI는 실행하지 않았다.
Release fixture 양쪽 컴파일·링크와 실행 exit 0, 변경 두 CPP의 Debug 최소 컴파일 exit 0을
확인했다. 복사된 기존 header의 C4819 경고가 있으며 제품 전체 Release/Debug 빌드 성공으로
기록하지 않는다. 최초 baseline runner의 ExitCode가 null로 관측되어 process handle을
보존한 runner로 다시 실행했고 두 번째 baseline은 exit 0을 확인했다.

`out/BernGeometryProxy20261014/budget/lifecycle-current-baseline-002`와
`lifecycle-current-candidate-001`을 비교한 `validation.json`은 64항목 모두 통과했다.

- 세 setter 각각 부분/전체 숨김 후 VS·index·map draw가 시작 값으로 정확히 복귀했다.
- 복귀 draw의 path/asset/mesh/LOD/index/instance/source ID 명령 집합도 같았다.
- 전체 숨김은 map draw/index 0, GPU active cohort/lane 0이었다.
- 부분 숨김은 기존 GPU active lane 2개 중 1개만 남겼다. 숨긴 원본 또는 HLOD claim 원본의
  활성 GPU lane은 모든 관측에서 0이었다.
- 잘못된 placement ID는 거부하고 시작 상태를 유지했다.
- 실제 TRS 변경 후 무효화된 claim은 숨김 해제만으로 복구되지 않았다.
- 모든 geometry query, indirect 결합, draw 목록이 유효하고 누락은 0이었다.

이 수정은 저폴리 대체를 위한 복귀 안전장치다. 새 저폴리 geometry/atlas를 제품 CModel에
연결한 것은 아니며 기존 40 FPS 문제 전체의 원인으로 이 작은 subset 차이를 단정하지 않는다.
전체 장면 1/8, 기본 카메라 실제 화면/성능 복귀, 최종 Release·Debug Product 빌드는 남아 있다.

## G40. 2026-10-14 추가 기하와 원본 재질·공유 atlas 검증

PILLAR02 `axial8x22-phase22p5`의 실제 GPU 비교는 원본 VS 3,931 / index 8,586에서
후보 VS 343 / index 1,056으로 줄었고 draw는 7→1이다. 삼각형 cache 순서 정리 전에는
VS 514로 1/8을 넘었으므로 CPU VB 개수만 보고 합격시키지 않았다. 16쌍의 수치가 같았고
입력 hash·원본 parser identity가 일치했다. 보이는 5개 구도의 GPU mask IoU는
0.9692308~0.9837464다. 이 측정은 position-only이며 재질·최종 색상은 포함하지 않는다.

PILLAR06/07은 원본 BG와 overlay family의 실제 material binder/평가 함수를 사용한
signed atlas와 기존 8 MRT 형식에서 동일한 VS/index 감소를 유지했다. overlay 원본은 실제
family별 pass를 사용했고, RT6/RT7 의미와 live RNM specular를 보존했다. 원본 TBN은
float3로 보간해 oct encoding의 접힘 오차를 분리했다. 보이는 구도의 수치는 다음과 같다.

| 후보 | diffuse RGB MAE | normal 평균 각도 오차 | GPU 실루엣 IoU |
|---|---:|---:|---:|
| PILLAR06 | 0.01480 | 11.91° | 0.99660 |
| PILLAR07 | 0.01440~0.01595 | 9.18~9.55° | 0.98421~0.98869 |

이 표는 GBuffer 비교이며 실제 deferred 최종 조명 합성 결과가 아니다. 원거리 외곽과
색상은 유지되지만 홈·재질 경계·normal의 오차가 남는다. 같은 HOUSE07의 oct→float3
대조에서 diffuse는 같고 normal 평균 오차가 일부 줄었으나 모든 투영 오차가 해결되지는 않았다.

공유 후보는 조명을 bake하지 않고 local 원본 TBN과 raw UV1을 저장한다. 기존 shader처럼
RNM과 static shadow 모두 UV1에 각 instance scale/bias를 적용한다. 실제 RNM·shadow page,
회전·scale이 다른 두 PILLAR06 배치가 한 번 생성한 10 atlas SRV와 같은 low VB/IB를
재사용하며 12개 paired view에서 VS 12,871→1,223 / index 33,810→3,840을 유지했다.
근거는 `native-bake/decode-pillar06-shared-material-512/shared-resource-proof.json`이다.
실험형식은 atlas 41,779,200 bytes/physical material identity로 크므로 제품 형식에는 별도
해상도·포맷·resident 예산이 필요하다. 공유 계약 성공과 가까운 구도의 품질 판정은 구분한다.

사용자 지정 `기술소개서/베른컷신최적화`에 PILLAR06/07 원본 재질 비교 PNG와 JSON,
PILLAR02 수치 근거를 추가했다. 현재 242개 파일이며 복사 hash와 manifest를 확인했다.
대표 재질 비교 이미지를 직접 열어 원본/새 geometry의 diffuse·normal·indirect 열을 확인했다.
자료안내에는 전체 게임 화면·제품 적용·FPS 개선으로 설명하지 않도록 범위를 기록했다.


## G41. 축소 geometry를 받는 CModel API 소스 반영과 실제 GPU 검증 — 2026-10-14

적용 파일은 `Engine/Public/Model.h`, `Engine/Private/Model_StaticProxy.inl` 두 개다.
동일 PLAN의 G41에 전문을 먼저 기록하고 before/candidate SHA를 확인하여 적용했다.
새 creator는 원본 mesh seam을 다시 생성하지 않고 축소 VB/IB·atlas UV를 기존 CModel 소유 자원으로 만든다.
source 범위는 stable placement ID 하나의 원본 mesh 전부이며, 전체 peer 검사·byte 예산·bounds 검사를 통과해야 한다.
기존 xatlas 생성과 CModel 객체 메모리 배치는 바꾸지 않았다.

실제 RTX 4050에서 새 API → source record shader binding → Prepare_StaticProxyVisibleSources →
Render_StaticProxy를 호출했다. raw GPU buffer만 직접 그린 앞선 proof와 구분한다.
16회 모두 PILLAR06 원본 VS 12,871 / index 33,810 → 새 CModel VS 1,223 / index 3,840으로 일치했다.
2-source full은 index 7,680 / VS 2,482, partial은 index 3,840 / VS 1,223, empty는 0이었다.
잘못된 선택은 기존 partial을 보존했다. 두 번째 source는 +8m 합성 TRS이며 실제 두 배치 화면 품질 증거가 아니다.
duplicate/missing mesh, NaN vertex/world, 잘못된 index, byte 예산, bounds 초과, zero/duplicate ID의
9가지 생성 실패에서 output과 stats가 모두 보존됐다.

- 근거: `out/product-engine-candidate/api-proof/api-proof.json`, `manifest.json`, `application.json`.
- Model.cpp 후보 Release/Debug 개별 컴파일 성공. 기존 C4267 한 건 외 신규 오류 없음.
- 제품 소스 두 파일과 PLAN의 `git diff --check` 통과.
- 전체 Product Release/Debug 빌드, 제품 컷신 활성, 전체 장면 1/8, 기본 카메라 화면·성능은 아직 미완료다.

### 7종 기하 검증과 재질의 별도 판정

HOUSE07·PILLAR06·PILLAR07·PILLAR02·BRIDGE02는 VS와 index 모두 1/8 이하이다.
CLIFF02는 VS 1,739→226 / index 5,400→672, PILLAR01은 VS 1,015→130 / index 2,544→288로
기하 감소는 확인했지만 VS가 엄격 1/8에 소폭 미달한다. 모두 같은 6개 카메라·16쌍 count가 일치했다.
이 7개 개별 모델의 합격은 전체 컷신 합격을 뜻하지 않는다.

compact8 atlas는 material identity당 256²/4mip/RGBA16F 8개, 5,570,560 bytes로 준비했다.
P06/P07을 실제 CRenderer 최종색으로 비교했을 때 지붕 하이라이트 차이가 남았다.
P06 RT4 간접광 RGB 평균 오차 0.0091에 비해 static-shadow alpha 평균 오차가 0.18365로 컸다.
packed baked bit·shadow channel·family는 일치했고, raw UV atlas의 mip/filter와 원본 shadow의
파생 texture footprint를 분리 진단 중이다. 이 단계는 최종 품질 통과나 제품 기본 활성로 기록하지 않는다.

Client의 batch/GPU mask/cutscene 소유권 후보 6개 파일은
`out/BernGeometryProxy20261014/product-batch-candidate`에 있으며 제품에 적용하지 않았다.
약속한 Baker API의 compile-only 선언으로 CPP 4개의 Debug 개별 컴파일까지 확인했다.
새 원본 색상 억제 flag는 그림자/피킹과 분리하고 기본 카메라에서는 선택 순회를 하지 않는다.
기존 MeshLodEnabled OFF는 이 대체도 해제하여 동일 카메라 A/B 기준을 유지한다.
실제 package API 연결과 통합 실행 검증은 후보 단계의 남은 작업이다.

### 이미지 산출물

`기술소개서/베른컷신최적화/02_새기하_단일모델_검증중/실제_GPU_기하감소_7종_검증중.png`에
개별 수치와 판정 범위를 표로 정리했다. `04_최종조명_품질검토중`에는 P06/P07의 남은 최종색 차이를
그대로 보존했다. 기준 화면, 개선 후보, 최종 품질 미완료 이미지를 서로 섞어 성공 증거로 표시하지 않는다.


## G42. 컷신 저폴리 선택과 native 재질 연결 소스 반영 — 2026-10-14

같은 PLAN G42에 전체 코드를 먼저 기록하고 11개 기존 source 파일의 before/candidate SHA를
대조하여 반영했다. 자료 설치 전에 소스를 컴파일하는 순서이며 이 기록 시점의
Client/Bin/Resources/Map/Derived/Bern/ReplacementIndex.json은 없다.
package가 없으면 기존 원본 경로를 유지한다. 제품 활성 성공으로 표시하지 않는다.

### 실제 반영한 책임

- Engine/Private/Model_StaticProxy.inl: 기존 proxy atlas 소유 상한을 9로 확장.
- Engine/Private/Material.cpp: BG/overlay 원본 파일 stamp 수집. Can_BakeStaticProxy 본문은 변경하지 않음.
- Client/Public/MapStaticProxyBaker.h, Client/Private/MapStaticProxyBaker.cpp: optional package index,
  실제 읽은 bytes SHA, CModel geometry/material identity, source file freshness, 기존 CModel staging과 live RNM binding.
- Client/Bin/ShaderFiles/Shader_VtxMeshMapProxy.hlsl: 기존 pass 0~3 보존, UV32/Jacobian32의 native pass 4~6 추가.
- Client/Public/MapStaticBatchObject.h, Client/Private/MapStaticBatchObject.cpp: 원본 색상만의 suppression,
  최종 카메라 선택, 원본 중복 제출 방지, 기존 callback의 low draw, late bind 실패 시 선택 원본 subset fallback.
- Client/Private/MapGpuScene.cpp: 모든 submesh active lane과 owner 계산에 같은 선택 적용.
- Client/Public/MapPlacementRuntime.h, Client/Private/MapPlacementRuntime.cpp: load staging과 컷신 선택 목록/복귀.
- Client/Private/Level_Bern.cpp: 기존 컷신 ownership/pose 성공 후 활성, 종료·실패에서 복귀.

새 C++ 파일, 프로젝트 등록, 원본 placement/MapLoadScope/저작 rendering option 변경은 없다.
원본 shadow/picking은 유지한다. 기본 카메라에서 low 선택의 instance 순회/할당/upload는 없다.
late-bind fallback은 현재 프레임 GPU/HLOD 소유권을 유지한 채 Selected slot의 전체 원본 mesh만 그린다.
다음 프레임부터 이 optional low 선택을 해제한다. 기본 shader/device 실패까지 복구했다고 주장하지 않는다.

### actual renderer native 품질 진전

이전 UV atlas를 색상처럼 축소/filter하면 서로 다른 원본 UV island 좌표가 섞이고,
원본 static shadow의 implicit texture footprint도 바뀌는 원인을 확인했다.
정확한 256px texel 중심 UV32와 원본 삼각형 UV Jacobian32로 affine map을 재구성하고
SampleGrad를 적용하자 P06 원거리 최종색 MAE 21.45→10.78/255,
shadow alpha MAE 0.18365→0.0399로 줄었다. P07 최종색 MAE 11.64→4.52,
shadow alpha 0.0192→0.00252로 줄었고 실제 보이는 5개 카메라의 RGB MAE는4.52~5.11이다.
완전 동일한 픽셀이라는 의미가 아니며 큰 근거리 P06 RNM variant는 품질 gate 밖으로 남겼다.

같은 CRenderer query의 P06 VS12895→1247/index33846→3876,
P07 VS11874→1078/index31140→3492는 조명/후처리 고정24VS/36index를 포함한다.
앞선 position-only 모델 수치와 분모를 구분한다. 전체 맵/캐릭터/그림자/게임 FPS 증거는 아니다.
04_최종조명_품질검토중에 원본 해상도 PNG, 공통 ROI 비교와 raw query를 추가했다.

### 선택·복귀 통합 검증의 현재 범위

실제 map 133 placements의 최소 fixture에서 기존 CModel 저폴리 API와 batch/CPU/GPU/HLOD를 실행했다.
position-only Baker shim, 증명용60m/2000px 조건에서 P06 19개 선택:
OFF VS348300/map index814386/map draw267 → ON VS215875/index521772/draw173.
원본 CPU visible/GPU active/HLOD source와의 중복0, OFF 복귀 및 prepare/preflight 실패의
원본 수치 정확 복원 등 4690검사 통과. 이는 제품 native package의 좁은 품질 gate나 전체 scene 결과가 아니다.
근거: out/BernGeometryProxy20261014/product-batch-proof/lifecycle-minimal-003/validation.json.

최초 out fixture는 이전 Loader/Level OBJ와 새 Runtime layout의 생성 COMDAT가 혼합되어 Clear에서
AV가 났다. old Loader/Level OBJ를 제거하고 실제 map 준비만의 최소 링크로 바꾼 뒤 exit0이었다.
이 실패 실행을 제품 런타임 성능·시각 결과로 기록하지 않는다.

### 남은 검증

source 11개와 PLAN의 git diff --check 통과. Engine2CPP Release/Debug, Client4CPP Debug,
Baker CPP와 shader 개별 컴파일은 통과했다. 20:14 KST Product Release 빌드를 시작했다.
실제 Baker package Prepare/9-SRV Render, 최신 late-bind fallback negative case,
제품 전체 Release/Debug 완료, 실제 설치/컷신 전체 수치와 기본 카메라 성능 검증은 진행 중이다.
전체1/8 목표는 미완료다.


### G42 후속 — 제품 LOD 대조, fallback LOD0, native package 설치 (2026-10-14)

초기 api-proof DLL이 예전 cpu-far-candidate/StaticMeshLod.native.obj를 계승했다.
추가 permissive LOD3가 있어 133개 배치 인스턴스 fixture의 초기 OFF 수치는 제품 baseline이 아니었다.
원본 WModel SHA, camera/LOD view bounds/투영계수/오차한계는 동일했으며 실제 제품
StaticMeshLod OBJ로 교체하자 이전 제품 baseline VS453913/index1099428과 정확히 같아졌다.
원본 LOD0과 공급 low geometry를 직접 그린 단일 모델 query 결과에는 이 LOD 선택을 사용하지 않았다.

최종 제품 LOD fixture: 133개 배치 인스턴스, P06 19개 증명용 선택, 넓은60m/2000px gate 및 position-only shim.
VS453913→281217→453913, map index1099392→698928→1099392, map draw276→180→276.
기존 remaining-visible bounds를 Selected fallback LOD에 재사용하던 경계를 고쳐
실패 한 프레임은 전체 원본 LOD0으로 그린다. 19×7=133 fallback draw의 원본 index가 모두 일치했다.
최종6869검사 PASS, native process handle로 exit0 확인. 동시 빌드·페이징의 CPU timing 잡음은
기본 카메라 FPS 회귀 없음의 확정 근거로 사용하지 않는다.
근거: out/BernGeometryProxy20261014/product-batch-proof/integration-result.json.

Native package 실제 로더 첫 거부는 Engine의 overlay provenance 확대 후 manifest에 빠진
원본 기본 diffuse/normal 두 DDS가 원인이었다. admission을 완화하지 않고 동일 source stamp로
의존 목록을 보완했다. 실제 Prepare/9-SRV/visible IB/Bind/native4·5·6pass/CRenderer를
3개 실제 배치×8조건=24쌍의 준비·렌더·기하량을 검증했다. 독립 native proof와의 최종 RGBA byte 일치는 3개 배치×6카메라=18쌍에서 확인했다. 나머지6쌍은 cull 조건이며 독립 이미지 비교 필드는 없다.
P06 가까운 큰 배치는 준비 재사용만 확인한 것이며 제품 distance/diameter gate 밖이다.
근거: out/BernGeometryProxy20261014/product-native-candidate/package-proof/validation.json.

새 Release Proxy.cso SHA12ef8138682887c21f6d679fbf3321bac334ec3c70791e567b23c26ae51390b3 및
original Instance.cso SHAfa64a095c0a62d2072b897f9193b5b0352bcbd727b631469320829195b23739e 확인.
자료27개/10717470bytes를 신규 derived 폴더에 검증·원자 설치하고 index를 마지막으로 공개했다.
원본 모델/재질 파일과 저작 렌더링 옵션은 변경하지 않았다.
설치 기록: out/BernGeometryProxy20261014/product-integration-applied/resource-installation.json.
독립 코드 리뷰에서 현재 지원 subset의 추가 확정 결함은 발견하지 못했다.

Release 전체 EXE 빌드는 아직 진행 중이며 Debug 재빌드, 실제 품질 gate 전체 맵 감소,
기본 카메라 성능 및 전체1/8 목표는 미완료다. 기술소개서의 현재상태.txt와 원문 검증 JSON을 갱신했다.

<!-- G42_PRODUCT_BUILD_FOLLOWUP_BEGIN -->
### G42 후속 — 최신 Product Release·Debug 빌드와 전달 자료 확인 (2026-10-14)

- Release: Product PASS, Client 완료 `2026-10-14T14:06:47.4409511Z` (UTC).
  영수증 `C:\Users\tnest\Desktop\LostArk\out\BuildPipeline\runs\20261014T140648402Z-release-product.json`; SHA-256 `af291a5089cb17a18fb3b92d43a6cd993556f69659e050d4f9b1dbbaa8972cc8`.
  EXE `C:\Users\tnest\Desktop\LostArk\Client\Bin\Release\Client.exe`, 19526144 bytes, 수정 UTC `2026-10-14T13:53:46.984231+00:00`, SHA-256 `f41e1db732d240efc9665dde1b9b86909b92a6b80868abce984a306da6ac3b00`.
  공식 SourceStable `C:\Users\tnest\Desktop\LostArk\out\BernGeometryProxy20261014\product-integration-applied\verified-release\source-stability.json`: buildExit=0, sourceStable=true, 전후 fingerprint `8fb07abf283151a405544b770ae167d15c20edf853eca9cef8a8f6c8fc97974e` 동일.
- Debug: Product PASS, Client 완료 `2026-10-14T14:07:08.3029643Z` (UTC).
  영수증 `C:\Users\tnest\Desktop\LostArk\out\BuildPipeline\runs\20261014T140709351Z-debug-product.json`; SHA-256 `79b201cc83d4e6ffb2d8cf8bed28b8e83b26122af695b248f458cde642a8862b`.
  EXE `C:\Users\tnest\Desktop\LostArk\Client\Bin\Debug\Client.exe`, 85805056 bytes, 수정 UTC `2026-10-14T14:05:04.963176+00:00`, SHA-256 `5b58a33cdf0c48b0391c448199ca1105406f347ced2c02f94e577a7980f6876c`.
  공식 SourceStable `C:\Users\tnest\Desktop\LostArk\out\BernGeometryProxy20261014\product-integration-applied\verified-debug\source-stability.json`: buildExit=0, sourceStable=true, 전후 fingerprint `c79fc03a42c5afcf137d53d06fb9d8b81a82fd977d106bdf03b9902cedb81004` 동일.

두 영수증은 skippedBuild=false이며 Engine·Shared·Server·Client 네 msbuild 단계와 runtime input 검증이 PASS다. 최신 G42 11개 소스(fallback LOD0 수정 포함) 및 G41 Model.h의 적용 SHA를 확인하고, 현재 EXE 시간·SHA와 배포 Engine.dll 일치를 기록했다. 빌드 PASS를 경고0으로 해석하지 않는다.

기하/복귀 fixture 6,869검사와 native package 24조건은 기존 검증 근거다. 독립 final RGBA byte 비교는 18카메라 쌍이며, 나머지6개 cull 조건에 같은 비교가 있다고 주장하지 않는다. 두 기술소개서 폴더는 기존 소유 manifest 항목만 갱신·대조하며 무관한 파일은 보존한다.

전체 베른 컷신의 동일 카메라 기하량1/8, 기본 카메라 게임 FPS, Client/UI 화면 검증은 아직 완료하지 않았다. 현재 빌드와 자료 전달의 성공을 이 항목들의 성공으로 대신하지 않는다.
최종 증거: `기술소개서/베른컷신최적화/03_측정근거/제품_Release_Debug_최종빌드검증.json`.
<!-- G42_PRODUCT_BUILD_FOLLOWUP_END -->

### G42 후속 — 빌드 이후 새 편집과 남은 실행 검증

위 Release·Debug PASS는 2026-10-14 23:07 KST까지 검증한 저장본의 결과다.
이후 사용자 작업인 `UISoundPlayer.cpp`·`.h`와 Client 프로젝트 등록이 추가로 저장됐다.
따라서 최신 작업 트리 전체의 빌드 완료를 뜻하지 않으며, 새 저장본으로 정식 Product 증분 빌드와
소스 안정성을 다시 확인한 뒤 전체 맵 GPU probe를 실행해야 한다. 기존 결과의 fingerprint를
바꿔 쓰거나 사용자 코드를 되돌리지 않는다. 변경 시점별 근거는
`../10-14/2026-10-14_KOUKU_FULL_FLOW_VERIFICATION_RESULT.md`의 G11에 기록했다.

전체 맵 probe는 앞선 검증 저장본으로 빌드·실행 준비까지 끝났지만 실제 GPU 비교는 미실행이다.
준비한 범위는 저장된 컷신 초반 0.201457~5.107775초의 6시점이며 전체 컷신 완주가 아니다.
메모리 부족으로 실행을 보류했으므로 새로운 전체 맵 VS·index 감소나 게임 FPS 결과는 없다.
기존 개별 모델·부분 배치의 실측 결과와 전체 1/8 목표 미완료 판정은 그대로 유지한다.

<!-- G43_CAPTURE_INVESTIGATION_20261015_BEGIN -->
## G43. 새 베른 캡처와 실제 저폴리 적용 범위 조사 — 2026-10-15

이 절은 새 캡처 분석·제품 코드 감사·진단 및 HiZ 수정 반영 결과다. 기존 G42 제품 반영 및
2026-10-14 빌드 결과와 구분한다. 아래 11개 파일을 제품에 적용하고 같은 파일 범위 안에서
`CounterNames` compile repair를 반영했다. 최신 Product Release003·Debug002는 모두 PASS이며
각 빌드의 입력 source fingerprint가 빌드 전후 동일하다. 새 제품의 native GPU 측정은 진행
중이다. 전체 장면 성능 개선이나 70m/1200px 후보의 제품 설치를 완료한 것으로 기록하지 않는다.
전체 베른 컷신의 실제 기하량 1/8 목표는 계속 미달성이다.

### 저장된 캡처와 관측 범위

사용자가 저장한 `Client/Bin/ProfilerCaptures/베른_컷신_20261015_011643_266_frame203_24544_0.json`은
59,140,428 bytes의 정상 JSON이다. SHA-256은
`eb30909341ac2c4b78e31427f028d60657ac5d26adea950d1ac98988bda720bf`이며 분석 전후
hash·크기·mtime이 같았다. `LostArkProfilerCapture.v3` 일반 Capture에 기록된 203개 프레임이
전부 저장됐고 excluded/evicted frame은 0이다. 기록 구간은 약 11.542초다.
이를 저장 실패나 사용자가 저장하지 않은 상태로 설명하면 안 된다.

다만 이 파일은 일반 프레임 캡처다. `bernCutsceneGeometry`, `mapDraws`, source/asset별 route,
프레임별 거리·컷신 cue가 없고 203개 `meshDraws` 배열도 모두 비어 있다.
따라서 P06/P07의 준비 성공, 실제 선택, 저폴리 draw 성공, 원본 fallback을 이 파일만으로
구분할 수 없다. 해당 필드 부재는 저폴리 OFF 또는 미제출의 직접 증거가 아니다.
metadata의 카메라·옵션은 저장 순간 한 번의 snapshot이며 203개 프레임 전체에 소급하지 않는다.

프레임 pipeline의 `vsInvocations`를 프레임마다 한 번만 읽은 결과는 첫 프레임 1,253,879,
중앙값 5,393,841, 최대 9,353,429, 마지막 2,082,816이다. 최대는 frame 75,
캡처 시작 후 약 1.786초다. 이 시점을 컷신 cue 1.786초라고 해석하지 않는다.
중첩 GPU scope의 VS를 더하지 않았으며 HUD에서 사용자가 본 약 80만과 같은 범위·시점으로
확정하지 않는다. 평균 프레임 간격 56.920ms의 역수는 약 17.57fps지만 동일 카메라의
변경 전 baseline이 없어 이번 수정의 회귀율이나 개선율은 계산할 수 없다.

기존 native HLOD는 624개 enabled, 131/203 프레임에서 Far draw, 최대 44 draw로 관측된다.
최대 VS 프레임에서 해당 HLOD 범위의 index는 334,509→294,267이다.
이는 기존 HLOD 동작 근거이며 G42의 별도 저폴리 package 적용 성공이나 전체 프레임의
같은 비율 감소를 뜻하지 않는다.

근거: `out/BernCapture20261015/latest-capture-analysis.json`, 같은 이름의 `.txt`.

### 제품에 설치된 실제 대체 범위와 거리·화면 크기 조건

설치 index는 `Client/Bin/Resources/Map/Derived/Bern/ReplacementIndex.json`이다.
현재 제품 package는 PILLAR06·PILLAR07 두 물리 모델뿐이다. P06 31개 asset ID/38개 authored
placement, P07 22개 asset ID/24개 authored placement로 총 53개 ID/62개 배치가 대응한다.
이 수는 index와 배치 데이터의 대응 수이며 현재 Level에서 전부 로드·표시·준비됐다는 뜻이 아니다.
앞선 7개 단일 모델의 기하 실험 성공을 7종 제품 적용으로 해석하면 안 된다.

`CMapStaticBatchObject::Prepare_ColorProxySelection`은 컷신 lease와 준비된 package 외에도
MeshLOD/source-material/final-view 등 기존 환경 조건을 확인한다. 실제 배치의 구 표면까지
거리 `max(0, length(center-camera)-radius)`가 최소 거리 이상이고 투영 구 지름 상한이
package 최대 픽셀 이하인 경우만 다음 frustum/preflight 단계로 넘긴다.
source geometry/material/shader hash·재질 정책·TRS 검증과 기존 CModel 준비도 별도 필요하다.
GPU 적격 배치라는 이유 하나로 일괄 배제하는 구조는 아니다.

저장 순간 카메라 `(148.550552, 48.154724, -79.587135)`, 1920×1080의 CPU 필요조건 계산은
다음과 같다. 프레임별 런타임 선택 결과가 아니라 해당 metadata 카메라 한 곳의 계산이다.

| Package | 최소 구 표면 거리 | 최대 투영 지름 | 대응 배치 | 거리 통과 | 픽셀 조건 통과 | 두 조건 동시 통과 |
|---|---:|---:|---:|---:|---:|---:|
| PILLAR06 | 121.6m | 202px | 38 | 0 | 0 | 0 |
| PILLAR07 | 82.6m | 553px | 24 | 10 | 0 | 0 |

P07의 배치 `9728128497115508879`는 구 표면 거리 114.395m로 거리 조건을 통과하지만
투영 지름 상한 794.495px로 553px 제한을 넘는다. 따라서 이 저장 시점에서는 거리만
완화해도 두 package가 적용되지 않는 문제를 해결할 수 없다. 픽셀 조건 계산에는 투영 깊이의
유효성도 포함되며, 숫자 0을 모든 물체가 화면 안에 있지만 크기만 초과했다는 의미로 쓰지 않는다.

이 계산에 사용한 기존 CPU bounds와 현재 BERN `mapassets`/`mapplacements` 48개 파일의 SHA를
대조했다. 17,739개 bounds 비교의 최대 중심 오차는 약 0.000031m이고 1mm 초과 불일치는 없었다.
행렬·카메라의 JSON 소수점 정밀도, 현재 Level 로드 범위, 실제 visible/suppression,
package 준비 실패, occlusion 및 cutscene ownership은 이 필요조건 계산의 범위 밖이다.
근거: `out/BernCapture20261015/export-camera-replacement-gates.json`.

반대로 저작 컷신 16개 key pose를 1920×1080/FOV45로 계산하면 시작부터 전부 탈락하지는 않는다.
구 frustum 추정과 두 조건을 함께 통과한 배치 수는 cue 0/664/1330/2145/2961/4035ms에서
각각 13/12/9/6/3/1개다. 5378/6352/7537ms는 0개, 8577ms는 2개,
9960/11407/12739/13946/14799/16000ms는 0개다.
이는 카메라 필요조건만의 후보 수이며 보이는 픽셀이나 실제 runtime draw 성공 수가 아니다.
따라서 저장 시점 0/62를 컷신 전체 미적용의 증거로 확대하지 않고 실제 준비·선택 계측이 필요하다.
근거: `out/BernCapture20261015/authored-key-gates.json`.

### 실제 native 진단으로 확정한 준비 거절 원인

새 제품의 공개 준비 진단을 P06/P07 지원 subset에서 읽었다. 준비 대상은 60개 batch이며
37개 준비 성공, 23개 거절, 실패 0개다. 62개 placement 중 준비된 것은 39개이고,
`unsupportedBatches=0`, `droppedFailures=0`, `indexError`·`stagingError`는 빈 값이다.
이 수치는 해당 subset의 로드 완료 결과이며 전체 베른 batch 수로 확대하지 않는다.

| native 최초 거절 reason | batch 수 | 확인된 경계 |
|---|---:|---|
| `replacement.batch_mirrored` | 13 | 미러 변환 batch의 현재 대체 경로 미지원 |
| `replacement.placement_transform` | 6 | 원본 placement가 현재 positive uniform TRS 계약을 충족하지 않음 |
| `replacement.static_material_identity_mismatch` | 4 | P06 `OVR_517CCBCAA767`의 재질 identity가 공유 package와 다름 |

23개 모두 HRESULT `S_FALSE`(1)로 원본을 유지한 거절이다. 앞선 offline 분석의 변환 조건
19개·재질 조건 4개가 실제 native 거절과 일치했다. OVR의
`bg_ber_berncastle_pillar07b_mi_rsh`는 설치 재질의 `specularIntensity=1.5`이고,
승인 package 대표 RNM 재질은 10.0이다. 이 값은 `ReplacementIdentity`가 해시에 포함하므로
같은 물리 모델이라는 이유만으로 해당 material package를 공유할 수 없다.

이 23개는 카메라 거리·픽셀 선택보다 앞선 준비 단계에서 거절됐다. 따라서 70m/1200px로
limits만 완화해도 준비되지 않으며 원본으로 남는다. 재질 SHA 검사를 우회하거나 원본
재질·변환을 바꾸지 않았다. authored 0초 ON의 선택 10개도 이 원자료에서 확인했지만,
준비·선택 수만으로 실제 draw 감소나 최종 10뷰 GPU 비교 성공을 판정하지 않는다.

원자료:
`out/BernCapture20261015/full-scene-probe/runtimes/diagnostic-subset-installed-001/sweep-cameras-results/authored-00s-on-selection.json`.
앞선 데이터 대조는 `out/BernCapture20261015/unprepared-placement-trs-audit.json`,
`out/BernCapture20261015/unprepared-ovr-material-audit.json`이며, 실제 최초 거절 사유의 정본은
위 native `preparation.failures`다.

### 기존 가림 경로에서 확인한 문제와 적용한 수정의 한계

수정 전 `Client/Private/MapStaticBatchObject.cpp`의 `Try_GetVisibilityDepthDesc`는
`Has_ColorProxySelection()`이 참이면 batch 전체를 HiZ 깊이 제공자에서 제외했다.
한 batch에서 일부 placement만 저폴리로 바뀌어도 남아 있는 원본 placement의 기존 가림자
역할까지 사라지는 코드 경계를 확인했다. 그 결과 실제 장면 비용이 얼마나 늘었는지는 아직
같은 카메라 전체 장면 OFF/ON 실행으로 측정하지 않았다.

적용한 수정은 위 전체 거절을 제거하고 descriptor 집계와 `Render_VisibilityDepth`의 실제
instance payload 선별 양쪽에서 `ColorProxySuppressed`만 제외한다. 선택된 저폴리 대신
원본 고기하로 깊이를 찍어 구멍을 메우지 않으며, 아직 원본으로 그리는 slot만 기존
opaque/material/meshLOD/HLOD 조건으로 깊이를 제공한다. 선택 0개는 기존 선별과 같고
전부 선택되면 accepted instance 0개로 깊이를 제공하지 않는다.
Renderer의 32 draw·80,000 triangle·2ms 예산과 기존 거리 hysteresis는 변경하지 않는다.

CPU `Try_GetStaticOcclusionDesc`의 low-selected batch 거절은 그대로 둔다. 이 callback은
cohort 또는 batch 전체 draw를 대표할 수 있어 일부 bounds만으로 제거하면 다른 원본·저폴리까지
사라질 수 있기 때문이다. 저폴리 `Render_ColorProxy`는 여전히 원본 GPU HiZ 생존 결과를
소비하지 않고 직접 draw한다. 선택된 원본 lane은 `MapGpuScene`에서 inactive이므로 해당 lane의
새 HiZ 생존 결과가 저폴리에 전달되는 구조도 아니다. ActiveOwners 수 변화로 HiZ 활성 임계치를
넘지 못할 가능성도 남는다. 별도 두 번째 렌더 경로를 추가해 이 경계를 우회하지 않았다.

이 때문에 P06/P07 subset의 준비·그리기·화질 검증과 전체 장면의 VS/index/GPU 비용 감소는
분리해야 한다. 단일 모델 저폴리화만으로, 기존에는 가려져 0개이던 원본의 비용까지 줄었다고
계산할 수 없다. 근거는 `MapStaticBatchObject`의 선택/깊이/render 함수,
`Client/Private/MapGpuScene.cpp`, `Engine/Private/Renderer.cpp`,
`Engine/Bin/ShaderFiles/Shader_GpuSceneVisibility.hlsl`의 실제 소비 순서다.

### 일반 F7 진단 적용과 빌드·실측의 구분

후보 위치는 `out/BernCapture20261015/diagnostics-candidate`다. manifest의 제품 before SHA와
후보 SHA를 대조했고 아래 기존 11개 파일의 전체 코드와 변경 내용을 G43 PLAN에 기록한 뒤
제품에 적용했다. 적용 당시 receipt는
`out/BernCapture20261015/source-applied-20261015-020547/application.json`이다.

- `Engine/Public/Profiler.h`: 기존 counter enum 끝에 32개 진단 항목 추가.
- `Client/Public/MapGeometryDiagnostics.h`: 로딩 때 확정한 준비 결과·실패 원인 보고서.
- `Client/Public/MapStaticProxyBaker.h`, `Client/Private/MapStaticProxyBaker.cpp`: index 부재/손상과
  asset 미지원을 구분하고 준비 거절 HRESULT·asset ID·이유 보존.
- `Client/Public/MapPlacementRuntime.h`, `Client/Private/MapPlacementRuntime.cpp`: Level 준비 보고서
  소유, lease/prepared/rejected/failed 등 현재 보유량 계측.
- `Client/Public/MapStaticBatchObject.h`, `Client/Private/MapStaticBatchObject.cpp`: 기존 선택 반복문에서
  조건별 탈락·candidate/selected를 기록하고 실제 draw 성공 뒤 low/fallback indices를 별도 기록.
  같은 batch의 재조회 중복과 동일 프레임 lease OFF→ON 합계 underflow를 방지하며 위 HiZ 최소 수정 포함.
- `Client/Public/ProfilerCaptureIO.h`, `Client/Private/ProfilerCaptureIO.cpp`,
  `Client/Private/ProfilerTool.cpp`: 일반 캡처 counter 이름·의미 및 metadata 준비 실패 보고서 저장.

준비 counter는 현재 보유량, candidate/selected는 선택 단계, submittedIndices는 실제 draw 성공
단계로 나눈다. 준비 index의 parse/hash 실패를 모든 asset 미지원으로 오인하지 않게 한다.
metadata는 저장 시점의 준비 상태이며 과거 모든 프레임의 상태를 대신하지 않는다.
기본 카메라에 추가 instance 순회·heap allocation·GPU readback을 넣지 않았다.
원본 suppression의 all-or-nothing 및 실패 fallback 계약은 유지한다.

`Client/Private/ProfilerCaptureIO.cpp`의 `CounterNames`는 CTAD로 initializer에서 실제 개수를
추론하고 enum `Count`와 `static_assert`로 대조하도록 compile repair했다. `Count` 크기로
고정한 배열의 빈 항목 기본 초기화 때문에 이름 누락을 놓치지 않게 한다. 별도 C++ 파일을
추가한 것이 아니라 위 11개 파일 중 하나를 보완했다. before/after SHA와 수정 이유는
`out/BernCapture20261015/counter-name-compile-repair.json`에 있다.

진단 수정 후 Product Release002가 먼저 PASS했고, 이후 다른 세션의 최신 사용자 리팩터를
보존한 상태에서 Product Release003·Debug002를 다시 실행해 모두 PASS했다. 중간 Debug001의
C1083은 `MainApp_ValtanAxe.cpp`가 삭제되는 동안 이전 프로젝트 목록이 그 파일을 컴파일한
것이었다. 현재 프로젝트 등록의 누락은 0개이며 최신 두 빌드는 성공했다. 이 중간 실패를
현재 빌드 실패로 남기거나 사용자 리팩터를 되돌리지 않았다.

| 최신 Product 빌드 | 공식 receipt | 게시 런타임 입력 검사 | missing/invalid 입력 | 설치 산출물 hash |
|---|---|---:|---:|---:|
| Release003 PASS | `out/BuildPipeline/runs/20261014T173957138Z-release-product.json` | 42/42 PASS | 0/0 | 270개 |
| Debug002 PASS | `out/BuildPipeline/runs/20261014T174038545Z-debug-product.json` | 42/42 PASS | 0/0 | 270개 |

두 receipt 모두 `skippedBuild=false`이며 실제 빌드한 결과다. 근거 디렉터리는
`out/BernGeometryProxy20261014/product-integration-applied/` 아래
`bern-geometry-final-20261015-release-003`, `bern-geometry-final-20261015-debug-002`다.
각각의 `source-stability.json`은 `buildExit=0`, `sourceStable=true`이고
`binary-verification.json`은 설치된 산출물 270개의 hash를 보존한다.
Release003의 빌드 전후 source SHA는
`055f532de64a1f16e0bd50901bce35cc9e2aa1e268e7201aa449d5d5cc0944c5`,
Debug002는 `cb0dc098b28403af9b2a09864c7a640852834d672cc4cfd53971867ccb4daf68`로
각 실행 안에서 동일했다. 이 42개 검사는 게시 런타임 입력 검증이며 Client 화면이나 GPU 성능
측정을 대신하지 않는다.

수정한 제품의 새 native GPU 측정은 아래 62-placement subset 10뷰에서 완료했다.
일반 캡처 직렬화 실행·전체 맵 OFF/ON/OFF 수치·기본 카메라 성능·사용자 화면 검증은 완료로
기록하지 않는다. 70m/1200px는 아래 비교 당시 out 후보이며 제품 설치 미완료다.
보조 compile 스크립트는 준비만 했으며 공식 제품 빌드와 별도로 실행하지 않았다.
단일 모델 성공·앞선 G42 빌드 성공을 이번 수정의 실행 검증 성공으로 대체하지 않는다.
`candidate.diff`, `depth-occluder.diff`, `manifest.json`, `review-report.json`은 코드 검토 근거다.

비교 이미지에서 제기했던 탑 소실 판단은 철회했다. 해당 탑 영역의 OFF/ON/OFF raw 픽셀은
동일했고 각 PNG도 대응 raw와 일치했다. alpha 또는 IA cache 결함을 확정하거나 수정하지 않았다.

이번 조사·진단 반영으로 원본 모델/재질, 저작 placement/로드 범위, 팀장 렌더링 옵션,
베른 기본 fog OFF 정본을 변경하지 않았다. 거리·픽셀 품질 gate도 제품에서 임의 완화하지 않았다.
후속 GPU 실측과 후보 반영 결과는 실행 후 이 절에 구분해 추가한다.
<!-- G43_CAPTURE_INVESTIGATION_20261015_END -->

<!-- G43_NATIVE_SUBSET_MEASUREMENT_20261015_BEGIN -->
### 최신 Release003의 실제 GPU 기하량: 지원 성탑 subset 10뷰

최신 Release003과 같은 제품 소스·OBJ를 소비한 native probe로 P06/P07 62 placement,
60 batch를 1920×1080에서 측정했다. 제품 CLoader/CModel/Renderer 경로를 사용하지만
다른 베른 맵 오브젝트·Level actor·sky·로컬 map light·UI·sound·physics는 포함하지 않는다.
따라서 이 결과는 사용자가 본 전체 화면 약 80만 Vtx의 전후 비교가 아니며 게임 FPS도 아니다.
Vtx(VS)는 최종 측정 프레임의 GPU VS invocation, Idx는 실제 draw에 제출한 index 참조 수다.
timestamp는 별도 16프레임 모두 유효함을 확인했고 기하량을 16프레임 평균으로 표시하지 않는다.

| 저작 cue | OFF Vtx(VS) | 설치 설정 ON | 70m/1200px 후보 ON | 후보 Idx | 후보의 OFF 대비 Vtx 감소 |
|---|---:|---:|---:|---:|---:|
| 0초 | 445,695 | 366,201 | 242,225 | 620,544 | 45.65% |
| 2초 | 332,568 | 277,131 | 188,564 | 489,624 | 43.30% |
| 4초 | 307,676 | 299,657 | 186,274 | 481,740 | 39.46% |
| 6초 | 127,010 | 127,010 | 99,144 | 258,198 | 21.94% |
| 8초 | 101,225 | 101,225 | 101,225 | 264,756 | 0% |
| 10초 | 280,486 | 280,486 | 263,505 | 671,394 | 6.05% |
| 12초 | 210,652 | 210,652 | 210,652 | 553,086 | 0% |
| 14초 | 214,234 | 214,234 | 197,253 | 515,922 | 7.93% |
| 15.999초 | 98,908 | 98,908 | 98,908 | 259,692 | 0% |
| F7 export 카메라 | 141,795 | 141,795 | 106,820 | 273,804 | 24.67% |

저작 cue는 재생 속도 0.5의 게임 경과 시간과 다르다. 16초에는 컷신 lease가 끝나므로
마지막 측정은 15.999초이며, export 카메라의 cue는 알 수 없어 특정 컷신 시간으로 붙이지 않는다.
0초의 Idx는 OFF 1,081,890 → 설치 ON 904,962 → 후보 ON 620,544다. 후보의 설치 ON 대비
추가 감소는 Vtx 33.85%, Idx 31.43%다. 후보에서도 8·12·15.999초는 선택 0개다.

각 variant의 OFF/ON/OFF 및 마지막 첫 카메라 OFF 복원까지 31 case가 모두 통과했다.
기하 query 유효, timestamp 16/16 유효, unknown indirect/dropped draw/fallback 0이다.
10개 카메라의 OFF raw RGB는 variant 사이에서도 byte 단위로 같으며 카메라·설정도 같다.
각 draw의 indexCount×instanceCount 합은 mapIndices와 일치한다. Engine Idx와 map Idx의
차이 36을 shadow 비용으로 분류하지 않았다. shadow의 전체 장면 순비용은 이 기록으로 판정하지 않는다.

근거는 `out/BernCapture20261015/full-scene-probe/runtimes/diagnostic-subset-installed-001/`과
`diagnostic-subset-70m-001/`의 `sweep-cameras-results/validation.json`, 후자의
`installed-vs-candidate.json`, `diagnostic-summary.txt`, `path-index-accounting.json`이다.
후자의 `quality/quality-manifest.json`에 10뷰 3열 원본 해상도 비교와 native crop의 출처를 기록했다.
외곽 형상은 유지되지만 일부 지붕 표면의 세부 무늬·반사 표현이 거칠어지는 차이를 확인했다.
화질이 같다는 판정을 내리지 않았고, geometry를 고정한 별도 512 atlas 후보를 out에서 검토 중이다.

### 전체 베른 맵 실행의 남은 한계

Product Bern descriptor의 전체 map scope로 0초·export 카메라를 비교하려던 실행은 map commit과
첫 draw 전에 메모리 guard로 중단됐다. 시작 free commit 8.477GiB가 51.88초에 1.416GiB로 내려가
설정된 1.5GiB 바닥에 도달했다. 해당 native process의 peak private는 5.701GiB였다.
자신이 실행한 native PID만 종료했고 사용자의 앱·OS·pagefile 설정을 바꾸지 않았다.
source/OBJ/binary/원본 및 overlay Resources/DataFiles의 실행 후 hash 검사는 PASS했다.

근거: `out/BernCapture20261015/full-scene-probe/runtimes/diagnostic-fullmap-70m-001/`
`bounded-attempt-result.json`, `memory-guard.json`.
전체 맵 GPU 감소, 기본 카메라 FPS 보존, 전체 장면 1/8 목표는 아직 검증하지 못했다.
위 subset의 45.65%를 전체 베른의 감소율로 확대하거나 전체 작업 달성으로 기록하지 않는다.
<!-- G43_NATIVE_SUBSET_MEASUREMENT_20261015_END -->

<!-- G43_LIMITS_INSTALLED_20261015_BEGIN -->
### 측정한 규칙의 실제 설치와 최종 빌드 현재성

2026-10-15 11:52 KST에 검토한 70m/1200px 규칙을 실제 `Client/Bin/Resources`에 설치했다.
수정 파일은 `Map/Derived/Bern/Pillar06/manifest.json`, `Pillar07/manifest.json`,
`ReplacementIndex.json` 세 개다. 두 manifest에서는 `limits` 두 필드만 바꾸고 index에서는
manifest SHA만 갱신했다. geometry·atlas bytes·원본 재질·저작 배치·렌더링 옵션을 바꾸지 않았다.
실제 제품 폴더의 bytes는 위 10뷰 GPU 측정 후보의 bytes와 같다.

설치 receipt는 `out/BernCapture20261015/validated-limits-install/application.json`의
`APPLIED_VERIFIED`다. 각 파일의 before/after backup, 적용 직전 SHA 확인, 원자적 교체,
실패 시 자기 변경만 rollback하는 절차를 사용했다. 설치 뒤 JSON 중복 key/asset ID 0개,
53개 asset ID 및 참조 파일 81개의 SHA 일치를 확인했다. 근거는
`out/BernCapture20261015/installed-resource-verification.json`이다.

11:49 KST의 읽기 전용 재검사에서 현재 제품 소스 2,371개는 Release003·Debug002의 각
빌드 후 fingerprint와 같았고 구성별 산출물 270개도 SHA·크기가 모두 같았다.
프로젝트의 명시 경로 1,370개에서 누락·미해석은 0개였다. 근거는
`out/BernGeometryProxy20261014/build-currency-20261015/summary.json`이다.
설치한 3개 derived JSON은 컴파일된 소스를 바꾸지 않는다. Release·Debug Product 빌드는
둘 다 PASS이며 기존 warning이 전부 없어졌다는 뜻은 아니다. Client·Server는 실행하지 않았다.

512 atlas 후보는 `out/BernCapture20261015/atlas512-candidate`에 입력과 runner만 준비했다.
geometry VB/IB·70m/1200px·원본 material·UV32/J32 계약을 고정한 702개 입력의 SHA 검사는
통과했다. 실행 전 8GiB free-commit 조건을 충족하지 못해 baker 자체를 시작하지 않았다.
guard 직후 관측치는 free commit 약 5.80GiB, free physical 약 0.67GiB였으며 정확한 guard
샘플 값으로 대신 기록하지 않는다. 제품은 256 atlas를 유지하고 512는 미베이크·미설치·
품질 미검증이다. 근거는 `atlas512-candidate/preflight.json`과
`installed256-limits-addendum.json`이다. 사용자의 앱을 닫거나 메모리 guard를 낮추지 않았다.

현재 완료는 실제 저폴리 선택 범위 수정·F7 탈락/실제 draw 진단·두 구성 빌드·지원 성탑의
GPU 감소·측정 bytes 설치다. 전체 컷신/일반 카메라의 게임 FPS, 전체 장면 1/8 감소,
지붕 표면 품질 보완과 사용자의 최종 화면 검증은 남아 있다.

수치가 포함된 4뷰 비교, native crop 3개, 10뷰 JSON 및 설치·빌드 증거를 다음 두 위치의
`2026-10-15_성탑62개_실측` 하위 폴더에 각각 10개 파일로 복사하고 SHA를 대조했다.

- `C:/Users/tnest/Desktop/LostArk/기술소개서/베른컷신최적화`
- `C:/Users/tnest/Desktop/Interview/기술소개서/첨부이미지/베른컷신최적화`

`검증결과.txt`와 그림 안에 62개 성탑 한정·전체 장면/게임 FPS 미측정을 명시했다.
복사 증거는 `out/BernCapture20261015/delivery-20261015/delivery-receipt.json`이다.
<!-- G43_LIMITS_INSTALLED_20261015_END -->

<!-- G44_CAPTURE_AND_MIRROR_20261015_BEGIN -->
### G44. 새 실제 캡처 분석과 균일 미러·거리 저장 수정

사용자는 설치된 Release에서 실제 geometry 감소와 FPS 향상을 확인했다. 이는 사용자
실행 관찰이며 아래 두 파일을 동일 카메라의 전후 A/B라고 해석하지 않는다.

| 실제 파일 (`Client/Bin/ProfilerCaptures` 아래) | 프레임 | frame interval 중앙값 | GPU VS 중앙값 |
|---|---:|---:|---:|
| `Bern_CutScene_20261015_140215_622_frame1122_18456_0.json` | 1,122 | 19.461ms | 1,799,260 |
| `Bern_CutScene_20261015_140227_980_frame1468_18456_1.json` | 120 | 51.967ms | 5,312,514 |

두 파일은 동일 Release PID18456의 서로 다른 시간 창이며 export viewport는1198×718이다.
GPU 유효 프레임은 각각1,118/116개이고 마지막4개는 pending이다. 저폴리 lease는 전체
활성이며 실제 low draw는 각각0~19/8~19개, late/preflight/fallback 실패는 모두0이다.
느린 창의 NonBlend VS 비중은96.94%다. `Map.Batch.Render` 평균20.78ms 안에
`Map.Material`14.66ms가 포함되므로 둘을 더하지 않는다. GPU BeginFrame의21.78ms는
0 draw·pipeline 미계측 준비 구간이며 vertex 처리 시간으로 귀속하지 않는다.

상세 CPU draw 수집이 꺼져 meshDraws는 비어 있고, 두 파일 모두 bernCutsceneGeometry가
없다. 현재 파일만으로 남은 비용의 asset 순위를 단정하거나 HUD의 비동기 geometry 수치와
같은 frame이라고 맞출 수 없다. 분석은 스트리밍 처리했고 두 파일의 SHA·크기·mtime
불변을 확인했다. 근거는 `out/BernCapture20261015/afternoon-analysis/summary.json`,
`slow-frame-attribution.json`과 파일별 analysis다.

### 적용한 제품 코드 5파일

`MapStaticBatchObject.cpp`의 blanket mirrored 거절을 제거하고
`MapStaticProxyBaker.cpp`의 `ReplacementSignedUniformAffine`에서 유한 affine,
균일 축 길이·직교·비퇴화 조건을 유지한 채 determinant 부호만 허용했다. 원본 source
World/역전치와 기존 SharedFront cull을 사용한다. 실제62 placement 중 준비 필요조건은
39→48개이며, 새 후보9개는 모두 균일 미러다. 비균일9개와 재질 변형5개는 계속 거절한다.
비균일 P06의 원본 vertex TBN과 local atlas TBN은 수치 반례에서 binormal 최대18.56°
차이가 있으므로 단순 gate 제거 대상에 포함하지 않았다. 거리70m/지름1200px·atlas256·
원본 재질·렌더링 옵션·기본 카메라 lease OFF 경로를 변경하지 않았다.

`ProfilerTool.h/.cpp/_Cutscene.inl`에서는 일반 JSON Save에 보관 거리 표본을 연결했다.
활성 기록은 stop→기존 최대2초 GPU drain→immutable completed→같은 단일 exporter로
저장한다. raw history와 거리 표본을 포함하되 최대200만 source행은 복제하지 않는다.
준비/worker 실패 시 completed를 유지하고 수동 재시도할 수 있다. 실제 Poll의 디스크
commit 결과로만 완료를 표시하며 거리 없는 저장은 그 사실을 명시한다. 현재 일반
프레임·export metadata와 이전에 보관한 베른 표본은 같은 시간 범위라고 가정하지 않는다.
앱 종료 시 기존 bounded cancel 정책은 유지한다. 이번 두 파일의 거리 누락이 어떤
단일 분기에서 발생했는지는 재현 증거가 없어 확정하지 않는다.

반영 전 후보·원본 SHA와 인코딩/개행을 대조하고 원본 backup 후 파일별 원자 교체했다.
근거는 `out/BernCapture20261015/source-g44-applied-20261015-142456/application.json`의
APPLIED_VERIFIED다. 전체 반영 코드는 같은 PLAN의 G44에 보존했다.

### 실행한 검증과 범위

- 실제62 TRS와 8종 signed rotation, 새9개 SharedFront pass, 비균일/shear/NaN/Inf/
  projective/퇴화 및 mixed determinant 거절을 실제 helper 추출 C++로179건 검사했다.
  `/W4 /WX` 컴파일과 실행 PASS다. `mirrored-uniform-candidate/predicate-result.json`.
- 실제 저장 함수 본문을 추출한 CPU 상태 검사는55건 PASS이며 Engine/readback/exporter는
  test double이다. 기존 native serializer13개 검증 범주도 PASS다. 실제 OS 비동기
  쓰기·게임 UI 성공으로 대신 기록하지 않는다. `distance-save-candidate/test-results.json`.
- 독립 코드 리뷰에서 worker 소유권, 중복 저장, 준비 예외, Poll 귀속과 명시 재시도 흐름의
  추가 결함은 발견하지 않았다.
- Release Product PASS: `out/BuildPipeline/runs/20261015T052636567Z-release-product.json`.
  소스 fingerprint `6af6274daf41ae37cb5d4bbd7d18ba5ae9643f0dc399a097a506accc0c27c855`가
  빌드 전후 동일하다.
- Debug Product PASS: `out/BuildPipeline/runs/20261015T052753112Z-debug-product.json`.
  소스 fingerprint `262650d203972e856b8cbd63b0b844c28d0d27b7765882ae6adad80fc0b403d5`가
  빌드 전후 동일하다. 두 빌드는 기존 warning을 포함하며 warning0을 뜻하지 않는다.

G44 native GPU 전후 비교는 별도 증거로 기록한다. 준비 후보9개 증가를 실제 draw9개
증가나 전체 장면 geometry 감소율로 바꾸어 보고하지 않는다. 전체 베른1/8 목표와
기본 카메라 FPS 보존의 통제된 비교는 아직 완료 판정 대상이 아니다.

최신 Release OBJ를 사용한 새 native probe compile/link와 소스·OBJ·출력 해시 검증은
PASS다. 기존 Release003 baseline은 소스2,371개·실제 link OBJ hash475개·EXE/DLL/CSO·
필요 리소스와5개 카메라를 별도 복사해 역사적 baseline으로 동결했다. 현재 소스의 빌드로
표시하지 않는다. `out/BernMirrorProbe20261015/prepared-result.json`에 경계를 남겼다.
GPU 비교의 실행 전 가용 commit은6.924282GiB로 기존8GiB 기준 미달이었다. 물리 여유는
3.906078GiB였으며 native process는 시작하지 않았다. 반복·guard 완화·사용자 앱 종료는
하지 않았다. 이번 G44의 추가 GPU 감소량은 따라서 미측정이다.

사용자의 전체 Git 변경 통합 지시에 따라 기존 소스·저작/게시 데이터·문서·기술소개
이미지와 이번 수정을 함께 PR로 전달한다. 기존 Resources 전달 계약은 Drive를 유지하며,
자동 생성 `Client.aps`는 `.gitignore`에 추가해 실제 파일은 보존했다. 변경 JSON141개와
XML18개 구문 검사, conflict marker 및 고확신 비밀키 패턴 검사를 통과했다. 이는 전체
게임플레이·모든 민감정보의 완전 검증을 의미하지 않는다. `git diff --check`는 기존 소스의
후행 공백51줄을 보고했고, 미추적 문서 전문까지 stage한 뒤 CRLF를 허용한 검사는3,351건의
공백 지적을 보고했다(대부분 기존 코드 전문 계획서). 이 검사를 PASS로 기록하거나
무관한 코드·문서 전체의 공백을 재작성하지 않았다. 검증 근거는
`out/BernCapture20261015/git-integration/validation.json`과 `staged-whitespace-findings.json`이다.
<!-- G44_CAPTURE_AND_MIRROR_20261015_END -->


<!-- G45_EVIDENCE_AUDIT_20261015 -->
## G45 — 적용 상태·HUD 지표와 기술소개 이미지 근거 정리

기준 main은 Effect/Collider anchor PR #542가 병합된 `92169ebe1`이다. 관련 소스와 G44 적용
5파일의 receipt SHA, 설치된70m/1200px index·manifest 및 참조81파일, 실제 사용자 JSON2개의
SHA를 대조했다. 제품 C++·저작 데이터·Resources를 추가 변경하지 않았다.

검증된 기하 감소는 G43 성탑62개 부분 장면의 동일 카메라 결과다. 0초 VS445,695→242,225
(45.65%↓), 엔진 제출 Idx1,081,890→620,544(42.64%↓)이며 전체 장면이나 게임 FPS의 감소율이
아니다. 실제 사용자 캡처에서도 저폴리 draw0~19/느린 구간8~19와 fallback0을 확인했다.
G44 균일 미러9개는 CPU 준비 필요조건을 확장했으나 추가 GPU 감소량은 아직 미측정이다.

HUD Vtx는 Geometry query의 전체 GPU VS이며 일반 F7 pipeline query와 전체 Update→Render
구간을 공유한다. HUD를 맵만, 일반 JSON을 전체 장면이라고 설명하지 않는다. 관측 frame/cue/
camera와 지연 완료·통계 집계가 다를 수 있다. 사용자가 관측한80만→50만에 대응하는 같은
카메라의 저장 표본 쌍은 확인하지 못했다. 37.5%를 검증된 개선율로 보고하지 않는다.
거리 JSON은 같은 Geometry query의 frame을 기록하므로 후속 재측정의 정본으로 사용한다.

새 실행 전 메모리 검사는 free commit5.189907GiB로8GiB 기준 미달, physical1.705193GiB였다.
native를 시작하지 않았고 guard를 낮추거나 사용자 앱을 종료하지 않았다. 과거 전체맵은
실제 로드 중 메모리 하한에서 첫 draw 전에 중단됐으므로 두 사건을 구분한다.

최신 main Product Release `20261015T094035032Z-release-product.json`, Debug
`20261015T094639831Z-debug-product.json` 모두 PASS. 각 빌드 전후 fingerprint는 각각
`ffdd693bccc60b8fa062f6e8339bd6b8fdb7603f3138b18beb215207f042ad63`,
`b67a0428b7dfa02d494e82a67d205caa0576aee882e1119b3214e28faf1647b5`로 동일했다.
기존 인코딩/DirectXTK PDB warning은 남으며, UI/실제 게임 동작 확인을 뜻하지 않는다.

`기술소개서/베른컷신최적화/2026-10-15_근거정리`에 PNG14개(원본12개+데이터 도표2개),
원자료·SHA·코드 행 증거와 적용/미적용 안내를 정리하고 Interview 첨부이미지 경로에도 같은
bytes로 복사했다. 과거 offscreen16-frame FPS 그림은 게임 FPS 증거에서 제외했다.
도표 수치·PNG SHA와 복사 동일성을 독립 검토했고 전체1/8·기본 카메라 통제 FPS·G44 GPU
추가 개선·실제 F7 UI 저장 재열기는 미완료로 유지한다.
