#pragma once

#include "Client_Defines.h"
#include "Level.h"

NS_BEGIN(Client)

class CValtan;

class CLevel_AssetTest final : public CLevel
{
private:
	CLevel_AssetTest(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CLevel_AssetTest();

public:
	virtual HRESULT Initialize() override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Ready_Lights();
	HRESULT Ready_Layer_Camera(const wstring_t& strLayerTag);
	HRESULT Ready_Valtan();
	void Update_ClickMove();
#ifdef _DEBUG
	void Update_NavigationDebug();
#endif

private:
	shared_ptr<CValtan> m_pValtan = { nullptr };
	bool_t m_bLeftMouseDown = { false };
#ifdef _DEBUG
	bool_t m_bF5Down = { false };
	bool_t m_bNavigationDebugVisible = { false };
#endif

public:
	static unique_ptr<CLevel_AssetTest> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
};

NS_END
