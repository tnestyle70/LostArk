#include "ValtanPatternTree.h"

#include "DataJson.h"
#include "ProjectDataRoot.h"

#include <algorithm>
#include <fstream>
#include <map>

namespace
{
	using Client::DATA_JSON_TYPE;
	using Client::DATA_JSON_VALUE;

	bool Read_TextDocument(
		const std::filesystem::path& Path,
		std::string& OutText,
		std::string& strOutError)
	{
		if (Path.empty())
		{
			strOutError = "path escaped the Data root";
			return false;
		}
		std::ifstream Input(Path, std::ios::binary);
		if (!Input)
		{
			strOutError = "could not open " + Path.string();
			return false;
		}
		OutText.assign(std::istreambuf_iterator<char>(Input),
			std::istreambuf_iterator<char>());
		return true;
	}

	bool Parse_Document(
		const std::filesystem::path& Relative,
		DATA_JSON_VALUE& OutRoot,
		std::string& strOutError)
	{
		std::string Text;
		if (!Read_TextDocument(
				Client::CProjectDataRoot::Resolve(Relative), Text, strOutError))
		{
			return false;
		}
		std::string ParseError;
		if (!Client::CDataJson::Parse(Text, OutRoot, ParseError) ||
			!OutRoot.Is_Object())
		{
			strOutError = Relative.generic_string() + ": " + ParseError;
			return false;
		}
		return true;
	}

	std::string Read_String(
		const DATA_JSON_VALUE& Object, const std::string_view Key)
	{
		const DATA_JSON_VALUE* pValue = Object.Find(Key);
		return nullptr == pValue || DATA_JSON_TYPE::STRING != pValue->Get_Type() ?
			std::string{} : pValue->Get_String();
	}

	double Read_Number(
		const DATA_JSON_VALUE& Object, const std::string_view Key)
	{
		const DATA_JSON_VALUE* pValue = Object.Find(Key);
		return nullptr == pValue || DATA_JSON_TYPE::NUMBER != pValue->Get_Type() ?
			0.0 : pValue->Get_Number();
	}
}

size_t Client::VALTAN_PATTERN_TREE_VIEW::Get_StageCount() const
{
	size_t iCount = 0u;
	for (const VALTAN_PATTERN_VIEW& Pattern : Gimmicks)
		iCount += Pattern.Stages.size();
	for (const VALTAN_PATTERN_VIEW& Pattern : Rotation)
		iCount += Pattern.Stages.size();
	return iCount;
}

size_t Client::VALTAN_PATTERN_TREE_VIEW::Get_EffectCount() const
{
	size_t iCount = 0u;
	const auto Count = [&iCount](const std::vector<VALTAN_PATTERN_VIEW>& Group)
	{
		for (const VALTAN_PATTERN_VIEW& Pattern : Group)
		{
			iCount += static_cast<size_t>(std::count_if(
				Pattern.Stages.begin(), Pattern.Stages.end(),
				[](const VALTAN_STAGE_VIEW& Stage)
				{
					return Stage.Has_Effect();
				}));
		}
	};
	Count(Gimmicks);
	Count(Rotation);
	return iCount;
}

size_t Client::VALTAN_PATTERN_TREE_VIEW::Get_EffectDocumentCount() const
{
	size_t iCount = 0u;
	const auto Count = [&iCount](const std::vector<VALTAN_PATTERN_VIEW>& Group)
	{
		for (const VALTAN_PATTERN_VIEW& Pattern : Group)
		{
			for (const VALTAN_STAGE_VIEW& Stage : Pattern.Stages)
				iCount += Stage.Effects.size();
		}
	};
	Count(Gimmicks);
	Count(Rotation);
	return iCount;
}

std::string Client::CValtanPatternTree::Build_StageEffectAssetId(
	const std::string& strPatternActionId,
	const VALTAN_STAGE_VIEW& Stage)
{
	if (strPatternActionId.empty() || Stage.strActionId.empty())
		return {};
	/* valtan.attack.whirlwind -> whirlwind; the category is what the tree
	   already shows, so it is not repeated inside the asset id. */
	std::string strPattern = strPatternActionId;
	size_t iCut = strPattern.find('.');
	if (std::string::npos != iCut)
	{
		iCut = strPattern.find('.', iCut + 1u);
		if (std::string::npos != iCut)
			strPattern = strPattern.substr(iCut + 1u);
	}
	const std::string strPrefix = strPatternActionId + ".";
	std::string strStage;
	if (Stage.strActionId.starts_with(strPrefix) &&
		Stage.strActionId.size() > strPrefix.size())
	{
		strStage = Stage.strActionId.substr(strPrefix.size());
	}
	else
	{
		strStage = Stage.strStageId;
		std::transform(strStage.begin(), strStage.end(), strStage.begin(),
			[](const unsigned char Character)
			{
				return '_' == Character ? '-' :
					static_cast<char>(std::tolower(Character));
			});
	}
	if (strPattern.empty() || strStage.empty())
		return {};
	return "effect.valtan." + strPattern + "." + strStage;
}

bool_t Client::CValtanPatternTree::Load(
	VALTAN_PATTERN_TREE_VIEW& OutView,
	std::string& strOutStatus)
{
	DATA_JSON_VALUE Encounter;
	DATA_JSON_VALUE Bindings;
	std::string Error;
	if (!Parse_Document(std::filesystem::path(L"Encounters") / L"Valtan" /
			L"ValtanEncounter.json", Encounter, Error))
	{
		strOutStatus = "Valtan encounter load failed: " + Error;
		return false;
	}
	if (!Parse_Document(std::filesystem::path(L"Animation") / L"Authored" /
			L"Valtan" / L"Valtan.patternbindings.json", Bindings, Error))
	{
		strOutStatus = "Valtan pattern bindings load failed: " + Error;
		return false;
	}

	/* Effect bindings are optional: a freshly seeded stage document is not in
	   patterneffects.json yet, and that must not fail the whole tree. */
	std::map<std::string, std::pair<std::string, std::string>, std::less<>>
		EffectByAction;
	DATA_JSON_VALUE Effects;
	if (Parse_Document(std::filesystem::path(L"Animation") / L"Authored" /
			L"Valtan" / L"Valtan.patterneffects.json", Effects, Error))
	{
		const DATA_JSON_VALUE* pRows = Effects.Find("bindings");
		if (nullptr != pRows && pRows->Is_Array())
		{
			for (const DATA_JSON_VALUE& Row : pRows->Get_Array())
			{
				if (!Row.Is_Object())
					continue;
				const std::string strAction = Read_String(Row, "actionId");
				if (strAction.empty())
					continue;
				EffectByAction[strAction] = {
					Read_String(Row, "effectAssetId"),
					Read_String(Row, "effectDocument") };
			}
		}
	}

	std::map<std::string, std::string, std::less<>> ClipByAction;
	const DATA_JSON_VALUE* pBindingRows = Bindings.Find("bindings");
	if (nullptr == pBindingRows || !pBindingRows->Is_Array())
	{
		strOutStatus = "Valtan pattern bindings has no bindings array.";
		return false;
	}
	for (const DATA_JSON_VALUE& Row : pBindingRows->Get_Array())
	{
		if (!Row.Is_Object())
			continue;
		const std::string strAction = Read_String(Row, "actionId");
		if (!strAction.empty())
			ClipByAction[strAction] = Read_String(Row, "clip");
	}

	const DATA_JSON_VALUE* pPatterns = Encounter.Find("patterns");
	if (nullptr == pPatterns || !pPatterns->Is_Array())
	{
		strOutStatus = "Valtan encounter has no patterns array.";
		return false;
	}

	VALTAN_PATTERN_TREE_VIEW Staged;
	for (const DATA_JSON_VALUE& PatternValue : pPatterns->Get_Array())
	{
		if (!PatternValue.Is_Object())
			continue;
		VALTAN_PATTERN_VIEW Pattern;
		Pattern.strPatternId = Read_String(PatternValue, "patternId");
		Pattern.strDisplayName = Read_String(PatternValue, "displayName");
		Pattern.strActionId = Read_String(PatternValue, "actionId");
		Pattern.iMinimumHealthBar = static_cast<int32_t>(
			Read_Number(PatternValue, "minimumHealthBar"));
		Pattern.iMaximumHealthBar = static_cast<int32_t>(
			Read_Number(PatternValue, "maximumHealthBar"));
		Pattern.iTriggerHealthBar = static_cast<int32_t>(
			Read_Number(PatternValue, "triggerHealthBar"));
		if (Pattern.strPatternId.empty())
			continue;

		const DATA_JSON_VALUE* pStages = PatternValue.Find("stages");
		if (nullptr != pStages && pStages->Is_Array())
		{
			for (const DATA_JSON_VALUE& StageValue : pStages->Get_Array())
			{
				if (!StageValue.Is_Object())
					continue;
				VALTAN_STAGE_VIEW Stage;
				Stage.strStageId = Read_String(StageValue, "stageId");
				Stage.strActionId = Read_String(StageValue, "actionId");
				Stage.strStageKind = Read_String(StageValue, "stageKind");
				Stage.iDurationMs = static_cast<uint32_t>(
					Read_Number(StageValue, "durationMs"));
				Stage.strHitShape = Read_String(StageValue, "hitShape");
				Stage.fHitOuterRadius = static_cast<f32_t>(
					Read_Number(StageValue, "hitOuterRadius"));
				Stage.fHitInnerRadius = static_cast<f32_t>(
					Read_Number(StageValue, "hitInnerRadius"));
				Stage.fHitAngleDegrees = static_cast<f32_t>(
					Read_Number(StageValue, "hitAngleDegrees"));
				Stage.fHitLength = static_cast<f32_t>(
					Read_Number(StageValue, "hitLength"));
				Stage.fHitHalfWidth = static_cast<f32_t>(
					Read_Number(StageValue, "hitHalfWidth"));
				Stage.iHitCount = static_cast<uint32_t>(
					Read_Number(StageValue, "hitCount"));
				Stage.iHitIntervalMs = static_cast<uint32_t>(
					Read_Number(StageValue, "hitIntervalMs"));
				Stage.strServerDamageProfileId = Read_String(
					StageValue, "serverDamageProfileId");
				if (Stage.strActionId.empty())
					continue;

				const auto ClipIterator = ClipByAction.find(Stage.strActionId);
				if (ClipIterator != ClipByAction.end())
					Stage.strRuntimeClipName = ClipIterator->second;

				const auto EffectIterator =
					EffectByAction.find(Stage.strActionId);
				if (EffectIterator != EffectByAction.end() &&
					!EffectIterator->second.first.empty() &&
					!EffectIterator->second.second.empty())
				{
					VALTAN_STAGE_EFFECT_VIEW Bound;
					Bound.strEffectAssetId = EffectIterator->second.first;
					Bound.DocumentPath = CProjectDataRoot::Get() /
						std::filesystem::path(
							EffectIterator->second.second).lexically_normal();
					Bound.eOrigin =
						VALTAN_STAGE_EFFECT_ORIGIN::PATTERN_EFFECT_BINDING;
					Stage.Effects.push_back(std::move(Bound));
				}
				/* A stage document seeded by the generator is not in
				   patterneffects.json and never will be, because that schema
				   requires per-binding source evidence. The naming rule is
				   therefore always checked too, and both documents stay
				   visible instead of one hiding the other. */
				const std::string strCandidate = Build_StageEffectAssetId(
					Pattern.strActionId, Stage);
				const bool_t bCandidateAlreadyBound = std::any_of(
					Stage.Effects.begin(), Stage.Effects.end(),
					[&strCandidate](const VALTAN_STAGE_EFFECT_VIEW& Effect)
					{
						return Effect.strEffectAssetId == strCandidate;
					});
				if (!strCandidate.empty() && !bCandidateAlreadyBound)
				{
					const std::filesystem::path Candidate =
						CProjectDataRoot::Resolve(
							std::filesystem::path(L"Effects") / L"Authored" /
							std::filesystem::path(
								strCandidate + ".effect.json"));
					std::error_code FileError;
					if (!Candidate.empty() && std::filesystem::exists(
							Candidate, FileError))
					{
						VALTAN_STAGE_EFFECT_VIEW Seeded;
						Seeded.strEffectAssetId = strCandidate;
						Seeded.DocumentPath = Candidate;
						Seeded.eOrigin = VALTAN_STAGE_EFFECT_ORIGIN::NAMING_RULE;
						Stage.Effects.push_back(std::move(Seeded));
					}
				}
				Pattern.Stages.push_back(std::move(Stage));
			}
		}
		if (Pattern.Is_Gimmick())
			Staged.Gimmicks.push_back(std::move(Pattern));
		else
			Staged.Rotation.push_back(std::move(Pattern));
	}

	if (Staged.Gimmicks.empty() && Staged.Rotation.empty())
	{
		strOutStatus = "Valtan encounter produced no patterns.";
		return false;
	}
	/* Gimmicks read as a timeline down the health bar, so order them the way
	   the fight actually presents them. */
	std::sort(Staged.Gimmicks.begin(), Staged.Gimmicks.end(),
		[](const VALTAN_PATTERN_VIEW& Left, const VALTAN_PATTERN_VIEW& Right)
		{
			return Left.iTriggerHealthBar > Right.iTriggerHealthBar;
		});

	/* A phase is the band of health bars that ends when its gimmick fires.
	   The encounter document has no phase field, so this band is derived from
	   triggerHealthBar and used only as a display grouping. */
	int32_t iTopHealthBar = 0;
	const auto RaiseTop = [&iTopHealthBar](
		const std::vector<VALTAN_PATTERN_VIEW>& Group)
	{
		for (const VALTAN_PATTERN_VIEW& Pattern : Group)
		{
			iTopHealthBar = (std::max)(iTopHealthBar,
				(std::max)(Pattern.iMaximumHealthBar, Pattern.iTriggerHealthBar));
		}
	};
	RaiseTop(Staged.Gimmicks);
	RaiseTop(Staged.Rotation);

	uint32_t iPhaseNumber = 0u;
	int32_t iBandTop = iTopHealthBar;
	for (size_t iGimmick = 0u; iGimmick < Staged.Gimmicks.size(); ++iGimmick)
	{
		const VALTAN_PATTERN_VIEW& Gimmick = Staged.Gimmicks[iGimmick];
		VALTAN_PHASE_VIEW Phase;
		Phase.iPhaseNumber = ++iPhaseNumber;
		Phase.iBandTopHealthBar = (std::max)(iBandTop, Gimmick.iTriggerHealthBar);
		Phase.iBandBottomHealthBar = Gimmick.iTriggerHealthBar;
		Phase.strGatePatternId = Gimmick.strPatternId;
		Phase.iGateTriggerHealthBar = Gimmick.iTriggerHealthBar;
		Phase.GimmickIndices.push_back(iGimmick);
		Staged.Phases.push_back(std::move(Phase));
		iBandTop = Gimmick.iTriggerHealthBar - 1;
	}
	if (iBandTop >= 1)
	{
		VALTAN_PHASE_VIEW Phase;
		Phase.iPhaseNumber = ++iPhaseNumber;
		Phase.iBandTopHealthBar = iBandTop;
		Phase.iBandBottomHealthBar = 1;
		Staged.Phases.push_back(std::move(Phase));
	}

	/* A rotation pattern belongs to every band its own bar range overlaps.
	   Most are 1-160 and therefore appear in all of them; two are narrower
	   and that is exactly the information the band grouping exposes. */
	for (VALTAN_PHASE_VIEW& Phase : Staged.Phases)
	{
		for (size_t iRotation = 0u; iRotation < Staged.Rotation.size();
			++iRotation)
		{
			const VALTAN_PATTERN_VIEW& Rotation = Staged.Rotation[iRotation];
			if (Rotation.iMinimumHealthBar > Phase.iBandTopHealthBar ||
				Rotation.iMaximumHealthBar < Phase.iBandBottomHealthBar)
			{
				continue;
			}
			Phase.RotationIndices.push_back(iRotation);
		}
	}

	const std::string strIntroPatternId = Read_String(
		Encounter, "introPatternId");
	if (!strIntroPatternId.empty())
	{
		const auto Intro = std::find_if(
			Staged.Rotation.begin(), Staged.Rotation.end(),
			[&strIntroPatternId](const VALTAN_PATTERN_VIEW& Pattern)
			{
				return Pattern.strPatternId == strIntroPatternId;
			});
		if (Intro != Staged.Rotation.end())
		{
			Staged.iIntroRotationIndex = static_cast<size_t>(
				std::distance(Staged.Rotation.begin(), Intro));
		}
	}

	OutView = std::move(Staged);
	strOutStatus = "Valtan tree: " +
		std::to_string(OutView.Phases.size()) + " phases, " +
		std::to_string(OutView.Get_PatternCount()) + " patterns, " +
		std::to_string(OutView.Get_StageCount()) + " stages, " +
		std::to_string(OutView.Get_EffectCount()) + " with an Effect (" +
		std::to_string(OutView.Get_EffectDocumentCount()) + " documents).";
	return true;
}
