#include "CombatObjectProjectionRuntime.h"

#include <algorithm>
#include <cctype>
#include <cmath>

namespace
{
	bool Is_StableId(const std::string& value)
	{
		return !value.empty() &&
			value.size() <= LostArk::Shared::MAX_STABLE_NETWORK_ID_BYTES &&
			std::all_of(value.begin(), value.end(), [](const unsigned char ch)
			{
				return 0 != std::isalnum(ch) || ch == '_' || ch == '-' || ch == '.';
			});
	}

	bool Is_FinitePose(const LostArk::Shared::COMBAT_OBJECT_SNAPSHOT& object)
	{
		return std::isfinite(object.fPositionX) &&
			std::isfinite(object.fPositionY) &&
			std::isfinite(object.fPositionZ) &&
			std::isfinite(object.fYawDegrees);
	}
}

const Client::COMBAT_OBJECT_PROJECTION_RECORD*
Client::CCombatObjectProjectionRuntime::Find(
	const LostArk::Shared::COMBAT_OBJECT_ID combatObjectId) const
{
	const auto record = m_Records.find(combatObjectId);
	return record == m_Records.end() ? nullptr : &record->second;
}

bool_t Client::CCombatObjectProjectionRuntime::Stage_Spawn(
	const LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED& message,
	COMBAT_OBJECT_PROJECTION_RECORD& outRecord,
	bool_t& outIsDuplicate,
	std::string& outStatus) const
{
	outRecord = {};
	outIsDuplicate = false;
	if (message.iCombatObjectId ==
			LostArk::Shared::INVALID_COMBAT_OBJECT_ID ||
		message.iSourceNetEntityId == LostArk::Shared::INVALID_NET_ENTITY_ID ||
		0u == message.iSpawnTick ||
		0u == message.iServerTick ||
		static_cast<std::int32_t>(
			message.iServerTick - message.iSpawnTick) < 0 ||
		!Is_StableId(message.strCombatObjectArchetypeId) ||
		!Is_StableId(message.strClientVisualId) ||
		!std::isfinite(message.fPositionX) ||
		!std::isfinite(message.fPositionY) ||
		!std::isfinite(message.fPositionZ) ||
		!std::isfinite(message.fYawDegrees))
	{
		outStatus = "Combat-object spawn is malformed";
		return false;
	}

	const auto existing = m_Records.find(message.iCombatObjectId);
	if (existing != m_Records.end())
	{
		const COMBAT_OBJECT_PROJECTION_RECORD& record = existing->second;
		outIsDuplicate = record.iSourceNetEntityId ==
				message.iSourceNetEntityId &&
			record.iSpawnTick == message.iSpawnTick &&
			record.iLastSpawnServerTick == message.iServerTick &&
			record.strCombatObjectArchetypeId ==
				message.strCombatObjectArchetypeId &&
			record.strClientVisualId == message.strClientVisualId &&
			record.Snapshot.fPositionX == message.fPositionX &&
			record.Snapshot.fPositionY == message.fPositionY &&
			record.Snapshot.fPositionZ == message.fPositionZ &&
			record.Snapshot.fYawDegrees == message.fYawDegrees;
		if (!outIsDuplicate)
			outStatus = "Conflicting duplicate combat-object spawn";
		return outIsDuplicate;
	}
	if (m_Records.size() >=
		LostArk::Shared::MAX_COMBAT_OBJECTS_PER_SNAPSHOT)
	{
		outStatus = "Combat-object projection reached its wire capacity";
		return false;
	}

	outRecord.iCombatObjectId = message.iCombatObjectId;
	outRecord.iSourceNetEntityId = message.iSourceNetEntityId;
	outRecord.iSpawnTick = message.iSpawnTick;
	outRecord.iLastSpawnServerTick = message.iServerTick;
	outRecord.strCombatObjectArchetypeId =
		message.strCombatObjectArchetypeId;
	outRecord.strClientVisualId = message.strClientVisualId;
	outRecord.Snapshot.iCombatObjectId = message.iCombatObjectId;
	outRecord.Snapshot.iSourceNetEntityId = message.iSourceNetEntityId;
	outRecord.Snapshot.fPositionX = message.fPositionX;
	outRecord.Snapshot.fPositionY = message.fPositionY;
	outRecord.Snapshot.fPositionZ = message.fPositionZ;
	outRecord.Snapshot.fYawDegrees = message.fYawDegrees;
	outStatus.clear();
	return true;
}

bool_t Client::CCombatObjectProjectionRuntime::Stage_Snapshot(
	const std::vector<LostArk::Shared::COMBAT_OBJECT_SNAPSHOT>& objects,
	std::vector<LostArk::Shared::COMBAT_OBJECT_ID>& outOrderedIds,
	std::string& outStatus) const
{
	outOrderedIds.clear();
	if (objects.size() != m_Records.size() ||
		objects.size() > LostArk::Shared::MAX_COMBAT_OBJECTS_PER_SNAPSHOT)
	{
		outStatus = "Combat-object full snapshot set does not match lifecycle state";
		return false;
	}
	outOrderedIds.reserve(objects.size());
	LostArk::Shared::COMBAT_OBJECT_ID previous =
		LostArk::Shared::INVALID_COMBAT_OBJECT_ID;
	for (const LostArk::Shared::COMBAT_OBJECT_SNAPSHOT& object : objects)
	{
		const auto record = m_Records.find(object.iCombatObjectId);
		if (object.iCombatObjectId ==
				LostArk::Shared::INVALID_COMBAT_OBJECT_ID ||
			object.iSourceNetEntityId == LostArk::Shared::INVALID_NET_ENTITY_ID ||
			(previous != LostArk::Shared::INVALID_COMBAT_OBJECT_ID &&
			 !(previous < object.iCombatObjectId)) ||
			!Is_FinitePose(object) || record == m_Records.end() ||
			record->second.iSourceNetEntityId != object.iSourceNetEntityId)
		{
			outOrderedIds.clear();
			outStatus = "Combat-object full snapshot is conflicting or non-canonical";
			return false;
		}
		previous = object.iCombatObjectId;
		outOrderedIds.push_back(object.iCombatObjectId);
	}
	outStatus.clear();
	return true;
}
