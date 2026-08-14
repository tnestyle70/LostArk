#pragma once

#include "Client_Defines.h"
#include "Effect_AuthoringDocument.h"

#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

NS_BEGIN(Client)

class DATA_JSON_VALUE;
struct EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM;

/* Stable acceptance counters for the one-way Artist F Track A -> authored
   document migration.  They deliberately describe authored data, not GPU
   admission or visual acceptance. */
struct EFFECT_ARTIST31470_UNIFIED_UPGRADE_STATS final
{
	size_t iCoreElementCount = 0u;
	size_t iParticleElementCount = 0u;
	size_t iFixedBurstEmitterCount = 0u;
	uint64_t iFixedBurstTotal = 0u;
	size_t iRootBasisBakedCount = 0u;
	size_t iFollowBasisBakedCount = 0u;
	size_t iTypedMaterialCount = 0u;
	size_t iFiniteCommonCount = 0u;
	size_t iFailClosedCount = 0u;
	size_t iMeshPreScaleCount = 0u;
	size_t iPortableParticleRecipeCount = 0u;
	size_t iPortableParticleModuleCount = 0u;
	size_t iPortableParticleDistributionCount = 0u;
};

/* Captured source occurrence inputs for lowering one reconstructed Element to
   an ordinary authored starting state.  The caller owns animation sampling:
   a follow attachment is never approximated by the codec when its exact
   emit-start parent-local transform is unavailable. */
struct EFFECT_GENERIC_AUTHORED_STARTING_BAKE_REQUEST final
{
	/* Cue and emitter-local position/rotation/scale are collapsed in source
	   composition order.  Dynamic velocity and revolution remain the
	   already-lowered ordinary Element's values. */
	EFFECT_TRANSFORM_DESC CueLocalTransform;
	EFFECT_TRANSFORM_DESC EmitterLocalTransform;
	f32_t fScheduleStartDelaySeconds = 0.f;
	f32_t fScheduleLifeTimeSeconds = 1.f;
	f32_t fEmitterDelaySeconds = 0.f;
	f32_t fEmitterDurationSeconds = 0.f;
	uint32_t iEmitterLoopCount = 1u;
	bool_t bAttachmentEnabled = false;
	bool_t bFollowAttachment = false;
	f32_t fSnapshotRootSourceBasisYawDegrees = 0.f;
	bool_t bHasFollowParentLocalTransform = false;
	/* Exact sampled suffix after cue-local space and before the ordinary root.
	   The caller includes any socket/anchor conversion needed by its runtime. */
	float4x4_t FollowParentLocalTransform{};
	float3_t vSourceTypeDataRotationDegrees = { 0.f, 0.f, 0.f };
	bool_t bTransformInheritanceEnabled = false;
};

/* One source occurrence imported into one ordinary authored Element.  The
   source identity remains read-only evidence while the target identity and
   display metadata are caller-owned stable authored data.  A material
   execution override, when present, must already have been compiled and
   admitted by a character-independent material compiler. */
struct EFFECT_GENERIC_AUTHORED_ELEMENT_IMPORT_REQUEST final
{
	std::string strSourceElementId;
	std::string strTargetElementId;
	std::string strTargetGroupId;
	std::string strTargetDisplayName;
	EFFECT_GENERIC_AUTHORED_STARTING_BAKE_REQUEST StartingState;
	bool_t bOverrideMaterialExecution = false;
	EFFECT_MATERIAL_EXECUTION_DESC MaterialExecution;
};

class CEffectDocumentCodec final
{
public:
	static bool_t Validate(
		const EFFECT_DOCUMENT_DESC& Document,
		std::string& strOutError);
	static bool_t Validate_SourceContract(
		const EFFECT_DOCUMENT_DESC& Document,
		std::string& strOutError);
	static bool_t Validate_Drawable(
		const EFFECT_DOCUMENT_DESC& Document,
		std::string& strOutError);
	static bool_t Validate_ReconstructedRuntimeDrawable(
		const EFFECT_DOCUMENT_DESC& Document,
		std::string& strOutError);
	static bool_t Validate_Artist31470ReconstructedRuntimeDrawable(
		const EFFECT_DOCUMENT_DESC& Document,
		std::string& strOutError);
	static bool_t Build_GenericAuthoredElementStartingCopy(
		const EFFECT_DOCUMENT_DESC& SourceDocument,
		std::string_view strElementId,
		std::string_view strTargetEffectAssetId,
		EFFECT_DOCUMENT_DESC& InOutDocument,
		std::string& strOutError);
	static bool_t Bake_GenericAuthoredElementStartingState(
		const EFFECT_ELEMENT_DESC& LoweredElement,
		const EFFECT_GENERIC_AUTHORED_STARTING_BAKE_REQUEST& Request,
		EFFECT_ELEMENT_DESC& OutBakedElement,
		std::string& strOutError);
	static bool_t Merge_GenericAuthoredElements(
		const EFFECT_DOCUMENT_DESC& TargetDocument,
		const std::vector<EFFECT_ELEMENT_DESC>& Elements,
		EFFECT_DOCUMENT_DESC& InOutDocument,
		std::string& strOutError);
	/* Materializes the executable particle-module subset of a native source
	   Element into the ordinary v13 sourceRecipe carrier. Native-v14 evidence,
	   admission, receipt, and geometry-authority fields never cross this seam;
	   authored Transform/Visible/resources/material execution remain owned by
	   the target Element. */
	static bool_t Apply_PortableAuthoredParticleRuntimeCarrier(
		const EFFECT_ELEMENT_DESC& SourceElement,
		EFFECT_ELEMENT_DESC& InOutElement,
		std::string& strOutError);
	/* Transactional one-way Track A/source occurrence import.  Native
	   provenance never crosses the authored boundary, portable Particle
	   execution is attached only after ordinary lowering/merge, and no input
	   or prior output is changed on failure. */
	static bool_t Build_GenericAuthoredElementImportStage(
		const EFFECT_DOCUMENT_DESC& SourceDocument,
		const EFFECT_DOCUMENT_DESC& TargetDocument,
		const EFFECT_GENERIC_AUTHORED_ELEMENT_IMPORT_REQUEST& Request,
		EFFECT_DOCUMENT_DESC& InOutDocument,
		std::string& strOutError);
	/* Data-only migration used by both the Effect Tool and the offline
	   materializer.  Existing authored transforms, generic resource bindings,
	   and same-lane DDS overrides remain authoritative; Track A supplies the
	   particle carrier and executable material recipe. */
	static bool_t Build_Artist31470UnifiedTrackAUpgrade(
		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
		const EFFECT_DOCUMENT_DESC& SourceDocument,
		const std::unordered_map<std::string,
			EFFECT_MATERIAL_EXECUTION_DESC>& MaterialSnapshots,
		const EFFECT_DOCUMENT_DESC& ExistingDocument,
		EFFECT_DOCUMENT_DESC& OutDocument,
		EFFECT_ARTIST31470_UNIFIED_UPGRADE_STATS& OutStats,
		std::string& strOutError);
	static bool_t Validate_Artist31470UnifiedTrackAUpgrade(
		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
		const EFFECT_DOCUMENT_DESC& SourceDocument,
		const EFFECT_DOCUMENT_DESC& Document,
		EFFECT_ARTIST31470_UNIFIED_UPGRADE_STATS& OutStats,
		std::string& strOutError);
	/* Metadata-only readiness gate for the saved/editing authored parent.  It
	   proves the stable Core33 joins and persisted Track A execution values
	   without rebuilding the source projection or touching GPU resources. */
	static bool_t Validate_Artist31470UnifiedAuthoredReadiness(
		const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM& Program,
		const EFFECT_DOCUMENT_DESC& Document,
		EFFECT_ARTIST31470_UNIFIED_UPGRADE_STATS& OutStats,
		std::string& strOutError);

	static bool_t Parse(
		std::string_view Json,
		EFFECT_DOCUMENT_DESC& OutDocument,
		std::string& strOutError);

	static bool_t Parse_Value(
		const DATA_JSON_VALUE& Value,
		EFFECT_DOCUMENT_DESC& OutDocument,
		std::string& strOutError);

	static std::string Serialize(const EFFECT_DOCUMENT_DESC& Document);

	static bool_t Load(
		const std::filesystem::path& Path,
		EFFECT_DOCUMENT_DESC& OutDocument,
		std::string& strOutError);

	static bool_t Save_Atomic(
		const std::filesystem::path& Path,
		const EFFECT_DOCUMENT_DESC& Document,
		std::string& strOutError);
	static bool_t Save_AtomicIfUnchanged(
		const std::filesystem::path& Path,
		const EFFECT_DOCUMENT_DESC& Document,
		std::string_view strExpectedCanonicalDocument,
		std::string& strOutError);

	static void Collect_ResourceAssetIds(
		const EFFECT_DOCUMENT_DESC& Document,
		std::vector<std::string>& OutAssetIds);

	static const char_t* To_Token(EFFECT_ELEMENT_KIND eKind);
	static const char_t* To_Token(EFFECT_RESOURCE_SLOT eSlot);
	static const char_t* To_Token(EFFECT_RENDER_PROFILE eProfile);
	static const char_t* To_Token(
		EFFECT_MATERIAL_EXECUTION_BACKEND eBackend);
	static bool_t Is_ResourceSlotAllowed(
		EFFECT_ELEMENT_KIND eKind,
		EFFECT_RESOURCE_SLOT eSlot);
	static bool_t Is_SafeResourceAssetId(
		const std::string& strAssetId,
		EFFECT_RESOURCE_FILE_KIND* pOutKind = nullptr);
	static bool_t Is_SafeModelCueAssetId(const std::string& strAssetId);
};

NS_END
