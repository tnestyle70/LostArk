#pragma once

#include "Client_Defines.h"
#include "DeployPropObject.h"
#include "Effect_PresentationService.h"
#include "MapEffectDocument.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

NS_BEGIN(Client)

class CDeployPropRuntime;
class CEncounterPatternReference;
class CWorldDestructionProjectionDocument;
struct ENCOUNTER_STAGE_REFERENCE;
struct VALTAN_PRESENTATION_STATE;

/* Product consumer for <AreaId>.mapeffects.json.  Surface packets are staged
   against stable Deploy/destruction owners and committed in one transaction.
   World Effects remain level-owned and are sampled from the authoritative
   boss stage clock; this class never creates a fake Character/Boss owner. */
class CMapEffectPresentationRuntime final
{
public:
	CMapEffectPresentationRuntime() = default;
	~CMapEffectPresentationRuntime();

	CMapEffectPresentationRuntime(const CMapEffectPresentationRuntime&) = delete;
	CMapEffectPresentationRuntime& operator=(
		const CMapEffectPresentationRuntime&) = delete;

	bool_t Load_Area(
		uint32_t levelIndex,
		const std::string& areaId,
		CDeployPropRuntime& deployRuntime,
		const CWorldDestructionProjectionDocument& destructionProjection,
		const CEncounterPatternReference& encounterReference,
		std::string& outStatus);
	void Update_ServerPresentation(
		const VALTAN_PRESENTATION_STATE& boss,
		f32_t timeDelta);
	void Clear();

	/* Explicit authoring boundary used by Effect Tool.  Product activation still
	   calls Load_Area and reads only the published Map data root.  These requests
	   target the one live Area runtime and preserve its current presentation if
	   candidate validation/staging fails. */
	static bool_t Request_DebugApply(
		uint32_t levelIndex,
		const CMapEffectDocument& stagedDocument,
		std::string& outStatus);
	static bool_t Request_PublishedReload(
		uint32_t levelIndex,
		std::string& outStatus);

	bool_t Is_Loaded() const { return m_isLoaded; }
	const CMapEffectDocument& Get_Document() const { return m_Document; }
	const std::string& Get_Status() const { return m_Status; }

private:
	struct ACTIVE_WORLD_EFFECT final
	{
		/* Immutable occurrence snapshot.  A successful hot reload replaces the
		   definition used by future spawns only; an already-active handle keeps
		   sampling the presentation/revision that created it. */
		MAP_EFFECT_WORLD_PRESENTATION Presentation;
		EFFECT_WORLD_ROOT_HANDLE Handle;
		f32_t fDurationSeconds = 0.f;
		uint32_t iPatternSequence = 0u;
		uint32_t iActionStartTick = 0u;
		uint32_t iLastServerTick = 0u;
		f32_t fLastSampleSeconds = 0.f;
	};
	struct SURFACE_EMISSIVE_BASELINE final
	{
		f32_t fEmissiveIntensity = 1.f;
		float4_t vEmissiveColor = float4_t(1.f, 1.f, 1.f, 1.f);
		f32_t fEmissiveMaskPower = 1.f;
	};

	bool_t Validate_AndStageSurfacePackets(
		const CMapEffectDocument& stagedDocument,
		CDeployPropRuntime& deployRuntime,
		const CWorldDestructionProjectionDocument& destructionProjection,
		const std::unordered_map<uint64_t, SURFACE_EMISSIVE_BASELINE>&
			previousBaselines,
		std::vector<std::pair<uint64_t, DEPLOY_SURFACE_PRESENTATION_PACKET>>&
			outPackets,
		std::unordered_map<uint64_t, SURFACE_EMISSIVE_BASELINE>&
			outBaselines,
		std::string& outStatus) const;
	bool_t Validate_WorldEffects(
		const CMapEffectDocument& stagedDocument,
		const CEncounterPatternReference& encounterReference,
		std::vector<f32_t>& outDurations,
		std::string& outStatus);
	bool_t Probe_WorldEffectAdmissions(
		uint32_t levelIndex,
		const CMapEffectDocument& stagedDocument,
		const std::vector<f32_t>& durations,
		std::string& outStatus) const;
	bool_t Commit_StagedDocument(
		uint32_t levelIndex,
		CMapEffectDocument stagedDocument,
		CDeployPropRuntime& deployRuntime,
		const CWorldDestructionProjectionDocument& destructionProjection,
		const CEncounterPatternReference& encounterReference,
		std::string& outStatus);
	float4x4_t Build_WorldRoot(
		const MAP_EFFECT_WORLD_PRESENTATION& presentation) const;
	bool_t Resolve_WorldSample(
		const MAP_EFFECT_WORLD_PRESENTATION& presentation,
		const ACTIVE_WORLD_EFFECT* active,
		const VALTAN_PRESENTATION_STATE& boss,
		const ENCOUNTER_STAGE_REFERENCE* activeStage,
		f32_t timeDelta,
		f32_t& outSampleSeconds,
		uint32_t& outOccurrenceSequence,
		uint32_t& outSampleStartTick) const;
	void Stop_WorldEffect(size_t index);
	void Restore_CurrentSurfaceBaselines();

private:
	uint32_t m_iLevelIndex = ETOUI(LEVEL::END);
	CMapEffectDocument m_Document;
	std::string m_AreaId;
	CDeployPropRuntime* m_pDeployRuntime = nullptr;
	const CWorldDestructionProjectionDocument* m_pDestructionProjection = nullptr;
	const CEncounterPatternReference* m_pEncounterReference = nullptr;
	std::vector<f32_t> m_WorldEffectDurations;
	std::vector<ACTIVE_WORLD_EFFECT> m_ActiveWorldEffects;
	std::unordered_map<uint64_t, SURFACE_EMISSIVE_BASELINE>
		m_SurfaceEmissiveBaselines;
	std::unordered_map<std::string, uint32_t>
		m_LastAttemptedServerSequenceByPlacement;
	std::unordered_set<std::string> m_LevelActiveSpawnAttemptedPlacements;
	std::string m_Status = "Map Effect presentation not loaded";
	bool_t m_isLoaded = false;
};

NS_END
