#include "imgui.h"

#include "ActionCompositionWorkbench.h"

#include "Animation_Tool.h"
#include "BalanceTool.h"
#include "BossTool.h"
#include "CameraTool.h"
#include "Effect_Catalog.h"
#include "Effect_Tool.h"
#include "EffectV2_Catalog.h"
#include "MainApp.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <functional>
#include <limits>
#include <numeric>
#include <sstream>
#include <string_view>

namespace
{
	constexpr float TIMELINE_LANE_LABEL_WIDTH = 112.f;
	constexpr float TIMELINE_ROW_HEIGHT = 24.f;
	/* A malformed/reference-only row must never turn one ImGui ruler into an
	   unbounded main-thread loop. Product authoring already caps each edited
	   clock at ten minutes; the renderer applies the same defensive ceiling. */
	constexpr uint32_t MAX_TIMELINE_RENDER_DURATION_MS = 600000u;

	struct COMPOSITION_DEFAULT_LAYOUT final
	{
		ImVec2 PatternsPos;
		ImVec2 PatternsSize;
		ImVec2 PreviewPos;
		ImVec2 PreviewSize;
		ImVec2 SequencerPos;
		ImVec2 SequencerSize;
		ImVec2 DetailsPos;
		ImVec2 DetailsSize;
		ImVec2 ResourcesPos;
		ImVec2 ResourcesSize;
		ImVec2 SessionPos;
		ImVec2 SessionSize;
	};

	COMPOSITION_DEFAULT_LAYOUT BuildCompositionDefaultLayout(
		const ImGuiViewport* const pViewport)
	{
		const ImVec2 origin = nullptr == pViewport ?
			ImVec2(20.f, 20.f) : pViewport->WorkPos;
		const ImVec2 available = nullptr == pViewport ?
			ImVec2(1600.f, 900.f) : pViewport->WorkSize;
		constexpr float margin = 8.f;
		constexpr float gap = 8.f;
		const float width = (std::max)(available.x, 1.f);
		const float height = (std::max)(available.y, 1.f);
		const float contentWidth = (std::max)(
			1.f, width - margin * 2.f - gap * 2.f);
		const float leftWidth = contentWidth * 0.21f;
		const float rightWidth = contentWidth * 0.25f;
		const float centerWidth = contentWidth - leftWidth - rightWidth;
		const float contentHeight = (std::max)(
			1.f, height - margin * 2.f);
		const float leftTopHeight = contentHeight * 0.58f;
		const float leftBottomHeight = (std::max)(
			1.f, contentHeight - leftTopHeight - gap);
		const float centerTopHeight = contentHeight * 0.30f;
		const float sessionHeight = contentHeight * 0.18f;
		const float sequencerHeight = (std::max)(
			1.f, contentHeight - centerTopHeight - sessionHeight - gap * 2.f);

		const float leftX = origin.x + margin;
		const float centerX = leftX + leftWidth + gap;
		const float rightX = centerX + centerWidth + gap;
		const float topY = origin.y + margin;
		COMPOSITION_DEFAULT_LAYOUT layout;
		layout.PatternsPos = ImVec2(leftX, topY);
		layout.PatternsSize = ImVec2(leftWidth, leftTopHeight);
		layout.ResourcesPos = ImVec2(
			leftX, topY + leftTopHeight + gap);
		layout.ResourcesSize = ImVec2(leftWidth, leftBottomHeight);
		layout.PreviewPos = ImVec2(centerX, topY);
		layout.PreviewSize = ImVec2(centerWidth, centerTopHeight);
		layout.SequencerPos = ImVec2(
			centerX, topY + centerTopHeight + gap);
		layout.SequencerSize = ImVec2(centerWidth, sequencerHeight);
		layout.SessionPos = ImVec2(
			centerX, layout.SequencerPos.y + sequencerHeight + gap);
		layout.SessionSize = ImVec2(centerWidth, sessionHeight);
		layout.DetailsPos = ImVec2(rightX, topY);
		layout.DetailsSize = ImVec2(rightWidth, contentHeight);
		return layout;
	}

	std::string Lower(std::string value)
	{
		std::transform(
			value.begin(), value.end(), value.begin(),
			[](const unsigned char character)
			{
				return static_cast<char>(std::tolower(character));
			});
		return value;
	}

	bool ContainsInsensitive(
		const std::string_view text,
		const std::string_view query)
	{
		if (query.empty())
			return true;
		return std::string::npos != Lower(std::string(text)).find(
			Lower(std::string(query)));
	}

	template <typename T>
	T SaturatingU32(const T value)
	{
		return (std::min)(
			value,
			static_cast<T>((std::numeric_limits<uint32_t>::max)()));
	}

	std::string BuildCompositionSlotId(
		const std::string& patternId,
		const std::string& stageId,
		const std::size_t ordinal)
	{
		std::ostringstream suffix;
		suffix << ".composition.clip." << std::setw(2) << std::setfill('0') <<
			ordinal + 1u;
		const std::string readable = patternId + "." + stageId + suffix.str();
		if (readable.size() <= 160u)
			return readable;
		uint64_t hash = 1469598103934665603ull;
		for (const unsigned char ch : patternId + "/" + stageId)
		{
			hash ^= ch;
			hash *= 1099511628211ull;
		}
		std::ostringstream compact;
		compact << "valtan.composition." << std::hex << std::setw(16) <<
			std::setfill('0') << hash << std::dec << suffix.str();
		return compact.str();
	}

	std::string BuildNextCompositionSlotId(
		const std::string& patternId,
		const std::string& stageId,
		const std::vector<Client::CBalanceTool::ANIMATION_SLOT_EDIT>& Slots)
	{
		/* A slot may be removed or reordered before another Sequence is
		   appended.  Current vector size is therefore not an identity allocator:
		   scan the bounded size+1 candidate set and choose an actually free ID. */
		for (std::size_t ordinal = 0u; ordinal <= Slots.size(); ++ordinal)
		{
			const std::string candidate = BuildCompositionSlotId(
				patternId, stageId, ordinal);
			const bool_t exists = std::any_of(
				Slots.begin(), Slots.end(),
				[&candidate](const Client::CBalanceTool::ANIMATION_SLOT_EDIT& Slot)
				{ return Slot.clipOccurrenceId == candidate; });
			if (!exists)
				return candidate;
		}
		return {};
	}

	std::vector<std::string> SplitResourcePath(const std::string_view value)
	{
		std::vector<std::string> Segments;
		std::size_t iStart = 0u;
		for (std::size_t i = 0u; i <= value.size(); ++i)
		{
			const bool_t bBoundary = i == value.size() || '.' == value[i] ||
				'/' == value[i] || ':' == value[i];
			if (!bBoundary)
				continue;
			if (i > iStart)
				Segments.emplace_back(value.substr(iStart, i - iStart));
			iStart = i + 1u;
		}
		return Segments;
	}

	void InsertResourceTree(
		Client::COMPOSITION_RESOURCE_TREE_NODE& Root,
		const std::vector<std::string>& CategorySegments,
		const std::size_t iLeafIndex)
	{
		Client::COMPOSITION_RESOURCE_TREE_NODE* pNode = &Root;
		std::string strPath;
		for (const std::string& Segment : CategorySegments)
		{
			if (Segment.empty())
				continue;
			if (!strPath.empty())
				strPath += '/';
			strPath += Segment;
			auto Child = std::find_if(
				pNode->Children.begin(), pNode->Children.end(),
				[&Segment](const Client::COMPOSITION_RESOURCE_TREE_NODE& Candidate)
				{ return Candidate.strSegment == Segment; });
			if (pNode->Children.end() == Child)
			{
				Client::COMPOSITION_RESOURCE_TREE_NODE NewChild;
				NewChild.strSegment = Segment;
				NewChild.strStablePath = strPath;
				pNode->Children.push_back(std::move(NewChild));
				Child = std::prev(pNode->Children.end());
			}
			pNode = &*Child;
		}
		pNode->LeafIndices.push_back(iLeafIndex);
	}

	std::size_t FinalizeResourceTree(
		Client::COMPOSITION_RESOURCE_TREE_NODE& Node)
	{
		std::sort(
			Node.Children.begin(), Node.Children.end(),
			[](const Client::COMPOSITION_RESOURCE_TREE_NODE& Left,
				const Client::COMPOSITION_RESOURCE_TREE_NODE& Right)
			{ return Left.strSegment < Right.strSegment; });
		Node.iRecursiveLeafCount = Node.LeafIndices.size();
		for (Client::COMPOSITION_RESOURCE_TREE_NODE& Child : Node.Children)
			Node.iRecursiveLeafCount += FinalizeResourceTree(Child);
		return Node.iRecursiveLeafCount;
	}

	void RenderResourceTree(
		const Client::COMPOSITION_RESOURCE_TREE_NODE& Node,
		const std::function<void(std::size_t)>& RenderLeaf)
	{
		for (const Client::COMPOSITION_RESOURCE_TREE_NODE& Child : Node.Children)
		{
			ImGui::PushID(Child.strStablePath.c_str());
			const std::string Label = Child.strSegment + " (" +
				std::to_string(Child.iRecursiveLeafCount) + ")";
			if (ImGui::TreeNodeEx(Label.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth))
			{
				RenderResourceTree(Child, RenderLeaf);
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		for (const std::size_t iLeafIndex : Node.LeafIndices)
			RenderLeaf(iLeafIndex);
	}

	const char_t* EffectV2TypeLabel(const int32_t iType)
	{
		switch (static_cast<Client::EFFECT_V2_TYPE>(iType))
		{
		case Client::EFFECT_V2_TYPE::MESH: return "Mesh";
		case Client::EFFECT_V2_TYPE::TEXTURE: return "Texture";
		case Client::EFFECT_V2_TYPE::PARTICLE: return "Particle";
		case Client::EFFECT_V2_TYPE::DECAL: return "Decal";
		case Client::EFFECT_V2_TYPE::TRAIL: return "Trail";
		case Client::EFFECT_V2_TYPE::SCREEN_POST: return "Screen Post";
		default: return "Unknown";
		}
	}

	std::string ClipReplacementRole(const std::string& clip)
	{
		const std::string normalized = Lower(clip);
		if (std::string::npos != normalized.find("_start"))
			return "start";
		if (std::string::npos != normalized.find("_loop"))
			return "loop";
		if (std::string::npos != normalized.find("_end"))
			return "end";
		return {};
	}

	bool_t FitCompositionSequenceCutsToStage(
		const std::vector<uint32_t>& RequestedCutsMs,
		const uint32_t iStageDurationMs,
		std::vector<uint32_t>& OutFittedCutsMs,
		std::string& Status)
	{
		OutFittedCutsMs = RequestedCutsMs;
		const uint64_t iRequestedTotalMs = std::accumulate(
			RequestedCutsMs.begin(), RequestedCutsMs.end(), uint64_t{ 0u });
		if (iRequestedTotalMs <= iStageDurationMs)
			return true;
		if (RequestedCutsMs.size() < 3u)
		{
			Status =
				"Replace rejected: this Sequence is longer than the selected Server Stage. Only a start / loop / end HOLD chain can be fitted without guessing which authored edge to trim.";
			return false;
		}

		const std::size_t iMiddleCount = RequestedCutsMs.size() - 2u;
		const uint64_t iFixedEdgeMs =
			static_cast<uint64_t>(RequestedCutsMs.front()) +
			RequestedCutsMs.back();
		if (iFixedEdgeMs + iMiddleCount > iStageDurationMs)
		{
			Status =
				"Replace rejected: the Sequence start/end cuts leave no positive loop window inside the selected Server Stage clock.";
			return false;
		}

		uint64_t iRemainingBudgetMs = iStageDurationMs - iFixedEdgeMs;
		uint64_t iRemainingRequestedMs = std::accumulate(
			RequestedCutsMs.begin() + 1u, RequestedCutsMs.end() - 1u,
			uint64_t{ 0u });
		for (std::size_t iClip = 1u;
			iClip + 1u < RequestedCutsMs.size(); ++iClip)
		{
			const std::size_t iRemainingClipCount =
				RequestedCutsMs.size() - iClip - 2u;
			uint64_t iAllocationMs = iRemainingBudgetMs;
			if (0u != iRemainingClipCount)
			{
				const uint64_t iProportionalMs = 0u == iRemainingRequestedMs ?
					1u :
					(iRemainingBudgetMs * RequestedCutsMs[iClip]) /
						iRemainingRequestedMs;
				iAllocationMs = (std::clamp)(
					iProportionalMs,
					uint64_t{ 1u },
					iRemainingBudgetMs - iRemainingClipCount);
			}
			OutFittedCutsMs[iClip] = static_cast<uint32_t>(iAllocationMs);
			iRemainingBudgetMs -= iAllocationMs;
			iRemainingRequestedMs -= (std::min)(
				iRemainingRequestedMs,
				static_cast<uint64_t>(RequestedCutsMs[iClip]));
		}
		Status = "Fitted the extracted HOLD chain to the existing Server Stage clock.";
		return true;
	}

	std::string StableEffectIdFragment(std::string value)
	{
		std::transform(
			value.begin(), value.end(), value.begin(),
			[](const unsigned char ch)
			{
				return static_cast<char>(std::tolower(ch));
			});
		for (char& ch : value)
		{
			if (0 == std::isalnum(static_cast<unsigned char>(ch)) &&
				'_' != ch && '-' != ch)
			{
				ch = '-';
			}
		}
		return value;
	}

	std::string BuildNextCompositionEffectCueId(
		const Client::VALTAN_PATTERN_VIEW& pattern,
		const Client::VALTAN_STAGE_VIEW& stage)
	{
		constexpr std::string_view OCCURRENCE_SUFFIX = ".occurrence.01";
		const std::string prefix = "cue.valtan.composition." +
			StableEffectIdFragment(pattern.strPatternId) + "." +
			StableEffectIdFragment(stage.strStageId) + ".";
		const auto IsUsed = [&stage](const std::string& candidate)
		{
			return std::any_of(
				stage.ProductCues.begin(), stage.ProductCues.end(),
				[&candidate](const Client::VALTAN_PRODUCT_EFFECT_CUE_VIEW& cue)
				{
					return cue.strBindingId == candidate;
				});
		};
		const auto FindFreeId = [&IsUsed, OCCURRENCE_SUFFIX](
			const std::string& candidatePrefix) -> std::string
		{
			for (std::uint32_t ordinal = 1u; ordinal <= 9999u; ++ordinal)
			{
				std::ostringstream id;
				id << candidatePrefix << std::setw(2) << std::setfill('0') <<
					ordinal;
				const std::string candidate = id.str();
				if (candidate.size() + OCCURRENCE_SUFFIX.size() > 160u)
					return {};
				if (!IsUsed(candidate))
					return candidate;
			}
			return {};
		};
		if (std::string readable = FindFreeId(prefix); !readable.empty())
			return readable;
		uint64_t hash = 1469598103934665603ull;
		for (const unsigned char ch :
			pattern.strPatternId + "/" + stage.strStageId)
		{
			hash ^= ch;
			hash *= 1099511628211ull;
		}
		std::ostringstream compact;
		compact << "cue.valtan.composition." << std::hex << std::setw(16) <<
			std::setfill('0') << hash << std::dec << ".";
		return FindFreeId(compact.str());
	}

	bool_t SameSavedValtanEffectCue(
		const Client::VALTAN_PRODUCT_EFFECT_CUE_VIEW& left,
		const Client::VALTAN_PRODUCT_EFFECT_CUE_VIEW& right)
	{
		return left.strBindingId == right.strBindingId &&
			left.strOccurrenceId == right.strOccurrenceId &&
			left.strPatternId == right.strPatternId &&
			left.strStageId == right.strStageId &&
			left.strActionId == right.strActionId &&
			left.strClipOccurrenceId == right.strClipOccurrenceId &&
			left.strEffectAssetId == right.strEffectAssetId &&
			left.strV1EffectAssetId == right.strV1EffectAssetId &&
			left.strAnchorSlotId == right.strAnchorSlotId &&
			left.eFollowPolicy == right.eFollowPolicy &&
			left.eStopPolicy == right.eStopPolicy &&
			left.strFollowPolicy == right.strFollowPolicy &&
			left.strStopPolicy == right.strStopPolicy &&
			left.strRepeatPolicy == right.strRepeatPolicy &&
			left.eScalePolicy == right.eScalePolicy &&
			left.strScalePolicy == right.strScalePolicy &&
			left.vWorldScale.x == right.vWorldScale.x &&
			left.vWorldScale.y == right.vWorldScale.y &&
			left.vWorldScale.z == right.vWorldScale.z &&
			left.bHasExplicitScalePolicy == right.bHasExplicitScalePolicy &&
			left.bUsesStageClock == right.bUsesStageClock &&
			left.iStageOffsetMs == right.iStageOffsetMs &&
			left.iSourceStartMs == right.iSourceStartMs &&
			left.iSourceEndMs == right.iSourceEndMs &&
			left.iStageDurationMs == right.iStageDurationMs &&
			left.bHasSourceEnd == right.bHasSourceEnd &&
			left.LocalTransform.vPosition.x ==
				right.LocalTransform.vPosition.x &&
			left.LocalTransform.vPosition.y ==
				right.LocalTransform.vPosition.y &&
			left.LocalTransform.vPosition.z ==
				right.LocalTransform.vPosition.z &&
			left.LocalTransform.vRotationDegrees.x ==
				right.LocalTransform.vRotationDegrees.x &&
			left.LocalTransform.vRotationDegrees.y ==
				right.LocalTransform.vRotationDegrees.y &&
			left.LocalTransform.vRotationDegrees.z ==
				right.LocalTransform.vRotationDegrees.z &&
			left.LocalTransform.vScale.x == right.LocalTransform.vScale.x &&
			left.LocalTransform.vScale.y == right.LocalTransform.vScale.y &&
			left.LocalTransform.vScale.z == right.LocalTransform.vScale.z;
	}

	bool_t BuildNextManualStageIdentity(
		const Client::VALTAN_PATTERN_VIEW& Pattern,
		std::string& strOutStageId,
		std::string& strOutActionId)
	{
		for (std::size_t ordinal = 1u; ordinal <= 9999u; ++ordinal)
		{
			std::ostringstream suffix;
			suffix << std::setw(2) << std::setfill('0') << ordinal;
			const std::string stageId = "COMPOSITION_" + suffix.str();
			std::string actionId = Pattern.strActionId +
				".composition-stage." + suffix.str();
			if (actionId.size() > 160u)
			{
				uint64_t hash = 1469598103934665603ull;
				for (const unsigned char ch : Pattern.strPatternId)
				{
					hash ^= ch;
					hash *= 1099511628211ull;
				}
				std::ostringstream compact;
				compact << "valtan.composition." << std::hex <<
					std::setw(16) << std::setfill('0') << hash <<
					std::dec << ".stage." << suffix.str();
				actionId = compact.str();
			}
			const bool_t stageExists = std::any_of(
				Pattern.Stages.begin(), Pattern.Stages.end(),
				[&stageId](const Client::VALTAN_STAGE_VIEW& Stage)
				{ return Stage.strStageId == stageId; });
			const bool_t actionExists = std::any_of(
				Pattern.Stages.begin(), Pattern.Stages.end(),
				[&actionId](const Client::VALTAN_STAGE_VIEW& Stage)
				{ return Stage.strActionId == actionId; });
			if (!stageExists && !actionExists)
			{
				strOutStageId = stageId;
				strOutActionId = actionId;
				return true;
			}
		}
		strOutStageId.clear();
		strOutActionId.clear();
		return false;
	}

	bool_t ComputeExactAnimationWallMs(
		const Client::CBalanceTool::PATTERN_STAGE_EDIT& Draft,
		uint32_t& iOutWallMs)
	{
		uint64_t iWallMs = 0u;
		if (!Draft.animationEditable || Draft.animationSlots.empty())
			return false;
		for (const Client::CBalanceTool::ANIMATION_SLOT_EDIT& Slot :
			Draft.animationSlots)
		{
			if (0u == Slot.playMs || Slot.repeatUntilStageEnd ||
				!std::isfinite(Slot.playRate) || Slot.playRate <= 0.0)
			{
				return false;
			}
			iWallMs += static_cast<uint64_t>(std::llround(
				static_cast<double>(Slot.playMs) / Slot.playRate));
			if (iWallMs > 600000u)
				return false;
		}
		iOutWallMs = static_cast<uint32_t>(iWallMs);
		return 0u != iOutWallMs;
	}

	bool_t ApplyStageClockPolicy(
		Client::CBalanceTool::PATTERN_STAGE_EDIT& Draft,
		const uint32_t iRequestedDurationMs,
		std::string& Status)
	{
		if (!Draft.animationEditable || Draft.animationSlots.empty() ||
			"NONE" == Draft.animationEndPolicy ||
			"LOOP_TO_STAGE_END" == Draft.animationEndPolicy)
		{
			Draft.durationMs = iRequestedDurationMs;
			return true;
		}
		uint32_t iAnimationWallMs = 0u;
		if (!ComputeExactAnimationWallMs(Draft, iAnimationWallMs))
		{
			Status =
				"Stage clock edit blocked: the Animation Sequence wall is unresolved. Fix zero/native/loop slots first.";
			return false;
		}
		if (iRequestedDurationMs < iAnimationWallMs)
		{
			Status = "Stage clock cannot trim below its Animation Sequence wall (" +
				std::to_string(iAnimationWallMs) + " ms).";
			return false;
		}
		Draft.durationMs = iRequestedDurationMs;
		Draft.animationEndPolicy = iRequestedDurationMs == iAnimationWallMs ?
			"EXACT" : "HOLD_LAST_POSE";
		return true;
	}

	Client::VALTAN_STAGE_VIEW BuildPatternSoundCandidateStage(
		const Client::VALTAN_STAGE_VIEW& BaselineStage,
		const Client::CBalanceTool::PATTERN_STAGE_EDIT& Draft)
	{
		Client::VALTAN_STAGE_VIEW Candidate = BaselineStage;
		Candidate.strActionId = Draft.actionId;
		Candidate.iDurationMs = Draft.durationMs;
		Candidate.iAuthoringRepeatCount = Draft.animationRepeatCount;
		Candidate.strAnimationEndPolicy = Draft.animationEndPolicy;
		Candidate.bSuppressAnimation = Draft.animationSlots.empty();
		Candidate.ClipOccurrences.clear();
		Candidate.ClipOccurrences.reserve(Draft.animationSlots.size());
		for (const Client::CBalanceTool::ANIMATION_SLOT_EDIT& Slot :
			Draft.animationSlots)
		{
			Client::VALTAN_CLIP_OCCURRENCE_VIEW Occurrence;
			Occurrence.strClipOccurrenceId = Slot.clipOccurrenceId;
			Occurrence.strClipName = Slot.clip;
			Occurrence.strMappingBasis = Slot.mappingBasis;
			Occurrence.iSourceStartMs = Slot.sourceStartMs;
			Occurrence.iPlayMs = Slot.playMs;
			Occurrence.fPlayRate = static_cast<f32_t>(Slot.playRate);
			Occurrence.bLoop = Slot.repeatUntilStageEnd;
			Candidate.ClipOccurrences.push_back(std::move(Occurrence));
		}
		return Candidate;
	}

	bool_t ValidateClipSourceDependencyWindow(
		const Client::VALTAN_STAGE_VIEW& CandidateStage,
		const std::string& strClipOccurrenceId,
		const uint32_t iSourceStartMs,
		const bool_t bHasSourceEnd,
		const uint32_t iSourceEndMs,
		const bool_t bRequireLoop,
		const std::string& strDependencyLabel,
		std::string& Status)
	{
		const std::size_t iOccurrenceCount = static_cast<std::size_t>(
			std::count_if(
				CandidateStage.ClipOccurrences.begin(),
				CandidateStage.ClipOccurrences.end(),
				[&strClipOccurrenceId](
					const Client::VALTAN_CLIP_OCCURRENCE_VIEW& Clip)
				{
					return Clip.strClipOccurrenceId == strClipOccurrenceId;
				}));
		const auto Found = std::find_if(
			CandidateStage.ClipOccurrences.begin(),
			CandidateStage.ClipOccurrences.end(),
			[&strClipOccurrenceId](
				const Client::VALTAN_CLIP_OCCURRENCE_VIEW& Clip)
			{
				return Clip.strClipOccurrenceId == strClipOccurrenceId;
			});
		if (1u != iOccurrenceCount ||
			CandidateStage.ClipOccurrences.end() == Found)
		{
			Status = strDependencyLabel +
				" no longer resolves exactly one clipOccurrenceId: " +
				strClipOccurrenceId + ".";
			return false;
		}

		const uint64_t iWindowBeginMs = Found->iSourceStartMs;
		const bool_t bFiniteWindow = 0u != Found->iPlayMs;
		const uint64_t iWindowEndExclusiveMs = iWindowBeginMs +
			static_cast<uint64_t>(Found->iPlayMs);
		const bool_t bStartInside =
			static_cast<uint64_t>(iSourceStartMs) >= iWindowBeginMs &&
			(!bFiniteWindow ||
			 static_cast<uint64_t>(iSourceStartMs) < iWindowEndExclusiveMs);
		const bool_t bEndInside = !bHasSourceEnd ||
			(iSourceEndMs > iSourceStartMs &&
			 (!bFiniteWindow ||
			  static_cast<uint64_t>(iSourceEndMs) <= iWindowEndExclusiveMs));
		if (!bStartInside || !bEndInside ||
			(bRequireLoop && !Found->bLoop))
		{
			Status = strDependencyLabel +
				" is outside the candidate Animation source window or requires a removed loop: " +
				strClipOccurrenceId + ".";
			return false;
		}
		return true;
	}

	bool_t SetValtanStageDraftWithSoundDependencyAdmission(
		Client::CAnimation_Tool* const pAnimationTool,
		Client::CBalanceTool* const pBalanceTool,
		const Client::VALTAN_PATTERN_SHAKE_CUE_DOCUMENT* const pPatternShakes,
		const Client::VALTAN_PATTERN_VIEW& BaselinePattern,
		const Client::VALTAN_STAGE_VIEW& BaselineStage,
		const Client::CBalanceTool::PATTERN_STAGE_EDIT& Draft,
		std::string& Status)
	{
		if (nullptr == pAnimationTool || nullptr == pBalanceTool)
		{
			Status =
				"Pattern/Animation mutation requires both the typed Stage owner and Pattern Sound dependency owner.";
			return false;
		}
		const Client::VALTAN_STAGE_VIEW CandidateStage =
			BuildPatternSoundCandidateStage(BaselineStage, Draft);
		if (!pAnimationTool->
				Validate_ValtanCompositionAnimationStageMutation(
					BaselineStage, CandidateStage, Status))
		{
			Status =
				"Stage mutation rejected before the typed draft changed: " + Status;
			return false;
		}
		if (!pAnimationTool->
				Validate_ValtanCompositionPatternSoundStageDependencies(
					BaselinePattern, BaselineStage, CandidateStage, Status))
		{
			Status =
				"Stage mutation rejected before the typed draft changed: " + Status;
			return false;
		}
		const bool_t bAnimationDependencyChanged =
			BaselineStage.strActionId != CandidateStage.strActionId ||
			BaselineStage.ClipOccurrences.size() !=
				CandidateStage.ClipOccurrences.size() ||
			!std::equal(
				BaselineStage.ClipOccurrences.begin(),
				BaselineStage.ClipOccurrences.end(),
				CandidateStage.ClipOccurrences.begin(),
				[](const Client::VALTAN_CLIP_OCCURRENCE_VIEW& Baseline,
					const Client::VALTAN_CLIP_OCCURRENCE_VIEW& Candidate)
				{
					return Baseline.strClipOccurrenceId ==
							Candidate.strClipOccurrenceId &&
						Baseline.strClipName == Candidate.strClipName &&
						Baseline.strMappingBasis == Candidate.strMappingBasis &&
						Baseline.iSourceStartMs == Candidate.iSourceStartMs &&
						Baseline.iPlayMs == Candidate.iPlayMs &&
						Baseline.fPlayRate == Candidate.fPlayRate &&
						Baseline.bLoop == Candidate.bLoop;
				});
		if (bAnimationDependencyChanged)
		{
			for (const Client::VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue :
				CandidateStage.ProductCues)
			{
				if (Cue.bUsesStageClock)
					continue;
				if (!ValidateClipSourceDependencyWindow(
						CandidateStage, Cue.strClipOccurrenceId,
						Cue.iSourceStartMs, Cue.bHasSourceEnd,
						Cue.iSourceEndMs,
						"each_loop" == Cue.strRepeatPolicy,
						"Effect occurrence " + Cue.strOccurrenceId, Status))
				{
					Status =
						"Stage mutation rejected before the typed draft changed: " +
						Status;
					return false;
				}
			}
		}
		if (bAnimationDependencyChanged && nullptr == pPatternShakes)
		{
			Status =
				"Stage mutation rejected before the typed draft changed: the Pattern Shake dependency source is unavailable.";
			return false;
		}
		if (bAnimationDependencyChanged)
		{
			for (const Client::VALTAN_PATTERN_SHAKE_CUE& Cue :
				pPatternShakes->Cues)
			{
				if (Cue.strPatternId != BaselinePattern.strPatternId ||
					Cue.strStageId != BaselineStage.strStageId)
				{
					continue;
				}
				if (Cue.strActionId != BaselineStage.strActionId ||
					Cue.strActionId != CandidateStage.strActionId)
				{
					Status =
						"Stage mutation rejected before the typed draft changed: Pattern Shake row no longer resolves the exact Pattern/Stage/action tuple: " +
						Cue.strOccurrenceId + ".";
					return false;
				}
				const auto FindOccurrence = [&Cue](
					const Client::VALTAN_STAGE_VIEW& Stage)
				{
					return std::find_if(
						Stage.ClipOccurrences.begin(), Stage.ClipOccurrences.end(),
						[&Cue](const Client::VALTAN_CLIP_OCCURRENCE_VIEW& Clip)
						{
							return Clip.strClipOccurrenceId ==
								Cue.strClipOccurrenceId;
						});
				};
				const std::size_t iBaselineCount = static_cast<std::size_t>(
					std::count_if(
						BaselineStage.ClipOccurrences.begin(),
						BaselineStage.ClipOccurrences.end(),
						[&Cue](const Client::VALTAN_CLIP_OCCURRENCE_VIEW& Clip)
						{
							return Clip.strClipOccurrenceId ==
								Cue.strClipOccurrenceId;
						}));
				const std::size_t iCandidateCount = static_cast<std::size_t>(
					std::count_if(
						CandidateStage.ClipOccurrences.begin(),
						CandidateStage.ClipOccurrences.end(),
						[&Cue](const Client::VALTAN_CLIP_OCCURRENCE_VIEW& Clip)
						{
							return Clip.strClipOccurrenceId ==
								Cue.strClipOccurrenceId;
						}));
				const auto BaselineClip = FindOccurrence(BaselineStage);
				const auto CandidateClip = FindOccurrence(CandidateStage);
				if (1u != iBaselineCount || 1u != iCandidateCount ||
					BaselineStage.ClipOccurrences.end() == BaselineClip ||
					CandidateStage.ClipOccurrences.end() == CandidateClip)
				{
					Status =
						"Stage mutation rejected before the typed draft changed: Pattern Shake row would dangle; retarget or remove it explicitly before replacing its clip occurrence: " +
						Cue.strOccurrenceId + ".";
					return false;
				}
				if (BaselineClip->strClipName != CandidateClip->strClipName &&
					"PROJECT_AUTHORED" != CandidateClip->strMappingBasis)
				{
					Status =
						"Stage mutation rejected before the typed draft changed: Pattern Shake-qualified clipOccurrenceId can change its resource only through an explicit PROJECT_AUTHORED slot replacement: " +
							Cue.strClipOccurrenceId + ".";
					return false;
				}
				if (!ValidateClipSourceDependencyWindow(
						CandidateStage, Cue.strClipOccurrenceId,
						Cue.iStartMs, false, 0u,
						Client::VALTAN_PATTERN_SHAKE_REPEAT_POLICY::EACH_LOOP ==
							Cue.eRepeatPolicy,
						"Pattern Shake occurrence " + Cue.strOccurrenceId,
						Status))
				{
					Status =
						"Stage mutation rejected before the typed draft changed: " +
						Status;
					return false;
				}
			}
		}
		return pBalanceTool->Set_ValtanStageDraft(
			BaselinePattern.strPatternId, BaselineStage.strStageId, Draft, Status);
	}

	const char* PreviewPathLabel(
		const Client::VALTAN_PATTERN_PREVIEW_PATH ePath)
	{
		switch (ePath)
		{
		case Client::VALTAN_PATTERN_PREVIEW_PATH::NORMAL:
			return "Normal / Timeout";
		case Client::VALTAN_PATTERN_PREVIEW_PATH::COUNTER_GROGGY:
			return "Counter Hit -> Groggy";
		case Client::VALTAN_PATTERN_PREVIEW_PATH::WALL_GROGGY:
			return "Wall Contact -> Groggy";
		case Client::VALTAN_PATTERN_PREVIEW_PATH::PART_BREAK:
			return "Wall Contact -> Part Break";
		default:
			return "Invalid";
		}
	}
}

Client::CActionCompositionWorkbench::CActionCompositionWorkbench(
	CAnimation_Tool* const pAnimationTool,
	CBalanceTool* const pBalanceTool,
	CBossTool* const pBossTool)
	: m_pAnimationTool(pAnimationTool),
	  m_pBalanceTool(pBalanceTool),
	  m_pBossTool(pBossTool)
{
}

void Client::CActionCompositionWorkbench::Reload_SemanticValtanEffects()
{
	m_bSemanticValtanEffectLoadAttempted = true;
	m_SemanticValtanEffectAssetIds.clear();
	m_FilteredEffectAssetIndices.clear();
	m_FilteredEffectV2DocumentIndices.clear();
	m_FilteredEffectV2GroupIndices.clear();
	m_bEffectFilterDirty = true;
	for (const std::string& EffectAssetId : CEffectCatalog::Get_EffectAssetIds())
	{
		if (0u == EffectAssetId.rfind("effect.valtan.", 0u) &&
			CEffectCatalog::Is_DirectAuthoredDocument(EffectAssetId))
		{
			m_SemanticValtanEffectAssetIds.push_back(EffectAssetId);
		}
	}
	std::sort(
		m_SemanticValtanEffectAssetIds.begin(),
		m_SemanticValtanEffectAssetIds.end());
	m_SemanticValtanEffectAssetIds.erase(
		std::unique(
			m_SemanticValtanEffectAssetIds.begin(),
			m_SemanticValtanEffectAssetIds.end()),
		m_SemanticValtanEffectAssetIds.end());

	std::string V2Status;
	const bool_t bV2Reloaded =
		CEffectV2Catalog::Get().Reload_BossValtan(V2Status);
	const std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> pV2Snapshot =
		CEffectV2Catalog::Get().Get_Snapshot();
	if (nullptr != pV2Snapshot && pV2Snapshot->Is_Ready())
	{
		m_EffectV2DocumentIds.clear();
		m_EffectV2DocumentTypes.clear();
		m_EffectV2GroupIds.clear();
		m_EffectV2DocumentIds.reserve(
			pV2Snapshot->Get_Documents().size());
		m_EffectV2DocumentTypes.reserve(
			pV2Snapshot->Get_Documents().size());
		for (const EFFECT_V2_DOCUMENT& Document :
			pV2Snapshot->Get_Documents())
		{
			m_EffectV2DocumentIds.push_back(Document.strEffectId);
			m_EffectV2DocumentTypes.push_back(
				static_cast<int32_t>(Document.eType));
		}
		m_EffectV2GroupIds.reserve(pV2Snapshot->Get_Groups().size());
		for (const EFFECT_V2_GROUP& Group : pV2Snapshot->Get_Groups())
			m_EffectV2GroupIds.push_back(Group.strGroupId);
		m_iEffectV2CatalogRevision = pV2Snapshot->Get_Revision();
	}
	else
	{
		m_EffectV2DocumentIds.clear();
		m_EffectV2DocumentTypes.clear();
		m_EffectV2GroupIds.clear();
		m_iEffectV2CatalogRevision = 0u;
	}
	m_strEffectCatalogStatus =
		"V1 authored " + std::to_string(m_SemanticValtanEffectAssetIds.size()) +
		" | V2 leaf " + std::to_string(m_EffectV2DocumentIds.size()) +
		" | V2 group " + std::to_string(m_EffectV2GroupIds.size());
	if (!bV2Reloaded && !V2Status.empty())
		m_strEffectCatalogStatus += " | V2 reload preserved previous snapshot: " +
			V2Status;

	bool_t bSelectedExists = m_strEffectAddAssetId.empty();
	if (!bSelectedExists)
	{
		const std::vector<std::string>* pOwner = nullptr;
		switch (m_eEffectAddResourceKind)
		{
		case EFFECT_RESOURCE_KIND::V1_PATTERN:
			pOwner = &m_SemanticValtanEffectAssetIds;
			break;
		case EFFECT_RESOURCE_KIND::V2_LEAF:
			pOwner = &m_EffectV2DocumentIds;
			break;
		case EFFECT_RESOURCE_KIND::V2_GROUP:
			pOwner = &m_EffectV2GroupIds;
			break;
		default:
			break;
		}
		bSelectedExists = nullptr != pOwner && pOwner->end() != std::find(
			pOwner->begin(), pOwner->end(), m_strEffectAddAssetId);
	}
	if (!bSelectedExists)
	{
		m_strEffectAddAssetId.clear();
	}
	Invalidate_TimelineCache();
}

void Client::CActionCompositionWorkbench::Reset_EffectCueEditor()
{
	m_strEffectEditIdentity.clear();
	m_EffectCueEditDraft = {};
	m_strEffectAddClipOccurrenceId.clear();
}

bool_t Client::CActionCompositionWorkbench::Open_Valtan()
{
	m_bPatternsWindowVisible = true;
	m_bPreviewWindowVisible = true;
	m_bSequencerWindowVisible = true;
	m_bDetailsWindowVisible = true;
	m_bResourcesWindowVisible = true;
	m_bSessionWindowVisible = true;
	if (!m_bLoadAttempted || ADMISSION_STATE::ADMITTED != m_eAdmission)
		return Reload_Canonical();
	return true;
}

void Client::CActionCompositionWorkbench::On_LevelChanged()
{
	/* Canonical data survives a Level transition, but model pose and Server
	   occurrence do not.  Keep semantic selection and re-query preview state. */
	m_iPlayheadMs = 0u;
}

const char_t* Client::CActionCompositionWorkbench::Admission_Label() const
{
	switch (m_eAdmission)
	{
	case ADMISSION_STATE::UNLOADED:
		return "NOT LOADED";
	case ADMISSION_STATE::ADMITTED:
		return "ADMITTED";
	case ADMISSION_STATE::STALE_PRESERVED:
		return "STALE PRESERVED / READ ONLY";
	case ADMISSION_STATE::REJECTED:
		return "REJECTED";
	default:
		return "INVALID";
	}
}

const char_t* Client::CActionCompositionWorkbench::Owner_Label(
	const DETAIL_OWNER eOwner)
{
	switch (eOwner)
	{
	case DETAIL_OWNER::PATTERN: return "Pattern Root / Flow";
	case DETAIL_OWNER::GAMEPLAY_STAGE: return "Gameplay / Logic / Collider";
	case DETAIL_OWNER::ANIMATION: return "Animation Sequence";
	case DETAIL_OWNER::EFFECT: return "Effect";
	case DETAIL_OWNER::SOUND: return "Sound";
	case DETAIL_OWNER::CAMERA: return "Camera / Light";
	case DETAIL_OWNER::WORLD: return "World";
	case DETAIL_OWNER::COMBAT_OBJECT: return "Combat Object";
	default: return "Unknown";
	}
}

const char_t* Client::CActionCompositionWorkbench::Lane_Label(
	const TIMELINE_LANE eLane)
{
	switch (eLane)
	{
	case TIMELINE_LANE::STAGE: return "Stage";
	case TIMELINE_LANE::ANIMATION: return "Animation";
	case TIMELINE_LANE::EFFECT: return "Effect";
	case TIMELINE_LANE::SOUND: return "Sound";
	case TIMELINE_LANE::LOGIC: return "Logic";
	case TIMELINE_LANE::COLLIDER: return "Collider";
	case TIMELINE_LANE::CAMERA: return "Camera";
	default: return "Invalid";
	}
}

uint32_t Client::CActionCompositionWorkbench::Lane_Color(
	const TIMELINE_LANE eLane)
{
	switch (eLane)
	{
	case TIMELINE_LANE::STAGE: return IM_COL32(98, 112, 132, 255);
	case TIMELINE_LANE::ANIMATION: return IM_COL32(76, 128, 224, 255);
	case TIMELINE_LANE::EFFECT: return IM_COL32(166, 94, 224, 255);
	case TIMELINE_LANE::SOUND: return IM_COL32(64, 178, 112, 255);
	case TIMELINE_LANE::LOGIC: return IM_COL32(210, 86, 92, 255);
	case TIMELINE_LANE::COLLIDER: return IM_COL32(76, 176, 184, 255);
	case TIMELINE_LANE::CAMERA: return IM_COL32(224, 156, 66, 255);
	default: return IM_COL32(116, 126, 138, 255);
	}
}

std::vector<const Client::VALTAN_PATTERN_VIEW*>
Client::CActionCompositionWorkbench::Collect_Patterns() const
{
	std::vector<const VALTAN_PATTERN_VIEW*> Patterns;
	Patterns.reserve(m_CanonicalView.Get_PatternCount());
	for (const VALTAN_PATTERN_VIEW& Pattern : m_CanonicalView.Gimmicks)
		if (m_PlayableInventory.Contains(Pattern.strPatternId))
			Patterns.push_back(&Pattern);
	for (const VALTAN_PATTERN_VIEW& Pattern : m_CanonicalView.Rotation)
		if (m_PlayableInventory.Contains(Pattern.strPatternId))
			Patterns.push_back(&Pattern);
	std::stable_sort(
		Patterns.begin(), Patterns.end(),
		[](const VALTAN_PATTERN_VIEW* const pLeft,
			const VALTAN_PATTERN_VIEW* const pRight)
		{
			if (pLeft->iMinimumPhase != pRight->iMinimumPhase)
				return pLeft->iMinimumPhase < pRight->iMinimumPhase;
			if (pLeft->Is_Gimmick() != pRight->Is_Gimmick())
				return pLeft->Is_Gimmick() > pRight->Is_Gimmick();
			return pLeft->strPatternId < pRight->strPatternId;
		});
	return Patterns;
}

std::vector<const Client::VALTAN_PATTERN_VIEW*>
Client::CActionCompositionWorkbench::
Collect_CanonicalPatternsForDependencyValidation() const
{
	/* The browser is intentionally filtered to Complete Play inventory rows,
	   but cross-owner Sound validation must receive the complete canonical
	   graph.  Legacy-compatible Patterns can own admitted Sound rows even when
	   they are not individually playable from the Workbench browser. */
	std::vector<const VALTAN_PATTERN_VIEW*> Patterns;
	Patterns.reserve(m_CanonicalView.Get_PatternCount());
	for (const VALTAN_PATTERN_VIEW& Pattern : m_CanonicalView.Gimmicks)
		Patterns.push_back(&Pattern);
	for (const VALTAN_PATTERN_VIEW& Pattern : m_CanonicalView.Rotation)
		Patterns.push_back(&Pattern);
	return Patterns;
}

void Client::CActionCompositionWorkbench::
Invalidate_SourceSequenceOwnerIndex()
{
	m_SourceSequenceOwnerIndex.clear();
	m_iSourceSequenceOwnerIndexGeneration = ~std::uint64_t{ 0u };
	m_strSourceSequenceServerPatternId.clear();
}

void Client::CActionCompositionWorkbench::Ensure_SourceSequenceOwnerIndex()
{
	if (m_iSourceSequenceOwnerIndexGeneration ==
		m_iCanonicalDisplayGeneration)
	{
		return;
	}

	std::vector<SOURCE_SEQUENCE_OWNER_INDEX_ENTRY> Staged;
	const std::vector<const VALTAN_PATTERN_VIEW*> Patterns = Collect_Patterns();
	Staged.reserve(Patterns.size());
	for (const VALTAN_PATTERN_VIEW* const pPattern : Patterns)
	{
		if (nullptr == pPattern)
			continue;
		for (const VALTAN_PRESENTATION_SOURCE_VIEW& Source :
			pPattern->PresentationSources)
		{
			/* REFERENCE rows are research/provenance links, not a claim that the
			   Product Pattern directly owns this raw Sequence for Server Play. */
			if ("PRIMARY" != Source.strRole)
				continue;
			auto Entry = std::find_if(
				Staged.begin(), Staged.end(),
				[&Source](const SOURCE_SEQUENCE_OWNER_INDEX_ENTRY& Candidate)
				{
					return Candidate.iSourceActionId ==
							Source.iSourceActionId &&
						Candidate.iSequenceIndex == Source.iSequenceIndex;
				});
			if (Entry == Staged.end())
			{
				SOURCE_SEQUENCE_OWNER_INDEX_ENTRY Candidate;
				Candidate.iSourceActionId = Source.iSourceActionId;
				Candidate.iSequenceIndex = Source.iSequenceIndex;
				Staged.push_back(std::move(Candidate));
				Entry = Staged.end() - 1;
			}
			if (Entry->Owners.end() == std::find(
					Entry->Owners.begin(), Entry->Owners.end(), pPattern))
			{
				Entry->Owners.push_back(pPattern);
			}
		}
	}
	std::sort(
		Staged.begin(), Staged.end(),
		[](const SOURCE_SEQUENCE_OWNER_INDEX_ENTRY& Left,
			const SOURCE_SEQUENCE_OWNER_INDEX_ENTRY& Right)
		{
			if (Left.iSourceActionId != Right.iSourceActionId)
				return Left.iSourceActionId < Right.iSourceActionId;
			return Left.iSequenceIndex < Right.iSequenceIndex;
		});
	m_SourceSequenceOwnerIndex = std::move(Staged);
	m_iSourceSequenceOwnerIndexGeneration = m_iCanonicalDisplayGeneration;
}

const Client::CActionCompositionWorkbench::
	SOURCE_SEQUENCE_OWNER_INDEX_ENTRY*
Client::CActionCompositionWorkbench::Find_SourceSequenceOwners(
	const uint32_t iSourceActionId,
	const uint32_t iSequenceIndex) const
{
	const std::uint64_t iKey =
		(static_cast<std::uint64_t>(iSourceActionId) << 32u) |
		static_cast<std::uint64_t>(iSequenceIndex);
	const auto Found = std::lower_bound(
		m_SourceSequenceOwnerIndex.begin(),
		m_SourceSequenceOwnerIndex.end(), iKey,
		[](const SOURCE_SEQUENCE_OWNER_INDEX_ENTRY& Entry,
			const std::uint64_t iCandidateKey)
		{
			const std::uint64_t iEntryKey =
				(static_cast<std::uint64_t>(Entry.iSourceActionId) << 32u) |
				static_cast<std::uint64_t>(Entry.iSequenceIndex);
			return iEntryKey < iCandidateKey;
		});
	if (Found == m_SourceSequenceOwnerIndex.end() ||
		Found->iSourceActionId != iSourceActionId ||
		Found->iSequenceIndex != iSequenceIndex)
	{
		return nullptr;
	}
	return &*Found;
}

const Client::VALTAN_PATTERN_VIEW*
Client::CActionCompositionWorkbench::Find_SelectedPattern() const
{
	/* The canonical view also carries encounter/reference-only compatibility
	   rows.  Composition authoring follows the exact Boss Tool/Complete Play
	   inventory and must never revive a legacy row through preserved selection. */
	if (!m_PlayableInventory.Contains(m_strSelectedPatternId))
		return nullptr;
	const auto Match = [this](const VALTAN_PATTERN_VIEW& Pattern)
	{
		return Pattern.strPatternId == m_strSelectedPatternId;
	};
	if (const auto Found = std::find_if(
			m_CanonicalView.Gimmicks.begin(), m_CanonicalView.Gimmicks.end(), Match);
		Found != m_CanonicalView.Gimmicks.end())
	{
		return &*Found;
	}
	if (const auto Found = std::find_if(
			m_CanonicalView.Rotation.begin(), m_CanonicalView.Rotation.end(), Match);
		Found != m_CanonicalView.Rotation.end())
	{
		return &*Found;
	}
	return nullptr;
}

const Client::VALTAN_STAGE_VIEW*
Client::CActionCompositionWorkbench::Find_SelectedStage(
	const VALTAN_PATTERN_VIEW* const pPattern) const
{
	if (nullptr == pPattern)
		return nullptr;
	const auto Found = std::find_if(
		pPattern->Stages.begin(), pPattern->Stages.end(),
		[this](const VALTAN_STAGE_VIEW& Stage)
		{
			return Stage.strStageId == m_strSelectedStageId;
		});
	return Found == pPattern->Stages.end() ? nullptr : &*Found;
}

void Client::CActionCompositionWorkbench::Normalize_Selection()
{
	const VALTAN_PATTERN_VIEW* pPattern = Find_SelectedPattern();
	bool_t bPatternSelectionChanged = false;
	if (nullptr == pPattern)
	{
		/* Collect/sort only while resolving an absent or rejected stable ID.
		   A warm frame with a valid selection performs no Pattern allocation. */
		const std::vector<const VALTAN_PATTERN_VIEW*> Patterns =
			Collect_Patterns();
		if (Patterns.empty())
		{
			m_strSelectedPatternId.clear();
			m_strSelectedStageId.clear();
			m_strSelectedStableId.clear();
			m_TimelineItems.clear();
			m_iTimelineDurationMs = 0u;
			return;
		}
		pPattern = Patterns.front();
		m_strSelectedPatternId = pPattern->strPatternId;
		bPatternSelectionChanged = true;
		m_eDetailOwner = DETAIL_OWNER::PATTERN;
		m_strSelectedStableId = pPattern->strPatternId;
	}
	if (pPattern->Stages.empty())
		m_strSelectedStageId.clear();
	else if (bPatternSelectionChanged || m_strSelectedStageId.empty())
		m_strSelectedStageId = pPattern->Stages.front().strStageId;
	m_iPlayheadMs = (std::min)(m_iPlayheadMs, m_iTimelineDurationMs);
}

bool_t Client::CActionCompositionWorkbench::Reload_Canonical()
{
	m_bLoadAttempted = true;
	Invalidate_EffectivePatternCache();
	std::string SoundDraftStatus;
	if (Is_PatternSoundDraftDirty(SoundDraftStatus))
	{
		m_strStatus =
			"Canonical reload blocked: save or explicitly discard the unsaved Pattern Sound owner draft before changing its dependency generation. " +
			SoundDraftStatus;
		return false;
	}
	std::string AuthoringRevision;
	std::string CanonicalSourceRevision;
	bool_t bAuthoringDirty = false;
	if (nullptr != m_pBalanceTool)
	{
		if (m_pBalanceTool->Is_ValtanDraftDirty())
		{
			m_strStatus =
				"Canonical reload skipped because the admitted effective authoring draft has unsaved changes. Save it first; the current admitted draft remains editable.";
			return false;
		}
	}
	CValtanCanonicalProductReadAdmission CanonicalAdmission;
	std::string CanonicalAdmissionStatus;
	if (!CanonicalAdmission.Acquire(CanonicalAdmissionStatus))
	{
		const bool_t bPreserved = 0u != m_CanonicalView.Get_PatternCount();
		m_eAdmission = bPreserved ? ADMISSION_STATE::STALE_PRESERVED :
			ADMISSION_STATE::REJECTED;
		m_strStatus = bPreserved ?
			"Canonical reload admission was busy; previous Workbench data is read-only: " +
				CanonicalAdmissionStatus :
			"Canonical reload admission failed; no mutation or Server playback is available: " +
				CanonicalAdmissionStatus;
		return false;
	}
	VALTAN_PATTERN_TREE_VIEW Staged;
	std::string Diagnostic;
	if (!CValtanPatternTree::Load_WhileAdmitted(
			CanonicalAdmission, Staged, Diagnostic))
	{
		const bool_t bPreserved = 0u != m_CanonicalView.Get_PatternCount();
		m_eAdmission = bPreserved ?
			ADMISSION_STATE::STALE_PRESERVED : ADMISSION_STATE::REJECTED;
		m_strStatus = bPreserved ?
			"Canonical reload rejected; previous graph is displayed read-only: " + Diagnostic :
			"Canonical graph admission failed; no mutation or Server playback is available: " + Diagnostic;
		Normalize_Selection();
		return false;
	}
	VALTAN_TOOL_AUDITION_INVENTORY Inventory;
	std::string InventoryStatus;
	if (!CValtanPatternTree::Build_PlayablePatternInventory(
			Staged, Inventory, InventoryStatus))
	{
		const bool_t bPreserved = 0u != m_CanonicalView.Get_PatternCount();
		m_eAdmission = bPreserved ?
			ADMISSION_STATE::STALE_PRESERVED : ADMISSION_STATE::REJECTED;
		m_strStatus = bPreserved ?
			"Playable inventory rejected; previous graph is displayed read-only: " + InventoryStatus :
			"Canonical graph loaded but its playable inventory was rejected: " + InventoryStatus;
		Normalize_Selection();
		return false;
	}
	const auto PreserveOrCommitReadOnlyProduct =
		[this, &CanonicalAdmission, &Staged, &Diagnostic, &Inventory](
			const std::string& Cause) -> bool_t
	{
		VALTAN_PATTERN_SHAKE_CUE_DOCUMENT StagedPatternShakes;
		VALTAN_COMBAT_OBJECT_SOUND_CUE_DOCUMENT StagedCombatObjectSounds;
		std::string StagedShakeStatus;
		std::string StagedCombatObjectSoundStatus;
		const bool_t bStagedPatternShakesReady =
			CValtanPatternShakeCueDocument::Load_Source(
				StagedPatternShakes, StagedShakeStatus);
		const bool_t bStagedCombatObjectSoundsReady =
			CValtanCombatObjectSoundCueDocument::Load_Source(
				StagedCombatObjectSounds, StagedCombatObjectSoundStatus);
		std::string CurrentStatus;
		if (!CanonicalAdmission.Validate_StillCurrent(CurrentStatus))
		{
			const bool_t bPreserved =
				0u != m_CanonicalView.Get_PatternCount();
			m_eAdmission = bPreserved ? ADMISSION_STATE::STALE_PRESERVED :
				ADMISSION_STATE::REJECTED;
			m_strStatus = bPreserved ?
				"Product/source join failed and the candidate Product generation changed before display commit; the previous display snapshot was preserved: " +
					CurrentStatus + " " + Cause :
				"Product/source join failed and no stable Product display generation was available: " +
					CurrentStatus + " " + Cause;
			return false;
		}

		const bool_t bPreserved = 0u != m_CanonicalView.Get_PatternCount();
		if (!bPreserved)
		{
			m_CanonicalView = std::move(Staged);
			m_PlayableInventory = Inventory;
			++m_iCanonicalDisplayGeneration;
			Invalidate_SourceSequenceOwnerIndex();
			m_strPinnedAuthoringSourceRevision.clear();
			m_strPinnedCanonicalSourceRevision.clear();
			m_bAuthoringDraftDirty = false;
			m_bPatternSoundDependencyDirty = false;
			m_PatternSoundEvents.clear();
			m_PatternShakes = std::move(StagedPatternShakes);
			m_bPatternShakesReady = bStagedPatternShakesReady;
			m_strShakeStatus = std::move(StagedShakeStatus);
			m_CombatObjectSounds = std::move(StagedCombatObjectSounds);
			m_bCombatObjectSoundsReady =
				bStagedCombatObjectSoundsReady;
			m_strCombatObjectSoundStatus =
				std::move(StagedCombatObjectSoundStatus);
			m_strDisplayProvenance =
				"PRODUCT_ONLY / canonical Product read admission; typed authoring and Pattern Sound owners are not joined.";
			Reset_EffectCueEditor();
			m_SemanticValtanEffectAssetIds.clear();
			m_bSemanticValtanEffectLoadAttempted = false;
			Invalidate_TimelineCache();
			Normalize_Selection();
		}
		m_eAdmission = ADMISSION_STATE::STALE_PRESERVED;
		m_bConfirmDiscardPatternSoundDraft = false;
		m_strStatus = bPreserved ?
			"Typed source-owner join failed; the previous display snapshot was preserved read-only without mixing generations. Provenance: " +
				m_strDisplayProvenance + " Cause: " + Cause :
			"Canonical Product display loaded read-only: " +
				std::to_string(m_CanonicalView.Get_PatternCount()) +
				" patterns / " +
				std::to_string(m_CanonicalView.Get_StageCount()) +
				" stages / " +
				std::to_string(Inventory.Get_PatternCount()) +
				" Complete Play inventory rows. Provenance: " +
				m_strDisplayProvenance + " Cause: " + Cause + " " + Diagnostic;
		return false;
	};

	std::string AuthoringStatus;
	if (nullptr == m_pBalanceTool ||
		!m_pBalanceTool->Reload_ValtanSource(AuthoringStatus) ||
		!m_pBalanceTool->Get_ValtanAuthoringState(
			AuthoringRevision, bAuthoringDirty, AuthoringStatus) ||
		!m_pBalanceTool->Get_ValtanCanonicalSourceRevision(
			CanonicalSourceRevision, AuthoringStatus))
	{
		return PreserveOrCommitReadOnlyProduct(
			"Typed authoring source reload/join rejected: " + AuthoringStatus);
	}
	if (!m_pBalanceTool->Verify_ValtanCanonicalSourceRevision_WhileAdmitted(
			CanonicalAdmission, CanonicalSourceRevision, Diagnostic))
	{
		return PreserveOrCommitReadOnlyProduct(
			"Canonical Balance/Product generation join rejected: " + Diagnostic);
	}
	if (nullptr == m_pAnimationTool ||
		!m_pAnimationTool->Reload_ValtanCompositionPatternSounds(
			m_strSoundStatus))
	{
		return PreserveOrCommitReadOnlyProduct(
			"Pattern Sound source join rejected: " + m_strSoundStatus);
	}
	VALTAN_PATTERN_SHAKE_CUE_DOCUMENT StagedPatternShakes;
	VALTAN_COMBAT_OBJECT_SOUND_CUE_DOCUMENT StagedCombatObjectSounds;
	std::string StagedShakeStatus;
	std::string StagedCombatObjectSoundStatus;
	const bool_t bStagedPatternShakesReady =
		CValtanPatternShakeCueDocument::Load_Source(
			StagedPatternShakes, StagedShakeStatus);
	const bool_t bStagedCombatObjectSoundsReady =
		CValtanCombatObjectSoundCueDocument::Load_Source(
			StagedCombatObjectSounds, StagedCombatObjectSoundStatus);
	if (!CanonicalAdmission.Validate_StillCurrent(CanonicalAdmissionStatus))
	{
		const bool_t bPreserved = 0u != m_CanonicalView.Get_PatternCount();
		m_eAdmission = bPreserved ? ADMISSION_STATE::STALE_PRESERVED :
			ADMISSION_STATE::REJECTED;
		m_strStatus = bPreserved ?
			"Canonical generation changed before Workbench commit; previous composition remains read-only: " +
				CanonicalAdmissionStatus :
			"Canonical generation changed before Workbench commit: " +
				CanonicalAdmissionStatus;
		return false;
	}

	m_CanonicalView = std::move(Staged);
	m_PlayableInventory = Inventory;
	++m_iCanonicalDisplayGeneration;
	Invalidate_SourceSequenceOwnerIndex();
	m_strPinnedAuthoringSourceRevision = std::move(AuthoringRevision);
	m_strPinnedCanonicalSourceRevision =
		std::move(CanonicalSourceRevision);
	m_bAuthoringDraftDirty = false;
	m_eAdmission = ADMISSION_STATE::ADMITTED;
	m_bConfirmDiscardPatternSoundDraft = false;
	m_strDisplayProvenance =
		"FULL_JOIN / canonical Product + typed authoring source + Pattern Sound source; authoring revision " +
		m_strPinnedAuthoringSourceRevision + ", canonical source revision " +
		m_strPinnedCanonicalSourceRevision + ".";
	m_PatternSoundEvents =
		m_pAnimationTool->Collect_ValtanCompositionPatternSoundEvents();
	m_bSoundFilterDirty = true;
	m_PatternShakes = std::move(StagedPatternShakes);
	m_bPatternShakesReady = bStagedPatternShakesReady;
	m_strShakeStatus = std::move(StagedShakeStatus);
	m_CombatObjectSounds = std::move(StagedCombatObjectSounds);
	m_bCombatObjectSoundsReady = bStagedCombatObjectSoundsReady;
	m_strCombatObjectSoundStatus =
		std::move(StagedCombatObjectSoundStatus);
	std::string RuntimeGateStatus;
	bool_t bSoundRuntimeApplied = false;
	LostArk::Shared::GameplayDataRevision ExpectedSoundRuntimeRevision{};
	if (nullptr != m_pBossTool &&
		m_pBossTool->Get_ServerActivePatternRevision(
			ExpectedSoundRuntimeRevision, RuntimeGateStatus))
	{
		std::string RuntimeApplyStatus;
		bSoundRuntimeApplied =
			m_pAnimationTool->
				Retry_ValtanCompositionPatternSoundRuntimeApply(
					ExpectedSoundRuntimeRevision,
					RuntimeApplyStatus);
		if (bSoundRuntimeApplied)
		{
			LostArk::Shared::GameplayDataRevision
				PostSoundRuntimeRevision{};
			std::string PostRuntimeGateStatus;
			const bool_t bPostRevisionAdmitted =
				m_pBossTool->Get_ServerActivePatternRevision(
					PostSoundRuntimeRevision,
					PostRuntimeGateStatus);
			if (!bPostRevisionAdmitted ||
				PostSoundRuntimeRevision != ExpectedSoundRuntimeRevision)
			{
				RuntimeApplyStatus =
					"Pattern Sound consumer reload lost its exact Server revision admission before commit. Expected " +
					LostArk::Shared::Format_GameplayDataRevision(
						ExpectedSoundRuntimeRevision) + ", observed " +
					LostArk::Shared::Format_GameplayDataRevision(
						PostSoundRuntimeRevision) + ". " +
					PostRuntimeGateStatus;
				m_pAnimationTool->
					Invalidate_ValtanCompositionPatternSoundRuntimeApply(
						RuntimeApplyStatus);
				bSoundRuntimeApplied = false;
			}
		}
		m_strSoundStatus = bSoundRuntimeApplied ?
			"Pattern Sound source and active consumers admitted against the exact Server-active revision. " +
				RuntimeApplyStatus :
			"Pattern Sound source is admitted, but active consumer apply was rejected; Server playback remains blocked: " +
				RuntimeApplyStatus;
		if (bSoundRuntimeApplied)
		{
			m_PatternSoundEvents =
				m_pAnimationTool->Collect_ValtanCompositionPatternSoundEvents();
			m_bSoundFilterDirty = true;
		}
	}
	else
	{
		m_strSoundStatus =
			"Pattern Sound source admitted data-only. Active Client presentation apply is deferred until this exact immutable revision is Server-active. " +
			RuntimeGateStatus;
	}
	m_strStatus = "Canonical composition admitted: " +
		std::to_string(m_CanonicalView.Get_PatternCount()) + " patterns / " +
		std::to_string(m_CanonicalView.Get_StageCount()) + " stages / " +
		std::to_string(Inventory.Get_PatternCount()) +
		" Complete Play patterns. " + Diagnostic + " " + m_strSoundStatus;
	/* A stable cue identity may now describe different source fields. Never let
	   the previous generation's editable value copy overwrite a fresh admit. */
	Reset_EffectCueEditor();
	m_SemanticValtanEffectAssetIds.clear();
	m_bSemanticValtanEffectLoadAttempted = false;
	Invalidate_TimelineCache();
	Normalize_Selection();
	return true;
}

void Client::CActionCompositionWorkbench::Select_Pattern(
	const VALTAN_PATTERN_VIEW& Pattern)
{
	m_strSelectedPatternId = Pattern.strPatternId;
	m_strSelectedStageId = Pattern.Stages.empty() ?
		std::string{} : Pattern.Stages.front().strStageId;
	m_strSelectedStableId = Pattern.strPatternId;
	m_eDetailOwner = DETAIL_OWNER::PATTERN;
	m_bDetailFocusRequested = false;
	m_iPlayheadMs = 0u;
	m_strSoundAddClipOccurrenceId.clear();
	m_iSoundAddStartMs = 0u;
	m_eSoundAddRepeatPolicy = VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE;
	Reset_EffectCueEditor();
	/* Pattern can alias the immutable effective snapshot used by later windows
	   in this same frame.  Keep that storage alive; the selected stable ID makes
	   the cache key miss next frame, when it is safe to replace the snapshot. */
	Invalidate_TimelineCache();
}

void Client::CActionCompositionWorkbench::Select_Stage(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage,
	const DETAIL_OWNER eOwner,
	const std::string& strStableId)
{
	m_strSelectedPatternId = Pattern.strPatternId;
	m_strSelectedStageId = Stage.strStageId;
	m_eDetailOwner = eOwner;
	m_strSelectedStableId = strStableId.empty() ?
		Stage.strStageId : strStableId;
	m_bDetailFocusRequested = true;
	m_strSoundAddClipOccurrenceId.clear();
	m_iSoundAddStartMs = 0u;
	m_eSoundAddRepeatPolicy = VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE;
	Reset_EffectCueEditor();
}

bool_t Client::CActionCompositionWorkbench::Reload_AnimationSequences()
{
	m_bAnimationSequenceLoadAttempted = true;
	m_FilteredAnimationSequenceIndices.clear();
	m_bAnimationSequenceFilterDirty = true;
	if (nullptr == m_pAnimationTool)
	{
		m_AnimationSequences.clear();
		m_strAnimationSequenceStatus =
			"Animation preview owner is unavailable.";
		return false;
	}
	std::vector<CAnimation_Tool::COMPOSITION_SEQUENCE_VIEW> Staged;
	if (!m_pAnimationTool->Get_ValtanCompositionSequences(
			Staged, m_strAnimationSequenceStatus))
	{
		m_AnimationSequences.clear();
		return false;
	}
	m_AnimationSequences = std::move(Staged);
	if (m_AnimationSequences.empty())
	{
		m_iSelectedSequenceSkillId = -1;
		m_iSelectedSequenceIndex = -1;
		return false;
	}
	const auto Selected = std::find_if(
		m_AnimationSequences.begin(), m_AnimationSequences.end(),
		[this](const CAnimation_Tool::COMPOSITION_SEQUENCE_VIEW& Sequence)
		{
			return Sequence.iSkillId == m_iSelectedSequenceSkillId &&
				Sequence.iSequenceIndex == m_iSelectedSequenceIndex;
		});
	if (Selected == m_AnimationSequences.end())
	{
		m_iSelectedSequenceSkillId = m_AnimationSequences.front().iSkillId;
		m_iSelectedSequenceIndex = m_AnimationSequences.front().iSequenceIndex;
	}
	return true;
}

bool_t Client::CActionCompositionWorkbench::Apply_SelectedSequenceToStage(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage,
	const bool_t bAppend)
{
	std::string SoundStatus;
	if (Is_PatternSoundDraftDirty(SoundStatus))
	{
		m_strStatus =
			"Sequence slot edit blocked: save or discard the dependency-qualified Pattern Sound draft first. " +
			SoundStatus;
		return false;
	}
	if (ADMISSION_STATE::ADMITTED != m_eAdmission ||
		nullptr == m_pBalanceTool || nullptr == m_pAnimationTool)
	{
		m_strStatus =
			"Sequence slot edit blocked until canonical admission and the typed authoring/native model owners are available.";
		return false;
	}
	if ("WAIT" == Stage.strSequenceRole)
	{
		m_strStatus =
			"Sequence slot edit rejected: WAIT is an explicit blank Stage and must remain Animation NONE. Insert or select an ACTIVE Stage before assigning a Sequence.";
		return false;
	}
	const auto Selected = std::find_if(
		m_AnimationSequences.begin(), m_AnimationSequences.end(),
		[this](const CAnimation_Tool::COMPOSITION_SEQUENCE_VIEW& Sequence)
		{
			return Sequence.iSkillId == m_iSelectedSequenceSkillId &&
				Sequence.iSequenceIndex == m_iSelectedSequenceIndex;
		});
	if (Selected == m_AnimationSequences.end() || Selected->Clips.empty())
	{
		m_strStatus = "Select one non-empty extracted Animation Sequence first.";
		return false;
	}
	CBalanceTool::PATTERN_STAGE_EDIT Draft;
	std::string Status;
	if (!m_pBalanceTool->Get_ValtanStageDraft(
			Pattern.strPatternId, Stage.strStageId, Draft, Status) ||
		!Draft.animationEditable)
	{
		m_strStatus = Status.empty() ?
			"This Stage does not admit Animation slot edits." : std::move(Status);
		return false;
	}
	if (bAppend && std::any_of(
			Draft.animationSlots.begin(), Draft.animationSlots.end(),
			[](const CBalanceTool::ANIMATION_SLOT_EDIT& Slot)
			{
				return 0u == Slot.playMs || Slot.repeatUntilStageEnd;
			}))
	{
		m_strStatus =
			"Append rejected: convert the current looping/native-duration slot to an exact wall-clock first.";
		return false;
	}
	std::vector<uint32_t> NativeDurationsMs;
	NativeDurationsMs.reserve(Selected->Clips.size());
	std::vector<uint32_t> RequestedCutsMs;
	RequestedCutsMs.reserve(Selected->Clips.size());
	for (std::size_t iClip = 0u; iClip < Selected->Clips.size(); ++iClip)
	{
		const CAnimation_Tool::COMPOSITION_SEQUENCE_CLIP_VIEW& Clip =
			Selected->Clips[iClip];
		if (0u == Clip.iDurationMs)
		{
			m_strStatus = "Sequence slot edit rejected: source duration is unresolved for " +
				Clip.strClipName + ". Open Animation Clips and review its native timing.";
			return false;
		}
		uint32_t iNativeDurationMs = 0u;
		if (!m_pAnimationTool->Resolve_ValtanCompositionNativeClipDurationMs(
				Clip.strClipName, iNativeDurationMs, Status))
		{
			m_strStatus =
				"Sequence slot edit rejected before the typed draft changed: " + Status;
			return false;
		}
		NativeDurationsMs.push_back(iNativeDurationMs);
		RequestedCutsMs.push_back(Clip.iDurationMs);
	}

	std::vector<uint32_t> FittedCutsMs = RequestedCutsMs;
	std::string FitStatus;
	const uint64_t iRequestedSequenceMs = std::accumulate(
		RequestedCutsMs.begin(), RequestedCutsMs.end(), uint64_t{ 0u });
	if (!bAppend && iRequestedSequenceMs > Draft.durationMs)
	{
		const bool_t bDeterministicHoldChain =
			3u == Selected->Clips.size() &&
			"start" == ClipReplacementRole(
				Selected->Clips[0u].strClipName) &&
			"loop" == ClipReplacementRole(
				Selected->Clips[1u].strClipName) &&
			"end" == ClipReplacementRole(
				Selected->Clips[2u].strClipName);
		if ("HOLD" != Selected->strMode || !bDeterministicHoldChain ||
			!FitCompositionSequenceCutsToStage(
				RequestedCutsMs, Draft.durationMs, FittedCutsMs, FitStatus))
		{
			m_strStatus = FitStatus.empty() ?
				"Replace rejected: the selected Sequence exceeds the existing Server Stage clock and has no deterministic HOLD-chain fit policy." :
				std::move(FitStatus);
			return false;
		}
	}

	struct EXPANDED_SEQUENCE_CLIP final
	{
		std::string strClip;
		uint32_t iPlayMs = 0u;
	};
	std::vector<EXPANDED_SEQUENCE_CLIP> ExpandedClips;
	for (std::size_t iClip = 0u; iClip < Selected->Clips.size(); ++iClip)
	{
		uint32_t iRemainingMs = FittedCutsMs[iClip];
		if (iRemainingMs > NativeDurationsMs[iClip] &&
			"loop" != ClipReplacementRole(
				Selected->Clips[iClip].strClipName))
		{
			m_strStatus =
				"Sequence slot edit rejected: only an explicit _loop clip may be materialized more than once; start/end/one-shot clips are never repeated by inference.";
			return false;
		}
		while (iRemainingMs > NativeDurationsMs[iClip])
		{
			ExpandedClips.push_back({
				Selected->Clips[iClip].strClipName,
				NativeDurationsMs[iClip] });
			iRemainingMs -= NativeDurationsMs[iClip];
			if (ExpandedClips.size() > 4096u)
			{
				m_strStatus =
					"Sequence slot edit rejected: native-loop materialization exceeds 4096 exact occurrences.";
				return false;
			}
		}
		if (0u != iRemainingMs)
		{
			ExpandedClips.push_back({
				Selected->Clips[iClip].strClipName, iRemainingMs });
		}
	}
	if (ExpandedClips.empty())
	{
		m_strStatus = "Sequence slot edit rejected: fitted Sequence is empty.";
		return false;
	}

	std::vector<CBalanceTool::ANIMATION_SLOT_EDIT> Slots =
		bAppend ? Draft.animationSlots :
		std::vector<CBalanceTool::ANIMATION_SLOT_EDIT>{};
	const std::size_t iExistingCount = Draft.animationSlots.size();
	/* Replace treats an occurrence ID as the stable logical timeline slot.  It
	   preserves an exact clip first and then a unique semantic start/loop/end
	   role.  Ambiguous repeated roles deliberately receive new IDs so a linked
	   Effect/Sound/Shake row can fail closed instead of silently following an
	   arbitrary loop.  A changed unique role is marked PROJECT_AUTHORED and all
	   dependency source windows are revalidated before the draft changes. */
	std::vector<CBalanceTool::ANIMATION_SLOT_EDIT> ReservedSlots =
		Draft.animationSlots;
	std::vector<bool_t> ReusedExistingSlots(iExistingCount, false);
	std::vector<std::size_t> ReusableSlots(
		ExpandedClips.size(), iExistingCount);
	if (!bAppend)
	{
		for (std::size_t iClip = 0u; iClip < ExpandedClips.size(); ++iClip)
		{
			for (std::size_t iExisting = 0u;
				iExisting < iExistingCount; ++iExisting)
			{
				if (!ReusedExistingSlots[iExisting] &&
					Draft.animationSlots[iExisting].clip ==
						ExpandedClips[iClip].strClip)
				{
					ReusableSlots[iClip] = iExisting;
					ReusedExistingSlots[iExisting] = true;
					break;
				}
			}
		}
		for (std::size_t iClip = 0u; iClip < ExpandedClips.size(); ++iClip)
		{
			if (ReusableSlots[iClip] < iExistingCount)
				continue;
			const std::string Role =
				ClipReplacementRole(ExpandedClips[iClip].strClip);
			if (Role.empty())
				continue;
			const std::size_t iUnassignedNewRoleCount =
				static_cast<std::size_t>(std::count_if(
					ExpandedClips.begin(), ExpandedClips.end(),
					[&ExpandedClips, &ReusableSlots, iExistingCount, &Role](
						const EXPANDED_SEQUENCE_CLIP& Candidate)
					{
						const std::size_t iCandidate = static_cast<std::size_t>(
							&Candidate - ExpandedClips.data());
						return ReusableSlots[iCandidate] == iExistingCount &&
							Role == ClipReplacementRole(Candidate.strClip);
					}));
			std::size_t iUniqueExisting = iExistingCount;
			std::size_t iUnassignedExistingRoleCount = 0u;
			for (std::size_t iExisting = 0u;
				iExisting < iExistingCount; ++iExisting)
			{
				if (!ReusedExistingSlots[iExisting] &&
					Role == ClipReplacementRole(
						Draft.animationSlots[iExisting].clip))
				{
					iUniqueExisting = iExisting;
					++iUnassignedExistingRoleCount;
				}
			}
			if (1u == iUnassignedNewRoleCount &&
				1u == iUnassignedExistingRoleCount)
			{
				ReusableSlots[iClip] = iUniqueExisting;
				ReusedExistingSlots[iUniqueExisting] = true;
			}
		}
	}
	uint64_t iDurationMs = 0u;
	for (const CBalanceTool::ANIMATION_SLOT_EDIT& Slot : Slots)
		iDurationMs += static_cast<uint64_t>(std::llround(
			static_cast<double>(Slot.playMs) / Slot.playRate));
	for (std::size_t iClip = 0u; iClip < ExpandedClips.size(); ++iClip)
	{
		CBalanceTool::ANIMATION_SLOT_EDIT Slot;
		bool_t bReusedIdentity = false;
		const std::size_t iReusableSlot = ReusableSlots[iClip];
		if (iReusableSlot < iExistingCount)
		{
			Slot.clipOccurrenceId =
				Draft.animationSlots[iReusableSlot].clipOccurrenceId;
			Slot.mappingBasis =
				Draft.animationSlots[iReusableSlot].clip ==
					ExpandedClips[iClip].strClip ?
					Draft.animationSlots[iReusableSlot].mappingBasis :
					"PROJECT_AUTHORED";
			bReusedIdentity = true;
		}
		else
		{
			Slot.clipOccurrenceId = BuildNextCompositionSlotId(
				Pattern.strPatternId, Stage.strStageId, ReservedSlots);
			if (Slot.clipOccurrenceId.empty())
			{
				m_strStatus =
					"Sequence slot edit rejected: no free stable occurrence ID is available.";
				return false;
			}
			Slot.mappingBasis = "PROJECT_AUTHORED";
		}
		Slot.clip = ExpandedClips[iClip].strClip;
		Slot.sourceStartMs = 0u;
		Slot.playRate = 1.0;
		Slot.playMs = ExpandedClips[iClip].iPlayMs;
		Slot.repeatUntilStageEnd = false;
		if (!bReusedIdentity)
			ReservedSlots.push_back(Slot);
		Slots.push_back(std::move(Slot));
		iDurationMs += ExpandedClips[iClip].iPlayMs;
	}
	if (iDurationMs < 1u || iDurationMs > 600000u)
	{
		m_strStatus = "Sequence slot edit rejected: combined wall clock is outside 1..600000 ms.";
		return false;
	}
	Draft.animationSlots = std::move(Slots);
	if (bAppend)
		Draft.durationMs = static_cast<uint32_t>(iDurationMs);
	Draft.animationEndPolicy = iDurationMs == Draft.durationMs ?
		"EXACT" : "HOLD_LAST_POSE";
	Draft.animationRepeatCount = 1u;
	if (!SetValtanStageDraftWithSoundDependencyAdmission(
			m_pAnimationTool, m_pBalanceTool,
			m_bPatternShakesReady ? &m_PatternShakes : nullptr,
			Pattern, Stage, Draft, Status))
	{
		m_strStatus = std::move(Status);
		return false;
	}
	if (!Draft.animationSlots.empty())
	{
		const std::size_t iSelectedSlot = bAppend ?
			(std::min)(iExistingCount, Draft.animationSlots.size() - 1u) : 0u;
		m_strSelectedStableId =
			Draft.animationSlots[iSelectedSlot].clipOccurrenceId;
		m_eDetailOwner = DETAIL_OWNER::ANIMATION;
	}
	m_strStatus = std::string(bAppend ? "Appended " : "Replaced with ") +
		std::to_string(ExpandedClips.size()) +
		" exact Sequence slots in the shared authoring draft; Server Stage clock " +
		std::to_string(Draft.durationMs) + " ms was " +
		(bAppend ? "extended" : "preserved") +
		(FitStatus.empty() ? ". " : ". " + FitStatus + " ") +
		"Press Save & Apply.";
	Invalidate_TimelineCache();
	return true;
}

bool_t Client::CActionCompositionWorkbench::Remove_AnimationOccurrence(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage,
	const std::string& strClipOccurrenceId,
	std::string& strOutStatus)
{
	if (nullptr == m_pAnimationTool || nullptr == m_pBalanceTool ||
		strClipOccurrenceId.empty())
	{
		strOutStatus =
			"Delete requires one selected Animation box and both typed owners.";
		return false;
	}
	CBalanceTool::PATTERN_STAGE_EDIT Draft;
	if (!m_pBalanceTool->Get_ValtanStageDraft(
			Pattern.strPatternId, Stage.strStageId, Draft, strOutStatus))
	{
		return false;
	}
	const auto Found = std::find_if(
		Draft.animationSlots.begin(), Draft.animationSlots.end(),
		[&strClipOccurrenceId](
			const CBalanceTool::ANIMATION_SLOT_EDIT& Slot)
		{
			return Slot.clipOccurrenceId == strClipOccurrenceId;
		});
	if (Draft.animationSlots.end() == Found)
	{
		strOutStatus =
			"Delete rejected: the selected Animation box is not in the current Stage draft.";
		return false;
	}
	Draft.animationSlots.erase(Found);
	if (Draft.animationSlots.empty())
	{
		Draft.animationEndPolicy = "NONE";
		Draft.animationRepeatCount = 0u;
	}
	else
	{
		uint32_t iWallMs = 0u;
		if (ComputeExactAnimationWallMs(Draft, iWallMs))
		{
			if (iWallMs > Draft.durationMs)
			{
				strOutStatus =
					"Delete rejected: remaining Animation boxes exceed the preserved Server Stage clock.";
				return false;
			}
			Draft.animationEndPolicy = iWallMs == Draft.durationMs ?
				"EXACT" : "HOLD_LAST_POSE";
		}
		Draft.animationRepeatCount = 1u;
	}
	if (!SetValtanStageDraftWithSoundDependencyAdmission(
			m_pAnimationTool, m_pBalanceTool,
			m_bPatternShakesReady ? &m_PatternShakes : nullptr,
			Pattern, Stage, Draft, strOutStatus))
	{
		return false;
	}
	m_strSelectedStableId = Draft.animationSlots.empty() ?
		Stage.strStageId : Draft.animationSlots.front().clipOccurrenceId;
	m_eDetailOwner = Draft.animationSlots.empty() ?
		DETAIL_OWNER::GAMEPLAY_STAGE : DETAIL_OWNER::ANIMATION;
	Invalidate_TimelineCache();
	strOutStatus =
		"Deleted the selected Animation box; the Server Stage clock was preserved. Use Save & Apply when ready.";
	return true;
}

bool_t Client::CActionCompositionWorkbench::Duplicate_AnimationOccurrence(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage,
	const std::string& strClipOccurrenceId,
	std::string& strOutStatus)
{
	if (nullptr == m_pAnimationTool || nullptr == m_pBalanceTool ||
		strClipOccurrenceId.empty())
	{
		strOutStatus =
			"Duplicate requires one selected Animation box and both typed owners.";
		return false;
	}
	CBalanceTool::PATTERN_STAGE_EDIT Draft;
	if (!m_pBalanceTool->Get_ValtanStageDraft(
			Pattern.strPatternId, Stage.strStageId, Draft, strOutStatus))
	{
		return false;
	}
	const auto Found = std::find_if(
		Draft.animationSlots.begin(), Draft.animationSlots.end(),
		[&strClipOccurrenceId](
			const CBalanceTool::ANIMATION_SLOT_EDIT& Slot)
		{
			return Slot.clipOccurrenceId == strClipOccurrenceId;
		});
	if (Draft.animationSlots.end() == Found ||
		Found->repeatUntilStageEnd || 0u == Found->playMs)
	{
		strOutStatus =
			"Duplicate rejected: select one finite Animation box; an open-ended loop must be converted to exact occurrences first.";
		return false;
	}
	CBalanceTool::ANIMATION_SLOT_EDIT Duplicate = *Found;
	Duplicate.clipOccurrenceId = BuildNextCompositionSlotId(
		Pattern.strPatternId, Stage.strStageId, Draft.animationSlots);
	Duplicate.mappingBasis = "PROJECT_AUTHORED";
	if (Duplicate.clipOccurrenceId.empty())
	{
		strOutStatus =
			"Duplicate rejected: no free stable Animation occurrence ID is available.";
		return false;
	}
	const std::size_t iInsertIndex = static_cast<std::size_t>(
		Found - Draft.animationSlots.begin()) + 1u;
	Draft.animationSlots.insert(
		Draft.animationSlots.begin() +
			static_cast<std::ptrdiff_t>(iInsertIndex), Duplicate);
	uint32_t iAnimationWallMs = 0u;
	if (!ComputeExactAnimationWallMs(Draft, iAnimationWallMs))
	{
		strOutStatus =
			"Duplicate rejected: the resulting Animation boxes do not have one finite exact clock.";
		return false;
	}
	if (iAnimationWallMs > Draft.durationMs)
		Draft.durationMs = iAnimationWallMs;
	Draft.animationEndPolicy = iAnimationWallMs == Draft.durationMs ?
		"EXACT" : "HOLD_LAST_POSE";
	const bool_t bOneRepeatedClip = Draft.animationSlots.size() > 1u &&
		std::all_of(
			Draft.animationSlots.begin() + 1u, Draft.animationSlots.end(),
			[&Draft](const CBalanceTool::ANIMATION_SLOT_EDIT& Slot)
			{
				return Slot.clip == Draft.animationSlots.front().clip;
			});
	Draft.animationRepeatCount = bOneRepeatedClip ?
		static_cast<uint32_t>(Draft.animationSlots.size()) : 1u;
	if (!SetValtanStageDraftWithSoundDependencyAdmission(
			m_pAnimationTool, m_pBalanceTool,
			m_bPatternShakesReady ? &m_PatternShakes : nullptr,
			Pattern, Stage, Draft, strOutStatus))
	{
		return false;
	}
	m_eDetailOwner = DETAIL_OWNER::ANIMATION;
	m_strSelectedStableId = Duplicate.clipOccurrenceId;
	Invalidate_TimelineCache();
	strOutStatus =
		"Duplicated the selected Animation box immediately after its source; the Server Stage clock now matches the exact boxes. Use Save & Apply when ready.";
	return true;
}

bool_t Client::CActionCompositionWorkbench::Reorder_AnimationOccurrence(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage,
	const std::string& strClipOccurrenceId,
	const std::size_t iTargetIndex,
	std::string& strOutStatus)
{
	if (nullptr == m_pAnimationTool || nullptr == m_pBalanceTool ||
		strClipOccurrenceId.empty())
	{
		strOutStatus =
			"Animation drag requires one selected box and both typed owners.";
		return false;
	}
	CBalanceTool::PATTERN_STAGE_EDIT Draft;
	if (!m_pBalanceTool->Get_ValtanStageDraft(
			Pattern.strPatternId, Stage.strStageId, Draft, strOutStatus))
	{
		return false;
	}
	const auto Found = std::find_if(
		Draft.animationSlots.begin(), Draft.animationSlots.end(),
		[&strClipOccurrenceId](
			const CBalanceTool::ANIMATION_SLOT_EDIT& Slot)
		{
			return Slot.clipOccurrenceId == strClipOccurrenceId;
		});
	if (Draft.animationSlots.end() == Found)
	{
		strOutStatus =
			"Animation drag rejected: the selected box is not in the current Stage draft.";
		return false;
	}
	const std::size_t iSourceIndex = static_cast<std::size_t>(
		Found - Draft.animationSlots.begin());
	const std::size_t iClampedTarget = (std::min)(
		iTargetIndex, Draft.animationSlots.size() - 1u);
	if (iSourceIndex == iClampedTarget)
	{
		strOutStatus = "Animation box order is unchanged.";
		return true;
	}
	CBalanceTool::ANIMATION_SLOT_EDIT Moving = std::move(*Found);
	Draft.animationSlots.erase(
		Draft.animationSlots.begin() +
			static_cast<std::ptrdiff_t>(iSourceIndex));
	Draft.animationSlots.insert(
		Draft.animationSlots.begin() +
			static_cast<std::ptrdiff_t>((std::min)(
				iClampedTarget, Draft.animationSlots.size())),
		std::move(Moving));
	if (!SetValtanStageDraftWithSoundDependencyAdmission(
			m_pAnimationTool, m_pBalanceTool,
			m_bPatternShakesReady ? &m_PatternShakes : nullptr,
			Pattern, Stage, Draft, strOutStatus))
	{
		return false;
	}
	Invalidate_TimelineCache();
	strOutStatus =
		"Reordered the selected Animation box; linked Effect/Sound/Shake rows kept the same stable occurrence. Use Save & Apply when ready.";
	return true;
}

bool_t Client::CActionCompositionWorkbench::Duplicate_SelectedTimelineBox(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage,
	std::string& strOutStatus)
{
	switch (m_eDetailOwner)
	{
	case DETAIL_OWNER::ANIMATION:
		return Duplicate_AnimationOccurrence(
			Pattern, Stage, m_strSelectedStableId, strOutStatus);
	case DETAIL_OWNER::EFFECT:
	{
		if (nullptr == m_pBalanceTool)
		{
			strOutStatus = "Effect Duplicate requires the typed Pattern owner.";
			return false;
		}
		const std::size_t iMatchCount = static_cast<std::size_t>(std::count_if(
			Stage.ProductCues.begin(), Stage.ProductCues.end(),
			[this](const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue)
			{ return Cue.strOccurrenceId == m_strSelectedStableId; }));
		const auto Found = std::find_if(
			Stage.ProductCues.begin(), Stage.ProductCues.end(),
			[this](const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue)
			{ return Cue.strOccurrenceId == m_strSelectedStableId; });
		if (1u != iMatchCount || Stage.ProductCues.end() == Found)
		{
			strOutStatus =
				"Effect Duplicate requires one exact selected invocation box.";
			return false;
		}
		VALTAN_PRODUCT_EFFECT_CUE_VIEW Duplicate = *Found;
		Duplicate.strBindingId = BuildNextCompositionEffectCueId(Pattern, Stage);
		if (Duplicate.strBindingId.empty())
		{
			strOutStatus =
				"Effect Duplicate rejected: no free stable invocation ID is available.";
			return false;
		}
		Duplicate.strOccurrenceId =
			Duplicate.strBindingId + ".occurrence.01";
		if (!m_pBalanceTool->Add_ValtanStageEffectCue(
				Pattern.strPatternId, Stage.strStageId, Stage.strActionId,
				Duplicate, strOutStatus))
		{
			return false;
		}
		m_eDetailOwner = DETAIL_OWNER::EFFECT;
		m_strSelectedStableId = Duplicate.strOccurrenceId;
		m_strEffectEditIdentity.clear();
		Invalidate_TimelineCache();
		strOutStatus =
			"Duplicated the selected Effect box at the same exact occurrence time; drag or trim the copy, then Save & Apply.";
		return true;
	}
	case DETAIL_OWNER::SOUND:
	{
		if (nullptr == m_pAnimationTool)
		{
			strOutStatus = "Sound Duplicate requires the typed Sound owner.";
			return false;
		}
		bool_t bDirty = false;
		std::string DraftStatus;
		const VALTAN_PATTERN_SOUND_CUE_DOCUMENT* const pSounds =
			m_pAnimationTool->Get_ValtanCompositionPatternSoundDraft(
				bDirty, DraftStatus);
		if (nullptr == pSounds)
		{
			strOutStatus = std::move(DraftStatus);
			return false;
		}
		const auto Found = std::find_if(
			pSounds->Cues.begin(), pSounds->Cues.end(),
			[this, &Pattern, &Stage](const VALTAN_PATTERN_SOUND_CUE& Cue)
			{
				return Cue.strOccurrenceId == m_strSelectedStableId &&
					Cue.strPatternId == Pattern.strPatternId &&
					Cue.strStageId == Stage.strStageId &&
					Cue.strActionId == Stage.strActionId;
			});
		if (pSounds->Cues.end() == Found)
		{
			strOutStatus =
				"Sound Duplicate requires one exact selected Sound box.";
			return false;
		}
		VALTAN_PATTERN_SOUND_CUE_ROW_ID Created;
		if (!m_pAnimationTool->Add_ValtanCompositionPatternSound(
				Pattern, Stage, Found->strClipOccurrenceId,
				Found->strSoundEvent, Found->iStartMs,
				Found->eRepeatPolicy, Created, strOutStatus))
		{
			return false;
		}
		m_eDetailOwner = DETAIL_OWNER::SOUND;
		m_strSelectedStableId = Created.strOccurrenceId;
		m_strSoundStatus = strOutStatus;
		Invalidate_TimelineCache();
		return true;
	}
	default:
		strOutStatus =
			"Duplicate is available for a selected Animation, Effect, or Sound box.";
		return false;
	}
}

bool_t Client::CActionCompositionWorkbench::Delete_SelectedTimelineBox(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage,
	std::string& strOutStatus)
{
	switch (m_eDetailOwner)
	{
	case DETAIL_OWNER::ANIMATION:
		return Remove_AnimationOccurrence(
			Pattern, Stage, m_strSelectedStableId, strOutStatus);
	case DETAIL_OWNER::EFFECT:
	{
		if (nullptr == m_pBalanceTool)
		{
			strOutStatus = "Effect Delete requires the typed Pattern owner.";
			return false;
		}
		const auto Found = std::find_if(
			Stage.ProductCues.begin(), Stage.ProductCues.end(),
			[this](const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue)
			{ return Cue.strOccurrenceId == m_strSelectedStableId; });
		if (Stage.ProductCues.end() == Found)
		{
			strOutStatus = "Effect Delete requires one exact invocation box.";
			return false;
		}
		if (!m_pBalanceTool->Remove_ValtanStageEffectCue(
				Pattern.strPatternId, Stage.strStageId, Stage.strActionId,
				Found->strBindingId, Found->strOccurrenceId,
				Found->strEffectAssetId, Found->strClipOccurrenceId,
				strOutStatus))
		{
			return false;
		}
		m_strSelectedStableId = Stage.strStageId;
		m_strEffectEditIdentity.clear();
		Invalidate_TimelineCache();
		return true;
	}
	case DETAIL_OWNER::SOUND:
	{
		if (nullptr == m_pAnimationTool)
		{
			strOutStatus = "Sound Delete requires the typed Sound owner.";
			return false;
		}
		bool_t bDirty = false;
		std::string DraftStatus;
		const VALTAN_PATTERN_SOUND_CUE_DOCUMENT* const pSounds =
			m_pAnimationTool->Get_ValtanCompositionPatternSoundDraft(
				bDirty, DraftStatus);
		if (nullptr == pSounds)
		{
			strOutStatus = std::move(DraftStatus);
			return false;
		}
		const auto Found = std::find_if(
			pSounds->Cues.begin(), pSounds->Cues.end(),
			[this, &Pattern, &Stage](const VALTAN_PATTERN_SOUND_CUE& Cue)
			{
				return Cue.strOccurrenceId == m_strSelectedStableId &&
					Cue.strPatternId == Pattern.strPatternId &&
					Cue.strStageId == Stage.strStageId &&
					Cue.strActionId == Stage.strActionId;
			});
		if (pSounds->Cues.end() == Found)
		{
			strOutStatus = "Sound Delete requires one exact Sound box.";
			return false;
		}
		VALTAN_PATTERN_SOUND_CUE_ROW_ID RowId;
		RowId.strBindingId = Found->strBindingId;
		RowId.strOccurrenceId = Found->strOccurrenceId;
		if (!m_pAnimationTool->Remove_ValtanCompositionPatternSound(
				Pattern, Stage, RowId, strOutStatus))
		{
			return false;
		}
		m_strSelectedStableId = Stage.strStageId;
		m_strSoundStatus = strOutStatus;
		Invalidate_TimelineCache();
		return true;
	}
	default:
		strOutStatus =
			"Delete is available for a selected Animation, Effect, or Sound box.";
		return false;
	}
}

bool_t Client::CActionCompositionWorkbench::Apply_AnimationOccurrenceTiming(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage,
	const std::string& strClipOccurrenceId,
	const uint32_t iSourceStartMs,
	const uint32_t iPlayMs,
	std::string& strOutStatus)
{
	if (nullptr == m_pBalanceTool || nullptr == m_pAnimationTool)
	{
		strOutStatus =
			"Animation occurrence timing requires the typed Stage and native Animation owners.";
		return false;
	}
	if ("WAIT" == Stage.strSequenceRole)
	{
		strOutStatus =
			"WAIT Stage timing is read-only because WAIT must remain Animation NONE.";
		return false;
	}

	CBalanceTool::PATTERN_STAGE_EDIT Draft;
	if (!m_pBalanceTool->Get_ValtanStageDraft(
			Pattern.strPatternId, Stage.strStageId, Draft, strOutStatus))
	{
		return false;
	}
	if (!Draft.animationEditable)
	{
		strOutStatus =
			"This Stage does not admit typed Animation occurrence timing edits.";
		return false;
	}
	const std::size_t iMatchCount = static_cast<std::size_t>(std::count_if(
		Draft.animationSlots.begin(), Draft.animationSlots.end(),
		[&strClipOccurrenceId](const CBalanceTool::ANIMATION_SLOT_EDIT& Slot)
		{
			return Slot.clipOccurrenceId == strClipOccurrenceId;
		}));
	const auto Found = std::find_if(
		Draft.animationSlots.begin(), Draft.animationSlots.end(),
		[&strClipOccurrenceId](const CBalanceTool::ANIMATION_SLOT_EDIT& Slot)
		{
			return Slot.clipOccurrenceId == strClipOccurrenceId;
		});
	if (1u != iMatchCount || Draft.animationSlots.end() == Found)
	{
		strOutStatus =
			"Animation timing edit requires one exact stable clipOccurrenceId: " +
			strClipOccurrenceId + ".";
		return false;
	}
	if (Found->repeatUntilStageEnd && 0u != iPlayMs)
	{
		strOutStatus =
			"A LOOP_TO_STAGE_END occurrence must keep playMs=0; change the typed end policy in Details before making it finite.";
		return false;
	}
	if (Found->sourceStartMs == iSourceStartMs && Found->playMs == iPlayMs)
	{
		strOutStatus = "Animation occurrence timing is unchanged.";
		return true;
	}

	/* The stable occurrence identity, clip, mapping, order, play rate and loop
	   policy are intentionally untouched. Native source-window and every
	   clip-qualified Effect/Sound/Shake dependency are admitted before the
	   shared Balance draft can change. Stage duration/end policy are never
	   inferred from a source trim. */
	Found->sourceStartMs = iSourceStartMs;
	Found->playMs = iPlayMs;
	if (!SetValtanStageDraftWithSoundDependencyAdmission(
			m_pAnimationTool, m_pBalanceTool,
			m_bPatternShakesReady ? &m_PatternShakes : nullptr,
			Pattern, Stage, Draft, strOutStatus))
	{
		return false;
	}
	strOutStatus = "Staged Animation occurrence timing for " +
		strClipOccurrenceId +
		". Use Save & Apply to commit the typed source owner.";
	return true;
}

void Client::CActionCompositionWorkbench::Render_SelectedAnimationTiming(
	const VALTAN_PATTERN_VIEW& Pattern,
	const bool_t bMutationAdmitted)
{
	if (DETAIL_OWNER::ANIMATION != m_eDetailOwner ||
		m_strSelectedStableId.empty() || nullptr == m_pBalanceTool)
	{
		return;
	}
	const VALTAN_STAGE_VIEW* const pStage = Find_SelectedStage(&Pattern);
	if (nullptr == pStage)
		return;
	if (ADMISSION_STATE::ADMITTED != m_eAdmission)
	{
		const auto Pinned = std::find_if(
			pStage->ClipOccurrences.begin(), pStage->ClipOccurrences.end(),
			[this](const VALTAN_CLIP_OCCURRENCE_VIEW& Slot)
			{
				return Slot.strClipOccurrenceId == m_strSelectedStableId;
			});
		if (pStage->ClipOccurrences.end() != Pinned)
		{
			ImGui::SeparatorText("Selected Animation Occurrence Timing");
			ImGui::Text("%s", Pinned->strClipName.c_str());
			ImGui::TextDisabled(
				"Pinned source %u ms | play %u ms | rate %.3f | read only",
				Pinned->iSourceStartMs, Pinned->iPlayMs, Pinned->fPlayRate);
		}
		return;
	}

	CBalanceTool::PATTERN_STAGE_EDIT Draft;
	std::string Status;
	if (!m_pBalanceTool->Get_ValtanStageDraft(
			Pattern.strPatternId, pStage->strStageId, Draft, Status))
	{
		ImGui::TextDisabled("Selected Animation timing unavailable: %s",
			Status.c_str());
		return;
	}
	const auto Found = std::find_if(
		Draft.animationSlots.begin(), Draft.animationSlots.end(),
		[this](const CBalanceTool::ANIMATION_SLOT_EDIT& Slot)
		{
			return Slot.clipOccurrenceId == m_strSelectedStableId;
		});
	if (Draft.animationSlots.end() == Found)
		return;

	ImGui::SeparatorText("Selected Animation Occurrence Timing");
	ImGui::Text("%s", Found->clip.c_str());
	ImGui::SameLine();
	ImGui::TextDisabled("| %s", Found->clipOccurrenceId.c_str());
	ImGui::TextDisabled(
		"Source Start trims into the native clip. Play Duration controls this occurrence wall block without changing the Server Stage clock.");
	int32_t iSourceStartMs = static_cast<int32_t>(Found->sourceStartMs);
	int32_t iPlayMs = static_cast<int32_t>(Found->playMs);
	ImGui::PushID("##SelectedAnimationOccurrenceTiming");
	ImGui::BeginDisabled(!bMutationAdmitted || !Draft.animationEditable);
	(void)ImGui::InputInt(
		"Source Start (ms)", &iSourceStartMs, 5, 100);
	const bool_t bSourceChanged = ImGui::IsItemDeactivatedAfterEdit();
	ImGui::BeginDisabled(Found->repeatUntilStageEnd);
	(void)ImGui::InputInt(
		"Play Duration (ms)", &iPlayMs, 5, 100);
	const bool_t bPlayChanged = ImGui::IsItemDeactivatedAfterEdit();
	ImGui::EndDisabled();
	if (bSourceChanged || bPlayChanged)
	{
		iSourceStartMs = (std::clamp)(iSourceStartMs, 0, 600000);
		iPlayMs = (std::clamp)(iPlayMs, 0, 600000);
		if (Apply_AnimationOccurrenceTiming(
				Pattern, *pStage, Found->clipOccurrenceId,
				static_cast<uint32_t>(iSourceStartMs),
				static_cast<uint32_t>(iPlayMs), Status))
		{
			Invalidate_TimelineCache();
		}
		m_strStatus = std::move(Status);
	}
	ImGui::EndDisabled();
	ImGui::PopID();
	if (Found->repeatUntilStageEnd)
	{
		ImGui::TextDisabled(
			"Loop occurrence: playMs remains 0 and the Server Stage clock owns its visible end.");
	}
}

bool_t Client::CActionCompositionWorkbench::Seek_EffectivePreview(
	const VALTAN_PATTERN_VIEW& Pattern,
	const uint32_t iPositionMs,
	const bool_t bPause,
	std::string& status)
{
	if (nullptr == m_pAnimationTool)
	{
		status = "Animation preview owner is unavailable.";
		return false;
	}
	const CAnimation_Tool::COMPOSITION_PREVIEW_STATE Preview =
		m_pAnimationTool->Get_ValtanCompositionPreviewState();
	const std::uint64_t iDraftGeneration = nullptr == m_pBalanceTool ? 0u :
		m_pBalanceTool->Get_ValtanDraftGeneration();
	if (!Preview.bPlaying || Preview.strPatternId != Pattern.strPatternId ||
		m_iPreviewDraftGeneration != iDraftGeneration ||
		m_eStagedPreviewPath != m_ePreviewPath)
	{
		if (!Play_EffectivePreview(Pattern, status))
			return false;
	}
	const bool_t bSeeked =
		m_pAnimationTool->Seek_ValtanCompositionPattern(
			Pattern.strPatternId, iPositionMs, bPause, status);
	if (bSeeked)
		m_bPreviewOwnerClaimRequested = true;
	return bSeeked;
}

bool_t Client::CActionCompositionWorkbench::Play_EffectivePreview(
	const VALTAN_PATTERN_VIEW& Pattern,
	std::string& status)
{
	if (nullptr == m_pAnimationTool)
	{
		status = "Animation preview owner is unavailable.";
		return false;
	}
	if (!m_pAnimationTool->Play_ValtanCompositionDraftPattern(
			Pattern, m_ePreviewPath, status))
	{
		return false;
	}
	m_iPreviewDraftGeneration = nullptr == m_pBalanceTool ? 0u :
		m_pBalanceTool->Get_ValtanDraftGeneration();
	m_eStagedPreviewPath = m_ePreviewPath;
	m_bPreviewOwnerClaimRequested = true;
	return true;
}

uint32_t Client::CActionCompositionWorkbench::Resolve_ClipSourceToStageMs(
	const VALTAN_STAGE_VIEW& Stage,
	const std::string& strClipOccurrenceId,
	const uint32_t iSourceMs) const
{
	uint64_t iWallCursorMs = 0u;
	for (const VALTAN_CLIP_OCCURRENCE_VIEW& Clip : Stage.ClipOccurrences)
	{
		if (Clip.strClipOccurrenceId == strClipOccurrenceId)
		{
			const uint32_t iLocalSourceMs = iSourceMs > Clip.iSourceStartMs ?
				iSourceMs - Clip.iSourceStartMs : 0u;
			const double fRate = Clip.fPlayRate > 0.f ? Clip.fPlayRate : 1.0;
			const uint64_t iLocalWallMs = static_cast<uint64_t>(std::llround(
				static_cast<double>(iLocalSourceMs) / fRate));
			return static_cast<uint32_t>((std::min)(
				iWallCursorMs + iLocalWallMs,
				static_cast<uint64_t>(Stage.iDurationMs)));
		}
		iWallCursorMs += Clip.iAuthoringWallMs;
	}
	return 0u;
}

void Client::CActionCompositionWorkbench::Invalidate_TimelineCache()
{
	m_strTimelineCachePatternId.clear();
	m_eTimelineCachePreviewPath = VALTAN_PATTERN_PREVIEW_PATH::END;
	m_iTimelineCacheDraftGeneration = ~std::uint64_t{ 0u };
	m_iTimelineCacheSoundGeneration = ~std::uint64_t{ 0u };
	m_iTimelineCacheEffectV2Revision = ~std::uint64_t{ 0u };
}

void Client::CActionCompositionWorkbench::Invalidate_EffectivePatternCache()
{
	m_EffectivePatternCache = {};
	m_strEffectivePatternCachePatternId.clear();
	m_strEffectivePatternCacheCanonicalRevision.clear();
	m_iEffectivePatternCacheDraftGeneration = ~std::uint64_t{ 0u };
	m_bEffectivePatternCacheReady = false;
}

void Client::CActionCompositionWorkbench::Ensure_TimelineCache(
	const VALTAN_PATTERN_VIEW* const pPattern)
{
	if (nullptr == pPattern)
	{
		m_TimelineItems.clear();
		Invalidate_TimelineCache();
		m_iTimelineDurationMs = 0u;
		m_iPlayheadMs = 0u;
		return;
	}
	const std::uint64_t iDraftGeneration = nullptr == m_pBalanceTool ? 0u :
		m_pBalanceTool->Get_ValtanDraftGeneration();
	const std::uint64_t iSoundGeneration = nullptr == m_pAnimationTool ? 0u :
		m_pAnimationTool->Get_ValtanCompositionPatternSoundDraftGeneration();
	const std::uint64_t iEffectV2Revision =
		CEffectV2Catalog::Get().Get_Revision();
	if (m_strTimelineCachePatternId == pPattern->strPatternId &&
		m_eTimelineCachePreviewPath == m_ePreviewPath &&
		m_iTimelineCacheDraftGeneration == iDraftGeneration &&
		m_iTimelineCacheSoundGeneration == iSoundGeneration &&
		m_iTimelineCacheEffectV2Revision == iEffectV2Revision)
	{
		m_iPlayheadMs = (std::min)(m_iPlayheadMs, m_iTimelineDurationMs);
		return;
	}
	Build_Timeline(*pPattern);
	m_iPlayheadMs = (std::min)(m_iPlayheadMs, m_iTimelineDurationMs);
}

void Client::CActionCompositionWorkbench::Build_Timeline(
	const VALTAN_PATTERN_VIEW& Pattern)
{
	m_TimelineItems.clear();
	bool_t bPatternSoundDirty = false;
	std::string PatternSoundStatus;
	const VALTAN_PATTERN_SOUND_CUE_DOCUMENT* const pPatternSounds =
		ADMISSION_STATE::ADMITTED != m_eAdmission ||
		nullptr == m_pAnimationTool ? nullptr :
		m_pAnimationTool->Get_ValtanCompositionPatternSoundDraft(
			bPatternSoundDirty, PatternSoundStatus);
	const std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> pEffectV2Snapshot =
		CEffectV2Catalog::Get().Get_Snapshot();
	std::vector<const VALTAN_STAGE_VIEW*> PreviewStages;
	std::string PreviewPathStatus;
	if (!CValtanPatternTree::Build_PreviewStagePath(
			Pattern, m_ePreviewPath, PreviewStages, PreviewPathStatus))
	{
		if (VALTAN_PATTERN_PREVIEW_PATH::NORMAL == m_ePreviewPath ||
			!CValtanPatternTree::Build_PreviewStagePath(
				Pattern, VALTAN_PATTERN_PREVIEW_PATH::NORMAL,
				PreviewStages, PreviewPathStatus))
		{
			m_iTimelineDurationMs = 0u;
			m_strTimelineCachePatternId = Pattern.strPatternId;
			m_eTimelineCachePreviewPath = m_ePreviewPath;
			m_iTimelineCacheDraftGeneration = nullptr == m_pBalanceTool ? 0u :
				m_pBalanceTool->Get_ValtanDraftGeneration();
			m_iTimelineCacheSoundGeneration = nullptr == m_pAnimationTool ? 0u :
				m_pAnimationTool->Get_ValtanCompositionPatternSoundDraftGeneration();
			m_iTimelineCacheEffectV2Revision =
				CEffectV2Catalog::Get().Get_Revision();
			m_strStatus = "Sequencer branch rejected: " + PreviewPathStatus;
			return;
		}
		m_ePreviewPath = VALTAN_PATTERN_PREVIEW_PATH::NORMAL;
		m_iPreviewDraftGeneration = 0u;
		m_strStatus =
			"Selected branch is unavailable for this Pattern; Sequencer returned to Normal / Timeout.";
	}
	uint64_t iStageBaseMs = 0u;
	for (const VALTAN_STAGE_VIEW* const pStage : PreviewStages)
	{
		if (nullptr == pStage)
			continue;
		const VALTAN_STAGE_VIEW& Stage = *pStage;
		uint32_t iStageDurationMs = Stage.iDurationMs;
		CBalanceTool::PATTERN_STAGE_EDIT StageDraft;
		bool_t bHasStageDraft = false;
		if (ADMISSION_STATE::ADMITTED == m_eAdmission &&
			nullptr != m_pBalanceTool)
		{
			std::string DraftStatus;
			bHasStageDraft = m_pBalanceTool->Get_ValtanStageDraft(
				Pattern.strPatternId, Stage.strStageId, StageDraft, DraftStatus);
			if (bHasStageDraft)
			{
				iStageDurationMs = StageDraft.durationMs;
			}
		}
		const auto ResolveSourceToStageMs =
			[&Stage, &StageDraft, bHasStageDraft, iStageDurationMs, this](
				const std::string& strClipOccurrenceId,
				const uint32_t iSourceMs)
		{
			if (!bHasStageDraft)
				return Resolve_ClipSourceToStageMs(
					Stage, strClipOccurrenceId, iSourceMs);
			uint64_t iWallCursorMs = 0u;
			for (const CBalanceTool::ANIMATION_SLOT_EDIT& Slot :
				StageDraft.animationSlots)
			{
				if (Slot.clipOccurrenceId == strClipOccurrenceId)
				{
					const uint32_t iLocalSourceMs = iSourceMs > Slot.sourceStartMs ?
						iSourceMs - Slot.sourceStartMs : 0u;
					const uint64_t iLocalWallMs = static_cast<uint64_t>(
						std::llround(static_cast<double>(iLocalSourceMs) /
							Slot.playRate));
					return static_cast<uint32_t>((std::min)(
						iWallCursorMs + iLocalWallMs,
						static_cast<uint64_t>(iStageDurationMs)));
				}
				const uint32_t iSlotWallMs = 0u == Slot.playMs ?
					(iStageDurationMs > iWallCursorMs ?
						static_cast<uint32_t>(iStageDurationMs - iWallCursorMs) : 0u) :
					static_cast<uint32_t>(std::llround(
						static_cast<double>(Slot.playMs) / Slot.playRate));
				iWallCursorMs += iSlotWallMs;
			}
			return 0u;
		};
		const uint32_t iStageStartMs = static_cast<uint32_t>(
			SaturatingU32(iStageBaseMs));
		const uint32_t iStageEndMs = static_cast<uint32_t>(SaturatingU32(
			iStageBaseMs + static_cast<uint64_t>(iStageDurationMs)));
		m_TimelineItems.push_back({
			DETAIL_OWNER::GAMEPLAY_STAGE, TIMELINE_LANE::STAGE, Pattern.strPatternId,
			Stage.strStageId, Stage.strStageId, {},
			Stage.strStageId + " | " + Stage.strStageKind,
			iStageStartMs, iStageEndMs,
			bHasStageDraft && StageDraft.durationEditable });

		const std::string strEffectiveHitShape = bHasStageDraft ?
			StageDraft.hitShape : Stage.strHitShape;
		if (!strEffectiveHitShape.empty() && "NONE" != strEffectiveHitShape)
		{
			const uint32_t iHitDelayMs = bHasStageDraft ?
				StageDraft.hitDelayMs : Stage.iHitDelayMs;
			const uint32_t iHitIntervalMs = bHasStageDraft ?
				StageDraft.hitIntervalMs : Stage.iHitIntervalMs;
			const uint32_t iHitCount = bHasStageDraft ?
				StageDraft.hitCount : Stage.iHitCount;
			const std::vector<uint32_t>& HitOffsets = bHasStageDraft ?
				StageDraft.hitOffsetsMs : Stage.HitOffsetsMs;
			uint32_t iFirstHitMs = iHitDelayMs;
			uint32_t iLastHitMs = iHitDelayMs;
			if (!HitOffsets.empty())
			{
				iFirstHitMs = *std::min_element(
					HitOffsets.begin(), HitOffsets.end());
				iLastHitMs = *std::max_element(
					HitOffsets.begin(), HitOffsets.end());
			}
			else if (iHitCount > 1u)
			{
				iLastHitMs += iHitIntervalMs * (iHitCount - 1u);
			}
			m_TimelineItems.push_back({
				DETAIL_OWNER::GAMEPLAY_STAGE, TIMELINE_LANE::COLLIDER,
				Pattern.strPatternId, Stage.strStageId,
				Stage.strStageId + "/collider", {},
				strEffectiveHitShape + " | " +
					std::to_string(iHitCount) + " hit(s)",
				iStageStartMs + (std::min)(iFirstHitMs, iStageDurationMs),
				iStageStartMs + (std::min)(
					iLastHitMs + 1u, iStageDurationMs),
				bHasStageDraft && StageDraft.hitEditable });
		}

		for (const VALTAN_STAGE_BRANCH_VIEW& Branch : Stage.Branches)
		{
			const std::string strTarget = Branch.strNextActionId.has_value() ?
				*Branch.strNextActionId : "PATTERN END";
			const bool_t bCounterBranch = "COUNTER_HIT" == Branch.strOutcome;
			m_TimelineItems.push_back({
				DETAIL_OWNER::GAMEPLAY_STAGE, TIMELINE_LANE::LOGIC,
				Pattern.strPatternId, Stage.strStageId,
				Stage.strStageId + "/branch/" + Branch.strOutcome + "/" + strTarget,
				{}, Branch.strOutcome + " -> " + strTarget,
				iStageStartMs, iStageEndMs,
				bHasStageDraft && bCounterBranch &&
					"WINDUP" == StageDraft.stageKind });
		}
		const std::vector<VALTAN_STAGE_ACTION_VIEW>& EffectiveActions =
			bHasStageDraft ? StageDraft.actions : Stage.Actions;
		for (const VALTAN_STAGE_ACTION_VIEW& Action : EffectiveActions)
		{
			if ("RELEASE_GRABBED_PLAYERS" != Action.strKind)
				continue;
			m_TimelineItems.push_back({
				DETAIL_OWNER::GAMEPLAY_STAGE, TIMELINE_LANE::LOGIC,
				Pattern.strPatternId, Stage.strStageId,
				Stage.strStageId + "/action/" + Action.strKind + "/" +
					Action.strTargetId, {},
				"Grab Release | " + Action.strReleaseMode,
				iStageStartMs, iStageEndMs, bHasStageDraft });
		}
		if ((bHasStageDraft && StageDraft.portalRushMotionEditable) ||
			Stage.Motion.has_value())
		{
			const std::string strMotionKind = bHasStageDraft ?
				StageDraft.motionKind : Stage.Motion->strKind;
			m_TimelineItems.push_back({
				DETAIL_OWNER::GAMEPLAY_STAGE, TIMELINE_LANE::LOGIC,
				Pattern.strPatternId, Stage.strStageId,
				Stage.strStageId + "/motion/" + strMotionKind, {},
				"Motion | " + strMotionKind,
				iStageStartMs, iStageEndMs,
				bHasStageDraft && StageDraft.portalRushMotionEditable });
		}

		uint64_t iAnimationCursorMs = iStageBaseMs;
		if (bHasStageDraft)
		{
			for (const CBalanceTool::ANIMATION_SLOT_EDIT& Slot :
				StageDraft.animationSlots)
			{
				const uint32_t iStartMs = static_cast<uint32_t>(
					SaturatingU32(iAnimationCursorMs));
				const uint32_t iWallMs = 0u == Slot.playMs ?
					(iStageDurationMs > iAnimationCursorMs - iStageBaseMs ?
						static_cast<uint32_t>(iStageDurationMs -
							(iAnimationCursorMs - iStageBaseMs)) : 0u) :
					static_cast<uint32_t>(std::llround(
						static_cast<double>(Slot.playMs) / Slot.playRate));
				iAnimationCursorMs += iWallMs;
				m_TimelineItems.push_back({
					DETAIL_OWNER::ANIMATION, TIMELINE_LANE::ANIMATION, Pattern.strPatternId,
					Stage.strStageId, Slot.clipOccurrenceId, Slot.clip,
					Slot.clip, iStartMs,
					static_cast<uint32_t>(SaturatingU32(iAnimationCursorMs)),
					StageDraft.animationEditable && !Slot.repeatUntilStageEnd });
			}
		}
		else
		{
			for (const VALTAN_CLIP_OCCURRENCE_VIEW& Clip : Stage.ClipOccurrences)
			{
				const uint32_t iStartMs = static_cast<uint32_t>(
					SaturatingU32(iAnimationCursorMs));
				iAnimationCursorMs += Clip.iAuthoringWallMs;
				m_TimelineItems.push_back({
					DETAIL_OWNER::ANIMATION, TIMELINE_LANE::ANIMATION, Pattern.strPatternId,
					Stage.strStageId, Clip.strClipOccurrenceId, Clip.strClipName,
					Clip.strClipName, iStartMs,
					static_cast<uint32_t>(SaturatingU32(iAnimationCursorMs)), false });
			}
		}

		for (const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue : Stage.ProductCues)
		{
			const uint32_t iLocalStartMs = Cue.bUsesStageClock ?
				Cue.iStageOffsetMs : ResolveSourceToStageMs(
					Cue.strClipOccurrenceId, Cue.iSourceStartMs);
			uint32_t iLocalEndMs = iLocalStartMs;
			if (Cue.bHasSourceEnd)
				iLocalEndMs = ResolveSourceToStageMs(
					Cue.strClipOccurrenceId, Cue.iSourceEndMs);
			if ("each_loop" == Cue.strRepeatPolicy)
				iLocalEndMs = iStageDurationMs;
			m_TimelineItems.push_back({
				DETAIL_OWNER::EFFECT, TIMELINE_LANE::EFFECT, Pattern.strPatternId,
				Stage.strStageId, Cue.strOccurrenceId, Cue.strEffectAssetId,
				Cue.strEffectAssetId + " [" + Cue.strStopPolicy + "/" +
					Cue.strRepeatPolicy + "]",
				iStageStartMs + (std::min)(iLocalStartMs, iStageDurationMs),
				iStageStartMs + (std::min)(iLocalEndMs, iStageDurationMs),
				!Cue.bUsesStageClock });
		}
		for (const VALTAN_COMBAT_OBJECT_EFFECT_VIEW& Object :
			Stage.CombatObjectEffects)
		{
			m_TimelineItems.push_back({
				DETAIL_OWNER::COMBAT_OBJECT, TIMELINE_LANE::EFFECT, Pattern.strPatternId,
				Stage.strStageId, Object.strCombatObjectArchetypeId,
				Object.strEffectAssetId,
				"Combat Object | " + Object.strEffectAssetId,
				iStageStartMs,
				iStageStartMs + (std::min)(Object.iLifetimeMs, iStageDurationMs),
				false });
		}
		if (nullptr != pEffectV2Snapshot && pEffectV2Snapshot->Is_Ready())
		{
			std::size_t iV2Ordinal = 0u;
			for (const EFFECT_V2_BINDING& Binding :
				pEffectV2Snapshot->Get_BossValtanBindings())
			{
				if (Binding.strStage != Stage.strActionId)
					continue;
				const std::string& strResourceId = Binding.strGroupId.empty() ?
					Binding.strEffectId : Binding.strGroupId;
				const std::string strStableId =
					"effect-v2/" + Stage.strActionId + "/" +
					std::to_string(iV2Ordinal++) + "/" + strResourceId;
				m_TimelineItems.push_back({
					DETAIL_OWNER::EFFECT, TIMELINE_LANE::EFFECT,
					Pattern.strPatternId, Stage.strStageId, strStableId,
					strResourceId,
					std::string("V2 ") +
						(Binding.strGroupId.empty() ? "Leaf | " : "Group | ") +
						strResourceId,
					iStageStartMs + (std::min)(
						Binding.iStartMs, iStageDurationMs),
					iStageStartMs + (std::min)(
						Binding.iStartMs, iStageDurationMs), false });
			}
		}

		if (nullptr != pPatternSounds)
		{
			for (const VALTAN_PATTERN_SOUND_CUE& Cue : pPatternSounds->Cues)
			{
				if (Cue.strPatternId != Pattern.strPatternId ||
					Cue.strStageId != Stage.strStageId)
					continue;
				const uint32_t iLocalMs = ResolveSourceToStageMs(
					Cue.strClipOccurrenceId, Cue.iStartMs);
				m_TimelineItems.push_back({
					DETAIL_OWNER::SOUND, TIMELINE_LANE::SOUND, Pattern.strPatternId,
					Stage.strStageId, Cue.strOccurrenceId, Cue.strSoundEvent,
					Cue.strSoundEvent +
						(VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP ==
							Cue.eRepeatPolicy ? " [each_loop]" : " [once]"),
					iStageStartMs + iLocalMs, iStageStartMs + iLocalMs, true });
			}
		}
		for (const VALTAN_CAMERA_INVOCATION_VIEW& Camera :
			Stage.CameraInvocations)
		{
			m_TimelineItems.push_back({
				DETAIL_OWNER::CAMERA, TIMELINE_LANE::CAMERA, Pattern.strPatternId,
				Stage.strStageId, Camera.strCameraInvocationId,
				Camera.strCameraCueId, Camera.strCameraCueId,
				iStageStartMs + Camera.iStartOffsetMs,
				iStageStartMs + Camera.iStartOffsetMs + Camera.iDurationMs,
				false });
		}
		if (m_bPatternShakesReady)
		{
			for (const VALTAN_PATTERN_SHAKE_CUE& Cue : m_PatternShakes.Cues)
			{
				if (Cue.strPatternId != Pattern.strPatternId ||
					Cue.strStageId != Stage.strStageId)
					continue;
				const uint32_t iLocalMs = ResolveSourceToStageMs(
					Cue.strClipOccurrenceId, Cue.iStartMs);
				const int64_t iSignedDurationMs = std::llround(
					static_cast<double>(Cue.Spec.fDurationSeconds) * 1000.0);
				const uint32_t iDurationMs = static_cast<uint32_t>((std::max)(
					int64_t{ 0 }, iSignedDurationMs));
				m_TimelineItems.push_back({
					DETAIL_OWNER::CAMERA, TIMELINE_LANE::CAMERA, Pattern.strPatternId,
					Stage.strStageId, Cue.strOccurrenceId, "camera shake",
					"Camera Shake", iStageStartMs + iLocalMs,
					iStageStartMs + (std::min)(
						iLocalMs + iDurationMs, iStageDurationMs), false });
			}
		}
		for (const VALTAN_WORLD_EVENT_TRIGGER_REF_VIEW& Event :
			Pattern.WorldEventTriggerRefs)
		{
			if (Event.strStageId != Stage.strStageId)
				continue;
			const bool_t bExit = std::string::npos !=
				Event.strTriggerKind.find("EXIT") || std::string::npos !=
				Event.strTriggerKind.find("END");
			const uint32_t iEventMs = bExit ? iStageEndMs : iStageStartMs;
			m_TimelineItems.push_back({
				DETAIL_OWNER::WORLD, TIMELINE_LANE::LOGIC, Pattern.strPatternId,
				Stage.strStageId, Event.strTriggerKind, {},
				Event.strTriggerKind, iEventMs, iEventMs, false });
		}
		iStageBaseMs += iStageDurationMs;
	}
	m_iTimelineDurationMs = static_cast<uint32_t>(SaturatingU32(iStageBaseMs));
	Pack_TimelineSubrows();
	m_strTimelineCachePatternId = Pattern.strPatternId;
	m_eTimelineCachePreviewPath = m_ePreviewPath;
	m_iTimelineCacheDraftGeneration = nullptr == m_pBalanceTool ? 0u :
		m_pBalanceTool->Get_ValtanDraftGeneration();
	m_iTimelineCacheSoundGeneration = nullptr == m_pAnimationTool ? 0u :
		m_pAnimationTool->Get_ValtanCompositionPatternSoundDraftGeneration();
	m_iTimelineCacheEffectV2Revision = CEffectV2Catalog::Get().Get_Revision();
}

void Client::CActionCompositionWorkbench::Pack_TimelineSubrows()
{
	m_TimelineLaneSubrowCounts.fill(1u);
	/* A point event renders at least four pixels wide. At the minimum supported
	   40 px/sec zoom that is 100 ms, so packing against this conservative window
	   prevents click-through overlap at every zoom without warm-frame sorting. */
	constexpr uint64_t MINIMUM_VISUAL_DURATION_MS = 100u;
	for (std::size_t iLane = 0u; iLane < m_TimelineLaneSubrowCounts.size(); ++iLane)
	{
		const TIMELINE_LANE eLane = static_cast<TIMELINE_LANE>(iLane);
		std::vector<std::size_t> LaneItems;
		for (std::size_t iItem = 0u; iItem < m_TimelineItems.size(); ++iItem)
		{
			if (m_TimelineItems[iItem].eLane == eLane)
				LaneItems.push_back(iItem);
		}
		std::stable_sort(
			LaneItems.begin(), LaneItems.end(),
			[this](const std::size_t iLeft, const std::size_t iRight)
			{
				const TIMELINE_ITEM& Left = m_TimelineItems[iLeft];
				const TIMELINE_ITEM& Right = m_TimelineItems[iRight];
				if (Left.iStartMs != Right.iStartMs)
					return Left.iStartMs < Right.iStartMs;
				if (Left.iEndMs != Right.iEndMs)
					return Left.iEndMs < Right.iEndMs;
				return Left.strStableId < Right.strStableId;
			});
		std::vector<uint64_t> SubrowVisualEndMs;
		for (const std::size_t iItem : LaneItems)
		{
			TIMELINE_ITEM& Item = m_TimelineItems[iItem];
			std::size_t iSubrow = 0u;
			for (; iSubrow < SubrowVisualEndMs.size(); ++iSubrow)
			{
				if (static_cast<uint64_t>(Item.iStartMs) >=
					SubrowVisualEndMs[iSubrow])
				{
					break;
				}
			}
			const uint64_t iVisualEndMs = (std::max)(
				static_cast<uint64_t>(Item.iEndMs),
				static_cast<uint64_t>(Item.iStartMs) +
					MINIMUM_VISUAL_DURATION_MS);
			if (iSubrow == SubrowVisualEndMs.size())
				SubrowVisualEndMs.push_back(iVisualEndMs);
			else
				SubrowVisualEndMs[iSubrow] = iVisualEndMs;
			Item.iSubrow = iSubrow;
		}
		m_TimelineLaneSubrowCounts[iLane] = (std::max)(
			std::size_t{ 1u }, SubrowVisualEndMs.size());
	}
}

bool_t Client::CActionCompositionWorkbench::Is_PatternSoundDraftDirty(
	std::string& strOutStatus) const
{
	if (nullptr == m_pAnimationTool)
	{
		strOutStatus = "Pattern Sound owner is unavailable.";
		return false;
	}
	bool_t bDirty = false;
	(void)m_pAnimationTool->Get_ValtanCompositionPatternSoundDraft(
		bDirty, strOutStatus);
	return bDirty;
}

bool_t Client::CActionCompositionWorkbench::
Validate_ManualStageTopologySoundDependencies(
	const VALTAN_PATTERN_VIEW& CandidatePattern,
	std::string& strOutStatus) const
{
	if (nullptr == m_pAnimationTool || nullptr == m_pBalanceTool)
	{
		strOutStatus =
			"Manual Stage topology requires both the Pattern owner and Pattern Sound dependency owner.";
		return false;
	}
	std::vector<VALTAN_PATTERN_VIEW> BaselinePatterns;
	std::vector<VALTAN_PATTERN_VIEW> CandidatePatterns;
	const std::vector<const VALTAN_PATTERN_VIEW*> Patterns =
		Collect_CanonicalPatternsForDependencyValidation();
	BaselinePatterns.reserve(Patterns.size());
	CandidatePatterns.reserve(Patterns.size());
	bool_t bCandidateReplaced = false;
	for (const VALTAN_PATTERN_VIEW* const pPattern : Patterns)
	{
		BaselinePatterns.push_back(*pPattern);
		if (pPattern->strPatternId == CandidatePattern.strPatternId)
		{
			CandidatePatterns.push_back(CandidatePattern);
			bCandidateReplaced = true;
			continue;
		}
		VALTAN_PATTERN_VIEW Candidate = *pPattern;
		if (pPattern->bAuthoringMasterManaged &&
			!m_pBalanceTool->Get_ValtanPatternDraft(
				pPattern->strPatternId, Candidate, strOutStatus))
		{
			strOutStatus =
				"Manual Stage topology could not assemble the effective Pattern graph: " +
				strOutStatus;
			return false;
		}
		CandidatePatterns.push_back(std::move(Candidate));
	}
	if (!bCandidateReplaced)
	{
		strOutStatus =
			"Manual Stage topology candidate is not part of the admitted canonical Pattern graph.";
		return false;
	}
	if (!m_pAnimationTool->
			Validate_ValtanCompositionPatternSoundGraphDependencies(
				BaselinePatterns, CandidatePatterns, strOutStatus))
	{
		strOutStatus =
			"Manual Stage topology rejected before the gameplay draft changed: " +
			strOutStatus;
		return false;
	}
	return true;
}

bool_t Client::CActionCompositionWorkbench::Save_Publish_Reload()
{
	if (ADMISSION_STATE::ADMITTED != m_eAdmission || nullptr == m_pBalanceTool ||
		nullptr == m_pBossTool)
	{
		m_strStatus =
			"Save blocked: canonical admission and both typed gameplay/Server owners are required.";
		return false;
	}
	std::string CurrentAuthoringRevision;
	std::string CurrentCanonicalSourceRevision;
	std::string CurrentRevisionStatus;
	bool_t bCurrentAuthoringDirty = false;
	if (!m_pBalanceTool->Get_ValtanAuthoringState(
			CurrentAuthoringRevision, bCurrentAuthoringDirty,
			CurrentRevisionStatus) ||
		!m_pBalanceTool->Get_ValtanCanonicalSourceRevision(
			CurrentCanonicalSourceRevision, CurrentRevisionStatus) ||
		CurrentAuthoringRevision != m_strPinnedAuthoringSourceRevision ||
		CurrentCanonicalSourceRevision != m_strPinnedCanonicalSourceRevision)
	{
		m_eAdmission = ADMISSION_STATE::STALE_PRESERVED;
		m_strStatus =
			"Save & Apply blocked: the draft and Workbench view no longer share the same data generation. Reload Canonical before editing or saving. " +
			CurrentRevisionStatus;
		return false;
	}
	std::string SoundStatus;
	if (Is_PatternSoundDraftDirty(SoundStatus))
	{
		m_strStatus =
			"Save & Apply blocked: save or discard the Pattern Sound draft before changing its Pattern/Animation data generation. " +
			SoundStatus;
		return false;
	}
	if (nullptr == m_pAnimationTool)
	{
		m_strStatus =
			"Save & Apply blocked: Pattern Sound data is unavailable.";
		return false;
	}
	std::vector<VALTAN_PATTERN_VIEW> BaselinePatterns;
	std::vector<VALTAN_PATTERN_VIEW> CandidatePatterns;
	const std::vector<const VALTAN_PATTERN_VIEW*> Patterns =
		Collect_CanonicalPatternsForDependencyValidation();
	BaselinePatterns.reserve(Patterns.size());
	CandidatePatterns.reserve(Patterns.size());
	for (const VALTAN_PATTERN_VIEW* const pPattern : Patterns)
	{
		BaselinePatterns.push_back(*pPattern);
		VALTAN_PATTERN_VIEW Candidate = *pPattern;
		if (pPattern->bAuthoringMasterManaged &&
			!m_pBalanceTool->Get_ValtanPatternDraft(
				pPattern->strPatternId, Candidate, SoundStatus))
		{
			m_strStatus =
				"Save & Apply blocked: the complete Pattern graph could not be assembled for Pattern Sound validation: " +
				SoundStatus;
			return false;
		}
		CandidatePatterns.push_back(std::move(Candidate));
	}
	if (!m_pAnimationTool->
			Validate_ValtanCompositionAnimationGraphMutations(
				BaselinePatterns, CandidatePatterns, SoundStatus))
	{
		m_strStatus =
			"Save & Apply blocked before data files changed by Animation validation: " +
			SoundStatus;
		return false;
	}
	if (!m_pAnimationTool->
			Validate_ValtanCompositionPatternSoundGraphDependencies(
				BaselinePatterns, CandidatePatterns, SoundStatus))
	{
		m_strStatus =
			"Save & Apply blocked before data files changed: " +
			SoundStatus;
		return false;
	}
	std::string SaveStatus;
	if (!m_pBalanceTool->Save_ValtanCanonicalProduct(SaveStatus))
	{
		m_strStatus = "Save & Apply could not write the validated data files; the previous active graph remains unchanged: " +
			SaveStatus;
		return false;
	}
	std::string RuntimeStatus;
	if (!m_pBalanceTool->Save_ValtanProduct(RuntimeStatus))
	{
		std::string LocalReloadStatus;
		(void)m_pBossTool->Reload_CanonicalGraph(LocalReloadStatus);
		(void)Reload_Canonical();
		m_strStatus =
			"Validated data files were saved, but Server apply preparation failed. Local tools reloaded the saved data; active Client presentation and Server playback remain unchanged: " +
			RuntimeStatus + " " + LocalReloadStatus;
		return false;
	}
	std::string ToolReloadStatus;
	if (!m_pBossTool->Reload_CanonicalGraph(ToolReloadStatus))
	{
		(void)Reload_Canonical();
		m_strStatus =
			"Data files were saved and Server apply was prepared, but Boss Tool could not reload them. Active Client presentation was not advanced: " +
			ToolReloadStatus;
		return false;
	}
	if (!Reload_Canonical())
	{
		m_strStatus = "Data files were saved and Server apply was prepared, but Workbench could not reload them: " +
			m_strStatus;
		return false;
	}
	m_strStatus =
		"SAVE & APPLY: data files saved, validation passed, and local tools reloaded. "
		"Server apply status follows; Restart and Complete Play remain blocked until the exact saved revision is active, and presentation changes can require re-entry. " +
		SaveStatus + " " + ToolReloadStatus + " " + RuntimeStatus;
	return true;
}

bool_t Client::CActionCompositionWorkbench::Render_Toolbar(
	const VALTAN_PATTERN_VIEW* const pPattern,
	const bool_t bMutationAdmitted)
{
	ImGui::SeparatorText("Pattern Save");
	const bool_t bSavedServerPending = m_bPatternSaveResultAvailable &&
		!m_bPatternSaveSucceeded &&
		(std::string::npos != m_strPatternSaveStatus.find(
			"Validated data files were saved") ||
		std::string::npos != m_strPatternSaveStatus.find(
			"Data files were saved"));
	const bool_t bLastSaveFailed = m_bAuthoringDraftDirty &&
		m_bPatternSaveResultAvailable && !m_bPatternSaveSucceeded &&
		!bSavedServerPending;
	const char_t* const pSaveSummary = bSavedServerPending ?
		"SAVED / SERVER PENDING" : (!m_bAuthoringDraftDirty ?
			"SAVED" : (bLastSaveFailed ? "SAVE FAILED" : "UNSAVED"));
	const ImVec4 SaveSummaryColor =
		!m_bAuthoringDraftDirty && !bSavedServerPending ?
		ImVec4(0.35f, 0.86f, 0.45f, 1.f) :
		(bLastSaveFailed ? ImVec4(1.f, 0.38f, 0.30f, 1.f) :
			ImVec4(1.f, 0.76f, 0.25f, 1.f));
	ImGui::TextColored(SaveSummaryColor, "Pattern: %s", pSaveSummary);
	if (bLastSaveFailed)
		ImGui::TextWrapped(
			"Save failed before writing files. Open Advanced Diagnostics.");
	else if (bSavedServerPending)
		ImGui::TextWrapped(
			"Files are saved. Restart Server, then re-enter the arena.");

	bool_t bCanonicalViewMayHaveChanged = false;
	if (ImGui::BeginTable(
			"##CompositionPrimaryActions", 2,
			ImGuiTableFlags_SizingStretchSame))
	{
		ImGui::TableNextColumn();
		ImGui::BeginDisabled(!bMutationAdmitted || nullptr == m_pBalanceTool);
		if (ImGui::Button(
				"Save & Apply##CompositionSession", ImVec2(-1.f, 0.f)))
		{
			m_bPatternSaveSucceeded = Save_Publish_Reload();
			m_bPatternSaveResultAvailable = true;
			m_strPatternSaveStatus = m_strStatus;
			bCanonicalViewMayHaveChanged = true;
		}
		ImGui::EndDisabled();
		ImGui::TableNextColumn();
		if (ImGui::Button("Reload Canonical", ImVec2(-1.f, 0.f)))
		{
			(void)Reload_Canonical();
			bCanonicalViewMayHaveChanged = true;
		}
		ImGui::EndTable();
	}
	if (bCanonicalViewMayHaveChanged)
		return true;

	std::string ServerPlayStatus;
	std::string SoundRuntimeStatus;
	LostArk::Shared::GameplayDataRevision ExpectedServerRevision{};
	/* Widget availability must stay memory-only. The exact physical Product
	   closure is hashed once on the command edge by the Boss owner. */
	const bool_t bServerRevisionAdmitted = nullptr != m_pBossTool &&
		m_pBossTool->Observe_ServerActivePatternRevision(
			ExpectedServerRevision, ServerPlayStatus);
	const bool_t bSoundRuntimeReady = nullptr != m_pAnimationTool &&
		bServerRevisionAdmitted &&
		m_pAnimationTool->Is_ValtanCompositionPatternSoundRuntimeReady(
			ExpectedServerRevision, SoundRuntimeStatus);
	const bool_t bServerPlayAdmitted =
		bServerRevisionAdmitted && bSoundRuntimeReady;
	ImGui::SeparatorText("Server Playback");
	ImGui::TextColored(
		bServerPlayAdmitted ? ImVec4(0.35f, 0.86f, 0.45f, 1.f) :
			ImVec4(1.f, 0.76f, 0.25f, 1.f),
		"Server: %s", bServerPlayAdmitted ? "READY" : "NOT READY");
	if (!bServerPlayAdmitted)
	{
		ImGui::TextWrapped(nullptr == pPattern ?
			"Select a Pattern to enable Server playback." :
			"Server not ready. Save and restart Server, then re-enter the arena.");
	}
	if (ImGui::BeginTable(
			"##CompositionServerActions", 3,
			ImGuiTableFlags_SizingStretchSame))
	{
		ImGui::TableNextColumn();
		ImGui::BeginDisabled(nullptr == pPattern || !bMutationAdmitted ||
			!bServerPlayAdmitted);
		if (ImGui::Button("Play on Server", ImVec2(-1.f, 0.f)))
		{
			std::string Status;
#ifdef _DEBUG
			if (CMainApp* const pApp = CMainApp::Get_Active();
				nullptr != pApp &&
				pApp->Debug_SelectCompletePlayPattern(pPattern->strPatternId))
			{
				(void)pApp->Debug_CompletePlaySelected(Status);
			}
			else
			{
				Status = "Complete Play selection was rejected by the admitted inventory.";
			}
#else
			Status = "Complete Play authoring control is unavailable outside a Debug Client.";
#endif
			m_strStatus = std::move(Status);
		}
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) &&
			!bServerPlayAdmitted)
		{
			ImGui::SetTooltip(
				"Server playback is not ready. Open Advanced Diagnostics for the exact reason.");
		}
		ImGui::TableNextColumn();
		if (ImGui::Button("Restart", ImVec2(-1.f, 0.f)))
		{
			std::string Status;
			if (nullptr == m_pBossTool ||
				!m_pBossTool->Restart_ServerPattern(
					pPattern->strPatternId, Status))
			{
				m_strStatus = Status;
			}
			else
			{
				m_strStatus = std::move(Status);
			}
		}
		ImGui::TableNextColumn();
		if (ImGui::Button("Queue Next", ImVec2(-1.f, 0.f)))
		{
			std::string Status;
			if (nullptr == m_pBossTool ||
				!m_pBossTool->Queue_NextServerPattern(
					pPattern->strPatternId, Status))
			{
				m_strStatus = std::move(Status);
			}
			else
			{
				m_strStatus = std::move(Status);
			}
		}
		ImGui::EndDisabled();
		ImGui::EndTable();
	}

	ImGui::SeparatorText("Tools");
	if (ImGui::BeginTable(
			"##CompositionToolActions", 3,
			ImGuiTableFlags_SizingStretchSame))
	{
		ImGui::TableNextColumn();
		ImGui::BeginDisabled(!bMutationAdmitted || nullptr == m_pBossTool);
		if (ImGui::Button("Restore Arena", ImVec2(-1.f, 0.f)))
		{
			std::string Status;
			(void)m_pBossTool->Set_ServerArenaPreset(
				LostArk::Shared::VALTAN_ARENA_PRESET::FRESH, Status);
			m_strStatus = std::move(Status);
		}
		ImGui::EndDisabled();
		ImGui::TableNextColumn();
		if (ImGui::Button("Animation Clips", ImVec2(-1.f, 0.f)))
			m_bOpenAnimationToolRequested = true;
		ImGui::TableNextColumn();
		if (ImGui::Button("Pattern Flow", ImVec2(-1.f, 0.f)))
		{
			std::string Status;
#ifdef _DEBUG
			CMainApp* const pApp = CMainApp::Get_Active();
			if (nullptr == pApp || !pApp->Debug_OpenBossPatternFlow(Status))
				m_strStatus = std::move(Status);
			else
				m_strStatus = std::move(Status);
#else
			Status = "Pattern Flow authoring is unavailable outside a Debug Client.";
			m_strStatus = std::move(Status);
#endif
		}
		ImGui::EndTable();
	}
	return false;
}

void Client::CActionCompositionWorkbench::Render_Browser(
	const VALTAN_PATTERN_VIEW* const pEffectiveSelectedPattern)
{
	ImGui::SeparatorText("Pattern / Stage Browser");
	ImGui::BeginDisabled(nullptr == pEffectiveSelectedPattern);
	if (ImGui::Button("Open Boss Pattern"))
	{
		m_bBossPatternWindowVisible = true;
		m_bBossPatternFocusRequested = true;
		m_bBossPatternFitRequested = true;
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::TextDisabled("Logic branches + Stage structure");
	ImGui::SetNextItemWidth(-1.f);
	ImGui::InputTextWithHint(
		"##CompositionPatternSearch", "Search Pattern ID or display name...",
		m_PatternSearch.data(), m_PatternSearch.size());
	const std::string Query = m_PatternSearch.data();
	const std::vector<const VALTAN_PATTERN_VIEW*> Patterns = Collect_Patterns();
	ImGui::TextDisabled("%zu canonical patterns", Patterns.size());
	for (const VALTAN_PATTERN_VIEW* const pPattern : Patterns)
	{
		if (nullptr == pPattern ||
			(!ContainsInsensitive(pPattern->strPatternId, Query) &&
			 !ContainsInsensitive(pPattern->strDisplayName, Query)))
			continue;
		ImGui::PushID(pPattern->strPatternId.c_str());
		const bool_t bSelected =
			pPattern->strPatternId == m_strSelectedPatternId;
		const VALTAN_PATTERN_VIEW* const pDisplayPattern =
			bSelected && nullptr != pEffectiveSelectedPattern &&
			pEffectiveSelectedPattern->strPatternId == pPattern->strPatternId ?
			pEffectiveSelectedPattern : pPattern;
		const std::string Label = pDisplayPattern->strDisplayName + "##pattern";
		const ImGuiTreeNodeFlags Flags = ImGuiTreeNodeFlags_OpenOnArrow |
			ImGuiTreeNodeFlags_SpanAvailWidth |
			(bSelected ? ImGuiTreeNodeFlags_Selected : 0);
		const bool_t bOpen = ImGui::TreeNodeEx(Label.c_str(), Flags);
		if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
			Select_Pattern(*pDisplayPattern);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("%s", pDisplayPattern->strPatternId.c_str());
		if (bOpen)
		{
			ImGui::TextDisabled(
				"%s | phase %u..%u | %zu stages",
				pDisplayPattern->strPatternId.c_str(),
				pDisplayPattern->iMinimumPhase,
				pDisplayPattern->iMaximumPhase,
				pDisplayPattern->Stages.size());
			for (const VALTAN_STAGE_VIEW& Stage : pDisplayPattern->Stages)
			{
				ImGui::PushID(Stage.strStageId.c_str());
				const bool_t bStageSelected = bSelected &&
					Stage.strStageId == m_strSelectedStageId;
				const std::string StageLabel = Stage.strStageId + " | " +
					Stage.strStageKind + " | " +
					std::to_string(Stage.iDurationMs) + " ms";
				if (ImGui::Selectable(
						StageLabel.c_str(), bStageSelected,
						0))
				{
					Select_Stage(*pDisplayPattern, Stage);
				}
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
}

void Client::CActionCompositionWorkbench::Render_SequenceBrowser(
	const VALTAN_PATTERN_VIEW* const pPattern,
	const VALTAN_STAGE_VIEW* const pStage,
	const bool_t bMutationAdmitted)
{
	if (!m_bAnimationSequenceLoadAttempted)
		(void)Reload_AnimationSequences();
	if (ImGui::Button("Reload Animation Sequences"))
		(void)Reload_AnimationSequences();
	ImGui::TextWrapped("%s", m_strAnimationSequenceStatus.c_str());
	ImGui::SetNextItemWidth(-1.f);
	if (ImGui::InputTextWithHint(
		"##CompositionSequenceSearch", "Search action, mode or clip...",
		m_ResourceSearch.data(), m_ResourceSearch.size()))
	{
		m_bAnimationSequenceFilterDirty = true;
	}
	const std::string Query = m_ResourceSearch.data();
	if (m_bAnimationSequenceFilterDirty ||
		m_strAnimationSequenceFilterQuery != Query)
	{
		m_FilteredAnimationSequenceIndices.clear();
		m_FilteredAnimationSequenceIndices.reserve(m_AnimationSequences.size());
		for (std::size_t iSequence = 0u;
			iSequence < m_AnimationSequences.size(); ++iSequence)
		{
			const CAnimation_Tool::COMPOSITION_SEQUENCE_VIEW& Sequence =
				m_AnimationSequences[iSequence];
			const bool_t bMatches =
				ContainsInsensitive(Sequence.strDisplayName, Query) ||
				ContainsInsensitive(Sequence.strMode, Query) ||
				std::any_of(
					Sequence.Clips.begin(), Sequence.Clips.end(),
					[&Query](
						const CAnimation_Tool::COMPOSITION_SEQUENCE_CLIP_VIEW& Clip)
					{
						return ContainsInsensitive(Clip.strClipName, Query);
					});
			if (bMatches)
				m_FilteredAnimationSequenceIndices.push_back(iSequence);
		}
		m_AnimationResourceTree = {};
		for (const std::size_t iSequence :
			m_FilteredAnimationSequenceIndices)
		{
			const CAnimation_Tool::COMPOSITION_SEQUENCE_VIEW& Sequence =
				m_AnimationSequences[iSequence];
			InsertResourceTree(
				m_AnimationResourceTree,
				{ Sequence.strMode.empty() ? "UNCLASSIFIED" : Sequence.strMode },
				iSequence);
		}
		(void)FinalizeResourceTree(m_AnimationResourceTree);
		m_strAnimationSequenceFilterQuery = Query;
		m_bAnimationSequenceFilterDirty = false;
	}
	ImGui::TextDisabled(
		"%zu matching source Sequences", m_FilteredAnimationSequenceIndices.size());

	const auto Selected = std::find_if(
		m_AnimationSequences.begin(), m_AnimationSequences.end(),
		[this](const CAnimation_Tool::COMPOSITION_SEQUENCE_VIEW& Sequence)
		{
			return Sequence.iSkillId == m_iSelectedSequenceSkillId &&
				Sequence.iSequenceIndex == m_iSelectedSequenceIndex;
		});
	const CAnimation_Tool::COMPOSITION_SEQUENCE_VIEW* pSelected =
		m_AnimationSequences.end() == Selected ? nullptr : &*Selected;
	if (nullptr != pSelected)
	{
		ImGui::SeparatorText("Selected Sequence");
		ImGui::TextWrapped("%s", pSelected->strDisplayName.c_str());
		ImGui::TextDisabled("Action %d | Sequence %d | %s",
			pSelected->iSkillId, pSelected->iSequenceIndex,
			pSelected->strMode.c_str());
		for (std::size_t iClip = 0u; iClip < pSelected->Clips.size(); ++iClip)
		{
			const CAnimation_Tool::COMPOSITION_SEQUENCE_CLIP_VIEW& Clip =
				pSelected->Clips[iClip];
			ImGui::BulletText("%02zu  %s  | %u ms%s", iClip + 1u,
				Clip.strClipName.c_str(), Clip.iDurationMs,
				Clip.bUsesNativeDuration ? " native" : " cut");
		}
		const CAnimation_Tool::COMPOSITION_PREVIEW_STATE SourcePreview =
			nullptr == m_pAnimationTool ?
				CAnimation_Tool::COMPOSITION_PREVIEW_STATE{} :
				m_pAnimationTool->Get_ValtanCompositionPreviewState();
		ImGui::TextColored(
			SourcePreview.bModelReady ?
				ImVec4(0.35f, 0.86f, 0.45f, 1.f) :
				ImVec4(1.f, 0.72f, 0.24f, 1.f),
			"Preview target: ARENA CLONE %s | Source: %s | Server Valtan: UNCHANGED",
			SourcePreview.bModelReady ? "READY" : "WILL STAGE ON CLICK",
			SourcePreview.bSourceSequencePlaying ? "PLAYING" : "IDLE");
		if (!SourcePreview.strSourceSequenceStatus.empty())
			ImGui::TextWrapped("%s",
				SourcePreview.strSourceSequenceStatus.c_str());
		ImGui::TextDisabled(
			"Raw Sequence preview needs only its admitted source catalog and Valtan CModel; it does not require a canonical Pattern revision.");
		ImGui::BeginDisabled(nullptr == m_pAnimationTool);
		if (ImGui::Button("Preview Sequence on Arena Clone"))
		{
			std::string Status;
			/* A failed Arena/model/source resolve must not steal viewport input
			   from another tool.  Claim Composition ownership only after the local
			   clone transaction has actually started. */
			if (m_pAnimationTool->Preview_ValtanCompositionSequence(
					pSelected->iSkillId, pSelected->iSequenceIndex, Status))
			{
				m_bPreviewOwnerClaimRequested = true;
			}
			m_strStatus = std::move(Status);
		}
		ImGui::EndDisabled();
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		{
			ImGui::SetTooltip(
				"Spawns/uses the collision-free local Valtan Model View in the current Valtan Arena. This does not mutate the Server boss.");
		}
		ImGui::BeginDisabled(nullptr == pPattern || nullptr == pStage ||
			!bMutationAdmitted || nullptr == m_pBalanceTool);
		if (ImGui::Button("Replace Stage Slots"))
			(void)Apply_SelectedSequenceToStage(*pPattern, *pStage, false);
		ImGui::SameLine();
		if (ImGui::Button("Append to Stage Slots"))
			(void)Apply_SelectedSequenceToStage(*pPattern, *pStage, true);
		ImGui::EndDisabled();
		ImGui::BeginDisabled(nullptr == m_pAnimationTool || !bMutationAdmitted);
		if (ImGui::Button("Use for Create New Pattern"))
		{
			std::string Status;
			if (m_pAnimationTool->Stage_ValtanCompositionIntakeSequence(
				pSelected->iSkillId, pSelected->iSequenceIndex,
				nullptr == pPattern ? std::string{} : pPattern->strPatternId,
				nullptr == pStage ? std::string{} : pStage->strStageId,
				Status))
			{
				m_bPatternsWindowVisible = true;
				m_iRequestedPatternTab = 1;
			}
			m_strStatus = std::move(Status);
		}
		ImGui::EndDisabled();

		const SOURCE_SEQUENCE_OWNER_INDEX_ENTRY* pSourceOwnerEntry = nullptr;
		if (pSelected->iSkillId >= 0 && pSelected->iSequenceIndex >= 0)
		{
			Ensure_SourceSequenceOwnerIndex();
			pSourceOwnerEntry = Find_SourceSequenceOwners(
				static_cast<uint32_t>(pSelected->iSkillId),
				static_cast<uint32_t>(pSelected->iSequenceIndex));
		}
		static const std::vector<const VALTAN_PATTERN_VIEW*> NoSourceOwners;
		const std::vector<const VALTAN_PATTERN_VIEW*>& SourceOwners =
			nullptr == pSourceOwnerEntry ? NoSourceOwners :
				pSourceOwnerEntry->Owners;
		const auto SelectedSourceOwner = std::find_if(
			SourceOwners.begin(), SourceOwners.end(),
			[this](const VALTAN_PATTERN_VIEW* const pCandidate)
			{
				return nullptr != pCandidate &&
					pCandidate->strPatternId ==
						m_strSourceSequenceServerPatternId;
			});
		if (!SourceOwners.empty() && SelectedSourceOwner == SourceOwners.end())
		{
			const auto CurrentPatternOwner = std::find_if(
				SourceOwners.begin(), SourceOwners.end(),
				[pPattern](const VALTAN_PATTERN_VIEW* const pCandidate)
				{
					return nullptr != pPattern && nullptr != pCandidate &&
						pCandidate->strPatternId == pPattern->strPatternId;
				});
			m_strSourceSequenceServerPatternId =
				(CurrentPatternOwner == SourceOwners.end() ?
					SourceOwners.front() : *CurrentPatternOwner)->strPatternId;
		}
		const VALTAN_PATTERN_VIEW* pServerPattern = nullptr;
		for (const VALTAN_PATTERN_VIEW* const pCandidate : SourceOwners)
		{
			if (nullptr != pCandidate && pCandidate->strPatternId ==
				m_strSourceSequenceServerPatternId)
			{
				pServerPattern = pCandidate;
				break;
			}
		}
		if (SourceOwners.size() > 1u)
		{
			const char_t* const pPreview = nullptr == pServerPattern ?
				"Select owning Pattern" : pServerPattern->strDisplayName.c_str();
			ImGui::SetNextItemWidth(-1.f);
			if (ImGui::BeginCombo("Server Pattern##SourceSequenceOwner", pPreview))
			{
				for (const VALTAN_PATTERN_VIEW* const pCandidate : SourceOwners)
				{
					if (nullptr == pCandidate)
						continue;
					const bool_t bSelectedOwner = pCandidate == pServerPattern;
					const std::string Label = pCandidate->strDisplayName + " | " +
						pCandidate->strPatternId;
					if (ImGui::Selectable(Label.c_str(), bSelectedOwner))
						m_strSourceSequenceServerPatternId =
							pCandidate->strPatternId;
					if (bSelectedOwner)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}
		else if (nullptr != pServerPattern)
		{
			ImGui::TextDisabled("Owning saved Pattern: %s | %s",
				pServerPattern->strDisplayName.c_str(),
				pServerPattern->strPatternId.c_str());
		}
		std::string ServerPlayStatus;
		std::string SoundRuntimeStatus;
		LostArk::Shared::GameplayDataRevision ExpectedServerRevision{};
		const bool_t bServerRevisionAdmitted = nullptr != m_pBossTool &&
			m_pBossTool->Observe_ServerActivePatternRevision(
				ExpectedServerRevision, ServerPlayStatus);
		const bool_t bSoundRuntimeReady = nullptr != m_pAnimationTool &&
			bServerRevisionAdmitted &&
			m_pAnimationTool->Is_ValtanCompositionPatternSoundRuntimeReady(
				ExpectedServerRevision, SoundRuntimeStatus);
		const bool_t bCanPlayServerPattern = nullptr != pServerPattern &&
			bServerRevisionAdmitted && bSoundRuntimeReady;
		ImGui::BeginDisabled(!bCanPlayServerPattern);
		if (ImGui::Button(
				"Play Owning Saved Active Revision on Server Valtan"))
		{
			std::string Status;
#ifdef _DEBUG
			if (CMainApp* const pApp = CMainApp::Get_Active();
				nullptr != pApp &&
				pApp->Debug_SelectCompletePlayPattern(
					pServerPattern->strPatternId))
			{
				(void)pApp->Debug_CompletePlaySelected(Status);
			}
			else
			{
				Status =
					"Server Pattern Play selection was rejected by the admitted inventory.";
			}
#else
			Status =
				"Server Pattern Play is unavailable outside a Debug Client.";
#endif
			m_strStatus = std::move(Status);
		}
		ImGui::EndDisabled();
		if (!SourceOwners.empty())
		{
			ImGui::TextDisabled(
				"Server target: SAVED ACTIVE REVISION. Unsaved local Pattern drafts are not sent.");
		}
		if (SourceOwners.empty())
		{
			ImGui::TextDisabled(
				"No saved Pattern owns this raw source. Use Create New Pattern, Save & Apply, then Server Play.");
		}
		else if (!bServerRevisionAdmitted && !ServerPlayStatus.empty())
		{
			ImGui::TextDisabled("Server Play unavailable: %s",
				ServerPlayStatus.c_str());
		}
		else if (!bSoundRuntimeReady && !SoundRuntimeStatus.empty())
		{
			ImGui::TextDisabled("Server Play unavailable: %s",
				SoundRuntimeStatus.c_str());
		}
	}

	ImGui::SeparatorText("Animation Sequence Tree");
	RenderResourceTree(
		m_AnimationResourceTree,
		[this, &pSelected](const std::size_t iSequence)
		{
			const CAnimation_Tool::COMPOSITION_SEQUENCE_VIEW& Sequence =
				m_AnimationSequences[iSequence];
			ImGui::PushID(Sequence.iSkillId);
			ImGui::PushID(Sequence.iSequenceIndex);
			const bool_t bSelected = Sequence.iSkillId ==
				m_iSelectedSequenceSkillId && Sequence.iSequenceIndex ==
				m_iSelectedSequenceIndex;
			const std::string Label = Sequence.strDisplayName + " | seq " +
				std::to_string(Sequence.iSequenceIndex) + " | " +
				std::to_string(Sequence.Clips.size()) + " clips";
			if (ImGui::Selectable(Label.c_str(), bSelected))
			{
				m_iSelectedSequenceSkillId = Sequence.iSkillId;
				m_iSelectedSequenceIndex = Sequence.iSequenceIndex;
				pSelected = &Sequence;
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Source action %d / sequence %d",
					Sequence.iSkillId, Sequence.iSequenceIndex);
			ImGui::PopID();
			ImGui::PopID();
		});
}

void Client::CActionCompositionWorkbench::Render_Preview(
	const VALTAN_PATTERN_VIEW* const pPattern,
	const bool_t bMutationAdmitted)
{
	ImGui::SeparatorText("Valtan Preview / Common Transport");
	CAnimation_Tool::COMPOSITION_PREVIEW_STATE Preview;
	if (nullptr != m_pAnimationTool)
	{
		if (bMutationAdmitted)
			m_pAnimationTool->Set_ValtanCompositionLoop(m_bLoopPreview);
		Preview = m_pAnimationTool->Get_ValtanCompositionPreviewState();
	}
	ImGui::TextColored(
		Preview.bModelReady ? ImVec4(0.35f, 0.86f, 0.45f, 1.f) :
			ImVec4(1.f, 0.72f, 0.24f, 1.f),
		"Dedicated Valtan Model View: %s",
		Preview.bModelReady ? "READY" : "DATA-ONLY");
	ImGui::TextDisabled(
		"The game viewport remains the real preview surface; this panel owns only typed transport and selection.");
	ImGui::TextColored(
		m_bPreviewOwnerActive ? ImVec4(0.35f, 0.86f, 0.45f, 1.f) :
			ImVec4(1.f, 0.72f, 0.24f, 1.f),
		"Viewport preview owner: %s",
		m_bPreviewOwnerActive ? "ACTIVE" :
			(m_bPreviewOwnerClaimRequested ? "CLAIM PENDING" : "INACTIVE"));
	ImGui::SameLine();
	ImGui::BeginDisabled(
		m_bPreviewOwnerActive || m_bPreviewOwnerClaimRequested);
	if (ImGui::Button("Claim Preview Owner"))
		m_bPreviewOwnerClaimRequested = true;
	ImGui::EndDisabled();
	if (!m_bPreviewOwnerActive)
	{
		ImGui::TextDisabled(
			"Effect/Camera/Animation deep-links transfer viewport input. Claim it here before using Workbench transport again.");
	}
	ImGui::TextColored(ImVec4(0.35f, 0.86f, 0.45f, 1.f),
		"LIVE transport: effective Animation slots + collider mirror + V1 Effect cue timing/yaw + Product V2 stage bindings.");
	ImGui::TextDisabled(
		"INSPECTION ONLY: Sound has no seek/stop handle; Camera/Light/World have no local composition transport adapter. Pattern-target snapshot Effects require Server Complete Play.");
	int32_t iPreviewPath = static_cast<int32_t>(m_ePreviewPath);
	ImGui::SetNextItemWidth(260.f);
	if (ImGui::Combo(
			"Preview Outcome",
			&iPreviewPath,
			"Normal / Timeout\0Counter Hit -> Groggy\0Wall Contact -> Groggy\0Wall Contact -> Part Break\0"))
	{
		if (Preview.bPlaying && nullptr != m_pAnimationTool)
		{
			std::string StopStatus;
			m_pAnimationTool->Stop_ValtanCompositionPattern(StopStatus);
		}
		m_ePreviewPath = static_cast<VALTAN_PATTERN_PREVIEW_PATH>(iPreviewPath);
		m_iPreviewDraftGeneration = 0u;
		m_iPlayheadMs = 0u;
		Invalidate_TimelineCache();
	}
	ImGui::SameLine();
	ImGui::TextDisabled("%s", PreviewPathLabel(m_ePreviewPath));

	ImGui::BeginDisabled(nullptr == m_pAnimationTool || !bMutationAdmitted);
	if (ImGui::Button("Stage Valtan Model View"))
	{
		std::string Status;
		(void)m_pAnimationTool->Stage_ValtanCompositionPreview(Status);
		m_strStatus = std::move(Status);
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(nullptr == m_pAnimationTool || nullptr == pPattern ||
		!bMutationAdmitted);
	if (ImGui::Button(Preview.bPlaying && !Preview.bPaused ? "Pause" : "Play"))
	{
		std::string Status;
		if (Preview.bPlaying)
		{
			(void)Seek_EffectivePreview(
				*pPattern,
				Preview.iPositionMs,
				!Preview.bPaused,
				Status);
		}
		else
		{
			(void)Play_EffectivePreview(*pPattern, Status);
		}
		m_strStatus = std::move(Status);
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	/* Stop is the sole stale-safe transport command: it only tears down an
	   already-running local preview and must remain available after admission
	   flips to STALE_PRESERVED. */
	ImGui::BeginDisabled(nullptr == m_pAnimationTool || !Preview.bPlaying);
	if (ImGui::Button("Stop"))
	{
		std::string Status;
		m_pAnimationTool->Stop_ValtanCompositionPattern(Status);
		m_strStatus = std::move(Status);
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(nullptr == m_pAnimationTool || nullptr == pPattern ||
		!bMutationAdmitted);
	if (ImGui::Button("Restart Preview"))
	{
		std::string Status;
		(void)Play_EffectivePreview(*pPattern, Status);
		m_strStatus = std::move(Status);
	}
	ImGui::EndDisabled();

	if (Preview.bPlaying && Preview.strPatternId == m_strSelectedPatternId &&
		m_eStagedPreviewPath == m_ePreviewPath)
		m_iPlayheadMs = Preview.iPositionMs;
	const uint32_t iDurationMs = (std::max)(m_iTimelineDurationMs, 1u);
	int32_t iPlayhead = static_cast<int32_t>((std::min)(
		m_iPlayheadMs, static_cast<uint32_t>(INT32_MAX)));
	const int32_t iMaximum = static_cast<int32_t>((std::min)(
		iDurationMs, static_cast<uint32_t>(INT32_MAX)));
	ImGui::SetNextItemWidth(-1.f);
	if (ImGui::SliderInt(
			"##CompositionPlayhead", &iPlayhead, 0, iMaximum,
			"%d ms", ImGuiSliderFlags_AlwaysClamp))
	{
		m_iPlayheadMs = static_cast<uint32_t>((std::max)(iPlayhead, 0));
		if (nullptr != pPattern && nullptr != m_pAnimationTool &&
			Preview.bModelReady && bMutationAdmitted)
		{
			std::string Status;
			(void)Seek_EffectivePreview(
				*pPattern, m_iPlayheadMs, true, Status);
			m_strStatus = std::move(Status);
		}
	}
	ImGui::Text(
		"%u / %u ms", m_iPlayheadMs, m_iTimelineDurationMs);
	ImGui::SameLine();
	ImGui::BeginDisabled(nullptr == m_pAnimationTool || !bMutationAdmitted);
	if (ImGui::Checkbox("Loop Preview", &m_bLoopPreview) &&
		nullptr != m_pAnimationTool)
	{
		m_pAnimationTool->Set_ValtanCompositionLoop(m_bLoopPreview);
	}
	ImGui::EndDisabled();
	if (!Preview.strStatus.empty())
		ImGui::TextWrapped("Preview: %s", Preview.strStatus.c_str());
}

void Client::CActionCompositionWorkbench::Render_GameplayStageDetails(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage,
	const bool_t bMutationAdmitted)
{
	if (ADMISSION_STATE::ADMITTED != m_eAdmission)
	{
		/* STALE_PRESERVED is one immutable pinned generation. Never pair its
		   canonical Pattern with a newer Balance draft merely to fill widgets. */
		ImGui::Text("%s / %s", Pattern.strPatternId.c_str(),
			Stage.strStageId.c_str());
		ImGui::TextDisabled("Action %s | %s | %u ms | sequence role %s",
			Stage.strActionId.c_str(), Stage.strStageKind.c_str(),
			Stage.iDurationMs, Stage.strSequenceRole.c_str());
		ImGui::TextColored(ImVec4(1.f, 0.70f, 0.25f, 1.f),
			"Pinned canonical generation (read only). Reload Canonical before viewing or editing a newer authoring draft.");
		ImGui::SeparatorText("Collider / Hit Schedule (Server authority)");
		ImGui::Text("Shape %s | count %u | delay %u ms | interval %u ms",
			Stage.strHitShape.c_str(), Stage.iHitCount,
			Stage.iHitDelayMs, Stage.iHitIntervalMs);
		ImGui::Text("Damage %s | response %s | attachment %s",
			Stage.strServerDamageProfileId.c_str(),
			Stage.strPlayerResponse.c_str(), Stage.strAttachmentSlot.c_str());
		ImGui::Text("Push %.2f m / %u ms | knockdown %s / %u ms",
			Stage.fPushRangeM, Stage.iPushMs,
			Stage.bKnockdown ? "true" : "false", Stage.iDownMs);
		for (const VALTAN_STAGE_ACTION_VIEW& Action : Stage.Actions)
		{
			if ("RELEASE_GRABBED_PLAYERS" != Action.strKind)
				continue;
			ImGui::SeparatorText("Grab Release");
			ImGui::Text("%s | %.2f m/s | %u ms | yaw %.1f deg",
				Action.strReleaseMode.c_str(), Action.fSpeedMps,
				Action.iDurationMs, Action.fYawOffsetDegrees);
		}
		const auto Counter = std::find_if(
			Stage.Branches.begin(), Stage.Branches.end(),
			[](const VALTAN_STAGE_BRANCH_VIEW& Branch)
			{ return "COUNTER_HIT" == Branch.strOutcome; });
		if (Counter != Stage.Branches.end())
		{
			ImGui::SeparatorText("Counter -> Groggy Edge");
			ImGui::Text("COUNTER_HIT -> %s",
				Counter->strNextActionId.has_value() ?
					Counter->strNextActionId->c_str() : "PATTERN END");
		}
		return;
	}
	if (nullptr == m_pBalanceTool)
	{
		ImGui::TextDisabled("Gameplay owner is unavailable.");
		return;
	}
	CBalanceTool::PATTERN_STAGE_EDIT Draft;
	std::string DraftStatus;
	if (!m_pBalanceTool->Get_ValtanStageDraft(
			Pattern.strPatternId, Stage.strStageId, Draft, DraftStatus))
	{
		ImGui::TextWrapped("Typed gameplay draft unavailable: %s", DraftStatus.c_str());
		return;
	}
	ImGui::Text("%s / %s", Pattern.strPatternId.c_str(), Stage.strStageId.c_str());
	ImGui::TextDisabled("Action %s | %s", Draft.actionId.c_str(), Draft.stageKind.c_str());
	const std::string strStageDetailPrefix = Stage.strStageId + "/";
	const bool_t bFocusStageClock = m_strSelectedStableId == Stage.strStageId;
	const bool_t bFocusTopology = m_strSelectedStableId ==
		Stage.strStageId + "/topology";
	const bool_t bFocusCollider = 0u == m_strSelectedStableId.rfind(
		strStageDetailPrefix + "collider", 0u);
	const auto SelectedBranch = std::find_if(
		Stage.Branches.begin(), Stage.Branches.end(),
		[this, &strStageDetailPrefix](const VALTAN_STAGE_BRANCH_VIEW& Branch)
		{
			const std::string strTarget = Branch.strNextActionId.has_value() ?
				*Branch.strNextActionId : "PATTERN END";
			return m_strSelectedStableId == strStageDetailPrefix + "branch/" +
				Branch.strOutcome + "/" + strTarget;
		});
	const bool_t bCounterAuthoringRequested = 0u == m_strSelectedStableId.rfind(
		strStageDetailPrefix + "branch/COUNTER_HIT/", 0u);
	const bool_t bFocusCounterBranch = bCounterAuthoringRequested ||
		(SelectedBranch != Stage.Branches.end() &&
			"COUNTER_HIT" == SelectedBranch->strOutcome);
	const bool_t bFocusReadOnlyBranch = SelectedBranch != Stage.Branches.end() &&
		!bFocusCounterBranch;
	const bool_t bFocusAction = 0u == m_strSelectedStableId.rfind(
		strStageDetailPrefix + "action/", 0u);
	const bool_t bFocusMotion = 0u == m_strSelectedStableId.rfind(
		strStageDetailPrefix + "motion/", 0u);
	const auto FocusDetailSection = [this](const bool_t bMatches)
	{
		if (!m_bDetailFocusRequested || !bMatches)
			return;
		ImGui::SetScrollHereY(0.12f);
		m_bDetailFocusRequested = false;
	};
	if (Pattern.bManualServerAudition)
	{
		ImGui::SeparatorText("Stage Structure");
		FocusDetailSection(bFocusTopology);
		ImGui::TextDisabled(
			"Add, move, or delete Stage nodes in Composition Boss Pattern. Details owns only the selected Stage's typed clocks, motion, collider, reactions, and Logic values.");
		if (ImGui::SmallButton("Open Boss Pattern Structure"))
		{
			m_bBossPatternWindowVisible = true;
			m_bBossPatternFocusRequested = true;
		}
	}
	else
	{
		ImGui::TextDisabled(
			"Stage add/remove/reorder is disabled for canonical rotation/gimmick Patterns.");
	}
	bool_t bWarpRushAppliedThisFrame = false;
	if ("VALTAN_WARP" == Pattern.strPatternId)
	{
		ImGui::SeparatorText("Warp Rush - All 8 Legs");
		FocusDetailSection(bFocusMotion);
		CBalanceTool::VALTAN_WARP_RUSH_EDIT RushDraft{};
		std::string RushStatus;
		const bool_t bRushReady =
			m_pBalanceTool->Get_ValtanWarpRushDraft(RushDraft, RushStatus);
		if (!bRushReady)
		{
			ImGui::TextWrapped("All-leg WARP draft unavailable: %s",
				RushStatus.c_str());
		}
		else
		{
			ImGui::TextDisabled(
				"STEP_02..STEP_09 share one Server motion contract. A change is normalized and committed to the in-memory Pattern draft only after all eight legs validate.");
			bool_t bRushChanged = false;
			ImGui::BeginDisabled(!bMutationAdmitted);
			int32_t iRetargetDelay = static_cast<int32_t>(
				RushDraft.retargetDelayMs);
			if (ImGui::DragInt(
					"Delay Before Rush (ms)##WarpAllLegs", &iRetargetDelay,
					5.f, 0, 120000,
					"%d ms", ImGuiSliderFlags_AlwaysClamp))
			{
				RushDraft.retargetDelayMs = static_cast<uint32_t>(iRetargetDelay);
				bRushChanged = true;
			}
			const double fMinimumSpeedMps = 0.1;
			const double fMaximumSpeedMps = 1000.0;
			const double fMinimumDistanceM = 0.000001;
			const double fMaximumDistanceM = 1000.0;
			bRushChanged |= ImGui::DragScalar(
				"Rush Speed (m/s)##WarpAllLegs", ImGuiDataType_Double,
				&RushDraft.speedMps, 0.05f, &fMinimumSpeedMps,
				&fMaximumSpeedMps, "%.3f", ImGuiSliderFlags_AlwaysClamp);
			bRushChanged |= ImGui::DragScalar(
				"Rush Distance (m)##WarpAllLegs", ImGuiDataType_Double,
				&RushDraft.distanceM, 0.05f, &fMinimumDistanceM,
				&fMaximumDistanceM, "%.3f", ImGuiSliderFlags_AlwaysClamp);
			int32_t iTrailingGap = static_cast<int32_t>(
				RushDraft.trailingGapMs);
			if (ImGui::DragInt(
					"Portal Gap After Rush (ms)##WarpAllLegs", &iTrailingGap,
					5.f, 0, 120000, "%d ms",
					ImGuiSliderFlags_AlwaysClamp))
			{
				RushDraft.trailingGapMs = static_cast<uint32_t>(iTrailingGap);
				bRushChanged = true;
			}
			ImGui::EndDisabled();

			if (bRushChanged)
			{
				CBalanceTool::PATTERN_STAGE_EDIT NormalizedStage{};
				if (m_pBalanceTool->Get_ValtanStageDraft(
						"VALTAN_WARP", "STEP_02", NormalizedStage, RushStatus))
				{
					NormalizedStage.portalRetargetDelayMs =
						RushDraft.retargetDelayMs;
					NormalizedStage.portalSpeedMps = RushDraft.speedMps;
					NormalizedStage.portalDistanceM = RushDraft.distanceM;
					if (CBalanceTool::Normalize_ValtanPortalRushDraft(
							NormalizedStage, RushDraft.trailingGapMs,
							RushStatus))
					{
						RushDraft.retargetDelayMs =
							NormalizedStage.portalRetargetDelayMs;
						RushDraft.speedMps = NormalizedStage.portalSpeedMps;
						RushDraft.distanceM = NormalizedStage.portalDistanceM;
						RushDraft.travelMs = RushDraft.distanceM /
							RushDraft.speedMps * 1000.0;
						RushDraft.hitCount = NormalizedStage.hitCount;
						if (m_pBalanceTool->Set_ValtanWarpRushDraft(
								RushDraft, RushStatus))
						{
							const std::string AppliedStatus = RushStatus;
							CBalanceTool::VALTAN_WARP_RUSH_EDIT Refreshed{};
							std::string RefreshStatus;
							if (m_pBalanceTool->Get_ValtanWarpRushDraft(
									Refreshed, RefreshStatus))
							{
								RushDraft = Refreshed;
							}
							RushStatus = AppliedStatus;
							bWarpRushAppliedThisFrame = true;
							Invalidate_TimelineCache();
						}
					}
				}
				m_strStatus = RushStatus;
			}
			ImGui::Text(
				"Computed leg total %u ms = delay %u ms + travel %.3f ms + portal gap %u ms",
				RushDraft.legDurationMs, RushDraft.retargetDelayMs,
				RushDraft.travelMs, RushDraft.trailingGapMs);
			ImGui::TextDisabled(
				"Swept hit count %u at 50 ms. Portal visuals use boss-root snapshots at Stage boundaries; they do not predict or expose a Server-authoritative endpoint.",
				RushDraft.hitCount);
			ImGui::TextColored(
				ImVec4(1.f, 0.70f, 0.20f, 1.f),
				"Distance endpoint currently bypasses navigation clamp; keep it inside the arena.");
		}
	}
	FocusDetailSection(bFocusStageClock);
	bool_t bChanged = false;
	ImGui::BeginDisabled(!bMutationAdmitted);
	if (Draft.stageKindEditable)
	{
		const char_t* const StageKinds[] = { "ACTIVE", "WINDUP", "GROGGY" };
		int32_t iStageKind = 0;
		for (std::size_t iKind = 0u; iKind < std::size(StageKinds); ++iKind)
		{
			if (Draft.stageKind == StageKinds[iKind])
				iStageKind = static_cast<int32_t>(iKind);
		}
		if (ImGui::Combo(
				"Stage Role", &iStageKind, StageKinds,
				static_cast<int32_t>(std::size(StageKinds))))
		{
			Draft.stageKind = StageKinds[iStageKind];
			bChanged = true;
		}
		ImGui::TextDisabled(
			"Manual Pattern only: retag this existing stable Stage here, or add a dedicated WINDUP/GROGGY/WAIT node in Composition Boss Pattern.");
	}
	else
	{
		ImGui::TextDisabled(
			"Stage Role is topology-owned for canonical gameplay Patterns.");
	}
	uint32_t iAnimationWallMs = 0u;
	const bool_t bExactWallKnown =
		ComputeExactAnimationWallMs(Draft, iAnimationWallMs);
	const bool_t bWarpLegClockOwned = Draft.portalRushMotionEditable;
	if (!bWarpLegClockOwned && Draft.durationEditable &&
		Draft.animationEditable &&
		"LOOP_TO_STAGE_END" != Draft.animationEndPolicy && bExactWallKnown)
	{
		ImGui::TextDisabled("Animation Sequence wall: %u ms", iAnimationWallMs);
		int32_t iGapMs = static_cast<int32_t>(Draft.durationMs > iAnimationWallMs ?
			Draft.durationMs - iAnimationWallMs : 0u);
		if (ImGui::DragInt(
				"Gap After Sequence / Before Next Stage (ms)", &iGapMs,
				10.f, 0, 120000, "%d ms", ImGuiSliderFlags_AlwaysClamp))
		{
			const uint64_t iDuration = static_cast<uint64_t>(iAnimationWallMs) +
				static_cast<uint32_t>(iGapMs);
			if (iDuration <= 600000u)
			{
				Draft.durationMs = static_cast<uint32_t>(iDuration);
				Draft.animationEndPolicy = 0 == iGapMs ?
					"EXACT" : "HOLD_LAST_POSE";
				bChanged = true;
			}
		}
		ImGui::TextDisabled(
			"EXACT means 0 ms gap; a positive gap holds the final pose. A true clip-between-clip blank is authored as its own WAIT/HOLD Stage.");
	}
	else if (!bWarpLegClockOwned)
	{
		int32_t iDurationMs = static_cast<int32_t>((std::min)(
			Draft.durationMs, static_cast<uint32_t>(INT32_MAX)));
		const char_t* const pDurationLabel =
			"LOOP_TO_STAGE_END" == Draft.animationEndPolicy ?
				"Loop Stage Length (ms)" : "Stage Duration (ms)";
		if (Draft.durationEditable && ImGui::DragInt(
				pDurationLabel, &iDurationMs, 10.f, 1, 120000,
				"%d ms", ImGuiSliderFlags_AlwaysClamp))
		{
			std::string ClockStatus;
			if (ApplyStageClockPolicy(
					Draft, static_cast<uint32_t>(iDurationMs), ClockStatus))
			{
				bChanged = true;
			}
			else
			{
				m_strStatus = std::move(ClockStatus);
			}
		}
	}
	if (bWarpLegClockOwned)
	{
		ImGui::TextDisabled(
			"Stage duration is derived for all eight legs from delay + distance/speed travel + Portal Gap After Rush above.");
	}
	if (!Draft.durationEditable)
		ImGui::TextDisabled("Stage duration is locked by its typed gameplay policy.");

	ImGui::SeparatorText("Collider / Hit Schedule (Server authority)");
	FocusDetailSection(bFocusCollider);
	const std::vector<std::string> DamageProfiles =
		m_pBalanceTool->Get_ValtanDamageProfileIds();
	if (!DamageProfiles.empty())
	{
		const auto CurrentProfile = std::find(
			DamageProfiles.begin(), DamageProfiles.end(), Draft.damageProfileId);
		if (CurrentProfile != DamageProfiles.end())
			m_iDamageProfileSelection = static_cast<int32_t>(
				CurrentProfile - DamageProfiles.begin());
		m_iDamageProfileSelection = (std::clamp)(
			m_iDamageProfileSelection, 0,
			static_cast<int32_t>(DamageProfiles.size() - 1u));
	}
	const bool_t bHasExistingServerHit = "NONE" != Draft.hitShape;
	const bool_t bManualColliderAddAdmitted =
		Pattern.bManualServerAudition &&
		"WAIT" != Stage.strSequenceRole && Draft.hitEditable;
	if (!bHasExistingServerHit)
	{
		ImGui::TextDisabled(
			"No Server collider. A visible Effect never becomes hit authority by itself.");
		ImGui::BeginDisabled(!bManualColliderAddAdmitted);
		if (!DamageProfiles.empty())
		{
			ImGui::SetNextItemWidth(-1.f);
			if (ImGui::BeginCombo(
					"Damage Profile", DamageProfiles[
						static_cast<std::size_t>(m_iDamageProfileSelection)].c_str()))
			{
				for (std::size_t iProfile = 0u;
					iProfile < DamageProfiles.size(); ++iProfile)
				{
					const bool_t bSelected = iProfile ==
						static_cast<std::size_t>(m_iDamageProfileSelection);
					if (ImGui::Selectable(
							DamageProfiles[iProfile].c_str(), bSelected))
						m_iDamageProfileSelection = static_cast<int32_t>(iProfile);
				}
				ImGui::EndCombo();
			}
			if (ImGui::Button("Add Server Collider"))
			{
				Draft.hitShape = "CIRCLE";
				Draft.hitOuterRadius = 5.0;
				Draft.hitCount = 1u;
				Draft.hitIntervalMs = 0u;
				Draft.hitDelayMs = 0u;
				Draft.hitOffsetsMs.clear();
				Draft.damageProfileId = DamageProfiles[
					static_cast<std::size_t>(m_iDamageProfileSelection)];
				bChanged = true;
			}
		}
		ImGui::EndDisabled();
		if (bManualColliderAddAdmitted)
		{
			ImGui::TextDisabled(
				"New Collider Add is admitted only because this is a non-WAIT manual audition Stage.");
		}
		else if ("WAIT" == Stage.strSequenceRole)
		{
			ImGui::TextDisabled(
				"New Collider Add is unavailable: WAIT remains a clock-only gap.");
		}
		else
		{
			ImGui::TextDisabled(
				"New Collider Add is unavailable on a canonical Stage with no existing hit contract. Create or select a manual audition Stage instead.");
		}
	}
	if (bHasExistingServerHit && Draft.hitEditable)
	{
		ImGui::TextDisabled(
			"Existing Server hit contract: tune its typed geometry/schedule, or remove it when no capture dependency owns it.");
		const char_t* const HitShapes[] = {
			"CIRCLE", "RING", "CONE", "BOX", "CROSS", "SIX_DIRECTIONS" };
		int32_t iShape = 0;
		for (std::size_t iKind = 0u; iKind < std::size(HitShapes); ++iKind)
			if (Draft.hitShape == HitShapes[iKind])
				iShape = static_cast<int32_t>(iKind);
		if (ImGui::Combo("Collider Shape", &iShape, HitShapes,
				static_cast<int32_t>(std::size(HitShapes))))
		{
			Draft.hitShape = HitShapes[iShape];
			Draft.hitOuterRadius = 0.0;
			Draft.hitInnerRadius = 0.0;
			Draft.hitAngleDegrees = 0.0;
			Draft.hitLength = 0.0;
			Draft.hitHalfWidth = 0.0;
			if ("CIRCLE" == Draft.hitShape)
				Draft.hitOuterRadius = 5.0;
			else if ("RING" == Draft.hitShape)
			{
				Draft.hitInnerRadius = 2.0;
				Draft.hitOuterRadius = 5.0;
			}
			else if ("CONE" == Draft.hitShape)
			{
				Draft.hitAngleDegrees = 90.0;
				Draft.hitLength = 8.0;
			}
			else
			{
				Draft.hitLength = 8.0;
				Draft.hitHalfWidth = 2.0;
			}
			bChanged = true;
		}
		if (!DamageProfiles.empty() && ImGui::BeginCombo(
				"Damage Profile", Draft.damageProfileId.c_str()))
		{
			for (const std::string& Profile : DamageProfiles)
			{
				const bool_t bSelected = Profile == Draft.damageProfileId;
				if (ImGui::Selectable(Profile.c_str(), bSelected))
				{
					Draft.damageProfileId = Profile;
					bChanged = true;
				}
			}
			ImGui::EndCombo();
		}
		if ("CIRCLE" == Draft.hitShape || "RING" == Draft.hitShape)
		{
			bChanged |= ImGui::DragScalar(
				"Outer Radius (m)", ImGuiDataType_Double,
				&Draft.hitOuterRadius, 0.05f, nullptr, nullptr, "%.2f");
		}
		if ("RING" == Draft.hitShape)
		{
			bChanged |= ImGui::DragScalar(
				"Inner Radius (m)", ImGuiDataType_Double,
				&Draft.hitInnerRadius, 0.05f, nullptr, nullptr, "%.2f");
		}
		if ("CONE" == Draft.hitShape)
		{
			bChanged |= ImGui::DragScalar(
				"Angle (deg)", ImGuiDataType_Double,
				&Draft.hitAngleDegrees, 0.5f, nullptr, nullptr, "%.1f");
		}
		if ("CONE" == Draft.hitShape || "BOX" == Draft.hitShape ||
			"CROSS" == Draft.hitShape || "SIX_DIRECTIONS" == Draft.hitShape)
		{
			bChanged |= ImGui::DragScalar(
				"Length (m)", ImGuiDataType_Double,
				&Draft.hitLength, 0.05f, nullptr, nullptr, "%.2f");
		}
		if ("BOX" == Draft.hitShape || "CROSS" == Draft.hitShape ||
			"SIX_DIRECTIONS" == Draft.hitShape)
		{
			bChanged |= ImGui::DragScalar(
				"Half Width (m)", ImGuiDataType_Double,
				&Draft.hitHalfWidth, 0.05f, nullptr, nullptr, "%.2f");
		}
		if (Draft.hitOffsetsMs.empty())
		{
			int32_t iHitDelay = static_cast<int32_t>(Draft.hitDelayMs);
			int32_t iHitInterval = static_cast<int32_t>(Draft.hitIntervalMs);
			int32_t iHitCount = static_cast<int32_t>(Draft.hitCount);
			if (ImGui::DragInt("Hit Delay (ms)", &iHitDelay, 5.f, 0, 120000))
			{
				Draft.hitDelayMs = static_cast<uint32_t>(iHitDelay);
				bChanged = true;
			}
			if (ImGui::DragInt("Hit Interval (ms)", &iHitInterval, 5.f, 0, 120000))
			{
				Draft.hitIntervalMs = static_cast<uint32_t>(iHitInterval);
				bChanged = true;
			}
			if (ImGui::DragInt("Hit Count", &iHitCount, 0.1f, 1, 64))
			{
				Draft.hitCount = static_cast<uint32_t>(iHitCount);
				if (1u == Draft.hitCount)
					Draft.hitIntervalMs = 0u;
				bChanged = true;
			}
		}
		else
		{
			ImGui::TextDisabled("Explicit Server hit offsets (%zu):",
				Draft.hitOffsetsMs.size());
			for (const uint32_t iOffsetMs : Draft.hitOffsetsMs)
				ImGui::SameLine(), ImGui::TextDisabled("%u", iOffsetMs);
		}
		const bool_t bCaptureCollider = "CAPTURE" == Draft.playerResponse;
		ImGui::BeginDisabled(bCaptureCollider);
		if (ImGui::Button("Remove Server Collider"))
		{
			Draft.hitShape = "NONE";
			Draft.hitOuterRadius = Draft.hitInnerRadius = 0.0;
			Draft.hitAngleDegrees = Draft.hitLength = Draft.hitHalfWidth = 0.0;
			Draft.hitCount = Draft.hitIntervalMs = Draft.hitDelayMs = 0u;
			Draft.hitOffsetsMs.clear();
			Draft.damageProfileId.clear();
			Draft.pushRangeM = 0.0;
			Draft.pushMs = 0u;
			Draft.knockdown = false;
			Draft.downMs = 0u;
			bChanged = true;
		}
		ImGui::EndDisabled();
		if (bCaptureCollider)
		{
			ImGui::TextDisabled(
				"Capture collider removal is blocked: grab branches, left-hand attachment, and release actions form one typed transaction.");
		}
	}
	else if (bHasExistingServerHit)
	{
		ImGui::TextDisabled(
			"This existing Server hit contract is not editable in the current Stage/admission state.");
	}

	ImGui::SeparatorText("Player Reaction / Grab Release");
	FocusDetailSection(bFocusAction);
	ImGui::BeginDisabled("NONE" == Draft.hitShape ||
		"CAPTURE" == Draft.playerResponse);
	if (ImGui::DragScalar(
			"Push Range (m)", ImGuiDataType_Double,
			&Draft.pushRangeM, 0.05f, nullptr, nullptr, "%.2f"))
	{
		if (0.0 == Draft.pushRangeM)
			Draft.pushMs = 0u;
		else if (0u == Draft.pushMs)
			Draft.pushMs = 500u;
		bChanged = true;
	}
	int32_t iPushMs = static_cast<int32_t>(Draft.pushMs);
	if (ImGui::DragInt("Push Duration (ms)", &iPushMs, 5.f, 0, 10000))
	{
		Draft.pushMs = static_cast<uint32_t>(iPushMs);
		if (0u == Draft.pushMs)
			Draft.pushRangeM = 0.0;
		else if (0.0 == Draft.pushRangeM)
			Draft.pushRangeM = 1.0;
		bChanged = true;
	}
	if (ImGui::Checkbox("Knockdown", &Draft.knockdown))
	{
		Draft.downMs = Draft.knockdown ?
			(std::max)(Draft.downMs, 1000u) : 0u;
		bChanged = true;
	}
	int32_t iDownMs = static_cast<int32_t>(Draft.downMs);
	if (ImGui::DragInt("Down Duration (ms)", &iDownMs, 5.f, 0, 30000))
	{
		Draft.downMs = static_cast<uint32_t>(iDownMs);
		Draft.knockdown = 0u != Draft.downMs;
		bChanged = true;
	}
	ImGui::EndDisabled();
	for (VALTAN_STAGE_ACTION_VIEW& Action : Draft.actions)
	{
		if ("RELEASE_GRABBED_PLAYERS" != Action.strKind)
			continue;
		ImGui::PushID(Action.strTargetId.c_str());
		ImGui::TextDisabled("Release trigger %s | target %s",
			Action.strTrigger.c_str(), Action.strTargetId.c_str());
		const char_t* const ReleaseModes[] = {
			"HOLD", "OPPOSITE_KNOCKBACK", "ARENA_EJECTION" };
		int32_t iReleaseMode = 0;
		for (std::size_t iMode = 0u; iMode < std::size(ReleaseModes); ++iMode)
		{
			if (Action.strReleaseMode == ReleaseModes[iMode])
				iReleaseMode = static_cast<int32_t>(iMode);
		}
		if (ImGui::Combo(
				"Release Mode", &iReleaseMode, ReleaseModes,
				static_cast<int32_t>(std::size(ReleaseModes))))
		{
			Action.strReleaseMode = ReleaseModes[iReleaseMode];
			if ("HOLD" == Action.strReleaseMode)
			{
				Action.fSpeedMps = 0.f;
				Action.iDurationMs = 0u;
				Action.fYawOffsetDegrees = 0.f;
			}
			else
			{
				if (Action.fSpeedMps <= 0.f)
					Action.fSpeedMps = 10.f;
				if (0u == Action.iDurationMs)
					Action.iDurationMs = 500u;
				if ("OPPOSITE_KNOCKBACK" == Action.strReleaseMode)
					Action.fYawOffsetDegrees = 0.f;
			}
			bChanged = true;
		}
		const bool_t bHoldRelease = "HOLD" == Action.strReleaseMode;
		const bool_t bArenaEjection =
			"ARENA_EJECTION" == Action.strReleaseMode;
		ImGui::BeginDisabled(bHoldRelease);
		bChanged |= ImGui::DragFloat(
			"Release Velocity (m/s)", &Action.fSpeedMps, 0.05f, 0.f, 50.f,
			"%.2f", ImGuiSliderFlags_AlwaysClamp);
		int32_t iReleaseDuration = static_cast<int32_t>(Action.iDurationMs);
		if (ImGui::DragInt(
				"Release Duration (ms)", &iReleaseDuration, 5.f, 0, 5000))
		{
			Action.iDurationMs = static_cast<uint32_t>(iReleaseDuration);
			bChanged = true;
		}
		ImGui::EndDisabled();
		ImGui::BeginDisabled(!bArenaEjection);
		bChanged |= ImGui::DragFloat(
			"Release Rotation / Yaw Offset (deg)",
			&Action.fYawOffsetDegrees, 0.5f, -180.f, 180.f, "%.1f",
			ImGuiSliderFlags_AlwaysClamp);
		ImGui::EndDisabled();
		if (bHoldRelease)
			ImGui::TextDisabled(
				"HOLD owns no launch velocity, duration, or rotation.");
		else if (!bArenaEjection)
			ImGui::TextDisabled(
				"OPPOSITE_KNOCKBACK derives direction from the boss/player relation; yaw is fixed at 0. Use ARENA_EJECTION for an authored rotation offset.");
		ImGui::PopID();
	}

	if (Draft.portalRushMotionEditable)
		ImGui::TextDisabled(
			"This selected leg's portal motion is owned by Warp Rush - All 8 Legs above.");

	if (SelectedBranch != Stage.Branches.end() && bFocusReadOnlyBranch)
	{
		ImGui::SeparatorText("Selected Logic Branch");
		FocusDetailSection(true);
		ImGui::Text("Outcome: %s", SelectedBranch->strOutcome.c_str());
		ImGui::Text("Target: %s",
			SelectedBranch->strNextActionId.has_value() ?
				SelectedBranch->strNextActionId->c_str() : "PATTERN END");
		ImGui::TextDisabled(
			"This admitted branch is read-only here. Counter authoring uses the typed COUNTER_HIT editor below.");
	}

	ImGui::SeparatorText("Counter -> Groggy Edge");
	FocusDetailSection(bFocusCounterBranch);
	const bool_t bWaitStage = "WAIT" == Stage.strSequenceRole;
	const bool_t bCounterSourceEditable =
		!bWaitStage && "WINDUP" == Stage.strStageKind;
	ImGui::BeginDisabled(!bCounterSourceEditable);
	CBalanceTool::VALTAN_COUNTER_WINDOW_EDIT Counter;
	std::string CounterStatus;
	if (m_pBalanceTool->Get_ValtanCounterWindowDraft(
			Pattern.strPatternId, Stage.strStageId, Counter, CounterStatus))
	{
		std::vector<const VALTAN_STAGE_VIEW*> GroggyTargets;
		const auto CurrentStage = std::find_if(
			Pattern.Stages.begin(), Pattern.Stages.end(),
			[&Stage](const VALTAN_STAGE_VIEW& Candidate)
			{ return Candidate.strStageId == Stage.strStageId; });
		const std::size_t iCurrentStageIndex =
			CurrentStage == Pattern.Stages.end() ? Pattern.Stages.size() :
			static_cast<std::size_t>(
				std::distance(Pattern.Stages.begin(), CurrentStage));
		for (std::size_t iCandidate = iCurrentStageIndex + 1u;
			iCandidate < Pattern.Stages.size(); ++iCandidate)
		{
			const VALTAN_STAGE_VIEW& Candidate = Pattern.Stages[iCandidate];
			if ("GROGGY" == Candidate.strStageKind &&
				Candidate.strStageId != Stage.strStageId)
			{
				GroggyTargets.push_back(&Candidate);
			}
		}
		bool_t bCounterChanged = ImGui::Checkbox("Counter Enabled", &Counter.enabled);
		if (Counter.enabled && (Counter.successStageId.empty() ||
			Counter.successActionId.empty()) && !GroggyTargets.empty())
		{
			Counter.successStageId = GroggyTargets.front()->strStageId;
			Counter.successActionId = GroggyTargets.front()->strActionId;
		}
		ImGui::BeginDisabled(!Counter.enabled || GroggyTargets.empty());
		const std::string GroggyPreview = Counter.successStageId.empty() ?
			std::string("Select Groggy Stage") :
			Counter.successStageId + " / " + Counter.successActionId;
		if (ImGui::BeginCombo("Counter Success Groggy", GroggyPreview.c_str()))
		{
			for (const VALTAN_STAGE_VIEW* const Candidate : GroggyTargets)
			{
				const bool_t bSelected =
					Candidate->strStageId == Counter.successStageId;
				const std::string Label = Candidate->strStageId + " / " +
					Candidate->strActionId;
				if (ImGui::Selectable(Label.c_str(), bSelected))
				{
					Counter.successStageId = Candidate->strStageId;
					Counter.successActionId = Candidate->strActionId;
					bCounterChanged = true;
				}
				if (bSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		ImGui::EndDisabled();
		if (Counter.enabled && GroggyTargets.empty())
		{
			Counter.enabled = false;
			bCounterChanged = false;
			ImGui::TextColored(ImVec4(1.f, 0.55f, 0.25f, 1.f),
				"Add or promote a same-Pattern GROGGY Stage before enabling Counter.");
		}
		if (bCounterChanged)
		{
			std::string Status;
			if (!m_pBalanceTool->Set_ValtanCounterWindowDraft(
					Pattern.strPatternId, Stage.strStageId, Counter, Status))
				m_strStatus = std::move(Status);
			else
				m_strStatus = std::move(Status);
		}
	}
	else
	{
		ImGui::TextDisabled("No typed Counter/Groggy edge on this Stage: %s",
			CounterStatus.c_str());
	}
	ImGui::EndDisabled();
	if (!bCounterSourceEditable)
	{
		ImGui::TextDisabled(
			bWaitStage ?
				"WAIT is a clock-only gap. Select or insert a WINDUP Stage to author Counter -> GROGGY." :
				"Counter is authored only on a WINDUP Stage and must target a later GROGGY Stage in the same Pattern.");
	}

	if (bChanged && !bWarpRushAppliedThisFrame)
	{
		std::string Status;
		if (SetValtanStageDraftWithSoundDependencyAdmission(
				m_pAnimationTool, m_pBalanceTool,
				m_bPatternShakesReady ? &m_PatternShakes : nullptr,
				Pattern, Stage, Draft, Status))
		{
			m_strStatus = std::move(Status);
			Invalidate_TimelineCache();
		}
		else
		{
			m_strStatus = std::move(Status);
		}
	}
	ImGui::EndDisabled();
}

void Client::CActionCompositionWorkbench::Render_AnimationStageDetails(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage,
	const bool_t bMutationAdmitted)
{
	if (ADMISSION_STATE::ADMITTED != m_eAdmission)
	{
		ImGui::TextWrapped(
			"Pinned canonical Animation generation (read only). Reload Canonical before viewing or editing a newer authoring draft.");
		ImGui::Text("End Policy: %s | Stage %u ms",
			Stage.strAnimationEndPolicy.c_str(), Stage.iDurationMs);
		if (Stage.ClipOccurrences.empty())
		{
			ImGui::TextDisabled("Animation Mode: NONE");
			return;
		}
		for (std::size_t iSlot = 0u;
			iSlot < Stage.ClipOccurrences.size(); ++iSlot)
		{
			const VALTAN_CLIP_OCCURRENCE_VIEW& Slot =
				Stage.ClipOccurrences[iSlot];
			ImGui::PushID(static_cast<int32_t>(iSlot));
			const bool_t bSelected =
				Slot.strClipOccurrenceId == m_strSelectedStableId;
			const std::string Label = "Slot " + std::to_string(iSlot + 1u) +
				" | " + Slot.strClipName;
			if (ImGui::Selectable(Label.c_str(), bSelected))
				m_strSelectedStableId = Slot.strClipOccurrenceId;
			ImGui::TextDisabled("%s", Slot.strClipOccurrenceId.c_str());
			ImGui::Text("Source %u ms | play %u ms | rate %.3f | loop %s",
				Slot.iSourceStartMs, Slot.iPlayMs, Slot.fPlayRate,
				Slot.bLoop ? "true" : "false");
			ImGui::PopID();
		}
		return;
	}
	if (nullptr == m_pBalanceTool)
	{
		ImGui::TextDisabled("Animation authoring owner is unavailable.");
		return;
	}
	CBalanceTool::PATTERN_STAGE_EDIT Draft;
	std::string Status;
	if (!m_pBalanceTool->Get_ValtanStageDraft(
			Pattern.strPatternId, Stage.strStageId, Draft, Status))
	{
		ImGui::TextWrapped("Animation slot draft unavailable: %s", Status.c_str());
		return;
	}
	ImGui::TextWrapped(
		"Slots are saved in Data/Valtan/Valtan.presentation.json through the same immutable authoring revision as the Stage clock. Generated patternbindings remains read-only.");
	ImGui::BeginDisabled(!bMutationAdmitted || !Draft.animationEditable);
	bool_t bChanged = false;
	if (Draft.animationSlots.empty())
	{
		ImGui::TextDisabled("Animation Mode: NONE | Stage clock and gameplay remain active.");
	}
	else
	{
		const char_t* const EndPolicies[] = {
			"EXACT", "HOLD_LAST_POSE", "LOOP_TO_STAGE_END" };
		int32_t iEndPolicy = "HOLD_LAST_POSE" == Draft.animationEndPolicy ? 1 :
			("LOOP_TO_STAGE_END" == Draft.animationEndPolicy ? 2 : 0);
		if (ImGui::Combo("End Policy", &iEndPolicy, EndPolicies,
				static_cast<int32_t>(std::size(EndPolicies))))
		{
			Draft.animationEndPolicy = EndPolicies[iEndPolicy];
			bChanged = true;
		}
	}

	std::size_t iRemove = Draft.animationSlots.size();
	std::size_t iMoveUp = Draft.animationSlots.size();
	std::size_t iMoveDown = Draft.animationSlots.size();
	std::size_t iDuplicate = Draft.animationSlots.size();
	for (std::size_t iSlot = 0u; iSlot < Draft.animationSlots.size(); ++iSlot)
	{
		CBalanceTool::ANIMATION_SLOT_EDIT& Slot = Draft.animationSlots[iSlot];
		ImGui::PushID(static_cast<int32_t>(iSlot));
		const bool_t bSelected = Slot.clipOccurrenceId == m_strSelectedStableId;
		const std::string Header = "Slot " + std::to_string(iSlot + 1u) +
			" | " + Slot.clip;
		if (ImGui::Selectable(Header.c_str(), bSelected))
		{
			m_strSelectedStableId = Slot.clipOccurrenceId;
			m_eDetailOwner = DETAIL_OWNER::ANIMATION;
		}
		ImGui::TextDisabled("%s", Slot.clipOccurrenceId.c_str());
		int32_t iSourceStartMs = static_cast<int32_t>(Slot.sourceStartMs);
		int32_t iPlayMs = static_cast<int32_t>(Slot.playMs);
		float fPlayRate = static_cast<float>(Slot.playRate);
		(void)ImGui::InputInt("Source Start (ms)", &iSourceStartMs, 5, 100);
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			Slot.sourceStartMs = static_cast<uint32_t>((std::clamp)(
				iSourceStartMs, 0, 600000));
			bChanged = true;
		}
		(void)ImGui::InputInt("Play Duration (ms)", &iPlayMs, 5, 100);
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			Slot.playMs = static_cast<uint32_t>((std::clamp)(
				iPlayMs, 0, 600000));
			bChanged = true;
		}
		(void)ImGui::InputFloat(
			"Play Rate", &fPlayRate, 0.01f, 0.1f, "%.3f");
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			Slot.playRate = (std::clamp)(fPlayRate, 0.01f, 16.f);
			bChanged = true;
		}
		if (ImGui::Checkbox("Loop to Stage End", &Slot.repeatUntilStageEnd))
		{
			if (Slot.repeatUntilStageEnd)
			{
				Slot.playMs = 0u;
				Draft.animationEndPolicy = "LOOP_TO_STAGE_END";
			}
			bChanged = true;
		}
		if (ImGui::SmallButton("Move Up")) iMoveUp = iSlot;
		ImGui::SameLine();
		if (ImGui::SmallButton("Move Down")) iMoveDown = iSlot;
		ImGui::SameLine();
		if (ImGui::SmallButton("Duplicate Clip")) iDuplicate = iSlot;
		ImGui::SameLine();
		if (ImGui::SmallButton("Remove Slot")) iRemove = iSlot;
		ImGui::Separator();
		ImGui::PopID();
	}
	if (iDuplicate < Draft.animationSlots.size())
	{
		CBalanceTool::ANIMATION_SLOT_EDIT Duplicate =
			Draft.animationSlots[iDuplicate];
		Duplicate.clipOccurrenceId = BuildNextCompositionSlotId(
			Pattern.strPatternId, Stage.strStageId, Draft.animationSlots);
		if (Duplicate.clipOccurrenceId.empty())
		{
			m_strStatus =
				"Duplicate Clip rejected: no free stable occurrence ID is available.";
		}
		else
		{
			Draft.animationSlots.insert(
				Draft.animationSlots.begin() +
					static_cast<std::ptrdiff_t>(iDuplicate + 1u),
				Duplicate);
			m_strSelectedStableId = Duplicate.clipOccurrenceId;
			bChanged = true;
		}
	}
	else if (iRemove < Draft.animationSlots.size())
	{
		const std::string RemovedOccurrenceId =
			Draft.animationSlots[iRemove].clipOccurrenceId;
		Draft.animationSlots.erase(Draft.animationSlots.begin() +
			static_cast<std::ptrdiff_t>(iRemove));
		if (m_strSelectedStableId == RemovedOccurrenceId)
		{
			m_strSelectedStableId = Draft.animationSlots.empty() ?
				Stage.strStageId : Draft.animationSlots.front().clipOccurrenceId;
		}
		bChanged = true;
	}
	else if (iMoveUp > 0u && iMoveUp < Draft.animationSlots.size())
	{
		std::swap(Draft.animationSlots[iMoveUp],
			Draft.animationSlots[iMoveUp - 1u]);
		bChanged = true;
	}
	else if (iMoveDown + 1u < Draft.animationSlots.size())
	{
		std::swap(Draft.animationSlots[iMoveDown],
			Draft.animationSlots[iMoveDown + 1u]);
		bChanged = true;
	}
	if (bChanged)
	{
		const bool_t bOneRepeatedClip = Draft.animationSlots.size() > 1u &&
			std::all_of(Draft.animationSlots.begin() + 1u,
				Draft.animationSlots.end(),
				[&Draft](const CBalanceTool::ANIMATION_SLOT_EDIT& Slot)
				{
					return Slot.clip == Draft.animationSlots.front().clip;
				});
		Draft.animationRepeatCount = Draft.animationSlots.empty() ? 0u :
			(bOneRepeatedClip ?
				static_cast<uint32_t>(Draft.animationSlots.size()) : 1u);
		if (Draft.animationSlots.empty())
			Draft.animationEndPolicy = "NONE";
		if ("LOOP_TO_STAGE_END" != Draft.animationEndPolicy)
		{
			uint64_t iWallMs = 0u;
			for (const CBalanceTool::ANIMATION_SLOT_EDIT& Slot :
				Draft.animationSlots)
			{
				if (0u == Slot.playMs || Slot.repeatUntilStageEnd ||
					!std::isfinite(Slot.playRate) || Slot.playRate <= 0.0)
				{
					iWallMs = 0u;
					break;
				}
				iWallMs += static_cast<uint64_t>(std::llround(
					static_cast<double>(Slot.playMs) / Slot.playRate));
			}
			if (0u != iWallMs && iWallMs <= 600000u)
			{
				if ("EXACT" == Draft.animationEndPolicy ||
					Draft.durationMs < iWallMs)
				{
					Draft.durationMs = static_cast<uint32_t>(iWallMs);
				}
			}
		}
		if (SetValtanStageDraftWithSoundDependencyAdmission(
				m_pAnimationTool, m_pBalanceTool,
				m_bPatternShakesReady ? &m_PatternShakes : nullptr,
				Pattern, Stage, Draft, Status))
		{
			m_strStatus = std::move(Status);
			Invalidate_TimelineCache();
		}
		else
		{
			m_strStatus = std::move(Status);
		}
	}
	ImGui::EndDisabled();
	if (!Draft.animationEditable)
	{
		ImGui::TextDisabled(
			"This canonical Stage has no editable animation occurrence. Manual audition animation NONE Stages admit their first Sequence through Replace Stage Slots.");
	}
	else if (Draft.animationSlots.empty())
	{
		ImGui::TextDisabled(
			"Animation NONE: select an extracted Sequence on the left and use Replace Stage Slots to assign the first stable occurrences.");
	}
	ImGui::SeparatorText("Sequence Resource");
	ImGui::TextDisabled(
		"Use the Animation Sequences browser on the left to preview and Replace/Append exact source sequences.");
}

void Client::CActionCompositionWorkbench::Request_EffectOwner(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage,
	const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue)
{
	m_strEffectPatternId = Pattern.strPatternId;
	m_strEffectStageId = Stage.strStageId;
	m_strEffectOccurrenceId = Cue.strOccurrenceId;
	m_strEffectAssetId = Cue.strEffectAssetId;
	m_bEffectToolOpenRequested = true;
}

void Client::CActionCompositionWorkbench::Render_Details(
	const VALTAN_PATTERN_VIEW* const pPattern,
	const VALTAN_STAGE_VIEW* const pStage,
	const bool_t bMutationAdmitted,
	const bool_t bPatternMutationAdmitted)
{
	ImGui::SeparatorText("Details");
	ImGui::Text("Owner: %s", Owner_Label(m_eDetailOwner));
	const bool_t bSavedServerPending = m_bPatternSaveResultAvailable &&
		!m_bPatternSaveSucceeded &&
		(std::string::npos != m_strPatternSaveStatus.find(
			"Validated data files were saved") ||
		std::string::npos != m_strPatternSaveStatus.find(
			"Data files were saved"));
	const bool_t bLastSaveFailed = m_bAuthoringDraftDirty &&
		m_bPatternSaveResultAvailable && !m_bPatternSaveSucceeded &&
		!bSavedServerPending;
	const char_t* const pSaveSummary = bSavedServerPending ?
		"SAVED / SERVER PENDING" : (!m_bAuthoringDraftDirty ?
			"SAVED" : (bLastSaveFailed ? "SAVE FAILED" : "UNSAVED"));
	const ImVec4 SaveSummaryColor =
		!m_bAuthoringDraftDirty && !bSavedServerPending ?
		ImVec4(0.35f, 0.86f, 0.45f, 1.f) :
		(bLastSaveFailed ? ImVec4(1.f, 0.38f, 0.30f, 1.f) :
			ImVec4(1.f, 0.72f, 0.25f, 1.f));
	ImGui::TextColored(SaveSummaryColor, "Pattern: %s", pSaveSummary);
	const bool_t bCanSavePattern = nullptr != pPattern &&
		bPatternMutationAdmitted && m_bAuthoringDraftDirty &&
		nullptr != m_pBalanceTool;
	ImGui::BeginDisabled(!bCanSavePattern);
	if (ImGui::Button("Save & Apply##CompositionDetails"))
		m_bSavePatternRequested = true;
	ImGui::EndDisabled();
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
	{
		ImGui::SetTooltip(
			bCanSavePattern ?
			"Save the validated Pattern and request a safe Server apply. The result below says whether it is active, pending, or needs restart/re-entry." :
			"Save & Apply is enabled for an admitted unsaved Pattern draft after separate Sound owner changes are saved or discarded.");
	}
	if (bLastSaveFailed)
	{
		ImGui::TextDisabled(
			"Open Save / Validate / Server > Advanced Diagnostics for the exact reason.");
	}
	if (nullptr == pPattern)
	{
		ImGui::TextDisabled("Select one canonical Pattern.");
		return;
	}
	bool_t bPatternSoundDirty = false;
	std::string PatternSoundStatus;
	const VALTAN_PATTERN_SOUND_CUE_DOCUMENT* const pPatternSounds =
		ADMISSION_STATE::ADMITTED != m_eAdmission ||
		nullptr == m_pAnimationTool ? nullptr :
		m_pAnimationTool->Get_ValtanCompositionPatternSoundDraft(
			bPatternSoundDirty, PatternSoundStatus);
	if (!PatternSoundStatus.empty())
		m_strSoundStatus = PatternSoundStatus;
	std::string strFirstSoundOccurrence;
	if (nullptr != pStage && nullptr != pPatternSounds)
	{
		const auto FirstSound = std::find_if(
			pPatternSounds->Cues.begin(), pPatternSounds->Cues.end(),
			[pPattern, pStage](const VALTAN_PATTERN_SOUND_CUE& Cue)
			{
				return Cue.strPatternId == pPattern->strPatternId &&
					Cue.strStageId == pStage->strStageId;
			});
		if (FirstSound != pPatternSounds->Cues.end())
			strFirstSoundOccurrence = FirstSound->strOccurrenceId;
	}
	const auto OwnerButton = [this](const char_t* pLabel,
		const DETAIL_OWNER eOwner, const bool_t bEnabled,
		const std::string& strStableId)
	{
		ImGui::BeginDisabled(!bEnabled);
		const bool_t bActive = m_eDetailOwner == eOwner;
		if (bActive)
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.43f, 0.72f, 1.f));
		if (ImGui::SmallButton(pLabel))
		{
			m_eDetailOwner = eOwner;
			m_strSelectedStableId = strStableId;
			m_bDetailFocusRequested = true;
		}
		if (bActive)
			ImGui::PopStyleColor();
		ImGui::EndDisabled();
	};
	OwnerButton("Pattern Root", DETAIL_OWNER::PATTERN, true,
		pPattern->strPatternId);
	ImGui::SameLine();
	OwnerButton("Gameplay / Logic / Collider", DETAIL_OWNER::GAMEPLAY_STAGE,
		nullptr != pStage,
		nullptr == pStage ? std::string{} : pStage->strStageId);
	ImGui::SameLine();
	OwnerButton("Animation", DETAIL_OWNER::ANIMATION,
		nullptr != pStage && (pPattern->bManualServerAudition ||
			!pStage->ClipOccurrences.empty()),
		nullptr == pStage ? std::string{} :
			(pStage->ClipOccurrences.empty() ? pStage->strStageId :
				pStage->ClipOccurrences.front().strClipOccurrenceId));
	OwnerButton("Effect", DETAIL_OWNER::EFFECT,
		nullptr != pStage && "WAIT" != pStage->strSequenceRole &&
			!pStage->bSuppressAnimation && !pStage->ClipOccurrences.empty(),
		nullptr == pStage || pStage->ProductCues.empty() ? std::string{} :
			pStage->ProductCues.front().strOccurrenceId);
	ImGui::SameLine();
	OwnerButton("Sound", DETAIL_OWNER::SOUND,
		nullptr != pStage && nullptr != pPatternSounds &&
			!pStage->ClipOccurrences.empty(),
		strFirstSoundOccurrence);
	ImGui::SameLine();
	OwnerButton("Camera", DETAIL_OWNER::CAMERA,
		nullptr != pStage && !pStage->CameraInvocations.empty(),
		nullptr == pStage || pStage->CameraInvocations.empty() ? std::string{} :
			pStage->CameraInvocations.front().strCameraInvocationId);
	if (DETAIL_OWNER::PATTERN == m_eDetailOwner || nullptr == pStage)
	{
		ImGui::TextDisabled(
			"Pattern Root is whole-pattern metadata and flow context. Edit branches and typed actions from Logic lane blocks.");
		ImGui::TextWrapped("%s", pPattern->strDisplayName.c_str());
		ImGui::TextDisabled("%s", pPattern->strPatternId.c_str());
		ImGui::Text("Category: %s", pPattern->strCategory.c_str());
		ImGui::Text("Phase: %u .. %u", pPattern->iMinimumPhase, pPattern->iMaximumPhase);
		ImGui::Text("Target: %s | Aim: %s",
			pPattern->strTargetPolicy.c_str(), pPattern->strAimPolicy.c_str());
		ImGui::Text("Range: %.2f .. %.2f m",
			pPattern->fMinimumRange, pPattern->fMaximumRange);
		ImGui::Text("Stages: %zu | Source Sequence: %u",
			pPattern->Stages.size(), pPattern->iSourceSequenceIndex);
		if (pPattern->bManualServerAudition)
		{
			ImGui::SeparatorText("New Pattern Authoring Coverage");
			ImGui::BulletText(
				"Sequence slots: select a Stage, open Animation Detail, then Replace/Append an exact source Sequence.");
			ImGui::BulletText(
				"Internal gap: add a WAIT / GAP node in Composition Boss Pattern, or tune the selected Stage's trailing gap in Details.");
			ImGui::BulletText(
				"Server collider: open a non-WAIT Stage's Gameplay Detail and Add Server Collider; Effect geometry is never copied into hit authority.");
			ImGui::BulletText(
				"Counter -> Groggy: insert/select WINDUP and a later GROGGY Stage, then enable the typed edge in Gameplay Detail.");
			ImGui::TextColored(
				ImVec4(1.f, 0.70f, 0.25f, 1.f),
				"Grab release action creation: unavailable in this revision. The typed Balance owner keeps joined action inventory read-only; Workbench can tune mode, velocity, duration and yaw only after a RELEASE_GRABBED_PLAYERS action is admitted by a dedicated gameplay transaction.");
		}
		ImGui::SeparatorText("Sequence Sources");
		for (const VALTAN_PRESENTATION_SOURCE_VIEW& Source :
			pPattern->PresentationSources)
		{
			ImGui::BulletText("%s | action %u | sequence %u",
				Source.strRole.c_str(), Source.iSourceActionId,
				Source.iSequenceIndex);
		}
		if (pPattern->ServerMotion.has_value())
		{
			const VALTAN_PATTERN_SERVER_MOTION_VIEW& Motion = *pPattern->ServerMotion;
			ImGui::SeparatorText("Pattern Motion");
			ImGui::Text("%s | anchor %s", Motion.strKind.c_str(), Motion.strAnchorId.c_str());
			ImGui::Text("Landing %.2f, %.2f, %.2f | apex %.2f",
				Motion.LandingPosition[0], Motion.LandingPosition[1],
				Motion.LandingPosition[2], Motion.fApexHeight);
		}
		ImGui::SeparatorText("Composition Navigation");
		ImGui::TextWrapped(
			"Select a Stage/Animation/Effect/Sound/Logic/Collider/Camera block in the Sequencer. Details opens the exact typed owner section; Pattern view stays concise.");
		if (ImGui::TreeNodeEx(
				"Stage Coverage Diagnostics (read-only)",
				ImGuiTreeNodeFlags_None))
		{
			ImGui::BeginDisabled();
			ImGui::SeparatorText("Legacy Stage Coverage Cards");
			for (const VALTAN_STAGE_VIEW& Stage : pPattern->Stages)
			{
			const auto Release = std::find_if(
				Stage.Actions.begin(), Stage.Actions.end(),
				[](const VALTAN_STAGE_ACTION_VIEW& Action)
				{ return "RELEASE_GRABBED_PLAYERS" == Action.strKind; });
			const auto Counter = std::find_if(
				Stage.Branches.begin(), Stage.Branches.end(),
				[](const VALTAN_STAGE_BRANCH_VIEW& Branch)
				{ return "COUNTER_HIT" == Branch.strOutcome; });
			const bool_t bCounter = Counter != Stage.Branches.end();
			const bool_t bCollider =
				!Stage.strHitShape.empty() && "NONE" != Stage.strHitShape;
			const bool_t bAnimation = !Stage.ClipOccurrences.empty();
			uint64_t iExplicitAnimationWallMs = 0u;
			bool_t bAnimationFillsStage = false;
			for (const VALTAN_CLIP_OCCURRENCE_VIEW& Clip : Stage.ClipOccurrences)
			{
				if (Clip.bLoop || 0u == Clip.iPlayMs)
				{
					bAnimationFillsStage = true;
					continue;
				}
				const double fRate = Clip.fPlayRate > 0.f ? Clip.fPlayRate : 1.0;
				iExplicitAnimationWallMs += static_cast<uint64_t>(std::llround(
					static_cast<double>(Clip.iPlayMs) / fRate));
			}
			ImGui::PushID(Stage.strStageId.c_str());
			ImGui::Text("%s | %s | %u ms", Stage.strStageId.c_str(),
				Stage.strStageKind.c_str(), Stage.iDurationMs);
			if (!bAnimation)
			{
				ImGui::TextDisabled(
					"No animation slots | full %u ms is the Server Stage clock/gap.",
					Stage.iDurationMs);
			}
			else if (!bAnimationFillsStage &&
				iExplicitAnimationWallMs < Stage.iDurationMs)
			{
				ImGui::TextDisabled(
					"Explicit clip wall %llu ms | trailing gap / pose hold %llu ms.",
					static_cast<unsigned long long>(iExplicitAnimationWallMs),
					static_cast<unsigned long long>(
						Stage.iDurationMs - iExplicitAnimationWallMs));
			}
			else
			{
				ImGui::TextDisabled(
					"Animation fills the Stage clock (%zu slot%s).",
					Stage.ClipOccurrences.size(),
					1u == Stage.ClipOccurrences.size() ? "" : "s");
			}
			ImGui::TextDisabled(
				"Edit Stage Clock / Gap: select the Stage lane block in Sequencer.");
			if (Release != Stage.Actions.end())
			{
				ImGui::TextDisabled(
					"Grab release: %s | velocity %.2f m/s | duration %u ms | yaw %.1f deg",
					Release->strReleaseMode.c_str(), Release->fSpeedMps,
					Release->iDurationMs, Release->fYawOffsetDegrees);
				ImGui::TextDisabled(
					"Edit Grab Release Velocity / Rotation: select its Logic lane block.");
			}
			else
			{
				ImGui::TextDisabled(
					"Grab release: NONE | this Stage has no admitted RELEASE_GRABBED_PLAYERS action.");
			}
			ImGui::TextDisabled("Counter -> Groggy: %s%s%s",
				bCounter ? "true" : "false",
				bCounter ? " | target " : "",
				bCounter && Counter->strNextActionId.has_value() ?
					Counter->strNextActionId->c_str() : "");
			if (bCounter || (pPattern->bManualServerAudition &&
				"WINDUP" == Stage.strStageKind))
			{
				ImGui::TextDisabled(bCounter ?
					"Edit Counter -> Groggy: select its Logic lane block." :
					"Configure Counter -> Groggy: use Logic lane +.");
			}
			ImGui::TextDisabled("Server collider: %s%s%s",
				bCollider ? Stage.strHitShape.c_str() : "NONE",
				bCollider ? " | damage " : "",
				bCollider ? Stage.strServerDamageProfileId.c_str() : "");
			const bool_t bCanAddManualCollider =
				pPattern->bManualServerAudition &&
				"WAIT" != Stage.strSequenceRole;
			if (bCollider || bCanAddManualCollider)
			{
				ImGui::TextDisabled(bCollider ?
					"Edit Collider / Hit Schedule: select its Collider lane block." :
					"Add Server Collider / Hit Schedule: use Collider lane +.");
			}
			if (bAnimation || pPattern->bManualServerAudition)
			{
				ImGui::TextDisabled(bAnimation ?
					"Edit Sequence Slots: select its Animation lane block." :
					"Assign First Sequence Slots: use Animation lane +.");
			}
			ImGui::Separator();
				ImGui::PopID();
			}
			ImGui::EndDisabled();
			ImGui::TreePop();
		}
		return;
	}

	if (DETAIL_OWNER::GAMEPLAY_STAGE == m_eDetailOwner)
	{
		Render_GameplayStageDetails(
			*pPattern, *pStage, bPatternMutationAdmitted);
		return;
	}
	if (DETAIL_OWNER::ANIMATION == m_eDetailOwner)
	{
		Render_AnimationStageDetails(
			*pPattern, *pStage, bPatternMutationAdmitted);
		return;
	}
	if (DETAIL_OWNER::EFFECT == m_eDetailOwner)
	{
		ImGui::SeparatorText("Stage Effect Invocations");
		ImGui::TextWrapped(
			"Typed source owner: Data/Valtan/Valtan.presentation.json");
		ImGui::TextDisabled(
			"Valtan.patterneffectcues.json is a read-only generated Product. Effect asset bodies remain owned by Effect Tool.");
		const bool_t bEffectStageEligible =
			"WAIT" != pStage->strSequenceRole && !pStage->bSuppressAnimation &&
			!pStage->ClipOccurrences.empty();
		if (!bEffectStageEligible)
		{
			ImGui::TextDisabled(
				"Select a non-WAIT Stage with at least one exact Animation Sequence occurrence.");
			return;
		}
		if (!m_bSemanticValtanEffectLoadAttempted)
			Reload_SemanticValtanEffects();
		ImGui::InputTextWithHint(
			"##ValtanEffectSearch", "Filter authored effect.valtan.*...",
			m_EffectSearch.data(), m_EffectSearch.size());
		ImGui::SameLine();
		if (ImGui::SmallButton("Refresh Effect Catalog"))
			Reload_SemanticValtanEffects();
		ImGui::TextDisabled(
			"%zu direct-authored Valtan Effects (semantic catalog only).",
			m_SemanticValtanEffectAssetIds.size());

		if (!pStage->ProductCues.empty())
		{
			const auto selected = std::find_if(
				pStage->ProductCues.begin(), pStage->ProductCues.end(),
				[this](const VALTAN_PRODUCT_EFFECT_CUE_VIEW& cue)
				{
					return cue.strOccurrenceId == m_strSelectedStableId;
				});
			if (pStage->ProductCues.end() == selected)
				m_strSelectedStableId = pStage->ProductCues.front().strOccurrenceId;
			const auto current = std::find_if(
				pStage->ProductCues.begin(), pStage->ProductCues.end(),
				[this](const VALTAN_PRODUCT_EFFECT_CUE_VIEW& cue)
				{
					return cue.strOccurrenceId == m_strSelectedStableId;
				});
			const char_t* const pPreview = current == pStage->ProductCues.end() ?
				"Select invocation" : current->strEffectAssetId.c_str();
			if (ImGui::BeginCombo("Existing invocation", pPreview))
			{
				for (const VALTAN_PRODUCT_EFFECT_CUE_VIEW& cue :
					pStage->ProductCues)
				{
					const bool_t bSelected =
						cue.strOccurrenceId == m_strSelectedStableId;
					const std::string label = cue.strEffectAssetId + "##" +
						cue.strOccurrenceId;
					if (ImGui::Selectable(label.c_str(), bSelected))
					{
						m_strSelectedStableId = cue.strOccurrenceId;
						m_strEffectEditIdentity.clear();
					}
					if (bSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}

		const auto RenderEffectAssetPicker = [this](
			const char_t* const pLabel, std::string& selection)
		{
			const char_t* const pPreview = selection.empty() ?
				"Select authored Valtan Effect" : selection.c_str();
			if (!ImGui::BeginCombo(pLabel, pPreview))
				return;
			for (const std::string& EffectAssetId :
				m_SemanticValtanEffectAssetIds)
			{
				if (!ContainsInsensitive(EffectAssetId, m_EffectSearch.data()))
					continue;
				const bool_t bSelected = EffectAssetId == selection;
				if (ImGui::Selectable(EffectAssetId.c_str(), bSelected))
					selection = EffectAssetId;
				if (bSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		};
		const auto RenderClipOccurrencePicker = [pStage](
			const char_t* const pLabel, std::string& selection)
		{
			const char_t* const pPreview = selection.empty() ?
				"Select exact Sequence slot" : selection.c_str();
			if (!ImGui::BeginCombo(pLabel, pPreview))
				return false;
			bool_t bChanged = false;
			for (const VALTAN_CLIP_OCCURRENCE_VIEW& clip :
				pStage->ClipOccurrences)
			{
				const bool_t bSelected =
					clip.strClipOccurrenceId == selection;
				const std::string label = clip.strClipName + " | " +
					clip.strClipOccurrenceId + "##" + pLabel;
				if (ImGui::Selectable(label.c_str(), bSelected))
				{
					selection = clip.strClipOccurrenceId;
					bChanged = true;
				}
				if (bSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
			return bChanged;
		};

		const auto Found = std::find_if(
			pStage->ProductCues.begin(), pStage->ProductCues.end(),
			[this](const VALTAN_PRODUCT_EFFECT_CUE_VIEW& cue)
			{
				return cue.strOccurrenceId == m_strSelectedStableId;
			});
		if (Found != pStage->ProductCues.end())
		{
			ImGui::SeparatorText("Selected Invocation Details");
			const std::string editIdentity = Found->strBindingId + "/" +
				Found->strOccurrenceId;
			if (m_strEffectEditIdentity != editIdentity)
			{
				m_EffectCueEditDraft = *Found;
				m_strEffectEditIdentity = editIdentity;
			}
			VALTAN_PRODUCT_EFFECT_CUE_VIEW& Draft = m_EffectCueEditDraft;
			ImGui::TextDisabled("Cue: %s", Draft.strBindingId.c_str());
			ImGui::TextDisabled("Occurrence: %s", Draft.strOccurrenceId.c_str());
			RenderEffectAssetPicker("Effect asset", Draft.strEffectAssetId);
			if (RenderClipOccurrencePicker(
					"Clip occurrence", Draft.strClipOccurrenceId))
			{
				const auto clip = std::find_if(
					pStage->ClipOccurrences.begin(), pStage->ClipOccurrences.end(),
					[&Draft](const VALTAN_CLIP_OCCURRENCE_VIEW& candidate)
					{
						return candidate.strClipOccurrenceId ==
							Draft.strClipOccurrenceId;
					});
				if (pStage->ClipOccurrences.end() != clip)
				{
					Draft.iSourceStartMs = clip->iSourceStartMs;
					Draft.iSourceEndMs = 0u;
					Draft.bHasSourceEnd = false;
					Draft.strStopPolicy = "natural";
					Draft.eStopPolicy = EFFECT_STOP_POLICY::NATURAL;
					if (!clip->bLoop)
						Draft.strRepeatPolicy = "once";
				}
			}
			ImGui::InputScalar(
				"Source start (ms)", ImGuiDataType_U32, &Draft.iSourceStartMs);
			bool_t bExplicitEnd = Draft.bHasSourceEnd;
			if (ImGui::Checkbox("Explicit cue end", &bExplicitEnd))
			{
				Draft.bHasSourceEnd = bExplicitEnd;
				Draft.strStopPolicy = bExplicitEnd ? "cue_end" : "natural";
				Draft.eStopPolicy = bExplicitEnd ?
					EFFECT_STOP_POLICY::CUE_END : EFFECT_STOP_POLICY::NATURAL;
				Draft.iSourceEndMs = bExplicitEnd ?
					Draft.iSourceStartMs + 1u : 0u;
			}
			ImGui::BeginDisabled(!Draft.bHasSourceEnd);
			ImGui::InputScalar(
				"Source end (ms)", ImGuiDataType_U32, &Draft.iSourceEndMs);
			ImGui::EndDisabled();

			const bool_t bTargetAnchorAdmitted =
				"LOCK_NEAREST_ON_START" == pPattern->strTargetPolicy ||
				"LOCK_RANDOM_ALIVE_ON_START" == pPattern->strTargetPolicy ||
				"LOCK_RANDOM_ALIVE_BEHIND_ON_START" == pPattern->strTargetPolicy;
			const bool_t bArenaAnchorAdmitted = pPattern->ServerMotion.has_value() &&
				"LEAP_TO_ANCHOR" == pPattern->ServerMotion->strKind &&
				pPattern->ServerMotion->bMoveToAnchorBeforeTakeoff;
			const bool_t bArenaFacingAnchorAdmitted = bArenaAnchorAdmitted &&
				"LOCK_FACING_ON_START" == pPattern->strAimPolicy &&
				"LOCK_RANDOM_ALIVE_ON_START" == pPattern->strTargetPolicy;
			if (ImGui::BeginCombo("Anchor", Draft.strAnchorSlotId.c_str()))
			{
				for (const char_t* const pAnchor :
					{ "root", "pattern.target.snapshot", "arena.center",
					  "arena.center.facing" })
				{
					const bool_t bAllowed = "root" == std::string_view(pAnchor) ||
						("pattern.target.snapshot" == std::string_view(pAnchor) &&
						 bTargetAnchorAdmitted) ||
						("arena.center" == std::string_view(pAnchor) &&
						 bArenaAnchorAdmitted) ||
						("arena.center.facing" == std::string_view(pAnchor) &&
						 bArenaFacingAnchorAdmitted);
					ImGui::BeginDisabled(!bAllowed);
					if (ImGui::Selectable(
							pAnchor, Draft.strAnchorSlotId == pAnchor) && bAllowed)
					{
						Draft.strAnchorSlotId = pAnchor;
						if ("root" != Draft.strAnchorSlotId)
						{
							Draft.strFollowPolicy = "snapshot";
							Draft.eFollowPolicy = EFFECT_FOLLOW_POLICY::SNAPSHOT;
						}
					}
					ImGui::EndDisabled();
				}
				ImGui::EndCombo();
			}
			if ("root" == Draft.strAnchorSlotId)
			{
				if (ImGui::BeginCombo(
						"Follow policy", Draft.strFollowPolicy.c_str()))
				{
					for (const char_t* const pPolicy : { "follow", "snapshot" })
					{
						if (ImGui::Selectable(
								pPolicy, Draft.strFollowPolicy == pPolicy))
						{
							Draft.strFollowPolicy = pPolicy;
							Draft.eFollowPolicy = "snapshot" ==
								Draft.strFollowPolicy ?
								EFFECT_FOLLOW_POLICY::SNAPSHOT :
								EFFECT_FOLLOW_POLICY::FOLLOW;
						}
					}
					ImGui::EndCombo();
				}
			}
			else
			{
				ImGui::TextDisabled("Follow policy: snapshot (required by anchor)");
			}
			const auto selectedClip = std::find_if(
				pStage->ClipOccurrences.begin(), pStage->ClipOccurrences.end(),
				[&Draft](const VALTAN_CLIP_OCCURRENCE_VIEW& clip)
				{
					return clip.strClipOccurrenceId ==
						Draft.strClipOccurrenceId;
				});
			const bool_t bSelectedClipLoops =
				pStage->ClipOccurrences.end() != selectedClip && selectedClip->bLoop;
			if (ImGui::BeginCombo("Repeat policy", Draft.strRepeatPolicy.c_str()))
			{
				if (ImGui::Selectable("once", "once" == Draft.strRepeatPolicy))
					Draft.strRepeatPolicy = "once";
				ImGui::BeginDisabled(!bSelectedClipLoops);
				if (ImGui::Selectable(
						"each_loop", "each_loop" == Draft.strRepeatPolicy) &&
					bSelectedClipLoops)
				{
					Draft.strRepeatPolicy = "each_loop";
				}
				ImGui::EndDisabled();
				ImGui::EndCombo();
			}
			ImGui::DragFloat3(
				"Position", &Draft.LocalTransform.vPosition.x, 0.01f);
			ImGui::DragFloat3(
				"Rotation (deg)",
				&Draft.LocalTransform.vRotationDegrees.x, 0.5f);
			ImGui::DragFloat3(
				"Scale", &Draft.LocalTransform.vScale.x, 0.01f, 0.001f, 1000.f);
			const bool_t bCompositionScalePolicy =
				0u == Draft.strBindingId.rfind("cue.valtan.composition.", 0u);
			ImGui::BeginDisabled(!bCompositionScalePolicy);
			if (ImGui::BeginCombo("Scale policy", Draft.strScalePolicy.c_str()))
			{
				for (const char_t* const pPolicy :
					{ "OWNER_RELATIVE", "GAMEPLAY_FOOTPRINT", "ARENA_ABSOLUTE" })
				{
					if (!ImGui::Selectable(
							pPolicy, Draft.strScalePolicy == pPolicy))
						continue;
					Draft.strScalePolicy = pPolicy;
					Draft.bHasExplicitScalePolicy = true;
					if ("OWNER_RELATIVE" == Draft.strScalePolicy)
					{
						Draft.eScalePolicy =
							VALTAN_PATTERN_EFFECT_SCALE_POLICY::OWNER_RELATIVE;
						Draft.vWorldScale = { 1.f, 1.f, 1.f };
					}
					else
					{
						Draft.eScalePolicy = "GAMEPLAY_FOOTPRINT" ==
							Draft.strScalePolicy ?
							VALTAN_PATTERN_EFFECT_SCALE_POLICY::GAMEPLAY_FOOTPRINT :
							VALTAN_PATTERN_EFFECT_SCALE_POLICY::ARENA_ABSOLUTE;
						Draft.vWorldScale = { 1.5f, 1.5f, 1.5f };
					}
				}
				ImGui::EndCombo();
			}
			ImGui::EndDisabled();
			if (!bCompositionScalePolicy)
			{
				ImGui::TextDisabled(
					"This admitted legacy cue keeps its canonical scalePolicy kind; tune local Scale above or create a composition cue.");
			}
			ImGui::BeginDisabled(
				!bPatternMutationAdmitted || nullptr == m_pBalanceTool);
			if (ImGui::Button("Update Invocation"))
			{
				std::string Status;
				if (m_pBalanceTool->Update_ValtanStageEffectCue(
						pPattern->strPatternId, pStage->strStageId,
						pStage->strActionId, Found->strBindingId,
						Found->strOccurrenceId, Draft, Status))
				{
					m_strStatus = std::move(Status);
					m_strEffectEditIdentity.clear();
					ImGui::EndDisabled();
					return;
				}
				m_strStatus = std::move(Status);
			}
			ImGui::SameLine();
			if (ImGui::Button("Remove Invocation"))
			{
				std::string Status;
				if (m_pBalanceTool->Remove_ValtanStageEffectCue(
						pPattern->strPatternId, pStage->strStageId,
						pStage->strActionId, Found->strBindingId,
						Found->strOccurrenceId, Found->strEffectAssetId,
						Found->strClipOccurrenceId, Status))
				{
					m_strStatus = std::move(Status);
					m_strSelectedStableId = pStage->strStageId;
					m_strEffectEditIdentity.clear();
					ImGui::EndDisabled();
					return;
				}
				m_strStatus = std::move(Status);
			}
			ImGui::EndDisabled();

			const VALTAN_PATTERN_VIEW* pSavedPattern = nullptr;
			const VALTAN_STAGE_VIEW* pSavedStage = nullptr;
			const VALTAN_PRODUCT_EFFECT_CUE_VIEW* pSavedCue = nullptr;
			if (ADMISSION_STATE::ADMITTED == m_eAdmission)
			{
				pSavedPattern = Find_SelectedPattern();
				if (nullptr != pSavedPattern &&
					pSavedPattern->strPatternId == pPattern->strPatternId)
				{
					const auto savedStage = std::find_if(
						pSavedPattern->Stages.begin(), pSavedPattern->Stages.end(),
						[pStage](const VALTAN_STAGE_VIEW& candidate)
						{
							return candidate.strStageId == pStage->strStageId &&
								candidate.strActionId == pStage->strActionId;
						});
					if (pSavedPattern->Stages.end() != savedStage)
					{
						pSavedStage = &*savedStage;
						const std::size_t iSavedOccurrenceCount =
							static_cast<std::size_t>(std::count_if(
								pSavedStage->ProductCues.begin(),
								pSavedStage->ProductCues.end(),
								[Found](const VALTAN_PRODUCT_EFFECT_CUE_VIEW& cue)
								{
									return cue.strOccurrenceId ==
										Found->strOccurrenceId;
								}));
						const auto savedCue = std::find_if(
							pSavedStage->ProductCues.begin(),
							pSavedStage->ProductCues.end(),
							[Found](const VALTAN_PRODUCT_EFFECT_CUE_VIEW& cue)
							{
								return cue.strOccurrenceId ==
									Found->strOccurrenceId;
							});
						if (1u == iSavedOccurrenceCount &&
							pSavedStage->ProductCues.end() != savedCue &&
							SameSavedValtanEffectCue(*Found, *savedCue))
						{
							pSavedCue = &*savedCue;
						}
					}
				}
			}
			const bool_t bEffectBodyDeepLinkAdmitted =
				nullptr != pSavedPattern && nullptr != pSavedStage &&
				nullptr != pSavedCue;
			ImGui::BeginDisabled(!bEffectBodyDeepLinkAdmitted);
			if (ImGui::Button(bEffectBodyDeepLinkAdmitted ?
					"Open Saved Product Effect Body" :
					"Save + Reload Before Opening Effect Body") &&
				bEffectBodyDeepLinkAdmitted)
			{
				Request_EffectOwner(*pSavedPattern, *pSavedStage, *pSavedCue);
			}
			ImGui::EndDisabled();
			if (!bEffectBodyDeepLinkAdmitted)
			{
				ImGui::TextDisabled(
					"Effect body deep-link requires this exact invocation to match one occurrence in the admitted saved Product. Use Save & Apply, then reload; draft-only Add/Update rows cannot open an asset-only fallback.");
			}
		}

		ImGui::SeparatorText("Add Invocation");
		if (m_strEffectAddAssetId.empty() &&
			!m_SemanticValtanEffectAssetIds.empty())
		{
			m_strEffectAddAssetId = m_SemanticValtanEffectAssetIds.front();
		}
		if (m_strEffectAddClipOccurrenceId.empty())
		{
			m_strEffectAddClipOccurrenceId =
				pStage->ClipOccurrences.front().strClipOccurrenceId;
		}
		RenderEffectAssetPicker("New Effect asset", m_strEffectAddAssetId);
		(void)RenderClipOccurrencePicker(
			"New clip occurrence", m_strEffectAddClipOccurrenceId);
		ImGui::BeginDisabled(
			!bPatternMutationAdmitted || nullptr == m_pBalanceTool ||
			m_strEffectAddAssetId.empty() ||
			m_strEffectAddClipOccurrenceId.empty());
		if (ImGui::Button("Add Effect Invocation"))
		{
			const auto clip = std::find_if(
				pStage->ClipOccurrences.begin(), pStage->ClipOccurrences.end(),
				[this](const VALTAN_CLIP_OCCURRENCE_VIEW& candidate)
				{
					return candidate.strClipOccurrenceId ==
						m_strEffectAddClipOccurrenceId;
				});
			if (pStage->ClipOccurrences.end() != clip)
			{
				VALTAN_PRODUCT_EFFECT_CUE_VIEW Cue;
				Cue.strBindingId = BuildNextCompositionEffectCueId(
					*pPattern, *pStage);
				Cue.strOccurrenceId = Cue.strBindingId + ".occurrence.01";
				Cue.strPatternId = pPattern->strPatternId;
				Cue.strStageId = pStage->strStageId;
				Cue.strActionId = pStage->strActionId;
				Cue.strClipOccurrenceId = clip->strClipOccurrenceId;
				Cue.strEffectAssetId = m_strEffectAddAssetId;
				Cue.strAnchorSlotId = "root";
				Cue.eFollowPolicy = EFFECT_FOLLOW_POLICY::FOLLOW;
				Cue.strFollowPolicy = "follow";
				Cue.eStopPolicy = EFFECT_STOP_POLICY::NATURAL;
				Cue.strStopPolicy = "natural";
				Cue.strRepeatPolicy = "once";
				Cue.eScalePolicy =
					VALTAN_PATTERN_EFFECT_SCALE_POLICY::OWNER_RELATIVE;
				Cue.strScalePolicy = "OWNER_RELATIVE";
				Cue.bHasExplicitScalePolicy = true;
				Cue.iSourceStartMs = clip->iSourceStartMs;
				Cue.iStageDurationMs = pStage->iDurationMs;
				std::string Status;
				if (m_pBalanceTool->Add_ValtanStageEffectCue(
						pPattern->strPatternId, pStage->strStageId,
						pStage->strActionId, Cue, Status))
				{
					m_strStatus = std::move(Status);
					m_strSelectedStableId = Cue.strOccurrenceId;
					m_strEffectEditIdentity.clear();
					ImGui::EndDisabled();
					return;
				}
				m_strStatus = std::move(Status);
			}
		}
		ImGui::EndDisabled();

		const bool_t bHasServerCollider = pStage->Has_HitShape();
		const bool_t bManualColliderAddAdmitted =
			pPattern->bManualServerAudition &&
			"WAIT" != pStage->strSequenceRole;
		const char_t* const pColliderOwnerLabel = bHasServerCollider ?
			"Tune / Remove Existing Server Collider / Hit Schedule" :
			(bManualColliderAddAdmitted ?
				"Add Manual Audition Server Collider / Hit Schedule" :
				"View Collider Authority (New Add Unavailable)");
		if (ImGui::Button(pColliderOwnerLabel))
		{
			m_eDetailOwner = DETAIL_OWNER::GAMEPLAY_STAGE;
			m_strSelectedStableId = pStage->strStageId;
		}
		ImGui::TextDisabled(
			bHasServerCollider ?
				"Exact existing Stage hit only: tune/remove remains Server gameplay authority; Effect is visual." :
				(bManualColliderAddAdmitted ?
					"No Server collider exists. New Add is available only because this is a non-WAIT manual audition Stage; no geometry is inferred from the Effect." :
					"No Server collider exists. New Add is unavailable on this canonical/WAIT Stage; Workbench never infers hit geometry from a visual Effect or its anchor."));
		return;
	}
	if (DETAIL_OWNER::SOUND == m_eDetailOwner)
	{
		ImGui::SeparatorText("Pattern Sound Typed Source");
		ImGui::TextWrapped(
			"Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json");
		ImGui::TextDisabled(
			"Sound is a separate source-owner transaction. Save & Apply does not silently save Sound, and Save Sound Owner does not apply Pattern gameplay/presentation changes.");
		if (nullptr == pStage || nullptr == pPatternSounds ||
			nullptr == m_pAnimationTool)
		{
			ImGui::TextDisabled(
				"The strict Pattern Sound source draft is not admitted for this Stage.");
			if (!m_strSoundStatus.empty())
				ImGui::TextWrapped("Sound: %s", m_strSoundStatus.c_str());
			return;
		}
		const auto Found = std::find_if(
			pPatternSounds->Cues.begin(), pPatternSounds->Cues.end(),
			[this](const VALTAN_PATTERN_SOUND_CUE& Cue)
			{
				return Cue.strOccurrenceId == m_strSelectedStableId;
			});
		const bool_t bSelectedSoundExact =
			Found != pPatternSounds->Cues.end() &&
			Found->strPatternId == pPattern->strPatternId &&
			Found->strStageId == pStage->strStageId &&
			Found->strActionId == pStage->strActionId;

		const bool_t bFreshSoundOwnerAdmitted =
			ADMISSION_STATE::ADMITTED == m_eAdmission && bMutationAdmitted;
		const bool_t bSoundMutationAdmitted =
			bMutationAdmitted && !m_bAuthoringDraftDirty &&
			bFreshSoundOwnerAdmitted;
		std::string SoundLifecycleStatus;
		const bool_t bSoundSourceCommitAdmitted =
			bFreshSoundOwnerAdmitted &&
			m_pAnimationTool->
				Can_CommitValtanCompositionPatternSoundGeneration(
					SoundLifecycleStatus);
		const auto ApplySoundToExactRuntime =
			[this](std::string& strOutStatus) -> bool_t
			{
				if (ADMISSION_STATE::ADMITTED != m_eAdmission)
				{
					strOutStatus =
						"Pattern Sound runtime apply requires a fresh FULL_JOIN canonical admission; STALE_PRESERVED is display-only.";
					return false;
				}
				std::string RuntimeGateStatus;
				LostArk::Shared::GameplayDataRevision ExpectedRevision{};
				if (nullptr == m_pBossTool ||
					!m_pBossTool->Get_ServerActivePatternRevision(
						ExpectedRevision, RuntimeGateStatus))
				{
					strOutStatus =
						"Pattern Sound active-consumer apply is deferred until the exact immutable Pattern revision is Server-active. " +
						RuntimeGateStatus;
					return false;
				}
				if (!m_pAnimationTool->
					Retry_ValtanCompositionPatternSoundRuntimeApply(
						ExpectedRevision, strOutStatus))
				{
					return false;
				}

				LostArk::Shared::GameplayDataRevision PostRevision{};
				std::string PostRevisionStatus;
				const bool_t bPostRevisionAdmitted =
					m_pBossTool->Get_ServerActivePatternRevision(
						PostRevision, PostRevisionStatus);
				if (!bPostRevisionAdmitted ||
					PostRevision != ExpectedRevision)
				{
					strOutStatus =
						"Pattern Sound consumer reload lost its exact Server revision admission before commit. Expected " +
						LostArk::Shared::Format_GameplayDataRevision(
							ExpectedRevision) + ", observed " +
						LostArk::Shared::Format_GameplayDataRevision(
							PostRevision) + ". " +
						PostRevisionStatus;
					m_pAnimationTool->
						Invalidate_ValtanCompositionPatternSoundRuntimeApply(
							strOutStatus);
					return false;
				}
				strOutStatus +=
					" Exact Server revision revalidated after active consumer reload.";
				return true;
			};
		if (m_bAuthoringDraftDirty)
		{
			ImGui::TextColored(ImVec4(1.f, 0.72f, 0.2f, 1.f),
				"Use Save & Apply before editing its dependency-qualified Sound rows.");
		}
		if (bPatternSoundDirty)
		{
			ImGui::TextColored(ImVec4(1.f, 0.72f, 0.2f, 1.f),
				"UNSAVED SOUND OWNER DRAFT");
		}
		if (!bSoundSourceCommitAdmitted && !SoundLifecycleStatus.empty())
		{
			ImGui::TextColored(ImVec4(1.f, 0.58f, 0.25f, 1.f),
				"LIVE OCCURRENCE PIN: %s", SoundLifecycleStatus.c_str());
		}

		if (!bPatternSoundDirty)
			m_bConfirmDiscardPatternSoundDraft = false;
		ImGui::BeginDisabled(!bSoundSourceCommitAdmitted);
		if (bSoundSourceCommitAdmitted && ImGui::Button(bPatternSoundDirty ?
				"Review Discard Sound Draft" : "Reload Sound Owner"))
		{
			if (bPatternSoundDirty)
			{
				m_bConfirmDiscardPatternSoundDraft = true;
			}
			else
			{
				std::string Status;
				const bool_t bReloaded = m_pAnimationTool->
					Reload_ValtanCompositionPatternSounds(Status);
				if (bReloaded && ApplySoundToExactRuntime(Status))
				{
					m_PatternSoundEvents =
						m_pAnimationTool->Collect_ValtanCompositionPatternSoundEvents();
					m_bSoundFilterDirty = true;
				}
				m_strSoundStatus = std::move(Status);
				Invalidate_TimelineCache();
				ImGui::EndDisabled();
				return;
			}
		}
		ImGui::EndDisabled();
		if (bPatternSoundDirty && m_bConfirmDiscardPatternSoundDraft)
		{
			ImGui::TextColored(
				ImVec4(1.f, 0.45f, 0.25f, 1.f),
				"This permanently discards the unsaved Sound owner draft only.");
			ImGui::BeginDisabled(!bSoundSourceCommitAdmitted);
			if (bSoundSourceCommitAdmitted &&
				ImGui::Button("Confirm Discard + Reload Sound Owner"))
			{
				std::string Status;
				const bool_t bReloaded =
					m_pAnimationTool->Reload_ValtanCompositionPatternSounds(
						Status);
				if (bReloaded && ApplySoundToExactRuntime(Status))
				{
					m_PatternSoundEvents =
						m_pAnimationTool->Collect_ValtanCompositionPatternSoundEvents();
					m_bSoundFilterDirty = true;
				}
				m_bConfirmDiscardPatternSoundDraft = false;
				m_strSoundStatus = std::move(Status);
				Invalidate_TimelineCache();
				ImGui::EndDisabled();
				return;
			}
			ImGui::EndDisabled();
			ImGui::SameLine();
			if (ImGui::Button("Cancel Discard"))
				m_bConfirmDiscardPatternSoundDraft = false;
		}
		ImGui::SameLine();
		ImGui::BeginDisabled(!bSoundSourceCommitAdmitted ||
			!bSoundMutationAdmitted || !bPatternSoundDirty);
		bool_t bSoundSaveRequested = false;
		if (ImGui::Button("Save Sound Owner"))
		{
			std::string Status;
			(void)m_pAnimationTool->Save_ValtanCompositionPatternSounds(Status);
			m_strSoundStatus = std::move(Status);
			Invalidate_TimelineCache();
			bSoundSaveRequested = true;
		}
		ImGui::EndDisabled();
		if (bSoundSaveRequested)
			return;
		std::string SoundRuntimeStatus;
		std::string RuntimeApplyGateStatus;
		LostArk::Shared::GameplayDataRevision ExpectedRuntimeRevision{};
		const bool_t bRuntimeApplyAdmitted = nullptr != m_pBossTool &&
			m_pBossTool->Observe_ServerActivePatternRevision(
				ExpectedRuntimeRevision, RuntimeApplyGateStatus);
		const bool_t bSoundRuntimeReady = bRuntimeApplyAdmitted &&
			m_pAnimationTool->Is_ValtanCompositionPatternSoundRuntimeReady(
				ExpectedRuntimeRevision, SoundRuntimeStatus);
		if (!bSoundRuntimeReady)
		{
			ImGui::BeginDisabled(
				!bSoundSourceCommitAdmitted || !bRuntimeApplyAdmitted);
			if (bSoundSourceCommitAdmitted && bRuntimeApplyAdmitted &&
				ImGui::Button("Retry Apply Saved Sound"))
			{
				(void)ApplySoundToExactRuntime(m_strSoundStatus);
				Invalidate_TimelineCache();
				ImGui::EndDisabled();
				return;
			}
			ImGui::EndDisabled();
			ImGui::TextWrapped("Runtime apply pending: %s",
				SoundRuntimeStatus.c_str());
			if (!bRuntimeApplyAdmitted && !RuntimeApplyGateStatus.empty())
			{
				ImGui::TextDisabled(
					"Apply is deferred until exact Server revision admission: %s",
					RuntimeApplyGateStatus.c_str());
			}
		}

		ImGui::SeparatorText("Add Sound Row");
		auto AddClip = std::find_if(
			pStage->ClipOccurrences.begin(), pStage->ClipOccurrences.end(),
			[this](const VALTAN_CLIP_OCCURRENCE_VIEW& Clip)
			{
				return Clip.strClipOccurrenceId ==
					m_strSoundAddClipOccurrenceId;
			});
		if (AddClip == pStage->ClipOccurrences.end() &&
			!pStage->ClipOccurrences.empty())
		{
			AddClip = pStage->ClipOccurrences.begin();
			m_strSoundAddClipOccurrenceId = AddClip->strClipOccurrenceId;
			m_iSoundAddStartMs = AddClip->iSourceStartMs;
			m_eSoundAddRepeatPolicy =
				VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE;
		}
		if (m_PatternSoundEvents.end() == std::find(
				m_PatternSoundEvents.begin(), m_PatternSoundEvents.end(),
				m_strSoundAddEvent) && !m_PatternSoundEvents.empty())
		{
			m_strSoundAddEvent = m_PatternSoundEvents.front();
		}

		ImGui::BeginDisabled(!bSoundMutationAdmitted);
		ImGui::SetNextItemWidth(-1.f);
		const char_t* const pAddClipLabel =
			AddClip == pStage->ClipOccurrences.end() ?
				"Select clip occurrence" : AddClip->strClipOccurrenceId.c_str();
		if (ImGui::BeginCombo("Clip Occurrence##AddSoundRow", pAddClipLabel))
		{
			for (const VALTAN_CLIP_OCCURRENCE_VIEW& Clip :
				pStage->ClipOccurrences)
			{
				const bool_t bSelected = Clip.strClipOccurrenceId ==
					m_strSoundAddClipOccurrenceId;
				if (ImGui::Selectable(
						Clip.strClipOccurrenceId.c_str(), bSelected) && !bSelected)
				{
					m_strSoundAddClipOccurrenceId = Clip.strClipOccurrenceId;
					m_iSoundAddStartMs = Clip.iSourceStartMs;
					m_eSoundAddRepeatPolicy =
						VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE;
				}
				if (bSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		AddClip = std::find_if(
			pStage->ClipOccurrences.begin(), pStage->ClipOccurrences.end(),
			[this](const VALTAN_CLIP_OCCURRENCE_VIEW& Clip)
			{
				return Clip.strClipOccurrenceId ==
					m_strSoundAddClipOccurrenceId;
			});

		ImGui::SetNextItemWidth(-1.f);
		const char_t* const pAddEventLabel = m_strSoundAddEvent.empty() ?
			"Select admitted Valtan event" : m_strSoundAddEvent.c_str();
		if (ImGui::BeginCombo("Sound Event##AddSoundRow", pAddEventLabel))
		{
			for (const std::string& Event : m_PatternSoundEvents)
			{
				const bool_t bSelected = Event == m_strSoundAddEvent;
				if (ImGui::Selectable(Event.c_str(), bSelected) && !bSelected)
					m_strSoundAddEvent = Event;
				if (bSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		uint32_t iAddMinimumStartMs = 0u;
		uint32_t iAddMaximumStartMs = 0u;
		bool_t bAddLoopClip = false;
		std::string AddWindowStatus;
		const bool_t bAddWindowReady =
			AddClip != pStage->ClipOccurrences.end() &&
			m_pAnimationTool->Resolve_ValtanCompositionPatternSoundWindow(
				*pStage, AddClip->strClipOccurrenceId,
				iAddMinimumStartMs, iAddMaximumStartMs,
				bAddLoopClip, AddWindowStatus);
		if (bAddWindowReady)
		{
			m_iSoundAddStartMs = std::clamp(
				m_iSoundAddStartMs, iAddMinimumStartMs, iAddMaximumStartMs);
			if (!bAddLoopClip &&
				VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP ==
					m_eSoundAddRepeatPolicy)
			{
				m_eSoundAddRepeatPolicy =
					VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE;
			}
		}
		ImGui::BeginDisabled(!bAddWindowReady);
		const uint32_t iAddStepMs = 1u;
		const uint32_t iAddFastStepMs = 100u;
		ImGui::SetNextItemWidth(210.f);
		if (ImGui::InputScalar("Source startMs##AddSoundRow",
				ImGuiDataType_U32, &m_iSoundAddStartMs,
				&iAddStepMs, &iAddFastStepMs, "%u"))
		{
			m_iSoundAddStartMs = std::clamp(
				m_iSoundAddStartMs, iAddMinimumStartMs, iAddMaximumStartMs);
		}
		ImGui::EndDisabled();

		const char_t* const pAddRepeatLabel =
			VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP ==
				m_eSoundAddRepeatPolicy ? "each_loop" : "once";
		ImGui::SetNextItemWidth(210.f);
		if (ImGui::BeginCombo(
				"Repeat Policy##AddSoundRow", pAddRepeatLabel))
		{
			const bool_t bOnce = VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE ==
				m_eSoundAddRepeatPolicy;
			if (ImGui::Selectable("once", bOnce) && !bOnce)
			{
				m_eSoundAddRepeatPolicy =
					VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE;
			}
			ImGui::BeginDisabled(!bAddLoopClip);
			const bool_t bEachLoop =
				VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP ==
					m_eSoundAddRepeatPolicy;
			if (ImGui::Selectable("each_loop", bEachLoop) && !bEachLoop)
			{
				m_eSoundAddRepeatPolicy =
					VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP;
			}
			ImGui::EndDisabled();
			ImGui::EndCombo();
		}
		const bool_t bCanAddSoundRow = bAddWindowReady &&
			!m_strSoundAddEvent.empty();
		ImGui::BeginDisabled(!bCanAddSoundRow);
		if (ImGui::Button("Add Sound Row"))
		{
			VALTAN_PATTERN_SOUND_CUE_ROW_ID CreatedRowId;
			std::string Status;
			if (m_pAnimationTool->Add_ValtanCompositionPatternSound(
					*pPattern, *pStage, m_strSoundAddClipOccurrenceId,
					m_strSoundAddEvent, m_iSoundAddStartMs,
					m_eSoundAddRepeatPolicy, CreatedRowId, Status))
			{
				m_strSelectedStableId = CreatedRowId.strOccurrenceId;
			}
			m_strSoundStatus = std::move(Status);
			Invalidate_TimelineCache();
			ImGui::EndDisabled();
			ImGui::EndDisabled();
			return;
		}
		ImGui::EndDisabled();
		ImGui::EndDisabled();
		if (!bAddWindowReady && !AddWindowStatus.empty())
		{
			ImGui::TextColored(ImVec4(1.f, 0.35f, 0.25f, 1.f),
				"%s", AddWindowStatus.c_str());
		}

		ImGui::SeparatorText("Selected Sound Row");
		if (!bSelectedSoundExact)
		{
			ImGui::TextDisabled(
				"Select an existing Sound timeline row or add one for this exact Pattern/Stage/clip occurrence.");
			if (!m_strSoundStatus.empty())
				ImGui::TextWrapped("Sound: %s", m_strSoundStatus.c_str());
			return;
		}
		ImGui::BeginDisabled(!bSoundMutationAdmitted);
		if (ImGui::Button("Remove Selected Sound Row"))
		{
			VALTAN_PATTERN_SOUND_CUE_ROW_ID RowId;
			RowId.strBindingId = Found->strBindingId;
			RowId.strOccurrenceId = Found->strOccurrenceId;
			std::string Status;
			if (m_pAnimationTool->Remove_ValtanCompositionPatternSound(
					*pPattern, *pStage, RowId, Status))
			{
				m_strSelectedStableId.clear();
			}
			m_strSoundStatus = std::move(Status);
			Invalidate_TimelineCache();
			ImGui::EndDisabled();
			return;
		}
		ImGui::EndDisabled();

		ImGui::TextDisabled("bindingId: %s", Found->strBindingId.c_str());
		ImGui::TextDisabled("occurrenceId: %s", Found->strOccurrenceId.c_str());
		ImGui::TextDisabled("clipOccurrenceId: %s",
			Found->strClipOccurrenceId.c_str());
		ImGui::TextDisabled("Sound Bank (derived): %s",
			Found->strSoundBank.c_str());

		std::string CandidateEvent = Found->strSoundEvent;
		uint32_t iCandidateStartMs = Found->iStartMs;
		VALTAN_PATTERN_SOUND_REPEAT_POLICY eCandidateRepeat =
			Found->eRepeatPolicy;
		bool_t bSubmitPatch = false;
		ImGui::BeginDisabled(!bSoundMutationAdmitted);
		ImGui::SetNextItemWidth(-1.f);
		if (ImGui::BeginCombo("Sound Event", CandidateEvent.c_str()))
		{
			for (const std::string& Event : m_PatternSoundEvents)
			{
				const bool_t bSelected = Event == CandidateEvent;
				if (ImGui::Selectable(Event.c_str(), bSelected) && !bSelected)
				{
					CandidateEvent = Event;
					bSubmitPatch = true;
				}
				if (bSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		uint32_t iMinimumStartMs = 0u;
		uint32_t iMaximumStartMs = 0u;
		bool_t bLoopClip = false;
		std::string WindowStatus;
		const bool_t bWindowReady =
			m_pAnimationTool->Resolve_ValtanCompositionPatternSoundWindow(
				*pStage, Found->strClipOccurrenceId,
				iMinimumStartMs, iMaximumStartMs, bLoopClip, WindowStatus);
		ImGui::BeginDisabled(!bWindowReady);
		const uint32_t iStepMs = 1u;
		const uint32_t iFastStepMs = 100u;
		ImGui::SetNextItemWidth(210.f);
		if (ImGui::InputScalar("Source startMs", ImGuiDataType_U32,
				&iCandidateStartMs, &iStepMs, &iFastStepMs, "%u"))
		{
			iCandidateStartMs = std::clamp(
				iCandidateStartMs, iMinimumStartMs, iMaximumStartMs);
			bSubmitPatch = true;
		}
		ImGui::EndDisabled();
		if (bWindowReady)
		{
			ImGui::TextDisabled("Admitted source window: %u..%u ms | loop=%s",
				iMinimumStartMs, iMaximumStartMs,
				bLoopClip ? "true" : "false");
		}
		else
		{
			ImGui::TextColored(ImVec4(1.f, 0.35f, 0.25f, 1.f),
				"%s", WindowStatus.c_str());
		}

		const char_t* const pRepeatLabel =
			VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP ==
				eCandidateRepeat ? "each_loop" : "once";
		ImGui::SetNextItemWidth(210.f);
		if (ImGui::BeginCombo("Repeat Policy", pRepeatLabel))
		{
			const bool_t bOnce =
				VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE == eCandidateRepeat;
			if (ImGui::Selectable("once", bOnce) && !bOnce)
			{
				eCandidateRepeat = VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE;
				bSubmitPatch = true;
			}
			ImGui::BeginDisabled(!bLoopClip);
			const bool_t bEachLoop =
				VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP ==
					eCandidateRepeat;
			if (ImGui::Selectable("each_loop", bEachLoop) && !bEachLoop)
			{
				eCandidateRepeat =
					VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP;
				bSubmitPatch = true;
			}
			ImGui::EndDisabled();
			ImGui::EndCombo();
		}
		ImGui::EndDisabled();

		if (bSubmitPatch)
		{
			std::string Status;
			(void)m_pAnimationTool->Patch_ValtanCompositionPatternSound(
				*pPattern, *pStage, Found->strOccurrenceId,
				CandidateEvent, iCandidateStartMs,
				eCandidateRepeat, Status);
			m_strSoundStatus = std::move(Status);
			Invalidate_TimelineCache();
			return;
		}
		if (!m_strSoundStatus.empty())
			ImGui::TextWrapped("Sound: %s", m_strSoundStatus.c_str());
		ImGui::TextDisabled(
			"Timeline playback remains inspection-only for Sound because the engine has no seek-safe per-cue stop handle; source authoring and Save are fully typed here.");
		return;
	}
	if (DETAIL_OWNER::CAMERA == m_eDetailOwner)
	{
		const auto Found = std::find_if(
			pStage->CameraInvocations.begin(), pStage->CameraInvocations.end(),
			[this](const VALTAN_CAMERA_INVOCATION_VIEW& Camera)
			{
				return Camera.strCameraInvocationId == m_strSelectedStableId;
			});
		if (Found != pStage->CameraInvocations.end())
		{
			ImGui::TextWrapped("%s", Found->strCameraCueId.c_str());
			ImGui::Text("Start %u ms | Duration %u ms | %s",
				Found->iStartOffsetMs, Found->iDurationMs,
				Found->strDurationPolicy.c_str());
			if (ImGui::Button("Open Camera Sequence Detail"))
			{
				m_strCameraCueId = Found->strCameraCueId;
				m_bCameraToolOpenRequested = true;
			}
		}
		else
		{
			ImGui::TextDisabled("Selected Camera row is a shake cue; open Animation source detail for its payload.");
		}
		return;
	}
	if (DETAIL_OWNER::WORLD == m_eDetailOwner)
	{
		ImGui::TextWrapped("World event: %s", m_strSelectedStableId.c_str());
		ImGui::TextDisabled(
			"World/Light rows remain Server/Map/Camera-owner data. The Sequencer never writes a duplicate sequence Product.");
		return;
	}
	if (DETAIL_OWNER::COMBAT_OBJECT == m_eDetailOwner)
	{
		const auto Found = std::find_if(
			pStage->CombatObjectEffects.begin(), pStage->CombatObjectEffects.end(),
			[this](const VALTAN_COMBAT_OBJECT_EFFECT_VIEW& Object)
			{
				return Object.strCombatObjectArchetypeId == m_strSelectedStableId;
			});
		if (Found != pStage->CombatObjectEffects.end())
		{
			ImGui::TextWrapped("%s", Found->strCombatObjectArchetypeId.c_str());
			ImGui::Text("Effect: %s", Found->strEffectAssetId.c_str());
			ImGui::Text("Motion %s | %.2f m/s | %.2f m | life %u ms",
				Found->strKind.c_str(), Found->fSpeedMps,
				Found->fMaximumDistanceM, Found->iLifetimeMs);
			ImGui::TextDisabled(
				"Collider and hit clocks are Server combat-object owner data; Effect visibility does not become hit authority.");
		}
	}
}

bool_t Client::CActionCompositionWorkbench::Validate_TimelineDependencyWindows(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage,
	const VALTAN_PRODUCT_EFFECT_CUE_VIEW* const pEffectOverride,
	const VALTAN_PATTERN_SOUND_CUE* const pSoundOverride,
	std::string& strOutStatus) const
{
	if (ADMISSION_STATE::ADMITTED != m_eAdmission ||
		nullptr == m_pAnimationTool)
	{
		strOutStatus =
			"Sequencer timing edit requires a fresh FULL_JOIN admission; stale-preserved rows are display-only.";
		return false;
	}
	if (!m_bPatternShakesReady)
	{
		strOutStatus =
			"Sequencer timing edit is blocked because the Pattern Shake dependency source is not admitted: " +
			m_strShakeStatus;
		return false;
	}

	bool_t bPatternSoundDirty = false;
	std::string PatternSoundStatus;
	const VALTAN_PATTERN_SOUND_CUE_DOCUMENT* const pPatternSounds =
		m_pAnimationTool->Get_ValtanCompositionPatternSoundDraft(
			bPatternSoundDirty, PatternSoundStatus);
	if (nullptr == pPatternSounds)
	{
		strOutStatus =
			"Sequencer timing edit is blocked because the Pattern Sound dependency owner is not admitted: " +
			PatternSoundStatus;
		return false;
	}

	std::size_t iEffectOverrideMatches = 0u;
	for (const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Existing : Stage.ProductCues)
	{
		const VALTAN_PRODUCT_EFFECT_CUE_VIEW* pCandidate = &Existing;
		if (nullptr != pEffectOverride &&
			Existing.strBindingId == pEffectOverride->strBindingId &&
			Existing.strOccurrenceId == pEffectOverride->strOccurrenceId)
		{
			pCandidate = pEffectOverride;
			++iEffectOverrideMatches;
		}
		if (pCandidate->strPatternId != Pattern.strPatternId ||
			pCandidate->strStageId != Stage.strStageId ||
			pCandidate->strActionId != Stage.strActionId)
		{
			strOutStatus =
				"Effect dependency no longer resolves the exact Pattern/Stage/action tuple: " +
				pCandidate->strOccurrenceId + ".";
			return false;
		}
		if (pCandidate->bUsesStageClock)
			continue;
		if (!ValidateClipSourceDependencyWindow(
				Stage, pCandidate->strClipOccurrenceId,
				pCandidate->iSourceStartMs, pCandidate->bHasSourceEnd,
				pCandidate->iSourceEndMs,
				"each_loop" == pCandidate->strRepeatPolicy,
				"Effect occurrence " + pCandidate->strOccurrenceId,
				strOutStatus))
		{
			return false;
		}
	}
	if (nullptr != pEffectOverride && 1u != iEffectOverrideMatches)
	{
		strOutStatus =
			"Sequencer Effect edit lost its exact bindingId/occurrenceId predecessor.";
		return false;
	}

	std::size_t iSoundOverrideMatches = 0u;
	for (const VALTAN_PATTERN_SOUND_CUE& Existing : pPatternSounds->Cues)
	{
		if (Existing.strPatternId != Pattern.strPatternId ||
			Existing.strStageId != Stage.strStageId)
		{
			continue;
		}
		const VALTAN_PATTERN_SOUND_CUE* pCandidate = &Existing;
		if (nullptr != pSoundOverride &&
			Existing.strBindingId == pSoundOverride->strBindingId &&
			Existing.strOccurrenceId == pSoundOverride->strOccurrenceId)
		{
			pCandidate = pSoundOverride;
			++iSoundOverrideMatches;
		}
		if (pCandidate->strActionId != Stage.strActionId ||
			!ValidateClipSourceDependencyWindow(
				Stage, pCandidate->strClipOccurrenceId,
				pCandidate->iStartMs, false, 0u,
				VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP ==
					pCandidate->eRepeatPolicy,
				"Pattern Sound occurrence " + pCandidate->strOccurrenceId,
				strOutStatus))
		{
			if (strOutStatus.empty())
			{
				strOutStatus =
					"Pattern Sound dependency no longer resolves the exact Pattern/Stage/action tuple: " +
					pCandidate->strOccurrenceId + ".";
			}
			return false;
		}
	}
	if (nullptr != pSoundOverride && 1u != iSoundOverrideMatches)
	{
		strOutStatus =
			"Sequencer Sound edit lost its exact bindingId/occurrenceId predecessor.";
		return false;
	}

	for (const VALTAN_PATTERN_SHAKE_CUE& Cue : m_PatternShakes.Cues)
	{
		if (Cue.strPatternId != Pattern.strPatternId ||
			Cue.strStageId != Stage.strStageId)
		{
			continue;
		}
		if (Cue.strActionId != Stage.strActionId ||
			!ValidateClipSourceDependencyWindow(
				Stage, Cue.strClipOccurrenceId, Cue.iStartMs, false, 0u,
				VALTAN_PATTERN_SHAKE_REPEAT_POLICY::EACH_LOOP ==
					Cue.eRepeatPolicy,
				"Pattern Shake occurrence " + Cue.strOccurrenceId,
				strOutStatus))
		{
			if (strOutStatus.empty())
			{
				strOutStatus =
					"Pattern Shake dependency no longer resolves the exact Pattern/Stage/action tuple: " +
					Cue.strOccurrenceId + ".";
			}
			return false;
		}
	}

	strOutStatus = bPatternSoundDirty ?
		"Validated Effect, unsaved Pattern Sound draft, and Pattern Shake windows." :
		"Validated Effect, Pattern Sound, and Pattern Shake dependency windows.";
	return true;
}

bool_t Client::CActionCompositionWorkbench::Apply_EffectOccurrenceTiming(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage,
	const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Current,
	const uint32_t iSourceStartMs,
	const uint32_t iSourceEndMs,
	std::string& strOutStatus)
{
	if (ADMISSION_STATE::ADMITTED != m_eAdmission ||
		nullptr == m_pBalanceTool || Current.bUsesStageClock)
	{
		strOutStatus =
			"Sequencer Effect timing requires one FULL_JOIN admitted, clip-qualified source occurrence.";
		return false;
	}
	std::string PatternSoundDraftStatus;
	if (Is_PatternSoundDraftDirty(PatternSoundDraftStatus))
	{
		strOutStatus =
			"Sequencer Effect timing is blocked until the independent Pattern Sound owner draft is saved or discarded: " +
			PatternSoundDraftStatus;
		return false;
	}
	VALTAN_PRODUCT_EFFECT_CUE_VIEW Candidate = Current;
	Candidate.iSourceStartMs = iSourceStartMs;
	Candidate.iSourceEndMs = Candidate.bHasSourceEnd ? iSourceEndMs : 0u;
	if (Candidate.iSourceStartMs == Current.iSourceStartMs &&
		Candidate.iSourceEndMs == Current.iSourceEndMs)
	{
		strOutStatus = "Sequencer Effect timing draft is unchanged.";
		return true;
	}
	if (!Validate_TimelineDependencyWindows(
			Pattern, Stage, &Candidate, nullptr, strOutStatus))
	{
		strOutStatus =
			"Sequencer Effect timing rejected before the typed draft changed: " +
			strOutStatus;
		return false;
	}
	if (!m_pBalanceTool->Update_ValtanStageEffectCue(
			Pattern.strPatternId, Stage.strStageId, Stage.strActionId,
			Current.strBindingId, Current.strOccurrenceId,
			Candidate, strOutStatus))
	{
		return false;
	}
	m_strEffectEditIdentity.clear();
	strOutStatus +=
		" Timeline edit remains unsaved; use Save & Apply when the timing is ready.";
	return true;
}

bool_t Client::CActionCompositionWorkbench::
Apply_PatternSoundOccurrenceTiming(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage,
	const VALTAN_PATTERN_SOUND_CUE& Current,
	const uint32_t iSourceStartMs,
	std::string& strOutStatus)
{
	if (ADMISSION_STATE::ADMITTED != m_eAdmission ||
		nullptr == m_pAnimationTool || m_bAuthoringDraftDirty)
	{
		strOutStatus =
			"Sequencer Sound timing requires a FULL_JOIN admission and a clean Pattern/Animation owner generation.";
		return false;
	}
	VALTAN_PATTERN_SOUND_CUE Candidate = Current;
	Candidate.iStartMs = iSourceStartMs;
	if (Candidate.iStartMs == Current.iStartMs)
	{
		strOutStatus = "Sequencer Pattern Sound timing draft is unchanged.";
		return true;
	}
	if (!Validate_TimelineDependencyWindows(
			Pattern, Stage, nullptr, &Candidate, strOutStatus))
	{
		strOutStatus =
			"Sequencer Sound timing rejected before the typed owner draft changed: " +
			strOutStatus;
		return false;
	}
	if (!m_pAnimationTool->Patch_ValtanCompositionPatternSound(
			Pattern, Stage, Current.strOccurrenceId, Current.strSoundEvent,
			iSourceStartMs, Current.eRepeatPolicy, strOutStatus))
	{
		return false;
	}
	strOutStatus +=
		" Timeline edit remains an unsaved Pattern Sound owner draft; Save/Apply was not invoked.";
	return true;
}

void Client::CActionCompositionWorkbench::Render_PatternDurationControl(
	const VALTAN_PATTERN_VIEW& Pattern,
	const bool_t bPatternMutationAdmitted)
{
	int32_t iRequestedDurationMs = static_cast<int32_t>((std::min)(
		m_iTimelineDurationMs, static_cast<uint32_t>(INT32_MAX)));
	ImGui::SetNextItemWidth(220.f);
	ImGui::BeginDisabled(!bPatternMutationAdmitted ||
		!Pattern.bManualServerAudition || Pattern.Stages.empty() ||
		nullptr == m_pBalanceTool);
	(void)ImGui::InputInt(
		"Pattern Total Duration (ms)", &iRequestedDurationMs, 10, 100);
	const bool_t bDurationEditCommitted = ImGui::IsItemDeactivatedAfterEdit();
	if (bDurationEditCommitted)
	{
		const uint32_t iRequested = static_cast<uint32_t>((std::clamp)(
			iRequestedDurationMs, 1, 600000));
		if (iRequested == m_iTimelineDurationMs)
		{
			m_strStatus = "Pattern duration is unchanged.";
		}
		else
		{
			const VALTAN_STAGE_VIEW& LastStage = Pattern.Stages.back();
			const bool_t bTrailingWait =
				"WAIT" == LastStage.strSequenceRole;
			if (bTrailingWait)
			{
				CBalanceTool::PATTERN_STAGE_EDIT Draft;
				std::string Status;
				if (m_pBalanceTool->Get_ValtanStageDraft(
						Pattern.strPatternId, LastStage.strStageId,
						Draft, Status))
				{
					const uint32_t iPrefixMs = m_iTimelineDurationMs >
						Draft.durationMs ?
						m_iTimelineDurationMs - Draft.durationMs : 0u;
					if (iRequested <= iPrefixMs)
					{
						Status =
							"Pattern duration rejected: the requested total would trim a gameplay Stage. Reduce or edit that Stage explicitly.";
					}
					else
					{
						Draft.durationMs = iRequested - iPrefixMs;
						if (SetValtanStageDraftWithSoundDependencyAdmission(
								m_pAnimationTool, m_pBalanceTool,
								m_bPatternShakesReady ? &m_PatternShakes : nullptr,
								Pattern, LastStage, Draft, Status))
						{
							Invalidate_TimelineCache();
						}
					}
				}
				m_strStatus = std::move(Status);
			}
			else if (iRequested > m_iTimelineDurationMs)
			{
				std::string NewStageId;
				std::string NewActionId;
				std::string Status;
				if (!BuildNextManualStageIdentity(
						Pattern, NewStageId, NewActionId))
				{
					Status =
						"Pattern duration rejected: no free stable Stage/Action identity remains for the trailing WAIT.";
				}
				else if (m_pBalanceTool->Insert_ValtanManualStageAfter(
						Pattern.strPatternId, LastStage.strStageId,
						NewStageId, NewActionId, "WAIT",
						iRequested - m_iTimelineDurationMs, Status))
				{
					m_strSelectedStageId = NewStageId;
					m_strSelectedStableId = NewStageId;
					m_eDetailOwner = DETAIL_OWNER::GAMEPLAY_STAGE;
					m_strTimelineCachePatternId.clear();
				}
				m_strStatus = std::move(Status);
			}
			else
			{
				m_strStatus =
					"Pattern duration rejected: shortening requires a trailing WAIT Stage so gameplay clocks are never trimmed implicitly.";
			}
		}
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::TextDisabled(
		"Stage clock sum; growth creates/extends trailing WAIT, shrink trims WAIT only.");
}

void Client::CActionCompositionWorkbench::Render_SelectedStageGapControl(
	const VALTAN_PATTERN_VIEW& Pattern,
	const bool_t bPatternMutationAdmitted)
{
	const VALTAN_STAGE_VIEW* const pStage = Find_SelectedStage(&Pattern);
	if (nullptr == pStage || nullptr == m_pBalanceTool ||
		ADMISSION_STATE::ADMITTED != m_eAdmission)
	{
		return;
	}

	CBalanceTool::PATTERN_STAGE_EDIT Draft;
	std::string Status;
	uint32_t iAnimationWallMs = 0u;
	if (!m_pBalanceTool->Get_ValtanStageDraft(
			Pattern.strPatternId, pStage->strStageId, Draft, Status) ||
		!Draft.durationEditable || !Draft.animationEditable ||
		"LOOP_TO_STAGE_END" == Draft.animationEndPolicy ||
		!ComputeExactAnimationWallMs(Draft, iAnimationWallMs))
	{
		ImGui::TextDisabled(
			"Selected Stage gap is unavailable for this Stage clock/Animation policy.");
		return;
	}

	int32_t iGapMs = static_cast<int32_t>(
		Draft.durationMs > iAnimationWallMs ?
			Draft.durationMs - iAnimationWallMs : 0u);
	ImGui::PushID("##SelectedStageSequencerGap");
	ImGui::SetNextItemWidth(220.f);
	ImGui::BeginDisabled(!bPatternMutationAdmitted);
	(void)ImGui::InputInt(
		"Selected Stage Gap (ms)", &iGapMs, 10, 100);
	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		iGapMs = (std::clamp)(iGapMs, 0, 120000);
		const uint64_t iDurationMs =
			static_cast<uint64_t>(iAnimationWallMs) +
			static_cast<uint32_t>(iGapMs);
		if (iDurationMs <= 600000u)
		{
			Draft.durationMs = static_cast<uint32_t>(iDurationMs);
			Draft.animationEndPolicy = 0 == iGapMs ?
				"EXACT" : "HOLD_LAST_POSE";
			if (SetValtanStageDraftWithSoundDependencyAdmission(
					m_pAnimationTool, m_pBalanceTool,
					m_bPatternShakesReady ? &m_PatternShakes : nullptr,
					Pattern, *pStage, Draft, Status))
			{
				Invalidate_TimelineCache();
			}
			m_strStatus = std::move(Status);
		}
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::TextDisabled(
		"%s | Animation %u ms -> Stage %u ms | Save & Apply when ready",
		pStage->strStageId.c_str(), iAnimationWallMs, Draft.durationMs);
	ImGui::PopID();
}

void Client::CActionCompositionWorkbench::Render_Timeline(
	const VALTAN_PATTERN_VIEW* const pPattern,
	const bool_t bMutationAdmitted,
	const bool_t bPatternMutationAdmitted)
{
	ImGui::SeparatorText("Sequencer");
	ImGui::TextDisabled(
		"One Server Pattern clock. Drag an Effect/Sound body to move its exact occurrence; Effect right trim is available only for once + cue_end. Stage/Animation right trims keep their existing typed clocks.");
	ImGui::TextDisabled(
		"LIVE: Animation + V1/V2 Effect + collider mirror on the Arena Clone. Logic/Stage are clock rows; Sound/Camera/World remain inspection-only during local seek.");
	ImGui::TextDisabled(
		"Effect timing and Sound timing remain unsaved drafts until Save & Apply; Camera/World are inspection only.");
	CAnimation_Tool::COMPOSITION_PREVIEW_STATE Preview;
	if (nullptr != m_pAnimationTool)
	{
		m_pAnimationTool->Set_ValtanCompositionLoop(m_bLoopPreview);
		Preview = m_pAnimationTool->Get_ValtanCompositionPreviewState();
	}
	if (Preview.bPlaying && nullptr != pPattern &&
		Preview.strPatternId == pPattern->strPatternId &&
		m_eStagedPreviewPath == m_ePreviewPath)
	{
		m_iPlayheadMs = Preview.iPositionMs;
	}
	ImGui::BeginDisabled(nullptr == m_pAnimationTool || nullptr == pPattern ||
		!bMutationAdmitted);
	if (ImGui::Button(Preview.bPlaying && !Preview.bPaused ? "Pause" : "Play"))
	{
		std::string Status;
		if (Preview.bPlaying)
		{
			(void)Seek_EffectivePreview(
				*pPattern, Preview.iPositionMs, !Preview.bPaused, Status);
		}
		else
		{
			(void)Play_EffectivePreview(*pPattern, Status);
		}
		m_strStatus = std::move(Status);
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(nullptr == m_pAnimationTool || !Preview.bPlaying);
	if (ImGui::Button("Stop"))
	{
		std::string Status;
		m_pAnimationTool->Stop_ValtanCompositionPattern(Status);
		m_strStatus = std::move(Status);
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(nullptr == m_pAnimationTool || nullptr == pPattern ||
		!bMutationAdmitted);
	if (ImGui::Button("Restart"))
	{
		std::string Status;
		(void)Play_EffectivePreview(*pPattern, Status);
		m_strStatus = std::move(Status);
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	(void)ImGui::Checkbox("Loop", &m_bLoopPreview);
	ImGui::SameLine();
	ImGui::TextDisabled(
		"%u / %u ms", m_iPlayheadMs, m_iTimelineDurationMs);
	int32_t iPreviewPath = static_cast<int32_t>(m_ePreviewPath);
	ImGui::SetNextItemWidth(250.f);
	if (ImGui::Combo(
			"Preview Branch", &iPreviewPath,
			"Normal / Timeout\0Counter Hit -> Groggy\0Wall Contact -> Groggy\0Wall Contact -> Part Break\0"))
	{
		if (Preview.bPlaying && nullptr != m_pAnimationTool)
		{
			std::string StopStatus;
			m_pAnimationTool->Stop_ValtanCompositionPattern(StopStatus);
		}
		m_ePreviewPath = static_cast<VALTAN_PATTERN_PREVIEW_PATH>(iPreviewPath);
		m_iPreviewDraftGeneration = 0u;
		m_iPlayheadMs = 0u;
		Invalidate_TimelineCache();
	}
	ImGui::SameLine();
	ImGui::TextDisabled("%s", PreviewPathLabel(m_ePreviewPath));
	ImGui::SetNextItemWidth(220.f);
	if (ImGui::SliderFloat(
			"Zoom (px/sec)", &m_fTimelinePixelsPerSecond,
			40.f, 500.f, "%.0f", ImGuiSliderFlags_Logarithmic))
	{
		m_fTimelinePixelsPerSecond = (std::clamp)(
			m_fTimelinePixelsPerSecond, 40.f, 500.f);
	}
	if (nullptr == pPattern)
	{
		ImGui::TextDisabled("Select one Pattern to build its joined tracks.");
		return;
	}
	const VALTAN_STAGE_VIEW* const pSelectedStage = Find_SelectedStage(pPattern);
	ImGui::BeginDisabled(
		nullptr == pSelectedStage || nullptr == m_pAnimationTool ||
		!bMutationAdmitted);
	if (ImGui::Button("Play Selected Stage (All Slots)") &&
		nullptr != pSelectedStage)
	{
		const std::array<VALTAN_PATTERN_PREVIEW_PATH, 5u> CandidatePaths = {
			m_ePreviewPath,
			VALTAN_PATTERN_PREVIEW_PATH::NORMAL,
			VALTAN_PATTERN_PREVIEW_PATH::COUNTER_GROGGY,
			VALTAN_PATTERN_PREVIEW_PATH::WALL_GROGGY,
			VALTAN_PATTERN_PREVIEW_PATH::PART_BREAK,
		};
		bool_t bFoundPath = false;
		uint32_t iSelectedStageStartMs = 0u;
		std::set<VALTAN_PATTERN_PREVIEW_PATH> TriedPaths;
		std::string Status;
		for (const VALTAN_PATTERN_PREVIEW_PATH eCandidatePath : CandidatePaths)
		{
			if (!TriedPaths.insert(eCandidatePath).second)
				continue;
			std::vector<const VALTAN_STAGE_VIEW*> CandidateStages;
			if (!CValtanPatternTree::Build_PreviewStagePath(
					*pPattern, eCandidatePath, CandidateStages, Status))
			{
				continue;
			}
			uint64_t iStageStartMs = 0u;
			for (const VALTAN_STAGE_VIEW* const pCandidateStage :
				CandidateStages)
			{
				if (nullptr == pCandidateStage)
					continue;
				if (pCandidateStage->strActionId ==
					pSelectedStage->strActionId)
				{
					m_ePreviewPath = eCandidatePath;
					iSelectedStageStartMs = static_cast<uint32_t>(
						SaturatingU32(iStageStartMs));
					bFoundPath = true;
					break;
				}
				iStageStartMs += pCandidateStage->iDurationMs;
			}
			if (bFoundPath)
				break;
		}
		if (bFoundPath)
		{
			m_iPreviewDraftGeneration = 0u;
			Invalidate_TimelineCache();
			m_iPlayheadMs = iSelectedStageStartMs;
			(void)Seek_EffectivePreview(
				*pPattern, iSelectedStageStartMs, false, Status);
		}
		else
		{
			Status =
				"Selected Stage is not reachable on any admitted preview branch.";
		}
		m_strStatus = std::move(Status);
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::TextDisabled(
		"Starts this Stage on the Arena Clone with Animation, Effect and collider mirror joined on one clock.");
	const auto SelectedSequence = std::find_if(
		m_AnimationSequences.begin(), m_AnimationSequences.end(),
		[this](const CAnimation_Tool::COMPOSITION_SEQUENCE_VIEW& Sequence)
		{
			return Sequence.iSkillId == m_iSelectedSequenceSkillId &&
				Sequence.iSequenceIndex == m_iSelectedSequenceIndex;
		});
	ImGui::SeparatorText("Selected Sequence -> Selected Stage");
	if (SelectedSequence == m_AnimationSequences.end())
	{
		ImGui::TextDisabled(
			"Choose one Animation resource on the left; click an Animation block below to choose its Stage.");
	}
	else
	{
		ImGui::Text("%s | %zu clips",
			SelectedSequence->strDisplayName.c_str(),
			SelectedSequence->Clips.size());
		ImGui::SameLine();
		ImGui::TextDisabled("-> %s",
			nullptr == pSelectedStage ? "select a Stage" :
				pSelectedStage->strStageId.c_str());
		ImGui::BeginDisabled(nullptr == m_pAnimationTool);
		if (ImGui::Button("Preview Candidate on Arena Clone"))
		{
			std::string Status;
			if (m_pAnimationTool->Preview_ValtanCompositionSequence(
					SelectedSequence->iSkillId,
					SelectedSequence->iSequenceIndex, Status))
			{
				m_bPreviewOwnerClaimRequested = true;
			}
			m_strStatus = std::move(Status);
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::BeginDisabled(
			nullptr == pSelectedStage || !bPatternMutationAdmitted);
		if (ImGui::Button("Replace Stage Slots"))
			(void)Apply_SelectedSequenceToStage(
				*pPattern, *pSelectedStage, false);
		ImGui::SameLine();
		if (ImGui::Button("Append to Stage Slots"))
			(void)Apply_SelectedSequenceToStage(
				*pPattern, *pSelectedStage, true);
		ImGui::EndDisabled();
	}
	const auto SelectedTimelineBox = std::find_if(
		m_TimelineItems.begin(), m_TimelineItems.end(),
		[this, pPattern, pSelectedStage](const TIMELINE_ITEM& Item)
		{
			return nullptr != pSelectedStage &&
				Item.strPatternId == pPattern->strPatternId &&
				Item.strStageId == pSelectedStage->strStageId &&
				Item.strStableId == m_strSelectedStableId &&
				(Item.eOwner == DETAIL_OWNER::ANIMATION ||
				 Item.eOwner == DETAIL_OWNER::EFFECT ||
				 Item.eOwner == DETAIL_OWNER::SOUND);
		});
	const bool_t bTimelineBoxSelected =
		m_TimelineItems.end() != SelectedTimelineBox;
	const bool_t bSelectedSoundBox = bTimelineBoxSelected &&
		DETAIL_OWNER::SOUND == SelectedTimelineBox->eOwner;
	const bool_t bSelectedBoxMutationAdmitted = bTimelineBoxSelected &&
		(bSelectedSoundBox ?
			(bMutationAdmitted && !m_bAuthoringDraftDirty) :
			bPatternMutationAdmitted);
	ImGui::SeparatorText("Selected Box");
	if (bTimelineBoxSelected)
	{
		ImGui::TextDisabled("%s | %s",
			Owner_Label(SelectedTimelineBox->eOwner),
			SelectedTimelineBox->strLabel.c_str());
	}
	else
	{
		ImGui::TextDisabled(
			"Click one Animation, Effect, or Sound box to edit it.");
	}
	ImGui::SameLine();
	ImGui::BeginDisabled(!bSelectedBoxMutationAdmitted);
	if (ImGui::Button("Duplicate Selected Box"))
	{
		std::string Status;
		(void)Duplicate_SelectedTimelineBox(
			*pPattern, *pSelectedStage, Status);
		m_strStatus = std::move(Status);
	}
	ImGui::SameLine();
	if (ImGui::Button("Delete Selected Box"))
	{
		std::string Status;
		(void)Delete_SelectedTimelineBox(
			*pPattern, *pSelectedStage, Status);
		m_strStatus = std::move(Status);
	}
	ImGui::EndDisabled();
	ImGui::TextDisabled(
		"Animation body drag reorders and right-edge drag trims. Effect/Sound body drag changes occurrence time. Duplicate inserts a typed copy; Delete never changes a collider by inference.");
	Render_PatternDurationControl(
		*pPattern, bPatternMutationAdmitted);
	Render_SelectedStageGapControl(
		*pPattern, bPatternMutationAdmitted);
	Render_SelectedAnimationTiming(*pPattern, bMutationAdmitted);
	const uint32_t iRenderDurationMs = (std::min)(
		m_iTimelineDurationMs, MAX_TIMELINE_RENDER_DURATION_MS);
	if (m_iTimelineDurationMs > iRenderDurationMs)
	{
		ImGui::TextColored(
			ImVec4(1.f, 0.72f, 0.25f, 1.f),
			"Timeline display capped at %u ms; authored duration is %u ms.",
			iRenderDurationMs, m_iTimelineDurationMs);
	}
	const float fCanvasWidth = (std::max)(
		520.f,
		static_cast<float>((std::max)(iRenderDurationMs, 1u)) *
			m_fTimelinePixelsPerSecond * 0.001f);
	const float fAvailableHeight = (std::max)(
		150.f, ImGui::GetContentRegionAvail().y);
	if (!ImGui::BeginChild(
			"##ActionCompositionTimelineCanvas", ImVec2(0.f, fAvailableHeight),
			ImGuiChildFlags_Borders,
			ImGuiWindowFlags_HorizontalScrollbar))
	{
		ImGui::EndChild();
		return;
	}

	ImGui::Dummy(ImVec2(TIMELINE_LANE_LABEL_WIDTH, TIMELINE_ROW_HEIGHT));
	ImGui::SameLine(0.f, 0.f);
	ImGui::InvisibleButton(
		"##TimelineRuler", ImVec2(fCanvasWidth, TIMELINE_ROW_HEIGHT));
	const ImVec2 RulerMin = ImGui::GetItemRectMin();
	const ImVec2 RulerMax = ImGui::GetItemRectMax();
	ImDrawList* const pDrawList = ImGui::GetWindowDrawList();
	pDrawList->AddRectFilled(RulerMin, RulerMax, IM_COL32(31, 34, 40, 255));
	const uint32_t iTickMs = m_fTimelinePixelsPerSecond >= 180.f ? 500u : 1000u;
	for (uint64_t iMs = 0u; iMs <= iRenderDurationMs; iMs += iTickMs)
	{
		const float fX = RulerMin.x +
			static_cast<float>(iMs) * m_fTimelinePixelsPerSecond * 0.001f;
		pDrawList->AddLine(
			ImVec2(fX, RulerMin.y), ImVec2(fX, RulerMax.y),
			IM_COL32(91, 96, 108, 255));
		const std::string Label = std::to_string(iMs) + " ms";
		pDrawList->AddText(ImVec2(fX + 3.f, RulerMin.y + 3.f),
			IM_COL32(200, 204, 212, 255), Label.c_str());
	}
	if (ImGui::IsItemActive() &&
		ImGui::IsMouseDown(ImGuiMouseButton_Left))
	{
		const float fLocalX = (std::clamp)(
			ImGui::GetIO().MousePos.x - RulerMin.x, 0.f, fCanvasWidth);
		const uint32_t iRequestedPlayheadMs = static_cast<uint32_t>((std::min)(
			static_cast<double>(iRenderDurationMs),
			static_cast<double>(fLocalX) * 1000.0 /
				static_cast<double>(m_fTimelinePixelsPerSecond)));
		if (iRequestedPlayheadMs != m_iPlayheadMs || ImGui::IsItemActivated())
		{
			m_iPlayheadMs = iRequestedPlayheadMs;
			if (nullptr != m_pAnimationTool && bMutationAdmitted)
			{
				std::string Status;
				(void)Seek_EffectivePreview(
					*pPattern, m_iPlayheadMs, true, Status);
				m_strStatus = std::move(Status);
			}
		}
	}

	/* Owner edits invalidate the cache and are materialized from a fresh joined
	   Pattern on the next frame.  The cached vector is therefore stable for this
	   entire pass and does not need a per-frame deep copy of every label/ID. */
	const std::vector<TIMELINE_ITEM>& TimelineItems = m_TimelineItems;
	bool_t bTimelineSoundDraftDirty = false;
	std::string TimelineSoundStatus;
	const VALTAN_PATTERN_SOUND_CUE_DOCUMENT* const pTimelineSounds =
		ADMISSION_STATE::ADMITTED != m_eAdmission ||
		nullptr == m_pAnimationTool ? nullptr :
		m_pAnimationTool->Get_ValtanCompositionPatternSoundDraft(
			bTimelineSoundDraftDirty, TimelineSoundStatus);
	const bool_t bSoundMoveAdmitted = bMutationAdmitted &&
		!m_bAuthoringDraftDirty && nullptr != pTimelineSounds;
	static constexpr std::array<TIMELINE_LANE, 7u> TIMELINE_LANE_ORDER = {
		TIMELINE_LANE::STAGE,
		TIMELINE_LANE::ANIMATION,
		TIMELINE_LANE::EFFECT,
		TIMELINE_LANE::SOUND,
		TIMELINE_LANE::LOGIC,
		TIMELINE_LANE::COLLIDER,
		TIMELINE_LANE::CAMERA,
	};
	const VALTAN_STAGE_VIEW* pLaneStage = Find_SelectedStage(pPattern);
	if (nullptr == pLaneStage && !pPattern->Stages.empty())
		pLaneStage = &pPattern->Stages.front();
	std::size_t iLaneOrdinal = 0u;
	for (const TIMELINE_LANE eLane : TIMELINE_LANE_ORDER)
	{
		const std::size_t iSubrowCount =
			m_TimelineLaneSubrowCounts[iLaneOrdinal];
		const float fLaneHeight = TIMELINE_ROW_HEIGHT *
			static_cast<float>(iSubrowCount);
		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted(Lane_Label(eLane));
		ImGui::SameLine(TIMELINE_LANE_LABEL_WIDTH - 28.f, 0.f);
		ImGui::PushID(static_cast<int32_t>(eLane));
		ImGui::BeginDisabled(nullptr == pLaneStage);
		if (ImGui::SmallButton("+##LaneAuthoring") && nullptr != pLaneStage)
			Request_LaneAuthoring(eLane, *pPattern, *pLaneStage);
		ImGui::EndDisabled();
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		{
			ImGui::SetTooltip(
				"Open %s authoring/detail for the selected Stage.",
				Lane_Label(eLane));
		}
		ImGui::PopID();
		ImGui::SameLine(TIMELINE_LANE_LABEL_WIDTH, 0.f);
		ImGui::PushID(static_cast<int32_t>(eLane));
		ImGui::InvisibleButton(
			"##TimelineLaneRow", ImVec2(fCanvasWidth, fLaneHeight));
		const ImVec2 RowMin = ImGui::GetItemRectMin();
		const ImVec2 RowMax = ImGui::GetItemRectMax();
		pDrawList->AddRectFilled(RowMin, RowMax,
			0u == (iLaneOrdinal & 1u) ? IM_COL32(24, 27, 32, 255) :
				IM_COL32(29, 32, 38, 255));
		for (std::size_t iItem = 0u; iItem < TimelineItems.size(); ++iItem)
		{
			const TIMELINE_ITEM& Item = TimelineItems[iItem];
			if (Item.eLane != eLane || Item.iStartMs > iRenderDurationMs)
				continue;
			const uint32_t iItemEndMs = (std::min)(
				Item.iEndMs, iRenderDurationMs);
			ImGui::PushID(static_cast<int32_t>(iItem));
		const float fStartX = RowMin.x +
			static_cast<float>(Item.iStartMs) *
			m_fTimelinePixelsPerSecond * 0.001f;
		const float fEndX = (std::max)(
			fStartX + 4.f,
			RowMin.x + static_cast<float>(iItemEndMs) *
				m_fTimelinePixelsPerSecond * 0.001f);
		const float fSubrowY = RowMin.y + TIMELINE_ROW_HEIGHT *
			static_cast<float>(Item.iSubrow);
		const ImVec2 BlockMin(fStartX, fSubrowY + 3.f);
		const ImVec2 BlockMax(
			(std::min)(fEndX, RowMax.x),
			fSubrowY + TIMELINE_ROW_HEIGHT - 3.f);
		pDrawList->AddRectFilled(
			BlockMin, BlockMax, Lane_Color(Item.eLane), 3.f);
		const bool_t bSelectedBlock =
			Item.strPatternId == m_strSelectedPatternId &&
			Item.strStageId == m_strSelectedStageId &&
			Item.strStableId == m_strSelectedStableId;
		if (bSelectedBlock)
		{
			pDrawList->AddRect(
				BlockMin, BlockMax, IM_COL32(255, 224, 92, 255),
				3.f, 0, 2.f);
		}
		const ImVec4 TextClipRect(
			BlockMin.x, BlockMin.y, BlockMax.x, BlockMax.y);
		pDrawList->AddText(
			nullptr, 0.f,
			ImVec2(BlockMin.x + 4.f, BlockMin.y + 1.f),
			IM_COL32(247, 247, 249, 255), Item.strLabel.c_str(),
			nullptr, 0.f, &TextClipRect);
		const bool_t bBlockHovered = ImGui::IsMouseHoveringRect(
			BlockMin, BlockMax, true);
		if (bBlockHovered)
		{
			ImGui::SetTooltip(
				"%s\n%u - %u ms", Item.strLabel.c_str(),
				Item.iStartMs, Item.iEndMs);
		}
		const auto ItemStage = std::find_if(
			pPattern->Stages.begin(), pPattern->Stages.end(),
			[&Item](const VALTAN_STAGE_VIEW& Stage)
			{
				return Stage.strStageId == Item.strStageId;
			});
		const VALTAN_PRODUCT_EFFECT_CUE_VIEW* pEffectCue = nullptr;
		if (DETAIL_OWNER::EFFECT == Item.eOwner &&
			ItemStage != pPattern->Stages.end())
		{
			const auto Cue = std::find_if(
				ItemStage->ProductCues.begin(), ItemStage->ProductCues.end(),
				[&Item](const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Candidate)
				{
					return Candidate.strOccurrenceId == Item.strStableId;
				});
			if (ItemStage->ProductCues.end() != Cue)
				pEffectCue = &*Cue;
		}
		const VALTAN_PATTERN_SOUND_CUE* pSoundCue = nullptr;
		if (DETAIL_OWNER::SOUND == Item.eOwner &&
			nullptr != pTimelineSounds && ItemStage != pPattern->Stages.end())
		{
			const auto Cue = std::find_if(
				pTimelineSounds->Cues.begin(), pTimelineSounds->Cues.end(),
				[&Item, &pPattern, &ItemStage](
					const VALTAN_PATTERN_SOUND_CUE& Candidate)
				{
					return Candidate.strOccurrenceId == Item.strStableId &&
						Candidate.strPatternId == pPattern->strPatternId &&
						Candidate.strStageId == ItemStage->strStageId &&
						Candidate.strActionId == ItemStage->strActionId;
				});
			if (pTimelineSounds->Cues.end() != Cue)
				pSoundCue = &*Cue;
		}
		const bool_t bEffectRightTrim = nullptr != pEffectCue &&
			!pEffectCue->bUsesStageClock && pEffectCue->bHasSourceEnd &&
			"cue_end" == pEffectCue->strStopPolicy &&
			"once" == pEffectCue->strRepeatPolicy;
		const bool_t bTrimOwner =
			(DETAIL_OWNER::GAMEPLAY_STAGE == Item.eOwner &&
			 TIMELINE_LANE::STAGE == Item.eLane) ||
			DETAIL_OWNER::ANIMATION == Item.eOwner || bEffectRightTrim;
		const bool_t bTrimMutationAdmitted = bPatternMutationAdmitted &&
			nullptr != m_pBalanceTool;
		const bool_t bTrimHandleHovered = Item.bEditable && bTrimOwner &&
			std::abs(ImGui::GetIO().MousePos.x - BlockMax.x) <= 7.f &&
			ImGui::GetIO().MousePos.y >= BlockMin.y &&
			ImGui::GetIO().MousePos.y <= BlockMax.y;
		const bool_t bThisTrimActive = m_bTimelineTrimActive &&
			m_strTimelineTrimPatternId == Item.strPatternId &&
			m_strTimelineTrimStageId == Item.strStageId &&
			m_strTimelineTrimStableId == Item.strStableId;
		const bool_t bAnimationMove =
			DETAIL_OWNER::ANIMATION == Item.eOwner &&
			TIMELINE_LANE::ANIMATION == Item.eLane && Item.bEditable &&
			ItemStage != pPattern->Stages.end();
		const bool_t bMoveOwner = bAnimationMove ||
			(nullptr != pEffectCue && !pEffectCue->bUsesStageClock) ||
			nullptr != pSoundCue;
		const bool_t bMoveMutationAdmitted =
			(bAnimationMove && bPatternMutationAdmitted) ||
			(nullptr != pEffectCue && bPatternMutationAdmitted) ||
			(nullptr != pSoundCue && bSoundMoveAdmitted);
		const bool_t bThisMoveActive = m_bTimelineMoveActive &&
			m_strTimelineMovePatternId == Item.strPatternId &&
			m_strTimelineMoveStageId == Item.strStageId &&
			m_strTimelineMoveStableId == Item.strStableId;
		if (bTrimHandleHovered || bThisTrimActive)
		{
			pDrawList->AddLine(
				ImVec2(BlockMax.x - 1.f, BlockMin.y),
				ImVec2(BlockMax.x - 1.f, BlockMax.y),
				IM_COL32(255, 255, 255, 255), 3.f);
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
		}
		if (bBlockHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			m_bDetailsWindowVisible = true;
			if (const VALTAN_STAGE_VIEW* const pStage = Find_SelectedStage(pPattern);
				nullptr == pStage || pStage->strStageId != Item.strStageId)
			{
				const auto FoundStage = std::find_if(
					pPattern->Stages.begin(), pPattern->Stages.end(),
					[&Item](const VALTAN_STAGE_VIEW& Stage)
					{
						return Stage.strStageId == Item.strStageId;
					});
				if (FoundStage != pPattern->Stages.end())
					Select_Stage(*pPattern, *FoundStage, Item.eOwner, Item.strStableId);
			}
			else
			{
				Select_Stage(*pPattern, *pStage, Item.eOwner, Item.strStableId);
			}
			if (bTrimHandleHovered && bTrimMutationAdmitted)
			{
				m_strSelectedStableId = Item.strStableId;
				m_eDetailOwner = Item.eOwner;
				m_bTimelineTrimActive = true;
				m_strTimelineTrimPatternId = Item.strPatternId;
				m_strTimelineTrimStageId = Item.strStageId;
				m_strTimelineTrimStableId = Item.strStableId;
			}
			else if (bMoveOwner && bMoveMutationAdmitted)
			{
				m_bTimelineMoveActive = true;
				m_strTimelineMovePatternId = Item.strPatternId;
				m_strTimelineMoveStageId = Item.strStageId;
				m_strTimelineMoveStableId = Item.strStableId;
				const float fMouseTimelineMs = (std::clamp)(
					(ImGui::GetIO().MousePos.x - RowMin.x) * 1000.f /
						m_fTimelinePixelsPerSecond,
					0.f, static_cast<float>(iRenderDurationMs));
				m_iTimelineMoveMouseStartMs = static_cast<uint32_t>(
					std::llround(fMouseTimelineMs));
				m_iTimelineMoveSourceStartMs = nullptr != pEffectCue ?
					pEffectCue->iSourceStartMs :
					(nullptr != pSoundCue ? pSoundCue->iStartMs : 0u);
				m_iTimelineMoveSourceEndMs = nullptr != pEffectCue &&
					pEffectCue->bHasSourceEnd ? pEffectCue->iSourceEndMs : 0u;
			}
		}
		if (bThisTrimActive && bTrimMutationAdmitted &&
			ImGui::IsMouseReleased(ImGuiMouseButton_Left) &&
			nullptr != m_pBalanceTool)
		{
			const float fMouseTimelineMs = (std::max)(
				0.f, ImGui::GetIO().MousePos.x - RowMin.x) * 1000.f /
				m_fTimelinePixelsPerSecond;
			const uint32_t iNewWallMs = static_cast<uint32_t>((std::clamp)(
				fMouseTimelineMs - static_cast<float>(Item.iStartMs),
				1.f, 120000.f));
			std::string Status;
			bool_t bApplied = false;
			if (ItemStage != pPattern->Stages.end() &&
				DETAIL_OWNER::GAMEPLAY_STAGE == Item.eOwner &&
				TIMELINE_LANE::STAGE == Item.eLane)
			{
				CBalanceTool::PATTERN_STAGE_EDIT Draft;
				bApplied = m_pBalanceTool->Get_ValtanStageDraft(
						Item.strPatternId, Item.strStageId, Draft, Status) &&
					ApplyStageClockPolicy(Draft, iNewWallMs, Status) &&
					SetValtanStageDraftWithSoundDependencyAdmission(
						m_pAnimationTool, m_pBalanceTool,
						m_bPatternShakesReady ? &m_PatternShakes : nullptr,
						*pPattern, *ItemStage, Draft, Status);
			}
			else if (ItemStage != pPattern->Stages.end() &&
				DETAIL_OWNER::ANIMATION == Item.eOwner)
			{
				CBalanceTool::PATTERN_STAGE_EDIT Draft;
				if (m_pBalanceTool->Get_ValtanStageDraft(
						Item.strPatternId, Item.strStageId, Draft, Status))
				{
					const auto Slot = std::find_if(
						Draft.animationSlots.begin(), Draft.animationSlots.end(),
						[&Item](const CBalanceTool::ANIMATION_SLOT_EDIT& Candidate)
						{
							return Candidate.clipOccurrenceId == Item.strStableId;
						});
					if (Draft.animationSlots.end() != Slot &&
						!Slot->repeatUntilStageEnd &&
						std::isfinite(Slot->playRate) && Slot->playRate > 0.0)
					{
						const uint64_t iSourcePlayMs = static_cast<uint64_t>(
							std::llround(static_cast<double>(iNewWallMs) *
								Slot->playRate));
						bApplied = Apply_AnimationOccurrenceTiming(
							*pPattern, *ItemStage, Item.strStableId,
							Slot->sourceStartMs,
							static_cast<uint32_t>((std::clamp)(
								iSourcePlayMs, uint64_t{ 1u }, uint64_t{ 600000u })),
							Status);
					}
					else
					{
						Status =
							"Animation right-edge trim requires one finite non-loop occurrence.";
					}
				}
			}
			else if (ItemStage != pPattern->Stages.end() &&
				nullptr != pEffectCue && bEffectRightTrim)
			{
				const auto Clip = std::find_if(
					ItemStage->ClipOccurrences.begin(),
					ItemStage->ClipOccurrences.end(),
					[pEffectCue](const VALTAN_CLIP_OCCURRENCE_VIEW& Candidate)
					{
						return Candidate.strClipOccurrenceId ==
							pEffectCue->strClipOccurrenceId;
					});
				if (ItemStage->ClipOccurrences.end() != Clip &&
					std::isfinite(Clip->fPlayRate) && Clip->fPlayRate > 0.f)
				{
					const uint64_t iRequestedSourceEnd =
						static_cast<uint64_t>(pEffectCue->iSourceStartMs) +
						static_cast<uint64_t>(std::llround(
							static_cast<double>(iNewWallMs) * Clip->fPlayRate));
					const uint64_t iMaximumSourceEnd = 0u == Clip->iPlayMs ?
						uint64_t{ 600000u } :
						static_cast<uint64_t>(Clip->iSourceStartMs) +
							Clip->iPlayMs;
					const uint32_t iCandidateSourceEnd = static_cast<uint32_t>(
						(std::clamp)(
							iRequestedSourceEnd,
							static_cast<uint64_t>(pEffectCue->iSourceStartMs) + 1u,
							iMaximumSourceEnd));
					bApplied = Apply_EffectOccurrenceTiming(
						*pPattern, *ItemStage, *pEffectCue,
						pEffectCue->iSourceStartMs, iCandidateSourceEnd,
						Status);
				}
				else
				{
					Status =
						"Effect right-edge trim lost its exact finite source-rate window.";
				}
			}
			if (bApplied)
			{
				m_strStatus = std::move(Status);
				Invalidate_TimelineCache();
			}
			else
			{
				m_strStatus = std::move(Status);
			}
		}
		if (bThisMoveActive && bMoveMutationAdmitted &&
			ImGui::IsMouseReleased(ImGuiMouseButton_Left) &&
			ItemStage != pPattern->Stages.end())
		{
			const float fMouseTimelineMs = (std::clamp)(
				(ImGui::GetIO().MousePos.x - RowMin.x) * 1000.f /
					m_fTimelinePixelsPerSecond,
				0.f, static_cast<float>(iRenderDurationMs));
			const int64_t iWallDeltaMs = static_cast<int64_t>(std::llround(
				fMouseTimelineMs)) -
				static_cast<int64_t>(m_iTimelineMoveMouseStartMs);
			if (bAnimationMove)
			{
				const uint32_t iDropMs = static_cast<uint32_t>(
					std::llround(fMouseTimelineMs));
				std::size_t iTargetIndex = 0u;
				for (const TIMELINE_ITEM& Candidate : TimelineItems)
				{
					if (TIMELINE_LANE::ANIMATION != Candidate.eLane ||
						DETAIL_OWNER::ANIMATION != Candidate.eOwner ||
						Candidate.strStageId != Item.strStageId ||
						Candidate.strStableId == Item.strStableId)
					{
						continue;
					}
					const uint32_t iMidpointMs = Candidate.iStartMs +
						(Candidate.iEndMs - Candidate.iStartMs) / 2u;
					if (iDropMs >= iMidpointMs)
						++iTargetIndex;
				}
				std::string Status;
				(void)Reorder_AnimationOccurrence(
					*pPattern, *ItemStage, Item.strStableId,
					iTargetIndex, Status);
				m_strStatus = std::move(Status);
			}
			else if (nullptr != pEffectCue)
			{
				const auto Clip = std::find_if(
					ItemStage->ClipOccurrences.begin(),
					ItemStage->ClipOccurrences.end(),
					[pEffectCue](const VALTAN_CLIP_OCCURRENCE_VIEW& Candidate)
					{
						return Candidate.strClipOccurrenceId ==
							pEffectCue->strClipOccurrenceId;
					});
				std::string Status;
				if (ItemStage->ClipOccurrences.end() == Clip ||
					!std::isfinite(Clip->fPlayRate) || Clip->fPlayRate <= 0.f)
				{
					Status =
						"Effect move lost its exact clip-qualified source-rate window.";
					m_strStatus = std::move(Status);
				}
				else
				{
					const uint64_t iWindowBegin = Clip->iSourceStartMs;
					const uint64_t iMaximumSourceEndpoint = 0u == Clip->iPlayMs ?
						uint64_t{ 600000u } : iWindowBegin + Clip->iPlayMs;
					const uint64_t iSourceLength = 0u == m_iTimelineMoveSourceEndMs ?
						0u : static_cast<uint64_t>(m_iTimelineMoveSourceEndMs) -
							m_iTimelineMoveSourceStartMs;
					const uint64_t iMaximumStart = 0u == Clip->iPlayMs ?
						iMaximumSourceEndpoint - iSourceLength :
						iMaximumSourceEndpoint -
							(std::max)(uint64_t{ 1u }, iSourceLength);
					const int64_t iRequestedStart =
						static_cast<int64_t>(m_iTimelineMoveSourceStartMs) +
						static_cast<int64_t>(std::llround(
							static_cast<double>(iWallDeltaMs) * Clip->fPlayRate));
					const uint32_t iCandidateStart = static_cast<uint32_t>(
						(std::clamp)(iRequestedStart,
							static_cast<int64_t>(iWindowBegin),
							static_cast<int64_t>(iMaximumStart)));
					const uint32_t iCandidateEnd = 0u == iSourceLength ? 0u :
						static_cast<uint32_t>(iCandidateStart + iSourceLength);
					if (iCandidateStart != pEffectCue->iSourceStartMs ||
						iCandidateEnd != pEffectCue->iSourceEndMs)
					{
						(void)Apply_EffectOccurrenceTiming(
							*pPattern, *ItemStage, *pEffectCue,
							iCandidateStart, iCandidateEnd, Status);
						m_strStatus = std::move(Status);
					}
				}
			}
			else if (nullptr != pSoundCue)
			{
				uint32_t iMinimumStartMs = 0u;
				uint32_t iMaximumStartMs = 0u;
				bool_t bLoop = false;
				std::string Status;
				if (!m_pAnimationTool->Resolve_ValtanCompositionPatternSoundWindow(
						*ItemStage, pSoundCue->strClipOccurrenceId,
						iMinimumStartMs, iMaximumStartMs, bLoop, Status))
				{
					m_strSoundStatus = std::move(Status);
				}
				else
				{
					const auto Clip = std::find_if(
						ItemStage->ClipOccurrences.begin(),
						ItemStage->ClipOccurrences.end(),
						[pSoundCue](const VALTAN_CLIP_OCCURRENCE_VIEW& Candidate)
						{
							return Candidate.strClipOccurrenceId ==
								pSoundCue->strClipOccurrenceId;
						});
					if (ItemStage->ClipOccurrences.end() == Clip ||
						!std::isfinite(Clip->fPlayRate) || Clip->fPlayRate <= 0.f)
					{
						m_strSoundStatus =
							"Pattern Sound move lost its exact source-rate window.";
					}
					else
					{
						const int64_t iRequestedStart =
							static_cast<int64_t>(m_iTimelineMoveSourceStartMs) +
							static_cast<int64_t>(std::llround(
								static_cast<double>(iWallDeltaMs) *
								Clip->fPlayRate));
						const uint32_t iCandidateStart = static_cast<uint32_t>(
							(std::clamp)(iRequestedStart,
								static_cast<int64_t>(iMinimumStartMs),
								static_cast<int64_t>(iMaximumStartMs)));
						if (iCandidateStart != pSoundCue->iStartMs)
						{
							(void)Apply_PatternSoundOccurrenceTiming(
								*pPattern, *ItemStage, *pSoundCue,
								iCandidateStart, Status);
							m_strSoundStatus = std::move(Status);
						}
					}
				}
			}
		}
		ImGui::PopID();
		}
		const float fPlayheadX = RowMin.x +
			static_cast<float>((std::min)(m_iPlayheadMs, iRenderDurationMs)) *
			m_fTimelinePixelsPerSecond * 0.001f;
		pDrawList->AddLine(
			ImVec2(fPlayheadX, RowMin.y), ImVec2(fPlayheadX, RowMax.y),
			IM_COL32(255, 220, 72, 220), 1.5f);
		ImGui::PopID();
		++iLaneOrdinal;
	}
	if (m_bTimelineTrimActive &&
		!ImGui::IsMouseDown(ImGuiMouseButton_Left))
	{
		m_bTimelineTrimActive = false;
		m_strTimelineTrimPatternId.clear();
		m_strTimelineTrimStageId.clear();
		m_strTimelineTrimStableId.clear();
	}
	if (m_bTimelineMoveActive &&
		!ImGui::IsMouseDown(ImGuiMouseButton_Left))
	{
		m_bTimelineMoveActive = false;
		m_strTimelineMovePatternId.clear();
		m_strTimelineMoveStageId.clear();
		m_strTimelineMoveStableId.clear();
		m_iTimelineMoveMouseStartMs = 0u;
		m_iTimelineMoveSourceStartMs = 0u;
		m_iTimelineMoveSourceEndMs = 0u;
	}
	ImGui::EndChild();
}

void Client::CActionCompositionWorkbench::Render_SemanticLinkedRows(
	const VALTAN_PATTERN_VIEW* const pPattern,
	const VALTAN_STAGE_VIEW* const pStage)
{
	ImGui::SeparatorText("Selected Pattern / Stage Semantic Links");
	if (nullptr == pPattern)
	{
		ImGui::TextDisabled(
			"Select a Pattern to resolve its typed Animation, Effect, Sound, Camera, World and Server collider links.");
		return;
	}

	ImGui::TextDisabled("Pattern: %s | Stage: %s | Selected occurrence: %s",
		pPattern->strPatternId.c_str(),
		nullptr == pStage ? "NONE" : pStage->strStageId.c_str(),
		m_strSelectedStableId.empty() ? "NONE" :
			m_strSelectedStableId.c_str());
	if (nullptr == pStage)
	{
		ImGui::TextDisabled("Select one Stage to resolve occurrence-level links.");
		return;
	}

	const auto SelectLinkedDetail = [this](
		const DETAIL_OWNER eOwner,
		const std::string& strStableId,
		const std::string& strLabel)
	{
		const bool_t bSelected = eOwner == m_eDetailOwner &&
			strStableId == m_strSelectedStableId;
		if (ImGui::Selectable(strLabel.c_str(), bSelected))
		{
			m_eDetailOwner = eOwner;
			m_strSelectedStableId = strStableId;
		}
	};
	const auto BeginLinkedTable = [](const char_t* const pId)
	{
		if (!ImGui::BeginTable(
				pId, 2,
				ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_RowBg |
				ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp))
		{
			return false;
		}
		ImGui::TableSetupColumn(
			"Semantic linked row", ImGuiTableColumnFlags_WidthStretch, 0.82f);
		ImGui::TableSetupColumn(
			"Typed owner", ImGuiTableColumnFlags_WidthStretch, 0.18f);
		ImGui::TableHeadersRow();
		return true;
	};
	const auto LinkedRow = [&SelectLinkedDetail](
		const DETAIL_OWNER eOwner,
		const std::string& strStableId,
		const std::string& strLabel,
		const char_t* const pOwnerLabel)
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		SelectLinkedDetail(eOwner, strStableId, strLabel);
		ImGui::TableSetColumnIndex(1);
		ImGui::TextDisabled("%s", pOwnerLabel);
	};

	if (ImGui::CollapsingHeader(
			"Animation Sequence Slots", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (pStage->ClipOccurrences.empty())
		{
			ImGui::TextDisabled("No Animation occurrence is linked to this Stage.");
		}
		else if (BeginLinkedTable("##CompositionSemanticAnimationRows"))
		{
			for (const VALTAN_CLIP_OCCURRENCE_VIEW& Clip :
				pStage->ClipOccurrences)
			{
				ImGui::PushID(Clip.strClipOccurrenceId.c_str());
				const std::string PlayLabel = 0u == Clip.iPlayMs ?
					"native" : std::to_string(Clip.iPlayMs) + " ms";
				LinkedRow(
					DETAIL_OWNER::ANIMATION, Clip.strClipOccurrenceId,
					Clip.strClipOccurrenceId + " | " + Clip.strClipName +
					" | source " + std::to_string(Clip.iSourceStartMs) +
					" ms | play " + PlayLabel,
					"Animation Detail");
				ImGui::PopID();
			}
			ImGui::EndTable();
		}
	}

	if ((!pStage->ProductCues.empty() ||
			!pStage->IndependentEffectIds.empty()) &&
		ImGui::CollapsingHeader(
			"Exact Effect Invocations", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (!pStage->ProductCues.empty() &&
			BeginLinkedTable("##CompositionSemanticEffectRows"))
		{
			for (const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue : pStage->ProductCues)
			{
				ImGui::PushID(Cue.strOccurrenceId.c_str());
				const std::string Clock = Cue.bUsesStageClock ?
					"stage clock" : Cue.strClipOccurrenceId;
				LinkedRow(
					DETAIL_OWNER::EFFECT, Cue.strOccurrenceId,
					Cue.strOccurrenceId + " | " + Cue.strEffectAssetId +
					" | " + Clock,
					"Effect Detail");
				ImGui::PopID();
			}
			ImGui::EndTable();
		}
		if (!pStage->IndependentEffectIds.empty())
		{
			for (const std::string& IndependentEffectId :
				pStage->IndependentEffectIds)
			{
				ImGui::BulletText(
					"Independent read-only ref: %s", IndependentEffectId.c_str());
			}
		}
	}

	bool_t bSoundDirty = false;
	std::string SoundStatus;
	const VALTAN_PATTERN_SOUND_CUE_DOCUMENT* const pSounds =
		ADMISSION_STATE::ADMITTED == m_eAdmission &&
		nullptr != m_pAnimationTool ?
		m_pAnimationTool->Get_ValtanCompositionPatternSoundDraft(
			bSoundDirty, SoundStatus) : nullptr;
	bool_t bFoundSound = false;
	if (nullptr != pSounds)
	{
		bFoundSound = std::any_of(
			pSounds->Cues.begin(), pSounds->Cues.end(),
			[pPattern, pStage](const VALTAN_PATTERN_SOUND_CUE& Cue)
			{
				return Cue.strPatternId == pPattern->strPatternId &&
					Cue.strStageId == pStage->strStageId;
			});
	}
	if (bFoundSound && ImGui::CollapsingHeader(
			"Pattern Sound Events", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (BeginLinkedTable("##CompositionSemanticSoundRows"))
		{
			for (const VALTAN_PATTERN_SOUND_CUE& Cue : pSounds->Cues)
			{
				if (Cue.strPatternId != pPattern->strPatternId ||
					Cue.strStageId != pStage->strStageId)
				{
					continue;
				}
				bFoundSound = true;
				ImGui::PushID(Cue.strOccurrenceId.c_str());
				const std::string Resolution = Cue.ResolvedAssetIds.empty() ?
					"UNRESOLVED" :
					std::to_string(Cue.ResolvedAssetIds.size()) +
					" resolved variant(s)";
				LinkedRow(
					DETAIL_OWNER::SOUND, Cue.strOccurrenceId,
					Cue.strOccurrenceId + " | " + Cue.strSoundEvent +
					" | " + Resolution + " | " + Cue.strClipOccurrenceId,
					"Sound Detail");
				ImGui::PopID();
			}
			ImGui::EndTable();
		}
		if (bSoundDirty)
		{
			ImGui::TextDisabled(
				"Rows include the unsaved typed Sound-owner draft generation.");
		}
	}

	bool_t bFoundCamera = !pStage->CameraInvocations.empty();
	if (!bFoundCamera && m_bPatternShakesReady)
	{
		bFoundCamera = std::any_of(
			m_PatternShakes.Cues.begin(), m_PatternShakes.Cues.end(),
			[pPattern, pStage](const VALTAN_PATTERN_SHAKE_CUE& Cue)
			{
				return Cue.strPatternId == pPattern->strPatternId &&
					Cue.strStageId == pStage->strStageId;
			});
	}
	if (bFoundCamera && ImGui::CollapsingHeader("Camera / Light References"))
	{
		if ((!pStage->CameraInvocations.empty() || m_bPatternShakesReady) &&
			BeginLinkedTable("##CompositionSemanticCameraRows"))
		{
			for (const VALTAN_CAMERA_INVOCATION_VIEW& Camera :
				pStage->CameraInvocations)
			{
				bFoundCamera = true;
				ImGui::PushID(Camera.strCameraInvocationId.c_str());
				LinkedRow(
					DETAIL_OWNER::CAMERA, Camera.strCameraInvocationId,
					Camera.strCameraInvocationId + " | " +
					Camera.strCameraCueId + " | start " +
					std::to_string(Camera.iStartOffsetMs) + " ms",
					"Camera Detail");
				ImGui::PopID();
			}
			if (m_bPatternShakesReady)
			{
				for (const VALTAN_PATTERN_SHAKE_CUE& Cue : m_PatternShakes.Cues)
				{
					if (Cue.strPatternId != pPattern->strPatternId ||
						Cue.strStageId != pStage->strStageId)
					{
						continue;
					}
					bFoundCamera = true;
					ImGui::PushID(Cue.strOccurrenceId.c_str());
					LinkedRow(
						DETAIL_OWNER::CAMERA, Cue.strOccurrenceId,
						Cue.strOccurrenceId + " | camera shake | " +
						Cue.strClipOccurrenceId,
						"Camera Detail");
					ImGui::PopID();
				}
			}
			ImGui::EndTable();
		}
	}

	if (ImGui::CollapsingHeader("World / Light Event References"))
	{
		bool_t bFoundWorld = false;
		if (!pPattern->WorldEventTriggerRefs.empty() &&
			BeginLinkedTable("##CompositionSemanticWorldRows"))
		{
			for (const VALTAN_WORLD_EVENT_TRIGGER_REF_VIEW& Event :
				pPattern->WorldEventTriggerRefs)
			{
				if (Event.strStageId != pStage->strStageId)
					continue;
				bFoundWorld = true;
				ImGui::PushID(Event.strTriggerKind.c_str());
				LinkedRow(
					DETAIL_OWNER::WORLD, Event.strTriggerKind,
					Event.strTriggerKind + " | " + Event.strStageId,
					"Server / Map Detail");
				ImGui::PopID();
			}
			ImGui::EndTable();
		}
		if (!bFoundWorld)
			ImGui::TextDisabled("No World Event reference is linked to this Stage.");
	}

	if (ImGui::CollapsingHeader(
			"Server Collider Owner", ImGuiTreeNodeFlags_DefaultOpen))
	{
		const std::string ColliderLabel = pStage->Has_HitShape() ?
			pStage->strStageId + " | " + pStage->strHitShape +
			" | damage " + pStage->strServerDamageProfileId + " | hits " +
			std::to_string(pStage->iHitCount) :
			pStage->strStageId + " | NONE | no Server hit shape";
		if (BeginLinkedTable("##CompositionSemanticColliderRows"))
		{
			ImGui::PushID(pStage->strStageId.c_str());
			LinkedRow(
				DETAIL_OWNER::GAMEPLAY_STAGE, pStage->strStageId,
				ColliderLabel, "Gameplay / Collider");
			ImGui::PopID();
			ImGui::EndTable();
		}
		ImGui::TextDisabled(
			"Server gameplay owns collider geometry, hit clocks and player response; Effect rows remain visual invocations.");
	}
}

void Client::CActionCompositionWorkbench::Render_DataFiles(
	const VALTAN_PATTERN_VIEW* const pPattern,
	const VALTAN_STAGE_VIEW* const pStage,
	const bool_t bPatternMutationAdmitted)
{
	if (!ImGui::CollapsingHeader("Advanced Diagnostics"))
		return;
	if (ImGui::Button("Reset Layout"))
		m_bResetLayoutRequested = true;
	ImGui::TextColored(
		bPatternMutationAdmitted ? ImVec4(0.35f, 0.86f, 0.45f, 1.f) :
			ImVec4(1.f, 0.58f, 0.22f, 1.f),
		"Admission: %s", Admission_Label());
	if (m_bPatternSoundDependencyDirty)
		ImGui::TextColored(
			ImVec4(1.f, 0.62f, 0.20f, 1.f), "Sound owner draft is pending.");
	if (nullptr != pPattern)
	{
		ImGui::TextDisabled("Pattern: %s", pPattern->strPatternId.c_str());
		if (nullptr != pStage)
		{
			ImGui::TextDisabled("Stage: %s", pStage->strStageId.c_str());
			ImGui::TextDisabled("Action: %s", pStage->strActionId.c_str());
		}
		ImGui::TextDisabled("Detail owner: %s", Owner_Label(m_eDetailOwner));
	}
	if (!m_strDisplayProvenance.empty())
		ImGui::TextWrapped("Provenance: %s", m_strDisplayProvenance.c_str());
	if (m_bPatternSaveResultAvailable && !m_strPatternSaveStatus.empty())
		ImGui::TextWrapped("Last save: %s", m_strPatternSaveStatus.c_str());
	if (!m_strStatus.empty())
		ImGui::TextWrapped("Last operation: %s", m_strStatus.c_str());
	if (!m_strSoundStatus.empty())
		ImGui::TextDisabled("Sound lane: %s", m_strSoundStatus.c_str());
	if (!m_strShakeStatus.empty())
		ImGui::TextDisabled("Camera shake lane: %s", m_strShakeStatus.c_str());
	if (!m_strCombatObjectSoundStatus.empty())
		ImGui::TextDisabled("Combat-object Sound lane: %s",
			m_strCombatObjectSoundStatus.c_str());

	std::string ServerPlayStatus;
	std::string SoundRuntimeStatus;
	LostArk::Shared::GameplayDataRevision ExpectedServerRevision{};
	const bool_t bServerRevisionAdmitted = nullptr != m_pBossTool &&
		m_pBossTool->Observe_ServerActivePatternRevision(
			ExpectedServerRevision, ServerPlayStatus);
	const bool_t bSoundRuntimeReady = nullptr != m_pAnimationTool &&
		bServerRevisionAdmitted &&
		m_pAnimationTool->Is_ValtanCompositionPatternSoundRuntimeReady(
			ExpectedServerRevision, SoundRuntimeStatus);
	if (!ServerPlayStatus.empty())
		ImGui::TextWrapped("Server revision: %s", ServerPlayStatus.c_str());
	if (!bSoundRuntimeReady && !SoundRuntimeStatus.empty())
		ImGui::TextWrapped("Server Sound runtime: %s",
			SoundRuntimeStatus.c_str());

	Render_SemanticLinkedRows(pPattern, pStage);
	if (nullptr != pPattern && ImGui::BeginTable(
				"##CompositionOwnerFiles", 3,
				ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
				ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp))
		{
			ImGui::TableSetupColumn("Lane", ImGuiTableColumnFlags_WidthFixed, 100.f);
			ImGui::TableSetupColumn("Source owner", ImGuiTableColumnFlags_WidthStretch, 0.55f);
			ImGui::TableSetupColumn("Projected / runtime consumer", ImGuiTableColumnFlags_WidthStretch, 0.45f);
			ImGui::TableHeadersRow();
			const auto Row = [](const char_t* pLane, const char_t* pSource,
				const char_t* pProduct)
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(pLane);
				ImGui::TableSetColumnIndex(1); ImGui::TextWrapped("%s", pSource);
				ImGui::TableSetColumnIndex(2); ImGui::TextWrapped("%s", pProduct);
			};
			Row("Gameplay", "Data/Valtan/Valtan.gameplay.json",
				"Data/Encounters/Valtan/ValtanEncounter.json -> Server");
			Row("Animation", "Data/Valtan/Valtan.presentation.json",
				"Valtan.patternbindings.json (read-only Product) -> CValtan");
			Row("Effect", "Data/Valtan/Valtan.presentation.json + Data/Effects/Authored/effect.valtan.*",
				"Valtan.patterneffectcues.json (read-only Product) -> Effect presentation");
			Row("Sound", "Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json",
				"CValtan typed Sound cue consumer");
			Row("Camera", "Data/Valtan/Valtan.presentation.json + typed Camera cue owner",
				"Camera presentation service");
			Row("World", "Data/Valtan/Valtan.gameplay.json / Area typed owners",
				"Server world event consumer");
		ImGui::EndTable();
	}
}

void Client::CActionCompositionWorkbench::Render_WindowMenu()
{
	if (!ImGui::BeginMenuBar())
		return;
	if (ImGui::BeginMenu("Windows"))
	{
		ImGui::MenuItem("Patterns", nullptr, &m_bPatternsWindowVisible);
		ImGui::MenuItem("Preview", nullptr, &m_bPreviewWindowVisible);
		ImGui::MenuItem("Sequencer", nullptr, &m_bSequencerWindowVisible);
		ImGui::MenuItem("Details", nullptr, &m_bDetailsWindowVisible);
		ImGui::MenuItem("Resources", nullptr, &m_bResourcesWindowVisible);
		ImGui::MenuItem("Save / Validate / Server", nullptr,
			&m_bSessionWindowVisible);
		ImGui::MenuItem("Boss Pattern", nullptr,
			&m_bBossPatternWindowVisible);
		ImGui::Separator();
		if (ImGui::MenuItem("Show All"))
		{
			m_bPatternsWindowVisible = true;
			m_bPreviewWindowVisible = true;
			m_bSequencerWindowVisible = true;
			m_bDetailsWindowVisible = true;
			m_bResourcesWindowVisible = true;
			m_bSessionWindowVisible = true;
			m_bBossPatternWindowVisible = true;
		}
		if (ImGui::MenuItem("Reset Window Layout"))
			m_bResetLayoutRequested = true;
		ImGui::EndMenu();
	}
	ImGui::EndMenuBar();
}

bool_t Client::CActionCompositionWorkbench::Render_SessionWindow(
	const VALTAN_PATTERN_VIEW* const pPattern,
	const VALTAN_STAGE_VIEW* const pStage,
	const bool_t bPatternMutationAdmitted)
{
	if (!m_bSessionWindowVisible)
		return false;
	const ImGuiViewport* const pViewport = ImGui::GetMainViewport();
	const COMPOSITION_DEFAULT_LAYOUT Layout =
		BuildCompositionDefaultLayout(pViewport);
	const ImGuiCond Condition = m_bApplyResetLayoutThisFrame ?
		ImGuiCond_Always : ImGuiCond_FirstUseEver;
	ImGui::SetNextWindowPos(Layout.SessionPos, Condition);
	ImGui::SetNextWindowSize(Layout.SessionSize, Condition);
	if (!ImGui::Begin(
			"Composition Save / Validate / Server###CompositionSessionWindow",
			&m_bSessionWindowVisible, ImGuiWindowFlags_MenuBar))
	{
		Render_WindowMenu();
		ImGui::End();
		return false;
	}
	Render_WindowMenu();
	if (Render_Toolbar(pPattern, bPatternMutationAdmitted))
	{
		ImGui::End();
		return true;
	}
	Render_DataFiles(pPattern, pStage, bPatternMutationAdmitted);
	ImGui::End();
	return false;
}

void Client::CActionCompositionWorkbench::Render_PatternsWindow(
	const VALTAN_PATTERN_VIEW* const pPattern,
	const VALTAN_STAGE_VIEW* const,
	const bool_t bPatternMutationAdmitted)
{
	if (!m_bPatternsWindowVisible)
		return;
	const ImGuiViewport* const pViewport = ImGui::GetMainViewport();
	const COMPOSITION_DEFAULT_LAYOUT Layout =
		BuildCompositionDefaultLayout(pViewport);
	const ImGuiCond Condition = m_bApplyResetLayoutThisFrame ?
		ImGuiCond_Always : ImGuiCond_FirstUseEver;
	if (m_iRequestedPatternTab >= 0)
	{
		ImGui::SetNextWindowCollapsed(false, ImGuiCond_Always);
		ImGui::SetNextWindowFocus();
	}
	ImGui::SetNextWindowPos(Layout.PatternsPos, Condition);
	ImGui::SetNextWindowSize(Layout.PatternsSize, Condition);
	if (!ImGui::Begin(
			"Composition Patterns###CompositionPatternsWindow",
			&m_bPatternsWindowVisible, ImGuiWindowFlags_MenuBar))
	{
		Render_WindowMenu();
		ImGui::End();
		return;
	}
	Render_WindowMenu();
	if (ImGui::BeginTabBar("##CompositionPatternDomainTabs"))
	{
		if (ImGui::BeginTabItem(
				"Patterns / Stages", nullptr,
				0 == m_iRequestedPatternTab ?
					ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None))
		{
			Render_Browser(pPattern);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem(
				"Create New Pattern", nullptr,
				1 == m_iRequestedPatternTab ?
					ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None))
		{
			ImGui::BeginDisabled(!bPatternMutationAdmitted);
			if (nullptr != m_pAnimationTool)
				m_pAnimationTool->Render_ValtanCompositionPatternCreator();
			else
				ImGui::TextDisabled("Animation Intake owner is unavailable.");
			ImGui::EndDisabled();
			if (!bPatternMutationAdmitted && m_bPatternSoundDependencyDirty)
			{
				ImGui::TextWrapped(
					"Create Pattern is blocked until the Pattern Sound source draft is saved or discarded; changing its animation dependency generation could leave a dangling clipOccurrenceId.");
			}
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
		m_iRequestedPatternTab = -1;
	}
	ImGui::End();
}

void Client::CActionCompositionWorkbench::Render_PreviewWindow(
	const VALTAN_PATTERN_VIEW* const pPattern,
	const bool_t bMutationAdmitted)
{
	if (!m_bPreviewWindowVisible)
		return;
	const ImGuiViewport* const pViewport = ImGui::GetMainViewport();
	const COMPOSITION_DEFAULT_LAYOUT Layout =
		BuildCompositionDefaultLayout(pViewport);
	const ImGuiCond Condition = m_bApplyResetLayoutThisFrame ?
		ImGuiCond_Always : ImGuiCond_FirstUseEver;
	ImGui::SetNextWindowPos(Layout.PreviewPos, Condition);
	ImGui::SetNextWindowSize(Layout.PreviewSize, Condition);
	if (ImGui::Begin(
			"Composition Preview###CompositionPreviewWindow",
			&m_bPreviewWindowVisible, ImGuiWindowFlags_MenuBar))
	{
		Render_WindowMenu();
		Render_Preview(pPattern, bMutationAdmitted);
	}
	else
	{
		Render_WindowMenu();
	}
	ImGui::End();
}

void Client::CActionCompositionWorkbench::Render_SequencerWindow(
	const VALTAN_PATTERN_VIEW* const pPattern,
	const bool_t bMutationAdmitted,
	const bool_t bPatternMutationAdmitted)
{
	if (!m_bSequencerWindowVisible)
		return;
	const ImGuiViewport* const pViewport = ImGui::GetMainViewport();
	const COMPOSITION_DEFAULT_LAYOUT Layout =
		BuildCompositionDefaultLayout(pViewport);
	const ImGuiCond Condition = m_bApplyResetLayoutThisFrame ?
		ImGuiCond_Always : ImGuiCond_FirstUseEver;
	ImGui::SetNextWindowPos(Layout.SequencerPos, Condition);
	ImGui::SetNextWindowSize(Layout.SequencerSize, Condition);
	if (ImGui::Begin(
			"Composition Sequencer###CompositionSequencerWindow",
			&m_bSequencerWindowVisible, ImGuiWindowFlags_MenuBar))
	{
		Render_WindowMenu();
		Render_Timeline(
			pPattern, bMutationAdmitted, bPatternMutationAdmitted);
	}
	else
	{
		Render_WindowMenu();
	}
	ImGui::End();
}

void Client::CActionCompositionWorkbench::Render_DetailsWindow(
	const VALTAN_PATTERN_VIEW* const pPattern,
	const VALTAN_STAGE_VIEW* const pStage,
	const bool_t bMutationAdmitted,
	const bool_t bPatternMutationAdmitted)
{
	if (!m_bDetailsWindowVisible)
		return;
	const ImGuiViewport* const pViewport = ImGui::GetMainViewport();
	const COMPOSITION_DEFAULT_LAYOUT Layout =
		BuildCompositionDefaultLayout(pViewport);
	const ImGuiCond Condition = m_bApplyResetLayoutThisFrame ?
		ImGuiCond_Always : ImGuiCond_FirstUseEver;
	ImGui::SetNextWindowPos(Layout.DetailsPos, Condition);
	ImGui::SetNextWindowSize(Layout.DetailsSize, Condition);
	if (ImGui::Begin(
			"Composition Details###CompositionDetailsWindow",
			&m_bDetailsWindowVisible, ImGuiWindowFlags_MenuBar))
	{
		Render_WindowMenu();
		Render_Details(
			pPattern, pStage, bMutationAdmitted, bPatternMutationAdmitted);
	}
	else
	{
		Render_WindowMenu();
	}
	ImGui::End();
}

void Client::CActionCompositionWorkbench::Request_LaneAuthoring(
	const TIMELINE_LANE eLane,
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage)
{
	const auto OpenResources = [this, &Pattern, &Stage](
		const RESOURCE_DOMAIN eDomain)
	{
		m_eRequestedResourceDomain = eDomain;
		m_bResourceDomainSelectionRequested = true;
		m_bResourcesWindowVisible = true;
		m_bResourcesWindowExpandRequested = true;
		m_bResourcesWindowFocusRequested = true;
		m_strResourceTargetPatternId = Pattern.strPatternId;
		m_strResourceTargetStageId = Stage.strStageId;
		const auto SelectedClip = std::find_if(
			Stage.ClipOccurrences.begin(), Stage.ClipOccurrences.end(),
			[this](const VALTAN_CLIP_OCCURRENCE_VIEW& Clip)
			{
				return Clip.strClipOccurrenceId == m_strSelectedStableId;
			});
		m_strResourceTargetClipOccurrenceId =
			Stage.ClipOccurrences.end() != SelectedClip ?
				SelectedClip->strClipOccurrenceId :
				(Stage.ClipOccurrences.empty() ? std::string{} :
					Stage.ClipOccurrences.back().strClipOccurrenceId);
		if (RESOURCE_DOMAIN::EFFECT == eDomain)
			m_strEffectAddClipOccurrenceId =
				m_strResourceTargetClipOccurrenceId;
		if (RESOURCE_DOMAIN::SOUND == eDomain)
		{
			m_strSoundAddClipOccurrenceId =
				m_strResourceTargetClipOccurrenceId;
			const auto SoundClip = std::find_if(
				Stage.ClipOccurrences.begin(), Stage.ClipOccurrences.end(),
				[this](const VALTAN_CLIP_OCCURRENCE_VIEW& Clip)
				{
					return Clip.strClipOccurrenceId ==
						m_strResourceTargetClipOccurrenceId;
				});
			m_iSoundAddStartMs = Stage.ClipOccurrences.end() == SoundClip ?
				0u : SoundClip->iSourceStartMs;
			m_eSoundAddRepeatPolicy =
				VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE;
		}
	};

	switch (eLane)
	{
	case TIMELINE_LANE::STAGE:
		m_bDetailsWindowVisible = true;
		Select_Stage(
			Pattern, Stage, DETAIL_OWNER::GAMEPLAY_STAGE,
			Stage.strStageId + "/topology");
		m_strStatus = Pattern.bManualServerAudition ?
			"Stage authoring opened. Insert ACTIVE/WINDUP/GROGGY or an explicit WAIT gap after the selected Stage." :
			"This canonical Pattern exposes Stage timing as read-only; Create New Pattern produces a manual authoring topology.";
		break;
	case TIMELINE_LANE::ANIMATION:
		OpenResources(RESOURCE_DOMAIN::ANIMATION);
		Select_Stage(
			Pattern, Stage, DETAIL_OWNER::ANIMATION,
			Stage.ClipOccurrences.empty() ? Stage.strStageId :
				Stage.ClipOccurrences.front().strClipOccurrenceId);
		m_strStatus =
			"Animation catalog opened. Select a semantic Sequence, then replace, append, or duplicate the selected Stage slots.";
		break;
	case TIMELINE_LANE::EFFECT:
		OpenResources(RESOURCE_DOMAIN::EFFECT);
		Select_Stage(Pattern, Stage, DETAIL_OWNER::EFFECT, Stage.strStageId);
		m_strEffectAddClipOccurrenceId =
			m_strResourceTargetClipOccurrenceId;
		m_strStatus =
			"Effect catalog and typed invocation Details opened for the selected Stage.";
		break;
	case TIMELINE_LANE::SOUND:
		OpenResources(RESOURCE_DOMAIN::SOUND);
		Select_Stage(Pattern, Stage, DETAIL_OWNER::SOUND, Stage.strStageId);
		m_strSoundAddClipOccurrenceId =
			m_strResourceTargetClipOccurrenceId;
		if (const auto TargetClip = std::find_if(
				Stage.ClipOccurrences.begin(), Stage.ClipOccurrences.end(),
				[this](const VALTAN_CLIP_OCCURRENCE_VIEW& Clip)
				{
					return Clip.strClipOccurrenceId ==
						m_strSoundAddClipOccurrenceId;
				}); TargetClip != Stage.ClipOccurrences.end())
		{
			m_iSoundAddStartMs = TargetClip->iSourceStartMs;
		}
		m_strStatus =
			"Sound event catalog and typed Pattern Sound row editor opened for the selected Stage.";
		break;
	case TIMELINE_LANE::LOGIC:
		OpenResources(RESOURCE_DOMAIN::LOGIC);
		Select_Stage(
			Pattern, Stage, DETAIL_OWNER::GAMEPLAY_STAGE,
			Stage.strStageId + "/branch/COUNTER_HIT/authoring");
		m_strStatus =
			"Logic templates opened. Only typed gameplay contracts admitted by this Stage can be authored.";
		break;
	case TIMELINE_LANE::COLLIDER:
		m_bDetailsWindowVisible = true;
		Select_Stage(
			Pattern, Stage, DETAIL_OWNER::GAMEPLAY_STAGE,
			Stage.strStageId + "/collider");
		m_strStatus =
			"Server Collider / Hit Schedule Details opened; visual Effect geometry is never copied into hit authority.";
		break;
	case TIMELINE_LANE::CAMERA:
		OpenResources(RESOURCE_DOMAIN::CAMERA);
		if (!Stage.CameraInvocations.empty())
		{
			Select_Stage(
				Pattern, Stage, DETAIL_OWNER::CAMERA,
				Stage.CameraInvocations.front().strCameraInvocationId);
		}
		else
		{
			m_bCameraToolOpenRequested = true;
			m_strStatus =
				"No exact Camera invocation is admitted on this Stage; opening the Camera owner tool.";
		}
		break;
	default:
		break;
	}
}

void Client::CActionCompositionWorkbench::Render_ResourcesWindow(
	const VALTAN_PATTERN_VIEW* const pPattern,
	const VALTAN_STAGE_VIEW* const pStage,
	const bool_t bMutationAdmitted,
	const bool_t bPatternMutationAdmitted)
{
	if (!m_bResourcesWindowVisible)
		return;
	const ImGuiViewport* const pViewport = ImGui::GetMainViewport();
	const COMPOSITION_DEFAULT_LAYOUT Layout =
		BuildCompositionDefaultLayout(pViewport);
	const ImGuiCond Condition = m_bApplyResetLayoutThisFrame ?
		ImGuiCond_Always : ImGuiCond_FirstUseEver;
	if (m_bResourcesWindowExpandRequested && nullptr != pViewport)
	{
		const ImVec2 ExpandedPos(
			pViewport->WorkPos.x + pViewport->WorkSize.x * 0.08f,
			pViewport->WorkPos.y + pViewport->WorkSize.y * 0.12f);
		const ImVec2 ExpandedSize(
			pViewport->WorkSize.x * 0.62f,
			pViewport->WorkSize.y * 0.72f);
		ImGui::SetNextWindowPos(ExpandedPos, ImGuiCond_Always);
		ImGui::SetNextWindowSize(ExpandedSize, ImGuiCond_Always);
		ImGui::SetNextWindowCollapsed(false, ImGuiCond_Always);
	}
	else
	{
		ImGui::SetNextWindowPos(Layout.ResourcesPos, Condition);
		ImGui::SetNextWindowSize(Layout.ResourcesSize, Condition);
	}
	if (m_bResourcesWindowFocusRequested)
		ImGui::SetNextWindowFocus();
	m_bResourcesWindowExpandRequested = false;
	m_bResourcesWindowFocusRequested = false;
	if (!ImGui::Begin(
			"Composition Resources###CompositionResourcesWindow",
			&m_bResourcesWindowVisible, ImGuiWindowFlags_MenuBar))
	{
		Render_WindowMenu();
		ImGui::End();
		return;
	}
	Render_WindowMenu();
	const VALTAN_PATTERN_VIEW* pResourcePattern = pPattern;
	if (!m_strResourceTargetPatternId.empty() &&
		(nullptr == pResourcePattern ||
		 pResourcePattern->strPatternId != m_strResourceTargetPatternId))
	{
		pResourcePattern = nullptr;
	}
	const VALTAN_STAGE_VIEW* pResourceStage = nullptr;
	if (nullptr != pResourcePattern)
	{
		const std::string& strTargetStageId =
			m_strResourceTargetStageId.empty() && nullptr != pStage ?
				pStage->strStageId : m_strResourceTargetStageId;
		const auto TargetStage = std::find_if(
			pResourcePattern->Stages.begin(), pResourcePattern->Stages.end(),
			[&strTargetStageId](const VALTAN_STAGE_VIEW& Candidate)
			{ return Candidate.strStageId == strTargetStageId; });
		if (pResourcePattern->Stages.end() != TargetStage)
			pResourceStage = &*TargetStage;
	}
	ImGui::TextDisabled(
		"Only the opened typed catalog is loaded; Render never scans repository files.");
	ImGui::TextDisabled("Append target: %s / %s",
		nullptr == pResourcePattern ? "UNAVAILABLE" :
			pResourcePattern->strPatternId.c_str(),
		nullptr == pResourceStage ? "UNAVAILABLE" :
			pResourceStage->strStageId.c_str());
	const auto ResourceTabFlags = [this](const RESOURCE_DOMAIN eDomain)
	{
		return m_bResourceDomainSelectionRequested &&
			m_eRequestedResourceDomain == eDomain ?
			ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None;
	};
	bool_t bResourceSelectionConsumed = false;
	if (ImGui::BeginTabBar("##CompositionResourceDomainTabs"))
	{
		if (ImGui::BeginTabItem(
				"Animation", nullptr,
				ResourceTabFlags(RESOURCE_DOMAIN::ANIMATION)))
		{
			bResourceSelectionConsumed |= m_bResourceDomainSelectionRequested &&
				RESOURCE_DOMAIN::ANIMATION == m_eRequestedResourceDomain;
			Render_SequenceBrowser(
				pResourcePattern, pResourceStage, bPatternMutationAdmitted);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem(
				"Effect", nullptr,
				ResourceTabFlags(RESOURCE_DOMAIN::EFFECT)))
		{
			bResourceSelectionConsumed |= m_bResourceDomainSelectionRequested &&
				RESOURCE_DOMAIN::EFFECT == m_eRequestedResourceDomain;
			if (!m_bSemanticValtanEffectLoadAttempted)
				Reload_SemanticValtanEffects();
			if (ImGui::Button("Refresh Effect Catalog"))
				Reload_SemanticValtanEffects();
			if (!m_strEffectCatalogStatus.empty())
				ImGui::TextWrapped("%s", m_strEffectCatalogStatus.c_str());
			ImGui::SetNextItemWidth(-1.f);
			if (ImGui::InputTextWithHint(
				"##CompositionResourceEffectSearch", "Search V1/V2 Effect or group...",
				m_EffectSearch.data(), m_EffectSearch.size()))
			{
				m_bEffectFilterDirty = true;
			}
			const std::string Query = m_EffectSearch.data();
			if (m_bEffectFilterDirty || m_strEffectFilterQuery != Query)
			{
				m_FilteredEffectAssetIndices.clear();
				m_FilteredEffectV2DocumentIndices.clear();
				m_FilteredEffectV2GroupIndices.clear();
				m_FilteredEffectAssetIndices.reserve(
					m_SemanticValtanEffectAssetIds.size());
				for (std::size_t i = 0u;
					i < m_SemanticValtanEffectAssetIds.size(); ++i)
				{
					if (ContainsInsensitive(
							m_SemanticValtanEffectAssetIds[i], Query))
					{
						m_FilteredEffectAssetIndices.push_back(i);
					}
				}
				for (std::size_t i = 0u; i < m_EffectV2DocumentIds.size(); ++i)
				{
					if (ContainsInsensitive(m_EffectV2DocumentIds[i], Query))
						m_FilteredEffectV2DocumentIndices.push_back(i);
				}
				for (std::size_t i = 0u; i < m_EffectV2GroupIds.size(); ++i)
				{
					if (ContainsInsensitive(m_EffectV2GroupIds[i], Query))
						m_FilteredEffectV2GroupIndices.push_back(i);
				}

				m_EffectV1ResourceTree = {};
				for (const std::size_t i : m_FilteredEffectAssetIndices)
				{
					std::vector<std::string> Segments =
						SplitResourcePath(m_SemanticValtanEffectAssetIds[i]);
					if (!Segments.empty())
						Segments.pop_back();
					InsertResourceTree(m_EffectV1ResourceTree, Segments, i);
				}
				(void)FinalizeResourceTree(m_EffectV1ResourceTree);
				m_EffectV2DocumentResourceTree = {};
				for (const std::size_t i : m_FilteredEffectV2DocumentIndices)
				{
					std::vector<std::string> Segments{
						EffectV2TypeLabel(m_EffectV2DocumentTypes[i]) };
					std::vector<std::string> IdSegments =
						SplitResourcePath(m_EffectV2DocumentIds[i]);
					if (!IdSegments.empty())
						IdSegments.pop_back();
					Segments.insert(
						Segments.end(), IdSegments.begin(), IdSegments.end());
					InsertResourceTree(
						m_EffectV2DocumentResourceTree, Segments, i);
				}
				(void)FinalizeResourceTree(m_EffectV2DocumentResourceTree);
				m_EffectV2GroupResourceTree = {};
				for (const std::size_t i : m_FilteredEffectV2GroupIndices)
				{
					std::vector<std::string> Segments =
						SplitResourcePath(m_EffectV2GroupIds[i]);
					if (!Segments.empty())
						Segments.pop_back();
					InsertResourceTree(m_EffectV2GroupResourceTree, Segments, i);
				}
				(void)FinalizeResourceTree(m_EffectV2GroupResourceTree);
				m_strEffectFilterQuery = Query;
				m_bEffectFilterDirty = false;
			}
			ImGui::SeparatorText("Selected Effect -> Append Target");
			const char_t* const pEffectKind =
				EFFECT_RESOURCE_KIND::V1_PATTERN == m_eEffectAddResourceKind ?
					"V1 Pattern" :
					(EFFECT_RESOURCE_KIND::V2_LEAF == m_eEffectAddResourceKind ?
						"V2 Leaf" : "V2 Group");
			ImGui::TextWrapped("%s | %s", pEffectKind,
				m_strEffectAddAssetId.empty() ?
					"Select one Effect below." :
					m_strEffectAddAssetId.c_str());
			const VALTAN_CLIP_OCCURRENCE_VIEW* pEffectTargetClip = nullptr;
			if (nullptr != pResourceStage)
			{
				const auto TargetClip = std::find_if(
					pResourceStage->ClipOccurrences.begin(),
					pResourceStage->ClipOccurrences.end(),
					[this](const VALTAN_CLIP_OCCURRENCE_VIEW& Clip)
					{
						return Clip.strClipOccurrenceId ==
							m_strEffectAddClipOccurrenceId;
					});
				if (pResourceStage->ClipOccurrences.end() != TargetClip)
					pEffectTargetClip = &*TargetClip;
			}
			const bool_t bV1Selection =
				EFFECT_RESOURCE_KIND::V1_PATTERN == m_eEffectAddResourceKind;
			const char_t* const pEffectClipPreview =
				nullptr == pEffectTargetClip ?
					"Select Animation box" :
					pEffectTargetClip->strClipName.c_str();
			ImGui::BeginDisabled(!bV1Selection);
			if (ImGui::BeginCombo(
					"Attach to Animation Box##ResourceEffect",
					pEffectClipPreview))
			{
				if (nullptr != pResourceStage)
				{
					for (const VALTAN_CLIP_OCCURRENCE_VIEW& Clip :
						pResourceStage->ClipOccurrences)
					{
						const bool_t bSelected = Clip.strClipOccurrenceId ==
							m_strEffectAddClipOccurrenceId;
						const std::string Label = Clip.strClipName + " | " +
							Clip.strClipOccurrenceId;
						if (ImGui::Selectable(Label.c_str(), bSelected))
							m_strEffectAddClipOccurrenceId =
								Clip.strClipOccurrenceId;
					}
				}
				ImGui::EndCombo();
			}
			ImGui::EndDisabled();
			const bool_t bEffectAppendAdmitted =
				nullptr != pResourcePattern && nullptr != pResourceStage &&
				nullptr != pEffectTargetClip && nullptr != m_pBalanceTool &&
				bPatternMutationAdmitted &&
				"WAIT" != pResourceStage->strSequenceRole &&
				!m_strEffectAddAssetId.empty() && bV1Selection;
			ImGui::BeginDisabled(!bEffectAppendAdmitted);
			if (ImGui::Button("Append V1 Effect to Pattern Draft") &&
				bEffectAppendAdmitted)
			{
				VALTAN_PRODUCT_EFFECT_CUE_VIEW Cue;
				Cue.strBindingId = BuildNextCompositionEffectCueId(
					*pResourcePattern, *pResourceStage);
				Cue.strOccurrenceId = Cue.strBindingId + ".occurrence.01";
				Cue.strPatternId = pResourcePattern->strPatternId;
				Cue.strStageId = pResourceStage->strStageId;
				Cue.strActionId = pResourceStage->strActionId;
				Cue.strClipOccurrenceId =
					pEffectTargetClip->strClipOccurrenceId;
				Cue.strEffectAssetId = m_strEffectAddAssetId;
				Cue.strAnchorSlotId = "root";
				Cue.eFollowPolicy = EFFECT_FOLLOW_POLICY::FOLLOW;
				Cue.strFollowPolicy = "follow";
				Cue.eStopPolicy = EFFECT_STOP_POLICY::NATURAL;
				Cue.strStopPolicy = "natural";
				Cue.strRepeatPolicy = "once";
				Cue.eScalePolicy =
					VALTAN_PATTERN_EFFECT_SCALE_POLICY::OWNER_RELATIVE;
				Cue.strScalePolicy = "OWNER_RELATIVE";
				Cue.bHasExplicitScalePolicy = true;
				Cue.iSourceStartMs = pEffectTargetClip->iSourceStartMs;
				Cue.iStageDurationMs = pResourceStage->iDurationMs;
				std::string Status;
				if (!Cue.strBindingId.empty() &&
					m_pBalanceTool->Add_ValtanStageEffectCue(
						pResourcePattern->strPatternId,
						pResourceStage->strStageId,
						pResourceStage->strActionId, Cue, Status))
				{
					m_eDetailOwner = DETAIL_OWNER::EFFECT;
					m_strSelectedStableId = Cue.strOccurrenceId;
					m_strEffectEditIdentity.clear();
					m_bDetailsWindowVisible = true;
					Invalidate_TimelineCache();
				}
				m_strStatus = Cue.strBindingId.empty() ?
					"Effect append rejected: no free stable invocation ID is available." :
					std::move(Status);
			}
			ImGui::EndDisabled();

			const uint32_t iStepMs = 1u;
			const uint32_t iFastStepMs = 100u;
			ImGui::BeginDisabled(bV1Selection || nullptr == pResourceStage);
			if (ImGui::InputScalar(
					"Stage-local start (ms)##ResourceEffectV2",
					ImGuiDataType_U32, &m_iEffectV2AddStartMs,
					&iStepMs, &iFastStepMs, "%u") && nullptr != pResourceStage)
			{
				m_iEffectV2AddStartMs = (std::min)(
					m_iEffectV2AddStartMs, pResourceStage->iDurationMs);
			}
			ImGui::EndDisabled();
			const bool_t bV2AppendAdmitted = !bV1Selection &&
				nullptr != pResourceStage && bMutationAdmitted &&
				!m_strEffectAddAssetId.empty();
			ImGui::BeginDisabled(!bV2AppendAdmitted);
			if (ImGui::Button("Append + Save V2 Stage Binding") &&
				bV2AppendAdmitted)
			{
				std::string Status;
				if (CEffectV2Catalog::Get().Append_BossValtanStageBinding(
						m_strEffectAddAssetId,
						EFFECT_RESOURCE_KIND::V2_GROUP ==
							m_eEffectAddResourceKind,
						pResourceStage->strActionId,
						m_iEffectV2AddStartMs, Status))
				{
					m_iEffectV2CatalogRevision =
						CEffectV2Catalog::Get().Get_Revision();
					Invalidate_TimelineCache();
				}
				m_strEffectCatalogStatus = Status;
				m_strStatus = std::move(Status);
			}
			ImGui::EndDisabled();
			if (!bV1Selection)
			{
				ImGui::TextDisabled(
					"V2 owns Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json. This button saves it immediately; Product publish + Server restart is still required.");
			}

			ImGui::SeparatorText("All Effect Resources");
			if (ImGui::TreeNodeEx(
					"V1 Pattern Effects", ImGuiTreeNodeFlags_SpanAvailWidth))
			{
				RenderResourceTree(
					m_EffectV1ResourceTree,
					[this](const std::size_t i)
					{
						const std::string& AssetId =
							m_SemanticValtanEffectAssetIds[i];
						const bool_t bSelected =
							EFFECT_RESOURCE_KIND::V1_PATTERN ==
								m_eEffectAddResourceKind &&
							AssetId == m_strEffectAddAssetId;
						if (ImGui::Selectable(AssetId.c_str(), bSelected))
						{
							m_eEffectAddResourceKind =
								EFFECT_RESOURCE_KIND::V1_PATTERN;
							m_strEffectAddAssetId = AssetId;
						}
					});
				ImGui::TreePop();
			}
			if (ImGui::TreeNodeEx(
					"V2 Authored Effects", ImGuiTreeNodeFlags_SpanAvailWidth))
			{
				RenderResourceTree(
					m_EffectV2DocumentResourceTree,
					[this](const std::size_t i)
					{
						const std::string& AssetId = m_EffectV2DocumentIds[i];
						const bool_t bSelected =
							EFFECT_RESOURCE_KIND::V2_LEAF ==
								m_eEffectAddResourceKind &&
							AssetId == m_strEffectAddAssetId;
						if (ImGui::Selectable(AssetId.c_str(), bSelected))
						{
							m_eEffectAddResourceKind =
								EFFECT_RESOURCE_KIND::V2_LEAF;
							m_strEffectAddAssetId = AssetId;
						}
					});
				ImGui::TreePop();
			}
			if (ImGui::TreeNodeEx(
					"V2 Effect Groups", ImGuiTreeNodeFlags_SpanAvailWidth))
			{
				RenderResourceTree(
					m_EffectV2GroupResourceTree,
					[this](const std::size_t i)
					{
						const std::string& AssetId = m_EffectV2GroupIds[i];
						const bool_t bSelected =
							EFFECT_RESOURCE_KIND::V2_GROUP ==
								m_eEffectAddResourceKind &&
							AssetId == m_strEffectAddAssetId;
						if (ImGui::Selectable(AssetId.c_str(), bSelected))
						{
							m_eEffectAddResourceKind =
								EFFECT_RESOURCE_KIND::V2_GROUP;
							m_strEffectAddAssetId = AssetId;
						}
					});
				ImGui::TreePop();
			}
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem(
				"Sound", nullptr,
				ResourceTabFlags(RESOURCE_DOMAIN::SOUND)))
		{
			bResourceSelectionConsumed |= m_bResourceDomainSelectionRequested &&
				RESOURCE_DOMAIN::SOUND == m_eRequestedResourceDomain;
			if (m_strSoundAddEvent.empty() && !m_PatternSoundEvents.empty())
				m_strSoundAddEvent = m_PatternSoundEvents.front();
			ImGui::SetNextItemWidth(-1.f);
			if (ImGui::InputTextWithHint(
					"##CompositionResourceSoundSearch", "Search Sound event...",
					m_SoundSearch.data(), m_SoundSearch.size()))
			{
				m_bSoundFilterDirty = true;
			}
			const std::string SoundQuery = m_SoundSearch.data();
			if (m_bSoundFilterDirty || m_strSoundFilterQuery != SoundQuery)
			{
				m_FilteredSoundEventIndices.clear();
				m_SoundResourceTree = {};
				for (std::size_t i = 0u; i < m_PatternSoundEvents.size(); ++i)
				{
					if (!ContainsInsensitive(m_PatternSoundEvents[i], SoundQuery))
						continue;
					m_FilteredSoundEventIndices.push_back(i);
					std::vector<std::string> Segments =
						SplitResourcePath(m_PatternSoundEvents[i]);
					if (!Segments.empty())
						Segments.pop_back();
					InsertResourceTree(m_SoundResourceTree, Segments, i);
				}
				(void)FinalizeResourceTree(m_SoundResourceTree);
				m_strSoundFilterQuery = SoundQuery;
				m_bSoundFilterDirty = false;
			}
			const VALTAN_CLIP_OCCURRENCE_VIEW* pSoundTargetClip = nullptr;
			if (nullptr != pResourceStage)
			{
				const auto TargetClip = std::find_if(
					pResourceStage->ClipOccurrences.begin(),
					pResourceStage->ClipOccurrences.end(),
					[this](const VALTAN_CLIP_OCCURRENCE_VIEW& Clip)
					{
						return Clip.strClipOccurrenceId ==
							m_strSoundAddClipOccurrenceId;
					});
				if (pResourceStage->ClipOccurrences.end() != TargetClip)
					pSoundTargetClip = &*TargetClip;
			}
			ImGui::SeparatorText("Selected Sound -> Append Target");
			ImGui::TextWrapped("%s",
				m_strSoundAddEvent.empty() ?
					"Select one Sound event below." :
					m_strSoundAddEvent.c_str());
			const char_t* const pSoundClipPreview =
				nullptr == pSoundTargetClip ?
					"Select Animation box" :
					pSoundTargetClip->strClipName.c_str();
			if (ImGui::BeginCombo(
					"Attach to Animation Box##ResourceSound",
					pSoundClipPreview))
			{
				if (nullptr != pResourceStage)
				{
					for (const VALTAN_CLIP_OCCURRENCE_VIEW& Clip :
						pResourceStage->ClipOccurrences)
					{
						const bool_t bSelected = Clip.strClipOccurrenceId ==
							m_strSoundAddClipOccurrenceId;
						const std::string Label = Clip.strClipName + " | " +
							Clip.strClipOccurrenceId;
						if (ImGui::Selectable(Label.c_str(), bSelected))
						{
							m_strSoundAddClipOccurrenceId =
								Clip.strClipOccurrenceId;
							m_iSoundAddStartMs = Clip.iSourceStartMs;
							m_eSoundAddRepeatPolicy =
								VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE;
						}
					}
				}
				ImGui::EndCombo();
			}
			/* Keep the catalog render O(visible rows).  Native model-window
			   validation can walk all animations, so the authoritative Add API does
			   that once on click instead of doing it every ImGui frame. */
			const bool_t bSoundTargetReady =
				nullptr != pResourceStage && nullptr != pSoundTargetClip &&
				nullptr != m_pAnimationTool;
			ImGui::BeginDisabled(!bSoundTargetReady);
			const uint32_t iStepMs = 1u;
			const uint32_t iFastStepMs = 100u;
			if (ImGui::InputScalar(
				"Source start (ms)##ResourceSound", ImGuiDataType_U32,
				&m_iSoundAddStartMs, &iStepMs, &iFastStepMs, "%u"))
			{
				m_iSoundAddStartMs = (std::min)(
					m_iSoundAddStartMs, uint32_t{ 600000u });
			}
			ImGui::EndDisabled();
			const bool_t bSoundAppendAdmitted =
				nullptr != pResourcePattern && nullptr != pResourceStage &&
				nullptr != pSoundTargetClip && nullptr != m_pAnimationTool &&
				bMutationAdmitted && !m_bAuthoringDraftDirty &&
				bSoundTargetReady && !m_strSoundAddEvent.empty();
			ImGui::BeginDisabled(!bSoundAppendAdmitted);
			if (ImGui::Button("Append Selected Sound to Stage") &&
				bSoundAppendAdmitted)
			{
				VALTAN_PATTERN_SOUND_CUE_ROW_ID Created;
				std::string Status;
				if (m_pAnimationTool->Add_ValtanCompositionPatternSound(
						*pResourcePattern, *pResourceStage,
						pSoundTargetClip->strClipOccurrenceId,
						m_strSoundAddEvent, m_iSoundAddStartMs,
						m_eSoundAddRepeatPolicy, Created, Status))
				{
					m_eDetailOwner = DETAIL_OWNER::SOUND;
					m_strSelectedStableId = Created.strOccurrenceId;
					m_bDetailsWindowVisible = true;
					Invalidate_TimelineCache();
				}
				m_strSoundStatus = Status;
				m_strStatus = std::move(Status);
			}
			ImGui::EndDisabled();
			if (!bSoundAppendAdmitted && m_bAuthoringDraftDirty)
			{
				ImGui::TextDisabled(
					"Save & Apply Pattern/Animation/Effect changes before editing the separate Sound owner draft.");
			}
			ImGui::SeparatorText("Sound Event Tree");
			RenderResourceTree(
				m_SoundResourceTree,
				[this](const std::size_t i)
				{
					const std::string& Event = m_PatternSoundEvents[i];
					const bool_t bSelected = Event == m_strSoundAddEvent;
					if (ImGui::Selectable(Event.c_str(), bSelected))
					{
						m_strSoundAddEvent = Event;
						m_eDetailOwner = DETAIL_OWNER::SOUND;
						m_bDetailsWindowVisible = true;
					}
				});
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem(
				"Camera", nullptr,
				ResourceTabFlags(RESOURCE_DOMAIN::CAMERA)))
		{
			bResourceSelectionConsumed |= m_bResourceDomainSelectionRequested &&
				RESOURCE_DOMAIN::CAMERA == m_eRequestedResourceDomain;
			ImGui::TextDisabled(
				"Camera cue bodies remain owned by Camera Tool; exact Pattern invocations appear in Details.");
			if (ImGui::Button("Open Camera Tool"))
				m_bCameraToolOpenRequested = true;
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem(
				"Logic Templates", nullptr,
				ResourceTabFlags(RESOURCE_DOMAIN::LOGIC)))
		{
			bResourceSelectionConsumed |= m_bResourceDomainSelectionRequested &&
				RESOURCE_DOMAIN::LOGIC == m_eRequestedResourceDomain;
			ImGui::TextUnformatted("Counter Window");
			ImGui::TextDisabled("WINDUP -> COUNTER_HIT -> same-Pattern GROGGY");
			ImGui::BeginDisabled(
				nullptr == pResourcePattern || nullptr == pResourceStage);
			if (ImGui::Button("Open Counter -> Groggy Detail") &&
				nullptr != pResourcePattern && nullptr != pResourceStage)
			{
				m_bDetailsWindowVisible = true;
				Select_Stage(
					*pResourcePattern, *pResourceStage,
					DETAIL_OWNER::GAMEPLAY_STAGE,
					pResourceStage->strStageId +
						"/branch/COUNTER_HIT/authoring");
			}
			ImGui::EndDisabled();

			ImGui::SeparatorText("Portal / Charge Motion");
			const bool_t bHasMotion = nullptr != pResourceStage &&
				pResourceStage->Motion.has_value();
			ImGui::BeginDisabled(nullptr == pResourcePattern || !bHasMotion);
			if (ImGui::Button("Open Motion Detail") &&
				nullptr != pResourcePattern && bHasMotion)
			{
				m_bDetailsWindowVisible = true;
				Select_Stage(
					*pResourcePattern, *pResourceStage,
					DETAIL_OWNER::GAMEPLAY_STAGE,
					pResourceStage->strStageId + "/motion/" +
						pResourceStage->Motion->strKind);
			}
			ImGui::EndDisabled();

			ImGui::SeparatorText("Grab / Release");
			const auto Release = nullptr == pResourceStage ?
				std::vector<VALTAN_STAGE_ACTION_VIEW>::const_iterator{} :
				std::find_if(
					pResourceStage->Actions.begin(),
					pResourceStage->Actions.end(),
					[](const VALTAN_STAGE_ACTION_VIEW& Action)
					{ return "RELEASE_GRABBED_PLAYERS" == Action.strKind; });
			const bool_t bHasRelease = nullptr != pResourceStage &&
				Release != pResourceStage->Actions.end();
			ImGui::BeginDisabled(nullptr == pResourcePattern || !bHasRelease);
			if (ImGui::Button("Open Grab Release Detail") &&
				nullptr != pResourcePattern && bHasRelease)
			{
				m_bDetailsWindowVisible = true;
				Select_Stage(
					*pResourcePattern, *pResourceStage,
					DETAIL_OWNER::GAMEPLAY_STAGE,
					pResourceStage->strStageId + "/action/" +
						Release->strKind + "/" +
						Release->strTargetId);
			}
			ImGui::EndDisabled();
			ImGui::TextDisabled(
				"Only templates admitted by the selected Stage's typed gameplay schema are editable.");
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	if (bResourceSelectionConsumed)
		m_bResourceDomainSelectionRequested = false;
	ImGui::End();
}

void Client::CActionCompositionWorkbench::Render()
{
	m_bApplyResetLayoutThisFrame = m_bResetLayoutRequested;
	m_bResetLayoutRequested = false;
	if (!m_bLoadAttempted)
		(void)Reload_Canonical();
	if (nullptr != m_pAnimationTool)
	{
		std::string CreatedPatternId;
		if (m_pAnimationTool->Consume_ValtanCompositionPatternCreated(
				CreatedPatternId))
		{
			m_bPatternsWindowVisible = true;
			m_iRequestedPatternTab = 0;
			if (Reload_Canonical())
			{
				const auto FindCreated = [&CreatedPatternId](
					const std::vector<VALTAN_PATTERN_VIEW>& Patterns)
					-> const VALTAN_PATTERN_VIEW*
				{
					const auto Found = std::find_if(
						Patterns.begin(), Patterns.end(),
						[&CreatedPatternId](const VALTAN_PATTERN_VIEW& Pattern)
						{ return Pattern.strPatternId == CreatedPatternId; });
					return Found == Patterns.end() ? nullptr : &*Found;
				};
				const VALTAN_PATTERN_VIEW* pCreated =
					FindCreated(m_CanonicalView.Gimmicks);
				if (nullptr == pCreated)
					pCreated = FindCreated(m_CanonicalView.Rotation);
				if (nullptr != pCreated)
				{
					Select_Pattern(*pCreated);
					m_strStatus =
						"Create New Pattern source/Product/reload closure admitted and selected: " +
						CreatedPatternId + ".";
				}
				else
				{
					m_eAdmission = ADMISSION_STATE::STALE_PRESERVED;
					m_strStatus =
						"Create New Pattern reported an admitted closure, but the reloaded Workbench inventory did not contain " +
						CreatedPatternId + ". Mutations are blocked.";
				}
			}
		}
	}
	Normalize_Selection();
	const VALTAN_PATTERN_VIEW* const pSavedPattern = Find_SelectedPattern();
	std::string AuthoringRevision;
	std::string CanonicalSourceRevision;
	std::string AuthoringStatus;
	bool_t bAuthoringDirty = false;
	const bool_t bCanonicalSourceJoined = nullptr != m_pBalanceTool &&
		m_pBalanceTool->Get_ValtanCanonicalSourceRevision(
			CanonicalSourceRevision, AuthoringStatus) &&
		CanonicalSourceRevision == m_strPinnedCanonicalSourceRevision;
	const bool_t bAuthoringJoined = bCanonicalSourceJoined &&
		m_pBalanceTool->Get_ValtanAuthoringState(
			AuthoringRevision, bAuthoringDirty, AuthoringStatus) &&
		AuthoringRevision == m_strPinnedAuthoringSourceRevision;
	if (ADMISSION_STATE::ADMITTED == m_eAdmission && !bCanonicalSourceJoined)
	{
		m_eAdmission = ADMISSION_STATE::STALE_PRESERVED;
		m_bConfirmDiscardPatternSoundDraft = false;
		m_strStatus =
			"The Balance draft moved away from the Workbench canonical source pin; the previous composition is now read-only until Reload Canonical succeeds. " +
			AuthoringStatus;
	}
	m_bAuthoringDraftDirty = ADMISSION_STATE::ADMITTED == m_eAdmission &&
		bAuthoringJoined && bAuthoringDirty;
	bool_t bEffectivePatternReady = false;
	if (ADMISSION_STATE::ADMITTED == m_eAdmission && nullptr != pSavedPattern &&
		bAuthoringJoined)
	{
		const std::uint64_t iDraftGeneration =
			m_pBalanceTool->Get_ValtanDraftGeneration();
		const bool_t bCacheMatches = m_bEffectivePatternCacheReady &&
			m_strEffectivePatternCachePatternId == pSavedPattern->strPatternId &&
			m_strEffectivePatternCacheCanonicalRevision ==
				m_strPinnedCanonicalSourceRevision &&
			m_iEffectivePatternCacheDraftGeneration == iDraftGeneration;
		if (!bCacheMatches)
		{
			VALTAN_PATTERN_VIEW StagedEffectivePattern;
			if (m_pBalanceTool->Get_ValtanPatternDraft(
					pSavedPattern->strPatternId,
					StagedEffectivePattern, AuthoringStatus))
			{
				m_EffectivePatternCache = std::move(StagedEffectivePattern);
				m_strEffectivePatternCachePatternId =
					pSavedPattern->strPatternId;
				m_strEffectivePatternCacheCanonicalRevision =
					m_strPinnedCanonicalSourceRevision;
				m_iEffectivePatternCacheDraftGeneration = iDraftGeneration;
				m_bEffectivePatternCacheReady = true;
			}
			else
			{
				Invalidate_EffectivePatternCache();
			}
		}
		bEffectivePatternReady = m_bEffectivePatternCacheReady;
	}
	const VALTAN_PATTERN_VIEW* const pPattern = bEffectivePatternReady ?
		&m_EffectivePatternCache : pSavedPattern;
	/* This generation belongs to the immutable pPattern selected above.  A
	   Sequencer widget can advance the live Balance draft later in this frame,
	   but the Boss Pattern projection must not label this older view with that
	   newer generation. */
	const std::uint64_t iPatternViewDraftGeneration = bEffectivePatternReady ?
		m_iEffectivePatternCacheDraftGeneration : 0u;
	if (nullptr != pPattern)
	{
		if (pPattern->Stages.empty())
		{
			m_strSelectedStageId.clear();
		}
		else if (nullptr == Find_SelectedStage(pPattern))
		{
			m_strSelectedStageId = pPattern->Stages.front().strStageId;
			if (DETAIL_OWNER::PATTERN != m_eDetailOwner)
			{
				m_eDetailOwner = DETAIL_OWNER::GAMEPLAY_STAGE;
				m_strSelectedStableId = m_strSelectedStageId;
			}
		}
	}
	Ensure_TimelineCache(pPattern);
	const VALTAN_STAGE_VIEW* const pStage = Find_SelectedStage(pPattern);
	const bool_t bMutationAdmitted =
		ADMISSION_STATE::ADMITTED == m_eAdmission && bEffectivePatternReady;
	std::string SoundDependencyStatus;
	m_bPatternSoundDependencyDirty = false;
	if (ADMISSION_STATE::ADMITTED == m_eAdmission)
	{
		m_bPatternSoundDependencyDirty =
			Is_PatternSoundDraftDirty(SoundDependencyStatus);
	}
	if (!SoundDependencyStatus.empty())
		m_strSoundStatus = SoundDependencyStatus;
	const bool_t bPatternMutationAdmitted =
		bMutationAdmitted && !m_bPatternSoundDependencyDirty;

	/* MainApp's tool-level focus request is consumed by the first top-level
	   Composition window. Keep the Sequencer first because it is the primary
	   editing surface; all windows share this frame's immutable Pattern view. */
	Render_SequencerWindow(
		pPattern, bMutationAdmitted, bPatternMutationAdmitted);
	if (Render_SessionWindow(
			pPattern, pStage, bPatternMutationAdmitted))
	{
		/* Save/reload replaced the canonical storage. The next frame resolves a
		   fresh immutable Pattern/Stage view before any other domain window. */
		if (m_bApplyResetLayoutThisFrame)
			m_bResetLayoutRequested = true;
		m_bApplyResetLayoutThisFrame = false;
		return;
	}
	Render_BossPatternWindow(
		pPattern, iPatternViewDraftGeneration,
		bMutationAdmitted, bPatternMutationAdmitted);
	Render_PatternsWindow(
		pPattern, pStage, bPatternMutationAdmitted);
	Render_PreviewWindow(pPattern, bMutationAdmitted);
	Render_DetailsWindow(
		pPattern, pStage, bMutationAdmitted, bPatternMutationAdmitted);
	Render_ResourcesWindow(
		pPattern, pStage, bMutationAdmitted, bPatternMutationAdmitted);
	if (m_bSavePatternRequested)
	{
		/* Details only queues this command.  All windows finish using this
		   frame's immutable Pattern/Stage views before a successful save reloads
		   and replaces the canonical storage. */
		m_bSavePatternRequested = false;
		m_bPatternSaveSucceeded = Save_Publish_Reload();
		m_bPatternSaveResultAvailable = true;
		m_strPatternSaveStatus = m_strStatus;
	}
	m_bApplyResetLayoutThisFrame = false;
}

bool_t Client::CActionCompositionWorkbench::Consume_EffectToolOpenRequest(
	EFFECT_TOOL_VALTAN_PRODUCT_OPEN_REQUEST& OutRequest)
{
	if (!m_bEffectToolOpenRequested)
		return false;
	OutRequest.strPatternId = std::move(m_strEffectPatternId);
	OutRequest.strStageId = std::move(m_strEffectStageId);
	OutRequest.strCueOccurrenceId = std::move(m_strEffectOccurrenceId);
	OutRequest.strEffectAssetId = std::move(m_strEffectAssetId);
	m_bEffectToolOpenRequested = false;
	return true;
}

bool_t Client::CActionCompositionWorkbench::Consume_CameraToolOpenRequest(
	CAMERA_TOOL_OPEN_REQUEST& OutRequest)
{
	if (!m_bCameraToolOpenRequested)
		return false;
	OutRequest.strCueId = std::move(m_strCameraCueId);
	m_bCameraToolOpenRequested = false;
	return true;
}

bool_t Client::CActionCompositionWorkbench::Consume_AnimationToolOpenRequest()
{
	const bool_t bRequested = m_bOpenAnimationToolRequested;
	m_bOpenAnimationToolRequested = false;
	return bRequested;
}

bool_t Client::CActionCompositionWorkbench::Consume_PreviewOwnerClaimRequest()
{
	const bool_t bRequested = m_bPreviewOwnerClaimRequested;
	m_bPreviewOwnerClaimRequested = false;
	return bRequested;
}
