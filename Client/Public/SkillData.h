#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <string>
#include <unordered_map>
#include <vector>

NS_BEGIN(Client)

/* An input window the game marks on a clip.

The source data is a designer note on an InputTiming notify, hand written and
spelled 56 different ways across the four classes. extract_action_loa.py folds
those onto the tokens below and writes them as "win=" in the .animnotify, so
nothing here has to match Korean -- Client sources compile under the CP949 code
page and stay ASCII.

Two axes: which action the window is about, and whether it buffers a press that
comes early (PRE) or lets the press cut the current clip short (CANCEL). Rows the
extractor could not place stay out of this enum entirely. */
enum class SKILL_WINDOW
{
	MOVE_PRE,
	SKILL_PRE,
	COMBO_PRE,
	MOVE_CANCEL,
	SKILL_CANCEL,
	COMBO_CANCEL,
	DODGE_CANCEL,
	ANY_CANCEL,
	COMBO_INPUT,
	END
};

/* The timing half of a class's extracted data: which clip allows what and when,
and when a skill lands.

One instance per asset name, shared by every character of that class -- the four
files together are a few hundred KB of text and re-parsing them per spawned
player would be pure waste. Load() hands out the cached instance.

Read-only once built. The clip chain itself lives in CCharacter, which reads
<asset>.clipseq; this is the layer that says when a chain may be interrupted. */
class CSkillData final
{
public:
	/* Seconds from the clip's own start, the clock the .animnotify rows use. */
	struct WINDOW
	{
		SKILL_WINDOW eKind = { SKILL_WINDOW::END };
		f32_t fStart = {};
		f32_t fEnd = {};
	};

	struct CLIP
	{
		/* The game's own clip length, which is not always the model's. */
		f32_t fLength = {};
		std::vector<WINDOW> Windows;
	};

	/* Milliseconds on the whole-cast clock, not the clip's -- a hit in the third
	clip of a chain is timed from where the cast began. */
	struct HIT
	{
		int32_t iTimeMs = {};
		int32_t iWidthMs = {};
	};

	struct SKILL
	{
		int32_t iCooldownMs = {};
		std::vector<HIT> Hits;
	};

public:
	/* Never null: a class with no files yields an empty instance, so callers can
	query without checking. */
	static shared_ptr<const CSkillData> Load(const char_t* pAssetName);

	const CLIP* Find_Clip(const char_t* pClipName) const;
	const SKILL* Find_Skill(int32_t iSkillId) const;

	/* Whether fClipTime falls inside a window of that kind on that clip. A window
	of zero length covers only its exact start, which the data does use. */
	bool_t Is_InWindow(const CLIP* pClip, SKILL_WINDOW eKind, f32_t fClipTime) const;

	size_t Get_ClipCount() const {
		return m_Clips.size();
	}
	size_t Get_SkillCount() const {
		return m_Skills.size();
	}

private:
	bool_t Load_Notify(const std::string& strPath);
	bool_t Load_Timing(const std::string& strPath);

private:
	std::unordered_map<std::string, CLIP> m_Clips;
	std::unordered_map<int32_t, SKILL> m_Skills;
};

NS_END
