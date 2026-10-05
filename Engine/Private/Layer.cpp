#include "Layer.h"

#include "GameInstance.h"
#include "ContainerObject.h"
#include "Profiler.h"
#include "CpuJobPool.h"
#include "Engine_RenderTypes.h"
#include <cmath>
#include <cfloat>
#include <stdexcept>
#include <limits>
#include <cstring>

CLayer::CLayer(uint32_t levelIndex, const wstring_t& layerTag)
{
    std::string tag = "Unnamed";
    const int length = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
        layerTag.data(), static_cast<int>(layerTag.size()), nullptr, 0, nullptr, nullptr);
    if (length > 0)
    {
        tag.resize(static_cast<size_t>(length));
        WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, layerTag.data(),
            static_cast<int>(layerTag.size()), tag.data(), length, nullptr, nullptr);
    }
    const std::string prefix = "Layer.L" + std::to_string(levelIndex) + "." + tag;
    m_ProfileScopeNames = { prefix + ".PriorityUpdate", prefix + ".Update",
        prefix + ".PostPhysicsUpdate", prefix + ".LateUpdate" };
    m_FinalCameraScopeName = prefix + ".FinalCamera";
}

CLayer::~CLayer()
{
    for (auto& member : m_FinalCameraMembers)
        if (member.Object->m_FinalCameraLayer.Owner == this)
        {
            member.Object->m_FinalCameraLayer.Owner = nullptr;
            member.Object->m_FinalCameraLayer.DirtyQueued = false;
        }
}

shared_ptr<CGameObject> CLayer::Get_GameObject(uint32_t iIndex)
{
	if (iIndex >= m_GameObjects.size())
		return nullptr;

	auto iter = m_GameObjects.begin();
	for (uint32_t index = 0; index < iIndex; ++index)
		++iter;

	return iter != m_GameObjects.end() ? *iter : nullptr;
}

shared_ptr<CComponent> CLayer::Get_Component(const wstring_t& strComponentTag, uint32_t iIndex)
{
	if (iIndex >= m_GameObjects.size())
		return nullptr;

	auto	iter = m_GameObjects.begin();

	for (uint32_t i = 0; i < iIndex; i++)
		++iter;

	if(iter == m_GameObjects.end())
		return nullptr;

	return (*iter)->Get_Component(strComponentTag);
}

shared_ptr<CComponent> CLayer::Get_Component(const wstring_t& strPartTag, const wstring_t& strComponentTag, uint32_t iIndex)
{
	if (iIndex >= m_GameObjects.size())
		return nullptr;

	auto	iter = m_GameObjects.begin();

	for (uint32_t i = 0; i < iIndex; i++)
		++iter;

	if (iter == m_GameObjects.end())
		return nullptr;

	return static_pointer_cast<CContainerObject>(*iter)->Get_Component(strPartTag, strComponentTag);
}

HRESULT CLayer::Add_GameObject(shared_ptr<CGameObject> pGameObject)
{
	if (nullptr == pGameObject)
		return E_FAIL;

    if (pGameObject->m_FinalCameraLayer.Owner != nullptr)
        return E_INVALIDARG;
	m_GameObjects.push_back(pGameObject);
	const uint8_t phases = pGameObject->Get_UpdatePhaseMask();
	for (size_t phase = 0; phase < m_PhaseObjects.size(); ++phase)
		if (phases & (1u << phase)) m_PhaseObjects[phase].push_back(pGameObject.get());
	if (pGameObject->Uses_FinalCameraSubmission())
    {
        m_FinalCameraMembers.push_back({ pGameObject.get() });
        pGameObject->m_FinalCameraLayer.Owner = this;
        pGameObject->m_FinalCameraLayer.Index = m_FinalCameraMembers.size() - 1u;
        pGameObject->m_FinalCameraLayer.DirtyQueued = false;
        m_FinalCameraTopologyDirty = true;
    }

	return S_OK;
}

HRESULT CLayer::Remove_GameObject(const shared_ptr<CGameObject>& pGameObject)
{
	if (nullptr == pGameObject)
		return E_FAIL;

	auto iter = find(m_GameObjects.begin(), m_GameObjects.end(), pGameObject);
	if (iter == m_GameObjects.end())
		return E_FAIL;

	for (auto& phase : m_PhaseObjects)
	{
		const auto member = std::find(phase.begin(), phase.end(), pGameObject.get());
		if (member != phase.end()) phase.erase(member);
	}
    if (pGameObject->m_FinalCameraLayer.Owner == this)
    {
        const size_t index = pGameObject->m_FinalCameraLayer.Index;
        m_FinalCameraMembers.erase(m_FinalCameraMembers.begin() + index);
        m_FinalCameraDirty.erase(std::remove(m_FinalCameraDirty.begin(), m_FinalCameraDirty.end(),
            pGameObject.get()), m_FinalCameraDirty.end());
        pGameObject->m_FinalCameraLayer.Owner = nullptr;
        pGameObject->m_FinalCameraLayer.DirtyQueued = false;
        for (size_t i = index; i < m_FinalCameraMembers.size(); ++i)
            m_FinalCameraMembers[i].Object->m_FinalCameraLayer.Index = i;
        m_FinalCameraTopologyDirty = true;
    }
	m_GameObjects.erase(iter);
	return S_OK;
}

void CLayer::Priority_Update(f32_t fTimeDelta)
{
	CProfilerScope profile(CGameInstance::Get().Get_Profiler(), m_ProfileScopeNames[0]);
	for (CGameObject* pGameObject : m_PhaseObjects[0])
	{
		if (nullptr != pGameObject)
			pGameObject->Priority_Update(fTimeDelta);
	}
}

void CLayer::Update(f32_t fTimeDelta)
{
	CProfilerScope profile(CGameInstance::Get().Get_Profiler(), m_ProfileScopeNames[1]);
	for (CGameObject* pGameObject : m_PhaseObjects[1])
	{
		if (nullptr != pGameObject)
			pGameObject->Update(fTimeDelta);
	}
}

void CLayer::Post_Physics_Update(f32_t fTimeDelta)
{
	CProfilerScope profile(CGameInstance::Get().Get_Profiler(), m_ProfileScopeNames[2]);
	for (CGameObject* pGameObject : m_PhaseObjects[2])
	{
		if (nullptr != pGameObject)
			pGameObject->Post_Physics_Update(fTimeDelta);
	}
}

void CLayer::Late_Update(f32_t fTimeDelta)
{
	CProfilerScope profile(CGameInstance::Get().Get_Profiler(), m_ProfileScopeNames[3]);
	for (CGameObject* pGameObject : m_PhaseObjects[3])
	{
		if (nullptr != pGameObject)
			pGameObject->Late_Update(fTimeDelta);
	}
}

namespace
{
    bool ValidSpatialBounds(const CGameObject::FINAL_CAMERA_SPATIAL_BOUNDS& bounds)
    {
        return std::isfinite(bounds.Center.x) && std::isfinite(bounds.Center.y) &&
            std::isfinite(bounds.Center.z) && std::isfinite(bounds.Radius) && bounds.Radius > 0.f;
    }

    bool CaptureLayerPlanes(std::array<float4_t, 6>& output)
    {
        auto& game = CGameInstance::Get();
        const auto* view = game.Get_Transform(D3DTS::VIEW);
        const auto* projection = game.Get_Transform(D3DTS::PROJ);
        if (!view || !projection) return false;
        const matrix_t columns = XMMatrixTranspose(XMLoadFloat4x4(view) * XMLoadFloat4x4(projection));
        const vector_t planes[6] = {
            columns.r[0] - columns.r[3], -columns.r[0] - columns.r[3],
            columns.r[1] - columns.r[3], -columns.r[1] - columns.r[3],
            columns.r[2] - columns.r[3], -columns.r[2]
        };
        for (size_t i = 0u; i < output.size(); ++i)
        {
            const float lengthSquared = XMVectorGetX(XMVector3LengthSq(planes[i]));
            if (!std::isfinite(lengthSquared) || lengthSquared <= 0.f ||
                XMVector4IsNaN(planes[i]) || XMVector4IsInfinite(planes[i])) return false;
            XMStoreFloat4(&output[i], XMPlaneNormalize(planes[i]));
            const auto& p = output[i];
            const double length = double(p.x) * p.x + double(p.y) * p.y + double(p.z) * p.z;
            if (!std::isfinite(p.w) || !std::isfinite(length) ||
                std::abs(length - 1.) > 8. * std::numeric_limits<float>::epsilon()) return false;
        }
        return true;
    }

    bool IntersectsLayerPlanes(const std::array<float4_t, 6>& planes,
        const std::array<double, 3>& minimum, const std::array<double, 3>& maximum,
        uint8_t& remainingPlanes)
    {
        if (remainingPlanes == 0u) return true;
        for (size_t i = 0u; i < planes.size(); ++i)
        {
            const uint8_t bit = static_cast<uint8_t>(1u << i);
            if (!(remainingPlanes & bit)) continue;
            const auto& plane = planes[i];
            const double x = double(plane.x) * (plane.x >= 0.f ? minimum[0] : maximum[0]);
            const double y = double(plane.y) * (plane.y >= 0.f ? minimum[1] : maximum[1]);
            const double z = double(plane.z) * (plane.z >= 0.f ? minimum[2] : maximum[2]);
            const double magnitude = std::abs(x) + std::abs(y) + std::abs(z) + std::abs(double(plane.w));
            // Keep touching/roundoff-scale candidates; the consumer performs exact culling.
            const double tolerance = 32. * std::numeric_limits<float>::epsilon() * (std::max)(1., magnitude);
            if (x + y + z + double(plane.w) > tolerance) return false;

            const double farX = double(plane.x) * (plane.x >= 0.f ? maximum[0] : minimum[0]);
            const double farY = double(plane.y) * (plane.y >= 0.f ? maximum[1] : minimum[1]);
            const double farZ = double(plane.z) * (plane.z >= 0.f ? maximum[2] : minimum[2]);
            const double farMagnitude = std::abs(farX) + std::abs(farY) + std::abs(farZ) + std::abs(double(plane.w));
            const double insideTolerance = 32. * std::numeric_limits<float>::epsilon() * (std::max)(1., farMagnitude);
            // Every descendant is inside this plane too. Require a margin so
            // touching bounds still receive the original conservative test.
            if (farX + farY + farZ + double(plane.w) < -insideTolerance)
                remainingPlanes &= static_cast<uint8_t>(~bit);
        }
        return true;
    }
}

void CLayer::Invalidate_FinalCameraSpatialBounds(CGameObject* object)
{
    if (!object || object->m_FinalCameraLayer.Owner != this || object->m_FinalCameraLayer.DirtyQueued)
        return;
    try
    {
        m_FinalCameraDirty.push_back(object);
        object->m_FinalCameraLayer.DirtyQueued = true;
    }
    catch (const std::bad_alloc&) { m_FinalCameraTopologyDirty = true; }
}

void CLayer::Refit_FinalCameraNode(uint32_t index)
{
    auto& node = m_FinalCameraNodes[index];
    node.RejectedFrames = 0u;
    if (node.Member != SIZE_MAX)
    {
        const auto& bounds = m_FinalCameraMembers[node.Member].Bounds;
        const double center[3] = { bounds.Center.x, bounds.Center.y, bounds.Center.z };
        for (size_t axis = 0; axis < 3; ++axis)
        {
            node.Minimum[axis] = center[axis] - double(bounds.Radius);
            node.Maximum[axis] = center[axis] + double(bounds.Radius);
        }
        node.RejectGraceFrames = bounds.RejectGraceFrames;
        node.ShadowCaster = bounds.ShadowCaster;
        return;
    }
    const auto& left = m_FinalCameraNodes[node.Left];
    const auto& right = m_FinalCameraNodes[node.Right];
    for (size_t axis = 0; axis < 3; ++axis)
    {
        node.Minimum[axis] = (std::min)(left.Minimum[axis], right.Minimum[axis]);
        node.Maximum[axis] = (std::max)(left.Maximum[axis], right.Maximum[axis]);
    }
    node.RejectGraceFrames = (std::max)(left.RejectGraceFrames, right.RejectGraceFrames);
    node.ShadowCaster = left.ShadowCaster || right.ShadowCaster;
}

uint32_t CLayer::Build_FinalCameraNode(size_t first, size_t end, uint32_t parent)
{
    const uint32_t index = static_cast<uint32_t>(m_FinalCameraNodes.size());
    m_FinalCameraNodes.emplace_back();
    m_FinalCameraNodes[index].Parent = parent;
    if (end - first == 1u)
    {
        const size_t member = m_FinalCameraOrder[first];
        m_FinalCameraNodes[index].Member = member;
        m_FinalCameraMembers[member].Leaf = index;
    }
    else
    {
        double low[3] = { DBL_MAX, DBL_MAX, DBL_MAX };
        double high[3] = { -DBL_MAX, -DBL_MAX, -DBL_MAX };
        for (size_t i = first; i < end; ++i)
        {
            const auto& center = m_FinalCameraMembers[m_FinalCameraOrder[i]].Bounds.Center;
            const double values[3] = { center.x, center.y, center.z };
            for (size_t axis = 0; axis < 3; ++axis)
            {
                low[axis] = (std::min)(low[axis], values[axis]);
                high[axis] = (std::max)(high[axis], values[axis]);
            }
        }
        size_t axis = 0u;
        for (size_t i = 1u; i < 3u; ++i)
            if (high[i] - low[i] > high[axis] - low[axis]) axis = i;
        const auto coordinate = [this, axis](size_t member)
        {
            const auto& center = m_FinalCameraMembers[member].Bounds.Center;
            return axis == 0u ? center.x : axis == 1u ? center.y : center.z;
        };
        const size_t middle = first + (end - first) / 2u;
        std::nth_element(m_FinalCameraOrder.begin() + first, m_FinalCameraOrder.begin() + middle,
            m_FinalCameraOrder.begin() + end, [&](size_t a, size_t b)
            { const float x = coordinate(a), y = coordinate(b); return x != y ? x < y : a < b; });
        const uint32_t left = Build_FinalCameraNode(first, middle, index);
        const uint32_t right = Build_FinalCameraNode(middle, end, index);
        m_FinalCameraNodes[index].Left = left;
        m_FinalCameraNodes[index].Right = right;
    }
    m_FinalCameraNodes[index].End = static_cast<uint32_t>(m_FinalCameraNodes.size());
    Refit_FinalCameraNode(index);
    return index;
}

void CLayer::Rebuild_FinalCameraHierarchy()
{
    m_FinalCameraNodes.clear(); m_FinalCameraOrder.clear(); m_FinalCameraAlways.clear();
    m_FinalCameraOrder.reserve(m_FinalCameraMembers.size());
    m_FinalCameraAlways.reserve(m_FinalCameraMembers.size());
    m_FinalCameraCandidates.reserve(m_FinalCameraMembers.size());
    m_FinalCameraCpuCandidates.reserve(m_FinalCameraMembers.size());
    for (size_t i = 0; i < m_FinalCameraMembers.size(); ++i)
    {
        auto& member = m_FinalCameraMembers[i];
        member.Leaf = UINT32_MAX;
        member.Object->m_FinalCameraLayer.DirtyQueued = false;
        member.Bounded = member.Object->Try_GetFinalCameraSpatialBounds(member.Bounds) && ValidSpatialBounds(member.Bounds);
        (member.Bounded ? m_FinalCameraOrder : m_FinalCameraAlways).push_back(i);
    }
    m_FinalCameraDirty.clear();
    if (m_FinalCameraOrder.size() > UINT32_MAX / 2u) throw std::length_error("final-camera hierarchy size");
    m_FinalCameraNodes.reserve(m_FinalCameraOrder.size() * 2u);
    if (!m_FinalCameraOrder.empty()) Build_FinalCameraNode(0u, m_FinalCameraOrder.size(), UINT32_MAX);
    m_FinalCameraTopologyDirty = false;
}

void CLayer::Refresh_FinalCameraHierarchy()
{
    if (m_FinalCameraTopologyDirty) { Rebuild_FinalCameraHierarchy(); return; }
    for (CGameObject* object : m_FinalCameraDirty) object->m_FinalCameraLayer.DirtyQueued = false;
    for (CGameObject* object : m_FinalCameraDirty)
    {
        auto& member = m_FinalCameraMembers[object->m_FinalCameraLayer.Index];
        CGameObject::FINAL_CAMERA_SPATIAL_BOUNDS bounds{};
        const bool bounded = object->Try_GetFinalCameraSpatialBounds(bounds) && ValidSpatialBounds(bounds);
        if (bounded != member.Bounded) { m_FinalCameraTopologyDirty = true; break; }
        member.Bounds = bounds;
        if (bounded)
            for (uint32_t node = member.Leaf; node != UINT32_MAX; node = m_FinalCameraNodes[node].Parent)
                Refit_FinalCameraNode(node);
    }
    m_FinalCameraDirty.clear();
    if (m_FinalCameraTopologyDirty) Rebuild_FinalCameraHierarchy();
}

void CLayer::Submit_FinalCamera()
{
    if (m_FinalCameraMembers.empty()) return;
    auto& game = CGameInstance::Get();
    CProfilerScope profile(game.Get_Profiler(), m_FinalCameraScopeName);
    try
    {
        const bool hierarchyUnchanged = !m_FinalCameraTopologyDirty && m_FinalCameraDirty.empty();
        Refresh_FinalCameraHierarchy();
        std::array<float4_t, 6> planes{};
        const auto optimization = game.Get_RenderOptimizationSettings();
        const bool cameraValid = CaptureLayerPlanes(planes);
        const bool shadowEnabled = game.Is_ShadowLightEnabled();
        const bool reuseCandidates = hierarchyUnchanged && m_FinalCameraCandidatesReusable &&
            optimization.Revision == m_FinalCameraOptimizationRevision &&
            shadowEnabled == m_FinalCameraShadowEnabled &&
            (!optimization.FrustumEnabled || (cameraValid &&
                std::memcmp(planes.data(), m_FinalCameraPlanes.data(), sizeof(planes)) == 0));
        if (!reuseCandidates)
        {
            bool gracePending = false;
            m_FinalCameraCandidatesReusable = false;
            m_FinalCameraCandidates.assign(m_FinalCameraAlways.begin(), m_FinalCameraAlways.end());
            m_FinalCameraCpuCandidates.assign(m_FinalCameraAlways.begin(), m_FinalCameraAlways.end());
            for (uint32_t i = 0u; i < m_FinalCameraNodes.size();)
            {
                auto& node = m_FinalCameraNodes[i];
                node.RemainingPlanes = node.Parent == UINT32_MAX ? uint8_t{ 0x3fu } :
                    m_FinalCameraNodes[node.Parent].RemainingPlanes;
                // A shadow-only subtree still submits every caster, but cannot
                // justify preparing detailed color payloads on worker threads.
                // A camera-outside parent also makes all descendants outside.
                node.CameraIntersects = !optimization.FrustumEnabled || !cameraValid ||
                    ((node.Parent == UINT32_MAX || m_FinalCameraNodes[node.Parent].CameraIntersects) &&
                     IntersectsLayerPlanes(planes, node.Minimum, node.Maximum, node.RemainingPlanes));
                const bool intersects = node.CameraIntersects || (shadowEnabled && node.ShadowCaster);
                if (intersects) node.RejectedFrames = 0u;
                else if (node.RejectedFrames < node.RejectGraceFrames)
                {
                    ++node.RejectedFrames;
                    gracePending = true;
                }
                else { i = node.End; continue; }
                if (node.Member != SIZE_MAX)
                {
                    m_FinalCameraCandidates.push_back(node.Member);
                    if (node.CameraIntersects) m_FinalCameraCpuCandidates.push_back(node.Member);
                }
                ++i;
            }
            // Spatial traversal changes candidate discovery only, never draw order.
            std::sort(m_FinalCameraCandidates.begin(), m_FinalCameraCandidates.end());
            std::sort(m_FinalCameraCpuCandidates.begin(), m_FinalCameraCpuCandidates.end());
            m_FinalCameraPlanes = planes;
            m_FinalCameraShadowEnabled = shadowEnabled;
            // Even an unchanged camera must advance grace until rejection settles.
            m_FinalCameraCandidatesReusable = (!optimization.FrustumEnabled || cameraValid) && !gracePending;
            m_FinalCameraOptimizationRevision = optimization.Revision;
        }
    }
    catch (const std::bad_alloc&)
    {
        m_FinalCameraTopologyDirty = true;
        m_FinalCameraCandidatesReusable = false;
        for (auto& member : m_FinalCameraMembers) member.Object->Submit_FinalCamera();
        return;
    }
    catch (const std::length_error&)
    {
        m_FinalCameraTopologyDirty = true;
        m_FinalCameraCandidatesReusable = false;
        for (auto& member : m_FinalCameraMembers) member.Object->Submit_FinalCamera();
        return;
    }
    Prepare_FinalCameraCpuJobs();
    for (size_t member : m_FinalCameraCandidates)
        m_FinalCameraMembers[member].Object->Submit_FinalCamera();
}


void CLayer::Prepare_FinalCameraCpuJobs()
{
    // Gathering itself costs more than a cached/small visibility pass. Opt in
    // only for measured workloads; OFF keeps the original owner-only path.
    if (!CGameInstance::Get().Get_MapVisibilitySettings().ParallelPreparationEnabled) return;
    auto* profiler = CGameInstance::Get().Get_Profiler();
    CProfilerWorkScope visibilityWork(profiler, EProfilerWork::MapBatchVisibility);
    CProfilerScope prepare(profiler, "Map.Visibility.Prepare");
    m_FinalCameraCpuJobs.clear();
    m_FinalCameraCpuRanges.clear();
    try
    {
        // Allocate before staging any borrowed work. A failed allocation leaves
        // every object on its original owner-thread preparation path.
        m_FinalCameraCpuJobs.reserve(m_FinalCameraCpuCandidates.size());
        m_FinalCameraCpuRanges.reserve(m_FinalCameraCpuCandidates.size() / 8u + 1u);
        uint64_t totalCost = 0u;
        for (const size_t member : m_FinalCameraCpuCandidates)
        {
            CGameObject::FINAL_CAMERA_CPU_JOB job{};
            if (!m_FinalCameraMembers[member].Object->Try_PrepareFinalCameraCpuJob(job) ||
                !job.Context || !job.Execute || !job.Cost) continue;
            m_FinalCameraCpuJobs.push_back(job);
            totalCost += job.Cost;
        }
        if (m_FinalCameraCpuJobs.empty()) return;
        size_t first = 0u;
        uint64_t cost = 0u;
        for (size_t i = 0u; i < m_FinalCameraCpuJobs.size(); ++i)
        {
            cost += m_FinalCameraCpuJobs[i].Cost;
            const size_t count = i + 1u - first;
            if ((cost >= 128u && count >= 8u) || count >= 64u)
            {
                m_FinalCameraCpuRanges.push_back({first, i + 1u});
                first = i + 1u; cost = 0u;
            }
        }
        if (first != m_FinalCameraCpuJobs.size())
            m_FinalCameraCpuRanges.push_back({first, m_FinalCameraCpuJobs.size()});
        const auto execute = [](void* context, size_t index)
        {
            auto& layer = *static_cast<CLayer*>(context);
            const auto range = layer.m_FinalCameraCpuRanges[index];
            for (size_t i = range.First; i < range.End; ++i)
            {
                const auto& job = layer.m_FinalCameraCpuJobs[i];
                job.Execute(job.Context);
            }
        };
        CPU_JOB_STATS stats{};
        {
            CProfilerScope dispatch(profiler, "Map.Visibility.Dispatch");
            stats = Run_CpuJobs(m_FinalCameraCpuRanges.size(), this, execute,
                totalCost >= 512u ? 3u : 0u, profiler, "Map.Visibility.Worker", "Map.Visibility.Join");
        }
        if (profiler)
        {
            profiler->Add_Counter(EProfilerCounter::MapVisibilityPreparedBatches, m_FinalCameraCpuJobs.size());
            profiler->Add_Counter(EProfilerCounter::MapVisibilityCpuJobs, m_FinalCameraCpuRanges.size());
            profiler->Add_Counter(EProfilerCounter::MapVisibilityCallerJobs, stats.CallerJobs);
            profiler->Add_Counter(EProfilerCounter::MapVisibilityWorkerJobs, stats.WorkerJobs);
            profiler->Add_Counter(EProfilerCounter::MapVisibilityAssistants, stats.Assistants);
        }
    }
    catch (...)
    {
        // The synchronous pool has already joined all running callbacks.
        // Objects consume finished staging or prepare unfinished work in Submit.
    }
}

shared_ptr<CLayer> CLayer::Create(uint32_t levelIndex, const wstring_t& layerTag)
{
	return shared_ptr<CLayer>(new CLayer(levelIndex, layerTag));
}
