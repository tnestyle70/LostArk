#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

NS_BEGIN(Engine)
class CModel;
NS_END

NS_BEGIN(Client)

class CAnimation_Tool final
{
private:
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
		/* Point kinds. */
		SOUND,
		EFFECT,
		SHAKE,
		END,
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

		/* 0 none, 1 box, 2 fan, 3 circle/ring (iAreaInner is the ring hole). */
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
	};

	/* One hit of a .skilltiming row. v2 files spell these out on "hit" lines; a
	v1 file only has windows, so the rest stays zero. */
	struct SKILL_HIT
	{
		int32_t iTimeMs = {};
		int32_t iWidthMs = {};
		bool_t bTimed = { true };
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
	void Render();

private:
	shared_ptr<Engine::CModel> Resolve_Model() const;
	void Render_Playback(const shared_ptr<Engine::CModel>& pModel);
	void Render_ClipChain(const shared_ptr<Engine::CModel>& pModel);
	void Render_HitEvents(const shared_ptr<Engine::CModel>& pModel);
	void Render_HitDetail(ANIM_EVENT& evt);
	void Render_AnimationList(const shared_ptr<Engine::CModel>& pModel);
	void Render_SkillReference(const shared_ptr<Engine::CModel>& pModel);

	bool_t Save_Events();
	bool_t Load_Events(const shared_ptr<Engine::CModel>& pModel);
	bool_t Load_SkillReference();
	bool_t Load_ClipMap();
	bool_t Load_ClipNotify();
	bool_t Load_ClipSeq();
	/* Milliseconds of clip that play before iIndex in seq, i.e. where that clip
	starts on the whole-cast clock the .skilltiming rows use. */
	int32_t Get_ChainOffsetMs(const CLIP_SEQ& seq, int32_t iIndex) const;
	/* Replaces this clip's previously imported events with the original notifies
	of the kinds currently ticked. Returns how many were added. */
	int32_t Import_Notifies(const char_t* pClipName, f32_t fTickRate);
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

	std::vector<ANIM_EVENT> m_Events;
	std::string m_AssetName = "LanceMaster";
	std::string m_Status;
	bool_t m_bDirty = false;
	bool_t m_bLoadAttempted = false;
	int32_t m_iSelectedEvent = -1;

	/* Buffer backing the payload text field of the selected point event. */
	char m_PayloadEdit[128]{};

	std::vector<SKILL_TIMING> m_SkillRef;
	bool_t m_bRefLoadAttempted = false;
	char m_RefFilter[128]{};

	std::map<std::string, CLIP_INFO> m_ClipMap;
	bool_t m_bClipMapLoadAttempted = false;

	std::map<std::string, std::vector<NOTIFY_ROW>> m_ClipNotify;
	/* Clip length in seconds, off the .animnotify headers. The game's own value,
	which is what the chain offsets have to be summed from. */
	std::map<std::string, f32_t> m_ClipLength;
	bool_t m_bClipNotifyLoadAttempted = false;

	std::vector<CLIP_SEQ> m_ClipSeqs;
	bool_t m_bClipSeqLoadAttempted = false;
	/* Which kinds Import_Notifies takes. Effects alone run to a few thousand
	rows, so being able to pull in just hits and cancels matters. */
	bool_t m_bImportKind[ETOI(EVENT_KIND::END)]{};
	/* Reference times run from the skill's first clip, so a mid-cast clip needs
	its own start subtracted before a stamp lands on the right local time. In
	milliseconds, matching both the reference file and ANIM_EVENT. */
	int32_t m_iCastOffsetMs = {};
};

NS_END
