#pragma once

#include "Client_Defines.h"
#include "Profiler.h"
#include "Engine_RenderTypes.h"
#include "RenderingProfileService.h"
#include "ProfilerCaptureIO.h"

#include <array>
#include <filesystem>
#include <string>
#include <vector>
#include <future>
#include <map>

NS_BEGIN(Client)

class CRenderingProfileService;

struct RENDERING_BENCHMARK_PASS final
{
    string name;
    uint32_t validFrames = 0;
    double inclusiveMs = 0, selfMs = 0, drawCalls = 0, indices = 0;
};

struct RENDERING_BENCHMARK_RUN final
{
	string strLabel;
	string strTimestamp;
	string strQualitySummary;
	string strComparisonConditions;
    string strFullConditions, strExperimentId, strVariant, strFailureReason;
    string strRecipeId, strExperimentGoal, strMetricGuide, strConfidence;
    string strComparisonRowId, strMeasurementId, strComparisonSessionId;
    RENDERING_EXPERIMENT_VALUES expectedA, expectedB;
    std::array<double, static_cast<size_t>(Engine::EProfilerCounter::Count)> counterTotals{};
    std::array<Engine::FProfilerWorkStats, static_cast<size_t>(Engine::EProfilerWork::Count)> workTotals{};
    double iaVerticesTotal = 0, iaPrimitivesTotal = 0, vsInvocationsTotal = 0;
    string rawEvidencePath, rawEvidenceStatus; // Same sampled frame window; never the later live history.
    bool rawEvidenceReady = false;
    string applicability;
    std::map<string,string> commonConditionFields, actualConditionFields, changedConditionFields;
    uint64_t fieldMask = 0, firstFrame = 0, lastFrame = 0;
    uint32_t warmupFrames = 0, repetition = 1, pendingGpuFrames = 0, invalidGpuFrames = 0;
    uint64_t droppedCpuScopes = 0, droppedGpuScopes = 0;
    RENDERING_EXPERIMENT_VALUES appliedValues;
    double fCpuP50Ms = 0, fCpuP99Ms = 0, fGpuP50Ms = 0, fGpuP99Ms = 0;
    double fIntervalAvgMs = 0, fIntervalP50Ms = 0, fIntervalP95Ms = 0, fIntervalP99Ms = 0, fIntervalMaxMs = 0;
    uint32_t iIntervalFrames = 0, iGpuScopeFrames = 0, iCpuScopeFrames = 0;
    double fMeshDrawsAvg = 0, fUniqueMeshesAvg = 0;
    vector<RENDERING_BENCHMARK_PASS> passes, cpuPasses;
	bool_t bSourceMaterials = true;
	bool_t bConditionsStable = true;
	uint32_t iFrames = 0u;
	uint32_t iGpuFrames = 0u;
	double fCpuAvgMs = 0.0;
	double fCpuP95Ms = 0.0;
	double fCpuMaxMs = 0.0;
	double fGpuAvgMs = 0.0;
	double fGpuP95Ms = 0.0;
	double fGpuMaxMs = 0.0;
	double fDrawCallsAvg = 0.0;
	double fInstancesAvg = 0.0;
	double fIndicesAvg = 0.0;
	double fPsInvocationsAvg = 0.0;
};

/* Rendering Workbench benchmark: captures N frames through the Engine
   profiler and records CPU/GPU/draw statistics next to the quality settings
   that were active. Session A/B and bounded one-field sweeps preserve authored
   settings; legacy material-mode captures share the same sample/result path. */
class CRenderingBenchmark final
{
public:
	[[nodiscard]] bool_t Is_Capturing() const noexcept { return m_bCapturing; }
    [[nodiscard]] bool_t Is_ExperimentActive() const noexcept { return m_bExperimentActive; }
    bool_t Start_SessionExperiment(CRenderingProfileService& profiles);
    void Render_SessionBar(CRenderingProfileService& profiles);
    void Render_PresentationSection(CRenderingProfileService& profiles);
    void Render_QuickComparison(CRenderingProfileService& profiles);
    void Render_OptimizationSection(CRenderingProfileService& profiles);
    void Set_DeviceInfo(ID3D11Device* device);
    void Shutdown(CRenderingProfileService& profiles);
	bool_t Begin(
		Engine::CProfiler* pProfiler,
		const string& strLabel,
		uint32_t iFrames,
		const string& strQualitySummary,
		string& strOutStatus);
	/* Called once per frame. Finalizes the run when enough frames exist. */
	void Update(Engine::CProfiler* pProfiler);
	/* Session profile comparison owns only its last successful activation. */
	void Update_RestorationPreview(CRenderingProfileService& Profiles, bool_t bToolVisible);
	void Notify_ProfileReload();
	void Render_Section(
		Engine::CProfiler* pProfiler,
		const string& strQualitySummary,
		CRenderingProfileService& Profiles);

private:
    bool_t Prepare_OptimizationPair(size_t recipeIndex, CRenderingProfileService& profiles);
    bool Queue_RawEvidence(Engine::FProfilerCaptureSnapshot&& snapshot, RENDERING_BENCHMARK_RUN& run);
    void Poll_RawEvidence();
    FProfilerCaptureContext Sample_OptimizationContext() const;
    bool_t Prepare_PresentationPair(int stage, CRenderingProfileService& profiles);
    bool_t Start_ComparisonMeasurement(Engine::CProfiler* profiler);
    void Render_ComparisonCost(const char* rowId);
    void Reset_ComparisonRows();
    void Mark_PreparedComparison(const string& rowId);
    struct COMPARISON_ROW
    {
        string sessionId, measurementId, experimentId, sceneConditions, commonConditions, failure;
        RENDERING_EXPERIMENT_VALUES a, b;
        uint64_t fields=0;
        uint32_t frames=0, warmup=0;
    };
    struct COMPARISON_COST
    {
        bool cpuValid=false, gpuValid=false, partialScopes=false;
        uint32_t frames=0, pendingGpu=0, invalidGpu=0;
        double cpuA=0, cpuB=0, gpuA=0, gpuB=0, cpuSpreadA=0, cpuSpreadB=0;
        string status;
        std::array<double, static_cast<size_t>(Engine::EProfilerCounter::Count)> countersA{}, countersB{};
        std::array<double, static_cast<size_t>(Engine::EProfilerWork::Count)> workA{}, workB{};
        string applicability;
        uint32_t evidenceReady = 0;
    };
    COMPARISON_COST Build_ComparisonCost(const string& rowId, const string& sceneConditions) const;
    void Render_ExperimentSection(Engine::CProfiler* profiler, CRenderingProfileService& profiles);
    void Render_RecipeSection();
    bool_t Apply_PresentationStage(int stage, CRenderingProfileService& profiles);
    bool_t Prepare_QuickComparison(CRenderingProfileService& profiles, bool_t qualitySuiteBase = false);
    bool_t Prepare_QualitySuiteComparison(CRenderingProfileService& profiles);
    bool_t Apply_ComparisonPair(const RENDERING_EXPERIMENT_VALUES& base,
        const RENDERING_EXPERIMENT_VALUES& candidate);
    bool_t Prepare_Recipe(bool_t replaceB);
    bool_t Prepare_RecipeById(const char* recipeId, CRenderingProfileService& profiles);
    bool_t Start_Experiment(CRenderingProfileService& profiles);
    bool_t Apply_ExperimentVariant(bool_t variantB);
    void End_Experiment();
    void Finish_Sequence();
    bool_t Start_Sweep(Engine::CProfiler* profiler);
    void Cancel_Capture(const string& reason, bool restoreOptimization = true);
    void Queue_Save(bool shutdown = false);
    void Poll_Save();
    uint64_t Experiment_FieldMask() const;
    uint64_t Experiment_BaselineMask() const;
    bool_t Adopt_BaselineFromB();
    string Current_Conditions(uint64_t excludedFields, std::map<string,string>* named = nullptr, bool rowContext = false) const;
    void Render_Results();
	bool_t Render_RestorationSection(CRenderingProfileService& Profiles);
	bool_t Render_PixelInputs();
	bool_t Activate_RestorationProfile(CRenderingProfileService& Profiles, const string& strProfileId);
	bool_t Return_ToEntryProfile(CRenderingProfileService& Profiles);
	void Release_RestorationOwnership();
	bool_t Finalize(Engine::CProfiler& Profiler);
	static bool_t Save_Json(
		const vector<RENDERING_BENCHMARK_RUN>& Runs,
		const filesystem::path& OutputPath,
		string& strOutError);
	static filesystem::path Make_DefaultPath();

private:
    bool m_bOptimizationSession = false, m_bKeepOptimizationHidden = false;
    bool m_bEvidenceDrain = false, m_bEvidenceNextVariantB = false;
    bool m_bDeviceInfoValid = false, m_bToolVisible = true;
    uint32_t m_DeviceCreationFlags = 0;
    string m_AdapterName, m_RawSavePath;
    CProfilerCaptureExporter m_RawExporter;
    FProfilerCaptureContext m_OptimizationCaptureContext;
	bool_t m_bCapturing = false;
    bool_t m_bExperimentActive = false, m_bVariantB = false, m_bSequence = false;
    bool_t m_bSequenceProfilerWasEnabled = false;
    bool_t m_bComparisonMeasurement=false, m_bComparisonRestoreVariantB=false;
    string m_strComparisonSessionId, m_strPreparedComparisonRowId, m_strMeasurementId, m_strMeasurementRowId;
    RENDERING_EXPERIMENT_VALUES m_PreparedComparisonA, m_PreparedComparisonB;
    std::map<string,COMPARISON_ROW> m_ComparisonRows;
    int m_iComparisonContextUiFrame=-1;
    string m_strComparisonContext;
    bool_t m_bSweep = false, m_bSweepRestoreVariantB = false;
    int m_iSweepField = 1, m_iSweepSteps = 5;
    float m_fSweepMinimum = .1f, m_fSweepMaximum = 2.f;
    vector<double> m_SweepPoints;
    RENDERING_EXPERIMENT_VALUES m_PreSweepB;
    bool_t m_bWarmup = false, m_bCaptureExperiment = false;
    uint32_t m_iSequenceStep = 0u, m_iSequenceTotal = 0u, m_iRepeatInput = 1u;
    uint32_t m_iWarmupInput = 60u, m_iCaptureWarmup = 0u;
    uint64_t m_iCaptureFields = 0u, m_iLastObservedFrame = 0u;
    uint64_t m_iExperimentProfileGeneration = 0u;
    uint32_t m_iExperimentLevel = 0u;
    string m_strExperimentId, m_strExperimentOwner, m_strCaptureVariant, m_strFullConditions;
    string m_strCaptureExperimentId, m_strFailureReason;
    std::map<string,string> m_CaptureCommonFields, m_CaptureActualFields, m_ChangedConditionFields;
    int m_iSelectedRecipe = 0, m_iPreparedRecipe = -1;
    int m_iPresentationStage = -1, m_iQuickTechnique = 0;
    bool_t m_bQuickQualitySuiteBase = false; // Candidate basis only; never replaces the original restore point.
    RENDERING_EXPERIMENT_VALUES m_ExperimentA, m_ExperimentB, m_ExperimentOriginal, m_CaptureValues;
    CRenderingProfileService* m_pExperimentProfiles = nullptr; // MainApp owns both services.
    Engine::CProfiler* m_pCaptureProfiler = nullptr;
    int m_iCompareFirst = -1, m_iCompareSecond = -1;
    float m_fTargetFps = 60.f;
    struct SAVE_RESULT { bool ok = false; string message; };
    std::future<SAVE_RESULT> m_SaveFuture;
    bool m_bSaveQueued = false;
    string m_strSaveStatus;
	bool_t m_bPixelDiagnosticsActive = false;
	uint32_t m_iPixelDiagnosticsLevel = 0u;
	string m_strPixelMaterialKey;
	Engine::MATERIAL_RENDER_SETTINGS m_PixelEntrySettings;
	bool_t m_bProfilerWasEnabled = false;
	uint64_t m_iStartFrame = 0u;
	uint32_t m_iTargetFrames = 0u;
	string m_strLabel;
	string m_strQualitySummary;
	string m_strComparisonConditions;
	bool_t m_bSourceMaterials = true;
	bool_t m_bConditionsStable = true;
	string m_strStatus = "Select a stage or technique to begin a temporary comparison.";
	array<char_t, 64> m_LabelBuffer = { "baseline" };
	int32_t m_iFrameInput = 300;
	vector<RENDERING_BENCHMARK_RUN> m_Runs;
	uint32_t m_iRestorationLevel = 0u;
	string m_strRestorationEntryProfileId;
	string m_strRestorationLastProfileId;
	string m_strRestorationLevelQualityId;
	string m_strRestorationStatus = "Choose a session comparison profile. No automatic Save or Publish.";
};

NS_END
