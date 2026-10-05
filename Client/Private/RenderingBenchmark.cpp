#include <WinSock2.h>
#include "imgui.h"
#pragma push_macro("new")
#undef new
#include "Engine_RenderTypes.h"
#pragma pop_macro("new")

#include "RenderingBenchmark.h"
#include "RenderingProfileService.h"
#include "GameInstance.h"
#include "Level_KakulSaydonArena.h"
#include "Presentation_Manager.h"
#include "MapAssetRenderUtils.h"
#include "RenderingTechniqueGuide.h"
#include "RenderingReferenceGuide.h"
#include "UserSettingsDocument.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <numeric>
#include <sstream>
#include <cmath>
#include <locale>
#include <cstdio>
#include <dxgi.h>

namespace
{
    using ExperimentField=Client::RENDERING_EXPERIMENT_FIELD;
    enum class RecipeKind { Workload, Filter, Contribution, Display };
    struct FExperimentRecipe final
    {
        const char* Id; const char* Name; ExperimentField Field;
        ExperimentField RequiredEnabled; bool SourceRequired; RecipeKind Kind;
        double Low, High; int Steps;
        const char* Goal; const char* Metrics; const char* Boundary;
    };
    constexpr auto NoGate=ExperimentField::COUNT;
    constexpr const char* QualitySuiteRowId = "suite.dx11-quality-expansion";
    constexpr FExperimentRecipe ExperimentRecipes[] = {
        {"texture.mip","표면 텍스처 mip 0/1/2/3",ExperimentField::TEXTURE_MIN_MIP,NoGate,false,RecipeKind::Filter,0,3,4,
         "최상 해상도부터 허용할 표면 mip을 비교합니다.","표면 디테일·Render.World GPU ms, 같은 카메라·거리", "실제 보유 mip만 사용합니다. UI·lookup·native Effect sampler는 보존하며 VRAM 상주량을 줄이지 않습니다."},
        {"ao.horizon","기존 SSAO / Horizon AO",ExperimentField::HORIZON_AO_ENABLED,ExperimentField::SSAO_ENABLED,false,RecipeKind::Workload,0,1,2,
         "동일한 depth/normal 수신면에서 가림 horizon 탐색과 기존 SSAO를 비교합니다.","Render.SSAO GPU ms, 접촉 음영·평면·얇은 형상의 halo", "현재 SSAO 수신면과 blur를 공유하는 자체 horizon 근사입니다. GTAO나 HBAO+ SDK가 아닙니다."},
        {"ssr.refinement","SSR 교차 구간 세부 탐색",ExperimentField::SSR_REFINEMENT_ENABLED,ExperimentField::SSR_ENABLED,true,RecipeKind::Workload,0,1,2,
         "거친 step 사이의 반사 hit를 추가 탐색하는 효과를 비교합니다.","Render.SSR GPU ms, 같은 반사면의 누락·잘못된 교차", "화면의 단일 depth 교차 보정입니다. 가려진 면·화면 밖 정보는 복원하지 않습니다."},
        {"ssr.roughnessFilter","SSR roughness 반사 필터",ExperimentField::SSR_ROUGHNESS_FILTER_ENABLED,ExperimentField::SSR_ENABLED,true,RecipeKind::Filter,0,1,2,
         "거칠기에 따른 hit 주변 radiance 필터와 반사 디테일을 비교합니다.","Render.SSR GPU ms, 거친 표면·depth/normal 경계의 반사 번짐", "depth/normal 경계로 제한한 화면 필터이며 다중 반사 ray·시간 누적·환경 convolution은 아닙니다."},
        {"ssao.pass","SSAO 패스 ON/OFF",ExperimentField::SSAO_ENABLED,NoGate,false,RecipeKind::Workload,0,1,2,
         "화면 공간 AO 패스와 최종 차폐 기여가 필요한지 비교합니다.","Render.SSAO GPU/self, Render.Combined, GPU frame P99, PSInvocations", "간접광 전체 OFF가 아닙니다. depth/normal·카메라·해상도·노출을 고정합니다."},
        {"ssao.samples","SSAO 샘플 수 4/8/12",ExperimentField::SSAO_SAMPLES,ExperimentField::SSAO_ENABLED,false,RecipeKind::Workload,4,12,3,
         "실제 sample loop 수와 AO 품질/시간의 관계를 조사합니다.","Render.SSAO GPU ms/P99, GPU frame, PSInvocations, halo/noise", "샘플 수를 줄여도 PS 호출 수는 같을 수 있습니다. ms 개선률을 sample 비율로 단정하지 않습니다."},
        {"ssao.radius","SSAO 반경",ExperimentField::SSAO_RADIUS,ExperimentField::SSAO_ENABLED,false,RecipeKind::Filter,.25,2,5,
         "차폐 범위와 얇은 표면 halo를 비교합니다. bias보다 크고 fade 이하인 값만 준비합니다.","Render.SSAO GPU ms, 동일 화면의 접촉 차폐·halo", "반경은 거리 입력이며 sample count가 아닙니다. 실행 loop 수가 같아도 결과가 달라집니다."},
        {"shadow.pass","방향광 그림자 ON/OFF",ExperimentField::SHADOW_ENABLED,NoGate,false,RecipeKind::Workload,0,1,2,
         "caster 생성과 receiver shadow 비교를 합친 기능의 총 기여를 조사합니다.","Render.Shadow StaticBuild/Dynamic/CacheCopy, Render.Lights/Combined, draw/index, cache hit/miss", "그림자 해상도는 고정입니다. cache warm/cold·caster·light pose를 동일하게 유지합니다."},
        {"shadow.pcf","PCF kernel 1/9/25",ExperimentField::PCF_RADIUS,ExperimentField::SHADOW_ENABLED,false,RecipeKind::Workload,0,2,3,
         "receiver 깊이 필터 sample 수와 경계 품질을 비교합니다.","Render.Lights/Render.Combined GPU, PSInvocations, Render.Shadow draw/index", "radius0/1/2는1/9/25위치이며 dynamic baked는위치당2depth입니다. caster draw 감소 실험이 아닙니다."},
        {"shadow.strength","그림자 합성 강도",ExperimentField::SHADOW_STRENGTH,ExperimentField::SHADOW_ENABLED,false,RecipeKind::Contribution,0,1,5,
         "그림자의 영상 기여를 분리합니다.","같은 caster 조건의 화면·GPU ms·draw/index", "강도0은 shadow pass OFF가 아니며 연산 제거를 보장하지 않습니다."},
        {"bloom.pass","Bloom 패스 ON/OFF",ExperimentField::BLOOM_ENABLED,NoGate,false,RecipeKind::Workload,0,1,2,
         "half-resolution Bloom 필터 전체의 시간과 영상 효과를 비교합니다.","Render.Bloom GPU/self, Render.Final, GPU P99, PSInvocations", "Bloom은 GI가 아닙니다. 노출·threshold·HDR 장면을 고정합니다."},
        {"bloom.intensity","Bloom 강도 (패스 유지)",ExperimentField::BLOOM_INTENSITY,ExperimentField::BLOOM_ENABLED,false,RecipeKind::Contribution,0,2,5,
         "Bloom을 계속 실행하면서 번짐의 기여만 확인합니다.","Render.Bloom/Final GPU, 밝은 경계·클리핑", "강도0과 패스 OFF의 비용 차이는 다른 실험으로 구분합니다."},
        {"bloom.threshold","Bloom 임계값",ExperimentField::BLOOM_THRESHOLD,ExperimentField::BLOOM_ENABLED,false,RecipeKind::Filter,.5,2,5,
         "어떤 HDR 밝기가 번짐으로 들어가는지 비교합니다.","Render.Bloom GPU, HDR/highlight 화면", "threshold는 밝기 입력입니다. 노출 변경을 함께 적용하면 단일 원인 비교가 아닙니다."},
        {"fxaa.pass","FXAA ON/OFF",ExperimentField::FXAA_ENABLED,NoGate,false,RecipeKind::Workload,0,1,2,
         "현재 공간 AA의 화면 효과와 최종 패스 비용을 비교합니다.","Render.Final GPU/PS, 가는 선·원거리 가장자리", "Mario의 저장된 OFF를 바꾸지 않습니다. 이 버튼은 명시적 세션 실험이며 TAA/TSR이 아닙니다."},
        {"fxaa.blend","FXAA subpixel blend",ExperimentField::FXAA_BLEND,ExperimentField::FXAA_ENABLED,false,RecipeKind::Filter,0,1,5,
         "subpixel 필터 기여와 선명도의 균형을 비교합니다.","Render.Final GPU ms, 가장자리·텍스트 선명도", "MSAA sample 수나 내부 render resolution을 조절하지 않습니다."},
        {"pbr.directDiffuse","PBR 직접 diffuse 기여",ExperimentField::PBR_DIFFUSE,NoGate,true,RecipeKind::Contribution,0,1,2,
         "지원 map PBR 표면의 직접 확산광 성분을 분리합니다.","Render.Lights/Combined GPU, light records/draw, PSInvocations와 화면", "광원 제거가 아닙니다. scale0이어도 광원 제출·BRDF 연산이 유지될 수 있습니다."},
        {"pbr.directSpecular","PBR 직접 specular 기여",ExperimentField::PBR_SPECULAR,NoGate,true,RecipeKind::Contribution,0,1,2,
         "지원 map PBR의 직접 반사 성분을 분리합니다.","Render.Lights/Combined GPU, roughness 고정 화면", "환경 반사 기여는 그대로이며 source character의 별도 경로까지 바뀌지 않습니다."},
        {"pbr.baked","MapPBR 간접 diffuse 기여",ExperimentField::PBR_BAKED,NoGate,true,RecipeKind::Contribution,0,1,2,
         "지원 MapPBR의 RNM 및 native SH·hemisphere·ambient 간접 diffuse 기여를 확인합니다.","Render.Combined GPU/PS, 직접광·노출 고정 화면", "BG RNM은 별도입니다. 지원 PBR의 간접 diffuse 합계를 바꾸며 RNM만의 비용이나 dynamic GI 실험이 아닙니다."},
        {"pbr.environment","IBL 환경 specular 기여",ExperimentField::PBR_ENVIRONMENT,NoGate,true,RecipeKind::Contribution,0,1,2,
         "환경 cubemap 반사가 실제 표면에 기여하는지 분리합니다.","Render.Combined GPU/PS, environment asset·roughness 고정", "SSR/planar/ray reflection이 아닙니다. 필요한 환경 입력이 없으면 화면 차이가 없을 수 있습니다."},
        {"pbr.cubeDiffuse","Cube diffuse 기여",ExperimentField::PBR_CUBE,NoGate,true,RecipeKind::Contribution,0,1,2,
         "환경에서 근사한 확산 간접광의 기여를 분리합니다.","Render.Combined GPU/PS, SH/환경 입력·RNM 고정", "일부 family/환경 입력에 한정되며 모든 object의 GI를 끄지 않습니다."},
        {"pbr.normal","PBR normal 강도",ExperimentField::NORMAL_STRENGTH,NoGate,true,RecipeKind::Filter,0,1,5,
         "노멀 입력이 직접광·환경 반사에서 만드는 형태 차이를 비교합니다.","Render.Combined GPU, 하이라이트·표면 디테일", "normal texture를 제거하거나 sampling을 건너뛰는 설정은 아닙니다."},
        {"pbr.roughness","PBR roughness offset",ExperimentField::ROUGHNESS_OFFSET,NoGate,true,RecipeKind::Filter,-.25,.25,5,
         "표면 거칠기와 직접/환경 반사 분포를 비교합니다.","Render.Lights/Combined GPU, specular 폭·환경 mip 결과", "입력에 offset을 적용합니다. 재질별 최종 roughness clamp와 family 차이를 확인합니다."},
        {"fog.pass","높이 안개 ON/OFF",ExperimentField::FOG_ENABLED,NoGate,false,RecipeKind::Workload,0,1,2,
         "현재 height/exponential fog의 영상 기여와 해당 합성 비용을 비교합니다.","Render.Combined/SceneHDR GPU, PSInvocations, 원거리 대비", "volumetric froxel/raymarch 실험이 아닙니다. 시간에 따른 안개 변화는 고정 replay되지 않습니다."},
        {"fog.density","높이 안개 밀도",ExperimentField::FOG_DENSITY,ExperimentField::FOG_ENABLED,false,RecipeKind::Contribution,0,.1,5,
         "같은 높이·거리에서 안개 감쇠 입력을 비교합니다.","Render.Combined GPU, 원거리 대비·색", "density는 매질 입력이며 raymarch step이나 volumetric quality가 아닙니다."},
        {"ssgi.pass","실험 SSGI ON/OFF",ExperimentField::SSGI_ENABLED,NoGate,true,RecipeKind::Workload,0,1,2,
         "화면에 보이는 diffuse bounce 근사 기여와 추가 GPU 비용을 비교합니다.","Render.SSGI GPU/self + Render.ScreenSpaceLighting.Copy, GPU frame P99, PSInvocations", "marker3 MapPBR 수신만 지원합니다. 기존 RNM/IBL을 유지하는 추가 기여이며 Lumen·광선추적·에너지 보존 GI가 아닙니다."},
        {"ssgi.resolution","SSGI 전체 / 절반 해상도",ExperimentField::SSGI_HALF_RESOLUTION,ExperimentField::SSGI_ENABLED,true,RecipeKind::Workload,0,1,2,
         "동일한 ray 수에서 GI 계산 해상도와 경계 보존 합성의 품질·비용을 비교합니다.","Render.SSGI.GatherHalf / ResolveHalf GPU + Copy, GPU frame P99", "절반 해상도는 depth·normal을 이용해 합성합니다. 화면 밖·temporal 복원은 없으며 전체 프레임 비용이 1/4이 되는 것은 아닙니다."},
        {"ssgi.samples","SSGI 샘플 4/8/16",ExperimentField::SSGI_SAMPLES,ExperimentField::SSGI_ENABLED,true,RecipeKind::Workload,4,16,3,
         "같은 화면·반경에서 sample 수와 공간 노이즈/시간의 관계를 비교합니다.","Render.SSGI/SSR GPU ms/P99 + Copy, 얇은 표면·접촉 누락", "temporal history는 없습니다. sample 비율과 ms 개선률은 같지 않습니다."},
        {"ssgi.radius","SSGI 검색 반경",ExperimentField::SSGI_RADIUS,ExperimentField::SSGI_ENABLED,true,RecipeKind::Filter,.5,8,5,
         "화면에서 간접광을 찾는 공간 범위와 빛 번짐을 비교합니다.","Render.SSGI/SSR GPU ms + Copy, 화면 경계·접촉 light leak", "화면 밖·가려진 radiance를 복원하지 않습니다. radius는 bounce 횟수가 아닙니다."},
        {"ssgi.strength","SSGI 기여 강도",ExperimentField::SSGI_STRENGTH,ExperimentField::SSGI_ENABLED,true,RecipeKind::Contribution,0,1,5,
         "기존 baked/IBL 위에 더하는 screen-space 간접 성분을 분리합니다.","Render.SSGI/SSR GPU ms + Copy, 같은 노출·RNM·환경광의 화면", "강도 0은 ray 탐색을 조기 종료하지만 full-screen 패스·복사는 남습니다. 기존 간접광과 중복되어 과밝아질 수 있습니다."},
        {"ssr.pass","실험 SSR ON/OFF",ExperimentField::SSR_ENABLED,NoGate,true,RecipeKind::Workload,0,1,2,
         "현재 화면의 specular reflection 근사와 비용을 비교합니다.","Render.SSR GPU/self + Render.ScreenSpaceLighting.Copy, GPU P99, PSInvocations", "marker3 MapPBR 수신의 추가 반사입니다. 화면 밖·가려진 반사는 없고 기존 IBL을 대체하지 않습니다."},
        {"ssr.steps","SSR step 16/32/64",ExperimentField::SSR_STEPS,ExperimentField::SSR_ENABLED,true,RecipeKind::Workload,16,64,3,
         "동일 ray 거리의 탐색 횟수와 hit/누락 품질을 비교합니다.","Render.SSGI/SSR GPU ms/P99 + Copy, PSInvocations, 반사 끊김", "최대 step은 실제 GPU instruction 수가 아닙니다. 조기 종료·화면 이탈·hit에 따라 달라집니다."},
        {"ssr.distance","SSR 최대 거리",ExperimentField::SSR_DISTANCE,ExperimentField::SSR_ENABLED,true,RecipeKind::Filter,2,40,5,
         "반사 ray 탐색 거리와 원거리 hit를 비교합니다.","Render.SSGI/SSR GPU ms + Copy, 거리별 반사 hit/누락", "step을 고정하면 거리 증가로 탐색 간격도 변할 수 있습니다. 화면 밖 정보는 없습니다."},
        {"ssr.thickness","SSR hit 두께",ExperimentField::SSR_THICKNESS,ExperimentField::SSR_ENABLED,true,RecipeKind::Filter,.05,.8,5,
         "depth 근사 hit 허용 폭과 누락/잘못된 반사를 비교합니다.","Render.SSGI/SSR GPU ms + Copy, 얇은 형상·교차 표면 화면", "실제 형상 두께가 아니라 depth hit 허용치입니다. 값 증가가 정확도 개선을 보장하지 않습니다."},
        {"ssr.strength","SSR 기여 강도",ExperimentField::SSR_STRENGTH,ExperimentField::SSR_ENABLED,true,RecipeKind::Contribution,0,1,5,
         "기존 환경 반사 위에 더하는 SSR 성분을 분리합니다.","Render.SSGI/SSR GPU ms + Copy, IBL·roughness·노출 고정 화면", "additive 반사라 기존 IBL과 중복될 수 있습니다. 강도 0은 ray 탐색을 조기 종료하지만 패스·복사는 남습니다."},
        {"display.exposure","노출 배율",ExperimentField::EXPOSURE,NoGate,false,RecipeKind::Display,.5,2,5,
         "빛의 영상 배율과 tone 결과를 확인합니다.","Render.Final GPU, 밝기·클리핑·Bloom threshold 영향", "밝기 조절 실험입니다. GI/재질 기여 비교 때는 노출을 고정해야 합니다."},
        {"display.gamma","표시 gamma",ExperimentField::GAMMA,NoGate,false,RecipeKind::Display,1.8,2.4,5,
         "표시 변환이 중간 밝기와 색에 미치는 영향을 확인합니다.","Render.Final GPU, source grading cache 준비 CPU, 동일 장면 화면", "새 gamma의 첫 준비 비용과 steady-state를 나누고 warm-up을 사용합니다."},
        {"pbr.sourceIndirect","원본 PBR 간접광 경로 ON/OFF",ExperimentField::SOURCE_PBR_INDIRECT,NoGate,true,RecipeKind::Contribution,0,1,2,
         "원본 SH·hemisphere·환경 입력을 소비하는 지원 PBR 경로와 이전 경로를 비교합니다.","Render.Combined와 지원 source material GPU, 동일 노출·광원·재질 화면", "간접광 전체 제거가 아닙니다. OFF는 보존된 이전 환경 경로로 돌아갑니다. 캐릭터 전체나 모든 재질의 GI 토글이 아닙니다."},
        {"display.sourcePostProcess","Source tone + grading 묶음 ON/OFF",ExperimentField::SOURCE_POST_PROCESS,NoGate,false,RecipeKind::Display,0,1,2,
         "원본 tone curve와 grading 묶음이 밝기·색에 미치는 영향을 분리합니다.","Render.Final GPU, source grading cache 준비 CPU, 같은 HDR 입력의 화면", "SourcePostProcess.enabled 하나를 비교합니다. OFF는 기본 Hable 표시 경로이며 순수 tone-only 또는 LUT-only 실험이 아닙니다. 저장된 curve·색·LUT 입력은 보존합니다."},
        {"display.lut","Source LUT grading ON/OFF",ExperimentField::LUT_ENABLED,ExperimentField::SOURCE_POST_PROCESS,false,RecipeKind::Display,0,1,2,
         "Source tone + grading 묶음을 켠 상태에서 저작 LUT의 색 기여만 비교합니다.","Render.Final GPU, 같은 노출·tone·색 입력의 화면", "저작 LUT 입력이 있어야 ON을 적용할 수 있습니다. LUT OFF는 source tone curve OFF가 아닙니다."},
        {"material.source", "기본 / 원본 재질", ExperimentField::SOURCE_MATERIALS, NoGate, false, RecipeKind::Contribution, 0, 1, 2,
         "지원 표면의 기본 textured shader와 복원 재질 연산을 비교합니다.", "동일 카메라의 재질·음영과 GPU frame", "현재 WModel·텍스처를 사용합니다. 미지원 native/forward 재질은 유지하며 최초 EXE의 재현은 아닙니다."},
        {"optimization.frustum", "프러스텀 컬링", ExperimentField::OPT_FRUSTUM, NoGate, false, RecipeKind::Workload, 0, 1, 2,
         "Layer 후보와 맵 상세 가시성", "Map.CullingCandidates/Visible, CPU visibility, draw/index", "화면 밖 맵 제출을 제외합니다. 거리·occlusion은 별도 설정입니다."},
        {"optimization.occlusion", "가림 컬링", ExperimentField::OPT_OCCLUSION, NoGate, false, RecipeKind::Workload, 0, 1, 2,
         "현재 카메라의 CPU 가림 판정", "Render.MapOcclusion, rejected batches/indices, cache hit", "가려지는 후보가 없는 시점은 효과를 입증할 수 없습니다."},
        {"optimization.distance", "거리 컬링", ExperimentField::OPT_DISTANCE, NoGate, false, RecipeKind::Workload, 0, 1, 2,
         "작은 장식물의 거리·픽셀 제한", "distance tested/rejected instances/indices, draw/index", "거리 scale·최대 pixel은 고정합니다. 큰 지형을 지우는 실험이 아닙니다."},
        {"optimization.mesh-lod", "화면 오차 LOD", ExperimentField::OPT_MESH_LOD, NoGate, false, RecipeKind::Workload, 0, 1, 2,
         "준비된 정적 index LOD 선택", "LOD available/0/1/2, source/submitted indices, IA/VS", "생성된 LOD만 사용합니다. 생성 하한·캐시·UV·오차를 바꾸지 않습니다."},
        {"optimization.map-instancing", "맵 인스턴싱", ExperimentField::OPT_MAP_INSTANCING, NoGate, false, RecipeKind::Workload, 0, 1, 2,
         "같은 payload의 instance별 draw / instanced draw", "draw/instanced draw, mesh instances, Map.Batch.Draw", "A도 기존 배치 객체와 buffer를 사용합니다. 과거 객체 생성 비용 재현이 아닙니다."},
        {"optimization.identical-batch", "동일 모델·재질 추가 병합", ExperimentField::OPT_IDENTICAL_BATCH, ExperimentField::OPT_MAP_INSTANCING, false, RecipeKind::Workload, 0, 1, 2,
         "인접 batch의 같은 geometry·재질 draw 결합", "identical source draws / actual draws, Map.Batch.Draw", "기존 asset별 instancing은 유지합니다. 상위 맵 instancing이 ON이어야 합니다."},
        {"optimization.lighting-bank", "동일 geometry 조명 bank 병합", ExperimentField::OPT_LIGHTING_BANK, ExperimentField::OPT_MAP_INSTANCING, false, RecipeKind::Workload, 0, 1, 2,
         "원본 RNM·재질 호환 batch의 draw 결합", "lighting bank source draws / actual draws, Map.Batch.Draw", "geometry·재질 호환 조건과 원본 조명은 유지합니다. 모든 재질을 하나로 바꾸지 않습니다."},
        {"optimization.shadow-cache", "정적 그림자 캐시", ExperimentField::OPT_SHADOW_CACHE, ExperimentField::SHADOW_ENABLED, false, RecipeKind::Workload, 0, 1, 2,
         "같은 정적 caster 결과 재사용", "shadow hit/miss, StaticBuild/CacheCopy CPU·GPU", "그림자 ON이 전제입니다. 현재 OFF인 그림자를 자동으로 켜지 않습니다."},
        {"optimization.npc-pose-reuse", "NPC pose 표본 재사용", ExperimentField::OPT_NPC_POSE_REUSE, NoGate, false, RecipeKind::Workload, 0, 1, 2,
         "허용된 NPC animation sample 재사용", "animation evaluation/reuse, CPU animation·NPC work", "offscreen pose 지연과 다른 항목입니다. clip·clock·event·Server 상태를 유지합니다."},
        {"optimization.particle-root-cache", "파티클 root 변환 재사용", ExperimentField::OPT_PARTICLE_ROOT_CACHE, NoGate, false, RecipeKind::Workload, 0, 1, 2,
         "같은 emitter의 root inverse·회전 계산 재사용", "particle root builds/reuse, particle CPU scope", "동일 계산의 반복만 줄입니다. 이펙트 개수·재생 시간·형태를 바꾸지 않습니다."},
        {"optimization.map-workers", "맵 CPU 준비 worker", ExperimentField::OPT_MAP_WORKERS, NoGate, false, RecipeKind::Workload, 0, 1, 2,
         "가시성 준비의 직렬 / coarse worker 분산", "prepared batches, caller/worker jobs, Prepare/Dispatch/Join", "기본 OFF입니다. 정지 cache hit는 작업0이며 worker 시간을 main에 합산하지 않습니다."},
        {"optimization.particle-workers", "파티클 CPU worker", ExperimentField::OPT_PARTICLE_WORKERS, NoGate, false, RecipeKind::Workload, 0, 1, 2,
         "허용된 particle update의 직렬 / worker 분산", "particle worker participation, Update/Join CPU", "작은 작업·의존성 있는 emitter는 계속 직렬입니다. gameplay/renderer 작업은 옮기지 않습니다."}
    };

    bool BuildRecipeCandidate(const FExperimentRecipe& recipe, const Client::RENDERING_EXPERIMENT_VALUES& base,
        bool sourceMaterials, Client::RENDERING_EXPERIMENT_VALUES& candidate, float& low, float& high, string& status)
    {
        const auto& fields=Client::CRenderingProfileService::Experiment_Fields();
        const size_t index=static_cast<size_t>(recipe.Field);
        if(recipe.RequiredEnabled!=NoGate && base.values[static_cast<size_t>(recipe.RequiredEnabled)]==0)
        {status="A 기준의 해당 패스가 OFF입니다. 먼저 ON/OFF 실험을 사용하세요. ON인 B를 적용하고 위의 B를 새 A 기준으로 채택한 뒤 이 recipe를 준비하세요. 다른 변수를 자동으로 켜지 않습니다.";return false;}
        if(recipe.SourceRequired && !sourceMaterials)
        {status="이 recipe는 지원 source map PBR 재질이 필요합니다. 현재 source material 경로가 OFF입니다.";return false;}
        if(recipe.Field==ExperimentField::PBR_CUBE && base.values[static_cast<size_t>(ExperimentField::SOURCE_PBR_INDIRECT)]!=0)
        {status="현재 A는 원본 PBR 간접광 경로입니다. 이 경로에서는 project cube diffuse가 비활성이라 해당 기여 비교를 준비할 수 없습니다. 원본 간접광 ON/OFF를 명시적으로 비교하거나, OFF인 B를 새 A로 채택하세요.";return false;}
        low=static_cast<float>((std::max)(recipe.Low,fields[index].minimum));
        high=static_cast<float>((std::min)(recipe.High,fields[index].maximum));
        if(recipe.Field==ExperimentField::SSAO_RADIUS)
        {
            low=(std::max)(low,static_cast<float>(base.values[static_cast<size_t>(ExperimentField::SSAO_BIAS)])+.001f);
            high=(std::min)(high,static_cast<float>(base.values[static_cast<size_t>(ExperimentField::SSAO_FADE)]));
        }
        if(!(low<high)){status="현재 A의 연결 제약에서 서로 다른 두 유효 값을 만들 수 없습니다. 이전 B는 유지합니다.";return false;}
        auto staged=base;staged.values[index]=base.values[index]==low?high:low;
        if(!Client::CRenderingProfileService::Validate_ExperimentValues(staged,status))return false;
        auto endpoint=base;endpoint.values[index]=high;
        if(!Client::CRenderingProfileService::Validate_ExperimentValues(endpoint,status))return false;
        candidate=staged;return true;
    }

    bool BuildQualitySuiteCandidate(const Client::RENDERING_EXPERIMENT_VALUES& base,
        Client::RENDERING_EXPERIMENT_VALUES& candidate, string& status)
    {
        if (base.values[static_cast<size_t>(ExperimentField::SOURCE_MATERIALS)]==0)
        { status="품질 확장 후보는 현재 source material 경로가 필요합니다. 재질 선택을 자동으로 바꾸지 않습니다."; return false; }
        auto staged=base;
        const auto set=[&](ExperimentField field,double value) { staged.values[static_cast<size_t>(field)]=value; };
        const auto positive=[&](ExperimentField field,double fallback) {
            if(staged.values[static_cast<size_t>(field)]==0) set(field,fallback);
        };
        set(ExperimentField::TEXTURE_MIN_MIP,0);
        set(ExperimentField::SSAO_ENABLED,1); set(ExperimentField::SSAO_SAMPLES,12);
        set(ExperimentField::HORIZON_AO_ENABLED,1); positive(ExperimentField::SSAO_INTENSITY,1);
        set(ExperimentField::PCF_RADIUS,2);
        set(ExperimentField::SSGI_ENABLED,1); set(ExperimentField::SSGI_SAMPLES,16);
        set(ExperimentField::SSGI_HALF_RESOLUTION,0); positive(ExperimentField::SSGI_STRENGTH,.25);
        set(ExperimentField::SSR_ENABLED,1); set(ExperimentField::SSR_STEPS,64);
        set(ExperimentField::SSR_REFINEMENT_ENABLED,1); set(ExperimentField::SSR_ROUGHNESS_FILTER_ENABLED,1);
        positive(ExperimentField::SSR_STRENGTH,.5);
        // This is an explicit multi-field candidate. Tone, AA, bloom, lighting and
        // authored data remain the original values; it is not a measured optimum.
        if(!Client::CRenderingProfileService::Validate_ExperimentValues(staged,status)) return false;
        candidate=staged; return true;
    }

    struct FPresentationStage { const char* Name; const char* Description; };
    constexpr FPresentationStage PresentationStages[] = {
        {"01  WModel 기본 재질 (초기 근사)", "현재 형상·UV·텍스처를 유지한 기본 재질입니다. 직접광·노출·감마는 유지합니다. A/B는 같은 기준을 반복해 측정 편차를 봅니다."},
        {"02  원본 재질 · BG baked 복구", "지원 표면의 원본 재질식을 복원합니다. BG의 RNM·정적 shadow 입력도 이 단계에 포함됩니다. hair·forward 등 기본 경로가 없는 재질은 유지합니다."},
        {"03  MapPBR · 간접 diffuse", "지원 MapPBR의 간접 diffuse 배율과 RNM 기여를 복원합니다. 원본 SH·hemisphere 경로는 05에서 돌아옵니다. BG RNM은 02에 포함됩니다."},
        {"04  MapPBR · 환경 반사", "지원 MapPBR의 환경 specular·project cube diffuse 기여를 복원합니다. 환경 입력이 없는 표면에서는 차이가 없을 수 있습니다."},
        {"05  원본 PBR · SH / BRDF 경로", "지원 표면의 원본 간접광 경로로 복귀합니다. OFF는 기존 환경식이며 GI 전체 제거가 아닙니다. native SH·128x32 BRDF 연결은 실제 재질별로 다릅니다."},
        {"06  방향광 그림자", "현재 방향광 shadow 설정을 복원합니다. 재질에 구워진 정적 ShadowMap2D는 별도입니다."},
        {"07  SSAO · 접촉 차폐", "depth/normal을 사용하는 현재 SSAO를 복원합니다. GTAO나 전체 GI가 아닙니다."},
        {"08  높이 안개", "현재 height/exponential fog 설정을 복원합니다. 원래 OFF라면 OFF를 유지합니다."},
        {"09  Source tone · grading", "원본 tone curve와 grading 묶음을 복원합니다. OFF는 Hable 경로이며 노출·감마는 유지합니다. 저작 LUT는 다음 단계입니다."},
        {"10  LUT · 장면 탈색", "저작 LUT와 Hable 경로의 장면 탈색 값을 복원합니다. Source tone OFF 또는 LUT 입력이 없으면 LUT 차이는 없습니다. 원본 grading의 탈색은 09에 포함됩니다."},
        {"11  Bloom · 빛 번짐", "현재 HDR Bloom 설정을 복원합니다. 광원이나 간접광을 추가하는 단계가 아닙니다."},
        {"12  FXAA · 가장자리", "현재 FXAA 설정을 복원합니다. 저장된 OFF는 그대로이며 켜기 실험은 Technique A/B에서 명시적으로 선택합니다."},
        {"13  SSGI (추가 실험)", "원래 켜져 있던 SSGI만 복원합니다. 원작 복구 이력과 별개인 marker3 MapPBR의 화면 공간 가산 기여입니다."},
        {"14  SSR · 현재 설정 (추가 실험)", "원래 켜져 있던 SSR까지 복원해 시작 때의 전체 설정에 도달합니다. 화면 밖 반사·Lumen·광선추적은 아닙니다."}
    };

    Client::RENDERING_EXPERIMENT_VALUES BuildPresentationStage(
        const Client::RENDERING_EXPERIMENT_VALUES& original, int stage)
    {
        auto candidate=original;
        const auto off=[&](ExperimentField field) { candidate.values[static_cast<size_t>(field)]=0; };
        if(stage<1) off(ExperimentField::SOURCE_MATERIALS);
        if(original.values[static_cast<size_t>(ExperimentField::SOURCE_MATERIALS)]!=0)
        {
            if(stage<2) off(ExperimentField::PBR_BAKED);
            if(stage<3) for(auto field:{ExperimentField::PBR_ENVIRONMENT,ExperimentField::PBR_CUBE}) off(field);
            if(stage<4) off(ExperimentField::SOURCE_PBR_INDIRECT);
        }
        if(stage<5) off(ExperimentField::SHADOW_ENABLED);
        if(stage<6) off(ExperimentField::SSAO_ENABLED);
        if(stage<7) off(ExperimentField::FOG_ENABLED);
        if(stage<8) off(ExperimentField::SOURCE_POST_PROCESS);
        // A disabled source post-process may still retain authored LUT inputs.
        // Keep those dormant inputs unowned; the service rejects LUT ON with PP OFF.
        if(stage<9 && original.values[static_cast<size_t>(ExperimentField::SOURCE_POST_PROCESS)]!=0)
            off(ExperimentField::LUT_ENABLED);
        if(stage<9) off(ExperimentField::DESATURATION);
        if(stage<10) off(ExperimentField::BLOOM_ENABLED);
        if(stage<11) off(ExperimentField::FXAA_ENABLED);
        if(stage<12) off(ExperimentField::SSGI_ENABLED);
        if(stage<13) off(ExperimentField::SSR_ENABLED);
        return candidate;
    }

    struct FQuickTechnique { const char* Name; const char* Recipe; int Group=1; }; // 0: source, 1: current, 2: optional extras.
    constexpr FQuickTechnique QuickTechniques[] = {
        {"표면 텍스처 · mip 품질", "texture.mip", 2},
        {"Horizon AO · 기존 SSAO와 비교", "ao.horizon", 2},
        {"SSR · 교차 구간 세부 탐색", "ssr.refinement", 2},
        {"SSR · roughness 반사 필터", "ssr.roughnessFilter", 2},
        {"기본 / 원본 재질", "material.source", 0},
        {"MapPBR · 간접 diffuse (RNM / SH)", "pbr.baked", 0},
        {"IBL · 환경 반사", "pbr.environment", 0},
        {"원본 PBR 간접광", "pbr.sourceIndirect", 0},
        {"PBR · normal", "pbr.normal"},
        {"PBR · 직접 반사", "pbr.directSpecular"},
        {"방향광 그림자", "shadow.pass"},
        {"SSAO · 굴곡 음영", "ssao.pass"},
        {"높이 안개", "fog.pass"},
        {"Tone + grading", "display.sourcePostProcess", 0},
        {"LUT · 색보정", "display.lut", 0},
        {"Bloom · 빛 번짐", "bloom.pass"},
        {"FXAA · 가장자리", "fxaa.pass"},
        {"SSGI · 화면 공간 간접광 (실험)", "ssgi.pass", 2},
        {"SSGI · 전체 / 절반 해상도", "ssgi.resolution", 2},
        {"SSR · 화면 공간 반사 (실험)", "ssr.pass", 2}
    };

    string RecipeConfidence(RecipeKind kind)
    {
        switch(kind)
        {
        case RecipeKind::Workload:return "실제 기능/샘플 수의 단일 변수 실험. 반복 편차·화질을 확인해야 하며 탐색 결과는 인과 보증이 아닙니다.";
        case RecipeKind::Contribution:return "화면 성분 분리용. 기여값0의 연산 생략 범위는 기법마다 다릅니다. PBR scale은 연산 유지 가능, SSGI/SSR은 trace 조기 종료 뒤 패스·복사는 남습니다.";
        case RecipeKind::Filter:return "필터·표면 품질 입력 실험. 실행 횟수는 유지될 수 있어 화면과 측정 ms를 따로 판단합니다.";
        case RecipeKind::Display:return "표시 변환 진단. 조명 알고리즘의 속도·정확도 비교로 해석하지 않습니다.";
        }
        return {};
    }

    std::map<string,string> ChangedConditionFields(const std::map<string,string>& before,const std::map<string,string>& after)
    {
        std::map<string,string> result;
        const auto clipped=[](const string& value) {
            if(value.size()<=180)return value;
            size_t end=180;while(end && (static_cast<unsigned char>(value[end])&0xc0u)==0x80u)--end;
            return value.substr(0,end)+"...";
        };
        for(const auto& [name,value]:before)
        {
            const auto found=after.find(name);
            if(found==after.end())result[name]=clipped(value)+" -> (없음)";
            else if(value!=found->second)result[name]=clipped(value)+" -> "+clipped(found->second);
        }
        for(const auto& [name,value]:after)if(!before.contains(name))result[name]="(없음) -> "+clipped(value);
        return result;
    }

    // Normalize only explicitly selected experiment fields. All other renderer
    // input fields below remain in the comparison; source mode is handled by caller.
    string ComparisonConditions(uint64_t excluded = 0u, const Engine::SHADOW_LIGHT_DESC* shadowBasis = nullptr, std::map<string,string>* named = nullptr, bool rowContext = false)
    {
        const auto& game = Engine::CGameInstance::Get();
        auto& mutableGame = Engine::CGameInstance::Get();
        ostringstream stream;
        stream.imbue(locale::classic());
        stream << setprecision(9) << scientific;
        const auto scalar = [&](const auto value, std::string_view key = {}) {
            stream << value << ' ';
            if (named && !key.empty())
            { ostringstream text; text.imbue(locale::classic()); text<<setprecision(9)<<scientific<<value; (*named)[string(key)]=text.str(); }
        };
        const auto vector = [&](const auto& value, std::string_view key = {}) {
            scalar(value.x); scalar(value.y); scalar(value.z); scalar(value.w);
            if (named && !key.empty())
            { ostringstream text; text.imbue(locale::classic()); text<<setprecision(9)<<scientific<<value.x<<' '<<value.y<<' '<<value.z<<' '<<value.w; (*named)[string(key)]=text.str(); }
        };
        scalar(game.Get_CurrentLevelID(),"scene.level");
        scalar(IsDebuggerPresent()!=FALSE,"capture.debuggerAttached");
        const auto* profiler=mutableGame.Get_Profiler();
        scalar(!rowContext && profiler && profiler->Is_CollectingDetailedScopes(),"capture.detailActive");
        scalar(profiler && profiler->Is_DetailedScopesEnabled(),"capture.detailRequested");
        scalar(Engine::CPresentation_Manager::Get().Are_TransientLightsEnabled(),"presentation.transientLights");
        scalar(Engine::CPresentation_Manager::Get().Are_ScreenPostsEnabled(),"presentation.screenPosts");
        const auto& user = Client::CUserSettings::Get().Get_Settings();
        scalar(user.Display.width,"video.width"); scalar(user.Display.height,"video.height"); scalar(static_cast<int>(user.Display.mode),"video.mode");
        for (const auto& [key, value] : user.Values) { stream << quoted(key); scalar(value,named?"video."+key:string{}); }
        DWORD foregroundProcess=0; GetWindowThreadProcessId(GetForegroundWindow(),&foregroundProcess);
        scalar(foregroundProcess==GetCurrentProcessId(),"window.processForeground");
        scalar(Client::CUserSettings::Get().Get_FrameLimit(foregroundProcess==GetCurrentProcessId()),"video.activeFpsCap");
        scalar(Client::CMapLightPresentationRuntime::Get_SceneIntensityMultiplier(),"scene.mapLightIntensity");
        if (const auto* arena = Client::CLevel_KakulSaydonArena::Get_Active())
            scalar(arena->Get_MapLightComparisonFingerprint(),"scene.mapLightComparison");
        const auto viewport = mutableGame.Get_ViewportSize(); scalar(viewport.x,"viewport.width"); scalar(viewport.y,"viewport.height");
        for (const auto type : {Engine::D3DTS::VIEW, Engine::D3DTS::PROJ})
        {
            const auto* matrix = mutableGame.Get_Transform(type);
            const char* key=type==Engine::D3DTS::VIEW?"camera.view":"camera.projection";
            if (!matrix) { stream << "missing-matrix "; if(named)(*named)[key]="missing";continue; }
            const auto offset=stream.tellp();
            for (const auto& row : matrix->m) for (const auto value : row) scalar(value);
            if(named)(*named)[key]=stream.str().substr(static_cast<size_t>(offset));
        }
        auto material = game.Get_MaterialRenderSettings();
        using F = Client::RENDERING_EXPERIMENT_FIELD;
        const auto omit = [&](F f) { return (excluded & Client::RenderingExperimentBit(f)) != 0u; };
        const auto zero = [&](F f, auto& target) { if (omit(f)) target = {}; };
        const uint64_t pbrMask = ((uint64_t{1} << (static_cast<size_t>(F::ROUGHNESS_OFFSET)+1u)) - 1u) &
            ~((uint64_t{1} << static_cast<size_t>(F::PBR_DIFFUSE)) - 1u);
        if (excluded & pbrMask)
        {
            if (!material.MapPBR.Is_Active(game.Get_CurrentLevelID())) material.MapPBR = {};
            material.MapPBR.bEnabled = false; material.MapPBR.iLevel = 0;
            zero(F::PBR_DIFFUSE, material.MapPBR.vContributionScale.x); zero(F::PBR_SPECULAR, material.MapPBR.vContributionScale.y);
            zero(F::PBR_BAKED, material.MapPBR.vContributionScale.z); zero(F::PBR_ENVIRONMENT, material.MapPBR.vContributionScale.w);
            zero(F::PBR_CUBE, material.MapPBR.fCubeDiffuseScale); zero(F::NORMAL_STRENGTH, material.MapPBR.vSurfaceParameters.x);
            zero(F::ROUGHNESS_OFFSET, material.MapPBR.vSurfaceParameters.y);
        }
        scalar(static_cast<uint32_t>(material.eDebugView),"material.debugView");
        scalar(material.MapPBR.bEnabled,"material.pbr.bEnabled"); scalar(material.MapPBR.iLevel,"material.pbr.iLevel");
        vector(material.MapPBR.vContributionScale,"material.pbr.vContributionScale"); vector(material.MapPBR.vSurfaceParameters,"material.pbr.vSurfaceParameters");
        scalar(material.MapPBR.fCubeDiffuseScale,"material.pbr.fCubeDiffuseScale");
        auto q = game.Get_RenderQualitySettings();
        zero(F::SSAO_ENABLED,q.bSSAOEnabled); zero(F::SSAO_RADIUS,q.fSSAORadius); zero(F::SSAO_BIAS,q.fSSAOBias);
        zero(F::SSAO_INTENSITY,q.fSSAOIntensity); zero(F::SSAO_POWER,q.fSSAOPower); zero(F::SSAO_FADE,q.fSSAODistanceFade);
        zero(F::BLOOM_ENABLED,q.bBloomEnabled); zero(F::BLOOM_THRESHOLD,q.fBloomThreshold); zero(F::BLOOM_KNEE,q.fBloomSoftKnee);
        zero(F::BLOOM_INTENSITY,q.fBloomIntensity); zero(F::BLOOM_SCATTER,q.fBloomScatter);
        zero(F::FXAA_ENABLED,q.bFXAAEnabled); zero(F::FXAA_BLEND,q.fFXAASubpixel); zero(F::FXAA_EDGE,q.fFXAAEdgeThreshold);
        zero(F::FXAA_EDGE_MIN,q.fFXAAEdgeThresholdMin); zero(F::EXPOSURE,q.fExposure); zero(F::GAMMA,q.fGamma);
        zero(F::DESATURATION,q.fSceneDesaturation);
        zero(F::SSAO_SAMPLES,q.iSSAOSampleCount); scalar(q.iSSAOSampleCount,"quality.iSSAOSampleCount");
        zero(F::SSGI_ENABLED,q.bSSGIEnabled); scalar(q.bSSGIEnabled,"quality.bSSGIEnabled");
        zero(F::SSGI_HALF_RESOLUTION,q.bSSGIHalfResolution); scalar(q.bSSGIHalfResolution,"quality.bSSGIHalfResolution");
        zero(F::SSGI_STRENGTH,q.fSSGIStrength); scalar(q.fSSGIStrength,"quality.fSSGIStrength");
        zero(F::SSGI_RADIUS,q.fSSGIRadius); scalar(q.fSSGIRadius,"quality.fSSGIRadius");
        zero(F::SSGI_SAMPLES,q.iSSGISampleCount); scalar(q.iSSGISampleCount,"quality.iSSGISampleCount");
        zero(F::SSR_ENABLED,q.bSSREnabled); scalar(q.bSSREnabled,"quality.bSSREnabled");
        zero(F::SSR_STRENGTH,q.fSSRStrength); scalar(q.fSSRStrength,"quality.fSSRStrength");
        zero(F::SSR_DISTANCE,q.fSSRMaxDistance); scalar(q.fSSRMaxDistance,"quality.fSSRMaxDistance");
        zero(F::SSR_THICKNESS,q.fSSRThickness); scalar(q.fSSRThickness,"quality.fSSRThickness");
        zero(F::SSR_STEPS,q.iSSRStepCount); scalar(q.iSSRStepCount,"quality.iSSRStepCount");
        zero(F::TEXTURE_MIN_MIP,q.iTextureMinMip); scalar(q.iTextureMinMip,"quality.iTextureMinMip");
        zero(F::HORIZON_AO_ENABLED,q.bHorizonAOEnabled); scalar(q.bHorizonAOEnabled,"quality.bHorizonAOEnabled");
        zero(F::SSR_REFINEMENT_ENABLED,q.bSSRRefinementEnabled); scalar(q.bSSRRefinementEnabled,"quality.bSSRRefinementEnabled");
        zero(F::SSR_ROUGHNESS_FILTER_ENABLED,q.bSSRRoughnessFilterEnabled); scalar(q.bSSRRoughnessFilterEnabled,"quality.bSSRRoughnessFilterEnabled");
        zero(F::SOURCE_POST_PROCESS,q.SourcePostProcess.bEnabled);
        if (omit(F::LUT_ENABLED)) q.SourcePostProcess.LutLayers.clear();
        scalar(q.bSSAOEnabled,"quality.bSSAOEnabled"); scalar(q.fSSAORadius,"quality.fSSAORadius"); scalar(q.fSSAOBias,"quality.fSSAOBias");
        scalar(q.fSSAOIntensity,"quality.fSSAOIntensity"); scalar(q.fSSAOPower,"quality.fSSAOPower"); scalar(q.fSSAODistanceFade,"quality.fSSAODistanceFade");
        scalar(q.bBloomEnabled,"quality.bBloomEnabled"); scalar(q.fBloomThreshold,"quality.fBloomThreshold"); scalar(q.fBloomSoftKnee,"quality.fBloomSoftKnee");
        scalar(q.fBloomIntensity,"quality.fBloomIntensity"); scalar(q.fBloomScatter,"quality.fBloomScatter"); scalar(q.fExposure,"quality.fExposure");
        scalar(q.fWhitePoint,"quality.fWhitePoint"); scalar(q.fGamma,"quality.fGamma"); scalar(q.bFXAAEnabled,"quality.bFXAAEnabled");
        vector(q.vBloomTint,"quality.vBloomTint"); scalar(q.fSceneDesaturation,"quality.fSceneDesaturation");
        scalar(q.iColorFilterType,"quality.iColorFilterType"); scalar(q.fColorFilterStrength,"quality.fColorFilterStrength");
        const auto& source = q.SourcePostProcess;
        scalar(source.bEnabled,"quality.source.bEnabled"); scalar(source.fToneScale,"quality.source.fToneScale"); scalar(source.fToneRange,"quality.source.fToneRange");
        scalar(source.fToneToe,"quality.source.fToneToe"); scalar(source.fDesaturation,"quality.source.fDesaturation");
        size_t sourceColor=0;
        for (const auto& value : { source.vHighlights, source.vMidtones, source.vShadows, source.vColorize })
        {
            const string key=named?"quality.source.color["+std::to_string(sourceColor++)+"]":string{};
            scalar(value.x,named?key+".x":string{}); scalar(value.y,named?key+".y":string{}); scalar(value.z,named?key+".z":string{});
        }
        scalar(source.LutLayers.size(),"quality.lut.count");
        size_t lutIndex=0; const auto lutOffset=stream.tellp();
        for (const auto& layer : source.LutLayers)
        {
            const bool detail=named && lutIndex<8;
            const string key=detail?"quality.lut["+std::to_string(lutIndex)+"]":string{}; ++lutIndex;
            scalar(layer.fWeight,detail?key+".weight":string{});
            if (layer.pLut) stream << quoted(layer.pLut->strAssetId);
            else stream << "neutral-lut ";
            if(detail)(*named)[key+".asset"]=layer.pLut?layer.pLut->strAssetId:"neutral";
        }
        if(named && source.LutLayers.size()>8)
        {
            uint64_t hash=14695981039346656037ull;
            for(unsigned char ch:stream.str().substr(static_cast<size_t>(lutOffset))){hash^=ch;hash*=1099511628211ull;}
            (*named)["quality.lut.allRecordsHash"]=std::to_string(hash);
        }
        scalar(q.fFXAASubpixel,"quality.fFXAASubpixel"); scalar(q.fFXAAEdgeThreshold,"quality.fFXAAEdgeThreshold"); scalar(q.fFXAAEdgeThresholdMin,"quality.fFXAAEdgeThresholdMin");
        auto fog = game.Get_HeightFogSettings(); zero(F::FOG_ENABLED,fog.bEnabled); zero(F::FOG_DENSITY,fog.fDensity);
        scalar(fog.bEnabled,"fog.bEnabled"); vector(fog.vColor,"fog.vColor"); scalar(fog.fDensity,"fog.fDensity"); scalar(fog.fHeightFalloff,"fog.fHeightFalloff");
        scalar(fog.fTopHeight,"fog.fTopHeight"); scalar(fog.fStartDistance,"fog.fStartDistance"); scalar(fog.fMaximumOpacity,"fog.fMaximumOpacity");
        scalar(fog.fDriftSpeed,"fog.fDriftSpeed"); scalar(fog.fDriftHeightAmplitude,"fog.fDriftHeightAmplitude"); scalar(fog.fDriftDensityAmplitude,"fog.fDriftDensityAmplitude");
        scalar(fog.fCoveragePercent,"fog.fCoveragePercent"); scalar(fog.fWindDirectionX,"fog.fWindDirectionX"); scalar(fog.fWindDirectionZ,"fog.fWindDirectionZ");
        scalar(fog.fWindSpeed,"fog.fWindSpeed"); scalar(fog.fPatchScale,"fog.fPatchScale"); scalar(fog.fPatchSoftness,"fog.fPatchSoftness");
        scalar(fog.bSourceExponential,"fog.bSourceExponential"); vector(fog.vInscatteringColor,"fog.vInscatteringColor"); vector(fog.vFogLightDirection,"fog.vFogLightDirection");
        auto environment = game.Get_RenderEnvironment();
        zero(F::SOURCE_PBR_INDIRECT,environment.bUseSourcePBRIndirect);
        scalar(environment.strCubePath.size(),"environment.cubePathLength");
        const auto cubeOffset=stream.tellp();
        for (const auto character : environment.strCubePath) scalar(static_cast<uint32_t>(character));
        if(named)(*named)["environment.cubePathCodepoints"]=stream.str().substr(static_cast<size_t>(cubeOffset));
        vector(environment.vColor,"environment.vColor"); vector(environment.vRotationIntensity,"environment.vRotationIntensity");
        scalar(environment.fDiffuseIntensity,"environment.fDiffuseIntensity");
        scalar(environment.bUseSourcePBRIndirect,"environment.bUseSourcePBRIndirect");
        size_t shIndex=0;
        for (const auto& row : environment.vDiffuseSH) vector(row,named?"environment.diffuseSH["+std::to_string(shIndex++)+"]":string{});
        auto shadow = game.Get_ShadowLightDesc();
        if (omit(F::SHADOW_ENABLED) && shadowBasis) shadow=*shadowBasis;
        zero(F::SHADOW_ENABLED,shadow.Settings.bEnabled);
        zero(F::SHADOW_STRENGTH,shadow.Settings.fStrength); vector(shadow.vEye,"shadow.vEye"); vector(shadow.vAt,"shadow.vAt");
        zero(F::PCF_RADIUS,shadow.Settings.iPCFFilterRadius); scalar(shadow.Settings.iPCFFilterRadius,"shadow.settings.iPCFFilterRadius");
        const auto& s = shadow.Settings; scalar(s.bEnabled,"shadow.settings.bEnabled"); scalar(s.fOrthographicWidth,"shadow.settings.fOrthographicWidth");
        scalar(s.fOrthographicHeight,"shadow.settings.fOrthographicHeight"); scalar(s.fNear,"shadow.settings.fNear"); scalar(s.fFar,"shadow.settings.fFar"); scalar(s.fDepthBias,"shadow.settings.fDepthBias");
        scalar(s.fNormalBias,"shadow.settings.fNormalBias"); scalar(s.fStrength,"shadow.settings.fStrength"); scalar(s.fDynamicBakedStrength,"shadow.settings.fDynamicBakedStrength");
        const auto& lights = game.Get_SceneLights(); scalar(lights.size(),"scene.lights.count");
        size_t lightIndex=0; const auto lightsOffset=stream.tellp();
        for (const auto& light : lights)
        {
            const string lightKey=named && lightIndex<8?"scene.light["+std::to_string(lightIndex)+"]":string{};
            ++lightIndex;
            // Named detail is bounded. The complete raw fingerprint still covers every record.
            auto* namedAll=named;if(lightIndex>8)named=nullptr;
            scalar(static_cast<uint32_t>(light.eType),named?lightKey+".type":string{}); vector(light.vDirection,named?lightKey+".vDirection":string{}); vector(light.vPosition,named?lightKey+".vPosition":string{});
            scalar(light.fRange,named?lightKey+".fRange":string{}); scalar(light.fFalloffExponent,named?lightKey+".fFalloffExponent":string{}); vector(light.vDiffuse,named?lightKey+".vDiffuse":string{});
            vector(light.vAmbient,named?lightKey+".vAmbient":string{}); vector(light.vSpecular,named?lightKey+".vSpecular":string{}); scalar(light.fSpotInnerCos,named?lightKey+".fSpotInnerCos":string{}); scalar(light.fSpotOuterCos,named?lightKey+".fSpotOuterCos":string{});
            vector(light.vSourceCharacterAmbient,named?lightKey+".vSourceCharacterAmbient":string{});
            scalar(static_cast<uint32_t>(light.eReceiver),named?lightKey+".receiver":string{}); scalar(light.staticShadowChannel,named?lightKey+".staticShadowChannel":string{});
            named=namedAll;
        }
        if(named && lights.size()>8)
        {
            uint64_t hash=14695981039346656037ull;
            for(unsigned char ch:stream.str().substr(static_cast<size_t>(lightsOffset))) {hash^=ch;hash*=1099511628211ull;}
            (*named)["scene.lights.allRecordsHash"]=std::to_string(hash);
        }
        const auto optimization=Client::CRenderingProfileService::Read_ExperimentValues();
        const auto& experimentFields=Client::CRenderingProfileService::Experiment_Fields();
        for(size_t i=static_cast<size_t>(F::OPT_FRUSTUM);i<experimentFields.size();++i)
            scalar(omit(static_cast<F>(i))?0.0:optimization.values[i],experimentFields[i].id);
        const auto visibility=game.Get_MapVisibilitySettings();
        scalar(visibility.DistanceScale,"optimization.distanceScale");
        scalar(visibility.DistanceMaxPixels,"optimization.distanceMaxPixels");
#ifdef _DEBUG
        scalar(1,"build.debug");
#else
        scalar(0,"build.debug");
#endif
        scalar(_ITERATOR_DEBUG_LEVEL,"build.iteratorDebugLevel");
        scalar(_MSC_VER,"build.msvc");
        scalar(sizeof(void*)*8u,"build.pointerBits");
        return stream.str();
    }
	double Percentile(std::vector<double> Values, const double fPercentile)
	{
		if (Values.empty())
			return 0.0;
		std::sort(Values.begin(), Values.end());
		const double fRank = fPercentile * static_cast<double>(Values.size() - 1u);
		const size_t iLower = static_cast<size_t>(fRank);
		const size_t iUpper = (std::min)(iLower + 1u, Values.size() - 1u);
		const double fFraction = fRank - static_cast<double>(iLower);
		return Values[iLower] + (Values[iUpper] - Values[iLower]) * fFraction;
	}

	string Now_Timestamp()
	{
		const auto Now = chrono::system_clock::now();
		const time_t CalendarTime = chrono::system_clock::to_time_t(Now);
		tm LocalTime{};
		localtime_s(&LocalTime, &CalendarTime);
		ostringstream Stream;
		Stream << put_time(&LocalTime, "%Y-%m-%d %H:%M:%S");
		return Stream.str();
	}

	string Escape_Json(const string& Value)
	{
		string Escaped;
		Escaped.reserve(Value.size() + 8u);
		for (const char Character : Value)
		{
			switch (Character)
			{
			case '"': Escaped += "\\\""; break;
			case '\\': Escaped += "\\\\"; break;
			case '\n': Escaped += "\\n"; break;
			case '\r': Escaped += "\\r"; break;
			case '\t': Escaped += "\\t"; break;
			default:
                if (static_cast<unsigned char>(Character) < 0x20u)
                {
                    static constexpr char hex[] = "0123456789abcdef";
                    Escaped += "\\u00"; Escaped += hex[(Character >> 4) & 15]; Escaped += hex[Character & 15];
                }
                else Escaped += Character;
                break;
			}
		}
		return Escaped;
	}
}

namespace
{
    std::string OptimizationApplicability(const Client::RENDERING_BENCHMARK_RUN& run)
    {
        using C=Engine::EProfilerCounter;
        const auto n=[&](C c){return run.counterTotals[static_cast<size_t>(c)];};
        if(!run.strRecipeId.starts_with("optimization.")) return {};
        double observed=0;
        if(run.strRecipeId=="optimization.frustum") observed=n(C::MapCullingCandidates);
        else if(run.strRecipeId=="optimization.occlusion") observed=n(C::MapOcclusionCandidates);
        else if(run.strRecipeId=="optimization.distance") observed=n(C::MapDistanceTestedInstances)+n(C::MapDistanceRejectedInstances);
        else if(run.strRecipeId=="optimization.mesh-lod") observed=n(C::MapLodAvailableDraws);
        else if(run.strRecipeId=="optimization.map-instancing") observed=n(C::MapVisibleInstances);
        else if(run.strRecipeId=="optimization.identical-batch") observed=n(C::MapIdenticalInstanceSourceDraws);
        else if(run.strRecipeId=="optimization.lighting-bank") observed=n(C::MapLightingBankSourceDraws);
        else if(run.strRecipeId=="optimization.shadow-cache") observed=n(C::ShadowStaticCasters);
        else if(run.strRecipeId=="optimization.npc-pose-reuse") observed=n(C::AnimationSampleRequests);
        else if(run.strRecipeId=="optimization.particle-root-cache") observed=n(C::EffectParticleRootInverseRequests);
        else if(run.strRecipeId=="optimization.map-workers") observed=n(C::MapVisibilityPreparedBatches);
        else if(run.strRecipeId=="optimization.particle-workers") observed=n(C::EffectParticleCallerJobs)+n(C::EffectParticleWorkerJobs);
        // Several producers count only the enabled path. OFF zero is not proof
        // that the scene has no candidates; use the ON phase to establish that.
        if(observed<=0) return run.strVariant=="A" ? "A: OFF 경로; 적용 대상은 B 표본으로 확인" :
            "B: 적용 대상 관측 0 · 최적화 효과 미입증";
        return run.strVariant+": 적용 대상 관측됨 · 비용과 실제 감소량을 함께 확인";
    }

    size_t RawEvidenceBytes(const Engine::FProfilerCaptureSnapshot& snapshot)
    {
        size_t bytes=sizeof(snapshot)+snapshot.Frames.capacity()*sizeof(Engine::FProfilerFrame);
        bytes+=snapshot.ScopeNames.capacity()*sizeof(std::string);
        for(const auto& name:snapshot.ScopeNames) bytes+=name.capacity();
        for(const auto& frame:snapshot.Frames)
        {
            bytes+=frame.CpuScopes.capacity()*sizeof(Engine::FProfilerScopeSample);
            bytes+=frame.GpuScopes.capacity()*sizeof(Engine::FProfilerGpuScopeSample);
            bytes+=frame.MeshDraws.capacity()*sizeof(Engine::FProfilerMeshDrawSample);
            bytes+=frame.ViewportPresents.capacity()*sizeof(Engine::FProfilerViewportPresent);
        }
        return bytes;
    }
}

void Client::CRenderingBenchmark::Set_DeviceInfo(ID3D11Device* device)
{
    m_bDeviceInfoValid=false; m_DeviceCreationFlags=0; m_AdapterName="unavailable";
    if(!device) return;
    m_DeviceCreationFlags=device->GetCreationFlags();
    Microsoft::WRL::ComPtr<IDXGIDevice> dxgi;
    Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
    DXGI_ADAPTER_DESC description{};
    if(SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(dxgi.GetAddressOf()))) &&
        SUCCEEDED(dxgi->GetAdapter(adapter.GetAddressOf())) && SUCCEEDED(adapter->GetDesc(&description)))
    {
        char name[512]{};
        if(WideCharToMultiByte(CP_UTF8,0,description.Description,-1,name,sizeof(name),nullptr,nullptr)>0)
        { m_AdapterName=name; m_bDeviceInfoValid=true; }
    }
}

void Client::CRenderingBenchmark::Shutdown(CRenderingProfileService& profiles)
{
    m_bKeepOptimizationHidden=false;
    Update_RestorationPreview(profiles,false);
    if(m_bCapturing || m_bSequence) Cancel_Capture("Benchmark shutdown; completed evidence retained.");
    Poll_RawEvidence();
    // The existing summary future owns no Engine data. Join the older summary
    // before queueing the final owned run list; otherwise queued runs are lost.
    if(m_SaveFuture.valid())
    {
        try { m_strSaveStatus=m_SaveFuture.get().message; }
        catch(const std::exception& error) { m_strSaveStatus=error.what(); }
    }
    for(auto& run:m_Runs)if(!run.rawEvidencePath.empty() && !run.rawEvidenceReady && run.rawEvidencePath==m_RawSavePath)
        run.rawEvidenceStatus="종료 시 원본 저장 완료 미확인 · 유효 증거로 사용하지 마세요";
    if(!m_Runs.empty())Queue_Save(true);
    // The raw exporter uses its existing cancellation/bounded join on destruction.
    // Final summary records pending raw evidence as incomplete, never successful.
}

Client::FProfilerCaptureContext Client::CRenderingBenchmark::Sample_OptimizationContext() const
{
    auto& game=Engine::CGameInstance::Get();
    FProfilerCaptureContext context;
    context.Valid=true; context.LevelId=game.Get_CurrentLevelID();
    context.Adapter=m_AdapterName; context.DeviceCreationFlags=m_DeviceCreationFlags;
    const auto viewport=game.Get_ViewportSize(); context.Viewport={viewport.x,viewport.y};
    if(const auto* p=game.Get_CamPosition()) context.CameraPosition={p->x,p->y,p->z,p->w};
    if(const auto* p=game.Get_Transform(Engine::D3DTS::VIEW)) std::memcpy(context.ViewMatrix.data(),p,sizeof(*p));
    if(const auto* p=game.Get_Transform(Engine::D3DTS::PROJ)) std::memcpy(context.ProjectionMatrix.data(),p,sizeof(*p));
    const auto& shadow=game.Get_ShadowLightDesc().Settings;
    context.ShadowEnabled=shadow.bEnabled; context.ShadowWidth=shadow.fOrthographicWidth;
    context.ShadowHeight=shadow.fOrthographicHeight; context.ShadowStrength=shadow.fStrength;
    const auto quality=game.Get_RenderQualitySettings();
    context.SSAOEnabled=quality.bSSAOEnabled; context.BloomEnabled=quality.bBloomEnabled; context.FXAAEnabled=quality.bFXAAEnabled;
    DWORD foreground=0; const auto window=GetForegroundWindow(); GetWindowThreadProcessId(window,&foreground);
    context.ProcessForeground=foreground==GetCurrentProcessId(); context.ClientWindowForeground=window==g_hWnd;
    context.WindowMinimized=IsIconic(g_hWnd)!=FALSE;
    context.ForegroundFpsLimit=CUserSettings::Get().Get_FrameLimit(true);
    context.BackgroundFpsLimit=CUserSettings::Get().Get_FrameLimit(false);
    context.EffectiveFpsLimit=context.ProcessForeground?context.ForegroundFpsLimit:context.BackgroundFpsLimit;
    const auto& fields=CRenderingProfileService::Experiment_Fields();
    for(size_t i=0;i<fields.size();++i) context.RenderingOptions[fields[i].id]=m_CaptureValues.values[i];
    const auto visibility=game.Get_MapVisibilitySettings();
    context.RenderingOptions["optimization.distanceScale"]=visibility.DistanceScale;
    context.RenderingOptions["optimization.distanceMaxPixels"]=visibility.DistanceMaxPixels;
    context.RenderingOptions["benchmark.workbenchVisible"]=m_bToolVisible?1:0;
    context.RenderingAssets["benchmark.contextSampling"]="start of sampled phase after warmup; per-frame condition fingerprint checked";
    context.RenderingAssets["benchmark.measurementId"]=m_strMeasurementId;
    context.RenderingAssets["benchmark.experimentId"]=m_strCaptureExperimentId;
    context.RenderingAssets["benchmark.variant"]=m_strCaptureVariant;
    context.RenderingAssets["benchmark.row"]=m_strMeasurementRowId;
    return context;
}

bool Client::CRenderingBenchmark::Queue_RawEvidence(Engine::FProfilerCaptureSnapshot&& snapshot,
    RENDERING_BENCHMARK_RUN& run)
{
    run.applicability=OptimizationApplicability(run);
    if(m_RawExporter.IsSaving()) { run.rawEvidenceStatus="원본 증거 저장기가 사용 중입니다."; return false; }
    // Preserve only this phase before a later phase can evict its history.
    double excludedMaximum=0;
    for(const auto& frame:snapshot.Frames)
        if(frame.FrameNumber<run.firstFrame || frame.FrameNumber>run.lastFrame)
            excludedMaximum=(std::max)(excludedMaximum,frame.FrameIntervalMs);
    std::erase_if(snapshot.Frames,[&](const auto& frame){return frame.FrameNumber<run.firstFrame || frame.FrameNumber>run.lastFrame;});
    if(snapshot.Frames.size()!=run.iFrames)
    { run.rawEvidenceStatus="원본 증거의 프레임 범위가 측정 결과와 다릅니다."; return false; }
    auto& window=snapshot.CaptureWindow;
    window.RequestedFrames=run.iFrames; window.SavedFrames=snapshot.Frames.size();
    window.ExcludedRetainedFrames=window.RetainedFrames-window.SavedFrames;
    window.FirstSavedFrameNumber=run.firstFrame; window.LastSavedFrameNumber=run.lastFrame;
    window.ExcludedMaxFrameIntervalMs=excludedMaximum;
    constexpr size_t maximumBytes=64u*1024u*1024u;
    if(RawEvidenceBytes(snapshot)>maximumBytes)
    { run.rawEvidenceStatus="단계 원본 증거가 64 MiB 한도를 넘었습니다. 측정 프레임/상세 scope를 줄여 다시 측정하세요."; return false; }
    auto path=Make_DefaultPath(); path.replace_filename("evidence_"+path.filename().string());
    auto context=m_OptimizationCaptureContext;
    context.RenderingAssets["benchmark.repetition"]=std::to_string(run.repetition);
    context.RenderingAssets["benchmark.conditionsStable"]=run.bConditionsStable?"true":"false";
    context.RenderingAssets["benchmark.failureReason"]=run.strFailureReason;
    context.RenderingAssets["benchmark.frameRange"]=std::to_string(run.firstFrame)+".."+std::to_string(run.lastFrame);
    string error;
    run.rawEvidencePath=path.string();
    if(!m_RawExporter.BeginSave(std::move(snapshot),path,&error,std::move(context)))
    { run.rawEvidenceStatus="원본 증거 저장 시작 실패: "+error; return false; }
    m_RawSavePath=run.rawEvidencePath;
    run.rawEvidenceStatus="저장 중";
    return true;
}

void Client::CRenderingBenchmark::Poll_RawEvidence()
{
    FProfilerCaptureSaveResult result;
    if(!m_RawExporter.Poll(result)) return;
    for(auto& run:m_Runs)
        if(run.rawEvidencePath==m_RawSavePath)
        {
            run.rawEvidenceReady=result.Succeeded;
            run.rawEvidenceStatus=result.Succeeded?"저장 완료":"저장 실패: "+result.Error;
        }
    m_RawSavePath.clear(); m_bSaveQueued=true;
    if(!result.Succeeded && m_bOptimizationSession)
        Cancel_Capture("원본 증거 저장 실패: "+result.Error);
}

bool_t Client::CRenderingBenchmark::Prepare_OptimizationPair(size_t recipeIndex, CRenderingProfileService& profiles)
{
    if(recipeIndex>=std::size(ExperimentRecipes) ||
        !std::string_view(ExperimentRecipes[recipeIndex].Id).starts_with("optimization.")) return false;
    const bool wasActive=m_bExperimentActive;
    if(!Start_SessionExperiment(profiles)) return false;
    const auto& recipe=ExperimentRecipes[recipeIndex];
    auto a=m_ExperimentOriginal, b=a;
    if(recipe.RequiredEnabled!=NoGate && a.values[static_cast<size_t>(recipe.RequiredEnabled)]==0)
    { if(!wasActive)End_Experiment();m_strStatus="전제 설정이 OFF입니다. 다른 옵션을 자동 변경하지 않습니다."; return false; }
    if(recipe.Field==ExperimentField::OPT_LIGHTING_BANK && a.values[static_cast<size_t>(ExperimentField::SOURCE_MATERIALS)]==0)
    { if(!wasActive)End_Experiment();m_strStatus="조명 bank는 원본 source material 경로가 필요합니다."; return false; }
    a.values[static_cast<size_t>(recipe.Field)]=0; b.values[static_cast<size_t>(recipe.Field)]=1;
    if(!Apply_ComparisonPair(a,b))
    {
        if(!wasActive) { const auto status=m_strStatus;End_Experiment();m_strStatus=status; }
        return false;
    }
    m_iWarmupInput=(std::max)(2u,m_iWarmupInput);
    m_iPresentationStage=-1; m_iSelectedRecipe=m_iPreparedRecipe=static_cast<int>(recipeIndex);
    m_iSweepField=static_cast<int>(recipe.Field);m_fSweepMinimum=0;m_fSweepMaximum=1;m_iSweepSteps=2;
    Mark_PreparedComparison(recipe.Id);
    std::snprintf(m_LabelBuffer.data(),m_LabelBuffer.size(),"%s",recipe.Id);
    m_strStatus="A는 해당 최적화 OFF, B는 ON입니다. 종료 시 시작할 때의 전체 설정으로 복원합니다.";
    return true;
}

void Client::CRenderingBenchmark::Render_OptimizationSection(CRenderingProfileService& profiles)
{
    auto& game=Engine::CGameInstance::Get();
    const bool busy=m_bCapturing || m_bSequence || m_RawExporter.IsSaving();
    const auto original=m_bExperimentActive?m_ExperimentOriginal:CRenderingProfileService::Read_ExperimentValues();
    ImGui::TextWrapped("같은 장면에서 한 최적화만 OFF/ON 비교합니다. 고정 카메라 ABBA: A1 → B1 → B2 → A2. 준비·저장 시간은 표본에서 제외하고 마지막에는 이전 A/B 화면으로 돌아갑니다.");
    ImGui::TextWrapped("카메라·viewport·다른 옵션이 바뀌면 비교를 제외합니다. 움직이는 컷신과 NPC·이펙트 재생은 결정적 replay가 아니므로 고정 시점의 비용과 원본 증거를 함께 확인하세요.");
    ImGui::BeginDisabled(busy);
    ImGui::SliderInt("측정 프레임##optimization",&m_iFrameInput,10,900);
    int warm=static_cast<int>(m_iWarmupInput);
    if(ImGui::SliderInt("준비 프레임##optimization",&warm,2,120)) m_iWarmupInput=static_cast<uint32_t>(warm);
    ImGui::EndDisabled();
    if(m_bOptimizationSession)
    {
        ImGui::Checkbox("영상 촬영: F1로 숨겨도 최적화 A/B 화면 유지",&m_bKeepOptimizationHidden);
        if(m_bKeepOptimizationHidden) ImGui::TextWrapped("다시 F1을 열고 원래 화면으로 복귀할 수 있습니다. 종료·취소·장면 변경·실패는 계속 복원합니다. 숨김 상태는 warmup이 끝나기 전에 고정하세요.");
    }
    if(busy && ImGui::Button("측정 취소 · 원래값 복원")) Cancel_Capture("최적화 측정을 취소했습니다.");
    ImGui::TextWrapped("%s",m_strStatus.c_str()); ImGui::TextWrapped("%s",m_strSaveStatus.c_str());
    const float height=(std::max)(ImGui::GetTextLineHeightWithSpacing()*12.f,ImGui::GetContentRegionAvail().y*.65f);
    if(ImGui::BeginTable("OptimizationComparison",3,ImGuiTableFlags_RowBg|ImGuiTableFlags_BordersInnerH|ImGuiTableFlags_Resizable|ImGuiTableFlags_ScrollY,ImVec2(0,height)))
    {
        ImGui::TableSetupColumn("최적화 · 실제 적용 범위",ImGuiTableColumnFlags_WidthStretch,1.4f);
        ImGui::TableSetupColumn("A OFF / B ON",ImGuiTableColumnFlags_WidthFixed,ImGui::GetFrameHeight()*5.f);
        ImGui::TableSetupColumn("실측 비용 · 처리량",ImGuiTableColumnFlags_WidthStretch,1.f);
        ImGui::TableSetupScrollFreeze(0,1);ImGui::TableHeadersRow();
        for(size_t i=0;i<std::size(ExperimentRecipes);++i)
        {
            const auto& recipe=ExperimentRecipes[i];if(!std::string_view(recipe.Id).starts_with("optimization."))continue;
            string unavailable;
            if(recipe.RequiredEnabled!=NoGate && original.values[static_cast<size_t>(recipe.RequiredEnabled)]==0)
                unavailable=recipe.RequiredEnabled==ExperimentField::SHADOW_ENABLED?"그림자가 OFF입니다.":"상위 맵 instancing이 OFF입니다.";
            if(recipe.Field==ExperimentField::OPT_LIGHTING_BANK && original.values[static_cast<size_t>(ExperimentField::SOURCE_MATERIALS)]==0)
                unavailable="원본 source material이 OFF입니다.";
            ImGui::PushID(static_cast<int>(i));ImGui::TableNextRow();ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(recipe.Name);ImGui::TextWrapped("runtime · %s",recipe.Goal);
            ImGui::TextWrapped("%s",unavailable.empty()?recipe.Boundary:unavailable.c_str());
            ImGui::TableSetColumnIndex(1);ImGui::BeginDisabled(busy || !unavailable.empty());
            if(ImGui::Button("A OFF")) {if(Prepare_OptimizationPair(i,profiles))Apply_ExperimentVariant(false);}
            ImGui::SameLine();if(ImGui::Button("B ON"))Prepare_OptimizationPair(i,profiles);
            if(ImGui::Button("ABBA 측정"))if(Prepare_OptimizationPair(i,profiles))Start_ComparisonMeasurement(game.Get_Profiler());
            ImGui::EndDisabled();
            if(m_bExperimentActive && m_strPreparedComparisonRowId==recipe.Id)ImGui::Text("현재 %s",m_bVariantB?"B ON":"A OFF");
            ImGui::TableSetColumnIndex(2);Render_ComparisonCost(recipe.Id);ImGui::PopID();
        }
        ImGui::EndTable();
    }
    if(ImGui::CollapsingHeader("적용 시점 · 비교 해석"))
    {
        ImGui::TextWrapped("맵 재준비 필요: 32m 공간 분할·생성 순서, geometry/atlas bake, LOD 생성 하한과 파생 캐시 cold/warm. 이 표의 runtime 스위치는 로드된 구조만 사용합니다. Atlas 경로는 기본 비활성 준비 상태이며 이 표로 켜지지 않습니다.");
        ImGui::TextWrapped("별도 실행 필요: Debug/Release와 D3D debug device. 최초 진입·파생 cache·OS 파일 cache는 별도 준비 조건을 기록해야 하며 재시작만으로 cold 상태가 보장되지 않습니다.");
        ImGui::TextWrapped("대상 관측 0은 최적화 효과 미입증입니다. 고정 카메라와 warmup 뒤의 occlusion은 warm cache 비용이며, 맵 worker 준비 작업은 cache hit로 0일 수 있습니다. CPU/GPU는 겹치고 Map render 안의 material/pass/draw도 중첩됩니다. Worker·Dispatch·Join 시간을 서로 더하지 마세요. GPU pending/invalid는 0ms가 아닙니다.");
        ImGui::TextWrapped("각 단계는 기존 Profiler 형식의 raw JSON을 별도로 저장합니다(단계 snapshot 메모리 한도 64 MiB). 저장 후 다음 warmup을 시작하며 결과 JSON의 rawEvidencePath와 frame range로 연결합니다.");
    }
    Render_Results();
}

string Client::CRenderingBenchmark::Current_Conditions(uint64_t excludedFields, std::map<string,string>* named, bool rowContext) const
{
    const auto* shadowBasis=m_pExperimentProfiles?&m_pExperimentProfiles->Get_ExperimentShadowBasis():nullptr;
    string result = ComparisonConditions(excludedFields,shadowBasis,named,rowContext);
    result += " deviceKnown="+std::to_string(m_bDeviceInfoValid)+" deviceFlags="+
        std::to_string(m_DeviceCreationFlags)+" adapter="+m_AdapterName;
    if(!rowContext) result+=" toolVisible="+std::to_string(m_bToolVisible);
    if(named)
    {
        (*named)["device.known"]=m_bDeviceInfoValid?"1":"0";
        (*named)["device.creationFlags"]=std::to_string(m_DeviceCreationFlags);
        (*named)["device.adapter"]=m_AdapterName;
        if(!rowContext)(*named)["ui.workbenchVisible"]=m_bToolVisible?"1":"0";
    }
    if (shadowBasis)
    {
        // Keep the underlying shadow owner in every fingerprint even while OFF
        // normalizes the visible Engine descriptor to defaults.
        ostringstream input; input.imbue(locale::classic()); input<<setprecision(9)<<scientific;
        const auto& s=shadowBasis->Settings;
        for (auto v:{shadowBasis->vEye.x,shadowBasis->vEye.y,shadowBasis->vEye.z,shadowBasis->vEye.w,
            shadowBasis->vAt.x,shadowBasis->vAt.y,shadowBasis->vAt.z,shadowBasis->vAt.w,
            s.fOrthographicWidth,s.fOrthographicHeight,s.fNear,s.fFar,s.fDepthBias,s.fNormalBias,
            s.fStrength,s.fDynamicBakedStrength}) input<<v<<' ';
        input<<s.bEnabled<<' '<<s.iPCFFilterRadius;result+=" shadowBasis="+input.str();
        if(named)(*named)["shadow.underlyingBasis"]=input.str();
    }
    if (m_bCaptureExperiment || m_bExperimentActive)
    {
        const bool source=(excludedFields & RenderingExperimentBit(RENDERING_EXPERIMENT_FIELD::SOURCE_MATERIALS))==0 &&
            Engine::CGameInstance::Get().Get_MaterialRenderSettings().bUseSourceMaterials;
        result += source ? " source=1" : " source=0";
        if(named)(*named)["material.sourceEnabled"]=source?"1":"0";
        if (m_pExperimentProfiles)
        {
            if(named)
            {
                (*named)["owner.profile"]=m_pExperimentProfiles->Get_ActiveProfileId();
                (*named)["owner.levelQuality"]=m_pExperimentProfiles->Get_LevelQualityProfileId();
                (*named)["owner.region"]=m_pExperimentProfiles->Get_AppliedEnvironmentRegionId();
                (*named)["owner.generation"]=std::to_string(m_pExperimentProfiles->Get_ProfileGeneration());
            }
            result += " owner=" + m_pExperimentProfiles->Get_ActiveProfileId() + "/" +
                m_pExperimentProfiles->Get_LevelQualityProfileId() + "/" + m_pExperimentProfiles->Get_AppliedEnvironmentRegionId() +
                "/" + std::to_string(m_pExperimentProfiles->Get_ProfileGeneration());
        }
    }
    return result;
}

namespace
{
    bool SameComparisonValues(const Client::RENDERING_EXPERIMENT_VALUES& left,
        const Client::RENDERING_EXPERIMENT_VALUES& right)
    {
        // Renderer setters consume floats. A decimal recipe endpoint and the
        // value read back from that setter must use the same precision.
        for (size_t i=0;i<left.values.size();++i)
            if (!std::isfinite(left.values[i]) || !std::isfinite(right.values[i]) ||
                static_cast<float>(left.values[i])!=static_cast<float>(right.values[i])) return false;
        return true;
    }

    bool ComparisonAppliedValuesMatch(const Client::RENDERING_EXPERIMENT_VALUES& actual,
        const Client::RENDERING_EXPERIMENT_VALUES& expected)
    {
        auto normalized=expected;
        using F=Client::RENDERING_EXPERIMENT_FIELD;
        // CShadow::Apply_Shadow_Light resets an OFF descriptor's strength,
        // but explicitly preserves PCF radius. Do not relax other inputs.
        if(normalized.values[static_cast<size_t>(F::SHADOW_ENABLED)]==0)
            normalized.values[static_cast<size_t>(F::SHADOW_STRENGTH)]=Engine::SHADOW_SETTINGS{}.fStrength;
        return SameComparisonValues(actual,normalized);
    }

    bool ComparisonRunValuesValid(const Client::RENDERING_BENCHMARK_RUN& run)
    {
        if (run.strMeasurementId.empty()) return true;
        return (run.strVariant=="A" || run.strVariant=="B") &&
            ComparisonAppliedValuesMatch(run.appliedValues,run.strVariant=="B"?run.expectedB:run.expectedA);
    }

    bool ComparisonRunsMatch(const Client::RENDERING_BENCHMARK_RUN& a,
        const Client::RENDERING_BENCHMARK_RUN& b)
    {
        if (!a.bConditionsStable || !b.bConditionsStable || a.fieldMask!=b.fieldMask ||
            a.strComparisonConditions!=b.strComparisonConditions || a.strExperimentId!=b.strExperimentId)
            return false;
        if (a.strMeasurementId.empty() && b.strMeasurementId.empty()) return true;
        return !a.strMeasurementId.empty() && a.strMeasurementId==b.strMeasurementId &&
            a.strComparisonRowId==b.strComparisonRowId && a.strComparisonSessionId==b.strComparisonSessionId &&
            a.repetition==b.repetition && a.iFrames==b.iFrames && a.iFrames>0 &&
            a.warmupFrames==b.warmupFrames && SameComparisonValues(a.expectedA,b.expectedA) &&
            SameComparisonValues(a.expectedB,b.expectedB) && ComparisonRunValuesValid(a) && ComparisonRunValuesValid(b);
    }
    bool OptimizationEvidenceComplete(const Client::RENDERING_BENCHMARK_RUN& current,
        const std::vector<Client::RENDERING_BENCHMARK_RUN>& runs)
    {
        if(!current.strRecipeId.starts_with("optimization."))return true;
        if(current.strMeasurementId.empty())return false;
        std::array<bool,4> found{};
        auto sameRepeat=current;
        for(const auto& run:runs)if(run.strMeasurementId==current.strMeasurementId)
        {
            sameRepeat.repetition=run.repetition;
            if(!run.rawEvidenceReady || !ComparisonRunsMatch(sameRepeat,run) ||
                run.repetition<1 || run.repetition>2 || (run.strVariant!="A" && run.strVariant!="B"))return false;
            const size_t index=(run.repetition-1u)*2u+(run.strVariant=="B"?1u:0u);
            if(found[index])return false;found[index]=true;
        }
        return std::all_of(found.begin(),found.end(),[](bool value){return value;});
    }


}

void Client::CRenderingBenchmark::Reset_ComparisonRows()
{
    static uint64_t revision=0;
    m_strComparisonSessionId=m_strExperimentId+".rows."+std::to_string(++revision);
    m_strPreparedComparisonRowId.clear(); m_ComparisonRows.clear();
    m_bOptimizationSession=false; m_bKeepOptimizationHidden=false;
    m_iComparisonContextUiFrame=-1; m_strComparisonContext.clear();
}

void Client::CRenderingBenchmark::Mark_PreparedComparison(const string& rowId)
{
    const auto found=m_ComparisonRows.find(rowId);
    if (found!=m_ComparisonRows.end() &&
        (!SameComparisonValues(found->second.a,m_ExperimentA) || !SameComparisonValues(found->second.b,m_ExperimentB)))
        m_ComparisonRows.erase(found);
    m_strPreparedComparisonRowId=rowId;
    m_bOptimizationSession=rowId.starts_with("optimization.");
    if(!m_bOptimizationSession) m_bKeepOptimizationHidden=false;
    m_PreparedComparisonA=m_ExperimentA; m_PreparedComparisonB=m_ExperimentB;
}

bool_t Client::CRenderingBenchmark::Prepare_PresentationPair(int stage, CRenderingProfileService& profiles)
{
    if(stage<0 || stage>=static_cast<int>(std::size(PresentationStages))) return false;
    const bool wasActive=m_bExperimentActive;
    if(!Start_SessionExperiment(profiles)) return false;
    const auto a=BuildPresentationStage(m_ExperimentOriginal,stage>0?stage-1:0);
    const auto b=BuildPresentationStage(m_ExperimentOriginal,stage);
    if(!Apply_ComparisonPair(a,b))
    {
        if(!wasActive) { const string reason=m_strStatus; End_Experiment(); m_strStatus=reason; }
        return false;
    }
    m_iPresentationStage=stage; m_iPreparedRecipe=-1;
    const string rowId="restoration.stage."+std::to_string(stage+1);
    Mark_PreparedComparison(rowId);
    std::snprintf(m_LabelBuffer.data(),m_LabelBuffer.size(),"%s",rowId.c_str());
    m_strStatus=PresentationStages[stage].Description;
    return true;
}

bool_t Client::CRenderingBenchmark::Start_ComparisonMeasurement(Engine::CProfiler* profiler)
{
    if(!profiler || !m_bExperimentActive || !m_pExperimentProfiles || !m_pExperimentProfiles->Has_ExperimentPreview() ||
        m_bCapturing || m_bSequence || m_bSweep || m_strPreparedComparisonRowId.empty() ||
        !SameComparisonValues(m_PreparedComparisonA,m_ExperimentA) || !SameComparisonValues(m_PreparedComparisonB,m_ExperimentB))
    { m_strStatus="행의 A/B를 다시 준비한 뒤 측정하세요. 완료된 결과는 보존합니다."; return false; }
    if(m_iFrameInput<10 || m_iFrameInput>900 || m_iWarmupInput>120)
    { m_strStatus="측정은 10~900프레임, 준비는 0~120프레임입니다."; return false; }
    const bool previousVariant=m_bVariantB;
    if(!Apply_ExperimentVariant(false)) return false;
    static uint64_t sequence=0;
    const string measurementId="measurement."+std::to_string(GetCurrentProcessId())+"."+
        std::to_string(GetTickCount64())+"."+std::to_string(++sequence);
    m_bComparisonMeasurement=true; m_bComparisonRestoreVariantB=previousVariant;
    m_strMeasurementId=measurementId; m_strMeasurementRowId=m_strPreparedComparisonRowId;
    m_bSequence=true; m_iSequenceStep=0; m_iSequenceTotal=4; // A1, B1, B2, A2.
    m_bSequenceProfilerWasEnabled=profiler->Is_Enabled(); m_bProfilerWasEnabled=m_bSequenceProfilerWasEnabled;
    m_pCaptureProfiler=profiler;
    if(!Begin(profiler,m_LabelBuffer.data(),static_cast<uint32_t>(m_iFrameInput),"row comparison ABBA",m_strStatus))
    { Finish_Sequence(); return false; }
    COMPARISON_ROW row;
    row.sessionId=m_strComparisonSessionId; row.measurementId=measurementId; row.experimentId=m_strExperimentId;
    row.a=m_ExperimentA; row.b=m_ExperimentB; row.fields=Experiment_FieldMask();
    row.frames=static_cast<uint32_t>(m_iFrameInput); row.warmup=m_iWarmupInput;
    m_ComparisonRows.insert_or_assign(m_strMeasurementRowId,std::move(row));
    m_iComparisonContextUiFrame=-1;
    return true;
}

Client::CRenderingBenchmark::COMPARISON_COST Client::CRenderingBenchmark::Build_ComparisonCost(
    const string& rowId, const string& sceneConditions) const
{
    COMPARISON_COST cost;
    const auto found=m_ComparisonRows.find(rowId);
    if(!m_bExperimentActive || found==m_ComparisonRows.end())
    { cost.status="미계측"; return cost; }
    const auto& row=found->second;
    if(row.sessionId!=m_strComparisonSessionId ||
        (!m_strPreparedComparisonRowId.empty() &&
         (!SameComparisonValues(m_PreparedComparisonA,m_ExperimentA) || !SameComparisonValues(m_PreparedComparisonB,m_ExperimentB))))
    { cost.status="이전 기준 결과 · A/B를 다시 준비하세요"; return cost; }
    if(m_bComparisonMeasurement && m_strMeasurementId==row.measurementId)
    { cost.status="ABBA 측정 중 "+std::to_string((std::min)(m_iSequenceStep+1u,4u))+" / 4"; return cost; }
    // The row context excludes staged fields so other rows can remain visible.
    // Still reject a later external change to the currently selected variant.
    if(!ComparisonAppliedValuesMatch(CRenderingProfileService::Read_ExperimentValues(),
        m_bVariantB?m_ExperimentB:m_ExperimentA))
    { cost.status="비교 제외: 현재 실효 설정이 준비한 A/B와 다릅니다"; return cost; }
    if(!row.failure.empty()) { cost.status="비교 제외: "+row.failure; return cost; }
    if(row.sceneConditions.empty() || row.sceneConditions!=sceneConditions)
    { cost.status="비교 제외: 측정 때의 카메라·장면 조건과 다릅니다"; return cost; }
    std::array<const RENDERING_BENCHMARK_RUN*,4> pair{};
    for(const auto& run:m_Runs)
    {
        if(run.strMeasurementId!=row.measurementId) continue;
        if(run.strComparisonRowId!=rowId || run.strComparisonSessionId!=row.sessionId ||
            run.strExperimentId!=row.experimentId || run.fieldMask!=row.fields ||
            run.repetition<1 || run.repetition>2 || (run.strVariant!="A" && run.strVariant!="B") ||
            run.iFrames!=row.frames || run.warmupFrames!=row.warmup ||
            !SameComparisonValues(run.expectedA,row.a) || !SameComparisonValues(run.expectedB,row.b) ||
            !ComparisonRunValuesValid(run) || !run.bConditionsStable ||
            run.strComparisonConditions!=row.commonConditions ||
            !std::isfinite(run.fCpuAvgMs) || run.fCpuAvgMs<0)
        { cost.status="비교 제외: 측정 조건·적용값·반복 짝 불일치"; return cost; }
        const size_t index=(run.repetition-1u)*2u+(run.strVariant=="B"?1u:0u);
        if(pair[index]) { cost.status="비교 제외: 중복 반복 표본"; return cost; }
        pair[index]=&run;
    }
    if(std::any_of(pair.begin(),pair.end(),[](const auto* value){return !value;}))
    { cost.status="N/A · ABBA 미완료 또는 128개 결과 이력에서 만료"; return cost; }
    if(!ComparisonRunsMatch(*pair[0],*pair[1]) || !ComparisonRunsMatch(*pair[2],*pair[3]))
    { cost.status="비교 제외: A/B 공통 조건이 다릅니다"; return cost; }
    cost.cpuValid=true; cost.gpuValid=true; cost.frames=row.frames*2u;
    cost.cpuA=(pair[0]->fCpuAvgMs+pair[2]->fCpuAvgMs)*.5;
    cost.cpuB=(pair[1]->fCpuAvgMs+pair[3]->fCpuAvgMs)*.5;
    cost.cpuSpreadA=std::abs(pair[0]->fCpuAvgMs-pair[2]->fCpuAvgMs);
    cost.cpuSpreadB=std::abs(pair[1]->fCpuAvgMs-pair[3]->fCpuAvgMs);
    for(const auto* run:pair)
    {
        cost.pendingGpu+=run->pendingGpuFrames; cost.invalidGpu+=run->invalidGpuFrames;
        cost.partialScopes=cost.partialScopes || run->droppedCpuScopes || run->droppedGpuScopes;
        cost.gpuValid=cost.gpuValid && run->iGpuFrames==run->iFrames && !run->pendingGpuFrames && !run->invalidGpuFrames &&
            std::isfinite(run->fGpuAvgMs) && run->fGpuAvgMs>=0;
    }
    if(cost.gpuValid)
    {
        cost.gpuA=(pair[0]->fGpuAvgMs+pair[2]->fGpuAvgMs)*.5;
        cost.gpuB=(pair[1]->fGpuAvgMs+pair[3]->fGpuAvgMs)*.5;
    }
    for(const auto* run:pair)
    {
        auto& counters=run->strVariant=="B"?cost.countersB:cost.countersA;
        auto& work=run->strVariant=="B"?cost.workB:cost.workA;
        for(size_t i=0;i<counters.size();++i) counters[i]+=run->counterTotals[i]/(2.0*run->iFrames);
        for(size_t i=0;i<work.size();++i) work[i]+=run->workTotals[i].CpuMs/(2.0*run->iFrames);
        if(run->rawEvidenceReady) ++cost.evidenceReady;
        if(!run->applicability.empty() && cost.applicability.find(run->applicability)==string::npos)
        { if(!cost.applicability.empty())cost.applicability+=" / ";cost.applicability+=run->applicability; }
    }
    if(std::string_view(rowId).starts_with("optimization.") && cost.evidenceReady!=4u)
    { cost.cpuValid=false;cost.gpuValid=false;cost.status="원본 증거 저장 대기 · "+std::to_string(cost.evidenceReady)+" / 4";return cost; }
    cost.status="ABBA 완료 · 프레임 차이 (탐색 측정)";
    return cost;
}

void Client::CRenderingBenchmark::Render_ComparisonCost(const char* rowId)
{
    if(!rowId) return;
    const int uiFrame=ImGui::GetFrameCount();
    if(m_iComparisonContextUiFrame!=uiFrame && !m_ComparisonRows.empty())
    {
        constexpr uint64_t allFields=(uint64_t{1}<<RENDERING_EXPERIMENT_FIELD_COUNT)-1u;
        m_strComparisonContext=Current_Conditions(allFields,nullptr,true);
        m_iComparisonContextUiFrame=uiFrame;
    }
    const auto cost=Build_ComparisonCost(rowId,m_strComparisonContext);
    if(!cost.cpuValid) { ImGui::TextWrapped("%s",cost.status.c_str()); return; }
    ImGui::TextWrapped("CPU A %.3f / B %.3f\nB-A %+.3f ms",cost.cpuA,cost.cpuB,cost.cpuB-cost.cpuA);
    if(cost.gpuValid) ImGui::TextWrapped("GPU A %.3f / B %.3f\nB-A %+.3f ms",cost.gpuA,cost.gpuB,cost.gpuB-cost.gpuA);
    else ImGui::TextWrapped("GPU N/A · pending %u / invalid %u",cost.pendingGpu,cost.invalidGpu);
    ImGui::TextWrapped("A/B 각 %u frames · 반복 CPU 차 %.3f / %.3f ms",cost.frames,cost.cpuSpreadA,cost.cpuSpreadB);
    if(cost.partialScopes) ImGui::TextWrapped("scope 일부 누락 · 위 수치는 전체 프레임 시간");
    if(std::string_view(rowId).starts_with("optimization."))
    {
        const auto c=[&](Engine::EProfilerCounter id,bool b){return (b?cost.countersB:cost.countersA)[static_cast<size_t>(id)];};
        ImGui::Text("draw A %.0f / B %.0f",c(Engine::EProfilerCounter::DrawCalls,false),c(Engine::EProfilerCounter::DrawCalls,true));
        ImGui::Text("index A %.0f / B %.0f",c(Engine::EProfilerCounter::Indices,false),c(Engine::EProfilerCounter::Indices,true));
        ImGui::Text("Map render A %.3f / B %.3f ms",cost.workA[static_cast<size_t>(Engine::EProfilerWork::MapBatchRender)],cost.workB[static_cast<size_t>(Engine::EProfilerWork::MapBatchRender)]);
        if(std::string_view(rowId)=="optimization.map-workers")
        {
            const double prepared=c(Engine::EProfilerCounter::MapVisibilityPreparedBatches,true);
            const double workers=c(Engine::EProfilerCounter::MapVisibilityWorkerJobs,true);
            ImGui::Text("준비 batch / worker job B %.1f / %.1f",prepared,workers);
            if(prepared==0 || workers==0) ImGui::TextWrapped("병렬 대상 없음 · 이 시간 차이로 worker 효과를 입증할 수 없습니다.");
        }
        if(std::string_view(rowId)=="optimization.occlusion")
            ImGui::Text("warm cache hit A %.1f / B %.1f",c(Engine::EProfilerCounter::MapOcclusionCacheHits,false),c(Engine::EProfilerCounter::MapOcclusionCacheHits,true));
        ImGui::TextWrapped("%s",cost.applicability.c_str());
        ImGui::Text("원본 증거 %u / 4 단계 저장",cost.evidenceReady);
    }
}


bool_t Client::CRenderingBenchmark::Begin(Engine::CProfiler* profiler, const string& label,
    uint32_t frames, const string& qualitySummary, string& status)
{
    if (!profiler || m_bCapturing || m_RawExporter.IsSaving()) { status = "Profiler unavailable or a capture is already running."; return false; }
    if(m_bOptimizationSession && m_iWarmupInput<2u)
    { status="최적화 적용 경계의 이전 frame을 제외하려면 준비 프레임이 2 이상이어야 합니다.";return false; }
    // Preserve other Profiler history. A run occupies a bounded window plus a
    // drain tail, and each repeat is finalized before starting the next.
    if (frames < 10u || frames > 900u || m_iWarmupInput > 120u)
    { status = "Use 10..900 sample frames and 0..120 warm-up frames."; return false; }
    if (m_bExperimentActive && !Apply_ExperimentVariant(m_bVariantB))
    { status = m_strStatus; return false; }
    m_bProfilerWasEnabled = m_bSequence ? m_bSequenceProfilerWasEnabled : profiler->Is_Enabled();
    Engine::FProfilerLiveStats live{}; profiler->Get_LiveStats(live);
    m_iCaptureWarmup = m_iWarmupInput;
    m_iStartFrame = live.FrameNumber + 1u + m_iCaptureWarmup;
    m_iLastObservedFrame = live.FrameNumber;
    profiler->Set_Enabled(true);
    m_iTargetFrames = frames; m_strLabel = label.empty() ? "run" : label;
    m_strQualitySummary = qualitySummary;
    m_bCaptureExperiment = m_bExperimentActive;
    m_iCaptureFields = m_bCaptureExperiment ? Experiment_FieldMask() : 0u;
    m_strCaptureExperimentId = m_bCaptureExperiment ? m_strExperimentId : "material.source";
    m_strCaptureVariant = m_bCaptureExperiment ? (m_bVariantB ? "B" : "A") :
        (Engine::CGameInstance::Get().Get_MaterialRenderSettings().bUseSourceMaterials ? "B" : "A");
    if (m_bSweep && m_iSequenceStep) m_strCaptureVariant = "sweep=" + std::to_string(m_SweepPoints[m_iSequenceStep-1u]);
    m_strComparisonConditions.clear(); m_strFullConditions.clear(); m_strFailureReason.clear();
    m_CaptureCommonFields.clear();m_CaptureActualFields.clear();m_ChangedConditionFields.clear();
    m_bSourceMaterials = Engine::CGameInstance::Get().Get_MaterialRenderSettings().bUseSourceMaterials;
    m_bConditionsStable = true; m_bWarmup = true; m_bCapturing = true; m_pCaptureProfiler = profiler;
    status = "Warm-up, then sampling complete frames. Camera and scene must remain unchanged.";
    return true;
}

void Client::CRenderingBenchmark::Cancel_Capture(const string& reason, const bool restoreOptimization)
{
    if (m_bCapturing && m_pCaptureProfiler && m_pCaptureProfiler->Is_Enabled() && !m_bProfilerWasEnabled)
        m_pCaptureProfiler->Set_Enabled(false);
    m_bCapturing = false; m_bWarmup = false;
    m_strFailureReason = reason; m_strStatus = reason;
    Finish_Sequence();
    if(restoreOptimization && m_bOptimizationSession && m_bExperimentActive && m_pExperimentProfiles)
    {
        string restored;
        if(m_pExperimentProfiles->Clear_ExperimentPreview(restored))
        { m_bExperimentActive=false; m_pExperimentProfiles=nullptr; Reset_ComparisonRows(); m_strStatus=reason+" 원래 설정으로 복원했습니다."; }
        else m_strStatus=reason+" 복원 실패: "+restored;
    }
}

void Client::CRenderingBenchmark::Update(Engine::CProfiler* profiler)
{
    Poll_RawEvidence();
    Poll_Save();
    if(m_bEvidenceDrain)
    {
        if(m_RawExporter.IsSaving()) return;
        m_bEvidenceDrain=false;
        if(m_bSequence && m_bExperimentActive && profiler)
        {
            if(!Apply_ExperimentVariant(m_bEvidenceNextVariantB) ||
                !Begin(profiler,m_LabelBuffer.data(),static_cast<uint32_t>(m_iFrameInput),"session experiment",m_strStatus))
                Cancel_Capture("다음 단계 준비 실패: "+m_strStatus);
        }
        return;
    }
    if (!m_bCapturing || !profiler) return;
    if (!profiler->Is_Enabled()) { Cancel_Capture("Capture stopped by the Profiler; completed runs are preserved."); return; }
    Engine::FProfilerLiveStats live{};
    if (!profiler->Get_LiveStats(live)) return;
    if (live.FrameNumber < m_iLastObservedFrame)
    { Cancel_Capture("Profiler history was reset; restart this run."); return; }
    m_iLastObservedFrame = live.FrameNumber;
    if (m_bCaptureExperiment && (!m_bExperimentActive || !m_pExperimentProfiles || !m_pExperimentProfiles->Has_ExperimentPreview()))
    { Cancel_Capture("Experiment owner changed; this run was cancelled."); return; }
    if (m_bWarmup)
    {
        if (live.FrameNumber < m_iStartFrame) return;
        m_bWarmup = false;
        m_strComparisonConditions = Current_Conditions(m_iCaptureFields,&m_CaptureCommonFields);
        m_strFullConditions = Current_Conditions(0u,&m_CaptureActualFields);
        m_CaptureValues = CRenderingProfileService::Read_ExperimentValues();
        if(m_bOptimizationSession) m_OptimizationCaptureContext=Sample_OptimizationContext();
        m_bSourceMaterials = m_CaptureValues.values[static_cast<size_t>(ExperimentField::SOURCE_MATERIALS)] != 0;
        m_strStatus = "Sampling (no GPU waits).";
        if(m_bComparisonMeasurement)
        {
            auto found=m_ComparisonRows.find(m_strMeasurementRowId);
            if(found==m_ComparisonRows.end() || found->second.measurementId!=m_strMeasurementId)
            { Cancel_Capture("Row measurement ownership changed."); return; }
            auto& row=found->second;
            constexpr uint64_t allFields=(uint64_t{1}<<RENDERING_EXPERIMENT_FIELD_COUNT)-1u;
            const string scene=Current_Conditions(allFields,nullptr,true);
            if(row.sceneConditions.empty()) { row.sceneConditions=scene; row.commonConditions=m_strComparisonConditions; }
            if(row.sceneConditions!=scene || row.commonConditions!=m_strComparisonConditions ||
                !ComparisonAppliedValuesMatch(m_CaptureValues,m_bVariantB?row.b:row.a))
            {
                m_bConditionsStable=false;
                m_strFailureReason="반복 사이 장면 조건 또는 실제 적용값 변경";
            }
        }
    }
    if (m_bConditionsStable && live.FrameNumber <= m_iStartFrame + m_iTargetFrames && m_strFullConditions != Current_Conditions(0u))
    {
        m_bConditionsStable = false;
        std::map<string,string> changed;Current_Conditions(0u,&changed);
        m_ChangedConditionFields=ChangedConditionFields(m_CaptureActualFields,changed);
        m_strFailureReason="수집 중 조건 변경: ";size_t shown=0;
        for(const auto& [name,value]:m_ChangedConditionFields){if(shown++)m_strFailureReason+=", ";m_strFailureReason+=name;if(shown==6)break;}
        if(m_ChangedConditionFields.empty())m_strFailureReason+="추가 raw 입력 (이름 진단 범위 밖)";
        else if(m_ChangedConditionFields.size()>shown)m_strFailureReason+=" 외 "+std::to_string(m_ChangedConditionFields.size()-shown)+"개";
    }
    const uint64_t lastSample = m_iStartFrame + m_iTargetFrames;
    if (live.FrameNumber < lastSample + Engine::CProfiler::GPU_READ_LATENCY) return;
    const auto snapshot = profiler->Snapshot();
    if (snapshot.Frames.empty() || snapshot.Frames.front().FrameNumber > m_iStartFrame + 1u)
    { Cancel_Capture("Sample history was reset or evicted; completed runs are preserved."); return; }
    bool pending = false;
    for (const auto& frame : snapshot.Frames)
        if (frame.FrameNumber > m_iStartFrame && frame.FrameNumber <= lastSample &&
            frame.GpuStatus == Engine::EProfilerGpuFrameStatus::Pending) pending = true;
    // Polling is done by Engine. A bounded tail prevents a lost query from
    // keeping the session running forever; unresolved samples remain pending.
    if (pending && live.FrameNumber < lastSample + 64u) { m_strStatus = "Resolving pending GPU samples."; return; }
    const bool sequence = m_bSequence;
    if (!Finalize(*profiler))
    {
        if(m_bOptimizationSession)
        { const auto found=m_ComparisonRows.find(m_strMeasurementRowId);if(found!=m_ComparisonRows.end())found->second.failure=m_strStatus;Cancel_Capture(m_strStatus); }
        else Finish_Sequence();
        return;
    }
    if (sequence && m_bConditionsStable && m_bExperimentActive && ++m_iSequenceStep < m_iSequenceTotal)
    {
        // AB, BA alternation balances cache/order effects across repeats.
        const bool nextB = (m_iSequenceStep % 4u == 1u || m_iSequenceStep % 4u == 2u);
        if (m_bSweep)
        {
            m_ExperimentB = m_ExperimentA;
            m_ExperimentB.values[m_iSweepField] = m_SweepPoints[m_iSequenceStep-1u];
        }
        if(m_bOptimizationSession && m_RawExporter.IsSaving())
        {
            m_bEvidenceDrain=true; m_bEvidenceNextVariantB=m_bSweep || nextB;
            m_strStatus="단계 원본 증거 저장 중 · 다음 warmup은 저장 후 시작합니다.";
            return;
        }
        if (!Apply_ExperimentVariant(m_bSweep || nextB) ||
            !Begin(profiler, m_LabelBuffer.data(), static_cast<uint32_t>(m_iFrameInput), "session experiment", m_strStatus))
            Finish_Sequence();
    }
    else
    {
        Finish_Sequence();
        if(m_bOptimizationSession && !m_bConditionsStable) Cancel_Capture(m_strFailureReason);
    }
}

bool_t Client::CRenderingBenchmark::Finalize(Engine::CProfiler& profiler)
{
    m_bCapturing = false;
    auto snapshot = profiler.Snapshot();
    if (!m_bProfilerWasEnabled && !m_bSequence) profiler.Set_Enabled(false);
    RENDERING_BENCHMARK_RUN run;
    run.strLabel = m_strLabel; run.strTimestamp = Now_Timestamp(); run.strQualitySummary = m_strQualitySummary;
    run.strComparisonConditions = m_strComparisonConditions; run.strFullConditions = m_strFullConditions;
    run.commonConditionFields=m_CaptureCommonFields;run.actualConditionFields=m_CaptureActualFields;run.changedConditionFields=m_ChangedConditionFields;
    if(m_iPreparedRecipe>=0)
    {
        const auto& recipe=ExperimentRecipes[m_iPreparedRecipe];
        if(m_iCaptureFields==Client::RenderingExperimentBit(recipe.Field))
        {run.strRecipeId=recipe.Id;run.strExperimentGoal=recipe.Goal;run.strMetricGuide=recipe.Metrics;run.strConfidence=RecipeConfidence(recipe.Kind);}
    }
    if(m_strPreparedComparisonRowId==QualitySuiteRowId)
    {
        run.strRecipeId=QualitySuiteRowId;
        run.strExperimentGoal="시작 화면과 DX11 품질 확장 후보의 합성 차이";
        run.strMetricGuide="CPU/GPU frame ms와 Render.SSAO/SSGI/SSR/Copy, 동일 카메라의 표면·반사·가림";
        run.strConfidence="다변수 후보의 총 비용이며 개별 기법의 비용이나 화질 상승률이 아닙니다.";
    }
    run.strExperimentId = m_strCaptureExperimentId; run.strVariant = m_strCaptureVariant;
    run.fieldMask = m_iCaptureFields; run.warmupFrames = m_iCaptureWarmup; run.repetition = m_bSweep ? m_iSequenceStep+1u : m_iSequenceStep / 2u + 1u;
    run.appliedValues = m_CaptureValues; run.bSourceMaterials = m_bSourceMaterials;
    if(m_bComparisonMeasurement)
    {
        run.strComparisonRowId=m_strMeasurementRowId; run.strMeasurementId=m_strMeasurementId;
        run.strComparisonSessionId=m_strComparisonSessionId;
        const auto found=m_ComparisonRows.find(m_strMeasurementRowId);
        if(found!=m_ComparisonRows.end()) { run.expectedA=found->second.a; run.expectedB=found->second.b; }
    }
    run.bConditionsStable = m_bConditionsStable; run.strFailureReason = m_strFailureReason;
    vector<double> cpu, gpu, interval;
    std::map<string, RENDERING_BENCHMARK_PASS> passes, cpuPasses;
    uint32_t completeGpuScopes = 0;
    const auto count = [](const auto& frame, Engine::EProfilerCounter c) { return static_cast<double>(frame.Counters[static_cast<size_t>(c)]); };
    for (const auto& frame : snapshot.Frames)
    {
        if (frame.FrameNumber <= m_iStartFrame || frame.FrameNumber > m_iStartFrame + m_iTargetFrames) continue;
        if (!run.firstFrame) run.firstFrame = frame.FrameNumber;
        run.lastFrame = frame.FrameNumber; ++run.iFrames; cpu.push_back(frame.CpuFrameMs);
        // Interval belongs to the preceding frame; exclude the transition edge.
        if (frame.FrameNumber > m_iStartFrame + 1u && frame.FrameIntervalMs > 0) interval.push_back(frame.FrameIntervalMs);
        run.fDrawCallsAvg += count(frame,Engine::EProfilerCounter::DrawCalls);
        run.fInstancesAvg += count(frame,Engine::EProfilerCounter::Instances);
        run.fIndicesAvg += count(frame,Engine::EProfilerCounter::Indices);
        run.fMeshDrawsAvg += count(frame,Engine::EProfilerCounter::MeshDrawCalls);
        run.fUniqueMeshesAvg += count(frame,Engine::EProfilerCounter::UniqueMeshes);
        for(size_t i=0;i<run.counterTotals.size();++i) run.counterTotals[i]+=static_cast<double>(frame.Counters[i]);
        for(size_t i=0;i<run.workTotals.size();++i)
        { run.workTotals[i].CpuMs+=frame.CpuWork[i].CpuMs; run.workTotals[i].Calls+=frame.CpuWork[i].Calls; }
        run.droppedCpuScopes += frame.DroppedCpuScopes; run.droppedGpuScopes += frame.DroppedGpuScopes;
        if (frame.DetailedCpuScopes && !frame.DroppedCpuScopes && snapshot.TicksPerSecond)
        {
            ++run.iCpuScopeFrames;
            for (const auto& sample : frame.CpuScopes)
            {
                if (sample.NameId >= snapshot.ScopeNames.size() || sample.EndTick < sample.BeginTick) continue;
                const string key = snapshot.ScopeNames[sample.NameId] + " [thread " + std::to_string(sample.ThreadId) + "]";
                auto& pass = cpuPasses[key]; pass.name = key;
                pass.inclusiveMs += static_cast<double>(sample.EndTick-sample.BeginTick)*1000.0/snapshot.TicksPerSecond;
            }
        }
        if (frame.GpuStatus == Engine::EProfilerGpuFrameStatus::Pending) ++run.pendingGpuFrames;
        else if (!frame.GpuValid) ++run.invalidGpuFrames;
        if (!frame.GpuValid) continue;
        gpu.push_back(frame.GpuFrameMs); run.fPsInvocationsAvg += static_cast<double>(frame.Pipeline.PSInvocations);
        run.iaVerticesTotal+=static_cast<double>(frame.Pipeline.IAVertices);
        run.iaPrimitivesTotal+=static_cast<double>(frame.Pipeline.IAPrimitives);
        run.vsInvocationsTotal+=static_cast<double>(frame.Pipeline.VSInvocations);
        if (!frame.GpuScopesSupported || frame.DroppedGpuScopes) continue;
        ++completeGpuScopes;
        for (const auto& sample : frame.GpuScopes)
        {
            if (sample.NameId >= snapshot.ScopeNames.size()) continue;
            auto& pass = passes[snapshot.ScopeNames[sample.NameId]];
            pass.name = snapshot.ScopeNames[sample.NameId]; pass.inclusiveMs += sample.DurationMs;
            pass.selfMs += sample.SelfMs; pass.drawCalls += static_cast<double>(sample.Draw.DrawCalls);
            pass.indices += static_cast<double>(sample.Draw.Indices);
        }
    }
    if (run.iFrames != m_iTargetFrames || cpu.empty())
    { m_strStatus = "Incomplete CPU sample window; run was not saved."; return false; }
    const auto average = [](const vector<double>& v) { return v.empty() ? 0.0 : std::accumulate(v.begin(),v.end(),0.0)/v.size(); };
    run.fCpuAvgMs=average(cpu); run.fCpuP50Ms=Percentile(cpu,.5); run.fCpuP95Ms=Percentile(cpu,.95);
    run.fCpuP99Ms=Percentile(cpu,.99); run.fCpuMaxMs=*std::max_element(cpu.begin(),cpu.end());
    run.iGpuFrames=static_cast<uint32_t>(gpu.size()); run.fGpuAvgMs=average(gpu);
    run.fGpuP50Ms=Percentile(gpu,.5); run.fGpuP95Ms=Percentile(gpu,.95); run.fGpuP99Ms=Percentile(gpu,.99);
    if (!gpu.empty()) { run.fGpuMaxMs=*std::max_element(gpu.begin(),gpu.end()); run.fPsInvocationsAvg/=gpu.size(); }
    run.iIntervalFrames=static_cast<uint32_t>(interval.size()); run.fIntervalAvgMs=average(interval);
    run.fIntervalP50Ms=Percentile(interval,.5); run.fIntervalP95Ms=Percentile(interval,.95); run.fIntervalP99Ms=Percentile(interval,.99);
    if (!interval.empty()) run.fIntervalMaxMs=*std::max_element(interval.begin(),interval.end());
    const double n=run.iFrames;
    run.fDrawCallsAvg/=n; run.fInstancesAvg/=n; run.fIndicesAvg/=n; run.fMeshDrawsAvg/=n; run.fUniqueMeshesAvg/=n;
    run.iGpuScopeFrames = completeGpuScopes;
    for (auto& [name,pass] : cpuPasses)
    { pass.validFrames = run.iCpuScopeFrames; pass.inclusiveMs /= run.iCpuScopeFrames; run.cpuPasses.push_back(pass); }
    for (auto& [name,pass] : passes)
    {
        pass.validFrames=completeGpuScopes; pass.inclusiveMs/=completeGpuScopes; pass.selfMs/=completeGpuScopes;
        pass.drawCalls/=completeGpuScopes; pass.indices/=completeGpuScopes; run.passes.push_back(pass);
    }
    if (m_Runs.size() == 128u) { m_Runs.erase(m_Runs.begin()); m_iCompareFirst=m_iCompareSecond=-1; }
    const bool evidenceOk=!m_bOptimizationSession || Queue_RawEvidence(std::move(snapshot),run);
    m_Runs.push_back(std::move(run)); m_iCompareSecond=static_cast<int>(m_Runs.size()-1u);
    if(!evidenceOk) { m_strStatus=m_Runs.back().rawEvidenceStatus; m_bSaveQueued=true; return false; }
    if (m_bSweep)
    {
        for (size_t i=0;i+1<m_Runs.size();++i)
            if (m_Runs[i].strVariant=="A" && m_Runs[i].strExperimentId==m_strCaptureExperimentId &&
                m_Runs[i].fieldMask==m_iCaptureFields && m_Runs[i].strComparisonConditions==m_strComparisonConditions)
            {m_iCompareFirst=static_cast<int>(i);break;}
    }
    m_strStatus=m_bConditionsStable ? "Run recorded. JSON save queued." : "Run recorded as INVALID: "+m_strFailureReason;
    if (m_bSequence) m_bSaveQueued=true;
    else Queue_Save();
    return true;
}

void Client::CRenderingBenchmark::Queue_Save(const bool shutdown)
{
    if (m_SaveFuture.valid() || (!shutdown && m_RawExporter.IsSaving())) { m_bSaveQueued=true; return; }
    auto runs=m_Runs; const auto path=Make_DefaultPath(); m_bSaveQueued=false;
    try
    {
        m_SaveFuture=std::async(std::launch::async,[runs=std::move(runs),path]() {
            SAVE_RESULT result; string error;
            result.ok=Save_Json(runs,path,error); result.message=result.ok ? "JSON 저장: "+path.string() : "JSON 저장 실패: "+error;
            return result;
        });
        m_strSaveStatus="JSON 저장 중 (측정 thread와 분리)";
    }
    catch (const std::exception& error) { m_strSaveStatus=string("JSON 저장 시작 실패: ")+error.what(); }
}

void Client::CRenderingBenchmark::Poll_Save()
{
    if (!m_SaveFuture.valid())
    { if (m_bSaveQueued && !m_bCapturing && !m_bSequence) Queue_Save(); return; }
    if (m_SaveFuture.wait_for(std::chrono::seconds(0)) != std::future_status::ready) return;
    try { m_strSaveStatus=m_SaveFuture.get().message; }
    catch (const std::exception& error) { m_strSaveStatus=string("JSON 저장 실패: ")+error.what(); }
    if (m_bSaveQueued && !m_bCapturing && !m_bSequence) Queue_Save();
}

void Client::CRenderingBenchmark::Release_RestorationOwnership()
{
	m_strRestorationEntryProfileId.clear();
	m_strRestorationLastProfileId.clear();
	m_strRestorationLevelQualityId.clear();
	m_iRestorationLevel = 0u;
}

void Client::CRenderingBenchmark::Notify_ProfileReload()
{
    if (m_bExperimentActive) End_Experiment();
	if (!m_strRestorationLastProfileId.empty())
	{
		Release_RestorationOwnership();
		m_strRestorationStatus = "Runtime reloaded. Comparison ownership released; the reloaded scene stays active.";
	}
	if (m_bCapturing)
		m_bConditionsStable = false;
}

void Client::CRenderingBenchmark::Update_RestorationPreview(
	CRenderingProfileService& Profiles, const bool_t bToolVisible)
{
    m_bToolVisible=bToolVisible;
    const bool keepHidden=m_bOptimizationSession && m_bKeepOptimizationHidden;
    if (m_bExperimentActive && ((!bToolVisible && !keepHidden) || !Profiles.Has_ExperimentPreview() ||
        Profiles.Get_ProfileGeneration() != m_iExperimentProfileGeneration ||
        Engine::CGameInstance::Get().Get_CurrentLevelID() != m_iExperimentLevel))
        End_Experiment();
    if (!bToolVisible && !keepHidden && (m_bCapturing || m_bSequence)) Cancel_Capture("Workbench closed; capture cancelled, completed runs preserved.");
    auto& game = Engine::CGameInstance::Get();
    if (m_bPixelDiagnosticsActive && (!bToolVisible || game.Get_CurrentLevelID() != m_iPixelDiagnosticsLevel))
    {
        (void)game.Apply_MaterialRenderSettings(m_PixelEntrySettings);
        m_bPixelDiagnosticsActive = false;
        m_strPixelMaterialKey.clear();
    }
	if (!bToolVisible)
    {
        Profiles.Clear_ComparisonOptions();
		if (auto* arena = CLevel_KakulSaydonArena::Get_Active())
			arena->Reset_MapLightComparison();
    }
	if (m_strRestorationLastProfileId.empty())
		return;
	if (Engine::CGameInstance::Get().Get_CurrentLevelID() != m_iRestorationLevel ||
		Profiles.Get_ActiveProfileId() != m_strRestorationLastProfileId ||
		Profiles.Get_LevelQualityProfileId() != m_strRestorationLevelQualityId)
	{
		Release_RestorationOwnership();
		m_strRestorationStatus = "Level or scene owner changed. Comparison released without changing the new scene.";
		return;
	}
	if (!bToolVisible && !Return_ToEntryProfile(Profiles))
	{
		// A hidden tool must not retry a failing renderer transaction every frame.
		Release_RestorationOwnership();
		m_strRestorationStatus += " Comparison ownership released; the current scene is retained.";
	}
}

bool_t Client::CRenderingBenchmark::Activate_RestorationProfile(
	CRenderingProfileService& Profiles, const string& strProfileId)
{
	if (m_bCapturing)
		return false;
	Update_RestorationPreview(Profiles, true);
	const string previous = Profiles.Get_ActiveProfileId();
	if (previous.empty() || !Profiles.Has_Profile(previous))
	{
		m_strRestorationStatus = "Cannot compare without an available entry profile.";
		return false;
	}
	if (!Profiles.Activate_Profile(strProfileId, m_strRestorationStatus))
		return false;
	if (m_strRestorationLastProfileId.empty())
	{
		m_strRestorationEntryProfileId = previous;
		m_iRestorationLevel = Engine::CGameInstance::Get().Get_CurrentLevelID();
		m_strRestorationLevelQualityId = Profiles.Get_LevelQualityProfileId();
	}
	m_strRestorationLastProfileId = strProfileId;
	m_strRestorationStatus = "Session profile applied. Return to entry or close this workbench to restore the entry profile.";
	return true;
}

bool_t Client::CRenderingBenchmark::Return_ToEntryProfile(CRenderingProfileService& Profiles)
{
	auto* arena = CLevel_KakulSaydonArena::Get_Active();
	const bool_t lightsChanged = arena &&
		arena->Get_MapLightComparison() != CLevel_KakulSaydonArena::MAP_LIGHT_COMPARISON::CURRENT;
	if (m_strRestorationLastProfileId.empty())
	{
		if (arena) arena->Reset_MapLightComparison();
		if (lightsChanged) m_strRestorationStatus = "Current map lights restored.";
		return lightsChanged;
	}
	if (Engine::CGameInstance::Get().Get_CurrentLevelID() != m_iRestorationLevel ||
		Profiles.Get_ActiveProfileId() != m_strRestorationLastProfileId ||
		Profiles.Get_LevelQualityProfileId() != m_strRestorationLevelQualityId)
	{
		Release_RestorationOwnership();
		m_strRestorationStatus = "The scene is now owned elsewhere. Entry restoration was skipped.";
		return false;
	}
	if (!Profiles.Activate_Profile(m_strRestorationEntryProfileId, m_strRestorationStatus))
		return false;
	if (arena) arena->Reset_MapLightComparison();
	Release_RestorationOwnership();
	if (m_bCapturing)
		m_bConditionsStable = false;
	m_strRestorationStatus = "Entry profile restored.";
	return true;
}

bool_t Client::CRenderingBenchmark::Render_RestorationSection(CRenderingProfileService& Profiles)
{
	Update_RestorationPreview(Profiles, true);
	ImGui::SeparatorText("Rendering restoration");
	const auto& game = Engine::CGameInstance::Get();
	const char* beforeId = nullptr;
	const char* restoredId = nullptr;
	switch (static_cast<LEVEL>(game.Get_CurrentLevelID()))
	{
	case LEVEL::BERN:
		beforeId = "scene.bern.before-restoration.v1";
		restoredId = "scene.bern.source-rendering.v1";
		break;
	case LEVEL::CHARACTER_SELECT:
		beforeId = "scene.character-select.before-restoration.v1";
		restoredId = "scene.character-select.source-rendering.v1";
		break;
	case LEVEL::VALTAN_ARENA:
		beforeId = "scene.valtan.before-restoration.v1";
		restoredId = "scene.valtan.source-rendering.v1";
		break;
	case LEVEL::KAKULSAYDON_ARENA:
		beforeId = "scene.kakulsaydon.before-restoration.v1";
		restoredId = "scene.kakulsaydon.source-rendering.v1";
		break;
	default:
		break;
	}
	bool_t changed = false;
	if (auto* arena = CLevel_KakulSaydonArena::Get_Active();
		arena && game.Get_CurrentLevelID() == ETOUI(LEVEL::KAKULSAYDON_ARENA))
	{
		struct AREA_PROFILE final { const char* label; const char* id; };
		static constexpr AREA_PROFILE areaProfiles[] = {
			{ "Start area - source", "scene.kakulsaydon.compare.start.v1" },
			{ "Gate 1 - source LUT02", "scene.kakulsaydon.compare.gate1.v1" },
			{ "Gate 2 - source", "scene.kakulsaydon.compare.gate2.v1" },
			{ "Gate 3 - source LUT01", "scene.kakulsaydon.compare.gate3.v1" },
			{ "Card maze - current baseline", "scene.kakulsaydon.compare.card-maze.v1" }
		};
		const char* areaLabel = "Current scene";
		for (const auto& item : areaProfiles)
			if (Profiles.Get_ActiveProfileId() == item.id) areaLabel = item.label;
		ImGui::BeginDisabled(m_bCapturing);
		if (ImGui::BeginCombo("Kouku area profile", areaLabel))
		{
			for (const auto& item : areaProfiles)
			{
				const bool available = Profiles.Has_Profile(item.id);
				ImGui::BeginDisabled(!available);
				if (ImGui::Selectable(item.label, Profiles.Get_ActiveProfileId() == item.id))
					changed = Activate_RestorationProfile(Profiles, item.id) || changed;
				ImGui::EndDisabled();
				if (!available && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
					ImGui::SetTooltip("This comparison profile has not been published.");
			}
			ImGui::EndCombo();
		}
		ImGui::EndDisabled();
		ImGui::TextWrapped("Applies a fixed area look to this view. Player position and raid state stay where they are. Card maze keeps its current baseline.");
		int selection = static_cast<int>(arena->Get_MapLightComparison());
		ImGui::BeginDisabled(m_bCapturing);
		if (ImGui::Combo("Map light comparison", &selection,
			"Current authored lights\0Imported source lights (2026-09-11)\0Map lights off\0"))
			changed = arena->Set_MapLightComparison(
				static_cast<CLevel_KakulSaydonArena::MAP_LIGHT_COMPARISON>(selection), m_strRestorationStatus);
		ImGui::EndDisabled();
		ImGui::TextWrapped("Source comparison uses 115 imported local lights with current receiver routing and gate placement. Scene and Effect lights remain active.");
		ImGui::TextWrapped("This session choice preserves authored lights. Closing this workbench restores them. Compare tone and grading with the profiles below.");
	}
	if (beforeId)
	{
		const bool beforeAvailable = Profiles.Has_Profile(beforeId);
		const bool restoredAvailable = Profiles.Has_Profile(restoredId);
		const bool beforeActive = Profiles.Get_ActiveProfileId() == beforeId;
		const bool restoredActive = Profiles.Get_ActiveProfileId() == restoredId;
		const bool canReturnFromBefore = beforeActive && !m_strRestorationEntryProfileId.empty();
		ImGui::BeginDisabled(m_bCapturing || (canReturnFromBefore ? false : !beforeAvailable || beforeActive));
		if (ImGui::Button(canReturnFromBefore ? "Return from before-restoration.v1" : "before-restoration.v1"))
			changed = (canReturnFromBefore ? Return_ToEntryProfile(Profiles) :
				Activate_RestorationProfile(Profiles, beforeId)) || changed;
		ImGui::EndDisabled();
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			ImGui::SetTooltip("%s", canReturnFromBefore ? m_strRestorationEntryProfileId.c_str() : beforeId);
		ImGui::SameLine();
		ImGui::BeginDisabled(m_bCapturing || !restoredAvailable || restoredActive);
		if (ImGui::Button(restoredActive ? "Restored source profile (active)" : "Restored source profile"))
			changed = Activate_RestorationProfile(Profiles, restoredId) || changed;
		ImGui::EndDisabled();
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			ImGui::SetTooltip("%s", restoredId);
		ImGui::TextWrapped("Baseline%s: %s", beforeActive ? " (active)" : "", beforeId);
		ImGui::TextWrapped("Restored profile: %s", restoredId);
		ImGui::TextWrapped("Switches saved scene lighting, environment, shadow, fog and post-process settings. Material equations and render passes keep the current code; map-light placements and Effect data keep their current assets.");
		if (Profiles.Get_ComparisonOptions().bActive)
			ImGui::TextWrapped("Live rendering comparison is still active. Use Reset comparison below to view this profile without those overrides.");
		if (!beforeAvailable || !restoredAvailable)
			ImGui::TextWrapped("A comparison profile is unavailable. Source applicability and published inputs must be confirmed for this map.");
	}
	else
		ImGui::TextWrapped("Source rendering applicability for this map is not confirmed. Comparison profiles are unavailable.");
	const auto* arena = CLevel_KakulSaydonArena::Get_Active();
	const bool lightsCompared = arena &&
		arena->Get_MapLightComparison() != CLevel_KakulSaydonArena::MAP_LIGHT_COMPARISON::CURRENT;
	ImGui::BeginDisabled(m_bCapturing || (m_strRestorationLastProfileId.empty() && !lightsCompared));
	if (ImGui::Button("Return to entry"))
		changed = Return_ToEntryProfile(Profiles) || changed;
	ImGui::EndDisabled();
	if (!m_strRestorationEntryProfileId.empty())
		ImGui::TextWrapped("Entry profile: %s", m_strRestorationEntryProfileId.c_str());
	ImGui::TextWrapped("Active profile: %s", Profiles.Get_ActiveProfileId().c_str());
	const auto& regionId = Profiles.Get_AppliedEnvironmentRegionId();
	ImGui::TextWrapped("Camera environment: %s", regionId.empty() ? "fixed profile (no region override)" : regionId.c_str());
	if (const auto* camera = Engine::CGameInstance::Get().Get_CamPosition())
		ImGui::Text("Camera XYZ %.3f / %.3f / %.3f", camera->x, camera->y, camera->z);
	const auto quality = game.Get_RenderQualitySettings();
	const auto fog = game.Get_HeightFogSettings();
	ImGui::Text("Exposure %.4f | Bloom %s: %.4f", quality.fExposure,
		quality.bBloomEnabled ? "on" : "off", quality.fBloomIntensity);
	ImGui::Text("Bloom threshold %.4f | Desaturation %.4f",
		quality.fBloomThreshold, quality.fSceneDesaturation);
	const auto& source = quality.SourcePostProcess;
	ImGui::Text("Tone mapping: %s", source.bEnabled ? "Source UE3 customizable" : "Hable");
	if (source.bEnabled)
	{
		ImGui::Text("Source tone scale %.4f | range %.4f | toe %.4f | desaturation %.4f",
			source.fToneScale, source.fToneRange, source.fToneToe, source.fDesaturation);
		for (const auto& layer : source.LutLayers)
			ImGui::TextWrapped("LUT %.3f: %s", layer.fWeight,
				layer.pLut ? layer.pLut->strAssetId.c_str() : "neutral");
	}
	ImGui::Text("Bloom tint RGB %.3f / %.3f / %.3f",
		quality.vBloomTint.x, quality.vBloomTint.y, quality.vBloomTint.z);
	ImGui::Text("Fog %s (%s) | density %.5f", fog.bEnabled ? "on" : "off",
		fog.bSourceExponential ? "source exponential" : "project height", fog.fDensity);
	for (const auto& light : game.Get_SceneLights())
	{
		if (light.eType != Engine::LIGHT::DIRECTIONAL)
			continue;
		ImGui::Text("Directional RGB %.3f / %.3f / %.3f",
			light.vDiffuse.x, light.vDiffuse.y, light.vDiffuse.z);
		ImGui::Text("Ambient RGB %.3f / %.3f / %.3f",
			light.vAmbient.x, light.vAmbient.y, light.vAmbient.z);
		ImGui::Text("Directional specular RGB %.3f / %.3f / %.3f",
			light.vSpecular.x, light.vSpecular.y, light.vSpecular.z);
		break;
	}
	ImGui::TextWrapped("%s", m_strRestorationStatus.c_str());
	ImGui::TextWrapped("Native inputs: recovered scene lights, baked RNM and source fog where verified. Receiver separation remains part of the map.");
	ImGui::TextWrapped("Source profiles use the recovered tone curve and LUT grading when enabled. Other profiles retain their saved tone mapping.");
	ImGui::TextWrapped("Bloom kernel, DOF, light shafts and map effects have separate restoration scopes; this comparison does not certify the whole scene.");
	ImGui::TextDisabled("Session only. No automatic Save or Publish. Profile switching is locked during capture.");
	return changed;
}

bool_t Client::CRenderingBenchmark::Render_PixelInputs()
{
    auto& game = Engine::CGameInstance::Get();
    auto settings = game.Get_MaterialRenderSettings();
    ImGui::SeparatorText("Pixel rendering inputs");
    ImGui::TextWrapped("Compare the whole scene first, then inspect a material contribution. Values below are the active shader inputs; textures still vary per pixel.");
    static constexpr const char* views[] = {
        "Final image", "Material base color", "Material normal", "Direct specular",
        "Reflection texture delta", "PBR roughness", "PBR metallic", "PBR material AO",
        "PBR baked diffuse (RNM)", "PBR environment specular", "PBR diffuse light (direct + unbaked ambient)",
        "Whole scene: HDR before final tone", "Whole scene: tone before grading",
        "Whole scene: grading before FXAA", "PBR cube diffuse sky (project approximation)"
    };
    static_assert(std::size(views) == static_cast<size_t>(Engine::MATERIAL_DEBUG_VIEW::END));
    ImGui::BeginDisabled(m_bCapturing);
    int view = static_cast<int>(settings.eDebugView);
    bool changed = ImGui::Combo("Pipeline view", &view, views, static_cast<int>(std::size(views)));
    settings.eDebugView = static_cast<Engine::MATERIAL_DEBUG_VIEW>(view);
    changed |= ImGui::Checkbox("Recovered material equations", &settings.bUseSourceMaterials);
    if (view >= 11 && view <= 13)
        ImGui::TextWrapped("Whole-scene views include characters and background. HDR uses one RGB scale for display; tone and grading views use the current scene settings. These are stage comparisons, not color-corrected presets.");
    else if (view >= 8)
        ImGui::TextWrapped("PBR contribution only. Other material families appear black. RNM and environment views precede screen AO and moving-caster shadow modulation.");

    if (ImGui::CollapsingHeader("PBR contribution comparison", ImGuiTreeNodeFlags_DefaultOpen))
    {
        changed |= ImGui::Checkbox("Enable comparison in this Level", &settings.MapPBR.bEnabled);
        settings.MapPBR.iLevel = game.Get_CurrentLevelID();
        ImGui::BeginDisabled(!settings.MapPBR.bEnabled || !settings.bUseSourceMaterials);
        auto& gains = settings.MapPBR.vContributionScale;
        auto& surface = settings.MapPBR.vSurfaceParameters;
        changed |= ImGui::SliderFloat("Diffuse lighting contribution", &gains.x, 0.f, 4.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        changed |= ImGui::SliderFloat("Direct specular contribution", &gains.y, 0.f, 4.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        changed |= ImGui::SliderFloat("Baked diffuse contribution (RNM)", &gains.z, 0.f, 4.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        changed |= ImGui::SliderFloat("Environment specular contribution", &gains.w, 0.f, 4.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        const bool nativeIndirect = game.Get_RenderEnvironment().bUseSourcePBRIndirect;
        ImGui::BeginDisabled(nativeIndirect);
        changed |= ImGui::SliderFloat("Cube diffuse sky contribution", &settings.MapPBR.fCubeDiffuseScale, 0.f, 4.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::EndDisabled();
        if (nativeIndirect) ImGui::TextWrapped("원본 PBR 간접광 ON: project cube diffuse는 소비되지 않습니다. 위 원본 간접광 recipe로 경로 자체를 명시적으로 비교하세요.");
        changed |= ImGui::SliderFloat("Normal strength multiplier", &surface.x, 0.f, 4.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        changed |= ImGui::SliderFloat("Roughness offset", &surface.y, -1.f, 1.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        bool legacy = surface.z != 0.f;
        if (ImGui::Checkbox("Compare previous RNM without energy split", &legacy))
        {
            surface.z = legacy ? 1.f : 0.f;
            changed = true;
        }
        ImGui::EndDisabled();
        if (ImGui::Button("Reset pixel comparison"))
        {
            settings = m_bPixelDiagnosticsActive ? m_PixelEntrySettings : Engine::MATERIAL_RENDER_SETTINGS{};
            changed = true;
        }
        ImGui::TextWrapped("These controls affect supported PBR map receivers; source character shaders keep their own equations. Native SH and hemisphere inputs use the existing source-indirect path where authored. Missing inputs are not synthesized by these sliders.");
        const auto sky = game.Get_RenderEnvironment();
        ImGui::Text("Cube diffuse sky profile intensity: %.3f", sky.fDiffuseIntensity);
        ImGui::TextWrapped("Project cube diffuse uses the scene RGBM cube projection before fog with material/screen AO. It is a separate approximation and is inactive while native source PBR indirect is ON. Its contribution slider does not control native SH or hemisphere lighting.");
        ImGui::TextDisabled("Diffuse lighting also includes the ambient fallback on surfaces without baked lighting.");
        ImGui::TextDisabled("Session only. Closing the workbench or changing Level clears this comparison. No Save or Publish.");
    }
    ImGui::EndDisabled();
    if (changed)
    {
        const auto previous = game.Get_MaterialRenderSettings();
        if (FAILED(game.Apply_MaterialRenderSettings(settings)))
            m_strRestorationStatus = "Invalid pixel comparison input; previous renderer state preserved.";
        else
        {
            if (!m_bPixelDiagnosticsActive) m_PixelEntrySettings = previous;
            m_bPixelDiagnosticsActive = true;
            m_iPixelDiagnosticsLevel = game.Get_CurrentLevelID();
        }
    }

    const auto number = [](const char* name, double value, const char* use = "active") {
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(name);
        ImGui::TableSetColumnIndex(1); ImGui::Text("%.6g", value);
        ImGui::TableSetColumnIndex(2); ImGui::TextUnformatted(use);
    };
    const auto vec = [](const char* name, const auto& v, const char* use = "active") {
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(name);
        ImGui::TableSetColumnIndex(1); ImGui::Text("%.6g / %.6g / %.6g", v.x, v.y, v.z);
        ImGui::TableSetColumnIndex(2); ImGui::TextUnformatted(use);
    };
    const auto table = [](const char* id) {
        if (!ImGui::BeginTable(id, 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_Resizable)) return false;
        ImGui::TableSetupColumn("Input"); ImGui::TableSetupColumn("Applied value");
        ImGui::TableSetupColumn("Use"); ImGui::TableHeadersRow(); return true;
    };
    if (ImGui::CollapsingHeader("Whole-scene composition inputs", ImGuiTreeNodeFlags_DefaultOpen) && table("ActiveScenePixelInputs"))
    {
        const auto q = game.Get_RenderQualitySettings();
        const auto& tone = q.SourcePostProcess;
        number("Exposure", q.fExposure);
        number("Gamma", q.fGamma, tone.bEnabled ? "source LUT bake exponent = 2.2 / gamma" : "display exponent = 1 / gamma");
        number("Hable white point", q.fWhitePoint, tone.bEnabled ? "inactive: source tone" : "active");
        const char* source = tone.bEnabled ? "active" : "inactive: Hable tone";
        number("Source tone scale", tone.fToneScale, source); number("Source tone range", tone.fToneRange, source);
        number("Source tone toe", tone.fToneToe, source); number("Source desaturation", tone.fDesaturation, source);
        vec("Source shadows", tone.vShadows, source); vec("Source highlights", tone.vHighlights, source);
        vec("Source midtones", tone.vMidtones, source); vec("Source colorize", tone.vColorize, source);
        number("Grading LUT layers", static_cast<double>(tone.LutLayers.size()), source);
        for (size_t i = 0; i < tone.LutLayers.size(); ++i)
        {
            const auto& layer = tone.LutLayers[i];
            number(("LUT weight " + std::to_string(i)).c_str(), layer.fWeight,
                layer.pLut ? layer.pLut->strAssetId.c_str() : "neutral lattice");
        }
        number("Display desaturation", q.fSceneDesaturation, tone.bEnabled ? "inactive: source grading" : "active");
        const char* bloom = q.bBloomEnabled ? "active" : "disabled";
        number("Bloom enabled", q.bBloomEnabled); number("Bloom intensity", q.fBloomIntensity, bloom);
        number("Bloom threshold", q.fBloomThreshold, bloom); number("Bloom soft knee", q.fBloomSoftKnee, bloom);
        number("Bloom scatter", q.fBloomScatter, bloom); vec("Bloom RGB tint", q.vBloomTint, bloom);
        const char* ao = q.bSSAOEnabled ? "active" : "disabled";
        number("SSAO enabled", q.bSSAOEnabled); number("SSAO radius (m)", q.fSSAORadius, ao);
        number("SSAO bias (m)", q.fSSAOBias, ao); number("SSAO intensity", q.fSSAOIntensity, ao);
        number("SSAO power", q.fSSAOPower, ao); number("SSAO distance fade", q.fSSAODistanceFade, ao);
        const char* aa = q.bFXAAEnabled ? "active" : "disabled";
        number("FXAA enabled", q.bFXAAEnabled); number("FXAA subpixel", q.fFXAASubpixel, aa);
        number("FXAA edge threshold", q.fFXAAEdgeThreshold, aa); number("FXAA minimum threshold", q.fFXAAEdgeThresholdMin, aa);
        number("Color filter type", q.iColorFilterType, tone.bEnabled ? "inactive: source return path" : "active");
        number("Color filter strength", q.fColorFilterStrength, tone.bEnabled ? "inactive: source return path" : "active");
        const auto env = game.Get_RenderEnvironment();
        vec("Scene environment RGB", env.vColor, env.pCube ? "bound scene cube; map MICs may use per-material cube" : "no scene cube");
        number("Scene environment floor", env.vColor.w);
        vec("Scene environment rotation/intensity", env.vRotationIntensity);
        const auto shadow = game.Get_ShadowLightDesc().Settings;
        const char* sh = shadow.bEnabled ? "active" : "disabled";
        number("Shadow enabled", shadow.bEnabled); number("Shadow depth bias", shadow.fDepthBias, sh);
        number("Shadow normal bias (m)", shadow.fNormalBias, sh); number("Shadow strength", shadow.fStrength, sh);
        number("Moving shadow on baked PBR", shadow.fDynamicBakedStrength, sh);
        number("Shadow coverage width (m)", shadow.fOrthographicWidth, sh);
        number("Shadow coverage height (m)", shadow.fOrthographicHeight, sh);
        number("Shadow near depth (m)", shadow.fNear, sh); number("Shadow far depth (m)", shadow.fFar, sh);
        const auto fog = game.Get_HeightFogSettings();
        const char* f = fog.bEnabled ? "active" : "disabled";
        number("Fog enabled", fog.bEnabled); number("Source exponential fog", fog.bSourceExponential, f);
        number("Fog density", fog.fDensity, f); number("Fog height falloff", fog.fHeightFalloff, f);
        number("Fog top height (m)", fog.fTopHeight, f); number("Fog start distance (m)", fog.fStartDistance, f);
        number("Fog maximum opacity", fog.fMaximumOpacity, f); vec("Fog color", fog.vColor, f);
        vec("Source fog inscattering", fog.vInscatteringColor, f); vec("Source fog light direction", fog.vFogLightDirection, f);
        number("Fog light terminator cosine", fog.vFogLightDirection.w, f);
        const char* drift = !fog.bEnabled ? "disabled" : (fog.bSourceExponential ? "inactive: source exponential" : "project height fog");
        number("Fog drift speed", fog.fDriftSpeed, drift); number("Fog drift height", fog.fDriftHeightAmplitude, drift);
        number("Fog drift density", fog.fDriftDensityAmplitude, drift); number("Fog coverage", fog.fCoveragePercent, drift);
        number("Fog wind X", fog.fWindDirectionX, drift); number("Fog wind Z", fog.fWindDirectionZ, drift);
        number("Fog wind speed", fog.fWindSpeed, drift); number("Fog patch scale", fog.fPatchScale, drift);
        number("Fog patch softness", fog.fPatchSoftness, drift);
        ImGui::EndTable();
    }
    if (ImGui::CollapsingHeader("Active scene light inputs") && table("SceneLightPixelInputs"))
    {
        size_t index = 0;
        for (const auto& light : game.Get_SceneLights())
        {
            const string prefix = "Light " + std::to_string(index++) + " ";
            number((prefix + "type").c_str(), static_cast<uint32_t>(light.eType));
            vec((prefix + "diffuse RGB").c_str(), light.vDiffuse);
            vec((prefix + "ambient RGB").c_str(), light.vAmbient, "baked PBR excludes duplicate ambient");
            vec((prefix + "character ambient RGB").c_str(), light.vSourceCharacterAmbient, "explicit character receiver input; not source SH");
            number((prefix + "character ambient override").c_str(), light.vSourceCharacterAmbient.w);
            vec((prefix + "specular RGB").c_str(), light.vSpecular, "native PBR uses incoming diffuse RGB instead");
            vec((prefix + "direction").c_str(), light.vDirection);
            vec((prefix + "position (m)").c_str(), light.vPosition);
            number((prefix + "range (m)").c_str(), light.fRange);
            number((prefix + "falloff exponent").c_str(), light.fFalloffExponent);
            number((prefix + "spot inner cosine").c_str(), light.fSpotInnerCos);
            number((prefix + "spot outer cosine").c_str(), light.fSpotOuterCos);
            number((prefix + "receiver").c_str(), static_cast<uint32_t>(light.eReceiver));
            number((prefix + "static shadow channel").c_str(), light.staticShadowChannel);
        }
        ImGui::EndTable();
    }
    if (ImGui::CollapsingHeader("Bound PBR material inputs"))
    {
        auto bindings = CMapAssetRenderUtils::Get_RecentSurfaceBindings();
        bindings.erase(std::remove_if(bindings.begin(), bindings.end(), [](const auto& row) {
            return row.activeProgram != 3u && row.activeProgram != 4u;
        }), bindings.end());
        const auto key = [](const auto& row) { return row.assetId + "|" + row.materialName; };
        auto selected = std::find_if(bindings.begin(), bindings.end(), [&](const auto& row) { return key(row) == m_strPixelMaterialKey; });
        if (selected == bindings.end() && !bindings.empty())
        {
            selected = bindings.begin();
            m_strPixelMaterialKey = key(*selected);
        }
        if (selected == bindings.end())
            ImGui::TextDisabled("No PBR surface bound in the last second. This panel samples successful draw bindings while open.");
        else
        {
            if (ImGui::BeginCombo("Inspect one bound surface", selected->materialName.c_str()))
            {
                for (const auto& row : bindings)
                {
                    const auto id = key(row);
                    ImGui::PushID(id.c_str());
                    if (ImGui::Selectable(row.materialName.c_str(), id == m_strPixelMaterialKey)) m_strPixelMaterialKey = id;
                    ImGui::PopID();
                }
                ImGui::EndCombo();
                selected = std::find_if(bindings.begin(), bindings.end(), [&](const auto& row) { return key(row) == m_strPixelMaterialKey; });
            }
            if (selected != bindings.end() && table("BoundMaterialPixelInputs"))
            {
                const auto& s = selected->surface;
                const auto& l = selected->lighting;
                const char* bound = "bound source constant";
                number("Source program", selected->activeProgram); vec("Base RGB multiplier", s.diffuseColor, bound);
                number("Base brightness", s.diffuseBrightness, bound); number("Base saturation", s.diffuseSaturation, bound);
                number("Base texture sRGB decode", s.diffuseSRGB); number("UV tile U", s.uvTiling.x); number("UV tile V", s.uvTiling.y);
                number("Normal intensity", s.normalIntensity); number("Detail normal intensity", s.detailNormalIntensity);
                number("Detail normal tiling", s.detailNormalTiling); number("Fixed normal UV", s.uvFixedNormal);
                number("Vertex normal weight", s.vertexAlpha); number("Masked alpha", s.pbrAlphaMasked);
                number("Alpha cutoff", .3333, s.pbrAlphaMasked ? "active source constant" : "inactive opaque material");
                number("Roughness intensity", s.roughnessIntensity); number("Roughness power", s.roughnessPower);
                number("Minimum roughness", s.minimumRoughness); number("Metallic intensity", s.metallicIntensity);
                number("Metallic power", s.metallicPower); number("AO intensity", s.aoIntensity); number("AO power", s.aoPower);
                number("ORM texture sRGB decode", s.ormSRGB); number("Dielectric specular intensity", s.specularPBRIntensity);
                number("Dielectric F0", .08 * std::clamp(s.specularPBRIntensity, 0.f, 1.f), "metallic pixels interpolate toward albedo");
                number("Nonmetallic brightness", s.nonmetallicBrightness); number("Metallic brightness", s.metallicBrightness);
                number("Reflection texture intensity", s.reflectionIntensity); number("Reflection contrast", s.reflectionContrast);
                number("Reflection tiling", s.reflectionTiling); vec("Reflection RGB", s.reflectionColor);
                number("World reflection UV", s.useWorldReflection); number("Reflection offset U", s.reflectionOriginOffset.x);
                number("Reflection offset V", s.reflectionOriginOffset.y); number("Reflection texture sRGB decode", s.reflectionSRGB);
                number("RNM texture binding enabled", s.hasBakedLighting);
                const char* placement = l.averageScale.w != 0.f ? "last bound placement" : "per-instance stream or absent; not sampled here";
                vec("RNM average scale", l.averageScale, placement); vec("RNM directional scale", l.directionalScale, placement);
                number("RNM UV scale U", l.scaleBias.x, placement); number("RNM UV scale V", l.scaleBias.y, placement);
                number("RNM UV offset U", l.scaleBias.z, placement); number("RNM UV offset V", l.scaleBias.w, placement);
                number("RNM sRGB decode", s.bakedLightingSRGB);
                number("Environment cube / BRDF lookup bound", s.hasEnvironmentCube);
                number("Native PBR indirect inputs available", s.hasSourceIndirect);
                number("Native PBR indirect profile enabled", CGameInstance::Get().Get_RenderEnvironment().bUseSourcePBRIndirect);
                if (s.hasSourceIndirect)
                {
                    vec("Native environment color", s.sourceIndirectColor);
                    number("Native rotation sine", s.sourceIndirectRotation.x);
                    number("Native rotation cosine", s.sourceIndirectRotation.y);
                    vec("Native ambient / sky factor", s.sourceAmbientAndSkyFactor);
                    number("Native upper sky R", s.sourceUpperSkyColor.x); number("Native upper sky G", s.sourceUpperSkyColor.y);
                    number("Native upper sky B", s.sourceUpperSkyColor.z); number("Native lower sky R", s.sourceLowerSkyColor.x);
                    number("Native lower sky G", s.sourceLowerSkyColor.y); number("Native lower sky B", s.sourceLowerSkyColor.z);
                }
                vec("Material environment RGB", s.environmentColor); number("Material environment floor", s.environmentColor.w);
                number("Environment rotation A", s.environmentRotation.x); number("Environment rotation B", s.environmentRotation.y);
                number("RGBM decode range", 6., "source shader constant"); number("Reflection mip scale", 5., "roughness AA * scale");
                number("Roughness derivative AA", .3, "source shader constant");
                number("Static shadow bound", s.hasStaticShadow); number("Static shadow channel", s.staticShadowChannel);
                vec("Static shadow bias / scale / power", s.staticShadowTransfer);
                const char* emissive = s.hasEmissive ? "active" : "disabled";
                number("Emission enabled", s.hasEmissive); vec("Emission RGB", s.emissiveColor, emissive);
                number("Emission intensity", s.emissiveIntensity, emissive);
                number("Emission tile U", s.emissiveUVTiling.x, emissive); number("Emission tile V", s.emissiveUVTiling.y, emissive);
                number("Emission flicker minimum", s.emissiveFlickerMinimum, emissive);
                number("Emission flicker speed", s.emissiveFlickerSpeed, emissive); number("Emission phase", s.emissivePhaseOffset, emissive);
                number("Emission sRGB decode", s.emissiveSRGB, emissive);
                ImGui::EndTable();
                ImGui::TextWrapped("Native source PBR uses restored SH/hemisphere and native 128x32 BRDF inputs where bound; the legacy path retains project approximations. Bound inputs are not GPU pixel readbacks.");
            }
        }
    }
    return changed;
}

uint64_t Client::CRenderingBenchmark::Experiment_FieldMask() const
{
    if (m_bSweep) return uint64_t{1} << m_iSweepField;
    uint64_t mask=0;
    for (size_t i=0;i<RENDERING_EXPERIMENT_FIELD_COUNT;++i)
        if (m_ExperimentA.values[i] != m_ExperimentB.values[i]) mask |= uint64_t{1}<<i;
    return mask;
}

uint64_t Client::CRenderingBenchmark::Experiment_BaselineMask() const
{
    uint64_t mask=0;
    for(size_t i=0;i<RENDERING_EXPERIMENT_FIELD_COUNT;++i)
        if(m_ExperimentA.values[i]!=m_ExperimentOriginal.values[i])mask|=uint64_t{1}<<i;
    return mask;
}

bool_t Client::CRenderingBenchmark::Adopt_BaselineFromB()
{
    if(!m_bExperimentActive || m_bCapturing || !m_pExperimentProfiles)return false;
    uint64_t ownership=0;
    for(size_t i=0;i<RENDERING_EXPERIMENT_FIELD_COUNT;++i)
        if(m_ExperimentB.values[i]!=m_ExperimentOriginal.values[i])ownership|=uint64_t{1}<<i;
    if(!m_pExperimentProfiles->Set_ExperimentPreview(m_ExperimentB,ownership,m_strStatus))return false;
    m_ExperimentA=m_ExperimentB;m_bVariantB=false;m_iPreparedRecipe=-1;m_iPresentationStage=-1;
    static uint64_t revision=0;
    m_strExperimentId="experiment.rebase."+std::to_string(GetCurrentProcessId())+"."+
        std::to_string(GetTickCount64())+"."+std::to_string(++revision);
    Reset_ComparisonRows();
    m_strStatus="현재 B를 새 A로 채택했습니다. 새 실험 ID로 이전 기준 결과와 분리하며 종료 시 원래 장면으로 복원합니다.";
    return true;
}

bool_t Client::CRenderingBenchmark::Start_SessionExperiment(CRenderingProfileService& profiles)
{
    if (m_bCapturing || m_bSequence || m_bSweep)
    { m_strStatus="측정·반복·sweep를 종료한 뒤 세션 실험을 시작하세요. 현재 설정을 유지합니다."; return false; }
    Update_RestorationPreview(profiles,true);
    if (m_bExperimentActive) return true;
    if (!m_strRestorationLastProfileId.empty())
    { m_strStatus="Rendering restoration에서 Return to entry를 누른 뒤 시작하세요. 현재 비교 profile은 유지합니다."; return false; }
    if (m_bPixelDiagnosticsActive)
    { m_strStatus="픽셀 진단을 원래 화면으로 복귀한 뒤 비교를 시작하세요."; return false; }
    if (profiles.Get_ComparisonOptions().bActive || profiles.Has_ExperimentPreview())
    { m_strStatus="기존 Live rendering comparison을 Reset한 뒤 시작하세요. 다른 비교의 설정을 자동 해제하지 않습니다."; return false; }
    return Start_Experiment(profiles);
}

bool_t Client::CRenderingBenchmark::Start_Experiment(CRenderingProfileService& profiles)
{
    if (m_bCapturing || m_bExperimentActive) return false;
    m_ExperimentOriginal=m_ExperimentA=m_ExperimentB=CRenderingProfileService::Read_ExperimentValues();m_iPreparedRecipe=-1;
    if (!profiles.Set_ExperimentPreview(m_ExperimentA,0u,m_strStatus)) return false;
    m_pExperimentProfiles=&profiles; m_bExperimentActive=true; m_bVariantB=false;
    m_iExperimentProfileGeneration=profiles.Get_ProfileGeneration();
    m_iExperimentLevel=Engine::CGameInstance::Get().Get_CurrentLevelID();
    m_strExperimentOwner=profiles.Get_ActiveProfileId()+" / "+profiles.Get_AppliedEnvironmentRegionId();
    static uint64_t sequence=0;
    m_strExperimentId="experiment."+std::to_string(GetCurrentProcessId())+"."+std::to_string(GetTickCount64())+"."+std::to_string(++sequence);
    Reset_ComparisonRows();
    return true;
}

bool_t Client::CRenderingBenchmark::Apply_ExperimentVariant(bool_t variantB)
{
    if (!m_bExperimentActive || !m_pExperimentProfiles) return false;
    if (!m_pExperimentProfiles->Set_ExperimentPreview(variantB ? m_ExperimentB : m_ExperimentA,
        Experiment_FieldMask()|Experiment_BaselineMask(),m_strStatus)) return false;
    m_bVariantB=variantB;
    if(!m_strPreparedComparisonRowId.empty() && !m_bSweep &&
        (!SameComparisonValues(m_PreparedComparisonA,m_ExperimentA) || !SameComparisonValues(m_PreparedComparisonB,m_ExperimentB)))
        Reset_ComparisonRows();
    return true;
}

void Client::CRenderingBenchmark::Finish_Sequence()
{
    if (m_pCaptureProfiler && !m_bProfilerWasEnabled && m_pCaptureProfiler->Is_Enabled())
        m_pCaptureProfiler->Set_Enabled(false);
    m_pCaptureProfiler=nullptr; // A completed run no longer owns a later F7 capture.
    m_bSequence = false; m_bEvidenceDrain=false;
    if(m_bComparisonMeasurement)
    {
        const auto found=m_ComparisonRows.find(m_strMeasurementRowId);
        if(found!=m_ComparisonRows.end() && found->second.measurementId==m_strMeasurementId)
        {
            size_t completed=0;
            for(const auto& run:m_Runs) if(run.strMeasurementId==m_strMeasurementId) ++completed;
            if(completed!=4 || !m_bConditionsStable) found->second.failure=
                m_strFailureReason.empty()?m_strStatus:m_strFailureReason;
        }
        m_bComparisonMeasurement=false;
        const string previousStatus=m_strStatus;
        if(m_bExperimentActive && m_pExperimentProfiles && m_pExperimentProfiles->Has_ExperimentPreview() &&
            Apply_ExperimentVariant(m_bComparisonRestoreVariantB)) m_strStatus=previousStatus;
        m_strMeasurementId.clear(); m_strMeasurementRowId.clear();
    }
    if (m_bSweep)
    {
        m_bSweep = false; m_ExperimentB = m_PreSweepB;
        const string previousStatus = m_strStatus;
        if (m_bExperimentActive && m_pExperimentProfiles && m_pExperimentProfiles->Has_ExperimentPreview() &&
            Apply_ExperimentVariant(m_bSweepRestoreVariantB)) m_strStatus = previousStatus;
    }
}

bool_t Client::CRenderingBenchmark::Start_Sweep(Engine::CProfiler* profiler)
{
    if (!profiler || !m_bExperimentActive || m_bCapturing) return false;
    const auto& info=CRenderingProfileService::Experiment_Fields()[m_iSweepField];
    vector<double> points;
    if (info.boolean) points={0,1};
    else if (m_iSweepField==static_cast<int>(RENDERING_EXPERIMENT_FIELD::SSAO_SAMPLES)) points={4,8,12};
    else if (m_iSweepField==static_cast<int>(RENDERING_EXPERIMENT_FIELD::PCF_RADIUS)) points={0,1,2};
    else if (m_iSweepField==static_cast<int>(RENDERING_EXPERIMENT_FIELD::TEXTURE_MIN_MIP)) points={0,1,2,3};
    else if (m_iSweepField==static_cast<int>(RENDERING_EXPERIMENT_FIELD::SSGI_SAMPLES)) points={4,8,16};
    else if (m_iSweepField==static_cast<int>(RENDERING_EXPERIMENT_FIELD::SSR_STEPS)) points={16,32,64};
    else
    {
        if (!std::isfinite(m_fSweepMinimum) || !std::isfinite(m_fSweepMaximum) ||
            m_fSweepMinimum>=m_fSweepMaximum || m_iSweepSteps<2 || m_iSweepSteps>9)
        { m_strStatus="Sweep requires finite minimum < maximum and 2..9 steps."; return false; }
        for (int i=0;i<m_iSweepSteps;++i)
            points.push_back(static_cast<float>(m_fSweepMinimum+(m_fSweepMaximum-m_fSweepMinimum)*i/(m_iSweepSteps-1)));
    }
    for (double point:points)
    {
        auto candidate=m_ExperimentA; candidate.values[m_iSweepField]=point;
        if (!CRenderingProfileService::Validate_ExperimentValues(candidate,m_strStatus)) return false;
    }
    m_SweepPoints=std::move(points); m_PreSweepB=m_ExperimentB; m_bSweepRestoreVariantB=m_bVariantB;
    m_bSweep=true; m_bSequence=true; m_iSequenceStep=0; m_iSequenceTotal=1u+static_cast<uint32_t>(m_SweepPoints.size());
    m_bSequenceProfilerWasEnabled=profiler->Is_Enabled(); m_bProfilerWasEnabled=m_bSequenceProfilerWasEnabled;
    m_pCaptureProfiler=profiler;
    if (!Apply_ExperimentVariant(false) || !Begin(profiler,m_LabelBuffer.data(),static_cast<uint32_t>(m_iFrameInput),"single-field sweep",m_strStatus))
    { Finish_Sequence(); return false; }
    return true;
}

void Client::CRenderingBenchmark::End_Experiment()
{
    Cancel_Capture("Session experiment ended. Completed runs remain available.",false);
    if (m_pExperimentProfiles && !m_pExperimentProfiles->Clear_ExperimentPreview(m_strStatus)) return;
    m_bExperimentActive=false; m_pExperimentProfiles=nullptr; m_iPresentationStage=-1;
    Reset_ComparisonRows();
}

bool_t Client::CRenderingBenchmark::Prepare_RecipeById(const char* recipeId, CRenderingProfileService& profiles)
{
    if (!recipeId) return false;
    const auto found=std::find_if(std::begin(ExperimentRecipes),std::end(ExperimentRecipes),
        [recipeId](const auto& recipe){return std::string_view(recipe.Id)==recipeId;});
    if (found==std::end(ExperimentRecipes))
    { m_strStatus="이 기법에는 연결된 실험 recipe가 없습니다. 현재 A/B를 유지합니다."; return false; }
    if(std::string_view(found->Id).starts_with("optimization."))
        return Prepare_OptimizationPair(static_cast<size_t>(found-std::begin(ExperimentRecipes)),profiles);
    const bool wasActive=m_bExperimentActive;
    if (!Start_SessionExperiment(profiles)) return false;
    const int previous=m_iSelectedRecipe;
    m_iSelectedRecipe=static_cast<int>(found-std::begin(ExperimentRecipes));
    if (Prepare_Recipe(true)) return true;
    m_iSelectedRecipe=previous;
    if (!wasActive)
    {
        const string reason=m_strStatus;
        End_Experiment();
        m_strStatus=reason;
    }
    return false;
}

bool_t Client::CRenderingBenchmark::Prepare_Recipe(bool_t replaceB)
{
    if(!m_bExperimentActive || m_bCapturing || !m_pExperimentProfiles)
    {m_strStatus="먼저 현재 품질을 A로 보관하고 세션 실험을 시작하세요.";return false;}
    const auto& recipe=ExperimentRecipes[m_iSelectedRecipe];
    RENDERING_EXPERIMENT_VALUES candidate;float low=0,high=0;
    if(!BuildRecipeCandidate(recipe,m_ExperimentA,m_ExperimentA.values[static_cast<size_t>(ExperimentField::SOURCE_MATERIALS)]!=0,
        candidate,low,high,m_strStatus))return false;
    if(replaceB)
    {
        // Stage through the existing owner before changing any UI draft. Rejection
        // preserves the previous B and its active renderer preview.
        if(!m_pExperimentProfiles->Set_ExperimentPreview(candidate,RenderingExperimentBit(recipe.Field)|Experiment_BaselineMask(),m_strStatus))return false;
        m_ExperimentB=candidate;m_bVariantB=true;
    }
    m_iPresentationStage=-1;
    m_iPreparedRecipe=m_iSelectedRecipe;m_iSweepField=static_cast<int>(recipe.Field);
    m_fSweepMinimum=low;m_fSweepMaximum=high;m_iSweepSteps=recipe.Steps;
    std::snprintf(m_LabelBuffer.data(),m_LabelBuffer.size(),"%s",recipe.Id);
    if(replaceB) Mark_PreparedComparison(recipe.Id);
    m_strStatus=replaceB?"A는 보존하고 기존 B를 한 변수 후보로 대체했습니다. 다음 완전한 프레임에 적용합니다.":
        "선택 recipe의 단일 변수 sweep 범위를 준비했습니다. B와 현재 화면은 유지합니다. 아래 sweep 시작으로 측정하세요.";
    return true;
}

bool_t Client::CRenderingBenchmark::Apply_ComparisonPair(const RENDERING_EXPERIMENT_VALUES& base,
    const RENDERING_EXPERIMENT_VALUES& candidate)
{
    if(!m_bExperimentActive || !m_pExperimentProfiles || m_bCapturing) return false;
    uint64_t ownership=0;
    for(size_t i=0;i<RENDERING_EXPERIMENT_FIELD_COUNT;++i)
        if(base.values[i]!=m_ExperimentOriginal.values[i] || candidate.values[i]!=m_ExperimentOriginal.values[i])
            ownership|=uint64_t{1}<<i;
    // Both variants own their differences from the immutable entry values. The
    // comparison mask itself still contains only A/B differences for measurement.
    if(!m_pExperimentProfiles->Set_ExperimentPreview(candidate,ownership,m_strStatus)) return false;
    if(base.values!=m_ExperimentA.values)
    {
        // Generic captures also use the experiment identity to prevent comparing
        // different A baselines. Row-owned ABBA records retain their own IDs.
        static uint64_t revision=0;
        m_strExperimentId="experiment.pair."+std::to_string(GetCurrentProcessId())+"."+
            std::to_string(GetTickCount64())+"."+std::to_string(++revision);
    }
    m_ExperimentA=base; m_ExperimentB=candidate; m_bVariantB=true; m_iPreparedRecipe=-1;
    return true;
}

bool_t Client::CRenderingBenchmark::Prepare_QualitySuiteComparison(CRenderingProfileService& profiles)
{
    const bool wasActive=m_bExperimentActive;
    if(!Start_SessionExperiment(profiles)) return false;
    RENDERING_EXPERIMENT_VALUES candidate;
    if(!BuildQualitySuiteCandidate(m_ExperimentOriginal,candidate,m_strStatus) ||
        !Apply_ComparisonPair(m_ExperimentOriginal,candidate))
    {
        if(!wasActive) { const string reason=m_strStatus; End_Experiment(); m_strStatus=reason; }
        return false;
    }
    m_iPresentationStage=-1; m_iPreparedRecipe=-1;
    Mark_PreparedComparison(QualitySuiteRowId);
    std::snprintf(m_LabelBuffer.data(),m_LabelBuffer.size(),"%s",QualitySuiteRowId);
    m_strStatus="A는 시작 화면, B는 DX11 품질 확장 후보입니다. 여러 기법의 합성 비교이며 측정된 우수성이나 UE5 전체 구현을 뜻하지 않습니다.";
    return true;
}

bool_t Client::CRenderingBenchmark::Apply_PresentationStage(int stage, CRenderingProfileService& profiles)
{
    return Prepare_PresentationPair(stage,profiles);
}

bool_t Client::CRenderingBenchmark::Prepare_QuickComparison(CRenderingProfileService& profiles, bool_t qualitySuiteBase)
{
    const bool wasActive=m_bExperimentActive;
    if(!Start_SessionExperiment(profiles)) return false;
    auto base=m_ExperimentOriginal;
    const auto& selected=QuickTechniques[m_iQuickTechnique];
    const auto found=std::find_if(std::begin(ExperimentRecipes),std::end(ExperimentRecipes),
        [&](const auto& recipe){return std::string_view(recipe.Id)==selected.Recipe;});
    RENDERING_EXPERIMENT_VALUES candidate; float low=0,high=0;
    const bool ready=(!qualitySuiteBase || BuildQualitySuiteCandidate(m_ExperimentOriginal,base,m_strStatus)) &&
        found!=std::end(ExperimentRecipes) && BuildRecipeCandidate(*found,base,
        base.values[static_cast<size_t>(ExperimentField::SOURCE_MATERIALS)]!=0,candidate,low,high,m_strStatus);
    if(!ready || !Apply_ComparisonPair(base,candidate))
    {
        if(!wasActive) { const string reason=m_strStatus; End_Experiment(); m_strStatus=reason; }
        return false;
    }
    m_iPresentationStage=-1;
    m_iSelectedRecipe=m_iPreparedRecipe=static_cast<int>(found-std::begin(ExperimentRecipes));
    m_iSweepField=static_cast<int>(found->Field); m_fSweepMinimum=low; m_fSweepMaximum=high; m_iSweepSteps=found->Steps;
    const string rowId=string(qualitySuiteBase?"quality-base.":"original-base.")+found->Id;
    std::snprintf(m_LabelBuffer.data(),m_LabelBuffer.size(),"%s",rowId.c_str());
    Mark_PreparedComparison(rowId);
    m_strStatus=qualitySuiteBase?"A는 품질 확장 후보, B는 그 안의 한 기법만 바꾼 설정입니다. 종료하면 시작 화면으로 복원합니다.":
        "A는 시연 시작 때의 설정, B는 선택한 한 기법만 바꾼 비교입니다.";
    return true;
}

void Client::CRenderingBenchmark::Render_SessionBar(CRenderingProfileService& profiles)
{
    const bool active=m_bExperimentActive || m_bPixelDiagnosticsActive ||
        !m_strRestorationLastProfileId.empty() || profiles.Get_ComparisonOptions().bActive;
    ImGui::TextUnformatted(active?"임시 비교 중 · 저장 설정 보존":"현재 장면 · 버튼을 눌러 비교 시작");
    if(active)
    {
        if(ImGui::Button("원래 화면으로 복귀"))
        {
            if(m_bExperimentActive) End_Experiment();
            else
            {
                Cancel_Capture("Preview ended; completed measurements retained.");
                profiles.Clear_ComparisonOptions();
                if(!m_strRestorationLastProfileId.empty()) Return_ToEntryProfile(profiles);
                if(m_bPixelDiagnosticsActive && SUCCEEDED(Engine::CGameInstance::Get().Apply_MaterialRenderSettings(m_PixelEntrySettings)))
                { m_bPixelDiagnosticsActive=false; m_strPixelMaterialKey.clear(); }
                if(auto* arena=CLevel_KakulSaydonArena::Get_Active()) arena->Reset_MapLightComparison();
            }
        }
    }
    ImGui::Separator();
}

void Client::CRenderingBenchmark::Render_PresentationSection(CRenderingProfileService& profiles)
{
    ImGui::TextWrapped("하이라이트 > 기술 및 설명 > 렌더링 > 이펙트 툴 > 렌더링 비교");
    ImGui::TextWrapped("초기 재질 근사부터 현재까지 누적 비교합니다. 각 행의 A는 직전 단계, B는 해당 단계입니다. 원래 OFF인 설정은 유지합니다.");
    if(m_bCapturing)
    {
        ImGui::Text("측정 중: %s | %s",m_LabelBuffer.data(),m_bWarmup?"준비":"수집 / GPU 회수");
        if(ImGui::Button("측정 중지")) Cancel_Capture("Capture cancelled; completed runs preserved.");
    }
    ImGui::BeginDisabled(m_bCapturing);
    const int stage=m_bExperimentActive?m_iPresentationStage:-1;
    ImGui::BeginDisabled(stage<=0);
    if(ImGui::Button("이전 단계")) Prepare_PresentationPair(stage-1,profiles);
    ImGui::EndDisabled(); ImGui::SameLine();
    ImGui::BeginDisabled(stage>=static_cast<int>(std::size(PresentationStages))-1);
    if(ImGui::Button(stage<0?"처음부터 시작":"다음 단계")) Prepare_PresentationPair(stage+1,profiles);
    ImGui::EndDisabled(); ImGui::SameLine();
    if(ImGui::Button("현재 완성 설정")) Prepare_PresentationPair(static_cast<int>(std::size(PresentationStages))-1,profiles);
    if(ImGui::CollapsingHeader("측정 설정"))
    {
        ImGui::SliderInt("표본 프레임",&m_iFrameInput,10,900);
        int warm=static_cast<int>(m_iWarmupInput);
        if(ImGui::SliderInt("준비 프레임",&warm,0,120))m_iWarmupInput=static_cast<uint32_t>(warm);
        ImGui::TextWrapped("행 비용은 A1 > B1 > B2 > A2 순서로 두 번씩 측정합니다. 추가 반복과 sweep는 Measure / Analyze에서 실행합니다.");
    }
    ImGui::EndDisabled();
    const auto original=m_bExperimentActive?m_ExperimentOriginal:CRenderingProfileService::Read_ExperimentValues();
    const float tableHeight=(std::max)(ImGui::GetTextLineHeightWithSpacing()*9.f,ImGui::GetContentRegionAvail().y*.68f);
    if(ImGui::BeginTable("RestorationProgression",3,ImGuiTableFlags_RowBg|ImGuiTableFlags_BordersInnerH|
        ImGuiTableFlags_Resizable|ImGuiTableFlags_ScrollY,ImVec2(0,tableHeight)))
    {
        ImGui::TableSetupColumn("복원 순서 · 적용 범위",ImGuiTableColumnFlags_WidthStretch,1.4f);
        ImGui::TableSetupColumn("A / B",ImGuiTableColumnFlags_WidthFixed,ImGui::GetFrameHeight()*4.6f);
        ImGui::TableSetupColumn("실측 프레임 비용 (ms)",ImGuiTableColumnFlags_WidthStretch,1.f);
        ImGui::TableSetupScrollFreeze(0,1); ImGui::TableHeadersRow();
        for(int i=0;i<static_cast<int>(std::size(PresentationStages));++i)
        {
            ImGui::PushID(i); ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0);
            ImGui::BeginDisabled(m_bCapturing);
            const bool active=m_bExperimentActive && m_iPresentationStage==i;
            if(ImGui::Selectable(PresentationStages[i].Name,active)) Prepare_PresentationPair(i,profiles);
            ImGui::EndDisabled();
            ImGui::TextWrapped("%s",PresentationStages[i].Description);
            if(i && BuildPresentationStage(original,i-1).values==BuildPresentationStage(original,i).values)
                ImGui::TextDisabled("설정 차이 없음 · 원래 OFF 또는 같은 값");
            ImGui::TableSetColumnIndex(1); ImGui::BeginDisabled(m_bCapturing);
            if(ImGui::Button("A")) {if(Prepare_PresentationPair(i,profiles)) Apply_ExperimentVariant(false);}
            ImGui::SameLine(); if(ImGui::Button("B")) Prepare_PresentationPair(i,profiles);
            if(ImGui::Button("비용 측정"))
                if(Prepare_PresentationPair(i,profiles)) Start_ComparisonMeasurement(Engine::CGameInstance::Get().Get_Profiler());
            ImGui::EndDisabled();
            if(active) ImGui::Text("적용: %s",m_bVariantB?"B":"A");
            ImGui::TableSetColumnIndex(2);
            const string rowId="restoration.stage."+std::to_string(i+1);
            Render_ComparisonCost(rowId.c_str());
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    ImGui::TextWrapped("%s",m_strStatus.c_str());
    ImGui::TextWrapped("비용 측정은 같은 카메라에서 A/B를 반복합니다. CPU와 GPU는 겹쳐 실행되므로 더하지 않습니다. 단계 차이는 그 단계 묶음의 관측값이며 개별 기법은 Technique A/B에서 측정합니다.");
    if(ImGui::CollapsingHeader("개발 · 복구 이력 / 시연 범위"))
    {
        ImGui::TextWrapped("2026-07-29 | WModel reader · 변환기 · MapTool 배치와 첫 렌더 연결");
        ImGui::TextWrapped("2026-08-08 | FP16 HDR · Bloom · Hable tone · 선택 FXAA와 첫 Workbench");
        ImGui::TextWrapped("2026-09-08 | 선택 바닥의 원본 MIC 수식 · tangent · PBR · UV1 RNM · cube 입력");
        ImGui::TextWrapped("2026-09-11 | 베른 UV1 · COLOR0 · tangent W · 배치 RNM · 정적 shadow · 원본 광원");
        ImGui::TextWrapped("2026-09-15~16 | 원본 tone curve · 지역 후처리 · grading 순서 · GPU LUT bake");
        ImGui::TextWrapped("2026-09-23 검증 / 09-24 반영 | 선택창 native SH · cube 회전 · 128x32 BRDF · 간접광 중복 제거");
        ImGui::TextWrapped("2026-09-30 기록 / 10-01 반영 | 베른 지형 형상 · hole · 원본 레이어 재질 복구");
        ImGui::TextWrapped("2026-10-03~04 | 세션 A/B · CPU/GPU 계측 · SSGI/SSR 추가 실험");
        ImGui::Separator();
        ImGui::TextWrapped("기본 단계는 현재 WModel의 기본 재질 근사입니다. 복원된 형상·텍스처는 유지하므로 최초 임포트 EXE와 동일한 화면은 아닙니다. 미지원 native/forward 재질과 이펙트는 그대로입니다.");
        ImGui::TextWrapped("UV·COLOR0·alpha·원본 texture/CB/sampler binding 복구는 현재 자산과 shader에 연결된 고정 복구입니다. 과거 결함 상태로 되돌리는 토글은 없습니다. BG RNM·정적 shadow는 02 원본 재질 단계에 포함됩니다.");
        ImGui::TextWrapped("기술 설명용 누적 순서이며 날짜별 빌드를 재실행하는 기능은 아닙니다. PBR 기여 0도 연산 제거를 보장하지 않습니다. 반복 편차보다 작은 ms 차이는 결론을 유보하고 움직이는 장면은 탐색 측정으로 봅니다.");
        ImGui::TextWrapped("복구 정본: .md/GB/렌더링이펙트복원V2.md | 세션·측정 계약: 10-03 PROFILER_RENDERING_WORKBENCH_RESULT | 현재 시연: 10-04 RENDERING_WORKBENCH_PRESENTATION_RESULT");
    }
}

void Client::CRenderingBenchmark::Render_QuickComparison(CRenderingProfileService& profiles)
{
    const auto original=m_bExperimentActive?m_ExperimentOriginal:CRenderingProfileService::Read_ExperimentValues();
    auto& game=Engine::CGameInstance::Get();
    const auto quality=game.Get_RenderQualitySettings();
    const auto environment=game.Get_RenderEnvironment();
    const auto* profile=profiles.Get_ActiveProfile();
    ImGui::SeparatorText("현재 연결된 원본 복구 기여 A/B");
    ImGui::TextWrapped("A는 시작 화면, B는 현재 연결된 한 경로의 ON/OFF 또는 기여만 바꾼 설정입니다. 새 원본 값이나 프로필을 만들지 않습니다. 저장·Publish 없이 종료하면 시작 화면으로 복원합니다.");
    ImGui::Text("현재 profile: %s | region: %s",profiles.Get_ActiveProfileId().c_str(),
        profiles.Get_AppliedEnvironmentRegionId().empty()?"scene":profiles.Get_AppliedEnvironmentRegionId().c_str());
    ImGui::Text("현재 source material %s / PBR indirect %s / tone + grading %s",
        game.Get_MaterialRenderSettings().bUseSourceMaterials?"ON":"OFF",
        environment.bUseSourcePBRIndirect?"ON":"OFF",quality.SourcePostProcess.bEnabled?"ON":"OFF");
    if(ImGui::CollapsingHeader("현재 연결 입력 · 원작 일치 인증 아님"))
    {
        const char* cube=profile && !profile->strEnvironmentCubeAssetId.empty()?profile->strEnvironmentCubeAssetId.c_str():"없음";
        ImGui::TextWrapped("Profile cube asset: %s | 현재 scene cube SRV: %s",cube,environment.pCube?"있음":"없음");
        ImGui::Text("현재 tone scale %.4g / range %.4g / toe %.4g | LUT layers %zu",
            quality.SourcePostProcess.fToneScale,quality.SourcePostProcess.fToneRange,
            quality.SourcePostProcess.fToneToe,quality.SourcePostProcess.LutLayers.size());
        for(const auto& layer:quality.SourcePostProcess.LutLayers)
            ImGui::TextWrapped("LUT: %s | weight %.4g",layer.pLut?layer.pLut->strAssetId.c_str():"neutral lattice",layer.fWeight);
        const auto bindings=CMapAssetRenderUtils::Get_RecentSurfaceBindings();
        ImGui::Text("최근1초 관측한 표면 정의: %zu (최대32개)",bindings.size());
        for(const auto& row:bindings)
            ImGui::TextWrapped("%s | %s | program %u / RNM %s / cube %s / source indirect %s",
                row.assetId.c_str(),row.materialName.c_str(),row.activeProgram,row.surface.hasBakedLighting?"있음":"없음",
                row.surface.hasEnvironmentCube?"있음":"없음",row.surface.hasSourceIndirect?"있음":"없음");
        ImGui::TextWrapped("입력의 존재와 현재 연결 상태를 표시합니다. 원본 영상과 일치한다는 판정이나 GPU 픽셀 검증은 아닙니다. 관측되지 않은 표면의 지원을 추정하지 않습니다.");
    }
    if(m_bCapturing)
    {
        ImGui::Text("측정 중: %s | %s",m_LabelBuffer.data(),m_bWarmup?"준비":"수집 / GPU 회수");
        if(ImGui::Button("측정 중지")) Cancel_Capture("Capture cancelled; completed runs preserved.");
    }
    const auto renderRows=[&](int group,const char* tableId,bool qualitySuiteBase) {
        auto base=original; string baseStatus;
        const bool baseReady=!qualitySuiteBase || BuildQualitySuiteCandidate(original,base,baseStatus);
        const float height=(std::max)(ImGui::GetTextLineHeightWithSpacing()*9.f,ImGui::GetContentRegionAvail().y*.55f);
        if(!ImGui::BeginTable(tableId,3,ImGuiTableFlags_RowBg|ImGuiTableFlags_BordersInnerH|
            ImGuiTableFlags_Resizable|ImGuiTableFlags_ScrollY,ImVec2(0,height))) return;
        ImGui::TableSetupColumn("기법 · 현재 연결 범위",ImGuiTableColumnFlags_WidthStretch,1.4f);
        ImGui::TableSetupColumn("A / B",ImGuiTableColumnFlags_WidthFixed,ImGui::GetFrameHeight()*4.6f);
        ImGui::TableSetupColumn("실측 프레임 비용 (ms)",ImGuiTableColumnFlags_WidthStretch,1.f);
        ImGui::TableSetupScrollFreeze(0,1); ImGui::TableHeadersRow();
        for(int i=0;i<static_cast<int>(std::size(QuickTechniques));++i)
        {
            const auto& technique=QuickTechniques[i]; if(technique.Group!=group) continue;
            const auto recipe=std::find_if(std::begin(ExperimentRecipes),std::end(ExperimentRecipes),
                [&](const auto& item){return std::string_view(item.Id)==technique.Recipe;});
            if(recipe==std::end(ExperimentRecipes))continue;
            RENDERING_EXPERIMENT_VALUES candidate;float low=0,high=0;string reason;
            const bool ready=baseReady && BuildRecipeCandidate(*recipe,base,
                base.values[static_cast<size_t>(ExperimentField::SOURCE_MATERIALS)]!=0,candidate,low,high,reason);
            if(!baseReady) reason=baseStatus;
            const string rowId=string(qualitySuiteBase?"quality-base.":"original-base.")+technique.Recipe;
            const auto prepare=[&]() {
                const int previous=m_iQuickTechnique;m_iQuickTechnique=i;
                if(Prepare_QuickComparison(profiles,qualitySuiteBase))return true;
                m_iQuickTechnique=previous;return false;
            };
            ImGui::PushID(i); ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(technique.Name);
            const size_t field=static_cast<size_t>(recipe->Field);
            if(ready) ImGui::Text("A %.4g / B %.4g",base.values[field],candidate.values[field]);
            ImGui::TextWrapped("%s",ready?recipe->Boundary:reason.c_str());
            ImGui::TableSetColumnIndex(1); ImGui::BeginDisabled(m_bCapturing || !ready);
            if(ImGui::Button("A")) {if(prepare())Apply_ExperimentVariant(false);}
            ImGui::SameLine();if(ImGui::Button("B"))prepare();
            if(ImGui::Button("비용 측정"))if(prepare())Start_ComparisonMeasurement(game.Get_Profiler());
            ImGui::EndDisabled();
            if(m_bExperimentActive && m_strPreparedComparisonRowId==rowId)
                ImGui::Text("적용: %s",m_bVariantB?"B":"A");
            ImGui::TableSetColumnIndex(2); Render_ComparisonCost(rowId.c_str());
            ImGui::PopID();
        }
        ImGui::EndTable();
    };
    renderRows(0,"SourceRestorationCosts",false);
    if(ImGui::CollapsingHeader("기타 현재 설정 A/B"))
    {
        ImGui::TextWrapped("시작 화면을 기준으로 기존 그림자·SSAO·안개·표시 설정을 한 항목씩 비교합니다. 원본 기법의 구현 여부를 판정하는 묶음이 아닙니다.");
        renderRows(1,"CurrentTechniqueCosts",false);
    }
    if(ImGui::CollapsingHeader("추가 품질 실험 (원본 복구와 별도)"))
    {
        RENDERING_EXPERIMENT_VALUES suite; string suiteStatus;
        const bool suiteReady=BuildQualitySuiteCandidate(original,suite,suiteStatus);
        ImGui::SeparatorText("DX11 품질 확장 실험");
        ImGui::TextWrapped("A는 시작 화면, B는 mip0 + 자체 Horizon AO 12방향 + PCF25 + SSGI16 전체 해상도 + SSR64 교차 보정·roughness 필터 후보입니다. 원본 복구를 뜻하지 않습니다. 저장값·노출·감마·FXAA·Bloom은 유지합니다.");
        if(suiteReady)
            ImGui::Text("후보 강도 AO %.3g / SSGI %.3g / SSR %.3g (원래 강도가 0일 때만 1 / 0.25 / 0.5)",
                suite.values[static_cast<size_t>(ExperimentField::SSAO_INTENSITY)],
                suite.values[static_cast<size_t>(ExperimentField::SSGI_STRENGTH)],
                suite.values[static_cast<size_t>(ExperimentField::SSR_STRENGTH)]);
        else ImGui::TextWrapped("%s",suiteStatus.c_str());
        ImGui::BeginDisabled(m_bCapturing || !suiteReady);
        if(ImGui::Button("시작 화면 A##qualitySuite")) { if(Prepare_QualitySuiteComparison(profiles)) Apply_ExperimentVariant(false); }
        ImGui::SameLine(); if(ImGui::Button("품질 확장 B##qualitySuite")) Prepare_QualitySuiteComparison(profiles);
        ImGui::SameLine(); if(ImGui::Button("통합 비용 측정##qualitySuite"))
            if(Prepare_QualitySuiteComparison(profiles)) Start_ComparisonMeasurement(game.Get_Profiler());
        ImGui::EndDisabled();
        Render_ComparisonCost(QualitySuiteRowId);
        ImGui::TextWrapped("SSGI/SSR은 marker3 MapPBR·FINAL 화면의 RNM/IBL 위에 가산합니다. Horizon AO는 현재 SSAO 수신면을 사용하며 PCF는 켜져 있는 그림자에만 보입니다. 시간 누적·UE5/Lumen·GTAO·경로 추적은 없습니다. 실제 비용과 같은 카메라의 화질을 따로 판단합니다.");
        ImGui::BeginDisabled(m_bCapturing);
        ImGui::Checkbox("아래 추가 실험의 A를 품질 확장 후보로 사용",&m_bQuickQualitySuiteBase);
        ImGui::EndDisabled();
        ImGui::TextWrapped(m_bQuickQualitySuiteBase?
            "아래 A는 품질 후보, B는 그 안의 한 기법만 바꾼 설정입니다. 위 원본 복구 비교는 계속 시작 화면을 기준으로 합니다.":
            "아래 A는 시작 화면, B는 한 기법만 바꾼 설정입니다. 체크하거나 영역을 펼치는 것만으로 화면은 바뀌지 않습니다.");
        renderRows(2,"ExtraTechniqueCosts",m_bQuickQualitySuiteBase);
    }
    ImGui::TextWrapped("%s",m_strStatus.c_str());
    ImGui::TextWrapped("양수는 B의 추가 프레임 시간, 음수는 감소입니다. B가 OFF인 행은 기능을 끈 차이입니다. GPU N/A·미측정은 0 ms가 아닙니다. sample/PCF/반경 sweep와 패스별 수치는 Measure / Analyze에서 확인합니다.");
}

void Client::CRenderingBenchmark::Render_RecipeSection()
{
    if(!ImGui::CollapsingHeader("원인별 실험 recipe / 한 변수로 시작"))return;
    ImGui::TextWrapped("recipe 선택만으로 설정을 바꾸지 않습니다. B 대체 버튼은 기존 B 조정을 A 기준의 한 변수로 명시적으로 교체합니다. sweep 준비는 범위만 설정합니다. 숫자는 비교 후보이며 측정 결과·추천 최적값이 아닙니다.");
    if(ImGui::BeginCombo("실험 목표",ExperimentRecipes[m_iSelectedRecipe].Name))
    {
        for(int i=0;i<static_cast<int>(std::size(ExperimentRecipes));++i)
            if(ImGui::Selectable(ExperimentRecipes[i].Name,m_iSelectedRecipe==i))m_iSelectedRecipe=i;
        ImGui::EndCombo();
    }
    const auto& recipe=ExperimentRecipes[m_iSelectedRecipe];
    const auto& field=CRenderingProfileService::Experiment_Fields()[static_cast<size_t>(recipe.Field)];
    ImGui::Text("단일 변수: %s",field.id);ImGui::TextWrapped("목표: %s",recipe.Goal);
    ImGui::TextWrapped("볼 수치: %s",recipe.Metrics);ImGui::TextWrapped("판정 범위: %s",recipe.Boundary);
    const auto confidence=RecipeConfidence(recipe.Kind);ImGui::TextWrapped("신뢰도 해석: %s",confidence.c_str());
    RENDERING_EXPERIMENT_VALUES candidate;float low=0,high=0;string reason;
    const bool ready=BuildRecipeCandidate(recipe,m_ExperimentA,m_ExperimentA.values[static_cast<size_t>(ExperimentField::SOURCE_MATERIALS)]!=0,
        candidate,low,high,reason);
    if(ready)ImGui::Text("A %.6g → B 후보 %.6g | sweep %.6g..%.6g (%d단계 / discrete는 허용값만)",
        m_ExperimentA.values[static_cast<size_t>(recipe.Field)],candidate.values[static_cast<size_t>(recipe.Field)],low,high,recipe.Steps);
    else ImGui::TextWrapped("준비 불가: %s",reason.c_str());
    ImGui::BeginDisabled(!ready);
    if(ImGui::Button("기존 B를 이 단일 변수로 대체하고 적용"))Prepare_Recipe(true);
    ImGui::SameLine();if(ImGui::Button("이 변수의 sweep 범위만 준비"))Prepare_Recipe(false);
    ImGui::EndDisabled();
    ImGui::TextWrapped("준비 후 아래 현재 A/B 측정·AB/BA 반복 또는 단일 변수 sweep 시작을 사용합니다. 화질은 같은 카메라에서 직접 확인하고 반복 편차보다 작은 시간 차이는 결론을 유보하세요.");
}

void Client::CRenderingBenchmark::Render_ExperimentSection(Engine::CProfiler* profiler, CRenderingProfileService& profiles)
{
    ImGui::SeparatorText("렌더링 실험 / 세션 A-B");
    ImGui::TextWrapped("A는 현재 화면의 실효 설정입니다. B만 임시 변경하며 저장 프로필·지역·사용자 Video 파일은 보존합니다. 창 닫기·레벨/지역/프로필 변경은 실험을 종료합니다.");
    if (!m_bExperimentActive)
    {
        ImGui::BeginDisabled(m_bCapturing);
        if (ImGui::Button("현재 품질을 A로 보관하고 실험 시작")) Start_SessionExperiment(profiles);
        ImGui::EndDisabled();
    }
    else
    {
        ImGui::Text("%s | %s 적용",m_strExperimentId.c_str(),m_bVariantB?"B":"A");
        ImGui::TextWrapped("장면: %s",m_strExperimentOwner.c_str());
        if (ImGui::Button("실험 종료 / 원래 장면 복원")) End_Experiment();
        if (!m_bExperimentActive) return;
        ImGui::BeginDisabled(m_bCapturing);
        if (ImGui::Button("A 적용")) Apply_ExperimentVariant(false);
        ImGui::SameLine(); if (ImGui::Button("B 적용")) Apply_ExperimentVariant(true);
        ImGui::SameLine(); if (ImGui::Button("B를 A에서 다시 복사")) { m_ExperimentB=m_ExperimentA; m_iPresentationStage=-1; Apply_ExperimentVariant(true); }
        if (ImGui::Button("현재 B를 새 A 기준으로 채택 (이전 비교와 분리)")) Adopt_BaselineFromB();
        ImGui::TextWrapped("기본 OFF 패스의 품질 수치를 비교하려면 B에서 패스를 ON → 새 A로 채택 → samples/radius recipe 순서입니다. 두 variant의 공통 기준 변경은 실험 변수에 포함하지 않으며 종료하면 원래 장면으로 복원합니다.");
        if (ImGui::Button("B: 본질 기준 (다변수 진단)"))
        {
            m_ExperimentB=m_ExperimentA; m_iPresentationStage=-1;
            for (const auto field : {RENDERING_EXPERIMENT_FIELD::SSAO_ENABLED,RENDERING_EXPERIMENT_FIELD::BLOOM_ENABLED,
                RENDERING_EXPERIMENT_FIELD::FXAA_ENABLED,RENDERING_EXPERIMENT_FIELD::FOG_ENABLED,
                RENDERING_EXPERIMENT_FIELD::LUT_ENABLED,RENDERING_EXPERIMENT_FIELD::DESATURATION,
                RENDERING_EXPERIMENT_FIELD::SSGI_ENABLED,RENDERING_EXPERIMENT_FIELD::SSR_ENABLED})
                m_ExperimentB.values[static_cast<size_t>(field)]=0;
            Apply_ExperimentVariant(true);
        }
        ImGui::TextWrapped("본질 기준은 SSAO·SSGI·SSR·Bloom·FXAA·안개·LUT·탈색을 끕니다. 재질·직접광·RNM·환경광·그림자·노출·감마는 A와 같습니다. GI 없는 화면이 아닙니다. 기여량 0도 계산 생략을 뜻하지 않습니다.");
        static const char* labels[]={"SSAO 켜기","SSAO 반경 (m)","SSAO bias","SSAO 강도","SSAO power","SSAO 거리 감쇠 (m)",
            "Bloom 켜기","Bloom 임계값","Bloom soft knee","Bloom 강도","Bloom scatter","FXAA 켜기","FXAA blend",
            "FXAA edge threshold","FXAA 최소 threshold","노출","표시 gamma","방향광 그림자 켜기","그림자 강도",
            "안개 켜기","안개 밀도","LUT 켜기","장면 탈색","SSAO 샘플 수","PCF 반경 (0/1/2 = 1/9/25 tap)","PBR 직접 diffuse","PBR 직접 specular","PBR baked RNM",
            "PBR 환경 specular","PBR cube diffuse","PBR normal 강도","PBR roughness offset",
            "실험 SSGI 켜기","SSGI 기여 강도","SSGI 반경 (m)","SSGI 샘플 수 (4/8/16)",
            "실험 SSR 켜기","SSR 기여 강도","SSR 최대 거리 (m)","SSR hit 두께 (m)","SSR step 수 (16/32/64)",
            "원본 PBR 간접광 경로","Source tone + grading 묶음","기본 / 원본 재질","SSGI 절반 해상도 + 경계 보존 합성",
            "표면 텍스처 최소 mip (0/1/2/3)","Horizon AO 사용 (SSAO 대신)","SSR 교차 구간 세부 탐색","SSR roughness 반사 필터",
            "맵 프러스텀 컬링","맵 가림 컬링","맵 거리 컬링","메시 LOD 선택","맵 인스턴싱","동일 모델·재질 추가 병합","조명 bank 병합","정적 그림자 캐시","NPC pose 표본 재사용","파티클 root 변환 재사용","맵 CPU worker","파티클 CPU worker"};
        static_assert(std::size(labels)==RENDERING_EXPERIMENT_FIELD_COUNT);
        size_t changedCount=0; const uint64_t mask=Experiment_FieldMask();
        for (size_t i=0;i<RENDERING_EXPERIMENT_FIELD_COUNT;++i) if (mask&(uint64_t{1}<<i)) ++changedCount;
        ImGui::Text("변경 필드 %zu개: %s",changedCount,changedCount==1?"단일 변수 비교":(changedCount?"다변수 진단 (개별 원인 비용 아님)":"동일 기준"));
        Render_RecipeSection();
        if (ImGui::CollapsingHeader("B 수치 조절 / 실제 지원 필드"))
        {
            const auto& fields=CRenderingProfileService::Experiment_Fields();
            if (ImGui::BeginTable("ExperimentFields",3,ImGuiTableFlags_RowBg|ImGuiTableFlags_BordersInnerH|ImGuiTableFlags_Resizable))
            {
                ImGui::TableSetupColumn("항목"); ImGui::TableSetupColumn("A (보관)"); ImGui::TableSetupColumn("B (임시 조절)"); ImGui::TableHeadersRow();
                bool changed=false;
                for (size_t i=0;i<fields.size();++i)
                {
                    ImGui::PushID(static_cast<int>(i)); ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted(labels[i]); if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s",fields[i].id);
                    ImGui::TableSetColumnIndex(1); ImGui::Text("%.5g",m_ExperimentA.values[i]);
                    ImGui::TableSetColumnIndex(2); ImGui::SetNextItemWidth(-1.f);
                    const bool inactiveCube=i==static_cast<size_t>(RENDERING_EXPERIMENT_FIELD::PBR_CUBE) &&
                        m_ExperimentB.values[static_cast<size_t>(RENDERING_EXPERIMENT_FIELD::SOURCE_PBR_INDIRECT)]!=0;
                    const bool inactiveLut=i==static_cast<size_t>(RENDERING_EXPERIMENT_FIELD::LUT_ENABLED) &&
                        m_ExperimentB.values[static_cast<size_t>(RENDERING_EXPERIMENT_FIELD::SOURCE_POST_PROCESS)]==0;
                    ImGui::BeginDisabled(inactiveCube || inactiveLut);
                    if (i==static_cast<size_t>(RENDERING_EXPERIMENT_FIELD::SSAO_SAMPLES))
                    { int value=static_cast<int>(m_ExperimentB.values[i]/4.0)-1; if (ImGui::Combo("##value",&value,"4\0" "8\0" "12\0")) {m_ExperimentB.values[i]=(value+1)*4;changed=true;} }
                    else if (i==static_cast<size_t>(RENDERING_EXPERIMENT_FIELD::PCF_RADIUS) || i==static_cast<size_t>(RENDERING_EXPERIMENT_FIELD::TEXTURE_MIN_MIP))
                    { const int maximum=i==static_cast<size_t>(RENDERING_EXPERIMENT_FIELD::TEXTURE_MIN_MIP)?3:2;
                      int value=static_cast<int>(m_ExperimentB.values[i]); if (ImGui::SliderInt("##value",&value,0,maximum)) {m_ExperimentB.values[i]=value;changed=true;} }
                    else if (i==static_cast<size_t>(RENDERING_EXPERIMENT_FIELD::SSGI_SAMPLES) || i==static_cast<size_t>(RENDERING_EXPERIMENT_FIELD::SSR_STEPS))
                    {
                        const bool gi=i==static_cast<size_t>(RENDERING_EXPERIMENT_FIELD::SSGI_SAMPLES); const int first=gi?4:16;
                        int value=m_ExperimentB.values[i]==first?0:(m_ExperimentB.values[i]==first*2?1:2);
                        if (ImGui::Combo("##value",&value,gi?"4\0" "8\0" "16\0":"16\0" "32\0" "64\0"))
                        { m_ExperimentB.values[i]=first*(1<<value);changed=true; }
                    }
                    else if (fields[i].boolean)
                    { bool value=m_ExperimentB.values[i]!=0; if (ImGui::Checkbox("##value",&value)) {m_ExperimentB.values[i]=value?1:0;changed=true;} }
                    else
                    { float value=static_cast<float>(m_ExperimentB.values[i]); if (ImGui::DragFloat("##value",&value,static_cast<float>(fields[i].step),static_cast<float>(fields[i].minimum),static_cast<float>(fields[i].maximum),"%.5g",ImGuiSliderFlags_AlwaysClamp)) {m_ExperimentB.values[i]=value;changed=true;} }
                    ImGui::EndDisabled();
                    if (inactiveCube) ImGui::TextWrapped("원본 간접광 ON: project cube diffuse 미사용");
                    if (inactiveLut) ImGui::TextWrapped("Source tone + grading OFF: LUT 미사용");
                    ImGui::PopID();
                }
                ImGui::EndTable(); if (changed) { m_iPresentationStage=-1; Apply_ExperimentVariant(true); }
            }
        }
        ImGui::TextWrapped("SSGI/SSR은 source 재질의 marker3 MapPBR 수신면과 정상 FINAL view가 필요합니다. 해당 면이 없으면 영상 기여 없이 full-screen 패스·복사 비용이 발생할 수 있습니다. PSInvocations는 ray hit 수가 아닙니다. 두 기법은 같은 원본 radiance를 읽으며 SSGI 결과를 SSR 입력으로 재사용하지 않습니다.");
        ImGui::TextWrapped("PBR 기여값은 지원되는 map PBR 재질에만 적용됩니다. Source character의 별도 계산은 그대로입니다. Video OFF를 ON으로 조절하는 경우도 현재 실험에만 적용되며 사용자 파일을 변경하지 않습니다.");
        ImGui::TextWrapped("원본 PBR 간접광 OFF는 보존된 이전 환경 경로이며 모든 GI 제거가 아닙니다. Source tone + grading OFF는 Hable fallback으로 전환합니다. LUT-only 비교와 달리 tone·색보정 묶음 전체를 바꾸며 노출·감마·저장된 curve/색/LUT 입력은 보존합니다.");
        ImGui::EndDisabled();
    }
    ImGui::BeginDisabled(m_bCapturing);
    ImGui::InputText("실험 이름",m_LabelBuffer.data(),m_LabelBuffer.size());
    ImGui::SliderInt("측정 프레임",&m_iFrameInput,10,900);
    int warm=static_cast<int>(m_iWarmupInput), repeat=static_cast<int>(m_iRepeatInput);
    if (ImGui::SliderInt("준비 프레임",&warm,0,120)) m_iWarmupInput=static_cast<uint32_t>(warm);
    if (ImGui::SliderInt("A/B 반복",&repeat,1,8)) m_iRepeatInput=static_cast<uint32_t>(repeat);
    ImGui::SliderFloat("목표 FPS 예산",&m_fTargetFps,15,240,"%.0f");
    ImGui::Text("목표 %.0f FPS = %.3f ms / 프레임",m_fTargetFps,1000.0/m_fTargetFps);
    if (m_bExperimentActive)
    {
        if (ImGui::Button("현재 A/B 측정")) { m_iSequenceStep=0; Begin(profiler,m_LabelBuffer.data(),static_cast<uint32_t>(m_iFrameInput),"session experiment",m_strStatus); }
        ImGui::SameLine();
        if (ImGui::Button("A/B 반복 측정 (AB / BA)") && profiler)
        {
            m_bSequenceProfilerWasEnabled=profiler->Is_Enabled();
            m_iSequenceStep=0; m_iSequenceTotal=2u*m_iRepeatInput;
            if (Apply_ExperimentVariant(false))
            { m_bSequence=true; if (!Begin(profiler,m_LabelBuffer.data(),static_cast<uint32_t>(m_iFrameInput),"session experiment",m_strStatus)) m_bSequence=false; }
        }
    }
    if (m_bExperimentActive && ImGui::CollapsingHeader("단일 변수 자동 sweep (A + 최대 9개 값)"))
    {
        const auto& fields=CRenderingProfileService::Experiment_Fields();
        if (ImGui::BeginCombo("변수",fields[m_iSweepField].id))
        {
            for (size_t i=0;i<fields.size();++i)
                if (ImGui::Selectable(fields[i].id,m_iSweepField==static_cast<int>(i)))
                { m_iSweepField=static_cast<int>(i); m_fSweepMinimum=static_cast<float>(fields[i].minimum); m_fSweepMaximum=static_cast<float>(fields[i].maximum); }
            ImGui::EndCombo();
        }
        const auto& f=fields[m_iSweepField];
        if (!f.boolean && m_iSweepField!=static_cast<int>(RENDERING_EXPERIMENT_FIELD::SSAO_SAMPLES) && m_iSweepField!=static_cast<int>(RENDERING_EXPERIMENT_FIELD::PCF_RADIUS)
            && m_iSweepField!=static_cast<int>(RENDERING_EXPERIMENT_FIELD::SSGI_SAMPLES) && m_iSweepField!=static_cast<int>(RENDERING_EXPERIMENT_FIELD::SSR_STEPS))
        {
            ImGui::DragFloat("최솟값",&m_fSweepMinimum,static_cast<float>(f.step),static_cast<float>(f.minimum),static_cast<float>(f.maximum),"%.5g",ImGuiSliderFlags_AlwaysClamp);
            ImGui::DragFloat("최댓값",&m_fSweepMaximum,static_cast<float>(f.step),static_cast<float>(f.minimum),static_cast<float>(f.maximum),"%.5g",ImGuiSliderFlags_AlwaysClamp);
            ImGui::SliderInt("단계 수",&m_iSweepSteps,2,9);
        }
        ImGui::TextWrapped("선택 필드 외에는 A를 유지합니다. bool은 0/1, SSAO는 4/8/12, PCF는 0/1/2, SSGI는 4/8/16, SSR은 16/32/64만 측정합니다. 각 값 준비→수집→GPU 회수 후 다음 값으로 이동하며 종료·취소 시 시작 전 A/B로 복원합니다.");
        if (ImGui::Button("A 기준 + sweep 시작")) Start_Sweep(profiler);
    }
    ImGui::EndDisabled();
    if (m_bCapturing)
    {
        ImGui::Text("%s | %s | 반복 %u / %u",m_bWarmup?"준비 중":"수집 / GPU 회수",m_strCaptureVariant.c_str(),m_iSequenceStep/2u+1u,m_bSequence?m_iRepeatInput:1u);
        if (ImGui::Button("측정 취소 (설정은 유지)")) Cancel_Capture("Capture cancelled; completed runs preserved.");
    }
    ImGui::TextWrapped("%s",m_strStatus.c_str());
    if (!m_strSaveStatus.empty()) ImGui::TextWrapped("%s",m_strSaveStatus.c_str());
    ImGui::TextWrapped("카메라·해상도·장면·Video·도구 창 표시·Profiler 상세 계측을 고정하세요. 동적 animation/Server gameplay를 고정 재생하지 않으므로 해당 장면 결과는 탐색 측정입니다. SSGI/SSR은 현재 화면의 marker3 MapPBR 수신용 공간 기법이며 temporal history·화면 밖 정보는 없습니다. 준비 프레임은 cache 안정화를 위한 사용자 정책입니다.");
}

void Client::CRenderingBenchmark::Render_Results()
{
    if (m_Runs.empty()) return;
    ImGui::SeparatorText("이전 결과 비교 / 프레임 비용");
    ImGui::BeginDisabled(m_bCapturing || m_bSequence || m_RawExporter.IsSaving());
    if (ImGui::Button("결과 JSON 저장")) Queue_Save();
    ImGui::SameLine(); if (ImGui::Button("목록 비우기")) {m_Runs.clear();m_iCompareFirst=m_iCompareSecond=-1;ImGui::EndDisabled();return;}
    ImGui::EndDisabled();
    const auto select=[&](const char* label,int& index) {
        const char* preview=index>=0&&static_cast<size_t>(index)<m_Runs.size()?m_Runs[index].strLabel.c_str():"선택";
        if (ImGui::BeginCombo(label,preview))
        {
            for (size_t i=0;i<m_Runs.size();++i)
            {
                const auto& r=m_Runs[i]; const string name=std::to_string(i+1)+". "+r.strLabel+" ["+r.strVariant+"] #"+std::to_string(r.repetition);
                if (ImGui::Selectable(name.c_str(),index==static_cast<int>(i))) index=static_cast<int>(i);
            }
            ImGui::EndCombo();
        }
    };
    if (m_iCompareFirst<0 && m_Runs.size()>1) m_iCompareFirst=static_cast<int>(m_Runs.size()-2);
    select("기준 결과",m_iCompareFirst); select("비교 결과",m_iCompareSecond);
    if (m_iCompareFirst>=0&&m_iCompareSecond>=0&&static_cast<size_t>(m_iCompareFirst)<m_Runs.size()&&static_cast<size_t>(m_iCompareSecond)<m_Runs.size())
    {
        const auto& a=m_Runs[m_iCompareFirst]; const auto& b=m_Runs[m_iCompareSecond];
        const bool same=ComparisonRunsMatch(a,b);
        if (!same)
        {
            ImGui::TextWrapped("비교 제외: 공통 조건·독립 변수·실험 ID가 다르거나 수집 중 조건이 바뀌었습니다.");
            if(a.strExperimentId!=b.strExperimentId)ImGui::TextWrapped("experimentId: %s → %s",a.strExperimentId.c_str(),b.strExperimentId.c_str());
            if(a.fieldMask!=b.fieldMask)ImGui::TextWrapped("선택 실험 변수 집합이 다릅니다. 두 run의 selectedFields를 확인하세요.");
            const auto changes=ChangedConditionFields(a.commonConditionFields,b.commonConditionFields);size_t shown=0;
            for(const auto& [name,value]:changes)
            {ImGui::TextWrapped("%s: %s",name.c_str(),value.c_str());if(++shown==12)break;}
            if(changes.size()>shown)ImGui::Text("추가 %zu개 조건 차이는 JSON named fields에 보관했습니다.",changes.size()-shown);
            if(changes.empty()&&a.strComparisonConditions!=b.strComparisonConditions)ImGui::TextWrapped("추가 raw 입력 차이: 이름 진단 범위 밖이며 같은 조건으로 간주하지 않습니다.");
        }
        else
        {
            ImGui::Text("비교 - 기준: CPU %+.3f ms | interval %+.3f ms | draw %+.0f | index %+.0f | mesh draw %+.0f",
                b.fCpuAvgMs-a.fCpuAvgMs,b.fIntervalAvgMs-a.fIntervalAvgMs,b.fDrawCallsAvg-a.fDrawCallsAvg,b.fIndicesAvg-a.fIndicesAvg,b.fMeshDrawsAvg-a.fMeshDrawsAvg);
            if (a.iGpuFrames==a.iFrames&&b.iGpuFrames==b.iFrames)
                ImGui::Text("GPU %+.3f ms | GPU p99 %+.3f ms | PS 호출 %+.0f",b.fGpuAvgMs-a.fGpuAvgMs,b.fGpuP99Ms-a.fGpuP99Ms,b.fPsInvocationsAvg-a.fPsInvocationsAvg);
            else ImGui::TextDisabled("GPU 비교 불가: 유효 GPU 표본이 전체 프레임보다 적습니다.");
            if ((a.iGpuScopeFrames!=a.iFrames || b.iGpuScopeFrames!=b.iFrames))
                ImGui::TextDisabled("GPU 패스 비교 N/A: 상세 scope 표본이 불완전합니다.");
            else if (ImGui::CollapsingHeader("GPU 패스별 비교 (부모/자식 중복 합산 금지)"))
            {
                std::map<string,std::pair<RENDERING_BENCHMARK_PASS,RENDERING_BENCHMARK_PASS>> rows;
                for (const auto& p:a.passes) rows[p.name].first=p;
                for (const auto& p:b.passes) rows[p.name].second=p;
                for (const auto& [name,p]:rows)
                    ImGui::Text("%s | self %+.3f ms | 전체 %+.3f ms | draw %+.1f | index %+.0f",name.c_str(),
                        p.second.selfMs-p.first.selfMs,p.second.inclusiveMs-p.first.inclusiveMs,p.second.drawCalls-p.first.drawCalls,p.second.indices-p.first.indices);
            }
            if (a.iCpuScopeFrames==a.iFrames && b.iCpuScopeFrames==b.iFrames && ImGui::CollapsingHeader("CPU scope 비교 (thread별 inclusive, 합산 금지)"))
            {
                std::map<string,std::pair<double,double>> rows;
                for (const auto& p:a.cpuPasses) rows[p.name].first=p.inclusiveMs;
                for (const auto& p:b.cpuPasses) rows[p.name].second=p.inclusiveMs;
                for (const auto& [name,p]:rows) ImGui::Text("%s | %+.3f ms",name.c_str(),p.second-p.first);
            }
        }
    }
    for (size_t i=0;i<m_Runs.size();++i)
    {
        const auto& r=m_Runs[i]; ImGui::PushID(static_cast<int>(i));
        const string title=std::to_string(i+1)+". "+r.strLabel+" ["+r.strVariant+"] "+(r.bConditionsStable?"입력 안정 (탐색)":"조건 변경");
        if (ImGui::TreeNode(title.c_str()))
        {
            ImGui::Text("CPU %u | GPU 유효 %u | pending %u | GPU 무효 %u | 준비 %u 프레임",r.iFrames,r.iGpuFrames,r.pendingGpuFrames,r.invalidGpuFrames,r.warmupFrames);
            ImGui::Text("CPU 평균/중앙/p95/p99/max: %.3f / %.3f / %.3f / %.3f / %.3f ms",r.fCpuAvgMs,r.fCpuP50Ms,r.fCpuP95Ms,r.fCpuP99Ms,r.fCpuMaxMs);
            if (r.iGpuFrames) ImGui::Text("GPU 평균/중앙/p95/p99/max: %.3f / %.3f / %.3f / %.3f / %.3f ms",r.fGpuAvgMs,r.fGpuP50Ms,r.fGpuP95Ms,r.fGpuP99Ms,r.fGpuMaxMs);
            else ImGui::TextDisabled("GPU N/A (미지원·무효·대기)");
            ImGui::Text("interval 평균/중앙/p95/p99/max: %.3f / %.3f / %.3f / %.3f / %.3f ms (%u 표본)",r.fIntervalAvgMs,r.fIntervalP50Ms,r.fIntervalP95Ms,r.fIntervalP99Ms,r.fIntervalMaxMs,r.iIntervalFrames);
            if (r.fIntervalAvgMs>0) ImGui::Text("관측 FPS %.2f | 목표 %.0f FPS / %.3f ms",1000.0/r.fIntervalAvgMs,m_fTargetFps,1000.0/m_fTargetFps);
            ImGui::Text("draw %.0f | mesh draw %.0f | 고유 mesh %.0f | instance %.0f | index %.0f",r.fDrawCallsAvg,r.fMeshDrawsAvg,r.fUniqueMeshesAvg,r.fInstancesAvg,r.fIndicesAvg);
            if(r.iGpuFrames) ImGui::Text("IA vertex / primitive / VS: %.0f / %.0f / %.0f (GPU 유효 %u 표본)",r.iaVerticesTotal/r.iGpuFrames,r.iaPrimitivesTotal/r.iGpuFrames,r.vsInvocationsTotal/r.iGpuFrames,r.iGpuFrames);
            if(!r.applicability.empty())ImGui::TextWrapped("%s",r.applicability.c_str());
            if(!r.rawEvidenceStatus.empty()) { ImGui::TextWrapped("원본 증거: %s",r.rawEvidenceStatus.c_str());ImGui::TextWrapped("%s",r.rawEvidencePath.c_str()); }
            if(ImGui::TreeNode("이름 있는 counters · 프레임 평균 / 전체 합"))
            {
                for(size_t n=0;n<r.counterTotals.size();++n)if(r.counterTotals[n]!=0)
                    ImGui::Text("%s: %.3f / %.0f",CProfilerCaptureIO::Counter_Name(n),r.counterTotals[n]/(std::max)(1u,r.iFrames),r.counterTotals[n]);
                ImGui::TreePop();
            }
            if(ImGui::TreeNode("CPU 작업 장부 · 프레임 평균 / 전체 ms / 호출 · 합산 금지"))
            {
                for(size_t n=0;n<r.workTotals.size();++n)if(r.workTotals[n].Calls)
                    ImGui::Text("%s: %.3f / %.3f / %llu",Engine::CProfiler::Get_WorkName(static_cast<Engine::EProfilerWork>(n)),r.workTotals[n].CpuMs/(std::max)(1u,r.iFrames),r.workTotals[n].CpuMs,static_cast<unsigned long long>(r.workTotals[n].Calls));
                ImGui::TreePop();
            }
            ImGui::Text("scope 누락 CPU %llu / GPU %llu",static_cast<unsigned long long>(r.droppedCpuScopes),static_cast<unsigned long long>(r.droppedGpuScopes));
            if (!r.strRecipeId.empty())
            {ImGui::TextWrapped("recipe %s: %s",r.strRecipeId.c_str(),r.strExperimentGoal.c_str());ImGui::TextWrapped("관찰: %s",r.strMetricGuide.c_str());ImGui::TextWrapped("해석: %s",r.strConfidence.c_str());}
            if (!r.strFailureReason.empty()) ImGui::TextWrapped("%s",r.strFailureReason.c_str());
            size_t changesShown=0;
            for(const auto& [name,value]:r.changedConditionFields)
            {ImGui::TextWrapped("%s: %s",name.c_str(),value.c_str());if(++changesShown==12)break;}
            if(r.changedConditionFields.size()>changesShown)ImGui::Text("추가 %zu개 조건 변경은 JSON에서 확인합니다.",r.changedConditionFields.size()-changesShown);
            const auto& fields=CRenderingProfileService::Experiment_Fields();
            for (size_t n=0;n<fields.size();++n) if (r.fieldMask&(uint64_t{1}<<n)) ImGui::Text("%s = %.6g",fields[n].id,r.appliedValues.values[n]);
            ImGui::TreePop();
        }
        ImGui::PopID();
    }
}

void Client::CRenderingBenchmark::Render_Section(Engine::CProfiler* profiler,
    const string& qualitySummary, CRenderingProfileService& profiles)
{
    Render_ExperimentSection(profiler,profiles);
    Render_Results();
    if(ImGui::CollapsingHeader("기법 사전 · 구현 근거"))
    {
        if (const char* recipe=RenderingTechniqueGuide::Render(m_bExperimentActive,m_bCapturing))
            Prepare_RecipeById(recipe,profiles);
        if (const char* recipe=RenderingReferenceGuide::Render(true,m_bExperimentActive,m_bCapturing))
            Prepare_RecipeById(recipe,profiles);
    }
    if(!ImGui::CollapsingHeader("고급 · 저장 프로필 비교 / 픽셀 입력")) return;
    ImGui::TextWrapped("%s",m_strStatus.c_str());
    ImGui::BeginDisabled(m_bExperimentActive);
    const bool changedProfile=Render_RestorationSection(profiles);
    const bool changedPixels=Render_PixelInputs();
    ImGui::SeparatorText("기존 재질 A/B / 현재 화면 측정");
    ImGui::BeginDisabled(m_bCapturing||changedProfile||changedPixels);
    if (ImGui::Button("현재 화면 측정")) {m_iSequenceStep=0;Begin(profiler,m_LabelBuffer.data(),static_cast<uint32_t>(m_iFrameInput),qualitySummary,m_strStatus);}
    for (const bool recovered:{false,true})
    {
        ImGui::SameLine();
        if (ImGui::Button(recovered?"원본 복원 재질 B 측정":"Legacy 재질 A 측정"))
        {
            auto& game=Engine::CGameInstance::Get(); const auto previous=game.Get_MaterialRenderSettings();
            auto selected=previous;selected.bUseSourceMaterials=recovered;
            if (SUCCEEDED(game.Apply_MaterialRenderSettings(selected)))
            {
                if (!Begin(profiler,m_LabelBuffer.data(),static_cast<uint32_t>(m_iFrameInput),qualitySummary,m_strStatus)) game.Apply_MaterialRenderSettings(previous);
                else {if (!m_bPixelDiagnosticsActive)m_PixelEntrySettings=previous;m_bPixelDiagnosticsActive=true;m_iPixelDiagnosticsLevel=game.Get_CurrentLevelID();}
            }
        }
    }
    ImGui::EndDisabled();ImGui::EndDisabled();
}

bool_t Client::CRenderingBenchmark::Save_Json(
	const vector<RENDERING_BENCHMARK_RUN>& Runs,
	const filesystem::path& OutputPath,
	string& strOutError)
{
	error_code Error;
	if (!OutputPath.parent_path().empty())
		filesystem::create_directories(OutputPath.parent_path(), Error);
	if (Error)
	{
		strOutError = Error.message();
		return false;
	}
	filesystem::path TemporaryPath = OutputPath;
	TemporaryPath += L".tmp";
	ofstream Stream(TemporaryPath, ios::binary | ios::trunc);
	if (!Stream)
	{
		strOutError = "Cannot open benchmark JSON output.";
		return false;
	}
    Stream.imbue(locale::classic());
    Stream << setprecision(17);
    Stream << "{\n  \"schema\": \"LostArkRenderingBenchmark.v2\",\n"
        "  \"timingContract\": \"CPU/GPU overlap; pass intervals inclusive; pending is not zero; dynamic gameplay is exploratory.\",\n"
        "  \"runs\": [\n";
    const auto& fields=CRenderingProfileService::Experiment_Fields();
    for (size_t iRun=0;iRun<Runs.size();++iRun)
    {
        const auto& r=Runs[iRun];
        Stream << "    {\n"
            << "      \"label\": \"" << Escape_Json(r.strLabel) << "\",\n"
            << "      \"timestamp\": \"" << Escape_Json(r.strTimestamp) << "\",\n"
            << "      \"qualitySummary\": \"" << Escape_Json(r.strQualitySummary) << "\",\n"
            << "      \"experimentId\": \"" << Escape_Json(r.strExperimentId) << "\",\n"
            << "      \"variant\": \"" << Escape_Json(r.strVariant) << "\",\n"
            << "      \"sourceMaterials\": " << (r.bSourceMaterials?"true":"false") << ",\n"
            << "      \"conditionsStable\": " << (r.bConditionsStable?"true":"false") << ",\n"
            << "      \"failureReason\": \"" << Escape_Json(r.strFailureReason) << "\",\n"
            << "      \"commonConditionFingerprint\": \"" << Escape_Json(r.strComparisonConditions) << "\",\n"
            << "      \"actualConditionFingerprint\": \"" << Escape_Json(r.strFullConditions) << "\",\n"
            << "      \"fieldMask\": " << r.fieldMask << ",\n      \"selectedFields\": [";
        bool comma=false;
        for (size_t i=0;i<fields.size();++i) if (r.fieldMask&(uint64_t{1}<<i))
        { if(comma)Stream<<',';comma=true;Stream<<'"'<<fields[i].id<<'"'; }
        Stream << "],\n      \"effectiveValues\": {";
        for (size_t i=0;i<fields.size();++i)
        { if(i)Stream<<',';Stream<<'"'<<fields[i].id<<"\":"<<r.appliedValues.values[i]; }
        Stream << "},\n"
            << "      \"firstFrame\": " << r.firstFrame << ", \"lastFrame\": " << r.lastFrame << ",\n"
            << "      \"frames\": " << r.iFrames << ", \"warmupFrames\": " << r.warmupFrames << ", \"repetition\": " << r.repetition << ",\n"
            << "      \"gpuFrames\": " << r.iGpuFrames << ", \"pendingGpuFrames\": " << r.pendingGpuFrames << ", \"invalidGpuFrames\": " << r.invalidGpuFrames << ",\n"
            << "      \"cpuScopeFrames\": " << r.iCpuScopeFrames << ", \"gpuScopeFrames\": " << r.iGpuScopeFrames << ",\n"
            << "      \"droppedCpuScopes\": " << r.droppedCpuScopes << ", \"droppedGpuScopes\": " << r.droppedGpuScopes << ",\n"
            << "      \"cpuAvgMs\": " << r.fCpuAvgMs << ", \"cpuP50Ms\": " << r.fCpuP50Ms << ", \"cpuP95Ms\": " << r.fCpuP95Ms << ", \"cpuP99Ms\": " << r.fCpuP99Ms << ", \"cpuMaxMs\": " << r.fCpuMaxMs << ",\n"
            << "      \"gpuAvgMs\": " << (r.iGpuFrames?std::to_string(r.fGpuAvgMs):"null") << ", \"gpuP50Ms\": " << (r.iGpuFrames?std::to_string(r.fGpuP50Ms):"null")
            << ", \"gpuP95Ms\": " << (r.iGpuFrames?std::to_string(r.fGpuP95Ms):"null") << ", \"gpuP99Ms\": " << (r.iGpuFrames?std::to_string(r.fGpuP99Ms):"null") << ", \"gpuMaxMs\": " << (r.iGpuFrames?std::to_string(r.fGpuMaxMs):"null") << ",\n"
            << "      \"intervalFrames\": " << r.iIntervalFrames << ", \"intervalAvgMs\": " << r.fIntervalAvgMs << ", \"intervalP50Ms\": " << r.fIntervalP50Ms << ", \"intervalP95Ms\": " << r.fIntervalP95Ms << ", \"intervalP99Ms\": " << r.fIntervalP99Ms << ", \"intervalMaxMs\": " << r.fIntervalMaxMs << ",\n"
            << "      \"drawCallsAvg\": " << r.fDrawCallsAvg << ", \"instancesAvg\": " << r.fInstancesAvg << ", \"indicesAvg\": " << r.fIndicesAvg << ",\n"
            << "      \"meshDrawsAvg\": " << r.fMeshDrawsAvg << ", \"uniqueMeshesAvg\": " << r.fUniqueMeshesAvg << ",\n"
            << "      \"psInvocationsAvg\": " << (r.iGpuFrames?std::to_string(r.fPsInvocationsAvg):"null") << ",\n";
        Stream<<"      \"applicability\": \""<<Escape_Json(r.applicability)<<"\",\n"
            <<"      \"rawEvidence\": {\"path\":\""<<Escape_Json(r.rawEvidencePath)<<"\",\"status\":\""<<Escape_Json(r.rawEvidenceStatus)
            <<"\",\"ready\":"<<(r.rawEvidenceReady?"true":"false")<<",\"schema\":\"LostArkProfilerCapture.v3\",\"firstFrame\":"<<r.firstFrame<<",\"lastFrame\":"<<r.lastFrame<<"},\n";
        Stream<<"      \"namedCounters\": {";
        for(size_t n=0;n<r.counterTotals.size();++n)
        {
            if(n)Stream<<',';
            Stream<<'"'<<CProfilerCaptureIO::Counter_Name(n)<<"\":{\"total\":"<<r.counterTotals[n]<<",\"validFrames\":"<<r.iFrames
                <<",\"average\":"<<(r.iFrames?std::to_string(r.counterTotals[n]/r.iFrames):"null")<<'}';
        }
        Stream<<"},\n      \"cpuWorkLedger\": {";
        for(size_t n=0;n<r.workTotals.size();++n)
        {
            if(n)Stream<<',';const auto& w=r.workTotals[n];
            Stream<<'"'<<Engine::CProfiler::Get_WorkName(static_cast<Engine::EProfilerWork>(n))<<"\":{\"totalMs\":"<<w.CpuMs<<",\"calls\":"<<w.Calls
                <<",\"validFrames\":"<<r.iFrames<<",\"averageMs\":"<<(r.iFrames?std::to_string(w.CpuMs/r.iFrames):"null")<<'}';
        }
        Stream<<"},\n      \"pipelineTotals\": {\"validFrames\":"<<r.iGpuFrames
            <<",\"iaVertices\":"<<(r.iGpuFrames?std::to_string(r.iaVerticesTotal):"null")
            <<",\"iaPrimitives\":"<<(r.iGpuFrames?std::to_string(r.iaPrimitivesTotal):"null")
            <<",\"vsInvocations\":"<<(r.iGpuFrames?std::to_string(r.vsInvocationsTotal):"null")<<"},\n"
            <<"      \"pipelineAverages\": {\"validFrames\":"<<r.iGpuFrames
            <<",\"iaVertices\":"<<(r.iGpuFrames?std::to_string(r.iaVerticesTotal/r.iGpuFrames):"null")
            <<",\"iaPrimitives\":"<<(r.iGpuFrames?std::to_string(r.iaPrimitivesTotal/r.iGpuFrames):"null")
            <<",\"vsInvocations\":"<<(r.iGpuFrames?std::to_string(r.vsInvocationsTotal/r.iGpuFrames):"null")<<"},\n";
        const auto writePasses=[&](const char* key,const auto& passes,bool cpu) {
            Stream << "      \"" << key << "\": [";
            for (size_t i=0;i<passes.size();++i)
            {
                const auto& p=passes[i];if(i)Stream<<',';
                Stream<<"{\"name\":\""<<Escape_Json(p.name)<<"\",\"validFrames\":"<<p.validFrames<<",\"inclusiveMs\":"<<p.inclusiveMs;
                if(!cpu)Stream<<",\"selfMs\":"<<p.selfMs<<",\"drawCalls\":"<<p.drawCalls<<",\"indices\":"<<p.indices;
                Stream<<'}';
            }
            Stream<<']';
        };
        const auto writeFields=[&](const char* key,const std::map<string,string>& values) {
            Stream<<"      \""<<key<<"\": {";bool comma=false;
            for(const auto& [name,value]:values){if(comma)Stream<<',';comma=true;Stream<<'"'<<Escape_Json(name)<<"\":\""<<Escape_Json(value)<<'"';}
            Stream<<"},\n";
        };
        Stream<<"      \"recipeId\": \""<<Escape_Json(r.strRecipeId)<<"\",\n"
            <<"      \"experimentGoal\": \""<<Escape_Json(r.strExperimentGoal)<<"\",\n"
            <<"      \"metricGuide\": \""<<Escape_Json(r.strMetricGuide)<<"\",\n"
            <<"      \"confidenceBoundary\": \""<<Escape_Json(r.strConfidence)<<"\",\n";
        Stream<<"      \"comparisonRowId\": \""<<Escape_Json(r.strComparisonRowId)<<"\",\n"
            <<"      \"measurementId\": \""<<Escape_Json(r.strMeasurementId)<<"\",\n"
            <<"      \"comparisonSessionId\": \""<<Escape_Json(r.strComparisonSessionId)<<"\",\n";
        const auto writeExpected=[&](const char* key,const RENDERING_EXPERIMENT_VALUES& values) {
            Stream<<"      \""<<key<<"\": ";
            if(r.strMeasurementId.empty()) { Stream<<"null,\n"; return; }
            Stream<<'{';
            for(size_t index=0;index<fields.size();++index)
            { if(index) Stream<<','; Stream<<'"'<<fields[index].id<<"\":"<<values.values[index]; }
            Stream<<"},\n";
        };
        writeExpected("expectedA",r.expectedA); writeExpected("expectedB",r.expectedB);
        writeFields("commonConditionFields",r.commonConditionFields);writeFields("actualConditionFields",r.actualConditionFields);
        writeFields("changedConditionFields",r.changedConditionFields);
        writePasses("cpuScopes",r.cpuPasses,true); Stream<<",\n";
        writePasses("gpuScopes",r.passes,false);
        const bool completeEvidence=OptimizationEvidenceComplete(r,Runs);
        Stream<<",\n      \"comparisonEvidenceComplete\": "<<(completeEvidence?"true":"false");
        int baseline=-1;
        const size_t candidateEnd=r.strMeasurementId.empty()?iRun:Runs.size();
        for (size_t previous=0;previous<candidateEnd;++previous)
        {
            const auto& a=Runs[previous];
            if (completeEvidence && a.strVariant=="A" && ComparisonRunsMatch(a,r) &&
                (r.strMeasurementId.empty() || r.strVariant=="B"))
            { baseline=static_cast<int>(previous);break; }
        }
        Stream<<",\n      \"deltaFromA\": ";
        if (baseline<0) Stream<<"null";
        else
        {
            const auto& a=Runs[baseline]; const bool validGpu=a.iGpuFrames==a.iFrames && r.iGpuFrames==r.iFrames;
            Stream<<"{\"runIndex\":"<<baseline<<",\"cpuAvgMs\":"<<r.fCpuAvgMs-a.fCpuAvgMs
                <<",\"cpuP99Ms\":"<<r.fCpuP99Ms-a.fCpuP99Ms<<",\"intervalAvgMs\":"<<r.fIntervalAvgMs-a.fIntervalAvgMs
                <<",\"gpuAvgMs\":"<<(validGpu?std::to_string(r.fGpuAvgMs-a.fGpuAvgMs):"null")
                <<",\"gpuP99Ms\":"<<(validGpu?std::to_string(r.fGpuP99Ms-a.fGpuP99Ms):"null")
                <<",\"drawCalls\":"<<r.fDrawCallsAvg-a.fDrawCallsAvg<<",\"indices\":"<<r.fIndicesAvg-a.fIndicesAvg
                <<",\"meshDraws\":"<<r.fMeshDrawsAvg-a.fMeshDrawsAvg<<'}';
        }
        Stream<<"\n    }"<<(iRun+1<Runs.size()?",":"")<<"\n";
    }
	Stream << "  ]\n}\n";
	Stream.close();
	if (!Stream)
	{
		filesystem::remove(TemporaryPath, Error);
		strOutError = "Failed while writing benchmark JSON.";
		return false;
	}
    // Win32 explicitly omits REPLACE_EXISTING: a name collision preserves the
    // old capture instead of allowing filesystem::rename to replace it.
    if (!MoveFileExW(TemporaryPath.c_str(),OutputPath.c_str(),MOVEFILE_WRITE_THROUGH))
    {
        const auto failure=GetLastError(); filesystem::remove(TemporaryPath,Error);
        strOutError="Cannot finalize benchmark JSON (Win32 "+std::to_string(failure)+"). Existing output preserved.";
        return false;
    }
	return true;
}

filesystem::path Client::CRenderingBenchmark::Make_DefaultPath()
{
	wchar_t ModulePath[32768]{};
	const DWORD iLength = GetModuleFileNameW(
		nullptr, ModulePath, static_cast<DWORD>(size(ModulePath)));
	const filesystem::path BaseDirectory =
		0 != iLength && iLength < size(ModulePath) ?
		filesystem::path(ModulePath).parent_path() : filesystem::current_path();
	const auto Now = chrono::system_clock::now();
	const time_t CalendarTime = chrono::system_clock::to_time_t(Now);
	tm LocalTime{};
	localtime_s(&LocalTime, &CalendarTime);
	wostringstream FileName;
	static uint64_t fileSequence=0;
    FileName << L"benchmark_" << put_time(&LocalTime, L"%Y%m%d_%H%M%S") << L'_' << GetCurrentProcessId()
        << L'_' << GetTickCount64() << L'_' << ++fileSequence << L".json";
	return (BaseDirectory / L".." / L"BenchmarkCaptures" / FileName.str())
		.lexically_normal();
}
