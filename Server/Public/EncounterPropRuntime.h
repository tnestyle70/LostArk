#pragma once

#include "Network/PacketMessages.h"

#include <cstdint>
#include <string>
#include <vector>

namespace LostArk::Server
{
	/* Repeatable encounter props. Wall destruction is one-way: a group leaves
	   INTACT once and never comes back. The four pillars are the opposite - the
	   same four slots are raised and shattered four times in one fight - so they
	   need a lifecycle of their own rather than a rewound destruction group.

	   Every transition is prepared against a copy and committed only if the whole
	   batch validates, so a half-applied cycle can never reach a client. */
	/* Slot IDs are the replicated encounter identity. The Client projection
	   resolves them to the four existing inner DEPLOY_ITR_02326 placements;
	   placement IDs never become Server combat authority. */
	/* A raised slot is cover. Its position is published from the one Deploy
	   placement the Client renders, so the Server and the screen can never
	   describe different ground. */
	struct ENCOUNTER_PROP_SLOT_DESCRIPTOR final
	{
		std::string strSlotId;
		float fPositionX = 0.f;
		float fPositionZ = 0.f;
	};

	struct ENCOUNTER_PROP_SET_DESCRIPTOR final
	{
		std::string strPropSetId;
		std::string strEncounterId;
		/* Metres. A boss attack whose segment to the target passes through this
		   circle around a raised slot is answered by the stele instead. */
		float fCoverRadiusMeters = 0.f;
		std::vector<ENCOUNTER_PROP_SLOT_DESCRIPTOR> Slots;
	};

	/* Reads <world>.encounterpropsbootstrap. A world that owns no encounter prop
	   set is not an error, so an absent file answers true with an empty set. */
	[[nodiscard]] bool Load_EncounterPropSets(
		LostArk::Shared::WORLD_ID worldId,
		std::vector<ENCOUNTER_PROP_SET_DESCRIPTOR>& outSets,
		std::string& status);

	struct ENCOUNTER_PROP_SLOT_STATE final
	{
		std::string strSlotId;
		LostArk::Shared::ENCOUNTER_PROP_STATE eState =
			LostArk::Shared::ENCOUNTER_PROP_STATE::HIDDEN;
		std::uint32_t iStateVersion = 0u;
		std::uint32_t iStateStartTick = 0u;
		std::uint32_t iOccurrenceSequence = 0u;
		/* Copied from the descriptor so a live cover query needs the slot list
		   alone rather than a second lookup against the authored set. */
		float fPositionX = 0.f;
		float fPositionZ = 0.f;
	};

	enum class ENCOUNTER_PROP_PREPARE_RESULT : std::uint8_t
	{
		READY,
		/* The requested transition is already the live state for this occurrence,
		   so a duplicate stage edge is a no-op instead of a second event. */
		NO_CHANGE,
		REJECTED
	};

	struct ENCOUNTER_PROP_TRANSACTION final
	{
		std::uint32_t iEncounterEpoch = 0u;
		std::uint32_t iCommitTick = 0u;
		std::uint32_t iOccurrenceSequence = 0u;
		std::vector<ENCOUNTER_PROP_SLOT_STATE> Slots;
	};

	class CEncounterPropRuntime final
	{
	public:
		bool Initialize(
			const ENCOUNTER_PROP_SET_DESCRIPTOR& descriptor,
			std::string& status,
			std::uint32_t initialServerTick = 1u);
		bool Reset(std::string& status, std::uint32_t resetServerTick = 1u);

		/* Raise every slot for a new occurrence. The sequence has to move
		   forward, which is what stops one pattern from spawning the same cycle
		   twice and what lets the next cycle reuse the same slots. */
		ENCOUNTER_PROP_PREPARE_RESULT Prepare_Spawn(
			std::uint32_t occurrenceSequence,
			std::uint32_t serverTick,
			ENCOUNTER_PROP_TRANSACTION& transaction,
			std::string& status) const;
		/* Shatter every raised slot of the current occurrence. Slots already
		   shattered on an earlier edge of the same occurrence are skipped, so
		   this doubles as the sweep that retires whatever a pattern left
		   standing when it ends. */
		ENCOUNTER_PROP_PREPARE_RESULT Prepare_Break(
			std::uint32_t occurrenceSequence,
			std::uint32_t serverTick,
			ENCOUNTER_PROP_TRANSACTION& transaction,
			std::string& status) const;
		/* Shatter only the named slots of the current occurrence. The original
		   breaks the four pillars two at a time on two separate stage edges, and
		   a player skill can shatter a single pillar early, so the caller names
		   which slots this edge owns. Slot IDs are the stored identity; a vector
		   index would break the moment the authored set is reordered. */
		ENCOUNTER_PROP_PREPARE_RESULT Prepare_BreakSlots(
			const std::vector<std::string>& slotIds,
			std::uint32_t occurrenceSequence,
			std::uint32_t serverTick,
			ENCOUNTER_PROP_TRANSACTION& transaction,
			std::string& status) const;
		/* Retire the shattered slots once their authored break time is spent. */
		ENCOUNTER_PROP_PREPARE_RESULT Prepare_DueRemoval(
			std::uint32_t serverTick,
			std::uint32_t breakingTicks,
			ENCOUNTER_PROP_TRANSACTION& transaction,
			std::string& status) const;

		bool Commit(
			const ENCOUNTER_PROP_TRANSACTION& transaction,
			std::string& status);

		bool Is_Initialized() const noexcept { return m_bInitialized; }
		std::uint32_t Get_EncounterEpoch() const noexcept
		{
			return m_iEncounterEpoch;
		}
		std::uint32_t Get_OccurrenceSequence() const noexcept
		{
			return m_iOccurrenceSequence;
		}
		float Get_CoverRadiusMeters() const noexcept
		{
			return m_Descriptor.fCoverRadiusMeters;
		}
		const std::string& Get_PropSetId() const noexcept
		{
			return m_Descriptor.strPropSetId;
		}
		const std::vector<ENCOUNTER_PROP_SLOT_STATE>& Get_SlotStates() const
		{
			return m_Slots;
		}

	private:
		ENCOUNTER_PROP_PREPARE_RESULT Prepare_Transition(
			LostArk::Shared::ENCOUNTER_PROP_STATE fromState,
			LostArk::Shared::ENCOUNTER_PROP_STATE toState,
			std::uint32_t occurrenceSequence,
			std::uint32_t serverTick,
			ENCOUNTER_PROP_TRANSACTION& transaction,
			std::string& status) const;

		ENCOUNTER_PROP_SET_DESCRIPTOR m_Descriptor;
		std::vector<ENCOUNTER_PROP_SLOT_STATE> m_Slots;
		std::uint32_t m_iEncounterEpoch = 0u;
		std::uint32_t m_iOccurrenceSequence = 0u;
		bool m_bInitialized = false;
	};
}
