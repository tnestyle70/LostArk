#pragma once

#include "Client_Defines.h"
#include "Level.h"
#include "LobbyCommandService.h"

NS_BEGIN(Client)

class CLevel_Loading final : public CLevel
{
private:
	CLevel_Loading(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CLevel_Loading();

public:
	virtual HRESULT Initialize(
		LEVEL eNextLevelID,
		LOBBY_COMMAND_TOKEN lobbyCommandToken);
	virtual void Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	void Recover_FromFailure(HRESULT result);
	void Cancel_LobbyCommand(const char_t* pReason);
	void Retry_LobbyLoad();

private:
	LEVEL m_eNextLevelID = LEVEL::END;
	LOBBY_COMMAND_TOKEN m_iLobbyCommandToken =
		INVALID_LOBBY_COMMAND_TOKEN;
	unique_ptr<class CLoader> m_pLoader = { nullptr };
	bool_t m_isActivationRequested = { false };
	bool_t m_isFailureReported = { false };
	bool_t m_isRetryRequested = { false };

public:
	static unique_ptr<CLevel_Loading> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		LEVEL eNextLevelID,
		LOBBY_COMMAND_TOKEN lobbyCommandToken =
			INVALID_LOBBY_COMMAND_TOKEN);
};

NS_END
