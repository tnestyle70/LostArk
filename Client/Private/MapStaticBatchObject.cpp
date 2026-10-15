#include "MapStaticBatchObject.h"
#include "MapGpuScene.h"
#include "MapStaticChunkObject.h"
#include "MapStaticProxyBaker.h"
#include "Engine_RenderTypes.h"
#include "Engine_VertexTypes.h"

#include "GameInstance.h"
#include "MapAssetRenderUtils.h"
#include "Model.h"
#include "MeshLod.h"
#include "OcclusionCuller.h"
#include "Profiler.h"
#include "EffectFailureDiagnostic.h"
#include "Shader.h"

#include <algorithm>
#include <array>
#include <stdexcept>
#include <cstring>
#include <limits>
#include <cmath>
#include <cfloat>
#include <sstream>

namespace
{
    bool SameInstanceProfile(const MAP_ASSET_RENDER_PROFILE& a,
        const MAP_ASSET_RENDER_PROFILE& b)
    {
        return a.renderMode == b.renderMode && a.cullMode == b.cullMode &&
            a.uvScale.x == b.uvScale.x && a.uvScale.y == b.uvScale.y &&
            a.uvSpeed.x == b.uvSpeed.x && a.uvSpeed.y == b.uvSpeed.y &&
            a.opacity == b.opacity && a.opacityPower == b.opacityPower &&
            a.emissiveIntensity == b.emissiveIntensity &&
            a.specularIntensity == b.specularIntensity && a.specularPower == b.specularPower &&
            a.colorTint.x == b.colorTint.x && a.colorTint.y == b.colorTint.y &&
            a.colorTint.z == b.colorTint.z && a.colorTint.w == b.colorTint.w &&
            a.triplanarHeightScale == b.triplanarHeightScale && a.castsShadow == b.castsShadow;
    }
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
    float StaticPropDistanceLimit(float radius, float scale)
    {
        if (!std::isfinite(radius) || !(radius > 0.f) ||
            !std::isfinite(scale) || scale < .25f || scale > 4.f) return 0.f;
        return (radius <= .5f ? 35.f : radius <= 1.5f ? 45.f : radius <= 3.f ? 60.f : 120.f) * scale;
    }

    bool StaticPropDistanceHidden(const FMapStaticInstance& instance, const float4_t& position,
        const MAP_CAMERA_CULL_SNAPSHOT& camera, const float2_t& viewport, float limit,
        float maximumPixels, uint64_t settingsRevision, float margin)
    {
        const bool wasHidden = instance.DistanceHidden && instance.DistanceSettingsRevision == settingsRevision;
        return CMapAssetRenderUtils::Is_SceneDistanceHidden(instance.WorldBoundsCenter,
            instance.WorldBoundsRadius, position, camera, viewport, limit, maximumPixels, margin, wasHidden);
    }

    bool StaticOcclusionBoundsMesh(const Engine::CModel& model, uint32_t mesh)
    {
        const auto* surface = model.Get_MaterialSurface(mesh);
        // Alpha-tested BG still has immutable geometry. It can be hidden by
        // another opaque object, but cannot itself fill the occlusion buffer.
        return surface && surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED &&
            !surface->sourceFoliageWind && !model.Has_MorphBaseVertices(mesh) &&
            (surface->renderMode == Engine::MODEL_SURFACE_RENDER_MODE::INHERIT ||
             surface->renderMode == Engine::MODEL_SURFACE_RENDER_MODE::DEFERRED);
    }

    bool StaticOpaqueOcclusionMesh(const Engine::CModel& model, uint32_t mesh)
    {
        const auto* surface = model.Get_MaterialSurface(mesh);
        return surface && surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED &&
            !surface->sourceFoliageWind && (surface->sourceBgFlags & 64u) == 0u &&
            (surface->renderMode == Engine::MODEL_SURFACE_RENDER_MODE::INHERIT ||
             surface->renderMode == Engine::MODEL_SURFACE_RENDER_MODE::DEFERRED) &&
            !model.Has_MaterialTexture(mesh, aiTextureType_OPACITY) && !model.Has_MorphBaseVertices(mesh);
    }

    bool TransformStaticBounds(const float3_t& localMin, const float3_t& localMax, const float4x4_t& world,
        float3_t& minimum, float3_t& maximum)
    {
        if (world._14 != 0.f || world._24 != 0.f || world._34 != 0.f || world._44 != 1.f) return false;
        float3_t stagedMin{}, stagedMax{};
        for (size_t axis = 0u; axis < 3u; ++axis)
        {
            double lo = world.m[3][axis], hi = lo, magnitude = std::abs(lo);
            for (size_t source = 0u; source < 3u; ++source)
            {
                if (!std::isfinite((&localMin.x)[source]) || !std::isfinite((&localMax.x)[source]) ||
                    (&localMin.x)[source] > (&localMax.x)[source]) return false;
                const double a = double((&localMin.x)[source]) * world.m[source][axis];
                const double b = double((&localMax.x)[source]) * world.m[source][axis];
                lo += (std::min)(a, b); hi += (std::max)(a, b); magnitude += (std::max)(std::abs(a), std::abs(b));
            }
            const double margin = 16. * FLT_EPSILON * (magnitude + 1.);
            lo -= margin; hi += margin;
            if (!std::isfinite(lo) || !std::isfinite(hi) || lo <= -FLT_MAX || hi >= FLT_MAX) return false;
            (&stagedMin.x)[axis] = std::nextafter(static_cast<float>(lo), -INFINITY);
            (&stagedMax.x)[axis] = std::nextafter(static_cast<float>(hi), INFINITY);
        }
        minimum = stagedMin; maximum = stagedMax;
        return true;
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
            view = &camera->lodView;
            viewScale = camera->lodViewScale;
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

struct CMapStaticBatchObject::CPU_VISIBILITY_PREPARATION final
{
    enum class STATE { EMPTY, STAGED, READY, FAILED };
    STATE State = STATE::EMPTY;
    HRESULT Result = S_OK;
    uint64_t Frame = 0u;
    bool HasCamera = false, DistanceEnabled = false, CountersCommitted = false;
    // Borrowed only until synchronous CPU execution ends. READY/FAILED states
    // retain the revision separately and never dereference this pointer again.
    const MAP_CAMERA_CULL_SNAPSHOT* Camera = nullptr;
    uint64_t CameraRevision = 0u;
    Engine::MAP_VISIBILITY_SETTINGS Settings{};
    float2_t Viewport{};
    float4_t CameraPosition{};
    MAP_FRUSTUM_RUNTIME_STATE BatchFrustum{};
    std::vector<MAP_FRUSTUM_RUNTIME_STATE> FrustumStates;
    bool RequiresNextTick = false, RejectedBatch = false, HasOcclusionBounds = false;
    float3_t OcclusionMin{}, OcclusionMax{};
    float4_t LodBounds{}, TightLodBounds{};
    float LodScale = 0.f;
    uint64_t CullingCandidates = 0u, DistanceTested = 0u, DistanceRejected = 0u;
};

// 같은 프레임의 GPU/HLOD/CPU 재조회가 카운터를 중복 합산하지 않도록 batch의
// 이전 기여만 Level 합계에서 빼고 마지막 판정을 넣는다. 배열은 고정 크기이며 main thread 전용이다.
struct ColorProxyDiagnosticScope final
{
    using COUNTERS = MAP_STATIC_BATCH_FRAME_STATE::COLOR_PROXY_SELECTION_COUNTERS;
    COUNTERS current{};
    COUNTERS& previous;
    COUNTERS& total;
    uint64_t& previousEpoch;
    const uint64_t epoch;
    Engine::CProfiler* const profiler;
    bool keepPrevious = false;

    void Add(Engine::EProfilerCounter counter, uint64_t value = 1u) noexcept
    {
        current[static_cast<size_t>(counter) - MAP_STATIC_BATCH_FRAME_STATE::ColorProxySelectionFirst] += value;
    }
    ~ColorProxyDiagnosticScope() noexcept
    {
        if (keepPrevious) return;
        if (previousEpoch != epoch) previous.fill(0u);
        for (size_t i = 0u; i < current.size(); ++i)
        {
            total[i] = total[i] - previous[i] + current[i];
            if (profiler)
                profiler->Set_Counter(static_cast<Engine::EProfilerCounter>(MAP_STATIC_BATCH_FRAME_STATE::ColorProxySelectionFirst + i), total[i]);
        }
        previous = current;
        previousEpoch = epoch;
    }
};

// GPU 자원은 batch와 같은 수명을 갖는다. atlas SRV는 다른 batch와 공유하지만
// 선택 IB와 아래 placement slot 목록은 이 batch만 변경한다.
struct CMapStaticBatchObject::COLOR_PROXY_STATE final
{
    std::shared_ptr<Engine::CModel> Model;
    std::shared_ptr<Engine::CShader> Shader;
    CMapStaticProxyBaker::REPLACEMENT_LIMITS Limits{};
    Microsoft::WRL::ComPtr<ID3D11Buffer> OriginalInstances;
    bool BindingFailed = false;
    std::vector<uint32_t> Selected, Candidate;
    std::vector<MAP_FRUSTUM_RUNTIME_STATE> CandidateFrusta;
    uint64_t Frame = UINT64_MAX, CameraRevision = UINT64_MAX;
    uint64_t DiagnosticEpoch = UINT64_MAX, SelectedIndexCount = 0u;
    MAP_STATIC_BATCH_FRAME_STATE::COLOR_PROXY_SELECTION_COUNTERS DiagnosticCounters{};
};

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
    for (auto& instance : m_Instances) instance.ColorProxySuppressed = false;
	// Initialize before Layer captures the immutable update phase mask.
	m_FrameState = desc.FrameState;

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

HRESULT CMapStaticBatchObject::Prepare_ColorProxy(CMapStaticProxyBaker& baker, const MAP_ASSET_ENTRY& asset, std::string& error)
{
    // Load_Area의 아직 공개하지 않은 batch에만 호출한다. 일부 mesh만 대체하거나
    // 현재 그림자를 버리는 대신, 전체 placement의 색상 geometry를 한 단위로 준비한다.
    error.clear();
    const auto fail = [&](const char* reason, HRESULT result)
    {
        error = reason;
        return result;
    };
    if (!m_FrameState || !m_pModelCom || m_Instances.empty()) return fail("replacement.batch_source_missing", S_FALSE);
    // 반사된 winding은 기존 Select_Pass와 SharedFront가 처리한다. World/역전치는
    // source record가 그대로 소유하며 baker가 균일 직교 affine 변환만 허용한다.
    if (m_RenderProfile.opacity != 1.f) return fail("replacement.batch_opacity", S_FALSE);
    if (!baker.Has_Replacement(m_AssetId, &error))
        return error.empty() ? fail("replacement.asset_unsupported", S_FALSE) : S_FALSE;
    try
    {
        std::vector<uint32_t> peers(m_pModelCom->Get_NumMeshes());
        for (uint32_t i = 0u; i < peers.size(); ++i) peers[i] = i;
        std::vector<Engine::CModel::STATIC_PROXY_PLACEMENT_SOURCE> sources;
        sources.reserve(m_Instances.size());
        for (const auto& instance : m_Instances)
        {
            Engine::CModel::STATIC_PROXY_PLACEMENT_SOURCE source;
            source.model = m_pModelCom.get(); source.sourceId = instance.PlacementId;
            source.sourceMeshIndices = peers;
            source.instance.World = instance.World;
            source.instance.WorldInvTranspose = instance.WorldInvTranspose;
            source.instance.vLightmapScaleBias = instance.BakedLighting.scaleBias;
            source.instance.vLightmapAverageScale = instance.BakedLighting.averageScale;
            source.instance.vLightmapDirectionalScale = instance.BakedLighting.directionalScale;
            source.instance.vStaticShadowScaleBias = instance.BakedLighting.shadowScaleBias;
            source.instance.vSourceWindOwnerPosition = instance.SourceWind.actorPositionSourceCm;
            source.instance.vSourceWindDimensionsAndRadius = instance.SourceWind.objectDimensionsAndRadiusSourceCm;
            sources.push_back(source);
        }
        auto staged = std::make_unique<COLOR_PROXY_STATE>();
        const HRESULT prepared = baker.Prepare_Replacement(m_pModelCom, asset, sources,
            staged->Model, staged->Shader, staged->Limits, error);
        if (prepared != S_OK) return prepared;
        if (!staged->Model || !staged->Shader ||
            staged->Model->Get_StaticProxySourceRanges().size() != m_Instances.size()) return fail("replacement.source_ranges_mismatch", E_UNEXPECTED);
        // 선택 확정 뒤 shader binding이 실패해도 선택된 원본만 같은 프레임에 그릴 수
        // 있도록 준비 시점의 payload를 소유한다. 프레임 중 할당/Map 실패를 추가하지 않는다.
        std::vector<VTXMESHINSTANCE> originalPayload;
        originalPayload.reserve(sources.size());
        for (const auto& source : sources) originalPayload.push_back(source.instance);
        if (originalPayload.size() > UINT32_MAX / sizeof(VTXMESHINSTANCE)) return fail("replacement.original_instance_bytes", E_INVALIDARG);
        D3D11_BUFFER_DESC originalDesc{};
        originalDesc.ByteWidth = static_cast<UINT>(originalPayload.size() * sizeof(VTXMESHINSTANCE));
        originalDesc.Usage = D3D11_USAGE_IMMUTABLE;
        originalDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        D3D11_SUBRESOURCE_DATA originalData{}; originalData.pSysMem = originalPayload.data();
        const HRESULT originalBuffer = m_pDevice->CreateBuffer(&originalDesc, &originalData, &staged->OriginalInstances);
        if (FAILED(originalBuffer)) return fail("replacement.original_instance_buffer", originalBuffer);
        staged->Selected.reserve(m_Instances.size()); staged->Candidate.reserve(m_Instances.size());
        staged->CandidateFrusta.resize(m_Instances.size());
        if (staged->Model->Prepare_StaticProxyVisibleSources({}) != S_OK) return fail("replacement.empty_selection_preflight", E_FAIL);
        m_ColorProxy = std::move(staged);
        return S_OK;
    }
    catch (const std::bad_alloc&) { return E_OUTOFMEMORY; }
    catch (const std::exception& exception) { error = exception.what(); return E_FAIL; }
}

bool CMapStaticBatchObject::Has_ColorProxySelection() const
{
    return m_ColorProxy && !m_ColorProxy->Selected.empty();
}

void CMapStaticBatchObject::Reset_ColorProxySelection()
{
    if (!m_ColorProxy) return;
    auto& proxy = *m_ColorProxy;
    proxy.Frame = proxy.CameraRevision = UINT64_MAX;
    proxy.SelectedIndexCount = 0u;
    if (proxy.Selected.empty()) return;
    for (uint32_t slot : proxy.Selected)
        if (slot < m_Instances.size()) m_Instances[slot].ColorProxySuppressed = false;
    proxy.Selected.clear();
    (void)proxy.Model->Prepare_StaticProxyVisibleSources({});
    m_bVisibleInstancesDirty = true; m_bFinalCameraPrepared = false;
    Invalidate_VisibilityCpu();
    Invalidate_ChunkSelection();
    // 원본 그림자·피킹 geometry는 계속 사용했으므로 shadow revision을 바꾸지 않는다.
}

void CMapStaticBatchObject::Prepare_ColorProxySelection(const MAP_CAMERA_CULL_SNAPSHOT* camera)
{
    // 기본 카메라에는 instance 순회·할당·GPU upload가 없다.
    if (!m_ColorProxy) return;
    auto& game = CGameInstance::Get();
    if (!m_FrameState || !m_FrameState->colorProxyEnabled)
    {
        Reset_ColorProxySelection();
        return;
    }
    auto& proxy = *m_ColorProxy;
    ColorProxyDiagnosticScope counters{ {}, proxy.DiagnosticCounters,
        m_FrameState->colorProxySelectionCounters, proxy.DiagnosticEpoch,
        m_FrameState->colorProxyDiagnosticEpoch, game.Get_Profiler() };
    counters.Add(Engine::EProfilerCounter::MapLowGeometryVisitedBatches);
    // 실패 프레임에는 GPU/HLOD가 이미 확정한 소유권을 보존하고 원본 subset으로
    // 그린다. 다음 프레임부터는 이 optional 대체를 해제해 원본 제출로 복귀한다.
    if (m_ColorProxy->BindingFailed)
    {
        counters.Add(Engine::EProfilerCounter::MapLowGeometryBindingDisabledBatches);
        if (!m_FrameState || m_ColorProxy->Frame != m_FrameState->frameNumber)
            Reset_ColorProxySelection();
        return;
    }
    const auto& settings = game.Get_MaterialRenderSettings();
    // 정책 거절은 서로 겹칠 수 있다. instance 단계의 탈락은 아래에서 첫 실패 하나만 센다.
    bool allowed = true;
    const auto rejectGate = [&](bool rejected, Engine::EProfilerCounter reason)
    {
        if (rejected) { counters.Add(reason); allowed = false; }
    };
    rejectGate(!camera, Engine::EProfilerCounter::MapLowGeometryNoCameraBatches);
    rejectGate(!game.Get_RenderOptimizationSettings().MeshLodEnabled, Engine::EProfilerCounter::MapLowGeometryMeshLodDisabledBatches);
    rejectGate(game.Is_SceneEnvironmentReplaced(), Engine::EProfilerCounter::MapLowGeometryEnvironmentReplacedBatches);
    rejectGate(!settings.bUseSourceMaterials, Engine::EProfilerCounter::MapLowGeometrySourceMaterialsDisabledBatches);
    rejectGate(settings.eDebugView != Engine::MATERIAL_DEBUG_VIEW::FINAL, Engine::EProfilerCounter::MapLowGeometryDebugViewBatches);
    rejectGate(settings.MapPBR.Is_Active(game.Get_CurrentLevelID()), Engine::EProfilerCounter::MapLowGeometryMapPbrBatches);
    rejectGate(CMapAssetRenderUtils::Is_SurfaceBindingCollectionActive(), Engine::EProfilerCounter::MapLowGeometryBindingCollectionBatches);
    rejectGate(m_FrustumCulling.bypass || m_FrustumCulling.diagnostics, Engine::EProfilerCounter::MapLowGeometryFrustumOverrideBatches);
    if (!allowed) { Reset_ColorProxySelection(); return; }
    if (proxy.Frame == m_FrameState->frameNumber && proxy.CameraRevision == camera->revision)
    {
        counters.keepPrevious = true;
        return;
    }
    const auto viewport = game.Get_ViewportSize();
    const auto* eye = game.Get_CamPosition();
    const auto& projection = camera->projection;
    if (!eye || !std::isfinite(viewport.x) || !std::isfinite(viewport.y) ||
        !(viewport.x > 0.f) || !(viewport.y > 0.f) || !(camera->lodViewScale > 0.f) ||
        projection._14 != 0.f || projection._24 != 0.f || projection._34 != 1.f || projection._44 != 0.f ||
        projection._12 != 0.f || projection._21 != 0.f || projection._13 != 0.f || projection._23 != 0.f ||
        projection._41 != 0.f || projection._42 != 0.f || !(projection._11 > 0.f) ||
        !(projection._22 > 0.f) || !(projection._33 > 1.f) || !(projection._43 < 0.f))
    {
        counters.Add(Engine::EProfilerCounter::MapLowGeometryInvalidCameraBatches);
        Reset_ColorProxySelection();
        return;
    }

    counters.Add(Engine::EProfilerCounter::MapLowGeometryTestedInstances, m_Instances.size());
    proxy.Candidate.clear();
    const double focal = .5 * (std::max)(double(projection._11) * viewport.x, double(projection._22) * viewport.y);
    for (uint32_t slot = 0u; slot < m_Instances.size(); ++slot)
    {
        const auto& instance = m_Instances[slot];
        if (!instance.Visible || instance.Suppressed || instance.CameraPreviewSuppressed)
        {
            counters.Add(Engine::EProfilerCounter::MapLowGeometryVisibilityRejectedInstances);
            continue;
        }
        const auto& center = instance.WorldBoundsCenter;
        const double dx = double(center.x) - eye->x, dy = double(center.y) - eye->y, dz = double(center.z) - eye->z;
        const double surfaceDistance = std::sqrt(dx * dx + dy * dy + dz * dz) - instance.WorldBoundsRadius;
        if (!std::isfinite(surfaceDistance) || surfaceDistance < proxy.Limits.minimumSurfaceDistanceMetres)
        {
            counters.Add(Engine::EProfilerCounter::MapLowGeometryDistanceRejectedInstances);
            continue;
        }
        float3_t view{};
        XMStoreFloat3(&view, XMVector3TransformCoord(XMLoadFloat3(&center), XMLoadFloat4x4(&camera->lodView)));
        const double radius = double(instance.WorldBoundsRadius) * camera->lodViewScale;
        const double depth = double(view.z) - radius;
        const double lateral = (std::max)(std::abs(double(view.x)), std::abs(double(view.y))) + radius;
        if (!(radius > 0.) || !(depth > -double(projection._43) / projection._33))
        {
            counters.Add(Engine::EProfilerCounter::MapLowGeometryDepthRejectedInstances);
            continue;
        }
        // 구면 전체의 투영 도함수를 상한으로 잡는다. 같은 식으로 실측한 package
        // 허용 직경보다 커지면 근거리 원본을 유지한다. m와 output pixel 단위다.
        const double diameter = 2. * radius * focal * (depth + lateral) / (depth * depth);
        if (!std::isfinite(diameter) || diameter > proxy.Limits.maximumProjectedDiameterPixels)
        {
            counters.Add(Engine::EProfilerCounter::MapLowGeometryPixelRejectedInstances);
            continue;
        }
        auto frustum = instance.FrustumState;
        MAP_FRUSTUM_CULL_DECISION decision{};
        if (CMapAssetRenderUtils::Evaluate_FrustumVisibility(m_FrustumCulling, *camera, m_AssetId,
            m_AssetGroupId, instance.PlacementId, center, instance.WorldBoundsRadius, frustum,
            decision, nullptr, MAP_FRUSTUM_CULL_DETAIL::VISIBILITY_ONLY) && !decision.shouldRender)
        {
            counters.Add(Engine::EProfilerCounter::MapLowGeometryFrustumRejectedInstances);
            continue;
        }
        proxy.Candidate.push_back(slot);
        proxy.CandidateFrusta[slot] = frustum;
    }
    counters.Add(Engine::EProfilerCounter::MapLowGeometryCandidateInstances, proxy.Candidate.size());
    // 원본 active mask를 내리기 전에 IB와 모든 shader binding을 준비한다.
    // 실패하면 이전 저폴리 선택도 해제하여 기존 CPU/GPU 원본 경로로 복귀한다.
    if (proxy.Model->Prepare_StaticProxyVisibleSources(proxy.Candidate) != S_OK ||
        (!proxy.Candidate.empty() &&
            (FAILED(CMapAssetRenderUtils::Bind_CameraCullSnapshot(proxy.Shader, *camera)) ||
             FAILED(game.Bind_CamPosition(proxy.Shader, "g_vCamPosition")) ||
             FAILED(CMapStaticProxyBaker::Bind_Replacement(proxy.Shader, proxy.Model, m_pModelCom)) ||
             FAILED(proxy.Shader->Begin(4u + CMapAssetRenderUtils::Select_Pass(m_RenderProfile, m_bMirrored))))))
    {
        counters.Add(Engine::EProfilerCounter::MapLowGeometryPreflightFailedBatches);
        Reset_ColorProxySelection();
        proxy.Frame = m_FrameState->frameNumber; proxy.CameraRevision = camera->revision;
        return;
    }
    const bool changed = proxy.Candidate != proxy.Selected;
    if (changed)
    {
        for (uint32_t slot : proxy.Selected) m_Instances[slot].ColorProxySuppressed = false;
        for (uint32_t slot : proxy.Candidate) m_Instances[slot].ColorProxySuppressed = true;
        proxy.Selected.swap(proxy.Candidate);
        m_bVisibleInstancesDirty = true; m_bFinalCameraPrepared = false;
        Invalidate_VisibilityCpu();
    }
    proxy.SelectedIndexCount = 0u;
    const auto ranges = proxy.Model->Get_StaticProxySourceRanges();
    for (uint32_t slot : proxy.Selected)
    {
        m_Instances[slot].FrustumState = proxy.CandidateFrusta[slot];
        proxy.SelectedIndexCount += ranges[slot].indexCount;
    }
    counters.Add(Engine::EProfilerCounter::MapLowGeometrySelectedInstances, proxy.Selected.size());
    // lazy HLOD가 같은 batch로 다시 들어와도 이번 선택을 반복하지 않는다.
    proxy.Frame = m_FrameState->frameNumber; proxy.CameraRevision = camera->revision;
    if (changed) Invalidate_ChunkSelection();
}

HRESULT CMapStaticBatchObject::Render_ColorProxy()
{
    if (!Has_ColorProxySelection() || CGameInstance::Get().Is_SceneEnvironmentReplaced()) return S_OK;
    auto& proxy = *m_ColorProxy;
    auto& game = CGameInstance::Get();
    const auto* camera = CMapAssetRenderUtils::Capture_CameraCullSnapshotView();
    if (!camera || FAILED(CMapAssetRenderUtils::Bind_CameraCullSnapshot(proxy.Shader, *camera)) ||
        FAILED(game.Bind_CamPosition(proxy.Shader, "g_vCamPosition")) ||
        FAILED(CMapStaticProxyBaker::Bind_Replacement(proxy.Shader, proxy.Model, m_pModelCom)) ||
        FAILED(proxy.Shader->Begin(4u + CMapAssetRenderUtils::Select_Pass(m_RenderProfile, m_bMirrored))))
        return Render_ColorProxyFallback(camera);
    const HRESULT result = proxy.Model->Render_StaticProxy();
    // 현재 CModel API는 DrawIndexed 이전의 검증 실패만 HRESULT로 반환한다.
    // 따라서 이 분기는 이미 성공한 저폴리 draw를 원본과 중복 제출하지 않는다.
    if (FAILED(result)) return Render_ColorProxyFallback(camera);
    if (result == S_OK)
    {
        if (auto* profiler = game.Get_Profiler())
        {
            profiler->Add_Counter(Engine::EProfilerCounter::MapLowGeometryDrawCalls);
            profiler->Add_Counter(Engine::EProfilerCounter::MapLowGeometrySubmittedIndices, proxy.SelectedIndexCount);
        }
    }
    if (result == S_OK && m_FrameState && m_FrameState->geometryDiagnostics)
    {
        auto& diagnostics = *m_FrameState->geometryDiagnostics;
        try
        {
            MAP_GEOMETRY_DIAGNOSTIC_DRAW draw;
            draw.path = "low_geometry_placement"; draw.assetId = m_AssetId;
            draw.meshIndex = UINT32_MAX; // 전체 source mesh를 합친 하나의 placement 범위다.
            draw.instanceCount = 1u; draw.vertexBufferCount = proxy.Model->Get_StaticProxyStats()->vertexCount;
            const auto ranges = proxy.Model->Get_StaticProxySourceRanges();
            for (uint32_t slot : proxy.Selected)
            {
                draw.indexCount += ranges[slot].indexCount;
                draw.sourcePlacementIds.push_back(ranges[slot].sourceId);
            }
            draw.sourceIdsKnown = true;
            diagnostics.Record(std::move(draw));
        }
        catch (...) { ++diagnostics.droppedDraws; }
    }
    return result;
}

HRESULT CMapStaticBatchObject::Render_ColorProxyFallback(const MAP_CAMERA_CULL_SNAPSHOT* camera)
{
    if (!Has_ColorProxySelection()) return S_OK;
    auto& proxy = *m_ColorProxy;
    proxy.BindingFailed = true;
    auto& game = CGameInstance::Get();
    if (auto* profiler = game.Get_Profiler())
        profiler->Add_Counter(Engine::EProfilerCounter::MapLowGeometryLateFailures);
    if (!proxy.OriginalInstances || !m_pModelCom || !m_pShaderCom) return E_FAIL;
    const HRESULT cameraBind = camera ? CMapAssetRenderUtils::Bind_CameraCullSnapshot(m_pShaderCom, *camera) :
        (FAILED(game.Bind_Transform(m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)) ||
         FAILED(game.Bind_Transform(m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)) ? E_FAIL : S_OK);
    if (FAILED(cameraBind)) return cameraBind;
    const uint32_t cull = CMapAssetRenderUtils::Select_Pass(m_RenderProfile, m_bMirrored);
    if (cull > 2u) return E_UNEXPECTED;
    // 현재 visible LOD bounds는 대체하지 않은 slot의 범위다. 실패한 Selected에
    // 그 범위를 재사용하지 않고 한 프레임만 원본 LOD0으로 복구한다.
    for (uint32_t mesh = 0u; mesh < m_pModelCom->Get_NumMeshes(); ++mesh)
    {
        // HLOD claim은 이 프레임의 나머지 원본 slot만 소유한다. Selected는 claim을
        // 건너뛰지 않고 전체 원본 mesh를 그려야 구멍과 중복 제출을 함께 피할 수 있다.
        const auto* surface = m_pModelCom->Get_MaterialSurface(mesh);
        const uint32_t pass = game.Get_MaterialRenderSettings().bUseSourceMaterials && surface &&
            surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED ? 24u + cull : cull;
        if (FAILED(CMapAssetRenderUtils::Bind_Material(m_pModelCom, m_pShaderCom, mesh,
            m_RenderProfile, Get_RenderElapsedTime(), nullptr, m_AssetId, nullptr, nullptr,
            MAP_MATERIAL_BINDING_MODE::INSTANCED)) || FAILED(m_pShaderCom->Begin(pass))) return E_FAIL;
        for (uint32_t slot : proxy.Selected)
        {
            const HRESULT draw = m_pModelCom->Render_Instanced(mesh, proxy.OriginalInstances.Get(),
                sizeof(VTXMESHINSTANCE), 1u, slot * sizeof(VTXMESHINSTANCE), nullptr);
            if (FAILED(draw)) return draw;
            if (draw == S_OK)
            {
                if (auto* profiler = game.Get_Profiler())
                {
                    profiler->Add_Counter(Engine::EProfilerCounter::MapLowGeometryFallbackDrawCalls);
                    profiler->Add_Counter(Engine::EProfilerCounter::MapLowGeometryFallbackIndices,
                        m_pModelCom->Get_StaticMeshSelectedIndexCount(mesh, nullptr));
                }
            }
            if (!m_FrameState || !m_FrameState->geometryDiagnostics) continue;
            auto& diagnostics = *m_FrameState->geometryDiagnostics;
            try
            {
                MAP_GEOMETRY_DIAGNOSTIC_DRAW row;
                row.path = "low_geometry_source_fallback"; row.assetId = m_AssetId;
                row.meshIndex = mesh; row.instanceCount = 1u;
                row.vertexBufferCount = m_pModelCom->Get_MeshVertexCount(mesh);
                row.indexCount = m_pModelCom->Get_StaticMeshSelectedIndexCount(mesh, nullptr);
                row.sourcePlacementIds.push_back(m_Instances[slot].PlacementId); row.sourceIdsKnown = true;
                diagnostics.Record(std::move(row));
            }
            catch (...) { ++diagnostics.droppedDraws; }
        }
    }
    return S_OK;
}

void CMapStaticBatchObject::Update(
	f32_t fTimeDelta)
{
	if (!m_FrameState)
		m_fElapsedTime += fTimeDelta;
}

void CMapStaticBatchObject::Late_Update(
	f32_t fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);
	if (m_FrameState)
		return;
	m_bFinalCameraPrepared = false;
    Invalidate_VisibilityCpu();

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

bool CMapStaticBatchObject::Is_ChunkMeshClaimed(uint32_t mesh) const
{
    return mesh < m_ChunkClaims.size() && m_ChunkClaims[mesh] && m_ChunkClaims[mesh]->Is_Active(
        mesh < m_ChunkClaimMemberIndices.size() ? m_ChunkClaimMemberIndices[mesh] : UINT32_MAX);
}

bool CMapStaticBatchObject::Are_AllChunkMeshesClaimed() const
{
    if (!m_pModelCom || m_pModelCom->Get_NumMeshes() == 0u ||
        m_ChunkClaims.size() < m_pModelCom->Get_NumMeshes()) return false;
    for (uint32_t mesh = 0u; mesh < m_pModelCom->Get_NumMeshes(); ++mesh)
        if (!Is_ChunkMeshClaimed(mesh)) return false;
    return true;
}

bool CMapStaticBatchObject::Is_GpuVisibilityActive() const
{
    const auto scene = m_GpuScene.lock();
    return scene && scene->Can_Use(*this);
}

bool CMapStaticBatchObject::Is_GpuVisibilityReady() const
{
    const auto scene = m_GpuScene.lock();
    return scene && scene->Is_Ready(*this);
}

void CMapStaticBatchObject::Invalidate_ChunkClaims()
{
    for (const auto& claim : m_ChunkClaims)
    {
        if (!claim || !claim->valid) continue;
        claim->valid = false;
        if (auto* profiler = CGameInstance::Get().Get_Profiler())
            profiler->Add_Counter(Engine::EProfilerCounter::MapChunkInvalidations);
    }
}

void CMapStaticBatchObject::Invalidate_ChunkSelection()
{
    // 같은 claim을 쓰는 원본 mesh와 대체 chunk가 다음 조회에서 함께 재판정된다.
    // 이미 TRS 변경으로 무효화된 claim은 표시 복귀만으로 되살리지 않는다.
    for (const auto& claim : m_ChunkClaims)
    {
        if (claim && claim->valid)
        {
            claim->Invalidate_FrameSelection();
        }
    }
}

bool_t CMapStaticBatchObject::Try_GetFinalCameraSpatialBounds(FINAL_CAMERA_SPATIAL_BOUNDS& bounds) const
{
    bounds = {};
    if (m_FrustumCulling.bypass || m_FrustumCulling.diagnostics) return false;
    const f32_t policyValues[] = { m_FrustumCulling.baseMargin, m_FrustumCulling.largeObjectRadiusThreshold,
        m_FrustumCulling.largeObjectAbsoluteMargin, m_FrustumCulling.largeObjectRelativeMargin };
    for (float value : policyValues) if (!std::isfinite(value) || value < 0.f) return false;
    if (m_bBatchBoundsDirty) Rebuild_BatchCullBounds();
    if (!m_bHasBatchBounds) return false;
    const double radius = m_BatchBounds.w;
    double margin = m_FrustumCulling.baseMargin;
    if (m_AssetGroupId == "landscape" || (m_FrustumCulling.largeObjectRadiusThreshold > 0.f &&
        radius >= m_FrustumCulling.largeObjectRadiusThreshold))
        margin = (std::max)({ margin, double(m_FrustumCulling.largeObjectAbsoluteMargin),
            radius * m_FrustumCulling.largeObjectRelativeMargin });
    const double expanded = radius + margin;
    if (!std::isfinite(expanded) || expanded <= 0. || expanded >= FLT_MAX) return false;
    bounds.Center = { m_BatchBounds.x, m_BatchBounds.y, m_BatchBounds.z };
    bounds.Radius = std::nextafter(static_cast<float>(expanded), (std::numeric_limits<float>::infinity)());
    bounds.RejectGraceFrames = (std::max)(m_FrustumCulling.rejectHysteresisFrames,
        m_BatchFrustumState.rejectGraceFrames);
    bounds.ShadowCaster = m_RenderProfile.castsShadow;
    return std::isfinite(bounds.Radius);
}

bool_t CMapStaticBatchObject::Try_GetStaticOcclusionDesc(STATIC_OCCLUSION_DESC& output) const
{
    output = {};
    if (Has_ColorProxySelection()) return false;
    auto& game = CGameInstance::Get();
    // GPU cohort의 callback은 다른 cell까지 대표한다. 이 cell의 CPU bounds로
    // callback을 제거하면 전체 cohort가 사라질 수 있어 GPU 준비 완료분은 제외한다.
    // 이번 프레임에 CPU fallback한 batch는 기존 가림/인접 병합을 그대로 소비한다.
    if (Is_GpuVisibilityReady() || !m_FrameState || !m_pModelCom || !Is_FinalCameraPrepared() || m_VisibleInstances.empty() ||
        m_iVisibilitySettingsRevision != game.Get_MapVisibilitySettings().Revision ||
        game.Is_SceneEnvironmentReplaced() || !game.Get_MaterialRenderSettings().bUseSourceMaterials ||
        CMapAssetRenderUtils::Is_SurfaceBindingCollectionActive() || m_FrustumCulling.bypass || m_FrustumCulling.diagnostics ||
        m_RenderProfile.renderMode != MAP_ASSET_RENDER_MODE::DEFERRED || m_RenderProfile.opacity != 1.f ||
        !m_bVisibleOcclusionBounds || m_iStaticShadowRevision == 0u) return false;
    const auto* camera = CMapAssetRenderUtils::Capture_CameraCullSnapshotView();
    if (!camera || !m_bVisibleInstancesUsedCamera || m_iVisibleCameraRevision != camera->revision) return false;
    STATIC_OCCLUSION_DESC staged{};
    staged.BoundsMin = m_VisibleOcclusionMin; staged.BoundsMax = m_VisibleOcclusionMax;
    staged.Revision = m_iStaticShadowRevision;
    Engine::MESH_SCREEN_LOD_DESC lod{};
    const bool hasLod = Build_ScreenLodView(*camera, lod);
    uint64_t occluderTriangles = 0u;
    for (uint32_t mesh = 0u; mesh < m_pModelCom->Get_NumMeshes(); ++mesh)
    {
        if (Is_ChunkMeshClaimed(mesh)) continue;
        if (!StaticOcclusionBoundsMesh(*m_pModelCom, mesh)) return false;
        const auto* view = Uses_MeshScreenLod(mesh, hasLod) ? &lod : nullptr;
        const size_t draws = game.Get_RenderOptimizationSettings().MapInstancingEnabled ? 1u : m_VisibleInstances.size();
        if (draws > UINT32_MAX - staged.Draws) return false;
        staged.Draws += static_cast<uint32_t>(draws);
        staged.Indices += uint64_t(m_pModelCom->Get_StaticMeshSelectedIndexCount(mesh, view)) * m_VisibleInstances.size();
        Engine::CModel::STATIC_OCCLUSION_MESH geometry;
        if (StaticOpaqueOcclusionMesh(*m_pModelCom, mesh) && m_pModelCom->Get_StaticMeshLodLevel(mesh, view) == 0u &&
            m_pModelCom->Try_GetStaticOcclusionMesh(mesh, geometry))
            occluderTriangles += uint64_t(geometry.indices.size() / 3u) * m_VisibleInstances.size();
    }
    if (staged.Draws == 0u || staged.Indices == 0u) return false;
    staged.OccluderTriangles = static_cast<uint32_t>((std::min)(occluderTriangles, uint64_t(UINT32_MAX)));
    output = staged;
    return true;
}

// Header additions beside Try_GetStaticOcclusionDesc:
// bool_t Try_GetVisibilityDepthDesc(STATIC_OCCLUSION_DESC& output) const override;
// HRESULT Render_VisibilityDepth(uint32_t triangleBudget, uint32_t& drawnTriangles) override;
// These are candidate methods only; coordinator owner integrates them into the full-file plan.

bool_t CMapStaticBatchObject::Try_GetVisibilityDepthDesc(STATIC_OCCLUSION_DESC& output) const
{
    output = {};
    // 일부 slot만 저폴리여도 남은 원본의 가림자 역할은 유지한다.
    // descriptor와 실제 depth draw가 같은 suppression 조건을 사용해야 한다.
    auto& game = CGameInstance::Get();
    if (!m_FrameState || !m_pModelCom || !m_pShaderCom || m_Instances.empty() ||
        game.Is_SceneEnvironmentReplaced() || !game.Get_MaterialRenderSettings().bUseSourceMaterials ||
        CMapAssetRenderUtils::Is_SurfaceBindingCollectionActive() || m_FrustumCulling.bypass ||
        m_FrustumCulling.diagnostics || m_RenderProfile.renderMode != MAP_ASSET_RENDER_MODE::DEFERRED ||
        m_RenderProfile.opacity != 1.f || !m_iStaticShadowRevision) return false;
    const auto* camera = CMapAssetRenderUtils::Capture_CameraCullSnapshotView();
    const auto* cameraPosition = game.Get_CamPosition();
    if (!camera || !cameraPosition) return false;
    const auto& settings = game.Get_MapVisibilitySettings();
    // GPU는 CPU visible payload를 만들지 않을 수 있다. 저작 visible/suppress와 현재
    // HLOD claim만 읽고, 거리 제거 자체가 불가능한 근거리 원본만 depth 제공자로 삼는다.
    // 화면 크기 문턱이나 GPU hysteresis와 무관하게 반드시 색상 draw에 남는 범위다.
    const auto accepts = [&](const FMapStaticInstance& instance)
    {
        if (!instance.Visible || instance.Suppressed || instance.CameraPreviewSuppressed ||
            instance.ColorProxySuppressed ||
            !(instance.WorldBoundsRadius > 0.f) || !std::isfinite(instance.WorldBoundsRadius)) return false;
        const float values[] = { instance.WorldBoundsCenter.x, instance.WorldBoundsCenter.y,
            instance.WorldBoundsCenter.z, cameraPosition->x, cameraPosition->y, cameraPosition->z };
        for (float value : values) if (!std::isfinite(value)) return false;
        float limit = 0.f, pixels = 0.f;
        if (CMapAssetRenderUtils::Get_SceneDistancePolicy(m_SceneVisibilityKind,
            instance.WorldBoundsRadius, settings, limit, pixels))
        {
            const double x = double(instance.WorldBoundsCenter.x) - cameraPosition->x;
            const double y = double(instance.WorldBoundsCenter.y) - cameraPosition->y;
            const double z = double(instance.WorldBoundsCenter.z) - cameraPosition->z;
            // GPU 후보의 distance는 공통 base policy에 1.1 margin을 추가한다.
            // 여기서는 margin 없는 더 짧은 거리 안에만 원본을 채운다.
            if (std::sqrt(x*x + y*y + z*z) > double(limit) + instance.WorldBoundsRadius) return false;
        }
        return true;
    };
    STATIC_OCCLUSION_DESC staged{};
    staged.BoundsMin = { FLT_MAX, FLT_MAX, FLT_MAX };
    staged.BoundsMax = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
    uint64_t instanceCount = 0u;
    for (const auto& instance : m_Instances)
    {
        if (!accepts(instance)) continue;
        for (size_t axis = 0u; axis < 3u; ++axis)
        {
            const float lo = (&instance.WorldBoundsCenter.x)[axis] - instance.WorldBoundsRadius;
            const float hi = (&instance.WorldBoundsCenter.x)[axis] + instance.WorldBoundsRadius;
            if (!std::isfinite(lo) || !std::isfinite(hi)) return false;
            (&staged.BoundsMin.x)[axis] = (std::min)((&staged.BoundsMin.x)[axis], lo);
            (&staged.BoundsMax.x)[axis] = (std::max)((&staged.BoundsMax.x)[axis], hi);
        }
        ++instanceCount;
    }
    if (!instanceCount) return false;
    for (uint32_t mesh = 0u; mesh < m_pModelCom->Get_NumMeshes(); ++mesh)
    {
        // 원본으로 depth를 만든 뒤 축약 색상 geometry의 구멍을 가리는 오류를 피한다.
        // 현재 CPU/GPU 어느 쪽으로도 축약할 수 없는 불투명 원본만 opt-in한다.
        if (Is_ChunkMeshClaimed(mesh) || !StaticOpaqueOcclusionMesh(*m_pModelCom, mesh) ||
            m_pModelCom->Has_MaterialTextureOverrides(mesh) || m_pModelCom->Can_UseStaticMeshLod(mesh)) continue;
        const uint32_t indices = m_pModelCom->Get_MeshIndexCount(mesh);
        if (!indices || indices % 3u) continue;
        ++staged.Draws;
        staged.Indices += uint64_t(indices) * instanceCount;
    }
    if (!staged.Draws || !staged.Indices) return false;
    staged.OccluderTriangles = static_cast<uint32_t>((std::min)(staged.Indices / 3u, uint64_t(UINT32_MAX)));
    staged.Revision = m_iStaticShadowRevision;
    output = staged;
    return true;
}

HRESULT CMapStaticBatchObject::Render_VisibilityDepth(uint32_t triangleBudget, uint32_t& drawnTriangles)
{
    drawnTriangles = 0u;
    STATIC_OCCLUSION_DESC descriptor{};
    if (!triangleBudget || !Try_GetVisibilityDepthDesc(descriptor)) return S_FALSE;
    auto& game = CGameInstance::Get();
    const auto* camera = CMapAssetRenderUtils::Capture_CameraCullSnapshotView();
    const auto* cameraPosition = game.Get_CamPosition();
    if (!camera || !cameraPosition) return S_FALSE;
    const auto& settings = game.Get_MapVisibilitySettings();
    const auto accepts = [&](const FMapStaticInstance& instance)
    {
        if (!instance.Visible || instance.Suppressed || instance.CameraPreviewSuppressed ||
            instance.ColorProxySuppressed ||
            !(instance.WorldBoundsRadius > 0.f) || !std::isfinite(instance.WorldBoundsRadius)) return false;
        const float values[] = { instance.WorldBoundsCenter.x, instance.WorldBoundsCenter.y,
            instance.WorldBoundsCenter.z, cameraPosition->x, cameraPosition->y, cameraPosition->z };
        for (float value : values) if (!std::isfinite(value)) return false;
        float limit = 0.f, pixels = 0.f;
        if (CMapAssetRenderUtils::Get_SceneDistancePolicy(m_SceneVisibilityKind,
            instance.WorldBoundsRadius, settings, limit, pixels))
        {
            const double x = double(instance.WorldBoundsCenter.x) - cameraPosition->x;
            const double y = double(instance.WorldBoundsCenter.y) - cameraPosition->y;
            const double z = double(instance.WorldBoundsCenter.z) - cameraPosition->z;
            if (std::sqrt(x*x + y*y + z*z) > double(limit) + instance.WorldBoundsRadius) return false;
        }
        return true;
    };
    const uint32_t cull = CMapAssetRenderUtils::Select_Pass(m_RenderProfile, m_bMirrored);
    if (cull > 2u || FAILED(CMapAssetRenderUtils::Bind_CameraCullSnapshot(m_pShaderCom, *camera))) return E_FAIL;
    try
    {
        for (uint32_t mesh = 0u; mesh < m_pModelCom->Get_NumMeshes(); ++mesh)
        {
            if (Is_ChunkMeshClaimed(mesh) || !StaticOpaqueOcclusionMesh(*m_pModelCom, mesh) ||
                m_pModelCom->Has_MaterialTextureOverrides(mesh) || m_pModelCom->Can_UseStaticMeshLod(mesh)) continue;
            const uint32_t triangles = m_pModelCom->Get_MeshIndexCount(mesh) / 3u;
            if (!triangles || triangles > triangleBudget - drawnTriangles) continue;
            const size_t capacity = (triangleBudget - drawnTriangles) / triangles;
            m_CandidateLightingBankInstances.clear();
            m_CandidateLightingBankInstances.reserve((std::min)(capacity, m_Instances.size()));
            for (const auto& instance : m_Instances)
            {
                if (!accepts(instance)) continue;
                VTXMESHINSTANCE payload{};
                payload.World = instance.World;
                payload.WorldInvTranspose = instance.WorldInvTranspose;
                m_CandidateLightingBankInstances.push_back(payload);
                if (m_CandidateLightingBankInstances.size() == capacity) break;
            }
            if (m_CandidateLightingBankInstances.empty()) continue;
            const uint32_t count = static_cast<uint32_t>(m_CandidateLightingBankInstances.size());
            // 임시 bank VB만 사용한다. GPU resident payload와 CPU fallback visible VB는
            // 바꾸지 않는다. 이 pass의 VS는 World만 읽고 PS는 null인 기존 opaque depth다.
            if (FAILED(Upload_LightingBankInstances()) || FAILED(m_pShaderCom->Begin(21u + cull)) ||
                FAILED(m_pModelCom->Render_Instanced(mesh, m_pLightingBankInstanceBuffer.Get(),
                    sizeof(VTXMESHINSTANCE), count))) return E_FAIL;
            drawnTriangles += triangles * count;
            if (drawnTriangles == triangleBudget) break;
        }
    }
    catch (const std::bad_alloc&) { return E_OUTOFMEMORY; }
    catch (const std::length_error&) { return E_OUTOFMEMORY; }
    return drawnTriangles ? S_OK : S_FALSE;
}

uint32_t CMapStaticBatchObject::Rasterize_StaticOccluder(Engine::COcclusionCuller& culler, uint32_t triangleBudget) const
{
    STATIC_OCCLUSION_DESC descriptor;
    if (triangleBudget == 0u || !Try_GetStaticOcclusionDesc(descriptor) || descriptor.OccluderTriangles == 0u) return 0u;
    const auto* camera = CMapAssetRenderUtils::Capture_CameraCullSnapshotView();
    if (!camera) return 0u;
    Engine::MESH_SCREEN_LOD_DESC lod{};
    const bool hasLod = Build_ScreenLodView(*camera, lod);
    const auto pass = CMapAssetRenderUtils::Select_Pass(m_RenderProfile, m_bMirrored);
    const auto cull = pass == 0u ? Engine::COcclusionCuller::CULL_MODE::BACK :
        pass == 1u ? Engine::COcclusionCuller::CULL_MODE::FRONT : Engine::COcclusionCuller::CULL_MODE::NONE;
    const uint32_t initialRasterized = culler.GetRasterizedTriangles();
    uint32_t attempted = 0u;
    for (uint32_t mesh = 0u; mesh < m_pModelCom->Get_NumMeshes(); ++mesh)
    {
        if (Is_ChunkMeshClaimed(mesh) || !StaticOpaqueOcclusionMesh(*m_pModelCom, mesh)) continue;
        const auto* view = Uses_MeshScreenLod(mesh, hasLod) ? &lod : nullptr;
        if (m_pModelCom->Get_StaticMeshLodLevel(mesh, view) != 0u) continue;
        Engine::CModel::STATIC_OCCLUSION_MESH geometry;
        if (!m_pModelCom->Try_GetStaticOcclusionMesh(mesh, geometry)) continue;
        const uint32_t triangles = static_cast<uint32_t>(geometry.indices.size() / 3u);
        if (triangles == 0u || triangles > triangleBudget - attempted) continue;
        for (const auto& instance : m_VisibleInstances)
        {
            if (triangles > triangleBudget - attempted) break;
            attempted += triangles;
            (void)culler.Rasterize(geometry.positions, geometry.vertexCount, geometry.strideBytes,
                geometry.indices, instance.World, cull);
        }
    }
    return culler.GetRasterizedTriangles() - initialRasterized;
}

void CMapStaticBatchObject::Rebuild_InstanceOcclusionBounds(FMapStaticInstance& instance) const
{
    instance.OcclusionBoundsValid = m_bOcclusionGeometryEligible && m_pModelCom->Has_LocalBounds() &&
        TransformStaticBounds(m_pModelCom->Get_LocalBoundsMin(), m_pModelCom->Get_LocalBoundsMax(), instance.World,
            instance.OcclusionBoundsMin, instance.OcclusionBoundsMax);
}

void CMapStaticBatchObject::Commit_DistanceSelection(uint64_t revision)
{
    for (const auto& [index, hidden] : m_CandidateDistanceChanges)
    {
        m_Instances[index].DistanceHidden = hidden;
        m_Instances[index].DistanceSettingsRevision = revision;
    }
}

HRESULT CMapStaticBatchObject::Prepare_FinalCameraVisibility(
	const MAP_CAMERA_CULL_SNAPSHOT* cameraSnapshot)
{
    Prepare_ColorProxySelection(cameraSnapshot);
    const bool hasCamera = cameraSnapshot != nullptr;
    if (Is_FinalCameraPrepared() && m_bVisibleInstancesUsedCamera == hasCamera &&
        m_iVisibleCameraRevision == (hasCamera ? cameraSnapshot->revision : 0u))
    {
        const auto settings = CGameInstance::Get().Get_MapVisibilitySettings();
        const bool distanceEnabled = m_bDistanceEligible && hasCamera && CGameInstance::Get().Get_CamPosition() &&
            settings.DistanceEnabled && !m_FrustumCulling.bypass && !m_FrustumCulling.diagnostics &&
            !CMapAssetRenderUtils::Is_SurfaceBindingCollectionActive();
        if (m_iVisibilitySettingsRevision == settings.Revision && m_bVisibleDistanceEnabled == distanceEnabled)
            return S_OK;
    }

	// Dirty can also mean next-frame rejection grace. Real instance mutations
	// clear prepared state, so repeated consumers must not advance grace twice.
	const HRESULT visibility = Upload_VisibleInstances(cameraSnapshot);
	m_bFinalCameraPrepared = SUCCEEDED(visibility);
	if (m_bFinalCameraPrepared && m_FrameState)
		m_iPreparedFrame = m_FrameState->frameNumber;
	return visibility;
}

bool CMapStaticBatchObject::Is_FinalCameraColorCandidate(const MAP_CAMERA_CULL_SNAPSHOT* camera)
{
    if (m_iAuthoredVisibleInstanceCount == 0u) return false;
    if (m_bBatchBoundsDirty) Rebuild_BatchCullBounds();
    if (camera && m_bHasBatchBounds && !m_FrustumCulling.bypass && !m_FrustumCulling.diagnostics)
    {
        auto state = m_BatchFrustumState;
        MAP_FRUSTUM_CULL_DECISION decision{};
        const float3_t center(m_BatchBounds.x, m_BatchBounds.y, m_BatchBounds.z);
        if (CMapAssetRenderUtils::Evaluate_FrustumVisibility(m_FrustumCulling, *camera,
            m_AssetId, m_AssetGroupId, 0u, center, m_BatchBounds.w, state,
            decision, nullptr, MAP_FRUSTUM_CULL_DETAIL::VISIBILITY_ONLY))
            return decision.wouldBeVisible;
    }
    // 비유한 bounds/카메라 등 검사 실패는 원본 제출을 보존한다. 이 조회는
    // Layer의 그림자 후보를 색상에서 걸러내지만 그림자 제출 자체는 막지 않는다.
    return true;
}

void CMapStaticBatchObject::Submit_FinalCamera()
{
    Prepare_ColorProxySelection(CMapAssetRenderUtils::Capture_CameraCullSnapshotView());
    auto& game = CGameInstance::Get();
    if (game.Is_SceneEnvironmentReplaced() || m_iAuthoredVisibleInstanceCount == 0u)
        return;

    const auto gpuScene = m_GpuScene.lock();
    if (gpuScene && gpuScene->Can_Use(*this))
    {
        const auto* camera = CMapAssetRenderUtils::Capture_CameraCullSnapshotView();
        if (Is_FinalCameraColorCandidate(camera))
        {
            gpuScene->Mark_Submitted(*this);
            game.Add_RenderObject(RENDERGROUP::NONBLEND, static_pointer_cast<CGameObject>(shared_from_this()));
        }
    }
    else if (Has_ColorProxySelection() || !Are_AllChunkMeshesClaimed())
    {
        const auto* camera = CMapAssetRenderUtils::Capture_CameraCullSnapshotView();
        const HRESULT visibility = Prepare_FinalCameraVisibility(camera);
        // A failed preparation keeps the original draw callback as the error/retry path.
        if (FAILED(visibility) || !m_VisibleInstances.empty() || Has_ColorProxySelection())
            game.Add_RenderObject(RENDERGROUP::NONBLEND,
                static_pointer_cast<CGameObject>(shared_from_this()));
    }

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

HRESULT CMapStaticBatchObject::Render_AdjacentNonBlend(
    std::span<const std::shared_ptr<CGameObject>> objects, size_t& consumed)
{
    if (Has_ColorProxySelection()) { consumed = 1u; return Render(); }
    // GPU에서 이미 모은 cohort는 인접 CPU 병합이 중복 소비하지 않도록 한 번만 그린다.
    if (auto scene = m_GpuScene.lock())
    {
        const HRESULT gpu = scene->Try_Render(*this);
        if (gpu != S_FALSE) { consumed = 1u; return gpu; }
    }
    // Shader capacity limits distinct lighting bundles, not adjacent batches.
    // One bundle uses the ordinary pass; two/three use the small bank.
    constexpr size_t MINIMUM_LIGHTING_BANK_BATCHES = 2u;
    consumed = 1u;
    auto& game = CGameInstance::Get();
    // Keep authoring diagnostics and all unsupported material/geometry paths exact.
    const auto optimization = game.Get_RenderOptimizationSettings();
    if (!optimization.MapInstancingEnabled ||
        objects.size() < MINIMUM_LIGHTING_BANK_BATCHES || objects.front().get() != this ||
        game.Is_SceneEnvironmentReplaced() ||
        !game.Get_MaterialRenderSettings().bUseSourceMaterials ||
        CMapAssetRenderUtils::Is_SurfaceBindingCollectionActive())
        return Render();

    if (optimization.IdenticalBatchEnabled)
    {
        const HRESULT identical = Try_RenderIdenticalInstances(objects, consumed);
        if (identical != S_FALSE) return identical;
    }
    if (!optimization.LightingBankEnabled) return Render();

    const auto* camera = CMapAssetRenderUtils::Capture_CameraCullSnapshotView();
    const bool_t hasCamera = camera != nullptr;
    const uint64_t cameraRevision = camera ? camera->revision : 0u;
    const auto ready = [&](const CMapStaticBatchObject& batch)
    {
        if (batch.Has_ColorProxySelection() || batch.Is_GpuVisibilityReady() || !batch.Is_FinalCameraPrepared() || !batch.m_pModelCom || !batch.m_pShaderCom ||
            batch.m_pModelCom->Get_NumMeshes() == 0u ||
            batch.m_bVisibleInstancesUsedCamera != hasCamera ||
            batch.m_iVisibleCameraRevision != cameraRevision || batch.m_VisibleInstances.empty() ||
            batch.m_VisibleInstances.size() > UINT32_MAX / sizeof(VTXMESHINSTANCE)) return false;
        for (uint32_t mesh = 0u; mesh < batch.m_pModelCom->Get_NumMeshes(); ++mesh)
            if (batch.Is_ChunkMeshClaimed(mesh)) return false;
        return true;
    };
    if (!ready(*this) || !m_pModelCom->Can_BatchStaticLightingWith(*m_pModelCom))
        return Render();

    // Match each ordinary draw's own LOD decision before combining instances.
    // Shared CMesh identity below makes equal levels select the same index range.
    Engine::MESH_SCREEN_LOD_DESC screenLod{};
    const bool_t hasScreenLod = camera && m_RenderProfile.opacity >= 1.f &&
        Build_ScreenLodView(*camera, screenLod);
    const uint32_t meshCount = m_pModelCom->Get_NumMeshes();

    std::array<const Engine::CModel*, 8u> models{};
    models[0] = m_pModelCom.get();
    size_t bankCount = 1u, count = 1u;
    size_t instanceCount = m_VisibleInstances.size();
    Engine::CProfiler* const profiler = game.Get_Profiler();
    try
    {
        m_CandidateLightingBankSlots.clear();
        m_CandidateLightingBankSlots.push_back(0u);
        for (; count < objects.size(); ++count)
        {
            const auto* next = dynamic_cast<const CMapStaticBatchObject*>(objects[count].get());
            // A different object is an ordering barrier; never scan or sort past it.
            if (!next || !ready(*next) || next->m_bMirrored != m_bMirrored ||
                next->Get_RenderElapsedTime() != Get_RenderElapsedTime() || !SameInstanceProfile(m_RenderProfile, next->m_RenderProfile) ||
                !m_pModelCom->Can_BatchStaticLightingWith(*next->m_pModelCom))
                break;
            Engine::MESH_SCREEN_LOD_DESC nextLod{};
            const bool_t hasNextLod = camera && next->m_RenderProfile.opacity >= 1.f &&
                next->Build_ScreenLodView(*camera, nextLod);
            bool_t matchingLods = true;
            for (uint32_t mesh = 0u; mesh < meshCount; ++mesh)
                if (m_pModelCom->Get_StaticMeshLodLevel(mesh,
                        Uses_MeshScreenLod(mesh, hasScreenLod) ? &screenLod : nullptr) !=
                    next->m_pModelCom->Get_StaticMeshLodLevel(mesh,
                        next->Uses_MeshScreenLod(mesh, hasNextLod) ? &nextLod : nullptr))
                { matchingLods = false; break; }
            if (!matchingLods) break;
            if (next->m_VisibleInstances.size() > UINT32_MAX / sizeof(VTXMESHINSTANCE) - instanceCount)
                break;
            size_t slot = 0u;
            for (; slot < bankCount; ++slot)
                if (models[slot]->Has_SameStaticLightingTextures(*next->m_pModelCom)) break;
            if (slot == bankCount)
            {
                if (bankCount == models.size()) break;
                models[bankCount++] = next->m_pModelCom.get();
            }
            m_CandidateLightingBankSlots.push_back(static_cast<uint8_t>(slot));
            instanceCount += next->m_VisibleInstances.size();
        }
    }
    catch (const std::bad_alloc&) { return Render(); }
    catch (const std::length_error&) { return Render(); }
    if (count < MINIMUM_LIGHTING_BANK_BATCHES)
        return Render();

    Engine::CProfilerWorkScope renderWork(profiler, Engine::EProfilerWork::MapBatchRender);
    try
    {
        m_CandidateLightingBankInstances.clear();
        m_CandidateLightingBankInstances.reserve(instanceCount);
        for (size_t batch = 0u; batch < count; ++batch)
        {
            const auto& source = static_cast<const CMapStaticBatchObject*>(objects[batch].get())->m_VisibleInstances;
            const size_t first = m_CandidateLightingBankInstances.size();
            m_CandidateLightingBankInstances.insert(m_CandidateLightingBankInstances.end(),
                source.begin(), source.end());
            // The single-bundle ordinary pass retains every original payload byte.
            if (bankCount > 1u)
                for (size_t i = first; i < m_CandidateLightingBankInstances.size(); ++i)
                    m_CandidateLightingBankInstances[i].vLightmapDirectionalScale.w =
                        static_cast<float>(m_CandidateLightingBankSlots[batch]);
        }
        if (FAILED(Upload_LightingBankInstances()))
            return Render();
    }
    catch (const std::bad_alloc&) { return Render(); }
    catch (const std::length_error&) { return Render(); }

    const HRESULT cameraBind = camera ?
        CMapAssetRenderUtils::Bind_CameraCullSnapshot(m_pShaderCom, *camera) :
        (FAILED(game.Bind_Transform(m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)) ||
         FAILED(game.Bind_Transform(m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)) ? E_FAIL : S_OK);
    if (FAILED(cameraBind))
        return Render();
    const uint32_t pass = CMapAssetRenderUtils::Select_Pass(m_RenderProfile, m_bMirrored);
    if (pass > 2u)
        return Render();
    const uint32_t noBank = 0u;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_MapLightingBankSize", &noBank, sizeof(noBank))))
        return Render();
    uint32_t submittedMeshes = 0u;
    const auto failedPreparation = [&](HRESULT failure)
    {
        // After any mesh was submitted, restarting ordinary Render would draw
        // that successful prefix again. Only an untouched batch may fall back.
        (void)m_pShaderCom->Bind_RawValue("g_MapLightingBankSize", &noBank, sizeof(noBank));
        return submittedMeshes == 0u ? Render() : failure;
    };
    for (uint32_t mesh = 0u; mesh < meshCount; ++mesh)
    {
        {
            Engine::CProfilerDetailScope scope(profiler, "Map.Batch.Material.Bind");
            Engine::CProfilerWorkScope work(profiler, Engine::EProfilerWork::MapBatchMaterial);
            HRESULT result = CMapAssetRenderUtils::Bind_Material(m_pModelCom, m_pShaderCom,
                mesh, m_RenderProfile, Get_RenderElapsedTime(), nullptr, m_AssetId, nullptr, nullptr,
                MAP_MATERIAL_BINDING_MODE::INSTANCED);
            if (FAILED(result)) return failedPreparation(result);
            if (bankCount > 1u)
            {
                result = m_pModelCom->Bind_StaticLightingBankMesh(m_pShaderCom,
                    std::span<const Engine::CModel* const>(models.data(), bankCount), mesh);
                if (FAILED(result)) return failedPreparation(result);
            }
        }
        {
            Engine::CProfilerDetailScope scope(profiler, "Map.Batch.Pass.Apply");
            Engine::CProfilerWorkScope work(profiler, Engine::EProfilerWork::MapBatchPass);
            const uint32_t bankPass = bankCount == 1u ? 24u : bankCount < 4u ? 30u : 27u;
            const HRESULT result = m_pShaderCom->Begin(bankPass + pass);
            if (FAILED(result)) return failedPreparation(result);
        }
        // Every constituent selected this mesh's same immutable index range
        // from its own visible bounds before any draw started.
        HRESULT result = E_FAIL;
        {
            Engine::CProfilerDetailScope scope(profiler, "Map.Batch.Mesh.Submit");
            Engine::CProfilerWorkScope work(profiler, Engine::EProfilerWork::MapBatchDraw);
            result = m_pModelCom->Render_Instanced(mesh, m_pLightingBankInstanceBuffer.Get(),
                sizeof(VTXMESHINSTANCE), static_cast<uint32_t>(instanceCount), 0u,
                Uses_MeshScreenLod(mesh, hasScreenLod) ? &screenLod : nullptr);
        }
        if (FAILED(result))
        {
            (void)m_pShaderCom->Bind_RawValue("g_MapLightingBankSize", &noBank, sizeof(noBank));
            return result;
        }
        Record_GeometryDiagnosticDraw("cpu_lighting_bank", mesh,
            static_cast<uint32_t>(instanceCount), Uses_MeshScreenLod(mesh, hasScreenLod) ? &screenLod : nullptr,
            objects.first(count));
        ++submittedMeshes;
    }
    const HRESULT reset = m_pShaderCom->Bind_RawValue("g_MapLightingBankSize", &noBank, sizeof(noBank));
    consumed = count;
    if (profiler)
    {
        profiler->Add_Counter(Engine::EProfilerCounter::MapBatchVisibleRenders);
        profiler->Add_Counter(Engine::EProfilerCounter::MapLightingBankSourceDraws, count * meshCount);
        profiler->Add_Counter(Engine::EProfilerCounter::MapLightingBankDraws, meshCount);
    }
    return reset;
}

HRESULT CMapStaticBatchObject::Try_RenderIdenticalInstances(
    std::span<const std::shared_ptr<CGameObject>> objects, size_t& consumed)
{
    const auto* camera = CMapAssetRenderUtils::Capture_CameraCullSnapshotView();
    const auto ready = [&](const CMapStaticBatchObject& batch)
    {
        return !batch.Has_ColorProxySelection() && !batch.Is_GpuVisibilityReady() && batch.Is_FinalCameraPrepared() && batch.m_pModelCom && batch.m_pShaderCom &&
            batch.m_bVisibleInstancesUsedCamera == (camera != nullptr) &&
            batch.m_iVisibleCameraRevision == (camera ? camera->revision : 0u) &&
            !batch.m_VisibleInstances.empty() &&
            batch.m_VisibleInstances.size() <= UINT32_MAX / sizeof(VTXMESHINSTANCE);
    };
    if (objects.size() < 2u || !ready(*this) ||
        !m_pModelCom->Can_ShareStaticInstanceStateWith(*m_pModelCom)) return S_FALSE;
    Engine::MESH_SCREEN_LOD_DESC firstView{};
    const bool_t firstLod = camera && m_RenderProfile.opacity >= 1.f && Build_ScreenLodView(*camera, firstView);
    size_t count = 1u, instanceCount = m_VisibleInstances.size();
    uint32_t drawnMeshes = 0u;
    for (uint32_t mesh = 0u; mesh < m_pModelCom->Get_NumMeshes(); ++mesh)
        if (!Is_ChunkMeshClaimed(mesh)) ++drawnMeshes;
    if (drawnMeshes == 0u) return S_FALSE;
    for (; count < objects.size(); ++count)
    {
        const auto* next = dynamic_cast<const CMapStaticBatchObject*>(objects[count].get());
        // The existing submission order and every non-map object remain barriers.
        if (!next || !ready(*next) || next->m_bMirrored != m_bMirrored ||
            next->Get_RenderElapsedTime() != Get_RenderElapsedTime() ||
            !SameInstanceProfile(m_RenderProfile, next->m_RenderProfile) ||
            !m_pModelCom->Can_ShareStaticInstanceStateWith(*next->m_pModelCom) ||
            next->m_VisibleInstances.size() > UINT32_MAX / sizeof(VTXMESHINSTANCE) - instanceCount)
            break;
        Engine::MESH_SCREEN_LOD_DESC nextView{};
        const bool_t nextLod = camera && next->m_RenderProfile.opacity >= 1.f &&
            next->Build_ScreenLodView(*camera, nextView);
        bool_t matching = true;
        for (uint32_t mesh = 0u; mesh < m_pModelCom->Get_NumMeshes(); ++mesh)
        {
            const bool_t claimed = Is_ChunkMeshClaimed(mesh);
            if (claimed != next->Is_ChunkMeshClaimed(mesh)) { matching = false; break; }
            if (claimed) continue;
            if (m_pModelCom->Get_StaticMeshLodLevel(mesh, Uses_MeshScreenLod(mesh, firstLod) ? &firstView : nullptr) !=
                next->m_pModelCom->Get_StaticMeshLodLevel(mesh, next->Uses_MeshScreenLod(mesh, nextLod) ? &nextView : nullptr))
            { matching = false; break; }
        }
        if (!matching) break;
        instanceCount += next->m_VisibleInstances.size();
    }
    if (count < 2u) return S_FALSE;
    // An exact prefix uses one lighting bundle regardless of its batch count.
    // Let the bank include a following compatible variant in the same draw.
    if (CGameInstance::Get().Get_RenderOptimizationSettings().LightingBankEnabled &&
        count < objects.size() && drawnMeshes == m_pModelCom->Get_NumMeshes() &&
        m_pModelCom->Can_BatchStaticLightingWith(*m_pModelCom))
    {
        const auto* next = dynamic_cast<const CMapStaticBatchObject*>(objects[count].get());
        if (next && ready(*next) && next->m_bMirrored == m_bMirrored &&
            next->Get_RenderElapsedTime() == Get_RenderElapsedTime() &&
            SameInstanceProfile(m_RenderProfile, next->m_RenderProfile) &&
            m_pModelCom->Can_BatchStaticLightingWith(*next->m_pModelCom) &&
            next->m_VisibleInstances.size() <= UINT32_MAX / sizeof(VTXMESHINSTANCE) - instanceCount)
        {
            Engine::MESH_SCREEN_LOD_DESC nextView{};
            const bool_t nextLod = camera && next->m_RenderProfile.opacity >= 1.f &&
                next->Build_ScreenLodView(*camera, nextView);
            bool_t extendsLightingBank = true;
            for (uint32_t mesh = 0u; mesh < m_pModelCom->Get_NumMeshes(); ++mesh)
            {
                if (next->Is_ChunkMeshClaimed(mesh) ||
                    m_pModelCom->Get_StaticMeshLodLevel(mesh, Uses_MeshScreenLod(mesh, firstLod) ? &firstView : nullptr) !=
                    next->m_pModelCom->Get_StaticMeshLodLevel(mesh, next->Uses_MeshScreenLod(mesh, nextLod) ? &nextView : nullptr))
                { extendsLightingBank = false; break; }
            }
            if (extendsLightingBank) return S_FALSE;
        }
    }
    try
    {
        m_CandidateLightingBankInstances.clear();
        m_CandidateLightingBankInstances.reserve(instanceCount);
        for (size_t i = 0u; i < count; ++i)
        {
            const auto& instances = static_cast<const CMapStaticBatchObject*>(objects[i].get())->m_VisibleInstances;
            m_CandidateLightingBankInstances.insert(m_CandidateLightingBankInstances.end(), instances.begin(), instances.end());
        }
        if (FAILED(Upload_LightingBankInstances())) return S_FALSE;
    }
    catch (const std::bad_alloc&) { return S_FALSE; }
    catch (const std::length_error&) { return S_FALSE; }
    const uint32_t noBank = 0u;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_MapLightingBankSize", &noBank, sizeof(noBank)))) return S_FALSE;
    auto* profiler = CGameInstance::Get().Get_Profiler();
    Engine::CProfilerWorkScope renderWork(profiler, Engine::EProfilerWork::MapBatchRender);
    const HRESULT result = Render_VisibleInstances(m_pLightingBankInstanceBuffer.Get(),
        static_cast<uint32_t>(instanceCount), camera, "cpu_identical_merge", objects.first(count));
    // Once submission starts, a failure must not draw a successful prefix twice.
    if (FAILED(result)) return result;
    consumed = count;
    if (profiler)
    {
        profiler->Add_Counter(Engine::EProfilerCounter::MapBatchVisibleRenders);
        profiler->Add_Counter(Engine::EProfilerCounter::MapIdenticalInstanceSourceDraws, count * drawnMeshes);
        profiler->Add_Counter(Engine::EProfilerCounter::MapIdenticalInstanceDraws, drawnMeshes);
    }
    return result;
}

HRESULT CMapStaticBatchObject::Upload_LightingBankInstances()
{
    const size_t required = m_CandidateLightingBankInstances.size();
    if (required == 0u || required > UINT32_MAX / sizeof(VTXMESHINSTANCE))
        return E_INVALIDARG;
    const bool_t samePayload = required == m_LightingBankInstances.size() &&
        0 == std::memcmp(m_CandidateLightingBankInstances.data(), m_LightingBankInstances.data(),
            required * sizeof(VTXMESHINSTANCE));
    if (samePayload && m_pLightingBankInstanceBuffer)
        return S_OK;

    ComPtr<ID3D11Buffer> target = m_pLightingBankInstanceBuffer;
    uint32_t capacity = m_iLightingBankInstanceCapacity;
    if (!target || required > capacity)
    {
        capacity = 1u;
        const uint32_t maximum = UINT32_MAX / sizeof(VTXMESHINSTANCE);
        while (capacity < required && capacity <= maximum / 2u) capacity *= 2u;
        if (capacity < required) capacity = static_cast<uint32_t>(required);
        D3D11_BUFFER_DESC desc{};
        desc.ByteWidth = capacity * sizeof(VTXMESHINSTANCE);
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        if (FAILED(m_pDevice->CreateBuffer(&desc, nullptr, target.ReleaseAndGetAddressOf())))
            return E_FAIL;
    }
    D3D11_MAPPED_SUBRESOURCE mapped{};
    if (FAILED(m_pContext->Map(target.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mapped)))
        return E_FAIL;
    std::memcpy(mapped.pData, m_CandidateLightingBankInstances.data(), required * sizeof(VTXMESHINSTANCE));
    m_pContext->Unmap(target.Get(), 0u);
    m_pLightingBankInstanceBuffer = std::move(target);
    m_iLightingBankInstanceCapacity = capacity;
    m_LightingBankInstances.swap(m_CandidateLightingBankInstances);
    if (auto* profiler = CGameInstance::Get().Get_Profiler())
        profiler->Add_Counter(Engine::EProfilerCounter::MapBatchUploadBytes,
            required * sizeof(VTXMESHINSTANCE));
    return S_OK;
}

HRESULT CMapStaticBatchObject::Render()
{
    if (auto scene = m_GpuScene.lock())
    {
        const HRESULT gpu = scene->Try_Render(*this);
        if (gpu != S_FALSE) return FAILED(gpu) ? gpu : Render_ColorProxy();
    }
    // 전 mesh의 HLOD 소유권이 확정되면 CPU VB 준비도 필요 없다. 위 GPU
    // callback은 먼저 실행해 같은 cohort의 다른 원본 member 제출을 보존한다.
    if (Are_AllChunkMeshesClaimed()) return Render_ColorProxy();
	Engine::CProfiler* const profiler = CGameInstance::Get().Get_Profiler();
	Engine::CProfilerWorkScope renderWork(profiler, Engine::EProfilerWork::MapBatchRender);
	const uint32_t noBank = 0u;
	if (!m_pShaderCom || FAILED(m_pShaderCom->Bind_RawValue("g_MapLightingBankSize", &noBank, sizeof(noBank))))
		return E_FAIL;
	if (CGameInstance::Get().Is_SceneEnvironmentReplaced())
		return S_OK;

	const MAP_CAMERA_CULL_SNAPSHOT* cameraSnapshot =
		CMapAssetRenderUtils::Capture_CameraCullSnapshotView();
	if (FAILED(Prepare_FinalCameraVisibility(cameraSnapshot)))
	{
		return E_FAIL;
	}
	if (m_VisibleInstances.empty())
	{
		if (profiler) profiler->Add_Counter(Engine::EProfilerCounter::MapBatchEmptyRenders);
		return Render_ColorProxy();
	}
	if (profiler) profiler->Add_Counter(Engine::EProfilerCounter::MapBatchVisibleRenders);

    const HRESULT sourceResult = Render_VisibleInstances(m_pInstanceBuffer.Get(),
        static_cast<uint32_t>(m_VisibleInstances.size()), cameraSnapshot);
    return FAILED(sourceResult) ? sourceResult : Render_ColorProxy();
}

// CMesh가 실제 제출한 직후만 호출한다. 전달된 LOD view는 draw와 동일하며
// 기본 프레임에는 null 포인터 확인만 수행한다. 값 복사/할당 실패는 진단 손실로 남긴다.
void CMapStaticBatchObject::Record_GeometryDiagnosticDraw(const char* path, uint32_t mesh,
    uint32_t instances, const Engine::MESH_SCREEN_LOD_DESC* lod,
    std::span<const std::shared_ptr<CGameObject>> sources, uint32_t firstVisibleInstance) const noexcept
{
    if (!m_FrameState || !m_FrameState->geometryDiagnostics) return;
    auto& diagnostics = *m_FrameState->geometryDiagnostics;
    try
    {
        MAP_GEOMETRY_DIAGNOSTIC_DRAW draw;
        draw.path = path; draw.assetId = m_AssetId; draw.meshIndex = mesh;
        draw.lod = m_pModelCom->Get_StaticMeshLodLevel(mesh, lod);
        draw.indexCount = m_pModelCom->Get_StaticMeshSelectedIndexCount(mesh, lod);
        draw.instanceCount = instances;
        draw.vertexBufferCount = m_pModelCom->Get_MeshVertexCount(mesh);
        // 기존 visible index가 보존된 draw만 거리 source와 연결한다. 추적하지 않은
        // 배치에서 전체 authored placement를 채우면 실제 생존 ID로 오해되므로 unknown이다.
        const auto append = [&](const CMapStaticBatchObject& batch, size_t first, size_t count)
        {
            if (batch.m_VisibleInstanceIndices.size() != batch.m_VisibleInstances.size() ||
                first > batch.m_VisibleInstanceIndices.size() ||
                count > batch.m_VisibleInstanceIndices.size() - first) return false;
            for (size_t i = first; i < first + count; ++i)
            {
                const uint32_t index = batch.m_VisibleInstanceIndices[i];
                if (index >= batch.m_Instances.size()) return false;
                draw.sourcePlacementIds.push_back(batch.m_Instances[index].PlacementId);
            }
            return true;
        };
        bool known = true;
        if (sources.empty())
            known = append(*this, firstVisibleInstance == UINT32_MAX ? 0u : firstVisibleInstance, instances);
        else
            for (const auto& source : sources)
            {
                const auto* batch = dynamic_cast<const CMapStaticBatchObject*>(source.get());
                if (!batch || !append(*batch, 0u, batch->m_VisibleInstances.size())) { known = false; break; }
            }
        draw.sourceIdsKnown = known && draw.sourcePlacementIds.size() == instances;
        if (!draw.sourceIdsKnown) draw.sourcePlacementIds.clear();
        diagnostics.Record(std::move(draw));
    }
    catch (...) { ++diagnostics.droppedDraws; }
}

bool_t CMapStaticBatchObject::Uses_MeshScreenLod(uint32_t meshIndex, bool_t hasScreenLod) const
{
    return hasScreenLod && CGameInstance::Get().Get_MaterialRenderSettings().bUseSourceMaterials &&
        m_pModelCom->Can_UseStaticMeshLod(meshIndex);
}

HRESULT CMapStaticBatchObject::Render_VisibleInstances(ID3D11Buffer* buffer, uint32_t instanceCount,
    const MAP_CAMERA_CULL_SNAPSHOT* cameraSnapshot, const char* diagnosticPath,
    std::span<const std::shared_ptr<CGameObject>> diagnosticSources)
{
    Engine::CProfiler* const profiler = CGameInstance::Get().Get_Profiler();
    const bool instancing = CGameInstance::Get().Get_RenderOptimizationSettings().MapInstancingEnabled;
    const bool_t hasCameraSnapshot = cameraSnapshot != nullptr;
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
        if (Is_ChunkMeshClaimed(meshIndex)) continue;
		const auto* surface = m_pModelCom->Get_MaterialSurface(meshIndex);
        // Material variants share CMesh geometry: re-admit the current draw's
        // material instead of inheriting the source model's LOD eligibility.
        const bool_t useMeshLod = Uses_MeshScreenLod(meshIndex, hasScreenLod);
		const uint32_t meshPass = useSourceMaterials && surface &&
			surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED ?
			24u + passIndex : passIndex;
		{
			Engine::CProfilerDetailScope scope(CGameInstance::Get().Get_Profiler(), "Map.Batch.Material.Bind");
			Engine::CProfilerWorkScope work(profiler, Engine::EProfilerWork::MapBatchMaterial);
			if (FAILED(CMapAssetRenderUtils::Bind_Material(m_pModelCom, m_pShaderCom,
				meshIndex, m_RenderProfile, Get_RenderElapsedTime(), nullptr, m_AssetId, nullptr, nullptr,
				MAP_MATERIAL_BINDING_MODE::INSTANCED))) return E_FAIL;
		}
		{
			Engine::CProfilerDetailScope scope(CGameInstance::Get().Get_Profiler(), "Map.Batch.Pass.Apply");
			Engine::CProfilerWorkScope work(profiler, Engine::EProfilerWork::MapBatchPass);
			if (FAILED(m_pShaderCom->Begin(meshPass))) return E_FAIL;
		}
        // Keep the exact payload, mesh, material and LOD. The baseline removes
        // instance aggregation only; it does not invent a second object runtime.
        const uint32_t draws = instancing ? 1u : instanceCount;
        for (uint32_t instance = 0u; instance < draws; ++instance)
        {
            Engine::CProfilerDetailScope scope(profiler, "Map.Batch.Mesh.Submit");
            Engine::CProfilerWorkScope work(profiler, Engine::EProfilerWork::MapBatchDraw);
            if (FAILED(m_pModelCom->Render_Instanced(meshIndex, buffer,
                sizeof(VTXMESHINSTANCE), instancing ? instanceCount : 1u,
                instancing ? 0u : instance * sizeof(VTXMESHINSTANCE),
                useMeshLod ? &screenLod : nullptr))) return E_FAIL;
            Record_GeometryDiagnosticDraw(diagnosticPath, meshIndex,
                instancing ? instanceCount : 1u, useMeshLod ? &screenLod : nullptr,
                diagnosticSources, instancing ? UINT32_MAX : instance);
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
    const bool instancing = CGameInstance::Get().Get_RenderOptimizationSettings().MapInstancingEnabled;
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
				iMesh, m_RenderProfile, Get_RenderElapsedTime());
			if (FAILED(result)) return fail("BindMaterial", result, iMesh, shadowPassBase + iCullPass);
		}
		{
			Engine::CProfilerDetailScope scope(CGameInstance::Get().Get_Profiler(), "Map.Shadow.Pass.Apply");
			result = m_pShaderCom->Begin(shadowPassBase + iCullPass);
			if (FAILED(result)) return fail("ApplyPass", result, iMesh, shadowPassBase + iCullPass);
		}
        const uint32_t draws = instancing ? 1u : iInstanceCount;
        for (uint32_t instance = 0u; instance < draws; ++instance)
        {
            Engine::CProfilerDetailScope scope(CGameInstance::Get().Get_Profiler(), "Map.Shadow.Mesh.Submit");
            result = m_pModelCom->Render_Instanced(iMesh, m_pShadowInstanceBuffer.Get(),
                sizeof(VTXMESHINSTANCE), instancing ? iInstanceCount : 1u,
                instancing ? 0u : instance * sizeof(VTXMESHINSTANCE));
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

#ifdef _DEBUG
bool_t CMapStaticBatchObject::Try_PickInspectionSurface(
    const float3_t& rayOrigin, const float3_t& rayDirection,
    const f32_t maxDistance, f32_t& outDistance,
    uint64_t& outPlacementId, uint32_t& outMeshIndex, std::string& outMaterialName) const
{
    if (!m_pModelCom || m_RenderProfile.opacity <= 0.f) return false;
    const vector_t origin = XMLoadFloat3(&rayOrigin);
    const vector_t direction = XMLoadFloat3(&rayDirection);
    if (!m_bBatchBoundsDirty && m_bHasBatchBounds)
    {
        const BoundingBox bounds(float3_t(m_BatchBounds.x, m_BatchBounds.y, m_BatchBounds.z),
            float3_t(m_BatchBounds.w, m_BatchBounds.w, m_BatchBounds.w));
        f32_t entry = 0.f;
        if (!bounds.Intersects(origin, direction, entry) || entry > maxDistance) return false;
    }
    const uint32_t cull = CMapAssetRenderUtils::Select_Pass(m_RenderProfile, m_bMirrored) % 3u;
    const auto mode = cull == 0u ? CModel::PICK_CULL_MODE::BACK :
        cull == 1u ? CModel::PICK_CULL_MODE::FRONT : CModel::PICK_CULL_MODE::NONE;
    f32_t nearest = maxDistance;
    uint64_t nearestPlacement = 0u;
    uint32_t nearestMesh = 0u;
    bool_t hit = false;
    for (const auto& instance : m_Instances)
    {
        if (!instance.Visible || instance.Suppressed || instance.CameraPreviewSuppressed) continue;
        const BoundingBox bounds(instance.WorldBoundsCenter, float3_t(
            instance.WorldBoundsRadius, instance.WorldBoundsRadius, instance.WorldBoundsRadius));
        f32_t entry = 0.f;
        if (!bounds.Intersects(origin, direction, entry) || entry > nearest) continue;
        for (uint32_t mesh = 0u; mesh < m_pModelCom->Get_NumMeshes(); ++mesh)
        {
            f32_t distance = nearest;
            if (m_pModelCom->Try_PickStaticSurface(mesh, instance.World, rayOrigin, rayDirection,
                nearest, mode, distance) && distance < nearest)
            { nearest = distance; nearestPlacement = instance.PlacementId; nearestMesh = mesh; hit = true; }
        }
    }
    if (!hit) return false;
    outDistance = nearest;
    outPlacementId = nearestPlacement;
    outMeshIndex = nearestMesh;
    outMaterialName = m_pModelCom->Get_MaterialName(nearestMesh);
    return true;
}
#endif

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

    Reset_ColorProxySelection();
    m_ColorProxy.reset();
    Invalidate_ChunkClaims();
	FMapStaticInstance& current = m_Instances[iter->second];
    // active bit만 바뀌는 visible/suppress는 resident 자료를 재생성하지 않는다.
    // TRS가 움직이는 cohort는 CPU로 복귀하여 매프레임 전 장면 업로드를 피한다.
    const bool transformChanged = 0 != std::memcmp(&current.World, &instance.World, sizeof(current.World));
    const bool payloadChanged = transformChanged ||
        0 != std::memcmp(&current.WorldInvTranspose, &instance.WorldInvTranspose, sizeof(current.WorldInvTranspose)) ||
        0 != std::memcmp(&current.WorldBoundsCenter, &instance.WorldBoundsCenter, sizeof(current.WorldBoundsCenter)) ||
        current.WorldBoundsRadius != instance.WorldBoundsRadius ||
        0 != std::memcmp(&current.BakedLighting.scaleBias, &instance.BakedLighting.scaleBias, sizeof(float4_t)) ||
        0 != std::memcmp(&current.BakedLighting.averageScale, &instance.BakedLighting.averageScale, sizeof(float4_t)) ||
        0 != std::memcmp(&current.BakedLighting.directionalScale, &instance.BakedLighting.directionalScale, sizeof(float4_t)) ||
        0 != std::memcmp(&current.BakedLighting.shadowScaleBias, &instance.BakedLighting.shadowScaleBias, sizeof(float4_t)) ||
        0 != std::memcmp(&current.SourceWind.actorPositionSourceCm, &instance.SourceWind.actorPositionSourceCm, sizeof(float4_t)) ||
        0 != std::memcmp(&current.SourceWind.objectDimensionsAndRadiusSourceCm, &instance.SourceWind.objectDimensionsAndRadiusSourceCm, sizeof(float4_t));
    if (transformChanged) m_bGpuMutableTransform = true;
    if (payloadChanged && m_bGpuMergedVisibility)
        if (auto scene = m_GpuScene.lock()) scene->Invalidate();
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
    current.ColorProxySuppressed = false;
	current.Suppressed = suppressed;
	current.CameraPreviewSuppressed = cameraPreviewSuppressed;
    m_InstanceLinearScaleBounds[iter->second] = LinearScaleBound(current.World);
    current.DistanceHidden = false; current.DistanceSettingsRevision = 0u;
    Rebuild_InstanceOcclusionBounds(current);
    m_bBatchBoundsDirty = true;
	m_bShadowInstancesDirty = true;
	m_bVisibleInstancesDirty = true;
	m_bFinalCameraPrepared = false;
    Invalidate_VisibilityCpu();
    Invalidate_FinalCameraSpatialBounds();
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
        Reset_ColorProxySelection();
        Invalidate_ChunkSelection();
        Invalidate_FinalCameraSpatialBounds();
		if (visible)
			++m_iAuthoredVisibleInstanceCount;
		else
			--m_iAuthoredVisibleInstanceCount;
		instance.Visible = visible;
        instance.DistanceHidden = false; instance.DistanceSettingsRevision = 0u;
		if (m_iStaticShadowRevision != 0u)
			++m_iStaticShadowRevision;
        m_bBatchBoundsDirty = true;
		m_bShadowInstancesDirty = true;
		m_bVisibleInstancesDirty = true;
		m_bFinalCameraPrepared = false;
    Invalidate_VisibilityCpu();
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
        Reset_ColorProxySelection();
        Invalidate_ChunkSelection();
        Invalidate_FinalCameraSpatialBounds();
		instance.Suppressed = suppressed;
        instance.DistanceHidden = false; instance.DistanceSettingsRevision = 0u;
		// Batch bounds stay conservative (they still cover this instance); only
		// the draw and shadow payloads and the cached static shadow change.
		if (m_iStaticShadowRevision != 0u)
			++m_iStaticShadowRevision;
		m_bShadowInstancesDirty = true;
		m_bVisibleInstancesDirty = true;
		m_bFinalCameraPrepared = false;
    Invalidate_VisibilityCpu();
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
        Reset_ColorProxySelection();
        Invalidate_ChunkSelection();
        Invalidate_FinalCameraSpatialBounds();
		instance.CameraPreviewSuppressed = suppressed;
        instance.DistanceHidden = false; instance.DistanceSettingsRevision = 0u;
		if (m_iStaticShadowRevision != 0u)
			++m_iStaticShadowRevision;
		m_bShadowInstancesDirty = true;
		m_bVisibleInstancesDirty = true;
		m_bFinalCameraPrepared = false;
    Invalidate_VisibilityCpu();
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

    // Distance is a separate presentation policy: static masked foliage/grass
    // remain eligible. Semantic landscape/background and gameplay models do not.
    m_SceneVisibilityKind = CMapAssetRenderUtils::Classify_SceneVisibility(*m_pModelCom, m_RenderProfile, m_AssetGroupId);
    m_bDistanceEligible = m_FrameState && (m_SceneVisibilityKind == MAP_SCENE_VISIBILITY_KIND::STATIC_SURFACE ||
        m_SceneVisibilityKind == MAP_SCENE_VISIBILITY_KIND::VEGETATION);
    m_DistanceSourceIndices = 0u;
    m_bOcclusionGeometryEligible = m_FrameState && m_pModelCom->Has_LocalBounds();
    for (uint32_t mesh = 0u; mesh < m_pModelCom->Get_NumMeshes(); ++mesh)
    {
        m_DistanceSourceIndices += m_pModelCom->Get_MeshIndexCount(mesh);
        m_bOcclusionGeometryEligible &= StaticOcclusionBoundsMesh(*m_pModelCom, mesh);
        const auto* surface = m_pModelCom->Get_MaterialSurface(mesh);
        if (surface && (surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_LANDSCAPE_OPAQUE ||
            surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_CHARACTER ||
            (surface->renderMode != Engine::MODEL_SURFACE_RENDER_MODE::INHERIT &&
             surface->renderMode != Engine::MODEL_SURFACE_RENDER_MODE::DEFERRED))) m_bDistanceEligible = false;
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
    if (m_bTrackProxySources)
    {
        m_VisibleInstanceIndices.reserve(newCapacity);
        m_CandidateVisibleInstanceIndices.reserve(newCapacity);
    }

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

void CMapStaticBatchObject::Invalidate_VisibilityCpu()
{
    if (m_CpuVisibility) m_CpuVisibility->State = CPU_VISIBILITY_PREPARATION::STATE::EMPTY;
}

HRESULT CMapStaticBatchObject::Stage_VisibilityCpu(const MAP_CAMERA_CULL_SNAPSHOT* cameraSnapshot)
{
    auto& game = CGameInstance::Get();
    const auto settings = game.Get_MapVisibilitySettings();
    const auto viewport = game.Get_ViewportSize();
    const bool hasCamera = cameraSnapshot != nullptr;
    const uint64_t cameraRevision = hasCamera ? cameraSnapshot->revision : 0u;
    const float4_t* position = hasCamera ? game.Get_CamPosition() : nullptr;
    const bool distanceEnabled = m_bDistanceEligible && position && settings.DistanceEnabled &&
        !m_FrustumCulling.bypass && !m_FrustumCulling.diagnostics &&
        !CMapAssetRenderUtils::Is_SurfaceBindingCollectionActive();
    const uint64_t frame = m_FrameState ? m_FrameState->frameNumber : 0u;
    const bool alreadyPrepared = m_bFinalCameraPrepared && (!m_FrameState || m_iPreparedFrame == frame);
    if ((alreadyPrepared || !m_bVisibleInstancesDirty) &&
        m_bVisibleInstancesUsedCamera == hasCamera && m_iVisibleCameraRevision == cameraRevision &&
        m_iVisibilitySettingsRevision == settings.Revision && m_bVisibleDistanceEnabled == distanceEnabled &&
        m_VisibleViewportSize.x == viewport.x && m_VisibleViewportSize.y == viewport.y)
    {
        if (!alreadyPrepared)
        {
            m_bFinalCameraPrepared = true;
            m_iPreparedFrame = m_FrameState ? m_FrameState->frameNumber : 0u;
            if (auto* profiler = game.Get_Profiler())
            {
                profiler->Add_Counter(Engine::EProfilerCounter::MapBatchVisibilityCacheHits);
                profiler->Add_Counter(Engine::EProfilerCounter::MapDistanceRejectedInstances, m_DistanceRejectedInstances);
                profiler->Add_Counter(Engine::EProfilerCounter::MapDistanceRejectedIndices, m_DistanceRejectedInstances * m_DistanceSourceIndices);
                profiler->Add_Counter(Engine::EProfilerCounter::MapVisibleInstances, m_VisibleInstances.size());
            }
        }
        return S_FALSE;
    }

    if (m_CpuVisibility)
    {
        const auto& previous = *m_CpuVisibility;
        if (previous.State != CPU_VISIBILITY_PREPARATION::STATE::EMPTY && previous.Frame == frame &&
            previous.HasCamera == hasCamera && previous.CameraRevision == cameraRevision &&
            previous.Settings.Revision == settings.Revision && previous.DistanceEnabled == distanceEnabled &&
            previous.Viewport.x == viewport.x && previous.Viewport.y == viewport.y)
            return previous.State == CPU_VISIBILITY_PREPARATION::STATE::FAILED ? previous.Result : S_OK;
    }
    // Most shadow candidates are outside the camera. Resolve their cheap broad
    // rejection before allocating/copying a detailed CPU job for every tiny batch.
    if (m_bBatchBoundsDirty) Rebuild_BatchCullBounds();
    MAP_FRUSTUM_RUNTIME_STATE batchFrustum = m_BatchFrustumState;
    bool requiresNextTick = false;
    if (hasCamera && m_bHasBatchBounds && !m_FrustumCulling.bypass)
    {
        MAP_FRUSTUM_CULL_DECISION decision{};
        MAP_FRUSTUM_CULLING_POLICY policy = m_FrustumCulling;
        policy.diagnostics = false;
        const float3_t center(m_BatchBounds.x, m_BatchBounds.y, m_BatchBounds.z);
        if (CMapAssetRenderUtils::Evaluate_FrustumVisibility(policy, *cameraSnapshot,
            m_AssetId, m_AssetGroupId, 0u, center, m_BatchBounds.w, batchFrustum,
            decision, nullptr, MAP_FRUSTUM_CULL_DETAIL::VISIBILITY_ONLY))
        {
            requiresNextTick = !decision.wouldBeVisible && decision.shouldRender;
            if (!decision.shouldRender && !m_FrustumCulling.diagnostics)
            {
                // Empty payload has no Map failure point. Commit only the broad
                // state; per-instance grace and distance state were not evaluated.
                m_BatchFrustumState = batchFrustum;
                m_VisibleInstances.clear(); m_VisibleInstanceIndices.clear();
                m_VisibleLodBounds = {}; m_VisibleTightLodBounds = {}; m_fVisibleLodScale = 0.f;
                m_bVisibleOcclusionBounds = false;
                m_VisibleOcclusionMin = {FLT_MAX, FLT_MAX, FLT_MAX};
                m_VisibleOcclusionMax = {-FLT_MAX, -FLT_MAX, -FLT_MAX};
                m_bVisibleInstancesDirty = false; m_bVisibleInstancesUsedCamera = hasCamera;
                m_iVisibleCameraRevision = cameraRevision; m_iVisibilitySettingsRevision = settings.Revision;
                m_VisibleViewportSize = viewport; m_bVisibleDistanceEnabled = distanceEnabled;
                m_DistanceRejectedInstances = 0u;
                m_bFinalCameraPrepared = true; m_iPreparedFrame = frame;
                Invalidate_VisibilityCpu();
                if (auto* profiler = game.Get_Profiler())
                {
                    profiler->Add_Counter(Engine::EProfilerCounter::MapBatchVisibilityRebuilds);
                    profiler->Add_Counter(Engine::EProfilerCounter::MapBatchBoundsRejected);
                }
                return HRESULT(2); // Successful new empty commit, not a cache hit.
            }
        }
    }
    try
    {
        if (!m_CpuVisibility) m_CpuVisibility = std::make_unique<CPU_VISIBILITY_PREPARATION>();
        auto& prepared = *m_CpuVisibility;
        prepared.State = CPU_VISIBILITY_PREPARATION::STATE::EMPTY;
        // No allocation, global capture, resource access or bounds rebuild remains in Execute.
        m_CandidateVisibleInstances.reserve(m_Instances.size());
        m_CandidateDistanceChanges.reserve(m_Instances.size());
        if (m_bTrackProxySources) m_CandidateVisibleInstanceIndices.reserve(m_Instances.size());
        prepared.FrustumStates.resize(m_Instances.size());
        prepared.BatchFrustum = batchFrustum; prepared.RequiresNextTick = requiresNextTick;
        prepared.Frame = frame; prepared.HasCamera = hasCamera;
        prepared.Camera = cameraSnapshot; prepared.CameraRevision = cameraRevision;
        prepared.Settings = settings; prepared.Viewport = viewport;
        prepared.CameraPosition = position ? *position : float4_t{};
        prepared.DistanceEnabled = distanceEnabled;
        prepared.CountersCommitted = false; prepared.Result = S_OK;
        prepared.State = CPU_VISIBILITY_PREPARATION::STATE::STAGED;
        return S_OK;
    }
    catch (const std::bad_alloc&) { return E_OUTOFMEMORY; }
    catch (...) { return E_FAIL; }
}

bool_t CMapStaticBatchObject::Try_PrepareFinalCameraCpuJob(FINAL_CAMERA_CPU_JOB& output)
{
    output = {};
    Prepare_ColorProxySelection(CMapAssetRenderUtils::Capture_CameraCullSnapshotView());
    if (Is_GpuVisibilityActive()) return false;
    auto& game = CGameInstance::Get();
    if (!m_FrameState || !game.Get_MapVisibilitySettings().ParallelPreparationEnabled ||
        game.Is_SceneEnvironmentReplaced() || m_iAuthoredVisibleInstanceCount == 0u ||
        m_FrustumCulling.diagnostics || m_FrustumCulling.bypass || Are_AllChunkMeshesClaimed())
        return false;
    if (Stage_VisibilityCpu(CMapAssetRenderUtils::Capture_CameraCullSnapshotView()) != S_OK ||
        !m_CpuVisibility || m_CpuVisibility->State != CPU_VISIBILITY_PREPARATION::STATE::STAGED)
        return false;
    output.Context = this; output.Execute = &Execute_VisibilityCpu;
    output.Cost = static_cast<uint32_t>((std::min)(m_Instances.size(), size_t(UINT32_MAX)));
    return output.Cost != 0u;
}

void CMapStaticBatchObject::Execute_VisibilityCpu(void* context) noexcept
{
    auto& batch = *static_cast<CMapStaticBatchObject*>(context);
    auto& prepared = *batch.m_CpuVisibility;
    if (prepared.State != CPU_VISIBILITY_PREPARATION::STATE::STAGED) return;
    try
    {
        batch.Compute_VisibilityCpu();
        prepared.Camera = nullptr;
        prepared.State = CPU_VISIBILITY_PREPARATION::STATE::READY;
    }
    catch (const std::bad_alloc&)
    {
        prepared.Camera = nullptr;
        prepared.Result = E_OUTOFMEMORY; prepared.State = CPU_VISIBILITY_PREPARATION::STATE::FAILED;
    }
    catch (...)
    {
        prepared.Camera = nullptr;
        prepared.Result = E_FAIL; prepared.State = CPU_VISIBILITY_PREPARATION::STATE::FAILED;
    }
}

void CMapStaticBatchObject::Compute_VisibilityCpu()
{
    auto& prepared = *m_CpuVisibility;
    const bool hasCameraSnapshot = prepared.HasCamera;
    const auto* cameraSnapshot = prepared.Camera;
    const auto& visibilitySettings = prepared.Settings;
    const auto& distanceViewport = prepared.Viewport;
    for (size_t i = 0u; i < m_Instances.size(); ++i)
        prepared.FrustumStates[i] = m_Instances[i].FrustumState;
	m_CandidateVisibleInstances.clear();
    m_CandidateVisibleInstanceIndices.clear();
    m_CandidateDistanceChanges.clear();
    bool candidateOcclusionBounds = m_bOcclusionGeometryEligible;
    float3_t candidateOcclusionMin{ FLT_MAX, FLT_MAX, FLT_MAX }, candidateOcclusionMax{ -FLT_MAX, -FLT_MAX, -FLT_MAX };
	bool_t requiresNextCameraTick = prepared.RequiresNextTick;
    INSTANCE_ENVELOPE visibleEnvelope;
    VIEW_LOD_ENVELOPE visibleViewEnvelope(m_bHasStaticMeshLod ? cameraSnapshot : nullptr);
    uint64_t cullingCandidates = 0u, distanceTested = 0u, distanceRejected = 0u;
    const float4_t* distanceCamera = &prepared.CameraPosition;
    const bool distanceEnabled = prepared.DistanceEnabled;
	for (size_t index = 0u; index < m_Instances.size(); ++index)
	{
        const FMapStaticInstance& instance = m_Instances[index];
		if (!instance.Visible || instance.Suppressed ||
			instance.CameraPreviewSuppressed || instance.ColorProxySuppressed)
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
				prepared.FrustumStates[index],
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

        bool distanceHidden = false;
        if (distanceEnabled)
        {
            float limit = 0.f, maximumPixels = 0.f;
            if (CMapAssetRenderUtils::Get_SceneDistancePolicy(m_SceneVisibilityKind, instance.WorldBoundsRadius,
                visibilitySettings, limit, maximumPixels))
            {
                ++distanceTested;
                float margin = (std::max)(0.f, m_FrustumCulling.baseMargin);
                if (instance.WorldBoundsRadius >= m_FrustumCulling.largeObjectRadiusThreshold)
                    margin += (std::max)(m_FrustumCulling.largeObjectAbsoluteMargin,
                        instance.WorldBoundsRadius * m_FrustumCulling.largeObjectRelativeMargin);
                // 큰 건물도 공통 정책으로 판정하되 2px보다 큰 외곽은 남긴다.
                distanceHidden = StaticPropDistanceHidden(instance, *distanceCamera, *cameraSnapshot, distanceViewport,
                    limit, maximumPixels, visibilitySettings.Revision, margin);
            }
        }
        if (distanceHidden != instance.DistanceHidden || (distanceHidden && instance.DistanceSettingsRevision != visibilitySettings.Revision))
            m_CandidateDistanceChanges.emplace_back(static_cast<uint32_t>(index), distanceHidden);
        if (distanceHidden) { ++distanceRejected; continue; }
        if (candidateOcclusionBounds)
        {
            candidateOcclusionBounds = instance.OcclusionBoundsValid;
            if (candidateOcclusionBounds)
            {
                XMStoreFloat3(&candidateOcclusionMin, XMVectorMin(XMLoadFloat3(&candidateOcclusionMin), XMLoadFloat3(&instance.OcclusionBoundsMin)));
                XMStoreFloat3(&candidateOcclusionMax, XMVectorMax(XMLoadFloat3(&candidateOcclusionMax), XMLoadFloat3(&instance.OcclusionBoundsMax)));
            }
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
        gpuInstance.vSourceWindOwnerPosition = instance.SourceWind.actorPositionSourceCm;
        gpuInstance.vSourceWindDimensionsAndRadius = instance.SourceWind.objectDimensionsAndRadiusSourceCm;

		m_CandidateVisibleInstances.push_back(
			gpuInstance);
        if (m_bTrackProxySources)
            m_CandidateVisibleInstanceIndices.push_back(static_cast<uint32_t>(index));
	}

    prepared.RequiresNextTick = requiresNextCameraTick;
    prepared.RejectedBatch = false;
    prepared.HasOcclusionBounds = candidateOcclusionBounds && !m_CandidateVisibleInstances.empty();
    prepared.OcclusionMin = candidateOcclusionMin; prepared.OcclusionMax = candidateOcclusionMax;
    prepared.LodBounds = {}; prepared.TightLodBounds = {};
    const bool hasLodBounds = visibleEnvelope.Store(prepared.LodBounds);
    if (!hasLodBounds) prepared.LodBounds = {};
    if (!hasLodBounds || !visibleViewEnvelope.Store(prepared.TightLodBounds)) prepared.TightLodBounds = {};
    prepared.LodScale = hasLodBounds ? visibleEnvelope.maximumScale : 0.f;
    prepared.CullingCandidates = cullingCandidates;
    prepared.DistanceTested = distanceTested; prepared.DistanceRejected = distanceRejected;
}

HRESULT CMapStaticBatchObject::Upload_VisibleInstances(const MAP_CAMERA_CULL_SNAPSHOT* cameraSnapshot)
{
    auto* profiler = CGameInstance::Get().Get_Profiler();
    Engine::CProfilerWorkScope work(profiler, Engine::EProfilerWork::MapBatchVisibility);
    const HRESULT staged = Stage_VisibilityCpu(cameraSnapshot);
    if (FAILED(staged)) return staged;
    if (staged == HRESULT(2)) return S_OK;
    if (staged == S_FALSE) return S_OK;
    // A Layer allocation/dispatch failure can leave a valid staged job unexecuted.
    if (m_CpuVisibility->State == CPU_VISIBILITY_PREPARATION::STATE::STAGED)
    {
        Engine::CProfilerDetailScope scope(profiler, "Map.Batch.CullAndPack");
        Execute_VisibilityCpu(this);
    }
    auto& prepared = *m_CpuVisibility;
    if (prepared.State != CPU_VISIBILITY_PREPARATION::STATE::READY) return prepared.Result;
    if (!prepared.CountersCommitted)
    {
        prepared.CountersCommitted = true;
        if (profiler)
        {
            profiler->Add_Counter(Engine::EProfilerCounter::MapBatchVisibilityRebuilds);
            profiler->Add_Counter(Engine::EProfilerCounter::MapBatchBoundsRejected, prepared.RejectedBatch ? 1u : 0u);
            profiler->Add_Counter(Engine::EProfilerCounter::MapDistanceTestedInstances, prepared.DistanceTested);
            profiler->Add_Counter(Engine::EProfilerCounter::MapCullingCandidates, prepared.CullingCandidates);
            profiler->Add_Counter(Engine::EProfilerCounter::MapCullingVisible, m_CandidateVisibleInstances.size());
        }
    }
    const bool payloadUnchanged = m_CandidateVisibleInstances.size() == m_VisibleInstances.size() &&
        (m_CandidateVisibleInstances.empty() || 0 == std::memcmp(m_CandidateVisibleInstances.data(),
            m_VisibleInstances.data(), m_CandidateVisibleInstances.size() * sizeof(VTXMESHINSTANCE)));
    if (!payloadUnchanged && !m_CandidateVisibleInstances.empty())
    {
        Engine::CProfilerDetailScope scope(profiler, "Map.Batch.InstanceUpload");
        if (FAILED(Ensure_InstanceCapacity(static_cast<uint32_t>(m_CandidateVisibleInstances.size())))) return E_FAIL;
        D3D11_MAPPED_SUBRESOURCE mapped{};
        if (FAILED(m_pContext->Map(m_pInstanceBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mapped))) return E_FAIL;
        std::memcpy(mapped.pData, m_CandidateVisibleInstances.data(), m_CandidateVisibleInstances.size() * sizeof(VTXMESHINSTANCE));
        m_pContext->Unmap(m_pInstanceBuffer.Get(), 0u);
        if (profiler) profiler->Add_Counter(Engine::EProfilerCounter::MapBatchUploadBytes,
            m_CandidateVisibleInstances.size() * sizeof(VTXMESHINSTANCE));
    }
    // Payload, source identity, bounds and both kinds of hysteresis commit together.
    m_VisibleLodBounds = prepared.LodBounds; m_VisibleTightLodBounds = prepared.TightLodBounds;
    m_fVisibleLodScale = prepared.LodScale;
    m_bVisibleOcclusionBounds = prepared.HasOcclusionBounds;
    m_VisibleOcclusionMin = prepared.OcclusionMin; m_VisibleOcclusionMax = prepared.OcclusionMax;
    Commit_DistanceSelection(prepared.Settings.Revision);
    m_BatchFrustumState = prepared.BatchFrustum;
    for (size_t i = 0u; i < m_Instances.size(); ++i) m_Instances[i].FrustumState = prepared.FrustumStates[i];
    if (!payloadUnchanged) m_VisibleInstances.swap(m_CandidateVisibleInstances);
    m_VisibleInstanceIndices.swap(m_CandidateVisibleInstanceIndices);
    m_bVisibleInstancesDirty = prepared.RequiresNextTick;
    m_bVisibleInstancesUsedCamera = prepared.HasCamera;
    m_iVisibleCameraRevision = prepared.CameraRevision;
    m_iVisibilitySettingsRevision = prepared.Settings.Revision;
    m_VisibleViewportSize = prepared.Viewport;
    m_bVisibleDistanceEnabled = prepared.DistanceEnabled;
    m_DistanceRejectedInstances = prepared.DistanceRejected;
    prepared.State = CPU_VISIBILITY_PREPARATION::STATE::EMPTY;
    if (profiler)
    {
        profiler->Add_Counter(Engine::EProfilerCounter::MapDistanceRejectedInstances, m_DistanceRejectedInstances);
        profiler->Add_Counter(Engine::EProfilerCounter::MapDistanceRejectedIndices, m_DistanceRejectedInstances * m_DistanceSourceIndices);
        profiler->Add_Counter(Engine::EProfilerCounter::MapVisibleInstances, m_VisibleInstances.size());
    }
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
		gpuInstance.vSourceWindOwnerPosition = instance.SourceWind.actorPositionSourceCm;
		gpuInstance.vSourceWindDimensionsAndRadius = instance.SourceWind.objectDimensionsAndRadiusSourceCm;
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
		FMapStaticInstance& instance =
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
        Rebuild_InstanceOcclusionBounds(instance);
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

void CMapStaticBatchObject::Rebuild_BatchCullBounds() const
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
    const auto& v = camera.lodView;
    const float viewScale = camera.lodViewScale;
    if (viewScale <= 0.f) return false;
    const auto viewport = CGameInstance::Get().Get_ViewportSize();
    if (viewport.x <= 0.f || viewport.y <= 0.f) return false;
    const vector_t center = XMVectorSet(m_VisibleLodBounds.x, m_VisibleLodBounds.y, m_VisibleLodBounds.z, 1.f);
    Engine::MESH_SCREEN_LOD_DESC candidate{};
    // Bern의 입장 컷신과 일반 이동에 1pixel 예산을 준다. 선택기는 여기에
    // 10% 여유를 적용하며 다른 맵은 기존0.25pixel 한계로 생성된 단계들을 선택한다.
    if (m_FrameState) candidate.maximumPixelError = 1.f;
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
