#pragma once

#include "Client_Defines.h"
#include "Level.h"

NS_BEGIN(Client)

class CLevel_Logo final : public CLevel
{
private:
	CLevel_Logo(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CLevel_Logo();

public:
	virtual HRESULT Initialize() override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Ready_Layer_BackGround(const wstring_t& strLayerTag);

public:
	static unique_ptr<CLevel_Logo> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);

};

NS_END