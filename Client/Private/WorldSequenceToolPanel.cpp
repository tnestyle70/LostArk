#include "imgui.h"

#include "WorldSequenceToolPanel.h"

#include "GameInstance.h"
#include "MapAssetCatalog.h"
#include "MapAssetObject.h"
#include "MapStaticBatchObject.h"
#include "Model.h"

#include <algorithm>
#include <charconv>
#include <cctype>
#include <cmath>
#include <cstring>
#include <fstream>
#include <limits>
#include <new>

namespace
{
	using namespace Client;
	using namespace Engine;

	constexpr f32_t RAD_TO_DEG = 180.f / DirectX::XM_PI;
	constexpr f32_t DEG_TO_RAD = DirectX::XM_PI / 180.f;

	void ShowKoreanHelp(const char_t* text)
	{
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
			ImGui::SetTooltip("%s", text);
	}

	bool_t MatchesFilter(const std::string& text, const char_t* filter)
	{
		if (nullptr == filter || '\0' == *filter)
			return true;
		std::string haystack = text;
		std::string needle = filter;
		std::transform(haystack.begin(), haystack.end(), haystack.begin(),
			[](const unsigned char value)
			{
				return static_cast<char_t>(std::tolower(value));
			});
		std::transform(needle.begin(), needle.end(), needle.begin(),
			[](const unsigned char value)
			{
				return static_cast<char_t>(std::tolower(value));
			});
		return std::string::npos != haystack.find(needle);
	}

	f32_t Clamp01(const f32_t value)
	{
		return (std::max)(0.f, (std::min)(1.f, value));
	}

	float3_t QuaternionToEulerDegrees(const float4_t& value)
	{
		const f32_t x = value.x;
		const f32_t y = value.y;
		const f32_t z = value.z;
		const f32_t w = value.w;
		const f32_t pitch = std::atan2(
			2.f * (w * x + y * z), 1.f - 2.f * (x * x + y * y));
		const f32_t yaw = std::asin((std::max)(-1.f,
			(std::min)(1.f, 2.f * (w * y - z * x))));
		const f32_t roll = std::atan2(
			2.f * (w * z + x * y), 1.f - 2.f * (y * y + z * z));
		return float3_t(pitch * RAD_TO_DEG, yaw * RAD_TO_DEG,
			roll * RAD_TO_DEG);
	}

	float4_t EulerDegreesToQuaternion(const float3_t& value)
	{
		float4_t result;
		vector_t quaternion = XMQuaternionRotationRollPitchYaw(
			value.x * DEG_TO_RAD, value.y * DEG_TO_RAD, value.z * DEG_TO_RAD);
		quaternion = XMQuaternionNormalize(quaternion);
		if (XMVectorGetW(quaternion) < 0.f)
			quaternion = XMVectorNegate(quaternion);
		XMStoreFloat4(&result, quaternion);
		return result;
	}

	void CopyToBuffer(
		char_t* destination,
		const size_t destinationSize,
		const std::string& source)
	{
		if (nullptr == destination || 0u == destinationSize)
			return;
		strncpy_s(destination, destinationSize, source.c_str(), _TRUNCATE);
	}

	std::string TruncateUtf8ToByteCount(
		const std::string& value,
		const size_t maximumBytes)
	{
		if (value.size() <= maximumBytes)
			return value;
		size_t end = maximumBytes;
		while (end > 0u &&
			(0x80u == (static_cast<unsigned char>(value[end]) & 0xC0u)))
		{
			--end;
		}
		return value.substr(0u, end);
	}

	std::string MakeRotationEditorKey(
		const WORLD_SEQUENCE_TEMPLATE& sequence,
		const WORLD_SEQUENCE_TRACK& track,
		const WORLD_SEQUENCE_TRANSFORM_KEY& key)
	{
		return sequence.sequenceId + "/" + track.slotId + "/" +
			std::to_string(key.timeMs);
	}

	bool_t IsSequenceTargetSupported(
		const CMapAssetCatalog& catalog,
		const MAP_PLACEMENT_RECORD& placement)
	{
		const MAP_ASSET_ENTRY* asset = catalog.Find(placement.assetId);
		return nullptr != asset &&
			MAP_ASSET_RENDER_MODE::BACKGROUND !=
				asset->renderProfile.renderMode;
	}

	bool_t ReadBoundedAuthoringBytes(
		const std::filesystem::path& path,
		const uintmax_t maximumBytes,
		const bool_t allowMissing,
		bool_t& outExists,
		std::string& outBytes,
		std::string& outStatus)
	{
		outExists = false;
		outBytes.clear();
		if (path.empty())
		{
			outStatus = "Authoring source path is empty";
			return false;
		}
		std::error_code error;
		if (!std::filesystem::exists(path, error))
		{
			if (error || !allowMissing)
			{
				outStatus = error ? "Could not inspect authoring source" :
					"Required authoring source is missing";
				return false;
			}
			return true;
		}
		if (!std::filesystem::is_regular_file(path, error) || error)
		{
			outStatus = "Authoring source is not a regular file";
			return false;
		}
		const uintmax_t byteCount = std::filesystem::file_size(path, error);
		if (error || byteCount > maximumBytes)
		{
			outStatus = error ? "Could not inspect authoring source size" :
				"Authoring source exceeds its bounded read limit";
			return false;
		}
		std::ifstream input(path, std::ios::binary);
		if (!input)
		{
			outStatus = "Could not open authoring source";
			return false;
		}
		try
		{
			outBytes.resize(static_cast<size_t>(byteCount));
		}
		catch (const std::bad_alloc&)
		{
			outStatus = "Could not allocate bounded authoring source input";
			return false;
		}
		if (!outBytes.empty())
			input.read(outBytes.data(),
				static_cast<std::streamsize>(outBytes.size()));
		if (input.bad() ||
			input.gcount() != static_cast<std::streamsize>(outBytes.size()) ||
			std::char_traits<char_t>::eof() != input.peek())
		{
			outStatus = "Authoring source changed or failed while reading";
			return false;
		}
		outExists = true;
		return true;
	}

	std::vector<DEPLOY_PROP_ANIMATION_CLIP> GetValidAnimationClips(
		const shared_ptr<CDeployPropObject>& object)
	{
		std::vector<DEPLOY_PROP_ANIMATION_CLIP> result;
		if (nullptr == object || object->Is_StaticDeployModel())
			return result;
		for (const DEPLOY_PROP_ANIMATION_CLIP& clip :
			object->Get_AnimationClips())
		{
			if (!clip.name.empty() && std::isfinite(clip.durationSeconds) &&
				clip.durationSeconds > 0.f)
			{
				result.push_back(clip);
			}
		}
		return result;
	}
}

Client::WORLD_SEQUENCE_PLACEMENT_MAP
Client::CWorldSequenceToolPanel::Collect_Placements(
	const CMapAssetCatalog& catalog,
	const std::vector<MAP_PLACEMENT_RECORD>& placements)
{
	WORLD_SEQUENCE_PLACEMENT_MAP result;
	result.reserve(placements.size());
	for (const MAP_PLACEMENT_RECORD& placement : placements)
	{
		result.emplace(placement.placementId,
			WORLD_SEQUENCE_PLACEMENT_INFO{
				placement.signedScale,
				IsSequenceTargetSupported(catalog, placement) });
	}
	return result;
}

Client::WORLD_SEQUENCE_DEPLOY_MAP
Client::CWorldSequenceToolPanel::Collect_DeployPlacements(
	const CDeployPropRuntime& deployRuntime)
{
	WORLD_SEQUENCE_DEPLOY_MAP result;
	result.reserve(deployRuntime.Get_Entries().size());
	for (const DEPLOY_RUNTIME_ENTRY& entry : deployRuntime.Get_Entries())
	{
		WORLD_SEQUENCE_DEPLOY_INFO info;
		if (nullptr != entry.object && !entry.object->Is_StaticDeployModel())
		{
			const std::vector<DEPLOY_PROP_ANIMATION_CLIP> clips =
				GetValidAnimationClips(entry.object);
			info.animationClips.reserve(clips.size());
			for (const DEPLOY_PROP_ANIMATION_CLIP& clip : clips)
				info.animationClips.push_back(clip.name);
			info.animationTargetSupported = !info.animationClips.empty();
		}
		result.emplace(entry.placement.runtimePlacementId, std::move(info));
	}
	return result;
}

bool_t Client::CWorldSequenceToolPanel::Try_ParseTargetId(
	const WORLD_SEQUENCE_BINDING& binding,
	uint64_t& outTargetId)
{
	if (binding.targetId.empty())
		return false;
	const char_t* const begin = binding.targetId.data();
	const char_t* const end = begin + binding.targetId.size();
	const auto parsed = std::from_chars(begin, end, outTargetId);
	return std::errc{} == parsed.ec && parsed.ptr == end && 0u != outTargetId;
}

Client::WORLD_SEQUENCE_PLACEMENT_MAP
Client::CWorldSequenceToolPanel::Collect_Placements(
	const CMapAssetCatalog& catalog,
	const std::vector<MAP_RUNTIME_PLACED_ENTRY>& placements)
{
	WORLD_SEQUENCE_PLACEMENT_MAP result;
	result.reserve(placements.size());
	for (const MAP_RUNTIME_PLACED_ENTRY& placement : placements)
	{
		result.emplace(placement.record.placementId,
			WORLD_SEQUENCE_PLACEMENT_INFO{
				placement.record.signedScale,
				IsSequenceTargetSupported(catalog, placement.record) });
	}
	return result;
}

Client::MAP_RUNTIME_PLACED_ENTRY*
Client::CWorldSequenceToolPanel::Find_Placement(
	std::vector<MAP_RUNTIME_PLACED_ENTRY>& placements,
	const uint64_t placementId)
{
	const auto found = std::find_if(placements.begin(), placements.end(),
		[placementId](const MAP_RUNTIME_PLACED_ENTRY& value)
		{
			return value.record.placementId == placementId;
		});
	return placements.end() == found ? nullptr : &*found;
}

const Client::WORLD_SEQUENCE_TRACK*
Client::CWorldSequenceToolPanel::Find_Track(
	const WORLD_SEQUENCE_TEMPLATE& sequence,
	const std::string& slotId)
{
	const auto found = std::find_if(sequence.tracks.begin(), sequence.tracks.end(),
		[&slotId](const WORLD_SEQUENCE_TRACK& value)
		{
			return value.slotId == slotId;
		});
	return sequence.tracks.end() == found ? nullptr : &*found;
}

Client::WORLD_SEQUENCE_TRANSFORM_KEY
Client::CWorldSequenceToolPanel::Sample_Track(
	const WORLD_SEQUENCE_TEMPLATE& sequence,
	const WORLD_SEQUENCE_TRACK& track,
	const f32_t timeMs)
{
	if (track.keys.empty())
		return {};
	if (timeMs <= static_cast<f32_t>(track.keys.front().timeMs))
		return track.keys.front();
	if (timeMs >= static_cast<f32_t>(track.keys.back().timeMs))
		return track.keys.back();
	const auto right = std::upper_bound(track.keys.begin(), track.keys.end(), timeMs,
		[](const f32_t time, const WORLD_SEQUENCE_TRANSFORM_KEY& key)
		{
			return time < static_cast<f32_t>(key.timeMs);
		});
	const WORLD_SEQUENCE_TRANSFORM_KEY& left = *(right - 1);
	const f32_t span = static_cast<f32_t>(right->timeMs - left.timeMs);
	f32_t factor = span <= 0.f ? 0.f :
		(timeMs - static_cast<f32_t>(left.timeMs)) / span;
	factor = Clamp01(factor);
	if (WORLD_SEQUENCE_INTERPOLATION::SMOOTH_STEP == sequence.interpolation)
		factor = factor * factor * (3.f - 2.f * factor);

	WORLD_SEQUENCE_TRANSFORM_KEY result;
	result.timeMs = static_cast<uint32_t>((std::max)(0.f, timeMs));
	XMStoreFloat3(&result.positionOffset,
		XMVectorLerp(XMLoadFloat3(&left.positionOffset),
			XMLoadFloat3(&right->positionOffset), factor));
	XMStoreFloat4(&result.rotationQuaternion,
		XMQuaternionNormalize(XMQuaternionSlerp(
			XMLoadFloat4(&left.rotationQuaternion),
			XMLoadFloat4(&right->rotationQuaternion), factor)));
	if (result.rotationQuaternion.w < 0.f)
	{
		result.rotationQuaternion.x = -result.rotationQuaternion.x;
		result.rotationQuaternion.y = -result.rotationQuaternion.y;
		result.rotationQuaternion.z = -result.rotationQuaternion.z;
		result.rotationQuaternion.w = -result.rotationQuaternion.w;
	}
	XMStoreFloat3(&result.scaleMultiplier,
		XMVectorLerp(XMLoadFloat3(&left.scaleMultiplier),
			XMLoadFloat3(&right->scaleMultiplier), factor));
	result.visible = left.visible;
	return result;
}

bool_t Client::CWorldSequenceToolPanel::Load_Area(
	const std::filesystem::path& path,
	const std::filesystem::path& placementPath,
	const std::string& areaId,
	const CMapAssetCatalog& catalog,
	const std::vector<MAP_PLACEMENT_RECORD>& placements,
	const CDeployPropRuntime& deployRuntime,
	std::string& outStatus)
{
	bool_t placementExistsBefore = false;
	bool_t sequenceExistsBefore = false;
	std::string placementBefore;
	std::string sequenceBefore;
	if (!ReadBoundedAuthoringBytes(placementPath, 64u * 1024u * 1024u,
		false, placementExistsBefore, placementBefore, outStatus) ||
		!placementExistsBefore ||
		!ReadBoundedAuthoringBytes(path, 16u * 1024u * 1024u,
		true, sequenceExistsBefore, sequenceBefore, outStatus))
	{
		return false;
	}
	CWorldSequenceDocument staged;
	if (!staged.Load(path, areaId,
		Collect_Placements(catalog, placements),
		Collect_DeployPlacements(deployRuntime), outStatus))
		return false;
	bool_t placementExistsAfter = false;
	bool_t sequenceExistsAfter = false;
	std::string placementAfter;
	std::string sequenceAfter;
	if (!ReadBoundedAuthoringBytes(placementPath, 64u * 1024u * 1024u,
		false, placementExistsAfter, placementAfter, outStatus) ||
		!placementExistsAfter ||
		!ReadBoundedAuthoringBytes(path, 16u * 1024u * 1024u,
		true, sequenceExistsAfter, sequenceAfter, outStatus))
	{
		return false;
	}
	if (placementBefore != placementAfter ||
		sequenceExistsBefore != sequenceExistsAfter ||
		sequenceBefore != sequenceAfter)
	{
		outStatus =
			"Linked map/sequence source changed while the Area was loading";
		return false;
	}
	m_Document = std::move(staged);
	m_Path = path;
	m_PlacementPath = placementPath;
	m_PlacementBaselineBytes = std::move(placementAfter);
	m_SequenceBaselineBytes = std::move(sequenceAfter);
	m_bSequenceBaselineExists = sequenceExistsAfter;
	m_bDirty = false;
	m_bSaveAllRequested = false;
	m_bReloadAllRequested = false;
	m_SelectedTemplateId.clear();
	m_SelectedInstanceId.clear();
	m_iSelectedTrack = 0;
	m_iSelectedAnimationTrack = 0;
	m_iSelectedKey = 0;
	m_RotationEditorKey.clear();
	m_fPreviewTimeMs = 0.f;
	m_bPreviewActive = false;
	m_bPlaying = false;
	m_bPreviewNeedsRefresh = false;
	m_PreviewTargets.clear();
	m_PreviewDeployTargets.clear();
	m_PreviewModels.clear();
	m_CachedObjectFilter.clear();
	m_iCachedObjectPlacementCount = SIZE_MAX;
	m_iCachedObjectFingerprint = 0;
	m_FilteredObjectIndices.clear();
	m_iSelectedDeployPlacementId = 0;
	if (!m_Document.Get_Instances().empty())
	{
		m_SelectedInstanceId = m_Document.Get_Instances().front().instanceId;
		m_SelectedTemplateId = m_Document.Get_Instances().front().templateId;
	}
	else if (!m_Document.Get_Templates().empty())
	{
		m_SelectedTemplateId = m_Document.Get_Templates().front().sequenceId;
	}
	m_Status = outStatus;
	return true;
}

bool_t Client::CWorldSequenceToolPanel::Save(
	const CMapAssetCatalog& catalog,
	const std::vector<MAP_RUNTIME_PLACED_ENTRY>& placements,
	const CDeployPropRuntime& deployRuntime,
	std::string& outStatus)
{
	if (!Is_Ready())
	{
		outStatus = "World sequence document is not ready";
		return false;
	}
	if (!m_Document.Save(m_Path,
		Collect_Placements(catalog, placements),
		Collect_DeployPlacements(deployRuntime), outStatus))
	{
		m_Status = outStatus;
		return false;
	}
	m_bDirty = false;
	m_bSaveAllRequested = false;
	m_Status = outStatus;
	return true;
}

bool_t Client::CWorldSequenceToolPanel::Validate(
	const CMapAssetCatalog& catalog,
	const std::vector<MAP_RUNTIME_PLACED_ENTRY>& placements,
	const CDeployPropRuntime& deployRuntime,
	std::string& outStatus) const
{
	return m_Document.Validate(
		Collect_Placements(catalog, placements),
		Collect_DeployPlacements(deployRuntime), outStatus);
}

bool_t Client::CWorldSequenceToolPanel::Matches_LinkedSourceBaseline(
	std::string& outStatus) const
{
	bool_t placementExists = false;
	bool_t sequenceExists = false;
	std::string placementBytes;
	std::string sequenceBytes;
	if (!ReadBoundedAuthoringBytes(m_PlacementPath, 64u * 1024u * 1024u,
		false, placementExists, placementBytes, outStatus) ||
		!placementExists ||
		!ReadBoundedAuthoringBytes(m_Path, 16u * 1024u * 1024u,
		true, sequenceExists, sequenceBytes, outStatus))
	{
		return false;
	}
	if (placementBytes != m_PlacementBaselineBytes ||
		sequenceExists != m_bSequenceBaselineExists ||
		sequenceBytes != m_SequenceBaselineBytes)
	{
		outStatus =
			"Save conflict: linked map/sequence source changed after Reload";
		return false;
	}
	outStatus = "Linked authoring source still matches the loaded baseline";
	return true;
}

void Client::CWorldSequenceToolPanel::Adopt_VerifiedLinkedSourceBaseline(
	CWorldSequenceToolPanel& verified) noexcept
{
	m_PlacementBaselineBytes.swap(verified.m_PlacementBaselineBytes);
	m_SequenceBaselineBytes.swap(verified.m_SequenceBaselineBytes);
	m_bSequenceBaselineExists = verified.m_bSequenceBaselineExists;
}

void Client::CWorldSequenceToolPanel::Update(
	const f32_t timeDelta,
	const bool_t isPanelActive,
	const uint32_t authoringLevelIndex,
	const CMapAssetCatalog& catalog,
	std::vector<MAP_RUNTIME_PLACED_ENTRY>& placements,
	CDeployPropRuntime& deployRuntime)
{
	if (!isPanelActive)
	{
		if (m_bPreviewActive)
			Stop_AndRestore(authoringLevelIndex, catalog, placements,
				deployRuntime);
		return;
	}
	if (m_bPreviewActive && m_bPreviewNeedsRefresh)
	{
		std::string validation;
		if (!Validate(catalog, placements, deployRuntime, validation))
		{
			Stop_AndRestore(authoringLevelIndex, catalog, placements,
				deployRuntime);
			if (!m_bPreviewActive)
				m_Status = "Preview stopped: " + validation;
			else
				m_Status += " | " + validation;
			return;
		}
		m_bPreviewNeedsRefresh = false;
		if (!m_bPlaying)
		{
			if (!Apply_Preview(authoringLevelIndex, catalog, placements,
				deployRuntime))
				Stop_AndRestore(authoringLevelIndex, catalog, placements,
					deployRuntime);
			return;
		}
	}
	if (!m_bPlaying)
		return;
	const WORLD_SEQUENCE_INSTANCE* instance =
		m_Document.Find_Instance(m_SelectedInstanceId);
	const WORLD_SEQUENCE_TEMPLATE* sequence = nullptr == instance ? nullptr :
		m_Document.Find_Template(instance->templateId);
	if (nullptr == instance || nullptr == sequence)
	{
		m_Status = "Preview selection no longer exists";
		Stop_AndRestore(authoringLevelIndex, catalog, placements, deployRuntime);
		return;
	}
	const f32_t speed = (std::max)(0.05f, instance->playbackSpeed);
	const f32_t totalMs = static_cast<f32_t>(instance->startDelayMs) +
		static_cast<f32_t>(sequence->durationMs) / speed;
	m_fPreviewTimeMs += (std::max)(0.f, timeDelta) * 1000.f;
	if (m_fPreviewTimeMs >= totalMs)
	{
		if (m_bLoop && totalMs > 0.f)
			m_fPreviewTimeMs = std::fmod(m_fPreviewTimeMs, totalMs);
		else
		{
			m_fPreviewTimeMs = totalMs;
			m_bPlaying = false;
		}
	}
	if (!Apply_Preview(authoringLevelIndex, catalog, placements, deployRuntime))
		Stop_AndRestore(authoringLevelIndex, catalog, placements, deployRuntime);
	else
		m_bPreviewNeedsRefresh = false;
}

void Client::CWorldSequenceToolPanel::Render(
	const bool_t isMapAuthoringLevel,
	const uint32_t authoringLevelIndex,
	const CMapAssetCatalog& catalog,
	std::vector<MAP_RUNTIME_PLACED_ENTRY>& placements,
	uint64_t& selectedPlacementId,
	CDeployPropRuntime& deployRuntime)
{
	if (!isMapAuthoringLevel || !Is_Ready())
	{
		ImGui::TextDisabled("World Sequence authoring is unavailable for this Area.");
		return;
	}
	ImGui::TextUnformatted("World Sequence Library");
	ImGui::SameLine();
	ImGui::TextDisabled("Authoring preview only; product runtime publish is separate.");
	ImGui::TextDisabled(
		"현재는 맵툴 저작과 미리보기 단계이며, 서버 상호작용·제품 재생·길 개방은 아직 연결되지 않았습니다.");
	ImGui::TextWrapped("File: %s", m_Path.filename().string().c_str());
	Render_DocumentToolbar(authoringLevelIndex, catalog, placements,
		deployRuntime);
	ImGui::TextWrapped("%s", m_Status.c_str());
	ImGui::TextDisabled(
		"연출 템플릿을 한 번 만든 뒤 여러 맵 오브젝트에 배치해서 재사용할 수 있습니다.");

	if (ImGui::BeginTable("##world-sequence-library", 4,
		ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable |
		ImGuiTableFlags_SizingStretchProp))
	{
		ImGui::TableSetupColumn("Sequence List", ImGuiTableColumnFlags_WidthStretch, 0.9f);
		ImGui::TableSetupColumn("Placed Instances", ImGuiTableColumnFlags_WidthStretch, 0.9f);
		ImGui::TableSetupColumn("Map Objects", ImGuiTableColumnFlags_WidthStretch, 1.2f);
		ImGui::TableSetupColumn("Animated Props", ImGuiTableColumnFlags_WidthStretch, 1.2f);
		ImGui::TableHeadersRow();
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		Render_TemplateList();
		ImGui::TableSetColumnIndex(1);
		Render_InstanceList();
		ImGui::TableSetColumnIndex(2);
		Render_ObjectBrowser(catalog, placements, selectedPlacementId);
		ImGui::TableSetColumnIndex(3);
		Render_DeployBrowser(deployRuntime);
		ImGui::EndTable();
	}

	ImGui::Separator();
	Render_TemplateEditor(authoringLevelIndex, catalog, placements,
		selectedPlacementId, deployRuntime);
}

void Client::CWorldSequenceToolPanel::Stop_AndRestore(
	const uint32_t authoringLevelIndex,
	const CMapAssetCatalog& catalog,
	std::vector<MAP_RUNTIME_PLACED_ENTRY>& placements,
	CDeployPropRuntime& deployRuntime)
{
	bool_t restored = true;
	if (m_bPreviewActive)
	{
		for (const PREVIEW_TARGET& target : m_PreviewTargets)
		{
			MAP_RUNTIME_PLACED_ENTRY* entry =
				Find_Placement(placements, target.placementId);
			MAP_PLACEMENT_RECORD restoredRecord = target.baseline;
			restoredRecord.visible = target.runtimeVisible;
			if (nullptr == entry || !Apply_RuntimeRecord(
				authoringLevelIndex, catalog, *entry, restoredRecord))
			{
				restored = false;
			}
		}
		for (const PREVIEW_DEPLOY_TARGET& target : m_PreviewDeployTargets)
		{
			const shared_ptr<CDeployPropObject> object =
				deployRuntime.Find(target.runtimePlacementId);
			if (nullptr == object)
				restored = false;
			else
				object->End_AnimationAuthoringPreview();
		}
	}
	m_bPlaying = false;
	m_bPreviewNeedsRefresh = false;
	if (restored)
	{
		m_bPreviewActive = false;
		m_PreviewTargets.clear();
		m_PreviewDeployTargets.clear();
	}
	else
	{
		m_Status = "Preview restore failed; targets remain owned for retry";
	}
}

void Client::CWorldSequenceToolPanel::Reset()
{
	m_Document = {};
	m_Path.clear();
	m_PlacementPath.clear();
	m_PlacementBaselineBytes.clear();
	m_SequenceBaselineBytes.clear();
	m_bSequenceBaselineExists = false;
	m_bDirty = false;
	m_bSaveAllRequested = false;
	m_bReloadAllRequested = false;
	m_Status = "Select an Area to edit world sequences";
	m_SelectedTemplateId.clear();
	m_SelectedInstanceId.clear();
	m_iSelectedTrack = 0;
	m_iSelectedAnimationTrack = 0;
	m_iSelectedKey = 0;
	m_RotationEditorKey.clear();
	m_bPreviewActive = false;
	m_bPlaying = false;
	m_bPreviewNeedsRefresh = false;
	m_fPreviewTimeMs = 0.f;
	m_PreviewTargets.clear();
	m_PreviewDeployTargets.clear();
	m_PreviewModels.clear();
	m_iSelectedDeployPlacementId = 0;
	m_CachedObjectFilter.clear();
	m_iCachedObjectPlacementCount = SIZE_MAX;
	m_iCachedObjectFingerprint = 0;
	m_FilteredObjectIndices.clear();
}

bool_t Client::CWorldSequenceToolPanel::Begin_Preview(
	const uint32_t authoringLevelIndex,
	const CMapAssetCatalog& catalog,
	std::vector<MAP_RUNTIME_PLACED_ENTRY>& placements,
	CDeployPropRuntime& deployRuntime)
{
	Stop_AndRestore(authoringLevelIndex, catalog, placements, deployRuntime);
	if (m_bPreviewActive)
		return false;
	std::string validation;
	if (!Validate(catalog, placements, deployRuntime, validation))
	{
		m_Status = validation;
		return false;
	}
	const WORLD_SEQUENCE_INSTANCE* instance =
		m_Document.Find_Instance(m_SelectedInstanceId);
	if (nullptr == instance)
	{
		m_Status = "Select a placed instance before preview";
		return false;
	}
	const auto abandonCapturedTargets = [&]()
	{
		for (const PREVIEW_DEPLOY_TARGET& captured :
			m_PreviewDeployTargets)
		{
			const shared_ptr<CDeployPropObject> object =
				deployRuntime.Find(captured.runtimePlacementId);
			if (nullptr != object)
				object->End_AnimationAuthoringPreview();
		}
		m_PreviewTargets.clear();
		m_PreviewDeployTargets.clear();
	};
	for (const WORLD_SEQUENCE_BINDING& binding : instance->bindings)
	{
		uint64_t targetId = 0;
		if (!Try_ParseTargetId(binding, targetId))
		{
			m_Status = "Preview binding target ID is invalid";
			abandonCapturedTargets();
			return false;
		}
		if (WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT == binding.targetKind)
		{
			const shared_ptr<CDeployPropObject> object =
				deployRuntime.Find(targetId);
			if (nullptr == object ||
				!object->Begin_AnimationAuthoringPreview())
			{
				m_Status = "Preview could not capture animated Deploy state";
				abandonCapturedTargets();
				return false;
			}
			m_PreviewDeployTargets.push_back({ targetId });
			continue;
		}
		MAP_RUNTIME_PLACED_ENTRY* entry =
			Find_Placement(placements, targetId);
		if (nullptr == entry)
		{
			m_Status = "Preview binding target is missing";
			abandonCapturedTargets();
			return false;
		}
		bool_t runtimeVisible = false;
		if (!CMapPlacementRuntime::Try_GetRuntimeVisible(*entry,
			runtimeVisible))
		{
			m_Status = "Preview could not capture runtime visibility";
			abandonCapturedTargets();
			return false;
		}
		m_PreviewTargets.push_back(
			{ targetId, entry->record, runtimeVisible });
	}
	m_bPreviewActive = true;
	m_bPreviewNeedsRefresh = false;
	if (!Apply_Preview(authoringLevelIndex, catalog, placements, deployRuntime))
	{
		Stop_AndRestore(authoringLevelIndex, catalog, placements, deployRuntime);
		return false;
	}
	return true;
}

bool_t Client::CWorldSequenceToolPanel::Apply_Preview(
	const uint32_t authoringLevelIndex,
	const CMapAssetCatalog& catalog,
	std::vector<MAP_RUNTIME_PLACED_ENTRY>& placements,
	CDeployPropRuntime& deployRuntime)
{
	if (!m_bPreviewActive)
		return false;
	const WORLD_SEQUENCE_INSTANCE* instance =
		m_Document.Find_Instance(m_SelectedInstanceId);
	const WORLD_SEQUENCE_TEMPLATE* sequence = nullptr == instance ? nullptr :
		m_Document.Find_Template(instance->templateId);
	if (nullptr == instance || nullptr == sequence)
	{
		m_Status = "Preview sequence is missing";
		return false;
	}
	const f32_t delayedMs = m_fPreviewTimeMs -
		static_cast<f32_t>(instance->startDelayMs);
	const f32_t localMs = delayedMs <= 0.f ? 0.f :
		(std::min)(static_cast<f32_t>(sequence->durationMs),
			delayedMs * instance->playbackSpeed);
	for (const WORLD_SEQUENCE_BINDING& binding : instance->bindings)
	{
		uint64_t targetId = 0;
		if (!Try_ParseTargetId(binding, targetId))
		{
			m_Status = "Preview target ID changed while playing";
			return false;
		}
		if (WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT == binding.targetKind)
		{
			const auto animationTrack = std::find_if(
				sequence->animationTracks.begin(),
				sequence->animationTracks.end(),
				[&binding](const WORLD_SEQUENCE_ANIMATION_TRACK& track)
				{
					return track.slotId == binding.slotId;
				});
			const auto baseline = std::find_if(m_PreviewDeployTargets.begin(),
				m_PreviewDeployTargets.end(),
				[targetId](const PREVIEW_DEPLOY_TARGET& value)
				{
					return value.runtimePlacementId == targetId;
				});
			const shared_ptr<CDeployPropObject> object =
				deployRuntime.Find(targetId);
			if (sequence->animationTracks.end() == animationTrack ||
				m_PreviewDeployTargets.end() == baseline || nullptr == object)
			{
				m_Status = "Animated Deploy target changed while playing";
				return false;
			}
			f32_t normalized = 0.f;
			if (delayedMs >= 0.f)
			{
				const std::vector<DEPLOY_PROP_ANIMATION_CLIP> clips =
					GetValidAnimationClips(object);
				const auto clip = std::find_if(clips.begin(), clips.end(),
					[&animationTrack](const DEPLOY_PROP_ANIMATION_CLIP& value)
					{
						return value.name == animationTrack->clipName;
					});
				if (clips.end() == clip ||
					!std::isfinite(clip->durationSeconds) ||
					clip->durationSeconds <= 0.f)
				{
					m_Status = "Animated Deploy clip duration is unavailable";
					return false;
				}
				normalized = localMs * animationTrack->playbackRate /
					(clip->durationSeconds * 1000.f);
				if (animationTrack->loop)
					normalized = std::fmod(normalized, 1.f);
				else if (normalized > 1.f)
					normalized = animationTrack->holdLastFrame ? 1.f : 0.f;
			}
			if (!object->Sample_AnimationAuthoringPreview(
				animationTrack->clipName, Clamp01(normalized),
				animationTrack->loop))
			{
				m_Status = "Animated Deploy sampling failed; original clip will be restored";
				return false;
			}
			continue;
		}
		const WORLD_SEQUENCE_TRACK* track = Find_Track(*sequence, binding.slotId);
		const auto baseline = std::find_if(m_PreviewTargets.begin(),
			m_PreviewTargets.end(),
			[targetId](const PREVIEW_TARGET& value)
			{
				return value.placementId == targetId;
			});
		MAP_RUNTIME_PLACED_ENTRY* entry =
			Find_Placement(placements, targetId);
		if (nullptr == track || m_PreviewTargets.end() == baseline || nullptr == entry)
		{
			m_Status = "Preview target changed while playing";
			return false;
		}
		if (delayedMs < 0.f)
		{
			MAP_PLACEMENT_RECORD restoredRecord = baseline->baseline;
			restoredRecord.visible = baseline->runtimeVisible;
			if (!Apply_RuntimeRecord(authoringLevelIndex, catalog, *entry,
				restoredRecord))
			{
				m_Status = "Preview start-delay baseline restore failed";
				return false;
			}
			continue;
		}
		const WORLD_SEQUENCE_TRANSFORM_KEY key =
			Sample_Track(*sequence, *track, localMs);
		MAP_PLACEMENT_RECORD sampled = baseline->baseline;
		const vector_t baselineRotation =
			XMLoadFloat4(&baseline->baseline.rotationQuaternion);
		float3_t rotatedOffset;
		XMStoreFloat3(&rotatedOffset,
			XMVector3Rotate(XMLoadFloat3(&key.positionOffset), baselineRotation));
		sampled.position.x += rotatedOffset.x;
		sampled.position.y += rotatedOffset.y;
		sampled.position.z += rotatedOffset.z;
		vector_t combined = XMQuaternionMultiply(
			XMLoadFloat4(&key.rotationQuaternion), baselineRotation);
		combined = XMQuaternionNormalize(combined);
		if (XMVectorGetW(combined) < 0.f)
			combined = XMVectorNegate(combined);
		XMStoreFloat4(&sampled.rotationQuaternion, combined);
		sampled.signedScale.x *= key.scaleMultiplier.x;
		sampled.signedScale.y *= key.scaleMultiplier.y;
		sampled.signedScale.z *= key.scaleMultiplier.z;
		const bool_t externalVisibilityAllowsShowing =
			baseline->runtimeVisible || !baseline->baseline.visible;
		sampled.visible = externalVisibilityAllowsShowing && key.visible;
		if (!Apply_RuntimeRecord(authoringLevelIndex, catalog, *entry, sampled))
		{
			m_Status = "Preview transform failed; original placement will be restored";
			return false;
		}
	}
	return true;
}

bool_t Client::CWorldSequenceToolPanel::Apply_RuntimeRecord(
	const uint32_t authoringLevelIndex,
	const CMapAssetCatalog& catalog,
	MAP_RUNTIME_PLACED_ENTRY& entry,
	const MAP_PLACEMENT_RECORD& record)
{
	bool_t transformed = false;
	if (nullptr != entry.object)
	{
		entry.object->Set_PlacementTransform(record.position,
			record.rotationQuaternion, record.signedScale);
		transformed = true;
	}
	else if (nullptr != entry.batch)
	{
		const MAP_ASSET_ENTRY* asset = catalog.Find(record.assetId);
		if (nullptr == asset)
			return false;
		shared_ptr<CModel>& model = m_PreviewModels[record.assetId];
		if (nullptr == model)
		{
			model = dynamic_pointer_cast<CModel>(
				CGameInstance::Get().Clone_Prototype(
					authoringLevelIndex, asset->prototypeTag));
		}
		FMapStaticInstance instance{};
		if (nullptr != model && SUCCEEDED(CMapPlacementRuntime::Build_StaticInstance(
			*asset, model, record, instance)) &&
			SUCCEEDED(entry.batch->Update_Instance(record.placementId, instance)))
		{
			transformed = true;
		}
	}
	return transformed &&
		CMapPlacementRuntime::Set_RuntimeVisible(entry, record.visible);
}

void Client::CWorldSequenceToolPanel::Create_Template()
{
	if (m_Document.Get_Templates().size() >=
		CWorldSequenceDocument::MAX_TEMPLATE_COUNT)
	{
		m_Status = "World sequence template limit reached";
		return;
	}
	WORLD_SEQUENCE_TEMPLATE value;
	value.sequenceId = Allocate_TemplateId();
	value.displayName = "New Sequence " +
		std::to_string(m_Document.Get_Templates().size() + 1u);
	WORLD_SEQUENCE_TRACK track;
	track.slotId = "object";
	track.keys.push_back({});
	WORLD_SEQUENCE_TRANSFORM_KEY end;
	end.timeMs = value.durationMs;
	track.keys.push_back(end);
	value.tracks.push_back(std::move(track));
	m_Document.Get_Templates().push_back(std::move(value));
	m_SelectedTemplateId = m_Document.Get_Templates().back().sequenceId;
	m_SelectedInstanceId.clear();
	m_iSelectedTrack = 0;
	m_iSelectedKey = 0;
	m_RotationEditorKey.clear();
	Mark_Dirty();
	m_Status = "Created reusable sequence template";
}

void Client::CWorldSequenceToolPanel::Create_AnimationTemplate(
	CDeployPropRuntime& deployRuntime)
{
	if (m_Document.Get_Templates().size() >=
		CWorldSequenceDocument::MAX_TEMPLATE_COUNT ||
		m_Document.Get_Instances().size() >=
		CWorldSequenceDocument::MAX_INSTANCE_COUNT)
	{
		m_Status = "World sequence template or instance limit reached";
		return;
	}
	const shared_ptr<CDeployPropObject> object =
		deployRuntime.Find(m_iSelectedDeployPlacementId);
	const std::vector<DEPLOY_PROP_ANIMATION_CLIP> clips =
		nullptr == object ? std::vector<DEPLOY_PROP_ANIMATION_CLIP>{} :
		GetValidAnimationClips(object);
	if (nullptr == object || clips.empty())
	{
		m_Status = "Select an animated prop with at least one clip first";
		return;
	}

	WORLD_SEQUENCE_TEMPLATE value;
	value.sequenceId = Allocate_TemplateId();
	value.displayName = "New Animation Sequence " +
		std::to_string(m_Document.Get_Templates().size() + 1u);
	value.category = "Animation";
	value.durationMs = (std::max)(1u, static_cast<uint32_t>(
		std::round(clips.front().durationSeconds * 1000.f)));
	WORLD_SEQUENCE_ANIMATION_TRACK track;
	track.slotId = "animated.prop";
	track.clipName = clips.front().name;
	value.animationTracks.push_back(std::move(track));

	WORLD_SEQUENCE_INSTANCE instance;
	instance.instanceId = Allocate_InstanceId(value.sequenceId);
	instance.templateId = value.sequenceId;
	instance.bindings.push_back({ "animated.prop",
		WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT,
		std::to_string(m_iSelectedDeployPlacementId) });
	m_SelectedTemplateId = value.sequenceId;
	m_SelectedInstanceId = instance.instanceId;
	m_Document.Get_Templates().push_back(std::move(value));
	m_Document.Get_Instances().push_back(std::move(instance));
	m_iSelectedTrack = 0;
	m_iSelectedKey = 0;
	m_RotationEditorKey.clear();
	Mark_Dirty();
	m_Status = "Created and bound reusable animation sequence";
}

void Client::CWorldSequenceToolPanel::Duplicate_SelectedTemplate()
{
	if (m_Document.Get_Templates().size() >=
		CWorldSequenceDocument::MAX_TEMPLATE_COUNT)
	{
		m_Status = "World sequence template limit reached";
		return;
	}
	const WORLD_SEQUENCE_TEMPLATE* selected =
		m_Document.Find_Template(m_SelectedTemplateId);
	if (nullptr == selected)
		return;
	WORLD_SEQUENCE_TEMPLATE copy = *selected;
	copy.sequenceId = Allocate_TemplateId();
	constexpr const char_t* COPY_SUFFIX = " Copy";
	copy.displayName = TruncateUtf8ToByteCount(copy.displayName,
		128u - std::char_traits<char_t>::length(COPY_SUFFIX)) + COPY_SUFFIX;
	m_Document.Get_Templates().push_back(std::move(copy));
	m_SelectedTemplateId = m_Document.Get_Templates().back().sequenceId;
	m_SelectedInstanceId.clear();
	m_iSelectedTrack = 0;
	m_iSelectedKey = 0;
	m_RotationEditorKey.clear();
	Mark_Dirty();
	m_Status = "Duplicated sequence template without duplicating bindings";
}

void Client::CWorldSequenceToolPanel::Delete_SelectedTemplate()
{
	auto& instances = m_Document.Get_Instances();
	instances.erase(std::remove_if(instances.begin(), instances.end(),
		[this](const WORLD_SEQUENCE_INSTANCE& value)
		{
			return value.templateId == m_SelectedTemplateId;
		}), instances.end());
	auto& templates = m_Document.Get_Templates();
	templates.erase(std::remove_if(templates.begin(), templates.end(),
		[this](const WORLD_SEQUENCE_TEMPLATE& value)
		{
			return value.sequenceId == m_SelectedTemplateId;
		}), templates.end());
	m_SelectedTemplateId = templates.empty() ? std::string{} :
		templates.front().sequenceId;
	m_SelectedInstanceId.clear();
	m_iSelectedTrack = 0;
	m_iSelectedKey = 0;
	m_RotationEditorKey.clear();
	Mark_Dirty();
	m_Status = "Deleted sequence template and its placed instances";
}

void Client::CWorldSequenceToolPanel::Apply_SelectedTemplate(
	const CMapAssetCatalog& catalog,
	const std::vector<MAP_RUNTIME_PLACED_ENTRY>& placements,
	const uint64_t placementId)
{
	if (m_Document.Get_Instances().size() >=
		CWorldSequenceDocument::MAX_INSTANCE_COUNT)
	{
		m_Status = "World sequence instance limit reached";
		return;
	}
	const WORLD_SEQUENCE_TEMPLATE* selected =
		m_Document.Find_Template(m_SelectedTemplateId);
	if (nullptr == selected || 0u == placementId || selected->tracks.empty())
	{
		m_Status = "Select a sequence and a map object first";
		return;
	}
	const auto placement = std::find_if(placements.begin(), placements.end(),
		[placementId](const MAP_RUNTIME_PLACED_ENTRY& value)
		{
			return value.record.placementId == placementId;
		});
	if (placements.end() == placement ||
		!IsSequenceTargetSupported(catalog, placement->record))
	{
		m_Status = "This map object cannot be controlled by a world sequence";
		return;
	}
	WORLD_SEQUENCE_INSTANCE value;
	value.instanceId = Allocate_InstanceId(selected->sequenceId);
	value.templateId = selected->sequenceId;
	value.bindings.push_back({ selected->tracks.front().slotId,
		WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT,
		std::to_string(placementId) });
	m_Document.Get_Instances().push_back(std::move(value));
	m_SelectedInstanceId = m_Document.Get_Instances().back().instanceId;
	Mark_Dirty();
	if (selected->tracks.size() > 1u)
		m_Status = "Instance created; bind the remaining target tracks before Save";
	else
		m_Status = "Applied sequence template to selected map object";
}

void Client::CWorldSequenceToolPanel::Apply_SelectedAnimationTemplate(
	CDeployPropRuntime& deployRuntime)
{
	if (m_Document.Get_Instances().size() >=
		CWorldSequenceDocument::MAX_INSTANCE_COUNT)
	{
		m_Status = "World sequence instance limit reached";
		return;
	}
	const WORLD_SEQUENCE_TEMPLATE* selected =
		m_Document.Find_Template(m_SelectedTemplateId);
	const shared_ptr<CDeployPropObject> object =
		deployRuntime.Find(m_iSelectedDeployPlacementId);
	if (nullptr == selected || selected->animationTracks.empty() ||
		nullptr == object)
	{
		m_Status = "Select an animation sequence and animated prop first";
		return;
	}
	const WORLD_SEQUENCE_ANIMATION_TRACK& firstTrack =
		selected->animationTracks.front();
	const std::vector<DEPLOY_PROP_ANIMATION_CLIP> clips =
		GetValidAnimationClips(object);
	const bool_t hasExactClip = clips.end() != std::find_if(
		clips.begin(), clips.end(), [&firstTrack](const auto& clip)
		{
			return clip.name == firstTrack.clipName;
		});
	if (!hasExactClip)
	{
		m_Status = "Selected animated prop does not contain the exact sequence clip";
		return;
	}
	WORLD_SEQUENCE_INSTANCE value;
	value.instanceId = Allocate_InstanceId(selected->sequenceId);
	value.templateId = selected->sequenceId;
	value.bindings.push_back({ firstTrack.slotId,
		WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT,
		std::to_string(m_iSelectedDeployPlacementId) });
	m_Document.Get_Instances().push_back(std::move(value));
	m_SelectedInstanceId = m_Document.Get_Instances().back().instanceId;
	Mark_Dirty();
	if (selected->tracks.size() + selected->animationTracks.size() > 1u)
		m_Status = "Instance created; bind the remaining target tracks before Save";
	else
		m_Status = "Applied animation sequence to selected animated prop";
}

void Client::CWorldSequenceToolPanel::Delete_SelectedInstance()
{
	auto& instances = m_Document.Get_Instances();
	instances.erase(std::remove_if(instances.begin(), instances.end(),
		[this](const WORLD_SEQUENCE_INSTANCE& value)
		{
			return value.instanceId == m_SelectedInstanceId;
		}), instances.end());
	m_SelectedInstanceId.clear();
	Mark_Dirty();
	m_Status = "Deleted placed sequence instance";
}

void Client::CWorldSequenceToolPanel::Mark_Dirty()
{
	if (!m_bDirty)
		m_Document.Touch();
	m_bDirty = true;
	if (m_bPreviewActive)
		m_bPreviewNeedsRefresh = true;
}

std::string Client::CWorldSequenceToolPanel::Allocate_TemplateId() const
{
	for (uint32_t index = 1; index <= CWorldSequenceDocument::MAX_TEMPLATE_COUNT;
		++index)
	{
		const std::string candidate = "sequence." +
			m_Document.Get_AreaId() + "." + std::to_string(index);
		if (nullptr == m_Document.Find_Template(candidate))
			return candidate;
	}
	return "sequence.unavailable";
}

std::string Client::CWorldSequenceToolPanel::Allocate_InstanceId(
	const std::string&) const
{
	for (uint32_t index = 1; index <= CWorldSequenceDocument::MAX_INSTANCE_COUNT;
		++index)
	{
		const std::string candidate = "world.sequence.instance." +
			std::to_string(index);
		if (nullptr == m_Document.Find_Instance(candidate))
			return candidate;
	}
	return "world.sequence.instance.unavailable";
}

void Client::CWorldSequenceToolPanel::Render_DocumentToolbar(
	const uint32_t authoringLevelIndex,
	const CMapAssetCatalog& catalog,
	std::vector<MAP_RUNTIME_PLACED_ENTRY>& placements,
	CDeployPropRuntime& deployRuntime)
{
	ImGui::BeginDisabled(m_bPreviewActive);
	if (ImGui::Button("New Sequence"))
		Create_Template();
	ImGui::EndDisabled();
	ShowKoreanHelp("새 재사용 연출 템플릿을 만듭니다. 기본 시작·끝 키가 자동으로 생성됩니다.");
	ImGui::SameLine();
	ImGui::BeginDisabled(m_bPreviewActive || 0u == m_iSelectedDeployPlacementId);
	if (ImGui::Button("New Animation Sequence"))
		Create_AnimationTemplate(deployRuntime);
	ImGui::EndDisabled();
	ShowKoreanHelp("Animated Props에서 고른 오브젝트의 첫 애니메이션으로 재사용 템플릿과 배치를 함께 만듭니다.");
	ImGui::SameLine();
	if (ImGui::Button("Save"))
	{
		m_bSaveAllRequested = true;
		m_Status = "Save All requested for linked map and sequence documents";
	}
	ShowKoreanHelp("맵 배치와 연출 연결이 어긋나지 않도록 현재 Area의 변경 사항을 함께 저장합니다.");
	ImGui::SameLine();
	if (ImGui::Button("Reload"))
	{
		m_bReloadAllRequested = true;
		m_Status = "Reload All requested for linked map and sequence documents";
	}
	ShowKoreanHelp("저장되지 않은 연출 편집을 버리고 파일을 다시 읽습니다.");
	ImGui::SameLine();
	if (ImGui::Button("Validate"))
	{
		std::string status;
		(void)Validate(catalog, placements, deployRuntime, status);
		m_Status = status;
	}
	ShowKoreanHelp("ID, 키 시간, 오브젝트 연결과 변환 값이 안전한지 검사합니다.");
	ImGui::SameLine();
	ImGui::TextColored(m_bDirty ? ImVec4(1.f, 0.75f, 0.2f, 1.f) :
		ImVec4(0.45f, 0.9f, 0.55f, 1.f), m_bDirty ? "UNSAVED" : "SAVED");
}

void Client::CWorldSequenceToolPanel::Render_TemplateList()
{
	ImGui::BeginDisabled(m_bPreviewActive);
	ImGui::SetNextItemWidth(-1.f);
	ImGui::InputTextWithHint("##sequence-filter", "Search sequences...",
		m_TemplateFilter, sizeof(m_TemplateFilter));
	ShowKoreanHelp("연출 이름, ID 또는 분류로 검색합니다.");
	if (ImGui::BeginChild("##sequence-list", ImVec2(0.f, 190.f), true))
	{
		for (const WORLD_SEQUENCE_TEMPLATE& value : m_Document.Get_Templates())
		{
			const std::string search = value.displayName + " " + value.sequenceId +
				" " + value.category;
			if (!MatchesFilter(search, m_TemplateFilter))
				continue;
			ImGui::PushID(value.sequenceId.c_str());
			const std::string label = value.displayName + "##template";
			if (ImGui::Selectable(label.c_str(),
				m_SelectedTemplateId == value.sequenceId,
				ImGuiSelectableFlags_AllowDoubleClick))
			{
				m_SelectedTemplateId = value.sequenceId;
				m_SelectedInstanceId.clear();
				m_iSelectedTrack = 0;
				m_iSelectedKey = 0;
				m_RotationEditorKey.clear();
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("%s\n%s\n%u ms", value.sequenceId.c_str(),
					value.category.c_str(), value.durationMs);
			ImGui::PopID();
		}
	}
	ImGui::EndChild();
	ImGui::EndDisabled();
}

void Client::CWorldSequenceToolPanel::Render_InstanceList()
{
	ImGui::SetNextItemWidth(-1.f);
	ImGui::InputTextWithHint("##instance-filter", "Search instances...",
		m_InstanceFilter, sizeof(m_InstanceFilter));
	ShowKoreanHelp("맵에 배치한 연출 인스턴스 ID 또는 템플릿 ID로 검색합니다.");
	ImGui::BeginDisabled(m_bPreviewActive);
	if (ImGui::BeginChild("##instance-list", ImVec2(0.f, 190.f), true))
	{
		for (const WORLD_SEQUENCE_INSTANCE& value : m_Document.Get_Instances())
		{
			const WORLD_SEQUENCE_TEMPLATE* sequence =
				m_Document.Find_Template(value.templateId);
			const std::string displayName = nullptr == sequence ?
				value.templateId : sequence->displayName;
			const std::string firstTarget = value.bindings.empty() ?
				"unbound" :
				(WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT ==
					value.bindings.front().targetKind ? "Map #" : "Deploy #") +
				value.bindings.front().targetId;
			const std::string search = value.instanceId + " " + value.templateId +
				" " + displayName + " " + firstTarget;
			if (!MatchesFilter(search, m_InstanceFilter))
				continue;
			ImGui::PushID(value.instanceId.c_str());
			const std::string label = displayName + "  ->  " + firstTarget +
				"##instance";
			if (ImGui::Selectable(label.c_str(),
				m_SelectedInstanceId == value.instanceId))
			{
				m_SelectedInstanceId = value.instanceId;
				m_SelectedTemplateId = value.templateId;
				m_iSelectedTrack = 0;
				m_iSelectedKey = 0;
				m_RotationEditorKey.clear();
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Instance: %s\nTemplate: %s\nTargets: %zu",
					value.instanceId.c_str(), value.templateId.c_str(),
					value.bindings.size());
			ImGui::PopID();
		}
	}
	ImGui::EndChild();
	ImGui::EndDisabled();
}

void Client::CWorldSequenceToolPanel::Render_ObjectBrowser(
	const CMapAssetCatalog& catalog,
	const std::vector<MAP_RUNTIME_PLACED_ENTRY>& placements,
	uint64_t& selectedPlacementId)
{
	ImGui::SetNextItemWidth(-1.f);
	ImGui::InputTextWithHint("##sequence-object-filter", "Search map objects...",
		m_ObjectFilter, sizeof(m_ObjectFilter));
	ShowKoreanHelp("placement ID, asset ID 또는 원본 오브젝트 이름으로 검색합니다.");
	if (ImGui::BeginChild("##sequence-object-list", ImVec2(0.f, 190.f), true))
	{
		const char_t* const filter = m_ObjectFilter;
		const bool_t hasFilter = '\0' != *filter;
		uint64_t placementFingerprint = 14695981039346656037ull;
		if (hasFilter)
		{
			for (const MAP_RUNTIME_PLACED_ENTRY& placement : placements)
			{
				placementFingerprint ^= placement.record.placementId;
				placementFingerprint *= 1099511628211ull;
				for (const unsigned char character : placement.record.assetId)
				{
					placementFingerprint ^= static_cast<uint64_t>(character);
					placementFingerprint *= 1099511628211ull;
				}
				for (const unsigned char character :
					placement.record.sourcePlacementId)
				{
					placementFingerprint ^= static_cast<uint64_t>(character);
					placementFingerprint *= 1099511628211ull;
				}
			}
		}
		if (hasFilter && (m_CachedObjectFilter != filter ||
			m_iCachedObjectPlacementCount != placements.size() ||
			m_iCachedObjectFingerprint != placementFingerprint))
		{
			m_CachedObjectFilter = filter;
			m_iCachedObjectPlacementCount = placements.size();
			m_iCachedObjectFingerprint = placementFingerprint;
			m_FilteredObjectIndices.clear();
			m_FilteredObjectIndices.reserve(placements.size());
			for (size_t index = 0; index < placements.size(); ++index)
			{
				const MAP_PLACEMENT_RECORD& record = placements[index].record;
				const std::string search = std::to_string(record.placementId) + " " +
					record.assetId + " " + record.sourcePlacementId;
				if (MatchesFilter(search, m_ObjectFilter))
					m_FilteredObjectIndices.push_back(index);
			}
		}
		const auto renderPlacement = [&](const size_t index)
		{
			const MAP_PLACEMENT_RECORD& record = placements[index].record;
			const bool_t supported =
				IsSequenceTargetSupported(catalog, record);
			const std::string id = std::to_string(record.placementId);
			ImGui::PushID(static_cast<int>(index));
			const std::string label = "#" + id + "  " + record.assetId +
				(supported ? "" : "  [Background - unavailable]") +
				"##map-object";
			ImGui::BeginDisabled(!supported);
			if (ImGui::Selectable(label.c_str(),
				selectedPlacementId == record.placementId))
			{
				selectedPlacementId = record.placementId;
			}
			ImGui::EndDisabled();
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("%s\n%s%s", record.sourcePlacementId.c_str(),
					record.sourceLevel.c_str(), supported ? "" :
					"\n배경 전용 오브젝트는 카메라를 따라가므로 연출 대상으로 사용할 수 없습니다.");
			}
			ImGui::PopID();
		};
		ImGuiListClipper clipper;
		const size_t visibleCount = !hasFilter ? placements.size() :
			m_FilteredObjectIndices.size();
		clipper.Begin(static_cast<int>(visibleCount));
		while (clipper.Step())
		{
			for (int visibleIndex = clipper.DisplayStart;
				visibleIndex < clipper.DisplayEnd; ++visibleIndex)
			{
				renderPlacement(!hasFilter ?
					static_cast<size_t>(visibleIndex) :
					m_FilteredObjectIndices[visibleIndex]);
			}
		}
	}
	ImGui::EndChild();
}

void Client::CWorldSequenceToolPanel::Render_DeployBrowser(
	CDeployPropRuntime& deployRuntime)
{
	ImGui::TextDisabled("Only animated Deploy props with cooked clips are listed.");
	ShowKoreanHelp("정적 메시나 애니메이션 클립이 없는 원본 파일은 연출 대상으로 표시하지 않습니다.");
	if (ImGui::BeginChild("##sequence-deploy-list", ImVec2(0.f, 190.f), true))
	{
		for (const DEPLOY_RUNTIME_ENTRY& entry : deployRuntime.Get_Entries())
		{
			if (nullptr == entry.object || entry.object->Is_StaticDeployModel())
				continue;
			const std::vector<DEPLOY_PROP_ANIMATION_CLIP> clips =
				GetValidAnimationClips(entry.object);
			if (clips.empty())
				continue;
			const DEPLOY_PROP_ASSET_ENTRY* asset =
				deployRuntime.Get_Catalog().Find(entry.placement.assetId);
			const std::string label = "#" +
				std::to_string(entry.placement.runtimePlacementId) + "  " +
				(nullptr == asset ? entry.placement.assetId : asset->label) +
				"  [" + std::to_string(clips.size()) + " clips]##deploy-object";
			ImGui::PushID(reinterpret_cast<void*>(
				static_cast<uintptr_t>(entry.placement.runtimePlacementId)));
			if (ImGui::Selectable(label.c_str(),
				m_iSelectedDeployPlacementId ==
					entry.placement.runtimePlacementId))
			{
				m_iSelectedDeployPlacementId =
					entry.placement.runtimePlacementId;
			}
			if (ImGui::IsItemHovered())
			{
				std::string clipSummary;
				for (const DEPLOY_PROP_ANIMATION_CLIP& clip : clips)
				{
					if (!clipSummary.empty())
						clipSummary += "\n";
					clipSummary += clip.name + "  " +
						std::to_string(clip.durationSeconds) + " s";
				}
				ImGui::SetTooltip("%s\n%s", entry.placement.assetId.c_str(),
					clipSummary.c_str());
			}
			ImGui::PopID();
		}
	}
	ImGui::EndChild();
}

void Client::CWorldSequenceToolPanel::Render_TemplateEditor(
	const uint32_t authoringLevelIndex,
	const CMapAssetCatalog& catalog,
	std::vector<MAP_RUNTIME_PLACED_ENTRY>& placements,
	const uint64_t selectedPlacementId,
	CDeployPropRuntime& deployRuntime)
{
	WORLD_SEQUENCE_TEMPLATE* sequence =
		m_Document.Find_Template(m_SelectedTemplateId);
	if (nullptr == sequence)
	{
		ImGui::TextDisabled("Create or select a sequence template.");
		return;
	}
	ImGui::Text("Template: %s", sequence->sequenceId.c_str());
	char_t displayName[129]{};
	CopyToBuffer(displayName, sizeof(displayName), sequence->displayName);
	if (ImGui::InputText("Display Name", displayName, sizeof(displayName)))
	{
		sequence->displayName = displayName;
		Mark_Dirty();
	}
	ShowKoreanHelp("팀원이 목록에서 알아볼 수 있는 연출 이름입니다.");
	char_t category[65]{};
	CopyToBuffer(category, sizeof(category), sequence->category);
	if (ImGui::InputText("Category", category, sizeof(category)))
	{
		sequence->category = category;
		Mark_Dirty();
	}
	ShowKoreanHelp("Door, Paper, Stage처럼 연출을 찾기 쉬운 분류를 적습니다.");

	size_t maximumKeyCount = 2u;
	for (const WORLD_SEQUENCE_TRACK& track : sequence->tracks)
		maximumKeyCount = (std::max)(maximumKeyCount, track.keys.size());
	const int minimumDurationMs = static_cast<int>(maximumKeyCount - 1u);
	int durationMs = static_cast<int>(sequence->durationMs);
	if (ImGui::DragInt("Duration (ms)", &durationMs, 10.f, 1,
		static_cast<int>(CWorldSequenceDocument::MAX_DURATION_MS)))
	{
		durationMs = (std::max)(minimumDurationMs, durationMs);
		const uint32_t oldDuration = sequence->durationMs;
		sequence->durationMs = static_cast<uint32_t>(durationMs);
		for (WORLD_SEQUENCE_TRACK& track : sequence->tracks)
		{
			for (size_t index = 1; index + 1u < track.keys.size(); ++index)
			{
				const double normalized = 0u == oldDuration ? 0.0 :
					static_cast<double>(track.keys[index].timeMs) /
					static_cast<double>(oldDuration);
				const uint32_t remaining = static_cast<uint32_t>(
					track.keys.size() - 1u - index);
				const uint32_t scaled = static_cast<uint32_t>(
					std::round(normalized * sequence->durationMs));
				track.keys[index].timeMs = (std::max)(
					track.keys[index - 1u].timeMs + 1u,
					(std::min)(sequence->durationMs - remaining, scaled));
			}
			track.keys.back().timeMs = sequence->durationMs;
		}
		Mark_Dirty();
	}
	ShowKoreanHelp("연출이 시작부터 끝까지 재생되는 기본 시간입니다.");
	int interpolation = WORLD_SEQUENCE_INTERPOLATION::LINEAR ==
		sequence->interpolation ? 0 : 1;
	if (ImGui::Combo("Motion", &interpolation, "Linear\0Smooth\0"))
	{
		sequence->interpolation = 0 == interpolation ?
			WORLD_SEQUENCE_INTERPOLATION::LINEAR :
			WORLD_SEQUENCE_INTERPOLATION::SMOOTH_STEP;
		Mark_Dirty();
	}
	ShowKoreanHelp("Linear는 일정 속도, Smooth는 시작과 끝을 부드럽게 움직입니다.");

	ImGui::BeginDisabled(m_bPreviewActive || sequence->tracks.empty());
	if (ImGui::Button("Apply to Selected Object"))
		Apply_SelectedTemplate(catalog, placements, selectedPlacementId);
	ImGui::EndDisabled();
	ShowKoreanHelp("오른쪽 Map Objects에서 고른 오브젝트에 이 연출을 재사용 배치합니다.");
	ImGui::SameLine();
	ImGui::BeginDisabled(m_bPreviewActive ||
		sequence->animationTracks.empty() || 0u == m_iSelectedDeployPlacementId);
	if (ImGui::Button("Apply to Selected Animated Prop"))
		Apply_SelectedAnimationTemplate(deployRuntime);
	ImGui::EndDisabled();
	ShowKoreanHelp("오른쪽 Animated Props에서 고른 애니메이션 오브젝트에 이 연출을 재사용 배치합니다.");
	ImGui::SameLine();
	ImGui::BeginDisabled(m_bPreviewActive);
	bool_t duplicatedTemplate = false;
	if (ImGui::Button("Duplicate Template"))
	{
		Duplicate_SelectedTemplate();
		duplicatedTemplate = true;
	}
	ShowKoreanHelp("키와 설정은 복사하되 기존 오브젝트 연결은 복사하지 않습니다.");
	ImGui::SameLine();
	if (ImGui::Button("Delete Template"))
		ImGui::OpenPopup("Delete sequence template?");
	ShowKoreanHelp("이 템플릿과 연결된 배치 인스턴스를 함께 삭제합니다.");
	bool_t deletedTemplate = false;
	if (ImGui::BeginPopupModal("Delete sequence template?", nullptr,
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::TextWrapped("Delete '%s' and every placed instance that uses it?",
			sequence->displayName.c_str());
		ImGui::TextDisabled("이 작업은 Save 전에는 Reload로 되돌릴 수 있습니다.");
		if (ImGui::Button("Delete"))
		{
			Delete_SelectedTemplate();
			deletedTemplate = true;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
	ImGui::EndDisabled();
	if (deletedTemplate)
		return;
	if (duplicatedTemplate)
		return;

	WORLD_SEQUENCE_INSTANCE* instance =
		m_Document.Find_Instance(m_SelectedInstanceId);
	if (nullptr != instance && instance->templateId == sequence->sequenceId)
	{
		ImGui::SeparatorText("Placed Instance");
		if (ImGui::Checkbox("Enabled", &instance->enabled))
			Mark_Dirty();
		ShowKoreanHelp("꺼두면 제품 런타임에서 이 배치를 시작할 수 없습니다.");
		int delay = static_cast<int>(instance->startDelayMs);
		if (ImGui::DragInt("Start Delay (ms)", &delay, 10.f, 0,
			static_cast<int>(CWorldSequenceDocument::MAX_DURATION_MS)))
		{
			instance->startDelayMs = static_cast<uint32_t>(delay);
			Mark_Dirty();
		}
		ShowKoreanHelp("연출 명령을 받은 뒤 실제 움직임을 시작하기 전 기다리는 시간입니다.");
		if (ImGui::DragFloat("Playback Speed", &instance->playbackSpeed,
			0.01f, 0.05f, 8.f, "%.2fx"))
			Mark_Dirty();
		ShowKoreanHelp("이 배치에서만 재생 속도를 빠르게 하거나 느리게 합니다.");

		if (ImGui::BeginTable("##sequence-bindings", 3,
			ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp))
		{
			ImGui::TableSetupColumn("Target Slot");
			ImGui::TableSetupColumn("Bound Target");
			ImGui::TableSetupColumn("Action");
			ImGui::TableHeadersRow();
			for (const WORLD_SEQUENCE_TRACK& track : sequence->tracks)
			{
				auto binding = std::find_if(instance->bindings.begin(),
					instance->bindings.end(),
					[&track](const WORLD_SEQUENCE_BINDING& value)
					{
						return value.slotId == track.slotId;
					});
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::TextUnformatted(track.slotId.c_str());
				ImGui::TableSetColumnIndex(1);
				if (instance->bindings.end() == binding)
					ImGui::TextColored(ImVec4(1.f, 0.4f, 0.3f, 1.f), "NOT BOUND");
				else
					ImGui::Text("Map #%s", binding->targetId.c_str());
				ImGui::TableSetColumnIndex(2);
				ImGui::PushID(track.slotId.c_str());
				ImGui::BeginDisabled(0u == selectedPlacementId || m_bPreviewActive);
				if (ImGui::Button("Bind Selected"))
				{
					const MAP_RUNTIME_PLACED_ENTRY* selectedPlacement =
						Find_Placement(placements, selectedPlacementId);
					const auto duplicate = std::find_if(
						instance->bindings.begin(), instance->bindings.end(),
						[&track, selectedPlacementId](
							const WORLD_SEQUENCE_BINDING& value)
						{
							return value.slotId != track.slotId &&
								WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT ==
									value.targetKind &&
								value.targetId == std::to_string(selectedPlacementId);
						});
					if (nullptr == selectedPlacement ||
						!IsSequenceTargetSupported(
							catalog, selectedPlacement->record))
					{
						m_Status = "Selected map object cannot be controlled by a world sequence";
					}
					else if (instance->bindings.end() != duplicate)
					{
						m_Status = "A map object can be bound to only one target slot in an instance";
					}
					else
					{
						if (instance->bindings.end() == binding)
							instance->bindings.push_back(
								{ track.slotId,
								  WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT,
								  std::to_string(selectedPlacementId) });
						else
						{
							binding->targetKind =
								WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT;
							binding->targetId = std::to_string(selectedPlacementId);
						}
						Mark_Dirty();
						m_Status = "Bound target slot to selected map object";
					}
				}
				ImGui::EndDisabled();
				ShowKoreanHelp("Map Objects에서 선택한 오브젝트를 이 역할에 연결합니다.");
				ImGui::PopID();
			}
			for (const WORLD_SEQUENCE_ANIMATION_TRACK& track :
				sequence->animationTracks)
			{
				auto binding = std::find_if(instance->bindings.begin(),
					instance->bindings.end(),
					[&track](const WORLD_SEQUENCE_BINDING& value)
					{
						return value.slotId == track.slotId;
					});
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("%s  [Animation]", track.slotId.c_str());
				ImGui::TableSetColumnIndex(1);
				if (instance->bindings.end() == binding)
					ImGui::TextColored(ImVec4(1.f, 0.4f, 0.3f, 1.f), "NOT BOUND");
				else
					ImGui::Text("Deploy #%s", binding->targetId.c_str());
				ImGui::TableSetColumnIndex(2);
				ImGui::PushID(track.slotId.c_str());
				ImGui::BeginDisabled(0u == m_iSelectedDeployPlacementId ||
					m_bPreviewActive);
				if (ImGui::Button("Bind Selected Animated"))
				{
					const shared_ptr<CDeployPropObject> object =
						deployRuntime.Find(m_iSelectedDeployPlacementId);
					const std::vector<DEPLOY_PROP_ANIMATION_CLIP> clips =
						nullptr == object ?
						std::vector<DEPLOY_PROP_ANIMATION_CLIP>{} :
						GetValidAnimationClips(object);
					const bool_t hasExactClip = clips.end() != std::find_if(
						clips.begin(), clips.end(), [&track](const auto& clip)
						{
							return clip.name == track.clipName;
						});
					const std::string selectedId =
						std::to_string(m_iSelectedDeployPlacementId);
					const auto duplicate = std::find_if(instance->bindings.begin(),
						instance->bindings.end(), [&track, &selectedId](const auto& value)
						{
							return value.slotId != track.slotId &&
								WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT ==
									value.targetKind && value.targetId == selectedId;
						});
					if (!hasExactClip)
						m_Status = "Selected animated prop does not contain the exact track clip";
					else if (instance->bindings.end() != duplicate)
						m_Status = "An animated prop can be bound to only one target slot in an instance";
					else
					{
						if (instance->bindings.end() == binding)
							instance->bindings.push_back({ track.slotId,
								WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT,
								selectedId });
						else
						{
							binding->targetKind =
								WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT;
							binding->targetId = selectedId;
						}
						Mark_Dirty();
						m_Status = "Bound animation slot to selected animated prop";
					}
				}
				ImGui::EndDisabled();
				ShowKoreanHelp("Animated Props에서 선택한 오브젝트를 이 애니메이션 역할에 연결합니다.");
				ImGui::PopID();
			}
			ImGui::EndTable();
		}
		ImGui::BeginDisabled(m_bPreviewActive);
		if (ImGui::Button("Delete Instance"))
			Delete_SelectedInstance();
		ImGui::EndDisabled();
		ShowKoreanHelp("템플릿은 남기고 이 맵 오브젝트 연결만 삭제합니다.");
	}

	ImGui::SeparatorText("Target Tracks and Keyframes");
	Render_KeyEditor(*sequence);
	ImGui::SeparatorText("Animation Tracks");
	Render_AnimationEditor(*sequence, deployRuntime);
	ImGui::SeparatorText("Preview");
	Render_Playback(authoringLevelIndex, catalog, placements, deployRuntime);
}

void Client::CWorldSequenceToolPanel::Render_KeyEditor(
	WORLD_SEQUENCE_TEMPLATE& sequence)
{
	if (sequence.tracks.empty())
		return;
	m_iSelectedTrack = (std::min)(m_iSelectedTrack, sequence.tracks.size() - 1u);
	if (ImGui::BeginCombo("Target Track",
		sequence.tracks[m_iSelectedTrack].slotId.c_str()))
	{
		for (size_t index = 0; index < sequence.tracks.size(); ++index)
		{
			if (ImGui::Selectable(sequence.tracks[index].slotId.c_str(),
				index == m_iSelectedTrack))
			{
				m_iSelectedTrack = index;
				m_iSelectedKey = 0;
				m_RotationEditorKey.clear();
			}
		}
		ImGui::EndCombo();
	}
	ShowKoreanHelp("여러 부품으로 만든 연출에서 지금 편집할 오브젝트 역할을 고릅니다.");
	WORLD_SEQUENCE_TRACK& track = sequence.tracks[m_iSelectedTrack];

	ImGui::BeginDisabled(sequence.tracks.size() +
		sequence.animationTracks.size() >=
		CWorldSequenceDocument::MAX_TRACK_COUNT);
	if (ImGui::Button("Add Target Track"))
	{
		WORLD_SEQUENCE_TRACK added;
		for (uint32_t index = 1; index <= CWorldSequenceDocument::MAX_TRACK_COUNT;
			++index)
		{
			const std::string candidate = "object." + std::to_string(index);
			const bool_t usedByAnimation = sequence.animationTracks.end() !=
				std::find_if(sequence.animationTracks.begin(),
					sequence.animationTracks.end(),
					[&candidate](const WORLD_SEQUENCE_ANIMATION_TRACK& value)
					{
						return value.slotId == candidate;
					});
			if (nullptr == Find_Track(sequence, candidate) && !usedByAnimation)
			{
				added.slotId = candidate;
				break;
			}
		}
		added.keys.push_back({});
		WORLD_SEQUENCE_TRANSFORM_KEY end;
		end.timeMs = sequence.durationMs;
		added.keys.push_back(end);
		sequence.tracks.push_back(std::move(added));
		m_iSelectedTrack = sequence.tracks.size() - 1u;
		m_iSelectedKey = 0;
		Mark_Dirty();
		m_Status = "Added target track; bind it in every placed instance";
	}
	ImGui::EndDisabled();
	ShowKoreanHelp("문, 종이 조각처럼 같이 움직일 부품 역할을 하나 더 추가합니다.");
	ImGui::SameLine();
	ImGui::BeginDisabled(sequence.tracks.size() +
		sequence.animationTracks.size() <= 1u || m_bPreviewActive);
	if (ImGui::Button("Delete Target Track"))
	{
		const std::string deletedSlot = track.slotId;
		sequence.tracks.erase(sequence.tracks.begin() + m_iSelectedTrack);
		for (WORLD_SEQUENCE_INSTANCE& instance : m_Document.Get_Instances())
		{
			if (instance.templateId != sequence.sequenceId)
				continue;
			instance.bindings.erase(std::remove_if(instance.bindings.begin(),
				instance.bindings.end(),
				[&deletedSlot](const WORLD_SEQUENCE_BINDING& binding)
				{
					return binding.slotId == deletedSlot;
				}), instance.bindings.end());
		}
		m_iSelectedTrack = 0;
		m_iSelectedKey = 0;
		m_RotationEditorKey.clear();
		Mark_Dirty();
	}
	ImGui::EndDisabled();
	ShowKoreanHelp("이 역할의 모든 키와 인스턴스 연결을 삭제합니다.");

	WORLD_SEQUENCE_TRACK& selectedTrack = sequence.tracks[m_iSelectedTrack];
	m_iSelectedKey = (std::min)(m_iSelectedKey, selectedTrack.keys.size() - 1u);
	if (ImGui::BeginTable("##sequence-key-list", 2,
		ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp))
	{
		ImGui::TableSetupColumn("Keyframes", ImGuiTableColumnFlags_WidthFixed, 180.f);
		ImGui::TableSetupColumn("Selected Key");
		ImGui::TableHeadersRow();
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		if (ImGui::BeginChild("##sequence-keys", ImVec2(0.f, 210.f), true))
		{
			for (size_t index = 0; index < selectedTrack.keys.size(); ++index)
			{
				const WORLD_SEQUENCE_TRANSFORM_KEY& key = selectedTrack.keys[index];
				const std::string label = (0u == index ? "START  " :
					(index + 1u == selectedTrack.keys.size() ? "END    " : "KEY    ")) +
					std::to_string(key.timeMs) + " ms##key";
				ImGui::PushID(static_cast<int>(index));
				if (ImGui::Selectable(label.c_str(), index == m_iSelectedKey))
				{
					m_iSelectedKey = index;
					m_RotationEditorKey.clear();
				}
				ImGui::PopID();
			}
		}
		ImGui::EndChild();
		ImGui::TableSetColumnIndex(1);
		WORLD_SEQUENCE_TRANSFORM_KEY& key = selectedTrack.keys[m_iSelectedKey];
		const bool_t endpoint = 0u == m_iSelectedKey ||
			m_iSelectedKey + 1u == selectedTrack.keys.size();
		int timeMs = static_cast<int>(key.timeMs);
		ImGui::BeginDisabled(endpoint);
		if (ImGui::DragInt("Time (ms)", &timeMs, 10.f,
			m_iSelectedKey > 0u ?
				static_cast<int>(selectedTrack.keys[m_iSelectedKey - 1u].timeMs + 1u) : 0,
			m_iSelectedKey + 1u < selectedTrack.keys.size() ?
				static_cast<int>(selectedTrack.keys[m_iSelectedKey + 1u].timeMs - 1u) :
				static_cast<int>(sequence.durationMs)))
		{
			key.timeMs = static_cast<uint32_t>(timeMs);
			m_RotationEditorKey.clear();
			Mark_Dirty();
		}
		ImGui::EndDisabled();
		ShowKoreanHelp("이 모습에 도달해야 하는 시간을 정합니다. 시작과 끝 시간은 자동 관리됩니다.");
		if (ImGui::DragFloat3("Position Offset", &key.positionOffset.x, 0.01f))
			Mark_Dirty();
		ShowKoreanHelp("배치된 원래 위치를 기준으로 로컬 X/Y/Z 방향으로 이동합니다.");
		const std::string rotationKey = MakeRotationEditorKey(sequence,
			selectedTrack, key);
		if (m_RotationEditorKey != rotationKey)
		{
			m_RotationEditorKey = rotationKey;
			m_RotationEditorDegrees =
				QuaternionToEulerDegrees(key.rotationQuaternion);
		}
		if (ImGui::DragFloat3("Rotation Offset", &m_RotationEditorDegrees.x,
			0.25f, -360.f, 360.f, "%.1f deg"))
		{
			key.rotationQuaternion =
				EulerDegreesToQuaternion(m_RotationEditorDegrees);
			Mark_Dirty();
		}
		ShowKoreanHelp("원래 회전을 기준으로 Pitch/Yaw/Roll 각도를 더합니다.");
		if (ImGui::DragFloat3("Scale Multiplier", &key.scaleMultiplier.x,
			0.01f, 0.001f, 100.f))
			Mark_Dirty();
		ShowKoreanHelp("원래 크기에 곱할 값입니다. 1,1,1이면 원래 크기입니다.");
		if (ImGui::Checkbox("Visible", &key.visible))
			Mark_Dirty();
		ShowKoreanHelp("이 키부터 오브젝트를 보이거나 숨깁니다.");
		if (ImGui::Button("Reset Key"))
		{
			const uint32_t preservedTime = key.timeMs;
			key = {};
			key.timeMs = preservedTime;
			m_RotationEditorKey.clear();
			Mark_Dirty();
		}
		ShowKoreanHelp("선택한 키를 이동 없음, 회전 없음, 원래 크기, 보임 상태로 되돌립니다.");
		ImGui::SameLine();
		ImGui::BeginDisabled(endpoint);
		if (ImGui::Button("Delete Key"))
		{
			selectedTrack.keys.erase(selectedTrack.keys.begin() + m_iSelectedKey);
			m_iSelectedKey = (std::min)(m_iSelectedKey,
				selectedTrack.keys.size() - 1u);
			m_RotationEditorKey.clear();
			Mark_Dirty();
		}
		ImGui::EndDisabled();
		ShowKoreanHelp("시작과 끝을 제외한 중간 키를 삭제합니다.");
		ImGui::EndTable();
	}

	f32_t localPreviewMs = m_fPreviewTimeMs;
	const WORLD_SEQUENCE_INSTANCE* previewInstance =
		m_Document.Find_Instance(m_SelectedInstanceId);
	if (nullptr != previewInstance &&
		previewInstance->templateId == sequence.sequenceId)
	{
		localPreviewMs = (std::max)(0.f,
			m_fPreviewTimeMs - static_cast<f32_t>(previewInstance->startDelayMs)) *
			previewInstance->playbackSpeed;
	}
	const uint32_t addTime = (std::max)(1u,
		(std::min)(sequence.durationMs - 1u,
			static_cast<uint32_t>((std::max)(0.f, localPreviewMs))));
	ImGui::BeginDisabled(sequence.durationMs <= 1u ||
		selectedTrack.keys.size() >= CWorldSequenceDocument::MAX_KEY_COUNT);
	if (ImGui::Button("Add Key at Preview Time"))
	{
		const auto existing = std::find_if(selectedTrack.keys.begin(),
			selectedTrack.keys.end(),
			[addTime](const WORLD_SEQUENCE_TRANSFORM_KEY& value)
			{
				return value.timeMs == addTime;
			});
		if (selectedTrack.keys.end() != existing)
		{
			m_iSelectedKey = static_cast<size_t>(existing - selectedTrack.keys.begin());
			m_Status = "A key already exists at the preview time";
		}
		else
		{
			WORLD_SEQUENCE_TRANSFORM_KEY added = Sample_Track(sequence,
				selectedTrack, static_cast<f32_t>(addTime));
			added.timeMs = addTime;
			const auto position = std::upper_bound(selectedTrack.keys.begin(),
				selectedTrack.keys.end(), addTime,
				[](const uint32_t time, const WORLD_SEQUENCE_TRANSFORM_KEY& value)
				{
					return time < value.timeMs;
				});
			m_iSelectedKey = static_cast<size_t>(position - selectedTrack.keys.begin());
			selectedTrack.keys.insert(position, added);
			m_RotationEditorKey.clear();
			Mark_Dirty();
		}
	}
	ImGui::EndDisabled();
	ShowKoreanHelp("현재 미리보기 시간의 모습을 중간 키로 추가합니다.");
}

void Client::CWorldSequenceToolPanel::Render_AnimationEditor(
	WORLD_SEQUENCE_TEMPLATE& sequence,
	CDeployPropRuntime& deployRuntime)
{
	const shared_ptr<CDeployPropObject> selectedObject =
		deployRuntime.Find(m_iSelectedDeployPlacementId);
	const std::vector<DEPLOY_PROP_ANIMATION_CLIP> selectedClips =
		nullptr == selectedObject ? std::vector<DEPLOY_PROP_ANIMATION_CLIP>{} :
		GetValidAnimationClips(selectedObject);

	ImGui::BeginDisabled(sequence.tracks.size() +
		sequence.animationTracks.size() >=
		CWorldSequenceDocument::MAX_TRACK_COUNT || selectedClips.empty());
	if (ImGui::Button("Add Animation Track"))
	{
		WORLD_SEQUENCE_ANIMATION_TRACK added;
		for (uint32_t index = 1u;
			index <= CWorldSequenceDocument::MAX_TRACK_COUNT; ++index)
		{
			const std::string candidate = "animated.prop." +
				std::to_string(index);
			const bool_t usedByTransform = sequence.tracks.end() != std::find_if(
				sequence.tracks.begin(), sequence.tracks.end(),
				[&candidate](const WORLD_SEQUENCE_TRACK& track)
				{
					return track.slotId == candidate;
				});
			const bool_t usedByAnimation = sequence.animationTracks.end() !=
				std::find_if(sequence.animationTracks.begin(),
					sequence.animationTracks.end(),
					[&candidate](const WORLD_SEQUENCE_ANIMATION_TRACK& track)
					{
						return track.slotId == candidate;
					});
			if (!usedByTransform && !usedByAnimation)
			{
				added.slotId = candidate;
				break;
			}
		}
		added.clipName = selectedClips.front().name;
		sequence.animationTracks.push_back(std::move(added));
		m_iSelectedAnimationTrack = sequence.animationTracks.size() - 1u;
		Mark_Dirty();
		m_Status = "Added animation track; bind it in every placed instance";
	}
	ImGui::EndDisabled();
	ShowKoreanHelp("Animated Props에서 선택한 오브젝트의 실제 클립으로 애니메이션 역할을 추가합니다.");

	if (sequence.animationTracks.empty())
	{
		ImGui::TextDisabled("No animation tracks in this template.");
		return;
	}
	m_iSelectedAnimationTrack = (std::min)(m_iSelectedAnimationTrack,
		sequence.animationTracks.size() - 1u);
	if (ImGui::BeginCombo("Animation Track",
		sequence.animationTracks[m_iSelectedAnimationTrack].slotId.c_str()))
	{
		for (size_t index = 0u; index < sequence.animationTracks.size(); ++index)
		{
			if (ImGui::Selectable(sequence.animationTracks[index].slotId.c_str(),
				index == m_iSelectedAnimationTrack))
			{
				m_iSelectedAnimationTrack = index;
			}
		}
		ImGui::EndCombo();
	}
	WORLD_SEQUENCE_ANIMATION_TRACK& track =
		sequence.animationTracks[m_iSelectedAnimationTrack];

	if (!selectedClips.empty() && ImGui::BeginCombo("Exact Clip",
		track.clipName.c_str()))
	{
		for (const DEPLOY_PROP_ANIMATION_CLIP& clip : selectedClips)
		{
			const std::string label = clip.name + "  (" +
				std::to_string(clip.durationSeconds) + " s)";
			if (ImGui::Selectable(label.c_str(), clip.name == track.clipName))
			{
				track.clipName = clip.name;
				Mark_Dirty();
			}
		}
		ImGui::EndCombo();
	}
	else if (selectedClips.empty())
	{
		ImGui::Text("Exact Clip: %s", track.clipName.c_str());
	}
	ShowKoreanHelp("선택한 Animated Prop의 조리된 WModel에 실제로 들어 있는 정확한 클립 이름만 선택합니다.");
	if (ImGui::DragFloat("Animation Rate", &track.playbackRate,
		0.01f, 0.05f, 8.f, "%.2fx"))
		Mark_Dirty();
	ShowKoreanHelp("클립 자체의 진행 속도를 조절합니다. 인스턴스 Playback Speed와 곱해집니다.");
	if (ImGui::Checkbox("Loop Animation", &track.loop))
		Mark_Dirty();
	ShowKoreanHelp("켜면 템플릿 시간 동안 애니메이션 클립을 반복합니다.");
	if (ImGui::Checkbox("Hold Last Frame", &track.holdLastFrame))
		Mark_Dirty();
	ShowKoreanHelp("반복하지 않을 때 클립 끝에 도달하면 마지막 자세를 유지합니다.");

	ImGui::BeginDisabled(sequence.tracks.size() +
		sequence.animationTracks.size() <= 1u || m_bPreviewActive);
	if (ImGui::Button("Delete Animation Track"))
	{
		const std::string deletedSlot = track.slotId;
		sequence.animationTracks.erase(sequence.animationTracks.begin() +
			m_iSelectedAnimationTrack);
		for (WORLD_SEQUENCE_INSTANCE& instance : m_Document.Get_Instances())
		{
			if (instance.templateId != sequence.sequenceId)
				continue;
			instance.bindings.erase(std::remove_if(instance.bindings.begin(),
				instance.bindings.end(), [&deletedSlot](const auto& binding)
				{
					return binding.slotId == deletedSlot;
				}), instance.bindings.end());
		}
		m_iSelectedAnimationTrack = 0u;
		Mark_Dirty();
	}
	ImGui::EndDisabled();
	ShowKoreanHelp("이 애니메이션 역할과 모든 인스턴스 연결을 삭제합니다.");
}

void Client::CWorldSequenceToolPanel::Render_Playback(
	const uint32_t authoringLevelIndex,
	const CMapAssetCatalog& catalog,
	std::vector<MAP_RUNTIME_PLACED_ENTRY>& placements,
	CDeployPropRuntime& deployRuntime)
{
	WORLD_SEQUENCE_INSTANCE* instance =
		m_Document.Find_Instance(m_SelectedInstanceId);
	WORLD_SEQUENCE_TEMPLATE* sequence = nullptr == instance ? nullptr :
		m_Document.Find_Template(instance->templateId);
	if (nullptr == instance || nullptr == sequence)
	{
		ImGui::TextDisabled("Apply the template to a map object, then select its instance.");
		ImGui::TextDisabled("먼저 템플릿을 맵 오브젝트에 적용해야 실제 위치에서 미리볼 수 있습니다.");
		return;
	}
	const f32_t totalMs = static_cast<f32_t>(instance->startDelayMs) +
		static_cast<f32_t>(sequence->durationMs) /
			(std::max)(0.05f, instance->playbackSpeed);
	if (!m_bPlaying)
	{
		if (ImGui::Button("Play"))
		{
			const bool_t wasPreviewActive = m_bPreviewActive;
			const bool_t rewound = m_fPreviewTimeMs >= totalMs;
			if (rewound)
				m_fPreviewTimeMs = 0.f;
			if (!m_bPreviewActive && !Begin_Preview(authoringLevelIndex,
				catalog, placements, deployRuntime))
			{
				return;
			}
			if (rewound && wasPreviewActive && m_bPreviewActive &&
				!Apply_Preview(authoringLevelIndex, catalog, placements,
					deployRuntime))
			{
				Stop_AndRestore(authoringLevelIndex, catalog, placements,
					deployRuntime);
				return;
			}
			m_bPlaying = true;
		}
		ShowKoreanHelp("선택한 배치의 연출을 현재 시간부터 재생합니다.");
	}
	else
	{
		if (ImGui::Button("Pause"))
			m_bPlaying = false;
		ShowKoreanHelp("현재 모습은 유지하고 시간만 멈춥니다.");
	}
	ImGui::SameLine();
	if (ImGui::Button("Stop / Restore"))
	{
		Stop_AndRestore(authoringLevelIndex, catalog, placements, deployRuntime);
		m_fPreviewTimeMs = 0.f;
	}
	ShowKoreanHelp("미리보기를 끝내고 모든 오브젝트를 원래 배치 상태로 되돌립니다.");
	ImGui::SameLine();
	ImGui::Checkbox("Loop", &m_bLoop);
	ShowKoreanHelp("끝까지 재생한 뒤 자동으로 처음부터 반복합니다.");
	f32_t previewTime = (std::min)(m_fPreviewTimeMs, totalMs);
	if (ImGui::SliderFloat("Time", &previewTime, 0.f, (std::max)(1.f, totalMs),
		"%.0f ms"))
	{
		m_fPreviewTimeMs = previewTime;
		if (!m_bPreviewActive)
			(void)Begin_Preview(authoringLevelIndex, catalog, placements,
				deployRuntime);
		else if (!Apply_Preview(authoringLevelIndex, catalog, placements,
			deployRuntime))
			Stop_AndRestore(authoringLevelIndex, catalog, placements,
				deployRuntime);
	}
	ShowKoreanHelp("시간 손잡이를 움직여 원하는 순간의 오브젝트 모습을 확인합니다.");
	ImGui::TextDisabled(
		"Preview does not edit Map Placements. Leaving this tab restores every object.");
	ImGui::TextDisabled(
		"미리보기는 원본 맵 배치를 수정하지 않으며 탭을 나가면 자동으로 원상복구됩니다.");
}
