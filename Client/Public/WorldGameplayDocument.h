#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Network/PacketType.h"

#include <filesystem>
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

private:
	std::vector<WORLD_GAMEPLAY_PLACEMENT> m_Placements;
	uint32_t m_iRevision = 1;
};

NS_END
