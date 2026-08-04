#pragma once

#include "Client_Defines.h"
#include "Level.h"
#include "LobbyCommandService.h"
#include "Network/PacketType.h"

#include <chrono>

NS_BEGIN(Client)

class CLevel_Lobby final : public CLevel
{
private:
	enum class ENTRY_STATE
	{
		IDLE,
		WAITING_FOR_APPROVAL
	};

private:
	CLevel_Lobby(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CLevel_Lobby();

public:
	virtual HRESULT Initialize() override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	bool_t Begin_StageRequest(LOBBY_STAGE eStage);
	bool_t Begin_NetworkEntry(
		LostArk::Shared::WORLD_ID eWorldId,
		LEVEL eTargetLevel);
	bool_t Resolve_Stage(
		LOBBY_STAGE eStage,
		LostArk::Shared::WORLD_ID& outWorldId,
		LEVEL& outTargetLevel) const;
	void Consume_EnterAccepted();
	void Cancel_PendingEntry(const string& reason);
	void Render_StagePanel();

private:
	ENTRY_STATE m_eEntryState = ENTRY_STATE::IDLE;
	LostArk::Shared::WORLD_ID m_ePendingWorldId =
		LostArk::Shared::WORLD_ID::END;
	LEVEL m_ePendingLevel = LEVEL::END;
	std::chrono::steady_clock::time_point m_ApprovalDeadline{};
	string m_strStatus =
		"Choose a stage directly or open Character Select to change class.";

public:
	static unique_ptr<CLevel_Lobby> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
};

NS_END
