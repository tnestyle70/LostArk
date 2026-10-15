#include "MapGpuScene.h"
#include "Engine_RenderTypes.h"
#include "MapStaticBatchObject.h"
#include "MapStaticChunkObject.h"
#include "MapAssetRenderUtils.h"
#include "Model.h"
#include "Shader.h"
#include "Profiler.h"
#include <array>
#include <cmath>
#include <cstring>
#include <unordered_map>

namespace
{
    bool SameProfile(const MAP_ASSET_RENDER_PROFILE& a,const MAP_ASSET_RENDER_PROFILE& b)
    {
        return a.renderMode==b.renderMode && a.cullMode==b.cullMode &&
            a.uvScale.x==b.uvScale.x && a.uvScale.y==b.uvScale.y && a.uvSpeed.x==b.uvSpeed.x && a.uvSpeed.y==b.uvSpeed.y &&
            a.opacity==b.opacity && a.opacityPower==b.opacityPower && a.emissiveIntensity==b.emissiveIntensity &&
            a.specularIntensity==b.specularIntensity && a.specularPower==b.specularPower &&
            a.colorTint.x==b.colorTint.x && a.colorTint.y==b.colorTint.y && a.colorTint.z==b.colorTint.z && a.colorTint.w==b.colorTint.w &&
            a.triplanarHeightScale==b.triplanarHeightScale && a.castsShadow==b.castsShadow;
    }
    VTXMESHINSTANCE Payload(const FMapStaticInstance& instance,uint32_t bank,bool hasBank)
    {
        VTXMESHINSTANCE out{};
        out.World=instance.World; out.WorldInvTranspose=instance.WorldInvTranspose;
        out.vLightmapScaleBias=instance.BakedLighting.scaleBias;
        out.vLightmapAverageScale=instance.BakedLighting.averageScale;
        out.vLightmapDirectionalScale=instance.BakedLighting.directionalScale;
        if(hasBank) out.vLightmapDirectionalScale.w=static_cast<float>(bank);
        out.vStaticShadowScaleBias=instance.BakedLighting.shadowScaleBias;
        out.vSourceWindOwnerPosition=instance.SourceWind.actorPositionSourceCm;
        out.vSourceWindDimensionsAndRadius=instance.SourceWind.objectDimensionsAndRadiusSourceCm;
        return out;
    }
}

bool CMapGpuScene::Eligible(const CMapStaticBatchObject& batch)
{
    if (!batch.m_FrameState || batch.m_bGpuMutableTransform || !batch.m_pModelCom || !batch.m_pShaderCom || batch.m_Instances.empty() ||
        batch.m_FrustumCulling.bypass || batch.m_FrustumCulling.diagnostics ||
        batch.m_RenderProfile.renderMode != MAP_ASSET_RENDER_MODE::DEFERRED || batch.m_RenderProfile.opacity != 1.f ||
        !batch.m_pModelCom->Can_BatchStaticLightingWith(*batch.m_pModelCom)) return false;
    for(uint32_t mesh=0;mesh<batch.m_pModelCom->Get_NumMeshes();++mesh)
    {
        const auto* surface=batch.m_pModelCom->Get_MaterialSurface(mesh);
        if(!surface || surface->sourceFoliageWind || surface->family!=Engine::MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED ||
            (surface->renderMode!=Engine::MODEL_SURFACE_RENDER_MODE::INHERIT && surface->renderMode!=Engine::MODEL_SURFACE_RENDER_MODE::DEFERRED) ||
            batch.m_pModelCom->Has_MaterialTextureOverrides(mesh)) return false;
    }
    return true;
}

std::shared_ptr<CMapGpuScene> CMapGpuScene::Create(
    std::span<const std::shared_ptr<CMapStaticBatchObject>> batches,
    const std::shared_ptr<MAP_STATIC_BATCH_FRAME_STATE>& frameState)
{
    if(!frameState || batches.empty()) return {};
    try
    {
        auto scene=std::shared_ptr<CMapGpuScene>(new CMapGpuScene);
        scene->m_FrameState=frameState;
        std::unordered_map<std::string,std::vector<uint32_t>> byGeometry;
        for(const auto& batch:batches)
        {
            if(!batch || !Eligible(*batch)) continue;
            const auto& hash=batch->m_pModelCom->Get_GeometryPayloadSha256();
            std::string key(reinterpret_cast<const char*>(hash.data()),hash.size());
            key.push_back(batch->m_bMirrored ? 1 : 0);
            auto& candidates=byGeometry[key];
            bool inserted=false;
            for(uint32_t index:candidates)
            {
                auto& cohort=scene->m_Cohorts[index];
                auto first=cohort.Members.front().Batch.lock();
                if(!first || !SameProfile(first->m_RenderProfile,batch->m_RenderProfile) ||
                    !first->m_pModelCom->Can_BatchStaticLightingWith(*batch->m_pModelCom)) continue;
                uint32_t bank=0;
                for(;bank<cohort.Banks.size();++bank)
                    if(cohort.Banks[bank]->Has_SameStaticLightingTextures(*batch->m_pModelCom)) break;
                if(bank==cohort.Banks.size())
                {
                    if(bank==8) continue;
                    cohort.Banks.push_back(batch->m_pModelCom);
                }
                cohort.Members.push_back({batch,bank,0}); inserted=true; break;
            }
            if(!inserted)
            {
                candidates.push_back(static_cast<uint32_t>(scene->m_Cohorts.size()));
                COHORT cohort;cohort.Members.push_back({batch,0,0});cohort.Banks.push_back(batch->m_pModelCom);
                scene->m_Cohorts.push_back(std::move(cohort));
            }
        }
        // 원본 draw 하나를 여러 LOD bin으로 늘리지 않는다. singleton은 기존 경로가 더 싸다.
        std::erase_if(scene->m_Cohorts,[](const auto& cohort){return cohort.Members.size()<2;});
        if(scene->m_Cohorts.empty()) return {};
        auto first=scene->m_Cohorts.front().Members.front().Batch.lock();
        scene->m_Gpu=std::make_shared<Engine::CGpuSceneVisibility>(first->m_pDevice.Get(),first->m_pContext.Get());
        if(FAILED(scene->m_Gpu->Initialize()) || FAILED(scene->Build_Resident())) return {};
        const std::weak_ptr<CMapGpuScene> weak=scene;
        scene->m_Gpu->Set_FramePrepareCallback([weak](const auto& frame)
        { auto owner=weak.lock();return owner ? owner->Prepare_Frame(frame) : S_FALSE; });
        for(uint32_t index=0;index<scene->m_Cohorts.size();++index)
            for(const auto& member:scene->m_Cohorts[index].Members)
                if(auto batch=member.Batch.lock())
                { batch->m_GpuScene=scene;batch->m_iGpuCohort=index;batch->m_bGpuMergedVisibility=true; }
        CGameInstance::Get().Register_GpuSceneVisibility(scene->m_Gpu);
        return scene;
    }
    catch(...) { return {}; }
}

CMapGpuScene::~CMapGpuScene()
{
    if(m_Gpu) CGameInstance::Get().Unregister_GpuSceneVisibility(m_Gpu.get());
    for(const auto& cohort:m_Cohorts)
        for(const auto& member:cohort.Members)
            if(auto batch=member.Batch.lock())
            { batch->m_bGpuMergedVisibility=false;batch->m_GpuScene.reset(); }
}

bool CMapGpuScene::Enabled() const
{
    auto& game=CGameInstance::Get();const auto optimization=game.Get_RenderOptimizationSettings();
    return m_Gpu && game.Get_MapVisibilitySettings().GpuVisibilityEnabled && optimization.MapInstancingEnabled &&
        optimization.IdenticalBatchEnabled && optimization.LightingBankEnabled &&
        game.Get_MaterialRenderSettings().bUseSourceMaterials && !game.Is_SceneEnvironmentReplaced() &&
        !CMapAssetRenderUtils::Is_SurfaceBindingCollectionActive();
}
bool CMapGpuScene::Can_Use(const CMapStaticBatchObject& batch) const
{ return m_ResidentValid && Enabled() && batch.m_bGpuMergedVisibility && batch.m_iGpuCohort<m_Cohorts.size() && !m_Cohorts[batch.m_iGpuCohort].Draws.empty(); }
bool CMapGpuScene::Is_Ready(const CMapStaticBatchObject& batch) const
{
    if (!Can_Use(batch) || !m_Frame || !m_Cohorts[batch.m_iGpuCohort].Active) return false;
    Engine::CGpuSceneVisibility::DRAW_OUTPUT output;
    return m_Gpu->Get_Draw(m_Cohorts[batch.m_iGpuCohort].Draws.front().Bin, m_Frame, output);
}
void CMapGpuScene::Mark_Submitted(CMapStaticBatchObject& batch)
{ batch.m_iGpuSubmittedFrame=m_FrameState->frameNumber; }
void CMapGpuScene::Invalidate() noexcept
{ m_ResidentValid=false; m_Frame=0; if(m_Gpu)m_Gpu->Invalidate_Frame(); }

HRESULT CMapGpuScene::Build_Resident()
{
    Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(),"Map.GpuScene.ResidentBuild");
    const auto settings=CGameInstance::Get().Get_MapVisibilitySettings();
    const auto optimization=CGameInstance::Get().Get_RenderOptimizationSettings();
    std::vector<Engine::CGpuSceneVisibility::SOURCE_INSTANCE> sources;
    std::vector<VTXMESHINSTANCE> payload;
    std::vector<Engine::CGpuSceneVisibility::DRAW_BIN> bins;
    auto staged=m_Cohorts;
    uint32_t activeCount=0;
    for(auto& cohort:staged)
    {
        cohort.Draws.clear();cohort.ActiveMeshes.clear();cohort.Active=false;cohort.DrawnFrame=0;
        auto first=cohort.Members.front().Batch.lock();
        bool compatible=first && Eligible(*first);
        if(compatible)
            for(const auto& member:cohort.Members)
            {
                auto batch=member.Batch.lock();
                compatible = compatible && batch && Eligible(*batch) && SameProfile(first->m_RenderProfile,batch->m_RenderProfile) &&
                    batch->m_bMirrored==first->m_bMirrored && first->m_pModelCom->Can_BatchStaticLightingWith(*batch->m_pModelCom) &&
                    cohort.Banks[member.Bank]->Has_SameStaticLightingTextures(*batch->m_pModelCom);
            }
        // 런타임 TRS가 변하는 소수 cohort는 기존 CPU 경로로 돌려 매프레임 scene 전체 재업로드를 피한다.
        if(!compatible) continue;
        const uint32_t meshCount=first->m_pModelCom->Get_NumMeshes();
        cohort.ActiveMeshes.assign(meshCount,0u);
        uint32_t capacity=0;
        for(auto& member:cohort.Members)
        {
            auto batch=member.Batch.lock();
            if(!batch || !Eligible(*batch) || !SameProfile(first->m_RenderProfile,batch->m_RenderProfile) ||
                batch->m_bMirrored!=first->m_bMirrored || !first->m_pModelCom->Can_BatchStaticLightingWith(*batch->m_pModelCom) ||
                !cohort.Banks[member.Bank]->Has_SameStaticLightingTextures(*batch->m_pModelCom)) return E_FAIL;
            // HLOD 소유권은 mesh별이다. instance 단일 bit로 전 mesh를 함께 끄면
            // HLOD가 대체하지 않은 submesh까지 사라지므로 mesh별 연속 구간을 둔다.
            if(batch->m_Instances.size()>UINT32_MAX-capacity || !meshCount ||
                batch->m_Instances.size()>(UINT32_MAX-activeCount)/meshCount) return E_INVALIDARG;
            member.ActiveFirst=activeCount;activeCount+=static_cast<uint32_t>(batch->m_Instances.size())*meshCount;
            capacity+=static_cast<uint32_t>(batch->m_Instances.size());
        }
        for(uint32_t mesh=0;mesh<first->m_pModelCom->Get_NumMeshes();++mesh)
        {
            struct Range {uint32_t Level,Count,First;float Error;};
            std::vector<Range> available;
            for(uint32_t level=0;level<5;++level)
            {
                uint32_t count=0,index=0;float error=0;
                if(first->m_pModelCom->Get_StaticMeshLodRange(mesh,level,count,index,error)) available.push_back({level,count,index,error});
            }
            if(available.empty() || available.front().Level!=0) return E_FAIL;
            std::vector<Range> selected{available.front()};
            if(available.size()>1)selected.push_back(available.back());
            for(size_t i=1;i+1<available.size() && selected.size()<cohort.Members.size();++i) selected.push_back(available[i]);
            std::sort(selected.begin(),selected.end(),[](const auto&a,const auto&b){return a.Level<b.Level;});
            uint32_t lodBins[5]{UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX};float errors[5]{};
            for(const auto& range:selected)
            {
                const uint32_t bin=static_cast<uint32_t>(bins.size());
                bins.push_back({range.Count,range.First,0,capacity});cohort.Draws.push_back({mesh,range.Level,bin,range.Count});
                lodBins[range.Level]=bin;errors[range.Level]=range.Error;
            }
            for(const auto& member:cohort.Members)
            {
                auto batch=member.Batch.lock();
                const auto* surface=batch->m_pModelCom->Get_MaterialSurface(mesh);
                for(uint32_t index=0;index<batch->m_Instances.size();++index)
                {
                    const auto& instance=batch->m_Instances[index];
                    Engine::CGpuSceneVisibility::SOURCE_INSTANCE source;
                    float margin=batch->m_FrustumCulling.baseMargin;
                    if(batch->m_AssetGroupId=="landscape" || instance.WorldBoundsRadius>=batch->m_FrustumCulling.largeObjectRadiusThreshold)
                        margin+=(std::max)(batch->m_FrustumCulling.largeObjectAbsoluteMargin,
                            instance.WorldBoundsRadius*batch->m_FrustumCulling.largeObjectRelativeMargin);
                    source.WorldSphere={instance.WorldBoundsCenter.x,instance.WorldBoundsCenter.y,instance.WorldBoundsCenter.z,
                        instance.WorldBoundsRadius+(std::max)(0.f,margin)};
                    for(uint32_t lod=0;lod<5;++lod)
                    {source.LodBins[lod]=lodBins[lod];source.WorldErrors[lod]=errors[lod]*batch->m_InstanceLinearScaleBounds[index];}
                    float distance=0,pixels=0;
                    if(CMapAssetRenderUtils::Get_SceneDistancePolicy(batch->m_SceneVisibilityKind,
                        instance.WorldBoundsRadius,settings,distance,pixels))
                    {source.DistanceLimit=distance*1.1f/settings.DistanceScale;source.MaximumPixels=pixels*.9f;}
                    source.ActiveSource=member.ActiveFirst+mesh*static_cast<uint32_t>(batch->m_Instances.size())+index;
                    source.Flags=(!surface || surface->sourceFoliageWind || (surface->sourceBgFlags&64u)!=0 ||
                        batch->m_pModelCom->Has_MaterialTexture(mesh,aiTextureType_OPACITY)) ? 1u : 0u;
                    sources.push_back(source);payload.push_back(Payload(instance,member.Bank,cohort.Banks.size()>1));
                }
            }
        }
    }
    if(sources.empty())return E_FAIL;
    const auto bytes=std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(payload.data()),payload.size()*sizeof(VTXMESHINSTANCE));
    const HRESULT hr=m_Gpu->Stage(sources,bytes,bins,activeCount,sizeof(VTXMESHINSTANCE));
    if(FAILED(hr))return hr;
    m_Cohorts.swap(staged);m_Active.assign(activeCount,0);m_SettingsRevision=settings.Revision;
    for(const auto& cohort:m_Cohorts)
        for(const auto& member:cohort.Members)
            if(auto batch=member.Batch.lock()) batch->m_bGpuMergedVisibility=!cohort.Draws.empty();
    m_OptimizationRevision=optimization.Revision;m_ResidentValid=true;return S_OK;
}

bool CMapGpuScene::Can_ReplaceHlodMesh(const CMapStaticBatchObject& batch, uint32_t mesh,
    const MAP_CHUNK_CLAIM& claim, std::span<const uint8_t> candidates,
    const MAP_CAMERA_CULL_SNAPSHOT& camera) const
{
    if(!Can_Use(batch))return false;
    bool anyColorCandidate=false;
    for(const auto& member:m_Cohorts[batch.m_iGpuCohort].Members)
    {
        auto peer=member.Batch.lock();
        if(!peer)return false;
        // final-camera Layer의 처리 순서와 무관하게 같은 보수적 색상 bounds를 쓴다.
        // 현재 후보 밖 원본이 하나라도 남으면 global GPU draw는 사라지지 않는다.
        if(!peer->Is_FinalCameraColorCandidate(&camera))continue;
        anyColorCandidate=true;
        if(mesh>=peer->m_ChunkClaims.size() || peer->m_ChunkClaims[mesh].get()!=&claim ||
            mesh>=peer->m_ChunkClaimMemberIndices.size())return false;
        const auto index=peer->m_ChunkClaimMemberIndices[mesh];
        if(index>=candidates.size() || !candidates[index])return false;
    }
    return anyColorCandidate;
}

uint32_t CMapGpuScene::Get_HlodMeshIndexLimit(const CMapStaticBatchObject& batch, uint32_t mesh) const
{
    if(!Can_Use(batch))return 0u;
    uint32_t limit=UINT32_MAX;
    const bool lodEnabled=CGameInstance::Get().Get_RenderOptimizationSettings().MeshLodEnabled;
    // GPU는 instance별로 batch CPU LOD보다 작은 단계를 고를 수 있다. 상주한
    // 모든 단계 중 최소 index 수를 하한으로 써 shader float 판정을 복제하지 않는다.
    // 실제 화면에서는 다소 보수적으로 탈락해도 HLOD가 원본 triangle을 늘리지는 않는다.
    for(const auto& draw:m_Cohorts[batch.m_iGpuCohort].Draws)
        if(draw.Mesh==mesh && (lodEnabled || !draw.Lod))limit=(std::min)(limit,draw.IndexCount);
    return limit==UINT32_MAX ? 0u : limit;
}

HRESULT CMapGpuScene::Prepare_Frame(const Engine::CGpuSceneVisibility::FRAME& frame)
{
    m_Frame=0;
    if(!Enabled())return S_FALSE;
    auto& game=CGameInstance::Get();
    if(!m_ResidentValid || m_SettingsRevision!=game.Get_MapVisibilitySettings().Revision ||
        m_OptimizationRevision!=game.Get_RenderOptimizationSettings().Revision)
    {
        const HRESULT hr=Build_Resident();if(FAILED(hr)){m_ResidentValid=false;return S_FALSE;}
    }
    Engine::CProfilerScope scope(game.Get_Profiler(),"Map.GpuScene.ActiveUpload");
    std::fill(m_Active.begin(),m_Active.end(),0u);
    bool hasGpuWork = false;
    uint32_t activeOwners = 0u;
    for(auto& cohort:m_Cohorts)
    {
        cohort.Active = false;
        std::fill(cohort.ActiveMeshes.begin(),cohort.ActiveMeshes.end(),uint8_t(0));
        if(cohort.Draws.empty())continue;
        uint64_t separateDraws = 0u;
        uint32_t cohortOwners = 0u;
        for(const auto& member:cohort.Members)
        {
            auto batch=member.Batch.lock();
            if(!batch || batch->m_iGpuSubmittedFrame!=m_FrameState->frameNumber)continue;
            if(!Eligible(*batch)){Invalidate();return S_FALSE;}
            const auto meshCount=batch->m_pModelCom->Get_NumMeshes();
            const auto instanceCount=static_cast<uint32_t>(batch->m_Instances.size());
            bool memberActive=false;
            for(uint32_t mesh=0u;mesh<meshCount;++mesh)
            {
                // HLOD는 이 mesh만 소유한다. 다른 mesh/cohort를 CPU로 풀지 않는다.
                if(batch->Is_ChunkMeshClaimed(mesh))continue;
                ++separateDraws;
                for(uint32_t i=0u;i<instanceCount;++i)
                {
                    const auto& instance=batch->m_Instances[i];
                    const uint32_t active=instance.Visible && !instance.Suppressed && !instance.CameraPreviewSuppressed && !instance.ColorProxySuppressed;
                    m_Active[member.ActiveFirst+mesh*instanceCount+i]=active;
                    cohort.ActiveMeshes[mesh]|=static_cast<uint8_t>(active);
                    memberActive |= active!=0u;
                }
            }
            // HiZ admission은 mesh 수가 아닌 기존 고유 instance 분모를 유지한다.
            if(memberActive)
                for(const auto& instance:batch->m_Instances)
                    cohortOwners+=instance.Visible && !instance.Suppressed && !instance.CameraPreviewSuppressed && !instance.ColorProxySuppressed;
        }
        if(!separateDraws)continue;
        const uint64_t indirectDraws=static_cast<uint64_t>(std::count_if(cohort.Draws.begin(),cohort.Draws.end(),
            [&](const DRAW& draw){return cohort.ActiveMeshes[draw.Mesh]!=0u;}));
        if(!indirectDraws)continue;
        if(indirectDraws>=separateDraws)
        {
            // 남은 mesh만 비교한다. CPU 복귀 cohort의 mask는 반드시 지워
            // 다른 cohort의 성공 때문에 불필요한 GPU source가 함께 실행되지 않게 한다.
            Engine::CProfilerScope fallbackScope(game.Get_Profiler(),"Map.GpuScene.CpuFallback");
            const auto* camera=CMapAssetRenderUtils::Capture_CameraCullSnapshotView();
            for(const auto& member:cohort.Members)
                if(auto batch=member.Batch.lock())
                {
                    const size_t count=batch->m_Instances.size()*cohort.ActiveMeshes.size();
                    std::fill_n(m_Active.begin()+member.ActiveFirst,count,0u);
                    if(batch->m_iGpuSubmittedFrame==m_FrameState->frameNumber && !batch->Are_AllChunkMeshesClaimed() &&
                        FAILED(batch->Prepare_FinalCameraVisibility(camera)))return S_FALSE;
                }
            continue;
        }
        auto first=cohort.Members.front().Batch.lock();
        if(!first || !Eligible(*first)){Invalidate();return S_FALSE;}
        for(const auto& member:cohort.Members)
        {
            auto batch=member.Batch.lock();
            if(!batch || batch->m_iGpuSubmittedFrame!=m_FrameState->frameNumber)continue;
            if(!Eligible(*batch) || !first->m_pModelCom->Can_BatchStaticLightingWith(*batch->m_pModelCom) ||
                !cohort.Banks[member.Bank]->Has_SameStaticLightingTextures(*batch->m_pModelCom))
            {Invalidate();return S_FALSE;}
        }
        cohort.Active=true;hasGpuWork=true;activeOwners+=cohortOwners;
    }
    if(!hasGpuWork)return S_FALSE;
    const HRESULT hr=m_Gpu->Set_ActiveSources(m_Active,activeOwners);
    if(SUCCEEDED(hr))m_Frame=frame.FrameId;
    return hr;
}

HRESULT CMapGpuScene::Try_Render(CMapStaticBatchObject& batch)
{
    if(!Can_Use(batch) || !m_Frame)return S_FALSE;
    auto& cohort=m_Cohorts[batch.m_iGpuCohort];
    Engine::CGpuSceneVisibility::DRAW_OUTPUT output;
    // Active는 이번 프레임에 GPU를 선택한 cohort만 의미한다. 소수 cell은 CPU로
    // 돌아가며 이전 GPU FrameResult를 재사용하거나 성공 처리해 원본을 누락하지 않는다.
    if(!cohort.Active || cohort.Draws.empty() || !m_Gpu->Get_Draw(cohort.Draws.front().Bin,m_Frame,output))return S_FALSE;
    if(cohort.DrawnFrame==m_Frame)return cohort.FrameResult;
    // 현재 GPU 결과를 소비하기로 확정한 뒤에는 member 하나만 CPU로 재시도하지 않는다.
    // 앞선 원본 draw와 뒤 cohort draw가 중복되는 것을 막기 위해 실패도 cohort 전체에 남긴다.
    cohort.DrawnFrame = m_Frame;
    cohort.FrameResult = E_FAIL;
    const auto& model = cohort.Banks.front();
    const auto* camera=CMapAssetRenderUtils::Capture_CameraCullSnapshotView();
    if(!camera)return E_FAIL;
    auto* profiler = CGameInstance::Get().Get_Profiler();
    Engine::CProfilerScope scope(profiler,"Map.GpuScene.IndirectRender");
    Engine::CProfilerWorkScope renderWork(profiler,Engine::EProfilerWork::MapBatchRender);
    std::array<const Engine::CModel*,8> models{};
    for(size_t i=0;i<cohort.Banks.size();++i)models[i]=cohort.Banks[i].get();
    const uint32_t pass=CMapAssetRenderUtils::Select_Pass(batch.m_RenderProfile,batch.m_bMirrored);
    if(pass>2 || FAILED(CMapAssetRenderUtils::Bind_CameraCullSnapshot(batch.m_pShaderCom,*camera)))return E_FAIL;
    uint32_t submitted=0,lastMesh=UINT32_MAX;
    const uint32_t noBank=0;
    if(FAILED(batch.m_pShaderCom->Bind_RawValue("g_MapLightingBankSize",&noBank,sizeof(noBank))))return E_FAIL;
    for(const auto& draw:cohort.Draws)
    {
        if(!cohort.ActiveMeshes[draw.Mesh])continue;
        if(!m_Gpu->Get_Draw(draw.Bin,m_Frame,output))return E_FAIL;
        if(draw.Mesh!=lastMesh)
        {
            HRESULT hr = S_OK;
            {
            Engine::CProfilerDetailScope materialScope(profiler,"Map.Batch.Material.Bind");
            Engine::CProfilerWorkScope materialWork(profiler,Engine::EProfilerWork::MapBatchMaterial);
            hr=CMapAssetRenderUtils::Bind_Material(model,batch.m_pShaderCom,draw.Mesh,
                batch.m_RenderProfile,batch.Get_RenderElapsedTime(),nullptr,batch.m_AssetId,nullptr,nullptr,MAP_MATERIAL_BINDING_MODE::INSTANCED);
            if(SUCCEEDED(hr) && cohort.Banks.size()>1)
                hr=model->Bind_StaticLightingBankMesh(batch.m_pShaderCom,
                    std::span<const Engine::CModel* const>(models.data(),cohort.Banks.size()),draw.Mesh);
            }
            if(SUCCEEDED(hr))
            {
                Engine::CProfilerDetailScope passScope(profiler,"Map.Batch.Pass.Apply");
                Engine::CProfilerWorkScope passWork(profiler,Engine::EProfilerWork::MapBatchPass);
                hr=batch.m_pShaderCom->Begin((cohort.Banks.size()==1 ? 24u : cohort.Banks.size()<4 ? 30u : 27u)+pass);
            }
            if(FAILED(hr))
            { (void)batch.m_pShaderCom->Bind_RawValue("g_MapLightingBankSize",&noBank,sizeof(noBank));return hr; }
            lastMesh=draw.Mesh;
        }
        {
            Engine::CProfilerDetailScope drawScope(profiler,"Map.Batch.Mesh.Submit");
            Engine::CProfilerWorkScope drawWork(profiler,Engine::EProfilerWork::MapBatchDraw);
            const HRESULT hr=model->Render_InstancedIndirect(draw.Mesh,output.InstanceBuffer,
                output.Stride,output.InstanceByteOffset,output.Arguments,output.ArgumentByteOffset,draw.Lod);
            if(FAILED(hr))return hr;
        }
        // args의 생존 수는 GPU 소유다. 성공한 명령/LOD만 기록하고 count는
        // profiler의 비동기 args 결과와 frame+buffer ordinal+offset으로 결합한다.
        if (m_FrameState && m_FrameState->geometryDiagnostics)
        {
            auto& diagnostics = *m_FrameState->geometryDiagnostics;
            try
            {
                MAP_GEOMETRY_DIAGNOSTIC_DRAW row;
                row.path = "gpu_indirect"; row.assetId = batch.m_AssetId;
                row.meshIndex = draw.Mesh; row.lod = draw.Lod;
                row.indexCount = draw.IndexCount;
                row.vertexBufferCount = model->Get_MeshVertexCount(draw.Mesh);
                row.instanceCountKnown = false;
                row.argsByteOffset = output.ArgumentByteOffset;
                row.cohortId = batch.m_iGpuCohort; row.bin = draw.Bin; row.gpuFrameId = m_Frame;
                if (profiler)
                {
                    row.geometryFrameNumber = profiler->Get_GeometryFrameNumber();
                    row.indirectBufferOrdinal = profiler->Get_GeometryIndirectBufferOrdinal(output.Arguments);
                }
                diagnostics.Record(std::move(row));
            }
            catch (...) { ++diagnostics.droppedDraws; }
        }
        ++submitted;
    }
    cohort.FrameResult = batch.m_pShaderCom->Bind_RawValue("g_MapLightingBankSize",&noBank,sizeof(noBank));
    return cohort.FrameResult;
}
