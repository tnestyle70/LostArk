#include "Effect_DirectAuthoredSourceIndex.h"
#include "ProjectDataRoot.h"
#include "ValtanPatternEffectCueAuthoring.h"
#include "ValtanPatternTree.h"

#include <algorithm>
#include <array>
#include <bit>
#include <filesystem>
#include <iostream>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

namespace
{
	using namespace Client;

	void Require(const bool_t bCondition, const char* const pMessage)
	{
		if (!bCondition)
			throw std::runtime_error(pMessage);
	}

	struct CUE_SNAPSHOT final
	{
		std::string strLocation;
		std::array<std::string, 13u> Strings;
		std::array<uint32_t, 15u> Numbers{};

		bool operator==(const CUE_SNAPSHOT&) const = default;
	};

	CUE_SNAPSHOT CaptureCue(
		const std::string& strLocation,
		const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue)
	{
		CUE_SNAPSHOT Result;
		Result.strLocation = strLocation;
		Result.Strings = {
			Cue.strBindingId,
			Cue.strOccurrenceId,
			Cue.strPatternId,
			Cue.strStageId,
			Cue.strActionId,
			Cue.strClipOccurrenceId,
			Cue.strEffectAssetId,
			Cue.strV1EffectAssetId,
			Cue.strAnchorSlotId,
			Cue.strFollowPolicy,
			Cue.strStopPolicy,
			Cue.strRepeatPolicy,
			Cue.strScalePolicy,
		};
		Result.Numbers = {
			static_cast<uint32_t>(Cue.eFollowPolicy),
			static_cast<uint32_t>(Cue.eStopPolicy),
			static_cast<uint32_t>(Cue.eScalePolicy),
			Cue.bHasExplicitScalePolicy ? 1u : 0u,
			Cue.bUsesStageClock ? 1u : 0u,
			Cue.iStageOffsetMs,
			Cue.iSourceStartMs,
			Cue.iSourceEndMs,
			Cue.iStageDurationMs,
			Cue.bHasSourceEnd ? 1u : 0u,
			std::bit_cast<uint32_t>(Cue.vWorldScale.x),
			std::bit_cast<uint32_t>(Cue.vWorldScale.y),
			std::bit_cast<uint32_t>(Cue.vWorldScale.z),
			std::bit_cast<uint32_t>(Cue.LocalTransform.vPosition.x),
			std::bit_cast<uint32_t>(Cue.LocalTransform.vPosition.y),
		};
		/* Fold the remaining transform values into the location without losing
		   their exact bit identity. This keeps the snapshot compact while still
		   detecting any mutation outside the requested cue. */
		for (const float Value : {
			Cue.LocalTransform.vPosition.z,
			Cue.LocalTransform.vRotationDegrees.x,
			Cue.LocalTransform.vRotationDegrees.y,
			Cue.LocalTransform.vRotationDegrees.z,
			Cue.LocalTransform.vScale.x,
			Cue.LocalTransform.vScale.y,
			Cue.LocalTransform.vScale.z })
		{
			Result.strLocation += ":" +
				std::to_string(std::bit_cast<uint32_t>(Value));
		}
		return Result;
	}

	std::vector<CUE_SNAPSHOT> CaptureCueGraph(
		const VALTAN_PATTERN_TREE_VIEW& Tree)
	{
		std::vector<CUE_SNAPSHOT> Result;
		for (const auto* const pGroup : { &Tree.Gimmicks, &Tree.Rotation })
		{
			for (const VALTAN_PATTERN_VIEW& Pattern : *pGroup)
			{
				for (const VALTAN_STAGE_VIEW& Stage : Pattern.Stages)
				{
					const std::string Prefix = Pattern.strPatternId + "/" +
						Stage.strStageId;
					for (std::size_t iCue = 0u;
						iCue < Stage.ProductCues.size(); ++iCue)
					{
						Result.push_back(CaptureCue(
							Prefix + "/flat/" + std::to_string(iCue),
							Stage.ProductCues[iCue]));
					}
					for (std::size_t iClip = 0u;
						iClip < Stage.ClipOccurrences.size(); ++iClip)
					{
						const VALTAN_CLIP_OCCURRENCE_VIEW& Clip =
							Stage.ClipOccurrences[iClip];
						for (std::size_t iCue = 0u;
							iCue < Clip.ProductCues.size(); ++iCue)
						{
							Result.push_back(CaptureCue(
								Prefix + "/nested/" +
								Clip.strClipOccurrenceId + "/" +
								std::to_string(iCue),
								Clip.ProductCues[iCue]));
						}
					}
					if (Stage.ProductCue.has_value())
					{
						Result.push_back(CaptureCue(
							Prefix + "/primary", *Stage.ProductCue));
					}
				}
			}
		}
		return Result;
	}

	std::vector<EFFECT_DIRECT_AUTHORED_SCANNED_FILE> ScanAuthoredEffects(
		const std::filesystem::path& AuthoredRoot)
	{
		constexpr std::string_view Suffix = ".effect.json";
		std::vector<EFFECT_DIRECT_AUTHORED_SCANNED_FILE> Result;
		std::error_code Error;
		for (std::filesystem::recursive_directory_iterator Iterator(
				 AuthoredRoot,
				 std::filesystem::directory_options::skip_permission_denied,
				 Error), End;
			!Error && Iterator != End; Iterator.increment(Error))
		{
			if (!Iterator->is_regular_file(Error) || Error)
				continue;
			const std::string Name = Iterator->path().filename().string();
			if (!Name.ends_with(Suffix))
				continue;
			EFFECT_DIRECT_AUTHORED_SCANNED_FILE File;
			File.strEffectAssetId = Name.substr(
				0u, Name.size() - Suffix.size());
			File.Path = Iterator->path();
			Result.push_back(std::move(File));
		}
		Require(!Error, "Effect authored source scan failed");
		Require(!Result.empty(), "Effect authored source scan was empty");
		return Result;
	}

	const VALTAN_STAGE_VIEW* FindStage(
		const VALTAN_PATTERN_TREE_VIEW& Tree,
		const std::string& strPatternId,
		const std::string& strStageId)
	{
		for (const auto* const pGroup : { &Tree.Gimmicks, &Tree.Rotation })
		{
			const auto Pattern = std::find_if(
				pGroup->begin(), pGroup->end(),
				[&](const VALTAN_PATTERN_VIEW& Candidate)
				{
					return Candidate.strPatternId == strPatternId;
				});
			if (pGroup->end() == Pattern)
				continue;
			const auto Stage = std::find_if(
				Pattern->Stages.begin(), Pattern->Stages.end(),
				[&](const VALTAN_STAGE_VIEW& Candidate)
				{
					return Candidate.strStageId == strStageId;
				});
			return Pattern->Stages.end() == Stage ? nullptr : &*Stage;
		}
		return nullptr;
	}

	VALTAN_STAGE_VIEW* FindStage(
		VALTAN_PATTERN_TREE_VIEW& Tree,
		const std::string& strPatternId,
		const std::string& strStageId)
	{
		for (auto* const pGroup : { &Tree.Gimmicks, &Tree.Rotation })
		{
			const auto Pattern = std::find_if(
				pGroup->begin(), pGroup->end(),
				[&](const VALTAN_PATTERN_VIEW& Candidate)
				{
					return Candidate.strPatternId == strPatternId;
				});
			if (pGroup->end() == Pattern)
				continue;
			const auto Stage = std::find_if(
				Pattern->Stages.begin(), Pattern->Stages.end(),
				[&](const VALTAN_STAGE_VIEW& Candidate)
				{
					return Candidate.strStageId == strStageId;
				});
			return Pattern->Stages.end() == Stage ? nullptr : &*Stage;
		}
		return nullptr;
	}

	struct STAGE_SELECTION final
	{
		std::string strPatternId;
		std::string strStageId;
		std::string strActionId;
		std::string strFirstClipOccurrenceId;
		uint32_t iFirstSourceStartMs = 0u;
		std::string strUpdateClipOccurrenceId;
		uint32_t iUpdateSourceStartMs = 0u;
		bool_t bMovesClip = false;
	};

	STAGE_SELECTION SelectStage(const VALTAN_PATTERN_TREE_VIEW& Tree)
	{
		const VALTAN_PATTERN_VIEW* pFallbackPattern = nullptr;
		const VALTAN_STAGE_VIEW* pFallbackStage = nullptr;
		for (const auto* const pGroup : { &Tree.Gimmicks, &Tree.Rotation })
		{
			for (const VALTAN_PATTERN_VIEW& Pattern : *pGroup)
			{
				if (!Pattern.bAuthoringMasterManaged)
					continue;
				for (const VALTAN_STAGE_VIEW& Stage : Pattern.Stages)
				{
					if ("WAIT" == Stage.strSequenceRole ||
						Stage.bSuppressAnimation ||
						Stage.ClipOccurrences.empty() ||
						Stage.ProductCues.size() >= 128u)
					{
						continue;
					}
					if (nullptr == pFallbackStage)
					{
						pFallbackPattern = &Pattern;
						pFallbackStage = &Stage;
					}
					if (Stage.ClipOccurrences.size() >= 2u)
					{
						pFallbackPattern = &Pattern;
						pFallbackStage = &Stage;
						break;
					}
				}
				if (nullptr != pFallbackStage &&
					pFallbackStage->ClipOccurrences.size() >= 2u)
				{
					break;
				}
			}
			if (nullptr != pFallbackStage &&
				pFallbackStage->ClipOccurrences.size() >= 2u)
			{
				break;
			}
		}
		Require(nullptr != pFallbackPattern && nullptr != pFallbackStage,
			"canonical graph has no editable animation Stage for Effect cue oracle");

		const VALTAN_CLIP_OCCURRENCE_VIEW& First =
			pFallbackStage->ClipOccurrences.front();
		const VALTAN_CLIP_OCCURRENCE_VIEW& Update =
			pFallbackStage->ClipOccurrences.size() >= 2u ?
			pFallbackStage->ClipOccurrences[1u] : First;
		return {
			pFallbackPattern->strPatternId,
			pFallbackStage->strStageId,
			pFallbackStage->strActionId,
			First.strClipOccurrenceId,
			First.iSourceStartMs,
			Update.strClipOccurrenceId,
			Update.iSourceStartMs,
			First.strClipOccurrenceId != Update.strClipOccurrenceId,
		};
	}

	std::pair<std::string, std::string> MakeUniqueCueIdentity(
		const VALTAN_PATTERN_TREE_VIEW& Tree)
	{
		std::set<std::string, std::less<>> CueIds;
		std::set<std::string, std::less<>> OccurrenceIds;
		for (const CUE_SNAPSHOT& Snapshot : CaptureCueGraph(Tree))
		{
			CueIds.insert(Snapshot.Strings[0u]);
			OccurrenceIds.insert(Snapshot.Strings[1u]);
		}
		for (uint32_t iOrdinal = 1u; iOrdinal <= 9999u; ++iOrdinal)
		{
			const std::string CueId =
				"cue.valtan.composition.native-oracle." +
				std::to_string(iOrdinal);
			const std::string OccurrenceId =
				CueId + ".occurrence.01";
			if (!CueIds.contains(CueId) &&
				!OccurrenceIds.contains(OccurrenceId))
			{
				return { CueId, OccurrenceId };
			}
		}
		throw std::runtime_error(
			"Effect cue oracle exhausted its stable identity namespace");
	}

	void VerifyEffectCueAuthoringContract()
	{
		VALTAN_PATTERN_TREE_VIEW Tree;
		std::string Status;
		if (!CValtanPatternTree::Load(Tree, Status))
			throw std::runtime_error(Status);

		const std::filesystem::path CatalogPath = CProjectDataRoot::Resolve(
			std::filesystem::path(L"Effects") / L"EffectCatalog.json");
		const std::filesystem::path AuthoredRoot = CProjectDataRoot::Resolve(
			std::filesystem::path(L"Effects") / L"Authored");
		EFFECT_DIRECT_AUTHORED_SOURCE_INDEX SourceIndex;
		const auto ScannedFiles = ScanAuthoredEffects(AuthoredRoot);
		if (!CEffectDirectAuthoredSourceIndex::Build(
				CatalogPath, AuthoredRoot, ScannedFiles, {}, {}, {},
				SourceIndex, Status))
		{
			throw std::runtime_error(Status);
		}
		std::set<std::string, std::less<>> SourceEffectIds;
		for (const EFFECT_DIRECT_AUTHORED_SOURCE_ENTRY& Entry :
			SourceIndex.Entries)
		{
			SourceEffectIds.insert(Entry.strEffectAssetId);
		}
		const auto Effect = std::find_if(
			SourceEffectIds.begin(), SourceEffectIds.end(),
			[](const std::string& EffectAssetId)
			{
				return 0u == EffectAssetId.rfind("effect.valtan.", 0u);
			});
		Require(SourceEffectIds.end() != Effect,
			"physical Effect catalog has no admitted effect.valtan.* source row");

		const STAGE_SELECTION Selection = SelectStage(Tree);
		const auto [CueId, OccurrenceId] = MakeUniqueCueIdentity(Tree);
		VALTAN_PRODUCT_EFFECT_CUE_VIEW Cue;
		Cue.strBindingId = CueId;
		Cue.strOccurrenceId = OccurrenceId;
		Cue.strPatternId = Selection.strPatternId;
		Cue.strStageId = Selection.strStageId;
		Cue.strActionId = Selection.strActionId;
		Cue.strClipOccurrenceId = Selection.strFirstClipOccurrenceId;
		Cue.strEffectAssetId = *Effect;
		Cue.strAnchorSlotId = "root";
		Cue.strFollowPolicy = "follow";
		Cue.eFollowPolicy = EFFECT_FOLLOW_POLICY::FOLLOW;
		Cue.strStopPolicy = "natural";
		Cue.eStopPolicy = EFFECT_STOP_POLICY::NATURAL;
		Cue.strRepeatPolicy = "once";
		Cue.strScalePolicy = "OWNER_RELATIVE";
		Cue.eScalePolicy =
			VALTAN_PATTERN_EFFECT_SCALE_POLICY::OWNER_RELATIVE;
		Cue.vWorldScale = { 1.f, 1.f, 1.f };
		Cue.bHasExplicitScalePolicy = true;
		Cue.LocalTransform.vScale = { 1.f, 1.f, 1.f };
		Cue.iSourceStartMs = Selection.iFirstSourceStartMs;

		VALTAN_EFFECT_CUE_AUTHORING_CONTEXT Context;
		Context.QuerySourceMembership =
			[&SourceEffectIds](const std::string& EffectAssetId,
				bool_t& bOutContains, std::string& strOutStatus)
			{
				bOutContains = SourceEffectIds.contains(EffectAssetId);
				strOutStatus = bOutContains ?
					"Effect source index contains the requested ID." :
					"Effect source index does not contain the requested ID.";
				return true;
			};

		const std::vector<CUE_SNAPSHOT> OriginalGraph = CaptureCueGraph(Tree);
		bool_t bDirty = false;
		bool_t bChanged = false;
		Require(!CValtanPatternEffectCueAuthoring::Add(
				Tree, Selection.strPatternId, Selection.strStageId,
				Selection.strActionId, Cue, Context, bChanged, Status) &&
			!bChanged && !bDirty && OriginalGraph == CaptureCueGraph(Tree),
			"DISPLAY_ONLY admission mutated the Effect cue graph or dirty state");

		Context.eAdmission = VALTAN_EFFECT_CUE_AUTHORING_ADMISSION::ADMITTED;
		Require(CValtanPatternEffectCueAuthoring::Add(
				Tree, Selection.strPatternId, Selection.strStageId,
				Selection.strActionId, Cue, Context, bChanged, Status) && bChanged,
			"admitted Effect cue Add failed");
		bDirty = bDirty || bChanged;
		Require(bDirty, "successful Effect cue Add did not dirty its owner");
		const VALTAN_STAGE_VIEW* pStage = FindStage(
			Tree, Selection.strPatternId, Selection.strStageId);
		Require(nullptr != pStage &&
			CValtanPatternEffectCueAuthoring::Validate_Mirrors(*pStage, Status),
			"Effect cue Add did not preserve flat/nested mirrors");

		bDirty = false;
		const std::vector<CUE_SNAPSHOT> AddedGraph = CaptureCueGraph(Tree);
		Require(!CValtanPatternEffectCueAuthoring::Add(
				Tree, Selection.strPatternId, Selection.strStageId,
				Selection.strActionId, Cue, Context, bChanged, Status) &&
			!bChanged && !bDirty && AddedGraph == CaptureCueGraph(Tree),
			"duplicate Effect cue Add changed graph or dirty state");

		VALTAN_PRODUCT_EFFECT_CUE_VIEW UpdatedCue = Cue;
		UpdatedCue.strClipOccurrenceId = Selection.strUpdateClipOccurrenceId;
		UpdatedCue.iSourceStartMs = Selection.iUpdateSourceStartMs;
		UpdatedCue.LocalTransform.vRotationDegrees.y = 37.5f;
		Require(!CValtanPatternEffectCueAuthoring::Update(
				Tree, Selection.strPatternId, Selection.strStageId,
				Selection.strActionId, CueId + ".missing", OccurrenceId,
				UpdatedCue, Context, bChanged, Status) && !bChanged &&
			!bDirty && AddedGraph == CaptureCueGraph(Tree),
			"missing Effect cue update predecessor changed graph or dirty state");

		VALTAN_PATTERN_TREE_VIEW MissingNestedTree = Tree;
		VALTAN_STAGE_VIEW* pMissingNestedStage = FindStage(
			MissingNestedTree, Selection.strPatternId, Selection.strStageId);
		Require(nullptr != pMissingNestedStage,
			"nested predecessor fixture could not resolve its Stage");
		std::size_t iRemovedNested = 0u;
		for (VALTAN_CLIP_OCCURRENCE_VIEW& Clip :
			pMissingNestedStage->ClipOccurrences)
		{
			const std::size_t iBefore = Clip.ProductCues.size();
			std::erase_if(
				Clip.ProductCues,
				[&](const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Nested)
				{
					return Nested.strBindingId == CueId &&
						Nested.strOccurrenceId == OccurrenceId;
				});
			iRemovedNested += iBefore - Clip.ProductCues.size();
		}
		Require(1u == iRemovedNested,
			"nested predecessor fixture did not remove exactly one mirror");
		const std::vector<CUE_SNAPSHOT> MissingNestedGraph =
			CaptureCueGraph(MissingNestedTree);
		bChanged = false;
		Require(!CValtanPatternEffectCueAuthoring::Update(
				MissingNestedTree, Selection.strPatternId, Selection.strStageId,
				Selection.strActionId, CueId, OccurrenceId, UpdatedCue,
				Context, bChanged, Status) && !bChanged &&
			MissingNestedGraph == CaptureCueGraph(MissingNestedTree),
			"missing nested update predecessor changed the corrupted input graph");
		VALTAN_PATTERN_TREE_VIEW MissingNestedRemoveTree = MissingNestedTree;
		bChanged = false;
		Require(!CValtanPatternEffectCueAuthoring::Remove(
				MissingNestedRemoveTree, Selection.strPatternId,
				Selection.strStageId, Selection.strActionId, CueId,
				OccurrenceId, Cue.strEffectAssetId,
				Cue.strClipOccurrenceId, Context, bChanged, Status) &&
			!bChanged && MissingNestedGraph ==
				CaptureCueGraph(MissingNestedRemoveTree),
			"missing nested remove predecessor changed the corrupted input graph");

		Require(CValtanPatternEffectCueAuthoring::Update(
				Tree, Selection.strPatternId, Selection.strStageId,
				Selection.strActionId, CueId, OccurrenceId, UpdatedCue,
				Context, bChanged, Status) && bChanged,
			"admitted Effect cue Update failed");
		bDirty = bDirty || bChanged;
		pStage = FindStage(Tree, Selection.strPatternId, Selection.strStageId);
		Require(bDirty && nullptr != pStage &&
			CValtanPatternEffectCueAuthoring::Validate_Mirrors(*pStage, Status),
			"Effect cue Update did not preserve owner dirty state and mirrors");
		std::size_t iUpdatedNestedMatches = 0u;
		for (const VALTAN_CLIP_OCCURRENCE_VIEW& Clip :
			pStage->ClipOccurrences)
		{
			for (const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Nested : Clip.ProductCues)
			{
				if (Nested.strBindingId == CueId &&
					Nested.strOccurrenceId == OccurrenceId)
				{
					++iUpdatedNestedMatches;
					Require(
						Clip.strClipOccurrenceId ==
							Selection.strUpdateClipOccurrenceId &&
						Nested.strClipOccurrenceId ==
							Selection.strUpdateClipOccurrenceId,
						"Effect cue Update did not move its exact nested clip mirror");
				}
			}
		}
		Require(1u == iUpdatedNestedMatches,
			"Effect cue Update did not leave exactly one nested mirror");

		bDirty = false;
		const std::vector<CUE_SNAPSHOT> UpdatedGraph = CaptureCueGraph(Tree);
		Require(!CValtanPatternEffectCueAuthoring::Remove(
				Tree, Selection.strPatternId, Selection.strStageId,
				Selection.strActionId, CueId, OccurrenceId,
				UpdatedCue.strEffectAssetId + ".missing",
				UpdatedCue.strClipOccurrenceId, Context, bChanged, Status) &&
			!bChanged && !bDirty && UpdatedGraph == CaptureCueGraph(Tree),
			"missing Effect cue remove predecessor changed graph or dirty state");

		Require(CValtanPatternEffectCueAuthoring::Remove(
				Tree, Selection.strPatternId, Selection.strStageId,
				Selection.strActionId, CueId, OccurrenceId,
				UpdatedCue.strEffectAssetId,
				UpdatedCue.strClipOccurrenceId, Context, bChanged, Status) &&
			bChanged,
			"admitted Effect cue Remove failed");
		bDirty = bDirty || bChanged;
		Require(bDirty && OriginalGraph == CaptureCueGraph(Tree),
			"Effect cue Add/Update/Remove did not restore the original graph");

		std::cout << "ValtanPatternEffectCueAuthoringContractTests: 9/9 passed"
			<< (Selection.bMovesClip ? " (cross-clip update)\n" :
				" (single-clip retime update)\n");
	}
}

int Run_ValtanPatternEffectCueAuthoringContractTests()
{
	try
	{
		VerifyEffectCueAuthoringContract();
		return 0;
	}
	catch (const std::exception& Error)
	{
		std::cerr << "FAIL Valtan Effect cue native authoring contract: " <<
			Error.what() << '\n';
		return 1;
	}
}
