#pragma once

#include "Component.h"

NS_BEGIN(Engine)

class ENGINE_DLL CNavigation final : public CComponent
{
public:
	typedef struct tagNavigationDesc
	{
		int32_t		iStartCellIndex = {-1};
		shared_ptr<class CTransform> pTransformCom;
	}NAVIGATION_DESC;

private:
	CNavigation(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CNavigation();

public:
	virtual HRESULT Initialize_Prototype(const tchar_t* pNavigationDataFiles, const tchar_t* pNeighborDataFile);
	virtual HRESULT Initialize(void* pArg) override;
	
public:
	HRESULT SetUp_Neighbors();
	HRESULT SetUp_Neighbors(const tchar_t* pNeighborDataFile);
	bool_t isMove(fvector_t vResultPos);
	void SetUp_OnNavigation(shared_ptr<class CTransform> pTargetTransform);

#ifdef _DEBUG
public:
	virtual HRESULT Render() override;
#endif

private:
	vector<shared_ptr<class CCell>>			m_Cells;
	int32_t									m_iCurrentCellIndex = { -1 };

private:
	shared_ptr<class CTransform>			m_pTargetTransformCom = {};


#ifdef _DEBUG
private:
	shared_ptr<class CShader>				m_pShader = { nullptr };
#endif


public:
	static unique_ptr<CNavigation> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, const tchar_t* pNavigationDataFiles, const tchar_t* pNeighborDataFile = nullptr);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END