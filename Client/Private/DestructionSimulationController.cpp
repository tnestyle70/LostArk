#include "DestructionSimulationController.h"

#include "DeployPropRuntime.h"
#include "WorldDestructionDocument.h"

#include <algorithm>
#include <cmath>

void Client::CDestructionSimulationController::Request_StageProfile(
	const DESTRUCTION_SIMULATION_PROFILE& profile,
	const std::string& selectedGroupId,
	const CWorldDestructionDocument& destructionDocument,
	CDeployPropRuntime& deployRuntime,
	const uint32_t levelId)
{
	STAGE_REQUEST request;
	request.profile = profile;
	request.selectedGroupId = selectedGroupId;
	request.pDestructionDocument = &destructionDocument;
	request.pDeployRuntime = &deployRuntime;
	request.levelId = levelId;
	m_StageRequest = std::move(request);
}

void Client::CDestructionSimulationController::Request_Play()
{
	m_requestPlay = true;
}

void Client::CDestructionSimulationController::Request_Pause()
{
	m_requestPause = true;
}

void Client::CDestructionSimulationController::Request_Reset()
{
	m_requestReset = true;
}

void Client::CDestructionSimulationController::Request_SingleStep()
{
	m_requestSingleStep = true;
}

void Client::CDestructionSimulationController::Request_Seek(
	const f32_t sampleTimeSeconds)
{
	m_SeekRequest = sampleTimeSeconds;
}

void Client::CDestructionSimulationController::Request_SetScope(
	const DESTRUCTION_SIMULATION_SCOPE scope,
	const std::string& selectedElementId)
{
	m_ScopeRequest = scope;
	m_ScopeElementId = selectedElementId;
}

void Client::CDestructionSimulationController::Request_Collision(
	const std::string& receiverCollisionId)
{
	m_CollisionRequest = receiverCollisionId;
}

bool_t Client::CDestructionSimulationController::Has_PendingCommand() const
{
	return m_StageRequest.has_value() || m_SeekRequest.has_value() ||
		m_ScopeRequest.has_value() || !m_CollisionRequest.empty() ||
		m_requestPlay || m_requestPause || m_requestReset ||
		m_requestSingleStep;
}

void Client::CDestructionSimulationController::Consume_Commands()
{
	if (m_StageRequest.has_value())
	{
		STAGE_REQUEST request = std::move(*m_StageRequest);
		m_StageRequest.reset();
		std::string status;
		if (nullptr == request.pDestructionDocument ||
			nullptr == request.pDeployRuntime ||
			!m_Runtime.Stage_Profile(
				request.profile,
				request.selectedGroupId,
				*request.pDestructionDocument,
				*request.pDeployRuntime,
				request.levelId,
				status))
		{
			const std::string failureStatus = status.empty() ?
				"Destruction simulation stage request is stale" : status;
			/* Stage-dependent requests are one command transaction. A failed
			   replacement must not resume the previously staged profile or hide
			   the validation error behind a later Play/Scope status. */
			m_Runtime.Clear();
			m_ScopeRequest.reset();
			m_ScopeElementId.clear();
			m_SeekRequest.reset();
			m_CollisionRequest.clear();
			m_requestPlay = false;
			m_requestPause = false;
			m_requestReset = false;
			m_requestSingleStep = false;
			m_fAccumulatorSeconds = 0.0;
			m_Snapshot.status = failureStatus;
			m_Snapshot.eState =
				DESTRUCTION_SIMULATION_PLAYBACK_STATE::STOPPED;
			Refresh_Snapshot();
			return;
		}
		else
		{
			m_Runtime.Set_PhysicsPaused(true, status);
			m_fAccumulatorSeconds = 0.0;
			m_Snapshot.eState =
				DESTRUCTION_SIMULATION_PLAYBACK_STATE::PAUSED;
			m_Snapshot.status = status;
		}
	}

	if (m_ScopeRequest.has_value())
	{
		const DESTRUCTION_SIMULATION_SCOPE scope = *m_ScopeRequest;
		const bool_t resumeAfterScope =
			DESTRUCTION_SIMULATION_PLAYBACK_STATE::PLAYING ==
				m_Snapshot.eState;
		m_ScopeRequest.reset();
		std::string status;
		if (m_Runtime.Set_Scope(scope, m_ScopeElementId, status))
		{
			m_Runtime.Set_PhysicsPaused(!resumeAfterScope, status);
			m_fAccumulatorSeconds = 0.0;
			m_Snapshot.eState = resumeAfterScope ?
				DESTRUCTION_SIMULATION_PLAYBACK_STATE::PLAYING :
				DESTRUCTION_SIMULATION_PLAYBACK_STATE::PAUSED;
		}
		m_Snapshot.status = status;
		m_ScopeElementId.clear();
	}

	if (m_requestReset)
	{
		m_requestReset = false;
		Reset_Internal();
	}

	if (m_SeekRequest.has_value())
	{
		const f32_t sampleTime = *m_SeekRequest;
		m_SeekRequest.reset();
		Seek_Internal(sampleTime);
	}

	if (!m_CollisionRequest.empty())
	{
		std::string status;
		m_Runtime.Notify_Collision(m_CollisionRequest, status);
		m_Snapshot.status = status;
		m_CollisionRequest.clear();
	}

	if (m_requestPause)
	{
		m_requestPause = false;
		m_requestPlay = false;
		std::string status;
		if (m_Runtime.Set_PhysicsPaused(true, status))
		{
			m_Snapshot.eState =
				DESTRUCTION_SIMULATION_PLAYBACK_STATE::PAUSED;
		}
		m_Snapshot.status = status;
	}
	else if (m_requestPlay)
	{
		m_requestPlay = false;
		if (DESTRUCTION_SIMULATION_PLAYBACK_STATE::FINISHED ==
			m_Snapshot.eState)
		{
			Reset_Internal();
		}
		std::string status;
		if (m_Runtime.Set_PhysicsPaused(false, status))
		{
			m_Snapshot.eState =
				DESTRUCTION_SIMULATION_PLAYBACK_STATE::PLAYING;
		}
		m_Snapshot.status = status;
	}

	if (m_requestSingleStep)
	{
		m_requestSingleStep = false;
		if (DESTRUCTION_SIMULATION_PLAYBACK_STATE::PAUSED !=
			m_Snapshot.eState)
		{
			m_Snapshot.status =
				"Single step requires paused destruction playback";
		}
		else if (Step_Once())
		{
			m_Snapshot.status = "Advanced one 1/60 destruction physics step";
			if (m_Runtime.Is_Finished())
				m_Snapshot.eState =
					DESTRUCTION_SIMULATION_PLAYBACK_STATE::FINISHED;
		}
	}
}

void Client::CDestructionSimulationController::Update(
	const f32_t frameDeltaSeconds)
{
	Consume_Commands();
	if (DESTRUCTION_SIMULATION_PLAYBACK_STATE::PLAYING !=
		m_Snapshot.eState)
	{
		/* Paused time never enters the accumulator. Existing sub-step residual is
		   preserved so resume continues from the same fixed-clock phase. */
		Refresh_Snapshot();
		return;
	}
	if (!std::isfinite(frameDeltaSeconds) || frameDeltaSeconds < 0.f)
	{
		m_Snapshot.status = "Destruction playback ignored invalid frame delta";
		Refresh_Snapshot();
		return;
	}

	m_fAccumulatorSeconds += static_cast<f64_t>((std::min)(
		frameDeltaSeconds, MAX_FRAME_DELTA_SECONDS));
	uint32_t steps = 0u;
	while (m_fAccumulatorSeconds + 0.000000001 >=
		static_cast<f64_t>(FIXED_DELTA_SECONDS) &&
		steps < MAX_STEPS_PER_UPDATE && !m_Runtime.Is_Finished())
	{
		if (!Step_Once())
		{
			m_Snapshot.eState =
				DESTRUCTION_SIMULATION_PLAYBACK_STATE::PAUSED;
			break;
		}
		m_fAccumulatorSeconds -= static_cast<f64_t>(FIXED_DELTA_SECONDS);
		++steps;
	}
	if (m_fAccumulatorSeconds < 0.0)
		m_fAccumulatorSeconds = 0.0;
	if (steps == MAX_STEPS_PER_UPDATE &&
		m_fAccumulatorSeconds >= static_cast<f64_t>(FIXED_DELTA_SECONDS))
	{
		m_fAccumulatorSeconds = std::fmod(
			m_fAccumulatorSeconds,
			static_cast<f64_t>(FIXED_DELTA_SECONDS));
		m_Snapshot.status =
			"Destruction playback dropped excess catch-up steps";
	}

	if (m_Runtime.Is_Finished())
	{
		std::string status;
		m_Runtime.Set_PhysicsPaused(true, status);
		m_Snapshot.eState =
			DESTRUCTION_SIMULATION_PLAYBACK_STATE::FINISHED;
		m_fAccumulatorSeconds = 0.0;
		m_Snapshot.status = "Destruction simulation finished";
	}
	Refresh_Snapshot();
}

void Client::CDestructionSimulationController::Post_Physics_Update()
{
	if (!m_Runtime.Is_Staged())
		return;
	std::string status;
	if (!m_Runtime.Post_Physics_Update(status))
		m_Snapshot.status = status;
	Refresh_Snapshot();
}

bool_t Client::CDestructionSimulationController::Step_Once()
{
	std::string status;
	if (!m_Runtime.Advance_Timeline(FIXED_DELTA_SECONDS, status) ||
		!m_Runtime.Simulate_PhysicsSteps(1u, status) ||
		!m_Runtime.Post_Physics_Update(status))
	{
		const std::string failureStatus = status.empty() ?
			"Destruction simulation fixed step failed" : status;
		std::string rollbackStatus;
		const bool_t rolledBack = !m_Runtime.Is_Staged() ||
			m_Runtime.Reset(rollbackStatus);
		m_Snapshot.status = failureStatus;
		if (!rolledBack)
			m_Snapshot.status += "; rollback failed: " + rollbackStatus;
		return false;
	}
	return true;
}

bool_t Client::CDestructionSimulationController::Reset_Internal()
{
	std::string status;
	if (!m_Runtime.Reset(status) ||
		!m_Runtime.Set_PhysicsPaused(true, status))
	{
		m_Snapshot.status = status;
		return false;
	}
	m_fAccumulatorSeconds = 0.0;
	m_Snapshot.eState = DESTRUCTION_SIMULATION_PLAYBACK_STATE::PAUSED;
	m_Snapshot.status = status;
	return true;
}

bool_t Client::CDestructionSimulationController::Seek_Internal(
	const f32_t sampleTimeSeconds)
{
	if (!m_Runtime.Is_Staged() || !std::isfinite(sampleTimeSeconds))
	{
		m_Snapshot.status = "Destruction seek needs a finite staged time";
		return false;
	}
	const f32_t clamped = (std::clamp)(sampleTimeSeconds, 0.f,
		m_Runtime.Get_DurationSeconds());
	const uint32_t targetStep = static_cast<uint32_t>(std::llround(
		static_cast<f64_t>(clamped) /
		static_cast<f64_t>(FIXED_DELTA_SECONDS)));
	if (!Reset_Internal())
		return false;
	for (uint32_t step = 0u; step < targetStep &&
		!m_Runtime.Is_Finished(); ++step)
	{
		if (!Step_Once())
			return false;
	}
	m_fAccumulatorSeconds = 0.0;
	m_Snapshot.eState = m_Runtime.Is_Finished() ?
		DESTRUCTION_SIMULATION_PLAYBACK_STATE::FINISHED :
		DESTRUCTION_SIMULATION_PLAYBACK_STATE::PAUSED;
	m_Snapshot.status = "Deterministically replayed destruction preview to " +
		std::to_string(m_Runtime.Get_Frame().fSampleTimeSeconds) + " s";
	return true;
}

void Client::CDestructionSimulationController::Refresh_Snapshot()
{
	const DESTRUCTION_SIMULATION_FRAME& frame = m_Runtime.Get_Frame();
	m_Snapshot.fSampleTimeSeconds = frame.fSampleTimeSeconds;
	m_Snapshot.fDurationSeconds = frame.fDurationSeconds;
	m_Snapshot.eScope = frame.eScope;
	m_Snapshot.selectedElementId = frame.selectedElementId;
	if (m_Snapshot.status.empty())
		m_Snapshot.status = m_Runtime.Get_Status();
}

void Client::CDestructionSimulationController::Clear()
{
	m_Runtime.Clear();
	m_StageRequest.reset();
	m_SeekRequest.reset();
	m_ScopeRequest.reset();
	m_ScopeElementId.clear();
	m_CollisionRequest.clear();
	m_fAccumulatorSeconds = 0.0;
	m_requestPlay = false;
	m_requestPause = false;
	m_requestReset = false;
	m_requestSingleStep = false;
	m_Snapshot = {};
}
