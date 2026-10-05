#include "MapStaticChunkObject.h"
#pragma push_macro("new")
#undef new
#include "Engine_RenderTypes.h"
#pragma pop_macro("new")
#include "MapStaticBatchObject.h"
#include "MapAssetRenderUtils.h"
#include "MapStaticProxyBaker.h"
#include "GameInstance.h"
#include "Model.h"
#include "MeshLod.h"
#include "Shader.h"
#include "Profiler.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <map>
#include <tuple>

namespace
{
    // Admission cannot save two draws unless at least three original mesh
    // groups exist. Share this limit with preparation to avoid dead chunks.
    constexpr uint32_t MINIMUM_CHUNK_SOURCE_DRAWS = 3u;
    constexpr uint32_t MINIMUM_PROXY_BUILD_DRAWS = 16u;
    constexpr uint32_t MINIMUM_PROXY_VISIBLE_DRAWS = 8u;
    constexpr float PROXY_CELL_METRES = 64.f;

    // A world-space texel must cover at most one output pixel. Bound the
    // perspective derivative over the entire source sphere, including off-axis
    // depth variation; unsupported/near-plane projections retain the source.
    bool ProxyDensityFits(const MAP_CAMERA_CULL_SNAPSHOT& camera, const float2_t& viewport,
        const float3_t& center, float radius, float texelsPerUnit)
    {
        const auto& p = camera.projection;
        if (!(camera.lodViewScale > 0.f) || !(texelsPerUnit > 0.f) || !(radius >= 0.f) ||
            p._14 != 0.f || p._24 != 0.f || p._34 != 1.f || p._44 != 0.f ||
            p._12 != 0.f || p._21 != 0.f || p._13 != 0.f || p._23 != 0.f ||
            p._41 != 0.f || p._42 != 0.f || !(p._11 > 0.f) || !(p._22 > 0.f) ||
            !(p._33 > 1.f) || !(p._43 < 0.f)) return false;
        float3_t view{};
        XMStoreFloat3(&view, XMVector3TransformCoord(XMLoadFloat3(&center), XMLoadFloat4x4(&camera.lodView)));
        const double r = double(radius) * camera.lodViewScale;
        const double depth = double(view.z) - r;
        if (!(depth > -double(p._43) / p._33)) return false;
        const double lateral = (std::max)(std::abs(double(view.x)), std::abs(double(view.y))) + r;
        const double pixels = (std::max)(double(p._11) * viewport.x, double(p._22) * viewport.y) * .5;
        const double density = 1.4142135623730951 * pixels * camera.lodViewScale * (depth + lateral) / (depth * depth);
        return std::isfinite(density) && density > 0. && density <= texelsPerUnit;
    }

    bool SameProfile(const MAP_ASSET_RENDER_PROFILE& a, const MAP_ASSET_RENDER_PROFILE& b)
    {
        return a.renderMode == b.renderMode && a.cullMode == b.cullMode &&
            a.uvScale.x == b.uvScale.x && a.uvScale.y == b.uvScale.y &&
            a.uvSpeed.x == b.uvSpeed.x && a.uvSpeed.y == b.uvSpeed.y &&
            a.opacity == b.opacity && a.opacityPower == b.opacityPower &&
            a.emissiveIntensity == b.emissiveIntensity && a.specularIntensity == b.specularIntensity &&
            a.specularPower == b.specularPower && a.colorTint.x == b.colorTint.x &&
            a.colorTint.y == b.colorTint.y && a.colorTint.z == b.colorTint.z && a.colorTint.w == b.colorTint.w &&
            a.triplanarHeightScale == b.triplanarHeightScale;
    }
    uint64_t GeometryBytes(const Engine::CModel::STATIC_CLUSTER_STATS& s)
    {
        return uint64_t(s.clusterVertices) * (sizeof(VTXMESH) + sizeof(uint32_t)) +
            uint64_t(s.nearIndices + uint64_t(s.hasFarGeometry ? s.farIndices : 0)) * sizeof(uint32_t) +
            uint64_t(s.sourceCount) * sizeof(VTXMESHINSTANCE);
    }
}

bool MAP_CHUNK_CLAIM::Is_Active(uint32_t member) const
{
    if (!valid) return false;
    if (owner) owner->Resolve_RenderSelection();
    return valid && frameActive && (member == UINT32_MAX ||
        (member < activeMembers.size() && activeMembers[member] != 0u));
}

CMapStaticChunkObject::CMapStaticChunkObject(ComPtr<ID3D11Device> device, ComPtr<ID3D11DeviceContext> context)
    : CGameObject(device, context) {}
CMapStaticChunkObject::CMapStaticChunkObject(const CMapStaticChunkObject& prototype) : CGameObject(prototype) {}
CMapStaticChunkObject::~CMapStaticChunkObject()
{
    if (m_Claim && m_Claim->owner == this)
    { m_Claim->owner = nullptr; m_Claim->frameActive = false; }
}

HRESULT CMapStaticChunkObject::Initialize(void* arg)
{
    if (!arg) return E_INVALIDARG;
    const auto& d = *static_cast<const DESC*>(arg);
    if (!d.model || !d.claim || (d.proxy ? !d.model->Get_StaticProxyStats() : !d.model->Get_StaticClusterStats()) ||
        FAILED(__super::Initialize(arg))) return E_INVALIDARG;
    if (FAILED(__super::Add_Component(d.prototypeLevelIndex,
        d.proxy ? CMapStaticProxyBaker::ShaderTag : ShaderTag, L"Com_Shader", m_Shader))) return E_FAIL;
    m_Model = d.model; m_Claim = d.claim; m_TimeSource = d.timeSource; m_Profile = d.profile; m_Culling = d.culling;
    m_Mirrored = d.mirrored; m_SourceDraws = d.sourceDraws; m_Proxy = d.proxy;
    m_Members = d.members; m_Claim->owner = this;
#ifdef _DEBUG
    m_SourceAssetIds = d.sourceAssetIds;
#endif
    float3_t minimum{}, maximum{};
    if (m_Proxy)
    {
        const auto ranges = m_Model->Get_StaticProxySourceRanges();
        if (ranges.empty() || m_Members.empty()) return E_INVALIDARG;
        minimum = ranges.front().boundsMin; maximum = ranges.front().boundsMax;
        for (const auto& range : ranges)
        {
            XMStoreFloat3(&minimum, XMVectorMin(XMLoadFloat3(&minimum), XMLoadFloat3(&range.boundsMin)));
            XMStoreFloat3(&maximum, XMVectorMax(XMLoadFloat3(&maximum), XMLoadFloat3(&range.boundsMax)));
        }
        const auto& stats = *m_Model->Get_StaticProxyStats();
        m_GpuBytes = stats.geometryBytes + CMapStaticProxyBaker::Estimate_AtlasBytes(stats);
        m_VisibleSourceSlots.reserve(ranges.size());
        m_DrawRepresentatives.reserve(m_Members.size()); m_DrawRepresentativeLods.reserve(m_Members.size());
        m_CandidateActiveMembers.resize(m_Members.size()); m_Claim->activeMembers.resize(m_Members.size());
    }
    else
    {
        const auto& stats = *m_Model->Get_StaticClusterStats();
        minimum = stats.boundsMin; maximum = stats.boundsMax; m_GpuBytes = GeometryBytes(stats);
    }
    XMStoreFloat3(&m_Center, (XMLoadFloat3(&minimum) + XMLoadFloat3(&maximum)) * .5f);
    m_Radius = std::nextafter(XMVectorGetX(XMVector3Length(XMLoadFloat3(&maximum) - XMLoadFloat3(&m_Center))), INFINITY);
    return std::isfinite(m_Radius) && m_Radius > 0.f ? S_OK : E_INVALIDARG;
}

bool_t CMapStaticChunkObject::Try_GetFinalCameraSpatialBounds(FINAL_CAMERA_SPATIAL_BOUNDS& bounds) const
{
    if (m_Culling.bypass || m_Culling.diagnostics) return false;
    bounds.Center = m_Center;
    float margin = (std::max)(0.f, m_Culling.baseMargin);
    if (m_Radius >= m_Culling.largeObjectRadiusThreshold)
        margin += (std::max)(m_Culling.largeObjectAbsoluteMargin, m_Radius * m_Culling.largeObjectRelativeMargin);
    bounds.Radius = m_Radius + (std::max)(0.f, margin);
    bounds.RejectGraceFrames = m_Culling.rejectHysteresisFrames;
    bounds.ShadowCaster = false; // Original batches retain all light-volume submissions.
    return true;
}

void CMapStaticChunkObject::Late_Update(float)
{
    m_Far = false;
    m_SelectionResolved = false;
    m_Claim->frameActive = false;
    std::fill(m_Claim->activeMembers.begin(), m_Claim->activeMembers.end(), uint8_t(0));
#ifdef _DEBUG
    m_Submitted = false;
#endif
    // All scene mutations precede final-camera traversal. A diagnostics lease or
    // UI option cannot switch a source and its replacement halfway through draw.
    m_ModeAllowed = m_Claim->valid && m_Claim->policy && m_Claim->policy->enabled &&
        CGameInstance::Get().Get_MaterialRenderSettings().bUseSourceMaterials &&
        !CMapAssetRenderUtils::Is_SurfaceBindingCollectionActive();
    if (m_Proxy)
    {
        const auto& settings = CGameInstance::Get().Get_MaterialRenderSettings();
        m_ModeAllowed = m_ModeAllowed && m_Claim->policy->atlasEnabled && settings.eDebugView == Engine::MATERIAL_DEBUG_VIEW::FINAL &&
            !settings.MapPBR.Is_Active(CGameInstance::Get().Get_CurrentLevelID()) &&
            CGameInstance::Get().Get_RenderQualitySettings().iTextureMinMip == 0u;
    }
    if (auto* p = CGameInstance::Get().Get_Profiler())
    {
        p->Add_Counter(Engine::EProfilerCounter::MapChunkCount);
        p->Add_Counter(Engine::EProfilerCounter::MapChunkGpuBytes, m_GpuBytes);
        if (m_ModeAllowed)
        {
            p->Add_Counter(Engine::EProfilerCounter::MapChunkEnabledCount);
            if (m_Claim->policy->hlodEnabled) p->Add_Counter(Engine::EProfilerCounter::MapChunkHlodEnabledCount);
        }
    }
}

void CMapStaticChunkObject::Resolve_RenderSelection()
{
    if (m_SelectionResolved) return;
    m_SelectionResolved = true;
    m_Claim->frameActive = false;
    if (!m_ModeAllowed || !m_Claim->valid || CGameInstance::Get().Is_SceneEnvironmentReplaced()) return;
    const auto* camera = CMapAssetRenderUtils::Capture_CameraCullSnapshotView();
    if (!camera) return;
    MAP_FRUSTUM_CULL_DECISION decision{};
    if (camera && CMapAssetRenderUtils::Evaluate_FrustumVisibility(m_Culling, *camera, "chunk", "", 0,
        m_Center, m_Radius, m_FrustumState, decision, nullptr, MAP_FRUSTUM_CULL_DETAIL::VISIBILITY_ONLY) &&
        !decision.shouldRender) return;
    if (m_Proxy) { Resolve_ProxySelection(*camera); return; }
    m_Far = false;
    const auto& s = *m_Model->Get_StaticClusterStats();
    if (camera && m_Claim->policy->hlodEnabled && s.hasFarGeometry && camera->lodViewScale > 0.f)
    {
        const auto& p = camera->projection;
        const auto viewport = CGameInstance::Get().Get_ViewportSize();
        if (p._14 == 0.f && p._24 == 0.f && p._34 == 1.f && p._44 == 0.f &&
            p._12 == 0.f && p._21 == 0.f && p._13 == 0.f && p._23 == 0.f &&
            p._41 == 0.f && p._42 == 0.f && p._11 > 0.f && p._22 > 0.f && p._33 > 1.f && p._43 < 0.f)
        {
            float3_t viewCenter{};
            XMStoreFloat3(&viewCenter, XMVector3TransformCoord(XMLoadFloat3(&m_Center), XMLoadFloat4x4(&camera->lodView)));
            const double radius = double(m_Radius) * camera->lodViewScale;
            const double error = double(s.maximumWorldError) * camera->lodViewScale;
            const double depth = double(viewCenter.z) - radius;
            const double lateral = (std::max)(std::abs(double(viewCenter.x)), std::abs(double(viewCenter.y))) + radius;
            const double pixels = (std::max)(double(p._11) * viewport.x, double(p._22) * viewport.y) * .5;
            // Bound both depth and lateral movement, including off-axis perspective.
            if (depth > -double(p._43) / p._33 + error && pixels > 0. && error >= 0.)
            {
                const double projected = pixels * error * (depth + lateral) / (depth * (depth - error));
                m_Far = std::isfinite(projected) && projected <= 1.;
            }
        }
    }
    Engine::CProfilerScope selection(CGameInstance::Get().Get_Profiler(), "Map.Chunk.Select");
    uint64_t originalIndices = 0u;
    uint32_t originalDraws = 0u;
    std::array<const Engine::CModel*, 8> bankRepresentatives{};
    std::array<uint32_t, 8> bankLods{}, bankMembers{}, bankMeshes{};
    size_t bankCount = 0u;
    for (const auto& member : m_Members)
    {
        const auto batch = member.batch.lock();
        if (!batch || FAILED(batch->Prepare_FinalCameraVisibility(camera))) return;
        const auto visible = batch->Get_VisibleInstanceCount();
        if (visible == 0u) continue;
        Engine::MESH_SCREEN_LOD_DESC lod{};
        const bool hasLod = batch->m_RenderProfile.opacity >= 1.f && batch->Build_ScreenLodView(*camera, lod);
        const auto* view = batch->Uses_MeshScreenLod(member.mesh, hasLod) ? &lod : nullptr;
        const auto& model = *batch->m_pModelCom;
        originalIndices += uint64_t(model.Get_StaticMeshSelectedIndexCount(member.mesh, view)) * visible;
        const auto level = model.Get_StaticMeshLodLevel(member.mesh, view);
        size_t bank = 0u;
        for (; bank < bankCount; ++bank)
            if (bankMembers[bank] < 8u && bankMeshes[bank] == member.mesh && bankLods[bank] == level &&
                (bankRepresentatives[bank]->Can_BatchStaticLightingWith(model) ||
                 bankRepresentatives[bank]->Can_ShareStaticInstanceStateWith(model))) break;
        // Count compatible instancing as one draw even if intervening original
        // objects would prevent adjacency. This deliberately underestimates the
        // baseline so a replacement cannot claim those draws as its savings.
        if (bank == bankCount)
        {
            ++originalDraws;
            if (bankCount < bankRepresentatives.size())
            {
                bankRepresentatives[bankCount] = &model; bankLods[bankCount] = level;
                bankMeshes[bankCount] = member.mesh; bankMembers[bankCount++] = 1u;
            }
        }
        else ++bankMembers[bank];
    }
    const uint64_t chosenIndices = m_Far ? s.farIndices : s.nearIndices;
    // Captured near merges cost more than the small draw saving. Admit only
    // replacements that remove at least two conservative baseline draws and
    // do not increase geometry over the camera-visible original mesh LODs.
    m_Claim->frameActive = originalDraws >= MINIMUM_CHUNK_SOURCE_DRAWS && chosenIndices <= originalIndices;
}

void CMapStaticChunkObject::Resolve_ProxySelection(const MAP_CAMERA_CULL_SNAPSHOT& camera)
{
    Engine::CProfilerScope selection(CGameInstance::Get().Get_Profiler(), "Map.Proxy.Select");
    m_VisibleSourceSlots.clear(); m_DrawRepresentatives.clear(); m_DrawRepresentativeLods.clear();
    std::fill(m_CandidateActiveMembers.begin(), m_CandidateActiveMembers.end(), uint8_t(0));
    m_SelectedIndices = 0u; m_OriginalVisibleIndices = 0u; m_SelectedSourceDraws = 0u;
    const auto& stats = *m_Model->Get_StaticProxyStats();
    const auto ranges = m_Model->Get_StaticProxySourceRanges();
    const auto viewport = CGameInstance::Get().Get_ViewportSize();
    const bool coarseDensityFits = ProxyDensityFits(camera, viewport, m_Center, m_Radius, stats.texelsPerUnit);
    for (uint32_t memberIndex = 0u; memberIndex < m_Members.size(); ++memberIndex)
    {
        const auto& member = m_Members[memberIndex];
        const auto batch = member.batch.lock();
        if (!batch || FAILED(batch->Prepare_FinalCameraVisibility(&camera))) return;
        const auto visible = batch->Get_VisibleInstanceIndices();
        if (visible.empty()) continue;
        if (visible.size() != batch->Get_VisibleInstanceCount()) return;
        const auto& model = *batch->m_pModelCom;
        if (!model.Can_BakeStaticProxy(member.mesh)) continue;
        Engine::MESH_SCREEN_LOD_DESC lod{};
        const bool hasLod = batch->Build_ScreenLodView(camera, lod);
        const auto* view = batch->Uses_MeshScreenLod(member.mesh, hasLod) ? &lod : nullptr;
        const auto originalCount = model.Get_StaticMeshSelectedIndexCount(member.mesh, view);
        const auto level = model.Get_StaticMeshLodLevel(member.mesh, view);
        const size_t start = m_VisibleSourceSlots.size();
        uint64_t memberIndices = 0u;
        bool accepted = originalCount != 0u;
        for (const uint32_t instanceIndex : visible)
        {
            if (!accepted || instanceIndex >= member.sourceSlots.size() || instanceIndex >= batch->m_Instances.size())
            { accepted = false; break; }
            const uint32_t slot = member.sourceSlots[instanceIndex];
            if (slot >= ranges.size() || ranges[slot].indexCount > originalCount)
            { accepted = false; break; }
            const auto& instance = batch->m_Instances[instanceIndex];
            if (!coarseDensityFits && !ProxyDensityFits(camera, viewport, instance.WorldBoundsCenter,
                instance.WorldBoundsRadius, stats.texelsPerUnit)) { accepted = false; break; }
            m_VisibleSourceSlots.push_back(slot); memberIndices += ranges[slot].indexCount;
        }
        if (!accepted) { m_VisibleSourceSlots.resize(start); continue; }
        m_CandidateActiveMembers[memberIndex] = 1u;
        m_SelectedIndices += memberIndices; m_OriginalVisibleIndices += uint64_t(originalCount) * visible.size();
        // Ignore adjacency and even the eight-variant bank limit. The resulting
        // count is a lower bound, so repeated models cannot inflate the saving.
        bool combined = false;
        for (size_t i = 0u; i < m_DrawRepresentatives.size(); ++i)
        {
            const auto& representative = m_Members[m_DrawRepresentatives[i]];
            const auto other = representative.batch.lock();
            if (other && representative.mesh == member.mesh && m_DrawRepresentativeLods[i] == level &&
                (other->m_pModelCom->Can_ShareStaticInstanceStateWith(model) ||
                 other->m_pModelCom->Can_BatchStaticLightingWith(model))) { combined = true; break; }
        }
        if (!combined) { m_DrawRepresentatives.push_back(memberIndex); m_DrawRepresentativeLods.push_back(level); }
    }
    m_SelectedSourceDraws = static_cast<uint32_t>(m_DrawRepresentatives.size());
    if (m_SelectedSourceDraws < MINIMUM_PROXY_VISIBLE_DRAWS || m_SelectedIndices > m_OriginalVisibleIndices ||
        m_VisibleSourceSlots.empty()) return;
    // Stage assigns consecutive ranges in member/local-instance order. Original
    // visibility traverses local indices ascending, retaining that same order.
    if (m_Model->Prepare_StaticProxyVisibleSources(m_VisibleSourceSlots) != S_OK) return;
    m_Claim->activeMembers.swap(m_CandidateActiveMembers);
    m_Claim->frameActive = true;
}

void CMapStaticChunkObject::Submit_FinalCamera()
{
    if (!m_Claim->Is_Active()) return;
    CGameInstance::Get().Add_RenderObject(RENDERGROUP::NONBLEND, std::static_pointer_cast<CGameObject>(shared_from_this()));
}

HRESULT CMapStaticChunkObject::Render()
{
    if (!m_Claim->Is_Active() || CGameInstance::Get().Is_SceneEnvironmentReplaced()) return S_OK;
    auto& game = CGameInstance::Get();
    auto* profiler = game.Get_Profiler();
    Engine::CProfilerScope scope(profiler, "Map.Chunk.Render");
    const auto* camera = CMapAssetRenderUtils::Capture_CameraCullSnapshotView();
    if (camera ? FAILED(CMapAssetRenderUtils::Bind_CameraCullSnapshot(m_Shader, *camera)) :
        (FAILED(game.Bind_Transform(m_Shader, "g_ViewMatrix", D3DTS::VIEW)) ||
         FAILED(game.Bind_Transform(m_Shader, "g_ProjMatrix", D3DTS::PROJ)))) return E_FAIL;
    if (m_Proxy)
    {
        if (FAILED(game.Bind_CamPosition(m_Shader, "g_vCamPosition")) ||
            FAILED(CMapStaticProxyBaker::Bind(m_Shader, m_Model)) ||
            FAILED(m_Shader->Begin(1u + CMapAssetRenderUtils::Select_Pass(m_Profile, m_Mirrored)))) return E_FAIL;
        const HRESULT result = m_Model->Render_StaticProxy();
#ifdef _DEBUG
        m_Submitted = SUCCEEDED(result);
#endif
        if (SUCCEEDED(result) && profiler)
        {
            profiler->Add_Counter(Engine::EProfilerCounter::MapChunkNearDraws);
            profiler->Add_Counter(Engine::EProfilerCounter::MapChunkSourceDraws, m_SelectedSourceDraws);
            profiler->Add_Counter(Engine::EProfilerCounter::MapChunkSubmittedIndices, m_SelectedIndices);
            profiler->Add_Counter(Engine::EProfilerCounter::MapChunkOriginalIndices, m_OriginalVisibleIndices);
        }
        return result;
    }
    const auto timeSource = m_TimeSource.lock();
    if (!timeSource) return E_FAIL;
    if (FAILED(CMapAssetRenderUtils::Bind_Material(m_Model, m_Shader, 0, m_Profile, timeSource->Get_RenderElapsedTime(), nullptr, "chunk",
        nullptr, nullptr, MAP_MATERIAL_BINDING_MODE::INSTANCED)) ||
        FAILED(m_Model->Bind_StaticClusterSources(m_Shader)) ||
        FAILED(m_Shader->Begin(CMapAssetRenderUtils::Select_Pass(m_Profile, m_Mirrored)))) return E_FAIL;
    const HRESULT result = m_Model->Render_StaticCluster(m_Far);
#ifdef _DEBUG
    m_Submitted = SUCCEEDED(result);
#endif
    if (SUCCEEDED(result) && profiler)
    {
        const auto& s = *m_Model->Get_StaticClusterStats();
        profiler->Add_Counter(m_Far ? Engine::EProfilerCounter::MapChunkFarDraws : Engine::EProfilerCounter::MapChunkNearDraws);
        profiler->Add_Counter(Engine::EProfilerCounter::MapChunkSourceDraws, m_SourceDraws);
        profiler->Add_Counter(Engine::EProfilerCounter::MapChunkSubmittedIndices, m_Far ? s.farIndices : s.nearIndices);
        profiler->Add_Counter(Engine::EProfilerCounter::MapChunkOriginalIndices, s.nearIndices);
    }
    return result;
}

void CMapStaticChunkObject::Stage(uint32_t levelIndex,
    std::span<const std::shared_ptr<CMapStaticBatchObject>> batches, const MAP_FRUSTUM_CULLING_POLICY& culling,
    const std::shared_ptr<MAP_CHUNK_POLICY>& policy,
    std::vector<std::shared_ptr<CMapStaticChunkObject>>& output, MAP_CHUNK_BUILD_STATS& stats)
{
    const auto start = std::chrono::steady_clock::now();
    Engine::CProfilerScope preparation(CGameInstance::Get().Get_Profiler(), "Map.Chunk.Prepare");
    struct MEMBER { std::shared_ptr<CMapStaticBatchObject> batch; uint32_t mesh; };
    struct GROUP { std::vector<MEMBER> members; uint64_t estimate = 0; };
    std::map<std::tuple<int64_t, int64_t, bool>, std::vector<GROUP>> cells;
    for (const auto& b : batches)
    {
        if (!b || !b->m_pModelCom || b->m_Instances.empty() || b->m_RenderProfile.opacity < 1.f ||
            b->m_RenderProfile.uvSpeed.x != 0.f || b->m_RenderProfile.uvSpeed.y != 0.f) continue;
        const auto& center = b->m_Instances.front().WorldBoundsCenter;
        if (!std::isfinite(center.x) || !std::isfinite(center.z) || std::abs(center.x) > 1.e8f || std::abs(center.z) > 1.e8f) continue;
        auto& groups = cells[{static_cast<int64_t>(std::floor(center.x / CellMetres)),
            static_cast<int64_t>(std::floor(center.z / CellMetres)), b->m_bMirrored}];
        b->m_ChunkClaims.resize(b->m_pModelCom->Get_NumMeshes());
        for (uint32_t mesh = 0; mesh < b->m_pModelCom->Get_NumMeshes(); ++mesh)
        {
            if (!b->m_pModelCom->Can_ShareStaticClusterMaterialWith(mesh, *b->m_pModelCom, mesh)) continue;
            const uint64_t estimate = uint64_t(b->m_pModelCom->Get_MeshIndexCount(mesh)) * b->m_Instances.size() * 88u;
            if (!estimate || estimate > 24u * 1024u * 1024u) continue;
            auto found = std::find_if(groups.begin(), groups.end(), [&](const GROUP& g) {
                const auto& first = g.members.front();
                return g.members.size() < 8u && g.estimate + estimate <= 24u * 1024u * 1024u &&
                    SameProfile(first.batch->m_RenderProfile, b->m_RenderProfile) &&
                    first.batch->m_pModelCom->Can_ShareStaticClusterMaterialWith(first.mesh, *b->m_pModelCom, mesh);
            });
            if (found == groups.end()) { groups.push_back({}); found = std::prev(groups.end()); }
            found->members.push_back({b, mesh}); found->estimate += estimate;
        }
    }
    std::vector<GROUP*> ranked;
    for (auto& [cell, groups] : cells)
        for (auto& g : groups)
            if (g.members.size() >= MINIMUM_CHUNK_SOURCE_DRAWS) ranked.push_back(&g);
    std::stable_sort(ranked.begin(), ranked.end(), [](const GROUP* a, const GROUP* b) {
        return double(a->members.size() - 1) / double(a->estimate) > double(b->members.size() - 1) / double(b->estimate);
    });
    output.reserve(output.size() + ranked.size());
    auto cache = Engine::CModel::Create_StaticClusterBuildCache();
    constexpr uint64_t budget = 384ull * 1024ull * 1024ull;
    for (const auto* group : ranked)
    {
        if (!cache || group->estimate > budget - stats.gpuBytes) { ++stats.rejectedGroups; continue; }
        std::vector<Engine::CModel::STATIC_CLUSTER_SOURCE> sources;
#ifdef _DEBUG
        std::vector<std::string> sourceAssetIds;
#endif
        for (const auto& member : group->members)
            for (const auto& instance : member.batch->m_Instances)
            {
                if (!instance.Visible || instance.Suppressed || instance.CameraPreviewSuppressed) continue;
                VTXMESHINSTANCE gpu{};
                gpu.World = instance.World; gpu.WorldInvTranspose = instance.WorldInvTranspose;
                gpu.vLightmapScaleBias = instance.BakedLighting.scaleBias;
                gpu.vLightmapAverageScale = instance.BakedLighting.averageScale;
                gpu.vLightmapDirectionalScale = instance.BakedLighting.directionalScale;
                gpu.vStaticShadowScaleBias = instance.BakedLighting.shadowScaleBias;
                gpu.vSourceWindOwnerPosition = instance.SourceWind.actorPositionSourceCm;
                gpu.vSourceWindDimensionsAndRadius = instance.SourceWind.objectDimensionsAndRadiusSourceCm;
                sources.push_back({member.batch->m_pModelCom.get(), member.mesh, instance.PlacementId, gpu});
#ifdef _DEBUG
                sourceAssetIds.push_back(member.batch->m_AssetId);
#endif
            }
        std::shared_ptr<Engine::CModel> model;
        Engine::CModel::STATIC_CLUSTER_STATS built{};
        if (sources.size() < 2u || Engine::CModel::Create_StaticCluster(sources, {}, *cache, model, built) != S_OK ||
            !model || GeometryBytes(built) > budget - stats.gpuBytes) { ++stats.rejectedGroups; continue; }
        const auto& first = group->members.front().batch;
        DESC d;
        d.prototypeLevelIndex = levelIndex; d.model = model; d.timeSource = first; d.profile = first->m_RenderProfile;
        d.culling = culling; d.mirrored = first->m_bMirrored;
        d.claim = std::make_shared<MAP_CHUNK_CLAIM>(); d.claim->policy = policy;
        d.sourceDraws = static_cast<uint32_t>(group->members.size());
        for (const auto& member : group->members) d.members.push_back({member.batch, member.mesh});
#ifdef _DEBUG
        d.sourceAssetIds = std::move(sourceAssetIds);
#endif
        std::shared_ptr<CGameObject> object;
        if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(levelIndex, PrototypeTag, levelIndex, LayerTag, &d, &object)))
        { ++stats.rejectedGroups; continue; }
        auto chunk = std::dynamic_pointer_cast<CMapStaticChunkObject>(object);
        if (!chunk)
        { CGameInstance::Get().Remove_GameObject_from_Layer(levelIndex, LayerTag, object); ++stats.rejectedGroups; continue; }
        output.push_back(chunk);
        // Claims publish only after both model buffers and the live object exist.
        for (const auto& member : group->members) member.batch->m_ChunkClaims[member.mesh] = d.claim;
        ++stats.chunks; stats.sourceDraws += d.sourceDraws; stats.sources += built.sourceCount;
        stats.farChunks += built.hasFarGeometry ? 1u : 0u; stats.gpuBytes += GeometryBytes(built);
        stats.nearIndices += built.nearIndices; stats.farIndices += built.hasFarGeometry ? built.farIndices : built.nearIndices;
    }
    stats.buildMilliseconds = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
    const std::string report = "[MapChunk] Prepared " + std::to_string(stats.chunks) +
        " chunks / " + std::to_string(stats.sourceDraws) + " source draws / " +
        std::to_string(stats.farChunks) + " HLOD / " + std::to_string(stats.gpuBytes) +
        " GPU bytes / " + std::to_string(stats.rejectedGroups) + " rejected / " +
        std::to_string(stats.buildMilliseconds) + " ms\n";
    OutputDebugStringA(report.c_str());
}

void CMapStaticChunkObject::Stage_Proxy(uint32_t levelIndex,
    std::span<const std::shared_ptr<CMapStaticBatchObject>> batches, const MAP_FRUSTUM_CULLING_POLICY& culling,
    const std::shared_ptr<MAP_CHUNK_POLICY>& policy,
    std::vector<std::shared_ptr<CMapStaticChunkObject>>& output, MAP_CHUNK_BUILD_STATS& stats)
{
    if (!policy || !policy->atlasEnabled) return;
    const auto start = std::chrono::steady_clock::now();
    Engine::CProfilerScope preparation(CGameInstance::Get().Get_Profiler(), "Map.Proxy.Prepare");
    constexpr uint64_t geometryLimit = 64ull * 1024ull * 1024ull;
    constexpr uint64_t budget = 512ull * 1024ull * 1024ull;
    constexpr uint32_t sourceLimit = 256u;
    struct MEMBER { std::shared_ptr<CMapStaticBatchObject> batch; uint32_t mesh = 0u; };
    struct GROUP
    {
        std::vector<MEMBER> members;
        uint64_t estimate = 0u;
        uint32_t sourceCount = 0u, drawCount = 0u;
    };
    std::map<std::tuple<int64_t, int64_t, bool>, std::vector<GROUP>> cells;
    for (const auto& batch : batches)
    {
        if (!batch || !batch->m_pModelCom || batch->m_Instances.empty() ||
            batch->m_RenderProfile.renderMode != MAP_ASSET_RENDER_MODE::DEFERRED ||
            batch->m_RenderProfile.opacity != 1.f || batch->m_RenderProfile.uvSpeed.x != 0.f ||
            batch->m_RenderProfile.uvSpeed.y != 0.f) continue;
        const auto& center = batch->m_Instances.front().WorldBoundsCenter;
        if (!std::isfinite(center.x) || !std::isfinite(center.z) ||
            std::abs(center.x) > 1.e8f || std::abs(center.z) > 1.e8f) continue;
        const auto sourceCount = static_cast<uint32_t>(std::count_if(batch->m_Instances.begin(), batch->m_Instances.end(),
            [](const auto& instance) { return instance.Visible && !instance.Suppressed && !instance.CameraPreviewSuppressed; }));
        if (sourceCount == 0u || sourceCount > sourceLimit) continue;
        auto& groups = cells[{static_cast<int64_t>(std::floor(center.x / PROXY_CELL_METRES)),
            static_cast<int64_t>(std::floor(center.z / PROXY_CELL_METRES)), batch->m_bMirrored}];
        batch->m_ChunkClaims.resize(batch->m_pModelCom->Get_NumMeshes());
        batch->m_ChunkClaimMemberIndices.resize(batch->m_pModelCom->Get_NumMeshes(), UINT32_MAX);
        for (uint32_t mesh = 0u; mesh < batch->m_pModelCom->Get_NumMeshes(); ++mesh)
        {
            if (!batch->m_pModelCom->Can_BakeStaticProxy(mesh)) continue;
            // A conservative preflight; xatlas and CModel enforce actual bytes.
            const uint64_t estimate = uint64_t(batch->m_pModelCom->Get_MeshIndexCount(mesh)) * sourceCount * 88u;
            if (estimate == 0u || estimate > geometryLimit) continue;
            auto found = std::find_if(groups.begin(), groups.end(), [&](const GROUP& group) {
                return group.sourceCount + sourceCount <= sourceLimit && group.estimate + estimate <= geometryLimit &&
                    SameProfile(group.members.front().batch->m_RenderProfile, batch->m_RenderProfile);
            });
            if (found == groups.end()) { groups.push_back({}); found = std::prev(groups.end()); }
            found->members.push_back({batch, mesh}); found->sourceCount += sourceCount; found->estimate += estimate;
        }
    }
    std::vector<GROUP*> ranked;
    for (auto& [cell, groups] : cells)
        for (auto& group : groups)
        {
            std::vector<size_t> representatives;
            for (size_t i = 0u; i < group.members.size(); ++i)
            {
                const auto& member = group.members[i];
                const bool combined = std::any_of(representatives.begin(), representatives.end(), [&](size_t index) {
                    const auto& first = group.members[index];
                    return first.mesh == member.mesh &&
                        (first.batch->m_pModelCom->Can_ShareStaticInstanceStateWith(*member.batch->m_pModelCom) ||
                         first.batch->m_pModelCom->Can_BatchStaticLightingWith(*member.batch->m_pModelCom));
                });
                if (!combined) representatives.push_back(i);
            }
            group.drawCount = static_cast<uint32_t>(representatives.size());
            if (group.drawCount >= MINIMUM_PROXY_BUILD_DRAWS) ranked.push_back(&group);
        }
    std::stable_sort(ranked.begin(), ranked.end(), [](const GROUP* a, const GROUP* b) {
        return double(a->drawCount - 1u) / double(a->estimate) > double(b->drawCount - 1u) / double(b->estimate);
    });
    output.reserve(output.size() + ranked.size());
    auto cache = Engine::CModel::Create_StaticClusterBuildCache();
    std::unique_ptr<CMapStaticProxyBaker> baker;
    for (const auto* group : ranked)
    {
        if (!cache || stats.gpuBytes >= budget) { ++stats.rejectedGroups; continue; }
        const auto& first = group->members.front().batch;
        if (!baker) baker = std::make_unique<CMapStaticProxyBaker>(first->m_pDevice, first->m_pContext);
        std::vector<Engine::CModel::STATIC_CLUSTER_SOURCE> sources;
        sources.reserve(group->sourceCount);
        DESC d;
        d.prototypeLevelIndex = levelIndex; d.timeSource = first; d.profile = first->m_RenderProfile;
        d.culling = culling; d.mirrored = first->m_bMirrored; d.proxy = true;
        d.sourceDraws = group->drawCount; d.members.reserve(group->members.size());
        for (const auto& member : group->members)
        {
            SOURCE_MEMBER mapped;
            mapped.batch = member.batch; mapped.mesh = member.mesh;
            mapped.sourceSlots.resize(member.batch->m_Instances.size(), UINT32_MAX);
            for (uint32_t i = 0u; i < member.batch->m_Instances.size(); ++i)
            {
                const auto& instance = member.batch->m_Instances[i];
                if (!instance.Visible || instance.Suppressed || instance.CameraPreviewSuppressed) continue;
                VTXMESHINSTANCE gpu{};
                gpu.World = instance.World; gpu.WorldInvTranspose = instance.WorldInvTranspose;
                gpu.vLightmapScaleBias = instance.BakedLighting.scaleBias;
                gpu.vLightmapAverageScale = instance.BakedLighting.averageScale;
                gpu.vLightmapDirectionalScale = instance.BakedLighting.directionalScale;
                gpu.vStaticShadowScaleBias = instance.BakedLighting.shadowScaleBias;
                gpu.vSourceWindOwnerPosition = instance.SourceWind.actorPositionSourceCm;
                gpu.vSourceWindDimensionsAndRadius = instance.SourceWind.objectDimensionsAndRadiusSourceCm;
                mapped.sourceSlots[i] = static_cast<uint32_t>(sources.size());
                sources.push_back({member.batch->m_pModelCom.get(), member.mesh, instance.PlacementId, gpu});
#ifdef _DEBUG
                d.sourceAssetIds.push_back(member.batch->m_AssetId);
#endif
            }
            d.members.push_back(std::move(mapped));
        }
        Engine::CModel::STATIC_PROXY_OPTIONS options;
        options.atlasResolution = 1024u; options.texelsPerUnit = 16.f; options.padding = 8u;
        options.maximumSources = sourceLimit; options.maximumGeometryBytes = geometryLimit;
        Engine::CModel::STATIC_PROXY_STATS built{};
        if (Engine::CModel::Create_StaticProxy(sources, options, *cache, d.model, built) != S_OK || !d.model)
        { ++stats.rejectedGroups; continue; }
        const uint64_t bytes = built.geometryBytes + CMapStaticProxyBaker::Estimate_AtlasBytes(built);
        std::string error;
        if (bytes > budget - stats.gpuBytes || baker->Prepare(sources,
            std::span<const MAP_ASSET_RENDER_PROFILE>(&d.profile, 1u), d.model, error) != S_OK)
        { ++stats.rejectedGroups; continue; }
        d.claim = std::make_shared<MAP_CHUNK_CLAIM>(); d.claim->policy = policy;
        std::shared_ptr<CGameObject> object;
        if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(levelIndex, PrototypeTag, levelIndex, LayerTag, &d, &object)))
        { ++stats.rejectedGroups; continue; }
        auto chunk = std::dynamic_pointer_cast<CMapStaticChunkObject>(object);
        if (!chunk)
        { CGameInstance::Get().Remove_GameObject_from_Layer(levelIndex, LayerTag, object); ++stats.rejectedGroups; continue; }
        output.push_back(chunk);
        // No source suppression is published until geometry, atlases and the
        // owner are ready. A failed candidate leaves original instancing intact.
        for (uint32_t i = 0u; i < group->members.size(); ++i)
        {
            const auto& member = group->members[i];
            member.batch->m_bTrackProxySources = true;
            member.batch->m_ChunkClaims[member.mesh] = d.claim;
            member.batch->m_ChunkClaimMemberIndices[member.mesh] = i;
        }
        ++stats.chunks; stats.sourceDraws += group->drawCount; stats.sources += built.sourceCount;
        stats.gpuBytes += bytes; stats.nearIndices += built.indexCount; stats.farIndices += built.indexCount;
    }
    stats.buildMilliseconds = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
    const std::string report = "[MapProxy] Prepared " + std::to_string(stats.chunks) +
        " coarse owners / " + std::to_string(stats.sourceDraws) + " conservative source draws / " +
        std::to_string(stats.gpuBytes) + " GPU bytes / " + std::to_string(stats.rejectedGroups) + " rejected / " +
        std::to_string(baker ? baker->Get_CacheHits() : 0u) + " atlas cache hits / " +
        std::to_string(stats.buildMilliseconds) + " ms\n";
    OutputDebugStringA(report.c_str());
}

void CMapStaticChunkObject::Remove(uint32_t levelIndex, std::vector<std::shared_ptr<CMapStaticChunkObject>>& objects)
{
    for (const auto& object : objects)
    {
        object->m_Claim->valid = false;
        object->m_Claim->frameActive = false;
        object->m_Claim->owner = nullptr;
        CGameInstance::Get().Remove_GameObject_from_Layer(levelIndex, LayerTag, object);
    }
    objects.clear();
}

std::unique_ptr<CMapStaticChunkObject> CMapStaticChunkObject::Create(ComPtr<ID3D11Device> device, ComPtr<ID3D11DeviceContext> context)
{
    return std::unique_ptr<CMapStaticChunkObject>(new CMapStaticChunkObject(device, context));
}
#ifdef _DEBUG
MAP_CHUNK_DEBUG_ROW CMapStaticChunkObject::Get_DebugRow(uint32_t chunkId) const
{
    MAP_CHUNK_DEBUG_ROW row;
    row.chunkId = chunkId; row.assetIds = m_SourceAssetIds; row.sourceDraws = m_SourceDraws;
    if (m_Proxy)
    {
        const auto ranges = m_Model->Get_StaticProxySourceRanges();
        row.minimum = ranges.front().boundsMin; row.maximum = ranges.front().boundsMax;
        for (const auto& range : ranges)
        {
            row.placementIds.push_back(range.sourceId);
            XMStoreFloat3(&row.minimum, XMVectorMin(XMLoadFloat3(&row.minimum), XMLoadFloat3(&range.boundsMin)));
            XMStoreFloat3(&row.maximum, XMVectorMax(XMLoadFloat3(&row.maximum), XMLoadFloat3(&range.boundsMax)));
        }
        row.materialName = "Baked static proxy";
        row.nearIndices = row.farIndices = m_Model->Get_StaticProxyStats()->indexCount;
    }
    else
    {
        const auto& s = *m_Model->Get_StaticClusterStats();
        row.minimum = s.boundsMin; row.maximum = s.boundsMax; row.materialName = m_Model->Get_MaterialName(0u);
        const auto ids = m_Model->Get_StaticClusterSourceIds(); row.placementIds.assign(ids.begin(), ids.end());
        row.nearIndices = s.nearIndices; row.farIndices = s.hasFarGeometry ? s.farIndices : s.nearIndices;
    }
    row.active = m_ModeAllowed; row.valid = m_Claim->valid; row.farSelected = m_Far;
    row.submitted = m_Submitted;
    return row;
}
#endif
std::shared_ptr<CPrototype> CMapStaticChunkObject::Clone(void* arg)
{
    auto result = std::shared_ptr<CMapStaticChunkObject>(new CMapStaticChunkObject(*this));
    return SUCCEEDED(result->Initialize(arg)) ? result : nullptr;
}
