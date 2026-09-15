#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Network/PacketType.h"
#include "BinaryAsset/ModelAssetData.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <string_view>
#include <vector>

NS_BEGIN(Client)

inline constexpr std::size_t MAX_BOSS_ARMOR_PARTS = 4u;
inline constexpr std::size_t MAX_BOSS_COMBAT_OBJECT_VISUALS = 16u;

/* What a native source-character material was authored with, kept past load.
The catalog states these once as JSON, but the character-creation screen moves
some of them while the character is on screen -- skin colour and gloss, the
make-up colours, the decal placement -- and the program reads them packed into
its constant rows, so a screen that changes one has to re-pack from all of
them. The value type matches SourceCharacterMaterial::PARAMETER_VALUES; this
header states it directly rather than pulling the generated packing in. */
struct CHARACTER_MATERIAL_PARAMETERS final
{
	std::string materialName;
	std::string family;
	std::map<std::string, std::array<float, 4>> values;
};

struct CHARACTER_ACTOR_ENTRY final
{
	std::string archetypeId;
	LostArk::Shared::CHARACTER_CLASS_ID networkClassId =
		LostArk::Shared::CHARACTER_CLASS_ID::END;
	std::string assetId;
	// Optional v4 visual multiplier; model import units and gameplay stay separate.
	f32_t presentationScale = 1.f;
	std::string bodyModel;
	std::vector<std::string> equipmentModels;
	std::vector<std::string> weaponModels;
	std::map<std::string, std::vector<Engine::MODEL_MATERIAL_OVERRIDE>, std::less<>> modelMaterialOverrides;
	/* Aligned with modelMaterialOverrides by material name, for the rows a native
	source-character program drives. Empty for every other material. */
	std::map<std::string, std::vector<CHARACTER_MATERIAL_PARAMETERS>, std::less<>>
		modelMaterialParameters;
	/* Shared-clip animation sets (.wmodel carriers) attached onto the body model
	at admission, in declaration order. A class ships one per clip family it
	borrows -- the Esther call, the customizing idle -- and the list is empty
	when it borrows none. */
	std::vector<std::string> animationSetModels;
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

/* One native LookInfo particle attachment; units are metres after source conversion.
   Stable occurrence IDs distinguish simultaneous uses of the same ParticleSystem. */
struct BOSS_DEFAULT_PARTICLE_ENTRY final
{
	std::string occurrenceId;
	std::string effectAssetId;
	std::string boneName;
	float3_t position{};
	float3_t rotationDegrees{};
	float3_t scale{ 1.f, 1.f, 1.f };
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
	/* Zero means this embedded-body presentation has no separate weapon model.
	Only a catalog row whose dedicated Client presentation contract admits that
	shape may use zero; Valtan remains body+weapon exact. */
	f32_t weaponModelPreScale = {};
	/* Pitch/yaw/roll in degrees applied to the weapon model before its scale,
	turning the authored weapon axes onto the socket bone. Zero without a
	weapon. */
	float3_t weaponModelPreRotationDegrees = {};
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
	std::vector<BOSS_DEFAULT_PARTICLE_ENTRY> defaultParticles;
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

/* One rider pose set on a vehicle: the body clips the class plays while seated.
The clip names are the cooked "<armature>_ride_<mode>_*" names on the class body. */
struct VEHICLE_RIDER_ENTRY final
{
	LostArk::Shared::CHARACTER_CLASS_ID characterClass =
		LostArk::Shared::CHARACTER_CLASS_ID::END;
	std::string idleClip;
	std::string runClip;
};

struct VEHICLE_SKILL_RIDER_ENTRY final
{
	LostArk::Shared::CHARACTER_CLASS_ID characterClass =
		LostArk::Shared::CHARACTER_CLASS_ID::END;
	std::vector<std::string> clips;
};

/* An original Particle notify group of one vehicle skill clip, restored as one
V1 Effect document. startMs is relative to that clip's start in the chain. */
struct VEHICLE_SKILL_EFFECT_CUE final
{
	std::uint32_t clipIndex = 0u;
	std::string effectAssetId;
	std::uint32_t startMs = 0u;
	bool_t bStopAtCueEnd = false;
};

/* An original AKEvent of one vehicle skill clip, played from the Vehicle sound
catalog class. */
struct VEHICLE_SKILL_SOUND_CUE final
{
	std::uint32_t clipIndex = 0u;
	std::string event;
	std::uint32_t startMs = 0u;
};

/* One vehicle skill's presentation: the quick slot it sits on and the clip chains
the vehicle and each rider class play back to back while the Server runs it. */
struct VEHICLE_SKILL_ENTRY final
{
	std::uint32_t skillId = 0u;
	std::string inputSlot;
	std::vector<std::string> vehicleClips;
	std::vector<VEHICLE_SKILL_RIDER_ENTRY> riders;
	std::vector<VEHICLE_SKILL_EFFECT_CUE> effectCues;
	std::vector<VEHICLE_SKILL_SOUND_CUE> soundCues;

	const VEHICLE_SKILL_RIDER_ENTRY* Find_Rider(
		const LostArk::Shared::CHARACTER_CLASS_ID characterClass) const
	{
		for (const VEHICLE_SKILL_RIDER_ENTRY& rider : riders)
			if (rider.characterClass == characterClass)
				return &rider;
		return nullptr;
	}
};

/* A rideable vehicle's presentation. vehicleId is the EFTable_Vehicle key the
Server replicates; everything else stays on the Client. */
struct VEHICLE_ACTOR_ENTRY final
{
	std::uint32_t vehicleId = 0u;
	std::string archetypeId;
	std::string modelAssetId;
	f32_t modelPreScale = 0.f;
	std::string seatBone;
	std::string vehicleIdleClip;
	std::string vehicleRunClip;
	std::vector<VEHICLE_RIDER_ENTRY> riders;
	std::vector<VEHICLE_SKILL_ENTRY> skills;
	std::string runtimeStatus;

	const VEHICLE_RIDER_ENTRY* Find_Rider(
		const LostArk::Shared::CHARACTER_CLASS_ID characterClass) const
	{
		for (const VEHICLE_RIDER_ENTRY& rider : riders)
			if (rider.characterClass == characterClass)
				return &rider;
		return nullptr;
	}

	const VEHICLE_SKILL_ENTRY* Find_Skill(const std::uint32_t skillId) const
	{
		for (const VEHICLE_SKILL_ENTRY& skill : skills)
			if (skill.skillId == skillId)
				return &skill;
		return nullptr;
	}

	const VEHICLE_SKILL_ENTRY* Find_SkillBySlot(const std::string_view inputSlot) const
	{
		for (const VEHICLE_SKILL_ENTRY& skill : skills)
			if (skill.inputSlot == inputSlot)
				return &skill;
		return nullptr;
	}
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
	// Exact catalog model ownership supplies the same overrides to product and
	// preview. Empty characterAssetId allows registered non-player model inputs;
	// an explicit player identity must own the model. Failure preserves outDesc.
	static bool_t Build_ModelLoadDescription(std::string_view modelAssetId,
		Engine::MODEL_ASSET_LOAD_DESC& outDesc, std::string& outStatus,
		std::string_view characterAssetId = {});
	// Explicit material owner for a derived static/skinned model. Empty source
	// keeps exact-model lookup; a source requires existing, matching native slots.
	// No material-name search or shader-family substitution. Failure preserves outDesc.
	static bool_t Build_DerivedModelLoadDescription(std::string_view modelAssetId,
		std::string_view materialSourceModelAssetId,
		Engine::MODEL_ASSET_LOAD_DESC& outDesc, std::string& outStatus);
	static const CHARACTER_ACTOR_ENTRY* Find_Character(
		LostArk::Shared::CHARACTER_CLASS_ID networkClassId);
	static const BOSS_ACTOR_ENTRY* Find_Boss(std::string_view archetypeId);
	static const std::vector<BOSS_ACTOR_ENTRY>& Get_Bosses();
	static const BOSS_COMBAT_OBJECT_VISUAL_ENTRY*
		Find_BossCombatObjectVisual(
			std::string_view bossArchetypeId,
			std::string_view combatObjectArchetypeId,
			std::string_view clientVisualId);
	static const NPC_ACTOR_ENTRY* Find_Npc(std::string_view archetypeId);
	static const std::vector<NPC_ACTOR_ENTRY>& Get_Npcs();
	static const VEHICLE_ACTOR_ENTRY* Find_Vehicle(std::uint32_t vehicleId);
	static const VEHICLE_ACTOR_ENTRY* Find_VehicleByArchetype(std::string_view archetypeId);
	static const std::vector<VEHICLE_ACTOR_ENTRY>& Get_Vehicles();
	static const MONSTER_ACTOR_ENTRY* Find_Monster(
		std::string_view archetypeId);
	static const std::vector<MONSTER_ACTOR_ENTRY>& Get_Monsters();
	static const std::string& Get_Status();
};

NS_END
