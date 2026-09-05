#pragma once

#include "Client_Defines.h"
#include "GameplayDataRevision.h"
#include "Network/PacketMessages.h"

#include <cstdint>
#include <string>
#include <string_view>

namespace Client
{
	enum class KOUKU_SAYDON_PATTERN_AUDITION_STATE : std::uint8_t
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

	struct KOUKU_SAYDON_PATTERN_AUDITION_SNAPSHOT final
	{
		KOUKU_SAYDON_PATTERN_AUDITION_STATE eState =
			KOUKU_SAYDON_PATTERN_AUDITION_STATE::IDLE;
		LostArk::Shared::KOUKUSAYDON_PATTERN_AUDITION_OPERATION eOperation =
			LostArk::Shared::KOUKUSAYDON_PATTERN_AUDITION_OPERATION::PLAY_SELECTED;
		std::uint32_t iRequestSequence = 0u;
		std::uint64_t iWorldInboundGeneration = 0u;
		std::uint32_t iRoomAuditionEpoch = 0u;
		LostArk::Shared::NET_ENTITY_ID iBossNetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		std::uint32_t iPatternSequence = 0u;
		std::uint32_t iStageIndex = 0u;
		LostArk::Shared::GameplayDataRevision ExpectedGameplayRevision{};
		LostArk::Shared::GameplayDataRevision PinnedGameplayRevision{};
		std::uint32_t iExpectedSourceRevision = 0u;
		std::uint32_t iPinnedSourceRevision = 0u;
		std::string strRequestedPatternId;
		std::string strLivePatternId;
		std::string strStatus;

		[[nodiscard]] bool Is_InFlight() const noexcept
		{
			return KOUKU_SAYDON_PATTERN_AUDITION_STATE::REQUEST_PENDING == eState ||
				KOUKU_SAYDON_PATTERN_AUDITION_STATE::QUEUED == eState ||
				KOUKU_SAYDON_PATTERN_AUDITION_STATE::ACTIVE == eState;
		}

		[[nodiscard]] bool Is_Live(
			std::string_view patternId,
			std::uint32_t localSourceRevision) const noexcept
		{
			return KOUKU_SAYDON_PATTERN_AUDITION_STATE::ACTIVE == eState &&
				!patternId.empty() && strLivePatternId == patternId &&
				0u != localSourceRevision &&
				localSourceRevision == iExpectedSourceRevision &&
				iExpectedSourceRevision == iPinnedSourceRevision;
		}
	};

	/* Sole Client consumer of the KoukuSaydon audition result/lifecycle queues.
	   It never derives Live from a local preview or an ImGui selection. Only an
	   exact Server lifecycle tuple may put one pattern in ACTIVE. */
	class CKoukuSaydonPatternAuditionService final
	{
	public:
		static CKoukuSaydonPatternAuditionService& Get();

		bool Play_Selected(
			std::string_view patternId,
			const LostArk::Shared::GameplayDataRevision& expectedGameplayRevision,
			std::uint32_t expectedSourceRevision,
			std::string& outStatus);
		bool Play_All(
			const LostArk::Shared::GameplayDataRevision& expectedGameplayRevision,
			std::uint32_t expectedSourceRevision,
			std::string& outStatus);
		void Update();
		void Reset(std::string_view reason = {});

		[[nodiscard]] const KOUKU_SAYDON_PATTERN_AUDITION_SNAPSHOT&
			Get_Snapshot() const noexcept
		{
			return m_Snapshot;
		}

	private:
		CKoukuSaydonPatternAuditionService() = default;

		bool Submit(
			LostArk::Shared::KOUKUSAYDON_PATTERN_AUDITION_OPERATION operation,
			std::string_view patternId,
			const LostArk::Shared::GameplayDataRevision& expectedGameplayRevision,
			std::uint32_t expectedSourceRevision,
			std::string& outStatus);
		void Apply_Result(
			const LostArk::Shared::
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_RESULT& result);
		void Apply_Lifecycle(
			const LostArk::Shared::
				S2C_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE& lifecycle);
		void Set_Terminal(
			KOUKU_SAYDON_PATTERN_AUDITION_STATE state,
			std::string status);
		void Advance_RequestSequence();
		[[nodiscard]] std::uint64_t Now_Milliseconds() const noexcept;

	private:
		KOUKU_SAYDON_PATTERN_AUDITION_SNAPSHOT m_Snapshot;
		std::uint32_t m_iNextRequestSequence = 1u;
		std::uint64_t m_iStateStartedAtMilliseconds = 0u;
	};

	const char* Describe_KoukuSaydonPatternAuditionState(
		KOUKU_SAYDON_PATTERN_AUDITION_STATE state) noexcept;
}
