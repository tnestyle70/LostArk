#pragma once

#include "Client_Defines.h"
#include "Level.h"
#include "MapPlacementRuntime.h"

NS_BEGIN(Client)

class CCamera_Free;

class CLevel_Baren final : public CLevel
{
private:
	CLevel_Baren(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CLevel_Baren();

public:
	virtual HRESULT Initialize() override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Ready_Layer_Camera(const wstring_t& strLayerTag);
	//HRESULT Ready_Character();
	//HRESULT Ready_Valtan();

private:
	CMapPlacementRuntime m_MapRuntime;
	shared_ptr<CCamera_Free> m_pCamera = { nullptr };

public:
	static unique_ptr<CLevel_Baren> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
};

NS_END
