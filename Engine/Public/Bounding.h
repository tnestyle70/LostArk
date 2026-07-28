#pragma once

#include "Engine_Defines.h"

NS_BEGIN(Engine)

class CBounding abstract 
{
public:
	typedef struct tagBoundingDesc
	{
		float3_t		vCenter;
	}BOUNDING_DESC;

protected:
	CBounding(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	~CBounding();

public:
	virtual HRESULT Initialize(const CBounding::BOUNDING_DESC* pArg);
	virtual void Update(fmatrix_t WorldMatrix) = 0;
	virtual bool_t Intersect(COLLIDER eTargetType, shared_ptr<CBounding> pTargetBounding) = 0;

#ifdef _DEBUG
public:
	virtual HRESULT Render(shared_ptr<PrimitiveBatch<VertexPositionColor>> pBatch) = 0;
#endif

protected:
	ComPtr<ID3D11Device>				m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext>			m_pContext = { nullptr };
	bool_t								m_isColl = { false };
};

NS_END