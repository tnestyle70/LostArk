#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <map>
#include <memory>
#include <random>
#include <string>
#include <vector>

NS_BEGIN(Client)

class CCharacter;

/* Retail floating status word over a character's head -- the word a status effect
puts on screen ("gongpo" = fear, "gijeol" = stun, ...).

Source of the presentation, traced from the shipped client:
  * Movie   damagetext.gfx (package OVSG0AHW8W2OD62YW96I6C.upk), class
            ark.ui.damageText.AdvancedDamageText (sprite 269). Its `type` picks
            one of ani0..ani5: 0 normal, 1 critical, 2 nullified, 3 status,
            4 dot, 5 (delegates to ani4). This is a different class from
            DamageTextCBT2, which is what CMainApp::RenderDamageNumbers already
            draws for damage numbers.
  * Motion  not settled by shipped data. EFTable_SkillBuff carries a per-buff
            FontShow of 0..5 that lines up with ani0..ani5, but the table cannot
            say whether it is the type or the type plus one: the 0-based reading
            explains ani5 being reachable, the 1-based one explains FontShow 0
            meaning "no word". The mainstream status words -- burn, bleed, stun,
            fear -- are FontShow 1, and watching the real client settles which
            motion they run: no bounce (ani1) and no growth from nothing
            (ani2/ani3), so ani0. The other motions are not written because no
            other status is replicated yet.
  * Font    DefineEditText 267 authors fontClass "$YG760" (= Font_YG760) at
            fontHeight 640 twips = 32 pt, which is the class constant
            ORIGINAL_TEXT_SIZE = 32, with a DropShadow (45 deg, blur 6.0,
            distance 0, strength 1.5, opaque black).
  * Word    EFTable_GameMsg namespace tip.name.skillbuffdmgfont_<buffId>.
  * Colour  EFTable_SkillBuff.FontColor for that same buff id.
  * Size    the authored 32 pt is 100%. EFTable_SkillBuff.BuffFontScale is a
            per-buff percentage override, but it is 0 (host default) on 33523 of
            the table's 33527 rows.

The colour and the word are therefore data, not code: the caller passes both.
This view owns no gameplay state -- it never decides that a status is active, it
only draws one word per submitted occurrence and forgets it when the motion ends. */
class CStatusEffectTextView final
{
public:
	struct REQUEST
	{
		/* Identity of whoever the word belongs to. One owner shows one word at a
		time; a changed key is a new occurrence and restarts the motion. */
		std::uint32_t				iOwnerEntityId = 0u;
		std::uint32_t				iOccurrenceKey = 0u;
		std::wstring				strWord;
		/* 0xRRGGBB, straight from EFTable_SkillBuff.FontColor. */
		std::uint32_t				iColorRgb = 0xFFFFFFu;
		std::weak_ptr<CCharacter>	pAnchor;
	};

public:
	/* Ignored when the owner's current key was already spawned, so a state that
	stays true across many snapshots still pops exactly one word. Words do stack
	on one head: each occurrence keeps its own scatter offset, the way several
	statuses landing together read on screen. */
	void Submit(const REQUEST& Request);
	void Update(f32_t fTimeDelta);
	/* Called from the level's Render pass; draws nothing when the anchor is gone
	or projects behind the camera. */
	void Render() const;

private:
	struct OCCURRENCE
	{
		std::wstring				strWord;
		std::uint32_t				iColorRgb = 0xFFFFFFu;
		std::weak_ptr<CCharacter>	pAnchor;
		f32_t						fAgeSeconds = 0.f;
		/* Stage-pixel scatter drawn once when the word spawns, so two words on one
		head do not sit on top of each other and so a repeat of the same word does
		not land in the same place. */
		float2_t					vScatterPx = {};
	};

private:
	vector<OCCURRENCE>						m_Occurrences;
	/* Last key spawned per owner. Bounded by the players in the room. */
	std::map<std::uint32_t, std::uint32_t>	m_SpawnedKeyByOwner;
	std::mt19937							m_Scatter{ std::random_device{}() };
};

NS_END
