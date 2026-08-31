#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Network/PacketMessages.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

NS_BEGIN(Client)

struct COMBAT_OBJECT_PROJECTION_RECORD final
{
	LostArk::Shared::COMBAT_OBJECT_ID iCombatObjectId =
		LostArk::Shared::INVALID_COMBAT_OBJECT_ID;
	LostArk::Shared::NET_ENTITY_ID iSourceNetEntityId =
		LostArk::Shared::INVALID_NET_ENTITY_ID;
	uint32_t iSpawnTick = 0u;
	uint32_t iLastSpawnServerTick = 0u;
	uint32_t iNextPresentationRetryTick = 0u;
	uint8_t iPresentationAttemptCount = 0u;
	std::string strCombatObjectArchetypeId;
	std::string strClientVisualId;
	LostArk::Shared::COMBAT_OBJECT_SNAPSHOT Snapshot{};
	uint64_t iPresentationHandle = 0u;
};

class CCombatObjectProjectionRuntime final
{
public:
	template<typename TPresentationSink>
	bool_t Apply_Spawn(
		const LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED& message,
		TPresentationSink& sink,
		std::string& outStatus)
	{
		COMBAT_OBJECT_PROJECTION_RECORD staged;
		bool_t isDuplicate = false;
		if (!Stage_Spawn(message, staged, isDuplicate, outStatus))
			return false;
		if (isDuplicate)
		{
			outStatus = "Ignored duplicate combat-object spawn";
			return true;
		}

		std::string presentationStatus;
		uint64_t presentationHandle = 0u;
		if (!sink.Spawn(message, presentationHandle, presentationStatus) ||
			0u == presentationHandle)
		{
			presentationHandle = 0u;
			staged.iPresentationAttemptCount = 1u;
			staged.iNextPresentationRetryTick =
				Next_RetryTick(message.iServerTick);
			outStatus = presentationStatus.empty() ?
				"Combat-object visual admission failed; logical state was kept" :
				presentationStatus;
		}
		else
		{
			staged.iPresentationAttemptCount = 1u;
			outStatus = "Applied combat-object spawn";
		}
		staged.iPresentationHandle = presentationHandle;
		m_Records.emplace(staged.iCombatObjectId, std::move(staged));
		return true;
	}

	template<typename TPresentationSink>
	bool_t Apply_Snapshot(
		const uint32_t serverTick,
		const std::vector<LostArk::Shared::COMBAT_OBJECT_SNAPSHOT>& objects,
		TPresentationSink& sink,
		std::string& outStatus)
	{
		if (0u == serverTick)
		{
			outStatus = "Combat-object full snapshot has no Server tick";
			return false;
		}
		std::vector<LostArk::Shared::COMBAT_OBJECT_ID> orderedIds;
		if (!Stage_Snapshot(objects, orderedIds, outStatus))
			return false;

		bool_t presentationSucceeded = true;
		for (size_t index = 0u; index < orderedIds.size(); ++index)
		{
			auto record = m_Records.find(orderedIds[index]);
			if (0u == record->second.iPresentationHandle &&
				record->second.iPresentationAttemptCount < 3u &&
				Has_ReachedTick(
					serverTick, record->second.iNextPresentationRetryTick))
			{
				LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED retry{};
				retry.iCombatObjectId = record->second.iCombatObjectId;
				retry.iSourceNetEntityId = record->second.iSourceNetEntityId;
				retry.iSpawnTick = record->second.iSpawnTick;
				retry.iServerTick = serverTick;
				retry.strCombatObjectArchetypeId =
					record->second.strCombatObjectArchetypeId;
				retry.strClientVisualId = record->second.strClientVisualId;
				retry.fPositionX = objects[index].fPositionX;
				retry.fPositionY = objects[index].fPositionY;
				retry.fPositionZ = objects[index].fPositionZ;
				retry.fYawDegrees = objects[index].fYawDegrees;
				retry.PinnedDefinitionRevision =
					record->second.Snapshot.PinnedDefinitionRevision;
				std::string retryStatus;
				uint64_t retryHandle = 0u;
				++record->second.iPresentationAttemptCount;
				if (sink.Spawn(retry, retryHandle, retryStatus) &&
					0u != retryHandle)
				{
					record->second.iPresentationHandle = retryHandle;
				}
				else
				{
					record->second.iNextPresentationRetryTick =
						Next_RetryTick(serverTick);
					presentationSucceeded = false;
				}
			}
			if (record->second.iPresentationHandle != 0u &&
				!sink.Update(
					record->second.iPresentationHandle, objects[index]))
			{
				sink.Stop(record->second.iPresentationHandle);
				record->second.iPresentationHandle = 0u;
				record->second.iNextPresentationRetryTick =
					Next_RetryTick(serverTick);
				presentationSucceeded = false;
			}
			record->second.Snapshot = objects[index];
		}
		outStatus = presentationSucceeded ?
			"Applied full combat-object snapshot" :
			"Combat-object root update failed; logical state was kept";
		return true;
	}

	template<typename TPresentationSink>
	bool_t Apply_Despawn(
		const LostArk::Shared::S2C_COMBAT_OBJECT_DESPAWNED& message,
		TPresentationSink& sink,
		std::string& outStatus)
	{
		if (message.iCombatObjectId ==
			LostArk::Shared::INVALID_COMBAT_OBJECT_ID)
		{
			outStatus = "Combat-object despawn has no stable ID";
			return false;
		}
		const auto record = m_Records.find(message.iCombatObjectId);
		if (record == m_Records.end())
		{
			outStatus = "Ignored duplicate combat-object despawn";
			return true;
		}
		if (record->second.iPresentationHandle != 0u)
			sink.Stop(record->second.iPresentationHandle);
		m_Records.erase(record);
		outStatus = "Applied combat-object despawn";
		return true;
	}

	template<typename TPresentationSink>
	size_t Remove_Source(
		const LostArk::Shared::NET_ENTITY_ID sourceNetEntityId,
		TPresentationSink& sink)
	{
		size_t removed = 0u;
		for (auto record = m_Records.begin(); record != m_Records.end();)
		{
			if (record->second.iSourceNetEntityId != sourceNetEntityId)
			{
				++record;
				continue;
			}
			if (record->second.iPresentationHandle != 0u)
				sink.Stop(record->second.iPresentationHandle);
			record = m_Records.erase(record);
			++removed;
		}
		return removed;
	}

	template<typename TPresentationSink>
	void Reset(TPresentationSink& sink)
	{
		for (const auto& [id, record] : m_Records)
		{
			(void)id;
			if (record.iPresentationHandle != 0u)
				sink.Stop(record.iPresentationHandle);
		}
		m_Records.clear();
	}

	size_t Get_Count() const { return m_Records.size(); }
	const COMBAT_OBJECT_PROJECTION_RECORD* Find(
		LostArk::Shared::COMBAT_OBJECT_ID combatObjectId) const;

private:
	static uint32_t Next_RetryTick(const uint32_t tick)
	{
		const uint32_t next = tick + 30u;
		return 0u == next ? 1u : next;
	}
	static bool_t Has_ReachedTick(
		const uint32_t candidate,
		const uint32_t target)
	{
		return 0u != candidate && 0u != target &&
			static_cast<std::int32_t>(candidate - target) >= 0;
	}
	bool_t Stage_Spawn(
		const LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED& message,
		COMBAT_OBJECT_PROJECTION_RECORD& outRecord,
		bool_t& outIsDuplicate,
		std::string& outStatus) const;
	bool_t Stage_Snapshot(
		const std::vector<LostArk::Shared::COMBAT_OBJECT_SNAPSHOT>& objects,
		std::vector<LostArk::Shared::COMBAT_OBJECT_ID>& outOrderedIds,
		std::string& outStatus) const;

	std::unordered_map<LostArk::Shared::COMBAT_OBJECT_ID,
		COMBAT_OBJECT_PROJECTION_RECORD> m_Records;
};

NS_END
