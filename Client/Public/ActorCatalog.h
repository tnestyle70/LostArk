#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Network/PacketType.h"

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <string_view>
#include <vector>

NS_BEGIN(Client)

inline constexpr std::size_t MAX_BOSS_ARMOR_PARTS = 4u;
inline constexpr std::size_t MAX_BOSS_COMBAT_OBJECT_VISUALS = 16u;

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

struct BOSS_ARMOR_PART_ENTRY final
{
	// Stable gameplay identity shared with BossParts balance data.
	std::string partId;
	// One persistent bit in BOSS_COMBAT_SNAPSHOT::iAlivePartMask.
	std::uint32_t stateMask = 0u;
	std::string modelAssetId;
};

struct BOSS_COMBAT_OBJECT_VISUAL_ENTRY final
{
	std::string combatObjectArchetypeId;
	std::string clientVisualId;
	std::string effectAssetId;
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
	/* Skinned armour pieces share the body skeleton. partId/stateMask, rather
	than array position or a prototype tag, join Server state to presentation. */
	std::vector<BOSS_ARMOR_PART_ENTRY> armorParts;
	std::vector<BOSS_COMBAT_OBJECT_VISUAL_ENTRY> combatObjectVisuals;
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
	/* Optional pinned shader family. "esther" renders with the dedicated
	esther NPC shader instead of the shared animated-mesh shader; empty keeps
	the shared one. Unknown values are rejected at parse. */
	std::string shaderProfile;
};

struct MONSTER_ACTOR_ENTRY final
{
	struct PRESENTATION_CLIPS final
	{
		std::string idle;
		std::string chase;
		std::string attack;
		std::string dead;
		/* Walk gait for the disengaged PATROL action: the trip home to the spawn
		anchor and the wander around it. Distinct from chase, which is the battle
		run this monster uses while it has a target. */
		std::string patrol;
		/* Reeling clip for HIT_STAGGER. Empty is legal and means this archetype
		wears super armour: its profile authors no stagger window, so the state
		is never entered and no clip is ever asked for. */
		std::string hit;
		/* One entry per authored swing, matching the Server's attack order. Empty
		means this archetype owns a single swing and `attack` is it, which is how
		an archetype that never authored a list keeps working. */
		struct ATTACK_CLIP final
		{
			std::string clip;
			/* Where in the source clip the swing starts, and how fast it runs.
			The Server's window for the same swing is authored as the clip length
			divided by this rate, so the whole motion lands inside the window
			instead of being cut off partway. */
			uint32_t sourceStartMs = 0;
			f32_t playRate = 1.f;
		};
		std::vector<ATTACK_CLIP> attacks;
	};

	std::string archetypeId;
	std::string clientPresentationId;
	std::string modelAssetId;
	f32_t modelScale = 1.f;
	f32_t modelYawDegrees = 0.f;
	/* Effect V2 document id played once where a player hit lands on this
	monster. Empty means this archetype shows only the hit flash. */
	std::string hitEffectId;
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
	static const BOSS_COMBAT_OBJECT_VISUAL_ENTRY*
		Find_BossCombatObjectVisual(
			std::string_view bossArchetypeId,
			std::string_view combatObjectArchetypeId,
			std::string_view clientVisualId);
	static const NPC_ACTOR_ENTRY* Find_Npc(std::string_view archetypeId);
	static const std::vector<NPC_ACTOR_ENTRY>& Get_Npcs();
	/* Every monster archetype the catalog admitted, in document order. The
	authoring tool lists these instead of carrying its own copy, so adding an
	archetype stays a catalog edit rather than a code edit. */
	static const std::vector<MONSTER_ACTOR_ENTRY>& Get_Monsters();
	static const MONSTER_ACTOR_ENTRY* Find_Monster(
		std::string_view archetypeId);
	static const std::string& Get_Status();
};

NS_END
