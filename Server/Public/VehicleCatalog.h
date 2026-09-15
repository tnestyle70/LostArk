#pragma once

#include "GameplayCatalog.h"
#include "Network/PacketMessages.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace LostArk::Server
{
	enum class VEHICLE_SKILL_SLOT : std::uint8_t
	{
		SPACE,
		Q,
		W,
		E,
	};

	/* One EFTable_Vehicle skill of a vehicle: MovingSkill on SPACE, SkillId0..2 on
	Q/W/E. The room owns its length, cooldown and any root motion the vehicle clip
	carries; the Client only presents the clips. */
	struct SERVER_VEHICLE_SKILL
	{
		LostArk::Shared::SKILL_ID iSkillId = LostArk::Shared::INVALID_SKILL_ID;
		VEHICLE_SKILL_SLOT eSlot = VEHICLE_SKILL_SLOT::SPACE;
		std::uint32_t iCooldownMs = 0u;
		std::uint32_t iActionDurationMs = 0u;
		std::vector<ROOT_MOTION_SAMPLE> RootMotion;
	};

	struct SERVER_VEHICLE_DEFINITION
	{
		LostArk::Shared::VEHICLE_ID iVehicleId = LostArk::Shared::INVALID_VEHICLE_ID;
		float fMoveSpeed = 0.f;
		std::vector<SERVER_VEHICLE_SKILL> Skills;

		const SERVER_VEHICLE_SKILL* Find_Skill(const LostArk::Shared::SKILL_ID skillId) const
		{
			for (const SERVER_VEHICLE_SKILL& skill : Skills)
				if (skill.iSkillId == skillId)
					return &skill;
			return nullptr;
		}
	};

	class CVehicleCatalog final
	{
	public:
		bool Load();

		const SERVER_VEHICLE_DEFINITION* Find_Vehicle(
			LostArk::Shared::VEHICLE_ID vehicleId) const;

		const std::string& Get_Status() const { return m_strStatus; }

	private:
		std::unordered_map<LostArk::Shared::VEHICLE_ID, SERVER_VEHICLE_DEFINITION> m_Vehicles;
		std::string m_strStatus;
	};
}
