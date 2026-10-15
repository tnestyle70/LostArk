#pragma once

#include "Component.h"
#include "Engine_VertexTypes.h"
#include "Assimp/material.h"

#include <array>
#include <filesystem>
#include <span>
#include <set>

//assimp
struct aiScene;
struct aiNode;
namespace Assimp { class Importer; }

NS_BEGIN(Engine)

struct MODEL_ASSET_DATA;
struct MODEL_ANIMATION_DATA;
struct MODEL_MATERIAL_SOURCE;
struct MODEL_MESH_DATA;
struct MODEL_ASSET_LOAD_DESC;
struct MODEL_COLOR_TINT;
struct MODEL_SURFACE_PARAMETERS;
struct MESH_SCREEN_LOD_DESC;
struct MODEL_SOURCE_CHARACTER_PARAMETERS;

// GameObject가 Component로 보유하는 모델이며 메시·재질·골격·애니메이션을 한 렌더 경로로 연결한다.
// Clone은 메시·재질의 참조를 공유하고 본·애니메이션 상태를 복제해 각 객체의 재생 상태를 분리한다.
class ENGINE_DLL CModel final : public CComponent
{
private:
	//device, context를 받는 create할 때 호출되는 생성자
	CModel(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	//prototype이 생성될 때 받는 생성자
	CModel(const CModel& Prototype);

public:
	virtual ~CModel();

public:
	uint32_t Get_NumMeshes() const {
		return m_iNumMeshes;
	}
	uint32_t Get_NumAnimations() const {
		return m_iNumAnimations;
	}
	uint32_t Get_CurrentAnimIndex() const {
		return m_iCurrentAnimIndex;
	}
	bool_t Is_AnimLoop() const {
		return m_isAnimLoop;
	}
	bool_t Is_Skinned() const {
		return MODEL::ANIM == m_eType;
	}
	bool_t Has_Animations() const {
		return !m_Animations.empty();
	}
	const char_t* Get_AnimationName(uint32_t iAnimIndex) const;
	bool_t Get_AnimationProgress(uint32_t iAnimIndex, f32_t& fOutPosition, f32_t& fOutDuration) const;
	f32_t Get_AnimationTickPerSecond(uint32_t iAnimIndex) const;

	void Set_AnimPaused(bool_t isPaused) {
		m_isAnimPaused = isPaused;
	}
	bool_t Is_AnimPaused() const {
		return m_isAnimPaused;
	}
	bool_t Set_AnimTrackPosition(uint32_t iAnimIndex, f32_t fTrackPosition);

	bool_t Has_LocalBounds() const { return m_bHasLocalBounds; }
	const float3_t& Get_LocalBoundsMin() const { return m_vLocalBoundsMin; }
	const float3_t& Get_LocalBoundsMax() const { return m_vLocalBoundsMax; }
    // asset pretransform을 적용한 기준 정점 경계이며 애니메이션 culling 경계와 별개다.
    bool_t Try_GetBindGeometryBounds(float3_t& minimum, float3_t& maximum) const;
    // 현재 WModel skin의 model-root 공간 보수 경계다. palette에 pretransform이 포함되어 호출자는 actor root만 적용한다. 미지원·morph이면 두 출력을 보존한다.
    bool_t Try_GetCurrentPoseBounds(float3_t& minimum, float3_t& maximum) const;
    // rest와 허용된 모든 WModel clip을 덮는 root 원점 구다. pretransform은 한 번 포함하며 미지원·morph·외부 pose는 culling을 확정하지 않는다.
    bool_t Try_GetAnimationEnvelopeRadius(f32_t& radius) const;
    // 현재 렌더 pose의 WModel 삼각형을 world 거리로 조회한다. 경계는 후보 선별용이며 미지원·morph geometry는 피킹하지 않는다.
    bool_t Try_PickCurrentPose(const float4x4_t& world, const float3_t& rayOrigin,
        const float3_t& rayDirection, f32_t& distance) const;
    // 가장 가까운 렌더 submesh도 반환한다. 실패하면 두 출력은 그대로다.
    bool_t Try_PickCurrentPose(const float4x4_t& world, const float3_t& rayOrigin,
        const float3_t& rayDirection, f32_t& distance, uint32_t& meshIndex) const;
    enum class PICK_CULL_MODE { NONE, BACK, FRONT };
    // 불변 WModel LOD0의 이동 표면을 조회한다. 방향은 내부 정규화하며 호출자가 재질·CW cull mode를 고르고 world 반사를 반영한다. 거리는 world 단위이고 miss·미지원 시 보존한다. alpha·shader 변위·애니메이션은 판정하지 않는다.
    bool_t Try_PickStaticSurface(uint32_t meshIndex, const float4x4_t& world,
        const float3_t& rayOrigin, const float3_t& rayDirection, f32_t maxDistance,
        PICK_CULL_MODE cullMode, f32_t& distance) const;
	bool_t Has_SelfConsistentUnauthenticatedGeometryMetadata() const {
		return m_bHasSelfConsistentUnauthenticatedGeometryMetadata;
	}
	uint16_t Get_GeometryFormatVersionMajor() const {
		return m_iGeometryFormatVersionMajor;
	}
	uint16_t Get_GeometryFormatVersionMinor() const {
		return m_iGeometryFormatVersionMinor;
	}
	uint32_t Get_GeometryChannelMask() const {
		return m_iGeometryChannelMask;
	}
	uint32_t Get_GeometryEvidenceFlags() const {
		return m_iGeometryEvidenceFlags;
	}
	f32_t Get_GeometryPreScale() const {
		return m_fGeometryPreScale;
	}
	const array<uint8_t, 32>& Get_GeometryPayloadSha256() const {
		return m_GeometryPayloadSha256;
	}
	const array<uint8_t, 32>& Get_GeometryMetadataIdentitySha256() const {
		return m_GeometryMetadataIdentitySha256;
	}

	matrix_t Get_BoneMatrix(const char_t* pBoneName);
	bool_t Has_Bone(const char_t* pBoneName);
	vector<string> Get_BoneNames() const;

	/* 외부 본 구동자는 animation local을 읽고 수정한 뒤 combined를 갱신한다. 본 index는 모델 수명 동안 고정되며 부모가 먼저 있어 순방향으로 재계산한다. */
	int32_t Find_BoneIndex(const char_t* pBoneName) const;
	int32_t Get_BoneParentIndex(uint32_t iBoneIndex) const;
	bool_t Get_BoneLocalMatrix(uint32_t iBoneIndex, matrix_t& outMatrix) const;
	bool_t Get_BoneRestLocalMatrix(uint32_t iBoneIndex, matrix_t& outMatrix) const;
	bool_t Get_BoneCombinedMatrix(uint32_t iBoneIndex, matrix_t& outMatrix) const;
	/* cursor·현재 palette를 바꾸지 않고 현재 clip을 sample한다. expectedAnimationIndex로 준비 이후 clip 교체 여부를 확인한다. */
	bool_t Sample_CurrentAnimationBoneCombinedMatrices(
		uint32_t iExpectedAnimationIndex,
		f32_t fTrackPositionTicks,
		std::span<const uint32_t> BoneIndices,
		std::span<float4x4_t> OutCombinedMatrices) const;
	/* 과거 action 재생용으로 명시한 경과 시간의 blend pose를 계산한다. cursor·blend 시계·현재 palette는 진행시키지 않는다. */
	bool_t Sample_CurrentAnimationBoneCombinedMatricesAtBlendElapsed(
		uint32_t iExpectedAnimationIndex,
		f32_t fTrackPositionTicks,
		f32_t fBlendElapsedSeconds,
		std::span<const uint32_t> BoneIndices,
		std::span<float4x4_t> OutCombinedMatrices) const;
	/* 이름으로 지정한 clip을 live 모델에 바인딩하지 않고 sample한다. key 없는 본은 rest를 쓰며 이름 누락·중복이면 출력을 보존한다. */
	bool_t Sample_AnimationBoneCombinedMatrices(
		const char_t* pAnimationName,
		f32_t fTrackPositionTicks,
		std::span<const uint32_t> BoneIndices,
		std::span<float4x4_t> OutCombinedMatrices) const;
	// root 억제 전 rest 대비 이동량이다. 반환하는 model 공간 벡터에는 조상 basis·model pretransform이 포함된다.
	bool_t Sample_AnimationRootTranslation(const char_t* pAnimationName,
		f32_t fTrackPositionTicks, uint32_t iRootBoneIndex, int32_t iVerticalAxis,
		f32_t fVerticalScale, float3_t& OutModelTranslation) const;
	struct ROOT_MOTION_SUPPRESSION_STATE final
	{
		const CModel* owner = nullptr;
		int32_t boneIndex = -1, verticalAxis = -1;
		float3_t restTranslation{}, unscaledTranslation{};
		f32_t verticalScale = 1.f;
	};
	ROOT_MOTION_SUPPRESSION_STATE Capture_RootMotionSuppression() const;
	bool_t Configure_RootMotionSuppressionFromRest(uint32_t iRootBoneIndex,
		int32_t iVerticalAxis, f32_t fVerticalScale);
	bool_t Restore_RootMotionSuppression(const ROOT_MOTION_SUPPRESSION_STATE& state);
	// 명시한 clip sample로 렌더 이력과 독립적인 전환을 정의한다. UINT32_MAX는 연결된 weapon clip이 없을 때 불변 rest를 선택한다.
	struct ANIMATION_TRANSITION_POSE final
	{
		uint32_t sourceIndex = UINT32_MAX, targetIndex = UINT32_MAX;
		f32_t sourceTicks = 0.f, targetTicks = 0.f;
		f32_t durationSeconds = 0.f, elapsedSeconds = 0.f, playRate = 1.f;
	};
	bool_t Set_AnimationTransitionPose(const ANIMATION_TRANSITION_POSE& pose);
	// 같은 명시적 전환을 현재 actor pose를 바꾸지 않고 sample한다.
	bool_t Sample_AnimationTransitionBoneCombinedMatrices(
		const ANIMATION_TRANSITION_POSE& pose,
		std::span<const uint32_t> BoneIndices,
		std::span<float4x4_t> OutCombinedMatrices) const;
	const ANIMATION_TRANSITION_POSE* Get_AnimationTransitionPose() const
	{ return m_bExplicitAnimationPose ? &m_ExplicitAnimationPose : nullptr; }
	void Clear_AnimationTransitionPose() { m_bExplicitAnimationPose = false; }
	bool_t Set_BoneLocalMatrix(uint32_t iBoneIndex, fmatrix_t Matrix);
	void Refresh_BoneCombinedMatrices();
	/* 이름이 같은 본은 원본 combined를 받고 장비 전용 본은 자신의 rest local을 부모에 연결한다. 부모부터 계산하며 원본에서 공급한 본 개수를 반환한다. */
	uint32_t Pose_BonesFrom(const CModel& source);
	bool_t Enable_RootMotionSuppression(
		const char_t* pBoneName, int32_t iVerticalAxis);

	// 보존된 root 이동 축만 확대하며 geometry·clip 시간은 바꾸지 않는다.
	bool_t Set_RootMotionVerticalScale(f32_t fScale);
	f32_t Get_RootMotionVerticalScale() const { return m_fRootMotionVerticalScale; }

	void Set_Animation(uint32_t iAnimIndex, bool_t isLoop = false,
		f32_t fBlendSeconds = 0.f) {
		if (iAnimIndex >= m_iNumAnimations)
			return;
		m_bExplicitAnimationPose = false;
		if (iAnimIndex != m_iCurrentAnimIndex)
			Begin_AnimBlend(fBlendSeconds);
		m_isAnimLoop = isLoop;
		m_iCurrentAnimIndex = iAnimIndex;
	}
	bool_t Set_Animation(const char_t* pAnimationName,
		bool_t isLoop = false, f32_t fBlendSeconds = 0.f);
	bool_t Is_AnimBlending() const {
		return m_fBlendElapsed < m_fBlendDuration;
	}
	void Skip_Blend() {
		m_fBlendDuration = 0.f;
		m_fBlendElapsed = 0.f;
	}
	bool_t Start_Animation(uint32_t iAnimIndex, bool_t isLoop = true);
	bool_t Start_Animation(const char_t* pAnimationName,
		bool_t isLoop = true);
	void Stop_Animation();
	void Set_AnimationSpeed(f32_t speed);
	bool_t Update_Animation(f32_t fTimeDelta);
    // clip·loop·blend 시계만 진행한다. 호출자는 렌더·action 변경 전에 Play_Animation(0)으로 palette를 계산하며 반환값은 기존 종료 flag와 같다.
    bool_t Advance_AnimationClock(f32_t fTimeDelta, bool_t applyAnimationSpeed = true);
	uint64_t Get_SkeletonHash() const {
		return m_iSkeletonHash;
	}
	HRESULT Attach_AnimationSet(const CModel& animationSet);
	// 초 단위 source sample은 모델 cursor·현재 pose를 바꾸지 않는다.
	bool_t Sample_AnimationLocalTransforms(const char_t* name, f32_t seconds, vector<float4x4_t>& output) const;
	// 전체 channel을 준비한 뒤 한 번 commit한다. native 이름은 바꾸지 않는다.
	bool_t Install_AuthoredAnimations(const vector<MODEL_ANIMATION_DATA>& animations, string& status);

public:
	virtual HRESULT Initialize_Prototype(MODEL eType, const char_t* pModelFilePath, fmatrix_t PreTransformMatrix);
	HRESULT Initialize_Prototype(MODEL eType,
		const MODEL_ASSET_LOAD_DESC& loadDesc,
		fmatrix_t PreTransformMatrix);
	virtual HRESULT Initialize(void* pArg) override;

public:
	HRESULT Render(uint32_t iMeshIndex);
    // 일반 MapAssetObject/Deploy도 기존 CModel -> CMesh 경로에서 화면 오차 LOD를 소비한다.
    // nullptr, 지원하지 않는 재질/변형, 잘못된 view는 기존 원본 draw로 돌아간다.
    HRESULT Render(uint32_t iMeshIndex, const MESH_SCREEN_LOD_DESC* screenLod);
    HRESULT Render_InstancedIndirect(uint32_t meshIndex, ID3D11Buffer* instanceBuffer,
        uint32_t stride, uint32_t byteOffset, ID3D11Buffer* args, uint32_t argsByteOffset,
        const MESH_SCREEN_LOD_DESC* screenLod = nullptr);
    HRESULT Render_InstancedIndirect(uint32_t meshIndex, ID3D11Buffer* instanceBuffer,
        uint32_t stride, uint32_t byteOffset, ID3D11Buffer* args, uint32_t argsByteOffset,
        uint32_t lodLevel);
	HRESULT Render_Instanced(
		uint32_t iMeshIndex, ID3D11Buffer* pInstanceBuffer,
		uint32_t iInstanceStride, uint32_t iNumInstances,
		uint32_t iInstanceByteOffset = 0u,
        const MESH_SCREEN_LOD_DESC* screenLod = nullptr);
    // 기존 CModel·CMaterial 안의 파생 표시 geometry다. 원본 소유권·피킹은 호출자에게 남기며 실패 시 출력은 보존한다. 원본 재질·geometry·배치·조명 변경 전에는 cluster를 무효화·재생성해야 한다.
    struct STATIC_CLUSTER_SOURCE final
    {
        const CModel* model = nullptr;
        uint32_t meshIndex = 0u;
        uint64_t sourceId = 0u;
        VTXMESHINSTANCE instance{};
    };
    struct STATIC_CLUSTER_OPTIONS final
    {
        f32_t targetRatio = .35f;
        f32_t maximumWorldError = .05f;
        uint32_t maximumVertices = 262144u;
        uint32_t maximumIndices = 1048576u;
        uint32_t maximumSources = 4096u;
    };
    struct STATIC_CLUSTER_STATS final
    {
        uint32_t sourceCount = 0u, materialCount = 0u;
        uint32_t clusterVertices = 0u, sourceIndices = 0u;
        uint32_t nearIndices = 0u, farIndices = 0u;
        f32_t maximumWorldError = 0.f;
        float3_t boundsMin{}, boundsMax{};
        bool_t hasFarGeometry = false;
        bool_t geometryCacheHit = false;
        uint64_t retainedCpuBytes = 0u;
        uint64_t decodedCacheBytes = 0u;
    };
    // Build-stage lifetime only. Reuses decoded WModel inputs across clusters;
    // release the cache after scene preparation to release all retained CPU data.
    struct STATIC_CLUSTER_BUILD_CACHE;
    static shared_ptr<STATIC_CLUSTER_BUILD_CACHE> Create_StaticClusterBuildCache();
    static HRESULT Create_StaticCluster(std::span<const STATIC_CLUSTER_SOURCE> sources,
        const STATIC_CLUSTER_OPTIONS& options, STATIC_CLUSTER_BUILD_CACHE& cache,
        shared_ptr<CModel>& output, STATIC_CLUSTER_STATS& stats);
    bool_t Can_ShareStaticClusterMaterialWith(uint32_t meshIndex,
        const CModel& other, uint32_t otherMeshIndex) const;
    // 일반 재질 바인딩 뒤, 해당 cluster VS pass 전에 source record와 기존 0·소수·8개 조명 bank를 설정한다.
    HRESULT Bind_StaticClusterSources(const shared_ptr<class CShader>& shader) const;
    HRESULT Render_StaticCluster(bool_t farGeometry);
    const STATIC_CLUSTER_STATS* Get_StaticClusterStats() const;
    std::span<const uint64_t> Get_StaticClusterSourceIds() const;
    struct STATIC_CLUSTER_SOURCE_RANGE final
    {
        uint64_t sourceId = 0u;
        uint32_t firstIndex = 0u, indexCount = 0u;
        float3_t boundsMin{}, boundsMax{};
        float maximumWorldError = 0.f;
    };
    std::span<const STATIC_CLUSTER_SOURCE_RANGE> Get_StaticClusterSourceRanges() const;
    // 현재 카메라가 승인한 source만 far index에 패킹하며 실패 시 이전 준비 상태를 유지한다.
    // sourceSlots는 오름차순이고 중복이 없어야 한다. 선택 IB는 clone마다 독립 소유한다.
    HRESULT Prepare_StaticClusterVisibleSources(std::span<const uint32_t> sourceSlots);
    HRESULT Render_StaticClusterVisible();
    uint32_t Get_MeshIndexCount(uint32_t meshIndex) const;

    // pretransform이 적용된 불변 LOD0 위치를 모델 수명 동안 차용한다. 재질 불투명·변형 허용 여부는 scene 호출자가 판정한다.
    struct STATIC_OCCLUSION_MESH final
    {
        const float* positions = nullptr;
        uint32_t vertexCount = 0u, strideBytes = 0u;
        std::span<const uint32_t> indices;
    };
    // 조회 실패는 출력을 보존하며 GPU 자원을 읽지 않는다.
    bool_t Try_GetStaticOcclusionMesh(uint32_t meshIndex, STATIC_OCCLUSION_MESH& output) const;

    // atlas proxy는 bake 전까지 원본 속성을 유지한다. slot 1에는 ATLASUV0 float2·SOURCEINDEX0 uint를 담는다.
    struct STATIC_PROXY_VERTEX final { float2_t atlasUV{}; uint32_t sourceIndex = 0u; };
    static_assert(sizeof(STATIC_PROXY_VERTEX) == 12u);
    struct STATIC_PROXY_METADATA final { uint32_t sourceFlags = 0u, stateFlags = 0u; };
    static_assert(sizeof(STATIC_PROXY_METADATA) == 8u);
    struct STATIC_PROXY_OPTIONS final
    {
        uint32_t atlasResolution = 2048u, padding = 8u, maximumSources = 256u;
        f32_t texelsPerUnit = 16.f;
        uint64_t maximumGeometryBytes = 64ull * 1024ull * 1024ull;
    };
    struct STATIC_PROXY_SOURCE_RANGE final
    {
        uint64_t sourceId = 0u;
        uint32_t sourceMeshIndex = 0u, firstIndex = 0u, indexCount = 0u;
        float3_t boundsMin{}, boundsMax{};
    };
    struct STATIC_PROXY_STATS final
    {
        uint32_t sourceCount = 0u, vertexCount = 0u, indexCount = 0u;
        uint32_t atlasWidth = 0u, atlasHeight = 0u, chartCount = 0u;
        f32_t texelsPerUnit = 0.f;
        uint64_t geometryBytes = 0u;
        bool_t loadedFromCache = false;
    };
    static HRESULT Create_StaticProxy(std::span<const STATIC_CLUSTER_SOURCE> sources,
        const STATIC_PROXY_OPTIONS& options, STATIC_CLUSTER_BUILD_CACHE& cache,
        shared_ptr<CModel>& output, STATIC_PROXY_STATS& stats);
    // 외부에서 검증한 축소 geometry다. 위치와 TBN에는 원본 model pretransform이 이미
    // 적용되어 있으며 단위는 모델 로컬 m다. atlasUV와 cache 순서 IB를 재생성하지 않는다.
    // span은 생성 호출 동안만 차용하고 성공한 proxy의 GPU/CPU 버퍼가 별도 복사본을 소유한다.
    struct STATIC_PROXY_GEOMETRY final
    {
        std::span<const VTXMESH> vertices;
        std::span<const float2_t> atlasUV;
        std::span<const uint32_t> indices;
    };
    // Client는 하나의 physical model 전체 mesh가 같은 패키지/재질 계약에 대응하는지
    // 먼저 검증한다. Engine은 아래 모델과 span을 저장하지 않고 instance 레코드만 복사한다.
    struct STATIC_PROXY_PLACEMENT_SOURCE final
    {
        const CModel* model = nullptr;
        uint64_t sourceId = 0u;
        VTXMESHINSTANCE instance{};
        std::span<const uint32_t> sourceMeshIndices;
    };
    // 이 range는 원본 mesh 하나가 아니라 placement의 모든 mesh를 원자적으로 대체한다.
    static constexpr uint32_t STATIC_PROXY_WHOLE_MODEL_MESH_INDEX = UINT32_MAX;
    // 한 placement도 허용한다. source별 low geometry를 그대로 복사하므로 선택/가림은
    // 기존 source range 경로를 따른다. xatlas·원본 재질 bake·파일 cache는 호출하지 않는다.
    // S_FALSE는 지원 범위/예산 제외, 실패 HRESULT는 잘못된 입력/자원 생성 실패이며
    // 어느 경우든 output과 stats는 성공 전까지 보존한다. chartCount=0은 외부 UV임을 뜻한다.
    static HRESULT Create_StaticProxy(const STATIC_PROXY_GEOMETRY& geometry,
        std::span<const STATIC_PROXY_PLACEMENT_SOURCE> sources,
        const STATIC_PROXY_OPTIONS& options, shared_ptr<CModel>& output, STATIC_PROXY_STATS& stats);
    const STATIC_PROXY_STATS* Get_StaticProxyStats() const;
    array<uint8_t, 32> Get_StaticProxyGeometryIdentity() const;
    bool_t Can_BakeStaticProxy(uint32_t meshIndex) const;
    std::span<const std::filesystem::path> Get_StaticProxySourcePaths(uint32_t meshIndex) const;
    bool_t Validate_StaticProxySourceFiles(uint32_t meshIndex) const;
    std::span<const STATIC_PROXY_SOURCE_RANGE> Get_StaticProxySourceRanges() const;
    HRESULT Set_StaticProxyMetadata(std::span<const STATIC_PROXY_METADATA> metadata);
    HRESULT Set_StaticProxyAtlases(std::span<const ComPtr<ID3D11ShaderResourceView>> atlases);
    HRESULT Bind_StaticProxySources(const shared_ptr<class CShader>& shader) const;
    HRESULT Bind_StaticProxyAtlases(const shared_ptr<class CShader>& shader,
        std::span<const char* const> shaderNames) const;
    // sourceSlots는 엄격한 오름차순이며 비어 있으면 runtime draw가 없다. 준비 실패 시 이전 선택·GPU buffer를 보존한다.
    HRESULT Prepare_StaticProxyVisibleSources(std::span<const uint32_t> sourceSlots);
    HRESULT Render_StaticProxySource(uint32_t sourceIndex);
    HRESULT Render_StaticProxy();

	// 다중 mesh·wind instance까지 포함한 공유 geometry·재질 상태를 비교한다.
	bool_t Can_ShareStaticInstanceStateWith(const CModel& other) const;

	// 대응하는 불변 BG mesh마다 별도 조명 bank를 공유한다.
	bool_t Can_BatchStaticLightingWith(const CModel& other) const;
	// Lighting identity only; callers retain material/geometry compatibility checks.
	bool_t Has_SameStaticLightingTextures(const CModel& other) const;
	HRESULT Bind_StaticLightingBank(const shared_ptr<class CShader>& shader,
		std::span<const CModel* const> models, uint32_t meshIndex = 0u) const;
    // Validate and bind only this mesh's bank. Multi-mesh callers must first
    // admit the complete candidate with Can_BatchStaticLightingWith. Geometry,
    // transform and current material compatibility are still checked per bind;
    // no prepared pointers or compatibility result survive this call.
    HRESULT Bind_StaticLightingBankMesh(const shared_ptr<class CShader>& shader,
        std::span<const CModel* const> models, uint32_t meshIndex) const;
	/* 호출자가 연속 범위의 동일 draw 상태를 보장한다. 원본 slot은 유지하며 S_FALSE는 batch 미지원, 실패는 출력 handle·기존 cache 보존을 뜻한다. */
	struct ORDERED_STATIC_GEOMETRY_RANGE final
	{
		uint32_t iSourceMesh = 0u, iSourceMaterial = 0u;
		uint32_t iFirstVertex = 0u, iVertexCount = 0u;
		uint32_t iFirstIndex = 0u, iIndexCount = 0u;
	};
	HRESULT Prepare_OrderedStaticGeometry(
		uint32_t iFirstMesh, uint32_t iMeshCount, uint32_t& iOutHandle);
	bool_t Get_OrderedStaticGeometryRanges(uint32_t iHandle,
		std::span<const ORDERED_STATIC_GEOMETRY_RANGE>& OutRanges) const;
	HRESULT Render_OrderedStaticGeometryInstanced(uint32_t iHandle,
		ID3D11Buffer* pInstanceBuffer, uint32_t iInstanceStride,
		uint32_t iNumInstances, uint32_t iInstanceByteOffset = 0u);
	bool_t Play_Animation(f32_t fTimeDelta);
    // World Sequence 준비가 명시적으로 선택한다. local·combined 본과 모델 상태는 독립이며 Debug·Release에 모두 적용된다.
    void Enable_AnimationSampleReuse();
	HRESULT Bind_BoneMatrices(shared_ptr<class CShader> pShader, const char_t* pConstantName, uint32_t iMeshIndex);
	// 복제본 pose·시계를 변경하지 않고 실제 mesh skin palette를 복사한다.
	bool_t Capture_BoneMatrices(uint32_t iMeshIndex, vector<float4x4_t>& outMatrices) const;
	HRESULT Bind_Material(shared_ptr<class CShader> pShader, const char_t* pConstantName, uint32_t iMeshIndex, aiTextureType eType, uint32_t iTextureIndex = 0);
	/* 이름 부분이 일치하는 재질의 texture slot을 override하고 적용 개수를 반환한다. null이면 원본 texture로 복원한다. */
	uint32_t Override_MaterialTexture(
		const char_t* pMaterialNameFragment,
		aiTextureType eType,
		uint32_t iTextureIndex,
		ComPtr<ID3D11ShaderResourceView> pTexture);
	void Clear_MaterialTextureOverrides();
	/* 같은 이름 부분 일치로 염색 가능 재질에만 색을 적용하고 개수를 반환한다. Clear는 원본 색을 복원한다. */
	uint32_t Override_MaterialDyeColor(
		const char_t* pMaterialNameFragment,
		const float4_t& vDiffuse,
		const float4_t& vRegionA);
	void Clear_MaterialDyeColorOverrides();
	/* Hair only; see CMaterial::Set_DyeTwoTone. */
	uint32_t Override_MaterialDyeTwoTone(
		const char_t* pMaterialNameFragment, f32_t fStrength, f32_t fRange);
	/* CMaterial::Set_DiffuseTint의 단순 곱셈 tint다. mask가 필요 없어 모든 재질에 적용할 수 있다. */
	uint32_t Override_MaterialDiffuseTint(
		const char_t* pMaterialNameFragment, const float4_t& vTint);
	/* 이름 부분이 일치하는 native source-character 재질의 상수·texture register를 override하고 적용 개수를 반환한다. */
	uint32_t Override_SourceCharacterConstants(
		const char_t* pMaterialNameFragment,
		const MODEL_SOURCE_CHARACTER_PARAMETERS& parameters);
	uint32_t Override_SourceCharacterTexture(
		const char_t* pMaterialNameFragment, uint32_t iRegister,
		ComPtr<ID3D11ShaderResourceView> pTexture);
	void Clear_SourceCharacterOverrides();
	/* Null when the mesh or its material is out of range. */
	const float4_t* Get_MaterialDiffuseTint(uint32_t iMeshIndex) const;
	bool_t Has_MaterialTexture(uint32_t iMeshIndex, aiTextureType eType, uint32_t iTextureIndex = 0) const;
	/* mesh·재질 범위가 잘못되면 null이다. color mask만 없으면 isEnabled=false인 identity tint를 반환한다. */
	const MODEL_COLOR_TINT* Get_MaterialColorTint(uint32_t iMeshIndex) const;
	HRESULT Bind_SourceSpecialSurface(shared_ptr<class CShader> shader, uint32_t meshIndex);
    HRESULT Bind_SourceLandscapeSurface(shared_ptr<class CShader> shader, uint32_t meshIndex);
	HRESULT Bind_SurfaceLighting(shared_ptr<class CShader> shader, uint32_t meshIndex);
	HRESULT Bind_SourceCharacter(shared_ptr<class CShader> shader, uint32_t meshIndex);
	HRESULT Bind_SourceCharacterForwardLight(shared_ptr<class CShader> shader, uint32_t meshIndex);
	const MODEL_SURFACE_PARAMETERS* Get_MaterialSurface(uint32_t iMeshIndex) const;
	// 잘못된 재질 참조도 정적 texture 가정을 사용할 수 없는 경우로 처리한다.
	bool_t Has_MaterialTextureOverrides(uint32_t iMeshIndex) const;
	HRESULT Bind_SurfaceTexture(shared_ptr<class CShader> pShader,
		const char_t* pConstantName, uint32_t iMeshIndex, aiTextureType eType);
	// mesh 순서와 독립적으로 보존된 원본 재질 slot을 반환한다.
	bool_t Try_GetSourceMaterialIndex(uint32_t iMeshIndex, uint32_t& iOutMaterialIndex) const;
	const string& Get_MaterialName(uint32_t iMeshIndex) const;
	uint64_t Get_MaterialNameHash(uint32_t iMeshIndex) const;

public:
	/* morph 정점은 WModel SUBMESH_DESC 순서의 mesh index와 그 mesh 내부 vertex index로 지정한다. 독립 buffer 준비를 호출한 모델에서만 기준 정점 비용이 발생한다. */
	uint32_t Get_MeshCount() const {
		return m_iNumMeshes;
	}
	uint32_t Get_MeshVertexCount(uint32_t iMeshIndex) const;
	bool_t Has_MorphBaseVertices(uint32_t iMeshIndex) const;
	// 기존 불변 mesh가 screen-space LOD를 사용할 수 있을 때만 true다.
	bool_t Has_StaticMeshLod(uint32_t iMeshIndex) const;
    enum class STATIC_MESH_LOD_SUPPORT : uint32_t
    {
        AVAILABLE, INVALID_MESH, SKINNED, MORPHED, WIND_DEFORMATION,
        SOURCE_PROGRAM, RENDER_MODE, UNSUPPORTED_FAMILY, NO_DERIVED_RANGE
    };
    STATIC_MESH_LOD_SUPPORT Get_StaticMeshLodSupport(uint32_t meshIndex) const;
    bool_t Can_UseStaticMeshLod(uint32_t meshIndex) const;
    // 현재 재질에 맞는 불변 IB 범위다. 실패하면 출력은 변경하지 않는다.
    bool_t Get_StaticMeshDrawRange(uint32_t meshIndex, const MESH_SCREEN_LOD_DESC* view,
        uint32_t& count, uint32_t& firstIndex) const;
    bool_t Get_StaticMeshLodRange(uint32_t meshIndex, uint32_t level,
        uint32_t& count, uint32_t& firstIndex, f32_t& localError) const;
	// 실제 instance draw와 같은 CPU LOD 선택이다. 잘못된 view는 LOD 0을 유지한다.
	uint32_t Get_StaticMeshLodLevel(uint32_t iMeshIndex, const MESH_SCREEN_LOD_DESC* view) const;
	// Render_Instanced와 같은 범위를 고르며 view 누락·오류 시 원래 index 수를 유지한다.
	uint32_t Get_StaticMeshSelectedIndexCount(uint32_t iMeshIndex, const MESH_SCREEN_LOD_DESC* view = nullptr) const;
	bool_t Get_MorphBaseVertex(uint32_t iMeshIndex, uint32_t iVertexIndex,
		float3_t& OutPosition, float3_t& OutNormal) const;
	/* 해당 복제본의 mesh를 갱신하기 전에 한 번 호출한다. 이미 독립 buffer면 처리하지 않는다. */
	HRESULT Make_MeshVertexBuffer_Unique(uint32_t iMeshIndex);
	HRESULT Update_Mesh_Vertices(uint32_t iMeshIndex, const vector<uint32_t>& iVertexIndices,
		const vector<float3_t>& Positions, const vector<float3_t>& Normals);
	/* 이 mesh에서 수정한 정점을 morph 가중치 0의 기준 상태로 복원한다. */
	HRESULT Reset_Mesh_Vertices(uint32_t iMeshIndex);

private:
	const aiScene*						m_pAIScene = { nullptr };
	/* Assimp 경로를 실제 사용하는 모델에만 importer를 만든다. WModel 전용 runtime 모델마다 불필요한 importer registry를 만들지 않는다. */
	unique_ptr<Assimp::Importer>		m_pImporter;

private:
	MODEL								m_eType = { MODEL::END };
	uint32_t							m_iNumMeshes = {};
	vector<shared_ptr<class CMesh>>		m_Meshes;
	shared_ptr<const MODEL_MATERIAL_SOURCE> m_pMaterialSource;
	float4x4_t							m_PreTransformMatrix = {};
	bool_t m_bRetainOrderedStaticGeometry = false;
	shared_ptr<const vector<MODEL_MESH_DATA>> m_pOrderedStaticGeometrySource;
    struct STATIC_CLUSTER_STORAGE;
    shared_ptr<const STATIC_CLUSTER_STORAGE> m_StaticCluster;
    struct STATIC_CLUSTER_SELECTION;
    std::shared_ptr<STATIC_CLUSTER_SELECTION> m_StaticClusterSelection;
    struct STATIC_PROXY_STORAGE;
    shared_ptr<const STATIC_PROXY_STORAGE> m_StaticProxy;
    vector<ComPtr<ID3D11ShaderResourceView>> m_StaticProxyAtlases;
    ComPtr<ID3D11ShaderResourceView> m_StaticProxyMetadata;
    ComPtr<ID3D11Buffer> m_StaticProxyVisibleIndices;
    vector<uint32_t> m_StaticProxyVisibleSources;
    uint32_t m_iStaticProxyVisibleIndexCount = 0u;
    bool_t m_bStaticProxyVisiblePrepared = false;

	struct ORDERED_STATIC_GEOMETRY final
	{
		uint32_t iHandle = 0u, iFirstMesh = 0u, iMeshCount = 0u;
		shared_ptr<class CMesh> pMesh;
		vector<ORDERED_STATIC_GEOMETRY_RANGE> Ranges;
	};
	vector<shared_ptr<const ORDERED_STATIC_GEOMETRY>> m_OrderedStaticGeometry;
	uint32_t m_iNextOrderedStaticGeometryHandle = 0u;
	bool_t Can_UseOrderedStaticGeometry(uint32_t iFirstMesh, uint32_t iMeshCount) const;

	uint32_t							m_iNumMaterials = {};
	vector<shared_ptr<class CMaterial>>	m_Materials;

	vector<shared_ptr<class CBone>>		m_Bones;
	// pose는 모델 복제본이 소유하고 mesh geometry는 공유한다.
	struct SKIN_PALETTE final
	{
		vector<float4x4_t> Matrices;
		uint64_t iPoseRevision = 0u;
	};
	vector<SKIN_PALETTE> m_SkinPalettes;
	uint64_t m_iBonePoseRevision = 1u;
	void Invalidate_SkinPalettes();
	/* 매 프레임 같은 골격 쌍을 연결하는 이름 대응 cache다. -1은 원본에 없는 본이다. */
	vector<int32_t>						m_SourcePoseBoneIndices;
	const CModel*						m_pSourcePoseModel = { nullptr };
	weak_ptr<const CPrototype> m_SourcePoseOwner;
	// 같은 프레임의 local 편집까지 포함해 양쪽 pose가 복사 당시와 같은지 확인한다.
	uint64_t m_iSourcePoseRevision = 0u;
	uint64_t m_iCopiedPoseRevision = 0u;
	uint32_t m_iSourcePoseSuppliedBones = 0u;
	vector<float4x4_t>					m_BoneRestLocalTransforms;
	uint64_t							m_iSkeletonHash = {};

	uint32_t								m_iCurrentAnimIndex = {};
	uint32_t								m_iNumAnimations = {};
	vector<shared_ptr<class CAnimation>>	m_Animations;
	std::set<string> m_AuthoredAnimationNames;
	bool_t									m_isAnimLoop = { false };
	bool_t									m_isAnimPaused = { false };
	f32_t									m_fAnimationSpeed = { 1.f };
	int32_t									m_iRootMotionBoneIndex = { -1 };
	int32_t									m_iRootMotionVerticalAxis = { -1 };
	float3_t								m_vRootMotionRestTranslation = {};
	float3_t m_vRootMotionUnscaledTranslation = {};
	f32_t m_fRootMotionVerticalScale = 1.f;
	vector<float4x4_t>						m_BlendFromPose;
	f32_t									m_fBlendElapsed = {};
	f32_t									m_fBlendDuration = {};
	bool_t									m_bHasLocalBounds = { false };
	float3_t								m_vLocalBoundsMin = {};
	float3_t								m_vLocalBoundsMax = {};
    mutable bool_t m_bAnimationEnvelopeAttempted = false;
    mutable f32_t m_fAnimationEnvelopeRadius = -1.f;
    bool_t m_bAnimationEnvelopeExternalPose = false;
    bool_t Build_AnimationEnvelopeRadius(f32_t& radius) const;
    bool_t m_bHasBindGeometryBounds = false;
    float3_t m_vBindGeometryBoundsMin{}, m_vBindGeometryBoundsMax{};
	bool_t									m_bHasSelfConsistentUnauthenticatedGeometryMetadata = { false };
	uint16_t								m_iGeometryFormatVersionMajor = {};
	uint16_t								m_iGeometryFormatVersionMinor = {};
	uint32_t								m_iGeometryChannelMask = {};
	uint32_t								m_iGeometryEvidenceFlags = {};
	f32_t									m_fGeometryPreScale = { 1.f };
	array<uint8_t, 32>						m_GeometryPayloadSha256 = {};
	array<uint8_t, 32>						m_GeometryMetadataIdentitySha256 = {};

private:
	bool_t Sample_BoneCombinedMatricesForAnimation(
		uint32_t iExpectedAnimationIndex,
		f32_t fTrackPositionTicks,
		bool_t bUseCurrentPoseAndBlend,
		f32_t fBlendElapsedSeconds,
		std::span<const uint32_t> BoneIndices,
		std::span<float4x4_t> OutCombinedMatrices) const;
	bool_t Build_AnimationTransitionPose(const ANIMATION_TRANSITION_POSE& pose,
		vector<float4x4_t>& local, vector<float4x4_t>& combined, float3_t* unscaledRoot = nullptr) const;
	ANIMATION_TRANSITION_POSE m_ExplicitAnimationPose;
	bool_t m_bExplicitAnimationPose = false;
	void Begin_AnimBlend(f32_t fBlendSeconds);
	void Restore_UnscaledRootVertical(float4x4_t& Local) const;
	void Apply_RootMotionTranslation(float4x4_t& Local) const;
	void Update_AnimBlend(f32_t fTimeDelta);
	HRESULT Ready_Meshes();
	HRESULT Ready_Materials(const char_t* pModelFilePath);
	HRESULT Ready_Bones(const aiNode* pAINode, int32_t iParentBoneIndex = -1);
	HRESULT Ready_Animations();
	HRESULT Ready_BinaryModel(const char_t* pModelFilePath);
    const MESH_SCREEN_LOD_DESC* Resolve_StaticMeshLodView(uint32_t meshIndex,
        const MESH_SCREEN_LOD_DESC* input, MESH_SCREEN_LOD_DESC& resolved) const;
    bool_t Uses_AttributeSensitiveLod(uint32_t meshIndex) const;
	HRESULT Ready_BinaryModel(const MODEL_ASSET_LOAD_DESC& loadDesc);
	HRESULT Apply_MaterialOverrides(MODEL_MATERIAL_SOURCE& source, const MODEL_ASSET_LOAD_DESC& loadDesc);
	HRESULT Ready_Meshes(const MODEL_ASSET_DATA& asset);
	HRESULT Ready_Materials(const MODEL_ASSET_DATA& asset);
	HRESULT Ready_Bones(const MODEL_ASSET_DATA& asset);
	HRESULT Ready_Animations(const MODEL_ASSET_DATA& asset);
	void Reset_LocalBounds();
    void Include_BindGeometryPosition(fvector_t position);
	void Include_LocalPosition(fvector_t vPosition);

public:
	static unique_ptr<CModel> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, MODEL eType, const char_t* pModelFilePath, fmatrix_t PreTransformMatrix,
		bool_t bRetainOrderedStaticGeometry = false);
	static unique_ptr<CModel> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		MODEL eType,
		const MODEL_ASSET_LOAD_DESC& loadDesc,
		fmatrix_t PreTransformMatrix,
		bool_t bRetainOrderedStaticGeometry = false);
    // 불변 GPU geometry를 재사용하고 pose·재질은 독립적으로 복제한다. load identity는 prototype과 같아야 하며 geometry를 재지정하지 않는다.
    static unique_ptr<CModel> Create_MaterialVariant(const CModel& prototype,
        const MODEL_ASSET_LOAD_DESC& loadDesc);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
	void Free();
};

NS_END
