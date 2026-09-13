#pragma once

#include "Effect_Tool.h"
#include "ProjectDataRoot.h"

namespace Client
{
struct CHARACTER_SPEC;
struct EFFECT_VISUAL_PROGRAM_RESOURCE_PACKET_ROW;
enum class EFFECT_RUNTIME_RENDERER_KIND : uint8_t;
}

// Private implementation contracts shared by the separately compiled Effect units.

namespace EffectToolDetail
{

    constexpr const wchar_t* PREVIEW_LAYER = L"Layer_EffectPreview";
	constexpr uint32_t ARTIST_F_CORE_SKILL_ID = 31470u;
	constexpr const char_t* ARTIST_F_VISUAL_PROGRAM_ASSET_ID =
		"effect.artist.skill.31470";
	constexpr const char_t* ARTIST_F_UNIFIED_EFFECT_ASSET_ID =
		"effect.artist.skill.31470.unified";
	constexpr const char_t* ARTIST_F_SOURCE_AUTHORING_OVERLAY_PATH =
		"Effects/AuthoredCorrections/Artist/"
		"effect.artist.skill.31470.source-authoring-overlay.json";
	constexpr uint32_t DIMENSION_MASTER_T_SKILL_ID = 2050500u;
	constexpr const char_t* DIMENSION_MASTER_T_UNIFIED_EFFECT_ASSET_ID =
		"effect.dimensionmaster.skill.2050500.unified";
	constexpr const char_t* VALTAN_EXACT_HISTORY_BINDING_ID =
		"valtan.whirlwind.420633.active";
	constexpr const char_t* VALTAN_EXACT_HISTORY_EFFECT_ASSET_ID =
		"effect.valtan.pattern.420633.active";
	constexpr const char_t* VALTAN_EXACT_HISTORY_V1_EFFECT_ASSET_ID =
		"effect.valtan.pattern.420633.active.v1.unified";
	constexpr const char_t* VALTAN_STANDALONE_STATIC_CLIP =
		"mesh_idle_battle_1";
	constexpr double VALTAN_PATTERN_TREE_RELOAD_RETRY_SECONDS = 0.25;
	constexpr const char_t* VALTAN_ARENA_BOSS_PLACEMENT_ID =
		"boss.valtan.center";
	constexpr const char_t* VALTAN_AREA_ID = "LV_LUT_HEARTRB_ED";
	constexpr const char_t* VALTAN_AREA_MAP_EFFECT_SOURCE =
		"Maps/Authoring/LV_LUT_HEARTRB_ED/"
		"LV_LUT_HEARTRB_ED.mapeffects.json";
	bool Is_ValtanExactHistoryPreviewEffectAssetId(
		const std::string_view strEffectAssetId);


	bool Matches_ValtanExactHistoryBinding(
		const std::string_view strBindingId,
		const std::string_view strBindingEffectAssetId,
		const std::string_view strPreviewEffectAssetId);


	const char_t* Resolve_ValtanServerPatternBossPlacement(
		const uint32_t iLevel);


	void Select_SharedCompletePlayPattern(
		const std::string& strPatternId);


	bool Remove_EffectDocumentIfCanonical(
		const std::filesystem::path& Path,
		const std::string_view strExpectedCanonical,
		std::string& strOutStatus);


	constexpr uint64_t MAX_EFFECT_TOOL_TRANSACTION_BYTES =
		16u * 1024u * 1024u;

	class CAuthoritativeProductSourceReadLocks final
	{
	public:
		~CAuthoritativeProductSourceReadLocks()
		{
			for (const HANDLE hSource : m_SourceHandles)
			{
				if (INVALID_HANDLE_VALUE != hSource)
					CloseHandle(hSource);
			}
		}

		bool_t Try_Acquire(std::string& strOutStatus)
		{
			struct SOURCE_LOCK_DESC final
			{
				std::filesystem::path RelativePath;
				std::string_view strLabel;
			};
			const std::array<SOURCE_LOCK_DESC, 11u> Sources{
				SOURCE_LOCK_DESC{ L"Effects/EffectCatalog.json", "EffectCatalog" },
				SOURCE_LOCK_DESC{ L"Valtan/Valtan.gameplay.json", "Valtan gameplay" },
				SOURCE_LOCK_DESC{ L"Valtan/Valtan.presentation.json", "Valtan presentation" },
				SOURCE_LOCK_DESC{ L"Encounters/Valtan/ValtanEncounter.json", "Valtan Encounter" },
				SOURCE_LOCK_DESC{ L"Encounters/Valtan/ValtanPatternRotations.json", "Valtan rotations" },
				SOURCE_LOCK_DESC{ L"Animation/Authored/Valtan/Valtan.patternbindings.json", "Valtan animation bindings" },
				SOURCE_LOCK_DESC{ L"Animation/Authored/Valtan/Valtan.patterneffectcues.json", "Valtan Product Effect cues" },
				SOURCE_LOCK_DESC{ L"Animation/Authored/Valtan/Valtan.patterneffectv1aliases.json", "Valtan Product Effect aliases" },
				SOURCE_LOCK_DESC{ L"Animation/Authored/Valtan/Valtan.patterneffects.json", "Valtan stage Effect bindings" },
				SOURCE_LOCK_DESC{ L"Actors/BossCatalog.json", "BossCatalog" },
				SOURCE_LOCK_DESC{ L"Encounters/Valtan/ValtanCombatObjects.json", "Valtan combat objects" }
			};
			m_SourceHandles.reserve(Sources.size());
			for (const SOURCE_LOCK_DESC& Source : Sources)
			{
				HANDLE hSource = INVALID_HANDLE_VALUE;
				if (!Try_AcquireOne(
						Client::CProjectDataRoot::Resolve(Source.RelativePath),
						Source.strLabel, hSource, strOutStatus))
				{
					return false;
				}
				m_SourceHandles.push_back(hSource);
			}
			strOutStatus =
				"Locked the complete authoritative Product read set against concurrent write/delete.";
			return true;
		}

	private:
		static bool_t Try_AcquireOne(
			const std::filesystem::path& Path,
			const std::string_view strLabel,
			HANDLE& hOutFile,
			std::string& strOutStatus)
		{
			hOutFile = INVALID_HANDLE_VALUE;
			if (Path.empty())
			{
				strOutStatus = std::string(strLabel) +
					" source path escaped project Data.";
				return false;
			}
			hOutFile = CreateFileW(Path.c_str(), GENERIC_READ,
				FILE_SHARE_READ, nullptr, OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
			if (INVALID_HANDLE_VALUE == hOutFile)
			{
				strOutStatus = std::string(strLabel) +
					" is being changed or could not be locked; no Draft file was changed.";
				return false;
			}
			return true;
		}

	private:
		std::vector<HANDLE> m_SourceHandles;
	};

	size_t Count_ValtanProductReferences(
		const Client::VALTAN_PATTERN_TREE_VIEW& Tree,
		const std::string& strEffectAssetId);


	bool_t Try_LockAndInspectAuthoritativeProductOwnership(
		const std::string& strEffectAssetId,
		CAuthoritativeProductSourceReadLocks& SourceLocks,
		bool_t& bOutProductOwned,
		std::string& strOutStatus);


	bool_t Read_TransactionRawBytes(
		const std::filesystem::path& Path,
		std::string& OutBytes,
		std::string& strOutStatus);


	std::filesystem::path Make_TransactionTemporaryPath(
		const std::filesystem::path& Destination,
		const wchar_t* pRole);


	bool_t Write_TransactionRawBytesAtomicIfUnchanged(
		const std::filesystem::path& Path,
		const std::string_view Replacement,
		const std::string_view ExpectedCurrent,
		std::string& strOutStatus);


	bool_t Create_TransactionRawBytesDurableIfAbsent(
		const std::filesystem::path& Path,
		const std::string_view Contents,
		std::string& strOutStatus);


	bool_t Remove_TransactionFileIfExactRaw(
		const std::filesystem::path& Path,
		const std::string_view ExpectedRaw,
		std::string& strOutStatus);


	bool_t Build_EffectCatalogWithDirectAuthoredRow(
		const std::string& strEffectAssetId,
		std::string& OutPreviousRaw,
		std::string& OutCandidateRaw,
		std::filesystem::path& OutCatalogPath,
		std::string& strOutStatus);


	const char_t* Tool_PlayerStanceLabel(
		const LostArk::Shared::PLAYER_STANCE_ID eStance);


	std::string Tool_SkillIdentitySuffix(
		const Client::PLAYER_SKILL_DEFINITION& Skill);


	struct TOOL_SOURCE_ANCHOR_REQUEST final
	{
		std::string strRuntimeAnchorSlotId;
		std::string strRuntimeBoneName;
		Client::EFFECT_TRANSFORM_DESC SocketLocalTransform{};
		Client::EFFECT_ATTACHMENT_ORIENTATION eOrientation =
			Client::EFFECT_ATTACHMENT_ORIENTATION::BONE;
		bool_t bNormalizeSourceImportScale = false;
	};

	std::vector<TOOL_SOURCE_ANCHOR_REQUEST> Collect_ToolSourceAnchorRequests(
		const Client::EFFECT_DOCUMENT_DESC& Document);


	bool_t Has_RequiredSourceFollowAttachments(const Client::EFFECT_DOCUMENT_DESC& Document);


	bool Try_ResolveToolAttachmentOwnerWorld(float4x4_t& OutWorld);


	bool Build_ToolValtanSourceAnchorWorld(
		const float4x4_t& RawBone,
		const float4x4_t& SampledOwnerRoot,
		const Client::VALTAN_PATTERN_EFFECT_SCALE_POLICY eScalePolicy,
		const float3_t& WorldScale,
		const float4x4_t& ActualOwnerWorld,
		const Client::EFFECT_ATTACHMENT_ORIENTATION eOrientation,
		float4x4_t& OutWorld);


	bool Try_ResolveToolCameraAnchorWorld(
		const Client::EFFECT_TRANSFORM_DESC& Local, float4x4_t& OutWorld);


	bool Resolve_ToolSourceAnchorWorlds(
		const Client::EFFECT_DOCUMENT_DESC& Document,
		const Client::VALTAN_PRODUCT_EFFECT_CUE_VIEW* pValtanCue,
		std::unordered_map<std::string, float4x4_t>& OutWorlds,
		std::string& strOutError);


	bool Is_CompilerOwnedPortableRecipe(
		const Client::EFFECT_DOCUMENT_DESC& Document,
		const Client::EFFECT_ELEMENT_DESC& Element);


	bool Try_ParseMaterialExecutionLaneSlotId(
		const std::string_view strSlotId,
		std::string_view& strOutLaneId);


	std::string MaterialExecutionLaneSlotId(const std::string& strLaneId);


	Client::EFFECT_MATERIAL_TEXTURE_LANE_DESC* Find_MaterialExecutionLane(
		Client::EFFECT_ELEMENT_DESC& Element,
		const std::string_view strSlotId);


	Client::EFFECT_NAMED_TEXTURE_DESC* Find_SourceMaterialTexture(
		Client::EFFECT_ELEMENT_DESC& Element,
		const std::string_view strSlotId);


	const Client::EFFECT_NAMED_TEXTURE_DESC* Find_SourceMaterialTexture(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const std::string_view strSlotId);


	bool Reset_AllAuthoringOverrides(
		Client::EFFECT_ELEMENT_DESC& Element,
		std::string& strOutError);


	const Client::EFFECT_MATERIAL_TEXTURE_LANE_DESC*
	Find_MaterialExecutionLane(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const std::string_view strSlotId);


	const char* MaterialExecutionBackendLabel(
		const Client::EFFECT_MATERIAL_EXECUTION_BACKEND eBackend);


	const char* MaterialTextureColorSpaceLabel(
		const Client::EFFECT_TEXTURE_COLOR_SPACE eColorSpace);


	const char* MaterialTextureFilterLabel(
		const Client::EFFECT_MATERIAL_TEXTURE_FILTER eFilter);


	bool Try_ResolveArtistCoreFamily(
		const Client::EFFECT_RUNTIME_RENDERER_KIND eRenderer,
		Client::EFFECT_GPU_RENDER_FAMILY& eOutFamily);


	bool Try_NarrowRuntimeFloat3(
		const std::array<double, 3u>& Source,
		float3_t& OutValue);


	const char* ArtistCoreFamilyLabel(
		const Client::EFFECT_GPU_RENDER_FAMILY eFamily);


	const char* VisualProgramFamilyLabel(
		const Client::EFFECT_VISUAL_PROGRAM_FAMILY eFamily);


	std::string StableIdentityLeaf(const std::string_view strIdentity);


	std::string ResourceAssetLeaf(const std::string_view strAssetId);


	std::string PrimaryVisualResourceLeaf(
		const std::vector<Client::EFFECT_VISUAL_PROGRAM_RESOURCE_PACKET_ROW>&
			Resources);


	std::string PrimaryAuthoringResourceLeaf(
		const Client::EFFECT_ELEMENT_DESC& Element);


	std::string FriendlyModelCueLabel(
		const size_t iOrdinal,
		const Client::EFFECT_MODEL_CUE_DESC& Cue);


	std::string FriendlyDocumentLabel(
		const Client::EFFECT_DOCUMENT_DESC& Document,
		const std::string_view strFallback);


	std::string StableUnifiedElementId(
		const Client::EFFECT_GPU_RENDER_FAMILY eFamily,
		const std::string_view strSourceIdentity);


	std::string VisualProgramResourceSlotSummary(
		const std::vector<Client::EFFECT_VISUAL_PROGRAM_RESOURCE_PACKET_ROW>&
			Resources);


	std::string AuthoringElementResourceSlotSummary(
		const Client::EFFECT_ELEMENT_DESC& Element);


	std::string VisualProgramElementRowLabel(
		const Client::EFFECT_VISUAL_PROGRAM_FAMILY eFamily,
		const size_t iElementOrdinal,
		const std::string& strSourceRecordId,
		const std::vector<Client::EFFECT_VISUAL_PROGRAM_RESOURCE_PACKET_ROW>&
			Resources);


	void Upsert_OccurrenceTuningEntry(
		Client::EFFECT_OCCURRENCE_TUNING_DOCUMENT& Document,
		const std::string& strOccurrenceId,
		const std::string& strSourceRowSha256,
		const Client::EFFECT_OCCURRENCE_LOCAL_TRANSFORM& Transform);


	void Remove_OccurrenceTuningEntry(
		Client::EFFECT_OCCURRENCE_TUNING_DOCUMENT& Document,
		const std::string& strOccurrenceId);


	void Upsert_SourceAuthoringOverlayEntry(
		Client::EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT& Document,
		const std::string& strOccurrenceId,
		const std::string& strSourceRowSha256,
		const std::string& strSourceElementId,
		const Client::EFFECT_OCCURRENCE_LOCAL_TRANSFORM& Transform);


	void Remove_SourceAuthoringOverlayEntry(
		Client::EFFECT_SOURCE_AUTHORING_OVERLAY_DOCUMENT& Document,
		const std::string& strOccurrenceId);


	bool Start_OwnedToolProcess(
		std::wstring Command,
		const std::filesystem::path& WorkingDirectory,
		const std::string_view strLabel,
		HANDLE& hOutProcess,
		std::string& strOutStatus);


    bool Run_OwnedToolProcess(
        std::wstring Command,
        const std::filesystem::path& WorkingDirectory,
        const DWORD iTimeoutMilliseconds,
        const std::string_view strLabel,
        std::string& strOutStatus);


    const char* Kind_Label(const Client::EFFECT_ELEMENT_KIND eKind);


	const char* AuthoringFamily_Label(
		const Client::EFFECT_AUTHORING_FAMILY eFamily);


	Client::EFFECT_ELEMENT_KIND AuthoringFamily_Kind(
		const Client::EFFECT_AUTHORING_FAMILY eFamily);


	bool_t AuthoringFamily_CanCreate(
		const Client::EFFECT_AUTHORING_FAMILY eFamily);


	bool_t AuthoringFamily_RequiresMesh(
		const Client::EFFECT_AUTHORING_FAMILY eFamily);


	const char* AuthoringFamily_ElementPrefix(
		const Client::EFFECT_AUTHORING_FAMILY eFamily);


	Client::EFFECT_AUTHORING_FAMILY Resolve_AuthoringFamily(
		const Client::EFFECT_ELEMENT_DESC& Element);


	bool_t Can_EditElementFollowAttachment(const Client::EFFECT_ELEMENT_DESC& Element);


	const char_t* ScreenPostProfile_Label(
		const Client::EFFECT_SCREEN_POST_PROFILE eProfile);


	bool_t HasAuthoringApproximate(
		const Client::EFFECT_DOCUMENT_DESC& Document);


	std::string FriendlyAuthoringElementLabel(
		const Client::EFFECT_AUTHORING_FAMILY eFamily,
		const size_t iOrdinal,
		const Client::EFFECT_ELEMENT_DESC& Element);


	// CAnimationTargetService reports the preview target by the pAssetName of
	// its ANIMATION_PREVIEW_ASSETS descriptor, which is "Valtan" for the boss
	// entry. Keeping the comparison in one place stops the boss authoring
	// paths from silently falling through to the playable-class branch when
	// that descriptor name changes.
	constexpr const char* VALTAN_ANIMATION_ASSET_NAME = "Valtan";

	bool_t AuthoringFamily_AllowsSlot(
		const Client::EFFECT_AUTHORING_FAMILY eFamily,
		const std::string_view strSlotId);


    const char* Slot_Label(const Client::EFFECT_RESOURCE_SLOT eSlot);


    const char* SourceMaterialStatus_Label(
        const Client::EFFECT_SOURCE_MATERIAL_STATUS eStatus);


    const char* Class_Label(
        const LostArk::Shared::CHARACTER_CLASS_ID eClass);


    const char* Resource_DomainId(
        const LostArk::Shared::CHARACTER_CLASS_ID eClass);


    bool Is_KoukuEffectAssetId(const std::string_view strEffectAssetId);


    bool Is_WorldEffectAssetId(const std::string_view strEffectAssetId);


    bool Is_SceneAnchoredEffectAssetId(const std::string_view strEffectAssetId);


    std::string EffectAsset_DomainId(const std::string& strEffectAssetId);


    std::string First_PathComponent(const std::filesystem::path& Relative);


    bool Try_DeriveEffectAssetIdFromFilename(
        const std::filesystem::path& Path,
        const Client::EFFECT_DOCUMENT_SOURCE eSource,
        std::string& OutAssetId);


    bool_t Ensure_PlayerSkillCatalog(std::string& OutStatus);


    const char* Animation_AssetName(
        const LostArk::Shared::CHARACTER_CLASS_ID eClass);


    std::vector<std::string> Collect_AnimationClipNames(
        const std::shared_ptr<Engine::CModel>& pModel);


    bool Read_TextFile(
        const std::filesystem::path& Path,
        std::string& OutText,
        std::string& OutStatus);


	bool Validate_RegistryBoundAuditionSourceFreshness(
		const std::filesystem::path& SourceDocumentPath,
		const std::string_view strExpectedRawSha256,
		std::string& strOutStatus);


    std::vector<Client::ANIMATION_SKILL_CLIP> Flatten_BindingClips(
        const Client::ANIMATION_SKILL_BINDING& Binding);


    bool Try_ParseEffectDiagnosticRow(
        const std::string_view Line,
        std::string& OutClip,
        bool_t& OutImported,
        bool_t& OutEmptyPayload);


	bool_t Try_ExtractPlanarYawDegrees(
		const float4x4_t& Root,
		f32_t& fOutYawDegrees);


	const char* Source_Label(const Client::EFFECT_DOCUMENT_SOURCE eSource);


	std::string Unified_CandidateAssetId(const std::string_view AssetId);


	const char* Profile_Label(const Client::EFFECT_RENDER_PROFILE eProfile);


    bool Contains_NoCase(
        const std::string& Value,
        const std::string_view Filter);


    std::string Build_ValtanV0EffectAssetId(const std::string_view ClipName);


    bool Matches_MeshShapeCategory(
        const std::string& strAssetId,
        const std::string_view strCategory);


    // Effect DDS filenames follow fx_<bucket>_<kind>_<index>[_variant]. The
    // bucket letter only mirrors the source package folder, so the kind token
    // is the only part that says what the texture is for. Tokens that also
    // appear inside unrelated words are anchored with the separator.
    bool Matches_TextureKindCategory(
        const std::string& strAssetId,
        const std::string_view strCategory);


    enum class CASCADE_RENDERER_KIND : uint8_t
    {
        MESH,
        SPRITE,
        UNRESOLVED
    };

    CASCADE_RENDERER_KIND Resolve_CascadeRendererKind(
        const Client::EFFECT_ELEMENT_DESC& Element);


    const char* CascadeRenderer_Label(const CASCADE_RENDERER_KIND eKind);


    const char* Element_RendererLabel(
        const Client::EFFECT_ELEMENT_DESC& Element);


	const char* PreviewPivot_Label(
		const Client::EFFECT_PREVIEW_PIVOT_KIND eKind);


	std::string Lower_Ascii(const std::string_view Value);


	struct SOURCE_MODULE_UI_DESC final
	{
		const char* pRole = "Source Module";
		const char* pDescription =
			"Lossless source values are shown with their original property paths.";
	};

	SOURCE_MODULE_UI_DESC Describe_SourceModule(
		const std::string_view strClassName);


	std::string Friendly_SourcePropertyLabel(
		const std::string_view strPropertyPath);


	Client::EFFECT_RESOURCE_FILE_KIND Resource_FileKind(
		const Client::EFFECT_RESOURCE_BINDING_DESC& Binding);


	void Render_SourceMaterialParameterGroups(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& SourceMaterial,
		Client::CEffectThumbnailCache* pThumbnailCache);


    bool Slot_Allowed(
        const Client::EFFECT_ELEMENT_KIND eKind,
        const Client::EFFECT_RESOURCE_SLOT eSlot);


    Client::EFFECT_RESOURCE_FILE_KIND Slot_FileKind(
        const Client::EFFECT_RESOURCE_SLOT eSlot);


    std::string Default_SlotId(const Client::EFFECT_ELEMENT_KIND eKind);


    const Client::EFFECT_RESOURCE_BINDING_DESC* Find_Binding(
        const Client::EFFECT_ELEMENT_DESC& Element,
        const std::string_view strSlotId);


	bool Is_ParticleMasterEmissionProgram(
		const Client::EFFECT_ELEMENT_DESC& Element);


	bool Has_EffectiveEmissiveRadianceInput(
		const Client::EFFECT_ELEMENT_DESC& Element);


	bool Is_SourceDecalBaseAdmissionCarrier(
		const Client::EFFECT_ELEMENT_DESC& Element);


	bool Has_BaseTextureBinding(const Client::EFFECT_ELEMENT_DESC& Element);


	bool Is_MissingBaseSourceDecal(
		const Client::EFFECT_ELEMENT_DESC& Element);


	bool Is_BaseTextureSlot(const std::string_view strSlotId);


	bool Is_ElementPreviewAdmitted(
		const Client::EFFECT_ELEMENT_DESC& Element);


	const char* ElementPreviewAdmissionReason(
		const Client::EFFECT_ELEMENT_DESC& Element);


	void Prune_MissingElementMarks(
		const Client::EFFECT_DOCUMENT_DESC& Document,
		std::set<std::string, std::less<>>& Marks);


    struct PARTICLE_LAYER_SUMMARY final
    {
        size_t iStandaloneMeshCount = 0u;
        size_t iStandaloneSpriteCount = 0u;
        size_t iSourceSystemCount = 0u;
        size_t iSourceEmitterCount = 0u;
        size_t iLayerCount = 0u;
        size_t iMeshRendererCount = 0u;
        size_t iSpriteRendererCount = 0u;
        size_t iUnresolvedRendererCount = 0u;
        uint64_t iParticleBudget = 0u;
    };

    PARTICLE_LAYER_SUMMARY Summarize_ParticleLayers(
        const Client::EFFECT_DOCUMENT_DESC& Document);


	bool_t Is_SourceParticleCarrier(
		const Client::EFFECT_ELEMENT_DESC& Element);


	bool_t Is_PreviewParticleSimulationElement(
		const Client::EFFECT_ELEMENT_DESC& Element);


	f32_t Element_PreviewEndSeconds(
		const Client::EFFECT_ELEMENT_DESC& Element);


    bool Slot_Allowed(
        const Client::EFFECT_ELEMENT_DESC& Element,
        const std::string_view strSlotId);


	bool Is_DirectHandAuthoredElement(
		const Client::EFFECT_ELEMENT_DESC& Element);


	bool Is_GenericLinearRevealCarrier(
		const Client::EFFECT_ELEMENT_DESC& Element);


	bool Is_OptionalHandAuthoredResourceSlot(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const std::string_view strSlotId);


	Client::EFFECT_RESOURCE_FILE_KIND Slot_FileKind(
        const Client::EFFECT_ELEMENT_DESC& Element,
        const std::string_view strSlotId);


    std::string Slot_Label(
        const Client::EFFECT_ELEMENT_DESC& Element,
        const std::string_view strSlotId);


    bool InputFloat2(const char* Label, float2_t& Value);


    bool InputFloat3(const char* Label, float3_t& Value);


    bool InputFloat4(const char* Label, float4_t& Value);


    bool DragFloat2(
        const char* Label,
        float2_t& Value,
        const float Speed,
        const float Minimum,
        const float Maximum,
        const char* Format = "%.3f");


    bool DragFloat3(
        const char* Label,
        float3_t& Value,
        const float Speed,
        const float Minimum,
        const float Maximum,
        const char* Format = "%.3f");


    bool DragFloat4(
        const char* Label,
        float4_t& Value,
        const float Speed,
        const float Minimum,
        const float Maximum,
        const char* Format = "%.3f");


    void Copy_Buffer(char* pDestination, const size_t iCapacity,
        const std::string& Source);


    bool_t Is_ManualElementGroupMember(
        const Client::EFFECT_ELEMENT_DESC& Element);


    std::string ManualGroup_Label(const std::string& strGroupId);


    std::string ManualElement_Label(
        const Client::EFFECT_ELEMENT_DESC& Element);


    float4x4_t Identity_Matrix();


    const Client::CHARACTER_SPEC* Resolve_CurrentTargetSpec();

}
using namespace EffectToolDetail;
