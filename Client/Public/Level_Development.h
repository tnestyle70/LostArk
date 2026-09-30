#pragma once

#include "Client_Defines.h"
#include "ClientReplication.h"
#include "DeployPropRuntime.h"
#include "Level.h"
#include "MapPlacementRuntime.h"
#include "PlayerController.h"
#include "PartyInteractionView.h"
#include "WorldPlayerNameplateView.h"
#include "WorldPlayerChatBubbleView.h"

NS_BEGIN(Client)

class CCamera_Free;
class CCharacter;
class CMapLightPresentationRuntime;
class IPlayerCommandSink;
class CMaharakaWaterpangPresentation;
class CInteractKeyPromptView;
class CColosseumIntroCutscene;
class CColosseumMatchStart;
class CRaidEntryPreviewView;

class CLevel_Development final : public CLevel
{
private:
	CLevel_Development(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		LEVEL eLevel);

public:
	virtual ~CLevel_Development();

public:
	virtual HRESULT Initialize() override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

	// The current Development/Training or Maharaka owner exposes its existing camera.
	static CLevel_Development* Get_Active(LEVEL level)
	{
		return s_pActiveInstance && s_pActiveInstance->m_eLevel == level ?
			s_pActiveInstance : nullptr;
	}
	shared_ptr<CCamera_Free> Get_DebugCamera() const { return m_pCamera.lock(); }
	// Product UI borrows the existing typed controller and replicated marker snapshot.
	CPlayerController& Get_PlayerController() { return m_PlayerController; }
	CClientReplication& Get_Replication() { return m_Replication; }
	const shared_ptr<IPlayerCommandSink>& Get_PlayerCommandSink() const { return m_pPlayerCommandSink; }
	const LostArk::Shared::S2C_PARTY_ROSTER& Get_PartyRoster() const { return m_Replication.Get_PartyRoster(); }
	const CReplicatedPlayerHealth& Get_PlayerHealth() const { return m_Replication.Get_PlayerHealth(); }
	void Drain_ChatLines(std::vector<CClientReplication::CHAT_LINE>& lines) { m_Replication.Drain_ChatLines(lines); }
	void Render_PartyInviteText();
	void Render_TravelModal();
	void Render_TravelModalText();
	bool_t Is_TravelModalOpen() const;
	void Collect_MinimapMarkers(
		CClientReplication::MINIMAP_MARKER_SNAPSHOT& outSnapshot) const
	{
		m_Replication.Collect_MinimapMarkers(outSnapshot);
	}
	// Maharaka only: marker over each authored jump (movePlayer) trigger box, like Kouku/Valtan.
	void Submit_TriggerMarkers();
	// Colosseum only: true from level entry until the match intro cutscene has faded out.
	bool_t Is_ColosseumIntroActive() const;
	// Colosseum only: F1 Developer Tools section that replays the match intro cutscene.
	static void Render_ColosseumIntroControls();
#ifdef _DEBUG
	// Borrow the existing Maharaka map; the Level remains its owner.
	CMapPlacementRuntime& Get_MapAuthoringRuntime() { return m_MapRuntime; }
	CDeployPropRuntime& Get_MapAuthoringDeploy() { return m_WaterpangDeploy; }
	const ComPtr<ID3D11Device>& Get_MapAuthoringDevice() const { return m_pDevice; }
	const ComPtr<ID3D11DeviceContext>& Get_MapAuthoringContext() const { return m_pContext; }
	void Set_MapAuthoringActive(bool_t active);
	void Set_WaterpangEffectAuthoringActive(bool_t active);
	std::shared_ptr<CCharacter> Get_DebugLocalCharacter() const { return m_Replication.Get_LocalCharacter(); }
	// Debug F1 typed Server requests (Waterpang forced patterns) go through the level's sink.
	std::shared_ptr<IPlayerCommandSink> Get_DebugCommandSink() const { return m_pPlayerCommandSink; }
	std::shared_ptr<CNpc> Find_MapAuthoringNpc(const std::string& placementId) const
	{ return m_Replication.Find_NpcPlacement(placementId); }
	void Rebase_MapAuthoringSelfMotions(const std::vector<MAP_PLACEMENT_RECORD>& records)
	{ m_MapRuntime.Rebase_AuthoringSelfMotions(records); }
#endif

private:
	HRESULT Ready_Lights();
	HRESULT Ready_Camera(const wstring_t& strLayerTag);
	bool_t Bind_CameraToLocalCharacter();

private:
	// Registry entry this instance plays; only DEVELOPMENT may open the Map Editor.
	LEVEL m_eLevel = LEVEL::DEVELOPMENT;
	CMapPlacementRuntime m_MapRuntime;
	CDeployPropRuntime m_WaterpangDeploy;
#ifdef _DEBUG
	bool_t m_bMapAuthoringActive = false;
	bool_t m_bWaterpangEffectAuthoringActive = false;
#endif
	// Maharaka only: the published source lights of the island, submitted every frame.
	shared_ptr<CMapLightPresentationRuntime> m_pMapLightPresentation;
	bool_t m_bMapLightSubmissionFailureReported = false;
	bool_t m_isMapEditorWorkspace = false;
	weak_ptr<CCamera_Free> m_pCamera;
	weak_ptr<CCharacter> m_pCameraTarget;
	CClientReplication m_Replication;
	shared_ptr<IPlayerCommandSink> m_pPlayerCommandSink;
	CPlayerController m_PlayerController;
	CPartyInteractionView m_PartyInteraction;
	CWorldPlayerNameplateView m_PlayerNameplateView;
	CWorldPlayerChatBubbleView m_ChatBubbleView;
	std::vector<REPLICATED_PLAYER_VIEW> m_NameplatePlayers;
	std::unique_ptr<CRaidEntryPreviewView> m_TravelEntryView;
	uint32_t m_iNextTravelVoteSequence = 1u;
	std::unique_ptr<CMaharakaWaterpangPresentation> m_Waterpang;
	std::unique_ptr<CInteractKeyPromptView> m_InteractPrompt;
	std::unique_ptr<CColosseumIntroCutscene> m_ColosseumIntro;
	// Colosseum only: the countdown banner and the gate that follow the intro cutscene.
	std::unique_ptr<CColosseumMatchStart> m_ColosseumMatchStart;
	bool_t m_bColosseumIntroWasActive = false;
	bool_t m_bColosseumMatchStartArmed = true;
	bool_t m_bColosseumMatchAuthority = false;
	// Maharaka only. One effect.world.move_destination on the exact centre of every enabled
	// single-movePlayer trigger box of the published viewer world document.
	struct TRIGGER_MARKER final
	{
		std::string placementId;
		uint64_t iHandleValue = 0u;
		float4x4_t rootWorld{};
		f32_t seconds = 0.f;
		bool_t started = false;
		bool_t clockStarted = false;
		bool_t active = false;
		bool_t retired = false;
	};
	std::vector<TRIGGER_MARKER> m_TriggerMarkers;
	bool_t Load_TriggerMarkers(const char* pAreaId);
	void Clear_TriggerMarkers();
	void Update_TriggerMarkerClocks(f32_t deltaSeconds);
	static CLevel_Development* s_pActiveInstance;

public:
	static unique_ptr<CLevel_Development> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		LEVEL eLevel = LEVEL::DEVELOPMENT);
};

NS_END
