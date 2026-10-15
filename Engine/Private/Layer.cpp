// 객체 수명과 단계별 갱신 목록, 최종 카메라 공간 후보를 관리하는 구현이다.
#include "Layer.h"

// 현재 카메라·그림자·최적화 설정과 Profiler를 조회한다. 조회가 저작 설정을 저장하지는 않는다.
#include "GameInstance.h"
// part tag를 받는 조회를 ContainerObject의 자식 Component 검색으로 연결한다.
#include "ContainerObject.h"
// Layer 단계 이름의 범위 측정과 후보/CPU 작업 개수의 진단 카운터를 기록한다.
#include "Profiler.h"
// 선택한 CPU 작업 묶음을 실행하고 합류한 뒤 제출하며 실행 통계를 받는다.
#include "CpuJobPool.h"
// 렌더 최적화·가시성 설정과 그 설정의 revision을 읽는 기존 include 경계다.
#include "Engine_RenderTypes.h"
// 구·평면의 유한성, 정규화 오차와 부호에 따른 거리 크기를 검사한다.
#include <cmath>
// 분할 축을 고르는 double 최솟값/최댓값 축적에 DBL_MAX를 사용한다.
#include <cfloat>
// 공간 노드 개수가 index 범위를 넘으면 length_error로 보고하고 보수적 전체 제출로 전환한다.
#include <stdexcept>
// 평면 경계 오차를 float epsilon 기준으로 계산한다.
#include <limits>
// 이전 여섯 카메라 평면과의 byte 일치를 확인해 후보 재사용을 판단한다.
#include <cstring>

// CObject_Manager가 Layer를 만들 때 호출한다. 프로파일 이름만 UTF-8로 준비하며 변환 실패는 Unnamed로 남긴다.
// 보충: Unnamed 대체는 UTF-8 변환 길이의 첫 조회가 실패한 경우다. 실제 변환을 수행하는 두 번째 호출의 반환값은 별도로 검사하지 않는다.
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

// shared_ptr 소유 목록이 해제되기 전에 객체의 Layer 역참조를 끊는다. 외부에 살아 있는 객체도 옛 Layer를 참조하지 않는다.
CLayer::~CLayer()
{
    for (auto& member : m_FinalCameraMembers)
        if (member.Object->m_FinalCameraLayer.Owner == this)
        {
            member.Object->m_FinalCameraLayer.Owner = nullptr;
            member.Object->m_FinalCameraLayer.DirtyQueued = false;
        }
}

// 현재 등록 순서의 iIndex까지 순회한다. 결과는 shared_ptr이므로 Layer 밖에서도 수명을 공유할 수 있다.
shared_ptr<CGameObject> CLayer::Get_GameObject(uint32_t iIndex)
{
	if (iIndex >= m_GameObjects.size())
		return nullptr;

	auto iter = m_GameObjects.begin();
	for (uint32_t index = 0; index < iIndex; ++index)
		++iter;

	return iter != m_GameObjects.end() ? *iter : nullptr;
}

// 일반 객체의 Component map 조회로 이어진다. 잘못된 인덱스는 nullptr를 반환한다.
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

// 이 overload는 CContainerObject를 전제로 한다. static_pointer_cast는 런타임 타입 검사가 아니다.
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

// 소유 목록과 phase 목록을 등록하고, 최종 카메라 제출 참여 객체에는 Layer 역참조를 설정한다.
// nullptr와 이미 등록된 최종 카메라 객체는 사전에 거부한다. 이후 할당 예외의 부분 변경 rollback은 없다.
HRESULT CLayer::Add_GameObject(shared_ptr<CGameObject> pGameObject)
{
	if (nullptr == pGameObject)
		return E_FAIL;

    if (pGameObject->m_FinalCameraLayer.Owner != nullptr)
        return E_INVALIDARG;
	m_GameObjects.push_back(pGameObject);
	const uint8_t phases = pGameObject->Get_UpdatePhaseMask();
	for (size_t phase = 0; phase < m_PhaseObjects.size(); ++phase)
		if (phases & (1u << phase))
			m_PhaseObjects[phase].push_back(pGameObject.get());
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

// 갱신 목록과 카메라/dirty 역참조를 먼저 지운 뒤 소유 참조를 제거한다. 이동한 member들의 index도 다시 맞춘다.
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
		if (member != phase.end())
			phase.erase(member);
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

// CObject_Manager가 전달하는 프레임 단계다. 입력 처리 뒤 카메라 갱신 전에 참여 객체만 호출한다.
void CLayer::Priority_Update(f32_t fTimeDelta)
{
	CProfilerScope profile(CGameInstance::Get().Get_Profiler(), m_ProfileScopeNames[0]);
	for (CGameObject* pGameObject : m_PhaseObjects[0])
	{
		if (nullptr != pGameObject)
			pGameObject->Priority_Update(fTimeDelta);
	}
}

// CObject_Manager가 전달하는 프레임 단계다. 카메라 갱신 뒤 일반 동작을 갱신한다. 원래 등록 순서를 유지한다.
void CLayer::Update(f32_t fTimeDelta)
{
	CProfilerScope profile(CGameInstance::Get().Get_Profiler(), m_ProfileScopeNames[1]);
	for (CGameObject* pGameObject : m_PhaseObjects[1])
	{
		if (nullptr != pGameObject)
			pGameObject->Update(fTimeDelta);
	}
}

// CObject_Manager가 전달하는 프레임 단계다. 물리 결과를 소비하는 객체만 호출한다.
void CLayer::Post_Physics_Update(f32_t fTimeDelta)
{
	CProfilerScope profile(CGameInstance::Get().Get_Profiler(), m_ProfileScopeNames[2]);
	for (CGameObject* pGameObject : m_PhaseObjects[2])
	{
		if (nullptr != pGameObject)
			pGameObject->Post_Physics_Update(fTimeDelta);
	}
}

// CObject_Manager가 전달하는 프레임 단계다. Level 갱신이 끝난 뒤 참여 객체만 호출한다.
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
    // 유한한 월드 공간 구와 양수 반경만 트리에 넣는다. 사용할 수 없는 bounds는 항상 제출 목록으로 보낸다.
    bool ValidSpatialBounds(const CGameObject::FINAL_CAMERA_SPATIAL_BOUNDS& bounds)
    {
        return std::isfinite(bounds.Center.x) && std::isfinite(bounds.Center.y) &&
            std::isfinite(bounds.Center.z) && std::isfinite(bounds.Radius) && bounds.Radius > 0.f;
    }

    // 최종 view/projection에서 평면 6개를 추출한다. 정규화 검증 실패 시 호출자는 frustum 탈락을 적용하지 않는다.
    bool CaptureLayerPlanes(std::array<float4_t, 6>& output)
    {
        auto& game = CGameInstance::Get();
        const auto* view = game.Get_Transform(D3DTS::VIEW);
        const auto* projection = game.Get_Transform(D3DTS::PROJ);
        if (!view || !projection)
            return false;
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
                XMVector4IsNaN(planes[i]) || XMVector4IsInfinite(planes[i]))
                return false;
            XMStoreFloat4(&output[i], XMPlaneNormalize(planes[i]));
            const auto& p = output[i];
            const double length = double(p.x) * p.x + double(p.y) * p.y + double(p.z) * p.z;
            if (!std::isfinite(p.w) || !std::isfinite(length) ||
                std::abs(length - 1.) > 8. * std::numeric_limits<float>::epsilon())
                return false;
        }
        return true;
    }

    // 정규화 평면의 바깥쪽이 양수인 규약으로 AABB를 보수적으로 검사한다.
    // 완전히 안쪽인 평면 bit만 지워 자식에서 같은 평면 검사를 반복하지 않게 한다.
    bool IntersectsLayerPlanes(const std::array<float4_t, 6>& planes,
        const std::array<double, 3>& minimum, const std::array<double, 3>& maximum,
        uint8_t& remainingPlanes)
    {
        if (remainingPlanes == 0u)
            return true;
        for (size_t i = 0u; i < planes.size(); ++i)
        {
            const uint8_t bit = static_cast<uint8_t>(1u << i);
            if (!(remainingPlanes & bit))
                continue;
            const auto& plane = planes[i];
            const double x = double(plane.x) * (plane.x >= 0.f ? minimum[0] : maximum[0]);
            const double y = double(plane.y) * (plane.y >= 0.f ? minimum[1] : maximum[1]);
            const double z = double(plane.z) * (plane.z >= 0.f ? minimum[2] : maximum[2]);
            const double magnitude = std::abs(x) + std::abs(y) + std::abs(z) + std::abs(double(plane.w));
            // 경계 접촉과 부동소수점 오차 범위는 후보로 남긴다. 정확한 판정은 각 제출 객체가 담당한다.
            const double tolerance = 32. * std::numeric_limits<float>::epsilon() * (std::max)(1., magnitude);
            if (x + y + z + double(plane.w) > tolerance)
                return false;

            const double farX = double(plane.x) * (plane.x >= 0.f ? maximum[0] : minimum[0]);
            const double farY = double(plane.y) * (plane.y >= 0.f ? maximum[1] : minimum[1]);
            const double farZ = double(plane.z) * (plane.z >= 0.f ? maximum[2] : minimum[2]);
            const double farMagnitude = std::abs(farX) + std::abs(farY) + std::abs(farZ) + std::abs(double(plane.w));
            const double insideTolerance = 32. * std::numeric_limits<float>::epsilon() * (std::max)(1., farMagnitude);
            // 가장 먼 꼭짓점도 여유를 두고 안쪽이면 모든 자식도 안쪽이다.
            // 평면에 닿는 경우에는 bit를 남겨 원래의 보수적 검사를 유지한다.
            if (farX + farY + farZ + double(plane.w) < -insideTolerance)
                remainingPlanes &= static_cast<uint8_t>(~bit);
        }
        return true;
    }
}

// CGameObject가 bounds/표시 정책 변경을 알리는 경로다. 한 객체는 한 번만 queue에 넣는다.
// queue 할당 실패 시 다음 제출에서 전체 rebuild하도록 표시해 변경을 잃지 않는다.
void CLayer::Invalidate_FinalCameraSpatialBounds(CGameObject* object)
{
    if (!object || object->m_FinalCameraLayer.Owner != this || object->m_FinalCameraLayer.DirtyQueued)
        return;
    try
    {
        m_FinalCameraDirty.push_back(object);
        object->m_FinalCameraLayer.DirtyQueued = true;
    }
    catch (const std::bad_alloc&)
    {
        m_FinalCameraTopologyDirty = true;
    }
}

// leaf는 객체의 구를 AABB로 감싸고, 내부 node는 두 자식의 AABB와 그림자/유예 정책을 합친다.
// 경계가 바뀌면 연속 탈락 횟수를 초기화한다.
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

// [first,end) 중심 분포가 가장 넓은 축의 중앙값으로 이분한다. 깊이 우선으로 node를 저장하여
// End만으로 subtree를 건너뛰며, 중심이 같으면 member index로 비교 순서를 고정한다.
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
            if (high[i] - low[i] > high[axis] - low[axis])
                axis = i;
        const auto coordinate = [this, axis](size_t member)
        {
            const auto& center = m_FinalCameraMembers[member].Bounds.Center;
            return axis == 0u ? center.x : axis == 1u ? center.y : center.z;
        };
        const size_t middle = first + (end - first) / 2u;
        std::nth_element(m_FinalCameraOrder.begin() + first, m_FinalCameraOrder.begin() + middle,
            m_FinalCameraOrder.begin() + end, [&](size_t a, size_t b)
            {
                const float x = coordinate(a), y = coordinate(b);
                return x != y ? x < y : a < b;
            });
        const uint32_t left = Build_FinalCameraNode(first, middle, index);
        const uint32_t right = Build_FinalCameraNode(middle, end, index);
        m_FinalCameraNodes[index].Left = left;
        m_FinalCameraNodes[index].Right = right;
    }
    m_FinalCameraNodes[index].End = static_cast<uint32_t>(m_FinalCameraNodes.size());
    Refit_FinalCameraNode(index);
    return index;
}

// 등록/제거 또는 bounds 제공 여부가 바뀌면 트리를 새로 만든다. 유효 bounds가 없는 객체는 Always에 둔다.
// 할당/크기 실패는 Submit_FinalCamera가 잡아 전체 객체 직접 제출로 돌아간다.
void CLayer::Rebuild_FinalCameraHierarchy()
{
    m_FinalCameraNodes.clear();
    m_FinalCameraOrder.clear();
    m_FinalCameraAlways.clear();
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
    if (m_FinalCameraOrder.size() > UINT32_MAX / 2u)
        throw std::length_error("final-camera hierarchy size");
    m_FinalCameraNodes.reserve(m_FinalCameraOrder.size() * 2u);
    if (!m_FinalCameraOrder.empty())
        Build_FinalCameraNode(0u, m_FinalCameraOrder.size(), UINT32_MAX);
    m_FinalCameraTopologyDirty = false;
}

// 구성은 그대로이고 구만 바뀌었다면 해당 leaf에서 부모까지 refit한다.
// bounded 여부가 달라지면 Always/트리 소속도 바뀌므로 전체 rebuild가 필요하다.
void CLayer::Refresh_FinalCameraHierarchy()
{
    if (m_FinalCameraTopologyDirty)
    {
        Rebuild_FinalCameraHierarchy();
        return;
    }
    for (CGameObject* object : m_FinalCameraDirty)
        object->m_FinalCameraLayer.DirtyQueued = false;
    for (CGameObject* object : m_FinalCameraDirty)
    {
        auto& member = m_FinalCameraMembers[object->m_FinalCameraLayer.Index];
        CGameObject::FINAL_CAMERA_SPATIAL_BOUNDS bounds{};
        const bool bounded = object->Try_GetFinalCameraSpatialBounds(bounds) && ValidSpatialBounds(bounds);
        if (bounded != member.Bounded)
        {
            m_FinalCameraTopologyDirty = true;
            break;
        }
        member.Bounds = bounds;
        if (bounded)
            for (uint32_t node = member.Leaf; node != UINT32_MAX; node = m_FinalCameraNodes[node].Parent)
                Refit_FinalCameraNode(node);
    }
    m_FinalCameraDirty.clear();
    if (m_FinalCameraTopologyDirty)
        Rebuild_FinalCameraHierarchy();
}

// 계층/bounds revision과 그림자 상태가 맞을 때 후보를 재사용한다.
// frustum을 켠 경우에는 유효한 카메라 평면 일치와 탈락 유예 종료도 필요하다.
void CLayer::Submit_FinalCamera()
{
    if (m_FinalCameraMembers.empty())
        return;
    auto& game = CGameInstance::Get();
    CProfilerScope profile(game.Get_Profiler(), m_FinalCameraScopeName);
    try
    {
        // 먼저 구성/경계를 갱신하고 현재 카메라·그림자·옵션이 이전 후보와 호환되는지 확인한다.
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
        // 재사용할 수 없으면 공간 트리를 순회한다. 경계 없는 객체와 그림자/탈락 유예가 필요한 객체는 보수적으로 남긴다.
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
                // 그림자만 필요한 subtree도 제출하지만 color payload의 병렬 준비 대상은 아니다.
                // 부모가 카메라 밖이면 그 안의 자식도 카메라 밖이라는 결과를 재사용한다.
                node.CameraIntersects = !optimization.FrustumEnabled || !cameraValid ||
                    ((node.Parent == UINT32_MAX || m_FinalCameraNodes[node.Parent].CameraIntersects) &&
                     IntersectsLayerPlanes(planes, node.Minimum, node.Maximum, node.RemainingPlanes));
                const bool intersects = node.CameraIntersects || (shadowEnabled && node.ShadowCaster);
                if (intersects)
                    node.RejectedFrames = 0u;
                else if (node.RejectedFrames < node.RejectGraceFrames)
                {
                    ++node.RejectedFrames;
                    gracePending = true;
                }
                else
                {
                    i = node.End;
                    continue;
                }
                if (node.Member != SIZE_MAX)
                {
                    m_FinalCameraCandidates.push_back(node.Member);
                    if (node.CameraIntersects)
                        m_FinalCameraCpuCandidates.push_back(node.Member);
                }
                ++i;
            }
            // 트리 순회는 후보를 찾는 순서만 바꾼다. 실제 제출은 원래 member 순서로 되돌린다.
            std::sort(m_FinalCameraCandidates.begin(), m_FinalCameraCandidates.end());
            std::sort(m_FinalCameraCpuCandidates.begin(), m_FinalCameraCpuCandidates.end());
            m_FinalCameraPlanes = planes;
            m_FinalCameraShadowEnabled = shadowEnabled;
            // 카메라가 같아도 탈락 유예 프레임이 남아 있으면 다시 검사해 카운터를 진행해야 한다.
            m_FinalCameraCandidatesReusable = (!optimization.FrustumEnabled || cameraValid) && !gracePending;
            m_FinalCameraOptimizationRevision = optimization.Revision;
        }
    }
    catch (const std::bad_alloc&)
    {
        m_FinalCameraTopologyDirty = true;
        m_FinalCameraCandidatesReusable = false;
        for (auto& member : m_FinalCameraMembers)
            member.Object->Submit_FinalCamera();
        return;
    }
    catch (const std::length_error&)
    {
        m_FinalCameraTopologyDirty = true;
        m_FinalCameraCandidatesReusable = false;
        for (auto& member : m_FinalCameraMembers)
            member.Object->Submit_FinalCamera();
        return;
    }
    // 후보 목록이 확정된 뒤 CPU 준비를 합류하고 등록 순서의 객체 제출로 넘어간다.
    Prepare_FinalCameraCpuJobs();
    for (size_t member : m_FinalCameraCandidates)
        m_FinalCameraMembers[member].Object->Submit_FinalCamera();
}


// Submit 직전에 owner thread에서 CPU 전용 작업을 모은다. job은 객체의 상태를 차용하며
// Run_CpuJobs가 모든 callback을 join한 뒤에만 반환하므로 CPU callback 실행이 후속 제출과 겹치지 않는다.
void CLayer::Prepare_FinalCameraCpuJobs()
{
    // 작업 수집에도 비용이 든다. 설정이 켜진 경우에만 준비하며 OFF이면 기존 owner thread 경로를 쓴다.
    if (!CGameInstance::Get().Get_MapVisibilitySettings().ParallelPreparationEnabled)
        return;
    auto* profiler = CGameInstance::Get().Get_Profiler();
    CProfilerWorkScope visibilityWork(profiler, EProfilerWork::MapBatchVisibility);
    CProfilerScope prepare(profiler, "Map.Visibility.Prepare");
    m_FinalCameraCpuJobs.clear();
    m_FinalCameraCpuRanges.clear();
    try
    {
        // 객체에서 작업을 차용하기 전에 목록 용량을 준비한다. 이 할당에 실패하면
        // 차용을 시작하지 않고 각 객체의 원래 제출 준비 경로를 유지한다.
        m_FinalCameraCpuJobs.reserve(m_FinalCameraCpuCandidates.size());
        m_FinalCameraCpuRanges.reserve(m_FinalCameraCpuCandidates.size() / 8u + 1u);
        // Cost는 객체가 제공한 상대 작업량이다. 시간 단위가 아니며 묶음 크기와 helper 수 선택에 쓴다.
        uint64_t totalCost = 0u;
        for (const size_t member : m_FinalCameraCpuCandidates)
        {
            CGameObject::FINAL_CAMERA_CPU_JOB job{};
            if (!m_FinalCameraMembers[member].Object->Try_PrepareFinalCameraCpuJob(job) ||
                !job.Context || !job.Execute || !job.Cost)
                continue;
            m_FinalCameraCpuJobs.push_back(job);
            totalCost += job.Cost;
        }
        if (m_FinalCameraCpuJobs.empty())
            return;
        // 비용 128 이상이며 8개 이상이거나, 64개에 도달하면 하나의 [First,End) 묶음을 닫는다.
        size_t first = 0u;
        uint64_t cost = 0u;
        for (size_t i = 0u; i < m_FinalCameraCpuJobs.size(); ++i)
        {
            cost += m_FinalCameraCpuJobs[i].Cost;
            const size_t count = i + 1u - first;
            if ((cost >= 128u && count >= 8u) || count >= 64u)
            {
                m_FinalCameraCpuRanges.push_back({first, i + 1u});
                first = i + 1u;
                cost = 0u;
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
        // 동기 pool은 예외를 다시 던지기 전에도 실행 중 callback을 모두 join한다.
        // 객체는 준비가 끝난 staging을 소비하거나 Submit에서 끝나지 않은 준비를 수행한다.
    }
}

// CObject_Manager가 소유할 Layer를 만든다. 실제 객체 등록은 Add_GameObject에서 수행한다.
shared_ptr<CLayer> CLayer::Create(uint32_t levelIndex, const wstring_t& layerTag)
{
	return shared_ptr<CLayer>(new CLayer(levelIndex, layerTag));
}
