#pragma once

#include "Client_Defines.h"
#include "Level.h"
#include "LevelTransitionService.h"
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
	bool_t Begin_StageRequest(const LOBBY_COMMAND& command);
	bool_t Begin_NetworkEntry(
		LostArk::Shared::WORLD_ID eWorldId,
		LEVEL eTargetLevel,
		LOBBY_COMMAND_PURPOSE purpose);
	bool_t Resolve_Stage(
		LOBBY_STAGE eStage,
		LOBBY_COMMAND_PURPOSE purpose,
		LostArk::Shared::WORLD_ID& outWorldId,
		LEVEL& outTargetLevel) const;
	void Consume_EnterRejected();
	void Consume_EnterAccepted();
	void Cancel_PendingEntry(
		const string& reason,
		LostArk::Shared::SESSION_DIAGNOSTIC_REASON diagnosticReason,
		const char_t* pDiagnosticSource);
#ifdef _DEBUG
	void Render_StagePanel();
#endif

private:
	ENTRY_STATE m_eEntryState = ENTRY_STATE::IDLE;
	LostArk::Shared::WORLD_ID m_ePendingWorldId =
		LostArk::Shared::WORLD_ID::END;
	LEVEL m_ePendingLevel = LEVEL::END;
	LOBBY_COMMAND_PURPOSE m_ePendingPurpose =
		LOBBY_COMMAND_PURPOSE::GAMEPLAY;
	bool_t m_hasPendingCharacterCreationEntry = false;
	std::chrono::steady_clock::time_point m_ApprovalDeadline{};
	bool_t m_hasRecoveryDiagnostic = false;
	CLIENT_RECOVERY_DIAGNOSTIC m_RecoveryDiagnostic;
	string m_strStatus =
		"Choose a stage directly or open Character Select to change class.";
	static CLevel_Lobby* s_pActiveInstance;

public:
	static bool_t Can_SubmitProductCommand();
	static unique_ptr<CLevel_Lobby> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
};

NS_END
