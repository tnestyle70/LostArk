#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "AnimationSkillBindingDocument.h"
#include "AnimationEffectCueDocument.h"
#include "CharacterPreviewPanel.h"
#include "EncounterPatternReference.h"
#include "ValtanPatternPreviewDocument.h"
#include "ValtanPatternTree.h"

#include <filesystem>
#include <unordered_set>

NS_BEGIN(Engine)
class CModel;
NS_END

NS_BEGIN(Client)

class CCharacter;

class CAnimation_Tool final
{
private:
	/* One exact occurrence on the admitted split Valtan gameplay/presentation
	   wall-clock. Source time remains independent: sourceStart/playMs/playRate
	   select the pose, while authoringWallMs decides when the Server stage
	   advances. */
	struct VALTAN_PATTERN_MASTER_PLAY_ITEM final
	{
		std::string strPatternId;
		std::string strPatternDisplayName;
		std::string strStageId;
		std::string strSequenceRole;
		std::string strActionId;
		std::string strClipOccurrenceId;
		std::string strClipName;
		uint32_t iSourceStartMs = 0u;
		uint32_t iPlayMs = 0u;
		uint32_t iAuthoringWallMs = 0u;
		uint32_t iTimelineStartMs = 0u;
		uint32_t iStageTimelineStartMs = 0u;
		uint32_t iOccurrenceNumber = 0u;
		uint32_t iOccurrenceCount = 0u;
		f32_t fPlayRate = 1.f;
		bool_t bRepeatUntilStageEnd = false;
	};

	/* Window kinds own a start..end span; point kinds fire on one instant and
	carry a payload naming the cue or particle. Everything before SOUND is a
	window (see Is_Window). Keeping SOUND and EFFECT apart rather than folding
	them into one payload kind is deliberate: different subsystems fire them. */
	enum class EVENT_KIND
	{
		/* Window kinds first; Is_Window relies on the ordering. */
		HIT,
		CANCEL,
		SUPERARMOR,
		INVULN,
		MOVE,
		COUNTER,
		/* Point kinds. */
		SOUND,
		EFFECT,
		SHAKE,
		STAGE,
		END,
	};

	enum class EFFECT_REFERENCE_KIND
	{
		NONE,
		SOURCE_REFERENCE,
		EFFECT_ASSET_ID,
	};

	/* Extracted combat values shared by an authored HIT and a reference row, so
	stamping is a copy rather than a field-by-field translation. Distances stay in
	raw game units; whoever consumes them owns the conversion to our world scale. */
	struct HIT_PARAMS
	{
		/* Hits after the first fire every iRepeatMs; the window is the tolerance
		of each hit, not a span the hits are spread across. */
		int32_t iRepeatCount = { 1 };
		int32_t iRepeatMs = {};

		/* Hitstop: hold for iFreezeMs, easing in and out over the blend times. */
		int32_t iFreezeMs = {};
		int32_t iFreezeInMs = {};
		int32_t iFreezeOutMs = {};

		/* Knockback. A negative iPushMs is a pull toward the caster. */
		int32_t iPushMs = {};
		int32_t iPushRange = {};

		/* 0 none, 1 circle/ring, 2 box (iAreaAngle is the width in cm), 3 fan
		(iAreaAngle is the sweep in degrees); iAreaInner is the ring hole. */
		int32_t iAreaType = {};
		int32_t iAreaRange = {};
		int32_t iAreaAngle = {};
		int32_t iAreaHeight = {};
		int32_t iAreaOffsetX = {};
		int32_t iAreaInner = {};
		int32_t iMaxTargets = {};
	};

	struct ANIM_EVENT
	{
		std::string clipName;
		EVENT_KIND eKind = EVENT_KIND::HIT;
		/* Milliseconds from the clip start. Frames are only a view: the tool
		converts with the clip's own tick rate, so a clip re-exported at another
		rate keeps its events where they were authored. */
		int32_t iStartMs = {};
		int32_t iEndMs = {};
		/* Point kinds only: which cue/particle to fire. */
		std::string sPayload;
		EFFECT_REFERENCE_KIND eEffectReferenceKind =
			EFFECT_REFERENCE_KIND::NONE;
		std::string sAnchorSlotId = "root";
		EFFECT_FOLLOW_POLICY eFollowPolicy = EFFECT_FOLLOW_POLICY::FOLLOW;
		EFFECT_ORIENTATION_POLICY eOrientationPolicy =
			EFFECT_ORIENTATION_POLICY::ANCHOR;
		EFFECT_STOP_POLICY eStopPolicy = EFFECT_STOP_POLICY::NATURAL;
		EFFECT_TRANSFORM_DESC EffectLocalTransform{};
		/* HIT only. */
		HIT_PARAMS hit;
		/* Came from Import_Notifies rather than being authored here. Re-importing
		drops these and keeps everything else, so the button stays idempotent
		without discarding hand-made work. */
		bool_t bImported = false;
	};

	/* One row of the .animnotify timeline lifted out of the game's Action table.
	Times are seconds from the clip start, the same clock the tool authors in. */
	struct NOTIFY_ROW
	{
		f32_t fTime = {};
		f32_t fDuration = {};
		EVENT_KIND eKind = EVENT_KIND::EFFECT;
		std::string sAsset;
		/* What the window is for, straight from the game's own comment on the
		notify ("이동캔슬", "[선콤]"). Only the input-timing windows carry one. */
		std::string sLabel;
	};

	/* One playback chain of a skill: the clips in the order the game plays them.
	A skill has several because each tripod build takes a different route, so a
	clip can appear in more than one chain. */
	struct CLIP_SEQ
	{
		int32_t iSkillId = {};
		int32_t iSeqIndex = {};
		std::string name;
		/* How the chain is driven: COMBO advances a step per press, HOLD is a
		charge, ONESHOT and SEQUENCE run to the end on one press. Inferred from
		clip naming by the extractor -- the game data has no such field. */
		std::string sMode;
		std::vector<std::string> clips;
		/* Per-step playback seconds from the source stage's unconditional
		handover notify, aligned with clips; empty when the sidecar is absent
		or misaligned. */
		std::vector<f32_t> cuts;
	};

	/* One step of a chain the animator assembles by hand instead of reading it
	out of .clipseq. The duration carries the same meaning the source cuts do:
	zero plays the clip's native length, shorter cuts it, longer loops it until
	the step is filled. Session-scoped -- nothing here is authoring data yet. */
	struct CUSTOM_CHAIN_STEP
	{
		std::string clipName;
		f32_t fDurationSeconds = {};
	};

	/* A saved chain in Valtan.presentation.debug.json. The target ids are the
	stage this chain is meant for and stay free text on purpose: the pattern may
	not be promoted into Valtan.gameplay.json yet, and the debug file is a
	handoff note rather than a document the publisher joins. */
	struct CUSTOM_CHAIN_ENTRY
	{
		std::string chainId;
		std::string targetPatternId;
		std::string targetStageId;
		std::vector<CUSTOM_CHAIN_STEP> steps;
	};

	/* One hit of a .skilltiming row. v2 files spell these out on "hit" lines; a
	v1 file only has windows, so the rest stays zero. */
	struct SKILL_HIT
	{
		int32_t iTimeMs = {};
		int32_t iWidthMs = {};
		bool_t bTimed = { true };
		std::string sSourceKeys;
		HIT_PARAMS hit;
	};

	struct SKILL_TIMING
	{
		/* iSkillId is the tripod variant; iBaseSkillId is the skill it belongs to
		and is what the clip map is keyed by. */
		int32_t iSkillId = {};
		int32_t iBaseSkillId = {};
		std::string name;
		std::vector<SKILL_HIT> hits;
		/* Per-skill flags from the Skill table, shown as context only. */
		int32_t iSuperArmor = {};
		int32_t iMoveSpeed = {};
	};

	/* Which skill a model clip belongs to, read from the .clipmap the extractor
	builds out of the game's Action table. Clip names are English and the skill
	names are Korean, so without this the two lists cannot be lined up by eye. */
	struct CLIP_INFO
	{
		int32_t iSkillId = {};
		std::string name;
	};

public:
	explicit CAnimation_Tool(
		shared_ptr<CCharacterPreviewPanel> pPreviewPanel);
	~CAnimation_Tool();

	void Update(f32_t fTimeDelta, bool_t bIsActiveTool);
	void Render();

private:
	shared_ptr<Engine::CModel> Resolve_Model() const;
	/* The character owning that model. Its spec names the data files, so the tool
	follows whichever class the level placed instead of assuming one. */
	shared_ptr<CCharacter> Resolve_Character() const;
	/* Points the file paths at the resolved character's asset and drops anything
	loaded for a previous one. */
	bool_t Sync_AssetName();
	void Adopt_AssetName(const std::string& assetName);
	void Render_TargetConflict();
	void Render_Playback(const shared_ptr<Engine::CModel>& pModel);
	void Render_ValtanPatternPreview(const shared_ptr<Engine::CModel>& pModel);
	void Render_ValtanPatternMaster(
		const shared_ptr<Engine::CModel>& pModel);
	void Render_ValtanPatternReferenceWindow(
		const shared_ptr<Engine::CModel>& pModel);
	void Render_ValtanCustomChainWindow(
		const shared_ptr<Engine::CModel>& pModel);
	/* Feeds the hand-built steps through the same preview playlist the source
	   sequences use, so transport, cuts and looping behave identically. */
	bool_t Start_ValtanCustomChainPreview(
		const shared_ptr<Engine::CModel>& pModel);
	std::filesystem::path Get_CustomChainFilePath() const;
	/* One reader for every chain document, so an imported file and a reloaded
	   one are admitted by exactly the same rules. */
	bool_t Parse_CustomChainDocument(
		const std::string& text,
		std::vector<CUSTOM_CHAIN_ENTRY>& Out,
		std::string& strOutError) const;
	bool_t Read_CustomChainDocument(
		const std::filesystem::path& source,
		std::vector<CUSTOM_CHAIN_ENTRY>& Out,
		std::string& strOutError) const;
	bool_t Load_CustomChainLibrary();
	/* Whole-file atomic replace. A rejected write leaves the previous library
	   on disk and in memory so a failed save never costs saved chains. */
	bool_t Save_CustomChainLibrary();
	bool_t Reload_ValtanPatternMaster();
	std::vector<const VALTAN_PATTERN_VIEW*>
		Collect_ValtanPatternMasterPatterns() const;
	bool_t Build_ValtanPatternMasterTimeline(
		const VALTAN_PATTERN_VIEW& Pattern,
		VALTAN_PATTERN_PREVIEW_PATH ePath,
		const shared_ptr<Engine::CModel>& pModel,
		std::vector<VALTAN_PATTERN_MASTER_PLAY_ITEM>& OutPlaylist,
		uint32_t& iOutDurationMs,
		std::string& strOutStatus) const;
	bool_t Start_ValtanPatternMasterPreview(
		const shared_ptr<Engine::CModel>& pModel,
		const VALTAN_PATTERN_VIEW& Pattern,
		VALTAN_PATTERN_PREVIEW_PATH ePath);
	bool_t Activate_ValtanPatternMasterItem(
		const shared_ptr<Engine::CModel>& pModel,
		std::size_t iItem,
		f32_t fLocalWallSeconds);
	bool_t Apply_ValtanPatternMasterPose(
		const shared_ptr<Engine::CModel>& pModel,
		const VALTAN_PATTERN_MASTER_PLAY_ITEM& Item,
		f32_t fLocalWallSeconds) const;
	bool_t Seek_ValtanPatternMasterPreview(
		const shared_ptr<Engine::CModel>& pModel,
		f32_t fTimelineSeconds,
		bool_t bPause);
	void Advance_ValtanPatternMasterPreview(
		const shared_ptr<Engine::CModel>& pModel);
	void Stop_ValtanPatternMasterPreview(
		const shared_ptr<Engine::CModel>& pModel,
		const std::string& status);
	void Reset_ValtanPatternMasterPreviewState(const std::string& status);
	void Update_ValtanPatternMasterHitAreaPreview();
	static const char_t* ValtanPatternMasterPathName(
		VALTAN_PATTERN_PREVIEW_PATH ePath);
	bool_t Start_ValtanSequencePreview(
		const shared_ptr<Engine::CModel>& pModel,
		std::size_t iSequenceIndex);
	bool_t Start_ValtanPatternPreview(
		const shared_ptr<Engine::CModel>& pModel,
		uint32_t iFirstPattern,
		uint32_t iLastPattern);
	bool_t Activate_ValtanPatternPreviewItem(
		const shared_ptr<Engine::CModel>& pModel);
	void Advance_ValtanPatternPreview(
		const shared_ptr<Engine::CModel>& pModel);
	void Stop_ValtanPatternPreview(
		const shared_ptr<Engine::CModel>& pModel,
		const std::string& status);
	void Reset_ValtanPatternPreviewState(const std::string& status);
	/* Mirrors the arena's pattern hit wires during tool playback: joins the
	   playing item's source action to its encounter pattern and hands the
	   boss preview the active stage actionId and stage-local clock. */
	void Update_ValtanPatternHitAreaPreview();
	void Render_ClipChain(const shared_ptr<Engine::CModel>& pModel);
	void Render_NotifyReference(const shared_ptr<Engine::CModel>& pModel);
	void Bind_ReferenceWire(const std::string& sourceKey);
	void Render_HitEvents(const shared_ptr<Engine::CModel>& pModel);
	void Render_HitDetail(ANIM_EVENT& evt);
	/* Wire overlay of the selected and playhead-active HIT areas, drawn on the
	scene character so an authored box/fan/circle can be judged against the
	pose. Reads the same ANIM_EVENT rows the list edits; nothing new is
	stored. */
	void Render_HitAreaWires(const shared_ptr<Engine::CModel>& pModel) const;
	void Render_AnimationList(const shared_ptr<Engine::CModel>& pModel);
	void Consume_EffectTransfer(const shared_ptr<Engine::CModel>& pModel);
	void Render_SkillReference(
		const shared_ptr<Engine::CModel>& pModel,
		bool_t bReadOnly);
	void Render_SkillBindings(
		const shared_ptr<Engine::CModel>& pModel,
		const shared_ptr<CCharacter>& pCharacter);
	void Render_SkillBindingReloadConfirmation(
		const shared_ptr<Engine::CModel>& pModel,
		LostArk::Shared::CHARACTER_CLASS_ID characterClass);

	bool_t Save_Events(const shared_ptr<Engine::CModel>& pModel);
	bool_t Load_Events(const shared_ptr<Engine::CModel>& pModel);
	bool_t Load_EventsFromPath(
		const std::filesystem::path& path,
		const shared_ptr<Engine::CModel>& pModel,
		std::vector<ANIM_EVENT>& outEvents,
		int32_t& outSourceVersion,
		std::string& outStatus) const;
	bool_t Write_EventsToPath(
		const std::filesystem::path& path,
		const std::vector<ANIM_EVENT>& events,
		std::string& outStatus) const;
	bool_t Validate_Events(
		const shared_ptr<Engine::CModel>& pModel,
		const std::vector<ANIM_EVENT>& events,
		std::string& outStatus) const;
	bool_t Events_AreEqual(
		const std::vector<ANIM_EVENT>& left,
		const std::vector<ANIM_EVENT>& right) const;
	void Render_ReloadConfirmation(
		const shared_ptr<Engine::CModel>& pModel);
	bool_t Load_SkillReference();
	bool_t Load_ClipMap();
	bool_t Load_ClipNotify();
	bool_t Load_ClipSeq();
	void Load_ClipCuts();
	bool_t Load_SkillBindings(
		const shared_ptr<Engine::CModel>& pModel,
		LostArk::Shared::CHARACTER_CLASS_ID characterClass);
	bool_t Save_SkillBindings(
		const shared_ptr<Engine::CModel>& pModel,
		const shared_ptr<CCharacter>& pCharacter);
	bool_t Create_SkillBindingDraft(
		const shared_ptr<Engine::CModel>& pModel,
		LostArk::Shared::CHARACTER_CLASS_ID characterClass);
	std::vector<std::string> Collect_ClipNames(
		const shared_ptr<Engine::CModel>& pModel) const;
	ANIMATION_SKILL_BINDING* Find_SkillBinding(
		LostArk::Shared::SKILL_ID skillId);
	bool_t Is_AnyDocumentDirty() const;
	/* Milliseconds of clip that play before iIndex in seq, i.e. where that clip
	starts on the whole-cast clock the .skilltiming rows use. */
	int32_t Get_ChainOffsetMs(const CLIP_SEQ& seq, int32_t iIndex) const;
	/* Replaces this clip's previously imported events with the original notifies
	of the kinds currently ticked. Returns how many were added. */
	int32_t Import_Notifies(const char_t* pClipName, f32_t fTickRate,
		int32_t& iShapedHits);
	const SKILL_TIMING* Find_ReferenceRow(const char_t* pClipName) const;
	int32_t Count_PrecedingChainHits(const char_t* pClipName) const;
	static int32_t Count_DistinctHitNotifies(const std::vector<NOTIFY_ROW>& rows);
	/* Korean skill name owning the clip, or nullptr when it is unmapped. */
	const CLIP_INFO* Find_ClipInfo(const char_t* pClipName) const;
	/* Selects the named clip in the model, or does nothing if it has no such clip. */
	void Select_Clip(const shared_ptr<Engine::CModel>& pModel, const std::string& clipName);
	std::string Get_EventFilePath() const;
	std::string Get_SkillReferencePath() const;
	std::string Get_ClipMapPath() const;
	std::string Get_ClipNotifyPath() const;
	std::string Get_ClipSeqPath() const;

	/* Tick rate of the named clip, falling back to DEFAULT_TICK_RATE when the
	clip is unknown or the asset carries no rate. */
	static f32_t Get_ClipTickRate(const shared_ptr<Engine::CModel>& pModel,
		const std::string& clipName);

	static bool_t Is_Window(EVENT_KIND eKind);
	static const char_t* Kind_Name(EVENT_KIND eKind);
	static const char_t* Area_Name(int32_t iAreaType);
	static int32_t Ms_To_Frame(int32_t iMs, f32_t fTickRate);
	static int32_t Frame_To_Ms(int32_t iFrame, f32_t fTickRate);
	/* When the iTickIndex'th hit of evt lands. */
	static int32_t Get_TickMs(const ANIM_EVENT& evt, int32_t iTickIndex);
	/* Which hit covers iMs, or -1 when the event is not active there. */
	static int32_t Get_ActiveTick(const ANIM_EVENT& evt, int32_t iMs);

private:
	char m_Filter[128]{};
	bool_t m_bLoop = true;
	bool_t m_bShowHitAreas = true;
	VALTAN_PATTERN_TREE_VIEW m_ValtanPatternMasterView;
	std::vector<VALTAN_PATTERN_MASTER_PLAY_ITEM>
		m_ValtanPatternMasterPlaylist;
	bool_t m_bValtanPatternMasterLoadAttempted = false;
	bool_t m_bValtanPatternMasterPlaying = false;
	bool_t m_bValtanPatternMasterPaused = false;
	bool_t m_bShowValtanSourceReferenceWindow = false;
	bool_t m_bShowValtanCustomChainWindow = false;
	std::vector<CUSTOM_CHAIN_STEP> m_CustomChainSteps;
	std::vector<CUSTOM_CHAIN_ENTRY> m_CustomChainLibrary;
	bool_t m_bCustomChainLibraryLoadAttempted = false;
	char m_CustomChainFilter[128]{};
	char m_CustomChainId[64]{};
	char m_CustomChainTargetPatternId[64]{};
	char m_CustomChainTargetStageId[64]{};
	std::string m_strCustomChainStatus;
	/* Preview-only cross-fade between chain steps. CCharacter already blends
	   its clips at 0.12 s; the product Valtan does not blend at all yet, so a
	   sequence judged here reads smoother than the live boss until the boss
	   owner adopts the same value. */
	f32_t m_fPreviewBlendSeconds = 0.12f;
	int32_t m_iValtanPatternMasterSelected = 0;
	std::size_t m_iValtanPatternMasterItem = 0u;
	f32_t m_fValtanPatternMasterItemElapsedSeconds = 0.f;
	uint32_t m_iValtanPatternMasterDurationMs = 0u;
	VALTAN_PATTERN_PREVIEW_PATH m_eValtanPatternMasterPath =
		VALTAN_PATTERN_PREVIEW_PATH::NORMAL;
	std::string m_strValtanPatternMasterStatus;
	std::weak_ptr<Engine::CModel> m_ValtanPatternMasterModel;
	uint64_t m_iValtanPatternMasterTargetGeneration = 0u;
	VALTAN_PATTERN_PREVIEW_DOCUMENT m_ValtanPatternPreviewDocument;
	std::vector<VALTAN_PATTERN_PREVIEW_PLAY_ITEM> m_ValtanPatternPreviewPlaylist;
	bool_t m_bValtanPatternPreviewLoadAttempted = false;
	bool_t m_bValtanPatternPreviewPlaying = false;
	bool_t m_bValtanPatternPreviewPaused = false;
	int32_t m_iValtanPatternPreviewSelected = 0;
	std::size_t m_iValtanPatternPreviewItem = 0u;
	f32_t m_fValtanPatternPreviewElapsedSeconds = 0.f;
	f32_t m_fValtanPatternPreviewItemDurationSeconds = 0.f;
	f32_t m_fValtanPatternPreviewSpeed = 1.f;
	std::string m_strValtanPatternPreviewStatus;
	std::weak_ptr<Engine::CModel> m_ValtanPatternPreviewModel;
	uint64_t m_iValtanPatternPreviewTargetGeneration = 0u;
	char_t m_ValtanPatternFilter[128]{};
	int32_t m_iValtanSequenceSelected = -1;
	bool_t m_bValtanRaidSequencesOnly = false;
	CEncounterPatternReference m_ValtanEncounterReference;
	bool_t m_bValtanEncounterReferenceLoadAttempted = false;
	/* Seconds of finished same-sequence items before the current one, so the
	   encounter stage lookup runs on the whole-pattern clock. */
	f32_t m_fValtanPatternHitTimelineBaseSeconds = 0.f;
	/* Shared with Effect Tool through MainApp. This tool only contributes the
	unsaved Animation document lock to that one preview session. */
	shared_ptr<CCharacterPreviewPanel> m_pPreviewPanel;

	std::vector<ANIM_EVENT> m_Events;
	/* Empty until a character resolves; Sync_AssetName fills it from the spec. */
	std::string m_AssetName;
	std::string m_Status;
	bool_t m_bDirty = false;
	bool_t m_bReloadConfirmationRequested = false;
	bool_t m_bLoadAttempted = false;
	std::string m_PendingAssetName;
	int32_t m_iSelectedEvent = -1;

	/* Buffer backing the payload text field of the selected point event. */
	char m_PayloadEdit[128]{};

	std::vector<SKILL_TIMING> m_SkillRef;
	bool_t m_bRefLoadAttempted = false;
	char m_RefFilter[128]{};
	int32_t m_iRefWireSkillId = 0;
	int32_t m_iRefWireHitIndex = -1;

	std::map<std::string, CLIP_INFO> m_ClipMap;
	bool_t m_bClipMapLoadAttempted = false;

	std::map<std::string, std::vector<NOTIFY_ROW>> m_ClipNotify;
	/* Clip length in seconds, off the .animnotify headers. The game's own value,
	which is what the chain offsets have to be summed from. */
	std::map<std::string, f32_t> m_ClipLength;
	bool_t m_bClipNotifyLoadAttempted = false;

	std::vector<CLIP_SEQ> m_ClipSeqs;
	bool_t m_bClipSeqLoadAttempted = false;
	std::unordered_map<std::string, int32_t> m_ClipChainCounts;
	std::unordered_set<std::string> m_DuplicateBodyClips;
	bool_t m_bDuplicateScanDone = false;

	ANIMATION_SKILL_BINDING_DOCUMENT m_SkillBindingDocument;
	bool_t m_bSkillBindingLoadAttempted = false;
	bool_t m_bSkillBindingDirty = false;
	bool_t m_bSkillBindingReloadConfirmationRequested = false;
	int32_t m_iSelectedSkillBinding = -1;
	int32_t m_iSelectedSkillStage = 0;
	int32_t m_iSelectedSkillClip = 0;
	std::string m_SkillBindingStatus;
	/* Which kinds Import_Notifies takes. Effects alone run to a few thousand
	rows, so being able to pull in just hits and cancels matters. */
	bool_t m_bImportKind[ETOI(EVENT_KIND::END)]{};
	/* Reference times run from the skill's first clip, so a mid-cast clip needs
	its own start subtracted before a stamp lands on the right local time. In
	milliseconds, matching both the reference file and ANIM_EVENT. */
	int32_t m_iCastOffsetMs = {};
};

NS_END
