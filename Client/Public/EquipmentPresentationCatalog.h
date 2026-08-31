#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Network/PacketMessages.h"

#include <string>
#include <string_view>
#include <vector>

NS_BEGIN(Client)

enum class EQUIPMENT_SLOT_ID
{
	HEAD,
	SHOULDER,
	UPPER,
	LOWER,
	HANDS,
	WEAPON,
	END
};

enum class EQUIPMENT_ATTACHMENT_MODE
{
	SKINNED,
	SOCKETED,
	END
};

struct EQUIPMENT_PRESENTATION_PART final
{
	std::string partId;
	std::string partRole;
	EQUIPMENT_ATTACHMENT_MODE attachmentMode =
		EQUIPMENT_ATTACHMENT_MODE::END;
	std::string modelAssetId;
	std::string socketBoneId;
	f32_t socketYawDegrees = 0.f;
	LostArk::Shared::PLAYER_STANCE_ID requiredStance =
		LostArk::Shared::PLAYER_STANCE_ID::NONE;
	uint32_t hiddenMeshMask = 0u;
};

struct EQUIPMENT_VISUAL_SET final
{
	std::string visualSetId;
	LostArk::Shared::CHARACTER_CLASS_ID classId =
		LostArk::Shared::CHARACTER_CLASS_ID::END;
	std::string categoryId;
	std::string catalogStatus;
	EQUIPMENT_SLOT_ID primarySlot = EQUIPMENT_SLOT_ID::END;
	std::vector<EQUIPMENT_SLOT_ID> occupiedSlots;
	std::vector<EQUIPMENT_PRESENTATION_PART> parts;
};

class CEquipmentPresentationCatalog final
{
public:
	bool_t Load(std::string& outError);

	const std::vector<EQUIPMENT_VISUAL_SET>& Get_Sets() const
	{
		return m_Sets;
	}

	const EQUIPMENT_VISUAL_SET* Find_Set(
		std::string_view visualSetId) const;
	std::vector<const EQUIPMENT_VISUAL_SET*> Find_ByClassAndPrimarySlot(
		LostArk::Shared::CHARACTER_CLASS_ID classId,
		EQUIPMENT_SLOT_ID slot) const;

private:
	std::vector<EQUIPMENT_VISUAL_SET> m_Sets;
};

const char_t* To_String(EQUIPMENT_SLOT_ID slot);

NS_END
