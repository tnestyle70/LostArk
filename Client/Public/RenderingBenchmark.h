#pragma once

#include "Client_Defines.h"
#include "Profiler.h"
#include "Engine_RenderTypes.h"

#include <array>
#include <filesystem>
#include <string>
#include <vector>

NS_BEGIN(Client)

class CRenderingProfileService;

struct RENDERING_BENCHMARK_RUN final
{
	string strLabel;
	string strTimestamp;
	string strQualitySummary;
	string strComparisonConditions;
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
   that were active, so A/B changes to SSAO/Bloom/Shadow/Fog are measured on
   the same basis. Explicit Capture A/B changes only the session material mode;
   camera, lighting and quality remain the user's current settings. */
class CRenderingBenchmark final
{
public:
	[[nodiscard]] bool_t Is_Capturing() const noexcept { return m_bCapturing; }
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
	bool_t m_bCapturing = false;
	bool_t m_bPixelDiagnosticsActive = false;
	uint32_t m_iPixelDiagnosticsLevel = 0u;
	string m_strPixelMaterialKey;
	Engine::MATERIAL_RENDER_SETTINGS m_PixelEntrySettings;
	bool_t m_bProfilerWasEnabled = false;
	bool_t m_bSkipActivationFrame = false;
	uint64_t m_iStartFrame = 0u;
	uint32_t m_iTargetFrames = 0u;
	string m_strLabel;
	string m_strQualitySummary;
	string m_strComparisonConditions;
	bool_t m_bSourceMaterials = true;
	bool_t m_bConditionsStable = true;
	string m_strStatus = "Idle. Capture measures the current viewport with the current quality settings.";
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
