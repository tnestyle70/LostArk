#pragma once

#include "Client_Defines.h"
#include "Level.h"
#include "MapPlacementRuntime.h"

NS_BEGIN(Client)

class CCamera_Free;
class CCharacter;

class CLevel_Development final : public CLevel
{
private:
	CLevel_Development(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CLevel_Development();

public:
	virtual HRESULT Initialize() override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Ready_Lights();
	HRESULT Ready_Camera(const wstring_t& strLayerTag);
	HRESULT Ready_Character();
	void Update_ClickMove();

private:
	CMapPlacementRuntime m_MapRuntime;
	shared_ptr<CCharacter> m_pCharacter = { nullptr };
	weak_ptr<CCamera_Free> m_pCamera;
	bool_t m_wasRightMouseDown = false;

public:
	static unique_ptr<CLevel_Development> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
};

NS_END
