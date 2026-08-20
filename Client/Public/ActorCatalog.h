#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Network/PacketType.h"

#include <map>
#include <string>
#include <string_view>
#include <vector>

NS_BEGIN(Client)

struct CHARACTER_ACTOR_ENTRY final
{
	std::string archetypeId;
	LostArk::Shared::CHARACTER_CLASS_ID networkClassId =
		LostArk::Shared::CHARACTER_CLASS_ID::END;
	std::string assetId;
	std::string bodyModel;
	std::vector<std::string> equipmentModels;
	std::vector<std::string> weaponModels;
	std::string animationSetId;
	std::string runtimeStatus;
};

struct BOSS_ACTOR_ENTRY final
{
	struct PRESENTATION_CLIPS final
	{
		std::string idle;
		std::string chase;
		std::string patternWindup;
		std::string patternActive;
		std::string patternRecovery;
		std::string dead;
	};

	std::string archetypeId;
	std::string visualAssetId;
	std::string bodyModel;
	std::string weaponModel;
	/* Skinned armour pieces the boss wears on the body rig. They share the
	body skeleton, so they carry no animation of their own, and the order
	here is the order the parts attach. */
	std::vector<std::string> armorModels;
	std::string animationSetId;
	std::string serverProfileId;
	std::string clientPresentationId;
	std::string presentationStatus;
	PRESENTATION_CLIPS presentationClips;
};

struct NPC_ACTOR_ENTRY final
{
	std::string archetypeId;
	std::string clientPresentationId;
	std::string modelAssetId;
	std::string animationSetId;
	std::string idleClip;
	std::string runtimeStatus;
	/* Optional server action id -> ordered clip chain for NPC entities the
	server drives through snapshot actions (raid Esther summons). Each clip
	plays once and the next starts when it finishes, mirroring the source
	action's stage list. Actions not listed here present with idleClip. */
	std::map<std::string, std::vector<std::string>, std::less<>> actionClips;
	/* Optional screen-cutin visibility window in milliseconds from the strike
	clip's start. The start seeds from the source EpicSkill SkillDecoDelayTime
	minus its 500ms cast offset; the end is a visual-tuning value because the
	source cutin length lives in the undecoded epicskill.gfx timeline. An end
	of 0 shows until the clip chain finishes. */
	std::uint32_t cutinStartMs = 0;
	std::uint32_t cutinEndMs = 0;
};

struct MONSTER_ACTOR_ENTRY final
{
	struct PRESENTATION_CLIPS final
	{
		std::string idle;
		std::string chase;
		std::string attack;
		std::string dead;
	};

	std::string archetypeId;
	std::string clientPresentationId;
	std::string modelAssetId;
	f32_t modelScale = 1.f;
	f32_t modelYawDegrees = 0.f;
	PRESENTATION_CLIPS presentationClips;
	std::string runtimeStatus;
};

class CActorCatalog final
{
public:
	static bool_t Initialize();
	static const CHARACTER_ACTOR_ENTRY* Find_Character(
		LostArk::Shared::CHARACTER_CLASS_ID networkClassId);
	static const BOSS_ACTOR_ENTRY* Find_Boss(std::string_view archetypeId);
	static const NPC_ACTOR_ENTRY* Find_Npc(std::string_view archetypeId);
	static const std::vector<NPC_ACTOR_ENTRY>& Get_Npcs();
	static const MONSTER_ACTOR_ENTRY* Find_Monster(
		std::string_view archetypeId);
	static const std::string& Get_Status();
};

NS_END
