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
	/* HIT owns a frame window (iStartFrame..iEndFrame) with iHitCount ticks.
	SOUND and EFFECT are point events: iEndFrame is ignored and sPayload names
	the sound cue or particle/effect key to fire at iStartFrame. */
	enum class EVENT_KIND
	{
		HIT,
		SOUND,
		EFFECT,
	};

	struct ANIM_EVENT
	{
		std::string clipName;
		EVENT_KIND eKind = EVENT_KIND::HIT;
		int32_t iStartFrame = {};
		int32_t iEndFrame = {};
		/* Hits fired across the window, spread evenly. 1 means a single hit. */
		int32_t iHitCount = { 1 };
		/* SOUND/EFFECT only: which cue/particle to fire (unused by HIT). */
		std::string sPayload;
	};

	/* One row of the .skilltiming reference exported from the game tables.
	Times are in seconds; the tool turns them into frames with m_fFps so the
	extracted skill timing can be stamped onto whichever clip is open. */
	struct SKILL_TIMING
	{
		int32_t iSkillId = {};
		std::string name;
		std::vector<std::pair<f32_t, f32_t>> hitWindows;
		f32_t fFreeze = {};
		f32_t fPush = {};
		int32_t iMultiHit = {};
		f32_t fInterval = {};
	};

public:
	void Render();

private:
	shared_ptr<Engine::CModel> Resolve_Model() const;
	void Render_Playback(const shared_ptr<Engine::CModel>& pModel);
	void Render_HitEvents(const shared_ptr<Engine::CModel>& pModel);
	void Render_AnimationList(const shared_ptr<Engine::CModel>& pModel);
	void Render_SkillReference(const shared_ptr<Engine::CModel>& pModel);

	bool_t Save_Events();
	bool_t Load_Events();
	bool_t Load_SkillReference();
	std::string Get_EventFilePath() const;
	std::string Get_SkillReferencePath() const;

	static const char_t* Kind_Name(EVENT_KIND eKind);
	static int32_t Get_TickFrame(const ANIM_EVENT& evt, int32_t iTickIndex);
	static int32_t Get_ActiveTick(const ANIM_EVENT& evt, int32_t iFrame);

private:
	char m_Filter[128]{};
	bool_t m_bLoop = true;

	std::vector<ANIM_EVENT> m_Events;
	std::string m_AssetName = "LanceMaster";
	std::string m_Status;
	bool_t m_bDirty = false;
	bool_t m_bLoadAttempted = false;
	int32_t m_iSelectedEvent = -1;

	/* Buffer backing the payload text field of the selected SOUND/EFFECT event. */
	char m_PayloadEdit[128]{};

	/* Skill-timing reference (game-extracted) and the seconds->frames rate. */
	std::vector<SKILL_TIMING> m_SkillRef;
	bool_t m_bRefLoadAttempted = false;
	char m_RefFilter[128]{};
	f32_t m_fFps = { 30.f };
	/* Reference times run from the skill's first clip; a skill that plays several
	clips in sequence needs the current clip's start (in cast frames) subtracted
	before a stamp lands on the right local frame. */
	int32_t m_iCastOffset = {};
};

NS_END
