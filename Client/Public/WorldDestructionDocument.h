#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "WorldGameplayDocument.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

NS_BEGIN(Client)

/* Which side of the destruction event blocks navigation.

   The published Valtan navgrid bakes every deploy footprint as walkable, so an
   intact wall has to ADD a blocker and a fractured wall removes it. An arena
   floor is the opposite: it is walkable until it collapses. One boolean cannot
   express both, so the polarity is stored per group. */
enum class DESTRUCTION_NAV_POLARITY
{
	BLOCK_WHILE_INTACT,
	BLOCK_WHILE_FRACTURED,
	END
};

/* What makes a mutation fire inside an authored boss pattern stage.
   STAGE_TIME uses iOffsetMs from the stage start; COLLISION_IMPACT waits for
   the Server to report the boss collider entering the receiver box. */
enum class DESTRUCTION_TRIGGER_KIND
{
	STAGE_ENTER,
	STAGE_TIME,
	STAGE_EXIT,
	COLLISION_IMPACT,
	END
};

/* A set of deploy props that always break together, plus the navigation
   regions they own. memberPlacementIds are .deployplacements runtimePlacementId
   values, never vector indices or prototype tags. */
struct DESTRUCTION_GROUP final
{
	std::string groupId;
	std::vector<uint64_t> memberPlacementIds;
	std::vector<std::string> navigationRegionIds;
	DESTRUCTION_NAV_POLARITY eNavPolarity =
		DESTRUCTION_NAV_POLARITY::BLOCK_WHILE_INTACT;
	WORLD_DESTROYABLE_STATE eInitialState =
		WORLD_DESTROYABLE_STATE::INTACT;
};

/* One authored world change: drive a group to a target state after an optional
   presentation window. The Server owns the commit; iBreakingDurationMs is how
   long BREAKING is shown before collider and navigation flip together. */
struct DESTRUCTION_MUTATION final
{
	std::string mutationId;
	std::string groupId;
	WORLD_DESTROYABLE_STATE eTargetState =
		WORLD_DESTROYABLE_STATE::FRACTURED;
	uint32_t iBreakingDurationMs = 0;
};

/* Where in the encounter a mutation is requested. patternId and stageId are
   stable IDs owned by Data/Encounters; this document never stores clip names,
   durations or damage. */
struct DESTRUCTION_BINDING final
{
	std::string bindingId;
	std::string mutationId;
	std::string patternId;
	std::string stageId;
	DESTRUCTION_TRIGGER_KIND eTriggerKind =
		DESTRUCTION_TRIGGER_KIND::STAGE_TIME;
	uint32_t iOffsetMs = 0;
	std::string receiverCollisionId;
	bool_t isEnabled = false;
};

/* Authoring document for Data/Encounters/<Boss>/<Boss>WorldEvents.json.

   It owns only stable-ID relationships: which walls break together, what the
   navigation polarity is, and which pattern stage requests the change. Deploy
   transforms stay in .deployplacements, navigation cells stay in .navblockers,
   and stage timing stays in the encounter profile. Nothing here is a runtime
   value; the Server decides when a wall actually breaks. */
class CWorldDestructionDocument final
{
public:
	static constexpr uint32_t MAX_GROUP_COUNT = 128u;
	static constexpr uint32_t MAX_MEMBER_COUNT = 256u;
	static constexpr uint32_t MAX_REGION_REF_COUNT = 64u;
	static constexpr uint32_t MAX_MUTATION_COUNT = 256u;
	static constexpr uint32_t MAX_BINDING_COUNT = 256u;
	static constexpr uint32_t MAX_DURATION_MS = 60000u;

public:
	bool_t Load(
		const std::filesystem::path& path,
		const std::string& expectedAreaId,
		const std::string& expectedEncounterId,
		std::string& outStatus);
	bool_t Save(
		const std::filesystem::path& path,
		const std::string& areaId,
		const std::string& encounterId,
		std::string& outStatus) const;
	/* Prepares an empty but valid document so a first Save can create the file
	   for an Area that has no world events yet. */
	void Reset_Empty();
	void Clear();

public:
	bool_t Add_Group(const std::string& groupId, std::string& outStatus);
	bool_t Remove_Group(const std::string& groupId, std::string& outStatus);
	bool_t Add_Member(
		const std::string& groupId,
		uint64_t runtimePlacementId,
		std::string& outStatus);
	bool_t Remove_Member(
		const std::string& groupId,
		uint64_t runtimePlacementId);
	bool_t Set_NavPolarity(
		const std::string& groupId,
		DESTRUCTION_NAV_POLARITY polarity);
	bool_t Set_InitialState(
		const std::string& groupId,
		WORLD_DESTROYABLE_STATE state);
	bool_t Add_NavigationRegion(
		const std::string& groupId,
		const std::string& regionId,
		std::string& outStatus);
	bool_t Remove_NavigationRegion(
		const std::string& groupId,
		const std::string& regionId);

	bool_t Add_Mutation(
		const DESTRUCTION_MUTATION& mutation,
		std::string& outStatus);
	bool_t Remove_Mutation(
		const std::string& mutationId,
		std::string& outStatus);
	bool_t Update_Mutation(
		const DESTRUCTION_MUTATION& mutation,
		std::string& outStatus);

	bool_t Add_Binding(
		const DESTRUCTION_BINDING& binding,
		std::string& outStatus);
	bool_t Remove_Binding(const std::string& bindingId);
	bool_t Update_Binding(
		const DESTRUCTION_BINDING& binding,
		std::string& outStatus);

public:
	bool_t Is_Ready() const { return m_isReady; }
	bool_t Is_Dirty() const { return m_isDirty; }
	void Clear_Dirty() { m_isDirty = false; }

	const std::vector<DESTRUCTION_GROUP>& Get_Groups() const
	{
		return m_Groups;
	}
	const std::vector<DESTRUCTION_MUTATION>& Get_Mutations() const
	{
		return m_Mutations;
	}
	const std::vector<DESTRUCTION_BINDING>& Get_Bindings() const
	{
		return m_Bindings;
	}
	const DESTRUCTION_GROUP* Find_Group(const std::string& groupId) const;
	const DESTRUCTION_MUTATION* Find_Mutation(
		const std::string& mutationId) const;
	const DESTRUCTION_BINDING* Find_Binding(
		const std::string& bindingId) const;
	/* A deploy placement belongs to at most one group; the panel uses this to
	   refuse a second membership instead of silently moving the wall. */
	const DESTRUCTION_GROUP* Find_GroupOfMember(
		uint64_t runtimePlacementId) const;

	static bool_t Is_StableId(const std::string& value);
	static const char_t* NavPolarity_ToString(
		DESTRUCTION_NAV_POLARITY polarity);
	static bool_t Try_ParseNavPolarity(
		const std::string& value,
		DESTRUCTION_NAV_POLARITY& outPolarity);
	static const char_t* TriggerKind_ToString(
		DESTRUCTION_TRIGGER_KIND kind);
	static bool_t Try_ParseTriggerKind(
		const std::string& value,
		DESTRUCTION_TRIGGER_KIND& outKind);

private:
	DESTRUCTION_GROUP* Find_GroupInternal(const std::string& groupId);
	bool_t Validate_Graph(std::string& outStatus) const;

private:
	std::vector<DESTRUCTION_GROUP> m_Groups;
	std::vector<DESTRUCTION_MUTATION> m_Mutations;
	std::vector<DESTRUCTION_BINDING> m_Bindings;
	bool_t m_isReady = false;
	bool_t m_isDirty = false;
};

NS_END
