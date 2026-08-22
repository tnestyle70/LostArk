#pragma once

#include "AnimationTargetService.h"
#include "CharacterPreviewPanel.h"
#include "AnimationSkillBindingDocument.h"
#include "Client_Defines.h"
#include "Effect_AuthoringDocument.h"
#include "Effect_ComponentDocument.h"
#include "Effect_OccurrenceTuning.h"
#include "EffectAuthoringTransfer.h"
#include "Engine_Defines.h"
#include "PlayerSkillCatalog.h"
#include "ValtanPatternTree.h"

#include <array>
#include <filesystem>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

NS_BEGIN(Engine)
class CModel;
NS_END

NS_BEGIN(Client)

class CEffectObject;
class CEffectThumbnailCache;
class EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION;
struct EFFECT_FIXED_STEP_TRANSFORM_SAMPLE;
struct EFFECT_VISUAL_PROGRAM;
struct EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION;
enum class EFFECT_VISUAL_PROGRAM_FAMILY : uint8_t;
enum class EFFECT_GPU_RENDER_FAMILY : uint8_t;

enum class EFFECT_PREVIEW_PIVOT_KIND : uint8_t
{
    WORLD,
    PLAYER_ROOT,
    WEAPON_SOCKET,
    MODEL_BONE,
    END
};

enum class EFFECT_PREVIEW_FILTER : uint8_t
{
    COMPLETE,
    SOLO_PARTICLE_SYSTEM,
	SOLO_STANDALONE_MESHES,
	SOLO_MESH_EMITTERS,
	SOLO_STANDALONE_SPRITES,
	SOLO_SPRITE_EMITTERS,
    SOLO_SELECTED,
    SOLO_MODEL_CUE,
	SOLO_MODEL_CUES,
	SOLO_AUTHORING_FAMILY,
    MUTE_SELECTED,
    SOLO_SELECTED_GROUP,
    MUTE_SELECTED_GROUP,
    END
};

enum class EFFECT_DETAIL_SELECTION : uint8_t
{
    NONE,
	SKILL,
    PARTICLE_SYSTEM,
    ELEMENT,
	MODEL_CUE,
	COMPONENT,
	EMITTER,
	SOURCE_MODULE,
	RUNTIME_OCCURRENCE,
    END
};

enum class EFFECT_DOCUMENT_SOURCE : uint8_t
{
    NEW_DOCUMENT,
    AUTHORED,
    IMPORTED,
    IMPORTED_REFERENCE,
	MIGRATION_REFERENCE,
	RUNTIME_ASSEMBLY,
	RUNTIME_COMPONENT,
	RUNTIME_VISUAL_PROGRAM,
    END
};

enum class EFFECT_TOOL_ALL_EFFECTS_OWNER_KIND : uint8_t
{
	PLAYER_CLASS,
	VALTAN_BOSS
};

struct EFFECT_TOOL_ALL_EFFECTS_OWNER_OPTION final
{
	EFFECT_TOOL_ALL_EFFECTS_OWNER_KIND eKind =
		EFFECT_TOOL_ALL_EFFECTS_OWNER_KIND::PLAYER_CLASS;
	LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass =
		LostArk::Shared::CHARACTER_CLASS_ID::END;
	std::string_view strLabel;
};

inline constexpr std::array<EFFECT_TOOL_ALL_EFFECTS_OWNER_OPTION, 7u>
	EFFECT_TOOL_ALL_EFFECTS_OWNER_OPTIONS = {{
		{ EFFECT_TOOL_ALL_EFFECTS_OWNER_KIND::PLAYER_CLASS,
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER,
			"Lance Master" },
		{ EFFECT_TOOL_ALL_EFFECTS_OWNER_KIND::PLAYER_CLASS,
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER,
			"Gunslinger" },
		{ EFFECT_TOOL_ALL_EFFECTS_OWNER_KIND::PLAYER_CLASS,
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER,
			"Slayer" },
		{ EFFECT_TOOL_ALL_EFFECTS_OWNER_KIND::PLAYER_CLASS,
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST,
			"Artist" },
		{ EFFECT_TOOL_ALL_EFFECTS_OWNER_KIND::PLAYER_CLASS,
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER,
			"Dimension Master" },
		{ EFFECT_TOOL_ALL_EFFECTS_OWNER_KIND::PLAYER_CLASS,
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD,
			"Warlord" },
		{ EFFECT_TOOL_ALL_EFFECTS_OWNER_KIND::VALTAN_BOSS,
			LostArk::Shared::CHARACTER_CLASS_ID::END,
			"Valtan" }
	}};

enum class EFFECT_AUTHORING_FAMILY : uint8_t
{
	MESH,
	SPRITE,
	MESH_PARTICLE,
	SPRITE_PARTICLE,
	LOCAL_DECAL,
	TRAIL_RIBBON,
	PRESENTATION_LIGHT,
	PRESENTATION_SCREEN_POST,
	END
};

inline EFFECT_AUTHORING_FAMILY Resolve_EffectToolAuthoringFamily(
	const EFFECT_ELEMENT_DESC& Element)
{
	switch (Element.eKind)
	{
	case EFFECT_ELEMENT_KIND::MESH:
		return EFFECT_AUTHORING_FAMILY::MESH;
	case EFFECT_ELEMENT_KIND::SPRITE:
		return EFFECT_AUTHORING_FAMILY::SPRITE;
	case EFFECT_ELEMENT_KIND::PARTICLE:
		for (const EFFECT_RESOURCE_BINDING_DESC& Binding :
			Element.ResourceBindings)
		{
			if (Binding.strSlotId == "meshModel")
				return EFFECT_AUTHORING_FAMILY::MESH_PARTICLE;
		}
		return EFFECT_AUTHORING_FAMILY::SPRITE_PARTICLE;
	case EFFECT_ELEMENT_KIND::DECAL:
		return EFFECT_AUTHORING_FAMILY::LOCAL_DECAL;
	case EFFECT_ELEMENT_KIND::TRAIL:
		return EFFECT_AUTHORING_FAMILY::TRAIL_RIBBON;
	case EFFECT_ELEMENT_KIND::LIGHT:
		return EFFECT_AUTHORING_FAMILY::PRESENTATION_LIGHT;
	case EFFECT_ELEMENT_KIND::SCREEN_POST:
		return EFFECT_AUTHORING_FAMILY::PRESENTATION_SCREEN_POST;
	case EFFECT_ELEMENT_KIND::END:
	default:
		return EFFECT_AUTHORING_FAMILY::END;
	}
}

inline const char_t* Get_EffectToolAuthoringFamilyLabel(
	const EFFECT_AUTHORING_FAMILY eFamily)
{
	switch (eFamily)
	{
	case EFFECT_AUTHORING_FAMILY::MESH: return "Mesh";
	case EFFECT_AUTHORING_FAMILY::SPRITE: return "Sprite";
	case EFFECT_AUTHORING_FAMILY::MESH_PARTICLE: return "Mesh Particle";
	case EFFECT_AUTHORING_FAMILY::SPRITE_PARTICLE: return "Sprite Particle";
	case EFFECT_AUTHORING_FAMILY::LOCAL_DECAL: return "Local Decal";
	case EFFECT_AUTHORING_FAMILY::TRAIL_RIBBON: return "Trail / Ribbon";
	case EFFECT_AUTHORING_FAMILY::PRESENTATION_LIGHT:
		return "Presentation Light";
	case EFFECT_AUTHORING_FAMILY::PRESENTATION_SCREEN_POST:
		return "Presentation Screen Post";
	case EFFECT_AUTHORING_FAMILY::END:
	default: return "Invalid";
	}
}

inline bool_t Is_EffectToolPresentationPreviewAdmitted(
	const EFFECT_ELEMENT_DESC& Element)
{
	if (!Element.bVisible)
		return false;
	return Is_EffectPresentationExecutionTarget(Element);
}

/* The Server reuses its staged-action index for both COMBO attacks and HOLD
   phases. Keep the Product documents phase-local, but give the Tool an exact
   semantic label so a HOLD start/charge/release family is never presented as
   three basic attacks merely because its stable IDs retain the legacy baN
   suffix. */
enum class EFFECT_TOOL_SKILL_PHASE_ROLE : uint8_t
{
	EFFECT,
	BASIC_ATTACK,
	HOLD_START,
	HOLD_CHARGE,
	HOLD_RELEASE,
	STAGE
};

constexpr EFFECT_TOOL_SKILL_PHASE_ROLE Resolve_EffectToolSkillPhaseRole(
	const LostArk::Shared::PLAYER_SKILL_KIND eSkillKind,
	const size_t iStageIndex,
	const size_t iStageCount) noexcept
{
	if (LostArk::Shared::PLAYER_SKILL_KIND::COMBO == eSkillKind)
		return EFFECT_TOOL_SKILL_PHASE_ROLE::BASIC_ATTACK;
	if (LostArk::Shared::PLAYER_SKILL_KIND::HOLD == eSkillKind &&
		1u < iStageCount && iStageIndex < iStageCount)
	{
		if (0u == iStageIndex)
			return EFFECT_TOOL_SKILL_PHASE_ROLE::HOLD_START;
		if (iStageIndex + 1u == iStageCount)
			return EFFECT_TOOL_SKILL_PHASE_ROLE::HOLD_RELEASE;
		return EFFECT_TOOL_SKILL_PHASE_ROLE::HOLD_CHARGE;
	}
	return 1u < iStageCount ?
		EFFECT_TOOL_SKILL_PHASE_ROLE::STAGE :
		EFFECT_TOOL_SKILL_PHASE_ROLE::EFFECT;
}

constexpr std::string_view EffectToolSkillPhaseRoleLabel(
	const EFFECT_TOOL_SKILL_PHASE_ROLE eRole) noexcept
{
	switch (eRole)
	{
	case EFFECT_TOOL_SKILL_PHASE_ROLE::BASIC_ATTACK:
		return "BA";
	case EFFECT_TOOL_SKILL_PHASE_ROLE::HOLD_START:
		return "Hold Start";
	case EFFECT_TOOL_SKILL_PHASE_ROLE::HOLD_CHARGE:
		return "Hold Charge";
	case EFFECT_TOOL_SKILL_PHASE_ROLE::HOLD_RELEASE:
		return "Hold Release";
	case EFFECT_TOOL_SKILL_PHASE_ROLE::STAGE:
		return "Stage";
	case EFFECT_TOOL_SKILL_PHASE_ROLE::EFFECT:
	default:
		return "Effect";
	}
}

constexpr EFFECT_DOCUMENT_SOURCE Resolve_ProductCueDocumentSource(
	const bool_t bHasVisualProgramProjection) noexcept
{
	return bHasVisualProgramProjection ?
		EFFECT_DOCUMENT_SOURCE::RUNTIME_VISUAL_PROGRAM :
		EFFECT_DOCUMENT_SOURCE::AUTHORED;
}

class CEffect_Tool final
{
private:
    struct EFFECT_RESOURCE_CATALOG_ENTRY final
    {
        std::string strAssetId;
        std::string strDomainId;
        std::string strCategory;
        EFFECT_RESOURCE_FILE_KIND eFileKind =
            EFFECT_RESOURCE_FILE_KIND::END;
    };

    struct EFFECT_RESOURCE_DOMAIN_CATALOG final
    {
        std::string strDomainId;
        std::array<std::vector<std::string>,
            static_cast<size_t>(EFFECT_RESOURCE_FILE_KIND::END)> Categories;
        std::array<size_t,
            static_cast<size_t>(EFFECT_RESOURCE_FILE_KIND::END)> ResourceCounts{};
    };

    struct EFFECT_SKILL_TREE_ENTRY final
    {
        struct PRODUCT_CUE final
        {
            ANIMATION_EFFECT_CUE Cue;
            size_t iBoundClipOrdinal = 0u;
			size_t iStageIndex = 0u;
			size_t iStageClipIndex = 0u;
        };

        PLAYER_SKILL_DEFINITION Skill;
        std::vector<PRODUCT_CUE> ProductCues;
        size_t iSourceReferenceCount = 0u;
        size_t iImportedReferenceCount = 0u;
        size_t iEmptySourceReferenceCount = 0u;
    };

    struct EFFECT_PRODUCT_PREVIEW final
    {
        LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass =
            LostArk::Shared::CHARACTER_CLASS_ID::END;
        LostArk::Shared::SKILL_ID iSkillId =
            LostArk::Shared::INVALID_SKILL_ID;
        EFFECT_SKILL_TREE_ENTRY::PRODUCT_CUE ProductCue;
    };

	struct VALTAN_PRODUCT_PREVIEW final
	{
		VALTAN_CLIP_OCCURRENCE_VIEW Clip;
		VALTAN_PRODUCT_EFFECT_CUE_VIEW Cue;
	};

    struct EFFECT_DATA_FILE_ENTRY final
    {
        std::string strAssetId;
        std::string strDomainId;
        std::filesystem::path Path;
        EFFECT_DOCUMENT_SOURCE eSource = EFFECT_DOCUMENT_SOURCE::END;
    };

	struct DIRECT_AUTHORED_EDITABLE_ENTRY final
	{
		std::filesystem::path Path;
		std::filesystem::file_time_type LastWriteTime{};
		uint64_t iFileSize = 0u;
		bool_t bIdentityObserved = false;
		bool_t bIdentityValid = false;
		std::string strStatus;
	};

    struct PENDING_DOCUMENT_LOAD final
    {
		std::filesystem::path Path;
		std::string strSelectionId;
		EFFECT_DOCUMENT_SOURCE eSource = EFFECT_DOCUMENT_SOURCE::END;
		std::string strElementSelectionId;
		std::string strModelCueSelectionId;
		bool_t bPlayCompleteAfterLoad = false;
		optional<VALTAN_CLIP_OCCURRENCE_VIEW> ValtanClip;
		optional<VALTAN_PRODUCT_EFFECT_CUE_VIEW> ValtanCue;
		optional<std::vector<VALTAN_CLIP_OCCURRENCE_VIEW>>
			ValtanReferenceClips;
    };

	struct UNIFIED_EFFECT_CACHE final
	{
		std::filesystem::path Path;
		std::filesystem::file_time_type LastWriteTime{};
		uint64_t iFileSize = 0u;
		bool_t bObserved = false;
		bool_t bExists = false;
		bool_t bValid = false;
		bool_t bDrawable = false;
		bool_t bPreviewReady = false;
		EFFECT_DOCUMENT_DESC Document;
		std::string strDrawableError;
		std::string strPreviewReadinessError;
		std::string strStatus;
	};

	/* Player clips retain the existing preview-loop switch. Valtan v2 clips
	   additionally carry the canonical source segment and explicit loop bit. */
	struct SYNCHRONIZED_ANIMATION_CLIP final
	{
		std::string strClipName;
		uint32_t iPlayMs = 0u;
		f32_t fPlayRate = 1.f;
		uint32_t iSourceStartMs = 0u;
		bool_t bLoop = false;
		bool_t bHasExplicitLoopPolicy = false;

		SYNCHRONIZED_ANIMATION_CLIP() = default;
		SYNCHRONIZED_ANIMATION_CLIP(
			std::string ClipName, uint32_t iClipPlayMs, f32_t fClipPlayRate)
			: strClipName(std::move(ClipName)), iPlayMs(iClipPlayMs),
			  fPlayRate(fClipPlayRate)
		{
		}
		SYNCHRONIZED_ANIMATION_CLIP(const ANIMATION_SKILL_CLIP& Clip)
			: strClipName(Clip.strClipName), iPlayMs(Clip.iPlayMs),
			  fPlayRate(Clip.fPlayRate)
		{
		}
	};

	struct UNIFIED_EFFECT_CANDIDATE_BINDING final
	{
		LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass =
			LostArk::Shared::CHARACTER_CLASS_ID::END;
		LostArk::Shared::SKILL_ID iSkillId =
			LostArk::Shared::INVALID_SKILL_ID;
		size_t iStageIndex = 0u;
		size_t iStageClipIndex = 0u;
		std::string strClipName;
		std::string strEffectAssetId;
		std::string strLegacyProductEffectAssetId;
		std::filesystem::path Path;
	};

	enum class ARTIST_F_PREPARATION_STATE : uint8_t
	{
		UNATTEMPTED,
		READY,
		FAILED
	};

	struct SOURCE_ELEMENT_PRESET_SELECTION final
	{
		std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
			pProjection;
		EFFECT_ELEMENT_DESC GenericElement;
		std::string strSourceEffectAssetId;
		std::string strOccurrenceId;
		std::string strRowSha256;
		std::string strTargetElementId;
		std::string strSourceRecordId;
		std::string strSourceFamily;
	};

public:
    CEffect_Tool(
        ComPtr<ID3D11Device> pDevice,
        ComPtr<ID3D11DeviceContext> pContext,
        shared_ptr<CCharacterPreviewPanel> pCharacterPreviewPanel);
    ~CEffect_Tool();

    void Update(f32_t fTimeDelta);
    void Render();

private:
    void Render_EffectToolWindow();
    void Render_ModelViewWindow();
    void Render_EffectDetailWindow();
    void Render_AuthoringSessionBar();
    void Render_AllEffectsWindow();
	void Render_ActiveAuthoredEffectTree();
    void Render_LoadedEffectContents();
    bool_t Render_ManualElementGroups(
        const EFFECT_DOCUMENT_DESC& Document,
        const std::string& strEffectAssetId,
        bool_t bDefaultOpen);
    void Render_DataFilesWindow();
    void Render_PendingDocumentLoadModal();
    void Render_EffectTypeSelector();
    void Render_MeshAuthoringWorkbench();
    void Render_ParticleSystemDetail();
    void Render_ResourceSlots(bool_t bMeshAuthoringDraft);
    void Render_ResourceGrid(bool_t bMeshAuthoringDraft);
    void Rebuild_ResourceBrowserView(
        EFFECT_RESOURCE_FILE_KIND eFileKind,
        const std::string& strFilter,
        const std::string& strDomainId,
        const std::string& strCategory,
        const std::string& strKindCategory);
	/* All Effects treats one Valtan pattern like one Character skill: saved
	   unified Effects first, then the ordered semantic-stage animations. */
	void Render_ValtanPatternTreeSection(const std::string& strSearch);
	void Render_ValtanPatternNode(
		const VALTAN_PATTERN_VIEW& Pattern,
		const char_t* pGroupLabel,
		const std::string& strSearch);
	void Render_ValtanStageRow(
		const VALTAN_STAGE_VIEW& Stage);
	void Render_ValtanClipOccurrence(
		const VALTAN_STAGE_VIEW& Stage,
		const VALTAN_CLIP_OCCURRENCE_VIEW& Clip,
		size_t iClipOrdinal);
	void Render_ValtanProductCue(
		const VALTAN_CLIP_OCCURRENCE_VIEW& Clip,
		const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue,
		size_t iCueOrdinal);
	bool_t Matches_ValtanPatternSearch(
		const VALTAN_PATTERN_VIEW& Pattern,
		const std::string& strSearch) const;
	/* Product rows replay their exact cue occurrence. Reference and world-root
	   rows replay the complete ordered clip sequence of their owner stage. */
	bool_t Play_ValtanClipOccurrence(
		const VALTAN_CLIP_OCCURRENCE_VIEW& Clip);
	bool_t Play_ValtanProductCue(
		const VALTAN_CLIP_OCCURRENCE_VIEW& Clip,
		const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue);
	bool_t Play_ValtanStageSequence(
		const std::vector<VALTAN_CLIP_OCCURRENCE_VIEW>& Clips);
	bool_t Try_PlayValtanSavedUnifiedEffect(
		const std::filesystem::path& Path,
		const std::string& strEffectAssetId,
		const VALTAN_CLIP_OCCURRENCE_VIEW& Clip,
		const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue);
	bool_t Try_OpenValtanSavedReferenceEffect(
		const std::filesystem::path& Path,
		const std::string& strEffectAssetId,
		const std::vector<VALTAN_CLIP_OCCURRENCE_VIEW>& Clips,
		bool_t bQueuePlayCompleteAfterLoad = false);
	bool_t Try_OpenValtanAuthoredEffect(
		const std::filesystem::path& Path,
		const std::string& strEffectAssetId,
		const VALTAN_CLIP_OCCURRENCE_VIEW& Clip,
		const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue,
		bool_t bQueuePlayCompleteAfterLoad = false);
    void Render_Detail(EFFECT_ELEMENT_DESC& Element, bool_t& bChanged);
    void Render_TransformDetail(EFFECT_DETAIL_DESC& Detail, bool_t& bChanged);
    void Render_ColorDetail(
        EFFECT_DETAIL_DESC& Detail,
        bool_t& bChanged,
        bool_t bHasEmissiveTexture);
    void Render_UVDetail(EFFECT_DETAIL_DESC& Detail, bool_t& bChanged);
    void Render_UVKeyframes(EFFECT_ELEMENT_DESC& Element, bool_t& bChanged);
    void Render_TimingDetail(EFFECT_DETAIL_DESC& Detail, bool_t& bChanged);
	void Render_SizeDetail(EFFECT_ELEMENT_DESC& Element, bool_t& bChanged);
	void Render_AuthoringMaterialParameters(
		EFFECT_ELEMENT_DESC& Element,
		bool_t& bChanged);
    void Render_KindDetail(EFFECT_ELEMENT_DESC& Element, bool_t& bChanged);
    void Render_SourceRecipeDetail(
        EFFECT_CASCADE_RECIPE_DESC& Recipe,
        bool_t& bChanged,
		bool_t bPortableReadOnly);
	void Render_SelectedVisualProgramEvidence() const;
    void Render_SourceModuleDetail(
        EFFECT_SOURCE_MODULE_DESC& Module,
		bool_t& bChanged,
		bool_t bDefaultOpen = false);
    void Render_SourceDistributionDetail(
        EFFECT_DISTRIBUTION_DESC& Distribution,
		const std::string_view strModuleClassName,
        bool_t& bChanged);
	void Render_SelectionPath() const;
	void Render_SkillSelectionDetail();
	void Render_AssemblyHierarchy(const std::string& strEffectAssetId);
	void Render_VisualProgramAuthoring(
		const EFFECT_SKILL_TREE_ENTRY& Entry,
		size_t iCueIndex);
	void Render_ArtistFCoreAuthoring();
	void Render_ModelCueDetail();
	void Render_UnifiedEffectTree(
		const UNIFIED_EFFECT_CACHE& Cache,
		const std::string& strFallbackDisplayName,
		const VALTAN_CLIP_OCCURRENCE_VIEW* pValtanClip = nullptr,
		const VALTAN_PRODUCT_EFFECT_CUE_VIEW* pValtanCue = nullptr);
	void Render_ComponentSelectionDetail();
	void Render_EmitterSelectionDetail();
	void Render_SourceModuleSelectionDetail();
    void Render_LerpDetail(EFFECT_DETAIL_DESC& Detail, bool_t& bChanged);
    void Render_AnimationControls(
        const std::shared_ptr<Engine::CModel>& pModel);
    void Refresh_AnimationClipLabels(
        const std::shared_ptr<Engine::CModel>& pModel,
        bool_t bForce);

	bool_t Try_CreateDocument();
	bool_t Try_CreateElementDraft();
	bool_t Try_CreateMeshEffect(bool_t bAddToCurrentEffect);
	bool_t Try_UseSelectedElementAsAuthoringPreset();
    bool_t Try_BindMeshAuthoringResource(const std::string& strAssetId);
    bool_t Try_ClearMeshAuthoringSlot();
	bool_t Try_DuplicateSelectedElement();
    bool_t Try_DeleteSelectedElement();
    bool_t Try_ClearElements();
    bool_t Try_ApplyDraftAndSave();
    bool_t Try_SaveDocument();
    size_t Count_ProductCueMappings(
        const std::string& strEffectAssetId) const;
    bool_t Can_HotReloadSavedProduct() const;
    bool_t Try_HotReloadSavedProduct();
    bool_t Try_SaveDocumentAs(const std::string& strAssetId);
	bool_t Try_SaveSelectedAdapterElementAsGenericAuthoredCopy(
		const std::string& strAssetId);
    bool_t Try_PromoteImportedDocument();
    bool_t Try_ReloadActiveDocument();
    bool_t Try_LoadDocument(const std::string& strAssetId);
    bool_t Try_LoadDocumentPath(
        const std::filesystem::path& Path,
        EFFECT_DOCUMENT_SOURCE eSource,
        const std::string& strSelectionId);
    bool_t Try_LoadDocumentPathStaged(
        const std::filesystem::path& Path,
        EFFECT_DOCUMENT_SOURCE eSource,
        const std::string& strSelectionId,
        bool_t bBypassUnsavedGuard);
    bool_t Execute_PendingDocumentLoad(bool_t bSaveFirst);
    bool_t Refresh_AllEffects(bool_t bReloadSkillCatalog = false);
	bool_t Refresh_ValtanPatternTree();
    bool_t Refresh_DataFiles();
    bool_t Refresh_ResourceCatalog();
    void Select_AuthoringDomain(const std::string& strDomainId);
    bool_t Select_AuthoringDomainForClass(
        LostArk::Shared::CHARACTER_CLASS_ID eClass);
    bool_t Try_BindResource(const std::string& strAssetId);
	bool_t Try_ResetAuthoringResourceOverride(const std::string& strSlotId);
	bool_t Try_ClearAuthoringOverrides();
    bool_t Try_ClearSelectedSlot();
    bool_t Try_CommitDocument(EFFECT_DOCUMENT_DESC&& Staged);
    bool_t Try_SetPreviewFilter(EFFECT_PREVIEW_FILTER eFilter);
    bool_t Ensure_WorldPreviewObject();
	bool_t Ensure_ExactCookedCanaryVariantsInstalled(std::string& strOutError);
	bool_t Has_ExactCookedCanaryMaterial(
		const EFFECT_DOCUMENT_DESC& Document) const;
	bool_t Try_SetExactCookedCanaryEnabled(bool_t bEnabled);
	void Reset_ExactCookedCanarySelection(std::string strReason);
	void Invalidate_ExactCookedCanaryInstallation(std::string strReason);
	bool_t Has_Glasshole02TranslatedCanaryOccurrence(
		const EFFECT_DOCUMENT_DESC& Document) const;
	bool_t Try_SetGlasshole02TranslatedCanaryEnabled(bool_t bEnabled);
	void Reset_Glasshole02TranslatedCanarySelection(std::string strReason);
	bool_t Has_ValtanTranslatedCanaryOccurrences(
		const EFFECT_DOCUMENT_DESC& Document) const;
	bool_t Try_SetValtanTranslatedCanaryEnabled(bool_t bEnabled);
	void Reset_ValtanTranslatedCanarySelection(std::string strReason);
	bool_t Try_StartArtist31470FullPreview();
	bool_t Try_ResetArtist31470PreviewIsolation();
	bool_t Try_SetArtist31470PreviewFamilyIsolation(
		EFFECT_GPU_RENDER_FAMILY eFamily);
	bool_t Ensure_ArtistFSourceSnapshotForAuthoring();
	bool_t Ensure_ArtistFMaterialExecutionSnapshots();
	void Reset_ArtistFPreparationFailureLatch();
	bool_t Ensure_ArtistFSourceAuthoringOverlaySession();
	bool_t Refresh_UnifiedEffectCache(
		UNIFIED_EFFECT_CACHE& Cache,
		const std::filesystem::path& Path,
		const std::string& strExpectedEffectAssetId);
	bool_t Refresh_DirectAuthoredEditableIndex(
		const std::vector<EFFECT_DATA_FILE_ENTRY>& DataFiles);
	const std::filesystem::path* Resolve_DirectAuthoredEditablePath(
		const std::string& strEffectAssetId,
		std::string& strOutStatus);
	bool_t Is_UnifiedEffectActive(
		const UNIFIED_EFFECT_CACHE& Cache) const;
	bool_t Validate_UnifiedEffectPreviewReadiness(
		const EFFECT_DOCUMENT_DESC& Document,
		std::string& strOutError) const;
	bool_t Try_LoadUnifiedElement(
		const UNIFIED_EFFECT_CACHE& Cache,
		const std::string& strElementId);
	bool_t Try_LoadUnifiedModelCue(
		const UNIFIED_EFFECT_CACHE& Cache,
		const std::string& strCueId);
	bool_t Try_PlayUnifiedAuthoringFamily(
		const std::string& strEffectAssetId,
		EFFECT_AUTHORING_FAMILY eFamily);
	bool_t Try_PlayUnifiedEffect(const UNIFIED_EFFECT_CACHE& Cache);
	bool_t Try_PlayActiveUnifiedEffect();
	bool_t Try_PlaySavedUnifiedEffect(
		const UNIFIED_EFFECT_CANDIDATE_BINDING& Binding);
	bool_t Try_PlayUnifiedModelCues(
		const std::string& strEffectAssetId);
	bool_t Try_CreateArtistFUnifiedDraft();
	bool_t Try_CreateDimensionMasterTUnifiedDraft();
	bool_t Try_ApplyArtistFTrackASeedData(
		const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter,
		const EFFECT_ELEMENT_DESC& SourceElement,
		EFFECT_ELEMENT_DESC& InOutElement,
		std::string& strOutError) const;
	bool_t Try_SelectRuntimeOccurrence(
		const std::string& strEffectAssetId,
		const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter);
	std::shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		Resolve_VisualProgramProjectionForAuthoring(
			const std::string& strEffectAssetId);
	bool_t Try_OpenVisualProgramElementForAuthoring(
		const std::string& strEffectAssetId,
		const std::string& strOccurrenceId,
		const std::string& strRowSha256,
		const std::string& strTargetElementId,
		const std::string& strSourceRecordId);
	bool_t Try_OpenArtistFReconstructedElementForAuthoring(
		const EFFECT_RUNTIME_PROGRAM_EMITTER& Emitter);
	bool_t Try_StageElementAsAuthoringPreset(
		const EFFECT_DOCUMENT_DESC& SourceDocument,
		const std::string& strElementId,
		SOURCE_ELEMENT_PRESET_SELECTION Selection);
	bool_t Try_SelectVisualOccurrence(
		const std::string& strEffectAssetId,
		const std::string& strOccurrenceId,
		const std::string& strRowSha256,
		const std::string& strTargetElementId,
		const std::string& strSourceRecordId);
	bool_t Try_SetVisualPreviewOccurrenceIsolation(
		const std::string& strTargetElementId);
	bool_t Try_SetVisualPreviewFamilyIsolation(
		const EFFECT_VISUAL_PROGRAM& Program,
		EFFECT_VISUAL_PROGRAM_FAMILY eFamily);
	bool_t Try_ResetVisualPreviewIsolation();
	void Render_RuntimeOccurrenceDetail();
	bool_t Stage_RuntimeOccurrenceTuningPreview(
		const EFFECT_OCCURRENCE_TUNING_DOCUMENT& Tuning);
	bool_t Stage_SourceAuthoringOverlayPreview(
		const EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT& Overlay);
	bool_t Try_ApplyRuntimeOccurrenceDraft();
	bool_t Try_ResetRuntimeOccurrenceToSource();
	bool_t Try_SaveRuntimeOccurrenceTuning();
	bool_t Try_ReloadRuntimeOccurrenceTuning();
	void Reset_RuntimeOccurrenceTuningSession();
	bool_t Synchronize_Artist31470FullPreview(
		const std::shared_ptr<const
			EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>& pPreparation);
	void Update_ReconstructedDiagnosticRoot();
	bool_t Prepare_Artist31470HistoricalPoseBinding(
		const std::shared_ptr<const
			EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>& pPreparation,
		CAnimationHistoricalPoseBinding& OutPoseBinding,
		f32_t& fOutDurationSeconds,
		std::string& strOutError) const;
	bool_t Build_Artist31470HistoricalTransformSample(
		const std::shared_ptr<const
			EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>& pPreparation,
		const CAnimationHistoricalPoseBinding& PoseBinding,
		f32_t fAnimationDurationSeconds,
		f32_t fEffectSampleTimeSeconds,
		EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& OutSample,
		std::string& strOutError) const;
	bool_t Prepare_ReconstructedSourceRuntimeTransformHistory();
	bool_t Build_ReconstructedSourceRuntimeTransformSample(
		f32_t fEffectSampleTimeSeconds,
		EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& OutSample,
		std::string& strOutError) const;
	bool_t Prepare_ValtanBossPatternTransformHistory(
		const BOSS_PATTERN_EFFECT_BINDING& Binding,
		const EFFECT_DOCUMENT_DESC& Document,
		std::string& strOutError);
	bool_t Build_ValtanBossPatternTransformSample(
		f32_t fEffectSampleTimeSeconds,
		EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& OutSample,
		std::string& strOutError) const;
	void Reset_ValtanBossPatternTransformHistory();
	bool_t Update_ReconstructedSourceRuntimeTimeline(f32_t fTimeDelta);
	bool_t Seek_ReconstructedSourceRuntimeTimeline(f32_t fSampleTimeSeconds);
	void Reset_ReconstructedSourceRuntimeTimeline();
    bool_t Stage_WorldPreview();
    bool_t Stage_WorldPreview(const EFFECT_DOCUMENT_DESC& Document);
	bool_t Stage_WorldPreview(const EFFECT_DOCUMENT_DESC& Document,
		bool_t bAllowReadOnlySourceProjection);
    EFFECT_DOCUMENT_DESC Build_PreviewDocument(
		const EFFECT_DOCUMENT_DESC& Document,
		bool_t bAllowValtanTranslatedCanaryProjection = false) const;
    bool_t Try_SelectProductCue(
        const EFFECT_SKILL_TREE_ENTRY& Entry,
        size_t iCueIndex);
	bool_t Try_PlayVisualProgramFamily(
		const EFFECT_SKILL_TREE_ENTRY& Entry,
		size_t iCueIndex,
		EFFECT_VISUAL_PROGRAM_FAMILY eFamily);
	bool_t Try_PlayVisualProgramElement(
		const EFFECT_SKILL_TREE_ENTRY& Entry,
		size_t iCueIndex,
		const std::string& strTargetElementId);
    bool_t Try_SelectParticleSystem(const std::string& strEffectAssetId);
    bool_t Try_SelectElement(
        const std::string& strEffectAssetId,
        const std::string& strElementId);
	bool_t Try_SelectModelCue(
		const std::string& strEffectAssetId,
		const std::string& strCueId);
	bool_t Try_SoloElement(
		const std::string& strEffectAssetId,
		const std::string& strElementId);
	bool_t Try_SoloElementGroup(
		const std::string& strEffectAssetId,
		const std::string& strGroupId);
	bool_t Try_SoloModelCue(
		const std::string& strEffectAssetId,
		const std::string& strCueId);
	bool_t Try_SelectComponent(
		const std::string& strEffectAssetId,
		const std::string& strComponentAssetId);
	bool_t Try_SelectEmitter(
		const std::string& strEffectAssetId,
		const std::string& strComponentAssetId,
		const std::string& strEmitterId);
	bool_t Try_SelectSourceModule(
		const std::string& strEffectAssetId,
		const std::string& strComponentAssetId,
		const std::string& strEmitterId,
		const std::string& strModuleStableId);
	bool_t Try_SelectFirstEmitter(
		const std::string& strEffectAssetId,
		const std::string& strComponentAssetId);
    bool_t Try_AuditionParticleSystem();
    bool_t Try_AuditionSelectedElement();
    bool_t Stage_ParticleSystemDraftPreview();
    bool_t Stage_DetailDraftPreview();
	bool_t Stage_ModelCueDraftPreview();
    bool_t Apply_ParticleSystemDraft(EFFECT_DOCUMENT_DESC& Document) const;
    bool_t Apply_DetailDraft(EFFECT_DOCUMENT_DESC& Document) const;
	bool_t Apply_ModelCueDraft(EFFECT_DOCUMENT_DESC& Document) const;
    bool_t Resolve_PreviewRoot(float4x4_t& OutRoot);
	bool_t Has_ProductCuePreview() const;
    f32_t Resolve_EffectSampleTime(f32_t fTimelineSeconds) const;
	f32_t Resolve_EffectTimelineTime(f32_t fEffectSampleSeconds) const;
    bool_t Is_ProductCueVisible(f32_t fTimelineSeconds) const;
	bool_t Restore_ValtanProductPreviewPlayback(
		const optional<VALTAN_PRODUCT_PREVIEW>& Preview,
		f32_t fTimelineSeconds,
		f32_t fDurationSeconds,
		bool_t bPlaying,
		bool_t bVisibleRequested,
		const float4x4_t& SnapshotRoot,
		bool_t bSnapshotCaptured,
		std::string& strOutError);
    void Clear_ProductCuePreview();
    void Reset_ProductCueSnapshot();
    void Start_WorldPreviewFromBeginning();
    void Synchronize_LoadedSkillPreview();
	void Select_PlayerPreviewCueCandidate(size_t iCandidateIndex);
    void Restart_SynchronizedAnimationSequence();
	bool_t Start_SynchronizedAnimationClip(size_t iClipIndex, bool_t bPaused);
    void Seek_SynchronizedAnimationSequence(f32_t fTimeSeconds);
    void Set_SynchronizedAnimationPaused(bool_t bPaused);
    bool_t Try_ResolveSynchronizedAnimationTime(f32_t& fOutTimeSeconds) const;
    void Update_SynchronizedAnimationSequence();
    void Reset_SynchronizedAnimationSequence();
    void Hide_WorldPreview();
    void Release_WorldPreview(bool_t bRemoveFromLayer);
    void Update_Picking();
    void Discard_ActiveDocument();
    void Reset_MeshAuthoringDraft();
    void Reset_ParticleSystemDraft();
    void Reset_DetailDraft();
	void Refresh_DetailDraftAdmission(const EFFECT_ELEMENT_DESC& Element);
	void Reset_ModelCueDraft();
    void Recalculate_PreviewDuration();
    void Recalculate_PreviewDuration(const EFFECT_DOCUMENT_DESC& Document);
    bool_t Has_UnsavedWork() const;
    bool_t Has_UnappliedDetailDraft() const;
    void Set_ActiveDocumentDrawableStatus(
        bool_t bDrawable,
        std::string strError);
    void Clear_ActiveDocumentDrawableStatus();
    void Refresh_RuntimeEquivalence();
    std::string Describe_ProductPlaybackAuthoredDivergence(
        const std::string& strProductEffectAssetId);
    const char_t* Runtime_SyncLabel() const;
    EFFECT_ELEMENT_DESC* Find_SelectedElement();
    const EFFECT_ELEMENT_DESC* Find_SelectedElement() const;
	EFFECT_MODEL_CUE_DESC* Find_SelectedModelCue();
	const EFFECT_MODEL_CUE_DESC* Find_SelectedModelCue() const;

private:
    ComPtr<ID3D11Device> m_pDevice;
    ComPtr<ID3D11DeviceContext> m_pContext;
    unique_ptr<CEffectThumbnailCache> m_pThumbnailCache;
    shared_ptr<CCharacterPreviewPanel> m_pCharacterPreviewPanel;
    weak_ptr<CEffectObject> m_pWorldPreviewObject;
    uint32_t m_iWorldPreviewLevel = UINT32_MAX;

    optional<EFFECT_DOCUMENT_DESC> m_ActiveDocument;
	optional<EFFECT_DOCUMENT_DESC> m_SourcePreviewDocument;
    optional<EFFECT_PRODUCT_PREVIEW> m_ProductPreview;
	optional<VALTAN_PRODUCT_PREVIEW> m_ValtanProductPreview;
    EFFECT_ELEMENT_DESC m_MeshAuthoringDraft;
	optional<SOURCE_ELEMENT_PRESET_SELECTION> m_SourceElementPresetSelection;
    optional<EFFECT_PARTICLE_SYSTEM_DESC> m_ParticleSystemDraft;
    optional<EFFECT_ELEMENT_DESC> m_DetailDraft;
	optional<EFFECT_MODEL_CUE_DESC> m_ModelCueDraft;
	optional<EFFECT_OCCURRENCE_TUNING_DOCUMENT> m_OccurrenceTuningDocument;
	optional<EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT>
		m_SourceAuthoringOverlayDocument;
	optional<EFFECT_OCCURRENCE_LOCAL_TRANSFORM> m_OccurrenceTransformDraft;
	std::vector<UNIFIED_EFFECT_CANDIDATE_BINDING>
		m_UnifiedCandidateBindings;
	std::unordered_map<std::string, UNIFIED_EFFECT_CACHE>
		m_UnifiedCandidateCaches;
	std::unordered_map<std::string, UNIFIED_EFFECT_CACHE>
		m_ValtanUnifiedEffectCaches;
	std::unordered_map<std::string, DIRECT_AUTHORED_EDITABLE_ENTRY>
		m_DirectAuthoredEditableEntries;
	std::unordered_map<std::string, size_t>
		m_BossProductCueMappingCounts;
	shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		m_pSelectedVisualSourceProjection;
	shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		m_pVisualPreviewProjection;
	shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
		m_pArtistFSourcePreparation;
	shared_ptr<const EFFECT_VISUAL_PROGRAM_DOCUMENT_PROJECTION>
		m_pArtistFSourceProjection;
	std::unordered_map<std::string, EFFECT_MATERIAL_EXECUTION_DESC>
		m_ArtistFMaterialExecutionSnapshots;
	ARTIST_F_PREPARATION_STATE m_eArtistFSourcePreparationState =
		ARTIST_F_PREPARATION_STATE::UNATTEMPTED;
	ARTIST_F_PREPARATION_STATE m_eArtistFMaterialPreparationState =
		ARTIST_F_PREPARATION_STATE::UNATTEMPTED;
	optional<PENDING_DOCUMENT_LOAD> m_PendingDocumentLoad;
    vector<EFFECT_RESOURCE_CATALOG_ENTRY> m_ResourceCatalog;
    vector<EFFECT_RESOURCE_DOMAIN_CATALOG> m_ResourceDomains;
    vector<size_t> m_VisibleResourceIndices;
    vector<EFFECT_SKILL_TREE_ENTRY> m_AllEffects;
	/* Session state, rebuilt by Refresh. A failed reload keeps the previous
	   tree so the window never empties on a transient read error. */
	VALTAN_PATTERN_TREE_VIEW m_ValtanPatternTree;
	std::string m_strValtanPatternTreeStatus;
	bool_t m_bValtanPatternTreeLoaded = false;
    vector<EFFECT_DATA_FILE_ENTRY> m_DataFiles;
    vector<string> m_DataFileDomains;
    vector<SYNCHRONIZED_ANIMATION_CLIP> m_SynchronizedAnimationClips;
	vector<ANIMATION_EFFECT_PREVIEW_CANDIDATE>
		m_PlayerPreviewCueCandidates;
    vector<string> m_AnimationClipDisplayLabels;
    vector<string> m_AnimationClipSearchTokens;
    EFFECT_ELEMENT_KIND m_eSelectedEffectType = EFFECT_ELEMENT_KIND::MESH;
	EFFECT_AUTHORING_FAMILY m_eSelectedAuthoringFamily =
		EFFECT_AUTHORING_FAMILY::MESH;
    EFFECT_DETAIL_SELECTION m_eDetailSelection =
        EFFECT_DETAIL_SELECTION::NONE;
    LostArk::Shared::CHARACTER_CLASS_ID m_eAllEffectsClass =
        LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER;
	bool_t m_bAllEffectsValtanBossSelected = false;
    EFFECT_PREVIEW_FILTER m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
    EFFECT_DOCUMENT_SOURCE m_eActiveDocumentSource =
        EFFECT_DOCUMENT_SOURCE::NEW_DOCUMENT;
    EFFECT_PREVIEW_PIVOT_KIND m_ePreviewPivotKind =
        EFFECT_PREVIEW_PIVOT_KIND::PLAYER_ROOT;
    std::filesystem::path m_ActiveDocumentPath;
	string m_strActiveDocumentBaselineCanonical;
    string m_strSelectedResourceSlotId = "meshModel";
    string m_strSelectedElementId;
	/* Ctrl/Shift-clicking Element rows marks them for one bulk delete. Empty
	   means the single m_strSelectedElementId is the delete target, which is
	   the behaviour every other command still assumes. */
	std::set<string, std::less<>> m_MarkedElementIds;
	std::set<string, std::less<>> m_ExactCookedCanarySourceMaterials;
    string m_strSelectedElementGroupId;
	string m_strSelectedModelCueId;
	string m_strPreviewIsolationElementId;
	string m_strPreviewIsolationGroupId;
	string m_strPreviewIsolationModelCueId;
	EFFECT_AUTHORING_FAMILY m_ePreviewIsolationAuthoringFamily =
		EFFECT_AUTHORING_FAMILY::END;
	string m_strSelectedComponentId;
	string m_strSelectedEmitterId;
	string m_strSelectedSourceModuleId;
	string m_strSelectedRuntimeOccurrenceEffectId;
	string m_strSelectedRuntimeOccurrenceId;
	string m_strSelectedRuntimeOccurrenceRowSha256;
	string m_strSelectedRuntimeOccurrenceElementId;
	string m_strSelectedRuntimeOccurrenceEmitterPath;
	string m_strArtistFSourceSnapshotStatus;
	string m_strOccurrenceTuningBaselineCanonical;
	std::filesystem::path m_OccurrenceTuningPath;
	string m_strSourceAuthoringOverlayBaselineCanonical;
	std::filesystem::path m_SourceAuthoringOverlayPath;
	EFFECT_OCCURRENCE_LOCAL_TRANSFORM m_SelectedOccurrenceSourceTransform;
    string m_strSelectedResourceAssetId;
    string m_strSelectedDataFileAssetId;
    string m_strSelectedAuthoringDomainId = "DimensionMaster";
    string m_strPreviewAnchorSlotId = "root";
	string m_strDetailDraftElementId;
	string m_strDetailDraftCapabilityReason;
	string m_strUnifiedCandidateStatus;
    string m_strResourceViewFilter;
    string m_strResourceViewDomainId;
    string m_strResourceViewCategory;
    string m_strResourceViewKindCategory;
    string m_strMeshShapeCategory = "All";
    string m_strTextureKindCategory = "All";

    array<char_t, 129> m_NewAssetId{};
    array<char_t, 65> m_NewDisplayName{};
    array<char_t, 129> m_NewElementId{};
    array<char_t, 129> m_ResourceFilter{};
    array<char_t, 129> m_ResourceCategory{};
    array<char_t, 129> m_AllEffectsSearch{};
    array<char_t, 129> m_DataFilesSearch{};
    array<char_t, 129> m_AnimationClipFilter{};
    array<char_t, 129> m_PreviewAnchorBuffer{};
	array<char_t, 257> m_ModelCueAssetIdDraft{};
	array<char_t, 129> m_ModelCueClipNameDraft{};

    float4x4_t m_PreviewWorldRoot{};
    float4x4_t m_ProductCueSnapshotRoot{};
    EFFECT_TRANSFORM_DESC m_CueTransferLocalTransform{};
    EFFECT_FOLLOW_POLICY m_eCueTransferFollowPolicy =
        EFFECT_FOLLOW_POLICY::FOLLOW;
	EFFECT_ORIENTATION_POLICY m_eCueTransferOrientationPolicy =
		EFFECT_ORIENTATION_POLICY::ANCHOR;
    EFFECT_STOP_POLICY m_eCueTransferStopPolicy =
        EFFECT_STOP_POLICY::NATURAL;
    float2_t m_vMouseViewportPosition{};
    float3_t m_vPickedWorldPosition{};
    f32_t m_fPreviewTimeSeconds = 0.f;
    f32_t m_fPreviewDurationSeconds = 1.f;
	double m_fDetailDraftPreviewDueSeconds = 0.0;
    bool_t m_bPreviewPlaying = false;
    bool_t m_bPreviewLoop = true;
	bool_t m_bPreviewVisibleRequested = false;
	bool_t m_bPreviewScreenPostEnabled = true;
	bool_t m_bReconstructedDiagnosticActive = false;
	bool_t m_bReconstructedSourceRuntimeActive = false;
	bool_t m_bReconstructedSourceRuntimeStartPending = false;
	bool_t m_bReconstructedSourceRuntimeNaturalTailActive = false;
	CAnimationHistoricalPoseBinding m_ReconstructedSourceRuntimePoseBinding;
	CAnimationHistoricalPoseBinding m_ValtanBossPatternPoseBinding;
	EFFECT_TRANSFORM_DESC m_ValtanBossPatternSocketLocalTransform{};
	std::string m_strValtanBossPatternPreviewEffectAssetId;
	std::string m_strValtanBossPatternAnchorSlotId;
	std::string m_strValtanBossPatternBoneName;
	f32_t m_fValtanBossPatternAnimationDurationSeconds = 0.f;
	bool_t m_bValtanBossPatternTransformHistoryRequired = false;
	bool_t m_bValtanBossPatternTransformHistoryActive = false;
	bool_t m_bDocumentDirty = false;
	bool_t m_bExactCookedCanaryEnabled = false;
	bool_t m_bExactCookedCanaryVariantsInstalled = false;
	bool_t m_bGlasshole02TranslatedCanaryEnabled = false;
	bool_t m_bValtanTranslatedCanaryEnabled = false;
	bool_t m_bActiveDocumentDrawable = false;
    bool_t m_bActiveDocumentMatchesRuntime = false;
    bool_t m_bResourceCatalogRefreshAttempted = false;
    bool_t m_bAllEffectsRefreshAttempted = false;
    bool_t m_bDataFilesRefreshAttempted = false;
    bool_t m_bPendingWorldPivotPick = false;
    bool_t m_bParticleSystemDraftDirty = false;
    bool_t m_bMeshAuthoringDraftInitialized = false;
    bool_t m_bDetailDraftDirty = false;
	bool_t m_bDetailDraftPortableRecipeReadOnly = false;
	bool_t m_bDetailDraftCapabilityDeferred = false;
	bool_t m_bDetailDraftPreviewPending = false;
	bool_t m_bModelCueDraftDirty = false;
	bool_t m_bOccurrenceTuningDirty = false;
	bool_t m_bOccurrenceTransformDraftDirty = false;
	bool_t m_bSourceAuthoringOverlayNeedsInitialSave = false;
    bool_t m_bDiscardConfirmationRequested = false;
    bool_t m_bPromoteConfirmationRequested = false;
    bool_t m_bPendingDocumentLoadModalRequested = false;
    bool_t m_bProductCueSnapshotCaptured = false;
	f32_t m_fProductCueActionFacingYawDegrees = 0.f;
	bool_t m_bProductCueActionFacingCaptured = false;
    uint64_t m_iFrameNumber = 0u;
    uint64_t m_iSynchronizedAnimationTargetGeneration = 0u;
    uint64_t m_iAnimationClipLabelTargetGeneration = 0u;
    uint64_t m_iResourceCatalogRevision = 0u;
	uint64_t m_iResourceViewRevision = UINT64_MAX;
	uint64_t m_iArtistFSourceSnapshotRevision = UINT64_MAX;
	uint64_t m_iArtistFMaterialExecutionSnapshotRevision = UINT64_MAX;
	uint64_t m_iArtistFSourcePreparationAttemptRevision = UINT64_MAX;
	uint64_t m_iArtistFMaterialPreparationAttemptRevision = UINT64_MAX;
    EFFECT_RESOURCE_FILE_KIND m_eResourceViewFileKind =
        EFFECT_RESOURCE_FILE_KIND::END;
    EFFECT_RESOURCE_FILE_KIND m_eResourceLibraryFileKind =
        EFFECT_RESOURCE_FILE_KIND::MODEL;
	uint32_t m_iCueTransferDurationMs = 250u;
	size_t m_iSynchronizedAnimationClipIndex = 0u;
	uint64_t m_iSynchronizedAnimationLoopEpoch = 0u;
	size_t m_iPlayerPreviewCueCandidateIndex = 0u;
	f32_t m_fReconstructedSourceRuntimeClockSeconds = 0.f;
	f32_t m_fReconstructedSourceRuntimeTailSeconds = 0.f;
	string m_strDocumentStatus;
	string m_strSaveHotReloadStatus;
	string m_strDirectAuthoredEditableStatus;
	string m_strActiveDocumentDrawableError;
    shared_ptr<const EFFECT_DOCUMENT_DESC> m_pRuntimeEquivalenceDocument;
    string m_strRuntimeEquivalenceCanonical;
    string m_strElementStatus;
    string m_strDetailStatus;
    string m_strResourceStatus;
    string m_strPreviewStatus;
	string m_strExactCookedCanaryStatus =
		"OFF: family-lite authoring preview remains active.";
	string m_strGlasshole02TranslatedCanaryStatus =
		"OFF: translated Glasshole02 Tool canary is not staged.";
	string m_strValtanTranslatedCanaryStatus =
		"OFF: translated Valtan core-three Tool canary is not staged.";
    string m_strPreviewAnimationStatus;
    string m_strAnimationClipLabelStatus;
};

NS_END
