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
	/* Optional shared-clip animation set (.wmodel, meshless carrier) attached
	onto the body model at admission; empty when the class ships none. */
	std::string animationSetModel;
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

enum class BOSS_COMBAT_OBJECT_ACTIVE_EFFECT_KIND : uint8_t
{
	EFFECT_V1,
	EFFECT_V2_GROUP
};

struct BOSS_COMBAT_OBJECT_EFFECT_V2_GROUP final
{
	std::string groupId;
	f32_t playbackRate = 1.f;
	/* Product presentation may be independent of Server damage.  When this is
	   false, visualHitMs/serverHitId stay empty defaults and only the reusable
	   group clock is consumed.  When true, both fields were present and passed
	   the exact visual-to-Server hit synchronization contract. */
	bool_t bHasHitSync = false;
	uint32_t visualHitMs = 0u;
	std::string serverHitId;
};

struct BOSS_COMBAT_OBJECT_VISUAL_ENTRY final
{
	std::string combatObjectArchetypeId;
	std::string clientVisualId;
	/* effectAssetId remains the stable V1 authoring/editor reference consumed by
	   All Effects and Composition.  An optional effectV2Group overrides only the
	   spawned Product presentation; the runtime never plays both lanes. */
	BOSS_COMBAT_OBJECT_ACTIVE_EFFECT_KIND activeEffectKind =
		BOSS_COMBAT_OBJECT_ACTIVE_EFFECT_KIND::EFFECT_V1;
	std::string effectAssetId;
	BOSS_COMBAT_OBJECT_EFFECT_V2_GROUP effectV2Group;
	/* Optional natural-lifetime Effect spawned for each reliable HIT_PULSE.
	   The Server sends only the stable combat-object/hit identity and pose;
	   presentation assets remain a typed Client catalog concern. */
	std::string hitEffectAssetId;
	// Presentation scale relative to the authoritative combat-object root.
	float3_t worldScale = { 1.f, 1.f, 1.f };

	float4x4_t Make_WorldRoot(const float3_t& position, const f32_t yawDegrees) const
	{
		float4x4_t root{};
		DirectX::XMStoreFloat4x4(&root,
			DirectX::XMMatrixScaling(worldScale.x, worldScale.y, worldScale.z) *
			DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(yawDegrees)) *
			DirectX::XMMatrixTranslation(position.x, position.y, position.z));
		return root;
	}
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
	f32_t presentationScale = {};
	// Asset-unit conversion happens before the actor presentation transform.
	f32_t bodyModelPreScale = {};
	f32_t weaponModelPreScale = {};
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
	/* Optional full-screen cutin flipbook fired with the strike action: the
	source Esther_Skill_<Name> Bink movie cooked to <framePrefix>_NNN.dds
	(NNN = 000..frameCount-1) at the layout reference resolution, played once at
	fps, starting delayMs after the strike clip (the summon spawn) begins. The
	delay is the source EFTable_EpicSkill SkillDecoDelayTime -- measured from the
	caster's call -- minus the Server's 1000ms summon landing delay, floored at 0.
	frameCount 0 means this NPC has no cutin. */
	struct CUTIN_MOVIE final
	{
		std::string framePrefix;
		std::uint32_t frameCount = 0;
		f32_t fps = 0.f;
		std::uint32_t delayMs = 0;
	};
	CUTIN_MOVIE cutinMovie;
	/* Optional pinned shader family. "esther" renders with the dedicated
	esther NPC shader instead of the shared animated-mesh shader; empty keeps
	the shared one. Unknown values are rejected at parse. */
	std::string shaderProfile;
};

struct MONSTER_ACTOR_ENTRY final
{
	struct ATTACK_PRESENTATION final
	{
		std::string clip;
		f32_t playbackRate = 1.f;
	};

	struct PRESENTATION_CLIPS final
	{
		std::string idle;
		std::string chase;
		std::string hit;
		std::string dead;
	};

	std::string archetypeId;
	std::string clientPresentationId;
	std::string modelAssetId;
	f32_t modelScale = 1.f;
	f32_t modelYawDegrees = 0.f;
	std::vector<ATTACK_PRESENTATION> attackPresentations;
	f32_t hitDurationSeconds = 0.f;
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
	static const MONSTER_ACTOR_ENTRY* Find_Monster(
		std::string_view archetypeId);
	static const std::vector<MONSTER_ACTOR_ENTRY>& Get_Monsters();
	static const std::string& Get_Status();
};

NS_END
