#include "Mesh.h"
// Assimp 메시의 정점·면·본 구조를 읽는다.
#include "Assimp/scene.h"
// static/skinned 정점의 GPU 배치와 stride를 결정한다.
#include "Engine_VertexTypes.h"
// WModel 디코더가 복원한 메시·골격 데이터를 입력받는다.
#include "BinaryAsset/ModelAssetData.h"
// 모델 본 이름을 연결하고 현재 combined 변환으로 skin palette를 만든다.
#include "Bone.h"

#include "Shader.h"
// 세션의 LOD 설정과 제출 geometry를 기록할 profiler를 조회한다.
#include "GameInstance.h"
// LOD 사용 여부를 읽는 렌더링 설정의 완전한 타입이 필요하다.
#include "Engine_RenderTypes.h"
// draw 수·index 수와 실제 제출한 메시를 계측한다.
#include "Profiler.h"
// 화면 오차로 선택할 파생 index 버퍼를 생성하고 조회한다.
#include "StaticMeshLod.h"
// BVH의 중앙 분할과 cluster index 범위 검증에 사용한다.
#include <algorithm>
// 피킹 위치·skin weight의 유한값과 정규화 오차를 검사한다.
#include <cmath>
// BVH 경계의 최초 최소·최대값을 준비한다.
#include <limits>

CMesh::CMesh(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CVIBuffer { pDevice, pContext }
{
}

CMesh::~CMesh()
{
}

// 원본 메시의 정점·삼각형과 스킨 정보를 GPU 버퍼로 준비한다. 실패한 새 메시의 수명 정리는 Create의 소유 포인터가 맡는다.
HRESULT CMesh::Initialize_Prototype(MODEL eType, const aiMesh* pAIMesh, const vector<shared_ptr<class CBone>>& Bones, fmatrix_t PreTransformMatrix)
{
#pragma region PUBLIC_DATA
	strcpy_s(m_szName, pAIMesh->mName.C_Str());
	m_iMaterialIndex = pAIMesh->mMaterialIndex;
	m_iVertexStride = MODEL::NONANIM == eType ? sizeof(VTXMESH) : sizeof(VTXANIMMESH);
	m_iNumVertices = pAIMesh->mNumVertices;
	m_iIndexStride = 4;
	m_iNumIndices = pAIMesh->mNumFaces * 3;
	m_iNumVertexBuffers = 1;
	m_eIndexFormat = DXGI_FORMAT_R32_UINT;
	m_ePrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
#pragma endregion

#pragma region VERTEX_BUFFER
	HRESULT hr = MODEL::NONANIM == eType ? 
		Ready_VertexBuffer_NonAnim(pAIMesh, PreTransformMatrix) : 
		Ready_VertexBuffer_Anim(pAIMesh, Bones);

	if (FAILED(hr))
		return E_FAIL;	
#pragma endregion

#pragma region INDEX_BUFFER
	D3D11_BUFFER_DESC		IndexBufferDesc{};
	IndexBufferDesc.ByteWidth = m_iIndexStride * m_iNumIndices;
	IndexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	IndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IndexBufferDesc.CPUAccessFlags = 0;
	IndexBufferDesc.MiscFlags = 0;
	IndexBufferDesc.StructureByteStride = m_iIndexStride;

	unique_ptr<uint32_t[]>		pIndices = make_unique<uint32_t[]>(m_iNumIndices);

	uint32_t	iNumIndices = {};

	for (uint32_t i = 0; i < pAIMesh->mNumFaces; i++)
	{
		pIndices[iNumIndices++] = pAIMesh->mFaces[i].mIndices[0];
		pIndices[iNumIndices++] = pAIMesh->mFaces[i].mIndices[1];
		pIndices[iNumIndices++] = pAIMesh->mFaces[i].mIndices[2];
	}

	D3D11_SUBRESOURCE_DATA			IndexInitialData{};
	IndexInitialData.pSysMem = pIndices.get();

	if (FAILED(m_pDevice->CreateBuffer(&IndexBufferDesc, &IndexInitialData, &m_pIB)))
		return E_FAIL;

#pragma endregion

	return S_OK;
}

// 디코더의 geometry를 GPU 버퍼로 변환한다. CPU 입력은 호출 중 차용하고 피킹에 필요한 값만 별도로 보관한다.
HRESULT CMesh::Initialize_Prototype(MODEL eType, const MODEL_MESH_DATA& mesh,
	const MODEL_SKELETON_DATA& skeleton, fmatrix_t PreTransformMatrix, bool_t preserveStaticVertices, bool_t buildPickingGeometry)
{
	const bool_t isAnimated = MODEL::ANIM == eType;
	if ((preserveStaticVertices && isAnimated) || mesh.name.size() >= MAX_PATH || mesh.indices.empty() ||
		(isAnimated && mesh.vertexKind != MODEL_VERTEX_KIND::SKINNED) ||
		(!isAnimated && mesh.vertexKind != MODEL_VERTEX_KIND::STATIC))
		return E_FAIL;

	strcpy_s(m_szName, mesh.name.c_str());
	m_iMaterialIndex = mesh.materialIndex;
	m_iVertexStride = isAnimated ? sizeof(VTXANIMMESH) : sizeof(VTXMESH);
	m_iNumVertices = isAnimated
		? static_cast<uint32_t>(mesh.skinnedVertices.size())
		: static_cast<uint32_t>(mesh.vertices.size());
	m_iIndexStride = sizeof(uint32_t);
	m_iNumIndices = static_cast<uint32_t>(mesh.indices.size());
	m_iNumVertexBuffers = 1;
	m_eIndexFormat = DXGI_FORMAT_R32_UINT;
	m_ePrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	if (0 == m_iNumVertices)
		return E_FAIL;

	D3D11_BUFFER_DESC vertexBufferDesc{};
	vertexBufferDesc.ByteWidth = m_iVertexStride * m_iNumVertices;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.StructureByteStride = m_iVertexStride;
	D3D11_SUBRESOURCE_DATA vertexInitialData{};

	vector<VTXMESH> staticVertices;
	if (isAnimated)
	{
		vertexInitialData.pSysMem = mesh.skinnedVertices.data();
		if (skeleton.bones.empty() || skeleton.bones.size() > 512)
			return E_FAIL;

		m_bUsesSkeletonPalette = true;
		m_iNumBones = static_cast<uint32_t>(skeleton.bones.size());
		m_BoneIndices.reserve(m_iNumBones);
		m_OffsetMatrices.reserve(m_iNumBones);
		for (uint32_t i = 0; i < m_iNumBones; ++i)
		{
			m_BoneIndices.push_back(i);
			m_OffsetMatrices.push_back(skeleton.bones[i].inverseBind);
		}
        Prepare_BoneVertexBounds(mesh);
	}
	else
	{
		if ((mesh.hasColor0 && mesh.color0Rgba8.size() != mesh.vertices.size()) ||
			(!mesh.hasColor0 && !mesh.color0Rgba8.empty()))
			return E_INVALIDARG;
		staticVertices = mesh.vertices;
		for (size_t index = 0; index < staticVertices.size(); ++index)
		{
			VTXMESH& vertex = staticVertices[index];
			// WModel reader가 RGBA를 이미 복원했으므로 채널 순서를 다시 바꾸지 않는다.
			vertex.color0Rgba8 = mesh.hasColor0 ? mesh.color0Rgba8[index] : 0xffffffffu;
            if (preserveStaticVertices) continue;
			XMStoreFloat3(&vertex.vPosition,
				XMVector3TransformCoord(XMLoadFloat3(&vertex.vPosition), PreTransformMatrix));
			XMStoreFloat3(&vertex.vNormal,
				XMVector3Normalize(XMVector3TransformNormal(XMLoadFloat3(&vertex.vNormal), PreTransformMatrix)));
			XMStoreFloat3(&vertex.vTangent,
				XMVector3Normalize(XMVector3TransformNormal(XMLoadFloat3(&vertex.vTangent), PreTransformMatrix)));
			XMStoreFloat3(&vertex.vBinormal,
				XMVector3Normalize(XMVector3TransformNormal(XMLoadFloat3(&vertex.vBinormal), PreTransformMatrix)));
		}
		vertexInitialData.pSysMem = staticVertices.data();
	}

	if (FAILED(m_pDevice->CreateBuffer(&vertexBufferDesc, &vertexInitialData, &m_pVB)))
		return E_FAIL;

	D3D11_BUFFER_DESC indexBufferDesc{};
	indexBufferDesc.ByteWidth = m_iIndexStride * m_iNumIndices;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.StructureByteStride = m_iIndexStride;
	D3D11_SUBRESOURCE_DATA indexInitialData{};
	indexInitialData.pSysMem = mesh.indices.data();
	if (FAILED(m_pDevice->CreateBuffer(&indexBufferDesc, &indexInitialData, &m_pIB)))
		return E_FAIL;

    // 피킹은 원래 placement를 사용한다. 표시 전용 cluster에 CPU geometry·BVH를 중복 보관하지 않는다.
    if (!buildPickingGeometry) return S_OK;

    auto pick = make_shared<PICK_GEOMETRY>();
    pick->skinned = isAnimated;
    pick->indices = mesh.indices;
    pick->vertices.reserve(m_iNumVertices);
    if (isAnimated)
        for (const auto& vertex : mesh.skinnedVertices)
            pick->vertices.push_back({vertex.vPosition, vertex.vBlendIndices, vertex.vBlendWeights});
    else
        for (const auto& vertex : staticVertices)
            pick->vertices.push_back({vertex.vPosition, {}, {}});
    if (!isAnimated) Prepare_StaticPickGeometry(*pick);
    m_PickGeometry = std::move(pick);
	return S_OK;
}

// 유효한 삼각형만 BVH에 넣는다. 가장 긴 축의 삼각형 중심값을 기준으로 개수를 절반으로 나눈다.
void CMesh::Prepare_StaticPickGeometry(PICK_GEOMETRY& geometry)
{
    if (geometry.skinned) return;
    geometry.triangles.clear();
    geometry.nodes.clear();
    geometry.triangles.reserve(geometry.indices.size() / 3u);
    for (size_t index = 0u; index + 2u < geometry.indices.size(); index += 3u)
    {
        bool valid = true;
        for (size_t lane = 0u; lane < 3u; ++lane)
        {
            const uint32_t vertex = geometry.indices[index + lane];
            if (vertex >= geometry.vertices.size()) { valid = false; break; }
            const auto& point = geometry.vertices[vertex].position;
            if (!std::isfinite(point.x) || !std::isfinite(point.y) || !std::isfinite(point.z))
            { valid = false; break; }
        }
        if (valid) geometry.triangles.push_back(static_cast<uint32_t>(index / 3u));
    }
    if (geometry.triangles.empty()) return;
    geometry.nodes.reserve(2u * (geometry.triangles.size() / 4u + 1u));
    const auto build = [&](const auto& self, const uint32_t first, const uint32_t count) -> void
    {
        const uint32_t nodeIndex = static_cast<uint32_t>(geometry.nodes.size());
        geometry.nodes.emplace_back();
        vector_t minimum = XMVectorReplicate((std::numeric_limits<float>::max)());
        vector_t maximum = -minimum;
        for (uint32_t ordinal = first; ordinal < first + count; ++ordinal)
            for (uint32_t lane = 0u; lane < 3u; ++lane)
            {
                const auto vertex = geometry.indices[geometry.triangles[ordinal] * 3u + lane];
                const vector_t point = XMLoadFloat3(&geometry.vertices[vertex].position);
                minimum = XMVectorMin(minimum, point); maximum = XMVectorMax(maximum, point);
            }
        XMStoreFloat3(&geometry.nodes[nodeIndex].minimum, minimum);
        XMStoreFloat3(&geometry.nodes[nodeIndex].maximum, maximum);
        if (count <= 8u)
        {
            geometry.nodes[nodeIndex].first = first;
            geometry.nodes[nodeIndex].count = count;
        }
        else
        {
            float3_t extent;
            XMStoreFloat3(&extent, maximum - minimum);
            const uint32_t axis = extent.x >= extent.y && extent.x >= extent.z ? 0u :
                (extent.y >= extent.z ? 1u : 2u);
            const auto centroid = [&](const uint32_t triangle)
            {
                double value = 0.0;
                for (uint32_t lane = 0u; lane < 3u; ++lane)
                {
                    const auto& point = geometry.vertices[geometry.indices[triangle * 3u + lane]].position;
                    value += axis == 0u ? point.x : (axis == 1u ? point.y : point.z);
                }
                return value;
            };
            const uint32_t half = count / 2u;
            std::nth_element(geometry.triangles.begin() + first,
                geometry.triangles.begin() + first + half, geometry.triangles.begin() + first + count,
                [&](const uint32_t left, const uint32_t right)
                {
                    const double a = centroid(left), b = centroid(right);
                    return a < b || (a == b && left < right);
                });
            self(self, first, half);
            self(self, first + half, count - half);
        }
        // 전위 배열에서 이 subtree 다음 위치를 기록해 ray가 경계를 놓치면 자식 전체를 건너뛴다.
        geometry.nodes[nodeIndex].end = static_cast<uint32_t>(geometry.nodes.size());
    };
    build(build, 0u, static_cast<uint32_t>(geometry.triangles.size()));
}

// CModel이 변환한 local ray로 정적 BVH를 조회한다. 놓친 경우 distance를 바꾸지 않는다.
bool_t CMesh::Try_PickStaticLocal(fvector_t origin, fvector_t direction,
    const f32_t maxDistance, const f32_t cullSign, f32_t& distance) const
{
    if (m_hasUniqueVertexBuffer || !m_PickGeometry || m_PickGeometry->skinned ||
        m_PickGeometry->nodes.empty()) return false;
    const auto& geometry = *m_PickGeometry;
    float closest = maxDistance;
    bool hit = false;
    for (uint32_t nodeIndex = 0u; nodeIndex < geometry.nodes.size();)
    {
        const auto& node = geometry.nodes[nodeIndex];
        BoundingBox bounds;
        BoundingBox::CreateFromPoints(bounds, XMLoadFloat3(&node.minimum), XMLoadFloat3(&node.maximum));
        float entry = 0.f;
        if (!bounds.Intersects(origin, direction, entry) || entry > closest)
        { nodeIndex = node.end; continue; }
        for (uint32_t ordinal = node.first; ordinal < node.first + node.count; ++ordinal)
        {
            const uint32_t triangle = geometry.triangles[ordinal] * 3u;
            const vector_t a = XMLoadFloat3(&geometry.vertices[geometry.indices[triangle]].position);
            const vector_t b = XMLoadFloat3(&geometry.vertices[geometry.indices[triangle + 1u]].position);
            const vector_t c = XMLoadFloat3(&geometry.vertices[geometry.indices[triangle + 2u]].position);
            // D3D 기본 CW 앞면은 관찰자를 향한다. LH view에서 입사 ray와 법선의 내적은 음수다.
            if (cullSign != 0.f && XMVectorGetX(XMVector3Dot(
                XMVector3Cross(b - a, c - a), direction)) * cullSign >= 0.f) continue;
            float candidate;
            if (TriangleTests::Intersects(origin, direction, a, b, c, candidate) && candidate <= closest)
            { closest = candidate; hit = true; }
        }
        ++nodeIndex;
    }
    if (hit) distance = closest;
    return hit;
}

HRESULT CMesh::Initialize(void* pArg)
{
	return S_OK;
}

// 현재 pose의 보수적 경계를 계산할 본별 bind 위치 범위를 만든다. 모든 weight 검증 뒤에만 반영한다.
void CMesh::Prepare_BoneVertexBounds(const MODEL_MESH_DATA& mesh)
{
    vector<BONE_VERTEX_BOUNDS> staged(m_iNumBones);
    for (const auto& vertex : mesh.skinnedVertices)
    {
        const float3_t& position = vertex.vPosition;
        if (!std::isfinite(position.x) || !std::isfinite(position.y) ||
            !std::isfinite(position.z)) return;
        const uint32_t indices[4] = { vertex.vBlendIndices.x, vertex.vBlendIndices.y,
            vertex.vBlendIndices.z, vertex.vBlendIndices.w };
        const float weights[4] = { vertex.vBlendWeights.x, vertex.vBlendWeights.y,
            vertex.vBlendWeights.z, vertex.vBlendWeights.w };
        float sum = 0.f;
        for (uint32_t lane = 0u; lane < 4u; ++lane)
        {
            if (!std::isfinite(weights[lane]) || weights[lane] < 0.f ||
                indices[lane] >= staged.size()) return;
            sum += weights[lane];
            if (weights[lane] == 0.f) continue;
            auto& bounds = staged[indices[lane]];
            if (!bounds.valid)
            {
                bounds.minimum = bounds.maximum = position;
                bounds.valid = true;
                continue;
            }
            bounds.minimum.x = (std::min)(bounds.minimum.x, position.x);
            bounds.minimum.y = (std::min)(bounds.minimum.y, position.y);
            bounds.minimum.z = (std::min)(bounds.minimum.z, position.z);
            bounds.maximum.x = (std::max)(bounds.maximum.x, position.x);
            bounds.maximum.y = (std::max)(bounds.maximum.y, position.y);
            bounds.maximum.z = (std::max)(bounds.maximum.z, position.z);
        }
        // WModel의 정규화된 비음수 skin weight에만 이 볼록 경계가 성립한다. geometry 자체는 바꾸지 않는다.
        if (!std::isfinite(sum) || std::fabs(sum - 1.f) > 1.e-5f) return;
    }
    m_BoneVertexBounds = std::move(staged);
}

// 메시의 inverse bind와 현재 본 pose를 합성해 CModel이 제공한 출력 배열에 기록한다.
void CMesh::Build_SkinPalette(const vector<shared_ptr<class CBone>>& Bones,
    float4x4_t* pOutMatrices) const
{
    for (uint32_t i = 0; i < m_iNumBones; ++i)
    {
        XMStoreFloat4x4(&pOutMatrices[i],
            XMLoadFloat4x4(&m_OffsetMatrices[i]) *
            Bones[m_BoneIndices[i]]->Get_CombinedTransformationMatrix());
    }
}

HRESULT CMesh::Render()
{
    const HRESULT result = CVIBuffer::Render();
    if (result == S_OK)
        if (CProfiler* profiler = CGameInstance::Get().Get_Profiler())
            profiler->Record_MeshSubmitted(this, m_iNumIndices, 1, m_szName, m_iNumVertices, m_iMaterialIndex);
    return result;
}

HRESULT CMesh::Render(const MESH_SCREEN_LOD_DESC* screenLod)
{
    if (!screenLod) return Render();
    if (!m_pVB || !m_pIB || !m_iNumIndices) return E_INVALIDARG;
    uint32_t count = 0u, first = 0u;
    const uint32_t level = Select_StaticLod(screenLod, count, first);
    const auto* lod = Get_StaticLod(screenLod->attributeSensitive);
    // 일반 draw도 동일한 원본 VB와 선택 IB만 소비한다. 다음 근경 draw에서 LOD0 IB를 복구한다.
    m_pContext->IASetIndexBuffer(level ? lod->Get_IndexBuffer() : m_pIB.Get(), m_eIndexFormat, 0u);
    m_pContext->DrawIndexed(count, first, 0);
    if (auto* profiler = CGameInstance::Get().Get_Profiler())
    {
        profiler->Add_Counter(EProfilerCounter::DrawCalls);
        profiler->Add_Counter(EProfilerCounter::Indices, count);
        if (lod && !Has_MorphBaseVertices()) profiler->Add_Counter(EProfilerCounter::MapLodAvailableDraws);
        profiler->Add_Counter(level >= 2u ? EProfilerCounter::MapLod2Draws :
            level ? EProfilerCounter::MapLod1Draws : EProfilerCounter::MapLod0Draws);
        profiler->Add_Counter(EProfilerCounter::MapLodSourceIndices, m_iNumIndices);
        profiler->Add_Counter(EProfilerCounter::MapLodSubmittedIndices, count);
        profiler->Record_MeshSubmitted(this, count, 1u, m_szName, m_iNumVertices, m_iMaterialIndex);
    }
    return S_OK;
}

const CStaticMeshLod* CMesh::Get_StaticLod(bool_t attributeSensitive) const
{
    return attributeSensitive ? m_AttributeLod.get() : m_StaticLod ? m_StaticLod.get() : m_AttributeLod.get();
}

bool_t CMesh::Get_StaticLodRange(uint32_t level, bool_t attributeSensitive,
    uint32_t& count, uint32_t& first, f32_t& localError) const
{
    if (!level) { count = m_iNumIndices; first = 0u; localError = 0.f; return true; }
    const auto* lod = Get_StaticLod(attributeSensitive);
    CStaticMeshLod::SELECTION selection{};
    if (!lod || Has_MorphBaseVertices() || !CGameInstance::Get().Get_RenderOptimizationSettings().MeshLodEnabled ||
        !lod->Get_Range(level, selection, localError)) return false;
    count = selection.indexCount; first = selection.firstIndex;
    return true;
}

uint32_t CMesh::Select_StaticLod(const MESH_SCREEN_LOD_DESC* view,
    uint32_t& indexCount, uint32_t& firstIndex) const
{
    indexCount = m_iNumIndices;
    firstIndex = 0u;
    CStaticMeshLod::SELECTION selection{ m_iNumIndices, 0u, 0u };
    const auto* lod = Get_StaticLod(view && view->attributeSensitive);
    if (!view || !lod || Has_MorphBaseVertices() ||
        !CGameInstance::Get().Get_RenderOptimizationSettings().MeshLodEnabled ||
        S_OK != lod->Select_Range(*view, selection))
        return 0u;
    indexCount = selection.indexCount;
    firstIndex = selection.firstIndex;
    return selection.level;
}

// CPU가 보유한 instance 수와 버퍼 범위를 확인한 뒤 공통 메시를 여러 배치에 제출한다.
HRESULT CMesh::Render_Instanced(ID3D11Buffer* pInstanceBuffer,
	uint32_t iInstanceStride, uint32_t iNumInstances,
	uint32_t iInstanceByteOffset, const MESH_SCREEN_LOD_DESC* screenLod)
{
	if (nullptr == pInstanceBuffer ||
		0 == iInstanceStride ||
		0 == iNumInstances)
	{
		return E_INVALIDARG;
	}

	D3D11_BUFFER_DESC instanceDesc{};
	pInstanceBuffer->GetDesc(&instanceDesc);
	const uint64_t requiredBytes = static_cast<uint64_t>(iInstanceByteOffset) +
		static_cast<uint64_t>(iInstanceStride) * iNumInstances;
	if (nullptr == m_pVB || nullptr == m_pIB || 0u == m_iNumIndices ||
		0u == (instanceDesc.BindFlags & D3D11_BIND_VERTEX_BUFFER) ||
		0u != (iInstanceByteOffset % sizeof(f32_t)) ||
		requiredBytes > instanceDesc.ByteWidth)
		return E_INVALIDARG;

    // CPU가 가진 view·불변 LOD 오차로 범위를 선택해 draw마다 1-thread compute를 제출하지 않는다.
    CStaticMeshLod::SELECTION selection{ m_iNumIndices, 0u, 0u };
    selection.level = Select_StaticLod(screenLod, selection.indexCount, selection.firstIndex);
    const bool_t useLod = selection.level > 0u;
    const auto* lod = Get_StaticLod(screenLod && screenLod->attributeSensitive);

	ID3D11Buffer* vertexBuffers[] =
	{
		m_pVB.Get(), pInstanceBuffer
	};

	const uint32_t strides[] =
	{
		m_iVertexStride,
		iInstanceStride
	};

	const uint32_t offsets[] =
	{
		0,
		iInstanceByteOffset
	};

	m_pContext->IASetVertexBuffers(
		0, 2, vertexBuffers, strides, offsets
	);

	m_pContext->IASetIndexBuffer(
        useLod ? lod->Get_IndexBuffer() : m_pIB.Get(),
		m_eIndexFormat,
		0
	);

	m_pContext->IASetPrimitiveTopology(
		m_ePrimitiveTopology);

	if (CProfiler* pProfiler =
		CGameInstance::Get().Get_Profiler())
	{
		pProfiler->Add_Counter(
			EProfilerCounter::DrawCalls);

		pProfiler->Add_Counter(
			EProfilerCounter::InstancedDrawCalls);

		pProfiler->Add_Counter(
			EProfilerCounter::Instances,
			iNumInstances);

        const uint64_t submittedIndices = uint64_t(selection.indexCount) * iNumInstances;
        pProfiler->Add_Counter(EProfilerCounter::Indices, submittedIndices);
        if (screenLod)
        {
            if (lod && !Has_MorphBaseVertices())
                pProfiler->Add_Counter(EProfilerCounter::MapLodAvailableDraws);
            // 확장된 원경 단계도 LOD0으로 오기록하지 않고 기존 LOD2+ 집계에 포함한다.
            const EProfilerCounter levelCounter = selection.level >= 2u ? EProfilerCounter::MapLod2Draws :
                selection.level == 1u ? EProfilerCounter::MapLod1Draws : EProfilerCounter::MapLod0Draws;
            pProfiler->Add_Counter(levelCounter);
            pProfiler->Add_Counter(EProfilerCounter::MapLodSourceIndices,
                uint64_t(m_iNumIndices) * iNumInstances);
            pProfiler->Add_Counter(EProfilerCounter::MapLodSubmittedIndices, submittedIndices);
        }
    }

    m_pContext->DrawIndexedInstanced(
        selection.indexCount,
        iNumInstances,
        selection.firstIndex,
        0,
        0);

    if (CProfiler* profiler = CGameInstance::Get().Get_Profiler())
        profiler->Record_MeshSubmitted(this, selection.indexCount, iNumInstances, m_szName, m_iNumVertices, m_iMaterialIndex);

	return S_OK;
}

// GPU가 확정한 draw 인자를 그대로 제출한다. 여기서는 버퍼 범위·LOD만 검사하고 GPU count를 읽지 않는다.
HRESULT CMesh::Render_InstancedIndirect(ID3D11Buffer* instanceBuffer, uint32_t stride,
    uint32_t byteOffset, ID3D11Buffer* args, uint32_t argsByteOffset,
    uint32_t lodLevel, bool_t attributeSensitive)
{
    if (!instanceBuffer || !args || !stride || !m_pVB || !m_pIB || !m_iNumIndices ||
        byteOffset % sizeof(float) || argsByteOffset % sizeof(uint32_t)) return E_INVALIDARG;
    D3D11_BUFFER_DESC instanceDesc{}, argsDesc{};
    instanceBuffer->GetDesc(&instanceDesc); args->GetDesc(&argsDesc);
    if (!(instanceDesc.BindFlags & D3D11_BIND_VERTEX_BUFFER) ||
        !(argsDesc.MiscFlags & D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS) ||
        uint64_t(byteOffset) + stride > instanceDesc.ByteWidth ||
        uint64_t(argsByteOffset) + 5u * sizeof(uint32_t) > argsDesc.ByteWidth) return E_INVALIDARG;
    uint32_t count = 0u, first = 0u; float error = 0.f;
    if (!Get_StaticLodRange(lodLevel, attributeSensitive, count, first, error)) return E_INVALIDARG;
    const auto* lod = Get_StaticLod(attributeSensitive);
    ID3D11Buffer* buffers[] = { m_pVB.Get(), instanceBuffer };
    const uint32_t strides[] = { m_iVertexStride, stride }, offsets[] = { 0u, byteOffset };
    m_pContext->IASetVertexBuffers(0u, 2u, buffers, strides, offsets);
    m_pContext->IASetIndexBuffer(lodLevel ? lod->Get_IndexBuffer() : m_pIB.Get(), m_eIndexFormat, 0u);
    m_pContext->IASetPrimitiveTopology(m_ePrimitiveTopology);
    m_pContext->DrawIndexedInstancedIndirect(args, argsByteOffset);
    if (auto* profiler = CGameInstance::Get().Get_Profiler())
    {
        profiler->Record_GeometryIndirectDraw(args, argsByteOffset);
        profiler->Add_Counter(EProfilerCounter::DrawCalls);
        profiler->Add_Counter(EProfilerCounter::InstancedDrawCalls);
        profiler->Add_Counter(EProfilerCounter::IndirectDrawCalls);
        // 실제 생존 개수는 GPU 소유다. 용량 상한을 실측 indices/instances로 둔갑시키지 않는다.
        profiler->Add_Counter(EProfilerCounter::IndirectIndexUpperBound,
            uint64_t(count) * ((instanceDesc.ByteWidth - byteOffset) / stride));
    }
    return S_OK;
}

// 정점별 원본 배치 번호와 선택적 원경 index를 불변 버퍼로 만든다. 실패하면 기존 cluster 참조를 유지한다.
HRESULT CMesh::Prepare_StaticClusterStreams(std::span<const uint32_t> sourceIndices,
    std::span<const uint32_t> farIndices)
{
    if (m_iVertexStride != sizeof(VTXMESH) || sourceIndices.size() != m_iNumVertices ||
        sourceIndices.empty() || sourceIndices.size() > UINT32_MAX / sizeof(uint32_t) ||
        farIndices.size() > UINT32_MAX / sizeof(uint32_t) || farIndices.size() % 3u != 0u ||
        std::any_of(farIndices.begin(), farIndices.end(),
            [&](uint32_t index) { return index >= m_iNumVertices; })) return E_INVALIDARG;
    const auto create = [&](std::span<const uint32_t> values, UINT flags, ComPtr<ID3D11Buffer>& output)
    {
        D3D11_BUFFER_DESC desc{};
        desc.ByteWidth = static_cast<UINT>(values.size_bytes());
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.BindFlags = flags;
        D3D11_SUBRESOURCE_DATA data{};
        data.pSysMem = values.data();
        return m_pDevice->CreateBuffer(&desc, &data, &output);
    };
    ComPtr<ID3D11Buffer> sources, reduced;
    HRESULT result = create(sourceIndices, D3D11_BIND_VERTEX_BUFFER, sources);
    if (FAILED(result)) return result;
    if (!farIndices.empty() && FAILED(result = create(farIndices, D3D11_BIND_INDEX_BUFFER, reduced))) return result;
    m_ClusterSourceIndices = std::move(sources);
    m_ClusterFarIndices = std::move(reduced);
    m_iClusterFarIndexCount = static_cast<uint32_t>(farIndices.size());
    return S_OK;
}

// CModel이 준비한 합성 geometry에 원본 번호 stream을 연결해 near 또는 far 삼각형을 제출한다.
HRESULT CMesh::Render_StaticCluster(bool_t farGeometry)
{
    if (!m_pVB || !m_pIB || !m_ClusterSourceIndices || m_iNumIndices == 0u ||
        Has_MorphBaseVertices()) return E_INVALIDARG;
    const bool reduced = farGeometry && m_ClusterFarIndices && m_iClusterFarIndexCount > 0u;
    const uint32_t count = reduced ? m_iClusterFarIndexCount : m_iNumIndices;
    ID3D11Buffer* buffers[] = { m_pVB.Get(), m_ClusterSourceIndices.Get() };
    const UINT strides[] = { sizeof(VTXMESH), sizeof(uint32_t) };
    const UINT offsets[] = { 0u, 0u };
    m_pContext->IASetVertexBuffers(0u, 2u, buffers, strides, offsets);
    m_pContext->IASetIndexBuffer(reduced ? m_ClusterFarIndices.Get() : m_pIB.Get(), DXGI_FORMAT_R32_UINT, 0u);
    m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    if (auto* profiler = CGameInstance::Get().Get_Profiler())
    {
        profiler->Add_Counter(EProfilerCounter::DrawCalls);
        profiler->Add_Counter(EProfilerCounter::Indices, count);
    }
    m_pContext->DrawIndexed(count, 0u, 0);
    if (auto* profiler = CGameInstance::Get().Get_Profiler())
        profiler->Record_MeshSubmitted(this, count, 1u, m_szName, m_iNumVertices, m_iMaterialIndex);
    return S_OK;
}

// Assimp 정적 정점에 모델 사전 변환을 한 번 적용한 뒤 GPU 버퍼로 복사한다.
HRESULT CMesh::Ready_VertexBuffer_NonAnim(const aiMesh* pAIMesh, fmatrix_t PreTransformMatrix)
{
	D3D11_BUFFER_DESC		VertexBufferDesc{};
	VertexBufferDesc.ByteWidth = m_iVertexStride * m_iNumVertices;
	VertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	VertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VertexBufferDesc.CPUAccessFlags = 0;
	VertexBufferDesc.MiscFlags = 0;
	VertexBufferDesc.StructureByteStride = m_iVertexStride;

	unique_ptr<VTXMESH[]> pVertices = make_unique<VTXMESH[]>(m_iNumVertices);

	for (uint32_t i = 0; i < m_iNumVertices; i++)
	{
		memcpy(&pVertices[i].vPosition, &pAIMesh->mVertices[i], sizeof(float3_t));
		XMStoreFloat3(&pVertices[i].vPosition,
			XMVector3TransformCoord(XMLoadFloat3(&pVertices[i].vPosition), PreTransformMatrix));

		memcpy(&pVertices[i].vNormal, &pAIMesh->mNormals[i], sizeof(float3_t));
		XMStoreFloat3(&pVertices[i].vNormal,
			XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vNormal), PreTransformMatrix));

		memcpy(&pVertices[i].vTangent, &pAIMesh->mTangents[i], sizeof(float3_t));
		XMStoreFloat3(&pVertices[i].vTangent,
			XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vTangent), PreTransformMatrix));

		memcpy(&pVertices[i].vBinormal, &pAIMesh->mBitangents[i], sizeof(float3_t));
		XMStoreFloat3(&pVertices[i].vBinormal,
			XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vBinormal), PreTransformMatrix));

		memcpy(&pVertices[i].vTexcoord, &pAIMesh->mTextureCoords[0][i], sizeof(float2_t));
		if (pAIMesh->HasTextureCoords(1))
			memcpy(&pVertices[i].vTexcoord1, &pAIMesh->mTextureCoords[1][i], sizeof(float2_t));
	}

	D3D11_SUBRESOURCE_DATA			VertexInitialData{};
	VertexInitialData.pSysMem = pVertices.get();

	if (FAILED(m_pDevice->CreateBuffer(&VertexBufferDesc, &VertexInitialData, &m_pVB)))
		return E_FAIL;


	return S_OK;
}

// 메시별 본 이름을 모델 골격 index에 연결한다. 정점은 bind 위치에 두고 이후 shader skinning이 변환한다.
HRESULT CMesh::Ready_VertexBuffer_Anim(const aiMesh* pAIMesh, const vector<shared_ptr<class CBone>>& Bones)
{
	D3D11_BUFFER_DESC		VertexBufferDesc{};
	VertexBufferDesc.ByteWidth = m_iVertexStride * m_iNumVertices;
	VertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	VertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VertexBufferDesc.CPUAccessFlags = 0;
	VertexBufferDesc.MiscFlags = 0;
	VertexBufferDesc.StructureByteStride = m_iVertexStride;

	unique_ptr<VTXANIMMESH[]> pVertices = make_unique<VTXANIMMESH[]>(m_iNumVertices);

	for (uint32_t i = 0; i < m_iNumVertices; i++)
	{
		memcpy(&pVertices[i].vPosition, &pAIMesh->mVertices[i], sizeof(float3_t));
		memcpy(&pVertices[i].vNormal, &pAIMesh->mNormals[i], sizeof(float3_t));		
		memcpy(&pVertices[i].vTangent, &pAIMesh->mTangents[i], sizeof(float3_t));		
		memcpy(&pVertices[i].vBinormal, &pAIMesh->mBitangents[i], sizeof(float3_t));		
		memcpy(&pVertices[i].vTexcoord, &pAIMesh->mTextureCoords[0][i], sizeof(float2_t));
		if (pAIMesh->HasTextureCoords(1))
			memcpy(&pVertices[i].vTexcoord1, &pAIMesh->mTextureCoords[1][i], sizeof(float2_t));
		if (pAIMesh->HasTextureCoords(2))
			memcpy(&pVertices[i].vTexcoord2, &pAIMesh->mTextureCoords[2][i], sizeof(float2_t));
	}

	m_iNumBones = pAIMesh->mNumBones;



	for (uint32_t i = 0; i < m_iNumBones; i++)
	{
		aiBone* pAIBone = pAIMesh->mBones[i];
		int32_t	iTotalBoneIndex = { -1 };
		float4x4_t	OffsetMatrix = {};

		memcpy(&OffsetMatrix, &pAIBone->mOffsetMatrix, sizeof(float4x4_t));
		XMStoreFloat4x4(&OffsetMatrix, XMMatrixTranspose(XMLoadFloat4x4(&OffsetMatrix)));


		m_OffsetMatrices.push_back(OffsetMatrix);


		auto	iter = find_if(Bones.begin(), Bones.end(), [&](shared_ptr<class CBone> pBone)->bool_t {
			++iTotalBoneIndex;
			return pBone->Compare_Name(pAIBone->mName.C_Str());
			});

		if (iter == Bones.end())
			return E_FAIL;

		m_BoneIndices.push_back(iTotalBoneIndex);

		// pAIBone->mNumWeights : 이 뼈가 몇개 정점에게 영향을 주는가? 
		for (uint32_t j = 0; j < pAIBone->mNumWeights; j++)
		{
			// pAIBone->mWeights[j].mVertexId : i번째 뼈가 영향을 주는 정점들중 j번째 영향을 주는 정점의 인덱스 
			if(0.f == pVertices[pAIBone->mWeights[j].mVertexId].vBlendWeights.x)
			{ 
				pVertices[pAIBone->mWeights[j].mVertexId].vBlendIndices.x = i;
				pVertices[pAIBone->mWeights[j].mVertexId].vBlendWeights.x = pAIBone->mWeights[j].mWeight;
			}

			else if (0.f == pVertices[pAIBone->mWeights[j].mVertexId].vBlendWeights.y)
			{
				pVertices[pAIBone->mWeights[j].mVertexId].vBlendIndices.y = i;
				pVertices[pAIBone->mWeights[j].mVertexId].vBlendWeights.y = pAIBone->mWeights[j].mWeight;
			}

			else if (0.f == pVertices[pAIBone->mWeights[j].mVertexId].vBlendWeights.z)
			{
				pVertices[pAIBone->mWeights[j].mVertexId].vBlendIndices.z = i;
				pVertices[pAIBone->mWeights[j].mVertexId].vBlendWeights.z = pAIBone->mWeights[j].mWeight;
			}

			else if (0.f == pVertices[pAIBone->mWeights[j].mVertexId].vBlendWeights.w)
			{
				pVertices[pAIBone->mWeights[j].mVertexId].vBlendIndices.w = i;
				pVertices[pAIBone->mWeights[j].mVertexId].vBlendWeights.w = pAIBone->mWeights[j].mWeight;
			}
		}
	}

	// 본 weight가 없는 부착 메시도 같은 이름의 모델 본에 연결할 palette 항목을 마련한다.
	if (0 == m_iNumBones)
	{
		m_iNumBones = 1;
		int32_t	iTotalBoneIndex = { -1 };
		float4x4_t	OffsetMatrix = {};

		auto	iter = find_if(Bones.begin(), Bones.end(), [&](shared_ptr<class CBone> pBone)->bool_t {
			++iTotalBoneIndex;
			return pBone->Compare_Name(m_szName);
		});

		if (iter == Bones.end())
			return E_FAIL;

		m_BoneIndices.push_back(iTotalBoneIndex);

		XMStoreFloat4x4(&OffsetMatrix, XMMatrixIdentity());

		m_OffsetMatrices.push_back(OffsetMatrix);
	}

	D3D11_SUBRESOURCE_DATA			VertexInitialData{};
	VertexInitialData.pSysMem = pVertices.get();

	if (FAILED(m_pDevice->CreateBuffer(&VertexBufferDesc, &VertexInitialData, &m_pVB)))
		return E_FAIL;


	return S_OK;
}

shared_ptr<CMesh> CMesh::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, MODEL eType, const aiMesh* pAIMesh, const vector<shared_ptr<class CBone>>& Bones, fmatrix_t PreTransformMatrix)
{
	auto pInstance = shared_ptr<CMesh>(new CMesh(pDevice, pContext));

	if (FAILED(pInstance->Initialize_Prototype(eType, pAIMesh, Bones, PreTransformMatrix)))
	{
		MSG_BOX("Failed to Created : CMesh");
		return nullptr;
	}

	return pInstance;
}

shared_ptr<CMesh> CMesh::Create(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext, MODEL eType,
	const MODEL_MESH_DATA& mesh, const MODEL_SKELETON_DATA& skeleton,
	fmatrix_t PreTransformMatrix)
{
	auto pInstance = shared_ptr<CMesh>(new CMesh(pDevice, pContext));
	if (FAILED(pInstance->Initialize_Prototype(
		eType, mesh, skeleton, PreTransformMatrix)))
	{
		MSG_BOX("Failed to Created : CMesh");
		return nullptr;
	}
	return pInstance;
}


bool_t CMesh::Get_MorphBaseVertex(uint32_t iIndex, float3_t& OutPosition, float3_t& OutNormal) const
{
	if (!Has_MorphBaseVertices() || iIndex >= m_iNumVertices)
		return false;
	OutPosition = (*m_pMorphBasePositions)[iIndex];
	OutNormal = (*m_pMorphBaseNormals)[iIndex];
	return true;
}

HRESULT CMesh::Make_VertexBuffer_Unique()
{
	if (m_hasUniqueVertexBuffer)
		return S_OK;
	if (0u == m_iNumVertices || nullptr == m_pVB)
		return E_FAIL;

	const uint32_t iByteWidth = m_iVertexStride * m_iNumVertices;

	/* DEFAULT vertex buffer는 직접 Map할 수 없어 staging으로 복사해 원래 위치·법선을 읽는다. 얼굴 편집을 연 모델에서만 준비한다. */
	D3D11_BUFFER_DESC stagingDesc{};
	stagingDesc.ByteWidth = iByteWidth;
	stagingDesc.Usage = D3D11_USAGE_STAGING;
	stagingDesc.BindFlags = 0;
	stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	stagingDesc.StructureByteStride = m_iVertexStride;

	ComPtr<ID3D11Buffer> pStaging;
	if (FAILED(m_pDevice->CreateBuffer(&stagingDesc, nullptr, &pStaging)))
		return E_FAIL;
	m_pContext->CopyResource(pStaging.Get(), m_pVB.Get());

	D3D11_MAPPED_SUBRESOURCE mapped{};
	if (FAILED(m_pContext->Map(pStaging.Get(), 0, D3D11_MAP_READ, 0, &mapped)) ||
		nullptr == mapped.pData)
	{
		return E_FAIL;
	}

	auto pPositions = make_shared<vector<float3_t>>(m_iNumVertices);
	auto pNormals = make_shared<vector<float3_t>>(m_iNumVertices);
	const uint8_t* pVertices = static_cast<const uint8_t*>(mapped.pData);
	for (uint32_t i = 0; i < m_iNumVertices; ++i)
	{
		/* VTXMESH·VTXANIMMESH 모두 위치 offset은 0, 법선 offset은 12다. */
		const uint8_t* pVertex = pVertices + static_cast<size_t>(i) * m_iVertexStride;
		memcpy(&(*pPositions)[i], pVertex, sizeof(float3_t));
		memcpy(&(*pNormals)[i], pVertex + sizeof(float3_t), sizeof(float3_t));
	}
	m_pContext->Unmap(pStaging.Get(), 0);

	D3D11_BUFFER_DESC vertexBufferDesc{};
	vertexBufferDesc.ByteWidth = iByteWidth;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.StructureByteStride = m_iVertexStride;

	ComPtr<ID3D11Buffer> pUniqueVB;
	if (FAILED(m_pDevice->CreateBuffer(&vertexBufferDesc, nullptr, &pUniqueVB)))
		return E_FAIL;

	/* prototype·다른 복제본과 공유하던 GPU buffer 전체를 복사해 나머지 정점 필드도 그대로 유지한다. */
	m_pContext->CopyResource(pUniqueVB.Get(), m_pVB.Get());

	m_pMorphBasePositions = pPositions;
	m_pMorphBaseNormals = pNormals;
	m_pVB = pUniqueVB;
	m_hasUniqueVertexBuffer = true;
	m_TouchedVertexIndices.clear();
	return S_OK;
}

HRESULT CMesh::Update_Vertices(const vector<uint32_t>& iIndices,
	const vector<float3_t>& Positions, const vector<float3_t>& Normals)
{
	if (!m_hasUniqueVertexBuffer)
		return E_FAIL;
	if (iIndices.size() != Positions.size() || iIndices.size() != Normals.size())
		return E_INVALIDARG;

	for (size_t i = 0; i < iIndices.size(); ++i)
	{
		const uint32_t iVertex = iIndices[i];
		if (iVertex >= m_iNumVertices)
			return E_INVALIDARG;

		const f32_t PositionNormal[6] = {
			Positions[i].x, Positions[i].y, Positions[i].z,
			Normals[i].x, Normals[i].y, Normals[i].z,
		};

		D3D11_BOX Box{};
		Box.left = iVertex * m_iVertexStride;
		Box.right = Box.left + sizeof(PositionNormal);
		Box.top = 0u;
		Box.bottom = 1u;
		Box.front = 0u;
		Box.back = 1u;

		/* 두 정점 형식의 위치·법선은 연속된 24바이트다. 이 구간만 갱신해 tangent·UV·skin 입력은 보존한다. */
		m_pContext->UpdateSubresource(m_pVB.Get(), 0, &Box, PositionNormal, 0, 0);
		m_TouchedVertexIndices.push_back(iVertex);
	}
	return S_OK;
}

HRESULT CMesh::Reset_Vertices()
{
	if (!m_hasUniqueVertexBuffer || m_TouchedVertexIndices.empty())
		return S_OK;
	if (!Has_MorphBaseVertices())
		return E_FAIL;

	for (uint32_t iVertex : m_TouchedVertexIndices)
	{
		const float3_t& BasePosition = (*m_pMorphBasePositions)[iVertex];
		const float3_t& BaseNormal = (*m_pMorphBaseNormals)[iVertex];
		const f32_t PositionNormal[6] = {
			BasePosition.x, BasePosition.y, BasePosition.z,
			BaseNormal.x, BaseNormal.y, BaseNormal.z,
		};

		D3D11_BOX Box{};
		Box.left = iVertex * m_iVertexStride;
		Box.right = Box.left + sizeof(PositionNormal);
		Box.top = 0u;
		Box.bottom = 1u;
		Box.front = 0u;
		Box.back = 1u;

		m_pContext->UpdateSubresource(m_pVB.Get(), 0, &Box, PositionNormal, 0, 0);
	}
	m_TouchedVertexIndices.clear();
	return S_OK;
}

shared_ptr<CPrototype> CMesh::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CMesh>(new CMesh(*this));

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned: CMesh");
		return nullptr;
	}

	return pInstance;
}


// CModel 로드 단계의 선택적 준비다. 같은 정점 배치에서 일반·속성 보존 LOD를 만들고 원본 IB는 유지한다.
HRESULT CMesh::Prepare_StaticLod(const MODEL_MESH_DATA& mesh, fmatrix_t preTransform)
{
    if (mesh.vertexKind != MODEL_VERTEX_KIND::STATIC || mesh.indices.size() < CStaticMeshLod::ATTRIBUTE_MINIMUM_INDICES ||
        mesh.indices.size() > 3u * 1024u * 1024u || Has_MorphBaseVertices()) return S_FALSE;
    try
    {
        auto vertices = mesh.vertices;
        for (size_t i = 0u; i < vertices.size(); ++i)
        {
            auto& v = vertices[i];
            v.color0Rgba8 = mesh.hasColor0 ? mesh.color0Rgba8[i] : 0xffffffffu;
            XMStoreFloat3(&v.vPosition, XMVector3TransformCoord(XMLoadFloat3(&v.vPosition), preTransform));
            XMStoreFloat3(&v.vNormal, XMVector3Normalize(XMVector3TransformNormal(XMLoadFloat3(&v.vNormal), preTransform)));
            XMStoreFloat3(&v.vTangent, XMVector3Normalize(XMVector3TransformNormal(XMLoadFloat3(&v.vTangent), preTransform)));
            XMStoreFloat3(&v.vBinormal, XMVector3Normalize(XMVector3TransformNormal(XMLoadFloat3(&v.vBinormal), preTransform)));
        }
        const HRESULT standard = CStaticMeshLod::Create(m_pDevice.Get(), vertices, mesh.indices, m_StaticLod);
        const HRESULT attributes = CStaticMeshLod::Create(m_pDevice.Get(), vertices, mesh.indices, m_AttributeLod, true);
        if (standard == S_OK || attributes == S_OK) return S_OK;
        return FAILED(standard) ? standard : attributes;
    }
    catch (const std::bad_alloc&) { return E_OUTOFMEMORY; }
    catch (const std::length_error&) { return E_OUTOFMEMORY; }
}
