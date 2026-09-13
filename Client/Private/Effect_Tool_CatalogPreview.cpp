#include "imgui.h"
#include "Effect_Tool_Internal.h"
#include "ActorCatalog.h"
#include "Character.h"
#include "CharacterSpec.h"
#include "CombatHUDViewModel.h"
#include "Effect_Catalog.h"
#include "Effect_DocumentCodec.h"
#include "Effect_DirectAuthoredSourceIndex.h"
#include "EffectResourceCatalog.h"
#include "ValtanPatternEffectCueDocument.h"
#include "Effect_RuntimeAuthority.h"
#include "Logic_DimensionMaster.h"
#include "MapEffectPresentationRuntime.h"
#include <algorithm>
#include <array>
#include <atomic>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <map>
#include <sstream>
#include <set>
#include <string_view>
#include <system_error>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include "Model.h"
#include "Transform.h"
#include "EffectAuthoringSequencer.h"

bool_t Client::CEffect_Tool::Refresh_UnifiedEffectCache(
	UNIFIED_EFFECT_CACHE& Cache,
	const std::filesystem::path& Path,
	const std::string& strExpectedEffectAssetId)
{
	std::error_code FileError;
	const bool_t bExists = !Path.empty() &&
		std::filesystem::exists(Path, FileError);
	if (FileError)
	{
		Cache.Path = Path;
		Cache.bObserved = true;
		Cache.bExists = true;
		Cache.bValid = false;
		Cache.bDrawable = false;
		Cache.bPreviewReady = false;
		Cache.Document = {};
		Cache.strDrawableError.clear();
		Cache.strPreviewReadinessError.clear();
		Cache.strStatus =
			"Unified Effect path could not be inspected: " + FileError.message();
		return false;
	}
	if (!bExists)
	{
		if (!Cache.bObserved || Cache.Path != Path || Cache.bExists)
		{
			Cache = {};
			Cache.Path = Path;
			Cache.bObserved = true;
			Cache.strStatus = "Unified Effect has not been created yet.";
		}
		return false;
	}

	const bool_t bRegular = std::filesystem::is_regular_file(Path, FileError);
	if (FileError || !bRegular)
	{
		Cache.Path = Path;
		Cache.bObserved = true;
		Cache.bExists = true;
		Cache.bValid = false;
		Cache.bDrawable = false;
		Cache.bPreviewReady = false;
		Cache.Document = {};
		Cache.strDrawableError.clear();
		Cache.strPreviewReadinessError.clear();
		Cache.strStatus = FileError ?
			("Unified Effect path could not be inspected: " + FileError.message()) :
			"Unified Effect path exists but is not a regular file; overwrite is blocked.";
		return false;
	}

	const std::filesystem::file_time_type LastWriteTime =
		std::filesystem::last_write_time(Path, FileError);
	if (FileError)
	{
		Cache.Path = Path;
		Cache.bObserved = true;
		Cache.bExists = true;
		Cache.bValid = false;
		Cache.bDrawable = false;
		Cache.bPreviewReady = false;
		Cache.Document = {};
		Cache.strDrawableError.clear();
		Cache.strPreviewReadinessError.clear();
		Cache.strStatus =
			"Unified Effect timestamp could not be read: " + FileError.message();
		return false;
	}
	const uint64_t iFileSize = static_cast<uint64_t>(
		std::filesystem::file_size(Path, FileError));
	if (FileError)
	{
		Cache.Path = Path;
		Cache.bObserved = true;
		Cache.bExists = true;
		Cache.bValid = false;
		Cache.bDrawable = false;
		Cache.bPreviewReady = false;
		Cache.Document = {};
		Cache.strDrawableError.clear();
		Cache.strPreviewReadinessError.clear();
		Cache.strStatus =
			"Unified Effect size could not be read: " + FileError.message();
		return false;
	}
	if (Cache.bObserved && Cache.bExists && Cache.Path == Path &&
		Cache.LastWriteTime == LastWriteTime && Cache.iFileSize == iFileSize)
	{
		return Cache.bValid;
	}

	UNIFIED_EFFECT_CACHE Staged;
	Staged.Path = Path;
	Staged.LastWriteTime = LastWriteTime;
	Staged.iFileSize = iFileSize;
	Staged.bObserved = true;
	Staged.bExists = true;
	std::string Error;
	if (!CEffectDocumentCodec::Load(Path, Staged.Document, Error))
	{
		Staged.strStatus = "Unified Effect could not be parsed: " + Error;
		Cache = std::move(Staged);
		return false;
	}
	if (Staged.Document.strEffectAssetId != strExpectedEffectAssetId)
	{
		Staged.strStatus = "Unified Effect ID mismatch; expected '" +
			strExpectedEffectAssetId + "', found '" +
			Staged.Document.strEffectAssetId + "'. Overwrite is blocked.";
		Staged.Document = {};
		Staged.bDrawable = false;
		Staged.strDrawableError.clear();
		Cache = std::move(Staged);
		return false;
	}
	if (!CEffectDocumentCodec::Validate_Drawable(Staged.Document, Error))
	{
		Staged.bValid = true;
		Staged.bDrawable = false;
		Staged.bPreviewReady = false;
		Staged.strDrawableError = Error;
		Staged.strPreviewReadinessError = Error;
		Staged.strStatus =
			"Saved authoring parent is structurally valid but not drawable yet: " +
			Error;
		Cache = std::move(Staged);
		return true;
	}
	Staged.bValid = true;
	Staged.bDrawable = true;
	Staged.strDrawableError.clear();
	Staged.bPreviewReady = Validate_UnifiedEffectPreviewReadiness(
		Staged.Document, Staged.strPreviewReadinessError);
	Staged.strStatus = Staged.bPreviewReady ?
		"Document validation passed; individual Element Solo eligibility is shown on its row." :
		("Editable Skill Effect is not ready for preview: " +
		 Staged.strPreviewReadinessError);
	Cache = std::move(Staged);
	return true;
}

void Client::CEffect_Tool::Initialize_CatalogMetadataView()
{
	if (m_bCatalogMetadataViewInitialized)
		return;
	m_bCatalogMetadataViewInitialized = true;
	/* A typed deep-link can explicitly refresh the exact authored/Product
	   indexes before the first visible Render.  Keep that admitted result;
	   the lightweight bootstrap must never replace a user-requested refresh. */
	if (m_bResourceCatalogRefreshAttempted ||
		m_bAllEffectsRefreshAttempted ||
		m_bDataFilesRefreshAttempted)
	{
		return;
	}

	/* Seed a failure-preserving tree from the already admitted runtime catalog.
	   All Effects discovers authored metadata on its first visible open.
	   Open/Play alone decode one exact document. */
	std::vector<EFFECT_DATA_FILE_ENTRY> StagedDataFiles;
	std::set<std::string> StagedDomains;
	std::unordered_map<std::string, DIRECT_AUTHORED_EDITABLE_ENTRY>
		StagedEditableEntries;
	std::vector<EFFECT_DIRECT_AUTHORED_SOURCE_ENTRY>
		StagedValtanSources;
	const std::vector<std::string> EffectAssetIds =
		CEffectCatalog::Get_EffectAssetIds();
	StagedDataFiles.reserve(EffectAssetIds.size());
	StagedEditableEntries.reserve(EffectAssetIds.size());
	StagedValtanSources.reserve(EffectAssetIds.size());
	for (const std::string& strEffectAssetId : EffectAssetIds)
	{
		const std::string strDomainId =
			EffectAsset_DomainId(strEffectAssetId);
		if (CEffectCatalog::Is_DirectAuthoredDocument(strEffectAssetId))
		{
			const std::filesystem::path Path = CProjectDataRoot::Resolve(
				std::filesystem::path(L"Effects") / L"Authored" /
				(std::filesystem::path(strEffectAssetId).wstring() +
					L".effect.json"));
			if (Path.empty())
				continue;
			EFFECT_DATA_FILE_ENTRY Entry{
				strEffectAssetId, strDomainId, Path,
				EFFECT_DOCUMENT_SOURCE::AUTHORED };
			Entry.strDocumentParseStatus =
				"Catalog metadata only; Open Editor loads Details on demand.";
			StagedDataFiles.push_back(std::move(Entry));
			DIRECT_AUTHORED_EDITABLE_ENTRY Editable;
			Editable.Path = Path;
			Editable.strStatus =
				"Catalog metadata ready; exact identity is checked when Open or Play is pressed.";
			StagedEditableEntries.emplace(
				strEffectAssetId, std::move(Editable));
			if (strEffectAssetId.starts_with("effect.valtan."))
			{
				EFFECT_DIRECT_AUTHORED_SOURCE_ENTRY Source;
				Source.strEffectAssetId = strEffectAssetId;
				Source.Path = Path;
				StagedValtanSources.push_back(std::move(Source));
			}
			StagedDomains.insert(strDomainId);
			continue;
		}

		if (nullptr != CEffectCatalog::Find_Assembly(strEffectAssetId))
		{
			StagedDataFiles.push_back({
				strEffectAssetId + "::assembly", strDomainId, {},
				EFFECT_DOCUMENT_SOURCE::RUNTIME_ASSEMBLY });
			StagedDomains.insert(strDomainId);
		}
	}
	for (const std::string& strComponentId :
		CEffectCatalog::Get_ComponentAssetIds())
	{
		const std::shared_ptr<const EFFECT_COMPONENT_DESC> Component =
			CEffectCatalog::Find_Component(strComponentId);
		if (nullptr == Component)
			continue;
		const std::string strDomainId =
			EffectAsset_DomainId(Component->strSourceEffectAssetId);
		StagedDataFiles.push_back({
			strComponentId, strDomainId, {},
			EFFECT_DOCUMENT_SOURCE::RUNTIME_COMPONENT });
		StagedDomains.insert(strDomainId);
	}

	std::ranges::sort(StagedDataFiles,
		[](const EFFECT_DATA_FILE_ENTRY& Left,
			const EFFECT_DATA_FILE_ENTRY& Right)
		{
			return std::tie(Left.strDomainId, Left.eSource,
				Left.strAssetId) <
				std::tie(Right.strDomainId, Right.eSource,
					Right.strAssetId);
		});
	std::ranges::sort(StagedValtanSources,
		[](const EFFECT_DIRECT_AUTHORED_SOURCE_ENTRY& Left,
			const EFFECT_DIRECT_AUTHORED_SOURCE_ENTRY& Right)
		{
			return Left.strEffectAssetId < Right.strEffectAssetId;
		});

	std::vector<EFFECT_SKILL_TREE_ENTRY> StagedSkills;
	std::vector<UNIFIED_EFFECT_CANDIDATE_BINDING> StagedBindings;
	std::unordered_map<std::string, UNIFIED_EFFECT_CACHE> StagedCaches;
	const std::vector<PLAYER_SKILL_DEFINITION>& Skills =
		CPlayerSkillCatalog::Get_Skills();
	StagedSkills.reserve(Skills.size());
	StagedBindings.reserve(StagedDataFiles.size());
	StagedCaches.reserve(StagedDataFiles.size());
	for (const PLAYER_SKILL_DEFINITION& Skill : Skills)
	{
		EFFECT_SKILL_TREE_ENTRY Entry;
		Entry.Skill = Skill;
		StagedSkills.push_back(std::move(Entry));
		const char_t* const pDomainId = Resource_DomainId(
			Skill.eCharacterClass);
		if (nullptr == pDomainId)
			continue;
		const std::string strSkillEffectPrefix = "effect." +
			Lower_Ascii(pDomainId) + ".skill." +
			std::to_string(static_cast<uint32_t>(Skill.iSkillId));
		for (const EFFECT_DATA_FILE_ENTRY& DataFile : StagedDataFiles)
		{
			if (EFFECT_DOCUMENT_SOURCE::AUTHORED != DataFile.eSource ||
				(DataFile.strAssetId != strSkillEffectPrefix &&
				 !DataFile.strAssetId.starts_with(
					 strSkillEffectPrefix + ".")))
			{
				continue;
			}
			UNIFIED_EFFECT_CANDIDATE_BINDING Binding;
			Binding.eCharacterClass = Skill.eCharacterClass;
			Binding.iSkillId = Skill.iSkillId;
			Binding.strEffectAssetId = DataFile.strAssetId;
			Binding.Path = DataFile.Path;
			StagedBindings.push_back(std::move(Binding));
			UNIFIED_EFFECT_CACHE Cache;
			Cache.Path = DataFile.Path;
			StagedCaches.emplace(DataFile.strAssetId, std::move(Cache));
		}
	}

	m_DataFiles = std::move(StagedDataFiles);
	m_DataFileDomains.assign(StagedDomains.begin(), StagedDomains.end());
	m_DirectAuthoredEditableEntries = std::move(StagedEditableEntries);
	m_UnifiedCandidateBindings = std::move(StagedBindings);
	m_UnifiedCandidateCaches = std::move(StagedCaches);
	m_ValtanExactAuthoredSources = std::move(StagedValtanSources);
	m_AllEffects = std::move(StagedSkills);
	if (m_DataFileDomains.end() == std::find(
			m_DataFileDomains.begin(), m_DataFileDomains.end(),
			m_strSelectedAuthoringDomainId) && !m_DataFileDomains.empty())
	{
		Select_AuthoringDomain(m_DataFileDomains.front());
	}
	m_strDocumentStatus =
		"Catalog metadata ready: " + std::to_string(m_DataFiles.size()) +
		" meaningful entries; document decode is deferred until Open or Play.";
	m_strDirectAuthoredEditableStatus =
		"Catalog metadata view is ready; stable authored paths are available on demand.";
	m_strUnifiedCandidateStatus =
		"Catalog stable IDs staged " +
		std::to_string(m_UnifiedCandidateBindings.size()) +
		" saved Player Effects; no Effect document was decoded.";
}

bool_t Client::CEffect_Tool::Refresh_DirectAuthoredEditableIndex(
	const std::vector<EFFECT_DATA_FILE_ENTRY>& DataFiles)
{
	const auto PreservePrevious = [this](std::string Status)
	{
		m_strDirectAuthoredEditableStatus = std::move(Status);
		m_strUnifiedCandidateStatus =
			"Saved authored catalog refresh preserved the previous index: " +
			m_strDirectAuthoredEditableStatus;
		return false;
	};
	const std::filesystem::path CatalogPath = CProjectDataRoot::Resolve(
		std::filesystem::path(L"Effects") / L"EffectCatalog.json");
	const std::filesystem::path AuditionCatalogPath = CProjectDataRoot::Resolve(
		std::filesystem::path(L"Effects") / L"EffectAuditionCatalog.json");
	const std::filesystem::path AuthoredRoot = CProjectDataRoot::Resolve(
		std::filesystem::path(L"Effects") / L"Authored");
	std::string SkillCatalogStatus;
	EFFECT_DIRECT_AUTHORED_OWNER_SET PlayerSkillOwners;
	std::string PlayerOwnerIsolationStatus;
	if (Ensure_PlayerSkillCatalog(SkillCatalogStatus))
	{
		for (const PLAYER_SKILL_DEFINITION& Skill :
			CPlayerSkillCatalog::Get_Skills())
		{
			PlayerSkillOwners.emplace(Skill.eCharacterClass, Skill.iSkillId);
		}
	}
	else
	{
		PlayerOwnerIsolationStatus =
			"Player Product joins were isolated while exact authored documents remained editor-eligible: " +
			SkillCatalogStatus;
	}
	VALTAN_PATTERN_EFFECT_CUE_DOCUMENT BossCueDocument;
	std::string BossCueStatus;
	EFFECT_DIRECT_AUTHORED_BOSS_OWNER_MAP BossPatternOwners;
	std::unordered_map<std::string, size_t> StagedBossProductCueMappingCounts;
	EFFECT_DIRECT_AUTHORED_BOSS_COMBAT_OBJECT_OWNER_MAP
		BossCombatObjectOwners;
	std::string BossOwnerIsolationStatus;
	if (CValtanPatternEffectCueDocument::Load_ReadOnlyProduct(
			BossCueDocument, BossCueStatus))
	{
		for (const VALTAN_PATTERN_EFFECT_CUE& Cue : BossCueDocument.Cues)
		{
			const EFFECT_DIRECT_AUTHORED_BOSS_OWNER Owner{
				BossCueDocument.strOwnerArchetypeId, Cue.strPatternId,
					Cue.strStageId, Cue.strActionId };
			BossPatternOwners.emplace(Cue.strEffectAssetId, Owner);
			++StagedBossProductCueMappingCounts[Cue.strEffectAssetId];
			if (!Cue.strV1EffectAssetId.empty())
			{
				BossPatternOwners.emplace(Cue.strV1EffectAssetId, Owner);
				++StagedBossProductCueMappingCounts[Cue.strV1EffectAssetId];
			}
		}

		const BOSS_ACTOR_ENTRY* pBossActor = CActorCatalog::Find_Boss(
			BossCueDocument.strOwnerArchetypeId);
		if (nullptr == pBossActor)
		{
			BossOwnerIsolationStatus =
				"Valtan combat-object saved rows were isolated: " +
				CActorCatalog::Get_Status();
		}
		else
		{
			for (const BOSS_COMBAT_OBJECT_VISUAL_ENTRY& Visual :
				pBossActor->combatObjectVisuals)
			{
				const EFFECT_DIRECT_AUTHORED_BOSS_COMBAT_OBJECT_OWNER Owner{
					pBossActor->archetypeId,
					Visual.combatObjectArchetypeId,
					Visual.clientVisualId };
				BossCombatObjectOwners.emplace(Visual.effectAssetId, Owner);
				++StagedBossProductCueMappingCounts[Visual.effectAssetId];
				if (!Visual.hitEffectAssetId.empty())
				{
					BossCombatObjectOwners.emplace(
						Visual.hitEffectAssetId, Owner);
					++StagedBossProductCueMappingCounts[
						Visual.hitEffectAssetId];
				}
			}
		}
	}
	else
	{
		BossOwnerIsolationStatus =
			"Valtan saved rows were isolated while Player saved rows remained available: " +
			BossCueStatus;
	}

	std::vector<EFFECT_DIRECT_AUTHORED_SCANNED_FILE> ScannedFiles;
	ScannedFiles.reserve(DataFiles.size());
	for (const EFFECT_DATA_FILE_ENTRY& DataFile : DataFiles)
	{
		if (EFFECT_DOCUMENT_SOURCE::AUTHORED == DataFile.eSource)
		{
			ScannedFiles.push_back(
				{ DataFile.strAssetId, DataFile.Path });
		}
	}
	EFFECT_DIRECT_AUTHORED_SOURCE_INDEX SourceIndex;
	std::string SourceIndexStatus;
	if (!CEffectDirectAuthoredSourceIndex::Build(
			CatalogPath, AuditionCatalogPath, AuthoredRoot, ScannedFiles,
			PlayerSkillOwners,
			BossPatternOwners, BossCombatObjectOwners,
			SourceIndex, SourceIndexStatus))
	{
		return PreservePrevious(SourceIndexStatus);
	}

	std::unordered_map<std::string, DIRECT_AUTHORED_EDITABLE_ENTRY>
		StagedEntries;
	StagedEntries.reserve(SourceIndex.Entries.size());
	std::vector<UNIFIED_EFFECT_CANDIDATE_BINDING> StagedBindings;
	StagedBindings.reserve(SourceIndex.Entries.size());
	std::vector<EFFECT_DIRECT_AUTHORED_SOURCE_ENTRY>
		StagedValtanExactAuthoredSources;
	StagedValtanExactAuthoredSources.reserve(SourceIndex.Entries.size());
	for (const EFFECT_DIRECT_AUTHORED_SOURCE_ENTRY& Source :
		SourceIndex.Entries)
	{
		DIRECT_AUTHORED_EDITABLE_ENTRY Staged;
		Staged.Path = Source.Path;
		Staged.bRegistryBoundAuditionOnly =
			Source.bRegistryBoundAuditionOnly;
		Staged.bAuditionSourceFreshnessValid =
			Source.bAuditionSourceFreshnessValid;
		Staged.strSourceEffectAssetId = Source.strSourceEffectAssetId;
		Staged.SourceDocumentPath = Source.SourceDocumentPath;
		Staged.strSourceDocumentRawSha256 =
			Source.strSourceDocumentRawSha256;
		Staged.LastWriteTime = Source.LastWriteTime;
		Staged.iFileSize = Source.iFileSize;
		const auto Existing = m_DirectAuthoredEditableEntries.find(
			Source.strEffectAssetId);
		if (Existing != m_DirectAuthoredEditableEntries.end() &&
			Existing->second.Path.lexically_normal() ==
				Staged.Path.lexically_normal() &&
			Existing->second.bRegistryBoundAuditionOnly ==
				Staged.bRegistryBoundAuditionOnly &&
			Existing->second.bAuditionSourceFreshnessValid ==
				Staged.bAuditionSourceFreshnessValid &&
			Existing->second.strSourceEffectAssetId ==
				Staged.strSourceEffectAssetId &&
			Existing->second.SourceDocumentPath.lexically_normal() ==
				Staged.SourceDocumentPath.lexically_normal() &&
			Existing->second.strSourceDocumentRawSha256 ==
				Staged.strSourceDocumentRawSha256 &&
			Existing->second.LastWriteTime == Staged.LastWriteTime &&
			Existing->second.iFileSize == Staged.iFileSize)
		{
			Staged = Existing->second;
		}
		StagedEntries.emplace(Source.strEffectAssetId, std::move(Staged));
		const bool_t bValtanOwner =
			((EFFECT_DIRECT_AUTHORED_OWNER_KIND::BOSS_PATTERN ==
					Source.eOwnerKind ||
				EFFECT_DIRECT_AUTHORED_OWNER_KIND::BOSS_COMBAT_OBJECT ==
					Source.eOwnerKind) &&
				"BOSS_VALTAN" == Source.strOwnerArchetypeId);
		if (bValtanOwner || Source.strEffectAssetId.starts_with("effect.valtan."))
		{
			StagedValtanExactAuthoredSources.push_back(Source);
		}
		/* A syntactically stable player source identity is sufficient to keep the
		   document discoverable in the editor. PlayerSkills admission is a
		   separate Product Play join and must not turn an exact source into
		   Saved 0. */
		if (LostArk::Shared::CHARACTER_CLASS_ID::END !=
				Source.eCharacterClass &&
			LostArk::Shared::INVALID_SKILL_ID != Source.iSkillId)
		{
			UNIFIED_EFFECT_CANDIDATE_BINDING Binding;
			Binding.eCharacterClass = Source.eCharacterClass;
			Binding.iSkillId = Source.iSkillId;
			Binding.strEffectAssetId = Source.strEffectAssetId;
			Binding.Path = Source.Path;
			Binding.bProductOwnerJoined =
				EFFECT_DIRECT_AUTHORED_OWNER_KIND::PLAYER_SKILL ==
					Source.eOwnerKind;
			StagedBindings.push_back(std::move(Binding));
		}
	}
	std::ranges::sort(
		StagedValtanExactAuthoredSources,
		[](const EFFECT_DIRECT_AUTHORED_SOURCE_ENTRY& Left,
			const EFFECT_DIRECT_AUTHORED_SOURCE_ENTRY& Right)
		{
			return Left.strEffectAssetId < Right.strEffectAssetId;
		});
	std::unordered_map<std::string, UNIFIED_EFFECT_CACHE> StagedCaches;
	StagedCaches.reserve(StagedBindings.size());
	for (const UNIFIED_EFFECT_CANDIDATE_BINDING& Binding : StagedBindings)
	{
		const auto Existing = m_UnifiedCandidateCaches.find(
			Binding.strEffectAssetId);
		if (Existing != m_UnifiedCandidateCaches.end() &&
			Existing->second.Path.lexically_normal() ==
				Binding.Path.lexically_normal())
		{
			StagedCaches.emplace(
				Binding.strEffectAssetId, std::move(Existing->second));
		}
		else
		{
			UNIFIED_EFFECT_CACHE Cache;
			Cache.Path = Binding.Path;
			StagedCaches.emplace(
				Binding.strEffectAssetId, std::move(Cache));
		}
	}
	m_DirectAuthoredEditableEntries = std::move(StagedEntries);
	m_UnifiedCandidateBindings = std::move(StagedBindings);
	m_UnifiedCandidateCaches = std::move(StagedCaches);
	m_ValtanExactAuthoredSources =
		std::move(StagedValtanExactAuthoredSources);
	m_BossProductCueMappingCounts =
		std::move(StagedBossProductCueMappingCounts);
	m_strDirectAuthoredEditableStatus = SourceIndexStatus;
	if (!PlayerOwnerIsolationStatus.empty())
		m_strDirectAuthoredEditableStatus += " " + PlayerOwnerIsolationStatus;
	if (!BossOwnerIsolationStatus.empty())
		m_strDirectAuthoredEditableStatus += " " + BossOwnerIsolationStatus;
	const size_t iJoinedPlayerProductOwners = static_cast<size_t>(
		std::count_if(m_UnifiedCandidateBindings.begin(),
			m_UnifiedCandidateBindings.end(),
			[](const UNIFIED_EFFECT_CANDIDATE_BINDING& Binding)
			{
				return Binding.bProductOwnerJoined;
			}));
	m_strUnifiedCandidateStatus =
		"Saved authored catalog indexed " +
		std::to_string(m_DirectAuthoredEditableEntries.size()) +
		" editor-ready documents and " +
		std::to_string(m_UnifiedCandidateBindings.size()) +
		" stable Player source identities (" +
		std::to_string(iJoinedPlayerProductOwners) +
		" Product owner joins) from EffectCatalog.json; document decode is deferred until Open or Play.";
	if (0u != SourceIndex.iOwnerJoinUnavailableCount)
	{
		m_strUnifiedCandidateStatus += " Product ownership was isolated for " +
			std::to_string(SourceIndex.iOwnerJoinUnavailableCount) +
			" documents without removing Open Editor; first: " +
			SourceIndex.strFirstOwnerJoinUnavailable;
	}
	if (0u != SourceIndex.iUnavailableCount)
	{
		const std::string IsolationStatus = " Isolated " +
			std::to_string(SourceIndex.iUnavailableCount) +
			" unavailable rows; first: " +
			SourceIndex.strFirstUnavailable;
		m_strUnifiedCandidateStatus += IsolationStatus;
	}
	return true;
}

const std::filesystem::path*
Client::CEffect_Tool::Observe_DirectAuthoredEditablePath(
	const std::string& strEffectAssetId,
	std::string& strOutStatus) const
{
	const auto Iterator =
		m_DirectAuthoredEditableEntries.find(strEffectAssetId);
	if (Iterator == m_DirectAuthoredEditableEntries.end() ||
		Iterator->second.Path.empty())
	{
		strOutStatus =
			"Direct-authored source is unavailable in the admitted catalog metadata.";
		return nullptr;
	}
	strOutStatus = Iterator->second.strStatus.empty() ?
		"Catalog metadata ready; exact identity is checked when Open or Play is pressed." :
		Iterator->second.strStatus;
	return &Iterator->second.Path;
}

const std::filesystem::path*
Client::CEffect_Tool::Resolve_DirectAuthoredEditablePath(
	const std::string& strEffectAssetId,
	std::string& strOutStatus)
{
	const auto Iterator =
		m_DirectAuthoredEditableEntries.find(strEffectAssetId);
	if (Iterator == m_DirectAuthoredEditableEntries.end())
	{
		strOutStatus =
			"Direct-authored source is unavailable: this Effect is not an exact writable "
			"DIRECT_AUTHORED_DOCUMENT source path.";
		return nullptr;
	}
	DIRECT_AUTHORED_EDITABLE_ENTRY& Entry = Iterator->second;
	if (Entry.bRegistryBoundAuditionOnly)
	{
		std::string FreshnessStatus;
		if (!Entry.bAuditionSourceFreshnessValid ||
			!Validate_RegistryBoundAuditionSourceFreshness(
				Entry.SourceDocumentPath,
				Entry.strSourceDocumentRawSha256, FreshnessStatus))
		{
			Entry.strStatus = Entry.bAuditionSourceFreshnessValid ?
				FreshnessStatus :
				"Registry-bound audition source pin was stale when the index was refreshed. Correct the source/hash pair, then Refresh Index.";
			strOutStatus = Entry.strStatus;
			return nullptr;
		}
	}
	std::error_code FileError;
	const std::filesystem::file_time_type LastWriteTime =
		std::filesystem::last_write_time(Entry.Path, FileError);
	uint64_t iFileSize = 0u;
	if (!FileError)
	{
		iFileSize = static_cast<uint64_t>(
			std::filesystem::file_size(Entry.Path, FileError));
	}
	if (FileError)
	{
		Entry.bIdentityObserved = true;
		Entry.bIdentityValid = false;
		Entry.strStatus =
			"Direct-authored source is unavailable: the file cannot be inspected: " +
			FileError.message();
		strOutStatus = Entry.strStatus;
		return nullptr;
	}
	if (Entry.LastWriteTime != LastWriteTime || Entry.iFileSize != iFileSize)
	{
		Entry.LastWriteTime = LastWriteTime;
		Entry.iFileSize = iFileSize;
		Entry.bIdentityObserved = false;
		Entry.bIdentityValid = false;
		Entry.strStatus.clear();
	}
	if (!Entry.bIdentityObserved)
	{
		EFFECT_DOCUMENT_DESC Document;
		std::string Error;
		Entry.bIdentityValid = CEffectDocumentCodec::Load(
			Entry.Path, Document, Error) &&
			Document.strEffectAssetId == strEffectAssetId;
		Entry.bIdentityObserved = true;
		if (Entry.bIdentityValid)
		{
			Entry.strStatus = Entry.bRegistryBoundAuditionOnly ?
				"Validated the writable registry-bound audition document and its pinned Product source. Save updates only this audition candidate; Product cues cannot consume its Effect ID." :
				"Validated the writable Data/Effects/Authored document. "
				"Save commits this file as the canonical Product source when the exact ID is mapped by a gameplay cue. The next spawn uses it immediately; a failed activation restores the previous disk and prepared target.";
		}
		else if (!Error.empty())
		{
			Entry.strStatus =
				"Direct-authored source validation failed: " +
				Error;
		}
		else
		{
			Entry.strStatus =
				"Direct-authored source is unavailable: the embedded Effect ID disagrees with EffectCatalog.json.";
		}
	}
	strOutStatus = Entry.strStatus;
	return Entry.bIdentityValid ? &Entry.Path : nullptr;
}

bool_t Client::CEffect_Tool::Validate_ActiveRegistryBoundAuditionFreshness(
	std::string& strOutStatus) const
{
	if (!m_ActiveDocument.has_value() ||
		!m_ActiveRegistryBoundAuditionProvenance.has_value() ||
		m_ActiveDocument->strEffectAssetId !=
			m_ActiveRegistryBoundAuditionProvenance->strEffectAssetId)
	{
		strOutStatus.clear();
		return true;
	}

	const REGISTRY_BOUND_AUDITION_PROVENANCE& Provenance =
		*m_ActiveRegistryBoundAuditionProvenance;
	EFFECT_DIRECT_AUTHORED_SOURCE_ENTRY Expected;
	Expected.strEffectAssetId = Provenance.strEffectAssetId;
	Expected.Path = Provenance.DocumentPath;
	Expected.bRegistryBoundAuditionOnly = true;
	Expected.strSourceEffectAssetId = Provenance.strSourceEffectAssetId;
	Expected.SourceDocumentPath = Provenance.SourceDocumentPath;
	Expected.strSourceDocumentRawSha256 =
		Provenance.strSourceDocumentRawSha256;
	std::string CatalogStatus;
	if (!CEffectDirectAuthoredSourceIndex::
			Validate_RegistryBoundAuditionCatalogProvenanceFresh(
				CProjectDataRoot::Resolve(
					std::filesystem::path(L"Effects") / L"EffectCatalog.json"),
				CProjectDataRoot::Resolve(
					std::filesystem::path(L"Effects") /
					L"EffectAuditionCatalog.json"),
				CProjectDataRoot::Resolve(
					std::filesystem::path(L"Effects") / L"Authored"),
				Expected, CatalogStatus))
	{
		strOutStatus =
			"Live EffectCatalog provenance validation failed. " + CatalogStatus;
		return false;
	}
	const auto Current = m_DirectAuthoredEditableEntries.find(
		Provenance.strEffectAssetId);
	if (Current == m_DirectAuthoredEditableEntries.end() ||
		!Current->second.bRegistryBoundAuditionOnly)
	{
		strOutStatus =
			"Registry-bound audition metadata disappeared or was reclassified after this document was opened. Refresh Index, then reopen the exact audition row.";
		return false;
	}
	const DIRECT_AUTHORED_EDITABLE_ENTRY& Entry = Current->second;
	if (Entry.Path.lexically_normal() !=
			Provenance.DocumentPath.lexically_normal() ||
		Entry.strSourceEffectAssetId != Provenance.strSourceEffectAssetId ||
		Entry.SourceDocumentPath.lexically_normal() !=
			Provenance.SourceDocumentPath.lexically_normal() ||
		Entry.strSourceDocumentRawSha256 !=
			Provenance.strSourceDocumentRawSha256)
	{
		strOutStatus =
			"Registry-bound audition metadata changed after this document was opened. Reopen the audition row before Play or Save.";
		return false;
	}
	if (!Entry.bAuditionSourceFreshnessValid)
	{
		strOutStatus =
			"Registry-bound audition source pin was stale when the index was refreshed. Correct the source/hash pair, Refresh Index, and reopen the audition row.";
		return false;
	}
	return Validate_RegistryBoundAuditionSourceFreshness(
		Entry.SourceDocumentPath,
		Entry.strSourceDocumentRawSha256,
		strOutStatus);
}

bool_t Client::CEffect_Tool::Is_UnifiedEffectActive(
	const UNIFIED_EFFECT_CACHE& Cache) const
{
	return Cache.bValid && m_ActiveDocument.has_value() &&
		EFFECT_DOCUMENT_SOURCE::AUTHORED == m_eActiveDocumentSource &&
		m_ActiveDocument->strEffectAssetId == Cache.Document.strEffectAssetId &&
		!m_ActiveDocumentPath.empty() &&
		m_ActiveDocumentPath.lexically_normal() == Cache.Path.lexically_normal();
}

bool_t Client::CEffect_Tool::Validate_UnifiedEffectPreviewReadiness(
	const EFFECT_DOCUMENT_DESC& Document,
	std::string& strOutError) const
{
	if (Document.strEffectAssetId != ARTIST_F_UNIFIED_EFFECT_ASSET_ID)
	{
		const bool_t bHasVisibleCarrier = std::any_of(
			Document.Elements.begin(), Document.Elements.end(),
			[](const EFFECT_ELEMENT_DESC& Element)
			{ return Is_ElementPreviewAdmitted(Element); }) ||
			std::any_of(Document.ModelCues.begin(), Document.ModelCues.end(),
				[](const EFFECT_MODEL_CUE_DESC& Cue)
				{ return Cue.bVisible; });
		if (!bHasVisibleCarrier)
		{
			strOutError =
				"Candidate has no visible Element or Model / Summon cue to stage.";
			return false;
		}
		strOutError.clear();
		return true;
	}
	const std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> pProgram =
		CEffectCatalog::Find_ReconstructedRuntimeProgram(
			ARTIST_F_VISUAL_PROGRAM_ASSET_ID);
	if (nullptr == pProgram)
	{
		strOutError = "Artist F Track A Program metadata is unavailable.";
		return false;
	}
	EFFECT_ARTIST31470_UNIFIED_UPGRADE_STATS Stats;
	return CEffectDocumentCodec::
		Validate_Artist31470UnifiedAuthoredReadiness(
			*pProgram, Document, Stats, strOutError);
}

bool_t Client::CEffect_Tool::Try_LoadUnifiedElement(
	const UNIFIED_EFFECT_CACHE& Cache,
	const std::string& strElementId)
{
	if (!Cache.bValid || strElementId.empty())
		return false;
	std::string strEditableStatus;
	const std::filesystem::path* pEditablePath =
		Resolve_DirectAuthoredEditablePath(
			Cache.Document.strEffectAssetId, strEditableStatus);
	if (nullptr == pEditablePath)
	{
		m_strDocumentStatus = strEditableStatus;
		return false;
	}
	if (!Is_UnifiedEffectActive(Cache))
	{
		if (!Try_LoadDocumentPath(*pEditablePath,
				EFFECT_DOCUMENT_SOURCE::AUTHORED,
				Cache.Document.strEffectAssetId))
		{
			if (m_PendingDocumentLoad.has_value() &&
				m_PendingDocumentLoad->Path == *pEditablePath)
			{
				m_PendingDocumentLoad->strElementSelectionId = strElementId;
				m_PendingDocumentLoad->strModelCueSelectionId.clear();
			}
			return false;
		}
	}
	return Try_SelectElement(Cache.Document.strEffectAssetId, strElementId);
}

bool_t Client::CEffect_Tool::Try_LoadUnifiedModelCue(
	const UNIFIED_EFFECT_CACHE& Cache,
	const std::string& strCueId)
{
	if (!Cache.bValid || strCueId.empty())
		return false;
	std::string strEditableStatus;
	const std::filesystem::path* pEditablePath =
		Resolve_DirectAuthoredEditablePath(
			Cache.Document.strEffectAssetId, strEditableStatus);
	if (nullptr == pEditablePath)
	{
		m_strDocumentStatus = strEditableStatus;
		return false;
	}
	if (!Is_UnifiedEffectActive(Cache))
	{
		if (!Try_LoadDocumentPath(*pEditablePath,
				EFFECT_DOCUMENT_SOURCE::AUTHORED,
				Cache.Document.strEffectAssetId))
		{
			if (m_PendingDocumentLoad.has_value() &&
				m_PendingDocumentLoad->Path == *pEditablePath)
			{
				m_PendingDocumentLoad->strElementSelectionId.clear();
				m_PendingDocumentLoad->strModelCueSelectionId = strCueId;
			}
			return false;
		}
	}
	return Try_SelectModelCue(Cache.Document.strEffectAssetId, strCueId);
}

bool_t Client::CEffect_Tool::Try_PlayUnifiedAuthoringFamily(
	const std::string& strEffectAssetId,
	const EFFECT_AUTHORING_FAMILY eFamily)
{
	if (!m_ActiveDocument.has_value() ||
		m_ActiveDocument->strEffectAssetId != strEffectAssetId ||
		EFFECT_AUTHORING_FAMILY::END == eFamily)
	{
		m_strPreviewStatus =
			"Load this Effect before playing one Family.";
		return false;
	}
	if (std::none_of(m_ActiveDocument->Elements.begin(),
		m_ActiveDocument->Elements.end(),
		[eFamily](const EFFECT_ELEMENT_DESC& Element)
		{
			return Resolve_AuthoringFamily(Element) == eFamily &&
				Is_ElementPreviewAdmitted(Element);
		}))
	{
		m_strPreviewStatus =
			"The selected Family has no visible authoring-admitted Element to play. Hard-locked Elements remain editable; APPROXIMATE Elements are admitted for authoring preview.";
		return false;
	}
	const EFFECT_AUTHORING_FAMILY ePreviousFamily =
		m_ePreviewIsolationAuthoringFamily;
	m_ePreviewIsolationAuthoringFamily = eFamily;
	if (!Try_SetPreviewFilter(EFFECT_PREVIEW_FILTER::SOLO_AUTHORING_FAMILY))
	{
		m_ePreviewIsolationAuthoringFamily = ePreviousFamily;
		return false;
	}
	Start_WorldPreviewFromBeginning();
	return true;
}

bool_t Client::CEffect_Tool::Try_PlayUnifiedEffect(
	const UNIFIED_EFFECT_CACHE& Cache)
{
	if (!Cache.bValid)
	{
		m_strPreviewStatus = "The saved Skill Effect is unavailable.";
		return false;
	}
	std::string strEditableStatus;
	const std::filesystem::path* pEditablePath =
		Resolve_DirectAuthoredEditablePath(
			Cache.Document.strEffectAssetId, strEditableStatus);
	if (nullptr == pEditablePath)
	{
		m_strPreviewStatus = strEditableStatus;
		return false;
	}
	const bool_t bActive = Is_UnifiedEffectActive(Cache);
	std::string ActiveReadinessError;
	const bool_t bPreviewReady = bActive ?
		Validate_UnifiedEffectPreviewReadiness(
			*m_ActiveDocument, ActiveReadinessError) : Cache.bPreviewReady;
	if (!bPreviewReady)
	{
		const std::string& ReadinessError = bActive ?
			ActiveReadinessError : Cache.strPreviewReadinessError;
		m_strPreviewStatus = ReadinessError.empty() ?
			"The saved Skill Effect is not ready for preview." :
			ReadinessError;
		return false;
	}
	if (!bActive)
	{
		if (!Try_LoadDocumentPath(*pEditablePath,
				EFFECT_DOCUMENT_SOURCE::AUTHORED,
				Cache.Document.strEffectAssetId))
		{
			if (m_PendingDocumentLoad.has_value() &&
				m_PendingDocumentLoad->Path == *pEditablePath)
			{
				m_PendingDocumentLoad->strElementSelectionId.clear();
				m_PendingDocumentLoad->strModelCueSelectionId.clear();
				m_PendingDocumentLoad->bPlayCompleteAfterLoad = true;
			}
			return false;
		}
	}
	if (m_ActiveDocument && !m_ProductPreview &&
		(m_ActiveDocument->strEffectAssetId.ends_with(".restore") ||
		 Is_SceneAnchoredEffectAssetId(m_ActiveDocument->strEffectAssetId)))
		return Try_PlayRecoveryEffect();
	if (!Try_SetPreviewFilter(EFFECT_PREVIEW_FILTER::COMPLETE))
		return false;
	Start_WorldPreviewFromBeginning();
	return true;
}

bool_t Client::CEffect_Tool::Prepare_RecoveryPreviewTarget()
{
    if (!m_ActiveDocument || !m_pAuthoringSequencer)
    { m_strPreviewStatus = "Load the recovery Effect and its authoring workspace first."; return false; }
    const std::string assetId = m_ActiveDocument->strEffectAssetId;
    if (Is_WorldEffectAssetId(assetId))
    {
        const bool selected = m_pAuthoringSequencer->Select_WorldEffect(assetId);
        m_strPreviewStatus = m_pAuthoringSequencer->Status();
        return selected;
    }
    if (Is_KoukuEffectAssetId(assetId))
    {
        EFFECT_DOCUMENT_DESC draft = *m_ActiveDocument;
        if ((m_bParticleSystemDraftDirty && !Apply_ParticleSystemDraft(draft)) ||
            (m_bDetailDraftDirty && !Apply_DetailDraft(draft)) ||
            (m_bModelCueDraftDirty && !Apply_ModelCueDraft(draft)))
        { m_strPreviewStatus = "Player-anchor preview could not apply the current draft."; return false; }
        const auto requests = Collect_ToolSourceAnchorRequests(draft);
        const bool requiresSourceModel = std::any_of(requests.begin(), requests.end(), [](const auto& request)
            { return request.eOrientation != EFFECT_ATTACHMENT_ORIENTATION::CAMERA_VIEW; });
        const bool selected = m_pAuthoringSequencer->Select_KoukuEffect(assetId, requiresSourceModel, false, &draft);
        m_strPreviewStatus = m_pAuthoringSequencer->Status();
        return selected;
    }
    const auto binding = std::find_if(m_UnifiedCandidateBindings.begin(), m_UnifiedCandidateBindings.end(),
        [&assetId](const auto& row) { return row.strEffectAssetId == assetId; });
    if (binding == m_UnifiedCandidateBindings.end())
    { m_strPreviewStatus = "The recovery Effect has no indexed character/skill identity."; return false; }
    const char_t* modelAsset = Animation_AssetName(binding->eCharacterClass);
    if (!modelAsset)
    { m_strPreviewStatus = "The recovery Effect class has no admitted model."; return false; }
    const uint32_t skillId = static_cast<uint32_t>(binding->iSkillId);
    std::optional<uint32_t> stageIndex;
    std::string productOwnerId = assetId;
    const auto sourceEntry = m_DirectAuthoredEditableEntries.find(assetId);
    if (sourceEntry != m_DirectAuthoredEditableEntries.end() &&
        !sourceEntry->second.strSourceEffectAssetId.empty())
        productOwnerId = sourceEntry->second.strSourceEffectAssetId;
    const auto* skill = CPlayerSkillCatalog::Find_ById(binding->iSkillId);
    if (skill && skill->eSkillKind == LostArk::Shared::PLAYER_SKILL_KIND::COMBO)
    {
        // Select from actual Product clip ownership, never a BA filename suffix
        // or the metadata-only candidate's default stage index.
        for (const auto& owner : m_AllEffects)
        {
            if (owner.Skill.iSkillId != binding->iSkillId ||
                owner.Skill.eCharacterClass != binding->eCharacterClass) continue;
            for (const auto& cue : owner.ProductCues)
            {
                if (cue.Cue.strEffectAssetId != assetId &&
                    cue.Cue.strEffectAssetId != productOwnerId) continue;
                if (cue.iStageIndex >= skill->ComboStages.size() ||
                    (stageIndex && *stageIndex != cue.iStageIndex))
                { m_strPreviewStatus = "Recovery Effect belongs to ambiguous or stale COMBO stages."; return false; }
                stageIndex = static_cast<uint32_t>(cue.iStageIndex);
            }
        }
        if (!stageIndex)
        { m_strPreviewStatus = "This COMBO recovery needs an exact Product clip-stage binding."; return false; }
    }
    if (!m_pAuthoringSequencer->Select_CharacterSkill(modelAsset, skillId, stageIndex))
    { m_strPreviewStatus = m_pAuthoringSequencer->Status(); return false; }
    return true;
}

bool_t Client::CEffect_Tool::Try_PlayRecoveryEffect()
{
    if (!m_ActiveDocument || !m_pAuthoringSequencer)
    { m_strPreviewStatus = "Load the recovery Effect and its authoring workspace first."; return false; }
    const std::string assetId = m_ActiveDocument->strEffectAssetId;
    EFFECT_DOCUMENT_DESC preview = *m_ActiveDocument;
    if ((m_bParticleSystemDraftDirty && !Apply_ParticleSystemDraft(preview)) ||
        (m_bDetailDraftDirty && !Apply_DetailDraft(preview)) ||
        (m_bModelCueDraftDirty && !Apply_ModelCueDraft(preview)))
    { m_strPreviewStatus = "Recovery preview could not apply the current draft."; return false; }
    float durationSeconds = 1.f;
    for (const auto& element : preview.Elements)
        if (element.bVisible) durationSeconds = (std::max)(durationSeconds, Element_PreviewEndSeconds(element));
    for (const auto& cue : preview.ModelCues)
        if (cue.bVisible) durationSeconds = (std::max)(durationSeconds, cue.fStartDelaySeconds + cue.fDurationSeconds);
    const uint32_t duration = static_cast<uint32_t>(
        (std::clamp)(std::ceil(durationSeconds * 1000.f), 1.f, 600000.f));
    if (!Prepare_RecoveryPreviewTarget()) return false;
    // Prepare only the sequencer occurrence. A disposable complete World
    // Preview here duplicated model/texture staging before every Play All.
    const bool result = m_pAuthoringSequencer->Preview(
        {EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT, assetId}, duration);
    m_strPreviewStatus = m_pAuthoringSequencer->Status();
    if (!result) return false;
    Release_WorldPreview(true);
    Set_SynchronizedAnimationPaused(true);
    Reset_SynchronizedAnimationSequence();
    m_bPreviewPlaying = false;
    m_bPreviewVisibleRequested = false;
    m_ePreviewFilter = EFFECT_PREVIEW_FILTER::COMPLETE;
    m_fPreviewTimeSeconds = 0.f;
    m_fPreviewDurationSeconds = durationSeconds;
    return true;
}

bool_t Client::CEffect_Tool::Try_PreviewElementTimeline(const std::string& strElementId)
{
    if (m_ProductPreview)
    { m_strPreviewStatus = "Open an authored Effect before using Timeline Solo."; return false; }
    return Try_PreviewElementsTimeline({strElementId}, false);
}

bool_t Client::CEffect_Tool::Try_PlayMarkedElementGroup()
{
    if (!m_ActiveDocument || m_MarkedElementIds.empty())
    { m_strPreviewStatus = "Shift-click Element rows before Play Group."; return false; }
    const std::vector<std::string> selected(m_MarkedElementIds.begin(), m_MarkedElementIds.end());
    return Try_PreviewElementsTimeline(selected, true);
}

bool_t Client::CEffect_Tool::Try_PreviewElementsTimeline(
    const std::vector<std::string>& elementIds, const bool loop)
{
    if (!m_ActiveDocument || !m_pAuthoringSequencer || elementIds.empty())
    { m_strPreviewStatus = "Open an authored Effect and select elements before previewing."; return false; }
    if (!Validate_ActiveRegistryBoundAuditionFreshness(m_strPreviewStatus)) return false;
    EFFECT_DOCUMENT_DESC draft = *m_ActiveDocument;
    if ((m_bParticleSystemDraftDirty && !Apply_ParticleSystemDraft(draft)) ||
        (m_bDetailDraftDirty && !Apply_DetailDraft(draft)) ||
        (m_bModelCueDraftDirty && !Apply_ModelCueDraft(draft)))
    { m_strPreviewStatus = "Element preview could not apply the current draft."; return false; }
    EFFECT_DOCUMENT_DESC preview;
    if (!Build_ElementsPreviewDocument(draft, elementIds, preview, m_strPreviewStatus)) return false;
    uint32_t focus = 0u, duration = 0u;
    std::string label;
    if (!Resolve_ElementsPreviewWindow(preview, elementIds, focus, duration, label, m_strPreviewStatus)) return false;
    const EFFECT_RESOURCE_KEY key{EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT, preview.strEffectAssetId};
    // Selected projections include their hidden source providers. Keep the
    // captured root when Solo/Group changes while staging any new model need.
    if (Is_WorldEffectAssetId(key.strStableId))
    {
        if (!m_pAuthoringSequencer->Select_WorldEffect(key.strStableId, true))
        { m_strPreviewStatus = m_pAuthoringSequencer->Status(); return false; }
    }
    else if (Is_KoukuEffectAssetId(key.strStableId))
    {
        const auto requests = Collect_ToolSourceAnchorRequests(preview);
        const bool requiresSourceModel = std::any_of(requests.begin(), requests.end(), [](const auto& request)
            { return request.eOrientation != EFFECT_ATTACHMENT_ORIENTATION::CAMERA_VIEW; });
        if (!m_pAuthoringSequencer->Select_KoukuEffect(key.strStableId, requiresSourceModel, true, &preview))
        { m_strPreviewStatus = m_pAuthoringSequencer->Status(); return false; }
    }
    else if (key.strStableId.ends_with(".restore") &&
        !m_pAuthoringSequencer->Uses_Resource(key) && !Prepare_RecoveryPreviewTarget()) return false;
    const bool result = loop ?
        m_pAuthoringSequencer->Preview_Elements(key, elementIds, label, duration, focus, true) :
        m_pAuthoringSequencer->Preview_Element(key, elementIds.front(), label, duration, focus);
    m_strPreviewStatus = m_pAuthoringSequencer->Status();
    if (!result) return false;
    Release_WorldPreview(true);
    Set_SynchronizedAnimationPaused(true);
    Reset_SynchronizedAnimationSequence();
    m_bPreviewPlaying = false;
    m_bPreviewVisibleRequested = false;
    m_ePreviewFilter = EFFECT_PREVIEW_FILTER::SOLO_SELECTED;
    m_strPreviewIsolationElementId = elementIds.size() == 1u ? elementIds.front() : std::string{};
    m_strPreviewIsolationGroupId.clear();
    m_fPreviewTimeSeconds = static_cast<float>(focus) * .001f;
    m_fPreviewDurationSeconds = static_cast<float>(duration) * .001f;
    return true;
}

void Client::CEffect_Tool::Render_RecoveryEffectForProduct(const std::string& strProductEffectId)
{
    const std::string_view suffix = strProductEffectId.ends_with(".unified") ? ".unified" : ".restore";
    if (!strProductEffectId.ends_with(suffix)) return;
    const std::string stem = strProductEffectId.substr(0, strProductEffectId.size() - suffix.size());
    // Snapshot IDs because Play/Open Editor may refresh the binding/cache vectors.
    const std::array<std::string, 3> recoveryIds = {stem + ".restore", stem + ".full.restore", stem + ".tuning.restore"};
    bool headingShown = false;
    for (const std::string& recoveryId : recoveryIds)
    {
        if (recoveryId == strProductEffectId) continue;
        const auto binding = std::find_if(m_UnifiedCandidateBindings.begin(), m_UnifiedCandidateBindings.end(),
            [&recoveryId](const auto& row) { return row.strEffectAssetId == recoveryId; });
        if (binding == m_UnifiedCandidateBindings.end()) continue;
        auto cache = m_UnifiedCandidateCaches.find(recoveryId);
        if (cache == m_UnifiedCandidateCaches.end()) continue;
        const UNIFIED_EFFECT_CANDIDATE_BINDING selectedBinding = *binding;
        if (!headingShown)
        {
            ImGui::SeparatorText("Recovery Effect");
            ImGui::TextDisabled("Independent preview. The Product skill still uses its current Effect.");
            headingShown = true;
        }
        const std::string name = cache->second.bValid && !cache->second.Document.strDisplayName.empty() ?
            cache->second.Document.strDisplayName : recoveryId;
        ImGui::PushID(recoveryId.c_str());
        ImGui::TextUnformatted(name.c_str());
        // Listing sibling recoveries must not decode every full composition.
        // Each command keeps its existing exact-file validation and failure path.
        if (ImGui::SmallButton("Play")) (void)Try_PlaySavedUnifiedEffect(selectedBinding);
        ImGui::SameLine();
        if (ImGui::SmallButton("Open Editor")) (void)Try_LoadDocument(recoveryId);
        cache = m_UnifiedCandidateCaches.find(recoveryId);
        if (cache != m_UnifiedCandidateCaches.end())
        {
            ImGui::SameLine();
            if (ImGui::SmallButton(cache->second.bObserved ? "Refresh Elements" : "Show Elements"))
                (void)Refresh_UnifiedEffectCache(cache->second, selectedBinding.Path, recoveryId);
            if (cache->second.bValid)
                Render_UnifiedEffectTree(cache->second, name);
            else if (cache->second.bObserved)
                ImGui::TextWrapped("Recovery unavailable (%s): %s", recoveryId.c_str(), cache->second.strStatus.c_str());
        }
        ImGui::PopID();
    }
}

bool_t Client::CEffect_Tool::Try_PlayActiveUnifiedEffect()
{
	if (!m_ActiveDocument.has_value())
	{
		m_strPreviewStatus = "No saved Effect is loaded for preview.";
		return false;
	}
	if (!m_bActiveDocumentDrawable)
	{
		m_strPreviewStatus = m_strActiveDocumentDrawableError.empty() ?
			"The saved Effect is not drawable." :
			m_strActiveDocumentDrawableError;
		return false;
	}
	std::string FreshnessStatus;
	if (!Validate_ActiveRegistryBoundAuditionFreshness(FreshnessStatus))
	{
		m_strPreviewStatus =
			"Play rejected: registry-bound audition source freshness failed. " +
			FreshnessStatus;
		return false;
	}
	std::string ReadinessError;
	if (!Validate_UnifiedEffectPreviewReadiness(
			*m_ActiveDocument, ReadinessError))
	{
		m_strPreviewStatus = ReadinessError.empty() ?
			"The saved Effect is not ready for preview." : ReadinessError;
		return false;
	}
	if (m_ActiveDocument && !m_ProductPreview &&
		(m_ActiveDocument->strEffectAssetId.ends_with(".restore") ||
		 Is_SceneAnchoredEffectAssetId(m_ActiveDocument->strEffectAssetId)))
		return Try_PlayRecoveryEffect();
	if (!Try_SetPreviewFilter(EFFECT_PREVIEW_FILTER::COMPLETE))
		return false;
	Start_WorldPreviewFromBeginning();
	return true;
}

bool_t Client::CEffect_Tool::Try_PlaySavedUnifiedEffect(
	const UNIFIED_EFFECT_CANDIDATE_BINDING& Binding)
{
	const auto Cache = m_UnifiedCandidateCaches.find(
		Binding.strEffectAssetId);
	if (Cache == m_UnifiedCandidateCaches.end())
	{
		m_strPreviewStatus =
			"The saved authored Effect is no longer in the source catalog index.";
		return false;
	}
	if (!Refresh_UnifiedEffectCache(Cache->second, Binding.Path,
			Binding.strEffectAssetId))
	{
		m_strPreviewStatus = Cache->second.strStatus;
		return false;
	}
	return Try_PlayUnifiedEffect(Cache->second);
}

bool_t Client::CEffect_Tool::Try_PlayUnifiedModelCues(
	const std::string& strEffectAssetId)
{
	if (!m_ActiveDocument.has_value() ||
		m_ActiveDocument->strEffectAssetId != strEffectAssetId ||
		m_ActiveDocument->ModelCues.empty())
	{
		m_strPreviewStatus =
			"Load this Effect before playing Model / Summon.";
		return false;
	}
	if (!Try_SetPreviewFilter(EFFECT_PREVIEW_FILTER::SOLO_MODEL_CUES))
		return false;
	Start_WorldPreviewFromBeginning();
	return true;
}
