#pragma once

// 정점·인덱스 버퍼의 COM 수명과 공통 IA 바인딩을 상속한다.
#include "VIBuffer.h"
// 정적 cluster의 원본 번호·원경 index 구간을 복사 없이 입력받는다.
#include <span>

struct aiMesh;

NS_BEGIN(Engine)

struct MODEL_MESH_DATA;
struct MODEL_SKELETON_DATA;
struct MESH_SCREEN_LOD_DESC;
class CStaticMeshLod;

// 모델의 메시 한 개가 사용하는 정점·인덱스 버퍼와 재질 번호·스킨 연결을 보관하고 Draw를 제출한다.
// CVIBuffer의 GPU 소유권을 이어 쓰며, 독립 정점 편집과 LOD·피킹 자료는 필요한 경로에서 따로 준비한다.
class ENGINE_DLL CMesh final : public CVIBuffer
{
private:
	friend class CModel;
	CMesh(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CMesh();

public:
	uint32_t Get_MaterialIndex() const {
		return m_iMaterialIndex;
	}
	uint32_t Get_NumVertices() const {
		return m_iNumVertices;
	}

public:
	// CModel의 Assimp 경로가 호출한다. 메시별 재질 번호와 GPU 버퍼, 스킨 본 연결을 준비한다.
	virtual HRESULT Initialize_Prototype(MODEL eType, const aiMesh* pAIMesh, const vector<shared_ptr<class CBone>>& Bones, fmatrix_t PreTransformMatrix);
	// 디코딩된 CPU geometry를 GPU에 올린다. cluster는 정점 재변환·피킹 자료 생성을 각각 생략할 수 있다.
	HRESULT Initialize_Prototype(MODEL eType, const MODEL_MESH_DATA& mesh,
		const MODEL_SKELETON_DATA& skeleton, fmatrix_t PreTransformMatrix,
        bool_t preserveStaticVertices = false, bool_t buildPickingGeometry = true);
	// Clone 훅이며 추가 입력을 소비하지 않는다. 복사한 GPU 자원은 계속 공유한다.
	virtual HRESULT Initialize(void* pArg) override;

public:
	// CModel이 Bind_Resources를 마친 뒤 draw를 제출하고 제출 geometry를 기록한다.
	virtual HRESULT Render() override;
    // 유효한 LOD가 없으면 원본 index를 사용한다. view의 수명은 이 호출 동안만 필요하다.
    HRESULT Render(const MESH_SCREEN_LOD_DESC* screenLod);
	// 외부 instance 버퍼의 지정 구간을 차용해 두 번째 정점 stream으로 바인딩하고 한 번에 제출한다.
	HRESULT Render_Instanced(ID3D11Buffer* pInstanceBuffer,
		uint32_t iInstanceStride, uint32_t iNumInstances,
		uint32_t iInstanceByteOffset = 0u,
        const MESH_SCREEN_LOD_DESC* screenLod = nullptr);
    // args의 count/firstIndex는 Get_Range와 동일하고 startInstance=0이어야 한다.
    // GPU 소유 instanceCount는 읽지 않는다. 호출자는 compaction 용량과 UAV 해제를 보장한다.
    HRESULT Render_InstancedIndirect(ID3D11Buffer* instanceBuffer, uint32_t stride,
        uint32_t byteOffset, ID3D11Buffer* args, uint32_t argsByteOffset,
        uint32_t lodLevel, bool_t attributeSensitive = false);

public:
	/* Make_VertexBuffer_Unique가 원래 위치·법선을 읽어 보관한 뒤 true다. */
	bool_t Has_MorphBaseVertices() const {
		return nullptr != m_pMorphBasePositions;
	}
	/* 이 instance의 GPU vertex buffer를 독립 복제하고 staging readback으로 morph 기준 위치·법선을 보관한다. GPU 대기가 발생하며 이미 독립 buffer면 재실행하지 않는다. */
	HRESULT Make_VertexBuffer_Unique();
	/* 독립 buffer 준비 뒤 mesh 내부 index의 위치·법선 24바이트만 갱신한다. UV·tangent·skin 입력은 보존한다. */
	// 뒤쪽 index에서 실패해도 앞서 갱신한 정점은 남는다. 일괄 반영이 필요하면 호출 전에 전체 index를 검증한다.
	HRESULT Update_Vertices(const vector<uint32_t>& iIndices,
		const vector<float3_t>& Positions, const vector<float3_t>& Normals);
	/* 수정한 정점을 보관된 morph 기준 위치·법선으로 복원한다. 독립 buffer를 만든 적이 없으면 처리하지 않는다. */
	HRESULT Reset_Vertices();
	/* base + 가중 delta 합성에 쓸 원래 위치·법선을 조회한다. 기준 데이터가 없거나 index가 잘못되면 두 출력을 보존하고 false다. */
	bool_t Get_MorphBaseVertex(uint32_t iIndex, float3_t& OutPosition, float3_t& OutNormal) const;

private:
    // CModel의 cluster 생성이 호출한다. 필요한 보조 GPU 버퍼를 준비한 뒤 기존 참조를 교체한다.
    HRESULT Prepare_StaticClusterStreams(std::span<const uint32_t> sourceIndices,
        std::span<const uint32_t> farIndices);
    // 정점별 원본 번호 stream을 함께 제출한다. 원경 IB가 없으면 원래 삼각형을 사용한다.
    HRESULT Render_StaticCluster(bool_t farGeometry);
    ComPtr<ID3D11Buffer> m_ClusterSourceIndices;
    ComPtr<ID3D11Buffer> m_ClusterFarIndices;
    uint32_t m_iClusterFarIndexCount = 0u;
    // 원본 VB를 유지하고 선택용 IB를 준비한다. 축약 대상이 아니면 S_FALSE이며 기본 mesh는 계속 유효하다.
    HRESULT Prepare_StaticLod(const MODEL_MESH_DATA& mesh, fmatrix_t preTransform);
    // 화면 오차·현재 옵션에 맞는 index 구간을 고른다. 선택 불가·morph 메시에는 LOD0을 돌려준다.
    uint32_t Select_StaticLod(const MESH_SCREEN_LOD_DESC* view,
        uint32_t& indexCount, uint32_t& firstIndex) const;
    std::shared_ptr<CStaticMeshLod> m_StaticLod;
    std::shared_ptr<CStaticMeshLod> m_AttributeLod;
    const CStaticMeshLod* Get_StaticLod(bool_t attributeSensitive) const;
    bool_t Get_StaticLodRange(uint32_t level, bool_t attributeSensitive,
        uint32_t& count, uint32_t& first, f32_t& localError) const;

	char_t					m_szName[MAX_PATH] = {};
	uint32_t				m_iMaterialIndex = {};
	uint32_t				m_iNumBones = {};
	vector<uint32_t>		m_BoneIndices;
	vector<float4x4_t>		m_OffsetMatrices;
	// 한 binary 모델의 mesh는 같은 순서의 inverse-bind palette를 사용한다.
	bool_t m_bUsesSkeletonPalette = false;
    struct BONE_VERTEX_BOUNDS final
    {
        float3_t minimum{}, maximum{};
        bool_t valid = false;
    };
    // 본 영향별 불변 geometry 정보이며 모델 복제본이 mesh와 함께 공유한다.
    vector<BONE_VERTEX_BOUNDS> m_BoneVertexBounds;
    // 애니메이션 경계 계산용 본 영향 범위를 준비한다. 비정상 위치·weight는 기존 경계를 보존한다.
    void Prepare_BoneVertexBounds(const MODEL_MESH_DATA& mesh);
    struct PICK_VERTEX final
    {
        float3_t position{};
        DirectX::XMUINT4 bones{};
        float4_t weights{};
    };
    struct PICK_GEOMETRY final
    {
        struct NODE final
        {
            float3_t minimum{}, maximum{};
            uint32_t first = 0u, count = 0u, end = 0u;
        };
        vector<PICK_VERTEX> vertices;
        vector<uint32_t> indices;
        // 전위 순서 BVH의 subtree 종료 link로 별도 stack 없이 조회한다. 삼각형 번호는 원래 LOD0 index 순서를 유지한다.
        vector<uint32_t> triangles;
        vector<NODE> nodes;
        bool skinned = false;
    };
    // WModel의 불변 geometry를 복제본이 공유한다. GPU readback으로 만들지 않는다.
    shared_ptr<const PICK_GEOMETRY> m_PickGeometry;
    // 정적 삼각형을 BVH로 묶어 피킹 후보를 줄인다. 원래 index 배열과 정점은 바꾸지 않는다.
    static void Prepare_StaticPickGeometry(PICK_GEOMETRY& geometry);
    // cullSign은 world determinant를 반영한 0=양면, +1=CW 앞면의 BACK, -1=FRONT다. local 방향은 단위 벡터다.
    bool_t Try_PickStaticLocal(fvector_t origin, fvector_t direction,
        f32_t maxDistance, f32_t cullSign, f32_t& distance) const;

	/* Make_VertexBuffer_Unique가 읽은 mesh 내부 index별 원래 위치·법선이다. 준비 전에는 null이며 이후 복제본은 shared_ptr로 공유한다. */
	shared_ptr<vector<float3_t>>	m_pMorphBasePositions;
	shared_ptr<vector<float3_t>>	m_pMorphBaseNormals;
	/* Make_VertexBuffer_Unique를 호출한 instance만 true이며 m_pVB가 prototype·다른 복제본과 독립적이다. */
	bool_t					m_hasUniqueVertexBuffer = { false };
	/* 마지막 준비·reset 뒤 변경한 local index 목록이다. Reset_Vertices는 이 정점만 복원한다. */
	vector<uint32_t>		m_TouchedVertexIndices;

private:
	// CModel의 렌더링·피킹 경로가 호출한다. offset * 현재 본 변환을 본 수만큼 마련된 출력 배열에 기록한다.
	void Build_SkinPalette(const vector<shared_ptr<class CBone>>& Bones,
		float4x4_t* pOutMatrices) const;
	HRESULT Ready_VertexBuffer_NonAnim(const aiMesh* pAIMesh, fmatrix_t PreTransformMatrix);
	HRESULT Ready_VertexBuffer_Anim(const aiMesh* pAIMesh, const vector<shared_ptr<class CBone>>& Bones);

public:
	// CModel이 소유할 메시를 만든다. 초기화 실패 시 생성 중인 GPU 참조를 정리하고 nullptr를 반환한다.
	static shared_ptr<CMesh> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, MODEL eType, const aiMesh* pAIMesh, const vector<shared_ptr<class CBone>>& Bones, fmatrix_t PreTransformMatrix);
	static shared_ptr<CMesh> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext, MODEL eType,
		const MODEL_MESH_DATA& mesh, const MODEL_SKELETON_DATA& skeleton,
		fmatrix_t PreTransformMatrix);
	// 기본 복사로 GPU 자원·불변 geometry를 공유한다. 독립 정점 편집은 Make_VertexBuffer_Unique로 준비한다.
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
