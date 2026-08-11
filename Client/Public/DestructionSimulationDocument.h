#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "WorldDestructionDocument.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

NS_BEGIN(Client)

class CDeployPropRuntime;

/* Authoring trigger for one debris element. Collision triggers are preview
   input only; the Server encounter binding remains in WorldEvents.json. */
enum class DESTRUCTION_SIMULATION_TRIGGER_KIND
{
	IMMEDIATE,
	TIMELINE_TIME,
	COLLISION_IMPACT,
	END
};

struct DESTRUCTION_SIMULATION_TRIGGER final
{
	DESTRUCTION_SIMULATION_TRIGGER_KIND eKind =
		DESTRUCTION_SIMULATION_TRIGGER_KIND::IMMEDIATE;
	f32_t fTimeSeconds = 0.f;
	std::string receiverCollisionId;
};

/* One visible DeployProp participating in a tool-only physics audition.
   suppressionAliasPlacementIds are stable, suppress-only source placements:
   they disappear with this emitter but never create duplicate debris bodies.
   Direction is world-space and unit length; the runtime converts direction *
   speed to PhysX linear velocity. gravityScale is an authoring multiplier
   implemented by the physics facade, not a Server gameplay value. */
struct DESTRUCTION_SIMULATION_ELEMENT final
{
	std::string elementId;
	uint64_t sourceRuntimePlacementId = 0u;
	std::vector<uint64_t> suppressionAliasPlacementIds;
	float3_t vSpawnOffset{};
	float3_t vDirection = { 0.f, 1.f, 0.f };
	f32_t fSpeedMetersPerSecond = 8.f;
	f32_t fGravityScale = 1.f;
	f32_t fLifetimeSeconds = 4.f;
	DESTRUCTION_SIMULATION_TRIGGER Trigger{};
};

/* A complete destruction audition, analogous to one complete Effect.
   previewGround* creates an editor-only support box centred below this
   profile's DeployProp group. It never becomes Server collision data. */
struct DESTRUCTION_SIMULATION_PROFILE final
{
	std::string profileId;
	std::string groupId;
	f32_t fDurationSeconds = 5.f;
	bool_t isPreviewGroundEnabled = true;
	f32_t fPreviewGroundHeight = 0.f;
	float2_t vPreviewGroundHalfExtents = { 20.f, 20.f };
	std::vector<DESTRUCTION_SIMULATION_ELEMENT> Elements;
};

/* Strict authoring-only document. It is intentionally outside the product
   destruction publisher gate: MapTool can tune debris before Server
   replication is admitted. */
class CDestructionSimulationDocument final
{
public:
	static constexpr uint32_t MAX_PROFILE_COUNT = 128u;
	static constexpr uint32_t MAX_ELEMENT_COUNT = 256u;
	static constexpr f32_t MAX_DURATION_SECONDS = 60.f;
	static constexpr f32_t MAX_SPEED_METERS_PER_SECOND = 250.f;
	static constexpr f32_t MAX_GRAVITY_SCALE = 10.f;
	static constexpr f32_t MAX_ABSOLUTE_OFFSET = 1000.f;
	static constexpr f32_t MAX_GROUND_HALF_EXTENT = 1000.f;

public:
	bool_t Load(
		const std::filesystem::path& path,
		const std::string& expectedAreaId,
		std::string& outStatus);
	bool_t Save(
		const std::filesystem::path& path,
		const std::string& areaId,
		std::string& outStatus) const;
	/* Commits WorldEvents and its debris simulation as one disk transaction.
	   Both documents are staged and cross-validated before either canonical
	   path changes. If the second commit fails, the first file is restored
	   byte-for-byte from its rollback sidecar. */
	static bool_t Save_AuthoringPair(
		const CWorldDestructionDocument& destructionDocument,
		const std::filesystem::path& destructionPath,
		const CDestructionSimulationDocument& simulationDocument,
		const std::filesystem::path& simulationPath,
		const std::string& areaId,
		const std::string& encounterId,
		std::string& outStatus);
	void Reset_Empty();
	void Clear();

public:
	bool_t Add_Profile(
		const DESTRUCTION_SIMULATION_PROFILE& profile,
		std::string& outStatus);
	bool_t Update_Profile(
		const DESTRUCTION_SIMULATION_PROFILE& profile,
		std::string& outStatus);
	bool_t Remove_Profile(
		const std::string& profileId,
		std::string& outStatus);
	bool_t Remove_ProfilesForGroup(
		const std::string& groupId,
		std::string& outStatus);
	bool_t Add_Element(
		const std::string& profileId,
		const DESTRUCTION_SIMULATION_ELEMENT& element,
		std::string& outStatus);
	bool_t Update_Element(
		const std::string& profileId,
		const DESTRUCTION_SIMULATION_ELEMENT& element,
		std::string& outStatus);
	bool_t Remove_Element(
		const std::string& profileId,
		const std::string& elementId,
		std::string& outStatus);

	/* Builds a useful first audition directly from the selected destruction
	   group. Every group member becomes one element, directions fan out from
	   the group centre, and the preview ground is fitted to loaded bounds. */
	static bool_t Create_DefaultForGroup(
		const DESTRUCTION_GROUP& group,
		const CDeployPropRuntime& deployRuntime,
		DESTRUCTION_SIMULATION_PROFILE& outProfile,
		std::string& outStatus);
	/* Preserves tuned elements and suppress-only aliases, removes stale members,
	   and appends defaults only for members not projected by either source or
	   alias as one document transaction. */
	bool_t Synchronize_Group(
		const DESTRUCTION_GROUP& group,
		const CDeployPropRuntime& deployRuntime,
		std::string& outStatus);
	/* Every complete profile must project each referenced group member exactly
	   once, either as an emitting source or as a suppress-only alias. */
	bool_t Validate_GroupReferences(
		const CWorldDestructionDocument& destructionDocument,
		std::string& outStatus) const;

public:
	bool_t Is_Ready() const { return m_isReady; }
	bool_t Is_Dirty() const { return m_isDirty; }
	void Clear_Dirty() { m_isDirty = false; }
	const std::vector<DESTRUCTION_SIMULATION_PROFILE>& Get_Profiles() const
	{
		return m_Profiles;
	}
	const DESTRUCTION_SIMULATION_PROFILE* Find_Profile(
		const std::string& profileId) const;
	DESTRUCTION_SIMULATION_PROFILE* Find_Profile(
		const std::string& profileId);
	const DESTRUCTION_SIMULATION_ELEMENT* Find_Element(
		const std::string& profileId,
		const std::string& elementId) const;

	static bool_t Is_StableId(const std::string& value);
	static const char_t* TriggerKind_ToString(
		DESTRUCTION_SIMULATION_TRIGGER_KIND kind);
	static bool_t Try_ParseTriggerKind(
		const std::string& value,
		DESTRUCTION_SIMULATION_TRIGGER_KIND& outKind);
	static bool_t Validate_Profile(
		const DESTRUCTION_SIMULATION_PROFILE& profile,
		std::string& outStatus);

private:
	bool_t Validate_Document(std::string& outStatus) const;

private:
	std::vector<DESTRUCTION_SIMULATION_PROFILE> m_Profiles;
	bool_t m_isReady = false;
	bool_t m_isDirty = false;
};

NS_END
