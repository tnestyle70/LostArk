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
#include "MapAssetRenderUtils.h"

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
        const auto material = game.Get_MaterialRenderSettings();
        scalar(static_cast<uint32_t>(material.eDebugView));
        scalar(material.MapPBR.bEnabled); scalar(material.MapPBR.iLevel);
        vector(material.MapPBR.vContributionScale); vector(material.MapPBR.vSurfaceParameters);
        const auto q = game.Get_RenderQualitySettings();
        scalar(q.bSSAOEnabled); scalar(q.fSSAORadius); scalar(q.fSSAOBias);
        scalar(q.fSSAOIntensity); scalar(q.fSSAOPower); scalar(q.fSSAODistanceFade);
        scalar(q.bBloomEnabled); scalar(q.fBloomThreshold); scalar(q.fBloomSoftKnee);
        scalar(q.fBloomIntensity); scalar(q.fBloomScatter); scalar(q.fExposure);
        scalar(q.fWhitePoint); scalar(q.fGamma); scalar(q.bFXAAEnabled);
        vector(q.vBloomTint); scalar(q.fSceneDesaturation);
        scalar(q.iColorFilterType); scalar(q.fColorFilterStrength);
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
        scalar(s.fNormalBias); scalar(s.fStrength); scalar(s.fDynamicBakedStrength);
        const auto& lights = game.Get_SceneLights(); scalar(lights.size());
        for (const auto& light : lights)
        {
            scalar(static_cast<uint32_t>(light.eType)); vector(light.vDirection); vector(light.vPosition);
            scalar(light.fRange); scalar(light.fFalloffExponent); vector(light.vDiffuse);
            vector(light.vAmbient); vector(light.vSpecular); scalar(light.fSpotInnerCos); scalar(light.fSpotOuterCos);
            vector(light.vSourceCharacterAmbient);
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
    auto& game = Engine::CGameInstance::Get();
    if (m_bPixelDiagnosticsActive && (!bToolVisible || game.Get_CurrentLevelID() != m_iPixelDiagnosticsLevel))
    {
        (void)game.Apply_MaterialRenderSettings(m_PixelEntrySettings);
        m_bPixelDiagnosticsActive = false;
        m_strPixelMaterialKey.clear();
    }
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

bool_t Client::CRenderingBenchmark::Render_PixelInputs()
{
    auto& game = Engine::CGameInstance::Get();
    auto settings = game.Get_MaterialRenderSettings();
    ImGui::SeparatorText("Pixel rendering inputs");
    ImGui::TextWrapped("Compare the whole scene first, then inspect a material contribution. Values below are the active shader inputs; textures still vary per pixel.");
    static constexpr const char* views[] = {
        "Final image", "Material base color", "Material normal", "Direct specular",
        "Reflection texture delta", "PBR roughness", "PBR metallic", "PBR material AO",
        "PBR baked diffuse (RNM)", "PBR environment specular", "PBR diffuse light (direct + unbaked ambient)",
        "Whole scene: HDR before final tone", "Whole scene: tone before grading",
        "Whole scene: grading before FXAA"
    };
    static_assert(std::size(views) == static_cast<size_t>(Engine::MATERIAL_DEBUG_VIEW::END));
    ImGui::BeginDisabled(m_bCapturing);
    int view = static_cast<int>(settings.eDebugView);
    bool changed = ImGui::Combo("Pipeline view", &view, views, static_cast<int>(std::size(views)));
    settings.eDebugView = static_cast<Engine::MATERIAL_DEBUG_VIEW>(view);
    changed |= ImGui::Checkbox("Recovered material equations", &settings.bUseSourceMaterials);
    if (view >= 11)
        ImGui::TextWrapped("Whole-scene views include characters and background. HDR uses one RGB scale for display; tone and grading views use the current scene settings. These are stage comparisons, not color-corrected presets.");
    else if (view >= 8)
        ImGui::TextWrapped("PBR contribution only. Other material families appear black. RNM and environment views precede screen AO and moving-caster shadow modulation.");

    if (ImGui::CollapsingHeader("PBR contribution comparison", ImGuiTreeNodeFlags_DefaultOpen))
    {
        changed |= ImGui::Checkbox("Enable comparison in this Level", &settings.MapPBR.bEnabled);
        settings.MapPBR.iLevel = game.Get_CurrentLevelID();
        ImGui::BeginDisabled(!settings.MapPBR.bEnabled || !settings.bUseSourceMaterials);
        auto& gains = settings.MapPBR.vContributionScale;
        auto& surface = settings.MapPBR.vSurfaceParameters;
        changed |= ImGui::SliderFloat("Diffuse lighting contribution", &gains.x, 0.f, 4.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        changed |= ImGui::SliderFloat("Direct specular contribution", &gains.y, 0.f, 4.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        changed |= ImGui::SliderFloat("Baked diffuse contribution (RNM)", &gains.z, 0.f, 4.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        changed |= ImGui::SliderFloat("Environment specular contribution", &gains.w, 0.f, 4.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        changed |= ImGui::SliderFloat("Normal strength multiplier", &surface.x, 0.f, 4.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        changed |= ImGui::SliderFloat("Roughness offset", &surface.y, -1.f, 1.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        bool legacy = surface.z != 0.f;
        if (ImGui::Checkbox("Compare previous RNM without energy split", &legacy))
        {
            surface.z = legacy ? 1.f : 0.f;
            changed = true;
        }
        ImGui::EndDisabled();
        if (ImGui::Button("Reset pixel comparison"))
        {
            settings = m_bPixelDiagnosticsActive ? m_PixelEntrySettings : Engine::MATERIAL_RENDER_SETTINGS{};
            changed = true;
        }
        ImGui::TextWrapped("These controls affect PBR map receivers. Source character shaders keep their own equations. Defaults use the recovered metallic/BRDF energy split; source SH and hemisphere inputs remain unresolved.");
        ImGui::TextDisabled("Diffuse lighting also includes the ambient fallback on surfaces without baked lighting.");
        ImGui::TextDisabled("Session only. Closing the workbench or changing Level clears this comparison. No Save or Publish.");
    }
    ImGui::EndDisabled();
    if (changed)
    {
        const auto previous = game.Get_MaterialRenderSettings();
        if (FAILED(game.Apply_MaterialRenderSettings(settings)))
            m_strRestorationStatus = "Invalid pixel comparison input; previous renderer state preserved.";
        else
        {
            if (!m_bPixelDiagnosticsActive) m_PixelEntrySettings = previous;
            m_bPixelDiagnosticsActive = true;
            m_iPixelDiagnosticsLevel = game.Get_CurrentLevelID();
        }
    }

    const auto number = [](const char* name, double value, const char* use = "active") {
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(name);
        ImGui::TableSetColumnIndex(1); ImGui::Text("%.6g", value);
        ImGui::TableSetColumnIndex(2); ImGui::TextUnformatted(use);
    };
    const auto vec = [](const char* name, const auto& v, const char* use = "active") {
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(name);
        ImGui::TableSetColumnIndex(1); ImGui::Text("%.6g / %.6g / %.6g", v.x, v.y, v.z);
        ImGui::TableSetColumnIndex(2); ImGui::TextUnformatted(use);
    };
    const auto table = [](const char* id) {
        if (!ImGui::BeginTable(id, 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_Resizable)) return false;
        ImGui::TableSetupColumn("Input"); ImGui::TableSetupColumn("Applied value");
        ImGui::TableSetupColumn("Use"); ImGui::TableHeadersRow(); return true;
    };
    if (ImGui::CollapsingHeader("Whole-scene composition inputs", ImGuiTreeNodeFlags_DefaultOpen) && table("ActiveScenePixelInputs"))
    {
        const auto q = game.Get_RenderQualitySettings();
        const auto& tone = q.SourcePostProcess;
        number("Exposure", q.fExposure);
        number("Gamma", q.fGamma, tone.bEnabled ? "source LUT bake exponent = 2.2 / gamma" : "display exponent = 1 / gamma");
        number("Hable white point", q.fWhitePoint, tone.bEnabled ? "inactive: source tone" : "active");
        const char* source = tone.bEnabled ? "active" : "inactive: Hable tone";
        number("Source tone scale", tone.fToneScale, source); number("Source tone range", tone.fToneRange, source);
        number("Source tone toe", tone.fToneToe, source); number("Source desaturation", tone.fDesaturation, source);
        vec("Source shadows", tone.vShadows, source); vec("Source highlights", tone.vHighlights, source);
        vec("Source midtones", tone.vMidtones, source); vec("Source colorize", tone.vColorize, source);
        number("Grading LUT layers", static_cast<double>(tone.LutLayers.size()), source);
        for (size_t i = 0; i < tone.LutLayers.size(); ++i)
        {
            const auto& layer = tone.LutLayers[i];
            number(("LUT weight " + std::to_string(i)).c_str(), layer.fWeight,
                layer.pLut ? layer.pLut->strAssetId.c_str() : "neutral lattice");
        }
        number("Display desaturation", q.fSceneDesaturation, tone.bEnabled ? "inactive: source grading" : "active");
        const char* bloom = q.bBloomEnabled ? "active" : "disabled";
        number("Bloom enabled", q.bBloomEnabled); number("Bloom intensity", q.fBloomIntensity, bloom);
        number("Bloom threshold", q.fBloomThreshold, bloom); number("Bloom soft knee", q.fBloomSoftKnee, bloom);
        number("Bloom scatter", q.fBloomScatter, bloom); vec("Bloom RGB tint", q.vBloomTint, bloom);
        const char* ao = q.bSSAOEnabled ? "active" : "disabled";
        number("SSAO enabled", q.bSSAOEnabled); number("SSAO radius (m)", q.fSSAORadius, ao);
        number("SSAO bias (m)", q.fSSAOBias, ao); number("SSAO intensity", q.fSSAOIntensity, ao);
        number("SSAO power", q.fSSAOPower, ao); number("SSAO distance fade", q.fSSAODistanceFade, ao);
        const char* aa = q.bFXAAEnabled ? "active" : "disabled";
        number("FXAA enabled", q.bFXAAEnabled); number("FXAA subpixel", q.fFXAASubpixel, aa);
        number("FXAA edge threshold", q.fFXAAEdgeThreshold, aa); number("FXAA minimum threshold", q.fFXAAEdgeThresholdMin, aa);
        number("Color filter type", q.iColorFilterType, tone.bEnabled ? "inactive: source return path" : "active");
        number("Color filter strength", q.fColorFilterStrength, tone.bEnabled ? "inactive: source return path" : "active");
        const auto env = game.Get_RenderEnvironment();
        vec("Scene environment RGB", env.vColor, env.pCube ? "bound scene cube; map MICs may use per-material cube" : "no scene cube");
        number("Scene environment floor", env.vColor.w);
        vec("Scene environment rotation/intensity", env.vRotationIntensity);
        const auto shadow = game.Get_ShadowLightDesc().Settings;
        const char* sh = shadow.bEnabled ? "active" : "disabled";
        number("Shadow enabled", shadow.bEnabled); number("Shadow depth bias", shadow.fDepthBias, sh);
        number("Shadow normal bias (m)", shadow.fNormalBias, sh); number("Shadow strength", shadow.fStrength, sh);
        number("Moving shadow on baked PBR", shadow.fDynamicBakedStrength, sh);
        number("Shadow coverage width (m)", shadow.fOrthographicWidth, sh);
        number("Shadow coverage height (m)", shadow.fOrthographicHeight, sh);
        number("Shadow near depth (m)", shadow.fNear, sh); number("Shadow far depth (m)", shadow.fFar, sh);
        const auto fog = game.Get_HeightFogSettings();
        const char* f = fog.bEnabled ? "active" : "disabled";
        number("Fog enabled", fog.bEnabled); number("Source exponential fog", fog.bSourceExponential, f);
        number("Fog density", fog.fDensity, f); number("Fog height falloff", fog.fHeightFalloff, f);
        number("Fog top height (m)", fog.fTopHeight, f); number("Fog start distance (m)", fog.fStartDistance, f);
        number("Fog maximum opacity", fog.fMaximumOpacity, f); vec("Fog color", fog.vColor, f);
        vec("Source fog inscattering", fog.vInscatteringColor, f); vec("Source fog light direction", fog.vFogLightDirection, f);
        number("Fog light terminator cosine", fog.vFogLightDirection.w, f);
        const char* drift = !fog.bEnabled ? "disabled" : (fog.bSourceExponential ? "inactive: source exponential" : "project height fog");
        number("Fog drift speed", fog.fDriftSpeed, drift); number("Fog drift height", fog.fDriftHeightAmplitude, drift);
        number("Fog drift density", fog.fDriftDensityAmplitude, drift); number("Fog coverage", fog.fCoveragePercent, drift);
        number("Fog wind X", fog.fWindDirectionX, drift); number("Fog wind Z", fog.fWindDirectionZ, drift);
        number("Fog wind speed", fog.fWindSpeed, drift); number("Fog patch scale", fog.fPatchScale, drift);
        number("Fog patch softness", fog.fPatchSoftness, drift);
        ImGui::EndTable();
    }
    if (ImGui::CollapsingHeader("Active scene light inputs") && table("SceneLightPixelInputs"))
    {
        size_t index = 0;
        for (const auto& light : game.Get_SceneLights())
        {
            const string prefix = "Light " + std::to_string(index++) + " ";
            number((prefix + "type").c_str(), static_cast<uint32_t>(light.eType));
            vec((prefix + "diffuse RGB").c_str(), light.vDiffuse);
            vec((prefix + "ambient RGB").c_str(), light.vAmbient, "baked PBR excludes duplicate ambient");
            vec((prefix + "character ambient RGB").c_str(), light.vSourceCharacterAmbient, "explicit character receiver input; not source SH");
            number((prefix + "character ambient override").c_str(), light.vSourceCharacterAmbient.w);
            vec((prefix + "specular RGB").c_str(), light.vSpecular, "native PBR uses incoming diffuse RGB instead");
            vec((prefix + "direction").c_str(), light.vDirection);
            vec((prefix + "position (m)").c_str(), light.vPosition);
            number((prefix + "range (m)").c_str(), light.fRange);
            number((prefix + "falloff exponent").c_str(), light.fFalloffExponent);
            number((prefix + "spot inner cosine").c_str(), light.fSpotInnerCos);
            number((prefix + "spot outer cosine").c_str(), light.fSpotOuterCos);
            number((prefix + "receiver").c_str(), static_cast<uint32_t>(light.eReceiver));
            number((prefix + "static shadow channel").c_str(), light.staticShadowChannel);
        }
        ImGui::EndTable();
    }
    if (ImGui::CollapsingHeader("Bound PBR material inputs"))
    {
        auto bindings = CMapAssetRenderUtils::Get_RecentSurfaceBindings();
        bindings.erase(std::remove_if(bindings.begin(), bindings.end(), [](const auto& row) {
            return row.activeProgram != 3u && row.activeProgram != 4u;
        }), bindings.end());
        const auto key = [](const auto& row) { return row.assetId + "|" + row.materialName; };
        auto selected = std::find_if(bindings.begin(), bindings.end(), [&](const auto& row) { return key(row) == m_strPixelMaterialKey; });
        if (selected == bindings.end() && !bindings.empty())
        {
            selected = bindings.begin();
            m_strPixelMaterialKey = key(*selected);
        }
        if (selected == bindings.end())
            ImGui::TextDisabled("No PBR surface bound in the last second. This panel samples successful draw bindings while open.");
        else
        {
            if (ImGui::BeginCombo("Inspect one bound surface", selected->materialName.c_str()))
            {
                for (const auto& row : bindings)
                {
                    const auto id = key(row);
                    ImGui::PushID(id.c_str());
                    if (ImGui::Selectable(row.materialName.c_str(), id == m_strPixelMaterialKey)) m_strPixelMaterialKey = id;
                    ImGui::PopID();
                }
                ImGui::EndCombo();
                selected = std::find_if(bindings.begin(), bindings.end(), [&](const auto& row) { return key(row) == m_strPixelMaterialKey; });
            }
            if (selected != bindings.end() && table("BoundMaterialPixelInputs"))
            {
                const auto& s = selected->surface;
                const auto& l = selected->lighting;
                const char* bound = "bound source constant";
                number("Source program", selected->activeProgram); vec("Base RGB multiplier", s.diffuseColor, bound);
                number("Base brightness", s.diffuseBrightness, bound); number("Base saturation", s.diffuseSaturation, bound);
                number("Base texture sRGB decode", s.diffuseSRGB); number("UV tile U", s.uvTiling.x); number("UV tile V", s.uvTiling.y);
                number("Normal intensity", s.normalIntensity); number("Detail normal intensity", s.detailNormalIntensity);
                number("Detail normal tiling", s.detailNormalTiling); number("Fixed normal UV", s.uvFixedNormal);
                number("Vertex normal weight", s.vertexAlpha); number("Masked alpha", s.pbrAlphaMasked);
                number("Alpha cutoff", .3333, s.pbrAlphaMasked ? "active source constant" : "inactive opaque material");
                number("Roughness intensity", s.roughnessIntensity); number("Roughness power", s.roughnessPower);
                number("Minimum roughness", s.minimumRoughness); number("Metallic intensity", s.metallicIntensity);
                number("Metallic power", s.metallicPower); number("AO intensity", s.aoIntensity); number("AO power", s.aoPower);
                number("ORM texture sRGB decode", s.ormSRGB); number("Dielectric specular intensity", s.specularPBRIntensity);
                number("Dielectric F0", .08 * std::clamp(s.specularPBRIntensity, 0.f, 1.f), "metallic pixels interpolate toward albedo");
                number("Nonmetallic brightness", s.nonmetallicBrightness); number("Metallic brightness", s.metallicBrightness);
                number("Reflection texture intensity", s.reflectionIntensity); number("Reflection contrast", s.reflectionContrast);
                number("Reflection tiling", s.reflectionTiling); vec("Reflection RGB", s.reflectionColor);
                number("World reflection UV", s.useWorldReflection); number("Reflection offset U", s.reflectionOriginOffset.x);
                number("Reflection offset V", s.reflectionOriginOffset.y); number("Reflection texture sRGB decode", s.reflectionSRGB);
                number("RNM texture binding enabled", s.hasBakedLighting);
                const char* placement = l.averageScale.w != 0.f ? "last bound placement" : "per-instance stream or absent; not sampled here";
                vec("RNM average scale", l.averageScale, placement); vec("RNM directional scale", l.directionalScale, placement);
                number("RNM UV scale U", l.scaleBias.x, placement); number("RNM UV scale V", l.scaleBias.y, placement);
                number("RNM UV offset U", l.scaleBias.z, placement); number("RNM UV offset V", l.scaleBias.w, placement);
                number("RNM sRGB decode", s.bakedLightingSRGB);
                number("Environment cube / BRDF lookup bound", s.hasEnvironmentCube);
                vec("Material environment RGB", s.environmentColor); number("Material environment floor", s.environmentColor.w);
                number("Environment rotation A", s.environmentRotation.x); number("Environment rotation B", s.environmentRotation.y);
                number("RGBM decode range", 6., "source shader constant"); number("Reflection mip scale", 5., "roughness AA * scale");
                number("Roughness derivative AA", .3, "source shader constant");
                number("Static shadow bound", s.hasStaticShadow); number("Static shadow channel", s.staticShadowChannel);
                vec("Static shadow bias / scale / power", s.staticShadowTransfer);
                const char* emissive = s.hasEmissive ? "active" : "disabled";
                number("Emission enabled", s.hasEmissive); vec("Emission RGB", s.emissiveColor, emissive);
                number("Emission intensity", s.emissiveIntensity, emissive);
                number("Emission tile U", s.emissiveUVTiling.x, emissive); number("Emission tile V", s.emissiveUVTiling.y, emissive);
                number("Emission flicker minimum", s.emissiveFlickerMinimum, emissive);
                number("Emission flicker speed", s.emissiveFlickerSpeed, emissive); number("Emission phase", s.emissivePhaseOffset, emissive);
                number("Emission sRGB decode", s.emissiveSRGB, emissive);
                ImGui::EndTable();
                ImGui::TextWrapped("SH/hemisphere owner constants are unresolved. The BRDF lookup is a project numerical approximation, not recovered native LUT bytes. Bound inputs are not GPU pixel readbacks.");
            }
        }
    }
    return changed;
}

void Client::CRenderingBenchmark::Render_Section(
	Engine::CProfiler* pProfiler,
	const string& strQualitySummary,
	CRenderingProfileService& Profiles)
{
	ImGui::SeparatorText("Benchmark");
	const bool_t profileChanged = Render_RestorationSection(Profiles);
	const bool_t pixelsChanged = Render_PixelInputs();
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
	ImGui::BeginDisabled(m_bCapturing || profileChanged || pixelsChanged);
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
            else
            {
                if (!m_bPixelDiagnosticsActive) m_PixelEntrySettings = previous;
                m_bPixelDiagnosticsActive = true;
                m_iPixelDiagnosticsLevel = game.Get_CurrentLevelID();
            }
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
