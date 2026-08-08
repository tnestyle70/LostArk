#pragma once

#include "Client_Defines.h"
#include "DestructionSimulationRuntime.h"
#include "Engine_Defines.h"

#include <cstdint>
#include <optional>
#include <string>

NS_BEGIN(Client)

class CDeployPropRuntime;
class CWorldDestructionDocument;

enum class DESTRUCTION_SIMULATION_PLAYBACK_STATE
{
	STOPPED,
	PLAYING,
	PAUSED,
	FINISHED,
	END
};

struct DESTRUCTION_SIMULATION_CONTROLLER_SNAPSHOT final
{
	DESTRUCTION_SIMULATION_PLAYBACK_STATE eState =
		DESTRUCTION_SIMULATION_PLAYBACK_STATE::STOPPED;
	f32_t fSampleTimeSeconds = 0.f;
	f32_t fDurationSeconds = 0.f;
	DESTRUCTION_SIMULATION_SCOPE eScope =
		DESTRUCTION_SIMULATION_SCOPE::ALL_DEBRIS;
	std::string selectedElementId;
	std::string status = "Destruction simulation is not staged";
};

/* UI methods only latch commands. CMapTool::Update consumes them here, so
   ImGui rendering never creates actors or mutates the world directly. */
class CDestructionSimulationController final
{
private:
	struct STAGE_REQUEST final
	{
		DESTRUCTION_SIMULATION_PROFILE profile;
		std::string selectedGroupId;
		const CWorldDestructionDocument* pDestructionDocument = nullptr;
		CDeployPropRuntime* pDeployRuntime = nullptr;
		uint32_t levelId = ETOUI(LEVEL::END);
	};

public:
	static constexpr f32_t FIXED_DELTA_SECONDS = 1.f / 60.f;
	static constexpr f32_t MAX_FRAME_DELTA_SECONDS = 0.25f;
	static constexpr uint32_t MAX_STEPS_PER_UPDATE = 16u;

public:
	void Request_StageProfile(
		const DESTRUCTION_SIMULATION_PROFILE& profile,
		const std::string& selectedGroupId,
		const CWorldDestructionDocument& destructionDocument,
		CDeployPropRuntime& deployRuntime,
		uint32_t levelId);
	void Request_Play();
	void Request_Pause();
	void Request_Reset();
	void Request_SingleStep();
	void Request_Seek(f32_t sampleTimeSeconds);
	void Request_SetScope(
		DESTRUCTION_SIMULATION_SCOPE scope,
		const std::string& selectedElementId = {});
	void Request_Collision(const std::string& receiverCollisionId);

	void Update(f32_t frameDeltaSeconds);
	void Post_Physics_Update();
	void Clear();

public:
	const CDestructionSimulationRuntime& Get_Runtime() const
	{
		return m_Runtime;
	}
	CDestructionSimulationRuntime& Get_Runtime() { return m_Runtime; }
	const DESTRUCTION_SIMULATION_CONTROLLER_SNAPSHOT& Get_Snapshot() const
	{
		return m_Snapshot;
	}
	bool_t Has_PendingCommand() const;

private:
	void Consume_Commands();
	bool_t Reset_Internal();
	bool_t Seek_Internal(f32_t sampleTimeSeconds);
	bool_t Step_Once();
	void Refresh_Snapshot();

private:
	CDestructionSimulationRuntime m_Runtime;
	DESTRUCTION_SIMULATION_CONTROLLER_SNAPSHOT m_Snapshot;
	std::optional<STAGE_REQUEST> m_StageRequest;
	std::optional<f32_t> m_SeekRequest;
	std::optional<DESTRUCTION_SIMULATION_SCOPE> m_ScopeRequest;
	std::string m_ScopeElementId;
	std::string m_CollisionRequest;
	f64_t m_fAccumulatorSeconds = 0.0;
	bool_t m_requestPlay = false;
	bool_t m_requestPause = false;
	bool_t m_requestReset = false;
	bool_t m_requestSingleStep = false;
};

NS_END
