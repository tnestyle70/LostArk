#pragma once

#include "Client_Defines.h"
#include "ClientReplication.h"
#include "Level.h"
#include "MapPlacementRuntime.h"
#include "ValtanCinematicCameraDocument.h"

#include "PartyInteractionView.h"
#include "PlayerController.h"
#include "RaidEntryPreviewView.h"
#include "WorldPlayerChatBubbleView.h"
#include "WorldPlayerNameplateView.h"

NS_BEGIN(Engine)
class CTransform;
NS_END

NS_BEGIN(Client)

class CCamera_Free;
class CCharacter;
class CTrigger_Box;
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

	/* CGameInstance::Draw_Text submits immediately (SpriteBatch), so every LOA-font label in
	this codebase is drawn from its own pass after CImGuiLayer::EndFrame() -- same reason
	Level_CharacterSelect splits Render_ArenaSpawnLabels() out. m_pValtanEntryView is private
	to this level, so CMainApp reaches it through Get_Active() instead of a second
	CRaidEntryPreviewView of its own. Defined out-of-line in the .cpp (not inline here) so
	files that merely include this header for Get_Active() etc. don't also need
	CRaidEntryPreviewView's full definition just to compile it. */
	void Render_ValtanEntryModalText();
	/* Drives the raid-entry popup's own CUI_Sprite visibility/hover state and hit-testing for
	   one frame; the sprites themselves draw through CObject_Manager's normal render cycle, so
	   the old ImGui foreground-drawlist ordering requirement against the combat HUD is gone.
	   Public (not called from this level's own Render()) because CMainApp owns the call site.
	   Only this level reacts to a true return (real NPC entry command); the debug-only preview
	   in Level_CharacterSelect never does. */
	void Render_ValtanEntryModal();
	/* Same reasoning, for CPartyInteractionView's invite-confirm popup text --
	   see CPartyInteractionView::Render_InvitePopupText's own comment. */
	void Render_PartyInviteText()
	{
		m_PartyInteraction.Render_InvitePopupText();
		m_PartyInteraction.Render_ContextMenuText();
	}
	static CLevel_Bern* Get_Active() { return s_pActiveInstance; }
	const LostArk::Shared::S2C_PARTY_ROSTER& Get_PartyRoster() const
	{
		return m_Replication.Get_PartyRoster();
	}
	const CReplicatedPlayerHealth& Get_PlayerHealth() const
	{
		return m_Replication.Get_PlayerHealth();
	}
	const shared_ptr<IPlayerCommandSink>& Get_PlayerCommandSink() const
	{
		return m_pPlayerCommandSink;
	}

private:
	HRESULT Ready_Layer_Camera(
		const wstring_t& strLayerTag,
		const std::string& areaId);

	bool_t Bind_CameraToLocalCharacter();

	/* Out-of-line for the same reason as Render_ValtanEntryModalText() above --
	   keeps CRaidEntryPreviewView's definition out of this header's own
	   compile requirement for unrelated includers. */
	bool_t Is_ValtanEntryModalOpen() const;

	/* Loads the Bern authoring document once and keeps only the two known
	Valtan-entry guide NPCs' authored positions (npc.bern.beda.guide,
	npc.bern.aylara) -- the same static, server-uninvolved position lookup
	Ready_DebugLevelChangeTriggers already does for its own trigger boxes, just
	not _DEBUG-only since the interaction it drives is a real product path. */
	bool_t Ready_ValtanEntryNpcs(const std::string& areaId);
	/* Right-click hit-test against m_ValtanEntryNpcs using a world-ray-vs-sphere
	pick (CPlayerController::Try_PickWorldRay) so the NPC can be clicked from
	anywhere on screen, not just while already standing next to it. A hit
	suppresses that frame's move command and walks the character to the NPC
	instead (Request_MoveToPoint); Advance_ValtanEntryWalk opens the confirm
	window once the character actually arrives. Runs before
	m_PlayerController.Update() each frame for the same suppression-timing
	reason. */
	void Update_ValtanEntryInteraction();
	/* Polled every frame regardless of this frame's click: once
	m_isWalkingToValtanEntryNpc is set, opens the confirm window as soon as
	the local character's live position is back within interaction range of
	the target NPC. */
	void Advance_ValtanEntryWalk();

	/* npc.bern.schmidt's authored position (real placement in Data/Worlds/
	LV_BER_BERNCASTLE/Gameplay.world.json, archetype NPC_SCHMIDT), loaded the same
	way Ready_ValtanEntryNpcs loads its own guide NPCs -- kept as a separate
	single-NPC lookup since it drives an unrelated interaction (opens the Item
	Upgrade window, not a level transfer, and has no confirm modal). */
	bool_t Ready_ItemUpgradeNpc(const std::string& areaId);
	/* Same right-click ray-vs-sphere pick pattern as Update_ValtanEntryInteraction,
	against the single Schmidt NPC position. Uses its own edge-detect state
	(m_wasRightMouseDownForItemUpgradeNpcInteract) rather than sharing
	m_wasRightMouseDownForNpcInteract -- both read the same live mouse button
	each frame independently, which is safe since a click can only ever land
	near one of the two NPCs. */
	void Update_ItemUpgradeNpcInteraction();
	/* Polled every frame: once m_isWalkingToItemUpgradeNpc is set, opens the Item
	Upgrade window (via CMainApp::Get_Active()) as soon as the local character's
	live position is back within interaction range of Schmidt -- no confirm
	modal, unlike Advance_ValtanEntryWalk. */
	void Advance_ItemUpgradeNpcWalk();

	/* Optional entrance cinematic: one authored camera cue from
	Data/Encounters/Bern/BernEntranceCamera.json plays exactly once right after
	entry through the same public product sampler the Valtan cinematics use.
	A missing or invalid document isolates the cinematic and never blocks the
	level; gameplay commands stay suppressed by the existing follow-disabled
	contract while the override owns the camera. */
	bool_t Ready_EntranceCinematic();
	void Update_EntranceCinematic(f32_t fTimeDelta);
	void End_EntranceCinematic();

#ifdef _DEBUG
	bool_t Ready_DebugLevelChangeTriggers(const std::string& areaId);
	/* O opens m_ValtanEntryView without walking to the guide NPC first --
	   debug-only shortcut for iterating on ValtanRaidEntry_Layout.json's visual
	   layout against a real screen instead of re-walking every time. Uses the
	   first authored Valtan-entry NPC's placement id if one is loaded so the
	   real Entrance button still submits a valid Request_ConfirmNpcEntry; falls
	   back to an empty id (Server rejects, panel still previews) if not.
	   Level_CharacterSelect has its own equivalent O-key debug preview, for the
	   same reason but permanently visual-only there (see its own comment). */
	void Update_ValtanEntryDebugPreviewKey();
#endif

private:
	/*베른성 맵 객체들의 생성과 제거는 기존 Map Runtime이 담당한다.
	Network Player 수명과 섞지 않는다.*/
	CMapPlacementRuntime m_MapRuntime;

	shared_ptr<CCamera_Free> m_pCamera = { nullptr };

	weak_ptr<CCharacter> m_pCameraTarget;

	CClientReplication m_Replication;
	CWorldPlayerNameplateView m_PlayerNameplateView;
	CWorldPlayerChatBubbleView m_ChatBubbleView;
	std::vector<REPLICATED_PLAYER_VIEW> m_NameplatePlayers;
	shared_ptr<IPlayerCommandSink> m_pPlayerCommandSink;
	CPartyInteractionView m_PartyInteraction;
	//PlayerController 추가
	CPlayerController m_PlayerController;

	struct VALTAN_ENTRY_NPC
	{
		std::string strPlacementId;
		float3_t vPosition{};
	};
	std::vector<VALTAN_ENTRY_NPC> m_ValtanEntryNpcs;
	unique_ptr<CRaidEntryPreviewView> m_pValtanEntryView;
	bool_t m_isWalkingToValtanEntryNpc = false;
	std::string m_strValtanEntryNpcPlacementId;
	bool_t m_wasRightMouseDownForNpcInteract = false;
	std::uint32_t m_iNextNpcEntryConfirmSequence = 1u;
	bool_t m_bBernBgmStarted = false;

	bool_t m_hasItemUpgradeNpc = false;
	float3_t m_vItemUpgradeNpcPosition{};
	bool_t m_isWalkingToItemUpgradeNpc = false;
	bool_t m_wasRightMouseDownForItemUpgradeNpcInteract = false;

	VALTAN_CINEMATIC_CAMERA_CUE m_EntranceCameraCue;
	bool_t m_hasEntranceCameraCue = false;
	bool_t m_bEntranceCinematicApplied = false;
	bool_t m_bEntranceCinematicDone = false;
	f32_t m_fEntranceCinematicSeconds = 0.f;
	bool_t m_bEntranceRestoreFollowRequested = false;
	bool_t m_wasEscapeDownForEntranceSkip = false;
	weak_ptr<CTransform> m_pEntranceRestoreTarget;

#ifdef _DEBUG
	std::vector<shared_ptr<CTrigger_Box>> m_DebugLevelChangeTriggers;
	bool_t m_wasODownForValtanEntryDebugPreview = false;
#endif

	static CLevel_Bern* s_pActiveInstance;

public:
	static unique_ptr<CLevel_Bern> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
};

NS_END
