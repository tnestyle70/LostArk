#if !defined(LOSTARK_BALANCE_TOOL_CONTRACT_TEST)
#include "imgui.h"
#endif

#include "BalanceTool.h"

#include "DataJson.h"
#include "NetworkManager.h"
#include "ProjectDataRoot.h"
#include "ValtanTuningCommandService.h"
#if !defined(LOSTARK_BALANCE_TOOL_CONTRACT_TEST)
#include "ActorCatalog.h"
#include "CombatHUDViewModel.h"
#include "GameInstance.h"
#endif

#include <Windows.h>
#include <algorithm>
#include <charconv>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cwchar>
#include <cwctype>
#include <filesystem>
#include <fstream>
#include <io.h>
#include <limits>
#include <sstream>
#include <string_view>
#include <unordered_set>

namespace
{
	using namespace Client;

	constexpr char VALTAN_GAMEPLAY_SOURCE_PATH[] =
		"Data/Valtan/Valtan.gameplay.json";
	constexpr char VALTAN_PRESENTATION_SOURCE_PATH[] =
		"Data/Valtan/Valtan.presentation.json";

	bool ReadJson(const std::filesystem::path& relativePath,
		DATA_JSON_VALUE& output, std::string& status)
	{
		const std::filesystem::path path = CProjectDataRoot::Resolve(relativePath);
		std::ifstream input(path, std::ios::binary);
		if (path.empty() || !input)
		{
			status = "Missing data document: " + relativePath.string();
			return false;
		}
		const std::string text{
			std::istreambuf_iterator<char>(input),
			std::istreambuf_iterator<char>() };
		return CDataJson::Parse(text, output, status) && output.Is_Object();
	}

	const DATA_JSON_VALUE* Field(const DATA_JSON_VALUE& object,
		const char* name, const DATA_JSON_TYPE type)
	{
		const DATA_JSON_VALUE* value = object.Find(name);
		return nullptr != value && value->Get_Type() == type ? value : nullptr;
	}

	bool ReadString(const DATA_JSON_VALUE& object, const char* name,
		std::string& output)
	{
		const DATA_JSON_VALUE* value = Field(object, name, DATA_JSON_TYPE::STRING);
		if (nullptr == value)
			return false;
		output = value->Get_String();
		return true;
	}

	bool ReadU32(const DATA_JSON_VALUE& object, const char* name,
		std::uint32_t& output)
	{
		const DATA_JSON_VALUE* value = Field(object, name, DATA_JSON_TYPE::NUMBER);
		if (nullptr == value || !std::isfinite(value->Get_Number()) ||
			std::floor(value->Get_Number()) != value->Get_Number() ||
			value->Get_Number() < 0.0 ||
			value->Get_Number() >
			static_cast<double>((std::numeric_limits<std::uint32_t>::max)()))
		{
			return false;
		}
		output = static_cast<std::uint32_t>(value->Get_Number());
		return true;
	}

	bool ReadDouble(const DATA_JSON_VALUE& object, const char* name, double& output)
	{
		const DATA_JSON_VALUE* value = Field(object, name, DATA_JSON_TYPE::NUMBER);
		if (nullptr == value)
			return false;
		output = value->Get_Number();
		return std::isfinite(output);
	}

	bool ReadBoolean(const DATA_JSON_VALUE& object, const char* name, bool& output)
	{
		const DATA_JSON_VALUE* value = Field(object, name, DATA_JSON_TYPE::BOOLEAN);
		if (nullptr == value)
			return false;
		output = value->Get_Boolean();
		return true;
	}

	bool IsExactObject(const DATA_JSON_VALUE& value,
		const std::initializer_list<const char*> keys)
	{
		if (!value.Is_Object() || value.Get_Object().size() != keys.size())
			return false;
		return std::all_of(keys.begin(), keys.end(),
			[&value](const char* key) { return nullptr != value.Find(key); });
	}

	bool HasSchemaVersion(const DATA_JSON_VALUE& root, const char* schema,
		const std::uint32_t version)
	{
		std::string actualSchema;
		std::uint32_t actualVersion = 0u;
		return ReadString(root, "schema", actualSchema) && actualSchema == schema &&
			ReadU32(root, "formatVersion", actualVersion) && actualVersion == version;
	}

	#if !defined(LOSTARK_BALANCE_TOOL_CONTRACT_TEST)
	bool EditU32(const char* label, std::uint32_t& value,
		const std::uint32_t minimum, const std::uint32_t maximum)
	{
		const bool changed = ImGui::InputScalar(
			label, ImGuiDataType_U32, &value, nullptr, nullptr, "%u");
		value = (std::clamp)(value, minimum, maximum);
		return changed;
	}

	bool EditDouble(const char* label, double& value, const float speed,
		const double minimum, const double maximum, const char* format = "%.3f")
	{
		const bool changed = ImGui::DragScalar(
			label, ImGuiDataType_Double, &value, speed, &minimum, &maximum, format);
		value = (std::clamp)(value, minimum, maximum);
		return changed;
	}

	bool EditFloat(const char* label, float& value, const float speed,
		const float minimum, const float maximum, const char* format = "%.3f")
	{
		const bool changed = ImGui::DragFloat(
			label, &value, speed, minimum, maximum, format);
		value = (std::clamp)(value, minimum, maximum);
		return changed;
	}

	const char* DescribeValtanDecisionSource(
		const LostArk::Shared::VALTAN_DECISION_TRACE_SOURCE source)
	{
		using SOURCE = LostArk::Shared::VALTAN_DECISION_TRACE_SOURCE;
		switch (source)
		{
		case SOURCE::NONE: return "NONE";
		case SOURCE::INTRO: return "INTRO";
		case SOURCE::FORCED_HEALTH_BAR: return "FORCED_HEALTH_BAR";
		case SOURCE::FORCED_AUDITION: return "FORCED_AUDITION";
		case SOURCE::ORDERED: return "ORDERED";
		case SOURCE::WEIGHTED: return "WEIGHTED";
		case SOURCE::GLOBAL: return "GLOBAL";
		default: return "UNKNOWN";
		}
	}

	const char* DescribeValtanDecisionResult(
		const LostArk::Shared::VALTAN_DECISION_TRACE_RESULT result)
	{
		using RESULT = LostArk::Shared::VALTAN_DECISION_TRACE_RESULT;
		switch (result)
		{
		case RESULT::SELECTED: return "SELECTED";
		case RESULT::WAITING_FOR_INTRO_RANGE:
			return "WAITING_FOR_INTRO_RANGE";
		case RESULT::NO_ELIGIBLE_PATTERN: return "NO_ELIGIBLE_PATTERN";
		case RESULT::NO_VALID_TARGET: return "NO_VALID_TARGET";
		case RESULT::CATALOG_UNAVAILABLE: return "CATALOG_UNAVAILABLE";
		case RESULT::MECHANIC_RESET_REQUIRED:
			return "MECHANIC_RESET_REQUIRED";
		default: return "UNKNOWN";
		}
	}

	const char* DescribeValtanDecisionQueryResult(
		const LostArk::Shared::VALTAN_DECISION_TRACE_QUERY_RESULT result)
	{
		using RESULT = LostArk::Shared::VALTAN_DECISION_TRACE_QUERY_RESULT;
		switch (result)
		{
		case RESULT::TRACE: return "TRACE";
		case RESULT::UNCHANGED: return "UNCHANGED";
		case RESULT::REJECTED_RELEASE_BUILD: return "REJECTED_RELEASE_BUILD";
		case RESULT::REJECTED_WRONG_WORLD: return "REJECTED_WRONG_WORLD";
		case RESULT::REJECTED_NO_BOSS: return "REJECTED_NO_BOSS";
		case RESULT::NO_TRACE: return "NO_TRACE";
		default: return "UNKNOWN";
		}
	}

	std::string DescribeValtanDecisionExclusions(const std::uint32_t mask)
	{
		using namespace LostArk::Shared;
		if (VALTAN_DECISION_TRACE_EXCLUDE_NONE == mask)
			return "ELIGIBLE";
		std::string result;
		const auto append = [&result](const char* label)
		{
			if (!result.empty()) result += " | ";
			result += label;
		};
		if (0u != (mask & VALTAN_DECISION_TRACE_EXCLUDE_WRONG_SELECTION_KIND))
			append("WRONG_KIND");
		if (0u != (mask & VALTAN_DECISION_TRACE_EXCLUDE_INTRO_ROW))
			append("INTRO_ROW");
		if (0u != (mask & VALTAN_DECISION_TRACE_EXCLUDE_NOT_IN_SELECTION_SET))
			append("NOT_IN_SET");
		if (0u != (mask & VALTAN_DECISION_TRACE_EXCLUDE_ARMOR_MISMATCH))
			append("ARMOR");
		if (0u != (mask & VALTAN_DECISION_TRACE_EXCLUDE_PHASE_REQUIREMENT))
			append("PHASE_REQUIREMENT");
		if (0u != (mask & VALTAN_DECISION_TRACE_EXCLUDE_PHASE_RANGE))
			append("PHASE_RANGE");
		if (0u != (mask & VALTAN_DECISION_TRACE_EXCLUDE_HEALTH_BAR_RANGE))
			append("HEALTH_BAR");
		if (0u != (mask & VALTAN_DECISION_TRACE_EXCLUDE_NO_TARGET))
			append("NO_TARGET");
		if (0u != (mask & VALTAN_DECISION_TRACE_EXCLUDE_BELOW_MINIMUM_RANGE))
			append("BELOW_RANGE");
		if (0u != (mask & VALTAN_DECISION_TRACE_EXCLUDE_ABOVE_MAXIMUM_RANGE))
			append("ABOVE_RANGE");
		if (0u != (mask & VALTAN_DECISION_TRACE_EXCLUDE_COOLDOWN))
			append("COOLDOWN");
		if (0u != (mask & VALTAN_DECISION_TRACE_EXCLUDE_SOFT_REPEAT_BLOCKED))
			append("REPEAT_BLOCKED");
		if (0u != (mask & VALTAN_DECISION_TRACE_EXCLUDE_SOFT_REPEAT_RELAXED))
			append("REPEAT_RELAXED");
		if (0u != (mask & VALTAN_DECISION_TRACE_EXCLUDE_DISABLED))
			append("DISABLED");
		if (0u != (mask & VALTAN_DECISION_TRACE_EXCLUDE_UNRESOLVED_DEFINITION))
			append("UNRESOLVED");
		return result;
	}
	#endif

	std::string Quote(const std::string& value)
	{
		return "\"" + CDataJson::Escape(value) + "\"";
	}

	std::string FormatJsonNumber(double value)
	{
		if (0.0 == value)
			value = 0.0;
		char buffer[64]{};
		const auto converted = std::to_chars(
			buffer, buffer + sizeof(buffer), value, std::chars_format::general);
		if (std::errc{} != converted.ec)
			return "0";
		return std::string(buffer, converted.ptr);
	}

	bool DurableWrite(const std::filesystem::path& path,
		const std::string& text, std::string& status)
	{
		FILE* file = nullptr;
		if (0 != _wfopen_s(&file, path.c_str(), L"wb") || nullptr == file)
		{
			status = "Could not open balance staging file.";
			return false;
		}
		const bool wrote = text.size() ==
			fwrite(text.data(), 1u, text.size(), file);
		const bool flushed = 0 == fflush(file) && 0 == _commit(_fileno(file));
		const bool closed = 0 == fclose(file);
		if (!wrote || !flushed || !closed)
		{
			std::error_code error;
			std::filesystem::remove(path, error);
			status = "Could not durably write balance staging file.";
			return false;
		}
		return true;
	}

	bool ParseStagedJson(const std::filesystem::path& path, std::string& status)
	{
		std::ifstream input(path, std::ios::binary);
		if (!input)
		{
			status = "Balance staging file disappeared.";
			return false;
		}
		const std::string text{
			std::istreambuf_iterator<char>(input),
			std::istreambuf_iterator<char>() };
		DATA_JSON_VALUE root;
		return CDataJson::Parse(text, root, status) && root.Is_Object();
	}

	std::string ReadTextFile(const std::filesystem::path& path)
	{
		std::ifstream input(path, std::ios::binary);
		if (!input)
			return {};
		return {
			std::istreambuf_iterator<char>(input),
			std::istreambuf_iterator<char>() };
	}

	std::string TrimWhitespace(std::string value)
	{
		const auto isWhitespace = [](const unsigned char character)
		{
			return 0 != std::isspace(character);
		};
		const auto begin = std::find_if_not(value.begin(), value.end(), isWhitespace);
		const auto end = std::find_if_not(value.rbegin(), value.rend(), isWhitespace).base();
		return begin < end ? std::string(begin, end) : std::string{};
	}

	bool IsLowerSha256(const std::string& value)
	{
		return 64u == value.size() && std::all_of(value.begin(), value.end(),
			[](const unsigned char character)
			{
				return (character >= '0' && character <= '9') ||
					(character >= 'a' && character <= 'f');
			});
	}

	bool EqualPathComponent(
		const std::filesystem::path& left,
		const std::filesystem::path& right)
	{
		const std::wstring leftValue = left.native();
		const std::wstring rightValue = right.native();
		if (leftValue.size() != rightValue.size())
			return false;
		for (std::size_t index = 0u; index < leftValue.size(); ++index)
		{
			if (std::towlower(leftValue[index]) !=
				std::towlower(rightValue[index]))
			{
				return false;
			}
		}
		return true;
	}

	bool IsPathInsideOrEqual(
		const std::filesystem::path& root,
		const std::filesystem::path& candidate)
	{
		auto rootIterator = root.begin();
		auto candidateIterator = candidate.begin();
		for (; rootIterator != root.end();
			++rootIterator, ++candidateIterator)
		{
			if (candidateIterator == candidate.end() ||
				!EqualPathComponent(*rootIterator, *candidateIterator))
			{
				return false;
			}
		}
		return true;
	}

	bool ResolveFixedValtanAuthoringPath(
		const std::filesystem::path& repositoryRoot,
		const std::filesystem::path& relativePath,
		std::filesystem::path& output,
		std::string& status)
	{
		if (repositoryRoot.empty() || relativePath.empty() ||
			relativePath.is_absolute() || relativePath.has_root_path())
		{
			status = "Saved Valtan authoring path is not a fixed repository-relative path.";
			return false;
		}
		for (const std::filesystem::path& component : relativePath)
		{
			if (component == L".." || component == L".")
			{
				status = "Saved Valtan authoring path contains a traversal component.";
				return false;
			}
		}

		std::error_code error;
		const std::filesystem::path canonicalRoot =
			std::filesystem::weakly_canonical(repositoryRoot, error);
		if (error || canonicalRoot.empty())
		{
			status = "Could not canonicalize the fixed repository root.";
			return false;
		}
		const DWORD rootAttributes = GetFileAttributesW(canonicalRoot.c_str());
		if (INVALID_FILE_ATTRIBUTES == rootAttributes ||
			0u != (rootAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
		{
			status = "The fixed repository root is missing or is a reparse point.";
			return false;
		}

		std::filesystem::path current = canonicalRoot;
		for (const std::filesystem::path& component : relativePath)
		{
			current /= component;
			const DWORD attributes = GetFileAttributesW(current.c_str());
			if (INVALID_FILE_ATTRIBUTES == attributes)
			{
				const DWORD pathError = GetLastError();
				if (ERROR_FILE_NOT_FOUND == pathError ||
					ERROR_PATH_NOT_FOUND == pathError)
				{
					break;
				}
				status = "Could not inspect saved Valtan authoring path (Win32 " +
					std::to_string(pathError) + ").";
				return false;
			}
			if (0u != (attributes & FILE_ATTRIBUTE_REPARSE_POINT))
			{
				status = "Saved Valtan authoring path crosses a symlink or reparse point.";
				return false;
			}
		}

		const std::filesystem::path candidate =
			std::filesystem::weakly_canonical(canonicalRoot / relativePath, error);
		if (error || candidate.empty() ||
			!IsPathInsideOrEqual(canonicalRoot, candidate))
		{
			status = "Saved Valtan authoring path escapes the fixed repository root.";
			return false;
		}
		output = candidate;
		return true;
	}

	bool ReadAbsoluteJsonObject(
		const std::filesystem::path& path,
		DATA_JSON_VALUE& output,
		std::string& status)
	{
		std::ifstream input(path, std::ios::binary);
		if (!input)
		{
			status = "Missing saved Valtan authoring document: " + path.string();
			return false;
		}
		const std::string text{
			std::istreambuf_iterator<char>(input),
			std::istreambuf_iterator<char>() };
		if (!CDataJson::Parse(text, output, status) || !output.Is_Object())
		{
			status = "Invalid saved Valtan authoring JSON at " + path.string() +
				": " + status;
			return false;
		}
		return true;
	}

	struct VALTAN_PIPELINE_RESULT final
	{
		bool ok = false;
		std::string command;
		std::string sourceRevision;
		std::string authoringRevision;
		std::string candidateRevision;
		std::string applyClass;
		std::string diagnostic;
		bool hasAuthoringRevisionField = false;
		bool hasApplyClassField = false;
		std::string gameplaySourceRevision;
		std::string presentationSourceRevision;
		bool hasSplitJoinValidatedField = false;
		bool splitJoinValidated = false;
	};

	bool ParseValtanPipelineResult(const std::string& captured,
		VALTAN_PIPELINE_RESULT& output, std::string& status)
	{
		DATA_JSON_VALUE root;
		const std::string text = TrimWhitespace(captured);
		if (!CDataJson::Parse(text, root, status) || !root.Is_Object() ||
			!HasSchemaVersion(root, "lostark.valtan-tuning-command-result", 1u) ||
			!ReadString(root, "command", output.command) ||
			!ReadBoolean(root, "ok", output.ok))
		{
			status = "Valtan pipeline returned a malformed structured result: " + status;
			return false;
		}
		const DATA_JSON_VALUE* sourceRevision = root.Find("sourceRevision");
		const DATA_JSON_VALUE* candidateRevision = root.Find("candidateRevision");
		const DATA_JSON_VALUE* payload = Field(root, "payload", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* errors = Field(root, "errors", DATA_JSON_TYPE::ARRAY);
		if (nullptr == sourceRevision ||
			(!sourceRevision->Is_Null() && !sourceRevision->Is_String()) ||
			nullptr == candidateRevision ||
			(!candidateRevision->Is_Null() && !candidateRevision->Is_String()) ||
			nullptr == payload || nullptr == errors)
		{
			status = "Valtan pipeline result fields are incomplete.";
			return false;
		}
		if (sourceRevision->Is_String())
			output.sourceRevision = sourceRevision->Get_String();
		if (candidateRevision->Is_String())
			output.candidateRevision = candidateRevision->Get_String();
		const DATA_JSON_VALUE* authoringRevision = payload->Find("authoringRevision");
		if (nullptr != authoringRevision)
		{
			output.hasAuthoringRevisionField = true;
			if (authoringRevision->Is_String())
				output.authoringRevision = authoringRevision->Get_String();
			else if (!authoringRevision->Is_Null())
			{
				status = "Valtan pipeline authoringRevision is neither null nor a string.";
				return false;
			}
		}
		const DATA_JSON_VALUE* applyClass = payload->Find("applyClass");
		if (nullptr != applyClass)
		{
			output.hasApplyClassField = true;
			if (!applyClass->Is_String())
			{
				status = "Valtan pipeline applyClass is not a string.";
				return false;
			}
			output.applyClass = applyClass->Get_String();
			if (output.applyClass != "HOT_RELOAD" &&
				output.applyClass != "ENCOUNTER_RESET" &&
				output.applyClass != "SERVER_RESTART")
			{
				status = "Valtan pipeline applyClass is not recognized.";
				return false;
			}
		}
		if (output.command != "PUBLISH_CANDIDATE" &&
			output.hasApplyClassField)
		{
			status = "Valtan pipeline returned applyClass for a non-publish command.";
			return false;
		}
		if (output.ok && output.command == "PUBLISH_CANDIDATE" &&
			!output.hasApplyClassField)
		{
			status = "Valtan pipeline publish result is missing applyClass.";
			return false;
		}
		const DATA_JSON_VALUE* files = payload->Find("files");
		if (nullptr != files)
		{
			if (!files->Is_Array())
			{
				status = "Valtan pipeline source files field is not an array.";
				return false;
			}
			for (const DATA_JSON_VALUE& file : files->Get_Array())
			{
				std::string path;
				std::string revision;
				if (!file.Is_Object() || !ReadString(file, "path", path) ||
					!ReadString(file, "sha256", revision))
				{
					status = "Valtan pipeline source file identity is malformed.";
					return false;
				}
				std::string* target = nullptr;
				if (path == VALTAN_GAMEPLAY_SOURCE_PATH)
					target = &output.gameplaySourceRevision;
				else if (path == VALTAN_PRESENTATION_SOURCE_PATH)
					target = &output.presentationSourceRevision;
				if (nullptr == target)
					continue;
				if (!target->empty() || !IsLowerSha256(revision))
				{
					status = "Valtan split source identity is duplicated or invalid: " + path;
					return false;
				}
				*target = std::move(revision);
			}
		}
		const DATA_JSON_VALUE* splitJoinValidated =
			payload->Find("splitJoinValidated");
		if (nullptr != splitJoinValidated)
		{
			output.hasSplitJoinValidatedField = true;
			if (!splitJoinValidated->Is_Boolean())
			{
				status = "Valtan pipeline splitJoinValidated is not a boolean.";
				return false;
			}
			output.splitJoinValidated = splitJoinValidated->Get_Boolean();
		}
		if (!errors->Get_Array().empty())
		{
			const DATA_JSON_VALUE& error = errors->Get_Array().front();
			std::string document;
			std::string path;
			std::string patternId;
			std::string stageId;
			std::string field;
			std::string errorCode;
			std::string message;
			ReadString(error, "document", document);
			ReadString(error, "path", path);
			ReadString(error, "patternId", patternId);
			ReadString(error, "stageId", stageId);
			ReadString(error, "field", field);
			ReadString(error, "errorCode", errorCode);
			ReadString(error, "message", message);
			output.diagnostic = errorCode + " | " + document;
			if (!path.empty()) output.diagnostic += " | " + path;
			if (!patternId.empty()) output.diagnostic += " | " + patternId;
			if (!stageId.empty()) output.diagnostic += "/" + stageId;
			if (!field.empty()) output.diagnostic += " | " + field;
			if (!message.empty()) output.diagnostic += " | " + message;
		}
		status.clear();
		return true;
	}

	const VALTAN_PATTERN_VIEW* FindValtanPattern(
		const VALTAN_PATTERN_TREE_VIEW& tree, const std::string& patternId)
	{
		for (const auto* group : { &tree.Gimmicks, &tree.Rotation })
		{
			const auto found = std::find_if(group->begin(), group->end(),
				[&](const VALTAN_PATTERN_VIEW& pattern)
				{ return pattern.strPatternId == patternId; });
			if (group->end() != found)
				return &*found;
		}
		return nullptr;
	}

	VALTAN_PATTERN_VIEW* FindValtanPattern(
		VALTAN_PATTERN_TREE_VIEW& tree, const std::string& patternId)
	{
		for (auto* group : { &tree.Gimmicks, &tree.Rotation })
		{
			const auto found = std::find_if(group->begin(), group->end(),
				[&](const VALTAN_PATTERN_VIEW& pattern)
				{ return pattern.strPatternId == patternId; });
			if (group->end() != found)
				return &*found;
		}
		return nullptr;
	}

	std::size_t CountManagedValtanPatterns(
		const VALTAN_PATTERN_TREE_VIEW& tree)
	{
		std::size_t count = 0u;
		for (const auto* group : { &tree.Gimmicks, &tree.Rotation })
		{
			count += static_cast<std::size_t>(std::count_if(
				group->begin(), group->end(),
				[](const VALTAN_PATTERN_VIEW& pattern)
				{
					return pattern.bAuthoringMasterManaged;
				}));
		}
		return count;
	}

	const VALTAN_STAGE_VIEW* FindValtanStage(
		const VALTAN_PATTERN_VIEW& pattern, const std::string& stageId)
	{
		const auto found = std::find_if(pattern.Stages.begin(), pattern.Stages.end(),
			[&](const VALTAN_STAGE_VIEW& stage)
			{ return stage.strStageId == stageId; });
		return pattern.Stages.end() == found ? nullptr : &*found;
	}

	VALTAN_STAGE_VIEW* FindValtanStage(
		VALTAN_PATTERN_VIEW& pattern, const std::string& stageId)
	{
		const auto found = std::find_if(pattern.Stages.begin(), pattern.Stages.end(),
			[&](const VALTAN_STAGE_VIEW& stage)
			{ return stage.strStageId == stageId; });
		return pattern.Stages.end() == found ? nullptr : &*found;
	}

	CBalanceTool::PATTERN_STAGE_EDIT BuildValtanStageDraft(
		const VALTAN_PATTERN_VIEW& pattern,
		const VALTAN_STAGE_VIEW& stage)
	{
		CBalanceTool::PATTERN_STAGE_EDIT draft{};
		draft.stageId = stage.strStageId;
		draft.actionId = stage.strActionId;
		draft.stageKind = stage.strStageKind;
		draft.durationMs = stage.iDurationMs;
		draft.hitShape = stage.strHitShape;
		draft.hitOuterRadius = stage.fHitOuterRadius;
		draft.hitInnerRadius = stage.fHitInnerRadius;
		draft.hitAngleDegrees = stage.fHitAngleDegrees;
		draft.hitLength = stage.fHitLength;
		draft.hitHalfWidth = stage.fHitHalfWidth;
		draft.hitCount = stage.iHitCount;
		draft.hitIntervalMs = stage.iHitIntervalMs;
		draft.hitDelayMs = stage.iHitDelayMs;
		draft.hitOffsetsMs = stage.HitOffsetsMs;
		draft.damageProfileId = stage.strServerDamageProfileId;
		draft.pushRangeM = stage.fPushRangeM;
		draft.pushMs = stage.iPushMs;
		draft.knockdown = stage.bKnockdown;
		draft.downMs = stage.iDownMs;
		draft.playerResponse = stage.strPlayerResponse;
		draft.attachmentSlot = stage.strAttachmentSlot;
		draft.durationEditable = !pattern.bManualServerAudition;
		/* Adding/removing a hit also owns DamageProfile selection.  Workbench
		   deliberately keeps that ownership in Balance Tool and edits only an
		   already admitted hit contract. */
		draft.hitEditable = "NONE" != stage.strHitShape;
		return draft;
	}

	bool IsValtanStageGeometryValid(
		const CBalanceTool::PATTERN_STAGE_EDIT& stage)
	{
		const bool finite = std::isfinite(stage.hitOuterRadius) &&
			std::isfinite(stage.hitInnerRadius) &&
			std::isfinite(stage.hitAngleDegrees) &&
			std::isfinite(stage.hitLength) &&
			std::isfinite(stage.hitHalfWidth);
		if (!finite)
			return false;
		if ("NONE" == stage.hitShape)
		{
			return 0.0 == stage.hitOuterRadius &&
				0.0 == stage.hitInnerRadius &&
				0.0 == stage.hitAngleDegrees &&
				0.0 == stage.hitLength &&
				0.0 == stage.hitHalfWidth;
		}
		if ("CIRCLE" == stage.hitShape)
		{
			return stage.hitOuterRadius > 0.0 &&
				stage.hitOuterRadius <= 1000.0 &&
				0.0 == stage.hitInnerRadius &&
				0.0 == stage.hitAngleDegrees &&
				0.0 == stage.hitLength &&
				0.0 == stage.hitHalfWidth;
		}
		if ("RING" == stage.hitShape)
		{
			return stage.hitInnerRadius > 0.0 &&
				stage.hitOuterRadius > stage.hitInnerRadius &&
				stage.hitOuterRadius <= 1000.0 &&
				0.0 == stage.hitAngleDegrees &&
				0.0 == stage.hitLength &&
				0.0 == stage.hitHalfWidth;
		}
		if ("CONE" == stage.hitShape)
		{
			return stage.hitAngleDegrees > 0.0 &&
				stage.hitAngleDegrees <= 180.0 &&
				stage.hitLength > 0.0 && stage.hitLength <= 1000.0 &&
				0.0 == stage.hitOuterRadius &&
				0.0 == stage.hitInnerRadius &&
				0.0 == stage.hitHalfWidth;
		}
		const bool boxLike = "BOX" == stage.hitShape ||
			"CROSS" == stage.hitShape ||
			"SIX_DIRECTIONS" == stage.hitShape;
		return boxLike && stage.hitLength > 0.0 &&
			stage.hitLength <= 1000.0 && stage.hitHalfWidth > 0.0 &&
			stage.hitHalfWidth <= 1000.0 &&
			0.0 == stage.hitOuterRadius &&
			0.0 == stage.hitInnerRadius &&
			0.0 == stage.hitAngleDegrees;
	}
}


Client::CBalanceTool::CBalanceTool()
{
	Reload();
}

void Client::CBalanceTool::Open()
{
	m_open = true;
	m_focusPending = true;
}

bool Client::CBalanceTool::Get_ValtanStageDurationDraft(
	const std::string& patternId,
	const std::string& stageId,
	std::uint32_t& durationMs,
	std::string& status) const
{
	PATTERN_STAGE_EDIT stage{};
	if (!Get_ValtanStageDraft(patternId, stageId, stage, status))
		return false;
	durationMs = stage.durationMs;
	return true;
}

bool Client::CBalanceTool::Set_ValtanStageDurationDraft(
	const std::string& patternId,
	const std::string& stageId,
	const std::uint32_t durationMs,
	std::string& status)
{
	PATTERN_STAGE_EDIT stage{};
	if (!Get_ValtanStageDraft(patternId, stageId, stage, status))
		return false;
	stage.durationMs = durationMs;
	return Set_ValtanStageDraft(patternId, stageId, stage, status);
}

bool Client::CBalanceTool::Get_ValtanStageDraft(
	const std::string& patternId,
	const std::string& stageId,
	PATTERN_STAGE_EDIT& stage,
	std::string& status) const
{
	const VALTAN_PATTERN_VIEW* pattern =
		FindValtanPattern(m_valtanPatternTree, patternId);
	if (nullptr == pattern || !pattern->bAuthoringMasterManaged)
	{
		status = "Valtan draft pattern is not authoring-master managed: " +
			patternId + ".";
		return false;
	}
	const VALTAN_STAGE_VIEW* admitted = FindValtanStage(*pattern, stageId);
	if (nullptr == admitted)
	{
		status = "Valtan draft stage is not admitted: " + patternId + "/" +
			stageId + ".";
		return false;
	}
	stage = BuildValtanStageDraft(*pattern, *admitted);
	status.clear();
	return true;
}

bool Client::CBalanceTool::Set_ValtanStageDraft(
	const std::string& patternId,
	const std::string& stageId,
	const PATTERN_STAGE_EDIT& candidate,
	std::string& status)
{
	VALTAN_PATTERN_VIEW* pattern =
		FindValtanPattern(m_valtanPatternTree, patternId);
	if (nullptr == pattern || !pattern->bAuthoringMasterManaged)
	{
		status = "Valtan stage edit rejected: pattern is not authoring-master managed: " +
			patternId + ".";
		return false;
	}
	VALTAN_STAGE_VIEW* stage = FindValtanStage(*pattern, stageId);
	if (nullptr == stage)
	{
		status = "Valtan stage edit rejected: stage is not admitted: " +
			patternId + "/" + stageId + ".";
		return false;
	}
	const PATTERN_STAGE_EDIT current = BuildValtanStageDraft(*pattern, *stage);
	if (candidate.stageId != current.stageId ||
		candidate.actionId != current.actionId ||
		candidate.stageKind != current.stageKind ||
		candidate.damageProfileId != current.damageProfileId ||
		candidate.playerResponse != current.playerResponse ||
		candidate.attachmentSlot != current.attachmentSlot ||
		candidate.hitOffsetsMs != current.hitOffsetsMs)
	{
		status = "Valtan stage edit rejected: stable identity, DamageProfile, response, and explicit-offset ownership are read-only in Workbench.";
		return false;
	}
	if (candidate.durationMs < 1u || candidate.durationMs > 600000u)
	{
		status = "Valtan stage duration must be between 1 and 600000 ms.";
		return false;
	}
	if (!current.durationEditable &&
		candidate.durationMs != current.durationMs)
	{
		status = "Manual Server audition duration is locked to its promoted animation wall-clock: " +
			patternId + "/" + stageId + ".";
		return false;
	}
	const bool hitChanged =
		candidate.hitShape != current.hitShape ||
		candidate.hitOuterRadius != current.hitOuterRadius ||
		candidate.hitInnerRadius != current.hitInnerRadius ||
		candidate.hitAngleDegrees != current.hitAngleDegrees ||
		candidate.hitLength != current.hitLength ||
		candidate.hitHalfWidth != current.hitHalfWidth ||
		candidate.hitCount != current.hitCount ||
		candidate.hitIntervalMs != current.hitIntervalMs ||
		candidate.hitDelayMs != current.hitDelayMs ||
		candidate.pushRangeM != current.pushRangeM ||
		candidate.pushMs != current.pushMs ||
		candidate.knockdown != current.knockdown ||
		candidate.downMs != current.downMs;
	if (hitChanged && !current.hitEditable)
	{
		status = "Valtan stage hit edit rejected: adding a new hit also requires a typed DamageProfile choice in Balance Tool.";
		return false;
	}
	if (("NONE" == current.hitShape) != ("NONE" == candidate.hitShape))
	{
		status = "Valtan stage hit edit rejected: Workbench cannot add or remove the DamageProfile-owned hit contract.";
		return false;
	}
	if (!IsValtanStageGeometryValid(candidate))
	{
		status = "Valtan stage collider geometry is invalid: " + patternId +
			"/" + stageId + ".";
		return false;
	}
	if ("NONE" == candidate.hitShape)
	{
		if (0u != candidate.hitCount || 0u != candidate.hitIntervalMs ||
			0u != candidate.hitDelayMs || !candidate.damageProfileId.empty() ||
			0.0 != candidate.pushRangeM || 0u != candidate.pushMs ||
			candidate.knockdown || 0u != candidate.downMs)
		{
			status = "Valtan NONE stage cannot own a hit schedule or player reaction.";
			return false;
		}
	}
	else
	{
		const bool explicitOffsets = !candidate.hitOffsetsMs.empty();
		const bool explicitScheduleValid = explicitOffsets &&
			candidate.hitCount == candidate.hitOffsetsMs.size() &&
			0u == candidate.hitIntervalMs && 0u == candidate.hitDelayMs &&
			candidate.hitOffsetsMs.back() < candidate.durationMs;
		const std::uint64_t finalIntervalHit =
			static_cast<std::uint64_t>(candidate.hitDelayMs) +
			static_cast<std::uint64_t>(candidate.hitCount -
				(0u == candidate.hitCount ? 0u : 1u)) *
			candidate.hitIntervalMs;
		const bool intervalScheduleValid = !explicitOffsets &&
			candidate.hitCount > 0u && candidate.hitCount <= 64u &&
			(1u == candidate.hitCount ? 0u == candidate.hitIntervalMs :
				candidate.hitIntervalMs > 0u) &&
			finalIntervalHit < candidate.durationMs;
		const bool validPush = std::isfinite(candidate.pushRangeM) &&
			std::abs(candidate.pushRangeM) <= 20.0 &&
			candidate.pushMs <= 600000u && candidate.downMs <= 600000u &&
			((0.0 == candidate.pushRangeM) == (0u == candidate.pushMs)) &&
			(candidate.knockdown == (0u != candidate.downMs));
		const bool captureReactionValid =
			"CAPTURE" != candidate.playerResponse ||
			(0.0 == candidate.pushRangeM && 0u == candidate.pushMs &&
				!candidate.knockdown && 0u == candidate.downMs);
		if ((!explicitScheduleValid && !intervalScheduleValid) ||
			candidate.damageProfileId.empty() || !validPush ||
			!captureReactionValid)
		{
			status = "Valtan stage hit schedule or player reaction is invalid: " +
				patternId + "/" + stageId + ".";
			return false;
		}
	}

	const bool durationChanged = candidate.durationMs != current.durationMs;
	if (!durationChanged && !hitChanged)
	{
		status = "Valtan stage draft is unchanged.";
		return true;
	}
	stage->iDurationMs = candidate.durationMs;
	stage->strHitShape = candidate.hitShape;
	stage->fHitOuterRadius = static_cast<float>(candidate.hitOuterRadius);
	stage->fHitInnerRadius = static_cast<float>(candidate.hitInnerRadius);
	stage->fHitAngleDegrees = static_cast<float>(candidate.hitAngleDegrees);
	stage->fHitLength = static_cast<float>(candidate.hitLength);
	stage->fHitHalfWidth = static_cast<float>(candidate.hitHalfWidth);
	stage->iHitCount = candidate.hitCount;
	stage->iHitIntervalMs = candidate.hitIntervalMs;
	stage->iHitDelayMs = candidate.hitDelayMs;
	stage->fPushRangeM = static_cast<float>(candidate.pushRangeM);
	stage->iPushMs = candidate.pushMs;
	stage->bKnockdown = candidate.knockdown;
	stage->iDownMs = candidate.downMs;
	MarkDirty(true);
	status = "Staged typed Server gameplay edit for " + patternId + "/" +
		stageId +
		". Press Save to validate the joined revision before Product generation.";
	return true;
}

bool Client::CBalanceTool::Save_ValtanProduct(std::string& status)
{
	std::string stepStatus;
	if (!Validate_ValtanDraft(stepStatus))
	{
		status = "Save validation failed; source and active runtime were preserved: " +
			stepStatus;
		return false;
	}

	const bool hadDirtyDraft = m_dirty;
	if (hadDirtyDraft && !Save_ValtanAuthoring(stepStatus))
	{
		status = "Save authoring failed; active runtime was preserved: " +
			stepStatus;
		return false;
	}

	if (!Publish_ValtanCandidate(stepStatus))
	{
		status = hadDirtyDraft ?
			"Authoring was saved, but Product bundle generation failed. The active runtime is unchanged; press Save again after fixing the diagnostic: " +
				stepStatus :
			"Product bundle generation failed; source and active runtime are unchanged: " +
				stepStatus;
		return false;
	}

	const std::string revisionLabel = m_valtanCandidateRevision.empty() ?
		std::string("UNKNOWN") : m_valtanCandidateRevision.substr(0u, 12u);
	if ("HOT_RELOAD" != m_valtanCandidateApplyClass)
	{
		status = "Saved Product revision " + revisionLabel +
			". This change requires " +
			(m_valtanCandidateApplyClass.empty() ? std::string("SERVER_RESTART") :
				m_valtanCandidateApplyClass) +
			"; the currently running encounter was left unchanged.";
		return true;
	}

	const CNetworkManager::GAMEPLAY_REVISION_CLIENT_STATE& revisionState =
		CNetworkManager::Get().Get_GameplayRevisionState();
	if (!CNetworkManager::Get().Is_Connected() ||
		!revisionState.ServerActiveRevision.Is_Valid())
	{
		status = "Saved Product revision " + revisionLabel +
			". No admitted Debug Server revision is connected, so it will be active after Server restart/re-entry.";
		return true;
	}
	if (CValtanTuningCommandService::Get().Has_PendingCommand())
	{
		status = "Saved Product revision " + revisionLabel +
			". A previous live-update transaction is still pending; active runtime was not replaced.";
		return true;
	}
	if (!Apply_ValtanRevision(stepStatus))
	{
		status = "Saved Product revision " + revisionLabel +
			". Live apply was not submitted and the active runtime was preserved: " +
			stepStatus;
		return true;
	}
	status = "Saved Product revision " + revisionLabel +
		" and submitted the Server/Client tick-boundary live update. Runtime becomes active only after the coordinator reports COMMITTED.";
	return true;
}

bool Client::CBalanceTool::Validate_ValtanDraft(std::string& status)
{
	return RunValtanDraftCommand(L"ValidateDraft", status);
}

bool Client::CBalanceTool::Save_ValtanAuthoring(std::string& status)
{
	return RunValtanDraftCommand(L"SaveAuthoring", status);
}

bool Client::CBalanceTool::Publish_ValtanCandidate(std::string& status)
{
	return RunValtanDraftCommand(L"PublishCandidate", status);
}

bool Client::CBalanceTool::Apply_ValtanRevision(std::string& status)
{
	if (m_dirty)
	{
		status = "Apply Revision blocked: save the current Valtan authoring draft first.";
		return false;
	}
	if (m_valtanCandidateRevision.empty())
	{
		status = "Apply Revision blocked: publish a candidate first.";
		return false;
	}
	if ("HOT_RELOAD" != m_valtanCandidateApplyClass)
	{
		status = "Apply Revision blocked: candidate apply class is " +
			(m_valtanCandidateApplyClass.empty() ? std::string("NONE") :
				m_valtanCandidateApplyClass) + ".";
		return false;
	}
	return RequestValtanHotReload(status);
}

void Client::CBalanceTool::MarkDirty(const bool changed)
{
	if (changed)
	{
		m_dirty = true;
		m_valtanDraftValidated = false;
		m_valtanCandidateRevision.clear();
		m_valtanCandidateApplyClass.clear();
	}
}

bool Client::CBalanceTool::Reload()
{
	DATA_JSON_VALUE playerRoot;
	DATA_JSON_VALUE skillRoot;
	DATA_JSON_VALUE damageRoot;
	DATA_JSON_VALUE bossRoot;
	DATA_JSON_VALUE encounterRoot;
	DATA_JSON_VALUE receiptRoot;
	std::string status;
	if (!ReadJson(L"Balance/PlayerProfiles.json", playerRoot, status) ||
		!ReadJson(L"Balance/PlayerSkills.json", skillRoot, status) ||
		!ReadJson(L"Balance/DamageProfiles.json", damageRoot, status) ||
		!ReadJson(L"Balance/BossProfiles.json", bossRoot, status) ||
		!ReadJson(L"Encounters/Valtan/ValtanEncounter.json", encounterRoot, status) ||
		!ReadJson(L"Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json",
			receiptRoot, status))
	{
		m_status = "Reload failed: " + status;
		return false;
	}
	std::uint32_t bossFormatVersion = 0u;
	std::uint32_t encounterFormatVersion = 0u;
	if (!IsExactObject(playerRoot, { "schema", "formatVersion", "players" }) ||
		!HasSchemaVersion(playerRoot, "lostark.player-profiles", 2u) ||
		!IsExactObject(skillRoot, { "schema", "formatVersion", "skills" }) ||
		!HasSchemaVersion(skillRoot, "lostark.player-skills", 3u) ||
		!IsExactObject(damageRoot, { "schema", "formatVersion", "profiles" }) ||
		!HasSchemaVersion(damageRoot, "lostark.damage-profiles", 2u) ||
		!IsExactObject(bossRoot, { "schema", "formatVersion", "bosses" }) ||
		!ReadU32(bossRoot, "formatVersion", bossFormatVersion) ||
		(bossFormatVersion != 3u && bossFormatVersion != 4u) ||
		nullptr == Field(bossRoot, "schema", DATA_JSON_TYPE::STRING) ||
		bossRoot.Find("schema")->Get_String() != "lostark.boss-profiles" ||
		!IsExactObject(encounterRoot, { "schema", "formatVersion", "encounterId",
			"bossArchetypeId", "authority", "fixedTickHz", "introPatternId",
			"states", "patterns" }) ||
		!ReadU32(encounterRoot, "formatVersion", encounterFormatVersion) ||
		encounterFormatVersion != 4u ||
		nullptr == Field(encounterRoot, "schema", DATA_JSON_TYPE::STRING) ||
		encounterRoot.Find("schema")->Get_String() != "lostark.encounter-profile" ||
		!IsExactObject(receiptRoot, { "schema", "formatVersion", "sourceBuildId",
			"referenceSkillLevel", "extractorSha256", "sourceFiles", "coverage", "entries" }) ||
		!HasSchemaVersion(receiptRoot, "lostark.balance-provenance-receipt", 1u))
	{
		m_status = "Reload failed: schema/version or root fields are not exact.";
		return false;
	}

	std::vector<PLAYER_EDIT> players;
	std::vector<SKILL_EDIT> skills;
	std::vector<DAMAGE_EDIT> damageProfiles;
	std::vector<BOSS_EDIT> bosses;
	std::vector<PATTERN_EDIT> patterns;
	std::vector<ENCOUNTER_STATE_EDIT> states;
	VALTAN_PATTERN_TREE_VIEW valtanPatternTree;
	std::vector<LEGACY_PATTERN_SUMMARY> legacyPatterns;
	std::string valtanPatternStatus;
	VALTAN_AXE_VOLLEY_EDIT valtanAxeVolley;
	std::unordered_map<std::string, std::string> bases;
	const DATA_JSON_VALUE* playerArray = Field(playerRoot, "players", DATA_JSON_TYPE::ARRAY);
	const DATA_JSON_VALUE* skillArray = Field(skillRoot, "skills", DATA_JSON_TYPE::ARRAY);
	const DATA_JSON_VALUE* damageArray = Field(damageRoot, "profiles", DATA_JSON_TYPE::ARRAY);
	const DATA_JSON_VALUE* bossArray = Field(bossRoot, "bosses", DATA_JSON_TYPE::ARRAY);
	const DATA_JSON_VALUE* productPatternArray =
		Field(encounterRoot, "patterns", DATA_JSON_TYPE::ARRAY);
	/* Generated Encounter rows are runtime Product, not Balance Tool authoring.
	   Keep the legacy parser below on an empty array so it can no longer turn a
	   v4 Product into a lossy v3 document. The strict-joined gameplay and
	   presentation authoring view is staged through CValtanPatternTree below. */
	const DATA_JSON_VALUE emptyPatternArray =
		DATA_JSON_VALUE::Array(DATA_JSON_VALUE::ARRAY{});
	const DATA_JSON_VALUE* patternArray = &emptyPatternArray;
	const DATA_JSON_VALUE* stateArray = Field(encounterRoot, "states", DATA_JSON_TYPE::ARRAY);
	const DATA_JSON_VALUE* receiptArray = Field(receiptRoot, "entries", DATA_JSON_TYPE::ARRAY);
	if (nullptr == playerArray || nullptr == skillArray || nullptr == damageArray ||
		nullptr == bossArray || nullptr == productPatternArray ||
		nullptr == stateArray ||
		nullptr == receiptArray)
	{
		m_status = "Reload failed: balance array is missing.";
		return false;
	}

	for (const DATA_JSON_VALUE& value : playerArray->Get_Array())
	{
		PLAYER_EDIT row{};
		if (!IsExactObject(value, { "characterClass", "maximumHp", "maximumResource",
			"resourceRegenPerSecond", "attackPower", "defense", "moveSpeed",
			"defenseStanceMoveSpeedScale", "maximumIdentity",
			"identityRegenPerSecond", "identityDrainPerSecond",
			"identityStanceSwitchCost", "identityCyclic", "defaultStance" }) ||
			!ReadString(value, "characterClass", row.characterClass) ||
			!ReadU32(value, "maximumHp", row.maximumHp) ||
			!ReadU32(value, "maximumResource", row.maximumResource) ||
			!ReadU32(value, "resourceRegenPerSecond", row.resourceRegenPerSecond) ||
			!ReadU32(value, "attackPower", row.attackPower) ||
			!ReadU32(value, "defense", row.defense) ||
			!ReadDouble(value, "moveSpeed", row.moveSpeed) ||
			!ReadDouble(value, "defenseStanceMoveSpeedScale",
				row.defenseStanceMoveSpeedScale) ||
			!ReadU32(value, "maximumIdentity", row.maximumIdentity) ||
			!ReadU32(value, "identityRegenPerSecond",
				row.identityRegenPerSecond) ||
			!ReadU32(value, "identityDrainPerSecond",
				row.identityDrainPerSecond) ||
			!ReadU32(value, "identityStanceSwitchCost",
				row.identityStanceSwitchCost) ||
			!ReadU32(value, "identityCyclic", row.identityCyclic) ||
			!ReadString(value, "defaultStance", row.defaultStance))
		{
			m_status = "Reload failed: invalid player profile.";
			return false;
		}
		players.push_back(std::move(row));
	}
	for (const DATA_JSON_VALUE& value : skillArray->Get_Array())
	{
		SKILL_EDIT row{};
		const DATA_JSON_VALUE* stagesValue = Field(value, "comboStages", DATA_JSON_TYPE::ARRAY);
		if (!IsExactObject(value, { "skillId", "staggerDamage", "partDamage",
			"counterPower", "characterClass", "inputSlot", "displayName",
			"actionId", "skillKind", "cooldownMs", "actionDurationMs", "hitTimeMs",
			"resourceCost", "identityCost", "movementDistance", "maximumRange", "serverDamageProfileId",
			"effectId", "requiredStance", "setsStance", "comboStages" }) ||
			!ReadU32(value, "skillId", row.skillId) ||
			!ReadU32(value, "staggerDamage", row.staggerDamage) ||
			!ReadU32(value, "partDamage", row.partDamage) ||
			!ReadU32(value, "counterPower", row.counterPower) ||
			!ReadString(value, "characterClass", row.characterClass) ||
			!ReadString(value, "inputSlot", row.inputSlot) ||
			!ReadString(value, "displayName", row.displayName) ||
			!ReadString(value, "actionId", row.actionId) ||
			!ReadString(value, "skillKind", row.skillKind) ||
			!ReadU32(value, "cooldownMs", row.cooldownMs) ||
			!ReadU32(value, "actionDurationMs", row.actionDurationMs) ||
			!ReadU32(value, "hitTimeMs", row.hitTimeMs) ||
			!ReadU32(value, "resourceCost", row.resourceCost) ||
			!ReadU32(value, "identityCost", row.identityCost) ||
			!ReadDouble(value, "movementDistance", row.movementDistance) ||
			!ReadDouble(value, "maximumRange", row.maximumRange) ||
			!ReadString(value, "serverDamageProfileId", row.damageProfileId) ||
			!ReadString(value, "effectId", row.effectId) ||
			!ReadString(value, "requiredStance", row.requiredStance) ||
			!ReadString(value, "setsStance", row.setsStance) ||
			nullptr == stagesValue)
		{
			m_status = "Reload failed: invalid skill definition.";
			return false;
		}
		for (const DATA_JSON_VALUE& stageValue : stagesValue->Get_Array())
		{
			COMBO_STAGE_EDIT stage{};
			if (!IsExactObject(stageValue, { "actionDurationMs", "hitTimeMs",
				"comboAdvanceMs", "inputOpenMs", "inputCloseMs" }) ||
				!ReadU32(stageValue, "actionDurationMs", stage.actionDurationMs) ||
				!ReadU32(stageValue, "hitTimeMs", stage.hitTimeMs) ||
				!ReadU32(stageValue, "comboAdvanceMs", stage.comboAdvanceMs) ||
				!ReadU32(stageValue, "inputOpenMs", stage.inputOpenMs) ||
				!ReadU32(stageValue, "inputCloseMs", stage.inputCloseMs))
			{
				m_status = "Reload failed: invalid combo stage.";
				return false;
			}
			row.comboStages.push_back(stage);
		}
		skills.push_back(std::move(row));
	}
	for (const DATA_JSON_VALUE& value : damageArray->Get_Array())
	{
		DAMAGE_EDIT row{};
		if (!IsExactObject(value, { "damageProfileId", "damageRatePercent" }) ||
			!ReadString(value, "damageProfileId", row.damageProfileId) ||
			!ReadU32(value, "damageRatePercent", row.damageRatePercent))
			return false;
		damageProfiles.push_back(std::move(row));
	}
	for (const DATA_JSON_VALUE& value : bossArray->Get_Array())
	{
		BOSS_EDIT row{};
		if (!ReadString(value, "archetypeId", row.archetypeId) ||
			!ReadString(value, "encounterId", row.encounterId) ||
			!ReadString(value, "displayName", row.displayName) ||
			!ReadU32(value, "maximumHp", row.maximumHp) ||
			!ReadU32(value, "maximumHealthBars", row.maximumHealthBars) ||
			!ReadU32(value, "attackPower", row.attackPower) ||
			!ReadDouble(value, "collisionRadius", row.collisionRadius) ||
			!ReadDouble(value, "engageDistance", row.engageDistance) ||
			!ReadDouble(value, "moveSpeed", row.moveSpeed))
			return false;
		const DATA_JSON_VALUE* phasePolicy = value.Find("phasePolicy");
		if (4u == bossFormatVersion)
		{
			if (nullptr == phasePolicy || !phasePolicy->Is_Object() ||
				!ReadString(*phasePolicy, "kind", row.phasePolicyKind))
				return false;
			if (row.phasePolicyKind == "AUTHORED_PATTERN_EVENT")
			{
				if (!IsExactObject(*phasePolicy, { "kind" }))
					return false;
			}
			else if (row.phasePolicyKind == "HEALTH_PERCENT_THRESHOLD")
			{
				if (!IsExactObject(*phasePolicy, { "kind", "thresholdPercent" }) ||
					!ReadU32(*phasePolicy, "thresholdPercent",
						row.phasePolicyThresholdPercent))
					return false;
			}
			else
				return false;
		}
		else
		{
			row.phasePolicyKind = "HEALTH_PERCENT_THRESHOLD";
			if (!ReadU32(value, "phaseTwoHpPercent",
					row.phasePolicyThresholdPercent))
				return false;
		}
		bosses.push_back(std::move(row));
	}
	for (const DATA_JSON_VALUE& value : patternArray->Get_Array())
	{
		PATTERN_EDIT row{};
		const DATA_JSON_VALUE* sourceActions =
			Field(value, "sourceActionIds", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* stages = Field(value, "stages", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* serverMotion = value.Find("serverMotion");
		const bool hasServerMotion = nullptr != serverMotion;
		const bool hasExactPatternFields = hasServerMotion ?
			IsExactObject(value, { "patternId", "displayName", "actionId", "sourceActionIds",
				"selectionMode", "minimumHealthBar", "maximumHealthBar",
				"triggerHealthBar", "triggerOrder", "armorRequirement",
				"phaseRequirement", "invulnerableWhileRunning",
				"selectionWeight", "maximumConsecutiveUses",
				"minimumRange", "maximumRange", "serverMotion", "stages" }) :
			IsExactObject(value, { "patternId", "displayName", "actionId", "sourceActionIds",
				"selectionMode", "minimumHealthBar", "maximumHealthBar",
				"triggerHealthBar", "triggerOrder", "armorRequirement",
				"phaseRequirement", "invulnerableWhileRunning",
				"selectionWeight", "maximumConsecutiveUses",
				"minimumRange", "maximumRange", "stages" });
		if (!hasExactPatternFields ||
			nullptr == sourceActions || sourceActions->Get_Array().empty() ||
			nullptr == stages || stages->Get_Array().empty() ||
			!ReadString(value, "patternId", row.patternId) ||
			!ReadString(value, "displayName", row.displayName) ||
			!ReadString(value, "actionId", row.actionId) ||
			!ReadString(value, "selectionMode", row.selectionMode) ||
			!ReadU32(value, "minimumHealthBar", row.minimumHealthBar) ||
			!ReadU32(value, "maximumHealthBar", row.maximumHealthBar) ||
			!ReadU32(value, "triggerHealthBar", row.triggerHealthBar) ||
			!ReadU32(value, "triggerOrder", row.triggerOrder) ||
			!ReadString(value, "armorRequirement", row.armorRequirement) ||
			(row.armorRequirement != "ANY" &&
				row.armorRequirement != "ARMORED" &&
				row.armorRequirement != "STRIPPED") ||
			!ReadString(value, "phaseRequirement", row.phaseRequirement) ||
			(row.phaseRequirement != "ANY" &&
				row.phaseRequirement != "PHASE_ONE" &&
				row.phaseRequirement != "PHASE_TWO") ||
			nullptr == Field(value, "invulnerableWhileRunning",
				DATA_JSON_TYPE::BOOLEAN) ||
			!ReadU32(value, "selectionWeight", row.selectionWeight) ||
			!ReadU32(value, "maximumConsecutiveUses", row.maximumConsecutiveUses) ||
			!ReadDouble(value, "minimumRange", row.minimumRange) ||
			!ReadDouble(value, "maximumRange", row.maximumRange))
			return false;
		if (hasServerMotion)
		{
			if (!serverMotion->Is_Object())
				return false;
			const DATA_JSON_VALUE* landingPosition =
				Field(*serverMotion, "landingPosition", DATA_JSON_TYPE::ARRAY);
			if (!IsExactObject(*serverMotion,
				{ "kind", "anchorId", "landingPosition", "apexHeight",
				  "travelStageId", "takeoffStartMs", "takeoffEndMs",
				  "travelStartMs", "travelEndMs" }) ||
				!ReadString(*serverMotion, "kind", row.serverMotion.kind) ||
				!ReadString(*serverMotion, "anchorId", row.serverMotion.anchorId) ||
				!ReadString(*serverMotion, "travelStageId",
					row.serverMotion.travelStageId) ||
				!ReadU32(*serverMotion, "takeoffStartMs",
					row.serverMotion.takeoffStartMs) ||
				!ReadU32(*serverMotion, "takeoffEndMs",
					row.serverMotion.takeoffEndMs) ||
				!ReadU32(*serverMotion, "travelStartMs",
					row.serverMotion.travelStartMs) ||
				!ReadU32(*serverMotion, "travelEndMs",
					row.serverMotion.travelEndMs) ||
				nullptr == landingPosition ||
				3u != landingPosition->Get_Array().size() ||
				!landingPosition->Get_Array()[0].Is_Number() ||
				!landingPosition->Get_Array()[1].Is_Number() ||
				!landingPosition->Get_Array()[2].Is_Number() ||
				!ReadDouble(*serverMotion, "apexHeight", row.serverMotion.apexHeight))
			{
				return false;
			}
			row.serverMotion.landingX =
				landingPosition->Get_Array()[0].Get_Number();
			row.serverMotion.landingY =
				landingPosition->Get_Array()[1].Get_Number();
			row.serverMotion.landingZ =
				landingPosition->Get_Array()[2].Get_Number();
			row.serverMotion.enabled =
				std::isfinite(row.serverMotion.landingX) &&
				std::isfinite(row.serverMotion.landingY) &&
				std::isfinite(row.serverMotion.landingZ);
			if (!row.serverMotion.enabled)
				return false;
		}
		for (const DATA_JSON_VALUE& sourceAction : sourceActions->Get_Array())
		{
			if (!sourceAction.Is_Number() ||
				!std::isfinite(sourceAction.Get_Number()) ||
				std::floor(sourceAction.Get_Number()) != sourceAction.Get_Number() ||
				sourceAction.Get_Number() <= 0.0 ||
				sourceAction.Get_Number() >
					static_cast<double>((std::numeric_limits<std::uint32_t>::max)()))
			{
				return false;
			}
			row.sourceActionIds.push_back(
				static_cast<std::uint32_t>(sourceAction.Get_Number()));
		}
		for (const DATA_JSON_VALUE& stageValue : stages->Get_Array())
		{
			PATTERN_STAGE_EDIT stage{};
			if (!IsExactObject(stageValue, { "stageId", "actionId", "stageKind",
				"durationMs", "hitShape", "hitOuterRadius", "hitInnerRadius",
				"hitAngleDegrees", "hitLength", "hitHalfWidth", "hitCount",
				"hitIntervalMs", "hitDelayMs", "serverDamageProfileId",
				"pushRangeM", "pushMs", "knockdown", "downMs" }) ||
				!ReadString(stageValue, "stageId", stage.stageId) ||
				!ReadString(stageValue, "actionId", stage.actionId) ||
				!ReadString(stageValue, "stageKind", stage.stageKind) ||
				!ReadU32(stageValue, "durationMs", stage.durationMs) ||
				!ReadString(stageValue, "hitShape", stage.hitShape) ||
				!ReadDouble(stageValue, "hitOuterRadius", stage.hitOuterRadius) ||
				!ReadDouble(stageValue, "hitInnerRadius", stage.hitInnerRadius) ||
				!ReadDouble(stageValue, "hitAngleDegrees", stage.hitAngleDegrees) ||
				!ReadDouble(stageValue, "hitLength", stage.hitLength) ||
				!ReadDouble(stageValue, "hitHalfWidth", stage.hitHalfWidth) ||
				!ReadU32(stageValue, "hitCount", stage.hitCount) ||
				!ReadU32(stageValue, "hitIntervalMs", stage.hitIntervalMs) ||
				!ReadU32(stageValue, "hitDelayMs", stage.hitDelayMs) ||
				!ReadString(stageValue, "serverDamageProfileId", stage.damageProfileId) ||
				!ReadDouble(stageValue, "pushRangeM", stage.pushRangeM) ||
				!ReadU32(stageValue, "pushMs", stage.pushMs) ||
				!ReadU32(stageValue, "downMs", stage.downMs))
			{
				return false;
			}
			const DATA_JSON_VALUE* knockdownValue = stageValue.Find("knockdown");
			if (nullptr == knockdownValue || !knockdownValue->Is_Boolean())
				return false;
			stage.knockdown = knockdownValue->Get_Boolean();
			row.stages.push_back(std::move(stage));
		}
		row.invulnerableWhileRunning =
			Field(value, "invulnerableWhileRunning",
				DATA_JSON_TYPE::BOOLEAN)->Get_Boolean();
		patterns.push_back(std::move(row));
	}
	for (const DATA_JSON_VALUE& value : stateArray->Get_Array())
	{
		ENCOUNTER_STATE_EDIT row{};
		if (!IsExactObject(value, { "id", "actionId", "next" }) ||
			!ReadString(value, "id", row.id) || !ReadString(value, "actionId", row.actionId))
			return false;
		const DATA_JSON_VALUE* next = value.Find("next");
		if (nullptr == next || (!next->Is_Null() && !next->Is_String()))
			return false;
		row.hasNext = next->Is_String();
		if (row.hasNext)
			row.next = next->Get_String();
		states.push_back(std::move(row));
	}
	std::string encounterId;
	std::string bossArchetypeId;
	std::string authority;
	std::string introPatternId;
	std::uint32_t fixedTickHz = 0;
	if (!ReadString(encounterRoot, "encounterId", encounterId) ||
		!ReadString(encounterRoot, "bossArchetypeId", bossArchetypeId) ||
		!ReadString(encounterRoot, "authority", authority) ||
		!ReadString(encounterRoot, "introPatternId", introPatternId) ||
		!ReadU32(encounterRoot, "fixedTickHz", fixedTickHz))
		return false;
	for (const DATA_JSON_VALUE& value : receiptArray->Get_Array())
	{
		std::string document;
		std::string targetId;
		std::string field;
		std::string basis;
		if (!ReadString(value, "targetDocument", document) ||
			!ReadString(value, "targetId", targetId) ||
			!ReadString(value, "targetField", field) ||
			!ReadString(value, "basis", basis))
			return false;
		bases.emplace(document + "#" + targetId + "." + field, basis);
	}
	if (!ReloadValtanPatternAuthoring(encounterRoot, valtanPatternTree,
		legacyPatterns, valtanPatternStatus, status))
	{
		m_status = "Reload failed: " + status;
		return false;
	}
	if (const VALTAN_PATTERN_VIEW* highJump =
			FindValtanPattern(valtanPatternTree, valtanAxeVolley.patternId))
	{
		if (const VALTAN_STAGE_VIEW* airborne =
				FindValtanStage(*highJump, valtanAxeVolley.stageId))
		{
			const auto object = std::find_if(
				airborne->CombatObjectEffects.begin(),
				airborne->CombatObjectEffects.end(),
				[](const VALTAN_COMBAT_OBJECT_EFFECT_VIEW& candidate)
				{
					return candidate.strCombatObjectArchetypeId ==
						"combatobject.valtan.high-jump.target-axe";
				});
			if (airborne->CombatObjectEffects.end() != object)
				valtanAxeVolley.countPerResolvedTarget = object->iSpawnValue;
		}
	}

	std::string valtanSourceRevision;
	std::string valtanAuthoringRevision;
	VALTAN_SOURCE_JOIN_STATUS valtanSourceJoin;
	if (!QueryValtanSourceRevision(valtanSourceRevision,
		valtanAuthoringRevision, valtanSourceJoin, status))
	{
		m_status = "Reload failed: Valtan source revision admission failed: " +
			status;
		return false;
	}
	if (!valtanAuthoringRevision.empty() &&
		!RestoreValtanSavedAuthoring(damageProfiles, bosses,
			valtanPatternTree, valtanAxeVolley, valtanSourceRevision,
			valtanAuthoringRevision, status))
	{
		m_status = "Reload failed: saved Valtan authoring revision was rejected: " +
			status;
		return false;
	}

	m_players = std::move(players);
	m_skills = std::move(skills);
	m_damageProfiles = std::move(damageProfiles);
	m_bosses = std::move(bosses);
	m_patterns = std::move(patterns);
	m_encounterStates = std::move(states);
	m_encounterId = std::move(encounterId);
	m_encounterBossArchetypeId = std::move(bossArchetypeId);
	m_encounterAuthority = std::move(authority);
	m_encounterIntroPatternId = std::move(introPatternId);
	m_fixedTickHz = fixedTickHz;
	m_basisByField = std::move(bases);
	m_valtanPatternTree = std::move(valtanPatternTree);
	m_legacyPatterns = std::move(legacyPatterns);
	m_valtanPatternStatus = std::move(valtanPatternStatus);
	m_valtanAxeVolley = std::move(valtanAxeVolley);
	m_loadedDamageProfiles = m_damageProfiles;
	m_loadedBosses = m_bosses;
	m_loadedValtanPatternTree = m_valtanPatternTree;
	m_loadedValtanAxeVolley = m_valtanAxeVolley;
	m_selectedPlayer = (std::min)(m_selectedPlayer,
		m_players.empty() ? 0u : m_players.size() - 1u);
	m_selectedBoss = (std::min)(m_selectedBoss,
		m_bosses.empty() ? 0u : m_bosses.size() - 1u);
	m_dirty = false;
	m_valtanDraftValidated = !valtanAuthoringRevision.empty() &&
		VALTAN_SOURCE_JOIN_STATE::JOINED_VALIDATED == valtanSourceJoin.state;
	m_valtanSourceRevision = std::move(valtanSourceRevision);
	m_valtanAuthoringRevision = std::move(valtanAuthoringRevision);
	m_valtanSourceJoin = std::move(valtanSourceJoin);
	m_valtanCandidateRevision.clear();
	m_valtanCandidateApplyClass.clear();
	if (VALTAN_SOURCE_JOIN_STATE::JOINED_VALIDATED !=
		m_valtanSourceJoin.state)
	{
		m_status =
			"Loaded Valtan transition data, but the Server Gameplay / Pattern "
			"Presentation strict join is not validated. Save, Publish, and Hot "
			"Reload remain blocked. " + m_valtanSourceJoin.diagnostic;
	}
	else if (m_valtanAuthoringRevision.empty())
	{
		m_status = "Loaded repository Valtan joined source closure at " +
			m_valtanSourceRevision.substr(0u, 12u) +
			". Generated Encounter v4 is read-only.";
	}
	else
	{
		m_status = "Resumed immutable Valtan authoring revision " +
			m_valtanAuthoringRevision.substr(0u, 12u) +
			" after strict pointer, manifest, hash, source, and empty-draft admission.";
	}
	return true;
}

bool Client::CBalanceTool::ReloadValtanPatternAuthoring(
	const DATA_JSON_VALUE& encounterRoot,
	VALTAN_PATTERN_TREE_VIEW& patternTree,
	std::vector<LEGACY_PATTERN_SUMMARY>& legacyPatterns,
	std::string& patternStatus,
	std::string& status)
{
	VALTAN_PATTERN_TREE_VIEW stagedTree;
	std::string treeStatus;
	if (!CValtanPatternTree::Load(stagedTree, treeStatus))
	{
		status = "Valtan split-source pattern-tree admission failed: " + treeStatus;
		return false;
	}

	std::unordered_set<std::string> managedPatternIds;
	managedPatternIds.reserve(stagedTree.Get_PatternCount());
	for (const auto* group : { &stagedTree.Gimmicks, &stagedTree.Rotation })
	{
		for (const VALTAN_PATTERN_VIEW& pattern : *group)
		{
			if (pattern.bAuthoringMasterManaged)
				managedPatternIds.insert(pattern.strPatternId);
		}
	}

	std::vector<LEGACY_PATTERN_SUMMARY> stagedLegacy;
	const DATA_JSON_VALUE* productPatterns =
		Field(encounterRoot, "patterns", DATA_JSON_TYPE::ARRAY);
	if (nullptr == productPatterns)
	{
		status = "Generated Valtan Encounter has no Product pattern array.";
		return false;
	}
	for (const DATA_JSON_VALUE& value : productPatterns->Get_Array())
	{
		LEGACY_PATTERN_SUMMARY row{};
		if (!ReadString(value, "patternId", row.patternId))
		{
			status = "Generated Valtan Encounter contains a pattern without a stable ID.";
			return false;
		}
		if (managedPatternIds.contains(row.patternId))
			continue;
		if (!ReadString(value, "displayName", row.displayName) ||
			!ReadString(value, "actionId", row.actionId) ||
			!ReadString(value, "selectionMode", row.selectionMode) ||
			!ReadString(value, "phaseRequirement", row.phaseRequirement) ||
			!ReadU32(value, "selectionWeight", row.selectionWeight))
		{
			status = "Generated legacy pattern summary is malformed: " + row.patternId;
			return false;
		}
		const DATA_JSON_VALUE* stages = Field(value, "stages", DATA_JSON_TYPE::ARRAY);
		if (nullptr == stages || stages->Get_Array().empty() ||
			stages->Get_Array().size() >
				static_cast<std::size_t>((std::numeric_limits<std::uint32_t>::max)()))
		{
			status = "Generated legacy pattern has no valid stages: " + row.patternId;
			return false;
		}
		row.stageCount = static_cast<std::uint32_t>(stages->Get_Array().size());
		stagedLegacy.push_back(std::move(row));
	}

	patternTree = std::move(stagedTree);
	legacyPatterns = std::move(stagedLegacy);
	patternStatus = treeStatus + " | managed=" +
		std::to_string(CountManagedValtanPatterns(patternTree)) +
		", legacy read-only=" + std::to_string(legacyPatterns.size()) + ".";
	return true;
}

bool Client::CBalanceTool::RestoreValtanSavedAuthoring(
	std::vector<DAMAGE_EDIT>& damageProfiles,
	std::vector<BOSS_EDIT>& bosses,
	VALTAN_PATTERN_TREE_VIEW& patternTree,
	VALTAN_AXE_VOLLEY_EDIT& axeVolley,
	const std::string& sourceRevision,
	const std::string& authoringRevision,
	std::string& status) const
{
	status.clear();
	if (!IsLowerSha256(sourceRevision) ||
		!IsLowerSha256(authoringRevision) ||
		sourceRevision != authoringRevision)
	{
		status = "Saved Valtan authoring source identity is not one exact lowercase SHA-256.";
		return false;
	}

	std::error_code pathError;
	const std::filesystem::path repositoryRoot =
		std::filesystem::weakly_canonical(
			CProjectDataRoot::Get().parent_path(), pathError);
	if (pathError || repositoryRoot.empty())
	{
		status = "Could not resolve the fixed repository root for saved Valtan authoring.";
		return false;
	}
	const std::filesystem::path authoringRelative =
		L"Intermediate/ValtanTuningAuthoring";
	std::filesystem::path authoringRoot;
	std::filesystem::path pointerPath;
	if (!ResolveFixedValtanAuthoringPath(repositoryRoot, authoringRelative,
			authoringRoot, status) ||
		!ResolveFixedValtanAuthoringPath(repositoryRoot,
			authoringRelative / L"current-authoring.json", pointerPath, status) ||
		!std::filesystem::is_directory(authoringRoot) ||
		!std::filesystem::is_regular_file(pointerPath))
	{
		if (status.empty())
			status = "Saved Valtan authoring pointer is missing or is not a regular file.";
		return false;
	}

	DATA_JSON_VALUE pointer;
	if (!ReadAbsoluteJsonObject(pointerPath, pointer, status) ||
		!IsExactObject(pointer, { "schema", "formatVersion", "revisionId",
			"manifest", "activeRuntimeChanged" }))
	{
		if (status.empty())
			status = "Saved Valtan authoring pointer fields are not exact.";
		return false;
	}
	std::string pointerSchema;
	std::string pointerRevision;
	std::string pointerManifest;
	std::uint32_t pointerVersion = 0u;
	bool activeRuntimeChanged = true;
	if (!ReadString(pointer, "schema", pointerSchema) ||
		pointerSchema != "lostark.valtan-tuning-authoring-pointer" ||
		!ReadU32(pointer, "formatVersion", pointerVersion) ||
		1u != pointerVersion ||
		!ReadString(pointer, "revisionId", pointerRevision) ||
		pointerRevision != authoringRevision ||
		!ReadString(pointer, "manifest", pointerManifest) ||
		pointerManifest != "revisions/" + authoringRevision +
			"/authoring-manifest.json" ||
		!ReadBoolean(pointer, "activeRuntimeChanged", activeRuntimeChanged) ||
		activeRuntimeChanged)
	{
		status = "Saved Valtan authoring pointer identity does not match the admitted revision.";
		return false;
	}
	const std::string admittedPointerBytes = ReadTextFile(pointerPath);
	if (admittedPointerBytes.empty())
	{
		status = "Saved Valtan authoring pointer could not be captured for commit guarding.";
		return false;
	}

	const std::string emptyPatch =
		"{\n  \"schema\": \"lostark.valtan-tuning-draft-patch\",\n"
		"  \"formatVersion\": 1,\n  \"sourceRevision\": " +
		Quote(authoringRevision) + ",\n  \"operations\": []\n}\n";
	const std::filesystem::path temporaryDirectory =
		std::filesystem::temp_directory_path(pathError);
	if (pathError)
	{
		status = "Could not resolve a temporary saved-authoring validation directory: " +
			pathError.message() + ".";
		return false;
	}
	const std::filesystem::path patchPath = temporaryDirectory /
		(L"LostArk.ValtanResume." + std::to_wstring(GetCurrentProcessId()) + L"." +
			std::to_wstring(GetTickCount64()) + L".json");
	if (!DurableWrite(patchPath, emptyPatch, status))
		return false;
	const std::wstring arguments = L"-Mode ValidateDraft -DraftPatchPath \"" +
		patchPath.wstring() + L"\"";
	std::string captured;
	std::string processStatus;
	const bool processSucceeded = RunPipeline(
		L"ValtanPipeline\\Publish-ValtanTuningRuntimeSet.ps1",
		arguments.c_str(), processStatus, &captured);
	std::filesystem::remove(patchPath, pathError);
	VALTAN_PIPELINE_RESULT validationResult;
	std::string parseStatus;
	if (!ParseValtanPipelineResult(captured, validationResult, parseStatus) ||
		!processSucceeded || !validationResult.ok ||
		validationResult.command != "VALIDATE_DRAFT" ||
		!validationResult.hasAuthoringRevisionField ||
		validationResult.sourceRevision != authoringRevision ||
		validationResult.authoringRevision != authoringRevision ||
		!validationResult.candidateRevision.empty())
	{
		status = !validationResult.diagnostic.empty() ?
			validationResult.diagnostic :
			(!parseStatus.empty() ? parseStatus :
				"Saved Valtan authoring empty-draft validation failed: " +
				processStatus);
		return false;
	}

	const std::filesystem::path revisionRelative = authoringRelative /
		L"revisions" / std::filesystem::path(authoringRevision);
	std::filesystem::path revisionRoot;
	std::filesystem::path gameplayPath;
	std::filesystem::path presentationPath;
	std::filesystem::path bossPath;
	std::filesystem::path damagePath;
	if (!ResolveFixedValtanAuthoringPath(repositoryRoot, revisionRelative,
			revisionRoot, status) ||
		!ResolveFixedValtanAuthoringPath(repositoryRoot,
			revisionRelative / L"Data/Valtan/Valtan.gameplay.json",
			gameplayPath, status) ||
		!ResolveFixedValtanAuthoringPath(repositoryRoot,
			revisionRelative / L"Data/Valtan/Valtan.presentation.json",
			presentationPath, status) ||
		!ResolveFixedValtanAuthoringPath(repositoryRoot,
			revisionRelative / L"Data/Balance/BossProfiles.json",
			bossPath, status) ||
		!ResolveFixedValtanAuthoringPath(repositoryRoot,
			revisionRelative / L"Data/Balance/DamageProfiles.json",
			damagePath, status) ||
		!std::filesystem::is_directory(revisionRoot) ||
		!std::filesystem::is_regular_file(gameplayPath) ||
		!std::filesystem::is_regular_file(presentationPath) ||
		!std::filesystem::is_regular_file(bossPath) ||
		!std::filesystem::is_regular_file(damagePath))
	{
		if (status.empty())
			status = "Saved Valtan authoring revision artifacts are missing or not regular files.";
		return false;
	}

	VALTAN_PATTERN_TREE_VIEW savedPatternTree;
	std::string splitTreeStatus;
	if (!CValtanPatternTree::Load_FromAuthoringPaths(
			gameplayPath, presentationPath, savedPatternTree, splitTreeStatus,
			VALTAN_PATTERN_TREE_LOAD_POLICY::RESTORE_AUTHORING_SNAPSHOT))
	{
		status = "Saved Valtan split gameplay/presentation join failed: " +
			splitTreeStatus;
		return false;
	}
	/* Balance Tool edits only gameplay fields, but the immutable presentation
	   artifact is part of admission.  Stage the fully joined saved tree so its
	   read-only presentation references survive restart exactly as saved. */
	patternTree = std::move(savedPatternTree);

	DATA_JSON_VALUE gameplayRoot;
	DATA_JSON_VALUE savedBossRoot;
	DATA_JSON_VALUE savedDamageRoot;
	if (!ReadAbsoluteJsonObject(gameplayPath, gameplayRoot, status) ||
		!ReadAbsoluteJsonObject(bossPath, savedBossRoot, status) ||
		!ReadAbsoluteJsonObject(damagePath, savedDamageRoot, status))
	{
		return false;
	}

	const auto readBoundedU32 = [&status](const DATA_JSON_VALUE& object,
		const char* field, const std::uint32_t minimum,
		const std::uint32_t maximum, std::uint32_t& output)
	{
		if (!ReadU32(object, field, output) || output < minimum || output > maximum)
		{
			status = std::string("Saved Valtan authoring integer is out of bounds: ") +
				field + ".";
			return false;
		}
		return true;
	};
	const auto readBoundedDouble = [&status](const DATA_JSON_VALUE& object,
		const char* field, const double minimum, const double maximum,
		double& output)
	{
		if (!ReadDouble(object, field, output) || output < minimum || output > maximum)
		{
			status = std::string("Saved Valtan authoring number is out of bounds: ") +
				field + ".";
			return false;
		}
		return true;
	};

	if (!IsExactObject(savedBossRoot, { "schema", "formatVersion", "bosses" }) ||
		!HasSchemaVersion(savedBossRoot, "lostark.boss-profiles", 4u))
	{
		status = "Saved Valtan BossProfiles root is not the admitted v4 contract.";
		return false;
	}
	const DATA_JSON_VALUE* savedBosses =
		Field(savedBossRoot, "bosses", DATA_JSON_TYPE::ARRAY);
	if (nullptr == savedBosses || savedBosses->Get_Array().size() != bosses.size())
	{
		status = "Saved Valtan BossProfiles stable-ID set does not match the loaded baseline.";
		return false;
	}
	std::unordered_set<std::string> savedBossIds;
	for (const DATA_JSON_VALUE& value : savedBosses->Get_Array())
	{
		std::string bossId;
		if (!ReadString(value, "archetypeId", bossId) ||
			!savedBossIds.insert(bossId).second)
		{
			status = "Saved Valtan BossProfiles contains a missing or duplicate stable ID.";
			return false;
		}
		const auto target = std::find_if(bosses.begin(), bosses.end(),
			[&](const BOSS_EDIT& row) { return row.archetypeId == bossId; });
		if (bosses.end() == target)
		{
			status = "Saved Valtan BossProfiles contains an unknown stable ID: " + bossId;
			return false;
		}
		if (bossId != "BOSS_VALTAN")
			continue;
		std::string encounterId;
		std::string phaseKind;
		const DATA_JSON_VALUE* phasePolicy =
			Field(value, "phasePolicy", DATA_JSON_TYPE::OBJECT);
		if (!ReadString(value, "encounterId", encounterId) ||
			encounterId != "ENCOUNTER_VALTAN" || nullptr == phasePolicy ||
			!IsExactObject(*phasePolicy, { "kind" }) ||
			!ReadString(*phasePolicy, "kind", phaseKind) ||
			phaseKind != "AUTHORED_PATTERN_EVENT" ||
			!readBoundedU32(value, "maximumHp", 1u, 0x7fffffffu,
				target->maximumHp) ||
			!readBoundedU32(value, "maximumHealthBars", 1u, 1000u,
				target->maximumHealthBars) ||
			!readBoundedU32(value, "attackPower", 0u, 0x7fffffffu,
				target->attackPower) ||
			!readBoundedDouble(value, "collisionRadius", 0.01, 1000.0,
				target->collisionRadius) ||
			!readBoundedDouble(value, "engageDistance", 0.0, 10000.0,
				target->engageDistance) ||
			!readBoundedDouble(value, "moveSpeed", 0.0, 1000.0,
				target->moveSpeed))
		{
			if (status.empty())
				status = "Saved BOSS_VALTAN base fields are malformed.";
			return false;
		}
	}

	if (!IsExactObject(savedDamageRoot, { "schema", "formatVersion", "profiles" }) ||
		!HasSchemaVersion(savedDamageRoot, "lostark.damage-profiles", 2u))
	{
		status = "Saved Valtan DamageProfiles root is not the admitted v2 contract.";
		return false;
	}
	const DATA_JSON_VALUE* savedDamageProfiles =
		Field(savedDamageRoot, "profiles", DATA_JSON_TYPE::ARRAY);
	if (nullptr == savedDamageProfiles ||
		savedDamageProfiles->Get_Array().size() != damageProfiles.size())
	{
		status = "Saved Valtan DamageProfiles stable-ID set does not match the loaded baseline.";
		return false;
	}
	std::unordered_set<std::string> savedDamageIds;
	for (const DATA_JSON_VALUE& value : savedDamageProfiles->Get_Array())
	{
		std::string damageId;
		std::uint32_t rate = 0u;
		if (!IsExactObject(value, { "damageProfileId", "damageRatePercent" }) ||
			!ReadString(value, "damageProfileId", damageId) ||
			!savedDamageIds.insert(damageId).second ||
			!readBoundedU32(value, "damageRatePercent", 0u, 0x7fffffffu, rate))
		{
			if (status.empty())
				status = "Saved Valtan DamageProfiles contains an invalid or duplicate row.";
			return false;
		}
		const auto target = std::find_if(damageProfiles.begin(), damageProfiles.end(),
			[&](const DAMAGE_EDIT& row) { return row.damageProfileId == damageId; });
		if (damageProfiles.end() == target)
		{
			status = "Saved Valtan DamageProfiles contains an unknown stable ID: " +
				damageId;
			return false;
		}
		target->damageRatePercent = rate;
	}

	if (!IsExactObject(gameplayRoot, { "schema", "formatVersion", "bossArchetypeId",
			"encounterId", "scope", "previewPaths", "retiredPatternIds",
			"decisionModel", "counterReactionLayers",
			"patterns" }) ||
		!HasSchemaVersion(gameplayRoot,
			"lostark.valtan-gameplay-authoring", 1u))
	{
		status =
			"Saved Valtan gameplay root is not the admitted split authoring contract.";
		return false;
	}
	std::string gameplayBossId;
	std::string gameplayEncounterId;
	if (!ReadString(gameplayRoot, "bossArchetypeId", gameplayBossId) ||
		gameplayBossId != "BOSS_VALTAN" ||
		!ReadString(gameplayRoot, "encounterId", gameplayEncounterId) ||
		gameplayEncounterId != "ENCOUNTER_VALTAN")
	{
		status = "Saved Valtan gameplay authoring identity is invalid.";
		return false;
	}
	const DATA_JSON_VALUE* decisionModel =
		Field(gameplayRoot, "decisionModel", DATA_JSON_TYPE::OBJECT);
	const DATA_JSON_VALUE* patternArray =
		Field(gameplayRoot, "patterns", DATA_JSON_TYPE::ARRAY);
	if (nullptr == decisionModel || nullptr == patternArray ||
		patternArray->Get_Array().size() !=
			CountManagedValtanPatterns(patternTree))
	{
		status = "Saved Valtan gameplay arrays do not match the strict joined tree.";
		return false;
	}

	/* CValtanPatternTree already parsed and validated the saved decisionModel
	   into typed sets/windows/mechanics before committing patternTree.  Do not
	   flatten it back through one per-pattern weight here: that would reject or
	   destroy intentionally different values in two health windows. */

	std::unordered_set<std::string> savedPatternIds;
	const DATA_JSON_VALUE* savedAxeEvent = nullptr;
	std::uint32_t savedAxeStageDurationMs = 0u;
	for (const DATA_JSON_VALUE& value : patternArray->Get_Array())
	{
		std::string patternId;
		const DATA_JSON_VALUE* eligibility =
			Field(value, "eligibility", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* stages = Field(value, "stages", DATA_JSON_TYPE::ARRAY);
		if (!ReadString(value, "patternId", patternId) ||
			!savedPatternIds.insert(patternId).second || nullptr == eligibility ||
			nullptr == stages)
		{
			status = "Saved Valtan pattern identity is missing or duplicate.";
			return false;
		}
		VALTAN_PATTERN_VIEW* targetPattern =
			FindValtanPattern(patternTree, patternId);
		if (nullptr == targetPattern ||
			!targetPattern->bAuthoringMasterManaged ||
			stages->Get_Array().size() != targetPattern->Stages.size())
		{
			status = "Saved Valtan pattern is unknown or changed its stable stage set: " +
				patternId;
			return false;
		}
		const DATA_JSON_VALUE* repeatPolicy =
			Field(*eligibility, "repeatPolicy", DATA_JSON_TYPE::OBJECT);
		std::string repeatKind;
		std::uint32_t repeatLimit = 0u;
		double minimumRange = 0.0;
		double maximumRange = 0.0;
		if (nullptr == repeatPolicy ||
			!IsExactObject(*repeatPolicy, { "kind", "limit" }) ||
			!ReadString(*repeatPolicy, "kind", repeatKind) ||
			repeatKind != "SOFT_AVOID_UNLESS_ONLY_ELIGIBLE" ||
			!readBoundedU32(*repeatPolicy, "limit", 0u, 64u, repeatLimit) ||
			!readBoundedDouble(*eligibility, "minimumRangeM", 0.0, 1000.0,
				minimumRange) ||
			!readBoundedDouble(*eligibility, "maximumRangeM", 0.0, 1000.0,
				maximumRange) || minimumRange > maximumRange)
		{
			if (status.empty())
				status = "Saved Valtan pattern repeat/range contract is invalid: " + patternId;
			return false;
		}
		targetPattern->iMaximumConsecutiveUses = repeatLimit;
		targetPattern->fMinimumRange = static_cast<float>(minimumRange);
		targetPattern->fMaximumRange = static_cast<float>(maximumRange);

		std::unordered_set<std::string> savedStageIds;
		for (const DATA_JSON_VALUE& stageValue : stages->Get_Array())
		{
			std::string stageId;
			std::uint32_t durationMs = 0u;
			const DATA_JSON_VALUE* hit =
				Field(stageValue, "hit", DATA_JSON_TYPE::OBJECT);
			if (!ReadString(stageValue, "stageId", stageId) ||
				!savedStageIds.insert(stageId).second || nullptr == hit ||
				!readBoundedU32(stageValue, "durationMs", 1u, 600000u,
					durationMs))
			{
				if (status.empty())
					status = "Saved Valtan stage identity/duration is invalid: " + patternId;
				return false;
			}
			VALTAN_STAGE_VIEW* targetStage =
				FindValtanStage(*targetPattern, stageId);
			if (nullptr == targetStage)
			{
				status = "Saved Valtan stage has an unknown stable ID: " + patternId +
					"/" + stageId;
				return false;
			}
			targetStage->iDurationMs = durationMs;
			const DATA_JSON_VALUE* shape =
				Field(*hit, "shape", DATA_JSON_TYPE::OBJECT);
			std::string shapeKind;
			if (nullptr == shape || !ReadString(*shape, "kind", shapeKind))
			{
				status = "Saved Valtan stage is missing its typed hit-shape identity: " +
					patternId + "/" + stageId;
				return false;
			}
			targetStage->strHitShape = shapeKind;

			targetStage->fHitOuterRadius = 0.f;
			targetStage->fHitInnerRadius = 0.f;
			targetStage->fHitAngleDegrees = 0.f;
			targetStage->fHitLength = 0.f;
			targetStage->fHitHalfWidth = 0.f;
			targetStage->iHitCount = 0u;
			targetStage->iHitIntervalMs = 0u;
			targetStage->iHitDelayMs = 0u;
			targetStage->HitOffsetsMs.clear();
			targetStage->strServerDamageProfileId.clear();
			targetStage->fPushRangeM = 0.f;
			targetStage->iPushMs = 0u;
			targetStage->bKnockdown = false;
			targetStage->iDownMs = 0u;

			const auto readShapeNumber = [&](const char* field,
				const double maximum, float& output)
			{
				double value = 0.0;
				if (!readBoundedDouble(*shape, field, 0.0, maximum, value))
					return false;
				output = static_cast<float>(value);
				return true;
			};
			if (shapeKind == "NONE")
			{
				if (!IsExactObject(*hit, { "shape" }) ||
					!IsExactObject(*shape, { "kind" }))
				{
					status = "Saved Valtan NONE hit contains hidden fields: " + patternId +
						"/" + stageId;
					return false;
				}
			}
			else
			{
				if (!IsExactObject(*hit, { "shape", "schedule",
						"serverDamageProfileId", "pushRangeM", "pushMs",
						"knockdown", "downMs" }))
				{
					status = "Saved Valtan hit fields are not exact: " + patternId +
						"/" + stageId;
					return false;
				}
				bool validShape = false;
				if (shapeKind == "CIRCLE")
				{
					validShape = IsExactObject(*shape, { "kind", "outerRadiusM" }) &&
						readShapeNumber("outerRadiusM", 1000.0,
							targetStage->fHitOuterRadius);
				}
				else if (shapeKind == "RING")
				{
					validShape = IsExactObject(*shape,
						{ "kind", "innerRadiusM", "outerRadiusM" }) &&
						readShapeNumber("innerRadiusM", 1000.0,
							targetStage->fHitInnerRadius) &&
						readShapeNumber("outerRadiusM", 1000.0,
							targetStage->fHitOuterRadius) &&
						targetStage->fHitInnerRadius < targetStage->fHitOuterRadius;
				}
				else if (shapeKind == "CONE")
				{
					validShape = IsExactObject(*shape,
						{ "kind", "angleDegrees", "lengthM" }) &&
						readShapeNumber("angleDegrees", 180.0,
							targetStage->fHitAngleDegrees) &&
						readShapeNumber("lengthM", 1000.0,
							targetStage->fHitLength);
				}
				else if (shapeKind == "BOX" || shapeKind == "CROSS" ||
					shapeKind == "SIX_DIRECTIONS")
				{
					validShape = IsExactObject(*shape,
						{ "kind", "lengthM", "halfWidthM" }) &&
						readShapeNumber("lengthM", 1000.0,
							targetStage->fHitLength) &&
						readShapeNumber("halfWidthM", 1000.0,
							targetStage->fHitHalfWidth);
				}
				if (!validShape)
				{
					if (status.empty())
						status = "Saved Valtan hit shape is invalid: " + patternId +
							"/" + stageId;
					return false;
				}

				const DATA_JSON_VALUE* schedule =
					Field(*hit, "schedule", DATA_JSON_TYPE::OBJECT);
				std::string scheduleKind;
				if (nullptr == schedule ||
					!ReadString(*schedule, "kind", scheduleKind))
				{
					status = "Saved Valtan hit schedule is missing: " + patternId +
						"/" + stageId;
					return false;
				}
				if (scheduleKind == "EXPLICIT_OFFSETS")
				{
					const DATA_JSON_VALUE* offsets =
						Field(*schedule, "offsetsMs", DATA_JSON_TYPE::ARRAY);
					if (!IsExactObject(*schedule, { "kind", "offsetsMs" }) ||
						nullptr == offsets || offsets->Get_Array().empty() ||
						offsets->Get_Array().size() > 64u)
					{
						status = "Saved Valtan explicit hit schedule is invalid: " + patternId +
							"/" + stageId;
						return false;
					}
					std::uint32_t previous = 0u;
					bool hasPrevious = false;
					for (const DATA_JSON_VALUE& offsetValue : offsets->Get_Array())
					{
						if (!offsetValue.Is_Number() ||
							!std::isfinite(offsetValue.Get_Number()) ||
							std::floor(offsetValue.Get_Number()) != offsetValue.Get_Number() ||
							offsetValue.Get_Number() < 0.0 ||
							offsetValue.Get_Number() >= durationMs)
						{
							status = "Saved Valtan explicit hit offset is out of bounds: " +
								patternId + "/" + stageId;
							return false;
						}
						const std::uint32_t offset =
							static_cast<std::uint32_t>(offsetValue.Get_Number());
						if (hasPrevious && offset <= previous)
						{
							status = "Saved Valtan explicit hit offsets are not strictly ordered: " +
								patternId + "/" + stageId;
							return false;
						}
						targetStage->HitOffsetsMs.push_back(offset);
						previous = offset;
						hasPrevious = true;
					}
					targetStage->iHitCount = static_cast<std::uint32_t>(
						targetStage->HitOffsetsMs.size());
				}
				else if (scheduleKind == "INTERVAL")
				{
					std::uint32_t count = 0u;
					std::uint32_t firstOffset = 0u;
					std::uint32_t interval = 0u;
					if (!IsExactObject(*schedule,
							{ "kind", "count", "firstOffsetMs", "intervalMs" }) ||
						!readBoundedU32(*schedule, "count", 1u, 64u, count) ||
						!readBoundedU32(*schedule, "firstOffsetMs", 0u,
							durationMs - 1u, firstOffset) ||
						!readBoundedU32(*schedule, "intervalMs", 0u,
							durationMs, interval) ||
						(count > 1u && 0u == interval) ||
						static_cast<std::uint64_t>(firstOffset) +
							static_cast<std::uint64_t>(count - 1u) * interval >= durationMs)
					{
						if (status.empty())
							status = "Saved Valtan interval hit schedule is invalid: " + patternId +
								"/" + stageId;
						return false;
					}
					targetStage->iHitCount = count;
					targetStage->iHitDelayMs = firstOffset;
					targetStage->iHitIntervalMs = interval;
				}
				else
				{
					status = "Saved Valtan hit schedule kind is unsupported: " + patternId +
						"/" + stageId;
					return false;
				}

				std::string damageId;
				double pushRange = 0.0;
				std::uint32_t pushMs = 0u;
				std::uint32_t downMs = 0u;
				bool knockdown = false;
				if (!ReadString(*hit, "serverDamageProfileId", damageId) ||
					0u != damageId.rfind("damage.valtan.", 0u) ||
					!savedDamageIds.contains(damageId) ||
					!readBoundedDouble(*hit, "pushRangeM", -20.0, 20.0,
						pushRange) ||
					!readBoundedU32(*hit, "pushMs", 0u, 600000u, pushMs) ||
					!ReadBoolean(*hit, "knockdown", knockdown) ||
					!readBoundedU32(*hit, "downMs", 0u, 600000u, downMs) ||
					((0.0 == pushRange) != (0u == pushMs)) ||
					(knockdown ? 0u == downMs : 0u != downMs))
				{
					if (status.empty())
						status = "Saved Valtan hit damage/push/down contract is invalid: " +
							patternId + "/" + stageId;
					return false;
				}
				targetStage->strServerDamageProfileId = std::move(damageId);
				targetStage->fPushRangeM = static_cast<float>(pushRange);
				targetStage->iPushMs = pushMs;
				targetStage->bKnockdown = knockdown;
				targetStage->iDownMs = downMs;
			}

			if (patternId == axeVolley.patternId && stageId == axeVolley.stageId)
			{
				savedAxeStageDurationMs = durationMs;
				const DATA_JSON_VALUE* events =
					Field(stageValue, "events", DATA_JSON_TYPE::ARRAY);
				if (nullptr == events)
				{
					status = "Saved Valtan axe stage has no typed events array.";
					return false;
				}
				for (const DATA_JSON_VALUE& event : events->Get_Array())
				{
					std::string eventId;
					if (ReadString(event, "eventId", eventId) &&
						eventId == axeVolley.eventId)
					{
						if (nullptr != savedAxeEvent)
						{
							status = "Saved Valtan axe-volley event ID is duplicated.";
							return false;
						}
						savedAxeEvent = &event;
					}
				}
			}
		}
	}
	if (savedPatternIds.size() != CountManagedValtanPatterns(patternTree))
	{
		status = "Saved Valtan pattern stable-ID set is incomplete.";
		return false;
	}
	if (nullptr == savedAxeEvent ||
		!IsExactObject(*savedAxeEvent, { "eventId", "trigger", "kind",
			"combatObjectArchetypeId", "volleyPolicy", "countPerResolvedTarget",
			"layout", "allowOverlap", "maximumTotalObjects", "spawnSchedule",
			"arenaRandom" }))
	{
		status = "Saved Valtan axe-volley event is missing or has hidden fields.";
		return false;
	}
	std::string axeEventId;
	std::string axeTrigger;
	std::string axeKind;
	std::string axeArchetype;
	std::string axePolicy;
	std::uint32_t axeCount = 0u;
	std::uint32_t axeMaximum = 0u;
	bool axeOverlap = true;
	const DATA_JSON_VALUE* layout =
		Field(*savedAxeEvent, "layout", DATA_JSON_TYPE::OBJECT);
	const DATA_JSON_VALUE* spawnSchedule =
		Field(*savedAxeEvent, "spawnSchedule", DATA_JSON_TYPE::OBJECT);
	const DATA_JSON_VALUE* arenaRandom =
		Field(*savedAxeEvent, "arenaRandom", DATA_JSON_TYPE::OBJECT);
	if (!ReadString(*savedAxeEvent, "eventId", axeEventId) ||
		axeEventId != axeVolley.eventId ||
		!ReadString(*savedAxeEvent, "trigger", axeTrigger) || axeTrigger != "ENTER" ||
		!ReadString(*savedAxeEvent, "kind", axeKind) ||
		axeKind != "SPAWN_COMBAT_OBJECT_VOLLEY" ||
		!ReadString(*savedAxeEvent, "combatObjectArchetypeId", axeArchetype) ||
		axeArchetype != "combatobject.valtan.high-jump.target-axe" ||
		!ReadString(*savedAxeEvent, "volleyPolicy", axePolicy) ||
		axePolicy != "PER_ALIVE_PLAYER" || nullptr == layout ||
		nullptr == spawnSchedule || nullptr == arenaRandom ||
		!readBoundedU32(*savedAxeEvent, "countPerResolvedTarget", 1u, 8u,
			axeCount) ||
		!ReadBoolean(*savedAxeEvent, "allowOverlap", axeOverlap) || axeOverlap ||
		!readBoundedU32(*savedAxeEvent, "maximumTotalObjects", 1u, 64u,
			axeMaximum))
	{
		if (status.empty())
			status = "Saved Valtan axe-volley identity or bounds are invalid.";
		return false;
	}
	std::string layoutKind;
	if (!ReadString(*layout, "kind", layoutKind))
	{
		status = "Saved Valtan axe-volley layout kind is missing.";
		return false;
	}
	double radiusM = 0.0;
	double startAngleDegrees = 0.0;
	double angleStepDegrees = 0.0;
	if (layoutKind == "TARGET_CENTER")
	{
		if (!IsExactObject(*layout, { "kind" }) || 1u != axeCount)
		{
			status = "Saved Valtan TARGET_CENTER volley must contain exactly one axe.";
			return false;
		}
	}
	else if (layoutKind == "RADIAL_AROUND_TARGET")
	{
		if (!IsExactObject(*layout,
				{ "kind", "radiusM", "startAngleDegrees", "angleStepDegrees" }) ||
			axeCount < 2u ||
			!readBoundedDouble(*layout, "radiusM", 0.01, 1000.0, radiusM) ||
			!readBoundedDouble(*layout, "startAngleDegrees", -360000.0,
				360000.0, startAngleDegrees) ||
			!readBoundedDouble(*layout, "angleStepDegrees", 0.000001,
				360.0, angleStepDegrees) ||
			angleStepDegrees * axeCount > 360.000001)
		{
			if (status.empty())
				status = "Saved Valtan radial axe-volley layout is invalid.";
			return false;
		}
	}
	else
	{
		status = "Saved Valtan axe-volley layout kind is unsupported.";
		return false;
	}
	std::string spawnScheduleKind;
	std::uint32_t spawnCount = 0u;
	std::uint32_t spawnFirstOffsetMs = 0u;
	std::uint32_t spawnIntervalMs = 0u;
	if (0u == savedAxeStageDurationMs ||
		!IsExactObject(*spawnSchedule,
			{ "kind", "count", "firstOffsetMs", "intervalMs" }) ||
		!ReadString(*spawnSchedule, "kind", spawnScheduleKind) ||
		spawnScheduleKind != "INTERVAL" ||
		!readBoundedU32(*spawnSchedule, "count", 1u, 8u, spawnCount) ||
		!readBoundedU32(*spawnSchedule, "firstOffsetMs", 0u,
			savedAxeStageDurationMs - 1u, spawnFirstOffsetMs) ||
		0u != spawnFirstOffsetMs ||
		!readBoundedU32(*spawnSchedule, "intervalMs", 0u,
			savedAxeStageDurationMs, spawnIntervalMs) ||
		(spawnCount > 1u && 0u == spawnIntervalMs) ||
		(1u == spawnCount && 0u != spawnIntervalMs) ||
		static_cast<std::uint64_t>(spawnCount - 1u) * spawnIntervalMs >=
			savedAxeStageDurationMs ||
		3u != spawnCount || 1333u != spawnIntervalMs)
	{
		if (status.empty())
			status = "Saved Valtan axe-volley spawn schedule is invalid.";
		return false;
	}
	std::string arenaRandomKind;
	std::string arenaAnchor;
	std::uint32_t arenaRandomCount = 0u;
	double arenaRandomRadiusM = 0.0;
	double arenaHeightToleranceM = 0.0;
	if (!IsExactObject(*arenaRandom,
			{ "kind", "anchor", "count", "radiusM", "heightToleranceM" }) ||
		!ReadString(*arenaRandom, "kind", arenaRandomKind) ||
		arenaRandomKind != "RANDOM_NAVIGABLE_CIRCLE" ||
		!ReadString(*arenaRandom, "anchor", arenaAnchor) ||
		arenaAnchor != "BOSS_SPAWN_POSITION" ||
		!readBoundedU32(*arenaRandom, "count", 1u, 32u,
			arenaRandomCount) ||
		!readBoundedDouble(*arenaRandom, "radiusM", 0.01, 1000.0,
			arenaRandomRadiusM) ||
		!readBoundedDouble(*arenaRandom, "heightToleranceM", 0.0, 1000.0,
			arenaHeightToleranceM) ||
		axeMaximum < axeCount + arenaRandomCount ||
		4u != arenaRandomCount || 14.0 != arenaRandomRadiusM ||
		1.0 != arenaHeightToleranceM)
	{
		if (status.empty())
			status = "Saved Valtan axe-volley arena random contract is invalid.";
		return false;
	}
	axeVolley.countPerResolvedTarget = axeCount;
	axeVolley.layoutKind = std::move(layoutKind);
	axeVolley.radiusM = radiusM;
	axeVolley.startAngleDegrees = startAngleDegrees;
	axeVolley.angleStepDegrees = angleStepDegrees;
	axeVolley.allowOverlap = false;
	axeVolley.maximumTotalObjects = axeMaximum;
	axeVolley.spawnScheduleKind = std::move(spawnScheduleKind);
	axeVolley.spawnCount = spawnCount;
	axeVolley.spawnFirstOffsetMs = spawnFirstOffsetMs;
	axeVolley.spawnIntervalMs = spawnIntervalMs;
	axeVolley.arenaRandomKind = std::move(arenaRandomKind);
	axeVolley.arenaAnchor = std::move(arenaAnchor);
	axeVolley.arenaRandomCount = arenaRandomCount;
	axeVolley.arenaRandomRadiusM = arenaRandomRadiusM;
	axeVolley.arenaHeightToleranceM = arenaHeightToleranceM;
	if (VALTAN_PATTERN_VIEW* highJump =
			FindValtanPattern(patternTree, axeVolley.patternId))
	{
		if (VALTAN_STAGE_VIEW* airborne =
				FindValtanStage(*highJump, axeVolley.stageId))
		{
			const auto object = std::find_if(
				airborne->CombatObjectEffects.begin(),
				airborne->CombatObjectEffects.end(),
				[&](const VALTAN_COMBAT_OBJECT_EFFECT_VIEW& candidate)
				{ return candidate.strCombatObjectArchetypeId == axeArchetype; });
			if (airborne->CombatObjectEffects.end() == object)
			{
				status = "Saved Valtan axe-volley visual join is missing from the loaded tree.";
				return false;
			}
			object->iSpawnValue = axeCount;
		}
	}

	if (ReadTextFile(pointerPath) != admittedPointerBytes)
	{
		status = "Saved Valtan authoring pointer changed while Reload was staging.";
		return false;
	}
	status = "Saved Valtan authoring revision restored into an isolated editable snapshot.";
	return true;
}

std::uint32_t* Client::CBalanceTool::FindDamageRate(
	const std::string& damageProfileId)
{
	const auto found = std::find_if(m_damageProfiles.begin(), m_damageProfiles.end(),
		[&](const DAMAGE_EDIT& row) { return row.damageProfileId == damageProfileId; });
	return m_damageProfiles.end() == found ? nullptr : &found->damageRatePercent;
}

const std::uint32_t* Client::CBalanceTool::FindDamageRate(
	const std::string& damageProfileId) const
{
	const auto found = std::find_if(m_damageProfiles.begin(), m_damageProfiles.end(),
		[&](const DAMAGE_EDIT& row) { return row.damageProfileId == damageProfileId; });
	return m_damageProfiles.end() == found ? nullptr : &found->damageRatePercent;
}

void Client::CBalanceTool::NormalizePatternStagePush(PATTERN_STAGE_EDIT& stage)
{
	if (stage.damageProfileId.empty())
	{
		stage.pushRangeM = 0.0;
		stage.pushMs = 0u;
		stage.knockdown = false;
		stage.downMs = 0u;
		return;
	}
	if (0.0 == stage.pushRangeM)
		stage.pushMs = 0u;
	else if (0u == stage.pushMs)
		stage.pushMs = 1u;
	if (!stage.knockdown)
		stage.downMs = 0u;
	else if (0u == stage.downMs)
		stage.downMs = 1u;
}

void Client::CBalanceTool::NormalizePatternStageForShape(
	PATTERN_STAGE_EDIT& stage)
{
	const bool none = "NONE" == stage.hitShape;
	const bool circle = "CIRCLE" == stage.hitShape;
	const bool ring = "RING" == stage.hitShape;
	const bool cone = "CONE" == stage.hitShape;
	const bool boxLike = "BOX" == stage.hitShape ||
		"CROSS" == stage.hitShape || "SIX_DIRECTIONS" == stage.hitShape;

	if (!circle && !ring)
		stage.hitOuterRadius = 0.0;
	if (!ring)
		stage.hitInnerRadius = 0.0;
	if (!cone)
		stage.hitAngleDegrees = 0.0;
	if (!cone && !boxLike)
		stage.hitLength = 0.0;
	if (!boxLike)
		stage.hitHalfWidth = 0.0;
	if (none)
	{
		stage.hitCount = 0u;
		stage.hitIntervalMs = 0u;
		stage.hitDelayMs = 0u;
		stage.damageProfileId.clear();
	}
	NormalizePatternStagePush(stage);
}

#if !defined(LOSTARK_BALANCE_TOOL_CONTRACT_TEST)
void Client::CBalanceTool::RenderBasis(const std::string& document,
	const std::string& targetId, const std::string& field) const
{
	const auto found = m_basisByField.find(document + "#" + targetId + "." + field);
	if (m_basisByField.end() != found)
	{
		ImGui::SameLine();
		ImGui::TextDisabled("[%s]", found->second.c_str());
	}
}

void Client::CBalanceTool::RenderPlayerEditor()
{
	if (m_players.empty())
		return;
	PLAYER_EDIT& player = m_players[m_selectedPlayer];
	const std::string profileTarget = "player:" + player.characterClass;
	ImGui::Text("%s", player.characterClass.c_str());
	ImGui::SameLine();
	ImGui::TextDisabled("Reference row: level 10; fixed single-level skills use level 1");
	ImGui::SeparatorText("Basic stats");
	MarkDirty(EditU32("Maximum HP", player.maximumHp, 1u, 100000000u));
	RenderBasis("Data/Balance/PlayerProfiles.json", profileTarget, "maximumHp");
	MarkDirty(EditU32("Attack power", player.attackPower, 1u, 1000000u));
	RenderBasis("Data/Balance/PlayerProfiles.json", profileTarget, "attackPower");
	MarkDirty(EditU32("Defense", player.defense, 1u, 1000000u));
	RenderBasis("Data/Balance/PlayerProfiles.json", profileTarget, "defense");
	MarkDirty(EditU32("Maximum resource", player.maximumResource, 1u, 1000000u));
	RenderBasis("Data/Balance/PlayerProfiles.json", profileTarget, "maximumResource");
	MarkDirty(EditU32("Resource / sec", player.resourceRegenPerSecond, 1u, player.maximumResource));
	RenderBasis("Data/Balance/PlayerProfiles.json", profileTarget, "resourceRegenPerSecond");
	ImGui::SeparatorText("Movement");
	MarkDirty(EditDouble("Move speed", player.moveSpeed, 0.01f, 0.01, 100.0, "%.2f"));
	RenderBasis("Data/Balance/PlayerProfiles.json", profileTarget, "moveSpeed");
	MarkDirty(EditDouble("Defense stance speed scale",
		player.defenseStanceMoveSpeedScale, 0.01f, 0.01, 1.0, "%.2f"));
	RenderBasis("Data/Balance/PlayerProfiles.json", profileTarget,
		"defenseStanceMoveSpeedScale");
	ImGui::SeparatorText("Identity");
	MarkDirty(EditU32("Maximum identity", player.maximumIdentity, 0u, 1000000u));
	RenderBasis("Data/Balance/PlayerProfiles.json", profileTarget,
		"maximumIdentity");
	MarkDirty(EditU32("Identity / sec", player.identityRegenPerSecond,
		0u, player.maximumIdentity));
	RenderBasis("Data/Balance/PlayerProfiles.json", profileTarget,
		"identityRegenPerSecond");
	MarkDirty(EditU32("Identity drain / sec", player.identityDrainPerSecond,
		0u, player.maximumIdentity));
	RenderBasis("Data/Balance/PlayerProfiles.json", profileTarget,
		"identityDrainPerSecond");
	MarkDirty(EditU32("Stance switch identity cost",
		player.identityStanceSwitchCost, 0u, player.maximumIdentity));
	RenderBasis("Data/Balance/PlayerProfiles.json", profileTarget,
		"identityStanceSwitchCost");
	MarkDirty(EditU32("Cyclic identity", player.identityCyclic, 0u, 1u));
	RenderBasis("Data/Balance/PlayerProfiles.json", profileTarget,
		"identityCyclic");
	ImGui::SeparatorText("Skills");
	for (SKILL_EDIT& skill : m_skills)
	{
		if (skill.characterClass != player.characterClass)
			continue;
		ImGui::PushID(static_cast<int>(skill.skillId));
		const std::string label = "[" + skill.inputSlot + "] " + skill.displayName +
			"##" + std::to_string(skill.skillId);
		if (ImGui::CollapsingHeader(label.c_str()))
		{
			const std::string target = "skill:" + std::to_string(skill.skillId);
			ImGui::TextDisabled("skillId %u | %s | %s", skill.skillId,
				skill.skillKind.c_str(), skill.actionId.c_str());
			MarkDirty(EditU32("Cooldown ms", skill.cooldownMs,
				skill.skillKind == "COMBO" ? 0u : 1u, 600000u));
			RenderBasis("Data/Balance/PlayerSkills.json", target, "cooldownMs");
			MarkDirty(EditU32("Resource cost", skill.resourceCost, 0u, player.maximumResource));
			RenderBasis("Data/Balance/PlayerSkills.json", target, "resourceCost");
			MarkDirty(EditU32("Identity cost", skill.identityCost,
				0u, player.maximumIdentity));
			RenderBasis("Data/Balance/PlayerSkills.json", target, "identityCost");
			std::uint32_t* damageRate = FindDamageRate(skill.damageProfileId);
			if (nullptr != damageRate)
			{
				MarkDirty(EditU32("Damage rate %", *damageRate, 1u, 100000u));
				RenderBasis("Data/Balance/DamageProfiles.json",
					"damage:" + skill.damageProfileId, "damageRatePercent");
				const std::uint64_t expected =
					static_cast<std::uint64_t>(player.attackPower) * *damageRate / 100ull;
				ImGui::TextDisabled("Expected raw hit: %llu",
					static_cast<unsigned long long>((std::max<std::uint64_t>)(1ull, expected)));
			}
			MarkDirty(EditU32("Action duration ms", skill.actionDurationMs, 1u, 600000u));
			MarkDirty(EditU32("Hit time ms", skill.hitTimeMs, 0u, skill.actionDurationMs));
			MarkDirty(EditDouble("Maximum range", skill.maximumRange,
				0.1f, 0.1, 1000.0));
			MarkDirty(EditDouble("Skill movement distance", skill.movementDistance,
				0.1f, 0.0, 1000.0));
			if (!skill.comboStages.empty() && ImGui::TreeNode("Staged action timing"))
			{
				for (std::size_t index = 0; index < skill.comboStages.size(); ++index)
				{
					COMBO_STAGE_EDIT& stage = skill.comboStages[index];
					ImGui::PushID(static_cast<int>(index));
					ImGui::SeparatorText(("Stage " + std::to_string(index + 1u)).c_str());
					MarkDirty(EditU32("Duration", stage.actionDurationMs, 1u, 600000u));
					MarkDirty(EditU32("Hit", stage.hitTimeMs, 0u, stage.actionDurationMs));
					MarkDirty(EditU32("Combo advance", stage.comboAdvanceMs,
						stage.hitTimeMs, stage.actionDurationMs));
					MarkDirty(EditU32("Input open", stage.inputOpenMs, 0u, stage.actionDurationMs));
					MarkDirty(EditU32("Input close", stage.inputCloseMs, 0u, stage.actionDurationMs));
					ImGui::PopID();
				}
				ImGui::TreePop();
			}
			ImGui::TextDisabled("Effect binding: %s", skill.effectId.empty() ?
				"not authored" : skill.effectId.c_str());
		}
		ImGui::PopID();
	}
}

void Client::CBalanceTool::RenderBossEditor()
{
	if (m_bosses.empty())
		return;
	BOSS_EDIT& boss = m_bosses[m_selectedBoss];
	const std::string target = "boss:" + boss.archetypeId;
	ImGui::Text("%s (%s)", boss.displayName.c_str(), boss.archetypeId.c_str());
	ImGui::SeparatorText("Base stats");
	MarkDirty(EditU32("Maximum HP", boss.maximumHp, 1u, 4000000000u));
	RenderBasis("Data/Balance/BossProfiles.json", target, "maximumHp");
	MarkDirty(EditU32("Maximum health bars", boss.maximumHealthBars, 1u, 1000u));
	RenderBasis("Data/Balance/BossProfiles.json", target, "maximumHealthBars");
	MarkDirty(EditU32("Attack power", boss.attackPower, 1u, 1000000u));
	RenderBasis("Data/Balance/BossProfiles.json", target, "attackPower");
	MarkDirty(EditDouble("Collision radius", boss.collisionRadius,
		0.1f, 0.1, 100.0));
	ImGui::SeparatorText("Detection and movement");
	MarkDirty(EditDouble("Engage distance", boss.engageDistance,
		0.1f, 0.1, 1000.0));
	RenderBasis("Data/Balance/BossProfiles.json", target, "engageDistance");
	MarkDirty(EditDouble("Move speed", boss.moveSpeed, 0.01f, 0.01, 100.0));
	RenderBasis("Data/Balance/BossProfiles.json", target, "moveSpeed");
	ImGui::SeparatorText("Phase");
	ImGui::TextWrapped(
		"Valtan phase 2 is committed by the authored 109 IMPACT/ENTER event. "
		"An HP-percent phase control is intentionally not exposed here.");
	if (boss.archetypeId == "BOSS_VALTAN")
	{
		RenderValtanPatternAuthoring();
		return;
	}
	ImGui::SeparatorText("Patterns");
	const HUD_BOSS_STATE& liveBoss = CCombatHUDViewModel::Get().Get_Boss();
	std::uint32_t liveHealthBar = 0u;
	if (liveBoss.isValid && liveBoss.iCurrentHp > 0u && liveBoss.iMaximumHp > 0u)
	{
		const std::uint64_t scaled = static_cast<std::uint64_t>(liveBoss.iCurrentHp) *
			boss.maximumHealthBars;
		liveHealthBar = static_cast<std::uint32_t>(
			(scaled + liveBoss.iMaximumHp - 1u) / liveBoss.iMaximumHp);
	}
	for (std::size_t index = 0; index < m_patterns.size(); ++index)
	{
		PATTERN_EDIT& pattern = m_patterns[index];
		ImGui::PushID(static_cast<int>(index));
		const std::string header = pattern.displayName + "##" + pattern.patternId;
		if (ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::TextDisabled("Pattern ID: %s", pattern.patternId.c_str());
			ImGui::TextDisabled("Semantic action: %s", pattern.actionId.c_str());
			std::ostringstream sourceActions;
			for (std::size_t sourceIndex = 0;
				sourceIndex < pattern.sourceActionIds.size(); ++sourceIndex)
			{
				if (sourceIndex > 0u)
					sourceActions << ", ";
				sourceActions << pattern.sourceActionIds[sourceIndex];
			}
			ImGui::TextWrapped("Original Actions: %s", sourceActions.str().c_str());
			const bool manualAudition =
				pattern.selectionMode == "AUDITION_ONLY";
			if (manualAudition)
			{
				ImGui::TextDisabled(
					"Selection: Manual Server audition (automatic rotation disabled)");
			}
			else
			{
				int selectionMode = pattern.selectionMode == "HEALTH_BAR" ? 1 : 0;
				if (ImGui::Combo("Selection mode", &selectionMode,
					"Normal pool\0Health bar trigger\0"))
				{
					pattern.selectionMode =
						0 == selectionMode ? "NORMAL" : "HEALTH_BAR";
					m_dirty = true;
				}
			}
			if (pattern.selectionMode == "NORMAL")
			{
				MarkDirty(EditU32("Minimum health bar", pattern.minimumHealthBar, 1u,
					boss.maximumHealthBars));
				MarkDirty(EditU32("Maximum health bar", pattern.maximumHealthBar,
					pattern.minimumHealthBar, boss.maximumHealthBars));
				MarkDirty(EditU32("Selection weight", pattern.selectionWeight, 1u, 100000u));
				MarkDirty(EditU32("Maximum consecutive uses",
					pattern.maximumConsecutiveUses, 1u, 100u));
				if (liveBoss.isValid)
				{
					std::uint64_t eligibleWeight = 0u;
					for (const PATTERN_EDIT& candidate : m_patterns)
					{
						if (candidate.selectionMode == "NORMAL" &&
							liveHealthBar >= candidate.minimumHealthBar &&
							liveHealthBar <= candidate.maximumHealthBar)
						{
							eligibleWeight += candidate.selectionWeight;
						}
					}
					const bool barEligible = liveHealthBar >= pattern.minimumHealthBar &&
						liveHealthBar <= pattern.maximumHealthBar;
					ImGui::TextDisabled("Live decision: bar gate %s | weight %.1f%% | target range checked on Server",
						barEligible ? "PASS" : "BLOCK",
						barEligible && eligibleWeight > 0u ?
							100.0 * static_cast<double>(pattern.selectionWeight) /
							static_cast<double>(eligibleWeight) : 0.0);
				}
			}
			else if (pattern.selectionMode == "HEALTH_BAR")
			{
				MarkDirty(EditU32("Trigger health bar", pattern.triggerHealthBar, 1u,
					boss.maximumHealthBars));
				MarkDirty(EditU32("Trigger order", pattern.triggerOrder, 1u, 100u));
				if (liveBoss.isValid)
					ImGui::TextDisabled("Live decision: threshold %s | once-only queue order %u",
						liveHealthBar > pattern.triggerHealthBar ? "ARMED" : "REACHED",
						pattern.triggerOrder);
			}
			if (!manualAudition)
			{
				MarkDirty(EditDouble("Minimum range", pattern.minimumRange,
					0.1f, 0.0, 1000.0));
				MarkDirty(EditDouble("Maximum range", pattern.maximumRange,
					0.1f, 0.0, 1000.0));
			}
			ImGui::SeparatorText("Server stages");
			for (std::size_t stageIndex = 0;
				stageIndex < pattern.stages.size(); ++stageIndex)
			{
				PATTERN_STAGE_EDIT& stage = pattern.stages[stageIndex];
				ImGui::PushID(static_cast<int>(stageIndex));
				const std::string stageHeader = stage.stageId + " | " + stage.actionId;
				if (ImGui::TreeNode(stageHeader.c_str()))
				{
					int stageKind = stage.stageKind == "ACTIVE" ? 1 :
						(stage.stageKind == "RECOVERY" ? 2 : 0);
					if (ImGui::Combo("Kind", &stageKind,
						"Windup\0Active\0Recovery\0"))
					{
						stage.stageKind = 0 == stageKind ? "WINDUP" :
							(1 == stageKind ? "ACTIVE" : "RECOVERY");
						m_dirty = true;
					}
					MarkDirty(EditU32("Duration ms", stage.durationMs, 1u, 600000u));
					int hitShape = 0;
					if (stage.hitShape == "CIRCLE") hitShape = 1;
					else if (stage.hitShape == "RING") hitShape = 2;
					else if (stage.hitShape == "CONE") hitShape = 3;
					else if (stage.hitShape == "BOX") hitShape = 4;
					else if (stage.hitShape == "CROSS") hitShape = 5;
					else if (stage.hitShape == "SIX_DIRECTIONS") hitShape = 6;
					if (ImGui::Combo("Collider", &hitShape,
						"None\0Circle\0Ring\0Cone\0Box\0Cross\0Six directions\0"))
					{
						static const char* shapes[] =
							{ "NONE", "CIRCLE", "RING", "CONE", "BOX", "CROSS",
							  "SIX_DIRECTIONS" };
						stage.hitShape = shapes[hitShape];
						NormalizePatternStageForShape(stage);
						m_dirty = true;
					}
					if (stage.hitShape == "CIRCLE" || stage.hitShape == "RING")
						MarkDirty(EditDouble("Outer radius", stage.hitOuterRadius,
							0.1f, 0.0, 1000.0));
					if (stage.hitShape == "RING")
						MarkDirty(EditDouble("Inner radius", stage.hitInnerRadius,
							0.1f, 0.0, 1000.0));
					if (stage.hitShape == "CONE")
						MarkDirty(EditDouble("Angle degrees", stage.hitAngleDegrees,
							1.f, 1.0, 180.0));
					if (stage.hitShape == "CONE" || stage.hitShape == "BOX" ||
						stage.hitShape == "CROSS" ||
						stage.hitShape == "SIX_DIRECTIONS")
					{
						MarkDirty(EditDouble("Length", stage.hitLength,
							0.1f, 0.0, 1000.0));
					}
					if (stage.hitShape == "BOX" || stage.hitShape == "CROSS" ||
						stage.hitShape == "SIX_DIRECTIONS")
						MarkDirty(EditDouble("Half width", stage.hitHalfWidth,
							0.1f, 0.0, 1000.0));
					if (stage.hitShape != "NONE")
					{
						MarkDirty(EditU32("Hit count", stage.hitCount, 1u, 100u));
						MarkDirty(EditU32("Hit interval ms", stage.hitIntervalMs,
							1u == stage.hitCount ? 0u : 1u, 600000u));
						MarkDirty(EditU32("Hit delay ms", stage.hitDelayMs,
							0u, 600000u));
						std::uint32_t* rate = FindDamageRate(stage.damageProfileId);
						if (nullptr != rate)
						{
							MarkDirty(EditU32("Damage rate %", *rate, 1u, 100000u));
							RenderBasis("Data/Balance/DamageProfiles.json",
								"damage:" + stage.damageProfileId, "damageRatePercent");
						}
						const bool pushRangeChanged = EditDouble(
							"Push range m (neg pulls)", stage.pushRangeM,
							0.1f, -20.0, 20.0);
						if (pushRangeChanged)
						{
							NormalizePatternStagePush(stage);
							m_dirty = true;
						}
						MarkDirty(EditU32("Push ms",
							stage.pushMs, 0.0 == stage.pushRangeM ? 0u : 1u, 600000u));
						if (ImGui::Checkbox("Knockdown", &stage.knockdown))
						{
							NormalizePatternStagePush(stage);
							m_dirty = true;
						}
						MarkDirty(EditU32("Down ms",
							stage.downMs, stage.knockdown ? 1u : 0u, 600000u));
					}
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
			ImGui::TextDisabled("Authority: Server 30 Hz | target: nearest alive player | geometry: XZ plane");
			if (liveBoss.isValid && liveBoss.strPatternId == pattern.patternId)
				ImGui::TextColored(ImVec4(0.4f, 1.f, 0.4f, 1.f), "LIVE: selected by Server snapshot");
		}
		ImGui::PopID();
	}
}

void Client::CBalanceTool::RenderValtanManagedPattern(
	VALTAN_PATTERN_VIEW& pattern,
	const char* const groupLabel,
	const std::size_t stableIndex)
{
	ImGui::PushID(groupLabel);
	ImGui::PushID(static_cast<int>(stableIndex));
	const std::string header = pattern.strDisplayName + "##" + pattern.strPatternId;
	if (ImGui::CollapsingHeader(header.c_str()))
	{
		const bool_t bManualAudition = pattern.bManualServerAudition;
		ImGui::TextDisabled("MANAGED | %s | %s", groupLabel,
			pattern.strPatternId.c_str());
		if (bManualAudition)
		{
			ImGui::TextDisabled(
				"Manual Server audition | phase %u | source chain %s | automatic selection disabled",
				pattern.iAuthoringPhase,
				pattern.strSourceAnimationChainId.c_str());
		}
		ImGui::SeparatorText(bManualAudition ?
			"Server Gameplay manual audition (hit authoring)" :
			"Server Gameplay canonical (editable)");
		ImGui::TextDisabled("Source: %s",
			m_valtanSourceJoin.gameplaySourcePath.c_str());
		ImGui::TextWrapped("Action %s | category %s | phase %u..%u",
			pattern.strActionId.c_str(), pattern.strCategory.c_str(),
			pattern.iMinimumPhase, pattern.iMaximumPhase);
		ImGui::TextWrapped("Selection %s | armor %s | phase gate %s | compatibility fallback weight %u | max repeat %u",
			pattern.strSelectionMode.c_str(), pattern.strArmorRequirement.c_str(),
			pattern.strPhaseRequirement.c_str(), pattern.iSelectionWeight,
			pattern.iMaximumConsecutiveUses);
		ImGui::TextWrapped("Health bars %d..%d | trigger %d order %u | range %.2f..%.2f m",
			pattern.iMinimumHealthBar, pattern.iMaximumHealthBar,
			pattern.iTriggerHealthBar, pattern.iTriggerOrder,
			pattern.fMinimumRange, pattern.fMaximumRange);
		ImGui::TextWrapped("Target %s | aim %s | invulnerable %s",
			pattern.strTargetPolicy.c_str(), pattern.strAimPolicy.c_str(),
			pattern.bInvulnerableWhileRunning ? "YES" : "NO");
		ImGui::SeparatorText(bManualAudition ?
			"Server decision contract (locked)" : "Server decision tuning");
		if (bManualAudition)
		{
			ImGui::TextDisabled(
				"AUDITION_ONLY keeps selection, repeat, and target range read-only until explicit rotation promotion.");
		}
		else
		{
			if ("NORMAL" == pattern.strSelectionMode)
			{
				ImGui::TextDisabled(
					"Compatibility fallback is read-only and independent of window weights above.");
			}
			MarkDirty(EditU32("Soft repeat avoidance limit",
				pattern.iMaximumConsecutiveUses, 0u, 64u));
			MarkDirty(EditFloat("Minimum target range m", pattern.fMinimumRange,
				0.1f, 0.f, 1000.f));
			MarkDirty(EditFloat("Maximum target range m", pattern.fMaximumRange,
				0.1f, 0.f, 1000.f));
			if (pattern.fMinimumRange > pattern.fMaximumRange)
			{
				ImGui::TextColored(ImVec4(1.f, 0.45f, 0.3f, 1.f),
					"Invalid draft: minimum range exceeds maximum range.");
			}
		}

		ImGui::TextDisabled(
			"Server replay is available in Boss Tool and Effect Tool; Repeat and Revive remain in Boss Tool.");

		if (pattern.ServerMotion.has_value())
		{
			const VALTAN_PATTERN_SERVER_MOTION_VIEW& motion = *pattern.ServerMotion;
			ImGui::TextWrapped(
				"Server motion %s | anchor %s | landing [%.3f, %.3f, %.3f] | apex %.2f | TAKEOFF %u..%u ms | %s %u..%u ms",
				motion.strKind.c_str(), motion.strAnchorId.c_str(),
				motion.LandingPosition[0], motion.LandingPosition[1],
				motion.LandingPosition[2], motion.fApexHeight,
				motion.iTakeoffStartMs, motion.iTakeoffEndMs,
				motion.strTravelStageId.c_str(),
				motion.iTravelStartMs, motion.iTravelEndMs);
		}
		if (!pattern.WorldEventTriggerRefs.empty())
		{
			ImGui::TextDisabled("World event projection (read-only before G09):");
			for (const auto& ref : pattern.WorldEventTriggerRefs)
			{
				ImGui::BulletText("%s / %s / %s", ref.strPatternId.c_str(),
					ref.strStageId.c_str(), ref.strTriggerKind.c_str());
			}
		}
		ImGui::SeparatorText("Pattern Presentation references (read-only)");
		ImGui::TextDisabled("Source: %s | edit through Animation/Effect Tool",
			m_valtanSourceJoin.presentationSourcePath.c_str());
		if (!pattern.CameraCueIds.empty())
		{
			ImGui::TextDisabled("Camera cues: %zu", pattern.CameraCueIds.size());
			for (const std::string& cueId : pattern.CameraCueIds)
				ImGui::BulletText("%s", cueId.c_str());
		}

		ImGui::SeparatorText(bManualAudition ?
			"Server Gameplay stage timeline (wall-clock locked; hit authoring)" :
			"Server Gameplay canonical stage timeline (editable)");
		for (std::size_t stageIndex = 0u;
			stageIndex < pattern.Stages.size(); ++stageIndex)
		{
			VALTAN_STAGE_VIEW& stage = pattern.Stages[stageIndex];
			ImGui::PushID(static_cast<int>(stageIndex));
			const std::string stageLabel = stage.strStageId + " | " +
				stage.strActionId + "##stage";
			if (ImGui::TreeNode(stageLabel.c_str()))
			{
				if (bManualAudition)
				{
					ImGui::TextDisabled(
						"Duration %u ms | locked to the promoted animation occurrence wall-clock",
						stage.iDurationMs);
				}
				else
				{
					MarkDirty(EditU32(
						"Duration ms", stage.iDurationMs, 1u, 600000u));
				}
				ImGui::TextWrapped("%s | %u ms | role %s | repeat %u | animation end %s",
					stage.strStageKind.c_str(), stage.iDurationMs,
					stage.strSequenceRole.c_str(), stage.iAuthoringRepeatCount,
					stage.strAnimationEndPolicy.c_str());
				ImGui::TextWrapped(
					"Hit %s | count %u | interval %u | delay %u | damage %s | push %.2f/%u ms | knockdown %s/%u ms",
					stage.strHitShape.c_str(), stage.iHitCount,
					stage.iHitIntervalMs, stage.iHitDelayMs,
					stage.strServerDamageProfileId.empty() ? "NONE" :
						stage.strServerDamageProfileId.c_str(),
					stage.fPushRangeM, stage.iPushMs,
					stage.bKnockdown ? "YES" : "NO", stage.iDownMs);
				if (stage.strHitShape == "CIRCLE" || stage.strHitShape == "RING")
					MarkDirty(EditFloat("Hit outer radius m", stage.fHitOuterRadius,
						0.1f, 0.f, 1000.f));
				if (stage.strHitShape == "RING")
					MarkDirty(EditFloat("Hit inner radius m", stage.fHitInnerRadius,
						0.1f, 0.f, 1000.f));
				if (stage.strHitShape == "CONE")
					MarkDirty(EditFloat("Hit angle degrees", stage.fHitAngleDegrees,
						1.f, 0.f, 180.f));
				if (stage.strHitShape == "CONE" || stage.strHitShape == "BOX" ||
					stage.strHitShape == "CROSS" ||
					stage.strHitShape == "SIX_DIRECTIONS")
				{
					MarkDirty(EditFloat("Hit length m", stage.fHitLength,
						0.1f, 0.f, 1000.f));
				}
				if (stage.strHitShape == "BOX" || stage.strHitShape == "CROSS" ||
					stage.strHitShape == "SIX_DIRECTIONS")
				{
					MarkDirty(EditFloat("Hit half width m", stage.fHitHalfWidth,
						0.1f, 0.f, 1000.f));
				}
				if (stage.Has_HitShape())
				{
					if (stage.HitOffsetsMs.empty())
					{
						MarkDirty(EditU32("Hit count", stage.iHitCount, 1u, 64u));
						MarkDirty(EditU32("First hit offset ms", stage.iHitDelayMs,
							0u, stage.iDurationMs - 1u));
						MarkDirty(EditU32("Hit interval ms", stage.iHitIntervalMs,
							stage.iHitCount > 1u ? 1u : 0u, stage.iDurationMs));
					}
					else
					{
						ImGui::TextDisabled("Explicit hit offsets (stage-relative)");
						for (std::size_t offsetIndex = 0u;
							offsetIndex < stage.HitOffsetsMs.size(); ++offsetIndex)
						{
							ImGui::PushID(static_cast<int>(offsetIndex));
							MarkDirty(EditU32("Offset ms", stage.HitOffsetsMs[offsetIndex],
								0u, stage.iDurationMs - 1u));
							ImGui::PopID();
						}
					}
					if (std::uint32_t* rate = FindDamageRate(
							stage.strServerDamageProfileId))
					{
						MarkDirty(EditU32("Damage rate %", *rate, 0u, 100000u));
					}
					const bool pushChanged = EditFloat("Push range m (negative pulls)",
						stage.fPushRangeM, 0.1f, -20.f, 20.f);
					if (pushChanged)
					{
						if (0.f == stage.fPushRangeM)
							stage.iPushMs = 0u;
						else if (0u == stage.iPushMs)
							stage.iPushMs = 1u;
						MarkDirty(true);
					}
					MarkDirty(EditU32("Push duration ms", stage.iPushMs,
						0.f == stage.fPushRangeM ? 0u : 1u, 600000u));
					if (ImGui::Checkbox("Knockdown", &stage.bKnockdown))
					{
						stage.iDownMs = stage.bKnockdown ?
							(std::max)(stage.iDownMs, 1u) : 0u;
						MarkDirty(true);
					}
					MarkDirty(EditU32("Down duration ms", stage.iDownMs,
						stage.bKnockdown ? 1u : 0u, 600000u));
				}
				for (const VALTAN_COMBAT_OBJECT_EFFECT_VIEW& object :
					stage.CombatObjectEffects)
				{
					const bool isAxeVolley = object.strCombatObjectArchetypeId ==
						"combatobject.valtan.high-jump.target-axe";
					ImGui::BulletText("Server combat-object spawn %s x%u per target",
						object.strCombatObjectArchetypeId.c_str(),
						isAxeVolley ? m_valtanAxeVolley.countPerResolvedTarget :
							object.iSpawnValue);
					if (isAxeVolley)
					{
						const std::uint32_t previousCount =
							m_valtanAxeVolley.countPerResolvedTarget;
						if (EditU32("Axes per alive player",
							m_valtanAxeVolley.countPerResolvedTarget, 1u, 8u))
						{
							if (1u == m_valtanAxeVolley.countPerResolvedTarget)
							{
								m_valtanAxeVolley.layoutKind = "TARGET_CENTER";
								m_valtanAxeVolley.radiusM = 0.0;
								m_valtanAxeVolley.startAngleDegrees = 0.0;
								m_valtanAxeVolley.angleStepDegrees = 0.0;
							}
							else if (1u == previousCount)
							{
								m_valtanAxeVolley.layoutKind = "RADIAL_AROUND_TARGET";
								m_valtanAxeVolley.radiusM = 3.0;
								m_valtanAxeVolley.angleStepDegrees = 360.0 /
									m_valtanAxeVolley.countPerResolvedTarget;
							}
							m_valtanAxeVolley.maximumTotalObjects = (std::max)(
								m_valtanAxeVolley.maximumTotalObjects,
								m_valtanAxeVolley.countPerResolvedTarget +
									m_valtanAxeVolley.arenaRandomCount);
							MarkDirty(true);
						}
						if (m_valtanAxeVolley.countPerResolvedTarget > 1u)
						{
							MarkDirty(EditDouble("Volley radius m",
								m_valtanAxeVolley.radiusM, 0.1f, 0.01, 1000.0));
							MarkDirty(EditDouble("Volley start angle",
								m_valtanAxeVolley.startAngleDegrees, 1.f,
								-360000.0, 360000.0));
							MarkDirty(EditDouble("Volley angle step",
								m_valtanAxeVolley.angleStepDegrees, 1.f,
								0.000001, 360.0));
						}
						MarkDirty(EditU32("Maximum total axe objects",
							m_valtanAxeVolley.maximumTotalObjects,
							m_valtanAxeVolley.countPerResolvedTarget +
								m_valtanAxeVolley.arenaRandomCount, 64u));
						ImGui::TextDisabled(
							"PER_ALIVE_PLAYER | overlap forbidden | atomic Server preflight");
					}
				}
				for (const VALTAN_STAGE_ACTION_VIEW& action : stage.Actions)
				{
					ImGui::BulletText("Event %s / %s -> %s (value %.3f, %u ms)",
						action.strTrigger.c_str(), action.strKind.c_str(),
						action.strTargetId.c_str(), action.fValue,
						action.iDurationMs);
				}
				for (const VALTAN_STAGE_BRANCH_VIEW& branch : stage.Branches)
				{
					ImGui::BulletText("Branch %s -> %s", branch.strOutcome.c_str(),
						branch.strNextActionId.has_value() ?
							branch.strNextActionId->c_str() : "PATTERN_END");
				}

				ImGui::SeparatorText("Pattern Presentation references (read-only)");
				ImGui::TextDisabled(
					"Edit animation occurrences and Effect cue bindings in their owning tools; this panel only joins stable IDs.");
				for (const VALTAN_CLIP_OCCURRENCE_VIEW& clip : stage.ClipOccurrences)
				{
					ImGui::BulletText(
						"Animation %s | %s | source %u ms | play %u ms @ %.3fx | loop %s",
						clip.strClipOccurrenceId.c_str(), clip.strClipName.c_str(),
						clip.iSourceStartMs, clip.iPlayMs, clip.fPlayRate,
						clip.bLoop ? "YES" : "NO");
				}
				for (const VALTAN_PRODUCT_EFFECT_CUE_VIEW& cue : stage.ProductCues)
				{
					if (VALTAN_PATTERN_EFFECT_SCALE_POLICY::OWNER_RELATIVE ==
						cue.eScalePolicy)
					{
						const BOSS_ACTOR_ENTRY* const pValtanActor =
							CActorCatalog::Find_Boss("BOSS_VALTAN");
						if (nullptr != pValtanActor)
						{
							ImGui::BulletText(
								"Effect %s | asset %s | anchor %s | %s/%s | repeat %s | scale OWNER %.2fx",
								cue.strBindingId.c_str(), cue.strEffectAssetId.c_str(),
								cue.strAnchorSlotId.c_str(), cue.strFollowPolicy.c_str(),
								cue.strStopPolicy.c_str(), cue.strRepeatPolicy.c_str(),
								pValtanActor->presentationScale);
						}
						else
						{
							ImGui::BulletText(
								"Effect %s | asset %s | anchor %s | %s/%s | repeat %s | scale OWNER (BossCatalog unavailable)",
								cue.strBindingId.c_str(), cue.strEffectAssetId.c_str(),
								cue.strAnchorSlotId.c_str(), cue.strFollowPolicy.c_str(),
								cue.strStopPolicy.c_str(), cue.strRepeatPolicy.c_str());
						}
					}
					else
					{
						const char* const scaleSpace =
							VALTAN_PATTERN_EFFECT_SCALE_POLICY::ARENA_ABSOLUTE ==
								cue.eScalePolicy ? "ARENA" : "WORLD";
						ImGui::BulletText(
							"Effect %s | asset %s | anchor %s | %s/%s | repeat %s | scale %s [%.2f, %.2f, %.2f]",
							cue.strBindingId.c_str(), cue.strEffectAssetId.c_str(),
							cue.strAnchorSlotId.c_str(), cue.strFollowPolicy.c_str(),
							cue.strStopPolicy.c_str(), cue.strRepeatPolicy.c_str(),
							scaleSpace, cue.vWorldScale.x, cue.vWorldScale.y,
							cue.vWorldScale.z);
					}
				}
				for (const VALTAN_COMBAT_OBJECT_EFFECT_VIEW& object :
					stage.CombatObjectEffects)
				{
					ImGui::BulletText(
						"Combat-object presentation %s | visual %s | Effect %s",
						object.strCombatObjectArchetypeId.c_str(),
						object.strClientVisualId.c_str(),
						object.strEffectAssetId.c_str());
				}
				if (!stage.IndependentEffectIds.empty())
				{
					for (const std::string& independentId : stage.IndependentEffectIds)
						ImGui::BulletText("Independent presentation %s", independentId.c_str());
				}
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
	}
	ImGui::PopID();
	ImGui::PopID();
}

void Client::CBalanceTool::RenderValtanSourceJoinStatus() const
{
	const char* state = "INVALID";
	ImVec4 color{ 1.f, 0.45f, 0.3f, 1.f };
	switch (m_valtanSourceJoin.state)
	{
	case VALTAN_SOURCE_JOIN_STATE::SPLIT_SOURCE_INCOMPLETE:
		state = "SPLIT SOURCE INCOMPLETE";
		break;
	case VALTAN_SOURCE_JOIN_STATE::SPLIT_SOURCE_UNVERIFIED:
		state = "SPLIT SOURCE UNVERIFIED";
		color = ImVec4(1.f, 0.75f, 0.25f, 1.f);
		break;
	case VALTAN_SOURCE_JOIN_STATE::JOINED_VALIDATED:
		state = "JOINED / VALIDATED";
		color = ImVec4(0.4f, 1.f, 0.4f, 1.f);
		break;
	case VALTAN_SOURCE_JOIN_STATE::END:
	default:
		break;
	}
	const auto shortRevision = [](const std::string& revision)
	{
		return revision.empty() ? std::string("NONE") :
			revision.substr(0u, 12u);
	};

	ImGui::SeparatorText("Valtan source ownership and strict join");
	ImGui::TextWrapped("Server Gameplay canonical (editable here): %s",
		m_valtanSourceJoin.gameplaySourcePath.c_str());
	ImGui::TextDisabled("Gameplay source revision: %s",
		shortRevision(m_valtanSourceJoin.gameplayRevision).c_str());
	ImGui::TextWrapped(
		"Pattern Presentation canonical (read-only here; edit in Animation/Effect Tool): %s",
		m_valtanSourceJoin.presentationSourcePath.c_str());
	ImGui::TextDisabled("Presentation source revision: %s",
		shortRevision(m_valtanSourceJoin.presentationRevision).c_str());
	ImGui::TextColored(color, "Join status: %s | joined revision: %s",
		state, shortRevision(m_valtanSourceJoin.joinedRevision).c_str());
	if (!m_valtanSourceJoin.diagnostic.empty())
		ImGui::TextWrapped("Join diagnostic: %s",
			m_valtanSourceJoin.diagnostic.c_str());
	ImGui::TextDisabled(
		"Join key: patternId + stageId + actionId. Product Encounter/bindings/cues are generated verification outputs, not editable sources.");
}

void Client::CBalanceTool::RenderValtanPatternAuthoring()
{
	RenderValtanSourceJoinStatus();
	ImGui::TextWrapped("%s", m_valtanPatternStatus.c_str());
	ImGui::TextDisabled(
		"Balance Tool edits only the Server Gameplay lane. Animation occurrences, Effect cues, and camera references remain presentation-owned and read-only in this panel.");
	if (ImGui::TreeNode("Phase and selection structure"))
	{
		for (const VALTAN_PHASE_VIEW& phase : m_valtanPatternTree.Phases)
		{
			ImGui::BulletText("Phase %u | bars %d..%d | gate %s @ %d | gimmicks %zu | rotation refs %zu",
				phase.iPhaseNumber, phase.iBandTopHealthBar,
				phase.iBandBottomHealthBar,
				phase.strGatePatternId.empty() ? "NONE" : phase.strGatePatternId.c_str(),
				phase.iGateTriggerHealthBar, phase.GimmickIndices.size(),
				phase.RotationIndices.size());
		}
		ImGui::TreePop();
	}
	ImGui::SeparatorText("Managed selection windows (next Server decision)");
	for (std::size_t windowIndex = 0u;
		windowIndex < m_valtanPatternTree.SelectionWindows.size(); ++windowIndex)
	{
		VALTAN_SELECTION_WINDOW_VIEW& window =
			m_valtanPatternTree.SelectionWindows[windowIndex];
		auto selectionSet = std::find_if(
			m_valtanPatternTree.SelectionSets.begin(),
			m_valtanPatternTree.SelectionSets.end(),
			[&window](const VALTAN_SELECTION_SET_VIEW& candidate)
			{ return candidate.strSelectionSetId == window.strSelectionSetId; });
		if (m_valtanPatternTree.SelectionSets.end() == selectionSet)
			continue;
		ImGui::PushID(static_cast<int>(windowIndex));
		const std::string label = window.strWindowId + "##selection-window";
		if (ImGui::TreeNode(label.c_str()))
		{
			ImGui::TextWrapped(
				"Phase %u | bars [%u..%u) | set %s | Product %s",
				window.iGameplayPhase, window.iMaximumHealthBarInclusive,
				window.iMinimumHealthBarExclusive,
				window.strSelectionSetId.c_str(),
				window.strCompatibilityRotationId.c_str());
			std::uint64_t enabledWeight = 0u;
			std::size_t enabledCount = 0u;
			for (const VALTAN_SELECTION_CANDIDATE_VIEW& candidate :
				selectionSet->Candidates)
			{
				if (candidate.bEnabled)
				{
					enabledWeight += candidate.iWeight;
					++enabledCount;
				}
			}
			for (std::size_t candidateIndex = 0u;
				candidateIndex < selectionSet->Candidates.size(); ++candidateIndex)
			{
				VALTAN_SELECTION_CANDIDATE_VIEW& candidate =
					selectionSet->Candidates[candidateIndex];
				ImGui::PushID(static_cast<int>(candidateIndex));
				ImGui::TextUnformatted(candidate.strPatternId.c_str());
				ImGui::SameLine();
				const bool_t blockLastDisable = candidate.bEnabled && 1u == enabledCount;
				ImGui::BeginDisabled(blockLastDisable);
				if (ImGui::Checkbox("Enabled", &candidate.bEnabled))
					MarkDirty(true);
				ImGui::EndDisabled();
				MarkDirty(EditU32("Weight", candidate.iWeight, 1u, 100000u));
				ImGui::TextDisabled("Enabled share %.2f%%",
					candidate.bEnabled && enabledWeight > 0u ?
						100.0 * candidate.iWeight /
						static_cast<double>(enabledWeight) : 0.0);
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
	}

	ImGui::SeparatorText("Managed health mechanics (next crossing evaluation)");
	std::uint32_t maximumHealthBars = 1000u;
	const auto valtanBoss = std::find_if(m_bosses.begin(), m_bosses.end(),
		[](const BOSS_EDIT& boss) { return "BOSS_VALTAN" == boss.archetypeId; });
	if (m_bosses.end() != valtanBoss)
		maximumHealthBars = valtanBoss->maximumHealthBars;
	for (std::size_t mechanicIndex = 0u;
		mechanicIndex < m_valtanPatternTree.Mechanics.size(); ++mechanicIndex)
	{
		VALTAN_MECHANIC_VIEW& mechanic =
			m_valtanPatternTree.Mechanics[mechanicIndex];
		ImGui::PushID(static_cast<int>(mechanicIndex));
		ImGui::TextWrapped("%s | %s | %s",
			mechanic.strMechanicId.c_str(), mechanic.strPatternId.c_str(),
			mechanic.strTriggerKind.c_str());
		const bool phaseBoundaryLocked =
			"VALTAN_ARENA_BREAK_109" == mechanic.strPatternId;
		ImGui::BeginDisabled(phaseBoundaryLocked);
		const bool barChanged = EditU32(
			"Health bar", mechanic.iHealthBar, 1u, maximumHealthBars);
		ImGui::EndDisabled();
		if (phaseBoundaryLocked)
		{
			ImGui::TextDisabled(
				"Phase boundary health is read-only until windows and legacy rotation topology can change atomically.");
		}
		const bool orderChanged = EditU32(
			"Same-bar trigger order", mechanic.iTriggerOrder, 1u, 100000u);
		if (barChanged || orderChanged)
		{
			if (VALTAN_PATTERN_VIEW* pattern = FindValtanPattern(
					m_valtanPatternTree, mechanic.strPatternId))
			{
				pattern->iTriggerHealthBar = static_cast<int32_t>(
					mechanic.iHealthBar);
				pattern->iTriggerOrder = mechanic.iTriggerOrder;
			}
			MarkDirty(true);
		}
		ImGui::TextDisabled("Health descending is primary; order breaks same-bar ties. %s",
			mechanic.bOncePerEncounter ? "Once per encounter" : "REARMABLE");
		ImGui::PopID();
	}

	if (!m_valtanPatternTree.LegacyRotations.empty())
	{
		ImGui::SeparatorText("Post-109 legacy rotations (read-only)");
		ImGui::TextDisabled(
			"These rows preserve Product order/duplicates but have no promoted stable selection-set IDs yet.");
		for (const VALTAN_LEGACY_ROTATION_VIEW& rotation :
			m_valtanPatternTree.LegacyRotations)
		{
			ImGui::BulletText("%s | bars %u..%u | %s",
				rotation.strRotationId.c_str(), rotation.iFromHealthBar,
				rotation.iToHealthBar, rotation.strSelectionMode.c_str());
			for (std::size_t patternIndex = 0u;
				patternIndex < rotation.PatternIds.size(); ++patternIndex)
			{
				ImGui::TextDisabled("  %zu. %s", patternIndex + 1u,
					rotation.PatternIds[patternIndex].c_str());
			}
		}
	}

	ImGui::SeparatorText("Managed mechanics");
	for (std::size_t index = 0u;
		index < m_valtanPatternTree.Gimmicks.size(); ++index)
	{
		if (!m_valtanPatternTree.Gimmicks[index].bAuthoringMasterManaged)
			continue;
		RenderValtanManagedPattern(
			m_valtanPatternTree.Gimmicks[index], "MECHANIC", index);
	}
	ImGui::SeparatorText("Managed manual Server auditions");
	for (std::size_t index = 0u;
		index < m_valtanPatternTree.Rotation.size(); ++index)
	{
		VALTAN_PATTERN_VIEW& pattern = m_valtanPatternTree.Rotation[index];
		if (!pattern.bAuthoringMasterManaged ||
			!pattern.bManualServerAudition)
		{
			continue;
		}
		RenderValtanManagedPattern(
			pattern, "MANUAL AUDITION", index);
	}
	ImGui::SeparatorText("Managed rotation");
	for (std::size_t index = 0u;
		index < m_valtanPatternTree.Rotation.size(); ++index)
	{
		VALTAN_PATTERN_VIEW& pattern = m_valtanPatternTree.Rotation[index];
		if (!pattern.bAuthoringMasterManaged ||
			pattern.bManualServerAudition)
		{
			continue;
		}
		RenderValtanManagedPattern(
			pattern, "ROTATION", index);
	}

	ImGui::SeparatorText("Independent presentation");
	for (const VALTAN_INDEPENDENT_EFFECT_VIEW& effect :
		m_valtanPatternTree.IndependentEffects)
	{
		ImGui::BulletText("%s | %s | owner %s/%s | Effect %s | combat object %s",
			effect.strIndependentEffectId.c_str(), effect.strOwnership.c_str(),
			effect.strOwnerPatternId.c_str(), effect.strOwnerStageId.c_str(),
			effect.strEffectAssetId.c_str(),
			effect.strCombatObjectArchetypeId.c_str());
	}

	ImGui::SeparatorText("Legacy Product patterns (read-only until Promote)");
	for (std::size_t index = 0u; index < m_legacyPatterns.size(); ++index)
	{
		const LEGACY_PATTERN_SUMMARY& pattern = m_legacyPatterns[index];
		ImGui::PushID(static_cast<int>(index));
		const std::string label = pattern.displayName + "##legacy." + pattern.patternId;
		if (ImGui::TreeNode(label.c_str()))
		{
			ImGui::TextWrapped("%s | action %s | %s/%s | weight %u | stages %u",
				pattern.patternId.c_str(), pattern.actionId.c_str(),
				pattern.selectionMode.c_str(), pattern.phaseRequirement.c_str(),
				pattern.selectionWeight, pattern.stageCount);
			ImGui::TextDisabled(
				"Read-only legacy row. Use Boss Tool or Effect Tool for Server replay.");
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
}

void Client::CBalanceTool::RenderValtanDecisionTrace(
	const std::uint32_t serverTick)
{
	const bool valtanView = !m_showPlayers && !m_bosses.empty() &&
		m_bosses[m_selectedBoss].archetypeId == "BOSS_VALTAN";
	if (!valtanView)
		return;

	RequestValtanDecisionTrace(serverTick, false, nullptr);
	CNetworkManager& network = CNetworkManager::Get();
	const CNetworkManager::VALTAN_DECISION_TRACE_CLIENT_STATE& state =
		network.Get_ValtanDecisionTraceState();
	ImGui::SeparatorText("Server decision trace");
	if (ImGui::Button("Refresh decision trace"))
	{
		std::string status;
		RequestValtanDecisionTrace(serverTick, true, &status);
		m_status = std::move(status);
	}
	ImGui::SameLine();
	if (state.isQueryPending)
		ImGui::TextDisabled("QUERY PENDING #%u",
			state.iSubmittedRequestSequence);
	else if (state.hasLatestResponse)
		ImGui::TextDisabled("%s #%u",
			DescribeValtanDecisionQueryResult(state.eLatestResponse),
			state.iLatestResponseRequestSequence);
	else
		ImGui::TextDisabled("NO SERVER RESPONSE");

	if (!state.hasLatestTrace)
	{
		ImGui::TextWrapped(
			"No Server-authored selector trace is available. Release builds answer this known query with a typed rejection.");
		return;
	}

	const LostArk::Shared::VALTAN_DECISION_TRACE_WIRE& trace =
		state.LatestTrace;
	const std::string revision =
		LostArk::Shared::Format_GameplayDataRevision(
			state.LatestDefinitionRevision);
	ImGui::Text("Trace %llu | tick %u | definition %.12s",
		static_cast<unsigned long long>(trace.iTraceSequence),
		trace.iServerTick,
		revision.empty() ? "NONE" : revision.c_str());
	ImGui::Text("Decision %s -> %s",
		DescribeValtanDecisionSource(trace.eSource),
		DescribeValtanDecisionResult(trace.eResult));
	ImGui::TextWrapped("Selected %s | pending %s (%s)",
		trace.strSelectedPatternId.empty() ? "-" :
			trace.strSelectedPatternId.c_str(),
		trace.strPendingPatternId.empty() ? "-" :
			trace.strPendingPatternId.c_str(),
		DescribeValtanDecisionSource(trace.ePendingSource));
	ImGui::TextWrapped("Rotation %s step %u | pattern sequence %u -> %u",
		trace.strRotationId.empty() ? "-" : trace.strRotationId.c_str(),
		trace.iRotationStepIndex,
		trace.iPatternSequenceBeforeDecision,
		trace.iExpectedPatternSequence);
	ImGui::Text("HP %u / %u | bar %u | phase %u",
		trace.iCurrentHp, trace.iMaximumHp, trace.iHealthBar,
		static_cast<unsigned int>(trace.iGameplayPhase));
	ImGui::Text("Target %u | distance %.2f | intro %s",
		trace.iTargetNetEntityId, trace.fTargetDistance,
		trace.isIntroPatternConsumed ? "CONSUMED" : "PENDING");
	ImGui::Text("RNG ticket %llu / total %llu",
		static_cast<unsigned long long>(trace.iRandomTicket),
		static_cast<unsigned long long>(trace.iTotalWeight));
	ImGui::TextDisabled("raw %016llx | mixed %016llx",
		static_cast<unsigned long long>(trace.iRawRandomInput),
		static_cast<unsigned long long>(trace.iMixedRandomValue));
	if (trace.isMaximumConsecutiveRelaxed)
		ImGui::TextDisabled("Soft repeat cap relaxed: every otherwise eligible candidate was blocked.");
	if (trace.areCandidatesTruncated)
		ImGui::TextDisabled("Candidate list truncated by the Server trace bound.");

	constexpr ImGuiTableFlags TABLE_FLAGS =
		ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
		ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY |
		ImGuiTableFlags_SizingFixedFit;
	if (ImGui::BeginTable(
			"##ValtanDecisionCandidates", 7, TABLE_FLAGS,
			ImVec2(0.f, 240.f), 900.f))
	{
		ImGui::TableSetupColumn("Pattern", ImGuiTableColumnFlags_WidthFixed, 170.f);
		ImGui::TableSetupColumn("Pick", ImGuiTableColumnFlags_WidthFixed, 38.f);
		ImGui::TableSetupColumn("Exclusion", ImGuiTableColumnFlags_WidthFixed, 250.f);
		ImGui::TableSetupColumn("Auth / Eff", ImGuiTableColumnFlags_WidthFixed, 82.f);
		ImGui::TableSetupColumn("Effective %", ImGuiTableColumnFlags_WidthFixed, 80.f);
		ImGui::TableSetupColumn("Ticket interval", ImGuiTableColumnFlags_WidthFixed, 130.f);
		ImGui::TableSetupColumn("Cooldown / Repeat", ImGuiTableColumnFlags_WidthFixed, 140.f);
		ImGui::TableHeadersRow();
		for (const LostArk::Shared::VALTAN_DECISION_TRACE_CANDIDATE_WIRE&
			candidate : trace.Candidates)
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::TextUnformatted(candidate.strPatternId.c_str());
			ImGui::TableSetColumnIndex(1);
			ImGui::TextUnformatted(candidate.isSelected ? "YES" : "-");
			ImGui::TableSetColumnIndex(2);
			const std::string exclusions =
				DescribeValtanDecisionExclusions(candidate.iExclusionMask);
			ImGui::TextUnformatted(exclusions.c_str());
			ImGui::TableSetColumnIndex(3);
			ImGui::Text("%u / %u", candidate.iAuthoredWeight,
				candidate.iEffectiveWeight);
			ImGui::TableSetColumnIndex(4);
			if (0u == trace.iTotalWeight)
				ImGui::TextUnformatted("-");
			else
				ImGui::Text("%.2f%%",
					static_cast<double>(candidate.iEffectiveWeight) * 100.0 /
					static_cast<double>(trace.iTotalWeight));
			ImGui::TableSetColumnIndex(5);
			if (0u == candidate.iEffectiveWeight)
				ImGui::TextUnformatted("-");
			else
				ImGui::Text("[%llu, %llu)",
					static_cast<unsigned long long>(
						candidate.iWeightBeginInclusive),
					static_cast<unsigned long long>(
						candidate.iWeightEndExclusive));
			ImGui::TableSetColumnIndex(6);
			if (0u == candidate.iMaximumConsecutiveUses)
				ImGui::Text("%u ticks / %u / INF",
					candidate.iCooldownRemainingTicks,
					candidate.iConsecutiveUses);
			else
				ImGui::Text("%u ticks / %u / %u",
					candidate.iCooldownRemainingTicks,
					candidate.iConsecutiveUses,
					candidate.iMaximumConsecutiveUses);
		}
		ImGui::EndTable();
	}
}

bool Client::CBalanceTool::RequestValtanDecisionTrace(
	const std::uint32_t serverTick,
	const bool force,
	std::string* status)
{
	const bool valtanView = !m_showPlayers && !m_bosses.empty() &&
		m_bosses[m_selectedBoss].archetypeId == "BOSS_VALTAN";
	const bool inValtanArena = ETOUI(LEVEL::VALTAN_ARENA) ==
		CGameInstance::Get().Get_CurrentLevelID();
	CNetworkManager& network = CNetworkManager::Get();
	const CNetworkManager::VALTAN_DECISION_TRACE_CLIENT_STATE& state =
		network.Get_ValtanDecisionTraceState();
	if (!valtanView || !inValtanArena || !network.Is_Connected())
	{
		if (nullptr != status)
			*status = "Server decision trace requires a connected Valtan Arena.";
		return false;
	}
	if (state.isQueryPending)
	{
		if (nullptr != status)
			*status = "A Server decision trace query is already pending.";
		return false;
	}
	constexpr std::uint32_t POLL_INTERVAL_TICKS = 15u;
	if (!force && 0u != m_valtanDecisionLastQueryServerTick &&
		static_cast<std::uint32_t>(
			serverTick - m_valtanDecisionLastQueryServerTick) <
			POLL_INTERVAL_TICKS)
	{
		return false;
	}

	m_valtanDecisionTraceRequestSequence =
		(std::numeric_limits<std::uint32_t>::max)() ==
			m_valtanDecisionTraceRequestSequence ?
			1u : m_valtanDecisionTraceRequestSequence + 1u;
	const std::uint64_t afterTraceSequence = state.hasLatestTrace ?
		state.LatestTrace.iTraceSequence : 0u;
	if (!network.Send_ValtanDecisionTraceQuery(
			m_valtanDecisionTraceRequestSequence,
			"boss.valtan.center", afterTraceSequence))
	{
		if (nullptr != status)
			*status = "Could not submit the typed Server decision trace query.";
		return false;
	}
	m_valtanDecisionLastQueryServerTick = serverTick;
	if (nullptr != status)
		*status = "Server decision trace query submitted.";
	return true;
}

void Client::CBalanceTool::RenderLiveVerification()
{
	const HUD_PLAYER_STATE& player = CCombatHUDViewModel::Get().Get_Player();
	const HUD_BOSS_STATE& boss = CCombatHUDViewModel::Get().Get_Boss();
	ImGui::TextUnformatted("Server snapshot");
	ImGui::Separator();
	if (player.isValid)
	{
		ImGui::Text("Player HP %u / %u", player.iCurrentHp, player.iMaximumHp);
		ImGui::Text("Resource %u / %u", player.iCurrentResource, player.iMaximumResource);
		ImGui::Text("Server tick %u", player.iServerTick);
		ImGui::Text("Combat ready: %s", player.isCombatReady ? "YES" : "PROTECTED");
		if (0u == player.iCurrentHp)
			ImGui::TextDisabled("Revive is available in F1 -> Boss Tool.");
	}
	else
		ImGui::TextDisabled("No replicated player snapshot");
	if (boss.isValid)
	{
		ImGui::SeparatorText("Boss live state");
		std::uint32_t currentHealthBar = 0u;
		if (!m_bosses.empty() && boss.iCurrentHp > 0u && boss.iMaximumHp > 0u)
		{
			const std::uint64_t scaled = static_cast<std::uint64_t>(boss.iCurrentHp) *
				m_bosses[m_selectedBoss].maximumHealthBars;
			currentHealthBar = static_cast<std::uint32_t>(
				(scaled + boss.iMaximumHp - 1u) / boss.iMaximumHp);
		}
		ImGui::Text("%s  HP %u / %u | bars %u / %u", boss.strDisplayName.c_str(),
			boss.iCurrentHp, boss.iMaximumHp, currentHealthBar,
			m_bosses.empty() ? 0u : m_bosses[m_selectedBoss].maximumHealthBars);
		ImGui::Text("Phase %u | pattern %s", boss.iPhase,
			boss.strPatternId.empty() ? "IDLE" : boss.strPatternId.c_str());
		ImGui::Text("Sequence %u | stage %u | action %s",
			boss.iPatternSequence, boss.iPatternStageIndex,
			boss.strActionId.empty() ? "-" : boss.strActionId.c_str());
		if (const VALTAN_PATTERN_VIEW* selected =
				FindValtanPattern(m_valtanPatternTree, boss.strPatternId);
			nullptr != selected && selected->bAuthoringMasterManaged)
		{
			ImGui::Text("Selected managed pattern: %s (%s)",
				selected->strDisplayName.c_str(), selected->strSelectionMode.c_str());
		}
		else
		{
			const auto legacy = std::find_if(
				m_legacyPatterns.begin(), m_legacyPatterns.end(),
				[&boss](const LEGACY_PATTERN_SUMMARY& pattern)
				{ return pattern.patternId == boss.strPatternId; });
			if (m_legacyPatterns.end() != legacy)
				ImGui::Text("Selected legacy pattern: %s (%s)",
					legacy->displayName.c_str(), legacy->selectionMode.c_str());
		}
	}
	else
		ImGui::TextDisabled("No replicated boss snapshot");
	RenderValtanDecisionTrace(player.iServerTick);
	ImGui::SeparatorText("Damage events");
	const auto& events = CCombatHUDViewModel::Get().Get_DamageEvents();
	const std::size_t begin = events.size() > 16u ? events.size() - 16u : 0u;
	for (std::size_t index = events.size(); index > begin; --index)
	{
		const HUD_DAMAGE_EVENT& event = events[index - 1u];
		ImGui::Text("t%u %s %u -> entity %u", event.iServerTick,
			event.Event.isOutgoing ? "OUT" : "IN",
			event.Event.iAmount, event.Event.iTargetNetEntityId);
	}
	if (events.empty())
		ImGui::TextDisabled("Use a server-approved skill or let Valtan hit a player.");
	ImGui::SeparatorText("Apply policy");
	ImGui::TextWrapped(
		"Save is the only author action. Internally it validates stable-ID joins, persists authoring, builds Product data, then requests a fail-closed live update when supported. A failed stage preserves the active runtime and reports the exact stopping point.");
}
#endif

bool Client::CBalanceTool::ValidateDraft(std::string& status) const
{
	if (m_players.size() != 6u || m_skills.size() != 94u ||
		m_damageProfiles.size() != 108u || m_bosses.size() != 1u ||
		m_patterns.empty())
	{
		status = "Draft required collections are incomplete.";
		return false;
	}
	const auto isKnownStance = [](const std::string& value)
	{
		return value == "NONE" || value == "LANCE_MASTER_LONG_SPEAR" ||
			value == "LANCE_MASTER_SHORT_SPEAR" || value == "WARLORD_NORMAL" ||
			value == "WARLORD_DEFENSE";
	};
	std::uint32_t maximumPlayerResource = 0u;
	std::uint32_t maximumPlayerIdentity = 0u;
	for (const PLAYER_EDIT& player : m_players)
	{
		if (0u == player.maximumHp || 0u == player.maximumResource ||
			0u == player.resourceRegenPerSecond || 0u == player.attackPower ||
			0u == player.defense || player.resourceRegenPerSecond > player.maximumResource ||
			!std::isfinite(player.moveSpeed) || player.moveSpeed <= 0.f ||
			!std::isfinite(player.defenseStanceMoveSpeedScale) ||
			player.defenseStanceMoveSpeedScale <= 0.f ||
			player.defenseStanceMoveSpeedScale > 1.f ||
			player.identityCyclic > 1u ||
			player.identityStanceSwitchCost > player.maximumIdentity ||
			(0u == player.maximumIdentity &&
				(0u != player.identityRegenPerSecond ||
				 0u != player.identityDrainPerSecond ||
				 0u != player.identityStanceSwitchCost ||
				 0u != player.identityCyclic)) ||
			(0u != player.identityCyclic &&
				(0u == player.identityRegenPerSecond ||
				 0u != player.identityDrainPerSecond ||
				 0u != player.identityStanceSwitchCost)) ||
			!isKnownStance(player.defaultStance))
		{
			status = "Player draft is invalid: " + player.characterClass;
			return false;
		}
		maximumPlayerResource = (std::max)(
			maximumPlayerResource, player.maximumResource);
		maximumPlayerIdentity = (std::max)(
			maximumPlayerIdentity, player.maximumIdentity);
	}
	std::unordered_set<std::uint32_t> skillIds;
	std::unordered_set<std::string> classesWithIdentityCost;
	for (const SKILL_EDIT& skill : m_skills)
	{
		const bool dealsDamage = !skill.damageProfileId.empty();
		const bool isActive = "ACTIVE" == skill.skillKind;
		const bool isCombo = "COMBO" == skill.skillKind;
		const bool isHold = "HOLD" == skill.skillKind;
		const bool isCounter = "COUNTER" == skill.skillKind;
		const bool isStandup = "STANDUP" == skill.skillKind;
		if (0u == skill.skillId || 0u == skill.actionDurationMs ||
			!skillIds.insert(skill.skillId).second ||
			skill.characterClass.empty() || skill.inputSlot.empty() ||
			skill.displayName.empty() || skill.actionId.empty() ||
			skill.hitTimeMs > skill.actionDurationMs ||
			skill.resourceCost > maximumPlayerResource ||
			skill.identityCost > maximumPlayerIdentity ||
			!std::isfinite(skill.maximumRange) ||
			!std::isfinite(skill.movementDistance) || skill.movementDistance < 0.f ||
			(dealsDamage && (nullptr == FindDamageRate(skill.damageProfileId) ||
				skill.maximumRange <= 0.f)) ||
			(!dealsDamage && (skill.maximumRange != 0.f || 0u != skill.hitTimeMs)) ||
			!(isActive || isCombo || isHold || isCounter || isStandup) ||
			((isActive || isStandup) &&
				(0u == skill.cooldownMs || !skill.comboStages.empty())) ||
			(isStandup && (dealsDamage || "NONE" != skill.requiredStance ||
				"NONE" != skill.setsStance)) ||
			(isCombo &&
				(skill.comboStages.size() < 2u || skill.comboStages.size() > 8u)) ||
			(isHold && (0u == skill.cooldownMs || 3u != skill.comboStages.size())) ||
			(isCounter && (0u == skill.cooldownMs || 2u != skill.comboStages.size())) ||
			!isKnownStance(skill.requiredStance) || !isKnownStance(skill.setsStance))
		{
			status = "Skill draft is invalid: " + std::to_string(skill.skillId);
			return false;
		}
		if (0u != skill.identityCost)
			classesWithIdentityCost.insert(skill.characterClass);

		std::uint64_t stagedDurationMs = 0u;
		for (std::size_t index = 0; index < skill.comboStages.size(); ++index)
		{
			const COMBO_STAGE_EDIT& stage = skill.comboStages[index];
			const bool isFinalStage =
				index + 1u == skill.comboStages.size();
			const bool isAutomaticComboStage = isCombo && !isFinalStage &&
				0u == stage.inputOpenMs && 0u == stage.inputCloseMs;
			const bool basicTimingInvalid = 0u == stage.actionDurationMs ||
				stage.hitTimeMs > stage.comboAdvanceMs ||
				stage.comboAdvanceMs > stage.actionDurationMs;
			const bool comboWindowInvalid = isCombo &&
				((isAutomaticComboStage &&
					stage.comboAdvanceMs != stage.actionDurationMs) ||
				 (!isFinalStage && !isAutomaticComboStage &&
					(stage.inputOpenMs >= stage.inputCloseMs ||
						stage.inputCloseMs > stage.actionDurationMs)) ||
				(isFinalStage &&
					(stage.comboAdvanceMs != stage.actionDurationMs ||
					 0u != stage.inputOpenMs || 0u != stage.inputCloseMs)));
			const bool holdStageInvalid = isHold &&
				(0u != stage.inputOpenMs || 0u != stage.inputCloseMs ||
					((index + 1u == skill.comboStages.size()) !=
						(0u != stage.hitTimeMs)));
			const bool counterStageInvalid = isCounter &&
				((0u == index &&
					(0u != stage.hitTimeMs ||
					 stage.inputOpenMs >= stage.inputCloseMs ||
					 stage.inputCloseMs > stage.actionDurationMs)) ||
				 (1u == index &&
					(0u == stage.hitTimeMs || 0u != stage.inputOpenMs ||
					 0u != stage.inputCloseMs)));
			if (basicTimingInvalid || comboWindowInvalid ||
				holdStageInvalid || counterStageInvalid)
			{
				status = "Staged skill draft is invalid: " +
					std::to_string(skill.skillId);
				return false;
			}
			stagedDurationMs += stage.actionDurationMs;
		}
		if (isHold && stagedDurationMs != skill.actionDurationMs)
		{
			status = "Hold stage duration sum is invalid: " +
				std::to_string(skill.skillId);
			return false;
		}
	}
	for (const PLAYER_EDIT& player : m_players)
	{
		if (0u != player.maximumIdentity &&
			0u == player.identityDrainPerSecond &&
			0u == player.identityStanceSwitchCost &&
			0u == player.identityCyclic &&
			classesWithIdentityCost.end() ==
				classesWithIdentityCost.find(player.characterClass))
		{
			status = "Player identity gauge has no spending path: " +
				player.characterClass;
			return false;
		}
	}
	for (const DAMAGE_EDIT& damage : m_damageProfiles)
	{
		if (damage.damageProfileId.empty() || 0u == damage.damageRatePercent)
		{
			status = "Damage profile draft is invalid: " + damage.damageProfileId;
			return false;
		}
	}
	for (const BOSS_EDIT& boss : m_bosses)
	{
		if (boss.archetypeId.empty() || boss.encounterId.empty() ||
			0u == boss.maximumHp || 0u == boss.maximumHealthBars ||
			boss.maximumHealthBars > 1000u || 0u == boss.attackPower ||
			!std::isfinite(boss.collisionRadius) || boss.collisionRadius <= 0.f ||
			!std::isfinite(boss.engageDistance) || boss.engageDistance <= 0.f ||
			!std::isfinite(boss.moveSpeed) || boss.moveSpeed <= 0.f ||
			(boss.phasePolicyKind != "AUTHORED_PATTERN_EVENT" &&
				boss.phasePolicyKind != "HEALTH_PERCENT_THRESHOLD") ||
			(boss.phasePolicyKind == "AUTHORED_PATTERN_EVENT" &&
				boss.archetypeId != "BOSS_VALTAN") ||
			(boss.phasePolicyKind == "HEALTH_PERCENT_THRESHOLD" &&
				(boss.phasePolicyThresholdPercent < 1u ||
					boss.phasePolicyThresholdPercent > 99u)))
		{
			status = "Boss draft is invalid: " + boss.archetypeId;
			return false;
		}
	}
	if (m_encounterId.empty() || m_encounterBossArchetypeId.empty() ||
		"server" != m_encounterAuthority || 30u != m_fixedTickHz ||
		m_encounterIntroPatternId.empty() ||
		m_encounterBossArchetypeId != m_bosses.front().archetypeId ||
		m_encounterId != m_bosses.front().encounterId)
	{
		status = "Encounter draft header is invalid.";
		return false;
	}
	std::unordered_set<std::string> patternIds;
	std::unordered_set<std::string> serverMotionAnchorIds;
	bool foundIntroPattern = false;
	for (const PATTERN_EDIT& pattern : m_patterns)
	{
		foundIntroPattern = foundIntroPattern ||
			(pattern.patternId == m_encounterIntroPatternId &&
			 pattern.selectionMode == "NORMAL");
		const bool normal = pattern.selectionMode == "NORMAL";
		const bool healthBar = pattern.selectionMode == "HEALTH_BAR";
		const bool manualAudition = pattern.selectionMode == "AUDITION_ONLY";
		const bool validSelection = normal ?
			(pattern.minimumHealthBar >= 1u &&
				pattern.maximumHealthBar >= pattern.minimumHealthBar &&
				pattern.maximumHealthBar <= m_bosses[m_selectedBoss].maximumHealthBars &&
				0u == pattern.triggerHealthBar && 0u == pattern.triggerOrder &&
				pattern.selectionWeight > 0u && pattern.maximumConsecutiveUses > 0u) :
			(healthBar ? (0u == pattern.minimumHealthBar &&
				0u == pattern.maximumHealthBar && pattern.triggerHealthBar >= 1u &&
				pattern.triggerHealthBar <= m_bosses[m_selectedBoss].maximumHealthBars &&
				pattern.triggerOrder > 0u && 0u == pattern.selectionWeight &&
				0u == pattern.maximumConsecutiveUses) :
			(manualAudition && 0u == pattern.minimumHealthBar &&
				0u == pattern.maximumHealthBar && 0u == pattern.triggerHealthBar &&
				0u == pattern.triggerOrder && 0u == pattern.selectionWeight &&
				0u == pattern.maximumConsecutiveUses));
		if (pattern.patternId.empty() ||
			!patternIds.insert(pattern.patternId).second ||
			pattern.displayName.empty() ||
			pattern.actionId.empty() || pattern.sourceActionIds.empty() ||
			pattern.stages.empty() || !validSelection ||
			!std::isfinite(pattern.minimumRange) || pattern.minimumRange < 0.f ||
			!std::isfinite(pattern.maximumRange) ||
			pattern.maximumRange <= pattern.minimumRange)
		{
			status = "Pattern draft is invalid: " + pattern.patternId;
			return false;
		}
		if (pattern.serverMotion.enabled &&
			(("LEAP_TO_ANCHOR" != pattern.serverMotion.kind &&
			  "LEAP_TO_TARGET" != pattern.serverMotion.kind) ||
			 pattern.serverMotion.anchorId.empty() ||
			 pattern.serverMotion.travelStageId.empty() ||
			 !serverMotionAnchorIds.insert(
				 pattern.serverMotion.anchorId).second ||
			 !std::isfinite(pattern.serverMotion.landingX) ||
			 !std::isfinite(pattern.serverMotion.landingY) ||
			 !std::isfinite(pattern.serverMotion.landingZ) ||
			 std::abs(pattern.serverMotion.landingX) > 100000.f ||
			 std::abs(pattern.serverMotion.landingY) > 100000.f ||
			 std::abs(pattern.serverMotion.landingZ) > 100000.f ||
			 !std::isfinite(pattern.serverMotion.apexHeight) ||
			 pattern.serverMotion.apexHeight <= 0.f ||
			 pattern.serverMotion.apexHeight > 200.f ||
			 pattern.serverMotion.takeoffStartMs >=
				pattern.serverMotion.takeoffEndMs ||
			 pattern.serverMotion.travelStartMs >=
				pattern.serverMotion.travelEndMs))
		{
			status = "Pattern server motion draft is invalid: " +
				pattern.patternId;
			return false;
		}
		if (pattern.serverMotion.enabled)
		{
			const auto travelStage = std::find_if(
				pattern.stages.begin(), pattern.stages.end(),
				[&pattern](const PATTERN_STAGE_EDIT& stage)
				{
					return stage.stageId ==
						pattern.serverMotion.travelStageId;
				});
			if (travelStage == pattern.stages.begin() ||
				pattern.stages.front().durationMs <
					pattern.serverMotion.takeoffEndMs ||
				travelStage == pattern.stages.end() ||
				travelStage->durationMs < pattern.serverMotion.travelEndMs)
			{
				status = "Pattern server motion window is outside its stage: " +
					pattern.patternId;
				return false;
			}
		}
		for (const PATTERN_STAGE_EDIT& stage : pattern.stages)
		{
			const bool none = stage.hitShape == "NONE";
			const bool circle = stage.hitShape == "CIRCLE";
			const bool ring = stage.hitShape == "RING";
			const bool cone = stage.hitShape == "CONE";
			const bool box = stage.hitShape == "BOX";
			const bool cross = stage.hitShape == "CROSS";
			const bool sixDirections = stage.hitShape == "SIX_DIRECTIONS";
			const bool validKind = stage.stageKind == "WINDUP" ||
				stage.stageKind == "ACTIVE" || stage.stageKind == "RECOVERY";
			const bool finiteGeometry = std::isfinite(stage.hitOuterRadius) &&
				std::isfinite(stage.hitInnerRadius) &&
				std::isfinite(stage.hitAngleDegrees) &&
				std::isfinite(stage.hitLength) &&
				std::isfinite(stage.hitHalfWidth);
			const bool validGeometry = none ?
				(0.0 == stage.hitOuterRadius && 0.0 == stage.hitInnerRadius &&
					0.0 == stage.hitAngleDegrees && 0.0 == stage.hitLength &&
					0.0 == stage.hitHalfWidth) :
				(circle ? stage.hitOuterRadius > 0.0 &&
					0.0 == stage.hitInnerRadius && 0.0 == stage.hitAngleDegrees &&
					0.0 == stage.hitLength && 0.0 == stage.hitHalfWidth :
				(ring ? stage.hitInnerRadius > 0.0 &&
					stage.hitOuterRadius > stage.hitInnerRadius &&
					0.0 == stage.hitAngleDegrees && 0.0 == stage.hitLength &&
					0.0 == stage.hitHalfWidth :
				(cone ? stage.hitAngleDegrees > 0.0 &&
					stage.hitAngleDegrees <= 180.0 && stage.hitLength > 0.0 &&
					0.0 == stage.hitOuterRadius && 0.0 == stage.hitInnerRadius &&
					0.0 == stage.hitHalfWidth :
				((box || cross || sixDirections) ? stage.hitLength > 0.0 &&
					stage.hitHalfWidth > 0.0 && 0.0 == stage.hitOuterRadius &&
					0.0 == stage.hitInnerRadius &&
					0.0 == stage.hitAngleDegrees : false))));
			const bool validHit = none ?
				(0u == stage.hitCount && 0u == stage.hitIntervalMs &&
					0u == stage.hitDelayMs &&
					stage.damageProfileId.empty()) :
				(stage.hitCount > 0u &&
					(1u == stage.hitCount ? 0u == stage.hitIntervalMs :
						stage.hitIntervalMs > 0u) &&
					stage.hitDelayMs < stage.durationMs &&
					static_cast<std::uint64_t>(stage.hitCount - 1u) *
						stage.hitIntervalMs < stage.durationMs &&
					nullptr != FindDamageRate(stage.damageProfileId));
			const bool validPush = std::isfinite(stage.pushRangeM) &&
				std::abs(stage.pushRangeM) <= 20.0 &&
				((0.0 == stage.pushRangeM) == (0u == stage.pushMs)) &&
				(stage.knockdown == (0u != stage.downMs)) &&
				(!stage.damageProfileId.empty() ||
					(0.0 == stage.pushRangeM && !stage.knockdown));
			if (stage.stageId.empty() || stage.actionId.empty() || !validKind ||
				0u == stage.durationMs || !finiteGeometry || !validGeometry ||
				!validHit || !validPush)
			{
				status = "Pattern stage draft is invalid: " + pattern.patternId +
					"/" + stage.stageId;
				return false;
			}
		}
	}
	if (!foundIntroPattern)
	{
		status = "Encounter intro pattern does not exist.";
		return false;
	}
	status = "Draft validation passed.";
	return true;
}

bool Client::CBalanceTool::RunPipeline(const wchar_t* scriptName,
	const wchar_t* arguments, std::string& status,
	std::string* const capturedOutput) const
{
	const std::filesystem::path projectRoot = CProjectDataRoot::Get().parent_path();
	const std::filesystem::path script = projectRoot / L"Tools" / scriptName;
	if (!std::filesystem::is_regular_file(script))
	{
		status = "Pipeline script is missing.";
		return false;
	}
	std::wstring command = L"powershell.exe -NoProfile -ExecutionPolicy Bypass -File \"" +
		script.wstring() + L"\"";
	if (nullptr != arguments && L'\0' != arguments[0])
		command += L" " + std::wstring(arguments);
	std::vector<wchar_t> mutableCommand(command.begin(), command.end());
	mutableCommand.push_back(L'\0');
	std::error_code pathError;
	const std::filesystem::path temporaryDirectory =
		std::filesystem::temp_directory_path(pathError);
	if (pathError)
	{
		status = "Could not resolve temporary pipeline output: " +
			pathError.message() + ".";
		return false;
	}
	const std::filesystem::path outputPath = temporaryDirectory /
		(L"LostArk.ValtanBalancePipeline." + std::to_wstring(GetCurrentProcessId()) +
			L"." + std::to_wstring(GetTickCount64()) + L".log");
	SECURITY_ATTRIBUTES security{};
	security.nLength = sizeof(security);
	security.bInheritHandle = TRUE;
	const HANDLE outputHandle = CreateFileW(
		outputPath.c_str(), GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_DELETE, &security, CREATE_ALWAYS,
		FILE_ATTRIBUTE_TEMPORARY, nullptr);
	if (INVALID_HANDLE_VALUE == outputHandle)
	{
		status = "Could not create pipeline output capture (Win32 " +
			std::to_string(GetLastError()) + ").";
		return false;
	}
	const HANDLE inputHandle = CreateFileW(
		L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		&security, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (INVALID_HANDLE_VALUE == inputHandle)
	{
		const DWORD error = GetLastError();
		CloseHandle(outputHandle);
		std::filesystem::remove(outputPath, pathError);
		status = "Could not create pipeline input handle (Win32 " +
			std::to_string(error) + ").";
		return false;
	}
	STARTUPINFOW startup{};
	startup.cb = sizeof(startup);
	startup.dwFlags = STARTF_USESTDHANDLES;
	startup.hStdInput = inputHandle;
	startup.hStdOutput = outputHandle;
	startup.hStdError = outputHandle;
	PROCESS_INFORMATION process{};
	const BOOL created = CreateProcessW(nullptr, mutableCommand.data(), nullptr,
		nullptr, TRUE, CREATE_NO_WINDOW, nullptr, projectRoot.c_str(), &startup,
		&process);
	const DWORD createError = created ? ERROR_SUCCESS : GetLastError();
	CloseHandle(inputHandle);
	CloseHandle(outputHandle);
	if (!created)
	{
		std::filesystem::remove(outputPath, pathError);
		status = "Could not start the balance pipeline (Win32 " +
			std::to_string(createError) + ").";
		return false;
	}
	CloseHandle(process.hThread);
	const DWORD wait = WaitForSingleObject(process.hProcess, 120000u);
	if (WAIT_TIMEOUT == wait)
	{
		TerminateProcess(process.hProcess, 124u);
		WaitForSingleObject(process.hProcess, 5000u);
		CloseHandle(process.hProcess);
		const std::string output = ReadTextFile(outputPath);
		std::filesystem::remove(outputPath, pathError);
		if (nullptr != capturedOutput)
			*capturedOutput = output;
		status = "Balance pipeline timed out and its owned process was terminated.";
		return false;
	}
	if (WAIT_FAILED == wait)
	{
		const DWORD waitError = GetLastError();
		CloseHandle(process.hProcess);
		const std::string output = ReadTextFile(outputPath);
		std::filesystem::remove(outputPath, pathError);
		if (nullptr != capturedOutput)
			*capturedOutput = output;
		status = "Could not wait for the balance pipeline (Win32 " +
			std::to_string(waitError) + ").";
		return false;
	}
	DWORD exitCode = 1u;
	const bool readExitCode = 0 != GetExitCodeProcess(process.hProcess, &exitCode);
	CloseHandle(process.hProcess);
	const std::string output = ReadTextFile(outputPath);
	std::filesystem::remove(outputPath, pathError);
	if (nullptr != capturedOutput)
		*capturedOutput = output;
	const bool succeeded = readExitCode && 0u == exitCode;
	status = succeeded ? "Balance pipeline succeeded." :
		"Balance pipeline failed with exit code " + std::to_string(exitCode) + ".";
	return succeeded;
}

bool Client::CBalanceTool::QueryValtanSourceRevision(
	std::string& sourceRevision,
	std::string& authoringRevision,
	VALTAN_SOURCE_JOIN_STATUS& sourceJoin,
	std::string& status) const
{
	std::string captured;
	std::string processStatus;
	const bool processSucceeded = RunPipeline(
		L"ValtanPipeline\\Publish-ValtanTuningRuntimeSet.ps1",
		L"-Mode SourceManifest", processStatus, &captured);
	VALTAN_PIPELINE_RESULT result;
	if (!ParseValtanPipelineResult(captured, result, status))
	{
		if (!processSucceeded && captured.empty())
			status = processStatus;
		return false;
	}
	if (!processSucceeded || !result.ok || result.command != "SOURCE_MANIFEST" ||
		!result.hasAuthoringRevisionField ||
		!IsLowerSha256(result.sourceRevision) ||
		(!result.authoringRevision.empty() &&
			(!IsLowerSha256(result.authoringRevision) ||
				result.authoringRevision != result.sourceRevision)))
	{
		status = !result.diagnostic.empty() ? result.diagnostic :
			"Valtan source-manifest result identity is invalid: " + processStatus;
		return false;
	}
	std::error_code pathError;
	const std::filesystem::path repositoryRoot =
		std::filesystem::weakly_canonical(
			CProjectDataRoot::Get().parent_path(), pathError);
	std::filesystem::path pointerPath;
	if (pathError || repositoryRoot.empty() ||
		!ResolveFixedValtanAuthoringPath(repositoryRoot,
			L"Intermediate/ValtanTuningAuthoring/current-authoring.json",
			pointerPath, status))
	{
		if (status.empty())
			status = "Could not inspect the fixed Valtan authoring pointer path.";
		return false;
	}
	if (result.authoringRevision.empty() &&
		std::filesystem::exists(pointerPath, pathError))
	{
		status = "Valtan source-manifest result omitted an existing authoring pointer.";
		return false;
	}
	if (pathError)
	{
		status = "Could not inspect the fixed Valtan authoring pointer: " +
			pathError.message() + ".";
		return false;
	}
	sourceRevision = result.sourceRevision;
	authoringRevision = result.authoringRevision;
	sourceJoin = {};
	sourceJoin.gameplaySourcePath = VALTAN_GAMEPLAY_SOURCE_PATH;
	sourceJoin.presentationSourcePath = VALTAN_PRESENTATION_SOURCE_PATH;
	sourceJoin.gameplayRevision = result.gameplaySourceRevision;
	sourceJoin.presentationRevision = result.presentationSourceRevision;
	const bool hasGameplaySource = !sourceJoin.gameplayRevision.empty();
	const bool hasPresentationSource =
		!sourceJoin.presentationRevision.empty();
	if (!hasGameplaySource || !hasPresentationSource)
	{
		sourceJoin.state =
			VALTAN_SOURCE_JOIN_STATE::SPLIT_SOURCE_INCOMPLETE;
		sourceJoin.diagnostic =
			"Split-source transition is incomplete; gameplay=" +
			std::string(hasGameplaySource ? "PRESENT" : "MISSING") +
			", presentation=" +
			std::string(hasPresentationSource ? "PRESENT" : "MISSING") +
			". No joined revision is claimed.";
	}
	else if (!result.hasSplitJoinValidatedField ||
		!result.splitJoinValidated)
	{
		sourceJoin.state =
			VALTAN_SOURCE_JOIN_STATE::SPLIT_SOURCE_UNVERIFIED;
		sourceJoin.diagnostic =
			"Both split source hashes are present, but the pipeline did not "
			"prove their strict stable-ID join. No joined revision is claimed.";
	}
	else
	{
		sourceJoin.state = VALTAN_SOURCE_JOIN_STATE::JOINED_VALIDATED;
		sourceJoin.joinedRevision = result.sourceRevision;
		sourceJoin.diagnostic =
			"Strict gameplay/presentation stable-ID join admitted by the pipeline.";
	}
	status = authoringRevision.empty() ?
		"Valtan repository source manifest loaded; split join state is reported separately." :
		"Valtan saved authoring head loaded; split join state is reported separately.";
	return true;
}

bool Client::CBalanceTool::BuildValtanDraftPatch(
	std::string& output, std::string& status) const
{
	if (!IsLowerSha256(m_valtanSourceRevision))
	{
		status = "Valtan draft has no admitted source revision.";
		return false;
	}
	std::vector<std::string> operations;
	const auto append = [&operations](std::ostringstream& operation)
	{
		operations.push_back(operation.str());
	};
	const auto appendBossField = [&](const char* field, const std::string& value)
	{
		std::ostringstream operation;
		operation << "    { \"op\": \"SET_BOSS_BASE_FIELD\", "
			"\"bossArchetypeId\": \"BOSS_VALTAN\", \"field\": "
			<< Quote(field) << ", \"value\": " << value << " }";
		append(operation);
	};

	const auto currentBoss = std::find_if(m_bosses.begin(), m_bosses.end(),
		[](const BOSS_EDIT& boss) { return boss.archetypeId == "BOSS_VALTAN"; });
	const auto loadedBoss = std::find_if(m_loadedBosses.begin(), m_loadedBosses.end(),
		[](const BOSS_EDIT& boss) { return boss.archetypeId == "BOSS_VALTAN"; });
	if (m_bosses.end() == currentBoss || m_loadedBosses.end() == loadedBoss)
	{
		status = "Valtan boss base row is missing from the loaded draft.";
		return false;
	}
	if (currentBoss->maximumHp != loadedBoss->maximumHp)
		appendBossField("maximumHp", std::to_string(currentBoss->maximumHp));
	if (currentBoss->maximumHealthBars != loadedBoss->maximumHealthBars)
		appendBossField("maximumHealthBars",
			std::to_string(currentBoss->maximumHealthBars));
	if (currentBoss->attackPower != loadedBoss->attackPower)
		appendBossField("attackPower", std::to_string(currentBoss->attackPower));
	if (currentBoss->collisionRadius != loadedBoss->collisionRadius)
		appendBossField("collisionRadius", FormatJsonNumber(currentBoss->collisionRadius));
	if (currentBoss->engageDistance != loadedBoss->engageDistance)
		appendBossField("engageDistance", FormatJsonNumber(currentBoss->engageDistance));
	if (currentBoss->moveSpeed != loadedBoss->moveSpeed)
		appendBossField("moveSpeed", FormatJsonNumber(currentBoss->moveSpeed));

	for (const DAMAGE_EDIT& damage : m_damageProfiles)
	{
		if (0u != damage.damageProfileId.rfind("damage.valtan.", 0u))
			continue;
		const auto loaded = std::find_if(
			m_loadedDamageProfiles.begin(), m_loadedDamageProfiles.end(),
			[&](const DAMAGE_EDIT& candidate)
			{ return candidate.damageProfileId == damage.damageProfileId; });
		if (m_loadedDamageProfiles.end() == loaded)
		{
			status = "Loaded damage snapshot is missing " + damage.damageProfileId + ".";
			return false;
		}
		if (damage.damageRatePercent == loaded->damageRatePercent)
			continue;
		std::ostringstream operation;
		operation << "    { \"op\": \"SET_DAMAGE_RATE\", "
			"\"damageProfileId\": " << Quote(damage.damageProfileId)
			<< ", \"value\": " << damage.damageRatePercent << " }";
		append(operation);
	}

	if (m_valtanPatternTree.SelectionSets.size() !=
		m_loadedValtanPatternTree.SelectionSets.size() ||
		m_valtanPatternTree.SelectionWindows.size() !=
		m_loadedValtanPatternTree.SelectionWindows.size() ||
		m_valtanPatternTree.Mechanics.size() !=
		m_loadedValtanPatternTree.Mechanics.size() ||
		m_valtanPatternTree.ManualAuditions.size() !=
		m_loadedValtanPatternTree.ManualAuditions.size())
	{
		status = "Valtan decision-model stable inventory changed during editing.";
		return false;
	}
	for (const VALTAN_MANUAL_AUDITION_VIEW& manual :
		m_valtanPatternTree.ManualAuditions)
	{
		const auto loaded = std::find_if(
			m_loadedValtanPatternTree.ManualAuditions.begin(),
			m_loadedValtanPatternTree.ManualAuditions.end(),
			[&manual](const VALTAN_MANUAL_AUDITION_VIEW& candidate)
			{
				return candidate.strPatternId == manual.strPatternId;
			});
		if (m_loadedValtanPatternTree.ManualAuditions.end() == loaded ||
			loaded->strSourceChainId != manual.strSourceChainId ||
			loaded->iAuthoringPhase != manual.iAuthoringPhase ||
			loaded->strAdmissionState != manual.strAdmissionState)
		{
			status = "Loaded Valtan manual audition identity changed: " +
				manual.strPatternId;
			return false;
		}
	}
	for (const VALTAN_SELECTION_SET_VIEW& selectionSet :
		m_valtanPatternTree.SelectionSets)
	{
		const auto loadedSet = std::find_if(
			m_loadedValtanPatternTree.SelectionSets.begin(),
			m_loadedValtanPatternTree.SelectionSets.end(),
			[&selectionSet](const VALTAN_SELECTION_SET_VIEW& candidate)
			{ return candidate.strSelectionSetId == selectionSet.strSelectionSetId; });
		if (m_loadedValtanPatternTree.SelectionSets.end() == loadedSet ||
			loadedSet->strMode != selectionSet.strMode ||
			loadedSet->Candidates.size() != selectionSet.Candidates.size())
		{
			status = "Loaded Valtan selection set is missing or changed shape: " +
				selectionSet.strSelectionSetId;
			return false;
		}
		for (const VALTAN_SELECTION_CANDIDATE_VIEW& candidate :
			selectionSet.Candidates)
		{
			const auto loadedCandidate = std::find_if(
				loadedSet->Candidates.begin(), loadedSet->Candidates.end(),
				[&candidate](const VALTAN_SELECTION_CANDIDATE_VIEW& loaded)
				{ return loaded.strPatternId == candidate.strPatternId; });
			if (loadedSet->Candidates.end() == loadedCandidate)
			{
				status = "Loaded Valtan selection candidate is missing: " +
					selectionSet.strSelectionSetId + "/" + candidate.strPatternId;
				return false;
			}
			if (candidate.iWeight != loadedCandidate->iWeight)
			{
				std::ostringstream operation;
				operation << "    { \"op\": \"SET_PATTERN_WEIGHT\", "
					"\"selectionSetId\": " << Quote(selectionSet.strSelectionSetId)
					<< ", \"patternId\": " << Quote(candidate.strPatternId)
					<< ", \"value\": " << candidate.iWeight << " }";
				append(operation);
			}
			if (candidate.bEnabled != loadedCandidate->bEnabled)
			{
				std::ostringstream operation;
				operation << "    { \"op\": \"SET_PATTERN_ENABLED\", "
					"\"selectionSetId\": " << Quote(selectionSet.strSelectionSetId)
					<< ", \"patternId\": " << Quote(candidate.strPatternId)
					<< ", \"value\": "
					<< (candidate.bEnabled ? "true" : "false") << " }";
				append(operation);
			}
		}
	}
	for (const VALTAN_MECHANIC_VIEW& mechanic : m_valtanPatternTree.Mechanics)
	{
		const auto loaded = std::find_if(
			m_loadedValtanPatternTree.Mechanics.begin(),
			m_loadedValtanPatternTree.Mechanics.end(),
			[&mechanic](const VALTAN_MECHANIC_VIEW& candidate)
			{
				return candidate.strMechanicId == mechanic.strMechanicId &&
					candidate.strPatternId == mechanic.strPatternId;
			});
		if (m_loadedValtanPatternTree.Mechanics.end() == loaded)
		{
			status = "Loaded Valtan mechanic is missing: " + mechanic.strMechanicId;
			return false;
		}
		if (mechanic.iHealthBar != loaded->iHealthBar ||
			mechanic.iTriggerOrder != loaded->iTriggerOrder)
		{
			std::ostringstream operation;
			operation << "    { \"op\": \"SET_MECHANIC_TRIGGER\", "
				"\"mechanicId\": " << Quote(mechanic.strMechanicId)
				<< ", \"patternId\": " << Quote(mechanic.strPatternId)
				<< ", \"healthBar\": " << mechanic.iHealthBar
				<< ", \"triggerOrder\": " << mechanic.iTriggerOrder << " }";
			append(operation);
		}
	}

	for (const auto* group : { &m_valtanPatternTree.Gimmicks,
		&m_valtanPatternTree.Rotation })
	{
		for (const VALTAN_PATTERN_VIEW& pattern : *group)
		{
			if (!pattern.bAuthoringMasterManaged)
				continue;
			const VALTAN_PATTERN_VIEW* loaded =
				FindValtanPattern(m_loadedValtanPatternTree, pattern.strPatternId);
			if (nullptr == loaded || !loaded->bAuthoringMasterManaged ||
				pattern.bManualServerAudition !=
					loaded->bManualServerAudition)
			{
				status = "Loaded pattern snapshot is missing " + pattern.strPatternId + ".";
				return false;
			}
			const bool_t bManualAudition = pattern.bManualServerAudition;
			if (bManualAudition &&
				(pattern.strSelectionMode != loaded->strSelectionMode ||
				 pattern.iMinimumHealthBar != loaded->iMinimumHealthBar ||
				 pattern.iMaximumHealthBar != loaded->iMaximumHealthBar ||
				 pattern.iTriggerHealthBar != loaded->iTriggerHealthBar ||
				 pattern.iTriggerOrder != loaded->iTriggerOrder ||
				 pattern.iSelectionWeight != loaded->iSelectionWeight ||
				 pattern.iMaximumConsecutiveUses !=
					loaded->iMaximumConsecutiveUses ||
				 pattern.fMinimumRange != loaded->fMinimumRange ||
				 pattern.fMaximumRange != loaded->fMaximumRange))
			{
				status = "Manual Server audition selection/repeat/range is locked: " +
					pattern.strPatternId + ".";
				return false;
			}
			if (pattern.iMaximumConsecutiveUses != loaded->iMaximumConsecutiveUses)
			{
				std::ostringstream operation;
				operation << "    { \"op\": \"SET_PATTERN_REPEAT_LIMIT\", "
					"\"patternId\": " << Quote(pattern.strPatternId)
					<< ", \"value\": " << pattern.iMaximumConsecutiveUses << " }";
				append(operation);
			}
			if (pattern.fMinimumRange != loaded->fMinimumRange ||
				pattern.fMaximumRange != loaded->fMaximumRange)
			{
				std::ostringstream operation;
				operation << "    { \"op\": \"SET_PATTERN_RANGE\", "
					"\"patternId\": " << Quote(pattern.strPatternId)
					<< ", \"minimumRangeM\": "
					<< FormatJsonNumber(pattern.fMinimumRange)
					<< ", \"maximumRangeM\": "
					<< FormatJsonNumber(pattern.fMaximumRange) << " }";
				append(operation);
			}

			for (const VALTAN_STAGE_VIEW& stage : pattern.Stages)
			{
				const VALTAN_STAGE_VIEW* loadedStage = FindValtanStage(*loaded, stage.strStageId);
				if (nullptr == loadedStage)
				{
					status = "Loaded stage snapshot is missing " + pattern.strPatternId +
						"/" + stage.strStageId + ".";
					return false;
				}
				if (bManualAudition &&
					stage.iDurationMs != loadedStage->iDurationMs)
				{
					status = "Manual Server audition stage duration is locked to the animation wall-clock: " +
						pattern.strPatternId + "/" + stage.strStageId + ".";
					return false;
				}
				if (stage.iDurationMs != loadedStage->iDurationMs)
				{
					std::ostringstream operation;
					operation << "    { \"op\": \"SET_STAGE_DURATION\", "
						"\"patternId\": " << Quote(pattern.strPatternId)
						<< ", \"stageId\": " << Quote(stage.strStageId)
						<< ", \"durationMs\": " << stage.iDurationMs << " }";
					append(operation);
				}
				const bool hitChanged =
					stage.strHitShape != loadedStage->strHitShape ||
					stage.fHitOuterRadius != loadedStage->fHitOuterRadius ||
					stage.fHitInnerRadius != loadedStage->fHitInnerRadius ||
					stage.fHitAngleDegrees != loadedStage->fHitAngleDegrees ||
					stage.fHitLength != loadedStage->fHitLength ||
					stage.fHitHalfWidth != loadedStage->fHitHalfWidth ||
					stage.iHitCount != loadedStage->iHitCount ||
					stage.iHitIntervalMs != loadedStage->iHitIntervalMs ||
					stage.iHitDelayMs != loadedStage->iHitDelayMs ||
					stage.HitOffsetsMs != loadedStage->HitOffsetsMs ||
					stage.strServerDamageProfileId != loadedStage->strServerDamageProfileId ||
					stage.fPushRangeM != loadedStage->fPushRangeM ||
					stage.iPushMs != loadedStage->iPushMs ||
					stage.bKnockdown != loadedStage->bKnockdown ||
					stage.iDownMs != loadedStage->iDownMs;
				if (!hitChanged)
					continue;
				std::ostringstream hit;
				hit << "{ \"shape\": { \"kind\": " << Quote(stage.strHitShape);
				if (stage.strHitShape == "CIRCLE")
					hit << ", \"outerRadiusM\": " << FormatJsonNumber(stage.fHitOuterRadius);
				else if (stage.strHitShape == "RING")
					hit << ", \"innerRadiusM\": " << FormatJsonNumber(stage.fHitInnerRadius)
						<< ", \"outerRadiusM\": " << FormatJsonNumber(stage.fHitOuterRadius);
				else if (stage.strHitShape == "CONE")
					hit << ", \"angleDegrees\": " << FormatJsonNumber(stage.fHitAngleDegrees)
						<< ", \"lengthM\": " << FormatJsonNumber(stage.fHitLength);
				else if (stage.strHitShape == "BOX" || stage.strHitShape == "CROSS" ||
					stage.strHitShape == "SIX_DIRECTIONS")
					hit << ", \"lengthM\": " << FormatJsonNumber(stage.fHitLength)
						<< ", \"halfWidthM\": " << FormatJsonNumber(stage.fHitHalfWidth);
				hit << " }";
				if (stage.strHitShape != "NONE")
				{
					if (!stage.HitOffsetsMs.empty())
					{
						hit << ", \"schedule\": { \"kind\": \"EXPLICIT_OFFSETS\", \"offsetsMs\": [";
						for (std::size_t offset = 0u; offset < stage.HitOffsetsMs.size(); ++offset)
						{
							if (0u != offset) hit << ", ";
							hit << stage.HitOffsetsMs[offset];
						}
						hit << "] }";
					}
					else
					{
						hit << ", \"schedule\": { \"kind\": \"INTERVAL\", "
							"\"count\": " << stage.iHitCount
							<< ", \"firstOffsetMs\": " << stage.iHitDelayMs
							<< ", \"intervalMs\": " << stage.iHitIntervalMs << " }";
					}
					hit << ", \"serverDamageProfileId\": "
						<< Quote(stage.strServerDamageProfileId)
						<< ", \"pushRangeM\": " << FormatJsonNumber(stage.fPushRangeM)
						<< ", \"pushMs\": " << stage.iPushMs
						<< ", \"knockdown\": " << (stage.bKnockdown ? "true" : "false")
						<< ", \"downMs\": " << stage.iDownMs;
				}
				hit << " }";
				std::ostringstream operation;
				operation << "    { \"op\": \"SET_STAGE_HIT\", "
					"\"patternId\": " << Quote(pattern.strPatternId)
					<< ", \"stageId\": " << Quote(stage.strStageId)
					<< ", \"hit\": " << hit.str() << " }";
				append(operation);
			}
		}
	}

	const bool volleyChanged =
		m_valtanAxeVolley.countPerResolvedTarget !=
			m_loadedValtanAxeVolley.countPerResolvedTarget ||
		m_valtanAxeVolley.layoutKind != m_loadedValtanAxeVolley.layoutKind ||
		m_valtanAxeVolley.radiusM != m_loadedValtanAxeVolley.radiusM ||
		m_valtanAxeVolley.startAngleDegrees !=
			m_loadedValtanAxeVolley.startAngleDegrees ||
		m_valtanAxeVolley.angleStepDegrees !=
			m_loadedValtanAxeVolley.angleStepDegrees ||
		m_valtanAxeVolley.allowOverlap != m_loadedValtanAxeVolley.allowOverlap ||
		m_valtanAxeVolley.maximumTotalObjects !=
			m_loadedValtanAxeVolley.maximumTotalObjects ||
		m_valtanAxeVolley.spawnScheduleKind !=
			m_loadedValtanAxeVolley.spawnScheduleKind ||
		m_valtanAxeVolley.spawnCount != m_loadedValtanAxeVolley.spawnCount ||
		m_valtanAxeVolley.spawnFirstOffsetMs !=
			m_loadedValtanAxeVolley.spawnFirstOffsetMs ||
		m_valtanAxeVolley.spawnIntervalMs !=
			m_loadedValtanAxeVolley.spawnIntervalMs ||
		m_valtanAxeVolley.arenaRandomKind !=
			m_loadedValtanAxeVolley.arenaRandomKind ||
		m_valtanAxeVolley.arenaAnchor != m_loadedValtanAxeVolley.arenaAnchor ||
		m_valtanAxeVolley.arenaRandomCount !=
			m_loadedValtanAxeVolley.arenaRandomCount ||
		m_valtanAxeVolley.arenaRandomRadiusM !=
			m_loadedValtanAxeVolley.arenaRandomRadiusM ||
		m_valtanAxeVolley.arenaHeightToleranceM !=
			m_loadedValtanAxeVolley.arenaHeightToleranceM;
	if (volleyChanged)
	{
		std::ostringstream operation;
		operation << "    { \"op\": \"SET_AXE_VOLLEY\", "
			"\"patternId\": " << Quote(m_valtanAxeVolley.patternId)
			<< ", \"stageId\": " << Quote(m_valtanAxeVolley.stageId)
			<< ", \"eventId\": " << Quote(m_valtanAxeVolley.eventId)
			<< ", \"countPerResolvedTarget\": "
			<< m_valtanAxeVolley.countPerResolvedTarget
			<< ", \"layout\": { \"kind\": "
			<< Quote(m_valtanAxeVolley.layoutKind);
		if (m_valtanAxeVolley.layoutKind == "RADIAL_AROUND_TARGET")
		{
			operation << ", \"radiusM\": " << FormatJsonNumber(m_valtanAxeVolley.radiusM)
				<< ", \"startAngleDegrees\": "
				<< FormatJsonNumber(m_valtanAxeVolley.startAngleDegrees)
				<< ", \"angleStepDegrees\": "
				<< FormatJsonNumber(m_valtanAxeVolley.angleStepDegrees);
		}
		operation << " }, \"allowOverlap\": "
			<< (m_valtanAxeVolley.allowOverlap ? "true" : "false")
			<< ", \"maximumTotalObjects\": "
			<< m_valtanAxeVolley.maximumTotalObjects
			<< ", \"spawnSchedule\": { \"kind\": "
			<< Quote(m_valtanAxeVolley.spawnScheduleKind)
			<< ", \"count\": " << m_valtanAxeVolley.spawnCount
			<< ", \"firstOffsetMs\": "
			<< m_valtanAxeVolley.spawnFirstOffsetMs
			<< ", \"intervalMs\": " << m_valtanAxeVolley.spawnIntervalMs
			<< " }, \"arenaRandom\": { \"kind\": "
			<< Quote(m_valtanAxeVolley.arenaRandomKind)
			<< ", \"anchor\": " << Quote(m_valtanAxeVolley.arenaAnchor)
			<< ", \"count\": " << m_valtanAxeVolley.arenaRandomCount
			<< ", \"radiusM\": "
			<< FormatJsonNumber(m_valtanAxeVolley.arenaRandomRadiusM)
			<< ", \"heightToleranceM\": "
			<< FormatJsonNumber(m_valtanAxeVolley.arenaHeightToleranceM)
			<< " } }";
		append(operation);
	}

	std::ostringstream document;
	document << "{\n  \"schema\": \"lostark.valtan-tuning-draft-patch\",\n"
		"  \"formatVersion\": 1,\n  \"sourceRevision\": "
		<< Quote(m_valtanSourceRevision) << ",\n  \"operations\": [\n";
	for (std::size_t index = 0u; index < operations.size(); ++index)
		document << operations[index] <<
			(index + 1u == operations.size() ? "\n" : ",\n");
	document << "  ]\n}\n";
	output = document.str();
	status = "Valtan stable-ID draft contains " +
		std::to_string(operations.size()) + " operation(s).";
	return true;
}

bool Client::CBalanceTool::RunValtanDraftCommand(
	const wchar_t* const mode, std::string& status)
{
	if (nullptr == mode || (0 != std::wcscmp(mode, L"ValidateDraft") &&
		0 != std::wcscmp(mode, L"SaveAuthoring") &&
		0 != std::wcscmp(mode, L"PublishCandidate")))
	{
		status = "Unsupported Valtan draft command.";
		return false;
	}
	const bool requiresValidatedSplitJoin =
		0 == std::wcscmp(mode, L"SaveAuthoring") ||
		0 == std::wcscmp(mode, L"PublishCandidate");
	if (requiresValidatedSplitJoin &&
		VALTAN_SOURCE_JOIN_STATE::JOINED_VALIDATED !=
			m_valtanSourceJoin.state)
	{
		status =
			"Command blocked: Server Gameplay and Pattern Presentation split "
			"sources do not have a pipeline-validated strict join.";
		return false;
	}
	std::string patchText;
	if (!BuildValtanDraftPatch(patchText, status))
		return false;
	std::error_code pathError;
	const std::filesystem::path temporaryDirectory =
		std::filesystem::temp_directory_path(pathError);
	if (pathError)
	{
		status = "Could not resolve a temporary Valtan draft directory: " +
			pathError.message() + ".";
		return false;
	}
	const std::filesystem::path patchPath = temporaryDirectory /
		(L"LostArk.ValtanDraft." + std::to_wstring(GetCurrentProcessId()) + L"." +
			std::to_wstring(GetTickCount64()) + L".json");
	if (!DurableWrite(patchPath, patchText, status))
		return false;
	std::wstring arguments = L"-Mode " + std::wstring(mode) +
		L" -DraftPatchPath \"" + patchPath.wstring() + L"\"";
	std::string captured;
	std::string processStatus;
	const bool processSucceeded = RunPipeline(
		L"ValtanPipeline\\Publish-ValtanTuningRuntimeSet.ps1",
		arguments.c_str(), processStatus, &captured);
	std::filesystem::remove(patchPath, pathError);
	VALTAN_PIPELINE_RESULT result;
	std::string parseStatus;
	if (!ParseValtanPipelineResult(captured, result, parseStatus))
	{
		status = !processSucceeded && captured.empty() ? processStatus : parseStatus;
		return false;
	}
	if (!processSucceeded || !result.ok)
	{
		status = !result.diagnostic.empty() ? result.diagnostic : processStatus;
		return false;
	}
	const std::string expectedCommand =
		0 == std::wcscmp(mode, L"ValidateDraft") ? "VALIDATE_DRAFT" :
		0 == std::wcscmp(mode, L"SaveAuthoring") ? "SAVE_AUTHORING" :
		"PUBLISH_CANDIDATE";
	if (result.command != expectedCommand)
	{
		status = "Valtan pipeline result command does not match the request.";
		return false;
	}
	if (0 == std::wcscmp(mode, L"ValidateDraft"))
	{
		m_valtanDraftValidated =
			VALTAN_SOURCE_JOIN_STATE::JOINED_VALIDATED ==
				m_valtanSourceJoin.state;
		if (!m_valtanDraftValidated)
		{
			status =
				"Valtan draft operations are structurally valid, but the split "
				"gameplay/presentation join is not validated. The draft remains "
				"unadmitted.";
		}
		else
		{
			status = "Valtan draft validation passed against joined source " +
				m_valtanSourceRevision.substr(0u, 12u) + ".";
		}
		return true;
	}
	if (0 == std::wcscmp(mode, L"SaveAuthoring"))
	{
		if (!IsLowerSha256(result.authoringRevision))
		{
			status = "Save Authoring returned no valid immutable authoring revision.";
			return false;
		}
		m_valtanAuthoringRevision = result.authoringRevision;
		m_valtanSourceRevision = result.authoringRevision;
		m_loadedDamageProfiles = m_damageProfiles;
		m_loadedBosses = m_bosses;
		m_loadedValtanPatternTree = m_valtanPatternTree;
		m_loadedValtanAxeVolley = m_valtanAxeVolley;
		m_dirty = false;
		m_valtanDraftValidated = true;
		m_valtanCandidateRevision.clear();
		m_valtanCandidateApplyClass.clear();
		if (VALTAN_SOURCE_JOIN_STATE::JOINED_VALIDATED ==
			m_valtanSourceJoin.state)
		{
			m_valtanSourceJoin.joinedRevision = m_valtanAuthoringRevision;
			m_valtanSourceJoin.diagnostic =
				"Saved immutable authoring overlay passed the same strict "
				"gameplay/presentation join transaction.";
		}
		status = "Saved immutable Valtan authoring revision " +
			m_valtanAuthoringRevision.substr(0u, 12u) + ".";
		return true;
	}
	if (!IsLowerSha256(result.candidateRevision) ||
		!result.hasApplyClassField)
	{
		status =
			"Publish Candidate returned no valid candidate revision/apply class.";
		return false;
	}
	m_valtanCandidateRevision = result.candidateRevision;
	m_valtanCandidateApplyClass = result.applyClass;
	if (m_valtanCandidateApplyClass == "HOT_RELOAD")
	{
		status = "Published immutable Valtan candidate " +
			m_valtanCandidateRevision.substr(0u, 12u) +
			". Runtime active pointer is unchanged until Hot Reload commit.";
	}
	else
	{
		status = "Published immutable Valtan candidate " +
			m_valtanCandidateRevision.substr(0u, 12u) + " with apply class " +
			m_valtanCandidateApplyClass +
			". Apply Hot Reload is blocked; use the future controlled reset/restart path.";
	}
	return true;
}

bool Client::CBalanceTool::RequestValtanHotReload(std::string& status)
{
	if (VALTAN_SOURCE_JOIN_STATE::JOINED_VALIDATED !=
		m_valtanSourceJoin.state)
	{
		status =
			"Hot Reload blocked: the split gameplay/presentation source join is not validated.";
		return false;
	}
	return CValtanTuningCommandService::Get().ApplyCandidate(
		m_valtanCandidateRevision, m_valtanCandidateApplyClass, status);
}

bool Client::CBalanceTool::Save(
	SERIALIZED_DRAFT_DOCUMENTS* const readOnlyCapture)
{
	if (nullptr == readOnlyCapture)
	{
		m_status =
			"Save blocked: this legacy serializer would rewrite generated "
			"ValtanEncounter v4 and omit owned Boss/Pattern fields. Use the "
			"lossless Valtan authoring transaction after it validates.";
		return false;
	}
	if (nullptr == readOnlyCapture && !m_dirty)
	{
		m_status = "No balance changes to save; authoring bytes were left untouched.";
		return true;
	}
	std::string status;
	if (!ValidateDraft(status))
	{
		m_status = status;
		return false;
	}
	std::ostringstream players;
	players << "{\n  \"schema\": \"lostark.player-profiles\",\n  \"formatVersion\": 2,\n  \"players\": [\n";
	for (std::size_t i = 0; i < m_players.size(); ++i)
	{
		const PLAYER_EDIT& p = m_players[i];
		players << "    {\n      \"characterClass\": " << Quote(p.characterClass)
			<< ",\n      \"maximumHp\": " << p.maximumHp
			<< ",\n      \"maximumResource\": " << p.maximumResource
			<< ",\n      \"resourceRegenPerSecond\": " << p.resourceRegenPerSecond
			<< ",\n      \"attackPower\": " << p.attackPower
			<< ",\n      \"defense\": " << p.defense
			<< ",\n      \"moveSpeed\": " << FormatJsonNumber(p.moveSpeed)
			<< ",\n      \"defenseStanceMoveSpeedScale\": "
			<< FormatJsonNumber(p.defenseStanceMoveSpeedScale)
			<< ",\n      \"maximumIdentity\": " << p.maximumIdentity
			<< ",\n      \"identityRegenPerSecond\": " << p.identityRegenPerSecond
			<< ",\n      \"identityDrainPerSecond\": " << p.identityDrainPerSecond
			<< ",\n      \"identityStanceSwitchCost\": " << p.identityStanceSwitchCost
			<< ",\n      \"identityCyclic\": " << p.identityCyclic
			<< ",\n      \"defaultStance\": " << Quote(p.defaultStance) << "\n    }"
			<< (i + 1u == m_players.size() ? "\n" : ",\n");
	}
	players << "  ]\n}\n";

	std::ostringstream damage;
	damage << "{\n  \"schema\": \"lostark.damage-profiles\",\n  \"formatVersion\": 2,\n  \"profiles\": [\n";
	for (std::size_t i = 0; i < m_damageProfiles.size(); ++i)
	{
		const DAMAGE_EDIT& p = m_damageProfiles[i];
		damage << "    { \"damageProfileId\": " << Quote(p.damageProfileId)
			<< ", \"damageRatePercent\": " << p.damageRatePercent << " }"
			<< (i + 1u == m_damageProfiles.size() ? "\n" : ",\n");
	}
	damage << "  ]\n}\n";

	std::ostringstream skills;
	skills << "{\n  \"schema\": \"lostark.player-skills\",\n  \"formatVersion\": 3,\n  \"skills\": [\n";
	for (std::size_t i = 0; i < m_skills.size(); ++i)
	{
		const SKILL_EDIT& s = m_skills[i];
		skills << "    {\n      \"skillId\": " << s.skillId
			<< ",\n      \"staggerDamage\": " << s.staggerDamage
			<< ",\n      \"partDamage\": " << s.partDamage
			<< ",\n      \"counterPower\": " << s.counterPower
			<< ",\n      \"characterClass\": " << Quote(s.characterClass)
			<< ",\n      \"inputSlot\": " << Quote(s.inputSlot)
			<< ",\n      \"displayName\": " << Quote(s.displayName)
			<< ",\n      \"actionId\": " << Quote(s.actionId)
			<< ",\n      \"skillKind\": " << Quote(s.skillKind)
			<< ",\n      \"cooldownMs\": " << s.cooldownMs
			<< ",\n      \"actionDurationMs\": " << s.actionDurationMs
			<< ",\n      \"hitTimeMs\": " << s.hitTimeMs
			<< ",\n      \"resourceCost\": " << s.resourceCost
			<< ",\n      \"identityCost\": " << s.identityCost
			<< ",\n      \"movementDistance\": " << FormatJsonNumber(s.movementDistance)
			<< ",\n      \"maximumRange\": " << FormatJsonNumber(s.maximumRange)
			<< ",\n      \"serverDamageProfileId\": " << Quote(s.damageProfileId)
			<< ",\n      \"effectId\": " << Quote(s.effectId)
			<< ",\n      \"requiredStance\": " << Quote(s.requiredStance)
			<< ",\n      \"setsStance\": " << Quote(s.setsStance)
			<< ",\n      \"comboStages\": [";
		if (!s.comboStages.empty()) skills << "\n";
		for (std::size_t stageIndex = 0; stageIndex < s.comboStages.size(); ++stageIndex)
		{
			const COMBO_STAGE_EDIT& stage = s.comboStages[stageIndex];
			skills << "        { \"actionDurationMs\": " << stage.actionDurationMs
				<< ", \"hitTimeMs\": " << stage.hitTimeMs
				<< ", \"comboAdvanceMs\": " << stage.comboAdvanceMs
				<< ", \"inputOpenMs\": " << stage.inputOpenMs
				<< ", \"inputCloseMs\": " << stage.inputCloseMs << " }"
				<< (stageIndex + 1u == s.comboStages.size() ? "\n" : ",\n");
		}
		skills << "      ]\n    }" << (i + 1u == m_skills.size() ? "\n" : ",\n");
	}
	skills << "  ]\n}\n";

	std::ostringstream bosses;
	bosses << "{\n  \"schema\": \"lostark.boss-profiles\",\n  \"formatVersion\": 4,\n  \"bosses\": [\n";
	for (std::size_t i = 0; i < m_bosses.size(); ++i)
	{
		const BOSS_EDIT& b = m_bosses[i];
		bosses << "    {\n      \"archetypeId\": " << Quote(b.archetypeId)
			<< ",\n      \"encounterId\": " << Quote(b.encounterId)
			<< ",\n      \"displayName\": " << Quote(b.displayName)
			<< ",\n      \"maximumHp\": " << b.maximumHp
			<< ",\n      \"maximumHealthBars\": " << b.maximumHealthBars
			<< ",\n      \"attackPower\": " << b.attackPower
			<< ",\n      \"collisionRadius\": " << FormatJsonNumber(b.collisionRadius)
			<< ",\n      \"engageDistance\": " << FormatJsonNumber(b.engageDistance)
			<< ",\n      \"moveSpeed\": " << FormatJsonNumber(b.moveSpeed)
			<< ",\n      \"phasePolicy\": { \"kind\": "
			<< Quote(b.phasePolicyKind);
		if (b.phasePolicyKind == "HEALTH_PERCENT_THRESHOLD")
			bosses << ", \"thresholdPercent\": "
				<< b.phasePolicyThresholdPercent;
		bosses << " }\n    }"
			<< (i + 1u == m_bosses.size() ? "\n" : ",\n");
	}
	bosses << "  ]\n}\n";

	std::ostringstream encounter;
	encounter << "{\n  \"schema\": \"lostark.encounter-profile\",\n  \"formatVersion\": 3,"
		<< "\n  \"encounterId\": " << Quote(m_encounterId)
		<< ",\n  \"bossArchetypeId\": " << Quote(m_encounterBossArchetypeId)
		<< ",\n  \"authority\": " << Quote(m_encounterAuthority)
		<< ",\n  \"fixedTickHz\": " << m_fixedTickHz
		<< ",\n  \"introPatternId\": " << Quote(m_encounterIntroPatternId)
		<< ",\n  \"states\": [\n";
	for (std::size_t i = 0; i < m_encounterStates.size(); ++i)
	{
		const ENCOUNTER_STATE_EDIT& state = m_encounterStates[i];
		encounter << "    { \"id\": " << Quote(state.id)
			<< ", \"actionId\": " << Quote(state.actionId) << ", \"next\": "
			<< (state.hasNext ? Quote(state.next) : "null") << " }"
			<< (i + 1u == m_encounterStates.size() ? "\n" : ",\n");
	}
	encounter << "  ],\n  \"patterns\": [\n";
	for (std::size_t i = 0; i < m_patterns.size(); ++i)
	{
		const PATTERN_EDIT& p = m_patterns[i];
		encounter << "    {\n      \"patternId\": " << Quote(p.patternId)
			<< ",\n      \"displayName\": " << Quote(p.displayName)
			<< ",\n      \"actionId\": " << Quote(p.actionId)
			<< ",\n      \"sourceActionIds\": [";
		for (std::size_t sourceIndex = 0;
			sourceIndex < p.sourceActionIds.size(); ++sourceIndex)
		{
			if (sourceIndex > 0u)
				encounter << ", ";
			encounter << p.sourceActionIds[sourceIndex];
		}
		encounter << "],\n      \"selectionMode\": " << Quote(p.selectionMode)
			<< ",\n      \"minimumHealthBar\": " << p.minimumHealthBar
			<< ",\n      \"maximumHealthBar\": " << p.maximumHealthBar
			<< ",\n      \"triggerHealthBar\": " << p.triggerHealthBar
			<< ",\n      \"triggerOrder\": " << p.triggerOrder
			<< ",\n      \"armorRequirement\": "
			<< Quote(p.armorRequirement)
			<< ",\n      \"phaseRequirement\": "
			<< Quote(p.phaseRequirement)
			<< ",\n      \"invulnerableWhileRunning\": "
			<< (p.invulnerableWhileRunning ? "true" : "false")
			<< ",\n      \"selectionWeight\": " << p.selectionWeight
			<< ",\n      \"maximumConsecutiveUses\": " << p.maximumConsecutiveUses
			<< ",\n      \"minimumRange\": " << FormatJsonNumber(p.minimumRange)
			<< ",\n      \"maximumRange\": " << FormatJsonNumber(p.maximumRange);
		if (p.serverMotion.enabled)
		{
			encounter << ",\n      \"serverMotion\": {\n"
				<< "        \"kind\": " << Quote(p.serverMotion.kind)
				<< ",\n        \"anchorId\": " << Quote(p.serverMotion.anchorId)
				<< ",\n        \"landingPosition\": ["
				<< FormatJsonNumber(p.serverMotion.landingX) << ", "
				<< FormatJsonNumber(p.serverMotion.landingY) << ", "
				<< FormatJsonNumber(p.serverMotion.landingZ)
				<< "],\n        \"apexHeight\": "
				<< FormatJsonNumber(p.serverMotion.apexHeight)
				<< ",\n        \"travelStageId\": "
				<< Quote(p.serverMotion.travelStageId)
				<< ",\n        \"takeoffStartMs\": "
				<< p.serverMotion.takeoffStartMs
				<< ",\n        \"takeoffEndMs\": "
				<< p.serverMotion.takeoffEndMs
				<< ",\n        \"travelStartMs\": "
				<< p.serverMotion.travelStartMs
				<< ",\n        \"travelEndMs\": "
				<< p.serverMotion.travelEndMs
				<< "\n      }";
		}
		encounter << ",\n      \"stages\": [\n";
		for (std::size_t stageIndex = 0; stageIndex < p.stages.size(); ++stageIndex)
		{
			const PATTERN_STAGE_EDIT& stage = p.stages[stageIndex];
			encounter << "        { \"stageId\": " << Quote(stage.stageId)
				<< ", \"actionId\": " << Quote(stage.actionId)
				<< ", \"stageKind\": " << Quote(stage.stageKind)
				<< ", \"durationMs\": " << stage.durationMs
				<< ", \"hitShape\": " << Quote(stage.hitShape)
				<< ", \"hitOuterRadius\": " << FormatJsonNumber(stage.hitOuterRadius)
				<< ", \"hitInnerRadius\": " << FormatJsonNumber(stage.hitInnerRadius)
				<< ", \"hitAngleDegrees\": " << FormatJsonNumber(stage.hitAngleDegrees)
				<< ", \"hitLength\": " << FormatJsonNumber(stage.hitLength)
				<< ", \"hitHalfWidth\": " << FormatJsonNumber(stage.hitHalfWidth)
				<< ", \"hitCount\": " << stage.hitCount
				<< ", \"hitIntervalMs\": " << stage.hitIntervalMs
				<< ", \"hitDelayMs\": " << stage.hitDelayMs
				<< ", \"serverDamageProfileId\": " << Quote(stage.damageProfileId)
				<< ", \"pushRangeM\": " << FormatJsonNumber(stage.pushRangeM)
				<< ", \"pushMs\": " << stage.pushMs
				<< ", \"knockdown\": " << (stage.knockdown ? "true" : "false")
				<< ", \"downMs\": " << stage.downMs
				<< " }" << (stageIndex + 1u == p.stages.size() ? "\n" : ",\n");
		}
		encounter << "      ]\n    }"
			<< (i + 1u == m_patterns.size() ? "\n" : ",\n");
	}
	encounter << "  ]\n}\n";
	if (nullptr != readOnlyCapture)
	{
		readOnlyCapture->players = players.str();
		readOnlyCapture->skills = skills.str();
		readOnlyCapture->damage = damage.str();
		readOnlyCapture->bosses = bosses.str();
		readOnlyCapture->encounter = encounter.str();
		return true;
	}

	struct WRITE final { std::filesystem::path relative; std::string text; };
	const std::vector<WRITE> writes{
		{ L"Balance/PlayerProfiles.json", players.str() },
		{ L"Balance/PlayerSkills.json", skills.str() },
		{ L"Balance/DamageProfiles.json", damage.str() },
		{ L"Balance/BossProfiles.json", bosses.str() },
		{ L"Encounters/Valtan/ValtanEncounter.json", encounter.str() } };
	std::vector<std::filesystem::path> temporaries;
	std::vector<std::filesystem::path> backups;
	std::size_t promotedCount = 0u;
	const std::wstring suffix = L"." + std::to_wstring(GetCurrentProcessId()) +
		L"." + std::to_wstring(GetTickCount64());
	for (const WRITE& write : writes)
	{
		const std::filesystem::path destination = CProjectDataRoot::Resolve(write.relative);
		std::filesystem::path temporary = destination;
		temporary += L".balance.tmp" + suffix;
		if (destination.empty() || !DurableWrite(temporary, write.text, status) ||
			!ParseStagedJson(temporary, status))
		{
			for (const auto& path : temporaries) { std::error_code error; std::filesystem::remove(path, error); }
			m_status = "Save failed before commit: " + status;
			return false;
		}
		temporaries.push_back(temporary);
	}
	for (std::size_t i = 0; i < writes.size(); ++i)
	{
		const std::filesystem::path destination = CProjectDataRoot::Resolve(writes[i].relative);
		std::filesystem::path backup = destination;
		backup += L".balance.rollback" + suffix;
		if (!CopyFileW(destination.c_str(), backup.c_str(), TRUE))
		{
			status = "Could not create balance rollback copy.";
			break;
		}
		backups.push_back(backup);
		if (!MoveFileExW(temporaries[i].c_str(), destination.c_str(),
			MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
		{
			status = "Could not promote balance staging file.";
			break;
		}
		++promotedCount;
	}
	if (promotedCount != writes.size())
	{
		for (std::size_t i = 0; i < backups.size(); ++i)
		{
			const std::filesystem::path destination = CProjectDataRoot::Resolve(writes[i].relative);
			MoveFileExW(backups[i].c_str(), destination.c_str(),
				MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
		}
		for (const auto& path : temporaries) { std::error_code error; std::filesystem::remove(path, error); }
		m_status = "Save rolled back: " + status;
		return false;
	}
	const std::filesystem::path receipt = CProjectDataRoot::Resolve(
		L"Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json");
	std::filesystem::path receiptBackup = receipt;
	receiptBackup += L".balance.rollback" + suffix;
	if (receipt.empty() || !CopyFileW(receipt.c_str(), receiptBackup.c_str(), TRUE))
	{
		for (std::size_t i = 0; i < backups.size(); ++i)
		{
			const std::filesystem::path destination =
				CProjectDataRoot::Resolve(writes[i].relative);
			MoveFileExW(backups[i].c_str(), destination.c_str(),
				MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
		}
		m_status = "Save rolled back: could not protect the provenance receipt.";
		return false;
	}
	if (!RunPipeline(L"GameplayPipeline\\Update-BalanceProvenanceReceipt.ps1", L"", status) ||
		!RunPipeline(L"GameplayPipeline\\Publish-BalanceRuntimeSet.ps1", L"-Mode Validate", status))
	{
		for (std::size_t i = 0; i < backups.size(); ++i)
		{
			const std::filesystem::path destination =
				CProjectDataRoot::Resolve(writes[i].relative);
			MoveFileExW(backups[i].c_str(), destination.c_str(),
				MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
		}
		MoveFileExW(receiptBackup.c_str(), receipt.c_str(),
			MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
		m_status = "Save rolled back after provenance/validation failure: " + status;
		return false;
	}
	for (const auto& path : backups)
	{
		std::error_code error;
		std::filesystem::remove(path, error);
	}
	{
		std::error_code error;
		std::filesystem::remove(receiptBackup, error);
	}
	m_dirty = false;
	m_status = "Saved authoring, synchronized changed fields to PROJECT_TUNED, and validated.";
	Reload();
	return true;
}

#if defined(LOSTARK_BALANCE_TOOL_CONTRACT_TEST)
bool Client::CBalanceTool::Run_ReadOnlyRoundTripContractTest(
	std::string& status)
{
	{
		CBalanceTool safetyTool(nullptr);
		if (!safetyTool.Reload())
		{
			status = "Balance Tool safety admission failed: " +
				safetyTool.m_status;
			return false;
		}
		VALTAN_TOOL_AUDITION_INVENTORY playableInventory;
		std::string inventoryStatus;
		if (!CValtanPatternTree::Build_PlayablePatternInventory(
				safetyTool.m_valtanPatternTree, playableInventory, inventoryStatus))
		{
			status = "Balance Tool playable inventory admission failed: " +
				inventoryStatus;
			return false;
		}
		const std::size_t expectedManagedPatternCount =
			playableInventory.Get_PatternCount();
		const std::size_t managedPatternCount =
			CountManagedValtanPatterns(safetyTool.m_valtanPatternTree);
		if (expectedManagedPatternCount != managedPatternCount ||
			safetyTool.m_patterns.size() !=
				managedPatternCount + safetyTool.m_legacyPatterns.size())
		{
			status = "Balance Tool did not preserve the dynamic managed / legacy "
				"Valtan identity partition.";
			return false;
		}
		const std::filesystem::path productPath = CProjectDataRoot::Resolve(
			L"Encounters/Valtan/ValtanEncounter.json");
		std::ifstream beforeInput(productPath, std::ios::binary);
		const std::string before{
			std::istreambuf_iterator<char>(beforeInput),
			std::istreambuf_iterator<char>() };
		if (before.empty() || safetyTool.Save(nullptr))
		{
			status = "Unsafe generated Encounter disk Save was not rejected.";
			return false;
		}
		std::ifstream afterInput(productPath, std::ios::binary);
		const std::string after{
			std::istreambuf_iterator<char>(afterInput),
			std::istreambuf_iterator<char>() };
		if (before != after)
		{
			status = "Rejected Save still changed generated Encounter bytes.";
			return false;
		}
		status = "Balance Tool admitted Encounter v4, exposed " +
			std::to_string(managedPatternCount) +
			" managed / " + std::to_string(safetyTool.m_legacyPatterns.size()) +
			" legacy Valtan patterns, and rejected the lossy generated Product Save.";
		return true;
	}

	CBalanceTool tool(nullptr);
	std::string validationStatus;
	if (!tool.Reload() || !tool.ValidateDraft(validationStatus))
	{
		status = "Balance Tool could not load/validate current authoring: " +
			(tool.m_status.empty() ? validationStatus : tool.m_status);
		return false;
	}

	SERIALIZED_DRAFT_DOCUMENTS serialized;
	if (!tool.Save(&serialized))
	{
		status = "Balance Tool read-only serialization failed: " + tool.m_status;
		return false;
	}
	const auto parseRoot = [&status](const std::string& text,
		DATA_JSON_VALUE& root, const char* name)
	{
		std::string parseStatus;
		if (!CDataJson::Parse(text, root, parseStatus) || !root.Is_Object())
		{
			status = std::string(name) + " serialization is invalid JSON: " +
				parseStatus;
			return false;
		}
		return true;
	};

	DATA_JSON_VALUE playerRoot;
	DATA_JSON_VALUE skillRoot;
	DATA_JSON_VALUE damageRoot;
	DATA_JSON_VALUE bossRoot;
	DATA_JSON_VALUE encounterRoot;
	if (!parseRoot(serialized.players, playerRoot, "PlayerProfiles") ||
		!parseRoot(serialized.skills, skillRoot, "PlayerSkills") ||
		!parseRoot(serialized.damage, damageRoot, "DamageProfiles") ||
		!parseRoot(serialized.bosses, bossRoot, "BossProfiles") ||
		!parseRoot(serialized.encounter, encounterRoot, "ValtanEncounter"))
	{
		return false;
	}

	const auto semanticallyEqual = [](const auto& self,
		const DATA_JSON_VALUE& left, const DATA_JSON_VALUE& right) -> bool
	{
		if (left.Get_Type() != right.Get_Type())
			return false;
		switch (left.Get_Type())
		{
		case DATA_JSON_TYPE::NULL_VALUE:
			return true;
		case DATA_JSON_TYPE::BOOLEAN:
			return left.Get_Boolean() == right.Get_Boolean();
		case DATA_JSON_TYPE::NUMBER:
			return left.Get_Number() == right.Get_Number();
		case DATA_JSON_TYPE::STRING:
			return left.Get_String() == right.Get_String();
		case DATA_JSON_TYPE::ARRAY:
		{
			if (left.Get_Array().size() != right.Get_Array().size())
				return false;
			for (std::size_t index = 0u; index < left.Get_Array().size(); ++index)
			{
				if (!self(self, left.Get_Array()[index], right.Get_Array()[index]))
					return false;
			}
			return true;
		}
		case DATA_JSON_TYPE::OBJECT:
		{
			if (left.Get_Object().size() != right.Get_Object().size())
				return false;
			for (const auto& [key, value] : left.Get_Object())
			{
				const DATA_JSON_VALUE* other = right.Find(key);
				if (nullptr == other || !self(self, value, *other))
					return false;
			}
			return true;
		}
		default:
			return false;
		}
	};
	DATA_JSON_VALUE originalPlayerRoot;
	DATA_JSON_VALUE originalSkillRoot;
	DATA_JSON_VALUE originalDamageRoot;
	DATA_JSON_VALUE originalBossRoot;
	DATA_JSON_VALUE originalEncounterRoot;
	std::string readStatus;
	if (!ReadJson(L"Balance/PlayerProfiles.json", originalPlayerRoot, readStatus) ||
		!ReadJson(L"Balance/PlayerSkills.json", originalSkillRoot, readStatus) ||
		!ReadJson(L"Balance/DamageProfiles.json", originalDamageRoot, readStatus) ||
		!ReadJson(L"Balance/BossProfiles.json", originalBossRoot, readStatus) ||
		!ReadJson(L"Encounters/Valtan/ValtanEncounter.json",
			originalEncounterRoot, readStatus) ||
		!semanticallyEqual(semanticallyEqual, originalPlayerRoot, playerRoot) ||
		!semanticallyEqual(semanticallyEqual, originalSkillRoot, skillRoot) ||
		!semanticallyEqual(semanticallyEqual, originalDamageRoot, damageRoot) ||
		!semanticallyEqual(semanticallyEqual, originalBossRoot, bossRoot) ||
		!semanticallyEqual(semanticallyEqual, originalEncounterRoot, encounterRoot))
	{
		status = "Balance Tool serializer did not preserve all five authoring documents: " +
			readStatus;
		return false;
	}

	const DATA_JSON_VALUE* players =
		Field(playerRoot, "players", DATA_JSON_TYPE::ARRAY);
	const DATA_JSON_VALUE* skills =
		Field(skillRoot, "skills", DATA_JSON_TYPE::ARRAY);
	const DATA_JSON_VALUE* patterns =
		Field(encounterRoot, "patterns", DATA_JSON_TYPE::ARRAY);
	if (nullptr == players || players->Get_Array().size() != 6u ||
		nullptr == skills || skills->Get_Array().size() != 94u ||
		nullptr == patterns ||
		patterns->Get_Array().size() != tool.m_patterns.size())
	{
		status = "Balance Tool round-trip changed required collections or Pattern identities.";
		return false;
	}

	for (std::size_t index = 0; index < tool.m_players.size(); ++index)
	{
		const PLAYER_EDIT& expected = tool.m_players[index];
		const DATA_JSON_VALUE& actual = players->Get_Array()[index];
		double stanceScale = 0.0;
		std::uint32_t maximumIdentity = 0u;
		std::uint32_t regen = 0u;
		std::uint32_t drain = 0u;
		std::uint32_t switchCost = 0u;
		std::uint32_t cyclic = 0u;
		if (!ReadDouble(actual, "defenseStanceMoveSpeedScale", stanceScale) ||
			!ReadU32(actual, "maximumIdentity", maximumIdentity) ||
			!ReadU32(actual, "identityRegenPerSecond", regen) ||
			!ReadU32(actual, "identityDrainPerSecond", drain) ||
			!ReadU32(actual, "identityStanceSwitchCost", switchCost) ||
			!ReadU32(actual, "identityCyclic", cyclic) ||
			stanceScale != expected.defenseStanceMoveSpeedScale ||
			maximumIdentity != expected.maximumIdentity ||
			regen != expected.identityRegenPerSecond ||
			drain != expected.identityDrainPerSecond ||
			switchCost != expected.identityStanceSwitchCost ||
			cyclic != expected.identityCyclic)
		{
			status = "Balance Tool round-trip changed player identity fields.";
			return false;
		}
	}

	std::size_t activeCount = 0u;
	std::size_t comboCount = 0u;
	std::size_t holdCount = 0u;
	std::size_t counterCount = 0u;
	std::size_t standupCount = 0u;
	bool artistMoonCost = false;
	bool artistSunCost = false;
	for (std::size_t index = 0; index < tool.m_skills.size(); ++index)
	{
		const SKILL_EDIT& expected = tool.m_skills[index];
		const DATA_JSON_VALUE& actual = skills->Get_Array()[index];
		std::uint32_t skillId = 0u;
		std::uint32_t identityCost = 0u;
		const DATA_JSON_VALUE* stages =
			Field(actual, "comboStages", DATA_JSON_TYPE::ARRAY);
		if (!ReadU32(actual, "skillId", skillId) || skillId != expected.skillId ||
			!ReadU32(actual, "identityCost", identityCost) ||
			identityCost != expected.identityCost || nullptr == stages ||
			stages->Get_Array().size() != expected.comboStages.size())
		{
			status = "Balance Tool round-trip changed skill identity/stage fields.";
			return false;
		}
		for (std::size_t stageIndex = 0;
			stageIndex < expected.comboStages.size(); ++stageIndex)
		{
			std::uint32_t comboAdvanceMs = 0u;
			if (!ReadU32(stages->Get_Array()[stageIndex], "comboAdvanceMs",
				comboAdvanceMs) ||
				comboAdvanceMs != expected.comboStages[stageIndex].comboAdvanceMs)
			{
				status = "Balance Tool round-trip changed comboAdvanceMs.";
				return false;
			}
		}
		activeCount += "ACTIVE" == expected.skillKind ? 1u : 0u;
		comboCount += "COMBO" == expected.skillKind ? 1u : 0u;
		holdCount += "HOLD" == expected.skillKind ? 1u : 0u;
		counterCount += "COUNTER" == expected.skillKind ? 1u : 0u;
		standupCount += "STANDUP" == expected.skillKind ? 1u : 0u;
		artistMoonCost = artistMoonCost ||
			(31110u == expected.skillId && 33u == identityCost);
		artistSunCost = artistSunCost ||
			(31050u == expected.skillId && 66u == identityCost);
	}
	if (76u != activeCount || 11u != comboCount || 2u != holdCount ||
		1u != counterCount || 4u != standupCount ||
		!artistMoonCost || !artistSunCost)
	{
		status = "Balance Tool did not preserve all skill kinds/identity costs.";
		return false;
	}

	std::string introPatternId;
	if (!ReadString(encounterRoot, "introPatternId", introPatternId) ||
		introPatternId != tool.m_encounterIntroPatternId)
	{
		status = "Balance Tool round-trip changed introPatternId.";
		return false;
	}
	std::size_t serverMotionCount = 0u;
	for (std::size_t index = 0; index < tool.m_patterns.size(); ++index)
	{
		const PATTERN_EDIT& expected = tool.m_patterns[index];
		const DATA_JSON_VALUE* motion =
			patterns->Get_Array()[index].Find("serverMotion");
		if (expected.serverMotion.enabled)
		{
			++serverMotionCount;
			if (nullptr == motion || !motion->Is_Object())
			{
				status = "Balance Tool round-trip changed serverMotion.";
				return false;
			}
			std::string kind;
			std::string anchorId;
			std::string travelStageId;
			double apexHeight = 0.0;
			std::uint32_t takeoffStartMs = 0u;
			std::uint32_t takeoffEndMs = 0u;
			std::uint32_t travelStartMs = 0u;
			std::uint32_t travelEndMs = 0u;
			const DATA_JSON_VALUE* landing =
				Field(*motion, "landingPosition", DATA_JSON_TYPE::ARRAY);
			const bool exactLanding = nullptr != landing &&
				3u == landing->Get_Array().size() &&
				landing->Get_Array()[0].Is_Number() &&
				landing->Get_Array()[1].Is_Number() &&
				landing->Get_Array()[2].Is_Number() &&
				landing->Get_Array()[0].Get_Number() == expected.serverMotion.landingX &&
				landing->Get_Array()[1].Get_Number() == expected.serverMotion.landingY &&
				landing->Get_Array()[2].Get_Number() == expected.serverMotion.landingZ;
			if (!ReadString(*motion, "kind", kind) ||
				!ReadString(*motion, "anchorId", anchorId) ||
				!ReadString(*motion, "travelStageId", travelStageId) ||
				!ReadU32(*motion, "takeoffStartMs", takeoffStartMs) ||
				!ReadU32(*motion, "takeoffEndMs", takeoffEndMs) ||
				!ReadU32(*motion, "travelStartMs", travelStartMs) ||
				!ReadU32(*motion, "travelEndMs", travelEndMs) ||
				!ReadDouble(*motion, "apexHeight", apexHeight) ||
				!exactLanding ||
				kind != expected.serverMotion.kind ||
				anchorId != expected.serverMotion.anchorId ||
				travelStageId != expected.serverMotion.travelStageId ||
				takeoffStartMs != expected.serverMotion.takeoffStartMs ||
				takeoffEndMs != expected.serverMotion.takeoffEndMs ||
				travelStartMs != expected.serverMotion.travelStartMs ||
				travelEndMs != expected.serverMotion.travelEndMs ||
				apexHeight != expected.serverMotion.apexHeight)
			{
				status = "Balance Tool round-trip changed serverMotion.";
				return false;
			}
		}
		else if (nullptr != motion)
		{
			status = "Balance Tool invented an optional serverMotion.";
			return false;
		}
	}
	if (6u != serverMotionCount)
	{
		status = "Balance Tool did not preserve all six authored serverMotion rows.";
		return false;
	}

	std::size_t normalizedPatternIndex = tool.m_patterns.size();
	std::size_t normalizedStageIndex = 0u;
	for (std::size_t patternIndex = 0u;
		patternIndex < tool.m_patterns.size(); ++patternIndex)
	{
		for (std::size_t stageIndex = 0u;
			stageIndex < tool.m_patterns[patternIndex].stages.size(); ++stageIndex)
		{
			const PATTERN_STAGE_EDIT& candidate =
				tool.m_patterns[patternIndex].stages[stageIndex];
			if ("NONE" != candidate.hitShape && !candidate.damageProfileId.empty())
			{
				normalizedPatternIndex = patternIndex;
				normalizedStageIndex = stageIndex;
				break;
			}
		}
		if (normalizedPatternIndex != tool.m_patterns.size())
			break;
	}
	if (normalizedPatternIndex == tool.m_patterns.size())
	{
		status = "Balance Tool normalization test could not find a damage stage.";
		return false;
	}
	PATTERN_STAGE_EDIT& normalizedStage =
		tool.m_patterns[normalizedPatternIndex].stages[normalizedStageIndex];
	const PATTERN_STAGE_EDIT originalStage = normalizedStage;
	normalizedStage.hitShape = "CONE";
	normalizedStage.hitOuterRadius = 123.0;
	normalizedStage.hitInnerRadius = 45.0;
	normalizedStage.hitAngleDegrees = 73.25;
	normalizedStage.hitLength = 4.75;
	normalizedStage.hitHalfWidth = 67.0;
	NormalizePatternStageForShape(normalizedStage);
	normalizedStage.pushRangeM = 0.0;
	normalizedStage.pushMs = 999u;
	normalizedStage.knockdown = false;
	normalizedStage.downMs = 999u;
	NormalizePatternStagePush(normalizedStage);
	SERIALIZED_DRAFT_DOCUMENTS normalizedDocuments;
	if (!tool.Save(&normalizedDocuments))
	{
		status = "A publisher-compatible collider/push edit was rejected: " +
			tool.m_status;
		return false;
	}
	DATA_JSON_VALUE normalizedEncounter;
	if (!parseRoot(normalizedDocuments.encounter, normalizedEncounter,
		"Normalized ValtanEncounter"))
	{
		return false;
	}
	const DATA_JSON_VALUE* normalizedPatterns =
		Field(normalizedEncounter, "patterns", DATA_JSON_TYPE::ARRAY);
	const DATA_JSON_VALUE* normalizedStages = nullptr;
	if (nullptr != normalizedPatterns &&
		normalizedPatternIndex < normalizedPatterns->Get_Array().size())
	{
		normalizedStages = Field(
			normalizedPatterns->Get_Array()[normalizedPatternIndex],
			"stages", DATA_JSON_TYPE::ARRAY);
	}
	double outer = -1.0;
	double inner = -1.0;
	double angle = -1.0;
	double length = -1.0;
	double halfWidth = -1.0;
	double pushRange = -1.0;
	std::uint32_t hitDelayMs =
		(std::numeric_limits<std::uint32_t>::max)();
	std::uint32_t pushMs = 1u;
	std::uint32_t downMs = 1u;
	const DATA_JSON_VALUE* normalizedStageValue =
		nullptr != normalizedStages &&
		normalizedStageIndex < normalizedStages->Get_Array().size() ?
		&normalizedStages->Get_Array()[normalizedStageIndex] : nullptr;
	const DATA_JSON_VALUE* knockdownValue = nullptr == normalizedStageValue ?
		nullptr : normalizedStageValue->Find("knockdown");
	const bool normalizedRoundTrip = nullptr != normalizedStageValue &&
		ReadDouble(*normalizedStageValue, "hitOuterRadius", outer) &&
		ReadDouble(*normalizedStageValue, "hitInnerRadius", inner) &&
		ReadDouble(*normalizedStageValue, "hitAngleDegrees", angle) &&
		ReadDouble(*normalizedStageValue, "hitLength", length) &&
		ReadDouble(*normalizedStageValue, "hitHalfWidth", halfWidth) &&
		ReadU32(*normalizedStageValue, "hitDelayMs", hitDelayMs) &&
		ReadDouble(*normalizedStageValue, "pushRangeM", pushRange) &&
		ReadU32(*normalizedStageValue, "pushMs", pushMs) &&
		ReadU32(*normalizedStageValue, "downMs", downMs) &&
		nullptr != knockdownValue && knockdownValue->Is_Boolean() &&
		0.0 == outer && 0.0 == inner && 73.25 == angle &&
		4.75 == length && 0.0 == halfWidth && 0.0 == pushRange &&
		hitDelayMs == originalStage.hitDelayMs &&
		0u == pushMs && !knockdownValue->Get_Boolean() && 0u == downMs;
	PATTERN_STAGE_EDIT noneStage = originalStage;
	noneStage.hitShape = "NONE";
	noneStage.hitOuterRadius = 1.0;
	noneStage.hitInnerRadius = 1.0;
	noneStage.hitAngleDegrees = 1.0;
	noneStage.hitLength = 1.0;
	noneStage.hitHalfWidth = 1.0;
	noneStage.hitDelayMs = 1u;
	noneStage.pushRangeM = 1.0;
	noneStage.pushMs = 1u;
	noneStage.knockdown = true;
	noneStage.downMs = 1u;
	NormalizePatternStageForShape(noneStage);
	const bool noneWasCleared = 0.0 == noneStage.hitOuterRadius &&
		0.0 == noneStage.hitInnerRadius && 0.0 == noneStage.hitAngleDegrees &&
		0.0 == noneStage.hitLength && 0.0 == noneStage.hitHalfWidth &&
		0u == noneStage.hitCount && 0u == noneStage.hitIntervalMs &&
		0u == noneStage.hitDelayMs && noneStage.damageProfileId.empty() &&
		0.0 == noneStage.pushRangeM &&
		0u == noneStage.pushMs && !noneStage.knockdown && 0u == noneStage.downMs;
	PATTERN_STAGE_EDIT pairedPush = originalStage;
	pairedPush.pushRangeM = 1.0;
	pairedPush.pushMs = 0u;
	pairedPush.knockdown = true;
	pairedPush.downMs = 0u;
	NormalizePatternStagePush(pairedPush);
	const bool pushWasPaired = 1u == pairedPush.pushMs && 1u == pairedPush.downMs;
	PATTERN_STAGE_EDIT hiddenGeometry = originalStage;
	hiddenGeometry.hitShape = "CIRCLE";
	hiddenGeometry.hitOuterRadius = 2.0;
	hiddenGeometry.hitInnerRadius = 0.0;
	hiddenGeometry.hitAngleDegrees = 45.0;
	hiddenGeometry.hitLength = 0.0;
	hiddenGeometry.hitHalfWidth = 0.0;
	normalizedStage = hiddenGeometry;
	const bool hiddenGeometryRejected = !tool.ValidateDraft(validationStatus);
	PATTERN_STAGE_EDIT unpairedPush = originalStage;
	unpairedPush.pushRangeM = 0.0;
	unpairedPush.pushMs = 50u;
	normalizedStage = unpairedPush;
	const bool unpairedPushRejected = !tool.ValidateDraft(validationStatus);
	normalizedStage = originalStage;
	if (!normalizedRoundTrip || !noneWasCleared || !pushWasPaired ||
		!hiddenGeometryRejected || !unpairedPushRejected)
	{
		status = "Balance Tool collider/push normalization did not round-trip exactly.";
		return false;
	}

	auto dimensionMaster = std::find_if(tool.m_skills.begin(), tool.m_skills.end(),
		[](const SKILL_EDIT& skill) { return 2050010u == skill.skillId; });
	if (tool.m_skills.end() == dimensionMaster ||
		dimensionMaster->comboStages.empty())
	{
		status = "DimensionMaster BA was absent from the Balance Tool draft.";
		return false;
	}
	dimensionMaster->comboStages.front().actionDurationMs = 1300u;
	dimensionMaster->comboStages.front().comboAdvanceMs = 1300u;
	SERIALIZED_DRAFT_DOCUMENTS edited;
	if (!tool.Save(&edited))
	{
		status = "A valid automatic combo timing edit was rejected.";
		return false;
	}
	DATA_JSON_VALUE editedSkillRoot;
	if (!parseRoot(edited.skills, editedSkillRoot, "Edited PlayerSkills"))
		return false;
	const DATA_JSON_VALUE* editedSkills =
		Field(editedSkillRoot, "skills", DATA_JSON_TYPE::ARRAY);
	bool foundEditedAdvance = false;
	if (nullptr != editedSkills)
	{
		for (const DATA_JSON_VALUE& value : editedSkills->Get_Array())
		{
			std::uint32_t skillId = 0u;
			if (!ReadU32(value, "skillId", skillId) || 2050010u != skillId)
				continue;
			const DATA_JSON_VALUE* stages =
				Field(value, "comboStages", DATA_JSON_TYPE::ARRAY);
			std::uint32_t duration = 0u;
			std::uint32_t advance = 0u;
			foundEditedAdvance = nullptr != stages && !stages->Get_Array().empty() &&
				ReadU32(stages->Get_Array().front(), "actionDurationMs", duration) &&
				ReadU32(stages->Get_Array().front(), "comboAdvanceMs", advance) &&
				1300u == duration && 1300u == advance;
			break;
		}
	}
	if (!foundEditedAdvance)
	{
		status = "A valid automatic combo timing edit was not serialized.";
		return false;
	}
	dimensionMaster->comboStages.front().comboAdvanceMs =
		dimensionMaster->comboStages.front().hitTimeMs - 1u;
	if (tool.ValidateDraft(validationStatus))
	{
		status = "An invalid comboAdvanceMs edit was accepted.";
		return false;
	}

	status = "Balance Tool read-only round-trip preserved " +
		std::to_string(tool.m_patterns.size()) +
		" loaded Valtan patterns plus manual auditions, all balance domains, "
		"identityCost, all skill kinds, comboAdvanceMs, introPatternId, and serverMotion.";
	return true;
}
#else
bool Client::CBalanceTool::Run_ReadOnlyRoundTripContractTest(
	std::string& status)
{
	status = "Balance Tool read-only contract test is harness-only.";
	return false;
}
#endif

#if !defined(LOSTARK_BALANCE_TOOL_CONTRACT_TEST)
void Client::CBalanceTool::Render()
{
	if (!m_open)
		return;
	if (m_focusPending)
	{
		ImGui::SetNextWindowCollapsed(false, ImGuiCond_Always);
		ImGui::SetNextWindowFocus();
		m_focusPending = false;
	}
	ImGui::SetNextWindowSize(ImVec2(1180.f, 760.f), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("LostArk Balance Tool", &m_open))
	{
		ImGui::End();
		return;
	}
	if (ImGui::Button("Players")) m_showPlayers = true;
	ImGui::SameLine();
	if (ImGui::Button("Bosses")) m_showPlayers = false;
	ImGui::SameLine();
	ImGui::TextDisabled("skill L10 / fixed L1 | %s", m_dirty ? "UNSAVED" : "saved");
	ImGui::SameLine();
	if (ImGui::Button("Reload"))
	{
		if (m_dirty)
			m_reloadConfirmationOpen = true;
		else
			Reload();
	}
	if (m_reloadConfirmationOpen)
	{
		ImGui::OpenPopup("Discard unsaved Balance draft?");
		m_reloadConfirmationOpen = false;
	}
	if (ImGui::BeginPopupModal(
			"Discard unsaved Balance draft?", nullptr,
			ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::TextWrapped(
			"Reload will discard the current in-memory draft. Source files have not been changed.");
		if (ImGui::Button("Discard and Reload"))
		{
			Reload();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
	ImGui::SameLine();
	const bool valtanView = !m_showPlayers && !m_bosses.empty() &&
		m_bosses[m_selectedBoss].archetypeId == "BOSS_VALTAN";
	if (valtanView)
	{
		const bool splitJoinValidated =
			VALTAN_SOURCE_JOIN_STATE::JOINED_VALIDATED ==
				m_valtanSourceJoin.state;
		ImGui::BeginDisabled(
			m_valtanSourceRevision.empty() || !splitJoinValidated);
		if (ImGui::Button("Save"))
		{
			std::string status;
			(void)Save_ValtanProduct(status);
			m_status = std::move(status);
		}
		ImGui::EndDisabled();
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		{
			ImGui::SetTooltip(
				"One action: validate joined stable IDs, save authoring, build Product data, and request a fail-closed live update when supported.");
		}
		const CNetworkManager::GAMEPLAY_REVISION_CLIENT_STATE& revisionState =
			CNetworkManager::Get().Get_GameplayRevisionState();
		const bool candidateIsHotReload =
			m_valtanCandidateApplyClass == "HOT_RELOAD";
		const ImVec4 applyClassColor = m_valtanCandidateApplyClass.empty() ?
			ImVec4(0.55f, 0.55f, 0.55f, 1.f) :
			(candidateIsHotReload ? ImVec4(0.35f, 0.85f, 0.45f, 1.f) :
				ImVec4(1.f, 0.70f, 0.20f, 1.f));
		ImGui::TextColored(applyClassColor, "Runtime activation: %s",
			m_valtanCandidateApplyClass.empty() ? "NONE" :
				m_valtanCandidateApplyClass.c_str());
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip(candidateIsHotReload ?
				"Save submits the current tick-boundary Server/Client transaction when a Debug Server is connected." :
				"This saved Product revision becomes active only after the required encounter reset or Server restart.");
		}
		ImGui::TextDisabled("Source %s | Authoring %s | Product %s | Draft %s",
			m_valtanSourceRevision.empty() ? "NONE" :
				m_valtanSourceRevision.substr(0u, 12u).c_str(),
			m_valtanAuthoringRevision.empty() ? "NONE" :
				m_valtanAuthoringRevision.substr(0u, 12u).c_str(),
			m_valtanCandidateRevision.empty() ? "NONE" :
				m_valtanCandidateRevision.substr(0u, 12u).c_str(),
			m_valtanDraftValidated ? "VALIDATED" : "UNVALIDATED");
		const char* sourceJoinState =
			VALTAN_SOURCE_JOIN_STATE::JOINED_VALIDATED ==
				m_valtanSourceJoin.state ? "JOINED / VALIDATED" :
			VALTAN_SOURCE_JOIN_STATE::SPLIT_SOURCE_UNVERIFIED ==
				m_valtanSourceJoin.state ? "UNVERIFIED" : "INCOMPLETE";
		ImGui::TextDisabled(
			"Gameplay %s | Presentation %s | Joined %s | Split join %s",
			m_valtanSourceJoin.gameplayRevision.empty() ? "NONE" :
				m_valtanSourceJoin.gameplayRevision.substr(0u, 12u).c_str(),
			m_valtanSourceJoin.presentationRevision.empty() ? "NONE" :
				m_valtanSourceJoin.presentationRevision.substr(0u, 12u).c_str(),
			m_valtanSourceJoin.joinedRevision.empty() ? "NONE" :
				m_valtanSourceJoin.joinedRevision.substr(0u, 12u).c_str(),
			sourceJoinState);
		const std::string activeRevision =
			LostArk::Shared::Format_GameplayDataRevision(
				revisionState.ServerActiveRevision);
		ImGui::TextDisabled("Runtime active %s | prepare %s | result %s",
			activeRevision.empty() ? "NONE" :
				activeRevision.substr(0u, 12u).c_str(),
			revisionState.hasLatestPrepare ?
				(LostArk::Shared::DATA_REVISION_PREPARE_STATUS::NACK ==
					revisionState.eLatestPrepareResponse ? "NACK" : "READY") :
				"NONE",
			revisionState.hasLatestResult ?
				(LostArk::Shared::DATA_REVISION_RESULT::COMMITTED ==
					revisionState.eLatestResult ? "COMMITTED" : "ABORTED") :
				"NONE");
		if (!revisionState.strLatestTransactionReason.empty())
		{
			ImGui::TextWrapped("Revision coordinator: %s",
				revisionState.strLatestTransactionReason.c_str());
		}
	}
	else
	{
		ImGui::BeginDisabled(true);
		if (ImGui::Button("Save + Validate (Valtan transaction only)")) Save();
		ImGui::EndDisabled();
		ImGui::SameLine();
		if (ImGui::Button("Validate"))
		{
			std::string status;
			m_status = RunPipeline(L"GameplayPipeline\\Publish-BalanceRuntimeSet.ps1",
				L"-Mode Validate", status) ?
				"Validation succeeded." : status;
		}
		ImGui::SameLine();
		ImGui::BeginDisabled(m_dirty);
		if (ImGui::Button("Publish Server Data"))
		{
			std::string status;
			m_status = RunPipeline(L"GameplayPipeline\\Publish-BalanceRuntimeSet.ps1",
				L"-Mode Publish", status) ?
				"Published. Restart Server.exe to apply." : status;
		}
		ImGui::EndDisabled();
	}
	ImGui::TextWrapped("%s", m_status.c_str());

	const float listWidth = 210.f;
	const float liveWidth = 300.f;
	ImGui::BeginChild("##BalanceTargets", ImVec2(listWidth, 0.f), true);
	ImGui::SeparatorText(m_showPlayers ? "Characters" : "Bosses");
	if (m_showPlayers)
	{
		for (std::size_t i = 0; i < m_players.size(); ++i)
			if (ImGui::Selectable(m_players[i].characterClass.c_str(), i == m_selectedPlayer))
				m_selectedPlayer = i;
	}
	else
	{
		for (std::size_t i = 0; i < m_bosses.size(); ++i)
			if (ImGui::Selectable(m_bosses[i].displayName.c_str(), i == m_selectedBoss))
				m_selectedBoss = i;
	}
	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::BeginChild("##BalanceEditor", ImVec2(-liveWidth - 8.f, 0.f), true);
	if (m_showPlayers) RenderPlayerEditor(); else RenderBossEditor();
	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::BeginChild("##BalanceLive", ImVec2(0.f, 0.f), true);
	RenderLiveVerification();
	ImGui::EndChild();
	ImGui::End();
}
#endif
