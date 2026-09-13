#include "imgui.h"
#include "Animation_Tool_Internal.h"
#include "Animation.h"
#include "AnimationPreviewAssets.h"
#include "BinaryAsset/WModelDecoder.h"
#include "Effect_Catalog.h"
#include "Model.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"
#include "SoundCueCatalog.h"
#include <charconv>
#include <algorithm>
#include <array>
#include <cerrno>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <io.h>
#include <iomanip>
#include <iterator>
#include <limits>
#include <map>
#include <sstream>
#include <span>
#include <system_error>
#include <tuple>
#include <unordered_map>
#include <unordered_set>




bool_t Client::CAnimation_Tool::Read_CompositionAnimationResources(
	std::vector<COMPOSITION_ANIMATION_RESOURCE>& outResources,
	std::string& outStatus) const
{
	m_bCompositionResourcesReadAttempted = true;
	std::vector<COMPOSITION_ANIMATION_RESOURCE> resources;
	std::string diagnostics;
	bool_t succeeded = true;
	for (const char* targetName : COMPOSITION_ANIMATION_TARGET_ASSET_NAMES)
	{
		const auto asset = std::find_if(ANIMATION_PREVIEW_ASSETS.begin(),
			ANIMATION_PREVIEW_ASSETS.end(), [targetName](const auto& candidate) {
				return std::string_view{ targetName } == candidate.pAssetName;
			});
		if (asset == ANIMATION_PREVIEW_ASSETS.end())
		{
			succeeded = false;
			diagnostics += std::string(targetName) + ": preview descriptor is missing. ";
			for (const auto& previous : m_CompositionAnimationResources)
				if (previous.strTargetAssetName == targetName) resources.push_back(previous);
			continue;
		}
		for (const char* source : { asset->pModelAssetId, asset->pAnimationSetAssetId })
		{
			if (nullptr == source) continue;
			std::vector<Engine::MODEL_ANIMATION_CATALOG_ENTRY> clips;
			std::string status;
			const std::filesystem::path path = CRuntimeAssetRoot::Resolve(source);
			if (path.empty() || !Engine::CWModelDecoder::Read_AnimationCatalog(path, clips, status))
			{
				succeeded = false;
				diagnostics += std::string(source) + ": " +
					(path.empty() ? "invalid Resources-relative path" : status) +
					"; previous rows retained. ";
				for (const auto& previous : m_CompositionAnimationResources)
					if (previous.strTargetAssetName == targetName && previous.strSourceAssetId == source)
						resources.push_back(previous);
				continue;
			}
			for (const auto& clip : clips)
			{
				COMPOSITION_ANIMATION_RESOURCE resource;
				resource.strTargetAssetName = targetName;
				resource.strModelAssetId = asset->pModelAssetId;
				resource.strSourceAssetId = source;
				resource.strProfileId = targetName;
				resource.strRuntimeClip = clip.name;
				resource.fDurationTicks = clip.durationTicks;
				// Match CAnimation's cooked clock, not the package import rate.
				resource.fTicksPerSecond = Engine::CAnimation::COOKED_TICK_RATE;
				resource.iDurationMs = static_cast<std::uint32_t>(std::clamp(
					std::round(static_cast<double>(resource.fDurationTicks) / resource.fTicksPerSecond * 1000.0),
					1.0, static_cast<double>((std::numeric_limits<std::uint32_t>::max)())));
				resources.push_back(std::move(resource));
			}
		}
	}
	std::sort(resources.begin(), resources.end(), [](const auto& left, const auto& right) {
		return std::tie(left.strTargetAssetName, left.strRuntimeClip, left.strSourceAssetId) <
			std::tie(right.strTargetAssetName, right.strRuntimeClip, right.strSourceAssetId);
	});
	m_CompositionAnimationResources = std::move(resources);
	outResources = m_CompositionAnimationResources;
	outStatus = std::to_string(outResources.size()) +
		" physical animation clips / 6 model targets. " + diagnostics;
	m_strCompositionAnimationResourceStatus = outStatus;
	return succeeded;
}

std::string Client::CAnimation_Tool::Get_EventFilePath() const
{
	return CProjectDataRoot::Resolve(
		filesystem::path(L"Animation/Authored") /
		filesystem::path(m_AssetName) /
		filesystem::path(m_AssetName + ".animevents")).string();
}

std::string Client::CAnimation_Tool::Get_SkillReferencePath() const
{
	return CProjectDataRoot::Resolve(
		filesystem::path(L"Animation/Reference") /
		filesystem::path(m_AssetName) /
		filesystem::path(m_AssetName + ".skilltiming")).string();
}

std::string Client::CAnimation_Tool::Get_ClipMapPath() const
{
	return CProjectDataRoot::Resolve(
		filesystem::path(L"Animation/Reference") /
		filesystem::path(m_AssetName) /
		filesystem::path(m_AssetName + ".clipmap")).string();
}

std::string Client::CAnimation_Tool::Get_ClipNotifyPath() const
{
	return CProjectDataRoot::Resolve(
		filesystem::path(L"Animation/Reference") /
		filesystem::path(m_AssetName) /
		filesystem::path(m_AssetName + ".animnotify")).string();
}

std::string Client::CAnimation_Tool::Get_ClipSeqPath() const
{
	return CProjectDataRoot::Resolve(
		filesystem::path(L"Animation/Reference") /
		filesystem::path(m_AssetName) /
		filesystem::path(m_AssetName + ".clipseq")).string();
}

bool_t Client::CAnimation_Tool::Write_EventsToPath(
	const std::filesystem::path& path,
	const std::vector<ANIM_EVENT>& events,
	std::string& outStatus) const
{
	outStatus.clear();

	FILE* file = nullptr;
	if (0 != _wfopen_s(&file, path.c_str(), L"wb") || nullptr == file)
	{
		outStatus = "Could not open temporary Animation Events file.";
		return false;
	}

	bool_t writeSucceeded = 0 <= fprintf(
		file,
		"%s %d \"%s\" %d\n",
		EVENT_FILE_MAGIC,
		EVENT_FILE_VERSION,
		m_AssetName.c_str(),
		static_cast<int32_t>(events.size()));

	for (const ANIM_EVENT& event : events)
	{
		const bool_t bAdmittedEffect =
			EVENT_KIND::EFFECT == event.eKind &&
			EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID ==
				event.eEffectReferenceKind;
		const char_t* kindName = Kind_Name(event.eKind);
		if (EVENT_KIND::HIT == event.eKind)
		{
			const HIT_PARAMS& hit = event.hit;
			writeSucceeded = writeSucceeded && 0 <= fprintf(
				file,
				"\"%s\" %s startms=%d endms=%d rep=%d repms=%d "
				"fz=%d fzin=%d fzout=%d push=%d pushr=%d "
				"area=%d ar=%d aa=%d ah=%d ax=%d arem=%d maxt=%d%s\n",
				event.clipName.c_str(), kindName,
				event.iStartMs, event.iEndMs,
				hit.iRepeatCount, hit.iRepeatMs,
				hit.iFreezeMs, hit.iFreezeInMs, hit.iFreezeOutMs,
				hit.iPushMs, hit.iPushRange,
				hit.iAreaType, hit.iAreaRange, hit.iAreaAngle,
				hit.iAreaHeight, hit.iAreaOffsetX, hit.iAreaInner,
				hit.iMaxTargets,
				event.bImported ? " src=orig" : "");
		}
		else if (Is_Window(event.eKind))
		{
			writeSucceeded = writeSucceeded && 0 <= fprintf(
				file,
				"\"%s\" %s startms=%d endms=%d payload=\"%s\"%s\n",
				event.clipName.c_str(), kindName,
				event.iStartMs, event.iEndMs, event.sPayload.c_str(),
				event.bImported ? " src=orig" : "");
		}
		else if (EVENT_KIND::EFFECT == event.eKind)
		{
			if (EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID ==
				event.eEffectReferenceKind)
			{
				writeSucceeded = writeSucceeded && 0 <= fprintf(
					file,
					"\"%s\" %s startms=%d endms=%d payload=\"%s\" "
					"effectref=asset anchor=\"%s\" follow=%s orientation=%s stop=%s "
					"px=%.9g py=%.9g pz=%.9g rx=%.9g ry=%.9g rz=%.9g "
					"sx=%.9g sy=%.9g sz=%.9g%s\n",
					event.clipName.c_str(), kindName,
					event.iStartMs, event.iEndMs,
					event.sPayload.c_str(), event.sAnchorSlotId.c_str(),
					EFFECT_FOLLOW_POLICY::FOLLOW == event.eFollowPolicy ?
						"follow" : "snapshot",
					EFFECT_ORIENTATION_POLICY::ANCHOR ==
						event.eOrientationPolicy ? "anchor" : "action_facing",
					EFFECT_STOP_POLICY::NATURAL == event.eStopPolicy ?
						"natural" : "cue_end",
					event.EffectLocalTransform.vPosition.x,
					event.EffectLocalTransform.vPosition.y,
					event.EffectLocalTransform.vPosition.z,
					event.EffectLocalTransform.vRotationDegrees.x,
					event.EffectLocalTransform.vRotationDegrees.y,
					event.EffectLocalTransform.vRotationDegrees.z,
					event.EffectLocalTransform.vScale.x,
					event.EffectLocalTransform.vScale.y,
					event.EffectLocalTransform.vScale.z,
					event.bImported ? " src=orig" : "");
			}
			else
			{
				writeSucceeded = writeSucceeded && 0 <= fprintf(
					file,
					"\"%s\" %s startms=%d payload=\"%s\" "
					"effectref=source%s\n",
					event.clipName.c_str(), kindName,
					event.iStartMs, event.sPayload.c_str(),
					event.bImported ? " src=orig" : "");
			}
		}
		else
		{
			writeSucceeded = writeSucceeded && 0 <= fprintf(
				file,
				"\"%s\" %s startms=%d payload=\"%s\"%s\n",
				event.clipName.c_str(), kindName,
				event.iStartMs, event.sPayload.c_str(),
				event.bImported ? " src=orig" : "");
		}
	}

	const bool_t flushSucceeded = 0 == fflush(file);
	const bool_t commitSucceeded =
		flushSucceeded && 0 == _commit(_fileno(file));
	const bool_t closeSucceeded = 0 == fclose(file);
	if (!writeSucceeded || !flushSucceeded ||
		!commitSucceeded || !closeSucceeded)
	{
		outStatus = "Could not flush Animation Events temporary file.";
		return false;
	}

	return true;
}

bool_t Client::CAnimation_Tool::Validate_Events(
	const shared_ptr<Engine::CModel>& pModel,
	const std::vector<ANIM_EVENT>& events,
	std::string& outStatus) const
{
	outStatus.clear();
	if (nullptr == pModel || m_AssetName.empty() ||
		events.size() > static_cast<size_t>(MAX_EVENT_COUNT))
	{
		outStatus = "Animation event document header is invalid.";
		return false;
	}

	for (const ANIM_EVENT& event : events)
	{
		const bool_t bAdmittedEffect =
			EVENT_KIND::EFFECT == event.eKind &&
			EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID ==
				event.eEffectReferenceKind;
		bool_t hasClip = false;
		for (uint32_t index = 0;
			index < pModel->Get_NumAnimations();
			++index)
		{
			const char_t* clipName =
				pModel->Get_AnimationName(index);
			if (nullptr != clipName && event.clipName == clipName)
			{
				hasClip = true;
				break;
			}
		}

		if (!hasClip ||
			!Is_SafeQuotedText(event.clipName) ||
			!Is_SafeQuotedText(event.sPayload) ||
			event.iStartMs < 0 ||
			event.iEndMs < event.iStartMs ||
			(!Is_Window(event.eKind) &&
				event.iEndMs != event.iStartMs &&
				!(bAdmittedEffect &&
					EFFECT_STOP_POLICY::CUE_END == event.eStopPolicy)) ||
			ETOI(event.eKind) < 0 ||
			ETOI(event.eKind) >= ETOI(EVENT_KIND::END))
		{
			outStatus =
				"Animation event clip, time, kind, or payload is invalid.";
			return false;
		}

		if (EVENT_KIND::HIT == event.eKind &&
			(event.hit.iRepeatCount < 1 ||
				event.hit.iRepeatCount > 1024 ||
				event.hit.iRepeatMs < 0))
		{
			outStatus = "Animation HIT repeat contract is invalid.";
			return false;
		}

		if (EVENT_KIND::EFFECT == event.eKind)
		{
			if (EFFECT_REFERENCE_KIND::NONE ==
				event.eEffectReferenceKind)
			{
				outStatus =
					"Animation EFFECT row has no reference kind.";
				return false;
			}
			if (EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID ==
				event.eEffectReferenceKind)
			{
				const EFFECT_TRANSFORM_DESC& local =
					event.EffectLocalTransform;
				if (!CEffectCatalog::Contains(event.sPayload) ||
					event.sAnchorSlotId.empty() ||
					!Is_SafeQuotedText(event.sAnchorSlotId) ||
					EFFECT_FOLLOW_POLICY::END == event.eFollowPolicy ||
					EFFECT_ORIENTATION_POLICY::END ==
						event.eOrientationPolicy ||
					(EFFECT_ORIENTATION_POLICY::ACTION_FACING ==
						event.eOrientationPolicy &&
						"root" != event.sAnchorSlotId) ||
					EFFECT_STOP_POLICY::END == event.eStopPolicy ||
					!std::isfinite(local.vPosition.x) ||
					!std::isfinite(local.vPosition.y) ||
					!std::isfinite(local.vPosition.z) ||
					!std::isfinite(local.vRotationDegrees.x) ||
					!std::isfinite(local.vRotationDegrees.y) ||
					!std::isfinite(local.vRotationDegrees.z) ||
					!std::isfinite(local.vScale.x) ||
					!std::isfinite(local.vScale.y) ||
					!std::isfinite(local.vScale.z) ||
					local.vScale.x <= 0.f || local.vScale.y <= 0.f ||
					local.vScale.z <= 0.f ||
					(EFFECT_STOP_POLICY::CUE_END == event.eStopPolicy &&
						event.iEndMs <= event.iStartMs))
				{
					outStatus =
						"Admitted Effect cue ID, anchor, policy, or local transform is invalid.";
					return false;
				}
			}
		}
		else if (EFFECT_REFERENCE_KIND::NONE !=
			event.eEffectReferenceKind)
		{
			outStatus =
				"Only Animation EFFECT rows may carry an effect reference.";
			return false;
		}
	}

	return true;
}

bool_t Client::CAnimation_Tool::Events_AreEqual(
	const std::vector<ANIM_EVENT>& left,
	const std::vector<ANIM_EVENT>& right) const
{
	if (left.size() != right.size())
		return false;

	const auto hitsAreEqual = [](const HIT_PARAMS& a, const HIT_PARAMS& b)
	{
		return a.iRepeatCount == b.iRepeatCount &&
			a.iRepeatMs == b.iRepeatMs &&
			a.iFreezeMs == b.iFreezeMs &&
			a.iFreezeInMs == b.iFreezeInMs &&
			a.iFreezeOutMs == b.iFreezeOutMs &&
			a.iPushMs == b.iPushMs &&
			a.iPushRange == b.iPushRange &&
			a.iAreaType == b.iAreaType &&
			a.iAreaRange == b.iAreaRange &&
			a.iAreaAngle == b.iAreaAngle &&
			a.iAreaHeight == b.iAreaHeight &&
			a.iAreaOffsetX == b.iAreaOffsetX &&
			a.iAreaInner == b.iAreaInner &&
			a.iMaxTargets == b.iMaxTargets;
	};
	const auto effectTransformsAreEqual = [](
		const EFFECT_TRANSFORM_DESC& a,
		const EFFECT_TRANSFORM_DESC& b)
	{
		return a.vPosition.x == b.vPosition.x &&
			a.vPosition.y == b.vPosition.y &&
			a.vPosition.z == b.vPosition.z &&
			a.vRotationDegrees.x == b.vRotationDegrees.x &&
			a.vRotationDegrees.y == b.vRotationDegrees.y &&
			a.vRotationDegrees.z == b.vRotationDegrees.z &&
			a.vScale.x == b.vScale.x &&
			a.vScale.y == b.vScale.y &&
			a.vScale.z == b.vScale.z;
	};

	for (size_t index = 0; index < left.size(); ++index)
	{
		const ANIM_EVENT& a = left[index];
		const ANIM_EVENT& b = right[index];
		if (a.clipName != b.clipName ||
			a.eKind != b.eKind ||
			a.iStartMs != b.iStartMs ||
			a.iEndMs != b.iEndMs ||
			a.sPayload != b.sPayload ||
			a.eEffectReferenceKind != b.eEffectReferenceKind ||
			a.sAnchorSlotId != b.sAnchorSlotId ||
			a.eFollowPolicy != b.eFollowPolicy ||
			a.eOrientationPolicy != b.eOrientationPolicy ||
			a.eStopPolicy != b.eStopPolicy ||
			!effectTransformsAreEqual(a.EffectLocalTransform,
				b.EffectLocalTransform) ||
			a.bImported != b.bImported ||
			!hitsAreEqual(a.hit, b.hit))
		{
			return false;
		}
	}
	return true;
}

bool_t Client::CAnimation_Tool::Load_EventsFromPath(
	const std::filesystem::path& path,
	const shared_ptr<Engine::CModel>& pModel,
	std::vector<ANIM_EVENT>& outEvents,
	int32_t& outSourceVersion,
	std::string& outStatus) const
{
	outEvents.clear();
	outSourceVersion = {};
	outStatus.clear();

	FILE* file = nullptr;
	if (0 != _wfopen_s(&file, path.c_str(), L"rb") || nullptr == file)
	{
		outStatus = "No event file: " + path.string();
		return false;
	}

	int32_t lineNumber = 1;
	const auto fail = [&](
		const std::string& message) -> bool_t
	{
		fclose(file);
		outStatus = message + " [line " +
			std::to_string(lineNumber) + "]";
		return false;
	};

	char_t line[4096]{};
	if (nullptr == fgets(line, sizeof(line), file))
		return fail("Animation event file is empty.");

	const char_t* cursor = line;
	std::string magic;
	std::string versionToken;
	std::string owner;
	std::string countToken;
	int32_t version = {};
	int32_t declaredCount = {};
	if (!Read_Token(cursor, magic) ||
		!Read_Token(cursor, versionToken) ||
		!Read_Quoted(cursor, owner) ||
		!Read_Token(cursor, countToken) ||
		Has_RemainingToken(cursor) ||
		EVENT_FILE_MAGIC != magic ||
		!Parse_Integer(versionToken, version) ||
		!Parse_Integer(countToken, declaredCount) ||
		version < 1 || version > EVENT_FILE_VERSION ||
		declaredCount < 0 || declaredCount > MAX_EVENT_COUNT ||
		owner != m_AssetName)
	{
		return fail("Animation event header is invalid.");
	}

	std::vector<ANIM_EVENT> staged;
	staged.reserve(static_cast<size_t>(declaredCount));

	while (nullptr != fgets(line, sizeof(line), file))
	{
		++lineNumber;
		if (staged.size() >= static_cast<size_t>(declaredCount))
			return fail("Animation event row count exceeds the header.");

		const size_t lineLength = strlen(line);
		if (lineLength == sizeof(line) - 1u &&
			'\n' != line[lineLength - 1u] && !feof(file))
		{
			return fail("Animation event row is too long.");
		}

		cursor = line;
		std::string clipName;
		std::string kindName;
		if (!Read_Quoted(cursor, clipName) ||
			!Read_Token(cursor, kindName))
		{
			return fail("Malformed Animation event row.");
		}

		ANIM_EVENT event{};
		event.clipName = clipName;
		if ("HIT" == kindName)
			event.eKind = EVENT_KIND::HIT;
		else if ("CANCEL" == kindName)
			event.eKind = EVENT_KIND::CANCEL;
		else if ("SUPERARMOR" == kindName)
			event.eKind = EVENT_KIND::SUPERARMOR;
		else if ("INVULN" == kindName)
			event.eKind = EVENT_KIND::INVULN;
		else if ("MOVE" == kindName)
			event.eKind = EVENT_KIND::MOVE;
		else if ("SOUND" == kindName)
			event.eKind = EVENT_KIND::SOUND;
		else if ("EFFECT" == kindName)
		{
			event.eKind = EVENT_KIND::EFFECT;
			if (version < 4)
			{
				event.eEffectReferenceKind =
					EFFECT_REFERENCE_KIND::SOURCE_REFERENCE;
			}
		}
		else if ("SHAKE" == kindName)
			event.eKind = EVENT_KIND::SHAKE;
		else if ("COUNTER" == kindName)
			event.eKind = EVENT_KIND::COUNTER;
		else if ("STAGE" == kindName)
			event.eKind = EVENT_KIND::STAGE;
		else
			return fail("Unknown Animation event kind.");

		int32_t startFrame = {};
		int32_t endFrame = {};
		int32_t intervalFrame = {};
		bool_t hasStartFrame = false;
		bool_t hasEndFrame = false;
		bool_t hasStartMs = false;
		bool_t hasEndMs = false;
		bool_t hasEffectReference = false;
		bool_t hasOrientation = false;

		if (1 == version)
		{
			std::string startToken;
			std::string endToken;
			if (EVENT_KIND::HIT != event.eKind ||
				!Read_Token(cursor, startToken) ||
				!Read_Token(cursor, endToken) ||
				Has_RemainingToken(cursor) ||
				!Parse_Integer(startToken, startFrame) ||
				!Parse_Integer(endToken, endFrame))
			{
				return fail("Invalid v1 Animation HIT row.");
			}
			hasStartFrame = true;
			hasEndFrame = true;
		}
		else
		{
			std::unordered_set<std::string> keys;
			while (Has_RemainingToken(cursor))
			{
				std::string key;
				std::string value;
				if (!Read_Pair(cursor, key, value) ||
					!keys.insert(key).second)
				{
					return fail(
						"Malformed or duplicate Animation event field.");
				}

				if ("payload" == key)
				{
					event.sPayload = value;
					continue;
				}
				if ("src" == key)
				{
					if ("orig" != value)
						return fail("Unknown Animation event source.");
					event.bImported = true;
					continue;
				}
				if ("effectref" == key)
				{
					if (version < 4 ||
						EVENT_KIND::EFFECT != event.eKind)
					{
						return fail(
							"effectref is only valid for v4-v6 EFFECT rows.");
					}
					if ("source" == value)
					{
						event.eEffectReferenceKind =
							EFFECT_REFERENCE_KIND::SOURCE_REFERENCE;
					}
					else if ("asset-id" == value || "asset" == value)
					{
						event.eEffectReferenceKind =
							EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID;
					}
					else
						return fail("Unknown Animation effect reference kind.");
					hasEffectReference = true;
					continue;
				}
				if ("anchor" == key)
				{
					if (version < 5 || EVENT_KIND::EFFECT != event.eKind)
						return fail("anchor is only valid for v5/v6 EFFECT rows.");
					event.sAnchorSlotId = value;
					continue;
				}
				if ("follow" == key)
				{
					if (version < 5 || EVENT_KIND::EFFECT != event.eKind)
						return fail("follow is only valid for v5/v6 EFFECT rows.");
					if ("follow" == value)
						event.eFollowPolicy = EFFECT_FOLLOW_POLICY::FOLLOW;
					else if ("snapshot" == value)
						event.eFollowPolicy = EFFECT_FOLLOW_POLICY::SNAPSHOT;
					else
						return fail("Unknown Effect follow policy.");
					continue;
				}
				if ("orientation" == key)
				{
					if (version < 6 || EVENT_KIND::EFFECT != event.eKind)
					{
						return fail(
							"orientation is only valid for v6 EFFECT rows.");
					}
					if ("anchor" == value)
					{
						event.eOrientationPolicy =
							EFFECT_ORIENTATION_POLICY::ANCHOR;
					}
					else if ("action_facing" == value)
					{
						event.eOrientationPolicy =
							EFFECT_ORIENTATION_POLICY::ACTION_FACING;
					}
					else
						return fail("Unknown Effect orientation policy.");
					hasOrientation = true;
					continue;
				}
				if ("stop" == key)
				{
					if (version < 5 || EVENT_KIND::EFFECT != event.eKind)
						return fail("stop is only valid for v5/v6 EFFECT rows.");
					if ("natural" == value)
						event.eStopPolicy = EFFECT_STOP_POLICY::NATURAL;
					else if ("cue_end" == value)
						event.eStopPolicy = EFFECT_STOP_POLICY::CUE_END;
					else
						return fail("Unknown Effect stop policy.");
					continue;
				}
				if (key == "px" || key == "py" || key == "pz" ||
					key == "rx" || key == "ry" || key == "rz" ||
					key == "sx" || key == "sy" || key == "sz")
				{
					if (version < 5 || EVENT_KIND::EFFECT != event.eKind)
						return fail("Effect local transform is only valid in v5/v6.");
					f32_t number = 0.f;
					if (!Parse_Float(value, number))
						return fail("Animation Effect transform number is invalid.");
					if ("px" == key) event.EffectLocalTransform.vPosition.x = number;
					else if ("py" == key) event.EffectLocalTransform.vPosition.y = number;
					else if ("pz" == key) event.EffectLocalTransform.vPosition.z = number;
					else if ("rx" == key) event.EffectLocalTransform.vRotationDegrees.x = number;
					else if ("ry" == key) event.EffectLocalTransform.vRotationDegrees.y = number;
					else if ("rz" == key) event.EffectLocalTransform.vRotationDegrees.z = number;
					else if ("sx" == key) event.EffectLocalTransform.vScale.x = number;
					else if ("sy" == key) event.EffectLocalTransform.vScale.y = number;
					else if ("sz" == key) event.EffectLocalTransform.vScale.z = number;
					continue;
				}

				int32_t number = {};
				if (!Parse_Integer(value, number))
					return fail("Animation event number is invalid.");

				if ("start" == key)
				{
					startFrame = number;
					hasStartFrame = true;
				}
				else if ("end" == key)
				{
					endFrame = number;
					hasEndFrame = true;
				}
				else if ("count" == key)
					event.hit.iRepeatCount = number;
				else if ("interval" == key)
					intervalFrame = number;
				else if ("startms" == key)
				{
					event.iStartMs = number;
					hasStartMs = true;
				}
				else if ("endms" == key)
				{
					event.iEndMs = number;
					hasEndMs = true;
				}
				else if ("rep" == key)
					event.hit.iRepeatCount = number;
				else if ("repms" == key)
					event.hit.iRepeatMs = number;
				else if ("fz" == key)
					event.hit.iFreezeMs = number;
				else if ("fzin" == key)
					event.hit.iFreezeInMs = number;
				else if ("fzout" == key)
					event.hit.iFreezeOutMs = number;
				else if ("push" == key)
					event.hit.iPushMs = number;
				else if ("pushr" == key)
					event.hit.iPushRange = number;
				else if ("area" == key)
					event.hit.iAreaType = number;
				else if ("ar" == key)
					event.hit.iAreaRange = number;
				else if ("aa" == key)
					event.hit.iAreaAngle = number;
				else if ("ah" == key)
					event.hit.iAreaHeight = number;
				else if ("ax" == key)
					event.hit.iAreaOffsetX = number;
				else if ("arem" == key)
					event.hit.iAreaInner = number;
				else if ("maxt" == key)
					event.hit.iMaxTargets = number;
				else
					return fail("Unknown Animation event field.");
			}
		}

		if (2 == version)
		{
			if (!hasStartFrame ||
				(Is_Window(event.eKind) && !hasEndFrame))
			{
				return fail("v2 Animation event time is missing.");
			}
			if (!hasEndFrame)
				endFrame = startFrame;
		}
		else if (version >= 3)
		{
			if (!hasStartMs ||
				(Is_Window(event.eKind) && !hasEndMs))
			{
				return fail("Animation event millisecond time is missing.");
			}
			if (!hasEndMs)
				event.iEndMs = event.iStartMs;
		}

		if (version < 3)
		{
			const f32_t tickRate =
				Get_ClipTickRate(pModel, event.clipName);
			if (2 == version &&
				intervalFrame > 0 &&
				event.hit.iRepeatCount <= 1)
			{
				event.hit.iRepeatCount =
					(endFrame - startFrame) / intervalFrame + 1;
			}
			if (event.hit.iRepeatCount > 1)
			{
				event.hit.iRepeatMs = intervalFrame > 0 ?
					Frame_To_Ms(intervalFrame, tickRate) :
					Frame_To_Ms(
						endFrame - startFrame,
						tickRate) /
						(event.hit.iRepeatCount - 1);
				endFrame = startFrame;
			}
			event.iStartMs = Frame_To_Ms(startFrame, tickRate);
			event.iEndMs = Frame_To_Ms(endFrame, tickRate);
		}

		if (event.hit.iRepeatCount < 1)
			event.hit.iRepeatCount = 1;
		if (!Is_Window(event.eKind) &&
			!(EVENT_KIND::EFFECT == event.eKind &&
				EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID ==
					event.eEffectReferenceKind &&
				EFFECT_STOP_POLICY::CUE_END == event.eStopPolicy))
			event.iEndMs = event.iStartMs;
		if (3 == version && EVENT_KIND::EFFECT == event.eKind &&
			!hasEffectReference)
		{
			event.eEffectReferenceKind =
				EFFECT_REFERENCE_KIND::SOURCE_REFERENCE;
		}
		if (version >= 4 &&
			EVENT_KIND::EFFECT == event.eKind &&
			!hasEffectReference)
		{
			if (event.bImported)
				event.eEffectReferenceKind =
					EFFECT_REFERENCE_KIND::SOURCE_REFERENCE;
			else
				return fail("Authored v4-v6 EFFECT row requires effectref.");
		}
		if (hasOrientation &&
			EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID !=
				event.eEffectReferenceKind)
		{
			return fail(
				"orientation is only valid for admitted Effect asset rows.");
		}
		if (EVENT_KIND::EFFECT == event.eKind &&
			EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID ==
				event.eEffectReferenceKind &&
			EFFECT_STOP_POLICY::CUE_END == event.eStopPolicy &&
			(!hasEndMs || event.iEndMs <= event.iStartMs))
		{
			return fail("cue_end requires endms greater than startms.");
		}

		staged.push_back(std::move(event));
	}

	if (0 != ferror(file))
		return fail("Animation event file read failed.");
	if (0 != fclose(file))
	{
		outStatus = "Animation event file close failed.";
		return false;
	}
	file = nullptr;

	if (staged.size() != static_cast<size_t>(declaredCount))
	{
		outStatus = "Animation event row count does not match the header.";
		return false;
	}

	outEvents = std::move(staged);
	outSourceVersion = version;
	return true;
}

bool_t Client::CAnimation_Tool::Save_Events(
	const shared_ptr<Engine::CModel>& pModel)
{
	std::string validationStatus;
	if (!Validate_Events(pModel, m_Events, validationStatus))
	{
		m_Status = "Save rejected: " + validationStatus;
		return false;
	}

	const std::filesystem::path destination{ Get_EventFilePath() };
	std::error_code directoryError;
	std::filesystem::create_directories(
		destination.parent_path(), directoryError);
	if (directoryError)
	{
		m_Status = "Save failed to create authoring directory: " +
			directoryError.message();
		return false;
	}

	std::filesystem::path temporary = destination;
	temporary += L".tmp";
	std::error_code removeError;
	std::filesystem::remove(temporary, removeError);
	if (removeError)
	{
		m_Status = "Save failed to clear stale temporary file: " +
			removeError.message();
		return false;
	}

	std::string transactionStatus;
	if (!Write_EventsToPath(
		temporary, m_Events, transactionStatus))
	{
		std::error_code cleanupError;
		std::filesystem::remove(temporary, cleanupError);
		m_Status = "Save failed; previous file preserved: " +
			transactionStatus;
		return false;
	}

	std::vector<ANIM_EVENT> staged;
	int32_t sourceVersion = {};
	if (!Load_EventsFromPath(
		temporary,
		pModel,
		staged,
		sourceVersion,
		transactionStatus) ||
		EVENT_FILE_VERSION != sourceVersion ||
		!Validate_Events(pModel, staged, transactionStatus) ||
		!Events_AreEqual(staged, m_Events))
	{
		std::error_code cleanupError;
		std::filesystem::remove(temporary, cleanupError);
		if (transactionStatus.empty())
		{
			transactionStatus =
				"Animation event round trip changed the document.";
		}
		m_Status = "Save validation failed; previous file preserved: " +
			transactionStatus;
		return false;
	}

	if (!MoveFileExW(
		temporary.c_str(),
		destination.c_str(),
		MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
	{
		const DWORD errorCode = GetLastError();
		std::error_code cleanupError;
		std::filesystem::remove(temporary, cleanupError);
		m_Status = "Atomic replace failed; previous file preserved. Win32 error " +
			std::to_string(errorCode) + ".";
		return false;
	}

	m_bDirty = false;
	m_Status = "Saved " + std::to_string(m_Events.size()) +
		" event(s) atomically to " + destination.string();
	return true;
}

bool_t Client::CAnimation_Tool::Load_Events(
	const shared_ptr<Engine::CModel>& pModel)
{
	const std::filesystem::path path{ Get_EventFilePath() };
	std::vector<ANIM_EVENT> staged;
	int32_t sourceVersion = {};
	std::string loadStatus;
	if (!Load_EventsFromPath(
		path, pModel, staged, sourceVersion, loadStatus) ||
		!Validate_Events(pModel, staged, loadStatus))
	{
		m_Status =
			"Load rejected; current document preserved: " + loadStatus;
		return false;
	}

	m_Events = std::move(staged);
	m_iSelectedEvent = -1;
	/* v3 already stores milliseconds and can be represented by v4 without moving
	any marker. Loading it must not create a false unsaved document that blocks a
	Character Select target change. Only v1/v2 frame conversion is dirty. */
	m_bDirty = sourceVersion < 3;
	m_Status = "Loaded " + std::to_string(m_Events.size()) +
		" event(s) from " + path.string();
	if (m_bDirty)
	{
		m_Status += " [converted from v" +
			std::to_string(sourceVersion) +
			" in memory; review and Save explicitly]";
	}
	else if (sourceVersion < EVENT_FILE_VERSION)
	{
		m_Status +=
			" [legacy source references preserved; the next authored Save writes v6]";
	}
	return true;
}

bool_t Client::CAnimation_Tool::Load_ClipMap()
{
	m_ClipMap.clear();

	const std::string path = Get_ClipMapPath();

	FILE* pFile = nullptr;
	if (0 != fopen_s(&pFile, path.c_str(), "r") || nullptr == pFile)
		return false;

	char_t szLine[1024]{};
	if (nullptr == fgets(szLine, sizeof(szLine), pFile))
	{
		fclose(pFile);
		return false;
	}

	const char_t* p = szLine;
	std::string magic, versionToken, owner, countToken;
	if (!Read_Token(p, magic) || !Read_Token(p, versionToken) ||
		!Read_Quoted(p, owner) || !Read_Token(p, countToken) ||
		magic != "LOSTARK_CLIP_MAP")
	{
		fclose(pFile);
		return false;
	}

	while (nullptr != fgets(szLine, sizeof(szLine), pFile))
	{
		p = szLine;

		std::string clip;
		if (!Read_Quoted(p, clip))
			continue;

		CLIP_INFO info{};
		std::string key, value;
		while (Read_Pair(p, key, value))
		{
			if ("skill" == key)
				info.iSkillId = atoi(value.c_str());
			else if ("name" == key)
				info.name = value;
		}

		m_ClipMap[clip] = info;
	}

	fclose(pFile);
	return true;
}

bool_t Client::CAnimation_Tool::Load_ClipNotify()
{
	m_ClipNotify.clear();

	const std::string path = Get_ClipNotifyPath();

	FILE* pFile = nullptr;
	if (0 != fopen_s(&pFile, path.c_str(), "r") || nullptr == pFile)
		return false;

	char_t szLine[2048]{};
	if (nullptr == fgets(szLine, sizeof(szLine), pFile))
	{
		fclose(pFile);
		return false;
	}

	const char_t* p = szLine;
	std::string magic, versionToken, owner, countToken;
	if (!Read_Token(p, magic) || !Read_Token(p, versionToken) ||
		!Read_Quoted(p, owner) || !Read_Token(p, countToken) ||
		magic != "LOSTARK_ANIM_NOTIFY")
	{
		fclose(pFile);
		return false;
	}

	std::vector<NOTIFY_ROW>* pCurrent = nullptr;

	while (nullptr != fgets(szLine, sizeof(szLine), pFile))
	{
		p = szLine;

		/* A clip header opens with a quoted name; its rows start with "n". */
		std::string token;
		const char_t* pProbe = p;
		if (Read_Quoted(pProbe, token))
		{
			pCurrent = &m_ClipNotify[token];

			/* The header also carries the clip's own length, which the chain
			offsets are summed from. */
			std::string hkey, hvalue;
			while (Read_Pair(pProbe, hkey, hvalue))
			{
				if ("len" == hkey)
					m_ClipLength[token] = static_cast<f32_t>(atof(hvalue.c_str()));
			}
			continue;
		}

		if (!Read_Token(p, token) || "n" != token || nullptr == pCurrent)
			continue;

		NOTIFY_ROW row{};
		bool_t bKnown = false;

		std::string key, value;
		while (Read_Pair(p, key, value))
		{
			if ("t" == key)
				row.fTime = static_cast<f32_t>(atof(value.c_str()));
			else if ("d" == key)
				row.fDuration = static_cast<f32_t>(atof(value.c_str()));
			else if ("asset" == key)
				row.sAsset = value;
			else if ("label" == key)
				row.sLabel = value;
			else if ("kind" == key)
			{
				bKnown = true;
				if ("HIT" == value)
					row.eKind = EVENT_KIND::HIT;
				else if ("CANCEL" == value)
					row.eKind = EVENT_KIND::CANCEL;
				else if ("SUPERARMOR" == value)
					row.eKind = EVENT_KIND::SUPERARMOR;
				else if ("COUNTER" == value)
					row.eKind = EVENT_KIND::COUNTER;
				else if ("SOUND" == value)
					row.eKind = EVENT_KIND::SOUND;
				else if ("EFFECT" == value)
					row.eKind = EVENT_KIND::EFFECT;
				else if ("SHAKE" == value)
					row.eKind = EVENT_KIND::SHAKE;
				else if ("STAGE" == value)
					row.eKind = EVENT_KIND::STAGE;
				else
					bKnown = false;
			}
		}

		if (bKnown)
			pCurrent->push_back(row);
	}

	fclose(pFile);
	return true;
}

bool_t Client::CAnimation_Tool::Load_ClipSeq()
{
	m_ClipSeqs.clear();
	m_ClipChainCounts.clear();

	const std::string path = Get_ClipSeqPath();

	FILE* pFile = nullptr;
	if (0 != fopen_s(&pFile, path.c_str(), "r") || nullptr == pFile)
		return false;

	char_t szLine[2048]{};
	if (nullptr == fgets(szLine, sizeof(szLine), pFile))
	{
		fclose(pFile);
		return false;
	}

	const char_t* p = szLine;
	std::string magic, versionToken, owner, countToken;
	if (!Read_Token(p, magic) || !Read_Token(p, versionToken) ||
		!Read_Quoted(p, owner) || !Read_Token(p, countToken) ||
		magic != "LOSTARK_CLIP_SEQ")
	{
		fclose(pFile);
		return false;
	}

	while (nullptr != fgets(szLine, sizeof(szLine), pFile))
	{
		p = szLine;

		std::string idToken, name;
		if (!Read_Token(p, idToken) || !Read_Quoted(p, name))
			continue;

		CLIP_SEQ seq{};
		seq.iSkillId = atoi(idToken.c_str());
		seq.name = name;

		std::string key, value;
		while (Read_Pair(p, key, value))
		{
			if ("seq" == key)
				seq.iSeqIndex = atoi(value.c_str());
			else if ("mode" == key)
				seq.sMode = value;
			else if ("clips" == key)
			{
				/* comma separated, in playback order */
				size_t start = 0;
				while (start <= value.size())
				{
					const size_t comma = value.find(',', start);
					const std::string one = value.substr(start,
						std::string::npos == comma ? std::string::npos : comma - start);
					if (!one.empty())
						seq.clips.push_back(one);
					if (std::string::npos == comma)
						break;
					start = comma + 1;
				}
			}
		}

		/* One-clip chains are kept: they are still what the skill casts, and the
		runtime reads the same file. */
		if (!seq.clips.empty())
			m_ClipSeqs.push_back(seq);
	}

	for (const CLIP_SEQ& seq : m_ClipSeqs)
	{
		for (const std::string& clip : seq.clips)
			++m_ClipChainCounts[clip];
	}

	fclose(pFile);
	Load_ClipCuts();
	return true;
}

void Client::CAnimation_Tool::Load_ClipCuts()
{
	const std::string path = CProjectDataRoot::Resolve(
		filesystem::path(L"Animation/Reference") /
		filesystem::path(m_AssetName) /
		filesystem::path(m_AssetName + ".clipcuts")).string();

	FILE* pFile = nullptr;
	if (0 != fopen_s(&pFile, path.c_str(), "r") || nullptr == pFile)
		return;

	char_t szLine[2048]{};
	if (nullptr == fgets(szLine, sizeof(szLine), pFile))
	{
		fclose(pFile);
		return;
	}

	const char_t* p = szLine;
	std::string magic, versionToken, owner, countToken;
	if (!Read_Token(p, magic) || !Read_Token(p, versionToken) ||
		!Read_Quoted(p, owner) || !Read_Token(p, countToken) ||
		magic != "LOSTARK_CLIP_CUTS")
	{
		fclose(pFile);
		return;
	}

	std::map<std::pair<int32_t, int32_t>, std::vector<f32_t>> cutsBySequence;
	while (nullptr != fgets(szLine, sizeof(szLine), pFile))
	{
		p = szLine;
		std::string idToken;
		if (!Read_Token(p, idToken))
			continue;

		int32_t iSeqIndex = 0;
		std::vector<f32_t> cuts;
		std::string key, value;
		while (Read_Pair(p, key, value))
		{
			if ("seq" == key)
				iSeqIndex = atoi(value.c_str());
			else if ("cuts" == key)
			{
				size_t start = 0;
				while (start <= value.size())
				{
					const size_t comma = value.find(',', start);
					const std::string one = value.substr(start,
						std::string::npos == comma ?
							std::string::npos : comma - start);
					if (!one.empty())
						cuts.push_back(
							static_cast<f32_t>(atof(one.c_str())));
					if (std::string::npos == comma)
						break;
					start = comma + 1;
				}
			}
		}
		if (!cuts.empty())
			cutsBySequence[{ atoi(idToken.c_str()), iSeqIndex }] =
				std::move(cuts);
	}
	fclose(pFile);

	for (CLIP_SEQ& seq : m_ClipSeqs)
	{
		const auto found =
			cutsBySequence.find({ seq.iSkillId, seq.iSeqIndex });
		if (cutsBySequence.end() != found &&
			found->second.size() == seq.clips.size())
		{
			seq.cuts = found->second;
		}
	}
}

int32_t Client::CAnimation_Tool::Import_Notifies(const char_t* pClipName, f32_t fTickRate,
	int32_t& iShapedHits)
{
	iShapedHits = 0;

	const auto it = m_ClipNotify.find(pClipName);
	if (m_ClipNotify.end() == it)
		return 0;

	/* Drop only what a previous import put here so hand-authored rows survive. */
	for (auto e = m_Events.begin(); e != m_Events.end(); )
		e = (e->bImported && e->clipName == pClipName) ? m_Events.erase(e) : e + 1;

	const SKILL_TIMING* pRef = m_bImportKind[ETOI(EVENT_KIND::HIT)]
		? Find_ReferenceRow(pClipName) : nullptr;
	int32_t iHitOrdinal = nullptr != pRef ? Count_PrecedingChainHits(pClipName) : 0;

	int32_t iAdded = 0;
	for (const NOTIFY_ROW& row : it->second)
	{
		if (!m_bImportKind[ETOI(row.eKind)])
			continue;

		ANIM_EVENT evt{};
		evt.clipName = pClipName;
		evt.eKind = row.eKind;
		if (EVENT_KIND::EFFECT == evt.eKind)
			evt.eEffectReferenceKind =
				EFFECT_REFERENCE_KIND::SOURCE_REFERENCE;
		evt.bImported = true;
		evt.iStartMs = static_cast<int32_t>(row.fTime * 1000.f + 0.5f);
		evt.iEndMs = Is_Window(row.eKind)
			? evt.iStartMs + static_cast<int32_t>(row.fDuration * 1000.f + 0.5f)
			: evt.iStartMs;
		/* Point kinds name what they fire; a window instead carries the game's
		note on what it is for, which is the only thing distinguishing a move
		cancel from a dodge cancel. */
		evt.sPayload = Is_Window(row.eKind) ? row.sLabel : row.sAsset;

		/* The game spawns the same particle from several attach points; for
		authoring those collapse to one row. */
		bool_t bDuplicate = false;
		for (const ANIM_EVENT& other : m_Events)
		{
			if (other.bImported && other.clipName == evt.clipName &&
				other.eKind == evt.eKind && other.iStartMs == evt.iStartMs &&
				other.iEndMs == evt.iEndMs && other.sPayload == evt.sPayload)
			{
				bDuplicate = true;
				break;
			}
		}
		if (bDuplicate)
			continue;

		if (EVENT_KIND::HIT == evt.eKind && nullptr != pRef)
		{
			const int32_t iLast = static_cast<int32_t>(pRef->hits.size()) - 1;
			const SKILL_HIT& src = pRef->hits[iHitOrdinal < iLast ? iHitOrdinal : iLast];
			++iHitOrdinal;
			if (src.hit.iAreaType > 0)
			{
				evt.hit = src.hit;
				++iShapedHits;
			}
		}

		m_Events.push_back(evt);
		++iAdded;
	}

	m_iSelectedEvent = -1;
	m_bDirty = true;
	return iAdded;
}

bool_t Client::CAnimation_Tool::Load_SkillReference()
{
	m_SkillRef.clear();

	const std::string path = Get_SkillReferencePath();

	FILE* pFile = nullptr;
	if (0 != fopen_s(&pFile, path.c_str(), "r") || nullptr == pFile)
		return false;

	char_t szLine[2048]{};
	if (nullptr == fgets(szLine, sizeof(szLine), pFile))
	{
		fclose(pFile);
		return false;
	}

	const char_t* p = szLine;
	std::string magic, versionToken, owner, countToken;
	if (!Read_Token(p, magic) || !Read_Token(p, versionToken) ||
		!Read_Quoted(p, owner) || !Read_Token(p, countToken) ||
		magic != "LOSTARK_SKILL_TIMING")
	{
		fclose(pFile);
		return false;
	}

	/* Set once the row being read gets a hit line of its own, so the windows read
	off the skill line are only a stand-in for v1 files. */
	bool_t bCurrentHasDetail = false;

	while (nullptr != fgets(szLine, sizeof(szLine), pFile))
	{
		p = szLine;

		std::string idToken, name;
		if (!Read_Token(p, idToken))
			continue;

		/* A v2 "hit" continuation line details the skill line above it. v1 files
		have none, and an older build drops them because they carry no quoted
		name where it expects one. */
		if (("hit" == idToken || "shape" == idToken) && !m_SkillRef.empty())
		{
			const bool_t bShape = "shape" == idToken;

			if (!bCurrentHasDetail)
			{
				bCurrentHasDetail = true;
				m_SkillRef.back().hits.clear();
			}

			SKILL_HIT hit{};
			hit.bTimed = !bShape;
			HIT_PARAMS& hp = hit.hit;

			std::string key, value;
			while (Read_Pair(p, key, value))
			{
				const int32_t v = atoi(value.c_str());

				if ("t" == key)
					hit.iTimeMs = v;
				else if ("w" == key)
					hit.iWidthMs = v;
				else if ("timed" == key)
					hit.bTimed = (0 != v);
				else if ("rep" == key)
					hp.iRepeatCount = v;
				else if ("repms" == key)
					hp.iRepeatMs = v;
				else if ("fz" == key)
					hp.iFreezeMs = v;
				else if ("fzin" == key)
					hp.iFreezeInMs = v;
				else if ("fzout" == key)
					hp.iFreezeOutMs = v;
				else if ("push" == key)
					hp.iPushMs = v;
				else if ("pushr" == key)
					hp.iPushRange = v;
				else if ("area" == key)
					hp.iAreaType = v;
				else if ("ar" == key)
					hp.iAreaRange = v;
				else if ("aa" == key)
					hp.iAreaAngle = v;
				else if ("ah" == key)
					hp.iAreaHeight = v;
				else if ("ax" == key)
					hp.iAreaOffsetX = v;
				else if ("arem" == key)
					hp.iAreaInner = v;
				else if ("maxt" == key)
					hp.iMaxTargets = v;
				else if ("pks" == key)
					hit.sSourceKeys = value;
			}

			if (hp.iRepeatCount < 1)
				hp.iRepeatCount = 1;

			if (bShape)
			{
				bool_t bDuplicate = false;
				for (const SKILL_HIT& other : m_SkillRef.back().hits)
				{
					const HIT_PARAMS& op = other.hit;
					if (!other.bTimed &&
						op.iAreaType == hp.iAreaType &&
						op.iAreaRange == hp.iAreaRange &&
						op.iAreaAngle == hp.iAreaAngle &&
						op.iAreaHeight == hp.iAreaHeight &&
						op.iAreaOffsetX == hp.iAreaOffsetX &&
						op.iAreaInner == hp.iAreaInner &&
						op.iMaxTargets == hp.iMaxTargets &&
						op.iRepeatCount == hp.iRepeatCount &&
						op.iRepeatMs == hp.iRepeatMs &&
						op.iFreezeMs == hp.iFreezeMs &&
						op.iFreezeInMs == hp.iFreezeInMs &&
						op.iFreezeOutMs == hp.iFreezeOutMs &&
						op.iPushMs == hp.iPushMs &&
						op.iPushRange == hp.iPushRange)
					{
						bDuplicate = true;
						break;
					}
				}
				if (bDuplicate)
					continue;
			}

			m_SkillRef.back().hits.push_back(hit);
			continue;
		}

		if (!Read_Quoted(p, name))
			continue;

		SKILL_TIMING row{};
		row.iSkillId = atoi(idToken.c_str());
		row.name = name;

		/* v1 has no hit lines, so its windows stand in as bare hits with no combat
		detail. A following hit line replaces them. */
		std::string key, value;
		while (Read_Pair(p, key, value))
		{
			if ("sa" == key)
				row.iSuperArmor = atoi(value.c_str());
			else if ("move" == key)
				row.iMoveSpeed = atoi(value.c_str());
			else if ("base" == key)
				row.iBaseSkillId = atoi(value.c_str());
			else if ("hits" == key)
			{
				/* value is "a-b,c-d": comma-separated second windows. */
				size_t start = 0;
				while (start <= value.size())
				{
					const size_t comma = value.find(',', start);
					const std::string span = value.substr(start,
						std::string::npos == comma ? std::string::npos : comma - start);

					const size_t dash = span.find('-');
					if (std::string::npos != dash)
					{
						const f32_t a = static_cast<f32_t>(atof(span.substr(0, dash).c_str()));
						const f32_t b = static_cast<f32_t>(atof(span.substr(dash + 1).c_str()));

						SKILL_HIT hit{};
						hit.iTimeMs = static_cast<int32_t>(a * 1000.f + 0.5f);
						hit.iWidthMs = static_cast<int32_t>((b - a) * 1000.f + 0.5f);
						row.hits.push_back(hit);
					}

					if (std::string::npos == comma)
						break;
					start = comma + 1;
				}
			}
		}

		m_SkillRef.push_back(row);
		bCurrentHasDetail = false;
	}

	fclose(pFile);
	return true;
}
