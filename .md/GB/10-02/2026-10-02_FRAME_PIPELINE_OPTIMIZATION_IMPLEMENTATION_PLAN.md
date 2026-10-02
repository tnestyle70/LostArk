# 프레임 계측·CPU·draw 제출 통합 최적화 구현 계획

## G00. 현재 근거와 완료 범위

이 계획은 현재 Client의 프레임 수집에서 CModel/CMaterial의 실제 draw까지를 하나의 경로로 설명하고,
검증 가능한 최적화를 기존 경로에 반영한다. 형식은 `.md/GB/local.md`와 연결된
`계획서작성규칙.md`를 따른다. 구현 전 후보 전문을 이 문서 뒤에 기록하고, 실제 검증과 남은
제품 실행 측정은 대응 RESULT에 구분한다. 기존 09-22·09-27·09-30 성능 계획의 이력은 복제하지 않는다.

2026-10-02 입력은 `Client/Bin/ProfilerCaptures`의 베른 기본·베른 무비·도화가 무비 JSON이다.
베른 기본은 120프레임 평균76.127ms(13.14FPS), CPU Update17.203/Render58.205ms다.
베른 무비의 저장된 앞20프레임은233.544ms, 중간41프레임은130.269ms다. 전체120프레임의
최대281.489ms이므로 사용자가 관찰한1~2FPS 구간 자체는 저장 범위에 없다.
두 베른 입력은 Detailed ON이며 무비79프레임에서 raw CPU scope1,584,603개가 누락됐다.
GPU 전체시간은 utilization이 아니다. 기존 캡처와 후보 kernel 실험을 제품 FPS 개선으로 바꾸어 기록하지 않는다.

현재 브랜치는 `codex/laptop-wifi-lan-20261002`이고 같은 checkout에서 다른 사용자 작업이 진행 중이다.
기존 수정은 baseline으로 보존하고 이 기능의 파일만 좁게 병합한다. 다른 세션이 반영 중인
Animation channel sample reuse는 중복 구현하지 않고 고정한 소스와 실물 WModel로 독립 검증한다.
Client/Server는 실행 중이며 소스 준비와 수치 검사는 가능하다. 제품 출력의 실제 링크·교체 시점에만
점유를 확인한다. Client/UI 실행과 최종 화면 캡처는 AGENTS의 사용자 전용 경계를 따른다.

## G01. Client.cpp → MainApp → GameInstance의 CPU 흐름

`Client/Default/Client.cpp`는 메시지 처리와 timer 갱신 뒤 Begin_Frame을 호출하고,
Client.Update → Client.Render → End_Frame → frame limiter 순서로 진행한다.
interval은 이전 Begin에서 현재 Begin까지이고 CPU frame은 현재 Begin에서 End 초반까지다.
급격한 한 프레임 지연은 interval 행과 원인 CPU 행 사이에 한 프레임 차이가 생길 수 있다.

`MainApp.cpp`는 입력·UI, network drain, Engine 갱신, presentation 준비, effect·저작 도구와
level/environment 후속 처리를 소유한다. `GameInstance::Update_Engine`의 실제 순서는
Input → Sound → PriorityUpdate → Camera → ObjectUpdate → Physics → PostPhysics → LevelUpdate → LateUpdate다.
Object_Manager/Layer는 기존 소유 컨테이너와 phase별 비소유 목록으로 객체를 순회한다.
Server fixed tick/navigation/damage 판정은 별도 프로세스이며 Client profiler에 들어오지 않는다.

모델 포즈는 CModel → CAnimation → CChannel → CBone 경로다. interpolation, model-local
root/blend, hierarchy combine, mesh palette 생성·업로드를 구분한다. 포즈 공유는 draw 병합과 별개다.
Effect는 playback별 fixed-step과 seek를 가지므로 낮은FPS의 긴delta가 다음CPU 갱신량을 늘릴 수 있다.
worker loading의 전체 경과·main commit·join과 main frame을 합산하지 않는다.

## G02. Renderer → MapStaticBatch → Shader/Material → Mesh

MainApp Render는 환경 effect의 visible update와 world render, ImGui, text와 Present를 포함한다.
CRenderer::Draw는 frame provider 뒤 최종 카메라 제출을 실행하고 shadow·portrait·NonBlend·SSAO·Lights·
SceneHDR·Blend·postprocess·UI 등 기존 패스 순서를 유지한다. CPU scope는 명령 준비/제출/대기의 경과이고,
GPU scope는 timestamp 사이의 경과다. 두 숫자를 더한 것을 프레임 비용으로 사용하지 않는다.

맵 placement50,021개는 batch16,424개와 개별 fallback1,222개로 구성된다. 베른 기본의 실제
batch Render는570개, visible instance1,035개, 전체draw1,089회다. 무비 앞20프레임에서는
visible instance8,101개, 전체draw5,178회로 증가한다. batch당mesh/material이 여러 개면 draw도 늘어난다.
RNM/lightmap/static-shadow texture와 material ABI 차이를 무시하고 batch key를 합치지 않는다.

`MapStaticBatchObject::Submit_FinalCamera`는 camera revision별 visibility payload를 준비하고
비어 있지 않은 배치만 제출한다. stationary camera는15,794개 visibility cache를 재사용하지만
이동 camera는 같은 수를 재평가한다. 이후 Render는 mesh마다 material bind → shader Begin/Apply →
CMesh::Render_Instanced의 IA 설정과 DrawIndexedInstanced를 수행한다. 기존 value cache와 LOD,
instance buffer 재사용은 이미 구현돼 있으므로 새 최적화의 성과로 다시 계산하지 않는다.

그림자는 camera가 아니라 light volume으로 판단한다. 투명/물/scene-color 소비 순서, authored visibility,
부착 연출, render settings와 shader 수식을 보존한다. 최종 카메라의 frustum 통과는 벽 뒤 occlusion 판정이 아니다.

## G03. Profiler.h/.cpp → ProfilerTool → ProfilerCaptureIO의 측정 계약

| 저장/표시 | 의미와 범위 | 남은 한계 |
|---|---|---|
| CPU raw scope | QPC 시작/끝, thread, nesting depth | 명시 계측 위치만 관측; lock/clock 비용 존재 |
| CPU inclusive/Self | 호출 전체/기록된 자식 제외 | 누락 frame의 Self를 정확한 exclusive 값으로 표시할 수 없음 |
| cpuWork | map/NPC/ambient 고정 배열 누적 | inclusive 중복 주의; raw cap과 별개 |
| GPU timestamp | frame 및 pass의 지연 query | busy%, occupancy, cache miss, VRAM 사용량 아님 |
| pipeline stats | 선택pass의 PS/VS invocation | ALU 명령 수나 shader당 비용 아님 |
| counters | 실제 enqueue/draw/index/instance/copy 등 | writer 없는 texture counter는N/A |
| animation join | 평가 모델과 성공 draw의 같은frame 연결 | notSubmitted는 정확한 frustum-out 수가 아님 |
| long operation | 8ms 이상 완료scope, 별도256개ring | 아직 끝나지 않은 작업은 표시 안 됨 |
| JSON metadata | 저장 순간의 adapter/build/camera/settings | 과거 모든frame 조건을 증명하지 않음 |

CPU raw cap8192/frame, history1200frame, GPU8slot/minimum4frame delayed query, GPU pass128/frame의
현재 상한을 유지한다. F7은 Debug 창 토글이며 Capture가 수집을 제어한다. 창을 닫아도 수집은 계속된다.
기본 선택120frame만 저장하므로 초기 저FPS가 export window 밖으로 나갈 수 있다.

이번 후보는 불완전한 CPU frame이 포함된 집계의 Self를 미확정으로 표시한다. Snapshot/JSON/UI에
선택·저장·보유·eviction 범위를 함께 남겨 선택창 밖의 느린 frame이 있는 것을 알려준다.
기존 v3는 additive 확장하고 enum ordinal, 기본 수집량, async 저장·GPU 비대기 회수를 유지한다.

## G04. Unreal·Unity의 방식과 이 프로젝트 적용 순서

아래는 공식 문서에 설명된 원리를 현재 코드에 적용한 설계 판단이다. 특정 엔진의 기능이나 성능을
그대로 구현했다고 주장하지 않는다.

| 공식 근거 | 해당 원리 | 현재 프로젝트의 적용 |
|---|---|---|
| [Unreal Mesh Drawing Pipeline](https://dev.epicgames.com/documentation/en-us/unreal-engine/mesh-drawing-pipeline-in-unreal-engine) | 변하지 않는 draw 설명·binding을 준비해 재사용하고 호환 명령을 정렬·병합 | 정적/프레임/객체 입력 분리와 명시 revision을 선행; 순서 민감한 draw 제외 |
| [Unity SRP Batcher](https://docs.unity.com/en-us/engine/6000.3/manual/analysis/graphics-performance-profiling/in-urp/reduce-draw-calls-urp/srpbatcher/srpbatcher) | 같은 shader variant의 material 상태를 지속 보관하여 draw당CPU setup 절감 | draw수 자체와 material/pass 준비비용을 별도로 측정; 기존Shader value cache와 중복 금지 |
| [Unreal Animation Sharing](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-sharing-plugin-in-unreal-engine) | 같은 상태의 애니메이션 평가 결과 재사용 | WORLD Movie의 동일 cooked channel·sample time만 공유; model별root/blend/combined pose 유지 |
| [Unity profiling best practices](https://unity.com/how-to/best-practices-for-profiling-game-performance) | 대상기기·제품 부하에서 병목을 구분하고 변경 전후 검증 | Debug/Detailed/도구창 비용과 제품 성능을 구분; 같은경로 반복캡처 |
| [D3D11 Immediate/Deferred Context](https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-render-multi-thread-render) | worker가 독립 context에서 기록하고 immediate context가 실행 | 공유 immediate context에 병렬for를 추가하지 않음; CPU 순수준비의 분리가 선행 |

## G05. 이번 실제 반영 후보와 검증

1. Profiler의 partial Self와 capture window 경계: overflow/nesting/reset/1200 eviction/선택 저장을
   실제 함수로 검증하고 JSON을 parse한다. 저장한 기존 캡처와 새 additive 필드를 모두 분석한다.
2. MapAssetRenderUtils의 visibility-only 경로: 상세진단 전체결과를 만드는 기본 경로는 유지한다.
   실제로 shouldRender/wouldBeVisible와 grace state만 소비하는 MapStaticBatch의 두 호출만 opt-in한다.
   유효 plane과 float 안전범위에서 첫 확실한 분리면으로 종료하고 진단·invalid·overflow 위험은 기존
   전체평가로 되돌린다. 설치placement·실제카메라·순서있는visible ID·grace·경계·역방향·실패 입력을 대조한다.
3. 다른 세션의 Movie channel sample reuse: 실물38개WModel로 고정한 Animation/Channel/Bone 구현의
   순차·역방향·혼합시각·clone·동시접근 결과를 독립 대조하고 cold/warm 시간과 메모리 범위를 기록한다.
4. `Tools/Profiler/analyze_capture.py`: 원본v3에서 frame 범위, percentile, mainCPU, cpuWork,
   valid GPU pass와 작업량을 재계산한다. 입력SHA와 조건차이를 기록하며 누락GPU를0으로 평균하지 않는다.
   전후 관찰 비율은 인과적 개선율이나 품질 동등성 증거로 대신하지 않는다.

새 C++ 파일은 없고 기존 vcxproj/filter 등록을 유지한다. Python 분석기는 제품빌드에 등록하지 않는다.
파일별 encoding/newline과 적용직전baseline SHA를 확인한다. 독립비평 후 전체 후보 전문을 아래에
보존하고 기능별diff로 설치한다. 실제제품 검증은 정상증분 Debug/Release Product Build이며
다른세션의 빌드와 동시에 같은출력을 쓰지 않는다. 실행중 EXE/DLL 점유는 교체시점에만 처리한다.

## G06. 더 큰 최적화의 설계 순서와 승인 근거

| 순서 | 실제 변경 단위 | 반드시 보존/측정할 것 | 다음 단계의 근거 |
|---|---|---|---|
| 정적draw 준비 보존 | CModel/CMaterial의 immutable mesh/material 입력과 per-frame binding 분리 | 같은 resource·pass·constant·실행순서, authoring revision invalidation | material/pass CPU가 여전히 큰 비중인지 |
| 공간계층 | 기존map placement/batch 위에 보수적 spatial chunk/BVH 후보조회 | camera/light별 독립판정, ordered visible ID, move/hide/reload 반영 | 움직임시 전체batch 검사가 지배적인지 |
| 호환batch 병합 | texture bank/array와 shader ABI를 함께 설계 | mip/format/sampler/RNM/static-shadow 동등성, 메모리·LOD·cull 손익 | 실제서로호환draw 비율과 shader변경검증 |
| 조명 coverage | source material row별 조명제출과 픽셀영역 축소 | 동일 light response/row identity/stencil/투명경계 | valid GPU Lights가 다음병목인지 |
| CPU 작업 병렬화 | immutable input snapshot에서 pose/cull 준비 후 main commit | lifetime/cancel/order, thread-local output, worker wait와 critical path | task scheduling비용보다 충분히큰독립작업인지 |
| GPU cull/indirect | 기존draw 경로에서 visibility/LOD 결과를 GPU에 유지 | CPU readback 없는결정, draw/order/fallback, shader·buffer ABI | CPU제출감소와 GPU추가비용의 실기기비교 |

이 단계들은 설계 순서이며 이번 작은변경의 구현완료 목록이 아니다. 데이터·GPU ABI·권위와
화질을 바꾸는 큰변경을 근거없는일괄수정으로 묶지 않는다. D3D12 전환·새렌더러 도입도
현재D3D11의 병목을 자동해결한다는 근거로 사용하지 않는다.

공간 색인은 화면 밖 대상을 싸게 찾는 방법이다. 이미 보이는 수천 개의 draw는 그대로 남는다.
따라서 가시성 kernel 절감 뒤에도 `MapBatchDraw`가 크면 다음 구현의 중심은 제출 횟수와
draw당 준비량이다. 호환 재질/베이크 조명 texture의 batch key 분포를 먼저 세고, 병합 가능한
범위에만 persistent binding과 공간 단위 병합/HLOD를 설계한다. HLOD는 여러 모델을 멀리서
하나의 대체 모델로 그리는 파생 데이터가 필요하며 일반 index LOD와 다르다.
도입하더라도 기존 CModel/CMaterial 소비 경로를 확장하고 두 번째 모델 런타임을 만들지 않는다.
D3D11에서 GPU가 visible instance를 골라도 batch별 indirect draw를 CPU가 각각 호출한다면
draw 제출 횟수는 그대로다. GPU culling이라는 이름만으로 CPU 제출 병목이 해결됐다고 하지 않는다.
근거는 [Unreal HLOD 개요](https://dev.epicgames.com/documentation/unreal-engine/hierarchical-level-of-detail-overview-in-unreal-engine)와
[D3D11 DrawIndexedInstancedIndirect](https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11devicecontext-drawindexedinstancedindirect)이며,
현재 코드에서 다음으로 바꿀 범위와 순서는 위 실측을 바탕으로 한 판단이다.

## G07. 반복 측정과 성공 판정

사용자캡처는 베른정지/베른입장동일구간/도화가동일구간을 분리하고 같은 해상도·화질·전원·카메라·
도구창 조건으로 반복한다. Detailed OFF, 충분한window, 느린구간직후 Capture OFF와 GPU회수후 저장을 사용한다.
현재제품의F7은Debug전용이다. Release에서F7이된다고안내하거나측정때문에임의로제품UI를노출하지 않는다.
Release 실기기 프레임 검증은 별도 지원되는 측정수단/사용자관찰과 구분한다.

baseline의 P50/P95/P99, CPU Update/Render/FinalCamera/NonBlend, animation evaluation,
cpuWork와 draw/visible/cache hit/rebuild를 함께 비교한다. 같은 품질·입력의 kernel 동등성,
컴파일·제품배포·새캡처·화면판정을 각각 다른 완료조건으로 기록한다.
76.127ms에서60FPS(16.667ms)는 전체경로약78.1%감소가 필요하다. 한 작은함수의배속을
전체FPS배속으로 환산하거나 반복횟수로 엔진의최종상한을 확정하지 않는다.

## G08. 고정 Movie의 사전 계산 범위

베른 입장 카메라는 `Data/Encounters/Bern/BernEntranceCamera.json`의 16초·16키 경로를
`Level_Bern`이 소비한다. 같은 경로를 반복한다는 점은 준비 비용을 로딩 단계로 옮길 근거다.
다만 렌더링은 영상 파일의 재생이 아니다. 같은 메시라도 매 프레임 GPU가 실제 정점을 처리하고
픽셀을 그려야 한다. CPU 사전 준비, draw 수 감소, GPU 실행량 감소는 별도 성과로 측정한다.

| 준비할 대상 | 보관할 결과 | 실행 중 남는 일과 무효화 조건 |
|---|---|---|
| 정적 mesh/material 연결 | mesh index, 호환 material/pass, 변하지 않는 resource binding 설명 | frame/camera/time/instance 값은 갱신; material override·reload·resource 수명 변경 시 재생성 |
| 카메라 구간별 가시 후보 | 구간 전체에서 보일 수 있는 정적 batch의 stable ID 집합 | 현재 카메라의 정확한 검사·grace·visibility는 유지; camera/aspect/placement revision이 다르면 기존 전체 경로 |
| 동일 animation 입력 | ordered bone index와 SRT key에서 특정 시각의 local matrix | actor별 root·blend·hierarchy·skin palette와 draw는 계속 수행 |
| 정적 LOD 파생 데이터 | 감소한 index 범위와 오차 | 실제 화면 크기에 따른 범위 선택과 GPU draw는 계속 수행 |

카메라 키프레임에서만 보이는 목록을 합치면 키 사이에서 잠깐 보이는 물체를 잃을 수 있다.
구간 전체의 보수적 가시 집합을 증명하거나 공간 계층으로 후보를 좁힌 뒤 기존 exact cull을 실행한다.
무비 전용 cache는 자유 카메라·편집·다른 화면 비율에서 사용하지 않는다. 캐릭터·이펙트·움직이는
배치·light/shadow의 가시성은 각 owner가 계속 판단한다. fixed movie cache를 이유로 숨김 상태를
저장 시점으로 되돌리지 않는다.

실제 우선순위는 모든 카메라에 이득이 있는 공간 후보 조회와 정적 binding 준비다. 고정 경로
cache는 그 위의 선택적 가속으로 둔다. D3D11 command list의 재생도 GPU 작업을 제거하지 않으며,
mutable constant/instance buffer와 context state 수명을 먼저 분리해야 한다.

현재 `CMapPlacementRuntime`는 area의 stage/commit/Clear 수명을 소유하지만, MapTool·WorldSequence·
self motion은 batch의 `Update_Instance`, `Set_InstanceVisible`, `Set_InstanceSuppressed`,
`Set_InstanceCameraPreviewSuppressed`도 직접 호출한다. 공간 색인의 dirty/revision은 이 변경 입구에
연결해야 한다. refit 전 변경 객체는 항상 후보에 포함하고, 원래 reject grace와 진단 transition을
보존한다. 후보 mark를 기존 layer 순서로 소비하여 draw와 payload 순서를 유지한다.
`CLayer::Submit_FinalCamera`의 전체 callback 순회까지 연결하지 않은 BVH는 callback 비용을 줄이지 못한다.

작은 후속 준비 후보는 `Build_ScreenLodView`의 projection 검사·view scale·viewport 변환처럼
카메라 공통인 값의 1회 계산이다. key는 view/projection revision뿐 아니라 viewport도 포함한다.
batch별 bounds·scale·현재 재질 admission은 남긴다. LOD가 없는 batch의 호출을 단순히 막으면
현재 screenLod 포인터로 세는 LOD0/source/submitted indices counter 분모가 바뀌므로 이번에 섞지 않는다.

## G09. 현재 LOD 구현과 과거 GPU 시도의 판정

현재는 `CStaticMeshLod::Create`가 로드 때 meshoptimizer로 정적 모델의 index LOD를 만들고,
`Select_Range`가 CPU에서 화면 오차를 비교한 뒤 기존 `DrawIndexedInstanced`를 실행한다.
`Engine/ThirdPartyLib/meshoptimizer`에는 MIT 라이선스 v1.0 subset이 이미 포함돼 있다.
별도의 GPU 라이브러리·DLL 설치가 필요한 상태가 아니다.

LOD 생성은 24,576~3,145,728 indices, 최대 1,048,576 vertices에 한정한다. 원래 vertex buffer를
유지하고 normal/tangent/binormal·UV 3개·vertex color를 오차에 포함하며 경계를 고정한다.
목표 index 비율은 55%/30%지만 결과가 이전 단계보다 15% 이상 줄지 않으면 채택하지 않는다.
draw admission은 지원하는 불투명/마스킹 BG 재질 등에 제한하며 원래 0.25px 오차 조건을 유지한다.
따라서 작은 메시, 경계/UV 제약이 많은 메시, 지원하지 않는 재질은 LOD 이득이 없을 수 있다.
원본 index를 줄여도 재질당 draw 제출 횟수는 자동으로 줄지 않는다.

[09-22 결과](../09-22/2026-09-22_BERN_RELEASE_PROFILER_OPTIMIZATION_RESULT.md)의 G02는
LOD 자체의 실패가 아니라 draw마다 compute dispatch/UAV/indirect로 LOD를 고르는 비용을
CPU 선택으로 제거한 변경이다. 120 draw 고정 fixture에서 작은 입력은 원본 0.471ms,
기존 compute LOD 1.022ms, CPU 선택 LOD 0.201ms였다. 이는 과거 독립 fixture이며
현재 베른 게임 FPS나 모든 에셋의 이득을 뜻하지 않는다. 설치 submesh 4개 중 1개만
148,224→102,456 indices로 줄었던 기록도 있다.

현재 `MODEL_MESH_DATA`는 mesh별 vertices/indices를 가지며 원본 LOD chain을 소비하는 계약은 없다.
설치 모델에서 실제 사용하는 LOD는 위 생성 경로다. 이 PC에서 원본 게임 패키지 전체와 LOD별
추출물을 확인하지 못했으므로 원작 리소스에 LOD가 없다고 결론 내리지 않는다. 원본 확인은
package의 실제 LOD descriptor와 converter 전달 여부를 조사하는 별도 단계다.

[Unreal 자동 LOD](https://dev.epicgames.com/documentation/en-us/unreal-engine/static-mesh-automatic-lod-generation-in-unreal-engine)도
먼저 단순화 geometry를 만든다. GPU 기반이라는 말은 geometry 생성, runtime LOD 선택,
가시성 판정, indirect 제출 중 어느 단계를 옮기는지 구분해야 한다.
향후 GPU 경로는 큰 후보 목록을 한 번에 cull/LOD하고 결과를 GPU buffer에 유지하여 CPU 제출까지
줄이는 경우에 검토한다. 기존처럼 작은 draw마다 compute를 추가하는 방식으로 되돌리지 않는다.

## G10. 이번 수정의 H/CPP 책임과 전체 코드

`FProfilerCaptureWindow`는 저장창 범위만 전달하는 DTO다. `m_EvictedHistoryFrames`는 Reset 이후
history ring에서 제거한 실제 프레임 수이며 frame number의 차이로 추정하지 않는다.
`Get_CaptureWindowLocked`는 Snapshot과 같은 mutex 아래에서 저장 범위를 만들고,
`Get_CaptureWindow`는 UI가 raw frame 복사 없이 이를 조회하게 한다.
`FProfilerScopeAggregate::SelfComplete`는 선택 구간의 CPU scope 누락을 보수적으로 전달한다.
ProfilerTool은 불완전 Self를 `--`로 표시하고, ProfilerCaptureIO는 v3에 `captureWindow`를 추가한다.
기존 시간 집계의 값·기본 저장량·GPU 예산은 바꾸지 않는다.

`MAP_FRUSTUM_CULL_DETAIL`은 기본 FULL이며, `VISIBILITY_ONLY`를 명시한 MapStaticBatch의
batch/instance 검사만 상세 plane 배열 작성을 생략한다. `g_ValidatedMaximumPlaneOffset`은
검증된 plane cache와 함께 갱신하는 float overflow 안전성 판정 값이다. 큰/무효 입력 또는
diagnostics는 FULL 경로를 따른다. 두 경로 아래의 grace와 state commit은 공통으로 유지한다.
공개 helper signature를 소비하는 기존 호출자는 기본 인수로 이전 동작을 유지한다.

아래 전문은 검증한 후보를 적용하기 전에 저장한다. 파일별 원본/후보 SHA와 검증 증거는
`out/FrameOptimization20261002`에 보존하며, 실제 반영 상태는 대응 RESULT가 정본이다.


### G10.profiler. 적용 전 전체 코드

#### `Engine/Public/Profiler.h`

후보 SHA256: `b2f29bd300a950ef459c64c4b9c1b066ab6ab479df55827451fd34e7d6db32f0`

```cpp
#pragma once

#include "Engine_Defines.h"
#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <deque>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

NS_BEGIN(Engine)

enum class EProfilerCounter : uint16_t
{
    DrawCalls,
    InstancedDrawCalls,
    Instances,
    Indices,
    RenderSubmissionsPriority,
    RenderSubmissionsShadow,
    RenderSubmissionsNonBlend,
    RenderSubmissionsBlend,
    MapPlacements,
    MapVisibleInstances,
    MapBatchCount,
    MapFallbackObjects,
    TextureRequests,
    TexturePathHits,
    TextureContentHits,
    TextureUniqueSrvs,
    TextureEstimatedGpuBytes,
    NavigationQueries,
    NavigationExpandedNodes,
    NavigationQueryMicroseconds,
    NavigationPathCells,
    SceneColorCopies,
    SceneColorCopyBytes,
    ImGuiDrawLists,
    ImGuiVertices,
    ImGuiIndices,
    ImGuiDrawCommands,
    ImGuiDrawCalls,
    ImGuiCallbacks,
    ImGuiRenderWindows,
    ImGuiActiveWindows,
    ImGuiPlatformViewports,
    ImGuiVertexUploadBytes,
    ImGuiIndexUploadBytes,
    ImGuiConstantUploadBytes,
    ImGuiTextureUploadBytes,
    ImGuiBufferGrowths,
    ImGuiTextureCreates,
    ImGuiTextureUpdates,
    ImGuiDeviceObjectBuilds,
    ImGuiBufferMapFailures,
    PickingReadbacks,
    PickingReadbackBytes,
    IndirectDrawCalls,
    IndirectIndexUpperBound,
    ShadowCacheHits,
    ShadowCacheMisses,
    ShadowStaticCasters,
    ShadowDynamicCasters,
    MapCullingCandidates,
    MapCullingVisible,
    MapLod0Draws,
    MapLod1Draws,
    MapLod2Draws,
    MapLodSourceIndices,
    MapLodSubmittedIndices,
    LightRecords,
    LightDrawCalls,
    LightUploadBytes,
    MapLodAvailableDraws,
    LightCullingCandidates,
    LightCullingRejected,
    EffectBoundsCandidates,
    EffectBoundsCulled,
    EffectMarkerSamples,
    EffectMarkerHistoryRequests,
    EffectAmbientSuspended,
    EffectAmbientAdvanced,
    ImGuiPresentAttempts,
    ImGuiPresentBusy,
    ImGuiPresentFailures,
    ImGuiPresentOccluded,
    MapBatchVisibilityCacheHits,
    MapBatchVisibilityRebuilds,
    MapBatchEmptyRenders,
    MapBatchVisibleRenders,
    MapBatchBoundsRejected,
    MapBatchUploadBytes,
    NpcAuthoredHiddenUpdates,
    AmbientUnboundedUpdates,
    NpcCullingCandidates,
    NpcCulled,
    NpcDeferredPoseEvaluations,
    Count
};

// Fixed main-thread work categories are accumulated even with raw detail disabled.
// Times are inclusive: a parent category can overlap its instrumented children.
enum class EProfilerWork : uint8_t
{
    MapBatchRender,
    MapBatchVisibility,
    MapBatchMaterial,
    MapBatchPass,
    MapBatchDraw,
    MapObjectRender,
    MapWaterRender,
    NpcUpdate,
    NpcLateUpdate,
    NpcRender,
    AmbientVisibility,
    AmbientAdvance,
    AmbientSubmit,
    Count
};

struct FProfilerWorkStats final
{
    uint64_t Calls = 0;
    double CpuMs = 0.0;
};

struct FProfilerWorkToken final
{
    uint64_t BeginTick = 0;
    uint64_t FrameNumber = 0;
    uint64_t CaptureEpoch = 0;
    uint64_t InstanceId = 0;
};

struct FProfilerScopeSample final
{
    uint32_t NameId = 0;
    uint32_t Depth = 0;
    /* Win32 thread id of the thread that ran the scope. Worker scopes (Loader,
       Effect preparation) are attributed to the frame in which they ended. */
    uint32_t ThreadId = 0;
    uint64_t BeginTick = 0;
    uint64_t EndTick = 0;
};

/* GPU samples belong to their original submitted frame. Pending is not a
   zero-duration result. Failed/unsupported GPU collection keeps CPU data. */
enum class EProfilerGpuFrameStatus : uint8_t
{
    Unsupported,
    Pending,
    Valid,
    Disjoint,
    Dropped,
    Error
};

/* Inclusive timestamp interval relative to the GPU frame begin. Nested scopes
   overlap; multiple copies with the same NameId remain separate samples. */
struct FProfilerGpuScopeSample final
{
    uint32_t NameId = 0;
    uint32_t Depth = 0;
    double BeginMs = 0.0;
    double EndMs = 0.0;
    double DurationMs = 0.0;
    bool PipelineValid = false;
    uint64_t PSInvocations = 0, VSInvocations = 0;
};

/* Main-thread animation evaluation joined to successful model submissions in
   the same frame. NotSubmitted does not by itself mean outside the frustum. */
struct FProfilerAnimationStats final
{
    uint64_t UpdateCalls = 0;
    uint64_t UpdatedModels = 0;
    uint64_t SubmittedUpdatedModels = 0;
    uint64_t NotSubmittedUpdatedModels = 0;
    uint64_t DroppedSamples = 0;
    double CpuMs = 0.0;
    double NotSubmittedCpuMs = 0.0;
};

struct FProfilerModelAnimationToken final
{
    uint64_t BeginTick = 0;
    uint64_t FrameNumber = 0;
};

// Secondary window presentation only. Main-thread capture; bounded per frame.
struct FProfilerViewportPresent final
{
    uint32_t ViewportId = 0;
    float X = 0, Y = 0, Width = 0, Height = 0;
    double CpuMs = 0;
    uint32_t SyncInterval = 0, Flags = 0;
    int32_t Result = 0;
};

struct FProfilerFrame final
{
    uint64_t FrameNumber = 0;
    // Dropped completions attributed to this frame; excludes deliberately disabled detail scopes.
    uint64_t DroppedCpuScopes = 0;
    bool DetailedCpuScopes = false;
    double CpuFrameMs = 0.0;
    double FrameIntervalMs = 0.0;
    FProfilerAnimationStats Animation{};
    std::array<FProfilerWorkStats, static_cast<size_t>(EProfilerWork::Count)> CpuWork{};
    std::vector<FProfilerViewportPresent> ViewportPresents;
    uint32_t DroppedViewportPresents = 0;
    double GpuFrameMs = 0.0;
    bool GpuValid = false;
    uint32_t GpuLatencyFrames = 0;
    EProfilerGpuFrameStatus GpuStatus = EProfilerGpuFrameStatus::Unsupported;
    bool GpuScopesSupported = false;
    uint32_t DroppedGpuScopes = 0;
    std::vector<FProfilerGpuScopeSample> GpuScopes;
    std::array<uint64_t, static_cast<size_t>(EProfilerCounter::Count)> Counters{};
    std::vector<FProfilerScopeSample> CpuScopes;
    D3D11_QUERY_DATA_PIPELINE_STATISTICS Pipeline{};
};

// Describes one newest-completed-frame export without copying any frame samples.
// Evicted frames are counted since Reset; excluded retained frames can still be saved.
struct FProfilerCaptureWindow final
{
    uint64_t RequestedFrames = 0;
    uint64_t RetainedFrames = 0;
    uint64_t SavedFrames = 0;
    uint64_t ExcludedRetainedFrames = 0;
    uint64_t EvictedFramesSinceReset = 0;
    uint64_t FirstRetainedFrameNumber = 0;
    uint64_t LastRetainedFrameNumber = 0;
    uint64_t FirstSavedFrameNumber = 0;
    uint64_t LastSavedFrameNumber = 0;
    double ExcludedMaxFrameIntervalMs = 0.0;
};

struct FProfilerCaptureSnapshot final
{
    std::vector<std::string> ScopeNames;
    std::vector<FProfilerFrame> Frames;
    FProfilerCaptureWindow CaptureWindow{};
    uint64_t DroppedCpuScopes = 0;
    uint64_t DroppedGpuFrames = 0;
    uint64_t DroppedGpuScopes = 0;
    uint64_t DroppedModelAnimationSamples = 0;
    bool GpuQueriesSupported = false;
    bool GpuScopesSupported = false;
    uint32_t MainThreadId = 0;
    uint64_t TicksPerSecond = 0;
};

struct FProfilerLiveStats final
{
    uint64_t TotalDroppedCpuScopes = 0;
    uint64_t TotalDroppedGpuFrames = 0;
    uint64_t TotalDroppedGpuScopes = 0;
    uint64_t TotalDroppedModelAnimationSamples = 0;
    uint64_t DroppedCpuScopes = 0;
    bool DetailedCpuScopes = false;
    uint64_t FrameNumber = 0;
    double CpuFrameMs = 0.0;
    double FrameIntervalMs = 0.0;
    FProfilerAnimationStats Animation{};
    std::array<FProfilerWorkStats, static_cast<size_t>(EProfilerWork::Count)> CpuWork{};
    std::array<uint64_t, static_cast<size_t>(EProfilerCounter::Count)> Counters{};

    EProfilerGpuFrameStatus LatestFrameGpuStatus = EProfilerGpuFrameStatus::Unsupported;
    uint64_t GpuFrameNumber = 0;
    double GpuFrameMs = 0.0;
    bool GpuValid = false;
    uint32_t GpuLatencyFrames = 0;
    bool GpuScopesSupported = false;
    uint32_t DroppedGpuScopes = 0;
    std::vector<FProfilerGpuScopeSample> GpuScopes;
    D3D11_QUERY_DATA_PIPELINE_STATISTICS Pipeline{};
};

/* One completed scope that took at least LONG_OPERATION_THRESHOLD_MS. It is
   kept in a small ring independent of frame history so a long JSON parse on
   the Loader thread stays visible after the frame it ended in scrolled out. */
struct FProfilerLongOperation final
{
    uint64_t Sequence = 0;
    uint64_t FrameNumber = 0;
    uint32_t NameId = 0;
    uint32_t ThreadId = 0;
    double DurationMs = 0.0;
};

/* Sum over a window of history frames for one (scope name, thread) pair.
   Self time excludes scopes nested inside it on the same thread. */
struct FProfilerScopeAggregate final
{
    uint32_t NameId = 0;
    uint32_t ThreadId = 0;
    uint64_t Calls = 0;
    double InclusiveMs = 0.0;
    double SelfMs = 0.0;
    // A dropped scope anywhere in the selected window makes self attribution incomplete.
    bool SelfComplete = true;
    double MaxMs = 0.0;
};

struct FProfilerGpuScopeAggregate final
{
    uint32_t NameId = 0;
    uint64_t Calls = 0;
    double InclusiveMs = 0.0;
    double MaxFrameMs = 0.0;
    double P95FrameMs = 0.0;
    uint64_t PipelineSamples = 0, PSInvocations = 0, VSInvocations = 0;
};

class ENGINE_DLL CProfiler final
{
public:
    static constexpr uint32_t GPU_QUERY_RING_SIZE = 8;
    static constexpr uint32_t GPU_READ_LATENCY = 4;
    static constexpr uint32_t MAX_GPU_SCOPES_PER_FRAME = 128;
    static constexpr uint32_t MAX_GPU_PIPELINE_SCOPES_PER_FRAME = 8;
    static constexpr size_t MAX_ANIMATION_MODELS_PER_FRAME = 16384;
    static constexpr size_t MAX_HISTORY_FRAMES = 1200;
    static constexpr size_t MAX_LONG_OPERATIONS = 256;
    static constexpr double LONG_OPERATION_THRESHOLD_MS = 8.0;

public:
    CProfiler();
    ~CProfiler() = default;
    HRESULT Initialize(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
    void Begin_Frame();
    void End_Frame();

    void Set_Enabled(bool enabled) noexcept;
    bool Is_Enabled() const noexcept;

    // Requested UI policy is applied at the next frame boundary.
    void Set_DetailedScopesEnabled(bool enabled) noexcept { m_RequestedDetailedScopes.store(enabled, std::memory_order_relaxed); }
    bool Is_DetailedScopesEnabled() const noexcept { return m_RequestedDetailedScopes.load(std::memory_order_relaxed); }
    bool Is_CollectingDetailedScopes() const noexcept { return m_DetailedScopes.load(std::memory_order_relaxed); }

    void Reset_History();

    /* Thread-safe. Nesting is tracked per thread, so a scope may begin and end
       on any thread and outside the main-thread frame boundaries. The token is
       only meaningful on the thread that began the scope. */
    uint32_t Register_ScopeName(std::string_view name) { return Intern_Name(name); }
    uint32_t Begin_Scope(std::string_view name);
    void End_Scope(uint32_t token) noexcept;

    /* Immediate-context main thread only; disabled/unsupported/overflow scopes
       return UINT32_MAX. No query creation or GPU wait occurs on this path. */
    uint32_t Begin_GpuScope(std::string_view name, bool collectPipeline = false);
    void End_GpuScope(uint32_t token) noexcept;

    // Main thread only. No raw samples, names, locks or allocation on this path.
    FProfilerWorkToken Begin_Work(EProfilerWork work) const noexcept;
    void End_Work(EProfilerWork work, FProfilerWorkToken token) noexcept;
    static const char* Get_WorkName(EProfilerWork work) noexcept;

    FProfilerModelAnimationToken Begin_ModelAnimation() const noexcept;
    void End_ModelAnimation(const void* model, FProfilerModelAnimationToken token);
    void Record_ModelSubmitted(const void* model);
    void Record_ViewportPresent(const FProfilerViewportPresent& sample);

    void Add_Counter(EProfilerCounter counter, uint64_t value = 1) noexcept;
    void Set_Counter(EProfilerCounter counter, uint64_t value) noexcept;

    FProfilerCaptureSnapshot Snapshot(size_t frameWindow = MAX_HISTORY_FRAMES) const;
    FProfilerCaptureWindow Get_CaptureWindow(size_t frameWindow) const;
    bool Get_LiveStats(FProfilerLiveStats& outStats) const;

    uint32_t Get_MainThreadId() const noexcept { return m_MainThreadId; }
    double Ticks_ToMs(uint64_t ticks) const noexcept;
    size_t Get_HistoryFrameCount() const;
    void Get_ScopeNames(std::vector<std::string>& outNames) const;
    /* Aggregates the most recent frameWindow history frames, sorted by
       inclusive time descending. */
    void Get_ScopeAggregates(
        size_t frameWindow,
        std::vector<FProfilerScopeAggregate>& outAggregates) const;
    /* Excludes incomplete GPU frames. Same-name calls are summed per frame;
       absent passes contribute zero to max/p95 and the valid-frame divisor. */
    void Get_GpuScopeAggregates(
        size_t frameWindow,
        std::vector<FProfilerGpuScopeAggregate>& outAggregates,
        size_t& outValidFrames,
        size_t& outPartialFrames) const;
    void Get_WindowFrameStats(
        size_t frameWindow,
        double& outCpuAvgMs,
        double& outCpuMaxMs,
        double& outGpuAvgMs,
        double& outGpuMaxMs,
        size_t& outFrames,
        size_t* outGpuValidFrames = nullptr) const;
    void Get_LongOperations(std::vector<FProfilerLongOperation>& outOperations) const;
    void Clear_LongOperations();

private:
    struct FGpuScopeQuery final
    {
        ComPtr<ID3D11Query> Begin;
        ComPtr<ID3D11Query> End;
        uint32_t Token = UINT32_MAX;
        uint32_t PipelineIndex = UINT32_MAX;
        uint32_t NameId = 0;
        uint32_t Depth = 0;
        bool Ended = false;
    };

    struct FGpuQuerySlot final
    {
        ComPtr<ID3D11Query> Disjoint;
        ComPtr<ID3D11Query> TimestampBegin;
        ComPtr<ID3D11Query> TimestampEnd;
        ComPtr<ID3D11Query> Pipeline;
        uint64_t FrameNumber = 0;
        uint64_t SubmittedPollFrame = 0;
        bool Pending = false;
        bool FrameEnded = false;
        uint32_t ScopeCount = 0;
        uint32_t PipelineScopeCount = 0;
        std::array<ComPtr<ID3D11Query>, MAX_GPU_PIPELINE_SCOPES_PER_FRAME> PassPipelines{};
        uint32_t OpenScopeCount = 0;
        std::array<uint32_t, MAX_GPU_SCOPES_PER_FRAME> OpenScopes{};
        std::array<FGpuScopeQuery, MAX_GPU_SCOPES_PER_FRAME> Scopes{};
    };

private:
    uint64_t Query_Tick() const noexcept;
    uint32_t Intern_Name(std::string_view name);
    bool Create_GpuQueries();
    void Begin_GpuFrame(uint64_t frameNumber);
    void End_GpuFrame(uint64_t frameNumber);
    void Resolve_GpuFrames(uint64_t currentPollFrame);
    void Commit_CurrentFrame();
    // The caller holds m_Mutex, keeping coverage and copied frames consistent.
    FProfilerCaptureWindow Get_CaptureWindowLocked(size_t frameWindow) const;

private:
    // Distinguishes a new profiler constructed at a previously used address.
    const uint64_t m_InstanceId;
    ComPtr<ID3D11Device> m_pDevice;
    ComPtr<ID3D11DeviceContext> m_pContext;
    LARGE_INTEGER m_Frequency{};
    // UI requests are latched at Begin_Frame so a capture never starts mid-frame.
    std::atomic_bool m_Enabled = false;
    std::atomic_bool m_Collecting = false;
    std::atomic_bool m_ResetRequested = false;
    std::atomic_uint64_t m_CaptureEpoch = 0;
    std::atomic_bool m_RequestedDetailedScopes = false;
    std::atomic_bool m_DetailedScopes = false;
    uint64_t m_FrameNumber = 0;
    uint64_t m_PollFrameNumber = 0;
    uint64_t m_FrameBeginTick = 0;
    uint64_t m_PreviousFrameBeginTick = 0;
    FProfilerFrame m_CurrentFrame{};
    std::array<std::atomic_uint64_t, static_cast<size_t>(EProfilerCounter::Count)> m_AtomicCounters{};
    std::array<FGpuQuerySlot, GPU_QUERY_RING_SIZE> m_GpuSlots{};
    bool m_GpuQueriesAvailable = false;
    bool m_GpuScopeQueriesAvailable = false;
    bool m_GpuPipelineQueriesAvailable = false;
    uint32_t m_ActiveGpuSlot = UINT32_MAX;
    uint32_t m_NextGpuScopeToken = 0;
    std::atomic_bool m_FrameActive = false;
    uint32_t m_MainThreadId = 0;
    mutable std::mutex m_Mutex;
    /* Completed scopes from every thread since the last End_Frame. */
    std::vector<FProfilerScopeSample> m_PendingScopes;
    std::deque<FProfilerLongOperation> m_LongOperations;
    uint64_t m_LongOperationSequence = 0;
    uint64_t m_SharedFrameNumber = 0;
    std::deque<FProfilerFrame> m_History;
    uint64_t m_EvictedHistoryFrames = 0;
    std::vector<std::string> m_ScopeNames;
    std::unordered_map<std::string, uint32_t> m_ScopeNameLookup;
    uint64_t m_DroppedCpuScopes = 0;
    uint64_t m_PendingDroppedCpuScopes = 0;
    uint64_t m_DroppedGpuFrames = 0;
    uint64_t m_DroppedGpuScopes = 0;
    uint64_t m_DroppedModelAnimationSamples = 0;
    struct FModelAnimationWork final
    {
        uint64_t Calls = 0;
        double CpuMs = 0.0;
    };
    std::unordered_map<const void*, FModelAnimationWork> m_ModelAnimationWork;
    std::unordered_set<const void*> m_SubmittedModels;
};

class ENGINE_DLL CProfilerScope final
{
public:
    CProfilerScope(CProfiler* profiler, std::string_view name)
        : m_pProfiler(profiler)
        , m_Token(profiler != nullptr ? profiler->Begin_Scope(name) : UINT32_MAX)
    {}

    ~CProfilerScope()
    {
        if (m_pProfiler != nullptr && m_Token != UINT32_MAX)
            m_pProfiler->End_Scope(m_Token);
    }

    CProfilerScope(const CProfilerScope&) = delete;
    CProfilerScope& operator=(const CProfilerScope&) = delete;

private:
    CProfiler* m_pProfiler = nullptr;
    uint32_t m_Token = UINT32_MAX;
};

// High-frequency per-draw CPU work is opt-in; pass timing and counters stay available.
class CProfilerDetailScope final
{
public:
    CProfilerDetailScope(CProfiler* profiler, std::string_view name)
        : m_Scope(profiler && profiler->Is_CollectingDetailedScopes() ? profiler : nullptr, name) {}
    CProfilerDetailScope(const CProfilerDetailScope&) = delete;
    CProfilerDetailScope& operator=(const CProfilerDetailScope&) = delete;
private:
    CProfilerScope m_Scope;
};

class CProfilerWorkScope final
{
public:
    CProfilerWorkScope(CProfiler* profiler, EProfilerWork work) noexcept
        : m_pProfiler(profiler), m_Work(work)
        , m_Token(profiler ? profiler->Begin_Work(work) : FProfilerWorkToken{}) {}
    ~CProfilerWorkScope()
    {
        if (m_pProfiler && m_Token.BeginTick != 0)
            m_pProfiler->End_Work(m_Work, m_Token);
    }
    CProfilerWorkScope(const CProfilerWorkScope&) = delete;
    CProfilerWorkScope& operator=(const CProfilerWorkScope&) = delete;
private:
    CProfiler* m_pProfiler = nullptr;
    EProfilerWork m_Work = EProfilerWork::Count;
    FProfilerWorkToken m_Token{};
};

class ENGINE_DLL CProfilerGpuScope final
{
public:
    CProfilerGpuScope(CProfiler* profiler, std::string_view name, bool collectPipeline = false)
        : m_pProfiler(profiler)
        , m_Token(profiler != nullptr ? profiler->Begin_GpuScope(name, collectPipeline) : UINT32_MAX)
    {}
    ~CProfilerGpuScope()
    {
        if (m_pProfiler != nullptr && m_Token != UINT32_MAX)
            m_pProfiler->End_GpuScope(m_Token);
    }
    CProfilerGpuScope(const CProfilerGpuScope&) = delete;
    CProfilerGpuScope& operator=(const CProfilerGpuScope&) = delete;
private:
    CProfiler* m_pProfiler = nullptr;
    uint32_t m_Token = UINT32_MAX;
};

class ENGINE_DLL CProfilerModelAnimationScope final
{
public:
    CProfilerModelAnimationScope(CProfiler* profiler, const void* model)
        : m_pProfiler(profiler), m_Model(model)
        , m_Token(profiler != nullptr ? profiler->Begin_ModelAnimation() : FProfilerModelAnimationToken{})
    {}
    ~CProfilerModelAnimationScope()
    {
        if (m_pProfiler != nullptr && m_Token.BeginTick != 0)
            m_pProfiler->End_ModelAnimation(m_Model, m_Token);
    }
    CProfilerModelAnimationScope(const CProfilerModelAnimationScope&) = delete;
    CProfilerModelAnimationScope& operator=(const CProfilerModelAnimationScope&) = delete;
private:
    CProfiler* m_pProfiler = nullptr;
    const void* m_Model = nullptr;
    FProfilerModelAnimationToken m_Token{};
};

NS_END
```

#### `Engine/Private/Profiler.cpp`

후보 SHA256: `cb102c663b498369802f283f22ec0d7df40b11482b61a6a412207b4983e8f74d`

```cpp
#include "Profiler.h"

#include <algorithm>
#include <cmath>
#include <exception>
#include <functional>
#include <limits>
#include <map>

using namespace Engine;

namespace
{
    constexpr size_t MAX_SCOPES_PER_FRAME = 8192;
    // Children complete first. Keep main-thread pass/root samples available
    // after dense map/particle detail exhausts its bounded frame budget.
    constexpr size_t MAIN_PASS_SCOPE_RESERVE = 1024;
    constexpr size_t MAIN_ROOT_SCOPE_RESERVE = 128;
    constexpr size_t MAX_OPEN_SCOPES_PER_THREAD = 64;

    uint64_t AllocateProfilerInstanceId()
    {
        static std::atomic_uint64_t nextId{1};
        const uint64_t id = nextId.fetch_add(1, std::memory_order_relaxed);
        if (id == 0) std::terminate(); // Never reuse a wrapped cache identity.
        return id;
    }

    struct FOpenScope final
    {
        uint32_t NameId;
        uint32_t Depth;
        uint64_t BeginTick;
        uint64_t CaptureEpoch;
    };

    /* Each thread owns its own nesting stack. A token is the index into this
       stack on the thread that began the scope.
       Kept trivially constructible (fixed array, no std::vector) on purpose: a thread_local
       with a dynamic initializer runs on every thread the process ever creates -- D3D driver,
       FMOD and PhysX workers included -- and the debug STL vector's heap-allocated container
       proxy then shows up in the CRT exit leak dump for every such thread still alive at
       shutdown. */
    struct FOpenScopeStack final
    {
        FOpenScope Scopes[MAX_OPEN_SCOPES_PER_THREAD];
        uint32_t Count;
    };
    thread_local FOpenScopeStack t_OpenScopes{};
}

CProfiler::CProfiler()
    : m_InstanceId(AllocateProfilerInstanceId())
{
}

HRESULT CProfiler::Initialize(
    ComPtr<ID3D11Device> device,
    ComPtr<ID3D11DeviceContext> context)
{
    if (nullptr == device || nullptr == context ||
        !QueryPerformanceFrequency(&m_Frequency) ||
        0 == m_Frequency.QuadPart)
        return E_INVALIDARG;

    m_pDevice = std::move(device);
    m_pContext = std::move(context);
    m_MainThreadId = GetCurrentThreadId();
    m_GpuQueriesAvailable = Create_GpuQueries();
    return S_OK;
}

void CProfiler::Begin_Frame()
{
    // Counts real main-loop frame boundaries even while capture is paused.
    // Query latency must not be synthesized by advancing the capture number.
    ++m_PollFrameNumber;
    if (m_ResetRequested.exchange(false, std::memory_order_relaxed))
        Reset_History();
    const bool enabled = m_Enabled.load(std::memory_order_relaxed);
    const bool previous = m_Collecting.exchange(enabled, std::memory_order_relaxed);
    if (previous != enabled)
    {
        m_CaptureEpoch.fetch_add(1, std::memory_order_relaxed);
        m_PreviousFrameBeginTick = 0;
        for (auto& counter : m_AtomicCounters) counter.store(0, std::memory_order_relaxed);
        std::lock_guard lock(m_Mutex);
        m_PendingScopes.clear();
        m_PendingDroppedCpuScopes = 0;
    }
    if (!enabled) return;

    m_FrameActive = true;
    ++m_FrameNumber;
    {
        std::lock_guard lock(m_Mutex);
        m_SharedFrameNumber = m_FrameNumber;
    }
    m_CurrentFrame = {};
    m_CurrentFrame.FrameNumber = m_FrameNumber;
    const bool detail = m_RequestedDetailedScopes.load(std::memory_order_relaxed);
    m_DetailedScopes.store(detail, std::memory_order_relaxed);
    m_CurrentFrame.DetailedCpuScopes = detail;
    m_CurrentFrame.GpuScopesSupported = m_GpuScopeQueriesAvailable;
    m_ModelAnimationWork.clear();
    m_SubmittedModels.clear();
    m_FrameBeginTick = Query_Tick();
    if (m_PreviousFrameBeginTick != 0 && m_FrameBeginTick >= m_PreviousFrameBeginTick)
        m_CurrentFrame.FrameIntervalMs = Ticks_ToMs(m_FrameBeginTick - m_PreviousFrameBeginTick);
    m_PreviousFrameBeginTick = m_FrameBeginTick;
    Begin_GpuFrame(m_FrameNumber);
}

void CProfiler::End_Frame()
{
    if (!m_FrameActive)
    {
        Resolve_GpuFrames(m_PollFrameNumber);
        return;
    }
    m_FrameActive = false;

    const uint64_t endTick = Query_Tick();
    m_CurrentFrame.CpuFrameMs =
        static_cast<double>(endTick - m_FrameBeginTick) * 1000.0 /
        static_cast<double>(m_Frequency.QuadPart);

    for (const auto& [model, work] : m_ModelAnimationWork)
    {
        FProfilerAnimationStats& stats = m_CurrentFrame.Animation;
        ++stats.UpdatedModels;
        stats.UpdateCalls += work.Calls;
        stats.CpuMs += work.CpuMs;
        if (m_SubmittedModels.find(model) != m_SubmittedModels.end())
            ++stats.SubmittedUpdatedModels;
        else
        {
            ++stats.NotSubmittedUpdatedModels;
            stats.NotSubmittedCpuMs += work.CpuMs;
        }
    }

    for (size_t index = 0; index < m_AtomicCounters.size(); ++index)
    {
        m_CurrentFrame.Counters[index] =
            m_AtomicCounters[index].exchange(0, std::memory_order_relaxed);
    }

    {
        /* Every scope that ended since the previous frame, on any thread,
           belongs to this frame. */
        std::lock_guard lock(m_Mutex);
        m_CurrentFrame.DroppedCpuScopes = m_PendingDroppedCpuScopes;
        m_PendingDroppedCpuScopes = 0;
        m_CurrentFrame.CpuScopes = std::move(m_PendingScopes);
        m_PendingScopes.clear();
        if (m_History.size() < MAX_HISTORY_FRAMES)
            m_PendingScopes.reserve(128);
    }

    End_GpuFrame(m_FrameNumber);
    Commit_CurrentFrame();
    Resolve_GpuFrames(m_PollFrameNumber);
}

void CProfiler::Set_Enabled(bool enabled) noexcept
{
    m_Enabled.store(enabled, std::memory_order_relaxed);
}

bool CProfiler::Is_Enabled() const noexcept
{
    return m_Enabled.load(std::memory_order_relaxed);
}

void CProfiler::Reset_History()
{
    if (m_FrameActive.load(std::memory_order_relaxed))
    {
        m_ResetRequested.store(true, std::memory_order_relaxed);
        return;
    }
    m_CaptureEpoch.fetch_add(1, std::memory_order_relaxed);
    std::lock_guard lock(m_Mutex);
    m_History.clear();
    m_EvictedHistoryFrames = 0;
    m_PendingScopes.clear();
    m_LongOperations.clear();
    m_DroppedCpuScopes = 0;
    m_PendingDroppedCpuScopes = 0;
    m_PreviousFrameBeginTick = 0;
    m_DroppedGpuFrames = 0;
    m_DroppedGpuScopes = 0;
    m_DroppedModelAnimationSamples = 0;
}

uint32_t CProfiler::Begin_Scope(std::string_view name)
{
    if (!m_Collecting.load(std::memory_order_relaxed))
        return UINT32_MAX;
    FOpenScopeStack& openScopes = t_OpenScopes;
    if (openScopes.Count >= MAX_OPEN_SCOPES_PER_THREAD)
    {
        std::lock_guard lock(m_Mutex);
        ++m_DroppedCpuScopes;
        ++m_PendingDroppedCpuScopes;
        return UINT32_MAX;
    }

    FOpenScope open{};
    open.NameId = Intern_Name(name);
    open.Depth = openScopes.Count;
    open.BeginTick = Query_Tick();
    open.CaptureEpoch = m_CaptureEpoch.load(std::memory_order_relaxed);
    openScopes.Scopes[openScopes.Count] = open;
    return openScopes.Count++;
}

void CProfiler::End_Scope(uint32_t token) noexcept
{
    FOpenScopeStack& openScopes = t_OpenScopes;
    if (token >= openScopes.Count)
        return;

    const uint64_t endTick = Query_Tick();
    const FOpenScope open = openScopes.Scopes[token];
    /* Unwinding to the token also closes any inner scope whose End_Scope was
       skipped, so a mismatched pair cannot corrupt later depths. */
    openScopes.Count = token;
    if (!m_Collecting.load(std::memory_order_relaxed) ||
        open.CaptureEpoch != m_CaptureEpoch.load(std::memory_order_relaxed))
        return;

    FProfilerScopeSample sample{};
    sample.NameId = open.NameId;
    sample.Depth = open.Depth;
    sample.ThreadId = GetCurrentThreadId();
    sample.BeginTick = open.BeginTick;
    sample.EndTick = endTick;
    const double durationMs = Ticks_ToMs(endTick - open.BeginTick);

    std::lock_guard lock(m_Mutex);
    // A worker may have waited while the frame owner reset the capture.
    if (!m_Collecting.load(std::memory_order_relaxed) ||
        open.CaptureEpoch != m_CaptureEpoch.load(std::memory_order_relaxed)) return;
    size_t sampleLimit = MAX_SCOPES_PER_FRAME - MAIN_PASS_SCOPE_RESERVE;
    if (sample.ThreadId == m_MainThreadId)
    {
        if (sample.Depth <= 2)
            sampleLimit = MAX_SCOPES_PER_FRAME;
        else if (sample.Depth == 3)
            sampleLimit = MAX_SCOPES_PER_FRAME - MAIN_ROOT_SCOPE_RESERVE;
    }
    if (m_PendingScopes.size() < sampleLimit)
    {
        // Cap vector growth as well as sample count; normal frames still grow
        // on demand and the history ring continues to recycle their buffers.
        if (m_PendingScopes.size() == m_PendingScopes.capacity())
            m_PendingScopes.reserve((std::min)(MAX_SCOPES_PER_FRAME,
                (std::max)(size_t{128}, m_PendingScopes.capacity() * 2)));
        m_PendingScopes.push_back(sample);
    }
    else
    {
        ++m_DroppedCpuScopes;
        ++m_PendingDroppedCpuScopes;
    }
    if (durationMs >= LONG_OPERATION_THRESHOLD_MS)
    {
        FProfilerLongOperation operation{};
        operation.Sequence = ++m_LongOperationSequence;
        operation.FrameNumber = m_SharedFrameNumber;
        operation.NameId = sample.NameId;
        operation.ThreadId = sample.ThreadId;
        operation.DurationMs = durationMs;
        m_LongOperations.push_back(operation);
        while (m_LongOperations.size() > MAX_LONG_OPERATIONS)
            m_LongOperations.pop_front();
    }
}

uint32_t CProfiler::Begin_GpuScope(std::string_view name, bool collectPipeline)
{
    if (GetCurrentThreadId() != m_MainThreadId ||
        !m_Collecting.load(std::memory_order_relaxed) ||
        !m_FrameActive || !m_GpuScopeQueriesAvailable ||
        m_ActiveGpuSlot == UINT32_MAX)
        return UINT32_MAX;
    FGpuQuerySlot& slot = m_GpuSlots[m_ActiveGpuSlot];
    if (slot.ScopeCount >= MAX_GPU_SCOPES_PER_FRAME)
    {
        ++m_CurrentFrame.DroppedGpuScopes;
        std::lock_guard lock(m_Mutex);
        ++m_DroppedGpuScopes;
        return UINT32_MAX;
    }
    const uint32_t index = slot.ScopeCount++;
    FGpuScopeQuery& scope = slot.Scopes[index];
    scope.NameId = Intern_Name(name);
    scope.Depth = slot.OpenScopeCount;
    scope.Ended = false;
    scope.PipelineIndex = UINT32_MAX;
    if (collectPipeline && m_GpuPipelineQueriesAvailable &&
        slot.PipelineScopeCount < MAX_GPU_PIPELINE_SCOPES_PER_FRAME)
    {
        scope.PipelineIndex = slot.PipelineScopeCount++;
        m_pContext->Begin(slot.PassPipelines[scope.PipelineIndex].Get());
    }
    if (m_NextGpuScopeToken == UINT32_MAX)
        m_NextGpuScopeToken = 0;
    scope.Token = m_NextGpuScopeToken++;
    slot.OpenScopes[slot.OpenScopeCount++] = index;
    m_pContext->End(scope.Begin.Get());
    return scope.Token;
}

void CProfiler::End_GpuScope(uint32_t token) noexcept
{
    if (GetCurrentThreadId() != m_MainThreadId ||
        token == UINT32_MAX || m_ActiveGpuSlot == UINT32_MAX)
        return;
    FGpuQuerySlot& slot = m_GpuSlots[m_ActiveGpuSlot];
    for (uint32_t position = slot.OpenScopeCount; position > 0; --position)
    {
        const uint32_t index = slot.OpenScopes[position - 1];
        if (slot.Scopes[index].Token != token)
            continue;
        while (slot.OpenScopeCount >= position)
        {
            FGpuScopeQuery& scope = slot.Scopes[slot.OpenScopes[--slot.OpenScopeCount]];
            m_pContext->End(scope.End.Get());
            if (scope.PipelineIndex != UINT32_MAX)
                m_pContext->End(slot.PassPipelines[scope.PipelineIndex].Get());
            scope.Ended = true;
        }
        return;
    }
}

FProfilerWorkToken CProfiler::Begin_Work(EProfilerWork work) const noexcept
{
    if (!m_Collecting.load(std::memory_order_relaxed) ||
        !m_FrameActive.load(std::memory_order_relaxed) ||
        GetCurrentThreadId() != m_MainThreadId ||
        static_cast<size_t>(work) >= static_cast<size_t>(EProfilerWork::Count))
        return {};
    return {Query_Tick(), m_FrameNumber,
        m_CaptureEpoch.load(std::memory_order_relaxed), m_InstanceId};
}

void CProfiler::End_Work(EProfilerWork work, FProfilerWorkToken token) noexcept
{
    if (token.BeginTick == 0 || !m_Collecting.load(std::memory_order_relaxed) ||
        !m_FrameActive.load(std::memory_order_relaxed) ||
        GetCurrentThreadId() != m_MainThreadId || token.InstanceId != m_InstanceId ||
        token.FrameNumber != m_FrameNumber ||
        token.CaptureEpoch != m_CaptureEpoch.load(std::memory_order_relaxed))
        return;
    const size_t index = static_cast<size_t>(work);
    if (index >= m_CurrentFrame.CpuWork.size()) return;
    const uint64_t endTick = Query_Tick();
    if (endTick < token.BeginTick) return;
    auto& stats = m_CurrentFrame.CpuWork[index];
    ++stats.Calls;
    stats.CpuMs += Ticks_ToMs(endTick - token.BeginTick);
}

const char* CProfiler::Get_WorkName(EProfilerWork work) noexcept
{
    static constexpr const char* names[] = {
        "Map.Batch.Render", "Map.Batch.Visibility", "Map.Batch.Material",
        "Map.Batch.Pass", "Map.Batch.Draw", "Map.Object.Render", "Map.Water.Render",
        "Npc.Update", "Npc.LateUpdate", "Npc.Render",
        "Ambient.Visibility", "Ambient.Advance", "Ambient.Submit"
    };
    static_assert(std::size(names) == static_cast<size_t>(EProfilerWork::Count));
    const size_t index = static_cast<size_t>(work);
    return index < std::size(names) ? names[index] : "<invalid>";
}

FProfilerModelAnimationToken CProfiler::Begin_ModelAnimation() const noexcept
{
    if (GetCurrentThreadId() != m_MainThreadId || !m_FrameActive ||
        !m_Collecting.load(std::memory_order_relaxed))
        return {};
    return {Query_Tick(), m_FrameNumber};
}

void CProfiler::End_ModelAnimation(const void* model, FProfilerModelAnimationToken token)
{
    if (!model || token.BeginTick == 0 || GetCurrentThreadId() != m_MainThreadId)
        return;
    if (!m_FrameActive || token.FrameNumber != m_FrameNumber)
    {
        std::lock_guard lock(m_Mutex);
        ++m_DroppedModelAnimationSamples;
        return;
    }
    const uint64_t end = Query_Tick();
    if (end < token.BeginTick)
        return;
    auto found = m_ModelAnimationWork.find(model);
    if (found == m_ModelAnimationWork.end())
    {
        if (m_ModelAnimationWork.size() >= MAX_ANIMATION_MODELS_PER_FRAME)
        {
            ++m_CurrentFrame.Animation.DroppedSamples;
            std::lock_guard lock(m_Mutex);
            ++m_DroppedModelAnimationSamples;
            return;
        }
        found = m_ModelAnimationWork.emplace(model, FModelAnimationWork{}).first;
    }
    FModelAnimationWork& work = found->second;
    ++work.Calls;
    work.CpuMs += Ticks_ToMs(end - token.BeginTick);
}

void CProfiler::Record_ModelSubmitted(const void* model)
{
    if (!model || GetCurrentThreadId() != m_MainThreadId || !m_FrameActive ||
        !m_Collecting.load(std::memory_order_relaxed) ||
        m_SubmittedModels.find(model) != m_SubmittedModels.end())
        return;
    if (m_SubmittedModels.size() >= MAX_ANIMATION_MODELS_PER_FRAME)
    {
        ++m_CurrentFrame.Animation.DroppedSamples;
        std::lock_guard lock(m_Mutex);
        ++m_DroppedModelAnimationSamples;
        return;
    }
    m_SubmittedModels.insert(model);
}

void CProfiler::Record_ViewportPresent(const FProfilerViewportPresent& sample)
{
    if (GetCurrentThreadId() != m_MainThreadId || !m_FrameActive ||
        !m_Collecting.load(std::memory_order_relaxed)) return;
    Add_Counter(EProfilerCounter::ImGuiPresentAttempts);
    if (sample.Result == DXGI_ERROR_WAS_STILL_DRAWING)
        Add_Counter(EProfilerCounter::ImGuiPresentBusy);
    else if (FAILED(sample.Result)) Add_Counter(EProfilerCounter::ImGuiPresentFailures);
    else if (sample.Result == DXGI_STATUS_OCCLUDED) Add_Counter(EProfilerCounter::ImGuiPresentOccluded);
    if (m_CurrentFrame.ViewportPresents.size() >= 32)
    {
        ++m_CurrentFrame.DroppedViewportPresents;
        return;
    }
    m_CurrentFrame.ViewportPresents.push_back(sample);
}

void CProfiler::Add_Counter(
    EProfilerCounter counter, uint64_t value) noexcept
{
    if (!m_Collecting.load(std::memory_order_relaxed) || !m_FrameActive)
        return;
    const size_t index = static_cast<size_t>(counter);
    if (index < m_AtomicCounters.size())
        m_AtomicCounters[index].fetch_add(value, std::memory_order_relaxed);
}

void CProfiler::Set_Counter(
    EProfilerCounter counter, uint64_t value) noexcept
{
    if (!m_Collecting.load(std::memory_order_relaxed) || !m_FrameActive)
        return;
    const size_t index = static_cast<size_t>(counter);
    if (index < m_AtomicCounters.size())
        m_AtomicCounters[index].store(value, std::memory_order_relaxed);
}

FProfilerCaptureSnapshot CProfiler::Snapshot(size_t frameWindow) const
{
    std::lock_guard lock(m_Mutex);
    FProfilerCaptureSnapshot snapshot{};
    snapshot.ScopeNames = m_ScopeNames;
    snapshot.CaptureWindow = Get_CaptureWindowLocked(frameWindow);
    const size_t count = (std::min)(frameWindow, m_History.size());
    snapshot.Frames.assign(m_History.end() - count, m_History.end());
    snapshot.DroppedCpuScopes = m_DroppedCpuScopes;
    snapshot.DroppedGpuFrames = m_DroppedGpuFrames;
    snapshot.DroppedGpuScopes = m_DroppedGpuScopes;
    snapshot.DroppedModelAnimationSamples = m_DroppedModelAnimationSamples;
    snapshot.GpuQueriesSupported = m_GpuQueriesAvailable;
    snapshot.GpuScopesSupported = m_GpuScopeQueriesAvailable;
    snapshot.MainThreadId = m_MainThreadId;
    snapshot.TicksPerSecond = static_cast<uint64_t>(m_Frequency.QuadPart);
    return snapshot;
}

FProfilerCaptureWindow CProfiler::Get_CaptureWindow(size_t frameWindow) const
{
    std::lock_guard lock(m_Mutex);
    return Get_CaptureWindowLocked(frameWindow);
}

FProfilerCaptureWindow CProfiler::Get_CaptureWindowLocked(size_t frameWindow) const
{
    FProfilerCaptureWindow window{};
    window.RequestedFrames = static_cast<uint64_t>(frameWindow);
    window.RetainedFrames = static_cast<uint64_t>(m_History.size());
    const size_t count = (std::min)(frameWindow, m_History.size());
    const size_t excluded = m_History.size() - count;
    window.SavedFrames = static_cast<uint64_t>(count);
    window.ExcludedRetainedFrames = static_cast<uint64_t>(excluded);
    window.EvictedFramesSinceReset = m_EvictedHistoryFrames;
    if (!m_History.empty())
    {
        window.FirstRetainedFrameNumber = m_History.front().FrameNumber;
        window.LastRetainedFrameNumber = m_History.back().FrameNumber;
    }
    if (count != 0)
    {
        window.FirstSavedFrameNumber = m_History[excluded].FrameNumber;
        window.LastSavedFrameNumber = m_History.back().FrameNumber;
    }
    for (size_t index = 0; index < excluded; ++index)
    {
        const double interval = m_History[index].FrameIntervalMs;
        if (std::isfinite(interval))
            window.ExcludedMaxFrameIntervalMs = (std::max)(window.ExcludedMaxFrameIntervalMs, interval);
    }
    return window;
}

bool CProfiler::Get_LiveStats(FProfilerLiveStats& outStats) const
{
    std::lock_guard lock(m_Mutex);
    outStats = {};
    outStats.TotalDroppedCpuScopes = m_DroppedCpuScopes;
    outStats.TotalDroppedGpuFrames = m_DroppedGpuFrames;
    outStats.TotalDroppedGpuScopes = m_DroppedGpuScopes;
    outStats.TotalDroppedModelAnimationSamples = m_DroppedModelAnimationSamples;
    if (m_History.empty())
        return false;

    const FProfilerFrame& latest = m_History.back();
    outStats.FrameNumber = latest.FrameNumber;
    outStats.DroppedCpuScopes = latest.DroppedCpuScopes;
    outStats.DetailedCpuScopes = latest.DetailedCpuScopes;
    outStats.CpuFrameMs = latest.CpuFrameMs;
    outStats.FrameIntervalMs = latest.FrameIntervalMs;
    outStats.Animation = latest.Animation;
    outStats.CpuWork = latest.CpuWork;
    outStats.Counters = latest.Counters;
    outStats.LatestFrameGpuStatus = latest.GpuStatus;
    outStats.GpuScopesSupported = m_GpuScopeQueriesAvailable;

    const auto gpuFrame = std::find_if(
        m_History.rbegin(), m_History.rend(),
        [](const FProfilerFrame& frame)
        { return frame.GpuValid; });
    if (gpuFrame != m_History.rend())
    {
        outStats.GpuFrameNumber = gpuFrame->FrameNumber;
        outStats.GpuFrameMs = gpuFrame->GpuFrameMs;
        outStats.GpuValid = true;
        outStats.GpuLatencyFrames = gpuFrame->GpuLatencyFrames;
        outStats.Pipeline = gpuFrame->Pipeline;
        outStats.GpuScopes = gpuFrame->GpuScopes;
        outStats.DroppedGpuScopes = gpuFrame->DroppedGpuScopes;
    }

    return true;
}

double CProfiler::Ticks_ToMs(uint64_t ticks) const noexcept
{
    if (0 == m_Frequency.QuadPart)
        return 0.0;
    return static_cast<double>(ticks) * 1000.0 /
        static_cast<double>(m_Frequency.QuadPart);
}

size_t CProfiler::Get_HistoryFrameCount() const
{
    std::lock_guard lock(m_Mutex);
    return m_History.size();
}

void CProfiler::Get_ScopeNames(std::vector<std::string>& outNames) const
{
    std::lock_guard lock(m_Mutex);
    outNames = m_ScopeNames;
}

void CProfiler::Get_ScopeAggregates(
    size_t frameWindow,
    std::vector<FProfilerScopeAggregate>& outAggregates) const
{
    outAggregates.clear();
    std::lock_guard lock(m_Mutex);
    if (m_History.empty() || 0 == frameWindow)
        return;

    // End_Scope appends in completion order on each thread. Reduce completed
    // children when their parent arrives instead of sorting every raw frame
    // again on every panel refresh. Scope IDs are dense, interned indices.
    struct FThreadReduction final
    {
        uint32_t ThreadId = 0;
        std::vector<FProfilerScopeAggregate> ByName;
        std::vector<size_t> Completed;
        size_t CompletedCount = 0;
    };
    std::vector<FThreadReduction> threads;
    threads.reserve(4);
    size_t lastThread = 0;
    const auto threadFor = [&](uint32_t id) -> FThreadReduction&
    {
        if (lastThread < threads.size() && threads[lastThread].ThreadId == id)
            return threads[lastThread];
        for (size_t i = 0; i < threads.size(); ++i)
            if (threads[i].ThreadId == id)
            {
                lastThread = i;
                return threads[i];
            }
        lastThread = threads.size();
        threads.emplace_back();
        FThreadReduction& added = threads.back();
        added.ThreadId = id;
        added.ByName.resize(m_ScopeNames.size());
        return added;
    };

    const size_t frameCount = (std::min)(frameWindow, m_History.size());
    bool selfComplete = true;
    for (size_t frameIndex = m_History.size() - frameCount;
        frameIndex < m_History.size(); ++frameIndex)
    {
        selfComplete = selfComplete && m_History[frameIndex].DroppedCpuScopes == 0;
        for (FThreadReduction& thread : threads) thread.CompletedCount = 0;
        const auto& frameScopes = m_History[frameIndex].CpuScopes;
        const size_t scopeCount = frameScopes.size();
        const FProfilerScopeSample* scopes = frameScopes.data();
        for (size_t index = 0; index < scopeCount; ++index)
        {
            const FProfilerScopeSample& sample = scopes[index];
            if (sample.NameId >= m_ScopeNames.size()) continue;
            FThreadReduction& thread = threadFor(sample.ThreadId);
            // There is at most one push per input sample. Allocate the bounded
            // stack once and avoid Debug STL container churn for every scope.
            if (thread.Completed.size() < scopeCount) thread.Completed.resize(scopeCount);
            size_t* completed = thread.Completed.data();
            const double inclusiveMs = Ticks_ToMs(sample.EndTick >= sample.BeginTick ?
                sample.EndTick - sample.BeginTick : 0);
            double selfMs = inclusiveMs;
            while (thread.CompletedCount != 0)
            {
                const FProfilerScopeSample& child = scopes[completed[thread.CompletedCount - 1]];
                if (child.Depth <= sample.Depth) break;
                // An orphan from an earlier interval is not this scope's child.
                if (child.BeginTick >= sample.BeginTick && child.EndTick <= sample.EndTick)
                    selfMs -= Ticks_ToMs(child.EndTick >= child.BeginTick ?
                        child.EndTick - child.BeginTick : 0);
                --thread.CompletedCount;
            }
            completed[thread.CompletedCount++] = index;

            FProfilerScopeAggregate& aggregate = thread.ByName.data()[sample.NameId];
            aggregate.NameId = sample.NameId;
            aggregate.ThreadId = sample.ThreadId;
            ++aggregate.Calls;
            aggregate.InclusiveMs += inclusiveMs;
            aggregate.SelfMs += (std::max)(0.0, selfMs);
            aggregate.MaxMs = (std::max)(aggregate.MaxMs, inclusiveMs);
        }
    }
    for (const FThreadReduction& thread : threads)
        for (FProfilerScopeAggregate aggregate : thread.ByName)
            if (aggregate.Calls != 0)
            {
                // Even a name absent from an incomplete frame may have lost calls.
                aggregate.SelfComplete = selfComplete;
                outAggregates.push_back(aggregate);
            }
    std::sort(outAggregates.begin(), outAggregates.end(),
        [](const FProfilerScopeAggregate& left, const FProfilerScopeAggregate& right)
        { return left.InclusiveMs > right.InclusiveMs; });
}

void CProfiler::Get_GpuScopeAggregates(
    size_t frameWindow,
    std::vector<FProfilerGpuScopeAggregate>& outAggregates,
    size_t& outValidFrames,
    size_t& outPartialFrames) const
{
    std::lock_guard lock(m_Mutex);
    outAggregates.clear();
    outValidFrames = 0;
    outPartialFrames = 0;
    const size_t count = std::min(frameWindow, m_History.size());
    struct FAccumulated final
    {
        FProfilerGpuScopeAggregate Aggregate;
        std::vector<double> FrameTimes;
    };
    std::map<uint32_t, FAccumulated> accumulated;
    for (size_t offset = m_History.size() - count; offset < m_History.size(); ++offset)
    {
        const FProfilerFrame& frame = m_History[offset];
        if (!frame.GpuValid || !frame.GpuScopesSupported)
            continue;
        if (frame.DroppedGpuScopes != 0)
        {
            ++outPartialFrames;
            continue;
        }
        const size_t frameIndex = outValidFrames++;
        for (const FProfilerGpuScopeSample& sample : frame.GpuScopes)
        {
            FAccumulated& value = accumulated[sample.NameId];
            value.Aggregate.NameId = sample.NameId;
            ++value.Aggregate.Calls;
            value.Aggregate.InclusiveMs += sample.DurationMs;
            if (sample.PipelineValid)
            {
                ++value.Aggregate.PipelineSamples;
                value.Aggregate.PSInvocations += sample.PSInvocations;
                value.Aggregate.VSInvocations += sample.VSInvocations;
            }
            value.FrameTimes.resize(frameIndex + 1, 0.0);
            value.FrameTimes[frameIndex] += sample.DurationMs;
        }
    }
    for (auto& [name, value] : accumulated)
    {
        value.FrameTimes.resize(outValidFrames, 0.0);
        std::sort(value.FrameTimes.begin(), value.FrameTimes.end());
        value.Aggregate.MaxFrameMs = value.FrameTimes.back();
        const size_t percentile = static_cast<size_t>(
            std::ceil(static_cast<double>(outValidFrames) * 0.95)) - 1;
        value.Aggregate.P95FrameMs = value.FrameTimes[percentile];
        outAggregates.push_back(value.Aggregate);
    }
    std::sort(outAggregates.begin(), outAggregates.end(),
        [](const FProfilerGpuScopeAggregate& left, const FProfilerGpuScopeAggregate& right)
        { return left.InclusiveMs > right.InclusiveMs; });
}

void CProfiler::Get_WindowFrameStats(
    size_t frameWindow,
    double& outCpuAvgMs,
    double& outCpuMaxMs,
    double& outGpuAvgMs,
    double& outGpuMaxMs,
    size_t& outFrames,
    size_t* outGpuValidFrames) const
{
    outCpuAvgMs = 0.0;
    outCpuMaxMs = 0.0;
    outGpuAvgMs = 0.0;
    outGpuMaxMs = 0.0;
    outFrames = 0;
    if (outGpuValidFrames)
        *outGpuValidFrames = 0;
    std::lock_guard lock(m_Mutex);
    if (m_History.empty() || 0 == frameWindow)
        return;
    const size_t frameCount = (std::min)(frameWindow, m_History.size());
    size_t gpuFrames = 0;
    for (size_t frameIndex = m_History.size() - frameCount;
        frameIndex < m_History.size(); ++frameIndex)
    {
        const FProfilerFrame& frame = m_History[frameIndex];
        outCpuAvgMs += frame.CpuFrameMs;
        outCpuMaxMs = (std::max)(outCpuMaxMs, frame.CpuFrameMs);
        if (frame.GpuValid)
        {
            outGpuAvgMs += frame.GpuFrameMs;
            outGpuMaxMs = (std::max)(outGpuMaxMs, frame.GpuFrameMs);
            ++gpuFrames;
        }
    }
    outFrames = frameCount;
    if (outGpuValidFrames)
        *outGpuValidFrames = gpuFrames;
    outCpuAvgMs /= static_cast<double>(frameCount);
    if (0 != gpuFrames)
        outGpuAvgMs /= static_cast<double>(gpuFrames);
}

void CProfiler::Get_LongOperations(
    std::vector<FProfilerLongOperation>& outOperations) const
{
    std::lock_guard lock(m_Mutex);
    outOperations.assign(m_LongOperations.begin(), m_LongOperations.end());
}

void CProfiler::Clear_LongOperations()
{
    std::lock_guard lock(m_Mutex);
    m_LongOperations.clear();
}

uint64_t CProfiler::Query_Tick() const noexcept
{
    LARGE_INTEGER value{};
    QueryPerformanceCounter(&value);
    return static_cast<uint64_t>(value.QuadPart);
}

uint32_t CProfiler::Intern_Name(std::string_view name)
{
    struct FNameHash final
    {
        using is_transparent = void;
        size_t operator()(std::string_view value) const noexcept
        {
            return std::hash<std::string_view>{}(value);
        }
    };
    struct FThreadNameCache final
    {
        uint64_t InstanceId = 0;
        std::unordered_map<std::string, uint32_t, FNameHash, std::equal_to<>> Names;
    };
    // Function-local TLS allocates only on threads that actually register a scope.
    static thread_local FThreadNameCache threadCache;
    if (threadCache.InstanceId != m_InstanceId)
    {
        threadCache.Names.clear();
        threadCache.InstanceId = m_InstanceId;
    }
    const auto cached = threadCache.Names.find(name);
    if (cached != threadCache.Names.end()) return cached->second;

    uint32_t id;
    {
        const std::string key(name);
        std::lock_guard lock(m_Mutex);
        const auto found = m_ScopeNameLookup.find(key);
        if (found != m_ScopeNameLookup.end()) id = found->second;
        else
        {
            id = static_cast<uint32_t>(m_ScopeNames.size());
            m_ScopeNames.push_back(key);
            m_ScopeNameLookup.emplace(m_ScopeNames.back(), id);
        }
    }
    // Dynamic names cannot grow one thread's cache without bound. Global IDs
    // survive this eviction and Reset_History, so a later miss remains stable.
    if (threadCache.Names.size() >= 512u) threadCache.Names.clear();
    threadCache.Names.emplace(std::string(name), id);
    return id;
}

bool CProfiler::Create_GpuQueries()
{
    D3D11_QUERY_DESC desc{};
    for (FGpuQuerySlot& slot : m_GpuSlots)
    {
        desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
        if (FAILED(m_pDevice->CreateQuery(&desc, &slot.Disjoint)))
            return false;
        desc.Query = D3D11_QUERY_TIMESTAMP;
        if (FAILED(m_pDevice->CreateQuery(&desc, &slot.TimestampBegin)) ||
            FAILED(m_pDevice->CreateQuery(&desc, &slot.TimestampEnd)))
            return false;
        desc.Query = D3D11_QUERY_PIPELINE_STATISTICS;
        if (FAILED(m_pDevice->CreateQuery(&desc, &slot.Pipeline)))
            return false;
    }
    // Pass-query allocation may fail without disabling full-frame GPU timing.
    desc.Query = D3D11_QUERY_TIMESTAMP;
    m_GpuScopeQueriesAvailable = true;
    for (FGpuQuerySlot& slot : m_GpuSlots)
    {
        for (FGpuScopeQuery& scope : slot.Scopes)
        {
            if (FAILED(m_pDevice->CreateQuery(&desc, &scope.Begin)) ||
                FAILED(m_pDevice->CreateQuery(&desc, &scope.End)))
            {
                m_GpuScopeQueriesAvailable = false;
                break;
            }
        }
        if (!m_GpuScopeQueriesAvailable)
            break;
    }
    if (!m_GpuScopeQueriesAvailable)
    {
        for (FGpuQuerySlot& slot : m_GpuSlots)
            for (FGpuScopeQuery& scope : slot.Scopes)
            {
                scope.Begin.Reset();
                scope.End.Reset();
            }
    }
    // Only the selected leaf passes need pipeline counts, not all 128 scopes.
    desc.Query = D3D11_QUERY_PIPELINE_STATISTICS;
    m_GpuPipelineQueriesAvailable = m_GpuScopeQueriesAvailable;
    for (FGpuQuerySlot& slot : m_GpuSlots)
    {
        if (!m_GpuPipelineQueriesAvailable) break;
        for (auto& query : slot.PassPipelines)
            if (FAILED(m_pDevice->CreateQuery(&desc, &query)))
            { m_GpuPipelineQueriesAvailable = false; break; }
    }
    if (!m_GpuPipelineQueriesAvailable)
        for (FGpuQuerySlot& slot : m_GpuSlots)
            for (auto& query : slot.PassPipelines) query.Reset();
    return true;
}

void CProfiler::Begin_GpuFrame(uint64_t frameNumber)
{
    m_ActiveGpuSlot = UINT32_MAX;
    if (!m_GpuQueriesAvailable)
        return;
    const uint32_t index = static_cast<uint32_t>(frameNumber % GPU_QUERY_RING_SIZE);
    FGpuQuerySlot& slot = m_GpuSlots[index];
    if (slot.Pending)
    {
        m_CurrentFrame.GpuStatus = EProfilerGpuFrameStatus::Dropped;
        std::lock_guard lock(m_Mutex);
        ++m_DroppedGpuFrames;
        return;
    }
    slot.FrameNumber = frameNumber;
    slot.SubmittedPollFrame = m_PollFrameNumber;
    slot.Pending = true;
    slot.FrameEnded = false;
    slot.ScopeCount = 0;
    slot.PipelineScopeCount = 0;
    slot.OpenScopeCount = 0;
    m_ActiveGpuSlot = index;
    m_CurrentFrame.GpuStatus = EProfilerGpuFrameStatus::Pending;
    m_pContext->Begin(slot.Disjoint.Get());
    m_pContext->Begin(slot.Pipeline.Get());
    m_pContext->End(slot.TimestampBegin.Get());
}

void CProfiler::End_GpuFrame(uint64_t frameNumber)
{
    if (m_ActiveGpuSlot == UINT32_MAX)
        return;
    FGpuQuerySlot& slot = m_GpuSlots[m_ActiveGpuSlot];
    if (!slot.Pending || slot.FrameNumber != frameNumber)
        return;
    if (slot.OpenScopeCount != 0)
    {
        const uint32_t dropped = slot.OpenScopeCount;
        // Complete outstanding query commands, but omit truncated intervals.
        while (slot.OpenScopeCount != 0)
        {
            const auto& scope = slot.Scopes[slot.OpenScopes[--slot.OpenScopeCount]];
            m_pContext->End(scope.End.Get());
            if (scope.PipelineIndex != UINT32_MAX)
                m_pContext->End(slot.PassPipelines[scope.PipelineIndex].Get());
        }
        m_CurrentFrame.DroppedGpuScopes += dropped;
        std::lock_guard lock(m_Mutex);
        m_DroppedGpuScopes += dropped;
    }
    m_pContext->End(slot.TimestampEnd.Get());
    m_pContext->End(slot.Pipeline.Get());
    m_pContext->End(slot.Disjoint.Get());
    slot.FrameEnded = true;
    m_ActiveGpuSlot = UINT32_MAX;
}

void CProfiler::Resolve_GpuFrames(uint64_t currentPollFrame)
{
    if (!m_GpuQueriesAvailable || currentPollFrame <= GPU_READ_LATENCY)
        return;
    constexpr uint32_t flags = D3D11_ASYNC_GETDATA_DONOTFLUSH;
    for (FGpuQuerySlot& slot : m_GpuSlots)
    {
        if (!slot.Pending || !slot.FrameEnded ||
            currentPollFrame < slot.SubmittedPollFrame + GPU_READ_LATENCY)
            continue;
        D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjoint{};
        uint64_t begin = 0;
        uint64_t end = 0;
        D3D11_QUERY_DATA_PIPELINE_STATISTICS pipeline{};
        EProfilerGpuFrameStatus status = EProfilerGpuFrameStatus::Valid;
        bool ready = true;
        const auto read = [&](ID3D11Query* query, void* data, UINT size)
        {
            const HRESULT result = m_pContext->GetData(query, data, size, flags);
            if (FAILED(result))
                status = EProfilerGpuFrameStatus::Error;
            else if (result != S_OK)
                ready = false;
        };
        read(slot.Disjoint.Get(), &disjoint, sizeof(disjoint));
        read(slot.TimestampBegin.Get(), &begin, sizeof(begin));
        read(slot.TimestampEnd.Get(), &end, sizeof(end));
        read(slot.Pipeline.Get(), &pipeline, sizeof(pipeline));
        if (status != EProfilerGpuFrameStatus::Error && !ready)
            continue;
        if (status != EProfilerGpuFrameStatus::Error &&
            (disjoint.Disjoint || disjoint.Frequency == 0 || end < begin))
            status = EProfilerGpuFrameStatus::Disjoint;
        std::array<FProfilerGpuScopeSample, MAX_GPU_SCOPES_PER_FRAME> samples{};
        uint32_t sampleCount = 0;
        if (status == EProfilerGpuFrameStatus::Valid)
        {
            for (uint32_t index = 0; index < slot.ScopeCount; ++index)
            {
                const FGpuScopeQuery& scope = slot.Scopes[index];
                if (!scope.Ended)
                    continue;
                uint64_t scopeBegin = 0;
                uint64_t scopeEnd = 0;
                read(scope.Begin.Get(), &scopeBegin, sizeof(scopeBegin));
                read(scope.End.Get(), &scopeEnd, sizeof(scopeEnd));
                if (!ready || status == EProfilerGpuFrameStatus::Error)
                    break;
                if (scopeBegin < begin || scopeEnd < scopeBegin || scopeEnd > end)
                {
                    status = EProfilerGpuFrameStatus::Error;
                    break;
                }
                FProfilerGpuScopeSample& sample = samples[sampleCount++];
                sample.NameId = scope.NameId;
                sample.Depth = scope.Depth;
                const double scale = 1000.0 / static_cast<double>(disjoint.Frequency);
                sample.BeginMs = static_cast<double>(scopeBegin - begin) * scale;
                sample.EndMs = static_cast<double>(scopeEnd - begin) * scale;
                sample.DurationMs = static_cast<double>(scopeEnd - scopeBegin) * scale;
                if (scope.PipelineIndex != UINT32_MAX)
                {
                    D3D11_QUERY_DATA_PIPELINE_STATISTICS passPipeline{};
                    const HRESULT result = m_pContext->GetData(
                        slot.PassPipelines[scope.PipelineIndex].Get(),
                        &passPipeline, sizeof(passPipeline), flags);
                    if (result == S_FALSE) { ready = false; break; }
                    sample.PipelineValid = result == S_OK;
                    if (sample.PipelineValid)
                    {
                        sample.PSInvocations = passPipeline.PSInvocations;
                        sample.VSInvocations = passPipeline.VSInvocations;
                    }
                }
            }
            if (status != EProfilerGpuFrameStatus::Error && !ready)
                continue;
        }
        std::lock_guard lock(m_Mutex);
        const auto frame = std::find_if(m_History.rbegin(), m_History.rend(),
            [&slot](const FProfilerFrame& value)
            { return value.FrameNumber == slot.FrameNumber; });
        if (frame != m_History.rend())
        {
            frame->GpuStatus = status;
            frame->GpuLatencyFrames = static_cast<uint32_t>(currentPollFrame - slot.SubmittedPollFrame);
            frame->GpuValid = status == EProfilerGpuFrameStatus::Valid;
            if (frame->GpuValid)
            {
                frame->Pipeline = pipeline;
                frame->GpuFrameMs = static_cast<double>(end - begin) * 1000.0 /
                    static_cast<double>(disjoint.Frequency);
                frame->GpuScopes.assign(samples.begin(), samples.begin() + sampleCount);
            }
        }
        slot.Pending = false;
        slot.FrameEnded = false;
    }
}

void CProfiler::Commit_CurrentFrame()
{
    std::lock_guard lock(m_Mutex);
    // Preserve worker scopes that completed between End_Frame and this lock.
    // Otherwise recycle the evicted CPU buffer instead of allocating each frame.
    if (m_History.size() >= MAX_HISTORY_FRAMES && m_PendingScopes.empty())
    {
        m_PendingScopes.swap(m_History.front().CpuScopes);
        m_PendingScopes.clear();
    }
    m_History.push_back(std::move(m_CurrentFrame));
    while (m_History.size() > MAX_HISTORY_FRAMES)
    {
        m_History.pop_front();
        ++m_EvictedHistoryFrames;
    }
}
```

#### `Client/Public/ProfilerTool.h`

후보 SHA256: `ce90bf3bac75f4edb6e2b026cf84129dd2a23925e6fcf1fcd677f2f4c89f2351`

```cpp
#pragma once

#include "Client_Defines.h"
#include "Profiler.h"
#include "ProfilerCaptureIO.h"

#include <array>
#include <string>
#include <vector>

NS_BEGIN(Client)

/* Profiler panel presented by the current Debug F1/F7 routes. It only reads Engine::CProfiler aggregates and never
   owns timing data: the Engine profiler stays the single owner of scopes,
   counters and GPU queries. */
class CProfilerTool final
{
public:
    explicit CProfilerTool(ID3D11Device* pDevice = nullptr);
    void Begin_Capture(Engine::CProfiler& Profiler);
	void Open() { m_bOpen = true; }
	[[nodiscard]] bool_t Is_Open() const noexcept { return m_bOpen; }
	void Render(Engine::CProfiler* pProfiler);
	void Request_Save(Engine::CProfiler& Profiler);
	void Update_SaveState();
	[[nodiscard]] bool_t Is_Saving() const noexcept { return m_Exporter.IsSaving(); }
	[[nodiscard]] const std::string& Get_CaptureStatus() const noexcept { return m_strCaptureStatus; }

private:
	void Refresh(Engine::CProfiler& Profiler);
	const char_t* Scope_Name(uint32_t iNameId) const;
	std::string Thread_Label(uint32_t iThreadId) const;
	void Render_Bottlenecks(bool_t bImGuiOnly = false);
	void Render_ImGui();
	void Rebuild_CpuRows(bool_t bImGuiOnly);
	void Render_Gpu();
	void Render_LongOperations();
	void Render_Counters() const;
	bool_t Refresh_CaptureFiles();
	void Render_CaptureFiles();

private:
	bool_t m_bOpen = true;
    FProfilerCaptureContext m_CaptureContext;
    bool_t m_bSaveWindowOnly = true;
	bool_t m_bShowUnobserved = true;
	bool_t m_bCatalogRegistered = false;
	int32_t m_iWindowFrameInput = 120;
	float m_fRefreshIntervalSeconds = 0.5f;
	double m_fLastRefreshTime = -1.0;
	uint32_t m_iMainThreadId = 0u;
	size_t m_iHistoryFrames = 0u;
    Engine::FProfilerCaptureWindow m_SaveWindowCoverage{};
	double m_fWindowCpuAvgMs = 0.0;
	double m_fWindowCpuMaxMs = 0.0;
	double m_fWindowGpuAvgMs = 0.0;
	double m_fWindowGpuMaxMs = 0.0;
	size_t m_iWindowFrames = 0u;
	Engine::FProfilerLiveStats m_Live{};
	bool_t m_bLiveValid = false;
	std::vector<std::string> m_ScopeNames;
	std::vector<Engine::FProfilerScopeAggregate> m_Aggregates;
	struct FVisibleCpuRow final
	{
		const char* Name = nullptr;
		const Engine::FProfilerScopeAggregate* Aggregate = nullptr;
	};
	std::vector<FVisibleCpuRow> m_VisibleCpuRows;
	std::string m_strCpuRowFilter;
	bool_t m_bCpuRowsDirty = true;
	bool_t m_bCpuRowsImGuiOnly = false;
	bool_t m_bCpuRowsShowUnobserved = true;
	std::vector<Engine::FProfilerGpuScopeAggregate> m_GpuAggregates;
	size_t m_iGpuValidFrames = 0u;
	size_t m_iGpuFrameValidFrames = 0u;
	size_t m_iGpuPartialFrames = 0u;
	std::vector<Engine::FProfilerLongOperation> m_LongOperations;
	std::array<char_t, 96> m_Filter = {};
	std::array<char_t, 241> m_CaptureName = {};
	std::vector<FProfilerCaptureFile> m_CaptureFiles;
	std::string m_strSelectedCaptureId;
	std::string m_strCaptureFilesStatus;
	bool_t m_bCaptureFilesLoaded = false;
	std::string m_strCaptureStatus;
	CProfilerCaptureExporter m_Exporter;
};

NS_END
```

#### `Client/Private/ProfilerTool.cpp`

후보 SHA256: `d70c847e92e0b9a6eb2645bab32ebe1a0386f050b94dd7a8e02a29e75bfe424f`

```cpp
#include "imgui.h"
#include "ProfilerTool.h"
#include "GameInstance.h"
#include "Engine_RenderTypes.h"
#include "ClientWindowDisplay.h"
#include "UserSettingsDocument.h"
#include <cstring>
#include <dxgi.h>

#include <algorithm>
#include <cctype>

namespace
{
    // SCOPE_CATALOG: kept in sync with actual CProfilerScope/GpuScope call sites.
    constexpr const char* CPU_SCOPE_CATALOG[] = {
        "Animation.Blend",
        "Animation.Bones.Combine",
        "Animation.Channels.Sample",
        "Animation.Channels.Update",
        "Animation.History.DebugVerify",
        "Animation.History.Sample",
        "Animation.Play",
        "Animation.SkinPalette.Bind",
        "Animation.SkinPalette.Build",
        "Animation.Transition.Build",
        "Catalog.PlayerSkills.Initialize",
        "Character.EffectCues.Admit",
        "Character.FaceMorph.Admit",
        "Character.FaceSliders.Admit",
        "Character.Initialize",
        "Character.SkillBindings.Admit",
        "CharacterAssets.Authoring.Capture",
        "CharacterAssets.Authoring.Prepare",
        "CharacterAssets.Commit",
        "CharacterAssets.FaceMorph.Prepare",
        "CharacterAssets.LevelTransition.Drain",
        "CharacterAssets.Prepare.Worker",
        "CharacterAssets.Retire.Worker",
        "Client.Render",
        "Client.Update",
        "Effect.Decal.Render",
        "Effect.FollowAnchors.Update",
        "Effect.LevelPresentation.VisibleUpdate",
        "Effect.Material.Bind",
        "Effect.Mesh.BindAndDraw",
        "Effect.Mesh.DrawSubmission",
        "Effect.Mesh.InstanceBuild",
        "Effect.Mesh.InstanceUpload",
        "Effect.Mesh.Render",
        "Effect.Occurrence.LateUpdate",
        "Effect.Occurrence.Render",
        "Effect.Occurrence.Update",
        "Effect.Particle.Render",
        "Effect.Particle.Spawn",
        "Effect.Particle.Update",
        "Effect.Playback.FixedStep",
        "Effect.Playback.FrameRebuild",
        "Effect.Playback.HistoryUpdate",
        "Effect.Playback.Update",
        "Effect.Prepare.Commit",
        "Effect.Prepare.Document",
        "Effect.Prepare.Metadata",
        "Effect.Prepare.Renderer",
        "Effect.Prepare.WorkerTarget",
        "Effect.Prewarm.Advance",
        "Effect.ProductGroups.Update",
        "Effect.Rect.Render",
        "Effect.Service.Update",
        "Effect.Spawn.Commit",
        "Effect.Spawn.CommitWorldRoots",
        "Effect.Sprite.DepthSort",
        "Effect.Sprite.DrawSubmission",
        "Effect.Sprite.InstanceBuild",
        "Effect.Sprite.InstanceUpload",
        "Effect.Trail.Render",
        "Effect.TrailAfterimage.Simulate",
        "EffectSequencer.LaneLayout",
        "EffectSequencer.Render",
        "EffectTool.AllEffectsWindow",
        "EffectTool.ArtistF.MaterialPreparation",
        "EffectTool.ArtistF.SourcePreparation",
        "EffectTool.AuthoringWindow",
        "EffectTool.DataFilesWindow",
        "EffectTool.DetailWindow",
        "EffectTool.DocumentLoad",
        "EffectTool.DocumentLoad.CanonicalBaseline",
        "EffectTool.DocumentLoad.Parse",
        "EffectTool.DocumentLoad.ValidateDrawable",
        "EffectTool.InitialIndexStep",
        "EffectTool.ModelViewWindow",
        "EffectTool.Render",
        "EffectTool.ResourceGrid",
        "EffectTool.ThumbnailTrim",
        "EffectTool.UnifiedEffectTree",
        "EffectTool.UnifiedFamilyRows",
        "Engine.Camera.Update",
        "Engine.Input.Update",
        "Engine.LateUpdate",
        "Engine.LevelUpdate",
        "Engine.ObjectUpdate",
        "Engine.Physics",
        "Engine.PostPhysicsUpdate",
        "Engine.PriorityUpdate",
        "Engine.Sound.Update",
        "ImGui.Composition.Details",
        "ImGui.Composition.PatternTree",
        "ImGui.Composition.PatternTree.Rebuild",
        "ImGui.Composition.Patterns",
        "ImGui.Composition.PresentationIndex.Rebuild",
        "ImGui.Composition.PresentationResources",
        "ImGui.Composition.Resources",
        "ImGui.Composition.Resources.Filter",
        "ImGui.Composition.Resources.Tree.Draw",
        "ImGui.Composition.Resources.Tree.Rebuild",
        "ImGui.Composition.Timeline",
        "ImGui.Composition.Timeline.Draw",
        "ImGui.Composition.Timeline.Layout",
        "ImGui.Composition.Toolbar",
        "Kouku.Presentation.BundleSample",
        "Kouku.Presentation.EncounterVisuals",
        "Kouku.Presentation.FrameLights",
        "Kouku.Presentation.PreviewPreparation",
        "Kouku.Presentation.SharedPresentation",
        "Kouku.Presentation.Update",
        "ImGui.BackendSubmit",
        "ImGui.BuildAndSubmit",
        "ImGui.DX11.BackupState",
        "ImGui.DX11.DeviceObjects",
        "ImGui.DX11.DrawSubmission",
        "ImGui.DX11.GrowBuffers",
        "ImGui.DX11.MapBuffers",
        "ImGui.DX11.RestoreState",
        "ImGui.DX11.SetupState",
        "ImGui.DX11.TextureUpdate",
        "ImGui.DX11.UploadBuffers",
        "ImGui.DeveloperTools",
        "ImGui.FinalizeDrawData",
        "ImGui.Hub.Build",
        "ImGui.Hub.CameraAndPlayer",
        "ImGui.Hub.FollowCamera",
        "ImGui.Hub.KoukuArena",
        "ImGui.Hub.KoukuCompletePlay",
        "ImGui.Hub.KoukuUIPreview",
        "ImGui.Hub.LevelNavigation",
        "ImGui.Hub.SequenceViewer.Build",
        "ImGui.Hub.SequenceViewer.Refresh",
        "ImGui.Hub.SequenceViewer.Update",
        "ImGui.Hub.ServerArena",
        "ImGui.Hub.ValtanCompletePlay",
        "ImGui.NewFrame",
        "ImGui.NewFrame.Core",
        "ImGui.NewFrame.DX11",
        "ImGui.NewFrame.Win32",
        "ImGui.PlatformRender",
        "ImGui.PlatformUpdate",
        "ImGui.PlatformViewport.Present",
        "ImGui.PlatformViewport.Render",
        "ImGui.ProfilerDetails",
        "ImGui.ProfilerOverlay",
        "ImGui.RenderDrawData",
        "ImGui.Tool.Animation.Build",
        "ImGui.Tool.Animation.Update",
        "ImGui.Tool.Balance.Build",
        "ImGui.Tool.Balance.Update_ServerRuntimeSetPublishJob",
        "ImGui.Tool.Balance.Update_ValtanSaveJob",
        "ImGui.Tool.Camera.Build",
        "ImGui.Tool.Camera.Update",
        "UI.Runtime.Chat.Update",
        "ImGui.Tool.Composition.Build",
        "ImGui.Tool.CompositionProfiler.Build",
        "ImGui.Tool.EffectV1.Build",
        "ImGui.Tool.EffectV1.Update",
        "ImGui.Tool.EffectV1.Update_AuthoringWorkspace",
        "ImGui.Tool.EffectV2.Build",
        "ImGui.Tool.EffectV2.Update_AuthoringWorkspace",
        "ImGui.Tool.Equipment.Build",
        "ImGui.Tool.HUDLayout.Build",
        "ImGui.Tool.KoukuBoss.Build",
        "ImGui.Tool.Map.Build",
        "ImGui.Tool.Map.Update",
        "ImGui.Tool.Open",
        "UI.Runtime.Party.Update",
        "ImGui.Tool.Rendering.Build",
        "ImGui.Tool.SequenceBenchmark.Build",
        "ImGui.Tool.ValtanBoss.Build",
        "ImGui.Tool.ValtanBoss.Render_LogicPatternWindow",
        "ImGui.Tool.ValtanBoss.Update",
        "ImGui.Tool.ValtanComposition.Update_SaveState",
        "ImGui.Tool.WorldObjects.Build",
        "ImGui.Tool.WorldObjects.Update",
        "Level.Kouku.Camera.Create",
        "Level.Kouku.CameraShots.Load",
        "Level.Kouku.Controller.Prepare",
        "Level.Kouku.DebugTriggers.Load",
        "Level.Kouku.DeployCommit",
        "Level.Kouku.InitialVisibility",
        "Level.Kouku.Initialize",
        "Level.Kouku.MapLights.Load",
        "Level.Kouku.MapPlacementCommit",
        "Level.Kouku.Replication.Initialize",
        "Level.Kouku.SelfMotion.Load",
        "Level.Kouku.StageMarkers.Load",
        "Level.Kouku.UI.Create",
        "Level.Kouku.WorldSequence.Load",
        "Loader.EffectPreparation",
        "Loader.LevelLoad",
        "MainApp.DebugTools.Update",
        "MainApp.Engine.Update",
        "MainApp.InputAndUI.Update",
        "MainApp.LevelAndEnvironment.Update",
        "MainApp.Presentation.Prepare",
        "Map.Batch.BindAndDraw",
        "Map.Batch.CullAndPack",
        "Map.Batch.InstanceUpload",
        "Map.Batch.ShadowPrepare",
        "Map.Batch.ShadowUpload",
        "Map.Batch.Visibility",
        "Model.Load.Animations",
        "Model.Load.Binary",
        "Model.Load.Bones",
        "Model.Load.Decode",
        "Model.Load.Materials",
        "Model.Load.MaterialWorker",
        "Model.Load.MaterialJoin",
        "Model.Load.Meshes",
        "Texture.Cache.Lookup",
        "Texture.Cache.SameKeyWait",
        "Texture.Load.FileAndUpload",
        "Navigation.AStar",
        "Navigation.FindPath",
        "Navigation.Follow",
        "Navigation.Request",
        "Navigation.RoundCorners",
        "Navigation.Simplify",
        "Network.DrainAndDispatch",
        "Network.PlayerAssets.Advance",
        "Network.PlayerPresentation.CommitSpawn",
        "Network.PlayerPresentation.Replace",
        "Picking.CopyPixel",
        "Picking.MapWait",
        "Picking.ReadPixel",
        "Picking.Readback",
        "Profiler.Capture.Snapshot",
        "Profiler.Panel.Refresh",
        "Map.Batch.Material.Bind",
        "Map.Batch.Pass.Apply",
        "Map.Batch.Mesh.Submit",
        "Map.Shadow.Material.Bind",
        "Map.Shadow.Pass.Apply",
        "Map.Shadow.Mesh.Submit",
        "Effect.Particle.Update.Worker",
        "Effect.Particle.Update.Join",
        "Render.BeginFrame",
        "Render.Blend",
        "Render.Bloom",
        "Render.BossShowcase",
        "Render.Combined",
        "Render.Debug",
        "Render.DisplayOverlays",
        "Render.Draw",
        "Render.Final",
        "Render.Lights",
        "Render.Lights.StageAndSubmit",
        "Render.Lights.UploadAndDraw",
        "Render.NonBlend",
        "Render.NonLight",
        "Render.Portraits",
        "Render.Present",
        "Render.Priority",
        "Render.SSAO",
        "Render.SceneColorSnapshot",
        "Render.SceneHDR",
        "Render.ScreenPosts",
        "Render.Shadow",
        "Render.Shadow.CacheAdmission",
        "Render.Shadow.CacheCopy",
        "Render.Shadow.StaticBuild",
        "Render.Shadow.Dynamic",
        "Render.SubmitFrameProviders",
        "Render.UI",
        "Render.UIText",
        "Render.World",
        "Replication.Update",
        "Shader.BuildBindings",
        "Shader.CreateEffect",
        "Shader.CreateInputLayouts",
        "Shader.Load",
        "Shader.ReadBytecode",
        "Tool.Composition.ReloadCanonical",
        "Tool.Composition.SaveReload",
        "Tool.ValtanBossTool.ReloadCanonicalGraph",
        "UI.Runtime.BossHealthBar.Update",
        "UI.Runtime.BossImmuneGauge.Update",
        "UI.Runtime.CharacterSelectWindow.Update",
        "UI.Runtime.ChargeGauge.Update",
        "UI.Runtime.CombatHUD.Update",
        "UI.Runtime.EstherGauge.Update",
        "UI.Runtime.ItemQuickSlots.Update",
        "UI.Runtime.ItemUpgrade.Update",
        "UI.Runtime.LanceMasterIdentityGauge.Update",
        "UI.Runtime.LobbyButtons.Update",
        "UI.Runtime.Minimap.Update",
        "UI.Runtime.PlayerHealthManaBar.Update",
        "UI.Runtime.QuickSlotFlash.Update",
        "UI.Runtime.SkillCooldowns.Update",
        "UI.Runtime.SkillIcons.Update",
        "WorldSequence.CollectLoadTargets",
        "WorldSequence.CommitPrepared",
        "WorldSequence.Document.Load",
        "WorldSequence.Document.Parse",
        "WorldSequence.Document.Validate",
        "WorldSequence.PrepareArea",
    };
    constexpr const char* GPU_SCOPE_CATALOG[] = {
        "ImGui.BackendSubmit",
        "ImGui.PlatformViewport.Present",
        "ImGui.PlatformViewport.Render",
        "ImGui.RenderDrawData",
        "Picking.CopyPixel",
        "Render.BeginFrame",
        "Render.Blend",
        "Render.Bloom",
        "Render.BossShowcase",
        "Render.Combined",
        "Render.Debug",
        "Render.DisplayOverlays",
        "Render.Draw",
        "Render.Final",
        "Render.Lights",
        "Render.NonBlend",
        "Render.NonLight",
        "Render.Portraits",
        "Render.Priority",
        "Render.SSAO",
        "Render.SceneColorCopy",
        "Render.SceneHDR",
        "Render.ScreenPosts",
        "Render.Shadow",
        "Render.Shadow.CacheCopy",
        "Render.Shadow.StaticBuild",
        "Render.Shadow.Dynamic",
        "Render.UI",
        "Render.UIText",
    };
    constexpr ImGuiTableFlags TABLE_FLAGS = ImGuiTableFlags_RowBg |
        ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_ScrollY |
        ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp;

    constexpr std::array<const char*, static_cast<size_t>(Engine::EProfilerCounter::Count)>
        COUNTER_LABELS = {
        "Draw calls", "Instanced draw calls", "Instances", "Indices",
        "Submissions: priority", "Submissions: shadow", "Submissions: non-blend", "Submissions: blend",
        "Map placements", "Map visible instances", "Map batches", "Map fallback objects",
        "Texture requests", "Texture path hits", "Texture content hits", "Texture unique SRVs",
        "Texture estimated GPU bytes", "Navigation queries (Client)", "Navigation expanded nodes (Client)",
        "Navigation query microseconds (Client)", "Navigation path cells (Client)",
        "Scene color copies", "Scene color copy bytes (source size)",
        "ImGui draw lists",
        "ImGui vertices",
        "ImGui indices",
        "ImGui draw commands",
        "ImGui actual draw calls",
        "ImGui callbacks",
        "ImGui rendered windows",
        "ImGui active windows",
        "ImGui platform viewports",
        "ImGui vertex upload bytes",
        "ImGui index upload bytes",
        "ImGui constant upload bytes",
        "ImGui texture upload bytes",
        "ImGui buffer growths",
        "ImGui texture creates",
        "ImGui texture updates",
        "ImGui device object builds",
        "ImGui buffer Map failures",
        "Picking readbacks",
        "Picking readback bytes",
        "Indirect draw calls",
        "Indirect indices (LOD0 upper bound)",
        "Shadow cache hits", "Shadow cache misses", "Shadow static casters", "Shadow dynamic casters",
        "Map culling candidates", "Map culling visible", "Map LOD 0 draws", "Map LOD 1 draws", "Map LOD 2 draws",
        "Map source indices (LOD 0)", "Map submitted indices (chosen LOD)",
        "Light records (receiver passes)", "Light draw calls", "Light record upload bytes",
        "Map draws with generated LOD geometry",
        "Local light culling candidates", "Local light culling rejected",
        "Effect bounds candidates", "Effect bounds culled",
        "Trigger marker sample attempts", "Trigger marker entry/reentry history requests",
        "Ambient effects suspended", "Ambient effects advanced",
        "ImGui present attempts", "ImGui present busy (deferred)", "ImGui present failures", "ImGui present occluded",
        "Map batch visibility cache hits", "Map batch visibility rebuilds",
        "Map batch renders: empty", "Map batch renders: visible", "Map batch broad bounds rejected",
        "Map visible instance upload bytes", "NPC authored-hidden updates", "Ambient invalidated-bound updates",
        "NPC culling candidates", "NPC culled", "NPC deferred pose evaluations",
    };

    bool Contains_CaseInsensitive(std::string_view text, const char* query)
    {
        if (!query || !query[0]) return true;
        const std::string_view needle(query);
        return std::search(text.begin(), text.end(), needle.begin(), needle.end(),
            [](char a, char b) { return std::tolower(static_cast<unsigned char>(a)) ==
                std::tolower(static_cast<unsigned char>(b)); }) != text.end();
    }

    std::string Capture_PathLabel(const std::filesystem::path& path)
    {
        const auto text = path.u8string();
        return std::string(text.begin(), text.end());
    }

    const char* Gpu_Status(Engine::EProfilerGpuFrameStatus status)
    {
        switch (status)
        {
        case Engine::EProfilerGpuFrameStatus::Unsupported: return "unsupported";
        case Engine::EProfilerGpuFrameStatus::Pending: return "pending";
        case Engine::EProfilerGpuFrameStatus::Valid: return "valid";
        case Engine::EProfilerGpuFrameStatus::Disjoint: return "disjoint";
        case Engine::EProfilerGpuFrameStatus::Dropped: return "dropped";
        case Engine::EProfilerGpuFrameStatus::Error: return "query error";
        }
        return "unknown";
    }

    void Unobserved_Row(const char* name, int columns)
    {
        ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextDisabled("%s", name);
        for (int i = 1; i < columns; ++i)
        {
            ImGui::TableNextColumn(); ImGui::TextDisabled(i == 1 ? "not observed" : "--");
        }
    }
}

Client::CProfilerTool::CProfilerTool(ID3D11Device* device)
{
    if (!device) return;
    m_CaptureContext.DeviceCreationFlags = device->GetCreationFlags();
    Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
    Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
    DXGI_ADAPTER_DESC desc{};
    if (FAILED(device->QueryInterface(IID_PPV_ARGS(dxgiDevice.GetAddressOf()))) ||
        FAILED(dxgiDevice->GetAdapter(adapter.GetAddressOf())) || FAILED(adapter->GetDesc(&desc))) return;
    const int bytes = WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1, nullptr, 0, nullptr, nullptr);
    if (bytes > 1)
    {
        std::string text(static_cast<size_t>(bytes), '\0');
        WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1, text.data(), bytes, nullptr, nullptr);
        text.pop_back();
        m_CaptureContext.Adapter = std::move(text);
    }
}

void Client::CProfilerTool::Begin_Capture(Engine::CProfiler& profiler)
{
    profiler.Reset_History();
    CProfilerCaptureIO::Reset_MovementSamples();
    profiler.Set_Enabled(true);
    m_fLastRefreshTime = -1.0;
}

void Client::CProfilerTool::Refresh(Engine::CProfiler& profiler)
{
    if (!m_bCatalogRegistered)
    {
        for (const char* name : CPU_SCOPE_CATALOG) profiler.Register_ScopeName(name);
        for (const char* name : GPU_SCOPE_CATALOG) profiler.Register_ScopeName(name);
        m_bCatalogRegistered = true;
    }
    Engine::CProfilerScope scope(&profiler, "Profiler.Panel.Refresh");
    m_iMainThreadId = profiler.Get_MainThreadId();
    m_iHistoryFrames = profiler.Get_HistoryFrameCount();
    profiler.Get_ScopeNames(m_ScopeNames);
    const size_t window = static_cast<size_t>((std::max)(m_iWindowFrameInput, 1));
    m_SaveWindowCoverage = profiler.Get_CaptureWindow(
        m_bSaveWindowOnly ? window : Engine::CProfiler::MAX_HISTORY_FRAMES);
    profiler.Get_ScopeAggregates(window, m_Aggregates);
    profiler.Get_GpuScopeAggregates(window, m_GpuAggregates, m_iGpuValidFrames, m_iGpuPartialFrames);
    profiler.Get_WindowFrameStats(window, m_fWindowCpuAvgMs, m_fWindowCpuMaxMs,
        m_fWindowGpuAvgMs, m_fWindowGpuMaxMs, m_iWindowFrames, &m_iGpuFrameValidFrames);
    profiler.Get_LongOperations(m_LongOperations);
    m_bLiveValid = profiler.Get_LiveStats(m_Live);
    m_bCpuRowsDirty = true;
}

const char_t* Client::CProfilerTool::Scope_Name(uint32_t id) const
{
    return id < m_ScopeNames.size() ? m_ScopeNames[id].c_str() : "<unknown>";
}

std::string Client::CProfilerTool::Thread_Label(uint32_t id) const
{
    return id == m_iMainThreadId ? "main" : "worker " + std::to_string(id);
}

void Client::CProfilerTool::Request_Save(Engine::CProfiler& profiler)
{
    if (m_Exporter.IsSaving()) return;
    std::string error;
    if (!CProfilerCaptureIO::Validate_Name(m_CaptureName.data(), &error))
    { m_strCaptureStatus = error; return; }
    Engine::FProfilerCaptureSnapshot snapshot;
    {
        Engine::CProfilerScope scope(&profiler, "Profiler.Capture.Snapshot");
        snapshot = profiler.Snapshot(m_bSaveWindowOnly ?
            static_cast<size_t>((std::max)(m_iWindowFrameInput, 1)) : Engine::CProfiler::MAX_HISTORY_FRAMES);
    }
    if (snapshot.Frames.empty())
    { m_strCaptureStatus = "No completed frames. Enable Capture and wait before saving."; return; }
    auto& game = Engine::CGameInstance::Get();
    auto context = m_CaptureContext;
    CProfilerCaptureIO::Copy_MovementSamples(snapshot, context);
    context.Valid = true;
    context.LevelId = game.Get_CurrentLevelID();
    const HWND foregroundWindow = GetForegroundWindow();
    DWORD foregroundProcessId = 0;
    context.ClientWindowForeground = foregroundWindow == g_hWnd;
    context.ProcessForeground = nullptr != foregroundWindow &&
        0 != GetWindowThreadProcessId(foregroundWindow, &foregroundProcessId) &&
        GetCurrentProcessId() == foregroundProcessId;
    context.WindowMinimized = CClientWindowDisplay::Is_Minimized();
    const auto& userSettings = CUserSettings::Get();
    context.ForegroundFpsLimit = userSettings.Get_FrameLimit(true);
    context.BackgroundFpsLimit = userSettings.Get_FrameLimit(false);
    context.EffectiveFpsLimit = userSettings.Get_FrameLimit(context.ProcessForeground);
    const auto viewport = game.Get_ViewportSize();
    context.Viewport = {viewport.x, viewport.y};
    if (const auto* camera = game.Get_CamPosition())
        context.CameraPosition = {camera->x, camera->y, camera->z, camera->w};
    if (const auto* view = game.Get_Transform(Engine::D3DTS::VIEW))
        std::memcpy(context.ViewMatrix.data(), view, sizeof(*view));
    if (const auto* projection = game.Get_Transform(Engine::D3DTS::PROJ))
        std::memcpy(context.ProjectionMatrix.data(), projection, sizeof(*projection));
    const auto& shadow = game.Get_ShadowLightDesc().Settings;
    context.ShadowEnabled = shadow.bEnabled;
    context.ShadowWidth = shadow.fOrthographicWidth;
    context.ShadowHeight = shadow.fOrthographicHeight;
    context.ShadowStrength = shadow.fStrength;
    const auto quality = game.Get_RenderQualitySettings();
    context.SSAOEnabled = quality.bSSAOEnabled;
    context.BloomEnabled = quality.bBloomEnabled;
    context.FXAAEnabled = quality.bFXAAEnabled;
    const uint64_t frame = snapshot.Frames.back().FrameNumber;
    std::filesystem::path output;
    if (!CProfilerCaptureIO::Make_NamedPath(m_CaptureName.data(), frame, output, &error))
    { m_strCaptureStatus = error; return; }
    m_strCaptureStatus = m_Exporter.BeginSave(std::move(snapshot), output, &error, std::move(context)) ?
        "Saving JSON in background..." : error;
}

void Client::CProfilerTool::Update_SaveState()
{
    FProfilerCaptureSaveResult saveResult;
    if (!m_Exporter.Poll(saveResult)) return;
    m_strCaptureStatus = saveResult.Succeeded ?
        "Saved " + Capture_PathLabel(saveResult.OutputPath) : saveResult.Error;
    if (saveResult.Succeeded && Refresh_CaptureFiles())
        for (const auto& file : m_CaptureFiles)
            if (file.FileName == saveResult.OutputPath.filename())
            { m_strSelectedCaptureId = file.StableId; break; }
}

void Client::CProfilerTool::Render(Engine::CProfiler* profiler)
{
    if (!m_bOpen) return;
    ImGui::SetNextWindowSize(ImVec2(1060.f, 720.f), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Composition Profiler###LostArkProfilerToolV1", &m_bOpen))
    {
        ImGui::End(); return;
    }
    if (!profiler)
    {
        ImGui::TextUnformatted("Engine profiler is unavailable."); ImGui::End(); return;
    }

    if (!m_bCaptureFilesLoaded) Refresh_CaptureFiles();
    ImGui::SetNextItemWidth(300.f);
    ImGui::InputTextWithHint("Save name", "Optional name (Korean supported)", m_CaptureName.data(), m_CaptureName.size());
    ImGui::SameLine(); ImGui::TextDisabled("Each save creates a new JSON file.");
#ifdef _DEBUG
    ImGui::TextDisabled("Build: Debug | F7 hides/shows this window; Capture controls measurement.");
#else
    ImGui::TextDisabled("Build: Release | Capture controls measurement.");
#endif
    ImGui::TextWrapped("For comparable runs: warm the scene, Reset, hide with F7, reproduce the same camera and actions, then reopen and Save. Closing this panel does not pause capture. Context in JSON is sampled at export.");
    bool enabled = profiler->Is_Enabled();
    if (ImGui::Checkbox("Capture", &enabled))
    {
        profiler->Set_Enabled(enabled); m_fLastRefreshTime = -1.0;
    }
    ImGui::SameLine(); ImGui::SetNextItemWidth(100.f);
    if (ImGui::DragInt("Frames", &m_iWindowFrameInput, 1.f, 1,
        static_cast<int>(Engine::CProfiler::MAX_HISTORY_FRAMES), "%d", ImGuiSliderFlags_AlwaysClamp))
        m_fLastRefreshTime = -1.0;
    ImGui::SameLine();
    if (ImGui::Button("Reset"))
    {
        profiler->Reset_History();
        CProfilerCaptureIO::Reset_MovementSamples();
        m_fLastRefreshTime = -1.0;
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(m_Exporter.IsSaving());
    if (ImGui::Button("Save JSON")) Request_Save(*profiler);
    ImGui::EndDisabled(); ImGui::SameLine();
    ImGui::TextDisabled("%zu / %zu history frames", m_iHistoryFrames, Engine::CProfiler::MAX_HISTORY_FRAMES);

    bool detailed = profiler->Is_DetailedScopesEnabled();
    if (ImGui::Checkbox("Detailed per-draw CPU scopes (higher overhead)", &detailed))
    {
        profiler->Set_DetailedScopesEnabled(detailed);
        profiler->Reset_History();
        CProfilerCaptureIO::Reset_MovementSamples();
        m_fLastRefreshTime = -1.0;
    }
    if (ImGui::Checkbox("Save only selected frame window", &m_bSaveWindowOnly))
        m_fLastRefreshTime = -1.0;
    ImGui::SameLine(); ImGui::TextDisabled("Uncheck to save all retained history (up to 1200 frames).");

    const double now = ImGui::GetTime();
    if (m_fLastRefreshTime < 0.0 ||
        ((enabled || (m_bLiveValid && m_Live.LatestFrameGpuStatus == Engine::EProfilerGpuFrameStatus::Pending)) &&
            now - m_fLastRefreshTime >= m_fRefreshIntervalSeconds))
    {
        Refresh(*profiler); m_fLastRefreshTime = now;
    }
    ImGui::TextDisabled("Save range: %llu frames (%llu - %llu); retained %llu; evicted since Reset %llu.",
        static_cast<unsigned long long>(m_SaveWindowCoverage.SavedFrames),
        static_cast<unsigned long long>(m_SaveWindowCoverage.FirstSavedFrameNumber),
        static_cast<unsigned long long>(m_SaveWindowCoverage.LastSavedFrameNumber),
        static_cast<unsigned long long>(m_SaveWindowCoverage.RetainedFrames),
        static_cast<unsigned long long>(m_SaveWindowCoverage.EvictedFramesSinceReset));
    if (m_SaveWindowCoverage.ExcludedRetainedFrames != 0)
        ImGui::TextWrapped("The selected save window omits %llu retained frames (peak interval %.2f ms). Uncheck Save only selected frame window to include them.",
            static_cast<unsigned long long>(m_SaveWindowCoverage.ExcludedRetainedFrames),
            m_SaveWindowCoverage.ExcludedMaxFrameIntervalMs);
    if (m_SaveWindowCoverage.EvictedFramesSinceReset != 0)
        ImGui::TextWrapped("Older frames have left the 1200-frame history and cannot be recovered by this save. Save shorter runs before the history fills.");
    if (ImGui::BeginTable("##FrameSummary", 3, ImGuiTableFlags_SizingStretchSame))
    {
        ImGui::TableNextColumn(); ImGui::TextDisabled("FRAME INTERVAL / FPS (latest)");
        if (m_bLiveValid && m_Live.FrameIntervalMs > 0.0)
            ImGui::Text("%.2f ms / %.1f FPS", m_Live.FrameIntervalMs, 1000.0 / m_Live.FrameIntervalMs);
        else ImGui::TextDisabled("--");
        ImGui::TableNextColumn(); ImGui::TextDisabled("CPU FRAME (window average / peak)");
        if (m_iWindowFrames) ImGui::Text("%.2f / %.2f ms", m_fWindowCpuAvgMs, m_fWindowCpuMaxMs);
        else ImGui::TextDisabled("--");
        ImGui::TableNextColumn(); ImGui::TextDisabled("GPU INTERVAL (valid average / peak)");
        if (m_iGpuFrameValidFrames > 0)
            ImGui::Text("%.2f / %.2f ms", m_fWindowGpuAvgMs, m_fWindowGpuMaxMs);
        else ImGui::TextDisabled("-- (no resolved results in window)");
        ImGui::EndTable();
    }
    if (!enabled) ImGui::TextDisabled("Capture is paused. Recorded results stay visible.");
    if (m_bLiveValid && (m_Live.TotalDroppedCpuScopes || m_Live.TotalDroppedGpuFrames ||
        m_Live.TotalDroppedGpuScopes || m_Live.TotalDroppedModelAnimationSamples))
        ImGui::TextWrapped("Incomplete capture since reset: omitted CPU scopes %llu / GPU frames %llu / GPU scopes %llu / animation samples %llu",
            static_cast<unsigned long long>(m_Live.TotalDroppedCpuScopes), static_cast<unsigned long long>(m_Live.TotalDroppedGpuFrames),
            static_cast<unsigned long long>(m_Live.TotalDroppedGpuScopes), static_cast<unsigned long long>(m_Live.TotalDroppedModelAnimationSamples));
    if (m_bLiveValid && m_Live.DroppedCpuScopes)
        ImGui::TextColored(ImVec4(1.f, .65f, .25f, 1.f), "Latest frame omitted %llu CPU scopes; parent/self attribution is incomplete.",
            static_cast<unsigned long long>(m_Live.DroppedCpuScopes));
    if (m_bLiveValid && m_Live.TotalDroppedCpuScopes)
        ImGui::TextWrapped("CPU scopes were omitted since Reset. Inclusive/Self tables may be incomplete; disable detailed scopes and Reset before comparing bottlenecks.");
    if (!m_strCaptureStatus.empty()) ImGui::TextWrapped("%s", m_strCaptureStatus.c_str());
    ImGui::Separator();
    ImGui::SetNextItemWidth(280.f);
    ImGui::InputTextWithHint("##ProfilerFilter", "Filter section (Animation, Map, Render...)", m_Filter.data(), m_Filter.size());
    ImGui::SameLine(); ImGui::Checkbox("Show unobserved sections", &m_bShowUnobserved);
    if (ImGui::BeginTabBar("##ProfilerTabs"))
    {
        if (ImGui::BeginTabItem("ImGui")) { Render_ImGui(); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("CPU sections")) { Render_Bottlenecks(); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("GPU passes")) { Render_Gpu(); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Workload")) { Render_Counters(); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Long operations")) { Render_LongOperations(); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Saved JSON")) { Render_CaptureFiles(); ImGui::EndTabItem(); }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void Client::CProfilerTool::Render_Bottlenecks(bool_t bImGuiOnly)
{
    Rebuild_CpuRows(bImGuiOnly);
    ImGui::TextWrapped("Avg includes child sections; Self excludes them on the same thread. Do not add parent and child times. Peak is one call. Unobserved means no completed sample in this window.");
    const double frames = static_cast<double>((std::max)(m_iWindowFrames, size_t{1}));
    if (!ImGui::BeginTable("##CpuSections", 6, TABLE_FLAGS)) return;
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("Section", ImGuiTableColumnFlags_WidthStretch, 3.f);
    for (const char* label : { "Thread", "Avg ms/frame", "Self ms/frame", "Peak call ms", "Calls/frame" })
        ImGui::TableSetupColumn(label);
    ImGui::TableHeadersRow();
    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(m_VisibleCpuRows.size()));
    while (clipper.Step())
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
        {
            const auto& visible = m_VisibleCpuRows[static_cast<size_t>(i)];
            if (!visible.Aggregate) { Unobserved_Row(visible.Name, 6); continue; }
            const auto& row = *visible.Aggregate;
            ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted(visible.Name);
            ImGui::TableNextColumn();
            if (row.ThreadId == m_iMainThreadId) ImGui::TextUnformatted("main");
            else ImGui::Text("worker %u", row.ThreadId);
            ImGui::TableNextColumn(); ImGui::Text("%.3f", row.InclusiveMs / frames);
            ImGui::TableNextColumn();
            if (row.SelfComplete) ImGui::Text("%.3f", row.SelfMs / frames);
            else
            {
                ImGui::TextDisabled("--");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Self time is unavailable: CPU scopes were dropped in this selected window. Missing child scopes would be incorrectly charged to their parent. Reset and capture with detailed scopes off.");
            }
            ImGui::TableNextColumn(); ImGui::Text("%.3f", row.MaxMs);
            ImGui::TableNextColumn(); ImGui::Text("%.2f", static_cast<double>(row.Calls) / frames);
        }
    ImGui::EndTable();
}

void Client::CProfilerTool::Rebuild_CpuRows(bool_t bImGuiOnly)
{
    if (!m_bCpuRowsDirty && m_strCpuRowFilter == m_Filter.data() &&
        m_bCpuRowsImGuiOnly == bImGuiOnly && m_bCpuRowsShowUnobserved == m_bShowUnobserved)
        return;
    m_VisibleCpuRows.clear();
    const auto matches = [&](const char* name)
    {
        const std::string_view text(name);
        const bool isImGui = text.starts_with("ImGui.") || text.starts_with("Profiler.Panel.") ||
            text.starts_with("Profiler.Capture.") || text.starts_with("EffectTool.") || text.starts_with("EffectSequencer.");
        return (!bImGuiOnly || isImGui) && Contains_CaseInsensitive(text, m_Filter.data());
    };
    for (const auto& row : m_Aggregates)
        if (matches(Scope_Name(row.NameId)))
            m_VisibleCpuRows.push_back({Scope_Name(row.NameId), &row});
    if (m_bShowUnobserved)
        for (const char* name : CPU_SCOPE_CATALOG)
            if (matches(name) && std::none_of(m_Aggregates.begin(), m_Aggregates.end(),
                [&](const auto& row) { return std::string_view(name) == Scope_Name(row.NameId); }))
                m_VisibleCpuRows.push_back({name, nullptr});
    m_strCpuRowFilter = m_Filter.data();
    m_bCpuRowsImGuiOnly = bImGuiOnly;
    m_bCpuRowsShowUnobserved = m_bShowUnobserved;
    m_bCpuRowsDirty = false;
}

void Client::CProfilerTool::Render_ImGui()
{
    ImGui::TextWrapped("Build = tool code and widget layout. DX11 = upload, state changes and drawing. Platform Present can include OS/driver waits. Compare the same scene with F1 closed, one tool open, then detached windows.");
    if (m_bLiveValid)
    {
        const auto count = [&](Engine::EProfilerCounter counter)
        { return static_cast<unsigned long long>(m_Live.Counters[static_cast<size_t>(counter)]); };
        ImGui::Text("CPU frame %llu | windows %llu / active %llu | viewports %llu | draws %llu / commands %llu",
            static_cast<unsigned long long>(m_Live.FrameNumber), count(Engine::EProfilerCounter::ImGuiRenderWindows),
            count(Engine::EProfilerCounter::ImGuiActiveWindows), count(Engine::EProfilerCounter::ImGuiPlatformViewports),
            count(Engine::EProfilerCounter::ImGuiDrawCalls), count(Engine::EProfilerCounter::ImGuiDrawCommands));
        ImGui::Text("Detached presents %llu | busy %llu | errors %llu | occluded %llu",
            count(Engine::EProfilerCounter::ImGuiPresentAttempts), count(Engine::EProfilerCounter::ImGuiPresentBusy),
            count(Engine::EProfilerCounter::ImGuiPresentFailures), count(Engine::EProfilerCounter::ImGuiPresentOccluded));
        ImGui::Text("Vertices %llu / indices %llu | vertex + index upload %.1f KiB | buffer growths %llu | Map failures %llu",
            count(Engine::EProfilerCounter::ImGuiVertices), count(Engine::EProfilerCounter::ImGuiIndices),
            (count(Engine::EProfilerCounter::ImGuiVertexUploadBytes) + count(Engine::EProfilerCounter::ImGuiIndexUploadBytes)) / 1024.0,
            count(Engine::EProfilerCounter::ImGuiBufferGrowths), count(Engine::EProfilerCounter::ImGuiBufferMapFailures));
    }
    for (const auto& row : m_GpuAggregates)
        if (std::string_view(Scope_Name(row.NameId)) == "ImGui.RenderDrawData")
            ImGui::Text("GPU drawing interval: avg %.3f ms / P95 %.3f ms (%zu complete frames; all viewports)",
                row.InclusiveMs / static_cast<double>((std::max)(m_iGpuValidFrames, size_t{1})), row.P95FrameMs, m_iGpuValidFrames);
    if (ImGui::TreeNode("Backend workload counters"))
    {
        if (ImGui::BeginTable("##ImGuiWorkload", 2, TABLE_FLAGS, ImVec2(0.f, 160.f)))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("Counter"); ImGui::TableSetupColumn("Latest CPU frame"); ImGui::TableHeadersRow();
            for (size_t i = static_cast<size_t>(Engine::EProfilerCounter::ImGuiDrawLists); i <= static_cast<size_t>(Engine::EProfilerCounter::ImGuiBufferMapFailures); ++i)
            {
                ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted(COUNTER_LABELS[i]); ImGui::TableNextColumn();
                if (m_bLiveValid) ImGui::Text("%llu", static_cast<unsigned long long>(m_Live.Counters[i]));
                else ImGui::TextDisabled("--");
            }
            ImGui::EndTable();
        }
        ImGui::TreePop();
    }
    Render_Bottlenecks(true);
}

void Client::CProfilerTool::Render_Gpu()
{
    ImGui::TextWrapped("GPU timestamps measure elapsed intervals, including possible waits for CPU submission. They are not GPU utilization. Nested passes overlap; do not add them. CPU-only animation/navigation have no GPU duration.");
    ImGui::Text("Complete GPU frames: %zu / %zu | partial frames excluded: %zu", m_iGpuValidFrames, m_iWindowFrames, m_iGpuPartialFrames);
    if (m_bLiveValid && !m_Live.GpuScopesSupported)
        ImGui::TextDisabled("GPU pass queries are unavailable. Whole-frame timing may still be available.");
    if (m_bLiveValid)
    {
        ImGui::Text("Latest CPU frame %llu: GPU %s", static_cast<unsigned long long>(m_Live.FrameNumber), Gpu_Status(m_Live.LatestFrameGpuStatus));
        if (m_Live.GpuValid) ImGui::Text("Last resolved GPU frame %llu | readback %u frames | omitted scopes %u",
            static_cast<unsigned long long>(m_Live.GpuFrameNumber), m_Live.GpuLatencyFrames, m_Live.DroppedGpuScopes);
    }
    if (m_bLiveValid && m_Live.GpuValid && ImGui::TreeNode("Last resolved frame intervals"))
    {
        if (ImGui::BeginTable("##GpuIntervals", 4, TABLE_FLAGS, ImVec2(0, 180.f)))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("Pass", ImGuiTableColumnFlags_WidthStretch, 3.f);
            for (const char* label : {"Begin ms", "End ms", "Elapsed ms"}) ImGui::TableSetupColumn(label);
            ImGui::TableHeadersRow();
            for (const auto& row : m_Live.GpuScopes)
            {
                if (!Contains_CaseInsensitive(Scope_Name(row.NameId), m_Filter.data())) continue;
                ImGui::TableNextRow(); ImGui::TableNextColumn();
                ImGui::Text("%*s%s", static_cast<int>(row.Depth * 2), "", Scope_Name(row.NameId));
                ImGui::TableNextColumn(); ImGui::Text("%.3f", row.BeginMs);
                ImGui::TableNextColumn(); ImGui::Text("%.3f", row.EndMs);
                ImGui::TableNextColumn(); ImGui::Text("%.3f", row.DurationMs);
            }
            ImGui::EndTable();
        }
        ImGui::TreePop();
    }
    ImGui::TextWrapped("PS / VS count shader invocations, not arithmetic instructions. Selected passes only; -- means unavailable or incomplete. Nested counts must not be added.");
    if (!ImGui::BeginTable("##GpuPasses", 7, TABLE_FLAGS)) return;
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("Pass", ImGuiTableColumnFlags_WidthStretch, 3.f);
    for (const char* label : {"Avg ms/frame", "Peak frame ms", "P95 frame ms", "Calls/frame", "PS / frame", "VS / frame"}) ImGui::TableSetupColumn(label);
    ImGui::TableHeadersRow();
    const double frames = static_cast<double>((std::max)(m_iGpuValidFrames, size_t{1}));
    for (const auto& row : m_GpuAggregates)
    {
        if (!Contains_CaseInsensitive(Scope_Name(row.NameId), m_Filter.data())) continue;
        ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted(Scope_Name(row.NameId));
        ImGui::TableNextColumn(); ImGui::Text("%.3f", row.InclusiveMs / frames);
        ImGui::TableNextColumn(); ImGui::Text("%.3f", row.MaxFrameMs);
        ImGui::TableNextColumn(); ImGui::Text("%.3f", row.P95FrameMs);
        ImGui::TableNextColumn(); ImGui::Text("%.2f", static_cast<double>(row.Calls) / frames);
        ImGui::TableNextColumn();
        if (row.PipelineSamples != 0 && row.PipelineSamples == row.Calls)
            ImGui::Text("%.0f", static_cast<double>(row.PSInvocations) / frames);
        else ImGui::TextDisabled("--");
        ImGui::TableNextColumn();
        if (row.PipelineSamples != 0 && row.PipelineSamples == row.Calls)
            ImGui::Text("%.0f", static_cast<double>(row.VSInvocations) / frames);
        else ImGui::TextDisabled("--");
    }
    if (m_bShowUnobserved)
        for (const char* name : GPU_SCOPE_CATALOG)
            if (Contains_CaseInsensitive(name, m_Filter.data()) &&
                std::none_of(m_GpuAggregates.begin(), m_GpuAggregates.end(),
                    [&](const auto& row) { return name == std::string_view(Scope_Name(row.NameId)); }))
                Unobserved_Row(name, 7);
    ImGui::EndTable();
}

void Client::CProfilerTool::Render_Counters() const
{
    if (!m_bLiveValid) { ImGui::TextDisabled("No captured frame yet."); return; }
    if (!ImGui::BeginChild("##WorkloadScroll")) { ImGui::EndChild(); return; }
    const auto& animation = m_Live.Animation;
    ImGui::Text("CPU frame %llu", static_cast<unsigned long long>(m_Live.FrameNumber));
    ImGui::TextWrapped("CPU work categories accumulate every main-thread call while Capture is on, independently of Detailed CPU scopes and its sample limit. Times are inclusive: parent and child rows overlap and must not be added together. Calls measure attempts, not visible draws.");
    if (ImGui::BeginTable("##CpuWorkCategories", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH))
    {
        ImGui::TableSetupColumn("CPU work");
        ImGui::TableSetupColumn("Calls (latest frame)");
        ImGui::TableSetupColumn("Inclusive ms");
        ImGui::TableHeadersRow();
        for (size_t i = 0; i < m_Live.CpuWork.size(); ++i)
        {
            const auto& work = m_Live.CpuWork[i];
            ImGui::TableNextRow(); ImGui::TableNextColumn();
            ImGui::TextUnformatted(Engine::CProfiler::Get_WorkName(static_cast<Engine::EProfilerWork>(i)));
            ImGui::TableNextColumn(); ImGui::Text("%llu", static_cast<unsigned long long>(work.Calls));
            ImGui::TableNextColumn(); ImGui::Text("%.3f", work.CpuMs);
        }
        ImGui::EndTable();
    }
    ImGui::Text("Animation evaluation: %.3f ms | %llu calls / %llu models", animation.CpuMs,
        static_cast<unsigned long long>(animation.UpdateCalls), static_cast<unsigned long long>(animation.UpdatedModels));
    ImGui::Text("Updated, not submitted: %llu models / %.3f ms", static_cast<unsigned long long>(animation.NotSubmittedUpdatedModels), animation.NotSubmittedCpuMs);
    if (animation.DroppedSamples) ImGui::Text("Animation sample limit: %llu omitted in this frame", static_cast<unsigned long long>(animation.DroppedSamples));
    ImGui::TextWrapped("Not submitted means no successful model draw in this frame. It can include hidden actors, tool previews or offscreen objects; it is not a frustum test. History sampling is measured separately in CPU sections.");
    if (m_Live.GpuValid)
        ImGui::Text("GPU frame %llu: IA vertices %llu | VS %llu | PS %llu | primitives %llu",
            static_cast<unsigned long long>(m_Live.GpuFrameNumber),
            static_cast<unsigned long long>(m_Live.Pipeline.IAVertices), static_cast<unsigned long long>(m_Live.Pipeline.VSInvocations),
            static_cast<unsigned long long>(m_Live.Pipeline.PSInvocations), static_cast<unsigned long long>(m_Live.Pipeline.IAPrimitives));
    ImGui::TextWrapped("Navigation timings below are Client-side queries. Authoritative Server navigation runs in another process. Texture cache counters have no producer yet and are shown as N/A.");
    ImGui::TextWrapped("Map visibility cache hits reuse the last camera result; zero rebuild candidates does not mean culling is disabled. NPC authored-hidden updates count visibility/composition suppression, not frustum visibility. Ambient invalidated-bound updates count deferred effects whose admitted bound was invalidated by a root change; effects that never admitted a bound are excluded.");
    if (ImGui::BeginTable("##WorkCounters", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH))
    {
        ImGui::TableSetupColumn("Counter"); ImGui::TableSetupColumn("Latest CPU frame"); ImGui::TableHeadersRow();
        for (size_t i = 0; i < COUNTER_LABELS.size(); ++i)
        {
            ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted(COUNTER_LABELS[i]);
            ImGui::TableNextColumn();
            if (i >= static_cast<size_t>(Engine::EProfilerCounter::TextureRequests) && i <= static_cast<size_t>(Engine::EProfilerCounter::TextureEstimatedGpuBytes))
                ImGui::TextDisabled("N/A");
            else ImGui::Text("%llu", static_cast<unsigned long long>(m_Live.Counters[i]));
        }
        ImGui::EndTable();
    }
    ImGui::EndChild();
}

void Client::CProfilerTool::Render_LongOperations()
{
    ImGui::TextWrapped("Completed calls of %.0f ms or more, newest first. Worker calls spanning frames belong to the frame in which they ended. They cannot be summed as main-thread frame time.", Engine::CProfiler::LONG_OPERATION_THRESHOLD_MS);
    if (!ImGui::BeginTable("##LongOperations", 4, TABLE_FLAGS)) return;
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("Section", ImGuiTableColumnFlags_WidthStretch, 3.f);
    for (const char* label : {"Thread", "Duration ms", "Completion frame"}) ImGui::TableSetupColumn(label);
    ImGui::TableHeadersRow();
    for (auto row = m_LongOperations.rbegin(); row != m_LongOperations.rend(); ++row)
    {
        if (!Contains_CaseInsensitive(Scope_Name(row->NameId), m_Filter.data())) continue;
        ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted(Scope_Name(row->NameId));
        ImGui::TableNextColumn(); ImGui::TextUnformatted(Thread_Label(row->ThreadId).c_str());
        ImGui::TableNextColumn(); ImGui::Text("%.3f", row->DurationMs);
        ImGui::TableNextColumn(); ImGui::Text("%llu", static_cast<unsigned long long>(row->FrameNumber));
    }
    ImGui::EndTable();
}


bool_t Client::CProfilerTool::Refresh_CaptureFiles()
{
    m_bCaptureFilesLoaded = true;
    if (!CProfilerCaptureIO::List_JsonFiles(CProfilerCaptureIO::Get_CaptureDirectory(),
        m_CaptureFiles, &m_strCaptureFilesStatus)) return false;
    if (std::none_of(m_CaptureFiles.begin(), m_CaptureFiles.end(),
        [&](const auto& file) { return file.StableId == m_strSelectedCaptureId; }))
        m_strSelectedCaptureId.clear();
    return true;
}

void Client::CProfilerTool::Render_CaptureFiles()
{
    if (ImGui::Button("Refresh files")) Refresh_CaptureFiles();
    ImGui::SameLine();
    const auto selected = std::find_if(m_CaptureFiles.begin(), m_CaptureFiles.end(),
        [&](const auto& file) { return file.StableId == m_strSelectedCaptureId; });
    ImGui::BeginDisabled(selected == m_CaptureFiles.end() || m_Exporter.IsSaving());
    if (ImGui::Button("Delete selected JSON") && selected != m_CaptureFiles.end())
    {
        const FProfilerCaptureFile file = *selected;
        std::string error;
        if (CProfilerCaptureIO::Delete_JsonFile(CProfilerCaptureIO::Get_CaptureDirectory(), file, &error))
        {
            m_strSelectedCaptureId.clear();
            std::erase_if(m_CaptureFiles, [&](const auto& row) { return row.StableId == file.StableId; });
            const bool refreshed = Refresh_CaptureFiles();
            m_strCaptureFilesStatus = "Deleted " + file.DisplayName +
                (refreshed ? std::string{} : ". Refresh failed: " + m_strCaptureFilesStatus);
        }
        else m_strCaptureFilesStatus = error;
    }
    ImGui::EndDisabled();
    ImGui::SameLine(); ImGui::TextDisabled("%zu files", m_CaptureFiles.size());
    ImGui::TextWrapped("%s", Capture_PathLabel(CProfilerCaptureIO::Get_CaptureDirectory()).c_str());
    if (!m_strCaptureFilesStatus.empty()) ImGui::TextWrapped("%s", m_strCaptureFilesStatus.c_str());
    if (!ImGui::BeginTable("##SavedCaptures", 3, TABLE_FLAGS)) return;
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("File", ImGuiTableColumnFlags_WidthStretch, 5.f);
    ImGui::TableSetupColumn("Size (KiB)"); ImGui::TableSetupColumn("Modified"); ImGui::TableHeadersRow();
    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(m_CaptureFiles.size()));
    while (clipper.Step())
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
        {
            const auto& file = m_CaptureFiles[static_cast<size_t>(i)];
            ImGui::TableNextRow(); ImGui::TableNextColumn();
            ImGui::PushID(file.StableId.c_str());
            const ImVec2 labelPosition = ImGui::GetCursorScreenPos();
            if (ImGui::Selectable("##CaptureFile", file.StableId == m_strSelectedCaptureId,
                ImGuiSelectableFlags_SpanAllColumns, ImVec2(0.f, ImGui::GetTextLineHeight())))
                m_strSelectedCaptureId = file.StableId;
            ImGui::GetWindowDrawList()->AddText(labelPosition, ImGui::GetColorU32(ImGuiCol_Text), file.DisplayName.c_str());
            ImGui::PopID();
            ImGui::TableNextColumn(); ImGui::Text("%.1f", file.SizeBytes / 1024.0);
            ImGui::TableNextColumn();
            const FILETIME utc{static_cast<DWORD>(file.LastWriteTicks), static_cast<DWORD>(file.LastWriteTicks >> 32u)};
            FILETIME local{}; SYSTEMTIME time{};
            if (FileTimeToLocalFileTime(&utc, &local) && FileTimeToSystemTime(&local, &time))
                ImGui::Text("%04u-%02u-%02u %02u:%02u:%02u", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
            else ImGui::TextDisabled("--");
        }
    ImGui::EndTable();
}
```

#### `Client/Private/ProfilerCaptureIO.cpp`

후보 SHA256: `baf8579117e77acf3737f99d7bb5edc7d42dc0b2b899c78df312d76d2dfa8fca`

```cpp
#include "ProfilerCaptureIO.h"

#include <array>
#include <algorithm>
#include <cwctype>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <locale>
#include <sstream>
#include <system_error>

namespace
{
    struct FMovementRing final
    {
        std::array<Client::FProfilerMovementSample, Client::CProfilerCaptureIO::MAX_MOVEMENT_SAMPLES> Samples{};
        size_t Next = 0, Count = 0;
        uint64_t Accepted = 0, Overwritten = 0, Rejected = 0, LastOverwrittenTick = 0;
    };
    FMovementRing MovementRing; // Only the Client main thread reads or writes this ring.

    const char* MovementKindName(Client::EProfilerMovementKind kind)
    {
        switch (kind)
        {
        case Client::EProfilerMovementKind::Frame: return "frame";
        case Client::EProfilerMovementKind::Snapshot: return "snapshot";
        case Client::EProfilerMovementKind::Command: return "command";
        }
        return nullptr;
    }

    bool ValidMovementSample(const Client::FProfilerMovementSample& sample)
    {
        const auto finite = [](const auto& values)
        { return std::all_of(values.begin(), values.end(), [](float value) { return std::isfinite(value); }); };
        return MovementKindName(sample.Kind) != nullptr && std::isfinite(sample.DeltaSeconds) &&
            sample.DeltaSeconds >= 0.f && std::isfinite(sample.MotionSeconds) && sample.MotionSeconds >= 0.f && finite(sample.Before) && finite(sample.After) &&
            finite(sample.Authority) && finite(sample.Waypoint);
    }

	constexpr std::array<const char*,
		static_cast<size_t>(Engine::EProfilerCounter::Count)>
		CounterNames = {
			"drawCalls",
			"instancedDrawCalls",
			"instances",
			"indices",
			"renderSubmissionsPriority",
			"renderSubmissionsShadow",
			"renderSubmissionsNonBlend",
			"renderSubmissionsBlend",
			"mapPlacements",
			"mapVisibleInstances",
			"mapBatchCount",
			"mapFallbackObjects",
			"textureRequests",
			"texturePathHits",
			"textureContentHits",
			"textureUniqueSrvs",
			"textureEstimatedGpuBytes",
			"navigationQueries",
			"navigationExpandedNodes",
			"navigationQueryMicroseconds",
			"navigationPathCells",
			"sceneColorCopies",
			"sceneColorCopyBytes",
			"imGuiDrawLists",
			"imGuiVertices",
			"imGuiIndices",
			"imGuiDrawCommands",
			"imGuiDrawCalls",
			"imGuiCallbacks",
			"imGuiRenderWindows",
			"imGuiActiveWindows",
			"imGuiPlatformViewports",
			"imGuiVertexUploadBytes",
			"imGuiIndexUploadBytes",
			"imGuiConstantUploadBytes",
			"imGuiTextureUploadBytes",
			"imGuiBufferGrowths",
			"imGuiTextureCreates",
			"imGuiTextureUpdates",
			"imGuiDeviceObjectBuilds",
			"imGuiBufferMapFailures",
			"pickingReadbacks",
			"pickingReadbackBytes",
            "indirectDrawCalls",
            "indirectIndexUpperBound",
            "shadowCacheHits", "shadowCacheMisses", "shadowStaticCasters", "shadowDynamicCasters",
            "mapCullingCandidates", "mapCullingVisible", "mapLod0Draws", "mapLod1Draws", "mapLod2Draws",
            "mapLodSourceIndices", "mapLodSubmittedIndices",
            "lightRecords", "lightDrawCalls", "lightUploadBytes",
            "mapLodAvailableDraws",
            "lightCullingCandidates", "lightCullingRejected",
            "effectBoundsCandidates", "effectBoundsCulled",
            "effectMarkerSamples", "effectMarkerHistoryRequests",
            "effectAmbientSuspended", "effectAmbientAdvanced",
            "imguiPresentAttempts", "imguiPresentBusy", "imguiPresentFailures", "imguiPresentOccluded",
            "mapBatchVisibilityCacheHits", "mapBatchVisibilityRebuilds",
            "mapBatchEmptyRenders", "mapBatchVisibleRenders", "mapBatchBoundsRejected",
            "mapBatchUploadBytes", "npcAuthoredHiddenUpdates", "ambientUnboundedUpdates",
            "npcCullingCandidates", "npcCulled", "npcDeferredPoseEvaluations",
	};

	const char* GpuStatusName(const Engine::EProfilerGpuFrameStatus Status)
	{
		switch (Status)
		{
		case Engine::EProfilerGpuFrameStatus::Unsupported: return "unsupported";
		case Engine::EProfilerGpuFrameStatus::Pending: return "pending";
		case Engine::EProfilerGpuFrameStatus::Valid: return "valid";
		case Engine::EProfilerGpuFrameStatus::Disjoint: return "disjoint";
		case Engine::EProfilerGpuFrameStatus::Dropped: return "dropped";
		case Engine::EProfilerGpuFrameStatus::Error: return "error";
		}
		return nullptr;
	}

	string EscapeJson(const string& Value)
	{
		ostringstream Stream;
		for (const unsigned char Character : Value)
		{
			switch (Character)
			{
			case '"': Stream << "\\\""; break;
			case '\\': Stream << "\\\\"; break;
			case '\b': Stream << "\\b"; break;
			case '\f': Stream << "\\f"; break;
			case '\n': Stream << "\\n"; break;
			case '\r': Stream << "\\r"; break;
			case '\t': Stream << "\\t"; break;
			default:
				if (Character < 0x20)
				{
					Stream << "\\u"
						<< hex << setw(4) << setfill('0')
						<< static_cast<uint32_t>(Character)
						<< dec << setfill(' ');
				}
				else
				{
					Stream << Character;
				}
				break;
			}
		}
		return Stream.str();
	}

	void SetError(string* pOutError, const string& Error)
	{
		if (nullptr != pOutError)
			*pOutError = Error;
	}

	struct FTemporaryCapture final
	{
		filesystem::path Path;
		~FTemporaryCapture()
		{
			error_code Ignored;
			filesystem::remove(Path, Ignored);
		}
	};

	bool Cancelled(const std::atomic_bool* pCancel, string* pOutError)
	{
		if (nullptr == pCancel || !pCancel->load(std::memory_order_relaxed))
			return false;
		SetError(pOutError, "Profiler capture save cancelled during shutdown.");
		return true;
	}

    void WriteDistribution(std::ostream& stream, std::vector<double> values)
    {
        std::sort(values.begin(), values.end());
        if (values.empty())
        {
            stream << "{\"samples\": 0, \"meanMs\": null, \"p50Ms\": null, \"p95Ms\": null, \"p99Ms\": null, \"maxMs\": null}";
            return;
        }
        double total = 0.0;
        for (double value : values) total += value;
        const auto percentile = [&](double fraction)
        {
            const size_t rank = static_cast<size_t>(std::ceil(fraction * values.size()));
            return values[(std::max)(size_t{1}, rank) - 1];
        };
        stream << "{\"samples\": " << values.size() << ", \"meanMs\": " << total / values.size()
            << ", \"p50Ms\": " << percentile(.50) << ", \"p95Ms\": " << percentile(.95)
            << ", \"p99Ms\": " << percentile(.99) << ", \"maxMs\": " << values.back() << "}";
    }

    void WriteCaptureWindow(std::ostream& stream, const Engine::FProfilerCaptureSnapshot& snapshot)
    {
        const auto& window = snapshot.CaptureWindow;
        stream << "  \"captureWindow\": {\n    \"requestedFrames\": " << window.RequestedFrames
            << ",\n    \"retainedFrames\": " << window.RetainedFrames
            << ",\n    \"savedFrames\": " << snapshot.Frames.size()
            << ",\n    \"recordedFramesSinceReset\": " << window.RetainedFrames + window.EvictedFramesSinceReset
            << ",\n    \"excludedRetainedFrames\": " << window.ExcludedRetainedFrames
            << ",\n    \"evictedFramesSinceReset\": " << window.EvictedFramesSinceReset
            << ",\n    \"firstRetainedFrameNumber\": " << window.FirstRetainedFrameNumber
            << ",\n    \"lastRetainedFrameNumber\": " << window.LastRetainedFrameNumber
            << ",\n    \"firstSavedFrameNumber\": " << window.FirstSavedFrameNumber
            << ",\n    \"lastSavedFrameNumber\": " << window.LastSavedFrameNumber
            << ",\n    \"excludedMaxFrameIntervalMs\": " << window.ExcludedMaxFrameIntervalMs
            << ",\n    \"note\": \"Newest completed frames only. Excluded retained frames can still be saved with a larger window; evicted frames cannot. Frame N interval runs from Begin(N-1) to Begin(N), while frame N CPU scopes measure work after Begin(N).\"\n  },\n";
    }

    void WriteSummary(std::ostream& stream, const Engine::FProfilerCaptureSnapshot& snapshot)
    {
        std::vector<double> intervals, cpu, gpu;
        size_t cpuPartial = 0, gpuPartial = 0, gpuPending = 0, over60 = 0, over30 = 0, over100 = 0;
        uint64_t omittedCpuScopes = 0;
        for (const auto& frame : snapshot.Frames)
        {
            if (std::isfinite(frame.FrameIntervalMs) && frame.FrameIntervalMs > 0.0)
            {
                intervals.push_back(frame.FrameIntervalMs);
                over60 += frame.FrameIntervalMs > 1000.0 / 60.0;
                over30 += frame.FrameIntervalMs > 1000.0 / 30.0;
                over100 += frame.FrameIntervalMs > 100.0;
            }
            if (std::isfinite(frame.CpuFrameMs) && frame.CpuFrameMs >= 0.0) cpu.push_back(frame.CpuFrameMs);
            omittedCpuScopes += frame.DroppedCpuScopes;
            cpuPartial += frame.DroppedCpuScopes != 0;
            gpuPending += frame.GpuStatus == Engine::EProfilerGpuFrameStatus::Pending;
            gpuPartial += frame.DroppedGpuScopes != 0;
            if (frame.GpuValid && frame.GpuStatus == Engine::EProfilerGpuFrameStatus::Valid &&
                std::isfinite(frame.GpuFrameMs) && frame.GpuFrameMs >= 0.0) gpu.push_back(frame.GpuFrameMs);
        }
        stream << "  \"summary\": {\n    \"percentileMethod\": \"nearest-rank\",\n"
            << "    \"frameCount\": " << snapshot.Frames.size() << ",\n"
            << "    \"frameInterval\": "; WriteDistribution(stream, std::move(intervals));
        stream << ",\n    \"cpuFrame\": "; WriteDistribution(stream, std::move(cpu));
        stream << ",\n    \"gpuFrameValidOnly\": "; WriteDistribution(stream, std::move(gpu));
        stream << ",\n    \"framesOver16_667Ms\": " << over60 << ",\n    \"framesOver33_333Ms\": " << over30
            << ",\n    \"framesOver100Ms\": " << over100 << ",\n    \"cpuPartialFrames\": " << cpuPartial
            << ",\n    \"windowDroppedCpuScopes\": " << omittedCpuScopes
            << ",\n    \"gpuPendingFrames\": " << gpuPending << ",\n    \"gpuPartialScopeFrames\": " << gpuPartial
            << ",\n    \"cpuScopesWithinBudget\": " << (cpuPartial == 0 ? "true" : "false")
            << ",\n    \"note\": \"Frame durations include all frames; scope attribution is incomplete in frames with drops. GPU timestamps measure elapsed intervals, not utilization. Reset-scoped drop totals can include history outside this exported window.\"\n  },\n";
    }

    template<size_t Size>
    void WriteFloatArray(std::ostream& stream, const std::array<float, Size>& values)
    {
        stream << '[';
        for (size_t i = 0; i < Size; ++i)
        {
            if (i) stream << ", ";
            if (std::isfinite(values[i])) stream << values[i]; else stream << "null";
        }
        stream << ']';
    }

    bool WriteMovement(std::ostream& stream, const Client::FProfilerCaptureContext& context,
        string* error, const std::atomic_bool* cancel)
    {
        const auto& coverage = context.MovementCoverage;
        stream << "  \"movementCoverage\": {\n    \"captured\": " << (coverage.Captured ? "true" : "false")
            << ",\n    \"capacity\": " << Client::CProfilerCaptureIO::MAX_MOVEMENT_SAMPLES
            << ",\n    \"windowBeginTick\": " << coverage.WindowBeginTick
            << ",\n    \"windowEndTick\": " << coverage.WindowEndTick
            << ",\n    \"framesWithBounds\": " << coverage.FramesWithBounds
            << ",\n    \"framesWithoutBounds\": " << coverage.FramesWithoutBounds
            << ",\n    \"windowMayBeTruncated\": " << (coverage.WindowMayBeTruncated ? "true" : "false")
            << ",\n    \"acceptedSinceReset\": " << coverage.AcceptedSinceReset
            << ",\n    \"overwrittenSinceReset\": " << coverage.OverwrittenSinceReset
            << ",\n    \"rejectedSinceReset\": " << coverage.RejectedSinceReset
            << ",\n    \"firstRetainedTick\": " << coverage.FirstRetainedTick
            << ",\n    \"lastRetainedTick\": " << coverage.LastRetainedTick
            << ",\n    \"outsideSavedFrames\": " << coverage.OutsideSavedFrames
            << ",\n    \"savedSamples\": " << context.MovementSamples.size()
            << ",\n    \"boundsSource\": \"completed-frame-main-thread-cpu-scopes\","
            << "\n    \"note\": \"Samples observe local character calls, not every rendered pixel. QPC ticks use ticksPerSecond. Only samples within a saved completed frame's main-thread scope bounds are included; scope bounds do not cover uninstrumented frame edges. Ring and rejected totals are since the last capture reset and can include history outside this window.\"\n  },\n";
        stream << "  \"movementSamples\": [\n";
        for (size_t i = 0; i < context.MovementSamples.size(); ++i)
        {
            if (i % 256 == 0 && Cancelled(cancel, error)) return false;
            const auto& sample = context.MovementSamples[i];
            if (!ValidMovementSample(sample))
            { SetError(error, "Invalid or non-finite movement capture sample."); return false; }
            stream << "    {\"kind\": \"" << MovementKindName(sample.Kind)
                << "\", \"qpcTick\": " << sample.QpcTick << ", \"frameNumber\": " << sample.FrameNumber
                << ", \"characterClass\": " << sample.CharacterClass << ", \"serverTick\": " << sample.ServerTick
                << ", \"sequence\": " << sample.Sequence << ", \"flags\": " << sample.Flags
                << ", \"disposition\": " << sample.Disposition << ", \"deltaSeconds\": " << sample.DeltaSeconds
                << ", \"motionSeconds\": " << sample.MotionSeconds
                << ", \"before\": "; WriteFloatArray(stream, sample.Before);
            stream << ", \"after\": "; WriteFloatArray(stream, sample.After);
            stream << ", \"authority\": "; WriteFloatArray(stream, sample.Authority);
            stream << ", \"waypoint\": "; WriteFloatArray(stream, sample.Waypoint);
            stream << "}" << (i + 1 < context.MovementSamples.size() ? "," : "") << "\n";
        }
        stream << "  ],\n";
        return true;
    }

    void WriteContext(std::ostream& stream, const Client::FProfilerCaptureContext& context)
    {
#ifdef _DEBUG
        constexpr const char* build = "Debug";
#else
        constexpr const char* build = "Release";
#endif
        stream << "  \"metadata\": {\n    \"sampledAtExport\": true,\n"
            << "    \"buildConfiguration\": \"" << build << "\",\n"
            << "    \"compilerMscVersion\": " << _MSC_VER << ",\n"
            << "    \"iteratorDebugLevel\": " << _ITERATOR_DEBUG_LEVEL << ",\n"
            << "    \"processId\": " << GetCurrentProcessId() << ",\n"
            << "    \"debuggerAttached\": " << (IsDebuggerPresent() ? "true" : "false") << ",\n"
            << "    \"logicalProcessors\": " << GetActiveProcessorCount(ALL_PROCESSOR_GROUPS) << ",\n"
            << "    \"runtimeContextValid\": " << (context.Valid ? "true" : "false") << ",\n"
            << "    \"adapter\": \"" << EscapeJson(context.Adapter) << "\",\n"
            << "    \"deviceCreationFlags\": " << context.DeviceCreationFlags << ",\n"
            << "    \"d3dDebugLayer\": " << ((context.DeviceCreationFlags & D3D11_CREATE_DEVICE_DEBUG) ? "true" : "false") << ",\n"
            << "    \"clientWindowForeground\": " << (context.ClientWindowForeground ? "true" : "false") << ",\n"
            << "    \"foregroundWindowOwnedByProcess\": " << (context.ProcessForeground ? "true" : "false") << ",\n"
            << "    \"windowMinimized\": " << (context.WindowMinimized ? "true" : "false") << ",\n"
            << "    \"configuredForegroundFpsLimit\": " << context.ForegroundFpsLimit << ",\n"
            << "    \"configuredBackgroundFpsLimit\": " << context.BackgroundFpsLimit << ",\n"
            << "    \"effectiveFpsLimit\": " << context.EffectiveFpsLimit << ",\n"
            << "    \"levelId\": " << context.LevelId << ",\n    \"viewport\": ";
        WriteFloatArray(stream, context.Viewport);
        stream << ",\n    \"cameraPosition\": "; WriteFloatArray(stream, context.CameraPosition);
        stream << ",\n    \"viewMatrix\": "; WriteFloatArray(stream, context.ViewMatrix);
        stream << ",\n    \"projectionMatrix\": "; WriteFloatArray(stream, context.ProjectionMatrix);
        stream << ",\n    \"shadowEnabled\": " << (context.ShadowEnabled ? "true" : "false")
            << ",\n    \"shadowWidthHeightStrength\": ";
        WriteFloatArray(stream, std::array<float, 3>{context.ShadowWidth, context.ShadowHeight, context.ShadowStrength});
        stream << ",\n    \"ssaoEnabled\": " << (context.SSAOEnabled ? "true" : "false")
            << ",\n    \"bloomEnabled\": " << (context.BloomEnabled ? "true" : "false")
            << ",\n    \"fxaaEnabled\": " << (context.FXAAEnabled ? "true" : "false")
            << ",\n    \"note\": \"Current runtime context only; historical frames may have different levels, cameras, viewport sizes, focus and settings. FPS limits are checkbox-resolved user limits (0 means disabled); effectiveFpsLimit selects by foreground process ownership and excludes the separate minimized-frame message wait.\"\n  },\n";
    }

bool SaveJsonImpl(
	const Engine::FProfilerCaptureSnapshot& Snapshot,
	const filesystem::path& OutputPath,
	string* pOutError, const std::atomic_bool* pCancel, bool ReplaceExisting,
    const Client::FProfilerCaptureContext& Context)
{
	if (Cancelled(pCancel, pOutError))
		return false;
	if (OutputPath.empty())
	{
		SetError(pOutError, "Profiler output path is empty.");
		return false;
	}

	error_code Error;
	if (!OutputPath.parent_path().empty())
		filesystem::create_directories(OutputPath.parent_path(), Error);
	if (Error)
	{
		SetError(pOutError, "Cannot create profiler capture directory: " + Error.message());
		return false;
	}

	static std::atomic_uint64_t TemporarySequence = 0;
	FTemporaryCapture Temporary{ OutputPath };
	Temporary.Path += L".tmp." + std::to_wstring(GetCurrentProcessId()) +
		L"." + std::to_wstring(TemporarySequence.fetch_add(1));

	ofstream Stream(Temporary.Path, ios::binary | ios::trunc);
	if (!Stream)
	{
		SetError(pOutError, "Cannot open profiler JSON output.");
		return false;
	}

	Stream.imbue(std::locale::classic());
	Stream << fixed << setprecision(6);
	Stream << "{\n";
	Stream << "  \"schema\": \"LostArkProfilerCapture.v3\",\n";
    WriteContext(Stream, Context);
    WriteCaptureWindow(Stream, Snapshot);
    WriteSummary(Stream, Snapshot);
    if (!WriteMovement(Stream, Context, pOutError, pCancel)) return false;
	Stream << "  \"droppedCpuScopes\": " << Snapshot.DroppedCpuScopes << ",\n";
	Stream << "  \"droppedGpuFrames\": " << Snapshot.DroppedGpuFrames << ",\n";
	Stream << "  \"droppedGpuScopes\": " << Snapshot.DroppedGpuScopes << ",\n";
	Stream << "  \"droppedModelAnimationSamples\": " << Snapshot.DroppedModelAnimationSamples << ",\n";
	Stream << "  \"gpuQueriesSupported\": " << (Snapshot.GpuQueriesSupported ? "true" : "false") << ",\n";
	Stream << "  \"gpuScopesSupported\": " << (Snapshot.GpuScopesSupported ? "true" : "false") << ",\n";
	Stream << "  \"mainThreadId\": " << Snapshot.MainThreadId << ",\n";
	Stream << "  \"ticksPerSecond\": " << Snapshot.TicksPerSecond << ",\n";
	Stream << "  \"scopeNames\": [";
	for (size_t i = 0; i < Snapshot.ScopeNames.size(); ++i)
	{
		if (0 == i % 256 && Cancelled(pCancel, pOutError))
			return false;
		if (0 != i)
			Stream << ", ";
		Stream << "\"" << EscapeJson(Snapshot.ScopeNames[i]) << "\"";
	}
	Stream << "],\n";
	Stream << "  \"frames\": [\n";

	for (size_t iFrame = 0; iFrame < Snapshot.Frames.size(); ++iFrame)
	{
		if (Cancelled(pCancel, pOutError))
			return false;
		const Engine::FProfilerFrame& Frame = Snapshot.Frames[iFrame];
		const char* pGpuStatus = GpuStatusName(Frame.GpuStatus);
		if (nullptr == pGpuStatus || !std::isfinite(Frame.CpuFrameMs) ||
			!std::isfinite(Frame.GpuFrameMs) || !std::isfinite(Frame.FrameIntervalMs) ||
			!std::isfinite(Frame.Animation.CpuMs) || !std::isfinite(Frame.Animation.NotSubmittedCpuMs))
		{
			SetError(pOutError, "Profiler frame has invalid GPU status or non-finite timing.");
			return false;
		}
		Stream << "    {\n";
		Stream << "      \"frameNumber\": " << Frame.FrameNumber << ",\n";
		Stream << "      \"cpuFrameMs\": " << Frame.CpuFrameMs << ",\n";
        Stream << "      \"droppedCpuScopes\": " << Frame.DroppedCpuScopes << ",\n";
        Stream << "      \"detailedCpuScopes\": " << (Frame.DetailedCpuScopes ? "true" : "false") << ",\n";
		Stream << "      \"frameIntervalMs\": " << Frame.FrameIntervalMs << ",\n";
		Stream << "      \"gpuFrameMs\": " << Frame.GpuFrameMs << ",\n";
		Stream << "      \"gpuValid\": " << (Frame.GpuValid ? "true" : "false") << ",\n";
		Stream << "      \"gpuStatus\": \"" << pGpuStatus << "\",\n";
		Stream << "      \"gpuLatencyFrames\": " << Frame.GpuLatencyFrames << ",\n";
		Stream << "      \"gpuScopesSupported\": " << (Frame.GpuScopesSupported ? "true" : "false") << ",\n";
		Stream << "      \"droppedGpuScopes\": " << Frame.DroppedGpuScopes << ",\n";
		Stream << "      \"animation\": {\n";
		Stream << "        \"updateCalls\": " << Frame.Animation.UpdateCalls << ",\n";
		Stream << "        \"updatedModels\": " << Frame.Animation.UpdatedModels << ",\n";
		Stream << "        \"submittedUpdatedModels\": " << Frame.Animation.SubmittedUpdatedModels << ",\n";
		Stream << "        \"notSubmittedUpdatedModels\": " << Frame.Animation.NotSubmittedUpdatedModels << ",\n";
		Stream << "        \"droppedSamples\": " << Frame.Animation.DroppedSamples << ",\n";
		Stream << "        \"cpuMs\": " << Frame.Animation.CpuMs << ",\n";
		Stream << "        \"notSubmittedCpuMs\": " << Frame.Animation.NotSubmittedCpuMs << "\n";
		Stream << "      },\n";
        Stream << "      \"cpuWork\": [\n";
        for (size_t iWork = 0; iWork < Frame.CpuWork.size(); ++iWork)
        {
            const auto& work = Frame.CpuWork[iWork];
            if (!std::isfinite(work.CpuMs) || work.CpuMs < 0.0)
            {
                SetError(pOutError, "Profiler work category has invalid timing.");
                return false;
            }
            Stream << "        {\"name\": \""
                << Engine::CProfiler::Get_WorkName(static_cast<Engine::EProfilerWork>(iWork))
                << "\", \"calls\": " << work.Calls << ", \"cpuMs\": " << work.CpuMs
                << "}" << (iWork + 1 < Frame.CpuWork.size() ? "," : "") << "\n";
        }
        Stream << "      ],\n";
		Stream << "      \"counters\": {\n";
		for (size_t iCounter = 0; iCounter < CounterNames.size(); ++iCounter)
		{
			Stream << "        \"" << CounterNames[iCounter] << "\": "
				<< Frame.Counters[iCounter]
				<< (iCounter + 1 < CounterNames.size() ? "," : "")
				<< "\n";
		}
		Stream << "      },\n";
		Stream << "      \"pipeline\": {\n";
		Stream << "        \"iaVertices\": " << Frame.Pipeline.IAVertices << ",\n";
		Stream << "        \"iaPrimitives\": " << Frame.Pipeline.IAPrimitives << ",\n";
		Stream << "        \"vsInvocations\": " << Frame.Pipeline.VSInvocations << ",\n";
		Stream << "        \"gsInvocations\": " << Frame.Pipeline.GSInvocations << ",\n";
		Stream << "        \"gsPrimitives\": " << Frame.Pipeline.GSPrimitives << ",\n";
		Stream << "        \"clipperInvocations\": " << Frame.Pipeline.CInvocations << ",\n";
		Stream << "        \"clipperPrimitives\": " << Frame.Pipeline.CPrimitives << ",\n";
		Stream << "        \"psInvocations\": " << Frame.Pipeline.PSInvocations << ",\n";
		Stream << "        \"hsInvocations\": " << Frame.Pipeline.HSInvocations << ",\n";
		Stream << "        \"dsInvocations\": " << Frame.Pipeline.DSInvocations << ",\n";
		Stream << "        \"csInvocations\": " << Frame.Pipeline.CSInvocations << "\n";
		Stream << "      },\n";
		Stream << "      \"droppedViewportPresents\": " << Frame.DroppedViewportPresents << ",\n";
		Stream << "      \"viewportPresents\": [";
		for (size_t i = 0; i < Frame.ViewportPresents.size(); ++i)
		{
			const auto& p = Frame.ViewportPresents[i];
			if (!std::isfinite(p.CpuMs) || !std::isfinite(p.X) || !std::isfinite(p.Y) ||
				!std::isfinite(p.Width) || !std::isfinite(p.Height))
			{ SetError(pOutError, "Non-finite viewport presentation sample."); return false; }
			if (i) Stream << ", ";
			Stream << "{\"viewportId\": " << p.ViewportId << ", \"x\": " << p.X
				<< ", \"y\": " << p.Y << ", \"width\": " << p.Width << ", \"height\": " << p.Height
				<< ", \"cpuMs\": " << p.CpuMs << ", \"syncInterval\": " << p.SyncInterval
				<< ", \"flags\": " << p.Flags << ", \"hresult\": " << p.Result << "}";
		}
		Stream << "],\n";
		Stream << "      \"cpuScopes\": [\n";
		for (size_t iScope = 0; iScope < Frame.CpuScopes.size(); ++iScope)
		{
			if (0 == iScope % 256 && Cancelled(pCancel, pOutError))
				return false;
			const Engine::FProfilerScopeSample& Scope = Frame.CpuScopes[iScope];
			Stream << "        {\"nameId\": " << Scope.NameId
				<< ", \"depth\": " << Scope.Depth
				<< ", \"threadId\": " << Scope.ThreadId
				<< ", \"beginTick\": " << Scope.BeginTick
				<< ", \"endTick\": " << Scope.EndTick << "}"
				<< (iScope + 1 < Frame.CpuScopes.size() ? "," : "")
				<< "\n";
		}
		Stream << "      ],\n";
		Stream << "      \"gpuScopes\": [\n";
		for (size_t iScope = 0; iScope < Frame.GpuScopes.size(); ++iScope)
		{
			if (0 == iScope % 256 && Cancelled(pCancel, pOutError))
				return false;
			const Engine::FProfilerGpuScopeSample& Scope = Frame.GpuScopes[iScope];
			if (!std::isfinite(Scope.BeginMs) || !std::isfinite(Scope.EndMs) ||
				!std::isfinite(Scope.DurationMs))
			{
				SetError(pOutError, "Profiler GPU scope has non-finite timing.");
				return false;
			}
			Stream << "        {\"nameId\": " << Scope.NameId
				<< ", \"depth\": " << Scope.Depth
				<< ", \"beginMs\": " << Scope.BeginMs
				<< ", \"endMs\": " << Scope.EndMs
				<< ", \"durationMs\": " << Scope.DurationMs
				<< ", \"pipelineValid\": " << (Scope.PipelineValid ? "true" : "false")
				<< ", \"psInvocations\": " << Scope.PSInvocations
				<< ", \"vsInvocations\": " << Scope.VSInvocations << "}"
				<< (iScope + 1 < Frame.GpuScopes.size() ? "," : "") << "\n";
		}
		Stream << "      ]\n";
		Stream << "    }"
			<< (iFrame + 1 < Snapshot.Frames.size() ? "," : "")
			<< "\n";
	}

	Stream << "  ]\n";
	Stream << "}\n";
	Stream.close();
	if (!Stream)
	{
		SetError(pOutError, "Failed while writing profiler JSON.");
		return false;
	}

	if (Cancelled(pCancel, pOutError))
		return false;
	// Both paths are in the same directory. Commit only the complete file;
	// never delete an existing capture before the replacement succeeds.
	if (!MoveFileExW(Temporary.Path.c_str(), OutputPath.c_str(),
		(ReplaceExisting ? MOVEFILE_REPLACE_EXISTING : 0u) | MOVEFILE_WRITE_THROUGH))
	{
		SetError(pOutError, "Cannot finalize profiler JSON output: " +
			std::error_code(static_cast<int>(GetLastError()), std::system_category()).message());
		return false;
	}

	if (nullptr != pOutError)
		pOutError->clear();
	return true;
}

	bool SaveJsonSafely(const Engine::FProfilerCaptureSnapshot& Snapshot,
		const filesystem::path& OutputPath, string* pOutError,
		const std::atomic_bool* pCancel = nullptr, bool ReplaceExisting = true,
        const Client::FProfilerCaptureContext& Context = {})
	{
		try
		{
			return SaveJsonImpl(Snapshot, OutputPath, pOutError, pCancel, ReplaceExisting, Context);
		}
		catch (const std::exception& Exception)
		{
			SetError(pOutError, "Profiler capture save failed: " + string(Exception.what()));
			return false;
		}
	}
}

void Client::CProfilerCaptureIO::Record_MovementSample(const Engine::CProfiler* profiler,
    FProfilerMovementSample sample) noexcept
{
    if (!profiler || !profiler->Is_Enabled()) return;
    if (GetCurrentThreadId() != profiler->Get_MainThreadId()) return;
    if (!ValidMovementSample(sample)) { ++MovementRing.Rejected; return; }
    LARGE_INTEGER tick{};
    if (!QueryPerformanceCounter(&tick) || tick.QuadPart <= 0) { ++MovementRing.Rejected; return; }
    sample.QpcTick = static_cast<uint64_t>(tick.QuadPart);
    sample.FrameNumber = 0;
    if (MovementRing.Count == MAX_MOVEMENT_SAMPLES)
    {
        ++MovementRing.Overwritten;
        MovementRing.LastOverwrittenTick = MovementRing.Samples[MovementRing.Next].QpcTick;
    }
    else ++MovementRing.Count;
    MovementRing.Samples[MovementRing.Next] = sample;
    MovementRing.Next = (MovementRing.Next + 1) % MAX_MOVEMENT_SAMPLES;
    ++MovementRing.Accepted;
}

void Client::CProfilerCaptureIO::Reset_MovementSamples() noexcept
{
    MovementRing.Next = MovementRing.Count = 0;
    MovementRing.Accepted = MovementRing.Overwritten = MovementRing.Rejected = MovementRing.LastOverwrittenTick = 0;
}

void Client::CProfilerCaptureIO::Copy_MovementSamples(const Engine::FProfilerCaptureSnapshot& snapshot,
    FProfilerCaptureContext& context)
{
    context.MovementSamples.clear();
    context.MovementCoverage = {};
    if (GetCurrentThreadId() != snapshot.MainThreadId) return;
    auto& coverage = context.MovementCoverage;
    coverage.Captured = true;
    coverage.AcceptedSinceReset = MovementRing.Accepted;
    coverage.OverwrittenSinceReset = MovementRing.Overwritten;
    coverage.RejectedSinceReset = MovementRing.Rejected;
    struct FFrameBounds final { uint64_t Number, Begin, End; };
    std::vector<FFrameBounds> bounds;
    bounds.reserve(snapshot.Frames.size());
    for (const auto& frame : snapshot.Frames)
    {
        uint64_t begin = UINT64_MAX, end = 0;
        for (const auto& scope : frame.CpuScopes)
        {
            if (scope.ThreadId != snapshot.MainThreadId || !scope.BeginTick || scope.EndTick < scope.BeginTick) continue;
            begin = (std::min)(begin, scope.BeginTick);
            end = (std::max)(end, scope.EndTick);
        }
        if (!end) { ++coverage.FramesWithoutBounds; continue; }
        bounds.push_back({frame.FrameNumber, begin, end});
        ++coverage.FramesWithBounds;
        coverage.WindowBeginTick = coverage.WindowBeginTick ? (std::min)(coverage.WindowBeginTick, begin) : begin;
        coverage.WindowEndTick = (std::max)(coverage.WindowEndTick, end);
    }
    coverage.WindowMayBeTruncated = coverage.FramesWithoutBounds != 0 || MovementRing.Rejected != 0 ||
        (coverage.WindowBeginTick && MovementRing.LastOverwrittenTick >= coverage.WindowBeginTick);
    const size_t first = (MovementRing.Next + MAX_MOVEMENT_SAMPLES - MovementRing.Count) % MAX_MOVEMENT_SAMPLES;
    context.MovementSamples.reserve(MovementRing.Count);
    for (size_t i = 0; i < MovementRing.Count; ++i)
    {
        auto sample = MovementRing.Samples[(first + i) % MAX_MOVEMENT_SAMPLES];
        if (!i) coverage.FirstRetainedTick = sample.QpcTick;
        coverage.LastRetainedTick = sample.QpcTick;
        const auto frame = std::find_if(bounds.begin(), bounds.end(), [&](const auto& range)
        { return sample.QpcTick >= range.Begin && sample.QpcTick <= range.End; });
        if (frame == bounds.end()) { ++coverage.OutsideSavedFrames; continue; }
        sample.FrameNumber = frame->Number;
        context.MovementSamples.push_back(sample);
    }
}

bool_t Client::CProfilerCaptureIO::Save_Json(
	const Engine::FProfilerCaptureSnapshot& Snapshot,
	const filesystem::path& OutputPath, string* pOutError, const FProfilerCaptureContext& Context)
{
	return SaveJsonSafely(Snapshot, OutputPath, pOutError, nullptr, true, Context);
}

struct Client::CProfilerCaptureExporter::FSaveJob final
{
	FProfilerCaptureSaveResult Result;
	std::atomic_bool Cancel = false;
};

Client::CProfilerCaptureExporter::~CProfilerCaptureExporter()
{
	if (!m_Worker.joinable())
		return;
	m_Job->Cancel.store(true, std::memory_order_relaxed);
	const HANDLE WorkerHandle = m_Worker.native_handle();
	CancelSynchronousIo(WorkerHandle);
	const DWORD WaitResult = WaitForSingleObject(WorkerHandle, 5000);
	if (WAIT_OBJECT_0 != WaitResult)
	{
		// Matches the Loader shutdown policy: never kill a worker and keep
		// running with partially owned state, or wait forever during exit.
		const DWORD ExitCode = WAIT_TIMEOUT == WaitResult ? ERROR_TIMEOUT : GetLastError();
		OutputDebugStringA("[Profiler] Capture writer exceeded shutdown deadline or wait failed.\n");
		if (!TerminateProcess(GetCurrentProcess(),
			ERROR_SUCCESS == ExitCode ? ERROR_INVALID_HANDLE : ExitCode))
			std::terminate();
		__assume(0);
	}
	m_Worker.join();
}

bool_t Client::CProfilerCaptureExporter::BeginSave(
	Engine::FProfilerCaptureSnapshot&& Snapshot,
	filesystem::path OutputPath, string* pOutError, FProfilerCaptureContext Context)
{
	if (IsSaving())
	{
		SetError(pOutError, "A profiler capture save is already in progress.");
		return false;
	}
	if (OutputPath.empty())
	{
		SetError(pOutError, "Profiler output path is empty.");
		return false;
	}
	try
	{
		auto Job = std::make_shared<FSaveJob>();
		Job->Result.OutputPath = std::move(OutputPath);
		// Capturing the snapshot in the worker also releases its large vectors
		// on that thread, rather than stalling the next main-thread Poll.
		m_Worker = std::thread([Job, Captured = std::move(Snapshot), Context = std::move(Context)]()
		{
			Job->Result.Succeeded = SaveJsonSafely(Captured,
				Job->Result.OutputPath, &Job->Result.Error, &Job->Cancel, false, Context);
			OutputDebugStringA(Job->Result.Succeeded ?
				"[Profiler] JSON capture save completed.\n" :
				"[Profiler] JSON capture save failed or was cancelled.\n");
			if (!Job->Result.Succeeded)
				OutputDebugStringA(Job->Result.Error.c_str());
		});
		m_Job = std::move(Job);
	}
	catch (const std::exception& Exception)
	{
		SetError(pOutError, "Cannot start profiler capture writer: " + string(Exception.what()));
		return false;
	}
	if (nullptr != pOutError)
		pOutError->clear();
	return true;
}

bool_t Client::CProfilerCaptureExporter::IsSaving() const noexcept
{
	// Includes a finished save until Poll consumes its result.
	return m_Worker.joinable();
}

bool_t Client::CProfilerCaptureExporter::Poll(FProfilerCaptureSaveResult& OutResult)
{
	if (!m_Worker.joinable())
		return false;
	const DWORD WaitResult = WaitForSingleObject(m_Worker.native_handle(), 0);
	if (WAIT_TIMEOUT == WaitResult)
		return false;
	if (WAIT_OBJECT_0 != WaitResult)
	{
		const DWORD ExitCode = GetLastError();
		OutputDebugStringA("[Profiler] Capture writer completion wait failed.\n");
		if (!TerminateProcess(GetCurrentProcess(),
			ERROR_SUCCESS == ExitCode ? ERROR_INVALID_HANDLE : ExitCode))
			std::terminate();
		__assume(0);
	}
	m_Worker.join();
	OutResult = std::move(m_Job->Result);
	m_Job.reset();
	return true;
}

filesystem::path Client::CProfilerCaptureIO::Get_CaptureDirectory()
{
	wchar_t ModulePath[32768]{};
	const DWORD Length = GetModuleFileNameW(nullptr, ModulePath, static_cast<DWORD>(size(ModulePath)));
	const filesystem::path Base = Length && Length < size(ModulePath) ?
		filesystem::path(ModulePath).parent_path() : filesystem::current_path();
	return (Base / L".." / L"ProfilerCaptures").lexically_normal();
}

namespace
{
	struct FCaptureHandle final
	{
		HANDLE Value = INVALID_HANDLE_VALUE;
		~FCaptureHandle() { if (Value != INVALID_HANDLE_VALUE) CloseHandle(Value); }
	};

	uint64_t CaptureTicks(const FILETIME& Time)
	{ return (uint64_t(Time.dwHighDateTime) << 32u) | Time.dwLowDateTime; }

	string Utf8Path(const filesystem::path& Path)
	{
		const auto Text = Path.u8string();
		return string(Text.begin(), Text.end());
	}

	bool CaptureError(string* Error, const char* Operation)
	{
		SetError(Error, string(Operation) + ": " +
			std::error_code(static_cast<int>(GetLastError()), std::system_category()).message());
		return false;
	}

	bool CaptureFinalPath(HANDLE Handle, filesystem::path& Path, string* Error)
	{
		wchar_t Buffer[32768]{};
		const DWORD Length = GetFinalPathNameByHandleW(Handle, Buffer, static_cast<DWORD>(size(Buffer)), FILE_NAME_NORMALIZED);
		if (!Length || Length >= size(Buffer)) return CaptureError(Error, "Cannot resolve capture path");
		Path = filesystem::path(Buffer).lexically_normal();
		return true;
	}

	bool OpenCaptureDirectory(const filesystem::path& Directory, FCaptureHandle& Handle,
		filesystem::path& FinalPath, string* Error)
	{
		// Deny renaming this directory until the list/delete operation completes.
		Handle.Value = CreateFileW(Directory.c_str(), FILE_READ_ATTRIBUTES,
			FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, nullptr);
		if (Handle.Value == INVALID_HANDLE_VALUE) return CaptureError(Error, "Cannot open capture directory");
		BY_HANDLE_FILE_INFORMATION Info{};
		if (!GetFileInformationByHandle(Handle.Value, &Info)) return CaptureError(Error, "Cannot inspect capture directory");
		if (!(Info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || (Info.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
		{ SetError(Error, "Capture directory must be a regular directory, not a link."); return false; }
		return CaptureFinalPath(Handle.Value, FinalPath, Error);
	}

	bool IsCaptureFileName(const filesystem::path& Name)
	{
		return !Name.empty() && !Name.has_root_path() && Name == Name.filename() &&
			Name.native().find_first_of(L"/\\:") == std::wstring::npos &&
			_wcsicmp(Name.extension().c_str(), L".json") == 0;
	}

	bool ReadCaptureFile(const filesystem::path& Directory, const filesystem::path& FinalDirectory,
		const filesystem::path& Name, bool ForDelete, FCaptureHandle& Handle,
		Client::FProfilerCaptureFile& Out, string* Error)
	{
		if (!IsCaptureFileName(Name)) { SetError(Error, "Select a JSON file inside the capture directory."); return false; }
		Handle.Value = CreateFileW((Directory / Name).c_str(), FILE_READ_ATTRIBUTES | (ForDelete ? DELETE : 0u),
			ForDelete ? FILE_SHARE_READ : FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			nullptr, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, nullptr);
		if (Handle.Value == INVALID_HANDLE_VALUE) return CaptureError(Error, "Cannot open selected capture");
		BY_HANDLE_FILE_INFORMATION Info{};
		if (!GetFileInformationByHandle(Handle.Value, &Info)) return CaptureError(Error, "Cannot inspect selected capture");
		filesystem::path FinalFile;
		if (!CaptureFinalPath(Handle.Value, FinalFile, Error)) return false;
		if ((Info.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) ||
			_wcsicmp(FinalFile.parent_path().c_str(), FinalDirectory.c_str()) != 0)
		{ SetError(Error, "Selected capture is not a regular file inside this directory."); return false; }
		Out.FileName = Name;
		Out.DisplayName = Utf8Path(Name);
		Out.VolumeSerial = Info.dwVolumeSerialNumber;
		Out.FileIndex = (uint64_t(Info.nFileIndexHigh) << 32u) | Info.nFileIndexLow;
		Out.SizeBytes = (uint64_t(Info.nFileSizeHigh) << 32u) | Info.nFileSizeLow;
		Out.LastWriteTicks = CaptureTicks(Info.ftLastWriteTime);
		Out.StableId = std::to_string(Out.VolumeSerial) + ":" + std::to_string(Out.FileIndex) + ":" + Out.DisplayName;
		return true;
	}

	bool CaptureName(std::string_view Name, std::wstring& Wide, string* Error)
	{
		if (Name.empty()) { Wide = L"profiler"; return true; }
		if (Name.size() > 240u) { SetError(Error, "Capture name is too long (maximum 80 UTF-16 characters). "); return false; }
		const int Count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, Name.data(), static_cast<int>(Name.size()), nullptr, 0);
		if (Count <= 0 || Count > 80) { SetError(Error, "Capture name must be valid UTF-8 and at most 80 UTF-16 characters."); return false; }
		Wide.resize(static_cast<size_t>(Count));
		MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, Name.data(), static_cast<int>(Name.size()), Wide.data(), Count);
		if (Wide == L"." || Wide == L".." || Wide.back() == L'.' || Wide.back() == L' ' ||
			std::any_of(Wide.begin(), Wide.end(), [](wchar_t C) { return C < 32 || C == 127 || std::wstring_view(L"<>:\"/\\|?*").find(C) != std::wstring_view::npos; }))
		{ SetError(Error, "Capture name must be a file label without path separators, reserved characters, or trailing dots/spaces."); return false; }
		std::wstring Stem = Wide.substr(0, Wide.find(L'.'));
		while (!Stem.empty() && (Stem.back() == L' ' || Stem.back() == L'.')) Stem.pop_back();
		std::transform(Stem.begin(), Stem.end(), Stem.begin(), [](wchar_t C) { return static_cast<wchar_t>(std::towupper(C)); });
		const bool DeviceNumber = Stem.size() == 4u && (Stem.starts_with(L"COM") || Stem.starts_with(L"LPT")) &&
			((Stem[3] >= L'1' && Stem[3] <= L'9') || Stem[3] == L'\u00b9' || Stem[3] == L'\u00b2' || Stem[3] == L'\u00b3');
		if (Stem == L"CON" || Stem == L"PRN" || Stem == L"AUX" || Stem == L"NUL" || Stem == L"CLOCK$" ||
			Stem == L"CONIN$" || Stem == L"CONOUT$" || DeviceNumber)
		{ SetError(Error, "Capture name is reserved by Windows. Choose another name."); return false; }
		return true;
	}
}

bool_t Client::CProfilerCaptureIO::Validate_Name(std::string_view Name, string* Error)
{
	try
	{
		std::wstring Wide;
		if (!CaptureName(Name, Wide, Error)) return false;
		if (Error) Error->clear();
		return true;
	}
	catch (const std::exception& Exception) { SetError(Error, string("Invalid capture name: ") + Exception.what()); return false; }
}

bool_t Client::CProfilerCaptureIO::Make_NamedPath(std::string_view Name, uint64_t Frame,
	filesystem::path& OutPath, string* Error)
{
	try
	{
		std::wstring Label;
		if (!CaptureName(Name, Label, Error)) return false;
		const auto Now = chrono::system_clock::now();
		const time_t CalendarTime = chrono::system_clock::to_time_t(Now);
		tm LocalTime{}; localtime_s(&LocalTime, &CalendarTime);
		const auto Milliseconds = chrono::duration_cast<chrono::milliseconds>(Now.time_since_epoch()).count() % 1000;
		static std::atomic_uint64_t Sequence = 0;
		wostringstream FileName;
		FileName << Label << L"_" << put_time(&LocalTime, L"%Y%m%d_%H%M%S")
			<< L"_" << setw(3) << setfill(L'0') << Milliseconds << L"_frame" << Frame
			<< L"_" << GetCurrentProcessId() << L"_" << Sequence.fetch_add(1) << L".json";
		OutPath = Get_CaptureDirectory() / FileName.str();
		if (Error) Error->clear();
		return true;
	}
	catch (const std::exception& Exception) { SetError(Error, string("Cannot name capture: ") + Exception.what()); return false; }
}

filesystem::path Client::CProfilerCaptureIO::Make_DefaultPath(uint64_t Frame)
{
	filesystem::path Result;
	Make_NamedPath({}, Frame, Result);
	return Result;
}

bool_t Client::CProfilerCaptureIO::List_JsonFiles(const filesystem::path& Directory,
	std::vector<FProfilerCaptureFile>& OutFiles, string* Error)
{
	try
	{
		std::error_code Code;
		if (!filesystem::exists(Directory, Code) && !Code)
		{ OutFiles.clear(); if (Error) Error->clear(); return true; }
		FCaptureHandle Root; filesystem::path FinalDirectory;
		if (!OpenCaptureDirectory(Directory, Root, FinalDirectory, Error)) return false;
		std::vector<FProfilerCaptureFile> Staged;
		for (const auto& Entry : filesystem::directory_iterator(Directory))
		{
			if (!IsCaptureFileName(Entry.path().filename())) continue;
			const DWORD Attributes = GetFileAttributesW(Entry.path().c_str());
			if (Attributes != INVALID_FILE_ATTRIBUTES && (Attributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT))) continue;
			FCaptureHandle File; FProfilerCaptureFile Row;
			if (!ReadCaptureFile(Directory, FinalDirectory, Entry.path().filename(), false, File, Row, Error)) return false;
			Staged.push_back(std::move(Row));
		}
		std::sort(Staged.begin(), Staged.end(), [](const auto& A, const auto& B)
		{ return A.LastWriteTicks != B.LastWriteTicks ? A.LastWriteTicks > B.LastWriteTicks : A.FileName < B.FileName; });
		OutFiles = std::move(Staged);
		if (Error) Error->clear();
		return true;
	}
	catch (const std::exception& Exception) { SetError(Error, string("Cannot list captures: ") + Exception.what()); return false; }
}

bool_t Client::CProfilerCaptureIO::Delete_JsonFile(const filesystem::path& Directory,
	const FProfilerCaptureFile& Selected, string* Error)
{
	try
	{
		FCaptureHandle Root; filesystem::path FinalDirectory;
		if (!OpenCaptureDirectory(Directory, Root, FinalDirectory, Error)) return false;
		FCaptureHandle File; FProfilerCaptureFile Current;
		if (!ReadCaptureFile(Directory, FinalDirectory, Selected.FileName, true, File, Current, Error)) return false;
		if (Current.StableId != Selected.StableId || Current.SizeBytes != Selected.SizeBytes || Current.LastWriteTicks != Selected.LastWriteTicks)
		{ SetError(Error, "Selected capture changed since the last refresh. Refresh and select it again."); return false; }
		FILE_DISPOSITION_INFO Disposition{ TRUE };
		if (!SetFileInformationByHandle(File.Value, FileDispositionInfo, &Disposition, sizeof(Disposition)))
			return CaptureError(Error, "Cannot delete selected capture");
		if (Error) Error->clear();
		return true;
	}
	catch (const std::exception& Exception) { SetError(Error, string("Cannot delete capture: ") + Exception.what()); return false; }
}
```


### G10.map. 적용 전 전체 코드

#### `Client/Public/MapAssetRenderUtils.h`

후보 SHA256: `df4a7025a9638e78cbef1937b57797d6376e5e66a8e9cae2455b5105606d12df`

```cpp
#pragma once

#include "Client_Defines.h"
#include "MapAssetCatalog.h"
#include "MapLoadScope.h"

NS_BEGIN(Engine)

class CModel;
class CShader;

NS_END

NS_BEGIN(Client)

struct MAP_CAMERA_CULL_SNAPSHOT final
{
	uint64_t revision = {};
	float4x4_t view = {};
	float4x4_t projection = {};
	float4_t worldPlanes[6] = {};
};

// The light clip volume has its own revision and never consumes camera visibility.
struct MAP_SHADOW_CULL_SNAPSHOT final
{
	uint64_t revision = {};
	float4_t worldPlanes[6] = {};
};

// VISIBILITY_ONLY preserves visibility and hysteresis; plane diagnostics are not guaranteed.
// Diagnostic policies always request the complete plane distances and rejecting plane.
enum class MAP_FRUSTUM_CULL_DETAIL : uint8_t { FULL, VISIBILITY_ONLY };

struct MAP_FRUSTUM_CULL_DECISION final
{
	bool_t wouldBeVisible = true;
	bool_t shouldRender = true;
	bool_t largeGeometry = false;
	f32_t baseRadius = {};
	f32_t margin = {};
	f32_t effectiveRadius = {};
	f32_t planeDistances[6] = {};
	f32_t planeTolerances[6] = {};
	int32_t rejectingPlane = -1;
};

struct MAP_SURFACE_BINDING_ROW final
{
	std::string assetId;
	std::string materialName;
	Engine::MODEL_SURFACE_FAMILY family = Engine::MODEL_SURFACE_FAMILY::LEGACY;
	uint32_t activeProgram = {};
	uint64_t lastSeenTickMs = {};
	Engine::MODEL_SURFACE_PARAMETERS surface;
	Engine::MODEL_BAKED_LIGHTING_INSTANCE lighting{};
};

// Instanced map shaders consume baked transforms from VTXMESHINSTANCE.
enum class MAP_MATERIAL_BINDING_MODE : uint8_t
{
	OBJECT,
	INSTANCED
};

class CMapAssetRenderUtils final
{
public:
	static bool_t Build_CameraCullSnapshot(
		const float4x4_t& view,
		const float4x4_t& projection,
		uint64_t revision,
		MAP_CAMERA_CULL_SNAPSHOT& outSnapshot,
		std::string* outFailureReason = nullptr);
	static bool_t Capture_CameraCullSnapshot(
		MAP_CAMERA_CULL_SNAPSHOT& outSnapshot,
		std::string* outFailureReason = nullptr);
	// Render-thread view of the same validated camera cache. Consume immediately;
	// a later camera capture may replace it. Failure returns null, never stale data.
	static const MAP_CAMERA_CULL_SNAPSHOT* Capture_CameraCullSnapshotView(
		std::string* outFailureReason = nullptr);
	static bool_t Build_ShadowCullSnapshot(
		const float4x4_t& view,
		const float4x4_t& projection,
		uint64_t revision,
		MAP_SHADOW_CULL_SNAPSHOT& outSnapshot);
	static bool_t Capture_ShadowCullSnapshot(
		MAP_SHADOW_CULL_SNAPSHOT& outSnapshot);
	// Invalid inputs retain the caster; only a separated valid sphere is rejected.
	static bool_t Intersects_ShadowCullSnapshot(
		const MAP_SHADOW_CULL_SNAPSHOT& snapshot,
		const float3_t& worldCenter,
		f32_t worldRadius);
	static HRESULT Bind_CameraCullSnapshot(
		const shared_ptr<Engine::CShader>& shader,
		const MAP_CAMERA_CULL_SNAPSHOT& snapshot);
	static bool_t Evaluate_FrustumVisibility(
		const MAP_FRUSTUM_CULLING_POLICY& policy,
		const MAP_CAMERA_CULL_SNAPSHOT& snapshot,
		const std::string& assetId,
		const std::string& assetGroupId,
		uint64_t placementId,
		const float3_t& worldCenter,
		f32_t worldRadius,
		MAP_FRUSTUM_RUNTIME_STATE& state,
		MAP_FRUSTUM_CULL_DECISION& outDecision,
		std::string* outFailureReason = nullptr,
		MAP_FRUSTUM_CULL_DETAIL detail = MAP_FRUSTUM_CULL_DETAIL::FULL);
	static void Begin_FrustumDiagnostics(
		const std::string& areaId,
		const MAP_FRUSTUM_CULLING_POLICY& policy);

	static uint32_t Select_Pass(
		const MAP_ASSET_RENDER_PROFILE& profile,
		bool_t mirrored);

	// Existing alpha-tested shadow passes may also be invariant under time/camera.
	// The caller must reject mutable texture overrides and morph geometry.
	static bool_t Uses_StaticShadowInputs(
		const Engine::MODEL_SURFACE_PARAMETERS* surface,
		const MAP_ASSET_RENDER_PROFILE& profile,
		bool_t useSourceMaterials);

	// Only families whose source shadow cannot discard or displace vertices.
	static bool_t Uses_OpaqueShadowPass(
		const Engine::MODEL_SURFACE_PARAMETERS* surface,
		const MAP_ASSET_RENDER_PROFILE& profile,
		bool_t useSourceMaterials);

	static HRESULT Bind_ShadowMaterial(
		const shared_ptr<Engine::CModel>& model,
		const shared_ptr<Engine::CShader>& shader,
		uint32_t meshIndex,
		const MAP_ASSET_RENDER_PROFILE& profile,
		f32_t elapsedTime);

	static HRESULT Bind_Material(
		const shared_ptr<Engine::CModel>& model,
		const shared_ptr<Engine::CShader>& shader,
		uint32_t meshIndex,
		const MAP_ASSET_RENDER_PROFILE& profile,
		f32_t elapsedTime,
		const ComPtr<ID3D11ShaderResourceView>& diffuseOverride = nullptr,
		const std::string& diagnosticAssetId = {},
        const Engine::MODEL_BAKED_LIGHTING_INSTANCE* bakedLighting = nullptr,
        const float4_t* worldCullSphere = nullptr,
        MAP_MATERIAL_BINDING_MODE bindingMode = MAP_MATERIAL_BINDING_MODE::OBJECT);

	/* Scene and transient lights, their ambient and the scene fog for a
	source-character material drawn forward after scene lighting. */
	static HRESULT Bind_SourceCharacterForwardLights(
		const shared_ptr<Engine::CShader>& shader);

	static std::vector<MAP_SURFACE_BINDING_ROW> Get_RecentSurfaceBindings();
};

NS_END
```

#### `Client/Private/MapAssetRenderUtils.cpp`

후보 SHA256: `2be8537c7582003b109e669bb22ae724f9e8900c855fba8bbfeef27d926ccfab`

```cpp
#include "MapAssetRenderUtils.h"
#include "SourceMovieMaterialPrograms.h"
#include "Engine_RenderTypes.h"

#include "GameInstance.h"
#include "Model.h"
#include "Shader.h"
#include "Presentation_Manager.h"
#include "AnimationTargetService.h"
#include "Character.h"
#include <array>
#include <atomic>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <mutex>

namespace
{
    HRESULT BindSourceFoliageWind(const std::shared_ptr<Engine::CShader>& shader,
        const Engine::MODEL_SURFACE_PARAMETERS* surface, float time, bool skinned = false)
    {
        const uint32_t enabled = surface &&
            (surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_FOLIAGE_MASKED ||
             surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_GRASS_MASKED ||
             (surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_CHARACTER &&
              surface->sourceCharacter.program >= 1100u && surface->sourceCharacter.program <= 1166u)) &&
            surface->sourceFoliageWind ? 1u : 0u;
        // Animated scenery shares the BG material evaluator, but its skinned VS
        // has no foliage-wind input. Do not bind the static-only reset there.
        // An actual wind request still fails instead of silently losing motion.
        if (skinned) return enabled ? E_INVALIDARG : S_OK;
        if (FAILED(shader->Bind_RawValue("g_SourceFoliageWindEnabled", &enabled, sizeof(enabled)))) return E_FAIL;
        if (!enabled) return S_OK;
        if (!std::isfinite(time)) return E_INVALIDARG;
        if (FAILED(shader->Bind_RawValue("g_SourceFoliageWindProgram", &surface->sourceFoliageWindProgram,
            sizeof(surface->sourceFoliageWindProgram)))) return E_FAIL;
        float4_t player = surface->sourceFoliageWindPlayerPosition;
        if (surface->sourceFoliageWindProgram == 3u)
        {
            const auto character = Client::CAnimationTargetService::Resolve_SceneCharacter();
            float4x4_t root{};
            if (character && character->Try_Get_PresentationRootMatrix(&root))
            {
                if (!std::isfinite(root._41) || !std::isfinite(root._42) || !std::isfinite(root._43)) return E_INVALIDARG;
                player = float4_t(root._41 * 100.f, -root._43 * 100.f, root._42 * 100.f, 0.f);
            }
        }
        for (const auto& pair : { std::pair<const char*, const float4_t*>("g_SourceFoliageWindLocalCenter", &surface->sourceFoliageWindLocalCenter),
            { "g_SourceFoliageWindLocalBounds", &surface->sourceFoliageWindLocalBounds },
            { "g_SourceFoliageWindActorPosition", &surface->sourceFoliageWindActorPosition },
            { "g_SourceFoliageWindDirectionSpeed", &surface->sourceFoliageWindDirectionSpeed },
            { "g_SourceFoliageWindPlayerPosition", &player } })
            if (FAILED(shader->Bind_RawValue(pair.first, pair.second, sizeof(float4_t)))) return E_FAIL;
        if (FAILED(shader->Bind_RawValue("g_SourceFoliageWindScalars", surface->sourceFoliageWindScalars, sizeof(surface->sourceFoliageWindScalars))) ||
            FAILED(shader->Bind_RawValue("g_SourceFoliageWindTime", &time, sizeof(time)))) return E_FAIL;
        return S_OK;
    }

    HRESULT BindForwardSceneLights(const std::shared_ptr<Engine::CShader>& shader, bool bakedReceiver, const float4_t* worldCullSphere,
        bool withAmbient = false)
    {
        constexpr size_t capacity = 400u; // Scene 16 + existing transient budget 384.
        // Only [0,count) is consumed by the forward shaders. Each appended
        // record initializes every lane, so unused capacity needs no upload.
        std::array<float4_t, capacity> positions, directions, colors, cones, ambients;
        uint32_t count = 0u;
        bool mainDirectionalConsumed = false;
        const auto append = [&](const Engine::LIGHT_DESC& light, bool scene) -> HRESULT
        {
            const bool mainDirectional = scene && !mainDirectionalConsumed && light.eType == LIGHT::DIRECTIONAL;
            if (mainDirectional) mainDirectionalConsumed = true;
            if (bakedReceiver && (light.eReceiver == LIGHT_RECEIVER::SOURCE_CHARACTER ||
                light.eReceiver == LIGHT_RECEIVER::UNBAKED)) return S_OK;
            if (light.vDiffuse.x == 0.f && light.vDiffuse.y == 0.f && light.vDiffuse.z == 0.f) return S_OK;
            if (worldCullSphere && light.eType != LIGHT::DIRECTIONAL)
            {
                const float x = light.vPosition.x - worldCullSphere->x;
                const float y = light.vPosition.y - worldCullSphere->y;
                const float z = light.vPosition.z - worldCullSphere->z;
                const float radius = light.fRange + worldCullSphere->w;
                if (radius <= 0.f || x*x + y*y + z*z > radius*radius) return S_OK;
            }
            if (count >= capacity) return E_BOUNDS;
            float type = 0.f;
            switch (light.eType)
            {
            case LIGHT::DIRECTIONAL: type = 1.f; break;
            case LIGHT::POINT: type = 2.f; break;
            case LIGHT::SPOT: type = 3.f; break;
            default: return E_INVALIDARG;
            }
            positions[count] = float4_t(light.vPosition.x, light.vPosition.y, light.vPosition.z, light.fRange);
            directions[count] = float4_t(light.vDirection.x, light.vDirection.y, light.vDirection.z, type);
            colors[count] = float4_t(light.vDiffuse.x, light.vDiffuse.y, light.vDiffuse.z, light.fFalloffExponent);
            cones[count] = float4_t(light.fSpotInnerCos, light.fSpotOuterCos, 0.f, light.staticShadowChannel != 0u ? float(light.staticShadowChannel) : mainDirectional ? 1.f : 0.f);
            ambients[count] = light.vAmbient;
            ++count;
            return S_OK;
        };
        for (const auto& light : Engine::CGameInstance::Get().Get_SceneLights())
            if (FAILED(append(light, true))) return E_FAIL;
        for (const auto& light : Engine::CPresentation_Manager::Get().Get_TransientLights())
            if (FAILED(append(light, false))) return E_FAIL;
        if (FAILED(shader->Bind_RawValue("g_SourceMapForwardLightCount", &count, sizeof(count)))) return E_FAIL;
        if (count == 0u) return S_OK;
        const uint32_t bytes = count * sizeof(float4_t);
        if (FAILED(shader->Bind_RawValue("g_SourceMapForwardLightPositionRange", positions.data(), bytes)) ||
            FAILED(shader->Bind_RawValue("g_SourceMapForwardLightDirectionType", directions.data(), bytes)) ||
            FAILED(shader->Bind_RawValue("g_SourceMapForwardLightColorExponent", colors.data(), bytes)) ||
            FAILED(shader->Bind_RawValue("g_SourceMapForwardLightConeShadow", cones.data(), bytes)) ||
            (withAmbient && FAILED(shader->Bind_RawValue("g_SourceMapForwardLightAmbient", ambients.data(), bytes)))) return E_FAIL;
        return S_OK;
    }

	// Only the Rendering Workbench consumes this diagnostic list. A short lease
	// lets its next draw populate the view without collecting while tools are shut.
	std::atomic<uint64_t> g_SurfaceBindingRequestedUntilMs{ 0u };
	std::mutex g_SurfaceBindingMutex;
	std::vector<Client::MAP_SURFACE_BINDING_ROW> g_SurfaceBindings;
	uint32_t g_SurfaceBindingLevelId = (std::numeric_limits<uint32_t>::max)();

	void RefreshSurfaceBindings(uint32_t levelId, uint64_t now)
	{
		if (g_SurfaceBindingLevelId != levelId)
		{
			g_SurfaceBindings.clear();
			g_SurfaceBindingLevelId = levelId;
		}
		g_SurfaceBindings.erase(std::remove_if(g_SurfaceBindings.begin(), g_SurfaceBindings.end(),
			[now](const Client::MAP_SURFACE_BINDING_ROW& row)
			{
				return now - row.lastSeenTickMs > 1000u;
			}), g_SurfaceBindings.end());
	}

	void RecordSurfaceBinding(const std::string& assetId, const std::string& materialName,
		const Engine::MODEL_SURFACE_PARAMETERS& surface, uint32_t program,
		const Engine::MODEL_BAKED_LIGHTING_INSTANCE& lighting)
	{
		const auto family = surface.family;
		const uint64_t now = GetTickCount64();
		if (now >= g_SurfaceBindingRequestedUntilMs.load(std::memory_order_relaxed))
			return;
		const uint32_t levelId = Engine::CGameInstance::Get().Get_CurrentLevelID();
		std::lock_guard<std::mutex> lock(g_SurfaceBindingMutex);
		RefreshSurfaceBindings(levelId, now);
		const auto existing = std::find_if(g_SurfaceBindings.begin(), g_SurfaceBindings.end(),
			[&](const Client::MAP_SURFACE_BINDING_ROW& row)
			{
				return row.assetId == assetId && row.materialName == materialName;
			});
		if (existing != g_SurfaceBindings.end())
		{
			existing->family = family;
			existing->activeProgram = program;
			existing->lastSeenTickMs = now;
			existing->surface = surface;
			existing->lighting = lighting;
			return;
		}
		if (g_SurfaceBindings.size() >= 32u)
		{
			g_SurfaceBindings.erase(std::min_element(g_SurfaceBindings.begin(), g_SurfaceBindings.end(),
				[](const Client::MAP_SURFACE_BINDING_ROW& left, const Client::MAP_SURFACE_BINDING_ROW& right)
				{
					return left.lastSeenTickMs < right.lastSeenTickMs;
				}));
		}
		g_SurfaceBindings.push_back({ assetId, materialName, family, program, now, surface, lighting });
	}

	std::mutex g_DiagnosticMutex;
	std::ofstream g_DiagnosticStream;
	std::string g_DiagnosticAreaId;
	uint64_t g_CameraMatrixRevision = {};
	bool_t g_HasCameraMatrices = false;
	float4x4_t g_LastView = {};
	float4x4_t g_LastProjection = {};
	Client::MAP_CAMERA_CULL_SNAPSHOT g_LastCameraSnapshot{};
	bool_t g_HasShadowMatrices = false;
	float4x4_t g_LastShadowView{};
	float4x4_t g_LastShadowProjection{};
	Client::MAP_SHADOW_CULL_SNAPSHOT g_LastShadowSnapshot{};
	uint64_t g_ValidatedPlaneRevision = {};
	bool_t g_HasValidatedPlanes = false;
	float4_t g_ValidatedPlanes[6]{};
	double g_ValidatedMaximumPlaneOffset = 0.;

	bool_t IsFiniteMatrix(const float4x4_t& matrix)
	{
		const f32_t* values = &matrix._11;
		for (uint32_t index = 0; index < 16u; ++index)
		{
			if (!std::isfinite(values[index]))
				return false;
		}
		return true;
	}

	bool_t ReportCullFailure(std::string* reason, const char* message)
	{
		if (nullptr != reason)
			*reason = message;
		return false;
	}

	bool_t IsInvertibleMatrix(const float4x4_t& matrix)
	{
		double values[4][4]{};
		const f32_t* source = &matrix._11;
		for (uint32_t row = 0; row < 4u; ++row)
			for (uint32_t column = 0; column < 4u; ++column)
				values[row][column] = source[row * 4u + column];
		for (uint32_t column = 0; column < 4u; ++column)
		{
			uint32_t pivot = column;
			for (uint32_t row = column + 1u; row < 4u; ++row)
				if (std::abs(values[row][column]) > std::abs(values[pivot][column]))
					pivot = row;
			if (!std::isfinite(values[pivot][column]) || 0.0 == values[pivot][column])
				return false;
			for (uint32_t entry = column; entry < 4u; ++entry)
				std::swap(values[column][entry], values[pivot][entry]);
			for (uint32_t row = column + 1u; row < 4u; ++row)
			{
				const double factor = values[row][column] / values[column][column];
				for (uint32_t entry = column + 1u; entry < 4u; ++entry)
					values[row][entry] -= factor * values[column][entry];
			}
		}
		return true;
	}

	void CacheValidatedPlanes(
		const Client::MAP_CAMERA_CULL_SNAPSHOT& snapshot)
	{
		g_ValidatedPlaneRevision = snapshot.revision;
		std::memcpy(g_ValidatedPlanes, snapshot.worldPlanes,
			sizeof(g_ValidatedPlanes));
		g_ValidatedMaximumPlaneOffset = 0.;
		for (const auto& plane : snapshot.worldPlanes)
			g_ValidatedMaximumPlaneOffset = (std::max)(g_ValidatedMaximumPlaneOffset,
				std::abs(static_cast<double>(plane.w)));
		g_HasValidatedPlanes = true;
	}

	bool_t ValidateNormalizedPlanes(
		const Client::MAP_CAMERA_CULL_SNAPSHOT& snapshot,
		std::string* reason)
	{
		if (g_HasValidatedPlanes &&
			g_ValidatedPlaneRevision == snapshot.revision &&
			0 == std::memcmp(g_ValidatedPlanes, snapshot.worldPlanes,
				sizeof(g_ValidatedPlanes)))
		{
			return true;
		}

		for (const float4_t& plane : snapshot.worldPlanes)
		{
			const double normSquared =
				static_cast<double>(plane.x) * plane.x +
				static_cast<double>(plane.y) * plane.y +
				static_cast<double>(plane.z) * plane.z;
			if (!std::isfinite(plane.w) || !std::isfinite(normSquared) ||
				std::abs(normSquared - 1.0) >
					8.0 * std::numeric_limits<f32_t>::epsilon())
			{
				return ReportCullFailure(reason,
					"invalid normalized frustum plane");
			}
		}

		CacheValidatedPlanes(snapshot);
		return true;
	}

	std::filesystem::path GetDiagnosticPath()
	{
		wchar_t modulePath[32768]{};
		const DWORD length = GetModuleFileNameW(
			nullptr, modulePath, static_cast<DWORD>(std::size(modulePath)));
		if (0u == length || length >= std::size(modulePath))
			return {};
		return std::filesystem::path(modulePath).parent_path() /
			L"Diagnostics" / L"BernFrustumCulling.log";
	}

	void RecordRejectedTransition(
		const Client::MAP_CAMERA_CULL_SNAPSHOT& snapshot,
		const std::string& assetId,
		const std::string& assetGroupId,
		const uint64_t placementId,
		const float3_t& worldCenter,
		const Client::MAP_FRUSTUM_CULL_DECISION& decision,
		const bool_t bypass)
	{
		std::scoped_lock lock(g_DiagnosticMutex);
		if (!g_DiagnosticStream)
			return;
		g_DiagnosticStream << std::setprecision(9)
			<< "event=VISIBLE_TO_REJECTED"
			<< " area=" << std::quoted(g_DiagnosticAreaId)
			<< " cameraRevision=" << snapshot.revision
			<< " bypass=" << (bypass ? 1 : 0)
			<< " assetId=" << std::quoted(assetId)
			<< " groupId=" << std::quoted(assetGroupId)
			<< " placementId=" << placementId
			<< " center=(" << worldCenter.x << ',' << worldCenter.y << ','
			<< worldCenter.z << ')'
			<< " baseRadius=" << decision.baseRadius
			<< " margin=" << decision.margin
			<< " effectiveRadius=" << decision.effectiveRadius
			<< " largeGeometry=" << (decision.largeGeometry ? 1 : 0)
			<< " planeDistances=[";
		for (uint32_t index = 0; index < 6u; ++index)
		{
			if (0u != index)
				g_DiagnosticStream << ',';
			g_DiagnosticStream << decision.planeDistances[index];
		}
		g_DiagnosticStream << ']';
		const matrix_t viewProjection =
			XMLoadFloat4x4(&snapshot.view) *
			XMLoadFloat4x4(&snapshot.projection);
		const vector_t clip = XMVector3Transform(
			XMVectorSetW(XMLoadFloat3(&worldCenter), 1.f), viewProjection);
		const f32_t clipW = XMVectorGetW(clip);
		if (std::isfinite(clipW) && std::abs(clipW) > 0.000001f)
		{
			const f32_t ndcX = XMVectorGetX(clip) / clipW;
			const f32_t ndcY = XMVectorGetY(clip) / clipW;
			const f32_t ndcZ = XMVectorGetZ(clip) / clipW;
			const f32_t ndcRadiusX = decision.baseRadius *
				snapshot.projection._11 / std::abs(clipW);
			const f32_t ndcRadiusY = decision.baseRadius *
				snapshot.projection._22 / std::abs(clipW);
			const bool_t onScreen = clipW > 0.f &&
				std::abs(ndcX) <= 1.f + ndcRadiusX &&
				std::abs(ndcY) <= 1.f + ndcRadiusY &&
				ndcZ >= 0.f && ndcZ <= 1.f;
			g_DiagnosticStream
				<< " ndc=(" << ndcX << ',' << ndcY << ',' << ndcZ << ')'
				<< " clipW=" << clipW
				<< " ndcRadius=(" << ndcRadiusX << ',' << ndcRadiusY << ')'
				<< " onScreen=" << (onScreen ? 1 : 0);
		}
		else
		{
			g_DiagnosticStream << " ndc=none clipW=" << clipW
				<< " ndcRadius=none onScreen=0";
		}
		g_DiagnosticStream << '\n';
		g_DiagnosticStream.flush();
	}
}

bool_t CMapAssetRenderUtils::Build_CameraCullSnapshot(
	const float4x4_t& view,
	const float4x4_t& projection,
	const uint64_t revision,
	MAP_CAMERA_CULL_SNAPSHOT& outSnapshot,
	std::string* outFailureReason)
{
	if (nullptr != outFailureReason)
		outFailureReason->clear();
	if (0u == revision)
		return ReportCullFailure(outFailureReason, "zero camera revision");
	if (!IsFiniteMatrix(view) || !IsFiniteMatrix(projection))
		return ReportCullFailure(outFailureReason, "non-finite camera matrix");
	if (!IsInvertibleMatrix(view) || !IsInvertibleMatrix(projection))
		return ReportCullFailure(outFailureReason, "singular camera matrix");

	// Extract the shader's clip half-spaces directly. A far corner plus two
	// nearby near corners loses their small edge in a float cross product.
	double clip[4][4]{};
	const f32_t* viewValues = &view._11;
	const f32_t* projectionValues = &projection._11;
	for (uint32_t row = 0; row < 4u; ++row)
	{
		for (uint32_t column = 0; column < 4u; ++column)
		{
			for (uint32_t inner = 0; inner < 4u; ++inner)
			{
				clip[row][column] +=
					static_cast<double>(viewValues[row * 4u + inner]) *
					projectionValues[inner * 4u + column];
			}
		}
	}

	MAP_CAMERA_CULL_SNAPSHOT candidate{};
	candidate.revision = revision;
	candidate.view = view;
	candidate.projection = projection;
	for (uint32_t planeIndex = 0; planeIndex < 6u; ++planeIndex)
	{
		double plane[4]{};
		for (uint32_t row = 0; row < 4u; ++row)
		{
			// Outward normals: right, left, top, bottom, far, near.
			switch (planeIndex)
			{
			case 0u: plane[row] = clip[row][0] - clip[row][3]; break;
			case 1u: plane[row] = -clip[row][0] - clip[row][3]; break;
			case 2u: plane[row] = clip[row][1] - clip[row][3]; break;
			case 3u: plane[row] = -clip[row][1] - clip[row][3]; break;
			case 4u: plane[row] = clip[row][2] - clip[row][3]; break;
			case 5u: plane[row] = -clip[row][2]; break;
			}
		}
		const double length = std::hypot(std::hypot(plane[0], plane[1]), plane[2]);
		if (!std::isfinite(length) || 0.0 == length)
			return ReportCullFailure(outFailureReason, "degenerate clip plane");
		f32_t* stored = &candidate.worldPlanes[planeIndex].x;
		for (uint32_t component = 0; component < 4u; ++component)
		{
			const double normalized = plane[component] / length;
			if (!std::isfinite(normalized) ||
				std::abs(normalized) > (std::numeric_limits<f32_t>::max)())
			{
				return ReportCullFailure(outFailureReason, "non-finite normalized clip plane");
			}
			stored[component] = static_cast<f32_t>(normalized);
		}
	}
	CacheValidatedPlanes(candidate);
	outSnapshot = candidate;
	return true;
}

bool_t CMapAssetRenderUtils::Capture_CameraCullSnapshot(
	MAP_CAMERA_CULL_SNAPSHOT& outSnapshot,
	std::string* outFailureReason)
{
	const auto* snapshot = Capture_CameraCullSnapshotView(outFailureReason);
	if (!snapshot)
		return false;
	outSnapshot = *snapshot;
	return true;
}

const MAP_CAMERA_CULL_SNAPSHOT* CMapAssetRenderUtils::Capture_CameraCullSnapshotView(
	std::string* outFailureReason)
{
	if (nullptr != outFailureReason)
		outFailureReason->clear();
	const float4x4_t* view = CGameInstance::Get().Get_Transform(D3DTS::VIEW);
	const float4x4_t* projection = CGameInstance::Get().Get_Transform(D3DTS::PROJ);
	if (nullptr == view || nullptr == projection)
	{
		ReportCullFailure(outFailureReason, "camera matrix unavailable");
		return nullptr;
	}

	const float4x4_t& stagedView = *view;
	const float4x4_t& stagedProjection = *projection;
	if (g_HasCameraMatrices &&
		0 == std::memcmp(&g_LastView, &stagedView, sizeof(float4x4_t)) &&
		0 == std::memcmp(&g_LastProjection, &stagedProjection, sizeof(float4x4_t)))
	{
		return &g_LastCameraSnapshot;
	}
	const uint64_t nextRevision =
		(std::numeric_limits<uint64_t>::max)() == g_CameraMatrixRevision ?
		1u : g_CameraMatrixRevision + 1u;
	MAP_CAMERA_CULL_SNAPSHOT candidate{};
	if (!Build_CameraCullSnapshot(stagedView, stagedProjection, nextRevision,
		candidate, outFailureReason))
	{
		return nullptr;
	}
	g_LastView = candidate.view;
	g_LastProjection = candidate.projection;
	g_CameraMatrixRevision = candidate.revision;
	g_LastCameraSnapshot = candidate;
	g_HasCameraMatrices = true;
	return &g_LastCameraSnapshot;
}

bool_t CMapAssetRenderUtils::Build_ShadowCullSnapshot(
	const float4x4_t& view, const float4x4_t& projection,
	const uint64_t revision, MAP_SHADOW_CULL_SNAPSHOT& outSnapshot)
{
	MAP_CAMERA_CULL_SNAPSHOT planes{};
	if (!Build_CameraCullSnapshot(view, projection, revision, planes))
		return false;
	MAP_SHADOW_CULL_SNAPSHOT candidate{};
	candidate.revision = revision;
	std::memcpy(candidate.worldPlanes, planes.worldPlanes, sizeof(candidate.worldPlanes));
	outSnapshot = candidate;
	return true;
}

bool_t CMapAssetRenderUtils::Capture_ShadowCullSnapshot(
	MAP_SHADOW_CULL_SNAPSHOT& outSnapshot)
{
	const auto* view = CGameInstance::Get().Get_ShadowLightTransform(D3DTS::VIEW);
	const auto* projection = CGameInstance::Get().Get_ShadowLightTransform(D3DTS::PROJ);
	if (!view || !projection)
		return false;
	const float4x4_t stagedView = *view;
	const float4x4_t stagedProjection = *projection;
	if (g_HasShadowMatrices &&
		0 == std::memcmp(&g_LastShadowView, &stagedView, sizeof(stagedView)) &&
		0 == std::memcmp(&g_LastShadowProjection, &stagedProjection, sizeof(stagedProjection)))
	{
		outSnapshot = g_LastShadowSnapshot;
		return true;
	}
	const uint64_t revision = g_LastShadowSnapshot.revision ==
		(std::numeric_limits<uint64_t>::max)() ? 1u : g_LastShadowSnapshot.revision + 1u;
	MAP_SHADOW_CULL_SNAPSHOT candidate{};
	if (!Build_ShadowCullSnapshot(stagedView, stagedProjection, revision, candidate))
		return false;
	g_LastShadowView = stagedView;
	g_LastShadowProjection = stagedProjection;
	g_LastShadowSnapshot = candidate;
	g_HasShadowMatrices = true;
	outSnapshot = candidate;
	return true;
}

bool_t CMapAssetRenderUtils::Intersects_ShadowCullSnapshot(
	const MAP_SHADOW_CULL_SNAPSHOT& snapshot,
	const float3_t& worldCenter, const f32_t worldRadius)
{
	if (snapshot.revision == 0u || !std::isfinite(worldCenter.x) ||
		!std::isfinite(worldCenter.y) || !std::isfinite(worldCenter.z) ||
		!std::isfinite(worldRadius) || worldRadius <= 0.f)
		return true;
	// Validate every plane before rejecting; a corrupt later plane cannot hide a caster.
	for (const auto& plane : snapshot.worldPlanes)
	{
		const double normSquared = static_cast<double>(plane.x) * plane.x +
			static_cast<double>(plane.y) * plane.y + static_cast<double>(plane.z) * plane.z;
		if (!std::isfinite(plane.w) || !std::isfinite(normSquared) ||
			std::abs(normSquared - 1.0) > 8.0 * std::numeric_limits<f32_t>::epsilon())
			return true;
	}
	// Placement bounds already include transform inflation; retain another 5 cm
	// and a magnitude-scaled float tolerance at all six shadow clip boundaries.
	const double radius = static_cast<double>(worldRadius) + 0.05;
	for (const auto& plane : snapshot.worldPlanes)
	{
		const double x = static_cast<double>(plane.x) * worldCenter.x;
		const double y = static_cast<double>(plane.y) * worldCenter.y;
		const double z = static_cast<double>(plane.z) * worldCenter.z;
		const double magnitude = std::abs(x) + std::abs(y) + std::abs(z) +
			std::abs(static_cast<double>(plane.w)) + radius;
		const double tolerance = 8.0 * std::numeric_limits<f32_t>::epsilon() *
			(std::max)(1.0, magnitude);
		if (x + y + z + plane.w > radius + tolerance)
			return false;
	}
	return true;
}

HRESULT CMapAssetRenderUtils::Bind_CameraCullSnapshot(
	const shared_ptr<Engine::CShader>& shader,
	const MAP_CAMERA_CULL_SNAPSHOT& snapshot)
{
	if (nullptr == shader || 0u == snapshot.revision)
		return E_INVALIDARG;
	const HRESULT viewResult = shader->Bind_Matrix("g_ViewMatrix", &snapshot.view);
	return FAILED(viewResult) ? viewResult :
		shader->Bind_Matrix("g_ProjMatrix", &snapshot.projection);
}

bool_t CMapAssetRenderUtils::Evaluate_FrustumVisibility(
	const MAP_FRUSTUM_CULLING_POLICY& policy,
	const MAP_CAMERA_CULL_SNAPSHOT& snapshot,
	const std::string& assetId,
	const std::string& assetGroupId,
	const uint64_t placementId,
	const float3_t& worldCenter,
	const f32_t worldRadius,
	MAP_FRUSTUM_RUNTIME_STATE& state,
	MAP_FRUSTUM_CULL_DECISION& outDecision,
	std::string* outFailureReason,
	const MAP_FRUSTUM_CULL_DETAIL detail)
{
	if (nullptr != outFailureReason)
		outFailureReason->clear();
	if (0u == snapshot.revision || !std::isfinite(worldCenter.x) ||
		!std::isfinite(worldCenter.y) || !std::isfinite(worldCenter.z) ||
		!std::isfinite(worldRadius) || worldRadius <= 0.f)
	{
		return ReportCullFailure(outFailureReason, "invalid camera revision or world sphere");
	}
	const f32_t policyValues[] = { policy.baseMargin,
		policy.largeObjectRadiusThreshold, policy.largeObjectAbsoluteMargin,
		policy.largeObjectRelativeMargin };
	for (const f32_t value : policyValues)
	{
		if (!std::isfinite(value) || value < 0.f)
			return ReportCullFailure(outFailureReason, "invalid frustum margin policy");
	}
	if (!ValidateNormalizedPlanes(snapshot, outFailureReason))
		return false;

	MAP_FRUSTUM_CULL_DECISION candidate{};
	candidate.baseRadius = worldRadius;
	candidate.largeGeometry = "landscape" == assetGroupId ||
		(policy.largeObjectRadiusThreshold > 0.f &&
		 worldRadius >= policy.largeObjectRadiusThreshold);
	double margin = policy.baseMargin;
	if (candidate.largeGeometry)
	{
		margin = (std::max)({ margin,
			static_cast<double>(policy.largeObjectAbsoluteMargin),
			static_cast<double>(worldRadius) * policy.largeObjectRelativeMargin });
	}
	const double effectiveRadius = static_cast<double>(worldRadius) + margin;
	if (!std::isfinite(effectiveRadius) ||
		effectiveRadius > (std::numeric_limits<f32_t>::max)())
	{
		return ReportCullFailure(outFailureReason, "effective sphere radius overflow");
	}
	candidate.margin = static_cast<f32_t>(margin);
	candidate.effectiveRadius = static_cast<f32_t>(effectiveRadius);
	bool visibilityOnly = false;
	if (detail == MAP_FRUSTUM_CULL_DETAIL::VISIBILITY_ONLY && !policy.diagnostics)
	{
		// Normalized xyz components have absolute value below 2. The quarter-range
		// bound encloses every float product and partial dot sum with rounding.
		// Huge inputs retain the full path so a later plane's overflow still fails.
		const double dotMagnitudeBound = 2. * (std::abs(static_cast<double>(worldCenter.x)) +
			std::abs(static_cast<double>(worldCenter.y)) + std::abs(static_cast<double>(worldCenter.z))) +
			g_ValidatedMaximumPlaneOffset;
		visibilityOnly = dotMagnitudeBound < double((std::numeric_limits<f32_t>::max)()) * .25;
	}
	if (visibilityOnly)
	{
		const vector_t center = XMLoadFloat3(&worldCenter);
		for (const float4_t& plane : snapshot.worldPlanes)
		{
			const double x = static_cast<double>(plane.x) * worldCenter.x;
			const double y = static_cast<double>(plane.y) * worldCenter.y;
			const double z = static_cast<double>(plane.z) * worldCenter.z;
			const double distance = XMVectorGetX(XMPlaneDotCoord(XMLoadFloat4(&plane), center));
			const double magnitude = std::abs(x) + std::abs(y) + std::abs(z) +
				std::abs(static_cast<double>(plane.w)) + effectiveRadius;
			const double tolerance = 8.0 * std::numeric_limits<f32_t>::epsilon() *
				(std::max)(1.0, magnitude);
			// Keep the original subtraction order at near-tangent boundaries.
			if (distance - effectiveRadius - tolerance > 0.0)
			{
				candidate.wouldBeVisible = false;
				break;
			}
		}
	}
	else
	{
		double largestSeparation = 0.0;
		const vector_t center = XMLoadFloat3(&worldCenter);
		for (uint32_t index = 0; index < 6u; ++index)
		{
			const float4_t& plane = snapshot.worldPlanes[index];
			const double x = static_cast<double>(plane.x) * worldCenter.x;
			const double y = static_cast<double>(plane.y) * worldCenter.y;
			const double z = static_cast<double>(plane.z) * worldCenter.z;
			const f32_t planeDistance = XMVectorGetX(XMPlaneDotCoord(
				XMLoadFloat4(&plane), center));
			const double distance = planeDistance;
			const double magnitude = std::abs(x) + std::abs(y) + std::abs(z) +
				std::abs(static_cast<double>(plane.w)) + effectiveRadius;
			const double tolerance = 8.0 * std::numeric_limits<f32_t>::epsilon() *
				(std::max)(1.0, magnitude);
			if (!std::isfinite(distance) ||
				std::abs(distance) > (std::numeric_limits<f32_t>::max)())
			{
				return ReportCullFailure(outFailureReason, "frustum distance overflow");
			}
			candidate.planeDistances[index] = static_cast<f32_t>(distance);
			candidate.planeTolerances[index] = static_cast<f32_t>(tolerance);
			const double separation = distance - effectiveRadius - tolerance;
			if (separation > 0.0)
			{
				candidate.wouldBeVisible = false;
				if (separation > largestSeparation)
				{
					largestSeparation = separation;
					candidate.rejectingPlane = static_cast<int32_t>(index);
				}
			}
		}
	}

	MAP_FRUSTUM_RUNTIME_STATE nextState = state;
	nextState.initialized = true;
	nextState.lastFrustumVisible = candidate.wouldBeVisible;
	if (candidate.wouldBeVisible)
		nextState.rejectGraceFrames = policy.rejectHysteresisFrames;
	candidate.shouldRender = policy.bypass || candidate.wouldBeVisible;
	if (!candidate.shouldRender && nextState.rejectGraceFrames > 0u)
	{
		candidate.shouldRender = true;
		--nextState.rejectGraceFrames;
	}
	if (state.initialized && state.lastFrustumVisible &&
		!candidate.wouldBeVisible && policy.diagnostics)
	{
		RecordRejectedTransition(snapshot, assetId, assetGroupId,
			placementId, worldCenter, candidate, policy.bypass);
	}
	state = nextState;
	outDecision = candidate;
	return true;
}

void CMapAssetRenderUtils::Begin_FrustumDiagnostics(
	const std::string& areaId,
	const MAP_FRUSTUM_CULLING_POLICY& policy)
{
	if (!policy.diagnostics)
		return;

	const std::filesystem::path path = GetDiagnosticPath();
	std::error_code error;
	if (path.empty() ||
		(!std::filesystem::create_directories(path.parent_path(), error) && error))
	{
		OutputDebugStringA("[BernFrustum] Diagnostic directory unavailable.\n");
		return;
	}

	std::scoped_lock lock(g_DiagnosticMutex);
	g_DiagnosticStream.close();
	g_DiagnosticStream.clear();
	g_DiagnosticStream.open(path, std::ios::binary | std::ios::trunc);
	if (!g_DiagnosticStream)
	{
		OutputDebugStringA("[BernFrustum] Diagnostic log unavailable.\n");
		return;
	}
	g_DiagnosticAreaId = areaId;
	g_DiagnosticStream
		<< "LOSTARK_BERN_FRUSTUM_DIAGNOSTICS 1\n"
		<< "mode=" << (policy.bypass ? "BYPASS_AND_LOG" : "CULL_AND_LOG")
		<< " baseMargin=" << policy.baseMargin
		<< " largeRadiusThreshold=" << policy.largeObjectRadiusThreshold
		<< " largeAbsoluteMargin=" << policy.largeObjectAbsoluteMargin
		<< " largeRelativeMargin=" << policy.largeObjectRelativeMargin
		<< " rejectHysteresisFrames=" << policy.rejectHysteresisFrames
		<< "\n";
	g_DiagnosticStream.flush();
	OutputDebugStringA(("[BernFrustum] Diagnostics ready: " +
		path.string() + "\n").c_str());
}

uint32_t CMapAssetRenderUtils::Select_Pass(const MAP_ASSET_RENDER_PROFILE& profile,
	bool_t mirrored)
{
	//map asset의 profile의 cullmode를 사용해서 culloffset 변수와 modeoffset 변수를 설정해준다
	MAP_ASSET_CULL_MODE cullMode = profile.cullMode;

	if (mirrored && MAP_ASSET_CULL_MODE::TWO_SIDED != cullMode)
	{
		cullMode = MAP_ASSET_CULL_MODE::CULL_BACK == cullMode ?
			MAP_ASSET_CULL_MODE::CULL_FRONT :
			MAP_ASSET_CULL_MODE::CULL_BACK;
	}
	//culloffset 설정
	const uint32_t  cullOfset =
		MAP_ASSET_CULL_MODE::CULL_BACK == cullMode ? 0u :
		MAP_ASSET_CULL_MODE::CULL_FRONT == cullMode ? 1u : 2u;
	//deffered translucent background
	/* Water sits after the shadow passes so adding it leaves every existing
	   pass index where it was; the three cull variants keep the +0/+1/+2 rule
	   even though the source water material is never two sided. */
	const uint32_t modeOffset =
		MAP_ASSET_RENDER_MODE::DEFERRED == profile.renderMode ? 0u :
		MAP_ASSET_RENDER_MODE::TRANSLUCENT == profile.renderMode ? 3u :
		MAP_ASSET_RENDER_MODE::BACKGROUND == profile.renderMode ? 6u :
		MAP_ASSET_RENDER_MODE::WATER == profile.renderMode ? 15u : 9u;

	return modeOffset + cullOfset;
}

HRESULT Client::CMapAssetRenderUtils::Bind_SourceCharacterForwardLights(
	const shared_ptr<Engine::CShader>& shader)
{
	if (nullptr == shader)
		return E_INVALIDARG;
	if (FAILED(BindForwardSceneLights(shader, false, nullptr, true)))
		return E_FAIL;
	return CGameInstance::Get().Bind_HeightFog(shader.get());
}

std::vector<Client::MAP_SURFACE_BINDING_ROW> Client::CMapAssetRenderUtils::Get_RecentSurfaceBindings()
{
	const uint64_t now = GetTickCount64();
	g_SurfaceBindingRequestedUntilMs.store(now + 1000u, std::memory_order_relaxed);
	const uint32_t levelId = CGameInstance::Get().Get_CurrentLevelID();
	std::lock_guard<std::mutex> lock(g_SurfaceBindingMutex);
	RefreshSurfaceBindings(levelId, now);
	return g_SurfaceBindings;
}

bool_t Client::CMapAssetRenderUtils::Uses_OpaqueShadowPass(
	const Engine::MODEL_SURFACE_PARAMETERS* surface,
	const MAP_ASSET_RENDER_PROFILE& profile,
	const bool_t useSourceMaterials)
{
	if (!surface || !useSourceMaterials ||
		profile.renderMode != MAP_ASSET_RENDER_MODE::DEFERRED ||
		!std::isfinite(profile.opacity) || profile.opacity < 1.f)
		return false;
	switch (surface->family)
	{
	case Engine::MODEL_SURFACE_FAMILY::PBR_SEAMLESS_OPAQUE:
	case Engine::MODEL_SURFACE_FAMILY::PBR_OPAQUE:
		return !surface->pbrAlphaMasked;
	case Engine::MODEL_SURFACE_FAMILY::SOURCE_SPECULAR_OPAQUE:
    case Engine::MODEL_SURFACE_FAMILY::SOURCE_LANDSCAPE_OPAQUE:
        // Landscape holes are source topology; painted alpha is height blending.
		return true;
	case Engine::MODEL_SURFACE_FAMILY::SOURCE_OVERLAY_OPAQUE:
		return (surface->sourceOverlayFlags & 64u) == 0u;
	case Engine::MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED:
		// BG parallax changes UV only. Its sole shadow discard is mask bit 64.
		return (surface->sourceBgFlags & 64u) == 0u;
	default:
		// Foliage, native character/map and other families retain their source VS/PS.
		return false;
	}
}

bool_t Client::CMapAssetRenderUtils::Uses_StaticShadowInputs(
	const Engine::MODEL_SURFACE_PARAMETERS* surface,
	const MAP_ASSET_RENDER_PROFILE& profile,
	const bool_t useSourceMaterials)
{
	if (Uses_OpaqueShadowPass(surface, profile, useSourceMaterials))
		return true;
	if (!surface || !useSourceMaterials ||
		profile.renderMode != MAP_ASSET_RENDER_MODE::DEFERRED ||
		!std::isfinite(profile.opacity) || profile.opacity < 1.f)
		return false;

	// Preserve the current alpha test and rasterizer pass. Only its inputs decide
	// whether the already rendered depth can survive another frame.
	switch (surface->family)
	{
	case Engine::MODEL_SURFACE_FAMILY::LEGACY:
	case Engine::MODEL_SURFACE_FAMILY::SOURCE_FOLIAGE_MASKED:
	case Engine::MODEL_SURFACE_FAMILY::SOURCE_GRASS_MASKED:
		return !surface->sourceFoliageWind && profile.uvSpeed.x == 0.f && profile.uvSpeed.y == 0.f;
	case Engine::MODEL_SURFACE_FAMILY::SPECULAR_TEXTURE_REFLECTION:
	case Engine::MODEL_SURFACE_FAMILY::DIFFUSE_SPECULAR_REFLECTION:
	case Engine::MODEL_SURFACE_FAMILY::PBR_SEAMLESS_OPAQUE:
	case Engine::MODEL_SURFACE_FAMILY::PBR_OPAQUE:
	case Engine::MODEL_SURFACE_FAMILY::SOURCE_OVERLAY_OPAQUE:
		// Source alpha uses raw UV, fixed tiling or the fixed overlay transform.
		return true;
	case Engine::MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED:
		// Parallax moves the masked sample with camera position. Panning moves it
		// with elapsed time. Neither input is part of the static light-depth key.
		return (surface->sourceBgFlags & 2u) == 0u &&
			surface->sourceBgPanning.x == 0.f && surface->sourceBgPanning.y == 0.f;
	default:
		return false;
	}
}

HRESULT Client::CMapAssetRenderUtils::Bind_ShadowMaterial(
	const shared_ptr<Engine::CModel>& model,
	const shared_ptr<Engine::CShader>& shader,
	uint32_t meshIndex,
	const MAP_ASSET_RENDER_PROFILE& profile,
	f32_t elapsedTime)
{
	if (nullptr == model || nullptr == shader ||
		meshIndex >= model->Get_NumMeshes())
		return E_INVALIDARG;

	const auto* surface = model->Get_MaterialSurface(meshIndex);
	if (surface)
	{
		switch (surface->family)
		{
		case Engine::MODEL_SURFACE_FAMILY::LEGACY:
		case Engine::MODEL_SURFACE_FAMILY::SPECULAR_TEXTURE_REFLECTION:
		case Engine::MODEL_SURFACE_FAMILY::DIFFUSE_SPECULAR_REFLECTION:
		case Engine::MODEL_SURFACE_FAMILY::PBR_SEAMLESS_OPAQUE:
		case Engine::MODEL_SURFACE_FAMILY::PBR_OPAQUE:
		case Engine::MODEL_SURFACE_FAMILY::SOURCE_SPECULAR_OPAQUE:
			break;
		default:
			// Native, layered and foliage families own additional shadow inputs.
			return Bind_Material(model, shader, meshIndex, profile, elapsedTime);
		}
	}

	const uint32_t noProgram = 0u;
	const uint32_t pbrMasked = surface && surface->pbrAlphaMasked ? 1u : 0u;
	if (FAILED(shader->Bind_RawValue("g_SurfacePBRMasked", &pbrMasked, sizeof(pbrMasked))) ||
		(pbrMasked && FAILED(shader->Bind_RawValue("g_SurfaceUVTiling", &surface->uvTiling, sizeof(surface->uvTiling)))))
		return E_FAIL;
	if (FAILED(shader->Bind_RawValue("g_DiffuseMirrorU", &noProgram, sizeof(noProgram))))
		return E_FAIL;
	// A preceding character draw may share the Effect. Map instances omit these
	// optional variables, while the binary shadow pass reads the character program.
	shader->Bind_RawValue("g_SourceCharacterProgram", &noProgram, sizeof(noProgram));
	shader->Bind_RawValue("g_SourceCharacterRow", &noProgram, sizeof(noProgram));

	const float2_t uvOffset(profile.uvSpeed.x * elapsedTime, profile.uvSpeed.y * elapsedTime);
	if (FAILED(model->Bind_Material(shader, "g_DiffuseTexture", meshIndex, aiTextureType_DIFFUSE)) ||
		FAILED(shader->Bind_RawValue("g_UVScale", &profile.uvScale, sizeof(profile.uvScale))) ||
		FAILED(shader->Bind_RawValue("g_UVOffset", &uvOffset, sizeof(uvOffset))) ||
		FAILED(shader->Bind_RawValue("g_ColorTint", &profile.colorTint, sizeof(profile.colorTint))) ||
		FAILED(shader->Bind_RawValue("g_Opacity", &profile.opacity, sizeof(profile.opacity))))
		return E_FAIL;

	const auto settings = CGameInstance::Get().Get_MaterialRenderSettings();
	const uint32_t program = surface &&
		surface->family != Engine::MODEL_SURFACE_FAMILY::LEGACY &&
		profile.renderMode == MAP_ASSET_RENDER_MODE::DEFERRED && settings.bUseSourceMaterials ?
		static_cast<uint32_t>(surface->family) : 0u;
	// Source diffuse can differ from the legacy texture. Preserve its alpha even
	// though the remaining source lighting textures are unused by these shadows.
	if (program != 0u &&
		FAILED(model->Bind_SurfaceTexture(shader, "g_DiffuseTexture", meshIndex, aiTextureType_DIFFUSE)))
		return E_FAIL;
	if (FAILED(BindSourceFoliageWind(shader, surface, elapsedTime, model->Is_Skinned()))) return E_FAIL;
	return shader->Bind_RawValue("g_SurfaceProgram", &program, sizeof(program));
}

HRESULT Client::CMapAssetRenderUtils::Bind_Material(
	const shared_ptr<Engine::CModel>& model,
	const shared_ptr<Engine::CShader>& shader,
	uint32_t meshIndex,
	const MAP_ASSET_RENDER_PROFILE& profile,
	f32_t elapsedTime, const ComPtr<ID3D11ShaderResourceView>& diffuseOverride,
	const std::string& diagnosticAssetId,
    const Engine::MODEL_BAKED_LIGHTING_INSTANCE* bakedLighting, const float4_t* worldCullSphere,
    MAP_MATERIAL_BINDING_MODE bindingMode)
{
	if (nullptr == model ||
		nullptr == shader ||
		meshIndex >= model->Get_NumMeshes())
	{
		return E_INVALIDARG;
	}

	// Shared shader state is reset even for legacy and diffuse-override draws.
	const uint32_t noSurfaceEmissive = 0u;
	if (FAILED(shader->Bind_RawValue("g_HasSurfaceEmissive",
		&noSurfaceEmissive, sizeof(noSurfaceEmissive))) ||
		FAILED(shader->Bind_RawValue("g_DiffuseMirrorU",
			&noSurfaceEmissive, sizeof(noSurfaceEmissive))))
		return E_FAIL;

	// Static world objects share this Effect with character equipment. An explicit
	// diffuse SRV skips CMaterial's reset, so a preceding source character draw
	// must not select its program (or leave its dye/hit tint) for this mesh.
	// The instanced map shader omits these optional character-only variables.
	if (bindingMode == MAP_MATERIAL_BINDING_MODE::OBJECT)
	{
		const float4_t identityEmissive(1.f, 1.f, 1.f, 1.f);
		for (const char_t* name : { "g_SourceCharacterProgram", "g_SourceCharacterRow",
			"g_HasDyeMask", "g_HasFullSurfaceEmissiveOverride" })
			shader->Bind_RawValue(name, &noSurfaceEmissive, sizeof(noSurfaceEmissive));
		shader->Bind_RawValue("g_EmissiveColor", &identityEmissive, sizeof(identityEmissive));
	}

	const auto* nativeSurface = model->Get_MaterialSurface(meshIndex);
    const uint32_t sourceBgUnlit = nativeSurface && nativeSurface->sourceBgUnlit &&
        nativeSurface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED ? 1u : 0u;
    if (FAILED(shader->Bind_RawValue("g_SourceBgUnlit", &sourceBgUnlit, sizeof(sourceBgUnlit)))) return E_FAIL;
	const auto settings = CGameInstance::Get().Get_MaterialRenderSettings();
	const bool sourceBg = nativeSurface &&
		nativeSurface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED &&
		profile.renderMode == MAP_ASSET_RENDER_MODE::DEFERRED &&
		!diffuseOverride && settings.bUseSourceMaterials;
	// Source BG uses its raw UV and native textures/constants. Keep the diffuse
	// binder's mirror/reset contract, but do not prepare unused legacy inputs.
	if (sourceBg)
	{
		// Non-instanced opaque props still consume opacity for presentation dither.
		if (FAILED(model->Bind_Material(shader, "g_DiffuseTexture", meshIndex, aiTextureType_DIFFUSE)) ||
			FAILED(shader->Bind_RawValue("g_Opacity", &profile.opacity, sizeof(profile.opacity))))
			return E_FAIL;
	}
	else
	{
	const uint32_t hasNormalTexture =
		model->Has_MaterialTexture(
			meshIndex, aiTextureType_NORMALS) ? 1u : 0u;

	const uint32_t hasEmissiveTexture =
		model->Has_MaterialTexture(
			meshIndex, aiTextureType_EMISSIVE) ? 1u : 0u;

	const uint32_t hasSpecularTexture =
		model->Has_MaterialTexture(
			meshIndex, aiTextureType_SPECULAR) ? 1u : 0u;

	const uint32_t hasOpacityTexture =
		model->Has_MaterialTexture(
			meshIndex, aiTextureType_OPACITY) ? 1u : 0u;

    const bool constantSource = nativeSurface &&
        nativeSurface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_CHARACTER &&
        (nativeSurface->sourceCharacter.program == 64u || nativeSurface->sourceCharacter.program == 65u);
	const float2_t uvOffset(
		profile.uvSpeed.x * elapsedTime,
		profile.uvSpeed.y * elapsedTime);

	if ((!constantSource && FAILED(diffuseOverride ? shader->Bind_Texture("g_DiffuseTexture", diffuseOverride) : model->Bind_Material(
		shader,
		"g_DiffuseTexture",
		meshIndex,
		aiTextureType_DIFFUSE))) ||

		FAILED(shader->Bind_RawValue(
			"g_UVScale",
			&profile.uvScale,
			sizeof(profile.uvScale))) ||

		FAILED(shader->Bind_RawValue(
			"g_UVOffset",
			&uvOffset,
			sizeof(uvOffset))) ||

		FAILED(shader->Bind_RawValue(
			"g_Opacity",
			&profile.opacity,
			sizeof(profile.opacity))) ||

		FAILED(shader->Bind_RawValue(
			"g_OpacityPower",
			&profile.opacityPower,
			sizeof(profile.opacityPower))) ||

		FAILED(shader->Bind_RawValue(
			"g_ColorTint",
			&profile.colorTint,
			sizeof(profile.colorTint))) ||

		FAILED(shader->Bind_RawValue(
			"g_HasNormalTexture",
			&hasNormalTexture,
			sizeof(hasNormalTexture))) ||

		(0 != hasNormalTexture &&
			FAILED(model->Bind_Material(
				shader,
				"g_NormalTexture",
				meshIndex,
				aiTextureType_NORMALS))) ||

		FAILED(shader->Bind_RawValue(
			"g_HasEmissiveTexture",
			&hasEmissiveTexture,
			sizeof(hasEmissiveTexture))) ||

		FAILED(shader->Bind_RawValue(
			"g_EmissiveIntensity",
			&profile.emissiveIntensity,
			sizeof(profile.emissiveIntensity))) ||

		(0 != hasEmissiveTexture &&
			FAILED(model->Bind_Material(
				shader,
				"g_EmissiveTexture",
				meshIndex,
				aiTextureType_EMISSIVE))) ||

		FAILED(shader->Bind_RawValue(
			"g_HasSpecularTexture",
			&hasSpecularTexture,
			sizeof(hasSpecularTexture))) ||

		FAILED(shader->Bind_RawValue(
			"g_SpecularIntensity",
			&profile.specularIntensity,
			sizeof(profile.specularIntensity))) ||

		FAILED(shader->Bind_RawValue(
			"g_SpecularPower",
			&profile.specularPower,
			sizeof(profile.specularPower))) ||

		FAILED(shader->Bind_RawValue(
			"g_TriplanarHeightScale",
			&profile.triplanarHeightScale,
			sizeof(profile.triplanarHeightScale))) ||

		(0 != hasSpecularTexture &&
			FAILED(model->Bind_Material(
				shader,
				"g_SpecularTexture",
				meshIndex,
				aiTextureType_SPECULAR))) ||

		FAILED(shader->Bind_RawValue(
			"g_HasOpacityTexture",
			&hasOpacityTexture,
			sizeof(hasOpacityTexture))) ||

		(0 != hasOpacityTexture &&
			FAILED(model->Bind_Material(
				shader,
				"g_OpacityTexture",
				meshIndex,
				aiTextureType_OPACITY))))
	{
		return E_FAIL;
	}

	}

    // A static World Object may own the same native material contract as an actor.
    // Submit its real CMaterial into the shared direct-light pass after legacy resets.
    const Engine::MODEL_BAKED_LIGHTING_INSTANCE emptyShadowLighting{};
    const auto& shadowLighting = bakedLighting ? *bakedLighting : emptyShadowLighting;
    const uint32_t hasStaticShadow = nativeSurface && nativeSurface->hasStaticShadow ? 1u : 0u;
    if (FAILED(shader->Bind_RawValue("g_HasStaticShadow", &hasStaticShadow, sizeof(hasStaticShadow))) ||
        (bindingMode == MAP_MATERIAL_BINDING_MODE::OBJECT &&
         FAILED(shader->Bind_RawValue("g_StaticShadowScaleBias", &shadowLighting.shadowScaleBias, sizeof(shadowLighting.shadowScaleBias)))) ||
        (hasStaticShadow && FAILED(model->Bind_SurfaceLighting(shader, meshIndex)))) return E_FAIL;

    if (nativeSurface && nativeSurface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_CHARACTER)
    {
        const uint32_t noMapSurface = 0u;
        if (FAILED(shader->Bind_RawValue("g_SurfaceProgram", &noMapSurface, sizeof(noMapSurface))) ||
            FAILED(shader->Bind_RawValue("g_HasSurfaceDefinition", &noMapSurface, sizeof(noMapSurface)))) return E_FAIL;
        const uint32_t sourceProgram = nativeSurface->sourceCharacter.program;
        const bool movieStatic = SourceMovieMaterial::Is_Static(sourceProgram);
        const bool movieForward = SourceMovieMaterial::Is_Forward(sourceProgram) ||
            sourceProgram == 224u || sourceProgram == 225u || sourceProgram == 226u || sourceProgram == 237u;
        const bool forwardBakedProgram = (sourceProgram >= 40u && sourceProgram <= 63u &&
            sourceProgram != 47u && sourceProgram != 53u && sourceProgram != 55u) || sourceProgram == 209u || movieForward;
        if ((sourceProgram >= 80u && sourceProgram <= 83u) || sourceProgram == 210u ||
            (sourceProgram >= 214u && sourceProgram <= 234u) || sourceProgram == 237u || movieStatic)
        {
            const Engine::MODEL_BAKED_LIGHTING_INSTANCE emptyLighting{};
            const auto& instanceLighting = bakedLighting ? *bakedLighting : emptyLighting;
            if (FAILED(shader->Bind_RawValue("g_LightmapScaleBias", &instanceLighting.scaleBias, sizeof(instanceLighting.scaleBias))) ||
                FAILED(shader->Bind_RawValue("g_LightmapAverageScale", &instanceLighting.averageScale, sizeof(instanceLighting.averageScale))) ||
                FAILED(shader->Bind_RawValue("g_LightmapDirectionalScale", &instanceLighting.directionalScale, sizeof(instanceLighting.directionalScale)))) return E_FAIL;
        }
        if ((sourceProgram >= 33u && sourceProgram <= 63u) || sourceProgram == 65u || sourceProgram == 209u || movieForward)
        {
            float4_t ambient(0.f, 0.f, 0.f, 1.f);
            for (const auto& light : CGameInstance::Get().Get_SceneLights())
            {
                ambient.x += light.vAmbient.x;
                ambient.y += light.vAmbient.y;
                ambient.z += light.vAmbient.z;
            }
            if (FAILED(CGameInstance::Get().Bind_HeightFog(shader.get())) ||
                FAILED(shader->Bind_RawValue("g_SourceMapAmbient", &ambient, sizeof(ambient))) ||
                FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Depth"), shader,
                    "g_SourceMapSceneDepth"))) return E_FAIL;
            if (((sourceProgram >= 38u && sourceProgram <= 63u) || SourceMovieMaterial::Needs_SceneColor(sourceProgram)) &&
                FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_EffectSceneColor"), shader,
                    "g_SourceMapSceneColor"))) return E_FAIL;
        }
        if (((sourceProgram >= 38u && sourceProgram <= 63u) || sourceProgram == 209u || movieForward) &&
            FAILED(BindForwardSceneLights(shader, forwardBakedProgram && nativeSurface->hasBakedLighting && bakedLighting, worldCullSphere))) return E_FAIL;
        if ((sourceProgram >= 40u && sourceProgram <= 63u) || sourceProgram == 209u || movieForward)
        {
            const uint32_t hasBaked = forwardBakedProgram && nativeSurface->hasBakedLighting && bakedLighting ? 1u : 0u;
            const Engine::MODEL_BAKED_LIGHTING_INSTANCE emptyLighting{};
            const auto& lighting = bakedLighting ? *bakedLighting : emptyLighting;
            if (FAILED(shader->Bind_RawValue("g_HasBakedLighting", &hasBaked, sizeof(hasBaked))) ||
                FAILED(shader->Bind_RawValue("g_LightmapScaleBias", &lighting.scaleBias, sizeof(lighting.scaleBias))) ||
                FAILED(shader->Bind_RawValue("g_LightmapAverageScale", &lighting.averageScale, sizeof(lighting.averageScale))) ||
                FAILED(shader->Bind_RawValue("g_LightmapDirectionalScale", &lighting.directionalScale, sizeof(lighting.directionalScale))) ||
                (hasBaked && !hasStaticShadow && FAILED(model->Bind_SurfaceLighting(shader, meshIndex)))) return E_FAIL;
        }
        if (FAILED(BindSourceFoliageWind(shader, nativeSurface, elapsedTime, model->Is_Skinned()))) return E_FAIL;
        if (FAILED(model->Bind_SourceCharacter(shader, meshIndex))) return E_FAIL;
        return movieForward ? model->Bind_SourceCharacterForwardLight(shader, meshIndex) : S_OK;
    }

	const auto* surface = nativeSurface;
	const bool_t hasDefinition = surface &&
		surface->family != Engine::MODEL_SURFACE_FAMILY::LEGACY &&
		profile.renderMode == MAP_ASSET_RENDER_MODE::DEFERRED && !diffuseOverride;
	const uint32_t hasSurface = hasDefinition ? 1u : 0u;
	const uint32_t program = hasDefinition && settings.bUseSourceMaterials ?
		static_cast<uint32_t>(surface->family) : 0u;
	const uint32_t debugView = static_cast<uint32_t>(settings.eDebugView);
    const bool pbrComparisonActive = settings.MapPBR.Is_Active(Engine::CGameInstance::Get().Get_CurrentLevelID());
    const float4_t pbrContributions = pbrComparisonActive ? settings.MapPBR.vContributionScale : float4_t(1.f, 1.f, 1.f, 1.f);
    const float4_t pbrParameters = pbrComparisonActive ? settings.MapPBR.vSurfaceParameters : float4_t(1.f, 0.f, 0.f, 0.f);
    if (FAILED(shader->Bind_RawValue("g_MapPBRContributionScale", &pbrContributions, sizeof(pbrContributions))) ||
        FAILED(shader->Bind_RawValue("g_MapPBRDiagnosticParameters", &pbrParameters, sizeof(pbrParameters)))) return E_FAIL;
    if (FAILED(BindSourceFoliageWind(shader, surface, elapsedTime, model->Is_Skinned()))) return E_FAIL;
	// Bind last: the legacy diffuse binder resets source programs on shared shaders.
	if (FAILED(shader->Bind_RawValue("g_SurfaceProgram", &program, sizeof(program))) ||
		FAILED(shader->Bind_RawValue("g_HasSurfaceDefinition", &hasSurface, sizeof(hasSurface))) ||
		FAILED(shader->Bind_RawValue("g_SurfaceDebugView", &debugView, sizeof(debugView))))
		return E_FAIL;
    const uint32_t hasBaked = (program == 3u || program == 4u || program == 5u || program == 7u || program == 8u || program == 9u || program == 10u || (program >= 11u && program <= 13u)) && surface->hasBakedLighting ? 1u : 0u;
    // Most map surfaces have no source indirect input. Avoid copying the
    // environment's cube reference and path string for those ordinary draws.
    const bool sourceIndirectEnabled = surface && surface->hasSourceIndirect &&
        Engine::CGameInstance::Get().Get_RenderEnvironment().bUseSourcePBRIndirect;
    const uint32_t hasEnvironment = (program == 3u || program == 4u) && surface->hasEnvironmentCube &&
        (surface->environmentLegacyEnabled || sourceIndirectEnabled) ? 1u : 0u;
    const Engine::MODEL_BAKED_LIGHTING_INSTANCE emptyLighting{};
    const auto& lighting = bakedLighting ? *bakedLighting : emptyLighting;
    if (FAILED(shader->Bind_RawValue("g_HasBakedLighting", &hasBaked, sizeof(hasBaked))) ||
        FAILED(shader->Bind_RawValue("g_HasEnvironmentCube", &hasEnvironment, sizeof(hasEnvironment))) ||
        FAILED(shader->Bind_RawValue("g_HasEnvironmentBRDFLookup", &hasEnvironment, sizeof(hasEnvironment))) ||
        (bindingMode == MAP_MATERIAL_BINDING_MODE::OBJECT &&
        (FAILED(shader->Bind_RawValue("g_LightmapScaleBias", &lighting.scaleBias, sizeof(lighting.scaleBias))) ||
        FAILED(shader->Bind_RawValue("g_LightmapAverageScale", &lighting.averageScale, sizeof(lighting.averageScale))) ||
        FAILED(shader->Bind_RawValue("g_LightmapDirectionalScale", &lighting.directionalScale, sizeof(lighting.directionalScale)))))) return E_FAIL;
    // A static-shadow bind above already supplies this material's RNM and environment SRVs.
    // Reuse only within this call; sibling materials share the Effect and may replace them.
    if ((hasBaked || hasEnvironment) && !hasStaticShadow)
    {
        if (FAILED(model->Bind_SurfaceLighting(shader, meshIndex))) return E_FAIL;
    }
    if (hasEnvironment)
    {
        const auto& environmentColor = sourceIndirectEnabled ? surface->sourceIndirectColor : surface->environmentColor;
        const auto& environmentRotation = sourceIndirectEnabled ? surface->sourceIndirectRotation : surface->environmentRotation;
        if (FAILED(shader->Bind_RawValue("g_EnvironmentColor", &environmentColor, sizeof(environmentColor))) ||
            FAILED(shader->Bind_RawValue("g_EnvironmentRotation", &environmentRotation, sizeof(environmentRotation)))) return E_FAIL;
    }
    const uint32_t useSourceIndirect = hasEnvironment && sourceIndirectEnabled ? 1u : 0u;
    if (FAILED(shader->Bind_RawValue("g_UseSourcePBRIndirect", &useSourceIndirect, sizeof(useSourceIndirect)))) return E_FAIL;
    if (useSourceIndirect &&
        (FAILED(shader->Bind_RawValue("g_SourcePBRPackedSH", surface->sourceIndirectSH.data(), sizeof(surface->sourceIndirectSH))) ||
         FAILED(shader->Bind_RawValue("g_SourcePBRUpperSky", &surface->sourceUpperSkyColor, sizeof(surface->sourceUpperSkyColor))) ||
         FAILED(shader->Bind_RawValue("g_SourcePBRLowerSky", &surface->sourceLowerSkyColor, sizeof(surface->sourceLowerSkyColor))) ||
         FAILED(shader->Bind_RawValue("g_SourcePBRAmbientAndSkyFactor", &surface->sourceAmbientAndSkyFactor, sizeof(surface->sourceAmbientAndSkyFactor))))) return E_FAIL;
	const auto recordBinding = [&]()
	{
		if (hasDefinition && !diagnosticAssetId.empty())
			RecordSurfaceBinding(diagnosticAssetId, model->Get_MaterialName(meshIndex), *surface, program, lighting);
	};
	if (program == 0u)
	{
		recordBinding();
		return S_OK;
	}
	if ((program == 3u || program == 4u || program == 7u || program == 8u || program == 9u || program == 10u) && surface->hasEmissive)
	{
		if (!std::isfinite(elapsedTime))
			return E_INVALIDARG;
		if (FAILED(model->Bind_SurfaceTexture(shader, "g_SurfaceEmissiveTexture", meshIndex, aiTextureType_EMISSIVE)) ||
			FAILED(shader->Bind_RawValue("g_SurfaceEmissiveColor", &surface->emissiveColor, sizeof(surface->emissiveColor))) ||
			FAILED(shader->Bind_RawValue("g_SurfaceEmissiveIntensity", &surface->emissiveIntensity, sizeof(surface->emissiveIntensity))) ||
			FAILED(shader->Bind_RawValue("g_SurfaceEmissiveUVTiling", &surface->emissiveUVTiling, sizeof(surface->emissiveUVTiling))) ||
			FAILED(shader->Bind_RawValue("g_SurfaceEmissiveFlickerMinimum", &surface->emissiveFlickerMinimum, sizeof(surface->emissiveFlickerMinimum))) ||
			FAILED(shader->Bind_RawValue("g_SurfaceEmissiveFlickerSpeed", &surface->emissiveFlickerSpeed, sizeof(surface->emissiveFlickerSpeed))) ||
			FAILED(shader->Bind_RawValue("g_SurfaceEmissivePhaseOffset", &surface->emissivePhaseOffset, sizeof(surface->emissivePhaseOffset))) ||
			FAILED(shader->Bind_RawValue("g_SurfaceEmissiveTime", &elapsedTime, sizeof(elapsedTime))) ||
            ((program == 3u || program == 4u) &&
             FAILED(shader->Bind_RawValue("g_SourceBgFlicker", &surface->sourceBgFlicker, sizeof(surface->sourceBgFlicker)))))
			return E_FAIL;
		const uint32_t hasSurfaceEmissive = 1u;
		if (FAILED(shader->Bind_RawValue("g_HasSurfaceEmissive", &hasSurfaceEmissive, sizeof(hasSurfaceEmissive))))
			return E_FAIL;
	}
	const auto* camera = CGameInstance::Get().Get_CamPosition();
	if (!camera || FAILED(shader->Bind_RawValue("g_vCamPosition", camera, sizeof(*camera))) ||
		FAILED(model->Bind_SurfaceTexture(shader, "g_DiffuseTexture", meshIndex, aiTextureType_DIFFUSE)) ||
        (program != 7u && program != 8u && program != 9u && program != 10u && program != 12u && program != 14u && FAILED(model->Bind_SurfaceTexture(shader, "g_ReflectionTexture", meshIndex, aiTextureType_REFLECTION))) ||
		((program == 1u || program == 5u) && FAILED(model->Bind_SurfaceTexture(shader, "g_SpecularTexture", meshIndex, aiTextureType_SPECULAR))))
		return E_FAIL;
	if (FAILED(shader->Bind_RawValue("g_SurfaceDiffuseBrightness", &surface->diffuseBrightness, sizeof(surface->diffuseBrightness))))
		return E_FAIL;
	if (FAILED(shader->Bind_RawValue("g_SurfaceNormalIntensity", &surface->normalIntensity, sizeof(surface->normalIntensity))))
		return E_FAIL;
	if (FAILED(shader->Bind_RawValue("g_SurfaceSpecularIntensity", &surface->specularIntensity, sizeof(surface->specularIntensity))))
		return E_FAIL;
	if (FAILED(shader->Bind_RawValue("g_SurfaceSpecularPower", &surface->specularPower, sizeof(surface->specularPower))))
		return E_FAIL;
	if (FAILED(shader->Bind_RawValue("g_SurfaceReflectionIntensity", &surface->reflectionIntensity, sizeof(surface->reflectionIntensity))))
		return E_FAIL;
	if (FAILED(shader->Bind_RawValue("g_SurfaceReflectionContrast", &surface->reflectionContrast, sizeof(surface->reflectionContrast))))
		return E_FAIL;
	if (FAILED(shader->Bind_RawValue("g_SurfaceReflectionTiling", &surface->reflectionTiling, sizeof(surface->reflectionTiling))))
		return E_FAIL;
	if (FAILED(shader->Bind_RawValue("g_SurfaceDiffuseSaturation", &surface->diffuseSaturation, sizeof(surface->diffuseSaturation))))
		return E_FAIL;
	if (FAILED(shader->Bind_RawValue("g_SurfaceDiffuseColor", &surface->diffuseColor, sizeof(surface->diffuseColor))))
		return E_FAIL;
	if (FAILED(shader->Bind_RawValue("g_SurfaceSpecularColor", &surface->specularColor, sizeof(surface->specularColor))))
		return E_FAIL;
	if (FAILED(shader->Bind_RawValue("g_SurfaceReflectionColor", &surface->reflectionColor, sizeof(surface->reflectionColor))))
		return E_FAIL;
    if (program == 14u && FAILED(model->Bind_SourceLandscapeSurface(shader, meshIndex)))
        return E_FAIL;
    if (program >= 11u && program <= 13u && FAILED(model->Bind_SourceSpecialSurface(shader, meshIndex)))
        return E_FAIL;
    if (program == 9u || program == 10u)
    {
        const uint32_t flags = surface->sourceFoliageFlags;
        if (((flags & 1u) && FAILED(model->Bind_SurfaceTexture(shader, "g_NormalTexture", meshIndex, aiTextureType_NORMALS))) ||
            ((flags & 8u) && FAILED(model->Bind_SurfaceTexture(shader, "g_SpecularTexture", meshIndex, aiTextureType_SPECULAR))) ||
            (program == 9u && FAILED(model->Bind_SurfaceTexture(shader, "g_SourceFoliageMaskTexture", meshIndex, aiTextureType_TRANSMISSION))) ||
            FAILED(shader->Bind_RawValue("g_SourceFoliageFlags", &flags, sizeof(flags))) ||
            FAILED(shader->Bind_RawValue("g_SourceFoliageTransmission", &surface->sourceFoliageTransmission,
                sizeof(surface->sourceFoliageTransmission)))) return E_FAIL;
    }
    if (program == 8u)
    {
        const uint32_t flags = surface->sourceBgFlags;
        if (!std::isfinite(elapsedTime) ||
            FAILED(shader->Bind_RawValue("g_SourceBgSubspecular", &surface->sourceBgSubspecular, sizeof(surface->sourceBgSubspecular))) ||
            FAILED(shader->Bind_RawValue("g_SourceBgRimlight", &surface->sourceBgRimlight, sizeof(surface->sourceBgRimlight))) ||
            FAILED(shader->Bind_RawValue("g_SourceBgSpecularSaturation", &surface->sourceBgSpecularSaturation, sizeof(surface->sourceBgSpecularSaturation))) ||
            FAILED(shader->Bind_RawValue("g_SourceBgPanning", &surface->sourceBgPanning, sizeof(surface->sourceBgPanning))) ||
            (!surface->hasEmissive &&
             FAILED(shader->Bind_RawValue("g_SurfaceEmissiveTime", &elapsedTime, sizeof(elapsedTime))))) return E_FAIL;
        if ((flags & 32768u) &&
            (FAILED(model->Bind_SurfaceTexture(shader, "g_DetailNormalTexture", meshIndex, aiTextureType_HEIGHT)) ||
             FAILED(shader->Bind_RawValue("g_SurfaceDetailNormalIntensity", &surface->detailNormalIntensity, sizeof(surface->detailNormalIntensity))) ||
             FAILED(shader->Bind_RawValue("g_SurfaceDetailNormalTiling", &surface->detailNormalTiling, sizeof(surface->detailNormalTiling))))) return E_FAIL;
        if (((flags & 1u) && FAILED(model->Bind_SurfaceTexture(shader, "g_NormalTexture", meshIndex, aiTextureType_NORMALS))) ||
            ((flags & 8u) && FAILED(model->Bind_SurfaceTexture(shader, "g_SpecularTexture", meshIndex, aiTextureType_SPECULAR))) ||
            ((flags & 16u) && FAILED(model->Bind_SurfaceTexture(shader, "g_ReflectionTexture", meshIndex, aiTextureType_REFLECTION))) ||
            FAILED(shader->Bind_RawValue("g_SourceBgFlags", &flags, sizeof(flags))) ||
            FAILED(shader->Bind_RawValue("g_SourceBgBump", &surface->sourceBgBump, sizeof(surface->sourceBgBump))) ||
            FAILED(shader->Bind_RawValue("g_SourceBgUV", &surface->sourceBgUV, sizeof(surface->sourceBgUV))) ||
            FAILED(shader->Bind_RawValue("g_SourceBgFlicker", &surface->sourceBgFlicker, sizeof(surface->sourceBgFlicker))) ||
            FAILED(shader->Bind_RawValue("g_SurfaceUVTiling", &surface->uvTiling, sizeof(surface->uvTiling))) ||
            FAILED(shader->Bind_RawValue("g_SurfaceReflectionOriginOffset", &surface->reflectionOriginOffset, sizeof(surface->reflectionOriginOffset))))
            return E_FAIL;
    }
    if (program == 5u)
    {
        if (FAILED(model->Bind_SurfaceTexture(shader, "g_NormalTexture", meshIndex, aiTextureType_NORMALS)) ||
            FAILED(shader->Bind_RawValue("g_SurfaceUVTiling", &surface->uvTiling, sizeof(surface->uvTiling))) ||
            FAILED(shader->Bind_RawValue("g_SurfaceReflectionOriginOffset", &surface->reflectionOriginOffset, sizeof(surface->reflectionOriginOffset))))
            return E_FAIL;
    }
    if (program == 7u)
    {
        const uint32_t separateSpecular = surface->overlaySeparateSpecular ? 1u : 0u;
        if (FAILED(shader->Bind_RawValue("g_SourceBgSubspecular", &surface->sourceBgSubspecular, sizeof(surface->sourceBgSubspecular))) ||
            FAILED(shader->Bind_RawValue("g_SourceBgSpecularSaturation", &surface->sourceBgSpecularSaturation, sizeof(surface->sourceBgSpecularSaturation))) ||
            FAILED(shader->Bind_RawValue("g_SourceBgBump", &surface->sourceBgBump, sizeof(surface->sourceBgBump))) ||
            FAILED(shader->Bind_RawValue("g_SourceOverlayFlags", &surface->sourceOverlayFlags, sizeof(surface->sourceOverlayFlags))) ||
            FAILED(shader->Bind_RawValue("g_SourceOverlayDirection", &surface->sourceOverlayDirection, sizeof(surface->sourceOverlayDirection))) ||
            FAILED(shader->Bind_RawValue("g_SourceOverlayUV", &surface->sourceBgUV, sizeof(surface->sourceBgUV))) ||
            FAILED(shader->Bind_RawValue("g_SurfaceDetailNormalIntensity", &surface->detailNormalIntensity, sizeof(surface->detailNormalIntensity))) ||
            FAILED(shader->Bind_RawValue("g_SurfaceDetailNormalTiling", &surface->detailNormalTiling, sizeof(surface->detailNormalTiling))) ||
            ((surface->sourceOverlayFlags & 32u) != 0u && FAILED(model->Bind_SurfaceTexture(shader, "g_DetailNormalTexture", meshIndex, aiTextureType_HEIGHT)))) return E_FAIL;
        if (FAILED(shader->Bind_RawValue("g_SurfaceOverlaySeparateSpecular", &separateSpecular, sizeof(separateSpecular))) ||
            FAILED(shader->Bind_RawValue("g_SurfaceUVTiling", &surface->uvTiling, sizeof(surface->uvTiling))) ||
            (separateSpecular && FAILED(model->Bind_SurfaceTexture(shader, "g_SpecularTexture", meshIndex, aiTextureType_SPECULAR)))) return E_FAIL;
        if (((surface->sourceOverlayFlags & 1u) != 0u && FAILED(model->Bind_SurfaceTexture(shader, "g_NormalTexture", meshIndex, aiTextureType_NORMALS))) ||
            FAILED(model->Bind_SurfaceTexture(shader, "g_SurfaceOverlayDiffuseTexture", meshIndex, aiTextureType_BASE_COLOR)) ||
            ((surface->sourceOverlayFlags & 2u) != 0u && FAILED(model->Bind_SurfaceTexture(shader, "g_SurfaceOverlayNormalTexture", meshIndex, aiTextureType_NORMAL_CAMERA))) ||
            FAILED(shader->Bind_RawValue("g_SurfaceOverlayColor", &surface->overlayColor, sizeof(surface->overlayColor))) ||
            FAILED(shader->Bind_RawValue("g_SurfaceOverlayTiling", &surface->overlayTiling, sizeof(surface->overlayTiling))) ||
            FAILED(shader->Bind_RawValue("g_SurfaceOverlayNormalIntensity", &surface->overlayNormalIntensity, sizeof(surface->overlayNormalIntensity))) ||
            FAILED(shader->Bind_RawValue("g_SurfaceOverlaySharpness", &surface->overlaySharpness, sizeof(surface->overlaySharpness))) ||
            FAILED(shader->Bind_RawValue("g_SurfaceOverlayBrightness", &surface->overlayBrightness, sizeof(surface->overlayBrightness))) ||
            FAILED(shader->Bind_RawValue("g_SurfaceOverlaySaturation", &surface->overlaySaturation, sizeof(surface->overlaySaturation))) ||
            FAILED(shader->Bind_RawValue("g_SurfaceOverlaySpecularIntensity", &surface->overlaySpecularIntensity, sizeof(surface->overlaySpecularIntensity)))) return E_FAIL;
    }
	if (program == 3u || program == 4u)
	{
		const uint32_t masked = surface->pbrAlphaMasked ? 1u : 0u;
		if (FAILED(shader->Bind_RawValue("g_SurfacePBRMasked", &masked, sizeof(masked))))
			return E_FAIL;
		if (FAILED(model->Bind_SurfaceTexture(shader, "g_NormalTexture", meshIndex, aiTextureType_NORMALS)) ||
			FAILED(model->Bind_SurfaceTexture(shader, "g_DetailNormalTexture", meshIndex, aiTextureType_HEIGHT)) ||
			FAILED(model->Bind_SurfaceTexture(shader, "g_SurfaceORMTexture", meshIndex, aiTextureType_UNKNOWN)))
			return E_FAIL;
		if (FAILED(shader->Bind_RawValue("g_SurfaceUVTiling", &surface->uvTiling, sizeof(surface->uvTiling))))
			return E_FAIL;
		if (FAILED(shader->Bind_RawValue("g_SurfaceDetailNormalIntensity", &surface->detailNormalIntensity, sizeof(surface->detailNormalIntensity))))
			return E_FAIL;
		if (FAILED(shader->Bind_RawValue("g_SurfaceDetailNormalTiling", &surface->detailNormalTiling, sizeof(surface->detailNormalTiling))))
			return E_FAIL;
		if (FAILED(shader->Bind_RawValue("g_SurfaceMetallicIntensity", &surface->metallicIntensity, sizeof(surface->metallicIntensity))))
			return E_FAIL;
		if (FAILED(shader->Bind_RawValue("g_SurfaceMetallicPower", &surface->metallicPower, sizeof(surface->metallicPower))))
			return E_FAIL;
		if (FAILED(shader->Bind_RawValue("g_SurfaceRoughnessIntensity", &surface->roughnessIntensity, sizeof(surface->roughnessIntensity))))
			return E_FAIL;
		if (FAILED(shader->Bind_RawValue("g_SurfaceRoughnessPower", &surface->roughnessPower, sizeof(surface->roughnessPower))))
			return E_FAIL;
		if (FAILED(shader->Bind_RawValue("g_SurfaceAOIntensity", &surface->aoIntensity, sizeof(surface->aoIntensity))))
			return E_FAIL;
		if (FAILED(shader->Bind_RawValue("g_SurfaceAOPower", &surface->aoPower, sizeof(surface->aoPower))))
			return E_FAIL;
		if (FAILED(shader->Bind_RawValue("g_SurfaceSpecularPBRIntensity", &surface->specularPBRIntensity, sizeof(surface->specularPBRIntensity))))
			return E_FAIL;
		if (FAILED(shader->Bind_RawValue("g_SurfaceNonmetallicBrightness", &surface->nonmetallicBrightness, sizeof(surface->nonmetallicBrightness))))
			return E_FAIL;
		if (FAILED(shader->Bind_RawValue("g_SurfaceMetallicBrightness", &surface->metallicBrightness, sizeof(surface->metallicBrightness))))
			return E_FAIL;
		if (FAILED(shader->Bind_RawValue("g_SurfaceMinimumRoughness", &surface->minimumRoughness, sizeof(surface->minimumRoughness))))
			return E_FAIL;
		if (FAILED(shader->Bind_RawValue("g_SurfaceReflectionOriginOffset", &surface->reflectionOriginOffset, sizeof(surface->reflectionOriginOffset))))
			return E_FAIL;
		if (FAILED(shader->Bind_RawValue("g_SurfaceVertexAlpha", &surface->vertexAlpha, sizeof(surface->vertexAlpha))))
			return E_FAIL;
		const uint32_t uvFixedNormal = surface->uvFixedNormal ? 1u : 0u;
		if (FAILED(shader->Bind_RawValue("g_SurfaceUVFixedNormal", &uvFixedNormal, sizeof(uvFixedNormal))))
			return E_FAIL;
		const uint32_t useWorldReflection = surface->useWorldReflection ? 1u : 0u;
		if (FAILED(shader->Bind_RawValue("g_SurfaceUseWorldReflection", &useWorldReflection, sizeof(useWorldReflection))))
			return E_FAIL;
	}
	recordBinding();
	return S_OK;
}
```

#### `Client/Private/MapStaticBatchObject.cpp`

후보 SHA256: `321eb6146656f8b5bb4900f745da268dd4394a24480e94eb91ec3d3a08d1be0d`

```cpp
#include "MapStaticBatchObject.h"
#pragma push_macro("new")
#undef new
#include "Engine_RenderTypes.h"
#pragma pop_macro("new")
#include "Engine_VertexTypes.h"

#include "GameInstance.h"
#include "MapAssetRenderUtils.h"
#include "Model.h"
#include "MeshLod.h"
#include "Profiler.h"
#include "EffectFailureDiagnostic.h"
#include "Shader.h"

#include <algorithm>
#include <cstring>
#include <limits>
#include <cmath>
#include <cfloat>
#include <sstream>

namespace
{
    // Conservative operator-norm bound for signed/nonuniform scale and shear.
    float LinearScaleBound(const float4x4_t& matrix)
    {
        double maximum = 0.;
        for (size_t i = 0u; i < 3u; ++i)
        {
            double row = 0.;
            for (size_t j = 0u; j < 3u; ++j)
            {
                double dot = 0.;
                for (size_t k = 0u; k < 3u; ++k) dot += double(matrix.m[i][k]) * matrix.m[j][k];
                row += std::abs(dot);
            }
            if (!std::isfinite(row)) return 0.f;
            maximum = (std::max)(maximum, row);
        }
        const double scale = std::sqrt(maximum);
        if (!std::isfinite(scale) || scale <= 0. || scale >= (std::numeric_limits<float>::max)()) return 0.f;
        return std::nextafter(static_cast<float>(scale), (std::numeric_limits<float>::infinity)());
    }
    struct INSTANCE_ENVELOPE final
    {
        double minimum[3] = { DBL_MAX, DBL_MAX, DBL_MAX };
        double maximum[3] = { -DBL_MAX, -DBL_MAX, -DBL_MAX };
        bool valid = true, any = false;
        float maximumScale = 0.f;
        void Add(const FMapStaticInstance& instance, const float scale)
        {
            const float* center = &instance.WorldBoundsCenter.x;
            if (!std::isfinite(instance.WorldBoundsRadius) || instance.WorldBoundsRadius <= 0.f) valid = false;
            for (size_t axis = 0u; axis < 3u; ++axis)
            {
                if (!std::isfinite(center[axis])) valid = false;
                minimum[axis] = (std::min)(minimum[axis], double(center[axis]) - instance.WorldBoundsRadius);
                maximum[axis] = (std::max)(maximum[axis], double(center[axis]) + instance.WorldBoundsRadius);
            }
            if (scale <= 0.f) valid = false;
            maximumScale = (std::max)(maximumScale, scale);
            any = true;
        }
        bool Store(float4_t& sphere) const
        {
            if (!valid || !any) return false;
            double radiusSquared = 0.;
            for (size_t axis = 0u; axis < 3u; ++axis)
            {
                const double c = (minimum[axis] + maximum[axis]) * .5;
                if (!std::isfinite(c) || std::abs(c) > (std::numeric_limits<float>::max)()) return false;
                (&sphere.x)[axis] = static_cast<float>(c);
                const double extent = (std::max)(maximum[axis] - (&sphere.x)[axis], double((&sphere.x)[axis]) - minimum[axis]);
                radiusSquared += extent * extent;
            }
            const double radius = std::sqrt(radiusSquared);
            if (!std::isfinite(radius) || radius <= 0. || radius >= (std::numeric_limits<float>::max)()) return false;
            sphere.w = std::nextafter(static_cast<float>(radius), (std::numeric_limits<float>::infinity)());
            return true;
        }
    };

    struct VIEW_LOD_ENVELOPE final
    {
        const float4x4_t* view = nullptr;
        double viewScale = 0.;
        double maximumX = 0., maximumY = 0., minimumZ = DBL_MAX;
        bool valid = false, any = false;

        explicit VIEW_LOD_ENVELOPE(const MAP_CAMERA_CULL_SNAPSHOT* camera)
        {
            if (!camera) return;
            view = &camera->view;
            if (view->_14 != 0.f || view->_24 != 0.f || view->_34 != 0.f || view->_44 != 1.f) return;
            viewScale = LinearScaleBound(*view);
            valid = viewScale > 0.;
        }

        void Add(const FMapStaticInstance& instance)
        {
            if (!valid) return;
            const double radius = double(instance.WorldBoundsRadius) * viewScale;
            if (!std::isfinite(radius) || radius <= 0.) { valid = false; return; }
            double center[3]{}, margin[3]{};
            for (size_t axis = 0u; axis < 3u; ++axis)
            {
                center[axis] = view->m[3][axis];
                double magnitude = std::abs(center[axis]);
                for (size_t source = 0u; source < 3u; ++source)
                {
                    const double term = double((&instance.WorldBoundsCenter.x)[source]) * view->m[source][axis];
                    center[axis] += term;
                    magnitude += std::abs(term);
                }
                // Enclose float matrix evaluation as well as the sphere itself,
                // including cancellation in translated/rotated view coordinates.
                margin[axis] = 16. * FLT_EPSILON * (magnitude + radius + 1.);
                if (!std::isfinite(center[axis]) || !std::isfinite(margin[axis])) { valid = false; return; }
            }
            maximumX = (std::max)(maximumX, std::abs(center[0]) + radius + margin[0]);
            maximumY = (std::max)(maximumY, std::abs(center[1]) + radius + margin[1]);
            minimumZ = (std::min)(minimumZ, center[2] - radius - margin[2]);
            any = true;
        }

        bool Store(float4_t& result) const
        {
            if (!valid || !any || maximumX >= FLT_MAX || maximumY >= FLT_MAX || std::abs(minimumZ) >= FLT_MAX)
                return false;
            result = {
                std::nextafter(static_cast<float>(maximumX), (std::numeric_limits<float>::infinity)()),
                std::nextafter(static_cast<float>(maximumY), (std::numeric_limits<float>::infinity)()),
                std::nextafter(static_cast<float>(minimumZ), -(std::numeric_limits<float>::infinity)()), 1.f };
            return true;
        }
    };

}

CMapStaticBatchObject::CMapStaticBatchObject(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CGameObject{ pDevice, pContext }
{}

CMapStaticBatchObject::CMapStaticBatchObject(
	const CMapStaticBatchObject& prototype)
	: CGameObject{ prototype }
{}

CMapStaticBatchObject::~CMapStaticBatchObject()
{}

HRESULT CMapStaticBatchObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMapStaticBatchObject::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_INVALIDARG;

	const DESC& desc =
		*static_cast<DESC*>(pArg);

	if (desc.AssetId.empty() ||
		desc.ModelPrototypeTag.empty() ||
		desc.Instances.empty() ||
		MAP_ASSET_RENDER_MODE::DEFERRED !=
		desc.RenderProfile.renderMode)
	{
		return E_INVALIDARG;
	}

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_AssetId = desc.AssetId;
	m_AssetGroupId = desc.AssetGroupId;
	m_RenderProfile = desc.RenderProfile;
	m_FrustumCulling = desc.FrustumCulling;
	m_bMirrored = desc.Mirrored;
	m_Instances = desc.Instances;

    // Hidden gate batches become visible together at the cinematic handoff.
    // Allocate both streams while staging so their first shadow draw only uploads.
	if (FAILED(Ready_Components(
		desc.PrototypeLevelIndex,
		desc.ModelPrototypeTag)) ||
		FAILED(Rebuild_PlacementLookup()) ||
		FAILED(Ensure_InstanceCapacity(
			static_cast<uint32_t>(
				m_Instances.size()))) ||
        (m_RenderProfile.castsShadow && FAILED(Ensure_ShadowInstanceCapacity(
            static_cast<uint32_t>(m_Instances.size())))))
	{
		return E_FAIL;
	}

	return S_OK;
}

void CMapStaticBatchObject::Update(
	f32_t fTimeDelta)
{
	m_fElapsedTime += fTimeDelta;
}

void CMapStaticBatchObject::Late_Update(
	f32_t fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);
	m_bFinalCameraPrepared = false;

	if (Engine::CProfiler* profiler =
		CGameInstance::Get().Get_Profiler())
	{
		profiler->Add_Counter(
			Engine::EProfilerCounter::MapPlacements,
			m_Instances.size());

		profiler->Add_Counter(
			Engine::EProfilerCounter::MapBatchCount);
	}
}

void CMapStaticBatchObject::Submit_FinalCamera()
{
    auto& game = CGameInstance::Get();
    if (game.Is_SceneEnvironmentReplaced() || m_iAuthoredVisibleInstanceCount == 0u)
        return;

    const auto* camera = CMapAssetRenderUtils::Capture_CameraCullSnapshotView();
    const HRESULT visibility = Upload_VisibleInstances(camera);
    m_bFinalCameraPrepared = SUCCEEDED(visibility);
    // A failed preparation keeps the original draw callback as the error/retry path.
    if (FAILED(visibility) || !m_VisibleInstances.empty())
        game.Add_RenderObject(RENDERGROUP::NONBLEND,
            static_pointer_cast<CGameObject>(shared_from_this()));

    // Camera rejection does not reject a caster whose shadow reaches the view.
    // Providers have now committed the final shadow switch and light volume.
    if (m_RenderProfile.castsShadow && game.Is_ShadowLightEnabled())
    {
        const HRESULT shadows = Upload_ShadowInstances();
        if (FAILED(shadows) || !m_ShadowInstances.empty())
            game.Add_RenderObject(RENDERGROUP::SHADOW,
                static_pointer_cast<CGameObject>(shared_from_this()));
    }
}

HRESULT CMapStaticBatchObject::Render()
{
	Engine::CProfiler* const profiler = CGameInstance::Get().Get_Profiler();
	Engine::CProfilerWorkScope renderWork(profiler, Engine::EProfilerWork::MapBatchRender);
	if (CGameInstance::Get().Is_SceneEnvironmentReplaced())
		return S_OK;

	const MAP_CAMERA_CULL_SNAPSHOT* cameraSnapshot =
		CMapAssetRenderUtils::Capture_CameraCullSnapshotView();
	const bool_t hasCameraSnapshot = nullptr != cameraSnapshot;
	const bool_t preparedForCamera = m_bFinalCameraPrepared &&
		m_bVisibleInstancesUsedCamera == hasCameraSnapshot &&
		m_iVisibleCameraRevision == (hasCameraSnapshot ? cameraSnapshot->revision : 0u);
	if (!preparedForCamera && FAILED(Upload_VisibleInstances(cameraSnapshot)))
	{
		return E_FAIL;
	}
	if (m_VisibleInstances.empty())
	{
		if (profiler) profiler->Add_Counter(Engine::EProfilerCounter::MapBatchEmptyRenders);
		return S_OK;
	}
	if (profiler) profiler->Add_Counter(Engine::EProfilerCounter::MapBatchVisibleRenders);

	const HRESULT cameraBindResult = hasCameraSnapshot ?
		CMapAssetRenderUtils::Bind_CameraCullSnapshot(
			m_pShaderCom, *cameraSnapshot) :
		(FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)) ||
		 FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)) ? E_FAIL : S_OK);
	if (FAILED(cameraBindResult))
	{
		return E_FAIL;
	}

	const uint32_t passIndex =
		CMapAssetRenderUtils::Select_Pass(
			m_RenderProfile,
			m_bMirrored);

	if (passIndex > 2u)
		return E_UNEXPECTED;

	const uint32_t instanceCount =
		static_cast<uint32_t>(
			m_VisibleInstances.size());

    Engine::MESH_SCREEN_LOD_DESC screenLod{};
    const bool_t hasScreenLod = hasCameraSnapshot && m_RenderProfile.opacity >= 1.f &&
        Build_ScreenLodView(*cameraSnapshot, screenLod);
	const bool_t useSourceMaterials =
		CGameInstance::Get().Get_MaterialRenderSettings().bUseSourceMaterials;
	{
		Engine::CProfilerDetailScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Map.Batch.BindAndDraw");
	for (uint32_t meshIndex = 0;
		meshIndex < m_pModelCom->Get_NumMeshes();
		++meshIndex)
	{
		const auto* surface = m_pModelCom->Get_MaterialSurface(meshIndex);
        // Material variants share CMesh geometry: re-admit the current draw's
        // material instead of inheriting the source model's LOD eligibility.
        const bool_t useMeshLod = hasScreenLod && useSourceMaterials && surface &&
            surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED &&
            (surface->sourceBgFlags & 64u) == 0u &&
            (surface->renderMode == Engine::MODEL_SURFACE_RENDER_MODE::INHERIT ||
             surface->renderMode == Engine::MODEL_SURFACE_RENDER_MODE::DEFERRED) &&
            !m_pModelCom->Has_MaterialTexture(meshIndex, aiTextureType_OPACITY);
		const uint32_t meshPass = useSourceMaterials && surface &&
			surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED ?
			24u + passIndex : passIndex;
		{
			Engine::CProfilerDetailScope scope(CGameInstance::Get().Get_Profiler(), "Map.Batch.Material.Bind");
			Engine::CProfilerWorkScope work(profiler, Engine::EProfilerWork::MapBatchMaterial);
			if (FAILED(CMapAssetRenderUtils::Bind_Material(m_pModelCom, m_pShaderCom,
				meshIndex, m_RenderProfile, m_fElapsedTime, nullptr, m_AssetId, nullptr, nullptr,
				MAP_MATERIAL_BINDING_MODE::INSTANCED))) return E_FAIL;
		}
		{
			Engine::CProfilerDetailScope scope(CGameInstance::Get().Get_Profiler(), "Map.Batch.Pass.Apply");
			Engine::CProfilerWorkScope work(profiler, Engine::EProfilerWork::MapBatchPass);
			if (FAILED(m_pShaderCom->Begin(meshPass))) return E_FAIL;
		}
		{
			Engine::CProfilerDetailScope scope(CGameInstance::Get().Get_Profiler(), "Map.Batch.Mesh.Submit");
			Engine::CProfilerWorkScope work(profiler, Engine::EProfilerWork::MapBatchDraw);
			if (FAILED(m_pModelCom->Render_Instanced(meshIndex, m_pInstanceBuffer.Get(),
				sizeof(VTXMESHINSTANCE), instanceCount, 0u, useMeshLod ? &screenLod : nullptr))) return E_FAIL;
		}
	}
	}

	return S_OK;
}

HRESULT CMapStaticBatchObject::Render_Shadow()
{
	if (CGameInstance::Get().Is_SceneEnvironmentReplaced())
		return S_OK;

	if (!m_RenderProfile.castsShadow)
		return S_OK;
	// Keep the failing asset and operation; the renderer only knows the object type.
	const auto fail = [this](const char* stage, HRESULT result,
		uint32_t mesh = UINT_MAX, uint32_t pass = UINT_MAX) noexcept -> HRESULT
	{
		try
		{
			std::ostringstream detail;
			detail << "stage=" << stage << " asset=" << std::quoted(m_AssetId)
				<< " mesh=" << mesh << " pass=" << pass
				<< " instances=" << m_ShadowInstances.size()
				<< " hr=0x" << std::hex << static_cast<unsigned long>(result)
				<< " device_hr=0x" << static_cast<unsigned long>(m_pDevice->GetDeviceRemovedReason());
			Write_EffectFailureDiagnostic("Map.StaticBatch.Shadow", detail.str());
		}
		catch (...) { }
		return result;
	};
	// Frame providers can change the light after Late_Update queued this batch.
	HRESULT result = Upload_ShadowInstances();
	if (FAILED(result))
		return fail("UploadInstances", result);
	if (m_ShadowInstances.empty())
		return S_OK;
	const bool_t useSourceMaterials =
		CGameInstance::Get().Get_MaterialRenderSettings().bUseSourceMaterials;

	result = CGameInstance::Get().Bind_ShadowLight_ShaderResource(
		m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW);
	if (FAILED(result)) return fail("BindLightView", result);
	result = CGameInstance::Get().Bind_ShadowLight_ShaderResource(
		m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ);
	if (FAILED(result)) return fail("BindLightProjection", result);

	const uint32_t iCullPass =
		CMapAssetRenderUtils::Select_Pass(
			m_RenderProfile, m_bMirrored);
	if (iCullPass > 2u)
		return fail("SelectPass", E_UNEXPECTED);

	const uint32_t iInstanceCount =
		static_cast<uint32_t>(m_ShadowInstances.size());
	for (uint32_t iMesh = 0;
		iMesh < m_pModelCom->Get_NumMeshes(); ++iMesh)
	{
		const auto* surface = m_pModelCom->Get_MaterialSurface(iMesh);
		if (surface && !surface->castsShadow)
			continue;
		const bool_t opaqueShadow = CMapAssetRenderUtils::Uses_OpaqueShadowPass(
			surface, m_RenderProfile, useSourceMaterials);
		uint32_t shadowPassBase = opaqueShadow ? 21u : 12u;
		if (!opaqueShadow)
		{
			// Simple families retain their alpha test when source mode is off or masked.
			if (!surface || surface->family == Engine::MODEL_SURFACE_FAMILY::LEGACY ||
				surface->family == Engine::MODEL_SURFACE_FAMILY::SPECULAR_TEXTURE_REFLECTION ||
				surface->family == Engine::MODEL_SURFACE_FAMILY::DIFFUSE_SPECULAR_REFLECTION ||
				surface->family == Engine::MODEL_SURFACE_FAMILY::PBR_SEAMLESS_OPAQUE ||
				surface->family == Engine::MODEL_SURFACE_FAMILY::PBR_OPAQUE ||
				surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_SPECULAR_OPAQUE)
				shadowPassBase = 18u;
			Engine::CProfilerDetailScope scope(CGameInstance::Get().Get_Profiler(), "Map.Shadow.Material.Bind");
			result = CMapAssetRenderUtils::Bind_ShadowMaterial(m_pModelCom, m_pShaderCom,
				iMesh, m_RenderProfile, m_fElapsedTime);
			if (FAILED(result)) return fail("BindMaterial", result, iMesh, shadowPassBase + iCullPass);
		}
		{
			Engine::CProfilerDetailScope scope(CGameInstance::Get().Get_Profiler(), "Map.Shadow.Pass.Apply");
			result = m_pShaderCom->Begin(shadowPassBase + iCullPass);
			if (FAILED(result)) return fail("ApplyPass", result, iMesh, shadowPassBase + iCullPass);
		}
		{
			Engine::CProfilerDetailScope scope(CGameInstance::Get().Get_Profiler(), "Map.Shadow.Mesh.Submit");
			result = m_pModelCom->Render_Instanced(iMesh, m_pShadowInstanceBuffer.Get(),
				sizeof(VTXMESHINSTANCE), iInstanceCount);
			if (FAILED(result)) return fail("DrawMesh", result, iMesh, shadowPassBase + iCullPass);
		}
	}

	return S_OK;
}

bool_t CMapStaticBatchObject::Try_GetStaticShadowRevision(uint64_t& outRevision) const
{
	outRevision = 0u;
	if (m_iStaticShadowRevision == 0u || !m_RenderProfile.castsShadow ||
		!m_pModelCom || m_pModelCom->Get_NumMeshes() == 0u ||
		!CGameInstance::Get().Get_MaterialRenderSettings().bUseSourceMaterials)
		return false;

	if (!m_bStaticShadowMaterialInputs)
		return false;
	// Surface/profile constants were admitted when this model was staged.
	// Mutable texture overrides and morph clones still invalidate cache use.
	for (const uint32_t mesh : m_StaticShadowCasterMeshes)
	{
		if (m_pModelCom->Has_MorphBaseVertices(mesh) ||
			m_pModelCom->Has_MaterialTextureOverrides(mesh))
			return false;
	}

	outRevision = m_iStaticShadowRevision;
	return true;
}

bool_t CMapStaticBatchObject::Try_PickMovementSurface(
	const float3_t& rayOrigin, const float3_t& rayDirection,
	const f32_t maxDistance, f32_t& outDistance) const
{
	if (!m_pModelCom || m_RenderProfile.opacity <= 0.f ||
		m_RenderProfile.renderMode != MAP_ASSET_RENDER_MODE::DEFERRED)
		return false;
	const vector_t origin = XMLoadFloat3(&rayOrigin);
	const vector_t direction = XMLoadFloat3(&rayDirection);
	// The cached envelope contains every authored-visible instance. A dirty
	// envelope cannot reject the current transform, so retain the instance scan.
	if (!m_bBatchBoundsDirty && m_bHasBatchBounds)
	{
		const BoundingBox bounds(
			float3_t(m_BatchBounds.x, m_BatchBounds.y, m_BatchBounds.z),
			float3_t(m_BatchBounds.w, m_BatchBounds.w, m_BatchBounds.w));
		f32_t entry = 0.f;
		if (!bounds.Intersects(origin, direction, entry) || entry > maxDistance)
			return false;
	}
	const uint32_t cull = CMapAssetRenderUtils::Select_Pass(m_RenderProfile, m_bMirrored) % 3u;
	const auto cullMode = cull == 0u ? CModel::PICK_CULL_MODE::BACK :
		cull == 1u ? CModel::PICK_CULL_MODE::FRONT : CModel::PICK_CULL_MODE::NONE;
	f32_t nearest = maxDistance;
	bool_t hit = false;
	for (const auto& instance : m_Instances)
	{
		if (!instance.Visible || instance.Suppressed || instance.CameraPreviewSuppressed)
			continue;
		const BoundingBox bounds(instance.WorldBoundsCenter, float3_t(
			instance.WorldBoundsRadius, instance.WorldBoundsRadius, instance.WorldBoundsRadius));
		f32_t boundDistance = 0.f;
		if (!bounds.Intersects(origin, direction, boundDistance) || boundDistance > nearest)
			continue;
		for (uint32_t mesh = 0u; mesh < m_pModelCom->Get_NumMeshes(); ++mesh)
		{
			const auto* surface = m_pModelCom->Get_MaterialSurface(mesh);
			// Masked floor geometry remains eligible; GPU alpha coverage is not
			// the movement contract. Animated foliage and shader displacement are excluded.
			if (m_pModelCom->Has_MorphBaseVertices(mesh) ||
				(surface && (surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_FOLIAGE_MASKED ||
					surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_GRASS_MASKED ||
					(surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_CHARACTER &&
						surface->sourceCharacter.program == 43u))))
				continue;
			f32_t distance = nearest;
			if (m_pModelCom->Try_PickStaticSurface(mesh, instance.World, rayOrigin, rayDirection,
				nearest, cullMode, distance) && distance < nearest)
			{
				nearest = distance;
				hit = true;
			}
		}
	}
	if (hit) outDistance = nearest;
	return hit;
}

HRESULT CMapStaticBatchObject::Update_Instance(
	uint64_t placementId,
	const FMapStaticInstance& instance)
{
	const auto iter =
		m_PlacementLookup.find(placementId);

	if (iter == m_PlacementLookup.end() ||
		instance.PlacementId != placementId)
	{
		return E_INVALIDARG;
	}

	FMapStaticInstance& current = m_Instances[iter->second];
	// Bounds participate in light-volume culling even when the world is unchanged.
	const bool_t shadowChanged = current.Visible != instance.Visible ||
		0 != std::memcmp(&current.World, &instance.World, sizeof(current.World)) ||
		0 != std::memcmp(&current.WorldInvTranspose, &instance.WorldInvTranspose,
			sizeof(current.WorldInvTranspose)) ||
		0 != std::memcmp(&current.WorldBoundsCenter, &instance.WorldBoundsCenter,
			sizeof(current.WorldBoundsCenter)) ||
		current.WorldBoundsRadius != instance.WorldBoundsRadius;
	if (shadowChanged && m_iStaticShadowRevision != 0u)
		++m_iStaticShadowRevision;
	if (current.Visible != instance.Visible)
	{
		if (instance.Visible)
			++m_iAuthoredVisibleInstanceCount;
		else
			--m_iAuthoredVisibleInstanceCount;
	}
	const bool_t suppressed = current.Suppressed;
	const bool_t cameraPreviewSuppressed = current.CameraPreviewSuppressed;
	current = instance;
	current.Suppressed = suppressed;
	current.CameraPreviewSuppressed = cameraPreviewSuppressed;
    m_InstanceLinearScaleBounds[iter->second] = LinearScaleBound(current.World);
    m_bBatchBoundsDirty = true;
	m_bShadowInstancesDirty = true;
	m_bVisibleInstancesDirty = true;
	m_bFinalCameraPrepared = false;
	return S_OK;
}

HRESULT CMapStaticBatchObject::Set_InstanceVisible(
	uint64_t placementId,
	bool_t visible)
{
	const auto iter =
		m_PlacementLookup.find(placementId);

	if (iter == m_PlacementLookup.end())
		return HRESULT_FROM_WIN32(
			ERROR_NOT_FOUND);

	FMapStaticInstance& instance = m_Instances[iter->second];
	if (instance.Visible != visible)
	{
		if (visible)
			++m_iAuthoredVisibleInstanceCount;
		else
			--m_iAuthoredVisibleInstanceCount;
		instance.Visible = visible;
		if (m_iStaticShadowRevision != 0u)
			++m_iStaticShadowRevision;
        m_bBatchBoundsDirty = true;
		m_bShadowInstancesDirty = true;
		m_bVisibleInstancesDirty = true;
		m_bFinalCameraPrepared = false;
	}
	return S_OK;
}

HRESULT CMapStaticBatchObject::Try_GetInstanceVisible(
	const uint64_t placementId,
	bool_t& outVisible) const
{
	const auto iter = m_PlacementLookup.find(placementId);
	if (iter == m_PlacementLookup.end() || iter->second >= m_Instances.size())
		return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
	outVisible = m_Instances[iter->second].Visible;
	return S_OK;
}

HRESULT CMapStaticBatchObject::Set_InstanceSuppressed(
	const uint64_t placementId,
	const bool_t suppressed)
{
	const auto iter = m_PlacementLookup.find(placementId);
	if (iter == m_PlacementLookup.end() || iter->second >= m_Instances.size())
		return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);

	FMapStaticInstance& instance = m_Instances[iter->second];
	if (instance.Suppressed != suppressed)
	{
		instance.Suppressed = suppressed;
		// Batch bounds stay conservative (they still cover this instance); only
		// the draw and shadow payloads and the cached static shadow change.
		if (m_iStaticShadowRevision != 0u)
			++m_iStaticShadowRevision;
		m_bShadowInstancesDirty = true;
		m_bVisibleInstancesDirty = true;
		m_bFinalCameraPrepared = false;
	}
	return S_OK;
}

HRESULT CMapStaticBatchObject::Set_InstanceCameraPreviewSuppressed(
	const uint64_t placementId,
	const bool_t suppressed)
{
	const auto iter = m_PlacementLookup.find(placementId);
	if (iter == m_PlacementLookup.end() || iter->second >= m_Instances.size())
		return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);

	FMapStaticInstance& instance = m_Instances[iter->second];
	if (instance.CameraPreviewSuppressed != suppressed)
	{
		instance.CameraPreviewSuppressed = suppressed;
		if (m_iStaticShadowRevision != 0u)
			++m_iStaticShadowRevision;
		m_bShadowInstancesDirty = true;
		m_bVisibleInstancesDirty = true;
		m_bFinalCameraPrepared = false;
	}
	return S_OK;
}

HRESULT CMapStaticBatchObject::Ready_Components(
	uint32_t prototypeLevelIndex,
	const std::wstring& modelPrototypeTag)
{
	if (FAILED(__super::Add_Component(
		prototypeLevelIndex,
		TEXT(
			"Prototype_Component_Shader_VtxMeshMapInstance"),
		TEXT("Com_Shader"),
		m_pShaderCom)) ||

		FAILED(__super::Add_Component(
			prototypeLevelIndex,
			modelPrototypeTag,
			TEXT("Com_Model"),
			m_pModelCom)))
	{
		return E_FAIL;
	}

	m_StaticShadowCasterMeshes.clear();
	m_bStaticShadowMaterialInputs = true;
	for (uint32_t mesh = 0u; mesh < m_pModelCom->Get_NumMeshes(); ++mesh)
	{
		const auto* surface = m_pModelCom->Get_MaterialSurface(mesh);
		if (surface && !surface->castsShadow)
			continue;
		m_StaticShadowCasterMeshes.push_back(mesh);
		m_bStaticShadowMaterialInputs &=
			CMapAssetRenderUtils::Uses_StaticShadowInputs(surface, m_RenderProfile, true);
	}
	m_bStaticShadowMaterialInputs &= !m_StaticShadowCasterMeshes.empty();

	// Small or unsimplifiable meshes never consume tight LOD bounds. Keep the
	// ordinary world envelope/culling and profiler draw denominators unchanged.
	m_bHasStaticMeshLod = false;
	for (uint32_t meshIndex = 0u; meshIndex < m_pModelCom->Get_NumMeshes(); ++meshIndex)
	{
		if (m_pModelCom->Has_StaticMeshLod(meshIndex))
		{
			m_bHasStaticMeshLod = true;
			break;
		}
	}
	return S_OK;
}

HRESULT CMapStaticBatchObject::Ensure_InstanceCapacity(
	uint32_t requiredCount)
{
	if (0 == requiredCount)
		return E_INVALIDARG;

	if (requiredCount <= m_iInstanceCapacity &&
		nullptr != m_pInstanceBuffer)
	{
		return S_OK;
	}

	uint32_t newCapacity = 1u;

	while (newCapacity < requiredCount)
		newCapacity <<= 1u;

	const uint64_t byteWidth =
		static_cast<uint64_t>(newCapacity) *
		sizeof(VTXMESHINSTANCE);

	if (byteWidth >
		(std::numeric_limits<uint32_t>::max)())
	{
		return E_OUTOFMEMORY;
	}

	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth =
		static_cast<uint32_t>(byteWidth);
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags =
		D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags =
		D3D11_CPU_ACCESS_WRITE;

	ComPtr<ID3D11Buffer> stagedBuffer;

	if (FAILED(m_pDevice->CreateBuffer(
		&bufferDesc,
		nullptr,
		&stagedBuffer)))
	{
		return E_FAIL;
	}

	m_pInstanceBuffer =
		std::move(stagedBuffer);

	m_iInstanceCapacity =
		newCapacity;

	m_VisibleInstances.reserve(
		newCapacity);
	m_CandidateVisibleInstances.reserve(
		newCapacity);

	return S_OK;
}

HRESULT CMapStaticBatchObject::Ensure_ShadowInstanceCapacity(
	uint32_t requiredCount)
{
	if (0 == requiredCount)
		return E_INVALIDARG;

	if (requiredCount <= m_iShadowInstanceCapacity &&
		nullptr != m_pShadowInstanceBuffer)
	{
		return S_OK;
	}

	uint32_t newCapacity = 1u;
	while (newCapacity < requiredCount)
		newCapacity <<= 1u;

	const uint64_t byteWidth =
		static_cast<uint64_t>(newCapacity) *
		sizeof(VTXMESHINSTANCE);
	if (byteWidth >
		(std::numeric_limits<uint32_t>::max)())
	{
		return E_OUTOFMEMORY;
	}

	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = static_cast<uint32_t>(byteWidth);
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	ComPtr<ID3D11Buffer> stagedBuffer;
	if (FAILED(m_pDevice->CreateBuffer(
		&bufferDesc, nullptr, &stagedBuffer)))
	{
		return E_FAIL;
	}

	m_pShadowInstanceBuffer = std::move(stagedBuffer);
	m_iShadowInstanceCapacity = newCapacity;
	m_ShadowInstances.reserve(newCapacity);
	m_CandidateShadowInstances.reserve(newCapacity);
	return S_OK;
}

HRESULT CMapStaticBatchObject::Upload_VisibleInstances(
	const MAP_CAMERA_CULL_SNAPSHOT* cameraSnapshot)
{
	Engine::CProfiler* const visibilityProfiler = CGameInstance::Get().Get_Profiler();
	Engine::CProfilerWorkScope visibilityWork(visibilityProfiler, Engine::EProfilerWork::MapBatchVisibility);
	const bool_t hasCameraSnapshot = nullptr != cameraSnapshot;
	const uint64_t cameraRevision = hasCameraSnapshot ?
		cameraSnapshot->revision : 0u;
	if (!m_bVisibleInstancesDirty &&
		m_bVisibleInstancesUsedCamera == hasCameraSnapshot &&
		m_iVisibleCameraRevision == cameraRevision)
	{
		if (Engine::CProfiler* profiler =
			CGameInstance::Get().Get_Profiler())
		{
			profiler->Add_Counter(Engine::EProfilerCounter::MapBatchVisibilityCacheHits);
			profiler->Add_Counter(
				Engine::EProfilerCounter::MapVisibleInstances,
				m_VisibleInstances.size());
		}
		return S_OK;
	}

	if (visibilityProfiler) visibilityProfiler->Add_Counter(Engine::EProfilerCounter::MapBatchVisibilityRebuilds);
	Engine::CProfilerDetailScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Map.Batch.Visibility");
	m_CandidateVisibleInstances.clear();
	bool_t requiresNextCameraTick = false;
    INSTANCE_ENVELOPE visibleEnvelope;
    VIEW_LOD_ENVELOPE visibleViewEnvelope(m_bHasStaticMeshLod ? cameraSnapshot : nullptr);
    uint64_t cullingCandidates = 0u;
    if (m_bBatchBoundsDirty) Rebuild_BatchCullBounds();
    bool_t rejectBatch = false;
    if (hasCameraSnapshot && m_bHasBatchBounds && !m_FrustumCulling.bypass)
    {
        MAP_FRUSTUM_CULL_DECISION batchDecision{};
        MAP_FRUSTUM_CULLING_POLICY broadPolicy = m_FrustumCulling;
        broadPolicy.diagnostics = false;
        const float3_t center(m_BatchBounds.x, m_BatchBounds.y, m_BatchBounds.z);
        if (CMapAssetRenderUtils::Evaluate_FrustumVisibility(broadPolicy, *cameraSnapshot,
            m_AssetId, m_AssetGroupId, 0u, center, m_BatchBounds.w, m_BatchFrustumState, batchDecision,
            nullptr, MAP_FRUSTUM_CULL_DETAIL::VISIBILITY_ONLY))
        {
            // Diagnostics retain real placement IDs and the precise loop.
            rejectBatch = !batchDecision.shouldRender && !m_FrustumCulling.diagnostics;
            requiresNextCameraTick = !batchDecision.wouldBeVisible && batchDecision.shouldRender;
        }
    }

	if (rejectBatch && visibilityProfiler)
		visibilityProfiler->Add_Counter(Engine::EProfilerCounter::MapBatchBoundsRejected);
	if (!rejectBatch)
	{
		Engine::CProfilerDetailScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Map.Batch.CullAndPack");
	for (size_t index = 0u; index < m_Instances.size(); ++index)
	{
        FMapStaticInstance& instance = m_Instances[index];
		if (!instance.Visible || instance.Suppressed ||
			instance.CameraPreviewSuppressed)
			continue;

        ++cullingCandidates;
		MAP_FRUSTUM_CULL_DECISION decision{};
		const bool_t evaluated = hasCameraSnapshot &&
			CMapAssetRenderUtils::Evaluate_FrustumVisibility(
				m_FrustumCulling,
				*cameraSnapshot,
				m_AssetId,
				m_AssetGroupId,
				instance.PlacementId,
				instance.WorldBoundsCenter,
				instance.WorldBoundsRadius,
				instance.FrustumState,
				decision, nullptr, MAP_FRUSTUM_CULL_DETAIL::VISIBILITY_ONLY);
		if (evaluated && !decision.wouldBeVisible &&
			decision.shouldRender && !m_FrustumCulling.bypass)
		{
			requiresNextCameraTick = true;
		}
		if (evaluated && !decision.shouldRender)
		{
			continue;
		}

        visibleEnvelope.Add(instance, m_InstanceLinearScaleBounds[index]);
        if (m_bHasStaticMeshLod) visibleViewEnvelope.Add(instance);
		VTXMESHINSTANCE gpuInstance{};
		gpuInstance.World =
			instance.World;
		gpuInstance.WorldInvTranspose =
			instance.WorldInvTranspose;
        gpuInstance.vLightmapScaleBias = instance.BakedLighting.scaleBias;
        gpuInstance.vLightmapAverageScale = instance.BakedLighting.averageScale;
        gpuInstance.vLightmapDirectionalScale = instance.BakedLighting.directionalScale;
        gpuInstance.vStaticShadowScaleBias = instance.BakedLighting.shadowScaleBias;

		m_CandidateVisibleInstances.push_back(
			gpuInstance);
	}
	}

	if (Engine::CProfiler* profiler =
		CGameInstance::Get().Get_Profiler())
	{
        // These are actual recomputation work, separate from cached frame totals.
        profiler->Add_Counter(Engine::EProfilerCounter::MapCullingCandidates, cullingCandidates);
        profiler->Add_Counter(Engine::EProfilerCounter::MapCullingVisible, m_CandidateVisibleInstances.size());
		profiler->Add_Counter(
			Engine::EProfilerCounter::
			MapVisibleInstances,
			m_CandidateVisibleInstances.size());
	}

	// Camera motion can leave the ordered GPU payload unchanged. Preserve the
	// existing buffer in that case instead of discarding it every camera tick.
	const bool_t payloadUnchanged =
		m_CandidateVisibleInstances.size() == m_VisibleInstances.size() &&
		(m_CandidateVisibleInstances.empty() || 0 == std::memcmp(
			m_CandidateVisibleInstances.data(), m_VisibleInstances.data(),
			m_CandidateVisibleInstances.size() * sizeof(VTXMESHINSTANCE)));
    float4_t candidateLodBounds{};
    const bool_t hasLodBounds = visibleEnvelope.Store(candidateLodBounds);
    float4_t candidateTightLodBounds{};
    const bool_t hasTightLodBounds = hasLodBounds && visibleViewEnvelope.Store(candidateTightLodBounds);
	if (payloadUnchanged || m_CandidateVisibleInstances.empty())
	{
        m_VisibleLodBounds = hasLodBounds ? candidateLodBounds : float4_t{};
        m_VisibleTightLodBounds = hasTightLodBounds ? candidateTightLodBounds : float4_t{};
        m_fVisibleLodScale = hasLodBounds ? visibleEnvelope.maximumScale : 0.f;
		if (!payloadUnchanged)
			m_VisibleInstances.swap(m_CandidateVisibleInstances);
		m_bVisibleInstancesDirty = requiresNextCameraTick;
		m_bVisibleInstancesUsedCamera = hasCameraSnapshot;
		m_iVisibleCameraRevision = cameraRevision;
		return S_OK;
	}

	{
		Engine::CProfilerDetailScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Map.Batch.InstanceUpload");
	if (FAILED(Ensure_InstanceCapacity(
		static_cast<uint32_t>(
			m_CandidateVisibleInstances.size()))))
	{
		return E_FAIL;
	}

	D3D11_MAPPED_SUBRESOURCE mapped{};

	if (FAILED(m_pContext->Map(
		m_pInstanceBuffer.Get(),
		0,
		D3D11_MAP_WRITE_DISCARD,
		0,
		&mapped)))
	{
		return E_FAIL;
	}

	std::memcpy(
		mapped.pData,
		m_CandidateVisibleInstances.data(),
		m_CandidateVisibleInstances.size() *
		sizeof(VTXMESHINSTANCE));

	m_pContext->Unmap(
		m_pInstanceBuffer.Get(),
		0);
	if (visibilityProfiler)
		visibilityProfiler->Add_Counter(Engine::EProfilerCounter::MapBatchUploadBytes,
			m_CandidateVisibleInstances.size() * sizeof(VTXMESHINSTANCE));
	}
    m_VisibleLodBounds = hasLodBounds ? candidateLodBounds : float4_t{};
    m_VisibleTightLodBounds = hasTightLodBounds ? candidateTightLodBounds : float4_t{};
    m_fVisibleLodScale = hasLodBounds ? visibleEnvelope.maximumScale : 0.f;
	// Commit only after upload succeeds so a failed Map cannot replace the
	// CPU payload associated with the previous successful GPU upload.
	m_VisibleInstances.swap(m_CandidateVisibleInstances);
	m_bVisibleInstancesDirty = requiresNextCameraTick;
	m_bVisibleInstancesUsedCamera = hasCameraSnapshot;
	m_iVisibleCameraRevision = cameraRevision;

	return S_OK;
}

HRESULT CMapStaticBatchObject::Upload_ShadowInstances()
{
	Engine::CProfilerDetailScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Map.Batch.ShadowPrepare");
	MAP_SHADOW_CULL_SNAPSHOT lightSnapshot{};
	const bool_t hasLightSnapshot =
		CMapAssetRenderUtils::Capture_ShadowCullSnapshot(lightSnapshot);
	const uint64_t lightRevision = hasLightSnapshot ? lightSnapshot.revision : 0u;
	if (!m_bShadowInstancesDirty &&
		m_bShadowInstancesUsedLight == hasLightSnapshot &&
		m_iShadowLightRevision == lightRevision)
		return S_OK;

	m_CandidateShadowInstances.clear();
	for (const FMapStaticInstance& instance : m_Instances)
	{
		if (!instance.Visible || instance.Suppressed ||
			instance.CameraPreviewSuppressed ||
			(hasLightSnapshot && !CMapAssetRenderUtils::Intersects_ShadowCullSnapshot(
				lightSnapshot, instance.WorldBoundsCenter, instance.WorldBoundsRadius)))
			continue;

		VTXMESHINSTANCE gpuInstance{};
		gpuInstance.World = instance.World;
		gpuInstance.WorldInvTranspose = instance.WorldInvTranspose;
		gpuInstance.vLightmapScaleBias = instance.BakedLighting.scaleBias;
		gpuInstance.vLightmapAverageScale = instance.BakedLighting.averageScale;
		gpuInstance.vLightmapDirectionalScale = instance.BakedLighting.directionalScale;
		gpuInstance.vStaticShadowScaleBias = instance.BakedLighting.shadowScaleBias;
		m_CandidateShadowInstances.push_back(gpuInstance);
	}

	const bool_t payloadUnchanged =
		m_CandidateShadowInstances.size() == m_ShadowInstances.size() &&
		(m_CandidateShadowInstances.empty() || 0 == std::memcmp(
			m_CandidateShadowInstances.data(), m_ShadowInstances.data(),
			m_CandidateShadowInstances.size() * sizeof(VTXMESHINSTANCE)));
	if (!payloadUnchanged && !m_CandidateShadowInstances.empty())
	{
		Engine::CProfilerDetailScope uploadScope(CGameInstance::Get().Get_Profiler(), "Map.Batch.ShadowUpload");
		HRESULT result = Ensure_ShadowInstanceCapacity(
			static_cast<uint32_t>(m_CandidateShadowInstances.size()));
		if (FAILED(result)) return result;

		D3D11_MAPPED_SUBRESOURCE mapped{};
		result = m_pContext->Map(m_pShadowInstanceBuffer.Get(), 0,
			D3D11_MAP_WRITE_DISCARD, 0, &mapped);
		if (FAILED(result)) return result;

		std::memcpy(mapped.pData, m_CandidateShadowInstances.data(),
			m_CandidateShadowInstances.size() * sizeof(VTXMESHINSTANCE));
		m_pContext->Unmap(m_pShadowInstanceBuffer.Get(), 0);
	}
	// A failed Map leaves both the successful payload and light revision intact.
	// The next call retries even when only the light, rather than instances, moved.
	if (!payloadUnchanged)
		m_ShadowInstances.swap(m_CandidateShadowInstances);
	m_bShadowInstancesDirty = false;
	m_bShadowInstancesUsedLight = hasLightSnapshot;
	m_iShadowLightRevision = lightRevision;
	return S_OK;
}

HRESULT CMapStaticBatchObject::
Rebuild_PlacementLookup()
{
	m_PlacementLookup.clear();
	m_iAuthoredVisibleInstanceCount = 0u;
	m_PlacementLookup.reserve(
		m_Instances.size());
    m_InstanceLinearScaleBounds.resize(m_Instances.size());

	for (uint32_t index = 0;
		index < m_Instances.size();
		++index)
	{
		const FMapStaticInstance& instance =
			m_Instances[index];

		if (0 == instance.PlacementId ||
			instance.WorldBoundsRadius <= 0.f)
		{
			return E_INVALIDARG;
		}

		const auto [iter, inserted] =
			m_PlacementLookup.emplace(
				instance.PlacementId,
				index);

		UNREFERENCED_PARAMETER(iter);

		if (!inserted)
			return E_INVALIDARG;
        m_InstanceLinearScaleBounds[index] = LinearScaleBound(instance.World);
		if (instance.Visible)
			++m_iAuthoredVisibleInstanceCount;
	}

	return S_OK;
}

unique_ptr<CMapStaticBatchObject>
CMapStaticBatchObject::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto instance =
		unique_ptr<CMapStaticBatchObject>(
			new CMapStaticBatchObject(
				pDevice,
				pContext));

	if (FAILED(
		instance->Initialize_Prototype()))
	{
		return nullptr;
	}

	return instance;
}

shared_ptr<CPrototype>
CMapStaticBatchObject::Clone(void* pArg)
{
	auto instance =
		shared_ptr<CMapStaticBatchObject>(
			new CMapStaticBatchObject(
				*this));

	if (FAILED(instance->Initialize(pArg)))
		return nullptr;

	return instance;
}

void CMapStaticBatchObject::Rebuild_BatchCullBounds()
{
    INSTANCE_ENVELOPE envelope;
    uint32_t grace = 0u;
    for (size_t index = 0u; index < m_Instances.size(); ++index)
    {
        const auto& instance = m_Instances[index];
        if (!instance.Visible) continue;
        envelope.Add(instance, m_InstanceLinearScaleBounds[index]);
        grace = (std::max)(grace, instance.FrustumState.rejectGraceFrames);
    }
    m_bHasBatchBounds = envelope.Store(m_BatchBounds);
    m_BatchFrustumState = {};
    m_BatchFrustumState.rejectGraceFrames = grace;
    m_bBatchBoundsDirty = false;
}

bool_t CMapStaticBatchObject::Build_ScreenLodView(const MAP_CAMERA_CULL_SNAPSHOT& camera,
    Engine::MESH_SCREEN_LOD_DESC& result) const
{
    const auto& p = camera.projection;
    // Other projection conventions retain the original direct draw.
    if (p._14 != 0.f || p._24 != 0.f || p._34 != 1.f || p._44 != 0.f ||
        p._12 != 0.f || p._21 != 0.f || p._13 != 0.f || p._23 != 0.f ||
        p._41 != 0.f || p._42 != 0.f || p._11 <= 0.f || p._22 <= 0.f ||
        p._33 <= 1.f || p._43 >= 0.f || m_fVisibleLodScale <= 0.f || m_VisibleLodBounds.w <= 0.f)
        return false;
    const auto& v = camera.view;
    if (v._14 != 0.f || v._24 != 0.f || v._34 != 0.f || v._44 != 1.f) return false;
    const float viewScale = LinearScaleBound(v);
    if (viewScale <= 0.f) return false;
    const auto viewport = CGameInstance::Get().Get_ViewportSize();
    if (viewport.x <= 0.f || viewport.y <= 0.f) return false;
    const vector_t center = XMVectorSet(m_VisibleLodBounds.x, m_VisibleLodBounds.y, m_VisibleLodBounds.z, 1.f);
    Engine::MESH_SCREEN_LOD_DESC candidate{};
    XMStoreFloat4(&candidate.viewBounds, XMVector3TransformCoord(center, XMLoadFloat4x4(&v)));
    candidate.viewBounds.w = m_VisibleLodBounds.w * viewScale;
    candidate.projectionPixels = { p._11 * viewport.x * .5f, p._22 * viewport.y * .5f };
    candidate.maximumScale = m_fVisibleLodScale * viewScale;
    candidate.nearPlane = -p._43 / p._33;
    if (m_VisibleTightLodBounds.w == 1.f && m_bVisibleInstancesUsedCamera &&
        m_iVisibleCameraRevision == camera.revision)
    {
        candidate.maximumAbsViewXY = { m_VisibleTightLodBounds.x, m_VisibleTightLodBounds.y };
        candidate.minimumViewDepth = m_VisibleTightLodBounds.z;
        candidate.hasTightViewBounds = true;
    }
    const float values[] = { candidate.viewBounds.x, candidate.viewBounds.y, candidate.viewBounds.z,
        candidate.viewBounds.w, candidate.projectionPixels.x, candidate.projectionPixels.y,
        candidate.maximumScale, candidate.nearPlane };
    for (const auto value : values) if (!std::isfinite(value)) return false;
    if (candidate.nearPlane <= 0.f) return false;
    result = candidate;
    return true;
}
```



## G11. JSON 분석 도구 전체 코드

제품 빌드 등록이 없는 Python CLI와 수치 해석 검증이다. 코드 블록은 문서 형식에 맞게 줄바꿈과 행 끝 공백만 정규화한다.

### `Tools/Profiler/analyze_capture.py`

```python
"""Read saved v3 captures without launching the Client or changing their data."""
from __future__ import annotations

import argparse
from collections import defaultdict
import hashlib
import json
import math
from pathlib import Path
import statistics


def distribution(values):
    values = sorted(values)
    if not values:
        return {"samples": 0, "meanMs": None, "p50Ms": None,
                "p95Ms": None, "p99Ms": None, "maxMs": None}
    def percentile(p):
        return values[max(0, math.ceil(p * len(values)) - 1)]
    return {"samples": len(values), "meanMs": statistics.fmean(values),
            "p50Ms": percentile(.50), "p95Ms": percentile(.95),
            "p99Ms": percentile(.99), "maxMs": values[-1]}


def finite_number(value, label, minimum=0):
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise ValueError(f"{label}: expected a number")
    if not math.isfinite(value) or value < minimum:
        raise ValueError(f"{label}: invalid number")
    return value


def interval_union(intervals):
    total = 0.0
    right = None
    for begin, end in sorted(intervals):
        if end < begin:
            raise ValueError("GPU scope ends before it begins")
        total += max(0.0, end - max(begin, right if right is not None else begin))
        right = max(end, right if right is not None else end)
    return total


def analyze(document, first=None, last=None):
    if document.get("schema") != "LostArkProfilerCapture.v3":
        raise ValueError("Only LostArkProfilerCapture.v3 is supported")
    source_frames = document["frames"]
    frames = [f for f in source_frames
              if (first is None or f["frameNumber"] >= first)
              and (last is None or f["frameNumber"] <= last)]
    if not frames:
        raise ValueError("Selected frame range is empty")
    ids = [f["frameNumber"] for f in frames]
    if ids != sorted(set(ids)):
        raise ValueError("Frame numbers must be unique and increasing")
    names = document["scopeNames"]
    tick_rate = finite_number(document["ticksPerSecond"], "ticksPerSecond", 1)
    factor = 1000.0 / tick_rate
    main_thread = document["mainThreadId"]
    partial_count = sum(f.get("droppedCpuScopes", 0) > 0 for f in frames)
    # Inclusive rows are intentionally not summed into a fictitious CPU total.
    cpu = defaultdict(lambda: {"totalMs": 0.0, "calls": 0, "frames": set()})
    gpu = defaultdict(lambda: {"totalMs": 0.0, "calls": 0, "frames": set()})
    work = defaultdict(lambda: {"totalMs": 0.0, "calls": 0, "samples": 0})
    counters = defaultdict(list)
    gpu_unions = []
    valid_gpu = []
    pass_gpu = []
    interval_values, cpu_values, animation_values = [], [], []
    detail_count = 0
    for frame in frames:
        frame_id = frame["frameNumber"]
        interval = finite_number(frame["frameIntervalMs"], "frameIntervalMs")
        if interval > 0:  # reset's first interval has no previous frame boundary
            interval_values.append(interval)
        cpu_values.append(finite_number(frame["cpuFrameMs"], "cpuFrameMs"))
        detail_count += bool(frame.get("detailedCpuScopes", False))
        for scope in frame.get("cpuScopes", []):
            index = scope["nameId"]
            if not isinstance(index, int) or not 0 <= index < len(names):
                raise ValueError("Invalid CPU scope nameId")
            begin = finite_number(scope["beginTick"], "beginTick")
            end = finite_number(scope["endTick"], "endTick")
            if end < begin:
                raise ValueError("CPU scope ends before it begins")
            row = cpu[names[index], scope["threadId"]]
            row["totalMs"] += (end - begin) * factor
            row["calls"] += 1
            row["frames"].add(frame_id)
        for item in frame.get("cpuWork", []):
            row = work[item["name"]]
            row["totalMs"] += finite_number(item["cpuMs"], "cpuWork cpuMs")
            row["calls"] += finite_number(item["calls"], "cpuWork calls")
            row["samples"] += 1
        for name, value in frame.get("counters", {}).items():
            counters[name].append(finite_number(value, f"counter {name}"))
        if "animation" in frame:
            animation_values.append(finite_number(frame["animation"]["cpuMs"], "animation cpuMs"))
        gpu_valid = bool(frame.get("gpuValid", False))
        if "gpuStatus" in frame and gpu_valid != (frame["gpuStatus"] == "valid"):
            raise ValueError("gpuValid and gpuStatus disagree")
        if gpu_valid:
            valid_gpu.append(finite_number(frame["gpuFrameMs"], "gpuFrameMs"))
            if frame.get("gpuScopesSupported", False) and frame.get("droppedGpuScopes", 0) == 0:
                pass_gpu.append(frame)
                ranges = []
                for scope in frame.get("gpuScopes", []):
                    index = scope["nameId"]
                    if not isinstance(index, int) or not 0 <= index < len(names):
                        raise ValueError("Invalid GPU scope nameId")
                    begin = finite_number(scope["beginMs"], "beginMs")
                    end = finite_number(scope["endMs"], "endMs")
                    ranges.append((begin, end))
                    row = gpu[names[index]]
                    row["totalMs"] += finite_number(scope["durationMs"], "GPU durationMs")
                    row["calls"] += 1
                    row["frames"].add(frame_id)
                gpu_unions.append(interval_union(ranges))
    count = len(frames)
    cpu_rows = [{"name": name, "threadId": thread, "mainThread": thread == main_thread,
                 "inclusiveMsPerFrame": row["totalMs"] / count,
                 "callsPerFrame": row["calls"] / count, "observedFrames": len(row["frames"])}
                for (name, thread), row in cpu.items()]
    cpu_rows.sort(key=lambda row: row["inclusiveMsPerFrame"], reverse=True)
    gpu_rows = [{"name": name, "inclusiveMsPerValidFrame": row["totalMs"] / len(pass_gpu),
                 "observedFrames": len(row["frames"])} for name, row in gpu.items()]
    gpu_rows.sort(key=lambda row: row["inclusiveMsPerValidFrame"], reverse=True)
    intervals = distribution(interval_values)
    warnings = [
        "CPU/cpuWork/GPU rows are inclusive and must not be added across parents and children.",
        "GPU timestamps are elapsed intervals, not utilization or pure GPU execution time.",
        "Metadata describes export time; identical metadata does not prove identical historical scenes.",
        "frameIntervalMs in row N spans Begin(N-1) to Begin(N); CPU scopes belong to row N.",
        "Texture counters with no runtime writer are not evidence of zero texture work.",
    ]
    if partial_count:
        warnings.append("Raw CPU scope attribution is incomplete; recorded totals can be lower bounds. Self is not computed.")
    if detail_count:
        warnings.append("Detailed CPU instrumentation was enabled; its overhead is part of these observations.")
    return {
        "schema": "LostArkProfilerAnalysis.v1",
        "metadataAtExport": document.get("metadata", {}),
        "captureWindow": document.get("captureWindow"),
        "selection": {"sourceFrames": len(source_frames), "frames": count,
                      "firstFrame": ids[0], "lastFrame": ids[-1],
                      "contiguous": all(b == a + 1 for a, b in zip(ids, ids[1:]))},
        "frameInterval": intervals,
        "fpsFromMeanInterval": 1000 / intervals["meanMs"] if intervals["meanMs"] else None,
        "cpuFrame": distribution(cpu_values), "gpuFrameValidOnly": distribution(valid_gpu),
        "gpuPassSampleFrames": len(pass_gpu), "gpuScopeUnion": distribution(gpu_unions),
        "recordedAnimation": distribution(animation_values),
        "quality": {"detailedFrames": detail_count, "partialCpuFrames": partial_count,
                    "windowDroppedCpuScopes": sum(f.get("droppedCpuScopes", 0) for f in frames),
                    "invalidOrPendingGpuFrames": count - len(valid_gpu)},
        "cpuScopes": cpu_rows, "gpuScopes": gpu_rows,
        "cpuWork": [{"name": name, "samples": row["samples"],
                     "inclusiveMsPerSampleFrame": row["totalMs"] / row["samples"],
                     "callsPerSampleFrame": row["calls"] / row["samples"]}
                    for name, row in sorted(work.items())],
        "counters": {name: {"samples": len(values), "mean": statistics.fmean(values),
                            "min": min(values), "max": max(values)}
                     for name, values in sorted(counters.items())},
        "slowCpuFrames": [{"frameNumber": f["frameNumber"], "cpuFrameMs": f["cpuFrameMs"],
                           "precedingIntervalMs": f["frameIntervalMs"],
                           "drawCalls": f.get("counters", {}).get("drawCalls"),
                           "droppedCpuScopes": f.get("droppedCpuScopes", 0)}
                          for f in sorted(frames, key=lambda f: f["cpuFrameMs"], reverse=True)[:10]],
        "warnings": warnings,
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("captures", type=Path, nargs="+")
    parser.add_argument("--first-frame", type=int)
    parser.add_argument("--last-frame", type=int)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if args.output.resolve() in {p.resolve() for p in args.captures}:
        parser.error("Output must not replace an input capture")
    if args.output.exists():
        parser.error("Output already exists; choose a new analysis filename")
    reports = []
    try:
        for path in args.captures:
            data = path.read_bytes()
            report = analyze(json.loads(data.decode("utf-8-sig")), args.first_frame, args.last_frame)
            report["input"] = {"path": str(path.resolve()), "sha256": hashlib.sha256(data).hexdigest(),
                               "bytes": len(data)}
            reports.append(report)
        result = {"captures": reports}
        if len(reports) == 2:
            a, b = reports
            keys = ("buildConfiguration", "adapter", "viewport", "d3dDebugLayer", "debuggerAttached",
                    "levelId", "effectiveFpsLimit", "shadowEnabled", "ssaoEnabled", "bloomEnabled", "fxaaEnabled")
            result["comparison"] = {
                "differentExportMetadata": [key for key in keys
                    if a["metadataAtExport"].get(key) != b["metadataAtExport"].get(key)],
                "observedMeanIntervalRatioSecondToFirst":
                    b["frameInterval"]["meanMs"] / a["frameInterval"]["meanMs"]
                    if a["frameInterval"]["meanMs"] and b["frameInterval"]["meanMs"] else None,
                "note": "Observed ratio only. Camera path, warmup, tool windows and capture mode require separate matching; not a causal speedup claim."}
        args.output.parent.mkdir(parents=True, exist_ok=True)
        with args.output.open("x", encoding="utf-8", newline="\n") as stream:
            json.dump(result, stream, ensure_ascii=False, indent=2, allow_nan=False)
            stream.write("\n")
    except (OSError, ValueError, KeyError, TypeError) as error:
        parser.exit(2, f"Capture analysis failed: {error}\n")
    for report in reports:
        print(json.dumps({"input": report["input"]["path"], "frames": report["selection"]["frames"],
                          "meanMs": report["frameInterval"]["meanMs"],
                          "fps": report["fpsFromMeanInterval"], "quality": report["quality"]}, ensure_ascii=False))


if __name__ == "__main__":
    main()
```

### `Tools/Profiler/test_analyze_capture.py`

```python
import copy
import unittest

from analyze_capture import analyze, interval_union


class CaptureAnalysisTests(unittest.TestCase):
    def fixture(self):
        return {"schema": "LostArkProfilerCapture.v3", "ticksPerSecond": 1000,
                "mainThreadId": 7, "scopeNames": ["Client.Render", "Render.Draw"],
                "frames": [
                    {"frameNumber": 10, "frameIntervalMs": 0, "cpuFrameMs": 10,
                     "cpuScopes": [{"nameId": 0, "threadId": 7, "beginTick": 0, "endTick": 10},
                                   {"nameId": 1, "threadId": 7, "beginTick": 2, "endTick": 7}],
                     "gpuValid": True, "gpuScopesSupported": True, "gpuFrameMs": 12,
                     "gpuScopes": [{"nameId": 0, "beginMs": 1, "endMs": 8, "durationMs": 7},
                                   {"nameId": 1, "beginMs": 2, "endMs": 6, "durationMs": 4}]},
                    {"frameNumber": 11, "frameIntervalMs": 20, "cpuFrameMs": 30,
                     "droppedCpuScopes": 9, "detailedCpuScopes": True,
                     "cpuScopes": [], "gpuValid": False, "gpuFrameMs": 0}]}

    def test_pending_reset_and_nested_intervals(self):
        result = analyze(self.fixture())
        self.assertEqual(result["frameInterval"]["samples"], 1)
        self.assertEqual(result["fpsFromMeanInterval"], 50)
        self.assertEqual(result["gpuFrameValidOnly"]["meanMs"], 12)
        self.assertEqual(result["gpuScopeUnion"]["meanMs"], 7)
        self.assertEqual(result["cpuScopes"][0]["inclusiveMsPerFrame"], 5)
        self.assertEqual(result["quality"]["partialCpuFrames"], 1)
        self.assertEqual(result["quality"]["windowDroppedCpuScopes"], 9)
        self.assertNotIn("selfMs", result["cpuScopes"][0])

    def test_selected_window_does_not_include_other_frames(self):
        result = analyze(self.fixture(), 10, 10)
        self.assertIsNone(result["fpsFromMeanInterval"])
        self.assertEqual(result["quality"]["windowDroppedCpuScopes"], 0)
        self.assertEqual(result["cpuScopes"][0]["inclusiveMsPerFrame"], 10)
        self.assertEqual(result["selection"]["sourceFrames"], 2)
        self.assertEqual(result["selection"]["frames"], 1)

    def test_partial_gpu_pass_excluded_from_pass_denominator(self):
        document = self.fixture()
        document["frames"][0]["droppedGpuScopes"] = 1
        result = analyze(document)
        self.assertEqual(result["gpuFrameValidOnly"]["samples"], 1)
        self.assertEqual(result["gpuPassSampleFrames"], 0)
        self.assertEqual(result["gpuScopes"], [])

    def test_invalid_data_rejected(self):
        for field, value in [("cpuFrameMs", float("nan")), ("frameIntervalMs", -1)]:
            document = copy.deepcopy(self.fixture())
            document["frames"][0][field] = value
            with self.assertRaises(ValueError):
                analyze(document)
        document = self.fixture()
        document["frames"][0]["cpuScopes"][0]["endTick"] = -1
        with self.assertRaises(ValueError):
            analyze(document)
        with self.assertRaises(ValueError):
            analyze(self.fixture(), 100)

    def test_overlapping_gpu_scopes_count_elapsed_once(self):
        self.assertEqual(interval_union([(1, 4), (2, 7), (8, 9), (1, 2)]), 7)

    def test_inconsistent_gpu_status_rejected(self):
        document = self.fixture()
        document["frames"][0]["gpuStatus"] = "pending"
        with self.assertRaises(ValueError):
            analyze(document)
        document["frames"][0]["gpuStatus"] = "valid"
        self.assertEqual(analyze(document)["gpuFrameValidOnly"]["samples"], 1)
        document["frames"][1]["gpuStatus"] = "valid"
        with self.assertRaises(ValueError):
            analyze(document)


if __name__ == "__main__":
    unittest.main()
```

## G12. 메모리와 로딩 안정성

2026-10-02 후속 읽기 전용 조사에서 기존 Client의 전용 commit 약15.85GiB와 시스템 commit
약46.44/50.07GiB가 관측됐다. 성능 우선순위에 로딩 메모리 생존성과 잔존량 조사를 추가한다.
이는 과거 종료 원인이 OOM이었다는 확정이 아니다. 이전 G05/G06의 FPS 작업과 별도 완료 조건이다.
현재 메모리 수치는 대응 RESULT G07과 timestamp가 있는 원시 관측 JSON을 따른다.

### G12.1. 용량·시간·수명을 함께 본다

RAM에는 모델/animation/객체/큐/도구 자료가, GPU 메모리에는 texture/VB/IB/render target 등이
존재한다. 같은 자산이라도 저장 파일 크기, CPU 펼침 크기, GPU 자원 크기가 다르다.
Working Set은 지금 RAM에 상주한 pageable 영역이고 Private Bytes는 process 전용 commit이다.
VirtualMemorySize를 RAM 사용량으로 읽거나 RAM·VRAM·공유 GPU 메모리를 단순 합산하지 않는다.

로딩 peak는 다음 단계별 동시 생존량으로 조사한다. WModel decode 중에는 원본 파일 bytes와
생성 중 asset이 겹치고, decoder 반환 뒤 원본 bytes는 해제된다. 이후 asset 준비 중에는 decoded
asset과 변환 정점·CPU picking geometry·GPU 자원·LOD 임시 배열이 겹칠 수 있다.
작업 개수뿐 아니라 각 작업의 크기와 완료 후 staging 보유 시간까지 중요하다.

화면 밖 culling은 렌더 제출을 생략할 뿐 geometry/texture를 해제하지 않는다. animation 호출을
줄여도 clip/key 데이터는 남을 수 있고, LOD는 원본 외 index buffer와 생성 임시 메모리를 추가한다.
사전 계산과 캐시는 CPU 시간을 줄이는 대신 저장 공간과 무효화·해제 수명을 요구한다.
따라서 ms와 MiB/GiB, 로딩 peak와 입장 후 유지량, 재진입 후 잔존량을 함께 비교한다.

### G12.2. 현재 소유 구조에서 확인된 경계

- Loader는 resolved model path별 geometry를 한 번 준비하고 material variant/Clone은 mesh를 공유한다.
  50,021 placement 수를 전체 geometry의 복제 횟수로 계산하지 않는다.
- CMesh는 GPU 생성 뒤에도 CPU picking vertex/index/BVH를 보유한다. CPU 소비자를 조사하기 전에
  GPU 업로드 완료만 근거로 지우지 않는다.
- CMaterial의 same-key texture cache는 weak reference다. cache map 존재를 영구 GPU 소유로 오인하지 않는다.
- MapPlacementRuntime의 process-global load-stage cache는 catalog/records를 복사하고 runtime Clear와
  별개로 남는다. 동일 area는 replace되므로 무한 누수라고 단정하지 않으며 실제 bytes와 owner를 센다.
- AssetPreparationBatch의 공통 실행 callback 상한4는 byte 예산이 아니다. skinned-material 준비에는
  별도의 최대3 worker도 있다. 지금 상태에서 worker 수부터 늘리지 않는다.
- 이전 Level은 전환 중 정리된다. 새 Loading worker 시작과 짧게 겹칠 수 있지만 이전 월드 전체를
  로딩 내내 보유한다고 설명하지 않는다. STATIC prototype·공유 자원·도구 cache는 따로 추적한다.

### G12.3. 다음 구현 전에 확보할 계측과 검증 순서

1. **실패와 압박 구분:** 같은 process의 loading attempt/phase와 allocation 실패 HRESULT·예외,
   device failure·취소 timeout·종료 코드를 연결한다. 낮은 가용 RAM이나 PRE_LEAK event만으로
   누수/OOM 종료를 확정하지 않는다.
2. **작은 주기 샘플:** process PrivateUsage/WorkingSet/peak, system commit/limit/available RAM,
   DXGI local/non-local CurrentUsage/Budget을 100~250ms 수준의 제한된 ring과 phase 경계에서 기록한다.
   Query 실패는 unavailable로 남긴다. GPU adapter/process node 식별과 시간 기준을 포함한다.
   main thread stall 동안에도 OS 샘플을 얻는 경로와 짧은 순간 peak의 누락 가능성을 구분한다.
3. **owner별 수명:** map geometry, texture, animation, picking/BVH, prepared queue, 문서 cache,
   profiler capture를 stable asset ID와 stage 단위로 센다. 추정 payload bytes와 실제 OS 점유를 분리하고,
   shared geometry/SRV를 여러 owner에서 중복 합산하지 않는다. 처음부터 모든 allocation의 stack을
   매 프레임 기록하는 고비용 추적을 기본 모드로 추가하지 않는다.
4. **같은 전환 비교:** Lobby 안정 → 입장 준비 → decode → GPU 준비 → prototype commit →
   level 활성화 → 안정 → 퇴장/재진입의 잔존량을 비교한다. 반복 시 높아지는 것이 의도된 cache인지
   해제되지 않은 owner인지 분리한다. 해제 뒤 allocator가 확보한 heap을 유지할 수도 있음을 고려한다.
5. **예산 있는 준비:** 확인된 큰 임시 복사·불필요 잔존을 먼저 줄이고, 실행 개수 상한에 더해
   in-flight 예상 byte와 완료 후 대기 byte를 제한하는 기존 preparation 경로의 확장을 설계한다.
   작업 하나가 예산보다 큰 경우의 직렬 처리/명확한 실패, 취소·rollback·FIFO·main commit을 보존한다.
6. **다시 프레임 측정:** 메모리 압박과 실패를 줄인 상태에서 같은 장면의 draw/animation/GPU를
   비교한다. page fault/driver residency 대기가 줄어 생긴 CPU 경과 단축을 순수 계산 최적화와 구분한다.

현재 CProfiler v3에는 이 RAM/commit/VRAM timeline이 구현돼 있지 않다. 기존
ClientSessionDiagnostic의 이벤트·stall·60초 heartbeat는 PrivateUsage/WorkingSet/available RAM을
기록하지만 로딩 peak를 보장하지 않는다. MapBatch/ImGui/SceneColor의 Bytes는 주로 해당 프레임
논리 전송량이며 현재 메모리 점유량이 아니다. 이 섹션은 후속 설계이며 이번 턴의 C++ 반영 목록이 아니다.

공식 근거:
[Windows Working Set](https://learn.microsoft.com/en-us/windows/win32/memory/working-set),
[Windows commit와 page file](https://learn.microsoft.com/en-us/troubleshoot/windows-client/performance/introduction-to-the-page-file),
[PrivateUsage 정의](https://learn.microsoft.com/en-us/windows/win32/api/psapi/ns-psapi-process_memory_counters_ex),
[DXGI video memory budget](https://learn.microsoft.com/en-us/windows/win32/api/dxgi1_4/ns-dxgi1_4-dxgi_query_video_memory_info).
[Unreal Memory Insights](https://dev.epicgames.com/documentation/en-us/unreal-engine/memory-insights-in-unreal-engine)는
시간축과 allocation/free 수명·asset별 소유를 조사하고,
[Unity Memory Profiler](https://docs.unity3d.com/Packages/com.unity.memoryprofiler@1.1/manual/index.html)는
메모리 snapshot을 수집해 사용량·보유 상태를 분석한다. 같은 질문을 현재 엔진의 기존 owner와 연결한다.
