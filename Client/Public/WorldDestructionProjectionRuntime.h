#pragma once

#include "Client_Defines.h"
#include "DeployPropCatalog.h"
#include "Network/PacketMessages.h"
#include "WorldDestructionProjectionDocument.h"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

NS_BEGIN(Client)

struct WORLD_DESTRUCTION_PROJECTION_TRANSACTION final
{
	std::string strCombatRuntimeRevision;
	uint32_t iEncounterEpoch = 0u;
	uint32_t iServerTick = 0u;
	std::vector<std::pair<uint64_t, DEPLOY_PROP_STATE>> PlacementStates;
	std::vector<LostArk::Shared::WORLD_DESTRUCTION_STATE_WIRE> GroupStates;
	std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE> LiveEvents;
	uint64_t iLastEventSequence = 0u;
};

class CWorldDestructionProjectionRuntime final
{
public:
	bool_t Stage_Full(
		const CWorldDestructionProjectionDocument& document,
		const LostArk::Shared::S2C_WORLD_DESTRUCTION_FULL_SYNC& message,
		WORLD_DESTRUCTION_PROJECTION_TRANSACTION& outTransaction,
		std::string& outStatus) const;
	bool_t Stage_Delta(
		const CWorldDestructionProjectionDocument& document,
		const LostArk::Shared::S2C_WORLD_DESTRUCTION_DELTA& message,
		WORLD_DESTRUCTION_PROJECTION_TRANSACTION& outTransaction,
		std::string& outStatus) const;
	void Reset();

	template<typename TStateSink>
	bool_t Apply_Full(
		const CWorldDestructionProjectionDocument& document,
		const LostArk::Shared::S2C_WORLD_DESTRUCTION_FULL_SYNC& message,
		TStateSink& stateSink,
		std::string& outStatus)
	{
		WORLD_DESTRUCTION_PROJECTION_TRANSACTION transaction;
		if (!Stage_Full(document, message, transaction, outStatus))
			return false;
		if (!stateSink.Set_States(transaction.PlacementStates))
		{
			outStatus = "World destruction projection sink rolled back";
			return false;
		}
		Commit(std::move(transaction));
		outStatus = "Applied persistent world destruction full sync";
		return true;
	}

	template<typename TStateSink>
	bool_t Apply_Delta(
		const CWorldDestructionProjectionDocument& document,
		const LostArk::Shared::S2C_WORLD_DESTRUCTION_DELTA& message,
		TStateSink& stateSink,
		std::string& outStatus,
		std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE>*
			pOutLiveEvents = nullptr)
	{
		WORLD_DESTRUCTION_PROJECTION_TRANSACTION transaction;
		if (!Stage_Delta(document, message, transaction, outStatus))
			return false;
		if (!transaction.PlacementStates.empty() &&
			!stateSink.Set_States(transaction.PlacementStates))
		{
			outStatus = "World destruction delta sink rolled back";
			return false;
		}
		const bool_t changed = !transaction.PlacementStates.empty();
		std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE> liveEvents =
			std::move(transaction.LiveEvents);
		Commit(std::move(transaction));
		if (nullptr != pOutLiveEvents)
			*pOutLiveEvents = std::move(liveEvents);
		outStatus = changed ?
			"Applied persistent world destruction delta" :
			"Ignored duplicate persistent world destruction delta";
		return true;
	}

	bool_t Is_Synchronized() const { return m_isSynchronized; }
	const std::string& Get_CombatRuntimeRevision() const
	{
		return m_strCombatRuntimeRevision;
	}
	uint32_t Get_EncounterEpoch() const { return m_iEncounterEpoch; }
	uint32_t Get_ServerTick() const { return m_iServerTick; }
	uint64_t Get_LastEventSequence() const { return m_iLastEventSequence; }
	const std::vector<LostArk::Shared::WORLD_DESTRUCTION_STATE_WIRE>&
	Get_GroupStates() const { return m_GroupStates; }

private:
	void Commit(WORLD_DESTRUCTION_PROJECTION_TRANSACTION transaction);


	std::string m_strCombatRuntimeRevision;
	uint32_t m_iEncounterEpoch = 0u;
	uint32_t m_iServerTick = 0u;
	uint64_t m_iLastEventSequence = 0u;
	std::vector<LostArk::Shared::WORLD_DESTRUCTION_STATE_WIRE> m_GroupStates;
	bool_t m_isSynchronized = false;
};

NS_END
