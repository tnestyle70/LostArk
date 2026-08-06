#pragma once

#include "Client_Defines.h"
#include "ClientReplication.h"
#include "DeployPropRuntime.h"
#include "Level.h"
#include "MapPlacementRuntime.h"
#include "PlayerController.h"

NS_BEGIN(Client)

class CCamera_Free;
class CCharacter;
class IPlayerCommandSink;

class CLevel_ValtanArena final : public CLevel
{
private:
	CLevel_ValtanArena(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CLevel_ValtanArena();

	virtual HRESULT Initialize() override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Ready_Layer_Camera(const wstring_t& strLayerTag);
	bool_t Bind_CameraToLocalCharacter();

private:
	CMapPlacementRuntime m_MapRuntime;
	CDeployPropRuntime m_DeployRuntime;
	shared_ptr<CCamera_Free> m_pCamera = { nullptr };
	weak_ptr<CCharacter> m_pCameraTarget;
	CClientReplication m_Replication;
	shared_ptr<IPlayerCommandSink> m_pPlayerCommandSink;
	CPlayerController m_PlayerController;

public:
	static unique_ptr<CLevel_ValtanArena> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
};

NS_END
