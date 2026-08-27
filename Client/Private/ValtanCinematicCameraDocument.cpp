#include "ValtanCinematicCameraDocument.h"

#include "DataJson.h"
#include "EncounterPatternReference.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <initializer_list>
#include <limits>
#include <sstream>
#include <unordered_set>

namespace
{
	using namespace Client;

	constexpr const char_t* SCHEMA = "lostark.encounter-cinematic-camera";
	constexpr uint32_t FORMAT_VERSION = 6u;
	constexpr f32_t MAX_SHAKE_AMPLITUDE = 2.f;
	constexpr uint32_t MAX_SHAKE_DURATION_MS = 1000u;
	constexpr uint32_t MAX_CUE_COUNT = 32u;
	constexpr uint32_t MAX_KEYFRAME_COUNT = 64u;
	constexpr f32_t MAX_WORLD_COORDINATE = 100000.f;
	constexpr size_t MAX_SKY_ASSET_ID_LENGTH = 128u;

	bool_t Is_StablePresentationAssetId(const std::string_view value)
	{
		return !value.empty() && value.size() <= MAX_SKY_ASSET_ID_LENGTH &&
			std::all_of(value.begin(), value.end(), [](const char_t character)
			{
				return (character >= 'a' && character <= 'z') ||
					(character >= 'A' && character <= 'Z') ||
					(character >= '0' && character <= '9') ||
					'_' == character || '-' == character || '.' == character;
			});
	}

	bool_t Is_ExactObject(
		const DATA_JSON_VALUE& value,
		const std::initializer_list<const char_t*> keys)
	{
		if (!value.Is_Object() || value.Get_Object().size() != keys.size())
			return false;
		return std::all_of(keys.begin(), keys.end(),
			[&value](const char_t* key) { return nullptr != value.Find(key); });
	}

	bool_t Is_ExactObjectWithOptionalTracking(
		const DATA_JSON_VALUE& value,
		const std::initializer_list<const char_t*> keys)
	{
		if (!value.Is_Object())
			return false;
		const bool_t hasMode = nullptr != value.Find("trackingMode");
		const bool_t hasOrigin = nullptr != value.Find("trackingOrigin");
		if (hasMode != hasOrigin ||
			value.Get_Object().size() != keys.size() + (hasMode ? 2u : 0u))
		{
			return false;
		}
		return std::all_of(keys.begin(), keys.end(),
			[&value](const char_t* key) { return nullptr != value.Find(key); });
	}

	bool_t Is_ExactCameraCueObject(
		const DATA_JSON_VALUE& value,
		const std::initializer_list<const char_t*> keys)
	{
		if (!value.Is_Object())
			return false;
		const bool_t hasMode = nullptr != value.Find("trackingMode");
		const bool_t hasOrigin = nullptr != value.Find("trackingOrigin");
		const bool_t hasTransitionIn = nullptr != value.Find("transitionInMs");
		const bool_t hasTransitionOut = nullptr != value.Find("transitionOutMs");
		if (hasMode != hasOrigin || value.Get_Object().size() !=
			keys.size() + (hasMode ? 2u : 0u) +
			(hasTransitionIn ? 1u : 0u) + (hasTransitionOut ? 1u : 0u))
		{
			return false;
		}
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
		if (nullptr == value || !value->Is_Number() ||
			value->Was_FloatingPointToken())
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

	bool_t Read_Interpolation(
		const DATA_JSON_VALUE& parent,
		VALTAN_CINEMATIC_CAMERA_INTERPOLATION& outValue)
	{
		std::string text;
		if (!Read_String(parent, "interpolation", text))
			return false;
		if ("LINEAR" == text)
			outValue = VALTAN_CINEMATIC_CAMERA_INTERPOLATION::LINEAR;
		else if ("CATMULL_ROM" == text)
			outValue = VALTAN_CINEMATIC_CAMERA_INTERPOLATION::CATMULL_ROM;
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

	bool_t Read_Transitions(
		const DATA_JSON_VALUE& parent,
		VALTAN_CINEMATIC_CAMERA_CUE& outCue)
	{
		if (nullptr == parent.Find("transitionInMs"))
			outCue.iTransitionInMs = 0u;
		else if (!Read_Unsigned(parent, "transitionInMs",
			VALTAN_CINEMATIC_CAMERA_CUE::MAX_TRANSITION_IN_MS,
			outCue.iTransitionInMs) ||
			outCue.iTransitionInMs > outCue.iDurationMs)
		{
			return false;
		}

		if (nullptr == parent.Find("transitionOutMs"))
			outCue.iTransitionOutMs = 0u;
		else if (!Read_Unsigned(parent, "transitionOutMs",
			VALTAN_CINEMATIC_CAMERA_CUE::MAX_TRANSITION_IN_MS,
			outCue.iTransitionOutMs))
		{
			return false;
		}
		return true;
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

	bool_t Read_Tracking(
		const DATA_JSON_VALUE& parent,
		VALTAN_CINEMATIC_TRACKING_MODE& outMode,
		float3_t& outOrigin)
	{
		const DATA_JSON_VALUE* mode = parent.Find("trackingMode");
		const DATA_JSON_VALUE* origin = parent.Find("trackingOrigin");
		if (nullptr == mode && nullptr == origin)
		{
			outMode = VALTAN_CINEMATIC_TRACKING_MODE::WORLD;
			outOrigin = {};
			return true;
		}
		std::string text;
		if (nullptr == mode || nullptr == origin ||
			!Read_String(parent, "trackingMode", text) ||
			!Read_Float3(parent, "trackingOrigin", outOrigin))
		{
			return false;
		}
		if ("BOSS_XZ" == text)
			outMode = VALTAN_CINEMATIC_TRACKING_MODE::BOSS_XZ;
		else if ("BOSS_FACING" == text)
			outMode = VALTAN_CINEMATIC_TRACKING_MODE::BOSS_FACING;
		else if ("PLAYER_BOSS_FRAME" == text)
			outMode = VALTAN_CINEMATIC_TRACKING_MODE::PLAYER_BOSS_FRAME;
		else
			return false;
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

	const char_t* Easing_Text(
		const VALTAN_CINEMATIC_CAMERA_EASING easing)
	{
		switch (easing)
		{
		case VALTAN_CINEMATIC_CAMERA_EASING::LINEAR: return "LINEAR";
		case VALTAN_CINEMATIC_CAMERA_EASING::SMOOTHSTEP: return "SMOOTHSTEP";
		case VALTAN_CINEMATIC_CAMERA_EASING::HOLD: return "HOLD";
		default: return nullptr;
		}
	}

	const char_t* Interpolation_Text(
		const VALTAN_CINEMATIC_CAMERA_INTERPOLATION interpolation)
	{
		switch (interpolation)
		{
		case VALTAN_CINEMATIC_CAMERA_INTERPOLATION::LINEAR:
			return "LINEAR";
		case VALTAN_CINEMATIC_CAMERA_INTERPOLATION::CATMULL_ROM:
			return "CATMULL_ROM";
		default:
			return nullptr;
		}
	}

	void Write_Float3(
		std::ostringstream& output,
		const float3_t& value)
	{
		output << '[' << value.x << ", " << value.y << ", " << value.z << ']';
	}

	bool_t Write_Tracking(
		std::ostringstream& output,
		const VALTAN_CINEMATIC_TRACKING_MODE mode,
		const float3_t& origin)
	{
		if (VALTAN_CINEMATIC_TRACKING_MODE::WORLD == mode)
			return true;
		const char_t* modeText = nullptr;
		switch (mode)
		{
		case VALTAN_CINEMATIC_TRACKING_MODE::BOSS_XZ:
			modeText = "BOSS_XZ";
			break;
		case VALTAN_CINEMATIC_TRACKING_MODE::BOSS_FACING:
			modeText = "BOSS_FACING";
			break;
		case VALTAN_CINEMATIC_TRACKING_MODE::PLAYER_BOSS_FRAME:
			modeText = "PLAYER_BOSS_FRAME";
			break;
		default:
			return false;
		}
		output << ",\n      \"trackingMode\": \"" << modeText << "\",\n"
			"      \"trackingOrigin\": ";
		Write_Float3(output, origin);
		return true;
	}

	void Write_Keyframes(
		std::ostringstream& output,
		const std::vector<VALTAN_CINEMATIC_CAMERA_KEYFRAME>& keyframes,
		const char_t* indent)
	{
		output << '[';
		for (size_t index = 0u; index < keyframes.size(); ++index)
		{
			const auto& keyframe = keyframes[index];
			output << (0u == index ? "\n" : ",\n") << indent << "  {\n"
				<< indent << "    \"sceneId\": \""
				<< CDataJson::Escape(keyframe.strSceneId) << "\",\n"
				<< indent << "    \"timeMs\": " << keyframe.iTimeMs << ",\n"
				<< indent << "    \"eye\": ";
			Write_Float3(output, keyframe.vEye);
			output << ",\n" << indent << "    \"lookAt\": ";
			Write_Float3(output, keyframe.vLookAt);
			output << ",\n" << indent << "    \"fovYDegrees\": "
				<< keyframe.fFovYDegrees << "\n" << indent << "  }";
		}
		if (!keyframes.empty())
			output << '\n' << indent;
		output << ']';
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
		{ "schema", "formatVersion", "encounterId", "provenance", "cues",
			"deathCue" }))
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
	std::unordered_set<std::string> sceneIds;
	std::unordered_set<std::string> tuples;
	for (const DATA_JSON_VALUE& cueValue : cues->Get_Array())
	{
		if (!Is_ExactCameraCueObject(cueValue,
			{ "cueId", "patternId", "stageId", "durationMs", "interpolation", "easing",
				"shakeAmplitude", "shakeDurationMs", "keyframes" }))
		{
			outStatus = "Cinematic camera cue has unexpected properties";
			return false;
		}
		VALTAN_CINEMATIC_CAMERA_CUE cue;
		if (!Read_String(cueValue, "cueId", cue.strCueId) ||
			!Read_String(cueValue, "patternId", cue.strPatternId) ||
			!Read_String(cueValue, "stageId", cue.strStageId) ||
			!Is_StablePresentationAssetId(cue.strCueId) ||
			!Read_Unsigned(cueValue, "durationMs",
				CEncounterPatternReference::MAX_STAGE_DURATION_MS, cue.iDurationMs) ||
			0u == cue.iDurationMs || !cueIds.insert(cue.strCueId).second ||
			!Read_Transitions(cueValue, cue) ||
			!Read_Interpolation(cueValue, cue.eInterpolation) ||
			!Read_Easing(cueValue, cue.eEasing) ||
			!Read_Shake(cueValue, cue) ||
			!Read_Tracking(cueValue, cue.eTrackingMode, cue.vTrackingOrigin))
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
				{ "sceneId", "timeMs", "eye", "lookAt", "fovYDegrees" }))
			{
				outStatus = "Cinematic camera keyframe has unexpected properties";
				return false;
			}
			VALTAN_CINEMATIC_CAMERA_KEYFRAME keyframe;
			if (!Read_String(keyframeValue, "sceneId", keyframe.strSceneId) ||
				!Is_StablePresentationAssetId(keyframe.strSceneId) ||
				!sceneIds.insert(keyframe.strSceneId).second ||
				!Read_Unsigned(keyframeValue, "timeMs", cue.iDurationMs,
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

	/* The clear shot is optional: an empty object means the encounter authors no
	   death camera and the ordinary follow view simply stays. */
	VALTAN_CINEMATIC_CAMERA_CUE deathCue;
	bool_t hasDeathCue = false;
	const DATA_JSON_VALUE* deathCueValue = root.Find("deathCue");
	if (nullptr == deathCueValue || !deathCueValue->Is_Object())
	{
		outStatus = "Cinematic death cue must be an object";
		return false;
	}
	if (!deathCueValue->Get_Object().empty())
	{
		if (!Is_ExactObject(*deathCueValue,
			{ "cueId", "durationMs", "interpolation", "easing", "shakeAmplitude",
				"shakeDurationMs", "keyframes" }))
		{
			outStatus = "Cinematic death cue has unexpected properties";
			return false;
		}
		if (!Read_String(*deathCueValue, "cueId", deathCue.strCueId) ||
			!Is_StablePresentationAssetId(deathCue.strCueId) ||
			!cueIds.insert(deathCue.strCueId).second ||
			!Read_Unsigned(*deathCueValue, "durationMs",
				CEncounterPatternReference::MAX_STAGE_DURATION_MS,
				deathCue.iDurationMs) ||
			0u == deathCue.iDurationMs ||
			!Read_Interpolation(*deathCueValue, deathCue.eInterpolation) ||
			!Read_Easing(*deathCueValue, deathCue.eEasing) ||
			!Read_Shake(*deathCueValue, deathCue))
		{
			outStatus = "Cinematic death cue identity is invalid";
			return false;
		}
		const DATA_JSON_VALUE* deathKeyframes = deathCueValue->Find("keyframes");
		if (nullptr == deathKeyframes || !deathKeyframes->Is_Array() ||
			deathKeyframes->Get_Array().size() < 2u ||
			deathKeyframes->Get_Array().size() > MAX_KEYFRAME_COUNT)
		{
			outStatus = "Cinematic death cue keyframe array is invalid";
			return false;
		}
		uint32_t previousDeathTime = 0u;
		for (size_t index = 0u; index < deathKeyframes->Get_Array().size();
			++index)
		{
			const DATA_JSON_VALUE& keyframeValue =
				deathKeyframes->Get_Array()[index];
			if (!Is_ExactObject(keyframeValue,
				{ "sceneId", "timeMs", "eye", "lookAt", "fovYDegrees" }))
			{
				outStatus = "Cinematic death keyframe has unexpected properties";
				return false;
			}
			VALTAN_CINEMATIC_CAMERA_KEYFRAME keyframe;
			if (!Read_String(keyframeValue, "sceneId", keyframe.strSceneId) ||
				!Is_StablePresentationAssetId(keyframe.strSceneId) ||
				!sceneIds.insert(keyframe.strSceneId).second ||
				!Read_Unsigned(keyframeValue, "timeMs", deathCue.iDurationMs,
					keyframe.iTimeMs) ||
				!Read_Float3(keyframeValue, "eye", keyframe.vEye) ||
				!Read_Float3(keyframeValue, "lookAt", keyframe.vLookAt) ||
				!Read_Fov(keyframeValue, keyframe.fFovYDegrees) ||
				(index == 0u && 0u != keyframe.iTimeMs) ||
				(index > 0u && keyframe.iTimeMs <= previousDeathTime))
			{
				outStatus = "Cinematic death keyframe is invalid";
				return false;
			}
			const vector_t deathEye = XMLoadFloat3(&keyframe.vEye);
			const vector_t deathLookAt = XMLoadFloat3(&keyframe.vLookAt);
			if (XMVectorGetX(XMVector3LengthSq(deathLookAt - deathEye)) <=
				0.000001f)
			{
				outStatus = "Cinematic death eye and lookAt must differ";
				return false;
			}
			previousDeathTime = keyframe.iTimeMs;
			deathCue.Keyframes.push_back(keyframe);
		}
		if (previousDeathTime != deathCue.iDurationMs)
		{
			outStatus = "Cinematic death cue final keyframe must match duration";
			return false;
		}
		hasDeathCue = true;
	}

	CValtanCinematicCameraDocument committed;
	committed.m_strEncounterId = std::move(encounterId);
	committed.m_Cues = std::move(staged);
	committed.m_DeathCue = std::move(deathCue);
	committed.m_hasDeathCue = hasDeathCue;
	committed.m_isReady = true;
	outDocument = std::move(committed);
	outStatus = "Loaded Valtan cinematic camera cues: " +
		std::to_string(outDocument.m_Cues.size());
	return true;
}

bool_t Client::CValtanCinematicCameraDocument::Stage_CameraDraft(
	const std::vector<VALTAN_CINEMATIC_CAMERA_CUE>& cues,
	const bool_t hasDeathCue,
	const VALTAN_CINEMATIC_CAMERA_CUE& deathCue,
	const CEncounterPatternReference& encounter,
	CValtanCinematicCameraDocument& outDocument,
	std::string& outText,
	std::string& outStatus) const
{
	if (!m_isReady)
	{
		outStatus = "Cinematic camera authoring requires a loaded source document";
		return false;
	}
	CValtanCinematicCameraDocument staged = *this;
	staged.m_Cues = cues;
	staged.m_hasDeathCue = hasDeathCue;
	staged.m_DeathCue = hasDeathCue ? deathCue : VALTAN_CINEMATIC_CAMERA_CUE{};
	std::string serialized;
	if (!staged.Serialize_Text(serialized, outStatus))
		return false;
	CValtanCinematicCameraDocument validated;
	if (!Parse_Text(serialized, encounter, validated, outStatus))
		return false;
	outDocument = std::move(validated);
	outText = std::move(serialized);
	outStatus = "Validated cinematic camera authoring draft";
	return true;
}

bool_t Client::CValtanCinematicCameraDocument::Serialize_Text(
	std::string& outText,
	std::string& outStatus) const
{
	if (!m_isReady || m_strEncounterId.empty())
	{
		outStatus = "Cinematic camera document is not ready to serialize";
		return false;
	}
	std::ostringstream output;
	output << std::setprecision(std::numeric_limits<f32_t>::max_digits10);
	output << "{\n"
		"  \"schema\": \"" << SCHEMA << "\",\n"
		"  \"formatVersion\": " << FORMAT_VERSION << ",\n"
		"  \"encounterId\": \"" << CDataJson::Escape(m_strEncounterId) << "\",\n"
		"  \"provenance\": \"PROJECT_AUTHORED\",\n"
		"  \"cues\": [";
	for (size_t index = 0u; index < m_Cues.size(); ++index)
	{
		const VALTAN_CINEMATIC_CAMERA_CUE& cue = m_Cues[index];
		const char_t* easing = Easing_Text(cue.eEasing);
		const char_t* interpolation = Interpolation_Text(cue.eInterpolation);
		if (nullptr == easing || nullptr == interpolation)
		{
			outStatus =
				"Cinematic camera cue has unsupported interpolation or easing";
			return false;
		}
		output << (0u == index ? "\n" : ",\n")
			<< "    {\n"
			<< "      \"cueId\": \"" << CDataJson::Escape(cue.strCueId) << "\",\n"
			<< "      \"patternId\": \"" << CDataJson::Escape(cue.strPatternId) << "\",\n"
			<< "      \"stageId\": \"" << CDataJson::Escape(cue.strStageId) << '"';
		if (!Write_Tracking(output, cue.eTrackingMode, cue.vTrackingOrigin))
		{
			outStatus = "Cinematic camera cue has an unsupported tracking mode";
			return false;
		}
		output << ",\n      \"durationMs\": " << cue.iDurationMs;
		if (0u != cue.iTransitionInMs)
		{
			if (cue.iTransitionInMs > cue.iDurationMs ||
				cue.iTransitionInMs >
					VALTAN_CINEMATIC_CAMERA_CUE::MAX_TRANSITION_IN_MS)
			{
				outStatus = "Cinematic camera cue transition is out of bounds";
				return false;
			}
			output << ",\n      \"transitionInMs\": " << cue.iTransitionInMs;
		}
		if (0u != cue.iTransitionOutMs)
		{
			if (cue.iTransitionOutMs >
				VALTAN_CINEMATIC_CAMERA_CUE::MAX_TRANSITION_IN_MS)
			{
				outStatus =
					"Cinematic camera cue exit transition is out of bounds";
				return false;
			}
			output << ",\n      \"transitionOutMs\": " << cue.iTransitionOutMs;
		}
		output << ",\n      \"interpolation\": \"" << interpolation << "\",\n"
			<< "      \"easing\": \"" << easing << "\",\n"
			<< "      \"shakeAmplitude\": " << cue.fShakeAmplitude << ",\n"
			<< "      \"shakeDurationMs\": " << cue.iShakeDurationMs << ",\n"
			<< "      \"keyframes\": ";
		Write_Keyframes(output, cue.Keyframes, "      ");
		output << "\n    }";
	}
	if (!m_Cues.empty())
		output << '\n';
	output << "  ],\n  \"deathCue\": ";
	if (!m_hasDeathCue)
	{
		output << "{}";
	}
	else
	{
		const char_t* easing = Easing_Text(m_DeathCue.eEasing);
		const char_t* interpolation =
			Interpolation_Text(m_DeathCue.eInterpolation);
		if (nullptr == easing || nullptr == interpolation)
		{
			outStatus =
				"Cinematic death cue has unsupported interpolation or easing";
			return false;
		}
		output << "{\n"
			<< "    \"cueId\": \"" << CDataJson::Escape(m_DeathCue.strCueId) << "\",\n"
			<< "    \"durationMs\": " << m_DeathCue.iDurationMs << ",\n"
			<< "    \"interpolation\": \"" << interpolation << "\",\n"
			<< "    \"easing\": \"" << easing << "\",\n"
			<< "    \"shakeAmplitude\": " << m_DeathCue.fShakeAmplitude << ",\n"
			<< "    \"shakeDurationMs\": " << m_DeathCue.iShakeDurationMs << ",\n"
			<< "    \"keyframes\": ";
		Write_Keyframes(output, m_DeathCue.Keyframes, "    ");
		output << "\n  }";
	}
	output << "\n}\n";
	outText = output.str();
	outStatus = "Serialized cinematic camera document";
	return true;
}

void Client::CValtanCinematicCameraDocument::Clear()
{
	m_strEncounterId.clear();
	m_Cues.clear();
	m_DeathCue = VALTAN_CINEMATIC_CAMERA_CUE{};
	m_hasDeathCue = false;
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
