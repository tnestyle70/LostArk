#pragma once

#include "Client_Defines.h"
#include "Level.h"

/* 특정 레벨에 진입하기전에 반드시 거쳐야하는 레벨. */

/* 레벨 본역의 역활 + 다음 레벨에 대한 자원을 준비해준다.*/


NS_BEGIN(Client)

class CLevel_Loading final : public CLevel
{
private:
	CLevel_Loading(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CLevel_Loading();

public:
	virtual HRESULT Initialize(LEVEL eNextLevelID);
	virtual void Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	LEVEL							m_eNextLevelID = { LEVEL::END };
	unique_ptr<class CLoader>		m_pLoader = { nullptr };


public:
	static unique_ptr<CLevel_Loading> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, LEVEL eNextLevelID);

};

NS_END