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
#include "RenderingTechniqueGuide.h"
#include "UserSettingsDocument.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <numeric>
#include <sstream>
#include <cmath>
#include <locale>

namespace
{
    // Normalize only explicitly selected experiment fields. All other renderer
    // input fields below remain in the comparison; source mode is handled by caller.
    string ComparisonConditions(uint64_t excluded = 0u, const Engine::SHADOW_LIGHT_DESC* shadowBasis = nullptr)
    {
        const auto& game = Engine::CGameInstance::Get();
        auto& mutableGame = Engine::CGameInstance::Get();
        ostringstream stream;
        stream.imbue(locale::classic());
        stream << setprecision(9) << scientific;
        const auto scalar = [&](const auto value) { stream << value << ' '; };
        const auto vector = [&](const auto& value) {
            scalar(value.x); scalar(value.y); scalar(value.z); scalar(value.w);
        };
        scalar(game.Get_CurrentLevelID());
        scalar(IsDebuggerPresent()!=FALSE);
        const auto* profiler=mutableGame.Get_Profiler();
        scalar(profiler && profiler->Is_CollectingDetailedScopes());
        scalar(profiler && profiler->Is_DetailedScopesEnabled());
        scalar(Engine::CPresentation_Manager::Get().Are_TransientLightsEnabled());
        scalar(Engine::CPresentation_Manager::Get().Are_ScreenPostsEnabled());
        const auto& user = Client::CUserSettings::Get().Get_Settings();
        scalar(user.Display.width); scalar(user.Display.height); scalar(static_cast<int>(user.Display.mode));
        for (const auto& [key, value] : user.Values) { stream << quoted(key); scalar(value); }
        DWORD foregroundProcess=0; GetWindowThreadProcessId(GetForegroundWindow(),&foregroundProcess);
        scalar(foregroundProcess==GetCurrentProcessId());
        scalar(Client::CUserSettings::Get().Get_FrameLimit(foregroundProcess==GetCurrentProcessId()));
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
        auto material = game.Get_MaterialRenderSettings();
        using F = Client::RENDERING_EXPERIMENT_FIELD;
        const auto omit = [&](F f) { return (excluded & Client::RenderingExperimentBit(f)) != 0u; };
        const auto zero = [&](F f, auto& target) { if (omit(f)) target = {}; };
        const uint64_t pbrMask = ((uint64_t{1} << Client::RENDERING_EXPERIMENT_FIELD_COUNT) - 1u) &
            ~((uint64_t{1} << static_cast<size_t>(F::PBR_DIFFUSE)) - 1u);
        if (excluded & pbrMask)
        {
            if (!material.MapPBR.Is_Active(game.Get_CurrentLevelID())) material.MapPBR = {};
            material.MapPBR.bEnabled = false; material.MapPBR.iLevel = 0;
            zero(F::PBR_DIFFUSE, material.MapPBR.vContributionScale.x); zero(F::PBR_SPECULAR, material.MapPBR.vContributionScale.y);
            zero(F::PBR_BAKED, material.MapPBR.vContributionScale.z); zero(F::PBR_ENVIRONMENT, material.MapPBR.vContributionScale.w);
            zero(F::PBR_CUBE, material.MapPBR.fCubeDiffuseScale); zero(F::NORMAL_STRENGTH, material.MapPBR.vSurfaceParameters.x);
            zero(F::ROUGHNESS_OFFSET, material.MapPBR.vSurfaceParameters.y);
        }
        scalar(static_cast<uint32_t>(material.eDebugView));
        scalar(material.MapPBR.bEnabled); scalar(material.MapPBR.iLevel);
        vector(material.MapPBR.vContributionScale); vector(material.MapPBR.vSurfaceParameters);
        scalar(material.MapPBR.fCubeDiffuseScale);
        auto q = game.Get_RenderQualitySettings();
        zero(F::SSAO_ENABLED,q.bSSAOEnabled); zero(F::SSAO_RADIUS,q.fSSAORadius); zero(F::SSAO_BIAS,q.fSSAOBias);
        zero(F::SSAO_INTENSITY,q.fSSAOIntensity); zero(F::SSAO_POWER,q.fSSAOPower); zero(F::SSAO_FADE,q.fSSAODistanceFade);
        zero(F::BLOOM_ENABLED,q.bBloomEnabled); zero(F::BLOOM_THRESHOLD,q.fBloomThreshold); zero(F::BLOOM_KNEE,q.fBloomSoftKnee);
        zero(F::BLOOM_INTENSITY,q.fBloomIntensity); zero(F::BLOOM_SCATTER,q.fBloomScatter);
        zero(F::FXAA_ENABLED,q.bFXAAEnabled); zero(F::FXAA_BLEND,q.fFXAASubpixel); zero(F::FXAA_EDGE,q.fFXAAEdgeThreshold);
        zero(F::FXAA_EDGE_MIN,q.fFXAAEdgeThresholdMin); zero(F::EXPOSURE,q.fExposure); zero(F::GAMMA,q.fGamma);
        zero(F::DESATURATION,q.fSceneDesaturation);
        zero(F::SSAO_SAMPLES,q.iSSAOSampleCount); scalar(q.iSSAOSampleCount);
        if (omit(F::LUT_ENABLED)) q.SourcePostProcess.LutLayers.clear();
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
        auto fog = game.Get_HeightFogSettings(); zero(F::FOG_ENABLED,fog.bEnabled); zero(F::FOG_DENSITY,fog.fDensity);
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
        scalar(environment.fDiffuseIntensity);
        scalar(environment.bUseSourcePBRIndirect);
        for (const auto& row : environment.vDiffuseSH) vector(row);
        auto shadow = game.Get_ShadowLightDesc();
        if (omit(F::SHADOW_ENABLED) && shadowBasis) shadow=*shadowBasis;
        zero(F::SHADOW_ENABLED,shadow.Settings.bEnabled);
        zero(F::SHADOW_STRENGTH,shadow.Settings.fStrength); vector(shadow.vEye); vector(shadow.vAt);
        zero(F::PCF_RADIUS,shadow.Settings.iPCFFilterRadius); scalar(shadow.Settings.iPCFFilterRadius);
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
			default:
                if (static_cast<unsigned char>(Character) < 0x20u)
                {
                    static constexpr char hex[] = "0123456789abcdef";
                    Escaped += "\\u00"; Escaped += hex[(Character >> 4) & 15]; Escaped += hex[Character & 15];
                }
                else Escaped += Character;
                break;
			}
		}
		return Escaped;
	}
}

string Client::CRenderingBenchmark::Current_Conditions(uint64_t excludedFields) const
{
    const auto* shadowBasis=m_pExperimentProfiles?&m_pExperimentProfiles->Get_ExperimentShadowBasis():nullptr;
    string result = ComparisonConditions(excludedFields,shadowBasis);
    if (shadowBasis)
    {
        // Keep the underlying shadow owner in every fingerprint even while OFF
        // normalizes the visible Engine descriptor to defaults.
        ostringstream input; input.imbue(locale::classic()); input<<setprecision(9)<<scientific;
        const auto& s=shadowBasis->Settings;
        for (auto v:{shadowBasis->vEye.x,shadowBasis->vEye.y,shadowBasis->vEye.z,shadowBasis->vEye.w,
            shadowBasis->vAt.x,shadowBasis->vAt.y,shadowBasis->vAt.z,shadowBasis->vAt.w,
            s.fOrthographicWidth,s.fOrthographicHeight,s.fNear,s.fFar,s.fDepthBias,s.fNormalBias,
            s.fStrength,s.fDynamicBakedStrength}) input<<v<<' ';
        input<<s.bEnabled<<' '<<s.iPCFFilterRadius;result+=" shadowBasis="+input.str();
    }
    if (m_bCaptureExperiment || m_bExperimentActive)
    {
        result += Engine::CGameInstance::Get().Get_MaterialRenderSettings().bUseSourceMaterials ? " source=1" : " source=0";
        if (m_pExperimentProfiles)
            result += " owner=" + m_pExperimentProfiles->Get_ActiveProfileId() + "/" +
                m_pExperimentProfiles->Get_LevelQualityProfileId() + "/" + m_pExperimentProfiles->Get_AppliedEnvironmentRegionId() +
                "/" + std::to_string(m_pExperimentProfiles->Get_ProfileGeneration());
    }
    return result;
}

bool_t Client::CRenderingBenchmark::Begin(Engine::CProfiler* profiler, const string& label,
    uint32_t frames, const string& qualitySummary, string& status)
{
    if (!profiler || m_bCapturing) { status = "Profiler unavailable or a capture is already running."; return false; }
    // Preserve other Profiler history. A run occupies a bounded window plus a
    // drain tail, and each repeat is finalized before starting the next.
    if (frames < 10u || frames > 900u || m_iWarmupInput > 120u)
    { status = "Use 10..900 sample frames and 0..120 warm-up frames."; return false; }
    if (m_bExperimentActive && !Apply_ExperimentVariant(m_bVariantB))
    { status = m_strStatus; return false; }
    m_bProfilerWasEnabled = m_bSequence ? m_bSequenceProfilerWasEnabled : profiler->Is_Enabled();
    Engine::FProfilerLiveStats live{}; profiler->Get_LiveStats(live);
    m_iCaptureWarmup = m_iWarmupInput;
    m_iStartFrame = live.FrameNumber + 1u + m_iCaptureWarmup;
    m_iLastObservedFrame = live.FrameNumber;
    profiler->Set_Enabled(true);
    m_iTargetFrames = frames; m_strLabel = label.empty() ? "run" : label;
    m_strQualitySummary = qualitySummary;
    m_bCaptureExperiment = m_bExperimentActive;
    m_iCaptureFields = m_bCaptureExperiment ? Experiment_FieldMask() : 0u;
    m_strCaptureExperimentId = m_bCaptureExperiment ? m_strExperimentId : "material.source";
    m_strCaptureVariant = m_bCaptureExperiment ? (m_bVariantB ? "B" : "A") :
        (Engine::CGameInstance::Get().Get_MaterialRenderSettings().bUseSourceMaterials ? "B" : "A");
    if (m_bSweep && m_iSequenceStep) m_strCaptureVariant = "sweep=" + std::to_string(m_SweepPoints[m_iSequenceStep-1u]);
    m_strComparisonConditions.clear(); m_strFullConditions.clear(); m_strFailureReason.clear();
    m_bSourceMaterials = Engine::CGameInstance::Get().Get_MaterialRenderSettings().bUseSourceMaterials;
    m_bConditionsStable = true; m_bWarmup = true; m_bCapturing = true; m_pCaptureProfiler = profiler;
    status = "Warm-up, then sampling complete frames. Camera and scene must remain unchanged.";
    return true;
}

void Client::CRenderingBenchmark::Cancel_Capture(const string& reason)
{
    if (m_bCapturing && m_pCaptureProfiler && m_pCaptureProfiler->Is_Enabled() && !m_bProfilerWasEnabled)
        m_pCaptureProfiler->Set_Enabled(false);
    m_bCapturing = false; m_bWarmup = false;
    Finish_Sequence();
    m_strFailureReason = reason; m_strStatus = reason;
}

void Client::CRenderingBenchmark::Update(Engine::CProfiler* profiler)
{
    Poll_Save();
    if (!m_bCapturing || !profiler) return;
    if (!profiler->Is_Enabled()) { Cancel_Capture("Capture stopped by the Profiler; completed runs are preserved."); return; }
    Engine::FProfilerLiveStats live{};
    if (!profiler->Get_LiveStats(live)) return;
    if (live.FrameNumber < m_iLastObservedFrame)
    { Cancel_Capture("Profiler history was reset; restart this run."); return; }
    m_iLastObservedFrame = live.FrameNumber;
    if (m_bCaptureExperiment && (!m_bExperimentActive || !m_pExperimentProfiles || !m_pExperimentProfiles->Has_ExperimentPreview()))
    { Cancel_Capture("Experiment owner changed; this run was cancelled."); return; }
    if (m_bWarmup)
    {
        if (live.FrameNumber < m_iStartFrame) return;
        m_bWarmup = false;
        m_strComparisonConditions = Current_Conditions(m_iCaptureFields);
        m_strFullConditions = Current_Conditions(0u);
        m_CaptureValues = CRenderingProfileService::Read_ExperimentValues();
        m_strStatus = "Sampling (no GPU waits).";
    }
    if (live.FrameNumber <= m_iStartFrame + m_iTargetFrames && m_strFullConditions != Current_Conditions(0u))
    {
        m_bConditionsStable = false;
        m_strFailureReason = "Camera, viewport, renderer inputs or user settings changed during sampling.";
    }
    const uint64_t lastSample = m_iStartFrame + m_iTargetFrames;
    if (live.FrameNumber < lastSample + Engine::CProfiler::GPU_READ_LATENCY) return;
    const auto snapshot = profiler->Snapshot();
    if (snapshot.Frames.empty() || snapshot.Frames.front().FrameNumber > m_iStartFrame + 1u)
    { Cancel_Capture("Sample history was reset or evicted; completed runs are preserved."); return; }
    bool pending = false;
    for (const auto& frame : snapshot.Frames)
        if (frame.FrameNumber > m_iStartFrame && frame.FrameNumber <= lastSample &&
            frame.GpuStatus == Engine::EProfilerGpuFrameStatus::Pending) pending = true;
    // Polling is done by Engine. A bounded tail prevents a lost query from
    // keeping the session running forever; unresolved samples remain pending.
    if (pending && live.FrameNumber < lastSample + 64u) { m_strStatus = "Resolving pending GPU samples."; return; }
    const bool sequence = m_bSequence;
    if (!Finalize(*profiler)) { Finish_Sequence(); return; }
    if (sequence && m_bConditionsStable && m_bExperimentActive && ++m_iSequenceStep < m_iSequenceTotal)
    {
        // AB, BA alternation balances cache/order effects across repeats.
        const bool nextB = (m_iSequenceStep % 4u == 1u || m_iSequenceStep % 4u == 2u);
        if (m_bSweep)
        {
            m_ExperimentB = m_ExperimentA;
            m_ExperimentB.values[m_iSweepField] = m_SweepPoints[m_iSequenceStep-1u];
        }
        if (!Apply_ExperimentVariant(m_bSweep || nextB) ||
            !Begin(profiler, m_LabelBuffer.data(), static_cast<uint32_t>(m_iFrameInput), "session experiment", m_strStatus))
            Finish_Sequence();
    }
    else Finish_Sequence();
}

bool_t Client::CRenderingBenchmark::Finalize(Engine::CProfiler& profiler)
{
    m_bCapturing = false;
    const auto snapshot = profiler.Snapshot();
    if (!m_bProfilerWasEnabled && !m_bSequence) profiler.Set_Enabled(false);
    RENDERING_BENCHMARK_RUN run;
    run.strLabel = m_strLabel; run.strTimestamp = Now_Timestamp(); run.strQualitySummary = m_strQualitySummary;
    run.strComparisonConditions = m_strComparisonConditions; run.strFullConditions = m_strFullConditions;
    run.strExperimentId = m_strCaptureExperimentId; run.strVariant = m_strCaptureVariant;
    run.fieldMask = m_iCaptureFields; run.warmupFrames = m_iCaptureWarmup; run.repetition = m_bSweep ? m_iSequenceStep+1u : m_iSequenceStep / 2u + 1u;
    run.appliedValues = m_CaptureValues; run.bSourceMaterials = m_bSourceMaterials;
    run.bConditionsStable = m_bConditionsStable; run.strFailureReason = m_strFailureReason;
    vector<double> cpu, gpu, interval;
    std::map<string, RENDERING_BENCHMARK_PASS> passes, cpuPasses;
    uint32_t completeGpuScopes = 0;
    const auto count = [](const auto& frame, Engine::EProfilerCounter c) { return static_cast<double>(frame.Counters[static_cast<size_t>(c)]); };
    for (const auto& frame : snapshot.Frames)
    {
        if (frame.FrameNumber <= m_iStartFrame || frame.FrameNumber > m_iStartFrame + m_iTargetFrames) continue;
        if (!run.firstFrame) run.firstFrame = frame.FrameNumber;
        run.lastFrame = frame.FrameNumber; ++run.iFrames; cpu.push_back(frame.CpuFrameMs);
        // Interval belongs to the preceding frame; exclude the transition edge.
        if (frame.FrameNumber > m_iStartFrame + 1u && frame.FrameIntervalMs > 0) interval.push_back(frame.FrameIntervalMs);
        run.fDrawCallsAvg += count(frame,Engine::EProfilerCounter::DrawCalls);
        run.fInstancesAvg += count(frame,Engine::EProfilerCounter::Instances);
        run.fIndicesAvg += count(frame,Engine::EProfilerCounter::Indices);
        run.fMeshDrawsAvg += count(frame,Engine::EProfilerCounter::MeshDrawCalls);
        run.fUniqueMeshesAvg += count(frame,Engine::EProfilerCounter::UniqueMeshes);
        run.droppedCpuScopes += frame.DroppedCpuScopes; run.droppedGpuScopes += frame.DroppedGpuScopes;
        if (frame.DetailedCpuScopes && !frame.DroppedCpuScopes && snapshot.TicksPerSecond)
        {
            ++run.iCpuScopeFrames;
            for (const auto& sample : frame.CpuScopes)
            {
                if (sample.NameId >= snapshot.ScopeNames.size() || sample.EndTick < sample.BeginTick) continue;
                const string key = snapshot.ScopeNames[sample.NameId] + " [thread " + std::to_string(sample.ThreadId) + "]";
                auto& pass = cpuPasses[key]; pass.name = key;
                pass.inclusiveMs += static_cast<double>(sample.EndTick-sample.BeginTick)*1000.0/snapshot.TicksPerSecond;
            }
        }
        if (frame.GpuStatus == Engine::EProfilerGpuFrameStatus::Pending) ++run.pendingGpuFrames;
        else if (!frame.GpuValid) ++run.invalidGpuFrames;
        if (!frame.GpuValid) continue;
        gpu.push_back(frame.GpuFrameMs); run.fPsInvocationsAvg += static_cast<double>(frame.Pipeline.PSInvocations);
        if (!frame.GpuScopesSupported || frame.DroppedGpuScopes) continue;
        ++completeGpuScopes;
        for (const auto& sample : frame.GpuScopes)
        {
            if (sample.NameId >= snapshot.ScopeNames.size()) continue;
            auto& pass = passes[snapshot.ScopeNames[sample.NameId]];
            pass.name = snapshot.ScopeNames[sample.NameId]; pass.inclusiveMs += sample.DurationMs;
            pass.selfMs += sample.SelfMs; pass.drawCalls += static_cast<double>(sample.Draw.DrawCalls);
            pass.indices += static_cast<double>(sample.Draw.Indices);
        }
    }
    if (run.iFrames != m_iTargetFrames || cpu.empty())
    { m_strStatus = "Incomplete CPU sample window; run was not saved."; return false; }
    const auto average = [](const vector<double>& v) { return v.empty() ? 0.0 : std::accumulate(v.begin(),v.end(),0.0)/v.size(); };
    run.fCpuAvgMs=average(cpu); run.fCpuP50Ms=Percentile(cpu,.5); run.fCpuP95Ms=Percentile(cpu,.95);
    run.fCpuP99Ms=Percentile(cpu,.99); run.fCpuMaxMs=*std::max_element(cpu.begin(),cpu.end());
    run.iGpuFrames=static_cast<uint32_t>(gpu.size()); run.fGpuAvgMs=average(gpu);
    run.fGpuP50Ms=Percentile(gpu,.5); run.fGpuP95Ms=Percentile(gpu,.95); run.fGpuP99Ms=Percentile(gpu,.99);
    if (!gpu.empty()) { run.fGpuMaxMs=*std::max_element(gpu.begin(),gpu.end()); run.fPsInvocationsAvg/=gpu.size(); }
    run.iIntervalFrames=static_cast<uint32_t>(interval.size()); run.fIntervalAvgMs=average(interval);
    run.fIntervalP50Ms=Percentile(interval,.5); run.fIntervalP95Ms=Percentile(interval,.95); run.fIntervalP99Ms=Percentile(interval,.99);
    if (!interval.empty()) run.fIntervalMaxMs=*std::max_element(interval.begin(),interval.end());
    const double n=run.iFrames;
    run.fDrawCallsAvg/=n; run.fInstancesAvg/=n; run.fIndicesAvg/=n; run.fMeshDrawsAvg/=n; run.fUniqueMeshesAvg/=n;
    run.iGpuScopeFrames = completeGpuScopes;
    for (auto& [name,pass] : cpuPasses)
    { pass.validFrames = run.iCpuScopeFrames; pass.inclusiveMs /= run.iCpuScopeFrames; run.cpuPasses.push_back(pass); }
    for (auto& [name,pass] : passes)
    {
        pass.validFrames=completeGpuScopes; pass.inclusiveMs/=completeGpuScopes; pass.selfMs/=completeGpuScopes;
        pass.drawCalls/=completeGpuScopes; pass.indices/=completeGpuScopes; run.passes.push_back(pass);
    }
    if (m_Runs.size() == 64u) { m_Runs.erase(m_Runs.begin()); m_iCompareFirst=m_iCompareSecond=-1; }
    m_Runs.push_back(std::move(run)); m_iCompareSecond=static_cast<int>(m_Runs.size()-1u);
    if (m_bSweep)
    {
        for (size_t i=0;i+1<m_Runs.size();++i)
            if (m_Runs[i].strVariant=="A" && m_Runs[i].strExperimentId==m_strCaptureExperimentId &&
                m_Runs[i].fieldMask==m_iCaptureFields && m_Runs[i].strComparisonConditions==m_strComparisonConditions)
            {m_iCompareFirst=static_cast<int>(i);break;}
    }
    m_strStatus=m_bConditionsStable ? "Run recorded. JSON save queued." : "Run recorded as INVALID: "+m_strFailureReason;
    if (m_bSequence) m_bSaveQueued=true;
    else Queue_Save();
    return true;
}

void Client::CRenderingBenchmark::Queue_Save()
{
    if (m_SaveFuture.valid()) { m_bSaveQueued=true; return; }
    auto runs=m_Runs; const auto path=Make_DefaultPath(); m_bSaveQueued=false;
    try
    {
        m_SaveFuture=std::async(std::launch::async,[runs=std::move(runs),path]() {
            SAVE_RESULT result; string error;
            result.ok=Save_Json(runs,path,error); result.message=result.ok ? "JSON 저장: "+path.string() : "JSON 저장 실패: "+error;
            return result;
        });
        m_strSaveStatus="JSON 저장 중 (측정 thread와 분리)";
    }
    catch (const std::exception& error) { m_strSaveStatus=string("JSON 저장 시작 실패: ")+error.what(); }
}

void Client::CRenderingBenchmark::Poll_Save()
{
    if (!m_SaveFuture.valid())
    { if (m_bSaveQueued && !m_bCapturing && !m_bSequence) Queue_Save(); return; }
    if (m_SaveFuture.wait_for(std::chrono::seconds(0)) != std::future_status::ready) return;
    try { m_strSaveStatus=m_SaveFuture.get().message; }
    catch (const std::exception& error) { m_strSaveStatus=string("JSON 저장 실패: ")+error.what(); }
    if (m_bSaveQueued && !m_bCapturing && !m_bSequence) Queue_Save();
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
    if (m_bExperimentActive) End_Experiment();
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
    if (m_bExperimentActive && (!bToolVisible || !Profiles.Has_ExperimentPreview() ||
        Profiles.Get_ProfileGeneration() != m_iExperimentProfileGeneration ||
        Engine::CGameInstance::Get().Get_CurrentLevelID() != m_iExperimentLevel))
        End_Experiment();
    if (!bToolVisible && m_bCapturing) Cancel_Capture("Workbench closed; capture cancelled, completed runs preserved.");
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
		const bool beforeActive = Profiles.Get_ActiveProfileId() == beforeId;
		const bool restoredActive = Profiles.Get_ActiveProfileId() == restoredId;
		const bool canReturnFromBefore = beforeActive && !m_strRestorationEntryProfileId.empty();
		ImGui::BeginDisabled(m_bCapturing || (canReturnFromBefore ? false : !beforeAvailable || beforeActive));
		if (ImGui::Button(canReturnFromBefore ? "Return from before-restoration.v1" : "before-restoration.v1"))
			changed = (canReturnFromBefore ? Return_ToEntryProfile(Profiles) :
				Activate_RestorationProfile(Profiles, beforeId)) || changed;
		ImGui::EndDisabled();
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			ImGui::SetTooltip("%s", canReturnFromBefore ? m_strRestorationEntryProfileId.c_str() : beforeId);
		ImGui::SameLine();
		ImGui::BeginDisabled(m_bCapturing || !restoredAvailable || restoredActive);
		if (ImGui::Button(restoredActive ? "Restored source profile (active)" : "Restored source profile"))
			changed = Activate_RestorationProfile(Profiles, restoredId) || changed;
		ImGui::EndDisabled();
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			ImGui::SetTooltip("%s", restoredId);
		ImGui::TextWrapped("Baseline%s: %s", beforeActive ? " (active)" : "", beforeId);
		ImGui::TextWrapped("Restored profile: %s", restoredId);
		ImGui::TextWrapped("Switches saved scene lighting, environment, shadow, fog and post-process settings. Material equations and render passes keep the current code; map-light placements and Effect data keep their current assets.");
		if (Profiles.Get_ComparisonOptions().bActive)
			ImGui::TextWrapped("Live rendering comparison is still active. Use Reset comparison below to view this profile without those overrides.");
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
        "Whole scene: grading before FXAA", "PBR cube diffuse sky (project approximation)"
    };
    static_assert(std::size(views) == static_cast<size_t>(Engine::MATERIAL_DEBUG_VIEW::END));
    ImGui::BeginDisabled(m_bCapturing);
    int view = static_cast<int>(settings.eDebugView);
    bool changed = ImGui::Combo("Pipeline view", &view, views, static_cast<int>(std::size(views)));
    settings.eDebugView = static_cast<Engine::MATERIAL_DEBUG_VIEW>(view);
    changed |= ImGui::Checkbox("Recovered material equations", &settings.bUseSourceMaterials);
    if (view >= 11 && view <= 13)
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
        changed |= ImGui::SliderFloat("Cube diffuse sky contribution", &settings.MapPBR.fCubeDiffuseScale, 0.f, 4.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
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
        const auto sky = game.Get_RenderEnvironment();
        ImGui::Text("Cube diffuse sky profile intensity: %.3f", sky.fDiffuseIntensity);
        ImGui::TextWrapped("Cube diffuse uses the scene RGBM cube projection, once before fog, with material and screen AO. It is a project approximation; native SH packing and hemisphere ownership remain unresolved. Set its contribution to zero to compare the previous lighting.");
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
                number("Native PBR indirect inputs available", s.hasSourceIndirect);
                number("Native PBR indirect profile enabled", CGameInstance::Get().Get_RenderEnvironment().bUseSourcePBRIndirect);
                if (s.hasSourceIndirect)
                {
                    vec("Native environment color", s.sourceIndirectColor);
                    number("Native rotation sine", s.sourceIndirectRotation.x);
                    number("Native rotation cosine", s.sourceIndirectRotation.y);
                    vec("Native ambient / sky factor", s.sourceAmbientAndSkyFactor);
                    number("Native upper sky R", s.sourceUpperSkyColor.x); number("Native upper sky G", s.sourceUpperSkyColor.y);
                    number("Native upper sky B", s.sourceUpperSkyColor.z); number("Native lower sky R", s.sourceLowerSkyColor.x);
                    number("Native lower sky G", s.sourceLowerSkyColor.y); number("Native lower sky B", s.sourceLowerSkyColor.z);
                }
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

uint64_t Client::CRenderingBenchmark::Experiment_FieldMask() const
{
    if (m_bSweep) return uint64_t{1} << m_iSweepField;
    uint64_t mask=0;
    for (size_t i=0;i<RENDERING_EXPERIMENT_FIELD_COUNT;++i)
        if (m_ExperimentA.values[i] != m_ExperimentB.values[i]) mask |= uint64_t{1}<<i;
    return mask;
}

bool_t Client::CRenderingBenchmark::Start_Experiment(CRenderingProfileService& profiles)
{
    if (m_bCapturing || m_bExperimentActive) return false;
    m_ExperimentA=m_ExperimentB=CRenderingProfileService::Read_ExperimentValues();
    if (!profiles.Set_ExperimentPreview(m_ExperimentA,0u,m_strStatus)) return false;
    m_pExperimentProfiles=&profiles; m_bExperimentActive=true; m_bVariantB=false;
    m_iExperimentProfileGeneration=profiles.Get_ProfileGeneration();
    m_iExperimentLevel=Engine::CGameInstance::Get().Get_CurrentLevelID();
    m_strExperimentOwner=profiles.Get_ActiveProfileId()+" / "+profiles.Get_AppliedEnvironmentRegionId();
    static uint64_t sequence=0;
    m_strExperimentId="experiment."+std::to_string(GetCurrentProcessId())+"."+std::to_string(GetTickCount64())+"."+std::to_string(++sequence);
    return true;
}

bool_t Client::CRenderingBenchmark::Apply_ExperimentVariant(bool_t variantB)
{
    if (!m_bExperimentActive || !m_pExperimentProfiles) return false;
    if (!m_pExperimentProfiles->Set_ExperimentPreview(variantB ? m_ExperimentB : m_ExperimentA,
        Experiment_FieldMask(),m_strStatus)) return false;
    m_bVariantB=variantB;
    return true;
}

void Client::CRenderingBenchmark::Finish_Sequence()
{
    if (m_pCaptureProfiler && !m_bProfilerWasEnabled && m_pCaptureProfiler->Is_Enabled())
        m_pCaptureProfiler->Set_Enabled(false);
    m_pCaptureProfiler=nullptr; // A completed run no longer owns a later F7 capture.
    m_bSequence = false;
    if (m_bSweep)
    {
        m_bSweep = false; m_ExperimentB = m_PreSweepB;
        const string previousStatus = m_strStatus;
        if (m_bExperimentActive && m_pExperimentProfiles && m_pExperimentProfiles->Has_ExperimentPreview() &&
            Apply_ExperimentVariant(m_bSweepRestoreVariantB)) m_strStatus = previousStatus;
    }
}

bool_t Client::CRenderingBenchmark::Start_Sweep(Engine::CProfiler* profiler)
{
    if (!profiler || !m_bExperimentActive || m_bCapturing) return false;
    const auto& info=CRenderingProfileService::Experiment_Fields()[m_iSweepField];
    vector<double> points;
    if (info.boolean) points={0,1};
    else if (m_iSweepField==static_cast<int>(RENDERING_EXPERIMENT_FIELD::SSAO_SAMPLES)) points={4,8,12};
    else if (m_iSweepField==static_cast<int>(RENDERING_EXPERIMENT_FIELD::PCF_RADIUS)) points={0,1,2};
    else
    {
        if (!std::isfinite(m_fSweepMinimum) || !std::isfinite(m_fSweepMaximum) ||
            m_fSweepMinimum>=m_fSweepMaximum || m_iSweepSteps<2 || m_iSweepSteps>9)
        { m_strStatus="Sweep requires finite minimum < maximum and 2..9 steps."; return false; }
        for (int i=0;i<m_iSweepSteps;++i)
            points.push_back(static_cast<float>(m_fSweepMinimum+(m_fSweepMaximum-m_fSweepMinimum)*i/(m_iSweepSteps-1)));
    }
    for (double point:points)
    {
        auto candidate=m_ExperimentA; candidate.values[m_iSweepField]=point;
        if (!CRenderingProfileService::Validate_ExperimentValues(candidate,m_strStatus)) return false;
    }
    m_SweepPoints=std::move(points); m_PreSweepB=m_ExperimentB; m_bSweepRestoreVariantB=m_bVariantB;
    m_bSweep=true; m_bSequence=true; m_iSequenceStep=0; m_iSequenceTotal=1u+static_cast<uint32_t>(m_SweepPoints.size());
    m_bSequenceProfilerWasEnabled=profiler->Is_Enabled(); m_bProfilerWasEnabled=m_bSequenceProfilerWasEnabled;
    m_pCaptureProfiler=profiler;
    if (!Apply_ExperimentVariant(false) || !Begin(profiler,m_LabelBuffer.data(),static_cast<uint32_t>(m_iFrameInput),"single-field sweep",m_strStatus))
    { Finish_Sequence(); return false; }
    return true;
}

void Client::CRenderingBenchmark::End_Experiment()
{
    Cancel_Capture("Session experiment ended. Completed runs remain available.");
    if (m_pExperimentProfiles && !m_pExperimentProfiles->Clear_ExperimentPreview(m_strStatus)) return;
    m_bExperimentActive=false; m_pExperimentProfiles=nullptr;
}

void Client::CRenderingBenchmark::Render_ExperimentSection(Engine::CProfiler* profiler, CRenderingProfileService& profiles)
{
    ImGui::SeparatorText("렌더링 실험 / 세션 A-B");
    ImGui::TextWrapped("A는 현재 화면의 실효 설정입니다. B만 임시 변경하며 저장 프로필·지역·사용자 Video 파일은 보존합니다. 창 닫기·레벨/지역/프로필 변경은 실험을 종료합니다.");
    if (!m_bExperimentActive)
    {
        ImGui::BeginDisabled(m_bCapturing);
        if (ImGui::Button("현재 품질을 A로 보관하고 실험 시작")) Start_Experiment(profiles);
        ImGui::EndDisabled();
    }
    else
    {
        ImGui::Text("%s | %s 적용",m_strExperimentId.c_str(),m_bVariantB?"B":"A");
        ImGui::TextWrapped("장면: %s",m_strExperimentOwner.c_str());
        if (ImGui::Button("실험 종료 / 원래 장면 복원")) End_Experiment();
        if (!m_bExperimentActive) return;
        ImGui::BeginDisabled(m_bCapturing);
        if (ImGui::Button("A 적용")) Apply_ExperimentVariant(false);
        ImGui::SameLine(); if (ImGui::Button("B 적용")) Apply_ExperimentVariant(true);
        ImGui::SameLine(); if (ImGui::Button("B를 A에서 다시 복사")) { m_ExperimentB=m_ExperimentA; Apply_ExperimentVariant(true); }
        if (ImGui::Button("B: 본질 기준 (다변수 진단)"))
        {
            m_ExperimentB=m_ExperimentA;
            for (const auto field : {RENDERING_EXPERIMENT_FIELD::SSAO_ENABLED,RENDERING_EXPERIMENT_FIELD::BLOOM_ENABLED,
                RENDERING_EXPERIMENT_FIELD::FXAA_ENABLED,RENDERING_EXPERIMENT_FIELD::FOG_ENABLED,
                RENDERING_EXPERIMENT_FIELD::LUT_ENABLED,RENDERING_EXPERIMENT_FIELD::DESATURATION})
                m_ExperimentB.values[static_cast<size_t>(field)]=0;
            Apply_ExperimentVariant(true);
        }
        ImGui::TextWrapped("본질 기준은 SSAO·Bloom·FXAA·안개·LUT·탈색만 끕니다. 재질·직접광·RNM·환경광·그림자·노출·감마는 A와 같습니다. GI 없는 화면이 아닙니다. 기여량 0도 계산 생략을 뜻하지 않습니다.");
        static const char* labels[]={"SSAO 켜기","SSAO 반경 (m)","SSAO bias","SSAO 강도","SSAO power","SSAO 거리 감쇠 (m)",
            "Bloom 켜기","Bloom 임계값","Bloom soft knee","Bloom 강도","Bloom scatter","FXAA 켜기","FXAA blend",
            "FXAA edge threshold","FXAA 최소 threshold","노출","표시 gamma","방향광 그림자 켜기","그림자 강도",
            "안개 켜기","안개 밀도","LUT 켜기","장면 탈색","SSAO 샘플 수","PCF 반경 (0/1/2 = 1/9/25 tap)","PBR 직접 diffuse","PBR 직접 specular","PBR baked RNM",
            "PBR 환경 specular","PBR cube diffuse","PBR normal 강도","PBR roughness offset"};
        static_assert(std::size(labels)==RENDERING_EXPERIMENT_FIELD_COUNT);
        size_t changedCount=0; const uint64_t mask=Experiment_FieldMask();
        for (size_t i=0;i<RENDERING_EXPERIMENT_FIELD_COUNT;++i) if (mask&(uint64_t{1}<<i)) ++changedCount;
        ImGui::Text("변경 필드 %zu개: %s",changedCount,changedCount==1?"단일 변수 비교":(changedCount?"다변수 진단 (개별 원인 비용 아님)":"동일 기준"));
        if (ImGui::CollapsingHeader("B 수치 조절 / 실제 지원 필드",ImGuiTreeNodeFlags_DefaultOpen))
        {
            const auto& fields=CRenderingProfileService::Experiment_Fields();
            if (ImGui::BeginTable("ExperimentFields",3,ImGuiTableFlags_RowBg|ImGuiTableFlags_BordersInnerH|ImGuiTableFlags_Resizable))
            {
                ImGui::TableSetupColumn("항목"); ImGui::TableSetupColumn("A (보관)"); ImGui::TableSetupColumn("B (임시 조절)"); ImGui::TableHeadersRow();
                bool changed=false;
                for (size_t i=0;i<fields.size();++i)
                {
                    ImGui::PushID(static_cast<int>(i)); ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted(labels[i]); if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s",fields[i].id);
                    ImGui::TableSetColumnIndex(1); ImGui::Text("%.5g",m_ExperimentA.values[i]);
                    ImGui::TableSetColumnIndex(2); ImGui::SetNextItemWidth(-1.f);
                    if (i==static_cast<size_t>(RENDERING_EXPERIMENT_FIELD::SSAO_SAMPLES))
                    { int value=static_cast<int>(m_ExperimentB.values[i]/4.0)-1; if (ImGui::Combo("##value",&value,"4\0" "8\0" "12\0")) {m_ExperimentB.values[i]=(value+1)*4;changed=true;} }
                    else if (i==static_cast<size_t>(RENDERING_EXPERIMENT_FIELD::PCF_RADIUS))
                    { int value=static_cast<int>(m_ExperimentB.values[i]); if (ImGui::SliderInt("##value",&value,0,2)) {m_ExperimentB.values[i]=value;changed=true;} }
                    else if (fields[i].boolean)
                    { bool value=m_ExperimentB.values[i]!=0; if (ImGui::Checkbox("##value",&value)) {m_ExperimentB.values[i]=value?1:0;changed=true;} }
                    else
                    { float value=static_cast<float>(m_ExperimentB.values[i]); if (ImGui::DragFloat("##value",&value,static_cast<float>(fields[i].step),static_cast<float>(fields[i].minimum),static_cast<float>(fields[i].maximum),"%.5g",ImGuiSliderFlags_AlwaysClamp)) {m_ExperimentB.values[i]=value;changed=true;} }
                    ImGui::PopID();
                }
                ImGui::EndTable(); if (changed) Apply_ExperimentVariant(true);
            }
        }
        ImGui::TextWrapped("PBR 기여값은 지원되는 map PBR 재질에만 적용됩니다. Source character의 별도 계산은 그대로입니다. Video OFF를 ON으로 조절하는 경우도 현재 실험에만 적용되며 사용자 파일을 변경하지 않습니다.");
        ImGui::EndDisabled();
    }
    ImGui::BeginDisabled(m_bCapturing);
    ImGui::InputText("실험 이름",m_LabelBuffer.data(),m_LabelBuffer.size());
    ImGui::SliderInt("측정 프레임",&m_iFrameInput,10,900);
    int warm=static_cast<int>(m_iWarmupInput), repeat=static_cast<int>(m_iRepeatInput);
    if (ImGui::SliderInt("준비 프레임",&warm,0,120)) m_iWarmupInput=static_cast<uint32_t>(warm);
    if (ImGui::SliderInt("A/B 반복",&repeat,1,8)) m_iRepeatInput=static_cast<uint32_t>(repeat);
    ImGui::SliderFloat("목표 FPS 예산",&m_fTargetFps,15,240,"%.0f");
    ImGui::Text("목표 %.0f FPS = %.3f ms / 프레임",m_fTargetFps,1000.0/m_fTargetFps);
    if (m_bExperimentActive)
    {
        if (ImGui::Button("현재 A/B 측정")) { m_iSequenceStep=0; Begin(profiler,m_LabelBuffer.data(),static_cast<uint32_t>(m_iFrameInput),"session experiment",m_strStatus); }
        ImGui::SameLine();
        if (ImGui::Button("A/B 반복 측정 (AB / BA)") && profiler)
        {
            m_bSequenceProfilerWasEnabled=profiler->Is_Enabled();
            m_iSequenceStep=0; m_iSequenceTotal=2u*m_iRepeatInput;
            if (Apply_ExperimentVariant(false))
            { m_bSequence=true; if (!Begin(profiler,m_LabelBuffer.data(),static_cast<uint32_t>(m_iFrameInput),"session experiment",m_strStatus)) m_bSequence=false; }
        }
    }
    if (m_bExperimentActive && ImGui::CollapsingHeader("단일 변수 자동 sweep (A + 최대 9개 값)"))
    {
        const auto& fields=CRenderingProfileService::Experiment_Fields();
        if (ImGui::BeginCombo("변수",fields[m_iSweepField].id))
        {
            for (size_t i=0;i<fields.size();++i)
                if (ImGui::Selectable(fields[i].id,m_iSweepField==static_cast<int>(i)))
                { m_iSweepField=static_cast<int>(i); m_fSweepMinimum=static_cast<float>(fields[i].minimum); m_fSweepMaximum=static_cast<float>(fields[i].maximum); }
            ImGui::EndCombo();
        }
        const auto& f=fields[m_iSweepField];
        if (!f.boolean && m_iSweepField!=static_cast<int>(RENDERING_EXPERIMENT_FIELD::SSAO_SAMPLES) && m_iSweepField!=static_cast<int>(RENDERING_EXPERIMENT_FIELD::PCF_RADIUS))
        {
            ImGui::DragFloat("최솟값",&m_fSweepMinimum,static_cast<float>(f.step),static_cast<float>(f.minimum),static_cast<float>(f.maximum),"%.5g",ImGuiSliderFlags_AlwaysClamp);
            ImGui::DragFloat("최댓값",&m_fSweepMaximum,static_cast<float>(f.step),static_cast<float>(f.minimum),static_cast<float>(f.maximum),"%.5g",ImGuiSliderFlags_AlwaysClamp);
            ImGui::SliderInt("단계 수",&m_iSweepSteps,2,9);
        }
        ImGui::TextWrapped("선택 필드 외에는 A를 유지합니다. bool은 0/1, SSAO는 4/8/12, PCF는 0/1/2만 측정합니다. 각 값 준비→수집→GPU 회수 후 다음 값으로 이동하며 종료·취소 시 시작 전 A/B로 복원합니다.");
        if (ImGui::Button("A 기준 + sweep 시작")) Start_Sweep(profiler);
    }
    ImGui::EndDisabled();
    if (m_bCapturing)
    {
        ImGui::Text("%s | %s | 반복 %u / %u",m_bWarmup?"준비 중":"수집 / GPU 회수",m_strCaptureVariant.c_str(),m_iSequenceStep/2u+1u,m_bSequence?m_iRepeatInput:1u);
        if (ImGui::Button("측정 취소 (설정은 유지)")) Cancel_Capture("Capture cancelled; completed runs preserved.");
    }
    ImGui::TextWrapped("%s",m_strStatus.c_str());
    if (!m_strSaveStatus.empty()) ImGui::TextWrapped("%s",m_strSaveStatus.c_str());
    ImGui::TextWrapped("카메라·해상도·장면·Video·도구 창 표시·Profiler 상세 계측을 고정하세요. 동적 animation/Server gameplay를 고정 재생하지 않으므로 해당 장면 결과는 탐색 측정입니다. 새 temporal 기법은 아직 없으며 현재 준비 프레임은 cache 안정화를 위한 사용자 정책입니다.");
}

void Client::CRenderingBenchmark::Render_Results()
{
    if (m_Runs.empty()) return;
    ImGui::SeparatorText("이전 결과 비교 / 프레임 비용");
    ImGui::BeginDisabled(m_bCapturing);
    if (ImGui::Button("결과 JSON 저장")) Queue_Save();
    ImGui::SameLine(); if (ImGui::Button("목록 비우기")) {m_Runs.clear();m_iCompareFirst=m_iCompareSecond=-1;ImGui::EndDisabled();return;}
    ImGui::EndDisabled();
    const auto select=[&](const char* label,int& index) {
        const char* preview=index>=0&&static_cast<size_t>(index)<m_Runs.size()?m_Runs[index].strLabel.c_str():"선택";
        if (ImGui::BeginCombo(label,preview))
        {
            for (size_t i=0;i<m_Runs.size();++i)
            {
                const auto& r=m_Runs[i]; const string name=std::to_string(i+1)+". "+r.strLabel+" ["+r.strVariant+"] #"+std::to_string(r.repetition);
                if (ImGui::Selectable(name.c_str(),index==static_cast<int>(i))) index=static_cast<int>(i);
            }
            ImGui::EndCombo();
        }
    };
    if (m_iCompareFirst<0 && m_Runs.size()>1) m_iCompareFirst=static_cast<int>(m_Runs.size()-2);
    select("기준 결과",m_iCompareFirst); select("비교 결과",m_iCompareSecond);
    if (m_iCompareFirst>=0&&m_iCompareSecond>=0&&static_cast<size_t>(m_iCompareFirst)<m_Runs.size()&&static_cast<size_t>(m_iCompareSecond)<m_Runs.size())
    {
        const auto& a=m_Runs[m_iCompareFirst]; const auto& b=m_Runs[m_iCompareSecond];
        const bool same=a.bConditionsStable&&b.bConditionsStable&&a.fieldMask==b.fieldMask&&
            a.strComparisonConditions==b.strComparisonConditions&&a.strExperimentId==b.strExperimentId;
        if (!same) ImGui::TextWrapped("비교 제외: 공통 조건·독립 변수·실험 ID가 다르거나 수집 중 조건이 바뀌었습니다. 각 run 원본 값은 아래에서 확인할 수 있습니다.");
        else
        {
            ImGui::Text("비교 - 기준: CPU %+.3f ms | interval %+.3f ms | draw %+.0f | index %+.0f | mesh draw %+.0f",
                b.fCpuAvgMs-a.fCpuAvgMs,b.fIntervalAvgMs-a.fIntervalAvgMs,b.fDrawCallsAvg-a.fDrawCallsAvg,b.fIndicesAvg-a.fIndicesAvg,b.fMeshDrawsAvg-a.fMeshDrawsAvg);
            if (a.iGpuFrames==a.iFrames&&b.iGpuFrames==b.iFrames)
                ImGui::Text("GPU %+.3f ms | GPU p99 %+.3f ms | PS 호출 %+.0f",b.fGpuAvgMs-a.fGpuAvgMs,b.fGpuP99Ms-a.fGpuP99Ms,b.fPsInvocationsAvg-a.fPsInvocationsAvg);
            else ImGui::TextDisabled("GPU 비교 불가: 유효 GPU 표본이 전체 프레임보다 적습니다.");
            if ((a.iGpuScopeFrames!=a.iFrames || b.iGpuScopeFrames!=b.iFrames))
                ImGui::TextDisabled("GPU 패스 비교 N/A: 상세 scope 표본이 불완전합니다.");
            else if (ImGui::CollapsingHeader("GPU 패스별 비교 (부모/자식 중복 합산 금지)"))
            {
                std::map<string,std::pair<RENDERING_BENCHMARK_PASS,RENDERING_BENCHMARK_PASS>> rows;
                for (const auto& p:a.passes) rows[p.name].first=p;
                for (const auto& p:b.passes) rows[p.name].second=p;
                for (const auto& [name,p]:rows)
                    ImGui::Text("%s | self %+.3f ms | 전체 %+.3f ms | draw %+.1f | index %+.0f",name.c_str(),
                        p.second.selfMs-p.first.selfMs,p.second.inclusiveMs-p.first.inclusiveMs,p.second.drawCalls-p.first.drawCalls,p.second.indices-p.first.indices);
            }
            if (a.iCpuScopeFrames==a.iFrames && b.iCpuScopeFrames==b.iFrames && ImGui::CollapsingHeader("CPU scope 비교 (thread별 inclusive, 합산 금지)"))
            {
                std::map<string,std::pair<double,double>> rows;
                for (const auto& p:a.cpuPasses) rows[p.name].first=p.inclusiveMs;
                for (const auto& p:b.cpuPasses) rows[p.name].second=p.inclusiveMs;
                for (const auto& [name,p]:rows) ImGui::Text("%s | %+.3f ms",name.c_str(),p.second-p.first);
            }
        }
    }
    for (size_t i=0;i<m_Runs.size();++i)
    {
        const auto& r=m_Runs[i]; ImGui::PushID(static_cast<int>(i));
        const string title=std::to_string(i+1)+". "+r.strLabel+" ["+r.strVariant+"] "+(r.bConditionsStable?"입력 안정 (탐색)":"조건 변경");
        if (ImGui::TreeNode(title.c_str()))
        {
            ImGui::Text("CPU %u | GPU 유효 %u | pending %u | GPU 무효 %u | 준비 %u 프레임",r.iFrames,r.iGpuFrames,r.pendingGpuFrames,r.invalidGpuFrames,r.warmupFrames);
            ImGui::Text("CPU 평균/중앙/p95/p99/max: %.3f / %.3f / %.3f / %.3f / %.3f ms",r.fCpuAvgMs,r.fCpuP50Ms,r.fCpuP95Ms,r.fCpuP99Ms,r.fCpuMaxMs);
            if (r.iGpuFrames) ImGui::Text("GPU 평균/중앙/p95/p99/max: %.3f / %.3f / %.3f / %.3f / %.3f ms",r.fGpuAvgMs,r.fGpuP50Ms,r.fGpuP95Ms,r.fGpuP99Ms,r.fGpuMaxMs);
            else ImGui::TextDisabled("GPU N/A (미지원·무효·대기)");
            ImGui::Text("interval 평균/중앙/p95/p99/max: %.3f / %.3f / %.3f / %.3f / %.3f ms (%u 표본)",r.fIntervalAvgMs,r.fIntervalP50Ms,r.fIntervalP95Ms,r.fIntervalP99Ms,r.fIntervalMaxMs,r.iIntervalFrames);
            if (r.fIntervalAvgMs>0) ImGui::Text("관측 FPS %.2f | 목표 %.0f FPS / %.3f ms",1000.0/r.fIntervalAvgMs,m_fTargetFps,1000.0/m_fTargetFps);
            ImGui::Text("draw %.0f | mesh draw %.0f | 고유 mesh %.0f | instance %.0f | index %.0f",r.fDrawCallsAvg,r.fMeshDrawsAvg,r.fUniqueMeshesAvg,r.fInstancesAvg,r.fIndicesAvg);
            ImGui::Text("scope 누락 CPU %llu / GPU %llu",static_cast<unsigned long long>(r.droppedCpuScopes),static_cast<unsigned long long>(r.droppedGpuScopes));
            if (!r.strFailureReason.empty()) ImGui::TextWrapped("%s",r.strFailureReason.c_str());
            const auto& fields=CRenderingProfileService::Experiment_Fields();
            for (size_t n=0;n<fields.size();++n) if (r.fieldMask&(uint64_t{1}<<n)) ImGui::Text("%s = %.6g",fields[n].id,r.appliedValues.values[n]);
            ImGui::TreePop();
        }
        ImGui::PopID();
    }
}

void Client::CRenderingBenchmark::Render_Section(Engine::CProfiler* profiler,
    const string& qualitySummary, CRenderingProfileService& profiles)
{
    Render_ExperimentSection(profiler,profiles);
    Render_Results();
    RenderingTechniqueGuide::Render();
    ImGui::BeginDisabled(m_bExperimentActive);
    const bool changedProfile=Render_RestorationSection(profiles);
    const bool changedPixels=Render_PixelInputs();
    ImGui::SeparatorText("기존 재질 A/B / 현재 화면 측정");
    ImGui::BeginDisabled(m_bCapturing||changedProfile||changedPixels);
    if (ImGui::Button("현재 화면 측정")) {m_iSequenceStep=0;Begin(profiler,m_LabelBuffer.data(),static_cast<uint32_t>(m_iFrameInput),qualitySummary,m_strStatus);}
    for (const bool recovered:{false,true})
    {
        ImGui::SameLine();
        if (ImGui::Button(recovered?"원본 복원 재질 B 측정":"Legacy 재질 A 측정"))
        {
            auto& game=Engine::CGameInstance::Get(); const auto previous=game.Get_MaterialRenderSettings();
            auto selected=previous;selected.bUseSourceMaterials=recovered;
            if (SUCCEEDED(game.Apply_MaterialRenderSettings(selected)))
            {
                if (!Begin(profiler,m_LabelBuffer.data(),static_cast<uint32_t>(m_iFrameInput),qualitySummary,m_strStatus)) game.Apply_MaterialRenderSettings(previous);
                else {if (!m_bPixelDiagnosticsActive)m_PixelEntrySettings=previous;m_bPixelDiagnosticsActive=true;m_iPixelDiagnosticsLevel=game.Get_CurrentLevelID();}
            }
        }
    }
    ImGui::EndDisabled();ImGui::EndDisabled();
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
    Stream.imbue(locale::classic());
    Stream << setprecision(17);
    Stream << "{\n  \"schema\": \"LostArkRenderingBenchmark.v2\",\n"
        "  \"timingContract\": \"CPU/GPU overlap; pass intervals inclusive; pending is not zero; dynamic gameplay is exploratory.\",\n"
        "  \"runs\": [\n";
    const auto& fields=CRenderingProfileService::Experiment_Fields();
    for (size_t iRun=0;iRun<Runs.size();++iRun)
    {
        const auto& r=Runs[iRun];
        Stream << "    {\n"
            << "      \"label\": \"" << Escape_Json(r.strLabel) << "\",\n"
            << "      \"timestamp\": \"" << Escape_Json(r.strTimestamp) << "\",\n"
            << "      \"qualitySummary\": \"" << Escape_Json(r.strQualitySummary) << "\",\n"
            << "      \"experimentId\": \"" << Escape_Json(r.strExperimentId) << "\",\n"
            << "      \"variant\": \"" << Escape_Json(r.strVariant) << "\",\n"
            << "      \"sourceMaterials\": " << (r.bSourceMaterials?"true":"false") << ",\n"
            << "      \"conditionsStable\": " << (r.bConditionsStable?"true":"false") << ",\n"
            << "      \"failureReason\": \"" << Escape_Json(r.strFailureReason) << "\",\n"
            << "      \"commonConditionFingerprint\": \"" << Escape_Json(r.strComparisonConditions) << "\",\n"
            << "      \"actualConditionFingerprint\": \"" << Escape_Json(r.strFullConditions) << "\",\n"
            << "      \"fieldMask\": " << r.fieldMask << ",\n      \"selectedFields\": [";
        bool comma=false;
        for (size_t i=0;i<fields.size();++i) if (r.fieldMask&(uint64_t{1}<<i))
        { if(comma)Stream<<',';comma=true;Stream<<'"'<<fields[i].id<<'"'; }
        Stream << "],\n      \"effectiveValues\": {";
        for (size_t i=0;i<fields.size();++i)
        { if(i)Stream<<',';Stream<<'"'<<fields[i].id<<"\":"<<r.appliedValues.values[i]; }
        Stream << "},\n"
            << "      \"firstFrame\": " << r.firstFrame << ", \"lastFrame\": " << r.lastFrame << ",\n"
            << "      \"frames\": " << r.iFrames << ", \"warmupFrames\": " << r.warmupFrames << ", \"repetition\": " << r.repetition << ",\n"
            << "      \"gpuFrames\": " << r.iGpuFrames << ", \"pendingGpuFrames\": " << r.pendingGpuFrames << ", \"invalidGpuFrames\": " << r.invalidGpuFrames << ",\n"
            << "      \"cpuScopeFrames\": " << r.iCpuScopeFrames << ", \"gpuScopeFrames\": " << r.iGpuScopeFrames << ",\n"
            << "      \"droppedCpuScopes\": " << r.droppedCpuScopes << ", \"droppedGpuScopes\": " << r.droppedGpuScopes << ",\n"
            << "      \"cpuAvgMs\": " << r.fCpuAvgMs << ", \"cpuP50Ms\": " << r.fCpuP50Ms << ", \"cpuP95Ms\": " << r.fCpuP95Ms << ", \"cpuP99Ms\": " << r.fCpuP99Ms << ", \"cpuMaxMs\": " << r.fCpuMaxMs << ",\n"
            << "      \"gpuAvgMs\": " << (r.iGpuFrames?std::to_string(r.fGpuAvgMs):"null") << ", \"gpuP50Ms\": " << (r.iGpuFrames?std::to_string(r.fGpuP50Ms):"null")
            << ", \"gpuP95Ms\": " << (r.iGpuFrames?std::to_string(r.fGpuP95Ms):"null") << ", \"gpuP99Ms\": " << (r.iGpuFrames?std::to_string(r.fGpuP99Ms):"null") << ", \"gpuMaxMs\": " << (r.iGpuFrames?std::to_string(r.fGpuMaxMs):"null") << ",\n"
            << "      \"intervalFrames\": " << r.iIntervalFrames << ", \"intervalAvgMs\": " << r.fIntervalAvgMs << ", \"intervalP50Ms\": " << r.fIntervalP50Ms << ", \"intervalP95Ms\": " << r.fIntervalP95Ms << ", \"intervalP99Ms\": " << r.fIntervalP99Ms << ", \"intervalMaxMs\": " << r.fIntervalMaxMs << ",\n"
            << "      \"drawCallsAvg\": " << r.fDrawCallsAvg << ", \"instancesAvg\": " << r.fInstancesAvg << ", \"indicesAvg\": " << r.fIndicesAvg << ",\n"
            << "      \"meshDrawsAvg\": " << r.fMeshDrawsAvg << ", \"uniqueMeshesAvg\": " << r.fUniqueMeshesAvg << ",\n"
            << "      \"psInvocationsAvg\": " << (r.iGpuFrames?std::to_string(r.fPsInvocationsAvg):"null") << ",\n";
        const auto writePasses=[&](const char* key,const auto& passes,bool cpu) {
            Stream << "      \"" << key << "\": [";
            for (size_t i=0;i<passes.size();++i)
            {
                const auto& p=passes[i];if(i)Stream<<',';
                Stream<<"{\"name\":\""<<Escape_Json(p.name)<<"\",\"validFrames\":"<<p.validFrames<<",\"inclusiveMs\":"<<p.inclusiveMs;
                if(!cpu)Stream<<",\"selfMs\":"<<p.selfMs<<",\"drawCalls\":"<<p.drawCalls<<",\"indices\":"<<p.indices;
                Stream<<'}';
            }
            Stream<<']';
        };
        writePasses("cpuScopes",r.cpuPasses,true); Stream<<",\n";
        writePasses("gpuScopes",r.passes,false);
        int baseline=-1;
        for (size_t previous=0;previous<iRun;++previous)
        {
            const auto& a=Runs[previous];
            if (a.strVariant=="A" && a.bConditionsStable && r.bConditionsStable && a.fieldMask==r.fieldMask &&
                a.strExperimentId==r.strExperimentId && a.strComparisonConditions==r.strComparisonConditions)
            { baseline=static_cast<int>(previous);break; }
        }
        Stream<<",\n      \"deltaFromA\": ";
        if (baseline<0) Stream<<"null";
        else
        {
            const auto& a=Runs[baseline]; const bool validGpu=a.iGpuFrames==a.iFrames && r.iGpuFrames==r.iFrames;
            Stream<<"{\"runIndex\":"<<baseline<<",\"cpuAvgMs\":"<<r.fCpuAvgMs-a.fCpuAvgMs
                <<",\"cpuP99Ms\":"<<r.fCpuP99Ms-a.fCpuP99Ms<<",\"intervalAvgMs\":"<<r.fIntervalAvgMs-a.fIntervalAvgMs
                <<",\"gpuAvgMs\":"<<(validGpu?std::to_string(r.fGpuAvgMs-a.fGpuAvgMs):"null")
                <<",\"gpuP99Ms\":"<<(validGpu?std::to_string(r.fGpuP99Ms-a.fGpuP99Ms):"null")
                <<",\"drawCalls\":"<<r.fDrawCallsAvg-a.fDrawCallsAvg<<",\"indices\":"<<r.fIndicesAvg-a.fIndicesAvg
                <<",\"meshDraws\":"<<r.fMeshDrawsAvg-a.fMeshDrawsAvg<<'}';
        }
        Stream<<"\n    }"<<(iRun+1<Runs.size()?",":"")<<"\n";
    }
	Stream << "  ]\n}\n";
	Stream.close();
	if (!Stream)
	{
		filesystem::remove(TemporaryPath, Error);
		strOutError = "Failed while writing benchmark JSON.";
		return false;
	}
    // Win32 explicitly omits REPLACE_EXISTING: a name collision preserves the
    // old capture instead of allowing filesystem::rename to replace it.
    if (!MoveFileExW(TemporaryPath.c_str(),OutputPath.c_str(),MOVEFILE_WRITE_THROUGH))
    {
        const auto failure=GetLastError(); filesystem::remove(TemporaryPath,Error);
        strOutError="Cannot finalize benchmark JSON (Win32 "+std::to_string(failure)+"). Existing output preserved.";
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
	static uint64_t fileSequence=0;
    FileName << L"benchmark_" << put_time(&LocalTime, L"%Y%m%d_%H%M%S") << L'_' << GetCurrentProcessId()
        << L'_' << GetTickCount64() << L'_' << ++fileSequence << L".json";
	return (BaseDirectory / L".." / L"BenchmarkCaptures" / FileName.str())
		.lexically_normal();
}
