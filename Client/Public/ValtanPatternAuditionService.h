#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "GameplayDataRevision.h"

#include <cstdint>
#include <string>
#include <string_view>

NS_BEGIN(Client)

enum class VALTAN_PATTERN_AUDITION_STATE : uint8_t
{
	IDLE,
	REQUEST_PENDING,
	QUEUED,
	ACTIVE,
	COMPLETED,
	REJECTED,
	ABORTED,
	END
};

struct VALTAN_PATTERN_AUDITION_SNAPSHOT final
{
	VALTAN_PATTERN_AUDITION_STATE eState =
		VALTAN_PATTERN_AUDITION_STATE::IDLE;
	uint32_t iRequestSequence = 0u;
	uint64_t iWorldInboundGeneration = 0u;
	uint32_t iRoomAuditionEpoch = 0u;
	uint32_t iObservedPatternSequence = 0u;
	LostArk::Shared::GameplayDataRevision PinnedDefinitionRevision{};
	bool_t isPresentationRevisionAvailable = false;
	std::string strConsumerId;
	std::string strBossPlacementId;
	std::string strPatternId;
	std::string strStatus;

	[[nodiscard]] bool_t Is_InFlight() const
	{
		return VALTAN_PATTERN_AUDITION_STATE::REQUEST_PENDING == eState ||
			VALTAN_PATTERN_AUDITION_STATE::QUEUED == eState ||
			VALTAN_PATTERN_AUDITION_STATE::ACTIVE == eState;
	}
};

/* The PLAY_PATTERN_ID wire has one result queue. Every authoring tool must go
   through this service so one panel cannot drain another panel's verdict.
   It deliberately reports the Client-local world generation separately from
   the Server roomAuditionEpoch; the two identities are not interchangeable.
   All methods are called from the Client main thread. */
class CValtanPatternAuditionService final
{
public:
	static CValtanPatternAuditionService& Get();

	bool_t Submit(
		std::string_view strConsumerId,
		std::string_view strBossPlacementId,
		std::string_view strPatternId,
		std::string& strOutStatus);
	void Update();

	[[nodiscard]] const VALTAN_PATTERN_AUDITION_SNAPSHOT& Get_Snapshot() const
	{
		return m_Snapshot;
	}

#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	void Harness_Reset();
	void Harness_ObserveBoss(
		bool_t bBossValid,
		std::string_view strPatternId,
		uint32_t iPatternSequence);
#endif

private:
	CValtanPatternAuditionService() = default;
	void Observe_Boss(
		bool_t bBossValid,
		std::string_view strPatternId,
		uint32_t iPatternSequence);
	void Set_Terminal(
		VALTAN_PATTERN_AUDITION_STATE eState,
		std::string strStatus);

	VALTAN_PATTERN_AUDITION_SNAPSHOT m_Snapshot;
	uint32_t m_iNextRequestSequence = 1u;
	uint64_t m_iStateStartedAtMilliseconds = 0u;
	bool_t m_hasAuthoritativeLifecycle = false;
};

const char_t* Describe_ValtanPatternAuditionState(
	VALTAN_PATTERN_AUDITION_STATE eState);

NS_END
