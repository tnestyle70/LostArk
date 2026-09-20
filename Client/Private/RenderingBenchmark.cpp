#include <WinSock2.h>
#include "imgui.h"
#pragma push_macro("new")
#undef new
#include "Engine_RenderTypes.h"
#pragma pop_macro("new")

#include "RenderingBenchmark.h"
#include "RenderingProfileService.h"
#include "GameInstance.h"
#include "Level_KakulSaydonArena.h"
#include "Presentation_Manager.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <numeric>
#include <sstream>

namespace
{
    // Serialize named fields, never struct padding. Exclude only the A/B switch.
    string ComparisonConditions()
    {
        const auto& game = Engine::CGameInstance::Get();
        auto& mutableGame = Engine::CGameInstance::Get();
        ostringstream stream;
        stream << setprecision(9) << scientific;
        const auto scalar = [&](const auto value) { stream << value << ' '; };
        const auto vector = [&](const auto& value) {
            scalar(value.x); scalar(value.y); scalar(value.z); scalar(value.w);
        };
        scalar(game.Get_CurrentLevelID());
        scalar(Engine::CPresentation_Manager::Get().Are_TransientLightsEnabled());
        scalar(Client::CMapLightPresentationRuntime::Get_SceneIntensityMultiplier());
        if (const auto* arena = Client::CLevel_KakulSaydonArena::Get_Active())
            scalar(arena->Get_MapLightComparisonFingerprint());
        const auto viewport = mutableGame.Get_ViewportSize(); scalar(viewport.x); scalar(viewport.y);
        for (const auto type : {Engine::D3DTS::VIEW, Engine::D3DTS::PROJ})
        {
            const auto* matrix = mutableGame.Get_Transform(type);
            if (!matrix) { stream << "missing-matrix "; continue; }
            for (const auto& row : matrix->m) for (const auto value : row) scalar(value);
        }
        scalar(static_cast<uint32_t>(game.Get_MaterialRenderSettings().eDebugView));
        const auto q = game.Get_RenderQualitySettings();
        scalar(q.bSSAOEnabled); scalar(q.fSSAORadius); scalar(q.fSSAOBias);
        scalar(q.fSSAOIntensity); scalar(q.fSSAOPower); scalar(q.fSSAODistanceFade);
        scalar(q.bBloomEnabled); scalar(q.fBloomThreshold); scalar(q.fBloomSoftKnee);
        scalar(q.fBloomIntensity); scalar(q.fBloomScatter); scalar(q.fExposure);
        scalar(q.fWhitePoint); scalar(q.fGamma); scalar(q.bFXAAEnabled);
        vector(q.vBloomTint); scalar(q.fSceneDesaturation);
        const auto& source = q.SourcePostProcess;
        scalar(source.bEnabled); scalar(source.fToneScale); scalar(source.fToneRange);
        scalar(source.fToneToe); scalar(source.fDesaturation);
        for (const auto& value : { source.vHighlights, source.vMidtones, source.vShadows, source.vColorize })
        { scalar(value.x); scalar(value.y); scalar(value.z); }
        scalar(source.LutLayers.size());
        for (const auto& layer : source.LutLayers)
        {
            scalar(layer.fWeight);
            if (layer.pLut) stream << quoted(layer.pLut->strAssetId);
            else stream << "neutral-lut ";
        }
        scalar(q.fFXAASubpixel); scalar(q.fFXAAEdgeThreshold); scalar(q.fFXAAEdgeThresholdMin);
        const auto fog = game.Get_HeightFogSettings();
        scalar(fog.bEnabled); vector(fog.vColor); scalar(fog.fDensity); scalar(fog.fHeightFalloff);
        scalar(fog.fTopHeight); scalar(fog.fStartDistance); scalar(fog.fMaximumOpacity);
        scalar(fog.fDriftSpeed); scalar(fog.fDriftHeightAmplitude); scalar(fog.fDriftDensityAmplitude);
        scalar(fog.fCoveragePercent); scalar(fog.fWindDirectionX); scalar(fog.fWindDirectionZ);
        scalar(fog.fWindSpeed); scalar(fog.fPatchScale); scalar(fog.fPatchSoftness);
        scalar(fog.bSourceExponential); vector(fog.vInscatteringColor); vector(fog.vFogLightDirection);
        const auto environment = game.Get_RenderEnvironment();
        scalar(environment.strCubePath.size());
        for (const auto character : environment.strCubePath) scalar(static_cast<uint32_t>(character));
        vector(environment.vColor); vector(environment.vRotationIntensity);
        const auto& shadow = game.Get_ShadowLightDesc(); vector(shadow.vEye); vector(shadow.vAt);
        const auto& s = shadow.Settings; scalar(s.bEnabled); scalar(s.fOrthographicWidth);
        scalar(s.fOrthographicHeight); scalar(s.fNear); scalar(s.fFar); scalar(s.fDepthBias);
        scalar(s.fNormalBias); scalar(s.fStrength);
        const auto& lights = game.Get_SceneLights(); scalar(lights.size());
        for (const auto& light : lights)
        {
            scalar(static_cast<uint32_t>(light.eType)); vector(light.vDirection); vector(light.vPosition);
            scalar(light.fRange); scalar(light.fFalloffExponent); vector(light.vDiffuse);
            vector(light.vAmbient); vector(light.vSpecular); scalar(light.fSpotInnerCos); scalar(light.fSpotOuterCos);
            scalar(static_cast<uint32_t>(light.eReceiver)); scalar(light.staticShadowChannel);
        }
        return stream.str();
    }
	double Percentile(std::vector<double> Values, const double fPercentile)
	{
		if (Values.empty())
			return 0.0;
		std::sort(Values.begin(), Values.end());
		const double fRank = fPercentile * static_cast<double>(Values.size() - 1u);
		const size_t iLower = static_cast<size_t>(fRank);
		const size_t iUpper = (std::min)(iLower + 1u, Values.size() - 1u);
		const double fFraction = fRank - static_cast<double>(iLower);
		return Values[iLower] + (Values[iUpper] - Values[iLower]) * fFraction;
	}

	string Now_Timestamp()
	{
		const auto Now = chrono::system_clock::now();
		const time_t CalendarTime = chrono::system_clock::to_time_t(Now);
		tm LocalTime{};
		localtime_s(&LocalTime, &CalendarTime);
		ostringstream Stream;
		Stream << put_time(&LocalTime, "%Y-%m-%d %H:%M:%S");
		return Stream.str();
	}

	string Escape_Json(const string& Value)
	{
		string Escaped;
		Escaped.reserve(Value.size() + 8u);
		for (const char Character : Value)
		{
			switch (Character)
			{
			case '"': Escaped += "\\\""; break;
			case '\\': Escaped += "\\\\"; break;
			case '\n': Escaped += "\\n"; break;
			case '\r': Escaped += "\\r"; break;
			case '\t': Escaped += "\\t"; break;
			default: Escaped += Character; break;
			}
		}
		return Escaped;
	}
}

bool_t Client::CRenderingBenchmark::Begin(
	Engine::CProfiler* pProfiler,
	const string& strLabel,
	const uint32_t iFrames,
	const string& strQualitySummary,
	string& strOutStatus)
{
	if (nullptr == pProfiler)
	{
		strOutStatus = "Engine profiler is unavailable.";
		return false;
	}
	if (m_bCapturing)
	{
		strOutStatus = "A capture is already running.";
		return false;
	}
	/* A capture started from this ImGui frame must not measure the partially
	   elapsed activation frame. Keep one warm-up slot in addition to the GPU
	   readback tail inside the bounded history. */
	constexpr uint32_t MAXIMUM_CAPTURE_FRAMES =
		static_cast<uint32_t>(Engine::CProfiler::MAX_HISTORY_FRAMES) -
		Engine::CProfiler::GPU_READ_LATENCY - 1u;
	if (iFrames < 10u || iFrames > MAXIMUM_CAPTURE_FRAMES)
	{
		strOutStatus = "Frame count must be between 10 and " +
			std::to_string(MAXIMUM_CAPTURE_FRAMES) + ".";
		return false;
	}
	m_bProfilerWasEnabled = pProfiler->Is_Enabled();
	Engine::FProfilerLiveStats Live{};
	const bool_t bHadCompletedFrame = pProfiler->Get_LiveStats(Live);
	/* If profiling was already active, the frame containing this button click
	   will still be committed after Reset_History. Exclude it by identity. When
	   profiling starts here, its first measured frame begins on the next tick
	   and is a complete frame. */
	m_bSkipActivationFrame = m_bProfilerWasEnabled;
	m_iStartFrame = bHadCompletedFrame ?
		Live.FrameNumber + (m_bSkipActivationFrame ? 1u : 0u) :
		(m_bSkipActivationFrame ? 1u : 0u);
	pProfiler->Set_Enabled(true);
	pProfiler->Reset_History();
	m_iTargetFrames = iFrames;
	m_strLabel = strLabel.empty() ? "run" : strLabel;
	m_strQualitySummary = strQualitySummary;
	m_strComparisonConditions = ComparisonConditions();
	m_bSourceMaterials = Engine::CGameInstance::Get().Get_MaterialRenderSettings().bUseSourceMaterials;
	m_bConditionsStable = true;
	m_bCapturing = true;
	strOutStatus = "Capturing " + std::to_string(iFrames) + " frames for '" +
		m_strLabel + "'. Keep the camera still for a comparable result.";
	return true;
}

void Client::CRenderingBenchmark::Update(Engine::CProfiler* pProfiler)
{
	if (!m_bCapturing || nullptr == pProfiler)
		return;
	if (m_strComparisonConditions != ComparisonConditions() ||
        m_bSourceMaterials != Engine::CGameInstance::Get().Get_MaterialRenderSettings().bUseSourceMaterials)
        m_bConditionsStable = false;
	if (pProfiler->Get_HistoryFrameCount() <
		static_cast<size_t>(m_iTargetFrames) +
		Engine::CProfiler::GPU_READ_LATENCY +
		(m_bSkipActivationFrame ? 1u : 0u))
	{
		return;
	}
	(void)Finalize(*pProfiler);
}

bool_t Client::CRenderingBenchmark::Finalize(Engine::CProfiler& Profiler)
{
	m_bCapturing = false;
	const Engine::FProfilerCaptureSnapshot Snapshot = Profiler.Snapshot();
	Profiler.Set_Enabled(m_bProfilerWasEnabled);

	RENDERING_BENCHMARK_RUN Run;
	Run.strLabel = m_strLabel;
	Run.strTimestamp = Now_Timestamp();
	Run.strQualitySummary = m_strQualitySummary;
	Run.strComparisonConditions = m_strComparisonConditions;
	Run.bSourceMaterials = m_bSourceMaterials;
	Run.bConditionsStable = m_bConditionsStable;
	std::vector<double> CpuMs;
	std::vector<double> GpuMs;
	double fDrawCalls = 0.0;
	double fInstances = 0.0;
	double fIndices = 0.0;
	double fPsInvocations = 0.0;
	const uint64_t iNewestFrame = Snapshot.Frames.empty() ?
		0u : Snapshot.Frames.back().FrameNumber;
	for (const Engine::FProfilerFrame& Frame : Snapshot.Frames)
	{
		/* Exclude the activation frame and the unresolved GPU-readback tail.
		   Thus every reported CPU sample covers a complete post-click frame and
		   every selected frame has reached the profiler's normal resolve age. */
		if (Frame.FrameNumber <= m_iStartFrame ||
			iNewestFrame < Frame.FrameNumber + Engine::CProfiler::GPU_READ_LATENCY)
		{
			continue;
		}
		if (Run.iFrames >= m_iTargetFrames)
			break;
		++Run.iFrames;
		CpuMs.push_back(Frame.CpuFrameMs);
		fDrawCalls += static_cast<double>(
			Frame.Counters[static_cast<size_t>(Engine::EProfilerCounter::DrawCalls)]);
		fInstances += static_cast<double>(
			Frame.Counters[static_cast<size_t>(Engine::EProfilerCounter::Instances)]);
		fIndices += static_cast<double>(
			Frame.Counters[static_cast<size_t>(Engine::EProfilerCounter::Indices)]);
		if (Frame.GpuValid)
		{
			GpuMs.push_back(Frame.GpuFrameMs);
			fPsInvocations += static_cast<double>(Frame.Pipeline.PSInvocations);
		}
	}
	if (0u == Run.iFrames)
	{
		m_strStatus = "Capture ended without any profiled frame.";
		return false;
	}
	const double fFrames = static_cast<double>(Run.iFrames);
	Run.iGpuFrames = static_cast<uint32_t>(GpuMs.size());
	Run.fCpuAvgMs = std::accumulate(CpuMs.begin(), CpuMs.end(), 0.0) / fFrames;
	Run.fCpuP95Ms = Percentile(CpuMs, 0.95);
	Run.fCpuMaxMs = *std::max_element(CpuMs.begin(), CpuMs.end());
	if (!GpuMs.empty())
	{
		Run.fGpuAvgMs = std::accumulate(GpuMs.begin(), GpuMs.end(), 0.0) /
			static_cast<double>(GpuMs.size());
		Run.fGpuP95Ms = Percentile(GpuMs, 0.95);
		Run.fGpuMaxMs = *std::max_element(GpuMs.begin(), GpuMs.end());
		Run.fPsInvocationsAvg = fPsInvocations / static_cast<double>(GpuMs.size());
	}
	Run.fDrawCallsAvg = fDrawCalls / fFrames;
	Run.fInstancesAvg = fInstances / fFrames;
	Run.fIndicesAvg = fIndices / fFrames;
	m_Runs.push_back(Run);

	string Error;
	const filesystem::path OutputPath = Make_DefaultPath();
	ostringstream Summary;
	Summary << fixed << setprecision(3)
		<< "Recorded '" << Run.strLabel << "': CPU " << Run.fCpuAvgMs
		<< " ms avg, GPU " << Run.fGpuAvgMs << " ms avg. ";
	if (Save_Json(m_Runs, OutputPath, Error))
		Summary << "Saved " << OutputPath.string();
	else
		Summary << "JSON save failed: " << Error;
	m_strStatus = Summary.str();
	return true;
}

void Client::CRenderingBenchmark::Release_RestorationOwnership()
{
	m_strRestorationEntryProfileId.clear();
	m_strRestorationLastProfileId.clear();
	m_strRestorationLevelQualityId.clear();
	m_iRestorationLevel = 0u;
}

void Client::CRenderingBenchmark::Notify_ProfileReload()
{
	if (!m_strRestorationLastProfileId.empty())
	{
		Release_RestorationOwnership();
		m_strRestorationStatus = "Runtime reloaded. Comparison ownership released; the reloaded scene stays active.";
	}
	if (m_bCapturing)
		m_bConditionsStable = false;
}

void Client::CRenderingBenchmark::Update_RestorationPreview(
	CRenderingProfileService& Profiles, const bool_t bToolVisible)
{
	if (!bToolVisible)
    {
        Profiles.Clear_ComparisonOptions();
		if (auto* arena = CLevel_KakulSaydonArena::Get_Active())
			arena->Reset_MapLightComparison();
    }
	if (m_strRestorationLastProfileId.empty())
		return;
	if (Engine::CGameInstance::Get().Get_CurrentLevelID() != m_iRestorationLevel ||
		Profiles.Get_ActiveProfileId() != m_strRestorationLastProfileId ||
		Profiles.Get_LevelQualityProfileId() != m_strRestorationLevelQualityId)
	{
		Release_RestorationOwnership();
		m_strRestorationStatus = "Level or scene owner changed. Comparison released without changing the new scene.";
		return;
	}
	if (!bToolVisible && !Return_ToEntryProfile(Profiles))
	{
		// A hidden tool must not retry a failing renderer transaction every frame.
		Release_RestorationOwnership();
		m_strRestorationStatus += " Comparison ownership released; the current scene is retained.";
	}
}

bool_t Client::CRenderingBenchmark::Activate_RestorationProfile(
	CRenderingProfileService& Profiles, const string& strProfileId)
{
	if (m_bCapturing)
		return false;
	Update_RestorationPreview(Profiles, true);
	const string previous = Profiles.Get_ActiveProfileId();
	if (previous.empty() || !Profiles.Has_Profile(previous))
	{
		m_strRestorationStatus = "Cannot compare without an available entry profile.";
		return false;
	}
	if (!Profiles.Activate_Profile(strProfileId, m_strRestorationStatus))
		return false;
	if (m_strRestorationLastProfileId.empty())
	{
		m_strRestorationEntryProfileId = previous;
		m_iRestorationLevel = Engine::CGameInstance::Get().Get_CurrentLevelID();
		m_strRestorationLevelQualityId = Profiles.Get_LevelQualityProfileId();
	}
	m_strRestorationLastProfileId = strProfileId;
	m_strRestorationStatus = "Session profile applied. Return to entry or close this workbench to restore the entry profile.";
	return true;
}

bool_t Client::CRenderingBenchmark::Return_ToEntryProfile(CRenderingProfileService& Profiles)
{
	auto* arena = CLevel_KakulSaydonArena::Get_Active();
	const bool_t lightsChanged = arena &&
		arena->Get_MapLightComparison() != CLevel_KakulSaydonArena::MAP_LIGHT_COMPARISON::CURRENT;
	if (m_strRestorationLastProfileId.empty())
	{
		if (arena) arena->Reset_MapLightComparison();
		if (lightsChanged) m_strRestorationStatus = "Current map lights restored.";
		return lightsChanged;
	}
	if (Engine::CGameInstance::Get().Get_CurrentLevelID() != m_iRestorationLevel ||
		Profiles.Get_ActiveProfileId() != m_strRestorationLastProfileId ||
		Profiles.Get_LevelQualityProfileId() != m_strRestorationLevelQualityId)
	{
		Release_RestorationOwnership();
		m_strRestorationStatus = "The scene is now owned elsewhere. Entry restoration was skipped.";
		return false;
	}
	if (!Profiles.Activate_Profile(m_strRestorationEntryProfileId, m_strRestorationStatus))
		return false;
	if (arena) arena->Reset_MapLightComparison();
	Release_RestorationOwnership();
	if (m_bCapturing)
		m_bConditionsStable = false;
	m_strRestorationStatus = "Entry profile restored.";
	return true;
}

bool_t Client::CRenderingBenchmark::Render_RestorationSection(CRenderingProfileService& Profiles)
{
	Update_RestorationPreview(Profiles, true);
	ImGui::SeparatorText("Rendering restoration");
	const auto& game = Engine::CGameInstance::Get();
	const char* beforeId = nullptr;
	const char* restoredId = nullptr;
	switch (static_cast<LEVEL>(game.Get_CurrentLevelID()))
	{
	case LEVEL::BERN:
		beforeId = "scene.bern.before-restoration.v1";
		restoredId = "scene.bern.source-rendering.v1";
		break;
	case LEVEL::CHARACTER_SELECT:
		beforeId = "scene.character-select.before-restoration.v1";
		restoredId = "scene.character-select.source-rendering.v1";
		break;
	case LEVEL::VALTAN_ARENA:
		beforeId = "scene.valtan.before-restoration.v1";
		restoredId = "scene.valtan.source-rendering.v1";
		break;
	case LEVEL::KAKULSAYDON_ARENA:
		beforeId = "scene.kakulsaydon.before-restoration.v1";
		restoredId = "scene.kakulsaydon.source-rendering.v1";
		break;
	default:
		break;
	}
	bool_t changed = false;
	if (auto* arena = CLevel_KakulSaydonArena::Get_Active();
		arena && game.Get_CurrentLevelID() == ETOUI(LEVEL::KAKULSAYDON_ARENA))
	{
		struct AREA_PROFILE final { const char* label; const char* id; };
		static constexpr AREA_PROFILE areaProfiles[] = {
			{ "Start area - source", "scene.kakulsaydon.compare.start.v1" },
			{ "Gate 1 - source LUT02", "scene.kakulsaydon.compare.gate1.v1" },
			{ "Gate 2 - source", "scene.kakulsaydon.compare.gate2.v1" },
			{ "Gate 3 - source LUT01", "scene.kakulsaydon.compare.gate3.v1" },
			{ "Card maze - current baseline", "scene.kakulsaydon.compare.card-maze.v1" }
		};
		const char* areaLabel = "Current scene";
		for (const auto& item : areaProfiles)
			if (Profiles.Get_ActiveProfileId() == item.id) areaLabel = item.label;
		ImGui::BeginDisabled(m_bCapturing);
		if (ImGui::BeginCombo("Kouku area profile", areaLabel))
		{
			for (const auto& item : areaProfiles)
			{
				const bool available = Profiles.Has_Profile(item.id);
				ImGui::BeginDisabled(!available);
				if (ImGui::Selectable(item.label, Profiles.Get_ActiveProfileId() == item.id))
					changed = Activate_RestorationProfile(Profiles, item.id) || changed;
				ImGui::EndDisabled();
				if (!available && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
					ImGui::SetTooltip("This comparison profile has not been published.");
			}
			ImGui::EndCombo();
		}
		ImGui::EndDisabled();
		ImGui::TextWrapped("Applies a fixed area look to this view. Player position and raid state stay where they are. Card maze keeps its current baseline.");
		int selection = static_cast<int>(arena->Get_MapLightComparison());
		ImGui::BeginDisabled(m_bCapturing);
		if (ImGui::Combo("Map light comparison", &selection,
			"Current authored lights\0Imported source lights (2026-09-11)\0Map lights off\0"))
			changed = arena->Set_MapLightComparison(
				static_cast<CLevel_KakulSaydonArena::MAP_LIGHT_COMPARISON>(selection), m_strRestorationStatus);
		ImGui::EndDisabled();
		ImGui::TextWrapped("Source comparison uses 115 imported local lights with current receiver routing and gate placement. Scene and Effect lights remain active.");
		ImGui::TextWrapped("This session choice preserves authored lights. Closing this workbench restores them. Compare tone and grading with the profiles below.");
	}
	if (beforeId)
	{
		const bool beforeAvailable = Profiles.Has_Profile(beforeId);
		const bool restoredAvailable = Profiles.Has_Profile(restoredId);
		ImGui::BeginDisabled(m_bCapturing || !beforeAvailable);
		if (ImGui::Button("Before"))
			changed = Activate_RestorationProfile(Profiles, beforeId);
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::BeginDisabled(m_bCapturing || !restoredAvailable);
		if (ImGui::Button("Restored source profile"))
			changed = Activate_RestorationProfile(Profiles, restoredId) || changed;
		ImGui::EndDisabled();
		if (!beforeAvailable || !restoredAvailable)
			ImGui::TextWrapped("A comparison profile is unavailable. Source applicability and published inputs must be confirmed for this map.");
	}
	else
		ImGui::TextWrapped("Source rendering applicability for this map is not confirmed. Comparison profiles are unavailable.");
	const auto* arena = CLevel_KakulSaydonArena::Get_Active();
	const bool lightsCompared = arena &&
		arena->Get_MapLightComparison() != CLevel_KakulSaydonArena::MAP_LIGHT_COMPARISON::CURRENT;
	ImGui::BeginDisabled(m_bCapturing || (m_strRestorationLastProfileId.empty() && !lightsCompared));
	if (ImGui::Button("Return to entry"))
		changed = Return_ToEntryProfile(Profiles) || changed;
	ImGui::EndDisabled();
	if (!m_strRestorationEntryProfileId.empty())
		ImGui::TextWrapped("Entry profile: %s", m_strRestorationEntryProfileId.c_str());
	ImGui::TextWrapped("Active profile: %s", Profiles.Get_ActiveProfileId().c_str());
	const auto& regionId = Profiles.Get_AppliedEnvironmentRegionId();
	ImGui::TextWrapped("Camera environment: %s", regionId.empty() ? "fixed profile (no region override)" : regionId.c_str());
	if (const auto* camera = Engine::CGameInstance::Get().Get_CamPosition())
		ImGui::Text("Camera XYZ %.3f / %.3f / %.3f", camera->x, camera->y, camera->z);
	const auto quality = game.Get_RenderQualitySettings();
	const auto fog = game.Get_HeightFogSettings();
	ImGui::Text("Exposure %.4f | Bloom %s: %.4f", quality.fExposure,
		quality.bBloomEnabled ? "on" : "off", quality.fBloomIntensity);
	ImGui::Text("Bloom threshold %.4f | Desaturation %.4f",
		quality.fBloomThreshold, quality.fSceneDesaturation);
	const auto& source = quality.SourcePostProcess;
	ImGui::Text("Tone mapping: %s", source.bEnabled ? "Source UE3 customizable" : "Hable");
	if (source.bEnabled)
	{
		ImGui::Text("Source tone scale %.4f | range %.4f | toe %.4f | desaturation %.4f",
			source.fToneScale, source.fToneRange, source.fToneToe, source.fDesaturation);
		for (const auto& layer : source.LutLayers)
			ImGui::TextWrapped("LUT %.3f: %s", layer.fWeight,
				layer.pLut ? layer.pLut->strAssetId.c_str() : "neutral");
	}
	ImGui::Text("Bloom tint RGB %.3f / %.3f / %.3f",
		quality.vBloomTint.x, quality.vBloomTint.y, quality.vBloomTint.z);
	ImGui::Text("Fog %s (%s) | density %.5f", fog.bEnabled ? "on" : "off",
		fog.bSourceExponential ? "source exponential" : "project height", fog.fDensity);
	for (const auto& light : game.Get_SceneLights())
	{
		if (light.eType != Engine::LIGHT::DIRECTIONAL)
			continue;
		ImGui::Text("Directional RGB %.3f / %.3f / %.3f",
			light.vDiffuse.x, light.vDiffuse.y, light.vDiffuse.z);
		ImGui::Text("Ambient RGB %.3f / %.3f / %.3f",
			light.vAmbient.x, light.vAmbient.y, light.vAmbient.z);
		ImGui::Text("Directional specular RGB %.3f / %.3f / %.3f",
			light.vSpecular.x, light.vSpecular.y, light.vSpecular.z);
		break;
	}
	ImGui::TextWrapped("%s", m_strRestorationStatus.c_str());
	ImGui::TextWrapped("Native inputs: recovered scene lights, baked RNM and source fog where verified. Receiver separation remains part of the map.");
	ImGui::TextWrapped("Source profiles use the recovered tone curve and LUT grading when enabled. Other profiles retain their saved tone mapping.");
	ImGui::TextWrapped("Bloom kernel, DOF, light shafts and map effects have separate restoration scopes; this comparison does not certify the whole scene.");
	ImGui::TextDisabled("Session only. No automatic Save or Publish. Profile switching is locked during capture.");
	return changed;
}

void Client::CRenderingBenchmark::Render_Section(
	Engine::CProfiler* pProfiler,
	const string& strQualitySummary,
	CRenderingProfileService& Profiles)
{
	ImGui::SeparatorText("Benchmark");
	const bool_t profileChanged = Render_RestorationSection(Profiles);
	ImGui::SeparatorText("Material performance capture");
	ImGui::SetNextItemWidth(160.f);
	ImGui::InputText("Label", m_LabelBuffer.data(), m_LabelBuffer.size());
	ImGui::SameLine();
	ImGui::SetNextItemWidth(120.f);
	ImGui::DragInt("Frames", &m_iFrameInput, 1.f, 10,
		static_cast<int32_t>(Engine::CProfiler::MAX_HISTORY_FRAMES -
			Engine::CProfiler::GPU_READ_LATENCY - 1u), "%d",
		ImGuiSliderFlags_AlwaysClamp);
	ImGui::SameLine();
	ImGui::BeginDisabled(m_bCapturing || profileChanged);
	if (ImGui::Button("Capture"))
	{
		(void)Begin(pProfiler, m_LabelBuffer.data(),
			static_cast<uint32_t>(m_iFrameInput), strQualitySummary, m_strStatus);
	}
	ImGui::SameLine();
    for (const bool recovered : {false, true})
    {
        if (ImGui::Button(recovered ? "Capture B: recovered" : "Capture A: legacy"))
        {
            auto& game = Engine::CGameInstance::Get();
            const auto previous = game.Get_MaterialRenderSettings();
            auto selected = previous; selected.bUseSourceMaterials = recovered;
            if (FAILED(game.Apply_MaterialRenderSettings(selected)))
                m_strStatus = "Could not apply material comparison mode.";
            else if (!Begin(pProfiler, string(m_LabelBuffer.data()) + (recovered ? " B" : " A"),
                static_cast<uint32_t>(m_iFrameInput), strQualitySummary, m_strStatus))
                (void)game.Apply_MaterialRenderSettings(previous);
        }
        ImGui::SameLine();
    }
	ImGui::EndDisabled();
	if (ImGui::Button("Clear runs"))
		m_Runs.clear();
	if (m_bCapturing && nullptr != pProfiler)
	{
		ImGui::Text("Capturing: %zu / %u frames",
			pProfiler->Get_HistoryFrameCount(), m_iTargetFrames);
	}
	ImGui::TextWrapped("%s", m_strStatus.c_str());
	ImGui::TextDisabled("Current settings: %s", strQualitySummary.c_str());
    ImGui::TextDisabled("A/B keeps the current camera and lighting. Stop animations for a static map comparison.");
	if (m_Runs.empty())
		return;
    const auto& latest = m_Runs.back();
    const RENDERING_BENCHMARK_RUN* counterpart = nullptr;
    for (auto it = m_Runs.rbegin() + 1; it != m_Runs.rend(); ++it)
        if (it->bSourceMaterials != latest.bSourceMaterials && it->bConditionsStable && latest.bConditionsStable &&
            it->strComparisonConditions == latest.strComparisonConditions && it->strQualitySummary == latest.strQualitySummary)
        { counterpart = &*it; break; }
    if (!latest.bConditionsStable)
        ImGui::TextWrapped("Comparison excluded: camera, viewport, lighting or render settings changed during this run.");
    else if (counterpart)
    {
        const auto& a = latest.bSourceMaterials ? *counterpart : latest;
        const auto& b = latest.bSourceMaterials ? latest : *counterpart;
        ImGui::Text("B - A CPU: %+.3f ms | Draw calls: %+.0f", b.fCpuAvgMs-a.fCpuAvgMs, b.fDrawCallsAvg-a.fDrawCallsAvg);
        if (a.iGpuFrames == a.iFrames && b.iGpuFrames == b.iFrames)
            ImGui::Text("B - A GPU: %+.3f ms | PS invocations: %+.0f", b.fGpuAvgMs-a.fGpuAvgMs, b.fPsInvocationsAvg-a.fPsInvocationsAvg);
        else ImGui::TextDisabled("GPU comparison unavailable: one run has incomplete GPU samples.");
    }
    else ImGui::TextDisabled("Capture the other material mode with identical camera, viewport and render settings.");
	constexpr ImGuiTableFlags TABLE_FLAGS = ImGuiTableFlags_Borders |
		ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp;
	if (!ImGui::BeginTable("##RenderingBenchmarkRuns", 9, TABLE_FLAGS))
		return;
	ImGui::TableSetupColumn("Label");
	ImGui::TableSetupColumn("Frames");
	ImGui::TableSetupColumn("CPU avg");
	ImGui::TableSetupColumn("CPU p95");
	ImGui::TableSetupColumn("GPU avg");
	ImGui::TableSetupColumn("GPU p95");
	ImGui::TableSetupColumn("Draw calls");
	ImGui::TableSetupColumn("Instances");
	ImGui::TableSetupColumn("Settings");
	ImGui::TableHeadersRow();
	for (const RENDERING_BENCHMARK_RUN& Run : m_Runs)
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text("%s [%s%s]", Run.strLabel.c_str(), Run.bSourceMaterials ? "B" : "A",
            Run.bConditionsStable ? "" : ", changed");
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("%s", Run.strTimestamp.c_str());
		ImGui::TableNextColumn();
		ImGui::Text("%u", Run.iFrames);
		ImGui::TableNextColumn();
		ImGui::Text("%.3f", Run.fCpuAvgMs);
		ImGui::TableNextColumn();
		ImGui::Text("%.3f", Run.fCpuP95Ms);
		ImGui::TableNextColumn();
		ImGui::Text("%.3f", Run.fGpuAvgMs);
		ImGui::TableNextColumn();
		ImGui::Text("%.3f", Run.fGpuP95Ms);
		ImGui::TableNextColumn();
		ImGui::Text("%.0f", Run.fDrawCallsAvg);
		ImGui::TableNextColumn();
		ImGui::Text("%.0f", Run.fInstancesAvg);
		ImGui::TableNextColumn();
		ImGui::TextUnformatted(Run.strQualitySummary.c_str());
	}
	ImGui::EndTable();
}

bool_t Client::CRenderingBenchmark::Save_Json(
	const vector<RENDERING_BENCHMARK_RUN>& Runs,
	const filesystem::path& OutputPath,
	string& strOutError)
{
	error_code Error;
	if (!OutputPath.parent_path().empty())
		filesystem::create_directories(OutputPath.parent_path(), Error);
	if (Error)
	{
		strOutError = Error.message();
		return false;
	}
	filesystem::path TemporaryPath = OutputPath;
	TemporaryPath += L".tmp";
	ofstream Stream(TemporaryPath, ios::binary | ios::trunc);
	if (!Stream)
	{
		strOutError = "Cannot open benchmark JSON output.";
		return false;
	}
	Stream << fixed << setprecision(6);
	Stream << "{\n  \"schema\": \"LostArkRenderingBenchmark.v1\",\n  \"runs\": [\n";
	for (size_t iRun = 0u; iRun < Runs.size(); ++iRun)
	{
		const RENDERING_BENCHMARK_RUN& Run = Runs[iRun];
		Stream << "    {\n"
			<< "      \"label\": \"" << Escape_Json(Run.strLabel) << "\",\n"
			<< "      \"timestamp\": \"" << Escape_Json(Run.strTimestamp) << "\",\n"
			<< "      \"qualitySummary\": \"" << Escape_Json(Run.strQualitySummary) << "\",\n"
			<< "      \"materialMode\": \"" << (Run.bSourceMaterials ? "RECOVERED_B" : "LEGACY_A") << "\",\n"
            << "      \"conditionsStable\": " << (Run.bConditionsStable ? "true" : "false") << ",\n"
            << "      \"comparisonConditions\": \"" << Escape_Json(Run.strComparisonConditions) << "\",\n"
			<< "      \"frames\": " << Run.iFrames << ",\n"
			<< "      \"gpuFrames\": " << Run.iGpuFrames << ",\n"
			<< "      \"cpuAvgMs\": " << Run.fCpuAvgMs << ",\n"
			<< "      \"cpuP95Ms\": " << Run.fCpuP95Ms << ",\n"
			<< "      \"cpuMaxMs\": " << Run.fCpuMaxMs << ",\n"
			<< "      \"gpuAvgMs\": " << Run.fGpuAvgMs << ",\n"
			<< "      \"gpuP95Ms\": " << Run.fGpuP95Ms << ",\n"
			<< "      \"gpuMaxMs\": " << Run.fGpuMaxMs << ",\n"
			<< "      \"drawCallsAvg\": " << Run.fDrawCallsAvg << ",\n"
			<< "      \"instancesAvg\": " << Run.fInstancesAvg << ",\n"
			<< "      \"indicesAvg\": " << Run.fIndicesAvg << ",\n"
			<< "      \"psInvocationsAvg\": " << Run.fPsInvocationsAvg << "\n"
			<< "    }" << (iRun + 1u < Runs.size() ? "," : "") << "\n";
	}
	Stream << "  ]\n}\n";
	Stream.close();
	if (!Stream)
	{
		filesystem::remove(TemporaryPath, Error);
		strOutError = "Failed while writing benchmark JSON.";
		return false;
	}
	filesystem::rename(TemporaryPath, OutputPath, Error);
	if (Error)
	{
		filesystem::remove(TemporaryPath, Error);
		strOutError = "Cannot finalize benchmark JSON output.";
		return false;
	}
	return true;
}

filesystem::path Client::CRenderingBenchmark::Make_DefaultPath()
{
	wchar_t ModulePath[32768]{};
	const DWORD iLength = GetModuleFileNameW(
		nullptr, ModulePath, static_cast<DWORD>(size(ModulePath)));
	const filesystem::path BaseDirectory =
		0 != iLength && iLength < size(ModulePath) ?
		filesystem::path(ModulePath).parent_path() : filesystem::current_path();
	const auto Now = chrono::system_clock::now();
	const time_t CalendarTime = chrono::system_clock::to_time_t(Now);
	tm LocalTime{};
	localtime_s(&LocalTime, &CalendarTime);
	wostringstream FileName;
	FileName << L"benchmark_" << put_time(&LocalTime, L"%Y%m%d_%H%M%S") << L".json";
	return (BaseDirectory / L".." / L"BenchmarkCaptures" / FileName.str())
		.lexically_normal();
}
