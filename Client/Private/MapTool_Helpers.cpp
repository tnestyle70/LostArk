#include "imgui.h"
#include "MapTool_Internal.h"
#include "DataJson.h"
#include "MapStaticBatchObject.h"
#include "DestructionSimulationController.h"
#include "Model.h"
#include "ProjectDataRoot.h"
#include "Gameplay/WorldCollisionContract.h"
#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <limits>
#include <map>
#include <sstream>
#include <system_error>
#include <unordered_map>
#include <unordered_set>


namespace MapToolDetail
{

	/* Every march instance whose id starts with that prefix, in document
	   order. One wave is a rank across all nine corridor lanes. */
	std::vector<std::string> Collect_CardMiroMarchInstanceIds(
		const CWorldSequenceDocument& document)
	{
		const size_t prefixLength = strlen(CARDMIRO_MARCH_INSTANCE_PREFIX);
		std::vector<std::string> ids;
		for (const WORLD_SEQUENCE_INSTANCE& instance : document.Get_Instances())
		{
			if (instance.instanceId.size() > prefixLength &&
				0 == instance.instanceId.compare(0, prefixLength,
					CARDMIRO_MARCH_INSTANCE_PREFIX))
			{
				ids.push_back(instance.instanceId);
			}
		}
		return ids;
	}

	bool_t PrepareAuthoringBackup(
		const std::filesystem::path& destination,
		AUTHORING_FILE_BACKUP& outBackup,
		std::string& outStatus)
	{
		if (destination.empty())
		{
			outStatus = "Linked authoring save has an empty destination";
			return false;
		}
		outBackup = {};
		outBackup.destination = destination;
		outBackup.backup = destination;
		outBackup.backup += L".world-sequence-transaction.bak";
		std::error_code error;
		const bool_t destinationExists =
			std::filesystem::exists(destination, error);
		if (error)
		{
			outStatus = "Could not inspect linked authoring destination";
			return false;
		}
		outBackup.hadOriginal = destinationExists &&
			std::filesystem::is_regular_file(destination, error);
		if (error || (destinationExists && !outBackup.hadOriginal))
		{
			outStatus = "Linked authoring destination is not a regular file";
			return false;
		}
		if (!outBackup.hadOriginal)
		{
			std::filesystem::remove(outBackup.backup, error);
			if (error)
			{
				outStatus = "Could not clear stale linked authoring backup";
				return false;
			}
			return true;
		}
		std::filesystem::copy_file(destination, outBackup.backup,
			std::filesystem::copy_options::overwrite_existing, error);
		if (error)
		{
			outStatus = "Could not create linked authoring rollback backup";
			return false;
		}
		return true;
	}

	bool_t RestoreAuthoringBackup(const AUTHORING_FILE_BACKUP& backup)
	{
		if (backup.hadOriginal)
		{
			std::error_code error;
			if (!std::filesystem::is_regular_file(backup.backup, error) || error)
				return false;
			std::filesystem::path restore = backup.destination;
			restore += L".world-sequence-restore.tmp";
			std::filesystem::copy_file(backup.backup, restore,
				std::filesystem::copy_options::overwrite_existing, error);
			if (error)
				return false;
			const bool_t committed =
				ReplaceFileW(backup.destination.c_str(), restore.c_str(), nullptr,
					REPLACEFILE_WRITE_THROUGH, nullptr, nullptr) ||
				MoveFileExW(restore.c_str(), backup.destination.c_str(),
					MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
			if (!committed)
				std::filesystem::remove(restore, error);
			return committed;
		}
		std::error_code error;
		const bool_t destinationExists =
			std::filesystem::exists(backup.destination, error);
		if (error || (destinationExists &&
			!std::filesystem::is_regular_file(backup.destination, error)) || error)
		{
			return false;
		}
		if (destinationExists)
			std::filesystem::remove(backup.destination, error);
		return !error;
	}

	void DiscardAuthoringBackup(const AUTHORING_FILE_BACKUP& backup)
	{
		std::error_code error;
		std::filesystem::remove(backup.backup, error);
	}

	std::filesystem::path GetAuthoringTransactionMarker(
		const std::filesystem::path& sequencePath)
	{
		std::filesystem::path marker = sequencePath;
		marker += L".world-sequence-transaction.pending";
		return marker;
	}

	bool_t WriteAuthoringTransactionMarker(
		const AUTHORING_FILE_BACKUP& placementBackup,
		const AUTHORING_FILE_BACKUP& sequenceBackup,
		std::string& outStatus)
	{
		const std::filesystem::path marker =
			GetAuthoringTransactionMarker(sequenceBackup.destination);
		std::filesystem::path temporary = marker;
		temporary += L".tmp";
		std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
		if (!output)
		{
			outStatus = "Could not create linked authoring transaction marker";
			return false;
		}
		output << "lostark-world-sequence-transaction-v1\n"
			<< "mapHadOriginal=" << (placementBackup.hadOriginal ? 1 : 0) << "\n"
			<< "sequenceHadOriginal=" << (sequenceBackup.hadOriginal ? 1 : 0) << "\n";
		output.flush();
		bool_t written = output.good();
		output.close();
		written = written && !output.fail();
		if (!written || !MoveFileExW(temporary.c_str(), marker.c_str(),
			MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
		{
			std::error_code error;
			std::filesystem::remove(temporary, error);
			outStatus = "Could not commit linked authoring transaction marker";
			return false;
		}
		return true;
	}

	bool_t ClearAuthoringTransactionMarker(
		const std::filesystem::path& sequencePath)
	{
		std::error_code error;
		const bool_t removed = std::filesystem::remove(
			GetAuthoringTransactionMarker(sequencePath), error);
		return removed && !error;
	}

	bool_t RecoverAuthoringTransactionUnderLock(
		const std::filesystem::path& placementPath,
		const std::filesystem::path& sequencePath,
		std::string& outStatus)
	{
		const std::filesystem::path marker =
			GetAuthoringTransactionMarker(sequencePath);
		std::error_code error;
		if (!std::filesystem::exists(marker, error))
		{
			if (error)
			{
				outStatus = "Could not inspect linked authoring transaction marker";
				return false;
			}
			return true;
		}
		if (!std::filesystem::is_regular_file(marker, error) || error)
		{
			outStatus = "Linked authoring transaction marker is invalid";
			return false;
		}
		constexpr uintmax_t MAX_TRANSACTION_MARKER_BYTES = 512u;
		const uintmax_t markerBytes = std::filesystem::file_size(marker, error);
		if (error || markerBytes > MAX_TRANSACTION_MARKER_BYTES)
		{
			outStatus = error ?
				"Could not inspect linked authoring transaction marker size" :
				"Linked authoring transaction marker exceeds its bounded read limit";
			return false;
		}
		std::ifstream input(marker, std::ios::binary);
		std::string markerText(static_cast<size_t>(markerBytes), '\0');
		if (!markerText.empty())
			input.read(markerText.data(),
				static_cast<std::streamsize>(markerText.size()));
		if (!input || input.bad() ||
			input.gcount() != static_cast<std::streamsize>(markerText.size()) ||
			std::char_traits<char_t>::eof() != input.peek())
		{
			outStatus =
				"Linked authoring transaction marker changed or failed while reading";
			return false;
		}
		std::istringstream markerInput(markerText);
		std::string header;
		std::string mapFlag;
		std::string sequenceFlag;
		std::string extra;
		if (!std::getline(markerInput, header) ||
			!std::getline(markerInput, mapFlag) ||
			!std::getline(markerInput, sequenceFlag) ||
			std::getline(markerInput, extra) ||
			header != "lostark-world-sequence-transaction-v1" ||
			(mapFlag != "mapHadOriginal=0" && mapFlag != "mapHadOriginal=1") ||
			(sequenceFlag != "sequenceHadOriginal=0" &&
				sequenceFlag != "sequenceHadOriginal=1"))
		{
			outStatus = "Linked authoring transaction marker content is invalid";
			return false;
		}
		AUTHORING_FILE_BACKUP placementBackup;
		placementBackup.destination = placementPath;
		placementBackup.backup = placementPath;
		placementBackup.backup += L".world-sequence-transaction.bak";
		placementBackup.hadOriginal = mapFlag.back() == '1';
		AUTHORING_FILE_BACKUP sequenceBackup;
		sequenceBackup.destination = sequencePath;
		sequenceBackup.backup = sequencePath;
		sequenceBackup.backup += L".world-sequence-transaction.bak";
		sequenceBackup.hadOriginal = sequenceFlag.back() == '1';
		const bool_t mapRestored = RestoreAuthoringBackup(placementBackup);
		const bool_t sequenceRestored = RestoreAuthoringBackup(sequenceBackup);
		if (!mapRestored || !sequenceRestored)
		{
			outStatus = "Linked authoring recovery is blocked; preserve backups " +
				placementBackup.backup.filename().string() + " and " +
				sequenceBackup.backup.filename().string();
			return false;
		}
		if (!ClearAuthoringTransactionMarker(sequencePath))
		{
			outStatus = "Linked authoring recovery restored files but could not clear marker";
			return false;
		}
		DiscardAuthoringBackup(placementBackup);
		DiscardAuthoringBackup(sequenceBackup);
		outStatus = "Recovered an interrupted linked map/sequence save";
		return true;
	}

	bool_t AreExactlySamePlacementRecords(
		const std::vector<MAP_PLACEMENT_RECORD>& expected,
		const std::vector<MAP_PLACEMENT_RECORD>& actual)
	{
		if (expected.size() != actual.size())
			return false;
		const auto sameFloat = [](const f32_t left, const f32_t right)
		{
			return left == right;
		};
		for (size_t index = 0u; index < expected.size(); ++index)
		{
			const MAP_PLACEMENT_RECORD& left = expected[index];
			const MAP_PLACEMENT_RECORD& right = actual[index];
			if (left.placementId != right.placementId ||
				left.sourcePlacementId != right.sourcePlacementId ||
				left.sourceLevel != right.sourceLevel ||
				left.transformSource != right.transformSource ||
				left.assetId != right.assetId || left.visible != right.visible ||
				!sameFloat(left.position.x, right.position.x) ||
				!sameFloat(left.position.y, right.position.y) ||
				!sameFloat(left.position.z, right.position.z) ||
				!sameFloat(left.rotationQuaternion.x,
					right.rotationQuaternion.x) ||
				!sameFloat(left.rotationQuaternion.y,
					right.rotationQuaternion.y) ||
				!sameFloat(left.rotationQuaternion.z,
					right.rotationQuaternion.z) ||
				!sameFloat(left.rotationQuaternion.w,
					right.rotationQuaternion.w) ||
				!sameFloat(left.signedScale.x, right.signedScale.x) ||
				!sameFloat(left.signedScale.y, right.signedScale.y) ||
				!sameFloat(left.signedScale.z, right.signedScale.z))
			{
				return false;
			}
		}
		return true;
	}

	bool_t NpcBatchBodiesOverlapVertically(
		const f32_t leftGroundY,
		const f32_t rightGroundY)
	{
		using namespace LostArk::Shared::WorldCollision;
		const f32_t leftCenterY = leftGroundY + PLAYER_CENTER_OFFSET_Y;
		const f32_t rightCenterY = rightGroundY + PLAYER_CENTER_OFFSET_Y;
		return std::abs(leftCenterY - rightCenterY) <=
			PLAYER_HALF_EXTENT_Y * 2.f;
	}

	bool_t NpcBatchBodyOverlapsCollisionHeight(
		const f32_t groundY,
		const WORLD_GAMEPLAY_PLACEMENT& collisionBox)
	{
		using namespace LostArk::Shared::WorldCollision;
		const f32_t centerY = groundY + PLAYER_CENTER_OFFSET_Y;
		return std::abs(centerY - collisionBox.position.y) <=
			collisionBox.halfExtents.y + PLAYER_HALF_EXTENT_Y;
	}

	bool_t IsFinite(const float3_t& value)
	{
		return std::isfinite(value.x) && std::isfinite(value.y) &&
			std::isfinite(value.z);
	}

	bool_t IsFiniteQuaternion(const float4_t& value)
	{
		return std::isfinite(value.x) && std::isfinite(value.y) &&
			std::isfinite(value.z) && std::isfinite(value.w);
	}

	/* Animated Prop authoring edits a whole placement pose in degrees and
	   stores the normalized quaternion the Deploy placement row owns. The
	   World Sequence panel keeps its own equivalent for keyframe offsets. */
	float3_t DeployQuaternionToEulerDegrees(const float4_t& value)
	{
		/* Exact inverse of XMQuaternionRotationRollPitchYaw (M = Mz*Mx*My,
		   row-vector). Yaw and roll use atan2 so Y keeps the full +-180
		   range; only pitch stays asin-limited to +-90. */
		constexpr f32_t radiansToDegrees = 180.f / DirectX::XM_PI;
		float4x4_t rotation{};
		XMStoreFloat4x4(&rotation, XMMatrixRotationQuaternion(
			XMLoadFloat4(&value)));
		const f32_t sinPitch =
			(std::max)(-1.f, (std::min)(1.f, -rotation._32));
		const f32_t pitch = std::asin(sinPitch);
		f32_t yaw = 0.f;
		f32_t roll = 0.f;
		if (std::abs(sinPitch) < 0.99999f)
		{
			yaw = std::atan2(rotation._31, rotation._33);
			roll = std::atan2(rotation._12, rotation._22);
		}
		else
		{
			yaw = std::atan2(-rotation._13, rotation._11);
		}
		return float3_t(pitch * radiansToDegrees, yaw * radiansToDegrees,
			roll * radiansToDegrees);
	}

	float4_t DeployEulerDegreesToQuaternion(const float3_t& value)
	{
		constexpr f32_t degreesToRadians = DirectX::XM_PI / 180.f;
		vector_t quaternion = XMQuaternionRotationRollPitchYaw(
			value.x * degreesToRadians,
			value.y * degreesToRadians,
			value.z * degreesToRadians);
		quaternion = XMQuaternionNormalize(quaternion);
		if (XMVectorGetW(quaternion) < 0.f)
			quaternion = XMVectorNegate(quaternion);
		float4_t result{};
		XMStoreFloat4(&result, quaternion);
		return result;
	}

	void ShowAuthoringHelp(const char_t* text)
	{
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
			ImGui::SetTooltip("%s", text);
	}

	bool_t MatchesAnimatedPropFilter(
		const std::string& label,
		const std::string& assetId,
		const char_t* filter)
	{
		if (nullptr == filter || '\0' == *filter)
			return true;
		const auto lowered = [](std::string value)
		{
			std::transform(value.begin(), value.end(), value.begin(),
				[](const unsigned char character)
				{
					return static_cast<char_t>(std::tolower(character));
				});
			return value;
		};
		const std::string needle = lowered(filter);
		return std::string::npos != lowered(label).find(needle) ||
			std::string::npos != lowered(assetId).find(needle);
	}

	std::string Editor_AreaShortName(const std::string& areaId)
	{
		if ("LV_BER_BERNCASTLE" == areaId) return "bern";
		if ("LV_LUT_HEARTRB_ED" == areaId) return "valtan";
		if ("LV_DEV_TRAINING_GROUND" == areaId) return "training";
		if ("LV_LOBBY_CLASSSELECT_SL00" == areaId) return "character-select";
		std::string lowered;
		for (const char character : areaId)
		{
			lowered += static_cast<char>(std::tolower(
				static_cast<unsigned char>(character)));
		}
		return lowered;
	}

	std::string Editor_NpcArchetypeToken(const std::string& archetypeId)
	{
		std::string token;
		token.reserve(archetypeId.size());
		for (const char_t character : archetypeId)
		{
			token += static_cast<char_t>(std::tolower(
				static_cast<unsigned char>(character)));
		}
		if (0 == token.rfind("npc_", 0))
			token.erase(0, 4);
		return token;
	}

	uint32_t Editor_StableSeed(const std::string& value)
	{
		uint32_t hash = 2166136261u;
		for (const unsigned char character : value)
		{
			hash ^= character;
			hash *= 16777619u;
		}
		return 0u == hash ? 1u : hash;
	}

	bool_t TryBuildCentralEditorFrame(
		const vector<float3_t>& positions,
		float3_t& outCenter,
		f32_t& outRadius)
	{
		vector<f32_t> xValues;
		vector<f32_t> yValues;
		vector<f32_t> zValues;
		xValues.reserve(positions.size());
		yValues.reserve(positions.size());
		zValues.reserve(positions.size());
		for (const float3_t& position : positions)
		{
			if (!IsFinite(position))
				continue;
			xValues.push_back(position.x);
			yValues.push_back(position.y);
			zValues.push_back(position.z);
		}
		if (xValues.empty())
			return false;

		std::sort(xValues.begin(), xValues.end());
		std::sort(yValues.begin(), yValues.end());
		std::sort(zValues.begin(), zValues.end());
		const auto sampleQuantile = [](const vector<f32_t>& values, f32_t quantile)
		{
			const size_t index = static_cast<size_t>(
				(values.size() - 1) * quantile + 0.5f);
			return values[index];
		};

		const float3_t center(
			sampleQuantile(xValues, 0.50f),
			sampleQuantile(yValues, 0.50f),
			sampleQuantile(zValues, 0.50f));
		const f32_t centralSpanX =
			sampleQuantile(xValues, 0.95f) - sampleQuantile(xValues, 0.05f);
		const f32_t centralSpanZ =
			sampleQuantile(zValues, 0.95f) - sampleQuantile(zValues, 0.05f);
		const f32_t radius = (std::max)(75.f,
			(std::max)(centralSpanX, centralSpanZ) * 0.35f);
		if (!IsFinite(center) || !std::isfinite(radius) || radius <= 0.f)
			return false;

		outCenter = center;
		outRadius = radius;
		return true;
	}

	/* Frames every enabled player spawn the Area declares. It used to take a
	preferred placement id, but no authored document has ever carried the two
	ids the caller asked for, so that lookup always fell through to whichever
	spawn happened to come first and the named branches were dead weight. The
	spawn set is the stable authoring focus: it is what the Area declares as
	the playable entry, and it cannot be dragged away by backdrop meshes. */
	bool_t TryBuildGameplaySpawnFrame(
		const CWorldGameplayDocument& document,
		float3_t& outCenter,
		f32_t& outRadius)
	{
		float3_t minimum{};
		float3_t maximum{};
		bool_t hasSpawn = false;
		for (const WORLD_GAMEPLAY_PLACEMENT& placement :
			document.Get_Placements())
		{
			if (WORLD_PLACEMENT_KIND::PLAYER_SPAWN != placement.eKind ||
				!placement.isEnabled || !IsFinite(placement.position))
			{
				continue;
			}
			if (!hasSpawn)
			{
				minimum = placement.position;
				maximum = placement.position;
				hasSpawn = true;
				continue;
			}
			minimum.x = (std::min)(minimum.x, placement.position.x);
			minimum.y = (std::min)(minimum.y, placement.position.y);
			minimum.z = (std::min)(minimum.z, placement.position.z);
			maximum.x = (std::max)(maximum.x, placement.position.x);
			maximum.y = (std::max)(maximum.y, placement.position.y);
			maximum.z = (std::max)(maximum.z, placement.position.z);
		}
		if (!hasSpawn)
			return false;

		outCenter = float3_t(
			(minimum.x + maximum.x) * 0.5f,
			(minimum.y + maximum.y) * 0.5f,
			(minimum.z + maximum.z) * 0.5f);
		outRadius = (std::max)(35.f,
			(std::max)(maximum.x - minimum.x, maximum.z - minimum.z) * 0.75f);
		return true;
	}

	bool_t IsBernLandscapePlacement(
		const CMapAssetCatalog& catalog,
		const MAP_PLACEMENT_RECORD& record)
	{
		if ("LV_BER_BERNCASTLE" != catalog.Get_AreaId())
			return false;

		const MAP_ASSET_ENTRY* asset = catalog.Find(record.assetId);
		return nullptr != asset && asset->groupId == "landscape";
	}

	bool_t IsBatchEligible(const MAP_ASSET_ENTRY& asset)
	{
		return CMapPlacementRuntime::Is_BatchEligible(asset);
	}

	HRESULT BuildStaticInstance(
		const MAP_ASSET_ENTRY& asset,
		const shared_ptr<CModel>& model,
		const MAP_PLACEMENT_RECORD& record,
		FMapStaticInstance& outInstance)
	{
		return CMapPlacementRuntime::Build_StaticInstance(
			asset, model, record, outInstance);
	}

	bool_t MatchesFilter(const std::string& text, const char* pFilter)
	{
		if (nullptr == pFilter || '\0' == *pFilter)
			return true;

		std::string haystack = text;
		std::string needle = pFilter;
		std::transform(haystack.begin(), haystack.end(), haystack.begin(),
			[](unsigned char value) { return static_cast<char>(std::tolower(value)); });
		std::transform(needle.begin(), needle.end(), needle.begin(),
			[](unsigned char value) { return static_cast<char>(std::tolower(value)); });
		return std::string::npos != haystack.find(needle);
	}

	uint64_t HashStableAuthoringId(const std::string& value)
	{
		uint64_t hash = 14695981039346656037ull;
		for (const unsigned char character : value)
		{
			hash ^= static_cast<uint64_t>(character);
			hash *= 1099511628211ull;
		}
		return hash;
	}

	std::string ToStableHex(const uint64_t value)
	{
		std::ostringstream output;
		output << std::hex << std::setfill('0') << std::setw(16) << value;
		return output.str();
	}

	bool_t IsSameNavigationFloat(f32_t left, f32_t right)
	{
		return std::fabs(left - right) <= 0.000001f;
	}

	bool_t HasSameNavigationGridIdentity(
		const NAVGRID_AUTHORING_DESC& left,
		const NAVGRID_AUTHORING_DESC& right)
	{
		return left.areaId == right.areaId &&
			left.width == right.width &&
			left.height == right.height &&
			IsSameNavigationFloat(left.cellSize, right.cellSize) &&
			IsSameNavigationFloat(left.originX, right.originX) &&
			IsSameNavigationFloat(left.originZ, right.originZ);
	}

	bool_t HasSameNavigationPath(
		const std::filesystem::path& left,
		const std::filesystem::path& right)
	{
		return left.lexically_normal() == right.lexically_normal();
	}

	bool_t ReadTextFile(
		const std::filesystem::path& path,
		std::string& outText)
	{
		std::ifstream input(path, std::ios::binary);
		if (!input)
			return false;
		outText.assign(
			std::istreambuf_iterator<char>(input),
			std::istreambuf_iterator<char>());
		return input.good() || input.eof();
	}

	bool_t ReadRequiredString(
		const Client::DATA_JSON_VALUE& object,
		const char_t* pName,
		std::string& outValue)
	{
		const Client::DATA_JSON_VALUE* value = object.Find(pName);
		if (nullptr == value || !value->Is_String() ||
			value->Get_String().empty())
		{
			return false;
		}
		outValue = value->Get_String();
		return true;
	}

	/* is_regular_file reports a missing file through the error_code, so a
	   bare "if (error)" cannot separate an absent optional document from a
	   real inspection failure. Only the latter may abort a load. */
	bool_t IsFileInspectionFailure(const std::error_code& error)
	{
		return error && error != std::errc::no_such_file_or_directory;
	}

	std::filesystem::path ResolveDataCatalogPath(
		const std::string& value)
	{
		const std::filesystem::path serialized(value);
		if (serialized.empty() || serialized.is_absolute() ||
			serialized.has_root_path())
		{
			return {};
		}
		auto part = serialized.begin();
		if (part == serialized.end() || *part != L"Data")
			return {};
		std::filesystem::path relative;
		for (++part; part != serialized.end(); ++part)
			relative /= *part;
		return CProjectDataRoot::Resolve(relative);
	}

	const char_t* SimulationPlaybackStateLabel(
		const DESTRUCTION_SIMULATION_PLAYBACK_STATE state)
	{
		switch (state)
		{
		case DESTRUCTION_SIMULATION_PLAYBACK_STATE::STOPPED:
			return "STOPPED";
		case DESTRUCTION_SIMULATION_PLAYBACK_STATE::PLAYING:
			return "PLAYING";
		case DESTRUCTION_SIMULATION_PLAYBACK_STATE::PAUSED:
			return "PAUSED";
		case DESTRUCTION_SIMULATION_PLAYBACK_STATE::FINISHED:
			return "FINISHED";
		case DESTRUCTION_SIMULATION_PLAYBACK_STATE::END:
		default:
			return "INVALID";
		}
	}

	const char_t* SimulationElementStateLabel(
		const DESTRUCTION_SIMULATION_ELEMENT_STATE state)
	{
		switch (state)
		{
		case DESTRUCTION_SIMULATION_ELEMENT_STATE::WAITING:
			return "WAITING";
		case DESTRUCTION_SIMULATION_ELEMENT_STATE::ACTIVE:
			return "ACTIVE";
		case DESTRUCTION_SIMULATION_ELEMENT_STATE::EXPIRED:
			return "EXPIRED";
		case DESTRUCTION_SIMULATION_ELEMENT_STATE::FILTERED:
			return "FILTERED";
		case DESTRUCTION_SIMULATION_ELEMENT_STATE::END:
		default:
			return "INVALID";
		}
	}

	/* Two co-located Deploy walls that share every authored simulation value are
	   one wall to the author. They stay two document elements because an element
	   owns exactly one source placement, but the outliner, the authoring panel and
	   the Solo scope treat them as a single emitter. Profiles whose emitters were
	   tuned apart (the 3705102 wall ring) stay split, so a linked Apply can never
	   overwrite a direction the author set per wall. */
	bool_t HasSharedEmitterAuthoring(
		const Client::DESTRUCTION_SIMULATION_ELEMENT& left,
		const Client::DESTRUCTION_SIMULATION_ELEMENT& right)
	{
		return left.vSpawnOffset.x == right.vSpawnOffset.x &&
			left.vSpawnOffset.y == right.vSpawnOffset.y &&
			left.vSpawnOffset.z == right.vSpawnOffset.z &&
			left.vDirection.x == right.vDirection.x &&
			left.vDirection.y == right.vDirection.y &&
			left.vDirection.z == right.vDirection.z &&
			left.fSpeedMetersPerSecond == right.fSpeedMetersPerSecond &&
			left.fGravityScale == right.fGravityScale &&
			left.fLifetimeSeconds == right.fLifetimeSeconds &&
			left.Trigger.eKind == right.Trigger.eKind &&
			left.Trigger.fTimeSeconds == right.Trigger.fTimeSeconds &&
			left.Trigger.receiverCollisionId ==
				right.Trigger.receiverCollisionId;
	}

	bool_t AreEmittersAuthoredAsOneWall(
		const Client::DESTRUCTION_SIMULATION_PROFILE& profile)
	{
		if (2u > profile.Elements.size())
			return false;
		for (size_t index = 1u; index < profile.Elements.size(); ++index)
		{
			if (!HasSharedEmitterAuthoring(
				profile.Elements[0], profile.Elements[index]))
			{
				return false;
			}
		}
		return true;
	}

	const char_t* SimulationScopeLabel(
		const DESTRUCTION_SIMULATION_SCOPE scope)
	{
		switch (scope)
		{
		case DESTRUCTION_SIMULATION_SCOPE::ALL_DEBRIS:
			return "ALL FRAGMENTS";
		case DESTRUCTION_SIMULATION_SCOPE::SOLO_SELECTED:
			return "SOLO EMITTER";
		case DESTRUCTION_SIMULATION_SCOPE::SOLO_FRAGMENT:
			return "SOLO FRAGMENT";
		case DESTRUCTION_SIMULATION_SCOPE::END:
		default:
			return "INVALID";
		}
	}

	bool_t NormalizeSimulationDirection(float3_t& direction)
	{
		const f32_t lengthSquared = direction.x * direction.x +
			direction.y * direction.y + direction.z * direction.z;
		if (!std::isfinite(lengthSquared) || lengthSquared <= 0.000001f)
			return false;
		const f32_t inverseLength = 1.f / std::sqrt(lengthSquared);
		direction.x *= inverseLength;
		direction.y *= inverseLength;
		direction.z *= inverseLength;
		return true;
	}
}
