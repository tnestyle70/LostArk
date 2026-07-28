#pragma once

#include "Bounding.h"

NS_BEGIN(Engine)

class CBounding_AABB final : public CBounding
{
public:
	typedef struct tagBoundingAABBDesc : public CBounding::BOUNDING_DESC
	{
		float3_t		vSize;
	}BOUNDING_AABB_DESC;
private:
	CBounding_AABB(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	~CBounding_AABB();

public:
	shared_ptr<BoundingBox> Get_Desc() const {
		return m_pDesc;
	}

public:
	virtual HRESULT Initialize(const CBounding::BOUNDING_DESC* pArg) override;
	virtual void Update(fmatrix_t WorldMatrix)  override;
	virtual bool_t Intersect(COLLIDER eTargetType, shared_ptr<CBounding> pTargetBounding) override;
#ifdef _DEBUG
public:
	virtual HRESULT Render(shared_ptr<PrimitiveBatch<VertexPositionColor>> pBatch);
#endif

private:
	shared_ptr<BoundingBox>			m_pOriginalDesc = { nullptr };
	shared_ptr<BoundingBox>			m_pDesc = { nullptr };

private:
	float3_t Compute_Min();
	float3_t Compute_Max();

	bool_t Intersect(shared_ptr<CBounding_AABB> pTarget);


public:
	static shared_ptr<CBounding_AABB> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, const CBounding::BOUNDING_DESC* pArg);

};

NS_END