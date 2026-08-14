#include "ValtanCinematicCameraDocument.h"

#include "DataJson.h"
#include "EncounterPatternReference.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <initializer_list>
#include <sstream>
#include <unordered_set>

namespace
{
	using namespace Client;

	constexpr const char_t* SCHEMA = "lostark.encounter-cinematic-camera";
	constexpr uint32_t FORMAT_VERSION = 2u;
	constexpr f32_t MAX_SHAKE_AMPLITUDE = 2.f;
	constexpr uint32_t MAX_SHAKE_DURATION_MS = 1000u;
	constexpr uint32_t MAX_CUE_COUNT = 32u;
	constexpr uint32_t MAX_KEYFRAME_COUNT = 64u;
	constexpr f32_t MAX_WORLD_COORDINATE = 100000.f;

	bool_t Is_ExactObject(
		const DATA_JSON_VALUE& value,
		const std::initializer_list<const char_t*> keys)
	{
		if (!value.Is_Object() || value.Get_Object().size() != keys.size())
			return false;
		return std::all_of(keys.begin(), keys.end(),
			[&value](const char_t* key) { return nullptr != value.Find(key); });
	}

	bool_t Read_String(
		const DATA_JSON_VALUE& parent,
		const char_t* key,
		std::string& outValue)
	{
		const DATA_JSON_VALUE* value = parent.Find(key);
		if (nullptr == value || !value->Is_String() || value->Get_String().empty())
			return false;
		outValue = value->Get_String();
		return true;
	}

	bool_t Read_Unsigned(
		const DATA_JSON_VALUE& parent,
		const char_t* key,
		const uint32_t maximum,
		uint32_t& outValue)
	{
		const DATA_JSON_VALUE* value = parent.Find(key);
		if (nullptr == value || !value->Is_Number())
			return false;
		const double number = value->Get_Number();
		if (!std::isfinite(number) || number < 0.0 ||
			number > static_cast<double>(maximum) || std::floor(number) != number)
		{
			return false;
		}
		outValue = static_cast<uint32_t>(number);
		return true;
	}

	bool_t Read_Easing(
		const DATA_JSON_VALUE& parent,
		VALTAN_CINEMATIC_CAMERA_EASING& outValue)
	{
		std::string text;
		if (!Read_String(parent, "easing", text))
			return false;
		if ("LINEAR" == text)
			outValue = VALTAN_CINEMATIC_CAMERA_EASING::LINEAR;
		else if ("SMOOTHSTEP" == text)
			outValue = VALTAN_CINEMATIC_CAMERA_EASING::SMOOTHSTEP;
		else if ("HOLD" == text)
			outValue = VALTAN_CINEMATIC_CAMERA_EASING::HOLD;
		else
			return false;
		return true;
	}

	/* Amplitude and duration are read together because a shake with only one
	   of them authored is an authoring mistake, not a silent no-op. */
	bool_t Read_Shake(
		const DATA_JSON_VALUE& parent,
		VALTAN_CINEMATIC_CAMERA_CUE& outCue)
	{
		const DATA_JSON_VALUE* amplitude = parent.Find("shakeAmplitude");
		if (nullptr == amplitude || !amplitude->Is_Number())
			return false;
		const double value = amplitude->Get_Number();
		if (!std::isfinite(value) || value < 0.0 ||
			value > static_cast<double>(MAX_SHAKE_AMPLITUDE))
		{
			return false;
		}
		outCue.fShakeAmplitude = static_cast<f32_t>(value);
		if (!Read_Unsigned(parent, "shakeDurationMs", MAX_SHAKE_DURATION_MS,
			outCue.iShakeDurationMs))
		{
			return false;
		}
		const bool_t hasAmplitude = outCue.fShakeAmplitude > 0.f;
		const bool_t hasDuration = 0u != outCue.iShakeDurationMs;
		return hasAmplitude == hasDuration &&
			outCue.iShakeDurationMs <= outCue.iDurationMs;
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
		f32_t components[3]{};
		for (size_t index = 0u; index < 3u; ++index)
		{
			const DATA_JSON_VALUE& component = value->Get_Array()[index];
			if (!component.Is_Number() || !std::isfinite(component.Get_Number()) ||
				std::abs(component.Get_Number()) > MAX_WORLD_COORDINATE)
			{
				return false;
			}
			components[index] = static_cast<f32_t>(component.Get_Number());
		}
		outValue = float3_t(components[0], components[1], components[2]);
		return true;
	}

	bool_t Read_Fov(
		const DATA_JSON_VALUE& parent,
		f32_t& outValue)
	{
		const DATA_JSON_VALUE* value = parent.Find("fovYDegrees");
		if (nullptr == value || !value->Is_Number() ||
			!std::isfinite(value->Get_Number()) ||
			value->Get_Number() < 10.0 || value->Get_Number() > 120.0)
		{
			return false;
		}
		outValue = static_cast<f32_t>(value->Get_Number());
		return true;
	}
}

bool_t Client::CValtanCinematicCameraDocument::Load(
	const std::filesystem::path& path,
	const CEncounterPatternReference& encounter,
	std::string& outStatus)
{
	std::ifstream input(path, std::ios::binary);
	if (path.empty() || !input.is_open())
	{
		outStatus = "Cinematic camera document is unreadable: " + path.string();
		return false;
	}
	std::ostringstream buffer;
	buffer << input.rdbuf();
	if (input.bad())
	{
		outStatus = "Cinematic camera document read failed: " + path.string();
		return false;
	}
	return Parse_Text(buffer.str(), encounter, *this, outStatus);
}

bool_t Client::CValtanCinematicCameraDocument::Parse_Text(
	const std::string_view text,
	const CEncounterPatternReference& encounter,
	CValtanCinematicCameraDocument& outDocument,
	std::string& outStatus)
{
	if (!encounter.Is_Ready())
	{
		outStatus = "Cinematic camera requires a validated encounter reference";
		return false;
	}
	DATA_JSON_VALUE root;
	std::string parseError;
	DATA_JSON_PARSE_LIMITS limits{};
	limits.iMaximumBytes = 512u * 1024u;
	limits.iMaximumDepth = 16u;
	limits.iMaximumValues = 4096u;
	if (!CDataJson::Parse(text, root, parseError, limits))
	{
		outStatus = "Cinematic camera parse failed: " + parseError;
		return false;
	}
	if (!Is_ExactObject(root,
		{ "schema", "formatVersion", "encounterId", "provenance", "cues" }))
	{
		outStatus = "Cinematic camera root has unexpected properties";
		return false;
	}

	std::string schema;
	std::string encounterId;
	std::string provenance;
	uint32_t formatVersion = 0u;
	if (!Read_String(root, "schema", schema) || SCHEMA != schema ||
		!Read_Unsigned(root, "formatVersion", FORMAT_VERSION, formatVersion) ||
		FORMAT_VERSION != formatVersion ||
		!Read_String(root, "encounterId", encounterId) ||
		encounterId != encounter.Get_EncounterId() ||
		!Read_String(root, "provenance", provenance) ||
		"PROJECT_AUTHORED" != provenance)
	{
		outStatus = "Cinematic camera header is invalid";
		return false;
	}

	const DATA_JSON_VALUE* cues = root.Find("cues");
	if (nullptr == cues || !cues->Is_Array() || cues->Get_Array().empty() ||
		cues->Get_Array().size() > MAX_CUE_COUNT)
	{
		outStatus = "Cinematic camera cue array is invalid";
		return false;
	}

	std::vector<VALTAN_CINEMATIC_CAMERA_CUE> staged;
	std::unordered_set<std::string> cueIds;
	std::unordered_set<std::string> tuples;
	for (const DATA_JSON_VALUE& cueValue : cues->Get_Array())
	{
		if (!Is_ExactObject(cueValue,
			{ "cueId", "patternId", "stageId", "durationMs", "easing",
				"shakeAmplitude", "shakeDurationMs", "keyframes" }))
		{
			outStatus = "Cinematic camera cue has unexpected properties";
			return false;
		}
		VALTAN_CINEMATIC_CAMERA_CUE cue;
		if (!Read_String(cueValue, "cueId", cue.strCueId) ||
			!Read_String(cueValue, "patternId", cue.strPatternId) ||
			!Read_String(cueValue, "stageId", cue.strStageId) ||
			!Read_Unsigned(cueValue, "durationMs",
				CEncounterPatternReference::MAX_STAGE_DURATION_MS, cue.iDurationMs) ||
			0u == cue.iDurationMs || !cueIds.insert(cue.strCueId).second ||
			!Read_Easing(cueValue, cue.eEasing) ||
			!Read_Shake(cueValue, cue))
		{
			outStatus = "Cinematic camera cue identity is invalid";
			return false;
		}

		const ENCOUNTER_PATTERN_REFERENCE* pattern =
			encounter.Find_Pattern(cue.strPatternId);
		if (nullptr == pattern)
		{
			outStatus = "Cinematic camera pattern is unknown: " + cue.strPatternId;
			return false;
		}
		const auto stage = std::find_if(pattern->stages.begin(), pattern->stages.end(),
			[&cue](const ENCOUNTER_STAGE_REFERENCE& value)
			{ return value.stageId == cue.strStageId; });
		if (pattern->stages.end() == stage || cue.iDurationMs > stage->iDurationMs)
		{
			outStatus = "Cinematic camera stage is invalid: " + cue.strCueId;
			return false;
		}
		cue.iStageIndex = static_cast<uint32_t>(stage - pattern->stages.begin());
		cue.strStageActionId = stage->actionId;
		const std::string tuple = cue.strPatternId + "\n" +
			std::to_string(cue.iStageIndex) + "\n" + cue.strStageActionId;
		if (!tuples.insert(tuple).second)
		{
			outStatus = "Duplicate cinematic camera encounter tuple";
			return false;
		}

		const DATA_JSON_VALUE* keyframes = cueValue.Find("keyframes");
		if (nullptr == keyframes || !keyframes->Is_Array() ||
			keyframes->Get_Array().size() < 2u ||
			keyframes->Get_Array().size() > MAX_KEYFRAME_COUNT)
		{
			outStatus = "Cinematic camera keyframe array is invalid: " + cue.strCueId;
			return false;
		}
		uint32_t previousTime = 0u;
		for (size_t index = 0u; index < keyframes->Get_Array().size(); ++index)
		{
			const DATA_JSON_VALUE& keyframeValue = keyframes->Get_Array()[index];
			if (!Is_ExactObject(keyframeValue,
				{ "timeMs", "eye", "lookAt", "fovYDegrees" }))
			{
				outStatus = "Cinematic camera keyframe has unexpected properties";
				return false;
			}
			VALTAN_CINEMATIC_CAMERA_KEYFRAME keyframe;
			if (!Read_Unsigned(keyframeValue, "timeMs", cue.iDurationMs,
					keyframe.iTimeMs) ||
				!Read_Float3(keyframeValue, "eye", keyframe.vEye) ||
				!Read_Float3(keyframeValue, "lookAt", keyframe.vLookAt) ||
				!Read_Fov(keyframeValue, keyframe.fFovYDegrees) ||
				(index == 0u && 0u != keyframe.iTimeMs) ||
				(index > 0u && keyframe.iTimeMs <= previousTime))
			{
				outStatus = "Cinematic camera keyframe is invalid: " + cue.strCueId;
				return false;
			}
			const vector_t eye = XMLoadFloat3(&keyframe.vEye);
			const vector_t lookAt = XMLoadFloat3(&keyframe.vLookAt);
			if (XMVectorGetX(XMVector3LengthSq(lookAt - eye)) <= 0.000001f)
			{
				outStatus = "Cinematic camera eye and lookAt must differ";
				return false;
			}
			previousTime = keyframe.iTimeMs;
			cue.Keyframes.push_back(keyframe);
		}
		if (previousTime != cue.iDurationMs)
		{
			outStatus = "Cinematic camera final keyframe must match cue duration";
			return false;
		}
		staged.push_back(std::move(cue));
	}

	CValtanCinematicCameraDocument committed;
	committed.m_strEncounterId = std::move(encounterId);
	committed.m_Cues = std::move(staged);
	committed.m_isReady = true;
	outDocument = std::move(committed);
	outStatus = "Loaded Valtan cinematic camera cues: " +
		std::to_string(outDocument.m_Cues.size());
	return true;
}

void Client::CValtanCinematicCameraDocument::Clear()
{
	m_strEncounterId.clear();
	m_Cues.clear();
	m_isReady = false;
}

const Client::VALTAN_CINEMATIC_CAMERA_CUE*
Client::CValtanCinematicCameraDocument::Find_Cue(
	const std::string_view patternId,
	const uint32_t stageIndex,
	const std::string_view stageActionId) const
{
	const auto iter = std::find_if(m_Cues.begin(), m_Cues.end(),
		[patternId, stageIndex, stageActionId](
			const VALTAN_CINEMATIC_CAMERA_CUE& cue)
		{
			return cue.strPatternId == patternId &&
				cue.iStageIndex == stageIndex &&
				cue.strStageActionId == stageActionId;
		});
	return m_Cues.end() == iter ? nullptr : &(*iter);
}
