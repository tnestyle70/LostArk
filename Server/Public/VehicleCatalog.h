#pragma once

#include "Network/PacketMessages.h"

#include <string>
#include <unordered_map>

namespace LostArk::Server
{
	struct SERVER_VEHICLE_DEFINITION
	{
		LostArk::Shared::VEHICLE_ID iVehicleId = LostArk::Shared::INVALID_VEHICLE_ID;
		float fMoveSpeed = 0.f;
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
