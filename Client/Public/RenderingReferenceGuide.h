#pragma once

#include "imgui.h"
#include <iterator>
#include <string_view>

// Shared read-only comparison with commercial rendering/profiling concepts.
// Workbench may consume explicit stable recipe requests; the Profiler stays read-only.
namespace Client::RenderingReferenceGuide
{
    enum class Support { Partial, Missing };
    struct Entry final
    {
        const char* Id; const char* Name; const char* Category; Support Status;
        const char* Concept; const char* Current; const char* Gap; const char* RequiredInputs;
        const char* MetricMeaning; const char* Boundary; const char* Validation;
        const char* Experiment; const char* Source;
    };
    inline constexpr Entry Entries[] = {
        {"frame-budget", "프레임 예산·CPU/GPU 병목", "시간", Support::Partial,
         "Unreal의 프레임 분석은 게임·렌더 제출·GPU와 프레임 간격을 나누어 느린 경로를 찾습니다.",
         "CPU 처리, 이전 CPU+프레임 틈, GPU 원래 frame, P50/P95/P99와 F7 프레임·기준 비교가 있습니다.",
         "Game/Render/RHI 전용 스레드 분해와 display latency 인과 추적은 없습니다.",
         "완료 frame ID, CPU QPC, GPU timestamp/disjoint, Present 정책, foreground·FPS cap.",
         "ms는 시간입니다. CPU와 GPU는 겹치므로 합하지 않으며 1000/관측 interval이 관측 FPS입니다.",
         "GPU pending·disjoint·누락은 0ms가 아닙니다. CPU Present 시간만으로 GPU 실행·디스플레이 대기를 구분할 수 없습니다.",
         "frame ID/유효 분모를 유지하고 같은 조건의 p99·평균을 비교합니다.",
         "F7 프레임 변화 → CPU/GPU 예산 → WB 단일 변수 AB/BA.",
         "https://dev.epicgames.com/documentation/en-us/unreal-engine/performance-and-profiling-overview"},
        {"timeline", "Timing Insights·타임라인", "시간", Support::Partial,
         "스레드별 CPU/GPU 사건을 시간축에서 확대하고 범위·호출 관계를 조사합니다.",
         "기존 CPU begin/end·thread·depth와 GPU frame 상대 timestamp를 사용한 범위 계측·표·타임라인을 제공합니다.",
         "CPU/GPU 공통 절대시계 보정, queue 간 동시성, 장기간 trace stream은 없습니다.",
         "clock domain, frame ID, thread ID, begin/end, parent/depth, 누락 수.",
         "막대 폭은 관측 구간의 경과 시간이며 worker 막대 합은 frame latency가 아닙니다.",
         "CPU 축과 GPU 상대축의 가로 위치로 둘의 동기 관계를 추정하지 않습니다. 빈 공간도 원인 미상입니다.",
         "중첩·동시 worker·frame 밖 종료·누락의 표본에서 포함/자체 시간이 중복 합산되지 않는지 확인합니다.",
         "F7 CPU/GPU 표와 타임라인에서 후보를 찾은 뒤 같은 scope의 A/B를 비교합니다.",
         "https://dev.epicgames.com/documentation/unreal-engine/using-the-timing-panel-in-unreal-insights-for-unreal-engine"},
        {"tasks", "Task Graph·대기·스케줄링", "시간", Support::Missing,
         "태스크 생성·준비·스케줄·시작·종료와 선행 의존성을 연결해 실행 대기 원인을 구분합니다.",
         "일부 worker/Loader 범위와 긴 작업 기록만 관측합니다.",
         "task ID/부모/의존 edge, runnable→running 지연, OS context switch·lock wait reason이 없습니다.",
         "task lifecycle·dependency, 스레드 전환, 동기화 객체·wait begin/end.",
         "실행 시간, queue 대기, 동기화 대기는 서로 다른 관측값입니다.",
         "scope 안의 wall time을 순수 연산량이나 lock 대기 시간으로 표시하지 않습니다.",
         "합성 task dependency와 알려진 wait를 넣어 실제 critical path가 재구성되는지 검증해야 합니다.",
         "현재는 worker가 겹치는지 관찰만 가능하며 wait 최적화 효과를 단정하지 않습니다.",
         "https://dev.epicgames.com/documentation/unreal-engine/task-graph-insights-in-unreal-engine-5?lang=en-US"},
        {"cpu-symbols", "CPU 호출 관계·표본 프로파일링", "시간", Support::Partial,
         "계측 이벤트와 통계적 callstack 표본은 함수·호출자별 비용을 서로 다른 방식으로 설명합니다.",
         "등록 scope의 이름·thread·depth·시간과 자체 비용을 비교합니다.",
         "모든 함수 자동 추적·instruction pointer 표본·심볼 resolve·전체 call tree는 없습니다.",
         "이벤트 scope 또는 sampling profiler, 심볼/PDB, 모듈 주소·스레드 ID.",
         "inclusive는 자식 포함, self는 관측한 직접 자식 제외입니다.",
         "계측되지 않은 함수는 분리되지 않으며 누락·잘못된 계층의 self는 미관측입니다.",
         "부모/자식/형제/다른 thread를 분리하고 누락 시 N/A 처리합니다.",
         "World.Render만 크면 하위 scope와 미분류 CPU를 확인합니다.",
         "https://dev.epicgames.com/documentation/unreal-engine/timing-insights-in-unreal-engine"},
        {"rhi", "Render/RHI 제출·명령 기록", "제출·리소스", Support::Partial,
         "UE RenderTrace는 RHI command list, 변환 작업, GPU 제출과 fence를 연결해 제출 경로를 분석합니다.",
         "DX11 immediate context의 등록 렌더 scope와 Engine draw/바인딩/업로드 계수가 있습니다.",
         "UE의 Render/RHI thread 모델, command list ID, 제출 batch/fence 인과 연결은 없습니다.",
         "명령·list·submit ID, render thread 소유, CPU 제출 및 GPU 실행 경계.",
         "CPU draw 제출 횟수와 GPU 실행 시간은 다른 축입니다.",
         "World.Render ms는 GPU 완료 시간이 아니며 draw 수가 같아도 shader·대역폭 비용은 다릅니다.",
         "같은 메시 수에서 batch/instance/바인딩 변화와 CPU 제출 ms를 함께 검증합니다.",
         "F7 draw·instance·index 및 Render.* CPU/GPU 차이를 함께 봅니다.",
         "https://dev.epicgames.com/documentation/unreal-engine/unreal-insights-reference-in-unreal-engine-5"},
        {"draw-trace", "개별 draw·메시 제출 이력", "제출·리소스", Support::Partial,
         "draw 사건을 메시·재질·패스·리소스 상태와 연결하면 과도한 제출의 위치를 찾을 수 있습니다.",
         "상세 ON에서 프레임당 최대512개의 CMesh 제출 이름·정점·인덱스·instance·material slot·패스 이름을 기록합니다.",
         "stable asset/instance ID, 셰이더 permutation·바인딩 SRV/RT/UAV와 모든 외부 draw의 연결은 없습니다.",
         "draw sequence ID, mesh/asset ID, 선택 LOD/index range, material/program, 바인딩 상태.",
         "mesh index는 제출량이며 가시 삼각형·고유 asset 개수와 다릅니다. 같은 CMesh는 여러 패스에 반복 제출될 수 있습니다.",
         "표본 누락 이후의 draw는 상세 목록에 없습니다. aggregate와 개별 표본 범위를 구분합니다.",
         "두 CModel이 공유한 CMesh·다중 패스·instancing·512초과에서 totals와 누락을 확인합니다.",
         "F7 선택 프레임 메시 draw와 scope draw/index를 함께 비교합니다.",
         "https://dev.epicgames.com/documentation/en-us/unreal-engine/using-renderdoc-with-unreal-engine"},
        {"draw-gpu", "개별 draw GPU 비용", "제출·리소스", Support::Missing,
         "GPU Visualizer는 렌더 이벤트 구간을, 그래픽 캡처 도구는 draw·리소스 상태를 조사하는 데 사용됩니다.",
         "GPU pass timestamp와 일부 pipeline statistics, CPU 제출 draw trace가 있습니다.",
         "개별 draw timestamp·shader occupancy·cache miss·메모리 stall 계측은 없습니다.",
         "bounded query 예산, draw ID와 command 순서, 원래 frame 귀속, vendor counter 도구.",
         "pass GPU ms는 해당 구간의 전체 시간이며 draw 수로 나눈 값은 개별 draw 측정값이 아닙니다.",
         "동시 실행·캐시·query overhead 때문에 패스 평균을 메시별 GPU ms로 배분하지 않습니다.",
         "단일 draw 격리 기준과 외부 GPU 도구 결과로 오차·오버헤드를 확인해야 합니다.",
         "현재는 비싼 pass를 찾은 후 그 pass의 제출 목록을 조사합니다.",
         "https://dev.epicgames.com/documentation/en-us/unreal-engine/gpu-profiling-in-unreal-engine"},
        {"rdg", "RDG·패스 의존 그래프", "제출·리소스", Support::Missing,
         "RDG는 패스의 읽기/쓰기 의존성으로 실행 순서·자원 수명·불필요 패스 제거를 관리합니다.",
         "수동 DX11 렌더 순서와 패스 계측이 있습니다.",
         "resource edge, producer/consumer, transient lifetime·alias·unused pass culling graph가 없습니다.",
         "typed pass/resource ID, subresource access, lifetime, graph compiler와 실행 검증.",
         "그래프 edge는 데이터 의존성이며 시간 막대의 포함 관계와 다릅니다.",
         "scope tree를 RDG나 자동 자원 최적화가 구현된 것으로 표시하지 않습니다.",
         "읽기 전 미기록·수명 외 사용·alias overlap을 오류로 잡고 원본 출력 parity를 확인해야 합니다.",
         "현재 pass self·draw를 이용해 확장 우선순위를 정하는 단계입니다.",
         "https://dev.epicgames.com/documentation/en-us/unreal-engine/render-dependency-graph-in-unreal-engine"},
        {"hazards", "리소스 hazard·barrier", "제출·리소스", Support::Missing,
         "읽기/쓰기 충돌과 subresource 상태 전환을 추적해야 GPU stall과 잘못된 자원 재사용을 설명할 수 있습니다.",
         "DX11 API·드라이버 상태 관리와 일부 Map/업로드/복사 scope를 사용합니다.",
         "앱이 소유한 resource hazard graph·barrier timeline·SRV/RT/UAV 충돌 이력이 없습니다.",
         "resource/subresource ID, 읽기·쓰기 접근, bind/unbind, fence 또는 backend 상태 전환.",
         "업로드 바이트는 트래픽 일부이며 실제 DRAM bandwidth나 stall 원인이 아닙니다.",
         "DX11 드라이버의 내부 barrier·cache flush를 현재 counter로 역산할 수 없습니다.",
         "의도적 alias/동시 bind 오류를 별도 debug validation과 대조해야 합니다.",
         "Map/Copy가 큰 frame에서 GPU pass와 CPU 대기 후보만 분리합니다.",
         "https://dev.epicgames.com/documentation/en-us/unreal-engine/render-dependency-graph-in-unreal-engine"},
        {"gpu-queues", "GPU queue·async compute", "제출·리소스", Support::Missing,
         "독립 작업을 graphics/compute/copy queue에 배치하고 fence로 의존성을 보장하는 구조입니다.",
         "DX11 immediate-context timestamp 구간을 기록합니다.",
         "독립 queue timeline, overlap, async compute dispatch scheduler가 없습니다.",
         "현대 backend, queue/submit/fence ID, 교차 clock calibration, 자원 접근 선언.",
         "여러 queue의 ms 합은 frame 완료 시간과 다릅니다.",
         "현재 GPU scope 중첩은 queue 병렬 실행 증거가 아닙니다.",
         "serial 기준과 async 기준의 출력·critical path·fence 대기를 함께 검증해야 합니다.",
         "지원 기능 toggle 없이 구조 도입 항목으로만 표시합니다.",
         "https://dev.epicgames.com/documentation/en-us/unreal-engine/render-dependency-graph-in-unreal-engine"},
        {"memory", "Memory Insights·할당 수명", "메모리·에셋", Support::Partial,
         "할당·재할당·해제 사건과 callstack을 재구성하면 특정 시점의 살아 있는 할당과 누수를 조사할 수 있습니다.",
         "Capture ON 동안1Hz로 process private commit/working set·process lifetime peak와 system commit/limit/available RAM을 표본화합니다. 실패 영역은 N/A입니다.",
         "allocation ID/크기/owner/tag/callstack/free 연결과 특정 자원 누수의 증명이 없습니다.",
         "allocator hook, 안정 ID, alloc/free 시각, symbol, LLM 유사 tag, 누락 정책.",
         "프로세스 private/working set은 개별 게임 자원의 실제 소유 바이트 합과 다릅니다.",
         "메모리 증가만으로 leak이라고 결론내리지 않습니다. 1Hz 사이의 순간 peak나 main-thread stall 구간은 놓칠 수 있으며 allocation leak 추적을 대체하지 않습니다.",
         "반복 생성/해제에서 ledger balance와 retained allocation을 확인해야 합니다.",
         "현재 수치 변화는 원인 조사 출발점이며 F7 시간·로더 범위와 함께 봅니다.",
         "https://dev.epicgames.com/documentation/en-us/unreal-engine/memory-insights-in-unreal-engine"},
        {"vram", "VRAM 예산·residency·자원 목록", "메모리·에셋", Support::Partial,
         "GPU 메모리는 할당 크기, 프로세스 예산, 상주 여부와 압박을 나누어 조사합니다.",
         "IDXGIAdapter3 node0 local/nonlocal의 process CurrentUsage/Budget을1Hz로 표본화하고 sample frame/tick/age·adapter LUID를 함께 보관합니다.",
         "texture/buffer/RT별 실제 allocation, residency·eviction·paging 사건과 매핑은 없습니다.",
         "resource 생성/파괴 ledger, desc/mips/format/MSAA, adapter budget/residency 정보.",
         "예산/사용량은 byte 단위입니다. Local은 UMA에서 전용 VRAM과 같지 않고 budget은 동적 OS 예산입니다. RAM·commit·GPU 메모리를 합하지 않습니다.",
         "너비×높이×format 계산은 추정 payload입니다. 압축·mip·heap alignment·driver reserve와 다릅니다.",
         "작은 알려진 texture/RT 생성·해제와 ledger/API값 차이를 설명해야 합니다.",
         "현재는 메모리 압박 후보와 texture/RT 로드 시점만 교차 조사합니다.",
         "https://dev.epicgames.com/documentation/en-us/unreal-engine/memory-insights-in-unreal-engine"},
        {"streaming", "Asset/Texture Streaming", "메모리·에셋", Support::Partial,
         "필요 mip·상주 mip·IO·decode·upload와 요청 우선순위를 연결해야 팝인과 로딩 hitch를 구분합니다.",
         "Loader/Model/Texture CPU scope, CMaterial::LoadSharedTexture의 requests/path-cache hits/새 SRV 생성 3counter가 연결됩니다.",
         "requested/resident mip, asset별 IO/read/decode/upload 단계·budget admission·우선순위 trace가 없습니다.",
         "asset ID, 요청/완료 시각, IO byte, decode시간, GPU upload·residency·목표 mip.",
         "로딩 wall time과 디스크 IO시간, 업로드 bytes와 VRAM 상주량은 각각 다릅니다.",
         "3counter는 CMaterial 공유 로드 경로 한정이며 전 엔진 texture 수가 아닙니다. content hash hit/GPU bytes는 N/A입니다. warm/cold cache를 구분합니다.",
         "같은 asset cold/warm 로드와 취소·실패에서 단계별 합계와 누락을 검증해야 합니다.",
         "F7 Loader 긴 작업과 해당 frame p99를 연결해 원인 후보를 찾습니다.",
         "https://dev.epicgames.com/documentation/en-us/unreal-engine/reporting-texture-streaming-metrics-in-unreal-engine"},
        {"shader", "Shader/PSO·재질 복잡도", "화면·장면", Support::Partial,
         "Material Analyzer와 shader 통계는 재질의 instruction·sample·permutation 부담을 조사합니다.",
         "재질 family/debug view·PBR 기여 실험과 일부 Shader 준비 CPU scope가 있습니다.",
         "material별 compiled instruction/texture sample 통계, permutation·PSO cache hit/miss 계측이 없습니다.",
         "material/program/permutation ID, compiled shader 통계, GPU pass 및 pipeline cache 사건.",
         "instruction 수는 정적 복잡도 지표이며 실제 GPU ms·FLOPs가 아닙니다.",
         "기여값 0이 shader 분기 제거·texture fetch 생략을 뜻하지 않습니다.",
         "같은 표면·program에서 기여 변화와 GPU ms가 독립적일 수 있음을 확인합니다.",
         "WB PBR 직접/간접/IBL 기여 recipe → Render.Combined·PS 호출과 화면 비교.",
         "https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-material-analyzer-tool"},
        {"overdraw", "Shader Complexity·Quad Overdraw", "화면·장면", Support::Missing,
         "화면에서 겹쳐 그려진 픽셀·quad와 shader 복잡도 시각화로 채움 비용 후보를 찾습니다.",
         "PS invocations와 투명/불투명 pass ms만 관측합니다.",
         "픽셀/quad별 coverage heatmap, depth-test 탈락·helper lane·재질 비용 시각화가 없습니다.",
         "전용 instrumentation shader/target, 해상도, blend/depth 상태, frame별 coverage.",
         "PSInvocations는 픽셀 shader 실행 수이며 unique pixel·정확한 광원 평가·quad overdraw 횟수가 아닙니다.",
         "PS 호출/화면픽셀 비율은 광범위한 대리지표이지 overdraw 정답이 아닙니다.",
         "겹친 quad 수가 알려진 장면과 depth on/off를 이용해 heatmap을 검증해야 합니다.",
         "투명 이펙트 frame에서 Render.Blend ms·PS·화면 면적을 함께 기록합니다.",
         "https://dev.epicgames.com/documentation/en-us/unreal-engine/viewport-modes-in-unreal-engine"},
        {"culling", "Scene proxy·가시성·LOD", "화면·장면", Support::Partial,
         "상용 엔진은 장면 primitive 표현과 view별 가시성·occlusion·LOD 선택을 별도 단계로 관리합니다.",
         "기존 CModel·맵 batch의 CPU bounds/culling·LOD·instancing 계수와 선택 index가 있습니다.",
         "UE scene proxy/GPU Scene·HZB occlusion·Nanite cluster streaming은 없습니다.",
         "stable placement/primitive ID, view ID, bounds, cull reason, LOD, 제출과 결과 연결.",
         "후보·통과·제출은 서로 다른 모집단입니다. 고유 CMesh는 가시 object 수가 아닙니다.",
         "CPU 컬링 성공만으로 최종 GPU 가시성이 입증되지 않습니다.",
         "카메라 경계·가림·공유메시·다중뷰에서 instance별 제외 이유와 실제 제출을 대조해야 합니다.",
         "F7 MapCulling/LOD/index 및 CPU Map.Batch.Visibility를 같은 frame에서 비교합니다.",
         "https://dev.epicgames.com/documentation/en-us/unreal-engine/visibility-and-occlusion-culling-in-unreal-engine"},
        {"light", "Light Complexity·직접광 범위", "화면·장면", Support::Partial,
         "광원 중첩·수광체·화면 coverage를 나누면 개수보다 실제 shading 범위의 영향을 볼 수 있습니다.",
         "World/Character receiver scope, local-light 후보·컬링·record/draw/upload와 일부 pipeline 수치를 기록합니다.",
         "광원별 GPU ms, pixel-light 평가 수, clustered tile/list overflow, light coverage heatmap은 없습니다.",
         "light ID/type/receiver, culled/submitted record, affected pixel/tile, pass GPU·PS.",
         "light records는 receiver pass별 중복 제출량입니다. PS 호출×광원 수는 정확한 연산량이 아닙니다.",
         "전체 light pass 시간에서 RNM/IBL·재질 합성 비용을 임의 분배하지 않습니다.",
         "동일 화면 coverage에서 광원 하나씩 추가·receiver 분리·컬링 경계를 확인해야 합니다.",
         "WB PBR direct diffuse/specular recipe는 화면 기여 진단이며 광원 제거 벤치는 아닙니다.",
         "https://dev.epicgames.com/documentation/en-us/unreal-engine/viewport-modes-in-unreal-engine"},
        {"shadow", "Shadow map·PCF·cache", "화면·장면", Support::Partial,
         "그림자는 caster 생성·정적 cache 갱신과 receiver의 깊이 비교 필터 비용을 나누어 분석합니다.",
         "방향광 static/dynamic/cache scope와 실제 PCF radius0/1/2, caster draw/index 계측이 있습니다.",
         "VSM page pool/요청/cache invalidation 지도, ray shadow BVH·denoiser는 없습니다.",
         "광원 projection, caster 변화, shadow resolution, kernel, cache hit/miss, receiver coverage.",
         "PCF는1/9/25 kernel 위치이고 dynamic baked 경로는 위치당 두 깊이를 읽습니다.",
         "PCF만 바꿔 caster draw가 줄 것으로 예상하지 않습니다. shadow OFF는 조명 품질 자체도 바꿉니다.",
         "기본3×3 parity와 동일 caster/coverage에서 sample sweep GPU·화질을 비교합니다.",
         "WB shadow ON/OFF·PCF·strength recipe, Render.Shadow/Render.Lights/Render.Combined.",
         "https://dev.epicgames.com/documentation/en-us/unreal-engine/virtual-shadow-maps-in-unreal-engine"},
        {"gi", "GI·Lumen·반사", "화면·장면", Support::Partial,
         "GI는 간접광 문제이며 Lumen은 scene 표현·추적·cache·필터를 묶은 UE의 특정 구현입니다.",
         "map family RNM·IBL과 source marker3 MapPBR에 가산하는 공간 SSGI/SSR 실험 패스·sample/step 세션변수가 있습니다.",
         "화면 밖/가려진 정보·temporal/denoise·distance field/surface cache·Lumen·hardware ray tracing은 없습니다.",
         "geometry/material scene representation, motion/depth/normal, tracing data, history·denoise·cache.",
         "baked/IBL contribution은 합성 비율입니다. SSGI sample/SSR step은 상한 입력이며 실제 ray hit/교차 수는 미계측입니다.",
         "Lumen·RTX 명칭을 현재 기여 slider의 실제 기능으로 표시하지 않습니다.",
         "직접광·baked·환경 반사를 분리하고 source family별 소비와 화면 밖 정보를 검증해야 합니다.",
         "WB baked/IBL·SSGI/SSR recipe, Render.SSGI/SSR + 공통 Copy, exposure 고정.",
         "https://dev.epicgames.com/documentation/en-us/unreal-engine/lumen-technical-details-in-unreal-engine"},
        {"temporal", "Temporal AA·upscale·history", "화면·장면", Support::Missing,
         "TAA/TSR은 motion·depth·jitter와 과거 frame을 재투영해 복원하고 disocclusion을 처리합니다.",
         "현재 FXAA와 display 후처리만 사용합니다.",
         "velocity buffer, history ownership/reset/rejection, TAA/TSR·vendor upscaler·frame generation이 없습니다.",
         "motion vectors, depth, jitter, exposure, history, camera cut·reactive mask 계약.",
         "내부 해상도·출력 해상도·렌더 FPS·표시 FPS를 따로 기록해야 합니다.",
         "현재 warm-up은 cache 안정화 정책일 뿐 미구현 temporal history reset 검증이 아닙니다.",
         "정지/이동/가림 해제/camera cut에서 ghosting·detail·latency와 GPU ms를 검증해야 합니다.",
         "WB FXAA ON/OFF·blend recipe가 현재 가능한 AA 비교입니다.",
         "https://dev.epicgames.com/documentation/en-us/unreal-engine/temporal-upscalers-in-unreal-engine"},
        {"fog-transparency", "안개·volume·투명도", "화면·장면", Support::Partial,
         "volume은 공간 매질을 적분하고 투명도는 여러 표면의 순서·혼합을 처리합니다.",
         "height/exponential fog와 기존 alpha blend pass가 있습니다.",
         "froxel 조명·volumetric cloud raymarch·OIT layer 저장/resolve·volume history는 없습니다.",
         "depth, medium density, light/shadow, voxel 또는 raymarch budget, alpha ordering.",
         "fog density는 매질 입력이며 raymarch step 수가 아닙니다. blend draw 수만으로 화면 coverage를 알 수 없습니다.",
         "현재 fog slider를 UE volumetric quality로 해석하지 않습니다.",
         "같은 depth/노출에서 fog ON/OFF·density 화질을 보고 pass 비용과 분리합니다.",
         "WB fog enabled/density recipe → Render.Combined·Render.Blend·PS를 비교합니다.",
         "https://dev.epicgames.com/documentation/en-us/unreal-engine/volumetric-fog-in-unreal-engine"},
        {"post", "Bloom·tone·노출·색변환", "화면·장면", Support::Partial,
         "후처리는 HDR 영상 필터와 표시 변환을 수행하며 장면의 새 간접광을 계산하지 않습니다.",
         "half-res Bloom, source/Hable tone, gamma·LUT·FXAA가 있습니다.",
         "DOF/모션블러/auto-exposure histogram·현대 temporal reconstruction은 없습니다.",
         "HDR input, depth/velocity(기법에 따라), 노출·tone·color-space와 출력 해상도.",
         "Bloom intensity·노출은 영상 배율입니다. intensity0과 pass OFF의 비용은 같다고 가정하지 않습니다.",
         "GI·재질 비교에서는 exposure와 gamma를 고정합니다.",
         "ON/OFF는 pass 비용, 강도 sweep은 화면 기여를 중심으로 비교합니다.",
         "WB Bloom ON/OFF·intensity, FXAA, exposure/gamma recipe.",
         "https://dev.epicgames.com/documentation/en-us/unreal-engine/post-process-effects-in-unreal-engine"},
        {"hardware", "하드웨어 counter·대역폭·FLOPs", "검증", Support::Missing,
         "GPU 하드웨어 counter는 장치별 occupancy·cache·메모리·실행 유닛 상태를 분석합니다.",
         "D3D11 pipeline invocation과 timestamp를 수집합니다.",
         "vendor별 GPU cycles/occupancy/cache hit·bandwidth/energy counter와 calibration이 없습니다.",
         "지원 GPU/tool API, counter multiplexing·sampling 조건, clock/power state.",
         "호출 수는 실행 횟수, byte counter는 계측 경로의 논리 byte입니다. 실제 DRAM 대역폭·FLOPs는 미관측입니다.",
         "PSInvocations를 빛 계산 횟수·texture fetch·연산 수로 바꾸지 않습니다.",
         "외부 도구와 동일 workload·clock 조건에서 카운터 정의와 sampling overhead를 검증해야 합니다.",
         "현재 pass GPU ms와 제출량으로 후보를 좁힌 뒤 외부 GPU 분석이 필요합니다.",
         "https://dev.epicgames.com/documentation/en-us/unreal-engine/gpu-profiling-in-unreal-engine"},
        {"reproducibility", "실험 재현성·계측 신뢰도", "검증", Support::Partial,
         "프로파일링 결과는 같은 실행 조건·입력·표본 범위와 계측 overhead가 있어야 비교할 수 있습니다.",
         "세션 A/B·AB/BA·단일 field sweep, warm-up, 조건 fingerprint·p99·pending drain·JSON이 있습니다.",
         "결정적 gameplay/animation replay, 장기 trace·자동 화질 점수·통계적 인과 보장은 없습니다.",
         "build/device/driver·해상도·camera·scene·cache·clock·cap·계측 상세 상태와 experiment ID.",
         "Δms는 관측 차이입니다. 한 번의 미세 차이를 최적화 효과로 확정하지 않습니다.",
         "입력 안정은 인과 입증이 아닙니다. 여러 값 preset은 다변수 진단으로 남깁니다.",
         "AB/BA 반복 분산보다 큰 일관된 차이와 p99·화질을 함께 확인합니다.",
         "WB 원인별 recipe로 한 변수만 선택하고 실패 원인·변경 필드와 함께 결과를 저장합니다.",
         "https://dev.epicgames.com/documentation/en-us/unreal-engine/performance-and-profiling-overview"},
    };
    inline const char* StatusLabel(Support value)
    { return value == Support::Partial ? "일부 관측 / 계약 차이 있음" : "현재 미구현"; }

    inline const char* RecipeFor(std::string_view id)
    {
        if (id == "light") return "pbr.directDiffuse";
        if (id == "shadow") return "shadow.pass";
        if (id == "gi") return "pbr.sourceIndirect";
        if (id == "fog-transparency") return "fog.pass";
        if (id == "post") return "display.sourcePostProcess";
        return nullptr;
    }

    inline const char* Render(bool allowExperiments = false, bool experimentActive = false, bool capturing = false)
    {
        ImGui::PushID("CommercialRenderingReference");
        if (!ImGui::CollapsingHeader("상용 도구와 비교 / 현재 관측·구조적 부족점")) { ImGui::PopID(); return nullptr; }
        const char* requestedRecipe = nullptr;
        ImGui::TextWrapped("Epic의 공개 문서를 기능군별로 비교합니다. UE의 모든 cvar 목록이나 동일 성능 인증이 아닙니다. 행 선택은 설정을 바꾸지 않으며 실제 구현된 실험만 명시적으로 준비할 수 있습니다.");
        static ImGuiTextFilter filter;
        filter.Draw("주제·부족점 검색");
        static int selected=0;
        if (ImGui::BeginTable("CoverageMatrix",3,ImGuiTableFlags_RowBg|ImGuiTableFlags_BordersInnerH|ImGuiTableFlags_Resizable|ImGuiTableFlags_ScrollY,ImVec2(0,240)))
        {
            ImGui::TableSetupColumn("비교 기능"); ImGui::TableSetupColumn("현재 범위"); ImGui::TableSetupColumn("핵심 부족점"); ImGui::TableHeadersRow();
            for (int i=0;i<static_cast<int>(std::size(Entries));++i)
            {
                const auto& e=Entries[i];
                if (!filter.PassFilter(e.Name)&&!filter.PassFilter(e.Category)&&!filter.PassFilter(e.Gap)) continue;
                ImGui::PushID(i);ImGui::TableNextRow();ImGui::TableSetColumnIndex(0);
                if(ImGui::Selectable(e.Name,selected==i))selected=i;
                ImGui::TableSetColumnIndex(1);ImGui::TextWrapped("%s",StatusLabel(e.Status));
                ImGui::TableSetColumnIndex(2);ImGui::TextWrapped("%s",e.Gap);ImGui::PopID();
            }
            ImGui::EndTable();
        }
        const auto& e=Entries[selected];ImGui::Text("%s / %s",e.Category,e.Name);
        if (const char* recipe=RecipeFor(e.Id))
        {
            ImGui::Text("현재 엔진의 연결 실험: %s (상용 기법 전체 구현을 뜻하지 않음)",recipe);
            if (allowExperiments)
            {
                ImGui::BeginDisabled(capturing);
                if (ImGui::Button(experimentActive ? "연결된 실험으로 기존 B 대체" : "현재 A 보관 + 연결된 B 준비")) requestedRecipe=recipe;
                ImGui::EndDisabled();
                ImGui::TextWrapped("위 세션의 A 적용 / B 적용 / 실험 종료를 사용합니다. 저장 설정은 유지합니다.");
            }
            else ImGui::TextDisabled("실제 A/B 준비는 Rendering Workbench에서 실행하세요.");
        }
        const auto field=[](const char* title,const char* value){ImGui::SeparatorText(title);ImGui::TextWrapped("%s",value);};
        field("상용 개념",e.Concept);field("현재 구현",e.Current);field("실제 부족점",e.Gap);
        field("구현에 필요한 입력",e.RequiredInputs);field("수치의 의미",e.MetricMeaning);
        field("현재 계측으로 결론낼 수 없는 것",e.Boundary);field("완료 판정 기준",e.Validation);
        field("지금 연결해서 볼 측정·실험",e.Experiment);field("공식 근거",e.Source);
        if(ImGui::Button("공식 문서 주소 복사"))ImGui::SetClipboardText(e.Source);
        ImGui::PopID();
        return requestedRecipe;
    }
}
