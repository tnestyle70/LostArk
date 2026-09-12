#pragma once

#include "VIBuffer.h"

NS_BEGIN(Engine)

struct MODEL_MESH_DATA;
struct MODEL_SKELETON_DATA;

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
	virtual HRESULT Initialize_Prototype(MODEL eType, const aiMesh* pAIMesh, const vector<shared_ptr<class CBone>>& Bones, fmatrix_t PreTransformMatrix);
	HRESULT Initialize_Prototype(MODEL eType, const MODEL_MESH_DATA& mesh,
		const MODEL_SKELETON_DATA& skeleton, fmatrix_t PreTransformMatrix);
	virtual HRESULT Initialize(void* pArg) override;

public:
	HRESULT Render_Instanced(ID3D11Buffer* pInstanceBuffer,
		uint32_t iInstanceStride, uint32_t iNumInstances,
		uint32_t iInstanceByteOffset = 0u);

public:
	/* True once Make_VertexBuffer_Unique() has run on this instance -- i.e. it has read back
	and kept the unmorphed base position/normal. */
	bool_t Has_MorphBaseVertices() const {
		return nullptr != m_pMorphBasePositions;
	}
	/* Gives this instance its own vertex buffer, independent of the prototype's and of every
	sibling clone's, so Update_Vertices() only ever affects this one, and reads the unmorphed
	base position/normal back off the GPU while doing it. The new buffer starts as an exact
	copy, so nothing changes visually until Update_Vertices() is called. A no-op if this
	instance already has a unique buffer.

	This is the one call in this class that stalls: the readback goes through a staging
	buffer (the vertex buffer is D3D11_USAGE_DEFAULT with no CPU access, so it cannot be
	Map()ed) and waits on the GPU. It is meant to be called once per character that actually
	morphs, not per frame. Nothing is paid by a model that never calls it. */
	HRESULT Make_VertexBuffer_Unique();
	/* Overwrites a sparse set of vertices' position and normal in place -- one
	UpdateSubresource per index, each restricted to that vertex's 24-byte
	position+normal span, so every other field (UV, tangent, blend indices/weights) is left
	untouched in the GPU buffer. Requires Make_VertexBuffer_Unique() first. iIndices are
	local to this CMesh (0..Get_NumVertices()-1). */
	HRESULT Update_Vertices(const vector<uint32_t>& iIndices,
		const vector<float3_t>& Positions, const vector<float3_t>& Normals);
	/* Restores every vertex this instance has ever touched via Update_Vertices() back to the
	retained base snapshot (weight-0 morph state). No-op if never made unique. */
	HRESULT Reset_Vertices();
	/* The unmorphed rest position/normal for one vertex, so a caller can compose
	base + sum(weight * delta) itself before calling Update_Vertices() -- .facemorphs only
	carries deltas, not a base. False (and both out-params left untouched) if
	Has_MorphBaseVertices() is false or iIndex is out of range. */
	bool_t Get_MorphBaseVertex(uint32_t iIndex, float3_t& OutPosition, float3_t& OutNormal) const;

private:
	char_t					m_szName[MAX_PATH] = {};
	uint32_t				m_iMaterialIndex = {};
	uint32_t				m_iNumBones = {};
	vector<uint32_t>		m_BoneIndices;
	vector<float4x4_t>		m_OffsetMatrices;
	// Binary meshes use the same ordered inverse-bind palette within one model.
	bool_t m_bUsesSkeletonPalette = false;

	/* The unmorphed rest position/normal per vertex, in this mesh's own local index space,
	read back off the GPU by Make_VertexBuffer_Unique(). Null until then. Held by
	shared_ptr so a later Clone() of an already-unique mesh shares it rather than copying. */
	shared_ptr<vector<float3_t>>	m_pMorphBasePositions;
	shared_ptr<vector<float3_t>>	m_pMorphBaseNormals;
	/* True only for a clone that called Make_VertexBuffer_Unique(): this instance's m_pVB is
	its own buffer, not the one shared with the prototype/siblings. */
	bool_t					m_hasUniqueVertexBuffer = { false };
	/* Which local vertex indices this instance's Update_Vertices() has touched since the
	last Make_VertexBuffer_Unique()/Reset_Vertices(), so Reset_Vertices() only has to
	revert those instead of rewriting the whole buffer. */
	vector<uint32_t>		m_TouchedVertexIndices;

private:
	void Build_SkinPalette(const vector<shared_ptr<class CBone>>& Bones,
		float4x4_t* pOutMatrices) const;
	HRESULT Ready_VertexBuffer_NonAnim(const aiMesh* pAIMesh, fmatrix_t PreTransformMatrix);
	HRESULT Ready_VertexBuffer_Anim(const aiMesh* pAIMesh, const vector<shared_ptr<class CBone>>& Bones);

public:
	static shared_ptr<CMesh> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, MODEL eType, const aiMesh* pAIMesh, const vector<shared_ptr<class CBone>>& Bones, fmatrix_t PreTransformMatrix);
	static shared_ptr<CMesh> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext, MODEL eType,
		const MODEL_MESH_DATA& mesh, const MODEL_SKELETON_DATA& skeleton,
		fmatrix_t PreTransformMatrix);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
