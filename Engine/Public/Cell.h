#pragma once

#include "Engine_Defines.h"

NS_BEGIN(Engine)

class CCell final 
{
private:
	CCell(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	~CCell();

public:
	vector_t Get_Center() {
		vector_t		vCenter = {};
		for (size_t i = 0; i < 3; i++)
			vCenter += XMLoadFloat3(&m_vPoints[i]);
		
		return XMVectorSetW(vCenter / 3.f, 1.f);
	}

	vector_t Get_Point(POINT ePoint) {
		return XMLoadFloat3(&m_vPoints[ETOUI(ePoint)]);
	}

	void Set_Neighbor(LINE eLine, shared_ptr<CCell> pNeighbor) {
		m_iNeighborIndices[ETOUI(eLine)] = pNeighbor->m_iIndex;
	}
	void Set_Neighbor(int32_t* pNeighborIndices) {
		memcpy(m_iNeighborIndices, pNeighborIndices, sizeof(int32_t) * 3);
	}



public:
	HRESULT Initialize(const float3_t* pPoints, int32_t iIndex);
	bool_t isIn(fvector_t vResultPos, int32_t* pNeighborIndex);

	bool_t Compare_Points(fvector_t vSourPoint, fvector_t vDestPoint);
	f32_t Compute_Height(fvector_t vPosition);

#ifdef _DEBUG
public:
	HRESULT Render();
#endif

private:
	ComPtr<ID3D11Device>				m_pDevice = {};
	ComPtr<ID3D11DeviceContext>			m_pContext = {};

	float3_t				m_vPoints[ETOUI(POINT::END)] = {};
	int32_t					m_iIndex = { -1 };	

	float3_t				m_vNormals[ETOUI(LINE::END)] = {};

	int32_t					m_iNeighborIndices[ETOUI(LINE::END)] = { -1, -1, -1 };

	float4_t				m_vPlane = {};

#ifdef _DEBUG
private:
	shared_ptr<class CVIBuffer_Cell>			m_pVIBuffer = { nullptr };

#endif

public:
	static shared_ptr<CCell> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, const float3_t* pPoints, int32_t iIndex);
};

NS_END