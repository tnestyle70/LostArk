#pragma once

#include "Client_Defines.h"
#include "ClientReplication.h"
#include "Level.h"
#include "LobbyCommandService.h"
#include "MapPlacementRuntime.h"
#include "Network/PacketType.h"
#include "PlayerController.h"

#include <array>
#include <chrono>
#include <optional>

NS_BEGIN(Client)

class CCamera_Free;
class CCharacter;
class CHUDRuntimeView;
class IPlayerCommandSink;
class IWorldEntityCommandSink;

class CLevel_CharacterSelect final : public CLevel
{
private:
	enum class MODE
	{
		CONNECTING,
		SERVER_ARENA,
		RETURNING_TO_LOBBY
	};

private:
	CLevel_CharacterSelect(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CLevel_CharacterSelect();

public:
	virtual HRESULT Initialize() override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Ready_Lights();
	HRESULT Ready_Camera();
	HRESULT Ready_ServerGameplay();
	bool_t Bind_CameraTarget(
		const shared_ptr<CCharacter>& character,
		const float3_t& positionOffset);
	bool_t Request_ClassChange(size_t index);
	void Consume_ClassChangeResults();
	bool_t Synchronize_LocalCharacter();
	void Fail_ServerArena(const string& reason);
	void Update_Connecting();
	void Update_ServerArena();
	bool_t Commit_ServerArena();
	bool_t Request_ValtanSpawn();
	bool_t Enter_Stage(LOBBY_STAGE eStage);
	void Render_SelectionPanel();
	void Render_ClassList();

private:
	static constexpr std::array<
		LostArk::Shared::CHARACTER_CLASS_ID, 6> SUPPORTED_CLASSES =
	{
		LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER,
		LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER,
		LostArk::Shared::CHARACTER_CLASS_ID::SLAYER,
		LostArk::Shared::CHARACTER_CLASS_ID::ARTIST,
		LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER,
		LostArk::Shared::CHARACTER_CLASS_ID::WARLORD
	};

	CMapPlacementRuntime m_MapRuntime;
	unique_ptr<CHUDRuntimeView> m_pClassSelectView = { nullptr };
	int32_t m_iExpandedCategory = -1;
	MODE m_eMode = MODE::CONNECTING;
	size_t m_iSelectedClassIndex = 0;
	std::optional<size_t> m_iPendingClassIndex;
	std::uint32_t m_iNextClassChangeSequence = 1u;
	std::uint32_t m_iPendingClassChangeSequence = 0u;
	shared_ptr<CCharacter> m_pActiveCharacter = { nullptr };
	shared_ptr<CCamera_Free> m_pCamera = { nullptr };
	weak_ptr<CCharacter> m_pCameraTarget;
	CClientReplication m_Replication;
	shared_ptr<IPlayerCommandSink> m_pPlayerCommandSink;
	shared_ptr<IWorldEntityCommandSink> m_pWorldEntityCommandSink;
	CPlayerController m_PlayerController;
	std::chrono::steady_clock::time_point m_ConnectionDeadline{};
	std::chrono::steady_clock::time_point m_ClassChangeDeadline{};
	std::chrono::steady_clock::time_point m_ValtanRequestDeadline{};
	bool_t m_isValtanSpawnRequested = false;
	string m_strStatus =
		"Waiting for the Lobby-approved Server character.";

public:
	static unique_ptr<CLevel_CharacterSelect> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
};

NS_END
