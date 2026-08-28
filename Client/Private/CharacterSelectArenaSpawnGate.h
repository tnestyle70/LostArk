#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>

namespace Client
{

enum class CHARACTER_SELECT_ARENA_SPAWN_GATE_STATE : std::uint8_t
{
	IDLE,
	PREWARM_PENDING,
	REQUEST_READY,
	REQUEST_SENT,
	PREWARM_FAILED_RETRYABLE,
	PREWARM_TIMED_OUT_RETRYABLE,
	REQUEST_FAILED_RETRYABLE,
	RESPONSE_TIMED_OUT_RETRYABLE
};

/* Keeps the on-demand Valtan presentation preparation separate from the
   authoritative spawn request.  REQUEST_READY may be consumed only once, so
   repeated frame probes cannot duplicate a Server command. */
class CCharacterSelectArenaSpawnGate final
{
public:
	static constexpr std::chrono::seconds PREWARM_TIMEOUT{ 30 };

	bool Begin(const bool bRequiresPrewarm)
	{
		if (Is_Busy())
			return false;
		m_eState = bRequiresPrewarm ?
			CHARACTER_SELECT_ARENA_SPAWN_GATE_STATE::PREWARM_PENDING :
			CHARACTER_SELECT_ARENA_SPAWN_GATE_STATE::REQUEST_READY;
		return true;
	}

	bool Mark_PrewarmReady()
	{
		if (CHARACTER_SELECT_ARENA_SPAWN_GATE_STATE::PREWARM_PENDING !=
			m_eState)
		{
			return false;
		}
		m_eState = CHARACTER_SELECT_ARENA_SPAWN_GATE_STATE::REQUEST_READY;
		return true;
	}

	bool Mark_PrewarmFailed(const bool bTimedOut)
	{
		if (CHARACTER_SELECT_ARENA_SPAWN_GATE_STATE::PREWARM_PENDING !=
				m_eState &&
			CHARACTER_SELECT_ARENA_SPAWN_GATE_STATE::REQUEST_READY != m_eState)
		{
			return false;
		}
		m_eState = bTimedOut ?
			CHARACTER_SELECT_ARENA_SPAWN_GATE_STATE::
				PREWARM_TIMED_OUT_RETRYABLE :
			CHARACTER_SELECT_ARENA_SPAWN_GATE_STATE::
				PREWARM_FAILED_RETRYABLE;
		return true;
	}

	bool Try_ConsumeServerRequest()
	{
		if (CHARACTER_SELECT_ARENA_SPAWN_GATE_STATE::REQUEST_READY != m_eState)
			return false;
		m_eState = CHARACTER_SELECT_ARENA_SPAWN_GATE_STATE::REQUEST_SENT;
		return true;
	}

	bool Mark_RequestFailed()
	{
		if (CHARACTER_SELECT_ARENA_SPAWN_GATE_STATE::REQUEST_SENT != m_eState)
			return false;
		m_eState = CHARACTER_SELECT_ARENA_SPAWN_GATE_STATE::
			REQUEST_FAILED_RETRYABLE;
		return true;
	}

	bool Mark_ResponseTimedOut()
	{
		if (CHARACTER_SELECT_ARENA_SPAWN_GATE_STATE::REQUEST_SENT != m_eState)
			return false;
		m_eState = CHARACTER_SELECT_ARENA_SPAWN_GATE_STATE::
			RESPONSE_TIMED_OUT_RETRYABLE;
		return true;
	}

	void Reset()
	{
		m_eState = CHARACTER_SELECT_ARENA_SPAWN_GATE_STATE::IDLE;
	}

	bool Is_Preparing() const
	{
		return CHARACTER_SELECT_ARENA_SPAWN_GATE_STATE::PREWARM_PENDING ==
			m_eState;
	}

	bool Is_RequestInFlight() const
	{
		return CHARACTER_SELECT_ARENA_SPAWN_GATE_STATE::REQUEST_SENT ==
			m_eState;
	}

	bool Is_Busy() const
	{
		return Is_Preparing() ||
			CHARACTER_SELECT_ARENA_SPAWN_GATE_STATE::REQUEST_READY == m_eState ||
			Is_RequestInFlight();
	}

	bool Can_Retry() const
	{
		switch (m_eState)
		{
		case CHARACTER_SELECT_ARENA_SPAWN_GATE_STATE::
			PREWARM_FAILED_RETRYABLE:
		case CHARACTER_SELECT_ARENA_SPAWN_GATE_STATE::
			PREWARM_TIMED_OUT_RETRYABLE:
		case CHARACTER_SELECT_ARENA_SPAWN_GATE_STATE::
			REQUEST_FAILED_RETRYABLE:
		case CHARACTER_SELECT_ARENA_SPAWN_GATE_STATE::
			RESPONSE_TIMED_OUT_RETRYABLE:
			return true;
		default:
			return false;
		}
	}

	CHARACTER_SELECT_ARENA_SPAWN_GATE_STATE Get_State() const
	{
		return m_eState;
	}

private:
	CHARACTER_SELECT_ARENA_SPAWN_GATE_STATE m_eState =
		CHARACTER_SELECT_ARENA_SPAWN_GATE_STATE::IDLE;
};

}
