#pragma once

#include "Client_Defines.h"
#include "Level.h"

NS_BEGIN(Client)

class CCamera_Free;
class CCharacter;
class CValtan;

class CLevel_AssetTest final : public CLevel
{
private:
	CLevel_AssetTest(ComPtr<ID3D11Device> pDevice, 
		ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CLevel_AssetTest();

public:
	virtual HRESULT Initialize() override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Ready_Lights();
	HRESULT Ready_Layer_Camera(const wstring_t& strLayerTag);
	HRESULT Ready_Character();
	HRESULT Ready_Valtan();

	bool_t Bind_CameraToCharacter(
		const shared_ptr<CCharacter>& pCharacter);

	void Update_ClickMove();
#ifdef _DEBUG
	void Update_NavigationDebug();
#endif

private:
	shared_ptr<CCharacter> m_pCharacter = { nullptr };
	shared_ptr<CValtan> m_pValtan = { nullptr };
	// 실제 수명은 Layer_Camera가 소유하고 Level은 대상 전환용 핸들만 유지한다.
	weak_ptr<CCamera_Free> m_pCamera;
	bool_t m_bRightMouseDown = { false };
#ifdef _DEBUG
	bool_t m_bNavigationDebugVisible = { false };
#endif

public:
	static unique_ptr<CLevel_AssetTest> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
};

NS_END
