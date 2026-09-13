#include "imgui.h"
#include "Animation_Tool_Internal.h"
#include "ActionPresentationTimeline.h"
#include "ActorCatalog.h"
#include "AnimationPreviewAssets.h"
#include "Character.h"
#include "Model.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"
#include "SoundCueCatalog.h"
#include <charconv>
#include <algorithm>
#include <array>
#include <cerrno>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <io.h>
#include <iomanip>
#include <iterator>
#include <limits>
#include <map>
#include <sstream>
#include <span>
#include <system_error>
#include <tuple>
#include <unordered_map>
#include <unordered_set>


namespace AnimationToolDetail
{

	/* Large-named Saydon actions preview at the size the running exe admitted
	for the big Saydon boss: the catalog bodyModelPreScale over the MN_RPCT_06
	preview admission scale, so the preview matches the spawned boss. The
	catalog is required; invalid input preserves the current preview scale. */
	f32_t Large_SaydonPreviewMultiplier()
	{
		const Client::BOSS_ACTOR_ENTRY* pBigSaydon =
			Client::CActorCatalog::Find_Boss("BOSS_KAKULSAYDON_G2_BIG_SAYDON");
		const auto asset = std::find_if(
			Client::ANIMATION_PREVIEW_ASSETS.begin(), Client::ANIMATION_PREVIEW_ASSETS.end(),
			[](const Client::ANIMATION_PREVIEW_ASSET& candidate)
			{
				return nullptr != candidate.pAssetName &&
					std::string_view(candidate.pAssetName) == "MN_RPCT_06";
			});
		if (nullptr == pBigSaydon || asset == Client::ANIMATION_PREVIEW_ASSETS.end() ||
			!std::isfinite(pBigSaydon->bodyModelPreScale) ||
			pBigSaydon->bodyModelPreScale <= 0.f || asset->fPreviewScale <= 0.f)
		{
			OutputDebugStringA("[KoukuPreview] Big Saydon catalog scale is unavailable; preview scale preserved.\n");
			return 0.f;
		}
		return pBigSaydon->bodyModelPreScale / asset->fPreviewScale;
	}

	f32_t Resolve_LargeNamePreviewScale(const std::string_view name)
	{
		if (name.find("\xEB\x8C\x80\xED\x98\x95") != std::string_view::npos)
			return Large_SaydonPreviewMultiplier();
		std::string lower(name);
		std::transform(lower.begin(), lower.end(), lower.begin(), [](const char c) {
			return c >= 'A' && c <= 'Z' ? static_cast<char>(c + ('a' - 'A')) : c;
		});
		return lower.find("large") != std::string::npos ? Large_SaydonPreviewMultiplier() : 1.f;
	}

	f32_t Resolve_ActionPreviewScale(
		const Client::KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT& reference,
		const std::uint32_t sourceActionId)
	{
		const auto action = std::find_if(reference.Actions.begin(), reference.Actions.end(),
			[sourceActionId](const auto& candidate) { return candidate.iSourceActionId == sourceActionId; });
		return action != reference.Actions.end() ? Resolve_LargeNamePreviewScale(action->strDisplayName) : 1.f;
	}

	bool_t Is_LowerSha256(const std::string_view strValue)
	{
		return 64u == strValue.size() &&
			std::all_of(strValue.begin(), strValue.end(),
				[](const char_t Character)
				{
					return (Character >= '0' && Character <= '9') ||
						(Character >= 'a' && Character <= 'f');
				});
	}

	bool_t Is_StablePatternAuthoringId(const std::string_view strValue)
	{
		return !strValue.empty() && strValue.size() <= 160u &&
			std::all_of(strValue.begin(), strValue.end(),
				[](const char_t Character)
				{
					return (Character >= 'a' && Character <= 'z') ||
						(Character >= 'A' && Character <= 'Z') ||
						(Character >= '0' && Character <= '9') ||
						Character == '_' || Character == '-' || Character == '.';
				});
	}

	bool_t Read_BoundedFile(
		const std::filesystem::path& Path,
		const std::uintmax_t iMaximumBytes,
		std::string& strOutBytes,
		std::string& strOutError)
	{
		std::error_code Error;
		const std::uintmax_t iSize = std::filesystem::file_size(Path, Error);
		if (Error || iSize > iMaximumBytes)
		{
			strOutError = Error ?
				"could not inspect " + Path.string() + ": " + Error.message() :
				"file exceeds the bounded diagnostic/input limit: " + Path.string();
			return false;
		}
		std::ifstream Stream(Path, std::ios::binary);
		if (!Stream)
		{
			strOutError = "could not open " + Path.string();
			return false;
		}
		strOutBytes.resize(static_cast<std::size_t>(iSize));
		if (0u != iSize && !Stream.read(
			strOutBytes.data(), static_cast<std::streamsize>(iSize)))
		{
			strOutError = "could not read " + Path.string();
			return false;
		}
		return true;
	}

	bool_t Resolve_PythonExecutable(
		std::filesystem::path& Out,
		std::string& strOutError)
	{
		std::vector<wchar_t> Buffer(32768u, L'\0');
		const DWORD iLength = SearchPathW(
			nullptr, L"python.exe", nullptr,
			static_cast<DWORD>(Buffer.size()), Buffer.data(), nullptr);
		if (0u == iLength || iLength >= Buffer.size())
		{
			strOutError =
				"python.exe was not found through the developer process PATH.";
			return false;
		}
		/* SearchPathW already returns an absolute executable path.  Do not ask
		   std::filesystem to resolve the target: WindowsApps App Execution Alias
		   reparse points can fail weakly_canonical() even though CreateProcessW
		   launches them correctly. */
		Out = std::filesystem::path(Buffer.data()).lexically_normal();
		if (Out.empty())
		{
			strOutError =
				"the resolved Python executable path is empty.";
			Out.clear();
			return false;
		}
		/* Microsoft Store Python is exposed through WindowsApps as a zero-byte
		   App Execution Alias reparse point.  It is a valid CreateProcessW
		   target, but std::filesystem::is_regular_file() reports false and used
		   to reject every Create Pattern request before the backend started. */
		const DWORD iAttributes = GetFileAttributesW(Out.c_str());
		if (INVALID_FILE_ATTRIBUTES == iAttributes ||
			0u != (iAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			strOutError =
				"the resolved Python executable is missing or is a directory.";
			Out.clear();
			return false;
		}
		return true;
	}

	std::string_view ValtanSoundBankForEvent(
		const std::string_view strSoundEvent)
	{
		if (strSoundEvent.starts_with("G_Voltan1_"))
			return "S_Mob_G_Voltan1";
		if (strSoundEvent.starts_with("G_Voltan2_"))
			return "S_Mob_G_Voltan2";
		return {};
	}

	bool_t IsValtanSoundAuthoringCandidate(const std::string& strSoundEvent)
	{
		if (ValtanSoundBankForEvent(strSoundEvent).empty())
			return false;
		const std::vector<std::string>& Variants =
			Client::CSoundCueCatalog::Find_Variants("Valtan", strSoundEvent);
		if (Variants.empty())
			return false;
		for (const std::string& strAssetId : Variants)
		{
			const std::filesystem::path Path =
				Client::CRuntimeAssetRoot::Resolve(strAssetId);
			std::error_code Error;
			if (Path.empty() ||
				!std::filesystem::is_regular_file(Path, Error) || Error)
			{
				return false;
			}
		}
		return true;
	}

	std::unordered_map<std::string, f32_t>
	CollectModelClipSourceDurationSeconds(
		const shared_ptr<Engine::CModel>& pModel)
	{
		std::unordered_map<std::string, f32_t> durations;
		if (nullptr == pModel)
			return durations;
		const std::uint32_t animationCount = pModel->Get_NumAnimations();
		durations.reserve(animationCount);
		for (std::uint32_t animation = 0u;
			animation < animationCount; ++animation)
		{
			const char_t* const clipName =
				pModel->Get_AnimationName(animation);
			const f32_t ticksPerSecond =
				pModel->Get_AnimationTickPerSecond(animation);
			f32_t positionTicks = 0.f;
			f32_t durationTicks = 0.f;
			if (nullptr == clipName || '\0' == clipName[0] ||
				ticksPerSecond <= 0.f ||
				!pModel->Get_AnimationProgress(
					animation, positionTicks, durationTicks) ||
				durationTicks <= 0.f)
			{
				continue;
			}
			const f32_t sourceDurationSeconds =
				durationTicks / ticksPerSecond;
			if (std::isfinite(sourceDurationSeconds) &&
				sourceDurationSeconds > 0.f)
			{
				durations.emplace(clipName, sourceDurationSeconds);
			}
		}
		return durations;
	}

	bool_t BuildStrictValtanNativeClipInventory(
		const std::vector<Client::COMPOSITION_ANIMATION_RESOURCE>& resources,
		VALTAN_NATIVE_CLIP_INVENTORY& Out,
		std::string& Status)
	{
		Out.clear();
		for (const auto& resource : resources)
		{
			if (resource.strTargetAssetName != "Valtan") continue;
			uint32_t roundedDurationMs = 0u;
			if (!Is_StablePatternAuthoringId(resource.strRuntimeClip) ||
				!Client::CActionPresentationTimeline::Validate_AuthoredSourceWindow(
					resource.fDurationTicks, resource.fTicksPerSecond,
					0u, 0u, 1.f, roundedDurationMs) || roundedDurationMs == 0u)
			{
				Status = "Valtan physical Animation metadata is invalid: " + resource.strRuntimeClip + ".";
				Out.clear();
				return false;
			}
			if (!Out.emplace(resource.strRuntimeClip, VALTAN_NATIVE_CLIP_TIMING{
				resource.fDurationTicks, resource.fTicksPerSecond }).second)
			{
				Status = "Valtan physical Animation metadata has an ambiguous clip: " + resource.strRuntimeClip + ".";
				Out.clear();
				return false;
			}
		}
		if (Out.empty())
		{
			Status = "Valtan physical Animation metadata is unavailable; refresh Animation Resources.";
			return false;
		}
		return true;
	}

	bool_t SameValtanAnimationAuthoringSignature(
		const Client::VALTAN_STAGE_VIEW& Left,
		const Client::VALTAN_STAGE_VIEW& Right)
	{
		if (Left.strSequenceRole != Right.strSequenceRole ||
			Left.bSuppressAnimation != Right.bSuppressAnimation ||
			Left.ClipOccurrences.size() != Right.ClipOccurrences.size())
		{
			return false;
		}
		return std::equal(
			Left.ClipOccurrences.begin(), Left.ClipOccurrences.end(),
			Right.ClipOccurrences.begin(),
			[](const Client::VALTAN_CLIP_OCCURRENCE_VIEW& A,
				const Client::VALTAN_CLIP_OCCURRENCE_VIEW& B)
			{
				return A.strClipOccurrenceId == B.strClipOccurrenceId &&
					A.strClipName == B.strClipName &&
					A.strMappingBasis == B.strMappingBasis &&
					A.iSourceStartMs == B.iSourceStartMs &&
					A.iPlayMs == B.iPlayMs &&
					A.fPlayRate == B.fPlayRate &&
					A.bLoop == B.bLoop;
			});
	}

	bool_t ValidateValtanStageNativeAnimationWindows(
		const Client::VALTAN_STAGE_VIEW& Stage,
		const VALTAN_NATIVE_CLIP_INVENTORY& Inventory,
		std::string& Status)
	{
		if ("WAIT" == Stage.strSequenceRole)
		{
			if (!Stage.bSuppressAnimation || !Stage.ClipOccurrences.empty())
			{
				Status = "WAIT Stage " + Stage.strStageId +
					" must remain Animation NONE. Create or select an ACTIVE Stage before assigning a Sequence.";
				return false;
			}
			return true;
		}
		if (Stage.bSuppressAnimation)
		{
			if (!Stage.ClipOccurrences.empty())
			{
				Status = "Animation NONE Stage " + Stage.strStageId +
					" cannot retain clip occurrences.";
				return false;
			}
			return true;
		}
		if (Stage.ClipOccurrences.empty())
		{
			Status = "Animation Stage " + Stage.strStageId +
				" has no native clip occurrence.";
			return false;
		}
		for (const Client::VALTAN_CLIP_OCCURRENCE_VIEW& Occurrence :
			Stage.ClipOccurrences)
		{
			const auto Native = Inventory.find(Occurrence.strClipName);
			if (Native == Inventory.end())
			{
				Status = "Animation Stage " + Stage.strStageId +
					" references a clip absent from the physical Valtan Animation catalog: " +
					Occurrence.strClipName + ".";
				return false;
			}
			uint32_t iRoundedRemainingMs = 0u;
			if (!Client::CActionPresentationTimeline::
					Validate_AuthoredSourceWindow(
						Native->second.fDurationTicks,
						Native->second.fTicksPerSecond,
						Occurrence.iSourceStartMs,
						Occurrence.iPlayMs,
						Occurrence.fPlayRate,
						iRoundedRemainingMs))
			{
				Status = "Animation Stage " + Stage.strStageId + " occurrence " +
					Occurrence.strClipOccurrenceId +
					" escapes the admitted native source window for " +
					Occurrence.strClipName + ".";
				return false;
			}
		}
		return true;
	}

	const CUSTOM_CHAIN_PROFILE* Find_CustomChainProfile(
		const std::string& strAssetName)
	{
		for (const CUSTOM_CHAIN_PROFILE& Profile : CUSTOM_CHAIN_PROFILES)
		{
			if (strAssetName == Profile.pAssetName)
				return &Profile;
		}
		return nullptr;
	}

	const KOUKU_SAYDON_ACTION_PROFILE_CONTRACT* Find_KoukuSaydonActionProfile(
		const std::string_view strProfileId)
	{
		const auto Found = std::find_if(
			KOUKU_SAYDON_ACTION_PROFILES.begin(), KOUKU_SAYDON_ACTION_PROFILES.end(),
			[strProfileId](const KOUKU_SAYDON_ACTION_PROFILE_CONTRACT& Candidate)
			{
				return strProfileId == Candidate.pProfileId;
			});
		return Found == KOUKU_SAYDON_ACTION_PROFILES.end() ? nullptr : &*Found;
	}

	const KOUKU_SAYDON_ACTION_PROFILE_CONTRACT* Default_KoukuSaydonActionProfileForAsset(
		const std::string_view strAssetName)
	{
		/* MN_RPCT_05 is the default for the shared 05/07 physical body. A
		   Resource Files request can still select the exact 07 alias. */
		for (const KOUKU_SAYDON_ACTION_PROFILE_CONTRACT& Profile : KOUKU_SAYDON_ACTION_PROFILES)
		{
			if (strAssetName == Profile.pPreviewAssetName)
				return &Profile;
		}
		return nullptr;
	}

	const Client::VALTAN_PATTERN_VIEW* Find_ValtanPatternMaster(
		const Client::VALTAN_PATTERN_TREE_VIEW& View,
		const std::string_view strPatternId)
	{
		const auto FindIn = [strPatternId](
			const std::vector<Client::VALTAN_PATTERN_VIEW>& Patterns)
			-> const Client::VALTAN_PATTERN_VIEW*
		{
			const auto Pattern = std::find_if(
				Patterns.begin(), Patterns.end(),
				[strPatternId](const Client::VALTAN_PATTERN_VIEW& Candidate)
				{
					return Candidate.bAuthoringMasterManaged &&
						Candidate.strPatternId == strPatternId;
				});
			return Pattern == Patterns.end() ? nullptr : &*Pattern;
		};
		if (const Client::VALTAN_PATTERN_VIEW* pPattern = FindIn(View.Rotation))
			return pPattern;
		return FindIn(View.Gimmicks);
	}

	bool_t Try_ResolveValtanArenaPatternAction(
		const std::string_view strStageKind,
		LostArk::Shared::WORLD_ENTITY_ACTION& eOutAction)
	{
		using LostArk::Shared::WORLD_ENTITY_ACTION;
		if ("WINDUP" == strStageKind)
			eOutAction = WORLD_ENTITY_ACTION::PATTERN_WINDUP;
		else if ("ACTIVE" == strStageKind || "GROGGY" == strStageKind)
			eOutAction = WORLD_ENTITY_ACTION::PATTERN_ACTIVE;
		else if ("RECOVERY" == strStageKind || "PART_BREAK" == strStageKind)
			eOutAction = WORLD_ENTITY_ACTION::PATTERN_RECOVERY;
		else
			return false;
		return true;
	}

	void Skip_Space(const char_t*& p)
	{
		while (' ' == *p || '\t' == *p)
			++p;
	}

	bool_t Read_Quoted(const char_t*& p, std::string& out)
	{
		Skip_Space(p);
		if ('\"' != *p)
			return false;
		++p;
		out.clear();
		while ('\0' != *p && '\"' != *p)
			out.push_back(*p++);
		if ('\"' != *p)
			return false;
		++p;
		return true;
	}

	bool_t Read_Token(const char_t*& p, std::string& out)
	{
		Skip_Space(p);
		out.clear();
		while ('\0' != *p && ' ' != *p && '\t' != *p && '\r' != *p && '\n' != *p)
			out.push_back(*p++);
		return !out.empty();
	}

	/* key=value, where value may be quoted so paths with spaces survive. */
	bool_t Read_Pair(const char_t*& p, std::string& key, std::string& value)
	{
		Skip_Space(p);
		key.clear();
		while ('\0' != *p && '=' != *p && ' ' != *p && '\t' != *p && '\r' != *p && '\n' != *p)
			key.push_back(*p++);
		if ('=' != *p || key.empty())
			return false;
		++p;

		value.clear();
		if ('\"' == *p)
		{
			++p;
			while ('\0' != *p && '\"' != *p)
				value.push_back(*p++);
			if ('\"' != *p)
				return false;
			++p;
		}
		else
		{
			while ('\0' != *p && ' ' != *p && '\t' != *p && '\r' != *p && '\n' != *p)
				value.push_back(*p++);
		}
		return true;
	}

	bool_t Parse_Integer(const std::string& text, int32_t& outValue)
	{
		if (text.empty())
			return false;

		const char_t* begin = text.data();
		const char_t* end = begin + text.size();
		const auto result = std::from_chars(begin, end, outValue);
		return std::errc{} == result.ec && result.ptr == end;
	}

	bool_t Parse_Float(const std::string& text, f32_t& outValue)
	{
		if (text.empty())
			return false;
		char_t* pEnd = nullptr;
		outValue = std::strtof(text.c_str(), &pEnd);
		return pEnd == text.c_str() + text.size() &&
			std::isfinite(outValue);
	}

	bool_t Has_RemainingToken(const char_t* p)
	{
		Skip_Space(p);
		return '\0' != *p && '\r' != *p && '\n' != *p;
	}

	bool_t Is_SafeQuotedText(const std::string& text)
	{
		return std::string::npos == text.find_first_of("\"\r\n");
	}

	bool_t Parse_CommaSeparatedStableTokens(
		const std::string& Text,
		std::vector<std::string>& Out)
	{
		Out.clear();
		std::size_t iStart = 0u;
		while (iStart <= Text.size())
		{
			const std::size_t iComma = Text.find(',', iStart);
			const std::string Token = Text.substr(
				iStart, std::string::npos == iComma ? std::string::npos :
					iComma - iStart);
			if (!Is_StablePatternAuthoringId(Token))
				return false;
			Out.push_back(Token);
			if (Out.size() > 64u || std::string::npos == iComma)
				break;
			iStart = iComma + 1u;
		}
		return !Out.empty() && Out.size() <= 64u;
	}

	bool_t Parse_CommaSeparatedNonnegativeSeconds(
		const std::string& Text,
		std::vector<f32_t>& Out)
	{
		Out.clear();
		std::size_t iStart = 0u;
		while (iStart <= Text.size())
		{
			const std::size_t iComma = Text.find(',', iStart);
			const std::string Token = Text.substr(
				iStart, std::string::npos == iComma ? std::string::npos :
					iComma - iStart);
			f32_t fSeconds = 0.f;
			if (!Parse_Float(Token, fSeconds) || fSeconds < 0.f ||
				fSeconds > 600.f)
			{
				return false;
			}
			Out.push_back(fSeconds);
			if (Out.size() > 64u || std::string::npos == iComma)
				break;
			iStart = iComma + 1u;
		}
		return !Out.empty() && Out.size() <= 64u;
	}

	bool_t Load_ValtanCompositionSequenceLibrary(
		std::vector<Client::CAnimation_Tool::COMPOSITION_SEQUENCE_VIEW>& Out,
		std::string& Status)
	{
		using SEQUENCE_VIEW =
			Client::CAnimation_Tool::COMPOSITION_SEQUENCE_VIEW;
		using CLIP_VIEW =
			Client::CAnimation_Tool::COMPOSITION_SEQUENCE_CLIP_VIEW;
		const std::filesystem::path Root =
			Client::CProjectDataRoot::Resolve(L"Animation/Reference/Valtan");
		const std::filesystem::path SequencePath = Root / L"Valtan.clipseq";
		const std::filesystem::path CutPath = Root / L"Valtan.clipcuts";
		std::ifstream SequenceFile(SequencePath, std::ios::binary);
		std::ifstream CutFile(CutPath, std::ios::binary);
		if (Root.empty() || !SequenceFile || !CutFile)
		{
			Status =
				"Valtan Sequence library is missing its .clipseq or .clipcuts source.";
			return false;
		}

		auto ParseHeader = [&Status](
			std::ifstream& Stream,
			const char_t* const pExpectedMagic,
			const int32_t iExpectedVersion,
			int32_t& iOutCount)
		{
			std::string Line;
			if (!std::getline(Stream, Line))
				return false;
			const char_t* p = Line.c_str();
			std::string Magic;
			std::string Version;
			std::string Owner;
			std::string Count;
			int32_t iVersion = 0;
			if (!Read_Token(p, Magic) || !Read_Token(p, Version) ||
				!Read_Quoted(p, Owner) || !Read_Token(p, Count) ||
				Has_RemainingToken(p) ||
				!Parse_Integer(Version, iVersion) ||
				!Parse_Integer(Count, iOutCount) ||
				Magic != pExpectedMagic || iVersion != iExpectedVersion ||
				"Valtan" != Owner || iOutCount < 1 || iOutCount > 4096)
			{
				Status = std::string(pExpectedMagic) +
					" header is malformed or does not name Valtan.";
				return false;
			}
			return true;
		};

		int32_t iSequenceCount = 0;
		if (!ParseHeader(
				SequenceFile, "LOSTARK_CLIP_SEQ", 2, iSequenceCount))
		{
			return false;
		}
		std::vector<SEQUENCE_VIEW> Staged;
		Staged.reserve(static_cast<std::size_t>(iSequenceCount));
		std::map<std::pair<int32_t, int32_t>, std::size_t> SequenceByKey;
		std::string Line;
		for (int32_t iRow = 0; iRow < iSequenceCount; ++iRow)
		{
			if (!std::getline(SequenceFile, Line))
			{
				Status = "Valtan.clipseq ended before its declared row count.";
				return false;
			}
			const char_t* p = Line.c_str();
			std::string Action;
			std::string DisplayName;
			int32_t iAction = 0;
			if (!Read_Token(p, Action) || !Read_Quoted(p, DisplayName) ||
				!Parse_Integer(Action, iAction) || iAction <= 0 ||
				!Is_SafeQuotedText(DisplayName) || DisplayName.empty())
			{
				Status = "Valtan.clipseq contains a malformed action identity/name row.";
				return false;
			}
			SEQUENCE_VIEW Sequence;
			Sequence.iSkillId = iAction;
			bool_t bSawSequence = false;
			bool_t bSawMode = false;
			bool_t bSawClips = false;
			std::vector<std::string> Clips;
			std::string Key;
			std::string Value;
			while (Read_Pair(p, Key, Value))
			{
				if ("seq" == Key && !bSawSequence)
				{
					bSawSequence = Parse_Integer(
						Value, Sequence.iSequenceIndex) &&
						Sequence.iSequenceIndex >= 0;
				}
				else if ("mode" == Key && !bSawMode)
				{
					bSawMode = "COMBO" == Value || "HOLD" == Value ||
						"SEQUENCE" == Value;
					Sequence.strMode = Value;
				}
				else if ("clips" == Key && !bSawClips)
				{
					bSawClips = Parse_CommaSeparatedStableTokens(Value, Clips);
				}
				else
				{
					Status = "Valtan.clipseq contains a duplicate or unknown field.";
					return false;
				}
			}
			if (Has_RemainingToken(p) || !bSawSequence || !bSawMode ||
				!bSawClips || !SequenceByKey.emplace(
					std::pair{ Sequence.iSkillId, Sequence.iSequenceIndex },
					Staged.size()).second)
			{
				Status = "Valtan.clipseq sequence row is incomplete or duplicated.";
				return false;
			}
			Sequence.strDisplayName = std::move(DisplayName);
			Sequence.strStableId = "Valtan:" +
				std::to_string(Sequence.iSkillId) + ":" +
				std::to_string(Sequence.iSequenceIndex);
			Sequence.strCategory = "Valtan";
			Sequence.strProfileId = "Valtan";
			Sequence.bValtanPatternCompatible = true;
			Sequence.Clips.reserve(Clips.size());
			for (std::string& Clip : Clips)
			{
				CLIP_VIEW View;
				View.strClipName = std::move(Clip);
				Sequence.Clips.push_back(std::move(View));
			}
			Staged.push_back(std::move(Sequence));
		}
		while (std::getline(SequenceFile, Line))
		{
			if (Line.find_first_not_of(" \t\r\n") != std::string::npos)
			{
				Status = "Valtan.clipseq contains rows beyond its declared count.";
				return false;
			}
		}

		int32_t iCutCount = 0;
		if (!ParseHeader(CutFile, "LOSTARK_CLIP_CUTS", 1, iCutCount) ||
			iCutCount != iSequenceCount)
		{
			Status = "Valtan.clipcuts does not match the Sequence catalog count.";
			return false;
		}
		std::map<std::pair<int32_t, int32_t>, std::vector<f32_t>> CutsByKey;
		for (int32_t iRow = 0; iRow < iCutCount; ++iRow)
		{
			if (!std::getline(CutFile, Line))
			{
				Status = "Valtan.clipcuts ended before its declared row count.";
				return false;
			}
			const char_t* p = Line.c_str();
			std::string Action;
			int32_t iAction = 0;
			int32_t iSequence = -1;
			bool_t bSawSequence = false;
			bool_t bSawCuts = false;
			std::vector<f32_t> Cuts;
			if (!Read_Token(p, Action) || !Parse_Integer(Action, iAction) ||
				iAction <= 0)
			{
				Status = "Valtan.clipcuts contains a malformed action identity.";
				return false;
			}
			std::string Key;
			std::string Value;
			while (Read_Pair(p, Key, Value))
			{
				if ("seq" == Key && !bSawSequence)
					bSawSequence = Parse_Integer(Value, iSequence) && iSequence >= 0;
				else if ("cuts" == Key && !bSawCuts)
					bSawCuts = Parse_CommaSeparatedNonnegativeSeconds(Value, Cuts);
				else
				{
					Status = "Valtan.clipcuts contains a duplicate or unknown field.";
					return false;
				}
			}
			if (Has_RemainingToken(p) || !bSawSequence || !bSawCuts ||
				!CutsByKey.emplace(std::pair{ iAction, iSequence },
					std::move(Cuts)).second)
			{
				Status = "Valtan.clipcuts row is incomplete or duplicated.";
				return false;
			}
		}

		for (SEQUENCE_VIEW& Sequence : Staged)
		{
			const auto Cuts = CutsByKey.find(
				{ Sequence.iSkillId, Sequence.iSequenceIndex });
			if (Cuts == CutsByKey.end() ||
				Cuts->second.size() != Sequence.Clips.size())
			{
				Status = "Valtan Sequence has no exact per-clip source timing: " +
					std::to_string(Sequence.iSkillId) + "/" +
					std::to_string(Sequence.iSequenceIndex) + ".";
				return false;
			}
			std::vector<CLIP_VIEW> TimedClips;
			TimedClips.reserve(Sequence.Clips.size());
			for (std::size_t iClip = 0u; iClip < Sequence.Clips.size(); ++iClip)
			{
				/* Extracted 0.000 cuts are explicit skipped source steps, not a
				   malformed Sequence and not a zero-duration authoring slot. Keep
				   the exact positional join above, then omit only those markers from
				   the model-independent composition view. */
				if (Cuts->second[iClip] <= 0.f)
					continue;
				CLIP_VIEW Clip = std::move(Sequence.Clips[iClip]);
				Clip.iDurationMs = static_cast<uint32_t>(
					std::llround(static_cast<double>(Cuts->second[iClip]) * 1000.0));
				if (0u == Clip.iDurationMs)
				{
					Status = "Valtan Sequence contains a positive cut below one millisecond: " +
						std::to_string(Sequence.iSkillId) + "/" +
						std::to_string(Sequence.iSequenceIndex) + ".";
					return false;
				}
				Clip.bUsesNativeDuration = false;
				TimedClips.push_back(std::move(Clip));
			}
			if (TimedClips.empty())
			{
				Status = "Valtan Sequence contains only skipped source steps: " +
					std::to_string(Sequence.iSkillId) + "/" +
					std::to_string(Sequence.iSequenceIndex) + ".";
				return false;
			}
			Sequence.Clips = std::move(TimedClips);
		}
		Out = std::move(Staged);
		Status = "Loaded " + std::to_string(Out.size()) +
			" exact Valtan source Sequences from .clipseq + .clipcuts (model-independent).";
		return true;
	}

	const char_t* Resolve_ActionCompositionCategory(
		const std::string_view strProfileId,
		const std::string_view strDisplayName)
	{
		/* RPCZ contains both ordinary and explicitly designer-labelled large
		   Kouku actions. RPCT_07 is the Kouku/Saydon combined planner body and
		   remains under Kouku; the 05/06 bodies carry Saydon and large-Saydon
		   authored names. Never infer a category from a model file scan. */
		return Client::CKoukuSaydonAnimationActionDocument::Resolve_ActionCategory(
			strProfileId, strDisplayName);
	}

	bool_t Load_KoukuSaydonCompositionSequenceLibrary(
		std::vector<Client::CAnimation_Tool::COMPOSITION_SEQUENCE_VIEW>& Out,
		std::string& Status)
	{
		using SEQUENCE_VIEW =
			Client::CAnimation_Tool::COMPOSITION_SEQUENCE_VIEW;
		using CLIP_VIEW =
			Client::CAnimation_Tool::COMPOSITION_SEQUENCE_CLIP_VIEW;
		using BINDING_KEY =
			std::tuple<std::uint32_t, std::string, std::string>;

		std::vector<SEQUENCE_VIEW> Staged;
		for (const KOUKU_SAYDON_ACTION_PROFILE_CONTRACT& Profile : KOUKU_SAYDON_ACTION_PROFILES)
		{
			const std::filesystem::path ReferencePath =
				Client::CKoukuSaydonAnimationActionDocument::Resolve_ReferencePath(
					Profile.pProfileId);
			const std::filesystem::path AuthoredPath =
				Client::CKoukuSaydonAnimationActionDocument::Resolve_AuthoredPath(
					Profile.pProfileId);
			std::string ReferenceBytes;
			std::string AuthoredBytes;
			if (ReferencePath.empty() || AuthoredPath.empty() ||
				!Read_BoundedFile(
					ReferencePath, 16u * 1024u * 1024u,
					ReferenceBytes, Status) ||
				!Read_BoundedFile(
					AuthoredPath, 8u * 1024u * 1024u,
					AuthoredBytes, Status))
			{
				Status = "Action Composition catalog could not read " +
					std::string(Profile.pProfileId) + ": " + Status;
				return false;
			}

			Client::KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT Reference;
			Client::KOUKU_SAYDON_ANIMATION_ACTION_AUTHORED_DOCUMENT Authored;
			if (!Client::CKoukuSaydonAnimationActionDocument::Parse_ReferenceText(
					ReferenceBytes, Reference, Status) ||
				!Client::CKoukuSaydonAnimationActionDocument::Parse_AuthoredText(
					AuthoredBytes, Authored, Status))
			{
				Status = "Action Composition catalog rejected " +
					std::string(Profile.pProfileId) + ": " + Status;
				return false;
			}

			/* This browser is model-independent, so its exact typed source documents
			   provide the bounded candidate clip set. Physical CModel admission still
			   happens after the profile is opened in Animation Tool. */
			std::vector<std::string> SourceClips;
			for (const Client::KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE& Action :
				Reference.Actions)
			{
				for (const Client::KOUKU_SAYDON_ANIMATION_ACTION_STAGE_REFERENCE& Stage :
					Action.Stages)
				{
					for (const Client::KOUKU_SAYDON_ANIMATION_ACTION_SLOT_REFERENCE& Slot :
						Stage.Slots)
					{
						SourceClips.push_back(Slot.strRuntimeClip);
					}
				}
			}
			for (const Client::KOUKU_SAYDON_ANIMATION_ACTION_BINDING& Binding :
				Authored.Bindings)
			{
				SourceClips.push_back(Binding.strRuntimeClip);
			}
			std::sort(SourceClips.begin(), SourceClips.end());
			SourceClips.erase(
				std::unique(SourceClips.begin(), SourceClips.end()),
				SourceClips.end());
			if (!Client::CKoukuSaydonAnimationActionDocument::Validate_Authored(
					Authored, Reference, Profile.pProfileId,
					Profile.pModelAssetId, SourceClips, Status))
			{
				Status = "Action Composition catalog rejected " +
					std::string(Profile.pProfileId) + ": " + Status;
				return false;
			}

			std::map<BINDING_KEY,
				const Client::KOUKU_SAYDON_ANIMATION_ACTION_BINDING*> BindingBySlot;
			for (const Client::KOUKU_SAYDON_ANIMATION_ACTION_BINDING& Binding :
				Authored.Bindings)
			{
				BindingBySlot.emplace(
					BINDING_KEY{ Binding.iSourceActionId,
						Binding.strStageId, Binding.strSlotId }, &Binding);
			}

			for (const Client::KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE& Action :
				Reference.Actions)
			{
				if ("REVIEW_CANDIDATE" != Action.strReviewStatus ||
					Action.iSourceActionId > static_cast<std::uint32_t>(
						(std::numeric_limits<int32_t>::max)()))
				{
					continue;
				}
				SEQUENCE_VIEW Sequence;
				Sequence.iSkillId = static_cast<int32_t>(Action.iSourceActionId);
				Sequence.iSequenceIndex = 0;
				Sequence.strStableId = std::string(Profile.pProfileId) + ":" +
					std::to_string(Action.iSourceActionId);
				Sequence.strCategory = Resolve_ActionCompositionCategory(
					Profile.pProfileId, Action.strDisplayName);
				Sequence.strProfileId = Profile.pProfileId;
				Sequence.strDisplayName = Action.strDisplayName;
				Sequence.strMode = "ACTION";
				Sequence.bValtanPatternCompatible = false;
				for (const Client::KOUKU_SAYDON_ANIMATION_ACTION_STAGE_REFERENCE& Stage :
					Action.Stages)
				{
					for (const Client::KOUKU_SAYDON_ANIMATION_ACTION_SLOT_REFERENCE& Slot :
						Stage.Slots)
					{
						const auto Binding = BindingBySlot.find(BINDING_KEY{
							Action.iSourceActionId, Stage.strStageId, Slot.strSlotId });
						CLIP_VIEW Clip;
						if (Binding == BindingBySlot.end())
						{
							Clip.strClipName = Slot.strRuntimeClip;
							Clip.iDurationMs = Slot.iPlayMs;
						}
						else
						{
							Clip.strClipName = Binding->second->strRuntimeClip;
							Clip.iDurationMs = Binding->second->iPlayMs;
						}
						Clip.bUsesNativeDuration = false;
						Sequence.Clips.push_back(std::move(Clip));
					}
				}
				/* HOLDOUT and slot-less planner rows stay in their owning Animation
				   Tool for inspection; they are not meaningful selectable Sequences. */
				if (!Sequence.Clips.empty())
					Staged.push_back(std::move(Sequence));
			}
		}

		Out.insert(Out.end(),
			std::make_move_iterator(Staged.begin()),
			std::make_move_iterator(Staged.end()));
		return true;
	}

	bool_t Contains_NoCase(const char_t* pText, const char_t* pNeedle)
	{
		if (nullptr == pText)
			return false;
		if ('\0' == pNeedle[0])
			return true;

		for (const char_t* p = pText; *p != '\0'; ++p)
		{
			const char_t* a = p;
			const char_t* b = pNeedle;
			while (*a != '\0' && *b != '\0' &&
				tolower(static_cast<unsigned char>(*a)) == tolower(static_cast<unsigned char>(*b)))
			{
				++a;
				++b;
			}
			if ('\0' == *b)
				return true;
		}
		return false;
	}

	/* Start_Animation cuts straight to the new clip's first frame, which is what
	   the product Valtan still does. The preview blends the way CCharacter
	   already does so the animator judges motion rather than the seam. The
	   blend snapshot has to be taken before the track reset overwrites the
	   bones, hence the two calls in this order. Clips repeated back to back
	   keep their hard restart: the pose is identical, so there is nothing to
	   blend, and a re-strike is meant to read as a new strike. */
	bool_t Start_PreviewClip(
		const shared_ptr<Engine::CModel>& pModel,
		const char_t* pClipName,
		const bool_t bLoop,
		const f32_t fBlendSeconds)
	{
		if (nullptr == pModel || nullptr == pClipName)
			return false;
		if (fBlendSeconds > 0.f)
			pModel->Set_Animation(pClipName, bLoop, fBlendSeconds);
		return pModel->Start_Animation(pClipName, bLoop);
	}
}
