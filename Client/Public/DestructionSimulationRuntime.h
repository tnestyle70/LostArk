#pragma once

#include "Client_Defines.h"
#include "DestructionSimulationDocument.h"
#include "Engine_Defines.h"

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

NS_BEGIN(Client)

class CDeployPropRuntime;
class CWorldDestructionDocument;

enum class DESTRUCTION_SIMULATION_SCOPE
{
	ALL_DEBRIS,
	SOLO_SELECTED,
	SOLO_FRAGMENT,
	END
};

enum class DESTRUCTION_SIMULATION_ELEMENT_STATE
{
	WAITING,
	ACTIVE,
	EXPIRED,
	FILTERED,
	END
};

/* Shared admission recipe for the project-authored Valtan stone proxies.
   MapTool admits these Resources-relative CModel assets under the stable
   prototype tags, then the preview runtime clones only those prototypes.
   sourceParticleAssetId remains documented separately because these meshes
   are a project-authored stand-in, not the recovered source particle. */
struct DESTRUCTION_SIMULATION_DEBRIS_MODEL_SPEC final
{
	std::wstring prototypeTag;
	std::string assetId;
	f32_t fUniformScale = 1.f;
};

struct DESTRUCTION_SIMULATION_ELEMENT_FRAME final
{
	struct FRAGMENT_FRAME final
	{
		std::string fragmentId;
		std::string modelAssetId;
		uint32_t pieceIndex = 0u;
		DESTRUCTION_SIMULATION_ELEMENT_STATE eState =
			DESTRUCTION_SIMULATION_ELEMENT_STATE::WAITING;
		float3_t vWorldPosition{};
		float4_t vWorldRotationQuaternion = { 0.f, 0.f, 0.f, 1.f };
		float3_t vLinearVelocity{};
		f32_t fNormalizedLife = 0.f;
	};

	std::string elementId;
	uint64_t sourceRuntimePlacementId = 0u;
	DESTRUCTION_SIMULATION_ELEMENT_STATE eState =
		DESTRUCTION_SIMULATION_ELEMENT_STATE::WAITING;
	float3_t vWorldPosition{};
	float4_t vWorldRotationQuaternion = { 0.f, 0.f, 0.f, 1.f };
	float3_t vLinearVelocity{};
	f32_t fNormalizedLife = 0.f;
	std::vector<FRAGMENT_FRAME> Fragments;
};

using DESTRUCTION_SIMULATION_FRAGMENT_FRAME =
	DESTRUCTION_SIMULATION_ELEMENT_FRAME::FRAGMENT_FRAME;

struct DESTRUCTION_SIMULATION_FRAME final
{
	std::string profileId;
	std::string groupId;
	f32_t fSampleTimeSeconds = 0.f;
	f32_t fDurationSeconds = 0.f;
	DESTRUCTION_SIMULATION_SCOPE eScope =
		DESTRUCTION_SIMULATION_SCOPE::ALL_DEBRIS;
	std::string selectedElementId;
	std::string selectedFragmentId;
	std::vector<DESTRUCTION_SIMULATION_ELEMENT_FRAME> Elements;
};

/* Owns the preview-only PhysX actors but never owns or reloads a model.
   Visible meshes remain the CDeployPropRuntime objects. MapTool applies this
   frame's poses through the DeployProp preview seam after PostPhysics. */
class CDestructionSimulationRuntime final
{
private:
	class CPhysicsAdapter;
	struct ELEMENT_RUNTIME;

public:
	CDestructionSimulationRuntime();
	~CDestructionSimulationRuntime();

	CDestructionSimulationRuntime(
		const CDestructionSimulationRuntime&) = delete;
	CDestructionSimulationRuntime& operator=(
		const CDestructionSimulationRuntime&) = delete;

public:
	static constexpr uint32_t PROJECT_AUTHORED_DEBRIS_PIECES_PER_ELEMENT = 12u;
	static constexpr const char_t* PROJECT_AUTHORED_SOURCE_PARTICLE_ID =
		"FX_ITR_02315.Par_G_Fracture_Dust_02_01";
	static const std::vector<DESTRUCTION_SIMULATION_DEBRIS_MODEL_SPEC>&
		Get_ProjectAuthoredDebrisModelSpecs();

	bool_t Stage_Profile(
		const DESTRUCTION_SIMULATION_PROFILE& profile,
		const std::string& selectedGroupId,
		const CWorldDestructionDocument& destructionDocument,
		CDeployPropRuntime& deployRuntime,
		uint32_t levelId,
		std::string& outStatus);
	void Clear();
	bool_t Reset(std::string& outStatus);
	/* selectedElementId carries an element ID for SOLO_SELECTED and an exact
	   `<element>.fragment.NN` ID for SOLO_FRAGMENT. ALL_DEBRIS ignores it. */
	bool_t Set_Scope(
		DESTRUCTION_SIMULATION_SCOPE scope,
		const std::string& selectedElementId,
		std::string& outStatus);
	bool_t Advance_Timeline(f32_t fixedDeltaSeconds, std::string& outStatus);
	bool_t Post_Physics_Update(std::string& outStatus);
	bool_t Notify_Collision(
		std::string_view receiverCollisionId,
		std::string& outStatus);
	bool_t Simulate_PhysicsSteps(
		uint32_t stepCount,
		std::string& outStatus);
	bool_t Set_PhysicsPaused(bool_t isPaused, std::string& outStatus);

public:
	bool_t Is_Staged() const { return m_isStaged; }
	bool_t Is_Finished() const;
	f32_t Get_DurationSeconds() const { return m_Profile.fDurationSeconds; }
	const DESTRUCTION_SIMULATION_FRAME& Get_Frame() const { return m_Frame; }
	const std::string& Get_Status() const { return m_Status; }

private:
	bool_t Validate_Stage(
		const DESTRUCTION_SIMULATION_PROFILE& profile,
		const std::string& selectedGroupId,
		const CWorldDestructionDocument& destructionDocument,
		const CDeployPropRuntime& deployRuntime,
		std::string& outStatus) const;
	bool_t Activate_Element(
		ELEMENT_RUNTIME& runtime,
		std::string& outStatus);
	void Expire_Element(ELEMENT_RUNTIME& runtime);
	void Destroy_Actors();
	void Release_DebrisPreviews();
	void Rebuild_Frame();
	bool_t Is_ElementInScope(const std::string& elementId) const;
	bool_t Is_FragmentInScope(const std::string& fragmentId) const;

private:
	std::unique_ptr<CPhysicsAdapter> m_pPhysics;
	DESTRUCTION_SIMULATION_PROFILE m_Profile;
	std::vector<ELEMENT_RUNTIME> m_Elements;
	DESTRUCTION_SIMULATION_FRAME m_Frame;
	CDeployPropRuntime* m_pDeployRuntime = nullptr;
	uint32_t m_iLevelId = ETOUI(LEVEL::END);
	f32_t m_fSampleTimeSeconds = 0.f;
	DESTRUCTION_SIMULATION_SCOPE m_eScope =
		DESTRUCTION_SIMULATION_SCOPE::ALL_DEBRIS;
	std::string m_SelectedElementId;
	std::string m_SelectedFragmentId;
	std::string m_Status = "Destruction simulation is not staged";
	bool_t m_isStaged = false;
	bool_t m_isPhysicsPaused = true;
};

NS_END
