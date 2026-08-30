#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "LobbyCommandService.h"
#include "Network/PacketMessages.h"
#include "RenderingProfileService.h"

#include <filesystem>

NS_BEGIN(Engine)
class CImGuiLayer;
NS_END

NS_BEGIN(Client)

class CMapTool;
class CEffect_Tool;
class CEffect_Tool_V2;
class CAnimation_Tool;
class CHUDLayoutTool;
class CHUDRuntimeView;
class CBalanceTool;
class CBossTool;
class CCameraTool;
class CCharacterPreviewPanel;
class CSkillWindowView;
class CInventoryView;
class CChatWindowView;
class CPartyWindowView;


class CMainApp final
{
#ifdef _DEBUG
private:
		enum class DEBUG_TOOL
	{
		NONE,
		MAP,
		ANIMATION,
		EFFECT,
		EFFECT_V2,
		RENDERING,
		UI,
		BALANCE,
		BOSS,
		CAMERA,
		COUNT
	};

	struct DEBUG_RESOURCE_FILE
	{
		string strDomain;
		string strSource;
		string strRelativePath;
		string strSearchText;
		DEBUG_TOOL eTool = DEBUG_TOOL::NONE;
	};
#endif

private:
	CMainApp();

public:
	~CMainApp();

public:
	HRESULT Initialize();
	void Update(f32_t fTimeDelta);
	HRESULT Render();

	/* Shared one-shot click sound for every real UI button (not Debug/Tool authoring
	widgets) -- single source of truth for the sound asset path so every button's own
	click-handling code calls this instead of repeating the literal path. */
	static void Play_UIButtonClickSound();

	/* CMainApp is a single process-lifetime instance (see Create()/Free()), but
	nothing previously exposed it back to a Level the way CLevel_Bern/
	CLevel_ValtanArena expose Get_Active() to CMainApp -- Open_ItemUpgradeWindow()
	below is the first case that needs the reverse direction (Bern's Schmidt NPC
	interaction opening the Item Upgrade window CMainApp itself owns). */
	static CMainApp* Get_Active() { return s_pActiveInstance; }
	/* Opens the Item Upgrade preview the same way the P key's rising edge does
	(same idle-state reset block), but idempotently -- a no-op if already open.
	Used by CLevel_Bern's Schmidt NPC right-click interaction; P itself still
	toggles (open when closed, close when open). */
	void Open_ItemUpgradeWindow();

#ifdef _DEBUG
	static void Update_DebugWindowTitleWithFps(const wchar_t* pBaseTitle);
	/* Every domain tool writes one stable Pattern ID into this process-wide
	   workspace selection before Complete Play.  The tools retain their local
	   draft/row state, but none owns a second Server replay selection or queue. */
	bool_t Debug_SelectCompletePlayPattern(const std::string& strPatternId);
	const std::string& Debug_GetSelectedCompletePlayPatternId() const;
	/* Shortcut used by every Complete Play button.  Submission still crosses
	   CBossTool -> CValtanPatternAuditionService and never resets Arena world,
	   navigation, wall, or debris state. */
	bool_t Debug_CompletePlaySelected(std::string& strOutStatus);
#endif

private:
	HRESULT Ready_Fonts();
	HRESULT Ready_Prototype_For_Static();
	HRESULT Ready_Prototype_For_LoadingChrome();
	HRESULT Start_Level(
		LEVEL eTargetLevel,
		LOBBY_COMMAND_TOKEN lobbyCommandToken =
			INVALID_LOBBY_COMMAND_TOKEN);
	void Apply_LevelRequest();
	HRESULT ReadyImGuiRuntime();
	void RenderCombatHUD();
	/* HealthBar/ManaBar's own JSON layer (HUD_Layout.json) is now just the dark empty-state
	background ("Empty bar.png"/"Empty bar reverse.png"), always drawn full by the generic
	CHUDRuntimeView::Render() pass. This draws the real colored fill ("HP Bar.png"/"MP Bar.png")
	UV-clipped by current/maximum HP and resource on top of it, same technique as the boss bar's
	fill -- CHUDRuntimeView::Render() never dynamically resized anything, so the bar previously
	always showed full regardless of actual HP/MP. */
	void RenderPlayerHealthManaBar();
	/* Draws whatever item icon is registered onto Item_1..4, and consumes any pending
	CInventoryView drag-drop to register a new one. Called after m_pHUDRuntimeView->Render()
	so the icon overlay lands on top of the already-drawn slot background. */
	void Render_ItemQuickSlots();
	/* Q/W/E/R/A/S/D/F, the right-side special-skill row (6/7/8/9/0 for now -- SpecialSkill_1..5;
	SpecialSkill_6 has no assigned key label yet), and the item quick slots (1/2/3/4) each get
	their bound key drawn into the pointed tab at the bottom of "Empty Slot.png"/"Empty Slot
	2.png"'s own art. Called after EndFrame() like the other LOA-font text, for the same
	z-order reason as Render_Text(). */
	void RenderQuickSlotKeyLabels();
	/* Lobby's authored Test/Character Select/Valtan/Bern buttons -- screen-space hit test against
	their stable Lobby_Layout.json slots, submitting the matching typed command only while the
	active Lobby is idle. A missing/old partial layout atomically falls back to the same four rects
	and product textures. This also draws the Release status line before EndFrame(). */
	void Render_LobbyButtons();
	/* White labels for the four authored Lobby command buttons. Called after EndFrame() like the
	other LOA-font text, for the same z-order reason as RenderQuickSlotKeyLabels. */
	void RenderLobbyButtonText();
	/* White "장비 재련" label for ItemUpgrade_ReforgeButton, same reasoning/pattern as
	RenderLobbyButtonText() -- the button image itself is blank (reused from
	UI/Lobby/create_character_button.png), text drawn separately on top. */
	void RenderItemUpgradeButtonText();
	void RenderItemUpgradeLevelText();
	void RenderItemUpgradeMaterialCounts();
	void RenderItemUpgradeGaugePercentText();
	/* "화면을 클릭하여 결과 즉시 확인" -- shown only while ITEM_UPGRADE_ATTEMPT_RESULT::WAITING,
	over the real SmeltLoding wait circle (ItemUpgrade_ResultWaitEmblem), matching the real
	reference screenshot's prompt placement near the bottom of the wait panel. */
	void RenderItemUpgradeResultWaitText();
	/* Item name/grade/"재련 성공" over the real diamond-frame decoration (ItemUpgrade_SuccessDiamondWinged/
	Frame, ItemUpgrade_SuccessItemIconMarker) while ITEM_UPGRADE_ATTEMPT_RESULT::SUCCESS is showing. */
	void RenderItemUpgradeSuccessDetailText();
	/* Item name/"재련 실패" over the real diamond-frame decoration (ItemUpgrade_FailDiamondFrame,
	ItemUpgrade_FailItemIconMarker) while ITEM_UPGRADE_ATTEMPT_RESULT::FAIL is showing. No grade/level
	marker -- a failed reforge does not change level. */
	void RenderItemUpgradeFailDetailText();
	void RenderItemUpgradeListText();
	/* Hover/click hit-test for the 6 left-list rows, mirrored from Render_LobbyButtons's
	Lobby_CreateCharacterButton pattern (screen-space rect from Get_SlotRect, ImGui mouse). On
	click, moves ItemUpgrade_ListSelectedExample and swaps ItemUpgrade_SelectedItemIcon's texture
	to the clicked row. */
	void Update_ItemUpgradeSelection();
	/* Hover/click hit-test for ItemUpgrade_LevelUpBtn ("성장"), same pattern as
	Update_ItemUpgradeSelection. On click (re)starts the 0->100 gauge fill from 0, restarts
	ItemUpgrade_CoreFlash once, and hides WingedRingGold/LevelUpMotion2Big/CompleteEffect until
	the fill reaches 100. */
	void Update_ItemUpgradeGrowButton();
	/* Hover/click hit-test for ItemUpgrade_ReforgeButton ("장비 재련"), same pattern as
	Update_ItemUpgradeGrowButton. Only acts once the gauge is held at 100% (a completed "성장");
	on click rolls a placeholder pass/fail (no real Server 재련 probability exists yet) into
	m_bItemUpgradePendingAttemptSuccess and shows the "화면을 클릭하여 결과 즉시 확인" wait
	overlay instead of the result immediately -- the roll is already decided, just not revealed. */
	void Update_ItemUpgradeReforgeButton();
	/* While ITEM_UPGRADE_ATTEMPT_RESULT::WAITING is showing, any left-click anywhere (matching the
	real "화면을 클릭하여 결과 즉시 확인" prompt, not one specific button rect) reveals the
	already-rolled outcome: hides the wait overlay and shows the matching result modal. */
	void Update_ItemUpgradeResultWaitClick();
	/* Hover/click hit-test for whichever of ItemUpgrade_SuccessOkBtn/_FailOkBtn is currently
	shown. On click, hides both result modals and resets the gauge back to idle (0%) the same way
	the P-key reopen path does, so the button can be tried again. */
	void Update_ItemUpgradeResultOkButton();
	/* Shared by Update_ItemUpgradeReforgeButton (snaps the gauge back to 0% the instant 재련 is
	clicked, so the wait/result screens never show the old 100% fill behind them) and
	Update_ItemUpgradeResultOkButton (same reset on dismiss) -- an idle, non-growing 0% state. */
	void Reset_ItemUpgradeIdleGauge();
	/* Returns a mutable reference into m_ItemUpgradeLevels, default-initializing an itemId's first
	lookup to 10. Every reader/writer of an item's 재련 level goes through this so a never-seen
	item and a previously-touched one behave identically. */
	int32_t& ItemUpgradeLevelRef(const string& strItemId);
	/* Hides (bVisible=false) or restores (true) the base window's own center-panel content --
	item icon, gauge, recipe materials, 성장/재련 buttons, level-transition arrow -- so none of it
	bleeds through behind the wait/success/fail modals. Real Lost Ark's wait/result screens read
	as their own separate screen (no base-window clutter visible behind them); this is the fix for
	that. Only ever restores the always-idle-visible set, never the gauge completion effects
	(WingedRingGold/LevelUpMotion2Big/CompleteEffect etc.), which stay owned by
	Reset_ItemUpgradeIdleGauge/Update_ItemUpgradeGrowButton's own state machine. */
	void Set_ItemUpgradeCenterPanelVisible(bool_t bVisible);
	void RenderBossHealthBar();
	/* Real HOLD skill (PLAYER_SKILL_KIND::HOLD) charge bar -- ChargeGauge_Bg/_Track/_Fill in
	HUD_Layout.json (ownerClass:null, same as HealthBar). Progress is reconstructed client-side
	from real Data/Balance/PlayerSkills.json comboStages[].actionDurationMs and the Server-owned
	iComboStage/iActionStartTick fields (no continuous charge-percent field exists on the wire). */
	void RenderChargeGauge();
	/* Skill display name, centered (X) over ChargeGauge_Track, drawn after
	CImGuiLayer::EndFrame() -- same reason as RenderBossHealthBarText, RenderChargeGauge's own
	fill image only composites there. */
	void RenderChargeGaugeText();
	/* Boss title/HP/bar-count text. Split out from RenderBossHealthBar and called after
	CImGuiLayer::EndFrame() (next to RenderCombatHUDText, same reason) -- CGameInstance::Draw_Text
	submits its SpriteBatch draw immediately, but the bar/frame images RenderBossHealthBar draws
	via ImGui's foreground draw list only composite later, inside EndFrame(). Calling both from
	inside the Begin/EndFrame block let the ImGui-composited opaque fill bury this text underneath
	it every frame. */
	void RenderBossHealthBarText();
	/* "사망하였습니다" title + "부활"/"관전하기" button labels over Valtan's death-screen panel
	(CLevel_ValtanArena owns the panel/button images and the revive click hit-test; this only
	draws the Korean text). Called after CImGuiLayer::EndFrame(), same reason as
	RenderBossHealthBarText -- CGameInstance::Draw_Text paints immediately, but the panel/button
	images composite later inside EndFrame() and would otherwise bury text drawn earlier in the
	frame. Rects come from CCombatHUDViewModel::Get_DeadSceneTextRects(), which
	CLevel_ValtanArena::Update_DeadScene() fills from its own (Level-private) m_pDeadSceneView
	every frame -- not hand-copied constants, so repositioning a slot in the HUD Layout Tool moves
	this text with it. */
	void RenderDeadSceneText();
	/* Same split as RenderDeadSceneText(), for the Valtan clear celebration overlay's real
	"[$]commander.dungeon_clear" headline. Rect comes from
	CCombatHUDViewModel::Get_RaidClearTextRects(), which
	CLevel_ValtanArena::Update_RaidClear() fills from its own (Level-private)
	m_pRaidClearView every frame the overlay is showing. */
	void RenderRaidClearText();
	/* Item acquisition toast headline -- same split as RenderRaidClearText, reading
	CCombatHUDViewModel::Get_ItemAnnounceTextRects(), which CLevel_ValtanArena::Update_ItemAnnounce()
	fills (rect AND the already-localized "OO을(를) 획득하였습니다" text) every frame a toast is
	showing. */
	void RenderItemAnnounceText();
	/* Room-shared raid Esther gauge bar. Draws nothing when the snapshot says
	the world has no Esther roster (maximum 0). */
	void RenderEstherGauge();
	/* LanceMaster's 3-segment identity meter -- drawn procedurally (matching the real
	LanceMasterProgress.as formula: target.rotation = maxDegree * value/100, cascading through
	3 segments) rather than from extracted art, since the real asset's moving "target" piece is
	a vector shape this pipeline can't crop as an image (see .../HudGfx_Extracted notes). */
	void RenderLanceMasterIdentityGauge();
	/* Floating combat-log numbers at each DAMAGE_EVENT's real hit position (Get_DamageEvents(),
	server-authoritative). Positions are world-space and captured at hit time, so a number stays
	where the hit landed instead of following the target. */
	void RenderDamageNumbers();
	void RenderSkillIcons();
	void RenderSkillCooldowns();
	/* Experimental replacement for RenderSkillIcons/RenderSkillCooldowns, built from the real
	extracted QuickSlot.gfx Scaleform asset (icon frame, cooldown sweep, on-use flash) instead
	of hand-placed layers -- see .../HudGfx_Extracted. */
	void RenderQuickSlot();
	void RenderCombatHUDText();

#ifdef _DEBUG
	HRESULT ReadyDebugTools();
	HRESULT EnsureDebugTool(DEBUG_TOOL eTool);
	bool_t IsDebugToolVisible(DEBUG_TOOL eTool) const;
	void SetDebugToolVisible(DEBUG_TOOL eTool, bool_t bVisible);
	void CloseAllDebugTools();
	void RefreshDebugResourceFiles();
	void RenderDebugResourceFiles();
	void OpenDebugResourceFile(size_t iFile);
	void RefreshCompletePlayPatternOptions();
	void RenderCompletePlayControls();
	void RenderServerArenaActiveControls();
	void UpdateDebugToolShortcut();
	void RenderDeveloperTools();
	void RenderRenderingWorkbench();
	void RenderProfilerOverlay();
	void RenderProfilerSettings();
#endif

private:
	ComPtr<ID3D11Device> m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext> m_pContext = { nullptr };
	CRenderingProfileService m_RenderingProfiles;
	unique_ptr<Engine::CImGuiLayer> m_pImGuiLayer = { nullptr };
	/* Not _DEBUG-gated: the runtime HUD art must render in Release too. */
	unique_ptr<CHUDRuntimeView> m_pHUDRuntimeView = { nullptr };
	/* UI/BossUI/BossUI.json's runtime consumer (RenderBossHealthBar) -- the boss health bar isn't
	tied to the local player's own class (m_pHUDRuntimeView/Combat HUD) and isn't part of the
	always-on top/bottom menu chrome (Screen UI) either, so it owns its own document/tab instead of
	being folded into either. */
	unique_ptr<CHUDRuntimeView> m_pBossUIView = { nullptr };
	/* UI/Esther/EstherUI.json's runtime consumer (RenderEstherGauge) -- same reasoning as
	m_pBossUIView: the Esther skill window is shared across every class, not tied to Combat HUD
	or Screen UI, so it gets its own document/tab too. */
	unique_ptr<CHUDRuntimeView> m_pEstherUIView = { nullptr };
	/* UI/ItemUpgrade/ItemUpgradeUI.json's runtime consumer -- a plain generic Render("Default", 0)
	pass (no per-slot hand-coded logic like RenderBossHealthBar's, since no real Server-side
	재련/enhancement data exists yet). Not _DEBUG-gated, same as m_pInventoryView/m_pSkillWindowView.
	P is a free normal gameplay keybind (not an F1/F6 tool-switch key) -- toggled the same
	GetAsyncKeyState edge-detect pattern as K/I below, since it started as an always-on debug
	preview that ended up blocking everything else on screen. */
	unique_ptr<CHUDRuntimeView> m_pItemUpgradeView = { nullptr };
	bool_t m_bItemUpgradePreviewVisible = false;
	/* Index into BuildItemUpgradeSlots()'s current-frame result (real "combat"-category inventory
	items) that is the current "재련 대상" -- Update_ItemUpgradeSelection() is the only writer, on
	a left-list row click. The list is rebuilt from the live inventory every frame, so this index
	is clamped against its size wherever it's read rather than a fixed 0..5 range. */
	int32_t m_iItemUpgradeSelectedSlot = 0;
	/* Real per-item level state, keyed by itemId (not a fixed-size array -- which equipment items
	exist depends on the live inventory). ItemUpgradeLevelRef() default-inits an item's first
	lookup to 10 and returns a mutable reference; Update_ItemUpgradeResultWaitClick increments the
	selected item's entry by 1 the moment a 재련 attempt actually succeeds, a fail leaves it
	untouched. Every level display in this preview (left list, right 재련 단계 ladder, center
	현재/다음, success detail) reads through the same helper so they can never drift out of sync. */
	unordered_map<string, int32_t> m_ItemUpgradeLevels;
	bool_t m_bPDown = false;
	/* Current held ItemUpgrade_GaugeFill percent (0..100), driven by the ItemUpgrade_LevelUpBtn
	("성장") click state machine (see m_bItemUpgradeGrowing) instead of a free-running clock. Stays
	0 until the button is clicked, holds at 100 once the fill completes. Also read directly by
	RenderItemUpgradeGaugePercentText() for the "%" number. */
	int32_t m_iItemUpgradePreviousPercent = 0;
	/* true while the 0->100 fill triggered by ItemUpgrade_LevelUpBtn is in progress -- gates
	ItemUpgrade_WingedRingGold/_LevelUpMotion2Big/_CompleteEffect (hidden while growing or idle,
	shown only once iPercent reaches 100) and ItemUpgrade_SmeltGlow (loops only while idle, i.e.
	!m_bItemUpgradeGrowing && 0 == m_iItemUpgradePreviousPercent). */
	bool_t m_bItemUpgradeGrowing = false;
	f64_t m_dItemUpgradeGrowStartSeconds = -1.0;
	/* Wall-clock start of WingedRingGold/LevelUpMotion2Big's fade-in (alpha 0->1) once the fill
	reaches 100% -- negative = not fading (either idle/growing with both hidden, or the fade
	already finished and both sit at full alpha). Reset to -1 on every new button click so the
	next completion fades in fresh instead of resuming a stale progress. */
	f64_t m_dItemUpgradeCompleteRevealStartSeconds = -1.0;
	/* Fires ItemUpgrade_CoreFlash exactly once at the start of a fill (Update_ItemUpgradeGrowButton
	sets this true on click; the per-frame gauge block consumes it the same frame and clears it) --
	real Scaleform ordering plays coreLevelEffect1 before compF_shockwave_red inside
	levelUpMotion_mc, so ShockwaveRing is scheduled for CoreFlash's own real 28-frame/20fps
	duration later instead of together. */
	bool_t m_bItemUpgradeCoreFlashPending = false;
	f64_t m_dItemUpgradeShockwaveScheduledAt = -1.0;
	/* Which result modal (if any) ItemUpgrade_ReforgeButton's last roll produced. NONE means no
	attempt is being shown, so the gauge/reforge button stays interactive; SUCCESS/FAIL means one
	of ItemUpgrade_SuccessModalBg/_FailModalBg (+ its own OK button) is on screen and blocks a new
	attempt until Update_ItemUpgradeResultOkButton() dismisses it. */
	enum class ITEM_UPGRADE_ATTEMPT_RESULT { NONE, WAITING, SUCCESS, FAIL };
	ITEM_UPGRADE_ATTEMPT_RESULT m_eItemUpgradeAttemptResult = ITEM_UPGRADE_ATTEMPT_RESULT::NONE;
	/* Rolled the instant Update_ItemUpgradeReforgeButton() clicks, but not shown until
	Update_ItemUpgradeResultWaitClick() reveals it -- WAITING holds this pending outcome so the
	"화면을 클릭하여 결과 즉시 확인" suspense screen can sit in front of an already-decided result. */
	bool_t m_bItemUpgradePendingAttemptSuccess = false;
	/* Real reforge result flow: the SmeltLoding circle keeps playing UNDER the SmeltSuccess/Fail
	burst only while that burst is actually playing (90 frames/30fps, real loop=false duration).
	Update_ItemUpgradeResultWaitClick() sets this to that real duration's wall-clock end; the per-
	frame check in Update() hides both the circle and the burst layer once past it, settling to the
	plain icon/name/result screen. Negative = no settle pending. */
	f64_t m_dItemUpgradeResultSettleAt = -1.0;
	/* Edge-detects the local player's stance so RenderCombatHUD only calls
	CHUDRuntimeView::Play_KeyframeAnimation on an actual change (or the first frame a stance is
	known at all), instead of re-triggering the icon's animation every frame. NONE never matches a
	real stance, so the very first Render sees an edge and plays the arrival pose. */
	LostArk::Shared::PLAYER_STANCE_ID m_ePreviousHudStance =
		LostArk::Shared::PLAYER_STANCE_ID::NONE;
	/* RenderChargeGauge's own default (single continuous fill) model needs the REAL elapsed time
	of each completed comboStage, not its authored iActionDurationMs -- a HOLD skill's loop stage
	can be cut short (PlayerSkillSystem.cpp's holdLeavesLoop/holdSkipsLoop) when the player
	releases early, and hasReleasedHold never reaches the wire, so the only way to know a stage's
	real length is to have actually watched it happen. Tracked across frames by observing
	iComboStage/iActionStartTick edges: m_iChargeGaugeStageStartTick is the tick the CURRENTLY
	tracked stage began, and m_fChargeGaugeElapsedBeforeCurrentStageMs accumulates every REAL
	elapsed stage that came before it in this same skill-use instance. INVALID_SKILL_ID means "not
	currently tracking a charge" -- the next observed HOLD charge starts fresh from there.
	m_bChargeGaugeCancelled latches true the instant a stage-advance is observed to have been
	shorter than its authored duration (or skipped a whole stage) -- proof the hold was released
	early -- and keeps the gauge hidden for the rest of this same skill-use instance, matching the
	real screen behavior (the charge visual cancels immediately, it doesn't keep tracking toward a
	now-moot full-charge target). */
	LostArk::Shared::SKILL_ID m_iChargeGaugeTrackedSkillId =
		LostArk::Shared::INVALID_SKILL_ID;
	std::uint8_t m_iChargeGaugeTrackedComboStage = 0;
	std::uint32_t m_iChargeGaugeStageStartTick = 0;
	f32_t m_fChargeGaugeElapsedBeforeCurrentStageMs = 0.f;
	bool_t m_bChargeGaugeCancelled = false;
	/* RenderQuickSlot edge-detects "skill just used" per Q..V slot as a ready-to-not-ready
	transition. iCooldownEndTick itself can't be compared directly across frames: for a ready
	skill CombatHUDViewModel defaults it to the current (ever-increasing) serverTick rather than
	a fixed sentinel, so a raw "did it grow" check fires every single frame. Index order matches
	RenderQuickSlot's own INPUT_SLOTS (Q W E R A S D F T V). */
	bool_t m_bPreviousQuickSlotReady[10] =
		{ true, true, true, true, true, true, true, true, true, true };
	/* RenderLanceMasterIdentityGauge edge-detects each of the 3 identity segments reaching 100 to
	trigger the real extracted gauge0/1/2 highLightMc "burn" flourish (Lance_Id_GaugeBurn0/1/2)
	exactly once per fill, not every frame it stays full. */
	bool_t m_bLanceGaugeSegmentWasFull[3] = { false, false, false };
	/* Real time (ImGui::GetTime()) each segment's burn ignite flourish started, so RenderLance-
	MasterIdentityGauge can swap Lance_Id_GaugeBurn -> Lance_Id_GaugeBurnLoop once the one-shot
	ignite has actually finished playing, instead of restarting the dim->bright ignite clip on
	every loop iteration. Negative = not currently igniting/looping. */
	f64_t m_dLanceGaugeIgniteStartSeconds[3] = { -1.0, -1.0, -1.0 };
	bool_t m_bLanceGaugeLoopStarted[3] = { false, false, false };
	/* RenderDamageNumbers' own client-side spawn record for one DAMAGE_EVENT, since
	CCombatHUDViewModel only retains the server-authoritative hit data (no spawn time / animation
	state -- that is presentation-only and does not belong on the ViewModel). */
	struct FLOATING_DAMAGE_NUMBER
	{
		f64_t dSpawnSeconds = 0.0;
		float3_t vWorldPosition = {};
		uint32_t iAmount = 0;
		bool_t isOutgoing = false;
	};
	vector<FLOATING_DAMAGE_NUMBER> m_FloatingDamageNumbers;
	/* RenderBossHealthBar's own edge-detect state, matching two real effects confirmed from the
	decompiled targetstatus_loc_int.gfx (ark.controls.ProgressMultiTrack / Progress):
	1) ProgressMultiTrack keeps a second "cloneTarget" fill instance with a colorTransform that
	   forces it fully white, cross-fading with the real-color fill whenever a bar segment ticks
	   over (hpCount decreases) instead of hard-cutting back to 100%.
	2) Progress::updateMark positions a "mark" clip at the fill's own edge; the real symbol (732)
	   is a 39-frame animated additive glow (scaleX/Y grows ~1.0->1.5/3.0, alpha fades ~256->92)
	   played on every position update, not a static line -- approximated procedurally (no source
	   art was traced for character 732) as a fading additive glow at the edge on every HP drop. */
	string m_strPreviousBossArchetypeId;
	uint32_t m_iPreviousBossBarsRemaining = 0u;
	f64_t m_dBossBarTickFlashStartSeconds = -1.0;
	uint32_t m_iPreviousBossCurrentHp = 0u;
	f64_t m_dBossHitGlowStartSeconds = -1.0;
	f32_t m_fBossHitGlowFillRatio = 0.f;
	/* Get_DamageEvents() is a rolling buffer that keeps growing (trimmed only once past 128
	entries) -- this is RenderDamageNumbers' read cursor so each event spawns exactly one floating
	number. Apply_DamageEvents batches all events from one snapshot under that snapshot's single
	serverTick and ticks are strictly increasing, so "iServerTick above the last one we spawned
	from" is correct even across trims (old, already-spawned batches simply age out of the buffer). */
	uint32_t m_iLastRenderedDamageServerTick = 0u;
	/* The Lobby's animated title-screen backdrop (Data/UI/Lobby/Lobby_Layout.json), drawn
	behind everything else instead of a flat clear color. Release-safe, like the HUD view. */
	unique_ptr<CHUDRuntimeView> m_pLobbyBackgroundView = { nullptr };
	/* Not _DEBUG-gated: K opens the skill window during real gameplay, in Release too. */
	unique_ptr<CSkillWindowView> m_pSkillWindowView = { nullptr };
	bool_t m_bKDown = false;
	/* Not _DEBUG-gated: I opens the inventory during real gameplay, in Release too. */
	unique_ptr<CInventoryView> m_pInventoryView = { nullptr };
	bool_t m_bIDown = false;
	/* Combat HUD's Item_1..4 quick slots (HUD_Layout.json). Which itemId each one holds is a
	Client-local binding only, set by dragging an item out of CInventoryView and dropping it on
	one of these four rects -- the Server has no concept of a quick slot, only an inventory by
	item ID, so this doesn't survive reconnect and isn't sent anywhere on its own. */
	string m_strItemQuickSlot[4];
	bool_t m_bItemKeyDown[4] = {};
	uint32_t m_iNextUseItemSequence = 1u;
	/* Not _DEBUG-gated: Enter opens the chat input during real gameplay, in Release too. */
	unique_ptr<CChatWindowView> m_pChatWindowView = { nullptr };
	bool_t m_bEnterDown = false;
	bool_t m_bEscapeDown = false;
	/* Not _DEBUG-gated: the party roster overlay draws in Release too, same as the rest of the
	combat HUD. UI-only placeholder roster until a party Shared protocol exists. */
	unique_ptr<CPartyWindowView> m_pPartyWindowView = { nullptr };

#ifdef _DEBUG
	unique_ptr<CMapTool> m_pMapTool = { nullptr };
	unique_ptr<CEffect_Tool> m_pEffectTool = { nullptr };
	unique_ptr<CEffect_Tool_V2> m_pEffectToolV2 = { nullptr };
	unique_ptr<CAnimation_Tool> m_pAnimationTool = { nullptr };
	shared_ptr<CCharacterPreviewPanel> m_pCharacterPreviewPanel = { nullptr };
	unique_ptr<CHUDLayoutTool> m_pHUDLayoutTool = { nullptr };
	unique_ptr<CBalanceTool> m_pBalanceTool = { nullptr };
	unique_ptr<CBossTool> m_pBossTool = { nullptr };
	unique_ptr<CCameraTool> m_pCameraTool = { nullptr };
	bool_t m_bF1Down = false;
	bool_t m_bDeveloperToolsVisible = false;
	bool_t m_bProfilerVisible = false;
	bool_t m_bRenderQualityDraftInitialized = false;
	array<bool_t, static_cast<size_t>(DEBUG_TOOL::COUNT)>
		m_DebugToolVisible = {};
	/* Visibility is deliberately independent from focus. At most one open tool
	   owns raw world-viewport input, while every visible tool may keep rendering
	   and advancing its non-input document state. */
	DEBUG_TOOL m_eDebugInputOwner = DEBUG_TOOL::NONE;
	DEBUG_TOOL m_eDebugWindowFocusPending = DEBUG_TOOL::NONE;
	vector<DEBUG_RESOURCE_FILE> m_DebugResourceFiles;
	array<char_t, 192> m_DebugResourceSearch = {};
	bool_t m_bDebugResourceScanAttempted = false;
	size_t m_iSelectedDebugResourceFile =
		static_cast<size_t>(-1);
	string m_strDebugResourceStatus =
		"Resource Files has not been indexed yet.";
	string m_strServerArenaActiveStatus =
		"Enter the Server-approved Valtan Arena to inspect Active state.";
	bool_t m_bServerArenaPresetStatusTracking = false;
	uint32_t m_iNextKakulStageTeleportRequestSequence = 1u;
	string m_strKakulStageTeleportStatus =
		"Enter the Server-approved KoukuSaton Arena to inspect SL01-SL05 stages.";
	vector<string> m_CompletePlayPatternIds;
	vector<string> m_CompletePlayPatternLabels;
	/* Stable identity is the selection authority.  UI indices are derived from
	   this value each frame and are never persisted as selection state. */
	string m_strCompletePlayPatternId;
	bool_t m_bCompletePlayPatternLoadAttempted = false;
	bool_t m_bCompletePlayStatusTracking = false;
	string m_strCompletePlayTrackedPatternId;
	string m_strCompletePlayStatus =
		"Select a saved semantic pattern, then submit Complete Play to the Server.";
	RENDER_QUALITY_SETTINGS m_RenderQualityDraft = {};
	SCENE_RENDERING_PROFILE m_SceneRenderingDraft = {};
	string m_strRenderingDraftProfileId;
	string m_strToolStatus =
		"Select a tool. Map authoring targets the current level Area.";
	string m_strRenderingStatus =
		"Rendering profiles are loaded from the published runtime catalog.";
	string m_strProfilerCaptureStatus;
	/* Debug-only give-item panel. Index into CItemCatalog::Get_Items() and a
	local, ever-incrementing request sequence -- the Server answers every
	C2S_DEBUG_GIVE_ITEM with a fresh S2C_INVENTORY_SNAPSHOT, so a duplicate
	sequence would only matter if this ever needed to correlate a specific
	reply, which the replace-in-full display does not. */
	int32_t m_iSelectedDebugItemIndex = 0;
	uint32_t m_iNextDebugGiveItemSequence = 1;
	string m_strDebugItemStatus;
#endif

private:
	static CMainApp* s_pActiveInstance;

public:
	static unique_ptr<CMainApp> Create();
	void Free();
};

NS_END
