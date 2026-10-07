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
