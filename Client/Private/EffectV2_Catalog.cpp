#include "EffectV2_Catalog.h"
#include "EffectV2_Runtime.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <map>
#include <utility>

namespace
{
	constexpr const char* BOSS_VALTAN_ARCHETYPE_ID = "BOSS_VALTAN";
	constexpr std::string_view BOSS_VALTAN_RESOURCE_PREFIX = "boss.valtan.";
	constexpr std::string_view DOCUMENT_SUFFIX = ".effectv2.json";
	constexpr std::string_view GROUP_SUFFIX = ".effectv2group.json";
	constexpr uint32_t MAX_BINDING_MS = 600000u;

	bool_t Fail(std::string& strOutError, std::string strMessage)
	{
		strOutError = std::move(strMessage);
		return false;
	}

	void Diagnose(
		std::vector<std::string>& Diagnostics,
		std::string strMessage)
	{
		Diagnostics.push_back(std::move(strMessage));
	}

	bool_t Read_TextFile(
		const std::filesystem::path& Path,
		std::string& strOutText,
		std::string& strOutError)
	{
		std::ifstream Stream(Path, std::ios::binary);
		if (!Stream)
		{
			return Fail(strOutError,
				"Cannot open: " + Path.string());
		}
		strOutText.assign(
			std::istreambuf_iterator<char>(Stream),
			std::istreambuf_iterator<char>());
		if (!Stream.good() && !Stream.eof())
		{
			return Fail(strOutError,
				"Cannot read: " + Path.string());
		}
		return true;
	}

	bool_t Enumerate_StableIds(
		const std::filesystem::path& Directory,
		const std::string_view Suffix,
		std::vector<std::string>& OutIds,
		std::vector<std::string>& Diagnostics,
		std::string& strOutError)
	{
		OutIds.clear();
		std::error_code Error;
		if (Directory.empty() ||
			!std::filesystem::is_directory(Directory, Error) || Error)
		{
			return Fail(strOutError,
				"Effect V2 catalog directory is unavailable: " +
				Directory.string());
		}

		std::filesystem::directory_iterator Iterator(
			Directory, std::filesystem::directory_options::none, Error);
		if (Error)
		{
			return Fail(strOutError,
				"Effect V2 catalog directory could not be opened: " +
				Directory.string());
		}
		const std::filesystem::directory_iterator End;
		for (; Iterator != End; Iterator.increment(Error))
		{
			if (Error)
			{
				return Fail(strOutError,
					"Effect V2 catalog directory enumeration failed: " +
					Directory.string());
			}

			const std::filesystem::directory_entry& Entry = *Iterator;
			const std::string strFileName = Entry.path().filename().string();
			if (!strFileName.ends_with(Suffix))
				continue;

			const std::filesystem::file_status Status =
				Entry.symlink_status(Error);
			if (Error || !std::filesystem::is_regular_file(Status))
			{
				Diagnose(Diagnostics,
					"Skipped non-regular Effect V2 source: " +
					Entry.path().string());
				Error.clear();
				continue;
			}

			const std::string strId = strFileName.substr(
				0u, strFileName.size() - Suffix.size());
			if (!Client::CEffectV2Document::Is_ValidEffectId(strId))
			{
				Diagnose(Diagnostics,
					"Skipped Effect V2 source with invalid stable ID: " +
					strFileName);
				continue;
			}
			OutIds.push_back(strId);
		}
		if (Error)
		{
			return Fail(strOutError,
				"Effect V2 catalog directory enumeration failed: " +
				Directory.string());
		}

		std::sort(OutIds.begin(), OutIds.end());
		const auto Duplicate =
			std::adjacent_find(OutIds.begin(), OutIds.end());
		if (Duplicate != OutIds.end())
		{
			Diagnose(Diagnostics,
				"Skipped duplicate Effect V2 stable IDs in: " +
				Directory.string());
			OutIds.erase(std::unique(OutIds.begin(), OutIds.end()), OutIds.end());
		}
		return true;
	}

	bool_t Stage_Documents(
		std::vector<Client::EFFECT_V2_DOCUMENT>& OutDocuments,
		std::vector<std::string>& Diagnostics,
		std::string& strOutError)
	{
		std::vector<std::string> Ids;
		if (!Enumerate_StableIds(
			Client::CEffectV2Document::Document_Directory(),
			DOCUMENT_SUFFIX, Ids, Diagnostics, strOutError))
		{
			return false;
		}

		std::vector<Client::EFFECT_V2_DOCUMENT> Staged;
		Staged.reserve(Ids.size());
		for (const std::string& strEffectId : Ids)
		{
			Client::EFFECT_V2_DOCUMENT Document;
			if (!Client::CEffectV2Document::Load_DocumentFile(
				strEffectId, Document, strOutError))
			{
				Diagnose(Diagnostics,
					"Skipped Effect V2 document '" + strEffectId +
					"': " + strOutError);
				strOutError.clear();
				continue;
			}
			Staged.push_back(std::move(Document));
		}
		OutDocuments = std::move(Staged);
		return true;
	}

	bool_t Stage_Groups(
		std::vector<Client::EFFECT_V2_GROUP>& OutGroups,
		std::vector<std::string>& Diagnostics,
		std::string& strOutError)
	{
		std::vector<std::string> Ids;
		if (!Enumerate_StableIds(
			Client::CEffectV2Document::Group_Directory(),
			GROUP_SUFFIX, Ids, Diagnostics, strOutError))
		{
			return false;
		}

		std::vector<Client::EFFECT_V2_GROUP> Staged;
		Staged.reserve(Ids.size());
		for (const std::string& strGroupId : Ids)
		{
			Client::EFFECT_V2_GROUP Group;
			if (!Client::CEffectV2Document::Load_GroupFile(
				strGroupId, Group, strOutError))
			{
				Diagnose(Diagnostics,
					"Skipped Effect V2 group '" + strGroupId +
					"': " + strOutError);
				strOutError.clear();
				continue;
			}
			Staged.push_back(std::move(Group));
		}
		OutGroups = std::move(Staged);
		return true;
	}

	bool_t Stage_BossValtanBindings(
		std::vector<Client::EFFECT_V2_BINDING>& OutBindings,
		bool_t& bOutComplete,
		std::vector<std::string>& Diagnostics,
		std::string& strOutError,
		std::string* const pOutSourceBytes = nullptr)
	{
		OutBindings.clear();
		bOutComplete = false;
		(void)Diagnostics;
		if (nullptr != pOutSourceBytes)
			pOutSourceBytes->clear();
		const std::filesystem::path Path =
			Client::CEffectV2Document::Binding_Path(BOSS_VALTAN_ARCHETYPE_ID);
		std::string strText;
		if (!Read_TextFile(Path, strText, strOutError))
		{
			strOutError =
				"BOSS_VALTAN strict formatVersion 2 binding read failed before "
				"snapshot commit: " + strOutError;
			return false;
		}
		std::vector<Client::EFFECT_V2_BINDING> Staged;
		if (!Client::CEffectV2Document::Parse_Bindings(
				strText, BOSS_VALTAN_ARCHETYPE_ID, Staged, strOutError))
		{
			strOutError =
				"BOSS_VALTAN strict formatVersion 2 binding parse failed before "
				"snapshot commit: " + strOutError;
			return false;
		}

		OutBindings = std::move(Staged);
		bOutComplete = true;
		if (nullptr != pOutSourceBytes)
			*pOutSourceBytes = std::move(strText);
		return true;
	}

	bool_t Is_StableAsciiId(
		const std::string_view strId,
		const size_t iMaximumLength = 160u)
	{
		if (strId.empty() || strId.size() > iMaximumLength)
			return false;
		return std::all_of(strId.begin(), strId.end(), [](const char Value)
		{
			const unsigned char Character = static_cast<unsigned char>(Value);
			return 0 != std::isalnum(Character) || '.' == Value ||
				'_' == Value || '-' == Value;
		});
	}

	bool_t Is_ValidLocalTransform(
		const Client::EFFECT_V2_LOCAL_TRANSFORM& Transform)
	{
		return std::isfinite(Transform.vTranslation.x) &&
			std::isfinite(Transform.vTranslation.y) &&
			std::isfinite(Transform.vTranslation.z) &&
			std::isfinite(Transform.vRotation.x) &&
			std::isfinite(Transform.vRotation.y) &&
			std::isfinite(Transform.vRotation.z) &&
			std::isfinite(Transform.vScale.x) &&
			std::isfinite(Transform.vScale.y) &&
			std::isfinite(Transform.vScale.z) &&
			Transform.vScale.x > 0.f && Transform.vScale.y > 0.f &&
			Transform.vScale.z > 0.f;
	}

	bool_t Contains_BindingId(
		const std::vector<Client::EFFECT_V2_BINDING>& Bindings,
		const std::string_view strBindingId)
	{
		return Bindings.end() != std::find_if(
			Bindings.begin(), Bindings.end(), [strBindingId](
				const Client::EFFECT_V2_BINDING& Binding)
			{
				return Binding.strBindingId == strBindingId;
			});
	}

	uint64_t Hash_Fnv1a64(const std::string_view strValue)
	{
		uint64_t iHash = 14695981039346656037ull;
		for (const unsigned char Value : strValue)
		{
			iHash ^= static_cast<uint64_t>(Value);
			iHash *= 1099511628211ull;
		}
		return iHash;
	}

	std::string Generate_StableBindingId(
		const std::vector<Client::EFFECT_V2_BINDING>& Bindings,
		const Client::EFFECT_V2_BINDING& Binding)
	{
		Client::EFFECT_V2_BINDING HashBinding = Binding;
		HashBinding.strBindingId = "binding.valtan.pending";
		const uint64_t iHash = Hash_Fnv1a64(
			Client::CEffectV2Document::Serialize_Bindings(
				BOSS_VALTAN_ARCHETYPE_ID, { HashBinding }));
		for (uint32_t iSuffix = 0u; iSuffix < 100000u; ++iSuffix)
		{
			char szBindingId[96]{};
			if (0u == iSuffix)
			{
				std::snprintf(szBindingId, sizeof(szBindingId),
					"binding.valtan.authored.%016llx",
					static_cast<unsigned long long>(iHash));
			}
			else
			{
				std::snprintf(szBindingId, sizeof(szBindingId),
					"binding.valtan.authored.%016llx.%u",
					static_cast<unsigned long long>(iHash), iSuffix);
			}
			if (!Contains_BindingId(Bindings, szBindingId))
				return szBindingId;
		}
		return {};
	}

	bool_t Cross_Validate(
		const std::vector<Client::EFFECT_V2_DOCUMENT>& Documents,
		const std::vector<Client::EFFECT_V2_GROUP>& Groups,
		const std::vector<Client::EFFECT_V2_BINDING>& Bindings,
		std::string& strOutError)
	{
		std::map<std::string, const Client::EFFECT_V2_DOCUMENT*, std::less<>>
			DocumentsById;
		for (const Client::EFFECT_V2_DOCUMENT& Document : Documents)
		{
			if (!DocumentsById.emplace(Document.strEffectId, &Document).second)
			{
				return Fail(strOutError,
					"Effect V2 catalog has duplicate effectId: " +
					Document.strEffectId);
			}
		}

		std::map<std::string, const Client::EFFECT_V2_GROUP*, std::less<>> GroupsById;
		for (const Client::EFFECT_V2_GROUP& Group : Groups)
		{
			if (DocumentsById.contains(Group.strGroupId))
			{
				return Fail(strOutError,
					"Effect V2 groupId collides with an authored effect: " +
					Group.strGroupId);
			}
			if (!GroupsById.emplace(Group.strGroupId, &Group).second)
			{
				return Fail(strOutError,
					"Effect V2 catalog has duplicate groupId: " +
					Group.strGroupId);
			}
			for (const Client::EFFECT_V2_GROUP_CHILD& Child : Group.Children)
			{
				if (Client::EFFECT_V2_RESOURCE_KIND::GROUP ==
					Child.eResourceKind)
				{
					return Fail(strOutError,
						"Nested Effect V2 groups are not supported by the current "
						"runtime: " + Group.strGroupId + " -> " +
						Child.strResourceId);
				}
				if (Client::EFFECT_V2_RESOURCE_KIND::LEAF !=
						Child.eResourceKind ||
					!DocumentsById.contains(Child.strResourceId))
				{
					return Fail(strOutError,
						"Effect V2 group child has no authored leaf document: " +
						Group.strGroupId + " -> " + Child.strResourceId);
				}
			}
		}

		std::map<std::string, bool_t, std::less<>> BindingIds;
		for (const Client::EFFECT_V2_BINDING& Binding : Bindings)
		{
			if (!Is_StableAsciiId(Binding.strBindingId) ||
				!BindingIds.emplace(Binding.strBindingId, true).second)
			{
				return Fail(strOutError,
					"BOSS_VALTAN Effect V2 binding requires one unique stable bindingId.");
			}
			if ((Client::EFFECT_V2_RESOURCE_KIND::LEAF !=
					Binding.eResourceKind &&
				Client::EFFECT_V2_RESOURCE_KIND::GROUP !=
					Binding.eResourceKind) ||
				!Is_StableAsciiId(Binding.strResourceId,
					Client::EFFECT_V2_RESOURCE_KIND::LEAF ==
						Binding.eResourceKind ? 80u : 160u) ||
				!Binding.strResourceId.starts_with(BOSS_VALTAN_RESOURCE_PREFIX))
			{
				return Fail(strOutError,
					"BOSS_VALTAN Effect V2 binding has an invalid typed boss.valtan.* resource.");
			}
			if (!Is_StableAsciiId(Binding.strPatternId) ||
				!Is_StableAsciiId(Binding.strStageId) ||
				!Is_StableAsciiId(Binding.strActionId))
			{
				return Fail(strOutError,
					"BOSS_VALTAN Effect V2 binding has an invalid typed pattern/stage/action scope.");
			}
			if (Binding.iStartMs > MAX_BINDING_MS ||
				(Client::EFFECT_V2_CLOCK_BASIS::STAGE !=
					Binding.eClockBasis &&
				Client::EFFECT_V2_CLOCK_BASIS::CLIP_OCCURRENCE !=
					Binding.eClockBasis) ||
				(Client::EFFECT_V2_REPEAT_POLICY::ONCE !=
					Binding.eRepeatPolicy &&
				Client::EFFECT_V2_REPEAT_POLICY::EACH_LOOP !=
					Binding.eRepeatPolicy) ||
				(Client::EFFECT_V2_CLOCK_BASIS::STAGE ==
						Binding.eClockBasis &&
					(!Binding.strClipOccurrenceId.empty() ||
					 Client::EFFECT_V2_REPEAT_POLICY::ONCE !=
						Binding.eRepeatPolicy)) ||
				(Client::EFFECT_V2_CLOCK_BASIS::CLIP_OCCURRENCE ==
						Binding.eClockBasis &&
					!Is_StableAsciiId(Binding.strClipOccurrenceId)))
			{
				return Fail(strOutError,
					"BOSS_VALTAN Effect V2 binding has an invalid typed clock.");
			}
			if (!Is_StableAsciiId(Binding.strAnchorSlotId) ||
				(Client::EFFECT_V2_FOLLOW_POLICY::FOLLOW_SLOT !=
					Binding.eFollowPolicy &&
				Client::EFFECT_V2_FOLLOW_POLICY::SNAPSHOT_AT_START !=
					Binding.eFollowPolicy) ||
				(Client::EFFECT_V2_ROTATION_BASIS::SLOT !=
					Binding.eRotationBasis &&
				Client::EFFECT_V2_ROTATION_BASIS::TARGET_YAW !=
					Binding.eRotationBasis &&
				Client::EFFECT_V2_ROTATION_BASIS::WORLD !=
					Binding.eRotationBasis) ||
				!Is_ValidLocalTransform(Binding.LocalTransform))
			{
				return Fail(strOutError,
					"BOSS_VALTAN Effect V2 binding has an invalid typed anchor.");
			}
			if ((Client::EFFECT_V2_STOP_POLICY::NATURAL !=
					Binding.eStopPolicy &&
				Client::EFFECT_V2_STOP_POLICY::STAGE_END !=
					Binding.eStopPolicy &&
				Client::EFFECT_V2_STOP_POLICY::CLIP_OCCURRENCE_END !=
					Binding.eStopPolicy &&
				Client::EFFECT_V2_STOP_POLICY::EXPLICIT !=
					Binding.eStopPolicy) ||
				(Client::EFFECT_V2_STOP_POLICY::CLIP_OCCURRENCE_END ==
						Binding.eStopPolicy &&
				 Client::EFFECT_V2_CLOCK_BASIS::CLIP_OCCURRENCE !=
					Binding.eClockBasis))
			{
				return Fail(strOutError,
					"BOSS_VALTAN Effect V2 binding has an invalid typed stopPolicy.");
			}

			if (Client::EFFECT_V2_RESOURCE_KIND::LEAF ==
				Binding.eResourceKind)
			{
				if (!DocumentsById.contains(Binding.strResourceId))
				{
					return Fail(strOutError,
						"BOSS_VALTAN Effect V2 binding has no authored document: " +
						Binding.strResourceId);
				}
			}
			else if (!GroupsById.contains(Binding.strResourceId))
			{
				return Fail(strOutError,
					"BOSS_VALTAN Effect V2 binding has no group document: " +
					Binding.strResourceId);
			}
		}
		return true;
	}

	bool_t Matches_DocumentBaseline(
		const std::vector<Client::EFFECT_V2_DOCUMENT>& Left,
		const std::vector<Client::EFFECT_V2_DOCUMENT>& Right)
	{
		if (Left.size() != Right.size())
			return false;
		for (size_t iIndex = 0u; iIndex < Left.size(); ++iIndex)
		{
			if (Client::CEffectV2Document::Serialize_Document(Left[iIndex]) !=
				Client::CEffectV2Document::Serialize_Document(Right[iIndex]))
			{
				return false;
			}
		}
		return true;
	}

	bool_t Matches_GroupBaseline(
		const std::vector<Client::EFFECT_V2_GROUP>& Left,
		const std::vector<Client::EFFECT_V2_GROUP>& Right)
	{
		if (Left.size() != Right.size())
			return false;
		for (size_t iIndex = 0u; iIndex < Left.size(); ++iIndex)
		{
			if (Client::CEffectV2Document::Serialize_Group(Left[iIndex]) !=
				Client::CEffectV2Document::Serialize_Group(Right[iIndex]))
			{
				return false;
			}
		}
		return true;
	}

	bool_t Validate_StageBindingIdentity(
		const Client::EFFECT_V2_STAGE_BINDING_KEY& Key,
		std::string& strOutError)
	{
		if (!Is_StableAsciiId(Key.strBindingId))
		{
			return Fail(strOutError,
				"Effect V2 mutation requires one stable formatVersion 2 bindingId.");
		}
		return true;
	}

	bool_t Validate_StageBindingAppendRequest(
		const Client::EFFECT_V2_STAGE_BINDING_KEY& Key,
		std::string& strOutError)
	{
		if (!Is_StableAsciiId(
				Key.strResourceId, Key.bGroup ? 160u : 80u))
		{
			return Fail(strOutError,
				"Effect V2 append rejected an invalid stable resource ID.");
		}
		if (!Is_StableAsciiId(Key.strPatternId) ||
			!Is_StableAsciiId(Key.strStageId) ||
			!Is_StableAsciiId(Key.strActionId))
		{
			return Fail(strOutError,
				"Effect V2 append requires stable patternId, stageId and actionId scope fields.");
		}
		if (Key.iStartMs > MAX_BINDING_MS)
		{
			return Fail(strOutError,
				"Effect V2 append startMs exceeds the ten-minute authoring limit.");
		}
		return true;
	}

	bool_t Validate_BossValtanMutationSubject(
		const Client::EFFECT_V2_STAGE_BINDING_KEY& Key,
		std::string& strOutError)
	{
		if (!Key.strResourceId.starts_with(BOSS_VALTAN_RESOURCE_PREFIX))
		{
			return Fail(strOutError,
				"BOSS_VALTAN Effect V2 mutation requires a boss.valtan.* resource: " +
				Key.strResourceId);
		}
		return true;
	}

	bool_t Matches_StageBindingKey(
		const Client::EFFECT_V2_BINDING& Binding,
		const Client::EFFECT_V2_STAGE_BINDING_KEY& Key)
	{
		return Binding.strBindingId == Key.strBindingId;
	}

	bool_t Resolve_UniqueStageBindingIndex(
		const std::vector<Client::EFFECT_V2_BINDING>& Bindings,
		const Client::EFFECT_V2_STAGE_BINDING_KEY& Key,
		size_t& iOutIndex,
		std::string& strOutError)
	{
		size_t iMatchCount = 0u;
		for (size_t iIndex = 0u; iIndex < Bindings.size(); ++iIndex)
		{
			if (!Matches_StageBindingKey(Bindings[iIndex], Key))
				continue;
			iOutIndex = iIndex;
			++iMatchCount;
		}
		if (1u == iMatchCount)
			return true;
		if (0u == iMatchCount)
		{
			return Fail(strOutError,
				"Effect V2 Stage binding no longer exists. Refresh the catalog.");
		}
		return Fail(strOutError,
			"Effect V2 Stage binding identity is ambiguous. Refresh and repair the source.");
	}

	bool_t Validate_NoLeafGroupClockOverlap(
		const std::vector<Client::EFFECT_V2_GROUP>& Groups,
		const std::vector<Client::EFFECT_V2_BINDING>& Bindings,
		std::string& strOutError)
	{
		std::map<std::string, const Client::EFFECT_V2_GROUP*, std::less<>>
			GroupsById;
		for (const Client::EFFECT_V2_GROUP& Group : Groups)
			GroupsById.emplace(Group.strGroupId, &Group);

		for (const Client::EFFECT_V2_BINDING& LeafBinding : Bindings)
		{
			if (Client::EFFECT_V2_RESOURCE_KIND::LEAF !=
				LeafBinding.eResourceKind)
				continue;
			for (const Client::EFFECT_V2_BINDING& GroupBinding : Bindings)
			{
				if (Client::EFFECT_V2_RESOURCE_KIND::GROUP !=
						GroupBinding.eResourceKind ||
					LeafBinding.strPatternId != GroupBinding.strPatternId ||
					LeafBinding.strStageId != GroupBinding.strStageId ||
					LeafBinding.strActionId != GroupBinding.strActionId ||
					LeafBinding.eClockBasis != GroupBinding.eClockBasis ||
					LeafBinding.strClipOccurrenceId !=
						GroupBinding.strClipOccurrenceId ||
					LeafBinding.eRepeatPolicy != GroupBinding.eRepeatPolicy)
				{
					continue;
				}
				const auto Found = GroupsById.find(GroupBinding.strResourceId);
				if (Found == GroupsById.end())
					continue;
				for (const Client::EFFECT_V2_GROUP_CHILD& Child :
					Found->second->Children)
				{
					const uint64_t iEffectiveStartMs =
						static_cast<uint64_t>(GroupBinding.iStartMs) +
						static_cast<uint64_t>(Child.iStartMs);
					if (Client::EFFECT_V2_RESOURCE_KIND::LEAF ==
							Child.eResourceKind &&
						Child.strResourceId == LeafBinding.strResourceId &&
						iEffectiveStartMs == LeafBinding.iStartMs)
					{
						return Fail(strOutError,
							"Effect V2 leaf overlaps the same leaf inside group '" +
							GroupBinding.strResourceId +
							"' at the same typed clock: " +
							LeafBinding.strResourceId);
					}
				}
			}
		}
		return true;
	}

	bool_t Isolate_InvalidCrossReferences(
		const std::vector<Client::EFFECT_V2_DOCUMENT>& Documents,
		std::vector<Client::EFFECT_V2_GROUP>& Groups,
		std::vector<Client::EFFECT_V2_BINDING>& Bindings,
		bool_t& bInOutBindingsComplete,
		std::vector<std::string>& Diagnostics,
		std::string& strOutError)
	{
		if (!bInOutBindingsComplete)
		{
			return Fail(strOutError,
				"BOSS_VALTAN formatVersion 2 bindings cannot be admitted as a partial document.");
		}
		std::map<std::string, bool_t, std::less<>> DocumentIds;
		for (const Client::EFFECT_V2_DOCUMENT& Document : Documents)
			DocumentIds.emplace(Document.strEffectId, true);

		std::vector<Client::EFFECT_V2_GROUP> ValidGroups;
		ValidGroups.reserve(Groups.size());
		std::map<std::string, bool_t, std::less<>> GroupIds;
		for (Client::EFFECT_V2_GROUP& Group : Groups)
		{
			std::string strReason;
			if (DocumentIds.contains(Group.strGroupId))
			{
				strReason = "groupId collides with an authored effect";
			}
			else if (GroupIds.contains(Group.strGroupId))
			{
				strReason = "duplicate groupId";
			}
			else
			{
				for (const Client::EFFECT_V2_GROUP_CHILD& Child : Group.Children)
				{
					if (Client::EFFECT_V2_RESOURCE_KIND::GROUP ==
						Child.eResourceKind)
					{
						strReason =
							"nested groups are not supported by the current runtime: " +
							Child.strResourceId;
						break;
					}
					if (Client::EFFECT_V2_RESOURCE_KIND::LEAF !=
							Child.eResourceKind ||
						!DocumentIds.contains(Child.strResourceId))
					{
						strReason =
							"missing authored leaf " + Child.strResourceId;
						break;
					}
				}
			}

			if (!strReason.empty())
			{
				Diagnose(Diagnostics,
					"Skipped Effect V2 group '" + Group.strGroupId +
					"': " + strReason + '.');
				continue;
			}
			GroupIds.emplace(Group.strGroupId, true);
			ValidGroups.push_back(std::move(Group));
		}
		Groups = std::move(ValidGroups);

		return Cross_Validate(Documents, Groups, Bindings, strOutError) &&
			Validate_NoLeafGroupClockOverlap(Groups, Bindings, strOutError);
	}

	std::string Format_IsolationSummary(
		const std::vector<std::string>& Diagnostics)
	{
		if (Diagnostics.empty())
			return {};
		std::string strSummary =
			"Loaded the valid Effect V2 subset; isolated " +
			std::to_string(Diagnostics.size()) + " item(s).";
		const size_t iShown = (std::min)(Diagnostics.size(), size_t{ 3u });
		for (size_t iIndex = 0u; iIndex < iShown; ++iIndex)
			strSummary += " " + Diagnostics[iIndex];
		if (iShown < Diagnostics.size())
		{
			strSummary += " " +
				std::to_string(Diagnostics.size() - iShown) +
				" additional item(s) were isolated.";
		}
		return strSummary;
	}
}

uint64_t Client::EFFECT_V2_CATALOG_SNAPSHOT::Get_Revision() const noexcept
{
	return m_iRevision;
}

bool_t Client::EFFECT_V2_CATALOG_SNAPSHOT::Is_Ready() const noexcept
{
	return 0u != m_iRevision;
}

const std::vector<Client::EFFECT_V2_DOCUMENT>&
Client::EFFECT_V2_CATALOG_SNAPSHOT::Get_Documents() const noexcept
{
	return m_Documents;
}

const std::vector<Client::EFFECT_V2_GROUP>&
Client::EFFECT_V2_CATALOG_SNAPSHOT::Get_Groups() const noexcept
{
	return m_Groups;
}

const std::vector<Client::EFFECT_V2_BINDING>&
Client::EFFECT_V2_CATALOG_SNAPSHOT::Get_BossValtanBindings() const noexcept
{
	return m_BossValtanBindings;
}

const std::vector<std::string>&
Client::EFFECT_V2_CATALOG_SNAPSHOT::Get_Diagnostics() const noexcept
{
	return m_Diagnostics;
}

bool_t Client::EFFECT_V2_CATALOG_SNAPSHOT::Has_IsolatedItems() const noexcept
{
	return !m_Diagnostics.empty();
}

bool_t Client::EFFECT_V2_CATALOG_SNAPSHOT::Can_MutateBossValtanBindings() const noexcept
{
	return m_bBossValtanBindingsComplete;
}

const Client::EFFECT_V2_DOCUMENT*
Client::EFFECT_V2_CATALOG_SNAPSHOT::Find_Document(
	const std::string_view strEffectId) const noexcept
{
	const auto Found = std::lower_bound(
		m_Documents.begin(), m_Documents.end(), strEffectId,
		[](const EFFECT_V2_DOCUMENT& Document, const std::string_view Id)
		{
			return Document.strEffectId < Id;
		});
	return Found != m_Documents.end() && Found->strEffectId == strEffectId ?
		&*Found : nullptr;
}

const Client::EFFECT_V2_GROUP*
Client::EFFECT_V2_CATALOG_SNAPSHOT::Find_Group(
	const std::string_view strGroupId) const noexcept
{
	const auto Found = std::lower_bound(
		m_Groups.begin(), m_Groups.end(), strGroupId,
		[](const EFFECT_V2_GROUP& Group, const std::string_view Id)
		{
			return Group.strGroupId < Id;
		});
	return Found != m_Groups.end() && Found->strGroupId == strGroupId ?
		&*Found : nullptr;
}

Client::EFFECT_V2_STAGE_BINDING_KEY
Client::EFFECT_V2_STAGE_BINDING_KEY::From_Binding(
	const EFFECT_V2_BINDING& Binding)
{
	EFFECT_V2_STAGE_BINDING_KEY Key;
	Key.strBindingId = Binding.strBindingId;
	Key.bGroup = EFFECT_V2_RESOURCE_KIND::GROUP == Binding.eResourceKind;
	Key.strResourceId = Binding.strResourceId;
	Key.strPatternId = Binding.strPatternId;
	Key.strStageId = Binding.strStageId;
	Key.strActionId = Binding.strActionId;
	Key.iStartMs = Binding.iStartMs;
	return Key;
}

Client::EFFECT_V2_STAGE_BINDING_KEY
Client::EFFECT_V2_STAGE_BINDING_KEY::From_StageBinding(
	const EFFECT_V2_BINDING& Binding)
{
	return From_Binding(Binding);
}

Client::CEffectV2Catalog& Client::CEffectV2Catalog::Get()
{
	static CEffectV2Catalog Instance;
	return Instance;
}

Client::CEffectV2Catalog::CEffectV2Catalog()
	: m_pSnapshot(std::make_shared<const EFFECT_V2_CATALOG_SNAPSHOT>())
{
}

bool_t Client::CEffectV2Catalog::Reload_BossValtan(std::string& strOutError)
{
	try
	{
		{
			const std::lock_guard Lock(m_SnapshotMutex);
			if (m_bBossValtanBindingDraftDirty)
			{
				return Fail(strOutError,
					"Effect V2 bindings have an unsaved Composition draft. Use the Workbench Save before loading from disk.");
			}
		}
		std::vector<EFFECT_V2_DOCUMENT> StagedDocuments;
		std::vector<EFFECT_V2_GROUP> StagedGroups;
		std::vector<EFFECT_V2_BINDING> StagedBindings;
		std::vector<std::string> Diagnostics;
		bool_t bBindingsComplete = true;
		if (!Stage_Documents(
				StagedDocuments, Diagnostics, strOutError) ||
			!Stage_Groups(
				StagedGroups, Diagnostics, strOutError) ||
			!Stage_BossValtanBindings(
				StagedBindings, bBindingsComplete,
				Diagnostics, strOutError) ||
			!Isolate_InvalidCrossReferences(
				StagedDocuments, StagedGroups, StagedBindings,
				bBindingsComplete, Diagnostics, strOutError))
		{
			return false;
		}

		auto pStaged = std::make_shared<EFFECT_V2_CATALOG_SNAPSHOT>();
		pStaged->m_Documents = std::move(StagedDocuments);
		pStaged->m_Groups = std::move(StagedGroups);
		pStaged->m_BossValtanBindings = std::move(StagedBindings);
		pStaged->m_Diagnostics = std::move(Diagnostics);
		pStaged->m_bBossValtanBindingsComplete = bBindingsComplete;
		const std::string strIsolationSummary =
			Format_IsolationSummary(pStaged->m_Diagnostics);

		{
			const std::lock_guard Lock(m_SnapshotMutex);
			const uint64_t iPreviousRevision =
				nullptr != m_pSnapshot ? m_pSnapshot->Get_Revision() : 0u;
			if ((std::numeric_limits<uint64_t>::max)() == iPreviousRevision)
			{
				return Fail(strOutError,
					"Effect V2 catalog revision is exhausted.");
			}
			pStaged->m_iRevision = iPreviousRevision + 1u;
			m_pSnapshot = std::move(pStaged);
			m_strBossValtanBindingDraftBaselineBytes.clear();
			m_bBossValtanBindingDraftDirty = false;
		}
		strOutError = strIsolationSummary;
		return true;
	}
	catch (const std::exception& Exception)
	{
		return Fail(strOutError,
			"Effect V2 catalog reload failed before commit: " +
			std::string(Exception.what()));
	}
	catch (...)
	{
		return Fail(strOutError,
			"Effect V2 catalog reload failed before commit.");
	}
}

bool_t Client::CEffectV2Catalog::
Discard_BossValtanBindingDraftAndReload(std::string& strOutError)
{
	try
	{
		std::vector<EFFECT_V2_DOCUMENT> StagedDocuments;
		std::vector<EFFECT_V2_GROUP> StagedGroups;
		std::vector<EFFECT_V2_BINDING> StagedBindings;
		std::vector<std::string> Diagnostics;
		bool_t bBindingsComplete = true;
		if (!Stage_Documents(
				StagedDocuments, Diagnostics, strOutError) ||
			!Stage_Groups(
				StagedGroups, Diagnostics, strOutError) ||
			!Stage_BossValtanBindings(
				StagedBindings, bBindingsComplete,
				Diagnostics, strOutError) ||
			!Isolate_InvalidCrossReferences(
				StagedDocuments, StagedGroups, StagedBindings,
				bBindingsComplete, Diagnostics, strOutError))
		{
			return false;
		}

		auto pStaged = std::make_shared<EFFECT_V2_CATALOG_SNAPSHOT>();
		pStaged->m_Documents = std::move(StagedDocuments);
		pStaged->m_Groups = std::move(StagedGroups);
		pStaged->m_BossValtanBindings = std::move(StagedBindings);
		pStaged->m_Diagnostics = std::move(Diagnostics);
		pStaged->m_bBossValtanBindingsComplete = bBindingsComplete;
		const std::string strIsolationSummary =
			Format_IsolationSummary(pStaged->m_Diagnostics);
		bool_t bDiscardedDraft = false;

		{
			const std::lock_guard Lock(m_SnapshotMutex);
			const uint64_t iPreviousRevision =
				nullptr != m_pSnapshot ? m_pSnapshot->Get_Revision() : 0u;
			if ((std::numeric_limits<uint64_t>::max)() == iPreviousRevision)
			{
				return Fail(strOutError,
					"Effect V2 catalog revision is exhausted.");
			}
			pStaged->m_iRevision = iPreviousRevision + 1u;
			bDiscardedDraft = m_bBossValtanBindingDraftDirty;
			m_pSnapshot = std::move(pStaged);
			m_strBossValtanBindingDraftBaselineBytes.clear();
			m_bBossValtanBindingDraftDirty = false;
		}
		CEffectV2Runtime::Invalidate_Caches();
		strOutError = bDiscardedDraft ?
			"Discarded the unsaved Effect V2 BOSS_VALTAN binding draft and reloaded the physical owner. " +
				strIsolationSummary :
			"Reloaded the Effect V2 BOSS_VALTAN owner; no draft required discard. " +
				strIsolationSummary;
		return true;
	}
	catch (const std::exception& Exception)
	{
		return Fail(strOutError,
			"Effect V2 discard/reload failed before commit; the previous snapshot was preserved: " +
			std::string(Exception.what()));
	}
	catch (...)
	{
		return Fail(strOutError,
			"Effect V2 discard/reload failed before commit; the previous snapshot was preserved.");
	}
}

bool_t Client::CEffectV2Catalog::Commit_BossValtanBindingsLocked(
	std::vector<EFFECT_V2_BINDING> CandidateBindings,
	const char* pOperation,
	const bool_t bWriteOwner,
	std::string& strOutError)
{
	if (nullptr == m_pSnapshot || !m_pSnapshot->Is_Ready())
	{
		return Fail(strOutError,
			"Reload the Effect V2 catalog before mutating a binding.");
	}
	if (!m_pSnapshot->Can_MutateBossValtanBindings())
	{
		return Fail(strOutError,
			"The snapshot did not admit one complete formatVersion 2 bindings "
			"owner. Reload and repair the source before saving.");
	}
	if ((std::numeric_limits<uint64_t>::max)() ==
		m_pSnapshot->Get_Revision())
	{
		return Fail(strOutError,
			"Effect V2 catalog revision is exhausted.");
	}

	auto pCandidate = std::make_shared<EFFECT_V2_CATALOG_SNAPSHOT>();
	pCandidate->m_Documents = m_pSnapshot->m_Documents;
	pCandidate->m_Groups = m_pSnapshot->m_Groups;
	pCandidate->m_BossValtanBindings = std::move(CandidateBindings);
	pCandidate->m_Diagnostics = m_pSnapshot->m_Diagnostics;
	pCandidate->m_bBossValtanBindingsComplete = true;
	if (!Cross_Validate(
			pCandidate->m_Documents, pCandidate->m_Groups,
			pCandidate->m_BossValtanBindings, strOutError) ||
		!Validate_NoLeafGroupClockOverlap(
			pCandidate->m_Groups, pCandidate->m_BossValtanBindings,
			strOutError))
	{
		return false;
	}

	const std::string strSerialized = CEffectV2Document::Serialize_Bindings(
		BOSS_VALTAN_ARCHETYPE_ID, pCandidate->m_BossValtanBindings);
	std::vector<EFFECT_V2_BINDING> ParsedBindings;
	if (!CEffectV2Document::Parse_Bindings(
			strSerialized, BOSS_VALTAN_ARCHETYPE_ID,
			ParsedBindings, strOutError) ||
		!Cross_Validate(
			pCandidate->m_Documents, pCandidate->m_Groups,
			ParsedBindings, strOutError) ||
		!Validate_NoLeafGroupClockOverlap(
			pCandidate->m_Groups, ParsedBindings, strOutError))
	{
		strOutError = "Effect V2 binding self-validation failed: " +
			strOutError;
		return false;
	}
	pCandidate->m_BossValtanBindings = std::move(ParsedBindings);
	pCandidate->m_iRevision = m_pSnapshot->Get_Revision() + 1u;
	std::string strSuccess = bWriteOwner ?
		"Saved Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json (" +
			std::string(nullptr != pOperation ? pOperation : "mutation") +
			"). Runtime caches were refreshed for the next Stage occurrence." :
		"Staged Effect V2 binding " +
			std::string(nullptr != pOperation ? pOperation : "mutation") +
			" in the Composition draft. Use Save to commit every owner together.";
	std::vector<EFFECT_V2_DOCUMENT> DiskDocuments;
	std::vector<EFFECT_V2_GROUP> DiskGroups;
	std::vector<EFFECT_V2_BINDING> DiskBindings;
	std::vector<std::string> DiskDiagnostics;
	bool_t bDiskBindingsComplete = true;
	std::string strDiskSourceBytes;
	if (!Stage_Documents(
			DiskDocuments, DiskDiagnostics, strOutError) ||
		!Stage_Groups(
			DiskGroups, DiskDiagnostics, strOutError) ||
		!Stage_BossValtanBindings(
			DiskBindings, bDiskBindingsComplete,
			DiskDiagnostics, strOutError, &strDiskSourceBytes) ||
		!Isolate_InvalidCrossReferences(
			DiskDocuments, DiskGroups, DiskBindings,
			bDiskBindingsComplete, DiskDiagnostics, strOutError))
	{
		strOutError =
			"Effect V2 validation read-set could not be re-read before save: " +
			strOutError;
		return false;
	}
	if (!bDiskBindingsComplete)
	{
		return Fail(strOutError,
			"Effect V2 bindings changed to an incomplete formatVersion 2 owner; "
			"reload and repair the source before saving.");
	}
	const std::string strDiskBaseline = CEffectV2Document::Serialize_Bindings(
		BOSS_VALTAN_ARCHETYPE_ID, DiskBindings);
	if (bWriteOwner && m_bBossValtanBindingDraftDirty)
	{
		return Fail(strOutError,
			"Effect V2 immediate save is blocked while Composition owns an unsaved binding draft.");
	}
	const bool_t bBindingBaselineMatches = m_bBossValtanBindingDraftDirty ?
		strDiskSourceBytes == m_strBossValtanBindingDraftBaselineBytes :
		strDiskBaseline == CEffectV2Document::Serialize_Bindings(
			BOSS_VALTAN_ARCHETYPE_ID, m_pSnapshot->m_BossValtanBindings);
	if (!Matches_DocumentBaseline(
			DiskDocuments, m_pSnapshot->m_Documents) ||
		!Matches_GroupBaseline(DiskGroups, m_pSnapshot->m_Groups) ||
		!bBindingBaselineMatches)
	{
		return Fail(strOutError,
			"Effect V2 catalog source changed; reload before saving.");
	}
	if (!Cross_Validate(
			DiskDocuments, DiskGroups,
			pCandidate->m_BossValtanBindings, strOutError) ||
		!Validate_NoLeafGroupClockOverlap(
			DiskGroups, pCandidate->m_BossValtanBindings, strOutError))
	{
		strOutError =
			"Effect V2 candidate no longer validates against the current read-set: " +
			strOutError;
		return false;
	}
	if (bWriteOwner && !CEffectV2Document::Write_AtomicFile(
			CEffectV2Document::Binding_Path(BOSS_VALTAN_ARCHETYPE_ID),
			strSerialized, strOutError))
	{
		strOutError = "Effect V2 binding save failed before commit: " +
			strOutError;
		return false;
	}

	m_pSnapshot = std::move(pCandidate);
	if (bWriteOwner)
	{
		m_strBossValtanBindingDraftBaselineBytes.clear();
		m_bBossValtanBindingDraftDirty = false;
	}
	else
	{
		if (!m_bBossValtanBindingDraftDirty)
			m_strBossValtanBindingDraftBaselineBytes =
				std::move(strDiskSourceBytes);
		m_bBossValtanBindingDraftDirty = true;
	}
	CEffectV2Runtime::Invalidate_Caches();
	strOutError = std::move(strSuccess);
	return true;
}

bool_t Client::CEffectV2Catalog::Mutate_BossValtanStageBinding(
	const EFFECT_V2_STAGE_BINDING_KEY& SourceKey,
	const uint32_t iTargetStartMs,
	const BOSS_VALTAN_BINDING_MUTATION eMutation,
	const bool_t bWriteOwner,
	std::string& strOutError)
{
	if (BOSS_VALTAN_BINDING_MUTATION::APPEND_BINDING == eMutation)
	{
		if (!Validate_StageBindingAppendRequest(SourceKey, strOutError) ||
			!Validate_BossValtanMutationSubject(SourceKey, strOutError))
		{
			return false;
		}
	}
	else if (!Validate_StageBindingIdentity(SourceKey, strOutError))
	{
		return false;
	}
	if (iTargetStartMs > MAX_BINDING_MS)
	{
		return Fail(strOutError,
			"Effect V2 mutation target startMs exceeds the ten-minute authoring limit.");
	}
	try
	{
		const std::lock_guard Lock(m_SnapshotMutex);
		if (nullptr == m_pSnapshot || !m_pSnapshot->Is_Ready())
		{
			return Fail(strOutError,
				"Reload the Effect V2 catalog before mutating a binding.");
		}

		std::vector<EFFECT_V2_BINDING> CandidateBindings =
			m_pSnapshot->m_BossValtanBindings;
		const char* pOperation = "mutation";
		switch (eMutation)
		{
		case BOSS_VALTAN_BINDING_MUTATION::APPEND_BINDING:
		{
			if (SourceKey.bGroup)
			{
				if (nullptr == m_pSnapshot->Find_Group(SourceKey.strResourceId))
				{
					return Fail(strOutError,
						"Effect V2 append group is not in the admitted catalog: " +
						SourceKey.strResourceId);
				}
			}
			else if (nullptr ==
				m_pSnapshot->Find_Document(SourceKey.strResourceId))
			{
				return Fail(strOutError,
					"Effect V2 append leaf is not in the admitted catalog: " +
					SourceKey.strResourceId);
			}
			EFFECT_V2_BINDING Binding;
			Binding.eResourceKind = SourceKey.bGroup ?
				EFFECT_V2_RESOURCE_KIND::GROUP : EFFECT_V2_RESOURCE_KIND::LEAF;
			Binding.strResourceId = SourceKey.strResourceId;
			Binding.strPatternId = SourceKey.strPatternId;
			Binding.strStageId = SourceKey.strStageId;
			Binding.strActionId = SourceKey.strActionId;
			Binding.eClockBasis = EFFECT_V2_CLOCK_BASIS::STAGE;
			Binding.strClipOccurrenceId.clear();
			Binding.iStartMs = SourceKey.iStartMs;
			Binding.eRepeatPolicy = EFFECT_V2_REPEAT_POLICY::ONCE;
			Binding.strAnchorSlotId = "b_effectroot";
			Binding.eFollowPolicy = EFFECT_V2_FOLLOW_POLICY::SNAPSHOT_AT_START;
			Binding.eRotationBasis = EFFECT_V2_ROTATION_BASIS::TARGET_YAW;
			Binding.LocalTransform = {};
			Binding.eStopPolicy = EFFECT_V2_STOP_POLICY::NATURAL;
			Binding.strBindingId = Generate_StableBindingId(
				CandidateBindings, Binding);
			if (Binding.strBindingId.empty())
			{
				return Fail(strOutError,
					"Effect V2 append could not allocate a unique stable bindingId.");
			}
			CandidateBindings.push_back(std::move(Binding));
			pOperation = "append";
			break;
		}
		case BOSS_VALTAN_BINDING_MUTATION::REMOVE_BINDING:
		{
			size_t iSourceIndex = 0u;
			if (!Resolve_UniqueStageBindingIndex(
					CandidateBindings, SourceKey, iSourceIndex, strOutError))
			{
				return false;
			}
			CandidateBindings.erase(CandidateBindings.begin() + iSourceIndex);
			pOperation = "remove";
			break;
		}
		case BOSS_VALTAN_BINDING_MUTATION::DUPLICATE_BINDING:
		case BOSS_VALTAN_BINDING_MUTATION::UPDATE_BINDING_START:
		{
			size_t iSourceIndex = 0u;
			if (!Resolve_UniqueStageBindingIndex(
					CandidateBindings, SourceKey, iSourceIndex, strOutError))
			{
				return false;
			}
			if (CandidateBindings[iSourceIndex].iStartMs == iTargetStartMs)
			{
				return Fail(strOutError,
					"Effect V2 mutation requires a different typed clock.");
			}
			if (BOSS_VALTAN_BINDING_MUTATION::DUPLICATE_BINDING == eMutation)
			{
				EFFECT_V2_BINDING Duplicate = CandidateBindings[iSourceIndex];
				Duplicate.iStartMs = iTargetStartMs;
				Duplicate.strBindingId = Generate_StableBindingId(
					CandidateBindings, Duplicate);
				if (Duplicate.strBindingId.empty())
				{
					return Fail(strOutError,
						"Effect V2 duplicate could not allocate a unique stable bindingId.");
				}
				CandidateBindings.push_back(std::move(Duplicate));
				pOperation = "duplicate";
			}
			else
			{
				CandidateBindings[iSourceIndex].iStartMs = iTargetStartMs;
				pOperation = "update start";
			}
			break;
		}
		default:
			return Fail(strOutError,
				"Effect V2 mutation rejected an unknown typed operation.");
		}

		if (BOSS_VALTAN_BINDING_MUTATION::REMOVE_BINDING != eMutation &&
			!Validate_NoLeafGroupClockOverlap(
				m_pSnapshot->m_Groups, CandidateBindings, strOutError))
		{
			return false;
		}
		return Commit_BossValtanBindingsLocked(
			std::move(CandidateBindings), pOperation, bWriteOwner, strOutError);
	}
	catch (const std::exception& Exception)
	{
		return Fail(strOutError,
			"Effect V2 binding mutation failed before commit: " +
			std::string(Exception.what()));
	}
	catch (...)
	{
		return Fail(strOutError,
			"Effect V2 binding mutation failed before commit.");
	}
}

bool_t Client::CEffectV2Catalog::Append_BossValtanStageBinding(
	const std::string& strResourceId,
	const bool_t bGroup,
	const std::string& strPatternId,
	const std::string& strStageId,
	const std::string& strActionId,
	const uint32_t iStartMs,
	std::string& strOutError)
{
	EFFECT_V2_STAGE_BINDING_KEY Key{};
	Key.strResourceId = strResourceId;
	Key.bGroup = bGroup;
	Key.strPatternId = strPatternId;
	Key.strStageId = strStageId;
	Key.strActionId = strActionId;
	Key.iStartMs = iStartMs;
	return Mutate_BossValtanStageBinding(
		Key, iStartMs,
		BOSS_VALTAN_BINDING_MUTATION::APPEND_BINDING, true, strOutError);
}

bool_t Client::CEffectV2Catalog::Remove_BossValtanStageBinding(
	const EFFECT_V2_STAGE_BINDING_KEY& Key,
	std::string& strOutError)
{
	return Mutate_BossValtanStageBinding(
		Key, Key.iStartMs,
		BOSS_VALTAN_BINDING_MUTATION::REMOVE_BINDING, true, strOutError);
}

bool_t Client::CEffectV2Catalog::Duplicate_BossValtanStageBinding(
	const EFFECT_V2_STAGE_BINDING_KEY& SourceKey,
	const uint32_t iDuplicateStartMs,
	std::string& strOutError)
{
	return Mutate_BossValtanStageBinding(
		SourceKey, iDuplicateStartMs,
		BOSS_VALTAN_BINDING_MUTATION::DUPLICATE_BINDING, true, strOutError);
}

bool_t Client::CEffectV2Catalog::Update_BossValtanStageBindingStart(
	const EFFECT_V2_STAGE_BINDING_KEY& SourceKey,
	const uint32_t iNewStartMs,
	std::string& strOutError)
{
	return Mutate_BossValtanStageBinding(
		SourceKey, iNewStartMs,
		BOSS_VALTAN_BINDING_MUTATION::UPDATE_BINDING_START, true, strOutError);
}

bool_t Client::CEffectV2Catalog::Stage_AppendBossValtanStageBinding(
	const std::string& strResourceId,
	const bool_t bGroup,
	const std::string& strPatternId,
	const std::string& strStageId,
	const std::string& strActionId,
	const uint32_t iStartMs,
	std::string& strOutError)
{
	EFFECT_V2_STAGE_BINDING_KEY Key{};
	Key.strResourceId = strResourceId;
	Key.bGroup = bGroup;
	Key.strPatternId = strPatternId;
	Key.strStageId = strStageId;
	Key.strActionId = strActionId;
	Key.iStartMs = iStartMs;
	return Mutate_BossValtanStageBinding(
		Key, iStartMs,
		BOSS_VALTAN_BINDING_MUTATION::APPEND_BINDING, false, strOutError);
}

bool_t Client::CEffectV2Catalog::Stage_RemoveBossValtanStageBinding(
	const EFFECT_V2_STAGE_BINDING_KEY& Key,
	std::string& strOutError)
{
	return Mutate_BossValtanStageBinding(
		Key, Key.iStartMs,
		BOSS_VALTAN_BINDING_MUTATION::REMOVE_BINDING, false, strOutError);
}

bool_t Client::CEffectV2Catalog::Stage_DuplicateBossValtanStageBinding(
	const EFFECT_V2_STAGE_BINDING_KEY& SourceKey,
	const uint32_t iDuplicateStartMs,
	std::string& strOutError)
{
	return Mutate_BossValtanStageBinding(
		SourceKey, iDuplicateStartMs,
		BOSS_VALTAN_BINDING_MUTATION::DUPLICATE_BINDING, false, strOutError);
}

bool_t Client::CEffectV2Catalog::Stage_UpdateBossValtanStageBindingStart(
	const EFFECT_V2_STAGE_BINDING_KEY& SourceKey,
	const uint32_t iNewStartMs,
	std::string& strOutError)
{
	return Mutate_BossValtanStageBinding(
		SourceKey, iNewStartMs,
		BOSS_VALTAN_BINDING_MUTATION::UPDATE_BINDING_START, false, strOutError);
}

bool_t Client::CEffectV2Catalog::Prepare_BossValtanBindingDraftSave(
	std::string& strOutBaselineBytes,
	std::string& strOutCandidateBytes,
	uint64_t& iOutDraftRevision,
	bool_t& bOutDirty,
	std::string& strOutError) const
{
	strOutBaselineBytes.clear();
	strOutCandidateBytes.clear();
	iOutDraftRevision = 0u;
	bOutDirty = false;
	const std::lock_guard Lock(m_SnapshotMutex);
	if (!m_bBossValtanBindingDraftDirty)
	{
		strOutError = "Effect V2 binding owner has no staged Composition changes.";
		return true;
	}
	if (nullptr == m_pSnapshot || !m_pSnapshot->Is_Ready() ||
		!m_pSnapshot->Can_MutateBossValtanBindings() ||
		m_strBossValtanBindingDraftBaselineBytes.empty())
	{
		return Fail(strOutError,
			"Effect V2 binding draft cannot be prepared from an incomplete catalog snapshot.");
	}

	const std::string Candidate = CEffectV2Document::Serialize_Bindings(
		BOSS_VALTAN_ARCHETYPE_ID, m_pSnapshot->m_BossValtanBindings);
	std::vector<EFFECT_V2_BINDING> Parsed;
	if (!CEffectV2Document::Parse_Bindings(
			Candidate, BOSS_VALTAN_ARCHETYPE_ID, Parsed, strOutError) ||
		!Cross_Validate(
			m_pSnapshot->m_Documents, m_pSnapshot->m_Groups,
			Parsed, strOutError) ||
		!Validate_NoLeafGroupClockOverlap(
			m_pSnapshot->m_Groups, Parsed, strOutError))
	{
		strOutError = "Effect V2 binding draft validation failed: " + strOutError;
		return false;
	}
	std::vector<EFFECT_V2_BINDING> DiskBindings;
	std::vector<std::string> DiskDiagnostics;
	bool_t bDiskComplete = true;
	std::string strDiskSourceBytes;
	if (!Stage_BossValtanBindings(
			DiskBindings, bDiskComplete, DiskDiagnostics, strOutError,
			&strDiskSourceBytes) ||
		!bDiskComplete ||
		strDiskSourceBytes != m_strBossValtanBindingDraftBaselineBytes)
	{
		return Fail(strOutError,
			"Effect V2 binding source changed after this Composition draft began; Load before saving.");
	}

	strOutBaselineBytes = m_strBossValtanBindingDraftBaselineBytes;
	strOutCandidateBytes = Candidate;
	iOutDraftRevision = m_pSnapshot->Get_Revision();
	bOutDirty = true;
	strOutError = "Prepared the Effect V2 binding draft for the Composition Save transaction.";
	return true;
}

bool_t Client::CEffectV2Catalog::Accept_BossValtanBindingDraftSave(
	const uint64_t iExpectedDraftRevision,
	const std::string& strExpectedCandidateBytes,
	std::string& strOutError)
{
	const std::lock_guard Lock(m_SnapshotMutex);
	if (!m_bBossValtanBindingDraftDirty || nullptr == m_pSnapshot ||
		m_pSnapshot->Get_Revision() != iExpectedDraftRevision ||
		CEffectV2Document::Serialize_Bindings(
			BOSS_VALTAN_ARCHETYPE_ID, m_pSnapshot->m_BossValtanBindings) !=
			strExpectedCandidateBytes)
	{
		return Fail(strOutError,
			"Effect V2 binding draft changed while the Composition Save transaction was running.");
	}
	std::vector<EFFECT_V2_BINDING> DiskBindings;
	std::vector<std::string> DiskDiagnostics;
	bool_t bDiskComplete = true;
	if (!Stage_BossValtanBindings(
			DiskBindings, bDiskComplete, DiskDiagnostics, strOutError) ||
		!bDiskComplete ||
		CEffectV2Document::Serialize_Bindings(
			BOSS_VALTAN_ARCHETYPE_ID, DiskBindings) != strExpectedCandidateBytes)
	{
		return Fail(strOutError,
			"The Composition transaction completed, but the Effect V2 owner did not reopen as the exact committed draft.");
	}
	m_strBossValtanBindingDraftBaselineBytes.clear();
	m_bBossValtanBindingDraftDirty = false;
	CEffectV2Runtime::Invalidate_Caches();
	strOutError = "Effect V2 binding draft committed and reopened.";
	return true;
}

bool_t Client::CEffectV2Catalog::Has_BossValtanBindingDraft() const
{
	const std::lock_guard Lock(m_SnapshotMutex);
	return m_bBossValtanBindingDraftDirty;
}

std::shared_ptr<const Client::EFFECT_V2_CATALOG_SNAPSHOT>
Client::CEffectV2Catalog::Get_Snapshot() const
{
	const std::lock_guard Lock(m_SnapshotMutex);
	return m_pSnapshot;
}

uint64_t Client::CEffectV2Catalog::Get_Revision() const
{
	const std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> pSnapshot =
		Get_Snapshot();
	return nullptr != pSnapshot ? pSnapshot->Get_Revision() : 0u;
}
