#pragma once

#include "Bounding.h"

NS_BEGIN(Engine)

class CBounding_OBB final : public CBounding
{
public:
	typedef struct tagBoundingOBBDesc : public CBounding::BOUNDING_DESC
	{
		float3_t		vSize;
		float3_t		vDegree;
	}BOUNDING_OBB_DESC;

	typedef struct tagOBBDesc
	{
		float3_t		vCenter;
		float3_t		vCenterDir[3];
		float3_t		vAlignDir[3];
	}OBB_DESC;
private:
	CBounding_OBB(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	~CBounding_OBB();

public:
	shared_ptr<BoundingOrientedBox> Get_Desc() const {
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
	shared_ptr<BoundingOrientedBox>			m_pOriginalDesc = { nullptr };
	shared_ptr<BoundingOrientedBox>			m_pDesc = { nullptr };

private:
	OBB_DESC Compute_OBBDesc();
	bool_t Intersect(shared_ptr<CBounding_OBB> pTarget);
public:
	static shared_ptr<CBounding_OBB> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, const CBounding::BOUNDING_DESC* pArg);

};

NS_END