#pragma once

#include "Client_Defines.h"
#include "GameplayDataRevision.h"
#include "Network/PacketMessages.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

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
		std::string strBundleId;
		std::string strGateId;
		std::uint32_t iCommonStartTick = 0;
		std::uint32_t iExpectedRunEpoch = 0;
		std::vector<LostArk::Shared::KOUKUSAYDON_BUNDLE_MEMBER_STATE> Members;
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

	struct KOUKU_SAYDON_PATTERN_FLOW_ENTRY final
	{
		std::string strEntryId;
		std::string strPatternId;
		std::string strBundleId;
		std::string strBossPlacementId;
		std::string strBossArchetypeId;
		std::uint32_t iWaitAfterMs = 0u;
	};

	struct KOUKU_SAYDON_PATTERN_FLOW_SNAPSHOT final
	{
		bool bActive = false;
		std::string strGateId;
		std::size_t iEntryIndex = 0u;
		std::size_t iEntryCount = 0u;
		std::string strEntryId;
		std::string strStatus;
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
		bool Play_Bundle(std::string_view bundleId, std::string_view gateId,
			const LostArk::Shared::GameplayDataRevision& revision, std::uint32_t sourceRevision, std::string& status);
		bool Play_Flow(std::string_view gateId,
			const std::vector<KOUKU_SAYDON_PATTERN_FLOW_ENTRY>& entries,
			const LostArk::Shared::GameplayDataRevision& revision,
			std::uint32_t sourceRevision, std::string& status);
		const KOUKU_SAYDON_PATTERN_FLOW_SNAPSHOT& Get_FlowSnapshot() const noexcept { return m_FlowSnapshot; }
		bool Stop(std::string& status);
		bool Restart_Bundle(std::string& status);
		void Update();
		void Reset(std::string_view reason = {});

		/* The live arena boss the next request names. The F1 gate buttons set
		it to the gate boss they raised; despawn returns it to the Gate 1
		Kouku. A request in flight keeps the scope it was sent with. */
		void Set_TargetBoss(
			std::string_view placementId,
			std::string_view archetypeId);
		/* Gate activation blocks new playback until its Server results settle.
		Existing request results and lifecycles continue to be consumed. */
		void Set_TargetTransitionPending(const bool pending) noexcept
		{
			m_bTargetTransitionPending = pending;
		}
		[[nodiscard]] const std::string& Get_TargetBossPlacementId() const noexcept
		{
			return m_strTargetBossPlacementId;
		}
		[[nodiscard]] const std::string& Get_TargetBossArchetypeId() const noexcept
		{
			return m_strTargetBossArchetypeId;
		}

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
			std::string& outStatus, std::string_view bundleId = {}, std::string_view gateId = {}, std::uint32_t expectedEpoch = 0);
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
		bool Submit_FlowEntry(std::string& status);
		void Update_Flow();
		void Cancel_Flow(std::string status);
		[[nodiscard]] std::uint64_t Now_Milliseconds() const noexcept;

	private:
		KOUKU_SAYDON_PATTERN_AUDITION_SNAPSHOT m_Snapshot;
		KOUKU_SAYDON_PATTERN_AUDITION_SNAPSHOT m_ControlPreviousSnapshot;
		bool m_bControlRequest = false;
		bool m_bRunHasStarted = false;
		/* Exact scope of the in-flight request; results and lifecycles must
		echo it even if the target boss changed afterwards. */
		LostArk::Shared::KOUKUSAYDON_PATTERN_AUDITION_SCOPE m_RequestScope{};
		std::string m_strTargetBossPlacementId = "boss.kakulsaydon.g1.kouku";
		std::string m_strTargetBossArchetypeId = "BOSS_KAKULSAYDON_G1_KOUKU";
		bool m_bTargetTransitionPending = false;
		std::uint32_t m_iNextRequestSequence = 1u;
		std::uint64_t m_iStateStartedAtMilliseconds = 0u;
		KOUKU_SAYDON_PATTERN_FLOW_SNAPSHOT m_FlowSnapshot;
		std::vector<KOUKU_SAYDON_PATTERN_FLOW_ENTRY> m_FlowEntries;
		LostArk::Shared::GameplayDataRevision m_FlowRevision{};
		std::uint32_t m_iFlowSourceRevision = 0u;
		std::uint32_t m_iFlowRequestSequence = 0u;
		std::uint64_t m_iFlowWorldGeneration = 0u;
		std::uint64_t m_iFlowNextStartAtMilliseconds = 0u;
		bool m_bSubmittingFlowEntry = false;
		bool m_bFlowEntryCompleted = false;
		bool m_bStopFlowWhenAdmitted = false;
	};

	const char* Describe_KoukuSaydonPatternAuditionState(
		KOUKU_SAYDON_PATTERN_AUDITION_STATE state) noexcept;
}
