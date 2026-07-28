#pragma once

/* 충돌체 정보를 의미한다. */
/* 구, 회전이없는박스, 회전이 있는 박스 */
#include "Component.h"
#include "Bounding_AABB.h"
#include "Bounding_OBB.h"
#include "Bounding_Sphere.h"

NS_BEGIN(Engine)

class ENGINE_DLL CCollider final : public CComponent
{
private:
	CCollider(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	~CCollider();

public:
	virtual HRESULT Initialize_Prototype(COLLIDER eType);
	virtual HRESULT Initialize(void* pArg) override;
public:
	void Update(fmatrix_t WorldMatrix);

	bool_t Intersect(shared_ptr<CCollider> pTargetCollider);



#ifdef _DEBUG
public:
	virtual HRESULT Render() override;

#endif

private:
	COLLIDER				m_eType = { COLLIDER::END };
	shared_ptr<CBounding>	m_pBounding = { nullptr };	
	bool_t					m_isColl = { false };

#ifdef _DEBUG
private:
	shared_ptr<PrimitiveBatch<VertexPositionColor>>			m_pBatch = { nullptr };
	shared_ptr<BasicEffect>									m_pEffect = { nullptr };
	ComPtr<ID3D11InputLayout>								m_pInputLayout = { nullptr };
#endif

public:
	static unique_ptr<CCollider> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, COLLIDER eType);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;

};

NS_END