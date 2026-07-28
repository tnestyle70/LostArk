#pragma once

#include "Component.h"

NS_BEGIN(Engine)

class ENGINE_DLL CNavigation final : public CComponent
{
public:
	typedef struct tagNavigationDesc
	{
		const _float4x4* pParentMatrix = { nullptr };
		_int	iCurrentCellIndex = { -1 };
	}NAVIGATION_DESC;

private:
	CNavigation(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CNavigation(const CNavigation& Prototype);
	virtual ~CNavigation() = default;

public:
	_vector Get_CellPos();

public:
	virtual HRESULT Initialize_Prototype(const _tchar* pNavigationData, const _tchar* pNavigationNeighborData);
	virtual HRESULT Initialize(void* pArg) override;

public:
	_vector SetUp_OnNavigation(_fvector vWorldPos);
	_bool isMove(_fvector vResultPos);
	const list<_vector>* Make_Route(_int iGoalIndex);


#ifdef _DEBUG
public:
	HRESULT Render();

#endif

private:
	vector<class CCell*>			m_Cells;
	_uint							m_iNumCells = {};
	_int							m_iCurrentCellIndex = { -1 };
	

private:
	static const _float4x4*			m_pParentMatrix;

private:
	_int					m_iOldGoalIndex = { -1 };
	list<_int>				m_OpenList;
	list<_int>				m_CloseList;

	list<_vector>				m_Route;


#ifdef _DEBUG
private:
	class CShader* m_pShader = { nullptr };

#endif
private:
	HRESULT SetUp_Neighbors(const _tchar* pNavigationNeighborData = nullptr);
	_bool isPush(_int iCellIndex);
public:
	static CNavigation* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* pNavigationData, const _tchar* pNavigationNeighborData = nullptr);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END