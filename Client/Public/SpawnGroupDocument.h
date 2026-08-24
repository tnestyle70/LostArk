#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <filesystem>
#include <string>
#include <vector>

NS_BEGIN(Client)

struct SPAWN_ANCHOR_RECORD
{
	std::string anchorId;
	float3_t position = {};
	f32_t yawDegrees = 0.f;
};

struct SPAWN_WAVE_ENTRY_RECORD
{
	std::string archetypeId;
	uint32_t count = 1;
	std::string anchorId;
	uint32_t initialDelayMs = 0;
	uint32_t spawnIntervalMs = 250;
};

struct SPAWN_WAVE_RECORD
{
	std::string waveId;
	uint32_t startDelayMs = 0;
	/* false = ALL_DEAD, the wave ends when everything it spawned is dead.
	true = TIMER, it ends on nextWaveDelayMs from its own start so the next wave
	can open while this one is still standing. */
	bool_t usesTimerNextWave = false;
	uint32_t nextWaveDelayMs = 0;
	std::vector<SPAWN_WAVE_ENTRY_RECORD> entries;
};

struct SPAWN_GROUP_RECORD
{
	std::string spawnGroupId;
	std::string requiredCompletedGroupId;
	uint32_t maxAlive = 8;
	/* false = ONCE. true = REPEAT, the group restarts at its first wave once the
	field is clear and repeatDelayMs has passed. */
	bool_t repeats = false;
	uint32_t repeatDelayMs = 0;
	std::vector<SPAWN_WAVE_RECORD> waves;
};

class CSpawnGroupDocument final
{
public:
	static constexpr uint32_t MAX_ANCHOR_COUNT = 128;
	static constexpr uint32_t MAX_GROUP_COUNT = 32;
	static constexpr uint32_t MAX_WAVE_COUNT = 16;
	static constexpr uint32_t MAX_ENTRY_COUNT = 16;
	static constexpr uint32_t MAX_TOTAL_SPAWN_COUNT = 1000;

	bool_t Load(const std::filesystem::path& path,
		const std::string& expectedAreaId, std::string& outStatus);
	bool_t Save(const std::filesystem::path& path,
		const std::string& areaId, std::string& outStatus) const;
	void Reset();

	SPAWN_ANCHOR_RECORD* Find_Anchor(const std::string& anchorId);
	const SPAWN_ANCHOR_RECORD* Find_Anchor(const std::string& anchorId) const;
	SPAWN_GROUP_RECORD* Find_Group(const std::string& spawnGroupId);
	const SPAWN_GROUP_RECORD* Find_Group(const std::string& spawnGroupId) const;
	bool_t Add_Anchor(const SPAWN_ANCHOR_RECORD& anchor, std::string& outStatus);
	bool_t Remove_Anchor(const std::string& anchorId, std::string& outStatus);
	bool_t Add_Group(const SPAWN_GROUP_RECORD& group, std::string& outStatus);
	bool_t Remove_Group(const std::string& spawnGroupId, std::string& outStatus);

	const std::vector<SPAWN_ANCHOR_RECORD>& Get_Anchors() const { return m_Anchors; }
	std::vector<SPAWN_ANCHOR_RECORD>& Get_Anchors() { return m_Anchors; }
	const std::vector<SPAWN_GROUP_RECORD>& Get_Groups() const { return m_Groups; }
	std::vector<SPAWN_GROUP_RECORD>& Get_Groups() { return m_Groups; }
	uint32_t Get_Revision() const { return m_iRevision; }
	void Mark_Edited() { ++m_iRevision; }

	static bool_t Is_ValidStableId(const std::string& value);

private:
	bool_t Validate(std::string& outStatus) const;

private:
	std::vector<SPAWN_ANCHOR_RECORD> m_Anchors;
	std::vector<SPAWN_GROUP_RECORD> m_Groups;
	uint32_t m_iRevision = 1;
};

NS_END
