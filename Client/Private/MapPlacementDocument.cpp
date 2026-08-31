#include "MapPlacementDocument.h"

#include "MapAssetCatalog.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <unordered_set>

namespace
{
	constexpr const char* PLACEMENT_MAGIC = "LOSTARK_MAP_PLACEMENTS";
	constexpr uint32_t LEGACY_PLACEMENT_VERSION = 1;
	constexpr uint32_t PLACEMENT_VERSION = 2;

	bool_t CommitTemporaryFile(const std::filesystem::path& destination,
		const std::filesystem::path& temporary)
	{
		std::error_code existsError;
		if (std::filesystem::exists(destination, existsError) &&
			ReplaceFileW(destination.c_str(), temporary.c_str(), nullptr,
				REPLACEFILE_WRITE_THROUGH, nullptr, nullptr))
			return true;
		return MoveFileExW(temporary.c_str(), destination.c_str(),
			MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
	}

	bool_t NormalizeQuaternion(MAP_PLACEMENT_RECORD& record)
	{
		const vector_t value = XMLoadFloat4(&record.rotationQuaternion);
		const float length = XMVectorGetX(XMVector4Length(value));
		if (!std::isfinite(length) || length < 0.000001f)
			return false;
		/* A saved quaternion is parsed again during linked-save verification.
		   Re-normalizing an already-unit float quaternion is not bitwise
		   idempotent, so the second parse could differ by one ULP and reject a
		   valid save. Preserve values already inside the unit tolerance while
		   still canonicalizing genuinely non-unit authoring input. */
		constexpr float UNIT_QUATERNION_TOLERANCE = 0.0001f;
		vector_t normalized =
			std::abs(length - 1.f) <= UNIT_QUATERNION_TOLERANCE ?
			value : XMQuaternionNormalize(value);
		if (XMVectorGetW(normalized) < 0.f)
			normalized = XMVectorNegate(normalized);
		XMStoreFloat4(&record.rotationQuaternion, normalized);
		return true;
	}
}

bool_t CMapPlacementDocument::Read(
	const std::filesystem::path& path,
	const CMapAssetCatalog& catalog,
	std::vector<MAP_PLACEMENT_RECORD>& outRecords,
	std::string& outStatus)
{
	std::error_code existsError;
	if (!std::filesystem::exists(path, existsError))
	{
		outRecords.clear();
		outStatus = "No placement file; starting with an empty map";
		return true;
	}
	std::ifstream input(path, std::ios::binary);
	if (!input)
	{
		outStatus = "Could not open placement file: " + path.string();
		return false;
	}
	std::string magic;
	std::string areaId;
	uint32_t version = {};
	uint32_t count = {};
	if (!(input >> magic >> version >> std::quoted(areaId) >> count) ||
		magic != PLACEMENT_MAGIC ||
		(version != LEGACY_PLACEMENT_VERSION && version != PLACEMENT_VERSION) ||
		areaId != catalog.Get_AreaId() || count > MAX_PLACEMENT_COUNT)
	{
		outStatus = "Placement header is invalid or belongs to another area";
		return false;
	}

	std::vector<MAP_PLACEMENT_RECORD> staged;
	std::unordered_set<uint64_t> runtimeIds;
	std::unordered_set<std::string> sourceIds;
	staged.reserve(count);
	for (uint32_t index = 0; index < count; ++index)
	{
		MAP_PLACEMENT_RECORD record{};
		int32_t visible = {};
		if (version == LEGACY_PLACEMENT_VERSION)
		{
			float3_t legacyRotation{};
			if (!(input >> record.placementId >> std::quoted(record.assetId) >>
				record.position.x >> record.position.y >> record.position.z >>
				legacyRotation.x >> legacyRotation.y >> legacyRotation.z >>
				record.signedScale.x >> record.signedScale.y >>
				record.signedScale.z >> visible))
			{
				outStatus = "Legacy placement row is truncated at index " +
					std::to_string(index);
				return false;
			}
			record.sourcePlacementId = "legacy:" +
				std::to_string(record.placementId);
			record.sourceLevel = "LEGACY";
			record.transformSource = "legacy";
			XMStoreFloat4(&record.rotationQuaternion,
				XMQuaternionRotationRollPitchYaw(
					XMConvertToRadians(legacyRotation.x),
					XMConvertToRadians(legacyRotation.y),
					XMConvertToRadians(legacyRotation.z)));
		}
		else
		{
			if (!(input >> record.placementId >>
				std::quoted(record.sourcePlacementId) >>
				std::quoted(record.sourceLevel) >>
				std::quoted(record.transformSource) >>
				std::quoted(record.assetId) >>
				record.position.x >> record.position.y >> record.position.z >>
				record.rotationQuaternion.x >> record.rotationQuaternion.y >>
				record.rotationQuaternion.z >> record.rotationQuaternion.w >>
				record.signedScale.x >> record.signedScale.y >>
				record.signedScale.z >> visible))
			{
				outStatus = "Placement v2 row is truncated at index " +
					std::to_string(index);
				return false;
			}
		}
		record.visible = 0 != visible;
		if (!NormalizeQuaternion(record) || !Is_Valid(record, catalog) ||
			!runtimeIds.insert(record.placementId).second ||
			!sourceIds.insert(record.sourcePlacementId).second)
		{
			outStatus = "Placement validation failed at index " +
				std::to_string(index);
			return false;
		}
		staged.push_back(std::move(record));
	}
	std::string trailing;
	if (input >> trailing)
	{
		outStatus = "Placement file contains unexpected trailing data";
		return false;
	}
	outRecords = std::move(staged);
	outStatus = "Placement document v" + std::to_string(version) +
		" validated: " + std::to_string(outRecords.size());
	return true;
}

bool_t CMapPlacementDocument::Write(
	const std::filesystem::path& path,
	const std::string& areaId,
	const std::vector<MAP_PLACEMENT_RECORD>& records,
	const CMapAssetCatalog& catalog,
	std::string& outStatus,
	std::vector<MAP_PLACEMENT_RECORD>* outStoredRecords)
{
	if (records.size() > MAX_PLACEMENT_COUNT || areaId != catalog.Get_AreaId())
	{
		outStatus = "Placement save header is invalid";
		return false;
	}
	std::vector<MAP_PLACEMENT_RECORD> normalized = records;
	std::unordered_set<uint64_t> runtimeIds;
	std::unordered_set<std::string> sourceIds;
	for (MAP_PLACEMENT_RECORD& record : normalized)
	{
		if (!NormalizeQuaternion(record) || !Is_Valid(record, catalog) ||
			!runtimeIds.insert(record.placementId).second ||
			!sourceIds.insert(record.sourcePlacementId).second)
		{
			outStatus = "Save aborted: duplicate ID or invalid placement";
			return false;
		}
	}
	std::error_code directoryError;
	std::filesystem::create_directories(path.parent_path(), directoryError);
	if (directoryError)
	{
		outStatus = "Could not create placement directory";
		return false;
	}
	const std::filesystem::path temporary = path.wstring() + L".tmp";
	std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
	if (!output)
	{
		outStatus = "Could not create temporary placement file";
		return false;
	}
	output << PLACEMENT_MAGIC << ' ' << PLACEMENT_VERSION << ' ' <<
		std::quoted(areaId) << ' ' << normalized.size() << '\n';
	output << std::setprecision(9);
	for (const MAP_PLACEMENT_RECORD& record : normalized)
	{
		output << record.placementId << ' ' <<
			std::quoted(record.sourcePlacementId) << ' ' <<
			std::quoted(record.sourceLevel) << ' ' <<
			std::quoted(record.transformSource) << ' ' <<
			std::quoted(record.assetId) << ' ' <<
			record.position.x << ' ' << record.position.y << ' ' <<
			record.position.z << ' ' <<
			record.rotationQuaternion.x << ' ' <<
			record.rotationQuaternion.y << ' ' <<
			record.rotationQuaternion.z << ' ' <<
			record.rotationQuaternion.w << ' ' <<
			record.signedScale.x << ' ' << record.signedScale.y << ' ' <<
			record.signedScale.z << ' ' << (record.visible ? 1 : 0) << '\n';
	}
	output.flush();
	const bool_t wroteSuccessfully = output.good();
	output.close();
	if (!wroteSuccessfully || !CommitTemporaryFile(path, temporary))
	{
		std::error_code removeError;
		std::filesystem::remove(temporary, removeError);
		outStatus = "Failed to commit placement file atomically";
		return false;
	}
	outStatus = "Saved placement document v2: " +
		std::to_string(normalized.size());
	if (nullptr != outStoredRecords)
		*outStoredRecords = std::move(normalized);
	return true;
}

bool_t CMapPlacementDocument::Is_Valid(
	const MAP_PLACEMENT_RECORD& record,
	const CMapAssetCatalog& catalog)
{
	const auto finite3 = [](const float3_t& value)
	{
		return std::isfinite(value.x) && std::isfinite(value.y) &&
			std::isfinite(value.z);
	};
	const auto validToken = [](const std::string& value, size_t maximum)
	{
		return !value.empty() && value.size() <= maximum &&
			std::all_of(value.begin(), value.end(), [](unsigned char character)
			{
				return 0 != std::isalnum(character) || character == '_' ||
					character == '-' || character == '.';
			});
	};
	const auto validSourceId = [](const std::string& value)
	{
		return !value.empty() && value.size() <= 256 &&
			std::none_of(value.begin(), value.end(), [](unsigned char character)
			{
				return 0 != std::iscntrl(character);
			});
	};
	const float quaternionLength = std::sqrt(
		record.rotationQuaternion.x * record.rotationQuaternion.x +
		record.rotationQuaternion.y * record.rotationQuaternion.y +
		record.rotationQuaternion.z * record.rotationQuaternion.z +
		record.rotationQuaternion.w * record.rotationQuaternion.w);
	const bool_t importedSource =
		record.transformSource == "actor" ||
		record.transformSource == "component";
	const bool_t editorSource =
		record.transformSource == "editor" ||
		record.transformSource == "legacy" ||
		record.transformSource == "overlay";
	const bool_t validIdDomain =
		(importedSource && 0 != (record.placementId & 0x8000000000000000ull)) ||
		(editorSource && record.placementId <= MAX_EDITOR_PLACEMENT_ID);
	return 0 != record.placementId &&
		validSourceId(record.sourcePlacementId) &&
		validToken(record.sourceLevel, 128) &&
		validToken(record.transformSource, 32) && validIdDomain &&
		!record.assetId.empty() && nullptr != catalog.Find(record.assetId) &&
		finite3(record.position) && finite3(record.signedScale) &&
		std::isfinite(record.rotationQuaternion.x) &&
		std::isfinite(record.rotationQuaternion.y) &&
		std::isfinite(record.rotationQuaternion.z) &&
		std::isfinite(record.rotationQuaternion.w) &&
		quaternionLength >= 0.000001f &&
		std::abs(record.signedScale.x) >= 0.000001f &&
		std::abs(record.signedScale.y) >= 0.000001f &&
		std::abs(record.signedScale.z) >= 0.000001f;
}
