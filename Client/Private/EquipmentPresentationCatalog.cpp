#include "EquipmentPresentationCatalog.h"

#include "DataJson.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cctype>
#include <fstream>
#include <limits>
#include <set>

namespace
{
	using namespace Client;

	constexpr const wchar_t* CATALOG_PATH =
		L"Actors/EquipmentPresentationCatalog.json";
	constexpr std::array<const char_t*, ETOI(EQUIPMENT_SLOT_ID::END)>
		SLOT_NAMES
	{
		"HEAD", "SHOULDER", "UPPER", "LOWER", "HANDS", "WEAPON"
	};

	bool_t Read_Text(
		const std::filesystem::path& path,
		std::string& outText,
		std::string& outError)
	{
		std::ifstream input(path, std::ios::binary);
		if (!input.is_open())
		{
			outError = "Equipment catalog file is missing: " + path.string();
			return false;
		}
		outText.assign(
			std::istreambuf_iterator<char>(input),
			std::istreambuf_iterator<char>());
		if (!input.good() && !input.eof())
		{
			outError = "Equipment catalog read failed: " + path.string();
			return false;
		}
		return true;
	}

	bool_t Read_RequiredString(
		const DATA_JSON_VALUE& object,
		const char_t* pName,
		std::string& outValue)
	{
		const DATA_JSON_VALUE* value = object.Find(pName);
		if (nullptr == value || !value->Is_String() ||
			value->Get_String().empty())
		{
			return false;
		}
		outValue = value->Get_String();
		return true;
	}

	bool_t Is_StableId(const std::string& value)
	{
		if (value.empty() || value.size() > 128u)
			return false;
		return std::all_of(value.begin(), value.end(),
			[](const unsigned char character)
			{
				return 0 != std::isalnum(character) ||
					character == '_' || character == '-' ||
					character == '.';
			});
	}

	bool_t Is_AllowedCategory(const std::string& value)
	{
		return value == "APPAREL_HEAD" || value == "APPAREL_OUTFIT" ||
			value == "APPAREL_UPPER" || value == "APPAREL_SHOULDER" ||
			value == "WEAPON_SET";
	}

	bool_t Is_AllowedStatus(const std::string& value)
	{
		return value == "READY_ALTERNATIVE" || value == "BASELINE_PART" ||
			value == "BASELINE_WEAPON";
	}

	EQUIPMENT_SLOT_ID Parse_Slot(const std::string& value)
	{
		for (size_t index = 0u; index < SLOT_NAMES.size(); ++index)
		{
			if (value == SLOT_NAMES[index])
				return static_cast<EQUIPMENT_SLOT_ID>(index);
		}
		return EQUIPMENT_SLOT_ID::END;
	}

	LostArk::Shared::CHARACTER_CLASS_ID Parse_Class(
		const std::string& value)
	{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		if (value == "LANCE_MASTER") return CHARACTER_CLASS_ID::LANCE_MASTER;
		if (value == "GUNSLINGER") return CHARACTER_CLASS_ID::GUNSLINGER;
		if (value == "SLAYER") return CHARACTER_CLASS_ID::SLAYER;
		if (value == "ARTIST") return CHARACTER_CLASS_ID::ARTIST;
		if (value == "DIMENSIONMASTER") return CHARACTER_CLASS_ID::DIMENSIONMASTER;
		if (value == "WARLORD") return CHARACTER_CLASS_ID::WARLORD;
		return CHARACTER_CLASS_ID::END;
	}

	LostArk::Shared::PLAYER_STANCE_ID Parse_Stance(
		const DATA_JSON_VALUE& value)
	{
		using LostArk::Shared::PLAYER_STANCE_ID;
		if (value.Is_Null()) return PLAYER_STANCE_ID::NONE;
		if (!value.Is_String()) return PLAYER_STANCE_ID::END;
		const std::string& name = value.Get_String();
		if (name == "NONE") return PLAYER_STANCE_ID::NONE;
		if (name == "LANCE_MASTER_LONG_SPEAR")
			return PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
		if (name == "LANCE_MASTER_SHORT_SPEAR")
			return PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR;
		if (name == "WARLORD_NORMAL") return PLAYER_STANCE_ID::WARLORD_NORMAL;
		if (name == "WARLORD_DEFENSE") return PLAYER_STANCE_ID::WARLORD_DEFENSE;
		return PLAYER_STANCE_ID::END;
	}

	bool_t Read_U32(
		const DATA_JSON_VALUE& object,
		const char_t* pName,
		uint32_t& outValue)
	{
		const DATA_JSON_VALUE* value = object.Find(pName);
		if (nullptr == value || !value->Is_Number())
			return false;
		const double number = value->Get_Number();
		if (!std::isfinite(number) || number < 0.0 ||
			std::floor(number) != number ||
			number > static_cast<double>(
				(std::numeric_limits<uint32_t>::max)()))
		{
			return false;
		}
		outValue = static_cast<uint32_t>(number);
		return true;
	}

	bool_t Read_FiniteFloat(
		const DATA_JSON_VALUE& object,
		const char_t* pName,
		f32_t& outValue)
	{
		const DATA_JSON_VALUE* value = object.Find(pName);
		if (nullptr == value || !value->Is_Number() ||
			!std::isfinite(value->Get_Number()))
		{
			return false;
		}
		outValue = static_cast<f32_t>(value->Get_Number());
		return std::isfinite(outValue);
	}

	bool_t Validate_Resource(
		const std::string& assetId,
		std::string& outError)
	{
		const std::filesystem::path relative(assetId);
		if (!assetId.starts_with("Character/") || relative.is_absolute() ||
			relative.has_root_path() || relative.extension() != L".wmodel")
		{
			outError = "Unsafe equipment model asset ID: " + assetId;
			return false;
		}
		for (const std::filesystem::path& component : relative)
		{
			if (component == L"..")
			{
				outError = "Equipment model escapes Resources: " + assetId;
				return false;
			}
		}

		const std::filesystem::path resolved =
			CRuntimeAssetRoot::Resolve(relative);
		std::error_code error;
		if (resolved.empty() ||
			!std::filesystem::is_regular_file(resolved, error) || error)
		{
			outError = "Equipment model file is missing: " + assetId;
			return false;
		}
		return true;
	}

	bool_t Parse_Part(
		const DATA_JSON_VALUE& value,
		EQUIPMENT_PRESENTATION_PART& outPart,
		std::string& outError)
	{
		if (!value.Is_Object() || 8u != value.Get_Object().size() ||
			!Read_RequiredString(value, "partId", outPart.partId) ||
			!Read_RequiredString(value, "partRole", outPart.partRole) ||
			!Read_RequiredString(
				value, "modelAssetId", outPart.modelAssetId) ||
			!Read_FiniteFloat(
				value, "socketYawDegrees", outPart.socketYawDegrees) ||
			!Read_U32(value, "hiddenMeshMask", outPart.hiddenMeshMask) ||
			!Is_StableId(outPart.partId) || !Is_StableId(outPart.partRole))
		{
			outError = "Equipment part contract mismatch.";
			return false;
		}

		const DATA_JSON_VALUE* attachment = value.Find("attachmentMode");
		const DATA_JSON_VALUE* socket = value.Find("socketBoneId");
		const DATA_JSON_VALUE* stance = value.Find("requiredStance");
		if (nullptr == attachment || !attachment->Is_String() ||
			nullptr == socket || nullptr == stance ||
			std::abs(outPart.socketYawDegrees) > 360.f)
		{
			outError = "Equipment attachment contract mismatch: " +
				outPart.partId;
			return false;
		}

		const std::string& mode = attachment->Get_String();
		if (mode == "SKINNED")
		{
			outPart.attachmentMode = EQUIPMENT_ATTACHMENT_MODE::SKINNED;
			if (!socket->Is_Null() || !stance->Is_Null() ||
				0.f != outPart.socketYawDegrees)
			{
				outError = "Skinned equipment cannot declare socket fields: " +
					outPart.partId;
				return false;
			}
		}
		else if (mode == "SOCKETED")
		{
			outPart.attachmentMode = EQUIPMENT_ATTACHMENT_MODE::SOCKETED;
			if (!socket->Is_String() || socket->Get_String().empty() ||
				!Is_StableId(socket->Get_String()))
			{
				outError = "Socketed equipment has an invalid bone ID: " +
					outPart.partId;
				return false;
			}
			outPart.socketBoneId = socket->Get_String();
			outPart.requiredStance = Parse_Stance(*stance);
			if (LostArk::Shared::PLAYER_STANCE_ID::END ==
				outPart.requiredStance)
			{
				outError = "Equipment part has an unknown stance: " +
					outPart.partId;
				return false;
			}
		}
		else
		{
			outError = "Unknown equipment attachment mode: " + mode;
			return false;
		}

		return Validate_Resource(outPart.modelAssetId, outError);
	}
}

const char_t* Client::To_String(const EQUIPMENT_SLOT_ID slot)
{
	const size_t index = static_cast<size_t>(slot);
	return index < SLOT_NAMES.size() ? SLOT_NAMES[index] : "UNKNOWN";
}

bool_t Client::CEquipmentPresentationCatalog::Load(std::string& outError)
{
	outError.clear();

	const std::filesystem::path path =
		CProjectDataRoot::Resolve(CATALOG_PATH);
	if (path.empty())
	{
		outError = "Equipment catalog path escapes Data root.";
		return false;
	}

	std::string text;
	if (!Read_Text(path, text, outError))
		return false;

	DATA_JSON_VALUE root;
	if (!CDataJson::Parse(text, root, outError) || !root.Is_Object())
	{
		if (outError.empty())
			outError = "Equipment catalog root is not an object.";
		return false;
	}

	const DATA_JSON_VALUE* schema = root.Find("schema");
	const DATA_JSON_VALUE* version = root.Find("formatVersion");
	const DATA_JSON_VALUE* slotIds = root.Find("slotIds");
	const DATA_JSON_VALUE* visualSets = root.Find("visualSets");
	if (4u != root.Get_Object().size() || nullptr == schema ||
		!schema->Is_String() ||
		schema->Get_String() != "lostark.equipment-presentation-catalog" ||
		nullptr == version || !version->Is_Number() ||
		1.0 != version->Get_Number() || nullptr == slotIds ||
		!slotIds->Is_Array() || SLOT_NAMES.size() != slotIds->Get_Array().size() ||
		nullptr == visualSets || !visualSets->Is_Array() ||
		visualSets->Get_Array().empty() || visualSets->Get_Array().size() > 256u)
	{
		outError = "Equipment catalog root contract mismatch.";
		return false;
	}

	for (size_t index = 0u; index < SLOT_NAMES.size(); ++index)
	{
		const DATA_JSON_VALUE& slot = slotIds->Get_Array()[index];
		if (!slot.Is_String() || slot.Get_String() != SLOT_NAMES[index])
		{
			outError = "Equipment slotIds must use the canonical order.";
			return false;
		}
	}

	std::set<std::string> visualSetIds;
	std::vector<EQUIPMENT_VISUAL_SET> stagedSets;
	stagedSets.reserve(visualSets->Get_Array().size());
	for (const DATA_JSON_VALUE& value : visualSets->Get_Array())
	{
		EQUIPMENT_VISUAL_SET set;
		std::string classId;
		std::string primarySlot;
		if (!value.Is_Object() || 7u != value.Get_Object().size() ||
			!Read_RequiredString(value, "visualSetId", set.visualSetId) ||
			!Read_RequiredString(value, "classId", classId) ||
			!Read_RequiredString(value, "categoryId", set.categoryId) ||
			!Read_RequiredString(value, "catalogStatus", set.catalogStatus) ||
			!Read_RequiredString(value, "primarySlot", primarySlot) ||
			!Is_StableId(set.visualSetId) ||
			!Is_AllowedCategory(set.categoryId) ||
			!Is_AllowedStatus(set.catalogStatus) ||
			!visualSetIds.insert(set.visualSetId).second)
		{
			outError = "Equipment visual-set contract mismatch.";
			return false;
		}

		set.classId = Parse_Class(classId);
		set.primarySlot = Parse_Slot(primarySlot);
		const DATA_JSON_VALUE* occupied = value.Find("occupiedSlots");
		const DATA_JSON_VALUE* parts = value.Find("parts");
		if (!LostArk::Shared::Is_Supported_Playable_Character_Class(set.classId) ||
			EQUIPMENT_SLOT_ID::END == set.primarySlot ||
			nullptr == occupied || !occupied->Is_Array() ||
			occupied->Get_Array().empty() ||
			occupied->Get_Array().size() > SLOT_NAMES.size() ||
			nullptr == parts || !parts->Is_Array() ||
			parts->Get_Array().empty() || parts->Get_Array().size() > 16u)
		{
			outError = "Equipment visual set has an invalid class, slot, or part list: " +
				set.visualSetId;
			return false;
		}

		std::set<EQUIPMENT_SLOT_ID> uniqueSlots;
		for (const DATA_JSON_VALUE& slotValue : occupied->Get_Array())
		{
			if (!slotValue.Is_String())
			{
				outError = "Equipment occupied slot is not a string: " +
					set.visualSetId;
				return false;
			}
			const EQUIPMENT_SLOT_ID slot = Parse_Slot(slotValue.Get_String());
			if (EQUIPMENT_SLOT_ID::END == slot ||
				!uniqueSlots.insert(slot).second)
			{
				outError = "Equipment occupied slot is unknown or duplicated: " +
					set.visualSetId;
				return false;
			}
			set.occupiedSlots.push_back(slot);
		}
		if (!uniqueSlots.contains(set.primarySlot))
		{
			outError = "Equipment primary slot is not occupied: " + set.visualSetId;
			return false;
		}

		std::set<std::string> partIds;
		for (const DATA_JSON_VALUE& partValue : parts->Get_Array())
		{
			EQUIPMENT_PRESENTATION_PART part;
			if (!Parse_Part(partValue, part, outError) ||
				!partIds.insert(part.partId).second)
			{
				if (outError.empty())
					outError = "Equipment part ID is duplicated: " + set.visualSetId;
				return false;
			}
			if (LostArk::Shared::PLAYER_STANCE_ID::NONE != part.requiredStance &&
				LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER != set.classId)
			{
				outError = "Equipment stance is incompatible with its class: " +
					set.visualSetId;
				return false;
			}
			set.parts.push_back(std::move(part));
		}
		stagedSets.push_back(std::move(set));
	}

	m_Sets = std::move(stagedSets);
	outError.clear();
	return true;
}

const Client::EQUIPMENT_VISUAL_SET*
Client::CEquipmentPresentationCatalog::Find_Set(
	const std::string_view visualSetId) const
{
	for (const EQUIPMENT_VISUAL_SET& set : m_Sets)
	{
		if (set.visualSetId == visualSetId)
			return &set;
	}
	return nullptr;
}

std::vector<const Client::EQUIPMENT_VISUAL_SET*>
Client::CEquipmentPresentationCatalog::Find_ByClassAndPrimarySlot(
	const LostArk::Shared::CHARACTER_CLASS_ID classId,
	const EQUIPMENT_SLOT_ID slot) const
{
	std::vector<const EQUIPMENT_VISUAL_SET*> result;
	for (const EQUIPMENT_VISUAL_SET& set : m_Sets)
	{
		if (set.classId == classId && set.primarySlot == slot)
			result.push_back(&set);
	}
	return result;
}
