#pragma once

#include "Client_Defines.h"
#include "ArenaCameraProfile.h"
#include "ClientReplication.h"
#include "Level.h"
#include "LobbyCommandService.h"
#include "MapLightPresentationRuntime.h"
#include "CustomizingCostumeDocument.h"
#include "EquipmentPresentationCatalog.h"
#include "EquipmentPresentationService.h"
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
class CPlayableCharacterAssetService;
class CCustomizingView;
class CUILayoutRuntime;
class CCharacterSelectArenaSpawnGate;
class CRaidEntryPreviewView;
class IPlayerCommandSink;
class IWorldEntityCommandSink;
class CMapLightPresentationRuntime;

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
	bool_t Reload_MapLights();
	void Set_MapLightAuthoringOverride(const shared_ptr<CMapLightPresentationRuntime>& preview)
	{ m_pMapLightAuthoringOverride = preview; }
	const ARENA_CAMERA_PROFILE& Get_FollowCameraProfile() const
	{ return m_FollowCameraProfile; }
	const std::string& Get_FollowCameraProfileStatus() const
	{ return m_strFollowCameraProfileStatus; }
	bool_t Set_FollowCameraProfile(const ARENA_CAMERA_PROFILE& profile,
		std::string& outStatus);



private:
	HRESULT Ready_Lights();
	HRESULT Ready_Camera();
	HRESULT Ready_ServerGameplay();
	bool_t Bind_CameraTarget(const shared_ptr<CCharacter>& character);
	bool_t Request_ClassChange(size_t index);
	void Advance_ClassAssetPreparation();
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
	/* Drives the retail customizing screen: consumes the Create Character button edge to
	open it, applies its camera orbit and head-part reveal while it is up, and turns its
	decide/back edges into the nickname step and the way back to the class roster. */
	void Update_Customizing(f32_t fTimeDelta);
	/* Places the Character Select camera on the pose the customizing screen asks for, through
	CCamera's presentation-override seam so the exact eye, look point and field of view apply
	instead of the follow camera's own smoothing. */
	void Apply_CustomizingCameraPose();
	/* The original character creation screen stands the character on a black stage, so
	the arena's own placements stop drawing while it is up and come back on close. */
	void Open_Customizing();
	void Close_Customizing();
	/* Puts the left column's selected try-on costume on the model through the existing
	equipment preview transaction. Loads its two documents on first use. */
	void Apply_CustomizingCostume();
	/* Same transaction for the hair tab's grid, over the catalog's HEAD sets. */
	void Apply_CustomizingHair();
	/* Puts strSetId in its own slot of m_CustomizingOutfit, drops whatever it collides
	with, and re-applies the whole outfit. */
	void Wear_CustomizingSet(const std::string& strSetId, const char_t* pWhat);
	/* Loads the costume document and the equipment catalog once. */
	bool_t Ensure_EquipmentPresentation();
	bool_t Enter_Stage(LOBBY_STAGE eStage);
	void Render_CreateCharacterProductInputHost();
	void Render_ProductStatus();
	bool_t Is_ProductPointerHovered() const;
#ifdef _DEBUG
	void Render_SelectionPanel();
#endif
	/* Drives the real CUI_Sprite GameObjects (CUILayoutRuntime) for the class roster: the
	accordion row/symbol/thumbnail slots (ClassList_Row0../Symbol0../Thumb/ThumbSymbol/
	ThumbFrame) and the selected class's own ownerClass-tagged right panel slots. Called from
	Update(), not Render() -- these slots self-render through the normal engine pipeline, they
	don't need a per-frame draw call, only their visibility/position/texture state kept current. */
	void Update_ClassList();
	/* Text-only counterpart of Update_ClassList above (class name, category labels, identity
	blurb, "클래스 선택" header) -- uses ImGui's own font/foreground draw list, not
	CGameInstance::Draw_Text, so unlike Render_ArenaSpawnLabels this has to stay in the
	ImGui-active Render() phase instead of the post-EndFrame LOA-font pass. Reads
	m_iExpandedCategory/m_iSelectedClassIndex only -- Update_ClassList (already run earlier this
	same frame, from Update()) owns writing them. */
	void Render_ClassListText();
	/* Real click/hover for GoBackIcon/SpawnMonsterButton/BossSpawnButton/SpawnCancelButton/
	CreateCharacterButton via CUIInputRouter. Called from Update() alongside Update_ClassList. */
	void Update_ArenaSpawnButtons();
	/* Forces every slot Update_ClassList/Update_ArenaSpawnButtons own invisible, without running
	their hover/click handling -- used by both their own MODE::SERVER_ARENA-gated early return
	and Update()'s Is_DebugRaidEntryPreviewOpen() gate, since a CUI_Sprite (unlike the old ImGui
	pass) keeps showing its last state until told otherwise. */
	void Hide_ClassList();
	void Hide_ArenaSpawnButtons();

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
	소환"/"되돌리기"). Draw_Text's SpriteBatch submits immediately, so this stays on the same
	post-CImGuiLayer::EndFrame() pass every other LOA-font label in this codebase uses (same
	reason CMainApp::RenderQuickSlotKeyLabels is split out from RenderCombatHUD) --
	m_pClassSelectView is private to this level, so CMainApp reaches it through Get_Active()
	instead of a second CUILayoutRuntime of its own. */
	void Render_ArenaSpawnLabels();
	/* The Create Character nickname step's own glyphs. Separate from the pass above
	because that one is suppressed while the customizing screen is up, and this modal
	opens from inside it. */
	void Render_CreateCharacterModalText();
	/* The customizing screen's own LOA-font labels, same post-EndFrame pass. */
	void Render_CustomizingText();
	/* True while the customizing screen owns the screen: CMainApp hides the combat HUD
	chrome behind it the same way it does for the Debug raid-entry preview. */
	bool_t Is_CustomizingOpen() const;
	/* Takes the class-list stage down while character creation is open and restores each
	placement's authored visibility when it closes. */
	void Update_CustomizingStageVisibility();
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
	/* The replicated local player (nullptr until the entry snapshot spawned it) -- read-only
	   presentation access for the character info window's live portrait. */
	shared_ptr<CCharacter> Get_LocalCharacter() const
	{
		return m_Replication.Get_LocalCharacter();
	}
	/* Authored and Debug Create Character buttons only stage this request. The common hidden
	product input host consumes it once and calls OpenPopup/BeginPopupModal under one stable ImGui
	ID stack, so Release does not need the visible Character Select diagnostic window. */
	void Request_CreateCharacterButtonClick() { m_hasCreateCharacterButtonClick = true; }
#ifdef _DEBUG
	/* F1 Level Navigation reuses the same typed product routes as this Level's
	   own buttons.  It never reaches the socket or changes Level directly. */
	bool_t Debug_Request_ProductStage(LOBBY_STAGE eStage)
	{
		return Enter_Stage(eStage);
	}
	bool_t Debug_Request_KakulSaydonArena();
	shared_ptr<CCamera_Free> Get_DebugCamera() const { return m_pCamera; }
	CPlayerController& Get_DebugPlayerController() { return m_PlayerController; }
	const string& Debug_GetNavigationStatus() const { return m_strStatus; }
#endif

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

	/* The try-on costumes: which visual set each of the five stands for, the catalog that owns
	their parts, and the service that swaps them onto the model. */
	CCustomizingCostumeDocument m_CostumeDocument{
		"lostark.customizing-costumes",
		"UI/Customizing/CustomizingCostumes.json", "costume" };
	CCustomizingCostumeDocument m_HairstyleDocument{
		"lostark.customizing-hairstyles",
		"UI/Customizing/CustomizingHairstyles.json", "hairstyle" };
	CEquipmentPresentationCatalog m_EquipmentCatalog;
	/* Built on first use: the service needs the device this Level already holds. */
	unique_ptr<CEquipmentPresentationService> m_pEquipmentPresentation;
	bool_t m_isEquipmentPresentationLoaded = false;
	/* What the model is wearing right now, one visual set id per slot. Apply_Preview takes
	the whole outfit, so hair and costume have to be applied together -- passing only the
	slot that just changed took everything else off. */
	std::array<std::string, ETOI(EQUIPMENT_SLOT_ID::END)> m_CustomizingOutfit{};

	CMapPlacementRuntime m_MapRuntime;
	bool_t m_isCustomizingStageHidden = false;
	shared_ptr<CMapLightPresentationRuntime> m_pMapLightPresentation;
	shared_ptr<CMapLightPresentationRuntime> m_pMapLightAuthoringOverride;
	bool_t m_bMapLightSubmissionFailureReported = false;
	unique_ptr<CUILayoutRuntime> m_pClassSelectView = { nullptr };
	unique_ptr<CCustomizingView> m_pCustomizingView;
	int32_t m_iExpandedCategory = -1;
	MODE m_eMode = MODE::CONNECTING;
	size_t m_iSelectedClassIndex = 0;
	std::optional<size_t> m_iPendingClassIndex;
	std::optional<size_t> m_iRequestedClassIndex;
	std::optional<size_t> m_iPreparingClassIndex;
	unique_ptr<CPlayableCharacterAssetService> m_pClassAssetPreparation;
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
	ARENA_CAMERA_PROFILE m_FollowCameraProfile =
		CArenaCameraProfile::Default(ARENA_CAMERA_MAP::CHARACTER_SELECT);
	std::string m_strFollowCameraProfileStatus;
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
	/* UTF-16 twin of m_NicknameDraft -- the edit buffer the runtime text field (WM_CHAR units
	via CUIInputRouter, no ImGui::InputText) actually appends/erases on; re-encoded into
	m_NicknameDraft (the UTF-8 contract Confirm_CreateCharacter validates/sends) on every edit. */
	wstring_t m_NicknameDraftW;
	bool_t m_isCreateCharacterModalOpen = false;
	bool_t m_hasCreateCharacterButtonClick = false;
	/* A Server-approved world transfer hands the live socket to the target
	   Product Level. Ordinary Back/failure destruction still owns the close. */
	bool_t m_preserveServerConnectionForTransfer = false;
#ifdef _DEBUG
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
