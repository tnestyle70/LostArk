#pragma once

#include "Client_Defines.h"
#include "ClientReplication.h"
#include "Level.h"
#include "LobbyCommandService.h"
#include "MapPlacementRuntime.h"
#include "Network/SessionDiagnostic.h"
#include "Network/PacketType.h"
#include "PlayerController.h"

#include <array>
#include <chrono>
#include <cstdint>
#include <optional>
#include <vector>

NS_BEGIN(Client)

class CCamera_Free;
class CCharacter;
class CHUDRuntimeView;
class CCharacterSelectArenaSpawnGate;
class CRaidEntryPreviewView;
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

	enum class CLASS_PRESENTATION_PREPARATION_STATE
	{
		IDLE,
		WAITING_FOR_PRODUCT_EFFECTS,
		REGISTRATION_FAILURE_ISOLATED
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
	bool_t Advance_DeferredClassPresentation();
	bool_t Is_ClassPresentationPreparationPending() const;
	void Reset_ClassPresentationPreparation();
	bool_t Synchronize_LocalCharacter();
	void Fail_ServerArena(
		const string& reason,
		LostArk::Shared::SESSION_DIAGNOSTIC_REASON diagnosticReason);
	void Leave_ServerArena();
	void Return_ServerArenaToLobby(
		const string& reason,
		const char_t* pTransitionSource);
	void Update_Connecting();
	void Update_ServerArena();
	bool_t Commit_ServerArena();
	bool_t Request_SelectedArenaSpawn();
	void Advance_ArenaSpawnRequest();
	void Isolate_ValtanSpawnPreparationFailure(
		const std::string& reason,
		bool_t bTimedOut);
	void Reset_ArenaSpawnRequest();
	void Open_CreateCharacterModal();
	bool_t Confirm_CreateCharacter();
	void Cancel_CreateCharacter();
	void Render_CreateCharacterModal();
	bool_t Enter_Stage(LOBBY_STAGE eStage);
	void Render_CreateCharacterProductInputHost();
	void Render_ProductStatus();
	bool_t Is_ProductPointerHovered() const;
#ifdef _DEBUG
	void Render_SelectionPanel();
#endif
	void Render_ClassList();
	/* Real click/hover for GoBackIcon/SpawnMonsterButton/BossSpawnButton/SpawnCancelButton --
	CHUDRuntimeView has no hit-test or hover of its own (see HUDRuntimeView.cpp), so this follows
	the same hand-rolled mouse-vs-rect pattern Render_ClassList already uses for the class list. */
	void Render_ArenaSpawnButtons();

#ifdef _DEBUG
	/* O opens a visual-only preview of the same "군단장 레이드 입장" panel
	   Level_Bern's guide NPC uses (CRaidEntryPreviewView, shared so this isn't
	   a second runtime of the same role) -- there is no NPC, no walk, and no
	   real entry command here, purely so the panel's layout can be checked
	   from Character Select too. Entrance/Decline just close it. */
	void Update_RaidEntryDebugPreviewKey();
#endif

public:
	/* SpawnMonsterButton/BossSpawnButton/SpawnCancelButton's small labels ("몬스터 소환"/"보스
	소환"/"되돌리기"). CGameInstance::Draw_Text submits immediately, but Render_ArenaSpawnButtons'
	hover art is composited later inside CImGuiLayer::EndFrame(), so this must run after EndFrame
	(same reason CMainApp::RenderQuickSlotKeyLabels is split out from RenderCombatHUD) --
	m_pClassSelectView is private to this level, so CMainApp reaches it through Get_Active()
	instead of a second CHUDRuntimeView of its own. */
	void Render_ArenaSpawnLabels();
#ifdef _DEBUG
	/* Same split as Render_ArenaSpawnLabels just above, plus the
	   GetForegroundDrawList() submission-order requirement
	   CRaidEntryPreviewView::Render() documents -- CMainApp calls this after
	   the combat HUD renders, and RenderText() after CImGuiLayer::EndFrame(). */
	void Render_RaidEntryDebugPreview();
	void Render_RaidEntryDebugPreviewText();
	/* CMainApp uses this to skip RenderCombatHUD/RenderSkillIcons/RenderQuickSlot
	   (and this level's own Render_SelectionPanel) while the preview is open --
	   those all draw the player's class HUD chrome, which otherwise bleeds
	   through the same screen region as the preview's left info column and
	   panel frame. Out-of-line: keeps CRaidEntryPreviewView's definition out of
	   this header's own compile requirement for unrelated includers. */
	bool_t Is_DebugRaidEntryPreviewOpen() const;
#endif
	static CLevel_CharacterSelect* Get_Active() { return s_pActiveInstance; }
	/* Authored and Debug Create Character buttons only stage this request. The common hidden
	product input host consumes it once and calls OpenPopup/BeginPopupModal under one stable ImGui
	ID stack, so Release does not need the visible Character Select diagnostic window. */
	void Request_CreateCharacterButtonClick() { m_hasCreateCharacterButtonClick = true; }

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
	std::uint32_t m_iNextDespawnRequestSequence = 1u;
	std::uint32_t m_iNextKakulArenaRequestSequence = 1u;
	std::uint32_t m_iPendingClassChangeSequence = 0u;
	CLASS_PRESENTATION_PREPARATION_STATE
		m_eClassPresentationPreparationState =
			CLASS_PRESENTATION_PREPARATION_STATE::IDLE;
	std::uint64_t m_iClassPresentationPreparationGeneration = 0u;
	LostArk::Shared::NET_ENTITY_ID m_iClassPresentationNetEntityId =
		LostArk::Shared::INVALID_NET_ENTITY_ID;
	LostArk::Shared::CHARACTER_CLASS_ID m_eClassPresentationTargetClass =
		LostArk::Shared::CHARACTER_CLASS_ID::END;
	bool_t m_hasClassPresentationCommitAttempted = false;
	std::vector<std::string> m_ClassPresentationEffectTargets;
	std::string m_strClassPresentationPreparationFailure;
	std::string m_strClassPresentationCommitWarning;
	shared_ptr<CCharacter> m_pActiveCharacter = { nullptr };
	shared_ptr<CCamera_Free> m_pCamera = { nullptr };
	weak_ptr<CCharacter> m_pCameraTarget;
	CClientReplication m_Replication;
	shared_ptr<IPlayerCommandSink> m_pPlayerCommandSink;
	shared_ptr<IWorldEntityCommandSink> m_pWorldEntityCommandSink;
	CPlayerController m_PlayerController;
	std::chrono::steady_clock::time_point m_ConnectionDeadline{};
	std::chrono::steady_clock::time_point m_ClassChangeDeadline{};
	std::chrono::steady_clock::time_point m_ArenaSpawnRequestDeadline{};
	std::chrono::steady_clock::time_point m_ValtanPrewarmDeadline{};
	size_t m_iSelectedArenaSpawnIndex = 0;
	std::optional<size_t> m_iArenaSpawnIntentIndex;
	std::optional<size_t> m_iPendingArenaSpawnIndex;
	std::array<bool_t, 3> m_ArenaSpawnAccepted{};
	unique_ptr<CCharacterSelectArenaSpawnGate> m_pArenaSpawnGate;
	std::vector<std::string> m_ValtanEffectPreparationTargets;
	std::array<char_t,
		LostArk::Shared::MAX_NICKNAME_BYTES + 1u> m_NicknameDraft{};
	bool_t m_isCreateCharacterModalOpen = false;
	bool_t m_hasCreateCharacterButtonClick = false;
	/* A Server-approved world transfer hands the live socket to the target
	   Product Level. Ordinary Back/failure destruction still owns the close. */
	bool_t m_preserveServerConnectionForTransfer = false;
#ifdef _DEBUG
	bool_t m_isCombatColliderDebugVisible = false;
	bool_t m_isSkillHitAreaDebugVisible = true;
	unique_ptr<CRaidEntryPreviewView> m_pDebugRaidEntryPreviewView;
	bool_t m_wasODownForRaidEntryDebugPreview = false;
#endif
	string m_strStatus =
		"Waiting for the Lobby-approved Server character.";
	static CLevel_CharacterSelect* s_pActiveInstance;

public:
	static unique_ptr<CLevel_CharacterSelect> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
};

NS_END
