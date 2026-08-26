#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Network/PacketType.h"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

NS_BEGIN(Client)

enum class WORLD_PLACEMENT_KIND
{
	PLAYER_SPAWN,
	NPC,
	BOSS,
	TRIGGER_BOX,
	COLLISION_BOX,
	DESTROYABLE,
	END
};

enum class WORLD_TRIGGER_EVENT_KIND
{
	MOVE_PLAYER,
	CHANGE_LEVEL,
	ACTIVATE_SPAWN_GROUP,
	ACTIVATE_ENCOUNTER,
	SET_CONDITION,
	SET_DESTROYABLE_STATE,
	END
};

enum class WORLD_DESTROYABLE_STATE
{
	INTACT,
	FRACTURED,
	DESPAWNED,
	END
};

enum class WORLD_NPC_BEHAVIOR_MODE
{
	STATIONARY,
	PATROL,
	WANDER,
	END
};

enum class WORLD_NPC_ROUTE_MODE
{
	LOOP,
	PING_PONG,
	ONCE,
	END
};

enum class WORLD_NPC_ACTION_SELECTION
{
	SEQUENCE,
	WEIGHTED,
	END
};

struct WORLD_NPC_WAYPOINT
{
	std::string waypointId;
	float3_t position = {};
	uint32_t waitMs = 0;
	std::optional<f32_t> lookYawDegrees;
};

struct WORLD_NPC_ACTION
{
	std::string actionId;
	std::string clipName;
	bool_t loop = false;
	uint32_t durationMs = 0;
	uint32_t waitAfterMs = 0;
	uint32_t weight = 1;
	f32_t playbackRate = 1.f;
	f32_t blendSeconds = 0.15f;
};

struct WORLD_NPC_BEHAVIOR
{
	WORLD_NPC_BEHAVIOR_MODE eMode = WORLD_NPC_BEHAVIOR_MODE::STATIONARY;
	WORLD_NPC_ROUTE_MODE eRouteMode = WORLD_NPC_ROUTE_MODE::LOOP;
	WORLD_NPC_ACTION_SELECTION eActionSelection =
		WORLD_NPC_ACTION_SELECTION::SEQUENCE;
	std::string walkClip;
	f32_t moveSpeed = 1.5f;
	f32_t wanderRadius = 0.f;
	uint32_t randomSeed = 1;
	uint32_t startDelayMs = 0;
	uint32_t idleMinMs = 1000;
	uint32_t idleMaxMs = 3000;
	std::string lookTargetPlacementId;
	std::vector<WORLD_NPC_WAYPOINT> waypoints;
	std::vector<WORLD_NPC_ACTION> actions;
};

struct WORLD_TRIGGER_EVENT
{
	WORLD_TRIGGER_EVENT_KIND eKind = WORLD_TRIGGER_EVENT_KIND::MOVE_PLAYER;
	float3_t targetPosition = {};
	f32_t durationSeconds = 0.8f;
	f32_t arcHeight = 0.f;
	LostArk::Shared::WORLD_ID eTargetWorldId =
		LostArk::Shared::WORLD_ID::END;
	std::string targetId;
	bool_t conditionValue = false;
	WORLD_DESTROYABLE_STATE eDestroyableState = WORLD_DESTROYABLE_STATE::FRACTURED;
};

struct WORLD_GAMEPLAY_PLACEMENT
{
	std::string placementId;
	WORLD_PLACEMENT_KIND eKind = WORLD_PLACEMENT_KIND::END;
	std::string archetypeId;
	std::string encounterId;
	std::string npcIdleClip;
	std::optional<WORLD_NPC_BEHAVIOR> npcBehavior;
	float3_t position = {};
	f32_t yawDegrees = {};
	bool_t isEnabled = true;
	float3_t halfExtents = float3_t(1.f, 1.f, 1.f);
	bool_t isTriggerOnce = true;
	std::vector<WORLD_TRIGGER_EVENT> triggerEvents;
	uint64_t deployRuntimePlacementId = 0;
	WORLD_DESTROYABLE_STATE eInitialState = WORLD_DESTROYABLE_STATE::INTACT;
};

class CWorldGameplayDocument final
{
public:
	static constexpr uint32_t MAX_PLACEMENT_COUNT = 4096;

	bool_t Load(
		const std::filesystem::path& path,
		const std::string& expectedAreaId,
		std::string& outStatus);
	bool_t Save(
		const std::filesystem::path& path,
		const std::string& areaId,
		std::string& outStatus) const;

	bool_t Add(
		const WORLD_GAMEPLAY_PLACEMENT& placement,
		std::string& outStatus);
	bool_t Remove(const std::string& placementId);
	WORLD_GAMEPLAY_PLACEMENT* Find(const std::string& placementId);
	const WORLD_GAMEPLAY_PLACEMENT* Find(
		const std::string& placementId) const;

	const std::vector<WORLD_GAMEPLAY_PLACEMENT>& Get_Placements() const
	{
		return m_Placements;
	}

	uint32_t Get_Revision() const { return m_iRevision; }
	void Mark_Edited() { ++m_iRevision; }

	static bool_t Is_Valid(
		const WORLD_GAMEPLAY_PLACEMENT& placement);
	static const char_t* Kind_ToString(WORLD_PLACEMENT_KIND kind);
	static bool_t Try_ParseKind(
		const std::string& value,
		WORLD_PLACEMENT_KIND& outKind);
	static const char_t* TriggerEventKind_ToString(WORLD_TRIGGER_EVENT_KIND kind);
	static bool_t Try_ParseTriggerEventKind(const std::string& value,
		WORLD_TRIGGER_EVENT_KIND& outKind);
	static const char_t* WorldId_ToString(
		LostArk::Shared::WORLD_ID worldId);
	static bool_t Try_ParseWorldId(
		const std::string& value,
		LostArk::Shared::WORLD_ID& outWorldId);
	static const char_t* DestroyableState_ToString(WORLD_DESTROYABLE_STATE state);
	static bool_t Try_ParseDestroyableState(const std::string& value,
		WORLD_DESTROYABLE_STATE& outState);
	static const char_t* NpcBehaviorMode_ToString(WORLD_NPC_BEHAVIOR_MODE mode);
	static bool_t Try_ParseNpcBehaviorMode(const std::string& value,
		WORLD_NPC_BEHAVIOR_MODE& outMode);
	static const char_t* NpcRouteMode_ToString(WORLD_NPC_ROUTE_MODE mode);
	static bool_t Try_ParseNpcRouteMode(const std::string& value,
		WORLD_NPC_ROUTE_MODE& outMode);
	static const char_t* NpcActionSelection_ToString(
		WORLD_NPC_ACTION_SELECTION selection);
	static bool_t Try_ParseNpcActionSelection(const std::string& value,
		WORLD_NPC_ACTION_SELECTION& outSelection);
	static bool_t Is_ValidNpcBehavior(const WORLD_NPC_BEHAVIOR& behavior);

private:
	std::vector<WORLD_GAMEPLAY_PLACEMENT> m_Placements;
	uint32_t m_iRevision = 1;
};

NS_END
