#pragma once

#include "Component.h"

#include <array>
#include <span>

NS_BEGIN(Engine)

struct MODEL_ASSET_DATA;
struct MODEL_ASSET_LOAD_DESC;
struct MODEL_COLOR_TINT;
struct MODEL_SURFACE_PARAMETERS;

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
	bool_t Set_BoneLocalMatrix(uint32_t iBoneIndex, fmatrix_t Matrix);
	void Refresh_BoneCombinedMatrices();
	bool_t Enable_RootMotionSuppression(
		const char_t* pBoneName, int32_t iVerticalAxis);

	// Scales only the preserved root translation axis; geometry and clip time stay unchanged.
	bool_t Set_RootMotionVerticalScale(f32_t fScale);
	f32_t Get_RootMotionVerticalScale() const { return m_fRootMotionVerticalScale; }

	void Set_Animation(uint32_t iAnimIndex, bool_t isLoop = false,
		f32_t fBlendSeconds = 0.f) {
		if (iAnimIndex >= m_iNumAnimations)
			return;
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
	uint64_t Get_SkeletonHash() const {
		return m_iSkeletonHash;
	}
	HRESULT Attach_AnimationSet(const CModel& animationSet);

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
		uint32_t iInstanceStride, uint32_t iNumInstances);
	bool_t Play_Animation(f32_t fTimeDelta);
	HRESULT Bind_BoneMatrices(shared_ptr<class CShader> pShader, const char_t* pConstantName, uint32_t iMeshIndex);
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
	/* Null when the mesh or its material is out of range. */
	const float4_t* Get_MaterialDiffuseTint(uint32_t iMeshIndex) const;
	bool_t Has_MaterialTexture(uint32_t iMeshIndex, aiTextureType eType, uint32_t iTextureIndex = 0) const;
	/* Null when the mesh or its material is out of range; identity tint (its
	isEnabled false) when the material simply has no colour mask. */
	const MODEL_COLOR_TINT* Get_MaterialColorTint(uint32_t iMeshIndex) const;
	HRESULT Bind_SurfaceLighting(shared_ptr<class CShader> shader, uint32_t meshIndex);
	HRESULT Bind_SourceCharacter(shared_ptr<class CShader> shader, uint32_t meshIndex);
	const MODEL_SURFACE_PARAMETERS* Get_MaterialSurface(uint32_t iMeshIndex) const;
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
	Assimp::Importer					m_Importer = {};

private:
	MODEL								m_eType = { MODEL::END };
	uint32_t							m_iNumMeshes = {};
	vector<shared_ptr<class CMesh>>		m_Meshes;
	float4x4_t							m_PreTransformMatrix = {};

	uint32_t							m_iNumMaterials = {};
	vector<shared_ptr<class CMaterial>>	m_Materials;

	vector<shared_ptr<class CBone>>		m_Bones;
	vector<float4x4_t>					m_BoneRestLocalTransforms;
	uint64_t							m_iSkeletonHash = {};

	uint32_t								m_iCurrentAnimIndex = {};
	uint32_t								m_iNumAnimations = {};
	vector<shared_ptr<class CAnimation>>	m_Animations;
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
	HRESULT Ready_Meshes(const MODEL_ASSET_DATA& asset);
	HRESULT Ready_Materials(const MODEL_ASSET_DATA& asset);
	HRESULT Ready_Bones(const MODEL_ASSET_DATA& asset);
	HRESULT Ready_Animations(const MODEL_ASSET_DATA& asset);
	void Reset_LocalBounds();
	void Include_LocalPosition(fvector_t vPosition);

public:
	static unique_ptr<CModel> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, MODEL eType, const char_t* pModelFilePath, fmatrix_t PreTransformMatrix);
	static unique_ptr<CModel> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		MODEL eType,
		const MODEL_ASSET_LOAD_DESC& loadDesc,
		fmatrix_t PreTransformMatrix);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
	void Free();
};

NS_END
