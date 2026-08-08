#include "DestructionSimulationDocument.h"

#include "DataJson.h"
#ifndef DESTRUCTION_SIMULATION_CODEC_ONLY
#include "DeployPropObject.h"
#include "DeployPropRuntime.h"
#endif

#include <algorithm>
#include <atomic>
#include <cfloat>
#include <charconv>
#include <cmath>
#include <cwchar>
#include <fstream>
#include <iomanip>
#include <initializer_list>
#include <limits>
#include <sstream>
#include <unordered_set>

namespace
{
	using namespace Client;

	constexpr const char_t* SCHEMA = "lostark.destruction-simulation";
	constexpr uint32_t FORMAT_VERSION = 1u;
	constexpr f32_t DIRECTION_LENGTH_TOLERANCE = 0.001f;

	bool_t Read_TextFile(
		const std::filesystem::path& path,
		std::string& outText)
	{
		std::ifstream input(path, std::ios::binary);
		if (!input.is_open())
			return false;
		std::ostringstream buffer;
		buffer << input.rdbuf();
		if (input.bad())
			return false;
		outText = buffer.str();
		return true;
	}

	bool_t Is_ExactObject(
		const DATA_JSON_VALUE& value,
		const std::initializer_list<const char_t*> keys)
	{
		if (!value.Is_Object() || value.Get_Object().size() != keys.size())
			return false;
		return std::all_of(keys.begin(), keys.end(),
			[&value](const char_t* key)
			{
				return nullptr != value.Find(key);
			});
	}

	bool_t Read_String(
		const DATA_JSON_VALUE& parent,
		const char_t* key,
		const bool_t allowEmpty,
		std::string& outValue)
	{
		const DATA_JSON_VALUE* value = parent.Find(key);
		if (nullptr == value || !value->Is_String() ||
			(!allowEmpty && value->Get_String().empty()))
		{
			return false;
		}
		outValue = value->Get_String();
		return true;
	}

	bool_t Read_Number(
		const DATA_JSON_VALUE& parent,
		const char_t* key,
		f32_t& outValue)
	{
		const DATA_JSON_VALUE* value = parent.Find(key);
		if (nullptr == value || !value->Is_Number() ||
			!std::isfinite(value->Get_Number()) ||
			value->Get_Number() < -static_cast<double>(FLT_MAX) ||
			value->Get_Number() > static_cast<double>(FLT_MAX))
		{
			return false;
		}
		outValue = static_cast<f32_t>(value->Get_Number());
		return std::isfinite(outValue);
	}

	bool_t Read_Bool(
		const DATA_JSON_VALUE& parent,
		const char_t* key,
		bool_t& outValue)
	{
		const DATA_JSON_VALUE* value = parent.Find(key);
		if (nullptr == value || !value->Is_Boolean())
			return false;
		outValue = value->Get_Boolean();
		return true;
	}

	bool_t Read_Float2(
		const DATA_JSON_VALUE& parent,
		const char_t* key,
		float2_t& outValue)
	{
		const DATA_JSON_VALUE* value = parent.Find(key);
		if (nullptr == value || !value->Is_Array() ||
			2u != value->Get_Array().size())
		{
			return false;
		}
		const DATA_JSON_VALUE& x = value->Get_Array()[0u];
		const DATA_JSON_VALUE& y = value->Get_Array()[1u];
		if (!x.Is_Number() || !y.Is_Number() ||
			!std::isfinite(x.Get_Number()) || !std::isfinite(y.Get_Number()) ||
			x.Get_Number() < -static_cast<double>(FLT_MAX) ||
			x.Get_Number() > static_cast<double>(FLT_MAX) ||
			y.Get_Number() < -static_cast<double>(FLT_MAX) ||
			y.Get_Number() > static_cast<double>(FLT_MAX))
		{
			return false;
		}
		outValue = {
			static_cast<f32_t>(x.Get_Number()),
			static_cast<f32_t>(y.Get_Number())
		};
		return std::isfinite(outValue.x) && std::isfinite(outValue.y);
	}

	bool_t Read_Float3(
		const DATA_JSON_VALUE& parent,
		const char_t* key,
		float3_t& outValue)
	{
		const DATA_JSON_VALUE* value = parent.Find(key);
		if (nullptr == value || !value->Is_Array() ||
			3u != value->Get_Array().size())
		{
			return false;
		}
		for (const DATA_JSON_VALUE& component : value->Get_Array())
		{
			if (!component.Is_Number() ||
				!std::isfinite(component.Get_Number()) ||
				component.Get_Number() < -static_cast<double>(FLT_MAX) ||
				component.Get_Number() > static_cast<double>(FLT_MAX))
			{
				return false;
			}
		}
		outValue = {
			static_cast<f32_t>(value->Get_Array()[0u].Get_Number()),
			static_cast<f32_t>(value->Get_Array()[1u].Get_Number()),
			static_cast<f32_t>(value->Get_Array()[2u].Get_Number())
		};
		return std::isfinite(outValue.x) && std::isfinite(outValue.y) &&
			std::isfinite(outValue.z);
	}

	bool_t Parse_Uint64(const std::string& text, uint64_t& outValue)
	{
		if (text.empty() || text.size() > 20u ||
			!std::all_of(text.begin(), text.end(),
				[](const char_t character)
				{
					return character >= '0' && character <= '9';
				}))
		{
			return false;
		}
		uint64_t parsed = 0u;
		const auto result = std::from_chars(
			text.data(), text.data() + text.size(), parsed);
		if (result.ec != std::errc{} ||
			result.ptr != text.data() + text.size())
		{
			return false;
		}
		outValue = parsed;
		return true;
	}

	bool_t Read_Uint64String(
		const DATA_JSON_VALUE& parent,
		const char_t* key,
		uint64_t& outValue)
	{
		std::string text;
		return Read_String(parent, key, false, text) &&
			Parse_Uint64(text, outValue) && 0u != outValue;
	}

	bool_t Commit_TemporaryFile(
		const std::filesystem::path& destination,
		const std::filesystem::path& temporary)
	{
		std::error_code existsError;
		if (std::filesystem::exists(destination, existsError) &&
			!existsError && ReplaceFileW(
				destination.c_str(), temporary.c_str(), nullptr,
				REPLACEFILE_WRITE_THROUGH, nullptr, nullptr))
		{
			return true;
		}
		return FALSE != MoveFileExW(
			temporary.c_str(), destination.c_str(),
			MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
	}

	std::wstring Make_PairTransactionSuffix()
	{
		static std::atomic<uint64_t> sequence{ 0u };
		return L".destruction-pair." +
			std::to_wstring(static_cast<uint64_t>(GetCurrentProcessId())) + L"." +
			std::to_wstring(static_cast<uint64_t>(GetTickCount64())) + L"." +
			std::to_wstring(sequence.fetch_add(
				1u, std::memory_order_relaxed) + 1u);
	}

	void Remove_PairArtifact(const std::filesystem::path& path)
	{
		std::error_code removeError;
		std::filesystem::remove(path, removeError);
		removeError.clear();
		std::filesystem::remove(path.wstring() + L".tmp", removeError);
	}

	bool_t Is_FiniteFloat3(const float3_t& value)
	{
		return std::isfinite(value.x) && std::isfinite(value.y) &&
			std::isfinite(value.z);
	}

	f32_t Length_Squared(const float3_t& value)
	{
		return value.x * value.x + value.y * value.y + value.z * value.z;
	}

#ifndef DESTRUCTION_SIMULATION_CODEC_ONLY
	const DEPLOY_RUNTIME_ENTRY* Find_Entry(
		const CDeployPropRuntime& runtime,
		const uint64_t placementId)
	{
		const auto& entries = runtime.Get_Entries();
		const auto found = std::find_if(entries.begin(), entries.end(),
			[placementId](const DEPLOY_RUNTIME_ENTRY& entry)
			{
				return entry.placement.runtimePlacementId == placementId;
			});
		return found == entries.end() ? nullptr : &*found;
	}
#endif
}

bool_t Client::CDestructionSimulationDocument::Is_StableId(
	const std::string& value)
{
	if (value.empty() || value.size() > 128u)
		return false;
	return std::all_of(value.begin(), value.end(),
		[](const char_t character)
		{
			return (character >= 'a' && character <= 'z') ||
				(character >= 'A' && character <= 'Z') ||
				(character >= '0' && character <= '9') ||
				'.' == character || '_' == character || '-' == character;
		});
}

const char_t* Client::CDestructionSimulationDocument::TriggerKind_ToString(
	const DESTRUCTION_SIMULATION_TRIGGER_KIND kind)
{
	switch (kind)
	{
	case DESTRUCTION_SIMULATION_TRIGGER_KIND::IMMEDIATE:
		return "IMMEDIATE";
	case DESTRUCTION_SIMULATION_TRIGGER_KIND::TIMELINE_TIME:
		return "TIMELINE_TIME";
	case DESTRUCTION_SIMULATION_TRIGGER_KIND::COLLISION_IMPACT:
		return "COLLISION_IMPACT";
	default:
		return "";
	}
}

bool_t Client::CDestructionSimulationDocument::Try_ParseTriggerKind(
	const std::string& value,
	DESTRUCTION_SIMULATION_TRIGGER_KIND& outKind)
{
	if ("IMMEDIATE" == value)
		outKind = DESTRUCTION_SIMULATION_TRIGGER_KIND::IMMEDIATE;
	else if ("TIMELINE_TIME" == value)
		outKind = DESTRUCTION_SIMULATION_TRIGGER_KIND::TIMELINE_TIME;
	else if ("COLLISION_IMPACT" == value)
		outKind = DESTRUCTION_SIMULATION_TRIGGER_KIND::COLLISION_IMPACT;
	else
		return false;
	return true;
}

void Client::CDestructionSimulationDocument::Reset_Empty()
{
	m_Profiles.clear();
	m_isReady = true;
	m_isDirty = false;
}

void Client::CDestructionSimulationDocument::Clear()
{
	m_Profiles.clear();
	m_isReady = false;
	m_isDirty = false;
}

bool_t Client::CDestructionSimulationDocument::Load(
	const std::filesystem::path& path,
	const std::string& expectedAreaId,
	std::string& outStatus)
{
	std::string text;
	if (path.empty() || expectedAreaId.empty() || !Read_TextFile(path, text))
	{
		outStatus = "Destruction simulation document is unreadable: " +
			path.string();
		return false;
	}

	DATA_JSON_VALUE root;
	std::string parseError;
	if (!CDataJson::Parse(text, root, parseError))
	{
		outStatus = "Destruction simulation parse failed: " + parseError;
		return false;
	}
	if (!Is_ExactObject(root,
		{ "schema", "formatVersion", "areaId", "profiles" }))
	{
		outStatus = "Destruction simulation root has unexpected properties";
		return false;
	}

	std::string schema;
	std::string areaId;
	f32_t formatVersion = 0.f;
	if (!Read_String(root, "schema", false, schema) || SCHEMA != schema ||
		!Read_Number(root, "formatVersion", formatVersion) ||
		static_cast<f32_t>(FORMAT_VERSION) != formatVersion ||
		!Read_String(root, "areaId", false, areaId) ||
		areaId != expectedAreaId)
	{
		outStatus = "Destruction simulation header does not match this Area";
		return false;
	}

	const DATA_JSON_VALUE* profiles = root.Find("profiles");
	if (nullptr == profiles || !profiles->Is_Array() ||
		profiles->Get_Array().size() > MAX_PROFILE_COUNT)
	{
		outStatus = "Destruction simulation profiles are invalid or over limit";
		return false;
	}

	std::vector<DESTRUCTION_SIMULATION_PROFILE> stagedProfiles;
	stagedProfiles.reserve(profiles->Get_Array().size());
	for (const DATA_JSON_VALUE& profileValue : profiles->Get_Array())
	{
		if (!Is_ExactObject(profileValue, {
			"profileId", "groupId", "durationSeconds",
			"previewGroundEnabled", "previewGroundHeight",
			"previewGroundHalfExtents", "elements" }))
		{
			outStatus = "Destruction simulation profile has unexpected properties";
			return false;
		}

		DESTRUCTION_SIMULATION_PROFILE profile;
		if (!Read_String(profileValue, "profileId", false, profile.profileId) ||
			!Read_String(profileValue, "groupId", false, profile.groupId) ||
			!Read_Number(profileValue, "durationSeconds",
				profile.fDurationSeconds) ||
			!Read_Bool(profileValue, "previewGroundEnabled",
				profile.isPreviewGroundEnabled) ||
			!Read_Number(profileValue, "previewGroundHeight",
				profile.fPreviewGroundHeight) ||
			!Read_Float2(profileValue, "previewGroundHalfExtents",
				profile.vPreviewGroundHalfExtents))
		{
			outStatus = "Destruction simulation profile field is invalid";
			return false;
		}

		const DATA_JSON_VALUE* elements = profileValue.Find("elements");
		if (nullptr == elements || !elements->Is_Array() ||
			elements->Get_Array().size() > MAX_ELEMENT_COUNT)
		{
			outStatus = "Destruction simulation elements are invalid or over limit";
			return false;
		}

		profile.Elements.reserve(elements->Get_Array().size());
		for (const DATA_JSON_VALUE& elementValue : elements->Get_Array())
		{
			if (!Is_ExactObject(elementValue, {
				"elementId", "sourceRuntimePlacementId", "spawnOffset",
				"direction", "speedMetersPerSecond", "gravityScale",
				"lifetimeSeconds", "trigger" }))
			{
				outStatus = "Destruction simulation element has unexpected properties";
				return false;
			}

			DESTRUCTION_SIMULATION_ELEMENT element;
			if (!Read_String(elementValue, "elementId", false,
					element.elementId) ||
				!Read_Uint64String(elementValue, "sourceRuntimePlacementId",
					element.sourceRuntimePlacementId) ||
				!Read_Float3(elementValue, "spawnOffset", element.vSpawnOffset) ||
				!Read_Float3(elementValue, "direction", element.vDirection) ||
				!Read_Number(elementValue, "speedMetersPerSecond",
					element.fSpeedMetersPerSecond) ||
				!Read_Number(elementValue, "gravityScale",
					element.fGravityScale) ||
				!Read_Number(elementValue, "lifetimeSeconds",
					element.fLifetimeSeconds))
			{
				outStatus = "Destruction simulation element field is invalid";
				return false;
			}

			const DATA_JSON_VALUE* trigger = elementValue.Find("trigger");
			if (nullptr == trigger || !Is_ExactObject(*trigger,
				{ "kind", "timeSeconds", "receiverCollisionId" }))
			{
				outStatus = "Destruction simulation trigger has unexpected properties";
				return false;
			}
			std::string triggerKind;
			if (!Read_String(*trigger, "kind", false, triggerKind) ||
				!Try_ParseTriggerKind(triggerKind, element.Trigger.eKind) ||
				!Read_Number(*trigger, "timeSeconds",
					element.Trigger.fTimeSeconds) ||
				!Read_String(*trigger, "receiverCollisionId", true,
					element.Trigger.receiverCollisionId))
			{
				outStatus = "Destruction simulation trigger field is invalid";
				return false;
			}
			profile.Elements.push_back(std::move(element));
		}
		stagedProfiles.push_back(std::move(profile));
	}

	CDestructionSimulationDocument staged;
	staged.m_Profiles = std::move(stagedProfiles);
	staged.m_isReady = true;
	if (!staged.Validate_Document(outStatus))
		return false;

	m_Profiles = std::move(staged.m_Profiles);
	m_isReady = true;
	m_isDirty = false;
	outStatus = "Loaded destruction simulations: " +
		std::to_string(m_Profiles.size()) + " profiles";
	return true;
}

bool_t Client::CDestructionSimulationDocument::Save(
	const std::filesystem::path& path,
	const std::string& areaId,
	std::string& outStatus) const
{
	if (path.empty() || areaId.empty() || !Is_StableId(areaId))
	{
		outStatus = "Destruction simulation save header is invalid";
		return false;
	}
	if (!Validate_Document(outStatus))
		return false;

	std::error_code directoryError;
	std::filesystem::create_directories(path.parent_path(), directoryError);
	if (directoryError)
	{
		outStatus = "Could not create destruction simulation directory";
		return false;
	}

	const std::filesystem::path temporary = path.wstring() + L".tmp";
	std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
	if (!output)
	{
		outStatus = "Could not create destruction simulation temporary file";
		return false;
	}
	output << std::setprecision(std::numeric_limits<f32_t>::max_digits10);
	output << "{\n"
		<< "  \"schema\": \"" << SCHEMA << "\",\n"
		<< "  \"formatVersion\": " << FORMAT_VERSION << ",\n"
		<< "  \"areaId\": \"" << CDataJson::Escape(areaId) << "\",\n"
		<< "  \"profiles\": [";
	for (size_t profileIndex = 0u; profileIndex < m_Profiles.size();
		++profileIndex)
	{
		const DESTRUCTION_SIMULATION_PROFILE& profile =
			m_Profiles[profileIndex];
		output << (0u == profileIndex ? "\n" : ",\n")
			<< "    {\n"
			<< "      \"profileId\": \""
			<< CDataJson::Escape(profile.profileId) << "\",\n"
			<< "      \"groupId\": \""
			<< CDataJson::Escape(profile.groupId) << "\",\n"
			<< "      \"durationSeconds\": " << profile.fDurationSeconds
			<< ",\n"
			<< "      \"previewGroundEnabled\": "
			<< (profile.isPreviewGroundEnabled ? "true" : "false") << ",\n"
			<< "      \"previewGroundHeight\": "
			<< profile.fPreviewGroundHeight << ",\n"
			<< "      \"previewGroundHalfExtents\": ["
			<< profile.vPreviewGroundHalfExtents.x << ", "
			<< profile.vPreviewGroundHalfExtents.y << "],\n"
			<< "      \"elements\": [";
		for (size_t elementIndex = 0u;
			elementIndex < profile.Elements.size(); ++elementIndex)
		{
			const DESTRUCTION_SIMULATION_ELEMENT& element =
				profile.Elements[elementIndex];
			output << (0u == elementIndex ? "\n" : ",\n")
				<< "        {\n"
				<< "          \"elementId\": \""
				<< CDataJson::Escape(element.elementId) << "\",\n"
				<< "          \"sourceRuntimePlacementId\": \""
				<< element.sourceRuntimePlacementId << "\",\n"
				<< "          \"spawnOffset\": ["
				<< element.vSpawnOffset.x << ", " << element.vSpawnOffset.y
				<< ", " << element.vSpawnOffset.z << "],\n"
				<< "          \"direction\": ["
				<< element.vDirection.x << ", " << element.vDirection.y
				<< ", " << element.vDirection.z << "],\n"
				<< "          \"speedMetersPerSecond\": "
				<< element.fSpeedMetersPerSecond << ",\n"
				<< "          \"gravityScale\": "
				<< element.fGravityScale << ",\n"
				<< "          \"lifetimeSeconds\": "
				<< element.fLifetimeSeconds << ",\n"
				<< "          \"trigger\": { \"kind\": \""
				<< TriggerKind_ToString(element.Trigger.eKind)
				<< "\", \"timeSeconds\": "
				<< element.Trigger.fTimeSeconds
				<< ", \"receiverCollisionId\": \""
				<< CDataJson::Escape(element.Trigger.receiverCollisionId)
				<< "\" }\n"
				<< "        }";
		}
		output << (profile.Elements.empty() ? "]\n" : "\n      ]\n")
			<< "    }";
	}
	output << (m_Profiles.empty() ? "]\n" : "\n  ]\n") << "}\n";
	output.flush();
	const bool_t writeSucceeded = output.good();
	output.close();

	CDestructionSimulationDocument verify;
	std::string verifyStatus;
	if (!writeSucceeded || !verify.Load(temporary, areaId, verifyStatus))
	{
		std::error_code removeError;
		std::filesystem::remove(temporary, removeError);
		outStatus = writeSucceeded ?
			"Destruction simulation reload check failed: " + verifyStatus :
			"Destruction simulation write failed";
		return false;
	}
	if (!Commit_TemporaryFile(path, temporary))
	{
		std::error_code removeError;
		std::filesystem::remove(temporary, removeError);
		outStatus = "Destruction simulation atomic replace failed";
		return false;
	}

	outStatus = "Saved destruction simulations: " +
		std::to_string(m_Profiles.size()) + " profiles";
	return true;
}

bool_t Client::CDestructionSimulationDocument::Save_AuthoringPair(
	const CWorldDestructionDocument& destructionDocument,
	const std::filesystem::path& destructionPath,
	const CDestructionSimulationDocument& simulationDocument,
	const std::filesystem::path& simulationPath,
	const std::string& areaId,
	const std::string& encounterId,
	std::string& outStatus)
{
	if (destructionPath.empty() || simulationPath.empty() ||
		areaId.empty() || encounterId.empty() ||
		!Is_StableId(areaId) ||
		!CWorldDestructionDocument::Is_StableId(encounterId))
	{
		outStatus = "Destruction authoring pair save header is invalid";
		return false;
	}

	std::error_code destructionAbsoluteError;
	std::error_code simulationAbsoluteError;
	const std::filesystem::path normalizedDestructionPath =
		std::filesystem::absolute(
			destructionPath, destructionAbsoluteError).lexically_normal();
	const std::filesystem::path normalizedSimulationPath =
		std::filesystem::absolute(
			simulationPath, simulationAbsoluteError).lexically_normal();
	if (destructionAbsoluteError || simulationAbsoluteError ||
		0 == _wcsicmp(normalizedDestructionPath.c_str(),
			normalizedSimulationPath.c_str()))
	{
		outStatus = "Destruction authoring pair paths are invalid";
		return false;
	}

	if (!destructionDocument.Is_Ready() || !simulationDocument.Is_Ready())
	{
		outStatus = "Destruction authoring pair is not ready";
		return false;
	}
	if (!simulationDocument.Validate_GroupReferences(
		destructionDocument, outStatus))
	{
		return false;
	}

	const std::wstring transactionSuffix = Make_PairTransactionSuffix();
	const std::filesystem::path destructionStage =
		destructionPath.wstring() + transactionSuffix + L".stage";
	const std::filesystem::path simulationStage =
		simulationPath.wstring() + transactionSuffix + L".stage";
	const std::filesystem::path destructionRollback =
		destructionPath.wstring() + transactionSuffix + L".rollback";

	std::string stageStatus;
	if (!destructionDocument.Save(
		destructionStage, areaId, encounterId, stageStatus))
	{
		Remove_PairArtifact(destructionStage);
		Remove_PairArtifact(simulationStage);
		Remove_PairArtifact(destructionRollback);
		outStatus = "World destruction pair staging failed: " + stageStatus;
		return false;
	}
	if (!simulationDocument.Save(simulationStage, areaId, stageStatus))
	{
		Remove_PairArtifact(destructionStage);
		Remove_PairArtifact(simulationStage);
		Remove_PairArtifact(destructionRollback);
		outStatus = "Destruction simulation pair staging failed: " +
			stageStatus;
		return false;
	}

	std::error_code inspectError;
	const bool_t destructionExisted =
		std::filesystem::exists(destructionPath, inspectError);
	if (inspectError)
	{
		Remove_PairArtifact(destructionStage);
		Remove_PairArtifact(simulationStage);
		Remove_PairArtifact(destructionRollback);
		outStatus = "Could not inspect world destruction destination";
		return false;
	}
	if (destructionExisted)
	{
		std::error_code backupError;
		const bool_t backedUp = std::filesystem::copy_file(
			destructionPath, destructionRollback,
			std::filesystem::copy_options::none, backupError);
		if (!backedUp || backupError)
		{
			Remove_PairArtifact(destructionStage);
			Remove_PairArtifact(simulationStage);
			Remove_PairArtifact(destructionRollback);
			outStatus = "Could not back up world destruction destination";
			return false;
		}
	}

	/* The first destination needs a rollback copy. The second destination is
	   still byte-exact until its single final replace succeeds, so a no-share
	   lock on it can fail closed without opening or copying that canonical. */
	if (!Commit_TemporaryFile(destructionPath, destructionStage))
	{
		Remove_PairArtifact(destructionStage);
		Remove_PairArtifact(simulationStage);
		Remove_PairArtifact(destructionRollback);
		outStatus = "World destruction pair commit failed";
		return false;
	}

	if (!Commit_TemporaryFile(simulationPath, simulationStage))
	{
		bool_t rollbackSucceeded = false;
		if (destructionExisted)
		{
			rollbackSucceeded = Commit_TemporaryFile(
				destructionPath, destructionRollback);
		}
		else
		{
			std::error_code rollbackError;
			rollbackSucceeded = std::filesystem::remove(
				destructionPath, rollbackError) && !rollbackError;
		}

		Remove_PairArtifact(destructionStage);
		Remove_PairArtifact(simulationStage);
		if (rollbackSucceeded)
		{
			Remove_PairArtifact(destructionRollback);
			outStatus = "Destruction simulation pair commit failed; "
				"original documents restored";
		}
		else
		{
			outStatus = "Destruction simulation pair commit failed and world "
				"destruction rollback failed; rollback sidecar retained";
		}
		return false;
	}

	Remove_PairArtifact(destructionRollback);
	outStatus = "Saved world destruction and simulations as one transaction";
	return true;
}

bool_t Client::CDestructionSimulationDocument::Validate_Profile(
	const DESTRUCTION_SIMULATION_PROFILE& profile,
	std::string& outStatus)
{
	if (!Is_StableId(profile.profileId) || !Is_StableId(profile.groupId))
	{
		outStatus = "Destruction simulation profile has an invalid stable ID";
		return false;
	}
	if (!std::isfinite(profile.fDurationSeconds) ||
		profile.fDurationSeconds < 1.f / 60.f ||
		profile.fDurationSeconds > MAX_DURATION_SECONDS)
	{
		outStatus = "Destruction simulation duration is out of range: " +
			profile.profileId;
		return false;
	}
	if (!std::isfinite(profile.fPreviewGroundHeight) ||
		std::abs(profile.fPreviewGroundHeight) > MAX_ABSOLUTE_OFFSET ||
		!std::isfinite(profile.vPreviewGroundHalfExtents.x) ||
		!std::isfinite(profile.vPreviewGroundHalfExtents.y) ||
		profile.vPreviewGroundHalfExtents.x <= 0.f ||
		profile.vPreviewGroundHalfExtents.y <= 0.f ||
		profile.vPreviewGroundHalfExtents.x > MAX_GROUND_HALF_EXTENT ||
		profile.vPreviewGroundHalfExtents.y > MAX_GROUND_HALF_EXTENT)
	{
		outStatus = "Destruction simulation preview ground is invalid: " +
			profile.profileId;
		return false;
	}
	if (profile.Elements.empty() ||
		profile.Elements.size() > MAX_ELEMENT_COUNT)
	{
		outStatus = "Destruction simulation profile needs 1.." +
			std::to_string(MAX_ELEMENT_COUNT) + " elements: " + profile.profileId;
		return false;
	}

	std::unordered_set<std::string> elementIds;
	std::unordered_set<uint64_t> placementIds;
	for (const DESTRUCTION_SIMULATION_ELEMENT& element : profile.Elements)
	{
		if (!Is_StableId(element.elementId) ||
			!elementIds.insert(element.elementId).second ||
			0u == element.sourceRuntimePlacementId ||
			!placementIds.insert(element.sourceRuntimePlacementId).second)
		{
			outStatus = "Destruction simulation element has a duplicate or invalid "
				"identity: " + element.elementId;
			return false;
		}
		if (!Is_FiniteFloat3(element.vSpawnOffset) ||
			std::abs(element.vSpawnOffset.x) > MAX_ABSOLUTE_OFFSET ||
			std::abs(element.vSpawnOffset.y) > MAX_ABSOLUTE_OFFSET ||
			std::abs(element.vSpawnOffset.z) > MAX_ABSOLUTE_OFFSET ||
			!Is_FiniteFloat3(element.vDirection))
		{
			outStatus = "Destruction simulation vector is invalid: " +
				element.elementId;
			return false;
		}
		const f32_t directionLength = std::sqrt(Length_Squared(
			element.vDirection));
		if (!std::isfinite(directionLength) ||
			std::abs(directionLength - 1.f) > DIRECTION_LENGTH_TOLERANCE)
		{
			outStatus = "Destruction simulation direction must be normalized: " +
				element.elementId;
			return false;
		}
		if (!std::isfinite(element.fSpeedMetersPerSecond) ||
			element.fSpeedMetersPerSecond < 0.f ||
			element.fSpeedMetersPerSecond > MAX_SPEED_METERS_PER_SECOND ||
			!std::isfinite(element.fGravityScale) ||
			element.fGravityScale < 0.f ||
			element.fGravityScale > MAX_GRAVITY_SCALE ||
			!std::isfinite(element.fLifetimeSeconds) ||
			element.fLifetimeSeconds <= 0.f ||
			element.fLifetimeSeconds > profile.fDurationSeconds)
		{
			outStatus = "Destruction simulation scalar is out of range: " +
				element.elementId;
			return false;
		}
		if (DESTRUCTION_SIMULATION_TRIGGER_KIND::END == element.Trigger.eKind ||
			!std::isfinite(element.Trigger.fTimeSeconds) ||
			element.Trigger.fTimeSeconds < 0.f ||
			element.Trigger.fTimeSeconds > profile.fDurationSeconds)
		{
			outStatus = "Destruction simulation trigger is invalid: " +
				element.elementId;
			return false;
		}

		const bool_t needsReceiver =
			DESTRUCTION_SIMULATION_TRIGGER_KIND::COLLISION_IMPACT ==
				element.Trigger.eKind;
		if (needsReceiver == element.Trigger.receiverCollisionId.empty() ||
			(!element.Trigger.receiverCollisionId.empty() &&
				!Is_StableId(element.Trigger.receiverCollisionId)))
		{
			outStatus = needsReceiver ?
				"COLLISION_IMPACT simulation element needs a receiver: " +
					element.elementId :
				"Only COLLISION_IMPACT may set a receiver: " + element.elementId;
			return false;
		}
		if (DESTRUCTION_SIMULATION_TRIGGER_KIND::TIMELINE_TIME !=
				element.Trigger.eKind &&
			0.f != element.Trigger.fTimeSeconds)
		{
			outStatus = "Only TIMELINE_TIME may set trigger time: " +
				element.elementId;
			return false;
		}
		if (DESTRUCTION_SIMULATION_TRIGGER_KIND::COLLISION_IMPACT !=
				element.Trigger.eKind &&
			element.Trigger.fTimeSeconds + element.fLifetimeSeconds >
				profile.fDurationSeconds + 0.000001f)
		{
			outStatus = "Element lifetime exceeds the profile timeline: " +
				element.elementId;
			return false;
		}
	}
	return true;
}

bool_t Client::CDestructionSimulationDocument::Validate_Document(
	std::string& outStatus) const
{
	if (!m_isReady)
	{
		outStatus = "Destruction simulation document is not ready";
		return false;
	}
	if (m_Profiles.size() > MAX_PROFILE_COUNT)
	{
		outStatus = "Destruction simulation profile count is over limit";
		return false;
	}
	std::unordered_set<std::string> profileIds;
	for (const DESTRUCTION_SIMULATION_PROFILE& profile : m_Profiles)
	{
		if (!profileIds.insert(profile.profileId).second)
		{
			outStatus = "Duplicate destruction simulation profile: " +
				profile.profileId;
			return false;
		}
		if (!Validate_Profile(profile, outStatus))
			return false;
	}
	return true;
}

bool_t Client::CDestructionSimulationDocument::Add_Profile(
	const DESTRUCTION_SIMULATION_PROFILE& profile,
	std::string& outStatus)
{
	if (!m_isReady)
	{
		outStatus = "Destruction simulation document is not ready";
		return false;
	}
	if (m_Profiles.size() >= MAX_PROFILE_COUNT)
	{
		outStatus = "Destruction simulation profile count is at its limit";
		return false;
	}
	if (nullptr != Find_Profile(profile.profileId))
	{
		outStatus = "Duplicate destruction simulation profile: " +
			profile.profileId;
		return false;
	}
	if (!Validate_Profile(profile, outStatus))
		return false;
	m_Profiles.push_back(profile);
	m_isDirty = true;
	outStatus = "Added destruction simulation profile: " + profile.profileId;
	return true;
}

bool_t Client::CDestructionSimulationDocument::Update_Profile(
	const DESTRUCTION_SIMULATION_PROFILE& profile,
	std::string& outStatus)
{
	DESTRUCTION_SIMULATION_PROFILE* destination = Find_Profile(
		profile.profileId);
	if (nullptr == destination)
	{
		outStatus = "Destruction simulation profile is unknown: " +
			profile.profileId;
		return false;
	}
	if (!Validate_Profile(profile, outStatus))
		return false;
	*destination = profile;
	m_isDirty = true;
	outStatus = "Updated destruction simulation profile: " + profile.profileId;
	return true;
}

bool_t Client::CDestructionSimulationDocument::Remove_Profile(
	const std::string& profileId,
	std::string& outStatus)
{
	const auto found = std::find_if(m_Profiles.begin(), m_Profiles.end(),
		[&profileId](const DESTRUCTION_SIMULATION_PROFILE& profile)
		{
			return profile.profileId == profileId;
		});
	if (found == m_Profiles.end())
	{
		outStatus = "Destruction simulation profile is unknown: " + profileId;
		return false;
	}
	m_Profiles.erase(found);
	m_isDirty = true;
	outStatus = "Removed destruction simulation profile: " + profileId;
	return true;
}

bool_t Client::CDestructionSimulationDocument::Remove_ProfilesForGroup(
	const std::string& groupId,
	std::string& outStatus)
{
	if (!m_isReady || !Is_StableId(groupId))
	{
		outStatus = "Simulation group ID is invalid";
		return false;
	}
	const size_t previousCount = m_Profiles.size();
	m_Profiles.erase(std::remove_if(
		m_Profiles.begin(), m_Profiles.end(),
		[&groupId](const DESTRUCTION_SIMULATION_PROFILE& profile)
		{
			return profile.groupId == groupId;
		}), m_Profiles.end());
	const size_t removedCount = previousCount - m_Profiles.size();
	if (0u != removedCount)
		m_isDirty = true;
	outStatus = "Removed " + std::to_string(removedCount) +
		" simulation profiles for " + groupId;
	return true;
}

bool_t Client::CDestructionSimulationDocument::Add_Element(
	const std::string& profileId,
	const DESTRUCTION_SIMULATION_ELEMENT& element,
	std::string& outStatus)
{
	DESTRUCTION_SIMULATION_PROFILE* profile = Find_Profile(profileId);
	if (nullptr == profile || profile->Elements.size() >= MAX_ELEMENT_COUNT ||
		nullptr != Find_Element(profileId, element.elementId))
	{
		outStatus = "Destruction simulation element cannot be added";
		return false;
	}
	DESTRUCTION_SIMULATION_PROFILE staged = *profile;
	staged.Elements.push_back(element);
	if (!Validate_Profile(staged, outStatus))
		return false;
	*profile = std::move(staged);
	m_isDirty = true;
	outStatus = "Added destruction simulation element: " + element.elementId;
	return true;
}

bool_t Client::CDestructionSimulationDocument::Update_Element(
	const std::string& profileId,
	const DESTRUCTION_SIMULATION_ELEMENT& element,
	std::string& outStatus)
{
	DESTRUCTION_SIMULATION_PROFILE* profile = Find_Profile(profileId);
	if (nullptr == profile)
	{
		outStatus = "Destruction simulation profile is unknown: " + profileId;
		return false;
	}
	DESTRUCTION_SIMULATION_PROFILE staged = *profile;
	const auto found = std::find_if(staged.Elements.begin(), staged.Elements.end(),
		[&element](const DESTRUCTION_SIMULATION_ELEMENT& candidate)
		{
			return candidate.elementId == element.elementId;
		});
	if (found == staged.Elements.end())
	{
		outStatus = "Destruction simulation element is unknown: " +
			element.elementId;
		return false;
	}
	*found = element;
	if (!Validate_Profile(staged, outStatus))
		return false;
	*profile = std::move(staged);
	m_isDirty = true;
	outStatus = "Updated destruction simulation element: " + element.elementId;
	return true;
}

bool_t Client::CDestructionSimulationDocument::Remove_Element(
	const std::string& profileId,
	const std::string& elementId,
	std::string& outStatus)
{
	DESTRUCTION_SIMULATION_PROFILE* profile = Find_Profile(profileId);
	if (nullptr == profile || profile->Elements.size() <= 1u)
	{
		outStatus = "A simulation profile must keep at least one element";
		return false;
	}
	const auto found = std::find_if(profile->Elements.begin(),
		profile->Elements.end(),
		[&elementId](const DESTRUCTION_SIMULATION_ELEMENT& element)
		{
			return element.elementId == elementId;
		});
	if (found == profile->Elements.end())
	{
		outStatus = "Destruction simulation element is unknown: " + elementId;
		return false;
	}
	profile->Elements.erase(found);
	m_isDirty = true;
	outStatus = "Removed destruction simulation element: " + elementId;
	return true;
}

const Client::DESTRUCTION_SIMULATION_PROFILE*
Client::CDestructionSimulationDocument::Find_Profile(
	const std::string& profileId) const
{
	const auto found = std::find_if(m_Profiles.begin(), m_Profiles.end(),
		[&profileId](const DESTRUCTION_SIMULATION_PROFILE& profile)
		{
			return profile.profileId == profileId;
		});
	return found == m_Profiles.end() ? nullptr : &*found;
}

Client::DESTRUCTION_SIMULATION_PROFILE*
Client::CDestructionSimulationDocument::Find_Profile(
	const std::string& profileId)
{
	const auto found = std::find_if(m_Profiles.begin(), m_Profiles.end(),
		[&profileId](const DESTRUCTION_SIMULATION_PROFILE& profile)
		{
			return profile.profileId == profileId;
		});
	return found == m_Profiles.end() ? nullptr : &*found;
}

const Client::DESTRUCTION_SIMULATION_ELEMENT*
Client::CDestructionSimulationDocument::Find_Element(
	const std::string& profileId,
	const std::string& elementId) const
{
	const DESTRUCTION_SIMULATION_PROFILE* profile = Find_Profile(profileId);
	if (nullptr == profile)
		return nullptr;
	const auto found = std::find_if(profile->Elements.begin(),
		profile->Elements.end(),
		[&elementId](const DESTRUCTION_SIMULATION_ELEMENT& element)
		{
			return element.elementId == elementId;
		});
	return found == profile->Elements.end() ? nullptr : &*found;
}

#ifndef DESTRUCTION_SIMULATION_CODEC_ONLY
bool_t Client::CDestructionSimulationDocument::Create_DefaultForGroup(
	const DESTRUCTION_GROUP& group,
	const CDeployPropRuntime& deployRuntime,
	DESTRUCTION_SIMULATION_PROFILE& outProfile,
	std::string& outStatus)
{
	if (!Is_StableId(group.groupId) || group.memberPlacementIds.empty() ||
		group.memberPlacementIds.size() > MAX_ELEMENT_COUNT ||
		!deployRuntime.Is_Loaded())
	{
		outStatus = "Default simulation requires a loaded, non-empty group";
		return false;
	}

	float3_t centre{};
	for (const uint64_t placementId : group.memberPlacementIds)
	{
		const DEPLOY_RUNTIME_ENTRY* entry = Find_Entry(
			deployRuntime, placementId);
		if (nullptr == entry || nullptr == entry->object)
		{
			outStatus = "Default simulation member is not loaded: " +
				std::to_string(placementId);
			return false;
		}
		centre.x += entry->placement.position.x;
		centre.y += entry->placement.position.y;
		centre.z += entry->placement.position.z;
	}
	const f32_t inverseCount = 1.f /
		static_cast<f32_t>(group.memberPlacementIds.size());
	centre.x *= inverseCount;
	centre.y *= inverseCount;
	centre.z *= inverseCount;

	DESTRUCTION_SIMULATION_PROFILE staged;
	staged.profileId = group.groupId + ".preview";
	staged.groupId = group.groupId;
	staged.fDurationSeconds = 5.f;
	staged.isPreviewGroundEnabled = true;
	f32_t groundHeight = FLT_MAX;
	f32_t halfExtentX = 1.f;
	f32_t halfExtentZ = 1.f;
	staged.Elements.reserve(group.memberPlacementIds.size());
	for (const uint64_t placementId : group.memberPlacementIds)
	{
		const DEPLOY_RUNTIME_ENTRY* entry = Find_Entry(
			deployRuntime, placementId);
		float3_t boundsCentre{};
		float3_t boundsHalfExtents{};
		if (nullptr == entry || nullptr == entry->object ||
			!entry->object->Get_WorldBounds(
				boundsCentre, boundsHalfExtents))
		{
			outStatus = "Default simulation needs baked model bounds: " +
				std::to_string(placementId);
			return false;
		}
		groundHeight = (std::min)(groundHeight,
			boundsCentre.y - boundsHalfExtents.y);
		halfExtentX = (std::max)(halfExtentX,
			std::abs(boundsCentre.x - centre.x) + boundsHalfExtents.x + 2.f);
		halfExtentZ = (std::max)(halfExtentZ,
			std::abs(boundsCentre.z - centre.z) + boundsHalfExtents.z + 2.f);

		float3_t direction = {
			entry->placement.position.x - centre.x,
			0.35f,
			entry->placement.position.z - centre.z
		};
		const f32_t length = std::sqrt(Length_Squared(direction));
		if (!std::isfinite(length) || length <= 0.000001f)
			direction = { 0.f, 1.f, 0.f };
		else
		{
			direction.x /= length;
			direction.y /= length;
			direction.z /= length;
		}

		DESTRUCTION_SIMULATION_ELEMENT element;
		element.elementId = "debris." + std::to_string(placementId);
		element.sourceRuntimePlacementId = placementId;
		element.vDirection = direction;
		element.fSpeedMetersPerSecond = 8.f;
		element.fGravityScale = 1.f;
		element.fLifetimeSeconds = 4.f;
		element.Trigger.eKind =
			DESTRUCTION_SIMULATION_TRIGGER_KIND::TIMELINE_TIME;
		element.Trigger.fTimeSeconds = 0.25f;
		staged.Elements.push_back(std::move(element));
	}
	staged.fPreviewGroundHeight = groundHeight;
	staged.vPreviewGroundHalfExtents = { halfExtentX, halfExtentZ };
	if (!Validate_Profile(staged, outStatus))
		return false;

	outProfile = std::move(staged);
	outStatus = "Built default destruction simulation for " + group.groupId +
		": " + std::to_string(outProfile.Elements.size()) + " elements";
	return true;
}

bool_t Client::CDestructionSimulationDocument::Synchronize_Group(
	const DESTRUCTION_GROUP& group,
	const CDeployPropRuntime& deployRuntime,
	std::string& outStatus)
{
	if (!m_isReady)
	{
		outStatus = "Destruction simulation document is not ready";
		return false;
	}
	const bool_t hasMatchingProfile = std::any_of(
		m_Profiles.begin(), m_Profiles.end(),
		[&group](const DESTRUCTION_SIMULATION_PROFILE& profile)
		{
			return profile.groupId == group.groupId;
		});
	if (!hasMatchingProfile)
	{
		outStatus = "No simulation profile needs group synchronization";
		return true;
	}

	DESTRUCTION_SIMULATION_PROFILE defaults;
	if (!Create_DefaultForGroup(group, deployRuntime, defaults, outStatus))
		return false;

	CDestructionSimulationDocument staged = *this;
	bool_t changed = false;
	for (DESTRUCTION_SIMULATION_PROFILE& profile : staged.m_Profiles)
	{
		if (profile.groupId != group.groupId)
			continue;

		const auto staleBegin = std::remove_if(
			profile.Elements.begin(), profile.Elements.end(),
			[&group](const DESTRUCTION_SIMULATION_ELEMENT& element)
			{
				return group.memberPlacementIds.end() == std::find(
					group.memberPlacementIds.begin(),
					group.memberPlacementIds.end(),
					element.sourceRuntimePlacementId);
			});
		if (staleBegin != profile.Elements.end())
		{
			profile.Elements.erase(staleBegin, profile.Elements.end());
			changed = true;
		}

		for (const DESTRUCTION_SIMULATION_ELEMENT& defaultElement :
			defaults.Elements)
		{
			const bool_t exists = std::any_of(
				profile.Elements.begin(), profile.Elements.end(),
				[&defaultElement](const DESTRUCTION_SIMULATION_ELEMENT& element)
				{
					return element.sourceRuntimePlacementId ==
						defaultElement.sourceRuntimePlacementId;
				});
			if (exists)
				continue;

			DESTRUCTION_SIMULATION_ELEMENT appended = defaultElement;
			const f32_t minimumLife = 1.f / 60.f;
			appended.Trigger.fTimeSeconds = (std::min)(
				appended.Trigger.fTimeSeconds,
				(std::max)(0.f, profile.fDurationSeconds - minimumLife));
			appended.fLifetimeSeconds = (std::min)(
				appended.fLifetimeSeconds,
				profile.fDurationSeconds - appended.Trigger.fTimeSeconds);
			profile.Elements.push_back(std::move(appended));
			changed = true;
		}

		if (!Validate_Profile(profile, outStatus))
			return false;
	}
	if (!staged.Validate_Document(outStatus))
		return false;

	if (changed)
	{
		m_Profiles = std::move(staged.m_Profiles);
		m_isDirty = true;
	}
	outStatus = changed ?
		"Synchronized simulation elements with destruction group " +
			group.groupId :
		"Simulation elements already match destruction group " + group.groupId;
	return true;
}
#endif

bool_t Client::CDestructionSimulationDocument::Validate_GroupReferences(
	const CWorldDestructionDocument& destructionDocument,
	std::string& outStatus) const
{
	if (!m_isReady || !destructionDocument.Is_Ready())
	{
		outStatus = "Destruction documents are not ready for cross-validation";
		return false;
	}
	for (const DESTRUCTION_SIMULATION_PROFILE& profile : m_Profiles)
	{
		const DESTRUCTION_GROUP* group =
			destructionDocument.Find_Group(profile.groupId);
		if (nullptr == group)
		{
			outStatus = "Simulation profile references an unknown group: " +
				profile.groupId;
			return false;
		}
		if (profile.Elements.size() != group->memberPlacementIds.size())
		{
			outStatus = "Simulation profile/member count mismatch: " +
				profile.profileId;
			return false;
		}
		for (const uint64_t placementId : group->memberPlacementIds)
		{
			const bool_t projected = std::any_of(
				profile.Elements.begin(), profile.Elements.end(),
				[placementId](const DESTRUCTION_SIMULATION_ELEMENT& element)
				{
					return element.sourceRuntimePlacementId == placementId;
				});
			if (!projected)
			{
				outStatus = "Simulation profile is missing group member " +
					std::to_string(placementId) + ": " + profile.profileId;
				return false;
			}
		}
	}
	outStatus = "Destruction simulation group references are valid";
	return true;
}
