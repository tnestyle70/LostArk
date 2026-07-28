#pragma once

#include "Client_Defines.h"
#include "Level.h"

NS_BEGIN(Client)

class CLevel_GamePlay final : public CLevel
{
private:
	CLevel_GamePlay(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CLevel_GamePlay();

public:
	virtual HRESULT Initialize() override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;
private:
	HRESULT Ready_Lights();
	HRESULT Ready_Layer_Camera(const wstring_t& strLayerTag);
	HRESULT Ready_Layer_Player(const wstring_t& strLayerTag);
	HRESULT Ready_Layer_Monster(const wstring_t& strLayerTag);
	HRESULT Ready_Layer_BackGround(const wstring_t& strLayerTag);
	HRESULT Ready_Layer_Effect(const wstring_t& strLayerTag);

public:
	static unique_ptr<CLevel_GamePlay> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);

};

NS_END