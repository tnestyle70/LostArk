#include "ValtanPatternSoundCueDocument.h"

#include "AnimationSkillBindingDocument.h"
#include "ActionPresentationTimeline.h"
#include "DataJson.h"
#include "EncounterPatternReference.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"
#include "SoundCueCatalog.h"
#include "ValtanPatternTree.h"

#include <bcrypt.h>
#include <windows.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <sstream>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

#pragma comment(lib, "bcrypt.lib")

namespace
{
	using namespace Client;

	constexpr std::string_view SCHEMA = "lostark.valtan-pattern-sound-cues";
	constexpr uint32_t FORMAT_VERSION = 1u;
	constexpr std::string_view OWNER_ARCHETYPE_ID = "BOSS_VALTAN";
	/* Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json real cue count grew past the
	old 512 cap (519 as of the animation-tool-presentation-parity content), which fail-closed the
	entire document -- every Valtan pattern sound cue silently stopped playing at once instead of
	just the cues past the limit. Raised with real headroom instead of matching the count exactly. */
	constexpr size_t MAX_CUE_COUNT = 1024u;
	constexpr wchar_t SOUND_AUTHORING_SAVE_MUTEX[] =
		L"Local\\LostArk.ValtanPatternSoundCueDocument.Save";
	constexpr std::uint64_t MAX_SOUND_SOURCE_BYTES = 512ull * 1024ull;

	struct VALTAN_PATTERN_SOUND_SOURCE_READ_STATE final
	{
		~VALTAN_PATTERN_SOUND_SOURCE_READ_STATE()
		{
			if (INVALID_HANDLE_VALUE != File)
				CloseHandle(File);
			if (OwnsMutex && nullptr != Mutex)
				ReleaseMutex(Mutex);
			if (nullptr != Mutex)
				CloseHandle(Mutex);
		}

		HANDLE Mutex = nullptr;
		bool_t OwnsMutex = false;
		HANDLE File = INVALID_HANDLE_VALUE;
	};

	bool_t BuildSoundSourceReceipt(
		const std::string_view Bytes,
		VALTAN_PATTERN_SOUND_SOURCE_RECEIPT& OutReceipt)
	{
		if (Bytes.empty() || Bytes.size() > MAX_SOUND_SOURCE_BYTES ||
			Bytes.size() > static_cast<std::size_t>(
				(std::numeric_limits<ULONG>::max)()))
		{
			return false;
		}

		BCRYPT_ALG_HANDLE Algorithm = nullptr;
		BCRYPT_HASH_HANDLE Hash = nullptr;
		DWORD ObjectBytes = 0u;
		DWORD HashBytes = 0u;
		DWORD Written = 0u;
		std::vector<unsigned char> HashObject;
		std::array<unsigned char, 32u> Digest{};
		bool_t Succeeded = false;
		if (0 <= BCryptOpenAlgorithmProvider(
				&Algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0u) &&
			0 <= BCryptGetProperty(
				Algorithm, BCRYPT_OBJECT_LENGTH,
				reinterpret_cast<PUCHAR>(&ObjectBytes), sizeof(ObjectBytes),
				&Written, 0u) &&
			0 <= BCryptGetProperty(
				Algorithm, BCRYPT_HASH_LENGTH,
				reinterpret_cast<PUCHAR>(&HashBytes), sizeof(HashBytes),
				&Written, 0u) &&
			HashBytes == Digest.size())
		{
			HashObject.resize(ObjectBytes);
			if (0 <= BCryptCreateHash(
					Algorithm, &Hash, HashObject.data(), ObjectBytes,
					nullptr, 0u, 0u) &&
				0 <= BCryptHashData(
					Hash,
					reinterpret_cast<PUCHAR>(
						const_cast<char_t*>(Bytes.data())),
					static_cast<ULONG>(Bytes.size()), 0u) &&
				0 <= BCryptFinishHash(
					Hash, Digest.data(), HashBytes, 0u))
			{
				static constexpr char_t HEX[] = "0123456789abcdef";
				VALTAN_PATTERN_SOUND_SOURCE_RECEIPT Staged;
				Staged.strSha256.resize(Digest.size() * 2u);
				for (std::size_t i = 0u; i < Digest.size(); ++i)
				{
					Staged.strSha256[i * 2u] = HEX[Digest[i] >> 4u];
					Staged.strSha256[i * 2u + 1u] = HEX[Digest[i] & 0x0fu];
				}
				Staged.iBytes = static_cast<std::uint64_t>(Bytes.size());
				OutReceipt = std::move(Staged);
				Succeeded = true;
			}
		}
		if (nullptr != Hash)
			BCryptDestroyHash(Hash);
		if (nullptr != Algorithm)
			BCryptCloseAlgorithmProvider(Algorithm, 0u);
		return Succeeded;
	}

	bool_t Is_ExactObject(
		const DATA_JSON_VALUE& Value,
		const std::initializer_list<const char_t*> Keys)
	{
		if (!Value.Is_Object() || Value.Get_Object().size() != Keys.size())
			return false;
		return std::all_of(Keys.begin(), Keys.end(),
			[&Value](const char_t* pKey)
			{
				return nullptr != Value.Find(pKey);
			});
	}

	bool_t Is_StableId(const std::string_view Value)
	{
		return !Value.empty() && Value.size() <= 160u &&
			std::all_of(Value.begin(), Value.end(),
				[](const char_t Character)
				{
					const unsigned char Value =
						static_cast<unsigned char>(Character);
					return 0 != std::isalnum(Value) || Character == '_' ||
						Character == '-' || Character == '.';
				});
	}

	bool_t Read_String(const DATA_JSON_VALUE& Parent,
		const char_t* pKey, std::string& strOutValue)
	{
		const DATA_JSON_VALUE* pValue = Parent.Find(pKey);
		if (nullptr == pValue || !pValue->Is_String() ||
			!Is_StableId(pValue->Get_String()))
		{
			return false;
		}
		strOutValue = pValue->Get_String();
		return true;
	}

	bool_t Read_Unsigned(const DATA_JSON_VALUE& Parent,
		const char_t* pKey, const uint32_t iMaximum, uint32_t& iOutValue)
	{
		const DATA_JSON_VALUE* pValue = Parent.Find(pKey);
		if (nullptr == pValue || !pValue->Is_Number())
			return false;
		const double Number = pValue->Get_Number();
		if (!std::isfinite(Number) || Number < 0.0 ||
			Number > static_cast<double>(iMaximum) ||
			std::floor(Number) != Number)
		{
			return false;
		}
		iOutValue = static_cast<uint32_t>(Number);
		return true;
	}

	bool_t Read_RepeatPolicy(const DATA_JSON_VALUE& Parent,
		VALTAN_PATTERN_SOUND_REPEAT_POLICY& eOutPolicy)
	{
		const DATA_JSON_VALUE* pValue = Parent.Find("repeatPolicy");
		if (nullptr == pValue || !pValue->Is_String())
			return false;
		if ("once" == pValue->Get_String())
			eOutPolicy = VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE;
		else if ("each_loop" == pValue->Get_String())
			eOutPolicy = VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP;
		else
			return false;
		return true;
	}

	std::filesystem::path Resolve_EncounterPath()
	{
		return CProjectDataRoot::Resolve(
			std::filesystem::path(L"Encounters") / L"Valtan" /
			L"ValtanEncounter.json");
	}

	bool_t Load_EncounterReference(CEncounterPatternReference& OutEncounter,
		std::string& strOutStatus)
	{
		return OutEncounter.Load(Resolve_EncounterPath(), strOutStatus);
	}

	bool_t Parse_AnimationBindings(
		const std::string_view Text,
		BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT& OutBindings,
		std::string& strOutStatus)
	{
		BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT Staged;
		if (!CValtanPatternAnimationBindingDocument::Parse_Text(
			Text, Staged, strOutStatus))
		{
			return false;
		}
		std::vector<std::string> DeclaredClips;
		for (const BOSS_PATTERN_ANIMATION_BINDING& Binding : Staged.Bindings)
		{
			for (const BOSS_PATTERN_ANIMATION_CLIP& Clip : Binding.Clips)
				DeclaredClips.push_back(Clip.strClipName);
		}
		if (!CValtanPatternAnimationBindingDocument::Validate(
			Staged, OWNER_ARCHETYPE_ID, DeclaredClips, strOutStatus))
		{
			return false;
		}
		OutBindings = std::move(Staged);
		return true;
	}

	bool_t Load_AnimationBindings(
		BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT& OutBindings,
		std::string& strOutStatus)
	{
		const std::filesystem::path Path =
			CValtanPatternAnimationBindingDocument::Resolve_Path("Valtan");
		std::ifstream Input(Path, std::ios::binary);
		if (Path.empty() || !Input)
		{
			strOutStatus =
				"Missing Valtan pattern animation binding document: " +
				Path.string();
			return false;
		}
		const std::string Text{
			std::istreambuf_iterator<char>(Input),
			std::istreambuf_iterator<char>() };
		return Parse_AnimationBindings(Text, OutBindings, strOutStatus);
	}

	const BOSS_PATTERN_ANIMATION_BINDING* Find_ActionBinding(
		const BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT& Document,
		const std::string_view ActionId)
	{
		const auto Found = std::find_if(Document.Bindings.begin(),
			Document.Bindings.end(),
			[ActionId](const BOSS_PATTERN_ANIMATION_BINDING& Binding)
			{
				return Binding.strActionId == ActionId;
			});
		return Document.Bindings.end() == Found ? nullptr : &*Found;
	}

	const BOSS_PATTERN_ANIMATION_CLIP* Find_ClipOccurrence(
		const BOSS_PATTERN_ANIMATION_BINDING& Binding,
		const std::string_view ClipOccurrenceId)
	{
		const auto Found = std::find_if(Binding.Clips.begin(),
			Binding.Clips.end(),
			[ClipOccurrenceId](const BOSS_PATTERN_ANIMATION_CLIP& Clip)
			{
				return Clip.strClipOccurrenceId == ClipOccurrenceId;
			});
		return Binding.Clips.end() == Found ? nullptr : &*Found;
	}

	bool_t Read_File(const std::filesystem::path& Path,
		std::string& strOutText)
	{
		std::ifstream Input(Path, std::ios::binary);
		if (Path.empty() || !Input)
			return false;
		strOutText.assign(std::istreambuf_iterator<char>(Input),
			std::istreambuf_iterator<char>());
		return !Input.bad();
	}

	std::string Serialize_Document(
		const VALTAN_PATTERN_SOUND_CUE_DOCUMENT& Document)
	{
		std::ostringstream Output;
		Output <<
			"{\n"
			"  \"schema\": \"lostark.valtan-pattern-sound-cues\",\n"
			"  \"formatVersion\": 1,\n"
			"  \"ownerArchetypeId\": \"" <<
			CDataJson::Escape(Document.strOwnerArchetypeId) <<
			"\",\n  \"cues\": [\n";
		for (std::size_t i = 0u; i < Document.Cues.size(); ++i)
		{
			const VALTAN_PATTERN_SOUND_CUE& Cue = Document.Cues[i];
			Output <<
				"    {\n"
				"      \"bindingId\": \"" <<
				CDataJson::Escape(Cue.strBindingId) << "\",\n"
				"      \"occurrenceId\": \"" <<
				CDataJson::Escape(Cue.strOccurrenceId) << "\",\n"
				"      \"patternId\": \"" <<
				CDataJson::Escape(Cue.strPatternId) << "\",\n"
				"      \"stageId\": \"" <<
				CDataJson::Escape(Cue.strStageId) << "\",\n"
				"      \"actionId\": \"" <<
				CDataJson::Escape(Cue.strActionId) << "\",\n"
				"      \"clipOccurrenceId\": \"" <<
				CDataJson::Escape(Cue.strClipOccurrenceId) << "\",\n"
				"      \"soundBank\": \"" <<
				CDataJson::Escape(Cue.strSoundBank) << "\",\n"
				"      \"soundEvent\": \"" <<
				CDataJson::Escape(Cue.strSoundEvent) << "\",\n"
				"      \"repeatPolicy\": \"" <<
				(VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP ==
					Cue.eRepeatPolicy ? "each_loop" : "once") << "\",\n"
				"      \"startMs\": " << Cue.iStartMs << "\n"
				"    }";
			if (i + 1u != Document.Cues.size())
				Output << ',';
			Output << '\n';
		}
		Output << "  ]\n}\n";
		return Output.str();
	}

	bool_t Count_DeclaredCues(const std::string_view Text,
		std::size_t& iOutCount)
	{
		DATA_JSON_VALUE Root;
		std::string ParseError;
		DATA_JSON_PARSE_LIMITS Limits{};
		Limits.iMaximumBytes = 512u * 1024u;
		Limits.iMaximumDepth = 8u;
		Limits.iMaximumValues = 8192u;
		if (!CDataJson::Parse(Text, Root, ParseError, Limits))
			return false;
		const DATA_JSON_VALUE* pCues = Root.Find("cues");
		if (nullptr == pCues || !pCues->Is_Array())
			return false;
		iOutCount = pCues->Get_Array().size();
		return true;
	}

	bool_t Is_ValidSoundAssetId(const std::string_view AssetId)
	{
		if (!AssetId.starts_with("Sound/") ||
			!AssetId.ends_with(".wav") ||
			std::string_view::npos != AssetId.find('\\') ||
			std::string_view::npos != AssetId.find(':'))
		{
			return false;
		}
		std::size_t Begin = 0u;
		while (Begin <= AssetId.size())
		{
			const std::size_t End = AssetId.find('/', Begin);
			const std::string_view Component = AssetId.substr(Begin,
				std::string_view::npos == End ? AssetId.size() - Begin :
				End - Begin);
			if (Component.empty() || "." == Component || ".." == Component)
				return false;
			if (std::string_view::npos == End)
				break;
			Begin = End + 1u;
		}
		return true;
	}

	std::string_view Required_SoundBank(const std::string_view SoundEvent)
	{
		if (SoundEvent.starts_with("G_Voltan1_"))
			return "S_Mob_G_Voltan1";
		if (SoundEvent.starts_with("G_Voltan2_"))
			return "S_Mob_G_Voltan2";
		return {};
	}

	bool_t Validate_CatalogAssets(
		VALTAN_PATTERN_SOUND_CUE_DOCUMENT& Document,
		std::string& strOutStatus)
	{
		CSoundCueCatalog::EVENT_VARIANTS CatalogEvents;
		if (!CSoundCueCatalog::Load_ClassSnapshot(
				"Valtan", CatalogEvents, strOutStatus))
			return false;
		for (VALTAN_PATTERN_SOUND_CUE& Cue : Document.Cues)
		{
			const std::string_view RequiredBank =
				Required_SoundBank(Cue.strSoundEvent);
			if (RequiredBank.empty() || RequiredBank != Cue.strSoundBank)
			{
				strOutStatus =
					"Valtan pattern Sound cue bank/event identity is invalid: " +
					Cue.strBindingId;
				return false;
			}
			const auto Event = CatalogEvents.find(Cue.strSoundEvent);
			if (CatalogEvents.end() == Event)
			{
				strOutStatus =
					"Valtan pattern Sound event is not declared by the catalog: " +
					Cue.strSoundEvent;
				return false;
			}
			/* A declared event may remain unresolved when the extracted Resource
			   pack has no WAV for it yet. Preserve that cue for authoring/Detail;
			   playback treats an empty resolved list as an isolated no-op. */
			for (const std::string& AssetId : Event->second)
			{
				const std::filesystem::path AssetPath =
					CRuntimeAssetRoot::Resolve(AssetId);
				std::error_code AssetError;
				if (!Is_ValidSoundAssetId(AssetId) || AssetPath.empty() ||
					!std::filesystem::is_regular_file(AssetPath, AssetError) ||
					AssetError)
				{
					strOutStatus =
						"Valtan pattern Sound catalog asset is missing or invalid: " +
						AssetId;
					return false;
				}
			}
			Cue.ResolvedAssetIds = Event->second;
		}
		return true;
	}

	using TIMINGS_BY_ACTION = std::unordered_map<std::string,
		std::vector<ACTION_PRESENTATION_CLIP_TIMING>>;

	bool_t Build_StrongRuntimeTimings(
		const VALTAN_PATTERN_SOUND_CUE_DOCUMENT& Document,
		const BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT& AnimationBindings,
		const std::unordered_map<std::string, f32_t>&
			ClipSourceDurationSecondsByName,
		TIMINGS_BY_ACTION& OutTimingsByAction,
		std::string& strOutStatus)
	{
		if (ClipSourceDurationSecondsByName.empty())
		{
			strOutStatus =
				"Valtan pattern Sound save requires current model clip source durations.";
			return false;
		}

		TIMINGS_BY_ACTION Staged;
		Staged.reserve(Document.Cues.size());
		for (const VALTAN_PATTERN_SOUND_CUE& Cue : Document.Cues)
		{
			if (Staged.contains(Cue.strActionId))
				continue;
			const BOSS_PATTERN_ANIMATION_BINDING* pBinding =
				Find_ActionBinding(AnimationBindings, Cue.strActionId);
			if (nullptr == pBinding || pBinding->bSuppressAnimation ||
				pBinding->Clips.empty())
			{
				strOutStatus =
					"Valtan pattern Sound timing join found no active action: " +
					Cue.strActionId;
				return false;
			}

			std::vector<ACTION_PRESENTATION_CLIP_TIMING> Timings;
			Timings.reserve(pBinding->Clips.size());
			for (const BOSS_PATTERN_ANIMATION_CLIP& Clip : pBinding->Clips)
			{
				const auto Duration =
					ClipSourceDurationSecondsByName.find(Clip.strClipName);
				if (ClipSourceDurationSecondsByName.end() == Duration ||
					!std::isfinite(Duration->second) || Duration->second <= 0.f)
				{
					strOutStatus =
						"Valtan pattern Sound timing join is missing a valid model duration: " +
						Clip.strClipName;
					return false;
				}

				ACTION_PRESENTATION_CLIP_TIMING Timing{
					Duration->second,
					Clip.iPlayMs,
					Clip.fPlayRate,
					Clip.bLoop,
					static_cast<f32_t>(Clip.iSourceStartMs) * 0.001f };
				f32_t fSourceDurationSeconds = 0.f;
				f32_t fWallDurationSeconds = 0.f;
				if (!CActionPresentationTimeline::Resolve_ClipDuration(
						Timing, fSourceDurationSeconds, fWallDurationSeconds))
				{
					strOutStatus =
						"Valtan pattern Sound timing join rejected a model clip window: " +
						Clip.strClipOccurrenceId;
					return false;
				}
				Timings.push_back(Timing);
			}
			if (!Staged.emplace(Cue.strActionId, std::move(Timings)).second)
			{
				strOutStatus =
					"Valtan pattern Sound timing join found a duplicate action: " +
					Cue.strActionId;
				return false;
			}
		}
		OutTimingsByAction = std::move(Staged);
		return true;
	}

	bool_t Validate_RuntimeTiming(
		const VALTAN_PATTERN_SOUND_CUE_DOCUMENT& Document,
		const BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT& AnimationBindings,
		const std::unordered_map<std::string, f32_t>&
			ClipSourceDurationSecondsByName,
		std::string& strOutStatus)
	{
		TIMINGS_BY_ACTION TimingsByAction;
		if (!Build_StrongRuntimeTimings(Document, AnimationBindings,
			ClipSourceDurationSecondsByName, TimingsByAction, strOutStatus))
		{
			return false;
		}

		for (const VALTAN_PATTERN_SOUND_CUE& Cue : Document.Cues)
		{
			const BOSS_PATTERN_ANIMATION_BINDING* pBinding =
				Find_ActionBinding(AnimationBindings, Cue.strActionId);
			const auto Timings = TimingsByAction.find(Cue.strActionId);
			if (nullptr == pBinding || TimingsByAction.end() == Timings)
			{
				strOutStatus =
					"Valtan pattern Sound cue has no strong action timeline: " +
					Cue.strOccurrenceId;
				return false;
			}
			const auto Clip = std::find_if(pBinding->Clips.begin(),
				pBinding->Clips.end(),
				[&Cue](const BOSS_PATTERN_ANIMATION_CLIP& Candidate)
				{
					return Candidate.strClipOccurrenceId ==
						Cue.strClipOccurrenceId;
				});
			if (pBinding->Clips.end() == Clip)
			{
				strOutStatus =
					"Valtan pattern Sound cue has no strong clip occurrence: " +
					Cue.strOccurrenceId;
				return false;
			}

			const std::size_t iClipIndex = static_cast<std::size_t>(
				Clip - pBinding->Clips.begin());
			f32_t fSourceDurationSeconds = 0.f;
			f32_t fWallDurationSeconds = 0.f;
			f32_t fCueStartWallSeconds = 0.f;
			const f32_t fCueSourceSeconds =
				static_cast<f32_t>(Cue.iStartMs) * 0.001f;
			if (iClipIndex >= Timings->second.size() ||
				!CActionPresentationTimeline::Resolve_ClipDuration(
					Timings->second[iClipIndex], fSourceDurationSeconds,
					fWallDurationSeconds) ||
				fCueSourceSeconds >=
					Timings->second[iClipIndex].fSourceStartSeconds +
					fSourceDurationSeconds ||
				!CActionPresentationTimeline::Resolve_CueWallOffset(
					Timings->second, iClipIndex, fCueSourceSeconds, 0u,
					fCueStartWallSeconds) ||
				fCueStartWallSeconds * 1000.f >=
					static_cast<f32_t>(Cue.iStageDurationMs))
			{
				strOutStatus =
					"Valtan pattern Sound cue is outside its runtime source/stage wall: " +
					Cue.strOccurrenceId;
				return false;
			}
			if (VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP ==
					Cue.eRepeatPolicy && !Timings->second[iClipIndex].bLoop)
			{
				strOutStatus =
					"Valtan each_loop pattern Sound cue has no runtime loop: " +
					Cue.strOccurrenceId;
				return false;
			}
		}
		return true;
	}

	bool_t Has_SameEditableFields(
		const VALTAN_PATTERN_SOUND_CUE& Current,
		const VALTAN_PATTERN_SOUND_CUE& Draft)
	{
		return Current.strSoundBank == Draft.strSoundBank &&
			Current.strSoundEvent == Draft.strSoundEvent &&
			Current.eRepeatPolicy == Draft.eRepeatPolicy &&
			Current.iStartMs == Draft.iStartMs;
	}

	std::string Make_AuthoringBindingId(
		const std::string_view ClipOccurrenceId,
		const uint32_t iOrdinal)
	{
		std::ostringstream Id;
		Id << "cue.sound.authoring." << ClipOccurrenceId << '.' <<
			std::setfill('0') << std::setw(4) << iOrdinal;
		return Id.str();
	}

	bool_t Allocate_AuthoringRowId(
		const VALTAN_PATTERN_SOUND_CUE_DOCUMENT& Document,
		const std::string_view ClipOccurrenceId,
		VALTAN_PATTERN_SOUND_CUE_ROW_ID& OutRowId,
		std::string& strOutStatus)
	{
		std::unordered_set<std::string> BindingIds;
		std::unordered_set<std::string> OccurrenceIds;
		BindingIds.reserve(Document.Cues.size());
		OccurrenceIds.reserve(Document.Cues.size());
		for (const VALTAN_PATTERN_SOUND_CUE& Cue : Document.Cues)
		{
			BindingIds.insert(Cue.strBindingId);
			OccurrenceIds.insert(Cue.strOccurrenceId);
		}
		for (uint32_t iOrdinal = 1u;
			iOrdinal <= static_cast<uint32_t>(MAX_CUE_COUNT); ++iOrdinal)
		{
			VALTAN_PATTERN_SOUND_CUE_ROW_ID Candidate;
			Candidate.strBindingId =
				Make_AuthoringBindingId(ClipOccurrenceId, iOrdinal);
			Candidate.strOccurrenceId =
				Candidate.strBindingId + ".occurrence.01";
			if (Is_StableId(Candidate.strBindingId) &&
				Is_StableId(Candidate.strOccurrenceId) &&
				!BindingIds.contains(Candidate.strBindingId) &&
				!OccurrenceIds.contains(Candidate.strOccurrenceId))
			{
				OutRowId = std::move(Candidate);
				return true;
			}
		}
		strOutStatus =
			"Valtan pattern Sound authoring IDs are exhausted for clip occurrence: " +
			std::string(ClipOccurrenceId);
		return false;
	}

	bool_t Is_DeterministicAuthoringRowId(
		const VALTAN_PATTERN_SOUND_CUE& Cue,
		uint32_t& iOutOrdinal)
	{
		const std::string Prefix =
			"cue.sound.authoring." + Cue.strClipOccurrenceId + ".";
		if (!Cue.strBindingId.starts_with(Prefix) ||
			Cue.strOccurrenceId != Cue.strBindingId + ".occurrence.01")
		{
			return false;
		}
		const std::string_view Suffix(Cue.strBindingId.data() + Prefix.size(),
			Cue.strBindingId.size() - Prefix.size());
		if (4u != Suffix.size() || !std::all_of(Suffix.begin(), Suffix.end(),
			[](const char_t Character)
			{
				return Character >= '0' && Character <= '9';
			}))
		{
			return false;
		}
		uint32_t iOrdinal = 0u;
		for (const char_t Character : Suffix)
			iOrdinal = iOrdinal * 10u + static_cast<uint32_t>(Character - '0');
		if (0u == iOrdinal || iOrdinal > MAX_CUE_COUNT ||
			Make_AuthoringBindingId(Cue.strClipOccurrenceId, iOrdinal) !=
				Cue.strBindingId)
		{
			return false;
		}
		iOutOrdinal = iOrdinal;
		return true;
	}

	bool_t Validate_ChangedRuntimeTiming(
		const VALTAN_PATTERN_SOUND_CUE_DOCUMENT& Current,
		const VALTAN_PATTERN_SOUND_CUE_DOCUMENT& Draft,
		const BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT& AnimationBindings,
		const std::unordered_map<std::string, f32_t>&
			ClipSourceDurationSecondsByName,
		std::string& strOutStatus)
	{
		if (ClipSourceDurationSecondsByName.empty())
		{
			strOutStatus =
				"Valtan pattern Sound save requires current model clip source durations.";
			return false;
		}
		std::unordered_map<std::string, const VALTAN_PATTERN_SOUND_CUE*> CurrentById;
		CurrentById.reserve(Current.Cues.size());
		for (const VALTAN_PATTERN_SOUND_CUE& Cue : Current.Cues)
			CurrentById.emplace(Cue.strBindingId, &Cue);

		VALTAN_PATTERN_SOUND_CUE_DOCUMENT Changed;
		Changed.iFormatVersion = Draft.iFormatVersion;
		Changed.strOwnerArchetypeId = Draft.strOwnerArchetypeId;
		for (const VALTAN_PATTERN_SOUND_CUE& Cue : Draft.Cues)
		{
			const auto Found = CurrentById.find(Cue.strBindingId);
			if (CurrentById.end() == Found ||
				!Has_SameEditableFields(*Found->second, Cue))
			{
				Changed.Cues.push_back(Cue);
			}
		}
		if (Changed.Cues.empty())
			return true;
		return Validate_RuntimeTiming(Changed, AnimationBindings,
			ClipSourceDurationSecondsByName, strOutStatus);
	}

	bool_t Parse_AuthoringText(const std::string_view Text,
		const CEncounterPatternReference& Encounter,
		const BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT& AnimationBindings,
		VALTAN_PATTERN_SOUND_CUE_DOCUMENT& OutDocument,
		std::string& strOutStatus)
	{
		std::size_t iDeclaredCueCount = 0u;
		VALTAN_PATTERN_SOUND_CUE_DOCUMENT Staged;
		if (!Count_DeclaredCues(Text, iDeclaredCueCount))
		{
			strOutStatus =
				"Valtan pattern Sound source cue array could not be counted strictly.";
			return false;
		}
		if (!CValtanPatternSoundCueDocument::Parse_Text(Text, Encounter,
			AnimationBindings, Staged, strOutStatus))
		{
			return false;
		}
		if (Staged.Cues.size() != iDeclaredCueCount)
		{
			strOutStatus =
				"Valtan pattern Sound authoring load requires every source cue to join.";
			return false;
		}
		if (!Validate_CatalogAssets(Staged, strOutStatus))
			return false;
		OutDocument = std::move(Staged);
		return true;
	}

	bool_t Validate_InventoryTransition(
		const VALTAN_PATTERN_SOUND_CUE_DOCUMENT& Current,
		const VALTAN_PATTERN_SOUND_CUE_DOCUMENT& Draft,
		const BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT& AnimationBindings,
		const std::unordered_map<std::string, f32_t>&
			ClipSourceDurationSecondsByName,
		std::string& strOutStatus)
	{
		std::unordered_map<std::string, const VALTAN_PATTERN_SOUND_CUE*> CurrentById;
		CurrentById.reserve(Current.Cues.size());
		for (const VALTAN_PATTERN_SOUND_CUE& Cue : Current.Cues)
		{
			if (!CurrentById.emplace(Cue.strBindingId, &Cue).second)
			{
				strOutStatus =
					"Valtan pattern Sound baseline has a duplicate stable binding ID.";
				return false;
			}
		}

		std::unordered_set<std::string> DraftBindingIds;
		std::unordered_set<std::string> DraftOccurrenceIds;
		DraftBindingIds.reserve(Draft.Cues.size());
		DraftOccurrenceIds.reserve(Draft.Cues.size());
		std::vector<const VALTAN_PATTERN_SOUND_CUE*> Added;
		for (const VALTAN_PATTERN_SOUND_CUE& Cue : Draft.Cues)
		{
			if (!DraftBindingIds.insert(Cue.strBindingId).second ||
				!DraftOccurrenceIds.insert(Cue.strOccurrenceId).second)
			{
				strOutStatus =
					"Valtan pattern Sound draft has a duplicate stable row identity.";
				return false;
			}
			const auto Found = CurrentById.find(Cue.strBindingId);
			if (CurrentById.end() == Found)
			{
				Added.push_back(&Cue);
				continue;
			}
			if (Found->second->strOccurrenceId != Cue.strOccurrenceId ||
				Found->second->strPatternId != Cue.strPatternId ||
				Found->second->strStageId != Cue.strStageId ||
				Found->second->strActionId != Cue.strActionId ||
				Found->second->strClipOccurrenceId != Cue.strClipOccurrenceId)
			{
				strOutStatus =
					"Valtan pattern Sound cue stable identity changed: " +
					Cue.strBindingId;
				return false;
			}
		}

		std::map<std::string, std::vector<const VALTAN_PATTERN_SOUND_CUE*>>
			AddedByClipOccurrence;
		for (const VALTAN_PATTERN_SOUND_CUE* pCue : Added)
		{
			uint32_t iOrdinal = 0u;
			if (nullptr == pCue ||
				!Is_DeterministicAuthoringRowId(*pCue, iOrdinal))
			{
				strOutStatus =
					"Added Valtan pattern Sound row has no deterministic authoring ID.";
				return false;
			}
			AddedByClipOccurrence[pCue->strClipOccurrenceId].push_back(pCue);
		}

		VALTAN_PATTERN_SOUND_CUE_DOCUMENT AllocationInventory = Current;
		for (const auto& [ClipOccurrenceId, Rows] : AddedByClipOccurrence)
		{
			std::unordered_set<std::string> ExpectedBindingIds;
			for (std::size_t i = 0u; i < Rows.size(); ++i)
			{
				VALTAN_PATTERN_SOUND_CUE_ROW_ID Expected;
				if (!Allocate_AuthoringRowId(AllocationInventory,
						ClipOccurrenceId, Expected, strOutStatus))
				{
					return false;
				}
				ExpectedBindingIds.insert(Expected.strBindingId);
				VALTAN_PATTERN_SOUND_CUE Reserved;
				Reserved.strBindingId = std::move(Expected.strBindingId);
				Reserved.strOccurrenceId = std::move(Expected.strOccurrenceId);
				AllocationInventory.Cues.push_back(std::move(Reserved));
			}
			for (const VALTAN_PATTERN_SOUND_CUE* pCue : Rows)
			{
				if (!ExpectedBindingIds.erase(pCue->strBindingId))
				{
					strOutStatus =
						"Added Valtan pattern Sound row skipped its deterministic authoring ordinal.";
					return false;
				}
			}
			if (!ExpectedBindingIds.empty())
				return false;
		}

		VALTAN_PATTERN_SOUND_CUE_DOCUMENT Removed;
		Removed.iFormatVersion = Current.iFormatVersion;
		Removed.strOwnerArchetypeId = Current.strOwnerArchetypeId;
		for (const VALTAN_PATTERN_SOUND_CUE& Cue : Current.Cues)
		{
			if (!DraftBindingIds.contains(Cue.strBindingId))
				Removed.Cues.push_back(Cue);
		}
		if (!Removed.Cues.empty())
		{
			TIMINGS_BY_ACTION RemovedTimings;
			if (!Build_StrongRuntimeTimings(Removed, AnimationBindings,
				ClipSourceDurationSecondsByName, RemovedTimings, strOutStatus))
			{
				strOutStatus =
					"Removed Valtan pattern Sound row lost its current model dependency: " +
					strOutStatus;
				return false;
			}
		}
		return true;
	}

	std::filesystem::path Make_TemporaryPath(
		const std::filesystem::path& Destination)
	{
		static std::atomic_uint64_t Sequence{ 0u };
		std::filesystem::path Temporary = Destination;
		Temporary += L".tmp." + std::to_wstring(GetCurrentProcessId()) +
			L"." + std::to_wstring(GetCurrentThreadId()) + L"." +
			std::to_wstring(GetTickCount64()) + L"." +
			std::to_wstring(Sequence.fetch_add(
				1u, std::memory_order_relaxed) + 1u);
		return Temporary;
	}

	void Remove_Temporary(const std::filesystem::path& Path)
	{
		std::error_code CleanupError;
		std::filesystem::remove(Path, CleanupError);
	}

	bool_t Write_DurableFile(const std::filesystem::path& Path,
		const std::string& Text, std::string& strOutStatus)
	{
		const HANDLE File = CreateFileW(Path.c_str(), GENERIC_WRITE, 0u,
			nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (INVALID_HANDLE_VALUE == File)
		{
			strOutStatus =
				"Valtan pattern Sound staging file could not be created (Win32 " +
				std::to_string(GetLastError()) + ").";
			return false;
		}
		bool_t bWrote = true;
		std::size_t iOffset = 0u;
		while (iOffset < Text.size())
		{
			const DWORD iRequested = static_cast<DWORD>(
				std::min<std::size_t>(Text.size() - iOffset,
					static_cast<std::size_t>(MAXDWORD)));
			DWORD iWritten = 0u;
			if (!WriteFile(File, Text.data() + iOffset, iRequested,
					&iWritten, nullptr) || 0u == iWritten)
			{
				bWrote = false;
				break;
			}
			iOffset += iWritten;
		}
		const bool_t bDurable = bWrote && FlushFileBuffers(File);
		const DWORD iWriteError = bDurable ? ERROR_SUCCESS : GetLastError();
		const bool_t bClosed = CloseHandle(File);
		const DWORD iCloseError = bClosed ? ERROR_SUCCESS : GetLastError();
		if (!bDurable || !bClosed)
		{
			Remove_Temporary(Path);
			strOutStatus =
				"Valtan pattern Sound staging write failed (Win32 " +
				std::to_string(bDurable ? iCloseError : iWriteError) +
				"); previous source preserved.";
			return false;
		}
		return true;
	}

	bool_t Commit_Temporary(const std::filesystem::path& Destination,
		const std::filesystem::path& Temporary)
	{
		std::error_code ExistsError;
		const bool_t bExists =
			std::filesystem::exists(Destination, ExistsError);
		if (ExistsError)
			return false;
		if (bExists && FALSE != ReplaceFileW(Destination.c_str(),
			Temporary.c_str(), nullptr, REPLACEFILE_WRITE_THROUGH,
			nullptr, nullptr))
		{
			return true;
		}
		return FALSE != MoveFileExW(Temporary.c_str(), Destination.c_str(),
			MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
	}

	struct SOUND_JOINED_OWNER_SOURCE_SNAPSHOT final
	{
		std::filesystem::path Path;
		std::string SourceBytes;
	};

	class SOUND_JOINED_OWNER_COMMIT_GUARD final
	{
	public:
		~SOUND_JOINED_OWNER_COMMIT_GUARD()
		{
			for (const HANDLE Handle : m_DependencyHandles)
				CloseHandle(Handle);
			if (m_ownsMutex && nullptr != m_Mutex)
				ReleaseMutex(m_Mutex);
			if (nullptr != m_Mutex)
				CloseHandle(m_Mutex);
		}

		bool_t Lock_AndVerify(
			const std::array<SOUND_JOINED_OWNER_SOURCE_SNAPSHOT, 2u>&
				DependencySnapshots,
			const std::filesystem::path& SoundDestination,
			const std::string_view ExpectedSoundSourceBytes,
			std::string& strOutStatus)
		{
			m_Mutex = CreateMutexW(
				nullptr, FALSE, SOUND_AUTHORING_SAVE_MUTEX);
			if (nullptr == m_Mutex)
			{
				strOutStatus =
					"Valtan pattern Sound Save could not create its bounded destination commit mutex.";
				return false;
			}
			const DWORD WaitResult = WaitForSingleObject(m_Mutex, 5000u);
			if (WAIT_OBJECT_0 != WaitResult && WAIT_ABANDONED != WaitResult)
			{
				strOutStatus =
					"Valtan pattern Sound Save timed out waiting for its destination commit mutex.";
				return false;
			}
			m_ownsMutex = true;

			for (const SOUND_JOINED_OWNER_SOURCE_SNAPSHOT& Snapshot :
				DependencySnapshots)
			{
				/* Deny write/delete sharing until the Sound ReplaceFile has
				   completed. Animation Save and Encounter publish therefore cannot
				   invalidate the exact join after preflight. */
				const HANDLE Handle = CreateFileW(
					Snapshot.Path.c_str(), GENERIC_READ, FILE_SHARE_READ,
					nullptr, OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
				if (INVALID_HANDLE_VALUE == Handle)
				{
					strOutStatus =
						"Valtan pattern Sound Save could not lock an Encounter/Animation dependency for commit CAS: " +
						Snapshot.Path.string();
					return false;
				}
				m_DependencyHandles.push_back(Handle);
				std::string CommitDependencyBytes;
				if (!Read_File(Snapshot.Path, CommitDependencyBytes) ||
					CommitDependencyBytes != Snapshot.SourceBytes)
				{
					strOutStatus =
						"Valtan pattern Sound Save rejected stale joined owners at commit; Encounter/Animation bytes changed while staging: " +
						Snapshot.Path.string();
					return false;
				}
			}

			std::string CurrentSoundBytes;
			if (!Read_File(SoundDestination, CurrentSoundBytes) ||
				CurrentSoundBytes != ExpectedSoundSourceBytes)
			{
				strOutStatus =
					"Valtan pattern Sound source changed during save; previous/current source preserved.";
				return false;
			}
			return true;
		}

	private:
		HANDLE m_Mutex = nullptr;
		bool_t m_ownsMutex = false;
		std::vector<HANDLE> m_DependencyHandles;
	};

#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	void Invoke_SoundDependencyPrecommitMutationTestHook()
	{
		constexpr wchar_t TEST_HOOK[] =
			L"LOSTARK_TEST_VALTAN_SOUND_MUTATE_DEPENDENCY_BEFORE_COMMIT";
		const DWORD Required =
			GetEnvironmentVariableW(TEST_HOOK, nullptr, 0u);
		if (Required <= 1u)
			return;
		std::vector<wchar_t> Value(Required);
		const DWORD Copied = GetEnvironmentVariableW(
			TEST_HOOK, Value.data(), Required);
		if (0u == Copied || Copied >= Required)
			return;
		std::ofstream Mutation(
			std::filesystem::path(Value.data()),
			std::ios::binary | std::ios::app);
		Mutation.put(' ');
		Mutation.flush();
	}
#endif
}

bool_t Client::VALTAN_PATTERN_SOUND_SOURCE_RECEIPT::Is_Valid() const
{
	return 64u == strSha256.size() && 0u != iBytes &&
		std::all_of(strSha256.begin(), strSha256.end(),
			[](const char_t Character)
			{
				return (Character >= '0' && Character <= '9') ||
					(Character >= 'a' && Character <= 'f');
			});
}

Client::CValtanPatternSoundSourceReadAdmission::
	~CValtanPatternSoundSourceReadAdmission()
{
	delete static_cast<VALTAN_PATTERN_SOUND_SOURCE_READ_STATE*>(m_pState);
	m_pState = nullptr;
}

bool_t Client::CValtanPatternSoundSourceReadAdmission::Acquire(
	VALTAN_PATTERN_SOUND_SOURCE_RECEIPT& OutReceipt,
	std::string& strOutStatus)
{
	if (nullptr != m_pState)
	{
		strOutStatus =
			"Valtan Pattern Sound source read admission is already held.";
		return false;
	}

	std::unique_ptr<VALTAN_PATTERN_SOUND_SOURCE_READ_STATE> State =
		std::make_unique<VALTAN_PATTERN_SOUND_SOURCE_READ_STATE>();
	State->Mutex = CreateMutexW(
		nullptr, FALSE, SOUND_AUTHORING_SAVE_MUTEX);
	if (nullptr == State->Mutex)
	{
		strOutStatus =
			"Valtan Pattern Sound playback could not create its source admission mutex.";
		return false;
	}
	const DWORD WaitResult = WaitForSingleObject(State->Mutex, 5000u);
	if (WAIT_OBJECT_0 != WaitResult && WAIT_ABANDONED != WaitResult)
	{
		strOutStatus =
			"Valtan Pattern Sound playback timed out waiting for the source owner.";
		return false;
	}
	State->OwnsMutex = true;

	const std::filesystem::path Path =
		CValtanPatternSoundCueDocument::Resolve_Path();
	State->File = CreateFileW(
		Path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
	if (Path.empty() || INVALID_HANDLE_VALUE == State->File)
	{
		strOutStatus =
			"Valtan Pattern Sound playback could not lock the exact source against replacement: " +
			Path.string() + ".";
		return false;
	}

	LARGE_INTEGER FileBytes{};
	if (FALSE == GetFileSizeEx(State->File, &FileBytes) ||
		FileBytes.QuadPart <= 0 ||
		static_cast<std::uint64_t>(FileBytes.QuadPart) > MAX_SOUND_SOURCE_BYTES)
	{
		strOutStatus =
			"Valtan Pattern Sound source size is invalid for exact playback admission.";
		return false;
	}
	std::string Bytes(static_cast<std::size_t>(FileBytes.QuadPart), '\0');
	std::size_t Offset = 0u;
	while (Offset < Bytes.size())
	{
		const DWORD Requested = static_cast<DWORD>((std::min)(
			Bytes.size() - Offset,
			static_cast<std::size_t>((std::numeric_limits<DWORD>::max)())));
		DWORD Read = 0u;
		if (FALSE == ReadFile(
				State->File, Bytes.data() + Offset, Requested, &Read, nullptr) ||
			0u == Read)
		{
			strOutStatus =
				"Valtan Pattern Sound playback could not read the locked source bytes.";
			return false;
		}
		Offset += Read;
	}

	VALTAN_PATTERN_SOUND_SOURCE_RECEIPT StagedReceipt;
	if (!BuildSoundSourceReceipt(Bytes, StagedReceipt) ||
		!StagedReceipt.Is_Valid())
	{
		strOutStatus =
			"Valtan Pattern Sound playback could not hash the locked source bytes.";
		return false;
	}
	m_pState = State.release();
	OutReceipt = std::move(StagedReceipt);
	strOutStatus =
		"Locked the exact Valtan Pattern Sound source generation for playback submission.";
	return true;
}

std::filesystem::path
Client::CValtanPatternSoundCueDocument::Resolve_Path()
{
	return CProjectDataRoot::Resolve(
		std::filesystem::path(L"Animation") / L"Authored" / L"Valtan" /
		L"Valtan.patternsoundcues.json");
}

bool_t Client::CValtanPatternSoundCueDocument::Parse_Text(
	const std::string_view Text,
	const CEncounterPatternReference& Encounter,
	const BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT& AnimationBindings,
	VALTAN_PATTERN_SOUND_CUE_DOCUMENT& InOutDocument,
	std::string& strOutStatus)
{
	if (!Encounter.Is_Ready() ||
		Encounter.Get_BossArchetypeId() != OWNER_ARCHETYPE_ID ||
		AnimationBindings.strBossArchetypeId != OWNER_ARCHETYPE_ID)
	{
		strOutStatus =
			"Valtan pattern Sound cues require the validated Valtan encounter.";
		return false;
	}

	DATA_JSON_VALUE Root;
	std::string ParseError;
	DATA_JSON_PARSE_LIMITS Limits{};
	Limits.iMaximumBytes = 512u * 1024u;
	Limits.iMaximumDepth = 8u;
	Limits.iMaximumValues = 8192u;
	if (!CDataJson::Parse(Text, Root, ParseError, Limits) ||
		!Is_ExactObject(Root,
			{ "schema", "formatVersion", "ownerArchetypeId", "cues" }))
	{
		strOutStatus = "Valtan pattern Sound cue JSON is malformed: " +
			ParseError;
		return false;
	}

	const DATA_JSON_VALUE* pSchema = Root.Find("schema");
	const DATA_JSON_VALUE* pOwner = Root.Find("ownerArchetypeId");
	const DATA_JSON_VALUE* pCues = Root.Find("cues");
	uint32_t iFormatVersion = 0u;
	if (nullptr == pSchema || !pSchema->Is_String() ||
		pSchema->Get_String() != SCHEMA ||
		!Read_Unsigned(Root, "formatVersion", FORMAT_VERSION,
			iFormatVersion) || iFormatVersion != FORMAT_VERSION ||
		nullptr == pOwner || !pOwner->Is_String() ||
		pOwner->Get_String() != OWNER_ARCHETYPE_ID ||
		pOwner->Get_String() != Encounter.Get_BossArchetypeId() ||
		nullptr == pCues || !pCues->Is_Array() ||
		pCues->Get_Array().empty() ||
		pCues->Get_Array().size() > MAX_CUE_COUNT)
	{
		strOutStatus = "Valtan pattern Sound cue header is invalid.";
		return false;
	}

	VALTAN_PATTERN_SOUND_CUE_DOCUMENT Staged;
	Staged.iFormatVersion = iFormatVersion;
	Staged.strOwnerArchetypeId = pOwner->Get_String();
	Staged.Cues.reserve(pCues->Get_Array().size());
	std::unordered_set<std::string> BindingIds;
	std::unordered_set<std::string> OccurrenceIds;
	std::unordered_set<std::string> ActionClipOccurrenceTuples;
	size_t iSkippedUnimplementedPatternCount = 0u;
	size_t iSkippedSuppressedAnimationCount = 0u;
	for (const DATA_JSON_VALUE& CueValue : pCues->Get_Array())
	{
		if (!Is_ExactObject(CueValue,
				{ "bindingId", "occurrenceId", "patternId", "stageId",
				  "actionId", "clipOccurrenceId", "soundBank", "soundEvent",
				  "repeatPolicy", "startMs" }))
		{
			strOutStatus = "Valtan pattern Sound cue has unexpected properties.";
			return false;
		}

		VALTAN_PATTERN_SOUND_CUE Cue;
		if (!Read_String(CueValue, "bindingId", Cue.strBindingId) ||
			!Read_String(CueValue, "occurrenceId", Cue.strOccurrenceId) ||
			!Read_String(CueValue, "patternId", Cue.strPatternId) ||
			!Read_String(CueValue, "stageId", Cue.strStageId) ||
			!Read_String(CueValue, "actionId", Cue.strActionId) ||
			!Read_String(CueValue, "clipOccurrenceId", Cue.strClipOccurrenceId) ||
			!Read_String(CueValue, "soundBank", Cue.strSoundBank) ||
			!Read_String(CueValue, "soundEvent", Cue.strSoundEvent) ||
			!Read_RepeatPolicy(CueValue, Cue.eRepeatPolicy) ||
			!Read_Unsigned(CueValue, "startMs",
				CEncounterPatternReference::MAX_STAGE_DURATION_MS,
				Cue.iStartMs) ||
			!BindingIds.insert(Cue.strBindingId).second ||
			!OccurrenceIds.insert(Cue.strOccurrenceId).second)
		{
			strOutStatus =
				"Valtan pattern Sound cue identity or policy is invalid.";
			return false;
		}

		/* Only an explicitly suppressed, known action (NONE + []) can leave a
		stale sound occurrence behind. Typos/unknown actions and broken active
		clip joins remain corrupt data and must preserve the previous document. */
		const BOSS_PATTERN_ANIMATION_BINDING* pAnimationBinding =
			Find_ActionBinding(AnimationBindings, Cue.strActionId);
		if (nullptr == pAnimationBinding)
		{
			strOutStatus = "Valtan pattern Sound cue action has no animation binding: " +
				Cue.strActionId;
			return false;
		}
		const bool_t isSuppressedAnimation =
			pAnimationBinding->bSuppressAnimation && pAnimationBinding->Clips.empty();
		if (pAnimationBinding->bSuppressAnimation && !pAnimationBinding->Clips.empty())
		{
			strOutStatus = "Valtan suppressed animation binding unexpectedly declares clips: " +
				Cue.strActionId;
			return false;
		}
		// Explicit NONE retires the entire animation tuple: its old encounter
		// stage can also have been removed (sequence.four.step-02).
		if (isSuppressedAnimation)
		{
			++iSkippedSuppressedAnimationCount;
			continue;
		}
		const BOSS_PATTERN_ANIMATION_CLIP* pAnimationClip =
			Find_ClipOccurrence(*pAnimationBinding, Cue.strClipOccurrenceId);
		if (nullptr == pAnimationClip)
		{
			strOutStatus = "Valtan pattern Sound cue clip occurrence is not owned by its action: " +
				Cue.strOccurrenceId;
			return false;
		}
		if (VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP ==
				Cue.eRepeatPolicy && !pAnimationClip->bLoop)
		{
			strOutStatus = "Valtan pattern Sound each_loop cue references a non-loop clip: " +
				Cue.strOccurrenceId;
			return false;
		}

		/* A pattern authored at the animation/sound-cue layer but not yet wired into
		Data/Encounters/Valtan/ValtanEncounter.json (real Server hit shape/damage/motion
		not authored yet -- e.g. VALTAN_ARENA_BREAK_33/_84) is not a corrupt document,
		just content that has not reached that layer yet. Skip only this cue instead of
		fail-closing every other real cue in the same document. */
		const ENCOUNTER_PATTERN_REFERENCE* pPattern =
			Encounter.Find_Pattern(Cue.strPatternId);
		if (nullptr == pPattern)
		{
			OutputDebugStringA(("[Client][Valtan] pattern Sound cue skipped -- "
				"pattern not yet in Encounter: " + Cue.strPatternId + "\n").c_str());
			++iSkippedUnimplementedPatternCount;
			continue;
		}
		const auto Stage = std::find_if(pPattern->stages.begin(),
			pPattern->stages.end(), [&Cue](const ENCOUNTER_STAGE_REFERENCE& Value)
			{
				return Value.stageId == Cue.strStageId;
			});
		if (pPattern->stages.end() == Stage ||
			Stage->actionId != Cue.strActionId || 0u == Stage->iDurationMs)
		{
			strOutStatus = "Valtan pattern Sound cue encounter tuple is invalid: " + Cue.strBindingId;
			return false;
		}
		Cue.iStageIndex = static_cast<uint32_t>(Stage - pPattern->stages.begin());
		Cue.iStageDurationMs = Stage->iDurationMs;

		const uint64_t iSegmentEndMs =
			static_cast<uint64_t>(pAnimationClip->iSourceStartMs) +
			static_cast<uint64_t>(pAnimationClip->iPlayMs);
		if (Cue.iStartMs < pAnimationClip->iSourceStartMs ||
			(0u != pAnimationClip->iPlayMs && Cue.iStartMs >= iSegmentEndMs))
		{
			strOutStatus =
				"Valtan pattern Sound cue source window is outside its clip segment: " +
				Cue.strOccurrenceId;
			return false;
		}

		const std::string Tuple = Cue.strActionId + "\n" +
			Cue.strClipOccurrenceId + "\n" + Cue.strOccurrenceId;
		if (!ActionClipOccurrenceTuples.insert(Tuple).second)
		{
			strOutStatus = "Duplicate Valtan action/clip/Sound cue occurrence tuple.";
			return false;
		}
		Staged.Cues.push_back(std::move(Cue));
	}

	std::sort(Staged.Cues.begin(), Staged.Cues.end(),
		[](const VALTAN_PATTERN_SOUND_CUE& Left,
			const VALTAN_PATTERN_SOUND_CUE& Right)
		{
			return std::tie(Left.strActionId,
				Left.strClipOccurrenceId, Left.iStartMs,
				Left.strOccurrenceId) <
				std::tie(Right.strActionId,
					Right.strClipOccurrenceId, Right.iStartMs,
					Right.strOccurrenceId);
		});
	InOutDocument = std::move(Staged);
	strOutStatus = "Parsed " + std::to_string(InOutDocument.Cues.size()) +
		" clip-occurrence-qualified Valtan pattern Sound cue(s), skipped " +
		std::to_string(iSkippedUnimplementedPatternCount) +
		" not-yet-implemented-pattern cue(s), " +
		std::to_string(iSkippedSuppressedAnimationCount) +
		" explicitly suppressed-animation cue(s).";
	return true;
}

bool_t Client::CValtanPatternSoundCueDocument::Load_Source(
	VALTAN_PATTERN_SOUND_CUE_DOCUMENT& InOutDocument,
	std::string& strOutStatus)
{
	VALTAN_PATTERN_SOUND_SOURCE_RECEIPT IgnoredReceipt;
	return Load_Source(InOutDocument, IgnoredReceipt, strOutStatus);
}

bool_t Client::CValtanPatternSoundCueDocument::Load_Source(
	VALTAN_PATTERN_SOUND_CUE_DOCUMENT& InOutDocument,
	VALTAN_PATTERN_SOUND_SOURCE_RECEIPT& InOutReceipt,
	std::string& strOutStatus)
{
	CValtanPatternSoundSourceReadAdmission SourceAdmission;
	VALTAN_PATTERN_SOUND_SOURCE_RECEIPT StagedReceipt;
	if (!SourceAdmission.Acquire(StagedReceipt, strOutStatus))
		return false;
	CEncounterPatternReference Encounter;
	if (!Load_EncounterReference(Encounter, strOutStatus))
		return false;
	const std::filesystem::path Path = Resolve_Path();
	std::ifstream Input(Path, std::ios::binary);
	if (Path.empty() || !Input)
	{
		strOutStatus =
			"Missing Valtan pattern Sound cue document: " + Path.string();
		return false;
	}
	const std::string Text{
		std::istreambuf_iterator<char>(Input),
		std::istreambuf_iterator<char>() };
	BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT AnimationBindings;
	if (!Load_AnimationBindings(AnimationBindings, strOutStatus))
		return false;
	VALTAN_PATTERN_SOUND_CUE_DOCUMENT Staged;
	if (!Parse_Text(Text, Encounter, AnimationBindings, Staged, strOutStatus) ||
		!Validate_CatalogAssets(Staged, strOutStatus))
		return false;
	InOutDocument = std::move(Staged);
	InOutReceipt = std::move(StagedReceipt);
	return true;
}

bool_t Client::CValtanPatternSoundCueDocument::Load_AuthoringSource(
	VALTAN_PATTERN_SOUND_CUE_DOCUMENT& InOutDocument,
	std::string& strOutStatus)
{
	std::string IgnoredBaselineSourceBytes;
	return Load_ForAuthoring(InOutDocument, IgnoredBaselineSourceBytes,
		strOutStatus);
}

bool_t Client::CValtanPatternSoundCueDocument::Load_ForAuthoring(
	VALTAN_PATTERN_SOUND_CUE_DOCUMENT& InOutDocument,
	std::string& InOutBaselineSourceBytes,
	std::string& strOutStatus)
{
	CEncounterPatternReference Encounter;
	if (!Load_EncounterReference(Encounter, strOutStatus))
		return false;
	BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT AnimationBindings;
	if (!Load_AnimationBindings(AnimationBindings, strOutStatus))
		return false;
	std::string Text;
	const std::filesystem::path Path = Resolve_Path();
	if (!Read_File(Path, Text))
	{
		strOutStatus =
			"Missing Valtan pattern Sound cue authoring source: " +
			Path.string();
		return false;
	}
	VALTAN_PATTERN_SOUND_CUE_DOCUMENT Staged;
	if (!Parse_AuthoringText(Text, Encounter, AnimationBindings,
			Staged, strOutStatus))
	{
		return false;
	}
	InOutDocument = std::move(Staged);
	InOutBaselineSourceBytes = std::move(Text);
	strOutStatus = "Loaded all " +
		std::to_string(InOutDocument.Cues.size()) +
		" strictly joined Valtan pattern Sound cue(s) with an exact authoring baseline.";
	return true;
}

bool_t Client::CValtanPatternSoundCueDocument::Validate_Joined(
	const VALTAN_PATTERN_SOUND_CUE_DOCUMENT& Document,
	const CEncounterPatternReference& Encounter,
	const BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT& AnimationBindings,
	const std::unordered_map<std::string, f32_t>&
		ClipSourceDurationSecondsByName,
	std::string& strOutStatus)
{
	if (FORMAT_VERSION != Document.iFormatVersion ||
		OWNER_ARCHETYPE_ID != Document.strOwnerArchetypeId ||
		Document.Cues.empty() || Document.Cues.size() > MAX_CUE_COUNT)
	{
		strOutStatus =
			"Valtan pattern Sound authoring document header is invalid.";
		return false;
	}
	VALTAN_PATTERN_SOUND_CUE_DOCUMENT Staged;
	if (!Parse_AuthoringText(Serialize_Document(Document), Encounter,
			AnimationBindings, Staged, strOutStatus))
	{
		return false;
	}
	if (Staged != Document)
	{
		strOutStatus =
			"Valtan pattern Sound draft changed derived join state or canonical row order.";
		return false;
	}
	if (!Validate_RuntimeTiming(Staged, AnimationBindings,
		ClipSourceDurationSecondsByName, strOutStatus))
	{
		return false;
	}
	strOutStatus = "Validated " + std::to_string(Document.Cues.size()) +
		" encounter/animation/catalog/model-timeline-joined Valtan pattern Sound cue(s).";
	return true;
}

bool_t Client::CValtanPatternSoundCueDocument::Add_AuthoringRow(
	VALTAN_PATTERN_SOUND_CUE_DOCUMENT& InOutDocument,
	const VALTAN_PATTERN_SOUND_CUE_ADD_ROW& Row,
	const std::unordered_map<std::string, f32_t>&
		ClipSourceDurationSecondsByName,
	VALTAN_PATTERN_SOUND_CUE_ROW_ID& InOutCreatedRowId,
	std::string& strOutStatus)
{
	if (FORMAT_VERSION != InOutDocument.iFormatVersion ||
		OWNER_ARCHETYPE_ID != InOutDocument.strOwnerArchetypeId ||
		InOutDocument.Cues.empty() ||
		InOutDocument.Cues.size() >= MAX_CUE_COUNT ||
		!Is_StableId(Row.strPatternId) || !Is_StableId(Row.strStageId) ||
		!Is_StableId(Row.strActionId) ||
		!Is_StableId(Row.strClipOccurrenceId) ||
		!Is_StableId(Row.strSoundBank) || !Is_StableId(Row.strSoundEvent) ||
		(Row.eRepeatPolicy != VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE &&
		 Row.eRepeatPolicy != VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP) ||
		Row.iStartMs > CEncounterPatternReference::MAX_STAGE_DURATION_MS)
	{
		strOutStatus = "Valtan pattern Sound Add Row request is invalid.";
		return false;
	}

	CEncounterPatternReference Encounter;
	BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT AnimationBindings;
	if (!Load_EncounterReference(Encounter, strOutStatus) ||
		!Load_AnimationBindings(AnimationBindings, strOutStatus))
	{
		return false;
	}

	VALTAN_PATTERN_SOUND_CUE_DOCUMENT VerifiedCurrent;
	if (!Parse_AuthoringText(Serialize_Document(InOutDocument), Encounter,
		AnimationBindings, VerifiedCurrent, strOutStatus))
	{
		strOutStatus =
			"Valtan pattern Sound Add Row requires a valid staged document: " +
			strOutStatus;
		return false;
	}

	VALTAN_PATTERN_SOUND_CUE_ROW_ID CreatedRowId;
	if (!Allocate_AuthoringRowId(VerifiedCurrent,
		Row.strClipOccurrenceId, CreatedRowId, strOutStatus))
	{
		return false;
	}

	VALTAN_PATTERN_SOUND_CUE Added;
	Added.strBindingId = CreatedRowId.strBindingId;
	Added.strOccurrenceId = CreatedRowId.strOccurrenceId;
	Added.strPatternId = Row.strPatternId;
	Added.strStageId = Row.strStageId;
	Added.strActionId = Row.strActionId;
	Added.strClipOccurrenceId = Row.strClipOccurrenceId;
	Added.strSoundBank = Row.strSoundBank;
	Added.strSoundEvent = Row.strSoundEvent;
	Added.eRepeatPolicy = Row.eRepeatPolicy;
	Added.iStartMs = Row.iStartMs;

	VALTAN_PATTERN_SOUND_CUE_DOCUMENT Candidate = VerifiedCurrent;
	Candidate.Cues.push_back(std::move(Added));
	VALTAN_PATTERN_SOUND_CUE_DOCUMENT VerifiedCandidate;
	if (!Parse_AuthoringText(Serialize_Document(Candidate), Encounter,
		AnimationBindings, VerifiedCandidate, strOutStatus) ||
		VerifiedCandidate.Cues.size() != VerifiedCurrent.Cues.size() + 1u)
	{
		return false;
	}
	const auto Created = std::find_if(VerifiedCandidate.Cues.begin(),
		VerifiedCandidate.Cues.end(), [&CreatedRowId](const auto& Cue)
		{
			return Cue.strBindingId == CreatedRowId.strBindingId &&
				Cue.strOccurrenceId == CreatedRowId.strOccurrenceId;
		});
	if (VerifiedCandidate.Cues.end() == Created)
	{
		strOutStatus =
			"Valtan pattern Sound Add Row lost its deterministic stable identity.";
		return false;
	}
	VALTAN_PATTERN_SOUND_CUE_DOCUMENT StrongAdded;
	StrongAdded.iFormatVersion = VerifiedCandidate.iFormatVersion;
	StrongAdded.strOwnerArchetypeId = VerifiedCandidate.strOwnerArchetypeId;
	StrongAdded.Cues.push_back(*Created);
	if (!Validate_RuntimeTiming(StrongAdded, AnimationBindings,
		ClipSourceDurationSecondsByName, strOutStatus))
	{
		return false;
	}

	InOutDocument = std::move(VerifiedCandidate);
	InOutCreatedRowId = std::move(CreatedRowId);
	strOutStatus = "Added one strongly joined Valtan pattern Sound authoring row.";
	return true;
}

bool_t Client::CValtanPatternSoundCueDocument::Remove_AuthoringRow(
	VALTAN_PATTERN_SOUND_CUE_DOCUMENT& InOutDocument,
	const VALTAN_PATTERN_SOUND_CUE_ROW_ID& RowId,
	std::string& strOutStatus)
{
	if (!Is_StableId(RowId.strBindingId) ||
		!Is_StableId(RowId.strOccurrenceId) || InOutDocument.Cues.size() <= 1u)
	{
		strOutStatus = "Valtan pattern Sound Remove Row key is invalid.";
		return false;
	}
	std::size_t iExactIndex = InOutDocument.Cues.size();
	std::size_t iExactCount = 0u;
	bool_t bPartialIdentityMatch = false;
	for (std::size_t i = 0u; i < InOutDocument.Cues.size(); ++i)
	{
		const VALTAN_PATTERN_SOUND_CUE& Cue = InOutDocument.Cues[i];
		const bool_t bBindingMatch = Cue.strBindingId == RowId.strBindingId;
		const bool_t bOccurrenceMatch =
			Cue.strOccurrenceId == RowId.strOccurrenceId;
		if (bBindingMatch && bOccurrenceMatch)
		{
			iExactIndex = i;
			++iExactCount;
		}
		else if (bBindingMatch || bOccurrenceMatch)
		{
			bPartialIdentityMatch = true;
		}
	}
	if (1u != iExactCount || bPartialIdentityMatch)
	{
		strOutStatus =
			"Valtan pattern Sound Remove Row did not resolve one exact stable row.";
		return false;
	}

	VALTAN_PATTERN_SOUND_CUE_DOCUMENT Staged = InOutDocument;
	Staged.Cues.erase(Staged.Cues.begin() + iExactIndex);
	InOutDocument = std::move(Staged);
	strOutStatus = "Removed one exact Valtan pattern Sound authoring row.";
	return true;
}

bool_t Client::CValtanPatternSoundCueDocument::Save_Atomic(
	const VALTAN_PATTERN_SOUND_CUE_DOCUMENT& Document,
	const std::unordered_map<std::string, f32_t>&
		ClipSourceDurationSecondsByName,
	std::string& InOutBaselineSourceBytes,
	std::string& strOutStatus)
{
	/* Pattern Sound remains a separate typed source owner, but every saved row
	   is joined against the canonical Encounter/Animation Product generation.
	   Hold the established shared generation admission from the first baseline
	   read through destination commit.  Canonical exclusive writers therefore
	   cannot replace either dependency while this independent source stages,
	   validates and commits; the destination mutex/CAS below still serializes
	   concurrent Sound writers. */
	CValtanCanonicalProductReadAdmission CanonicalAdmission;
	std::string CanonicalAdmissionStatus;
	if (!CanonicalAdmission.Acquire(CanonicalAdmissionStatus))
	{
		strOutStatus =
			"Valtan pattern Sound Save could not join canonical dependency-generation admission; source and draft were preserved: " +
			CanonicalAdmissionStatus;
		return false;
	}

	const std::filesystem::path Destination = Resolve_Path();
	std::string PreviousBytes;
	if (Destination.empty() || InOutBaselineSourceBytes.empty() ||
		!Read_File(Destination, PreviousBytes) ||
		PreviousBytes != InOutBaselineSourceBytes)
	{
		strOutStatus =
			"Valtan pattern Sound source no longer matches the authoring baseline; reload before saving.";
		return false;
	}

	std::array<SOUND_JOINED_OWNER_SOURCE_SNAPSHOT, 2u>
		DependencySnapshots = {
			SOUND_JOINED_OWNER_SOURCE_SNAPSHOT{ Resolve_EncounterPath(), {} },
			SOUND_JOINED_OWNER_SOURCE_SNAPSHOT{
				CValtanPatternAnimationBindingDocument::Resolve_Path("Valtan"), {} }
		};
	if (DependencySnapshots[0u].Path.empty() ||
		DependencySnapshots[1u].Path.empty() ||
		!Read_File(DependencySnapshots[0u].Path,
			DependencySnapshots[0u].SourceBytes) ||
		!Read_File(DependencySnapshots[1u].Path,
			DependencySnapshots[1u].SourceBytes))
	{
		strOutStatus =
			"Valtan pattern Sound Save could not snapshot exact Encounter/Animation dependency bytes.";
		return false;
	}

	CEncounterPatternReference Encounter;
	if (!Encounter.Load(DependencySnapshots[0u].Path, strOutStatus))
		return false;
	std::string EncounterBytesAfterLoad;
	if (!Read_File(
			DependencySnapshots[0u].Path, EncounterBytesAfterLoad) ||
		EncounterBytesAfterLoad != DependencySnapshots[0u].SourceBytes)
	{
		strOutStatus =
			"Valtan pattern Sound Save rejected an Encounter dependency that changed during preflight.";
		return false;
	}
	BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT AnimationBindings;
	if (!Parse_AnimationBindings(
			DependencySnapshots[1u].SourceBytes,
			AnimationBindings, strOutStatus))
		return false;
	VALTAN_PATTERN_SOUND_CUE_DOCUMENT Current;
	if (!Parse_AuthoringText(PreviousBytes, Encounter, AnimationBindings,
			Current, strOutStatus) ||
		!Validate_InventoryTransition(Current, Document, AnimationBindings,
			ClipSourceDurationSecondsByName, strOutStatus))
	{
		return false;
	}

	const std::string Serialized = Serialize_Document(Document);
	VALTAN_PATTERN_SOUND_CUE_DOCUMENT VerifiedSerialization;
	if (!Parse_AuthoringText(Serialized, Encounter, AnimationBindings,
			VerifiedSerialization, strOutStatus) ||
		VerifiedSerialization != Document)
	{
		strOutStatus =
			"Valtan pattern Sound serialization failed strict joined verification: " +
			strOutStatus;
		return false;
	}
	if (!Validate_ChangedRuntimeTiming(Current, VerifiedSerialization,
		AnimationBindings, ClipSourceDurationSecondsByName, strOutStatus))
	{
		return false;
	}

	std::error_code DirectoryError;
	std::filesystem::create_directories(
		Destination.parent_path(), DirectoryError);
	if (DirectoryError)
	{
		strOutStatus =
			"Could not create the Valtan pattern Sound authoring directory: " +
			DirectoryError.message();
		return false;
	}
	const std::filesystem::path Temporary =
		Make_TemporaryPath(Destination);
	if (!Write_DurableFile(Temporary, Serialized, strOutStatus))
		return false;

	std::string TemporaryBytes;
	VALTAN_PATTERN_SOUND_CUE_DOCUMENT Reparsed;
	if (!Read_File(Temporary, TemporaryBytes) ||
		TemporaryBytes != Serialized ||
		!Parse_AuthoringText(TemporaryBytes, Encounter, AnimationBindings,
			Reparsed, strOutStatus) || Reparsed != Document ||
		!Validate_InventoryTransition(Current, Reparsed, AnimationBindings,
			ClipSourceDurationSecondsByName, strOutStatus) ||
		!Validate_ChangedRuntimeTiming(Current, Reparsed,
			AnimationBindings, ClipSourceDurationSecondsByName, strOutStatus))
	{
		Remove_Temporary(Temporary);
		strOutStatus =
			"Valtan pattern Sound sibling temp failed strict reload verification: " +
			strOutStatus;
		return false;
	}

#if defined(LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS)
	Invoke_SoundDependencyPrecommitMutationTestHook();
#endif
	SOUND_JOINED_OWNER_COMMIT_GUARD CommitGuard;
	if (!CommitGuard.Lock_AndVerify(
		DependencySnapshots, Destination,
		InOutBaselineSourceBytes, strOutStatus))
	{
		Remove_Temporary(Temporary);
		return false;
	}
	if (!CanonicalAdmission.Validate_StillCurrent(CanonicalAdmissionStatus))
	{
		Remove_Temporary(Temporary);
		strOutStatus =
			"Valtan pattern Sound Save lost canonical dependency-generation admission before destination commit; previous source preserved: " +
			CanonicalAdmissionStatus;
		return false;
	}
	if (!Commit_Temporary(Destination, Temporary))
	{
		const DWORD Error = GetLastError();
		Remove_Temporary(Temporary);
		strOutStatus =
			"Could not atomically replace the Valtan pattern Sound source (Win32 " +
			std::to_string(Error) + "); previous source preserved.";
		return false;
	}

	InOutBaselineSourceBytes = Serialized;
	strOutStatus = "Saved " + std::to_string(Document.Cues.size()) +
		" identity-preserving Valtan pattern Sound cue(s) and advanced the authoring baseline.";
	return true;
}
