#include "Effect_DirectAuthoredSourceIndex.h"

#include "DataJson.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cwchar>
#include <fstream>
#include <limits>
#include <sstream>
#include <string_view>
#include <system_error>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

namespace
{
	bool_t Read_TextFile(const std::filesystem::path& Path,
		std::string& strOutText, std::string& strOutStatus)
	{
		strOutText.clear();
		std::ifstream Input(Path, std::ios::binary);
		if (Path.empty() || !Input)
		{
			strOutStatus = "Could not open EffectCatalog.json: " +
				Path.string();
			return false;
		}
		std::ostringstream Buffer;
		Buffer << Input.rdbuf();
		if (!Input.good() && !Input.eof())
		{
			strOutStatus = "Could not read EffectCatalog.json: " +
				Path.string();
			return false;
		}
		strOutText = Buffer.str();
		return true;
	}

	bool_t Try_ParsePlayerOwner(const std::string_view strAssetId,
		LostArk::Shared::CHARACTER_CLASS_ID& eOutClass,
		LostArk::Shared::SKILL_ID& iOutSkillId)
	{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		struct OWNER_PREFIX final
		{
			std::string_view Prefix;
			CHARACTER_CLASS_ID eClass = CHARACTER_CLASS_ID::END;
		};
		static constexpr std::array<OWNER_PREFIX, 4u> Prefixes = {
			OWNER_PREFIX{ "effect.artist.skill.", CHARACTER_CLASS_ID::ARTIST },
			OWNER_PREFIX{ "effect.dimensionmaster.skill.",
				CHARACTER_CLASS_ID::DIMENSIONMASTER },
			OWNER_PREFIX{ "effect.lancemaster.skill.",
				CHARACTER_CLASS_ID::LANCE_MASTER },
			OWNER_PREFIX{ "effect.warlord.skill.",
				CHARACTER_CLASS_ID::WARLORD }
		};
		if (!strAssetId.ends_with(".unified"))
			return false;
		for (const OWNER_PREFIX& Owner : Prefixes)
		{
			if (!strAssetId.starts_with(Owner.Prefix))
				continue;
			const std::string_view Tail = strAssetId.substr(Owner.Prefix.size());
			const size_t iSkillEnd = Tail.find('.');
			const std::string_view SkillToken = Tail.substr(0u, iSkillEnd);
			if (SkillToken.empty())
				return false;
			uint32_t iSkillId = 0u;
			for (const char Character : SkillToken)
			{
				if (Character < '0' || Character > '9')
					return false;
				const uint32_t iDigit =
					static_cast<uint32_t>(Character - '0');
				if (iSkillId >
					((std::numeric_limits<uint32_t>::max)() - iDigit) / 10u)
				{
					return false;
				}
				iSkillId = iSkillId * 10u + iDigit;
			}
			if (0u == iSkillId)
				return false;
			eOutClass = Owner.eClass;
			iOutSkillId =
				static_cast<LostArk::Shared::SKILL_ID>(iSkillId);
			return true;
		}
		return false;
	}

	bool_t Try_DeriveAssetId(const std::filesystem::path& Path,
		std::string& strOutAssetId)
	{
		constexpr std::string_view EffectSuffix = ".effect.json";
		const std::string Name = Path.filename().string();
		if (!Name.ends_with(EffectSuffix))
			return false;
		std::string AssetId = Name.substr(
			0u, Name.size() - EffectSuffix.size());
		if (AssetId.empty() || AssetId.size() > 128u ||
			!std::all_of(AssetId.begin(), AssetId.end(),
				[](const char Character)
				{
					const unsigned char Value =
						static_cast<unsigned char>(Character);
					return 0 != std::isalnum(Value) || Character == '_' ||
						Character == '-' || Character == '.';
				}))
		{
			return false;
		}
		strOutAssetId = std::move(AssetId);
		return true;
	}

	bool_t PathComponentEquals(const std::filesystem::path& Left,
		const std::filesystem::path& Right)
	{
		const std::wstring LeftText = Left.native();
		const std::wstring RightText = Right.native();
#if defined(_WIN32)
		return 0 == _wcsicmp(LeftText.c_str(), RightText.c_str());
#else
		return LeftText == RightText;
#endif
	}

	bool_t Is_StrictDescendant(const std::filesystem::path& Root,
		const std::filesystem::path& Candidate)
	{
		auto RootIterator = Root.begin();
		auto CandidateIterator = Candidate.begin();
		for (; RootIterator != Root.end();
			++RootIterator, ++CandidateIterator)
		{
			if (CandidateIterator == Candidate.end() ||
				!PathComponentEquals(*RootIterator, *CandidateIterator))
			{
				return false;
			}
		}
		return CandidateIterator != Candidate.end();
	}
}

bool Client::CEffectDirectAuthoredSourceIndex::Build(
	const std::filesystem::path& CatalogPath,
	const std::filesystem::path& AuthoredRoot,
	const std::vector<EFFECT_DIRECT_AUTHORED_SCANNED_FILE>& ScannedFiles,
	const EFFECT_DIRECT_AUTHORED_OWNER_SET& ValidOwners,
	const EFFECT_DIRECT_AUTHORED_BOSS_OWNER_MAP& ValidBossOwners,
	EFFECT_DIRECT_AUTHORED_SOURCE_INDEX& InOutIndex,
	std::string& strOutStatus)
{
	std::string Text;
	std::string Error;
	if (CatalogPath.empty() || AuthoredRoot.empty() ||
		!Read_TextFile(CatalogPath, Text, Error))
	{
		strOutStatus = CatalogPath.empty() || AuthoredRoot.empty() ?
			"Direct authored source index path is empty." : Error;
		return false;
	}
	std::error_code CanonicalError;
	const std::filesystem::path CanonicalAuthoredRoot =
		std::filesystem::weakly_canonical(AuthoredRoot, CanonicalError);
	if (CanonicalError || CanonicalAuthoredRoot.empty() ||
		!std::filesystem::is_directory(CanonicalAuthoredRoot, CanonicalError) ||
		CanonicalError)
	{
		strOutStatus =
			"Direct authored source index could not canonicalize the Authored root.";
		return false;
	}
	DATA_JSON_VALUE Root;
	if (!CDataJson::Parse(Text, Root, Error) || !Root.Is_Object())
	{
		strOutStatus =
			"Could not parse EffectCatalog.json: " + Error;
		return false;
	}
	const DATA_JSON_VALUE* pVersion = Root.Find("formatVersion");
	const DATA_JSON_VALUE* pEffects = Root.Find("effects");
	if (Root.Get_Object().size() != 2u || nullptr == pVersion ||
		pVersion->Get_Type() != DATA_JSON_TYPE::NUMBER ||
		pVersion->Get_Number() != 1.0 || nullptr == pEffects ||
		pEffects->Get_Type() != DATA_JSON_TYPE::ARRAY)
	{
		strOutStatus = "EffectCatalog.json rejected the exact source-index root contract.";
		return false;
	}

	std::unordered_map<std::string, const EFFECT_DIRECT_AUTHORED_SCANNED_FILE*>
		ScannedByAssetId;
	ScannedByAssetId.reserve(ScannedFiles.size());
	for (const EFFECT_DIRECT_AUTHORED_SCANNED_FILE& File : ScannedFiles)
		ScannedByAssetId.try_emplace(File.strEffectAssetId, &File);

	EFFECT_DIRECT_AUTHORED_SOURCE_INDEX Staged;
	Staged.Entries.reserve(pEffects->Get_Array().size());
	std::unordered_set<std::string> DirectAssetIds;
	DirectAssetIds.reserve(pEffects->Get_Array().size());
	const auto RecordUnavailable = [&Staged](std::string Status)
	{
		++Staged.iUnavailableCount;
		if (Staged.strFirstUnavailable.empty())
			Staged.strFirstUnavailable = std::move(Status);
	};

	for (const DATA_JSON_VALUE& CatalogEntry : pEffects->Get_Array())
	{
		if (!CatalogEntry.Is_Object())
		{
			strOutStatus = "EffectCatalog.json contains a non-object row.";
			return false;
		}
		const DATA_JSON_VALUE* pPayloadKind = CatalogEntry.Find("payloadKind");
		if (nullptr == pPayloadKind)
			continue;
		if (pPayloadKind->Get_Type() != DATA_JSON_TYPE::STRING)
		{
			strOutStatus = "EffectCatalog.json contains a non-string payloadKind.";
			return false;
		}
		if (pPayloadKind->Get_String() != "DIRECT_AUTHORED_DOCUMENT_V13")
			continue;
		++Staged.iCatalogDirectCount;
		const DATA_JSON_VALUE* pAssetId = CatalogEntry.Find("effectAssetId");
		const DATA_JSON_VALUE* pAuthoringPath = CatalogEntry.Find("authoringPath");
		if (CatalogEntry.Get_Object().size() != 3u || nullptr == pAssetId ||
			pAssetId->Get_Type() != DATA_JSON_TYPE::STRING ||
			pAssetId->Get_String().empty() || nullptr == pAuthoringPath ||
			pAuthoringPath->Get_Type() != DATA_JSON_TYPE::STRING ||
			pAuthoringPath->Get_String().empty())
		{
			strOutStatus =
				"EffectCatalog.json contains a malformed direct-authored row.";
			return false;
		}
		const std::string strAssetId = pAssetId->Get_String();
		if (!DirectAssetIds.insert(strAssetId).second)
		{
			strOutStatus = "EffectCatalog.json contains duplicate Effect ID: " +
				strAssetId;
			return false;
		}

		EFFECT_DIRECT_AUTHORED_SOURCE_ENTRY Entry;
		Entry.strEffectAssetId = strAssetId;
		if (Try_ParsePlayerOwner(
				strAssetId, Entry.eCharacterClass, Entry.iSkillId))
		{
			Entry.eOwnerKind = EFFECT_DIRECT_AUTHORED_OWNER_KIND::PLAYER_SKILL;
			if (!ValidOwners.contains(
					std::make_pair(Entry.eCharacterClass, Entry.iSkillId)))
			{
				RecordUnavailable(
					"direct authored Effect ID has no PlayerSkills owner: " +
					strAssetId);
				continue;
			}
		}
		else
		{
			const auto BossOwner = ValidBossOwners.find(strAssetId);
			if (ValidBossOwners.end() == BossOwner ||
				BossOwner->second.strOwnerArchetypeId.empty() ||
				BossOwner->second.strPatternId.empty() ||
				BossOwner->second.strStageId.empty() ||
				BossOwner->second.strActionId.empty())
			{
				RecordUnavailable(
					"direct authored Effect ID has no stable player-skill or boss-pattern owner: " +
					strAssetId);
				continue;
			}
			Entry.eOwnerKind = EFFECT_DIRECT_AUTHORED_OWNER_KIND::BOSS_PATTERN;
			Entry.strOwnerArchetypeId =
				BossOwner->second.strOwnerArchetypeId;
			Entry.strPatternId = BossOwner->second.strPatternId;
			Entry.strStageId = BossOwner->second.strStageId;
			Entry.strActionId = BossOwner->second.strActionId;
		}

		const std::string strAuthoringPath = pAuthoringPath->Get_String();
		if (strAuthoringPath.find('\\') != std::string::npos ||
			strAuthoringPath.find(':') != std::string::npos ||
			strAuthoringPath.starts_with('/') ||
			strAuthoringPath.ends_with('/') ||
			strAuthoringPath.find("//") != std::string::npos)
		{
			RecordUnavailable("unsafe direct authoringPath: " + strAuthoringPath);
			continue;
		}
		const std::filesystem::path RelativePath(strAuthoringPath);
		if (RelativePath.empty() || RelativePath.is_absolute() ||
			RelativePath.has_root_path() ||
			RelativePath.generic_string() != strAuthoringPath)
		{
			RecordUnavailable(
				"noncanonical direct authoringPath: " + strAuthoringPath);
			continue;
		}
		std::vector<std::string> Components;
		for (const std::filesystem::path& Component : RelativePath)
		{
			const std::string Token = Component.generic_string();
			if (Token.empty() || Token == "." || Token == "..")
			{
				Components.clear();
				break;
			}
			Components.push_back(Token);
		}
		std::string strDerivedAssetId;
		if (Components.size() < 3u || Components[0] != "Effects" ||
			Components[1] != "Authored" ||
			!Try_DeriveAssetId(RelativePath, strDerivedAssetId) ||
			strDerivedAssetId != strAssetId)
		{
			RecordUnavailable(
				"direct authoringPath/Effect ID mismatch: " + strAssetId);
			continue;
		}
		std::filesystem::path ExpectedPath = CanonicalAuthoredRoot;
		for (size_t iComponent = 2u; iComponent < Components.size();
			++iComponent)
		{
			ExpectedPath /= Components[iComponent];
		}
		std::error_code FileError;
		FileError.clear();
		ExpectedPath =
			std::filesystem::weakly_canonical(ExpectedPath, FileError);
		if (FileError || ExpectedPath.empty() ||
			!Is_StrictDescendant(CanonicalAuthoredRoot, ExpectedPath) ||
			!std::filesystem::is_regular_file(ExpectedPath, FileError) ||
			FileError)
		{
			RecordUnavailable(
				"direct authoringPath escaped Data/Effects/Authored: " + strAssetId);
			continue;
		}
		const auto Scanned = ScannedByAssetId.find(strAssetId);
		if (Scanned == ScannedByAssetId.end())
		{
			RecordUnavailable(
				"direct authored file is absent from the Authored scan: " +
				strAssetId);
			continue;
		}
		FileError.clear();
		if (!std::filesystem::equivalent(
				ExpectedPath, Scanned->second->Path, FileError) || FileError)
		{
			RecordUnavailable(
				"direct authoringPath disagrees with the Authored scan: " +
				strAssetId);
			continue;
		}
		Entry.LastWriteTime =
			std::filesystem::last_write_time(ExpectedPath, FileError);
		if (FileError)
		{
			RecordUnavailable(
				"direct authored timestamp is unavailable: " + strAssetId);
			continue;
		}
		Entry.iFileSize = static_cast<uint64_t>(
			std::filesystem::file_size(ExpectedPath, FileError));
		if (FileError)
		{
			RecordUnavailable(
				"direct authored file size is unavailable: " + strAssetId);
			continue;
		}
		Entry.Path = ExpectedPath;
		Staged.Entries.push_back(std::move(Entry));
	}

	std::sort(Staged.Entries.begin(), Staged.Entries.end(),
		[](const EFFECT_DIRECT_AUTHORED_SOURCE_ENTRY& Left,
			const EFFECT_DIRECT_AUTHORED_SOURCE_ENTRY& Right)
		{
			return std::tie(Left.eOwnerKind, Left.eCharacterClass,
				Left.iSkillId, Left.strOwnerArchetypeId, Left.strPatternId,
				Left.strStageId, Left.strActionId, Left.strEffectAssetId) <
				std::tie(Right.eOwnerKind, Right.eCharacterClass,
					Right.iSkillId, Right.strOwnerArchetypeId,
					Right.strPatternId, Right.strStageId,
					Right.strActionId, Right.strEffectAssetId);
		});
	strOutStatus = "Direct authored source index admitted " +
		std::to_string(Staged.Entries.size()) + " / " +
		std::to_string(Staged.iCatalogDirectCount) +
		" DIRECT_AUTHORED_DOCUMENT_V13 source paths.";
	if (0u != Staged.iUnavailableCount)
	{
		strOutStatus += " Isolated " +
			std::to_string(Staged.iUnavailableCount) +
			" unavailable rows; first: " + Staged.strFirstUnavailable;
	}
	InOutIndex = std::move(Staged);
	return true;
}
