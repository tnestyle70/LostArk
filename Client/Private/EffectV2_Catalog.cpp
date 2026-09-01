#include "EffectV2_Catalog.h"
#include "DataJson.h"
#include "EffectV2_Runtime.h"

#include <algorithm>
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

	std::string Json_String(const std::string_view strValue)
	{
		return "\"" + Client::CDataJson::Escape(strValue) + "\"";
	}

	std::string Serialize_JsonValue(const Client::DATA_JSON_VALUE& Value)
	{
		switch (Value.Get_Type())
		{
		case Client::DATA_JSON_TYPE::NULL_VALUE:
			return "null";
		case Client::DATA_JSON_TYPE::BOOLEAN:
			return Value.Get_Boolean() ? "true" : "false";
		case Client::DATA_JSON_TYPE::NUMBER:
		{
			char szNumber[64]{};
			std::snprintf(
				szNumber, sizeof(szNumber), "%.17g", Value.Get_Number());
			return szNumber;
		}
		case Client::DATA_JSON_TYPE::STRING:
			return Json_String(Value.Get_String());
		case Client::DATA_JSON_TYPE::ARRAY:
		{
			std::string strText = "[";
			for (size_t iIndex = 0u; iIndex < Value.Get_Array().size(); ++iIndex)
			{
				if (0u != iIndex)
					strText += ',';
				strText += Serialize_JsonValue(Value.Get_Array()[iIndex]);
			}
			return strText + ']';
		}
		case Client::DATA_JSON_TYPE::OBJECT:
		{
			std::string strText = "{";
			size_t iIndex = 0u;
			for (const auto& [strKey, Child] : Value.Get_Object())
			{
				if (0u != iIndex++)
					strText += ',';
				strText += Json_String(strKey) + ':' + Serialize_JsonValue(Child);
			}
			return strText + '}';
		}
		default:
			return "null";
		}
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

	bool_t Matches_BindingParserIdentity(
		const Client::EFFECT_V2_BINDING& Left,
		const Client::EFFECT_V2_BINDING& Right)
	{
		return Left.strEffectId == Right.strEffectId &&
			Left.strGroupId == Right.strGroupId &&
			Left.strClip == Right.strClip &&
			Left.strStage == Right.strStage &&
			Left.iStartMs == Right.iStartMs &&
			Left.strBone == Right.strBone;
	}

	bool_t Stage_BossValtanBindings(
		std::vector<Client::EFFECT_V2_BINDING>& OutBindings,
		bool_t& bOutComplete,
		std::vector<std::string>& Diagnostics,
		std::string& strOutError,
		std::string* const pOutSourceBytes = nullptr)
	{
		OutBindings.clear();
		bOutComplete = true;
		if (nullptr != pOutSourceBytes)
			pOutSourceBytes->clear();
		const std::filesystem::path Path =
			Client::CEffectV2Document::Binding_Path(BOSS_VALTAN_ARCHETYPE_ID);
		std::string strText;
		if (!Read_TextFile(Path, strText, strOutError))
		{
			Diagnose(Diagnostics,
				"Skipped BOSS_VALTAN Effect V2 bindings: " + strOutError);
			strOutError.clear();
			bOutComplete = false;
			return true;
		}
		if (nullptr != pOutSourceBytes)
			*pOutSourceBytes = strText;

		Client::DATA_JSON_VALUE Root;
		if (!Client::CDataJson::Parse(strText, Root, strOutError) ||
			!Root.Is_Object())
		{
			if (strOutError.empty())
				strOutError = "bindings root is not an object.";
			Diagnose(Diagnostics,
				"Skipped BOSS_VALTAN Effect V2 bindings: " + strOutError);
			strOutError.clear();
			bOutComplete = false;
			return true;
		}

		const Client::DATA_JSON_VALUE* pSchema = Root.Find("schema");
		const Client::DATA_JSON_VALUE* pVersion = Root.Find("formatVersion");
		const Client::DATA_JSON_VALUE* pArchetype = Root.Find("archetypeId");
		const Client::DATA_JSON_VALUE* pRows = Root.Find("bindings");
		if (nullptr == pSchema || !pSchema->Is_String() ||
			pSchema->Get_String() != "lostark.effect-v2-bindings" ||
			nullptr == pVersion || !pVersion->Is_Number() ||
			pVersion->Get_Number() != 1.0 ||
			nullptr == pArchetype || !pArchetype->Is_String() ||
			pArchetype->Get_String() != BOSS_VALTAN_ARCHETYPE_ID ||
			nullptr == pRows || !pRows->Is_Array())
		{
			Diagnose(Diagnostics,
				"Skipped BOSS_VALTAN Effect V2 bindings: "
				"schema/formatVersion/archetypeId/bindings mismatch.");
			bOutComplete = false;
			return true;
		}

		const auto& Rows = pRows->Get_Array();
		OutBindings.reserve(Rows.size());
		for (size_t iRow = 0u; iRow < Rows.size(); ++iRow)
		{
			const std::string strSingleRow =
				"{\"schema\":\"lostark.effect-v2-bindings\","
				"\"formatVersion\":1,\"archetypeId\":" +
				Json_String(BOSS_VALTAN_ARCHETYPE_ID) +
				",\"bindings\":[" + Serialize_JsonValue(Rows[iRow]) + "]}";
			std::vector<Client::EFFECT_V2_BINDING> Parsed;
			std::string strRowError;
			if (!Client::CEffectV2Document::Parse_Bindings(
					strSingleRow, BOSS_VALTAN_ARCHETYPE_ID,
					Parsed, strRowError) ||
				1u != Parsed.size())
			{
				Diagnose(Diagnostics,
					"Skipped BOSS_VALTAN Effect V2 binding row " +
					std::to_string(iRow) + ": " + strRowError);
				bOutComplete = false;
				continue;
			}

			const auto Duplicate = std::find_if(
				OutBindings.begin(), OutBindings.end(),
				[&Parsed](const Client::EFFECT_V2_BINDING& Existing)
				{
					return Matches_BindingParserIdentity(
						Existing, Parsed.front());
				});
			if (Duplicate != OutBindings.end())
			{
				Diagnose(Diagnostics,
					"Skipped duplicate BOSS_VALTAN Effect V2 binding row " +
					std::to_string(iRow) + ".");
				bOutComplete = false;
				continue;
			}
			OutBindings.push_back(std::move(Parsed.front()));
		}
		return true;
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
				if (!DocumentsById.contains(Child.strEffectId))
				{
					return Fail(strOutError,
						"Effect V2 group child has no authored leaf document: " +
						Group.strGroupId + " -> " + Child.strEffectId);
				}
			}
		}

		for (const Client::EFFECT_V2_BINDING& Binding : Bindings)
		{
			if (Binding.strEffectId.empty() == Binding.strGroupId.empty() ||
				Binding.strClip.empty() == Binding.strStage.empty())
			{
				return Fail(strOutError,
					"BOSS_VALTAN Effect V2 binding lost its exact subject or clock owner.");
			}
			if (!Binding.strEffectId.empty())
			{
				if (!DocumentsById.contains(Binding.strEffectId))
				{
					return Fail(strOutError,
						"BOSS_VALTAN Effect V2 binding has no authored document: " +
						Binding.strEffectId);
				}
			}
			else if (!GroupsById.contains(Binding.strGroupId))
			{
				return Fail(strOutError,
					"BOSS_VALTAN Effect V2 binding has no group document: " +
					Binding.strGroupId);
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

	bool_t Validate_StageBindingKey(
		const Client::EFFECT_V2_STAGE_BINDING_KEY& Key,
		std::string& strOutError)
	{
		if (!Client::CEffectV2Document::Is_ValidEffectId(Key.strResourceId))
		{
			return Fail(strOutError,
				"Effect V2 mutation rejected an invalid stable resource ID.");
		}
		if (Key.strStageActionId.empty() == Key.strClipName.empty() ||
			Key.strStageActionId.size() > 160u || Key.strClipName.size() > 160u)
		{
			return Fail(strOutError,
				"Effect V2 mutation requires exactly one stable Server Stage actionId or Animation clip name.");
		}
		if (Key.iStartMs > MAX_BINDING_MS)
		{
			return Fail(strOutError,
				"Effect V2 mutation startMs exceeds the ten-minute authoring limit.");
		}
		if (Key.strBone.size() > 160u ||
			Key.eRotation < Client::CEffectV2Object::PIVOT_ROTATION::BONE ||
			Key.eRotation >= Client::CEffectV2Object::PIVOT_ROTATION::END ||
			!std::isfinite(Key.vOffset.x) ||
			!std::isfinite(Key.vOffset.y) ||
			!std::isfinite(Key.vOffset.z) ||
			!std::isfinite(Key.fYawDegrees))
		{
			return Fail(strOutError,
				"Effect V2 mutation requires one valid persisted row baseline.");
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
		return Binding.strStage == Key.strStageActionId &&
			Binding.strClip == Key.strClipName &&
			Binding.iStartMs == Key.iStartMs &&
			Binding.strBone == Key.strBone &&
			Binding.bFollowBone == Key.bFollowBone &&
			Binding.eRotation == Key.eRotation &&
			Binding.bStopWithClip == Key.bStopWithClip &&
			Binding.vOffset.x == Key.vOffset.x &&
			Binding.vOffset.y == Key.vOffset.y &&
			Binding.vOffset.z == Key.vOffset.z &&
			Binding.fYawDegrees == Key.fYawDegrees &&
			(Key.bGroup ? Binding.strEffectId.empty() :
				Binding.strGroupId.empty()) &&
			(Key.bGroup ? Binding.strGroupId : Binding.strEffectId) ==
				Key.strResourceId;
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

	bool_t Contains_StageBindingKey(
		const std::vector<Client::EFFECT_V2_BINDING>& Bindings,
		const Client::EFFECT_V2_STAGE_BINDING_KEY& Key,
		const size_t iIgnoredIndex = (std::numeric_limits<size_t>::max)())
	{
		for (size_t iIndex = 0u; iIndex < Bindings.size(); ++iIndex)
		{
			if (iIndex != iIgnoredIndex &&
				Matches_StageBindingKey(Bindings[iIndex], Key))
			{
				return true;
			}
		}
		return false;
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
			if (LeafBinding.strEffectId.empty())
				continue;
			for (const Client::EFFECT_V2_BINDING& GroupBinding : Bindings)
			{
				if (GroupBinding.strGroupId.empty() ||
					LeafBinding.strClip != GroupBinding.strClip ||
					LeafBinding.strStage != GroupBinding.strStage)
				{
					continue;
				}
				const auto Found = GroupsById.find(GroupBinding.strGroupId);
				if (Found == GroupsById.end())
					continue;
				for (const Client::EFFECT_V2_GROUP_CHILD& Child :
					Found->second->Children)
				{
					const uint64_t iEffectiveStartMs =
						static_cast<uint64_t>(GroupBinding.iStartMs) +
						static_cast<uint64_t>(Child.iStartMs);
					if (Child.strEffectId == LeafBinding.strEffectId &&
						iEffectiveStartMs == LeafBinding.iStartMs)
					{
						return Fail(strOutError,
							"Effect V2 leaf overlaps the same leaf inside group '" +
							GroupBinding.strGroupId + "' at the same Stage clock: " +
							LeafBinding.strEffectId);
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
					if (!DocumentIds.contains(Child.strEffectId))
					{
						strReason = "missing authored leaf " + Child.strEffectId;
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

		std::vector<Client::EFFECT_V2_BINDING> ValidBindings;
		ValidBindings.reserve(Bindings.size());
		for (Client::EFFECT_V2_BINDING& Binding : Bindings)
		{
			const std::string& strResourceId = Binding.strEffectId.empty() ?
				Binding.strGroupId : Binding.strEffectId;
			const bool_t bResourceExists = Binding.strEffectId.empty() ?
				GroupIds.contains(Binding.strGroupId) :
				DocumentIds.contains(Binding.strEffectId);
			if (!bResourceExists)
			{
				Diagnose(Diagnostics,
					"Skipped BOSS_VALTAN Effect V2 binding for unavailable resource '" +
					strResourceId + "'.");
				bInOutBindingsComplete = false;
				continue;
			}

			std::vector<Client::EFFECT_V2_BINDING> Candidate = ValidBindings;
			Candidate.push_back(Binding);
			std::string strOverlapError;
			if (!Validate_NoLeafGroupClockOverlap(
					Groups, Candidate, strOverlapError))
			{
				Diagnose(Diagnostics,
					"Skipped BOSS_VALTAN Effect V2 binding for '" +
					strResourceId + "': " + strOverlapError);
				bInOutBindingsComplete = false;
				continue;
			}
			ValidBindings.push_back(std::move(Binding));
		}
		Bindings = std::move(ValidBindings);

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
	Key.bGroup = !Binding.strGroupId.empty();
	Key.strResourceId = Key.bGroup ? Binding.strGroupId : Binding.strEffectId;
	Key.strStageActionId = Binding.strStage;
	Key.strClipName = Binding.strClip;
	Key.iStartMs = Binding.iStartMs;
	Key.strBone = Binding.strBone;
	Key.bFollowBone = Binding.bFollowBone;
	Key.eRotation = Binding.eRotation;
	Key.bStopWithClip = Binding.bStopWithClip;
	Key.vOffset = Binding.vOffset;
	Key.fYawDegrees = Binding.fYawDegrees;
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
			"The bindings source contains an isolated row. Repair that row before "
			"saving another binding so the source is not silently discarded.");
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
			"Effect V2 bindings changed to an incomplete source; reload and repair "
			"the isolated row before saving.");
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
	if (!Validate_StageBindingKey(SourceKey, strOutError))
		return false;
	if (BOSS_VALTAN_BINDING_MUTATION::REMOVE_BINDING != eMutation &&
		!Validate_BossValtanMutationSubject(SourceKey, strOutError))
	{
		return false;
	}
	if (iTargetStartMs > MAX_BINDING_MS)
	{
		return Fail(strOutError,
			"Effect V2 mutation target startMs exceeds the ten-minute authoring limit.");
	}
	if (BOSS_VALTAN_BINDING_MUTATION::UPDATE_BINDING_START == eMutation &&
		SourceKey.iStartMs == iTargetStartMs)
	{
		return Fail(strOutError,
			"Effect V2 update requires a different Stage clock.");
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
			if (Contains_StageBindingKey(CandidateBindings, SourceKey))
			{
				return Fail(strOutError,
					"Effect V2 binding already exists with this exact row baseline.");
			}

			EFFECT_V2_BINDING Binding;
			if (SourceKey.bGroup)
				Binding.strGroupId = SourceKey.strResourceId;
			else
				Binding.strEffectId = SourceKey.strResourceId;
			Binding.strStage = SourceKey.strStageActionId;
			Binding.strClip = SourceKey.strClipName;
			Binding.iStartMs = SourceKey.iStartMs;
			Binding.strBone = SourceKey.strBone;
			Binding.bFollowBone = SourceKey.bFollowBone;
			Binding.eRotation = SourceKey.eRotation;
			Binding.bStopWithClip = SourceKey.bStopWithClip;
			Binding.vOffset = SourceKey.vOffset;
			Binding.fYawDegrees = SourceKey.fYawDegrees;
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
			EFFECT_V2_STAGE_BINDING_KEY TargetKey = SourceKey;
			TargetKey.iStartMs = iTargetStartMs;
			const size_t iIgnoredIndex =
				BOSS_VALTAN_BINDING_MUTATION::UPDATE_BINDING_START == eMutation ?
				iSourceIndex : (std::numeric_limits<size_t>::max)();
			if (Contains_StageBindingKey(
					CandidateBindings, TargetKey, iIgnoredIndex))
			{
				return Fail(strOutError,
					"Effect V2 mutation target already exists at this Stage clock.");
			}
			if (BOSS_VALTAN_BINDING_MUTATION::DUPLICATE_BINDING == eMutation)
			{
				EFFECT_V2_BINDING Duplicate = CandidateBindings[iSourceIndex];
				Duplicate.iStartMs = iTargetStartMs;
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
	const std::string& strStageActionId,
	const uint32_t iStartMs,
	std::string& strOutError)
{
	EFFECT_V2_STAGE_BINDING_KEY Key{};
	Key.strResourceId = strResourceId;
	Key.bGroup = bGroup;
	Key.strStageActionId = strStageActionId;
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
	const std::string& strStageActionId,
	const uint32_t iStartMs,
	std::string& strOutError)
{
	EFFECT_V2_STAGE_BINDING_KEY Key{};
	Key.strResourceId = strResourceId;
	Key.bGroup = bGroup;
	Key.strStageActionId = strStageActionId;
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
