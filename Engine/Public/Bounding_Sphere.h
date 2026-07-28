#pragma once

#include "Bounding.h"

NS_BEGIN(Engine)

class CBounding_Sphere final : public CBounding
{
public:
	typedef struct tagBoundingSphereDesc : public CBounding::BOUNDING_DESC
	{
		f32_t		fRadius;
	}BOUNDING_SPHERE_DESC;
private:
	CBounding_Sphere(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	~CBounding_Sphere();
public:
	shared_ptr<BoundingSphere> Get_Desc() const {
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
	shared_ptr<BoundingSphere>			m_pOriginalDesc = { nullptr };
	shared_ptr<BoundingSphere>			m_pDesc = { nullptr };


public:
	static shared_ptr<CBounding_Sphere> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, const CBounding::BOUNDING_DESC* pArg);

};

NS_END