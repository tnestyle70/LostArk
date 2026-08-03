#pragma once

#include "Client_Defines.h"
#include "ClientReplication.h"
#include "Level.h"
#include "MapPlacementRuntime.h"

#include "PlayerController.h"

NS_BEGIN(Client)

class CCamera_Free;
class CCharacter;
class IPlayerCommandSink;

class CLevel_Bern final : public CLevel
{
private:
	CLevel_Bern(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CLevel_Bern();

public:
	virtual HRESULT Initialize() override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Ready_Layer_Camera(
		const wstring_t& strLayerTag);

	bool_t Bind_CameraToLocalCharacter();

private:
	/*베른성 맵 객체들의 생성과 제거는 기존 Map Runtime이 담당한다.
	Network Player 수명과 섞지 않는다.*/
	CMapPlacementRuntime m_MapRuntime;

	shared_ptr<CCamera_Free> m_pCamera = { nullptr };

	weak_ptr<CCharacter> m_pCameraTarget;

	CClientReplication m_Replication;
	shared_ptr<IPlayerCommandSink> m_pPlayerCommandSink;
	//PlayerController 추가
	CPlayerController m_PlayerController;

public:
	static unique_ptr<CLevel_Bern> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
};

NS_END
