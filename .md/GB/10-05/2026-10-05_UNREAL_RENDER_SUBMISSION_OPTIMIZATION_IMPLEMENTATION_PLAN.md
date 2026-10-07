# Unreal 참조 렌더 제출 최적화 구현 계획

## G00. 실제 범위와 현재 구조

UE 5.8.3 release의 현재 source checkout은 tracked 224,004개 중 missing 0개다. Niagara와 Lumen 소스도 있으며 shallow clone은 과거 Git 이력의 제한이다. 소스 읽기에 CMake는 필요하지 않다. Unreal은 UBT와 GenerateProjectFiles를 사용한다.

이 변경은 현재 LostArk 렌더 경로의 반복 CPU 제출 비용을 줄인다. 기존 LOD, 가시성, 재질, 조명과 셰이더 program을 바꾸지 않는다. Lumen/Nanite/TSR 전체 이식이나 새로운 두 번째 renderer를 구현한 것으로 설명하지 않는다.

현재 정적 mesh LOD는 4,096 triangle 이상에서 meshoptimizer가 원본 정점을 공유하는 최대 두 추가 index range를 준비한다. 19개 속성의 오차에 UV 가중치를 높이고 mesh 경계를 잠근다. Select_Range가 보수적 화면 오차로 선택한다. Bern은 공간 분할, Layer BVH, final-camera visibility, CPU occlusion, 작은 소품의 거리/pixel 정책, exact instance 및 RNM bank 제출을 사용한다. atlas HLOD는 기본 비활성이다. shadow는 별도의 기존 원본 index 계약을 유지한다.

## G01. 큰 raw 상수 캐시 후보의 제외

Shader.h/Shader.cpp의 65~4096byte raw 캐시는 정확성 검사를 통과했지만 최종 적용에서 제외한다. 실제 Release의 변경 입력 경로에서는 추가 비교·복사 비용이 생겼고, 기존 재질 경로는 동일 light 입력을 이미 중복 제거한다. 모든 바인딩을 고정한 반복 측정의 이득만으로 공통 shader setter를 바꾸지 않는다. 기존 작은 값 캐시와 program variant/revision 경로를 유지한다. 후보의 검사·측정 근거는 RESULT G01에 남긴다.

## G02. Model.h / Model.cpp / MapStaticBatchObject.cpp의 RNM bank 검증

후보 수집의 Can_BatchStaticLightingWith 전체 모델 검증과 기존 public Bind_StaticLightingBank를 유지한다. 실제 mesh draw에는 현재 mesh를 안전하게 검증하는 Bind_StaticLightingBankMesh를 사용한다. NONANIM, device/context, pretransform/prescale, mesh count/current shared mesh/morph, material index와 현재 material의 전체 비조명 입력·override·SRV 호환성을 검사한다. 프레임 간 불변이라고 가정하는 캐시는 만들지 않는다.

각 mesh 바인딩이 모든 다른 mesh의 재질까지 반복 검사하던 K*M*M 비용을 현재 mesh의 K개 재질 검사로 줄인다. stack array8을 사용하고 draw마다 heap을 할당하지 않는다. 원본 instance payload, LOD range, shader, RNM SRV 순서와 failure-before-draw/after-draw 처리는 유지한다. 실제 함수의 성공/실패 입력과 바인딩 기록을 비교하고, M=1 및 다중 mesh에서 검증 횟수와 CPU 비용을 구분한다.

## G03. GPU local-light 후보의 채택 조건

기존 source-character용 screen clip을 ordinary point/spot에도 확장하는 후보는 별도 실제 compiled-shader parity와 PS invocation 검사 후에만 적용한다. near-plane/비정상 projection은 fullscreen fallback을 유지한다. 과거 Bern 캡처의 ordinary light는 near-plane에 걸려 이 후보의 절감이0이었던 증거가 있으므로 Bern FPS 해결책으로 단정하지 않는다. 검사 전 해당 C++ flag는 이 계획의 적용 코드에 포함하지 않는다.

## G04. 프로젝트 등록과 검증

새 제품 C++/HLSL 파일이 없어 vcxproj/filters 항목 추가는 없다. 기존 인코딩과 CRLF를 보존한다. PLAN에 아래 후보 전체 코드를 먼저 기록한 뒤 작업 브랜치에 적용한다. 변경 함수의 집중 검증, 해당 Engine/Client TU 및 정상 Product Build, git diff --check를 수행한다. 기존 사용자 또는 다른 세션의 변경, Resources, 저작/게시 데이터, 팀장 rendering option은 보존한다. Client/UI 자율 실행은 하지 않으며 사용자 장면·실제 FPS는 별도 판정이다.

Unreal 비교 시작점은 StaticMeshSceneProxy.cpp(GetLOD/GetLODMask), SceneVisibility.cpp(LaunchVisibilityTasks), MeshDrawCommands.cpp(SubmitMeshDrawCommandsRange), DeferredShadingRenderer.cpp(Render), MeshMaterialShader.cpp(ShouldCompilePermutation)이다. 우리 Shader의 program shard와 변경 revision cache, material row sharing도 이미 있는 기능으로 구분한다.

## G05. 적용 파일 전체 코드

### Engine/Public/Model.h

위치: 기존 파일 전체 교체. 기존 미변경 코드를 포함한 적용 후보 전문이다.

```cpp
#pragma once

#include "Component.h"
#include "Engine_VertexTypes.h"
#pragma push_macro("new")
#undef new
#include "Assimp/material.h"
#pragma pop_macro("new")

#include <array>
#include <filesystem>
#include <span>
#include <set>

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

class ENGINE_DLL CModel final : public CComponent
{
private:
	CModel(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
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
    // Reference vertex bounds after asset pretransform; independent of animated culling.
    bool_t Try_GetBindGeometryBounds(float3_t& minimum, float3_t& maximum) const;
    // Conservative current WModel skin bounds in model-root space. The bone
    // palette already includes pretransform; callers apply only the actor root.
    // Unsupported/morphed geometry leaves both outputs unchanged.
    bool_t Try_GetCurrentPoseBounds(float3_t& minimum, float3_t& maximum) const;
    // Conservative root-origin sphere covering rest and every admitted WModel clip.
    // Includes the asset pretransform once; unsupported/morphed/external poses fail open.
    bool_t Try_GetAnimationEnvelopeRadius(f32_t& radius) const;
    // WModel triangle query at the current rendered pose. Bounds are broad phase
    // only; distance is in world units. Unsupported/morphed geometry is not picked.
    bool_t Try_PickCurrentPose(const float4x4_t& world, const float3_t& rayOrigin,
        const float3_t& rayDirection, f32_t& distance) const;
    // Also identify the nearest rendered submesh; both outputs are unchanged on failure.
    bool_t Try_PickCurrentPose(const float4x4_t& world, const float3_t& rayOrigin,
        const float3_t& rayDirection, f32_t& distance, uint32_t& meshIndex) const;
    enum class PICK_CULL_MODE { NONE, BACK, FRONT };
    // Movement surface query over immutable static WModel LOD0 triangles. The
    // caller selects eligible materials and the final rasterizer's CW-front cull
    // mode; world reflection is included when testing triangle winding. No GPU
    // readback, query allocation, alpha test, shader displacement or animation.
    // Direction is normalized internally; distance/maxDistance are world units.
    // Unsupported geometry and misses leave distance unchanged.
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

	/* Secondary-motion seam. A caller that drives bones itself resolves indices
	once, reads what the animation produced this frame, writes its own local
	matrices back, and refreshes the combined matrices before skinning reads
	them. Indices are stable for the model's lifetime and every bone's parent
	comes before it, so one forward pass rebuilds the whole hierarchy. */
	int32_t Find_BoneIndex(const char_t* pBoneName) const;
	int32_t Get_BoneParentIndex(uint32_t iBoneIndex) const;
	bool_t Get_BoneLocalMatrix(uint32_t iBoneIndex, matrix_t& outMatrix) const;
	bool_t Get_BoneRestLocalMatrix(uint32_t iBoneIndex, matrix_t& outMatrix) const;
	bool_t Get_BoneCombinedMatrix(uint32_t iBoneIndex, matrix_t& outMatrix) const;
	/* Samples the currently bound animation without moving its cursor or the
	   live bone palette.  expectedAnimationIndex closes the race where a tool
	   prepared one clip and another owner changed it before the sample. */
	bool_t Sample_CurrentAnimationBoneCombinedMatrices(
		uint32_t iExpectedAnimationIndex,
		f32_t fTrackPositionTicks,
		std::span<const uint32_t> BoneIndices,
		std::span<float4x4_t> OutCombinedMatrices) const;
	/* Historical action playback can prepare future clip poses while the live
	   model is still at the start of its transition.  This variant evaluates the
	   saved blend-from pose at an explicit elapsed time without advancing either
	   the animation cursor, blend clock, or live bone palette. */
	bool_t Sample_CurrentAnimationBoneCombinedMatricesAtBlendElapsed(
		uint32_t iExpectedAnimationIndex,
		f32_t fTrackPositionTicks,
		f32_t fBlendElapsedSeconds,
		std::span<const uint32_t> BoneIndices,
		std::span<float4x4_t> OutCombinedMatrices) const;
	/* Samples one explicitly named clip without binding it to the live model.
	   Unkeyed bones use the immutable skeleton rest pose and the unrelated live
	   transition is not applied. Missing/ambiguous names leave output unchanged. */
	bool_t Sample_AnimationBoneCombinedMatrices(
		const char_t* pAnimationName,
		f32_t fTrackPositionTicks,
		std::span<const uint32_t> BoneIndices,
		std::span<float4x4_t> OutCombinedMatrices) const;
	// Raw root translation relative to immutable rest, before suppression. The
	// returned model-space vector already includes ancestor basis and model pre-transform.
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
	// Explicit clip samples define a transition independently of render history.
	// UINT32_MAX selects the immutable rest pose (for an unmapped weapon clip).
	struct ANIMATION_TRANSITION_POSE final
	{
		uint32_t sourceIndex = UINT32_MAX, targetIndex = UINT32_MAX;
		f32_t sourceTicks = 0.f, targetTicks = 0.f;
		f32_t durationSeconds = 0.f, elapsedSeconds = 0.f, playRate = 1.f;
	};
	bool_t Set_AnimationTransitionPose(const ANIMATION_TRANSITION_POSE& pose);
	// The same explicit transition, sampled without changing the live actor pose.
	bool_t Sample_AnimationTransitionBoneCombinedMatrices(
		const ANIMATION_TRANSITION_POSE& pose,
		std::span<const uint32_t> BoneIndices,
		std::span<float4x4_t> OutCombinedMatrices) const;
	const ANIMATION_TRANSITION_POSE* Get_AnimationTransitionPose() const
	{ return m_bExplicitAnimationPose ? &m_ExplicitAnimationPose : nullptr; }
	void Clear_AnimationTransitionPose() { m_bExplicitAnimationPose = false; }
	bool_t Set_BoneLocalMatrix(uint32_t iBoneIndex, fmatrix_t Matrix);
	void Refresh_BoneCombinedMatrices();
	/* Poses this model's skeleton from another one, matched by bone name, for a worn part that
	rides a body's animation. A bone the source also has takes the source's combined matrix; a
	bone only this model has -- the costume-only chains a hairstyle or a dress adds -- is
	rebuilt from its own rest local onto whichever parent was just posed, so it hangs off the
	animated body instead of collapsing.

	Without this a part with extra bones cannot be drawn from its own palette at all: the body's
	palette is shorter, and every vertex weighted past its end reads a zero matrix. Returns how
	many bones the source supplied, so a caller can tell a matched skeleton from an unrelated
	one. Bones are stored parent-before-child, so one forward pass is enough. */
	uint32_t Pose_BonesFrom(const CModel& source);
	bool_t Enable_RootMotionSuppression(
		const char_t* pBoneName, int32_t iVerticalAxis);

	// Scales only the preserved root translation axis; geometry and clip time stay unchanged.
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
    // Advances the existing clip/loop/blend clocks, leaving the palette untouched.
    // The caller evaluates Play_Animation(0) before rendering or changing an action.
    // Return value is the same finished flag as Play_Animation/Update_Animation.
    bool_t Advance_AnimationClock(f32_t fTimeDelta, bool_t applyAnimationSpeed = true);
	uint64_t Get_SkeletonHash() const {
		return m_iSkeletonHash;
	}
	HRESULT Attach_AnimationSet(const CModel& animationSet);
	// Source sampling never changes the model cursor or live pose. Time is seconds.
	bool_t Sample_AnimationLocalTransforms(const char_t* name, f32_t seconds, vector<float4x4_t>& output) const;
	// Stages every channel before one commit; native names are immutable.
	bool_t Install_AuthoredAnimations(const vector<MODEL_ANIMATION_DATA>& animations, string& status);

public:
	virtual HRESULT Initialize_Prototype(MODEL eType, const char_t* pModelFilePath, fmatrix_t PreTransformMatrix);
	HRESULT Initialize_Prototype(MODEL eType,
		const MODEL_ASSET_LOAD_DESC& loadDesc,
		fmatrix_t PreTransformMatrix);
	virtual HRESULT Initialize(void* pArg) override;

public:
	HRESULT Render(uint32_t iMeshIndex);
	HRESULT Render_Instanced(
		uint32_t iMeshIndex, ID3D11Buffer* pInstanceBuffer,
		uint32_t iInstanceStride, uint32_t iNumInstances,
		uint32_t iInstanceByteOffset = 0u,
        const MESH_SCREEN_LOD_DESC* screenLod = nullptr);
    // A cluster is derived presentation geometry inside this CModel/CMaterial
    // path. Original placement identities, model assets and materials stay owned
    // by the caller. Cluster picking is disabled; pick the original placements.
    // Preparation never changes any source model or output on failure.
    // Caller invalidates/rebuilds derived clusters before changing source material
    // identities, geometry, per-placement transforms or baked-lighting inputs.
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
    // Sets g_MapClusterSources and the existing zero/small/eight lighting bank.
    // Call after ordinary material binding, before the matching cluster VS pass.
    HRESULT Bind_StaticClusterSources(const shared_ptr<class CShader>& shader) const;
    HRESULT Render_StaticCluster(bool_t farGeometry);
    const STATIC_CLUSTER_STATS* Get_StaticClusterStats() const;
    std::span<const uint64_t> Get_StaticClusterSourceIds() const;
    uint32_t Get_MeshIndexCount(uint32_t meshIndex) const;

    // Borrowed LOD0 positions already include the model pre-transform. This is
    // immutable original geometry; it remains valid for this model's lifetime.
    // Material opacity/deformation admission belongs to the scene caller.
    struct STATIC_OCCLUSION_MESH final
    {
        const float* positions = nullptr;
        uint32_t vertexCount = 0u, strideBytes = 0u;
        std::span<const uint32_t> indices;
    };
    // Failed lookup leaves output untouched and never reads GPU resources.
    bool_t Try_GetStaticOcclusionMesh(uint32_t meshIndex, STATIC_OCCLUSION_MESH& output) const;

    // Static atlas proxy: original source attributes remain intact until baking.
    // Slot 1 is STATIC_PROXY_VERTEX (ATLASUV0 float2, SOURCEINDEX0 uint).
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
    // sourceSlots must be strictly increasing. Empty means no runtime draw.
    // Failed preparation leaves the last complete selection and GPU buffer intact.
    HRESULT Prepare_StaticProxyVisibleSources(std::span<const uint32_t> sourceSlots);
    HRESULT Render_StaticProxySource(uint32_t sourceIndex);
    HRESULT Render_StaticProxy();

	// Exact shared geometry/material state, including multi-mesh and wind instances.
	bool_t Can_ShareStaticInstanceStateWith(const CModel& other) const;

	// Corresponding immutable BG meshes share a separate lighting bank per mesh.
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
	/* Preparation is explicit: the caller owns the proof that every source
	   submesh in this contiguous range uses the same effective draw state.
	   Original meshes/material slots remain intact. S_FALSE means this model
	   cannot batch (not retained, skinned, or made mutable for morphing).
	   Failure leaves the output handle and every existing cache entry intact. */
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
    // World Sequence preparation opts in once; local/combined bones and all
    // model state remain independent. Applies in both Debug and Release.
    void Enable_AnimationSampleReuse();
	HRESULT Bind_BoneMatrices(shared_ptr<class CShader> pShader, const char_t* pConstantName, uint32_t iMeshIndex);
	// Copy the actual mesh skin palette without changing this clone's pose or clock.
	bool_t Capture_BoneMatrices(uint32_t iMeshIndex, vector<float4x4_t>& outMatrices) const;
	HRESULT Bind_Material(shared_ptr<class CShader> pShader, const char_t* pConstantName, uint32_t iMeshIndex, aiTextureType eType, uint32_t iTextureIndex = 0);
	/* Repaints one texture slot of every material whose name contains pMaterialNameFragment,
	for the character-creation choices that change a face's look without changing its mesh.
	Matching by name fragment rather than by index keeps the caller out of the material order,
	which differs per class rig. Returns how many materials took it, so a caller can tell an
	empty match from a successful one. A null view restores the authored texture. */
	uint32_t Override_MaterialTexture(
		const char_t* pMaterialNameFragment,
		aiTextureType eType,
		uint32_t iTextureIndex,
		ComPtr<ID3D11ShaderResourceView> pTexture);
	void Clear_MaterialTextureOverrides();
	/* The same match by name fragment, for the dyed colour rather than the texture. Only
	materials that actually dye take it, so the count says whether the choice landed on
	anything. Clear restores what the asset shipped. */
	uint32_t Override_MaterialDyeColor(
		const char_t* pMaterialNameFragment,
		const float4_t& vDiffuse,
		const float4_t& vRegionA);
	void Clear_MaterialDyeColorOverrides();
	/* Hair only; see CMaterial::Set_DyeTwoTone. */
	uint32_t Override_MaterialDyeTwoTone(
		const char_t* pMaterialNameFragment, f32_t fStrength, f32_t fRange);
	/* See CMaterial::Set_DiffuseTint. Unlike the dye overrides this takes on any material,
	because a plain multiply needs no mask to ride on. */
	uint32_t Override_MaterialDiffuseTint(
		const char_t* pMaterialNameFragment, const float4_t& vTint);
	/* The same match by name fragment, for a material drawn by a native source-character
	program: the creation screen's skin and make-up choices are that program's own constants
	and texture registers. See CMaterial::Set_SourceCharacterConstants. Only materials on such
	a program take these, so the count says whether the choice landed on anything at all. */
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
	/* Null when the mesh or its material is out of range; identity tint (its
	isEnabled false) when the material simply has no colour mask. */
	const MODEL_COLOR_TINT* Get_MaterialColorTint(uint32_t iMeshIndex) const;
	HRESULT Bind_SourceSpecialSurface(shared_ptr<class CShader> shader, uint32_t meshIndex);
    HRESULT Bind_SourceLandscapeSurface(shared_ptr<class CShader> shader, uint32_t meshIndex);
	HRESULT Bind_SurfaceLighting(shared_ptr<class CShader> shader, uint32_t meshIndex);
	HRESULT Bind_SourceCharacter(shared_ptr<class CShader> shader, uint32_t meshIndex);
	HRESULT Bind_SourceCharacterForwardLight(shared_ptr<class CShader> shader, uint32_t meshIndex);
	const MODEL_SURFACE_PARAMETERS* Get_MaterialSurface(uint32_t iMeshIndex) const;
	// Invalid material references also opt out of callers' static texture assumptions.
	bool_t Has_MaterialTextureOverrides(uint32_t iMeshIndex) const;
	HRESULT Bind_SurfaceTexture(shared_ptr<class CShader> pShader,
		const char_t* pConstantName, uint32_t iMeshIndex, aiTextureType eType);
	// Returns the preserved source material slot, independently of mesh order.
	bool_t Try_GetSourceMaterialIndex(uint32_t iMeshIndex, uint32_t& iOutMaterialIndex) const;
	const string& Get_MaterialName(uint32_t iMeshIndex) const;
	uint64_t Get_MaterialNameHash(uint32_t iMeshIndex) const;

public:
	/* Face MorphTarget application (character-creation base tab). A vertex is addressed as
	(iMeshIndex, iVertexIndex): iMeshIndex indexes this CModel's own m_Meshes -- one CMesh per
	submesh, in the exact order CModel::Ready_Meshes built them in, which is the same order
	the .wmodel's own SUBMESH_DESC table lists them (see
	Tools/CharacterCustomizing/build_face_morph_vertex_map.py's global_to_local(), which
	produces the (meshIndex, localIndex) pairs a .facemorphmap on disk stores). iVertexIndex
	is local to that one CMesh's own vertex buffer, 0..Get_MeshVertexCount(iMeshIndex)-1.
	Nothing here is opt-in at load time and no model pays for it until
	Make_MeshVertexBuffer_Unique() is actually called on it. */
	uint32_t Get_MeshCount() const {
		return m_iNumMeshes;
	}
	uint32_t Get_MeshVertexCount(uint32_t iMeshIndex) const;
	bool_t Has_MorphBaseVertices(uint32_t iMeshIndex) const;
	// True only when the existing immutable mesh can consume screen-space LOD.
	bool_t Has_StaticMeshLod(uint32_t iMeshIndex) const;
	// Read-only CPU selection shared with the actual instanced draw. Invalid views keep LOD 0.
	uint32_t Get_StaticMeshLodLevel(uint32_t iMeshIndex, const MESH_SCREEN_LOD_DESC* view) const;
	// Same selected range as Render_Instanced; missing/invalid views keep the original index count.
	uint32_t Get_StaticMeshSelectedIndexCount(uint32_t iMeshIndex, const MESH_SCREEN_LOD_DESC* view = nullptr) const;
	bool_t Get_MorphBaseVertex(uint32_t iMeshIndex, uint32_t iVertexIndex,
		float3_t& OutPosition, float3_t& OutNormal) const;
	/* Must be called once (per CModel instance, i.e. per clone) before Update_Mesh_Vertices()
	targets that mesh; a no-op if already unique. */
	HRESULT Make_MeshVertexBuffer_Unique(uint32_t iMeshIndex);
	HRESULT Update_Mesh_Vertices(uint32_t iMeshIndex, const vector<uint32_t>& iVertexIndices,
		const vector<float3_t>& Positions, const vector<float3_t>& Normals);
	/* Restores every vertex Update_Mesh_Vertices() has touched on this mesh back to its
	unmorphed rest state (weight-0). */
	HRESULT Reset_Mesh_Vertices(uint32_t iMeshIndex);

private:
	const aiScene*						m_pAIScene = { nullptr };
	/* Built only for a model that actually goes through Assimp. Its constructor registers
	every importer Assimp ships, and the runtime loads .wmodel exclusively, so as a plain
	member it built that whole registry for every model in the game and never read a file
	with it -- which is also what the CRT leak dump was full of. */
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
	// Pose data belongs to the model clone, while mesh geometry remains shared.
	struct SKIN_PALETTE final
	{
		vector<float4x4_t> Matrices;
		uint64_t iPoseRevision = 0u;
	};
	vector<SKIN_PALETTE> m_SkinPalettes;
	uint64_t m_iBonePoseRevision = 1u;
	void Invalidate_SkinPalettes();
	/* Pose_BonesFrom's name join, kept because it is the same two skeletons every frame.
	-1 marks a bone the source does not have. */
	vector<int32_t>						m_SourcePoseBoneIndices;
	const CModel*						m_pSourcePoseModel = { nullptr };
	weak_ptr<const CPrototype> m_SourcePoseOwner;
	// Both poses must still match the copy, including same-frame local edits.
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
    // Reuses immutable GPU geometry and clones the pose with independent materials.
    // Load identity must match the prototype; this cannot retarget geometry.
    static unique_ptr<CModel> Create_MaterialVariant(const CModel& prototype,
        const MODEL_ASSET_LOAD_DESC& loadDesc);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
	void Free();
};

NS_END

```

### Engine/Private/Model.cpp

위치: 기존 파일 전체 교체. 기존 미변경 코드를 포함한 적용 후보 전문이다.

```cpp
#include "Model.h"
#include "SourceCharacterProgramRegistry.h"
#pragma push_macro("new")
#undef new
#include "Assimp/Importer.hpp"
#include "Assimp/postprocess.h"
#pragma pop_macro("new")
#pragma push_macro("new")
#undef new
#include "Assimp/scene.h"
#pragma pop_macro("new")
#include "Engine_VertexTypes.h"
#include "Profiler.h"
#include "GameInstance.h"

#include "BinaryAsset/ModelAssetData.h"
#include "BinaryAsset/ModelDecoderRegistry.h"
#include "Mesh.h"
#include "Bone.h"
#include "Shader.h"
#include "Material.h"
#include "Animation.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstring>
#include <limits>
#include <new>
#include <stdexcept>

namespace Engine
{
    struct MODEL_MATERIAL_SOURCE
    {
        struct MESH_CHANNELS
        {
            uint32_t materialIndex;
            MODEL_VERTEX_KIND vertexKind;
            bool_t hasColor0, hasTexcoord1, hasTexcoord2, hasTangentHandedness;
        };
        MODEL_ASSET_LOAD_DESC identity;
        vector<MODEL_MATERIAL_DATA> materials;
        vector<MESH_CHANNELS> meshes;
    };
}

#include "Model_StaticCluster.inl"
#include "Model_StaticProxy.inl"

namespace
{
    bool NativeHairUsesExtraUV(const Engine::MODEL_SOURCE_CHARACTER_PARAMETERS& source)
    {
        // Program 7 uses UV1 only for two-tone colour interpolation. Some retail
        // hair meshes contain UV0 alone and disable this exact native branch.
        return source.program == 7u &&
            (source.baseConstants[16].x != 0.f || source.lightConstants[13].x != 0.f ||
             source.baseConstants[18].y == 0.f || source.baseConstants[18].w == 0.f ||
             source.lightConstants[15].y == 0.f || source.lightConstants[15].w == 0.f);
    }

	bool Is_FiniteMatrix(const float4x4_t& Matrix)
	{
		const f32_t* const Values = &Matrix._11;
		for (size_t i = 0u; i < 16u; ++i)
		{
			if (!std::isfinite(Values[i]))
				return false;
		}
		return true;
	}

	bool Try_BlendLocalMatrix(
		const float4x4_t& From,
		const f32_t fWeight,
		float4x4_t& InOutTarget)
	{
		vector_t FromScale{};
		vector_t FromRotation{};
		vector_t FromTranslation{};
		vector_t TargetScale{};
		vector_t TargetRotation{};
		vector_t TargetTranslation{};
		if (!XMMatrixDecompose(&FromScale, &FromRotation, &FromTranslation,
				XMLoadFloat4x4(&From)) ||
			!XMMatrixDecompose(&TargetScale, &TargetRotation,
				&TargetTranslation, XMLoadFloat4x4(&InOutTarget)))
		{
			return false;
		}
		XMStoreFloat4x4(&InOutTarget,
			XMMatrixAffineTransformation(
				XMVectorLerp(FromScale, TargetScale, fWeight),
				XMVectorZero(),
				XMQuaternionSlerp(FromRotation, TargetRotation, fWeight),
				XMVectorLerp(FromTranslation, TargetTranslation, fWeight)));
		return Is_FiniteMatrix(InOutTarget);
	}
}

CModel::CModel(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
    : CComponent { pDevice, pContext }
{
}

CModel::CModel(const CModel& Prototype)
    : CComponent { Prototype }
    , m_pAIScene { Prototype.m_pAIScene }
    , m_eType { Prototype.m_eType }
    , m_iNumMeshes { Prototype.m_iNumMeshes }
    , m_Meshes { Prototype.m_Meshes }
    , m_pMaterialSource { Prototype.m_pMaterialSource }
    , m_PreTransformMatrix { Prototype.m_PreTransformMatrix }
    , m_bRetainOrderedStaticGeometry { Prototype.m_bRetainOrderedStaticGeometry }
    , m_pOrderedStaticGeometrySource { Prototype.m_pOrderedStaticGeometrySource }
    , m_StaticCluster { Prototype.m_StaticCluster }
    , m_StaticProxy { Prototype.m_StaticProxy }
    , m_StaticProxyAtlases { Prototype.m_StaticProxyAtlases }
    , m_StaticProxyMetadata { Prototype.m_StaticProxyMetadata }
    , m_OrderedStaticGeometry { Prototype.m_OrderedStaticGeometry }
    , m_iNextOrderedStaticGeometryHandle { Prototype.m_iNextOrderedStaticGeometryHandle }
    , m_iNumMaterials { Prototype.m_iNumMaterials }
    , m_Materials { Prototype.m_Materials }
   // , m_Bones { Prototype.m_Bones }
    , m_BoneRestLocalTransforms { Prototype.m_BoneRestLocalTransforms }
    , m_iSkeletonHash { Prototype.m_iSkeletonHash }
    , m_iCurrentAnimIndex { Prototype.m_iCurrentAnimIndex }
    , m_iNumAnimations { Prototype.m_iNumAnimations}
    // , m_Animations { Prototype.m_Animations }
    , m_AuthoredAnimationNames { Prototype.m_AuthoredAnimationNames }
    , m_isAnimLoop { Prototype.m_isAnimLoop }
	, m_isAnimPaused { Prototype.m_isAnimPaused }
	, m_fAnimationSpeed { Prototype.m_fAnimationSpeed }
	, m_iRootMotionBoneIndex { Prototype.m_iRootMotionBoneIndex }
	, m_iRootMotionVerticalAxis { Prototype.m_iRootMotionVerticalAxis }
	, m_vRootMotionRestTranslation { Prototype.m_vRootMotionRestTranslation }
	, m_vRootMotionUnscaledTranslation { Prototype.m_vRootMotionUnscaledTranslation }
	, m_fRootMotionVerticalScale { Prototype.m_fRootMotionVerticalScale }
	, m_bHasLocalBounds { Prototype.m_bHasLocalBounds }
	, m_vLocalBoundsMin { Prototype.m_vLocalBoundsMin }
	, m_vLocalBoundsMax { Prototype.m_vLocalBoundsMax }
    , m_bAnimationEnvelopeAttempted { Prototype.m_bAnimationEnvelopeAttempted }
    , m_fAnimationEnvelopeRadius { Prototype.m_fAnimationEnvelopeRadius }
    , m_bAnimationEnvelopeExternalPose { Prototype.m_bAnimationEnvelopeExternalPose }
    , m_bHasBindGeometryBounds { Prototype.m_bHasBindGeometryBounds }
    , m_vBindGeometryBoundsMin { Prototype.m_vBindGeometryBoundsMin }
    , m_vBindGeometryBoundsMax { Prototype.m_vBindGeometryBoundsMax }
	, m_bHasSelfConsistentUnauthenticatedGeometryMetadata { Prototype.m_bHasSelfConsistentUnauthenticatedGeometryMetadata }
	, m_iGeometryFormatVersionMajor { Prototype.m_iGeometryFormatVersionMajor }
	, m_iGeometryFormatVersionMinor { Prototype.m_iGeometryFormatVersionMinor }
	, m_iGeometryChannelMask { Prototype.m_iGeometryChannelMask }
	, m_iGeometryEvidenceFlags { Prototype.m_iGeometryEvidenceFlags }
	, m_fGeometryPreScale { Prototype.m_fGeometryPreScale }
	, m_GeometryPayloadSha256 { Prototype.m_GeometryPayloadSha256 }
	, m_GeometryMetadataIdentitySha256 { Prototype.m_GeometryMetadataIdentitySha256 }
{
    for (auto& pPrototype : Prototype.m_Bones)
        m_Bones.push_back(pPrototype->Clone());

    for (auto& pPrototype : Prototype.m_Animations)    
        m_Animations.push_back(pPrototype->Clone());
}

CModel::~CModel()
{
}

matrix_t CModel::Get_BoneMatrix(const char_t* pBoneName)
{
    auto    iter = find_if(m_Bones.begin(), m_Bones.end(), [&](shared_ptr<CBone> pBone)->bool_t {
        if (true == pBone->Compare_Name(pBoneName))
            return true;
        return false;
    });

    if (iter == m_Bones.end())
        return XMMatrixIdentity();

    return (*iter)->Get_CombinedTransformationMatrix();    
}

bool_t CModel::Set_Animation(
    const char_t* pAnimationName,
    bool_t isLoop,
    f32_t fBlendSeconds)
{
    if (nullptr == pAnimationName)
        return false;

    for (uint32_t i = 0; i < m_Animations.size(); ++i)
    {
        if (!m_Animations[i]->Compare_Name(pAnimationName))
            continue;

        if (i != m_iCurrentAnimIndex)
            Begin_AnimBlend(fBlendSeconds);

        m_bExplicitAnimationPose = false;
        m_iCurrentAnimIndex = i;
        m_isAnimLoop = isLoop;
        return true;
    }
    return false;
}

void CModel::Begin_AnimBlend(f32_t fBlendSeconds)
{
    if (fBlendSeconds <= 0.f || m_Bones.empty())
    {
        m_fBlendDuration = 0.f;
        m_fBlendElapsed = 0.f;
        return;
    }

    m_BlendFromPose.resize(m_Bones.size());
    for (size_t i = 0; i < m_Bones.size(); ++i)
        XMStoreFloat4x4(
            &m_BlendFromPose[i],
            m_Bones[i]->Get_TransformationMatrix());

    if (m_fRootMotionVerticalScale != 1.f && m_iRootMotionBoneIndex >= 0 &&
        static_cast<size_t>(m_iRootMotionBoneIndex) < m_BlendFromPose.size())
        Restore_UnscaledRootVertical(m_BlendFromPose[m_iRootMotionBoneIndex]);
    m_fBlendDuration = fBlendSeconds;
    m_fBlendElapsed = 0.f;
}

void CModel::Update_AnimBlend(f32_t fTimeDelta)
{
    Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Animation.Blend");
    if (m_fBlendElapsed >= m_fBlendDuration ||
        m_BlendFromPose.size() != m_Bones.size())
        return;

    m_fBlendElapsed += fTimeDelta;
    const f32_t fRatio = m_fBlendDuration > 0.f ?
        (min)(m_fBlendElapsed / m_fBlendDuration, 1.f) : 1.f;

    for (size_t i = 0; i < m_Bones.size(); ++i)
    {
        m_Bones[i]->Blend_TransformationMatrix(
            XMLoadFloat4x4(&m_BlendFromPose[i]),
            fRatio);
    }
}

bool_t CModel::Has_Bone(const char_t* pBoneName)
{
    if (nullptr == pBoneName || '\0' == pBoneName[0])
        return false;

    return m_Bones.end() != find_if(
        m_Bones.begin(),
        m_Bones.end(),
        [&](const shared_ptr<CBone>& pBone)
        {
            return nullptr != pBone && pBone->Compare_Name(pBoneName);
        });
}

vector<string> CModel::Get_BoneNames() const
{
    vector<string> names;
    names.reserve(m_Bones.size());
    for (const auto& bone : m_Bones)
        if (bone) names.emplace_back(bone->Get_Name());
    return names;
}

int32_t CModel::Find_BoneIndex(const char_t* pBoneName) const
{
    if (nullptr == pBoneName || '\0' == pBoneName[0])
        return -1;

    for (size_t i = 0; i < m_Bones.size(); ++i)
    {
        if (nullptr != m_Bones[i] && m_Bones[i]->Compare_Name(pBoneName))
            return static_cast<int32_t>(i);
    }
    return -1;
}

int32_t CModel::Get_BoneParentIndex(const uint32_t iBoneIndex) const
{
    if (iBoneIndex >= m_Bones.size() || nullptr == m_Bones[iBoneIndex])
        return -1;

    return m_Bones[iBoneIndex]->Get_ParentBoneIndex();
}

bool_t CModel::Get_BoneLocalMatrix(
    const uint32_t iBoneIndex, matrix_t& outMatrix) const
{
    if (iBoneIndex >= m_Bones.size() || nullptr == m_Bones[iBoneIndex])
        return false;

    outMatrix = m_Bones[iBoneIndex]->Get_TransformationMatrix();
    return true;
}

bool_t CModel::Get_BoneRestLocalMatrix(
    const uint32_t iBoneIndex, matrix_t& outMatrix) const
{
    if (iBoneIndex >= m_BoneRestLocalTransforms.size())
        return false;

    outMatrix = XMLoadFloat4x4(&m_BoneRestLocalTransforms[iBoneIndex]);
    return true;
}

bool_t CModel::Get_BoneCombinedMatrix(
    const uint32_t iBoneIndex, matrix_t& outMatrix) const
{
    if (iBoneIndex >= m_Bones.size() || nullptr == m_Bones[iBoneIndex])
        return false;

    outMatrix = m_Bones[iBoneIndex]->Get_CombinedTransformationMatrix();
    return true;
}

bool_t CModel::Sample_AnimationBoneCombinedMatrices(
	const char_t* pAnimationName,
	const f32_t fTrackPositionTicks,
	const std::span<const uint32_t> BoneIndices,
	const std::span<float4x4_t> OutCombinedMatrices) const
{
	if (nullptr == pAnimationName || '\0' == pAnimationName[0])
		return false;
	uint32_t iAnimationIndex = UINT32_MAX;
	for (size_t i = 0u; i < m_Animations.size(); ++i)
	{
		if (nullptr == m_Animations[i] ||
			!m_Animations[i]->Compare_Name(pAnimationName))
		{
			continue;
		}
		if (iAnimationIndex != UINT32_MAX)
			return false;
		iAnimationIndex = static_cast<uint32_t>(i);
	}
	return iAnimationIndex != UINT32_MAX &&
		Sample_BoneCombinedMatricesForAnimation(
			iAnimationIndex, fTrackPositionTicks, false, 0.f,
			BoneIndices, OutCombinedMatrices);
}

bool_t CModel::Sample_AnimationRootTranslation(const char_t* name,
    const f32_t ticks, const uint32_t rootIndex, const int32_t verticalAxis,
    const f32_t verticalScale, float3_t& output) const
{
    if (!name || !*name || !std::isfinite(ticks) || ticks < 0.f ||
        verticalAxis < 0 || verticalAxis > 2 || !std::isfinite(verticalScale) ||
        verticalScale < 0.f || rootIndex >= m_Bones.size() ||
        m_BoneRestLocalTransforms.size() != m_Bones.size() ||
        !Is_FiniteMatrix(m_PreTransformMatrix)) return false;
    const CAnimation* animation = nullptr;
    for (const auto& candidate : m_Animations)
        if (candidate && candidate->Compare_Name(name))
        {
            if (animation) return false;
            animation = candidate.get();
        }
    if (!animation || ticks > animation->Get_Duration()) return false;
    auto local = m_BoneRestLocalTransforms;
    auto initial = m_BoneRestLocalTransforms;
    if (!animation->Sample_LocalBoneTransforms(ticks, local) ||
        !animation->Sample_LocalBoneTransforms(0.f, initial)) return false;
    const auto& raw = local[rootIndex];
    const auto& rest = m_BoneRestLocalTransforms[rootIndex];
    if (!m_Bones[rootIndex] || !Is_FiniteMatrix(raw) || !Is_FiniteMatrix(rest)) return false;
    matrix_t basis = XMMatrixIdentity();
    int32_t parent = m_Bones[rootIndex]->Get_ParentBoneIndex();
    uint32_t child = rootIndex;
    while (parent >= 0)
    {
        if (static_cast<uint32_t>(parent) >= child || !m_Bones[parent] ||
            !Is_FiniteMatrix(local[parent]) || !Is_FiniteMatrix(initial[parent]) ||
            !animation->Is_BoneTransformConstant(static_cast<uint32_t>(parent))) return false;
        // Native-key constancy above owns this admission. Re-sampling equal
        // quaternion keys can change a scale-100 matrix by float roundoff
        // (Albion: 2.38e-5 at 5 ms); that is not animated ancestor motion.
        // Always use the admitted initial basis, independent of sample time.
        basis = basis * XMLoadFloat4x4(&initial[parent]);
        child = static_cast<uint32_t>(parent);
        parent = m_Bones[parent]->Get_ParentBoneIndex();
    }
    if (parent != -1) return false;
    float3_t delta{raw._41 - rest._41, raw._42 - rest._42, raw._43 - rest._43};
    if (verticalAxis == 0) delta.x *= verticalScale;
    else if (verticalAxis == 1) delta.y *= verticalScale;
    else delta.z *= verticalScale;
    float3_t result;
    XMStoreFloat3(&result, XMVector3TransformNormal(XMLoadFloat3(&delta),
        basis * XMLoadFloat4x4(&m_PreTransformMatrix)));
    if (!std::isfinite(result.x) || !std::isfinite(result.y) || !std::isfinite(result.z)) return false;
    output = result;
    return true;
}

CModel::ROOT_MOTION_SUPPRESSION_STATE CModel::Capture_RootMotionSuppression() const
{
    return {this, m_iRootMotionBoneIndex, m_iRootMotionVerticalAxis,
        m_vRootMotionRestTranslation, m_vRootMotionUnscaledTranslation, m_fRootMotionVerticalScale};
}

bool_t CModel::Configure_RootMotionSuppressionFromRest(const uint32_t rootIndex,
    const int32_t verticalAxis, const f32_t verticalScale)
{
    if (rootIndex >= m_Bones.size() || !m_Bones[rootIndex] ||
        rootIndex >= m_BoneRestLocalTransforms.size() || verticalAxis < 0 || verticalAxis > 2 ||
        !std::isfinite(verticalScale) || verticalScale < 0.f ||
        !Is_FiniteMatrix(m_BoneRestLocalTransforms[rootIndex])) return false;
    const auto& rest = m_BoneRestLocalTransforms[rootIndex];
    m_iRootMotionBoneIndex = static_cast<int32_t>(rootIndex);
    m_iRootMotionVerticalAxis = verticalAxis;
    m_vRootMotionRestTranslation = {rest._41, rest._42, rest._43};
    float4x4_t current;
    XMStoreFloat4x4(&current, m_Bones[rootIndex]->Get_TransformationMatrix());
    m_vRootMotionUnscaledTranslation = {current._41, current._42, current._43};
    m_bAnimationEnvelopeAttempted = false;
    m_fRootMotionVerticalScale = verticalScale;
    return true;
}

bool_t CModel::Restore_RootMotionSuppression(const ROOT_MOTION_SUPPRESSION_STATE& state)
{
    if (state.owner != this || state.boneIndex < -1 ||
        (state.boneIndex >= 0 && static_cast<size_t>(state.boneIndex) >= m_Bones.size())) return false;
    m_iRootMotionBoneIndex = state.boneIndex;
    m_iRootMotionVerticalAxis = state.verticalAxis;
    m_vRootMotionRestTranslation = state.restTranslation;
    m_vRootMotionUnscaledTranslation = state.unscaledTranslation;
    m_bAnimationEnvelopeAttempted = false;
    m_fRootMotionVerticalScale = state.verticalScale;
    return true;
}

bool_t CModel::Sample_AnimationTransitionBoneCombinedMatrices(
    const ANIMATION_TRANSITION_POSE& pose,
    const std::span<const uint32_t> BoneIndices,
    const std::span<float4x4_t> OutCombinedMatrices) const
{
    if (BoneIndices.empty() || BoneIndices.size() != OutCombinedMatrices.size() ||
        std::any_of(BoneIndices.begin(), BoneIndices.end(),
            [this](uint32_t index) { return index >= m_Bones.size(); })) return false;
    std::vector<float4x4_t> local, combined;
    if (!Build_AnimationTransitionPose(pose, local, combined)) return false;
    for (size_t index = 0u; index < BoneIndices.size(); ++index)
        OutCombinedMatrices[index] = combined[BoneIndices[index]];
    return true;
}

bool_t CModel::Build_AnimationTransitionPose(const ANIMATION_TRANSITION_POSE& pose,
    vector<float4x4_t>& local, vector<float4x4_t>& combined, float3_t* unscaledRoot) const
{
    Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Animation.Transition.Build");
    if (m_Bones.empty() || m_BoneRestLocalTransforms.size() != m_Bones.size() ||
        !std::isfinite(pose.durationSeconds) || pose.durationSeconds <= 0.f || pose.durationSeconds > 1.f ||
        !std::isfinite(pose.elapsedSeconds) || pose.elapsedSeconds < 0.f ||
        !std::isfinite(pose.playRate) || pose.playRate <= 0.f || !Is_FiniteMatrix(m_PreTransformMatrix)) return false;
    auto sample = [&](uint32_t index, float ticks, vector<float4x4_t>& out)
    {
        if (!std::isfinite(ticks) || ticks < 0.f) return false;
        out = m_BoneRestLocalTransforms;
        if (index == UINT32_MAX) return ticks == 0.f;
        return index < m_Animations.size() && m_Animations[index] &&
            std::isfinite(m_Animations[index]->Get_Duration()) && ticks <= m_Animations[index]->Get_Duration() &&
            m_Animations[index]->Sample_LocalBoneTransforms(ticks, out);
    };
    vector<float4x4_t> from;
    if (!sample(pose.sourceIndex, pose.sourceTicks, from) || !sample(pose.targetIndex, pose.targetTicks, local)) return false;
    const float alpha = (std::min)(pose.elapsedSeconds / pose.durationSeconds, 1.f);
    {
        Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Animation.Blend");
    for (size_t i = 0; i < local.size(); ++i)
        if (!m_Bones[i] || !Is_FiniteMatrix(from[i]) || !Is_FiniteMatrix(local[i]) ||
            (alpha < 1.f && !Try_BlendLocalMatrix(from[i], alpha, local[i]))) return false;
    }
    if (m_iRootMotionBoneIndex >= 0)
    {
        if (size_t(m_iRootMotionBoneIndex) >= local.size()) return false;
        const auto& root = local[m_iRootMotionBoneIndex];
        if (unscaledRoot) *unscaledRoot = {root._41, root._42, root._43};
        Apply_RootMotionTranslation(local[m_iRootMotionBoneIndex]);
    }
    {
        Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Animation.Bones.Combine");
    combined.resize(local.size());
    for (size_t i = 0; i < local.size(); ++i)
    {
        const int parent = m_Bones[i]->Get_ParentBoneIndex();
        if (parent < -1 || (parent >= 0 && size_t(parent) >= i)) return false;
        XMStoreFloat4x4(&combined[i], XMLoadFloat4x4(&local[i]) *
            (parent == -1 ? XMLoadFloat4x4(&m_PreTransformMatrix) : XMLoadFloat4x4(&combined[parent])));
        if (!Is_FiniteMatrix(combined[i])) return false;
    }
    }
    return true;
}

bool_t CModel::Set_AnimationTransitionPose(const ANIMATION_TRANSITION_POSE& pose)
{
    Engine::CProfilerModelAnimationScope modelAnimationScope(CGameInstance::Get().Get_Profiler(), this);
    vector<float4x4_t> local, combined;
    float3_t unscaledRoot = m_vRootMotionUnscaledTranslation;
    if (!Build_AnimationTransitionPose(pose, local, combined, &unscaledRoot)) return false;
    // Admission completes before touching either the cursor or live palette.
    if (pose.targetIndex != UINT32_MAX)
    {
        m_iCurrentAnimIndex = pose.targetIndex;
        m_Animations[pose.targetIndex]->Set_TrackPosition(pose.targetTicks);
    }
    for (size_t i = 0; i < local.size(); ++i)
        m_Bones[i]->Update_TransformationMatrix(XMLoadFloat4x4(&local[i]));
    Refresh_BoneCombinedMatrices();
    m_vRootMotionUnscaledTranslation = unscaledRoot;
    Skip_Blend();
    m_isAnimLoop = false;
    m_bAnimationEnvelopeExternalPose = true;
    m_ExplicitAnimationPose = pose;
    m_bExplicitAnimationPose = true;
    return true;
}

bool_t CModel::Sample_CurrentAnimationBoneCombinedMatrices(
	const uint32_t iExpectedAnimationIndex,
	const f32_t fTrackPositionTicks,
	const std::span<const uint32_t> BoneIndices,
	const std::span<float4x4_t> OutCombinedMatrices) const
{
	return Sample_CurrentAnimationBoneCombinedMatricesAtBlendElapsed(
		iExpectedAnimationIndex, fTrackPositionTicks, m_fBlendElapsed,
		BoneIndices, OutCombinedMatrices);
}

bool_t CModel::Sample_CurrentAnimationBoneCombinedMatricesAtBlendElapsed(
	const uint32_t iExpectedAnimationIndex,
	const f32_t fTrackPositionTicks,
	const f32_t fBlendElapsedSeconds,
	const std::span<const uint32_t> BoneIndices,
	const std::span<float4x4_t> OutCombinedMatrices) const
{
	if (m_bExplicitAnimationPose && iExpectedAnimationIndex == m_iCurrentAnimIndex)
	{
		if (BoneIndices.empty() || BoneIndices.size() != OutCombinedMatrices.size() ||
			iExpectedAnimationIndex >= m_Animations.size()) return false;
		auto pose = m_ExplicitAnimationPose;
		const float tps = Get_AnimationTickPerSecond(iExpectedAnimationIndex);
		if (!std::isfinite(tps) || tps <= 0.f) return false;
		pose.elapsedSeconds = (std::max)(0.f, pose.elapsedSeconds +
			(fTrackPositionTicks - pose.targetTicks) / (tps * pose.playRate));
		pose.targetTicks = fTrackPositionTicks;
		vector<float4x4_t> local, combined;
		if (!Build_AnimationTransitionPose(pose, local, combined)) return false;
		for (const auto index : BoneIndices) if (index >= combined.size()) return false;
		for (size_t i = 0; i < BoneIndices.size(); ++i) OutCombinedMatrices[i] = combined[BoneIndices[i]];
		return true;
	}
	return Sample_BoneCombinedMatricesForAnimation(
		iExpectedAnimationIndex, fTrackPositionTicks, true, fBlendElapsedSeconds,
		BoneIndices, OutCombinedMatrices);
}

bool_t CModel::Sample_BoneCombinedMatricesForAnimation(
	const uint32_t iExpectedAnimationIndex,
	const f32_t fTrackPositionTicks,
	const bool_t bUseCurrentPoseAndBlend,
	const f32_t fBlendElapsedSeconds,
	const std::span<const uint32_t> BoneIndices,
	const std::span<float4x4_t> OutCombinedMatrices) const
{
	Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Animation.History.Sample");
	if ((bUseCurrentPoseAndBlend && iExpectedAnimationIndex != m_iCurrentAnimIndex) ||
		iExpectedAnimationIndex >= m_Animations.size() ||
		nullptr == m_Animations[iExpectedAnimationIndex] ||
		!std::isfinite(fTrackPositionTicks) ||
		!std::isfinite(fBlendElapsedSeconds) ||
		fBlendElapsedSeconds < 0.f ||
		fTrackPositionTicks < 0.f ||
		fTrackPositionTicks >
			m_Animations[iExpectedAnimationIndex]->Get_Duration() ||
		!std::isfinite(
			m_Animations[iExpectedAnimationIndex]->Get_Duration()) ||
		BoneIndices.empty() ||
		BoneIndices.size() != OutCombinedMatrices.size() ||
		m_Bones.empty() ||
		(!bUseCurrentPoseAndBlend && m_BoneRestLocalTransforms.size() != m_Bones.size()))
	{
		return false;
	}
	for (const uint32_t iBoneIndex : BoneIndices)
	{
		if (iBoneIndex >= m_Bones.size() || nullptr == m_Bones[iBoneIndex])
			return false;
	}

	const auto BuildCombined = [this, iExpectedAnimationIndex, bUseCurrentPoseAndBlend,
		fTrackPositionTicks, fBlendElapsedSeconds](
			std::vector<float4x4_t>& OutCombined)
	{
		std::vector<float4x4_t> LocalTransforms(m_Bones.size());
		for (size_t iBone = 0u; iBone < m_Bones.size(); ++iBone)
		{
			if (nullptr == m_Bones[iBone])
				return false;
			if (bUseCurrentPoseAndBlend)
			{
				XMStoreFloat4x4(&LocalTransforms[iBone],
					m_Bones[iBone]->Get_TransformationMatrix());
			}
			else
			{
				LocalTransforms[iBone] = m_BoneRestLocalTransforms[iBone];
			}
			if (!Is_FiniteMatrix(LocalTransforms[iBone]))
				return false;
		}
		if (bUseCurrentPoseAndBlend && m_fRootMotionVerticalScale != 1.f && m_iRootMotionBoneIndex >= 0 &&
			static_cast<size_t>(m_iRootMotionBoneIndex) < LocalTransforms.size())
			Restore_UnscaledRootVertical(LocalTransforms[m_iRootMotionBoneIndex]);
		if (!m_Animations[iExpectedAnimationIndex]->Sample_LocalBoneTransforms(
				fTrackPositionTicks, LocalTransforms))
		{
			return false;
		}

		if (bUseCurrentPoseAndBlend && (!std::isfinite(m_fBlendElapsed) ||
			!std::isfinite(m_fBlendDuration) ||
			m_fBlendElapsed < 0.f || m_fBlendDuration < 0.f))
		{
			return false;
		}
		/* Only an active live transition proves that m_BlendFromPose belongs to
		   this clip edge.  Once the live transition is complete (or when the same
		   clip is restarted without a new blend), ignore the stale saved pose. */
		if (bUseCurrentPoseAndBlend && m_fBlendElapsed < m_fBlendDuration &&
			fBlendElapsedSeconds < m_fBlendDuration)
		{
			if (m_fBlendDuration <= 0.f ||
				m_BlendFromPose.size() != LocalTransforms.size())
			{
				return false;
			}
			const f32_t fRatio =
				(min)(fBlendElapsedSeconds / m_fBlendDuration, 1.f);
			for (size_t iBone = 0u; iBone < LocalTransforms.size(); ++iBone)
			{
				if (!Is_FiniteMatrix(m_BlendFromPose[iBone]) ||
					!Try_BlendLocalMatrix(
						m_BlendFromPose[iBone], fRatio,
						LocalTransforms[iBone]))
				{
					return false;
				}
			}
		}

		if (m_iRootMotionBoneIndex >= 0)
		{
			if (static_cast<size_t>(m_iRootMotionBoneIndex) >=
					LocalTransforms.size() ||
				m_iRootMotionVerticalAxis < -1 ||
				m_iRootMotionVerticalAxis > 2)
			{
				return false;
			}
			float4x4_t& Root =
				LocalTransforms[static_cast<size_t>(m_iRootMotionBoneIndex)];
			Apply_RootMotionTranslation(Root);
		}

		OutCombined.resize(LocalTransforms.size());
		const matrix_t PreTransform =
			XMLoadFloat4x4(&m_PreTransformMatrix);
		if (!Is_FiniteMatrix(m_PreTransformMatrix))
			return false;
		for (size_t iBone = 0u; iBone < LocalTransforms.size(); ++iBone)
		{
			const int32_t iParent = m_Bones[iBone]->Get_ParentBoneIndex();
			matrix_t Combined;
			if (-1 == iParent)
			{
				Combined = XMLoadFloat4x4(&LocalTransforms[iBone]) *
					PreTransform;
			}
			else
			{
				if (iParent < 0 || static_cast<size_t>(iParent) >= iBone)
					return false;
				Combined = XMLoadFloat4x4(&LocalTransforms[iBone]) *
					XMLoadFloat4x4(
						&OutCombined[static_cast<size_t>(iParent)]);
			}
			XMStoreFloat4x4(&OutCombined[iBone], Combined);
			if (!Is_FiniteMatrix(OutCombined[iBone]))
				return false;
		}
		return true;
	};

#if defined(_DEBUG)
	const uint32_t iCurrentAnimationBefore = m_iCurrentAnimIndex;
	const bool_t bPausedBefore = m_isAnimPaused;
	const bool_t bLoopBefore = m_isAnimLoop;
	const f32_t fBlendElapsedBefore = m_fBlendElapsed;
	const f32_t fBlendDurationBefore = m_fBlendDuration;
	const f32_t fTrackPositionBefore =
		m_Animations[iExpectedAnimationIndex]->Get_CurrentTrackPosition();
	const std::vector<uint32_t> LeftKeyFrameIndicesBefore =
		m_Animations[iExpectedAnimationIndex]->m_iLeftKeyFrameIndices;
	std::vector<float4x4_t> LiveLocalBefore(m_Bones.size());
	std::vector<float4x4_t> LiveCombinedBefore(m_Bones.size());
	for (size_t iBone = 0u; iBone < m_Bones.size(); ++iBone)
	{
		XMStoreFloat4x4(&LiveLocalBefore[iBone],
			m_Bones[iBone]->Get_TransformationMatrix());
		XMStoreFloat4x4(&LiveCombinedBefore[iBone],
			m_Bones[iBone]->Get_CombinedTransformationMatrix());
	}
#endif

	std::vector<float4x4_t> StagedCombined;
	if (!BuildCombined(StagedCombined))
		return false;

#if defined(_DEBUG)
	{
		Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Animation.History.DebugVerify");
	std::vector<float4x4_t> DeterministicCombined;
	if (!BuildCombined(DeterministicCombined) ||
		iCurrentAnimationBefore != m_iCurrentAnimIndex ||
		bPausedBefore != m_isAnimPaused || bLoopBefore != m_isAnimLoop ||
		fBlendElapsedBefore != m_fBlendElapsed || fBlendDurationBefore != m_fBlendDuration ||
		DeterministicCombined.size() != StagedCombined.size() ||
		0 != std::memcmp(DeterministicCombined.data(), StagedCombined.data(),
			StagedCombined.size() * sizeof(float4x4_t)) ||
		fTrackPositionBefore !=
			m_Animations[iExpectedAnimationIndex]->Get_CurrentTrackPosition() ||
		LeftKeyFrameIndicesBefore !=
			m_Animations[iExpectedAnimationIndex]->m_iLeftKeyFrameIndices)
	{
		return false;
	}
	for (size_t iBone = 0u; iBone < m_Bones.size(); ++iBone)
	{
		float4x4_t LiveLocalAfter{};
		float4x4_t LiveCombinedAfter{};
		XMStoreFloat4x4(&LiveLocalAfter,
			m_Bones[iBone]->Get_TransformationMatrix());
		XMStoreFloat4x4(&LiveCombinedAfter,
			m_Bones[iBone]->Get_CombinedTransformationMatrix());
		if (0 != std::memcmp(&LiveLocalBefore[iBone], &LiveLocalAfter,
				sizeof(float4x4_t)) ||
			0 != std::memcmp(&LiveCombinedBefore[iBone], &LiveCombinedAfter,
				sizeof(float4x4_t)))
		{
			return false;
		}
	}
	}
#endif

	std::vector<float4x4_t> StagedOutput(BoneIndices.size());
	for (size_t i = 0u; i < BoneIndices.size(); ++i)
		StagedOutput[i] = StagedCombined[BoneIndices[i]];
	std::copy(StagedOutput.begin(), StagedOutput.end(),
		OutCombinedMatrices.begin());
	return true;
}

bool_t CModel::Set_BoneLocalMatrix(
    const uint32_t iBoneIndex, fmatrix_t Matrix)
{
    if (iBoneIndex >= m_Bones.size() || nullptr == m_Bones[iBoneIndex])
        return false;

    m_bAnimationEnvelopeExternalPose = true;
    m_Bones[iBoneIndex]->Update_TransformationMatrix(Matrix);
    // Costume-only locals are consumed by Pose_BonesFrom before a full refresh.
    m_iCopiedPoseRevision = 0u;
    return true;
}

uint32_t CModel::Pose_BonesFrom(const CModel& source)
{
    m_bAnimationEnvelopeExternalPose = true;
    /* By name, because the two skeletons are cooked separately and neither order nor count
       matches: a worn part carries the body's bones plus its own. */
    // A weak owner distinguishes a new model allocated at a retired source address.
    // Unowned prototypes retain the uncached path without extending their lifetime.
    const auto sourceOwner = source.weak_from_this().lock();
    const bool sameSource = sourceOwner && m_pSourcePoseModel == &source &&
        m_SourcePoseOwner.lock() == sourceOwner;
    if (sameSource && m_iSourcePoseRevision == source.m_iBonePoseRevision &&
        m_iCopiedPoseRevision == m_iBonePoseRevision)
        return m_iSourcePoseSuppliedBones;

    if (!sameSource || m_SourcePoseBoneIndices.size() != m_Bones.size())
    {
        m_SourcePoseBoneIndices.assign(m_Bones.size(), -1);
        for (size_t index = 0; index < m_Bones.size(); ++index)
        {
            if (nullptr == m_Bones[index])
                continue;
            for (size_t other = 0; other < source.m_Bones.size(); ++other)
            {
                if (nullptr == source.m_Bones[other] ||
                    !source.m_Bones[other]->Compare_Name(m_Bones[index]->Get_Name()))
                    continue;
                m_SourcePoseBoneIndices[index] = static_cast<int32_t>(other);
                break;
            }
        }
        m_pSourcePoseModel = &source;
        m_SourcePoseOwner = sourceOwner;
    }

    uint32_t supplied = 0u;
    for (size_t index = 0; index < m_Bones.size(); ++index)
    {
        if (nullptr == m_Bones[index])
            continue;
        const int32_t other = m_SourcePoseBoneIndices[index];
        if (other >= 0)
        {
            m_Bones[index]->Set_CombinedTransformationMatrix(
                source.m_Bones[static_cast<size_t>(other)]->Get_CombinedTransformationMatrix());
            ++supplied;
            continue;
        }
        /* A costume-only bone: its parent was posed already, so its own rest local carries it. */
        m_Bones[index]->Update_CombinedTransformationMatrix(
            m_Bones, XMLoadFloat4x4(&m_PreTransformMatrix));
    }
    Invalidate_SkinPalettes();
    m_iSourcePoseRevision = source.m_iBonePoseRevision;
    m_iCopiedPoseRevision = m_iBonePoseRevision;
    m_iSourcePoseSuppliedBones = supplied;
    return supplied;
}

void CModel::Invalidate_SkinPalettes()
{
    m_iCopiedPoseRevision = 0u;
    if (++m_iBonePoseRevision == 0u)
    {
        m_SkinPalettes.clear();
        m_iBonePoseRevision = 1u;
    }
}

void CModel::Refresh_BoneCombinedMatrices()
{
    Invalidate_SkinPalettes();
    Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Animation.Bones.Combine");
    for (auto& pBone : m_Bones)
    {
        pBone->Update_CombinedTransformationMatrix(
            m_Bones, XMLoadFloat4x4(&m_PreTransformMatrix));
    }
}

bool_t CModel::Enable_RootMotionSuppression(
    const char_t* pBoneName, const int32_t iVerticalAxis)
{
    if (nullptr == pBoneName || '\0' == pBoneName[0] ||
        iVerticalAxis < -1 || iVerticalAxis > 2)
        return false;

    for (size_t i = 0; i < m_Bones.size(); ++i)
    {
        if (nullptr == m_Bones[i] || !m_Bones[i]->Compare_Name(pBoneName))
            continue;

        float4x4_t rest{};
        XMStoreFloat4x4(&rest, m_Bones[i]->Get_TransformationMatrix());
        m_vRootMotionRestTranslation = { rest._41, rest._42, rest._43 };
        m_vRootMotionUnscaledTranslation = m_vRootMotionRestTranslation;
        m_iRootMotionBoneIndex = static_cast<int32_t>(i);
        m_iRootMotionVerticalAxis = iVerticalAxis;
        m_bAnimationEnvelopeAttempted = false;
        return true;
    }
    return false;
}

void CModel::Restore_UnscaledRootVertical(float4x4_t& Local) const
{
    if (0 == m_iRootMotionVerticalAxis) Local._41 = m_vRootMotionUnscaledTranslation.x;
    if (1 == m_iRootMotionVerticalAxis) Local._42 = m_vRootMotionUnscaledTranslation.y;
    if (2 == m_iRootMotionVerticalAxis) Local._43 = m_vRootMotionUnscaledTranslation.z;
}

void CModel::Apply_RootMotionTranslation(float4x4_t& Local) const
{
    if (0 != m_iRootMotionVerticalAxis) Local._41 = m_vRootMotionRestTranslation.x;
    else if (m_fRootMotionVerticalScale != 1.f)
        Local._41 = m_vRootMotionRestTranslation.x + (Local._41 - m_vRootMotionRestTranslation.x) * m_fRootMotionVerticalScale;
    if (1 != m_iRootMotionVerticalAxis) Local._42 = m_vRootMotionRestTranslation.y;
    else if (m_fRootMotionVerticalScale != 1.f)
        Local._42 = m_vRootMotionRestTranslation.y + (Local._42 - m_vRootMotionRestTranslation.y) * m_fRootMotionVerticalScale;
    if (2 != m_iRootMotionVerticalAxis) Local._43 = m_vRootMotionRestTranslation.z;
    else if (m_fRootMotionVerticalScale != 1.f)
        Local._43 = m_vRootMotionRestTranslation.z + (Local._43 - m_vRootMotionRestTranslation.z) * m_fRootMotionVerticalScale;
}

bool_t CModel::Set_RootMotionVerticalScale(const f32_t fScale)
{
    if (!std::isfinite(fScale) || fScale < 0.f || fScale > 1.f ||
        (fScale != 1.f && (m_iRootMotionBoneIndex < 0 || m_iRootMotionVerticalAxis < 0))) return false;
    if (m_fRootMotionVerticalScale == fScale) return true;
    const f32_t previousScale = m_fRootMotionVerticalScale;
    m_fRootMotionVerticalScale = fScale;
    m_bAnimationEnvelopeAttempted = false;
    if (m_iRootMotionBoneIndex >= 0 && static_cast<size_t>(m_iRootMotionBoneIndex) < m_Bones.size() && m_Bones[m_iRootMotionBoneIndex])
    {
        float4x4_t local{};
        XMStoreFloat4x4(&local, m_Bones[m_iRootMotionBoneIndex]->Get_TransformationMatrix());
        if (previousScale != 1.f) Restore_UnscaledRootVertical(local);
        else m_vRootMotionUnscaledTranslation = {local._41, local._42, local._43};
        Apply_RootMotionTranslation(local);
        m_Bones[m_iRootMotionBoneIndex]->Update_TransformationMatrix(XMLoadFloat4x4(&local));
        Refresh_BoneCombinedMatrices();
    }
    return true;
}

bool_t CModel::Start_Animation(
	const uint32_t iAnimIndex,
	const bool_t isLoop)
{
	if (iAnimIndex >= m_Animations.size())
		return false;
	m_bExplicitAnimationPose = false;
	m_iCurrentAnimIndex = iAnimIndex;
	m_isAnimLoop = isLoop;
	m_isAnimPaused = false;
	m_Animations[iAnimIndex]->Set_TrackPosition(0.f);
	Play_Animation(0.f);
	return true;
}

bool_t CModel::Start_Animation(
	const char_t* pAnimationName,
	const bool_t isLoop)
{
	if (!Set_Animation(pAnimationName, isLoop))
		return false;
	return Start_Animation(m_iCurrentAnimIndex, isLoop);
}

void CModel::Stop_Animation()
{
	m_isAnimPaused = true;
}

void CModel::Set_AnimationSpeed(const f32_t speed)
{
	m_fAnimationSpeed = isfinite(speed)
		? clamp(speed, -16.f, 16.f) : 1.f;
}

bool_t CModel::Update_Animation(const f32_t fTimeDelta)
{
	if (!isfinite(fTimeDelta))
		return false;
	return Play_Animation(fTimeDelta * m_fAnimationSpeed);
}

bool_t CModel::Advance_AnimationClock(const f32_t fTimeDelta, const bool_t applyAnimationSpeed)
{
    if (!std::isfinite(fTimeDelta) || m_bExplicitAnimationPose ||
        m_Animations.empty() || m_iCurrentAnimIndex >= m_Animations.size() ||
        !m_Animations[m_iCurrentAnimIndex]) return false;
    const float delta = m_isAnimPaused ? 0.f : fTimeDelta * (applyAnimationSpeed ? m_fAnimationSpeed : 1.f);
    if (!std::isfinite(delta)) return false;
    const bool_t finished = m_Animations[m_iCurrentAnimIndex]->Advance_Clock(delta, m_isAnimLoop);
    if (m_fBlendElapsed < m_fBlendDuration && m_BlendFromPose.size() == m_Bones.size())
        m_fBlendElapsed += delta;
    return finished;
}

const char_t* CModel::Get_AnimationName(uint32_t iAnimIndex) const
{
    if (iAnimIndex >= m_Animations.size())
        return nullptr;

    return m_Animations[iAnimIndex]->Get_Name();
}

bool_t CModel::Get_AnimationProgress(uint32_t iAnimIndex, f32_t& fOutPosition, f32_t& fOutDuration) const
{
    if (iAnimIndex >= m_Animations.size())
        return false;

    fOutPosition = m_Animations[iAnimIndex]->Get_CurrentTrackPosition();
    fOutDuration = m_Animations[iAnimIndex]->Get_Duration();
    return true;
}

/* 트랙 위치(틱)를 시간으로 환산할 때 쓴다. 유효하지 않으면 0을 돌려주므로
호출부가 자체 기본값을 쓸지 판단할 수 있다. */
f32_t CModel::Get_AnimationTickPerSecond(uint32_t iAnimIndex) const
{
    if (iAnimIndex >= m_Animations.size())
        return 0.f;

    return m_Animations[iAnimIndex]->Get_TickPerSecond();
}

bool_t CModel::Set_AnimTrackPosition(uint32_t iAnimIndex, f32_t fTrackPosition)
{
    if (iAnimIndex >= m_Animations.size())
        return false;

    m_Animations[iAnimIndex]->Set_TrackPosition(fTrackPosition);
    return true;
}

HRESULT CModel::Initialize_Prototype(MODEL eType, const char_t* pModelFilePath, fmatrix_t PreTransformMatrix)
{
    if (nullptr == pModelFilePath)
        return E_FAIL;

    XMStoreFloat4x4(&m_PreTransformMatrix, PreTransformMatrix);
    m_eType = eType;
	Reset_LocalBounds();
	m_pOrderedStaticGeometrySource.reset();
	m_OrderedStaticGeometry.clear();

    string extension = filesystem::path(pModelFilePath).extension().string();
    transform(extension.begin(), extension.end(), extension.begin(),
        [](unsigned char value) { return static_cast<char_t>(tolower(value)); });
    if (".wmodel" == extension)
        return Ready_BinaryModel(pModelFilePath);

    uint32_t  iFlag = { aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_Fast };

    if (MODEL::NONANIM == eType)
        iFlag |= aiProcess_PreTransformVertices;

    m_pImporter = make_unique<Assimp::Importer>();
    m_pAIScene = m_pImporter->ReadFile(pModelFilePath, iFlag);
    if (nullptr == m_pAIScene)
        return E_FAIL;

    if (FAILED(Ready_Bones(m_pAIScene->mRootNode)))
        return E_FAIL;

    if (FAILED(Ready_Meshes()))
        return E_FAIL;

    if (FAILED(Ready_Materials(pModelFilePath)))
        return E_FAIL;

    if (FAILED(Ready_Animations()))
        return E_FAIL;

    return S_OK;
}

HRESULT CModel::Initialize_Prototype(
	const MODEL eType,
	const MODEL_ASSET_LOAD_DESC& loadDesc,
	fmatrix_t PreTransformMatrix)
{
	if (loadDesc.meshPath.empty())
		return E_INVALIDARG;

	XMStoreFloat4x4(&m_PreTransformMatrix, PreTransformMatrix);
	m_eType = eType;
	Reset_LocalBounds();
	m_pOrderedStaticGeometrySource.reset();
	m_OrderedStaticGeometry.clear();
	return Ready_BinaryModel(loadDesc);
}

HRESULT CModel::Initialize(void* pArg)
{
    return S_OK;
}

HRESULT CModel::Render(uint32_t iMeshIndex)
{
    if (FAILED(m_Meshes[iMeshIndex]->Bind_Resources()))
        return E_FAIL;

    const HRESULT drawResult = m_Meshes[iMeshIndex]->Render();
    if (FAILED(drawResult))
        return E_FAIL;
    if (S_OK == drawResult)
        if (auto* profiler = CGameInstance::Get().Get_Profiler())
            profiler->Record_ModelSubmitted(this);
    return S_OK;
}

HRESULT CModel::Render_Instanced(uint32_t iMeshIndex,
    ID3D11Buffer* pInstanceBuffer, uint32_t iInstanceStride, uint32_t iNumInstances,
    uint32_t iInstanceByteOffset, const MESH_SCREEN_LOD_DESC* screenLod)
{
    if (iMeshIndex >= m_Meshes.size() ||
        nullptr == m_Meshes[iMeshIndex])
    {
        return E_INVALIDARG;
    }

    const HRESULT drawResult = m_Meshes[iMeshIndex]->Render_Instanced(
        pInstanceBuffer, iInstanceStride, iNumInstances, iInstanceByteOffset, screenLod);
    if (S_OK == drawResult)
        if (auto* profiler = CGameInstance::Get().Get_Profiler())
            profiler->Record_ModelSubmitted(this);
    return drawResult;
}

bool_t CModel::Can_ShareStaticInstanceStateWith(const CModel& other) const
{
    if (m_eType != MODEL::NONANIM || other.m_eType != MODEL::NONANIM ||
        m_StaticCluster || other.m_StaticCluster || m_pDevice != other.m_pDevice ||
        m_pContext != other.m_pContext || m_Meshes.empty() || m_Meshes != other.m_Meshes ||
        m_Materials != other.m_Materials || m_fGeometryPreScale != other.m_fGeometryPreScale)
        return false;
    for (size_t row = 0u; row < 4u; ++row)
        for (size_t column = 0u; column < 4u; ++column)
            if (m_PreTransformMatrix.m[row][column] != other.m_PreTransformMatrix.m[row][column]) return false;
    for (const auto& mesh : m_Meshes)
        if (!mesh || mesh->Has_MorphBaseVertices() ||
            mesh->Get_MaterialIndex() >= m_Materials.size() || !m_Materials[mesh->Get_MaterialIndex()])
            return false;
    return true;
}

bool_t CModel::Can_BatchStaticLightingWith(const CModel& other) const
{
    if (m_eType != MODEL::NONANIM || other.m_eType != MODEL::NONANIM ||
        m_StaticCluster || other.m_StaticCluster || m_pDevice != other.m_pDevice ||
        m_pContext != other.m_pContext || m_Meshes.empty() || m_Meshes != other.m_Meshes ||
        m_fGeometryPreScale != other.m_fGeometryPreScale)
        return false;
    for (size_t row = 0u; row < 4u; ++row)
        for (size_t column = 0u; column < 4u; ++column)
            if (m_PreTransformMatrix.m[row][column] != other.m_PreTransformMatrix.m[row][column]) return false;
    for (const auto& mesh : m_Meshes)
    {
        if (!mesh || mesh->Has_MorphBaseVertices()) return false;
        const uint32_t material = mesh->Get_MaterialIndex();
        if (material >= m_Materials.size() || material >= other.m_Materials.size() ||
            !m_Materials[material] || !other.m_Materials[material] ||
            !m_Materials[material]->Can_BatchStaticLightingWith(*other.m_Materials[material])) return false;
    }
    return true;
}

bool_t CModel::Has_SameStaticLightingTextures(const CModel& other) const
{
    // One instance slot is shared across every submesh. A representative is
    // reusable only when its whole ordered material-lighting bundle matches.
    if (m_Meshes.empty() || m_Meshes.size() != other.m_Meshes.size()) return false;
    for (size_t meshIndex = 0u; meshIndex < m_Meshes.size(); ++meshIndex)
    {
        const auto& mesh = m_Meshes[meshIndex];
        const auto& otherMesh = other.m_Meshes[meshIndex];
        if (!mesh || !otherMesh) return false;
        const uint32_t material = mesh->Get_MaterialIndex();
        const uint32_t otherMaterial = otherMesh->Get_MaterialIndex();
        if (material >= m_Materials.size() || otherMaterial >= other.m_Materials.size() ||
            !m_Materials[material] || !other.m_Materials[otherMaterial] ||
            !m_Materials[material]->Has_SameStaticLightingTextures(*other.m_Materials[otherMaterial]))
            return false;
    }
    return true;
}

HRESULT CModel::Bind_StaticLightingBank(const shared_ptr<CShader>& shader,
    std::span<const CModel* const> models, uint32_t meshIndex) const
{
    if (!shader || models.size() < 2u || models.size() > 8u || models.front() != this ||
        meshIndex >= m_Meshes.size() || !m_Meshes[meshIndex])
        return E_INVALIDARG;
    std::array<const CMaterial*, 8> materials{};
    for (size_t i = 0u; i < models.size(); ++i)
    {
        if (!models[i] || !Can_BatchStaticLightingWith(*models[i])) return E_INVALIDARG;
        materials[i] = models[i]->m_Materials[m_Meshes[meshIndex]->Get_MaterialIndex()].get();
    }
    return materials[0]->Bind_StaticLightingBank(shader,
        std::span<const CMaterial* const>(materials.data(), models.size()));
}

HRESULT CModel::Bind_StaticLightingBankMesh(const shared_ptr<CShader>& shader,
    std::span<const CModel* const> models, uint32_t meshIndex) const
{
    if (!shader || models.size() < 2u || models.size() > 8u || models.front() != this ||
        meshIndex >= m_Meshes.size() || !m_Meshes[meshIndex] ||
        m_Meshes[meshIndex]->Has_MorphBaseVertices())
        return E_INVALIDARG;
    const uint32_t materialIndex = m_Meshes[meshIndex]->Get_MaterialIndex();
    std::array<const CMaterial*, 8> materials{};
    for (size_t i = 0u; i < models.size(); ++i)
    {
        const CModel* model = models[i];
        if (!model || m_eType != MODEL::NONANIM || model->m_eType != MODEL::NONANIM ||
            m_StaticCluster || model->m_StaticCluster || m_pDevice != model->m_pDevice ||
            m_pContext != model->m_pContext || m_Meshes.size() != model->m_Meshes.size() ||
            m_Meshes[meshIndex] != model->m_Meshes[meshIndex] ||
            m_fGeometryPreScale != model->m_fGeometryPreScale ||
            materialIndex >= model->m_Materials.size() || !model->m_Materials[materialIndex])
            return E_INVALIDARG;
        for (size_t row = 0u; row < 4u; ++row)
            for (size_t column = 0u; column < 4u; ++column)
                if (m_PreTransformMatrix.m[row][column] != model->m_PreTransformMatrix.m[row][column])
                    return E_INVALIDARG;
        materials[i] = model->m_Materials[materialIndex].get();
    }
    // CMaterial revalidates the current material's entire surface, overrides,
    // non-lighting resources and required lighting inputs before binding SRVs.
    // Other meshes were admitted by the caller and are checked at their own bind.
    return materials[0]->Bind_StaticLightingBank(shader,
        std::span<const CMaterial* const>(materials.data(), models.size()));
}
bool_t CModel::Can_UseOrderedStaticGeometry(
    const uint32_t iFirstMesh, const uint32_t iMeshCount) const
{
    if (MODEL::NONANIM != m_eType || nullptr == m_pOrderedStaticGeometrySource ||
        iMeshCount < 2u || iFirstMesh >= m_Meshes.size() ||
        iMeshCount > m_Meshes.size() - iFirstMesh ||
        m_pOrderedStaticGeometrySource->size() != m_Meshes.size())
        return false;
    for (uint32_t i = iFirstMesh; i < iFirstMesh + iMeshCount; ++i)
    {
        if (nullptr == m_Meshes[i] || m_Meshes[i]->Has_MorphBaseVertices() ||
            (*m_pOrderedStaticGeometrySource)[i].vertexKind != MODEL_VERTEX_KIND::STATIC)
            return false;
    }
    return true;
}

HRESULT CModel::Prepare_OrderedStaticGeometry(
    const uint32_t iFirstMesh, const uint32_t iMeshCount, uint32_t& iOutHandle)
{
    if (iMeshCount == 0u || iFirstMesh >= m_Meshes.size() ||
        iMeshCount > m_Meshes.size() - iFirstMesh)
        return E_INVALIDARG;
    if (!Can_UseOrderedStaticGeometry(iFirstMesh, iMeshCount))
        return S_FALSE;
    for (const auto& Existing : m_OrderedStaticGeometry)
    {
        if (Existing->iFirstMesh == iFirstMesh && Existing->iMeshCount == iMeshCount)
        {
            iOutHandle = Existing->iHandle;
            return S_OK;
        }
    }
    if (m_iNextOrderedStaticGeometryHandle == UINT32_MAX)
        return E_OUTOFMEMORY;
    try
    {
        MODEL_MESH_DATA Combined;
        Combined.name = "ordered-static-geometry";
        Combined.vertexKind = MODEL_VERTEX_KIND::STATIC;
        // Material identity belongs to the preserved ranges, not this carrier.
        Combined.materialIndex = UINT32_MAX;
        Combined.hasColor0 = true;
        uint64_t iVertices = 0u, iIndices = 0u;
        for (uint32_t i = iFirstMesh; i < iFirstMesh + iMeshCount; ++i)
        {
            const auto& Source = (*m_pOrderedStaticGeometrySource)[i];
            if (Source.vertices.empty() || Source.indices.empty() ||
                Source.indices.size() % 3u != 0u ||
                (Source.hasColor0 && Source.color0Rgba8.size() != Source.vertices.size()))
                return E_INVALIDARG;
            iVertices += Source.vertices.size();
            iIndices += Source.indices.size();
        }
        if (iVertices > UINT32_MAX / sizeof(VTXMESH) ||
            iIndices > UINT32_MAX / sizeof(uint32_t))
            return E_INVALIDARG;
        Combined.vertices.reserve(static_cast<size_t>(iVertices));
        Combined.indices.reserve(static_cast<size_t>(iIndices));
        Combined.color0Rgba8.reserve(static_cast<size_t>(iVertices));
        auto Staged = make_shared<ORDERED_STATIC_GEOMETRY>();
        Staged->iHandle = m_iNextOrderedStaticGeometryHandle;
        Staged->iFirstMesh = iFirstMesh;
        Staged->iMeshCount = iMeshCount;
        Staged->Ranges.reserve(iMeshCount);
        for (uint32_t i = iFirstMesh; i < iFirstMesh + iMeshCount; ++i)
        {
            const auto& Source = (*m_pOrderedStaticGeometrySource)[i];
            const uint32_t iBaseVertex = static_cast<uint32_t>(Combined.vertices.size());
            Staged->Ranges.push_back({ i, Source.materialIndex, iBaseVertex,
                static_cast<uint32_t>(Source.vertices.size()),
                static_cast<uint32_t>(Combined.indices.size()),
                static_cast<uint32_t>(Source.indices.size()) });
            Combined.vertices.insert(Combined.vertices.end(),
                Source.vertices.begin(), Source.vertices.end());
            for (size_t v = 0u; v < Source.vertices.size(); ++v)
                Combined.color0Rgba8.push_back(
                    Source.hasColor0 ? Source.color0Rgba8[v] : 0xffffffffu);
            for (const uint32_t iIndex : Source.indices)
            {
                if (iIndex >= Source.vertices.size())
                    return E_INVALIDARG;
                Combined.indices.push_back(iBaseVertex + iIndex);
            }
            Combined.hasTexcoord1 |= Source.hasTexcoord1;
            Combined.hasTexcoord2 |= Source.hasTexcoord2;
        }
        // Reuse the exact existing CMesh static conversion and pretransform.
        // No readback, disk decode, or alternate model runtime is introduced.
        Staged->pMesh = shared_ptr<CMesh>(new CMesh(m_pDevice, m_pContext));
        const MODEL_SKELETON_DATA NoSkeleton{};
        const HRESULT Result = Staged->pMesh->Initialize_Prototype(
            MODEL::NONANIM, Combined, NoSkeleton, XMLoadFloat4x4(&m_PreTransformMatrix));
        if (FAILED(Result))
            return Result;
        m_OrderedStaticGeometry.push_back(Staged);
        ++m_iNextOrderedStaticGeometryHandle;
        iOutHandle = Staged->iHandle;
        return S_OK;
    }
    catch (const std::bad_alloc&) { return E_OUTOFMEMORY; }
    catch (const std::length_error&) { return E_OUTOFMEMORY; }
}

bool_t CModel::Get_OrderedStaticGeometryRanges(const uint32_t iHandle,
    std::span<const ORDERED_STATIC_GEOMETRY_RANGE>& OutRanges) const
{
    for (const auto& Geometry : m_OrderedStaticGeometry)
    {
        if (Geometry->iHandle != iHandle)
            continue;
        if (!Can_UseOrderedStaticGeometry(Geometry->iFirstMesh, Geometry->iMeshCount))
            return false;
        OutRanges = Geometry->Ranges;
        return true;
    }
    return false;
}

HRESULT CModel::Render_OrderedStaticGeometryInstanced(const uint32_t iHandle,
    ID3D11Buffer* pInstanceBuffer, const uint32_t iInstanceStride,
    const uint32_t iNumInstances, const uint32_t iInstanceByteOffset)
{
    for (const auto& Geometry : m_OrderedStaticGeometry)
    {
        if (Geometry->iHandle != iHandle)
            continue;
        if (!Can_UseOrderedStaticGeometry(Geometry->iFirstMesh, Geometry->iMeshCount))
            return S_FALSE;
        const HRESULT drawResult = Geometry->pMesh->Render_Instanced(
            pInstanceBuffer, iInstanceStride, iNumInstances, iInstanceByteOffset);
        if (S_OK == drawResult)
            if (auto* profiler = CGameInstance::Get().Get_Profiler())
                profiler->Record_ModelSubmitted(this);
        return drawResult;
    }
    return E_INVALIDARG;
}

void CModel::Enable_AnimationSampleReuse()
{
    for (const auto& animation : m_Animations)
        if (animation) animation->Enable_SampleReuse();
}

bool_t CModel::Play_Animation(f32_t fTimeDelta)
{
    Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Animation.Play");
    if (m_bExplicitAnimationPose && m_isAnimPaused) return false;
    if (m_Animations.empty() || m_iCurrentAnimIndex >= m_Animations.size())
        return false;

    Engine::CProfilerModelAnimationScope modelAnimationScope(CGameInstance::Get().Get_Profiler(), this);
    // Default scale leaves external local-pose edits untouched. Only a scaled
    // pose needs restoration before an unkeyed channel or blend consumes it.
    if (m_fRootMotionVerticalScale != 1.f && m_iRootMotionBoneIndex >= 0 &&
        static_cast<size_t>(m_iRootMotionBoneIndex) < m_Bones.size() && m_Bones[m_iRootMotionBoneIndex])
    {
        float4x4_t local{};
        XMStoreFloat4x4(&local, m_Bones[m_iRootMotionBoneIndex]->Get_TransformationMatrix());
        Restore_UnscaledRootVertical(local);
        m_Bones[m_iRootMotionBoneIndex]->Update_TransformationMatrix(XMLoadFloat4x4(&local));
    }
    bool_t      isFinished = { false };
    /* 내가 로드한 애니메이션 중, 
    현재 취해야하는 애니메이션의 포즈뼈들의 m_TransformationMatrix를 갱신해준다. */
    /* 일시정지 중에는 재생 위치를 전진시키지 않되, 뼈 행렬은 그대로 다시 계산해
    현재 프레임의 포즈를 유지한다. */
    isFinished = m_Animations[m_iCurrentAnimIndex]->Update_TransformationMatrix(
        m_isAnimPaused ? 0.f : fTimeDelta, m_Bones, m_isAnimLoop);

    Update_AnimBlend(m_isAnimPaused ? 0.f : fTimeDelta);

    if (m_iRootMotionBoneIndex >= 0 &&
        static_cast<size_t>(m_iRootMotionBoneIndex) < m_Bones.size())
    {
        const shared_ptr<CBone>& pRoot = m_Bones[m_iRootMotionBoneIndex];
        if (nullptr != pRoot)
        {
            float4x4_t local{};
            XMStoreFloat4x4(&local, pRoot->Get_TransformationMatrix());
            m_vRootMotionUnscaledTranslation = {local._41, local._42, local._43};
            Apply_RootMotionTranslation(local);
            pRoot->Update_TransformationMatrix(XMLoadFloat4x4(&local));
        }
    }

    /* 뼈들 자체 행렬은 갱신이 됐지만, 최종행렬은 아직 미완성(m_Transformation * Parent`s CombinedTransfor4mationMatrix). */
    Refresh_BoneCombinedMatrices();

    return isFinished;
}

bool_t CModel::Capture_BoneMatrices(const uint32_t iMeshIndex, vector<float4x4_t>& outMatrices) const
{
    if (!Is_Skinned() || iMeshIndex >= m_Meshes.size() || !m_Meshes[iMeshIndex])
        return false;
    const CMesh& mesh = *m_Meshes[iMeshIndex];
    if (mesh.m_iNumBones == 0u || mesh.m_iNumBones > 512u)
        return false;
    vector<float4x4_t> staged(mesh.m_iNumBones);
    mesh.Build_SkinPalette(m_Bones, staged.data());
    if (!std::all_of(staged.begin(), staged.end(), Is_FiniteMatrix))
        return false;
    outMatrices = std::move(staged);
    return true;
}

HRESULT CModel::Bind_BoneMatrices(shared_ptr<class CShader> pShader, const char_t* pConstantName, uint32_t iMeshIndex)
{
    if (nullptr == pShader || iMeshIndex >= m_Meshes.size() || !m_Meshes[iMeshIndex])
        return E_INVALIDARG;

    const CMesh& mesh = *m_Meshes[iMeshIndex];
    // WModel meshes share the full skeleton palette; Assimp meshes retain
    // their own bone subsets and inverse-bind offsets in slots 1..N.
    const size_t paletteIndex = mesh.m_bUsesSkeletonPalette ? 0u : iMeshIndex + 1u;
    if (m_SkinPalettes.size() <= paletteIndex)
        m_SkinPalettes.resize(paletteIndex + 1u);
    SKIN_PALETTE& palette = m_SkinPalettes[paletteIndex];
    if (palette.iPoseRevision != m_iBonePoseRevision)
    {
        Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Animation.SkinPalette.Build");
        palette.Matrices.resize(mesh.m_iNumBones);
        mesh.Build_SkinPalette(m_Bones, palette.Matrices.data());
        palette.iPoseRevision = m_iBonePoseRevision;
    }

    Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Animation.SkinPalette.Bind");
    return pShader->Bind_Matrices(pConstantName, palette.Matrices.data(), mesh.m_iNumBones);
}

HRESULT CModel::Bind_Material(shared_ptr<class CShader> pShader, const char_t* pConstantName, uint32_t iMeshIndex, aiTextureType eType, uint32_t iTextureIndex)
{    
    uint32_t        iMaterialIndex = m_Meshes[iMeshIndex]->Get_MaterialIndex();

    if (iMaterialIndex >= m_iNumMaterials)
        return E_FAIL;

    return m_Materials[iMaterialIndex]->Bind_Material(pShader, pConstantName, eType, iTextureIndex);
}

uint32_t CModel::Override_MaterialTexture(
    const char_t* pMaterialNameFragment,
    const aiTextureType eType,
    const uint32_t iTextureIndex,
    ComPtr<ID3D11ShaderResourceView> pTexture)
{
    if (nullptr == pMaterialNameFragment || 0 == pMaterialNameFragment[0])
        return 0u;

    /* Case-insensitive: the source materials are named inconsistently across rigs and the
       caller should not have to know which spelling a given class shipped. */
    string fragment = pMaterialNameFragment;
    transform(fragment.begin(), fragment.end(), fragment.begin(),
        [](unsigned char c) { return static_cast<char_t>(tolower(c)); });

    uint32_t matched = 0u;
    for (auto& pMaterial : m_Materials)
    {
        if (nullptr == pMaterial)
            continue;
        string name = pMaterial->Get_Name();
        transform(name.begin(), name.end(), name.begin(),
            [](unsigned char c) { return static_cast<char_t>(tolower(c)); });
        if (name.find(fragment) == string::npos)
            continue;
        pMaterial->Set_TextureOverride(eType, iTextureIndex, pTexture);
        ++matched;
    }
    return matched;
}

uint32_t CModel::Override_MaterialDyeColor(
    const char_t* pMaterialNameFragment,
    const float4_t& vDiffuse,
    const float4_t& vRegionA)
{
    if (nullptr == pMaterialNameFragment || 0 == pMaterialNameFragment[0])
        return 0u;

    string fragment = pMaterialNameFragment;
    transform(fragment.begin(), fragment.end(), fragment.begin(),
        [](unsigned char c) { return static_cast<char_t>(tolower(c)); });

    uint32_t matched = 0u;
    for (auto& pMaterial : m_Materials)
    {
        if (nullptr == pMaterial || !pMaterial->Has_DyeColor())
            continue;
        string name = pMaterial->Get_Name();
        transform(name.begin(), name.end(), name.begin(),
            [](unsigned char c) { return static_cast<char_t>(tolower(c)); });
        if (name.find(fragment) == string::npos)
            continue;
        pMaterial->Set_DyeColorOverride(vDiffuse, vRegionA);
        ++matched;
    }
    return matched;
}

namespace
{
    /* Every Override_* below matches a material the same way: case-insensitively, on a
    fragment of its name, so a caller never has to know a class rig's material order. */
    bool_t MaterialNameContains(const string& name, const string& fragment)
    {
        string lowered = name;
        transform(lowered.begin(), lowered.end(), lowered.begin(),
            [](unsigned char c) { return static_cast<char_t>(tolower(c)); });
        return lowered.find(fragment) != string::npos;
    }

    string LoweredFragment(const char_t* pMaterialNameFragment)
    {
        string fragment = nullptr != pMaterialNameFragment ? pMaterialNameFragment : "";
        transform(fragment.begin(), fragment.end(), fragment.begin(),
            [](unsigned char c) { return static_cast<char_t>(tolower(c)); });
        return fragment;
    }
}

uint32_t CModel::Override_SourceCharacterConstants(
    const char_t* pMaterialNameFragment,
    const MODEL_SOURCE_CHARACTER_PARAMETERS& parameters)
{
    const string fragment = LoweredFragment(pMaterialNameFragment);
    if (fragment.empty())
        return 0u;

    uint32_t matched = 0u;
    for (auto& pMaterial : m_Materials)
    {
        if (nullptr == pMaterial || !pMaterial->Has_SourceCharacterProgram() ||
            !MaterialNameContains(pMaterial->Get_Name(), fragment))
            continue;
        if (NativeHairUsesExtraUV(parameters) && m_pMaterialSource)
        {
            const size_t materialIndex = static_cast<size_t>(&pMaterial - m_Materials.data());
            if (any_of(m_pMaterialSource->meshes.begin(), m_pMaterialSource->meshes.end(),
                [&](const auto& mesh) { return mesh.materialIndex == materialIndex && !mesh.hasTexcoord1; }))
                continue;
        }
        if (pMaterial.use_count() > 1) pMaterial = pMaterial->Clone_ForOverrides();
        if (pMaterial->Set_SourceCharacterConstants(parameters))
            ++matched;
    }
    return matched;
}

uint32_t CModel::Override_SourceCharacterTexture(
    const char_t* pMaterialNameFragment, const uint32_t iRegister,
    ComPtr<ID3D11ShaderResourceView> pTexture)
{
    const string fragment = LoweredFragment(pMaterialNameFragment);
    if (fragment.empty())
        return 0u;

    uint32_t matched = 0u;
    for (auto& pMaterial : m_Materials)
    {
        if (nullptr == pMaterial || !pMaterial->Has_SourceCharacterProgram() ||
            !MaterialNameContains(pMaterial->Get_Name(), fragment))
            continue;
        if (pMaterial.use_count() > 1) pMaterial = pMaterial->Clone_ForOverrides();
        if (pMaterial->Set_SourceCharacterTextureOverride(iRegister, pTexture))
            ++matched;
    }
    return matched;
}

void CModel::Clear_SourceCharacterOverrides()
{
    for (auto& pMaterial : m_Materials)
    {
        if (nullptr != pMaterial && pMaterial->Has_SourceCharacterProgram())
        {
            if (pMaterial.use_count() > 1) pMaterial = pMaterial->Clone_ForOverrides();
            pMaterial->Clear_SourceCharacterOverrides();
        }
    }
}

uint32_t CModel::Override_MaterialDiffuseTint(
    const char_t* pMaterialNameFragment, const float4_t& vTint)
{
    if (nullptr == pMaterialNameFragment || 0 == pMaterialNameFragment[0])
        return 0u;

    string fragment = pMaterialNameFragment;
    transform(fragment.begin(), fragment.end(), fragment.begin(),
        [](unsigned char c) { return static_cast<char_t>(tolower(c)); });

    uint32_t matched = 0u;
    for (auto& pMaterial : m_Materials)
    {
        if (nullptr == pMaterial)
            continue;
        string name = pMaterial->Get_Name();
        transform(name.begin(), name.end(), name.begin(),
            [](unsigned char c) { return static_cast<char_t>(tolower(c)); });
        if (name.find(fragment) == string::npos)
            continue;
        pMaterial->Set_DiffuseTint(vTint);
        ++matched;
    }
    return matched;
}

const float4_t* CModel::Get_MaterialDiffuseTint(const uint32_t iMeshIndex) const
{
    if (iMeshIndex >= m_iNumMeshes)
        return nullptr;
    const uint32_t iMaterialIndex = m_Meshes[iMeshIndex]->Get_MaterialIndex();
    if (iMaterialIndex >= m_iNumMaterials || nullptr == m_Materials[iMaterialIndex])
        return nullptr;
    return &m_Materials[iMaterialIndex]->Get_DiffuseTint();
}

uint32_t CModel::Override_MaterialDyeTwoTone(
    const char_t* pMaterialNameFragment, const f32_t fStrength, const f32_t fRange)
{
    if (nullptr == pMaterialNameFragment || 0 == pMaterialNameFragment[0])
        return 0u;

    string fragment = pMaterialNameFragment;
    transform(fragment.begin(), fragment.end(), fragment.begin(),
        [](unsigned char c) { return static_cast<char_t>(tolower(c)); });

    uint32_t matched = 0u;
    for (auto& pMaterial : m_Materials)
    {
        if (nullptr == pMaterial || !pMaterial->Has_DyeColor())
            continue;
        string name = pMaterial->Get_Name();
        transform(name.begin(), name.end(), name.begin(),
            [](unsigned char c) { return static_cast<char_t>(tolower(c)); });
        if (name.find(fragment) == string::npos)
            continue;
        pMaterial->Set_DyeTwoTone(fStrength, fRange);
        ++matched;
    }
    return matched;
}

void CModel::Clear_MaterialDyeColorOverrides()
{
    for (auto& pMaterial : m_Materials)
    {
        if (nullptr != pMaterial)
            pMaterial->Clear_DyeColorOverride();
    }
}

void CModel::Clear_MaterialTextureOverrides()
{
    for (auto& pMaterial : m_Materials)
    {
        if (nullptr != pMaterial)
            pMaterial->Clear_TextureOverrides();
    }
}

bool_t CModel::Has_MaterialTexture(uint32_t iMeshIndex,
    aiTextureType eType,
    uint32_t iTextureIndex) const
{
    if (iMeshIndex >= m_Meshes.size())
        return false;

    const uint32_t materialIndex = m_Meshes[iMeshIndex]->Get_MaterialIndex();
    return materialIndex < m_Materials.size() &&
        m_Materials[materialIndex]->Has_Texture(eType, iTextureIndex);
}

const MODEL_COLOR_TINT* CModel::Get_MaterialColorTint(
    uint32_t iMeshIndex) const
{
    if (iMeshIndex >= m_Meshes.size())
        return nullptr;

    const uint32_t materialIndex = m_Meshes[iMeshIndex]->Get_MaterialIndex();
    if (materialIndex >= m_Materials.size())
        return nullptr;

    return &m_Materials[materialIndex]->Get_ColorTint();
}

HRESULT CModel::Bind_SourceCharacter(shared_ptr<CShader> shader, uint32_t meshIndex)
{
    if (meshIndex >= m_Meshes.size()) return E_INVALIDARG;
    const uint32_t materialIndex = m_Meshes[meshIndex]->Get_MaterialIndex();
    if (materialIndex >= m_Materials.size() || !m_Materials[materialIndex]) return E_INVALIDARG;
    return m_Materials[materialIndex]->Bind_SourceCharacter(shader);
}

HRESULT CModel::Bind_SourceCharacterForwardLight(shared_ptr<CShader> shader, uint32_t meshIndex)
{
    if (meshIndex >= m_Meshes.size()) return E_INVALIDARG;
    const uint32_t materialIndex = m_Meshes[meshIndex]->Get_MaterialIndex();
    if (materialIndex >= m_Materials.size() || !m_Materials[materialIndex]) return E_INVALIDARG;
    return m_Materials[materialIndex]->Bind_SourceCharacterForwardLight(shader);
}

HRESULT CModel::Bind_SourceLandscapeSurface(shared_ptr<CShader> shader, uint32_t meshIndex)
{
    if (!shader || meshIndex >= m_Meshes.size()) return E_INVALIDARG;
    const uint32_t materialIndex = m_Meshes[meshIndex]->Get_MaterialIndex();
    if (materialIndex >= m_Materials.size() || !m_Materials[materialIndex]) return E_INVALIDARG;
    return m_Materials[materialIndex]->Bind_SourceLandscapeSurface(shader);
}

HRESULT CModel::Bind_SourceSpecialSurface(shared_ptr<CShader> shader, uint32_t meshIndex)
{
    if (meshIndex >= m_Meshes.size()) return E_INVALIDARG;
    const uint32_t materialIndex = m_Meshes[meshIndex]->Get_MaterialIndex();
    if (materialIndex >= m_Materials.size()) return E_INVALIDARG;
    return m_Materials[materialIndex]->Bind_SourceSpecialSurface(shader);
}

HRESULT CModel::Bind_SurfaceLighting(shared_ptr<CShader> shader, uint32_t meshIndex)
{
    if (meshIndex >= m_Meshes.size()) return E_INVALIDARG;
    const uint32_t materialIndex = m_Meshes[meshIndex]->Get_MaterialIndex();
    if (materialIndex >= m_Materials.size()) return E_INVALIDARG;
    return m_Materials[materialIndex]->Bind_SurfaceLighting(shader);
}

const MODEL_SURFACE_PARAMETERS* CModel::Get_MaterialSurface(uint32_t iMeshIndex) const
{
	if (iMeshIndex >= m_Meshes.size())
		return nullptr;
	const uint32_t materialIndex = m_Meshes[iMeshIndex]->Get_MaterialIndex();
	return materialIndex < m_Materials.size() ?
		&m_Materials[materialIndex]->Get_Surface() : nullptr;
}

bool_t CModel::Has_MaterialTextureOverrides(uint32_t iMeshIndex) const
{
	if (iMeshIndex >= m_Meshes.size() || !m_Meshes[iMeshIndex])
		return true;
	const uint32_t materialIndex = m_Meshes[iMeshIndex]->Get_MaterialIndex();
	return materialIndex >= m_Materials.size() || !m_Materials[materialIndex] ||
		m_Materials[materialIndex]->Has_TextureOverrides();
}

HRESULT CModel::Bind_SurfaceTexture(shared_ptr<CShader> pShader,
	const char_t* pConstantName, uint32_t iMeshIndex, aiTextureType eType)
{
	if (iMeshIndex >= m_Meshes.size())
		return E_INVALIDARG;
	const uint32_t materialIndex = m_Meshes[iMeshIndex]->Get_MaterialIndex();
	return materialIndex < m_Materials.size() ?
		m_Materials[materialIndex]->Bind_SurfaceTexture(pShader, pConstantName, eType) : E_FAIL;
}

bool_t CModel::Try_GetSourceMaterialIndex(
    const uint32_t iMeshIndex, uint32_t& iOutMaterialIndex) const
{
    if (iMeshIndex >= m_Meshes.size() || nullptr == m_Meshes[iMeshIndex])
        return false;
    const uint32_t iMaterialIndex = m_Meshes[iMeshIndex]->Get_MaterialIndex();
    if (iMaterialIndex >= m_Materials.size())
        return false;
    iOutMaterialIndex = iMaterialIndex;
    return true;
}

const string& CModel::Get_MaterialName(uint32_t iMeshIndex) const
{
	static const string Empty;
	if (iMeshIndex >= m_Meshes.size())
		return Empty;
	const uint32_t materialIndex = m_Meshes[iMeshIndex]->Get_MaterialIndex();
	return materialIndex < m_Materials.size() ?
		m_Materials[materialIndex]->Get_Name() : Empty;
}

uint64_t CModel::Get_MaterialNameHash(uint32_t iMeshIndex) const
{
	if (iMeshIndex >= m_Meshes.size())
		return 0u;
	const uint32_t materialIndex = m_Meshes[iMeshIndex]->Get_MaterialIndex();
	return materialIndex < m_Materials.size() ?
		m_Materials[materialIndex]->Get_NameHash() : 0u;
}

uint32_t CModel::Get_MeshVertexCount(uint32_t iMeshIndex) const
{
	if (iMeshIndex >= m_Meshes.size())
		return 0u;
	return m_Meshes[iMeshIndex]->Get_NumVertices();
}

bool_t CModel::Has_StaticMeshLod(uint32_t iMeshIndex) const
{
	return iMeshIndex < m_Meshes.size() && m_Meshes[iMeshIndex] &&
		m_Meshes[iMeshIndex]->m_StaticLod && !m_Meshes[iMeshIndex]->Has_MorphBaseVertices();
}

uint32_t CModel::Get_StaticMeshLodLevel(uint32_t iMeshIndex, const MESH_SCREEN_LOD_DESC* view) const
{
    if (iMeshIndex >= m_Meshes.size() || !m_Meshes[iMeshIndex])
        return 0u;
    uint32_t indexCount = 0u, firstIndex = 0u;
    return m_Meshes[iMeshIndex]->Select_StaticLod(view, indexCount, firstIndex);
}

uint32_t CModel::Get_StaticMeshSelectedIndexCount(uint32_t iMeshIndex, const MESH_SCREEN_LOD_DESC* view) const
{
    if (iMeshIndex >= m_Meshes.size() || !m_Meshes[iMeshIndex])
        return 0u;
    uint32_t indexCount = 0u, firstIndex = 0u;
    m_Meshes[iMeshIndex]->Select_StaticLod(view, indexCount, firstIndex);
    return indexCount;
}

bool_t CModel::Has_MorphBaseVertices(uint32_t iMeshIndex) const
{
	if (iMeshIndex >= m_Meshes.size())
		return false;
	return m_Meshes[iMeshIndex]->Has_MorphBaseVertices();
}

bool_t CModel::Get_MorphBaseVertex(uint32_t iMeshIndex, uint32_t iVertexIndex,
	float3_t& OutPosition, float3_t& OutNormal) const
{
	if (iMeshIndex >= m_Meshes.size())
		return false;
	return m_Meshes[iMeshIndex]->Get_MorphBaseVertex(iVertexIndex, OutPosition, OutNormal);
}

HRESULT CModel::Make_MeshVertexBuffer_Unique(uint32_t iMeshIndex)
{
	if (iMeshIndex >= m_Meshes.size())
		return E_INVALIDARG;
	return m_Meshes[iMeshIndex]->Make_VertexBuffer_Unique();
}

HRESULT CModel::Update_Mesh_Vertices(uint32_t iMeshIndex, const vector<uint32_t>& iVertexIndices,
	const vector<float3_t>& Positions, const vector<float3_t>& Normals)
{
	if (iMeshIndex >= m_Meshes.size())
		return E_INVALIDARG;
	return m_Meshes[iMeshIndex]->Update_Vertices(iVertexIndices, Positions, Normals);
}

HRESULT CModel::Reset_Mesh_Vertices(uint32_t iMeshIndex)
{
	if (iMeshIndex >= m_Meshes.size())
		return E_INVALIDARG;
	return m_Meshes[iMeshIndex]->Reset_Vertices();
}

HRESULT CModel::Ready_Meshes()
{
    m_iNumMeshes = m_pAIScene->mNumMeshes;

    for (size_t i = 0; i < m_iNumMeshes; i++)
    {
		if (MODEL::NONANIM == m_eType)
		{
			const aiMesh* pAIMesh = m_pAIScene->mMeshes[i];
			for (uint32_t vertexIndex = 0; vertexIndex < pAIMesh->mNumVertices; ++vertexIndex)
			{
				float3_t position{};
				memcpy(&position, &pAIMesh->mVertices[vertexIndex], sizeof(float3_t));
				Include_LocalPosition(XMVector3TransformCoord(
					XMLoadFloat3(&position), XMLoadFloat4x4(&m_PreTransformMatrix)));
			}
		}
        else if (MODEL::ANIM == m_eType)
        {
            const aiMesh* pAIMesh = m_pAIScene->mMeshes[i];
            for (uint32_t vertexIndex = 0; vertexIndex < pAIMesh->mNumVertices; ++vertexIndex)
            {
                float3_t position{};
                memcpy(&position, &pAIMesh->mVertices[vertexIndex], sizeof(float3_t));
                Include_BindGeometryPosition(XMVector3TransformCoord(
                    XMLoadFloat3(&position), XMLoadFloat4x4(&m_PreTransformMatrix)));
            }
        }

        auto pMesh = CMesh::Create(m_pDevice, m_pContext, m_eType, m_pAIScene->mMeshes[i], m_Bones, XMLoadFloat4x4(&m_PreTransformMatrix));
        if (nullptr == pMesh)
            return E_FAIL;

        m_Meshes.push_back(pMesh);
    }

    return S_OK;
}

HRESULT CModel::Ready_Materials(const char_t* pModelFilePath)
{
    m_iNumMaterials = m_pAIScene->mNumMaterials;

    for (uint32_t i = 0; i < m_iNumMaterials; i++)
    {
        auto        pMaterial = CMaterial::Create(m_pDevice, m_pContext, m_pAIScene->mMaterials[i], pModelFilePath);
        if (nullptr == pMaterial)
            return E_FAIL;

        m_Materials.push_back(pMaterial);
    }

    return S_OK;
}

HRESULT CModel::Ready_Bones(const aiNode* pAINode, int32_t iParentBoneIndex)
{
    auto        pBone = CBone::Create(pAINode, iParentBoneIndex);
    if (nullptr == pBone)
        return E_FAIL;

    float4x4_t RestLocal{};
    XMStoreFloat4x4(&RestLocal, pBone->Get_TransformationMatrix());
    m_Bones.push_back(pBone);
    m_BoneRestLocalTransforms.push_back(RestLocal);

    int32_t     iParentIndex = m_Bones.size() - 1;

    for (uint32_t i = 0; i < pAINode->mNumChildren; ++i)
    {
        Ready_Bones(pAINode->mChildren[i], iParentIndex);
    }

    return S_OK;
}

HRESULT CModel::Ready_Animations()
{
    m_iNumAnimations = m_pAIScene->mNumAnimations;

    for (uint32_t i = 0; i < m_iNumAnimations; i++)
    {
        auto      pAnimation = CAnimation::Create(m_pAIScene->mAnimations[i], m_Bones);
        if (nullptr == pAnimation)
            return E_FAIL;

        m_Animations.push_back(pAnimation);
    }

    return S_OK;
}

HRESULT CModel::Ready_BinaryModel(const char_t* pModelFilePath)
{
    MODEL_ASSET_LOAD_DESC desc{};
    desc.meshPath = filesystem::path(pModelFilePath).lexically_normal();

    filesystem::path absolutePath = filesystem::absolute(desc.meshPath).lexically_normal();
    for (filesystem::path current = absolutePath.parent_path();
        !current.empty(); current = current.parent_path())
    {
        if (L"Resources" == current.filename())
        {
            desc.assetRoot = current;
            break;
        }
        if (current == current.root_path())
            break;
    }

    return Ready_BinaryModel(desc);
}

HRESULT CModel::Apply_MaterialOverrides(MODEL_MATERIAL_SOURCE& materialSource, const MODEL_ASSET_LOAD_DESC& loadDesc)
{
	/* Join before allocating meshes/materials. A malformed replacement never
	   changes a shared prototype or an already admitted material instance. */
	vector<string> overriddenNames;
	for (const MODEL_MATERIAL_OVERRIDE& replacement : loadDesc.materialOverrides)
	{
		const auto failOverride = [&](const char* reason)
		{
			OutputDebugStringA(("[CModel] Material override rejected: " +
				loadDesc.meshPath.string() + " / " + replacement.materialName +
				" / " + reason + "\n").c_str());
			return E_INVALIDARG;
		};
        if (replacement.surface.renderMode > MODEL_SURFACE_RENDER_MODE::WATER ||
            replacement.surface.cullMode > MODEL_SURFACE_CULL_MODE::TWO_SIDED)
            return failOverride("invalid material render or cull mode");
        if (!replacement.surface.environmentLegacyEnabled && !replacement.surface.hasSourceIndirect)
            return failOverride("disabled legacy environment requires source indirect inputs");
        if (replacement.surface.hasSourceIndirect)
        {
            const auto& source = replacement.surface;
            if ((source.family != MODEL_SURFACE_FAMILY::PBR_OPAQUE &&
                 source.family != MODEL_SURFACE_FAMILY::PBR_SEAMLESS_OPAQUE) ||
                !source.hasEnvironmentCube || replacement.environmentCubePath.empty() ||
                replacement.environmentBRDFPath.empty() || replacement.sourceIndirectCubePath.empty() ||
                replacement.sourceIndirectBRDFPath.empty())
                return failOverride("source indirect requires PBR environment inputs");
            for (const float value : { source.sourceIndirectColor.x, source.sourceIndirectColor.y,
                source.sourceIndirectColor.z, source.sourceIndirectColor.w })
                if (!std::isfinite(value) || value < 0.f)
                    return failOverride("invalid source indirect environment color");
            const auto& rotation = source.sourceIndirectRotation;
            if (!std::isfinite(rotation.x) || !std::isfinite(rotation.y) ||
                std::abs(rotation.x) > 1.f || std::abs(rotation.y) > 1.f ||
                std::abs(rotation.x*rotation.x+rotation.y*rotation.y-1.f) > 0.0001f)
                return failOverride("invalid source indirect rotation");
            for (const auto& row : source.sourceIndirectSH)
                for (const float value : { row.x, row.y, row.z, row.w })
                    if (!std::isfinite(value) || std::abs(value) > 64.f)
                        return failOverride("invalid source indirect SH component");
            if (source.sourceIndirectSH[6].w != 1.f)
                return failOverride("source indirect SH reserved w must be one");
            const float nonnegative[] = { source.sourceUpperSkyColor.x, source.sourceUpperSkyColor.y,
                source.sourceUpperSkyColor.z, source.sourceLowerSkyColor.x, source.sourceLowerSkyColor.y,
                source.sourceLowerSkyColor.z, source.sourceAmbientAndSkyFactor.x,
                source.sourceAmbientAndSkyFactor.y, source.sourceAmbientAndSkyFactor.z };
            if (any_of(begin(nonnegative), end(nonnegative), [](float value) {
                    return !std::isfinite(value) || value < 0.f || value > 64.f;
                }) || !std::isfinite(source.sourceAmbientAndSkyFactor.w) ||
                source.sourceAmbientAndSkyFactor.w < 0.f || source.sourceAmbientAndSkyFactor.w > 4.f)
                return failOverride("invalid source indirect sky or ambient factor");
        }
		if (replacement.materialName.empty() ||
			find(overriddenNames.begin(), overriddenNames.end(), replacement.materialName) != overriddenNames.end())
			return failOverride("empty or duplicate material name");
        if (replacement.surface.hasStaticShadow)
        {
            const auto& transfer = replacement.surface.staticShadowTransfer;
            const auto root = loadDesc.assetRoot.lexically_normal();
            const auto& path = replacement.staticShadowPath;
            const auto relative = path.lexically_normal().lexically_relative(root);
            if (!replacement.surface.hasBakedLighting || replacement.surface.staticShadowChannel < 1u || replacement.surface.staticShadowChannel > 15u || !root.is_absolute() || !path.is_absolute() ||
                relative.empty() || relative.is_absolute() ||
                any_of(relative.begin(), relative.end(), [](const filesystem::path& part) { return part == ".."; }) ||
                !std::isfinite(transfer.x) || !std::isfinite(transfer.y) || !std::isfinite(transfer.z) ||
                transfer.y < 1.f || transfer.z <= 0.f || transfer.z > 128.f)
                return failOverride("invalid source static shadow inputs");
        }
        if (replacement.surface.sourceFoliageWind)
        {
            const auto& surface = replacement.surface;
            const bool nativeMap = surface.family == MODEL_SURFACE_FAMILY::SOURCE_CHARACTER &&
                surface.sourceCharacter.program >= 1100u && surface.sourceCharacter.program <= 1166u;
            if (!nativeMap && surface.family != MODEL_SURFACE_FAMILY::SOURCE_FOLIAGE_MASKED &&
                surface.family != MODEL_SURFACE_FAMILY::SOURCE_GRASS_MASKED) return failOverride("unsupported wind family");
            const auto finite = [](const float4_t& v) { return std::isfinite(v.x) && std::isfinite(v.y) &&
                std::isfinite(v.z) && std::isfinite(v.w) && std::abs(v.x) <= 1e8f && std::abs(v.y) <= 1e8f &&
                std::abs(v.z) <= 1e8f && std::abs(v.w) <= 1e8f; };
            const auto& b = surface.sourceFoliageWindLocalBounds;
            const auto& w = surface.sourceFoliageWindDirectionSpeed;
            if ((surface.sourceFoliageWindProgram == 1u && !nativeMap && surface.family != MODEL_SURFACE_FAMILY::SOURCE_FOLIAGE_MASKED) ||
                !finite(b) || !finite(w) || !finite(surface.sourceFoliageWindLocalCenter) ||
                !finite(surface.sourceFoliageWindActorPosition) || !finite(surface.sourceFoliageWindPlayerPosition) ||
                surface.sourceFoliageWindLocalCenter.w != 1.f || surface.sourceFoliageWindActorPosition.w != 0.f ||
                b.x < 0.f || b.y < 0.f || b.z < 0.f || b.w <= 0.f || b.x > 1e5f || b.y > 1e5f || b.z > 1e5f || b.w > 1e5f ||
                w.w < 0.f || (w.x == 0.f && w.y == 0.f && w.z == 0.f) ||
                !surface.Has_ValidSourceFoliageWindProgramInputs() ||
                any_of(begin(surface.sourceFoliageWindScalars), end(surface.sourceFoliageWindScalars), [&](const float4_t& v) { return !finite(v); }))
                return failOverride("invalid source foliage wind inputs");
        }
        if (replacement.surface.family == MODEL_SURFACE_FAMILY::SOURCE_LANDSCAPE_OPAQUE)
        {
            const auto& surface = replacement.surface;
            const auto& source = surface.sourceLandscape;
            if (!source.Has_ValidInputs() ||
                surface.hasEnvironmentCube || surface.hasSourceIndirect || surface.hasEmissive ||
                (surface.renderMode != MODEL_SURFACE_RENDER_MODE::INHERIT && surface.renderMode != MODEL_SURFACE_RENDER_MODE::DEFERRED))
                return failOverride("invalid source landscape parameters");
            const auto root = loadDesc.assetRoot.lexically_normal();
            const auto validPath = [&](bool required, const filesystem::path& path) {
                if (!required) return path.empty();
                const auto relative = path.lexically_normal().lexically_relative(root);
                return root.is_absolute() && path.is_absolute() && path.extension() == L".dds" &&
                    !relative.empty() && !relative.is_absolute() &&
                    none_of(relative.begin(), relative.end(), [](const filesystem::path& part) { return part == ".."; });
            };
            const auto& textures = replacement.sourceLandscapeTextures;
            for (uint32_t i = 0u; i < SOURCE_LANDSCAPE_LAYER_COUNT; ++i)
                if (!validPath((source.layerMask & (1u << i)) != 0u, textures.diffuse[i]) ||
                    !validPath((source.normalMask & (1u << i)) != 0u, textures.normal[i]))
                    return failOverride("source landscape layer texture mismatch or escape");
            for (uint32_t i = 0u; i < SOURCE_LANDSCAPE_WEIGHTMAP_COUNT; ++i)
                if (!validPath(i < source.weightmapCount, textures.weightmaps[i])) return failOverride("source landscape weight texture mismatch or escape");
            if (!validPath(true, textures.heightmap)) return failOverride("source landscape height texture missing or escape");
            if (!validPath(surface.hasBakedLighting, replacement.bakedAveragePath) ||
                !validPath(surface.hasBakedLighting, replacement.bakedDirectionalPath) ||
                !validPath(surface.hasStaticShadow, replacement.staticShadowPath))
                return failOverride("source landscape lighting texture mismatch or escape");
            size_t matches = 0u;
            for (size_t materialIndex = 0u; materialIndex < materialSource.materials.size(); ++materialIndex)
            {
                auto& material = materialSource.materials[materialIndex];
                if (material.name != replacement.materialName) continue;
                if (any_of(materialSource.meshes.begin(), materialSource.meshes.end(), [&](const auto& mesh) {
                    return mesh.materialIndex == materialIndex && mesh.vertexKind != MODEL_VERTEX_KIND::STATIC;
                })) return failOverride("source landscape requires a static painted grid");
                material.surface = surface;
                material.sourceLandscapeTextures = textures;
                // Landscape lightmap coordinates derive from the painted grid UV0;
                // the existing placement scale/bias carries the native padding and atlas.
                material.bakedAveragePath = replacement.bakedAveragePath;
                material.bakedDirectionalPath = replacement.bakedDirectionalPath;
                material.staticShadowPath = replacement.staticShadowPath;
                ++matches;
            }
            if (matches != 1u) return failOverride("source landscape material name is absent or ambiguous");
            overriddenNames.push_back(replacement.materialName);
            continue;
        }
        if (replacement.surface.family == MODEL_SURFACE_FAMILY::SOURCE_CHARACTER)
        {
            const auto& source = replacement.surface.sourceCharacter;
            const uint32_t mask = source.baseTextureMask | source.lightTextureMask;
            // These native programs consume constants only in both source passes.
            const bool textureless = source.program == 64u || source.program == 65u ||
                source.program == 1510u || source.program == 1512u;
            if (!Is_SourceCharacterProgramSupported(source.program) ||
                (mask == 0u && !textureless) ||
                (textureless && mask != 0u) || source.requiredExtraUVMask > 3u ||
                (mask >> SOURCE_CHARACTER_TEXTURE_COUNT) != 0u ||
                (replacement.surface.hasBakedLighting && !(source.program == 209u || source.program == 210u || (source.program >= 214u && source.program <= 234u) || source.program == 237u || (source.program >= 1100u && source.program <= 1166u) || (source.program >= 1400u && source.program <= 1413u) || (source.program >= 80u && source.program <= 83u) ||
                    (source.program >= 40u && source.program <= 63u && source.program != 47u && source.program != 53u && source.program != 55u))) ||
                replacement.surface.hasEnvironmentCube)
                return failOverride("invalid source character program or texture mask");
            for (const auto* constants : { &source.baseConstants, &source.lightConstants })
                for (const auto& value : *constants)
                    for (const float scalar : { value.x, value.y, value.z, value.w })
                        if (!std::isfinite(scalar) || std::abs(scalar) > 1000000.f)
                            return failOverride("invalid source character parameter");
            const auto root = loadDesc.assetRoot.lexically_normal();
            if (!root.is_absolute()) return failOverride("source character root is not absolute");
            for (uint32_t index = 0u; index < SOURCE_CHARACTER_TEXTURE_COUNT; ++index)
            {
                const auto& path = replacement.sourceCharacterTextures[index].path;
                if ((mask & (1u << index)) == 0u)
                {
                    if (!path.empty()) return failOverride("unused source character texture");
                    continue;
                }
                const auto relative = path.lexically_normal().lexically_relative(root);
                if (!path.is_absolute() || relative.empty() || relative.is_absolute() ||
                    any_of(relative.begin(), relative.end(), [](const filesystem::path& part) { return part == ".."; }))
                    return failOverride("source character texture escapes the resource root");
            }
            if (replacement.surface.hasBakedLighting)
            {
                for (const auto& path : { replacement.bakedAveragePath, replacement.bakedDirectionalPath })
                {
                    const auto relative = path.lexically_normal().lexically_relative(root);
                    if (!path.is_absolute() || relative.empty() || relative.is_absolute() ||
                        any_of(relative.begin(), relative.end(), [](const filesystem::path& part) { return part == ".."; }))
                        return failOverride("source map monster lighting texture escapes the resource root");
                }
            }
            size_t matches = 0u;
            for (auto& material : materialSource.materials)
            {
                if (material.name != replacement.materialName) continue;
                const size_t materialIndex = static_cast<size_t>(&material - materialSource.materials.data());
                if (any_of(materialSource.meshes.begin(), materialSource.meshes.end(), [&](const auto& mesh) {
                    return mesh.materialIndex == materialIndex &&
                        (((source.requiredExtraUVMask & 1u) != 0u && !mesh.hasTexcoord1) ||
                         ((source.requiredExtraUVMask & 2u) != 0u && !mesh.hasTexcoord2));
                })) return failOverride("source material requires preserved extra UV channels");
                if ((source.program == 5u || NativeHairUsesExtraUV(source) ||
                    source.program == 18u || source.program == 19u) &&
                    any_of(materialSource.meshes.begin(), materialSource.meshes.end(), [&](const auto& mesh) {
                        return mesh.materialIndex == materialIndex &&
                            (!mesh.hasTexcoord1 || (source.program == 5u && !mesh.hasTexcoord2));
                    }))
                    return failOverride("source character requires native extra UV channels");
                if (replacement.surface.hasBakedLighting &&
                    any_of(materialSource.meshes.begin(), materialSource.meshes.end(), [&](const auto& mesh) {
                        return mesh.materialIndex == materialIndex && (!mesh.hasTexcoord1 || mesh.vertexKind != MODEL_VERTEX_KIND::STATIC);
                    })) return failOverride("source map lightmap requires native static UV1");
                // Some multipart weapons repeat one MIC in several material
                // slots. An exact source name intentionally replaces all of them.
                material.surface = replacement.surface;
                material.sourceCharacterTextures = replacement.sourceCharacterTextures;
                material.bakedAveragePath = replacement.bakedAveragePath;
                material.bakedDirectionalPath = replacement.bakedDirectionalPath;
                material.staticShadowPath = replacement.staticShadowPath;
                ++matches;
            }
            if (matches == 0u) return failOverride("source character material name is absent");
            overriddenNames.push_back(replacement.materialName);
            continue;
        }
        size_t matches = 0u;
        for (auto match = materialSource.materials.begin(); match != materialSource.materials.end(); ++match)
        {
        if (match->name != replacement.materialName) continue;
        ++matches;
		const auto& surface = replacement.surface;
        const bool sourceSpecial = surface.family >= MODEL_SURFACE_FAMILY::SOURCE_SNOWICE_OPAQUE &&
            surface.family <= MODEL_SURFACE_FAMILY::SOURCE_WET_OPAQUE;
		if (replacement.hasDiffuseAddressU)
		{
			if (surface.family != MODEL_SURFACE_FAMILY::LEGACY || match->diffusePath.empty())
				return failOverride("diffuse sampler requires an existing legacy diffuse input");
			match->diffuseMirrorU = replacement.diffuseMirrorU;
			continue;
		}
		if (surface.family != MODEL_SURFACE_FAMILY::SPECULAR_TEXTURE_REFLECTION &&
			surface.family != MODEL_SURFACE_FAMILY::DIFFUSE_SPECULAR_REFLECTION &&
			surface.family != MODEL_SURFACE_FAMILY::PBR_SEAMLESS_OPAQUE &&
			surface.family != MODEL_SURFACE_FAMILY::PBR_OPAQUE &&
			surface.family != MODEL_SURFACE_FAMILY::SOURCE_SPECULAR_OPAQUE &&
            surface.family != MODEL_SURFACE_FAMILY::SOURCE_OVERLAY_OPAQUE &&
            surface.family != MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED &&
            surface.family != MODEL_SURFACE_FAMILY::SOURCE_FOLIAGE_MASKED &&
            surface.family != MODEL_SURFACE_FAMILY::SOURCE_GRASS_MASKED && !sourceSpecial)
			return failOverride("unsupported surface family");
		const f32_t scalars[] = { surface.diffuseBrightness,
            surface.family == MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED ? std::abs(surface.normalIntensity) : surface.normalIntensity,
			surface.specularIntensity, surface.specularPower, surface.reflectionIntensity,
			surface.reflectionContrast, surface.reflectionTiling, surface.diffuseSaturation,
			surface.diffuseColor.x, surface.diffuseColor.y, surface.diffuseColor.z, surface.diffuseColor.w,
			surface.specularColor.x, surface.specularColor.y, surface.specularColor.z, surface.specularColor.w,
			surface.reflectionColor.x, surface.reflectionColor.y, surface.reflectionColor.z, surface.reflectionColor.w };
		if (any_of(begin(scalars), end(scalars), [](f32_t value) { return !std::isfinite(value) || value < 0.f; }) ||
			(surface.specularPower < 1.f && surface.family != MODEL_SURFACE_FAMILY::SOURCE_OVERLAY_OPAQUE && surface.family != MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED && surface.family != MODEL_SURFACE_FAMILY::SOURCE_FOLIAGE_MASKED &&
                surface.family != MODEL_SURFACE_FAMILY::SOURCE_GRASS_MASKED && !sourceSpecial) || surface.reflectionTiling <= 0.f)
			return failOverride("invalid surface value");
		const auto root = loadDesc.assetRoot.lexically_normal();
		const auto reflection = replacement.reflectionPath.lexically_normal();
		const auto relative = reflection.lexically_relative(root);
        if (surface.family != MODEL_SURFACE_FAMILY::SOURCE_OVERLAY_OPAQUE &&
            surface.family != MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED &&
            surface.family != MODEL_SURFACE_FAMILY::SOURCE_FOLIAGE_MASKED &&
            surface.family != MODEL_SURFACE_FAMILY::SOURCE_GRASS_MASKED && !sourceSpecial &&
            (!root.is_absolute() || !reflection.is_absolute() || relative.empty() ||
             relative.is_absolute() || any_of(relative.begin(), relative.end(),
                [](const filesystem::path& part) { return part == ".."; })))
			return failOverride("reflection escapes the resource root");
		if (surface.family != MODEL_SURFACE_FAMILY::SOURCE_OVERLAY_OPAQUE &&
            surface.family != MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED &&
            surface.family != MODEL_SURFACE_FAMILY::SOURCE_FOLIAGE_MASKED &&
            surface.family != MODEL_SURFACE_FAMILY::SOURCE_GRASS_MASKED && !sourceSpecial &&
            surface.family != MODEL_SURFACE_FAMILY::PBR_OPAQUE &&
            surface.family != MODEL_SURFACE_FAMILY::PBR_SEAMLESS_OPAQUE &&
            (match->diffusePath.empty() || match->normalPath.empty() ||
			(surface.family == MODEL_SURFACE_FAMILY::SPECULAR_TEXTURE_REFLECTION && match->specularPath.empty())))
			return failOverride("required material input is absent");
		if (surface.family == MODEL_SURFACE_FAMILY::PBR_SEAMLESS_OPAQUE ||
			surface.family == MODEL_SURFACE_FAMILY::PBR_OPAQUE)
		{
			const f32_t pbr[] = { surface.uvTiling.x, surface.uvTiling.y,
				surface.detailNormalIntensity, surface.detailNormalTiling,
				surface.metallicIntensity, surface.metallicPower, surface.roughnessIntensity,
				surface.roughnessPower, surface.aoIntensity, surface.aoPower,
				surface.specularPBRIntensity, surface.nonmetallicBrightness, surface.metallicBrightness,
				surface.minimumRoughness, surface.vertexAlpha };
			if (any_of(begin(pbr), end(pbr), [](f32_t v) { return !std::isfinite(v) || v < 0.f; }) ||
				surface.uvTiling.x <= 0.f || surface.uvTiling.y <= 0.f || surface.detailNormalTiling <= 0.f ||
				surface.minimumRoughness <= 0.f || surface.minimumRoughness > 1.f || surface.vertexAlpha > 1.f ||
				!std::isfinite(surface.reflectionOriginOffset.x) || !std::isfinite(surface.reflectionOriginOffset.y))
				return failOverride("invalid PBR value");
			const filesystem::path inputs[] = { replacement.surfaceDiffusePath, replacement.surfaceNormalPath,
				replacement.detailNormalPath, replacement.surfaceORMPath };
			for (const auto& path : inputs)
			{
				const auto rel = path.lexically_normal().lexically_relative(root);
				if (!path.is_absolute() || rel.empty() || rel.is_absolute() ||
					any_of(rel.begin(), rel.end(), [](const filesystem::path& p) { return p == ".."; }))
					return failOverride("PBR texture escapes the resource root");
			}
			match->surfaceDiffusePath = replacement.surfaceDiffusePath;
			match->surfaceNormalPath = replacement.surfaceNormalPath;
			match->detailNormalPath = replacement.detailNormalPath;
			match->surfaceORMPath = replacement.surfaceORMPath;
		}
        if (surface.family == MODEL_SURFACE_FAMILY::SOURCE_FOLIAGE_MASKED ||
            surface.family == MODEL_SURFACE_FAMILY::SOURCE_GRASS_MASKED)
        {
            const uint32_t flags = surface.sourceFoliageFlags;
            const auto& transmission = surface.sourceFoliageTransmission;
            const float values[] = { transmission.x, transmission.y, transmission.z, transmission.w };
            if ((flags & ~127u) != 0u || ((flags & 8u) && !(flags & 4u)) ||
                ((flags & 64u) && !(flags & 32u)) || surface.hasEnvironmentCube ||
                surface.hasEmissive != ((flags & 32u) != 0u) ||
                (surface.family == MODEL_SURFACE_FAMILY::SOURCE_GRASS_MASKED && (flags & (1u | 8u))) ||
                any_of(begin(values), end(values), [](float v) { return !std::isfinite(v) || v < 0.f; }))
                return failOverride("invalid source foliage branch or parameter");
            const std::pair<const filesystem::path*, filesystem::path*> inputs[] = {
                { &replacement.surfaceDiffusePath, &match->surfaceDiffusePath },
                { &replacement.surfaceNormalPath, &match->surfaceNormalPath },
                { &replacement.surfaceSpecularPath, &match->surfaceSpecularPath },
                { &replacement.sourceFoliageMaskPath, &match->sourceFoliageMaskPath }
            };
            const bool required[] = { true, (flags & 1u) != 0u, (flags & 8u) != 0u,
                surface.family == MODEL_SURFACE_FAMILY::SOURCE_FOLIAGE_MASKED };
            for (size_t i = 0; i < size(inputs); ++i)
            {
                const auto& path = *inputs[i].first;
                if (path.empty()) { if (required[i]) return failOverride("source foliage selected input missing"); continue; }
                const auto rel = path.lexically_normal().lexically_relative(root);
                if (!root.is_absolute() || !path.is_absolute() || rel.empty() || rel.is_absolute() ||
                    any_of(rel.begin(), rel.end(), [](const filesystem::path& p) { return p == ".."; }))
                    return failOverride("source foliage texture escapes the resource root");
                *inputs[i].second = path;
            }
        }
        if (sourceSpecial)
        {
            const auto& special = surface.sourceSpecial;
            const bool ice = surface.family == MODEL_SURFACE_FAMILY::SOURCE_SNOWICE_OPAQUE;
            const bool blend = surface.family == MODEL_SURFACE_FAMILY::SOURCE_VERTEXBLEND_OPAQUE;
            if ((special.flags & ~3u) || (!ice && !blend && special.flags) || surface.hasEnvironmentCube || surface.hasEmissive)
                return failOverride("invalid source special branch");
            for (const auto& value : { special.iceCoreColor, special.iceOuterColor, special.iceBlend,
                special.wetParameters, surface.sourceBgRimlight })
                for (float v : { value.x, value.y, value.z, value.w })
                    if (!std::isfinite(v) || v < 0.f) return failOverride("invalid source special vector");
            for (const auto* layers : { &special.blendDiffuse, &special.blendSpecular, &special.blendLayers })
                for (const auto& value : *layers)
                    for (float v : { value.x, value.y, value.z, value.w })
                        if (!std::isfinite(v) || v < 0.f) return failOverride("invalid source blend layer");
            for (float v : { special.normalTiling, special.wetSpecularPower, special.blendSharpness,
                surface.detailNormalIntensity, surface.detailNormalTiling, surface.uvTiling.x, surface.uvTiling.y })
                if (!std::isfinite(v) || v < 0.f) return failOverride("invalid source special scalar");
            if (!std::isfinite(special.iceBumpOffset) || special.normalTiling <= 0.f ||
                surface.uvTiling.x <= 0.f || surface.uvTiling.y <= 0.f || surface.detailNormalTiling <= 0.f)
                return failOverride("invalid source special UV or parallax");
            if (blend) for (size_t i = 0; i < 4; ++i)
                if ((i < 2 || (special.flags & (1u << (i - 2)))) &&
                    (special.blendLayers[i].x <= 0.f || special.blendLayers[i].y <= 0.f))
                    return failOverride("invalid source blend UV");
            const std::pair<const filesystem::path*, filesystem::path*> inputs[] = {
                { &replacement.surfaceDiffusePath, &match->surfaceDiffusePath },
                { &replacement.surfaceNormalPath, &match->surfaceNormalPath },
                { &replacement.surfaceSpecularPath, &match->surfaceSpecularPath },
                { &replacement.reflectionPath, &match->reflectionPath },
                { &replacement.detailNormalPath, &match->detailNormalPath },
                { &replacement.overlayDiffusePath, &match->overlayDiffusePath },
                { &replacement.overlayNormalPath, &match->overlayNormalPath },
                { &replacement.sourceSpecialMaskPath, &match->sourceSpecialMaskPath },
                { &replacement.sourceBlendDiffuseGPath, &match->sourceBlendDiffuseGPath },
                { &replacement.sourceBlendNormalGPath, &match->sourceBlendNormalGPath },
                { &replacement.sourceBlendDiffuseBPath, &match->sourceBlendDiffuseBPath },
                { &replacement.sourceBlendNormalBPath, &match->sourceBlendNormalBPath }
            };
            const bool required[] = { true, true, !blend && (!ice || (special.flags & 2u)), !blend,
                blend || (ice && (special.flags & 1u)), blend, blend, ice,
                blend && (special.flags & 1u), blend && (special.flags & 1u),
                blend && (special.flags & 2u), blend && (special.flags & 2u) };
            for (size_t i = 0; i < size(inputs); ++i)
            {
                const auto& path = *inputs[i].first;
                if (path.empty()) { if (required[i]) return failOverride("source special input missing"); continue; }
                if (!required[i]) return failOverride("source special inactive input supplied");
                const auto rel = path.lexically_normal().lexically_relative(root);
                if (!root.is_absolute() || !path.is_absolute() || rel.empty() || rel.is_absolute() ||
                    any_of(rel.begin(), rel.end(), [](const filesystem::path& p) { return p == ".."; }))
                    return failOverride("source special texture escapes the resource root");
                *inputs[i].second = path;
            }
        }
        if (surface.family == MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED)
        {
            const uint32_t flags = surface.sourceBgFlags;
            const float viewValues[] = { surface.sourceBgSubspecular.x, surface.sourceBgSubspecular.y,
                surface.sourceBgRimlight.x, surface.sourceBgRimlight.y, surface.sourceBgRimlight.z,
                surface.sourceBgRimlight.w, surface.sourceBgSpecularSaturation };
            if (any_of(begin(viewValues), end(viewValues), [](float v) { return !std::isfinite(v) || v < 0.f; }) ||
                !std::isfinite(surface.sourceBgPanning.x) || !std::isfinite(surface.sourceBgPanning.y))
                return failOverride("invalid source BG view lighting or panning");
            const float values[] = { surface.sourceBgBump.x, surface.sourceBgBump.y,
                surface.sourceBgBump.z, surface.sourceBgUV.x, surface.sourceBgUV.y,
                surface.sourceBgUV.z, surface.sourceBgUV.w, surface.uvTiling.x, surface.uvTiling.y,
                surface.reflectionOriginOffset.x, surface.reflectionOriginOffset.y };
            if ((flags & ~65535u) != 0u || ((flags & 8u) != 0u && (flags & 4u) == 0u &&
                surface.sourceBgSubspecular.x <= 0.f) ||
                ((flags & 32u) != 0u && (flags & 16u) == 0u) || surface.sourceBgFlicker > 2u ||
                surface.hasEnvironmentCube ||
                any_of(begin(values), end(values), [](float v) { return !std::isfinite(v); }) ||
                std::abs(surface.sourceBgUV.x * surface.sourceBgUV.x +
                    surface.sourceBgUV.y * surface.sourceBgUV.y - 1.f) > 0.0001f)
                return failOverride("invalid source BG branch or parameter");
            const std::pair<const filesystem::path*, filesystem::path*> inputs[] = {
                { &replacement.surfaceDiffusePath, &match->surfaceDiffusePath },
                { &replacement.surfaceNormalPath, &match->surfaceNormalPath },
                { &replacement.surfaceSpecularPath, &match->surfaceSpecularPath },
                { &replacement.reflectionPath, &match->reflectionPath }
            };
            const bool required[] = { true, (flags & 1u) != 0u, (flags & 8u) != 0u, (flags & 16u) != 0u };
            for (size_t i = 0; i < size(inputs); ++i)
            {
                const auto& path = *inputs[i].first;
                if (path.empty()) { if (required[i]) return failOverride("source BG selected input missing"); continue; }
                const auto rel = path.lexically_normal().lexically_relative(root);
                if (!root.is_absolute() || !path.is_absolute() || rel.empty() || rel.is_absolute() ||
                    any_of(rel.begin(), rel.end(), [](const filesystem::path& p) { return p == ".."; }))
                    return failOverride("source BG texture escapes the resource root");
                *inputs[i].second = path;
            }
            if ((flags & 32768u) != 0u)
            {
                const auto& path = replacement.detailNormalPath;
                const auto rel = path.lexically_normal().lexically_relative(root);
                if (!path.is_absolute() || rel.empty() || rel.is_absolute() ||
                    any_of(rel.begin(), rel.end(), [](const filesystem::path& p) { return p == ".."; }) ||
                    !std::isfinite(surface.detailNormalIntensity) || !std::isfinite(surface.detailNormalTiling))
                    return failOverride("invalid source detail normal");
                match->detailNormalPath = path;
            }
            // Sampler identity accompanies the source diffuse, including the old explicit mirror rows.
            match->diffuseMirrorU = replacement.diffuseMirrorU;
        }
        if (surface.family == MODEL_SURFACE_FAMILY::SOURCE_SPECULAR_OPAQUE)
        {
            if (!std::isfinite(surface.uvTiling.x) || !std::isfinite(surface.uvTiling.y) ||
                surface.uvTiling.x <= 0.f || surface.uvTiling.y <= 0.f ||
                !std::isfinite(surface.reflectionOriginOffset.x) || !std::isfinite(surface.reflectionOriginOffset.y) ||
                surface.hasEnvironmentCube)
                return failOverride("invalid source specular UV or unsupported environment");
            const std::pair<const filesystem::path*, filesystem::path*> inputs[] = {
                { &replacement.surfaceDiffusePath, &match->surfaceDiffusePath },
                { &replacement.surfaceNormalPath, &match->surfaceNormalPath },
                { &replacement.surfaceSpecularPath, &match->surfaceSpecularPath }
            };
            for (const auto& input : inputs)
            {
                const auto& path = *input.first;
                const auto rel = path.lexically_normal().lexically_relative(root);
                if (!path.is_absolute() || rel.empty() || rel.is_absolute() ||
                    any_of(rel.begin(), rel.end(), [](const filesystem::path& p) { return p == ".."; }))
                    return failOverride("source specular texture escapes the resource root");
                *input.second = path;
            }
        }
        if (surface.family == MODEL_SURFACE_FAMILY::SOURCE_OVERLAY_OPAQUE)
        {
            const uint32_t materialIndex = static_cast<uint32_t>(distance(materialSource.materials.begin(), match));
            if (any_of(materialSource.meshes.begin(), materialSource.meshes.end(), [&](const auto& mesh) {
                return mesh.materialIndex == materialIndex &&
                    (mesh.vertexKind != MODEL_VERTEX_KIND::STATIC || !mesh.hasTangentHandedness);
            })) return failOverride("source overlay requires preserved static geometry and tangent handedness");
            const float values[] = { surface.overlayColor.x, surface.overlayColor.y, surface.overlayColor.z,
                surface.overlayColor.w, surface.overlayTiling, surface.overlayNormalIntensity,
                surface.overlaySharpness, surface.overlayBrightness, surface.overlaySaturation,
                surface.overlaySpecularIntensity, surface.uvTiling.x, surface.uvTiling.y, surface.detailNormalIntensity, surface.detailNormalTiling,
                surface.sourceBgSubspecular.x, surface.sourceBgSubspecular.y, surface.sourceBgSpecularSaturation };
            if (any_of(begin(values), end(values), [](float v) { return !std::isfinite(v) || v < 0.f; }) ||
                surface.sourceOverlayFlags > 2047u || surface.overlayTiling <= 0.f || surface.uvTiling.x <= 0.f || surface.uvTiling.y <= 0.f || surface.hasEnvironmentCube ||
                !replacement.reflectionPath.empty()) return failOverride("invalid source overlay surface");
            const float signedValues[] = { surface.sourceOverlayDirection.x, surface.sourceOverlayDirection.y,
                surface.sourceOverlayDirection.z, surface.sourceOverlayDirection.w,
                surface.sourceBgUV.x, surface.sourceBgUV.y, surface.sourceBgUV.z, surface.sourceBgUV.w,
                surface.sourceBgBump.x, surface.sourceBgBump.y, surface.sourceBgBump.z, surface.sourceBgBump.w };
            if (any_of(begin(signedValues), end(signedValues), [](float v) { return !std::isfinite(v); }) ||
                (((surface.sourceOverlayFlags & 32u) != 0u) != !replacement.detailNormalPath.empty()))
                return failOverride("invalid source overlay direction, UV or detail branch");
            if (surface.hasEmissive && (surface.emissiveFlickerMinimum != 0.f ||
                surface.emissiveFlickerSpeed != 0.f || surface.emissivePhaseOffset != 0.f))
                return failOverride("unsupported source overlay emissive flicker");
            if (surface.overlaySeparateSpecular)
            {
                const auto path = replacement.surfaceSpecularPath.lexically_normal();
                const auto rel = path.lexically_relative(root);
                if (!root.is_absolute() || !path.is_absolute() || rel.empty() || rel.is_absolute() ||
                    any_of(rel.begin(), rel.end(), [](const filesystem::path& p) { return p == ".."; }))
                    return failOverride("source overlay specular escapes the resource root");
                match->surfaceSpecularPath = path;
            }
            const std::pair<const filesystem::path*, filesystem::path*> inputs[] = {
                { &replacement.surfaceDiffusePath, &match->surfaceDiffusePath },
                { &replacement.surfaceNormalPath, &match->surfaceNormalPath },
                { &replacement.overlayDiffusePath, &match->overlayDiffusePath },
                { &replacement.overlayNormalPath, &match->overlayNormalPath },
                { &replacement.detailNormalPath, &match->detailNormalPath }
            };
            for (const auto& input : inputs)
            {
                const auto& path = *input.first;
                if (path.empty() && ((input.first == &replacement.surfaceNormalPath && (surface.sourceOverlayFlags & 1u) == 0u) ||
                    (input.first == &replacement.overlayNormalPath && (surface.sourceOverlayFlags & 2u) == 0u) ||
                    (input.first == &replacement.detailNormalPath && (surface.sourceOverlayFlags & 32u) == 0u))) continue;
                const auto rel = path.lexically_normal().lexically_relative(root);
                if (!root.is_absolute() || !path.is_absolute() || rel.empty() || rel.is_absolute() ||
                    any_of(rel.begin(), rel.end(), [](const filesystem::path& p) { return p == ".."; }))
                    return failOverride("source overlay texture escapes the resource root");
                *input.second = path;
            }
        }
        if (surface.hasBakedLighting)
        {
            const uint32_t materialIndex = static_cast<uint32_t>(distance(materialSource.materials.begin(), match));
            if (any_of(materialSource.meshes.begin(), materialSource.meshes.end(), [&](const auto& mesh) {
                return mesh.materialIndex == materialIndex && (!mesh.hasTexcoord1 || mesh.vertexKind != MODEL_VERTEX_KIND::STATIC);
            })) return failOverride("baked lighting requires preserved static TEXCOORD1");
        }
        const float environmentValues[] = { surface.environmentColor.x, surface.environmentColor.y,
            surface.environmentColor.z, surface.environmentColor.w, surface.environmentRotation.x, surface.environmentRotation.y };
        if (surface.hasEnvironmentCube &&
            (any_of(begin(environmentValues), end(environmentValues), [](float v) { return !std::isfinite(v); }) ||
             surface.environmentColor.x < 0.f || surface.environmentColor.y < 0.f || surface.environmentColor.z < 0.f ||
             std::abs(surface.environmentRotation.x*surface.environmentRotation.x + surface.environmentRotation.y*surface.environmentRotation.y-1.f) > 0.0001f))
            return failOverride("invalid environment color or rotation");
        const std::pair<const filesystem::path*, filesystem::path*> lightingPaths[] = {
            { &replacement.bakedAveragePath, &match->bakedAveragePath },
            { &replacement.bakedDirectionalPath, &match->bakedDirectionalPath },
            { &replacement.staticShadowPath, &match->staticShadowPath },
            { &replacement.environmentCubePath, &match->environmentCubePath },
            { &replacement.environmentBRDFPath, &match->environmentBRDFPath },
            { &replacement.sourceIndirectCubePath, &match->sourceIndirectCubePath },
            { &replacement.sourceIndirectBRDFPath, &match->sourceIndirectBRDFPath }
        };
        if ((surface.hasBakedLighting && (replacement.bakedAveragePath.empty() || replacement.bakedDirectionalPath.empty())) ||
            (surface.hasEnvironmentCube && (replacement.environmentCubePath.empty() || replacement.environmentBRDFPath.empty())))
            return failOverride("missing lighting texture");
        for (const auto& path : lightingPaths)
        {
            if (path.first->empty()) continue;
            const auto relative = path.first->lexically_normal().lexically_relative(root);
            if (!path.first->is_absolute() || relative.empty() || relative.is_absolute() ||
                any_of(relative.begin(), relative.end(), [](const filesystem::path& p) { return p == ".."; }))
                return failOverride("lighting texture escapes the resource root");
            *path.second = *path.first;
        }
		if (surface.hasEmissive)
		{
			const f32_t emissionValues[] = { surface.emissiveColor.x, surface.emissiveColor.y,
				surface.emissiveColor.z, surface.emissiveColor.w, surface.emissiveIntensity,
				surface.emissiveUVTiling.x, surface.emissiveUVTiling.y,
				surface.emissiveFlickerMinimum, surface.emissiveFlickerSpeed };
			if ((surface.family != MODEL_SURFACE_FAMILY::PBR_SEAMLESS_OPAQUE &&
				surface.family != MODEL_SURFACE_FAMILY::PBR_OPAQUE &&
                surface.family != MODEL_SURFACE_FAMILY::SOURCE_OVERLAY_OPAQUE &&
                surface.family != MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED &&
            surface.family != MODEL_SURFACE_FAMILY::SOURCE_FOLIAGE_MASKED &&
            surface.family != MODEL_SURFACE_FAMILY::SOURCE_GRASS_MASKED && !sourceSpecial) ||
				any_of(begin(emissionValues), end(emissionValues),
					[](f32_t value) { return !std::isfinite(value) || value < 0.f; }) ||
				surface.emissiveUVTiling.x <= 0.f || surface.emissiveUVTiling.y <= 0.f ||
				(surface.family != MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED &&
            surface.family != MODEL_SURFACE_FAMILY::SOURCE_FOLIAGE_MASKED &&
            surface.family != MODEL_SURFACE_FAMILY::SOURCE_GRASS_MASKED && surface.emissiveFlickerMinimum > 1.f) || !std::isfinite(surface.emissivePhaseOffset))
				return failOverride("invalid PBR emissive value");
			const auto emissive = replacement.surfaceEmissivePath.lexically_normal();
			const auto emissiveRelative = emissive.lexically_relative(root);
			if (!emissive.is_absolute() || emissiveRelative.empty() || emissiveRelative.is_absolute() ||
				any_of(emissiveRelative.begin(), emissiveRelative.end(),
					[](const filesystem::path& part) { return part == ".."; }))
				return failOverride("emissive texture escapes the resource root");
			match->surfaceEmissivePath = emissive;
		}
		match->surface = surface;
		match->reflectionPath = reflection;
        }
        if (matches == 0u) return failOverride("material name is absent");
		overriddenNames.push_back(replacement.materialName);
	}
	return S_OK;
}

HRESULT CModel::Ready_BinaryModel(
	const MODEL_ASSET_LOAD_DESC& loadDesc)
{
    Engine::CProfilerScope loadScope(CGameInstance::Get().Get_Profiler(), "Model.Load.Binary");
	MODEL_ASSET_DATA asset{};
	bool decoded = false;
	{
		Engine::CProfilerScope decodeScope(CGameInstance::Get().Get_Profiler(), "Model.Load.Decode");
		decoded = CModelDecoderRegistry::Get().Decode(loadDesc, asset);
	}
	if (!decoded)
	{
		const MODEL_DECODE_REPORT report = CModelDecoderRegistry::Get().Get_LastReport();
		OutputDebugStringA(("[CModel] Binary decode failed for " +
			loadDesc.meshPath.string() + ": " + report.error + "\n").c_str());
		return E_FAIL;
	}
	if (asset.meshes.empty() ||
		((MODEL::ANIM == m_eType) != asset.hasSkeleton))
	{
		OutputDebugStringA(("[CModel] Binary decode produced an unusable asset for " +
			loadDesc.meshPath.string() + " (meshes=" +
			std::to_string(asset.meshes.size()) + ", hasSkeleton=" +
			(asset.hasSkeleton ? "true" : "false") + ").\n").c_str());
		return E_FAIL;
	}
    auto materialSource = std::make_shared<MODEL_MATERIAL_SOURCE>();
    materialSource->identity = loadDesc;
    materialSource->identity.materialOverrides.clear();
    materialSource->materials = asset.materials;
    materialSource->meshes.reserve(asset.meshes.size());
    for (const auto& mesh : asset.meshes)
        materialSource->meshes.push_back({ mesh.materialIndex, mesh.vertexKind,
            mesh.hasColor0, mesh.hasTexcoord1, mesh.hasTexcoord2, !mesh.tangentHandedness.empty() });
    // Keep only small material rows/channel facts, never decoded vertex/index arrays.
    m_pMaterialSource = materialSource;
    MODEL_MATERIAL_SOURCE staged = *materialSource;
    if (FAILED(Apply_MaterialOverrides(staged, loadDesc))) return E_INVALIDARG;
    asset.materials = std::move(staged.materials);
	m_iSkeletonHash = asset.hasSkeleton ? asset.skeleton.skeletonHash : 0;

	m_bHasSelfConsistentUnauthenticatedGeometryMetadata =
		asset.geometryMetadata.present;
	m_iGeometryFormatVersionMajor = {};
	m_iGeometryFormatVersionMinor = {};
	m_iGeometryChannelMask = {};
	m_iGeometryEvidenceFlags = {};
	m_fGeometryPreScale = 1.f;
	m_GeometryPayloadSha256.fill(0);
	m_GeometryMetadataIdentitySha256.fill(0);
	if (m_bHasSelfConsistentUnauthenticatedGeometryMetadata)
	{
		m_iGeometryFormatVersionMajor = asset.geometryMetadata.versionMajor;
		m_iGeometryFormatVersionMinor = asset.geometryMetadata.versionMinor;
		m_iGeometryChannelMask = asset.geometryMetadata.channelMask;
		m_iGeometryEvidenceFlags = asset.geometryMetadata.evidenceFlags;
		m_fGeometryPreScale = asset.geometryMetadata.geometryPreScale;
		m_GeometryPayloadSha256 = asset.geometryMetadata.payloadSha256;
		m_GeometryMetadataIdentitySha256 =
			asset.geometryMetadata.metadataIdentitySha256;
	}

	if (FAILED(Ready_Bones(asset)) ||
		FAILED(Ready_Meshes(asset)) ||
		FAILED(Ready_Materials(asset)) ||
		FAILED(Ready_Animations(asset)))
	{
		return E_FAIL;
	}

	Refresh_BoneCombinedMatrices();

	if (!asset.animations.empty())
	{
		uint32_t animationIndex = {};
		if (!loadDesc.defaultAnimationName.empty())
		{
			const auto iterator = find_if(
				asset.animations.begin(), asset.animations.end(),
				[&loadDesc](const MODEL_ANIMATION_DATA& animation)
				{
					return animation.name ==
						loadDesc.defaultAnimationName;
				});
			if (iterator == asset.animations.end())
				return E_FAIL;
			animationIndex = static_cast<uint32_t>(distance(
				asset.animations.begin(), iterator));
		}
		if (!Start_Animation(
			animationIndex,
			asset.animations[animationIndex].defaultLoop))
		{
			return E_FAIL;
		}
	}
	return S_OK;
}

HRESULT CModel::Ready_Meshes(const MODEL_ASSET_DATA& asset)
{
    Engine::CProfilerScope loadScope(CGameInstance::Get().Get_Profiler(), "Model.Load.Meshes");
    m_iNumMeshes = static_cast<uint32_t>(asset.meshes.size());
    m_Meshes.reserve(m_iNumMeshes);
	for (const MODEL_MESH_DATA& mesh : asset.meshes)
	{
		if (MODEL::NONANIM == m_eType)
		{
			/* Runtime culling bounds are derived from every decoded vertex. The
			   embedded AABB is validated import metadata, but it must not be the
			   sole authority for a placement disappearing at the camera edge. */
			for (const VTXMESH& vertex : mesh.vertices)
			{
				Include_LocalPosition(XMVector3TransformCoord(
					XMLoadFloat3(&vertex.vPosition),
					XMLoadFloat4x4(&m_PreTransformMatrix)));
			}
		}
        else if (MODEL::ANIM == m_eType)
        {
            for (const VTXANIMMESH& vertex : mesh.skinnedVertices)
                Include_BindGeometryPosition(XMVector3TransformCoord(
                    XMLoadFloat3(&vertex.vPosition), XMLoadFloat4x4(&m_PreTransformMatrix)));
        }

        auto pMesh = CMesh::Create(m_pDevice, m_pContext, m_eType,
            mesh, asset.skeleton, XMLoadFloat4x4(&m_PreTransformMatrix));
        if (nullptr == pMesh)
            return E_FAIL;
        // Only undeformed native opaque map geometry opts into additional LOD
        // indices. Other models retain their original buffer and render contract.
        if (MODEL::NONANIM == m_eType && mesh.materialIndex < asset.materials.size())
        {
            const auto& material = asset.materials[mesh.materialIndex];
            const auto& surface = material.surface;
            if (surface.family == MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED &&
                !surface.sourceFoliageWind && (surface.sourceBgFlags & 64u) == 0u && material.opacityPath.empty() &&
                (surface.renderMode == MODEL_SURFACE_RENDER_MODE::INHERIT ||
                 surface.renderMode == MODEL_SURFACE_RENDER_MODE::DEFERRED))
            {
                const HRESULT lodResult = pMesh->Prepare_StaticLod(mesh, XMLoadFloat4x4(&m_PreTransformMatrix));
                if (FAILED(lodResult))
                {
                    char message[192]{};
                    sprintf_s(message, "[Engine][MeshLOD] optional index LOD unavailable hr=0x%08X; keeping original mesh\n", static_cast<uint32_t>(lodResult));
                    OutputDebugStringA(message);
                }
            }
        }
        m_Meshes.push_back(pMesh);
    }
    // Keep source geometry only for explicitly opted-in multi-submesh effects.
    // Clones share it; ordinary map/character models pay no retained CPU cost.
    if (m_bRetainOrderedStaticGeometry && MODEL::NONANIM == m_eType &&
        asset.meshes.size() > 1u)
    {
        try
        {
            m_pOrderedStaticGeometrySource =
                make_shared<const vector<MODEL_MESH_DATA>>(asset.meshes);
        }
        catch (const std::bad_alloc&) { return E_OUTOFMEMORY; }
        catch (const std::length_error&) { return E_OUTOFMEMORY; }
    }
    return S_OK;
}

void CModel::Reset_LocalBounds()
{
	const f32_t maximum = (numeric_limits<f32_t>::max)();
	m_vLocalBoundsMin = float3_t(maximum, maximum, maximum);
	m_vLocalBoundsMax = float3_t(-maximum, -maximum, -maximum);
	m_bHasLocalBounds = false;
    m_bHasBindGeometryBounds = false;
    m_vBindGeometryBoundsMin = float3_t(maximum, maximum, maximum);
    m_vBindGeometryBoundsMax = float3_t(-maximum, -maximum, -maximum);
}

void CModel::Include_LocalPosition(fvector_t vPosition)
{
	float3_t position{};
	XMStoreFloat3(&position, vPosition);
	if (!std::isfinite(position.x) || !std::isfinite(position.y) || !std::isfinite(position.z))
		return;

	m_vLocalBoundsMin.x = (min)(m_vLocalBoundsMin.x, position.x);
	m_vLocalBoundsMin.y = (min)(m_vLocalBoundsMin.y, position.y);
	m_vLocalBoundsMin.z = (min)(m_vLocalBoundsMin.z, position.z);
	m_vLocalBoundsMax.x = (max)(m_vLocalBoundsMax.x, position.x);
	m_vLocalBoundsMax.y = (max)(m_vLocalBoundsMax.y, position.y);
	m_vLocalBoundsMax.z = (max)(m_vLocalBoundsMax.z, position.z);
	m_bHasLocalBounds = true;
}

bool_t CModel::Try_GetBindGeometryBounds(float3_t& minimum, float3_t& maximum) const
{
    if (MODEL::NONANIM == m_eType && m_bHasLocalBounds)
    { minimum = m_vLocalBoundsMin; maximum = m_vLocalBoundsMax; return true; }
    if (!m_bHasBindGeometryBounds) return false;
    minimum = m_vBindGeometryBoundsMin; maximum = m_vBindGeometryBoundsMax; return true;
}

namespace
{
    bool MakeModelPickRay(const float4x4_t& world, const float3_t& rayOrigin,
        const float3_t& rayDirection, vector_t& origin, vector_t& direction,
        float& localLength, float& windingSign)
    {
        const auto finite = [](vector_t value) { return !XMVector3IsNaN(value) && !XMVector3IsInfinite(value); };
        const vector_t ray = XMLoadFloat3(&rayDirection);
        const float rayLength = XMVectorGetX(XMVector3Length(ray));
        if (!finite(ray) || !finite(XMLoadFloat3(&rayOrigin)) ||
            !std::isfinite(rayLength) || rayLength <= 1.e-8f) return false;
        for (const auto& row : world.m) for (const float value : row)
            if (!std::isfinite(value)) return false;
        // A world transform must be affine. Reject a projective matrix rather
        // than treating a perspective ray as a line after the inverse.
        if (world._14 != 0.f || world._24 != 0.f || world._34 != 0.f || world._44 != 1.f) return false;
        vector_t determinant;
        const matrix_t inverse = XMMatrixInverse(&determinant, XMLoadFloat4x4(&world));
        const float det = XMVectorGetX(determinant);
        if (!std::isfinite(det) || det == 0.f) return false;
        origin = XMVector3TransformCoord(XMLoadFloat3(&rayOrigin), inverse);
        const vector_t localRay = XMVector3TransformNormal(ray / rayLength, inverse);
        localLength = XMVectorGetX(XMVector3Length(localRay));
        if (!finite(origin) || !finite(localRay) || !std::isfinite(localLength) || localLength <= 0.f) return false;
        direction = localRay / localLength;
        windingSign = det < 0.f ? -1.f : 1.f;
        return true;
    }
}

bool_t CModel::Try_GetStaticOcclusionMesh(const uint32_t meshIndex,
    STATIC_OCCLUSION_MESH& output) const
{
    if (MODEL::NONANIM != m_eType || m_StaticCluster || m_StaticProxy ||
        meshIndex >= m_Meshes.size()) return false;
    const auto& mesh = m_Meshes[meshIndex];
    if (!mesh || mesh->m_hasUniqueVertexBuffer || !mesh->m_PickGeometry) return false;
    const auto& geometry = *mesh->m_PickGeometry;
    // Prepare_StaticPickGeometry validates every indexed position and index at
    // initialization. A filtered triangle count means the original IB is unsafe.
    if (geometry.skinned || geometry.vertices.empty() || geometry.nodes.empty() ||
        geometry.vertices.size() > (std::numeric_limits<uint32_t>::max)() ||
        geometry.indices.empty() || geometry.indices.size() % 3u != 0u ||
        geometry.triangles.size() != geometry.indices.size() / 3u) return false;
    static_assert(offsetof(CMesh::PICK_VERTEX, position) == 0u);
    output = {&geometry.vertices.front().position.x,
        static_cast<uint32_t>(geometry.vertices.size()),
        static_cast<uint32_t>(sizeof(CMesh::PICK_VERTEX)), geometry.indices};
    return true;
}

bool_t CModel::Try_PickStaticSurface(const uint32_t meshIndex, const float4x4_t& world,
    const float3_t& rayOrigin, const float3_t& rayDirection, const f32_t maxDistance,
    const PICK_CULL_MODE cullMode, f32_t& distance) const
{
    if (MODEL::NONANIM != m_eType || meshIndex >= m_Meshes.size() ||
        !m_Meshes[meshIndex] || !m_bHasLocalBounds || !std::isfinite(maxDistance) || maxDistance < 0.f ||
        (cullMode != PICK_CULL_MODE::NONE && cullMode != PICK_CULL_MODE::BACK &&
            cullMode != PICK_CULL_MODE::FRONT)) return false;
    vector_t origin, direction;
    float localLength = 0.f, windingSign = 1.f;
    if (!MakeModelPickRay(world, rayOrigin, rayDirection, origin, direction, localLength, windingSign)) return false;
    const float localLimit = static_cast<float>((std::min)(
        static_cast<double>(maxDistance) * localLength,
        static_cast<double>((std::numeric_limits<float>::max)())));
    BoundingBox bounds;
    BoundingBox::CreateFromPoints(bounds, XMLoadFloat3(&m_vLocalBoundsMin), XMLoadFloat3(&m_vLocalBoundsMax));
    float entry = 0.f;
    if (!bounds.Intersects(origin, direction, entry) || entry > localLimit) return false;
    const float cullSign = cullMode == PICK_CULL_MODE::NONE ? 0.f :
        windingSign * (cullMode == PICK_CULL_MODE::BACK ? 1.f : -1.f);
    float localDistance = 0.f;
    if (!m_Meshes[meshIndex]->Try_PickStaticLocal(origin, direction, localLimit, cullSign, localDistance)) return false;
    const float worldDistance = localDistance / localLength;
    if (!std::isfinite(worldDistance) || worldDistance > maxDistance) return false;
    distance = worldDistance;
    return true;
}

bool_t CModel::Try_PickCurrentPose(const float4x4_t& world, const float3_t& rayOrigin,
    const float3_t& rayDirection, f32_t& distance) const
{
    uint32_t meshIndex = 0u;
    return Try_PickCurrentPose(world, rayOrigin, rayDirection, distance, meshIndex);
}

bool_t CModel::Try_PickCurrentPose(const float4x4_t& world, const float3_t& rayOrigin,
    const float3_t& rayDirection, f32_t& distance, uint32_t& meshIndex) const
{
    const auto finite = [](vector_t value) { return !XMVector3IsNaN(value) && !XMVector3IsInfinite(value); };
    vector_t origin, direction;
    float localLength = 0.f, windingSign = 1.f;
    if (!MakeModelPickRay(world, rayOrigin, rayDirection, origin, direction, localLength, windingSign)) return false;
    float3_t minimum, maximum;
    if (!Try_GetCurrentPoseBounds(minimum, maximum)) return false;
    BoundingBox bounds;
    BoundingBox::CreateFromPoints(bounds, XMLoadFloat3(&minimum), XMLoadFloat3(&maximum));
    float boundDistance;
    if (!bounds.Intersects(origin, direction, boundDistance)) return false;
    float closest = (std::numeric_limits<float>::max)();
    bool hit = false;
    uint32_t closestMesh = 0u;
    for (uint32_t meshOrdinal = 0u; meshOrdinal < m_Meshes.size(); ++meshOrdinal)
    {
        const auto& mesh = m_Meshes[meshOrdinal];
        if (!mesh || mesh->m_hasUniqueVertexBuffer || !mesh->m_PickGeometry) continue;
        const auto& geometry = *mesh->m_PickGeometry;
        if (!geometry.skinned)
        {
            float candidate;
            if (mesh->Try_PickStaticLocal(origin, direction, closest, 0.f, candidate) && candidate < closest)
            { closest = candidate; closestMesh = meshOrdinal; hit = true; }
            continue;
        }
        vector<float4x4_t> palette;
        if (geometry.skinned)
        {
            if (mesh->m_iNumBones == 0u || mesh->m_iNumBones > 512u) continue;
            palette.resize(mesh->m_iNumBones);
            mesh->Build_SkinPalette(m_Bones, palette.data());
        }
        vector<float3_t> positions(geometry.vertices.size());
        bool valid = true;
        for (size_t index = 0; index < geometry.vertices.size(); ++index)
        {
            const auto& vertex = geometry.vertices[index];
            vector_t position = XMLoadFloat3(&vertex.position);
            if (geometry.skinned)
            {
                const uint32_t bones[] = {vertex.bones.x, vertex.bones.y, vertex.bones.z, vertex.bones.w};
                const float weights[] = {vertex.weights.x, vertex.weights.y, vertex.weights.z, vertex.weights.w};
                position = XMVectorZero();
                for (size_t lane = 0; lane < 4u; ++lane)
                {
                    if (bones[lane] >= palette.size()) { valid = false; break; }
                    position += XMVector3TransformCoord(XMLoadFloat3(&vertex.position),
                        XMLoadFloat4x4(&palette[bones[lane]])) * weights[lane];
                }
            }
            if (!valid || !finite(position)) { valid = false; break; }
            XMStoreFloat3(&positions[index], position);
        }
        if (!valid) continue;
        for (size_t index = 0; index + 2u < geometry.indices.size(); index += 3u)
        {
            const auto a = geometry.indices[index], b = geometry.indices[index + 1u], c = geometry.indices[index + 2u];
            if (a >= positions.size() || b >= positions.size() || c >= positions.size()) continue;
            float candidate;
            if (TriangleTests::Intersects(origin, direction, XMLoadFloat3(&positions[a]),
                XMLoadFloat3(&positions[b]), XMLoadFloat3(&positions[c]), candidate) && candidate < closest)
            { closest = candidate; closestMesh = meshOrdinal; hit = true; }
        }
    }
    if (!hit) return false;
    const float worldDistance = closest / localLength;
    if (!std::isfinite(worldDistance)) return false;
    distance = worldDistance;
    meshIndex = closestMesh;
    return true;
}

bool_t CModel::Try_GetCurrentPoseBounds(float3_t& minimum, float3_t& maximum) const
{
    if (MODEL::NONANIM == m_eType)
    {
        if (!m_bHasLocalBounds) return false;
        minimum = m_vLocalBoundsMin; maximum = m_vLocalBoundsMax; return true;
    }
    if (MODEL::ANIM != m_eType || m_Meshes.empty()) return false;
    const float limit = (std::numeric_limits<float>::max)();
    vector_t low = XMVectorReplicate(limit), high = XMVectorReplicate(-limit);
    bool hasPoint = false;
    for (const auto& mesh : m_Meshes)
    {
        if (!mesh || mesh->m_hasUniqueVertexBuffer ||
            mesh->m_BoneVertexBounds.size() != mesh->m_iNumBones ||
            mesh->m_BoneIndices.size() != mesh->m_iNumBones ||
            mesh->m_OffsetMatrices.size() != mesh->m_iNumBones) return false;
        for (uint32_t index = 0u; index < mesh->m_iNumBones; ++index)
        {
            const auto& bounds = mesh->m_BoneVertexBounds[index];
            if (!bounds.valid) continue;
            const uint32_t bone = mesh->m_BoneIndices[index];
            if (bone >= m_Bones.size() || !m_Bones[bone]) return false;
            // Exactly the same inverse-bind * combined order as the GPU palette.
            // Combined already owns the asset pretransform; do not apply it twice.
            const matrix_t skin = XMLoadFloat4x4(&mesh->m_OffsetMatrices[index]) *
                m_Bones[bone]->Get_CombinedTransformationMatrix();
            for (uint32_t corner = 0u; corner < 8u; ++corner)
            {
                const vector_t point = XMVector3TransformCoord(XMVectorSet(
                    (corner & 1u) ? bounds.maximum.x : bounds.minimum.x,
                    (corner & 2u) ? bounds.maximum.y : bounds.minimum.y,
                    (corner & 4u) ? bounds.maximum.z : bounds.minimum.z, 1.f), skin);
                if (XMVector3IsNaN(point) || XMVector3IsInfinite(point)) return false;
                low = XMVectorMin(low, point); high = XMVectorMax(high, point);
                hasPoint = true;
            }
        }
    }
    if (!hasPoint) return false;
    // Cover float rounding in normalized skin weights without changing scale.
    const vector_t margin = XMVectorReplicate(1.e-4f);
    XMStoreFloat3(&minimum, XMVectorSubtract(low, margin));
    XMStoreFloat3(&maximum, XMVectorAdd(high, margin));
    return true;
}

bool_t CModel::Try_GetAnimationEnvelopeRadius(f32_t& radius) const
{
    if (m_bAnimationEnvelopeExternalPose || m_bExplicitAnimationPose ||
        m_fAnimationSpeed < 0.f || m_fBlendElapsed < 0.f) return false;
    for (const auto& mesh : m_Meshes)
        if (!mesh || mesh->m_hasUniqueVertexBuffer) return false;
    if (!m_bAnimationEnvelopeAttempted)
    {
        m_bAnimationEnvelopeAttempted = true;
        m_fAnimationEnvelopeRadius = -1.f;
        Build_AnimationEnvelopeRadius(m_fAnimationEnvelopeRadius);
    }
    if (!std::isfinite(m_fAnimationEnvelopeRadius) || m_fAnimationEnvelopeRadius <= 0.f) return false;
    radius = m_fAnimationEnvelopeRadius;
    return true;
}

bool_t CModel::Build_AnimationEnvelopeRadius(f32_t& radius) const
{
    if (MODEL::ANIM != m_eType || m_Bones.empty() || m_Animations.empty() || m_Meshes.empty() ||
        m_BoneRestLocalTransforms.size() != m_Bones.size()) return false;
    const auto matrixEnvelope = [](const float4x4_t& m, double& stretch, std::array<double, 3>& translation) {
        const float* values = &m._11;
        for (size_t i = 0; i < 16; ++i) if (!std::isfinite(values[i])) return false;
        if (m._14 != 0.f || m._24 != 0.f || m._34 != 0.f || m._44 != 1.f) return false;
        // Gershgorin bound of A*A^T bounds the largest singular value, including shear.
        double squaredStretch = 0.;
        for (size_t row = 0; row < 3; ++row)
        {
            double sum = 0.;
            for (size_t other = 0; other < 3; ++other)
            {
                double dot = 0.;
                for (size_t axis = 0; axis < 3; ++axis)
                    dot += double(values[row * 4 + axis]) * values[other * 4 + axis];
                sum += std::abs(dot);
            }
            squaredStretch = (std::max)(squaredStretch, sum);
        }
        stretch = std::sqrt(squaredStretch) * 1.0001;
        translation = {std::abs(double(m._41)), std::abs(double(m._42)), std::abs(double(m._43))};
        return std::isfinite(stretch);
    };
    const auto length = [](const std::array<double, 3>& v) {return std::sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);};
    vector<std::array<double, 3>> localTranslation(m_Bones.size());
    vector<double> localScale(m_Bones.size()), combinedScale(m_Bones.size()), combinedRadius(m_Bones.size());
    for (size_t i = 0; i < m_Bones.size(); ++i)
        if (!m_Bones[i] || !matrixEnvelope(m_BoneRestLocalTransforms[i], localScale[i], localTranslation[i])) return false;
    for (const auto& animation : m_Animations)
        if (!animation || !animation->Accumulate_TransformEnvelope(localTranslation, localScale)) return false;
    if (m_iRootMotionBoneIndex >= 0)
    {
        if (size_t(m_iRootMotionBoneIndex) >= localTranslation.size() ||
            m_iRootMotionVerticalAxis < -1 || m_iRootMotionVerticalAxis > 2 ||
            !std::isfinite(m_fRootMotionVerticalScale) || m_fRootMotionVerticalScale < 0.f) return false;
        const double rest[3] = {m_vRootMotionRestTranslation.x, m_vRootMotionRestTranslation.y, m_vRootMotionRestTranslation.z};
        auto& bound = localTranslation[size_t(m_iRootMotionBoneIndex)];
        for (size_t axis = 0; axis < 3; ++axis)
        {
            if (!std::isfinite(rest[axis])) return false;
            bound[axis] = int32_t(axis) == m_iRootMotionVerticalAxis ?
                std::abs(rest[axis] * (1. - m_fRootMotionVerticalScale)) + bound[axis] * m_fRootMotionVerticalScale :
                std::abs(rest[axis]);
        }
    }
    double preScale = 0.; std::array<double, 3> preTranslation{};
    if (!matrixEnvelope(m_PreTransformMatrix, preScale, preTranslation)) return false;
    for (size_t i = 0; i < m_Bones.size(); ++i)
    {
        const int32_t parent = m_Bones[i]->Get_ParentBoneIndex();
        if (parent < -1 || (parent >= 0 && size_t(parent) >= i)) return false;
        const double parentScale = parent < 0 ? preScale : combinedScale[size_t(parent)];
        const double parentRadius = parent < 0 ? length(preTranslation) : combinedRadius[size_t(parent)];
        combinedScale[i] = localScale[i] * parentScale;
        combinedRadius[i] = length(localTranslation[i]) * parentScale + parentRadius;
        if (!std::isfinite(combinedScale[i]) || !std::isfinite(combinedRadius[i])) return false;
    }
    double result = 0.; bool hasVertex = false;
    for (const auto& mesh : m_Meshes)
    {
        if (!mesh || mesh->m_hasUniqueVertexBuffer || mesh->m_BoneVertexBounds.size() != mesh->m_iNumBones ||
            mesh->m_BoneIndices.size() != mesh->m_iNumBones || mesh->m_OffsetMatrices.size() != mesh->m_iNumBones) return false;
        for (size_t index = 0; index < mesh->m_iNumBones; ++index)
        {
            const auto& bounds = mesh->m_BoneVertexBounds[index];
            if (!bounds.valid) continue;
            const uint32_t bone = mesh->m_BoneIndices[index];
            if (bone >= m_Bones.size()) return false;
            const auto& offset = mesh->m_OffsetMatrices[index];
            for (const auto& row : offset.m)
                for (const float value : row) if (!std::isfinite(value)) return false;
            // Inverse binds may retain a positive homogeneous scale after
            // inversion. Divide by that exact w instead of rejecting or
            // rounding it. Positive skin weights remain a convex combination
            // after their homogeneous weights are normalized by the GPU.
            if (offset._14 != 0.f || offset._24 != 0.f || offset._34 != 0.f || offset._44 <= 0.f) return false;
            const double inverseW = 1. / double(offset._44);
            for (uint32_t corner = 0; corner < 8; ++corner)
            {
                const double x = (corner & 1u) ? bounds.maximum.x : bounds.minimum.x;
                const double y = (corner & 2u) ? bounds.maximum.y : bounds.minimum.y;
                const double z = (corner & 4u) ? bounds.maximum.z : bounds.minimum.z;
                const std::array<double, 3> point = {x*offset._11+y*offset._21+z*offset._31+offset._41,
                    x*offset._12+y*offset._22+z*offset._32+offset._42,
                    x*offset._13+y*offset._23+z*offset._33+offset._43};
                const double bound = length(point) * inverseW * combinedScale[bone] + combinedRadius[bone];
                if (!std::isfinite(bound)) return false;
                result = (std::max)(result, bound); hasVertex = true;
            }
        }
    }
    // Skin weights are normalized nonnegative values admitted by CMesh's bounds.
    // Their convex combination stays inside this sphere; cover float summation.
    result = result * 1.0001 + .0001;
    if (!hasVertex || result <= 0. || result >= (std::numeric_limits<float>::max)()) return false;
    radius = std::nextafter(static_cast<float>(result), (std::numeric_limits<float>::infinity)());
    return true;
}

void CModel::Include_BindGeometryPosition(fvector_t value)
{
    float3_t position; XMStoreFloat3(&position, value);
    if (!std::isfinite(position.x) || !std::isfinite(position.y) || !std::isfinite(position.z)) return;
    m_vBindGeometryBoundsMin.x = (std::min)(m_vBindGeometryBoundsMin.x, position.x);
    m_vBindGeometryBoundsMin.y = (std::min)(m_vBindGeometryBoundsMin.y, position.y);
    m_vBindGeometryBoundsMin.z = (std::min)(m_vBindGeometryBoundsMin.z, position.z);
    m_vBindGeometryBoundsMax.x = (std::max)(m_vBindGeometryBoundsMax.x, position.x);
    m_vBindGeometryBoundsMax.y = (std::max)(m_vBindGeometryBoundsMax.y, position.y);
    m_vBindGeometryBoundsMax.z = (std::max)(m_vBindGeometryBoundsMax.z, position.z);
    m_bHasBindGeometryBounds = true;
}

HRESULT CModel::Ready_Materials(const MODEL_ASSET_DATA& asset)
{
    Engine::CProfilerScope loadScope(CGameInstance::Get().Get_Profiler(), "Model.Load.Materials");
    // Each slot owns its material and device-only texture preparation. Publish
    // the complete vector only after every worker has finished successfully.
    vector<shared_ptr<CMaterial>> staged(asset.materials.size());
    static std::atomic_uint backgroundWorkers{ 0u };
    struct MATERIAL_PREPARATION final
    {
        const vector<MODEL_MATERIAL_DATA>& inputs;
        vector<shared_ptr<CMaterial>>& outputs;
        ComPtr<ID3D11Device> device;
        ComPtr<ID3D11DeviceContext> context;
        std::atomic_uint& backgroundWorkers;
        std::atomic_size_t next{ 0u };
        std::atomic<HRESULT> result{ S_OK };

        void Run() noexcept
        {
            try
            {
                while (SUCCEEDED(result.load(std::memory_order_relaxed)))
                {
                    const size_t index = next.fetch_add(1u, std::memory_order_relaxed);
                    if (index >= inputs.size()) return;
                    auto material = CMaterial::Create(device, context, inputs[index]);
                    if (!material) { result.store(E_FAIL, std::memory_order_relaxed); return; }
                    outputs[index] = std::move(material);
                }
            }
            catch (const std::bad_alloc&) { result.store(E_OUTOFMEMORY, std::memory_order_relaxed); }
            catch (...) { result.store(E_FAIL, std::memory_order_relaxed); }
        }

        static void CALLBACK Work(PTP_CALLBACK_INSTANCE, void* parameter, PTP_WORK)
        {
            auto& batch = *static_cast<MATERIAL_PREPARATION*>(parameter);
            const HRESULT apartment = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
            if (SUCCEEDED(apartment) || apartment == RPC_E_CHANGED_MODE)
            {
                try
                {
                    Engine::CProfilerScope workerScope(CGameInstance::Get().Get_Profiler(), "Model.Load.MaterialWorker");
                    batch.Run();
                }
                catch (const std::bad_alloc&) { batch.result.store(E_OUTOFMEMORY, std::memory_order_relaxed); }
                catch (...) { batch.result.store(E_FAIL, std::memory_order_relaxed); }
            }
            // If COM preparation fails the owning loader drains the same queue.
            if (SUCCEEDED(apartment)) CoUninitialize();
            batch.backgroundWorkers.fetch_sub(1u, std::memory_order_relaxed);
        }
    } batch{ asset.materials, staged, m_pDevice, m_pContext, backgroundWorkers };

    // Small/static map materials stay serial. At most three extra workers
    // across all simultaneous model loads use Windows' existing thread pool.
    const unsigned processors = static_cast<unsigned>(GetActiveProcessorCount(ALL_PROCESSOR_GROUPS));
    const unsigned limit = processors > 2u ? (std::min)(3u, processors - 2u) : 0u;
    PTP_WORK work = asset.hasSkeleton && asset.materials.size() >= 3u && limit != 0u &&
        0u == (m_pDevice->GetCreationFlags() & D3D11_CREATE_DEVICE_SINGLETHREADED) ?
        CreateThreadpoolWork(&MATERIAL_PREPARATION::Work, &batch, nullptr) : nullptr;
    struct WORK_JOIN final
    {
        PTP_WORK work;
        ~WORK_JOIN()
        {
            if (!work) return;
            WaitForThreadpoolWorkCallbacks(work, FALSE);
            CloseThreadpoolWork(work);
        }
    } join{ work };
    if (work)
    {
        for (size_t index = 1u; index < asset.materials.size(); ++index)
        {
            unsigned active = backgroundWorkers.load(std::memory_order_relaxed);
            while (active < limit && !backgroundWorkers.compare_exchange_weak(
                active, active + 1u, std::memory_order_relaxed)) {}
            if (active >= limit) break;
            SubmitThreadpoolWork(work);
        }
    }
    batch.Run();
    if (work)
    {
        Engine::CProfilerScope waitScope(CGameInstance::Get().Get_Profiler(), "Model.Load.MaterialJoin");
        WaitForThreadpoolWorkCallbacks(work, FALSE);
        CloseThreadpoolWork(work);
        join.work = nullptr;
    }
    const HRESULT result = batch.result.load(std::memory_order_relaxed);
    if (FAILED(result)) return result;
    if (std::any_of(staged.begin(), staged.end(), [](const auto& material) { return !material; }))
        return E_FAIL;
    m_iNumMaterials = static_cast<uint32_t>(staged.size());
    m_Materials = std::move(staged);
    return S_OK;
}

HRESULT CModel::Ready_Bones(const MODEL_ASSET_DATA& asset)
{
    Engine::CProfilerScope loadScope(CGameInstance::Get().Get_Profiler(), "Model.Load.Bones");
    m_Bones.reserve(asset.skeleton.bones.size());
    m_BoneRestLocalTransforms.reserve(asset.skeleton.bones.size());
    for (const MODEL_BONE_DATA& bone : asset.skeleton.bones)
    {
        auto pBone = CBone::Create(bone);
        if (nullptr == pBone)
            return E_FAIL;
        m_Bones.push_back(pBone);
        m_BoneRestLocalTransforms.push_back(bone.restLocal);
    }
    return S_OK;
}

HRESULT CModel::Ready_Animations(const MODEL_ASSET_DATA& asset)
{
    Engine::CProfilerScope loadScope(CGameInstance::Get().Get_Profiler(), "Model.Load.Animations");
    m_iNumAnimations = static_cast<uint32_t>(asset.animations.size());
    m_Animations.reserve(m_iNumAnimations);
    for (const MODEL_ANIMATION_DATA& animation : asset.animations)
    {
        auto pAnimation = CAnimation::Create(animation, m_Bones);
        if (nullptr == pAnimation)
            return E_FAIL;
        m_Animations.push_back(pAnimation);
    }
    return S_OK;
}

HRESULT CModel::Attach_AnimationSet(const CModel& animationSet)
{
	if (MODEL::ANIM != m_eType || MODEL::ANIM != animationSet.m_eType ||
		m_Bones.empty() || animationSet.m_Bones.empty() ||
		0 == m_iSkeletonHash ||
		m_iSkeletonHash != animationSet.m_iSkeletonHash ||
		m_Bones.size() != animationSet.m_Bones.size())
	{
		return E_FAIL;
	}

	for (const auto& pIncoming : animationSet.m_Animations)
	{
		for (const auto& pExisting : m_Animations)
		{
			if (pExisting->Compare_Name(pIncoming->Get_Name()))
				return E_FAIL;
		}
	}

	m_Animations.reserve(
		m_Animations.size() + animationSet.m_Animations.size());
	for (const auto& pIncoming : animationSet.m_Animations)
		m_Animations.push_back(pIncoming->Clone());
	m_iNumAnimations = static_cast<uint32_t>(m_Animations.size());
    m_bAnimationEnvelopeAttempted = false;
	return S_OK;
}

bool_t CModel::Sample_AnimationLocalTransforms(const char_t* name,
    const f32_t seconds, vector<float4x4_t>& output) const
{
    if (!name || !std::isfinite(seconds) || seconds < 0.f || m_Bones.empty() ||
        m_BoneRestLocalTransforms.size() != m_Bones.size()) return false;
    const CAnimation* selected = nullptr;
    for (const auto& animation : m_Animations)
        if (animation && animation->Compare_Name(name))
        {
            if (selected) return false;
            selected = animation.get();
        }
    if (!selected || selected->Get_TickPerSecond() <= 0.f) return false;
    const auto ticks = seconds * selected->Get_TickPerSecond();
    if (!std::isfinite(ticks) || ticks > selected->Get_Duration() + .001f) return false;
    auto staged = m_BoneRestLocalTransforms;
    if (!selected->Sample_LocalBoneTransforms((std::min)(ticks, selected->Get_Duration()), staged)) return false;
    for (const auto& local : staged) if (!Is_FiniteMatrix(local)) return false;
    output = std::move(staged); return true;
}

bool_t CModel::Install_AuthoredAnimations(const vector<MODEL_ANIMATION_DATA>& input, string& status)
{
    try
    {
        if (MODEL::ANIM != m_eType || !m_iSkeletonHash || m_Bones.empty() || input.size() > 32u)
            throw std::runtime_error("Authored clips need an admitted animated skeleton");
        auto staged = m_Animations;
        auto names = m_AuthoredAnimationNames;
        std::set<string> incoming;
        size_t totalKeys = 0u;
        for (auto animation : input)
        {
            if (!animation.name.starts_with("authored.") || animation.name.size() >= 128u ||
                !std::all_of(animation.name.begin(), animation.name.end(), [](unsigned char c) {
                    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                        (c >= '0' && c <= '9') || c == '.' || c == '_' || c == '-'; }) ||
                !incoming.insert(animation.name).second || animation.skeletonHash != m_iSkeletonHash ||
                !std::isfinite(animation.durationTicks) || animation.durationTicks <= 0.f || animation.durationTicks > 1800.f ||
                animation.ticksPerSecond != CAnimation::COOKED_TICK_RATE || animation.channels.size() != m_Bones.size())
                throw std::runtime_error("Invalid authored clip identity, skeleton or duration");
            std::set<int32_t> bones;
            for (auto& channel : animation.channels)
            {
                if (channel.resolvedBoneIndex < 0 || channel.resolvedBoneIndex >= static_cast<int32_t>(m_Bones.size()) ||
                    !bones.insert(channel.resolvedBoneIndex).second || channel.positionKeys.empty() ||
                    channel.rotationKeys.empty() || channel.scaleKeys.empty()) throw std::runtime_error("Invalid authored bone channels");
                const auto validateTimes = [&](const auto& keys)
                {
                    float previous = -1.f;
                    if (keys.size() > 8192u) throw std::runtime_error("Authored key count exceeds limit");
                    totalKeys += keys.size();
                    if (totalKeys > 6000000u) throw std::runtime_error("Authored clip key budget exceeded");
                    for (const auto& key : keys)
                    {
                        if (!std::isfinite(key.timeTicks) || key.timeTicks < 0.f || key.timeTicks <= previous ||
                            key.timeTicks > animation.durationTicks + .001f) throw std::runtime_error("Invalid authored key time");
                        previous = key.timeTicks;
                    }
                };
                validateTimes(channel.positionKeys); validateTimes(channel.scaleKeys); validateTimes(channel.rotationKeys);
                for (const auto& key : channel.positionKeys)
                    if (!std::isfinite(key.value.x) || !std::isfinite(key.value.y) || !std::isfinite(key.value.z))
                        throw std::runtime_error("Nonfinite authored translation");
                for (const auto& key : channel.scaleKeys)
                    if (!std::isfinite(key.value.x) || !std::isfinite(key.value.y) || !std::isfinite(key.value.z) ||
                        std::abs(key.value.x) <= .000001f || std::abs(key.value.y) <= .000001f || std::abs(key.value.z) <= .000001f) throw std::runtime_error("Invalid authored scale");
                for (auto& key : channel.rotationKeys)
                {
                    auto q = XMLoadFloat4(&key.value);
                    const auto length = XMVectorGetX(XMVector4LengthSq(q));
                    if (!std::isfinite(length) || length <= .000001f) throw std::runtime_error("Invalid authored quaternion");
                    XMStoreFloat4(&key.value, XMQuaternionNormalize(q));
                }
            }
            const auto existing = std::find_if(staged.begin(), staged.end(), [&](const auto& clip) {
                return clip && clip->Compare_Name(animation.name.c_str()); });
            if (existing != staged.end() && !names.contains(animation.name))
                throw std::runtime_error("Authored animation cannot replace a native clip");
            auto compiled = CAnimation::Create(animation, m_Bones);
            if (!compiled) throw std::runtime_error("Authored animation channel creation failed");
            if (existing == staged.end()) staged.push_back(std::move(compiled));
            else *existing = std::move(compiled);
            names.insert(animation.name);
        }
        m_Animations.swap(staged); m_AuthoredAnimationNames.swap(names);
        m_bAnimationEnvelopeAttempted = false;
        m_iNumAnimations = static_cast<uint32_t>(m_Animations.size());
        status = "Authored animation channels installed"; return true;
    }
    catch (const std::exception& error) { status = error.what(); return false; }
}


unique_ptr<CModel> CModel::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, MODEL eType, const char_t* pModelFilePath, fmatrix_t PreTransformMatrix,
    const bool_t bRetainOrderedStaticGeometry)
{
    auto pInstance = unique_ptr<CModel>(new CModel(pDevice, pContext));
    pInstance->m_bRetainOrderedStaticGeometry = bRetainOrderedStaticGeometry;

    if (FAILED(pInstance->Initialize_Prototype(eType, pModelFilePath, PreTransformMatrix)))
    {
        OutputDebugStringA("[CModel] Prototype creation failed.\n");
        return nullptr;
    }

    return pInstance;
}

unique_ptr<CModel> CModel::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const MODEL eType,
	const MODEL_ASSET_LOAD_DESC& loadDesc,
	fmatrix_t PreTransformMatrix,
	const bool_t bRetainOrderedStaticGeometry)
{
	auto pInstance = unique_ptr<CModel>(
		new CModel(pDevice, pContext));
	pInstance->m_bRetainOrderedStaticGeometry = bRetainOrderedStaticGeometry;
	if (FAILED(pInstance->Initialize_Prototype(
		eType, loadDesc, PreTransformMatrix)))
	{
		OutputDebugStringA("[CModel] Binary prototype creation failed.\n");
		return nullptr;
	}
	return pInstance;
}


unique_ptr<CModel> CModel::Create_MaterialVariant(const CModel& prototype,
    const MODEL_ASSET_LOAD_DESC& loadDesc)
{
    if (!prototype.m_pMaterialSource) return nullptr;
    const auto& identity = prototype.m_pMaterialSource->identity;
    if (identity.assetRoot.lexically_normal() != loadDesc.assetRoot.lexically_normal() ||
        identity.meshPath.lexically_normal() != loadDesc.meshPath.lexically_normal() ||
        identity.materialPath != loadDesc.materialPath || identity.skeletonPath != loadDesc.skeletonPath ||
        identity.animationPaths != loadDesc.animationPaths || identity.fallbackDiffusePath != loadDesc.fallbackDiffusePath ||
        identity.defaultAnimationName != loadDesc.defaultAnimationName) return nullptr;
    auto instance = unique_ptr<CModel>(new CModel(prototype));
    MODEL_MATERIAL_SOURCE staged = *prototype.m_pMaterialSource;
    if (FAILED(instance->Apply_MaterialOverrides(staged, loadDesc))) return nullptr;
    MODEL_ASSET_DATA materialAsset;
    materialAsset.materials = std::move(staged.materials);
    instance->m_Materials.clear();
    if (FAILED(instance->Ready_Materials(materialAsset))) return nullptr;
    return instance;
}

shared_ptr<CPrototype> CModel::Clone(void* pArg)
{
    auto pInstance = shared_ptr<CPrototype>(new CModel(*this));

    if (FAILED(pInstance->Initialize(pArg)))
    {
        OutputDebugStringA("[CModel] Clone failed.\n");
        return nullptr;
    }

    return pInstance;
}

void CModel::Free()
{
    if (false == m_isCloned && nullptr != m_pImporter)
        m_pImporter->FreeScene();
}

```

### Client/Private/MapStaticBatchObject.cpp

위치: 기존 파일 전체 교체. 기존 미변경 코드를 포함한 적용 후보 전문이다.

```cpp
#include "MapStaticBatchObject.h"
#include "MapStaticChunkObject.h"
#pragma push_macro("new")
#undef new
#include "Engine_RenderTypes.h"
#pragma pop_macro("new")
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
        if (!std::isfinite(radius) || !(radius > 0.f) || radius > 3.f ||
            !std::isfinite(scale) || scale < .25f || scale > 4.f) return 0.f;
        return (radius <= .5f ? 35.f : radius <= 1.5f ? 45.f : 60.f) * scale;
    }

    bool BeyondStaticPropDistance(const FMapStaticInstance& instance, const float4_t& camera,
        float limit, float margin)
    {
        if (!(limit > 0.f) || !std::isfinite(margin) || margin < 0.f ||
            !std::isfinite(camera.x) || !std::isfinite(camera.y) || !std::isfinite(camera.z)) return false;
        const double x = double(instance.WorldBoundsCenter.x) - camera.x;
        const double y = double(instance.WorldBoundsCenter.y) - camera.y;
        const double z = double(instance.WorldBoundsCenter.z) - camera.z;
        const double distanceSquared = x * x + y * y + z * z;
        const double threshold = double(limit) + instance.WorldBoundsRadius + margin;
        return std::isfinite(distanceSquared) && std::isfinite(threshold) && distanceSquared > threshold * threshold;
    }

    bool StaticPropFitsPixelLimit(const FMapStaticInstance& instance, const MAP_CAMERA_CULL_SNAPSHOT& camera,
        const float2_t& viewport, float maximumPixels, float margin)
    {
        const auto& p = camera.projection;
        if (!(camera.lodViewScale > 0.f) || !std::isfinite(maximumPixels) || !(maximumPixels > 0.f) || maximumPixels > 128.f ||
            !(viewport.x > 0.f) || !(viewport.y > 0.f) || p._14 != 0.f || p._24 != 0.f || p._34 != 1.f || p._44 != 0.f ||
            p._12 != 0.f || p._21 != 0.f || p._13 != 0.f || p._23 != 0.f || p._41 != 0.f || p._42 != 0.f ||
            !(p._11 > 0.f) || !(p._22 > 0.f) || !(p._33 > 1.f) || !(p._43 < 0.f)) return false;
        float3_t center{};
        XMStoreFloat3(&center, XMVector3TransformCoord(XMLoadFloat3(&instance.WorldBoundsCenter), XMLoadFloat4x4(&camera.lodView)));
        if (!std::isfinite(center.x) || !std::isfinite(center.y) || !std::isfinite(center.z)) return false;
        const double radius = (double(instance.WorldBoundsRadius) + margin) * camera.lodViewScale;
        const double depth = center.z;
        if (!(radius > 0.) || !(depth - radius > -double(p._43) / p._33)) return false;
        // Project all eight corners of the enclosing view-space cube. A sphere
        // cannot exceed this rectangle; off-axis depth expansion is retained.
        double maximumDiameter = 0.;
        for (size_t axis = 0u; axis < 2u; ++axis)
        {
            double minimum = DBL_MAX, maximum = -DBL_MAX;
            for (int sign = -1; sign <= 1; sign += 2)
                for (int side = -1; side <= 1; side += 2)
                {
                    const double projected = (double((&center.x)[axis]) + sign * radius) / (depth + side * radius);
                    minimum = (std::min)(minimum, projected); maximum = (std::max)(maximum, projected);
                }
            const double pixels = (axis == 0u ? double(p._11) * viewport.x : double(p._22) * viewport.y) * .5;
            maximumDiameter = (std::max)(maximumDiameter, (maximum - minimum) * pixels);
        }
        return std::isfinite(maximumDiameter) && maximumDiameter > 0. &&
            maximumDiameter * (1. + 32. * FLT_EPSILON) <= maximumPixels;
    }

    bool StaticPropDistanceHidden(const FMapStaticInstance& instance, const float4_t& position,
        const MAP_CAMERA_CULL_SNAPSHOT& camera, const float2_t& viewport, float limit,
        float maximumPixels, uint64_t settingsRevision, float margin)
    {
        const bool wasHidden = instance.DistanceHidden && instance.DistanceSettingsRevision == settingsRevision;
        return BeyondStaticPropDistance(instance, position, wasHidden ? limit : limit * 1.1f, margin) &&
            StaticPropFitsPixelLimit(instance, camera, viewport, wasHidden ? maximumPixels : maximumPixels * .9f, margin);
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
    auto& game = CGameInstance::Get();
    if (!m_FrameState || !m_pModelCom || !Is_FinalCameraPrepared() || m_VisibleInstances.empty() ||
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

void CMapStaticBatchObject::Submit_FinalCamera()
{
    auto& game = CGameInstance::Get();
    if (game.Is_SceneEnvironmentReplaced() || m_iAuthoredVisibleInstanceCount == 0u)
        return;

    if (!Are_AllChunkMeshesClaimed())
    {
        const auto* camera = CMapAssetRenderUtils::Capture_CameraCullSnapshotView();
        const HRESULT visibility = Prepare_FinalCameraVisibility(camera);
        // A failed preparation keeps the original draw callback as the error/retry path.
        if (FAILED(visibility) || !m_VisibleInstances.empty())
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
        if (!batch.Is_FinalCameraPrepared() || !batch.m_pModelCom || !batch.m_pShaderCom ||
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
        return batch.Is_FinalCameraPrepared() && batch.m_pModelCom && batch.m_pShaderCom &&
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
        static_cast<uint32_t>(instanceCount), camera);
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
		return S_OK;
	}
	if (profiler) profiler->Add_Counter(Engine::EProfilerCounter::MapBatchVisibleRenders);

    return Render_VisibleInstances(m_pInstanceBuffer.Get(),
        static_cast<uint32_t>(m_VisibleInstances.size()), cameraSnapshot);
}

bool_t CMapStaticBatchObject::Uses_MeshScreenLod(uint32_t meshIndex, bool_t hasScreenLod) const
{
    const auto* surface = m_pModelCom->Get_MaterialSurface(meshIndex);
    return hasScreenLod && CGameInstance::Get().Get_MaterialRenderSettings().bUseSourceMaterials && surface &&
        surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED &&
        !surface->sourceFoliageWind && (surface->sourceBgFlags & 64u) == 0u &&
        (surface->renderMode == Engine::MODEL_SURFACE_RENDER_MODE::INHERIT ||
         surface->renderMode == Engine::MODEL_SURFACE_RENDER_MODE::DEFERRED) &&
        !m_pModelCom->Has_MaterialTexture(meshIndex, aiTextureType_OPACITY);
}

HRESULT CMapStaticBatchObject::Render_VisibleInstances(ID3D11Buffer* buffer, uint32_t instanceCount,
    const MAP_CAMERA_CULL_SNAPSHOT* cameraSnapshot)
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

    Invalidate_ChunkClaims();
	FMapStaticInstance& current = m_Instances[iter->second];
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
        Invalidate_ChunkClaims();
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
        Invalidate_ChunkClaims();
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
        Invalidate_ChunkClaims();
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
    m_bDistanceEligible = m_FrameState && m_AssetGroupId != "landscape" && m_RenderProfile.opacity == 1.f;
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
			instance.CameraPreviewSuppressed)
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
            const float limit = StaticPropDistanceLimit(instance.WorldBoundsRadius, visibilitySettings.DistanceScale);
            if (limit > 0.f)
            {
                ++distanceTested;
                float margin = (std::max)(0.f, m_FrustumCulling.baseMargin);
                if (instance.WorldBoundsRadius >= m_FrustumCulling.largeObjectRadiusThreshold)
                    margin += (std::max)(m_FrustumCulling.largeObjectAbsoluteMargin,
                        instance.WorldBoundsRadius * m_FrustumCulling.largeObjectRelativeMargin);
                distanceHidden = StaticPropDistanceHidden(instance, *distanceCamera, *cameraSnapshot, distanceViewport,
                    limit, visibilitySettings.DistanceMaxPixels, visibilitySettings.Revision, margin);
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

```
