#pragma once

#include "imgui.h"
#include <array>
#include <string_view>

// Read-only explanations. Availability describes this renderer, never the
// selected GPU's marketing name. Settings are owned by RenderingBenchmark.
namespace Client::RenderingTechniqueGuide
{
    enum class Availability { Current, MaterialFamily, Foundation, Backend, Reference };
    struct Technique final
    {
        const char* Name;
        const char* Category;
        Availability Status;
        const char* Concept;
        const char* Implementation;
        const char* Variables;
        const char* Cost;
        const char* Experiment;
        const char* Source;
    };

    inline constexpr Technique Techniques[] = {
        {"GPU Gems", "학습·측정", Availability::Reference,
         "GPU Gems는 NVIDIA가 공개한 그래픽 알고리즘 자료 모음입니다. GI나 프레임 생성처럼 켜는 단일 기능이 아닙니다. 각 장의 가정·입력·오차를 현재 renderer에 맞춰 검증합니다.",
         "현재 엔진의 기법과 원문을 대조하는 학습 항목입니다. 같은 주제의 구현이 있다는 사실이 책의 특정 알고리즘을 그대로 구현했다는 뜻은 아닙니다.",
         "AO·그림자 필터·간접광·산란·geometry 처리 중 한 문제를 고르고 해당 알고리즘의 sample 수·반경·해상도를 독립 변수로 선택합니다.",
         "draw/정점 처리, texture sampling, 메모리 대역폭, pass 수 중 어디에 비용이 생기는지 먼저 구분합니다.",
         "같은 카메라·장면·노출에서 baseline과 한 기법을 비교합니다. 화질, GPU ms, CPU 제출 ms를 함께 봅니다.",
         "https://developer.nvidia.com/gpugems"},
        {"PBR / BRDF", "재질·빛", Availability::MaterialFamily,
         "표면의 base color·roughness·metallic·normal로 빛의 확산과 반사를 계산합니다. 재질 모델이며 GI 자체는 아닙니다.",
         "Map source PBR family와 source character 경로가 있습니다. legacy·forward 재질까지 같은 BRDF로 통일된 것은 아닙니다.",
         "기존 Pixel Inputs의 diffuse/specular 기여, normal 배율, roughness offset. 어떤 family가 소비하는지 확인합니다.",
         "G-buffer 대역폭, 직접광 PS 호출과 BRDF texture/ALU, 투명 재질 overdraw.",
         "roughness만 바꾸고 직접광·환경광·노출을 고정합니다. 0 기여가 shader 계산 생략을 의미하지는 않습니다.",
         "https://dev.epicgames.com/documentation/en-us/unreal-engine/physically-based-materials-in-unreal-engine"},
        {"GI / Baked RNM", "간접광·반사", Availability::MaterialFamily,
         "GI는 표면 사이에서 반사된 빛까지 고려하는 문제 영역입니다. baked 조명은 미리 계산해 저장하며 RNM은 방향 정보를 가진 lightmap 표현입니다.",
         "기존 map RNM baked diffuse 소비가 있습니다. 동적 오브젝트·광원 변화 전체를 재계산하는 동적 GI는 아닙니다.",
         "PBR baked diffuse contribution. 베이크 입력·receiver·직접광 중복 여부는 동일하게 유지합니다.",
         "lightmap texture sampling·대역폭과 간접 합성. 오프라인 bake 시간은 프레임 비용과 별개입니다.",
         "RNM contribution 0/1로 화면 기여를 확인합니다. 직접광만 보기와 본질 preset은 서로 다른 실험입니다.",
         "https://dev.epicgames.com/documentation/en-us/unreal-engine/global-illumination-in-unreal-engine"},
        {"IBL / 환경 반사", "간접광·반사", Availability::MaterialFamily,
         "주변 환경에서 오는 빛을 cubemap·probe로 근사합니다. roughness에 따라 반사의 퍼짐을 달리합니다. 환경 반사는 화면 공간 반사와 다릅니다.",
         "source PBR 환경 specular와 cube diffuse 입력이 있습니다. 실시간 모든 geometry 반사는 아닙니다.",
         "environment specular/cube diffuse contribution와 roughness. source 환경 입력과 exposure를 고정합니다.",
         "cubemap sampling, mip 접근, 간접광 합성 PS 비용.",
         "환경 기여를 각각 0/1로 비교하고 거울 표면에서 실제 장면 geometry와 일치하는지도 확인합니다.",
         "https://dev.epicgames.com/documentation/en-us/unreal-engine/reflections-environment-in-unreal-engine"},
        {"SSAO / GTAO", "차폐·그림자", Availability::Current,
         "AO는 주변 geometry가 환경광을 가리는 정도입니다. SSAO는 depth/normal로 화면 안의 근접 차폐를 근사합니다. GTAO는 별도의 차폐 추정 알고리즘입니다.",
         "현재 실행 가능한 것은 기존 SSAO입니다. GTAO라는 이름으로 현재 SSAO를 바꾸어 표시하지 않습니다.",
         "SSAO ON/OFF, sample 4/8/12, radius, bias, intensity, power, distance fade. intensity는 차폐 강도이며 광원 밝기가 아닙니다.",
         "Render.SSAO의 sample 비용·해상도·필터와 간접 합성. 반경이 커질 때 halo·두께 오차도 확인합니다.",
         "먼저 OFF/ON, 다음 sample 4/8/12 또는 radius만 sweep합니다. 원문 AO 알고리즘과 현재 screen-space 근사를 구분합니다.",
         "https://developer.nvidia.com/gpugems/gpugems3/part-ii-light-and-shadows/chapter-12-high-quality-ambient-occlusion"},
        {"Shadow map / PCF / Cache", "차폐·그림자", Availability::Current,
         "광원 시점의 깊이를 저장해 가려짐을 판정합니다. PCF는 여러 비교 sample로 경계를 필터링하며 cache는 변하지 않은 caster 결과를 재사용합니다.",
         "현재 방향광 그림자와 정적 cache·동적 제출을 계측합니다. GPU Gems의 variance shadow map은 별도 알고리즘입니다.",
         "shadow ON/OFF·strength, PCF radius 0/1/2 = 1/9/25 kernel 위치를 비교합니다. 기본3×3을 유지하며 shadow 해상도2048은 이 실험의 조절 변수가 아닙니다.",
         "caster draw/index, cache 재생성, depth texture 대역폭, 수광 PS sampling. dynamic baked 경로는 위치당 두 depth를 읽어2/18/50 fetch입니다.",
         "먼저 캐시 적중/실패와 StaticBuild/Dynamic을 비교합니다. 해상도 감소 이득과 화면 품질 손실을 함께 기록합니다.",
         "https://developer.nvidia.com/gpugems/gpugems/part-ii-lighting-and-shadows/chapter-11-shadow-map-antialiasing"},
        {"Bloom / Tone / Exposure", "영상·재구성", Availability::Current,
         "Bloom은 밝은 영상의 번짐, tone mapping은 HDR을 표시 범위로 바꾸는 곡선, exposure는 입력 빛의 배율입니다. 셋 모두 새 간접광을 계산하지 않습니다.",
         "기존 HDR·half-resolution Bloom·source/Hable tone·gamma·LUT 경로를 사용합니다.",
         "Bloom threshold/knee/intensity/scatter, exposure, gamma, FXAA. 현재 tone 방식과 white point의 유효 여부를 확인합니다.",
         "Render.Bloom의 다운샘플·필터, Render.Final의 tone/색보정·출력 비용.",
         "GI 비교에서는 exposure와 tone을 고정합니다. Bloom 강도 0과 pass OFF가 같은 비용인지 별도로 측정합니다.",
         "https://dev.epicgames.com/documentation/en-us/unreal-engine/post-process-effects-in-unreal-engine"},
        {"FXAA", "영상·재구성", Availability::Current,
         "최종 영상의 경계를 탐지해 공간적으로 부드럽게 만듭니다. 이전 프레임 정보가 없어 시간 흔들림이나 세부 복원은 제한됩니다.",
         "현재 제품의 AA 방식입니다. 세션 실험은 저장된 Mario/scene FXAA 값을 덮어쓰지 않습니다.",
         "ON/OFF, subpixel, edge threshold/min. UI 앞에서 적용하는 현재 순서를 유지합니다.",
         "최종 화면 PS texture sample·대역폭. 작은 디테일의 흐림과 비용을 함께 봅니다.",
         "정지 화면과 카메라 이동 둘 다 비교하되 결과를 별도 run으로 수집합니다.",
         "https://dev.epicgames.com/documentation/en-us/unreal-engine/anti-aliasing-and-upscaling-in-unreal-engine"},
        {"Height Fog / Volumetric Fog", "공기·투명", Availability::Current,
         "height fog는 거리·높이에 따른 감쇠 근사입니다. volumetric fog는 공간 격자에서 빛의 산란과 감쇠를 적분하므로 광선·볼륨 그림자를 표현할 수 있습니다.",
         "현재 실행 경로는 height fog입니다. froxel volume 조명은 미구현입니다.",
         "현재 fog ON/OFF·density·height falloff. 후속 volume에는 grid 크기·step·anisotropy·history가 필요합니다.",
         "현재 합성 PS와, 후속 volume의 light injection·ray integration·3D texture 대역폭을 분리합니다.",
         "안개 제거로 드러나는 geometry/대비 변화와 GPU 이득을 구분합니다. 높이 안개를 volumetric으로 부르지 않습니다.",
         "https://developer.nvidia.com/gpugems/gpugems/part-vi-beyond-triangles/chapter-39-volume-rendering-techniques"},
        {"Instancing / LOD / GPU Driven", "geometry·제출", Availability::MaterialFamily,
         "instancing은 같은 geometry 제출을 묶고 LOD는 필요 정밀도를 낮춥니다. GPU driven은 가시성·제출 인자 일부를 GPU에서 생성합니다.",
         "맵 batch/instance·CPU screen LOD 경로가 있습니다. indirect counter 선언만으로 실행 경로가 있다고 볼 수 없으며 현재 GPU driven culling/indirect 제출은 미구현입니다.",
         "기존 scene의 batch/LOD/컬링 조건. 이번 실험에서 지원하지 않는 설정은 자동 변경하지 않습니다.",
         "CPU CullAndPack/BindAndDraw·instance upload와 GPU IA/VS를 확인합니다. 향후 indirect 실행량에는 별도 GPU 계측이 필요합니다.",
         "같은 카메라에서 index 감소와 draw 감소를 따로 봅니다. index가 줄어도 pixel 병목이면 FPS 이득이 작을 수 있습니다.",
         "https://developer.nvidia.com/gpugems/gpugems2/part-i-geometric-complexity/chapter-3-inside-geometry-instancing"},
        {"실험 SSGI", "간접광·반사", Availability::MaterialFamily,
         "현재 화면의 depth/normal/radiance로 주변 표면에서 반사된 diffuse 빛을 추정합니다. 화면 밖·가려진 표면의 정보가 없습니다.",
         "D3D11 screen-space diffuse bounce 실험입니다. source marker3 MapPBR 수신·FINAL view만 영상 기여하며 기존 RNM/IBL 위에 가산합니다. temporal/history·denoise가 없습니다.",
         "세션 ON/OFF, 강도0..2, 반경0.1..20m, sample4/8/16. 기본OFF이며 저장 profile에 추가하지 않습니다.",
         "Render.SSGI GPU와 Render.ScreenSpaceLighting.Copy를 함께 봅니다. 강도0은 trace 조기 종료지만 full-screen 패스·복사는 남습니다. PSInvocations는 ray hit 수가 아닙니다.",
         "직접광·RNM·IBL·노출을 고정하고 recipe로 단일 변수 A/B/sweep합니다. 카메라 이동/화면 경계·가림·빛 누출을 확인하며 에너지 보존 GI로 간주하지 않습니다.",
         "https://dev.epicgames.com/documentation/en-us/unreal-engine/screen-space-global-illumination"},
        {"실험 SSR / Planar Reflection", "간접광·반사", Availability::MaterialFamily,
         "SSR은 화면 depth를 따라 specular ray를 찾습니다. planar reflection은 반사 카메라로 scene을 다시 그리는 별개 방식입니다.",
         "D3D11 SSR 실험은 source marker3 MapPBR 수신·FINAL view에 가산하며 기존 IBL을 유지합니다. Planar·temporal/history·화면 밖 반사는 미구현입니다.",
         "세션 ON/OFF, 강도0..2, 최대거리0.1..100m, hit두께0.01..2m, step16/32/64. 기본OFF.",
         "Render.SSR GPU + 공통 Copy, depth/radiance 대역폭을 봅니다. 강도0은 trace를 조기 종료하지만 패스/복사는 남고 최대step은 실제hit수와 다릅니다.",
         "동일 roughness·IBL·노출에서 화면 경계/얇은 물체/반사 누락을 비교합니다. SSGI와 같은 원본radiance를 읽어 SSGI 결과를 새 반사입력으로 재사용하지 않습니다.",
         "https://dev.epicgames.com/documentation/en-us/unreal-engine/screen-space-reflections-in-unreal-engine"},
        {"Probe GI / DDGI", "간접광·반사", Availability::Foundation,
         "공간에 배치한 probe의 방향별 조명과 가시성을 갱신하고 표면 위치에서 보간합니다. DDGI는 동적 갱신을 포함하는 접근입니다.",
         "동적 probe volume 미구현. probe 배치·trace backend·visibility 저장·geometry 이동 시 invalidation이 필요합니다.",
         "향후 probe 간격·ray 수·매 프레임 갱신 예산·hysteresis·normal/view bias.",
         "probe trace/update GPU, texture memory·보간. sparse probe의 light leaking과 갱신 지연이 절충입니다.",
         "닫힌 방·문 열림·움직이는 emissive를 같은 시퀀스로 비교하고 수렴 시간을 기록합니다.",
         "https://developer.nvidia.com/rtxgi"},
        {"Lumen", "간접광·반사", Availability::Backend,
         "Unreal의 동적 diffuse GI와 반사 시스템입니다. screen trace와 scene trace, 표면 조명 cache·시간 누적을 결합합니다. GI는 개념이고 Lumen은 그 구현입니다.",
         "이 자체 D3D11 엔진에는 Lumen이 없습니다. software 경로도 distance field/scene 표현이 필요하고 hardware 경로는 ray tracing backend가 필요합니다.",
         "UE에서는 scene detail/view distance, final gather quality, reflection quality/trace mode 등을 조절합니다. 현재 제품 변수로 연결되지 않습니다.",
         "geometry/cache 갱신, diffuse gather, reflection trace, denoise. 이름 하나의 비용으로 취급하지 않습니다.",
         "현재 RNM/IBL과 같은 노출·장면 목표를 정의한 뒤 backend·asset 경로까지 연결해야 실행 비교가 가능합니다.",
         "https://dev.epicgames.com/documentation/en-us/unreal-engine/lumen-technical-details-in-unreal-engine"},
        {"NVIDIA RTX / DXR", "광선 추적", Availability::Backend,
         "ray tracing은 geometry와 광선 교차를 구해 shadow·reflection·GI에 사용하는 방법입니다. RTX는 NVIDIA 기술군이며 그 자체가 하나의 화질 옵션은 아닙니다.",
         "현재 D3D11 renderer에는 DXR 실행 경로가 없습니다. DXR용 D3D12 device/resource·BLAS/TLAS·hit material·동기화가 필요합니다.",
         "향후 rays/pixel, bounce, ray distance, roughness cutoff, AS update budget, denoise quality.",
         "AS build/update, traversal/intersection, hit shader, denoise와 메모리. CPU 제출 감소만으로 이득을 판단하지 않습니다.",
         "shadow/reflection/GI 중 하나씩 raster 대체안과 비교하며 skinned geometry 갱신 비용도 포함합니다.",
         "https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html"},
        {"Path Tracing", "광선 추적", Availability::Backend,
         "여러 번 반사되는 광선 경로를 표본화해 빛 전달을 계산합니다. 적은 sample에는 noise가 있고 누적 sample로 수렴합니다.",
         "미구현. ray tracing backend와 일관된 material/lighting 평가, accumulation·denoise가 필요합니다.",
         "향후 samples/pixel, max bounce, clamp, denoise, accumulation reset.",
         "ray 수와 bounce·hit material에 따른 GPU 비용. 정지 누적 품질과 실시간 움직임은 다른 목표입니다.",
         "참조 화질로 사용할 때 같은 tone/exposure를 유지합니다. 누적 FPS를 실제 게임 입력 FPS로 표시하지 않습니다.",
         "https://dev.epicgames.com/documentation/en-us/unreal-engine/path-tracer-in-unreal-engine"},
        {"TAA / TAAU / TSR", "영상·재구성", Availability::Foundation,
         "이전 프레임을 현재 픽셀에 재투영해 aliasing을 줄입니다. temporal upscaling은 낮은 내부 해상도에서 더 높은 출력 영상을 복원합니다. TSR은 UE 구현입니다.",
         "현재 FXAA만 있으며 temporal 재구성은 미구현. jitter·motion vectors·history와 camera cut/resize reset이 필요합니다.",
         "향후 내부 해상도·history weight·clamp·sharpen·reactive mask. 새 object/skinning velocity도 연결해야 합니다.",
         "velocity pass·history memory·reprojection/filter. 낮은 내부 해상도의 이득과 복원 비용을 함께 잽니다.",
         "움직이는 머리카락·투명 이펙트·가림 해제·카메라 순간이동에서 잔상과 안정성을 비교합니다.",
         "https://dev.epicgames.com/documentation/en-us/unreal-engine/temporal-upscalers-in-unreal-engine"},
        {"DLSS / FSR / XeSS", "영상·재구성", Availability::Backend,
         "vendor별 업스케일링 기술군입니다. 각 세대·모드의 입력과 GPU/API 지원이 다릅니다. 지원 GPU가 있다는 것만으로 자동 적용되지 않습니다.",
         "SDK 통합·지원 확인·depth/motion/exposure/history 계약이 미구현입니다. 현재 실험 해상도 변경을 DLSS로 표시하지 않습니다.",
         "향후 quality mode·render scale·sharpen, motion vector scale, jitter/history reset.",
         "낮은 내부 해상도 이득, 재구성 pass와 추가 입력 생성 비용. vendor와 GPU별 결과를 분리합니다.",
         "같은 출력 해상도에서 native AA와 비교합니다. 정지 선명도만으로 움직임 품질을 평가하지 않습니다.",
         "https://developer.nvidia.com/rtx/dlss"},
        {"DLSS Ray Reconstruction", "광선 추적", Availability::Backend,
         "ray tracing의 드문 조명 표본을 AI 기반으로 재구성하는 기술입니다. ray를 추적하는 기능이나 일반 GI의 다른 이름이 아닙니다.",
         "ray-traced 입력·SDK·필수 보조 buffer가 없어 미구현입니다.",
         "향후 지원 mode·재구성 입력·ray sample budget. SDK의 정확한 입력 계약을 따라야 합니다.",
         "재구성 GPU 비용과 대체하는 denoiser 비용, 필요한 buffer 대역폭.",
         "같은 ray budget에서 기존 denoiser와 비교하고 세부 복원·잔상·지연을 함께 평가합니다.",
         "https://developer.nvidia.com/rtx/dlss"},
        {"Frame Generation", "영상·재구성", Availability::Backend,
         "렌더된 프레임 사이에 표시할 영상을 생성합니다. 게임 simulation·입력 갱신 횟수가 같은 비율로 늘지는 않습니다. GPU Gems와 다른 용어입니다.",
         "미구현. SDK 지원·motion/depth·HUD 처리·present/scheduling·지연 관리가 필요합니다.",
         "향후 지원 mode와 지연 정책. rendered FPS, displayed FPS, simulation Hz를 각각 기록합니다.",
         "생성 pass·VRAM·출력 지연. base rendering이 느린 원인을 숨기는 해결책으로 쓰지 않습니다.",
         "Profiler 원본 frame time과 별도의 표시율을 함께 측정하고 입력 지연도 검증합니다.",
         "https://developer.nvidia.com/blog/how-to-successfully-integrate-dlss-3/"},
        {"Forward+ / Clustered / MegaLights", "재질·빛", Availability::Foundation,
         "tile/cluster별 관련 광원 목록은 불필요한 광원 평가를 줄입니다. MegaLights는 UE의 stochastic 직접광 접근이며 clustered와 동일한 알고리즘은 아닙니다.",
         "현재 CPU light culling·batch/receiver 경로가 있습니다. cluster light list와 MegaLights는 미구현입니다.",
         "향후 tile/Z slice·list 용량·광원 샘플 수·shadow 정책. overflow 시 광원을 조용히 버리지 않습니다.",
         "list build compute·upload, light PS 평가, stochastic noise와 denoise. CPU와 GPU 비용이 이동할 수 있습니다.",
         "같은 화면 coverage·광원 수·수광체에서 비교하고 목록 overflow와 fallback 비용도 표시합니다.",
         "https://dev.epicgames.com/documentation/en-us/unreal-engine/megalights-in-unreal-engine"},
        {"Nanite / Virtual Geometry", "geometry·제출", Availability::Backend,
         "geometry를 cluster로 나누어 가시성·정밀도·streaming을 함께 관리합니다. Nanite는 Unreal의 virtualized geometry 구현입니다.",
         "현재 CModel/CMesh·LOD를 사용합니다. cluster hierarchy·streaming·raster 경로가 없어 Nanite 미구현입니다.",
         "향후 screen error·cluster/page budget·streaming cache. source 변환 pipeline과 runtime을 함께 바꿔야 합니다.",
         "cluster culling/raster, page 요청·메모리, material resolve. 단순 polygon 수만으로 예상하지 않습니다.",
         "현재 LOD/instancing과 같은 화면 오차 목표에서 비교하고 근접/빠른 이동의 streaming 변화를 확인합니다.",
         "https://dev.epicgames.com/documentation/en-us/unreal-engine/nanite-virtualized-geometry-in-unreal-engine"},
        {"Virtual Shadow Maps / CSM", "차폐·그림자", Availability::Foundation,
         "CSM은 거리별 shadow 영역을 나눕니다. VSM은 필요한 고해상도 shadow page를 관리합니다. VSM은 variance shadow map의 약자로도 쓰이므로 구분합니다.",
         "현재 단일 shadow/cache 경로와 별개입니다. UE Virtual Shadow Maps와 cascade/page 관리 미구현입니다.",
         "향후 cascade count/split, page budget, resolution bias, cache invalidation, filter sample.",
         "caster 재제출, page allocation/render, shadow sampling, 움직이는 geometry의 cache 실패.",
         "멀리 있는 작은 그림자와 근접 품질을 동시에 비교하고 cache 적중률을 기록합니다.",
         "https://dev.epicgames.com/documentation/en-us/unreal-engine/virtual-shadow-maps-in-unreal-engine"},
        {"Volumetric Scattering / Clouds", "공기·투명", Availability::Foundation,
         "공간 안의 입자가 빛을 산란·흡수하는 양을 적분합니다. GPU Gems의 화면 후처리 광선 근사와 3D volume ray marching은 서로 다른 입력·한계를 가집니다.",
         "height fog·기존 sprite effect를 volume 구름으로 간주하지 않습니다. 전용 volume grid와 lighting 미구현입니다.",
         "향후 step·resolution·density·anisotropy·max distance·shadow/temporal quality.",
         "ray integration·light sampling·3D texture·history. pixel 수와 step 수가 함께 중요합니다.",
         "가림·카메라 방향 전환·광원 이동에서 화면 근사의 누락과 3D 방법의 비용을 비교합니다.",
         "https://developer.nvidia.com/gpugems/gpugems3/part-ii-light-and-shadows/chapter-13-volumetric-light-scattering-post-process"},
        {"Transparency / OIT", "공기·투명", Availability::Foundation,
         "반투명 표면은 순서·혼합·여러 겹의 overdraw가 중요합니다. OIT는 정렬 의존을 완화하는 방법군이며 정확성·메모리 비용이 방식마다 다릅니다.",
         "현재 Blend/Effect 경로가 있습니다. 범용 OIT accumulation·resolve는 미구현입니다.",
         "향후 weighted blend 가중치·layer capacity·resolution. alpha 의미와 source blend 계약을 보존합니다.",
         "PS overdraw·blend bandwidth, 추가 render targets와 resolve pass.",
         "같은 particle 수와 겹침에서 비교합니다. 객체 수가 같아도 화면을 덮는 면적이 다르면 비용이 달라집니다.",
         "https://dev.epicgames.com/documentation/en-us/unreal-engine/using-transparency-in-unreal-engine-materials"},
        {"Substrate / SSS / Hair", "재질·빛", Availability::Foundation,
         "layered 재질은 코팅·기저층의 빛을 조합하고 SSS는 표면 아래 산란을, hair 모델은 섬유의 방향성 반사를 다룹니다. Substrate는 UE 재질 체계입니다.",
         "현재 source character/material family의 표현이 있습니다. 범용 Substrate·새 SSS/hair renderer는 미구현입니다.",
         "향후 layer 수·lobe·산란 반경·roughness/aniso·sample budget. 기존 carrier 복원과 별도 변경입니다.",
         "재질 평가 ALU·texture, G-buffer 확장·SSS filter·투명 hair overdraw.",
         "동일 조명에서 얼굴·머리카락을 비교하고 재질별 지원과 원본 source carrier를 구분합니다.",
         "https://dev.epicgames.com/documentation/en-us/unreal-engine/substrate-materials-in-unreal-engine"},
        {"DOF / Motion Blur", "영상·재구성", Availability::Foundation,
         "DOF는 초점 거리에서 벗어난 흐림, motion blur는 노출 시간 동안 움직임의 적분 근사입니다. aliasing이나 느린 simulation을 해결하지 않습니다.",
         "새 범용 depth/velocity 기반 DOF·motion blur pass는 미구현입니다. Effect의 화면 연출과 구분합니다.",
         "향후 focus/aperture·CoC clamp, shutter angle·velocity clamp·sample 수.",
         "gather/scatter filter와 depth/velocity 입력, 큰 blur 반경의 texture 비용.",
         "정지/빠른 이동을 분리하고 HUD와 초상 view가 잘못 섞이지 않는지 확인합니다.",
         "https://dev.epicgames.com/documentation/en-us/unreal-engine/post-process-effects-in-unreal-engine"},
        {"VRS / Dynamic Resolution", "영상·재구성", Availability::Backend,
         "VRS는 영역별 shading 빈도를, dynamic resolution은 frame budget에 따라 내부 pixel 수를 바꿉니다. 출력 해상도와 내부 해상도를 구분해야 합니다.",
         "D3D12 VRS 경로·frame-budget 제어 미구현입니다. 창 크기 변경을 동적 해상도 구현으로 부르지 않습니다.",
         "향후 shading rate/tile, render scale min/max·target ms·응답 속도.",
         "줄어드는 PS 작업과 rate image/재구성 비용. geometry/CPU 병목에는 효과가 제한될 수 있습니다.",
         "같은 출력 크기에서 실제 내부 크기와 화질을 기록합니다. 자동 해상도 변화는 고정조건 A/B에서 잠급니다.",
         "https://learn.microsoft.com/en-us/windows/win32/direct3d12/vrs"},
        {"Low-resolution Particles", "공기·투명", Availability::Foundation,
         "넓은 연기·안개 파티클을 낮은 해상도에 그린 뒤 depth 경계를 보존하며 합성해 fill cost를 줄입니다. geometry 수를 줄이는 방법과 다릅니다.",
         "별도 저해상도 particle RT·depth downsample·경계 합성은 미구현입니다. 기존 source blend와 distortion은 분리해야 합니다.",
         "향후 render scale·depth edge threshold·soft intersection·대상 effect family.",
         "PS overdraw 절감과 추가 RT·합성 비용. 작은 날카로운 이펙트는 품질이 나빠질 수 있습니다.",
         "쿠크 2관문·빙고와 느린 장면의 Blend PS/면적을 먼저 비교하고 같은 파티클에서 실험합니다.",
         "https://developer.nvidia.com/gpugems/gpugems3/part-iv-image-effects/chapter-23-high-speed-screen-particles"},
        {"Refraction / Distortion", "공기·투명", Availability::MaterialFamily,
         "앞서 그린 scene color를 굴절·왜곡 vector로 다시 읽어 투명 표면 뒤 영상을 근사합니다. 실제 ray로 찾는 투과와는 차이가 있습니다.",
         "현재 source effect의 scene color snapshot·distortion 소비가 있습니다. 공통 실험변수로 연결되지 않은 effect 문서는 자동 수정하지 않습니다.",
         "기존 effect 저작의 distortion/mask. 향후 세션 실험에는 명시적 field whitelist와 copy 해상도 계약이 필요합니다.",
         "scene color copy 대역폭, PS fetch, 겹침과 합성. snapshot이 불필요하게 반복되는지도 확인합니다.",
         "같은 effect occurrence에서 왜곡과 copy 수를 비교하고 전경 누출·화면 가장자리 오차를 확인합니다.",
         "https://developer.nvidia.com/gpugems/gpugems2/part-ii-shading-lighting-and-shadows/chapter-19-generic-refraction-simulation"},
        {"Virtual Texturing / Streaming", "geometry·제출", Availability::Foundation,
         "필요한 texture page를 작은 물리 cache에 공급해 큰 자료를 다룹니다. texture 압축·mipmap과는 다른 page 관리 체계입니다.",
         "현재 일반 texture 경로와 구분되는 page table·feedback·streaming residency는 미구현입니다.",
         "향후 page/cache 크기·mip bias·upload budget·prefetch.",
         "feedback/readback·I/O·upload·page table lookup·VRAM. 부족한 page의 품질 변화와 stutter가 중요합니다.",
         "정지 상태 평균뿐 아니라 카메라 이동 중 p95/p99와 업로드를 측정하고 실제 메모리 계측을 추가합니다.",
         "https://dev.epicgames.com/documentation/en-us/unreal-engine/virtual-texturing-in-unreal-engine"},
        {"Displacement / Geometry Clipmaps", "geometry·제출", Availability::Foundation,
         "displacement는 실제 정점을 이동합니다. geometry clipmap은 카메라 주변의 여러 heightfield 격자로 지형 정밀도와 갱신을 관리합니다.",
         "기존 WModel 지형을 즉시 교체하는 기능은 없습니다. heightfield·bounds·LOD·collision 계약까지 필요합니다.",
         "향후 grid 크기·level 수·screen error·height scale·update 거리.",
         "정점/도형 생성, texture sampling·streaming, bounds 갱신·그림자 caster 증가.",
         "같은 실루엣 오차에서 기존 mesh LOD와 비교하고 지형 seam·그림자·collision 차이를 별도 확인합니다.",
         "https://developer.nvidia.com/gpugems/gpugems2/part-i-geometric-complexity/chapter-2-terrain-rendering-using-gpu-based-geometry"}
    };

    inline const char* StatusLabel(Availability status)
    {
        switch (status)
        {
        case Availability::Current: return "현재 경로 실험";
        case Availability::MaterialFamily: return "일부 재질·경로 지원";
        case Availability::Foundation: return "추가 패스·입력 필요";
        case Availability::Backend: return "기반·SDK 통합 필요";
        default: return "원리·자료";
        }
    }

    inline void Render()
    {
        if (!ImGui::CollapsingHeader("기법 사전 · GPU Gems / GI / RTX")) return;
        ImGui::TextWrapped("개념과 현재 실행 가능한 범위를 함께 표시합니다. 위 세션 실험에서 지원하는 변수만 실제 적용됩니다. 향후 변수는 설계 설명이며 이 목록이 기능을 켜거나 저장하지 않습니다.");
        ImGui::TextWrapped("현재 기법의 실제 비교는 위 원인별 recipe에서 한 변수로 준비합니다. 아래 상용 도구 비교표는 시간축·작업 대기·자원/메모리·coverage의 계측 공백과 검증 조건을 설명합니다.");
        static ImGuiTextFilter filter;
        filter.Draw("기법 검색");
        static int selected = 0;
        if (ImGui::BeginCombo("기법", Techniques[selected].Name))
        {
            for (int i = 0; i < static_cast<int>(std::size(Techniques)); ++i)
            {
                const auto& item = Techniques[i];
                if (!filter.PassFilter(item.Name) && !filter.PassFilter(item.Category)) continue;
                ImGui::PushID(i);
                if (ImGui::Selectable(item.Name, selected == i)) selected = i;
                ImGui::PopID();
            }
            ImGui::EndCombo();
        }
        const auto& item = Techniques[selected];
        ImGui::Text("%s | %s", item.Category, StatusLabel(item.Status));
        const auto field = [](const char* title, const char* content)
        {
            ImGui::SeparatorText(title);
            ImGui::TextWrapped("%s", content);
        };
        field("개념", item.Concept);
        field("현재 적용 범위", item.Implementation);
        field("조절 변수와 단위", item.Variables);
        field("어디에 비용이 생기는가", item.Cost);
        field("A/B에서 확인할 것", item.Experiment);
        ImGui::SeparatorText("공식 자료");
        ImGui::TextWrapped("%s", item.Source);
        if (ImGui::Button("공식 자료 주소 복사")) ImGui::SetClipboardText(item.Source);
    }
}
